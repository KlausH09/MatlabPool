#include <iostream>
#include <exception>

#include "MatlabPool.hpp"
#include "TestSuite.hpp"

#include <queue>
#include <cmath>

// TODO test mit valgrind
// TODO test C0 ueberdeckung

int main()
{
    using namespace MatlabPool;
    using namespace TestSuite;

    // Start MATLAB engine synchronously
    const std::size_t nof_worker = 2;
    std::vector<std::u16string> options = {u"-nojvm", u"-nosplash"};
    auto pool_guard = std::unique_ptr<Pool>(LibLoader::createPool(nof_worker, options));
    Pool *pool = pool_guard.get();

    // Create  MATLAB data array factory
    matlab::data::ArrayFactory factory;

    Test::run("sqrt(i) mit i = 0,1,...,30, double", [&]() {
        using Float = double;
        constexpr const std::size_t N = 31;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job("sqrt", 1, {factory.createArray<Float>({1}, {Float(i)})}));

        for (std::size_t i = 0; i < N; i++)
        {
            Job job = pool->wait(jobid[i]);
            UnexpectOutputSize::check(1,job.result.size());
            matlab::data::TypedArray<Float> result = job.result[0];
            UnexpectNumValue<Float>::check(std::sqrt(Float(i)), Float(result[0]));
        }
    });

    Test::run("sqrt(i) mit i = 0,1,...,30, float", [&]() {
        using Float = float;
        constexpr const std::size_t N = 31;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job("sqrt", 1, {factory.createArray<Float>({1}, {Float(i)})}));

        for (std::size_t i = 0; i < N; i++)
        {
            Job job = pool->wait(jobid[i]);
            UnexpectOutputSize::check(1,job.result.size());
            matlab::data::TypedArray<Float> result = job.result[0];
            UnexpectNumValue<Float>::check(std::sqrt(Float(i)), Float(result[0]));
        }
    });

    Test::run("wait for undefined job", [&]() {
        UnexpectException<Pool::JobNotExists>::check([&](){
            pool->wait(9999);
        });
    });

    Test::run("wait x2 for same job", [&]() {
        JobID id = pool->submit(Job("sqrt", 1, {factory.createArray<double>({1}, {0.5})}));
        pool->wait(id);
        UnexpectException<Pool::JobNotExists>::check([&](){
            pool->wait(id);
        });
    });

    Test::run("restart pool", [&]() {
        throw std::runtime_error("TODO");
        pool_guard = std::unique_ptr<Pool>(LibLoader::createPool(nof_worker, options));
        pool = pool_guard.get();
    }); 

    Test::run("invalid job", [&]() {
        JobID id = pool->submit(Job("sqrt", 1, {factory.createArray<double>({0}, {})}));
        pool->wait(id); // TODO
    }); 



}
