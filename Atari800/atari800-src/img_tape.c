/*
 * img_tape.c - support for CAS and raw tape images
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "atari.h"
#include "cassette.h"
#include "img_tape.h"
#include "memory.h"
#include "sio.h"
#include "util.h"

enum { MAX_BLOCKS = 2048 };

/* Standard record length, needed by ReadRecord() when reading raw files */
enum { DEFAULT_BUFFER_SIZE = 132 };

/* Baudrate for all written blocks and for reading from raw files. */
enum { DEFAULT_BAUDRATE = 600 };

struct IMG_TAPE_t {
	FILE *file; /* Stream for reading/writing of the tape image */
	int isCAS; /* Indicates if the file is in CAS format, or a raw binary file */
	UBYTE *buffer; /* Holds bytes of the last read or currently written data block */
	size_t buffer_size; /* Size of the space allocated for BUFFER */
	ULONG savetime; /* Time elapsed since last byte writing, in CPU ticks */
	ULONG save_gap; /* Length of the IRG before the currently written block */
	int next_blockbyte; /* Index of the byte in this block that will be read next (counted from 0) */
	unsigned int current_block; /* Number of the currently-read/written block (counted from 0) */
	int block_length; /* Length of the block currently held in BUFFER */
	int num_blocks; /* Number of data blocks in the whole file */
	ULONG block_offsets[MAX_BLOCKS]; /* File offsets for each data block*/
	int block_baudrates[MAX_BLOCKS]; /* Baudrates for each data block in the file */
	char description[CASSETTE_DESCRIPTION_MAX]; /* Tape description, only for CAS files */
	int was_writing; /* Indicated if the last operation on the file was writing */
};

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

int IMG_TAPE_FileSupported(UBYTE const start_bytes[4])
{
	/* Note: doesn't detect raw binary files. */
	return start_bytes[0] == 'F' && start_bytes[1] == 'U'
	    && start_bytes[2] == 'J' && start_bytes[3] == 'I';
}

/* Write contents of the file's block buffer to file, as a separate record;
   then empty the buffer.
   Returns TRUE on success or FALSE on write error. */
static int WriteRecord(IMG_TAPE_t *file)
{
	CAS_Header header;
	int result;

	/* on a raw file, saving is denied because it can hold
	    only 1 file and could cause confusion */
	if (!file->isCAS)
		return FALSE;
	/* always append */
	if (fseek(file->file, file->block_offsets[file->num_blocks], SEEK_SET) != 0)
		return FALSE;
	/* write record header */
	Util_strncpy(header.identifier, "data", 4);
	header.length_lo = file->block_length & 0xFF;
	header.length_hi = (file->block_length >> 8) & 0xFF;
	header.aux_lo = file->save_gap & 0xff;
	header.aux_hi = (file->save_gap >> 8) & 0xff;
	if (fwrite(&header, 1, 8, file->file) != 8)
		return FALSE;
	/* Saving is supported only with standard baudrate. */
	file->block_baudrates[file->num_blocks] = DEFAULT_BAUDRATE;
	file->num_blocks++;
	file->block_offsets[file->num_blocks] = file->block_offsets[file->num_blocks - 1] + file->block_length + 8;
	file->current_block = file->num_blocks;
	/* write record */
	result = fwrite(file->buffer, 1, file->block_length, file->file) == file->block_length;
	if (result) {
		file->save_gap = 0;
		file->block_length = 0;
	}
	return result;
}

/* Flush any unwritten data to tape. */
static int CassetteFlush(IMG_TAPE_t *file)
{
	if (file->block_length > 0)
		return WriteRecord(file) && fflush(file->file) == 0;
	return TRUE;
}

IMG_TAPE_t *IMG_TAPE_Open(char const *filename, int *writable, char const **description)
{
	IMG_TAPE_t *img;
	CAS_Header header;

	img = Util_malloc(sizeof(IMG_TAPE_t));
	/* Check if the file is writable. If not, recording will be disabled. */
	img->file = fopen(filename, "rb+");
	*writable = img->file != NULL;
	/* If opening for reading+writing failed, reopen it as read-only. */
	if (img->file == NULL)
		img->file = fopen(filename, "rb");
	if (img->file == NULL) {
		free(img);
		return NULL;
	}
	img->description[0] = '\0';

	if (fread(&header, 1, 6, img->file) == 6
		&& header.identifier[0] == 'F'
		&& header.identifier[1] == 'U'
		&& header.identifier[2] == 'J'
		&& header.identifier[3] == 'I') {
		/* CAS file */
		UWORD length;
		UWORD skip;
		int blocks;
		int baudrate = DEFAULT_BAUDRATE;

		img->isCAS = TRUE;
		fseek(img->file, 2L, SEEK_CUR);	/* ignore the aux bytes */

		/* read or skip file description */
		skip = length = header.length_lo | (header.length_hi << 8);
		if (length < CASSETTE_DESCRIPTION_MAX)
			skip = 0;
		else
			skip -= CASSETTE_DESCRIPTION_MAX - 1;
		if (fread(img->description, 1, length - skip, img->file) < (length - skip)) {
			fclose(img->file);
			free(img);
			return NULL;
		}
		img->description[length - skip] = '\0';
		fseek(img->file, skip, SEEK_CUR);

		/* count number of blocks */
		blocks = 0;
		img->block_baudrates[0] = DEFAULT_BAUDRATE;
		img->block_offsets[0] = ftell(img->file);
		for (;;) {
			/* chunk header is always 8 bytes */
			if (fread(&header, 1, 8, img->file) != 8)
				break;
			length = header.length_lo + (header.length_hi << 8);
			if (header.identifier[0] == 'b'
			 && header.identifier[1] == 'a'
			 && header.identifier[2] == 'u'
			 && header.identifier[3] == 'd') {
				baudrate=header.aux_lo + (header.aux_hi << 8);
				img->block_offsets[blocks] += length + 8;
			}
			else if (header.identifier[0] == 'd'
			 && header.identifier[1] == 'a'
			 && header.identifier[2] == 't'
			 && header.identifier[3] == 'a') {
				img->block_baudrates[blocks] = baudrate;
				if (++blocks >= MAX_BLOCKS) {
					--blocks;
					break;
				}
				img->block_offsets[blocks] = img->block_offsets[blocks - 1] + length + 8;
			}
			/* skip possibly present data block */
			fseek(img->file, length, SEEK_CUR);
		}
		img->num_blocks = blocks;
		*description = img->description;
	}
	else {
		/* raw file */
		int file_length = Util_flen(img->file);
		img->num_blocks = ((file_length + 127) >> 7) + 1;
		img->isCAS = FALSE;
		*writable = FALSE; /* Writing raw files is not supported */
		*description = NULL;
	}

	img->savetime = 0;
	img->save_gap = 0;
	img->next_blockbyte = 0;
	img->block_length = 0;
	img->current_block = 0;
	img->buffer = Util_malloc((img->buffer_size = DEFAULT_BUFFER_SIZE) * sizeof(UBYTE));
	img->was_writing = FALSE;

	return img;
}

void IMG_TAPE_Close(IMG_TAPE_t *file)
{
	if (file->was_writing)
		CassetteFlush(file);
	fclose(file->file);
	free(file->buffer);
	free(file);
}

IMG_TAPE_t *IMG_TAPE_Create(char const *filename, char const *description)
{
	IMG_TAPE_t *img;
	CAS_Header header;
	size_t desc_len;
	FILE *file = NULL;

	/* create new file */
	file = fopen(filename, "wb+");
	if (file == NULL)
		return NULL;

	/* Write the initial FUJI and baud blocks of the CAS file. */
	desc_len = strlen(description);
	memset(&header, 0, sizeof(header));
	/* write CAS-header */
	header.length_lo = (UBYTE) desc_len;
	header.length_hi = (UBYTE) (desc_len >> 8);
	if (fwrite("FUJI", 1, 4, file) != 4
	    || fwrite(&header.length_lo, 1, 4, file) != 4
	    || fwrite(description, 1, desc_len, file) != desc_len) {
		fclose(file);
		return NULL;
	}

	memset(&header, 0, sizeof(header));
	/* All records are written with 600 baud speed. */
	header.aux_lo = DEFAULT_BAUDRATE & 0xff;
	header.aux_hi = DEFAULT_BAUDRATE >> 8;
	if (fwrite("baud", 1, 4, file) != 4
	    || fwrite(&header.length_lo, 1, 4, file) != 4) {
		fclose(file);
		return NULL;
	}

	img = Util_malloc(sizeof(IMG_TAPE_t));
	img->file = file;
	if (description != NULL)
		Util_strlcpy(img->description, description, CASSETTE_DESCRIPTION_MAX);
	img->isCAS = TRUE;
	img->savetime = 0;
	img->save_gap = 0;
	img->next_blockbyte = 0;
	img->block_length = 0;
	img->current_block = 0;
	img->num_blocks = 0;
	img->block_offsets[0] = strlen(description) + 16;
	img->buffer = Util_malloc((img->buffer_size = DEFAULT_BUFFER_SIZE) * sizeof(UBYTE));
	img->was_writing = TRUE;

	return img;
}

/* Enlarge file->buffer to (at least) SIZE if needed. */
static void EnlargeBuffer(IMG_TAPE_t *file, size_t size)
{
	if (file->buffer_size < size) {
		/* Enlarge the buffer at least 2 times. */
		file->buffer_size *= 2;
		if (file->buffer_size < size)
			file->buffer_size = size;
		file->buffer = Util_realloc(file->buffer, file->buffer_size * sizeof(UBYTE));
	}
}

/* Read a record from the file. FALSE on error/EOF.
   Writes length of pre-record gap (in ms) into *gap. */
static int ReadNextRecord(IMG_TAPE_t *file, int *gap)
{
	int length;

	/* 0 indicates that there was no previous block being read and
	   current_block already contains the current block number. */
	if (file->block_length != 0) {
		/* Non-zero - a block was being read and it's finished, increase the block number. */
		file->block_length = 0;
		if (++file->current_block >= file->num_blocks)
			/* Last block was already read. */
			return FALSE;
	}

	if (file->isCAS) {
		CAS_Header header;

		/* While reading a block, offset by 4 to skip its 4-byte name (assume it's "data") */
		if (fseek(file->file, file->block_offsets[file->current_block] + 4, SEEK_SET) != 0
		    || fread(&header.length_lo, 1, 4, file->file) < 4)
			return FALSE;
		length = header.length_lo + (header.length_hi << 8);
		*gap = header.aux_lo + (header.aux_hi << 8);
		/* read block into buffer */
		EnlargeBuffer(file, length);
		if (fread(file->buffer, 1, length, file->file) < length)
			return FALSE;
	}
	else {
		length = 132;
		/* Don't enlarge buffer - its default size is at least 132. */
		*gap = (file->current_block == 0 ? 19200 : 260);
		file->buffer[0] = 0x55;
		file->buffer[1] = 0x55;
		if (file->current_block + 1 >= file->num_blocks) {
			/* EOF record */
			file->buffer[2] = 0xfe;
			memset(file->buffer + 3, 0, 128);
		}
		else {
			int bytes;
			if (fseek(file->file, file->current_block * 128, SEEK_SET) != 0
			    || (bytes = fread(file->buffer + 3, 1, 128, file->file)) == 0)
				return FALSE;
			if (bytes < 128) {
				file->buffer[2] = 0xfa; /* non-full record */
				memset(file->buffer + 3 + bytes, 0, 127 - bytes);
				file->buffer[0x82] = bytes;
			}
			else
				file->buffer[2] = 0xfc;	/* full record */
		}
		file->buffer[0x83] = SIO_ChkSum(file->buffer, 0x83);
	}
	file->block_length = length;
	return TRUE;
}

int IMG_TAPE_Read(IMG_TAPE_t *file, unsigned int *duration, int *is_gap, UBYTE *byte)
{
	int gap;
	if (file->was_writing) {
		CassetteFlush(file);
		file->was_writing = FALSE;
	}
	if (file->next_blockbyte >= file->block_length) {
		/* Buffer is exhausted, load next record. */

		if (!ReadNextRecord(file, &gap))
			return FALSE;
		file->next_blockbyte = 0;
		if (gap > 0) {
			/* Convert gap from ms to CPU ticks. */
			*duration = gap * 1789 + gap * 790 / 1000; /* (gap * 1789790 / 1000), avoiding overflow */
			*is_gap = TRUE;
			return TRUE;
		}
	}
	*byte = file->buffer[file->next_blockbyte++];
	*is_gap = FALSE;
	/* Next event will be after 10 bits of data gets loaded. */
	*duration = 10 * 1789790 / (file->isCAS ? file->block_baudrates[file->current_block] : 600);
	return TRUE;
}

void IMG_TAPE_WriteAdvance(IMG_TAPE_t *file, unsigned int num_ticks)
{
	if (!file->was_writing) {
		file->savetime = 0;
		file->save_gap = 0;
		file->next_blockbyte = 0;
		file->block_length = 0;
		file->was_writing = TRUE;
		/* Always append to end of file. */
		file->current_block = file->num_blocks;
	}
	file->savetime += num_ticks;
}

int IMG_TAPE_WriteByte(IMG_TAPE_t *file, UBYTE byte, unsigned int pokey_counter)
{
	/* put_delay is time between end of last byte write / motor start, and
	   start of writing of current BYTE (in ms). */
	/* Note: byte duration in seconds: pokey_counter / (1789790/2) * 10
	 * in milliseconds: pokey_counter * 10 * 1000 / 1789790/2 */
	int put_delay = file->savetime /1790 - 10 * pokey_counter / 895; /* better accuracy not needed */
	if (put_delay > 05) {

		/* write previous block */
		if (file->block_length > 0) {
			if (!WriteRecord(file))
				return FALSE; /* Write error */
		}
		/* set new gap-time */
		file->save_gap += put_delay;
	}
	/* put byte into buffer */
	EnlargeBuffer(file, file->block_length + 1);
	file->buffer[file->block_length++] = byte;
	/* set new last byte-put time */
	file->savetime = 0;

	return TRUE;
}

int IMG_TAPE_Flush(IMG_TAPE_t *file)
{
	if (file->was_writing)
		return CassetteFlush(file);
	return TRUE;
}

/* Returns position in blocks/samples, counted from 0. */
unsigned int IMG_TAPE_GetPosition(IMG_TAPE_t *file)
{
	return file->current_block;
}
/* Returns size in blocks/samples. */
unsigned int IMG_TAPE_GetSize(IMG_TAPE_t *file)
{
	return file->num_blocks;
}

void IMG_TAPE_Seek(IMG_TAPE_t *file, unsigned int position)
{
	if (file->was_writing) {
		CassetteFlush(file);
		file->was_writing = FALSE;
	}
	file->current_block = (int)position;
	if (file->current_block > file->num_blocks)
		file->current_block = file->num_blocks;
	file->savetime = 0;
	file->save_gap = 0;
	file->next_blockbyte = 0;
	file->block_length = 0;
}

int IMG_TAPE_SerinStatus(IMG_TAPE_t *file, int event_time_left)
{
	int bit = 0;

	if (file->was_writing || file->next_blockbyte == 0)
		return 1;
	/* exam rate; if time_to_irq < duration of one byte */
	if (event_time_left <
		10 * 1789790 / (file->isCAS ? file->block_baudrates[file->current_block] : 600) - 1) {
		bit = event_time_left / (1789790 / (file->isCAS ? file->block_baudrates[file->current_block] : 600));
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
	return (file->buffer[file->next_blockbyte - 1] >> (8 - bit)) & 1;
}

int IMG_TAPE_SkipToData(IMG_TAPE_t *file, int ms)
{
	if (file->was_writing) {
		CassetteFlush(file);
		file->was_writing = FALSE;
	}

	while (ms > 0) {
		if (file->next_blockbyte < file->block_length) {
			int bytes = ms * (file->isCAS ? file->block_baudrates[file->current_block] : 600) / 1000 / 10;
			if (bytes > file->block_length - file->next_blockbyte)
				bytes = file->block_length - file->next_blockbyte;
			file->next_blockbyte += bytes;
			ms -= bytes * 10 * 1000 / (file->isCAS ? file->block_baudrates[file->current_block] : 600);
			continue;
		}
		else {
			int gap;
			if (!ReadNextRecord(file, &gap))
				return FALSE;
			file->next_blockbyte = 0;
			ms -= gap;
		}
	}
	return TRUE;
}

int IMG_TAPE_ReadToMemory(IMG_TAPE_t *file, UWORD dest_addr, int length)
{
	int read_length;
	if (file->was_writing) {
		CassetteFlush(file);
		file->was_writing = FALSE;
	}
	read_length = file->block_length - file->next_blockbyte;

	if (read_length == 0) {
		/* No bytes left in current block, need to read next block. */
		int gap;
		if (!ReadNextRecord(file, &gap))
			/* EOF or read error */
			return -1;
		file->block_length = read_length;
		file->next_blockbyte = 0;
	}
	/* Copy record to memory, excluding the checksum byte if it exists. */
	MEMORY_CopyToMem(file->buffer + file->next_blockbyte, dest_addr, read_length >= length ? length : read_length);
	file->next_blockbyte += (read_length >= length + 1 ? length + 1 : read_length);
	return read_length >= length + 1 &&
	       file->buffer[length] == SIO_ChkSum(file->buffer, length);
}

int IMG_TAPE_WriteFromMemory(IMG_TAPE_t *file, UWORD src_addr, int length, int gap)
{
	if (!file->was_writing) {
		file->savetime = 0;
		file->save_gap = 0;
		file->next_blockbyte = 0;
		file->block_length = 0;
		file->was_writing = TRUE;
	}
	EnlargeBuffer(file, length + 1);
	/* Put record into buffer. */
	MEMORY_CopyFromMem(src_addr, file->buffer, length);
	/* Eval checksum over buffer data. */
	file->buffer[length] = SIO_ChkSum(file->buffer, length);
	file->save_gap = gap;
	file->block_length = length + 1;
	return WriteRecord(file);
}
