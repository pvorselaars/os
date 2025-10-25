#include "platform/pc/disk.h"

int read_blocks(unsigned long device, void* buffer, unsigned long start, unsigned short count)
{

	// TODO: check for supported functions with IDENTIFY command

	outb(0x1f6, 0x40);
	outb(0x1f2, count >> 8);
	outb(0x1f3, start >> 24);
	outb(0x1f4, start >> 32);
	outb(0x1f5, start >> 40);
	outb(0x1f2, count >> 8);
	outb(0x1f3, start);
	outb(0x1f4, start >> 8);
	outb(0x1f5, start >> 16);
	outb(0x1f7, 0x24);

	unsigned short *ptr = (unsigned short *) buffer;
	
	for (int block = 0; block < count; block++) {

		char c = inb(0x1f7);
		while (!(c & 0x08)) {
			c = inb(0x1f7);
		}

		for (int word = 0; word < 256; word++) {
			*ptr = inw(0x1f0);
			ptr++;
		}
	}

	return 0;
}

int write_blocks(unsigned long device, void* buffer, unsigned long start, unsigned short count)
{

	outb(0x1f6, 0x40);
	outb(0x1f2, count >> 8);
	outb(0x1f3, start >> 24);
	outb(0x1f4, start >> 32);
	outb(0x1f5, start >> 40);
	outb(0x1f2, count >> 8);
	outb(0x1f3, start);
	outb(0x1f4, start >> 8);
	outb(0x1f5, start >> 16);
	outb(0x1f7, 0x34);

	unsigned short *ptr = (unsigned short *) buffer;
	
	for (int block = 0; block < count; block++) {

		char c = inb(0x1f7);
		while (!(c & 0x08)) {
			c = inb(0x1f7);
		}

		for (int word = 0; word < 256; word++) {
			outw(0x1f0, *ptr);
			ptr++;
		}
	}

	return 0;
}
