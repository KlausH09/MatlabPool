#include "MatlabPool/StreamBuf.hpp"

namespace MatlabPool
{

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

    std::shared_ptr<StreamBuf> EmptyStreamBuffer::get() const noexcept
    {
        return std::shared_ptr<StreamBuf>(nullptr);
    }

    std::u16string EmptyStreamBuffer::str() const noexcept
    {
        return u"";
    }

    bool EmptyStreamBuffer::empty() const noexcept
    {
        return true;
    }

    void swap(EmptyStreamBuffer &rhs, EmptyStreamBuffer &lhs) noexcept {}

    std::size_t RealStreamBuffer::BasicStringBuf::size() const noexcept
    {
        MATLABPOOL_ASSERT(pptr() >= pbase());
        return pptr() - pbase();
    }
    bool RealStreamBuffer::BasicStringBuf::empty() const noexcept
    {
        return pptr() == pbase();
    }

    RealStreamBuffer::RealStreamBuffer() : buffer(std::make_shared<BasicStringBuf>()) {}

    std::shared_ptr<StreamBuf> RealStreamBuffer::get() const noexcept
    {
        return std::static_pointer_cast<StreamBuf>(buffer);
    }

    std::u16string RealStreamBuffer::str() const
    {
        return buffer->str();
    }

    bool RealStreamBuffer::empty() const
    {
        return buffer->empty();
    }

    RealStreamBuffer &RealStreamBuffer::operator<<(const std::u16string &val)
    {
        buffer->sputn(&val[0], val.size());
        return *this;
    }
    RealStreamBuffer &RealStreamBuffer::operator<<(const char16_t *val)
    {
        buffer->sputn(val, strlen16(val));
        return *this;
    }

    void swap(RealStreamBuffer &lhs, RealStreamBuffer &rhs) noexcept
    {
        using std::swap;
        swap(rhs.buffer, lhs.buffer);
    }

    std::size_t RealStreamBuffer::strlen16(const char16_t *strarg)
    {
        MATLABPOOL_ASSERT(strarg);
        const char16_t *str = strarg;
        for (; *str; ++str)
            ;
        return std::size_t(str - strarg);
    }

} // namespace MatlabPool
