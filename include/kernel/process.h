#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H

#include "kernel/definitions.h"

/* Platform-agnostic process management interface */

typedef enum {
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_DONE
} process_state_t;

/* Opaque process handle - platform-specific implementation */
typedef struct process process_t;

/* Process configuration for creation */
typedef struct {
    void *entry_point;          /* Entry point function */
    uint64_t stack_size;       /* Stack size in bytes */
    int priority;               /* Process priority (platform-specific) */
    void *user_data;            /* Optional user data pointer */
} process_config_t;

/* Process management API */

/* Initialize the process subsystem */
void process_subsystem_init(void);

/* Create a new process */
process_t *process_create(const process_config_t *config);

/* Destroy a process */
void process_destroy(process_t *proc);

/* Start a process (transition to READY state) */
int process_start(process_t *proc);

/* Suspend a process */
int process_suspend(process_t *proc);

/* Resume a process */
int process_resume(process_t *proc);

/* Get process state */
process_state_t process_get_state(process_t *proc);

/* Get current running process */
process_t *process_get_current(void);

/* Yield CPU to scheduler */
void process_yield(void);

/* Process scheduler - called by timer interrupt */
void process_schedule(void);

/* Exit current process */
void process_exit(int exit_code);

#endif