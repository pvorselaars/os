#ifndef ARCH_H
#define ARCH_H

#include "definitions.h"

typedef enum {
    ARCH_OK = 0,
    ARCH_ERROR = -1,
    ARCH_INVALID = -2,
    ARCH_UNSUPPORTED = -3
} arch_result;

arch_result arch_init(void);
void arch_halt(void);
// void arch_shutdown(void);

arch_result arch_interrupt_init(void);
int arch_register_interrupt(unsigned vector, void (*handler)(void));
void arch_handle_interrupt(unsigned vector);
void arch_interrupt_enable(void);
void arch_interrupt_disable(void);

arch_result arch_timer_init(unsigned int frequency_hz);
uint64_t arch_time_ns(void);
arch_result arch_register_default_handlers(void);

// Memory management API

// Serial interface - arch-specific implementations
arch_result arch_serial_init(void);
int arch_serial_write(const void *buf, size_t len);
int arch_serial_read(void *buf, size_t len);
bool arch_serial_data_available(void);

arch_result arch_memory_init(void);
void *arch_memory_allocate_page(void);
void arch_memory_deallocate_page(void *page);

arch_result arch_memory_map_page(uint64_t virtual_addr, uint64_t physical_addr, int flags);
arch_result arch_memory_unmap_page(uint64_t virtual_addr);
void arch_memory_flush_tlb(void);

void arch_memory_set(void *ptr, uint8_t value, uint64_t size);
void arch_memory_copy(void *dest, const void *src, uint64_t size);
void arch_memory_move(void *dest, const void *src, uint64_t size);
int arch_memory_compare(const void *ptr1, const void *ptr2, uint64_t size);

void arch_debug_printf(const char *format, ...);

#endif