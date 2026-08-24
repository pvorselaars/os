#include "arch/x86_64/memory.h"
#include "arch/x86_64/syscall.h"
#include "arch/x86_64/registers.h"

extern void syscall_entry();

void syscall_dispatch(const uint64_t syscall) {
    while (1);
}

void x86_64_syscall_init() {
    x86_64_write_msr(X86_64_MSR_IA32_STAR, (uint64_t)USER_CODE_SEG << 48 | (uint64_t)CODE_SEG << 32);
    x86_64_write_msr(X86_64_MSR_IA32_EFER, x86_64_read_msr(X86_64_MSR_IA32_EFER) | 1);
    x86_64_write_msr(X86_64_MSR_IA32_LSTAR, (uint64_t)syscall_entry);
    x86_64_write_msr(X86_64_MSR_IA32_FMASK, 1 << 9);
}