#include "MatlabPoolLib/JobFuture.hpp"

#include "MatlabPool/Assert.hpp"

namespace MatlabPool
{
    JobFuture::JobFuture() noexcept : JobFeval() {}

    JobFuture::JobFuture(JobFeval &&job) noexcept : JobFuture()
    {
        using std::swap;
        std::swap(static_cast<JobFeval &>(*this), job);
    }

    JobFuture::JobFuture(JobFuture &&other) noexcept : JobFuture()
    {
        using std::swap;
        swap(*this, other);
    }

    JobFuture &JobFuture::operator=(JobFuture &&other) noexcept
    {
        using std::swap;
        swap(*this, other);
        return *this;
    }

    JobFuture::~JobFuture()
    {
        cancel();
    }

    void swap(JobFuture &j1, JobFuture &j2) noexcept
    {
        using std::swap;
        swap(static_cast<JobFeval &>(j1), static_cast<JobFeval &>(j2));
        swap(j1.future, j2.future);
    }

    void JobFuture::set_future(Future &&val) noexcept
    {
        future = std::move(val);
    }

    void JobFuture::wait() noexcept
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
        catch (...)
        {
            MATLABPOOL_ERROR("unexpect exception");
        }
    }

    void JobFuture::cancel() noexcept
    {
        if (status != Status::Canceled && future.valid())
        {
            future.cancel();
            status = Status::Canceled;
        }
    }

    JobFuture::Status JobFuture::get_status() const noexcept
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

} // namespace MatlabPool
