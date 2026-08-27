#include "kernel/syscall.h"
#include "arch/x86_64/memory.h"
#include "arch/x86_64/syscall.h"

#include "lib/utils.h"

#include "arch/x86_64/registers.h"

extern void x86_64_syscall_entry();

typedef struct cpu_local {
    uint64_t kernel_stack_top;
} cpu_local_t;

static cpu_local_t cpu0;

void x86_64_syscall_set_kernel_stack(const uint64_t stack_top) {
   cpu0.kernel_stack_top = stack_top;
}

void x86_64_syscall_init() {
    x86_64_write_msr(X86_64_MSR_IA32_STAR, (uint64_t)DATA_SEG << 48 | (uint64_t)CODE_SEG << 32);
    x86_64_write_msr(X86_64_MSR_IA32_EFER, x86_64_read_msr(X86_64_MSR_IA32_EFER) | 1);
    x86_64_write_msr(X86_64_MSR_IA32_LSTAR, (uint64_t)x86_64_syscall_entry);
    x86_64_write_msr(X86_64_MSR_IA32_FMASK, 1 << 9);
    x86_64_write_msr(X86_64_MSR_IA32_KERNEL_GS_BASE, (uint64_t)&cpu0);
}

result_t syscall_dispatch(const arch_syscall_frame_t* frame) {
    return syscall_handle(frame->rdi, frame->rsi);
}
