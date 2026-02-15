#pragma once

#include <cstdlib>

namespace evqovv
{
namespace utils
{
[[noreturn]] inline void terminate() noexcept
{
#if defined(__has_builtin)
#if __has_builtin(__builtin_trap)
    __builtin_trap();
#elif __has_builtin(__builtin_abort)
    __builtin_abort();
#else
    ::std::abort();
#endif
#else
    ::std::abort();
#endif
}
} // namespace utils
} // namespace evqovv