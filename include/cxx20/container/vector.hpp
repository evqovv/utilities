#pragma once

#include <cstdlib>
#include <cstring>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace evqovv
{
namespace utils
{
namespace cxx20
{
namespace vector_detail
{
template <typename T, typename Alloc>
class throw_guard
{
    T first_;
    T &cur_;
    Alloc &a_;
    bool flag_ = true;

public:
    throw_guard(T &res, Alloc &a) noexcept : first_(res), cur_(res), a_(a)
    {
    }

    ~throw_guard()
    {
        if (flag_)
        {
            for (; first_ != cur_; ++first_)
            {
                ::std::allocator_traits<Alloc>::destroy(a_, ::std::to_address(first_));
            }
        }
    }

    void release() noexcept
    {
        flag_ = false;
    }
};

template <typename InIt, typename OutIt, typename Alloc>
void copy(Alloc &a, InIt b1, InIt e1, OutIt b2, OutIt e2)
{
    throw_guard guard(b2, a);
    for (; b1 != e1 && b2 != e2; (void)++b1, ((void)++b2))
    {
        ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(b2), *b1);
    }
    guard.release();
}

template <typename It, typename T, typename Alloc>
void fill(Alloc &a, It b, It e, const T &value)
{
    throw_guard guard(b, a);
    for (; b != e; (void)++b)
    {
        ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(b), value);
    }
    guard.release();
}

template <typename InIt, typename OutIt, typename Alloc>
void move(Alloc &a, InIt b1, InIt e1, OutIt b2, OutIt e2)
{
    throw_guard guard(b2, a);
    for (; b1 != e1 && b2 != e2; (void)++b1, ((void)++b2))
    {
        ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(b2), ::std::move(*b1));
    }
    guard.release();
}

template <typename It, typename Alloc>
void fill(Alloc &a, It b, It e)
{
    throw_guard guard(b, a);
    for (; b != e; (void)++b)
    {
        ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(b));
    }
    guard.release();
}

template <typename It, typename Alloc>
void destroy(Alloc &a, It b, It e) noexcept
{
    for (; b != e; ++b)
    {
        ::std::allocator_traits<Alloc>::destroy(a, ::std::to_address(b));
    }
}

} // namespace vector_detail

template <typename T, typename Alloc = ::std::allocator<T>>
class vector
{
public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = ::std::size_t;
    using difference_type = ::std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = typename ::std::allocator_traits<Alloc>::pointer;
    using const_pointer = typename ::std::allocator_traits<Alloc>::const_pointer;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = ::std::reverse_iterator<iterator>;
    using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

private:
    pointer data_;

    size_type size_;

    size_type cap_;

    [[no_unique_address]] Alloc alloc_;

    using atraits_t = ::std::allocator_traits<allocator_type>;

public:
    explicit vector(const Alloc &allocator) noexcept : alloc_(allocator)
    {
    }

    vector() noexcept(noexcept(Alloc())) : vector(Alloc())
    {
    }

    vector(::std::initializer_list<value_type> init, const Alloc &allocator = Alloc())
    {
        reserve(init.size());
        vector_detail::copy(alloc_, data_, data_ + init.size(), init.cbegin(), init.cend());
        size_ = init.size();
    }

    vector(vector &&other) noexcept
        : data_(::std::exchange(other.data_, nullptr)), size_(::std::exchange(other.size_, 0)),
          cap_(::std::exchange(other.cap_, 0)), alloc_(::std::move(other.alloc_))
    {
    }

    explicit vector(size_type count, const value_type &value = value_type(), const Alloc &allocator = Alloc())
        : alloc_(allocator)
    {
        reserve(count);
        vector_detail::fill(alloc_, data_, data_ + count, value);
        size_ = count;
    }

    ~vector()
    {
        clear();
        if (data_) [[likely]]
        {
            alloc_.deallocate(data_, cap_);
        }
    }

    vector &operator=(const vector &other)
    {
        if (::std::addressof(other) != this) [[likely]]
        {
            clear();
            reserve(other.size_);
            vector_detail::move(alloc_, data_, data_ + other.size_, other.cbegin(), other.cend());
            size_ = other.size_;
        }
        return *this;
    }

    vector &operator=(vector &&other)
    {
        if (::std::addressof(other) != this) [[likely]]
        {
            clear();
            if (data_) [[likely]]
            {
                alloc_.deallocate(data_, cap_);
            }

            data_ = ::std::exchange(other.data_, nullptr);
            size_ = ::std::exchange(other.size_, 0);
            cap_ = ::std::exchange(other.cap_, 0);
            alloc_ = ::std::move(other.alloc_);
        }
        return *this;
    }

    vector &operator=(::std::initializer_list<value_type> list)
    {
        clear();
        reserve(list.size());

        for (size_type i = 0; i != list.size(); ++i)
        {
            atraits_t::construct(alloc_, data_ + i, list[i]);
        }
        size_ = list.size();
    }

    void assign(size_type count, const T &value)
    {
        clear();
        reserve(count);
        vector_detail::fill(alloc_, data_, data_ + count, value);
        size_ = count;
    }

    template <class InputIt>
    void assign(InputIt first, InputIt last)
    {
        clear();
        size_type diff = last - first;
        reserve(diff);
        vector_detail::copy(alloc_, data_, data_ + diff, first, last);
        size_ = diff;
    }

    void assign(::std::initializer_list<T> list)
    {
        assign(list.cbegin(), list.cend());
    }

    [[nodiscard]] allocator_type get_allocator() const
    {
        return alloc_;
    }

    [[nodiscard]] reference at(size_type pos)
    {
        if (size_ <= pos) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + pos);
    }

    [[nodiscard]] const_reference at(size_type pos) const
    {
        if (size_ <= pos) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + pos);
    }

    [[nodiscard]] reference operator[](size_type pos)
    {
        if (size_ <= pos) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + pos);
    }

    [[nodiscard]] const_reference operator[](size_type pos) const
    {
        if (size_ <= pos) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + pos);
    }

    [[nodiscard]] reference front()
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *data_;
    }

    [[nodiscard]] const_reference front() const
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *data_;
    }

    [[nodiscard]] reference back()
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + size_ - 1);
    }

    [[nodiscard]] const_reference back() const
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + size_ - 1);
    }

    [[nodiscard]] value_type *data()
    {
        return data_;
    }

    [[nodiscard]] const value_type *data() const
    {
        return data_;
    }

    [[nodiscard]] iterator begin()
    {
        return data_;
    }

    [[nodiscard]] const_iterator begin() const
    {
        return data_;
    }

    [[nodiscard]] const_iterator cbegin() const noexcept
    {
        return data_;
    }

    [[nodiscard]] iterator end()
    {
        return data_ + size_;
    }

    [[nodiscard]] const_iterator end() const
    {
        return data_ + size_;
    }

    [[nodiscard]] const_iterator cend() const noexcept
    {
        return data_ + size_;
    }

    [[nodiscard]] reverse_iterator rbegin()
    {
        return ::std::make_reverse_iterator(data_ + size_);
    }

    [[nodiscard]] const_reverse_iterator rbegin() const
    {
        return ::std::make_reverse_iterator(data_);
    }

    [[nodiscard]] const_reverse_iterator crbegin() const noexcept
    {
        return ::std::make_reverse_iterator(data_);
    }

    [[nodiscard]] reverse_iterator rend()
    {
        return ::std::make_reverse_iterator(data_ + size_);
    }

    [[nodiscard]] const_reverse_iterator rend() const
    {
        return ::std::make_reverse_iterator(data_);
    }

    [[nodiscard]] const_reverse_iterator crend() const noexcept
    {
        return ::std::make_reverse_iterator(data_);
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return size_ == 0;
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return size_;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
        return ::std::numeric_limits<::std::size_t>::max() / sizeof(value_type);
    }

    void reserve(size_type required_cap)
    {
        if (required_cap <= cap_) [[unlikely]]
        {
            return;
        }
        size_type base_cap = cap_ == 0 ? 8 : cap_;

        size_type grown_cap = base_cap + base_cap / 2;
        while (grown_cap < required_cap)
        {
            grown_cap += grown_cap / 2;
        }

        pointer old_ptr = data_;
        size_type prev_cap = cap_;

        data_ = alloc_.allocate(grown_cap);
        cap_ = grown_cap;

        vector_detail::move(alloc_, old_ptr, old_ptr + size_, data_, data_ + size_);
        vector_detail::destroy(alloc_, old_ptr, old_ptr + size_);

        alloc_.deallocate(old_ptr, prev_cap);
    }

    [[nodiscard]] constexpr size_type capacity() const noexcept
    {
        return cap_;
    }

    void shrink_to_fit()
    {
        if (size_ == cap_) [[unlikely]]
        {
            return;
        }

        pointer old_data = data_;
        size_type old_cap = cap_;

        data_ = alloc_.allocate(size_);
        cap_ = size_;

        vector_detail::move(alloc_, old_data, old_data + size_, data_, data_ + size_);
        vector_detail::destroy(alloc_, old_data, old_data + size_);

        alloc_.deallocate(old_data, old_cap);
    }

    void clear() noexcept
    {
        vector_detail::destroy(begin(), end());
        size_ = 0;
    }

    iterator insert(const_iterator pos, const value_type &value)
    {
        return emplace(pos, value);
    }

    iterator insert(const_iterator pos, T &&value)
    {
        return emplace(pos, ::std::move(value));
    }

    iterator insert(const_iterator pos, size_type count, const T &value)
    {
        reserve(size_ + count);
        for (size_type i = size_ + count - 1; i != pos; --i)
        {
            atraits_t::construct(::std::to_address(data_ + i), ::std::move(data_[i - 1]));
            atraits_t::destroy(::std::to_address(data_ + i - 1));
        }
        vector_detail::fill(alloc_, pos, pos + count, value);
        size_ += count;
    }

    template <class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        size_type diff = last - first;
        reserve(size_ + diff);
        for (size_type i = size_ + diff - 1; i != pos; --i)
        {
            atraits_t::construct(::std::to_address(data_ + i), ::std::move(data_[i - 1]));
            atraits_t::destroy(::std::to_address(data_ + i - 1));
        }
        vector_detail::copy(alloc_, pos, pos + diff, first, last);
        size_ += diff;
    }

    iterator insert(const_iterator pos, ::std::initializer_list<T> list)
    {
        insert(pos, list.cbegin(), list.cend());
    }

    template <class... Args>
    iterator emplace(const_iterator pos, Args &&...args)
    {
        reserve(size_ + 1);
        size_type idx = pos - cbegin();
        for (size_type i = size_ - 1; i != pos; --i)
        {
            atraits_t::construct(::std::to_address(data_ + i), ::std::move(data_[i - 1]));
            atraits_t::destroy(::std::to_address(data_ + i - 1));
        }
        atraits_t::construct(alloc_, data_ + idx, ::std::forward<Args>(args)...);
        ++size_;
    }

    iterator erase(const_iterator pos)
    {
        return erase(pos, pos + 1);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        size_type diff = last - first;
        for (size_type i = last - cbegin(); i != size_; ++i)
        {
            data_[i - diff] = ::std::move(data_[i]);
        }
        for (size_type i = size_ - diff; i != size_; ++i)
        {
            atraits_t::destroy(::std::to_address(data_ + i));
        }

        size_ -= diff;
    }

    void push_back(const T &value)
    {
        emplace_back(value);
    }

    void push_back(T &&value)
    {
        emplace_back(::std::move(value));
    }

    template <typename... Args>
    reference emplace_back(Args &&...args)
    {
        reserve(size_ + 1);
        atraits_t::construct(alloc_, ::std::to_address(data_ + size_), ::std::forward<Args>(args)...);
        ++size_;
    }

    void pop_back()
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        atraits_t::destroy(alloc_, ::std::to_address(data_ + size_ - 1));
        --size_;
    }

    void resize(size_type new_size)
    {
        if (new_size > size_)
        {
            resize(new_size);
            vector_detail::fill(alloc_, data_ + size_, data_ + new_size);
        }
        if (new_size < size_)
        {
            vector_detail::destroy(alloc_, data_ + new_size, data_ + size_);
        }
        size_ = new_size;
    }

    void resize(size_type new_size, const value_type &value)
    {
        if (new_size > size_)
        {
            resize(new_size);
            vector_detail::fill(alloc_, data_ + size_, data_ + new_size, value);
        }
        if (new_size < size_)
        {
            vector_detail::destroy(alloc_, data_ + new_size, data_ + size_);
        }
        size_ = new_size;
    }

    void swap(vector &other) noexcept(noexcept(::std::is_nothrow_swappable_v<pointer> &&
                                               ::std::is_nothrow_swappable_v<Alloc>))
    {
        using ::std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(cap_, other.cap_);
        swap(alloc_, other.alloc_);
    }
};
} // namespace cxx20
} // namespace utils
} // namespace evqovv