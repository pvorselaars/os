#ifndef MEMORY_H
#define MEMORY_H

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITE   (1 << 1)
#define PAGE_PS      (1 << 7)

#define SDA_P        (1 << 7)     // Present segment
#define SDA_U        (3 << 5)     // User segment
#define SDA_S        (1 << 4)     // Descriptor type; system or code/data segment
#define SDA_E        (1 << 3)     // Executable segment
#define SDA_D        (1 << 2)     // Expand down (non-executable segments)
#define SDA_C        (1 << 2)     // Conforming code segment (executable only)
#define SDA_W        (1 << 1)     // Writeable (non-executable segments)
#define SDA_R        (1 << 1)     // Readable (executable segments)
#define SDA_A        (1 << 0)     // Accessed

#define SDF_G        (1 << 3)     // Granularity flag
#define SDF_DB       (1 << 2)     // Size flag
#define SDF_L        (1 << 1)     // Long mode flag

#define CR0_PE       (1 << 0)     // Protected Mode Enable bit
#define CR0_PG       (1 << 31)    // Paging bit

#define CR4_PGE      (1 << 7)     // Page Global Enable bit
#define CR4_PAE      (1 << 5)     // Physical Address Extension bit

#define MSR_EFER     0xC0000080   // EFER model specific register
#define EFER_LME     (1 << 8)     // Long mode bit

#define CODE_SEG     0x18         // Kernel code segment index
#define DATA_SEG     0x20         // Kernel data segment index

#define PML4_ADDRESS 0x1000
#define BOOT_SEGMENT 0xF000

#define PAGE_SIZE    0x1000

#ifdef __ASSEMBLER__

#define SEGMENT(access, flags, base, limit) \
        .word (limit & 0xffff), (base & 0xffff); \
        .byte ((base >> 16) & 0xff), access, (((flags) << 4) | ((limit >> 16) & 0xf)), ((base >> 24) & 0xff)

#else

#include "definitions.h"

typedef uint64_t pte;
typedef uint64_t pde;
typedef uint64_t pdpte;
typedef uint64_t pml4e;

typedef uint64_t page;
typedef uint64_t address;

void memsetb(const void *ptr, const int8_t value, const uint32_t num);
void memsetw(const void *ptr, const int16_t value, const uint32_t num);
void memsetl(const void *ptr, const int32_t value, const uint32_t num);
void memsetq(const void *ptr, const int64_t value, const uint32_t num);

int memcmp(const void *ptr1, const void *ptr2, const uint32_t num);

void memmove(const void *dst, const void *src, const uint32_t num);
void memcpy(const void *dst, const void *src, const uint32_t num);

int map(address va, address pa, int32_t flags);
int unmap(address va);
page* alloc(void);
void dealloc(page *p);

void memory_init();
void print_regions();
void print_pagetable_entries(address a);
void examine(void* ptr, uint64_t bytes);

#endif


#endif
