/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

//
// simple sscanf replacement to match scummvm usage patterns
//

bool scanInt(const char **in, va_list *ap, int max) {
	// skip leading space characters
	while (**in && **in == ' ') {
		(*in)++;
	}

	// number optionally preceeded with a + or - sign.
	bool negate = false;
	if (**in == '-') {
		(*in)++;
		negate = true;
	}

	if (**in == '+') {
		(*in)++;
	}

	int *arg = va_arg(*ap, int*);
	char *end;
	long n = strtol(*in, &end, 10);
	if (negate) {
		n = -n;
	}

	bool err = false;
	if (end == *in || (max > 0 && (end - *in) > max)) {
		err = true;
	} else {
		*arg = (int)n;
		*in = end;
	}
	return err;
}

bool scanHex(const char **in, va_list *ap) {
	unsigned *arg = va_arg(*ap, unsigned*);
	char *end;
	long n = strtol(*in, &end, 16);
	if (end == *in) {
		return true;
	}

	*in = end;
	*arg = (unsigned) n;
	return false;
}

bool scanString(const char **in, va_list *ap) {
	char *arg = va_arg(*ap, char*);
	while (**in && **in != ' ' && **in != '\n' && **in != '\t') {
		*arg = **in;
		arg++;
		(*in)++;
	}
	*arg = '\0';
	(*in)++;
	return false;
}

bool scanStringUntil(const char **in, va_list *ap, char c_end) {
	char *arg = va_arg(*ap, char*);
	while (**in && **in != c_end) {
		*arg = **in;
		*arg++;
		(*in)++;
	}
	*arg = 0;
	(*in)++;
	return false;
}

bool scanChar(const char **in, va_list *ap) {
	char *arg = va_arg(*ap, char*);
	if (**in) {
		*arg = **in;
		(*in)++;
	}
	return false;
}

extern "C" int simple_sscanf(const char *input, const char *format, ...) {
	va_list ap;
	int result = 0;
	const char *next = input;

	va_start(ap, format);

	while (*format) {
		if (*format == '%') {
			format++;
			int max = 0;
			while (isdigit(*format)) {
				max = (max * 10) + (*format - '0');
				format++;
			}

			bool err = false;
			switch (*format++) {
			case 'c':
				err = scanChar(&next, &ap);
				break;
			case 'd':
			case 'u':
				err = scanInt(&next, &ap, max);
				break;
			case 'x':
				err = scanHex(&next, &ap);
				break;
			case 's':
				err = scanString(&next, &ap);
				break;
			case '[':
				// assume %[^c]
				if ('^' != *format) {
					err = true;
				} else {
					format++;
					if (*format && *(format+1) == ']') {
						err = scanStringUntil(&next, &ap, *format);
						format += 2;
					} else {
						err = true;
					}
				}
				break;
			default:
				err = true;
				break;
			}

			if (err) {
				break;
			} else {
				result++;
			}
		} else if (*format++ != *next++) {
			// match input
			break;
		}
	}

	va_end(ap);
	return result;
}

#if defined(TEST)
int main(int argc, char *pArgv[]) {
	int x,y,xx,yy,h;
	char buffer[100];
	unsigned u;
	char c;
	strcpy(buffer, "hello");
	char *b = buffer;

	if (simple_sscanf("BBX 00009 -1 +10 000",
										"BBX %d %d %d %d",
										&x, &y, &xx, &yy) != 4) {
		printf("Failed\n");
	} else {
		printf("Success %d %d %d %d\n", x, y, xx, yy);
	}

	if (simple_sscanf("CAT 123x-10 0x100h 123456.AUD $ ",
										"CAT %dx%d %xh %06u.AUD %c",
										&x, &y, &h, &u, &c) != 5) {
		printf("Failed\n");
	} else {
		printf("Success %d %d %d %d '%c' \n", x, y, h, u, c);
	}

	if (simple_sscanf("COPYRIGHT \"Copyright (c) 1984, 1987 Foo Systems Incorporated",
										"COPYRIGHT \"%[^\"]",
										b) != 1) {
		printf("Failed\n");
	} else {
		printf("Success %s\n", buffer);
	}

	return 0;
}
#endif
