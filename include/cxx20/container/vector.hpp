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
    Alloc &a_;
    T first_;
    T &cur_;
    bool flag_ = true;

public:
    throw_guard(Alloc &a, T &res) noexcept : a_(a), first_(res), cur_(res)
    {
    }

    ~throw_guard()
    {
        if (flag_) [[unlikely]]
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
    throw_guard guard(a, b2);
    for (; b1 != e1 && b2 != e2; (void)++b1, (void)++b2)
    {
        ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(b2), *b1);
    }
    guard.release();
}

template <typename It, typename T, typename Alloc>
void fill(Alloc &a, It b, It e, const T &value)
{
    throw_guard guard(a, b);
    for (; b != e; (void)++b)
    {
        ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(b), value);
    }
    guard.release();
}

template <typename InIt, typename OutIt, typename Alloc>
void move_if_noexcept(Alloc &a, InIt b1, InIt e1, OutIt b2, OutIt e2)
{
    throw_guard guard(a, b2);
    for (; b1 != e1 && b2 != e2; (void)++b1, ((void)++b2))
    {
        ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(b2), ::std::move_if_noexcept(*b1));
    }
    guard.release();
}

template <typename It, typename Alloc>
void fill(Alloc &a, It b, It e)
{
    throw_guard guard(a, b);
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
    class alloc_guard
    {
        Alloc &a_;
        pointer data_;
        size_type cap_;
        bool flag_ = true;

    public:
        alloc_guard(Alloc &a, pointer data, size_type cap) noexcept : a_(a), data_(data), cap_(cap)
        {
        }

        void release() noexcept
        {
            flag_ = false;
        }

        ~alloc_guard()
        {
            if (flag_) [[likely]]
            {
                a_.deallocate(data_, cap_);
            }
        }
    };

    pointer data_{};

    size_type size_{};

    size_type cap_{};

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

    vector(const vector &other)
    {
        assign(other.cbegin(), other.cend());
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
            assign(other.cbegin(), other.cend());
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
        return *this;
    }

    void assign(size_type count, const T &value)
    {
        reserve(count);
        size_type i = 0;
        for (; i != size_; ++i)
        {
            data_[i] = value;
        }

        if (count < size_)
        {
            vector_detail::destroy(alloc_, data_ + count, data_ + size_);
        }

        if (count > size_)
        {
            vector_detail::fill(alloc_, data_ + i, data_ + count, value);
        }
        size_ = count;
    }

    template <typename InputIt>
    void assign(InputIt first, InputIt last)
    {
        size_type diff = last - first;
        reserve(diff);
        size_type i = 0;
        for (; i != size_; ++i, ++first)
        {
            data_[i] = first;
        }

        if (diff < size_)
        {
            vector_detail::destroy(alloc_, data_ + diff, data_ + size_);
        }

        if (diff > size_)
        {
            vector_detail::copy(alloc_, data_ + i, data_ + diff, first, last);
        }
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

    [[nodiscard]] reference index_unchecked(size_type pos)
    {
        return *(data_ + pos);
    }

    [[nodiscard]] const_reference index_unchecked(size_type pos) const
    {
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

    [[nodiscard]] constexpr reference front_unchecked() noexcept
    {
        return *data_;
    }

    [[nodiscard]] constexpr const_reference front_unchecked() const noexcept
    {
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

    [[nodiscard]] constexpr reference back_unchecked() noexcept
    {
        return *(data_ + size_ - 1);
    }

    [[nodiscard]] constexpr const_reference back_unchecked() const noexcept
    {
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

        size_type grown_cap = cap_ < 8 ? 8 : cap_;
        while (grown_cap < required_cap)
        {
            grown_cap += grown_cap / 2;
        }

        pointer new_data = alloc_.allocate(grown_cap);
        alloc_guard guard(alloc_, new_data, grown_cap);

        vector_detail::move_if_noexcept(alloc_, begin(), end(), new_data, new_data + size_);

        vector_detail::destroy(alloc_, begin(), end());

        alloc_.deallocate(data_, cap_);

        data_ = new_data;
        cap_ = grown_cap;

        guard.release();
    }

    void shrink_to_fit()
    {
        if (size_ == cap_) [[unlikely]]
        {
            return;
        }

        pointer new_data = alloc_.allocate(size_);
        alloc_guard guard(alloc_, new_data, size_);

        vector_detail::move_if_noexcept(alloc_, begin(), end(), new_data, new_data + size_);

        vector_detail::destroy(alloc_, begin(), end());

        alloc_.deallocate(data_, cap_);

        data_ = new_data;
        cap_ = size_;

        guard.release();
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
        size_type pos_i = pos - cbegin();
        reserve(size_ + count);
        for (size_type i = size_ + count - 1; i != pos_i; --i)
        {
            vector_detail::construct_at(alloc_, data_ + i, ::std::move(data_[i - count]));
            vector_detail::destroy_at(alloc_, data_ + i - count - 1);
        }
        vector_detail::fill(alloc_, data_ + pos_i, data_ + pos_i + count, value);
        size_ += count;
        return {data_ + pos_i};
    }

    template <typename InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        size_type pos_i = pos - cbegin();
        size_type diff = last - first;
        reserve(size_ + diff);
        for (size_type i = size_ + diff - 1; i != pos_i; --i)
        {
            vector_detail::construct_at(alloc_, data_ + i, ::std::move(data_[i - diff]));
            vector_detail::destroy_at(alloc_, data_ + i - diff - 1);
        }
        vector_detail::copy(alloc_, data_ + pos_i, data_ + pos_i + diff, first, last);
        size_ += diff;
        return {data_ + pos_i};
    }

    iterator insert(const_iterator pos, ::std::initializer_list<T> list)
    {
        return insert(pos, list.cbegin(), list.cend());
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args &&...args)
    {
        size_type pos_i = pos - cbegin();
        reserve(size_ + 1);
        for (size_type i = size_; i != pos_i; --i)
        {
            vector_detail::construct_at(alloc_, data_ + i, ::std::move(data_[i - 1]));
            vector_detail::destroy_at(alloc_, data_ + i - 1);
        }
        vector_detail::construct_at(alloc_, data_ + pos_i, ::std::forward<Args>(args)...);
        ++size_;
        return {data_ + pos_i};
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
        return const_cast<iterator>(first);
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
        return back();
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
            reserve(new_size);
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
            reserve(new_size);
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