#ifndef MATLABPOOL_STREAMBUF_HPP
#define MATLABPOOL_STREAMBUF_HPP

#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <locale>
#include <codecvt>

namespace MatlabPool
{
    using StreamBuf = std::basic_stringbuf<char16_t>;

    std::string convertUTF16StringToASCIIString(const std::u16string &str)
    {
        std::unique_ptr<char[]> asciistr_ptr(new char[str.size() + 1]);
        asciistr_ptr.get()[str.size()] = '\0';
        const char *u16_src = reinterpret_cast<const char *>(str.c_str());
        for (size_t n = 0; n < str.size(); ++n)
        {
            asciistr_ptr.get()[n] = u16_src[2 * n];
        }
        return std::string(asciistr_ptr.get());
    }
    // TODO virtual
    class EmptyStreamBuffer
    {
    protected:

    public:
        std::shared_ptr<StreamBuf> get() const noexcept
        {
            return std::shared_ptr<StreamBuf>(nullptr);
        }

        std::u16string str() const noexcept
        {
            return u"";
        }

        bool empty() const noexcept
        {
            return true;
        }

        template <typename T>
        EmptyStreamBuffer &operator<<(const T &val) noexcept
        {
            return *this;
        }

        friend void swap(EmptyStreamBuffer &rhs, EmptyStreamBuffer &lhs) noexcept {}

    private:
    };

    class RealStreamBuffer : public EmptyStreamBuffer
    {
    public:
        RealStreamBuffer() : buffer(std::make_shared<StreamBuf>()) {}

        std::shared_ptr<StreamBuf> get() const noexcept
        {
            return buffer;
        }

        std::u16string str() const
        {
            return buffer->str();
        }

        bool empty() const
        {
            return str().empty(); // TODO !
        }

        RealStreamBuffer &operator<<(std::size_t val) // TODO
        {
            static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
            *this << conv.from_bytes(std::to_string(val));
            return *this;
        }

        RealStreamBuffer &operator<<(const std::u16string &val)
        {
            buffer->sputn(&val[0], val.size());
            return *this;
        }

        friend void swap(RealStreamBuffer &lhs, RealStreamBuffer &rhs)
        {
            using std::swap;
            swap(rhs.buffer, lhs.buffer);
        }

    private:
        std::shared_ptr<StreamBuf> buffer;
    };

} // namespace MatlabPool

#endif