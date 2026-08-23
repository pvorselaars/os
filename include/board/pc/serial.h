#ifndef PC_SERIAL_H
#define PC_SERIAL_H

#include "kernel/base.h"

#define PC_SERIAL_PORT_0 0x3F8

result_t pc_serial_register(void);
void pc_serial_write_early(const char *buffer, size_t length);

#endif