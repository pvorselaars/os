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

// Serial interface - arch-specific implementations  
typedef struct arch_serial_device arch_serial_device_t;  // Opaque handle

typedef struct {
    arch_serial_device_t *device;  // Opaque arch-specific device handle
    const char *name;               // Suggested device name (e.g., "serial0")
} arch_serial_info_t;

int arch_serial_get_count(void);                                         // How many devices exist?
arch_result arch_serial_get_info(int index, arch_serial_info_t *info);   // Get info for device N
arch_result arch_serial_init(arch_serial_device_t *device);             // Initialize specific device
int arch_serial_write(arch_serial_device_t *device, const void *buf, size_t len);  // Write to device
int arch_serial_read(arch_serial_device_t *device, void *buf, size_t len);         // Read from device
bool arch_serial_data_available(arch_serial_device_t *device);                     // Check if data available

// Parallel interface - arch-specific implementations
typedef struct arch_parallel_device arch_parallel_device_t;  // Opaque handle

typedef struct {
    arch_parallel_device_t *device;  // Opaque arch-specific device handle
    const char *name;                 // Suggested device name (e.g., "parallel0")
} arch_parallel_info_t;

int arch_parallel_get_count(void);                                           // How many devices exist?
arch_result arch_parallel_get_info(int index, arch_parallel_info_t *info);  // Get info for device N
arch_result arch_parallel_init(arch_parallel_device_t *device);             // Initialize specific device
int arch_parallel_write(arch_parallel_device_t *device, const void *buf, size_t len);  // Write to device

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