#ifndef ASSERT_HPP
#define ASSERT_HPP
// TODO 
#include <iostream>

#ifdef NDEBUG
#define ASSERT (condition)((void)0)
#define ERROR (msg)((void)0)
#else
#define ASSERT(condition)(safe_assert((condition), #condition, \
                                       __FILE__, __LINE__, __func__))
#define ERROR(msg)(safe_error((msg), __FILE__, __LINE__, __func__))
#endif

inline void safe_assert(bool condition, const char *condition_text,
                        const char *filename, int line, const char *function) noexcept
{
    if (!condition)
    {
        try
        {
            std ::cerr << " Assertion failed : " << condition_text << ", file " << filename << ", line " << line << ", function " << function << std ::endl;
        }
        catch (...)
        {
        }
        std ::abort();
    }
}

inline void safe_error(const char *msg, const char *filename, int line, const char *function) noexcept
{
    try
    {
        std ::cerr << " an error has occurred : " << msg << ", file " << filename << ", line " << line << ", function " << function << std ::endl;
    }
    catch (...)
    {
    }
    std ::abort();
}
#endif