#ifndef MATLABPOOL_JOBEVAL_HPP
#define MATLABPOOL_JOBEVAL_HPP

#include "./JobBase.hpp"

namespace MatlabPool
{

    class JobEval : public JobBase
    {
    public:
        enum class Status : uint8_t
        {
            Empty,
            NoError,
            Error,
        };

    public:
        JobEval() : JobBase(), status(Status::Empty) {}
        JobEval(std::u16string cmd) : JobBase(cmd),  status(Status::NoError) {}

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
            swap(j1.status,j2.status);
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

        Status get_status() const noexcept
        {
            return status;
        }

        private:
            Status status;
    };

} // namespace MatlabPool

#endif