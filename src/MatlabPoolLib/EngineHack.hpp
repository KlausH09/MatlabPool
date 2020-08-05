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

    class EngineHack : public matlab::engine::MATLABEngine
    {
        struct MatlabPromiseHack
        {
            std::promise<std::vector<matlab::data::Array>> *prom;
            Notifier notifier;
        };

    public:
        EngineHack(const EngineHack &) = delete;
        EngineHack &operator=(const EngineHack &) = delete;

        EngineHack(const std::vector<std::u16string> &options);

        void eval_job(JobFuture &job, Notifier &&notifier);

    private:
        std::future<uint64_t> start_matlabasync(const std::vector<std::u16string> &options);

        static void set_feval_promise_data_hack(void *p, size_t nlhs, bool straight,
                                                matlab::data::impl::ArrayImpl **plhs);

        static void set_feval_promise_exception_hack(void *p, size_t nlhs, bool straight,
                                                     size_t excTypeNumber, const void *msg);
    };
} // namespace MatlabPool

#endif