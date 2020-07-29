#ifndef MATLABPOOL_LIB_HPP
#define MATLABPOOL_LIB_HPP

#ifdef _WIN32
#ifdef WIN_EXPORT
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __declspec(dllimport)
#endif
#else
#define EXPORTED
#endif

#include "MatlabPool/Pool.hpp"

extern "C"
{
    EXPORTED MatlabPool::Pool* construct(unsigned int n, const std::vector<std::u16string> &options);
}

#endif