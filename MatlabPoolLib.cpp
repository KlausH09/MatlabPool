#include "MatlabPoolLib.hpp"
#include "MatlabPool/Pool_impl.hpp"

MatlabPool::Pool* construct(std::size_t n, const std::vector<std::u16string> &options)
{
    return new MatlabPool::PoolImpl(n, options);
}