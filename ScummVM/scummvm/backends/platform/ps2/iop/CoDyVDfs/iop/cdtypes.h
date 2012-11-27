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

#ifndef __CDTYPES_H__
#define __CDTYPES_H__

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef signed int int32;
typedef signed short int16;
typedef signed char int8;

typedef struct {
	uint8	len_di;
	uint8	attributeLength;
	uint8	lba[4];
	uint16	parent;
	char	name[256]; // arbitrary number
} ISOPathTableRecord __attribute__ ((packed));

typedef struct {
    uint8 year; // Number of years since 1900
    uint8 month; // Month of the year from 1 to 12
    uint8 day; // Day of the Month from 1 to 31
    uint8 hour; // Hour of the day from 0 to 23
    uint8 min; // Minute of the hour from 0 to 59
    uint8 sec; // second of the minute from 0 to 59
    uint8 gmtOff; // Offset from Greenwich Mean Time in number of 15 minute intervals from -48(West) to +52(East)
	uint8 padding[10];
} ISOTime __attribute__ ((packed));

typedef struct {
    uint8 year; // Number of years since 1900
    uint8 month; // Month of the year from 1 to 12
    uint8 day; // Day of the Month from 1 to 31
    uint8 hour; // Hour of the day from 0 to 23
    uint8 min; // Minute of the hour from 0 to 59
    uint8 sec; // second of the minute from 0 to 59
    uint8 gmtOff; // Offset from Greenwich Mean Time in number of 15 minute intervals from -48(West) to +52(East)
	//uint8 padding[10];
} ISOFileTime __attribute__ ((packed));

typedef struct {
	uint8 len_dr;
	uint8 attributeLength;
	uint8 lba[4];
	uint8 lba_BE[4];
	uint8 size[4];
	uint8 size_BE[4];
	ISOFileTime time;
	uint8 flags;
	uint8 fieldSize;
	uint8 gapSize;
	uint8 sequenceNumber[4];
	uint8 len_fi;
	char name[256]; // arbitrary number
} ISODirectoryRecord __attribute__ ((packed));

typedef struct {
	char volumeSetId[128];
	char publisherId[128];
	char preparerId[128];
	char applicationId[128];
	char copyrightId[37];
	char abstractId[37];
	char bibliographicId[37];
} ISOIds __attribute__ ((packed));

typedef struct {
	uint16	length;
	uint32 tocLBA;
	uint32 tocLBA_bigend;
	uint32 tocSize;
	uint32 tocSize_bigend;
	uint8	dateStamp[8];
	uint8	reserved[6];
	uint8	reserved2;
	uint8	reserved3;
} ISORoot __attribute__((packed));	// 0x22

typedef struct {
	uint8	type;					// 0x00
	char	identifier[5];			// 0x01
	uint8	version;				// 0x06
	uint8	reserved1;				// 0x07
	char	systemIdentifier[32];	// 0x08
	char	volumeIdentifier[32];	// 0x28
	uint8	reserved2[8];			// 0x48
	uint32	volumeSpaceSize;		// 0x50
	uint32	volumeSpaceSizeBE;		// 0x54
	char	reserved3[32];			// 0x58
	uint32	volumeSetSize;			// 0x78
	uint32	volumeSequenceNumber;	// 0x7C
	uint32	logicalBlockSize;		// 0x80
	uint32	pathTableSize;			// 0x84
	uint32	pathTableSizeBE;		// 0x88
	uint32	pathTablePos;			// 0x8C
	uint32	pathTable2Pos;			// 0x90
	uint32	pathTablePosBE;			// 0x94
	uint32	pathTable2PosBE;		// 0x98
	ISORoot	rootDir;				// 0x9C
	ISOIds	ids;					// 0xBE
	ISOTime creation;				// 0x32D
	ISOTime	modification;			// 0x33E
	ISOTime expiration;				// 0x34F
	ISOTime effective;				// 0x360
	uint8	fileStructureVersion;	// 0x371
	uint8	reserved4;				// 0x372
	uint8	applicationUse[512];	// 0x373
	uint8	reserved5[653];			// 0x573
} ISOPvd __attribute__ ((packed));	// 0x800

#endif // __CDTYPES_H__
