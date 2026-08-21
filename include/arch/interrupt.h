#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H

#include "kernel/base.h"

typedef void (*arch_interrupt_handler_t)(void);

result_t arch_interrupt_register(uint32_t vector, arch_interrupt_handler_t handler);

#endif
