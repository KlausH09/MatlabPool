#include "MatlabPool/JobEval.hpp"

namespace MatlabPool
{

    JobEval::JobEval() noexcept : JobBase() {}
    JobEval::JobEval(std::u16string cmd) : JobBase(cmd) {}

    JobEval::JobEval(JobEval &&other) noexcept : JobEval()
    {
        using std::swap;
        swap(*this, other);
    }

    JobEval &JobEval::operator=(JobEval &&other) noexcept
    {
        using std::swap;
        swap(*this, other);
        return *this;
    }

    void swap(JobEval &j1, JobEval &j2) noexcept
    {
        using std::swap;
        swap(static_cast<JobBase &>(j1), static_cast<JobBase &>(j2));
    }

    void JobEval::add_error(StreamBuf &buf, std::size_t workerID)
    {
        if (!buf.empty())
        {
            errorBuf << u"============= Error: Worker " << workerID + 1 << u", Job: " << id << u" ===============\n";
            errorBuf << buf.str();
        }
        else
        {
            errorBuf << u"Error: Worker " << workerID + 1 << u", Job: " << id << u"\n";
        }
    }

    void JobEval::add_output(StreamBuf &buf, std::size_t workerID)
    {
        if (!buf.empty())
        {
            outputBuf << u"============= Output Worker " << workerID + 1 << u", Job: " << id << u" ===============\n";
            outputBuf << buf.str();
        }
    }

} // namespace MatlabPool
