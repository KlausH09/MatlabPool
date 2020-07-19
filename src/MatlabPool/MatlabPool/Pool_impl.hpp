#ifndef MATLABPOOL_POOL_IMPL_HPP
#define MATLABPOOL_POOL_IMPL_HPP

#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <thread>
#include <map>
#include <queue>
#include <exception>
#include <sstream>
#include <memory>
#include <type_traits>
#include <iomanip>

#include "MatlabPool/Pool.hpp"
#include "MatlabPool/JobFeval.hpp"
#include "MatlabPool/JobFuture.hpp"
#include "MatlabPool/EngineHack.hpp"
#include "assert.hpp"

namespace MatlabPool
{
    class PoolImpl : public Pool
    {
        using EnginePtr = std::unique_ptr<EngineHack>;
        PoolImpl(const PoolImpl &) = delete;
        PoolImpl &operator=(const PoolImpl &) = delete;

    public:
        PoolImpl(unsigned int n, const std::vector<std::u16string> &options)
            : stop(false),
              worker_ready(n, false),
              engine(n),
              job_in_progress(0)
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
                    while (!stop && jobs.empty())
                        cv_queue.wait(lock_jobs);

                    if (stop)
                        break;

                    JobFuture &job = jobs.front();
                    job.set_AssignToWorker_status();

                    int workerID;
                    MatlabPool::EngineHack *worker;

                    // wait for next free worker
                    lock_jobs.unlock(); // do not block adding new jobs
                    {
                        std::unique_lock<std::mutex> lock_worker(mutex_worker);
                        workerID = get_free_worker(lock_worker);
                        worker_ready[workerID] = false;
                        worker = engine[workerID].get();
                    }
                    lock_jobs.lock();

                    // check if job is canceled during waitting
                    if (job.get_status() == JobFeval::Status::Canceled)
                    {
                        jobs.pop_front();
                        std::unique_lock<std::mutex> lock_worker(mutex_worker);
                        worker_ready[workerID] = true;
                    }
                    else if (job.get_status() == JobFeval::Status::AssignToWorker)
                    {
                        job.set_workerID(workerID); // set also job status to "InProgress"
                        worker->eval_job(job, [=]() {
                            std::unique_lock<std::mutex> lock_worker(mutex_worker);
                            worker_ready[workerID] = true;
                            cv_worker.notify_one();
                        });
                        JobID id_tmp = job.get_ID();
                        futureMap[id_tmp] = std::move(job);
                        jobs.pop_front();
                        cv_future.notify_one();
                    }
                    else
                    {
                        ERROR("unexpect job status");
                    }
                }
            });
        }
        ~PoolImpl() override
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
                for (auto &e : futureMap)
                    e.second.cancel();
            }
        }

        void resize(unsigned int n_new, const std::vector<std::u16string> &options) override
        {
            if (n_new == 0)
                throw EmptyPool();
            if (n_new == engine.size())
                return;

            std::unique_lock<std::mutex> lock_worker(mutex_worker);

            if (n_new < engine.size())
            {
                // remove the last engines, because the engines notify the master with
                // their index, so it is not a good idea to remove e.g. the first engine

                // block the master thread to avoid new job assignments
                std::unique_lock<std::mutex> lock_jobs(mutex_jobs);

                for (std::size_t i = engine.size() - 1; n_new <= i; i--)
                {
                    while (!worker_ready[i])
                        cv_worker.wait(lock_worker);
                    engine.pop_back();
                    worker_ready.pop_back();
                }
            }

            if (n_new > engine.size())
            {
                for (std::size_t i = engine.size(); i < n_new; i++)
                {
                    engine.push_back(std::make_unique<EngineHack>(options));
                    worker_ready.push_back(true);
                }
            }
        }

        std::size_t size() const override
        {
            return engine.size();
        }

        JobID submit(JobFeval &&job) override
        {
            JobID job_id = job.get_ID();
            std::unique_lock<std::mutex> lock_jobs(mutex_jobs);
            jobs.push_back(JobFuture(std::move(job)));
            cv_queue.notify_one();
            return job_id;
        }

        JobFeval wait(JobID id) override
        {
#ifdef MATLABPOOL_CHECK_EXIST_BEFORE_WAIT
            if (!exists(id))
                throw JobNotExists(id);
#endif
            std::unique_lock<std::mutex> lock_jobs(mutex_jobs);

            decltype(futureMap)::iterator it;
            while ((it = futureMap.find(id)) == futureMap.end())
            {
                cv_future.wait(lock_jobs);
            }

            auto job = std::move(futureMap[id]);

            futureMap.erase(it);
            job.wait();

            return job;
        }

        void eval(JobEval &job) override
        {
            std::size_t n = engine.size();

            std::vector<OutputBuf> outBuf_vec(n);
            std::vector<ErrorBuf> errBuf_vec(n);

            std::vector<matlab::engine::FutureResult<void>> future(n);

            for (std::size_t i = 0; i < n; i++)
                future[i] = engine[i]->evalAsync(job.get_cmd(), outBuf_vec[i].get(), errBuf_vec[i].get());

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

            if(job.get_status() == JobEval::Status::Error)
                throw JobBase::ExecutionError(job.get_errBuf().get());
        }

        matlab::data::StructArray get_job_status() override
        {
            std::unique_lock<std::mutex> lock_jobs(mutex_jobs);
            std::size_t n = jobs.size() + futureMap.size();

            using StatusType = std::underlying_type<JobFeval::Status>::type;

            auto jobID = factory.createArray<JobID>({n});
            auto status = factory.createArray<StatusType>({n});
            auto worker = factory.createArray<int>({n});

            std::size_t i = 0;
            for (const auto &j : jobs)
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

            auto result = factory.createStructArray({1}, {"JobID", "Status", "WorkerID"});
            result[0]["JobID"] = std::move(jobID);
            result[0]["Status"] = std::move(status);
            result[0]["WorkerID"] = std::move(worker);

            return result;
        }

        matlab::data::StructArray get_worker_status() override
        {
            std::unique_lock<std::mutex> lock_worker(mutex_worker);
            std::size_t n = engine.size();

            auto ready = factory.createArray<bool>({n});
            for (std::size_t i = 0; i < n; i++)
                ready[i] = worker_ready[i];

            lock_worker.unlock();

            auto result = factory.createStructArray({1}, {"Ready"});
            result[0]["Ready"] = std::move(ready);
            return result;
        }

        void cancel(JobID jobID) override
        {
            std::unique_lock<std::mutex> lock_jobs(mutex_jobs);

            // job queue
            for (auto it_job = jobs.begin(); it_job != jobs.end(); ++it_job)
            {
                if (it_job->get_ID() == jobID)
                {
                    if (it_job->get_status() == JobFeval::Status::AssignToWorker)
                        it_job->cancel();
                    else if (it_job->get_status() == JobFeval::Status::Wait)
                        jobs.erase(it_job);
                    else
                        ERROR("unexpect job status");
                    return;
                }
            }

            // finished jobs or in progress
            decltype(futureMap)::iterator it_future;
            if ((it_future = futureMap.find(jobID)) != futureMap.end())
            {
                futureMap[jobID].cancel();
                futureMap.erase(it_future);
                return;
            }

            throw JobNotExists(jobID);
        }

    private:
        bool exists(JobID id) noexcept
        {
            std::unique_lock<std::mutex> lock_jobs(mutex_jobs);

            for (const auto &e : jobs)
                if (e.get_ID() == id)
                    return true;

            for (const auto &e : futureMap)
                if (e.first == id)
                    return true;

            return false;
        }

        int get_free_worker(std::unique_lock<std::mutex> &lock) noexcept
        {
            for (;;)
            {
                for (std::size_t i = 0; i < worker_ready.size(); i++)
                    if (worker_ready[i])
                        return int(i);

                cv_worker.wait(lock);
            }
        }

    private:
        bool stop;                      // mutex_jobs
        std::vector<bool> worker_ready; // mutex_worker
        std::vector<EnginePtr> engine;  // mutex_worker
        JobID job_in_progress;          // mutex_jobs

        std::thread master;

        std::deque<JobFuture> jobs;           // mutex_jobs
        std::map<JobID, JobFuture> futureMap; // mutex_jobs
        std::condition_variable cv_queue;
        std::condition_variable cv_worker;
        std::condition_variable cv_future;
        std::mutex mutex_jobs;
        std::mutex mutex_worker;

        matlab::data::ArrayFactory factory;
    };

} // namespace MatlabPool

#endif