#ifndef MATLABPOOL_JOB_HPP
#define MATLABPOOL_JOB_HPP

#include <string>
#include <vector>

#include "MatlabDataArray.hpp"

namespace MatlabPool
{
    struct Job
    {
    private:
        using Args = std::vector<matlab::data::Array>;
        Job(const Job &) = delete;
        Job &operator=(const Job &) = delete;

    public:
        Job() : id(0) {}

        // TODO iterator durch "args"
        Job(std::u16string &&function, std::size_t nlhs, Args &&args) : id(id_count++),
                                                                        function(std::move(function)),
                                                                        nlhs(nlhs),
                                                                        args(std::move(args))
        {
        }

        Job(Job &&other) : Job()
        {
            swap(*this, other);
        }

        Job &operator=(Job &&other)
        {
            swap(*this, other);
            return *this;
        }

        void swap(Job &j1, Job &j2)
        {
            std::swap(j1.id, j2.id);
            std::swap(j1.function, j2.function);
            std::swap(j1.nlhs, j2.nlhs);
            std::swap(j1.args, j2.args);
        }

    public: // TODO
        static std::size_t id_count;
        std::size_t id;
        std::u16string function;
        std::size_t nlhs;
        Args args;
    };
    std::size_t Job::id_count = 1;
} // namespace MatlabPool

#endif