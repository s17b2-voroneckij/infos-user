#include <infos.h>

extern void make_pagefault();

int main() {
    try {
        make_pagefault();
    } catch (const PageFaultException& exc) {
        printf("page fault exception thrown addr=%p!\n", exc.addr);
    }
    make_pagefault();
    printf("exiting\n");
    return 0;
}