#ifndef MATLABPOOL_POOL_IMPL_HPP
#define MATLABPOOL_POOL_IMPL_HPP

#include <mutex>
#include <condition_variable>
#include <vector>
#include <thread>
#include <map>
#include <queue>

#include "MatlabPool/Pool.hpp"
#include "MatlabPoolLib/JobFuture.hpp"
#include "MatlabPoolLib/EngineHack.hpp"

namespace MatlabPool
{
    class PoolImpl : public Pool
    {
        using EnginePtr = std::unique_ptr<EngineHack>;

    public:
        PoolImpl(const PoolImpl &) = delete;
        PoolImpl &operator=(const PoolImpl &) = delete;

        PoolImpl(unsigned int n, const std::vector<std::u16string> &options);
        ~PoolImpl() override;

        void resize(unsigned int n_new, const std::vector<std::u16string> &options) override;

        std::size_t size() const override;

        JobID submit(JobFeval &&job) override;

        JobFeval wait(JobID id) override;

        void eval(JobEval &job) override;

        matlab::data::StructArray get_job_status() override;

        matlab::data::StructArray get_worker_status() override;

        void cancel(JobID jobID) override;

        void clear() override;

    private:
        bool exists(JobID id) noexcept;

        std::size_t get_free_worker(std::unique_lock<std::mutex> &lock) noexcept;

    private:
        bool stop;                      // mutex_jobs
        bool sleep;                     // mutex_jobs
        std::vector<bool> worker_ready; // mutex_worker
        std::vector<EnginePtr> engine;  // mutex_worker

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