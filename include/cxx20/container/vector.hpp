#pragma once

#include <algorithm>
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

template <typename It, typename... Args, typename Alloc>
void construct_at(Alloc &a, It pos, Args... args)
{
    ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(pos), ::std::forward<Args>(args)...);
}

template <typename It, typename Alloc>
void destroy_at(Alloc &a, It pos)
{
    ::std::allocator_traits<Alloc>::destroy(a, ::std::to_address(pos));
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

public:
    explicit vector(const Alloc &alloc) noexcept : alloc_(alloc)
    {
    }

    vector() noexcept(noexcept(Alloc())) : vector(Alloc())
    {
    }

    vector(::std::initializer_list<value_type> init, const Alloc &alloc = Alloc())
    {
        reserve(init.size());
        vector_detail::copy(alloc_, init.begin(), init.end(), data_, data_ + init.size());
        size_ = init.size();
    }

    vector(vector &&other) noexcept
        : data_(::std::exchange(other.data_, nullptr)), size_(::std::exchange(other.size_, 0)),
          cap_(::std::exchange(other.cap_, 0)), alloc_(::std::move(other.alloc_))
    {
    }

    explicit vector(size_type count, const value_type &value = value_type(), const Alloc &alloc = Alloc())
        : alloc_(alloc)
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
        assign(list.cbegin(), list.cend());
    }

    void assign(size_type count, const T &value)
    {
        clear();
        reserve(count);
        vector_detail::fill(alloc_, data_, data_ + count, value);
        size_ = count;
    }

    template <typename InputIt>
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

    [[nodiscard]] allocator_type get_allocator() const noexcept
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

    [[nodiscard]] constexpr reference front() noexcept
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *data_;
    }

    [[nodiscard]] constexpr const_reference front() const noexcept
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *data_;
    }

    [[nodiscard]] constexpr reference back() noexcept
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + size_ - 1);
    }

    [[nodiscard]] constexpr const_reference back() const noexcept
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + size_ - 1);
    }

    [[nodiscard]] constexpr pointer data() noexcept
    {
        return data_;
    }

    [[nodiscard]] constexpr const_pointer data() const noexcept
    {
        return data_;
    }

    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return data_;
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return data_;
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return data_;
    }

    [[nodiscard]] constexpr iterator end() noexcept
    {
        return data_ + size_;
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return data_ + size_;
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return data_ + size_;
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept
    {
        return ::std::make_reverse_iterator(data_ + size_);
    }

    [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept
    {
        return ::std::make_reverse_iterator(data_);
    }

    [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
    {
        return ::std::make_reverse_iterator(data_);
    }

    [[nodiscard]] constexpr reverse_iterator rend() noexcept
    {
        return ::std::make_reverse_iterator(data_ + size_);
    }

    [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept
    {
        return ::std::make_reverse_iterator(data_);
    }

    [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept
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

    [[nodiscard]] constexpr size_type capacity() const noexcept
    {
        return cap_;
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
        vector_detail::destroy(alloc_, begin(), end());
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
        size_type pos_i = pos - cbegin();
        for (size_type i = size_ + count - 1; i != pos_i; --i)
        {
            vector_detail::construct_at(alloc_, data_ + i, ::std::move(data_[i - 1]));
            vector_detail::destroy_at(alloc_, data_ + i - 1);
        }
        vector_detail::fill(alloc_, pos, pos + count, value);
        size_ += count;
    }

    template <typename InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        size_type diff = last - first;
        reserve(size_ + diff);
        size_type pos_i = pos - cbegin();
        for (size_type i = size_ + diff - 1; i != pos_i; --i)
        {
            vector_detail::construct_at(alloc_, data_ + i, ::std::move(data_[i - 1]));
            vector_detail::destroy_at(alloc_, data_ + i - 1);
        }
        vector_detail::copy(alloc_, pos, pos + diff, first, last);
        size_ += diff;
    }

    iterator insert(const_iterator pos, ::std::initializer_list<T> list)
    {
        insert(pos, list.cbegin(), list.cend());
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args &&...args)
    {
        reserve(size_ + 1);
        size_type pos_i = pos - cbegin();
        for (size_type i = size_ - 1; i != pos_i; --i)
        {
            vector_detail::construct_at(alloc_, data_ + i, ::std::move(data_[i - 1]));
            vector_detail::destroy_at(alloc_, data_ + i - 1);
        }
        vector_detail::construct_at(alloc_, pos, ::std::forward<Args>(args)...);
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
            vector_detail::destroy_at(alloc_, data_ + i);
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
        vector_detail::construct_at(alloc_, data_ + size_, ::std::forward<Args>(args)...);
        ++size_;
    }

    void pop_back()
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        vector_detail::destroy_at(alloc_, data_ + size_ - 1);
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

template <typename T, typename Alloc>
void swap(vector<T, Alloc> &lhs, vector<T, Alloc> &rhs) noexcept(noexcept(lhs.swap(rhs)))
{
    lhs.swap(rhs);
}

template <typename T, typename Alloc>
bool operator==(const vector<T, Alloc> &lhs, const vector<T, Alloc> &rhs)
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }

    for (decltype(lhs.size()) i = 0; i != lhs.size(); ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }

    return true;
}

template <class T, class Alloc>
constexpr auto operator<=>(const vector<T, Alloc> &lhs, const vector<T, Alloc> &rhs)
{
    decltype(lhs.size()) min_size = (lhs.size() < rhs.size()) ? lhs.size() : rhs.size();
    for (decltype(lhs.size()) i = 0; i != min_size; ++i)
    {
        auto cmp = lhs[i] <=> rhs[i];
        if (cmp != 0)
        {
            return cmp;
        }
    }
    return lhs.size() <=> rhs.size();
}

template <typename T, typename Alloc, typename U = T>
constexpr typename vector<T, Alloc>::size_type erase(vector<T, Alloc> &c, const U &value)
{
    auto old_size = c.size();

    auto first = c.begin();
    auto last = c.end();

    first = ::std::find(first, last, value);

    if (first == last)
    {
        return 0;
    }

    auto dest = first;
    ++first;
    for (; first != last; ++first)
    {
        if (*first != value)
        {
            *dest = ::std::move(*first);
            ++dest;
        }
    }

    c.erase(dest, last);
    return old_size - c.size();
}

template <typename T, typename Alloc, typename Pred>
constexpr typename vector<T, Alloc>::size_type erase_if(vector<T, Alloc> &c, Pred pred)
{
    auto old_size = c.size();

    auto first = c.begin();
    auto last = c.end();

    first = ::std::find_if(first, last, pred);

    if (first == last)
    {
        return 0;
    }

    auto dest = first;
    ++first;
    for (; first != last; ++first)
    {
        if (!pred(*first))
        {
            *dest = ::std::move(*first);
            ++dest;
        }
    }

    c.erase(dest, last);
    return old_size - c.size();
}
} // namespace cxx20
} // namespace utils
} // namespace evqovv