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
        class EmptyPool : public Exception
        {
        public:
            const char *what() const noexcept override
            {
                return "pool size is equal zero";
            }
        };

        virtual ~Pool(){}
        virtual void resize(unsigned int n_new, const std::vector<std::u16string> &options) = 0;
        virtual std::size_t size() const = 0;
        virtual JobID submit(Job_feval &&job) = 0;
        virtual Job_feval wait(JobID job_id) = 0;
        virtual void eval(Job &job) = 0;
        virtual matlab::data::StructArray get_job_status() = 0;
        virtual matlab::data::StructArray get_worker_status() = 0;
        virtual void cancel(JobID jobID) = 0;
    };
} // namespace MatlabPool


#endif