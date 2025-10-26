#include "arch/x86_64/tss.h"
#include "arch/x86_64/gdt.h"
#include "arch/x86_64/memory.h"

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

tss64 tss = {.rsp0 = 0x1000};

void x86_64_tss_set_entry(int index, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags)
{
    x86_64_gdt_set_entry(index, base, limit, access, flags);
    x86_64_gdt_set_entry(index + 1, 0, 0, 0, 0); // Clear the next entry
}

void x86_64_tss_init(void)
{
    /* TSS initialization can be done here if needed */
    uint64_t tss_base = (uint64_t)&tss;
    uint64_t tss_limit = sizeof(tss) - 1;

    x86_64_tss_set_entry(5, tss_base, tss_limit, SDA_P | SDA_A | SDA_TSS, 0x0);

    /* Load TSS */
    __asm__ volatile("ltr %0" : : "r"(0x28)); // Selector for TSS entry (index 5)
}

void arch_set_interrupt_stack_pointer(uint64_t sp)
{
    tss.rsp0 = sp;
}