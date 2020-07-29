#ifndef MATLABPOOL_STREAMBUF_HPP
#define MATLABPOOL_STREAMBUF_HPP

#include "MatlabPool/Assert.hpp"

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
        class BasicStringBuf : public StreamBuf
        {
        public:
            std::size_t size() const noexcept
            {
                MATLABPOOL_ASSERT(pptr() >= pbase());
                return pptr() - pbase();
            }
            bool empty() const noexcept
            {
                return pptr() == pbase();
            }
        };
    public:
        RealStreamBuffer() : buffer(std::make_shared<BasicStringBuf>()) {}

        std::shared_ptr<StreamBuf> get() const noexcept
        {
            return std::static_pointer_cast<StreamBuf>(buffer);
        }

        std::u16string str() const
        {
            return buffer->str();
        }

        bool empty() const
        {
            return buffer->empty();
        }

        template <typename T>
        RealStreamBuffer &operator<<(const T &val)
        {
            static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
            auto tmp = std::u16string{conv.from_bytes(std::to_string(val))};
            buffer->sputn(&tmp[0], tmp.size());
            return *this;
        }

        RealStreamBuffer &operator<<(const std::u16string &val)
        {
            buffer->sputn(&val[0], val.size());
            return *this;
        }
        RealStreamBuffer &operator<<(const char16_t *val)
        {
            buffer->sputn(val, strlen16(val));
            return *this;
        }

        friend void swap(RealStreamBuffer &lhs, RealStreamBuffer &rhs)
        {
            using std::swap;
            swap(rhs.buffer, lhs.buffer);
        }

    private:
        std::size_t strlen16(const char16_t *strarg)
        {
            MATLABPOOL_ASSERT(strarg);
            const char16_t *str = strarg;
            for (; *str; ++str)
                ;
            return std::size_t(str - strarg);
        }

    private:
        std::shared_ptr<BasicStringBuf> buffer;
    };

} // namespace MatlabPool

#endif