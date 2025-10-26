#include "platform/init.h"
#include "platform/timer.h"
#include "platform/pc/pit.h"
#include "platform/pc/pic.h"
#include "platform/pc/serial.h"
#include "arch/x86_64/io.h"
#include "arch/x86_64/gdt.h"
#include "arch/interrupt.h"
#include "lib/printf.h"

extern void timer_interrupt(void);
extern void ps2_keyboard_interrupt(void);
extern void serial_receive_interrupt(void);

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
    printf("Divide by zero!\n");
    platform_halt();
}

void nmi_handler()
{
    printf("Non-maskable interrupt!\n");
    platform_halt();
}

void overflow_handler()
{
    printf("Overflow!\n");
    platform_halt();
}

void double_fault_handler()
{
    printf("Double fault!\n");
    platform_halt();
}

void general_protection_fault_handler()
{
    printf("General protection fault!\n");
    platform_halt();
}

void page_fault_handler()
{
    printf("Page fault!\n");
    platform_halt();
}

static unsigned long timer_ticks = 0;

void timer_interrupt()
{
    timer_ticks++;
    
    if (timer_ticks % 1000 == 0) {
        printf("Timer: %lu seconds\n", timer_ticks / 1000);
    }
    
}

void platform_init(void)
{

    arch_register_interrupt(0, (addr_t)divide_by_zero_handler);
    arch_register_interrupt(2, (addr_t)nmi_handler);
    arch_register_interrupt(4, (addr_t)overflow_handler);
    arch_register_interrupt(8, (addr_t)double_fault_handler);
    arch_register_interrupt(13, (addr_t)general_protection_fault_handler);
    arch_register_interrupt(14, (addr_t)page_fault_handler);
    arch_register_interrupt(0x20, (addr_t)timer_interrupt);
    arch_register_interrupt(0x21, (addr_t)ps2_keyboard_interrupt);

    platform_pic_remap();

    platform_timer_init(1000); // 1000 Hz
}