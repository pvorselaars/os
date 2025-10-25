#ifndef PROCESS_H
#define PROCESS_H

#include "definitions.h"
#include "memory.h"

typedef enum
{
    READY,
    RUNNING,
    BLOCKED,
    DONE
} process_state;

typedef struct process process;

struct process
{
    void *entry;
    uint64_t kstack_base;
    uint64_t kstack_pointer;
    uint64_t ustack_base;
    uint64_t pagetable;
    uint64_t size;
    process_state state;
};

void process_init();
uint64_t schedule(uint64_t stack_pointer);

#endif