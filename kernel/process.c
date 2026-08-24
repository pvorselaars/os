#include "kernel/process.h"
#include "arch/process.h"

#include "memory.h"
#include "arch/cpu.h"
#include "arch/memory.h"
#include "lib/memory.h"

typedef enum {
    PROCESS_DONE,
    PROCESS_READY,
    PROCESS_RUNNING,
} process_state_t;

typedef struct process_t {
    process_state_t state;
    arch_process_context_t *context;
} process_t;

process_t processes[MAX_PROCESS_COUNT];

static int32_t current_process = -1;

static void map_user_page(uint64_t address) {
    const uint64_t physical = PHYSICAL_ADDRESS(address);
    const uint64_t page = ALIGN_DOWN(physical, PAGE_SIZE);

    arch_memory_map_page(page, page, ARCH_MEMORY_MAP_READ | ARCH_MEMORY_MAP_USER);
}

void process_create(void (*entry)(void)) {
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        if (processes[i].state == PROCESS_DONE) {

            map_user_page((uint64_t)entry);

            processes[i].context = arch_process_context_create(PHYSICAL_ADDRESS(entry));
            processes[i].state = PROCESS_READY;
            return;
        }
    }
}

void process_start_scheduler() {
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        if (processes[i].state == PROCESS_READY) {
            current_process = i;
        }
    }
}

arch_process_context_t *process_schedule(arch_process_context_t *current) {
    if (current_process == -1)
        return current;

    processes[current_process].state = PROCESS_READY;

    int32_t next = current_process == MAX_PROCESS_COUNT - 1 ? 0 : current_process + 1;
    while (processes[next].state != PROCESS_READY) {
        next = next == MAX_PROCESS_COUNT - 1 ? 0 : next + 1;
    }

    processes[next].state = PROCESS_RUNNING;

    if (next != current_process) {
        arch_process_save_context(current, processes[current_process].context);
    }

    current_process = next;

    return processes[next].context;
}