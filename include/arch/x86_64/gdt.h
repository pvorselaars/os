#ifndef GDT_H
#define GDT_H

#include "definitions.h"

extern uint64_t *gdt;

#pragma pack(1)
typedef struct
{
    uint16_t limit;
    uint64_t base;
} gdt_descriptor;
#pragma pack()

void arch_gdt_set(gdt_descriptor *gdt_descriptor);

void arch_gdt_set_entry(int index, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags);

void arch_gdt_init(void);

#endif