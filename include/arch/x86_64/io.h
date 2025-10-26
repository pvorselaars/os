#ifndef IO_H
#define IO_H

#include "definitions.h"

uint8_t inb(uint32_t port);
uint16_t inw(uint32_t port);

void outb(uint32_t port, uint8_t value);
void outw(uint32_t port, uint16_t value);

#endif
