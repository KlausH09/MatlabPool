#ifndef MATLABPOOLMEX_HPP
#define MATLABPOOLMEX_HPP

#include "mex.hpp"
#include "mexAdapter.hpp"

#include <exception>

#include "MatlabPool.hpp"

class MexFunction : public matlab::mex::Function
{
    using ArgumentList = matlab::mex::ArgumentList;
    using MatlabPtr = std::shared_ptr<matlab::engine::MATLABEngine>;

public:
    class MexFunctionException : public MatlabPool::Exception
    {
    };
    class EmptyPool : public MexFunctionException
    {
    public:
        const char *what() const noexcept override;
        const char *identifier() const noexcept override;
    };
    class UndefCmd : public MexFunctionException
    {
    public:
        const char *what() const noexcept override;
        const char *identifier() const noexcept override;
    };
    class InvalidInputSize : public MexFunctionException
    {
    public:
        InvalidInputSize(std::size_t size);
        const char *what() const noexcept override;
        const char *identifier() const noexcept override;
    private:
        std::string msg;
    };

    class InvalidParameterSize : public MexFunctionException
    {
    public:
        InvalidParameterSize(std::vector<std::size_t> size);
        const char *what() const noexcept override;
        const char *identifier() const noexcept override;
    private:
        std::string msg;
    };

public:
    void operator()(ArgumentList outputs, ArgumentList inputs);

    void resize(ArgumentList &outputs, ArgumentList &inputs);
    void submit(ArgumentList &outputs, ArgumentList &inputs);
    void wait(ArgumentList &outputs, ArgumentList &inputs);
    void statusJobs(ArgumentList &outputs, ArgumentList &inputs);
    void statusWorker(ArgumentList &outputs, ArgumentList &inputs);
    void eval(ArgumentList &outputs, ArgumentList &inputs);
    void cancel(ArgumentList &outputs, ArgumentList &inputs);
    void size(ArgumentList &outputs, ArgumentList &inputs);
    void clear(ArgumentList &outputs, ArgumentList &inputs);

private:
    template <typename T>
    T get_scalar(const matlab::data::Array &data) const
    {
        if (data.getNumberOfElements() != 1)
            throw InvalidParameterSize(data.getDimensions());
        T val = ((matlab::data::TypedArray<T>)(data))[0];
        return val;
    }
    std::u16string get_string(const matlab::data::Array &data) const
    {
        return ((matlab::data::CharArray)data).toUTF16();
    }

    template <typename T>
    void disp(const T &msg)
    {
        matlabPtr->feval(u"disp", 0,
                         std::vector<matlab::data::Array>({factory.createScalar(msg)}));
    }

    template <typename T>
    void throwError(const char *id, const T &msg)
    {
        std::string id_msg{"MatlabPoolMEX:"};
        id_msg += id;
        matlabPtr->feval(u"error", 0,
                         std::vector<matlab::data::Array>({factory.createScalar(id_msg),
                                                           factory.createScalar(msg)}));
    }

private:
    matlab::data::ArrayFactory factory;
    MatlabPtr matlabPtr = getEngine();
    std::unique_ptr<MatlabPool::Pool> pool;
};

const char *MexFunction::EmptyPool::what() const noexcept
{
    return "MatlabPool is not initialized";
}
const char *MexFunction::EmptyPool::identifier() const noexcept
{
    return "EmptyPool";
}

const char *MexFunction::UndefCmd::what() const noexcept
{
    return "Undefined command";
}
const char *MexFunction::UndefCmd::identifier() const noexcept
{
    return "UndefCMD";
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

#endif