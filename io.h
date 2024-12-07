#ifndef IO_H
#define IO_H

unsigned char inb(unsigned char port);
unsigned short inw(unsigned char port);

void outb(int port, unsigned char value);
void outw(int port, unsigned short value);

#endif
