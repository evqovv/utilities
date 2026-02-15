#pragma once

#include "helper.hpp"
#include <pthread.h>

namespace evqovv
{
namespace utils
{
class mutex
{
public:
    using native_handle_type = ::pthread_mutex_t *;

    mutex(const mutex &) = delete;
    mutex &operator=(const mutex &) = delete;

    mutex(mutex &&) = delete;
    mutex &operator=(mutex &&) = delete;

    constexpr mutex()
    {
        if (::pthread_mutex_init(&mutex_, nullptr) != 0)
        {
            throw_system_exception("pthread_mutex_init failed: ");
        }
    }

    ~mutex()
    {
        ::pthread_mutex_destroy(&mutex_);
    }

    void lock()
    {
        if (::pthread_mutex_lock(&mutex_) != 0)
        {
            throw_system_exception("pthread_mutex_lock failed: ");
        }
    }

    bool try_lock()
    {
        auto ret = ::pthread_mutex_trylock(&mutex_);
        if (ret == 0)
        {
            return true;
        }
        else if (ret == EBUSY)
        {
            return false;
        }
        else
        {
            throw_system_exception("pthread_mutex_trylock failed: ");
        }
    }

    void unlock()
    {
        if (::pthread_mutex_unlock(&mutex_) != 0)
        {
            throw_system_exception("pthread_mutex_unlock failed: ");
        }
    }

    [[nodiscard]] native_handle_type native_handle() noexcept
    {
        return &mutex_;
    }

private:
    ::pthread_mutex_t mutex_;
};
} // namespace utils
} // namespace evqovv