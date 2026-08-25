#ifndef PROCESS_H
#define PROCESS_H
#include "arch/process.h"

void process_create(void (*entry)());

arch_process_context_t *process_schedule(arch_process_context_t *current);
void process_scheduler_start();

#endif
