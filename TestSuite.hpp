#ifndef TESTSUITE_HPP
#define TESTSUITE_HPP

#include <iostream>
#include <iomanip>
#include <exception>
#include <string>

namespace TestSuite
{
    class Test
    {
        inline static std::size_t count = 1;

    public:
        template <typename T>
        static void run(const std::string &description, T &&operation)
        {
            std::cout << "Test " << std::setw(2) << std::right << count++ << ": ";
            std::cout << std::setw(50) << std::left << description << " ";
            try
            {
                operation();
                std::cout << "ok" << std::endl;
            }
            catch (std::exception &e)
            {
                std::cout << "exception: " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cout << "unknown exception" << std::endl;
            }
        }
    };

    class TestSuiteException : public std::exception {};
    
    class UnexpectOutputSize : public TestSuiteException
    {
    public:
        UnexpectOutputSize(std::size_t expect, std::size_t size)
        {
            std::ostringstream os;
            os << "expect output size = " << expect << ", output size = " << size;
            msg = os.str();
        }
        const char *what() const noexcept override
        {
            return msg.c_str();
        }
        static void check(std::size_t expect, std::size_t size)
        {
            if (expect != size)
                throw UnexpectOutputSize(expect, size);
        }

    private:
        std::string msg;
    };

    template <typename T>
    class UnexpectNumValue : public TestSuiteException
    {
    public:
        UnexpectNumValue(T expect, T value)
        {
            std::ostringstream os;
            os << "expect value = " << expect << ", value = " << value;
            msg = os.str();
        }
        const char *what() const noexcept override
        {
            return msg.c_str();
        }
        static void check(T expect, T val, T tol = T(0))
        {
            T diff = expect > val ? expect - val : val - expect;
            if (diff > tol)
                throw UnexpectNumValue(expect, val);
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
            os << "expect exception but there was not one";
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
} // namespace TestSuite

#endif