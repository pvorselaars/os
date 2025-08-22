#ifndef IO_H
#define IO_H

unsigned char inb(uint32_t port);
unsigned short inw(uint32_t port);

void outb(uint32_t port, int32_t value);
void outw(uint32_t port, int32_t value);

#endif
