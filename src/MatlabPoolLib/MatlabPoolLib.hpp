#ifndef MATLABPOOL_LIB_HPP
#define MATLABPOOL_LIB_HPP

#include "MatlabPool/Pool.hpp"

extern "C"
{
    MatlabPool::Pool *construct(unsigned int n, const std::vector<std::u16string> &options);
}

#endif