#ifndef MATLABPOOL_ENGINEHACK_HPP
#define MATLABPOOL_ENGINEHACK_HPP

#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"

#include "./Job.hpp"

namespace MatlabPool
{
    using Future = matlab::execution::FutureResult<std::vector<matlab::data::Array>>;
    using Notifier = std::function<void()>;

    class EngineHack : public matlab::engine::MATLABEngine
    {
        struct MatlabPromiseHack
        {
            std::promise<std::vector<matlab::data::Array>> *prom;
            Notifier notifier;
        };
        EngineHack(const EngineHack &) = delete;
        EngineHack &operator=(const EngineHack &) = delete;

    public:
        EngineHack(const std::vector<std::u16string> &options) : matlab::engine::MATLABEngine(start_matlabasync(options).get()) {}

        Future eval_job(Job_feval &job, Notifier &&notifier)
        {
            using namespace matlab::execution;

            // ================== fevalAsync, angepasste Implementierung =========================
            size_t nrhs = job.args.size();
            std::unique_ptr<matlab::data::impl::ArrayImpl *, void (*)(matlab::data::impl::ArrayImpl **)> argsImplPtr(new matlab::data::impl::ArrayImpl *[nrhs], [](matlab::data::impl::ArrayImpl **ptr) {
                delete[] ptr;
            });
            matlab::data::impl::ArrayImpl **argsImpl = argsImplPtr.get();
            size_t i = 0;
            for (auto &e : job.args)
                argsImpl[i++] = matlab::data::detail::Access::getImpl<matlab::data::impl::ArrayImpl>(std::move(e));

            std::promise<std::vector<matlab::data::Array>> *p = new std::promise<std::vector<matlab::data::Array>>();
            MatlabPromiseHack *p_hack = new MatlabPromiseHack{p, std::move(notifier)};
            std::future<std::vector<matlab::data::Array>> f = p->get_future();

            // the maltab implementation will call 'delete output' and 'delete error', so we have to copy it
            void *output = job.outputBuf.get() ? new std::shared_ptr<StreamBuffer>(job.outputBuf.get()) : nullptr;
            void *error = job.errorBuf.get() ? new std::shared_ptr<StreamBuffer>(job.errorBuf.get()) : nullptr;

            std::string funstr = convertUTF16StringToASCIIString(job.function);

            uintptr_t handle = cpp_engine_feval_with_completion(matlabHandle, funstr.c_str(),
                                                                job.nlhs, false, argsImpl, nrhs,
                                                                set_feval_promise_data_hack,
                                                                set_feval_promise_exception_hack,
                                                                p_hack, output, error,
                                                                &writeToStreamBuffer,
                                                                &deleteStreamBufferImpl);

            return FutureResult<std::vector<matlab::data::Array>>(std::move(f), std::make_shared<TaskReference>(handle, cpp_engine_cancel_feval_with_completion));
        }

    private:
        inline std::future<uint64_t> start_matlabasync(const std::vector<std::u16string> &options)
        {
            initSession();
            auto startMATLABType = [options]() {
                std::vector<char16_t *> options_v(options.size());
                std::transform(options.begin(), options.end(), options_v.begin(), [](const std::u16string &option) { return const_cast<char16_t *>(option.c_str()); });
                bool errFlag = false;
                uint64_t matlab = cpp_engine_create_out_of_process_matlab(options_v.data(), options_v.size(), &errFlag);
                if (errFlag)
                    throw matlab::engine::EngineException("MATLAB process cannot be created.");
                return matlab;
            };
            std::future<uint64_t> future = std::async(std::launch::async, startMATLABType);
            return future;
        }

        static void set_feval_promise_data_hack(void *p, size_t nlhs, bool straight, matlab::data::impl::ArrayImpl **plhs)
        {
            MatlabPromiseHack *p_hack = reinterpret_cast<MatlabPromiseHack *>(p);
            matlab::execution::set_feval_promise_data(p_hack->prom, nlhs, straight, plhs);
            p_hack->notifier();
            delete p_hack;
        }

        static void set_feval_promise_exception_hack(void *p, size_t nlhs, bool straight, size_t excTypeNumber, const void *msg)
        {
            MatlabPromiseHack *p_hack = reinterpret_cast<MatlabPromiseHack *>(p);
            matlab::execution::set_feval_promise_exception(p_hack->prom, nlhs, straight, excTypeNumber, msg);
            p_hack->notifier();
            delete p_hack;
        }

        // copied from ExecutionInterface::convertUTF16StringToASCIIString
        inline std::string convertUTF16StringToASCIIString(const std::u16string &str)
        {
            std::unique_ptr<char []> asciistr_ptr(new char[str.size()+1]);
            asciistr_ptr.get()[str.size()] = '\0';
            const char* u16_src = reinterpret_cast<const char*>(str.c_str());
            for(size_t n = 0; n < str.size(); ++n)
            {   
               asciistr_ptr.get()[n] = u16_src[2*n];
            }
            return std::string(asciistr_ptr.get());
        }
    };
} // namespace MatlabPool

#endif