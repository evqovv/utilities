#pragma once

#include <concepts>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>

namespace evqovv
{
namespace utils
{
namespace cxx20
{
template <typename T, typename Allocator = ::std::allocator<T>>
class vector
{
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = ::std::size_t;
    using difference_type = ::std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = typename ::std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename ::std::allocator_traits<Allocator>::const_pointer;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = ::std::reverse_iterator<iterator>;
    using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

    explicit vector(const Allocator &allocator) noexcept : allocator_(allocator)
    {
    }

    vector() noexcept(noexcept(Allocator())) : vector(Allocator())
    {
    }

    ~vector()
    {
    }

    vector(size_type count, const value_type & = value_type(), const Allocator &allocator = Allocator())
        : allocator_(allocator)
    {
    }

    reference at(size_type pos)
    {
        if (size_ <= pos) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + pos);
    }

    const_reference at(size_type pos) const
    {
        if (size_ <= pos) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + pos);
    }

    reference operator[](size_type pos)
    {
        if (size_ <= pos) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + pos);
    }

    const_reference operator[](size_type pos) const
    {
        if (size_ <= pos) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + pos);
    }

    reference front()
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *data_;
    }

    const_reference front() const
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *data_;
    }

    reference back()
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + size_ - 1);
    }

    const_reference back() const
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        return *(data_ + size_ - 1);
    }

    value_type *data()
    {
        return data;
    }

    const value_type *data() const
    {
        return data;
    }

    iterator begin() {
        return data_;
    }

    const_iterator begin() const {
        return data_;
    }

    const_iterator cbegin() const noexcept {
        return data_;
    }

    iterator end() {
        return data_ + size_;
    }

    const_iterator end() const {
        return data_ + size_;
    }

    const_iterator cend() const noexcept {
        return data_ + size_;
    }

    reverse_iterator rbegin() {
        return ::std::make_reverse_iterator(data_ + size_);
    }

    const_reverse_iterator rbegin() const {
        return ::std::make_reverse_iterator(data_);
    }

    const_reverse_iterator crbegin() const noexcept {
        return ::std::make_reverse_iterator(data_);
    }

    reverse_iterator rend() {
        return ::std::make_reverse_iterator(data_ + size_);
    }

    const_reverse_iterator rend() const {
        return ::std::make_reverse_iterator(data_);
    }

    const_reverse_iterator crend() const noexcept {
        return ::std::make_reverse_iterator(data_);
    }

    constexpr bool empty() const noexcept
    {
        return size_ == 0;
    }

    constexpr size_type size() const noexcept {
        return size_;
    }

    constexpr size_type max_size() const noexcept
    {
        return ::std::numeric_limits<::std::size_t>::max() / sizeof(value_type);
    }

    constexpr size_type capacity() const noexcept
    {
        return capacity_;
    }

    void push_back(const value_type &value)
    {
    }

    void push_back(value_type &&value)
    {

    }

    template <typename... Args>
    reference emplace_back(Args &&...args)
    {
    }

private:
    value_type *data_;
    size_type size_;
    size_type capacity_;
    [[no_unique_address]] Allocator allocator_;
};
} // namespace cxx20
} // namespace utils
} // namespace evqovv