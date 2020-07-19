#ifndef MATLABPOOL_EXCEPTION_HPP
#define MATLABPOOL_EXCEPTION_HPP

#include <exception>

namespace MatlabPool
{
    class Exception : public std::exception
    {
    public:
        virtual const char *identifier() const noexcept = 0;
    };

} // namespace MatlabPool

#endif