#ifndef VGA_CONSOLE_H
#define VGA_CONSOLE_H

#include "kernel/definitions.h"
#include "arch/x86_64/memory.h"
#include "arch/x86_64/io.h"

/* VGA Console Driver Interface */

/* Initialize VGA text mode console */
void vga_console_init(void);

/* Write a single character to the console */
void vga_console_putchar(char c);

/* Write a buffer of characters to the console */
void vga_console_write(const char *str, uint64_t len);

/* Write a null-terminated string to the console */
void vga_console_puts(const char *str);

/* Scroll the screen up by one line */
void vga_scroll_up(void);

/* Update hardware cursor position */
void vga_update_cursor(void);

/* Set cursor position (x, y) */
void vga_set_cursor(int x, int y);

/* Get current cursor position */
void vga_get_cursor(int *x, int *y);

/* Clear the entire screen */
void vga_clear_screen(void);

/* Set text colors (foreground/background) */
void vga_set_color(uint8_t fg, uint8_t bg);

#endif