#include "process.h"

static process process_table[2];

extern void process_start(process *p);

void process_1()
{
    while (1) {
        // process 1
    }
}

void process_2()
{
    while (1) {
        // process 2
    }
}

uint64_t schedule(uint64_t stack_pointer) {
    process *old, *new;

    if (process_table[0].state == RUNNING) {
        old = &process_table[0];
        new = &process_table[1];
    } else {
        old = &process_table[1];
        new = &process_table[0];
    }

    old->state = READY;
    old->stack_pointer = stack_pointer;

    new->state = RUNNING;
    memory_map_userpages(new->pagetable);

    interrupt_set_stack_pointer(new->stack_base + PAGE_SIZE);

    return new->stack_pointer;
}

void process_init()
{
    process *p2 = &process_table[1];

    uint64_t p2_address = (uint64_t)physical_address(&process_2);
    uint64_t p2_page = ALIGN_DOWN(p2_address, PAGE_SIZE);
    uint64_t p2_entry = p2_address - p2_page;
    uint64_t p2_pagetable = memory_map(0, p2_page, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

    void *p2_kernel_stack = memory_allocate();
    assert(p2_kernel_stack);


    p2->state = READY;
    p2->stack_base = (uint64_t)virtual_address(p2_kernel_stack);

    uint64_t *sp = (uint64_t*)(p2->stack_base + PAGE_SIZE);

    // iretq frame
    *(--sp) = 0x43;      // DS
    *(--sp) = PAGE_SIZE; // stack top
    *(--sp) = 0x202;     // RFLAGS
    *(--sp) = 0x3b;      // CS
    *(--sp) = p2_entry;

    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = 0;
    *(--sp) = p2->stack_base;

    p2->stack_pointer = (uint64_t)sp;
    p2->pagetable = p2_pagetable;
    p2->entry = (void*)p2_entry;
    p2->size = PAGE_SIZE;

    memory_map_userpages(0);

    process *p1 = &process_table[0];

    uint64_t p1_address = (uint64_t)physical_address(&process_1);
    uint64_t p1_page = ALIGN_DOWN(p1_address, PAGE_SIZE);
    uint64_t p1_entry = p1_address - p1_page;
    uint64_t p1_pagetable = memory_map(0, p1_page, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

    void *p1_kernel_stack = memory_allocate();
    assert(p1_kernel_stack);

    p1->state = RUNNING;
    p1->stack_base = (uint64_t)virtual_address(p1_kernel_stack);
    p1->stack_pointer = p1->stack_base + PAGE_SIZE;
    p1->pagetable = p1_pagetable;
    p1->entry = (void*)p1_entry;
    p1->size = PAGE_SIZE;

    interrupt_set_stack_pointer(p1->stack_base + PAGE_SIZE);

    process_start(p1);
}