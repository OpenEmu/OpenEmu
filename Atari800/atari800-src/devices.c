/*
 * devices.c - emulation of H:, P:, E: and K: Atari devices
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2010 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#ifdef HAVE_UNIXIO_H
/* VMS */
#include <unixio.h>
#endif
#ifdef HAVE_FILE_H
/* VMS */
#include <file.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_DIRECT_H
/* WIN32 */
#include <direct.h> /* mkdir, rmdir */
#endif
/* XXX: <sys/dir.h>, <ndir.h>, <sys/ndir.h> */
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include "atari.h"
#include "binload.h"
#include "cpu.h"
#include "devices.h"
#include "esc.h"
#include "log.h"
#include "memory.h"
#include "sio.h"
#include "util.h"
#ifdef R_IO_DEVICE
#include "rdevice.h"
#endif
#ifdef __PLUS
#include "misc_win.h"
#endif

#ifndef S_IREAD
#define S_IREAD S_IRUSR
#endif
#ifndef S_IWRITE
#define S_IWRITE S_IWUSR
#endif

#ifdef HAVE_WINDOWS_H

#include <windows.h>

#undef FILENAME_CONV
#undef FILENAME

#ifdef UNICODE
#define FILENAME_CONV \
	WCHAR wfilename[FILENAME_MAX]; \
	if (MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, FILENAME_MAX) <= 0) \
		return FALSE;
#define FILENAME wfilename
#else /* UNICODE */
#define FILENAME_CONV
#define FILENAME filename
#endif /* UNICODE */

#endif /* HAVE_WINDOWS_H */


/* Read Directory abstraction layer -------------------------------------- */

#ifdef HAVE_WINDOWS_H

static char dir_path[FILENAME_MAX];
static WIN32_FIND_DATA wfd;
static HANDLE dh = INVALID_HANDLE_VALUE;

static int Devices_OpenDir(const char *filename)
{
	FILENAME_CONV;
	Util_splitpath(filename, dir_path, NULL);
	if (dh != INVALID_HANDLE_VALUE)
		FindClose(dh);
	dh = FindFirstFile(FILENAME, &wfd);
	if (dh == INVALID_HANDLE_VALUE) {
		/* don't raise error if the path is ok but no file matches:
		   Win98 returns ERROR_FILE_NOT_FOUND,
		   WinCE returns ERROR_NO_MORE_FILES */
		DWORD err = GetLastError();
		if (err != ERROR_FILE_NOT_FOUND && err != ERROR_NO_MORE_FILES)
			return FALSE;
	}
	return TRUE;
}

static int Devices_ReadDir(char *fullpath, char *filename, int *isdir,
                          int *readonly, int *size, char *timetext)
{
#ifdef UNICODE
	char afilename[MAX_PATH];
#endif
	if (dh == INVALID_HANDLE_VALUE)
		return FALSE;
	/* don't match "." nor ".."  */
	while (wfd.cFileName[0] == '.' &&
	       (wfd.cFileName[1] == '\0' || (wfd.cFileName[1] == '.' && wfd.cFileName[2] == '\0'))
	) {
		if (!FindNextFile(dh, &wfd)) {
			FindClose(dh);
			dh = INVALID_HANDLE_VALUE;
			return FALSE;
		}
	}
#ifdef UNICODE
	if (WideCharToMultiByte(CP_ACP, 0, wfd.cFileName, -1, afilename, MAX_PATH, NULL, NULL) <= 0)
		strcpy(afilename, "?ERROR");
#define FOUND_FILENAME afilename
#else
#define FOUND_FILENAME wfd.cFileName
#endif /* UNICODE */
	if (filename != NULL)
		strcpy(filename, FOUND_FILENAME);
	if (fullpath != NULL)
		Util_catpath(fullpath, dir_path, FOUND_FILENAME);
	if (isdir != NULL)
		*isdir = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
	if (readonly != NULL)
		*readonly = (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? TRUE : FALSE;
	if (size != NULL)
		*size = (int) wfd.nFileSizeLow;
	if (timetext != NULL) {
		FILETIME lt;
		SYSTEMTIME st;
		if (FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &lt) != 0
		 && FileTimeToSystemTime(&lt, &st) != 0) {
			int hour = st.wHour;
			char ampm = 'a';
			if (hour >= 12) {
				hour -= 12;
				ampm = 'p';
			}
			if (hour == 0)
				hour = 12;
			sprintf(timetext, "%2d-%02d-%02d %2d:%02d%c",
				st.wMonth, st.wDay, st.wYear % 100, hour, st.wMinute, ampm);
		}
		else
			strcpy(timetext, " 1-01-01 12:00p");
	}

	if (!FindNextFile(dh, &wfd)) {
		FindClose(dh);
		dh = INVALID_HANDLE_VALUE;
	}
	return TRUE;
}

#define DO_DIR

#elif defined(HAVE_OPENDIR)

static int match(const char *pattern, const char *filename)
{
	if (strcmp(pattern, "*.*") == 0)
		return TRUE;

	for (;;) {
		switch (*pattern) {
		case '\0':
			return (*filename == '\0');
		case '?':
			if (*filename == '\0' || *filename == '.')
				return FALSE;
			pattern++;
			filename++;
			break;
		case '*':
			if (Util_chrieq(*filename, pattern[1]))
				pattern++;
			else if (*filename == '\0')
				return FALSE; /* because pattern[1] != '\0' */
			else
				filename++;
			break;
		default:
			if (!Util_chrieq(*pattern, *filename))
				return FALSE;
			pattern++;
			filename++;
			break;
		}
	}
}

static char dir_path[FILENAME_MAX];
static char filename_pattern[FILENAME_MAX];
static DIR *dp = NULL;

static int Devices_OpenDir(const char *filename)
{
	Util_splitpath(filename, dir_path, filename_pattern);
	if (dp != NULL)
		closedir(dp);
	dp = opendir(dir_path);
	return dp != NULL;
}

static int Devices_ReadDir(char *fullpath, char *filename, int *isdir,
                          int *readonly, int *size, char *timetext)
{
	struct dirent *entry;
	char temppath[FILENAME_MAX];
#ifdef HAVE_STAT
	struct stat status;
#endif
	for (;;) {
		entry = readdir(dp);
		if (entry == NULL) {
			closedir(dp);
			dp = NULL;
			return FALSE;
		}
		if (entry->d_name[0] == '.') {
			/* don't match Unix hidden files unless specifically requested */
			if (filename_pattern[0] != '.')
				continue;
			/* never match "." */
			if (entry->d_name[1] == '\0')
				continue;
			/* never match ".." */
			if (entry->d_name[1] == '.' && entry->d_name[2] == '\0')
				continue;
		}
		if (match(filename_pattern, entry->d_name))
			break;
	}
	if (filename != NULL)
		strcpy(filename, entry->d_name);
	Util_catpath(temppath, dir_path, entry->d_name);
	if (fullpath != NULL)
		strcpy(fullpath, temppath);
#ifdef HAVE_STAT
	if (stat(temppath, &status) == 0) {
		if (isdir != NULL)
			*isdir = (status.st_mode & S_IFDIR) ? TRUE : FALSE;
		if (readonly != NULL)
			*readonly = (status.st_mode & S_IWRITE) ? FALSE : TRUE;
		if (size != NULL)
			*size = (int) status.st_size;
		if (timetext != NULL) {
#ifdef HAVE_LOCALTIME
			struct tm *ft;
			int hour;
			char ampm = 'a';
			ft = localtime(&status.st_mtime);
			hour = ft->tm_hour;
			if (hour >= 12) {
				hour -= 12;
				ampm = 'p';
			}
			if (hour == 0)
				hour = 12;
			sprintf(timetext, "%2d-%02d-%02d %2d:%02d%c",
				ft->tm_mon + 1, ft->tm_mday, ft->tm_year % 100,
				hour, ft->tm_min, ampm);
#else
			strcpy(timetext, " 1-01-01 12:00p");
#endif /* HAVE_LOCALTIME */
		}
	}
	else
#endif /* HAVE_STAT */
	{
		if (isdir != NULL)
			*isdir = FALSE;
		if (readonly != NULL)
			*readonly = FALSE;
		if (size != NULL)
			*size = 0;
		if (timetext != NULL)
			strcpy(timetext, " 1-01-01 12:00p");
	}
	return TRUE;
}

#define DO_DIR

#elif defined(PS2)

extern char dir_path[FILENAME_MAX];

int Atari_OpenDir(const char *filename);

#define Devices_OpenDir Atari_OpenDir

int Atari_ReadDir(char *fullpath, char *filename, int *isdir,
                  int *readonly, int *size, char *timetext);

static int Devices_ReadDir(char *fullpath, char *filename, int *isdir,
                          int *readonly, int *size, char *timetext)
{
	char tmp_filename[FILENAME_MAX];
	if (filename == NULL)
		filename = tmp_filename;
	do {
		if (!Atari_ReadDir(fullpath, filename, isdir, readonly, size, timetext))
			return FALSE;
		/* reject "." and ".." */
	} while (filename[0] == '.' &&
	         (filename[1] == '\0' || (filename[1] == '.' && filename[2] == '\0')));
	return TRUE;
}

#define DO_DIR

#endif /* defined(PS2) */


/* Rename File/Directory abstraction layer ------------------------------- */

#ifdef HAVE_WINDOWS_H

static int Devices_Rename(const char *oldname, const char *newname)
{
#ifdef UNICODE
	WCHAR woldname[FILENAME_MAX];
	WCHAR wnewname[FILENAME_MAX];
	if (MultiByteToWideChar(CP_ACP, 0, oldname, -1, woldname, FILENAME_MAX) <= 0
	 || MultiByteToWideChar(CP_ACP, 0, newname, -1, wnewname, FILENAME_MAX) <= 0)
		return FALSE;
	return MoveFile(woldname, wnewname) != 0;
#else
	return MoveFile(oldname, newname) != 0;
#endif /* UNICODE */
}

#define DO_RENAME

#elif defined(HAVE_RENAME)

static int Devices_Rename(const char *oldname, const char *newname)
{
	return rename(oldname, newname) == 0;
}

#define DO_RENAME

#endif


/* Set/Reset Read-Only Attribute abstraction layer ----------------------- */

#ifdef HAVE_WINDOWS_H

/* Enables/disables read-only mode for the file. Returns TRUE on success. */
static int Devices_SetReadOnly(const char *filename, int readonly)
{
	DWORD attr;
	FILENAME_CONV;
	attr = GetFileAttributes(FILENAME);
	if (attr == 0xffffffff)
		return FALSE;
	return SetFileAttributes(FILENAME, readonly
		? (attr | FILE_ATTRIBUTE_READONLY)
		: (attr & ~FILE_ATTRIBUTE_READONLY)) != 0;
}

#define DO_LOCK

#elif defined(HAVE_CHMOD)

static int Devices_SetReadOnly(const char *filename, int readonly)
{
	return chmod(filename, readonly ? S_IREAD : (S_IREAD | S_IWRITE)) == 0;
}

#define DO_LOCK

#endif /* defined(HAVE_CHMOD) */


/* Make Directory abstraction layer -------------------------------------- */

#ifdef HAVE_WINDOWS_H

static int Devices_MakeDirectory(const char *filename)
{
	FILENAME_CONV;
	return CreateDirectory(FILENAME, NULL) != 0;
}

#define DO_MKDIR

#elif defined(HAVE_MKDIR)

static int Devices_MakeDirectory(const char *filename)
{
	return mkdir(filename
#ifndef MKDIR_TAKES_ONE_ARG
		, 0777
#endif
		) == 0;
}

#define DO_MKDIR

#endif /* defined(HAVE_MKDIR) */


/* Remove Directory abstraction layer ------------------------------------ */

#ifdef HAVE_WINDOWS_H

static UBYTE Devices_RemoveDirectory(const char *filename)
{
	FILENAME_CONV;
	if (RemoveDirectory(FILENAME) != 0)
		return 1;
	return (UBYTE) ((HRESULT_CODE(GetLastError()) == ERROR_DIR_NOT_EMPTY) ? 167 : 150);
}

#define DO_RMDIR

#elif defined(HAVE_RMDIR)

static UBYTE Devices_RemoveDirectory(const char *filename)
{
	if (rmdir(filename) == 0)
		return 1;
	return (UBYTE) ((errno == ENOTEMPTY) ? 167 : 150);
}

#define DO_RMDIR

#endif /* defined(HAVE_RMDIR) */


/* H: device emulation --------------------------------------------------- */

#define DEFAULT_H_PATH  "H1:>DOS;>DOS"

/* emulator debugging mode */
static int devbug = FALSE;

/* host path for each H: unit */
char Devices_atari_h_dir[4][FILENAME_MAX] = { "", "", "", "" };

/* read only mode for H: device */
int Devices_h_read_only = TRUE;

/* ';'-separated list of Atari paths checked by the "load executable"
   command. if a path does not start with "Hn:", then the selected device
   is used. */
char Devices_h_exe_path[FILENAME_MAX] = DEFAULT_H_PATH;

/* Devices_h_current_dir must be empty or terminated with Util_DIR_SEP_CHAR;
   only Util_DIR_SEP_CHAR can be used as a directory separator here */
char Devices_h_current_dir[4][FILENAME_MAX];

/* stream open via H: device per IOCB */
static FILE *h_fp[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

/* H: text mode per IOCB */
static int h_textmode[8];

/* last read character was CR, per IOCB */
static int h_wascr[8];

/* last operation: 'o': open, 'r': read, 'w': write, per IOCB */
/* (this is needed to apply fseek(fp, 0, SEEK_CUR) between reads and writes
   in update (12) mode) */
static char h_lastop[8];

Util_tmpbufdef(static, h_tmpbuf[8])

/* IOCB #, 0-7 */
static int h_iocb;

/* H: device number, 0-3 */
static int h_devnum;

/* filename as specified after "Hn:" */
static char atari_filename[FILENAME_MAX];

#ifdef DO_RENAME
/* new filename (no directories!) */
static char new_filename[FILENAME_MAX];
#endif

/* atari_filename applied to H:'s current dir, with Util_DIR_SEP_CHARs only */
static char atari_path[FILENAME_MAX];

/* full filename for the current operation */
static char host_path[FILENAME_MAX];

int Devices_H_CountOpen(void)
{
	int r = 0;
	int i;
	for (i = 0; i < 8; i++)
		if (h_fp[i] != NULL)
			r++;
	return r;
}

void Devices_H_CloseAll(void)
{
	int i;
	for (i = 0; i < 8; i++)
		if (h_fp[i] != NULL) {
			Util_fclose(h_fp[i], h_tmpbuf[i]);
			h_fp[i] = NULL;
		}
}

static void Devices_H_Init(void)
{
	if (devbug)
		Log_print("HHINIT");
	Devices_h_current_dir[0][0] = '\0';
	Devices_h_current_dir[1][0] = '\0';
	Devices_h_current_dir[2][0] = '\0';
	Devices_h_current_dir[3][0] = '\0';
	Devices_H_CloseAll();
}

int Devices_Initialise(int *argc, char *argv[])
{
	int i;
	int j;
	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */

		if (strcmp(argv[i], "-H1") == 0) {
			if (i_a)
				Util_strlcpy(Devices_atari_h_dir[0], argv[++i], FILENAME_MAX);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-H2") == 0) {
			if (i_a)
				Util_strlcpy(Devices_atari_h_dir[1], argv[++i], FILENAME_MAX);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-H3") == 0) {
			if (i_a)
				Util_strlcpy(Devices_atari_h_dir[2], argv[++i], FILENAME_MAX);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-H4") == 0) {
			if (i_a)
				Util_strlcpy(Devices_atari_h_dir[3], argv[++i], FILENAME_MAX);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-Hpath") == 0) {
			if (i_a)
				Util_strlcpy(Devices_h_exe_path, argv[++i], FILENAME_MAX);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-hreadonly") == 0)
			Devices_h_read_only = TRUE;
		else if (strcmp(argv[i], "-hreadwrite") == 0)
			Devices_h_read_only = FALSE;
		else if (strcmp(argv[i], "-devbug") == 0)
			devbug = TRUE;
		else {
			if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-H1 <path>       Set path for H1: device");
				Log_print("\t-H2 <path>       Set path for H2: device");
				Log_print("\t-H3 <path>       Set path for H3: device");
				Log_print("\t-H4 <path>       Set path for H4: device");
				Log_print("\t-Hpath <path>    Set path for Atari executables on the H: device");
				Log_print("\t-hreadonly       Enable read-only mode for H: device");
				Log_print("\t-hreadwrite      Disable read-only mode for H: device");
				Log_print("\t-devbug          Debugging messages for H: and P: devices");
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			return FALSE;
		}
	}
	*argc = j;
	Devices_H_Init();

	return TRUE;
}

#define IS_DIR_SEP(c) ((c) == '/' || (c) == '\\' || (c) == ':' || (c) == '>')

static int Devices_IsValidForFilename(char ch)
{
	if ((ch >= 'A' && ch <= 'Z')
	 || (ch >= 'a' && ch <= 'z')
	 || (ch >= '0' && ch <= '9'))
		return TRUE;
	switch (ch) {
	case '!':
	case '#':
	case '$':
	case '&':
	case '\'':
	case '(':
	case ')':
	case '*':
	case '-':
	case '.':
	case '?':
	case '@':
	case '_':
		return TRUE;
	default:
		return FALSE;
	}
}

UWORD Devices_SkipDeviceName(void)
{
	UWORD bufadr;
	for (bufadr = MEMORY_dGetWordAligned(Devices_ICBALZ); ; bufadr++) {
		char c = (char) MEMORY_dGetByte(bufadr);
		if (c == ':')
			return (UWORD) (bufadr + 1);
		if (c < '!' || c > '\x7e')
			return 0;
	}
}

/* devnum must be 0-3; p must point inside atari_filename */
static UWORD Devices_GetAtariPath(int devnum, char *p)
{
	UWORD bufadr = Devices_SkipDeviceName();
	if (bufadr != 0) {
		while (p < atari_filename + sizeof(atari_filename) - 1) {
			char c = (char) MEMORY_dGetByte(bufadr);
			if (Devices_IsValidForFilename(c) || IS_DIR_SEP(c) || c == '<') {
				*p++ = c;
				bufadr++;
			}
			else {
				/* end of filename */
				/* now apply it to Devices_h_current_dir */
				const char *q = atari_filename;
				*p = '\0';
				if (IS_DIR_SEP(*q)) {
					/* absolute path on H: device */
					q++;
					p = atari_path;
				}
				else {
					strcpy(atari_path, Devices_h_current_dir[devnum]);
					p = atari_path + strlen(atari_path);
				}
				for (;;) {
					/* we are here at the beginning of a path element,
					   i.e. at the beginning of atari_path or after Util_DIR_SEP_CHAR */
					if (*q == '<'
					 || (*q == '.' && q[1] == '.' && (q[2] == '\0' || IS_DIR_SEP(q[2])))) {
						/* "<" or "..": parent directory */
						if (p == atari_path) {
							CPU_regY = 150; /* Sparta: directory not found */
							CPU_SetN;
							return 0;
						}
						do
							p--;
						while (p > atari_path && p[-1] != Util_DIR_SEP_CHAR);
						if (*q == '.') {
							if (q[2] != '\0')
								q++;
							q++;
						}
						q++;
						continue;
					}
					if (IS_DIR_SEP(*q)) {
						/* duplicate DIR_SEP */
						CPU_regY = 165; /* bad filename */
						CPU_SetN;
						return 0;
					}
					do {
						if (p >= atari_path + sizeof(atari_path) - 1) {
							CPU_regY = 165; /* bad filename */
							CPU_SetN;
							return 0;
						}
						*p++ = *q;
						if (*q == '\0')
							return bufadr;
						q++;
					} while (!IS_DIR_SEP(*q));
					*p++ = Util_DIR_SEP_CHAR;
					q++;
				}
			}
		}
	}
	CPU_regY = 165; /* bad filename */
	CPU_SetN;
	return 0;
}

static int Devices_GetIOCB(void)
{
	if ((CPU_regX & 0x8f) != 0) {
		CPU_regY = 134; /* invalid IOCB number */
		CPU_SetN;
		return FALSE;
	}
	h_iocb = CPU_regX >> 4;
	return TRUE;
}

static int Devices_GetNumber(int set_textmode)
{
	int devnum;
	if (!Devices_GetIOCB())
		return -1;
	devnum = MEMORY_dGetByte(Devices_ICDNOZ);
	if (devnum > 9 || devnum == 0 || devnum == 5) {
		CPU_regY = 160; /* invalid unit/drive number */
		CPU_SetN;
		return -1;
	}
	if (devnum < 5) {
		if (set_textmode)
			h_textmode[h_iocb] = FALSE;
		return devnum - 1;
	}
	if (set_textmode)
		h_textmode[h_iocb] = TRUE;
	return devnum - 6;
}

static UWORD Devices_GetHostPath(int set_textmode)
{
	UWORD bufadr;
	h_devnum = Devices_GetNumber(set_textmode);
	if (h_devnum < 0)
		return 0;
	bufadr = Devices_GetAtariPath(h_devnum, atari_filename);
	if (bufadr == 0)
		return 0;
	Util_catpath(host_path, Devices_atari_h_dir[h_devnum], atari_path);
	return bufadr;
}

static void Devices_H_Open(void)
{
	FILE *fp;
	UBYTE aux1;
#ifdef DO_DIR
	UBYTE aux2;
	char entryname[FILENAME_MAX];
	int isdir;
	int readonly;
	int size;
	char timetext[16];
#endif

	if (devbug)
		Log_print("HHOPEN");

	if (Devices_GetHostPath(TRUE) == 0)
		return;

	if (h_fp[h_iocb] != NULL)
		Util_fclose(h_fp[h_iocb], h_tmpbuf[h_iocb]);

#if 0
	if (devbug)
		Log_print("atari_filename=\"%s\", atari_path=\"%s\" host_path=\"%s\"", atari_filename, atari_path, host_path);
#endif

	fp = NULL;
	h_wascr[h_iocb] = FALSE;
	h_lastop[h_iocb] = 'o';

	aux1 = MEMORY_dGetByte(Devices_ICAX1Z);
	switch (aux1) {
	case 4:
		/* don't bother using "r" for textmode:
		   we want to support LF, CR/LF and CR, not only native EOLs */
		fp = Util_fopen(host_path, "rb", h_tmpbuf[h_iocb]);
		if (fp != NULL) {
			CPU_regY = 1;
			CPU_ClrN;
		}
		else {
			CPU_regY = 170; /* file not found */
			CPU_SetN;
		}
		break;
#ifdef DO_DIR
	case 6:
	case 7:
		fp = Util_tmpopen(h_tmpbuf[h_iocb]);
		if (fp == NULL) {
			CPU_regY = 144; /* device done error */
			CPU_SetN;
			break;
		}
		if (!Devices_OpenDir(host_path)) {
			Util_fclose(fp, h_tmpbuf[h_iocb]);
			fp = NULL;
			CPU_regY = 144; /* device done error */
			CPU_SetN;
			break;
		}
		aux2 = MEMORY_dGetByte(Devices_ICAX2Z);
		if (aux2 >= 128) {
			fprintf(fp, "\nVolume:    HDISK%c\nDirectory: ", '1' + h_devnum);
			/* if (strcmp(dir_path, Devices_atari_h_dir[h_devnum]) == 0) */
			if (strchr(atari_path, Util_DIR_SEP_CHAR) == NULL)
				fprintf(fp, "MAIN\n\n");
			else {
				char end_dir_str[FILENAME_MAX];
				Util_splitpath(dir_path, NULL, end_dir_str);
				fprintf(fp, "%s\n\n", /* Util_strupper */(end_dir_str));
			}
		}

		while (Devices_ReadDir(NULL, entryname, &isdir, &readonly, &size,
		                      (aux2 >= 128) ? timetext : NULL)) {
			char *ext;
			/* Util_strupper(entryname); */
			ext = strrchr(entryname, '.');
			if (ext == NULL)
				ext = "";
			else {
				/* replace the dot with NUL,
				   so entryname is without extension */
				*ext++ = '\0';
				if (ext[0] != '\0' && ext[1] != '\0' && ext[2] != '\0' && ext[3] != '\0') {
					ext[2] = '+';
					ext[3] = '\0';
				}
			}
			if (strlen(entryname) > 8) {
				entryname[7] = '+';
				entryname[8] = '\0';
			}
			if (aux2 >= 128) {
				if (isdir)
					fprintf(fp, "%-13s<DIR>  %s\n", entryname, timetext);
				else {
					if (size > 999999)
						size = 999999;
					fprintf(fp, "%-9s%-3s %6d %s\n", entryname, ext, size, timetext);
				}
			}
			else {
				char dirchar = ' ';
				size = (size + 255) >> 8;
				if (size > 999)
					size = 999;
				if (isdir) {
					if (MEMORY_dGetByte(0x700) == 'M') /* MyDOS */
						dirchar = ':';
					else /* Sparta */
						ext = "\304\311\322"; /* "DIR" with bit 7 set */
				}
				fprintf(fp, "%c%c%-8s%-3s %03d\n", readonly ? '*' : ' ',
				        dirchar, entryname, ext, size);
			}
		}

		if (aux2 >= 128)
			fprintf(fp, "   999 FREE SECTORS\n");
		else
			fprintf(fp, "999 FREE SECTORS\n");

		Util_rewind(fp);
		h_textmode[h_iocb] = TRUE;
		CPU_regY = 1;
		CPU_ClrN;
		break;
#endif /* DO_DIR */
	case 8: /* write: "w" */
	case 9: /* write at end of file (append): "a" */
	case 12: /* write and read (update): "r+" || "w+" */
	case 13: /* append and read: "a+" */
		if (Devices_h_read_only) {
			CPU_regY = 163; /* disk write-protected */
			CPU_SetN;
			break;
		}
		{
			char mode[4];
			char *p = mode + 1;
			mode[0] = (aux1 & 1) ? 'a' : (aux1 < 12) ? 'w' : 'r';
			if (!h_textmode[h_iocb])
				*p++ = 'b';
			if (aux1 >= 12)
				*p++ = '+';
			*p = '\0';
			fp = Util_fopen(host_path, mode, h_tmpbuf[h_iocb]);
			if (fp == NULL && aux1 == 12) {
				mode[0] = 'w';
				fp = Util_fopen(host_path, mode, h_tmpbuf[h_iocb]);
			}
		}
		if (fp != NULL) {
			CPU_regY = 1;
			CPU_ClrN;
		}
		else {
			CPU_regY = 144; /* device done error */
			CPU_SetN;
		}
		break;
	default:
		CPU_regY = 168; /* invalid device command */
		CPU_SetN;
		break;
	}
	h_fp[h_iocb] = fp;
}

static void Devices_H_Close(void)
{
	if (devbug)
		Log_print("HHCLOS");
	if (!Devices_GetIOCB())
		return;
	if (h_fp[h_iocb] != NULL) {
		Util_fclose(h_fp[h_iocb], h_tmpbuf[h_iocb]);
		h_fp[h_iocb] = NULL;
	}
	CPU_regY = 1;
	CPU_ClrN;
}

static void Devices_H_Read(void)
{
	if (devbug)
		Log_print("HHREAD");
	if (!Devices_GetIOCB())
		return;
	if (h_fp[h_iocb] != NULL) {
		int ch;
		if (h_lastop[h_iocb] == 'w')
			fseek(h_fp[h_iocb], 0, SEEK_CUR);
		h_lastop[h_iocb] = 'r';
		ch = fgetc(h_fp[h_iocb]);
		if (ch != EOF) {
			if (h_textmode[h_iocb]) {
				switch (ch) {
				case 0x0d:
					h_wascr[h_iocb] = TRUE;
					ch = 0x9b;
					break;
				case 0x0a:
					if (h_wascr[h_iocb]) {
						/* ignore LF next to CR */
						ch = fgetc(h_fp[h_iocb]);
						if (ch != EOF) {
							if (ch == 0x0d) {
								h_wascr[h_iocb] = TRUE;
								ch = 0x9b;
							}
							else
								h_wascr[h_iocb] = FALSE;
						}
						else {
							CPU_regY = 136; /* end of file */
							CPU_SetN;
							break;
						}
					}
					else
						ch = 0x9b;
					break;
				default:
					h_wascr[h_iocb] = FALSE;
					break;
				}
			}
			CPU_regA = (UBYTE) ch;
			CPU_regY = 1;
			CPU_ClrN;
		}
		else {
			CPU_regY = 136; /* end of file */
			CPU_SetN;
		}
	}
	else {
		CPU_regY = 136; /* end of file; XXX: this seems to be what Atari DOSes return */
		CPU_SetN;
	}
}

static void Devices_H_Write(void)
{
	if (devbug)
		Log_print("HHWRIT");
	if (!Devices_GetIOCB())
		return;
	if (h_fp[h_iocb] != NULL) {
		int ch;
		if (h_lastop[h_iocb] == 'r')
			fseek(h_fp[h_iocb], 0, SEEK_CUR);
		h_lastop[h_iocb] = 'w';
		ch = CPU_regA;
		if (ch == 0x9b && h_textmode[h_iocb])
			ch = '\n';
		fputc(ch, h_fp[h_iocb]);
		CPU_regY = 1;
		CPU_ClrN;
	}
	else {
		CPU_regY = 135; /* attempted to write to a read-only device */
		            /* XXX: this seems to be what Atari DOSes return */
		CPU_SetN;
	}
}

static void Devices_H_Status(void)
{
	if (devbug)
		Log_print("HHSTAT");

	CPU_regY = 146; /* function not implemented in handler; XXX: check file existence? */
	CPU_SetN;
}

#define CHECK_READ_ONLY \
	if (Devices_h_read_only) { \
		CPU_regY = 163; \
		CPU_SetN; \
		return; \
	}

#ifdef DO_RENAME

static void fillin(const char *pattern, char *filename)
{
	const char *filename_end = filename + strlen(filename);
	for (;;) {
		switch (*pattern) {
		case '\0':
			*filename = '\0';
			return;
		case '?':
			pattern++;
			if (filename < filename_end)
				filename++;
			break;
		case '*':
			if (filename >= filename_end || *filename == pattern[1])
				pattern++;
			else
				filename++;
			break;
		default:
			*filename++ = *pattern++;
			break;
		}
	}
}

static void Devices_H_Rename(void)
{
	UWORD bufadr;
	char c;
	char *p;
	int num_changed = 0;
	int num_failed = 0;
	int num_locked = 0;
	int readonly = FALSE;

	if (devbug)
		Log_print("RENAME Command");
	CHECK_READ_ONLY;

	bufadr = Devices_GetHostPath(FALSE);
	if (bufadr == 0)
		return;
	/* skip space between filenames */
	for (;;) {
		c = (char) MEMORY_dGetByte(bufadr);
		if (Devices_IsValidForFilename(c))
			break;
		if (c == '\0' || (UBYTE) c > 0x80 || IS_DIR_SEP(c)) {
			CPU_regY = 165; /* bad filename */
			CPU_SetN;
			return;
		}
		bufadr++;
	}
	/* get new filename */
	p = new_filename;
	do {
		if (p >= new_filename + sizeof(new_filename) - 1) {
			CPU_regY = 165; /* bad filename */
			CPU_SetN;
			return;
		}
		*p++ = c;
		bufadr++;
		c = (char) MEMORY_dGetByte(bufadr);
	} while (Devices_IsValidForFilename(c));
	*p = '\0';

#ifdef DO_DIR
	if (!Devices_OpenDir(host_path)) {
		CPU_regY = 170; /* file not found */
		CPU_SetN;
		return;
	}
	while (Devices_ReadDir(host_path, NULL, NULL, &readonly, NULL, NULL))
#endif /* DO_DIR */
	{
		/* Check file write permission to mimic Atari
		   permission system: read-only ("locked") file
		   cannot be renamed. */
		if (readonly)
			num_locked++;
		else {
			char new_dirpart[FILENAME_MAX];
			char new_filepart[FILENAME_MAX];
			char new_path[FILENAME_MAX];
			/* split old filepath into dir part and file part */
			Util_splitpath(host_path, new_dirpart, new_filepart);
			/* replace old file part with new file part */
			fillin(new_filename, new_filepart);
			/* combine new filepath */
			Util_catpath(new_path, new_dirpart, new_filepart);
			if (Devices_Rename(host_path, new_path))
				num_changed++;
			else
				num_failed++;
		}
	}

	if (devbug)
		Log_print("%d renamed, %d failed, %d locked",
		       num_changed, num_failed, num_locked);

	if (num_locked) {
		CPU_regY = 167; /* file locked */
		CPU_SetN;
	}
	else if (num_failed != 0 || num_changed == 0) {
		CPU_regY = 170; /* file not found */
		CPU_SetN;
	}
	else {
		CPU_regY = 1;
		CPU_ClrN;
	}
}

#endif /* DO_RENAME */

#ifdef HAVE_UTIL_UNLINK

static void Devices_H_Delete(void)
{
	int num_deleted = 0;
	int num_failed = 0;
	int num_locked = 0;
	int readonly = FALSE;

	if (devbug)
		Log_print("DELETE Command");
	CHECK_READ_ONLY;

	if (Devices_GetHostPath(FALSE) == 0)
		return;

#ifdef DO_DIR
	if (!Devices_OpenDir(host_path)) {
		CPU_regY = 170; /* file not found */
		CPU_SetN;
		return;
	}
	while (Devices_ReadDir(host_path, NULL, NULL, &readonly, NULL, NULL))
#endif /* DO_DIR */
	{
		/* Check file write permission to mimic Atari
		   permission system: read-only ("locked") file
		   cannot be deleted. Modern systems have
		   a different permission for file deletion. */
		if (readonly)
			num_locked++;
		else
			if (Util_unlink(host_path) == 0)
				num_deleted++;
			else
				num_failed++;
	}

	if (devbug)
		Log_print("%d deleted, %d failed, %d locked",
		       num_deleted, num_failed, num_locked);

	if (num_locked) {
		CPU_regY = 167; /* file locked */
		CPU_SetN;
	}
	else if (num_failed != 0 || num_deleted == 0) {
		CPU_regY = 170; /* file not found */
		CPU_SetN;
	}
	else {
		CPU_regY = 1;
		CPU_ClrN;
	}
}

#endif /* HAVE_UTIL_UNLINK */

#ifdef DO_LOCK

static void Devices_H_LockUnlock(int readonly)
{
	int num_changed = 0;
	int num_failed = 0;

	CHECK_READ_ONLY;

	if (Devices_GetHostPath(FALSE) == 0)
		return;

#ifdef DO_DIR
	if (!Devices_OpenDir(host_path)) {
		CPU_regY = 170; /* file not found */
		CPU_SetN;
		return;
	}
	while (Devices_ReadDir(host_path, NULL, NULL, NULL, NULL, NULL))
#endif /* DO_DIR */
	{
		if (Devices_SetReadOnly(host_path, readonly))
			num_changed++;
		else
			num_failed++;
	}

	if (devbug)
		Log_print("%d changed, %d failed",
		       num_changed, num_failed);

	if (num_failed != 0 || num_changed == 0) {
		CPU_regY = 170; /* file not found */
		CPU_SetN;
	}
	else {
		CPU_regY = 1;
		CPU_ClrN;
	}
}

static void Devices_H_Lock(void)
{
	if (devbug)
		Log_print("LOCK Command");
	Devices_H_LockUnlock(TRUE);
}

static void Devices_H_Unlock(void)
{
	if (devbug)
		Log_print("UNLOCK Command");
	Devices_H_LockUnlock(FALSE);
}

#endif /* DO_LOCK */

static void Devices_H_Note(void)
{
	if (devbug)
		Log_print("NOTE Command");
	if (!Devices_GetIOCB())
		return;
	if (h_fp[h_iocb] != NULL) {
		long pos = ftell(h_fp[h_iocb]);
		if (pos >= 0) {
			int iocb = Devices_IOCB0 + h_iocb * 16;
			MEMORY_dPutByte(iocb + Devices_ICAX5, (UBYTE) pos);
			MEMORY_dPutByte(iocb + Devices_ICAX3, (UBYTE) (pos >> 8));
			MEMORY_dPutByte(iocb + Devices_ICAX4, (UBYTE) (pos >> 16));
			CPU_regY = 1;
			CPU_ClrN;
		}
		else {
			CPU_regY = 144; /* device done error */
			CPU_SetN;
		}
	}
	else {
		CPU_regY = 130; /* specified device does not exist; XXX: correct? */
		CPU_SetN;
	}
}

static void Devices_H_Point(void)
{
	if (devbug)
		Log_print("POINT Command");
	if (!Devices_GetIOCB())
		return;
	if (h_fp[h_iocb] != NULL) {
		int iocb = Devices_IOCB0 + h_iocb * 16;
		long pos = (MEMORY_dGetByte(iocb + Devices_ICAX4) << 16) +
			(MEMORY_dGetByte(iocb + Devices_ICAX3) << 8) + (MEMORY_dGetByte(iocb + Devices_ICAX5));
		if (fseek(h_fp[h_iocb], pos, SEEK_SET) == 0) {
			CPU_regY = 1;
			CPU_ClrN;
		}
		else {
			CPU_regY = 166; /* invalid POINT request */
			CPU_SetN;
		}
	}
	else {
		CPU_regY = 130; /* specified device does not exist; XXX: correct? */
		CPU_SetN;
	}
}

static FILE *binf = NULL;
static int runBinFile;
static int initBinFile;

/* Read a word from file */
static int Devices_H_BinReadWord(void)
{
	UBYTE buf[2];
	if (fread(buf, 1, 2, binf) != 2) {
		fclose(binf);
		binf = NULL;
		if (BINLOAD_start_binloading) {
			BINLOAD_start_binloading = FALSE;
			Log_print("binload: not valid BIN file");
			CPU_regY = 180; /* MyDOS: not a binary file */
			CPU_SetN;
			return -1;
		}
		if (runBinFile)
			CPU_regPC = MEMORY_dGetWordAligned(0x2e0);
		CPU_regY = 1;
		CPU_ClrN;
		return -1;
	}
	return buf[0] + (buf[1] << 8);
}

static void Devices_H_BinLoaderCont(void)
{
	if (binf == NULL)
		return;
	if (BINLOAD_start_binloading) {
		MEMORY_dPutByte(0x244, 0);
		MEMORY_dPutByte(0x09, 1);
	}
	else
		CPU_regS += 2;				/* pop ESC code */

	MEMORY_dPutByte(0x2e3, 0xd7);
	do {
		int temp;
		UWORD from;
		UWORD to;
		do
			temp = Devices_H_BinReadWord();
		while (temp == 0xffff);
		if (temp < 0)
			return;
		from = (UWORD) temp;

		temp = Devices_H_BinReadWord();
		if (temp < 0)
			return;
		to = (UWORD) temp;

		if (devbug)
			Log_print("H: Load: From %04X to %04X", from, to);

		if (BINLOAD_start_binloading) {
			if (runBinFile)
				MEMORY_dPutWordAligned(0x2e0, from);
			BINLOAD_start_binloading = FALSE;
		}

		to++;
		do {
			int byte = fgetc(binf);
			if (byte == EOF) {
				fclose(binf);
				binf = NULL;
				if (runBinFile)
					CPU_regPC = MEMORY_dGetWordAligned(0x2e0);
				if (initBinFile && (MEMORY_dGetByte(0x2e3) != 0xd7)) {
					/* run INIT routine which RTSes directly to RUN routine */
					CPU_regPC--;
					MEMORY_dPutByte(0x0100 + CPU_regS--, CPU_regPC >> 8);	/* high */
					MEMORY_dPutByte(0x0100 + CPU_regS--, CPU_regPC & 0xff);	/* low */
					CPU_regPC = MEMORY_dGetWordAligned(0x2e2);
				}
				return;
			}
			MEMORY_PutByte(from, (UBYTE) byte);
			from++;
		} while (from != to);
	} while (!initBinFile || MEMORY_dGetByte(0x2e3) == 0xd7);

	CPU_regS--;
	ESC_Add((UWORD) (0x100 + CPU_regS), ESC_BINLOADER_CONT, Devices_H_BinLoaderCont);
	CPU_regS--;
	MEMORY_dPutByte(0x0100 + CPU_regS--, 0x01);	/* high */
	MEMORY_dPutByte(0x0100 + CPU_regS, CPU_regS + 1);	/* low */
	CPU_regS--;
	CPU_regPC = MEMORY_dGetWordAligned(0x2e2);
	CPU_SetC;

	MEMORY_dPutByte(0x0300, 0x31);		/* for "Studio Dream" */
}

static void Devices_H_LoadProceed(int mydos)
{
	/* Log_print("MyDOS %d, AX1 %d, AX2 %d", mydos, MEMORY_dGetByte(Devices_ICAX1Z), MEMORY_dGetByte(Devices_ICAX2Z)); */
	if (mydos) {
		switch (MEMORY_dGetByte(Devices_ICAX1Z) /* XXX: & 7 ? */) {
		case 4:
			runBinFile = TRUE;
			initBinFile = TRUE;
			break;
		case 5:
			runBinFile = TRUE;
			initBinFile = FALSE;
			break;
		case 6:
			runBinFile = FALSE;
			initBinFile = TRUE;
			break;
		case 7:
		default:
			runBinFile = FALSE;
			initBinFile = FALSE;
			break;
		}
	}
	else {
		if (MEMORY_dGetByte(Devices_ICAX2Z) < 128)
			runBinFile = TRUE;
		else
			runBinFile = FALSE;
		initBinFile = TRUE;
	}

	BINLOAD_start_binloading = TRUE;
	Devices_H_BinLoaderCont();
}

static void Devices_H_Load(int mydos)
{
	const char *p;
	UBYTE buf[2];
	if (devbug)
		Log_print("LOAD Command");
	h_devnum = Devices_GetNumber(FALSE);
	if (h_devnum < 0)
		return;

	/* search for program on Devices_h_exe_path */
	for (p = Devices_h_exe_path; *p != '\0'; ) {
		int devnum;
		const char *q;
		char *r;
		if (p[0] == 'H' && p[1] >= '1' && p[1] <= '4' && p[2] == ':') {
			devnum = p[1] - '1';
			p += 3;
		}
		else
			devnum = h_devnum;
		for (q = p; *q != '\0' && *q != ';'; q++);
		r = atari_filename + (q - p);
		if (q != p) {
			memcpy(atari_filename, p, q - p);
			if (!IS_DIR_SEP(q[-1]))
				*r++ = '>';
		}
		if (Devices_GetAtariPath(devnum, r) == 0)
			return;
		Util_catpath(host_path, Devices_atari_h_dir[devnum], atari_path);
		binf = fopen(host_path, "rb");
		if (binf != NULL || *q == '\0')
			break;
		p = q + 1;
	}

	if (binf == NULL) {
		/* open from the specified location */
		if (Devices_GetAtariPath(h_devnum, atari_filename) == 0)
			return;
		Util_catpath(host_path, Devices_atari_h_dir[h_devnum], atari_path);
		binf = fopen(host_path, "rb");
		if (binf == NULL) {
			CPU_regY = 170;
			CPU_SetN;
			return;
		}
	}

	/* check header */
	if (fread(buf, 1, 2, binf) != 2 || buf[0] != 0xff || buf[1] != 0xff) {
		fclose(binf);
		binf = NULL;
		Log_print("H: load: not valid BIN file");
		CPU_regY = 180;
		CPU_SetN;
		return;
	}

	Devices_H_LoadProceed(mydos);
}

static void Devices_H_FileLength(void)
{
	if (devbug)
		Log_print("Get File Length Command");
	if (!Devices_GetIOCB())
		return;
	/* if IOCB is closed then assume it is a MyDOS Load File command */
	if (h_fp[h_iocb] == NULL)
		Devices_H_Load(TRUE);
	/* if we are running MyDOS then assume it is a MyDOS Load File command */
	else if (MEMORY_dGetByte(0x700) == 'M') {
		/* XXX: if (binf != NULL) fclose(binf); ? */
		binf = h_fp[h_iocb];
		Devices_H_LoadProceed(TRUE);
		/* XXX: don't close binf when complete? */
	}
	/* otherwise assume it is a file length command */
	else {
		int iocb = Devices_IOCB0 + h_iocb * 16;
		int filesize;
#if 0
		/* old, less portable implementation */
		struct stat fstatus;
		fstat(fileno(h_fp[h_iocb]), &fstatus);
		filesize = fstatus.st_size;
#else
		FILE *fp = h_fp[h_iocb];
		long currentpos = ftell(fp);
		filesize = Util_flen(fp);
		fseek(fp, currentpos, SEEK_SET);
#endif
		MEMORY_dPutByte(iocb + Devices_ICAX3, (UBYTE) filesize);
		MEMORY_dPutByte(iocb + Devices_ICAX4, (UBYTE) (filesize >> 8));
		MEMORY_dPutByte(iocb + Devices_ICAX5, (UBYTE) (filesize >> 16));
		CPU_regY = 1;
		CPU_ClrN;
	}
}

#ifdef DO_MKDIR
static void Devices_H_MakeDirectory(void)
{
	if (devbug)
		Log_print("MKDIR Command");
	CHECK_READ_ONLY;

	if (Devices_GetHostPath(FALSE) == 0)
		return;

	if (Devices_MakeDirectory(host_path)) {
		CPU_regY = 1;
		CPU_ClrN;
	}
	else {
		CPU_regY = 144; /* device done error */
		CPU_SetN;
	}
}
#endif

#ifdef DO_RMDIR
static void Devices_H_RemoveDirectory(void)
{
	if (devbug)
		Log_print("RMDIR Command");
	CHECK_READ_ONLY;

	if (Devices_GetHostPath(FALSE) == 0)
		return;

	CPU_regY = Devices_RemoveDirectory(host_path);
	if (CPU_regY >= 128)
		CPU_SetN;
	else
		CPU_ClrN;
}
#endif

static void Devices_H_ChangeDirectory(void)
{
	if (devbug)
		Log_print("CD Command");

	if (Devices_GetHostPath(FALSE) == 0)
		return;

	if (!Util_direxists(host_path)) {
		CPU_regY = 150;
		CPU_SetN;
		return;
	}

	if (atari_path[0] == '\0')
		Devices_h_current_dir[h_devnum][0] = '\0';
	else {
		char *p = Util_stpcpy(Devices_h_current_dir[h_devnum], atari_path);
		p[0] = Util_DIR_SEP_CHAR;
		p[1] = '\0';
	}

	CPU_regY = 1;
	CPU_ClrN;
}

static void Devices_H_DiskInfo(void)
{
	static UBYTE info[16] = {
		0x20,                                                  /* disk version: Sparta >= 2.0 */
		0x00,                                                  /* sector size: 0x100 */
		0xff, 0xff,                                            /* total sectors: 0xffff */
		0xff, 0xff,                                            /* free sectors: 0xffff */
		'H', 'D', 'I', 'S', 'K', '1' /* + devnum */, ' ', ' ', /* disk name */
		1,                                                     /* seq. number (number of writes) */
		1 /* + devnum */                                       /* random number (disk id) */
	};
	int devnum;

	if (devbug)
		Log_print("Get Disk Information Command");

	devnum = Devices_GetNumber(FALSE);
	if (devnum < 0)
		return;

	info[11] = (UBYTE) ('1' + devnum);
	info[15] = (UBYTE) (1 + devnum);
	MEMORY_CopyToMem(info, (UWORD) MEMORY_dGetWordAligned(Devices_ICBLLZ), 16);

	CPU_regY = 1;
	CPU_ClrN;
}

static void Devices_H_ToAbsolutePath(void)
{
	UWORD bufadr;
	const char *p;

	if (devbug)
		Log_print("To Absolute Path Command");

	if (Devices_GetHostPath(FALSE) == 0)
		return;

	/* XXX: we sometimes check here for directories
	   with a trailing Util_DIR_SEP_CHAR. It seems to work on Win32 and DJGPP. */
	if (!Util_direxists(host_path)) {
		CPU_regY = 150;
		CPU_SetN;
		return;
	}

	bufadr = MEMORY_dGetWordAligned(Devices_ICBLLZ);
	if (atari_path[0] != '\0') {
		MEMORY_PutByte(bufadr, '>');
		bufadr++;
		for (p = atari_path; *p != '\0'; p++) {
			if (*p == Util_DIR_SEP_CHAR) {
				if (p[1] == '\0')
					break;
				MEMORY_PutByte(bufadr, '>');
			}
			else
				MEMORY_PutByte(bufadr, (UBYTE) *p);
			bufadr++;
		}
	}
	MEMORY_PutByte(bufadr, 0x00);

	CPU_regY = 1;
	CPU_ClrN;
}

static void Devices_H_Special(void)
{
	if (devbug)
		Log_print("HHSPEC");

	switch (MEMORY_dGetByte(Devices_ICCOMZ)) {
#ifdef DO_RENAME
	case 0x20:
		Devices_H_Rename();
		return;
#endif
#ifdef HAVE_UTIL_UNLINK
	case 0x21:
		Devices_H_Delete();
		return;
#endif
#ifdef DO_LOCK
	case 0x23:
		Devices_H_Lock();
		return;
	case 0x24:
		Devices_H_Unlock();
		return;
#endif
	case 0x26:
		Devices_H_Note();
		return;
	case 0x25:
		Devices_H_Point();
		return;
	case 0x27: /* Sparta, MyDOS=Load */
		Devices_H_FileLength();
		return;
	case 0x28: /* Sparta */
		Devices_H_Load(FALSE);
		return;
#ifdef DO_MKDIR
	case 0x22: /* MyDOS */
	case 0x2a: /* MyDOS, Sparta */
		Devices_H_MakeDirectory();
		return;
#endif
#ifdef DO_RMDIR
	case 0x2b: /* Sparta */
		Devices_H_RemoveDirectory();
		return;
#endif
	case 0x29: /* MyDOS */
	case 0x2c: /* Sparta */
		Devices_H_ChangeDirectory();
		return;
	case 0x2f: /* Sparta */
		Devices_H_DiskInfo();
		return;
	case 0x30: /* Sparta */
		Devices_H_ToAbsolutePath();
		return;
	case 0xfe:
		if (devbug)
			Log_print("FORMAT Command");
		break;
	default:
		if (devbug)
			Log_print("UNKNOWN Command %02X", MEMORY_dGetByte(Devices_ICCOMZ));
		break;
	}

	CPU_regY = 168; /* invalid device command */
	CPU_SetN;
}


/* P: device emulation --------------------------------------------------- */

char Devices_print_command[256] = "lpr %s";

int Devices_SetPrintCommand(const char *command)
{
	const char *p = command;
	int was_percent_s = FALSE;
	while (*p != '\0') {
		if (*p++ == '%') {
			char c = *p++;
			if (c == '%')
				continue; /* %% is safe */
			if (c == 's' && !was_percent_s) {
				was_percent_s = TRUE; /* only one %s is safe */
				continue;
			}
			return FALSE;
		}
	}
	strcpy(Devices_print_command, command);
	return TRUE;
}

#ifdef HAVE_SYSTEM

static FILE *phf = NULL;
static char spool_file[FILENAME_MAX];

static void Devices_P_Close(void)
{
	if (devbug)
		Log_print("PHCLOS");

	if (phf != NULL) {
		fclose(phf);
		phf = NULL;

#ifdef __PLUS
		if (!Misc_ExecutePrintCmd(spool_file))
#endif
		{
			char command[256 + FILENAME_MAX]; /* 256 for Devices_print_command + FILENAME_MAX for spool_file */
			int retval;
			sprintf(command, Devices_print_command, spool_file);
			retval = system(command);
#if defined(HAVE_UTIL_UNLINK) && !defined(VMS) && !defined(MACOSX)
			if (Util_unlink(spool_file) != 0) {
				perror(spool_file);
			}
#endif
		}
	}
	CPU_regY = 1;
	CPU_ClrN;
}

static void Devices_P_Open(void)
{
	if (devbug)
		Log_print("PHOPEN");

	if (phf != NULL)
		Devices_P_Close();

	phf = Util_uniqopen(spool_file, "w");
	if (phf != NULL) {
		CPU_regY = 1;
		CPU_ClrN;
	}
	else {
		CPU_regY = 144; /* device done error */
		CPU_SetN;
	}
}

static void Devices_P_Write(void)
{
	UBYTE byte;

	if (devbug)
		Log_print("PHWRIT");

	byte = CPU_regA;
	if (byte == 0x9b)
		byte = '\n';

	fputc(byte, phf);
	CPU_regY = 1;
	CPU_ClrN;
}

static void Devices_P_Status(void)
{
	if (devbug)
		Log_print("PHSTAT");
}

static void Devices_P_Init(void)
{
	if (devbug)
		Log_print("PHINIT");

	if (phf != NULL) {
		fclose(phf);
		phf = NULL;
#ifdef HAVE_UTIL_UNLINK
		Util_unlink(spool_file);
#endif
	}
	CPU_regY = 1;
	CPU_ClrN;
}

#endif /* HAVE_SYSTEM */


/* K: and E: handlers for BASIC version, using getchar() and putchar() --- */

#ifdef BASIC

static void Devices_E_Read(void)
{
	int ch;

	ch = getchar();
	switch (ch) {
	case EOF:
		Atari800_Exit(FALSE);
		exit(0);
		break;
	case '\n':
		ch = 0x9b;
		break;
	default:
		break;
	}
	CPU_regA = (UBYTE) ch;
	CPU_regY = 1;
	CPU_ClrN;
}

static void Devices_E_Write(void)
{
	UBYTE ch;

	ch = CPU_regA;
	/* XXX: are '\f', '\b' and '\a' fully portable? */
	switch (ch) {
	case 0x7d: /* Clear Screen */
		putchar('\x0c'); /* ASCII Form Feed */
		break;
	case 0x7e:
		putchar('\x08'); /* ASCII Backspace */
		break;
	case 0x7f:
		putchar('\t');
		break;
	case 0x9b:
		putchar('\n');
		break;
	case 0xfd:
		putchar('\x07'); /* ASCII Bell */
		break;
	default:
		if ((ch >= 0x20) && (ch <= 0x7e))
			putchar(ch);
		break;
	}
	CPU_regY = 1;
	CPU_ClrN;
}

static void Devices_K_Read(void)
{
	int ch;
	int ch2;

	ch = getchar();
	switch (ch) {
	case EOF:
		Atari800_Exit(FALSE);
		exit(0);
		break;
	case '\n':
		ch = 0x9b;
		break;
	default:
		/* ignore characters until EOF or EOL */
		do
			ch2 = getchar();
		while (ch2 != EOF && ch2 != '\n');
		break;
	}
	CPU_regA = (UBYTE) ch;
	CPU_regY = 1;
	CPU_ClrN;
}

#endif /* BASIC */


/* Atari BASIC loader ---------------------------------------------------- */

static UWORD ehopen_addr = 0;
static UWORD ehclos_addr = 0;
static UWORD ehread_addr = 0;
static UWORD ehwrit_addr = 0;

static void Devices_IgnoreReady(void);
static void Devices_GetBasicCommand(void);
static void Devices_OpenBasicFile(void);
static void Devices_ReadBasicFile(void);
static void Devices_CloseBasicFile(void);

static void Devices_RestoreHandler(UWORD address, UBYTE esc_code)
{
	ESC_Remove(esc_code);
	/* restore original OS code */
	MEMORY_dCopyToMem(Atari800_machine_type == Atari800_MACHINE_XLXE
	            ? MEMORY_os + address - 0xc000
	            : MEMORY_os + address - 0xd800,
	           address, 3);
}

static void Devices_RestoreEHOPEN(void)
{
	Devices_RestoreHandler(ehopen_addr, ESC_EHOPEN);
}

static void Devices_RestoreEHCLOS(void)
{
	Devices_RestoreHandler(ehclos_addr, ESC_EHCLOS);
}

#ifndef BASIC

static void Devices_RestoreEHREAD(void)
{
	Devices_RestoreHandler(ehread_addr, ESC_EHREAD);
}

static void Devices_RestoreEHWRIT(void)
{
	Devices_RestoreHandler(ehwrit_addr, ESC_EHWRIT);
}

static void Devices_InstallIgnoreReady(void)
{
	ESC_AddEscRts(ehwrit_addr, ESC_EHWRIT, Devices_IgnoreReady);
}

#endif

/* Atari Basic loader step 1: ignore "READY" printed on E: after booting */
/* or step 6: ignore "READY" printed on E: after the "ENTER" command */

static const UBYTE * const ready_prompt = (const UBYTE *) "\x9bREADY\x9b";

static const UBYTE *ready_ptr = NULL;

static const UBYTE *basic_command_ptr = NULL;

static void Devices_IgnoreReady(void)
{
	if (ready_ptr != NULL && CPU_regA == *ready_ptr) {
		ready_ptr++;
		if (*ready_ptr == '\0') {
			ready_ptr = NULL;
			/* uninstall patch */
#ifdef BASIC
			ESC_AddEscRts(ehwrit_addr, ESC_EHWRIT, Devices_E_Write);
#else
			CPU_rts_handler = Devices_RestoreEHWRIT;
#endif
			if (BINLOAD_loading_basic == BINLOAD_LOADING_BASIC_SAVED) {
				basic_command_ptr = (const UBYTE *) "RUN \"E:\"\x9b";
				ESC_AddEscRts(ehread_addr, ESC_EHREAD, Devices_GetBasicCommand);
			}
			else if (BINLOAD_loading_basic == BINLOAD_LOADING_BASIC_LISTED) {
				basic_command_ptr = (const UBYTE *) "ENTER \"E:\"\x9b";
				ESC_AddEscRts(ehread_addr, ESC_EHREAD, Devices_GetBasicCommand);
			}
			else if (BINLOAD_loading_basic == BINLOAD_LOADING_BASIC_RUN) {
				basic_command_ptr = (const UBYTE *) "RUN\x9b";
				ESC_AddEscRts(ehread_addr, ESC_EHREAD, Devices_GetBasicCommand);
			}
		}
		CPU_regY = 1;
		CPU_ClrN;
		return;
	}
	/* not "READY" (maybe "BOOT ERROR" or a DOS message) */
	if (BINLOAD_loading_basic == BINLOAD_LOADING_BASIC_RUN) {
		/* don't "RUN" if no "READY" (probably "ERROR") */
		BINLOAD_loading_basic = 0;
		ready_ptr = NULL;
	}
	if (ready_ptr != NULL) {
		/* If ready_ptr != ready_prompt then we skipped some characters
		   from ready_prompt, which weren't part of full ready_prompt.
		   Well, they probably weren't that important. :-) */
		ready_ptr = ready_prompt;
	}
	/* call original handler */
#ifdef BASIC
	Devices_E_Write();
#else
	CPU_rts_handler = Devices_InstallIgnoreReady;
	Devices_RestoreEHWRIT();
	CPU_regPC = ehwrit_addr;
#endif
}

/* Atari Basic loader step 2: type command to load file from E: */
/* or step 7: type "RUN" for ENTERed program */

static void Devices_GetBasicCommand(void)
{
	if (basic_command_ptr != NULL) {
		CPU_regA = *basic_command_ptr++;
		CPU_regY = 1;
		CPU_ClrN;
		if (*basic_command_ptr != '\0')
			return;
		if (BINLOAD_loading_basic == BINLOAD_LOADING_BASIC_SAVED || BINLOAD_loading_basic == BINLOAD_LOADING_BASIC_LISTED)
			ESC_AddEscRts(ehopen_addr, ESC_EHOPEN, Devices_OpenBasicFile);
		basic_command_ptr = NULL;
	}
#ifdef BASIC
	ESC_AddEscRts(ehread_addr, ESC_EHREAD, Devices_E_Read);
#else
	CPU_rts_handler = Devices_RestoreEHREAD;
#endif
}

/* Atari Basic loader step 3: open file */

static void Devices_OpenBasicFile(void)
{
	if (BINLOAD_bin_file != NULL) {
		fseek(BINLOAD_bin_file, 0, SEEK_SET);
		ESC_AddEscRts(ehclos_addr, ESC_EHCLOS, Devices_CloseBasicFile);
		ESC_AddEscRts(ehread_addr, ESC_EHREAD, Devices_ReadBasicFile);
		CPU_regY = 1;
		CPU_ClrN;
	}
	CPU_rts_handler = Devices_RestoreEHOPEN;
}

/* Atari Basic loader step 4: read byte */

static void Devices_ReadBasicFile(void)
{
	if (BINLOAD_bin_file != NULL) {
		int ch = fgetc(BINLOAD_bin_file);
		if (ch == EOF) {
			CPU_regY = 136;
			CPU_SetN;
			return;
		}
		switch (BINLOAD_loading_basic) {
		case BINLOAD_LOADING_BASIC_LISTED:
			switch (ch) {
			case 0x9b:
				BINLOAD_loading_basic = BINLOAD_LOADING_BASIC_LISTED_ATARI;
				break;
			case 0x0a:
				BINLOAD_loading_basic = BINLOAD_LOADING_BASIC_LISTED_LF;
				ch = 0x9b;
				break;
			case 0x0d:
				BINLOAD_loading_basic = BINLOAD_LOADING_BASIC_LISTED_CR_OR_CRLF;
				ch = 0x9b;
				break;
			default:
				break;
			}
			break;
		case BINLOAD_LOADING_BASIC_LISTED_CR:
			if (ch == 0x0d)
				ch = 0x9b;
			break;
		case BINLOAD_LOADING_BASIC_LISTED_LF:
			if (ch == 0x0a)
				ch = 0x9b;
			break;
		case BINLOAD_LOADING_BASIC_LISTED_CRLF:
			if (ch == 0x0a) {
				ch = fgetc(BINLOAD_bin_file);
				if (ch == EOF) {
					CPU_regY = 136;
					CPU_SetN;
					return;
				}
			}
			if (ch == 0x0d)
				ch = 0x9b;
			break;
		case BINLOAD_LOADING_BASIC_LISTED_CR_OR_CRLF:
			if (ch == 0x0a) {
				BINLOAD_loading_basic = BINLOAD_LOADING_BASIC_LISTED_CRLF;
				ch = fgetc(BINLOAD_bin_file);
				if (ch == EOF) {
					CPU_regY = 136;
					CPU_SetN;
					return;
				}
			}
			else
				BINLOAD_loading_basic = BINLOAD_LOADING_BASIC_LISTED_CR;
			if (ch == 0x0d)
				ch = 0x9b;
			break;
		case BINLOAD_LOADING_BASIC_SAVED:
		case BINLOAD_LOADING_BASIC_LISTED_ATARI:
		default:
			break;
		}
		CPU_regA = (UBYTE) ch;
		CPU_regY = 1;
		CPU_ClrN;
	}
}

/* Atari Basic loader step 5: close file */

static void Devices_CloseBasicFile(void)
{
	if (BINLOAD_bin_file != NULL) {
		fclose(BINLOAD_bin_file);
		BINLOAD_bin_file = NULL;
		/* "RUN" ENTERed program */
		if (BINLOAD_loading_basic != 0 && BINLOAD_loading_basic != BINLOAD_LOADING_BASIC_SAVED) {
			ready_ptr = ready_prompt;
			ESC_AddEscRts(ehwrit_addr, ESC_EHWRIT, Devices_IgnoreReady);
			BINLOAD_loading_basic = BINLOAD_LOADING_BASIC_RUN;
		}
		else
			BINLOAD_loading_basic = 0;
	}
#ifdef BASIC
	ESC_AddEscRts(ehread_addr, ESC_EHREAD, Devices_E_Read);
#else
	Devices_RestoreEHREAD();
#endif
	CPU_rts_handler = Devices_RestoreEHCLOS;
	CPU_regY = 1;
	CPU_ClrN;
}


/* Patches management ---------------------------------------------------- */

int Devices_enable_h_patch = TRUE;
int Devices_enable_p_patch = TRUE;
int Devices_enable_r_patch = FALSE;

/* Devices_PatchOS is called by ESC_PatchOS to modify standard device
   handlers in Atari OS. It puts escape codes at beginnings of OS routines,
   so the patches work even if they are called directly, without CIO.
   Returns TRUE if something has been patched.
   Currently we only patch P: and, in BASIC version, E: and K:.
   We don't replace C: with H: now, so the cassette works even
   if H: is enabled.
*/
int Devices_PatchOS(void)
{
	UWORD addr;
	int i;
	int patched = FALSE;

	switch (Atari800_machine_type) {
	case Atari800_MACHINE_OSA:
	case Atari800_MACHINE_OSB:
		addr = 0xf0e3;
		break;
	case Atari800_MACHINE_XLXE:
		addr = 0xc42e;
		break;
	default:
		return FALSE;
	}

	for (i = 0; i < 5; i++) {
		UWORD devtab = MEMORY_dGetWord(addr + 1);
		switch (MEMORY_dGetByte(addr)) {
#ifdef HAVE_SYSTEM
		case 'P':
			if (Devices_enable_p_patch) {
				ESC_AddEscRts((UWORD) (MEMORY_dGetWord(devtab + Devices_TABLE_OPEN) + 1),
				                   ESC_PHOPEN, Devices_P_Open);
				ESC_AddEscRts((UWORD) (MEMORY_dGetWord(devtab + Devices_TABLE_CLOS) + 1),
				                   ESC_PHCLOS, Devices_P_Close);
				ESC_AddEscRts((UWORD) (MEMORY_dGetWord(devtab + Devices_TABLE_WRIT) + 1),
				                   ESC_PHWRIT, Devices_P_Write);
				ESC_AddEscRts((UWORD) (MEMORY_dGetWord(devtab + Devices_TABLE_STAT) + 1),
				                   ESC_PHSTAT, Devices_P_Status);
				ESC_AddEscRts2((UWORD) (devtab + Devices_TABLE_INIT), ESC_PHINIT,
				                    Devices_P_Init);
				patched = TRUE;
			}
			else {
				ESC_Remove(ESC_PHOPEN);
				ESC_Remove(ESC_PHCLOS);
				ESC_Remove(ESC_PHWRIT);
				ESC_Remove(ESC_PHSTAT);
				ESC_Remove(ESC_PHINIT);
			}
			break;
#endif

		case 'E':
			if (BINLOAD_loading_basic) {
				ehopen_addr = MEMORY_dGetWord(devtab + Devices_TABLE_OPEN) + 1;
				ehclos_addr = MEMORY_dGetWord(devtab + Devices_TABLE_CLOS) + 1;
				ehread_addr = MEMORY_dGetWord(devtab + Devices_TABLE_READ) + 1;
				ehwrit_addr = MEMORY_dGetWord(devtab + Devices_TABLE_WRIT) + 1;
				ready_ptr = ready_prompt;
				ESC_AddEscRts(ehwrit_addr, ESC_EHWRIT, Devices_IgnoreReady);
				patched = TRUE;
			}
#ifdef BASIC
			else
				ESC_AddEscRts((UWORD) (MEMORY_dGetWord(devtab + Devices_TABLE_WRIT) + 1),
				                   ESC_EHWRIT, Devices_E_Write);
			ESC_AddEscRts((UWORD) (MEMORY_dGetWord(devtab + Devices_TABLE_READ) + 1),
			                   ESC_EHREAD, Devices_E_Read);
			patched = TRUE;
			break;
		case 'K':
			ESC_AddEscRts((UWORD) (MEMORY_dGetWord(devtab + Devices_TABLE_READ) + 1),
			                   ESC_KHREAD, Devices_K_Read);
			patched = TRUE;
			break;
#endif
		default:
			break;
		}
		addr += 3;				/* Next Device in HATABS */
	}
	return patched;
}

/* New handling of H: device.
   Previously we simply replaced C: device in OS with our H:.
   Now we don't change ROM for H: patch, but add H: to HATABS in RAM
   and put the device table and patches in unused address space
   (0xd100-0xd1ff), which is meant for 'new devices' (like hard disk).
   We have to continuously check if our H: is still in HATABS,
   because RESET routine in Atari OS clears HATABS and initializes it
   using a table in ROM (see Devices_PatchOS).
   Before we put H: entry in HATABS, we must make sure that HATABS is there.
   For example a program that doesn't use Atari OS can use this memory area
   for its own data, and we shouldn't place 'H' there.
   We also allow an Atari program to change address of H: device table.
   So after we put H: entry in HATABS, we only check if 'H' is still where
   we put it (h_entry_address).
   Devices_UpdateHATABSEntry and Devices_RemoveHATABSEntry can be used to add
   other devices than H:. */

#define HATABS 0x31a

UWORD Devices_UpdateHATABSEntry(char device, UWORD entry_address,
							   UWORD table_address)
{
	UWORD address;
	if (entry_address != 0 && MEMORY_dGetByte(entry_address) == device)
		return entry_address;
	if (MEMORY_dGetByte(HATABS) != 'P' || MEMORY_dGetByte(HATABS + 3) != 'C'
		|| MEMORY_dGetByte(HATABS + 6) != 'E' || MEMORY_dGetByte(HATABS + 9) != 'S'
		|| MEMORY_dGetByte(HATABS + 12) != 'K')
		return entry_address;
	for (address = HATABS + 15; address < HATABS + 33; address += 3) {
		if (MEMORY_dGetByte(address) == device)
			return address;
		if (MEMORY_dGetByte(address) == 0) {
			MEMORY_dPutByte(address, device);
			MEMORY_dPutWord(address + 1, table_address);
			return address;
		}
	}
	/* HATABS full */
	return entry_address;
}

void Devices_RemoveHATABSEntry(char device, UWORD entry_address,
							  UWORD table_address)
{
	if (entry_address != 0 && MEMORY_dGetByte(entry_address) == device
		&& MEMORY_dGetWord(entry_address + 1) == table_address) {
		MEMORY_dPutByte(entry_address, 0);
		MEMORY_dPutWord(entry_address + 1, 0);
	}
}

static UWORD h_entry_address = 0;
#ifdef R_IO_DEVICE
static UWORD r_entry_address = 0;
#endif

#define H_DEVICE_BEGIN  0xd140
#define H_TABLE_ADDRESS 0xd140
#define H_PATCH_OPEN    0xd150
#define H_PATCH_CLOS    0xd153
#define H_PATCH_READ    0xd156
#define H_PATCH_WRIT    0xd159
#define H_PATCH_STAT    0xd15c
#define H_PATCH_SPEC    0xd15f
#define H_DEVICE_END    0xd161

#ifdef R_IO_DEVICE
#define R_DEVICE_BEGIN  0xd180
#define R_TABLE_ADDRESS 0xd180
#define R_PATCH_OPEN    0xd1a0
#define R_PATCH_CLOS    0xd1a3
#define R_PATCH_READ    0xd1a6
#define R_PATCH_WRIT    0xd1a9
#define R_PATCH_STAT    0xd1ac
#define R_PATCH_SPEC    0xd1af
#define R_PATCH_INIT    0xd1b3
#define R_DEVICE_END    0xd1b5
#endif

void Devices_Frame(void)
{
	if (Devices_enable_h_patch)
		h_entry_address = Devices_UpdateHATABSEntry('H', h_entry_address, H_TABLE_ADDRESS);

#ifdef R_IO_DEVICE
	if (Devices_enable_r_patch)
		r_entry_address = Devices_UpdateHATABSEntry('R', r_entry_address, R_TABLE_ADDRESS);
#endif
}

/* this is called when Devices_enable_h_patch is toggled */
void Devices_UpdatePatches(void)
{
	if (Devices_enable_h_patch) {		/* enable H: device */
		/* change memory attributes for the area, where we put
		   the H: handler table and patches */
		MEMORY_SetROM(H_DEVICE_BEGIN, H_DEVICE_END);
		/* set handler table */
		MEMORY_dPutWord(H_TABLE_ADDRESS + Devices_TABLE_OPEN, H_PATCH_OPEN - 1);
		MEMORY_dPutWord(H_TABLE_ADDRESS + Devices_TABLE_CLOS, H_PATCH_CLOS - 1);
		MEMORY_dPutWord(H_TABLE_ADDRESS + Devices_TABLE_READ, H_PATCH_READ - 1);
		MEMORY_dPutWord(H_TABLE_ADDRESS + Devices_TABLE_WRIT, H_PATCH_WRIT - 1);
		MEMORY_dPutWord(H_TABLE_ADDRESS + Devices_TABLE_STAT, H_PATCH_STAT - 1);
		MEMORY_dPutWord(H_TABLE_ADDRESS + Devices_TABLE_SPEC, H_PATCH_SPEC - 1);
		/* set patches */
		ESC_AddEscRts(H_PATCH_OPEN, ESC_HHOPEN, Devices_H_Open);
		ESC_AddEscRts(H_PATCH_CLOS, ESC_HHCLOS, Devices_H_Close);
		ESC_AddEscRts(H_PATCH_READ, ESC_HHREAD, Devices_H_Read);
		ESC_AddEscRts(H_PATCH_WRIT, ESC_HHWRIT, Devices_H_Write);
		ESC_AddEscRts(H_PATCH_STAT, ESC_HHSTAT, Devices_H_Status);
		ESC_AddEscRts(H_PATCH_SPEC, ESC_HHSPEC, Devices_H_Special);
		/* H: in HATABS will be added next frame by Devices_Frame */
	}
	else {						/* disable H: device */
		/* remove H: entry from HATABS */
		Devices_RemoveHATABSEntry('H', h_entry_address, H_TABLE_ADDRESS);
		/* remove patches */
		ESC_Remove(ESC_HHOPEN);
		ESC_Remove(ESC_HHCLOS);
		ESC_Remove(ESC_HHREAD);
		ESC_Remove(ESC_HHWRIT);
		ESC_Remove(ESC_HHSTAT);
		ESC_Remove(ESC_HHSPEC);
		/* fill memory area used for table and patches with 0xff */
		MEMORY_dFillMem(H_DEVICE_BEGIN, 0xff, H_DEVICE_END - H_DEVICE_BEGIN + 1);
	}

#ifdef R_IO_DEVICE
	if (Devices_enable_r_patch) {		/* enable R: device */
		/* change memory attributes for the area, where we put
		   the R: handler table and patches */
		MEMORY_SetROM(R_DEVICE_BEGIN, R_DEVICE_END);
		/* set handler table */
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_OPEN, R_PATCH_OPEN - 1);
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_CLOS, R_PATCH_CLOS - 1);
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_READ, R_PATCH_READ - 1);
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_WRIT, R_PATCH_WRIT - 1);
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_STAT, R_PATCH_STAT - 1);
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_SPEC, R_PATCH_SPEC - 1);
		MEMORY_dPutWord(R_TABLE_ADDRESS + Devices_TABLE_INIT, R_PATCH_INIT - 1);
		/* set patches */
		ESC_AddEscRts(R_PATCH_OPEN, ESC_ROPEN, RDevice_OPEN);
		ESC_AddEscRts(R_PATCH_CLOS, ESC_RCLOS, RDevice_CLOS);
		ESC_AddEscRts(R_PATCH_READ, ESC_RREAD, RDevice_READ);
		ESC_AddEscRts(R_PATCH_WRIT, ESC_RWRIT, RDevice_WRIT);
		ESC_AddEscRts(R_PATCH_STAT, ESC_RSTAT, RDevice_STAT);
		ESC_AddEscRts(R_PATCH_SPEC, ESC_RSPEC, RDevice_SPEC);
		ESC_AddEscRts(R_PATCH_INIT, ESC_RINIT, RDevice_INIT);
		/* R: in HATABS will be added next frame by Devices_Frame */
	}
	else {						/* disable R: device */
		/* remove R: entry from HATABS */
		Devices_RemoveHATABSEntry('R', r_entry_address, R_TABLE_ADDRESS);
		/* remove patches */
		ESC_Remove(ESC_ROPEN);
		ESC_Remove(ESC_RCLOS);
		ESC_Remove(ESC_RREAD);
		ESC_Remove(ESC_RWRIT);
		ESC_Remove(ESC_RSTAT);
		ESC_Remove(ESC_RSPEC);
		/* fill memory area used for table and patches with 0xff */
		MEMORY_dFillMem(R_DEVICE_BEGIN, 0xff, R_DEVICE_END - R_DEVICE_BEGIN + 1);
	}
#endif /* defined(R_IO_DEVICE) */
}

/*
vim:ts=4:sw=4:
*/
