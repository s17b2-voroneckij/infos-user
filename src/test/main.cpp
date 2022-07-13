#include <infos.h>

class Int {
        public:
        explicit Int(unsigned int a) {
            n = a;
        }

        unsigned int n;
};

const int ITER = 100000;
const int ITER2 = 4000;

bool run_iters = true;

void func(void*) {
    printf("thread about to make syscall\n");
    syscall(Syscall::SYS_RUN_EXC_TESTS, 1);
}

int main() {
    printf("starting tests\n");
	printf("making syscall\n");
	try {
		syscall(Syscall::SYS_RUN_EXC_TESTS, 1);
	} catch (Int& a) {
		printf("caught Int& a with value %u from the kernel\n", a.n);
	}
	printf("after try-catch\n");

    auto thread = create_thread(func, NULL);
    usleep(1000);
    printf("still alive\n");
    usleep(1000);
    printf("still alive\n");
    if (!run_iters) {
        return 0;
    }
    auto start_time = syscall(Syscall::SYS_GET_TICKS);
    for (int i = 0; i < ITER; i++) {
        try {
            syscall(Syscall::SYS_RUN_EXC_TESTS, 0);
        } catch (Int& a) {
        }
    }
    printf("elapsed  with  exeptions: %u\n", syscall(Syscall::SYS_GET_TICKS) - start_time);
    start_time = syscall(Syscall::SYS_GET_TICKS);
    printf("about to start SYS_NOPs\n");
    for (int i = 0; i < ITER; i++) {
        syscall(Syscall::SYS_NOP);
    }
    printf("elapsed without exeptions: %u\n", syscall(Syscall::SYS_GET_TICKS) - start_time);
    printf("now testing file opening\n");
    start_time = syscall(Syscall::SYS_GET_TICKS);
    char wrong_name[] = "/usr/testuuu";
    for (int i = 0; i < ITER2; i++) {
        try {
            int fd = syscall(Syscall::SYS_OPEN, (unsigned long)wrong_name, 0);
        } catch (...) {
        }
    }
    printf("elapsed while opening wrong file: %u\n", syscall(Syscall::SYS_GET_TICKS) - start_time);
    char right_name[] = "/usr/test";
    start_time = syscall(Syscall::SYS_GET_TICKS);
    for (int i = 0; i < ITER2; i++) {
        int fd = syscall(Syscall::SYS_OPEN, (unsigned long)right_name, 0);
        syscall(Syscall::SYS_CLOSE, fd);
    }
    printf("elapsed while opening right file: %u\n", syscall(Syscall::SYS_GET_TICKS) - start_time);
	return 0;
}

