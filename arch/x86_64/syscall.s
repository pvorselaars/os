.code64
.globl x86_64_syscall_entry

.section .text

x86_64_syscall_entry:
    movq %rsp, %r10 # save user RSP
    swapgs

    movq %gs:0, %rsp # kernel_stack_top

    pushq %r10 # user RSP
    pushq %r11 # user RFLAGS
    pushq %rcx # user RIP

    pushq %rbx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    movq %rsp, %rdi
    sti
    call syscall_dispatch
    cli

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rbx

    popq %rcx
    popq %r11
    popq %rsp

    swapgs
    sysretq