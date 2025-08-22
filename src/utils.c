#include "utils.h"

void fatal(const int8_t *format, ...)
{
	va_list args;
	va_start(args, format);

	printf("FATAL: ");
	vprintf(format, args);

	va_end(args);

	while (1);
}


void sleep(uint64_t milliseconds)
{
	uint64_t start = ticks;
	while (ticks - start < milliseconds)
		halt();
}