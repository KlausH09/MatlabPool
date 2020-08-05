#include "MatlabPool/StreamBuf.hpp"

namespace MatlabPool
{
    
    // this function is copied from:
    //   matlabroot/extern/include/MatlabEngine/detail/engine_execution_interface_impl.hpp
    //   (Line 542)
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

    std::size_t StreamBuf::BasicStringBuf::size() const noexcept
    {
        MATLABPOOL_ASSERT(pptr() >= pbase());
        return pptr() - pbase();
    }
    bool StreamBuf::BasicStringBuf::empty() const noexcept
    {
        return pptr() == pbase();
    }

    StreamBuf::StreamBuf() : buffer(std::make_shared<BasicStringBuf>()) {}

    std::shared_ptr<StringBuf> StreamBuf::get() const noexcept
    {
        return std::static_pointer_cast<StringBuf>(buffer);
    }

    std::u16string StreamBuf::str() const
    {
        return buffer->str();
    }

    bool StreamBuf::empty() const
    {
        return buffer->empty();
    }

    StreamBuf &StreamBuf::operator<<(const std::u16string &val)
    {
        buffer->sputn(&val[0], val.size());
        return *this;
    }
    StreamBuf &StreamBuf::operator<<(const char16_t *val)
    {
        buffer->sputn(val, strlen16(val));
        return *this;
    }

    void swap(StreamBuf &lhs, StreamBuf &rhs) noexcept
    {
        using std::swap;
        swap(rhs.buffer, lhs.buffer);
    }

    std::size_t StreamBuf::strlen16(const char16_t *strarg)
    {
        MATLABPOOL_ASSERT(strarg);
        const char16_t *str = strarg;
        for (; *str; ++str)
            ;
        return std::size_t(str - strarg);
    }

} // namespace MatlabPool
