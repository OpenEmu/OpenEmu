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

#ifndef PORT_DEFS_H
#define PORT_DEFS_H

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <new>

#define M_PI 3.14159265358979323846

#ifdef __cplusplus
 #include <ctype.h> // causes a link error when building c programs
 #define C_LINKAGE_BEGIN extern "C" {
 #define C_LINKAGE_END }
#else
 #define C_LINKAGE_BEGIN
 #define C_LINKAGE_END
#endif

C_LINKAGE_BEGIN

// for libFLAC
#undef fseeko
#undef ftello
#define fseeko fseek
#define ftello ftell

// overcome use of fprintf since bada/newlib (1.2) does not
// support stderr/stdout (undefined reference to `_impure_ptr').

void stderr_fprintf(void*, const char *format, ...);
void stderr_vfprintf(void*, const char *format, va_list ap);

#undef fprintf
#undef vfprintf
#undef stderr
#undef stdout
#undef stdin
#undef fputs
#undef fflush

#define stderr (void *)0
#define stdout (void *)1
#define stdin	 (void *)2
#define fputs(str, file)
#define fflush(file)
#define sscanf simple_sscanf
#define fprintf stderr_fprintf
#define vfprintf stderr_vfprintf

int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int simple_sscanf(const char *buffer, const char *format, ...);
char *strdup(const char *s1);
int vsprintf(char *str, const char *format, va_list ap);

C_LINKAGE_END

#endif
