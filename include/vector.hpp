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
namespace vector_detail
{
template <typename It, typename Alloc>
class construction_guard
{
    Alloc &a_;
    It start_;
    It &cur_;
    bool flag_ = true;

public:
    construction_guard(Alloc &a, It &cur) noexcept : a_(a), start_(cur), cur_(cur)
    {
    }

    ~construction_guard()
    {
        if (flag_) [[unlikely]]
        {
            for (; start_ != cur_; (void)++start_)
            {
                ::std::allocator_traits<Alloc>::destroy(a_, ::std::to_address(start_));
            }
        }
    }

    void release() noexcept
    {
        flag_ = false;
    }
};

template <typename Alloc>
class raw_memory
{
    using atraits_t = ::std::allocator_traits<Alloc>;
    using pointer = typename atraits_t::pointer;
    using size_type = typename atraits_t::size_type;

    Alloc &a_;
    pointer data_{};
    size_type cap_{};
    bool flag_ = true;

public:
    raw_memory(Alloc &a, size_type cap) : a_(a), data_(a.allocate(cap)), cap_(cap)
    {
    }

    ~raw_memory()
    {
        if (flag_ && data_) [[unlikely]]
        {
            a_.deallocate(data_, cap_);
        }
    }

    pointer release() noexcept
    {
        flag_ = false;
        return data_;
    }

    [[nodiscard]] pointer get() noexcept
    {
        return data_;
    }
};

template <typename InIt, typename OutIt, typename Alloc>
void uninitialized_copy(Alloc &a, InIt b1, InIt e1, OutIt b2)
{
    construction_guard guard(a, b2);
    for (; b1 != e1; (void)++b1, (void)++b2)
    {
        ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(b2), *b1);
    }
    guard.release();
}

template <typename It, typename T, typename Alloc>
void uninitialized_fill(Alloc &a, It b, It e, const T &value)
{
    construction_guard guard(a, b);
    for (; b != e; (void)++b)
    {
        ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(b), value);
    }
    guard.release();
}

template <typename InIt, typename OutIt, typename Alloc>
void uninitialized_move_if_noexcept(Alloc &a, InIt b1, InIt e1, OutIt b2)
{
    construction_guard guard(a, b2);
    for (; b1 != e1; (void)++b1, (void)++b2)
    {
        ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(b2), ::std::move_if_noexcept(*b1));
    }
    guard.release();
}

template <typename It, typename Alloc>
void uninitialized_default_construct(Alloc &a, It b, It e)
{
    construction_guard guard(a, b);
    for (; b != e; (void)++b)
    {
        ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(b));
    }
    guard.release();
}

template <typename It, typename... Args, typename Alloc>
void construct_at(Alloc &a, It pos, Args &&...args)
{
    ::std::allocator_traits<Alloc>::construct(a, ::std::to_address(pos), ::std::forward<Args>(args)...);
}

template <typename It, typename Alloc>
void destroy_at(Alloc &a, It pos) noexcept
{
    ::std::allocator_traits<Alloc>::destroy(a, ::std::to_address(pos));
}

template <typename It, typename Alloc>
void destroy_range(Alloc &a, It b, It e) noexcept
{
    for (; b != e; (void)++b)
    {
        ::std::allocator_traits<Alloc>::destroy(a, ::std::to_address(b));
    }
}

} // namespace vector_detail

template <typename T, typename Alloc = ::std::allocator<T>>
class vector
{
    using atraits_t = ::std::allocator_traits<Alloc>;

public:
    using value_type = T;
    using allocator_type = Alloc;
    using size_type = typename atraits_t::size_type;
    using difference_type = typename atraits_t::difference_type;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = typename atraits_t::pointer;
    using const_pointer = typename atraits_t::const_pointer;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = ::std::reverse_iterator<iterator>;
    using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

    explicit vector(const Alloc &alloc) noexcept : alloc_(alloc)
    {
    }

    vector() noexcept(noexcept(Alloc())) : vector(Alloc())
    {
    }

    template <class InputIt>
    vector(InputIt first, InputIt last, const Alloc &alloc = Alloc()) : alloc_(alloc)
    {
        if constexpr (::std::forward_iterator<InputIt>)
        {
            auto count = ::std::distance(first, last);
            reserve(count);
            vector_detail::uninitialized_copy(alloc_, first, last, data_, data_ + count);
            size_ = count;
        }
        else
        {
            for (; first != last; (void)++first)
            {
                emplace_back(*first);
            }
        }
    }

    vector(::std::initializer_list<value_type> init, const Alloc &alloc = Alloc())
        : vector(init.begin(), init.end(), alloc)
    {
    }

    vector(const vector &other) : alloc_(atraits_t::select_on_container_copy_construction(other.alloc_))
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
        vector_detail::uninitialized_fill(alloc_, data_, data_ + count, value);
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

    void assign(size_type count, const T &value)
    {
        reserve(count);

        for (size_type i = 0; i != (count < size_) ? count : size_; ++i)
        {
            data_[i] = value;
        }
        if (count < size_)
        {
            vector_detail::destroy_range(alloc_, data_ + count, data_ + size_);
        }
        if (count > size_)
        {
            vector_detail::uninitialized_fill(alloc_, data_ + size_, data_ + size_ + count, value);
        }
        size_ = count;
    }

    template <typename InputIt>
    void assign(InputIt first, InputIt last)
    {
        auto count = last - first;
        reserve(count);

        for (size_type i = 0; i != (count < size_) ? count : size_; ++i, (void)++first)
        {
            data_[i] = *first;
        }

        if (count < size_)
        {
            vector_detail::destroy_range(alloc_, data_ + count, data_ + size_);
        }
        if (count > size_)
        {
            vector_detail::uninitialized_copy(alloc_, first, last, data_ + size_, data_ + size_ + count);
        }
        size_ = count;
    }

    void assign(::std::initializer_list<T> list)
    {
        assign(list.cbegin(), list.cend());
    }

    vector &operator=(const vector &other)
    {
        if (::std::addressof(other) == this) [[unlikely]]
        {
            return *this;
        }

        if constexpr (atraits_t::propagate_on_container_copy_assignment::value)
        {
            destroy_and_deallocate();
            data_ = nullptr;
            size_ = 0;
            cap_ = 0;
            alloc_ = other.alloc_;
        }
        assign(other.cbegin(), other.cend());

        return *this;
    }

    vector &operator=(vector &&other)
    {
        if (::std::addressof(other) == this) [[unlikely]]
        {
            return *this;
        }

        if constexpr (atraits_t::propagate_on_container_move_assignment::value)
        {
            destroy_and_deallocate();
            data_ = ::std::exchange(other.data_, nullptr);
            size_ = ::std::exchange(other.size_, 0);
            cap_ = ::std::exchange(other.cap_, 0);
            alloc_ = ::std::move(other.alloc_);
        }
        else
        {
            assign(::std::make_move_iterator(other.begin()), ::std::make_move_iterator(other.end()));
        }

        return *this;
    }

    vector &operator=(::std::initializer_list<value_type> list)
    {
        assign(list.cbegin(), list.cend());
        return *this;
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

    [[nodiscard]] allocator_type get_allocator() const noexcept
    {
        return alloc_;
    }

    void reserve(size_type required_cap)
    {
        if (required_cap <= cap_) [[unlikely]]
        {
            return;
        }

        reallocate(required_cap);
    }

    void shrink_to_fit()
    {
        if (size_ == cap_) [[unlikely]]
        {
            return;
        }

        if (size_ == 0) [[unlikely]]
        {
            destroy_and_deallocate();
            data_ = nullptr;
            cap_ = 0;
            return;
        }

        reallocate(size_);
    }

    void clear() noexcept
    {
        vector_detail::destroy_range(alloc_, begin(), end());
        size_ = 0;
    }

    iterator insert(const_iterator pos, const value_type &value)
    {
        return emplace_impl(index_of(pos), value);
    }

    iterator insert(const_iterator pos, T &&value)
    {
        return emplace_impl(index_of(pos), ::std::move(value));
    }

    iterator insert(const_iterator pos, size_type count, const T &value)
    {
        return insert_impl(index_of(pos), count, value);
    }

    template <typename InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {

        return insert_impl(index_of(pos), first, last);
    }

    iterator insert(const_iterator pos, ::std::initializer_list<T> list)
    {
        return insert_impl(index_of(pos), list.begin(), list.end());
    }

    template <typename... Args>
    iterator emplace(const_iterator pos, Args &&...args)
    {
        return emplace_impl(index_of(pos), ::std::forward<Args>(args)...);
    }

    iterator erase(const_iterator pos)
    {
        return erase_impl(pos, pos + 1);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        return erase_impl(first, last);
    }

    void push_back(const T &value)
    {
        (void)emplace_back_impl(value);
    }

    void push_back(T &&value)
    {
        (void)emplace_back_impl(::std::move(value));
    }

    template <typename... Args>
    reference emplace_back(Args &&...args)
    {
        return *emplace_back_impl(::std::forward<Args>(args)...);
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
        if (new_size < size_)
        {
            truncate_to(new_size);
        }
        if (new_size > size_)
        {
            append_default_n(new_size - size_);
        }
    }

    void resize(size_type new_size, const value_type &value)
    {
        if (new_size < size_)
        {
            truncate_to(new_size);
        }
        if (new_size > size_)
        {
            append_fill_n(new_size - size_, value);
        }
    }

    void swap(vector &other) noexcept(noexcept(::std::is_nothrow_swappable_v<pointer> &&
                                               ::std::is_nothrow_swappable_v<Alloc>))
    {
        using ::std::swap;
        swap(data_, other.data_);
        swap(size_, other.size_);
        swap(cap_, other.cap_);
        if constexpr (atraits_t::propagate_on_container_swap::value)
        {
            swap(alloc_, other.alloc_);
        }
    }

private:
    [[nodiscard]] size_type next_capacity(size_type required)
    {
        size_type cap = cap_ < 8 ? 8 : cap_;
        while (cap < required)
        {
            cap += cap / 2;
        }
        return cap;
    }

    [[nodiscard]] size_type index_of(const_iterator pos) const
    {
        if (data_ == nullptr)
        {
            if (pos != cbegin()) [[unlikely]]
            {
                ::std::abort();
            }
            return 0;
        }
        auto idx = static_cast<size_type>(pos - cbegin());
        if (idx > size_) [[unlikely]]
        {
            ::std::abort();
        }
        return idx;
    }

    iterator insert_impl(size_type pos_i, size_type count, const value_type &value)
    {
        if (cap_ < size_ + count)
        {
            auto new_cap = next_capacity(size_ + count);
            vector_detail::raw_memory raw(alloc_, new_cap);
            auto raw_p = raw.get();
            vector_detail::uninitialized_fill(alloc_, raw_p + pos_i, raw_p + pos_i + count, value);
            vector_detail::uninitialized_move_if_noexcept(alloc_, data_, data_ + pos_i, raw_p);
            vector_detail::uninitialized_move_if_noexcept(alloc_, data_ + pos_i, data_ + size_, raw_p + pos_i + count);
            destroy_and_deallocate();
            data_ = raw.release();
            size_ += count;
            cap_ = new_cap;
        }
        else
        {
            auto old_end = data_ + size_;
            vector_detail::uninitialized_fill(alloc_, old_end, old_end + count, value);
            size_ += count;
            ::std::rotate(data_ + pos_i, old_end, data_ + size_);
        }
        return data_ + pos_i;
    }

    template <typename It>
    iterator insert_impl(size_type pos_i, It b, It e)
    {
        auto count = ::std::distance(b, e);
        if (cap_ < size_ + count)
        {
            auto new_cap = next_capacity(size_ + count);
            vector_detail::raw_memory raw(alloc_, new_cap);
            auto raw_p = raw.get();
            vector_detail::uninitialized_copy(alloc_, raw_p + pos_i, raw_p + pos_i + count, b);
            vector_detail::uninitialized_move_if_noexcept(alloc_, data_, data_ + pos_i, raw_p);
            vector_detail::uninitialized_move_if_noexcept(alloc_, data_ + pos_i, data_ + size_, raw_p + pos_i + count);
            destroy_and_deallocate();
            data_ = raw.release();
            size_ += count;
            cap_ = new_cap;
        }
        else
        {
            auto old_end = data_ + size_;
            vector_detail::uninitialized_fill(alloc_, old_end, old_end + count, b);
            size_ += count;
            ::std::rotate(data_ + pos_i, old_end, data_ + size_);
        }
        return data_ + pos_i;
    }

    template <typename... Args>
    iterator emplace_impl(size_type pos_i, Args &&...args)
    {
        if (cap_ < size_ + 1)
        {
            auto new_cap = next_capacity(size_ + 1);
            vector_detail::raw_memory raw(alloc_, new_cap);
            auto raw_p = raw.get();
            vector_detail::construct_at(alloc_, raw_p + pos_i, ::std::forward<Args>(args)...);
            vector_detail::uninitialized_move_if_noexcept(alloc_, data_, data_ + pos_i, raw_p);
            vector_detail::uninitialized_move_if_noexcept(alloc_, data_ + pos_i, data_ + size_, raw_p + pos_i + 1);
            destroy_and_deallocate();
            data_ = raw.release();
            ++size_;
            cap_ = new_cap;
        }
        else
        {
            auto old_end = data_ + size_;
            vector_detail::construct_at(alloc_, old_end, ::std::forward<Args>(args)...);
            ++size_;
            ::std::rotate(data_ + pos_i, old_end, data_ + size_);
        }
        return data_ + pos_i;
    }

    template <typename... Args>
    iterator emplace_back_impl(Args &&...args)
    {
        if (cap_ < size_ + 1)
        {
            auto new_cap = next_capacity(size_ + 1);
            vector_detail::raw_memory raw(alloc_, new_cap);
            auto raw_p = raw.get();
            vector_detail::construct_at(alloc_, raw_p + size_, ::std::forward<Args>(args)...);
            vector_detail::uninitialized_move_if_noexcept(alloc_, data_, data_ + size_, raw_p);
            destroy_and_deallocate();
            data_ = raw.release();
            ++size_;
            cap_ = new_cap;
        }
        else
        {
            vector_detail::construct_at(alloc_, data_ + size_, ::std::forward<Args>(args)...);
            ++size_;
        }
        return end();
    }

    iterator erase_impl(const_iterator first, const_iterator last)
    {
        auto erase_count = last - first;

        auto dst = const_cast<iterator>(first);
        auto src = dst + erase_count;
        auto old_end = data_ + size_;

        for (; src != old_end; (void)++dst, (void)++src)
        {
            *dst = ::std::move(*src);
        }

        vector_detail::destroy_range(alloc_, old_end - erase_count, old_end);

        size_ -= erase_count;

        return const_cast<iterator>(first);
    }

    void destroy_and_deallocate()
    {
        if (data_) [[likely]]
        {
            vector_detail::destroy_range(alloc_, begin(), end());
            alloc_.deallocate(data_, cap_);
        }
    }

    void reallocate(size_type new_cap)
    {
        vector_detail::raw_memory raw(alloc_, new_cap);
        auto raw_p = raw.get();
        vector_detail::uninitialized_move_if_noexcept(alloc_, begin(), end(), raw_p);

        destroy_and_deallocate();

        data_ = raw.release();
        cap_ = new_cap;
    }

    void truncate_to(size_type new_size) noexcept
    {
        vector_detail::destroy_range(alloc_, data_ + new_size, data_ + size_);
        size_ = new_size;
    }

    void append_default_n(size_type count)
    {
        reserve(size_ + count);
        vector_detail::uninitialized_fill(alloc_, data_ + size_, data_ + size_ + count);
        size_ += count;
    }

    void append_fill_n(size_type count, const value_type &value)
    {
        reserve(size_ + count);
        vector_detail::uninitialized_fill(alloc_, data_ + size_, data_ + size_ + count, value);
        size_ += count;
    }

    pointer data_{};

    size_type size_{};

    size_type cap_{};

    [[no_unique_address]] Alloc alloc_{};
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
    auto min_size = (lhs.size() < rhs.size()) ? lhs.size() : rhs.size();
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
    for (; first != last; (void)++first)
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
    for (; first != last; (void)++first)
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
} // namespace utils
} // namespace evqovv