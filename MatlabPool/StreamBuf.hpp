#ifndef MATLABPOOL_STREAMBUF_HPP
#define MATLABPOOL_STREAMBUF_HPP

#include <memory>
#include <sstream>
#include <utility>
#include <locale>
#include <codecvt>

namespace MatlabPool
{
    class StreamBuf
    {
    protected:
        using SBuf = std::basic_streambuf<char16_t>;

    public:
        std::shared_ptr<SBuf> get();
        std::u16string str() const;
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

        std::u16string str() const
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
        RealStreamBuffer() : stream(), buffer(stream.rdbuf(), [](SBuf *) {}) {}

        std::shared_ptr<SBuf> get()
        {
            return buffer;
        }

        std::u16string str() const
        {
            return stream.str();
        }

        RealStreamBuffer &operator<<(std::size_t val)
        {
            static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
            stream << conv.from_bytes(std::to_string(val));
            return *this;
        }

        RealStreamBuffer &operator<<(const std::u16string &val)
        {
            stream << val;
            return *this;
        }

        RealStreamBuffer &operator<<(const char16_t *val)
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
            std::swap(rhs.stream, lhs.stream);
        }

    private:
        std::basic_ostringstream<char16_t> stream;
        std::shared_ptr<SBuf> buffer;
    };

} // namespace MatlabPool

#endif