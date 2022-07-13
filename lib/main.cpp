/* SPDX-License-Identifier: MIT */

#include <infos.h>
#include <abi.h>

extern int main(const char *cmdline);

HFILE __console_handle;

void make_pagefault() {
    volatile int *a = (volatile int *)0xffffffffffffffff;
    *a = 0;
}

void unwind_entry(struct _Unwind_Exception *exc) {
    _Unwind_RaiseException(exc);
    __cxa_exception* exception_header = (__cxa_exception*)(exc + 1) - 1;
    printf("THIS IS NORMALLY UNREACHABLE\n");
    printf("exception of type %s not caught\n", exception_header->exceptionTypeName);
    syscall(Syscall::SYS_THREAD_LEAVE);
}

struct ExceptionPrepareInfo {
    void* unwind_address;
    void *(*malloc_ptr) (uint64_t);
    void (*free_ptr)(void *);
};

extern void perform_malloc_check();
extern void prepare_thread_local();
extern void clean_up_thread_local();
void run_thread_atexit();

void infos_main(const char *cmdline, const char *path)
{
    __console_handle = open("/dev/console", 0);
    if (is_error(__console_handle)) {
        exit(1);
    }
    bool need_prepare = true;
    if (strcmp(path, "/usr/init") == 0 || strcmp(path, "/usr/shell") == 0) {
        need_prepare = false;
    }
    if (need_prepare) {
        auto malloc_ptr = &malloc;
        auto free_ptr = &free;
        process_elf(path);
        ExceptionPrepareInfo exceptionPrepareInfo {(void*)&unwind_entry, malloc_ptr, free_ptr};
        syscall(Syscall::SYS_SET_EXC_INFO, (uintptr_t)&exceptionPrepareInfo);
        prepare_thread_local();
    }
    
    int rc;
    try {
        rc = main(cmdline);
    } catch (...) {
        rc = 1;
        printf("leaving due to exception of type %s\n", __cxa_last_exception_name());
    }
    if (need_prepare) {
        exceptions_clean_up();
        run_thread_atexit();
        clean_up_thread_local();
        perform_malloc_check();
    }
    close(__console_handle);
    exit(rc);
}

struct atexit_info {
    void (*func)(void*);
    void* obj;
    atexit_info* next;
};

static thread_local atexit_info* first_atexit_info;
extern "C" {
void* __dso_handle;

void __cxa_thread_atexit(void (*func)(void*), void *obj, void *dso_symbol) {
    auto new_atexit_info = new atexit_info();
    *new_atexit_info = {.func = func, .obj = obj, .next = first_atexit_info};
    first_atexit_info = new_atexit_info;
}
}

void run_thread_atexit() {
    while (first_atexit_info) {
        first_atexit_info->func(first_atexit_info->obj);
        auto next = first_atexit_info->next;
        delete first_atexit_info;
        first_atexit_info = next;
    }
}
