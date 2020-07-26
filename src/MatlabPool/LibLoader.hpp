#ifndef MATLABPOOL_LIBLOADER_HPP
#define MATLABPOOL_LIBLOADER_HPP

#include "MatlabPool/Exception.hpp"

#include <string>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#define MATLABPOOL_HANDLE HMODULE
#define MATLABPOOL_LOADLIBRARY(path) LoadLibraryA((path))
#define MATLABPOOL_LOADLIBFUN(handle, name) GetProcAddress((handle), (name))
#define MATLABPOOL_FREELIBRARY(handle) FreeLibrary((handle))
#define MATLABPOOL_GET_ERROR_MSG std::to_string(GetLastError())
#else
#include <dlfcn.h>
#define MATLABPOOL_HANDLE void *
#define MATLABPOOL_LOADLIBRARY(path) dlopen((path), RTLD_LAZY | RTLD_LOCAL)
#define MATLABPOOL_LOADLIBFUN(handle, name) dlsym((handle), (name))
#define MATLABPOOL_FREELIBRARY(handle) dlclose((handle))
#define MATLABPOOL_GET_ERROR_MSG dlerror()
#endif

namespace MatlabPool
{
    class LibLoader
    {
        LibLoader(const LibLoader &) = delete;
        LibLoader &operator=(const LibLoader &) = delete;

    public:
        class LibLoaderException : public Exception
        {
        };
        class CannotLoadDLL : public LibLoaderException
        {
        public:
            CannotLoadDLL(const char *path);
            const char *what() const noexcept override;
            const char *identifier() const noexcept override;

        private:
            std::string msg;
        };
        class CannotLoadDLLFunction : public LibLoaderException
        {
        public:
            CannotLoadDLLFunction(const char *name);
            const char *what() const noexcept override;
            const char *identifier() const noexcept override;

        private:
            std::string msg;
        };

    public:
        LibLoader(const char *path)
        {
            handle = MATLABPOOL_LOADLIBRARY(path);
            if (!handle)
                throw CannotLoadDLL(path);
        }
        ~LibLoader()
        {
            if (handle)
                MATLABPOOL_FREELIBRARY(handle);
        }

        template <typename FUN>
        FUN* load_fun(const char *name)
        {
            FUN *fun = reinterpret_cast<FUN *>(MATLABPOOL_LOADLIBFUN(handle, name));
            if (!fun)
                throw CannotLoadDLLFunction(name);
            return fun;
        }

    private:
        MATLABPOOL_HANDLE handle;
    };

    LibLoader::CannotLoadDLL::CannotLoadDLL(const char *path)
    {
        std::ostringstream os;
        os << "Cannot load Library \"" << path << "\"\n"
           << "Error: " << MATLABPOOL_GET_ERROR_MSG << '\n';
        msg = os.str();
    }
    const char *LibLoader::CannotLoadDLL::what() const noexcept
    {
        return msg.c_str();
    }
    const char *LibLoader::CannotLoadDLL::identifier() const noexcept
    {
        return "CannotLoadDLL";
    }

    LibLoader::CannotLoadDLLFunction::CannotLoadDLLFunction(const char *name)
    {
        std::ostringstream os;
        os << "Cannot load Library Function \"" << name << "\"\n"
           << "Error: " << MATLABPOOL_GET_ERROR_MSG << '\n';
        msg = os.str();
    }
    const char *LibLoader::CannotLoadDLLFunction::what() const noexcept
    {
        return msg.c_str();
    }
    const char *LibLoader::CannotLoadDLLFunction::identifier() const noexcept
    {
        return "CannotLoadDLLFunction";
    }

} // namespace MatlabPool

#endif