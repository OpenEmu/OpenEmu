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

#include <FApp.h>
#include <FGraphics.h>
#include <FUi.h>
#include <FSystem.h>
#include <FBase.h>

#include "backends/platform/bada/portdefs.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define BUF_SIZE 255

void systemError(const char *message);

C_LINKAGE_BEGIN

int __errno; // for overridden method in saves/default/default-saves.cpp

void __assert_func(const char *file, int line,
									 const char *func, const char *err) {
	char buffer[BUF_SIZE];
	snprintf(buffer, sizeof(buffer), "%s %d %s %s", file, line, func, err);
	systemError(buffer);
}

void stderr_fprintf(void*, const char *format, ...) {
	va_list ap;
	char buffer[BUF_SIZE];

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);

	AppLog(buffer);
}

void stderr_vfprintf(void*, const char *format, va_list ap) {
	char buffer[BUF_SIZE];
	vsnprintf(buffer, sizeof(buffer), format, ap);
	AppLog(buffer);
}

int printf(const char *format, ...) {
	int result = 0;
	va_list ap;
	char buffer[BUF_SIZE];

	va_start(ap, format);
	result = vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);

	AppLog(buffer);

	return result;
}

int sprintf(char *str, const char *format, ...) {
	va_list ap;
	int result;
	char buffer[BUF_SIZE];

	va_start(ap, format);
	result = vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);

	strcpy(str, buffer);

	return result;
}

char *strdup(const char *strSource) {
	char *buffer;
	int len = strlen(strSource) + 1;
	buffer = (char *)malloc(len);
	if (buffer) {
		memcpy(buffer, strSource, len);
	}
	return buffer;
}

int vsprintf(char *str, const char *format, va_list ap) {
	char buffer[BUF_SIZE];
	int result = vsnprintf(buffer, sizeof(buffer), format, ap);
	strcpy(str, buffer);
	return result;
}

C_LINKAGE_END
