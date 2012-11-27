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

#include <types.h>
#include <defs.h>
#include <irx.h>
#include <sys/stat.h>

#include <stdio.h>
#include <sysclib.h>
#include <sysmem.h>
#include <thbase.h>
#include <iomanX.h>

#include <io_common.h>
#include <errno.h>

#include "fiofs.h"
#include "codyvdfs.h"
#include "../common/codyvdirx.h"

#define DBG_PRINTF printf

char cachedDir[256];
int cachedDirLba, cachedDirOfs, cachedDirSize;
static uint8 cacheBuf[2340];

int mediaType, fsRootLba, fsRootSize;

int verifyDriveReady(void) {
	int res;
	u32 trayStat;
	res = CdTrayReq(CdTrayCheck, &trayStat);
	if ((mediaType == DISC_UNKNOWN) || (res == 0) || (trayStat == 1)) {
		// media was exchanged
		if (checkDiscReady(100) == 0) // wait up to 1 second
			return initDisc();
		else {
			mediaType = DISC_UNKNOWN;
			return -1; // drive still not ready
		}
	}
    if (mediaType == DISC_NONE)
		return -1;
	return 0;
}

int cacheEnterDir(ISODirectoryRecord *dir) {
	CdRMode rmode = { 16, 0, CdSect2048, 0 };
	char *cacheName;
	cachedDirLba = READ_ARRAY32(dir->lba);
	cachedDirSize = READ_ARRAY32(dir->size);
	cachedDirOfs = 0;
	cacheName = cachedDir + strlen(cachedDir);
	memcpy(cacheName, dir->name, dir->len_fi);
    cacheName[dir->len_fi] = '/';
	cacheName[dir->len_fi + 1] = '\0';
	return cdReadSectors(cachedDirLba, 1, cacheBuf, &rmode);
}

int initRootCache(void) {
	CdRMode rmode = { 16, 0, CdSect2048, 0 };
	ISODirectoryRecord *root = (ISODirectoryRecord*)cacheBuf;

	if (cdReadSectors(fsRootLba, 1, cacheBuf, &rmode) == 0) {
		cachedDir[0] = '\0';
		fsRootSize = READ_ARRAY32(root->size);
		cachedDirLba = fsRootLba;
		cachedDirOfs = 0;
		cachedDirSize = fsRootSize;
		if (READ_ARRAY32(root->lba) == fsRootLba)
			return 0;
	}
	DBG_PRINTF("Can't read root directory table in sector %d\n", fsRootLba);
	mediaType = DISC_NONE;
	return -1;
}

ISODirectoryRecord *findEntryInCache(const char *name, int nameLen) {
	int i;
	CdRMode rmode = { 16, 0, CdSect2048, 0 };
	ISODirectoryRecord *entry;
	for (i = 0; i < NUM_SECTORS(cachedDirSize); i++) {
		entry = (ISODirectoryRecord *)cacheBuf;
		if (i != cachedDirOfs) {
			if (cdReadSectors(cachedDirLba + i, 1, cacheBuf, &rmode) < 0)
				return NULL;
            cachedDirOfs = i;
		}

		while (entry->len_dr && ((uint8*)entry < cacheBuf + SECTOR_SIZE)) {
			if ((entry->len_fi > 2) && (entry->name[entry->len_fi - 2] == ';') && (entry->name[entry->len_fi - 1] == '1')) {
				if ((nameLen == entry->len_fi - 2) && (strnicmp(name, entry->name, entry->len_fi - 2) == 0))
					return entry;
			} else {
				if ((nameLen == entry->len_fi) && (strnicmp(name, entry->name, entry->len_fi) == 0))
					return entry;
			}
			entry = (ISODirectoryRecord *)( (uint8*)entry + entry->len_dr );
		}
	}
    return NULL;
}

ISODirectoryRecord *findPath(const char *path) {
	ISODirectoryRecord *pathPos;
	const char *tok;

	if (path[0] == '/')
		path++;

	if (strnicmp(cachedDir, path, strlen(cachedDir)) == 0)
		path += strlen(cachedDir); // requested path is somewhere in our cache
	else
		initRootCache(); // not cached, we start again at the cd root

	if (path[0] == '/')
		path++;

	if (!path[0]) { // open root dir
		if (cachedDirOfs)
			initRootCache();
		return (ISODirectoryRecord *)cacheBuf;
	}

	do {
		tok = strchr(path, '/');
		if (tok)
			pathPos = findEntryInCache(path, tok - path);
		else
			pathPos = findEntryInCache(path, strlen(path));

		if (pathPos && tok && tok[1] && IS_DIR(pathPos))
			if (cacheEnterDir(pathPos) < 0)
				return NULL;

		path = tok + 1;
	} while (pathPos && tok && tok[1]);
	return pathPos;
}

int checkDiscReady(int retries) {
	if (retries == -1) {
		if (CdDiskReady(0) == 2) // block until drive ready, should always return 2
			return 0;
	} else {
		do {
			if (CdDiskReady(1) == 2)
				return 0;
			DelayThread(10 * 1000);
		} while (--retries >= 0);
	}
	return -1;
}


int initDisc(void) {
	int type, sector, discType;
	ISOPvd *pvd;
	CdRMode rmode = { 16, 0, CdSect2048, 0 };
	ISOPathTableRecord *rootRec;

	if (checkDiscReady(0) < 0) {
		printf("disc not ready\n");
		mediaType = DISC_UNKNOWN; // retry later
		return -1;
	}

	do {	// wait until drive detected disc type
		type = CdGetDiskType();
		if (DISC_NOT_READY(type))
			DelayThread(10 * 1000);
	} while (DISC_NOT_READY(type));

	if (type == CdDiskIllegal) {
		printf("Illegal disc type\n");
		mediaType = DISC_NONE;
		return -1;
	}
	if (type == CdDiskNone) {
		printf("Tray empty\n");
		mediaType = DISC_NONE;
		return -1;
	}

	discType = DISC_DVD;
	switch (type) {
		case CdDiskCDPS1:
		case CdDiskCDDAPS1:
		case CdDiskCDPS2:
		case CdDiskCDDAPS2:
		case CdDiskCDDA:
			discType = DISC_MODE2;
			rmode.datapattern = CdSect2340;
		default:
			break;
	}

	for (sector = 16; sector < 32; sector++) {
		printf("sec %d\n", sector);
		if (cdReadSectors(sector, 1, cacheBuf, &rmode) == 0) {
			if (discType == DISC_DVD)
				pvd = (ISOPvd *)cacheBuf;
			else {
				switch (cacheBuf[3]) {
					case 1:
						discType = DISC_MODE1;
						printf("Disc: Mode1\n");
						pvd = (ISOPvd*)(cacheBuf + 4);
						break;
					case 2:
						discType = DISC_MODE2;
						printf("Disc: Mode2\n");
						pvd = (ISOPvd*)(cacheBuf + 12);
						break;
					default:
						DBG_PRINTF("Unknown Sector Type %02X\n", cacheBuf[3]);
						return -1;
				}
			}
			rmode.datapattern = CdSect2048;
			if ((pvd->type == 1) && (memcmp(pvd->identifier, "CD001", 5) == 0)) { // found ISO9660 PVD
				DBG_PRINTF("Found ISO9660 PVD in sector %d\n", sector);

				DBG_PRINTF("reading path table from sector %d\n", pvd->pathTablePos);
				if (cdReadSectors(pvd->pathTablePos, 1, cacheBuf, &rmode) < 0) {
					DBG_PRINTF("Can't read path table\n");
					return -1;
				}

				rootRec = (ISOPathTableRecord *)cacheBuf;
				if ((rootRec->len_di != 1) || (rootRec->name[0] != 0)) {
					DBG_PRINTF("Root entry missing: %02X - %02X\n", rootRec->len_di, rootRec->name[0]);
					return -1;
				}

				fsRootLba = READ_ARRAY32(rootRec->lba); // this points to the root record

				mediaType = discType;
				DBG_PRINTF("Root directory in sector %d\n", fsRootLba);
                return initRootCache();
			}
		}
	}
	mediaType = DISC_NONE;
	// PVD not found
	return -1;
}

int cdReadSectors(int lba, int num, void *dest, CdRMode *rmode) {
	int err;
	if (CdRead(lba, num, dest, rmode) == 1) {
		CdSync(0);
		err = CdGetError();
		return (err > 0) ? -err : err;
	}
	return -0xFF;
}

int cd_dummy(void) {
	printf("cd_dummy called!\n");
	return -1;
}

int cd_init(iop_device_t *dev) {
	printf("FS init\n");
	memset(cachedDir, 0, 256);
	cachedDirLba = cachedDirOfs = 0;
	mediaType = DISC_UNKNOWN;
	return 0;
}

iop_device_ops_t FS_ops = {
    (void *) cd_init,
    (void *) cd_dummy,
    (void *) cd_dummy,
    (void *) cd_open,
    (void *) cd_close,
    (void *) cd_read,
    (void *) cd_dummy,
    (void *) cd_lseek,
    (void *) cd_dummy,
    (void *) cd_dummy,
    (void *) cd_dummy,
    (void *) cd_dummy,
    (void *) cd_dopen,
    (void *) cd_dclose,
    (void *) cd_dread,
    (void *) cd_dummy,
    (void *) cd_dummy,
};

#define FS_NAME "cdfs"
#define FS_DESC "CD-ROM"

iop_device_t fsdriver = {
    FS_NAME,
    IOP_DT_FS | IOP_DT_FSEXT,
    1,
    FS_DESC,
    &FS_ops
};

int _start(void) {
    printf("CoDyVDfs v0.01\n");

	CdInit(1);
    DelDrv(FS_NAME);
    AddDrv(&fsdriver);

	initRpc();
	initFio();
    return(0);
}

int strnicmp(const char *s1, const char *s2, int n) {
	if (n) {
		do {
			if (tolower(*s1) != tolower(*s2))
				return tolower(*s1) - tolower(*s2);
			if (*s1++ == '\0')
				break;
			s2++;
		} while (--n);
	}
	return 0;
}
