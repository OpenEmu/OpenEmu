/*
 * cassette.c - cassette emulation
 *
 * Copyright (C) 2001 Piotr Fusik
 * Copyright (C) 2001-2010 Atari800 development team (see DOC/CREDITS)
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

#include "atari.h"
#include "cpu.h"
#include "cassette.h"
#include "log.h"
#include "memory.h"
#include "sio.h"
#include "util.h"
#include "pokey.h"

#define MAX_BLOCKS 2048

static FILE *cassette_file = NULL;
static int cassette_isCAS;
UBYTE CASSETTE_buffer[4096];
static ULONG cassette_block_offset[MAX_BLOCKS];
static SLONG cassette_elapsedtime;  /* elapsed time since begin of file */
                                    /* in scanlines */
static SLONG cassette_savetime;	    /* helper for cas save */
static SLONG cassette_nextirqevent; /* timestamp of next irq in scanlines */

static char cassette_filename[FILENAME_MAX];
static char cassette_description[CASSETTE_DESCRIPTION_MAX];
static int cassette_current_blockbyte = 0;
static int cassette_current_block;
static int cassette_max_blockbytes = 0;
static int cassette_max_block = 0;
static int cassette_savefile = FALSE;
static int cassette_gapdelay = 0;	/* in ms, includes leader and all gaps */
static int cassette_putdelay = 0;	/* in ms, delay since last putbyte */
static int cassette_motor = 0;
static int cassette_baudrate = 600;
static int cassette_baudblock[MAX_BLOCKS];

int CASSETTE_hold_start_on_reboot = 0;
int CASSETTE_hold_start = 0;
int CASSETTE_press_space = 0;
static int eof_of_tape = 0;

typedef struct {
	char identifier[4];
	UBYTE length_lo;
	UBYTE length_hi;
	UBYTE aux_lo;
	UBYTE aux_hi;
} CAS_Header;
/*
Just for remembering - CAS format in short:
It consists of chunks. Each chunk has a header, possibly followed by data.
If a header is unknown or unexpected it may be skipped by the length of the
header (8 bytes), and additionally the length given in the length-field(s).
There are (until now) 3 types of chunks:
-CAS file marker, has to be at begin of file - identifier "FUJI", length is
number of characters of an (optional) ascii-name (without trailing 0), aux
is always 0.
-baud rate selector - identifier "baud", length always 0, aux is rate in baud
(usually 600; one byte is 8 bits + startbit + stopbit, makes 60 bytes per
second).
-data record - identifier "data", length is length of the data block (usually
$84 as used by the OS), aux is length of mark tone (including leader and gaps)
just before the record data in milliseconds.
*/

int CASSETTE_Initialise(int *argc, char *argv[])
{
	int i;
	int j;

	strcpy(cassette_filename, "None");
	memset(cassette_description, 0, sizeof(cassette_description));

	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */

		if (strcmp(argv[i], "-tape") == 0) {
			if (i_a)
				CASSETTE_Insert(argv[++i]);
			else a_m = TRUE;
		}
		else if (strcmp(argv[i], "-boottape") == 0) {
			if (i_a) {
				CASSETTE_Insert(argv[++i]);
				CASSETTE_hold_start = 1;
			}
			else a_m = TRUE;
		}
		else {
			if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-tape <file>     Insert cassette image");
				Log_print("\t-boottape <file> Insert cassette image and boot it");
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			return FALSE;
		}
	}

	*argc = j;

	return TRUE;
}

/* Gets information about the cassette image. Returns TRUE if ok.
   To get information without attaching file, use:
   char description[CASSETTE_DESCRIPTION_MAX];
   int last_block;
   int isCAS;
   CASSETTE_CheckFile(filename, NULL, description, &last_block, &isCAS);
*/
int CASSETTE_CheckFile(const char *filename, FILE **fp, char *description, int *last_block, int *isCAS)
{
	FILE *f;
	CAS_Header header;
	int blocks;

/* atm, no appending to existing cas files - too dangerous to data!
   and will not work anyway without patched sio
	f = fopen(filename, "rb+");
	if (f == NULL) */
		f = fopen(filename, "rb");
	if (f == NULL)
		return FALSE;
	if (description)
		memset(description, 0, CASSETTE_DESCRIPTION_MAX);

	if (fread(&header, 1, 6, f) == 6
		&& header.identifier[0] == 'F'
		&& header.identifier[1] == 'U'
		&& header.identifier[2] == 'J'
		&& header.identifier[3] == 'I') {
		/* CAS file */
		UWORD length;
		UWORD skip;
		if (isCAS)
			*isCAS = TRUE;
		fseek(f, 2L, SEEK_CUR);	/* ignore the aux bytes */

		/* read or skip file description */
		skip = length = header.length_lo + (header.length_hi << 8);
		if (description) {
			if (length < CASSETTE_DESCRIPTION_MAX)
				skip = 0;
			else
				skip -= CASSETTE_DESCRIPTION_MAX - 1;
			if (fread(description, 1, length - skip, f) < (length - skip)) {
				Log_print("Error reading cassette file.\n");
			}
		}
		fseek(f, skip, SEEK_CUR);

		/* count number of blocks */
		blocks = 0;
		cassette_baudblock[0]=600;
		do {
			/* chunk header is always 8 bytes */
			if (fread(&header, 1, 8, f) != 8)
				break;
			if (header.identifier[0] == 'b'
			 && header.identifier[1] == 'a'
			 && header.identifier[2] == 'u'
			 && header.identifier[3] == 'd') {
				cassette_baudrate=header.aux_lo + (header.aux_hi << 8);
			}
			else if (header.identifier[0] == 'd'
			 && header.identifier[1] == 'a'
			 && header.identifier[2] == 't'
			 && header.identifier[3] == 'a') {
				blocks++;
				if (fp) {
					cassette_block_offset[blocks] = ftell(f) - 4;
					cassette_baudblock[blocks] = cassette_baudrate;
				}
			}
			length = header.length_lo + (header.length_hi << 8);
			/* skip possibly present data block */
			fseek(f, length, SEEK_CUR);
		} while (blocks < MAX_BLOCKS);
	}
	else {
		/* raw file */
		int file_length = Util_flen(f);
		blocks = ((file_length + 127) >> 7) + 1;
		if (isCAS)
			*isCAS = FALSE;
	}
	if (last_block)
		*last_block = blocks;
	/* if filelength is 0, then it is a savefile */
	fseek(f,0L,SEEK_SET);
	if (fread(&header, 1, 4, f) == 0) {
		cassette_savefile = TRUE;
	}
	if (fp)
		*fp = f;
	else
		fclose(f);
	return TRUE;
}

/* Create CAS file header for saving data to tape */
int CASSETTE_CreateFile(const char *filename, FILE **fp, int *isCAS)
{
	CAS_Header header;
	memset(&header, 0, sizeof(header));

	/* if no file pointer location was given, this function is senseless */
	if (fp == NULL)
		return FALSE;
	/* close if open */
	if (*fp != NULL)
		fclose(*fp);
	/* create new file */
	*fp = fopen(filename, "wb");
	if (*fp == NULL)
		return FALSE;

	/* write CAS-header */
	{
		size_t desc_len = strlen(cassette_description);
		header.length_lo = (UBYTE) desc_len;
		header.length_hi = (UBYTE) (desc_len >> 8);
		if (fwrite("FUJI", 1, 4, *fp) == 4
		 && fwrite(&header.length_lo, 1, 4, *fp) == 4
		 && fwrite(cassette_description, 1, desc_len, *fp) == desc_len) {
			/* ok */
		}
		else {
			fclose(*fp);
			*fp = NULL;
			return FALSE;
		}
	}

	memset(&header, 0, sizeof(header));
	header.aux_lo = cassette_baudrate & 0xff;	/* provisional: 600 baud */
	header.aux_hi = cassette_baudrate >> 8;
	if ((fwrite("baud", 1, 4, *fp) == 4)
	 && (fwrite(&header.length_lo, 1, 4, *fp) == 4)) {
		/* ok */
	}
	else {
		fclose(*fp);
		*fp = NULL;
		return FALSE;
	};

	/* file is now a cassette file */
	*isCAS = TRUE;
	return TRUE;
}

int CASSETTE_Insert(const char *filename)
{
	CASSETTE_Remove();
	strcpy(cassette_filename, filename);
	cassette_elapsedtime = 0;
	cassette_savetime = 0;
	cassette_nextirqevent = 0;
	cassette_current_blockbyte = 0;
	cassette_max_blockbytes = 0;
	cassette_current_block = 1;
	cassette_gapdelay = 0;
	eof_of_tape = 0;
	cassette_savefile = FALSE;
	cassette_baudrate = 600;
	return CASSETTE_CheckFile(filename, &cassette_file, cassette_description,
		 &cassette_max_block, &cassette_isCAS);
}

void CASSETTE_Remove(void)
{
	if (cassette_file != NULL) {
		/* save last block, ignore trailing space tone */
		if ((cassette_current_blockbyte > 0) && cassette_savefile)
			CASSETTE_Write(cassette_current_blockbyte);

		fclose(cassette_file);
		cassette_file = NULL;
	}
	cassette_max_block = 0;
	strcpy(cassette_filename, "None");
	memset(cassette_description, 0, sizeof(cassette_description));
}

/* Read a record by SIO-patch
   returns block length (with checksum) */
static int ReadRecord_SIO(void)
{
	int length = 0;
	if (cassette_isCAS) {
		CAS_Header header;
		/* if waiting for gap was longer than gap of record, skip
		   atm there is no check if we start then inmidst a record */
		int filegaptimes = 0;
		while (cassette_gapdelay >= filegaptimes) {
			if (cassette_current_block > cassette_max_block) {
				length = -1;
				break;
			};
			fseek(cassette_file, cassette_block_offset[cassette_current_block], SEEK_SET);
			if (fread(&header.length_lo, 1, 4, cassette_file) < 4) {
				Log_print("Error reading cassette file.\n");
			}
			length = header.length_lo + (header.length_hi << 8);
			/* add gaplength */
			filegaptimes += header.aux_lo + (header.aux_hi << 8);
			/* add time used by the data themselves
			   a byte is encoded into 10 bits */
			filegaptimes += length * 10 * 1000 / cassette_baudblock[cassette_current_block];

			if (fread(&CASSETTE_buffer[0], 1, length, cassette_file) < length) {
				Log_print("Error reading cassette file.\n");
			}
			cassette_current_block++;
		}
		cassette_gapdelay = 0;
	}
	else {
		length = 132;
		CASSETTE_buffer[0] = 0x55;
		CASSETTE_buffer[1] = 0x55;
		if (cassette_current_block >= cassette_max_block) {
			/* EOF record */
			CASSETTE_buffer[2] = 0xfe;
			memset(CASSETTE_buffer + 3, 0, 128);
		}
		else {
			int bytes;
			fseek(cassette_file, (cassette_current_block - 1) * 128, SEEK_SET);
			bytes = fread(CASSETTE_buffer + 3, 1, 128, cassette_file);
			if (bytes < 128) {
				CASSETTE_buffer[2] = 0xfa;	/* non-full record */
				memset(CASSETTE_buffer + 3 + bytes, 0, 127 - bytes);
				CASSETTE_buffer[0x82] = bytes;
			}
			else
				CASSETTE_buffer[2] = 0xfc;	/* full record */
		}
		CASSETTE_buffer[0x83] = SIO_ChkSum(CASSETTE_buffer, 0x83);
		cassette_current_block++;
	}
	return length;
}

static UWORD WriteRecord(int len)
{
	CAS_Header header;
	memset(&header, 0, sizeof(header));

	/* always append */
	fseek(cassette_file, 0, SEEK_END);
	/* write recordheader */
	Util_strncpy(header.identifier, "data", 4);
	header.length_lo = len & 0xFF;
	header.length_hi = (len >> 8) & 0xFF;
	header.aux_lo = cassette_gapdelay & 0xff;
	header.aux_hi = (cassette_gapdelay >> 8) & 0xff;
	cassette_gapdelay = 0;
	fwrite(&header, 1, 8, cassette_file);
	cassette_max_block++;
	cassette_current_block++;
	/* write record */
	return fwrite(&CASSETTE_buffer, 1, len, cassette_file);
}

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
	cassette_gapdelay = 9600;
	/* registers for SETVBV: third system timer, ~0.1 sec */
	CPU_regA = 3;
	CPU_regX = 0;
	CPU_regY = 5;
}

/* indicates that a save leader is written by the OS */
void CASSETTE_LeaderSave(void)
{
	cassette_gapdelay = 19200;
	/* registers for SETVBV: third system timer, ~0.1 sec */
	CPU_regA = 3;
	CPU_regX = 0;
	CPU_regY = 5;
}

/* Read Cassette Record from Storage medium
  returns size of data in buffer (atm equal with record size, but there
    are protections with variable baud rates imaginable where a record
    must be split and a baud chunk inserted inbetween) or -1 for error */
int CASSETTE_Read(void)
{
	/* no file or blockpositions don't match anymore after saving */
	if ((cassette_file == NULL) || (cassette_savefile == TRUE))
		return -1;
	if (cassette_current_block > cassette_max_block)
		return -1; /* EOF */
	return ReadRecord_SIO();
}

/* Write Cassette Record to Storage medium
  length is size of the whole data with checksum(s)
  returns really written bytes, -1 for error */
int CASSETTE_Write(int length)
{
	/* there must be a filename given for saving */
	if (strcmp(cassette_filename, "None") == 0)
		return -1;
	/* if file doesn't exist (or has no records), create the header */
	if ((cassette_file == NULL || ftell(cassette_file) == 0) &&
			(CASSETTE_CreateFile(cassette_filename,
			&cassette_file,&cassette_isCAS) == FALSE))
		return -1;
	/* on a raw file, saving is denied because it can hold
	    only 1 file and could cause confusion */
	if (cassette_isCAS == FALSE)
		return -1;
	/* save record (always appends to the end of file) */
	cassette_savefile = TRUE;
	return WriteRecord(length);
}

/* convert milliseconds to scanlines */
static SLONG MSToScanLines(int ms)
{
	/* for PAL resolution, deviation in NTSC is negligible */
	return 312*50*ms/1000;
}

/* Support to read a record by POKEY-registers
   evals gap length
   returns block length (with checksum) */
static int ReadRecord_POKEY(void)
{
	int length = 0;
	if (cassette_isCAS) {
		CAS_Header header;
		/* no file or blockpositions don't match anymore after saving */
		if ((cassette_file == NULL) || (cassette_savefile == TRUE)) {
			cassette_nextirqevent = -1;
			length = -1;
			return length;
		}
		/* if end of CAS file */
		if (cassette_current_block > cassette_max_block){
			cassette_nextirqevent = -1;
			length = -1;
			eof_of_tape = 1;
			return length;
		}
		else {
			fseek(cassette_file, cassette_block_offset[
				cassette_current_block], SEEK_SET);
			if (fread(&header.length_lo, 1, 4, cassette_file) < 4) {
				Log_print("Error reading cassette file.\n");
			}
			length = header.length_lo + (header.length_hi << 8);
			/* eval total length as pregap + 1 byte */
			cassette_nextirqevent = cassette_elapsedtime +
				MSToScanLines(
				header.aux_lo + (header.aux_hi << 8)
				+ 10 * 1000 / cassette_baudblock[
				cassette_current_block]);
			/* read block into buffer */
			if (fread(&CASSETTE_buffer[0], 1, length, cassette_file) < length) {
				Log_print("Error reading cassette file.\n");
			}
			cassette_max_blockbytes = length;
			cassette_current_blockbyte = 0;
			cassette_current_block++;
		}
	}
	else {
		length = 132;
		CASSETTE_buffer[0] = 0x55;
		CASSETTE_buffer[1] = 0x55;
		if (cassette_current_block >= cassette_max_block) {
			/* EOF record */
			CASSETTE_buffer[2] = 0xfe;
			memset(CASSETTE_buffer + 3, 0, 128);
			if (cassette_current_block > cassette_max_block) {
				eof_of_tape = 1;
			}
		}
		else {
			int bytes;
			fseek(cassette_file,
				(cassette_current_block - 1) * 128, SEEK_SET);
			bytes = fread(CASSETTE_buffer + 3, 1, 128,
				cassette_file);
			if (bytes < 128) {
				CASSETTE_buffer[2] = 0xfa; /* non-full record */
				memset(CASSETTE_buffer + 3 + bytes, 0,
					127 - bytes);
				CASSETTE_buffer[0x82] = bytes;
			}
			else
				CASSETTE_buffer[2] = 0xfc; /* full record */
		}
		CASSETTE_buffer[0x83] = SIO_ChkSum(CASSETTE_buffer, 0x83);
		/* eval time to first byte coming out of pokey */
		if (cassette_current_block == 1) {
			/* on first block, length includes a leader */
			cassette_nextirqevent = cassette_elapsedtime +
				MSToScanLines(19200
				+ 10 * 1000 / 600); /* always 600 baud */
		}
		else {
			cassette_nextirqevent = cassette_elapsedtime +
				MSToScanLines(260
				+ 10 * 1000 / 600); /* always 600 baud */
		}
		cassette_max_blockbytes = length;
		cassette_current_blockbyte = 0;
		cassette_current_block++;
	}
	return length;
}

/* sets the stamp of next irq and loads new record if necessary
   adjust is a value to correction of time of next irq*/
static int SetNextByteTime_POKEY(int adjust)
{
	int length = 0;
	cassette_current_blockbyte += 1;
	/* if there are still bytes in the buffer, take next byte */
	if (cassette_current_blockbyte < cassette_max_blockbytes) {
		cassette_nextirqevent = cassette_elapsedtime + adjust
			+ MSToScanLines(10 * 1000 / (cassette_isCAS?cassette_baudblock[
			cassette_current_block-1]:600));
		return 0;
	}

	/* if buffer is exhausted, load next record */
	length = ReadRecord_POKEY();
	/* if failed, return -1 */
	if (length == -1) {
		cassette_nextirqevent = -1;
		return -1;
	}
	cassette_nextirqevent += adjust;
	return 0;
}

/* Get the byte for which the serial data ready int has been triggered */
int CASSETTE_GetByte(void)
{
	/* there are programs which load 2 blocks as one */
	return CASSETTE_buffer[cassette_current_blockbyte];
}

/* Check status of I/O-line
  Mark tone and stop bit gives 1, start bit 0
  $55 as data give 1,0,1,0,1,0,1,0 (sync to evaluate tape speed,
    least significant bit first)
  returns state of I/O-line as block.byte.bit */
int CASSETTE_IOLineStatus(void)
{
	int bit = 0;

	/* if motor off and EOF return always 1 (equivalent the mark tone) */
	if ((cassette_motor == 0)
		|| (eof_of_tape != 0)) {
		return 1;
	}

	/* exam rate; if elapsed time > nextirq - duration of one byte */
	if (cassette_elapsedtime >
		(cassette_nextirqevent - 10 * 50 * 312 / 
		(cassette_isCAS?cassette_baudblock[cassette_current_block-1]:
		600) + 1)) {
		bit = (cassette_nextirqevent - cassette_elapsedtime)
			/ (50 * 312 / (cassette_isCAS?cassette_baudblock[
			cassette_current_block-1]:600));
	}
	else {
		bit = 0;
	}

	/* if stopbit or out of range, return mark tone */
	if ((bit <= 0) || (bit > 9))
		return 1;

	/* if start bit, return space tone */
	if (bit == 9)
		return 0;

	/* eval tone to return */
	return (CASSETTE_GetByte() >> (8 - bit)) & 1;
}

/* Get the delay to trigger the next interrupt
   remark: The I/O-Line-status is also evaluated on this basis */
int CASSETTE_GetInputIRQDelay(void)
{
	int timespan;

	/* if no file or eof or motor off, return zero */
	if ((cassette_file == NULL)
		|| (eof_of_tape != 0)
		|| (cassette_motor == 0)
		|| (cassette_nextirqevent < 0)
		|| cassette_savefile) {
		return 0;
	}

	/* return time span */
	timespan = cassette_nextirqevent - cassette_elapsedtime;
	/* if timespan is negative, eval timespan to next byte */
	if (timespan <= 0) {
		if (SetNextByteTime_POKEY(0) == -1) {
			return -1;
		}
		/* return time span */
		timespan = cassette_nextirqevent - cassette_elapsedtime;
	}
	if (timespan < 40) {
		timespan += ((312 * 50 - 1) / (cassette_isCAS?cassette_baudblock[
			cassette_current_block-1]:600)) * 10;
	}

	/* if still negative, return "failed" */
	if (timespan < 0) {
		timespan = -1;
	}
	return timespan;
}

/* return if save file */
int CASSETTE_IsSaveFile(void)
{
	return cassette_savefile;
}

/* put a byte into the cas file */
/* the block is being written at first putbyte of the subsequent block */
void CASSETTE_PutByte(int byte)
{
	/* get time since last byte-put resp. motor on */
	cassette_putdelay = 1000*(cassette_elapsedtime-cassette_savetime)/312/50;
	/* subtract one byte-duration from put delay */
	cassette_putdelay -= 1000 * 10 * (POKEY_AUDF[POKEY_CHAN3] + POKEY_AUDF[POKEY_CHAN4]*0x100)/895000;
	/* if putdelay > 5 (ms) */
	if (cassette_putdelay > 05) {

		/* write previous block */
		if (cassette_current_blockbyte > 0) {
			CASSETTE_Write(cassette_current_blockbyte);
			cassette_current_blockbyte = 0;
			cassette_gapdelay = 0;
		}
		/* set new gap-time */
		cassette_gapdelay += cassette_putdelay;
	}
	/* put byte into buffer */
	CASSETTE_buffer[cassette_current_blockbyte] = byte;
	cassette_current_blockbyte++;
	/* set new last byte-put time */
	cassette_savetime = cassette_elapsedtime;
}

/* set motor status
 1 - on, 0 - off
 remark: the real evaluation is done in AddScanLine*/
void CASSETTE_TapeMotor(int onoff)
{
	if (cassette_file != NULL) {
		if (cassette_savefile) { /* cas save */
			if (onoff != cassette_motor) {
				if (onoff == 0) {
					/* get time since last byte-put resp. motor on */
					cassette_putdelay = 1000*(cassette_elapsedtime-cassette_savetime)/312/50;
					/* subtract one byte-duration from put delay */
					cassette_putdelay -= 1000*10 * (POKEY_AUDF[POKEY_CHAN3] + POKEY_AUDF[POKEY_CHAN4]*0x100)/895000;
					/* set putdelay non-negative */
        				if (cassette_putdelay < 0) {
						cassette_putdelay = 0;
					}
					/* write block from buffer */
					if (cassette_current_blockbyte > 0) {
						CASSETTE_Write(cassette_current_blockbyte);
						cassette_gapdelay = 0;
						cassette_current_blockbyte = 0;
					}
					/* set new gap-time */
					cassette_gapdelay += cassette_putdelay;

				}
				else {
					/* motor on if it was off - restart gap time */
					cassette_savetime = cassette_elapsedtime;
				}
				cassette_motor = onoff;
			}
		}
		else {
			cassette_motor = onoff;
			/* restart gap time */
			if (onoff == 1)
				cassette_savetime = cassette_elapsedtime;
		};
	}
}

void CASSETTE_AddScanLine(void)
{
	int tmp;
	/* if motor on and a cassette file is opened, and not eof */
	/* increment elapsed cassette time */
	if (cassette_motor && (cassette_file != NULL)
		&& ((eof_of_tape == 0) || cassette_savefile)) {
		cassette_elapsedtime++;

		/* only for loading: set next byte interrupt time */
		/* valid cassette times are up to 870 baud, giving
		   a time span of 18 scanlines, so comparing with
		   -2 leaves a safe timespan for letting get the bit out
		   of the pokey */
		if (((cassette_nextirqevent - cassette_elapsedtime) <= -2)
			&& !cassette_savefile) {
			tmp = SetNextByteTime_POKEY(-2);
		}
	}
}

/*
vim:ts=4:sw=4:
*/
