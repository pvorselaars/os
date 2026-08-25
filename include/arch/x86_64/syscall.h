#ifndef X86_64_SYSCALL_H
#define X86_64_SYSCALL_H

void x86_64_syscall_init();

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
