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

#include "./Pool.hpp"
#include "./Job.hpp"
#include "./EngineHack.hpp"

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

                    Job_feval job = std::move(jobs.front());
                    jobs.pop_front();
                    job_in_progress = job.id;

                    lock_jobs.unlock(); // do not block adding new jobs

                    MatlabPool::Future futureResult;
                    {
                        std::unique_lock<std::mutex> lock_worker(mutex_worker);
                        int workerID = get_free_worker(lock_worker);
                        job.set_workerID(workerID);
                        worker_ready[workerID] = false;

                        futureResult = engine[workerID].get()->eval_job(job, [=]() {
                            std::unique_lock<std::mutex> lock_worker(mutex_worker);
                            worker_ready[workerID] = true;
                            cv_worker.notify_one();
                        });
                    }
                    lock_jobs.lock();

                    if (job_in_progress)
                    {
                        futureMap[job_in_progress] = std::pair<Job_feval, Future>(std::move(job), std::move(futureResult));
                        job_in_progress = 0;
                        cv_future.notify_one();
                    }
                    else
                    {
                        futureResult.cancel();
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
                    e.second.second.cancel(true);
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

        JobID submit(Job_feval &&job) override
        {
            JobID job_id = job.id;
            std::unique_lock<std::mutex> lock_jobs(mutex_jobs);
            jobs.push_back(std::move(job));
            cv_queue.notify_one();
            return job_id;
        }

        Job_feval wait(JobID id) override // TODO: check error result
        {
#ifdef MATLABPOOL_AVOID_ENDLESS_WAIT
            if (!exists(id))
                throw JobNotExists(id);
#endif
            std::unique_lock<std::mutex> lock_jobs(mutex_jobs);

            decltype(futureMap)::iterator it;
            while ((it = futureMap.find(id)) == futureMap.end())
            {
                cv_future.wait(lock_jobs);
            }

            auto tmp = std::move(futureMap[id]);
            futureMap.erase(it);

            tmp.first.result = tmp.second.get();
            return std::move(tmp.first);
        }

        void eval(Job &job) override // TODO: check error result
        {
            std::lock_guard<std::mutex> lock_worker(mutex_worker);

            std::vector<OutputBuf> outBuf(engine.size());
            std::vector<ErrorBuf> errBuf(engine.size());

            std::vector<matlab::engine::FutureResult<void>> future(engine.size());

            for (std::size_t i = 0; i < engine.size(); i++)
            {
                future[i] = engine[i]->evalAsync(job.function, outBuf[i].get(), errBuf[i].get());
            }

            for (std::size_t i = 0; i < engine.size(); i++)
            {
                future[i].get();
                if (!outBuf[i].empty())
                {
                    job.outputBuf << u"============= Output Worker " << i + 1 << u" ==============";
                    job.outputBuf << outBuf[i].str();
                }
                if (!errBuf[i].empty())
                {
                    job.errorBuf << u"============= Error Worker " << i + 1 << u" ===============";
                    job.errorBuf << errBuf[i].str();
                }
            }
        }

        matlab::data::StructArray get_job_status() override
        {
            std::unique_lock<std::mutex> lock_jobs(mutex_jobs);
            std::size_t n = jobs.size() + futureMap.size() + (job_in_progress ? 1 : 0);

            using StatusType = std::underlying_type<JobStatus>::type;

            auto jobID = factory.createArray<JobID>({n});
            auto status = factory.createArray<StatusType>({n});
            auto worker = factory.createArray<int>({n});

            std::size_t i = 0;
            for (const auto &j : jobs)
            {
                jobID[i] = j.id;
                status[i] = static_cast<StatusType>(JobStatus::Wait);
                worker[i] = 0;
                ++i;
            }
            if (job_in_progress)
            {
                jobID[i] = job_in_progress;
                status[i] = static_cast<StatusType>(JobStatus::AssignWorker);
                worker[i] = 0;
                ++i;
            }
            for (const auto &j : futureMap)
            {
                const auto &future = j.second.second;
                jobID[i] = j.first;
                if (future.valid())
                {
                    if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                        status[i] = static_cast<StatusType>(JobStatus::Done);
                    else
                        status[i] = static_cast<StatusType>(JobStatus::InProgress);
                }
                else
                {
                    status[i] = static_cast<StatusType>(JobStatus::Error);
                }
                worker[i] = j.second.first.get_workerID();
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
                if (it_job->id == jobID)
                {
                    jobs.erase(it_job);
                    return;
                }
            }

            // next assigned job
            if (jobID == job_in_progress)
            {
                job_in_progress = 0;
                return;
            }

            // finished jobs or in progress
            decltype(futureMap)::iterator it_future;
            if ((it_future = futureMap.find(jobID)) != futureMap.end())
            {
                futureMap[jobID].second.cancel();
                futureMap.erase(it_future);
                return;
            }

            throw JobNotExists(jobID);
        }

    private:
        bool exists(JobID id) noexcept
        {
            std::unique_lock<std::mutex> lock_jobs(mutex_jobs);

            if (id == job_in_progress)
                return true;

            for (const auto &e : jobs)
                if (e.id == id)
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

        std::deque<Job_feval> jobs;                              // mutex_jobs
        std::map<JobID, std::pair<Job_feval, Future>> futureMap; // mutex_jobs
        std::condition_variable cv_queue;
        std::condition_variable cv_worker;
        std::condition_variable cv_future;
        std::mutex mutex_jobs;
        std::mutex mutex_worker;

        matlab::data::ArrayFactory factory;
    };

} // namespace MatlabPool

#endif