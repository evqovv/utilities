#pragma once

#include <memory>
#include <type_traits>

namespace evqovv {
namespace utils {
namespace detail {
template <typename Allocator,
          bool = ::std::is_empty_v<Allocator> && !::std::is_final_v<Allocator>>
class vector_policy_base {};

template <typename Allocator>
class vector_policy_base<Allocator, true> : private Allocator {
protected:
    constexpr Allocator &get_deleter() noexcept {
        return *this;
    }
    constexpr const Allocator &get_deleter() const noexcept {
        return *this;
    }
};

template <typename Allocator>
class vector_policy_base<Allocator, false> {
protected:
    constexpr Allocator &get_deleter() noexcept {
        return allocator_;
    }
    constexpr const Allocator &get_deleter() const noexcept {
        return allocator_;
    }

private:
    Allocator allocator_;
};
} // namespace detail
template <typename T, typename Allocator = ::std::allocator<T>>
class vector : private detail::vector_policy_base<Allocator> {
    using base = detail::vector_policy_base<Allocator>;

public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = ::std::size_t;
    using difference_type = ::std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = typename ::std::allocator_traits<Allocator>::pointer;
    using const_pointer =
        typename ::std::allocator_traits<Allocator>::const_pointer;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = ::std::reverse_iterator<iterator>;
    using const_reverse_iterator = ::std::reverse_iterator<const_iterator>;

    vector() : base(Allocator()) {

    }

    ~vector() {

    }

    private:
    value_type * data_;
    ::std::size_t size_;
    ::std::size_t capacity_;
    
     
};
} // namespace utils
} // namespace evqovv