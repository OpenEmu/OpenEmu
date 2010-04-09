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

typedef enum
{
 CDIF_FORMAT_AUDIO,
 CDIF_FORMAT_MODE1,
 CDIF_FORMAT_MODE2,
 CDIF_FORMAT_PSX,
 CDIF_FORMAT_CDI
} CDIF_Track_Format;

bool CDIF_Open(const char *device_name);
bool CDIF_Close(void);

bool CDIF_Init(void);
void CDIF_Deinit(void);

int32 CDIF_GetFirstTrack();
int32 CDIF_GetLastTrack();

bool CDIF_GetTrackStartPositionMSF(int32 track, int &min, int &sec, int &frame);
bool CDIF_GetTrackFormat(int32 track, CDIF_Track_Format &format);

uint32 CDIF_GetTrackStartPositionLBA(int32 track);
int CDIF_FindTrackByLBA(uint32 LBA);

bool CDIF_ReadSector(uint8* pBuf, uint8 *SubPWBuf, uint32 sector, uint32 nSectors);
uint32 CDIF_GetSectorCountLBA(void);
bool CDIF_ReadAudioSector(int16 *buffer, uint8 *SubPWBuf, uint32 read_sec);

uint32 CDIF_GetTrackSectorCount(int32 track);

bool CDIF_CheckSubQChecksum(uint8 *SubQBuf);

#endif /* CDROM_INTERFACE_H_INCLUDED */

