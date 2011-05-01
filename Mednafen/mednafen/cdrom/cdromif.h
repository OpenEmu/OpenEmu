/******************************************************************************
	[CdromInterface.h]

	CD-ROM デバイスを操作するためのインタフェイスを定義します。
	Define interface for controlling CD-ROM device.

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
******************************************************************************/
#ifndef CDROM_INTERFACE_H_INCLUDED
#define CDROM_INTERFACE_H_INCLUDED

#include "cdromfile-stuff.h"

bool CDIF_Open(const char *device_name);
bool CDIF_Close(void);

// Basic functions
bool CDIF_ReadTOC(CD_TOC *toc);

// lba_end is NOT inclusive.  IE for a count of 1, lba_end will be lba + 1.
// Passing ~0U for "lba_end" is equivalent to passing the LBA of the leadout track.
bool CDIF_HintReadSector(uint32 lba);
bool CDIF_ReadRawSector(uint8 *buf, uint32 lba);

// Call for mode 1 or mode 2 form 1 only.
// Will only evaluate checksum and L-EC data if cdrom.lec_eval setting is true(the default), or the disc is real/physical.
bool CDIF_ValidateRawSector(uint8 *buf);

// Utility/Wrapped functions
bool CDIF_ReadSector(uint8* pBuf, uint32 sector, uint32 nSectors);

uint32 CDIF_GetTrackStartPositionLBA(int32 track);
int CDIF_FindTrackByLBA(uint32 LBA);

uint32 CDIF_GetTrackSectorCount(int32 track);

bool CDIF_CheckSubQChecksum(uint8 *SubQBuf);

#endif /* CDROM_INTERFACE_H_INCLUDED */

