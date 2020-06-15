#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"
#include <iostream>

int blaid()
{
    static int val = 47;
    return val++;
}

auto set_feval_promise_data_hack([](void *p, size_t nlhs, bool straight, matlab::data::impl::ArrayImpl **plhs) {
    matlab::execution::set_feval_promise_data(p, nlhs, straight, plhs);
    std::cerr << "Hello World from " << blaid() << "\n";
});

struct Job
{
    static std::size_t id_count;
    const std::size_t id;

    Job() : id(id_count++)
    {
    }

    std::string function;
    std::size_t nlhs;
    std::vector<matlab::data::Array> args;
    matlab::engine::FutureResult<std::vector<matlab::data::Array>> future_result;

    inline void set_function(const std::u16string &str)
    {
        std::unique_ptr<char[]> asciistr_ptr(new char[str.size() + 1]);
        function.resize(str.size());

        const char *u16_src = reinterpret_cast<const char *>(str.c_str());
        for (size_t n = 0; n < str.size(); ++n)
            function[n] = u16_src[2 * n];
    }
    
    void eval(std::unique_ptr<matlab::engine::MATLABEngine> &matlabPtr)
    {
        future_result = matlabPtr->fevalAsync(function,nlhs,args);
    }
};
std::size_t Job::id_count = 0;

class EngineHack : public matlab::engine::MATLABEngine
{
    EngineHack(const EngineHack &) = delete;
    EngineHack &operator=(const EngineHack &) = delete;

public:
    inline void eval_job(Job &job,
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
        std::future<std::vector<matlab::data::Array>> f = p->get_future();

        void *output_ = output ? new std::shared_ptr<StreamBuffer>(std::move(output)) : nullptr;
        void *error_ = error ? new std::shared_ptr<StreamBuffer>(std::move(error)) : nullptr;

        uintptr_t handle = cpp_engine_feval_with_completion(matlabHandle, function.c_str(), nlhs, false, argsImpl, nrhs, set_feval_promise_data_hack, &set_feval_promise_exception, p, output_, error_, &writeToStreamBuffer, &deleteStreamBufferImpl);

        job.future_result = FutureResult<std::vector<matlab::data::Array>>(std::move(f), std::make_shared<TaskReference>(handle, cpp_engine_cancel_feval_with_completion));
    }
};

void callFevalsqrt()
{
    // Call MATLAB sqrt function on array

    using namespace matlab::engine;

    // Start MATLAB engine synchronously

    std::vector<matlab::engine::String> options = {u"-nojvm", u"-nosplash"};
    std::unique_ptr<MATLABEngine> matlabPtr = startMATLAB(options);
    EngineHack *hack = static_cast<EngineHack *>(matlabPtr.get());

    // Create  MATLAB data array factory
    matlab::data::ArrayFactory factory;

    // Define a four-element array

    Job job1;
    job1.function = "sqrt";
    job1.nlhs = 1;
    job1.args.push_back(factory.createArray<double>({1, 4}, {-2.0, 2.0, 6.0, 8.0}));


    Job job2;
    job2.function = "pause";
    job2.nlhs = 0;
    job2.args.push_back(factory.createArray<double>({1, 1}, {5.0}));

    // Call MATLAB function
    //FutureResult<std::vector<matlab::data::Array>> future = hack->fevalAsync(u"sqrt",1, argArray); // std::move(argArray)
    hack->eval_job(job1); // std::move(argArray)
    hack->eval_job(job2); // std::move(argArray)

    std::cerr << "Bla Bla\n";

    std::vector<matlab::data::Array> result_vec = job1.future_result.get();
    job2.future_result.get();

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