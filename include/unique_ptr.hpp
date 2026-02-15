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

#if __cplusplus == 202002L
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
#endif

#if __cplusplus == 201703L
namespace detail
{
template <typename T, typename = void>
struct is_complete_type_v : ::std::false_type
{
};

template <typename T>
struct is_complete_type_v<T, ::std::void_t<decltype(sizeof(T))>> : ::std::true_type
{
};

template <typename T, typename D, typename = void>
struct pointer
{
    using type = T *;
};

template <typename T, typename D>
struct pointer<T, D, ::std::void_t<typename ::std::remove_reference_t<D>::pointer>>
{
    using type = typename ::std::remove_reference_t<D>::pointer;
};

template <typename T, typename D>
using pointer_t = typename pointer<T, D>::type;

template <typename T>
constexpr bool is_unbounded_array_v = false;

template <typename T>
constexpr bool is_unbounded_array_v<T[]> = true;

template <typename T, typename D, bool = ::std::is_empty_v<D> && !::std::is_final_v<D>>
class unique_ptr_storage_type
{
public:
    using pointer = T;
    using deleter_type = D;

    constexpr unique_ptr_storage_type() : ptr_(), deleter_()
    {
    }

    template <typename Dx>
    constexpr unique_ptr_storage_type(pointer p, Dx &&d) : ptr_(::std::move(p)), deleter_(::std::forward<Dx>(d))
    {
    }

    pointer &get_ptr() noexcept
    {
        return ptr_;
    }
    const pointer &get_ptr() const noexcept
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
    pointer ptr_;
    D deleter_;
};

template <typename T, typename D>
class unique_ptr_storage_type<T, D, true> : private D
{
public:
    using pointer = T;
    using deleter_type = D;

    constexpr unique_ptr_storage_type() : D(), ptr_()
    {
    }

    template <typename Dx>
    constexpr unique_ptr_storage_type(pointer p, Dx &&d) : D(::std::forward<Dx>(d)), ptr_(::std::move(p))
    {
    }

    pointer &get_ptr() noexcept
    {
        return ptr_;
    }
    const pointer &get_ptr() const noexcept
    {
        return ptr_;
    }

    D &get_deleter() noexcept
    {
        return *this;
    }

    const D &get_deleter() const noexcept
    {
        return *this;
    }

private:
    pointer ptr_;
};
}; // namespace detail

template <typename T>
struct default_deleter
{
    constexpr default_deleter() noexcept = default;

    template <typename U, typename = ::std::enable_if_t<::std::is_convertible_v<U *, T *>>>
    default_deleter(const default_deleter<U> &) noexcept
    {
    }

    void operator()(T *ptr)
    {
        static_assert(detail::is_complete_type_v<T>(), "can't delete an incomplete type.");
        delete ptr;
    }
};

template <typename T>
struct default_deleter<T[]>
{
    void operator()(T *ptr)
    {
        static_assert(detail::is_complete_type_v<T>(), "can't delete an incomplete type.");
        delete[] ptr;
    }
};

template <typename T, typename D = default_deleter<T>>
class unique_ptr
{
public:
    using element_type = T;
    using pointer = detail::pointer_t<T, D>;
    using deleter_type = D;

private:
    detail::unique_ptr_storage_type<pointer, D> storage_;

public:
    template <typename Dx = D, ::std::enable_if_t<::std::is_lvalue_reference_v<Dx>, int> = 0>
    unique_ptr(pointer p, ::std::remove_reference_t<D> &&d) = delete;

    unique_ptr(const unique_ptr &other) = delete;

    unique_ptr &operator=(const unique_ptr &other) = delete;

    template <typename Dx = D,
              ::std::enable_if_t<!::std::is_pointer_v<Dx> && ::std::is_default_constructible_v<Dx>, int> = 0>
    constexpr unique_ptr() noexcept : storage_()
    {
    }

    template <typename Dx = D,
              ::std::enable_if_t<!::std::is_pointer_v<Dx> && ::std::is_default_constructible_v<Dx>, int> = 0>
    explicit unique_ptr(pointer p) noexcept : storage_(p, D())
    {
    }

    template <typename Dx = D,
              ::std::enable_if_t<!::std::is_pointer_v<D> && ::std::is_default_constructible_v<Dx>, int> = 0>
    constexpr unique_ptr(::std::nullptr_t) noexcept : storage_()
    {
    }

    template <typename Dx = D, ::std::enable_if_t<::std::is_constructible_v<Dx, const D &>, int> = 0>
    unique_ptr(pointer p, const D &d) noexcept : storage_(p, d)
    {
    }

    template <typename Dx = D,
              ::std::enable_if<!::std::is_reference_v<D> && ::std::is_constructible_v<Dx, Dx &&>, int> = 0>
    unique_ptr(pointer p, ::std::remove_reference_t<D> &&d) noexcept : storage_(p, ::std::move(d))
    {
    }

    unique_ptr(unique_ptr &&other) noexcept : storage_(other.release(), ::std::forward<D>(other.get_deleter()))
    {
    }

    template <typename U, typename E,
              typename = ::std::enable_if_t<::std::is_convertible_v<typename unique_ptr<U, E>::pointer, pointer> &&
                                            !::std::is_array_v<U> &&
                                            ((::std::is_reference_v<D> && ::std::is_same_v<D, E>) ||
                                             (!::std::is_reference_v<D> && ::std::is_convertible_v<E, D>))>>
    unique_ptr(unique_ptr<U, E> &&other) noexcept : storage_(other.release(), ::std::forward<E>(other.get_deleter()))
    {
    }

    ~unique_ptr()
    {
        if (get()) [[likely]]
        {
            get_deleter()(get());
        }
    }

    unique_ptr &operator=(unique_ptr &&other) noexcept
    {
        reset(other.release());
        get_deleter() = ::std::forward<D>(other.get_deleter());
        return *this;
    }

    unique_ptr &operator=(::std::nullptr_t) noexcept
    {
        reset();
        return *this;
    }

    void reset(pointer p = pointer()) noexcept
    {
        auto old_p = ::std::exchange(storage_.get_ptr(), p);
        if (old_p) [[likely]]
        {
            get_deleter()(old_p);
        }
    }

    pointer release() noexcept
    {
        return ::std::exchange(storage_.get_ptr(), nullptr);
    }

    void swap(unique_ptr &other) noexcept
    {
        using ::std::swap;
        swap(storage_.get_ptr(), other.storage_.get_ptr());
        swap(storage_.get_deleter(), other.storage_.get_deleter());
    }

    pointer get() const noexcept
    {
        return storage_.get_ptr();
    }

    deleter_type &get_deleter() noexcept
    {
        return storage_.get_deleter();
    }

    const deleter_type &get_deleter() const noexcept
    {
        return storage_.get_deleter();
    }

    explicit operator bool() const noexcept
    {
        return get() != nullptr;
    }

    typename ::std::add_lvalue_reference_t<T> operator*() const noexcept(noexcept(*::std::declval<pointer>()))
    {
        return *get();
    }

    pointer operator->() const noexcept
    {
        return get();
    }
};

template <typename T, typename D>
class unique_ptr<T[], D>
{
public:
    using pointer = T *;
    using element_type = T;
    using deleter_type = D;

private:
    detail::unique_ptr_storage_type<pointer, D> storage_;

    template <typename U>
    using is_safe_array_conversion = std::is_convertible<U (*)[], T (*)[]>;

    template <typename U>
    using is_pointer_compatible =
        std::disjunction<std::is_same<U, pointer>, std::is_same<U, std::nullptr_t>,
                         std::conjunction<std::is_same<pointer, element_type *>, std::is_pointer<U>,
                                          is_safe_array_conversion<std::remove_pointer_t<U>>>>;

public:
    template <typename U, typename Dx = D, ::std::enable_if_t<::std::is_lvalue_reference_v<Dx>, int> = 0>
    unique_ptr(U p, ::std::remove_reference_t<D> &&d) = delete;

    unique_ptr(const unique_ptr &) = delete;

    unique_ptr &operator=(const unique_ptr &) = delete;

    template <typename Dx = D, ::std::enable_if_t<!::std::is_pointer_v<Dx> && ::std::is_constructible_v<D>, int> = 0>
    constexpr unique_ptr() noexcept : storage_()
    {
    }

    template <typename Dx = D, ::std::enable_if_t<!::std::is_pointer_v<Dx> && ::std::is_constructible_v<D>, int> = 0>
    constexpr unique_ptr(::std::nullptr_t) noexcept : storage_()
    {
    }

    template <typename U, typename Dx = D,
              ::std::enable_if_t<!::std::is_pointer_v<Dx> && ::std::is_default_constructible_v<Dx> &&
                                     is_pointer_compatible<U>::value,
                                 int> = 0>
    explicit unique_ptr(U p) noexcept : storage_(p, D())
    {
    }

    template <typename U, typename Dx = D,
              ::std::enable_if<::std::is_constructible_v<Dx, const Dx &> && is_pointer_compatible<U>::value, int> = 0>
    unique_ptr(U p, const D &d) noexcept : storage_(p, d)
    {
    }

    template <typename U, typename Dx = D,
              ::std::enable_if_t<!::std::is_reference_v<Dx> && ::std::is_constructible_v<Dx, Dx &&> &&
                                     is_pointer_compatible<U>::value,
                                 int> = 0>
    unique_ptr(U p, ::std::remove_reference_t<D> &&d) noexcept : storage_(p, ::std::move(d))
    {
    }

    unique_ptr &operator=(unique_ptr &&other) noexcept
    {
        reset(other.release());
        get_deleter() = ::std::forward<D>(other.get_deleter());
        return *this;
    }

    unique_ptr(unique_ptr &&other) noexcept : storage_(other.release(), ::std::forward<D>(other.get_deleter()))
    {
    }

    template <typename U, typename E,
              ::std::enable_if_t<
                  ::std::is_array_v<U> && ::std::is_same_v<pointer, element_type *> && ::std::is_same_v<
                      typename unique_ptr<U, E>::pointer, typename unique_ptr<U, E>::element_type *>
                      && ::std::is_convertible_v<typename unique_ptr<U, E>::element_type (*)[], element_type (*)[]> &&
                  ((::std::is_reference_v<D> && ::std::is_same_v<D, E>) ||
                   (!::std::is_reference_v<D> && ::std::is_convertible_v<E, D>))>>
    unique_ptr(unique_ptr &&other) noexcept : storage_(other.release(), ::std::forward<E>(other.get_deleter()))
    {
    }

    template <
        typename U, typename E,
        typename = ::std::enable_if_t<::std::is_array_v<U> && ::std::is_convertible_v<
            typename unique_ptr<U, E>::element_type (*)[], element_type (*)[]> && ::std::is_assignable_v<D, E &&>>>
    unique_ptr &operator=(unique_ptr<U, E> &&other) noexcept
    {
        reset(other.release());
        get_deleter() = std::forward<E>(other.get_deleter());
        return *this;
    }

    pointer release() noexcept
    {
        return ::std::exchange(storage_.get_ptr(), pointer{});
    }

    void reset(pointer p = pointer{}) noexcept
    {
        auto old_p = ::std::exchange(storage_.get_ptr(), p);
        if (old_p) [[likely]]
        {
            get_deleter()(old_p);
        }
    }

    void swap(unique_ptr &other) noexcept
    {
        using ::std::swap;
        swap(storage_.get_ptr(), other.storage_.get_ptr());
        swap(storage_.get_deleter(), other.storage_.get_deleter());
    }

    pointer get() const noexcept
    {
        return storage_.get_ptr();
    }

    deleter_type &get_deleter() noexcept
    {
        return storage_.get_deleter();
    }

    const deleter_type &get_deleter() const noexcept
    {
        return storage_.get_deleter();
    }

    explicit operator bool() const noexcept
    {
        return get() != nullptr;
    }

    ::std::add_lvalue_reference_t<T> operator[](::std::size_t i) const
    {
        return get()[i];
    }
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

template <typename T, typename... Args, ::std::enable_if_t<!detail::is_unbounded_array_v<T>, int> = 0>
unique_ptr<T> make_unique(Args &&...args)
{
    return unique_ptr<T>(new T(::std::forward<Args>(args)...));
}

template <typename T, ::std::enable_if_t<!detail::is_unbounded_array_v<T>, int> = 0>
unique_ptr<T> make_unique_for_overwrite()
{
    return unique_ptr(new T);
}

template <typename T, ::std::enable_if_t<detail::is_unbounded_array_v<T>, int> = 0>
unique_ptr<T> make_unique(::std::size_t size)
{
    return unique_ptr(new ::std::remove_extent_t<T>[size]());
}

template <typename T, ::std::enable_if_t<detail::is_unbounded_array_v<T>, int> = 0>
unique_ptr<T> make_unique_for_overwrite(::std::size_t size)
{
    return unique_ptr(new ::std::remove_extent_t<T>[size]);
}
#endif
} // namespace utils
} // namespace evqovv