#ifndef ARCH_PROCESS_H
#define ARCH_PROCESS_H

#include "arch/memory.h"
#include "kernel/base.h"

#define MAX_PROCESS_COUNT 2

typedef struct arch_process_context arch_process_context_t;

arch_process_context_t* arch_process_context_create(uint64_t entry, const arch_memory_address_space_t* space);

void arch_process_start(arch_process_context_t *context);
void arch_process_save_context(const arch_process_context_t *from, arch_process_context_t *to);

#endif
