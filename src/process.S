.globl process_start
.globl fake_init

.code64
.section .text

process_start:
    mov $0x43, %rcx    # user data segment selector
    mov %rcx, %ds
    mov %rcx, %es
    mov %rcx, %fs
    mov %rcx, %gs
    push %rcx          # SS
    push %rsi          # RSP
    pushfq             # RFLAGS
    mov $0x3B, %rcx    # user code segment selector
    push %rcx          # CS
    push %rdi          # RIP

    iretq              # pop into user mode