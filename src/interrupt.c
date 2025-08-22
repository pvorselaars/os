#include "interrupt.h"

#include "console.h"
#include "utils.h"
#include "memory.h"
#include "io.h"

#pragma pack(1)
typedef struct {
	unsigned short size;
	interrupt_descriptor *offset;
} idt_descriptor;
#pragma pack()

#define MAX_INTERRUPTS 256

#define PIC1					 0x20
#define PIC1_DATA			 (PIC1+1)

#define PIC2					 0xA0
#define PIC2_DATA			 (PIC2+1)

#define PIC_INIT			 0x10
#define PIC_4					 0x01
#define PIC_8086			 0x01

interrupt_descriptor idt[MAX_INTERRUPTS];
idt_descriptor idtr;

extern void load_idt(idt_descriptor * idtr);

extern void interrupt0();
extern void interrupt2();
extern void interrupt4();
extern void interrupt13();
extern void interrupt14();

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
	char mask1, mask2;

	mask1 = inb(PIC1_DATA);
	mask2 = inb(PIC2_DATA);

	outb(PIC1, PIC_INIT | PIC_4);
	outb(PIC2, PIC_INIT | PIC_4);

	outb(PIC1_DATA, 0x20);
	outb(PIC2_DATA, 0x28);

	outb(PIC1_DATA, 4);
	outb(PIC2_DATA, 2);

	outb(PIC1_DATA, PIC_8086);
	outb(PIC2_DATA, PIC_8086);

	outb(PIC1_DATA, mask1);
	outb(PIC2_DATA, mask2);

}

void register_interrupt(interrupt_descriptor * idt, unsigned int number, int selector, void (*function)(void),
			privilege_level access, gate_type type, int stack_table)
{

	assert(number < MAX_INTERRUPTS - 1);

	idt[number].offset_low = (unsigned long)function & 0xffff;
	idt[number].selector = selector;
	idt[number].flags = (1 << 15) | (access << 11) | (type << 8) | stack_table;
	idt[number].offset_mid = ((unsigned long)function >> 16);
	idt[number].offset_high = ((unsigned long)function >> 32);
	idt[number].reserved = 0;

}

void interrupt_init()
{

	idtr.size = MAX_INTERRUPTS * sizeof(interrupt_descriptor) - 1;
	idtr.offset = idt;

	register_interrupt(idt, 0,  CODE_SEG, interrupt0, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 2,  CODE_SEG, interrupt2, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 4,  CODE_SEG, interrupt4, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 13, CODE_SEG, interrupt13, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 14, CODE_SEG, interrupt14, KERNEL, INTERRUPT_GATE, 0);

	load_idt(&idtr);
	remap_PIC();
}
