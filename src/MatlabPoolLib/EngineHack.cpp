#include "MatlabPoolLib/EngineHack.hpp"

namespace MatlabPool
{
    EngineHack::EngineHack(const std::vector<std::u16string> &options)
        : matlab::engine::MATLABEngine(start_matlabasync(options).get()) {}

    // this function is copied and adjusted accordingly, source:
    //   matlabroot/extern/include/cppmex/detail/mexApiAdapterImpl.hpp
    //   (Line 504)
    void EngineHack::eval_job(JobFuture &job, Notifier &&notifier)
    {
        using namespace matlab::execution;
        using matlab::data::impl::ArrayImpl;

        size_t nrhs = job.get_args().size();

        std::unique_ptr<ArrayImpl *, void (*)(ArrayImpl **)> argsImplPtr(
            new ArrayImpl *[nrhs],
            [](ArrayImpl **ptr)
            {
                delete[] ptr;
            }
        );

        ArrayImpl **argsImpl = argsImplPtr.get();
        size_t i = 0;
        for (auto &e : job.get_args())
            argsImpl[i++] = matlab::data::detail::Access::getImpl<ArrayImpl>(std::move(e));

        std::promise<std::vector<matlab::data::Array>> *p =
            new std::promise<std::vector<matlab::data::Array>>();

        MatlabPromiseHack *p_hack = new MatlabPromiseHack{ p, std::move(notifier) };

        std::future<std::vector<matlab::data::Array>> f = p->get_future();

        // the maltab implementation will call 'delete output' and 
        // 'delete error', so we have to copy it
        void *output = job.get_outBuf().get()
            ? new std::shared_ptr<StreamBuffer>(job.get_outBuf().get())
            : nullptr;
        void *error = job.get_errBuf().get()
            ? new std::shared_ptr<StreamBuffer>(job.get_errBuf().get())
            : nullptr;

        std::string funstr = MatlabPool::convertUTF16StringToASCIIString(job.get_cmd());

        uintptr_t handle = cpp_engine_feval_with_completion(
            matlabHandle,
            funstr.c_str(),
            job.get_nlhs(), false, argsImpl, nrhs,
            set_feval_promise_data_hack,
            set_feval_promise_exception_hack,
            p_hack, output, error,
            &writeToStreamBuffer,
            &deleteStreamBufferImpl
        );

        job.set_future(FutureResult<std::vector<matlab::data::Array>>(
            std::move(f),
            std::make_shared<TaskReference>(handle, cpp_engine_cancel_feval_with_completion))
        );
    }

    // this function is copied from:
    //   matlabroot/extern/include/MatlabEngine/detail/engine_factory_impl.hpp
    //   (Line 36)
    std::future<uint64_t> EngineHack::start_matlabasync(const std::vector<std::u16string> &options)
    {
        initSession();

        auto startMATLABType = [options]() {
            std::vector<char16_t *> options_v(options.size());

            std::transform(options.begin(), options.end(), options_v.begin(),
                [](const std::u16string &option) {
                    return const_cast<char16_t *>(option.c_str());
                }
            );

            bool errFlag = false;

            uint64_t matlab = cpp_engine_create_out_of_process_matlab(
                options_v.data(), options_v.size(), &errFlag);

            if (errFlag)
                throw matlab::engine::EngineException(
                    "MATLAB process cannot be created.");

            return matlab;
        };
        std::future<uint64_t> future = std::async(std::launch::async, startMATLABType);
        return future;
    }

    void EngineHack::set_feval_promise_data_hack(
        void *p, size_t nlhs, bool straight,
        matlab::data::impl::ArrayImpl **plhs)
    {
        MatlabPromiseHack *p_hack = reinterpret_cast<MatlabPromiseHack *>(p);

        // call the original function
        matlab::execution::set_feval_promise_data(
            p_hack->prom, nlhs, straight, plhs);

        // call the notifier
        p_hack->notifier();

        // the MatlabPromiseHack object is no longer needed
        delete p_hack;
    }

    void EngineHack::set_feval_promise_exception_hack(
        void *p, size_t nlhs, bool straight, 
        size_t excTypeNumber, const void *msg)
    {
        MatlabPromiseHack *p_hack = reinterpret_cast<MatlabPromiseHack *>(p);

        // call the original function
        matlab::execution::set_feval_promise_exception(
            p_hack->prom, nlhs, straight, excTypeNumber, msg);

        // call the notifier
        p_hack->notifier();

        // the MatlabPromiseHack object is no longer needed
        delete p_hack;
    }
} // namespace MatlabPool
