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

#ifndef SKY_LOGIC_H
#define SKY_LOGIC_H

#include "common/util.h"
#include "common/random.h"

namespace Sky {

struct Compact;

enum scriptVariableOffsets {
	RESULT = 0,
	SCREEN = 1,
	LOGIC_LIST_NO = 2,
	MOUSE_LIST_NO = 6,
	DRAW_LIST_NO = 8,
	CUR_ID = 12,
	MOUSE_STATUS = 13,
	MOUSE_STOP = 14,
	BUTTON = 15,
	SPECIAL_ITEM = 17,
	GET_OFF = 18,
	CURSOR_ID = 22,
	SAFEX = 25,
	SAFEY = 26,
	PLAYER_X = 27,
	PLAYER_Y = 28,
	PLAYER_MOOD = 29,
	PLAYER_SCREEN = 30,
	HIT_ID = 37,
	LAYER_0_ID = 41,
	LAYER_1_ID = 42,
	LAYER_2_ID = 43,
	LAYER_3_ID = 44,
	GRID_1_ID = 45,
	GRID_2_ID = 46,
	GRID_3_ID = 47,
	THE_CHOSEN_ONE = 51,
	TEXT1 = 53,
	MENU_LENGTH = 100,
	SCROLL_OFFSET = 101,
	MENU = 102,
	OBJECT_HELD = 103,
	LAMB_GREET = 109,
	RND = 115,
	CUR_SECTION = 143,
	JOEY_SECTION = 145,
	LAMB_SECTION = 146,
	KNOWS_PORT = 190,
	GOT_SPONSOR = 240,
	GOT_JAMMER = 258,
	CONSOLE_TYPE = 345,
	S15_FLOOR = 450,
	FOREMAN_FRIEND = 451,
	REICH_DOOR_FLAG = 470,
	CARD_STATUS = 479,
	CARD_FIX = 480,
	GUARDIAN_THERE = 640,
	FS_COMMAND = 643,
	ENTER_DIGITS = 644,
	LINC_DIGIT_0 = 646,
	LINC_DIGIT_1 = 647,
	LINC_DIGIT_2 = 648,
	LINC_DIGIT_3 = 649,
	LINC_DIGIT_4 = 650,
	LINC_DIGIT_5 = 651,
	LINC_DIGIT_6 = 651,
	LINC_DIGIT_7 = 653,
	LINC_DIGIT_8 = 654,
	LINC_DIGIT_9 = 655,
	DOOR_67_68_FLAG = 678,
	SC70_IRIS_FLAG = 693,
	DOOR_73_75_FLAG = 704,
	SC76_CABINET1_FLAG = 709,
	SC76_CABINET2_FLAG = 710,
	SC76_CABINET3_FLAG = 711,
	DOOR_77_78_FLAG = 719,
	SC80_EXIT_FLAG = 720,
	SC31_LIFT_FLAG = 793,
	SC32_LIFT_FLAG = 797,
	SC33_SHED_DOOR_FLAG = 798,
	BAND_PLAYING = 804,
	COLSTON_AT_TABLE = 805,
	SC36_NEXT_DEALER = 806,
	SC36_DOOR_FLAG = 807,
	SC37_DOOR_FLAG = 808,
	SC40_LOCKER_1_FLAG = 817,
	SC40_LOCKER_2_FLAG = 818,
	SC40_LOCKER_3_FLAG = 819,
	SC40_LOCKER_4_FLAG = 820,
	SC40_LOCKER_5_FLAG = 821
};

#define NUM_SKY_SCRIPTVARS 838

class AutoRoute;
class Control;
class Disk;
class Grid;
class Mouse;
class MusicBase;
class Screen;
class Sound;
class Text;
class SkyCompact;

class Logic;

typedef void (Logic::*LogicTable) ();
typedef bool (Logic::*McodeTable) (uint32, uint32, uint32);

class Logic {
public:
	Logic(
		SkyCompact *skyCompact,
		Screen *skyScreen,
		Disk *skyDisk,
		Text *skyText,
		MusicBase *skyMusic,
		Mouse *skyMouse,
		Sound *skySound);
	~Logic();
	void engine();
	void useControlInstance(Control *control) { _skyControl = control; }

	uint16 mouseScript(uint32 scrNum, Compact *scriptComp);

	static uint32 _scriptVariables[NUM_SKY_SCRIPTVARS];
	Grid *_skyGrid;

	uint16 script(uint16 scriptNo, uint16 offset);
	void initScreen0();
	void parseSaveData(uint32 *data);

private:
	void setupLogicTable();
	void setupMcodeTable();
	const LogicTable *_logicTable;
	const McodeTable *_mcodeTable;

protected:
	void push(uint32);
	uint32 pop();
	void checkModuleLoaded(uint16 moduleNo);
	bool collide(Compact *cpt);
	void initScriptVariables();
	void mainAnim();
	void runGetOff();
	void stopAndWait();
	bool checkProtection();

	void nop();
	void logicScript();
	void autoRoute();
	void arAnim();
	void arTurn();
	void alt();
	void anim();
	void turn();
	void cursor();
	void talk();
	void listen();
	void stopped();
	void choose();
	void frames();
	void pause();
	void waitSync();
	void simpleAnim();

	bool fnCacheChip(uint32 a, uint32 b, uint32 c);
	bool fnCacheFast(uint32 a, uint32 b, uint32 c);
	bool fnDrawScreen(uint32 a, uint32 b, uint32 c);
	bool fnAr(uint32 a, uint32 b, uint32 c);
	bool fnArAnimate(uint32 a, uint32 b, uint32 c);
	bool fnIdle(uint32 a, uint32 b, uint32 c);
	bool fnInteract(uint32 a, uint32 b, uint32 c);
	bool fnStartSub(uint32 a, uint32 b, uint32 c);
	bool fnTheyStartSub(uint32 a, uint32 b, uint32 c);
	bool fnAssignBase(uint32 a, uint32 b, uint32 c);
	bool fnDiskMouse(uint32 a, uint32 b, uint32 c);
	bool fnNormalMouse(uint32 a, uint32 b, uint32 c);
	bool fnBlankMouse(uint32 a, uint32 b, uint32 c);
	bool fnCrossMouse(uint32 a, uint32 b, uint32 c);
	bool fnCursorRight(uint32 a, uint32 b, uint32 c);
	bool fnCursorLeft(uint32 a, uint32 b, uint32 c);
	bool fnCursorDown(uint32 a, uint32 b, uint32 c);
	bool fnOpenHand(uint32 a, uint32 b, uint32 c);
	bool fnCloseHand(uint32 a, uint32 b, uint32 c);
	bool fnGetTo(uint32 a, uint32 b, uint32 c);
	bool fnSetToStand(uint32 a, uint32 b, uint32 c);
	bool fnTurnTo(uint32 a, uint32 b, uint32 c);
	bool fnArrived(uint32 a, uint32 b, uint32 c);
	bool fnLeaving(uint32 a, uint32 b, uint32 c);
	bool fnSetAlternate(uint32 a, uint32 b, uint32 c);
	bool fnAltSetAlternate(uint32 a, uint32 b, uint32 c);
	bool fnKillId(uint32 a, uint32 b, uint32 c);
	bool fnNoHuman(uint32 a, uint32 b, uint32 c);
	bool fnAddHuman(uint32 a, uint32 b, uint32 c);
	bool fnAddButtons(uint32 a, uint32 b, uint32 c);
	bool fnNoButtons(uint32 a, uint32 b, uint32 c);
	bool fnSetStop(uint32 a, uint32 b, uint32 c);
	bool fnClearStop(uint32 a, uint32 b, uint32 c);
	bool fnPointerText(uint32 a, uint32 b, uint32 c);
	bool fnQuit(uint32 a, uint32 b, uint32 c);
	bool fnSpeakMe(uint32 targetId, uint32 mesgNum, uint32 animNum);
	bool fnSpeakMeDir(uint32 targetId, uint32 mesgNum, uint32 animNum);
	bool fnSpeakWait(uint32 a, uint32 b, uint32 c);
	bool fnSpeakWaitDir(uint32 a, uint32 b, uint32 c);
	bool fnChooser(uint32 a, uint32 b, uint32 c);
	bool fnHighlight(uint32 a, uint32 b, uint32 c);
	bool fnTextKill(uint32 a, uint32 b, uint32 c);
	bool fnStopMode(uint32 a, uint32 b, uint32 c);
	bool fnWeWait(uint32 a, uint32 b, uint32 c);
	bool fnSendSync(uint32 a, uint32 b, uint32 c);
	bool fnSendFastSync(uint32 a, uint32 b, uint32 c);
	bool fnSendRequest(uint32 a, uint32 b, uint32 c);
	bool fnClearRequest(uint32 a, uint32 b, uint32 c);
	bool fnCheckRequest(uint32 a, uint32 b, uint32 c);
	bool fnStartMenu(uint32 a, uint32 b, uint32 c);
	bool fnUnhighlight(uint32 a, uint32 b, uint32 c);
	bool fnFaceId(uint32 a, uint32 b, uint32 c);
	bool fnForeground(uint32 a, uint32 b, uint32 c);
	bool fnBackground(uint32 a, uint32 b, uint32 c);
	bool fnNewBackground(uint32 a, uint32 b, uint32 c);
	bool fnSort(uint32 a, uint32 b, uint32 c);
	bool fnNoSpriteEngine(uint32 a, uint32 b, uint32 c);
	bool fnNoSpritesA6(uint32 a, uint32 b, uint32 c);
	bool fnResetId(uint32 a, uint32 b, uint32 c);
	bool fnToggleGrid(uint32 a, uint32 b, uint32 c);
	bool fnPause(uint32 a, uint32 b, uint32 c);
	bool fnRunAnimMod(uint32 a, uint32 b, uint32 c);
	bool fnSimpleMod(uint32 a, uint32 b, uint32 c);
	bool fnRunFrames(uint32 a, uint32 b, uint32 c);
	bool fnAwaitSync(uint32 a, uint32 b, uint32 c);
	bool fnIncMegaSet(uint32 a, uint32 b, uint32 c);
	bool fnDecMegaSet(uint32 a, uint32 b, uint32 c);
	bool fnSetMegaSet(uint32 a, uint32 b, uint32 c);
	bool fnMoveItems(uint32 a, uint32 b, uint32 c);
	bool fnNewList(uint32 a, uint32 b, uint32 c);
	bool fnAskThis(uint32 a, uint32 b, uint32 c);
	bool fnRandom(uint32 a, uint32 b, uint32 c);
	bool fnPersonHere(uint32 a, uint32 b, uint32 c);
	bool fnToggleMouse(uint32 a, uint32 b, uint32 c);
	bool fnMouseOn(uint32 a, uint32 b, uint32 c);
	bool fnMouseOff(uint32 a, uint32 b, uint32 c);
	bool fnFetchX(uint32 a, uint32 b, uint32 c);
	bool fnFetchY(uint32 a, uint32 b, uint32 c);
	bool fnTestList(uint32 a, uint32 b, uint32 c);
	bool fnFetchPlace(uint32 a, uint32 b, uint32 c);
	bool fnCustomJoey(uint32 a, uint32 b, uint32 c);
	bool fnSetPalette(uint32 a, uint32 b, uint32 c);
	bool fnTextModule(uint32 a, uint32 b, uint32 c);
	bool fnChangeName(uint32 a, uint32 b, uint32 c);
	bool fnMiniLoad(uint32 a, uint32 b, uint32 c);
	bool fnFlushBuffers(uint32 a, uint32 b, uint32 c);
	bool fnFlushChip(uint32 a, uint32 b, uint32 c);
	bool fnSaveCoods(uint32 a, uint32 b, uint32 c);
	bool fnPlotGrid(uint32 a, uint32 b, uint32 c);
	bool fnRemoveGrid(uint32 a, uint32 b, uint32 c);
	bool fnEyeball(uint32 a, uint32 b, uint32 c);
	bool fnCursorUp(uint32 a, uint32 b, uint32 c);
	bool fnLeaveSection(uint32 a, uint32 b, uint32 c);
	bool fnEnterSection(uint32 sectionNo, uint32 b, uint32 c);
	bool fnRestoreGame(uint32 a, uint32 b, uint32 c);
	bool fnRestartGame(uint32 a, uint32 b, uint32 c);
	bool fnNewSwingSeq(uint32 a, uint32 b, uint32 c);
	bool fnWaitSwingEnd(uint32 a, uint32 b, uint32 c);
	bool fnSkipIntroCode(uint32 a, uint32 b, uint32 c);
	bool fnBlankScreen(uint32 a, uint32 b, uint32 c);
	bool fnPrintCredit(uint32 a, uint32 b, uint32 c);
	bool fnLookAt(uint32 a, uint32 b, uint32 c);
	bool fnLincTextModule(uint32 a, uint32 b, uint32 c);
	bool fnTextKill2(uint32 a, uint32 b, uint32 c);
	bool fnSetFont(uint32 a, uint32 b, uint32 c);
	bool fnStartFx(uint32 a, uint32 b, uint32 c);
	bool fnStopFx(uint32 a, uint32 b, uint32 c);
	bool fnStartMusic(uint32 a, uint32 b, uint32 c);
	bool fnStopMusic(uint32 a, uint32 b, uint32 c);
	bool fnFadeDown(uint32 a, uint32 b, uint32 c);
	bool fnFadeUp(uint32 a, uint32 b, uint32 c);
	bool fnQuitToDos(uint32 a, uint32 b, uint32 c);
	bool fnPauseFx(uint32 a, uint32 b, uint32 c);
	bool fnUnPauseFx(uint32 a, uint32 b, uint32 c);
	bool fnPrintf(uint32 a, uint32 b, uint32 c);

	void stdSpeak(Compact *target, uint32 textNum, uint32 animNum, uint32 base);
	void fnExec(uint16 num, uint32 a, uint32 b, uint32 c);

	uint16 *_moduleList[16];
	uint32 _stack[20];
	byte _stackPtr;

	Compact *_compact;

	uint32 _objectList[30];

	uint32 _currentSection;

	Common::RandomSource _rnd;

	SkyCompact	*_skyCompact;
	Screen		*_skyScreen;
	Disk		*_skyDisk;
	Text		*_skyText;
	MusicBase	*_skyMusic;
	Sound		*_skySound;
	AutoRoute	*_skyAutoRoute;
	Mouse		*_skyMouse;
	Control		*_skyControl;

	friend class Debugger;
};

} // End of namespace Sky

#endif
