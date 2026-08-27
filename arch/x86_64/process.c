#include "arch/x86_64/memory.h"
#include "arch/x86_64/process.h"

#include "syscall.h"
#include "tss.h"
#include "vmm.h"
#include "kernel/base.h"
#include "lib/memory.h"
#include "lib/utils.h"

static arch_process_context_t contexts[MAX_PROCESS_COUNT];
static uint64_t kernel_stack_tops[MAX_PROCESS_COUNT];

#define KERNEL_STACK_AREA ((uint64_t)KERNEL_BASE + 0x400000)
#define KERNEL_STACK_SLOT (2 * PAGE_SIZE)

arch_process_context_t* arch_process_context_create(const uint32_t id, uint64_t entry) {
    assert(id < MAX_PROCESS_COUNT);

    // Create user stack page
    const uint64_t stack_bottom = PAGE_SIZE; // add unmapped guard page at bottom
    const uint64_t stack_top = stack_bottom + PAGE_SIZE;
    const uint64_t stack_page = (uint64_t)arch_memory_allocate_page();

    // Create code page
    const uint64_t physical = PHYSICAL_ADDRESS(entry);
    const uint64_t code_page = ALIGN_DOWN(physical, PAGE_SIZE);

    // Create kernel stack page
    const uint64_t kernel_stack_bottom = KERNEL_STACK_AREA + id * KERNEL_STACK_SLOT + PAGE_SIZE;
    const uint64_t kernel_stack_top = kernel_stack_bottom + PAGE_SIZE;
    const uint64_t kernel_stack_page = (uint64_t)arch_memory_allocate_page();

    // Create process page table
    const uint64_t page_table = x86_64_vmm_page_table_create();

    // Map pages into process page table
    arch_memory_map_page(page_table, code_page, code_page, ARCH_MEMORY_MAP_READ | ARCH_MEMORY_MAP_USER);
    arch_memory_map_page(page_table, stack_bottom, stack_page, ARCH_MEMORY_MAP_WRITE | ARCH_MEMORY_MAP_USER);
    arch_memory_map_page(page_table, kernel_stack_bottom, kernel_stack_page, ARCH_MEMORY_MAP_WRITE);

    kernel_stack_tops[id] = kernel_stack_top;

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

void arch_process_activate(uint32_t id) {
    assert(id < MAX_PROCESS_COUNT);
    arch_set_interrupt_stack_pointer(kernel_stack_tops[id]);
    x86_64_syscall_set_kernel_stack(kernel_stack_tops[id]);
}

void arch_process_start(arch_process_context_t* context)
{
    x86_64_process_start(context);
}

void arch_process_save_context(const arch_process_context_t *from, arch_process_context_t *to) {
    memory_copy(to, from, sizeof(arch_process_context_t));
}