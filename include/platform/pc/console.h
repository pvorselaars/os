#ifndef CONSOLE_H
#define CONSOLE_H

#include "definitions.h"
#include "memory.h"
#include "io.h"
#include "string.h"

void console_init();
void print(const int8_t *str);
void put(const int8_t c);
int printf(const int8_t *format, ...);
int vprintf(const int8_t *format, va_list args);

#endif
