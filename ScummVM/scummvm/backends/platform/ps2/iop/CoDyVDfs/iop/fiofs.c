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

#include "cdtypes.h"
#include "codyvdfs.h"
#include <iomanX.h>
#include <io_common.h>
#include <errno.h>
#include <sysmem.h>
#include <sysclib.h>
#include <sys/stat.h>

#define MAX_DIO_HANDLES 8
#define MAX_FIO_HANDLES 32

typedef struct {
	int lba, size, lbaOfs;
	uint32 curOfs;
	uint8 *buf;
} DioHandle;

typedef struct {
	int lba, size;
	int pos;
	uint8 *buf;
	int cachedLba;
} FioHandle;

static DioHandle dioHandles[MAX_DIO_HANDLES];
static FioHandle fioHandles[MAX_FIO_HANDLES];

int initFio(void) {
	int i;

	for (i = 0; i < MAX_DIO_HANDLES; i++)
		dioHandles[i].lba = 0;
	for (i = 0; i < MAX_FIO_HANDLES; i++)
		fioHandles[i].lba = 0;
	return 0;
}

int allocDioHandle(void) {
	int i;
	for (i = 0; i < MAX_DIO_HANDLES; i++)
		if (!dioHandles[i].lba)
			return i;
	return -1;
}

int allocFioHandle(void) {
	int i;
	for (i = 0; i < MAX_FIO_HANDLES; i++)
		if (!fioHandles[i].lba)
			return i;
	return -1;
}

int cd_open(iop_file_t *handle, const char *name, int mode) {
	int	fdSlot;
	FioHandle *fd;
	ISODirectoryRecord *rec;

	if (verifyDriveReady() < 0)
		return -EIO;

	rec = findPath(name);
	if (!rec)
		return -ENOENT;

	if (IS_DIR(rec))
		return -EISDIR;

	fdSlot = allocFioHandle();
	if (fdSlot < 0)
		return -ENFILE;

	fd = fioHandles + fdSlot;

	fd->buf = AllocSysMemory(ALLOC_FIRST, 0x800, NULL);
	if (!fd->buf)
		return -ENOMEM;

	fd->lba = READ_ARRAY32(rec->lba);
	fd->size = READ_ARRAY32(rec->size);
	fd->pos = 0;
	fd->cachedLba = 0;

	handle->privdata = (void*)fdSlot;
	return 0;
}

int cd_lseek(iop_file_t *handle, int ofs, int whence) {
	FioHandle *fd = fioHandles + (int)handle->privdata;
	int newOfs;
	switch(whence) {
		case SEEK_SET:
			newOfs = ofs;
			break;
		case SEEK_CUR:
			newOfs = fd->pos + ofs;
			break;
		case SEEK_END:
			newOfs = fd->size + ofs;
			break;
		default:
			newOfs = -1;
	}
	if ((newOfs >= 0) && (newOfs <= fd->size)) {
		fd->pos = newOfs;
		return newOfs;
	} else
		return -1;
}

int cd_read(iop_file_t *handle, void *dest, int length) {
	FioHandle *fd = fioHandles + (int)handle->privdata;
	CdRMode rmode = { 16, 0, CdSect2048, 0 };
	int readLba, readPos, bytesLeft;
	uint8 *destPos = (uint8*)dest;
	int doCopy;
	int numLba;
	readLba = fd->lba + (fd->pos >> 11);
	readPos = fd->pos & 0x7FF;

	if (fd->pos + length > fd->size)
		length = fd->size - fd->pos;

	if (length < 0)
		return 0;

	bytesLeft = length;
	while (bytesLeft > 0) {
		if (readPos || (bytesLeft < 0x800) || (readLba == fd->cachedLba)) {
			if (fd->cachedLba != readLba) {
				if (cdReadSectors(readLba, 1, fd->buf, &rmode) == 0)
					fd->cachedLba = readLba;
				else
					break; // read error
			}
			doCopy = 0x800 - readPos;
			if (doCopy > bytesLeft)
				doCopy = bytesLeft;

			memcpy(destPos, fd->buf + readPos, doCopy);
			readPos += doCopy;
			readLba += readPos >> 11;
			readPos &= 0x7FF;
			bytesLeft -= doCopy;
			fd->pos += doCopy;
			destPos += doCopy;
		} else {
			numLba = bytesLeft >> 11;
			if (cdReadSectors(readLba, numLba, destPos, &rmode) != 0)
				break;
            readLba += numLba;
			fd->pos += numLba << 11;
			destPos += numLba << 11;
			bytesLeft &= 0x7FF;
		}
	}
    return destPos - (uint8*)dest;
}

int cd_close(iop_file_t *handle) {
	FioHandle *hd = fioHandles + (int)handle->privdata;
	FreeSysMemory(hd->buf);
	hd->lba = hd->pos = hd->size = 0;
	hd->buf = 0;
	return 0;
}

int cd_dopen(iop_file_t *handle, const char *path) {
	CdRMode rmode = { 16, 0, CdSect2048, 0 };
	int fdSlot;
	ISODirectoryRecord *rec;

	if (verifyDriveReady() < 0)
		return -EIO;

	rec = findPath(path);
	if (!rec || !IS_DIR(rec))
		return -ENOENT;

	fdSlot = allocDioHandle();

	if (fdSlot < 0)
		return -ENFILE;

	dioHandles[fdSlot].buf = AllocSysMemory(ALLOC_FIRST, 0x800, NULL);
	if (!dioHandles[fdSlot].buf)
		return -ENOMEM;

	dioHandles[fdSlot].lba = READ_ARRAY32(rec->lba);
	dioHandles[fdSlot].size = READ_ARRAY32(rec->size);
	if (cdReadSectors(dioHandles[fdSlot].lba, 1, dioHandles[fdSlot].buf, &rmode) != 0) {
		dioHandles[fdSlot].lba = dioHandles[fdSlot].size = 0;
		FreeSysMemory(dioHandles[fdSlot].buf);
		dioHandles[fdSlot].buf = NULL;
		return -EIO;
	}
	dioHandles[fdSlot].curOfs = 0;
	dioHandles[fdSlot].lbaOfs = 0;
	handle->privdata = (void*)fdSlot;
	return fdSlot;
}

int cd_dread(iop_file_t *handle, iox_dirent_t *buf) {
	CdRMode rmode = { 16, 0, CdSect2048, 0 };
	ISODirectoryRecord *rec;
	DioHandle *hd = dioHandles + (int)handle->privdata;
	int i;
	for (i = hd->lbaOfs; i < NUM_SECTORS(hd->size); i++) {
		if (i != hd->lbaOfs) {
			cdReadSectors(hd->lba + i, 1, hd->buf, &rmode);
			hd->lbaOfs = i;
			hd->curOfs = 0;
		}
		while ((hd->curOfs < SECTOR_SIZE) && ((ISODirectoryRecord *)(hd->buf + hd->curOfs))->len_dr) {
			rec = (ISODirectoryRecord *)(hd->buf + hd->curOfs);
			hd->curOfs += rec->len_dr;
			if ((rec->len_fi != 1) || ((rec->name[0] != 0) && (rec->name[0] != 1))) { // skip '.' / '..'
				memcpy(buf->name, rec->name, rec->len_fi);
				if ((buf->name[rec->len_fi - 2] == ';') && (buf->name[rec->len_fi - 1] == '1'))
					buf->name[rec->len_fi - 2] = '\0';
				else
					buf->name[rec->len_fi] = '\0';
				buf->stat.mode = FIO_S_IRUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IXOTH;
				buf->stat.mode |= IS_DIR(rec) ? FIO_S_IFDIR : FIO_S_IFREG;
				buf->stat.attr = 0;
				buf->stat.size = READ_ARRAY32(rec->size);
				buf->stat.hisize = 0;
				return 1;
			}
		}
	}
    return 0;
}

int cd_dclose(iop_file_t *handle) {
	DioHandle *hd;
	hd = dioHandles + (int)handle->privdata;
	FreeSysMemory(hd->buf);
	hd->size = hd->lba = 0;
	return 0;
}
