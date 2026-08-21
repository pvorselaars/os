#ifndef ARCH_CPU_H
#define ARCH_CPU_H

#include "kernel/base.h"

result_t arch_cpu_init(void);
void arch_cpu_idle(void);
void arch_cpu_halt(void);
void arch_cpu_interrupt_enable(void);
void arch_cpu_interrupt_disable(void);

#endif
