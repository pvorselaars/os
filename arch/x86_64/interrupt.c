#include "arch/cpu.h"
#include "arch/interrupt.h"

#include "arch/x86_64/tss.h"
#include "arch/x86_64/idt.h"
#include "lib/utils.h"

static void (*interrupt_handlers[256])(void) = {0};

extern void exception_0(void), exception_2(void), exception_4(void);
extern void exception_8(void), exception_13(void), exception_14(void);
extern void irq_0x20(void), irq_0x21(void), irq_0x24(void);

static void default_exception_handler(const char *name)
{
    debug_printf("Exception: %s\n", name);
    arch_cpu_halt();
}

static void divide_by_zero_handler() { default_exception_handler("Divide by zero"); }
static void nmi_handler() { default_exception_handler("NMI"); }
static void overflow_handler() { default_exception_handler("Overflow"); }
static void double_fault_handler() { default_exception_handler("Double fault"); }
static void gpf_handler() { default_exception_handler("General protection fault"); }
static void page_fault_handler() { default_exception_handler("Page fault"); }

result_t arch_interrupt_register(uint32_t vector, arch_interrupt_handler_t handler)
{
    if (vector >= 256 || !handler) {
        return RESULT_ERROR;
    }

    interrupt_handlers[vector] = handler;
    return RESULT_OK;
}

void x86_64_interrupt_init()
{
    x86_64_idt_init();

    x86_64_idt_set_entry(0x0, exception_0, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x2, exception_2, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x4, exception_4, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x8, exception_8, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0xD, exception_13, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0xE, exception_14, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x20, irq_0x20, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x21, irq_0x21, IDT_FLAG_INTERRUPT_GATE);
    x86_64_idt_set_entry(0x24, irq_0x24, IDT_FLAG_INTERRUPT_GATE);

    arch_interrupt_register(0x0, divide_by_zero_handler);
    arch_interrupt_register(0x2, nmi_handler);
    arch_interrupt_register(0x4, overflow_handler);
    arch_interrupt_register(0x8, double_fault_handler);
    arch_interrupt_register(0xD, gpf_handler);
    arch_interrupt_register(0xE, page_fault_handler);

    x86_64_tss_init();
}

void arch_handle_interrupt(const unsigned vector)
{
    if (vector < 256 && interrupt_handlers[vector]) {
        void (*handler)() = interrupt_handlers[vector];
        handler();
    }
}

void arch_cpu_interrupt_enable(void)
{
    __asm__ volatile("sti");
}

void arch_cpu_interrupt_disable(void)
{
    __asm__ volatile("cli");
}
