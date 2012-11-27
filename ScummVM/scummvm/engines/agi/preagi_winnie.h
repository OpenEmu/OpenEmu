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

#ifndef AGI_PREAGI_WINNIE_H
#define AGI_PREAGI_WINNIE_H

namespace Agi {

#define WTP_SAVEGAME_VERSION	1
#define IDI_XOR_KEY				0x80

// strings
#define IDS_WTP_ROOM_DOS		"rooms/rm.%02d"
#define IDS_WTP_ROOM_AMIGA		"room.%d"
#define IDS_WTP_ROOM_C64		"room%02d"
#define IDS_WTP_ROOM_APPLE		"room%d.obj"
#define IDS_WTP_OBJ_DOS			"obj.%02d"
#define IDS_WTP_OBJ_AMIGA		"object.%d"
#define IDS_WTP_OBJ_C64			"object%02d"
#define IDS_WTP_OBJ_APPLE		"object%d.obj"
#define IDS_WTP_SND_DOS			"snd.%02d"
#define IDS_WTP_SND_AMIGA		"Sounds"
#define IDS_WTP_SND_C64			"sound.obj"
#define IDS_WTP_SND_APPLE		"sound.obj"

#define IDS_WTP_FILE_LOGO		"logo"
#define IDS_WTP_FILE_TITLE		"title"
#define IDS_WTP_FILE_SAVEGAME	"savegame"
#define IDS_WTP_FILE_RND		"rnd"

#define IDS_WTP_DISK_ERROR		"There is a problem with your disk drive.Please make sure your Winnie-the-Pooh   disk is in the drive correctly."

#define IDS_WTP_INTRO_0			"                 PRESENT"
#define IDS_WTP_INTRO_1			"       TM designates trademark of\n          Sierra On-Line, Inc.\n    (c) 1985 Walt Disney Productions"

#define IDS_WTP_HELP_0			"The <SPACE BAR> moves the pointer.      Press <RETURN> when it is by the choice you want.  Press the <Backspace> key to see what you just finished reading."
#define IDS_WTP_HELP_1			"Press <C> to see what you are carrying. <Ctrl-S> turns the sound off and on.    <ESC> takes you to the playroom (in caseyou get lost or want to save the game)."

#define IDS_WTP_GAME_OVER_0		"Congratulations!!  You did it!  You     returned everything that was lost.  Now,Christopher Robin invites you to a Hero party."
#define IDS_WTP_GAME_OVER_1		"The good news is:  YOU are the Hero!!   The bad news is:  you have to find the  party by yourself.  Good luck!"

#define IDS_WTP_OWL_0			"\"For example, that object you are       carrying now is interesting.  I know    I've seen it before.  Hmmm.  Let me     think about this . . .\""
#define IDS_WTP_OWL_1			"\"You know, this object here beside me isfamiliar.  I'm sure I could give you    some sort of clue about it.             Let me see. . .\""

#define IDS_WTP_WIND_0			"Oh, no!  The Blustery Wind begins to    howl.  It has returned, and mixed up    all the objects in the Wood."
#define IDS_WTP_WIND_1			"But don't worry.  Everyone still has theobjects you returned to them.\n\n             (Today must be Winds-day!)"
#define IDS_WTP_TIGGER			"\"Hallooooo, there!!!!  It's ME, Tigger! Let's BOUNCE!\""
#define IDS_WTP_MIST			"Oh, look out!  The mysterious mist is   coming in.  It gets so thick that you   can't see through it.  Just keep walkingand it will soon clear up."

#define IDS_WTP_SONG_0			"Winnie-the-Pooh, Winnie-the-Pooh, Tubby little cubby all stuffed with fluff,    He's Winnie-the-Pooh, Winnie-the-Pooh,  Willy, nilly, silly, old bear."
#define IDS_WTP_SONG_1			"Deep in the Hundred Acre Wood,          Where Christopher Robin plays,          You will find the enchanted neighborhoodof Christopher's childhood days."
#define IDS_WTP_SONG_2			"A donkey named Eeyore is his friend,    and Kanga and little Roo.  There's      Rabbit and Piglet and there's Owl       But most of all Winnie-the-Pooh!"

#define IDS_WTP_NSEW			"North  South  East  West"
#define IDS_WTP_TAKE			"Take"
#define IDS_WTP_DROP			"Drop"
#define IDS_WTP_CANT_GO			"\nSorry, but you can't go that way."
#define IDS_WTP_CANT_TAKE		"You can't take it.  You can only carry  one object at a time."
#define IDS_WTP_CANT_DROP		"You can't drop it.  Another object is   already here."
#define IDS_WTP_WRONG_PLACE		"\nOk, but it doesn't belong here."
#define IDS_WTP_OK				"\nOk."

#define IDS_WTP_INVENTORY_0		"You are carrying nothing."
#define IDS_WTP_INVENTORY_1		"Number of objects still missing:  %d"

// COMMODORE 64 version strings

#define IDS_WTP_FILE_SAVEGAME_C64	"saved game"
#define IDS_WTP_DISK_ERROR_C64		"There is a problem with your disk drive.Please make sure your disk is in the    drive correctly."
#define IDS_WTP_HELP_0_C64			"The <SPACE BAR> moves the pointer.      Press <RETURN> when it is by the choice you want.  <F1> brings back what you    have already read."
#define IDS_WTP_HELP_1_C64			"<F3> takes you back to the playroom (if you get lost, or want to save the game).<F5> turns the sound off and on.        <F7> shows what you're carrying."
#define IDS_WTP_WRONG_PLACE_C64	"\nOk, but this is not the right place."

// maximum values

#define IDI_WTP_MAX_OBJ_MISSING	10

#define IDI_WTP_MAX_ROOM		62
#define IDI_WTP_MAX_OBJ			40
#define IDI_WTP_MAX_SND			14
#define IDI_WTP_MAX_PIC			2

#define IDI_WTP_MAX_ROOM_NORMAL			57
#define IDI_WTP_MAX_ROOM_TELEPORT		30
#define IDI_WTP_MAX_ROOM_OBJ			42
#define IDI_WTP_MAX_BLOCK				4
#define IDI_WTP_MAX_STR					6
#define IDI_WTP_MAX_OBJ_STR				4
#define IDI_WTP_MAX_OBJ_STR_END			2
#define IDI_WTP_MAX_FLAG				40
#define IDI_WTP_MAX_OPTION				3
#define IDI_WTP_MAX_DIR					4
#define IDI_WTP_MAX_MOVES_UNTIL_WIND	150

// positions

#define IDI_WTP_ROW_MENU		21
#define IDI_WTP_ROW_OPTION_1	21
#define IDI_WTP_ROW_OPTION_2	22
#define IDI_WTP_ROW_OPTION_3	23
#define IDI_WTP_ROW_OPTION_4	24

#define IDI_WTP_COL_MENU		0
#define IDI_WTP_COL_OPTION		1
#define IDI_WTP_COL_NSEW		1
#define IDI_WTP_COL_NORTH		1
#define IDI_WTP_COL_SOUTH		8
#define IDI_WTP_COL_EAST		15
#define IDI_WTP_COL_WEST		21
#define IDI_WTP_COL_TAKE		29
#define IDI_WTP_COL_DROP		35
#define IDI_WTP_COL_PRESENT		17

// data file offset modifiers

#define IDI_WTP_OFS_ROOM		0x5400
#define IDI_WTP_OFS_OBJ			0x0800

// picture

#define IDI_WTP_PIC_WIDTH	140
#define IDI_WTP_PIC_HEIGHT	159
#define IDI_WTP_PIC_X0		10
#define IDI_WTP_PIC_Y0		0
#define IDI_WTP_PIC_FLAGS	IDF_AGI_PIC_V2

// selections

enum {
	IDI_WTP_SEL_HOME = -2,
	IDI_WTP_SEL_BACK,
	IDI_WTP_SEL_OPT_1,
	IDI_WTP_SEL_OPT_2,
	IDI_WTP_SEL_OPT_3,
	IDI_WTP_SEL_NORTH,
	IDI_WTP_SEL_SOUTH,
	IDI_WTP_SEL_EAST,
	IDI_WTP_SEL_WEST,
	IDI_WTP_SEL_TAKE,
	IDI_WTP_SEL_DROP,
	IDI_WTP_SEL_REAL_OPT_1,
	IDI_WTP_SEL_REAL_OPT_2,
	IDI_WTP_SEL_REAL_OPT_3
};

#define IDI_WTP_SEL_LAST	IDI_WTP_SEL_REAL_OPT_3

// rooms

enum {
	IDI_WTP_ROOM_NONE = -1,
	IDI_WTP_ROOM_NORTH,
	IDI_WTP_ROOM_SOUTH,
	IDI_WTP_ROOM_EAST,
	IDI_WTP_ROOM_WEST
};

#define IDI_WTP_ROOM_HIDE	0

#define IDI_WTP_ROOM_PICNIC	2
#define IDI_WTP_ROOM_HOME	28
#define IDI_WTP_ROOM_PARTY	58
#define IDI_WTP_ROOM_MIST	59
#define IDI_WTP_ROOM_TIGGER	61

// sound

enum ENUM_WTP_SOUND {
	IDI_WTP_SND_POOH_0 = 1,
	IDI_WTP_SND_TIGGER,
	IDI_WTP_SND_TAKE,
	IDI_WTP_SND_DROP,
	IDI_WTP_SND_DROP_OK,
	IDI_WTP_SND_FANFARE,
	IDI_WTP_SND_POOH_1,
	IDI_WTP_SND_KEYHELP,
	IDI_WTP_SND_POOH_2,
	IDI_WTP_SND_WIND_0,
	IDI_WTP_SND_WIND_1
};

// script opcodes

#define IDO_WTP_GOTO_ROOM	0x06
#define IDO_WTP_PRINT_MSG	0x08
#define IDO_WTP_PRINT_STR	0x0A
#define IDO_WTP_DROP_OBJ	0x0C
#define IDO_WTP_FLAG_CLEAR	0x0E
#define IDO_WTP_FLAG_SET	0x10
#define IDO_WTP_GAME_OVER	0x12
#define IDO_WTP_WALK_MIST	0x14
#define IDO_WTP_PLAY_SOUND	0x16
#define IDO_WTP_SAVE_GAME	0x18
#define IDO_WTP_LOAD_GAME	0x1A
#define IDO_WTP_OWL_HELP	0x1C
#define IDO_WTP_GOTO_RND	0x1E

#define IDO_WTP_OPTION_0	0x15
#define IDO_WTP_OPTION_1	0x16
#define IDO_WTP_OPTION_2	0x17

enum {
	IDI_WTP_OBJ_DESC = 0,
	IDI_WTP_OBJ_TAKE,
	IDI_WTP_OBJ_DROP,
	IDI_WTP_OBJ_HELP
};

enum {
	IDI_WTP_PAR_OK = 0,
	IDI_WTP_PAR_GOTO,
	IDI_WTP_PAR_BACK
};

// room file option block

struct WTP_ROOM_BLOCK {
	uint16	ofsOpt[IDI_WTP_MAX_BLOCK];
};

// room file header

struct WTP_ROOM_HDR {
	uint8	roomNumber;
	uint8	objId;
	uint16	ofsPic;
	uint16	fileLen;
	uint16	reserved0;
	int8	roomNew[IDI_WTP_MAX_DIR];
	uint8	objX;
	uint8	objY;
	uint16	reserved1;
	uint16	ofsDesc[IDI_WTP_MAX_BLOCK];
	uint16	ofsBlock[IDI_WTP_MAX_BLOCK];
	uint16	ofsStr[IDI_WTP_MAX_STR];
	uint32	reserved2;
	WTP_ROOM_BLOCK	opt[IDI_WTP_MAX_BLOCK];
};

// object file header

struct WTP_OBJ_HDR {
	uint16	fileLen;
	uint16	objId;
	uint16	ofsEndStr[IDI_WTP_MAX_OBJ_STR_END];
	uint16	ofsStr[IDI_WTP_MAX_OBJ_STR];
	uint16	ofsPic;
};

// savegame

struct WTP_SAVE_GAME {
	uint8	fSound;
	uint8	nMoves;
	uint8	nObjMiss;
	uint8	nObjRet;
	uint8	iObjHave;
	uint8	fGame[IDI_WTP_MAX_FLAG];
	uint8	iUsedObj[IDI_WTP_MAX_OBJ_MISSING];
	uint8	iObjRoom[IDI_WTP_MAX_ROOM_OBJ];
};

class PreAgiEngine;

class WinnieEngine : public PreAgiEngine {
public:
	WinnieEngine(OSystem *syst, const AGIGameDescription *gameDesc);
	~WinnieEngine();

	void init();
	Common::Error go();

	void debugCurRoom();

	GUI::Debugger *getDebugger() { return _console; }

private:
	WinnieConsole *_console;

	WTP_SAVE_GAME _gameStateWinnie;
	int _room;
	int	_mist;
	bool _doWind;
	bool _winnieEvent;
	int	_tiggerMist;

	int _roomOffset;
	int _objOffset;
	bool _isBigEndian;
	Common::Rect hotspotNorth, hotspotSouth, hotspotEast, hotspotWest;

	void randomize();
	void intro();
	void drawPic(const char*);
	void gameLoop();

	void parseRoomHeader(WTP_ROOM_HDR *roomHdr, byte *buffer, int len);
	void parseObjHeader(WTP_OBJ_HDR *objHdr, byte *buffer, int len);
	uint32 readRoom(int, uint8*, WTP_ROOM_HDR&);

	void drawRoomPic();
	int parser(int, int, uint8*);
	int getObjInRoom(int);
	bool getSelOkBack();
	void getMenuSel(char*, int*, int[]);
	void keyHelp();
	void clrMenuSel(int*, int[]);
	void incMenuSel(int*, int[]);
	void decMenuSel(int*, int[]);
	void drawMenu(char*, int, int[]);
	void printRoomStr(int, int);
	void inventory();
	void printObjStr(int, int);
	uint32 readObj(int, uint8*);
	void takeObj(int);
	void dropObj(int);
	bool isRightObj(int, int, int*);
	void drawObjPic(int, int, int);
	void getMenuMouseSel(int*, int[], int, int);
	void setFlag(int);
	void clearFlag(int);
	void gameOver();
	void saveGame();
	void loadGame();
	void dropObjRnd();
	void setTakeDrop(int[]);
	void makeSel(int*, int[]);

	void wind();
	void mist();
	void tigger();

	void showOwlHelp();
	bool playSound(ENUM_WTP_SOUND);

	void printStrWinnie(char *szMsg);
};

}  // End of namespace Agi

#endif
