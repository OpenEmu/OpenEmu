/*
 * sio.c - Serial I/O emulation
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
#include <math.h>

#include "afile.h"
#include "antic.h"  /* ANTIC_ypos */
#include "atari.h"
#include "binload.h"
#include "cassette.h"
#include "compfile.h"
#include "cpu.h"
#include "esc.h"
#include "log.h"
#include "memory.h"
#include "platform.h"
#include "pokey.h"
#include "pokeysnd.h"
#include "sio.h"
#include "util.h"
#ifndef BASIC
#include "statesav.h"
#endif

#undef DEBUG_PRO
#undef DEBUG_VAPI

/* If ATR image is in double density (256 bytes per sector),
   then the boot sectors (sectors 1-3) can be:
   - logical (as seen by Atari) - 128 bytes in each sector
   - physical (as stored on the disk) - 256 bytes in each sector.
     Only the first half of sector is used for storing data, the rest is zero.
   - SIO2PC (the type used by the SIO2PC program) - 3 * 128 bytes for data
     of boot sectors, then 3 * 128 unused bytes (zero)
   The XFD images in double density have either logical or physical
   boot sectors. */
#define BOOT_SECTORS_LOGICAL	0
#define BOOT_SECTORS_PHYSICAL	1
#define BOOT_SECTORS_SIO2PC		2
static int boot_sectors_type[SIO_MAX_DRIVES];

static int image_type[SIO_MAX_DRIVES];
#define IMAGE_TYPE_XFD  0
#define IMAGE_TYPE_ATR  1
#define IMAGE_TYPE_PRO  2
#define IMAGE_TYPE_VAPI 3
static FILE *disk[SIO_MAX_DRIVES] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
static int sectorcount[SIO_MAX_DRIVES];
static int sectorsize[SIO_MAX_DRIVES];
/* these two are used by the 1450XLD parallel disk device */
int SIO_format_sectorcount[SIO_MAX_DRIVES];
int SIO_format_sectorsize[SIO_MAX_DRIVES];
static int io_success[SIO_MAX_DRIVES];
/* stores dup sector counter for PRO images */
typedef struct tagpro_additional_info_t {
	int max_sector;
	unsigned char *count;
} pro_additional_info_t;

#define MAX_VAPI_PHANTOM_SEC  		40
#define VAPI_BYTES_PER_TRACK        26042.0 
#define VAPI_CYCLES_PER_ROT 		372706
#define VAPI_CYCLES_PER_TRACK_STEP 	35780 /*70937*/
#define VAPI_CYCLES_HEAD_SETTLE 	70134
#define VAPI_CYCLES_TRACK_READ_DELTA 1426
#define VAPI_CYCLES_CMD_ACK_TRANS 	3188
#define VAPI_CYCLES_SECTOR_READ 	29014
#define VAPI_CYCLES_MISSING_SECTOR	(2*VAPI_CYCLES_PER_ROT + 14453)
#define VAPI_CYCLES_BAD_SECTOR_NUM	1521

/* stores dup sector information for VAPI images */
typedef struct tagvapi_sec_info_t {
	int sec_count;
	unsigned int sec_offset[MAX_VAPI_PHANTOM_SEC];
	unsigned char sec_status[MAX_VAPI_PHANTOM_SEC];
	unsigned int sec_rot_pos[MAX_VAPI_PHANTOM_SEC];
} vapi_sec_info_t;

typedef struct tagvapi_additional_info_t {
	vapi_sec_info_t *sectors;
	int sec_stat_buff[4];
	int vapi_delay_time;
} vapi_additional_info_t;

/* VAPI Format Header */
typedef struct tagvapi_file_header_t {
	unsigned char signature[4];
	unsigned char majorver;
	unsigned char minorver;
	unsigned char reserved1[22];
	unsigned char startdata[4];
	unsigned char reserved[16];
} vapi_file_header_t;

typedef struct tagvapi_track_header_t {
	unsigned char  next[4];
	unsigned char  type[2];
	unsigned char  reserved1[2];
	unsigned char  tracknum;
	unsigned char  reserved2;
	unsigned char  sectorcnt[2];
	unsigned char  reserved3[8];
	unsigned char  startdata[4];
	unsigned char  reserved4[8];
} vapi_track_header_t;

typedef struct tagvapi_sector_list_header_t {
	unsigned char  sizelist[4];
	unsigned char  type;
	unsigned char  reserved[3];
} vapi_sector_list_header_t;

typedef struct tagvapi_sector_header_t {
	unsigned char  sectornum;
	unsigned char  sectorstatus;
	unsigned char  sectorpos[2];
	unsigned char  startdata[4];
} vapi_sector_header_t;

#define VAPI_32(x) (x[0] + (x[1] << 8) + (x[2] << 16) + (x[3] << 24))
#define VAPI_16(x) (x[0] + (x[1] << 8))

/* Additional Info for all copy protected disk types */
static void *additional_info[SIO_MAX_DRIVES];

SIO_UnitStatus SIO_drive_status[SIO_MAX_DRIVES];
char SIO_filename[SIO_MAX_DRIVES][FILENAME_MAX];

Util_tmpbufdef(static, sio_tmpbuf[SIO_MAX_DRIVES])

int SIO_last_op;
int SIO_last_op_time = 0;
int SIO_last_drive;
int SIO_last_sector;
char SIO_status[256];

/* Serial I/O emulation support */
#define SIO_NoFrame         (0x00)
#define SIO_CommandFrame    (0x01)
#define SIO_StatusRead      (0x02)
#define SIO_ReadFrame       (0x03)
#define SIO_WriteFrame      (0x04)
#define SIO_FinalStatus     (0x05)
#define SIO_FormatFrame     (0x06)
#define SIO_CasRead         (0x60)
#define SIO_CasWrite        (0x61)
static UBYTE CommandFrame[6];
static int CommandIndex = 0;
static UBYTE DataBuffer[256 + 3];
static int DataIndex = 0;
static int TransferStatus = SIO_NoFrame;
static int ExpectedBytes = 0;

int ignore_header_writeprotect = FALSE;

int SIO_Initialise(int *argc, char *argv[])
{
	int i;
	for (i = 0; i < SIO_MAX_DRIVES; i++) {
		strcpy(SIO_filename[i], "Off");
		SIO_drive_status[i] = SIO_OFF;
		SIO_format_sectorsize[i] = 128;
		SIO_format_sectorcount[i] = 720;
	}
	TransferStatus = SIO_NoFrame;

	return TRUE;
}

/* umount disks so temporary files are deleted */
void SIO_Exit(void)
{
	int i;
	for (i = 1; i <= SIO_MAX_DRIVES; i++)
		SIO_Dismount(i);
}

int SIO_Mount(int diskno, const char *filename, int b_open_readonly)
{
	FILE *f = NULL;
	SIO_UnitStatus status = SIO_READ_WRITE;
	struct AFILE_ATR_Header header;

	/* avoid overruns in SIO_filename[] */
	if (strlen(filename) >= FILENAME_MAX)
		return FALSE;

	/* release previous disk */
	SIO_Dismount(diskno);

	/* open file */
	if (!b_open_readonly)
		f = Util_fopen(filename, "rb+", sio_tmpbuf[diskno - 1]);
	if (f == NULL) {
		f = Util_fopen(filename, "rb", sio_tmpbuf[diskno - 1]);
		if (f == NULL)
			return FALSE;
		status = SIO_READ_ONLY;
	}

	/* read header */
	if (fread(&header, 1, sizeof(struct AFILE_ATR_Header), f) != sizeof(struct AFILE_ATR_Header)) {
		fclose(f);
		return FALSE;
	}

	/* detect compressed image and uncompress */
	switch (header.magic1) {
	case 0xf9:
	case 0xfa:
		/* DCM */
		{
			FILE *f2 = Util_tmpopen(sio_tmpbuf[diskno - 1]);
			if (f2 == NULL)
				return FALSE;
			Util_rewind(f);
			if (!CompFile_DCMtoATR(f, f2)) {
				Util_fclose(f2, sio_tmpbuf[diskno - 1]);
				fclose(f);
				return FALSE;
			}
			fclose(f);
			f = f2;
		}
		Util_rewind(f);
		if (fread(&header, 1, sizeof(struct AFILE_ATR_Header), f) != sizeof(struct AFILE_ATR_Header)) {
			Util_fclose(f, sio_tmpbuf[diskno - 1]);
			return FALSE;
		}
		status = SIO_READ_ONLY;
		/* XXX: status = b_open_readonly ? SIO_READ_ONLY : SIO_READ_WRITE; */
		break;
	case 0x1f:
		if (header.magic2 == 0x8b) {
			/* ATZ/ATR.GZ, XFZ/XFD.GZ */
			fclose(f);
			f = Util_tmpopen(sio_tmpbuf[diskno - 1]);
			if (f == NULL)
				return FALSE;
			if (!CompFile_ExtractGZ(filename, f)) {
				Util_fclose(f, sio_tmpbuf[diskno - 1]);
				return FALSE;
			}
			Util_rewind(f);
			if (fread(&header, 1, sizeof(struct AFILE_ATR_Header), f) != sizeof(struct AFILE_ATR_Header)) {
				Util_fclose(f, sio_tmpbuf[diskno - 1]);
				return FALSE;
			}
			status = SIO_READ_ONLY;
			/* XXX: status = b_open_readonly ? SIO_READ_ONLY : SIO_READ_WRITE; */
		}
		break;
	default:
		break;
	}

	boot_sectors_type[diskno - 1] = BOOT_SECTORS_LOGICAL;

	if (header.magic1 == AFILE_ATR_MAGIC1 && header.magic2 == AFILE_ATR_MAGIC2) {
		/* ATR (may be temporary from DCM or ATR/ATR.GZ) */
		image_type[diskno - 1] = IMAGE_TYPE_ATR;

		sectorsize[diskno - 1] = (header.secsizehi << 8) + header.secsizelo;
		if (sectorsize[diskno - 1] != 128 && sectorsize[diskno - 1] != 256) {
			Util_fclose(f, sio_tmpbuf[diskno - 1]);
			return FALSE;
		}

		if (header.writeprotect != 0 && !ignore_header_writeprotect)
			status = SIO_READ_ONLY;

		/* ATR header contains length in 16-byte chunks. */
		/* First compute number of 128-byte chunks
		   - it's number of sectors on single density disk */
		sectorcount[diskno - 1] = ((header.hiseccounthi << 24)
			+ (header.hiseccountlo << 16)
			+ (header.seccounthi << 8)
			+ header.seccountlo) >> 3;

		/* Fix number of sectors if double density */
		if (sectorsize[diskno - 1] == 256) {
			if ((sectorcount[diskno - 1] & 1) != 0)
				/* logical (128-byte) boot sectors */
				sectorcount[diskno - 1] += 3;
			else {
				/* 256-byte boot sectors */
				/* check if physical or SIO2PC: physical if there's
				   a non-zero byte in bytes 0x190-0x30f of the ATR file */
				UBYTE buffer[0x180];
				int i;
				fseek(f, 0x190, SEEK_SET);
				if (fread(buffer, 1, 0x180, f) != 0x180) {
					Util_fclose(f, sio_tmpbuf[diskno - 1]);
					return FALSE;
				}
				boot_sectors_type[diskno - 1] = BOOT_SECTORS_SIO2PC;
				for (i = 0; i < 0x180; i++)
					if (buffer[i] != 0) {
						boot_sectors_type[diskno - 1] = BOOT_SECTORS_PHYSICAL;
						break;
					}
			}
			sectorcount[diskno - 1] >>= 1;
		}
	}
	else if (header.magic1 == 'A' && header.magic2 == 'T' && header.seccountlo == '8' &&
		 header.seccounthi == 'X') {
		int file_length = Util_flen(f);
		vapi_additional_info_t *info;
		vapi_file_header_t fileheader;
		vapi_track_header_t trackheader;
		int trackoffset, totalsectors;

		/* .atx is read only for now */
#ifndef VAPI_WRITE_ENABLE
		if (!b_open_readonly) {
			fclose(f);
			f = Util_fopen(filename, "rb", sio_tmpbuf[diskno - 1]);
			if (f == NULL)
				return FALSE;
			status = SIO_READ_ONLY;
		}
#endif
		
		image_type[diskno - 1] = IMAGE_TYPE_VAPI;
		sectorsize[diskno - 1] = 128;
		sectorcount[diskno - 1] = 720;
		fseek(f,0,SEEK_SET);
		if (fread(&fileheader,1,sizeof(fileheader),f) != sizeof(fileheader)) {
			Util_fclose(f, sio_tmpbuf[diskno - 1]);
			Log_print("VAPI: Bad File Header");
			return(FALSE);
			}
		trackoffset = VAPI_32(fileheader.startdata);	
		if (trackoffset > file_length) {
			Util_fclose(f, sio_tmpbuf[diskno - 1]);
			Log_print("VAPI: Bad Track Offset");
			return(FALSE);
			}
#ifdef DEBUG_VAPI
		Log_print("VAPI File Version %d.%d",fileheader.majorver,fileheader.minorver);
#endif
		/* Read all of the track headers to get the total sector count */
		totalsectors = 0;
		while (trackoffset > 0 && trackoffset < file_length) {
			ULONG next;
			UWORD tracktype;

			fseek(f,trackoffset,SEEK_SET);
			if (fread(&trackheader,1,sizeof(trackheader),f) != sizeof(trackheader)) {
				Util_fclose(f, sio_tmpbuf[diskno - 1]);
				Log_print("VAPI: Bad Track Header");
				return(FALSE);
				}
			next = VAPI_32(trackheader.next);
			tracktype = VAPI_16(trackheader.type);
			if (tracktype == 0) {
				totalsectors += VAPI_16(trackheader.sectorcnt);
				}
			trackoffset += next;
		}

		info = Util_malloc(sizeof(vapi_additional_info_t));
		additional_info[diskno-1] = info;
		info->sectors = Util_malloc(sectorcount[diskno - 1] * 
 					    sizeof(vapi_sec_info_t));
		memset(info->sectors, 0, sectorcount[diskno - 1] * 
 					 sizeof(vapi_sec_info_t));

		/* Now read all the sector data */
		trackoffset = VAPI_32(fileheader.startdata);
		while (trackoffset > 0 && trackoffset < file_length) {
			int sectorcnt, seclistdata,next;
			vapi_sector_list_header_t sectorlist;
			vapi_sector_header_t sectorheader;
			vapi_sec_info_t *sector;
			UWORD tracktype;
			int j;

			fseek(f,trackoffset,SEEK_SET);
			if (fread(&trackheader,1,sizeof(trackheader),f) != sizeof(trackheader)) {
				free(info->sectors);
				free(info);
				Util_fclose(f, sio_tmpbuf[diskno - 1]);
				Log_print("VAPI: Bad Track Header while reading sectors");
				return(FALSE);
				}
			next = VAPI_32(trackheader.next);
			sectorcnt = VAPI_16(trackheader.sectorcnt);
			tracktype = VAPI_16(trackheader.type);
			seclistdata = VAPI_32(trackheader.startdata) + trackoffset;
#ifdef DEBUG_VAPI
			Log_print("Track %d: next %x type %d seccnt %d secdata %x",trackheader.tracknum,
				trackoffset + next,VAPI_16(trackheader.type),sectorcnt,seclistdata);
#endif
			if (tracktype == 0) {
				if (seclistdata > file_length) {
					free(info->sectors);
					free(info);
					Util_fclose(f, sio_tmpbuf[diskno - 1]);
					Log_print("VAPI: Bad Sector List Offset");
					return(FALSE);
					}
				fseek(f,seclistdata,SEEK_SET);
				if (fread(&sectorlist,1,sizeof(sectorlist),f) != sizeof(sectorlist)) {
					free(info->sectors);
					free(info);
					Util_fclose(f, sio_tmpbuf[diskno - 1]);
					Log_print("VAPI: Bad Sector List");
					return(FALSE);
					}
#ifdef DEBUG_VAPI
				Log_print("Size sec list %x type %d",VAPI_32(sectorlist.sizelist),sectorlist.type);
#endif
				for (j=0;j<sectorcnt;j++) {
					double percent_rot;

					if (fread(&sectorheader,1,sizeof(sectorheader),f) != sizeof(sectorheader)) {
						free(info->sectors);
						free(info);
						Util_fclose(f, sio_tmpbuf[diskno - 1]);
						Log_print("VAPI: Bad Sector Header");
						return(FALSE);
						}
					if (sectorheader.sectornum > 18)  {
						Util_fclose(f, sio_tmpbuf[diskno - 1]);
						Log_print("VAPI: Bad Sector Index: Track %d Sec Num %d Index %d",
								trackheader.tracknum,j,sectorheader.sectornum);
						return(FALSE);
						}
					sector = &info->sectors[trackheader.tracknum * 18 + sectorheader.sectornum - 1];

					percent_rot = ((double) VAPI_16(sectorheader.sectorpos))/VAPI_BYTES_PER_TRACK;
					sector->sec_rot_pos[sector->sec_count] = (unsigned int) (percent_rot * VAPI_CYCLES_PER_ROT);
					sector->sec_offset[sector->sec_count] = VAPI_32(sectorheader.startdata) + trackoffset;
					sector->sec_status[sector->sec_count] = ~sectorheader.sectorstatus;
					sector->sec_count++;
					if (sector->sec_count > MAX_VAPI_PHANTOM_SEC) {
						free(info->sectors);
						free(info);
						Util_fclose(f, sio_tmpbuf[diskno - 1]);
						Log_print("VAPI: Too many Phantom Sectors");
						return(FALSE);
						}
#ifdef DEBUG_VAPI
					Log_print("Sector %d status %x position %f %d %d data %x",sectorheader.sectornum,
						sector->sec_status[sector->sec_count-1],percent_rot,
						sector->sec_rot_pos[sector->sec_count-1],
						VAPI_16(sectorheader.sectorpos),
						sector->sec_offset[sector->sec_count-1]);				
#endif				
				}
#ifdef DEBUG_VAPI
				Log_flushlog();
#endif
			} else {
				Log_print("Unknown VAPI track type Track:%d Type:%d",trackheader.tracknum,tracktype);
			}
			trackoffset += next;
		}			
	}
	else {
		int file_length = Util_flen(f);
		/* check for PRO */
		if ((file_length-16)%(128+12) == 0 &&
				(header.magic1*256 + header.magic2 == (file_length-16)/(128+12)) &&
				header.seccountlo == 'P') {
			pro_additional_info_t *info;
			/* .pro is read only for now */
			if (!b_open_readonly) {
				fclose(f);
				f = Util_fopen(filename, "rb", sio_tmpbuf[diskno - 1]);
				if (f == NULL)
					return FALSE;
				status = SIO_READ_ONLY;
			}
			image_type[diskno - 1] = IMAGE_TYPE_PRO;
			sectorsize[diskno - 1] = 128;
			if (file_length >= 1040*(128+12)+16) {
				/* assume enhanced density */
				sectorcount[diskno - 1] = 1040;
			}
			else {
				/* assume single density */
				sectorcount[diskno - 1] = 720;
			}

			info = Util_malloc(sizeof(pro_additional_info_t));
			additional_info[diskno-1] = info;
			info->count = Util_malloc(sectorcount[diskno - 1]);
			memset(info->count, 0, sectorcount[diskno -1]);
			info->max_sector = (file_length-16)/(128+12);
		}
		else {
			/* XFD (may be temporary from XFZ/XFD.GZ) */

			image_type[diskno - 1] = IMAGE_TYPE_XFD;

			if (file_length <= (1040 * 128)) {
				/* single density */
				sectorsize[diskno - 1] = 128;
				sectorcount[diskno - 1] = file_length >> 7;
			}
			else {
				/* double density */
				sectorsize[diskno - 1] = 256;
				if ((file_length & 0xff) == 0) {
					boot_sectors_type[diskno - 1] = BOOT_SECTORS_PHYSICAL;
					sectorcount[diskno - 1] = file_length >> 8;
				}
				else
					sectorcount[diskno - 1] = (file_length + 0x180) >> 8;
			}
		}
	}

#ifdef DEBUG
	Log_print("sectorcount = %d, sectorsize = %d",
		   sectorcount[diskno - 1], sectorsize[diskno - 1]);
#endif
	SIO_format_sectorsize[diskno - 1] = sectorsize[diskno - 1];
	SIO_format_sectorcount[diskno - 1] = sectorcount[diskno - 1];
	strcpy(SIO_filename[diskno - 1], filename);
	SIO_drive_status[diskno - 1] = status;
	disk[diskno - 1] = f;
	return TRUE;
}

void SIO_Dismount(int diskno)
{
	if (disk[diskno - 1] != NULL) {
		Util_fclose(disk[diskno - 1], sio_tmpbuf[diskno - 1]);
		disk[diskno - 1] = NULL;
		SIO_drive_status[diskno - 1] = SIO_NO_DISK;
		strcpy(SIO_filename[diskno - 1], "Empty");
		if (image_type[diskno - 1] == IMAGE_TYPE_PRO) {
			free(((pro_additional_info_t *)additional_info[diskno-1])->count);
		}
		else if (image_type[diskno - 1] == IMAGE_TYPE_VAPI) {
			free(((vapi_additional_info_t *)additional_info[diskno-1])->sectors);
		}
		free(additional_info[diskno - 1]);
		additional_info[diskno - 1] = 0;
	}
}

void SIO_DisableDrive(int diskno)
{
	SIO_Dismount(diskno);
	SIO_drive_status[diskno - 1] = SIO_OFF;
	strcpy(SIO_filename[diskno - 1], "Off");
}

void SIO_SizeOfSector(UBYTE unit, int sector, int *sz, ULONG *ofs)
{
	int size;
	ULONG offset;
	int header_size = (image_type[unit] == IMAGE_TYPE_ATR ? 16 : 0);

	if (BINLOAD_start_binloading) {
		if (sz)
			*sz = 128;
		if (ofs)
			*ofs = 0;
		return;
	}

	if (image_type[unit] == IMAGE_TYPE_PRO) {
		size = 128;
		offset = 16 + (128+12)*(sector -1); /* returns offset of header */
	}
	else if (image_type[unit] == IMAGE_TYPE_VAPI) {
		vapi_additional_info_t *info;
		vapi_sec_info_t *secinfo;

		size = 128;
		info = additional_info[unit];
		if (info == NULL)
			offset = 0;
		else if (sector > sectorcount[unit])
			offset = 0;
		else {
			secinfo = &info->sectors[sector-1];
			if (secinfo->sec_count == 0  )
				offset = 0;
			else
				offset = secinfo->sec_offset[0];
		}
	}
	else if (sector < 4) {
		/* special case for first three sectors in ATR and XFD image */
		size = 128;
		offset = header_size + (sector - 1) * (boot_sectors_type[unit] == BOOT_SECTORS_PHYSICAL ? 256 : 128);
	}
	else {
		size = sectorsize[unit];
		offset = header_size + (boot_sectors_type[unit] == BOOT_SECTORS_LOGICAL ? 0x180 : 0x300) + (sector - 4) * size;
	}

	if (sz)
		*sz = size;

	if (ofs)
		*ofs = offset;
}

static int SeekSector(int unit, int sector)
{
	ULONG offset;
	int size;

	SIO_last_sector = sector;
	snprintf(SIO_status, sizeof(SIO_status), "%d: %d", unit + 1, sector);
	SIO_SizeOfSector((UBYTE) unit, sector, &size, &offset);
	fseek(disk[unit], offset, SEEK_SET);

	return size;
}

/* Unit counts from zero up */
int SIO_ReadSector(int unit, int sector, UBYTE *buffer)
{
	int size;
	if (BINLOAD_start_binloading)
		return BINLOAD_LoaderStart(buffer);

	io_success[unit] = -1;
	if (SIO_drive_status[unit] == SIO_OFF)
		return 0;
	if (disk[unit] == NULL)
		return 'N';
	if (sector <= 0 || sector > sectorcount[unit])
		return 'E';
	SIO_last_op = SIO_LAST_READ;
	SIO_last_op_time = 1;
	SIO_last_drive = unit + 1;
	/* FIXME: what sector size did the user expect? */
	size = SeekSector(unit, sector);
	if (image_type[unit] == IMAGE_TYPE_PRO) {
		pro_additional_info_t *info;
		unsigned char *count;
		info = (pro_additional_info_t *)additional_info[unit];
		count = info->count;
		if (fread(buffer, 1, 12, disk[unit]) < 12) {
			Log_print("Error in header of .pro image: sector:%d", sector);
			return 'E';
		}
		/* handle duplicate sectors */
		if (buffer[5] != 0) {
			int dupnum = count[sector];
#ifdef DEBUG_PRO
			Log_print("duplicate sector:%d dupnum:%d",sector, dupnum);
#endif
			count[sector] = (count[sector]+1) % (buffer[5]+1);
			if (dupnum != 0)  {
				sector = sectorcount[unit] + buffer[6+dupnum];
				/* can dupnum be 5? */
				if (dupnum > 4 || sector <= 0 || sector > info->max_sector) {
					Log_print("Error in .pro image: sector:%d dupnum:%d", sector, dupnum);
					return 'E';
				}
				size = SeekSector(unit, sector);
				/* read sector header */
				if (fread(buffer, 1, 12, disk[unit]) < 12) {
					Log_print("Error in header2 of .pro image: sector:%d dupnum:%d", sector, dupnum);
					return 'E';
				}
			}
		}
		/* bad sector */
		if (buffer[1] != 0xff) {
			if (fread(buffer, 1, size, disk[unit]) < size) {
				Log_print("Error in bad sector of .pro image: sector:%d", sector);
			}
			io_success[unit] = sector;
#ifdef DEBUG_PRO
			Log_print("bad sector:%d", sector);
#endif
			return 'E';
		}
	}
	else if (image_type[unit] == IMAGE_TYPE_VAPI) {
		vapi_additional_info_t *info;
		vapi_sec_info_t *secinfo;
		ULONG secindex = 0;
		static int lasttrack = 0;
		unsigned int currpos, time, delay, rotations, bestdelay;
		unsigned char beststatus;
		int fromtrack, trackstostep, j;

		info = (vapi_additional_info_t *)additional_info[unit];
		info->vapi_delay_time = 0;

		if (sector > sectorcount[unit]) {
#ifdef DEBUG_VAPI
			Log_print("bad sector num:%d", sector);
#endif
			info->sec_stat_buff[0] = 9;
			info->sec_stat_buff[1] = 0xFF; 
			info->sec_stat_buff[2] = 0xe0;
			info->sec_stat_buff[3] = 0;
			info->vapi_delay_time= VAPI_CYCLES_BAD_SECTOR_NUM;
			return 'E';
		}

		secinfo = &info->sectors[sector-1];
		fromtrack = lasttrack;
		lasttrack = (sector-1)/18;

		if (secinfo->sec_count == 0) {
#ifdef DEBUG_VAPI
			Log_print("missing sector:%d", sector);
#endif
			info->sec_stat_buff[0] = 0xC;
			info->sec_stat_buff[1] = 0xEF; 
			info->sec_stat_buff[2] = 0xe0;
			info->sec_stat_buff[3] = 0;
			info->vapi_delay_time= VAPI_CYCLES_MISSING_SECTOR;
			return 'E';
		}

		trackstostep = abs((sector-1)/18 - fromtrack);
		time = (unsigned int) ANTIC_CPU_CLOCK;
		if (trackstostep)
			time += trackstostep * VAPI_CYCLES_PER_TRACK_STEP + VAPI_CYCLES_HEAD_SETTLE ;
		time += VAPI_CYCLES_CMD_ACK_TRANS;
		rotations = time/VAPI_CYCLES_PER_ROT;
		currpos = time - rotations*VAPI_CYCLES_PER_ROT;

#ifdef DEBUG_VAPI
		Log_print(" sector:%d sector count :%d time %d", sector,secinfo->sec_count,ANTIC_CPU_CLOCK);
#endif

		bestdelay = 10 * VAPI_CYCLES_PER_ROT;
		beststatus = 0;
		for (j=0;j<secinfo->sec_count;j++) {
			if (secinfo->sec_rot_pos[j]  < currpos)
				delay = (VAPI_CYCLES_PER_ROT - currpos) + secinfo->sec_rot_pos[j];
			else
				delay = secinfo->sec_rot_pos[j] - currpos; 
#ifdef DEBUG_VAPI
			Log_print("%d %d %d %d %d %x",j,secinfo->sec_rot_pos[j],
					  ((unsigned int) ANTIC_CPU_CLOCK) - ((((unsigned int) ANTIC_CPU_CLOCK)/VAPI_CYCLES_PER_ROT)*VAPI_CYCLES_PER_ROT),
					  currpos,delay,secinfo->sec_status[j]);
#endif
			if (delay < bestdelay) {
				bestdelay = delay;
				beststatus = secinfo->sec_status[j];
				secindex = j;
			}
		}
		if (trackstostep)
			info->vapi_delay_time = bestdelay + trackstostep * VAPI_CYCLES_PER_TRACK_STEP + 
				     VAPI_CYCLES_HEAD_SETTLE   +  VAPI_CYCLES_TRACK_READ_DELTA +
						       VAPI_CYCLES_CMD_ACK_TRANS + VAPI_CYCLES_SECTOR_READ;
		else
			info->vapi_delay_time = bestdelay + 
						       VAPI_CYCLES_CMD_ACK_TRANS + VAPI_CYCLES_SECTOR_READ;
#ifdef DEBUG_VAPI
		Log_print("Bestdelay = %d VapiDelay = %d",bestdelay,info->vapi_delay_time);
		if (secinfo->sec_count > 1)
			Log_print("duplicate sector:%d dupnum:%d delay:%d",sector, secindex,info->vapi_delay_time);
#endif
		fseek(disk[unit],secinfo->sec_offset[secindex],SEEK_SET);
		info->sec_stat_buff[0] = 0x8 | ((secinfo->sec_status[secindex] == 0xFF) ? 0 : 0x04);
		info->sec_stat_buff[1] = secinfo->sec_status[secindex];
		info->sec_stat_buff[2] = 0xe0;
		info->sec_stat_buff[3] = 0;
		if (secinfo->sec_status[secindex] != 0xFF) {
			if (fread(buffer, 1, size, disk[unit]) < size) {
				Log_print("error reading sector:%d", sector);
			}
			io_success[unit] = sector;
			info->vapi_delay_time += VAPI_CYCLES_PER_ROT + 10000;
#ifdef DEBUG_VAPI
			Log_print("bad sector:%d 0x%0X delay:%d", sector, secinfo->sec_status[secindex],info->vapi_delay_time );
#endif
			{
			int i;
				if (secinfo->sec_status[secindex] == 0xB7) {
					for (i=0;i<128;i++) {
						Log_print("0x%02x",buffer[i]);
						if (buffer[i] == 0x33)
							buffer[i] = rand() & 0xFF;
					}
				}
			}
			return 'E';
		}
#ifdef DEBUG_VAPI
		Log_flushlog();
#endif		
	}
	if (fread(buffer, 1, size, disk[unit]) < size) {
		Log_print("incomplete sector num:%d", sector);
	}
	io_success[unit] = 0;
	return 'C';
}

int SIO_WriteSector(int unit, int sector, const UBYTE *buffer)
{
	int size;
	io_success[unit] = -1;
	if (SIO_drive_status[unit] == SIO_OFF)
		return 0;
	if (disk[unit] == NULL)
		return 'N';
	if (SIO_drive_status[unit] != SIO_READ_WRITE || sector <= 0 || sector > sectorcount[unit])
		return 'E';
	SIO_last_op = SIO_LAST_WRITE;
	SIO_last_op_time = 1;
	SIO_last_drive = unit + 1;
#ifdef VAPI_WRITE_ENABLE 	
 	if (image_type[unit] == IMAGE_TYPE_VAPI) {
		vapi_additional_info_t *info;
		vapi_sec_info_t *secinfo;

		info = (vapi_additional_info_t *)additional_info[unit];
		secinfo = &info->sectors[sector-1];
		
		if (secinfo->sec_count != 1) {
			// No writes to sectors with duplicates or missing sectors
			return 'E';
		}
		
		if (secinfo->sec_status[0] != 0xFF) {
			// No writes to bad sectors
			return 'E';
		}
		
		size = SeekSector(unit, sector);
		fseek(disk[unit],secinfo->sec_offset[0],SEEK_SET);
		fwrite(buffer, 1, size, disk[unit]);
		io_success[unit] = 0;
		return 'C';
#if 0		
	} else if (image_type[unit] == IMAGE_TYPE_PRO) {
		pro_additional_info_t *info;
		pro_phantom_sec_info_t *phantom;
		
		info = (pro_additional_info_t *)additional_info[unit];
		phantom = &info->phantom[sector-1];
		
		if (phantom->phantom_count != 0) {
			// No writes to sectors with duplicates 
			return 'E';
		}
		
		size = SeekSector(unit, sector);
		if (buffer[1] != 0xff) {
#endif			
	} 
#endif
	size = SeekSector(unit, sector);
	fwrite(buffer, 1, size, disk[unit]);
	io_success[unit] = 0;
	return 'C';
}

int SIO_FormatDisk(int unit, UBYTE *buffer, int sectsize, int sectcount)
{
	char fname[FILENAME_MAX];
	int is_atr;
	int save_boot_sectors_type;
	int bootsectsize;
	int bootsectcount;
	FILE *f;
	int i;
	io_success[unit] = -1;
	if (SIO_drive_status[unit] == SIO_OFF)
		return 0;
	if (disk[unit] == NULL)
		return 'N';
	if (SIO_drive_status[unit] != SIO_READ_WRITE)
		return 'E';
	/* Note formatting the disk can change size of the file.
	   There is no portable way to truncate the file at given position.
	   We have to close the "rb+" open file and open it in "wb" mode.
	   First get the information about the disk image, because we are going
	   to umount it. */
	memcpy(fname, SIO_filename[unit], FILENAME_MAX);
	is_atr = (image_type[unit] == IMAGE_TYPE_ATR);
	save_boot_sectors_type = boot_sectors_type[unit];
	bootsectsize = 128;
	if (sectsize == 256 && save_boot_sectors_type != BOOT_SECTORS_LOGICAL)
		bootsectsize = 256;
	bootsectcount = sectcount < 3 ? sectcount : 3;
	/* Umount the file and open it in "wb" mode (it will truncate the file) */
	SIO_Dismount(unit + 1);
	f = fopen(fname, "wb");
	if (f == NULL) {
		Log_print("SIO_FormatDisk: failed to open %s for writing", fname);
		return 'E';
	}
	/* Write ATR header if necessary */
	if (is_atr) {
		struct AFILE_ATR_Header header;
		ULONG disksize = (bootsectsize * bootsectcount + sectsize * (sectcount - bootsectcount)) >> 4;
		memset(&header, 0, sizeof(header));
		header.magic1 = AFILE_ATR_MAGIC1;
		header.magic2 = AFILE_ATR_MAGIC2;
		header.secsizelo = (UBYTE) sectsize;
		header.secsizehi = (UBYTE) (sectsize >> 8);
		header.seccountlo = (UBYTE) disksize;
		header.seccounthi = (UBYTE) (disksize >> 8);
		header.hiseccountlo = (UBYTE) (disksize >> 16);
		header.hiseccounthi = (UBYTE) (disksize >> 24);
		fwrite(&header, 1, sizeof(header), f);
	}
	/* Write boot sectors */
	memset(buffer, 0, sectsize);
	for (i = 1; i <= bootsectcount; i++)
		fwrite(buffer, 1, bootsectsize, f);
	/* Write regular sectors */
	for ( ; i <= sectcount; i++)
		fwrite(buffer, 1, sectsize, f);
	/* Close file and mount the disk back */
	fclose(f);
	SIO_Mount(unit + 1, fname, FALSE);
	/* We want to keep the current PHYSICAL/SIO2PC boot sectors type
	   (since the image is blank it can't be figured out by SIO_Mount) */
	if (bootsectsize == 256)
		boot_sectors_type[unit] = save_boot_sectors_type;
	/* Return information for Atari (buffer filled with ff's - no bad sectors) */
	memset(buffer, 0xff, sectsize);
	io_success[unit] = 0;
	return 'C';
}

/* Set density and number of sectors
   This function is used before the format (0x21) command
   to set how the disk will be formatted.
   Note this function does *not* affect the currently attached disk
   (previously sectorsize/sectorcount were used which could result in
   a corrupted image).
*/
int SIO_WriteStatusBlock(int unit, const UBYTE *buffer)
{
	int size;
#ifdef DEBUG
	Log_print("Write Status-Block: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
		buffer[0], buffer[1], buffer[2], buffer[3],
		buffer[4], buffer[5], buffer[6], buffer[7],
		buffer[8], buffer[9], buffer[10], buffer[11]);
#endif
	if (SIO_drive_status[unit] == SIO_OFF)
		return 0;
	/* We only care about the density and the sector count here.
	   Setting everything else right here seems to be non-sense.
	   I'm not sure about this density settings, my XF551
	   honors only the sector size and ignores the density */
	size = buffer[6] * 256 + buffer[7];
	if (size == 128 || size == 256)
		SIO_format_sectorsize[unit] = size;
	/* Note that the number of heads are minus 1 */
	SIO_format_sectorcount[unit] = buffer[0] * (buffer[2] * 256 + buffer[3]) * (buffer[4] + 1);
	if (SIO_format_sectorcount[unit] < 1 || SIO_format_sectorcount[unit] > 65535)
		SIO_format_sectorcount[unit] = 720;
	return 'C';
}

int SIO_ReadStatusBlock(int unit, UBYTE *buffer)
{
	UBYTE tracks;
	UBYTE heads;
	int spt;
	if (SIO_drive_status[unit] == SIO_OFF)
		return 0;
	/* default to 1 track, 1 side for non-standard images */
	tracks = 1;
	heads = 1;
	spt = sectorcount[unit];

	if (spt % 40 == 0) {
		/* standard disk */
		tracks = 40;
		spt /= 40;
		if (spt > 26 && spt % 2 == 0) {
			/* double-sided */
			heads = 2;
			spt >>= 1;
			if (spt > 26 && spt % 2 == 0) {
				/* double-sided, 80 tracks */
				tracks = 80;
				spt >>= 1;
			}
		}
	}

	buffer[0] = tracks;              /* # of tracks */
	buffer[1] = 1;                   /* step rate */
	buffer[2] = (UBYTE) (spt >> 8);  /* sectors per track. HI byte */
	buffer[3] = (UBYTE) spt;         /* sectors per track. LO byte */
	buffer[4] = (UBYTE) (heads - 1); /* # of heads minus 1 */
	/* FM for single density, MFM otherwise */
	buffer[5] = (sectorsize[unit] == 128 && sectorcount[unit] <= 720) ? 0 : 4;
	buffer[6] = (UBYTE) (sectorsize[unit] >> 8); /* bytes per sector. HI byte */
	buffer[7] = (UBYTE) sectorsize[unit];        /* bytes per sector. LO byte */
	buffer[8] = 1;                   /* drive is online */
	buffer[9] = 192;                 /* transfer speed, whatever this means */
	buffer[10] = 0;
	buffer[11] = 0;
	return 'C';
}

/*
   Status Request from Atari 400/800 Technical Reference Notes

   DVSTAT + 0   Command Status
   DVSTAT + 1   Hardware Status
   DVSTAT + 2   Timeout
   DVSTAT + 3   Unused

   Command Status Bits

   Bit 0 = 1 indicates an invalid command frame was received
   Bit 1 = 1 indicates an invalid data frame was received
   Bit 2 = 1 indicates that last read/write operation was unsuccessful
   Bit 3 = 1 indicates that the diskette is write protected
   Bit 4 = 1 indicates active/standby

   plus

   Bit 5 = 1 indicates double density
   Bit 7 = 1 indicates dual density disk (1050 format)
 */
int SIO_DriveStatus(int unit, UBYTE *buffer)
{
	if (BINLOAD_start_binloading) {
		buffer[0] = 16 + 8;
		buffer[1] = 255;
		buffer[2] = 1;
		buffer[3] = 0 ;
		return 'C';
	}

	if (SIO_drive_status[unit] == SIO_OFF)
		return 0;

	/* .PRO contains status information in the sector header */
	if (io_success[unit] != 0  && image_type[unit] == IMAGE_TYPE_PRO) {
		int sector = io_success[unit];
		SeekSector(unit, sector);
		if (fread(buffer, 1, 4, disk[unit]) < 4) {
			Log_print("SIO_DriveStatus: failed to read sector header");
		}
		return 'C';
	}
	else if (io_success[unit] != 0  && image_type[unit] == IMAGE_TYPE_VAPI &&
			 SIO_drive_status[unit] != SIO_NO_DISK) {
		vapi_additional_info_t *info;
		info = (vapi_additional_info_t *)additional_info[unit];
		buffer[0] = info->sec_stat_buff[0];
		buffer[1] = info->sec_stat_buff[1];
		buffer[2] = info->sec_stat_buff[2];
		buffer[3] = info->sec_stat_buff[3];
		Log_print("Drive Status unit %d %x %x %x %x",unit,buffer[0], buffer[1], buffer[2], buffer[3]);
		return 'C';
	}	
	buffer[0] = 16;         /* drive active */
	buffer[1] = disk[unit] != NULL ? 255 /* WD 177x OK */ : 127 /* no disk */;
	if (io_success[unit] != 0)
		buffer[0] |= 4;     /* failed RW-operation */
	if (SIO_drive_status[unit] == SIO_READ_ONLY)
		buffer[0] |= 8;     /* write protection */
	if (SIO_format_sectorsize[unit] == 256)
		buffer[0] |= 32;    /* double density */
	if (SIO_format_sectorcount[unit] == 1040)
		buffer[0] |= 128;   /* 1050 enhanced density */
	buffer[2] = 1;
	buffer[3] = 0;
	return 'C';
}

#ifndef NO_SECTOR_DELAY
/* A hack for the "Overmind" demo.  This demo verifies if sectors aren't read
   faster than with a typical disk drive.  We introduce a delay
   of SECTOR_DELAY scanlines between successive reads of sector 1. */
#define SECTOR_DELAY 3200
static int delay_counter = 0;
static int last_ypos = 0;
#endif

/* SIO patch emulation routine */
void SIO_Handler(void)
{
	int sector = MEMORY_dGetWordAligned(0x30a);
	UBYTE unit = (MEMORY_dGetByte(0x300) + MEMORY_dGetByte(0x301) + 0xff ) - 0x31;
	UBYTE result = 0x00;
	UWORD data = MEMORY_dGetWordAligned(0x304);
	int length = MEMORY_dGetWordAligned(0x308);
	int realsize = 0;
	int cmd = MEMORY_dGetByte(0x302);

	if ((unsigned int)MEMORY_dGetByte(0x300) + (unsigned int)MEMORY_dGetByte(0x301) > 0xff) {
		/* carry */
		unit++;
	}
	/* A real atari just adds the bytes and 0xff. The result could wrap.*/
	/* XL OS: E99D: LDA $0300 ADC $0301 ADC #$FF STA 023A */
	/* Disk 1 is ASCII '1' = 0x31 etc */
	/* Disk 1 -> unit = 0 */
	if (MEMORY_dGetByte(0x300) != 0x60 && unit < SIO_MAX_DRIVES && (SIO_drive_status[unit] != SIO_OFF || BINLOAD_start_binloading)) {	/* UBYTE range ! */
#ifdef DEBUG
		Log_print("SIO disk command is %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x",
			cmd, MEMORY_dGetByte(0x303), MEMORY_dGetByte(0x304), MEMORY_dGetByte(0x305), MEMORY_dGetByte(0x306),
			MEMORY_dGetByte(0x308), MEMORY_dGetByte(0x309), MEMORY_dGetByte(0x30a), MEMORY_dGetByte(0x30b),
			MEMORY_dGetByte(0x30c), MEMORY_dGetByte(0x30d));
#endif
		switch (cmd) {
		case 0x4e:				/* Read Status Block */
			if (12 == length) {
				result = SIO_ReadStatusBlock(unit, DataBuffer);
				if (result == 'C')
					MEMORY_CopyToMem(DataBuffer, data, 12);
			}
			else
				result = 'E';
			break;
		case 0x4f:				/* Write Status Block */
			if (12 == length) {
				MEMORY_CopyFromMem(data, DataBuffer, 12);
				result = SIO_WriteStatusBlock(unit, DataBuffer);
			}
			else
				result = 'E';
			break;
		case 0x50:				/* Write */
		case 0x57:
		case 0xD0:				/* xf551 hispeed */
		case 0xD7:
			SIO_SizeOfSector(unit, sector, &realsize, NULL);
			if (realsize == length) {
				MEMORY_CopyFromMem(data, DataBuffer, realsize);
				result = SIO_WriteSector(unit, sector, DataBuffer);
			}
			else
				result = 'E';
			break;
		case 0x52:				/* Read */
		case 0xD2:				/* xf551 hispeed */
#ifndef NO_SECTOR_DELAY
			if (sector == 1) {
				if (delay_counter > 0) {
					if (last_ypos != ANTIC_ypos) {
						last_ypos = ANTIC_ypos;
						delay_counter--;
					}
					CPU_regPC = 0xe459;	/* stay at SIO patch */
					return;
				}
				delay_counter = SECTOR_DELAY;
			}
			else {
				delay_counter = 0;
			}
#endif
			SIO_SizeOfSector(unit, sector, &realsize, NULL);
			if (realsize == length) {
				result = SIO_ReadSector(unit, sector, DataBuffer);
				if (result == 'C')
					MEMORY_CopyToMem(DataBuffer, data, realsize);
			}
			else
				result = 'E';
			break;
		case 0x53:				/* Status */
			if (4 == length) {
				result = SIO_DriveStatus(unit, DataBuffer);
				if (result == 'C') {
					MEMORY_CopyToMem(DataBuffer, data, 4);
				}
			}
			else
				result = 'E';
			break;
		/*case 0x66:*/			/* US Doubler Format - I think! */
		case 0x21:				/* Format Disk */
		case 0xA1:				/* xf551 hispeed */
			realsize = SIO_format_sectorsize[unit];
			if (realsize == length) {
				result = SIO_FormatDisk(unit, DataBuffer, realsize, SIO_format_sectorcount[unit]);
				if (result == 'C')
					MEMORY_CopyToMem(DataBuffer, data, realsize);
			}
			else {
				/* there are programs which send the format-command but don't wait for the result (eg xf-tools) */
				SIO_FormatDisk(unit, DataBuffer, realsize, SIO_format_sectorcount[unit]);
				result = 'E';
			}
			break;
		case 0x22:				/* Enhanced Density Format */
		case 0xA2:				/* xf551 hispeed */
			realsize = 128;
			if (realsize == length) {
				result = SIO_FormatDisk(unit, DataBuffer, 128, 1040);
				if (result == 'C')
					MEMORY_CopyToMem(DataBuffer, data, realsize);
			}
			else {
				SIO_FormatDisk(unit, DataBuffer, 128, 1040);
				result = 'E';
			}
			break;
		default:
			result = 'N';
		}
	}
	/* cassette i/o */
	else if (MEMORY_dGetByte(0x300) == 0x60) {
		int storagelength = 0;
		UBYTE gaps = MEMORY_dGetByte(0x30b);
		switch (cmd){
		case 0x52:	/* read */
			/* set expected Gap */
			CASSETTE_AddGap(gaps == 0 ? 2000 : 160);
			SIO_last_op = SIO_LAST_READ;
			SIO_last_drive = 0x61;
			SIO_last_op_time = 0x10;
			/* get record from storage medium */
			storagelength = CASSETTE_Read();
			if (storagelength - 1 != length)	/* includes -1 as error */
				result = 'E';
			else
				result = 'C';
			/* check checksum */
			if (CASSETTE_buffer[length] != SIO_ChkSum(CASSETTE_buffer, length))
				result = 'E';
			/* if all went ok, copy to Atari */
			if (result == 'C')
				MEMORY_CopyToMem(CASSETTE_buffer, data, length);
			break;
		case 0x57:	/* write */
			SIO_last_op = SIO_LAST_WRITE;
			SIO_last_drive = 0x61;
			SIO_last_op_time = 0x10;
			/* put record into buffer */
			MEMORY_CopyFromMem(data, CASSETTE_buffer, length);
			/* eval checksum over buffer data */
			CASSETTE_buffer[length] = SIO_ChkSum(CASSETTE_buffer, length);
			/* add pregap length */
			CASSETTE_AddGap(gaps == 0 ? 3000 : 260);
			/* write full record to storage medium */
			storagelength = CASSETTE_Write(length + 1);
			if (storagelength - 1 != length)	/* includes -1 as error */
				result = 'E';
			else
				result = 'C';
			break;
		default:
			result = 'N';
		}
	}

	switch (result) {
	case 0x00:					/* Device disabled, generate timeout */
		CPU_regY = 138;
		CPU_SetN;
		break;
	case 'A':					/* Device acknowledge */
	case 'C':					/* Operation complete */
		CPU_regY = 1;
		CPU_ClrN;
		break;
	case 'N':					/* Device NAK */
		CPU_regY = 144;
		CPU_SetN;
		break;
	case 'E':					/* Device error */
	default:
		CPU_regY = 146;
		CPU_SetN;
		break;
	}
	CPU_regA = 0;	/* MMM */
	MEMORY_dPutByte(0x0303, CPU_regY);
	MEMORY_dPutByte(0x42,0);
	CPU_SetC;
}

UBYTE SIO_ChkSum(const UBYTE *buffer, int length)
{
#if 0
	/* old, less efficient version */
	int i;
	int checksum = 0;
	for (i = 0; i < length; i++, buffer++) {
		checksum += *buffer;
		while (checksum > 255)
			checksum -= 255;
	}
#else
	int checksum = 0;
	while (--length >= 0)
		checksum += *buffer++;
	do
		checksum = (checksum & 0xff) + (checksum >> 8);
	while (checksum > 255);
#endif
	return checksum;
}

static UBYTE Command_Frame(void)
{
	int unit;
	int sector;
	int realsize;

	sector = CommandFrame[2] | (((UWORD) CommandFrame[3]) << 8);
	unit = CommandFrame[0] - '1';

	if (unit < 0 || unit >= SIO_MAX_DRIVES) {
		/* Unknown device */
		Log_print("Unknown command frame: %02x %02x %02x %02x %02x",
			   CommandFrame[0], CommandFrame[1], CommandFrame[2],
			   CommandFrame[3], CommandFrame[4]);
		TransferStatus = SIO_NoFrame;
		return 0;
	}
	switch (CommandFrame[1]) {
	case 0x4e:				/* Read Status */
#ifdef DEBUG
		Log_print("Read-status frame: %02x %02x %02x %02x %02x",
			CommandFrame[0], CommandFrame[1], CommandFrame[2],
			CommandFrame[3], CommandFrame[4]);
#endif
		DataBuffer[0] = SIO_ReadStatusBlock(unit, DataBuffer + 1);
		DataBuffer[13] = SIO_ChkSum(DataBuffer + 1, 12);
		DataIndex = 0;
		ExpectedBytes = 14;
		TransferStatus = SIO_ReadFrame;
		POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL;
		return 'A';
	case 0x4f:				/* Write status */
#ifdef DEBUG
		Log_print("Write-status frame: %02x %02x %02x %02x %02x",
			CommandFrame[0], CommandFrame[1], CommandFrame[2],
			CommandFrame[3], CommandFrame[4]);
#endif
		ExpectedBytes = 13;
		DataIndex = 0;
		TransferStatus = SIO_WriteFrame;
		return 'A';
	case 0x50:				/* Write */
	case 0x57:
	case 0xD0:				/* xf551 hispeed */
	case 0xD7:
#ifdef DEBUG
		Log_print("Write-sector frame: %02x %02x %02x %02x %02x",
			CommandFrame[0], CommandFrame[1], CommandFrame[2],
			CommandFrame[3], CommandFrame[4]);
#endif
		SIO_SizeOfSector((UBYTE) unit, sector, &realsize, NULL);
		ExpectedBytes = realsize + 1;
		DataIndex = 0;
		TransferStatus = SIO_WriteFrame;
		SIO_last_op = SIO_LAST_WRITE;
		SIO_last_op_time = 10;
		SIO_last_drive = unit + 1;
		return 'A';
	case 0x52:				/* Read */
	case 0xD2:				/* xf551 hispeed */
#ifdef DEBUG
		Log_print("Read-sector frame: %02x %02x %02x %02x %02x",
			CommandFrame[0], CommandFrame[1], CommandFrame[2],
			CommandFrame[3], CommandFrame[4]);
#endif
		SIO_SizeOfSector((UBYTE) unit, sector, &realsize, NULL);
		DataBuffer[0] = SIO_ReadSector(unit, sector, DataBuffer + 1);
		DataBuffer[1 + realsize] = SIO_ChkSum(DataBuffer + 1, realsize);
		DataIndex = 0;
		ExpectedBytes = 2 + realsize;
		TransferStatus = SIO_ReadFrame;
		/* wait longer before confirmation because bytes could be lost */
		/* before the buffer was set (see $E9FB & $EA37 in XL-OS) */
		POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL << 2; 
		if (image_type[unit] == IMAGE_TYPE_VAPI) {
			vapi_additional_info_t *info;
			info = (vapi_additional_info_t *)additional_info[unit];
			if (info == NULL)
				POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL << 2; 
			else
				POKEY_DELAYED_SERIN_IRQ = ((info->vapi_delay_time + 114/2) / 114) - 12;
		} 
#ifndef NO_SECTOR_DELAY
		else if (sector == 1) {
			POKEY_DELAYED_SERIN_IRQ += delay_counter;
			delay_counter = SECTOR_DELAY;
		}
		else {
			delay_counter = 0;
		}
#endif
		SIO_last_op = SIO_LAST_READ;
		SIO_last_op_time = 10;
		SIO_last_drive = unit + 1;
		return 'A';
	case 0x53:				/* Status */
#ifdef DEBUG
		Log_print("Status frame: %02x %02x %02x %02x %02x",
			CommandFrame[0], CommandFrame[1], CommandFrame[2],
			CommandFrame[3], CommandFrame[4]);
#endif
		DataBuffer[0] = SIO_DriveStatus(unit, DataBuffer + 1);
		DataBuffer[1 + 4] = SIO_ChkSum(DataBuffer + 1, 4);
		DataIndex = 0;
		ExpectedBytes = 6;
		TransferStatus = SIO_ReadFrame;
		POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL;
		return 'A';
	/*case 0x66:*/			/* US Doubler Format - I think! */
	case 0x21:				/* Format Disk */
	case 0xa1:				/* xf551 hispeed */
#ifdef DEBUG
		Log_print("Format-disk frame: %02x %02x %02x %02x %02x",
			CommandFrame[0], CommandFrame[1], CommandFrame[2],
			CommandFrame[3], CommandFrame[4]);
#endif
		realsize = SIO_format_sectorsize[unit];
		DataBuffer[0] = SIO_FormatDisk(unit, DataBuffer + 1, realsize, SIO_format_sectorcount[unit]);
		DataBuffer[1 + realsize] = SIO_ChkSum(DataBuffer + 1, realsize);
		DataIndex = 0;
		ExpectedBytes = 2 + realsize;
		TransferStatus = SIO_FormatFrame;
		POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL;
		return 'A';
	case 0x22:				/* Dual Density Format */
	case 0xa2:				/* xf551 hispeed */
#ifdef DEBUG
		Log_print("Format-Medium frame: %02x %02x %02x %02x %02x",
			CommandFrame[0], CommandFrame[1], CommandFrame[2],
			CommandFrame[3], CommandFrame[4]);
#endif
		DataBuffer[0] = SIO_FormatDisk(unit, DataBuffer + 1, 128, 1040);
		DataBuffer[1 + 128] = SIO_ChkSum(DataBuffer + 1, 128);
		DataIndex = 0;
		ExpectedBytes = 2 + 128;
		TransferStatus = SIO_FormatFrame;
		POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL;
		return 'A';
	default:
		/* Unknown command for a disk drive */
#ifdef DEBUG
		Log_print("Command frame: %02x %02x %02x %02x %02x",
			CommandFrame[0], CommandFrame[1], CommandFrame[2],
			CommandFrame[3], CommandFrame[4]);
#endif
		TransferStatus = SIO_NoFrame;
		return 'E';
	}
}

/* Enable/disable the Tape Motor */
void SIO_TapeMotor(int onoff)
{
	/* if sio is patched, do not do anything */
	if (ESC_enable_sio_patch)
		return;
	if (onoff) {
		/* set frame to cassette frame, if not */
		/* in a transfer with an intelligent peripheral */
		if (TransferStatus == SIO_NoFrame || (TransferStatus & 0xfe) == SIO_CasRead) {
			if (CASSETTE_IsSaveFile()) {
				TransferStatus = SIO_CasWrite;
				CASSETTE_TapeMotor(onoff);
				SIO_last_op = SIO_LAST_WRITE;
			}
			else {
				TransferStatus = SIO_CasRead;
				CASSETTE_TapeMotor(onoff);
				POKEY_DELAYED_SERIN_IRQ = CASSETTE_GetInputIRQDelay();
				SIO_last_op = SIO_LAST_READ;
			};
			SIO_last_drive = 0x60;
			SIO_last_op_time = 0x10;
		}
		else {
			CASSETTE_TapeMotor(onoff);
		}
	}
	else {
		/* set frame to none */
		if (TransferStatus == SIO_CasWrite) {
			TransferStatus = SIO_NoFrame;
			CASSETTE_TapeMotor(onoff);
		}
		else if (TransferStatus == SIO_CasRead) {
			TransferStatus = SIO_NoFrame;
			CASSETTE_TapeMotor(onoff);
			POKEY_DELAYED_SERIN_IRQ = 0; /* off */
		}
		else {
			CASSETTE_TapeMotor(onoff);
			POKEY_DELAYED_SERIN_IRQ = 0; /* off */
		}
		SIO_last_op_time = 0;
	}
}

/* Enable/disable the command frame */
void SIO_SwitchCommandFrame(int onoff)
{
	if (onoff) {				/* Enabled */
		if (TransferStatus != SIO_NoFrame)
			Log_print("Unexpected command frame at state %x.", TransferStatus);
		CommandIndex = 0;
		DataIndex = 0;
		ExpectedBytes = 5;
		TransferStatus = SIO_CommandFrame;
	}
	else {
		if (TransferStatus != SIO_StatusRead && TransferStatus != SIO_NoFrame &&
			TransferStatus != SIO_ReadFrame) {
			if (!(TransferStatus == SIO_CommandFrame && CommandIndex == 0))
				Log_print("Command frame %02x unfinished.", TransferStatus);
			TransferStatus = SIO_NoFrame;
		}
		CommandIndex = 0;
	}
}

static UBYTE WriteSectorBack(void)
{
	UWORD sector;
	UBYTE unit;

	sector = CommandFrame[2] + (CommandFrame[3] << 8);
	unit = CommandFrame[0] - '1';
	if (unit >= SIO_MAX_DRIVES)		/* UBYTE range ! */
		return 0;
	switch (CommandFrame[1]) {
	case 0x4f:				/* Write Status Block */
		return SIO_WriteStatusBlock(unit, DataBuffer);
	case 0x50:				/* Write */
	case 0x57:
	case 0xD0:				/* xf551 hispeed */
	case 0xD7:
		return SIO_WriteSector(unit, sector, DataBuffer);
	default:
		return 'E';
	}
}

/* Put a byte that comes out of POKEY. So get it here... */
void SIO_PutByte(int byte)
{
	switch (TransferStatus) {
	case SIO_CommandFrame:
		if (CommandIndex < ExpectedBytes) {
			CommandFrame[CommandIndex++] = byte;
			if (CommandIndex >= ExpectedBytes) {
				if (CommandFrame[0] >= 0x31 && CommandFrame[0] <= 0x38 && (SIO_drive_status[CommandFrame[0]-0x31] != SIO_OFF || BINLOAD_start_binloading)) {
					TransferStatus = SIO_StatusRead;
					POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL + SIO_ACK_INTERVAL;
				}
				else
					TransferStatus = SIO_NoFrame;
			}
		}
		else {
			Log_print("Invalid command frame!");
			TransferStatus = SIO_NoFrame;
		}
		break;
	case SIO_WriteFrame:		/* Expect data */
		if (DataIndex < ExpectedBytes) {
			DataBuffer[DataIndex++] = byte;
			if (DataIndex >= ExpectedBytes) {
				UBYTE sum = SIO_ChkSum(DataBuffer, ExpectedBytes - 1);
				if (sum == DataBuffer[ExpectedBytes - 1]) {
					UBYTE result = WriteSectorBack();
					if (result != 0) {
						DataBuffer[0] = 'A';
						DataBuffer[1] = result;
						DataIndex = 0;
						ExpectedBytes = 2;
						POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL + SIO_ACK_INTERVAL;
						TransferStatus = SIO_FinalStatus;
					}
					else
						TransferStatus = SIO_NoFrame;
				}
				else {
					DataBuffer[0] = 'E';
					DataIndex = 0;
					ExpectedBytes = 1;
					POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL + SIO_ACK_INTERVAL;
					TransferStatus = SIO_FinalStatus;
				}
			}
		}
		else {
			Log_print("Invalid data frame!");
		}
		break;
	case SIO_CasWrite:
		CASSETTE_PutByte(byte);
		break;
	}
	/* POKEY_DELAYED_SEROUT_IRQ = SIO_SEROUT_INTERVAL; */ /* already set in pokey.c */
}

/* Get a byte from the floppy to the pokey. */
int SIO_GetByte(void)
{
	int byte = 0;

	switch (TransferStatus) {
	case SIO_StatusRead:
		byte = Command_Frame();		/* Handle now the command */
		break;
	case SIO_FormatFrame:
		TransferStatus = SIO_ReadFrame;
		POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL << 3;
		/* FALL THROUGH */
	case SIO_ReadFrame:
		if (DataIndex < ExpectedBytes) {
			byte = DataBuffer[DataIndex++];
			if (DataIndex >= ExpectedBytes) {
				TransferStatus = SIO_NoFrame;
			}
			else {
				/* set delay using the expected transfer speed */
				POKEY_DELAYED_SERIN_IRQ = (DataIndex == 1) ? SIO_SERIN_INTERVAL
					: ((SIO_SERIN_INTERVAL * POKEY_AUDF[POKEY_CHAN3] - 1) / 0x28 + 1);
			}
		}
		else {
			Log_print("Invalid read frame!");
			TransferStatus = SIO_NoFrame;
		}
		break;
	case SIO_FinalStatus:
		if (DataIndex < ExpectedBytes) {
			byte = DataBuffer[DataIndex++];
			if (DataIndex >= ExpectedBytes) {
				TransferStatus = SIO_NoFrame;
			}
			else {
				if (DataIndex == 0)
					POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL + SIO_ACK_INTERVAL;
				else
					POKEY_DELAYED_SERIN_IRQ = SIO_SERIN_INTERVAL;
			}
		}
		else {
			Log_print("Invalid read frame!");
			TransferStatus = SIO_NoFrame;
		}
		break;
	case SIO_CasRead:
		byte = CASSETTE_GetByte();
		POKEY_DELAYED_SERIN_IRQ = CASSETTE_GetInputIRQDelay();
		break;
	default:
		break;
	}
	return byte;
}

#if !defined(BASIC) && !defined(__PLUS)
int SIO_RotateDisks(void)
{
	char tmp_filenames[SIO_MAX_DRIVES][FILENAME_MAX];
	int i;
	int bSuccess = TRUE;

	for (i = 0; i < SIO_MAX_DRIVES; i++) {
		strcpy(tmp_filenames[i], SIO_filename[i]);
		SIO_Dismount(i + 1);
	}

	for (i = 1; i < SIO_MAX_DRIVES; i++) {
		if (strcmp(tmp_filenames[i], "None") && strcmp(tmp_filenames[i], "Off") && strcmp(tmp_filenames[i], "Empty") ) {
			if (!SIO_Mount(i, tmp_filenames[i], FALSE)) /* Note that this is NOT i-1 because SIO_Mount is 1 indexed */
				bSuccess = FALSE;
		}
	}

	i = SIO_MAX_DRIVES - 1;
	while (i > -1 && (!strcmp(tmp_filenames[i], "None") || !strcmp(tmp_filenames[i], "Off") || !strcmp(tmp_filenames[i], "Empty")) ) {
		i--;
	}

	if (i > -1)	{
		if (!SIO_Mount(i + 1, tmp_filenames[0], FALSE))
			bSuccess = FALSE;
	}

	return bSuccess;
}
#endif /* !defined(BASIC) && !defined(__PLUS) */

#ifndef BASIC

void SIO_StateSave(void)
{
	int i;

	for (i = 0; i < 8; i++) {
		StateSav_SaveINT((int *) &SIO_drive_status[i], 1);
		StateSav_SaveFNAME(SIO_filename[i]);
	}
}

void SIO_StateRead(void)
{
	int i;

	for (i = 0; i < 8; i++) {
		int saved_drive_status;
		char filename[FILENAME_MAX];

		StateSav_ReadINT(&saved_drive_status, 1);
		SIO_drive_status[i] = (SIO_UnitStatus)saved_drive_status;

		StateSav_ReadFNAME(filename);
		if (filename[0] == 0)
			continue;

		/* If the disk drive wasn't empty or off when saved,
		   mount the disk */
		switch (saved_drive_status) {
		case SIO_READ_ONLY:
			SIO_Mount(i + 1, filename, TRUE);
			break;
		case SIO_READ_WRITE:
			SIO_Mount(i + 1, filename, FALSE);
			break;
		default:
			break;
		}
	}
}

#endif /* BASIC */

/*
vim:ts=4:sw=4:
*/
