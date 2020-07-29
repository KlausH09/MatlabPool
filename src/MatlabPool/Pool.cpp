#include "MatlabPool/Pool.hpp"

namespace MatlabPool
{

    Pool::JobNotExists::JobNotExists(JobID id)
    {
        std::ostringstream os;
        os << "job with id=" << id << " does not exists";
        msg = os.str();
    }
    const char *Pool::JobNotExists::what() const noexcept
    {
        return msg.c_str();
    }
    const char *Pool::JobNotExists::identifier() const noexcept
    {
        return "JobNotExists";
    }

    const char *Pool::EmptyPool::what() const noexcept
    {
        return "pool size is equal zero";
    }
    const char *Pool::EmptyPool::identifier() const noexcept
    {
        return "EmptyPool";
    }

} // namespace MatlabPool
