#ifndef MATLABPOOL_UTILITIES_HPP
#define MATLABPOOL_UTILITIES_HPP

#include "MatlabDataArray.hpp"
#include <vector>
#include <string>

namespace MatlabPool::Utilities
{
    // unfortunately all fields of a matlab struct must be defined 
    // during the construction. This function functions adds new 
    // fields to the struct, by creating a new struct and swaping
    // the values of the original struct
    matlab::data::StructArray addFields(matlab::data::StructArray val,
                                        std::vector<std::string> fieldsNew);

} // namespace MatlabPool::Utilities

#endif