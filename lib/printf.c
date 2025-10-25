#include "lib/printf.h"

/* Platform-agnostic printf implementation */

/* Function pointer for output method - can be set to different backends */
static printf_output_func_t output_func = NULL;

void printf_set_output(printf_output_func_t func)
{
	output_func = func;
}

printf_output_func_t printf_get_output(void)
{
	return output_func;
}

int printf(const char *format, ...)
{
	if (!output_func) {
		return -1; // No output function set
	}

	va_list args;
	char buffer[PRINTF_BUFFER_SIZE];
	
	va_start(args, format);
	int len = vsnprintf(buffer, PRINTF_BUFFER_SIZE, format, args);
	va_end(args);

	if (len > 0 && len < PRINTF_BUFFER_SIZE) {
		output_func(buffer, len);
	}

	return len;
}

int vprintf(const char *format, va_list args)
{
	if (!output_func) {
		return -1; // No output function set
	}

	char buffer[PRINTF_BUFFER_SIZE];
	int len = vsnprintf(buffer, PRINTF_BUFFER_SIZE, format, args);

	if (len > 0 && len < PRINTF_BUFFER_SIZE) {
		output_func(buffer, len);
	}

	return len;
}

int snprintf(char *str, uint64_t size, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int len = vsnprintf(str, size, format, args);
	va_end(args);
	return len;
}

/* Helper function to calculate string length */
static uint64_t printf_strlen(const char *str)
{
	uint64_t len = 0;
	while (*str++) len++;
	return len;
}

/* Convenience functions */
int puts(const char *str)
{
	if (!output_func) {
		return -1;
	}

	uint64_t len = printf_strlen(str);
	output_func(str, len);
	output_func("\n", 1); // puts adds newline

	return len + 1;
}

int putchar(int c)
{
	if (!output_func) {
		return -1;
	}

	char ch = (char)c;
	output_func(&ch, 1);
	return c;
}