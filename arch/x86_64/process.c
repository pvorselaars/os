#include "arch/memory.h"
#include "arch/x86_64/memory.h"
#include "kernel/base.h"
#include "lib/memory.h"
#include "lib/utils.h"

struct arch_process_context {
    uint64_t rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t vector, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

#include "arch/process.h"

void x86_64_process_start(struct arch_process_context * context);

static struct arch_process_context contexts[MAX_PROCESS_COUNT];

arch_process_context_t* arch_process_context_create(const uint64_t entry) {
    static uint32_t id = -1;
    id++;
    assert(id < MAX_PROCESS_COUNT);

    const uint64_t stack = (uint64_t) arch_memory_allocate_page();
    assert(stack);
    arch_memory_map_page(PAGE_SIZE, stack, ARCH_MEMORY_MAP_WRITE | ARCH_MEMORY_MAP_USER);

    memory_zero_struct(&contexts[id]);
    contexts[id].rip = entry;
    contexts[id].rbp = PAGE_SIZE;
    contexts[id].rsp = 2 * PAGE_SIZE;
    contexts[id].rflags = 0x202;
    contexts[id].cs = USER_CODE_SEG | 3;
    contexts[id].ss = USER_DATA_SEG | 3;

    return &contexts[id];
}

void arch_process_save_context(const arch_process_context_t *from, arch_process_context_t *to) {
    memory_copy(to, from, sizeof(arch_process_context_t));
}