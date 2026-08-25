#include "arch/x86_64/tss.h"
#include "arch/x86_64/gdt.h"

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
#pragma pack()

uint64_t kernel_stack[64];

tss64 tss = {.rsp0 = 0x1000};

void x86_64_tss_init(void)
{
    tss.rsp0 = (uint64_t)&kernel_stack[64];
    tss.iomap_base = sizeof(tss64);

    uint64_t tss_base = (uint64_t)&tss;
    uint64_t tss_limit = sizeof(tss64) - 1;
    x86_64_gdt_set_tss_entry(tss_base, tss_limit);

    __asm__ volatile("ltr %0" : : "r"(0x38));
}

void arch_set_interrupt_stack_pointer(uint64_t sp)
{
    tss.rsp0 = sp;
}