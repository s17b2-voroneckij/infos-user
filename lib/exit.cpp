/* SPDX-License-Identifier: MIT */

#include <infos.h>

extern "C" {
void exit(int exit_code)
{
	syscall(Syscall::SYS_EXIT, exit_code);
	__builtin_unreachable();
}
}