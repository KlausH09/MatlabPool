#ifndef MATLABPOOL_STREAMBUF_HPP
#define MATLABPOOL_STREAMBUF_HPP

#include <memory>
#include <sstream>
#include <utility>

namespace MatlabPool
{
    class StreamBuf
    {
    protected:
        using SBuf = std::basic_streambuf<char16_t>;
    public:
        std::shared_ptr<SBuf> get();
        const char16_t* str() const;
        std::size_t size();
        bool empty();
    };

    class EmptyStreamBuffer : public StreamBuf
    {
    public:
        std::shared_ptr<SBuf> get()
        {
            return std::shared_ptr<SBuf>(nullptr);
        }

        const char16_t* str() const
        {
            return u"";
        }

        template <typename T>
        EmptyStreamBuffer &operator<<(const T &val)
        {
            return *this;
        }

        std::size_t size()
        {
            return 0;
        }

        bool empty()
        {
            return true;
        }

        friend void swap(EmptyStreamBuffer &rhs, EmptyStreamBuffer &lhs) {}

    private:
    };

    class RealStreamBuffer : public StreamBuf
    {
    public:

        std::shared_ptr<SBuf> get()
        {
            return std::shared_ptr<SBuf>(stream.rdbuf());
        }

        const char16_t* str() const
        {
            return stream.str().c_str();
        }

        template <typename T>
        RealStreamBuffer &operator<<(const T &val)
        {
            stream << val;
            return *this;
        }

        std::size_t size()
        {
            auto tmp = stream.tellp();
            return tmp < 0 ? 0 : std::size_t(tmp);
        }

        bool empty()
        {
            return size() == 0;
        }

        friend void swap(RealStreamBuffer &lhs, RealStreamBuffer &rhs)
        {
            std::swap(rhs.stream,lhs.stream);
        }

    private:
        std::basic_ostringstream<char16_t> stream;
    };

} // namespace MatlabPool

#endif