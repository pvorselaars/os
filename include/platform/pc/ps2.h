#ifndef PS2_H
#define PS2_H

#include "kernel/definitions.h"
#include "arch/x86_64/io.h"

#define PS2_DATA_PORT       0x60
#define PS2_STATUS_PORT     0x64
#define PS2_COMMAND_PORT    0x64

void ps2_init();
void p2s_keyboard_interrupt();

#endif