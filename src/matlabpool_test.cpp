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

void run_tests()
{
    using Effort = TestSuite::Effort;
    using namespace MatlabPool;

    constexpr const std::size_t N = 50; // count of jobs in a single test

    TestSuite test;
    test.set_countEval(Effort::Small, 100);
    test.set_countEval(Effort::Normal, 30);
    test.set_countEval(Effort::Large, 5);
    test.set_countEval(Effort::Huge, 1);

    const unsigned int nof_worker = 2;
    std::vector<std::u16string> options = {u"-nojvm", u"-nosplash"};

    auto pool = std::unique_ptr<Pool>(PoolLibLoader::createPool(nof_worker, options));

    test.set_postFun([&]() {
        using StatusType = std::underlying_type<JobFeval::Status>::type;
        auto status = pool->get_job_status();
        std::size_t count_jobs = status[0]["JobID"].getNumberOfElements();
        if (count_jobs == 1)
        {
            matlab::data::TypedArray<StatusType> jobStatus = status[0]["Status"];
            JobFeval::Status jobStatusVal = static_cast<JobFeval::Status>(StatusType(jobStatus[0]));
            UnexpectCondition::Assert(jobStatusVal == JobFeval::Status::Canceled,
                                      "there are a job in the pool");
            return;
        }
        UnexpectCondition::Assert(0 == count_jobs, "there are jobs in the pool");
    });

    // Create  MATLAB data array factory
    matlab::data::ArrayFactory factory;

    test.run("sqrt(i) with i = 0,1,...,N, double", Effort::Normal, [&]() {
        using Float = double;
        std::array<JobID, N> jobid;
        std::vector<bool> worker_used(pool->size(), false);
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(JobFeval(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));

        for (std::size_t i = 0; i < N; i++)
        {
            JobFeval job = pool->wait(jobid[i]);
            worker_used[job.get_workerID()] = true;
            UnexpectCondition::Assert(1 == job.peek_result().size(), "unexpect size of result");
            matlab::data::TypedArray<Float> result = job.peek_result()[0];
            UnexpectCondition::Assert(std::sqrt(Float(i)) == Float(result[0]), "unexpect result");
            UnexpectCondition::Assert(job.get_outBuf().empty(), "output buffer should be empty");
            UnexpectCondition::Assert(job.get_errBuf().empty(), "error buffer should be empty");
        }
        for (auto e : worker_used)
            UnexpectCondition::Assert(e, "unused worker");
    });

    test.run("sqrt(i) with i = 0,1,...,N, float", Effort::Normal, [&]() {
        using Float = float;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(JobFeval(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));

        for (std::size_t i = 0; i < N; i++)
        {
            JobFeval job = pool->wait(jobid[i]);
            UnexpectCondition::Assert(1 == job.peek_result().size(), "unexpect size of result");
            matlab::data::TypedArray<Float> result = job.peek_result()[0];
            UnexpectCondition::Assert(std::sqrt(Float(i)) == Float(result[0]), "unexpect result");
        }
    });

    test.run("increase/decrease pool size", Effort::Huge, [&]() {
        std::array<JobID, N> jobid;
        // increase
        for (JobID &i : jobid)
            i = pool->submit(JobFeval(u"pause", 0, {factory.createScalar<double>(0.01)}));
        pool->resize(pool->size() + 2, options);
        for (JobID i : jobid)
            pool->wait(i);

        // decrease
        for (JobID &i : jobid)
            i = pool->submit(JobFeval(u"pause", 0, {factory.createScalar<double>(0.01)}));
        pool->resize(pool->size() - 2, options);
        for (JobID i : jobid)
            pool->wait(i);
    });

    test.run("restart pool", Effort::Huge, [&]() {
        std::array<JobID, N> jobid;
        for (JobID &i : jobid)
            i = pool->submit(JobFeval(u"pause", 0, {factory.createScalar<double>(0.01)}));

        pool = std::unique_ptr<Pool>(PoolLibLoader::createPool(nof_worker, options));
    });

    test.run("empty pool size", Effort::Small, [&]() {
        UnexpectException<Pool::EmptyPool>::check([&]() {
            pool->resize(0, options);
        });
    });

    test.run("wait for undefined job", Effort::Small, [&]() {
        UnexpectException<Pool::JobNotExists>::check([&]() {
            pool->wait(JobID(-1));
        });
    });

    test.run("wait x2 for same job", Effort::Small, [&]() {
        JobID id = pool->submit(JobFeval(u"sqrt", 1, {factory.createScalar<double>(0.5)}));
        pool->wait(id);
        UnexpectException<Pool::JobNotExists>::check([&]() {
            pool->wait(id);
        });
    });

    test.run("eval", Effort::Normal, [&]() {
        JobEval job(u"pwd");
        pool->eval(job);

        UnexpectCondition::Assert(job.get_status() == JobEval::Status::NoError, "error in at least one worker");
        UnexpectCondition::Assert(job.get_errBuf().empty(), "error buffer should be empty");

#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
        UnexpectCondition::Assert(!job.get_outBuf().empty(), "empty output buffer");
#else
            UnexpectCondition::Assert(job.get_outBuf().empty(), "output buffer should be empty");
#endif
    });

    test.run("jobs and eval", Effort::Normal, [&]() {
        using Float = double;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(JobFeval(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));

        JobEval job_eval(u"pwd");
        pool->eval(job_eval);

        for (std::size_t i = 0; i < N; i++)
            pool->wait(jobid[i]);
    });

    test.run("invalid eval", Effort::Normal, [&]() {
        JobEval job(u"pwwd");

        UnexpectException<JobBase::ExecutionError>::check([&]() {
            pool->eval(job);
        });

        UnexpectCondition::Assert(job.get_status() == JobEval::Status::Error, "there was no error during \"eval\"");
        UnexpectCondition::Assert(job.get_outBuf().empty(), "output buffer should be empty");
#ifdef MATLABPOOL_DISP_WORKER_ERROR
        UnexpectCondition::Assert(!job.get_errBuf().empty(), "empty error buffer");
#else
            UnexpectCondition::Assert(job.get_errBuf().empty(), "error buffer should be empty");
#endif
    });

    test.run("invalid job", Effort::Normal, [&]() {
        JobID id = pool->submit(JobFeval(u"sqqqqrt", 1, {factory.createArray<double>({0})}));
        auto job = pool->wait(id);

        UnexpectException<JobBase::ExecutionError>::check([&]() {
            job.pop_result();
        });
#ifdef MATLABPOOL_DISP_WORKER_ERROR
        UnexpectCondition::Assert(!job.get_errBuf().empty(), "empty error buffer");
#else
            UnexpectCondition::Assert(job.get_errBuf().empty(), "error buffer should be empty");
#endif
    });

    test.run("job with disp", Effort::Normal, [&]() {
        JobID id = pool->submit(JobFeval(u"disp", 0, {factory.createCharArray("Hello World!")}));
        JobFeval job = pool->wait(id);

        UnexpectCondition::Assert(job.get_errBuf().empty(), "error buffer should be empty");
#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
        UnexpectCondition::Assert(!job.get_outBuf().empty(), "empty output buffer");
#else
            UnexpectCondition::Assert(job.get_outBuf().empty(), "output buffer should be empty");
#endif
    });

    test.run("get job status", Effort::Normal, [&]() {
        using Float = float;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(JobFeval(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));
        for (std::size_t i = 0; i < N; i++)
        {
            JobFeval job = pool->wait(jobid[i]);
            UnexpectCondition::Assert(1 == job.peek_result().size(), "unexpect size of result");
            matlab::data::TypedArray<Float> result = job.peek_result()[0];
            UnexpectCondition::Assert(std::sqrt(Float(i)) == Float(result[0]), "unexpect result");

            auto status = pool->get_job_status();
            UnexpectCondition::Assert(N - i - 1 == status[0]["JobID"].getNumberOfElements(), "unexpect size for \"JobID\" field");
            UnexpectCondition::Assert(N - i - 1 == status[0]["Status"].getNumberOfElements(), "unexpect size for \"Status\" field");
            UnexpectCondition::Assert(N - i - 1 == status[0]["WorkerID"].getNumberOfElements(), "unexpect size for \"WorkerID\" field");
        }
    });

    test.run("cancel all jobs", Effort::Normal, [&]() {
        using Float = float;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(JobFeval(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));

        pool->clear();
    });

    test.run("cancel jobs", Effort::Normal, [&]() {
        using Float = double;
        Float pause = 0.05;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(JobFeval(u"pause", 0, {factory.createScalar<Float>(pause)}));

        for (std::size_t i = 0; i < N; i++)
            pool->cancel(jobid[N - i - 1]);

        auto status = pool->get_job_status();
        matlab::data::TypedArray<JobID> jobid_field = status[0]["JobID"];
        using StatusType = std::underlying_type<JobFeval::Status>::type;
        matlab::data::TypedArray<StatusType> status_field = status[0]["Status"];

        if (jobid_field.getNumberOfElements() == 0)
            return;
        else if (jobid_field.getNumberOfElements() == 1)
        {
            if (static_cast<JobFeval::Status>(StatusType(status_field[0])) != JobFeval::Status::AssignToWorker)
                UnexpectCondition("unexpect jobs status");
        }
        else
            UnexpectCondition("unexpect jobs in pool");
    });

    test.run("get worker status", Effort::Large, [&]() {
        using Float = double;
        JobID id = pool->submit(JobFeval(u"pause", 0, {factory.createScalar<Float>(Float(1))}));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        matlab::data::TypedArray<bool> ready1 = pool->get_worker_status()[0]["Ready"];
        std::size_t count_busy = 0;
        for (bool b : ready1)
            if (!b)
                ++count_busy;

        UnexpectCondition::Assert(count_busy == 1, "only on worker should be busy");
        pool->wait(id);
    });
}

int main()
{
    try
    {
        run_tests();
    }
    catch (const std::exception &e)
    {
        std::cout << "Abort test: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cout << "Abort test" << std::endl;
        return 1;
    }
    return 0;
}