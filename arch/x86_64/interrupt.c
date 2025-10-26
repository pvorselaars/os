#include "definitions.h"
#include "arch/arch.h"
#include "arch/x86_64/idt.h"
#include "arch/x86_64/pic.h"

static void (*interrupt_handlers[256])(void) = {0};

extern void exception_0(void), exception_2(void), exception_4(void);
extern void exception_8(void), exception_13(void), exception_14(void);
extern void irq_0x20(void), irq_0x21(void), irq_0x24(void);

arch_result arch_interrupt_init(void)
{
    x86_64_idt_init();
    
    x86_64_idt_set_entry(0, exception_0, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(2, exception_2, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(4, exception_4, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(8, exception_8, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(13, exception_13, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(14, exception_14, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x20, irq_0x20, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x21, irq_0x21, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x24, irq_0x24, IDT_FLAG_INTERRUPT_GATE);

    x86_64_pic_remap();

    return ARCH_OK;
}

static void default_exception_handler(const char *name)
{
    arch_debug_printf("Exception: %s\n", name);
    arch_halt();
}

static void divide_by_zero_handler(void) { default_exception_handler("Divide by zero"); }
static void nmi_handler(void) { default_exception_handler("NMI"); }
static void overflow_handler(void) { default_exception_handler("Overflow"); }
static void double_fault_handler(void) { default_exception_handler("Double fault"); }
static void gpf_handler(void) { default_exception_handler("General protection fault"); }
static void page_fault_handler(void) { default_exception_handler("Page fault"); }
extern void timer_handler(void);

arch_result arch_register_default_handlers(void)
{
    arch_register_interrupt(0, divide_by_zero_handler);
    arch_register_interrupt(2, nmi_handler);
    arch_register_interrupt(4, overflow_handler);
    arch_register_interrupt(8, double_fault_handler);
    arch_register_interrupt(13, gpf_handler);
    arch_register_interrupt(14, page_fault_handler);
    arch_register_interrupt(0x20, timer_handler);

    return ARCH_OK;
}

int arch_register_interrupt(unsigned vector, void (*handler)(void))
{
    if (vector >= 256) {
        return -1;
    }
    
    interrupt_handlers[vector] = handler;
    return 0;
}

void arch_handle_interrupt(unsigned vector)
{
    if (vector < 256 && interrupt_handlers[vector]) {
        void (*handler)(void) = (void (*)(void))interrupt_handlers[vector];
        handler();
    }
}

void arch_interrupt_enable(void)
{
    __asm__ volatile("sti");
}

void arch_interrupt_disable(void)
{
    __asm__ volatile("cli");
}
