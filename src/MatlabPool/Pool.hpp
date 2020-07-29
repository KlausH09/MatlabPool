#ifndef MATLABPOOL_POOL_HPP
#define MATLABPOOL_POOL_HPP

#include "MatlabPool/JobFeval.hpp"
#include "MatlabPool/JobEval.hpp"

namespace MatlabPool
{
    class Pool
    {
        Pool(const Pool &) = delete;
        Pool &operator=(const Pool &) = delete;
    protected:
        Pool(){}
    public:
        class PoolException : public Exception
        {};

        class JobNotExists : public PoolException
        {
        public:
            JobNotExists(JobID id);
            const char *what() const noexcept override;
            const char *identifier() const noexcept override;
        private:
            std::string msg;
        };
        
        class EmptyPool : public PoolException
        {
        public:
            const char *what() const noexcept override;
            const char *identifier() const noexcept override;
        };

        virtual ~Pool(){}
        virtual void resize(unsigned int n_new, const std::vector<std::u16string> &options) = 0;
        virtual std::size_t size() const = 0;
        virtual JobID submit(JobFeval &&job) = 0;
        virtual JobFeval wait(JobID job_id) = 0;
        virtual void eval(JobEval &job) = 0;
        virtual matlab::data::StructArray get_job_status() = 0;
        virtual matlab::data::StructArray get_worker_status() = 0;
        virtual void cancel(JobID jobID) = 0;
        virtual void clear() = 0;
    };
} // namespace MatlabPool


#endif