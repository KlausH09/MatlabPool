#ifndef MATLABPOOL_JOBEVAL_HPP
#define MATLABPOOL_JOBEVAL_HPP

#include "MatlabPool/JobBase.hpp"

namespace MatlabPool
{

    class JobEval : public JobBase
    {
    public:
        JobEval(const JobEval &) = delete;
        JobEval &operator=(const JobEval &) = delete;

        JobEval() noexcept;
        JobEval(std::u16string cmd);
        JobEval(JobEval &&other) noexcept;

        JobEval &operator=(JobEval &&other) noexcept;

        friend void swap(JobEval &j1, JobEval &j2) noexcept;

        void add_error(StreamBuf &buf, std::size_t workerID);
        void add_output(StreamBuf &buf, std::size_t workerID);
    };

} // namespace MatlabPool

#endif