#include <iostream>
#include <exception>

#include "MatlabPool.hpp"
#include "TestSuite.hpp"

#include <queue>
#include <cmath>

// TODO test mit valgrind
// TODO test C0 ueberdeckung

//#define INTENSIVE_TEST

void run_test()
{
    using namespace MatlabPool;
    using namespace TestSuite;

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
        std::vector<bool> worker_used(pool->size(), false);
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job_feval(u"sqrt", 1, {factory.createArray<Float>({1}, {Float(i)})}));

        for (std::size_t i = 0; i < N; i++)
        {
            Job_feval job = pool->wait(jobid[i]);
            worker_used[job.get_workerID()] = true;
            UnexpectOutputSize::check(1, job.result.size());
            matlab::data::TypedArray<Float> result = job.result[0];
            UnexpectNumValue<Float>::check(std::sqrt(Float(i)), Float(result[0]));
        }
        for (auto e : worker_used)
            if (!e)
                std::runtime_error("unused workers");
    });

    Test::run("sqrt(i) mit i = 0,1,...,30, float", [&]() {
        using Float = float;
        constexpr const std::size_t N = 31;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job_feval(u"sqrt", 1, {factory.createArray<Float>({1}, {Float(i)})}));

        for (std::size_t i = 0; i < N; i++)
        {
            Job_feval job = pool->wait(jobid[i]);
            UnexpectOutputSize::check(1, job.result.size());
            matlab::data::TypedArray<Float> result = job.result[0];
            UnexpectNumValue<Float>::check(std::sqrt(Float(i)), Float(result[0]));
        }
    });

#ifdef INTENSIVE_TEST

    Test::run("increase pool size", [&]() {
        std::array<JobID, 31> jobid;
        for (JobID &i : jobid)
            i = pool->submit(Job_feval(u"pause", 0, {factory.createArray<double>({1}, {0.01})}));

        pool->resize(pool->size() + 2, options);

        for (JobID i : jobid)
            pool->wait(i);
    });

    Test::run("decrease pool size", [&]() {
        std::array<JobID, 31> jobid;
        for (JobID &i : jobid)
            i = pool->submit(Job_feval(u"pause", 0, {factory.createArray<double>({1}, {0.01})}));

        pool->resize(pool->size() - 2, options);

        for (JobID i : jobid)
            pool->wait(i);
    });

    Test::run("restart pool", [&]() {
        std::array<JobID, 31> jobid;
        for (JobID &i : jobid)
            i = pool->submit(Job_feval(u"pause", 0, {factory.createArray<double>({1}, {0.01})}));

        pool_guard = std::unique_ptr<Pool>(LibLoader::createPool(nof_worker, options));
        pool = pool_guard.get();
    });

#endif

    Test::run("empty pool size", [&]() {
        UnexpectException<Pool::EmptyPool>::check([&]() {
            pool->resize(0, options);
        });
    });

    Test::run("wait for undefined job", [&]() {
        UnexpectException<Pool::JobNotExists>::check([&]() {
            pool->wait(9999);
        });
    });

    Test::run("wait x2 for same job", [&]() {
        JobID id = pool->submit(Job_feval(u"sqrt", 1, {factory.createArray<double>({1}, {0.5})}));
        pool->wait(id);
        UnexpectException<Pool::JobNotExists>::check([&]() {
            pool->wait(id);
        });
    });

    Test::run("eval", [&]() {
        Job job(u"pwd");
        pool->eval(job);
#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
        UnexpectCondition::Assert(!job.outputBuf.str().empty(), "empty output");
#else
            UnexpectCondition::Assert(job.outputBuf.str().empty(), "output should be empty");
#endif
    });

    Test::run("invalid job", [&]() {
        JobID id = pool->submit(Job_feval(u"sqrt", 1, {factory.createArray<double>({0}, {})}));
        Job job = pool->wait(id); // TODO
    });

    Test::run("get job status", [&]() {
        using Float = float;
        constexpr const std::size_t N = 31;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job_feval(u"sqrt", 1, {factory.createArray<Float>({1}, {Float(i)})}));
        for (std::size_t i = 0; i < N; i++)
        {
            Job_feval job = pool->wait(jobid[i]);
            UnexpectOutputSize::check(1, job.result.size());
            matlab::data::TypedArray<Float> result = job.result[0];
            UnexpectNumValue<Float>::check(std::sqrt(Float(i)), Float(result[0]));

            auto status = pool->get_job_status();
            UnexpectOutputSize::check(N - i - 1, status[0]["JobID"].getNumberOfElements());
            UnexpectOutputSize::check(N - i - 1, status[0]["Status"].getNumberOfElements());
            UnexpectOutputSize::check(N - i - 1, status[0]["WorkerID"].getNumberOfElements());
        }
    });
}

int main()
{
    try
    {
        run_test();
    }
    catch (const std::exception &e)
    {
        std::cout << "Abort test: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Abort test" << std::endl;
    }
    return 0;
}