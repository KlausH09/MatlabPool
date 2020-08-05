#ifndef MATLABPOOL_UTILITIES_HPP
#define MATLABPOOL_UTILITIES_HPP

#include "MatlabDataArray.hpp"
#include <vector>
#include <string>

namespace MatlabPool::Utilities
{
    matlab::data::StructArray addFields(matlab::data::StructArray val,
                                        std::vector<std::string> fieldsNew);

} // namespace MatlabPool::Utilities

#endif