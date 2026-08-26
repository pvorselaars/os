#include "arch/x86_64/memory.h"
#include "arch/x86_64/process.h"

#include "vmm.h"
#include "kernel/base.h"
#include "lib/memory.h"
#include "lib/utils.h"

static arch_process_context_t contexts[MAX_PROCESS_COUNT];

arch_process_context_t* arch_process_context_create(const uint64_t entry) {
    static uint32_t id = -1;
    id++;
    assert(id < MAX_PROCESS_COUNT);

    const uint64_t stack_bottom = PAGE_SIZE; // add unmapped guard page at bottom
    const uint64_t stack_top = stack_bottom + PAGE_SIZE;
    const uint64_t stack_page = (uint64_t) arch_memory_allocate_page();

    const uint64_t physical = PHYSICAL_ADDRESS(entry);
    const uint64_t code_page = ALIGN_DOWN(physical, PAGE_SIZE);
    const uint64_t page_table = x86_64_vmm_page_table_create();

    arch_memory_map_page(page_table, code_page, code_page, ARCH_MEMORY_MAP_READ | ARCH_MEMORY_MAP_USER);
    arch_memory_map_page(page_table, stack_bottom, stack_page, ARCH_MEMORY_MAP_WRITE | ARCH_MEMORY_MAP_USER);

    memory_zero_struct(&contexts[id]);
    contexts[id].cr3 = page_table;
    contexts[id].rip = physical;
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