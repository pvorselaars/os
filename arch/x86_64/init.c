#include "arch/arch.h"
#include "arch/x86_64/gdt.h"
#include "arch/x86_64/interrupt.h"

result_t arch_init(void)
{
    arch_interrupt_disable();

    x86_64_gdt_init();
    x86_64_interrupt_init();
    arch_memory_init();

    return RESULT_OK;
}