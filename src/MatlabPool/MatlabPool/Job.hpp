#ifndef MATLABPOOL_JOB_HPP
#define MATLABPOOL_JOB_HPP

#include <string>
#include <vector>
#include <functional>

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

    enum class JobStatus : uint8_t
    {
        Wait,
        AssignWorker,
        InProgress,
        Done,
        Error,
    };


    class Job
    {
        using SBuf = std::basic_stringbuf<char16_t>;

        Job(const Job &) = delete;
        Job &operator=(const Job &) = delete;

    public:
        Job() noexcept : id(0), workerID(-1){};
        Job(std::u16string function, std::size_t nlhs, std::vector<matlab::data::Array> &&args) : id(id_count++),
                                                                                                  function(std::move(function)),
                                                                                                  nlhs(nlhs),
                                                                                                  args(std::move(args)),
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
            std::swap(j1.function, j2.function);

            std::swap(j1.nlhs, j2.nlhs);
            std::swap(j1.args, j2.args);
            std::swap(j1.result, j2.result);

            std::swap(j1.outputBuf, j2.outputBuf);
            std::swap(j1.errorBuf, j2.errorBuf);

            std::swap(j1.workerID, j2.workerID);
        }

        void set_workerID(int val) noexcept
        {
            workerID = val;
        }
        int get_workerID() const noexcept
        {
            return workerID;
        }

    public:
        JobID id;
        std::u16string function;

        std::size_t nlhs;
        std::vector<matlab::data::Array> args;
        std::vector<matlab::data::Array> result;

        OutputBuf outputBuf;
        ErrorBuf errorBuf;

    private:
        inline static JobID id_count = 1;
        int workerID;
    };

} // namespace MatlabPool

#endif