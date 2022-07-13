#include <infos.h>

extern "C" {
extern void* init_unwinding(const void*, const void*);
extern void __deregister_frame (void *begin);
}

#define __packed __attribute__((packed))

struct ELF64Header
{
    struct
    {
        union
        {
            char magic_bytes[4];
            uint32_t magic_number;
        };

        uint8_t eclass, data, version, osabi, abiversion;
        uint8_t pad[7];
    } ident __packed;

    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry_point;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} __packed;

typedef struct {
    uint32_t	sh_name;
    uint32_t	sh_type;
    uint64_t	sh_flags;
    uint64_t	sh_addr;
    uint64_t	sh_offset;
    uint64_t	sh_size;
    uint32_t	sh_link;
    uint32_t	sh_info;
    uint64_t	sh_addralign;
    uint64_t	sh_entsize;
} Elf64_Shdr;

static void* EH_FRAME_START;
static void* NEXT_SECTION_START;
static Elf64_Shdr tbss;
static Elf64_Shdr tdata;
static const char* ELF_NAME;
static uint64_t thread_local_size;

void *memset(void *s, int c, size_t n) {
    auto ptr = (char*)s;
    for (size_t i = 0; i < n; i++) {
        ptr[i] = (char)c;
    }
    return s;
}

uint64_t alignup(uint64_t a, uint64_t alignment) {
    return ((a - 1) / alignment + 1) * alignment;
}

void prepare_thread_local() {
    thread_local_size = tbss.sh_size + tdata.sh_size;
    if (thread_local_size == 0) {
        return ;
    }
    uint64_t alignment = tdata.sh_addralign;
    if (tbss.sh_addralign > alignment) {
        alignment = tbss.sh_addralign;
    }
    thread_local_size = alignup(thread_local_size, alignment);
    auto addr = (char *)malloc(thread_local_size + sizeof(long) + alignment);
    //addr = (char *)alignup((uint64_t)addr, alignment);
    wrfsbase((uint64_t)(addr + thread_local_size));
    *(long *)(addr + thread_local_size) = (addr + thread_local_size);

    // now we need to initialize memory
    int fd = open(ELF_NAME, 0);
    pread(fd, addr, tdata.sh_size, tdata.sh_offset);
    close(fd);
    memset(addr + tdata.sh_size, 0, tbss.sh_size);
}

void clean_up_thread_local() {
    if (thread_local_size == 0) {
        return;
    }
    free((void *)(rdfsbase() - thread_local_size));
}

void process_elf(const char* FILE_NAME) {
    ELF_NAME = FILE_NAME;
	auto fd = open(FILE_NAME, 0);
    ELF64Header hdr;
    pread(fd, (char*)&hdr, sizeof(ELF64Header), 0);
    Elf64_Shdr string_table_section;
    pread(fd, (char*)&string_table_section, sizeof(Elf64_Shdr),hdr.shoff + hdr.shstrndx * sizeof(Elf64_Shdr));
    char strings[string_table_section.sh_size];
    pread(fd, strings, string_table_section.sh_size, string_table_section.sh_offset);
    bool previos_eh_frame = false;
    for (int i = 1; i < hdr.shnum; i++) {
        Elf64_Shdr section_header;
        pread(fd, (char*)&section_header, sizeof(Elf64_Shdr), hdr.shoff + i * sizeof(Elf64_Shdr));
        char* name = &strings[section_header.sh_name];
        if (previos_eh_frame) {
            NEXT_SECTION_START = (void *)section_header.sh_addr;
            previos_eh_frame = false;
        }
        if (strcmp(name, ".eh_frame") == 0) {
            previos_eh_frame = true;
            EH_FRAME_START = (void *)section_header.sh_addr;
        }
        else if (strcmp(name, ".tdata") == 0) {
            tdata = section_header;
        }
        else if (strcmp(name, ".tbss") == 0) {
            tbss = section_header;
        }
    }
	init_unwinding(EH_FRAME_START, NEXT_SECTION_START);
    close(fd);
}

void exceptions_clean_up() {
    __deregister_frame(EH_FRAME_START);
}

extern void make_pagefault();
