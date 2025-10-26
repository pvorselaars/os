#ifndef IDT_H
#define IDT_H

#include "definitions.h"

#define MAX_INTERRUPTS 256

#define IDT_FLAG_INTERRUPT_GATE  0x8E  // Present, Ring 0, 32-bit Interrupt Gate
#define IDT_FLAG_TRAP_GATE       0x8F  // Present, Ring 0, 32-bit Trap Gate

typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t vector, error_code;
    uint64_t rip, cs, rflags, user_rsp, ss;
} interrupt_context;

int x86_64_idt_set_entry(unsigned vector, void (*handler)(void), uint8_t flags);
void x86_64_idt_init(void);
void x86_64_handle_interrupt(unsigned vector, interrupt_context *context);

#endif