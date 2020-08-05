#ifndef MATLABPOOL_JOBEVAL_HPP
#define MATLABPOOL_JOBEVAL_HPP

#include "MatlabPool/JobBase.hpp"

namespace MatlabPool
{
    // this class can be use to execute a function on every 
    // Matlab instance. E.g. you can run the "eval" function
    // to execute the Matlab command "cd(...)" to set the 
    // current path on each worker.
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

        // store error messages
        void add_error(StreamBuf &buf, std::size_t workerID);

        // store output messages
        void add_output(StreamBuf &buf, std::size_t workerID);
    };

} // namespace MatlabPool

#endif