#include <unwind/tconfig.h>
#include <infos-c.h>

void gcc_assert(int a) {}

void *memcpy(void *dest, const void *src, size_t n) {
	for (size_t i = 0; i < n; i++) {
		((char *)dest)[i] = ((char *)src)[i];
	}
	return dest;
}

void *memset(void *s, int c, size_t n) {
	for (size_t i = 0; i < n; i++) {
		((char *)s)[i] = c;
	}
	return s;
}

void gcc_unreachable() {}

void puts(const char* t) {
	printf(t);
}

void abort() {
    exit(0);
}

asm(
	".global atomic_exchange\n"
	"atomic_exchange:\n"
	"	movq    %rsi, %rax\n"
	"	xchgq   (%rdi), %rax\n"
	"	ret\n"
);

asm(
	".global atomic_inc\n"
	"atomic_inc:\n"
	"	lock incq       (%rdi)\n"
	"	ret\n"
);

asm(
	".global atomic_dec\n"
	"atomic_dec:\n"
	"	lock decq       (%rdi)\n"
	"	ret\n"
);
