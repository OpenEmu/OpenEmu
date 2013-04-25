/*
 * statesav.c - saving the emulator's state to a file
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
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h> /* getcwd */
#endif
#ifdef HAVE_DIRECT_H
#include <direct.h> /* getcwd on MSVC*/
#endif
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif
#ifdef DREAMCAST
#include <bzlib/bzlib.h>
#define MEMCOMPR     /* compress in memory before writing */
#include "vmu.h"
#include "icon.h"
#include "version.h"
#endif

#include "atari.h"
#include "statesav.h"
#include "antic.h"
#include "cartridge.h"
#include "cpu.h"
#include "gtia.h"
#include "log.h"
#include "pbi.h"
#include "pia.h"
#include "pokey.h"
#include "sio.h"
#include "util.h"
#ifdef PBI_MIO
#include "pbi_mio.h"
#endif
#ifdef PBI_BB
#include "pbi_bb.h"
#endif
#ifdef PBI_XLD
#include "pbi_xld.h"
#endif
#ifdef XEP80_EMULATION
#include "xep80.h"
#endif

#define SAVE_VERSION_NUMBER 7 /* Last changed after Atari800 2.2.1 */

#if defined(MEMCOMPR)
static gzFile *mem_open(const char *name, const char *mode);
static int mem_close(gzFile *stream);
static size_t mem_read(void *buf, size_t len, gzFile *stream);
static size_t mem_write(const void *buf, size_t len, gzFile *stream);
#define GZOPEN(X, Y)     mem_open(X, Y)
#define GZCLOSE(X)       mem_close(X)
#define GZREAD(X, Y, Z)  mem_read(Y, Z, X)
#define GZWRITE(X, Y, Z) mem_write(Y, Z, X)
#undef GZERROR
#elif defined(HAVE_LIBZ) /* above MEMCOMPR, below HAVE_LIBZ */
#define GZOPEN(X, Y)     gzopen(X, Y)
#define GZCLOSE(X)       gzclose(X)
#define GZREAD(X, Y, Z)  gzread(X, Y, Z)
#define GZWRITE(X, Y, Z) gzwrite(X, (const voidp) Y, Z)
#define GZERROR(X, Y)    gzerror(X, Y)
#else
#define GZOPEN(X, Y)     fopen(X, Y)
#define GZCLOSE(X)       fclose(X)
#define GZREAD(X, Y, Z)  fread(Y, Z, 1, X)
#define GZWRITE(X, Y, Z) fwrite(Y, Z, 1, X)
#undef GZERROR
#define gzFile  FILE *
#define Z_OK    0
#endif

static gzFile StateFile = NULL;
static int nFileError = Z_OK;

static void GetGZErrorText(void)
{
#ifdef GZERROR
	const char *error = GZERROR(StateFile, &nFileError);
	if (nFileError == Z_ERRNO) {
#ifdef HAVE_STRERROR
		Log_print("The following general file I/O error occurred:");
		Log_print(strerror(errno));
#else
		Log_print("A file I/O error occurred");
#endif
		return;
	}
	Log_print("ZLIB returned the following error: %s", error);
#endif /* GZERROR */
	Log_print("State file I/O failed.");
}

/* Value is memory location of data, num is number of type to save */
void StateSav_SaveUBYTE(const UBYTE *data, int num)
{
	if (!StateFile || nFileError != Z_OK)
		return;

	/* Assumption is that UBYTE = 8bits and the pointer passed in refers
	   directly to the active bits if in a padded location. If not (unlikely)
	   you'll have to redefine this to save appropriately for cross-platform
	   compatibility */
	if (GZWRITE(StateFile, data, num) == 0)
		GetGZErrorText();
}

/* Value is memory location of data, num is number of type to save */
void StateSav_ReadUBYTE(UBYTE *data, int num)
{
	if (!StateFile || nFileError != Z_OK)
		return;

	if (GZREAD(StateFile, data, num) == 0)
		GetGZErrorText();
}

/* Value is memory location of data, num is number of type to save */
void StateSav_SaveUWORD(const UWORD *data, int num)
{
	if (!StateFile || nFileError != Z_OK)
		return;

	/* UWORDS are saved as 16bits, regardless of the size on this particular
	   platform. Each byte of the UWORD will be pushed out individually in
	   LSB order. The shifts here and in the read routines will work for both
	   LSB and MSB architectures. */
	while (num > 0) {
		UWORD temp;
		UBYTE byte;

		temp = *data++;
		byte = temp & 0xff;
		if (GZWRITE(StateFile, &byte, 1) == 0) {
			GetGZErrorText();
			break;
		}

		temp >>= 8;
		byte = temp & 0xff;
		if (GZWRITE(StateFile, &byte, 1) == 0) {
			GetGZErrorText();
			break;
		}
		num--;
	}
}

/* Value is memory location of data, num is number of type to save */
void StateSav_ReadUWORD(UWORD *data, int num)
{
	if (!StateFile || nFileError != Z_OK)
		return;

	while (num > 0) {
		UBYTE byte1, byte2;

		if (GZREAD(StateFile, &byte1, 1) == 0) {
			GetGZErrorText();
			break;
		}

		if (GZREAD(StateFile, &byte2, 1) == 0) {
			GetGZErrorText();
			break;
		}

		*data++ = (byte2 << 8) | byte1;
		num--;
	}
}

void StateSav_SaveINT(const int *data, int num)
{
	if (!StateFile || nFileError != Z_OK)
		return;

	/* INTs are always saved as 32bits (4 bytes) in the file. They can be any size
	   on the platform however. The sign bit is clobbered into the fourth byte saved
	   for each int; on read it will be extended out to its proper position for the
	   native INT size */
	while (num > 0) {
		UBYTE signbit = 0;
		unsigned int temp;
		UBYTE byte;
		int temp0;

		temp0 = *data++;
		if (temp0 < 0) {
			temp0 = -temp0;
			signbit = 0x80;
		}
		temp = (unsigned int) temp0;

		byte = temp & 0xff;
		if (GZWRITE(StateFile, &byte, 1) == 0) {
			GetGZErrorText();
			break;
		}

		temp >>= 8;
		byte = temp & 0xff;
		if (GZWRITE(StateFile, &byte, 1) == 0) {
			GetGZErrorText();
			break;
		}

		temp >>= 8;
		byte = temp & 0xff;
		if (GZWRITE(StateFile, &byte, 1) == 0) {
			GetGZErrorText();
			break;
		}

		temp >>= 8;
		byte = (temp & 0x7f) | signbit;
		if (GZWRITE(StateFile, &byte, 1) == 0) {
			GetGZErrorText();
			break;
		}

		num--;
	}
}

void StateSav_ReadINT(int *data, int num)
{
	if (!StateFile || nFileError != Z_OK)
		return;

	while (num > 0) {
		UBYTE signbit = 0;
		int temp;
		UBYTE byte1, byte2, byte3, byte4;

		if (GZREAD(StateFile, &byte1, 1) == 0) {
			GetGZErrorText();
			break;
		}

		if (GZREAD(StateFile, &byte2, 1) == 0) {
			GetGZErrorText();
			break;
		}

		if (GZREAD(StateFile, &byte3, 1) == 0) {
			GetGZErrorText();
			break;
		}

		if (GZREAD(StateFile, &byte4, 1) == 0) {
			GetGZErrorText();
			break;
		}

		signbit = byte4 & 0x80;
		byte4 &= 0x7f;

		temp = (byte4 << 24) | (byte3 << 16) | (byte2 << 8) | byte1;
		if (signbit)
			temp = -temp;
		*data++ = temp;

		num--;
	}
}

void StateSav_SaveFNAME(const char *filename)
{
	UWORD namelen;
#ifdef HAVE_GETCWD
	char dirname[FILENAME_MAX]="";

	/* Check to see if file is in application tree, if so, just save as
	   relative path....*/
	if (getcwd(dirname, FILENAME_MAX) != NULL) {
		if (strncmp(filename, dirname, strlen(dirname)) == 0)
			/* XXX: check if '/' or '\\' follows dirname in filename? */
			filename += strlen(dirname) + 1;
	}
#endif

	namelen = strlen(filename);
	/* Save the length of the filename, followed by the filename */
	StateSav_SaveUWORD(&namelen, 1);
	StateSav_SaveUBYTE((const UBYTE *) filename, namelen);
}

void StateSav_ReadFNAME(char *filename)
{
	UWORD namelen = 0;

	StateSav_ReadUWORD(&namelen, 1);
	if (namelen >= FILENAME_MAX) {
		Log_print("Filenames of %d characters not supported on this platform", (int) namelen);
		return;
	}
	StateSav_ReadUBYTE((UBYTE *) filename, namelen);
	filename[namelen] = 0;
}

int StateSav_SaveAtariState(const char *filename, const char *mode, UBYTE SaveVerbose)
{
	UBYTE StateVersion = SAVE_VERSION_NUMBER;

	if (StateFile != NULL) {
		GZCLOSE(StateFile);
		StateFile = NULL;
	}
	nFileError = Z_OK;

	StateFile = GZOPEN(filename, mode);
	if (StateFile == NULL) {
		Log_print("Could not open %s for state save.", filename);
		GetGZErrorText();
		return FALSE;
	}
	if (GZWRITE(StateFile, "ATARI800", 8) == 0) {
		GetGZErrorText();
		GZCLOSE(StateFile);
		StateFile = NULL;
		return FALSE;
	}

	StateSav_SaveUBYTE(&StateVersion, 1);
	StateSav_SaveUBYTE(&SaveVerbose, 1);
	/* The order here is important. Atari800_StateSave must be first because it saves the machine type, and
	   decisions on what to save/not save are made based off that later in the process */
	Atari800_StateSave();
	CARTRIDGE_StateSave();
	SIO_StateSave();
	ANTIC_StateSave();
	CPU_StateSave(SaveVerbose);
	GTIA_StateSave();
	PIA_StateSave();
	POKEY_StateSave();
#ifdef XEP80_EMULATION
	XEP80_StateSave();
#else
	{
		int local_xep80_enabled = FALSE;
		StateSav_SaveINT(&local_xep80_enabled, 1);
	}
#endif /* XEP80_EMULATION */
	PBI_StateSave();
#ifdef PBI_MIO
	PBI_MIO_StateSave();
#else
	{
		int local_mio_enabled = FALSE;
		StateSav_SaveINT(&local_mio_enabled, 1);
	}
#endif /* PBI_MIO */
#ifdef PBI_BB
	PBI_BB_StateSave();
#else
	{
		int local_bb_enabled = FALSE;
		StateSav_SaveINT(&local_bb_enabled, 1);
	}
#endif /* PBI_BB */
#ifdef PBI_XLD
	PBI_XLD_StateSave();
#else
	{
		int local_xld_enabled = FALSE;
		StateSav_SaveINT(&local_xld_enabled, 1);
	}
#endif /* PBI_XLD */
#ifdef DREAMCAST
	DCStateSave();
#endif

	if (GZCLOSE(StateFile) != 0) {
		StateFile = NULL;
		return FALSE;
	}
	StateFile = NULL;

	if (nFileError != Z_OK)
		return FALSE;

	return TRUE;
}

int StateSav_ReadAtariState(const char *filename, const char *mode)
{
	char header_string[8];
	UBYTE StateVersion = 0;  /* The version of the save file */
	UBYTE SaveVerbose = 0;   /* Verbose mode means save basic, OS if patched */

	if (StateFile != NULL) {
		GZCLOSE(StateFile);
		StateFile = NULL;
	}
	nFileError = Z_OK;

	StateFile = GZOPEN(filename, mode);
	if (StateFile == NULL) {
		Log_print("Could not open %s for state read.", filename);
		GetGZErrorText();
		return FALSE;
	}

	if (GZREAD(StateFile, header_string, 8) == 0) {
		GetGZErrorText();
		GZCLOSE(StateFile);
		StateFile = NULL;
		return FALSE;
	}
	if (memcmp(header_string, "ATARI800", 8) != 0) {
		Log_print("This is not an Atari800 state save file.");
		GZCLOSE(StateFile);
		StateFile = NULL;
		return FALSE;
	}

	if (GZREAD(StateFile, &StateVersion, 1) == 0
	 || GZREAD(StateFile, &SaveVerbose, 1) == 0) {
		Log_print("Failed read from Atari state file.");
		GetGZErrorText();
		GZCLOSE(StateFile);
		StateFile = NULL;
		return FALSE;
	}

	if (StateVersion > SAVE_VERSION_NUMBER || StateVersion < 3) {
		Log_print("Cannot read this state file because it is an incompatible version.");
		GZCLOSE(StateFile);
		StateFile = NULL;
		return FALSE;
	}

	Atari800_StateRead(StateVersion);
	if (StateVersion >= 4) {
		CARTRIDGE_StateRead(StateVersion);
		SIO_StateRead();
	}
	ANTIC_StateRead();
	CPU_StateRead(SaveVerbose, StateVersion);
	GTIA_StateRead(StateVersion);
	PIA_StateRead(StateVersion);
	POKEY_StateRead();
	if (StateVersion >= 6) {
#ifdef XEP80_EMULATION
		XEP80_StateRead();
#else
		int local_xep80_enabled;
		StateSav_ReadINT(&local_xep80_enabled,1);
		if (local_xep80_enabled) {
			Log_print("Cannot read this state file because this version does not support XEP80.");
			GZCLOSE(StateFile);
			StateFile = NULL;
			return FALSE;
		}
#endif /* XEP80_EMULATION */
		PBI_StateRead();
#ifdef PBI_MIO
		PBI_MIO_StateRead();
#else
		{
			int local_mio_enabled;
			StateSav_ReadINT(&local_mio_enabled,1);
			if (local_mio_enabled) {
				Log_print("Cannot read this state file because this version does not support MIO.");
				GZCLOSE(StateFile);
				StateFile = NULL;
				return FALSE;
			}
		}
#endif /* PBI_MIO */
#ifdef PBI_BB
		PBI_BB_StateRead();
#else
		{
			int local_bb_enabled;
			StateSav_ReadINT(&local_bb_enabled,1);
			if (local_bb_enabled) {
				Log_print("Cannot read this state file because this version does not support the Black Box.");
				GZCLOSE(StateFile);
				StateFile = NULL;
				return FALSE;
			}
		}
#endif /* PBI_BB */
#ifdef PBI_XLD
		PBI_XLD_StateRead();
#else
		{
			int local_xld_enabled;
			StateSav_ReadINT(&local_xld_enabled,1);
			if (local_xld_enabled) {
				Log_print("Cannot read this state file because this version does not support the 1400XL/1450XLD.");
				GZCLOSE(StateFile);
				StateFile = NULL;
				return FALSE;
			}
		}
#endif /* PBI_XLD */
	}
#ifdef DREAMCAST
	DCStateRead();
#endif

	GZCLOSE(StateFile);
	StateFile = NULL;

	if (nFileError != Z_OK)
		return FALSE;

	return TRUE;
}


/* hack to compress in memory before writing
 * - for DREAMCAST only
 * - 2 reasons for this:
 * - use bzip2 instead of zip: better compression ratio (the DC VMUs are small)
 * - write in DC specific file format to provide icon and description
 */
#ifdef MEMCOMPR

static char * plainmembuf;
static unsigned int plainmemoff;
static char * comprmembuf;
#define OM_READ  1
#define OM_WRITE 2
static int openmode;
static unsigned int unclen;
static char savename[FILENAME_MAX];
#define HDR_LEN 640

#define ALLOC_LEN 210000

/* replacement for GZOPEN */
static gzFile *mem_open(const char *name, const char *mode)
{
	if (*mode == 'w') {
		/* open for write (save) */
		openmode = OM_WRITE;
		strcpy(savename, name); /* remember name */
		plainmembuf = Util_malloc(ALLOC_LEN);
		plainmemoff = 0; /*HDR_LEN;*/
		return (gzFile *) plainmembuf;
	}
	else {
		/* open for read (read) */
		FILE *f;
		size_t len;
		openmode = OM_READ;
		unclen = ALLOC_LEN;
		f = fopen(name, mode);
		if (f == NULL)
			return NULL;
		plainmembuf = Util_malloc(ALLOC_LEN);
		comprmembuf = Util_malloc(ALLOC_LEN);
		len = fread(comprmembuf, 1, ALLOC_LEN, f);
		fclose(f);
		/* XXX: does DREAMCAST's fread return ((size_t) -1) ? */
		if (len != 0
		 && BZ2_bzBuffToBuffDecompress(plainmembuf, &unclen, comprmembuf + HDR_LEN, len - HDR_LEN, 1, 0) == BZ_OK) {
#ifdef DEBUG
			printf("decompress: old len %lu, new len %lu\n",
				   (unsigned long) len - 1024, (unsigned long) unclen);
#endif
			free(comprmembuf);
			plainmemoff = 0;
			return (gzFile *) plainmembuf;
		}
		free(comprmembuf);
		free(plainmembuf);
		return NULL;
	}
}

/* replacement for GZCLOSE */
static int mem_close(gzFile *stream)
{
	int status = -1;
	unsigned int comprlen = ALLOC_LEN - HDR_LEN;
	if (openmode != OM_WRITE) {
		/* was opened for read */
		free(plainmembuf);
		return 0;
	}
	comprmembuf = Util_malloc(ALLOC_LEN);
	if (BZ2_bzBuffToBuffCompress(comprmembuf + HDR_LEN, &comprlen, plainmembuf, plainmemoff, 9, 0, 0) == BZ_OK) {
		FILE *f;
		f = fopen(savename, "wb");
		if (f != NULL) {
			char icon[32 + 512];
#ifdef DEBUG
			printf("mem_close: plain len %lu, compr len %lu\n",
			       (unsigned long) plainmemoff, (unsigned long) comprlen);
#endif
			memcpy(icon, palette, 32);
			memcpy(icon + 32, bitmap, 512);
			ndc_vmu_create_vmu_header(comprmembuf, "Atari800DC",
						  "Atari800DC " A800DCVERASC " saved state",
						  comprlen, icon);
			comprlen = (comprlen + HDR_LEN + 511) & ~511;
			ndc_vmu_do_crc(comprmembuf, comprlen);
			status = (fwrite(comprmembuf, 1, comprlen, f) == comprlen) ? 0 : -1;
			status |= fclose(f);
#ifdef DEBUG
			if (status != 0)
				printf("mem_close: fwrite: error!!\n");
#endif
		}
	}
	free(comprmembuf);
	free(plainmembuf);
	return status;
}

/* replacement for GZREAD */
static size_t mem_read(void *buf, size_t len, gzFile *stream)
{
	if (plainmemoff + len > unclen) return 0;  /* shouldn't happen */
	memcpy(buf, plainmembuf + plainmemoff, len);
	plainmemoff += len;
	return len;
}

/* replacement for GZWRITE */
static size_t mem_write(const void *buf, size_t len, gzFile *stream)
{
	if (plainmemoff + len > ALLOC_LEN) return 0;  /* shouldn't happen */
	memcpy(plainmembuf + plainmemoff, buf, len);
	plainmemoff += len;
	return len;
}

#endif /* #ifdef MEMCOMPR */

/*
vim:ts=4:sw=4:
*/
