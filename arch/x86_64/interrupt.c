#include "arch/cpu.h"
#include "arch/interrupt.h"

#include "io.h"
#include "arch/x86_64/process.h"
#include "arch/x86_64/tss.h"
#include "arch/x86_64/idt.h"
#include "kernel/process.h"
#include "lib/utils.h"

static arch_interrupt_handler_t interrupt_handlers[256] = {0};

extern void exception_0(), exception_2(), exception_4();
extern void exception_8(), exception_13(), exception_14();
extern void irq_0x20(), irq_0x21(), irq_0x24();

static void default_exception_handler(const char *name, const arch_process_context_t *context)
{
    debug_printf("EXCEPTION: %s\n", name);
    debug_printf("Instruction 0x%x\n", context->rip);
    debug_printf("Error code: 0x%x\n", context->error_code);
    arch_cpu_halt();
}

static void divide_by_zero_handler(const arch_process_context_t *context) { default_exception_handler("Divide by zero", context); }
static void nmi_handler(const arch_process_context_t *context) { default_exception_handler("NMI", context); }
static void overflow_handler(const arch_process_context_t *context) { default_exception_handler("Overflow", context); }
static void double_fault_handler(const arch_process_context_t *context) { default_exception_handler("Double fault", context); }
static void gpf_handler(const arch_process_context_t *context) { default_exception_handler("General protection fault", context); }
static void page_fault_handler(const arch_process_context_t *context)
{
    debug_printf("EXCEPTION: Page fault\n");
    debug_printf("Instruction 0x%x ", context->rip);
    const uint64_t error = context->error_code;
    if (error & 0x1) {
        if (error & 0x2) {
            debug_printf("write violation at 0x%x\n", context->cr2);
        } else {
            debug_printf("read violation at 0x%x\n", context->cr2);
        }
    } else {
        debug_printf("page not present 0x%x\n", context->cr2);
    }
    arch_cpu_halt();
}

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

    arch_interrupt_register(0x0, (arch_interrupt_handler_t)divide_by_zero_handler);
    arch_interrupt_register(0x2, (arch_interrupt_handler_t)nmi_handler);
    arch_interrupt_register(0x4, (arch_interrupt_handler_t)overflow_handler);
    arch_interrupt_register(0x8, (arch_interrupt_handler_t)double_fault_handler);
    arch_interrupt_register(0xD, (arch_interrupt_handler_t)gpf_handler);
    arch_interrupt_register(0xE, (arch_interrupt_handler_t)page_fault_handler);

    x86_64_tss_init();
}

static void eoi(const unsigned vector) {
    if (vector >= 0x28)
        pc_io_write8(0xA0, 0x20);

    pc_io_write8(0x20, 0x20);
}

arch_process_context_t * x86_64_handle_interrupt(const unsigned vector, arch_process_context_t *context)
{
    if (vector < 256 && interrupt_handlers[vector]) {
        if (vector >= 0x20) {
            void (*handler)(arch_process_context_t *context) = (void (*)(arch_process_context_t*))interrupt_handlers[vector];
            handler(context);
        } else {
            interrupt_handlers[vector]();
        }

    }

    if (vector >= 0x20 && vector < 0x30)
        eoi(vector);

    if (vector == 0x20)
        return process_schedule(context);

    return context;
}

void arch_cpu_interrupt_enable()
{
    __asm__ volatile("sti");
}

void arch_cpu_interrupt_disable()
{
    __asm__ volatile("cli");
}
