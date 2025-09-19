#ifndef PROCESS_H
#define PROCESS_H

#include "definitions.h"
#include "memory.h"

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    DONE
} process_state;

typedef struct process process;

struct process {
    process *parent;
    process_state state;
    void (*memory)();
    uint64_t size;
};

void process_init();
void schedule();

#endif