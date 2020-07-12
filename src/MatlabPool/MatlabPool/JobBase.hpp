#ifndef MATLABPOOL_JOBBASE_HPP
#define MATLABPOOL_JOBBASE_HPP

#include <string>
#include <vector>
#include <ostream>

#include "./StreamBuf.hpp"

#include "MatlabDataArray.hpp"

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
    using JobID = std::uint64_t;

    class JobBase
    {
        using SBuf = std::basic_stringbuf<char16_t>;

        JobBase(const JobBase &) = delete;
        JobBase &operator=(const JobBase &) = delete;

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

        class Exception : public std::exception
        {
        };
        class ExecutionError : public Exception
        {
        public:
            ExecutionError(JobID id)
            {
                std::ostringstream os;
                os << "an error has occurred during job execution (id: " << id << ")";
                msg = os.str();
            }
            const char *what() const noexcept override
            {
                return msg.c_str();
            }

        private:
            std::string msg;
        };
        

    protected:
        JobBase() noexcept : id(0), status(Status::Empty){};
        JobBase(std::u16string cmd, Status status) : id(id_count++),
                                                     cmd(std::move(cmd)),
                                                     status(status)
        {
        }

    public:
        JobBase(JobBase &&other) noexcept : JobBase()
        {
            using std::swap;
            swap(*this, other);
        }
        JobBase &operator=(JobBase &&other) noexcept
        {
            using std::swap;
            swap(*this, other);
            return *this;
        }
        friend void swap(JobBase &j1, JobBase &j2) noexcept
        {
            std::swap(j1.id, j2.id);
            std::swap(j1.cmd, j2.cmd);

            std::swap(j1.outputBuf, j2.outputBuf);
            std::swap(j1.errorBuf, j2.errorBuf);

            std::swap(j1.status, j2.status);
        }

        JobID get_ID() const noexcept
        {
            return id;
        }
        const std::u16string &get_cmd() const noexcept
        {
            return cmd;
        }
        OutputBuf &get_outBuf() noexcept
        {
            return outputBuf;
        }
        ErrorBuf &get_errBuf() noexcept
        {
            return errorBuf;
        }

        Status get_status() const noexcept
        {
            return status;
        }

    protected:
        JobID id;
        std::u16string cmd;

        OutputBuf outputBuf;
        ErrorBuf errorBuf;

        Status status;

    private:
        inline static JobID id_count = 1;
    };

} // namespace MatlabPool

#endif