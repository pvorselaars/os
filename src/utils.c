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