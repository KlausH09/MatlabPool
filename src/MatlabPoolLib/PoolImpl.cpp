#include "MatlabPoolLib/PoolImpl.hpp"

namespace MatlabPool
{
    PoolImpl::PoolImpl(unsigned int n, const std::vector<std::u16string> &options)
        : stop(false),
        sleep(false),
        worker_ready(n, false),
        engine(n)
    {
        if (n == 0)
            throw EmptyPool();

        for (auto &e : engine)
            e = std::make_unique<EngineHack>(options);

        worker_ready.flip();

        master = std::thread([=]() {
            for (;;)
            {
                std::unique_lock<std::mutex> lock_jobs(mutex_jobs);
                while ((!stop && jobQueue.empty()) || sleep)
                    cv_queue.wait(lock_jobs);

                if (stop)
                    break;

                std::size_t workerID;
                MatlabPool::EngineHack *worker;

                // wait for next free worker
                lock_jobs.unlock(); // do not block adding new jobs
                {
                    std::unique_lock<std::mutex> lock_worker(mutex_worker);
                    workerID = get_free_worker(lock_worker);
                    worker_ready[workerID] = false;
                    worker = engine[workerID].get();
                }
                Notifier notifier = [=]() {
                    std::unique_lock<std::mutex> lock_worker(mutex_worker);
                    worker_ready[workerID] = true;
                    cv_worker.notify_one();
                };

                lock_jobs.lock();

                // check if there are still jobs in the queue
                if (jobQueue.empty())
                {
                    notifier();
                    continue;
                }

                JobFuture &job = jobQueue.front();
                MATLABPOOL_ASSERT(job.get_status() == JobFeval::Status::Wait);

                job.set_workerID(workerID); // set also job status to "InProgress"
                worker->eval_job(job, std::move(notifier));
                JobID id_tmp = job.get_ID();
                futureMap[id_tmp] = std::move(job);
                jobQueue.pop_front();
                cv_future.notify_one();
            }
            });
    }
    PoolImpl::~PoolImpl()
    {
        // join master thread
        {
            std::unique_lock<std::mutex> lock(mutex_jobs);
            stop = true;
            cv_queue.notify_one();
        }
        master.join();

        // cancel jobs
        {
            std::unique_lock<std::mutex> lock(mutex_jobs);
            futureMap.clear();
        }
    }

    void PoolImpl::resize(unsigned int n_new, const std::vector<std::u16string> &options)
    {
        std::size_t n_old = engine.size();
        if (n_new == 0)
            throw EmptyPool();

        if (n_new < n_old)
        {
            // block the master thread to avoid new job assignments
            {
                std::unique_lock<std::mutex> lock_jobs(mutex_jobs);
                sleep = true;
            }

            {
                std::unique_lock<std::mutex> lock_worker(mutex_worker);
                // remove the last engines, because the engines notify the master with
                // their index, so it is not a good idea to remove e.g. the first engine
                for (std::size_t i = engine.size() - 1; n_new <= i; i--)
                {
                    while (!worker_ready[i])
                        cv_worker.wait(lock_worker);
                    engine.pop_back();
                    worker_ready.pop_back();
                }
            }

            // unblock the master thread
            {
                std::unique_lock<std::mutex> lock_jobs(mutex_jobs);
                sleep = false;
                cv_queue.notify_one();
            }
        }
        else if (n_new > n_old)
        {
            std::unique_lock<std::mutex> lock_worker(mutex_worker);
            for (std::size_t i = n_old; i < n_new; i++)
            {
                engine.push_back(std::make_unique<EngineHack>(options));
                worker_ready.push_back(true);
            }
        }
    }

    std::size_t PoolImpl::size() const
    {
        return engine.size();
    }

    JobID PoolImpl::submit(JobFeval &&job)
    {
        JobID job_id = job.get_ID();
        std::unique_lock<std::mutex> lock_jobs(mutex_jobs);
        jobQueue.push_back(JobFuture(std::move(job)));
        cv_queue.notify_one();
        return job_id;
    }

    JobFeval PoolImpl::wait(JobID id)
    {
        if (!exists(id))
            throw JobNotExists(id);

        std::unique_lock<std::mutex> lock_jobs(mutex_jobs);

        decltype(futureMap)::iterator it;
        while ((it = futureMap.find(id)) == futureMap.end())
        {
            cv_future.wait(lock_jobs);
        }

        auto job = std::move(futureMap[id]);

        futureMap.erase(it);
        job.wait();

        return std::move(job);
    }

    void PoolImpl::eval(JobEval &job)
    {
        std::size_t n = engine.size();

        std::vector<StreamBuf> outBuf_vec(n);
        std::vector<StreamBuf> errBuf_vec(n);

        std::vector<matlab::engine::FutureResult<void>> future(n);

        for (std::size_t i = 0; i < n; i++)
            future[i] = engine[i]->evalAsync(job.get_cmd(),
                outBuf_vec[i].get(),
                errBuf_vec[i].get());

        for (std::size_t i = 0; i < n; i++)
        {
            try
            {
                future[i].get();
            }
            catch (const matlab::engine::Exception &e)
            {
                job.add_error(errBuf_vec[i], i);
            }
            job.add_output(outBuf_vec[i], i);
        }
    }

    matlab::data::StructArray PoolImpl::get_job_status()
    {
        std::unique_lock<std::mutex> lock_jobs(mutex_jobs);
        std::size_t n = jobQueue.size() + futureMap.size();

        using StatusType = std::underlying_type<JobFeval::Status>::type;

        auto jobID = factory.createArray<JobID>({ n });
        auto status = factory.createArray<StatusType>({ n });
        auto worker = factory.createArray<int>({ n });

        std::size_t i = 0;
        for (const auto &j : jobQueue)
        {
            jobID[i] = j.get_ID();
            status[i] = static_cast<StatusType>(j.get_status());
            worker[i] = j.get_workerID();
            ++i;
        }
        for (const auto &j : futureMap)
        {
            jobID[i] = j.second.get_ID();
            status[i] = static_cast<StatusType>(j.second.get_status());
            worker[i] = j.second.get_workerID();
            ++i;
        }
        lock_jobs.unlock();

        auto result = factory.createStructArray({ 1 },
            { "JobID", "Status", "WorkerID" });
        result[0]["JobID"] = std::move(jobID);
        result[0]["Status"] = std::move(status);
        result[0]["WorkerID"] = std::move(worker);

        return result;
    }

    matlab::data::StructArray PoolImpl::get_worker_status()
    {
        std::unique_lock<std::mutex> lock_worker(mutex_worker);
        std::size_t n = engine.size();

        auto ready = factory.createArray<bool>({ n });
        for (std::size_t i = 0; i < n; i++)
            ready[i] = worker_ready[i];

        lock_worker.unlock();

        auto result = factory.createStructArray({ 1 }, { "Ready" });
        result[0]["Ready"] = std::move(ready);
        return result;
    }

    void PoolImpl::cancel(JobID jobID)
    {
        std::unique_lock<std::mutex> lock_jobs(mutex_jobs);

        // job queue
        for (auto it_job = jobQueue.begin(); it_job != jobQueue.end(); ++it_job)
        {
            if (it_job->get_ID() == jobID)
            {
                jobQueue.erase(it_job);
                return;
            }
        }

        // finished jobs or in progress
        auto it_future = futureMap.find(jobID);
        if (it_future != futureMap.end())
        {
            it_future->second.cancel();
            futureMap.erase(it_future);
            return;
        }

        throw JobNotExists(jobID);
    }

    void PoolImpl::clear()
    {
        std::unique_lock<std::mutex> lock_jobs(mutex_jobs);

        // job queue
        while (!jobQueue.empty() && jobQueue.back().get_status() == JobFeval::Status::Wait)
            jobQueue.pop_back();
        MATLABPOOL_ASSERT(jobQueue.empty());

        // future jobs
        futureMap.clear();
    }

    bool PoolImpl::exists(JobID id) noexcept
    {
        std::unique_lock<std::mutex> lock_jobs(mutex_jobs);

        for (const auto &e : jobQueue)
            if (e.get_ID() == id)
                return true;

        for (const auto &e : futureMap)
            if (e.first == id)
                return true;

        return false;
    }

    std::size_t PoolImpl::get_free_worker(std::unique_lock<std::mutex> &lock) noexcept
    {
        for (;;)
        {
            for (std::size_t i = 0; i < worker_ready.size(); i++)
                if (worker_ready[i])
                    return i;

            cv_worker.wait(lock);
        }
    }

} // namespace MatlabPool
