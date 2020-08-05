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
    using StringBuf = std::basic_stringbuf<char16_t>;

    // converts a std::u16string to a std::string by cutting the 
    // extra bytes
    std::string convertUTF16StringToASCIIString(const std::u16string &str);

    // this class works like a std::ostringstream but with char16_t 
    // characters. It also delivers a shared pointer to the string
    // buffer, this is usefull for working with the Matlab Engine API
    class StreamBuf
    {
        // derived class for an access to the protected base class
        // function. These protected functions are only used for
        // computing the size and for checking whether no characters
        // in the buffer. 
        class BasicStringBuf : public StringBuf
        {
        public:
            std::size_t size() const noexcept;
            bool empty() const noexcept;
        };

    public:
        StreamBuf(const StreamBuf &) = delete;
        StreamBuf &operator=(const StreamBuf &) = delete;

        StreamBuf();

        std::shared_ptr<StringBuf> get() const noexcept;

        std::u16string str() const;

        bool empty() const;

        template <typename T>
        StreamBuf &operator<<(const T &val)
        {
            static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
            auto tmp = std::u16string{conv.from_bytes(std::to_string(val))};
            buffer->sputn(&tmp[0], tmp.size());
            return *this;
        }

        StreamBuf &operator<<(const std::u16string &val);
        StreamBuf &operator<<(const char16_t *val);

        friend void swap(StreamBuf &lhs, StreamBuf &rhs) noexcept;

    private:
        // like std::strlen
        std::size_t strlen16(const char16_t *strarg);

    private:
        std::shared_ptr<BasicStringBuf> buffer;
    };

} // namespace MatlabPool

#endif