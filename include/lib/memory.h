#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

#define ALIGN_UP(x, size)   (((x) + (size) - 1) & ~((size) - 1))
#define ALIGN_DOWN(x, size) ((x) & ~((size) - 1))
#define IS_ALIGNED(x, size) (((x) & ((size) - 1)) == 0)

#define KERNEL_BASE 0xFFFFFF8000000000
#define KERNEL_STACK KERNEL_BASE + 0x200000 - 1

#define physical_address(va) ((uint64_t)(va) - KERNEL_BASE)
#define virtual_address(pa) ((void *)((uint64_t)(pa) + KERNEL_BASE))

#ifndef __ASSEMBLER__

#include "definitions.h"

void memory_set(void *ptr, uint8_t value, uint64_t size);
void memory_copy(void *dest, const void *src, uint64_t size);
void memory_move(void *dest, const void *src, uint64_t size);
int memory_compare(const void *ptr1, const void *ptr2, uint64_t size);

#define memory_zero(ptr, count) memory_set(ptr, 0, count)
#define memory_zero_struct(ptr) memory_set(ptr, 0, sizeof(*(ptr)))

#endif
#endif