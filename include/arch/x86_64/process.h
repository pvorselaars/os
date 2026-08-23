#ifndef X86_64_PROCESS_H
#define X86_64_PROCESS_H

#include "arch/process.h"

struct arch_process_context {
    uint64_t cr2;
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t vector, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

#endif
