#include "interrupt.h"

interrupt_descriptor idt[MAX_INTERRUPTS];
idt_descriptor idtr;

uint64_t ticks = 0;

extern void load_idt(idt_descriptor * idtr);

extern void interrupt0();
extern void interrupt2();
extern void interrupt4();
extern void interrupt13();
extern void interrupt14();
extern void interrupt20();

void zero()
{
	fatal("Divide by zero!");
}

void nmi()
{
	fatal("Non-maskable interrupt!");
}

void overflow()
{
	fatal("Overflow!");
}

void general_protection_fault(address instruction, unsigned long selector)
{
	fatal("General protection fault (%lx:%x)!", instruction, selector);
}

void page_fault(address instruction, address a, unsigned long error)
{
	fatal("Page fault (%lx %s %lx)!", instruction, (error & 2) ? "writing" : "reading", a);
}

void remap_PIC()
{
	outb(PIC1, PIC_INIT | PIC_4);
	outb(PIC2, PIC_INIT | PIC_4);

	outb(PIC1_DATA, 0x20);
	outb(PIC2_DATA, 0x28);

	outb(PIC1_DATA, 4);
	outb(PIC2_DATA, 2);

	outb(PIC1_DATA, PIC_8086);
	outb(PIC2_DATA, PIC_8086);

	outb(PIC1_DATA, 0xfe);
	outb(PIC2_DATA, 0xff);

}

void pit_init()
{
    uint16_t divisor = 1193180 / 1000;
    outb(PIT_COMMAND, 0x34);
    outb(PIT_CHANNEL_0, (uint8_t) (divisor));
    outb(PIT_CHANNEL_0, (uint8_t) (divisor >> 8));

	return;
}

void register_interrupt(interrupt_descriptor * idt, unsigned int number, int selector, void (*function)(void),
			privilege_level access, gate_type type, int stack_table)
{

	assert(number < MAX_INTERRUPTS - 1);

	uint64_t handler = (uint64_t)function;

	idt[number].offset_low = handler & 0xffff;
	idt[number].selector = selector;
	idt[number].ist = stack_table & 0x07;
	idt[number].flags = 0x80 | (access << 5) | type;
	idt[number].offset_mid = (handler >> 16) & 0xffff;
	idt[number].offset_high = (handler >> 32) & 0xffffffff;
	idt[number].reserved = 0;

}

void interrupt_init()
{
	remap_PIC();
	pit_init();

	idtr.size = MAX_INTERRUPTS * sizeof(interrupt_descriptor) - 1;
	idtr.offset = idt;

	register_interrupt(idt, 0x0,  CODE_SEG, interrupt0, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 0x2,  CODE_SEG, interrupt2, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 0x4,  CODE_SEG, interrupt4, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 0xD,  CODE_SEG, interrupt13, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 0xE,  CODE_SEG, interrupt14, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 0x20, CODE_SEG, interrupt20, KERNEL, INTERRUPT_GATE, 0);

	load_idt(&idtr);
	enable_interrupts();
}
