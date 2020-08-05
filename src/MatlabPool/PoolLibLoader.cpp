#include "MatlabPool/PoolLibLoader.hpp"

#ifndef MATLABPOOL_DLL_PATH
#define MATLABPOOL_DLL_PATH "./lib/libMatlabPool.dll"
#endif

static constexpr const char lib_path[] = MATLABPOOL_DLL_PATH;
static constexpr const char lib_function[] = "construct";

namespace MatlabPool
{

    PoolLibLoader::PoolLibLoader()
        : LibLoader(lib_path),
        constructor(load_fun<Constructor>(lib_function))
    {
    }

    PoolLibLoader &PoolLibLoader::get()
    {
        static PoolLibLoader instance;
        return instance;
    }

    Pool *PoolLibLoader::createPool(std::size_t n,
        const std::vector<std::u16string> &options)
    {
        return get().constructor(n, options);
    }

} // namespace MatlabPool
