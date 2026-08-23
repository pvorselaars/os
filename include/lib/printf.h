#ifndef LIB_PRINTF_H
#define LIB_PRINTF_H

#include "kernel/base.h"

#define PRINTF_BUFFER_SIZE 256

int vsnprintf(int8_t *s, uint32_t n, const int8_t *format, va_list args);

#endif