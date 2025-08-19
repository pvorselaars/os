#ifndef DISK_H
#define DISK_H

int read_blocks(unsigned long device, void* buffer, unsigned long start, unsigned short count);
int write_blocks(unsigned long device, void* buffer, unsigned long start, unsigned short count);

#endif
