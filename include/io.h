#ifndef IO_H
#define IO_H

unsigned char inb(int port);
unsigned short inw(int port);

void outb(int port, int value);
void outw(int port, int value);

#endif
