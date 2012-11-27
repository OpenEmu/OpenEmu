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

#ifndef __CoDyVDfs_H__
#define __CoDyVDfs_H__

#include <cdvdman.h>
#include "cdtypes.h"

typedef cd_read_mode_t CdRMode;

// for reading misaligned ints
#define READ_ARRAY32(a) \
	(a[0] | (a[1] << 8) | (a[2] << 16) | (a[3] << 24))

#define SECTOR_SIZE 0x800
#define SECTOR_MASK 0x7FF
#define NUM_SECTORS(a) ((a + 2047) >> 11)

#define IS_DIR(a)  ((a)->flags & 2)

#define DISC_UNKNOWN 0xFE
#define DISC_NONE  0xFF
#define DISC_DVD   0
#define DISC_MODE1 1
#define DISC_MODE2 2

enum ReadModes {
	CdSect2048 = 0,
	CdSect2328, // 1
	CdSect2340  // 2
};

enum {
    CdDiskNone = 0x00,
    CdDiskDetect, // 0x01
    CdDiskDetectCD, // 0x02
    CdDiskDetectDVD, // 0x03
    CdDiskDetectUnk = 0x05,
    CdDiskCDPS1 = 0x10,
    CdDiskCDDAPS1 = 0x11,
    CdDiskCDPS2 = 0x12,
    CdDiskCDDAPS2 = 0x13,
    CdDiskDVDPS2 = 0x14,
    CdDiskDVDV2 = 0xFC,
    CdDiskCDDA = 0xFD,
    CdDiskDVDV = 0xFE,
    CdDiskIllegal = 0xFF
};

#define DISC_NOT_READY(type) ((type > CdDiskNone) && (type < CdDiskCDPS1) && (type != CdDiskDetectUnk))

int initRpc(void);

int verifyDriveReady(void);
int cdReadSectors(int lba, int num, void *dest, CdRMode *rmode);
int initRootCache(void);
ISODirectoryRecord *findPath(const char *path);
int cacheEnterDir(ISODirectoryRecord *dir);
int initDisc(void);
int checkDiscReady(int retries);
int strnicmp(const char *s1, const char *s2, int n);

#endif // __MYCDVDFS_H__
