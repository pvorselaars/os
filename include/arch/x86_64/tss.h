#ifndef TSS_H
#define TSS_H

#include "definitions.h"

void x86_64_tss_set_entry(int index, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags);
void x86_64_tss_init(void);
void arch_set_interrupt_stack_pointer(uint64_t sp);

#endif