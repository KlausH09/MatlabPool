#ifndef MATLABPOOL_POOL_HPP
#define MATLABPOOL_POOL_HPP

#include <exception>
#include <future>
#include <queue>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "MatlabEngine.hpp"
#include "MatlabDataArray.hpp"

#include "./Job.hpp"

namespace MatlabPool
{

    class Pool
    {
        using Result = std::vector<matlab::data::Array>;

        const std::vector<matlab::engine::String> start_options = {u"-nosplash",u"nojvm",u"-singleCompThread"};


    public:
        Pool(std::size_t n = std::thread::hardware_concurrency()) : stop(false), running(0)
        {
            threads.resize(n);

            for (auto &t : threads)
            {
                t = std::thread([this]() {
                    
                    std::unique_ptr<matlab::engine::MATLABEngine> matlabPtr = matlab::engine::startMATLAB(start_options);
                    Job job;

                    for (;;)
                    {
                        {
                            std::unique_lock<std::mutex> lock(mutex);
                            while (jobs.empty() && !stop)
                            {
                                cv.wait(lock);
                            }
                            if (stop)
                                break;
                            job = std::move(jobs.front());
                            jobs.pop();
                            running++;
                        }

                        Result result = matlabPtr->feval(std::move(job.function), job.nlhs, std::move(job.args));

                        {
                            std::unique_lock<std::mutex> lock(mutex);
                            running--;
                            results[job.id] = std::move(result);
                            cv_finished.notify_one();
                        }
                    }
                });
            }
        }
        ~Pool()
        {
            {
                std::unique_lock<std::mutex> lock(mutex);
                stop = true;
                cv.notify_all();
            }
            for (auto &t : threads)
                t.join();
        }
        std::size_t submit(Job &&job)
        {
            std::size_t job_id = job.id;
            std::unique_lock<std::mutex> lock(mutex);
            jobs.push(std::move(job));
            cv.notify_one();
            return job_id;
        }
        void wait()
        {
            std::unique_lock<std::mutex> lock(mutex);
            while (!jobs.empty() || running)
                cv_finished.wait(lock);
        }

        Result wait(std::size_t job_id)
        {
            std::unique_lock<std::mutex> lock(mutex);

            std::map<std::size_t, Result>::iterator it;
            while((it = results.find(job_id)) == results.end())
            {
                cv_finished.wait(lock);
            }

            Result tmp = std::move(results[job_id]);
            results.erase(it);
            return tmp;
        }

    private:
        bool stop;
        std::size_t running;
        std::queue<Job> jobs;
        std::map<std::size_t, Result> results;
        std::vector<std::thread> threads;
        std::condition_variable cv;
        std::condition_variable cv_finished;
        std::mutex mutex;
    };

} // namespace MatlabPool

#endif