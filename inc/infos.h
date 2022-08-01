/* SPDX-License-Identifier: MIT */

#pragma once

extern "C" {
#include "infos-c.h"
}

enum SchedulingEntityPriority
{
    REALTIME = 0,
    INTERACTIVE = 1,
    NORMAL = 2,
    DAEMON = 3,
    IDLE = 4,
};

typedef void (*ThreadProc)(void *);
extern HTHREAD create_thread(ThreadProc tp, void *arg, SchedulingEntityPriority priority = SchedulingEntityPriority::NORMAL);
extern void stop_thread(HTHREAD thread);
extern void join_thread(HTHREAD thread);
extern void set_thread_name(HTHREAD thread, const char *name);
extern void usleep(unsigned long us);

unsigned long syscall(Syscall nr);

unsigned long syscall(Syscall nr, unsigned long a1);

unsigned long syscall(Syscall nr, unsigned long a1, unsigned long a2);

unsigned long syscall(Syscall nr, unsigned long a1, unsigned long a2, unsigned long a3);

extern char getch();

static inline bool is_error(HANDLE h)
{
	return h == (HANDLE)-1;
}

struct PageFaultException {
    explicit PageFaultException(uint64_t a) : addr(a) {}

    uint64_t addr;
};

extern void process_elf(const char* FILE_NAME);
extern void exceptions_clean_up();

extern void my_assert(bool b);

struct thread_entry_arg {
	ThreadProc f;
	void* ptr;
};

extern void thread_func(thread_entry_arg* arg);
extern void prepare_thread_local();
extern void clean_up_thread_local();
