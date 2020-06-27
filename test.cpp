#include <iostream>
#include <algorithm>


#include "MatlabPool.hpp"


int main()
{
    using namespace MatlabPool;
    
    // Start MATLAB engine synchronously
    std::vector<std::u16string> options = {u"-nojvm", u"-nosplash"};
    Pool* pool = LibLoader::createPool(2, options);

    // Create  MATLAB data array factory
    matlab::data::ArrayFactory factory;

    std::array<Job,2> jobs = {
        Job("sqrt", 1, {factory.createArray<double>({1, 4}, {-2.0, 2.0, 6.0, 8.0})}),
        Job("disp", 0, {factory.createArray<double>({1, 1}, {2.0})})};

    std::vector<JobID> id(jobs.size());

    for (std::size_t i = 0; i < jobs.size(); i++)
        id[i] = pool->submit(std::move(jobs[i]));
    

    for (std::size_t i = 0; i < jobs.size(); i++)
        jobs[i] = pool->wait(id[i]);

    // Display results
    matlab::data::TypedArray<std::complex<double>> results = jobs[0].result[0];

    for (auto r : results)
    {
        double realPart = r.real();
        double imgPart = r.imag();
        std::cout << "Square root is " << realPart << " + " << imgPart << "i" << std::endl;
    }

    if (jobs[1].get_outputBuf())
    {
        //auto str = jobs[1].get_outputBuf()->get()->str();
        //std::wcout << str << std::endl;
    }

    std::cout << "Ende" << std::endl;

    delete pool;
}
