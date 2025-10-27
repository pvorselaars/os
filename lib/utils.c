#include "arch/arch.h"
#include "lib/utils.h"
#include "lib/string.h"
#include "lib/printf.h"

void fatal(const int8_t *format, ...)
{
	va_list args;
	va_start(args, format);

	arch_debug_printf("Fatal error: ");
	arch_debug_vprintf(format, args);
	arch_debug_printf("\n");

	va_end(args);

	arch_halt();
}


void sleep(uint64_t milliseconds)
{
	uint64_t start = arch_time_ns();
	while (arch_time_ns() - start < milliseconds * 1000000ULL) {
		arch_halt();
	}
}