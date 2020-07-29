
#include "MatlabPool/LibLoader.hpp"

#include <sstream>

namespace MatlabPool
{

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

    LibLoader::LibLoader(const char *path)
    {
        handle = MATLABPOOL_LOADLIBRARY(path);
        if (!handle)
            throw CannotLoadDLL(path);
    }

    LibLoader::~LibLoader()
    {
        if (handle)
            MATLABPOOL_FREELIBRARY(handle);
    }

} // namespace MatlabPool
