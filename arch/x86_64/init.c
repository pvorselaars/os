#include "arch/arch.h"
#include "arch/interrupt.h"
#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/serial.h"

arch_result arch_init(void)
{
    x86_64_gdt_init();

    arch_interrupt_init();
    
    // Initialize debug output early
    x86_64_serial_init();
    arch_debug_printf("x86_64: arch initialization complete\n");
    
    return ARCH_OK;
}