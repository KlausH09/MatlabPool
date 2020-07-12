#ifndef MATLABPOOL_JOB_FUTURE_HPP
#define MATLABPOOL_JOB_FUTURE_HPP

#include <future>
#include <chrono>
#include "./Job.hpp"

#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"

namespace MatlabPool
{

    class Job_future : public Job
    {
        using Result = std::vector<matlab::data::Array>;
        using Future = matlab::engine::FutureResult<Result>;

    public:
        Job_future() : Job() {}
        Job_future(Job &&job) noexcept : Job()
        {
            using std::swap;
            std::swap(static_cast<Job &>(*this), job);
        }
        Job_future(Job_future &&other) noexcept : Job_future()
        {
            using std::swap;
            swap(*this, other);
        }
        Job &operator=(Job_future &&other) noexcept
        {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        friend void swap(Job_future &j1, Job_future &j2) noexcept
        {
            using std::swap;
            swap(static_cast<Job &>(j1), static_cast<Job &>(j2));
            swap(j1.future, j2.future);
        }

        void set_future(Future &&val) noexcept
        {
            future = std::move(val);
        }
        void get_futureResult()
        {
            result = future.get(); // TODO
        }
        bool cancel() noexcept
        {
            return future.cancel();
        }

        JobStatus get_JobStatus() const noexcept
        {
            if (future.valid())
            {
                if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    return JobStatus::Done;
                else
                    return JobStatus::InProgress;
            }
            return JobStatus::Error;
        }

    private:
        Future future;
    };

} // namespace MatlabPool

#endif