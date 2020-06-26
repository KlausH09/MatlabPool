#ifndef MATLABPOOL_DEFINITIONS_HPP
#define MATLABPOOL_DEFINITIONS_HPP

#include <utility>
#include <vector>
#include "MatlabDataArray.hpp"


namespace MatlabPool
{
    using JobID = std::size_t;

    using ResultVal = std::vector<matlab::data::Array>;
    using ArgVal = std::vector<matlab::data::Array>;

} // namespace MatlabPool


#endif