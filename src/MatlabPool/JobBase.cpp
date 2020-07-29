#include "MatlabPool/JobBase.hpp"

namespace MatlabPool
{

    JobBase::ExecutionError::ExecutionError(std::shared_ptr<StreamBuf> buffer)
    {
        std::ostringstream os;
        os << "an error has occurred during execution";
        if (buffer)
        {
            os << '\n'
               << convertUTF16StringToASCIIString(buffer->str());
        }
        msg = os.str();
    }

    JobBase::ExecutionError::ExecutionError(JobID id, std::shared_ptr<StreamBuf> buffer)
    {
        std::ostringstream os;
        os << "an error has occurred during job execution (id: " << id << ")";
        if (buffer)
        {
            os << '\n'
               << convertUTF16StringToASCIIString(buffer->str());
        }
        msg = os.str();
    }

    const char *JobBase::ExecutionError::what() const noexcept
    {
        return msg.c_str();
    }

    const char *JobBase::ExecutionError::identifier() const noexcept
    {
        return "JobExecutionError";
    }

    JobBase::JobBase() noexcept : id(0){};
    JobBase::JobBase(std::u16string cmd) : id(id_count++),
                                           cmd(std::move(cmd))
    {
    }

    JobBase::JobBase(JobBase &&other) noexcept : JobBase()
    {
        using std::swap;
        swap(*this, other);
    }

    JobBase &JobBase::operator=(JobBase &&other) noexcept
    {
        using std::swap;
        swap(*this, other);
        return *this;
    }

    void swap(JobBase &j1, JobBase &j2) noexcept
    {
        using std::swap;
        swap(j1.id, j2.id);
        swap(j1.cmd, j2.cmd);

        swap(j1.outputBuf, j2.outputBuf);
        swap(j1.errorBuf, j2.errorBuf);
    }

    JobID JobBase::get_ID() const noexcept
    {
        return id;
    }

    const std::u16string &JobBase::get_cmd() const noexcept
    {
        return cmd;
    }

    OutputBuf &JobBase::get_outBuf() noexcept
    {
        return outputBuf;
    }

    ErrorBuf &JobBase::get_errBuf() noexcept
    {
        return errorBuf;
    }

} // namespace MatlabPool
