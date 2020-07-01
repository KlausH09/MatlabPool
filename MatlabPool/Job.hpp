#ifndef MATLABPOOL_JOB_HPP
#define MATLABPOOL_JOB_HPP

#include <string>
#include <vector>
#include <functional>

#include "./StreamBuf.hpp"

#include "MatlabDataArray.hpp"

//#define MATLABPOOL_DISP_WORKER_OUTPUT
//#define MATLABPOOL_DISP_WORKER_ERROR

namespace MatlabPool
{
#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
    constexpr const bool disp_output_buffer = true;
    using OutputBuf = RealStreamBuffer;
#else
    constexpr const bool disp_output_buffer = false;
    using OutputBuf = EmptyStreamBuffer;
#endif
#ifdef MATLABPOOL_DISP_WORKER_ERROR
    constexpr const bool disp_error_buffer = true;
    using ErrorBuf = RealStreamBuffer;
#else
    constexpr const bool disp_error_buffer = false;
    using ErrorBuf = EmptyStreamBuffer;
#endif
    using JobID = std::size_t;

    class Job
    {
        using SBuf = std::basic_stringbuf<char16_t>;

        Job(const Job &) = delete;
        Job &operator=(const Job &) = delete;

    public:
        Job() noexcept : id(0) {}

        Job(std::u16string function)
            : id(id_count++),
              function(std::move(function))
        {}

        Job(Job &&other) noexcept : Job()
        {
            swap(*this, other);
        }

        Job &operator=(Job &&other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        void swap(Job &j1, Job &j2) noexcept
        {
            std::swap(j1.id, j2.id);
            std::swap(j1.function, j2.function);

            std::swap(j1.outputBuf, j2.outputBuf);
            std::swap(j1.errorBuf, j2.errorBuf);
        }

    public: // TODO
        inline static JobID id_count = 1;
        JobID id;
        std::u16string function;

        OutputBuf outputBuf;
        ErrorBuf errorBuf;
    };

    class Job_feval : public Job
    {
    public:
        Job_feval() : Job(){};
        Job_feval(std::u16string function, std::size_t nlhs, std::vector<matlab::data::Array> &&args) : Job(std::move(function)),
                                                                                                        nlhs(nlhs),
                                                                                                        args(std::move(args)),
                                                                                                        workerID(0)
        {
        }

        Job_feval(Job_feval &&other) noexcept : Job_feval()
        {
            swap(*this, other);
        }

        Job_feval &operator=(Job_feval &&other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        void swap(Job_feval &j1, Job_feval &j2) noexcept
        {
            Job::swap(j1,j2);
            std::swap(j1.nlhs, j2.nlhs);
            std::swap(j1.args, j2.args);
            std::swap(j1.result, j2.result);
            std::swap(j1.workerID, j2.workerID);
        }

        void set_workerID(std::size_t val) noexcept
        {
            workerID = val + 1;
        }
        ssize_t get_workerID() const noexcept
        {
            return ((ssize_t)workerID) - 1;
        }

    public:
        std::size_t nlhs;
        std::vector<matlab::data::Array> args;
        std::vector<matlab::data::Array> result;

    private:
        std::size_t workerID;
    };

} // namespace MatlabPool

#endif