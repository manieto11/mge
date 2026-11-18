#if !defined(MGE_EXCEPTIONS_HPP) && !defined(MGE_DISABLE_EXCEPTIONS)
#define MGE_EXCEPTIONS_HPP

#include <stdexcept>

namespace mge
{
    class NotImplementedException : public std::logic_error
    {
    public:
        NotImplementedException();
        NotImplementedException(const char *function);
    };
} // namespace mge

#endif