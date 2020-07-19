#include "MatlabPoolMEX.hpp"
#include "MexCommands.hpp"

void MexFunction::operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs)
{
    using namespace MatlabPool;
    MexCommands::CmdID id;

    try
    {
        if (inputs.size() < 1)
            throw InvalidInputSize(inputs.size());

        id = get_scalar<MexCommands::CmdID>(inputs[0]);
    }
    catch (const MatlabPool::Exception &e)
    {
        throwError(e.identifier(), e.what());
    }

    try
    {
        (this->*MexCommands::get_fun(id))(outputs, inputs);
    }
    catch (const MatlabPool::Exception &e)
    {
        throwError(e.identifier(), e.what());
    }
    catch (const std::exception &e)
    {
        std::ostringstream os;
        os << "error in MatlabPoolMex::" << MexCommands::get_name(id) << "()\n"
           << e.what();
        throwError("ErrorInCmd", os.str());
    }
    catch (...)
    {
        std::ostringstream os;
        os << "error in MatlabPoolMex::" << MexCommands::get_name(id) << "()";
        throwError("ErrorInCmd", os.str());
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
}

void MexFunction::submit(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs)
{
    using namespace MatlabPool;

    if (!pool)
        throw EmptyPool();
    if (inputs.size() < 3)
        throw InvalidInputSize(inputs.size());

    JobID jobid = pool->submit(JobFeval(get_string(inputs[1]),
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
    JobFeval job = pool->wait(jobid);

    std::vector<matlab::data::Array> result = job.pop_result();

    if (!job.get_outBuf().empty())
        disp(job.get_outBuf().str());

    auto result_cell = factory.createCellArray({result.size()});

    for (std::size_t i = 0; i < result.size(); i++)
        result_cell[i] = std::move(result[i]);

    outputs[0] = std::move(result_cell);
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

    MatlabPool::JobEval job(get_string(inputs[1]));
    pool->eval(job);

    if (!job.get_outBuf().empty())
        disp(job.get_outBuf().str());
}
void MexFunction::cancel(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs)
{
    if (!pool)
        throw EmptyPool();
    if (inputs.size() != 2)
        throw InvalidInputSize(inputs.size());

    pool->cancel(get_scalar<MatlabPool::JobID>(inputs[1]));
}

void MexFunction::size(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs)
{
    if (inputs.size() != 1)
        throw InvalidInputSize(inputs.size());

    if (!pool)
        outputs[0] = factory.createScalar<std::size_t>(0);
    else
        outputs[0] = factory.createScalar<std::size_t>(pool->size());
}
