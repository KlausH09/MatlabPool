#ifndef MATLABPOOL_JOBBASE_HPP
#define MATLABPOOL_JOBBASE_HPP

#include <string>

#include "MatlabPool/StreamBuf.hpp"
#include "MatlabPool/Exception.hpp"
#include "MatlabPool/Utilities.hpp"

#include "MatlabDataArray.hpp"

namespace MatlabPool
{
    using JobID = std::uint64_t;

    // base class for job classes. Every object of this class gets
    // an unique id (JobID). This class also provides an error and
    // output buffer, these buffers can be used during the job
    // execution to save e.g. error information.
    class JobBase
    {
    public:
        class JobBaseException : public MatlabPool::Exception
        {
        };
        class ExecutionError : public JobBaseException
        {
        public:
            ExecutionError(std::shared_ptr<StringBuf> buffer);
            ExecutionError(JobID id, std::shared_ptr<StringBuf> buffer);
            const char *what() const noexcept override;
            const char *identifier() const noexcept override;

        private:
            std::string msg;
        };

    protected:
        JobBase() noexcept;
        JobBase(std::u16string cmd);

    public:
        JobBase(const JobBase &) = delete;
        JobBase &operator=(const JobBase &) = delete;

        JobBase(JobBase &&other) noexcept;
        JobBase &operator=(JobBase &&other) noexcept;

        friend void swap(JobBase &j1, JobBase &j2) noexcept;

        JobID get_ID() const noexcept;
        const std::u16string &get_cmd() const noexcept;
        StreamBuf &get_outBuf() noexcept;
        StreamBuf &get_errBuf() noexcept;

        // store the members of this object in a matlab struct
        matlab::data::StructArray toStruct();

    protected:
        inline static matlab::data::ArrayFactory factory;

        JobID id;
        std::u16string cmd;

        StreamBuf outputBuf;
        StreamBuf errorBuf;
    private:
        inline static JobID id_count = 1;
    };

} // namespace MatlabPool

#endif