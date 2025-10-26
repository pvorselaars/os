#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

#include "definitions.h"

/* Utility macros */
#define ALIGN_UP(x, size)   (((x) + (size) - 1) & ~((size) - 1))
#define ALIGN_DOWN(x, size) ((x) & ~((size) - 1))
#define IS_ALIGNED(x, size) (((x) & ((size) - 1)) == 0)

#endif