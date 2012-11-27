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

#ifndef AGOS_INTERN_H
#define AGOS_INTERN_H

namespace AGOS {

enum ChildType {
	kRoomType = 1,
	kObjectType = 2,
	kPlayerType = 3,
	kGenExitType = 4,   // Elvira 1 specific
	kSuperRoomType = 4, // Elvira 2 specific

	kContainerType = 7,
	kChainType = 8,
	kUserFlagType = 9,

	kInheritType = 255
};

struct Child {
	Child *next;
	uint16 type;
};

struct SubRoom : Child {
	uint16 subroutine_id;
	uint16 roomExitStates;
	uint16 roomExit[1];
	uint16 roomShort;
	uint16 roomLong;
	uint16 flags;
};

struct SubSuperRoom : Child {
	uint16 subroutine_id;
	uint16 roomX;
	uint16 roomY;
	uint16 roomZ;
	uint16 roomExitStates[1];
};

struct SubObject : Child {
	uint16 objectName;
	uint16 objectSize;
	uint16 objectWeight;
	uint32 objectFlags;
	int16 objectFlagValue[1];
};

struct SubPlayer : Child {
	int16 userKey;
	int16 size;
	int16 weight;
	int16 strength;
	int16 flags;
	int16 level;
	int32 score;
};

struct SubGenExit : Child {
	uint16 subroutine_id;
	uint16 dest[6];
};

struct SubContainer : Child {
	uint16 subroutine_id;
	uint16 volume;
	uint16 flags;
};

struct SubChain : Child {
	uint16 subroutine_id;
	uint16 chChained;
};

struct SubUserFlag : Child {
	uint16 subroutine_id;
	uint16 userFlags[8];
	uint16 userItems[1];
};

struct SubInherit : Child {
	uint16 subroutine_id;
	uint16 inMaster;
};

enum {
	SubRoom_SIZE = sizeof(SubRoom) - sizeof(uint16),
	SubSuperRoom_SIZE = sizeof(SubSuperRoom) - sizeof(uint16),
	SubObject_SIZE = sizeof(SubObject) - sizeof(int16)
};

struct RoomState {
	uint16 state;
	uint16 classFlags;
	uint16 roomExitStates;

	RoomState() { memset(this, 0, sizeof(*this)); }
};

struct Item {
	uint16 parent;
	uint16 child;
	uint16 next;
	int16 noun;
	int16 adjective;
	int16 state;										/* signed int */
	uint16 classFlags;
	uint16 itemName;
	Child *children;

	Item() { memset(this, 0, sizeof(*this)); }
};

struct IconEntry {
	Item *item;
	uint16 boxCode;
};

struct IconBlock {
	int16 line;
	Item *itemRef;
	IconEntry iconArray[64];
	int16 upArrow, downArrow;
	uint16 classMask;
};

struct WindowBlock {
	byte mode;
	byte flags;
	int16 x, y;
	int16 width, height;
	int16 textColumn, textRow;
	int16 scrollY;
	uint16 textColumnOffset, textLength, textMaxLength;
	uint8 fillColor, textColor;
	IconBlock *iconPtr;
	WindowBlock() { memset(this, 0, sizeof(*this)); }
	~WindowBlock() { free (iconPtr); }
};
// note on text offset:
// the actual x-coordinate is: textColumn * 8 + textColumnOffset
// the actual y-coordinate is: textRow * 8

enum {
	SUBROUTINE_LINE_SMALL_SIZE = 2,
	SUBROUTINE_LINE_BIG_SIZE = 8
};

#include "common/pack-start.h"

struct Subroutine {
	uint16 id;								/* subroutine ID */
	uint16 first;								/* offset from subroutine start to first subroutine line */
	Subroutine *next;							/* next subroutine in linked list */
};

struct SubroutineLine {
	uint16 next;
	int16 verb;
	int16 noun1;
	int16 noun2;
};

#include "common/pack-end.h"

struct TimeEvent {
	uint32 time;
	uint16 subroutine_id;
	TimeEvent *next;
};

struct GameSpecificSettings {
	const char *base_filename;
	const char *restore_filename;
	const char *tbl_filename;
	const char *effects_filename;
	const char *speech_filename;
};

enum BoxFlags {
	kBFToggleBox      = 0x1, // Elvira 1/2
	kBFTextBox        = 0x1, // Others
	kBFBoxSelected    = 0x2,
	kBFInvertSelect   = 0x4, // Elvira 1/2
	kBFNoTouchName    = 0x4, // Others
	kBFInvertTouch    = 0x8,
	kBFHyperBox       = 0x10, // Feeble Files
	kBFDragBox        = 0x10, // Others
	kBFBoxInUse       = 0x20,
	kBFBoxDead        = 0x40,
	kBFBoxItem        = 0x80
};

enum OldBoxFlags_PN {
	kOBFObject         = 0x1,
	kOBFExit           = 0x2,
	kOBFDraggable      = 0x4,
	kOBFUseEmptyLine   = 0x8,
	kOBFBoxDisabled    = 0x10,
	kOBFInventoryBox   = 0x20,
	kOBFRoomBox        = 0x40,
	kOBFMoreBox        = 0x80,
	kOBFNoShowName     = 0x100,
	kOBFUseMessageList = 0x400,
	// ScummVM specific
	kOBFBoxSelected    = 0x800,
	kOBFInvertTouch    = 0x1000
};

enum SubObjectFlags {
	kOFText           = 0x1,
	kOFSize           = 0x2,
	kOFWorn           = 0x4, // Elvira 1
	kOFWeight         = 0x4, // Others
	kOFVolume         = 0x8,
	kOFIcon           = 0x10,
	kOFKeyColor1      = 0x20,
	kOFKeyColor2      = 0x40,
	kOFMenu           = 0x80,
	kOFNumber         = 0x100,
	kOFSoft           = 0x200, // Waxworks
	kOFVoice          = 0x200  // Others
};

enum GameFeatures {
	GF_TALKIE          = 1 << 0,
	GF_OLD_BUNDLE      = 1 << 1,
	GF_CRUNCHED        = 1 << 2,
	GF_CRUNCHED_GAMEPC = 1 << 3,
	GF_ZLIBCOMP        = 1 << 4,
	GF_32COLOR         = 1 << 5,
	GF_EGA             = 1 << 6,
	GF_PLANAR          = 1 << 7,
	GF_DEMO            = 1 << 8,
	GF_PACKED          = 1 << 9
};

enum GameFileTypes {
	GAME_BASEFILE = 1 << 0,
	GAME_ICONFILE = 1 << 1,
	GAME_GMEFILE  = 1 << 2,
	GAME_MENUFILE = 1 << 3,
	GAME_STRFILE  = 1 << 4,
	GAME_RMSLFILE = 1 << 5,
	GAME_STATFILE = 1 << 6,
	GAME_TBLFILE  = 1 << 7,
	GAME_XTBLFILE = 1 << 8,
	GAME_RESTFILE = 1 << 9,
	GAME_TEXTFILE = 1 << 10,
	GAME_VGAFILE  = 1 << 11,

	GAME_GFXIDXFILE = 1 << 12
};

enum GameIds {
	GID_PN,
	GID_ELVIRA1,
	GID_ELVIRA2,
	GID_WAXWORKS,

	GID_SIMON1,
	GID_SIMON1DOS,
	GID_SIMON1CD32,

	GID_SIMON2,

	GID_FEEBLEFILES,

	GID_DIMP,
	GID_JUMBLE,
	GID_PUZZLE,
	GID_SWAMPY
};

} // End of namespace AGOS

#endif
