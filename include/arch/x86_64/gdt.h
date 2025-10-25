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

void gdt_init(gdt_descriptor *gdt_descriptor);

#endif