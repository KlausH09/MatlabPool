#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"
#include <iostream>
#include <map>

#include "MatlabPool/Job.hpp"

using namespace MatlabPool;

using FResult = matlab::engine::FutureResult<std::vector<matlab::data::Array>>;
using Result = std::vector<matlab::data::Array>;

std::map<std::size_t, Result> results;

struct MatlabPromiseHack
{
    using MatlabPromise = std::promise<std::vector<matlab::data::Array>>;
    MatlabPromise *prom;
    std::size_t jobID;

    MatlabPromiseHack(MatlabPromise *prom, std::size_t jobID) : prom(prom), jobID(jobID) {}
};

void set_feval_promise_data_hack(void *p, size_t nlhs, bool straight, matlab::data::impl::ArrayImpl **plhs) {
    MatlabPromiseHack *p_hack = reinterpret_cast<MatlabPromiseHack *>(p);
    matlab::execution::set_feval_promise_data(p_hack->prom, nlhs, straight, plhs);
    std::cerr << "promise: set data " << p_hack->jobID << "\n";
    delete p_hack;
}

void set_feval_promise_exception_hack(void *p, size_t nlhs, bool straight, size_t excTypeNumber, const void *msg) {
    MatlabPromiseHack *p_hack = reinterpret_cast<MatlabPromiseHack *>(p);
    matlab::execution::set_feval_promise_exception(p_hack->prom, nlhs, straight, excTypeNumber, msg);
    std::cerr << "promise: error " << p_hack->jobID << "\n";
    delete p_hack;
}

class EngineHack : public matlab::engine::MATLABEngine
{
    EngineHack(const EngineHack &) = delete;
    EngineHack &operator=(const EngineHack &) = delete;

public:
    inline matlab::execution::FutureResult<std::vector<matlab::data::Array>> eval_job(Job &job,
                                                                   const std::shared_ptr<matlab::execution::StreamBuffer> &output = nullptr,
                                                                   const std::shared_ptr<matlab::execution::StreamBuffer> &error = nullptr)
    {
        using namespace matlab::execution;
        const std::string &function = job.function;
        const size_t nlhs = job.nlhs;
        const std::vector<matlab::data::Array> &args = job.args;

        // ================== fevalAsync Implementierung =========================
        size_t nrhs = args.size();
        std::unique_ptr<matlab::data::impl::ArrayImpl *, void (*)(matlab::data::impl::ArrayImpl **)> argsImplPtr(new matlab::data::impl::ArrayImpl *[nrhs], [](matlab::data::impl::ArrayImpl **ptr) {
            delete[] ptr;
        });
        matlab::data::impl::ArrayImpl **argsImpl = argsImplPtr.get();
        size_t i = 0;
        for (auto e : args)
        {
            argsImpl[i++] = matlab::data::detail::Access::getImpl<matlab::data::impl::ArrayImpl>(e);
        }

        std::promise<std::vector<matlab::data::Array>> *p = new std::promise<std::vector<matlab::data::Array>>();
        MatlabPromiseHack *p_hack = new MatlabPromiseHack(p, job.id);
        std::future<std::vector<matlab::data::Array>> f = p->get_future();

        void *output_ = output ? new std::shared_ptr<StreamBuffer>(std::move(output)) : nullptr;
        void *error_ = error ? new std::shared_ptr<StreamBuffer>(std::move(error)) : nullptr;

        uintptr_t handle = cpp_engine_feval_with_completion(matlabHandle, function.c_str(), nlhs, false, argsImpl, nrhs, set_feval_promise_data_hack, set_feval_promise_exception_hack, p_hack, output_, error_, &writeToStreamBuffer, &deleteStreamBufferImpl);
        std::cerr << matlabHandle << "Bla Bla\n";
        return FutureResult<std::vector<matlab::data::Array>>(std::move(f), std::make_shared<TaskReference>(handle, cpp_engine_cancel_feval_with_completion));
    }
};

void callFevalsqrt()
{
    // Call MATLAB sqrt function on array

    using namespace matlab::engine;

    // Start MATLAB engine synchronously

    std::vector<matlab::engine::String> options = {u"-nojvm", u"-nosplash"};
    
    std::unique_ptr<MATLABEngine> matlabPtr1 = startMATLAB(options);
    std::unique_ptr<MATLABEngine> matlabPtr2 = startMATLAB(options);
    EngineHack *hack1 = static_cast<EngineHack *>(matlabPtr1.get());
    EngineHack *hack2 = static_cast<EngineHack *>(matlabPtr2.get());

    // Create  MATLAB data array factory
    matlab::data::ArrayFactory factory;

    // Define a four-element array
    Job job1("sqrt", 1, {factory.createArray<double>({1, 4}, {-2.0, 2.0, 6.0, 8.0})});
    Job job2("pause", 0, {factory.createArray<double>({1, 1}, {2.0})});

    // Call MATLAB function
    FutureResult<std::vector<matlab::data::Array>> fresult1 = hack1->eval_job(job1); // std::move(argArray)
    FutureResult<std::vector<matlab::data::Array>> fresult2 = hack2->eval_job(job2); // std::move(argArray)

    std::cerr << "Bla Bla\n";

    std::vector<matlab::data::Array> result_vec = fresult1.get();
    fresult2.get();

    // Display results
    int i = 0;
    matlab::data::TypedArray<double> arg = job1.args[0];
    matlab::data::TypedArray<std::complex<double>> results = result_vec[0];

    for (auto r : results)
    {
        double a = arg[i++];
        double realPart = r.real();
        double imgPart = r.imag();
        std::cerr << "Square root of " << a << " is " << realPart << " + " << imgPart << "i\n";
    }
}

int main()
{
    callFevalsqrt();
}