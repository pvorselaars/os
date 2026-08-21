#include "board/pc/serial.h"
#include "lib/printf.h"

void debug_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[256];
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length > 0)
        pc_serial_write_early(buffer, (size_t)length);
}