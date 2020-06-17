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

                    job.notifier = [=]() {
                        std::unique_lock<std::mutex> lock_worker(mutex_worker);
                        worker_ready[workerID] = true;
                        cv_worker.notify_one();
                    };

                    {
                        std::unique_lock<std::mutex> lock(mutex);
                        std::size_t job_id = job.id;
                        futures[job_id] = std::move(engine[workerID].eval_job(std::move(job)));
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
                for (auto &e : futures)
                    e.second.cancel(true);
            }

            // close Matlab engines
            for (std::size_t i = 0; i < n; i++)
                engine[i].~EngineHack();

            operator delete(engine);
            delete[] worker_ready;
        }

        std::size_t submit(Job &&job)
        {
            std::size_t job_id = job.id;
            std::unique_lock<std::mutex> lock(mutex);
            jobs.push(std::move(job));
            cv_queue.notify_one();
            return job_id;
        }

        void wait()
        {
            std::unique_lock<std::mutex> lock(mutex);
            while (!jobs.empty())
                cv_future.wait(lock);
        }

        std::vector<matlab::data::Array> wait(std::size_t job_id)
        {
            std::unique_lock<std::mutex> lock(mutex);

            std::map<std::size_t, Future>::iterator it;
            while ((it = futures.find(job_id)) == futures.end())
            {
                cv_future.wait(lock);
            }

            Future tmp = std::move(futures[job_id]);
            futures.erase(it);
            return tmp.get();
        }

    private:
        std::size_t n;
        EngineHack *engine;
        bool stop;
        bool *worker_ready;

        std::thread master;

        std::queue<Job> jobs;
        std::map<std::size_t, Future> futures;
        std::condition_variable cv_queue;
        std::condition_variable cv_worker;
        std::condition_variable cv_future;
        std::mutex mutex;
        std::mutex mutex_worker;
    };

} // namespace MatlabPool

#endif