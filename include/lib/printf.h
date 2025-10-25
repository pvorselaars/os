#ifndef LIB_PRINTF_H
#define LIB_PRINTF_H

#include "kernel/definitions.h"
#include "lib/string.h"

#define PRINTF_BUFFER_SIZE 256

/* Function pointer type for output function */
typedef void (*printf_output_func_t)(const char *str, uint64_t len);

/* Set/get the output function for printf family */
void printf_set_output(printf_output_func_t func);
printf_output_func_t printf_get_output(void);

/* Standard printf functions */
int printf(const char *format, ...);
int vprintf(const char *format, va_list args);
int snprintf(char *str, uint64_t size, const char *format, ...);

/* Convenience functions */
int puts(const char *str);
int putchar(int c);

#endif