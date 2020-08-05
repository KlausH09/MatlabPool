#ifndef TESTSUITE_HPP
#define TESTSUITE_HPP

#include <iostream>
#include <iomanip>
#include <exception>
#include <string>
#include <functional>
#include <chrono>
#include <map>

// class for testing
class TestSuite
{
    static constexpr std::size_t header_length = 74;

public:
    // to describe the effort of a test 
    enum class Effort
    {
        Small,
        Normal,
        Large,
        Huge
    };

public:
    TestSuite(const TestSuite &) = delete;
    TestSuite &operator=(const TestSuite &) = delete;

    TestSuite(const std::string &header = "Start Test") : count(0), failed(0),
        prefun([]() {}), postfun([]() {})
    {
        std::size_t len1 = 0;
        std::size_t len2 = 0;
        if (header.size() + 2 < header_length)
        {
            len1 = (header_length - (header.size() + 2)) / 2;
            len2 = header_length - (header.size() + 2 + len1);
        }
        std::cout << std::string(len1, '=') << " " << header << " "
            << std::string(len2, '=') << std::endl;
    }
    ~TestSuite()
    {
        std::cout << std::string(header_length, '=') << std::endl;
    }

    // set a function which runs before every test
    void set_preFun(std::function<void()> fun)
    {
        prefun = std::move(fun);
    }

    // set a function which runs after every test
    void set_postFun(std::function<void()> fun)
    {
        postfun = std::move(fun);
    }

    // run test multiple times depending on their effort
    void set_countEval(Effort effort, std::size_t count)
    {
        count_evaluations[effort] = count;
    }

    // count of failed test
    std::size_t get_failed() const noexcept
    {
        return failed;
    }

    // run a test
    template <typename T>
    void run(const std::string &description, Effort effort, T &&operation)
    {
        auto flags = std::cout.flags();
        std::cout << "Test " << std::setw(3) << std::right << ++count << ": ";
        std::cout << std::setw(50) << std::left << description << " ";

        try
        {
            ++failed;
            tic();
            for (std::size_t i = 0; i < get_countEval(effort); i++)
            {
                prefun();
                operation();
                postfun();
            }

            std::cout.setf(std::ios::fixed | std::ios_base::right);
            std::cout << "ok " << std::setw(6) << std::setprecision(2) << toc()
                << " sec" << std::endl;
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
        std::cout.flags(flags);
    }

private:
    std::size_t get_countEval(Effort effort) const
    {
        auto it = count_evaluations.find(effort);

        if (it == count_evaluations.end())
            return 1;
        else
            return it->second;
    }

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

    std::map<Effort, std::size_t> count_evaluations;

    std::function<void()> prefun;
    std::function<void()> postfun;

    std::chrono::high_resolution_clock::time_point t0;
    std::chrono::high_resolution_clock::duration elapsed;
};

class TestSuiteException : public std::exception
{
};

class UnexpectCondition : public TestSuiteException
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
private:
    std::string msg;
};

template <typename Error>
class UnexpectException : public TestSuiteException
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


void Assert(bool cond, const char *msg = nullptr)
{
    if (!cond)
        throw UnexpectCondition(msg);
}

#endif