#include <iostream>

#include "MatlabPool.hpp"
#include "MatlabDataArray.hpp"

int main()
{
    MatlabPool::Pool pool(2);

    // Create  MATLAB data array factory
    matlab::data::ArrayFactory factory;

    MatlabPool::Job job1(u"sqrt", 1, {factory.createArray<double>({1, 4}, {-2.0, 2.0, 6.0, 8.0})});
    MatlabPool::Job job2(u"pause", 0, {factory.createArray<double>({1, 1}, {2.0})});

    std::size_t id1 = pool.submit(std::move(job1));
    std::size_t id2 = pool.submit(std::move(job2));

    std::vector<matlab::data::Array> result1 = pool.wait(id1);
    std::vector<matlab::data::Array> result2 = pool.wait(id2);

    matlab::data::TypedArray<std::complex<double>> results = result1[0];
    for (auto r : results)
    {
        double realPart = r.real();
        double imgPart = r.imag();
        std::cerr << "Square root of " << " is " << realPart << " + " << imgPart << "i\n";
    }
    
    std::cerr << "Ende\n";
    return 0;
}
