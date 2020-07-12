#ifndef MATLABPOOL_JOBFUTURE_HPP
#define MATLABPOOL_JOBFUTURE_HPP

#include <future>
#include <chrono>

#include "./Job.hpp"

#include "assert.hpp"

#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"

namespace MatlabPool
{

    class JobFuture : public Job
    {
        using Result = std::vector<matlab::data::Array>;
        using Future = matlab::engine::FutureResult<Result>;

    public:
        JobFuture() : Job() {}
        JobFuture(Job &&job) noexcept : Job()
        {
            using std::swap;
            std::swap(static_cast<Job &>(*this), job);
        }
        JobFuture(JobFuture &&other) noexcept : JobFuture()
        {
            using std::swap;
            swap(*this, other);
        }
        JobFuture &operator=(JobFuture &&other) noexcept
        {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        friend void swap(JobFuture &j1, JobFuture &j2) noexcept
        {
            using std::swap;
            swap(static_cast<Job &>(j1), static_cast<Job &>(j2));
            swap(j1.future, j2.future);
        }

        void set_future(Future &&val) noexcept
        {
            future = std::move(val);
        }
        void wait() noexcept
        {
            try
            {
                result = future.get();
                status = Status::Done;
            }
            catch (const matlab::engine::Exception &e)
            {
                status = Status::Error;
            }
            catch(...)
            {
                ERROR("unexpect exception");
            }
        }
        void cancel() noexcept
        {
            if (status == Status::AssignToWorker)
            {
                status = Status::Canceled;
            }
            else if(future.valid())
            {
                future.cancel();
                status = Status::Canceled;
            }
        }

        Status get_status() const noexcept
        {
            if (future.valid())
            {
                if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                    return Status::Done;
                else
                    return Status::InProgress;
            }
            else
            {
                return status;
            }
        }

    private:
        Future future;
    };

} // namespace MatlabPool

#endif