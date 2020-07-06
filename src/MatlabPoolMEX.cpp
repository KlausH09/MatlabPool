#include "MatlabPoolMEX.hpp"
#include "MexCommands.hpp"

void MexFunction::operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
{
    using namespace MatlabPool;
    MexCommands::CmdID id;
    try
    {
        id = get_scalar<MexCommands::CmdID>(inputs[0]);
    }
    catch (const std::exception &e)
    {
        throwError("cannot extraced mode");
    }

    try
    {
        (this->*MexCommands::get_fun(id))(outputs, inputs);
    }
    catch (const std::exception &e)
    {
        std::ostringstream os;
        os << "error in MatlabPoolMex::" << MexCommands::get_name(id) << "()\n"
           << e.what();
        throwError(os.str());
    }
    catch(...)
    {
        std::ostringstream os;
        os << "error in MatlabPoolMex::" << MexCommands::get_name(id) << "()";
        throwError(os.str());
    }
}

void MexFunction::resize(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs)
{
    if (inputs.size() < 2)
        throw InvalidInputSize(inputs.size());

    unsigned int nof_worker = (unsigned int)get_scalar<uint32_t>(inputs[1]);

    std::vector<std::u16string> options;
    for (auto e = inputs.begin() + 2; e != inputs.end(); ++e)
        options.push_back(get_string(*e));

    if (!pool)
        pool = std::unique_ptr<MatlabPool::Pool>(MatlabPool::LibLoader::createPool(nof_worker, options));
    else
        pool->resize(nof_worker, options);
    return;
}

void MexFunction::submit(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs)
{
    using namespace MatlabPool;

    if (!pool)
        throw EmptyPool();
    if (inputs.size() < 3)
        throw InvalidInputSize(inputs.size());

    JobID jobid = pool->submit(Job_feval(get_string(inputs[1]),
                                         get_scalar<std::size_t>(inputs[2]),
                                         {inputs.begin() + 3, inputs.end()}));
    outputs[0] = factory.createScalar<JobID>(jobid);
}

void MexFunction::wait(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs)
{
    using namespace MatlabPool;

    if (!pool)
        throw EmptyPool();
    if (inputs.size() != 2)
        throw InvalidInputSize(inputs.size());

    JobID jobid = get_scalar<JobID>(inputs[1]);
    Job_feval job = pool->wait(jobid);

    // TODO
}

void MexFunction::statusJobs(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs)
{
    if (!pool)
        throw EmptyPool();
    if (inputs.size() != 1)
        throw InvalidInputSize(inputs.size());

    outputs[0] = pool->get_job_status();
}

void MexFunction::statusWorker(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs)
{
    if (!pool)
        throw EmptyPool();
    if (inputs.size() != 1)
        throw InvalidInputSize(inputs.size());

    outputs[0] = pool->get_worker_status();
}

void MexFunction::eval(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs)
{
    if (!pool)
        throw EmptyPool();
    if (inputs.size() != 2)
        throw InvalidInputSize(inputs.size());

    MatlabPool::Job job(get_string(inputs[1]));
    pool->eval(job);

    // TODO
}
void MexFunction::cancel(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs)
{
    if (!pool)
        throw EmptyPool();
    if (inputs.size() != 2)
        throw InvalidInputSize(inputs.size());

    pool->cancel(get_scalar<MatlabPool::JobID>(inputs[1]));
}
