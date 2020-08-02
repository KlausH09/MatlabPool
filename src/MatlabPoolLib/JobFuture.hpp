#ifndef MATLABPOOL_JOBFUTURE_HPP
#define MATLABPOOL_JOBFUTURE_HPP

#include "MatlabPool/JobFeval.hpp"

#include "MatlabEngine.hpp"

namespace MatlabPool
{

    class JobFuture : public JobFeval
    {
        using Result = std::vector<matlab::data::Array>;
        using Future = matlab::engine::FutureResult<Result>;

    public:
        JobFuture() noexcept;
        JobFuture(JobFeval &&job) noexcept;
        JobFuture(JobFuture &&other) noexcept;
        JobFuture &operator=(JobFuture &&other) noexcept;
        ~JobFuture();

        friend void swap(JobFuture &j1, JobFuture &j2) noexcept;

        void set_future(Future &&val) noexcept;
        void wait() noexcept;
        void cancel() noexcept;

        Status get_status() const noexcept;

    private:
        Future future;
    };

} // namespace MatlabPool

#endif