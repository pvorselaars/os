#ifndef LIB_PRINTF_H
#define LIB_PRINTF_H

#include "definitions.h"
#include "lib/string.h"

#define PRINTF_BUFFER_SIZE 256

int vsnprintf(int8_t *s, uint32_t n, const int8_t *format, va_list args);

#endif