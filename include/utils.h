#ifndef UTILS_H
#define UTILS_H

#include "defs.h"
#include "arg.h"
#include "console.h"
#include "interrupt.h"
#include "io.h"


#include "definitions.h"
#include "console.h"
#include "interrupt.h"
#include "io.h"

void fatal(const int8_t *format, ...);
void sleep(uint64_t ms);

#define assert(e) if (!(e)) { fatal("%s:%u assertion failed\n",__FILE__, __LINE__); };


#endif
