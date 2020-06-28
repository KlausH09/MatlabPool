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

#include "./Pool.hpp"
#include "./Job.hpp"
#include "./EngineHack.hpp"

namespace MatlabPool
{
    class PoolImpl : public Pool
    {
        PoolImpl(const PoolImpl &) = delete;
        PoolImpl &operator=(const PoolImpl &) = delete;

    public:
        PoolImpl(std::size_t n, const std::vector<std::u16string> &options)
            : n(n),
              worker_ready(n),
              stop(false),
              engine(static_cast<EngineHack *>(operator new(sizeof(EngineHack) * n)))
        {
            for (std::size_t i = 0; i < n; i++)
            {
                new (engine + i) EngineHack(options);
                worker_ready[i] = true;
            }

            master = std::thread([=]() {
                for (;;)
                {

                    Job job;

                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        while (!stop && jobs.empty())
                            cv_queue.wait(lock);

                        if (stop)
                            break;

                        job = std::move(jobs.front());
                        jobs.pop_front();
                    }

                    std::size_t workerID = get_free_worker();

                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        JobID job_id = job.id;

                        auto futureResult = engine[workerID].eval_job(job, [=]() {
                            std::unique_lock<std::mutex> lock_worker(mutex_worker);
                            worker_ready[workerID] = true;
                            cv_worker.notify_one();
                        });
                        futureMap[job_id] = std::pair<Job, Future>(std::move(job), std::move(futureResult));

                        cv_future.notify_one();
                    }
                }
            });
        }
        ~PoolImpl() override
        {
            // join master thread
            {
                std::unique_lock<std::mutex> lock(mutex);
                stop = true;
                cv_queue.notify_one();
            }
            master.join();

            // cancel jobs
            {
                std::unique_lock<std::mutex> lock(mutex);
                for (auto &e : futureMap)
                    e.second.second.cancel(true);
            }

            // close Matlab engines
            for (std::size_t i = 0; i < n; i++)
                engine[i].~EngineHack();

            operator delete(engine);
        }

        JobID submit(Job &&job) override
        {
            JobID job_id = job.id;
            std::unique_lock<std::mutex> lock(mutex);
            jobs.push_back(std::move(job));
            cv_queue.notify_one();
            return job_id;
        }

        bool exists(JobID id) noexcept override
        {
            std::unique_lock<std::mutex> lock(mutex);

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

            std::unique_lock<std::mutex> lock(mutex);

            decltype(futureMap)::iterator it;
            while ((it = futureMap.find(id)) == futureMap.end())
            {
                cv_future.wait(lock);
            }

            auto tmp = std::move(futureMap[id]);
            futureMap.erase(it);

            tmp.first.result = tmp.second.get();
            return std::move(tmp.first);
        }

    private:
        std::size_t get_free_worker() noexcept
        {
            std::unique_lock<std::mutex> lock(mutex_worker);
            for (;;)
            {
                for (std::size_t i = 0; i < n; i++)
                    if (worker_ready[i])
                        return i;

                cv_worker.wait(lock);
            }
        }

    private:
        std::size_t n;
        std::vector<bool> worker_ready;
        bool stop;
        EngineHack *engine;

        std::thread master;

        std::deque<Job> jobs;
        std::map<JobID, std::pair<Job, Future>> futureMap;
        std::condition_variable cv_queue;
        std::condition_variable cv_worker;
        std::condition_variable cv_future;
        std::mutex mutex;
        std::mutex mutex_worker;
    };

} // namespace MatlabPool

#endif