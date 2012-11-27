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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TINSEL_SCN_H	// prevent multiple includes
#define TINSEL_SCN_H

#include "tinsel/dw.h"

namespace Tinsel {

// chunk identifier numbers

// V2 chunks

#define	CHUNK_STRING			0x33340001L	// same in V1 and V2
#define	CHUNK_BITMAP			0x33340002L	// same in V1 and V2
#define	CHUNK_CHARPTR			0x33340003L	// not used!
#define	CHUNK_CHARMATRIX		0x33340004L	// not used!
#define	CHUNK_PALETTE			0x33340005L	// not used!
#define	CHUNK_IMAGE				0x33340006L	// not used!
#define	CHUNK_ANI_FRAME			0x33340007L	// not used!
#define	CHUNK_FILM				0x33340008L	// not used!
#define	CHUNK_FONT				0x33340009L	// not used!
#define	CHUNK_PCODE				0x3334000AL
#define	CHUNK_ENTRANCE			0x3334000BL	// not used!
#define	CHUNK_POLYGONS			0x3334000CL	// not used!
#define	CHUNK_ACTORS			0x3334000DL	// not used!

#define CHUNK_PROCESSES			0x3334000EL // Tinsel 2 only

// Following chunk Ids should be decremented by 1 for Tinsel 1
#define	CHUNK_SCENE				0x3334000FL
#define	CHUNK_TOTAL_ACTORS		0x33340010L
#define	CHUNK_TOTAL_GLOBALS		0x33340011L
#define	CHUNK_TOTAL_OBJECTS		0x33340012L
#define	CHUNK_OBJECTS			0x33340013L
#define	CHUNK_MIDI				0x33340014L	// not used!
#define	CHUNK_SAMPLE			0x33340015L	// not used!
#define	CHUNK_TOTAL_POLY		0x33340016L

// Following chunks are Tinsel 2 only
#define CHUNK_NUM_PROCESSES		0x33340017L	// Master scene only
#define CHUNK_MASTER_SCRIPT		0x33340018L
#define CHUNK_CDPLAY_FILENUM	0x33340019L
#define CHUNK_CDPLAY_HANDLE		0x3334001AL
#define CHUNK_CDPLAY_FILENAME	0x3334001BL
#define CHUNK_MUSIC_FILENAME	0x3334001CL
#define CHUNK_MUSIC_SCRIPT		0x3334001DL
#define CHUNK_MUSIC_SEGMENT		0x3334001EL
#define CHUNK_SCENE_HOPPER		0x3334001FL	// Hopper file only
#define CHUNK_SCENE_HOPPER2		0x33340030L	// Hopper file only
#define CHUNK_TIME_STAMPS		0x33340020L

// This single chunk is common to all Tinsel versions
#define	CHUNK_MBSTRING			0x33340022L

// This is a base, subsequent numbers may also get used
#define CHUNK_GRAB_NAME			0x33340100L

byte *FindChunk(SCNHANDLE handle, uint32 chunk);

} // End of namespace Tinsel

#endif /* TINSEL_SCN_H */
