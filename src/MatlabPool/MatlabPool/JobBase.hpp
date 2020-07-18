#ifndef MATLABPOOL_JOBBASE_HPP
#define MATLABPOOL_JOBBASE_HPP

#include <string>
#include <vector>
#include <ostream>

#include "./StreamBuf.hpp"
#include "./Exception.hpp"

#include "MatlabDataArray.hpp"

namespace MatlabPool
{

// TODO was wenn dll und mex unterschiedlich kompiliert werden 'MATLABPOOL_DISP_WORKER_OUTPUT'
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
        JobBase(const JobBase &) = delete;
        JobBase &operator=(const JobBase &) = delete;

    public:
        class JobBaseException : public MatlabPool::Exception
        {
        };
        class ExecutionError : public JobBaseException
        {
        public:
            ExecutionError(JobID id, std::shared_ptr<StreamBuf> buffer)
            {
                std::ostringstream os;
                os << "an error has occurred during job execution (id: " << id << ")";
                if(buffer)
                {
                    os << '\n' << convertUTF16StringToASCIIString(buffer->str());
                }
                msg = os.str();
            }
            const char *what() const noexcept override
            {
                return msg.c_str();
            }
            const char *identifier() const noexcept override
            {
                return "JobExecutionError";
            }
        private:
            std::string msg;
        };

    protected:
        JobBase() noexcept : id(0){};
        JobBase(std::u16string cmd) : id(id_count++),
                                      cmd(std::move(cmd))
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
            using std::swap;
            swap(j1.id, j2.id);
            swap(j1.cmd, j2.cmd);

            swap(j1.outputBuf, j2.outputBuf);
            swap(j1.errorBuf, j2.errorBuf);
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

    protected:
        JobID id;
        std::u16string cmd;

        OutputBuf outputBuf;
        ErrorBuf errorBuf;

    private:
        inline static JobID id_count = 1;
    };

} // namespace MatlabPool

#endif