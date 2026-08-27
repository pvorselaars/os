#include "kernel/process.h"
#include "arch/process.h"

#include "arch/cpu.h"

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

void process_create(void (*entry)()) {
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        if (processes[i].state == PROCESS_DONE) {
            processes[i].context = arch_process_context_create(i, (uint64_t)entry);
            processes[i].state = PROCESS_READY;
            return;
        }
    }
}

void process_scheduler_start() {
    arch_cpu_interrupt_disable();
    for (int i = 0; i < MAX_PROCESS_COUNT; i++) {
        if (processes[i].state == PROCESS_READY) {
            current_process = i;
            processes[i].state = PROCESS_RUNNING;
            arch_process_activate(i);
            arch_process_start(processes[i].context);
        }
    }
}

arch_process_context_t *process_schedule(arch_process_context_t *current) {
    if (current_process == -1)
        return current;

    int32_t next = current_process;

    for (int32_t offset = 1; offset < MAX_PROCESS_COUNT; offset++) {
        int32_t i = (current_process + offset) % MAX_PROCESS_COUNT;
        if (processes[i].state == PROCESS_READY) {
            next = i;
            break;
        }
    }

    processes[current_process].state = PROCESS_READY;
    processes[next].state = PROCESS_RUNNING;

    if (next != current_process) {
        processes[current_process].context = current;
        arch_process_activate(next);
        current_process = next;
        return processes[next].context;
    }

    return current;
}