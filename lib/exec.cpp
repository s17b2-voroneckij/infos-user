/* SPDX-License-Identifier: MIT */

#include <infos.h>
#include <abi.h>

HPROC exec(const char *program, const char *args)
{
	return (HPROC)syscall(Syscall::SYS_EXEC, (unsigned long)program, (unsigned long)args);
}

void wait_proc(HPROC proc)
{
	syscall(Syscall::SYS_WAIT_PROC, proc);
}

extern void run_thread_atexit();

class ThreadLocalHolder {
public:
    ThreadLocalHolder(thread_entry_arg* arg) {
        prepare_thread_local();
        ptr = arg;
    }

    ~ThreadLocalHolder() {
        run_thread_atexit();
        clean_up_thread_local();
        delete ptr;
        syscall(Syscall::SYS_THREAD_LEAVE);
    }
private:
    thread_entry_arg* ptr;
};

void thread_func(thread_entry_arg* arg) {
    ThreadLocalHolder holder(arg);
    try {
        arg->f(arg->ptr);
    } catch (...) {
        printf("thread leaving due to exception of type %s\n", __cxa_last_exception_name());
    }
}

HTHREAD create_thread(ThreadProc tp, void *arg, SchedulingEntityPriority priority)
{
	auto thread_arg = new thread_entry_arg();
	thread_arg->f = tp;
	thread_arg->ptr = arg;
    return (HTHREAD)syscall(Syscall::SYS_CREATE_THREAD, (unsigned long)thread_func, (unsigned long)thread_arg, (unsigned long)priority);
}

void stop_thread(HTHREAD thread)
{
	syscall(Syscall::SYS_STOP_THREAD, thread);
}

void join_thread(HTHREAD thread)
{
	syscall(Syscall::SYS_JOIN_THREAD, thread);
}

void set_thread_name(HTHREAD thread, const char *name)
{
	syscall(Syscall::SYS_SET_THREAD_NAME, thread, (unsigned long)name);
}

void usleep(unsigned long us)
{
	syscall(Syscall::SYS_USLEEP, us);
}
