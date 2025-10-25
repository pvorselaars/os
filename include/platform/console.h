#ifndef PLATFORM_CONSOLE_H
#define PLATFORM_CONSOLE_H

#include "definitions.h"

/* Minimal console (serial) primitives the kernel core uses. Platform
   implementations provide the backing (PC COM1, RPi PL011, etc.). */

int platform_console_init(void);
void platform_console_putc(char c);
void platform_console_write(const char *buf, unsigned len);

#endif
