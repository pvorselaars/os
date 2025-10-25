.globl process_start

.code64
.section .text

process_start:
    mov $0x23, %rcx    # user data segment selector (index 4, RPL=3)
    mov %rcx, %ds
    mov %rcx, %es
    mov %rcx, %fs
    mov %rcx, %gs

    mov 8(%rdi), %rbp  # Setup kernel stack
    mov 16(%rdi), %rsp

    push %rcx          # SS
    mov 32(%rdi), %rbx # Top of process memory
    push %rbx          # RSP

    pushfq             # RFLAGS
    mov $0x1B, %rcx    # user code segment selector (index 3, RPL=3)
    push %rcx          # CS

    mov (%rdi), %rax   # entry
    push %rax          # RIP

    xor %rax, %rax     # clear registers
    xor %rbx, %rbx
    xor %rcx, %rcx
    xor %rdx, %rdx
    xor %rdi, %rdi
    xor %rsi, %rsi

    iretq              # pop into user mode