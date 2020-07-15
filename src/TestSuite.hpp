#ifndef TESTSUITE_HPP
#define TESTSUITE_HPP

#include <iostream>
#include <iomanip>
#include <exception>
#include <string>
#include <functional>
#include <chrono>

class TestSuite
{
    TestSuite(const TestSuite &) = delete;
    TestSuite &operator=(const TestSuite &) = delete;

public:
    enum class Effort
    {
        Small,
        Normal,
        Large,
        Huge
    };
    class TestSuiteException : public std::exception
    {
    };

public:
    TestSuite() : count(0), failed(0),
                  count_repeats(0),
                  maxEffort(Effort::Normal),
                  prefun([]() {}), postfun([]() {})
    {
    }

    void set_preFun(std::function<void()> fun)
    {
        prefun = std::move(fun);
    }

    void set_postFun(std::function<void()> fun)
    {
        postfun = std::move(fun);
    }

    void set_repeats(std::size_t repeats)
    {
        count_repeats = repeats;
    }

    void set_maxEffort(Effort val)
    {
        maxEffort = val;
    }

    template <typename T>
    void run(const std::string &description, Effort effort, T &&operation)
    {
        if (effort > maxEffort)
            return;
        std::cout << "Test " << std::setw(2) << std::right << ++count << ": ";
        std::cout << std::setw(50) << std::left << description << " ";
        try
        {
            ++failed;
            tic();
            for (std::size_t i = 0; i < count_repeats + 1; i++)
            {
                prefun();
                operation();
                postfun();
            }
            std::cout << "ok, " << std::setprecision(4) << toc() << " sec" << std::endl;
            --failed;
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
        }
        catch (...)
        {
            std::cout << "unknown exception" << std::endl;
        }
    }

private:
    inline void tic()
    {
        t0 = std::chrono::high_resolution_clock::now();
    }
    inline float toc()
    {
        using namespace std::chrono;
        elapsed = high_resolution_clock::now() - t0;
        return duration<float, seconds::period>(elapsed).count();
    }

private:
    std::size_t count;
    std::size_t failed;

    std::size_t count_repeats;
    Effort maxEffort;

    std::function<void()> prefun;
    std::function<void()> postfun;

    std::chrono::high_resolution_clock::time_point t0;
    std::chrono::high_resolution_clock::duration elapsed;
};

class UnexpectCondition : public TestSuite::TestSuiteException
{
public:
    UnexpectCondition(const char *msg_)
    {
        std::ostringstream os;
        os << "assertion failed: ";
        if (msg_)
            os << " \"" << msg_ << "\"";
        msg = os.str();
    }
    const char *what() const noexcept override
    {
        return msg.c_str();
    }
    static void Assert(bool cond, const char *msg_ = nullptr)
    {
        if (!cond)
            throw UnexpectCondition(msg_);
    }

private:
    std::string msg;
};

template <typename Error>
class UnexpectException : public TestSuite::TestSuiteException
{
public:
    UnexpectException()
    {
        std::ostringstream os;
        os << "expect exception but there wasn't one";
        msg = os.str();
    }
    UnexpectException(const std::exception &e)
    {
        std::ostringstream os;
        os << "expect exception what(): " << e.what();
        msg = os.str();
    }
    const char *what() const noexcept override
    {
        return msg.c_str();
    }

    template <typename T>
    static void check(T &&operation)
    {
        try
        {
            operation();
        }
        catch (const Error &e)
        {
            return;
        }
        catch (const std::exception &e)
        {
            throw UnexpectException(e);
        }
        throw UnexpectException();
    }

private:
    std::string msg;
};

#endif