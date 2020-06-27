// TODO evtl loeschen

#ifndef MATLABPOOL_DEFINITIONS_HPP
#define MATLABPOOL_DEFINITIONS_HPP

#include <cstddef>

namespace MatlabPool
{
    using JobID = std::size_t;

    constexpr const char lib_path[] = "MatlabPoolLib.dll";
    constexpr const char lib_function[] = "construct";
} // namespace MatlabPool


#endif