#include "platform/console.h"
#include "serial.h"

int platform_console_init(void)
{
    serial_init();
    return 0;
}

void platform_console_putc(char c)
{
    serial_write(SERIAL_PORT_0, (uint8_t)c);
}

void platform_console_write(const char *buf, unsigned len)
{
    for (unsigned i = 0; i < len; ++i)
        platform_console_putc(buf[i]);
}
