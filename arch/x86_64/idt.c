#include "arch/x86_64/gdt.h"
#include "arch/x86_64/memory.h"
#include "arch/x86_64/idt.h"

#pragma pack(1)

typedef struct
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} interrupt_descriptor;

typedef struct
{
    unsigned short size;
    interrupt_descriptor *offset;
} idt_descriptor;
#pragma pack()

#define MAX_INTERRUPTS 256

typedef enum
{
    INTERRUPT_GATE = 0xe,
    TRAP_GATE = 0xf
} gate_type;

typedef enum
{
    KERNEL = 0,
    USER = 3
} privilege_level;

interrupt_descriptor idt[MAX_INTERRUPTS] = {0};
idt_descriptor idtr;

int x86_64_idt_set_entry(unsigned vector, void (*handler)(void), uint8_t flags)
{
    if (vector >= MAX_INTERRUPTS)
        return -1;

    uint64_t addr = (uint64_t)handler;
    idt[vector].offset_low = addr & 0xffff;
    idt[vector].selector = CODE_SEG;
    idt[vector].ist = 0;
    idt[vector].flags = flags | 0x80 | (KERNEL << 5) | INTERRUPT_GATE;
    idt[vector].offset_mid = (addr >> 16) & 0xffff;
    idt[vector].offset_high = (addr >> 32) & 0xffffffff;
    idt[vector].reserved = 0;

    return 0;
}

void x86_64_idt_init(void)
{
    idtr.size = sizeof(interrupt_descriptor) * MAX_INTERRUPTS - 1;
    idtr.offset = idt;
    __asm__ volatile("lidt %0" : : "m"(idtr));

}