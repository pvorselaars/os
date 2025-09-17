#include "process.h"

static process *process_table;

void process_init() {

    process_table = (process *)alloc();
    if (!process_table) {
        fatal("Failed to allocate memory for process table");
    }
}