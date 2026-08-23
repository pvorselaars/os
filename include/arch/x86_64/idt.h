#ifndef IDT_H
#define IDT_H

#include "kernel/base.h"

#define MAX_INTERRUPTS 256

#define IDT_FLAG_INTERRUPT_GATE  0x8E  // Present, Ring 0, 32-bit Interrupt Gate
#define IDT_FLAG_TRAP_GATE       0x8F  // Present, Ring 0, 32-bit Trap Gate

int x86_64_idt_set_entry(unsigned vector, void (*handler)(), uint8_t flags);
void x86_64_idt_init();

#endif