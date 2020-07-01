#ifndef MATLABPOOL_JOB_HPP
#define MATLABPOOL_JOB_HPP

#include <string>
#include <vector>
#include <functional>

#include "MatlabDataArray.hpp"

#define MATLABPOOL_DISP_WORKER_OUTPUT
#define MATLABPOOL_DISP_WORKER_ERROR

namespace MatlabPool
{
    using JobID = std::size_t;

    class Job
    {
        using SBuf = std::basic_stringbuf<char16_t>;

        Job(const Job &) = delete;
        Job &operator=(const Job &) = delete;

    public:
        Job() noexcept : id(0) {}

        Job(std::u16string function, std::size_t nlhs, std::vector<matlab::data::Array> &&args)
            : id(id_count++),
              function(std::move(function)),
              nlhs(nlhs),
              args(std::move(args))
        {
#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
            outputBuf = std::make_shared<SBuf>();
#endif
#ifdef MATLABPOOL_DISP_WORKER_ERROR
            errorBuf = std::make_shared<SBuf>();
#endif
        }

        Job(Job &&other) noexcept : Job()
        {
            swap(*this, other);
        }

        Job &operator=(Job &&other) noexcept
        {
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
        }

        std::shared_ptr<SBuf> *get_outputBuf() noexcept
        {
#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
            return &outputBuf;
#else
            return nullptr;
#endif
        }
        std::shared_ptr<SBuf> *get_errorBuf() noexcept
        {
#ifdef MATLABPOOL_DISP_WORKER_ERROR
            return &errorBuf;
#else
            return nullptr;
#endif
        }

    public: // TODO
        inline static JobID id_count = 1;
        JobID id;
        std::u16string function;
        std::size_t nlhs;
        std::vector<matlab::data::Array> args;
        std::vector<matlab::data::Array> result;

    private:
        std::shared_ptr<SBuf> outputBuf;
        std::shared_ptr<SBuf> errorBuf;

    };

} // namespace MatlabPool

#endif