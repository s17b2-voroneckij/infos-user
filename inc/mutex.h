#pragma once

#include <infos.h>

extern "C" {
extern uint64_t atomic_exchange(uint64_t* a, uint64_t value);
extern void atomic_inc(uint64_t* a);
extern void atomic_dec(uint64_t* a);
}

class mutex
{
public:
    mutex() = default;

    ~mutex() {
        unlock();
    }

    mutex(const mutex&) = delete;

    mutex(mutex&&) = delete;

    mutex& operator=(const mutex&) = delete;

    mutex& operator=(mutex&&) = delete;

    void lock()
    {
        atomic_inc(&waiters);
        while (atomic_exchange(&is_taken, 1)) {
            syscall(Syscall::SYS_FUTEX_WAIT, (unsigned long)&is_taken, 1);
        }
        atomic_dec(&waiters);
    }

    void unlock() 
    {
        is_taken = 0;
        if (waiters) {
            syscall(Syscall::SYS_FUTEX_WAKE, (unsigned long)&is_taken, 1);
        }
    }

private:
    uint64_t waiters = 0;
    uint64_t is_taken = 0;
};

template <class T>
class unique_lock
{
public:
    unique_lock(T &m) : lock(m)
    {
        lock.lock();
    }

    ~unique_lock()
    {
        lock.unlock();
    }
private:
    T& lock;
};
