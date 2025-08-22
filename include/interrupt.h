#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "defs.h"
#include "pit.h"

#pragma pack(1)
typedef struct {
	unsigned short offset_low;
	unsigned short selector;
	unsigned char ist;
	unsigned char flags;
	unsigned short offset_mid;
	unsigned int offset_high;
	unsigned int reserved;
} interrupt_descriptor;
#pragma pack()

typedef enum {
	INTERRUPT_GATE = 0xe,
	TRAP_GATE = 0xf
} gate_type;

typedef enum {
	KERNEL = 0,
	USER = 3
} privilege_level;

void interrupt_init();

void enable_interrupts();
void disable_interrupts();

void interrupt(unsigned int num);

void halt();

extern uint64_t ticks;

#endif
