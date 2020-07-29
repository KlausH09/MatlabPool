#include "MatlabPool/JobFeval.hpp"

#include "MatlabPool/Assert.hpp"

namespace MatlabPool
{

    JobFeval::JobFeval() noexcept : JobBase(), status(Status::Empty), workerID(-1){};

    JobFeval::JobFeval(std::u16string cmd, std::size_t nlhs, std::vector<matlab::data::Array> &&args)
        : JobBase(cmd),
          status(Status::Wait),
          nlhs(nlhs),
          args(std::move(args)),
          workerID(-1)
    {
    }

    JobFeval::JobFeval(JobFeval &&other) noexcept : JobFeval()
    {
        using std::swap;
        swap(*this, other);
    }

    JobFeval &JobFeval::operator=(JobFeval &&other) noexcept
    {
        using std::swap;
        swap(*this, other);
        return *this;
    }

    void swap(JobFeval &j1, JobFeval &j2) noexcept
    {
        using std::swap;
        swap(static_cast<JobBase &>(j1), static_cast<JobBase &>(j2));
        swap(j1.status, j2.status);
        swap(j1.nlhs, j2.nlhs);
        swap(j1.args, j2.args);
        swap(j1.result, j2.result);
        swap(j1.workerID, j2.workerID);
    }

    void JobFeval::set_AssignToWorker_status()
    {
        MATLABPOOL_ASSERT(status == Status::Wait);
        status = Status::AssignToWorker;
    }

    std::size_t JobFeval::get_nlhs() const noexcept
    {
        return nlhs;
    }

    std::vector<matlab::data::Array> &JobFeval::get_args() noexcept
    {
        return args;
    }

    void JobFeval::set_workerID(int val) noexcept
    {
        if (val >= 0)
        {
            MATLABPOOL_ASSERT(status == Status::AssignToWorker);
            status = Status::InProgress;
        }
        workerID = val;
    }

    int JobFeval::get_workerID() const noexcept
    {
        return workerID;
    }

    JobFeval::Status JobFeval::get_status() const noexcept
    {
        return status;
    }

    const std::vector<matlab::data::Array> &JobFeval::peek_result() const
    {
        if (status == Status::Error)
            throw ExecutionError(id, errorBuf.get());
        MATLABPOOL_ASSERT(status == Status::Done);

        return result;
    }

    std::vector<matlab::data::Array> JobFeval::pop_result()
    {
        if (status == Status::Error)
            throw ExecutionError(id, errorBuf.get());
        MATLABPOOL_ASSERT(status == Status::Done);

        status = Status::DoneEmpty;
        return std::move(result);
    }

} // namespace MatlabPool
