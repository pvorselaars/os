#ifndef STRING_H
#define STRING_H

#include "definitions.h"
#include "memory.h"

#define FLAGS_ZERO      (1 << 0)
#define FLAGS_LEFT      (1 << 1)
#define FLAGS_PLUS      (1 << 2)
#define FLAGS_SPACE     (1 << 3)
#define FLAGS_HASH      (1 << 4)
#define FLAGS_UPPERCASE (1 << 5)
#define FLAGS_LONG      (1 << 6)
#define FLAGS_SHORT     (1 << 7)
#define FLAGS_CHAR      (1 << 8)

int vsnprintf(int8_t *s, uint32_t n, const int8_t *format, va_list arg);

#endif
