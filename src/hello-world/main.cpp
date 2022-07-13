/*
 * "Hello, world!"
 * SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 0
 */

#include <infos.h>

class A {};

int main(const char *cmdline)
{
	try {
		printf("throwing\n");
		throw A();
	} catch (A& a) {
		printf("caught\n");
	}
    printf("Hello World\n");
    return 0;
}
