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

#ifndef AGI_PREAGI_TROLL_H
#define AGI_PREAGI_TROLL_H

namespace Agi {

// strings

#define	IDS_TRO_DISK		"ERROR ERROR !"
#define IDS_TRO_PATH_PIC	"%s"

#define IDS_TRO_PRESSANYKEY	"PRESS ANY KEY TO CONTINUE:"

#define IDS_TRO_INTRO_0		"SIERRA ON-LINE INC."
#define IDS_TRO_INTRO_1		"Presents :"
#define IDS_TRO_INTRO_2		"Copyright 1984 Sierra On-Line Inc."
#define IDS_TRO_INTRO_3		"    Press any key to continue.    "
#define IDS_TRO_INTRO_4		"HAVE YOU PLAYED THIS GAME BEFORE ?"
#define IDS_TRO_INTRO_5		"PRESS <Y> OR <N>"

#define IDS_TRO_TUTORIAL_0	" First press the <space bar>.\n  1. Turn the screen GREEN.\n  2. Turn the screen BLACK.\n *3. SEE a SURPRISE, and then more."
#define IDS_TRO_TUTORIAL_1	" Press <return> to make your choice.\n  1. Turn the screen GREEN.\n  2. Turn the screen BLACK.\n  3. SEE a SURPRISE, and then more."
//#define IDS_TRO_TUTORIAL_0	"First press the <space bar>."
//#define IDS_TRO_TUTORIAL_1	"1. Turn the screen GREEN."
//#define IDS_TRO_TUTORIAL_2	"2. Turn the screen BLACK."
//#define IDS_TRO_TUTORIAL_3	"3. SEE a SURPRISE, and then more."
//#define IDS_TRO_TUTORIAL_4	"Press <return> to make your choice."
#define IDS_TRO_TUTORIAL_5	"Would you like more practice ?"
#define IDS_TRO_TUTORIAL_6	"Press <Y> for yes, <N> for no."
#define IDS_TRO_TUTORIAL_7	"The evil TROLL has hidden all the"
#define IDS_TRO_TUTORIAL_8	"Treasures of MARK, the Dwarf King."
#define IDS_TRO_TUTORIAL_9	"Help KING MARK find his Treasures."
#define IDS_TRO_TUTORIAL_10	"You can't take a Treasure if the TROLL"
#define IDS_TRO_TUTORIAL_11	"is in the same picture as the Treasure."
#define IDS_TRO_TUTORIAL_12	"To make the TROLL go away you have to"
#define IDS_TRO_TUTORIAL_13	"make the picture change."
#define IDS_TRO_TUTORIAL_14	"During the game see the Treasures you"
#define IDS_TRO_TUTORIAL_15	"have already found by pressing <F>."
#define IDS_TRO_TUTORIAL_16	"During the game you can turn the sound"
#define IDS_TRO_TUTORIAL_17	"on or off by pressing the <S> key "
#define IDS_TRO_TUTORIAL_18	"while holding down the <Ctrl> key."
#define IDS_TRO_TUTORIAL_19	"The TROLL has hidden these Treasures:"

#define IDS_TRO_CREDITS_0	"Prepare to enter the world of . . ."
#define IDS_TRO_CREDITS_1	"TROLL'S TALE (tm)"
#define IDS_TRO_CREDITS_2	"------------"
#define IDS_TRO_CREDITS_3	"Written by MIKE MACCHESNEY"
#define IDS_TRO_CREDITS_4	"Conversion by PETER OLIPHANT"
#define IDS_TRO_CREDITS_5	"Graphic Art by DOUG MACNEILL"
#define IDS_TRO_CREDITS_6	"Original Version by AL LOWE"

#define IDS_TRO_TREASURE_0	"TREASURES FOUND"
#define IDS_TRO_TREASURE_1	"---------------"
#define IDS_TRO_TREASURE_2	"NONE"
#define IDS_TRO_TREASURE_3	"THERE ARE STILL %d TREASURES TO FIND"
#define IDS_TRO_TREASURE_4	"%d TREASURES TO FIND"
#define IDS_TRO_TREASURE_5	"%d TREASURE TO FIND"
#define IDS_TRO_TREASURE_6	"YOU HAVE FOUND ALL OF THE TREASURES!!"
#define IDS_TRO_TREASURE_7	"THERE'S ONLY ONE MORE TREASURE TO FIND."
#define IDS_TRO_TREASURE_8	"GREAT!! YOU HAVE FOUND EVERY TREASURE."
#define IDS_TRO_TREASURE_9	"TAKE THE TREASURES TO THE GUARD."

#define IDS_TRO_GAMEOVER_0	"You took %d moves to complete TROLL'S"
#define IDS_TRO_GAMEOVER_1	"TALE. Do you think you can do better?"

// picture

#define IDI_TRO_PICNUM 47

#define IDI_TRO_PIC_WIDTH	160
#define IDI_TRO_PIC_HEIGHT	168
#define IDI_TRO_PIC_X0		0
#define IDI_TRO_PIC_Y0		0
#define IDI_TRO_PIC_FLAGS	IDF_AGI_PIC_V15

// max values

#define IDI_TRO_MAX_TREASURE	16
#define IDI_TRO_MAX_OPTION		3

#define IDI_TRO_SEL_OPTION_1	0
#define IDI_TRO_SEL_OPTION_2	1
#define IDI_TRO_SEL_OPTION_3	2

#define IDI_TRO_MAX_ROW_PIC	21

#define IDI_TRO_NUM_ROOMDESCS	65
#define IDI_TRO_NUM_OPTIONS		129
#define IDI_TRO_NUM_NUMROOMS	43

#define IDI_TRO_NUM_USERMSGS    34

#define IDI_TRO_NUM_LOCDESCS    59

#define IDI_TRO_NUM_NONTROLL    9

// offsets

#define IDA_TRO_BINNAME "troll.img"

#define IDO_TRO_DATA_START    0x3A40
#define IDO_TRO_PIC_START	  0x3EF5
#define IDO_TRO_LOCMESSAGES   0x1F7C
#define IDO_TRO_USERMESSAGES  0x34A4
#define IDO_TRO_ROOMDESCS     0x0082
#define IDO_TRO_OPTIONS       0x0364
#define IDO_TRO_PICSTARTIDX   0x02CD
#define IDO_TRO_ROOMPICDELTAS 0x030C
#define IDO_TRO_ALLTREASURES  0x3B24
#define IDO_TRO_ITEMS         0x34E8
#define IDO_TRO_FRAMEPIC      0x3EC2
#define IDO_TRO_ROOMCONNECTS  0x02FA
#define IDO_TRO_NONTROLLROOMS 0x3CF9

enum OptionType {
	OT_GO,
	OT_GET,
	OT_DO,
	OT_FLASHLIGHT
};

struct RoomDesc {
	int options[3];
	OptionType optionTypes[3];
	int roomDescIndex[3];
};

struct UserMsg {
	int num;
	char msg[3][40];
};

struct Item {
	byte bg;
	byte fg;
	char name[16];
};

class TrollEngine : public PreAgiEngine {
public:
	TrollEngine(OSystem *syst, const AGIGameDescription *gameDesc);
	~TrollEngine();

	Common::Error go();

private:
	int _roomPicture;
	int _treasuresLeft;
	int _currentRoom;
	int _moves;

	bool _isTrollAway;

	int _inventory[IDI_TRO_MAX_TREASURE];

	bool _soundOn;

	byte *_gameData;

	void init();
	void intro();
	void drawPic(int iPic, bool f3IsCont, bool clear, bool troll = false);
	void drawTroll();
	void gameLoop();
	void gameOver();
	void tutorial();
	void credits();

	void inventory();
	void pickupTreasure(int treasureId);

	int drawRoom(char *menu);
	void printUserMessage(int msgId);

	void pressAnyKey(int col = 4);
	void waitAnyKeyIntro();

	void playTune(int tune, int len);

	bool getMenuSel(const char*, int*, int);

	void drawMenu(const char *szMenu, int iSel);

	void fillOffsets();

	// These are come from game data
	int _pictureOffsets[IDI_TRO_PICNUM];
	int _roomPicStartIdx[IDI_TRO_NUM_NUMROOMS];
	int _roomPicDeltas[IDI_TRO_NUM_NUMROOMS];
	int _roomStates[IDI_TRO_NUM_NUMROOMS];
	UserMsg _userMessages[IDI_TRO_NUM_USERMSGS];
	int _locMessagesIdx[IDI_TRO_NUM_LOCDESCS];
	RoomDesc _roomDescs[IDI_TRO_NUM_ROOMDESCS];
	int _options[IDI_TRO_NUM_OPTIONS];
	Item _items[IDI_TRO_MAX_TREASURE];
	int _roomConnects[IDI_TRO_NUM_OPTIONS];
	int _nonTrollRooms[IDO_TRO_NONTROLLROOMS];

	int _tunes[6];
};

} // End of namespace Agi

#endif
