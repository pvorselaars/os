#ifndef UTILS_H
#define UTILS_H

#include "definitions.h"
#include "platform/timer.h"
#include "platform/init.h"

void fatal(const int8_t *format, ...);
void sleep(uint64_t ms);

#define assert(e)                                              \
    if (!(e))                                                  \
    {                                                          \
        fatal("%s:%d assertion failed in %s\n", __FILE__, __LINE__, __func__); \
    };
#define ALIGN_UP(x, size) (((x) + (size) - 1) & ~((size) - 1))
#define ALIGN_DOWN(x, size) ((x) & ~((size) - 1))
#define IS_ALIGNED(x, size) (((x) & ((size) - 1)) == 0)

#endif