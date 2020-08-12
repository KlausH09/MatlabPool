#include "MatlabEngine.hpp"

int main()
{
    using namespace matlab::engine; 
    std::unique_ptr<MATLABEngine> engptr = startMATLAB({u"-nojvm"});
}