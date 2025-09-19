#include "interrupt.h"

extern uint64_t gdt[7];
interrupt_descriptor idt[MAX_INTERRUPTS];
idt_descriptor idtr;
tss64 tss;

uint64_t kernel_stack[512];
uint64_t gp_stack[512];
uint64_t df_stack[512];

uint64_t ticks = 0;

extern void load_idt(idt_descriptor * idtr);
extern void load_tss(uint32_t selector);

extern void interrupt0();
extern void interrupt2();
extern void interrupt4();
extern void interrupt8();
extern void interrupt13();
extern void interrupt14();
extern void interrupt32();
extern void interrupt33();
extern void interrupt36();

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

void double_fault()
{
	fatal("Double fault!");
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

	outb(PIC1_DATA, (uint8_t)0b11101000);
	outb(PIC2_DATA, (uint8_t)0xff);
}

void pit_init()
{
    uint16_t divisor = 1193180 / 1000;
    outb(PIT_COMMAND, 0x34);
    outb(PIT_CHANNEL_0, (uint8_t) (divisor));
    outb(PIT_CHANNEL_0, (uint8_t) (divisor >> 8));

	return;
}

void set_gdt_entry(int index, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags)
{
	assert(index <= 5);

	gdt[index] = 0;

	gdt[index] |= (limit & 0xffff);            		// Limit bits 0-15
	gdt[index] |= (base & 0xffffff) << 16;     		// Base bits 0-23
	gdt[index] |= (uint64_t)(access & 0xff) << 40;  // Access byte
	gdt[index] |= ((limit >> 16) & 0xf) << 48; 		// Limit bits 16-19
	gdt[index] |= (uint64_t)(flags & 0xf) << 52;    // Flags
	gdt[index] |= ((base >> 24) & 0xff) << 56; 		// Base bits 24-31
}

void set_tss_entry(int index, uint64_t base, uint64_t limit, uint8_t access, uint8_t flags)
{
	gdt[index] = 0;
	gdt[index+1] = 0;

	set_gdt_entry(index, base, limit, access, flags);

	gdt[index+1] |= (base >> 32) & 0xffffffff;   // Base bits 32-63
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

	tss.rsp0 = (uint64_t) &kernel_stack[511];
	tss.ist1 = (uint64_t) &df_stack[511];
	tss.ist2 = (uint64_t) &gp_stack[511];
	tss.iomap_base = sizeof(tss64);

	set_tss_entry(5, (uint64_t)&tss, sizeof(tss64)-1, 0x89, 0);
	load_tss(5*8);

	idtr.size = MAX_INTERRUPTS * sizeof(interrupt_descriptor) - 1;
	idtr.offset = idt;

	register_interrupt(idt, 0x0,  CODE_SEG, interrupt0, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 0x2,  CODE_SEG, interrupt2, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 0x4,  CODE_SEG, interrupt4, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 0x8,  CODE_SEG, interrupt8, KERNEL, INTERRUPT_GATE, 1);
	register_interrupt(idt, 0xD,  CODE_SEG, interrupt13, KERNEL, INTERRUPT_GATE, 2);
	register_interrupt(idt, 0xE,  CODE_SEG, interrupt14, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 0x20, CODE_SEG, interrupt32, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 0x21, CODE_SEG, interrupt33, KERNEL, INTERRUPT_GATE, 0);
	register_interrupt(idt, 0x24, CODE_SEG, interrupt36, KERNEL, INTERRUPT_GATE, 0);

	load_idt(&idtr);

	pit_init();
	enable_interrupts();
}
