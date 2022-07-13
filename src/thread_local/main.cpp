#include <infos.h>

extern "C" {
    unsigned int cpuid();
}

class Destructable {
public:
    Destructable() {
        n = 0;
        initialised = true;
        printf("Destructable()\n");
    }

    ~Destructable() {
        printf("~Destructable()\n");
    }

    int n;
    bool initialised = 0;
};

thread_local Destructable destructable;

thread_local long* jrnvcrnrivbhr;
thread_local long a = 0;
thread_local long b = 1;
thread_local long c = 2;
thread_local long d = 3;
thread_local long e = 4;
thread_local long f = 5;
thread_local long g = 6;
thread_local int h = 7;
thread_local int j = 8;
thread_local char trrr[] = "12";

thread_local long gh;

thread_local long* jrnvcjoiornrivbhr;

const int LIST_SIZE = 4096;
thread_local long list[LIST_SIZE];

void func(void *) {
    my_assert(a == 0);
    my_assert(b == 1);
    my_assert(c == 2);
    my_assert(d == 3);
    my_assert(strcmp("12", trrr) == 0);
    printf("entering new thread\n");
    a++;
    b++;
    my_assert(a == 1);
    my_assert(b == 2);
    for (int i = 0; i < LIST_SIZE; i++) {
        my_assert(list[i] == 0);
    }
    printf("thread leaving\n");
    return;
}

class A{};

void func2(void *) {
    printf("thread throwing\n");
    throw A();
}

void func3(void *) {
    printf("func3 working\n");
    while (1) {
        for (int i = 0; i < 10000; i++) {
            usleep(10);
        }
    }
}

void func4(void *) {
    printf("thread leaving in a legacy way\n");
    stop_thread(-1);
}

int main() {
    auto init_fs = rdfsbase();
    auto init_gs = rdgsbase();
    const unsigned long FS_VALUE = 0xffffff, GS_VALUE = 0xaaaaa;
    wrfsbase(FS_VALUE);
    wrgsbase(GS_VALUE);
    my_assert(FS_VALUE == rdfsbase());
    my_assert(GS_VALUE == rdgsbase());
    syscall(Syscall::SYS_NOP);
    my_assert(FS_VALUE == rdfsbase());
    my_assert(GS_VALUE == rdgsbase());
    wrfsbase(init_fs);
    wrgsbase(init_gs);
    printf("rd/wr fs/gs base test completed\n");
    printf("a: %d, b: %d, c: %d\n", a, b, c);
    printf("d: %d, e: %d, f: %d\n", d, e, f);
    my_assert(a == 0);
    my_assert(b == 1);
    my_assert(c == 2);
    my_assert(d == 3);
    printf("so far, so good");
    a++;
    b++;
    a++;
    b++;
    printf("a: %ld, b: %ld\n", a, b);
    my_assert(a == 2);
    my_assert(b == 3);
    for (long i = 0; i < LIST_SIZE; i++) {
        my_assert(list[i] == 0);
        list[i] = i * i;
    }
    auto thread = create_thread(func, NULL);
    join_thread(thread);
    my_assert(a == 2);
    my_assert(b == 3);
    printf("thread_local test success\n");
    thread = create_thread(func2, NULL);
    join_thread(thread);
    thread = create_thread(func3, NULL);
    usleep(600);
    printf("going to stop thread\n");
    stop_thread(thread);
    usleep(10);
    syscall(Syscall::SYS_NOP);

    thread = create_thread(func4, NULL);
    join_thread(thread);
    printf("checking that destructors are called\n");
    my_assert(destructable.initialised);
    printf("%d\n", destructable.n);

    printf("finally leaving\n");
    return 0;
}