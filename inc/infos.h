/* SPDX-License-Identifier: MIT */

#pragma once

typedef unsigned char			uint8_t;
typedef unsigned short			uint16_t;
typedef unsigned int			uint32_t;
typedef unsigned long long int	uint64_t;

typedef signed char				int8_t;
typedef signed short			int16_t;
typedef signed int				int32_t;
typedef signed long long int	int64_t;

typedef unsigned long	size_t;
typedef unsigned long	off_t;
typedef unsigned long	uintptr_t;
typedef signed long		intptr_t;

typedef signed long int		intmax_t;
typedef unsigned long int	uintmax_t;

#define ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof(_arr[0]))

enum class Syscall
{
	SYS_NOP		= 0,
	SYS_YIELD	= 1,
	SYS_EXIT	= 2,

	SYS_OPEN	= 3,
	SYS_CLOSE	= 4,
	SYS_READ	= 5,
	SYS_WRITE	= 6,

	SYS_OPENDIR	= 7,
	SYS_READDIR = 8,
	SYS_CLOSEDIR = 9,

	SYS_EXEC	= 10,
	SYS_WAIT_PROC	= 11,
	SYS_CREATE_THREAD = 12,
	SYS_STOP_THREAD = 13,
	SYS_JOIN_THREAD = 14,
	SYS_USLEEP = 15,

	SYS_GET_TOD = 16,
	SYS_SET_THREAD_NAME = 17
};

typedef unsigned long HANDLE;
typedef HANDLE HFILE;
typedef HANDLE HDIR;
typedef HANDLE HPROC;
typedef HANDLE HTHREAD;

#define HTHREAD_SELF ((HTHREAD)-1)

static inline bool is_error(HANDLE h) { return h == (HANDLE)-1; }

extern unsigned long syscall(Syscall nr);
extern unsigned long syscall(Syscall nr, unsigned long a1);
extern unsigned long syscall(Syscall nr, unsigned long a1, unsigned long a2);
extern unsigned long syscall(Syscall nr, unsigned long a1, unsigned long a2, unsigned long a3);

extern void exit(int exit_code) __attribute__((noreturn));

extern HFILE open(const char *filename, int flags);
extern int read(HFILE file, char *buffer, size_t size);
extern int write(HFILE file, const char *buffer, size_t size);
extern void close(HFILE file);

struct dirent
{
	char name[64];
	unsigned int size;
	int flags;
};

extern HDIR opendir(const char *path, int flags);
extern int readdir(HDIR dir, struct dirent *de);
extern void closedir(HDIR dir);

extern HPROC exec(const char *filename, const char *args);
extern void wait_proc(HPROC proc);

typedef void (*ThreadProc)(void *);
extern HTHREAD create_thread(ThreadProc tp, void *arg);
extern void stop_thread(HTHREAD thread);
extern void join_thread(HTHREAD thread);
extern void set_thread_name(HTHREAD thread, const char *name);
extern void usleep(unsigned long us);

struct tod {
	unsigned short seconds, minutes, hours, day_of_month, month, year;
};

extern int get_time_of_day(struct tod *t);

#define va_start(v,l)   __builtin_va_start(v,l)
#define va_end(v)       __builtin_va_end(v)
#define va_arg(v,l)     __builtin_va_arg(v,l)

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

extern int snprintf(char *buffer, int size, const char *fmt, ...);
extern int printf(const char *fmt, ...);
extern int sprintf(char *buffer, const char *fmt, ...);
extern int vsnprintf(char *buffer, int size, const char *fmt, va_list args);

extern int strcmp(const char *l, const char *r);
extern int strlen(const char *str);

extern char getch();

extern HFILE __console_handle;

#define NULL 0
