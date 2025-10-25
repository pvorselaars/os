#include "platform/init.h"
#include "platform/console.h"
#include "platform/pc/pit.h"
#include "platform/pc/pic.h"
#include "arch/x86_64/io.h"
#include "arch/x86_64/interrupt.h"
#include "arch/x86_64/gdt.h"
#include "arch/interrupt.h"

void platform_halt()
{
    while (1)
        __asm__ volatile("hlt");
}

void platform_pic_remap()
{
    outb(PIC1, PIC_INIT | PIC_4);
    outb(PIC2, PIC_INIT | PIC_4);

    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    outb(PIC1_DATA, 4);
    outb(PIC2_DATA, 2);

    outb(PIC1_DATA, PIC_8086);
    outb(PIC2_DATA, PIC_8086);

    outb(PIC1_DATA, (uint8_t)0b11101000);
    outb(PIC2_DATA, (uint8_t)0xff);
}

uint64_t ticks = 0;

void platform_pit_init()
{
    uint16_t divisor = 1193180 / 1000;
    outb(PIT_COMMAND, 0x34);
    outb(PIT_CHANNEL_0, (uint8_t)(divisor));
    outb(PIT_CHANNEL_0, (uint8_t)(divisor >> 8));

    return;
}

void platform_pic_eoi(unsigned vector)
{
    /* If slave PIC (IRQ >= 8) send EOI to slave first */
    if (vector >= 0x28)
        outb(PIC2, 0x20);

    /* Always send EOI to master */
    if (vector >= 0x20 && vector < 0x30)
        outb(PIC1, 0x20);
}

void divide_by_zero_handler()
{
    platform_console_write("Divide by zero!", 16);
    platform_halt();
}

void nmi_handler()
{
    platform_console_write("Non-maskable interrupt!", 24);
    platform_halt();
}

void overflow_handler()
{
    platform_console_write("Overflow!", 9);
    platform_halt();
}

void double_fault_handler()
{
    platform_console_write("Double fault!", 13);
    platform_halt();
}

void general_protection_fault_handler()
{
    platform_console_write("General protection fault!", 25);
    platform_halt();
}

void page_fault_handler()
{
    platform_console_write("Page fault!", 11);
    platform_halt();
}

void timer_interrupt_handler()
{
    platform_console_write("Timer interrupt!", 16);
    platform_halt();
}

void platform_init(void)
{
    platform_console_init();

    arch_gdt_init();
    arch_interrupt_init();

    arch_register_interrupt(0, (addr_t)divide_by_zero_handler);
    arch_register_interrupt(2, (addr_t)nmi_handler);
    arch_register_interrupt(4, (addr_t)overflow_handler);
    arch_register_interrupt(8, (addr_t)double_fault_handler);
    arch_register_interrupt(13, (addr_t)general_protection_fault_handler);
    arch_register_interrupt(14, (addr_t)page_fault_handler);
    arch_register_interrupt(0x20, (addr_t)timer_interrupt_handler);

    platform_pic_remap();

    arch_irq_enable();

    platform_pit_init();
}