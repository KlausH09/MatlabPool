#include <iostream>
#include <exception>

#include "MatlabPool.hpp"
#include "TestSuite.hpp"

#include <queue>
#include <cmath>
#include <chrono>
#include <thread>

// TODO test mit valgrind
// TODO test C0 ueberdeckung

void run_test()
{
    using namespace MatlabPool;
    using namespace TestSuite;
    using Effort = Test::Effort;

    //Test::maxEffort = Effort::Large;
    Test::maxEffort = Effort::Huge;

    const std::size_t nof_worker = 2;
    std::vector<std::u16string> options = {u"-nojvm", u"-nosplash"};

    auto pool_guard = std::unique_ptr<Pool>(LibLoader::createPool(nof_worker, options));
    Pool *pool = pool_guard.get();

    // Create  MATLAB data array factory
    matlab::data::ArrayFactory factory;

    Test::run("sqrt(i) mit i = 0,1,...,30, double", Effort::Normal, [&]() {
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
            UnexpectCondition::Assert(e, "unused worker");
    });

    Test::run("sqrt(i) mit i = 0,1,...,30, float", Effort::Normal, [&]() {
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

    Test::run("increase pool size", Effort::Huge, [&]() {
        std::array<JobID, 31> jobid;
        for (JobID &i : jobid)
            i = pool->submit(Job_feval(u"pause", 0, {factory.createArray<double>({1}, {0.01})}));

        pool->resize(pool->size() + 2, options);

        for (JobID i : jobid)
            pool->wait(i);
    });

    Test::run("decrease pool size", Effort::Small, [&]() {
        std::array<JobID, 31> jobid;
        for (JobID &i : jobid)
            i = pool->submit(Job_feval(u"pause", 0, {factory.createArray<double>({1}, {0.01})}));

        pool->resize(pool->size() - 2, options);

        for (JobID i : jobid)
            pool->wait(i);
    });

    Test::run("restart pool", Effort::Huge, [&]() {
        std::array<JobID, 31> jobid;
        for (JobID &i : jobid)
            i = pool->submit(Job_feval(u"pause", 0, {factory.createArray<double>({1}, {0.01})}));

        pool_guard = std::unique_ptr<Pool>(LibLoader::createPool(nof_worker, options));
        pool = pool_guard.get();
    });

    Test::run("empty pool size", Effort::Small, [&]() {
        UnexpectException<Pool::EmptyPool>::check([&]() {
            pool->resize(0, options);
        });
    });

    Test::run("wait for undefined job", Effort::Small, [&]() {
        UnexpectException<Pool::JobNotExists>::check([&]() {
            pool->wait(9999);
        });
    });

    Test::run("wait x2 for same job", Effort::Small, [&]() {
        JobID id = pool->submit(Job_feval(u"sqrt", 1, {factory.createArray<double>({1}, {0.5})}));
        pool->wait(id);
        UnexpectException<Pool::JobNotExists>::check([&]() {
            pool->wait(id);
        });
    });

    Test::run("eval", Effort::Normal, [&]() {
        Job job(u"pwd");
        pool->eval(job);
#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
        UnexpectCondition::Assert(!job.outputBuf.str().empty(), "empty output");
#else
            UnexpectCondition::Assert(job.outputBuf.str().empty(), "output should be empty");
#endif
    });

    Test::run("invalid job", Effort::Normal, [&]() {
        JobID id = pool->submit(Job_feval(u"sqrt", 1, {factory.createArray<double>({0}, {})}));
        Job job = pool->wait(id); // TODO
    });

    Test::run("get job status", Effort::Large, [&]() {
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

    Test::run("cancel jobs", Effort::Large, [&]() {
        using Float = double;
        constexpr const std::size_t N = 31;
        Float pause = 0.05;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job_feval(u"pause", 1, {factory.createArray<Float>({1}, {pause})}));

        std::this_thread::sleep_for(std::chrono::milliseconds(int(pause / 2 * N) * 1000));

        for (std::size_t i = 0; i < N; i++)
            pool->cancel(jobid[N - i - 1]);

        auto status = pool->get_job_status();
        UnexpectOutputSize::check(0, status[0]["JobID"].getNumberOfElements());
    });
}

int main()
{
    std::cout << "======================== Start Test ==========================" << std::endl;
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
    std::cout << "========================= End Test ===========================" << std::endl;
    return 0;
}