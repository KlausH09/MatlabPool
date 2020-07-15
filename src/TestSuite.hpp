#ifndef TESTSUITE_HPP
#define TESTSUITE_HPP

#include <iostream>
#include <iomanip>
#include <exception>
#include <string>
#include <functional>

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
            prefun();
            operation();
            postfun();
            std::cout << "ok" << std::endl;
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
    std::size_t count;
    std::size_t failed;
    Effort maxEffort;

    std::function<void()> prefun;
    std::function<void()> postfun;
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