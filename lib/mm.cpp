#include <infos.h>
#include <mutex.h>

struct AssertionException{};

void my_assert(bool b) {
    if (!b) {
        printf("!!!!assertion failed!!!!\n");
        throw AssertionException();
    }
}

static const int MAX_ALLOCATIONS = 240;
static int allocations = 0;
static bool freed[MAX_ALLOCATIONS];
static uint8_t* allocations_addresses[MAX_ALLOCATIONS];
static bool check_possible = true;

static uint8_t* free_memory_start = NULL;
static uint64_t free_memory = 0;
static mutex malloc_mutex;
extern "C" {
void* malloc(uint64_t size) {
    unique_lock<mutex> lock(malloc_mutex);
    if (free_memory < size) {
        uint64_t to_allocate = 10 + (size - 1) / 0x1000 + 1;
        auto start = syscall(Syscall::SYS_ALLOCATE_MEMORY, to_allocate);
        if (!free_memory_start) {
            free_memory_start = (uint8_t *)start;
        }
        free_memory += to_allocate * 0x1000;
    }
    auto result = free_memory_start;
    free_memory_start += size;
    free_memory -= size;
    if (allocations + 1 <= MAX_ALLOCATIONS) {
        freed[allocations] = false;
        allocations_addresses[allocations] = result;
        allocations++;
    } else {
        check_possible = false;
    }
    return result;
}


void free(void* va) {
    unique_lock<mutex> lock(malloc_mutex);
    if (check_possible) {
        for (int i = 0; i < allocations; i++) {
            if (allocations_addresses[i] == va) {
                freed[i] = true;
                return ;
            }
        }
        printf("ERROR: mismatched free with va: %llx\n", va);
        check_possible = false;
    }
}
}

void perform_malloc_check() {
    if (!check_possible) {
        printf("check impossible\n");
        return;
    }
    for (int i = 0; i < allocations; i++) {
        if (!freed[i]) {
            printf("ERROR: address %llx not freed\n", allocations_addresses[i]);
        }
    }
    printf("malloc check completed\n");
}


asm(
    ".global rdfsbase\n"
    "rdfsbase:\n"
    "   rdfsbase    %rax\n"
    "   ret\n"
);

asm(
    ".global rdgsbase\n"
    "rdgsbase:\n"
    "   rdgsbase    %rax\n"
    "   ret\n"
);

asm(
    ".global wrfsbase\n"
    "wrfsbase:\n"
    "   wrfsbase    %rdi\n"
    "   ret\n"
);

asm(
    ".global wrgsbase\n"
    "wrgsbase:\n"
    "   wrgsbase    %rdi\n"
    "   ret\n"
);

void *operator new(size_t size)
{
	return malloc(size);
}

void *operator new[](size_t size)
{
	return malloc(size);
}

void operator delete(void *p) {
    free(p);
}

void operator delete(void *p, size_t sz) {
    free(p);
}

void operator delete[](void* ptr) {
    free(ptr);
}