#ifndef MATLABPOOL_POOLLIBLOADER_HPP
#define MATLABPOOL_POOLLIBLOADER_HPP

#include "MatlabPool/Pool.hpp"
#include "MatlabPool/LibLoader.hpp"

namespace MatlabPool
{
    // This singleton class is just for loading the MatlabPool
    // library
    class PoolLibLoader : protected LibLoader
    {
        using Constructor = Pool *(std::size_t, const std::vector<std::u16string> &);

        PoolLibLoader(const PoolLibLoader &) = delete;
        PoolLibLoader &operator=(const PoolLibLoader &) = delete;

        PoolLibLoader();

        static PoolLibLoader &get();

    public:
        static Pool *createPool(std::size_t n, const std::vector<std::u16string> &options);

    private:
        Constructor *constructor;
    };
} // namespace MatlabPool

#endif