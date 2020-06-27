#ifndef MATLABPOOL_POOL_HPP
#define MATLABPOOL_POOL_HPP

#include "./Job.hpp"
#include "./Definitions.hpp"

namespace MatlabPool
{
    class Pool
    {
        Pool(const Pool &) = delete;
        Pool &operator=(const Pool &) = delete;
    protected:
        Pool(){}
    public:
        virtual ~Pool(){}

        virtual JobID submit(Job &&job) = 0;
        virtual Job wait(JobID job_id) = 0;
    };
} // namespace MatlabPool


#endif