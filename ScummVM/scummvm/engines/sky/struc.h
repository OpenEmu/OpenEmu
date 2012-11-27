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

#ifndef SKY_STRUC_H
#define SKY_STRUC_H

namespace Sky {

struct DisplayedText {
	byte *textData;
	uint32 textWidth;
	uint16 compactNum;
};

#include "common/pack-start.h"	// START STRUCT PACKING

struct DataFileHeader {
	uint16 flag; // bit 0: set for color data, clear for not
	// bit 1: set for compressed, clear for uncompressed
	// bit 2: set for 32 colors, clear for 16 colors
	uint16 s_x;
	uint16 s_y;
	uint16 s_width;
	uint16 s_height;
	uint16 s_sp_size;
	uint16 s_tot_size;
	uint16 s_n_sprites;
	int16 s_offset_x;
	int16 s_offset_y;
	uint16 s_compressed_size;
} PACKED_STRUCT;

struct TurnTable {
	uint16 turnTableUp[5];
	uint16 turnTableDown[5];
	uint16 turnTableLeft[5];
	uint16 turnTableRight[5];
	uint16 turnTableTalk[5];
} PACKED_STRUCT;

struct MegaSet {
	uint16 gridWidth;	 //  0
	uint16 colOffset;	 //  1
	uint16 colWidth;	 //  2
	uint16 lastChr;		 //  3

	uint16 animUpId;	 //  4
	uint16 animDownId;	 //  5
	uint16 animLeftId;	 //  6
	uint16 animRightId;	 //  7

	uint16 standUpId;	 //  8
	uint16 standDownId;	 //  9
	uint16 standLeftId;	 // 10
	uint16 standRightId; // 11
	uint16 standTalkId;	 // 12
	uint16 turnTableId;	 // 13
} PACKED_STRUCT;

struct Compact {
	uint16 logic;		 //  0: Entry in logic table to run (byte as <256entries in logic table
	uint16 status;		 //  1
	uint16 sync;		 //  2: flag sent to compacts by other things

	uint16 screen;		 //  3: current screen
	uint16 place;		 //  4: so's this one
	uint16 getToTableId; //  5: Address of how to get to things table

	uint16 xcood;		 //  6
	uint16 ycood;		 //  7

	uint16 frame;		 //  8

	uint16 cursorText;	 //  9
	uint16 mouseOn;		 // 10
	uint16 mouseOff;	 // 11
	uint16 mouseClick;	 // 12

	int16 mouseRelX;	 // 13
	int16 mouseRelY;	 // 14
	uint16 mouseSizeX;	 // 15
	uint16 mouseSizeY;	 // 16

	uint16 actionScript; // 17

	uint16 upFlag;		 // 18: usually holds the Action Mode
	uint16 downFlag;	 // 19: used for passing back
	uint16 getToFlag;	 // 20: used by action script for get to attempts, also frame store (hence word)
	uint16 flag;		 // 21: a use any time flag

	uint16 mood;		 // 22: high level - stood or not

	uint16 grafixProgId; // 23
	uint16 grafixProgPos;// 24

	uint16 offset;		 // 25

	uint16 mode;		 // 26: which mcode block

	uint16 baseSub;		 // 27: 1st mcode block relative to start of compact
	uint16 baseSub_off;	 // 28
	uint16 actionSub;	 // 29
	uint16 actionSub_off;// 30
	uint16 getToSub;	 // 31
	uint16 getToSub_off; // 32
	uint16 extraSub;	 // 33
	uint16 extraSub_off; // 34

	uint16 dir;			 // 35

	uint16 stopScript;	 // 36
	uint16 miniBump;	 // 37
	uint16 leaving;		 // 38
	uint16 atWatch;		 // 39: pointer to script variable
	uint16 atWas;		 // 40: pointer to script variable
	uint16 alt;			 // 41: alternate script
	uint16 request;		 // 42

	uint16 spWidth_xx;	 // 43
	uint16 spColor;	 // 44
	uint16 spTextId;	 // 45
	uint16 spTime;		 // 46

	uint16 arAnimIndex;	 // 47
	uint16 turnProgId;	 // 48
	uint16 turnProgPos;	 // 49

	uint16 waitingFor;	 // 50

	uint16 arTargetX;	 // 51
	uint16 arTargetY;	 // 52

	uint16 animScratchId;// 53: data area for AR

	uint16 megaSet;		 // 54

	MegaSet megaSet0;	 // 55
	MegaSet megaSet1;	 //
	MegaSet megaSet2;	 //
	MegaSet megaSet3;	 //
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

} // End of namespace Sky

#endif
