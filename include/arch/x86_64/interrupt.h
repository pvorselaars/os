#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "definitions.h"

void x86_64_interrupt_init();
result_t arch_register_interrupt(unsigned int vector, void (*handler)());

#endif
