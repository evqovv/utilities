#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace evqovv
{
namespace utils
{
namespace cxx20
{

template <typename T>
struct default_deleter
{
    constexpr default_deleter() noexcept = default;

    template <typename Tx>
        requires ::std::convertible_to<Tx *, T *>
    default_deleter(const default_deleter<Tx> &) noexcept
    {
    }

    void operator()(T *p) noexcept
    {
        delete p;
    }
};

template <typename T>
struct default_deleter<T[]>
{
    constexpr default_deleter() noexcept = default;

    template <typename Tx>
        requires ::std::same_as<::std::remove_cv_t<Tx>, ::std::remove_cv_t<T>>
    default_deleter(const default_deleter<Tx[]> &) noexcept
    {
    }

    void operator()(T *p) noexcept
    {
        delete[] p;
    }
};

template <typename T, typename D = default_deleter<T>>
class unique_ptr
{
    template <typename Tx, typename Dx>
    friend class unique_ptr;

public:
    using pointer = T *;
    using element_type = T;
    using deleter_type = D;

    unique_ptr() noexcept : ptr_(), deleter_()
    {
    }

    explicit unique_ptr(T *p) noexcept : ptr_(p), deleter_()
    {
    }

    template <typename Dx>
        requires ::std::constructible_from<D, const Dx &>
    unique_ptr(T *p, const Dx &d) noexcept : ptr_(p), deleter_(d)
    {
    }

    template <typename Dx>
        requires std::constructible_from<D, Dx>
    unique_ptr(T *p, Dx &&d) noexcept : ptr_(p), deleter_(::std::move(d))
    {
    }

    template <typename Tx, typename Dx>
        requires ::std::convertible_to<Tx *, T *>
    unique_ptr(unique_ptr<Tx, Dx> &&other) noexcept
        : ptr_(other.release()), deleter_(::std::forward<Dx>(other.deleter_))
    {
    }

    unique_ptr(unique_ptr &&other) noexcept : ptr_(other.release()), deleter_(::std::forward<D>(other.deleter_))
    {
    }

    ~unique_ptr()
    {
        if (ptr_) [[likely]]
        {
            deleter_(ptr_);
        }
    }

    unique_ptr &operator=(unique_ptr &&other) noexcept
    {
        reset(other.release());
        deleter_ = ::std::forward<D>(other.deleter_);
        return *this;
    }

    template <typename Tx, typename Dx>
    unique_ptr &operator=(unique_ptr<Tx, Dx> other)
    {
        reset(other.release());
        deleter_ = std::forward<Dx>(other.deleter_);
        return *this;
    }

    unique_ptr &operator=(::std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    ::std::add_lvalue_reference_t<T> operator*() const noexcept
    {
        return *ptr_;
    }

    T *operator->() const noexcept
    {
        return ptr_;
    }

    explicit operator bool() const noexcept
    {
        return ptr_ != nullptr;
    }

    T *release() noexcept
    {
        return ::std::exchange(ptr_, nullptr);
    }

    void reset(T *p = nullptr) noexcept
    {
        auto old_p = ::std::exchange(ptr_, p);
        if (old_p) [[likely]]
        {
            deleter_(old_p);
        }
    }

    void swap(unique_ptr &other) noexcept
    {
        using ::std::swap;
        swap(ptr_, other.ptr_);
        swap(deleter_, other.deleter_);
    }

    T *get() const noexcept
    {
        return ptr_;
    }

    D &get_deleter() noexcept
    {
        return deleter_;
    }

    const D &get_deleter() const noexcept
    {
        return deleter_;
    }

private:
    T *ptr_;
    [[no_unique_address]] D deleter_;
};

template <typename T, typename D>
class unique_ptr<T[], D>
{
public:
    using pointer = T *;
    using element_type = T;
    using deleter_type = D;

    unique_ptr() noexcept : ptr_(), deleter_()
    {
    }

    unique_ptr(T *p) noexcept : ptr_(p), deleter_()
    {
    }

    unique_ptr(T *p, const D &d) noexcept : ptr_(p), deleter_(d)
    {
    }

    unique_ptr(T *p, std::remove_reference_t<D> &&d) noexcept : ptr_(p), deleter_(::std::move(d))
    {
    }

    template <typename Tx, typename Dx>
    unique_ptr(unique_ptr<Tx, Dx> &&other) noexcept
        : ptr_(other.release()), deleter_(::std::forward<Dx>(other.deleter_))
    {
    }

    unique_ptr(unique_ptr &&other) noexcept : ptr_(other.release()), deleter_(::std::forward<D>(other.deleter_))
    {
    }

    ~unique_ptr()
    {
        if (ptr_) [[likely]]
        {
            deleter_(ptr_);
        }
    }

    unique_ptr &operator=(unique_ptr &&other) noexcept
    {
        reset(other.release());
        deleter_ = ::std::forward<D>(other.deleter_);
        return *this;
    }

    template <typename Tx, typename Dx>
    unique_ptr &operator=(unique_ptr<Tx, Dx> other)
    {
        reset(other.release());
        deleter_ = ::std::forward<Dx>(other.deleter_);
        return *this;
    }

    unique_ptr &operator=(::std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    explicit operator bool() const noexcept
    {
        return ptr_ != nullptr;
    }

    ::std::add_lvalue_reference_t<T> operator[](::std::size_t i) const
    {
        return ptr_[i];
    }

    T *release() noexcept
    {
        return ::std::exchange(ptr_, nullptr);
    }

    void reset(T *p = nullptr) noexcept
    {
        auto old_p = ::std::exchange(ptr_, p);
        if (old_p) [[likely]]
        {
            deleter_(old_p);
        }
    }

    void swap(unique_ptr &other) noexcept
    {
        using ::std::swap;
        swap(ptr_, other.ptr_);
        swap(deleter_, other.deleter_);
    }

    D &get_deleter() noexcept
    {
        return deleter_;
    }

    const D &get_deleter() const noexcept
    {
        return deleter_;
    }

private:
    T *ptr_;
    [[no_unique_address]] D deleter_;
};

template <typename T1, typename D1, typename T2, typename D2>
bool operator==(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
    return x.get() == y.get();
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator!=(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
    return x.get() != y.get();
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator<(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
    return ::std::less<typename unique_ptr<T1, D1>::pointer>()(x.get(), y.get());
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator<=(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
    return !::std::less<typename unique_ptr<T1, D1>::pointer>()(y.get(), x.get());
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator>(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
    return ::std::less<typename unique_ptr<T1, D1>::pointer>()(y.get(), x.get());
}

template <typename T1, typename D1, typename T2, typename D2>
bool operator>=(const unique_ptr<T1, D1> &x, const unique_ptr<T2, D2> &y)
{
    return !::std::less<typename unique_ptr<T1, D1>::pointer>()(x.get(), y.get());
}

template <typename T, typename D>
bool operator==(const unique_ptr<T, D> &x, ::std::nullptr_t) noexcept
{
    return !x;
}

template <typename T, typename D>
bool operator==(::std::nullptr_t, const unique_ptr<T, D> &x) noexcept
{
    return !x;
}

template <typename T, typename D>
bool operator!=(const unique_ptr<T, D> &x, ::std::nullptr_t) noexcept
{
    return static_cast<bool>(x);
}

template <typename T, typename D>
bool operator!=(::std::nullptr_t, const unique_ptr<T, D> &x) noexcept
{
    return static_cast<bool>(x);
}

template <typename T, typename... Args>
    requires(!::std::is_array_v<T>)
unique_ptr<T> make_unique(Args &&...args)
{
    return unique_ptr<T>(new T(::std::forward<Args>(args)...));
}

template <typename T, typename... Args>
    requires(::std::is_array_v<T>)
unique_ptr<T> make_unique(::std::size_t size)
{
    return unique_ptr<T>(new ::std::remove_extent_t<T>[size]());
}

template <typename T>
    requires(!::std::is_array_v<T>)
unique_ptr<T> make_unique_for_overwrite()
{
    return unique_ptr(new T);
}

template <typename T>
    requires(::std::is_array_v<T>)
unique_ptr<T> make_unique_for_overwrite(::std::size_t size)
{
    return unique_ptr(new ::std::remove_extent_t<T>[size]);
}
} // namespace cxx20
} // namespace utils
} // namespace evqovv