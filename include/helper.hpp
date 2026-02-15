#pragma once

#include <cstdlib>
#include <system_error>

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

inline constexpr void assert_condition(bool condition) noexcept
{
    if (!condition)
    {
        terminate();
    }
}

[[noreturn]] inline void throw_system_exception(const char *what)
{
    throw std::system_error(errno, std::generic_category(), what);
}

} // namespace utils
} // namespace evqovv