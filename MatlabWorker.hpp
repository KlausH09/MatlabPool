#ifndef MATLABWORKER_HPP
#define MATLABWORKER_HPP

#include <thread>
#include "MatlabEngine.hpp"

class MatlabWorker
{
    MatlabWorker()
    {
        
    }


private:
    std::thread thread;

};

#endif