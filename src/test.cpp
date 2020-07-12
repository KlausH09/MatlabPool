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

    Test::maxEffort = Effort::Large;
    //Test::maxEffort = Effort::Huge;

    const unsigned int nof_worker = 2;
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
            jobid[i] = pool->submit(Job(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));

        for (std::size_t i = 0; i < N; i++)
        {
            Job job = pool->wait(jobid[i]);
            worker_used[job.get_workerID()] = true;
            UnexpectOutputSize::check(1, job.peek_result().size());
            matlab::data::TypedArray<Float> result = job.peek_result()[0];
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
            jobid[i] = pool->submit(Job(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));

        for (std::size_t i = 0; i < N; i++)
        {
            Job job = pool->wait(jobid[i]);
            UnexpectOutputSize::check(1, job.peek_result().size());
            matlab::data::TypedArray<Float> result = job.peek_result()[0];
            UnexpectNumValue<Float>::check(std::sqrt(Float(i)), Float(result[0]));
        }
    });

    Test::run("increase/decrease pool size", Effort::Huge, [&]() {
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

    Test::run("restart pool", Effort::Huge, [&]() {
        std::array<JobID, 31> jobid;
        for (JobID &i : jobid)
            i = pool->submit(Job(u"pause", 0, {factory.createScalar<double>(0.01)}));

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
        JobID id = pool->submit(Job(u"sqrt", 1, {factory.createScalar<double>(0.5)}));
        pool->wait(id);
        UnexpectException<Pool::JobNotExists>::check([&]() {
            pool->wait(id);
        });
    });

    Test::run("eval", Effort::Normal, [&]() {
        std::u16string cmd{u"pwd"};
        pool->eval(cmd);

        // TODO
#ifdef MATLABPOOL_DISP_WORKER_OUTPUT
        //UnexpectCondition::Assert(!job.outputBuf.str().empty(), "empty output");
#else
        //UnexpectCondition::Assert(job.outputBuf.str().empty(), "output should be empty");
#endif
    });

    Test::run("invalid job", Effort::Normal, [&]() {
        JobID id = pool->submit(Job(u"sqrt", 1, {factory.createArray<double>({0})}));
        Job job = pool->wait(id); // TODO
    });

    Test::run("get job status", Effort::Large, [&]() {
        UnexpectOutputSize::check(0, pool->get_job_status()[0]["JobID"].getNumberOfElements());

        using Float = float;
        constexpr const std::size_t N = 31;
        std::array<JobID, N> jobid;
        for (std::size_t i = 0; i < N; i++)
            jobid[i] = pool->submit(Job(u"sqrt", 1, {factory.createScalar<Float>(Float(i))}));
        for (std::size_t i = 0; i < N; i++)
        {
            Job job = pool->wait(jobid[i]);
            UnexpectOutputSize::check(1, job.peek_result().size());
            matlab::data::TypedArray<Float> result = job.peek_result()[0];
            UnexpectNumValue<Float>::check(std::sqrt(Float(i)), Float(result[0]));

            auto status = pool->get_job_status();
            UnexpectOutputSize::check(N - i - 1, status[0]["JobID"].getNumberOfElements());
            UnexpectOutputSize::check(N - i - 1, status[0]["Status"].getNumberOfElements());
            UnexpectOutputSize::check(N - i - 1, status[0]["WorkerID"].getNumberOfElements());
        }
    });

    Test::run("cancel jobs", Effort::Large, [&]() {
        UnexpectOutputSize::check(0, pool->get_job_status()[0]["JobID"].getNumberOfElements());

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

    Test::run("get worker status", Effort::Large, [&]() {
        UnexpectOutputSize::check(0, pool->get_job_status()[0]["JobID"].getNumberOfElements());

        using Float = double;
        std::size_t size = pool->size();
        JobID id = pool->submit(Job(u"pause", 0, {factory.createScalar<Float>(Float(1))}));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        matlab::data::TypedArray<bool> ready1 = pool->get_worker_status()[0]["Ready"];
        UnexpectCondition::Assert(!(ready1[0]), "first worker should be busy");
        for (std::size_t i = 1; i < size; i++)
            UnexpectCondition::Assert(ready1[i], "other worker should sleep");
        pool->wait(id);

        std::vector<JobID> ids(size);
        for (auto &i : ids)
            i = pool->submit(Job(u"pause", 0, {factory.createScalar<Float>(Float(1))}));
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        matlab::data::TypedArray<bool> ready2 = pool->get_worker_status()[0]["Ready"];
        for (std::size_t i = 0; i < ready2.getNumberOfElements(); i++)
            UnexpectCondition::Assert(!(ready2[i]), "all workers should be busy");
        for (auto i : ids)
            pool->wait(i);
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