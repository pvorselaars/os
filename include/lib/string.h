#ifndef STRING_H
#define STRING_H

#include "kernel/base.h"

uint64_t strlen(const char *str);
int strcmp(const char *s1, const char *s2);
char *strncpy(char *dest, const char *src, size_t n);

#endif
