#ifndef ARCH_MEMORY_H
#define ARCH_MEMORY_H

#include "definitions.h"

/* Architecture Memory Interface
 * 
 * Each architecture must provide implementations of these functions
 * using optimized assembly or C code as appropriate for the platform.
 */

/* Memory set functions - set memory to specific values
 * These should be optimized using architecture-specific instructions
 * (e.g., x86_64 REP STOS, ARM NEON, etc.) */
void memory_set_byte(void *ptr, const uint8_t value, const uint64_t count);
void memory_set_word(void *ptr, const uint16_t value, const uint64_t count);
void memory_set_dword(void *ptr, const uint32_t value, const uint64_t count);
void memory_set_qword(void *ptr, const uint64_t value, const uint64_t count);

/* Memory comparison function
 * Should be optimized for the target architecture */
int memory_compare(const void *ptr1, const void *ptr2, const uint64_t count);

/* Memory move/copy functions
 * Should handle overlapping (move) and non-overlapping (copy) regions
 * Should be optimized using architecture-specific block copy instructions */
void memory_move(void *dst, const void *src, const uint64_t count);
void memory_copy(void *dst, const void *src, const uint64_t count);

/* Architecture-specific memory management functions */
void *memory_allocate(void);
void memory_deallocate(void *ptr);

/* Architecture-specific virtual memory functions */
uint64_t memory_map(uint64_t va, uint64_t pa, int32_t flags);
int memory_unmap(uint64_t va);
void flush_tlb(void);

#endif