#ifndef MATLABPOOL_JOBBASE_HPP
#define MATLABPOOL_JOBBASE_HPP

#include <string>

#include "MatlabPool/StreamBuf.hpp"
#include "MatlabPool/Exception.hpp"


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
        JobBase(const JobBase &) = delete;
        JobBase &operator=(const JobBase &) = delete;

    public:
        class JobBaseException : public MatlabPool::Exception
        {
        };
        class ExecutionError : public JobBaseException
        {
        public:
            ExecutionError(std::shared_ptr<StreamBuf> buffer);
            ExecutionError(JobID id, std::shared_ptr<StreamBuf> buffer);
            const char *what() const noexcept override;
            const char *identifier() const noexcept override;
        private:
            std::string msg;
        };

    protected:
        JobBase() noexcept;
        JobBase(std::u16string cmd);

    public:
        JobBase(JobBase &&other) noexcept;
        JobBase &operator=(JobBase &&other) noexcept;

        friend void swap(JobBase &j1, JobBase &j2) noexcept;

        JobID get_ID() const noexcept;
        const std::u16string &get_cmd() const noexcept;
        OutputBuf &get_outBuf() noexcept;
        ErrorBuf &get_errBuf() noexcept;

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