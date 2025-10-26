#ifndef X86_64_SERIAL_H
#define X86_64_SERIAL_H
#include "arch/x86_64/io.h"
#include "lib/utils.h"

typedef enum {
    SERIAL_PORT_0 = 0x3F8,
} serial_port;

#define SERIAL_BUFFER_SIZE 8

#endif