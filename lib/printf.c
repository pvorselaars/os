#include "lib/printf.h"

#define FLAGS_ZERO      (1 << 0)
#define FLAGS_LEFT      (1 << 1)
#define FLAGS_PLUS      (1 << 2)
#define FLAGS_SPACE     (1 << 3)
#define FLAGS_HASH      (1 << 4)
#define FLAGS_UPPERCASE (1 << 5)
#define FLAGS_LONG      (1 << 6)
#define FLAGS_SHORT     (1 << 7)
#define FLAGS_CHAR      (1 << 8)

static bool is_digit(const char c)
{
	return (c >= '0' && c <= '9');
}

static int atoi(const char *str)
{
	int i = 0;

	while (is_digit(*str)) {
		i = i * 10 + *(str++) - '0';
	}

	return i;
}

static int ntoar(unsigned long value, char *str, unsigned int base)
{
	int length = 0;
	int n = 0;

	do {
		char digit = (char)(value % base);
		str[n++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
		value /= base;
	} while (value && n < 32);

	length = n;

	return length;
}

int vsnprintf(int8_t *str, unsigned int n, const int8_t *format, va_list args)
{
	unsigned int characters, length, width, base, flags;
	bool negative = false;

	length = 0;

	char buffer[64];

	while (*format != 0 && length < n - 1) {

		width = 0;
		negative = false;
		flags = 0;

		// Format specifier: %[flags][width][.precision][length]specifier
		if (*format != '%') {
			length++;
			*(str++) = *format++;
			continue;
		}
		// Flags
		format++;
		for (bool done = false; !done;) {
			switch (*format) {
			case '0':
				flags |= FLAGS_ZERO;
				format++;
				break;
			case '-':
				flags |= FLAGS_LEFT;
				format++;
				break;
			case '+':
				flags |= FLAGS_PLUS;
				format++;
				break;
			case ' ':
				flags |= FLAGS_SPACE;
				format++;
				break;
			case '#':
				flags |= FLAGS_HASH;
				format++;
				break;
			default:
				done = true;
				break;
			}
		}

		// Width
		if (is_digit(*format)) {
			width = atoi(format);
			while (is_digit(*format)) {
				format++;
			}
		} else if (*format == '*') {
			const int w = va_arg(args, int);
			if (w < 0) {
				flags |= FLAGS_LEFT;
				width = -w;
			} else {
				width = w;
			}
			format++;
		}
		// TODO: Precision

		// Length
		switch (*format) {
		case 'l':
			flags |= FLAGS_LONG;
			format++;
			break;

		default:
			break;
		}

		// Specifier

		switch (*format) {
		case 'i':
		case 'd':
			base = 10;

			if (flags & FLAGS_LONG) {
				const long value = va_arg(args, long);
				negative = value < 0;
				characters = ntoar((unsigned long)(negative ? 0 - value : value), buffer, base);
			} else {
				const int value = va_arg(args, int);
				negative = value < 0;
				characters = ntoar((unsigned long)(negative ? 0 - value : value), buffer, base);
			}

			break;
		case 'u':
			base = 10;

			if (flags & FLAGS_LONG) {
				const unsigned long value = va_arg(args, long);
				characters = ntoar((unsigned long)(value > 0 ? value : 0 - value), buffer, base);
			} else {
				const unsigned int value = va_arg(args, int);
				characters = ntoar((unsigned long)(value > 0 ? value : 0 - value), buffer, base);
			}

			break;

		case 'o':
			base = 8;

			if (flags & FLAGS_LONG) {
				const unsigned long value = va_arg(args, long);
				characters = ntoar((unsigned long)(value > 0 ? value : 0 - value), buffer, base);
			} else {
				const unsigned int value = va_arg(args, int);
				characters = ntoar((unsigned long)(value > 0 ? value : 0 - value), buffer, base);
			}

			break;

		case 'p':
		case 'x':
		case 'X':
			base = 16;

			if (flags & FLAGS_LONG) {
				const unsigned long value = va_arg(args, long);
				characters = ntoar((unsigned long)(value > 0 ? value : 0 - value), buffer, base);
			} else {
				const unsigned int value = va_arg(args, int);
				characters = ntoar((unsigned long)(value > 0 ? value : 0 - value), buffer, base);
			}

			break;

		case 'c':
			buffer[0] = (char)va_arg(args, int);
			characters = 1;

			break;

		case 's':
			const char *c = va_arg(args, const char *);
			characters = strlen(c);
			for (int i = characters - 1; i >= 0; i--) {
				buffer[i] = *(c++);
			}
			break;

			// TODO: f,F,e,E,g,G
		default:
			continue;
		}

		format++;

		// Left padding
		if ((flags & FLAGS_ZERO) && !(flags & FLAGS_LEFT)) {

			while (characters < width) {
				buffer[characters++] = '0';
			}

		}

		if (negative || (flags & FLAGS_PLUS)) {
			buffer[characters++] = negative ? '-' : '+';
		}

		if (!(flags & FLAGS_ZERO) && !(flags & FLAGS_LEFT)) {

			while (characters < width) {
				buffer[characters++] = ' ';
			}

		}

		for (int i = characters - 1; i >= 0; i--) {
			*(str++) = buffer[i];
		}

		// Right padding
		if (flags & FLAGS_LEFT) {
			while (characters < width) {
				*(str++) = ' ';
				characters++;
			}
		}

		length += characters;
	}

	return length;
}