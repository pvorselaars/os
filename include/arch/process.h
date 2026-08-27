#ifndef ARCH_PROCESS_H
#define ARCH_PROCESS_H

#include "arch/memory.h"
#include "kernel/base.h"

#define MAX_PROCESS_COUNT 2

typedef struct arch_process_context arch_process_context_t;

arch_process_context_t* arch_process_context_create(uint32_t id, uint64_t entry);

void arch_process_activate(uint32_t id);
void arch_process_start(arch_process_context_t *context);
void arch_process_save_context(const arch_process_context_t *from, arch_process_context_t *to);

#endif
