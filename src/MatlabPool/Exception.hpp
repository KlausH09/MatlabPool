#ifndef MATLABPOOL_EXCEPTION_HPP
#define MATLABPOOL_EXCEPTION_HPP

#include <exception>

namespace MatlabPool
{
    // Base class for exceptions. The purpose of this class and
    // the extra function "identifier" is to generate nice 
    // error messages in Matlab. Because Matlab exception have 
    // a "identifier" field for uniqueness. Of course it is
    // possible to use the same identifier (e.g. "MatlabPool")
    // for all exceptions which can be thrown by this library,
    // however, that makes it difficult to catch specific errors.
    class Exception : public std::exception
    {
    public:
        virtual const char *identifier() const noexcept = 0;
    };

} // namespace MatlabPool

#endif