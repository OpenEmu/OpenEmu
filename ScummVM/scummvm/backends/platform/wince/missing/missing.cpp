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

/* Original code:
 * Implementation for standard and semi-standard C library calls missing in WinCE
 * environment.
 * by Vasyl Tsvirkunov
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common/debug.h"

char *strdup(const char *strSource);

#ifdef __GNUC__
#define EXT_C extern "C"
#else
#define EXT_C
#endif

// common missing functions required by both gcc and evc

void *bsearch(const void *key, const void *base, size_t nmemb,
              size_t size, int (*compar)(const void *, const void *)) {
	// Perform binary search
	size_t lo = 0;
	size_t hi = nmemb;
	while (lo < hi) {
		size_t mid = (lo + hi) / 2;
		const void *p = ((const char *)base) + mid * size;
		int tmp = (*compar)(key, p);
		if (tmp < 0)
			hi = mid;
		else if (tmp > 0)
			lo = mid + 1;
		else
			return const_cast<void *>(p);
	}

	return NULL;
}

static char cwd[MAX_PATH + 1] = "";

EXT_C char *getcwd(char *buffer, int maxlen) {
	TCHAR fileUnc[MAX_PATH + 1];
	char *plast;

	if (cwd[0] == 0) {
		GetModuleFileName(NULL, fileUnc, MAX_PATH);
		WideCharToMultiByte(CP_ACP, 0, fileUnc, -1, cwd, MAX_PATH, NULL, NULL);
		plast = strrchr(cwd, '\\');
		if (plast)
			*plast = 0;
		/* Special trick to keep start menu clean... */
		if (_stricmp(cwd, "\\windows\\start menu") == 0)
			strcpy(cwd, "\\Apps");
	}
	if (buffer)
		strncpy(buffer, cwd, maxlen);
	return cwd;
}

#ifdef __GNUC__
#undef GetCurrentDirectory
#endif
EXT_C void GetCurrentDirectory(int len, char *buf) {
	getcwd(buf, len);
}

/*
Windows CE fopen has non-standard behavior -- not
fully qualified paths refer to root folder rather
than current folder (concept not implemented in CE).
*/
#undef fopen
EXT_C FILE *wce_fopen(const char *fname, const char *fmode) {
	char fullname[MAX_PATH + 1];

	if (!fname || fname[0] == '\0')
		return NULL;
	if (fname[0] != '\\' && fname[0] != '/') {
		getcwd(fullname, MAX_PATH);
		strcat(fullname, "\\");
		strcat(fullname, fname);
		return fopen(fullname, fmode);
	} else
		return fopen(fname, fmode);
}

/* Remove file by name */
int remove(const char *path) {
	TCHAR pathUnc[MAX_PATH + 1];
	MultiByteToWideChar(CP_ACP, 0, path, -1, pathUnc, MAX_PATH);
	return !DeleteFile(pathUnc);
}


/* check out file access permissions */
int _access(const char *path, int mode) {
	TCHAR fname[MAX_PATH];
	char fullname[MAX_PATH + 1];

	if (path[0] != '\\' && path[0] != '/') {
		getcwd(fullname, MAX_PATH);
		strcat(fullname, "\\");
		strcat(fullname, path);
		MultiByteToWideChar(CP_ACP, 0, fullname, -1, fname, sizeof(fname) / sizeof(TCHAR));
	} else
		MultiByteToWideChar(CP_ACP, 0, path, -1, fname, sizeof(fname) / sizeof(TCHAR));

	WIN32_FIND_DATA ffd;
	HANDLE h = FindFirstFile(fname, &ffd);
	FindClose(h);

	if (h == INVALID_HANDLE_VALUE) {
		// WORKAROUND: WinCE 3.0 doesn't find paths ending in '\'
		if (path[strlen(path) - 1] == '\\') {
			char p2[MAX_PATH];
			strncpy(p2, path, strlen(path) - 1);
			p2[strlen(path) - 1] = '\0';
			return _access(p2, mode);
		} else
			return -1;  //Can't find file
	}

	if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		// WORKAROUND: WinCE (or the emulator) sometimes returns bogus directory
		// hits for files that don't exist. TRIPLE checking for the same fname
		// seems to weed out those false positives.
		// Exhibited in kyra engine.
		h = FindFirstFile(fname, &ffd);
		FindClose(h);
		if (h == INVALID_HANDLE_VALUE)
			return -1;  //Can't find file
		h = FindFirstFile(fname, &ffd);
		FindClose(h);
		if (h == INVALID_HANDLE_VALUE)
			return -1;  //Can't find file

		return 0; //Always return success if target is directory and exists
	}
	switch (mode) {
	case 00: //Check existence
		return 0;
	case 06: //Check Read & Write permission
	case 02: //Check Write permission
		return ffd.dwFileAttributes & FILE_ATTRIBUTE_READONLY ? -1 : 0;
	case 04: //Check Read permission
		return 0; //Assume always have read permission
	}
	//Bad mode value supplied, return failure
	return -1;
}

// evc only functions follow
#ifndef __GNUC__

char *strdup(const char *strSource) {
	char *buffer;
	size_z len = strlen(strSource) + 1;
	buffer = (char *)malloc(len);
	if (buffer)
		memcpy(buffer, strSource, len);
	return buffer;
}

// gcc build only functions follow
#else // defined(__GNUC__)

#ifndef __MINGW32CE__
int islower(int c) {
	return (c >= 'a' && c <= 'z');
}

int isspace(int c) {
	return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v');
}

int isalpha(int c) {
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

int isalnum(int c) {
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
}

int isprint(int c) {
	//static const char punct[] = "!\"#%&'();<=>?[\\]*+,-./:^_{|}~";
	//return (isalnum(c) || strchr(punct, c));
	return (32 <= c && c <= 126);   // based on BSD manpage
}
#endif

#endif
