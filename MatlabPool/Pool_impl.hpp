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
        PoolImpl(std::size_t n, const std::vector<std::u16string> &options)
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

                    Job job = std::move(jobs.front());
                    jobs.pop_front();
                    job_in_progress = job.id;

                    lock_jobs.unlock(); // do not block adding new jobs

                    MatlabPool::Future futureResult;
                    {
                        std::unique_lock<std::mutex> lock_worker(mutex_worker);
                        std::size_t workerID = get_free_worker(lock_worker);

                        futureResult = engine[workerID].get()->eval_job(job, [=]() {
                            std::unique_lock<std::mutex> lock_worker(mutex_worker);
                            worker_ready[workerID] = true;
                            cv_worker.notify_one();
                        });
                    }
                    lock_jobs.lock();

                    futureMap[job_in_progress] = std::pair<Job, Future>(std::move(job), std::move(futureResult));
                    job_in_progress = 0;
                    cv_future.notify_one();
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

        void resize(std::size_t n_new, const std::vector<std::u16string> &options) override
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

        JobID submit(Job &&job) override
        {
            JobID job_id = job.id;
            std::unique_lock<std::mutex> lock_jobs(mutex_jobs);
            jobs.push_back(std::move(job));
            cv_queue.notify_one();
            return job_id;
        }

        bool exists(JobID id) noexcept override
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

        Job wait(JobID id) override
        {
            if (!exists(id))
                throw JobNotExists(id);

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

    private:
        std::size_t get_free_worker(std::unique_lock<std::mutex> &lock) noexcept
        {
            for (;;)
            {
                for (std::size_t i = 0; i < worker_ready.size(); i++)
                    if (worker_ready[i])
                        return i;

                cv_worker.wait(lock);
            }
        }

    private:
        bool stop;                      // mutex_jobs
        std::vector<bool> worker_ready; // mutex_worker
        std::vector<EnginePtr> engine;  // mutex_worker
        JobID job_in_progress;          // mutex_jobs

        std::thread master;

        std::deque<Job> jobs;                              // mutex_jobs
        std::map<JobID, std::pair<Job, Future>> futureMap; // mutex_jobs
        std::condition_variable cv_queue;
        std::condition_variable cv_worker;
        std::condition_variable cv_future;
        std::mutex mutex_jobs;
        std::mutex mutex_worker;
    };

} // namespace MatlabPool

#endif