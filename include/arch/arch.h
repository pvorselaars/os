#ifndef ARCH_H
#define ARCH_H

#include "definitions.h"

result_t arch_init();
void arch_idle();
void arch_halt();
// void arch_shutdown(void);
void arch_interrupt_enable();
void arch_interrupt_disable();

void arch_memory_init();
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

result_t arch_memory_map_page(uint64_t virtual_addr, uint64_t physical_addr, int flags);
result_t arch_memory_unmap_page(uint64_t virtual_addr);
void *arch_memory_allocate_page(void);
void arch_memory_deallocate_page(void *page);
void arch_memory_flush_tlb(void);


void arch_memory_map_userpages(uint64_t pdpt);



#endif