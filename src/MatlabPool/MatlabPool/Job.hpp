#ifndef MATLABPOOL_JOB_HPP
#define MATLABPOOL_JOB_HPP

#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <ostream>

#include "./JobBase.hpp"
#include "assert.hpp"

#include "MatlabDataArray.hpp"

namespace MatlabPool
{

    class Job : public JobBase
    {
        Job(const Job &) = delete;
        Job &operator=(const Job &) = delete;

    public:
        enum class Status : uint8_t
        {
            Wait,
            AssignToWorker,
            InProgress,
            Done,
            Error,
            Canceled,
            DoneEmpty,
            Empty,
        };

    public:
        Job() noexcept : JobBase(), status(Status::Empty), workerID(-1){};
        Job(std::u16string cmd, std::size_t nlhs, std::vector<matlab::data::Array> &&args) : JobBase(cmd),
                                                                                             status(Status::Wait),
                                                                                             nlhs(nlhs),
                                                                                             args(std::move(args)),
                                                                                             workerID(-1)

        {
        }

        Job(Job &&other) noexcept : Job()
        {
            using std::swap;
            swap(*this, other);
        }

        Job &operator=(Job &&other) noexcept
        {
            using std::swap;
            swap(*this, other);
            return *this;
        }

        friend void swap(Job &j1, Job &j2) noexcept
        {
            using std::swap;
            swap(static_cast<JobBase &>(j1), static_cast<JobBase &>(j2));
            swap(j1.status,j2.status);
            swap(j1.nlhs, j2.nlhs);
            swap(j1.args, j2.args);
            swap(j1.result, j2.result);
            swap(j1.workerID, j2.workerID);
        }
        void set_AssignToWorker_status()
        {
            ASSERT(status == Status::Wait);
            status = Status::AssignToWorker;
        }

        std::size_t get_nlhs() const noexcept
        {
            return nlhs;
        }
        std::vector<matlab::data::Array> &get_args() noexcept
        {
            return args;
        }

        void set_workerID(int val) noexcept
        {
            if (val >= 0)
            {
                ASSERT(status == Status::AssignToWorker);
                status = Status::InProgress;
            }
            workerID = val;
        }

        int get_workerID() const noexcept
        {
            return workerID;
        }

        Status get_status() const noexcept
        {
            return status;
        }

        const std::vector<matlab::data::Array> &peek_result() const
        {
            if (status == Status::Error)
                throw ExecutionError(id);
            ASSERT(status == Status::Done);

            return result;
        }
        std::vector<matlab::data::Array> pop_result()
        {
            if (status == Status::Error)
                throw ExecutionError(id);
            ASSERT(status == Status::Done);

            status = Status::DoneEmpty;
            return std::move(result);
        }

    protected:
        Status status;

        std::size_t nlhs;
        std::vector<matlab::data::Array> args;
        std::vector<matlab::data::Array> result;

    private:
        int workerID;
    };

} // namespace MatlabPool

#endif