#pragma once

#include <cstddef>
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

    constexpr reference at(size_type pos)
    {
        if (pos >= N) [[unlikely]]
        {
            ::std::abort();
        }

        return eles_[pos];
    }

    constexpr const_reference at(size_type pos) const
    {
        if (pos >= N) [[unlikely]]
        {
            ::std::abort();
        }

        return eles_[pos];
    }

    constexpr reference operator[](size_type pos)
    {
        if (pos >= N) [[unlikely]]
        {
            ::std::abort();
        }

        return eles_[pos];
    }

    constexpr const_reference operator[](size_type pos) const
    {
        if (pos >= N) [[unlikely]]
        {
            ::std::abort();
        }

        return eles_[pos];
    }

    reference front() noexcept
    {
        return eles_[0];
    }

    const_reference front() const noexcept
    {
        return eles_[0];
    }

    reference back() noexcept
    {
        return eles_[N - 1];
    }

    const_reference back() const noexcept
    {
        return eles_[N - 1];
    }

    pointer data() noexcept
    {
        return eles_;
    }

    const_pointer data() const noexcept
    {
        return eles_;
    }

    iterator begin() noexcept
    {
        return eles_;
    }

    const_iterator begin() const noexcept
    {
        return eles_;
    }

    const_iterator cbegin() const noexcept
    {
        return eles_;
    }

    iterator end() noexcept
    {
        return eles_ + N;
    }

    const_iterator end() const noexcept
    {
        return eles_ + N;
    }

    const_iterator cend() const noexcept
    {
        return eles_ + N;
    }

    reverse_iterator rbegin() noexcept
    {
        return {end()};
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return {end()};
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return {end()};
    }

    reverse_iterator rend() noexcept
    {
        return {begin()};
    }

    const_reverse_iterator rend() const noexcept
    {
        return {begin()};
    }

    const_reverse_iterator crend() const noexcept
    {
        return {begin()};
    }

    constexpr bool empty() const noexcept
    {
        return false;
    }

    constexpr size_type size() const noexcept
    {
        return N;
    }

    constexpr size_type max_size() const noexcept
    {
        return N;
    }

    void fill(const_reference value) noexcept(::std::is_nothrow_copy_assignable_v<T>)
    {
        for (size_type i = 0; i != N; ++i)
        {
            eles_[i] = value;
        }
    }

    void swap(array &other) noexcept(::std::is_nothrow_swappable_v<T>)
    {
        for (size_t i = 0; i != N; ++i)
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

    constexpr reference at(size_type pos) noexcept
    {
        static_assert(false, "The length of this array is 0.");
    }

    constexpr const_reference at(size_type pos) const noexcept
    {
        static_assert(false, "The length of this array is 0.");
    }

    constexpr reference operator[](size_type pos) noexcept
    {
        static_assert(false, "The length of this array is 0.");
    }

    constexpr const_reference operator[](size_type pos) const noexcept
    {
        static_assert(false, "The length of this array is 0.");
    }

    constexpr reference front() noexcept
    {
        static_assert(false, "The length of this array is 0.");
    }

    constexpr const_reference front() const noexcept
    {
        static_assert(false, "The length of this array is 0.");
    }

    constexpr reference back() noexcept
    {
        static_assert(false, "The length of this array is 0.");
    }

    constexpr const_reference back() const noexcept
    {
        static_assert(false, "The length of this array is 0.");
    }

    constexpr pointer data() noexcept
    {
        return nullptr;
    }

    constexpr const_pointer data() const noexcept
    {
        return nullptr;
    }

    constexpr iterator begin() noexcept
    {
        return nullptr;
    }

    constexpr const_iterator begin() const noexcept
    {
        return nullptr;
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return nullptr;
    }

    constexpr iterator end() noexcept
    {
        return nullptr;
    }

    constexpr const_iterator end() const noexcept
    {
        return nullptr;
    }

    constexpr const_iterator cend() const noexcept
    {
        return nullptr;
    }

    constexpr reverse_iterator rbegin() noexcept
    {
        return nullptr;
    }

    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return nullptr;
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return nullptr;
    }

    constexpr reverse_iterator rend() noexcept
    {
        return nullptr;
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
        return nullptr;
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return nullptr;
    }

    constexpr bool empty() const noexcept
    {
        return true;
    }

    constexpr size_type size() const noexcept
    {
        return 0;
    }

    constexpr size_type max_size() const noexcept
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
    for (size_t i = 0; i != N; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }
    return true;
}

template <class T, ::std::size_t N>
bool operator!=(const array<T, N> &lhs, const array<T, N> &rhs)
{
    return !(lhs == rhs);
}

template <class T, ::std::size_t N>
bool operator<(const array<T, N> &lhs, const array<T, N> &rhs)
{
    for (size_t i = 0; i != N; ++i)
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
    for (size_t i = 0; i != N; ++i)
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

template <::std::size_t I, typename T, ::std::size_t N>
constexpr T &get(array<T, N> &a) noexcept
{
    return a[I];
}

template <::std::size_t I, typename T, ::std::size_t N>
T &&get(array<T, N> &&a) noexcept
{
    return a[I];
}

template <::std::size_t I, typename T, ::std::size_t N>
const T &get(const array<T, N> &a) noexcept
{
    return a[I];
}

template <::std::size_t I, typename T, ::std::size_t N>
const T &&get(const array<T, N> &&a) noexcept
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