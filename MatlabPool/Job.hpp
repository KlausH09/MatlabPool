#ifndef MATLABPOOL_JOB_HPP
#define MATLABPOOL_JOB_HPP

#include <string>
#include <vector>
#include <functional>

#include "MatlabDataArray.hpp"

#include "./Definitions.hpp"

#define MATLABPOOL_DISP_WORKER_OUTPUT
#define MATLABPOOL_DISP_WORKER_ERROR

namespace MatlabPool
{

    class Job
    {
        using SBuf = std::basic_stringbuf<char16_t>;

        Job(const Job &) = delete;
        Job &operator=(const Job &) = delete;

    public:
        Job() : id(0) {}

        Job(std::string &&function, std::size_t nlhs, ArgVal &&args) : id(id_count++),
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

        Job(Job &&other) : Job()
        {
            swap(*this, other);
        }

        Job &operator=(Job &&other)
        {
            swap(*this, other);
            return *this;
        }

        void swap(Job &j1, Job &j2) // TODO friend
        {
            std::swap(j1.id, j2.id);
            std::swap(j1.function, j2.function);
            std::swap(j1.nlhs, j2.nlhs);
            std::swap(j1.args, j2.args);
            std::swap(j1.result, j2.result);

#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
            std::swap(j1.outputBuf, j2.outputBuf);
#endif
#ifdef MATLABPOOL_DISP_WORKER_ERROR
            std::swap(j1.errorBuf, j2.errorBuf);
#endif
        }

        std::shared_ptr<SBuf> *get_outputBuf()
        {
#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
            return &outputBuf;
#else
            return nullptr;
#endif
        }
        std::shared_ptr<SBuf> *get_errorBuf()
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
        std::string function;
        std::size_t nlhs;
        ArgVal args;

    private:
#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
        std::shared_ptr<SBuf> outputBuf;
#endif
#ifdef MATLABPOOL_DISP_WORKER_ERROR
        std::shared_ptr<SBuf> errorBuf;
#endif

    public: // TOOD
        ResultVal result;
    };

} // namespace MatlabPool

#endif