#include "arch/cpu.h"

void arch_cpu_idle(void)
{
    __asm__ volatile("hlt");
}

void arch_cpu_halt(void)
{
    while (1)
        arch_cpu_idle();
}