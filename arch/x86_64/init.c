#include "arch/cpu.h"
#include "arch/x86_64/gdt.h"
#include "arch/x86_64/interrupt.h"
#include "arch/x86_64/memory.h"
#include "arch/x86_64/vmm.h"

result_t arch_cpu_init(void)
{
    arch_cpu_interrupt_disable();

    x86_64_gdt_init();
    x86_64_interrupt_init();
    x86_64_memory_init();
    x86_64_vmm_init();

    return RESULT_OK;
}