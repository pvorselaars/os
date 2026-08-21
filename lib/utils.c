#include "arch/cpu.h"
#include "lib/utils.h"
#include "lib/printf.h"
#include "kernel/time.h"

void fatal(const int8_t *format, ...)
{
	va_list args;
	va_start(args, format);

	int8_t buffer[PRINTF_BUFFER_SIZE];
	vsnprintf(buffer, PRINTF_BUFFER_SIZE, format, args);
	debug_printf("Fatal error: %s\n", buffer);

	va_end(args);

	arch_cpu_halt();
}


void sleep(uint64_t milliseconds)
{
	uint64_t start = time_now_ns();
	while (time_now_ns() - start < milliseconds * 1000000) {
		arch_cpu_idle();
	}
}