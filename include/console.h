#ifndef CONSOLE_H
#define CONSOLE_H

#include "definitions.h"

void console_init();
void print(const int8_t *str);
int printf(const int8_t *format, ...);
int vprintf(const int8_t *format, va_list args);

#endif
