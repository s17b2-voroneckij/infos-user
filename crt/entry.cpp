/* SPDX-License-Identifier: MIT */

#include <infos.h>

extern int infos_main(const char *cmdline, const char *path);

extern "C"
{
	void _start(const char *cmdline, const char *path)
	{
		exit(infos_main(cmdline, path));
	}

	void __stack_chk_fail()
	{
		exit(1);
	}
}
