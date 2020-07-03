#ifndef MATLABPOOL_LIBLOADER_HPP
#define MATLABPOOL_LIBLOADER_HPP

#include "./Pool.hpp"



#include <exception>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#define MATLABPOOL_HANDLE HMODULE
#define MATLABPOOL_LOADLIBRARY(path) LoadLibraryA((path))
#define MATLABPOOL_LOADLIBFUN(handle, name) GetProcAddress((handle), (name))
#define MATLABPOOL_FREELIBRARY(handle) FreeLibrary((handle))
#else
#include <dlfcn.h>
#define MATLABPOOL_HANDLE void*
#define MATLABPOOL_LOADLIBRARY(path) dlopen((path), RTLD_LAZY | RTLD_LOCAL)
#define MATLABPOOL_LOADLIBFUN(handle, name) dlsym((handle), (name))
#define MATLABPOOL_FREELIBRARY(handle) dlclose((handle))
#endif

namespace MatlabPool
{
    class LibLoader
    {
        using Constructor = Pool *(std::size_t, const std::vector<std::u16string> &);
        static constexpr const char lib_path[] = "MatlabPoolLib.dll";
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
                throw CannotLoadFunction(lib_function);
            }
        }

        static LibLoader &get()
        {
            static LibLoader instance;
            return instance;
        }

    public:
        class Exception : public std::exception
        {
        };
        class CannotLoadDLL : public Exception
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
        private:
            std::string msg;
        };
        class CannotLoadFunction : public Exception
        {
        public:
            CannotLoadFunction(const char* name)
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
        MATLABPOOL_HANDLE handle;
        Constructor *constructor;
    };
} // namespace MatlabPool

#endif