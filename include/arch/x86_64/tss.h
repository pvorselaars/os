#ifndef TSS_H
#define TSS_H

#include "definitions.h"

void x86_64_tss_init(void);
void arch_set_interrupt_stack_pointer(uint64_t sp);

#endif