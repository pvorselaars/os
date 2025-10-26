#include "arch/arch.h"
#include "arch/x86_64/gdt.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/serial.h"

arch_result arch_init(void)
{
    x86_64_gdt_init();

    arch_interrupt_init();
    
    arch_result result = arch_memory_init();
    if (result != ARCH_OK) {
        return result;
    }
    
    result = arch_register_default_handlers();
    if (result != ARCH_OK) {
        return result;
    }
    
    result = arch_timer_init(100);
    if (result != ARCH_OK) {
        return result;
    }
    
    x86_64_serial_init();
    arch_debug_printf("x86_64: arch initialization complete\n");

    arch_interrupt_enable();
    
    return ARCH_OK;
}