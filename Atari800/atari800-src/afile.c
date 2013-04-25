/*
 * afile.c - Detection and opening of different Atari file types.
 *
 * Copyright (c) 1998-2008 Atari800 development team (see DOC/CREDITS)
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
#include "atari.h"
#include "afile.h"
#include "binload.h"
#include "cartridge.h"
#include "cassette.h"
#include "gtia.h"
#include "img_tape.h"
#include "log.h"
#include "sio.h"
#include "statesav.h"
#include "util.h"
#ifndef BASIC
#include "ui.h"
#endif /* BASIC */
#ifdef HAVE_LIBZ
#include <zlib.h>
#endif
#include <stdio.h>


int AFILE_DetectFileType(const char *filename)
{
	UBYTE header[4];
	int file_length;
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		return AFILE_ERROR;
	if (fread(header, 1, 4, fp) != 4) {
		fclose(fp);
		return AFILE_ERROR;
	}
	switch (header[0]) {
	case 0:
		if (header[1] == 0 && (header[2] != 0 || header[3] != 0) /* && file_length < 37 * 1024 */) {
			fclose(fp);
			return AFILE_BAS;
		}
		break;
	case 0x1f:
		if (header[1] == 0x8b) {
#ifndef HAVE_LIBZ
			fclose(fp);
			Log_print("\"%s\" is a compressed file.", filename);
			Log_print("This executable does not support compressed files. You can uncompress this file");
			Log_print("with an external program that supports gzip (*.gz) files (e.g. gunzip)");
			Log_print("and then load into this emulator.");
			return AFILE_ERROR;
#else /* HAVE_LIBZ */
			gzFile gzf;
			fclose(fp);
			gzf = gzopen(filename, "rb");
			if (gzf == NULL)
				return AFILE_ERROR;
			if (gzread(gzf, header, 4) != 4) {
				gzclose(gzf);
				return AFILE_ERROR;
			}
			gzclose(gzf);
			if (header[0] == 0x96 && header[1] == 0x02)
				return AFILE_ATR_GZ;
			if (header[0] == 'A' && header[1] == 'T' && header[2] == 'A' && header[3] == 'R')
				return AFILE_STATE_GZ;
			return AFILE_XFD_GZ;
#endif /* HAVE_LIBZ */
		}
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		if ((header[1] >= '0' && header[1] <= '9') || header[1] == ' ') {
			fclose(fp);
			return AFILE_LST;
		}
		break;
	case 'A':
		if (header[1] == 'T' && header[2] == 'A' && header[3] == 'R') {
			fclose(fp);
			return AFILE_STATE;
		}
		if (header[1] == 'T' && header[2] == '8' && header[3] == 'X') {
			fclose(fp);
			return AFILE_ATX;
		}
		break;
	case 'C':
		if (header[1] == 'A' && header[2] == 'R' && header[3] == 'T') {
			fclose(fp);
			return AFILE_CART;
		}
		break;
	case 0x96:
		if (header[1] == 0x02) {
			fclose(fp);
			return AFILE_ATR;
		}
		break;
	case 0xf9:
	case 0xfa:
		fclose(fp);
		return AFILE_DCM;
	case 0xff:
		if (header[1] == 0xff && (header[2] != 0xff || header[3] != 0xff)) {
			fclose(fp);
			return AFILE_XEX;
		}
		break;
	default:
		break;
	}
	file_length = Util_flen(fp);
	fclose(fp);
	/* Detect .pro images */
	/* # of sectors is in header */
	if ((file_length-16)%(128+12) == 0 &&
			header[0]*256 + header[1] == (file_length-16)/(128+12) &&
			header[2] == 'P') {
#ifdef DEBUG_PRO
		Log_print(".pro file detected");
#endif
		return AFILE_PRO;
	}
	/* 40K or a-power-of-two between 4K and CARTRIDGE_MAX_SIZE */
	if (file_length >= 4 * 1024 && file_length <= CARTRIDGE_MAX_SIZE
	 && ((file_length & (file_length - 1)) == 0 || file_length == 40 * 1024))
		return AFILE_ROM;
	/* BOOT_TAPE is a raw file containing a program booted from a tape */
	if ((header[1] << 7) == file_length)
		return AFILE_BOOT_TAPE;
	if ((file_length & 0x7f) == 0)
		return AFILE_XFD;
	if (IMG_TAPE_FileSupported(header))
		return AFILE_CAS;
	return AFILE_ERROR;
}

int AFILE_OpenFile(const char *filename, int reboot, int diskno, int readonly)
{
	int type = AFILE_DetectFileType(filename);
	switch (type) {
	case AFILE_ATR:
	case AFILE_ATX:
	case AFILE_XFD:
	case AFILE_ATR_GZ:
	case AFILE_XFD_GZ:
	case AFILE_DCM:
	case AFILE_PRO:
		if (!SIO_Mount(diskno, filename, readonly))
			return AFILE_ERROR;
		if (reboot)
			Atari800_Coldstart();
		break;
	case AFILE_XEX:
	case AFILE_BAS:
	case AFILE_LST:
		if (!BINLOAD_Loader(filename))
			return AFILE_ERROR;
		break;
	case AFILE_CART:
	case AFILE_ROM:
		{
			int r;
			if (reboot)
				r = CARTRIDGE_InsertAutoReboot(filename);
			else
				r = CARTRIDGE_Insert(filename);
			switch (r) {
			case CARTRIDGE_CANT_OPEN:
			case CARTRIDGE_BAD_FORMAT:
				return AFILE_ERROR;
			case CARTRIDGE_BAD_CHECKSUM:
			case 0:
				/* ok */
				break;
			default:
#ifdef BASIC
				Log_print("Raw cartridge images are not supported in BASIC version.");
				return AFILE_ERROR;
#else /* BASIC */
				/* r > 0 */
#ifndef ANDROID
				CARTRIDGE_SetTypeAutoReboot(&CARTRIDGE_main, UI_SelectCartType(r));
#else
				return (r << 8) | AFILE_ROM;
#endif /* ANDROID */
				break;
#endif /* BASIC */
			}
		}
		break;
	case AFILE_CAS:
	case AFILE_BOOT_TAPE:
		if (!CASSETTE_Insert(filename))
			return AFILE_ERROR;
		if (reboot) {
			CASSETTE_hold_start = TRUE;
			Atari800_Coldstart();
		}
		break;
	case AFILE_STATE:
	case AFILE_STATE_GZ:
#ifdef BASIC
		Log_print("State files are not supported in BASIC version");
		return AFILE_ERROR;
#else
		if (!StateSav_ReadAtariState(filename, "rb"))
			return AFILE_ERROR;
		/* Don't press Option */
		GTIA_consol_table[1] = GTIA_consol_table[2] = 0xf;
		break;
#endif
	default:
		break;
	}
	return type;
}
