#pragma once

#include <infos-c.h>

extern uint64_t atomic_exchange(uint64_t* a, uint64_t value);
extern void atomic_inc(uint64_t* a);
extern void atomic_dec(uint64_t* a);

struct mutex
{
    uint64_t waiters;
    uint64_t is_taken;
};

void mutex_lock(struct mutex* m)
{
    atomic_inc(&m->waiters);
    while (atomic_exchange(&m->is_taken, 1)) {
        syscall(SYS_FUTEX_WAIT, (unsigned long)&m->is_taken, 1, 0, 0);
    }
    atomic_dec(&m->waiters);
}

void mutex_unlock(struct mutex* m) 
{
    m->is_taken = 0;
    if (m->waiters) {
        syscall(SYS_FUTEX_WAKE, (unsigned long)&m->is_taken, 1, 0, 0);
    }
}