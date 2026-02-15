#pragma once

#include "helper.hpp"
#include <cassert>
#include <compare>
#include <iterator>

namespace evqovv
{
namespace utils
{

template <typename T, ::std::size_t N>
struct array
{
    T eles_[N];

    using value_type = T;
    using size_type = ::std::size_t;
    using difference_type = ::std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = ::std::reverse_iterator<iterator>;
    using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

    constexpr reference operator[](size_type pos)
    {
        if (pos >= N) [[unlikely]]
        {
            terminate();
        }

        return eles_[pos];
    }

    [[nodiscard]] constexpr const_reference operator[](size_type pos) const
    {
        if (pos >= N) [[unlikely]]
        {
            terminate();
        }

        return eles_[pos];
    }

    [[nodiscard]] reference index_unchecked(size_type pos)
    {
        return eles_[pos];
    }

    [[nodiscard]] const_reference index_unchecked(size_type pos) const
    {
        return eles_[pos];
    }

    [[nodiscard]] reference front() noexcept
    {
        return *begin();
    }

    [[nodiscard]] const_reference front() const noexcept
    {
        return *cbegin();
    }

    [[nodiscard]] reference back() noexcept
    {
        return *end();
    }

    [[nodiscard]] const_reference back() const noexcept
    {
        return *cend();
    }

    [[nodiscard]] pointer data() noexcept
    {
        return eles_;
    }

    [[nodiscard]] const_pointer data() const noexcept
    {
        return eles_;
    }

    [[nodiscard]] iterator begin() noexcept
    {
        return eles_;
    }

    [[nodiscard]] const_iterator begin() const noexcept
    {
        return eles_;
    }

    [[nodiscard]] const_iterator cbegin() const noexcept
    {
        return eles_;
    }

    [[nodiscard]] iterator end() noexcept
    {
        return eles_ + N;
    }

    [[nodiscard]] const_iterator end() const noexcept
    {
        return eles_ + N;
    }

    [[nodiscard]] const_iterator cend() const noexcept
    {
        return eles_ + N;
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept
    {
        return ::std::make_reverse_iterator(end());
    }

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
    {
        return ::std::make_reverse_iterator(end());
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
    {
        return ::std::make_reverse_iterator(cend());
    }

    [[nodiscard]] constexpr reverse_iterator rend() noexcept
    {
        return ::std::make_reverse_iterator(begin());
    }

    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
    {
        return ::std::make_reverse_iterator(begin());
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
    {
        return ::std::make_reverse_iterator(cbegin());
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return false;
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return N;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
        return N;
    }

    void fill(const_reference value) noexcept(::std::is_nothrow_copy_assignable_v<T>)
    {
        for (auto i = size_type(0); i != N; ++i)
        {
            eles_[i] = value;
        }
    }

    void swap(array &other) noexcept(::std::is_nothrow_swappable_v<T>)
    {
        for (auto i = size_type(0); i != N; ++i)
        {
            ::std::swap(eles_[i], other.eles_[i]);
        }
    }
};

template <typename T>
class array<T, 0>
{
public:
    using value_type = T;
    using size_type = ::std::size_t;
    using difference_type = ::std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = ::std::reverse_iterator<iterator>;
    using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

    [[noreturn]] constexpr reference operator[](size_type pos) noexcept
    {
        assert_condition(false);
    }

    [[noreturn]] constexpr const_reference operator[](size_type pos) const noexcept
    {
        assert_condition(false);
    }

    [[noreturn]] reference index_unchecked(size_type pos)
    {
        assert_condition(false);
    }

    [[noreturn]] const_reference index_unchecked(size_type pos) const
    {
        assert_condition(false);
    }

    [[noreturn]] constexpr reference front() noexcept
    {
        assert_condition(false);
    }

    [[noreturn]] constexpr const_reference front() const noexcept
    {
        assert_condition(false);
    }

    [[noreturn]] constexpr reference back() noexcept
    {
        assert_condition(false);
    }

    [[noreturn]] constexpr const_reference back() const noexcept
    {
        assert_condition(false);
    }

    [[nodiscard]] constexpr pointer data() noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr const_pointer data() const noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr iterator end() noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr reverse_iterator rend() noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
    {
        return nullptr;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return true;
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return 0;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
        return 0;
    }

    constexpr void fill(const_reference value) noexcept(::std::is_nothrow_copy_assignable_v<T>)
    {
    }

    constexpr void swap(array &other) noexcept(::std::is_nothrow_swappable_v<T>)
    {
    }
};

template <class T, ::std::size_t N>
bool operator==(const array<T, N> &lhs, const array<T, N> &rhs)
{
    for (auto i = ::std::size_t(0); i != N; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }
    return true;
}

#if __cplusplus >= 202002L
template <typename T, ::std::size_t N>
constexpr auto operator<=>(const std::array<T, N> &lhs, const std::array<T, N> &rhs)
{
    for (auto i = ::std::size_t(0); i != N; ++i)
    {
        auto cmp = lhs[i] <=> rhs[i];
        if (cmp != 0)
        {
            return cmp;
        }
    }
}
#else
template <class T, ::std::size_t N>
bool operator!=(const array<T, N> &lhs, const array<T, N> &rhs)
{
    return !(lhs == rhs);
}

template <class T, ::std::size_t N>
bool operator<(const array<T, N> &lhs, const array<T, N> &rhs)
{
    for (auto i = ::std::size_t(0); i != N; ++i)
    {
        if (lhs[i] < rhs[i])
        {
            return false;
        }
    }
    return true;
}

template <class T, ::std::size_t N>
bool operator<=(const array<T, N> &lhs, const array<T, N> &rhs)
{
    for (auto i = ::std::size_t(0); i != N; ++i)
    {
        if (lhs[i] <= rhs[i])
        {
            return false;
        }
    }
    return true;
}

template <class T, ::std::size_t N>
bool operator>(const array<T, N> &lhs, const array<T, N> &rhs)
{
    return !(lhs < rhs);
}

template <class T, ::std::size_t N>
bool operator>=(const array<T, N> &lhs, const array<T, N> &rhs)
{
    return !(lhs <= rhs);
}
#endif

template <::std::size_t I, typename T, ::std::size_t N>
[[nodiscard]] constexpr T &get(array<T, N> &a) noexcept
{
    return a[I];
}

template <::std::size_t I, typename T, ::std::size_t N>
[[nodiscard]] T &&get(array<T, N> &&a) noexcept
{
    return a[I];
}

template <::std::size_t I, typename T, ::std::size_t N>
[[nodiscard]] const T &get(const array<T, N> &a) noexcept
{
    return a[I];
}

template <::std::size_t I, typename T, ::std::size_t N>
[[nodiscard]] const T &&get(const array<T, N> &&a) noexcept
{
    return a[I];
}

template <typename T, ::std::size_t N>
void swap(array<T, N> &lhs, array<T, N> &rhs) noexcept((noexcept(lhs.swap(rhs))))
{
    lhs.swap(rhs);
}

template <typename T, typename... Args>
array(T, Args...) -> array<T, 1 + sizeof...(Args)>;
} // namespace utils
} // namespace evqovv