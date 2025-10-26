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

void arch_debug_printf(const char *format, ...);

#endif