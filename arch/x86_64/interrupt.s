.code64
.section .text

# Exception handlers (no error code)
.macro EXCEPTION_HANDLER_NOERR vector
.globl exception_\vector
exception_\vector:
    pushq $0          # Dummy error code
    pushq $\vector    # Vector number
    jmp common_interrupt_handler
.endm

# Exception handlers (with error code)  
.macro EXCEPTION_HANDLER_ERR vector
.globl exception_\vector
exception_\vector:
    pushq $\vector    # Vector number (error code already pushed by CPU)
    jmp common_interrupt_handler
.endm

# Hardware interrupt handlers
.macro IRQ_HANDLER vector
.globl irq_\vector
irq_\vector:
    pushq $0          # Dummy error code
    pushq $\vector    # Vector number
    jmp common_interrupt_handler
.endm

# Common interrupt handler
common_interrupt_handler:
    # Save all registers
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    
    # Get vector number from stack
    movq 120(%rsp), %rdi    # Vector is at offset 120 (15*8 + 8)
    movq %rsp, %rsi         # Context pointer
    call arch_handle_interrupt
    
    # Restore all registers
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    
    # Clean up error code and vector from stack
    addq $16, %rsp
    
    # Send EOI for hardware interrupts (0x20-0x2F)
    cmpq $0x20, -16(%rsp)  # Check vector 
    jb skip_eoi
    cmp $0x30, -16(%rsp)
    jae skip_eoi
    mov $0x20, %al
    out %al, $0x20
skip_eoi:
    iretq

# Generate exception handlers
EXCEPTION_HANDLER_NOERR 0   # Divide by zero
EXCEPTION_HANDLER_NOERR 2   # NMI
EXCEPTION_HANDLER_NOERR 4   # Overflow
EXCEPTION_HANDLER_ERR   8   # Double fault
EXCEPTION_HANDLER_ERR   13  # General protection fault
EXCEPTION_HANDLER_ERR   14  # Page fault

# Generate IRQ handlers  
IRQ_HANDLER 0x20  # Timer
IRQ_HANDLER 0x21  # PS2 Keyboard
IRQ_HANDLER 0x24  # Serial
