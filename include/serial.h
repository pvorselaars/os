#ifndef SERIAL_H
#define SERIAL_H

#include "io.h"
#include "utils.h"

typedef enum {
    SERIAL_PORT_0 = 0x3F8,
} serial_ports;

void serial_init();
uint8_t serial_read(serial_ports);
void serial_receive();

#endif