#ifndef X86_64_SYSCALL_H
#define X86_64_SYSCALL_H

#include "arch/syscall.h"

struct arch_syscall_frame {
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rbx;
    uint64_t rip;
    uint64_t rflags;
    uint64_t rsp;
};

void x86_64_syscall_init();

void x86_64_syscall_set_kernel_stack(uint64_t stack_top);

__attribute__((always_inline)) inline
result_t arch_syscall(uint64_t number, uint64_t arg1)
{
    result_t result;

    __asm__ volatile(
        "syscall"
        : "=a"(result)
        : "D"(number), "S"(arg1)
        : "rcx", "r11", "memory"
    );

    return result;
}

#endif
