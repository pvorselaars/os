#ifndef IDT_H
#define IDT_H

#include "definitions.h"

#define MAX_INTERRUPTS 256

// IDT entry flags
#define IDT_FLAG_INTERRUPT_GATE  0x8E  // Present, Ring 0, 32-bit Interrupt Gate
#define IDT_FLAG_TRAP_GATE       0x8F  // Present, Ring 0, 32-bit Trap Gate

// x86_64 interrupt context (saved by assembly wrapper)
typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t vector, error_code;
    uint64_t rip, cs, rflags, user_rsp, ss;
} interrupt_context;

int x86_64_idt_set_entry(unsigned vector, addr_t handler, uint8_t flags);
void x86_64_idt_init(void);
void x86_64_handle_interrupt(unsigned vector, interrupt_context *context);

// Assembly interrupt stub declarations
extern void exception_0(void);   // Divide by zero
extern void exception_2(void);   // NMI  
extern void exception_4(void);   // Overflow
extern void exception_8(void);   // Double fault
extern void exception_13(void);  // General protection fault
extern void exception_14(void);  // Page fault
extern void irq_0x20(void);      // Timer
extern void irq_0x21(void);      // PS2 Keyboard
extern void irq_0x24(void);      // Serial

#endif