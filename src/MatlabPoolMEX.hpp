#ifndef MATLABPOOLMEX_HPP
#define MATLABPOOLMEX_HPP

#include "mex.hpp"
#include "mexAdapter.hpp"

#include <exception>

#include "MatlabPool.hpp"

class MexFunction : public matlab::mex::Function
{
public:
    class Exception : public std::exception
    {
    };
    class EmptyPool : public Exception
    {
    public:
        const char *what() const noexcept override
        {
            return "MatlabPool is not initialized";
        }
    };
    class UndefCmd : public Exception
    {
    public:
        const char *what() const noexcept override
        {
            return "Undefined command";
        }
    };
    class InvalidInputSize : public Exception
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

    private:
        std::string msg;
    };

    class InvalidParameterSize : public Exception
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

    private:
        std::string msg;
    };

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

    void throwError(std::string msg)
    {
        matlabPtr->feval(u"error", 0,
                         std::vector<matlab::data::Array>({factory.createScalar(std::move(msg))}));
    }

private:
    matlab::data::ArrayFactory factory;
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr = getEngine();
    std::unique_ptr<MatlabPool::Pool> pool;
};

#endif