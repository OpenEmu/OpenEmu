/*
 * img_tape.h - support for CAS and raw tape images
 *
 * Copyright (C) 2011 Tomasz Krasuski
 * Copyright (C) 2011 Atari800 development team (see DOC/CREDITS)
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
#ifndef IMG_TAPE_H_
#define IMG_TAPE_H_

#include "atari.h"

typedef struct IMG_TAPE_t IMG_TAPE_t;

/* Checks if a file is a valid tape image, based on its first four bytes
   stored in HEADER. */
int IMG_TAPE_FileSupported(UBYTE const start_bytes[4]);

/* Opens a cassette image pointed to by FILENAME.
   Stores a boolean in *WRITABLE, indicating if the file is writable.
   For CAS files, stores in *DESCRIPTION a pointer to the file's description.
   Returns pointer to the opened file on success or NULL otherwise. */
IMG_TAPE_t *IMG_TAPE_Open(char const *filename, int *writable, char const **description);

/* Closes a tape image FILE. */
void IMG_TAPE_Close(IMG_TAPE_t *file);

/* Creates a new writable CAS file at FILENAME, with a given description.
   Returns pointer to the opened file on success or NULL otherwise. */
IMG_TAPE_t *IMG_TAPE_Create(char const *filename, char const *description);

/* Reads the next tape event from FILE.
   Stores duration of the event (in CPU ticks) in *DURATION.
   Stores a boolean in *IS_GAP, indicating if the event is an IRG (TRUE)
   or a byte (FALSE).
   If *IS_GAP == FALSE, stores the byte being read in *BYTE.
   Returns TRUE on success or FALSE on read error or EOF. */
int IMG_TAPE_Read(IMG_TAPE_t *file, unsigned int *duration, int *is_gap, UBYTE *byte);

/* Advances the tape file by a given DURATION (in CPU ticks) during
   writing. */
void IMG_TAPE_WriteAdvance(IMG_TAPE_t *file, unsigned int duration);

/* Writes a BYTE to the tape file. POKEY_COUNTER is value of the POKEY counter
   used for serial transmission.
   Returns TRUE on success or FALSE on write error. */
int IMG_TAPE_WriteByte(IMG_TAPE_t *file, UBYTE byte, unsigned int pokey_counter);

/* Flushes the file's buffers. Any CAS chunk being currently written is
   immediately ended.
   Returns TRUE on success or FALSE on write error. */
int IMG_TAPE_Flush(IMG_TAPE_t *file);

/* Returns the file's current position (block), counted from 0. */
unsigned int IMG_TAPE_GetPosition(IMG_TAPE_t *file);
/* Returns the file's size (blocks). */
unsigned int IMG_TAPE_GetSize(IMG_TAPE_t *file);
/* Positions the file at the start of a block given in POSITION (counted from
   0). */
void IMG_TAPE_Seek(IMG_TAPE_t *file, unsigned int position);

/* Returns direct state of POKEY's serial input port during tape reading.
   EVENT_TIME_LEFT is number of CPU ticks left till the end of the byte that's
   currently being read. */
int IMG_TAPE_SerinStatus(IMG_TAPE_t *file, int event_time_left);

/* --- Functions used by patched SIO --- */

/* Forwards the tape past a given amount of milliseconds.
   Returns TRUE on success or FALSE on read error or EOF. */
int IMG_TAPE_SkipToData(IMG_TAPE_t *file, int ms);
/* Reads a record of specified LENGTH from tape and stores it in system's
   memory starting ad DEST_ADDR.
   Returns TRUE on success (requested number of bytes read successfully and
   checksum is correct), FALSE on too short record or bad checksum,
   or -1 on read error/EOF (also implicates too short record). */
int IMG_TAPE_ReadToMemory(IMG_TAPE_t *file, UWORD dest_addr, int length);
/* Writes data from system memory starting at SRC_ADDR of specified LENGTH
   to a tape record. GAP is length of the pre-record gap, in ms.
   Returns TRUE on success or FALSE on write error. */
int IMG_TAPE_WriteFromMemory(IMG_TAPE_t *file, UWORD src_addr, int length, int gap);

#endif /* IMG_TAPE_H_ */
