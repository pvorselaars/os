#include "arch/x86_64/memory.h"
.globl arch_process_start

.code64
.section .text

arch_process_start:
    mov $(USER_DATA_SEG | 3), %rcx    # user data segment selector
    mov %rcx, %ds
    mov %rcx, %es
    mov %rcx, %fs
    mov %rcx, %gs
    push %rcx          # SS

    mov $PAGE_SIZE, %rbp
    mov 8(%rdi), %rbx
    push %rbx          # RSP

    pushfq                            # RFLAGS
    mov $(USER_CODE_SEG | 3), %rcx    # user code segment selector
    push %rcx                         # CS

    mov (%rdi), %rax   # entry
    push %rax          # RIP

    xor %rax, %rax     # clear registers
    xor %rbx, %rbx
    xor %rcx, %rcx
    xor %rdx, %rdx
    xor %rdi, %rdi
    xor %rsi, %rsi

    iretq