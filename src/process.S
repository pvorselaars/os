.globl process_start

.code64
.section .text

process_start:
    mov $0x43, %rcx    # user data segment selector
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
    mov $0x3B, %rcx    # user code segment selector
    push %rcx          # CS

    mov (%rdi), %rax   # entry
    push %rax          # RIP

    iretq              # pop into user mode