#include "arch/arch.h"
#include "board/pc/serial.h"
#include "lib/printf.h"
#include <stdarg.h>

void arch_debug_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    arch_debug_vprintf(format, args);
    va_end(args);
}

void arch_debug_vprintf(const char *format, va_list args)
{
    char buffer[256];
    int len = vsnprintf(buffer, sizeof(buffer), format, args);

    for (int i = 0; i < len && buffer[i]; i++) {
        while (!(inb(SERIAL_PORT_0 + 5) & 0x40));
        outb(SERIAL_PORT_0, (uint8_t)buffer[i]);
    }
}