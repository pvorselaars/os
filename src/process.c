#include "process.h"

static process process_table[2];

extern void process_start(uint64_t memory, uint64_t stack);

void process_1()
{
    while (1) {
        printf("Process 1 running\n");
        sleep(1000);
    }
}

void process_2()
{
    while (1) {
        printf("Process 2 running\n");
        sleep(1000);
    }
}

void schedule()
{
    disable_interrupts();
    halt();
}

void process_init()
{

    uint64_t a = (uint64_t)&process_1;
    uint64_t p = ALIGN(a, PAGE_SIZE);
    uint64_t entry = a - p;

    print_pagetable_entries(0x0);
    memory_map(0x0, p, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);
    print_pagetable_entries(0x0);

    process *p1 = &process_table[0];

    p1->parent = NULL;
    p1->state = RUNNING;
    p1->memory = (void *)(entry);
    p1->size = 0x100;

    process_start(entry, p1->size);
}