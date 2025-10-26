#ifndef STRING_H
#define STRING_H

#include "definitions.h"
#include "arch/x86_64/memory.h"

uint64_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
char *strncpy(char *dest, const char *src, size_t n);

#endif
