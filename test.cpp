#include "MatlabDataArray.hpp"
#include "MatlabEngine.hpp"
#include <iostream>
#include <map>

#include "MatlabPool/EngineHack.hpp"
#include "MatlabPool/Job.hpp"
#include "MatlabPool/Pool.hpp"

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
    MatlabPool::Pool pool(2, options);

    // Create  MATLAB data array factory
    matlab::data::ArrayFactory factory;

    // Define a four-element array
    Job job1("sqrt", 1, {factory.createArray<double>({1, 4}, {-2.0, 2.0, 6.0, 8.0})});
    Job job2("disp", 0, {factory.createArray<double>({1, 1}, {2.0})});

    std::size_t id1 = pool.submit(std::move(job1));
    std::size_t id2 = pool.submit(std::move(job2));

    job1 = pool.wait(id1);
    job2 = pool.wait(id2);

    // Display results
    matlab::data::TypedArray<std::complex<double>> results = job1.result[0];

    for (auto r : results)
    {
        double realPart = r.real();
        double imgPart = r.imag();
        std::cerr << "Square root is " << realPart << " + " << imgPart << "i\n";
    }

    if(job2.get_outputBuf())
    {
        auto str = job2.get_outputBuf()->get()->str();
        std::cerr << convertUTF16StringToUTF8String(str)  << '\n';
    }
        

    std::cerr << "Ende \n";
}
