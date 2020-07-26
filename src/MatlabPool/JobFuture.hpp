#ifndef MATLABPOOL_JOBFUTURE_HPP
#define MATLABPOOL_JOBFUTURE_HPP

#include <future>
#include <chrono>

#include "MatlabPool/JobFeval.hpp"
#include "MatlabPool/Assert.hpp"

#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"

namespace MatlabPool
{

    class JobFuture : public JobFeval
    {
        using Result = std::vector<matlab::data::Array>;
        using Future = matlab::engine::FutureResult<Result>;

    public:
        JobFuture() : JobFeval() {}
        JobFuture(JobFeval &&job) noexcept : JobFeval()
        {
            using std::swap;
            std::swap(static_cast<JobFeval &>(*this), job);
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
        ~JobFuture()
        {
            cancel();
        }
        friend void swap(JobFuture &j1, JobFuture &j2) noexcept
        {
            using std::swap;
            swap(static_cast<JobFeval &>(j1), static_cast<JobFeval &>(j2));
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
                MATLABPOOL_ERROR("unexpect exception");
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