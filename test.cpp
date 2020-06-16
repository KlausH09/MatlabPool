#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"
#include <iostream>
#include <map>

#include "MatlabPool/EngineHack.hpp"
#include "MatlabPool/Job.hpp"

using namespace MatlabPool;

using FResult = matlab::engine::FutureResult<std::vector<matlab::data::Array>>;
using Result = std::vector<matlab::data::Array>;

std::map<std::size_t, Result> results;


int main()
{
    // Call MATLAB sqrt function on array

    using namespace matlab::engine;

    // Start MATLAB engine synchronously

    std::vector<std::u16string> options = {u"-nojvm", u"-nosplash"};
    EngineHack engine1(options);
    EngineHack engine2(options);

    // Create  MATLAB data array factory
    matlab::data::ArrayFactory factory;

    // Define a four-element array
    Job job1("sqrt", 1, {factory.createArray<double>({1, 4}, {-2.0, 2.0, 6.0, 8.0})});
    Job job2("pause", 0, {factory.createArray<double>({1, 1}, {2.0})});

    // Call MATLAB function
    FutureResult<std::vector<matlab::data::Array>> fresult1 = engine1.eval_job(std::move(job1)); 
    FutureResult<std::vector<matlab::data::Array>> fresult2 = engine2.eval_job(std::move(job2)); 

    std::cerr << "Bla Bla\n";

    std::vector<matlab::data::Array> result_vec = fresult1.get();
    fresult2.get();

    // Display results
    matlab::data::TypedArray<std::complex<double>> results = result_vec[0];

    for (auto r : results)
    {
        double realPart = r.real();
        double imgPart = r.imag();
        std::cerr << "Square root is " << realPart << " + " << imgPart << "i\n";
    }
    std::cerr << "Ende \n";
}
