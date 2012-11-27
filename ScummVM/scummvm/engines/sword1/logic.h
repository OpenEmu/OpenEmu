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

#ifndef SWORD1_LOGIC_H
#define SWORD1_LOGIC_H
// combination of logic.c and scr_int.c

#include "sword1/sworddefs.h"
#include "sword1/objectman.h"
#include "common/util.h"
#include "common/random.h"
#include "audio/mixer.h"

class OSystem;

namespace Sword1 {

#define NON_ZERO_SCRIPT_VARS 95
#define NUM_SCRIPT_VARS      1179

class SwordEngine;
class Text;
class Sound;
class EventManager;
class Menu;
class Router;
class Screen;
class Mouse;
class Music;

class Logic;
typedef int (Logic::*BSMcodeTable)(Object *, int32, int32, int32, int32, int32, int32, int32);

class Logic {
public:
	Logic(SwordEngine *vm, ObjectMan *pObjMan, ResMan *resMan, Screen *pScreen, Mouse *pMouse, Sound *pSound, Music *pMusic, Menu *pMenu, OSystem *system, Audio::Mixer *mixer);
	~Logic();
	void initialize();
	void newScreen(uint32 screen);
	void engine();
	void updateScreenParams();
	void runMouseScript(Object *cpt, int32 scriptId);
	void startPositions(uint32 pos);

	static uint32 _scriptVars[NUM_SCRIPT_VARS];
// public for mouse (menu looking)
	int cfnPresetScript(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
private:
	SwordEngine *_vm;
	ObjectMan *_objMan;
	OSystem *_system;
	Audio::Mixer *_mixer;
	ResMan *_resMan;
	Screen *_screen;
	Sound *_sound;
	Mouse *_mouse;
	Router *_router;
	Text *_textMan;
	EventManager *_eventMan;
	Menu *_menu;
	Music *_music;
	uint32 _newScript; // <= ugly, but I can't avoid it.
	bool _speechRunning, _speechFinished, _textRunning;
	uint8 _speechClickDelay;
	Common::RandomSource _rnd;

	int scriptManager(Object *compact, uint32 id);
	void processLogic(Object *compact, uint32 id);
	int interpretScript(Object *compact, int id, Header *scriptModule, int scriptBase, int scriptNum);

	int logicWaitTalk(Object *compact);
	int logicStartTalk(Object *compact);
	int logicArAnimate(Object *compact, uint32 id);
	int speechDriver(Object *compact);
	int fullAnimDriver(Object *compact);
	int animDriver(Object *compact);

	void setupMcodeTable();
	const BSMcodeTable *_mcodeTable;

//- mcodeTable:
	int fnBackground(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnForeground(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSort(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnNoSprite(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnMegaSet(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnAnim(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetFrame(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFullAnim(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFullSetFrame(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFadeDown(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFadeUp(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnCheckFade(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetSpritePalette(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetWholePalette(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetFadeTargetPalette(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetPaletteToFade(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetPaletteToCut(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPlaySequence(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

	int fnIdle(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPause(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPauseSeconds(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnQuit(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnKillId(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSuicide(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnNewScript(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSubScript(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnRestartScript(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetBookmark(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGotoBookmark(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSendSync(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnWaitSync(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

	int cfnClickInteract(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int cfnSetScript(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

	int fnInteract(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnIssueEvent(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnCheckForEvent(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnWipeHands(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnISpeak(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnTheyDo(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnTheyDoWeWait(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnWeWait(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnChangeSpeechText(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnTalkError(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStartTalk(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnCheckForTextLine(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnAddTalkWaitStatusBit(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnRemoveTalkWaitStatusBit(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

	int fnNoHuman(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnAddHuman(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnBlankMouse(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnNormalMouse(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnLockMouse(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnUnlockMouse(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetMousePointer(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetMouseLuggage(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnMouseOn(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnMouseOff(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnChooser(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnEndChooser(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStartMenu(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnEndMenu(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

	int cfnReleaseMenu(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

	int fnAddSubject(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnAddObject(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnRemoveObject(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnEnterSection(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnLeaveSection(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnChangeFloor(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnWalk(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnTurn(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStand(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStandAt(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFace(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnFaceXy(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnIsFacing(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGetTo(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGetToError(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGetPos(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGetGamepadXy(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPlayFx(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStopFx(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPlayMusic(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnStopMusic(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnInnerSpace(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnRandom(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetScreen(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPreload(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnCheckCD(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnRestartGame(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnQuitGame(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnDeathScreen(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnSetParallax(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnTdebug(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);

	int fnRedFlash(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnBlueFlash(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnYellow(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnGreen(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnPurple(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	int fnBlack(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x);
	static const uint32 _scriptVarInit[NON_ZERO_SCRIPT_VARS][2];
	static const uint8 *const _startData[];
	static const uint8 *const _helperData[];
	void startPosCallFn(uint8 fnId, uint32 param1, uint32 param2, uint32 param3);
	void runStartScript(const uint8 *data);
};

enum StartPosOpcodes {
	opcSeqEnd = 0,
	opcCallFn,
	opcCallFnLong,
	opcSetVar8,
	opcSetVar16,
	opcSetVar32,
	opcGeorge,
	opcRunStart,
	opcRunHelper,
	opcPlaySequence,
	opcAddObject,
	opcRemoveObject,
	opcMegaSet,
	opcNoSprite
};

enum HelperScripts {
	HELP_IRELAND = 0,
	HELP_SYRIA,
	HELP_SPAIN,
	HELP_NIGHTTRAIN,
	HELP_SCOTLAND,
	HELP_WHITECOAT,
	HELP_SPAIN2
};

} // End of namespace Sword1

#endif //BSLOGIC_H
