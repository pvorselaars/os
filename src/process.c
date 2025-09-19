#include "process.h"

static process process_table[2];

extern void process_start(uint64_t memory, uint64_t stack);

void process_1(){
    while (1);
}

void process_2(){
    while (1);
}

void schedule() {
    disable_interrupts();
    halt();
}

void process_init() {

    process *p1 = &process_table[0];

    p1->parent = NULL;
    p1->state = RUNNING;
    p1->memory = &process_1;
    p1->size = 0x100; 

    process_start((uint64_t)p1->memory, (uint64_t)p1->memory + p1->size);

}