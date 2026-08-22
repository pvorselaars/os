#include "kernel/process.h"
#include "arch/process.h"

#include "memory.h"
#include "arch/memory.h"
#include "lib/memory.h"

typedef struct process_t {
    uint32_t id;
    arch_process_context_t *arch_process_context;
} process_t;

static process_t process = {
    .id = 1,
};

void process_start(void (*entry)(void)) {

    uint64_t start = (uint64_t)entry - (uint64_t)KERNEL_BASE;

    uint64_t page = ALIGN_DOWN(start, PAGE_SIZE);

    arch_memory_map_page(page, page, ARCH_MEMORY_MAP_WRITE | ARCH_MEMORY_MAP_USER);

    process.arch_process_context = arch_process_context_create(start);
    arch_process_start(process.arch_process_context);

}