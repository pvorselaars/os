#ifndef X86_64_PIT_H
#define X86_64_PIT_H

#include "definitions.h"

void x86_64_pit_init(unsigned int frequency_hz);
void x86_64_pit_disable(void);
uint16_t x86_64_pit_read_count(void);

#endif