#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "definitions.h"

void x86_64_interrupt_init();
result_t x86_64_register_interrupt(unsigned int vector, void (*handler)());

#endif
