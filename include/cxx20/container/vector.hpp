#pragma once

#include <concepts>
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
        reserve(16);
    }

    vector() noexcept(noexcept(Allocator())) : vector(Allocator())
    {
        reserve(16);
    }

    vector(::std::initializer_list<value_type> init, const Allocator &allocator = Allocator())
    {
        reserve(init.size());
        for (size_type i = 0; i != init.size(); ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator, data_ + i, init[i]);
        }
        size_ = init.size();
    }

    vector(vector &&other) noexcept
        : data_(other.data_, nullptr), size_(other.size_), capacity_(other.capacity_),
          allocator_(::std::move(other.allocator_))
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    vector(size_type count, const value_type & = value_type(), const Allocator &allocator = Allocator())
        : allocator_(allocator)
    {
    }

    ~vector()
    {
        for (size_type i = 0; i != size_; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, data_ + i);
        }

        if (capacity_ != 0) [[likely]]
        {
            allocator_.deallocate(data_, capacity_);
        }
    }

    vector &operator=(const vector &other)
    {
        if (::std::addressof(other) != this) [[likely]]
        {
            for (size_type i = 0; i != size_; ++i)
            {
                ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, data_ + i);
            }

            if (capacity_ < other.size_)
            {
                reserve(other.size_ * 2);
            }
            for (size_type i = 0; i != other.size_; ++i)
            {
                ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i,
                                                                                 ::std::move(*(other.data_ + i)));
            }
            size_ = other.capacity_;
        }
        return *this;
    }

    vector &operator=(vector &&other)
    {
        if (::std::addressof(other) != this) [[likely]]
        {
            for (size_type i = 0; i != size_; ++i)
            {
                ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, data_ + i);
            }

            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            allocator_ = ::std::move(other.allocator_);
        }
        return *this;
    }

    vector &operator=(::std::initializer_list<value_type> list)
    {
        if (capacity_ < list.size())
        {
            reserve(list.size() * 2);
        }

        for (size_type i = 0; i != size_; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, data_ + i);
        }

        for (size_type i = 0; i != list.size(); ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i, list[i]);
        }
        size_ = list.size();
    }

    void assign(size_type count, const T &value)
    {
        if (capacity_ < count)
        {
            reserve(count * 2);
        }

        for (size_type i = 0; i != size_; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, data_ + i);
        }

        for (size_type i = 0; i != count; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i, value);
        }
        size_ = count;
    }

    template <class InputIt>
    void assign(InputIt first, InputIt last)
    {
        size_type count = last - first;
        if (capacity_ < count)
        {
            reserve(count * 2);
        }

        for (size_type i = 0; i != size_; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, data_ + i);
        }

        for (size_type i = 0; i != count; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i, first + i);
        }
        size_ = count;
    }

    void assign(::std::initializer_list<T> list)
    {
        if (capacity_ < list.size())
        {
            reserve(list.size() * 2);
        }

        for (size_type i = 0; i != size_; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, data_ + i);
        }

        for (size_type i = 0; i != list.size(); ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i, list[i]);
        }
        size_ = list.size();
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

    iterator begin()
    {
        return data_;
    }

    const_iterator begin() const
    {
        return data_;
    }

    const_iterator cbegin() const noexcept
    {
        return data_;
    }

    iterator end()
    {
        return data_ + size_;
    }

    const_iterator end() const
    {
        return data_ + size_;
    }

    const_iterator cend() const noexcept
    {
        return data_ + size_;
    }

    reverse_iterator rbegin()
    {
        return ::std::make_reverse_iterator(data_ + size_);
    }

    const_reverse_iterator rbegin() const
    {
        return ::std::make_reverse_iterator(data_);
    }

    const_reverse_iterator crbegin() const noexcept
    {
        return ::std::make_reverse_iterator(data_);
    }

    reverse_iterator rend()
    {
        return ::std::make_reverse_iterator(data_ + size_);
    }

    const_reverse_iterator rend() const
    {
        return ::std::make_reverse_iterator(data_);
    }

    const_reverse_iterator crend() const noexcept
    {
        return ::std::make_reverse_iterator(data_);
    }

    constexpr bool empty() const noexcept
    {
        return size_ == 0;
    }

    constexpr size_type size() const noexcept
    {
        return size_;
    }

    constexpr size_type max_size() const noexcept
    {
        return ::std::numeric_limits<::std::size_t>::max() / sizeof(value_type);
    }

    void reserve(size_type new_cap)
    {
        if (new_cap <= capacity_)
        {
            return;
        }

        value_type *new_data = allocator_.allocate(new_cap);

        for (size_type i = 0; i != size_; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>(allocator_, new_data + i, ::std::move(*(data_ + i)));
        }

        for (size_type i = 0; i != size_; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, new_data + i);
        }

        allocator_.deallocate(data_, size_);

        data_ = new_data;
        capacity_ = new_cap;
    }

    constexpr size_type capacity() const noexcept
    {
        return capacity_;
    }

    void clear() noexcept
    {
        for (size_type i = 0; i != size_; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, data_ + i);
        }
        size_ = 0;
    }

    iterator insert(const_iterator pos, const value_type &value)
    {
        if (size_ == capacity_)
        {
            reserve(capacity_ * 2 + 1);
        }

        size_type insert_pos = cend() - pos;
        for (size_type i = size_; i != insert_pos; --i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i,
                                                                             ::std::move(data_ + i - 1));
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, *(data_ + i - 1));
        }
        ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + insert_pos, value);
        ++size_;
    }

    iterator insert(const_iterator pos, T &&value)
    {
        if (size_ == capacity_)
        {
            reserve(capacity_ * 2 + 1);
        }

        size_type insert_pos = cend() - pos;
        for (size_type i = size_; i != (cend() - pos); --i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i,
                                                                             ::std::move(data_ + i - 1));
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, *(data_ + i - 1));
        }
        ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + insert_pos,
                                                                         ::std::move(value));
        ++size_;
    }

    iterator insert(const_iterator pos, size_type count, const T &value)
    {
        if (size_ + count >= capacity_)
        {
            reserve(capacity_ * 2 + count);
        }

        size_type insert_pos = cend() - pos;
        for (size_type i = size_ + count - 1; i != (cend() - pos); --i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i,
                                                                             ::std::move(data_ + i - 1));
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, *(data_ + i - 1));
        }

        for (size_type i = insert_pos; i < insert_pos + count; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + insert_pos, value);
        }
        size_ += count;
    }

    template <class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        size_type count = last - first;
        if (size_ + count >= capacity_)
        {
            reserve(capacity_ * 2 + count);
        }

        size_type insert_pos = cend() - pos;
        for (size_type i = size_ + count - 1; i != (cend() - pos); --i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i,
                                                                             ::std::move(data_ + i - 1));
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, *(data_ + i - 1));
        }

        for (size_type i = insert_pos; i < insert_pos + count; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + insert_pos,
                                                                             *(first + i));
        }
        size_ += count;
    }

    iterator insert(const_iterator pos, std::initializer_list<T> list)
    {
        size_type count = list.size();
        if (size_ + count >= capacity_)
        {
            reserve(capacity_ * 2 + count);
        }
        size_type insert_pos = cend() - pos;
        for (size_type i = size_ + count - 1; i != (cend() - pos); --i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i,
                                                                             ::std::move(data_ + i - 1));
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, *(data_ + i - 1));
        }

        for (size_type i = insert_pos; i < insert_pos + count; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + insert_pos, list[i]);
        }
        size_ += count;
    }

    iterator erase(const_iterator pos)
    {
        size_type idx = pos - cbegin();
        ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, pos);

        for (size_type i = idx; i != size_ - 1; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i,
                                                                             ::std::move(data_ + i + 1));
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, data_ + i + 1);
        }
        --size_;
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        size_type idx = first - cbegin();
        size_type count = last - first;
        for (auto it = first; it != last; ++it)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, it);
        }

        for (size_type i = idx; i != size_ - count; ++i)
        {
            ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + i,
                                                                             ::std::move(data_ + i + count));
            ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, data_ + i + count);
        }
        size_ -= count;
    }

    void push_back(const value_type &value)
    {
        if (size_ == capacity_)
        {
            reserve(capacity_ * 2);
        }

        ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + size_, value);

        ++size_;
    }

    void push_back(value_type &&value)
    {
        if (size_ == capacity_)
        {
            reserve(capacity_ * 2);
        }

        ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + size_, ::std::move(value));

        ++size_;
    }

    template <typename... Args>
    reference emplace_back(Args &&...args)
    {
        if (size_ == capacity_)
        {
            reserve(capacity_ * 2);
        }

        ::std::allocator_traits<::std::allocator<value_type>>::construct(allocator_, data_ + size_,
                                                                         ::std::forward<Args>(args)...);

        ++size_;
    }

    void pop_back()
    {
        if (empty()) [[unlikely]]
        {
            ::std::abort();
        }
        ::std::allocator_traits<::std::allocator<value_type>>::destroy(allocator_, end());
        --size_;
    }

    void resize(size_type new_size)
    {
        if (new_size > size_)
        {
            if (new_size > capacity_)
            {
                reserve(new_size * 2);
            }
            for (size_type i = 0; i < new_size; ++i)
            {
                ::std::allocator_traits<::std::allocator_traits<value_type>>::construct(allocator_,
                                                                                        begin() + size_ + i);
            }
        }
        if (new_size < size_)
        {
            for (size_type i = size_; i != new_size; --i)
            {
                ::std::allocator_traits<::std::allocator_traits<value_type>>::destroy(allocator_, begin() + i);
            }
        }
        size_ = new_size;
    }

    void resize(size_type new_size, const value_type &value)
    {
        if (new_size > size_)
        {
            if (new_size > capacity_)
            {
                reserve(new_size * 2);
            }
            for (size_type i = 0; i < new_size; ++i)
            {
                ::std::allocator_traits<::std::allocator_traits<value_type>>::construct(allocator_, begin() + size_ + i,
                                                                                        value);
            }
        }
        if (new_size < size_)
        {
            for (size_type i = size_; i != new_size; --i)
            {
                ::std::allocator_traits<::std::allocator_traits<value_type>>::destroy(allocator_, begin() + i);
            }
        }
        size_ = new_size;
    }

    void swap(vector &other)
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