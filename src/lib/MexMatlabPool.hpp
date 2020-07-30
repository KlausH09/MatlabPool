#ifndef MATLABPOOLMEX_HPP
#define MATLABPOOLMEX_HPP

#include "mex.hpp"
#include "mexAdapter.hpp"

#include <exception>

#include "MatlabPool.hpp"

class MexFunction : public matlab::mex::Function
{
    using MatlabPtr = std::shared_ptr<matlab::engine::MATLABEngine>;

public:
    class MexFunctionException : public MatlabPool::Exception
    {
    public:
        virtual const char *identifier() const noexcept = 0;
    };
    class EmptyPool : public MexFunctionException
    {
    public:
        const char *what() const noexcept override
        {
            return "MatlabPool is not initialized";
        }
        const char *identifier() const noexcept override
        {
            return "EmptyPool";
        }
    };
    class UndefCmd : public MexFunctionException
    {
    public:
        const char *what() const noexcept override
        {
            return "Undefined command";
        }
        const char *identifier() const noexcept override
        {
            return "UndefCMD";
        }
    };
    class InvalidInputSize : public MexFunctionException
    {
    public:
        InvalidInputSize(std::size_t size)
        {
            std::ostringstream os;
            os << "invalid input size: " << size;
            msg = os.str();
        }
        const char *what() const noexcept override
        {
            return msg.c_str();
        }
        const char *identifier() const noexcept override
        {
            return "InvalidInputSize";
        }
    private:
        std::string msg;
    };

    class InvalidParameterSize : public MexFunctionException
    {
    public:
        InvalidParameterSize(std::vector<std::size_t> size)
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
        const char *what() const noexcept override
        {
            return msg.c_str();
        }
        const char *identifier() const noexcept override
        {
            return "InvalidParameterSize";
        }
    private:
        std::string msg;
    };

public:
    MexFunction()
    {
        //mexLock();
    }
    ~MexFunction()
    {
        //mexUnlock();
    }

    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs);

    void resize(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs);
    void submit(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs);
    void wait(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs);
    void statusJobs(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs);
    void statusWorker(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs);
    void eval(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs);
    void cancel(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs);
    void size(matlab::mex::ArgumentList &outputs, matlab::mex::ArgumentList &inputs);

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

#endif