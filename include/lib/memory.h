#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

#include "kernel/definitions.h"

/* Platform-agnostic memory utility interface
 * 
 * These functions provide a consistent API for memory operations
 * across different architectures. The actual implementations use
 * architecture-specific optimized assembly instructions.
 */

/* Memory set functions - set memory to a specific value */
void memory_set_byte(void *ptr, const uint8_t value, const uint64_t count);
void memory_set_word(void *ptr, const uint16_t value, const uint64_t count);
void memory_set_dword(void *ptr, const uint32_t value, const uint64_t count);
void memory_set_qword(void *ptr, const uint64_t value, const uint64_t count);

/* Generic memory set function - automatically chooses optimal size */
void memory_set(void *ptr, const uint8_t value, const uint64_t count);

/* Memory comparison function */
int memory_compare(const void *ptr1, const void *ptr2, const uint64_t count);

/* Memory move/copy functions - handle overlapping and non-overlapping regions */
void memory_move(void *dst, const void *src, const uint64_t count);
void memory_copy(void *dst, const void *src, const uint64_t count);

/* Utility macros */
#define ALIGN_UP(x, size)   (((x) + (size) - 1) & ~((size) - 1))
#define ALIGN_DOWN(x, size) ((x) & ~((size) - 1))
#define IS_ALIGNED(x, size) (((x) & ((size) - 1)) == 0)

/* Memory zero functions - convenience wrappers */
#define memory_zero(ptr, count) memory_set(ptr, 0, count)
#define memory_zero_struct(ptr) memory_set(ptr, 0, sizeof(*(ptr)))

#endif