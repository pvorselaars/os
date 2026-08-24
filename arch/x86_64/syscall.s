.code64
.globl syscall_entry
.globl syscall

.section .text

syscall_entry:
    pushq %rcx
    pushq %r11

    mov %rax, %rdi

    call syscall_dispatch

    popq %r11
    popq %rcx
    sysretq

syscall:
    movq %rdi, %rax
    syscall
    ret