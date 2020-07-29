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

    std::string convertUTF16StringToASCIIString(const std::u16string &str);

    class EmptyStreamBuffer
    {
    public:
        std::shared_ptr<StreamBuf> get() const noexcept;

        std::u16string str() const noexcept;

        bool empty() const noexcept;

        template <typename T>
        EmptyStreamBuffer &operator<<(const T &val) noexcept
        {
            return *this;
        }

        friend void swap(EmptyStreamBuffer &rhs, EmptyStreamBuffer &lhs) noexcept;

    private:
    };

    class RealStreamBuffer : public EmptyStreamBuffer
    {
        class BasicStringBuf : public StreamBuf
        {
        public:
            std::size_t size() const noexcept;
            bool empty() const noexcept;
        };

    public:
        RealStreamBuffer();

        std::shared_ptr<StreamBuf> get() const noexcept;

        std::u16string str() const;

        bool empty() const;

        template <typename T>
        RealStreamBuffer &operator<<(const T &val)
        {
            static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
            auto tmp = std::u16string{conv.from_bytes(std::to_string(val))};
            buffer->sputn(&tmp[0], tmp.size());
            return *this;
        }

        RealStreamBuffer &operator<<(const std::u16string &val);
        RealStreamBuffer &operator<<(const char16_t *val);

        friend void swap(RealStreamBuffer &lhs, RealStreamBuffer &rhs) noexcept;

    private:
        std::size_t strlen16(const char16_t *strarg);

    private:
        std::shared_ptr<BasicStringBuf> buffer;
    };

} // namespace MatlabPool

#endif