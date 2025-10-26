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

void arch_memory_set(void *ptr, uint8_t value, uint64_t size);
void arch_memory_set_byte(void *ptr, uint8_t value, uint64_t size);
void arch_memory_set_word(void *ptr, uint16_t value, uint64_t size);
void arch_memory_set_dword(void *ptr, uint32_t value, uint64_t size);
void arch_memory_set_qword(void *ptr, uint64_t value, uint64_t size);
void arch_memory_copy(void *dest, const void *src, uint64_t size);
void arch_memory_move(void *dest, const void *src, uint64_t size);
int arch_memory_compare(const void *ptr1, const void *ptr2, uint64_t size);

#define arch_memory_zero(ptr, count) arch_memory_set(ptr, 0, count)
#define arch_memory_zero_struct(ptr) arch_memory_set(ptr, 0, sizeof(*(ptr)))

arch_result arch_memory_map_page(uint64_t virtual_addr, uint64_t physical_addr, int flags);
arch_result arch_memory_unmap_page(uint64_t virtual_addr);
void *arch_memory_allocate_page(void);
void arch_memory_deallocate_page(void *page);
void arch_memory_flush_tlb(void);

arch_result arch_memory_init(void);

void arch_memory_map_userpages(uint64_t pdpt);

#endif

#endif
