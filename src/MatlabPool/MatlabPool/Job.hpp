#ifndef MATLABPOOL_JOB_HPP
#define MATLABPOOL_JOB_HPP

#include <string>
#include <vector>
#include <functional>
#include <exception>
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

    class Job
    {
        using SBuf = std::basic_stringbuf<char16_t>;

        Job(const Job &) = delete;
        Job &operator=(const Job &) = delete;

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
        class NoResults : public Exception
        {
        public:
            NoResults(JobID id, Status status)
            {
                std::ostringstream os;
                os << "there are no results for this job (id: " << id << ", status: " << static_cast<unsigned>(status) << ")";
                msg = os.str();
            }
            const char *what() const noexcept override
            {
                return msg.c_str();
            }

        private:
            std::string msg;
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
        class AssignToWorkerStatus : public Exception
        {
        public:
            AssignToWorkerStatus(JobID id, Status status)
            {
                std::ostringstream os;
                os << "job status must be \"Wait\" before it can be set to \"AssignToWorker\" (id: "
                   << id << ", status: " << static_cast<unsigned>(status) << ")";
                msg = os.str();
            }
            const char *what() const noexcept override
            {
                return msg.c_str();
            }

        private:
            std::string msg;
        };
        class InProgressStatus : public Exception
        {
        public:
            InProgressStatus(JobID id, Status status)
            {
                std::ostringstream os;
                os << "job status must be \"AssignToWorker\" before it can be set to \"InProgress\" (id: "
                   << id << ", status: " << static_cast<unsigned>(status) << ")";
                msg = os.str();
            }
            const char *what() const noexcept override
            {
                return msg.c_str();
            }

        private:
            std::string msg;
        };

    public:
        Job() noexcept : id(0), status(Status::Empty), workerID(-1){};
        Job(std::u16string cmd, std::size_t nlhs, std::vector<matlab::data::Array> &&args) : id(id_count++),
                                                                                             cmd(std::move(cmd)),
                                                                                             nlhs(nlhs),
                                                                                             args(std::move(args)),
                                                                                             status(Status::Wait),
                                                                                             workerID(-1)
        {
        }

        Job(Job &&other) noexcept : Job()
        {
            using std::swap;
            swap(*this, other);
        }

        Job &operator=(Job &&other) noexcept
        {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        friend void swap(Job &j1, Job &j2) noexcept
        {
            std::swap(j1.id, j2.id);
            std::swap(j1.cmd, j2.cmd);

            std::swap(j1.nlhs, j2.nlhs);
            std::swap(j1.args, j2.args);

            std::swap(j1.outputBuf, j2.outputBuf);
            std::swap(j1.errorBuf, j2.errorBuf);

            std::swap(j1.status, j2.status);
            std::swap(j1.result, j2.result);

            std::swap(j1.workerID, j2.workerID);
        }
        void set_AssignToWorker_status()
        {
            if (status != Status::Wait)
                throw AssignToWorkerStatus(id, status);
            status = Status::AssignToWorker;
        }

        JobID get_ID() const noexcept
        {
            return id;
        }
        const std::u16string &get_cmd() const noexcept
        {
            return cmd;
        }
        std::size_t get_nlhs() const noexcept
        {
            return nlhs;
        }
        std::vector<matlab::data::Array>& get_args() noexcept
        {   
            return args;
        }
        OutputBuf &get_outBuf() noexcept
        {
            return outputBuf;
        }
        ErrorBuf &get_errBuf() noexcept
        {
            return errorBuf;
        }
        void set_workerID(int val)
        {
            if (val >= 0)
            {
                if (status != Status::AssignToWorker)
                    throw InProgressStatus(id, status);
                status = Status::InProgress;
            }
            workerID = val;
        }

        int get_workerID() const noexcept
        {
            return workerID;
        }

        Status get_status() const noexcept
        {
            return status;
        }

        const std::vector<matlab::data::Array> &peek_result() const
        {
            check_result();
            return result;
        }
        std::vector<matlab::data::Array> pop_result()
        {
            check_result();
            status = Status::DoneEmpty;
            return std::move(result);
        }

    private:
        inline void check_result() const
        {
            if (status == Status::Error)
                throw ExecutionError(id);
            if (status != Status::Done)
                throw NoResults(id, status);
        }

    protected:
        JobID id;
        std::u16string cmd;

        std::size_t nlhs;
        std::vector<matlab::data::Array> args;

        OutputBuf outputBuf;
        ErrorBuf errorBuf;

        Status status;
        std::vector<matlab::data::Array> result;

    private:
        inline static JobID id_count = 1;
        int workerID;
    };

} // namespace MatlabPool

#endif