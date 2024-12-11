#include "console.h"
#include "memory.h"
#include "io.h"
#include "string.h"

static char *vga = (char *)0xb8000;

void console_init()
{
	memsetw(vga, 0x0f20, 4000);
}

void print(const char *str)
{
	static int cursor = 0;

	while (*str) {

		switch (*str) {
		case '\n':
			cursor = (cursor / 160 + 1) * 160;
			break;

		case '\t':
			cursor += 8;
			break;

		default:
			vga[cursor] = *str;
			cursor += 2;
			break;
		}

		// scroll screen up
		if (cursor > 4000) {
			memcpy(vga, vga + 160, 3840);
			memsetw(vga + 3840, 0x0f20, 160);
			cursor = 3840;
		} else {
			str++;
		}

	}

	// update cursor
	outb(0x3d4, 0x0f);
	outb(0x3d5, cursor / 2 & 0xff);
	outb(0x3d4, 0x0e);
	outb(0x3d5, (cursor / 2 >> 8) & 0xff);

}

int printf(const char *format, ...)
{
	va_list args;
	char buffer[128];
	va_start(args, format);

	int i = vsnprintf(buffer, 128, format, args);
	buffer[i] = 0;
	print(buffer);

	va_end(args);

	return i;
}
