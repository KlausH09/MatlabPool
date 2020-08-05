#include "MatlabPool/JobFeval.hpp"

#include "MatlabPool/Assert.hpp"

namespace MatlabPool
{

    const char *JobFeval::NoResults::what() const noexcept
    {
        return "there are no results left, maybe you already invoke them";
    }
    const char *JobFeval::NoResults::identifier() const noexcept
    {
        return "JobFevalNoResults";
    }

    JobFeval::JobFeval() noexcept
        : JobBase(),
        status(Status::Empty),
        workerID(-1) {}

    JobFeval::JobFeval(std::u16string cmd, std::size_t nlhs,
        std::vector<matlab::data::Array> &&args)
        : JobBase(cmd),
        status(Status::Wait),
        nlhs(nlhs),
        args(std::move(args)),
        workerID(-1) {}

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

    std::size_t JobFeval::get_nlhs() const noexcept
    {
        return nlhs;
    }

    std::vector<matlab::data::Array> &JobFeval::get_args() noexcept
    {
        return args;
    }

    void JobFeval::set_workerID(std::size_t val) noexcept
    {
        status = Status::InProgress;
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
        if (status != Status::Done)
            throw NoResults();

        return result;
    }

    std::vector<matlab::data::Array> JobFeval::pop_result()
    {
        if (status == Status::Error)
            throw ExecutionError(id, errorBuf.get());
        if (status != Status::Done)
            throw NoResults();

        status = Status::DoneEmpty;
        return std::move(result);
    }

    matlab::data::StructArray JobFeval::toStruct()
    {
        auto st = Utilities::addFields(JobBase::toStruct(), { "result" });

        if (status == Status::Done)
        {
            if (result.size() == 0)
            {
            }
            else if (result.size() == 1)
                st[0]["result"] = std::move(result[0]);
            else
            {
                auto tmp = factory.createCellArray({ 1, result.size() });
                std::move(result.begin(), result.end(), tmp.begin());
                st[0]["result"] = std::move(tmp);
            }
            status = Status::DoneEmpty;
        }
        return st;
    }

} // namespace MatlabPool
