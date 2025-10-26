#include "arch/arch.h"

void arch_halt(void)
{
    __asm__ volatile("hlt");
}