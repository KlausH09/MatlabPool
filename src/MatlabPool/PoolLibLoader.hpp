#ifndef MATLABPOOL_POOLLIBLOADER_HPP
#define MATLABPOOL_POOLLIBLOADER_HPP

#include "MatlabPool/Pool.hpp"
#include "MatlabPool/LibLoader.hpp"

#ifndef MATLABPOOL_DLL_PATH
#define MATLABPOOL_DLL_PATH "./lib/libMatlabPool.dll"
#endif

namespace MatlabPool
{

    class PoolLibLoader : protected LibLoader
    {
        using Constructor = Pool *(std::size_t, const std::vector<std::u16string> &);
        static constexpr const char lib_path[] = MATLABPOOL_DLL_PATH;
        static constexpr const char lib_function[] = "construct";

        PoolLibLoader(const PoolLibLoader &) = delete;
        PoolLibLoader &operator=(const PoolLibLoader &) = delete;

        PoolLibLoader() : LibLoader(lib_path),
                          constructor(load_fun<Constructor>(lib_function))
        {
        }

        static PoolLibLoader &get()
        {
            static PoolLibLoader instance;
            return instance;
        }

    public:
        static Pool *createPool(std::size_t n, const std::vector<std::u16string> &options)
        {
            return get().constructor(n, options);
        }

    private:
        Constructor *constructor;
    };
} // namespace MatlabPool

#endif