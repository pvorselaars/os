#ifndef IO_H
#define IO_H

#include "definitions.h"

unsigned char inb(uint32_t port);
unsigned short inw(uint32_t port);

void outb(uint32_t port, uint8_t value);
void outw(uint32_t port, uint16_t value);

#endif
