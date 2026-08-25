.code64
.globl x86_64_syscall_entry

.section .text

// TODO: rsp is in user memory, this is unsafe
// save user rsp and restore kernel rsp
x86_64_syscall_entry:
    pushq %rcx
    pushq %r11

    call syscall_dispatch

    popq %r11
    popq %rcx
    sysretq