.globl process_start

.code64
.section .text

process_start:
    mov $0x43, %rcx    # user data segment selector (index 8, RPL=3)
    mov %rcx, %ds
    mov %rcx, %es
    mov %rcx, %fs
    mov %rcx, %gs

    # Load kernel stack base and pointer from struct
    mov 8(%rdi), %rbp  # kstack_base
    mov 16(%rdi), %rsp # kstack_pointer

    push %rcx          # SS (user data selector)
    # Compute top of user stack: ustack_base (24(%rdi)) + size (40(%rdi))
    mov 24(%rdi), %rbx  # ustack_base
    add 40(%rdi), %rbx  # ustack_base + size
    push %rbx           # RSP

    pushfq             # RFLAGS
    mov $0x3B, %rcx    # user code segment selector (index 7, RPL=3)
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