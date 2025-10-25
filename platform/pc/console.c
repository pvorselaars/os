#include "platform/pc/console.h"
#include "platform/pc/vga_console.h"
#include "lib/printf.h"

/* Console Bridge - connects printf system to VGA console */

/* Output function for printf - routes to VGA console */
static void console_output_func(const char *str, uint64_t len)
{
	vga_console_write(str, len);
}

void console_init()
{
	/* Initialize VGA console driver */
	vga_console_init();
	
	/* Set printf to use VGA console as output */
	printf_set_output(console_output_func);
}

/* Legacy compatibility functions */
void put(const char c)
{
	vga_console_putchar(c);
}

void print(const char *str)
{
	vga_console_puts(str);
}