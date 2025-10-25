#include "../include/lib/utils.h"
#include "../include/lib/string.h"
#include "../include/lib/printf.h"

void fatal(const int8_t *format, ...)
{
	va_list args;
	va_start(args, format);

	printf(format, args);

	va_end(args);

	platform_halt();
}


void sleep(uint64_t milliseconds)
{
	uint64_t start = platform_time_ns();
	while (platform_time_ns() - start < milliseconds * 1000000)
		platform_halt();
}