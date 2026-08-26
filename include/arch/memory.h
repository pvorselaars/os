#ifndef ARCH_MEMORY_H
#define ARCH_MEMORY_H

#include "kernel/base.h"

void arch_memory_set_byte(void *ptr, uint8_t value, size_t size);
void arch_memory_set_word(void *ptr, uint16_t value, size_t size);
void arch_memory_set_dword(void *ptr, uint32_t value, size_t size);
void arch_memory_set_qword(void *ptr, uint64_t value, size_t size);
void arch_memory_copy(void *dest, const void *src, size_t size);
void arch_memory_move(void *dest, const void *src, size_t size);
int arch_memory_compare(const void *ptr1, const void *ptr2, size_t size);

void *arch_memory_allocate_page();
void arch_memory_deallocate_page(void *page);

enum {
    ARCH_MEMORY_MAP_READ = 1 << 0,
    ARCH_MEMORY_MAP_WRITE = 1 << 1,
    ARCH_MEMORY_MAP_USER = 1 << 2,
};

result_t arch_memory_map_page(uint64_t page_table, uint64_t virtual_address, uint64_t physical_address, uint32_t flags);
result_t arch_memory_unmap_page(uint64_t page_table, uint64_t virtual_address);

extern char KERNEL_BASE[];
extern char KERNEL_VMA[];
extern char KERNEL_END[];

#define VIRTUAL_ADDRESS(x) ((uint64_t)(x) + (uint64_t)(KERNEL_BASE))
#define PHYSICAL_ADDRESS(x) ((uint64_t)(x) - (uint64_t)(KERNEL_BASE))

#endif
