#ifndef PROCESS_H
#define PROCESS_H

#include "definitions.h"
#include "memory.h"

typedef struct {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint64_t rflags;
} process_context;

typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    DONE
} process_state;

typedef struct process process;

struct process {
    uint64_t pid;
    process *parent;
    process_state state;
    process_context context;
    void *memory;
    uint64_t size;
};

void process_init();

#endif