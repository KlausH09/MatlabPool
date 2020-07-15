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

    using Effort = TestSuite::Effort;

    TestSuite Test;
    Test.set_repeats(30);
    Test.set_maxEffort(Effort::Normal);
    //Test.set_maxEffort(Effort::Huge);

    const unsigned int nof_worker = 2;
    std::vector<std::u16string> options = {u"-nojvm", u"-nosplash"};

    auto pool_guard = std::unique_ptr<Pool>(LibLoader::createPool(nof_worker, options));
    Pool *pool = pool_guard.get();

    Test.set_postFun([&]() {
        std::size_t count_jobs = pool->get_job_status()[0]["JobID"].getNumberOfElements();
        if (count_jobs == 1)
        {
            // TODO
            return;
        }
        UnexpectCondition::Assert(0 == count_jobs, "there are jobs in the pool");
    });

    // Create  MATLAB data array factory
    matlab::data::ArrayFactory factory;

    Test.run("sqrt(i) mit i = 0,1,...,30, double", Effort::Normal, [&]() {
        using Float = double;
        constexpr const std::size_t N = 31;
        std::array<JobID, N> jobid;
        std::vector<bool> worker_used(pool->size(), false);
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));

        for (std::size_t i = 0; i < N; i++)
        {
            Job job = pool->wait(jobid[i]);
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

    Test.run("sqrt(i) mit i = 0,1,...,30, float", Effort::Normal, [&]() {
        using Float = float;
        constexpr const std::size_t N = 31;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));

        for (std::size_t i = 0; i < N; i++)
        {
            Job job = pool->wait(jobid[i]);
            UnexpectCondition::Assert(1 == job.peek_result().size(), "unexpect size of result");
            matlab::data::TypedArray<Float> result = job.peek_result()[0];
            UnexpectCondition::Assert(std::sqrt(Float(i)) == Float(result[0]), "unexpect result");
        }
    });

    Test.run("increase/decrease pool size", Effort::Huge, [&]() {
        std::array<JobID, 31> jobid;
        // increase
        for (JobID &i : jobid)
            i = pool->submit(Job(u"pause", 0, {factory.createScalar<double>(0.01)}));
        pool->resize(pool->size() + 2, options);
        for (JobID i : jobid)
            pool->wait(i);

        // decrease
        for (JobID &i : jobid)
            i = pool->submit(Job(u"pause", 0, {factory.createScalar<double>(0.01)}));
        pool->resize(pool->size() - 2, options);
        for (JobID i : jobid)
            pool->wait(i);
    });

    Test.run("restart pool", Effort::Huge, [&]() {
        std::array<JobID, 31> jobid;
        for (JobID &i : jobid)
            i = pool->submit(Job(u"pause", 0, {factory.createScalar<double>(0.01)}));

        pool_guard = std::unique_ptr<Pool>(LibLoader::createPool(nof_worker, options));
        pool = pool_guard.get();
    });

    Test.run("empty pool size", Effort::Small, [&]() {
        UnexpectException<Pool::EmptyPool>::check([&]() {
            pool->resize(0, options);
        });
    });

    Test.run("wait for undefined job", Effort::Small, [&]() {
        UnexpectException<Pool::JobNotExists>::check([&]() {
            pool->wait(9999);
        });
    });

    Test.run("wait x2 for same job", Effort::Small, [&]() {
        JobID id = pool->submit(Job(u"sqrt", 1, {factory.createScalar<double>(0.5)}));
        pool->wait(id);
        UnexpectException<Pool::JobNotExists>::check([&]() {
            pool->wait(id);
        });
    });

    Test.run("eval", Effort::Normal, [&]() {
        for (size_t i = 0; i < 100; i++)
        {
            JobEval job(u"pwd;pause(0.1)");
            pool->eval(job);

            UnexpectCondition::Assert(job.get_status() == JobEval::Status::NoError, "error in at least one worker");
            UnexpectCondition::Assert(job.get_errBuf().empty(), "error buffer should be empty");
#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
            UnexpectCondition::Assert(!job.get_outBuf().empty(), "empty output buffer");
#else
            UnexpectCondition::Assert(job.get_outBuf().empty(), "output buffer should be empty");
#endif
        }
    });

    Test.run("jobs and eval", Effort::Normal, [&]() {
        using Float = double;
        constexpr const std::size_t N = 31;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));

        JobEval job_eval(u"pwd");
        pool->eval(job_eval);

        for (std::size_t i = 0; i < N; i++)
            pool->wait(jobid[i]);
    });

    Test.run("invalid eval", Effort::Normal, [&]() {
        JobEval job(u"pwwd");
        pool->eval(job);

        UnexpectCondition::Assert(job.get_status() == JobEval::Status::Error, "there was no error during \"eval\"");
        UnexpectCondition::Assert(job.get_outBuf().empty(), "output buffer should be empty");
#ifdef MATLABPOOL_DISP_WORKER_ERROR
        UnexpectCondition::Assert(!job.get_errBuf().empty(), "empty error buffer");
#else
            UnexpectCondition::Assert(job.get_errBuf().empty(), "error buffer should be empty");
#endif
    });

    Test.run("invalid job", Effort::Normal, [&]() {
        JobID id = pool->submit(Job(u"sqqqqrt", 1, {factory.createArray<double>({0})}));
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

    Test.run("job with disp", Effort::Normal, [&]() {
        JobID id = pool->submit(Job(u"disp", 0, {factory.createCharArray("Hello World!")}));
        Job job = pool->wait(id);

        UnexpectCondition::Assert(job.get_errBuf().empty(), "error buffer should be empty");
#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
        UnexpectCondition::Assert(!job.get_outBuf().empty(), "empty output buffer");
#else
            UnexpectCondition::Assert(job.get_outBuf().empty(), "output buffer should be empty");
#endif
    });

    Test.run("get job status", Effort::Large, [&]() {
        using Float = float;
        constexpr const std::size_t N = 31;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));
        for (std::size_t i = 0; i < N; i++)
        {
            Job job = pool->wait(jobid[i]);
            UnexpectCondition::Assert(1 == job.peek_result().size(), "unexpect size of result");
            matlab::data::TypedArray<Float> result = job.peek_result()[0];
            UnexpectCondition::Assert(std::sqrt(Float(i)) == Float(result[0]), "unexpect result");

            auto status = pool->get_job_status();
            UnexpectCondition::Assert(N - i - 1 == status[0]["JobID"].getNumberOfElements(), "unexpect size for \"JobID\" field");
            UnexpectCondition::Assert(N - i - 1 == status[0]["Status"].getNumberOfElements(), "unexpect size for \"Status\" field");
            UnexpectCondition::Assert(N - i - 1 == status[0]["WorkerID"].getNumberOfElements(), "unexpect size for \"WorkerID\" field");
        }
    });

    Test.run("cancel jobs", Effort::Normal, [&]() {
        using Float = double;
        constexpr const std::size_t N = 31;
        Float pause = 0.05;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job(u"pause", 0, {factory.createScalar<Float>(pause)}));

        std::this_thread::sleep_for(std::chrono::milliseconds(int(pause / 2 * N) * 1000));

        for (std::size_t i = 0; i < N; i++)
            pool->cancel(jobid[N - i - 1]);

        auto status = pool->get_job_status();
        matlab::data::TypedArray<JobID> jobid_field = status[0]["JobID"];
        using StatusType = std::underlying_type<Job::Status>::type;
        matlab::data::TypedArray<StatusType> status_field = status[0]["Status"];

        if (jobid_field.getNumberOfElements() == 0)
            return;
        else if (jobid_field.getNumberOfElements() == 1)
        {
            if (static_cast<Job::Status>(StatusType(status_field[0])) != Job::Status::AssignToWorker)
                UnexpectCondition("unexpect jobs status");
        }
        else
            UnexpectCondition("unexpect jobs in pool");
    });

    Test.run("get worker status", Effort::Large, [&]() {
        using Float = double;
        JobID id = pool->submit(Job(u"pause", 0, {factory.createScalar<Float>(Float(1))}));
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