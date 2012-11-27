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

#ifndef CREATE_DRASCULA_H
#define CREATE_DRASCULA_H

#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))

#define DATAALIGNMENT 4

#define NUM_LANGS 5
#define NUM_TEXT 501
#define NUM_TEXTD 84
#define NUM_TEXTB 15
#define NUM_TEXTBJ 29
#define NUM_TEXTE 24
#define NUM_TEXTI 33
#define NUM_TEXTL 32
#define NUM_TEXTP 20
#define NUM_TEXTT 25
#define NUM_TEXTVB 63
#define NUM_TEXTSYS 4
#define NUM_TEXTHIS 5
#define NUM_TEXTVERBS 6
#define NUM_TEXTMISC 3
#define NUM_TEXTD1 11

typedef unsigned char   uint8;
typedef unsigned short uint16;
typedef signed short int16;

enum Verbs {
	kVerbDefault = -1,
	kVerbLook = 1,
	kVerbPick = 2,
	kVerbOpen = 3,
	kVerbClose = 4,
	kVerbTalk = 5,
	kVerbMove = 6
};

struct RoomTalkAction {
	int room;
	int chapter;
	int action;
	int objectID;
	int speechID;
};

struct ItemLocation {
	int x;
	int y;
};

struct CharInfo {
	char inChar;
	uint16 mappedChar;
	char charType;	// 0 - letters, 1 - signs, 2 - accented
};

struct RoomUpdate {
	int roomNum;
	int flag;
	int flagValue;
	int sourceX;
	int sourceY;
	int destX;
	int destY;
	int width;
	int height;
	int type;	// 0 - background, 1 - rect
};

enum TalkSequenceCommands {
	kPause = 0,
	kSetFlag = 1,
	kClearFlag = 2,
	kPickObject = 3,
	kAddObject = 4,
	kBreakOut = 5,
	kConverse = 6,
	kPlaceVB = 7,
	kUpdateRoom = 8,
	kUpdateScreen = 9,
	kTrackProtagonist = 10,
	kPlaySound = 11,
	kFinishSound = 12,
	kTalkerGeneral = 13,
	kTalkerDrunk = 14,
	kTalkerPianist = 15,
	kTalkerBJ = 16,
	kTalkerVBNormal = 17,
	kTalkerVBDoor = 18,
	kTalkerIgorSeated = 19,
	kTalkerWerewolf = 20,
	kTalkerMus = 21,
	kTalkerDrascula = 22,
	kTalkerBartender0 = 23,
	kTalkerBartender1 = 24
};

struct TalkSequenceCommand {
	int chapter;
	int sequence;
	int commandType;
	int action;
};

#endif /* CREATE_DRASCULA_H */
