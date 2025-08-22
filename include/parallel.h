#ifndef PARALLEL_H
#define PARALLEL_H

#include "definitions.h"
#include "utils.h"

typedef enum {
    PARALLEL_PORT_0 = 0x378,
} parallel_port;

void parallel_write(parallel_port, uint8_t);

#endif 