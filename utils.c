#include "utils.h"

#include "arg.h"
#include "console.h"

void fatal(const char *format, ...)
{
	va_list args;
	va_start(args, format);

	printf("FATAL: ");
	vprintf(format, args);

	va_end(args);

	while (1) ;
}

void examine(void* ptr, unsigned long bytes)
{

	unsigned char *mem = (unsigned char*)ptr;

	for (int i = 0; i < bytes; i++) {
		printf("%02x ", *mem++);
	}
	printf("\n");
}
