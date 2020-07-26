#ifndef MATLABPOOL_ASSERT_HPP
#define MATLABPOOL_ASSERT_HPP

#include <iostream>

#ifdef NDEBUG
#define MATLABPOOL_ASSERT(condition) ((void)0)
#define MATLABPOOL_ERROR(msg) ((void)0)
#else
#define MATLABPOOL_ASSERT(condition) (safe_assert((condition), #condition, \
                                                  __FILE__, __LINE__, __func__))
#define MATLABPOOL_ERROR(msg) (safe_error((msg), __FILE__, __LINE__, __func__))
#endif

namespace MatlabPool
{

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

} // namespace MatlabPool

#endif