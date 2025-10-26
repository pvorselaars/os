#include "lib/string.h"

uint64_t strlen(const char *str)
{
	uint64_t length = 0;
	while (*str++ != 0) {
		length++;
	}

	return length;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return *(unsigned char *)s1 - *(unsigned char *)s2;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	size_t i;
	
	for (i = 0; i < n && src[i] != '\0'; i++) {
		dest[i] = src[i];
	}
	
	for (; i < n; i++) {
		dest[i] = '\0';
	}
	
	return dest;
}
