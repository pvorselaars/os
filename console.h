#ifndef CONSOLE_H
#define CONSOLE_H

#include "arg.h"

void console_init();
void print(const char *str);
int printf(const char *format, ...);
int vprintf(const char *format, va_list args);

#endif
