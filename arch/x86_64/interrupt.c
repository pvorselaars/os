#include "arch/x86_64/gdt.h"
#include "arch/x86_64/memory.h"

#pragma pack(1)
typedef struct
{
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} tss64;

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
tss64 tss = {.rsp0 = 0x1000};

int arch_register_interrupt(unsigned vector, addr_t handler)
{
    if (vector >= MAX_INTERRUPTS)
        return -1;

    idt[vector].offset_low = handler & 0xffff;
    idt[vector].selector = CODE_SEG;
    idt[vector].ist = 0;
    idt[vector].flags = 0x80 | (KERNEL << 5) | INTERRUPT_GATE;
    idt[vector].offset_mid = (handler >> 16) & 0xffff;
    idt[vector].offset_high = (handler >> 32) & 0xffffffff;
    idt[vector].reserved = 0;

    return 0;
}

void arch_irq_eoi(unsigned vector)
{
    extern void platform_pic_eoi(unsigned vector);
    platform_pic_eoi(vector);
}

void arch_set_interrupt_stack_pointer(uint64_t sp)
{
    tss.rsp0 = sp;
}

static void set_tss_entry(int index, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags)
{
    arch_gdt_set_entry(index, base, limit, access, flags);
    arch_gdt_set_entry(index + 1, 0, 0, 0, 0); // Clear the next entry
}

void arch_interrupt_init()
{
    idtr.size = sizeof(interrupt_descriptor) * MAX_INTERRUPTS - 1;
    idtr.offset = idt;
    __asm__ volatile("lidt %0" : : "m"(idtr));

    uint64_t tss_base = (uint64_t)&tss;
    uint64_t tss_limit = sizeof(tss) - 1;

    set_tss_entry(5, tss_base, tss_limit, SDA_P | SDA_A | SDA_TSS, 0x0);

    /* Load TSS */
    __asm__ volatile("ltr %0" : : "r"(0x28)); // Selector for TSS entry (index 5)
}