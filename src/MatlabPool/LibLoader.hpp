#ifndef MATLABPOOL_LIBLOADER_HPP
#define MATLABPOOL_LIBLOADER_HPP

#include "MatlabPool/Pool.hpp"
#include "MatlabPool/Exception.hpp"

#include <exception>
#include <string>
#include <sstream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
using Handle = HMODULE;
#define MATLABPOOL_LOADLIBRARY(path) LoadLibraryA((path))
#define MATLABPOOL_LOADLIBFUN(handle, name) GetProcAddress((handle), (name))
#define MATLABPOOL_FREELIBRARY(handle) FreeLibrary((handle))
#else
#include <dlfcn.h>
using Handle = void *;
#define MATLABPOOL_LOADLIBRARY(path) dlopen((path), RTLD_LAZY | RTLD_LOCAL)
#define MATLABPOOL_LOADLIBFUN(handle, name) dlsym((handle), (name))
#define MATLABPOOL_FREELIBRARY(handle) dlclose((handle))
#endif

namespace MatlabPool
{
    class LibLoader
    {
        using Constructor = Pool *(std::size_t, const std::vector<std::u16string> &);
        static constexpr const char lib_path[] = "/home/oem/repos/MatlabPool/libMatlabPool.so"; // TODO windows/linux und lib...
        static constexpr const char lib_function[] = "construct";

        LibLoader(const LibLoader &) = delete;
        LibLoader &operator=(const LibLoader &) = delete;

        LibLoader()
        {
            handle = MATLABPOOL_LOADLIBRARY(lib_path);
            if (!handle)
                throw CannotLoadDLL(lib_path);

            constructor =
                reinterpret_cast<Constructor *>(MATLABPOOL_LOADLIBFUN(handle, lib_function));
            if (!constructor)
            {
                MATLABPOOL_FREELIBRARY(handle);
                throw CannotLoadDLLFunction(lib_function);
            }
        }

        static LibLoader &get()
        {
            static LibLoader instance;
            return instance;
        }

    public:
        class LibLoaderException : public Exception
        {};
        class CannotLoadDLL : public LibLoaderException
        {
        public:
            CannotLoadDLL(const char *path)
            {
                std::ostringstream os;
                os << "Cannot load Library \"" << path << "\"\n";
                addErrorMsg(os);
                msg = os.str();
            }
            const char *what() const noexcept override
            {
                return msg.c_str();
            }
            const char *identifier() const noexcept override
            {
                return "CannotLoadDLL";
            }
        private:
            std::string msg;
        };
        class CannotLoadDLLFunction : public LibLoaderException
        {
        public:
            CannotLoadDLLFunction(const char *name)
            {
                std::ostringstream os;
                os << "Cannot load Library Function \"" << name << "\"\n";
                addErrorMsg(os);
                msg = os.str();
            }
            const char *what() const noexcept override
            {
                return msg.c_str();
            }
            const char *identifier() const noexcept override
            {
                return "CannotLoadDLLFunction";
            }
        private:
            std::string msg;
        };

        ~LibLoader()
        {
            if (handle)
                MATLABPOOL_FREELIBRARY(handle);
        }

        static Pool *createPool(std::size_t n, const std::vector<std::u16string> &options)
        {
            return get().constructor(n, options);
        }

    private:
        static void addErrorMsg(std::ostringstream &os)
        {
#ifdef _WIN32
            os << "Error Code: " << std::to_string(GetLastError());
#else
            os << "Error: " << dlerror();
#endif
        }

    private:
        Handle handle;
        Constructor *constructor;
    };
} // namespace MatlabPool

#endif