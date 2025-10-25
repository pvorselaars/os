#ifndef CONSOLE_H
#define CONSOLE_H

#include "kernel/definitions.h"
#include "platform/pc/vga_console.h"
#include "lib/printf.h"

/* Console Interface - High-level console operations */

/* Initialize console system (VGA + printf) */
void console_init(void);

/* Legacy compatibility functions */
void print(const char *str);
void put(const char c);

/* Printf functions are now provided by lib/printf.h */
/* Include lib/printf.h to use printf(), vprintf(), etc. */

#endif
