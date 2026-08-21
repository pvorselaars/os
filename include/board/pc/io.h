#ifndef PC_IO_H
#define PC_IO_H

#include "kernel/base.h"

uint8_t pc_io_read8(uint16_t address);
void pc_io_write8(uint16_t address, uint8_t value);
uint16_t pc_io_read16(uint16_t address);
void pc_io_write16(uint16_t address, uint16_t value);

#define arch_io_read8 pc_io_read8
#define arch_io_write8 pc_io_write8
#define arch_io_read16 pc_io_read16
#define arch_io_write16 pc_io_write16

#endif
