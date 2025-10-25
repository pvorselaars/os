#include "platform/pc/vga_console.h"
#include "lib/memory.h"

/* VGA Console Driver - handles low-level VGA text mode operations */

static char *vga = (char *)virtual_address(0xb8000);
static int cursor = 0;

void vga_console_init()
{
	// Clear screen with white-on-black attribute (0x0f20)
	memory_set_word(vga, 0x0f20, 4000);
	cursor = 0;
	vga_update_cursor();
}

void vga_console_putchar(char c)
{
	switch (c) {
	case '\n':
		cursor = (cursor / 160 + 1) * 160;
		break;

	case '\t':
		cursor += 8;
		break;

	default:
		vga[cursor] = c;
		cursor += 2;
		break;
	}

	// Handle screen scrolling
	if (cursor > 4000) {
		vga_scroll_up();
	}

	vga_update_cursor();
}

void vga_console_write(const char *str, uint64_t len)
{
	for (uint64_t i = 0; i < len; i++) {
		vga_console_putchar(str[i]);
	}
}

void vga_console_puts(const char *str)
{
	while (*str) {
		vga_console_putchar(*str++);
	}
}

void vga_scroll_up()
{
	// Move all lines up by one
	memory_copy(vga, vga + 160, 3840);
	// Clear the bottom line
	memory_set_word(vga + 3840, 0x0f20, 160);
	cursor = 3840;
}

void vga_update_cursor()
{
	uint16_t pos = cursor / 2;
	
	// Send low byte
	outb(0x3d4, 0x0f);
	outb(0x3d5, pos & 0xff);
	
	// Send high byte
	outb(0x3d4, 0x0e);
	outb(0x3d5, (pos >> 8) & 0xff);
}

void vga_set_cursor(int x, int y)
{
	cursor = (y * 160) + (x * 2);
	vga_update_cursor();
}

void vga_get_cursor(int *x, int *y)
{
	*x = (cursor / 2) % 80;
	*y = (cursor / 2) / 80;
}

void vga_clear_screen()
{
	memory_set_word(vga, 0x0f20, 4000);
	cursor = 0;
	vga_update_cursor();
}

void vga_set_color(uint8_t fg, uint8_t bg)
{
	// This would require tracking current color and applying it to new characters
	// For now, we use the default 0x0f (white on black)
}