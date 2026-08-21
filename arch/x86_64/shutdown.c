#include "arch/arch.h"

void arch_idle(void)
{
    __asm__ volatile("hlt");
}

void arch_halt(void)
{
    while (1)
        arch_idle();
}