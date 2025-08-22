#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "definitions.h"

#pragma pack(1)
typedef struct {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t ist;
	uint8_t flags;
	uint16_t offset_mid;
	uint32_t offset_high;
	uint32_t reserved;
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

void interrupt(uint32_t number);

#endif
