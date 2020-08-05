#include "MatlabPoolMEX.hpp"
#include "MexCommands.hpp"

const char *MexFunction::EmptyPool::what() const noexcept
{
    return "MatlabPool is not initialized";
}
const char *MexFunction::EmptyPool::identifier() const noexcept
{
    return "EmptyPool";
}

MexFunction::InvalidInputSize::InvalidInputSize(std::size_t size)
{
    std::ostringstream os;
    os << "invalid input size: " << size;
    msg = os.str();
}
const char *MexFunction::InvalidInputSize::what() const noexcept
{
    return msg.c_str();
}
const char *MexFunction::InvalidInputSize::identifier() const noexcept
{
    return "InvalidInputSize";
}

MexFunction::InvalidParameterSize::InvalidParameterSize(std::vector<std::size_t> size)
{
    std::ostringstream os;
    os << "invalid parameter size: (";
    std::string seperator = "";
    for (auto s : size)
    {
        os << seperator << s;
        seperator = ",";
    }
    os << ")";
    msg = os.str();
}
const char *MexFunction::InvalidParameterSize::what() const noexcept
{
    return msg.c_str();
}
const char *MexFunction::InvalidParameterSize::identifier() const noexcept
{
    return "InvalidParameterSize";
}

void MexFunction::operator()(ArgumentList outputs, ArgumentList inputs)
{
    using namespace MatlabPool;
    MexCommands::CmdID id = 0;

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

void MexFunction::resize(ArgumentList &outputs, ArgumentList &inputs)
{
    if (inputs.size() < 2)
        throw InvalidInputSize(inputs.size());

    unsigned int nof_worker = (unsigned int)get_scalar<uint32_t>(inputs[1]);

    std::vector<std::u16string> options;
    for (auto e = inputs.begin() + 2; e != inputs.end(); ++e)
        options.push_back(((matlab::data::CharArray)*e).toUTF16());

    if (!pool)
    {
        using namespace MatlabPool;
        pool = std::unique_ptr<Pool>(PoolLibLoader::createPool(nof_worker, options));
    }
    else
        pool->resize(nof_worker, options);
}

void MexFunction::submit(ArgumentList &outputs, ArgumentList &inputs)
{
    using namespace MatlabPool;

    if (!pool)
        throw EmptyPool();
    if (inputs.size() < 3)
        throw InvalidInputSize(inputs.size());

    std::u16string funname = ((matlab::data::CharArray)inputs[1]).toUTF16();

    JobID jobid = pool->submit(JobFeval(std::move(funname),
        get_scalar<std::size_t>(inputs[2]),
        { inputs.begin() + 3, inputs.end() }));
    outputs[0] = factory.createScalar<JobID>(jobid);
}

void MexFunction::wait(ArgumentList &outputs, ArgumentList &inputs)
{
    using namespace MatlabPool;

    if (!pool)
        throw EmptyPool();
    if (inputs.size() != 2)
        throw InvalidInputSize(inputs.size());

    JobID jobid = get_scalar<JobID>(inputs[1]);
    JobFeval job = pool->wait(jobid);

    outputs[0] = job.toStruct();
}

void MexFunction::statusJobs(ArgumentList &outputs, ArgumentList &inputs)
{
    if (!pool)
        throw EmptyPool();
    if (inputs.size() != 1)
        throw InvalidInputSize(inputs.size());

    outputs[0] = pool->get_job_status();
}

void MexFunction::statusWorker(ArgumentList &outputs, ArgumentList &inputs)
{
    if (!pool)
        throw EmptyPool();
    if (inputs.size() != 1)
        throw InvalidInputSize(inputs.size());

    outputs[0] = pool->get_worker_status();
}

void MexFunction::eval(ArgumentList &outputs, ArgumentList &inputs)
{
    if (!pool)
        throw EmptyPool();
    if (inputs.size() != 2)
        throw InvalidInputSize(inputs.size());

    std::u16string funname = ((matlab::data::CharArray)inputs[1]).toUTF16();

    MatlabPool::JobEval job(std::move(funname));
    pool->eval(job);

    outputs[0] = job.toStruct();
}
void MexFunction::cancel(ArgumentList &outputs, ArgumentList &inputs)
{
    if (!pool)
        throw EmptyPool();
    if (inputs.size() != 2)
        throw InvalidInputSize(inputs.size());

    pool->cancel(get_scalar<MatlabPool::JobID>(inputs[1]));
}

void MexFunction::size(ArgumentList &outputs, ArgumentList &inputs)
{
    if (inputs.size() != 1)
        throw InvalidInputSize(inputs.size());

    if (!pool)
        outputs[0] = factory.createScalar<std::size_t>(0);
    else
        outputs[0] = factory.createScalar<std::size_t>(pool->size());
}

void MexFunction::clear(ArgumentList &outputs, ArgumentList &inputs)
{
    if (inputs.size() != 1)
        throw InvalidInputSize(inputs.size());

    if (pool)
        pool->clear();
}
