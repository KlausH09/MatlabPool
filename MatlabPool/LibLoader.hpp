#ifndef MATLABPOOL_LIBLOADER_HPP
#define MATLABPOOL_LIBLOADER_HPP

#include "./Pool.hpp"

#include <windows.h>
//#include <dlfcn.h>

#include <exception>
#include <string>

using namespace std::string_literals;

namespace MatlabPool
{
    class LibLoader
    {
        using Constructor = Pool *(std::size_t, const std::vector<std::u16string> &);

        LibLoader(const LibLoader &) = delete;
        LibLoader &operator=(const LibLoader &) = delete;

        LibLoader() : handle(nullptr)
        {
            handle = LoadLibraryA(lib_path);
            //handle = dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL);

            if (!handle)
                throw std::runtime_error("Cannot load Library \""s + lib_path + "\"\nError Code: " + std::to_string(GetLastError()));

            constructor =
                reinterpret_cast<Constructor *>(GetProcAddress(handle, lib_function));
            //constructor =
            //   reinterpret_cast<Constructor*>(dlsym(handle, lib_function));

            if (!constructor)
                throw std::runtime_error("Cannot load Library Function \""s + lib_function + "\"\nError Code: " + std::to_string(GetLastError()));
        }

        static LibLoader &get()
        {
            static LibLoader instance;
            return instance;
        }

    public:
        ~LibLoader()
        {
            if (handle)
            {
                FreeLibrary(handle);
                //dlclose(handle);
            }
        }

        static Pool *createPool(std::size_t n, const std::vector<std::u16string> &options)
        {
            return get().constructor(n, options);
        }

    private:
        HMODULE handle;
        Constructor *constructor;
    };
} // namespace MatlabPool

#endif