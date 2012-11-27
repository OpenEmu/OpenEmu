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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef WINCE_PORTDEFS_H
#define WINCE_PORTDEFS_H

#ifndef _WIN32_WCE
#error For use on WinCE only
#endif

// Missing string/stdlib/assert declarations for WinCE 2.xx
#if _WIN32_WCE < 300

	#define SMALL_SCREEN_DEVICE

	void *calloc(size_t n, size_t s);
	int isalnum(int c);
	int isdigit(int c);
	int isprint(int c);
	int isspace(int c);
	char *strrchr(const char *s, int c);
	char *strdup(const char *s);
	void assert(void *expression);
	void assert(int expression);
	long int strtol(const char *nptr, char **endptr, int base);
	char *_strdup(const char *s);
	char *strpbrk(const char *s, const char *accept);

#endif


#ifndef __GNUC__
	void *bsearch(const void *, const void *, size_t, size_t, int (*x)(const void *, const void *));
	char *getcwd(char *buf, int size);
	typedef int ptrdiff_t;
	void GetCurrentDirectory(int len, char *buf);
	#define INVALID_FILE_ATTRIBUTES 0xffffffff
#else
	#include <math.h>
	#undef GetCurrentDirectory
	extern "C" void GetCurrentDirectory(int len, char *buf);
	#define snprintf _snprintf
	#define strdup _strdup
	#define fopen wce_fopen
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <stdarg.h>
#include <assert.h>
#include <mmsystem.h>
#include <ctype.h>
//#include <direct.h>
#include <new>

#ifdef __MINGW32CE__
	void *bsearch(const void *, const void *, size_t, size_t, int (*x)(const void *, const void *));
#endif
int remove(const char *path);
int _access(const char *path, int mode);

void drawError(char *);

#define vsnprintf _vsnprintf


#endif
