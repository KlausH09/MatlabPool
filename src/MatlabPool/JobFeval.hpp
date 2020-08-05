#ifndef MATLABPOOL_JOB_HPP
#define MATLABPOOL_JOB_HPP

#include <string>
#include <vector>

#include "MatlabPool/JobBase.hpp"

namespace MatlabPool
{
    // this class describes a single job processed by
    // a single worker. Objects of this class can store
    // results of the jobs ("nlhs" number of return values,
    // "result" vector of matlab data types).
    // the "nlhs" value is necessary for the function
    // call or rather the job execution, because matlab
    // function can be depend on the number of output
    // arguments, e.g.
    //      [e] = eig(A);     % nlhs = 1
    //      [V,D] = eig(A);   % nlhs = 2
    // with
    //      e : eigenvalues (vector)
    //      V : eigenvectors (matrix)
    //      D : eigenvalues (diagonal matrix) 
    class JobFeval : public JobBase
    {
    public:
        enum class Status : uint8_t
        {
            Wait,
            InProgress,
            Done,
            Error,
            Canceled,
            DoneEmpty,
            Empty,
        };
        class JobFevalException : public Exception
        {
        };
        class NoResults : public JobFevalException
        {
        public:
            const char *what() const noexcept;
            const char *identifier() const noexcept;
        };

    public:
        JobFeval(const JobFeval &) = delete;
        JobFeval &operator=(const JobFeval &) = delete;

        JobFeval() noexcept;

        JobFeval(std::u16string cmd, std::size_t nlhs,
            std::vector<matlab::data::Array> &&args);

        JobFeval(JobFeval &&other) noexcept;

        JobFeval &operator=(JobFeval &&other) noexcept;

        friend void swap(JobFeval &j1, JobFeval &j2) noexcept;

        std::size_t get_nlhs() const noexcept;

        std::vector<matlab::data::Array> &get_args() noexcept;

        void set_workerID(std::size_t val) noexcept;

        int get_workerID() const noexcept;

        Status get_status() const noexcept;

        // return a reference to the results of the job
        const std::vector<matlab::data::Array> &peek_result() const;

        // pop the result of the job. This will set the
        // status to "DoneEmpty", so you can call this 
        // function only once
        std::vector<matlab::data::Array> pop_result();

        // store the members of this object in a matlab struct
        matlab::data::StructArray toStruct();

    protected:
        Status status;

        // number of return values
        std::size_t nlhs;

        // function/job arguments
        std::vector<matlab::data::Array> args;

        // result
        std::vector<matlab::data::Array> result;

    private:
        int workerID;
    };

} // namespace MatlabPool

#endif