#include "arch/x86_64/memory.h"
#include "arch/x86_64/process.h"
#include "kernel/base.h"
#include "lib/memory.h"
#include "lib/utils.h"

static arch_process_context_t contexts[MAX_PROCESS_COUNT];

arch_process_context_t* arch_process_context_create(const uint64_t entry, const arch_memory_address_space_t* space) {
    static uint32_t id = -1;
    id++;
    assert(id < MAX_PROCESS_COUNT);

    const uint64_t stack_bottom = PAGE_SIZE; // add unmapped guard page at bottom
    const uint64_t stack_top = stack_bottom + PAGE_SIZE;

    // TODO: processes should not share cr3/pml4
    const uint64_t page = (uint64_t) arch_memory_allocate_page();
    arch_memory_map_page(space, stack_bottom, page, ARCH_MEMORY_MAP_WRITE | ARCH_MEMORY_MAP_USER);

    memory_zero_struct(&contexts[id]);
    contexts[id].rip = entry;
    contexts[id].rbp = stack_top;
    contexts[id].rsp = stack_top;
    contexts[id].rflags = 0x202;
    contexts[id].cs = USER_CODE_SEG | 3;
    contexts[id].ss = USER_DATA_SEG | 3;

    return &contexts[id];
}

void arch_process_save_context(const arch_process_context_t *from, arch_process_context_t *to) {
    memory_copy(to, from, sizeof(arch_process_context_t));
}