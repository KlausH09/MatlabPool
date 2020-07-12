#ifndef MATLABPOOL_JOBEVAL_HPP
#define MATLABPOOL_JOBEVAL_HPP

#include "./JobBase.hpp"

namespace MatlabPool
{

    class JobEval : public JobBase
    {

    public:
        JobEval() : JobBase() {}
        JobEval(std::u16string cmd) : JobBase(cmd, Status::Wait) {}

        JobEval(JobEval &&other) noexcept : JobEval()
        {
            using std::swap;
            swap(*this, other);
        }
        JobEval &operator=(JobEval &&other) noexcept
        {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        friend void swap(JobEval &j1, JobEval &j2) noexcept
        {
            using std::swap;
            swap(static_cast<JobBase &>(j1), static_cast<JobBase &>(j2));
        }

        void add_error(ErrorBuf &buf, std::size_t workerID)
        {
            status = Status::Error;
            if (!buf.empty())
            {
                errorBuf << u"============= Error: Worker " << workerID + 1 << u", Job: " << id << u" ===============\n";
                errorBuf << buf.str();
            }
            else
            {
                errorBuf << u"Error: Worker " << workerID + 1 << u", Job: " << id <<  u"\n";
            }
            
        }
        void add_output(OutputBuf &buf, std::size_t workerID)
        {
            if (!buf.empty())
            {
                outputBuf << u"============= Output Worker " << workerID + 1 << u", Job: " << id << u" ===============\n";
                outputBuf << buf.str();
            }
        }
    };

} // namespace MatlabPool

#endif