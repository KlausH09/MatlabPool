#ifndef MATLABPOOL_POOL_HPP
#define MATLABPOOL_POOL_HPP

#include "./Job.hpp"

namespace MatlabPool
{
    class Pool
    {
        Pool(const Pool &) = delete;
        Pool &operator=(const Pool &) = delete;
    protected:
        Pool(){}
    public:

        class Exception : public std::exception
        {
        };
        class JobNotExists : public Exception
        {
        public:
            JobNotExists(JobID id)
            {
                std::ostringstream os;
                os << "job with id=" << id << " does not exists";
                msg = os.str();
            }
            const char *what() const noexcept override
            {
                return msg.c_str();
            }
        private:
            std::string msg;
        };

        virtual ~Pool(){}

        virtual JobID submit(Job &&job) = 0;
        virtual bool exists(JobID id) noexcept = 0;
        virtual Job wait(JobID job_id) = 0;
    };
} // namespace MatlabPool


#endif