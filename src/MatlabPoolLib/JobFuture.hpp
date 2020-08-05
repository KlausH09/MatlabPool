#ifndef MATLABPOOL_JOBFUTURE_HPP
#define MATLABPOOL_JOBFUTURE_HPP

#include "MatlabPool/JobFeval.hpp"

#include "MatlabEngine.hpp"

namespace MatlabPool
{

    // This class complets the "JobFeval" class. The
    // additional functions and members depends on the
    // Matlab Engine API, thats why this class is 
    // outsourced in the MatlabPool library
    class JobFuture : public JobFeval
    {
        using Result = std::vector<matlab::data::Array>;
        using Future = matlab::engine::FutureResult<Result>;

    public:
        JobFuture(const JobFuture &other) = delete;
        JobFuture &operator=(const JobFuture &other) = delete;

        JobFuture() noexcept;
        JobFuture(JobFeval &&job) noexcept;
        JobFuture(JobFuture &&other) noexcept;
        JobFuture &operator=(JobFuture &&other) noexcept;
        ~JobFuture();

        friend void swap(JobFuture &j1, JobFuture &j2) noexcept;

        // set results of the jobs
        void set_future(Future &&val) noexcept;

        // wait until the job is done
        void wait() noexcept;

        // cancel the job
        void cancel() noexcept;

        Status get_status() const noexcept;

    private:
        Future future;
    };

} // namespace MatlabPool

#endif