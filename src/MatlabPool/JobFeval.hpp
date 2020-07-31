#ifndef MATLABPOOL_JOB_HPP
#define MATLABPOOL_JOB_HPP

#include <string>
#include <vector>

#include "MatlabPool/JobBase.hpp"

namespace MatlabPool
{
    class JobFeval : public JobBase
    {
        JobFeval(const JobFeval &) = delete;
        JobFeval &operator=(const JobFeval &) = delete;

    public:
        enum class Status : uint8_t
        {
            Wait,
            AssignToWorker,
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
        JobFeval() noexcept;
        JobFeval(std::u16string cmd, std::size_t nlhs, std::vector<matlab::data::Array> &&args);
        JobFeval(JobFeval &&other) noexcept;

        JobFeval &operator=(JobFeval &&other) noexcept;

        friend void swap(JobFeval &j1, JobFeval &j2) noexcept;

        void set_AssignToWorker_status();

        std::size_t get_nlhs() const noexcept;

        std::vector<matlab::data::Array> &get_args() noexcept;

        void set_workerID(int val) noexcept;

        int get_workerID() const noexcept;

        Status get_status() const noexcept;

        const std::vector<matlab::data::Array> &peek_result() const;

        std::vector<matlab::data::Array> pop_result();

        matlab::data::StructArray toStruct();

    protected:
        Status status;

        std::size_t nlhs;
        std::vector<matlab::data::Array> args;
        std::vector<matlab::data::Array> result;

    private:
        int workerID;
    };

} // namespace MatlabPool

#endif