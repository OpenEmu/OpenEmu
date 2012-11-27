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

#include "common/array.h"
#include "common/textconsole.h"

#include "drascula/drascula.h"

namespace Drascula {

struct doorInfo {
	int chapter;
	int doorNum;
	int flag;
};

static const doorInfo doors[] = {
	{	2,	138,	 0 },	{	2,	136,	 8 },
	{	2,	156,	16 },	{	2,	163,	17 },
	{	2,	177,	15 },	{	2,	175,	40 },
	{	2,	173,	36 },	{	4,	103,	 0 },
	{	4,	104,	 1 },	{	4,	105,	 1 },
	{	4,	106,	 2 },	{	4,	107,	 2 },
	{	4,	110,	 6 },	{	4,	114,	 4 },
	{	4,	115,	 4 },	{	4,	117,	 5 },
	{	4,	120,	 8 },	{	4,	122,	 7 }
};

typedef bool (DrasculaEngine::*RoomParser)(int args);

struct DrasculaRoomParser {
	DrasculaRoomParser(const char* d, RoomParser p) : desc(d), proc(p) {}
	const char* desc;
	RoomParser proc;
};

typedef void (DrasculaEngine::*Updater)();

struct DrasculaUpdater {
	DrasculaUpdater(const char* d, Updater p) : desc(d), proc(p) {}
	const char* desc;
	Updater proc;
};

struct RoomHandlers {
	Common::Array<DrasculaRoomParser *> roomParsers;
	Common::Array<DrasculaUpdater *> roomPreupdaters;
	Common::Array<DrasculaUpdater *> roomUpdaters;
};

#define ROOM(x) _roomHandlers->roomParsers.push_back(new DrasculaRoomParser(#x, &DrasculaEngine::x))
#define PREUPDATEROOM(x) _roomHandlers->roomPreupdaters.push_back(new DrasculaUpdater(#x, &DrasculaEngine::x))
#define UPDATEROOM(x) _roomHandlers->roomUpdaters.push_back(new DrasculaUpdater(#x, &DrasculaEngine::x))

void DrasculaEngine::setupRoomsTable() {
	_roomHandlers = new RoomHandlers();

	ROOM(room_0);	// default
	ROOM(room_1);
	ROOM(room_3);
	ROOM(room_4);
	ROOM(room_5);
	ROOM(room_6);
	ROOM(room_7);
	ROOM(room_8);
	ROOM(room_9);
	ROOM(room_12);
	ROOM(room_13);	// returns bool
	ROOM(room_14);
	ROOM(room_15);
	ROOM(room_16);
	ROOM(room_17);
	ROOM(room_18);
	ROOM(room_21);	// returns bool
	ROOM(room_22);
	ROOM(room_23);
	ROOM(room_24);
	ROOM(room_26);
	ROOM(room_27);
	ROOM(room_29);
	ROOM(room_30);
	ROOM(room_31);
	ROOM(room_34);
	ROOM(room_35);
	ROOM(room_49);
	ROOM(room_53);
	ROOM(room_54);
	ROOM(room_55);
	ROOM(room_56);	// returns bool
	ROOM(room_58);
	ROOM(room_59);
	ROOM(room_60);	// returns bool
	ROOM(room_62);
	ROOM(room_102);

	PREUPDATEROOM(update_1_pre);
	PREUPDATEROOM(update_6_pre);
	PREUPDATEROOM(update_9_pre);
	PREUPDATEROOM(update_14_pre);
	PREUPDATEROOM(update_16_pre);
	PREUPDATEROOM(update_18_pre);
	PREUPDATEROOM(update_23_pre);
	PREUPDATEROOM(update_26_pre);
	PREUPDATEROOM(update_35_pre);
	PREUPDATEROOM(update_58_pre);
	PREUPDATEROOM(update_59_pre);
	PREUPDATEROOM(update_60_pre);
	PREUPDATEROOM(update_62_pre);

	UPDATEROOM(update_2);
	UPDATEROOM(update_3);
	UPDATEROOM(update_4);
	UPDATEROOM(update_13);
	UPDATEROOM(update_26);
	UPDATEROOM(update_58);
	UPDATEROOM(update_60);
	UPDATEROOM(update_62);
	UPDATEROOM(update_102);
}

void DrasculaEngine::freeRoomsTable() {
	if (_roomHandlers == 0)
		return;

	for (uint32 i = 0; i < _roomHandlers->roomParsers.size(); i++)
		delete _roomHandlers->roomParsers[i];
	_roomHandlers->roomParsers.clear();

	for (uint32 i = 0; i < _roomHandlers->roomPreupdaters.size(); i++)
		delete _roomHandlers->roomPreupdaters[i];
	_roomHandlers->roomPreupdaters.clear();

	for (uint32 i = 0; i < _roomHandlers->roomUpdaters.size(); i++)
		delete _roomHandlers->roomUpdaters[i];
	_roomHandlers->roomUpdaters.clear();

	delete _roomHandlers;
	_roomHandlers = 0;
}

bool DrasculaEngine::roomParse(int rN, int fl) {
	bool seen = false;

	for (int i = 0; i < _roomActionsSize; i++) {
		if (_roomActions[i].room == rN) {
			seen = true;
			if (_roomActions[i].chapter == currentChapter ||
				_roomActions[i].chapter == -1) {
				if (_roomActions[i].action == pickedObject ||
					_roomActions[i].action == kVerbDefault) {
					if (_roomActions[i].objectID == fl ||
						_roomActions[i].objectID == -1) {
						talk(_roomActions[i].speechID);
						hasAnswer = 1;
						return true;
					}
				}
			}
		} else if (seen) // Stop searching down the list
			break;
	}

	return false;
}

bool DrasculaEngine::room_0(int fl) {
	(void)fl;

	static const int lookExcuses[3] = {100, 101, 54};
	static const int actionExcuses[6] = {11, 109, 111, 110, 115, 116};

	// non-default actions
	if (currentChapter == 2 || currentChapter == 4 ||
		currentChapter == 5 || currentChapter == 6) {
		if (pickedObject == kVerbLook) {
			talk(lookExcuses[curExcuseLook]);
			curExcuseLook++;
			if (curExcuseLook == 3)
				curExcuseLook = 0;
		} else {
			talk(actionExcuses[curExcuseAction]);
			curExcuseAction++;
			if (curExcuseAction == 6)
				curExcuseAction = 0;
		}
	}

	return true;
}

bool DrasculaEngine::room_1(int fl) {
	if (pickedObject == kVerbLook && fl == 118) {
		talk(1);
		pause(10);
		talk(2);
	} else if (pickedObject == kVerbLook && fl == 120 && flags[8] == 0)
		talk(14);
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_3(int fl) {
	if (pickedObject == kVerbTalk && fl == 129) {
		talk(23);
		pause(6);
		talk_sync(_text[50], "50.als", "11111111111144432554433");
	} else if (pickedObject == kVerbTalk && fl == 133) {
		talk_sync(_text[322], "322.als", "13333334125433333333");
		updateRoom();
		updateScreen();
		pause(25);
		talk(33);
	} else if (pickedObject == kVerbLook && fl == 165) {
		talk(149);
		talk(150);
	} else if (pickedObject == kVerbPick && fl == 165) {
		copyBackground(0, 0, 0,0, 320, 200, bgSurface, screenSurface);
		updateRefresh_pre();
		copyRect(44, 1, curX, curY, 41, 70, drawSurface2, screenSurface);
		updateRefresh();
		updateScreen();
		pause(4);
		pickObject(10);
		flags[3] = 1;
		visible[8] = 0;
	} else if (pickedObject == 14 && fl == 166 && flags[37] == 0) {
		animation_7_2();
		pickObject(8);
	} else if (pickedObject == 14 && fl == 166 && flags[37] == 1)
		talk(323);
	else if (pickedObject == kVerbTalk && fl == 211) {
		talk(185);
		talk(186);
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_4(int fl) {
	if (pickedObject == kVerbMove && fl == 189 && flags[34] == 0) {
		talk(327);
		pickObject(13);
		flags[34] = 1;
		if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
			flags[38] = 1;
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_5(int fl) {
	if (pickedObject == kVerbLook && fl == 136 && flags[8] == 0)
		talk(14);
	else if (pickedObject == 10 && fl == 136) {
		animation_5_2();
		removeObject(kItemSpike);
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_6(int fl) {
	if (pickedObject == kVerbLook && fl==144) {
		talk(41);
		talk(42);
	} else if (pickedObject == kVerbOpen && fl == 138)
		toggleDoor(0, 1, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 138)
		toggleDoor(0, 1, kCloseDoor);
	else if (pickedObject == kVerbOpen && fl == 143 && flags[2] == 0) {
		copyBackground();
		updateRefresh_pre();
		copyRect(228, 102, curX + 5, curY - 1, 47, 73, drawSurface3, screenSurface);
		updateScreen();
		pause(10);
		playSound(3);
		flags[2] = 1;
		updateRoom();
		updateScreen();
		finishSound();
	} else if (pickedObject == kVerbClose && fl == 143 && flags[2] == 1) {
		copyBackground();
		flags[2] = 0;
		updateRefresh_pre();
		copyRect(228, 102, curX + 5, curY - 1, 47, 73, drawSurface3, screenSurface);
		updateScreen();
		pause(5);
		playSound(4);
		updateRoom();
		updateScreen();
		finishSound();
	} else if (pickedObject == kVerbOpen && fl == 139 && flags[1] == 0) {
		copyBackground();
		updateRefresh_pre();
		copyRect(267, 1, curX - 14, curY - 2, 52, 73, drawSurface3, screenSurface);
		updateScreen();
		pause(19);
		playSound(3);
		flags[1] = 1;
		visible[4] = 1;
		visible[2] = 0;
		updateRoom();
		updateScreen();
		finishSound();
	} else if (pickedObject == kVerbPick && fl == 140) {
		copyBackground();
		updateRefresh_pre();
		copyRect(267, 1, curX - 14, curY - 2, 52, 73, drawSurface3, screenSurface);
		updateScreen();
		pause(19);
		pickObject(9);
		visible[4] = 0;
		flags[10] = 1;
	} else if (pickedObject == kVerbOpen && fl == 140)
		hasAnswer = 1;
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_7(int fl) {
	if (pickedObject == kVerbPick && fl == 190) {
		pickObject(17);
		flags[35] = 1;
		visible[3] = 0;
		if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
			flags[38] = 1;
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_8(int fl) {
	if (pickedObject == kVerbLook && fl == 147) {
		if (flags[7] == 0) {
			talk(58);
			pickObject(15);
			flags[7] = 1;
			if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
				flags[38] = 1;
		} else {
			talk(59);
		}
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_9(int fl) {
	if (pickedObject == kVerbTalk && fl == 51 && flags[4] == 0)
		animation_4_2();
	else if (pickedObject == kVerbTalk && fl == 51 && flags[4] == 1)
		animation_33_2();
	else if (pickedObject == 7 && fl == 51) {
		animation_6_2();
		removeObject(kItemMoney);
		pickObject(14);}
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_12(int fl) {
	if (pickedObject == kVerbOpen && fl == 156)
		toggleDoor(16, 4, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 156)
		toggleDoor(16, 4, kCloseDoor);
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_13(int fl) {
	if (pickedObject == kVerbLook && fl == 51) {
		talk(411);
		trackProtagonist = 3;
		talk(412);
		strcpy(objName[1], "yoda");
	} else if (pickedObject == kVerbTalk && fl == 51) {
		converse(7);
	} else if (pickedObject == 19 && fl == 51) {
		talk(413);
		grr();
		pause(50);
		talk(414);
	} else if (pickedObject == 9 && fl == 51) {
		animation_2_3();
		return true;
	} else
		hasAnswer = 0;

	return false;
}

bool DrasculaEngine::room_14(int fl) {
	if (pickedObject == kVerbTalk && fl == 54 && flags[39] == 0)
		animation_12_2();
	else if (pickedObject == kVerbTalk && fl == 54 && flags[39] == 1)
		talk(109);
	else if (pickedObject == 12 && fl == 54)
		animation_26_2();
	else if (pickedObject == kVerbTalk && fl == 52 && flags[5] == 0)
		animation_11_2();
	else if (pickedObject == kVerbTalk && fl == 52 && flags[5] == 1)
		animation_36_2();
	else if (pickedObject == kVerbTalk && fl == 53)
		animation_13_2();
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_15(int fl) {
	if (pickedObject == 19 && fl == 188 && flags[27] == 0)
		talk(335);
	else if (pickedObject == 19 && fl == 188 && flags[27] == 1) {
		talk(336);
		trackProtagonist = 3;
		talk(337);
		talk_sync(_text[46], "46.als", "4442444244244");
		trackProtagonist = 1;
	} else if (pickedObject == 18 && fl == 188 && flags[26] == 0) {
		copyBackground();
		copyRect(133, 135, curX + 6, curY, 39, 63, drawSurface3, screenSurface);
		updateScreen();
		playSound(8);
		finishSound();
		talk(338);
		flags[27] = 0;
		pickObject(19);
		removeObject(kItemTwoCoins);
	} else if (pickedObject == kVerbMove && fl == 188 && flags[27] == 0) {
		animation_34_2();
		talk(339);
		pickObject(16);
		flags[26] = 1;
		flags[27] = 1;
		if (flags[7] == 1 && flags[26] == 1 && flags[34] == 1 && flags[35] == 1 && flags[37] == 1)
			flags[38] = 1;
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_16(int fl) {
	if (pickedObject == kVerbOpen && fl == 163)
		toggleDoor(17, 0, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 163)
		toggleDoor(17, 0, kCloseDoor);
	else if (pickedObject == kVerbTalk && fl == 183) {
		talk(341);
		pause(10);
		talk_sync(_text[50], "50.als", "11111111111144432554433");
		pause(3);
		talk_trunk(83);
	} else if (pickedObject == kVerbOpen && fl == 183) {
		toggleDoor(19, NO_DOOR, kOpenDoor);
		if (flags[20] == 0) {
			flags[20] = 1;
			trackProtagonist = 3;
			updateRoom();
			updateScreen();
			talk(342);
			pickObject(22);
		}
	} else if (pickedObject == kVerbClose && fl == 183)
		toggleDoor(19, NO_DOOR, kCloseDoor);
	else if (pickedObject == kVerbLook && fl == 187) {
		talk(343);
		trackProtagonist = 3;
		updateRoom();
		updateScreen();
		talk(344);
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_17(int fl) {
	if (pickedObject == kVerbLook && fl == 177)
		talk(35);
	else if (pickedObject == kVerbTalk && fl == 177 && flags[18] == 0)
		talk(6);
	else if (pickedObject == kVerbTalk && fl == 177 && flags[18] == 1) {
		talk(378);
		talk_vonBraun(4, kVonBraunDoor);
		converse(3);
	} else if (pickedObject == kVerbOpen && fl == 177 && flags[18] == 1)
		talk(346);
	else if (pickedObject == kVerbOpen && fl == 177 && flags[14] == 0 && flags[18] == 0)
		playTalkSequence(22);	// sequence 22, chapter 2
	else if (pickedObject == kVerbOpen && fl == 177 && flags[14] == 1)
		toggleDoor(15, 1, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 177 && flags[14] == 1)
		toggleDoor(15, 1, kCloseDoor);
	else if (pickedObject == 11 && fl == 50 && flags[22] == 0) {
		talk(347);
		flags[29] = 1;
		pickObject(23);
		removeObject(kItemEarplugs);
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_18(int fl) {
	if (pickedObject == kVerbTalk && fl == 55 && flags[36] == 0)
		animation_24_2();
	else if (pickedObject == kVerbTalk && fl == 55 && flags[36] == 1)
		talk(109);
	else if (pickedObject == kVerbPick && fl == 182) {
		copyBackground();
		updateRefresh_pre();
		copyRect(44, 1, curX, curY, 41, 70, drawSurface2, screenSurface);
		updateRefresh();
		updateScreen();
		pause(4);
		pickObject(12);
		visible[2] = 0;
		flags[28] = 1;
	} else if (fl == 55 && flags[38] == 0 && flags[33] == 0) {
		if (pickedObject == 8 || pickedObject == 13 || pickedObject == 15 ||
			pickedObject == 16 || pickedObject == 17)
			talk(349);
	} else if (fl == 55 && flags[38] == 1 && flags[33] == 1) {
		if (pickedObject == 8 || pickedObject == 13 || pickedObject == 15 ||
			pickedObject == 16 || pickedObject == 17)
			animation_24_2();
	}
	else if (pickedObject == 11 && fl == 50 && flags[22] == 0) {
		trackProtagonist = 3;
		updateRoom();
		updateScreen();
		copyBackground();
		updateRefresh_pre();
		copyRect(1, 1, curX - 1, curY + 3, 42, 67, drawSurface2, screenSurface);
		updateRefresh();
		updateScreen();
		pause(6);
		talk(347);
		flags[29] = 1;
		pickObject(23);
		removeObject(kItemEarplugs);
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_21(int fl) {
	if (pickedObject == kVerbOpen && fl == 101 && flags[28] == 0)
		talk(419);
	else if (pickedObject == kVerbOpen && fl == 101 && flags[28] == 1)
		toggleDoor(0, 1, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 101)
		toggleDoor(0, 1, kCloseDoor);
	else if (pickedObject == kVerbPick && fl == 141) {
		pickObject(19);
		visible[2] = 0;
		flags[10] = 1;
	} else if (pickedObject == 7 && fl == 101) {
		flags[28] = 1;
		toggleDoor(0, 1, kOpenDoor);
		selectVerb(kVerbNone);
	} else if (pickedObject == 21 && fl == 179) {
		animate("st.bin", 14);
		fadeToBlack(1);
		return true;
	} else
		hasAnswer = 0;

	return false;
}

bool DrasculaEngine::room_22(int fl) {
	if (pickedObject == 11 && fl == 140) {
		pickObject(18);
		visible[1] = 0;
		flags[24] = 1;
	} else if (pickedObject == 22 && fl == 52) {
		animate("up.bin",14);
		flags[26]=1;
		playSound(1);
		hiccup(14);
		finishSound();
		selectVerb(kVerbNone);
		removeObject(22);
		updateVisible();
		trackProtagonist = 3;
		talk(499);
		talk(500);
	} else if (pickedObject == kVerbPick && fl == 180 && flags[26] == 0)
		talk(420);
	else if (pickedObject == kVerbPick && fl == 180 && flags[26] == 1) {
		pickObject(7);
		visible[3] = 0;
		flags[27] = 1;
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_23(int fl) {
	if (pickedObject == kVerbOpen && fl == 103) {
		toggleDoor(0, 0, kOpenDoor);
		updateVisible();
	} else if (pickedObject == kVerbClose && fl == 103) {
		toggleDoor(0, 0, kCloseDoor);
		updateVisible();
	} else if (pickedObject == kVerbOpen && fl == 104)
		toggleDoor(1, 1, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 104)
		toggleDoor(1, 1, kCloseDoor);
	else if (pickedObject == kVerbPick && fl == 142) {
		pickObject(8);
		visible[2] = 0;
		flags[11] = 1;
		if (flags[22] == 1 && flags[14] == 1)
			flags[18] = 1;
		if (flags[18] == 1)
			animation_6_4();
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_24(int fl) {
	if (pickedObject == kVerbOpen && fl == 105)
		toggleDoor(1, 0, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 105)
		toggleDoor(1, 0, kCloseDoor);
	else if (pickedObject == kVerbOpen && fl == 106)
		toggleDoor(2, 1, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 106)
		toggleDoor(2, 1, kCloseDoor);
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_26(int fl) {
	if (pickedObject == kVerbOpen && fl == 107 && flags[30] == 0)
		toggleDoor(2, 0, kOpenDoor);
	else if (pickedObject == kVerbOpen && fl == 107 && flags[30] == 1)
		talk(421);
	else if (pickedObject == kVerbClose && fl == 107)
		toggleDoor(2, 0, kCloseDoor);
	else if (pickedObject == 10 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (pickedObject == 8 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (pickedObject == 12 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (pickedObject == 16 && fl == 50 && flags[18] == 1 && flags[12] == 1)
		animation_5_4();
	else if (pickedObject == kVerbPick && fl == 143 && flags[18] == 1) {
		gotoObject(260, 180);
		pickObject(10);
		visible[1] = 0;
		flags[12] = 1;
		toggleDoor(2, 0, kCloseDoor);
		trackProtagonist = 2;
		talk_igor(27, kIgorDoor);
		flags[30] = 1;
		talk_igor(28, kIgorDoor);
		gotoObject(153, 180);
	} else if (pickedObject == kVerbPick && fl == 143 && flags[18] == 0) {
		gotoObject(260, 180);
		copyBackground(80, 78, 199, 94, 38, 27, drawSurface3, screenSurface);
		updateScreen(199, 94, 199, 94, 38, 27, screenSurface);
		pause(3);
		talk_igor(25, kIgorWig);
		gotoObject(153, 180);
	} else if (pickedObject == kVerbTalk && fl == 51)
		animation_1_4();
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_27(int fl) {
	if (pickedObject == kVerbOpen && fl == 110)
		toggleDoor(6, 1, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 110)
		toggleDoor(6, 1, kCloseDoor);
	else if (pickedObject == kVerbOpen && fl == 116 && flags[23] == 0)
		talk(419);
	else if (pickedObject == kVerbOpen && fl == 116 && flags[23] == 1)
		toggleDoor(5, 3, kOpenDoor);
	else if (pickedObject == 17 && fl == 116) {
		flags[23] = 1;
		toggleDoor(5, 3, kOpenDoor);
		selectVerb(kVerbNone);
	} else if (fl == 150)
		talk(460);
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_29(int fl) {
	if (pickedObject == kVerbOpen && fl == 114)
		toggleDoor(4, 1, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 114)
		toggleDoor(4, 1, kCloseDoor);
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_30(int fl) {
	if (pickedObject == kVerbOpen && fl == 115)
		toggleDoor(4, 0, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 115)
		toggleDoor(4, 0, kCloseDoor);
	else if (pickedObject == kVerbOpen && fl == 144 && flags[19] == 0)
		talk(422);
	else if (pickedObject == kVerbOpen && fl == 144 && flags[19] == 1 && flags[22] == 1)
		toggleDoor(16, 1, kOpenDoor);
	else if (pickedObject == kVerbOpen && fl == 144 && flags[19] == 1 && flags[22] == 0) {
		toggleDoor(16, 1, kOpenDoor);
		talk(423);
		flags[22] = 1;
		pickObject(12);
		if (flags[11] == 1 && flags[14] == 1)
			flags[18] = 1;
		if (flags[18] == 1)
			animation_6_4();
	} else if (pickedObject == kVerbClose && fl == 144)
		toggleDoor(16, 1, kCloseDoor);
	else if (pickedObject == 13 && fl == 144) {
		talk(424);
		flags[19] = 1;
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_31(int fl) {
	if (pickedObject == kVerbPick && fl == 145) {
		pickObject(11);
		visible[1] = 0;
		flags[13] = 1;
	} else if (pickedObject == kVerbOpen && fl == 117)
		toggleDoor(5, 0, kOpenDoor);
	else if (pickedObject == kVerbClose && fl == 117)
		toggleDoor(5, 0, kCloseDoor);
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_34(int fl) {
	if (pickedObject == kVerbMove && fl == 146)
		animation_8_4();
	else if (pickedObject == kVerbOpen && fl == 120 && flags[25] == 1)
		toggleDoor(8, 2, kOpenDoor);
	else if (pickedObject == kVerbOpen && fl == 120 && flags[25] == 0) {
		toggleDoor(8, 2, kOpenDoor);
		trackProtagonist = 3;
		talk(425);
		pickObject(14);
		flags[25] = 1;
	} else if (pickedObject == kVerbClose && fl == 120)
		toggleDoor(8, 2, kCloseDoor);
	else
		hasAnswer=0;

	return true;
}

bool DrasculaEngine::room_35(int fl) {
	if (pickedObject == kVerbPick && fl == 148) {
		pickObject(16);
		visible[2] = 0;
		flags[14] = 1;
	if (flags[11] == 1 && flags[22] == 1)
		flags[18] = 1;
	if (flags[18] == 1)
		animation_6_4();
	} else if (pickedObject == kVerbPick && fl == 147) {
		talk(426);
		pickObject(15);
		visible[1] = 0;
		flags[15] = 1;
		flags[17] = 1;
		updateVisible();
	} else if (pickedObject == kVerbPick && fl == 149) {
		pickObject(13);
		visible[3] = 0;
		flags[17] = 0;
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_49(int fl) {
	if (pickedObject == kVerbTalk && fl ==51)
		converse(9);
	else if ((pickedObject == 8 && fl == 51) || (pickedObject == 8 && fl == 203))
		animation_5_5();
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_53(int fl) {
	if (pickedObject == kVerbPick && fl == 120) {
		pickObject(16);
		visible[3] = 0;
	} else if (pickedObject == kVerbMove && fl == 123) {
		animation_11_5();
	} else if (pickedObject == 12 && fl == 52) {
		flags[3] = 1;
		talk(401);
		selectVerb(kVerbNone);
		removeObject(12);
	} else if (pickedObject == 15 && fl == 52) {
		flags[4] = 1;
		talk(401);
		selectVerb(kVerbNone);
		removeObject(15);
	} else if (pickedObject == 16 && fl == 121) {
		flags[2] = 1;
		selectVerb(kVerbNone);
		updateVisible();
		pickedObject = kVerbMove;
	} else if (pickedObject == 16) {
		// Wall plug in chapter 5
		visible[3] = 1;
		hasAnswer = 0;
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_54(int fl) {
	if ((pickedObject == kVerbTalk && fl == 118) || (pickedObject == kVerbLook && fl == 118 && flags[0] == 0))
		animation_1_5();
	else if (pickedObject == kVerbLook && fl == 118 && flags[0] == 1)
		talk(124);
	else if (pickedObject == kVerbTalk && fl == 53 && flags[14] == 0) {
		talk(288);
		flags[12] = 1;
		pause(10);
		talk_mus(1);
		talk(289);
		talk_mus(2);
		talk_mus(3);
		converse(10);
		flags[12] = 0;
		flags[14] = 1;
	} else if (pickedObject == kVerbTalk && fl == 53 && flags[14] == 1)
		talk(109);
	else if (pickedObject == kVerbPick && fl == 9999 && flags[13] == 0) {
		pickObject(8);
		flags[13] = 1;
		talk_mus(10);
		updateVisible();
	} else if (pickedObject == 10 && fl == 119) {
		pause(4);
		talk(436);
		selectVerb(kVerbNone);
		removeObject(10);
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_55(int fl) {
	if (pickedObject == kVerbPick && fl == 122) {
		pickObject(12);
		flags[8] = 1;
		updateVisible();
	} else if (fl == 206) {
		playSound(11);
		animate("det.bin", 17);
		finishSound();
		gotoObject(curX - 3, curY + curHeight + 6);
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_56(int fl) {
	if (pickedObject == kVerbOpen && fl == 124) {
		animation_14_5();
		return true;
	} else
		hasAnswer = 0;

	return false;
}

bool DrasculaEngine::room_58(int fl) {
	if (pickedObject == kVerbMove && fl == 103) {
		flags[8] = 1;
		updateVisible();
	} else {
		hasAnswer = 0;
	}

	return true;
}

bool DrasculaEngine::room_59(int fl) {
	if ((pickedObject == kVerbTalk && fl == 51) || (pickedObject == kVerbLook && fl == 51)) {
		flags[9] = 1;
		talk(259);
		talk_bj_bed(13);
		talk(263);
		talk_bj_bed(14);
		pause(40);
		talk(264);
		talk_bj_bed(15);
		talk(265);
		flags[9] = 0;
		if (flags[11] == 0) {
			playSound(12);
			delay(40);
			finishSound();
			delay(10);
			gotoObject(174, 168);
			trackProtagonist = 2;
			updateRoom();
			updateScreen();
			pause(40);
			playSound(12);
			pause(19);
			stopSound();
			hare_se_ve = 0;
			updateRoom();
			copyRect(101, 34, curX - 4, curY - 1, 37, 70, drawSurface3, screenSurface);
			copyBackground(0, 0, 0, 0, 320, 200, screenSurface, bgSurface);
			updateScreen();
			hare_se_ve = 1;
			clearRoom();
			loadPic("tlef0.alg", bgSurface, COMPLETE_PAL);
			loadPic("tlef1.alg", drawSurface3);
			loadPic("tlef2.alg", frontSurface);
			loadPic("tlef3.alg", backSurface);
			talk_htel(240);

			color_abc(kColorBrown);
			talk_solo(_textvb[58], "VB58.als");
			talk_htel(241);
			color_abc(kColorBrown);
			talk_solo(_textvb[59], "VB59.als");
			talk_htel(242);
			color_abc(kColorBrown);
			talk_solo(_textvb[60], "VB60.als");
			talk_htel(196);
			color_abc(kColorBrown);
			talk_solo(_textvb[61],"VB61.als");
			talk_htel(244);
			color_abc(kColorBrown);
			talk_solo(_textvb[62], "VB62.als");
			clearRoom();
			loadPic("aux59.alg", drawSurface3);
			loadPic(96, frontSurface, COMPLETE_PAL);
			loadPic(99, backSurface);
			loadPic(59, bgSurface, HALF_PAL);
			trackProtagonist = 3;
			talk(245);
			selectVerb(kVerbNone);
			flags[11] = 1;
		}
	} else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_60(int fl) {
	if (pickedObject == kVerbMove && fl == 112) {
		playSound(14);
		copyBackground();
		updateRefresh_pre();
		copyBackground(164, 85, 155, 48, 113, 114, drawSurface3, screenSurface);
		updateScreen();
		finishSound();
		talk_bartender(23, 1);
		flags[7] = 1;
	} else if (pickedObject == kVerbTalk && fl == 52) {
		talk(266);
		talk_bartender(1, 1);
		converse(12);
		selectVerb(kVerbNone);
		pickedObject = 0;
	} else if (pickedObject == 21 && fl == 56) {
		flags[6] = 1;
		selectVerb(kVerbNone);
		removeObject(21);
		animate("beb.bin", 10);
	} else if (pickedObject == 9 && fl == 56 && flags[6] == 1) {
		animation_9_6();
		return true;
	} else if (pickedObject == 9 && fl == 56 && flags[6] == 0) {
		animate("cnf.bin", 14);
		talk(455);
	} else
		hasAnswer = 0;

	return false;
}

bool DrasculaEngine::room_62(int fl) {
	if (pickedObject == kVerbTalk && fl == 53)
		converse(13);
	else if (pickedObject == kVerbTalk && fl == 52 && flags[0] == 0)
		animation_3_1();
	else if (pickedObject == kVerbTalk && fl == 52 && flags[0] == 1)
		talk(109);
	else if (pickedObject == kVerbTalk && fl == 54)
		animation_4_1();
	else
		hasAnswer = 0;

	return true;
}

bool DrasculaEngine::room_102(int fl) {
	if (pickedObject == kVerbPick && fl == 101)
		pickObject(20);
	else if (pickedObject == 20 && fl == 100)
		animation_6_6();
	else if (pickedObject == kVerbPick || pickedObject == kVerbOpen)
		talk(453);
	else
		hasAnswer = 0;

	return true;
}

void DrasculaEngine::updateRefresh() {
	// Check generic updaters
	for (int i = 0; i < _roomUpdatesSize; i++) {
		if (_roomUpdates[i].roomNum == roomNumber) {
			if (_roomUpdates[i].flag < 0 ||
				flags[_roomUpdates[i].flag] == _roomUpdates[i].flagValue) {
				if (_roomUpdates[i].type == 0) {
					copyBackground(_roomUpdates[i].sourceX, _roomUpdates[i].sourceY,
								   _roomUpdates[i].destX, _roomUpdates[i].destY,
								   _roomUpdates[i].width, _roomUpdates[i].height,
								   drawSurface3, screenSurface);
				} else {
					copyRect(_roomUpdates[i].sourceX, _roomUpdates[i].sourceY,
							 _roomUpdates[i].destX, _roomUpdates[i].destY,
							 _roomUpdates[i].width, _roomUpdates[i].height,
							 drawSurface3, screenSurface);
				}
			}
		}
	}

	// Call room-specific updater
	char rm[20];
	sprintf(rm, "update_%d", roomNumber);
	for (uint i = 0; i < _roomHandlers->roomUpdaters.size(); i++) {
		if (!strcmp(rm, _roomHandlers->roomUpdaters[i]->desc)) {
			debug(8, "Calling room updater %d", roomNumber);
			(this->*(_roomHandlers->roomUpdaters[i]->proc))();
			break;
		}
	}

	if (roomNumber == 10)
		showMap();
	else if (roomNumber == 45)
		showMap();
}

void DrasculaEngine::updateRefresh_pre() {
	// Check generic preupdaters
	for (int i = 0; i < _roomPreUpdatesSize; i++) {
		if (_roomPreUpdates[i].roomNum == roomNumber) {
			if (_roomPreUpdates[i].flag < 0 ||
				flags[_roomPreUpdates[i].flag] == _roomPreUpdates[i].flagValue) {
				if (_roomPreUpdates[i].type == 0) {
					copyBackground(_roomPreUpdates[i].sourceX, _roomPreUpdates[i].sourceY,
								   _roomPreUpdates[i].destX, _roomPreUpdates[i].destY,
								   _roomPreUpdates[i].width, _roomPreUpdates[i].height,
								   drawSurface3, screenSurface);
				} else {
					copyRect(_roomPreUpdates[i].sourceX, _roomPreUpdates[i].sourceY,
							 _roomPreUpdates[i].destX, _roomPreUpdates[i].destY,
							 _roomPreUpdates[i].width, _roomPreUpdates[i].height,
							 drawSurface3, screenSurface);
				}
			}
		}
	}

	// Call room-specific preupdater
	char rm[20];
	sprintf(rm, "update_%d_pre", roomNumber);
	for (uint i = 0; i < _roomHandlers->roomPreupdaters.size(); i++) {
		if (!strcmp(rm, _roomHandlers->roomPreupdaters[i]->desc)) {
			debug(8, "Calling room preupdater %d", roomNumber);
			(this->*(_roomHandlers->roomPreupdaters[i]->proc))();
			break;
		}
	}

	if (currentChapter == 1 && roomNumber == 16)
		placeBJ();
}

void DrasculaEngine::update_1_pre() {
	if (curX > 98 && curX < 153) {
		setDefaultPalette(darkPalette);
	} else {
		setDefaultPalette(brightPalette);
	}

	if (flags[8] == 0)
		copyBackground(2, 158, 208, 67, 27, 40, drawSurface3, screenSurface);
}

void DrasculaEngine::update_2() {
	int difference;
	int w, h;
	int batX[] = {0, 38, 76, 114, 152, 190, 228, 266,
					0, 38, 76, 114, 152, 190, 228, 266,
					0, 38, 76, 114, 152, 190,
					0, 48, 96, 144, 192, 240,
					30, 88, 146, 204, 262,
					88, 146, 204, 262,
					88, 146, 204, 262};

	int batY[] = {179, 179, 179, 179, 179, 179, 179, 179,
					158, 158, 158, 158, 158, 158, 158, 158,
					137, 137, 137, 137, 137, 137,
					115, 115, 115, 115, 115, 115,
					78, 78, 78, 78, 78,
					41, 41, 41, 41,
					4, 4, 4, 4};

	if (actorFrames[kFrameBat] == 41)
		actorFrames[kFrameBat] = 0;

	if (actorFrames[kFrameBat] < 22) {
		w = 37;
		h = 21;
	} else if (actorFrames[kFrameBat] > 27) {
		w = 57;
		h = 36;
	} else {
		w = 47;
		h = 22;
	}

	copyRect(batX[actorFrames[kFrameBat]], batY[actorFrames[kFrameBat]],
			 239, 19, w, h, drawSurface3, screenSurface);
	difference = getTime() - savedTime;
	if (difference >= 6) {
		actorFrames[kFrameBat]++;
		savedTime = getTime();
	}

	copyRect(29, 37, 58, 114, 57, 39, drawSurface3, screenSurface);
	showMap();
}

void DrasculaEngine::update_3() {
	if (curY + curHeight < 118)
		copyRect(129, 110, 194, 36, 126, 88, drawSurface3, screenSurface);
	copyRect(47, 57, 277, 143, 43, 50, drawSurface3, screenSurface);
}

void DrasculaEngine::update_4() {
	if (curX > 190) {
		setDefaultPalette(darkPalette);
	} else {
		setDefaultPalette(brightPalette);
	}
}

void DrasculaEngine::update_6_pre() {
	if ((curX > 149 && curY + curHeight > 160 && curX < 220 && curY + curHeight < 188) ||
		(curX > 75 && curY + curHeight > 183 && curX < 145)) {
		setDefaultPalette(brightPalette);
	} else {
		setDefaultPalette(darkPalette);
	}
}

void DrasculaEngine::update_9_pre() {
	int blindX[] = {26, 68, 110, 152, 194, 236, 278, 26, 68};
	int blindY[] = {51, 51, 51, 51, 51, 51, 51, 127, 127};
	int difference;

	copyRect(blindX[actorFrames[kFrameBlind]], blindY[actorFrames[kFrameBlind]],
			 122, 57, 41, 72, drawSurface3, screenSurface);
	if (flags[9] == 0) {
		difference = getTime() - savedTime;
		if (difference >= 11) {
			actorFrames[kFrameBlind]++;
			savedTime = getTime();
		}
		if (actorFrames[kFrameBlind] == 9)
			actorFrames[kFrameBlind] = 0;
	} else
		actorFrames[kFrameBlind] = 3;
}

void DrasculaEngine::update_13() {
	if (curX > 55 && flags[3] == 0)
		animation_6_3();
	// These cannot be placed in an array, as they're between screen updates
	if (flags[1] == 0)
		copyRect(185, 110, 121, 65, 67, 88, drawSurface3, screenSurface);
	if (flags[2] == 0)
		copyRect(185, 21, 121, 63, 67, 88, drawSurface3, screenSurface);
	copyRect(3, 127, 99, 102, 181, 71, drawSurface3, screenSurface);
}

void DrasculaEngine::update_14_pre() {
	int difference;

	copyBackground(123, _candleY[actorFrames[kFrameCandles]], 142, 14, 39, 13, drawSurface3, screenSurface);
	copyBackground(_candleX[actorFrames[kFrameCandles]], 146, 311, 80, 4, 8, drawSurface3, screenSurface);

	if (blinking == 5)
		copyBackground(1, 149, 127, 52, 9, 5, drawSurface3, screenSurface);
	if (curX > 101 && curX < 155)
		copyBackground(31, 138, 178, 51, 18, 16, drawSurface3, screenSurface);
	if (flags[11] == 0)
		copyBackground(_pianistX[actorFrames[kFramePianist]], 157, 245, 130, 29, 42, drawSurface3, screenSurface);
	else if (flags[5] == 0)
		copyBackground(145, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	else
		copyBackground(165, 140, 229, 117, 43, 59, drawSurface3, screenSurface);

	if (flags[12] == 1)
		copyBackground(_drunkX[actorFrames[kFrameDrunk]], 82, 170, 50, 40, 53, drawSurface3, screenSurface);
	difference = getTime() - savedTime;
	if (difference > 6) {
		if (flags[12] == 1) {
			actorFrames[kFrameDrunk]++;
			if (actorFrames[kFrameDrunk] == 8) {
				actorFrames[kFrameDrunk] = 0;
				flags[12] = 0;
			}
		} else if ((_rnd->getRandomNumber(94) == 15) && (flags[13] == 0))
			flags[12] = 1;

		actorFrames[kFrameCandles]++;
		if (actorFrames[kFrameCandles] == 3)
			actorFrames[kFrameCandles] = 0;
		actorFrames[kFramePianist]++;
		if (actorFrames[kFramePianist] == 9)
			actorFrames[kFramePianist] = 0;
		blinking = _rnd->getRandomNumber(10);
		savedTime = getTime();
	}
}

void DrasculaEngine::update_16_pre() {
	if (currentChapter != 2) {
		debug(4, "update_16_pre: Special case, current chapter is not 2, not performing update");
		return;
	}

	if (flags[17] == 0)
		copyBackground(1, 103, 24, 72, 33, 95, drawSurface3, screenSurface);
	if (flags[19] == 1)
		copyBackground(37, 151, 224, 115, 56, 47, drawSurface3, screenSurface);
}

void DrasculaEngine::update_18_pre() {
	int difference;
	int snore_x[] = {95, 136, 95, 136, 95, 95, 95, 95, 136, 95, 95, 95, 95, 95, 95, 95};
	int snore_y[] = {18, 18, 56, 56, 94, 94, 94, 94, 94, 18, 18, 18, 18, 18, 18, 18};

	if (flags[21] == 0) {
		copyBackground(1, 69, 120, 58, 56, 61, drawSurface3, screenSurface);
		copyBackground(snore_x[actorFrames[kFrameSnore]],
					  snore_y[actorFrames[kFrameSnore]],
					  124, 59, 40, 37, drawSurface3, screenSurface);
	} else
		moveVonBraun();

	difference = getTime() - savedTime;
	if (difference > 9) {
		actorFrames[kFrameSnore]++;
		if (actorFrames[kFrameSnore] == 16)
			actorFrames[kFrameSnore] = 0;
		savedTime = getTime();
	}
}

void DrasculaEngine::update_23_pre() {
	if (flags[11] == 1 && flags[0] == 0)
		copyBackground(87, 171, 237, 110, 20, 28, drawSurface3, screenSurface);

	// It might be possible to put these in an array, though I'm a bit unsure
	// of the draw order
	if (flags[0] == 1)
		copyBackground(29, 126, 239, 94, 57, 73, drawSurface3, screenSurface);

	if (flags[1] == 1)
		copyRect(1, 135, 7, 94, 27, 64, drawSurface3, screenSurface);
}

void DrasculaEngine::update_26_pre() {
	int difference;

	if (blinking == 5 && flags[18] == 0)
		copyBackground(52, 172, 226, 106, 3, 4, drawSurface3, screenSurface);

	difference = getTime() - savedTime;
	if (difference >= 10) {
		blinking = _rnd->getRandomNumber(10);
		savedTime = getTime();
	}

	if (flags[20] == 1)
		copyBackground(182, 133, 199, 95, 50, 66, drawSurface3, screenSurface);
}

void DrasculaEngine::update_26() {
	if (flags[29] == 1)
		copyRect(93, 1, curX, curY, 45, 78, backSurface, screenSurface);

	copyRect(233, 107, 17, 102, 66, 92, drawSurface3, screenSurface);
}

void DrasculaEngine::update_35_pre() {
	if (flags[17] == 0 && flags[15] == 1)
		copyBackground(111, 150, 118, 52, 40, 23, drawSurface3, screenSurface);
}

void DrasculaEngine::update_58_pre() {
	if (flags[1] == 0 && flags[0] == 0)
		copyRect(278, 171, 173, 116, 25, 28, drawSurface3, screenSurface);
	if (flags[2] == 0) {
		placeIgor();
		placeDrascula();
	}
	// Not possible to put these in an array, as there are other surfaces
	// copied above
	if (flags[3] == 1)
		copyRect(1, 29, 204, 0, 18, 125, drawSurface3, screenSurface);
	if (flags[8] == 1)
		copyBackground(20, 60, 30, 64, 46, 95, drawSurface3, screenSurface);
}

void DrasculaEngine::update_58() {
	if (hare_se_ve == 1)
		copyRect(67, 139, 140, 147, 12, 16, drawSurface3, screenSurface);
}

void DrasculaEngine::update_59_pre() {
	if (flags[9] == 1) {
		copyBackground(65, 103, 65, 103, 49, 38, bgSurface, screenSurface);
		copyRect(1, 105, 65, 103, 49, 38, drawSurface3, screenSurface);
	}
}

void DrasculaEngine::update_60_pre() {
	int difference;

	if (flags[5] == 0)
		placeDrascula();

	copyBackground(123, _candleY[actorFrames[kFrameCandles]], 142, 14, 39, 13, drawSurface3, screenSurface);

	if (flag_tv == 1)
		copyBackground(114, 158, 8, 30, 8, 23, drawSurface3, screenSurface);

	difference = getTime() - savedTime;
	blinking = _rnd->getRandomNumber(7);
	if (blinking == 5 && flag_tv == 0)
		flag_tv = 1;
	else if (blinking == 5 && flag_tv == 1)
		flag_tv = 0;
	if (difference > 6) {
		actorFrames[kFrameCandles]++;
		if (actorFrames[kFrameCandles] == 3)
			actorFrames[kFrameCandles] = 0;
		savedTime = getTime();
	}
}

void DrasculaEngine::update_60() {
	if (curY - 10 < drasculaY && flags[5] == 0)
		placeDrascula();
}

void DrasculaEngine::update_62_pre() {
	int difference;

	copyBackground(123, _candleY[actorFrames[kFrameCandles]], 142, 14, 39, 13, drawSurface3, screenSurface);
	copyBackground(_candleX[actorFrames[kFrameCandles]], 146, 311, 80, 4, 8, drawSurface3, screenSurface);

	if (blinking == 5)
		copyBackground(1, 149, 127, 52, 9, 5, drawSurface3, screenSurface);

	if (curX > 101 && curX < 155)
		copyBackground(31, 138, 178, 51, 18, 16, drawSurface3, screenSurface);

	if (flags[11] == 0)
		copyBackground(_pianistX[actorFrames[kFramePianist]], 157, 245, 130, 29, 42, drawSurface3, screenSurface);
	else if (flags[5] == 0)
		copyBackground(145, 139, 228, 112, 47, 60, extraSurface, screenSurface);
	else
		copyBackground(165, 140, 229, 117, 43, 59, drawSurface3, screenSurface);

	if (flags[12] == 1)
		copyBackground(_drunkX[actorFrames[kFrameDrunk]], 82, 170, 50, 40, 53, drawSurface3, screenSurface);

	difference = getTime() - savedTime;
	if (difference > 6) {
		if (flags[12] == 1) {
			actorFrames[kFrameDrunk]++;
			if (actorFrames[kFrameDrunk] == 8) {
				actorFrames[kFrameDrunk] = 0;
				flags[12] = 0;
			}
		} else if ((_rnd->getRandomNumber(94) == 15) && (flags[13] == 0))
			flags[12] = 1;

		actorFrames[kFrameCandles]++;
		if (actorFrames[kFrameCandles] == 3)
			actorFrames[kFrameCandles] = 0;
		actorFrames[kFramePianist]++;
		if (actorFrames[kFramePianist] == 9)
			actorFrames[kFramePianist] = 0;
		blinking = _rnd->getRandomNumber(10);
		savedTime = getTime();
	}
}

void DrasculaEngine::update_62() {
	copyRect(1, 1, 0, 0, 62, 142, drawSurface2, screenSurface);

	if (curY + curHeight < 89) {
		copyRect(205, 1, 180, 9, 82, 80, drawSurface3, screenSurface);
		copyBackground(_drunkX[actorFrames[kFrameDrunk]], 82, 170, 50, 40, 53, drawSurface3, screenSurface);
	}
}

void DrasculaEngine::update_102() {
	int pendulum_x[] = {40, 96, 152, 208, 264, 40, 96, 152, 208, 208, 152, 264, 40, 96, 152, 208, 264};
	int difference;
	byte *pendulumSurface = drawSurface3;

	if (actorFrames[kFramePendulum] <= 4)
		pendulumSurface = drawSurface3;
	else if (actorFrames[kFramePendulum] <= 11)
		pendulumSurface = extraSurface;
	else
		pendulumSurface = frontSurface;

	copyBackground(pendulum_x[actorFrames[kFramePendulum]], 19, 152, 0, 55, 125, pendulumSurface, screenSurface);

	if (flags[1] == 2)
		copyRect(18, 145, 145, 105, 25, 29, drawSurface3, screenSurface);

	if (flags[1] == 0)
		copyRect(44, 145, 145, 105, 25, 29, drawSurface3, screenSurface);

	difference = getTime() - savedTime;
	if (difference > 8) {
		actorFrames[kFramePendulum]++;
		if (actorFrames[kFramePendulum] == 17)
			actorFrames[kFramePendulum] = 0;
		savedTime = getTime();
	}
}

bool DrasculaEngine::checkAction(int fl) {
	hideCursor();
	characterMoved = 0;
	updateRoom();
	updateScreen();

	hasAnswer = 1;

	if (_menuScreen && roomParse(200, fl)) {
		;
	} else if (!_menuScreen && roomParse(201, fl)) {
		;
	} else if (_menuScreen) {
		if (currentChapter == 1) {
			hasAnswer = 0;
		} else if (currentChapter == 2) {
			if ((pickedObject == kVerbLook && fl == 22 && flags[23] == 0)
				|| (pickedObject == kVerbOpen && fl == 22 && flags[23] == 0)) {
				talk(164);
				flags[23] = 1;
				selectVerb(kVerbNone);
				addObject(kItemMoney);
				addObject(kItemTwoCoins);
			} else if (pickedObject == kVerbLook && fl == 22 && flags[23] == 1)
				talk(307);
			else
				hasAnswer = 0;
		} else if (currentChapter == 3) {
			hasAnswer = 0;
		} else if (currentChapter == 4) {
			if ((pickedObject == 18 && fl == 19) || (pickedObject == 19 && fl == 18)) {
				selectVerb(kVerbNone);
				chooseObject(21);
				removeObject(18);
				removeObject(19);
			} else if (pickedObject == kVerbLook && fl == 9) {
				talk(482);
				talk(483);
			} else if (pickedObject == kVerbLook && fl == 19) {
				talk(494);
				talk(495);
			} else
				hasAnswer = 0;
		} else if (currentChapter == 5 || currentChapter == 6) {
			if (pickedObject == kVerbLook && fl == 9) {
				talk(482);
				talk(483);
			} else
				hasAnswer = 0;
		}
	} else {
		if (currentChapter == 1) {
			hasAnswer = 0;
		} else if (currentChapter == 2) {
			// Note: the original check was strcmp(num_room, "18.alg")
			if (pickedObject == 11 && fl == 50 && flags[22] == 0 && roomNumber != 18)
				talk(315);
			else
				hasAnswer = 0;
		} else if (currentChapter == 3) {
			if (roomNumber == 13) {
				if (room(13, fl)) {
					showCursor();
					return true;
				}
			} else
				hasAnswer = 0;
		} else if (currentChapter == 4) {
			if (roomNumber == 28)
				talk(178);
			else if (pickedObject == 8 && fl == 50 && flags[18] == 0)
				talk(481);
			else if (pickedObject == 12 && fl == 50 && flags[18] == 0)
				talk(487);
			else if (roomNumber == 21) {
				if (room(21, fl)) {
					showCursor();
					return true;
				}
			} else
				hasAnswer = 0;
		} else if (currentChapter == 5) {
			if (roomNumber == 56) {
				if (room(56, fl)) {
					showCursor();
					return true;
				}
			} else
				hasAnswer = 0;
		} else if (currentChapter == 6) {
			if (pickedObject == kVerbLook && fl == 50 && flags[0] == 1)
				talk(308);
			else if (pickedObject == kVerbLook && fl == 50 && flags[0] == 0)
				talk(310);
			else if (roomNumber == 102)
				room(102, fl);
			else if (roomNumber == 60) {
				if (room(60, fl)) {
					showCursor();
					return true;
				}
			}
			else
				hasAnswer = 0;
		}
	}

	if (hasAnswer == 0) {
		hasAnswer = 1;

		room(roomNumber, fl);
	}

	if (hasAnswer == 0 && (_hasName || _menuScreen))
		room(0, -1);

	showCursor();
	return false;
}

bool DrasculaEngine::room(int rN, int fl) {
	if (!roomParse(rN, fl)) {
		// Call room-specific parser
		char rm[20];
		sprintf(rm, "room_%d", rN);
		for (uint i = 0; i < _roomHandlers->roomParsers.size(); i++) {
			if (!strcmp(rm, _roomHandlers->roomParsers[i]->desc)) {
				debug(4, "Calling room parser %d", rN);

				return (this->*(_roomHandlers->roomParsers[i]->proc))(fl);
			}
		}

		// We did not find any parser, let default one work
		hasAnswer = 0;
	}

	return false;
}

void DrasculaEngine::enterRoom(int roomIndex) {
	debug(2, "Entering room %d", roomIndex);
	showCursor();

	char fileName[20];
	sprintf(fileName, "%d.ald", roomIndex);
	int soc, l, martin = 0, objIsExit = 0;
	float chiquez = 0, pequegnez = 0;
	char pant1[20], pant2[20], pant3[20], pant4[20];
	int palLevel = 0;

	_hasName = false;

	strcpy(currentData, fileName);

	Common::SeekableReadStream *stream = _archives.open(fileName);
	if (!stream) {
		error("missing data file %s", fileName);
	}

	TextResourceParser p(stream, DisposeAfterUse::YES);

	p.parseInt(roomNumber);
	p.parseInt(roomMusic);
	p.parseString(roomDisk);
	p.parseInt(palLevel);

	if (currentChapter == 2)
		p.parseInt(martin);

	if (currentChapter == 2 && martin != 0) {
		curWidth = martin;
		p.parseInt(curHeight);
		p.parseInt(feetHeight);
		p.parseInt(stepX);
		p.parseInt(stepY);

		p.parseString(pant1);
		p.parseString(pant2);
		p.parseString(pant3);
		p.parseString(pant4);

		strcpy(menuBackground, pant4);
	}

	p.parseInt(numRoomObjs);

	for (l = 0; l < numRoomObjs; l++) {
		p.parseInt(objectNum[l]);
		p.parseString(objName[l]);
		p.parseInt(x1[l]);
		p.parseInt(y1[l]);
		p.parseInt(x2[l]);
		p.parseInt(y2[l]);
		p.parseInt(roomObjX[l]);
		p.parseInt(roomObjY[l]);
		p.parseInt(trackObj[l]);
		p.parseInt(visible[l]);
		p.parseInt(isDoor[l]);
		if (isDoor[l] != 0) {
			p.parseString(_targetSurface[l]);
			p.parseInt(_destX[l]);
			p.parseInt(_destY[l]);
			p.parseInt(trackCharacter_alkeva[l]);
			p.parseInt(roomExits[l]);
			updateDoor(l);
		}
	}

	p.parseInt(floorX1);
	p.parseInt(floorY1);
	p.parseInt(floorX2);
	p.parseInt(floorY2);

	if (currentChapter != 2) {
		p.parseInt(upperLimit);
		p.parseInt(lowerLimit);
	}
	// no need to delete the stream, since TextResourceParser takes ownership
	// delete stream;

	if (currentChapter == 2 && martin != 0) {
		loadPic(pant2, extraSurface);
		loadPic(pant1, frontSurface);
		loadPic(pant4, backSurface);
	}

	if (currentChapter == 2) {
		if (martin == 0) {
			stepX = STEP_X;
			stepY = STEP_Y;
			curHeight = CHARACTER_HEIGHT;
			curWidth = CHARACTER_WIDTH;
			feetHeight = FEET_HEIGHT;
			loadPic(97, extraSurface);
			loadPic(96, frontSurface);
			loadPic(99, backSurface);

			strcpy(menuBackground, "99.alg");
		}
	}

	for (l = 0; l < numRoomObjs; l++) {
		if (objectNum[l] == objExit)
			objIsExit = l;
	}

	if (currentChapter == 2) {
		if (curX == -1) {
			curX = _destX[objIsExit];
			curY = _destY[objIsExit] - curHeight;
		}
		characterMoved = 0;
	}

	loadPic(roomDisk, drawSurface3);
	loadPic(roomNumber, bgSurface, HALF_PAL);

	copyBackground(0, 171, 0, 0, OBJWIDTH, OBJHEIGHT, backSurface, drawSurface3);

	setDefaultPalette(defaultPalette);
	if (palLevel != 0)
		setPaletteBase(palLevel);
	assignPalette(brightPalette);
	setDefaultPalette(defaultPalette);
	setPaletteBase(palLevel + 2);
	assignPalette(darkPalette);

	setDefaultPalette(brightPalette);

	if (currentChapter == 2)
		color_abc(kColorLightGreen);

	if (currentChapter != 2) {
		for (l = 0; l <= floorY1; l++)
			factor_red[l] = upperLimit;
		for (l = floorY1; l <= 201; l++)
			factor_red[l] = lowerLimit;

		chiquez = (float)(lowerLimit - upperLimit) / (float)(floorY2 - floorY1);
		for (l = floorY1; l <= floorY2; l++) {
			factor_red[l] = (int)(upperLimit + pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (roomNumber == 24) {
		for (l = floorY1 - 1; l > 74; l--) {
			factor_red[l] = (int)(upperLimit - pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (currentChapter == 5 && roomNumber == 54) {
		for (l = floorY1 - 1; l > 84; l--) {
			factor_red[l] = (int)(upperLimit - pequegnez);
			pequegnez = pequegnez + chiquez;
		}
	}

	if (currentChapter != 2) {
		if (curX == -1) {
			curX = _destX[objIsExit];
			curY = _destY[objIsExit];
			curHeight = (CHARACTER_HEIGHT * factor_red[curY]) / 100;
			curWidth = (CHARACTER_WIDTH * factor_red[curY]) / 100;
			curY = curY - curHeight;
		} else {
			curHeight = (CHARACTER_HEIGHT * factor_red[curY]) / 100;
			curWidth = (CHARACTER_WIDTH * factor_red[curY]) / 100;
		}
		characterMoved = 0;
	}

	if (currentChapter == 2) {
		soc = 0;
		for (l = 0; l < 6; l++) {
			soc += curWidth;
			_frameX[l] = soc;
		}
	}

	if (currentChapter == 5)
		hare_se_ve = 1;

	updateVisible();

	if (currentChapter == 1)
		isDoor[7] = 0;

	if (currentChapter == 2) {
		if (roomNumber == 14 && flags[39] == 1)
			roomMusic = 16;
		else if (roomNumber == 15 && flags[39] == 1)
			roomMusic = 16;
		if (roomNumber == 14 && flags[5] == 1)
			roomMusic = 0;
		else if (roomNumber == 15 && flags[5] == 1)
			roomMusic = 0;

		if (previousMusic != roomMusic && roomMusic != 0)
			playMusic(roomMusic);
		if (roomMusic == 0)
			stopMusic();
	} else {
		if (previousMusic != roomMusic && roomMusic != 0)
			playMusic(roomMusic);
	}

	if (currentChapter == 2) {
		if (roomNumber == 9 || roomNumber == 2 || roomNumber == 14 || roomNumber == 18)
			savedTime = getTime();
	}
	if (currentChapter == 4) {
		if (roomNumber == 26)
			savedTime = getTime();
	}

	if (currentChapter == 4 && roomNumber == 24 && flags[29] == 1)
		animation_7_4();

	if (currentChapter == 5) {
		if (roomNumber == 45)
			hare_se_ve = 0;
		if (roomNumber == 49 && flags[7] == 0) {
			playTalkSequence(4);	// sequence 4, chapter 5
		}
	}

	updateRoom();
}

void DrasculaEngine::clearRoom() {
	_system->fillScreen(0);
	_system->updateScreen();
}

bool DrasculaEngine::exitRoom(int doorNumber) {
	debug(2, "Exiting room from door %d", doorNumber);

	int roomNum = 0;

	// Player can't exit the inn in chapter 1
	if (currentChapter == 1 && objectNum[doorNumber] == 104) {
		return false;
	}

	if (currentChapter == 1 && objectNum[doorNumber] == 105 && flags[0] == 0) {
		talk(442);
		return false;
	}

	updateDoor(doorNumber);
	if (isDoor[doorNumber] != 0 &&
		((currentChapter != 3 && currentChapter != 5) || visible[doorNumber] == 1)) {

		hideCursor();
		gotoObject(roomObjX[doorNumber], roomObjY[doorNumber]);
		if (currentChapter != 2) {
			trackProtagonist = trackObj[doorNumber];
			updateRoom();
			updateScreen();
		}
		characterMoved = 0;
		trackProtagonist = trackCharacter_alkeva[doorNumber];
		objExit = roomExits[doorNumber];
		doBreak = 1;
		previousMusic = roomMusic;

		// Object specific actions
		if (currentChapter == 1 && objectNum[doorNumber] == 105) {
			animation_2_1();
			return true;
		} else if (currentChapter == 2) {
			if (objectNum[doorNumber] == 136)
				animation_2_2();
			if (objectNum[doorNumber] == 124) {
				gotoObject(163, 106);
				gotoObject(287, 101);
				trackProtagonist = 0;
			}
			if (objectNum[doorNumber] == 173) {
				animation_35_2();
				return true;
			}
			if (objectNum[doorNumber] == 146 && flags[39] == 1) {
				flags[5] = 1;
				flags[11] = 1;
			}
			if (objectNum[doorNumber] == 176 && flags[29] == 1) {
				flags[29] = 0;
				removeObject(kItemEarWithEarPlug);
				addObject(kItemEarplugs);
			}
		} else if (currentChapter == 4 && objectNum[doorNumber] == 108) {
			gotoObject(171, 78);
		}

		if (currentChapter == 5)
			hare_se_ve = 1;

		clearRoom();
		if (!sscanf(_targetSurface[doorNumber], "%d", &roomNum)) {
			error("Malformed roomNum in targetSurface (%s)", _targetSurface[doorNumber]);
		}
		curX = -1;
		enterRoom(roomNum);

		if (currentChapter == 6 && objExit == 105)
			animation_19_6();
	}

	return false;
}

void DrasculaEngine::updateRoom() {
	copyBackground();
	updateRefresh_pre();
	if (currentChapter == 3) {
		if (flags[0] == 0)
			moveCharacters();
		else
			copyRect(113, 54, curX - 20, curY - 1, 77, 89, drawSurface3, screenSurface);
	} else {
		moveCharacters();
	}
	updateRefresh();
}

void DrasculaEngine::updateDoor(int doorNum) {
	if (currentChapter != 2 && currentChapter != 4)
		return;

	for (int i = 0; i < ARRAYSIZE(doors); i++) {
		if (doors[i].chapter == currentChapter &&
			objectNum[doorNum] == doors[i].doorNum) {
			isDoor[doorNum] = flags[doors[i].flag];
			return;
		}
	}

	if (currentChapter == 4) {
		if (objectNum[doorNum] == 101 && flags[0] == 0)
			isDoor[doorNum] = 0;
		else if (objectNum[doorNum] == 101 && flags[0] == 1 && flags[28] == 1)
			isDoor[doorNum] = 1;
		else if (objectNum[doorNum] == 116 && flags[5] == 0)
			isDoor[doorNum] = 0;
		else if (objectNum[doorNum] == 116 && flags[5] == 1 && flags[23] == 1)
			isDoor[doorNum] = 1;
	}
}

void DrasculaEngine::toggleDoor(int nflag, int doorNum, int action) {
	if ((flags[nflag] == 0 && action == kOpenDoor) ||
		(flags[nflag] == 1 && action == kCloseDoor)) {
		if (currentChapter == 1 && nflag == 7 && action == kOpenDoor)
			return;

		if (action == kOpenDoor) {
			playSound(3);
			flags[nflag] = 1;
		} else {
			playSound(4);
			flags[nflag] = 0;
		}

		if (doorNum != NO_DOOR)
			updateDoor(doorNum);
		updateRoom();
		updateScreen();
		finishSound();
		selectVerb(kVerbNone);
	}
}

} // End of namespace Drascula
