#ifndef SERIAL_H
#define SERIAL_H

#include "arch/x86_64/io.h"
#include "lib/utils.h"

typedef enum {
    SERIAL_PORT_0 = 0x3F8,
} serial_port;

#define SERIAL_BUFFER_SIZE 8


void serial_init();
uint8_t serial_read();
void serial_write(serial_port, uint8_t);
void serial_output_func(const char *str, uint64_t len);

#endif