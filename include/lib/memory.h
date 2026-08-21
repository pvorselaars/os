#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

#include "definitions.h"

#define ALIGN_UP(x, size)   (((x) + (size) - 1) & ~((size) - 1))
#define ALIGN_DOWN(x, size) ((x) & ~((size) - 1))
#define IS_ALIGNED(x, size) (((x) & ((size) - 1)) == 0)

void memory_set(void *ptr, uint8_t value, uint64_t size);
void memory_copy(void *dest, const void *src, uint64_t size);
void memory_move(void *dest, const void *src, uint64_t size);
int memory_compare(const void *ptr1, const void *ptr2, uint64_t size);

#define memory_zero(ptr, count) memory_set(ptr, 0, count)
#define memory_zero_struct(ptr) memory_set(ptr, 0, sizeof(*(ptr)))

#endif