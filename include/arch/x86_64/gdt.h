#ifndef GDT_H
#define GDT_H

#include "definitions.h"

void arch_gdt_set_entry(int index, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags);

void arch_gdt_init(void);

#endif