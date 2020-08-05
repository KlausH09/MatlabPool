#ifndef MATLABPOOL_ENGINEHACK_HPP
#define MATLABPOOL_ENGINEHACK_HPP

#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"

#include "MatlabPoolLib/JobFuture.hpp"
#include "MatlabPool/StreamBuf.hpp"

namespace MatlabPool
{
    using Future = matlab::execution::FutureResult<std::vector<matlab::data::Array>>;
    using Notifier = std::function<void()>;

    // This class provides another function for job execution
    // on a matlab instance. This new functions works like the
    // matlab::engine::MATLABEngine::fevalAsync function but
    // it also executes a "Notifier" function handle after
    // the function call or rather after setting a value or
    // exception in the std::promise object.
    class EngineHack : public matlab::engine::MATLABEngine
    {
        // This struct is needed to deliver the extra field
        // "notifier" to the "set_feval_promise_data_hack" 
        // and "set_feval_promise_exception_hack".
        struct MatlabPromiseHack
        {
            std::promise<std::vector<matlab::data::Array>> *prom;
            Notifier notifier;
        };

    public:
        EngineHack(const EngineHack &) = delete;
        EngineHack &operator=(const EngineHack &) = delete;

        EngineHack(const std::vector<std::u16string> &options);

        // like matlab::engine::MATLABEngine::fevalAsync, but
        // it also runs the notifier at the end of the job
        void eval_job(JobFuture &job, Notifier &&notifier);

    private:
        // start a matlab session 
        std::future<uint64_t> start_matlabasync(const std::vector<std::u16string> &options);

        // set a value in the std::promise object and call 
        // the "Notifier" object
        // original function: set_feval_promise_data
        // source: matlabroot/extern/include/MatlabEngine/detail/engine_execution_interface_impl.hpp
        static void set_feval_promise_data_hack(void *p, size_t nlhs, bool straight,
                                                matlab::data::impl::ArrayImpl **plhs);

        // set a exception in the std::promise object and call 
        // the "Notifier" object
        // original function: set_feval_promise_exception
        // source: matlabroot/extern/include/MatlabEngine/detail/engine_execution_interface_impl.hpp
        static void set_feval_promise_exception_hack(void *p, size_t nlhs, bool straight,
                                                     size_t excTypeNumber, const void *msg);
    };
} // namespace MatlabPool

#endif