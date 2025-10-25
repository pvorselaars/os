#ifndef MEMORY_H
#define MEMORY_H

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITE (1 << 1)
#define PAGE_USER (1 << 2)
#define PAGE_PS (1 << 7)

#define SDA_P (1 << 7) // Present segment
#define SDA_U (3 << 5) // User segment
#define SDA_S (1 << 4) // Descriptor type; system or code/data segment
#define SDA_E (1 << 3) // Executable segment
#define SDA_D (1 << 2) // Expand down (non-executable segments)
#define SDA_C (1 << 2) // Conforming code segment (executable only)
#define SDA_W (1 << 1) // Writeable (non-executable segments)
#define SDA_R (1 << 1) // Readable (executable segments)
#define SDA_A (1 << 0) // Accessed

#define SDF_G (1 << 3)  // Granularity flag
#define SDF_DB (1 << 2) // Size flag
#define SDF_L (1 << 1)  // Long mode flag

#define CR0_PE (1 << 0)  // Protected Mode Enable bit
#define CR0_PG (1 << 31) // Paging bit

#define CR4_PGE (1 << 7) // Page Global Enable bit
#define CR4_PAE (1 << 5) // Physical Address Extension bit

#define MSR_EFER 0xC0000080 // EFER model specific register
#define EFER_LME (1 << 8)   // Long mode bit

#define CODE_SEG 0x18 // Kernel 64-bit code segment selector (index 3)
#define DATA_SEG 0x20 // Kernel 64-bit data segment selector (index 4)

#define PML4_ADDRESS 0x1000
#define BOOT_SEGMENT 0xF000

#define PAGE_SIZE 0x1000

#define KERNEL_BASE 0xFFFFFF8000000000
#define KERNEL_STACK KERNEL_BASE + 0x200000 - 1

#define physical_address(va) ((uint64_t)(va) - KERNEL_BASE)
#define virtual_address(pa) ((void *)((uint64_t)(pa) + KERNEL_BASE))

#ifdef __ASSEMBLER__

#define SEGMENT(access, flags, base, limit)     \
        .word(limit & 0xffff), (base & 0xffff); \
        .byte((base >> 16) & 0xff), access, (((flags) << 4) | ((limit >> 16) & 0xf)), ((base >> 24) & 0xff)

#else

#include "definitions.h"
#include "console.h"
#include "utils.h"

extern char KERNEL_VMA[];
extern char KERNEL_END[];

typedef uint64_t pte;
typedef uint64_t pde;
typedef uint64_t pdpte;
typedef uint64_t pml4e;

typedef uint64_t address;

void memsetb(void *ptr, const int8_t value, const uint64_t num);
void memsetw(void *ptr, const int16_t value, const uint64_t num);
void memsetl(void *ptr, const int32_t value, const uint64_t num);
void memsetq(void *ptr, const int64_t value, const uint64_t num);

int memcmp(const void *ptr1, const void *ptr2, const uint64_t num);

void memory_move(void *dst, const void *src, const uint64_t num);
void memory_copy(void *dst, const void *src, const uint64_t num);

uint64_t memory_map(uint64_t va, uint64_t pa, int32_t flags);
void memory_map_userpages(uint64_t pdpt);
int memory_unmap(address va);
void *memory_allocate(void);
void memory_deallocate(void *page);

void memory_init();
void print_regions();
void print_pagetable_entries(address a);
void examine(void *ptr, uint64_t bytes);

extern void flush_tlb();

#endif

#endif
