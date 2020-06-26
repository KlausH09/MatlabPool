#ifndef MATLABPOOL_POOL_HPP
#define MATLABPOOL_POOL_HPP

#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <thread>
#include <map>
#include <queue>

#include "./Job.hpp"
#include "./EngineHack.hpp"
#include "./Definitions.hpp"

namespace MatlabPool
{
    class Pool
    {
        Pool(const Pool &) = delete;
        Pool &operator=(const Pool &) = delete;

    public:
        Pool(std::size_t n, const std::vector<std::u16string> &options) : n(n),
                                                                          engine(static_cast<EngineHack *>(operator new(sizeof(EngineHack) * n))),
                                                                          stop(false),
                                                                          worker_ready(new bool[n])
        {
            for (std::size_t i = 0; i < n; i++)
            {
                new (engine + i) EngineHack(options);
                worker_ready[i] = true;
            }

            master = std::thread([=]() {
                auto get_free_worker = [=]() {
                    std::size_t i;
                    for (i = 0; i < n; i++)
                        if (worker_ready[i])
                            break;
                    return i;
                };

                for (;;)
                {
                    std::size_t workerID;
                    Job job;

                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        while (!stop && jobs.empty())
                            cv_queue.wait(lock);

                        if (stop)
                            break;

                        job = std::move(jobs.front());
                        jobs.pop();
                    }

                    {
                        std::unique_lock<std::mutex> lock_worker(mutex_worker);

                        while ((workerID = get_free_worker()) >= n)
                            cv_worker.wait(lock_worker);
                    }

                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        JobID job_id = job.id;

                        auto futureResult = engine[workerID].eval_job(job, [=]() {
                            std::unique_lock<std::mutex> lock_worker(mutex_worker);
                            worker_ready[workerID] = true;
                            cv_worker.notify_one();
                        });
                        futureMap[job_id] = std::pair<Job, Future>(std::move(job),std::move(futureResult));

                        cv_future.notify_one();
                    }
                }
            });
        }
        ~Pool()
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
            delete[] worker_ready;
        }

        JobID submit(Job &&job)
        {
            JobID job_id = job.id;
            std::unique_lock<std::mutex> lock(mutex);
            jobs.push(std::move(job));
            cv_queue.notify_one();
            return job_id;
        }

        Job wait(JobID job_id)
        {
            std::unique_lock<std::mutex> lock(mutex);

            decltype(futureMap)::iterator it;
            while ((it = futureMap.find(job_id)) == futureMap.end())
            {
                cv_future.wait(lock);
            }

            auto tmp = std::move(futureMap[job_id]);
            futureMap.erase(it);

            tmp.first.result = tmp.second.get();
            return std::move(tmp.first);
        }

    private:
        std::size_t n;
        EngineHack *engine;
        bool stop;
        bool *worker_ready;

        std::thread master;

        std::queue<Job> jobs;
        std::map<JobID, std::pair<Job, Future>> futureMap;
        std::condition_variable cv_queue;
        std::condition_variable cv_worker;
        std::condition_variable cv_future;
        std::mutex mutex;
        std::mutex mutex_worker;
    };

} // namespace MatlabPool

#endif