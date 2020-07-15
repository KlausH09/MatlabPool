#ifndef MATLABPOOL_STREAMBUF_HPP
#define MATLABPOOL_STREAMBUF_HPP

#include <memory>
#include <sstream>
#include <utility>
#include <locale>
#include <codecvt>

namespace MatlabPool
{

    class EmptyStreamBuffer
    {
    protected:
        using SBuf = std::basic_stringbuf<char16_t>;
    public:
        std::shared_ptr<SBuf> get() noexcept
        {
            return std::shared_ptr<SBuf>(nullptr);
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
        RealStreamBuffer() : buffer(std::make_shared<SBuf>()) {}

        std::shared_ptr<SBuf> get() noexcept
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

        RealStreamBuffer &operator<<(std::size_t val)
        {
            static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
            *this << conv.from_bytes(std::to_string(val));
            return *this;
        }

        RealStreamBuffer &operator<<(const std::u16string& val)
        {
            buffer->sputn(&val[0],val.size());
            return *this;
        }

        friend void swap(RealStreamBuffer &lhs, RealStreamBuffer &rhs)
        {
            using std::swap;
            swap(rhs.buffer, lhs.buffer);
        }

    private:
        std::shared_ptr<SBuf> buffer;
    };

} // namespace MatlabPool

#endif