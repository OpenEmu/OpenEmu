/*
 * cassette.c - cassette emulation
 *
 * Copyright (C) 2001 Piotr Fusik
 * Copyright (C) 2001-2011 Atari800 development team (see DOC/CREDITS)
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

#include "atari.h"
#include "cpu.h"
#include "cassette.h"
#include "esc.h"
#include "img_tape.h"
#include "log.h"
#include "util.h"
#include "pokey.h"

static IMG_TAPE_t *cassette_file = NULL;

/* Time till the end of the current tape event (byte or gap), in CPU ticks. */
static SLONG event_time_left = 0;

/* Indicates that there is a SERIN transmission in progress and when it ends,
   the current byte should be copied to POKEY_SERIN. This can be reset by
   rewinding/removing the tape or by resetting POKEY.
   Note that this variable has any meaning when PASSING_GAP is FALSE,
   so it doesn't have to be reset during PASSING_IRG. */
static int pending_serin = FALSE;

/* Indicates that an Inter-Record-Gap is currently being passed. It's set to TRUE
   at the beginning of each block. */
static int passing_gap = FALSE;

/* if penting_serin == TRUE, this holds the byte that is currently loaded from
   tape. It might be later copied to serin_byte. */
static UBYTE pending_serin_byte = 0xff;

/* Byte most recently loaded from tape; will be accessed by SIO_GetByte(). */
static UBYTE serin_byte = 0xff;

char CASSETTE_filename[FILENAME_MAX];
CASSETTE_status_t CASSETTE_status = CASSETTE_STATUS_NONE;
int CASSETTE_write_protect = FALSE;
int CASSETTE_record = FALSE;
static int cassette_writable = FALSE;
static int cassette_readable = FALSE;

char CASSETTE_description[CASSETTE_DESCRIPTION_MAX];
static int cassette_gapdelay = 0;	/* in ms, includes leader and all gaps */
static int cassette_motor = 0;

int CASSETTE_hold_start_on_reboot = 0;
int CASSETTE_hold_start = 0;
int CASSETTE_press_space = 0;
/* Indicates whether the tape has ended. During saving the value is always 0;
   during loading it is equal to (CASSETTE_GetPosition() >= CASSETTE_GetSize()). */
static int eof_of_tape = 0;

/* Call this function after each change of
   cassette_motor, CASSETTE_status or eof_of_tape. */
static void UpdateFlags(void)
{
	cassette_readable = cassette_motor &&
	                    (CASSETTE_status == CASSETTE_STATUS_READ_WRITE ||
	                     CASSETTE_status == CASSETTE_STATUS_READ_ONLY) &&
	                     !eof_of_tape;
	cassette_writable = cassette_motor &&
	                    CASSETTE_status == CASSETTE_STATUS_READ_WRITE &&
	                    !CASSETTE_write_protect;
}

int CASSETTE_ReadConfig(char *string, char *ptr)
{
	if (strcmp(string, "CASSETTE_FILENAME") == 0)
		Util_strlcpy(CASSETTE_filename, ptr, sizeof(CASSETTE_filename));
	else if (strcmp(string, "CASSETTE_LOADED") == 0) {
		int value = Util_sscanbool(ptr);
		if (value == -1)
			return FALSE;
		CASSETTE_status = (value ? CASSETTE_STATUS_READ_WRITE : CASSETTE_STATUS_NONE);
	}
	else if (strcmp(string, "CASSETTE_WRITE_PROTECT") == 0) {
		int value = Util_sscanbool(ptr);
		if (value == -1)
			return FALSE;
		CASSETTE_write_protect = value;
	}
	else return FALSE;
	return TRUE;
}

void CASSETTE_WriteConfig(FILE *fp)
{
	fprintf(fp, "CASSETTE_FILENAME=%s\n", CASSETTE_filename);
	fprintf(fp, "CASSETTE_LOADED=%d\n", CASSETTE_status != CASSETTE_STATUS_NONE);
	fprintf(fp, "CASSETTE_WRITE_PROTECT=%d\n", CASSETTE_write_protect);
}

int CASSETTE_Initialise(int *argc, char *argv[])
{
	int i;
	int j;
	int protect = FALSE; /* Is write-protect requested in command line? */

	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */

		if (strcmp(argv[i], "-tape") == 0) {
			if (i_a) {
				Util_strlcpy(CASSETTE_filename, argv[++i], sizeof(CASSETTE_filename));
				CASSETTE_status = CASSETTE_STATUS_READ_WRITE;
				/* Reset any write-protection read from config file. */
				CASSETTE_write_protect = FALSE;
			}
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-boottape") == 0) {
			if (i_a) {
				Util_strlcpy(CASSETTE_filename, argv[++i], sizeof(CASSETTE_filename));
				CASSETTE_status = CASSETTE_STATUS_READ_WRITE;
				/* Reset any write-protection read from config file. */
				CASSETTE_write_protect = FALSE;
				CASSETTE_hold_start = 1;
			}
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-tape-readonly") == 0)
			protect = TRUE;
		else {
			if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-tape <file>      Insert cassette image");
				Log_print("\t-boottape <file>  Insert cassette image and boot it");
				Log_print("\t-tape-readonly    Mark the attached cassette image as read-only");
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			return FALSE;
		}
	}

	*argc = j;

	/* If CASSETTE_status was set in this function or in CASSETTE_ReadConfig(),
	   then tape is to be mounted. */
	if (CASSETTE_status != CASSETTE_STATUS_NONE && CASSETTE_filename[0] != '\0') {
		/* Tape is mounted unprotected by default - overrun it if needed. */
		protect = protect || CASSETTE_write_protect;
		if (!CASSETTE_Insert(CASSETTE_filename)) {
			CASSETTE_status = CASSETTE_STATUS_NONE;
			Log_print("Cannot open cassette image %s", CASSETTE_filename);
		}
		else if (protect)
			CASSETTE_ToggleWriteProtect();
	}

	return TRUE;
}

void CASSETTE_Exit(void)
{
	CASSETTE_Remove();
}

int CASSETTE_Insert(const char *filename)
{
	int writable;
	char const *description;

	IMG_TAPE_t *file = IMG_TAPE_Open(filename, &writable, &description);
	if (file == NULL)
		return FALSE;

	CASSETTE_Remove();
	cassette_file = file;
	/* Guard against providing CASSETTE_filename as parameter. */
	if (CASSETTE_filename != filename)
		strcpy(CASSETTE_filename, filename);
	eof_of_tape = 0;

	CASSETTE_status = (writable ? CASSETTE_STATUS_READ_WRITE : CASSETTE_STATUS_READ_ONLY);
	event_time_left = 0;
	pending_serin = FALSE;
	passing_gap = FALSE;

	if (description != NULL)
		Util_strlcpy(CASSETTE_description, description, sizeof(CASSETTE_description));
	CASSETTE_write_protect = FALSE;
	CASSETTE_record = FALSE;
	UpdateFlags();
	cassette_gapdelay = 0;

	return TRUE;
}

void CASSETTE_Remove(void)
{
	if (cassette_file != NULL) {
		IMG_TAPE_Close(cassette_file);
		cassette_file = NULL;
	}
	CASSETTE_status = CASSETTE_STATUS_NONE;
	CASSETTE_description[0] = '\0';
	UpdateFlags();
}

int CASSETTE_CreateCAS(const char *filename, const char *description) {
	IMG_TAPE_t *file = IMG_TAPE_Create(filename, description);
	if (file == NULL)
		return FALSE;

	CASSETTE_Remove(); /* Unmount any previous tape image. */
	cassette_file = file;
	Util_strlcpy(CASSETTE_filename, filename, sizeof(CASSETTE_filename));
	if (description != NULL)
		Util_strlcpy(CASSETTE_description, description, sizeof(CASSETTE_description));
	CASSETTE_status = CASSETTE_STATUS_READ_WRITE;
	event_time_left = 0;
	pending_serin = FALSE;
	passing_gap = FALSE;
	cassette_gapdelay = 0;
	eof_of_tape = 0;
	CASSETTE_record = TRUE;
	CASSETTE_write_protect = FALSE;
	UpdateFlags();

	return TRUE;
}

unsigned int CASSETTE_GetPosition(void)
{
	if (cassette_file == NULL)
		return 0;
	return IMG_TAPE_GetPosition(cassette_file) + 1;
}

unsigned int CASSETTE_GetSize(void)
{
	if (cassette_file == NULL)
		return 0;
	return IMG_TAPE_GetSize(cassette_file);
}

void CASSETTE_Seek(unsigned int position)
{
	if (cassette_file != NULL) {
		if (position > 0)
			position --;
		IMG_TAPE_Seek(cassette_file, position);

		event_time_left = 0;
		pending_serin = FALSE;
		passing_gap = FALSE;
		eof_of_tape = 0;
		CASSETTE_record = FALSE;
		UpdateFlags();
	}
}

int CASSETTE_GetByte(void)
{
	return serin_byte;
}

int CASSETTE_IOLineStatus(void)
{
	/* if motor off and EOF return always 1 (equivalent the mark tone) */
	if (!cassette_readable || CASSETTE_record || passing_gap) {
		return 1;
	}

	return IMG_TAPE_SerinStatus(cassette_file, event_time_left);
}

void CASSETTE_PutByte(int byte)
{
	if (!ESC_enable_sio_patch && cassette_writable && CASSETTE_record)
		IMG_TAPE_WriteByte(cassette_file, byte, POKEY_AUDF[POKEY_CHAN3] + POKEY_AUDF[POKEY_CHAN4]*0x100);
}

void CASSETTE_TapeMotor(int onoff)
{
	if (cassette_motor != onoff) {
		if (CASSETTE_record && cassette_writable)
			/* Recording disabled, flush the tape */
			IMG_TAPE_Flush(cassette_file);
		cassette_motor = onoff;
		UpdateFlags();
	}
}

int CASSETTE_ToggleWriteProtect(void)
{
	if (CASSETTE_status != CASSETTE_STATUS_READ_WRITE)
		return FALSE;
	CASSETTE_write_protect = !CASSETTE_write_protect;
	UpdateFlags();
	return TRUE;
}

int CASSETTE_ToggleRecord(void)
{
	if (CASSETTE_status == CASSETTE_STATUS_NONE)
		return FALSE;
	CASSETTE_record = !CASSETTE_record;
	if (CASSETTE_record)
		eof_of_tape = FALSE;
	else if (cassette_writable)
		/* Recording disabled, flush the tape */
		IMG_TAPE_Flush(cassette_file);
	event_time_left = 0;
	pending_serin = FALSE;
	passing_gap = FALSE;
	UpdateFlags();
	/* Return FALSE to indicate that recording will not work. */
	return !CASSETTE_record || (CASSETTE_status == CASSETTE_STATUS_READ_WRITE && !CASSETTE_write_protect);
}

static void CassetteWrite(int num_ticks)
{
	if (cassette_writable)
		IMG_TAPE_WriteAdvance(cassette_file, num_ticks);
}

/* Sets the stamp of next SERIN IRQ event and loads new record if necessary.
   Returns TRUE if a new byte was loaded and POKEY_SERIN should be updated.
   The function assumes that current_block <= max_block. */
static int CassetteRead(int num_ticks)
{
	if (cassette_readable) {
		int loaded = FALSE; /* Function's return value */
		event_time_left -= num_ticks;
		while (event_time_left < 0) {
			unsigned int length;
			if (!passing_gap && pending_serin) {
				serin_byte = pending_serin_byte;
				/* A byte is loaded, return TRUE so it gets stored in POKEY_SERIN. */
				loaded = TRUE;
			}

			/* If POKEY is in reset state, no serial I/O occurs. */
			pending_serin = (POKEY_SKCTL & 0x03) != 0;

			if (!IMG_TAPE_Read(cassette_file, &length, &passing_gap, &pending_serin_byte)) {
				eof_of_tape = 1;
				UpdateFlags();
				return loaded;
			}

			event_time_left += length;
		}
		return loaded;
	}
	return FALSE;
}

int CASSETTE_AddScanLine(void)
{
	/* increment elapsed cassette time */
	if (CASSETTE_record) {
		CassetteWrite(114);
		return FALSE;
	} else
		return CassetteRead(114);
}

void CASSETTE_ResetPOKEY(void)
{
	/* Resetting POKEY stops any serial transmission. */
	pending_serin = FALSE;
	pending_serin_byte = 0xff;
}

/* --- Functions for loading/saving with SIO patch --- */

int CASSETTE_AddGap(int gaptime)
{
	cassette_gapdelay += gaptime;
	if (cassette_gapdelay < 0)
		cassette_gapdelay = 0;
	return cassette_gapdelay;
}

/* Indicates that a loading leader is expected by the OS */
void CASSETTE_LeaderLoad(void)
{
	if (CASSETTE_record)
		CASSETTE_ToggleRecord();
	CASSETTE_TapeMotor(TRUE);
	cassette_gapdelay = 9600;
	/* registers for SETVBV: third system timer, ~0.1 sec */
	CPU_regA = 3;
	CPU_regX = 0;
	CPU_regY = 5;
}

/* indicates that a save leader is written by the OS */
void CASSETTE_LeaderSave(void)
{
	if (!CASSETTE_record)
	CASSETTE_ToggleRecord();
	CASSETTE_TapeMotor(TRUE);
	cassette_gapdelay = 19200;
	/* registers for SETVBV: third system timer, ~0.1 sec */
	CPU_regA = 3;
	CPU_regX = 0;
	CPU_regY = 5;
}

int CASSETTE_ReadToMemory(UWORD dest_addr, int length)
{
	CASSETTE_TapeMotor(1);
	if (!cassette_readable)
		return 0;

	/* Convert wait_time to ms ( wait_time * 1000 / 1789790 ) and subtract. */
	cassette_gapdelay -= event_time_left / 1789; /* better accuracy not needed */
	if (!IMG_TAPE_SkipToData(cassette_file, cassette_gapdelay)) {
		/* Ignore the eventual error, assume it is the end of file */
		cassette_gapdelay = 0;
		eof_of_tape = 1;
		UpdateFlags();
		return 0;
	}
	cassette_gapdelay = 0;

	/* Load bytes */
	switch (IMG_TAPE_ReadToMemory(cassette_file, dest_addr, length)) {
	case TRUE:
		return TRUE;
	case -1: /* Read error/EOF */
		eof_of_tape = 1;
		UpdateFlags();
		/* FALLTHROUGH */
	default: /* case FALSE */
		return FALSE;
	}
}

int CASSETTE_WriteFromMemory(UWORD src_addr, int length)
{
	int result;
	CASSETTE_TapeMotor(1);
	if (!cassette_writable)
		return 0;

	result = IMG_TAPE_WriteFromMemory(cassette_file, src_addr, length, cassette_gapdelay);
	cassette_gapdelay = 0;
	return result;
}


/*
vim:ts=4:sw=4:
*/
