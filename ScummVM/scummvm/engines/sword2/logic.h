/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

// logic management

#ifndef SWORD2_LOGIC_H
#define SWORD2_LOGIC_H

#include "common/endian.h"
#include "sword2/animation.h"
#include "sword2/memory.h"

namespace Sword2 {

#define MAX_events 10

#define TREE_SIZE 3

// This must allow for the largest number of objects in a screen
#define OBJECT_KILL_LIST_SIZE 50

#define MAX_SEQUENCE_TEXT_LINES 15

class Sword2Engine;
class Router;

struct EventUnit {
	uint32 id;
	uint32 interact_id;
};

class Logic {
private:
	Sword2Engine *_vm;

	inline byte *decodePtr(int32 n) {
		return _vm->_memory->decodePtr(n);
	}

	uint32 _objectKillList[OBJECT_KILL_LIST_SIZE];

	// keeps note of no. of objects in the kill list
	uint32 _kills;

	// denotes the res id of the game-object-list in current use
	uint32 _currentRunList;

	//pc during logic loop
	uint32 _pc;

	// each object has one of these tacked onto the beginning
	ObjectHub _curObjectHub;

	EventUnit _eventList[MAX_events];

	MoviePlayer *_moviePlayer;

	// Resource id of the wav to use as lead-in/lead-out from smacker
	uint32 _smackerLeadIn;
	uint32 _smackerLeadOut;

	// keeps count of number of text lines to disaply during the sequence
	uint32 _sequenceTextLines;

	MovieText _sequenceTextList[MAX_SEQUENCE_TEXT_LINES];

	// when not playing a wav we calculate the speech time based upon
	// length of ascii

	uint32 _speechTime;

	uint32 _animId;

	// 0 lip synced and repeating - 1 normal once through
	uint32 _speechAnimType;

	uint32 _leftClickDelay;		// click-delay for LEFT mouse button
	uint32 _rightClickDelay;	// click-delay for RIGHT mouse button

	// calculated by locateTalker() for use in speech-panning & text-sprite
	// positioning

	int16 _textX, _textY;

	void locateTalker(int32 *params);
	void formText(int32 *params);
	bool wantSpeechForLine(uint32 wavId);

	// Set by fnPassMega()
	byte _engineMega[56];


	bool _cycleSkip;
	bool _speechRunning;

public:
	Logic(Sword2Engine *vm);
	~Logic();

	EventUnit *getEventList() { return _eventList; }
	byte *getEngineMega() { return _engineMega; }

	byte _saveLogic[8];
	byte _saveGraphic[12];
	byte _saveMega[56];

	// Point to the global variable data
	byte *_scriptVars;

	// "TEXT" - current official text line number - will match the wav
	// filenames

	int16 _officialTextNumber;

	// so speech text cleared when running a new start-script
	uint32 _speechTextBlocNo;

	uint32 readVar(int n) {
		return READ_LE_UINT32(_scriptVars + 4 * n);
	}

	void writeVar(int n, uint32 value) {
		WRITE_LE_UINT32(_scriptVars + 4 * n, value);
	}

	int runResScript(uint32 scriptRes, uint32 offset);
	int runResObjScript(uint32 scriptRes, uint32 objRes, uint32 offset);
	int runScript(byte *scriptData, byte *objectData, uint32 offset);
	int runScript2(byte *scriptData, byte *objectData, byte *offset);

	void sendEvent(uint32 id, uint32 interact_id);
	void setPlayerActionEvent(uint32 id, uint32 interact_id);
	void startEvent();
	int checkEventWaiting();
	void clearEvent(uint32 id);
	void killAllIdsEvents(uint32 id);

	uint32 countEvents();

	struct SyncUnit {
		uint32 id;
		uint32 sync;
	};

	// There won't be many, will there? Probably 2 at most i reckon
	SyncUnit _syncList[10];

	void clearSyncs(uint32 id);
	void sendSync(uint32 id, uint32 sync);
	int getSync();

	Router *_router;

	typedef int32 (Logic::*OpcodeProc)(int32 *);
	struct OpcodeEntry {
		OpcodeProc proc;
		const char *desc;
	};
	const OpcodeEntry *_opcodes;
	int _numOpcodes;
	void setupOpcodes();

	int32 fnTestFunction(int32 *params);
	int32 fnTestFlags(int32 *params);
	int32 fnRegisterStartPoint(int32 *params);
	int32 fnInitBackground(int32 *params);
	int32 fnSetSession(int32 *params);
	int32 fnBackSprite(int32 *params);
	int32 fnSortSprite(int32 *params);
	int32 fnForeSprite(int32 *params);
	int32 fnRegisterMouse(int32 *params);
	int32 fnAnim(int32 *params);
	int32 fnRandom(int32 *params);
	int32 fnPreLoad(int32 *params);
	int32 fnAddSubject(int32 *params);
	int32 fnInteract(int32 *params);
	int32 fnChoose(int32 *params);
	int32 fnWalk(int32 *params);
	int32 fnWalkToAnim(int32 *params);
	int32 fnTurn(int32 *params);
	int32 fnStandAt(int32 *params);
	int32 fnStand(int32 *params);
	int32 fnStandAfterAnim(int32 *params);
	int32 fnPause(int32 *params);
	int32 fnMegaTableAnim(int32 *params);
	int32 fnAddMenuObject(int32 *params);
	int32 fnStartConversation(int32 *params);
	int32 fnEndConversation(int32 *params);
	int32 fnSetFrame(int32 *params);
	int32 fnRandomPause(int32 *params);
	int32 fnRegisterFrame(int32 *params);
	int32 fnNoSprite(int32 *params);
	int32 fnSendSync(int32 *params);
	int32 fnUpdatePlayerStats(int32 *params);
	int32 fnPassGraph(int32 *params);
	int32 fnInitFloorMouse(int32 *params);
	int32 fnPassMega(int32 *params);
	int32 fnFaceXY(int32 *params);
	int32 fnEndSession(int32 *params);
	int32 fnNoHuman(int32 *params);
	int32 fnAddHuman(int32 *params);
	int32 fnWeWait(int32 *params);
	int32 fnTheyDoWeWait(int32 *params);
	int32 fnTheyDo(int32 *params);
	int32 fnWalkToTalkToMega(int32 *params);
	int32 fnFadeDown(int32 *params);
	int32 fnISpeak(int32 *params);
	int32 fnTotalRestart(int32 *params);
	int32 fnSetWalkGrid(int32 *params);
	int32 fnSpeechProcess(int32 *params);
	int32 fnSetScaling(int32 *params);
	int32 fnStartEvent(int32 *params);
	int32 fnCheckEventWaiting(int32 *params);
	int32 fnRequestSpeech(int32 *params);
	int32 fnGosub(int32 *params);
	int32 fnTimedWait(int32 *params);
	int32 fnPlayFx(int32 *params);
	int32 fnStopFx(int32 *params);
	int32 fnPlayMusic(int32 *params);
	int32 fnStopMusic(int32 *params);
	int32 fnSetValue(int32 *params);
	int32 fnNewScript(int32 *params);
	int32 fnGetSync(int32 *params);
	int32 fnWaitSync(int32 *params);
	int32 fnRegisterWalkGrid(int32 *params);
	int32 fnReverseMegaTableAnim(int32 *params);
	int32 fnReverseAnim(int32 *params);
	int32 fnAddToKillList(int32 *params);
	int32 fnSetStandbyCoords(int32 *params);
	int32 fnBackPar0Sprite(int32 *params);
	int32 fnBackPar1Sprite(int32 *params);
	int32 fnForePar0Sprite(int32 *params);
	int32 fnForePar1Sprite(int32 *params);
	int32 fnSetPlayerActionEvent(int32 *params);
	int32 fnSetScrollCoordinate(int32 *params);
	int32 fnStandAtAnim(int32 *params);
	int32 fnSetScrollLeftMouse(int32 *params);
	int32 fnSetScrollRightMouse(int32 *params);
	int32 fnColor(int32 *params);
	int32 fnFlash(int32 *params);
	int32 fnPreFetch(int32 *params);
	int32 fnGetPlayerSaveData(int32 *params);
	int32 fnPassPlayerSaveData(int32 *params);
	int32 fnSendEvent(int32 *params);
	int32 fnAddWalkGrid(int32 *params);
	int32 fnRemoveWalkGrid(int32 *params);
	int32 fnCheckForEvent(int32 *params);
	int32 fnPauseForEvent(int32 *params);
	int32 fnClearEvent(int32 *params);
	int32 fnFaceMega(int32 *params);
	int32 fnPlaySequence(int32 *params);
	int32 fnShadedSprite(int32 *params);
	int32 fnUnshadedSprite(int32 *params);
	int32 fnFadeUp(int32 *params);
	int32 fnDisplayMsg(int32 *params);
	int32 fnSetObjectHeld(int32 *params);
	int32 fnAddSequenceText(int32 *params);
	int32 fnResetGlobals(int32 *params);
	int32 fnSetPalette(int32 *params);
	int32 fnRegisterPointerText(int32 *params);
	int32 fnFetchWait(int32 *params);
	int32 fnRelease(int32 *params);
	int32 fnPrepareMusic(int32 *params);
	int32 fnSoundFetch(int32 *params);
	int32 fnSmackerLeadIn(int32 *params);
	int32 fnSmackerLeadOut(int32 *params);
	int32 fnStopAllFx(int32 *params);
	int32 fnCheckPlayerActivity(int32 *params);
	int32 fnResetPlayerActivityDelay(int32 *params);
	int32 fnCheckMusicPlaying(int32 *params);
	int32 fnPlayCredits(int32 *params);
	int32 fnSetScrollSpeedNormal(int32 *params);
	int32 fnSetScrollSpeedSlow(int32 *params);
	int32 fnRemoveChooser(int32 *params);
	int32 fnSetFxVolAndPan(int32 *params);
	int32 fnSetFxVol(int32 *params);
	int32 fnRestoreGame(int32 *params);
	int32 fnRefreshInventory(int32 *params);
	int32 fnChangeShadows(int32 *params);

	// do one cycle of the current session
	int processSession();

	// cause the logic loop to terminate and drop out
	void expressChangeSession(uint32 sesh_id);

	uint32 getRunList();

	// setup script_id and script_pc in _curObjectHub - called by fnGosub()
	void logicUp(uint32 new_script);

	void logicReplace(uint32 new_script);
	void logicOne(uint32 new_script);
	void resetKillList();

	// Read location number from script vars
	uint32 getLocationNum();
};

} // End of namespace Sword2

#endif
