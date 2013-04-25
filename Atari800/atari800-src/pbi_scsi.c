/*
 * pbi_scsi.c - SCSI emulation for the MIO and Black Box
 *
 * Copyright (C) 2007-2008 Perry McFarlane
 * Copyright (C) 1998-2008 Atari800 development team (see DOC/CREDITS)
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

#include "atari.h"
#include "util.h"
#include "log.h"
#include "pbi_scsi.h"

#ifdef PBI_DEBUG
#define D(a) a
#else
#define D(a) do{}while(0)
#endif

int PBI_SCSI_CD = FALSE;
int PBI_SCSI_MSG = FALSE;
int PBI_SCSI_IO = FALSE;
int PBI_SCSI_BSY = FALSE;
int PBI_SCSI_REQ = FALSE;
int PBI_SCSI_ACK = FALSE;

int PBI_SCSI_SEL = FALSE;

static UBYTE scsi_byte;

#define SCSI_PHASE_SELECTION 0
#define SCSI_PHASE_DATAIN 1
#define SCSI_PHASE_DATAOUT 2
#define SCSI_PHASE_COMMAND 3
#define SCSI_PHASE_STATUS 4 
#define SCSI_PHASE_MSGIN 5

static int scsi_phase = SCSI_PHASE_SELECTION;
static int scsi_bufpos = 0;
static UBYTE scsi_buffer[256];
static int scsi_count = 0;

FILE *PBI_SCSI_disk = NULL;

static void scsi_changephase(int phase)
{
	D(printf("scsi_changephase:%d\n",phase));
	switch(phase) {
		case SCSI_PHASE_SELECTION:
				PBI_SCSI_REQ = FALSE;
				PBI_SCSI_BSY = FALSE;
				PBI_SCSI_CD = FALSE;
				PBI_SCSI_IO = FALSE;
				PBI_SCSI_MSG = FALSE;
				break;
		case SCSI_PHASE_DATAOUT:
				PBI_SCSI_REQ = TRUE;
				PBI_SCSI_BSY = TRUE;
				PBI_SCSI_CD = FALSE;
				PBI_SCSI_IO = FALSE;
				PBI_SCSI_MSG = FALSE;
				break;
		case SCSI_PHASE_DATAIN:
				PBI_SCSI_REQ = TRUE;
				PBI_SCSI_BSY = TRUE;
				PBI_SCSI_CD = FALSE;
				PBI_SCSI_IO = TRUE;
				PBI_SCSI_MSG = FALSE;
				break;
		case SCSI_PHASE_COMMAND:
				PBI_SCSI_REQ = TRUE;
				PBI_SCSI_BSY = TRUE;
				PBI_SCSI_CD = TRUE;
				PBI_SCSI_IO = FALSE;
				PBI_SCSI_MSG = FALSE;
				break;
		case SCSI_PHASE_STATUS:
				PBI_SCSI_REQ = TRUE;
				PBI_SCSI_BSY = TRUE;
				PBI_SCSI_CD = TRUE;
				PBI_SCSI_IO = TRUE;
				PBI_SCSI_MSG = FALSE;
				break;
		case SCSI_PHASE_MSGIN:
				PBI_SCSI_REQ = TRUE;
				PBI_SCSI_BSY = TRUE;
				PBI_SCSI_CD = TRUE;
				PBI_SCSI_IO = FALSE;
				PBI_SCSI_MSG = TRUE;
				break;
	}
	scsi_bufpos = 0;
	scsi_phase = phase;
}

static void scsi_process_command(void)
{
	int i;
	int lba;
/*	int lun;*/
	D(printf("SCSI command:"));
	for (i = 0; i < 6; i++) {
		D(printf(" %02x",scsi_buffer[i]));
	}
	D(printf("\n"));
	switch (scsi_buffer[0]) {
		case 0x00:
			/* test unit ready */
			D(printf("SCSI: test unit ready\n"));
			scsi_changephase(SCSI_PHASE_STATUS);
			scsi_buffer[0] = 0;
			break;
		case 0x03:
			/* request sense */
			D(printf("SCSI: request sense\n"));
			scsi_changephase(SCSI_PHASE_DATAIN);
			memset(scsi_buffer,0,1);
			scsi_count = 4;
			break;
		case 0x08:
			/* read */
/*			lun = ((scsi_buffer[1]&0xe0)>>5);*/
			lba = (((scsi_buffer[1]&0x1f)<<16)|(scsi_buffer[2]<<8)|(scsi_buffer[3]));
			D(printf("SCSI: read lun:%d lba:%d\n",lun,lba));
			fseek(PBI_SCSI_disk, lba*256, SEEK_SET);
			scsi_count = fread(scsi_buffer, 1, 256, PBI_SCSI_disk);
			scsi_changephase(SCSI_PHASE_DATAIN);
			/* scsi_count = 256; */
			break;
		case 0x0a:
			/* write */
/*			lun = ((scsi_buffer[1]&0xe0)>>5);*/
			lba = (((scsi_buffer[1]&0x1f)<<16)|(scsi_buffer[2]<<8)|(scsi_buffer[3]));
			D(printf("SCSI: write lun:%d lba:%d\n",lun,lba));
			fseek(PBI_SCSI_disk, lba*256, SEEK_SET);
			scsi_changephase(SCSI_PHASE_DATAOUT);
			scsi_count = 256;
			break;
		default:
			D(printf("SCSI: unknown command:%2x\n", scsi_buffer[0]));
			scsi_changephase(SCSI_PHASE_SELECTION);
			break;
	}
}

static void scsi_nextbyte(void)
{
	if (scsi_phase == SCSI_PHASE_DATAIN) {
		scsi_bufpos++;
		if (scsi_bufpos >= scsi_count) {
			scsi_changephase(SCSI_PHASE_STATUS);
			scsi_buffer[0] = 0;
		}
	}
	else if (scsi_phase == SCSI_PHASE_STATUS) {
		D(printf("SCSI status\n"));
		scsi_changephase(SCSI_PHASE_MSGIN);
		scsi_buffer[0] = 0;
	}
	else if (scsi_phase == SCSI_PHASE_MSGIN) {
		D(printf("SCSI msg\n"));
		scsi_changephase(SCSI_PHASE_SELECTION);
	}
	else if (scsi_phase == SCSI_PHASE_COMMAND) {
		scsi_buffer[scsi_bufpos++] = scsi_byte;
		if (scsi_bufpos >= 0x06) {
			scsi_process_command();
			scsi_bufpos = 0;
		}
	}
	else if (scsi_phase == SCSI_PHASE_DATAOUT) {
		D(printf("SCSI data out:%2x\n", scsi_byte));
		scsi_buffer[scsi_bufpos++] = scsi_byte;
		if (scsi_bufpos >= scsi_count) {
			fwrite(scsi_buffer, 1, 256, PBI_SCSI_disk);
			scsi_changephase(SCSI_PHASE_STATUS);
			scsi_buffer[0] = 0;
		}
	}
}

void PBI_SCSI_PutSEL(int newsel)
{
	if (newsel != PBI_SCSI_SEL) {
		/* SEL changed state */
		PBI_SCSI_SEL = newsel;
		if (PBI_SCSI_SEL && scsi_phase == SCSI_PHASE_SELECTION && scsi_byte == 0x01) {
			scsi_changephase(SCSI_PHASE_COMMAND);
		}
		D(printf("changed SEL:%d  scsi_byte:%2x\n",PBI_SCSI_SEL, scsi_byte));
	}
}

void PBI_SCSI_PutACK(int newack)
{
	if (newack != PBI_SCSI_ACK) {
		/* ACK changed state */
		PBI_SCSI_ACK = newack;
		if (PBI_SCSI_ACK) {
			/* REQ goes false when ACK goes true */
			PBI_SCSI_REQ = FALSE;
		}
		else {
			/* falling ACK triggers next byte */
			if (scsi_phase != SCSI_PHASE_SELECTION) {
				PBI_SCSI_REQ = TRUE;
				scsi_nextbyte();
			}
		}
	}
}

UBYTE PBI_SCSI_GetByte(void)
{
	return (scsi_buffer[scsi_bufpos]);
}

void PBI_SCSI_PutByte(UBYTE byte)
{
	scsi_byte = byte;
}

/*
vim:ts=4:sw=4:
*/
