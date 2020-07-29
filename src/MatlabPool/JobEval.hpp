#ifndef MATLABPOOL_JOBEVAL_HPP
#define MATLABPOOL_JOBEVAL_HPP

#include "MatlabPool/JobBase.hpp"

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
        JobEval() noexcept;
        JobEval(std::u16string cmd);
        JobEval(JobEval &&other) noexcept;

        JobEval &operator=(JobEval &&other) noexcept;

        friend void swap(JobEval &j1, JobEval &j2) noexcept;

        void add_error(ErrorBuf &buf, std::size_t workerID);
        void add_output(OutputBuf &buf, std::size_t workerID);

        Status get_status() const noexcept;

    private:
        Status status;
    };

} // namespace MatlabPool

#endif