#ifndef ARCH_PROCESS_H
#define ARCH_PROCESS_H

#include "kernel/base.h"

typedef struct arch_process_context arch_process_context_t;

arch_process_context_t *arch_process_context_create(uint64_t entry);

void arch_process_switch(arch_process_context_t *from, arch_process_context_t *to);
void arch_process_start(arch_process_context_t *context);

#endif
