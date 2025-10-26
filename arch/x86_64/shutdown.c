#include "arch/arch.h"

void arch_halt(void)
{
    while (1)
        __asm__ volatile("hlt");
}