#ifndef MEMORY_H
#define MEMORY_H

#define PAGE_PRESENT (1 << 0)
#define PAGE_WRITE (1 << 1)
#define PAGE_USER (1 << 2)
#define PAGE_PS (1 << 7)

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

#define MEMORY_MAP 0x8000
#define MEMORY_MAP_ENTRIES 0x8300

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
#include "lib/utils.h"
#include "arch/arch.h"

extern char KERNEL_VMA[];
extern char KERNEL_END[];

typedef uint64_t pte;
typedef uint64_t pde;
typedef uint64_t pdpte;
typedef uint64_t pml4e;

typedef uint64_t address;

#endif

#endif
