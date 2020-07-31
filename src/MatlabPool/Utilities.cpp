#include "MatlabPool/Utilities.hpp"
#include <algorithm>

namespace MatlabPool::Utilities
{
    matlab::data::StructArray addFields(matlab::data::StructArray val, std::vector<std::string> fieldsNew)
    {
        using namespace matlab::data;
        ArrayFactory factory;

        std::vector<std::string> fields;

        auto fieldsOld = val.getFieldNames();

        std::move(fieldsOld.begin(), fieldsOld.end(),
                  std::back_inserter(fields));
        std::move(fieldsNew.begin(), fieldsNew.end(),
                  std::back_inserter(fields));

        auto valNew = factory.createStructArray(val.getDimensions(), fields);

        for (const std::string &name : fieldsOld)
            for (std::size_t i = 0; i < val.getNumberOfElements(); i++)
            {
                matlab::data::Array tmp = std::move(val[i][name]);
                valNew[i][name] = std::move(tmp);
            }

        return valNew;
    }

} // namespace MatlabPool::Utilities