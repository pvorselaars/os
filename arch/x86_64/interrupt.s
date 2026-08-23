.code64
.section .text

.macro EXCEPTION_HANDLER_NOERR vector
.globl exception_\vector
exception_\vector:
    pushq $0
    pushq $\vector
    jmp common_interrupt_handler
.endm

.macro EXCEPTION_HANDLER_ERR vector
.globl exception_\vector
exception_\vector:
    pushq $\vector
    jmp common_interrupt_handler
.endm

.macro IRQ_HANDLER vector
.globl irq_\vector
irq_\vector:
    pushq $0
    pushq $\vector
    jmp common_interrupt_handler
.endm

common_interrupt_handler:
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %r11
    pushq %r10
    pushq %r9
    pushq %r8
    pushq %rbp
    pushq %rdi
    pushq %rsi
    pushq %rdx
    pushq %rcx
    pushq %rbx
    pushq %rax

    movq 120(%rsp), %rdi    # Vector is at offset 120 (15*8 + 8)
    movq %rsp, %rsi         # Context pointer
    call x86_64_handle_interrupt
    movq %rax, %rsp

.globl x86_64_restore_context
x86_64_restore_context:
    popq %rax
    popq %rbx
    popq %rcx
    popq %rdx
    popq %rsi
    popq %rdi
    popq %rbp
    popq %r8
    popq %r9
    popq %r10
    popq %r11
    popq %r12
    popq %r13
    popq %r14
    popq %r15

    addq $16, %rsp
    
    iretq

EXCEPTION_HANDLER_NOERR 0   # Divide by zero
EXCEPTION_HANDLER_NOERR 2   # NMI
EXCEPTION_HANDLER_NOERR 4   # Overflow
EXCEPTION_HANDLER_ERR   8   # Double fault
EXCEPTION_HANDLER_ERR   13  # General protection fault
EXCEPTION_HANDLER_ERR   14  # Page fault

IRQ_HANDLER 0x20  # Timer
IRQ_HANDLER 0x21  # PS2 Keyboard
IRQ_HANDLER 0x24  # Serial
