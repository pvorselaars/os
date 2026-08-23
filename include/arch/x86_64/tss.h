#ifndef TSS_H
#define TSS_H

#include "kernel/base.h"

void x86_64_tss_init();
void arch_set_interrupt_stack_pointer(uint64_t sp);

#endif