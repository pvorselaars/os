#include "definitions.h"
#include "arch/arch.h"
#include "arch/x86_64/idt.h"

static addr_t interrupt_handlers[256] = {0}; // Standard max interrupt vectors

// Assembly stubs we have available
extern void exception_0(void), exception_2(void), exception_4(void);
extern void exception_8(void), exception_13(void), exception_14(void);
extern void irq_0x20(void), irq_0x21(void), irq_0x24(void);

// Initialize interrupt system - pre-populate IDT with available stubs
arch_result arch_interrupt_init(void)
{
    // Initialize IDT structure
    x86_64_idt_init();
    
    // Pre-populate IDT with our available stubs
    x86_64_idt_set_entry(0, (addr_t)exception_0, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(2, (addr_t)exception_2, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(4, (addr_t)exception_4, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(8, (addr_t)exception_8, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(13, (addr_t)exception_13, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(14, (addr_t)exception_14, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x20, (addr_t)irq_0x20, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x21, (addr_t)irq_0x21, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x24, (addr_t)irq_0x24, IDT_FLAG_INTERRUPT_GATE);
    
    return ARCH_OK;
}

int arch_register_interrupt(unsigned vector, addr_t handler)
{
    if (vector >= 256) {
        return -1;
    }
    
    // Simply store the handler - arch-specific init handles IDT setup
    interrupt_handlers[vector] = handler;
    return 0;
}

void arch_handle_interrupt(unsigned vector)
{
    if (vector < 256 && interrupt_handlers[vector]) {
        // Call the registered C handler
        void (*handler)(void) = (void (*)(void))interrupt_handlers[vector];
        handler();
    }
}
