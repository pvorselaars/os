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

void *arch_memory_allocate_page(void);
void arch_memory_deallocate_page(void *page);

#endif
