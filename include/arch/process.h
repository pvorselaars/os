#ifndef ARCH_PROCESS_H
#define ARCH_PROCESS_H

#include "kernel/definitions.h"
#include "kernel/process.h"

/* Architecture-specific process context management
 * 
 * These functions handle low-level context switching, register
 * management, and architecture-specific process setup.
 */

/* Opaque architecture-specific context */
typedef struct arch_context arch_context_t;

/* Initialize architecture-specific process subsystem */
void arch_process_init(void);

/* Create architecture-specific process context */
arch_context_t *arch_context_create(void *entry_point, uint64_t stack_size);

/* Destroy architecture-specific process context */
void arch_context_destroy(arch_context_t *context);

/* Perform context switch from old to new context
 * Returns the stack pointer of the old context for saving */
uint64_t arch_context_switch(arch_context_t *old_ctx, arch_context_t *new_ctx);

/* Start a process in user mode (first time execution) */
void arch_process_start_user(arch_context_t *context);

/* Get/set context stack pointer (for scheduler) */
uint64_t arch_context_get_sp(arch_context_t *context);
void arch_context_set_sp(arch_context_t *context, uint64_t sp);

/* Architecture-specific memory management for processes */
int arch_process_setup_memory(arch_context_t *context, void *entry_point);
void arch_process_cleanup_memory(arch_context_t *context);

#endif