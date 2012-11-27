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

// Actor data table header file

#ifndef SAGA_ITEDATA_H
#define SAGA_ITEDATA_H

namespace Saga {

enum ActorFlags {
	kProtagonist	= 0x01,	// (1<<0) Actor is protagonist
	kFollower		= 0x02,	// (1<<1) Actor is follower
	kCycle			= 0x04, // (1<<2) Actor stand has a cycle
	kFaster			= 0x08, // (1<<3) Actor is fast
	kFastest		= 0x10, // (1<<4) Actor is faster
	kExtended		= 0x20, // (1<<5) Actor uses extended sprites
	kUsable			= 0x40, // (1<<6) Actor can be used
	kNoScale		= 0x80  // (1<<7) Actor is not scaled
};

struct ActorTableData {
	byte flags;
	byte nameIndex;
	int32 sceneIndex;
	int16 x;
	int16 y;
	int16 z;
	int32 spriteListResourceId;
	int32 frameListResourceId;
	byte scriptEntrypointNumber;
	byte speechColor;
	byte currentAction;
	byte facingDirection;
	byte actionDirection;
};

#define ITE_ACTORCOUNT 181

extern ActorTableData ITE_ActorTable[ITE_ACTORCOUNT];

enum {
	kObjUseWith = 0x01,
	kObjNotFlat = 0x02
};

struct ObjectTableData {
	byte nameIndex;
	int32 sceneIndex;
	int16 x;
	int16 y;
	int16 z;
	int32 spriteListResourceId;
	byte scriptEntrypointNumber;
	uint16 interactBits;
};

struct IteFxTable {
	byte res;
	byte vol;
};

#define ITE_OBJECTCOUNT 39
#define ITE_SFXCOUNT 63

extern ObjectTableData ITE_ObjectTable[ITE_OBJECTCOUNT];
extern IteFxTable ITE_SfxTable[ITE_SFXCOUNT];

extern const char *ITEinterfaceTextStrings[][53];

#define PUZZLE_PIECES 15

struct RawPoint { int x, y; };
extern const RawPoint pieceOrigins[PUZZLE_PIECES];
extern const char *pieceNames[][PUZZLE_PIECES];

#define NUM_SOLICIT_REPLIES 5
extern const char *solicitStr[][NUM_SOLICIT_REPLIES];

#define NUM_SAKKA 3
extern const char *sakkaStr[][NUM_SAKKA];

#define NUM_WHINES 5
extern const char *whineStr[][NUM_WHINES];

extern const char *hintStr[][4];
extern const char portraitList[];
extern const char *optionsStr[][4];

} // End of namespace Saga

#endif
