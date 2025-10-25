#include "lib/memory.h"
#include "kernel/process.h"
#include "arch/process.h"

#define MAX_PROCESSES 32

/* Platform-agnostic process structure */
struct process {
    process_state_t state;
    int pid;
    int priority;
    void *entry_point;
    void *user_data;
    arch_context_t *arch_context;
    struct process *next;  /* For linked list */
};

/* Process table and scheduler state */
static process_t process_table[MAX_PROCESSES];
static process_t *ready_queue = NULL;
static process_t *current_process = NULL;
static int next_pid = 1;

/* Internal functions */
static process_t *allocate_process(void);
static void free_process(process_t *proc);
static void add_to_ready_queue(process_t *proc);
static process_t *remove_from_ready_queue(void);
static process_t *find_next_ready_process(void);

void process_subsystem_init(void)
{
    /* Initialize process table */
    memory_zero(process_table, sizeof(process_table));
    
    ready_queue = NULL;
    current_process = NULL;
    next_pid = 1;
    
    /* Initialize architecture-specific components */
    arch_process_init();
}

process_t *process_create(const process_config_t *config)
{
    if (!config || !config->entry_point) {
        return NULL;
    }
    
    process_t *proc = allocate_process();
    if (!proc) {
        return NULL;  /* No free process slots */
    }
    
    /* Initialize process structure */
    proc->state = PROCESS_READY;
    proc->pid = next_pid++;
    proc->priority = config->priority;
    proc->entry_point = config->entry_point;
    proc->user_data = config->user_data;
    proc->next = NULL;
    
    /* Create architecture-specific context */
    proc->arch_context = arch_context_create(config->entry_point, config->stack_size);
    if (!proc->arch_context) {
        free_process(proc);
        return NULL;
    }
    
    /* Setup memory mapping for the process */
    if (arch_process_setup_memory(proc->arch_context, config->entry_point) != 0) {
        arch_context_destroy(proc->arch_context);
        free_process(proc);
        return NULL;
    }
    
    return proc;
}

void process_destroy(process_t *proc)
{
    if (!proc) return;
    
    /* Remove from ready queue if present */
    if (proc->state == PROCESS_READY) {
        /* TODO: Remove from ready queue */
    }
    
    /* Cleanup architecture-specific resources */
    if (proc->arch_context) {
        arch_process_cleanup_memory(proc->arch_context);
        arch_context_destroy(proc->arch_context);
    }
    
    free_process(proc);
}

int process_start(process_t *proc)
{
    if (!proc || proc->state != PROCESS_READY) {
        return -1;
    }
    
    add_to_ready_queue(proc);
    return 0;
}

int process_suspend(process_t *proc)
{
    if (!proc) return -1;
    
    if (proc->state == PROCESS_RUNNING) {
        proc->state = PROCESS_BLOCKED;
        process_yield();  /* Force context switch */
    } else if (proc->state == PROCESS_READY) {
        proc->state = PROCESS_BLOCKED;
        /* TODO: Remove from ready queue */
    }
    
    return 0;
}

int process_resume(process_t *proc)
{
    if (!proc || proc->state != PROCESS_BLOCKED) {
        return -1;
    }
    
    proc->state = PROCESS_READY;
    add_to_ready_queue(proc);
    return 0;
}

process_state_t process_get_state(process_t *proc)
{
    return proc ? proc->state : PROCESS_DONE;
}

process_t *process_get_current(void)
{
    return current_process;
}

void process_yield(void)
{
    process_schedule();
}

void process_schedule(void)
{
    process_t *old_process = current_process;
    process_t *new_process = find_next_ready_process();
    
    if (!new_process) {
        /* No ready processes, continue with current */
        return;
    }
    
    if (old_process) {
        if (old_process->state == PROCESS_RUNNING) {
            old_process->state = PROCESS_READY;
            add_to_ready_queue(old_process);
        }
    }
    
    new_process->state = PROCESS_RUNNING;
    current_process = new_process;
    
    /* Perform architecture-specific context switch */
    if (old_process && old_process->arch_context && new_process->arch_context) {
        uint64_t old_sp = arch_context_switch(old_process->arch_context, 
                                             new_process->arch_context);
        arch_context_set_sp(old_process->arch_context, old_sp);
    } else if (new_process->arch_context) {
        /* First time running this process */
        arch_process_start_user(new_process->arch_context);
    }
}

void process_exit(int exit_code)
{
    if (current_process) {
        current_process->state = PROCESS_DONE;
        /* TODO: Cleanup and remove process */
        process_schedule();  /* Switch to next process */
    }
}

/* Internal helper functions */

static process_t *allocate_process(void)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_DONE || 
            process_table[i].pid == 0) {
            return &process_table[i];
        }
    }
    return NULL;
}

static void free_process(process_t *proc)
{
    if (proc) {
        memory_zero_struct(proc);
    }
}

static void add_to_ready_queue(process_t *proc)
{
    if (!proc) return;
    
    proc->next = ready_queue;
    ready_queue = proc;
}

static process_t *remove_from_ready_queue(void)
{
    if (!ready_queue) return NULL;
    
    process_t *proc = ready_queue;
    ready_queue = ready_queue->next;
    proc->next = NULL;
    return proc;
}

static process_t *find_next_ready_process(void)
{
    return remove_from_ready_queue();
}