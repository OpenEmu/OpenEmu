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

#include "common/endian.h"
#include "common/util.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "sword1/logic.h"
#include "sword1/text.h"
#include "sword1/sound.h"
#include "sword1/eventman.h"
#include "sword1/menu.h"
#include "sword1/router.h"
#include "sword1/screen.h"
#include "sword1/mouse.h"
#include "sword1/sword1.h"
#include "sword1/music.h"
#include "sword1/swordres.h"
#include "sword1/animation.h"

#include "sword1/debug.h"

#include "gui/message.h"

namespace Sword1 {

#define MAX_STACK_SIZE 10
#define SCRIPT_VERSION  13
#define LAST_FRAME 999

uint32 Logic::_scriptVars[NUM_SCRIPT_VARS];

Logic::Logic(SwordEngine *vm, ObjectMan *pObjMan, ResMan *resMan, Screen *pScreen, Mouse *pMouse, Sound *pSound, Music *pMusic, Menu *pMenu, OSystem *system, Audio::Mixer *mixer)
	: _rnd("sword1") {

	_vm = vm;
	_objMan = pObjMan;
	_resMan = resMan;
	_screen = pScreen;
	_mouse = pMouse;
	_music = pMusic;
	_sound = pSound;
	_menu = pMenu;
	_textMan = NULL;
	_screen->useTextManager(_textMan);
	_router = new Router(_objMan, _resMan);
	_eventMan = NULL;
	_system = system;
	_mixer = mixer;

	setupMcodeTable();
}

Logic::~Logic() {
	delete _textMan;
	delete _router;
	delete _eventMan;
}

void Logic::initialize() {
	memset(_scriptVars, 0, NUM_SCRIPT_VARS * sizeof(uint32));
	for (uint8 cnt = 0; cnt < NON_ZERO_SCRIPT_VARS; cnt++)
		_scriptVars[_scriptVarInit[cnt][0]] = _scriptVarInit[cnt][1];
	if (SwordEngine::_systemVars.isDemo)
		_scriptVars[PLAYINGDEMO] = 1;

	delete _eventMan;
	_eventMan = new EventManager();

	delete _textMan;
	_textMan = new Text(_objMan, _resMan,
	                    (SwordEngine::_systemVars.language == BS1_CZECH) ? true : false);
	_screen->useTextManager(_textMan);
	_textRunning = _speechRunning = false;
	_speechFinished = true;
}

void Logic::newScreen(uint32 screen) {
	Object *compact = (Object *)_objMan->fetchObject(PLAYER);

	// work around script bug #911508
	if (((screen == 25) || (_scriptVars[SCREEN] == 25)) && (_scriptVars[SAND_FLAG] == 4)) {
		Object *cpt = _objMan->fetchObject(SAND_25);
		Object *george = _objMan->fetchObject(PLAYER);
		if (george->o_place == HOLDING_REPLICA_25) // is george holding the replica in his hands?
			fnFullSetFrame(cpt, SAND_25, IMPFLRCDT, IMPFLR, 0, 0, 0, 0); // empty impression in floor
		else
			fnFullSetFrame(cpt, SAND_25, IMPPLSCDT, IMPPLS, 0, 0, 0, 0); // impression filled with plaster
	}

	// work around, at screen 69 in psx version TOP menu gets stuck at disabled, fix it at next screen (71)
	if ((screen == 71) && (SwordEngine::isPsx()))
		_scriptVars[TOP_MENU_DISABLED] = 0;

	if (SwordEngine::_systemVars.justRestoredGame) { // if we've just restored a game - we want George to be exactly as saved
		fnAddHuman(NULL, 0, 0, 0, 0, 0, 0, 0);
		if (_scriptVars[GEORGE_WALKING]) { // except that if George was walking when we saveed the game
			fnStandAt(compact, PLAYER, _scriptVars[CHANGE_X], _scriptVars[CHANGE_Y], _scriptVars[CHANGE_DIR], _scriptVars[CHANGE_STANCE], 0, 0);
			fnIdle(compact, PLAYER, 0, 0, 0, 0, 0, 0);
			_scriptVars[GEORGE_WALKING] = 0;
		}
		SwordEngine::_systemVars.justRestoredGame = 0;
		_music->startMusic(_scriptVars[CURRENT_MUSIC], 1);
	} else { // if we haven't just restored a game, set George to stand, etc
		compact->o_screen = _scriptVars[NEW_SCREEN]; //move the mega/player at this point between screens
		fnStandAt(compact, PLAYER, _scriptVars[CHANGE_X], _scriptVars[CHANGE_Y], _scriptVars[CHANGE_DIR], _scriptVars[CHANGE_STANCE], 0, 0);
		fnChangeFloor(compact, PLAYER, _scriptVars[CHANGE_PLACE], 0, 0, 0, 0, 0);
	}
}

void Logic::engine() {
	debug(8, "\n\nNext logic cycle");
	_eventMan->serviceGlobalEventList();

	for (uint16 sectCnt = 0; sectCnt < TOTAL_SECTIONS; sectCnt++) {
		if (_objMan->sectionAlive(sectCnt)) {
			uint32 numCpts = _objMan->fetchNoObjects(sectCnt);
			for (uint32 cptCnt = 0; cptCnt < numCpts; cptCnt++) {
				uint32 currentId = sectCnt * ITM_PER_SEC + cptCnt;
				Object *compact = _objMan->fetchObject(currentId);

				if (compact->o_status & STAT_LOGIC) { // does the object want to be processed?
					if (compact->o_status & STAT_EVENTS) {
						//subscribed to the global-event-switcher? and in logic mode
						switch (compact->o_logic) {
						case LOGIC_pause_for_event:
						case LOGIC_idle:
						case LOGIC_AR_animate:
							_eventMan->checkForEvent(compact);
							break;
						}
					}
					debug(7, "Logic::engine: handling compact %d (%X)", currentId, currentId);
					processLogic(compact, currentId);
					compact->o_sync = 0; // syncs are only available for 1 cycle.
				}

				if ((uint32)compact->o_screen == _scriptVars[SCREEN]) {
					if (compact->o_status & STAT_FORE)
						_screen->addToGraphicList(0, currentId);
					if (compact->o_status & STAT_SORT)
						_screen->addToGraphicList(1, currentId);
					if (compact->o_status & STAT_BACK)
						_screen->addToGraphicList(2, currentId);

					if (compact->o_status & STAT_MOUSE)
						_mouse->addToList(currentId, compact);
				}
			}
		}
	}
	//_collision->checkCollisions();

}

void Logic::processLogic(Object *compact, uint32 id) {
	int logicRet;
	do {
		switch (compact->o_logic) {
		case LOGIC_idle:
			logicRet = 0;
			break;
		case LOGIC_pause:
		case LOGIC_pause_for_event:
			if (compact->o_pause) {
				compact->o_pause--;
				logicRet = 0;
			} else {
				compact->o_logic = LOGIC_script;
				logicRet = 1;
			}
			break;
		case LOGIC_quit:
			compact->o_logic = LOGIC_script;
			logicRet = 0;
			break;
		case LOGIC_wait_for_sync:
			if (compact->o_sync) {
				logicRet = 1;
				compact->o_logic = LOGIC_script;
			} else
				logicRet = 0;
			break;
		case LOGIC_choose:
			_scriptVars[CUR_ID] = id;
			logicRet = _menu->logicChooser(compact);
			break;
		case LOGIC_wait_for_talk:
			logicRet = logicWaitTalk(compact);
			break;
		case LOGIC_start_talk:
			logicRet = logicStartTalk(compact);
			break;
		case LOGIC_script:
			_scriptVars[CUR_ID] = id;
			logicRet = scriptManager(compact, id);
			break;
		case LOGIC_new_script:
			compact->o_tree.o_script_pc[compact->o_tree.o_script_level] = _newScript;
			compact->o_tree.o_script_id[compact->o_tree.o_script_level] = _newScript;
			compact->o_logic = LOGIC_script;
			logicRet = 1;
			break;
		case LOGIC_AR_animate:
			logicRet = logicArAnimate(compact, id);
			break;
		case LOGIC_restart:
			compact->o_tree.o_script_pc[compact->o_tree.o_script_level] = compact->o_tree.o_script_id[compact->o_tree.o_script_level];
			compact->o_logic = LOGIC_script;
			logicRet = 1;
			break;
		case LOGIC_bookmark:
			memcpy(&(compact->o_tree.o_script_level), &(compact->o_bookmark.o_script_level), sizeof(ScriptTree));
			if (id == GMASTER_79) {
				// workaround for ending script.
				// GMASTER_79 is not prepared for mega_interact receiving INS_quit
				fnSuicide(compact, id, 0, 0, 0, 0, 0, 0);
				logicRet = 0;
			} else {
				compact->o_logic = LOGIC_script;
				logicRet = 1;
			}
			break;
		case LOGIC_speech:
			logicRet = speechDriver(compact);
			break;
		case LOGIC_full_anim:
			logicRet = fullAnimDriver(compact);
			break;
		case LOGIC_anim:
			logicRet = animDriver(compact);
			break;

		default:
			error("Fatal error: compact %d's logic == %X", id, compact->o_logic);
			break;
		}
	} while (logicRet);
}

int Logic::logicWaitTalk(Object *compact) {
	Object *target = _objMan->fetchObject(compact->o_down_flag);

	if (target->o_status & STAT_TALK_WAIT) {
		compact->o_logic = LOGIC_script;
		return 1;
	} else {
		return 0;
	}
}

int Logic::logicStartTalk(Object *compact) {
	Object *target = _objMan->fetchObject(compact->o_down_flag); //holds id of person we're waiting for
	if (target->o_status & STAT_TALK_WAIT) { //response?
		compact->o_logic = LOGIC_script; //back to script again
		return SCRIPT_CONT;
	}
	if (_eventMan->eventValid(compact->o_down_flag))
		return SCRIPT_STOP; //event still valid - keep waiting
	//the event has gone - so back to script with error code
	compact->o_down_flag = 0;
	compact->o_logic = LOGIC_script;
	return SCRIPT_CONT;
}

int Logic::logicArAnimate(Object *compact, uint32 id) {
	WalkData *route;
	int32 walkPc;
	if ((_scriptVars[GEORGE_WALKING] == 0) && (id == PLAYER))
		_scriptVars[GEORGE_WALKING] = 1;

	compact->o_resource = compact->o_walk_resource;
	compact->o_status |= STAT_SHRINK;
	route = compact->o_route;

	walkPc            = compact->o_walk_pc;
	compact->o_frame  = route[walkPc].frame;
	compact->o_dir    = route[walkPc].dir;
	compact->o_xcoord = route[walkPc].x;
	compact->o_ycoord = route[walkPc].y;
	compact->o_anim_x = compact->o_xcoord;
	compact->o_anim_y = compact->o_ycoord;

	if (((_scriptVars[GEORGE_WALKING] == 2) && (walkPc > 5) && (id == PLAYER) &&
	        (route[walkPc - 1].step == 5) && (route[walkPc].step == 0)) ||
	        ((_scriptVars[GEORGE_WALKING] == 3) && (id == PLAYER))) {

		compact->o_frame = 96 + compact->o_dir;                     //reset
		if ((compact->o_dir != 2) && (compact->o_dir != 6)) {  // on verticals and diagonals stand where george is
			compact->o_xcoord = route[walkPc - 1].x;
			compact->o_ycoord = route[walkPc - 1].y;
			compact->o_anim_x = compact->o_xcoord;
			compact->o_anim_y = compact->o_ycoord;
		}
		compact->o_logic = LOGIC_script;
		compact->o_down_flag = 0;       //0 means error
		_scriptVars[GEORGE_WALKING] = 0;
		route[compact->o_walk_pc + 1].frame = 512;                  //end of sequence
		if (_scriptVars[MEGA_ON_GRID] == 2)
			_scriptVars[MEGA_ON_GRID] = 0;
	}
	compact->o_walk_pc++;

	if (route[compact->o_walk_pc].frame == 512) {                   //end of sequence
		compact->o_logic = LOGIC_script;
		if (((_scriptVars[GEORGE_WALKING] == 2) || (_scriptVars[GEORGE_WALKING] == 1)) &&
		        (id == PLAYER)) {
			_scriptVars[GEORGE_WALKING] = 0;
			if (_scriptVars[MEGA_ON_GRID] == 2)
				_scriptVars[MEGA_ON_GRID] = 0;
		}
	}
	return 0;
}

int Logic::speechDriver(Object *compact) {
	if ((!_speechClickDelay) && (_mouse->testEvent() & BS1L_BUTTON_DOWN))
		_speechFinished = true;
	if (_speechClickDelay)
		_speechClickDelay--;

	if (_speechRunning) {
		if (_sound->speechFinished())
			_speechFinished = true;
	} else {
		if (!compact->o_speech_time)
			_speechFinished = true;
		else
			compact->o_speech_time--;
	}
	if (_speechFinished) {
		if (_speechRunning)
			_sound->stopSpeech();
		compact->o_logic = LOGIC_script;
		if (_textRunning) {
			_textMan->releaseText(compact->o_text_id);
			_objMan->fetchObject(compact->o_text_id)->o_status = 0; // kill compact linking text sprite
		}
		_speechRunning = _textRunning = false;
		_speechFinished = true;
	}
	if (compact->o_anim_resource) {
		uint8 *animData = ((uint8 *)_resMan->openFetchRes(compact->o_anim_resource)) + sizeof(Header);
		int32 numFrames = _resMan->readUint32(animData);
		animData += 4;
		compact->o_anim_pc++; // go to next frame of anim

		if (_speechFinished || (compact->o_anim_pc >= numFrames) ||
		        (_speechRunning && (_sound->amISpeaking() == 0)))
			compact->o_anim_pc = 0; //set to frame 0, closed mouth

		AnimUnit *animPtr = (AnimUnit *)(animData + sizeof(AnimUnit) * compact->o_anim_pc);
		if (!(compact->o_status & STAT_SHRINK)) {
			compact->o_anim_x = _resMan->getUint32(animPtr->animX);
			compact->o_anim_y = _resMan->getUint32(animPtr->animY);
		}
		compact->o_frame = _resMan->getUint32(animPtr->animFrame);
		_resMan->resClose(compact->o_anim_resource);
	}
	return 0;
}

int Logic::fullAnimDriver(Object *compact) {
	if (compact->o_sync) { // return to script immediately if we've received a sync
		compact->o_logic = LOGIC_script;
		return 1;
	}
	uint8 *data = ((uint8 *)_resMan->openFetchRes(compact->o_anim_resource)) + sizeof(Header);
	uint32 numFrames = _resMan->readUint32(data);
	data += 4;
	AnimUnit *animPtr = (AnimUnit *)(data + compact->o_anim_pc * sizeof(AnimUnit));

	compact->o_anim_x = compact->o_xcoord = _resMan->getUint32(animPtr->animX);
	compact->o_anim_y = compact->o_ycoord = _resMan->getUint32(animPtr->animY);
	compact->o_frame = _resMan->getUint32(animPtr->animFrame);

	compact->o_anim_pc++;
	if (compact->o_anim_pc == (int)numFrames)
		compact->o_logic = LOGIC_script;

	_resMan->resClose(compact->o_anim_resource);
	return 0;
}

int Logic::animDriver(Object *compact) {
	if (compact->o_sync) {
		compact->o_logic = LOGIC_script;
		return 1;
	}
	uint8 *data = ((uint8 *)_resMan->openFetchRes(compact->o_anim_resource)) + sizeof(Header);
	uint32 numFrames = _resMan->readUint32(data);
	AnimUnit *animPtr = (AnimUnit *)(data + 4 + compact->o_anim_pc * sizeof(AnimUnit));

	if (!(compact->o_status & STAT_SHRINK)) {
		compact->o_anim_x = _resMan->getUint32(animPtr->animX);
		compact->o_anim_y = _resMan->getUint32(animPtr->animY);
	}

	compact->o_frame = _resMan->getUint32(animPtr->animFrame);
	compact->o_anim_pc++;
	if (compact->o_anim_pc == (int)numFrames)
		compact->o_logic = LOGIC_script;

	_resMan->resClose(compact->o_anim_resource);
	return 0;
}

void Logic::updateScreenParams() {
	Object *compact = (Object *)_objMan->fetchObject(PLAYER);
	_screen->setScrolling((int16)(compact->o_xcoord - _scriptVars[FEET_X]),
	                      (int16)(compact->o_ycoord - _scriptVars[FEET_Y]));
}

int Logic::scriptManager(Object *compact, uint32 id) {
	int ret;
	do {
		uint32 level = compact->o_tree.o_script_level;
		uint32 script = compact->o_tree.o_script_id[level];
		Debug::interpretScript(id, level, script, compact->o_tree.o_script_pc[level] & ITM_ID);
		ret = interpretScript(compact, id, _resMan->lockScript(script), script, compact->o_tree.o_script_pc[level] & ITM_ID);
		_resMan->unlockScript(script);
		if (!ret) {
			if (compact->o_tree.o_script_level)
				compact->o_tree.o_script_level--;
			else
				error("ScriptManager: basescript %d for cpt %d ended", script, id);
		} else
			compact->o_tree.o_script_pc[level] = ret;
	} while (!ret);
	return 1;
	//Logic continues - but the script must have changed logic mode
	//this is a radical change from S2.0 where once a script finished there
	//was no more processing for that object on that cycle - the Logic_engine terminated.
	//This meant that new logics that needed immediate action got a first call from the
	//setup function. This was a bit tweeky. This technique ensures that the script is a
	//totally seamless concept that takes up zero cycle time. The only downside is that
	//an FN_quit becomes slightly more convoluted, but so what you might ask.
}

void Logic::runMouseScript(Object *cpt, int32 scriptId) {
	Header *script = _resMan->lockScript(scriptId);
	debug(9, "running mouse script %d", scriptId);
	interpretScript(cpt, _scriptVars[SPECIAL_ITEM], script, scriptId, scriptId);
	_resMan->unlockScript(scriptId);
}

int Logic::interpretScript(Object *compact, int id, Header *scriptModule, int scriptBase, int scriptNum) {
	int32 *scriptCode = (int32 *)(((uint8 *)scriptModule) + sizeof(Header));
	int32 stack[MAX_STACK_SIZE];
	int32 stackIdx = 0;
	int32 offset;
	int32 pc;
	if (memcmp(scriptModule->type, "Script", 6))
		error("Invalid script module");
	if (scriptModule->version != SCRIPT_VERSION)
		error("Illegal script version");
	if (scriptNum < 0)
		error("negative script number");
	if ((uint32)scriptNum >= scriptModule->decomp_length)
		error("Script number out of bounds");

	if (scriptNum < scriptCode[0])
		pc = scriptCode[scriptNum + 1];
	else
		pc = scriptNum;
	int32 startOfScript = scriptCode[(scriptBase & ITM_ID) + 1];

	int32 a, b, c, d, e, f;
	int mCodeReturn = 0;
	int32 mCodeNumber = 0, mCodeArguments = 0;
	uint32 varNum = 0;
	while (1) {
		assert((stackIdx >= 0) && (stackIdx <= MAX_STACK_SIZE));
		switch (scriptCode[pc++]) {
		case IT_MCODE:
			a = b = c = d = e = f = 0;
			mCodeNumber = scriptCode[pc++];
			mCodeArguments = scriptCode[pc++];
			switch (mCodeArguments) {
			case 6: f = stack[--stackIdx];
			case 5: e = stack[--stackIdx];
			case 4: d = stack[--stackIdx];
			case 3: c = stack[--stackIdx];
			case 2: b = stack[--stackIdx];
			case 1: a = stack[--stackIdx];
			case 0:
				Debug::callMCode(mCodeNumber, mCodeArguments, a, b, c, d, e, f);
				mCodeReturn = (this->*_mcodeTable[mCodeNumber])(compact, id, a, b, c, d, e, f);
				break;
			default:
				warning("mcode[%d]: too many arguments(%d)", mCodeNumber, mCodeArguments);
			}
			if (mCodeReturn == 0)
				return pc;
			break;
		case IT_PUSHNUMBER:
			debug(9, "IT_PUSH: %d", scriptCode[pc]);
			stack[stackIdx++] = scriptCode[pc++];
			break;
		case IT_PUSHVARIABLE:
			debug(9, "IT_PUSHVARIABLE: ScriptVar[%d] => %d", scriptCode[pc], _scriptVars[scriptCode[pc]]);
			varNum = scriptCode[pc++];
			if (SwordEngine::_systemVars.isDemo && SwordEngine::isWindows()) {
				if (varNum >= 397) // BS1 Demo has different number of script variables
					varNum++;
				if (varNum >= 699)
					varNum++;
			}
			stack[stackIdx++] = _scriptVars[varNum];
			break;
		case IT_NOTEQUAL:
			stackIdx--;
			debug(9, "IT_NOTEQUAL: RESULT = %d", stack[stackIdx - 1] != stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] != stack[stackIdx]);
			break;
		case IT_ISEQUAL:
			stackIdx--;
			debug(9, "IT_ISEQUAL: RESULT = %d", stack[stackIdx - 1] == stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] == stack[stackIdx]);
			break;
		case IT_PLUS:
			stackIdx--;
			debug(9, "IT_PLUS: RESULT = %d", stack[stackIdx - 1] + stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] + stack[stackIdx]);
			break;
		case IT_TIMES:
			stackIdx--;
			debug(9, "IT_TIMES: RESULT = %d", stack[stackIdx - 1] * stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] * stack[stackIdx]);
			break;
		case IT_ANDAND:
			stackIdx--;
			debug(9, "IT_ANDAND: RESULT = %d", stack[stackIdx - 1] && stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] && stack[stackIdx]);
			break;
		case IT_OROR:           // ||
			stackIdx--;
			debug(9, "IT_OROR: RESULT = %d", stack[stackIdx - 1] || stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] || stack[stackIdx]);
			break;
		case IT_LESSTHAN:
			stackIdx--;
			debug(9, "IT_LESSTHAN: RESULT = %d", stack[stackIdx - 1] < stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] < stack[stackIdx]);
			break;
		case IT_NOT:
			debug(9, "IT_NOT: RESULT = %d", stack[stackIdx - 1] ? 0 : 1);
			if (stack[stackIdx - 1])
				stack[stackIdx - 1] = 0;
			else
				stack[stackIdx - 1] = 1;
			break;
		case IT_MINUS:
			stackIdx--;
			debug(9, "IT_MINUS: RESULT = %d", stack[stackIdx - 1] - stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] - stack[stackIdx]);
			break;
		case IT_AND:
			stackIdx--;
			debug(9, "IT_AND: RESULT = %d", stack[stackIdx - 1] & stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] & stack[stackIdx]);
			break;
		case IT_OR:
			stackIdx--;
			debug(9, "IT_OR: RESULT = %d", stack[stackIdx - 1] | stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] | stack[stackIdx]);
			break;
		case IT_GTE:
			stackIdx--;
			debug(9, "IT_GTE: RESULT = %d", stack[stackIdx - 1] >= stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] >= stack[stackIdx]);
			break;
		case IT_LTE:
			stackIdx--;
			debug(9, "IT_LTE: RESULT = %d", stack[stackIdx - 1] <= stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] <= stack[stackIdx]);
			break;
		case IT_DEVIDE:
			stackIdx--;
			debug(9, "IT_DEVIDE: RESULT = %d", stack[stackIdx - 1] / stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] / stack[stackIdx]);
			break;
		case IT_GT:
			stackIdx--;
			debug(9, "IT_GT: RESULT = %d", stack[stackIdx - 1] > stack[stackIdx]);
			stack[stackIdx - 1] = (stack[stackIdx - 1] > stack[stackIdx]);
			break;
		case IT_SCRIPTEND:
			debug(9, "IT_SCRIPTEND");
			return 0;
		case IT_POPVAR:         // pop a variable
			debug(9, "IT_POPVAR: ScriptVars[%d] = %d", scriptCode[pc], stack[stackIdx - 1]);
			varNum = scriptCode[pc++];
			if (SwordEngine::_systemVars.isDemo && SwordEngine::isWindows()) {
				if (varNum >= 397) // BS1 Demo has different number of script variables
					varNum++;
				if (varNum >= 699)
					varNum++;
			}
			_scriptVars[varNum] = stack[--stackIdx];
			break;
		case IT_POPLONGOFFSET:
			offset = scriptCode[pc++];
			debug(9, "IT_POPLONGOFFSET: Cpt[%d] = %d", offset, stack[stackIdx - 1]);
			*((int32 *)((uint8 *)compact + offset)) = stack[--stackIdx];
			break;
		case IT_PUSHLONGOFFSET:
			offset = scriptCode[pc++];
			debug(9, "IT_PUSHLONGOFFSET: PUSH Cpt[%d] (==%d)", offset, *((int32 *)((uint8 *)compact + offset)));
			stack[stackIdx++] = *((int32 *)((uint8 *)compact + offset));
			break;
		case IT_SKIPONFALSE:
			debug(9, "IT_SKIPONFALSE: %d (%s)", scriptCode[pc], (stack[stackIdx - 1] ? "IS TRUE (NOT SKIPPED)" : "IS FALSE (SKIPPED)"));
			if (stack[--stackIdx])
				pc++;
			else
				pc += scriptCode[pc];
			break;
		case IT_SKIP:
			debug(9, "IT_SKIP: %d", scriptCode[pc]);
			pc += scriptCode[pc];
			break;
		case IT_SWITCH:         // The mega switch statement
			debug(9, "IT_SWITCH: [SORRY, NO DEBUG INFO]");
			{
				int switchValue = stack[--stackIdx];
				int switchCount = scriptCode[pc++];
				int doneSwitch = 0;

				for (int cnt = 0; (cnt < switchCount) && (doneSwitch == 0); cnt++) {
					if (switchValue == scriptCode[pc]) {
						pc += scriptCode[pc + 1];
						doneSwitch = 1;
					} else
						pc += 2;
				}
				if (doneSwitch == 0)
					pc += scriptCode[pc];
			}
			break;
		case IT_SKIPONTRUE:     // skip if expression true
			debug(9, "IT_SKIPONTRUE: %d (%s)", scriptCode[pc], (stack[stackIdx - 1] ? "IS TRUE (SKIPPED)" : "IS FALSE (NOT SKIPPED)"));
			stackIdx--;
			if (stack[stackIdx])
				pc += scriptCode[pc];
			else
				pc++;
			break;
		case IT_PRINTF:
			debug(0, "IT_PRINTF(%d)", stack[stackIdx]);
			break;
		case IT_RESTARTSCRIPT:
			debug(9, "IT_RESTARTSCRIPT");
			pc = startOfScript;
			break;
		case IT_POPWORDOFFSET:
			offset = scriptCode[pc++];
			debug(9, "IT_POPWORDOFFSET: Cpt[%d] = %d", offset, stack[stackIdx - 1] & 0xFFFF);
			*((int32 *)((uint8 *)compact + offset)) = stack[--stackIdx] & 0xffff;
			break;
		case IT_PUSHWORDOFFSET:
			offset = scriptCode[pc++];
			debug(9, "IT_PUSHWORDOFFSET: PUSH Cpt[%d] == %d", offset, (*((int32 *)((uint8 *)compact + offset))) & 0xffff);
			stack[stackIdx++] = (*((int32 *)((uint8 *)compact + offset))) & 0xffff;
			break;
		default:
			error("Invalid operator %d", scriptCode[pc - 1]);
			return 0;   // for compilers that don't support NORETURN
		}
	}
}

void Logic::setupMcodeTable() {
	static const BSMcodeTable mcodeTable[100] = {
		&Logic::fnBackground,
		&Logic::fnForeground,
		&Logic::fnSort,
		&Logic::fnNoSprite,
		&Logic::fnMegaSet,
		&Logic::fnAnim,
		&Logic::fnSetFrame,
		&Logic::fnFullAnim,
		&Logic::fnFullSetFrame,
		&Logic::fnFadeDown,
		&Logic::fnFadeUp,
		&Logic::fnCheckFade,
		&Logic::fnSetSpritePalette,
		&Logic::fnSetWholePalette,
		&Logic::fnSetFadeTargetPalette,
		&Logic::fnSetPaletteToFade,
		&Logic::fnSetPaletteToCut,
		&Logic::fnPlaySequence,
		&Logic::fnIdle,
		&Logic::fnPause,
		&Logic::fnPauseSeconds,
		&Logic::fnQuit,
		&Logic::fnKillId,
		&Logic::fnSuicide,
		&Logic::fnNewScript,
		&Logic::fnSubScript,
		&Logic::fnRestartScript,
		&Logic::fnSetBookmark,
		&Logic::fnGotoBookmark,
		&Logic::fnSendSync,
		&Logic::fnWaitSync,
		&Logic::cfnClickInteract,
		&Logic::cfnSetScript,
		&Logic::cfnPresetScript,
		&Logic::fnInteract,
		&Logic::fnIssueEvent,
		&Logic::fnCheckForEvent,
		&Logic::fnWipeHands,
		&Logic::fnISpeak,
		&Logic::fnTheyDo,
		&Logic::fnTheyDoWeWait,
		&Logic::fnWeWait,
		&Logic::fnChangeSpeechText,
		&Logic::fnTalkError,
		&Logic::fnStartTalk,
		&Logic::fnCheckForTextLine,
		&Logic::fnAddTalkWaitStatusBit,
		&Logic::fnRemoveTalkWaitStatusBit,
		&Logic::fnNoHuman,
		&Logic::fnAddHuman,
		&Logic::fnBlankMouse,
		&Logic::fnNormalMouse,
		&Logic::fnLockMouse,
		&Logic::fnUnlockMouse,
		&Logic::fnSetMousePointer,
		&Logic::fnSetMouseLuggage,
		&Logic::fnMouseOn,
		&Logic::fnMouseOff,
		&Logic::fnChooser,
		&Logic::fnEndChooser,
		&Logic::fnStartMenu,
		&Logic::fnEndMenu,
		&Logic::cfnReleaseMenu,
		&Logic::fnAddSubject,
		&Logic::fnAddObject,
		&Logic::fnRemoveObject,
		&Logic::fnEnterSection,
		&Logic::fnLeaveSection,
		&Logic::fnChangeFloor,
		&Logic::fnWalk,
		&Logic::fnTurn,
		&Logic::fnStand,
		&Logic::fnStandAt,
		&Logic::fnFace,
		&Logic::fnFaceXy,
		&Logic::fnIsFacing,
		&Logic::fnGetTo,
		&Logic::fnGetToError,
		&Logic::fnGetPos,
		&Logic::fnGetGamepadXy,
		&Logic::fnPlayFx,
		&Logic::fnStopFx,
		&Logic::fnPlayMusic,
		&Logic::fnStopMusic,
		&Logic::fnInnerSpace,
		&Logic::fnRandom,
		&Logic::fnSetScreen,
		&Logic::fnPreload,
		&Logic::fnCheckCD,
		&Logic::fnRestartGame,
		&Logic::fnQuitGame,
		&Logic::fnDeathScreen,
		&Logic::fnSetParallax,
		&Logic::fnTdebug,
		&Logic::fnRedFlash,
		&Logic::fnBlueFlash,
		&Logic::fnYellow,
		&Logic::fnGreen,
		&Logic::fnPurple,
		&Logic::fnBlack
	};

	_mcodeTable = mcodeTable;
}

int Logic::fnBackground(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {

	cpt->o_status &= ~(STAT_FORE | STAT_SORT);
	cpt->o_status |= STAT_BACK;
	return SCRIPT_CONT;
}

int Logic::fnForeground(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status &= ~(STAT_BACK | STAT_SORT);
	cpt->o_status |= STAT_FORE;
	return SCRIPT_CONT;
}

int Logic::fnSort(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status &= ~(STAT_BACK | STAT_FORE);
	cpt->o_status |= STAT_SORT;
	return SCRIPT_CONT;
}

int Logic::fnNoSprite(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status &= ~(STAT_BACK | STAT_FORE | STAT_SORT);
	return SCRIPT_CONT;
}

int Logic::fnMegaSet(Object *cpt, int32 id, int32 walk_data, int32 spr, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_mega_resource = walk_data;
	cpt->o_walk_resource = spr;
	return SCRIPT_CONT;
}

int Logic::fnAnim(Object *cpt, int32 id, int32 cdt, int32 spr, int32 e, int32 f, int32 z, int32 x) {
	AnimSet *animTab;

	if (cdt && (!spr)) {
		animTab = (AnimSet *)((uint8 *)_resMan->openFetchRes(cdt) + sizeof(Header));
		animTab += cpt->o_dir;

		cpt->o_anim_resource = _resMan->getUint32(animTab->cdt);
		cpt->o_resource = _resMan->getUint32(animTab->spr);
		_resMan->resClose(cdt);
	} else {
		cpt->o_anim_resource = cdt;
		cpt->o_resource = spr;
	}
	if ((cpt->o_anim_resource == 0) || (cpt->o_resource == 0))
		error("fnAnim called width (%d/%d) => (%d/%d)", cdt, spr, cpt->o_anim_resource, cpt->o_resource);

	FrameHeader *frameHead = _resMan->fetchFrame(_resMan->openFetchRes(cpt->o_resource), 0);
	if (frameHead->offsetX || frameHead->offsetY) { // boxed mega anim?
		cpt->o_status |= STAT_SHRINK;
		cpt->o_anim_x = cpt->o_xcoord; // set anim coords to 'feet' coords - only need to do this once
		cpt->o_anim_y = cpt->o_ycoord;
	} else {
		// Anim_driver sets anim coords to cdt coords for every frame of a loose anim
		cpt->o_status &= ~STAT_SHRINK;
	}
	_resMan->resClose(cpt->o_resource);

	cpt->o_logic = LOGIC_anim;
	cpt->o_anim_pc = 0;
	cpt->o_sync = 0;
	return SCRIPT_STOP;
}

int Logic::fnSetFrame(Object *cpt, int32 id, int32 cdt, int32 spr, int32 frameNo, int32 f, int32 z, int32 x) {

	AnimUnit   *animPtr;

	uint8 *data = (uint8 *)_resMan->openFetchRes(cdt);
	data += sizeof(Header);
	if (frameNo == LAST_FRAME)
		frameNo = _resMan->readUint32(data) - 1;

	data += 4;
	animPtr = (AnimUnit *)(data + frameNo * sizeof(AnimUnit));

	cpt->o_anim_x = _resMan->getUint32(animPtr->animX);
	cpt->o_anim_y = _resMan->getUint32(animPtr->animY);
	cpt->o_frame = _resMan->getUint32(animPtr->animFrame);

	cpt->o_resource = spr;
	cpt->o_status &= ~STAT_SHRINK;
	_resMan->resClose(cdt);
	return SCRIPT_CONT;
}

int Logic::fnFullAnim(Object *cpt, int32 id, int32 anim, int32 graphic, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_full_anim;

	cpt->o_anim_pc = 0;
	cpt->o_anim_resource = anim;
	cpt->o_resource = graphic;
	cpt->o_status &= ~STAT_SHRINK;
	cpt->o_sync = 0;
	return SCRIPT_STOP;
}

int Logic::fnFullSetFrame(Object *cpt, int32 id, int32 cdt, int32 spr, int32 frameNo, int32 f, int32 z, int32 x) {
	uint8 *data = (uint8 *)_resMan->openFetchRes(cdt) + sizeof(Header);

	if (frameNo == LAST_FRAME)
		frameNo = _resMan->readUint32(data) - 1;
	data += 4;

	AnimUnit *animPtr = (AnimUnit *)(data + sizeof(AnimUnit) * frameNo);
	cpt->o_anim_x = cpt->o_xcoord = _resMan->getUint32(animPtr->animX);
	cpt->o_anim_y = cpt->o_ycoord = _resMan->getUint32(animPtr->animY);
	cpt->o_frame = _resMan->getUint32(animPtr->animFrame);

	cpt->o_resource = spr;
	cpt->o_status &= ~STAT_SHRINK;

	_resMan->resClose(cdt);
	return SCRIPT_CONT;
}

int Logic::fnFadeDown(Object *cpt, int32 id, int32 speed, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_screen->fadeDownPalette();
	return SCRIPT_CONT;
}

int Logic::fnFadeUp(Object *cpt, int32 id, int32 speed, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_screen->fadeUpPalette();
	return SCRIPT_CONT;
}

int Logic::fnCheckFade(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_scriptVars[RETURN_VALUE] = (uint8)_screen->stillFading();
	return SCRIPT_CONT;
}

int Logic::fnSetSpritePalette(Object *cpt, int32 id, int32 spritePal, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_screen->fnSetPalette(184, 72, spritePal, false);
	return SCRIPT_CONT;
}

int Logic::fnSetWholePalette(Object *cpt, int32 id, int32 spritePal, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_screen->fnSetPalette(0, 256, spritePal, false);
	return SCRIPT_CONT;
}

int Logic::fnSetFadeTargetPalette(Object *cpt, int32 id, int32 spritePal, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_screen->fnSetPalette(0, 184, spritePal, true);
	return SCRIPT_CONT;
}

int Logic::fnSetPaletteToFade(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	SwordEngine::_systemVars.wantFade = true;
	return SCRIPT_CONT;
}

int Logic::fnSetPaletteToCut(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	SwordEngine::_systemVars.wantFade = false;
	return SCRIPT_CONT;
}

int Logic::fnPlaySequence(Object *cpt, int32 id, int32 sequenceId, int32 d, int32 e, int32 f, int32 z, int32 x) {

	// A cutscene usually (always?) means the room will change. In the
	// meantime, we don't want any looping sound effects still playing.
	_sound->quitScreen();

	MoviePlayer *player = makeMoviePlayer(sequenceId, _vm, _textMan, _resMan, _system);
	if (player) {
		_screen->clearScreen();
		if (player->load(sequenceId))
			player->play();
		delete player;
	}
	return SCRIPT_CONT;
}

int Logic::fnIdle(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_tree.o_script_level = 0; // force to level 0
	cpt->o_logic = LOGIC_idle;
	return SCRIPT_STOP;
}

int Logic::fnPause(Object *cpt, int32 id, int32 pause, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_pause = pause;
	cpt->o_logic = LOGIC_pause;
	return SCRIPT_STOP;
}

int Logic::fnPauseSeconds(Object *cpt, int32 id, int32 pause, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_pause = pause * FRAME_RATE;
	cpt->o_logic = LOGIC_pause;
	return SCRIPT_STOP;
}

int Logic::fnQuit(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_quit;
	return SCRIPT_STOP;
}

int Logic::fnKillId(Object *cpt, int32 id, int32 target, int32 d, int32 e, int32 f, int32 z, int32 x) {
	Object *targetObj = _objMan->fetchObject(target);
	targetObj->o_status = 0;
	return SCRIPT_CONT;
}

int Logic::fnSuicide(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status = 0;
	cpt->o_logic = LOGIC_quit;
	return SCRIPT_STOP;
}

int Logic::fnNewScript(Object *cpt, int32 id, int32 script, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_new_script;
	_newScript = script;
	return SCRIPT_STOP;
}

int Logic::fnSubScript(Object *cpt, int32 id, int32 script, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_tree.o_script_level++;
	if (cpt->o_tree.o_script_level == TOTAL_script_levels)
		error("Compact %d: script level exceeded in fnSubScript", id);
	cpt->o_tree.o_script_pc[cpt->o_tree.o_script_level] = script;
	cpt->o_tree.o_script_id[cpt->o_tree.o_script_level] = script;
	return SCRIPT_STOP;
}

int Logic::fnRestartScript(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_restart;
	return SCRIPT_STOP;
}

int Logic::fnSetBookmark(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	memcpy(&cpt->o_bookmark.o_script_level, &cpt->o_tree.o_script_level, sizeof(ScriptTree));
	return SCRIPT_CONT;
}

int Logic::fnGotoBookmark(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_bookmark;
	return SCRIPT_STOP;
}

int Logic::fnSendSync(Object *cpt, int32 id, int32 sendId, int32 syncValue, int32 e, int32 f, int32 z, int32 x) {
	Object *target = _objMan->fetchObject(sendId);
	target->o_sync = syncValue;
	return SCRIPT_CONT;
}

int Logic::fnWaitSync(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_logic = LOGIC_wait_for_sync;
	return SCRIPT_STOP;
}

int Logic::cfnClickInteract(Object *cpt, int32 id, int32 target, int32 d, int32 e, int32 f, int32 z, int32 x) {
	Object *tar = _objMan->fetchObject(target);
	cpt = _objMan->fetchObject(PLAYER);
	cpt->o_tree.o_script_level = 0;
	cpt->o_tree.o_script_pc[0] = tar->o_interact;
	cpt->o_tree.o_script_id[0] = tar->o_interact;
	cpt->o_logic = LOGIC_script;
	return SCRIPT_STOP;
}

int Logic::cfnSetScript(Object *cpt, int32 id, int32 target, int32 script, int32 e, int32 f, int32 z, int32 x) {
	Object *tar = _objMan->fetchObject(target);
	tar->o_tree.o_script_level = 0;
	tar->o_tree.o_script_pc[0] = script;
	tar->o_tree.o_script_id[0] = script;
	tar->o_logic = LOGIC_script;
	return SCRIPT_CONT;
}

int Logic::cfnPresetScript(Object *cpt, int32 id, int32 target, int32 script, int32 e, int32 f, int32 z, int32 x) {
	Object *tar = _objMan->fetchObject(target);
	tar->o_tree.o_script_level = 0;
	tar->o_tree.o_script_pc[0] = script;
	tar->o_tree.o_script_id[0] = script;
	if (tar->o_logic == LOGIC_idle)
		tar->o_logic = LOGIC_script;
	return SCRIPT_CONT;
}

int Logic::fnInteract(Object *cpt, int32 id, int32 target, int32 d, int32 e, int32 f, int32 z, int32 x) {
	Object *tar = _objMan->fetchObject(target);
	cpt->o_place = tar->o_place;

	Object *floorObject = _objMan->fetchObject(tar->o_place);
	cpt->o_scale_a = floorObject->o_scale_a;
	cpt->o_scale_b = floorObject->o_scale_b;

	cpt->o_tree.o_script_level++;
	cpt->o_tree.o_script_pc[cpt->o_tree.o_script_level] = tar->o_interact;
	cpt->o_tree.o_script_id[cpt->o_tree.o_script_level] = tar->o_interact;

	return SCRIPT_STOP;
}

int Logic::fnIssueEvent(Object *cpt, int32 id, int32 event, int32 delay, int32 e, int32 f, int32 z, int32 x) {
	_eventMan->fnIssueEvent(cpt, id, event, delay);
	return SCRIPT_CONT;
}

int Logic::fnCheckForEvent(Object *cpt, int32 id, int32 pause, int32 d, int32 e, int32 f, int32 z, int32 x) {
	return _eventMan->fnCheckForEvent(cpt, id, pause);
}

int Logic::fnWipeHands(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_scriptVars[OBJECT_HELD] = 0;
	_mouse->setLuggage(0, 0);
	_menu->refresh(MENU_TOP);
	return SCRIPT_CONT;
}

int Logic::fnISpeak(Object *cpt, int32 id, int32 cdt, int32 textNo, int32 spr, int32 f, int32 z, int32 x) {
	_speechClickDelay = 3;
	if (((textNo & ~1) == 0x3f0012) && (!cdt) && (!spr)) {
		cdt = GEOSTDLCDT; // workaround for missing animation when examining
		spr = GEOSTDL;    // the conductor on the train roof
	}
	cpt->o_logic = LOGIC_speech;

	// first setup the talk animation
	if (cdt && (!spr)) { // if 'cdt' is non-zero but 'spr' is zero - 'cdt' is an anim table tag
		AnimSet *animTab = (AnimSet *)((uint8 *)_resMan->openFetchRes(cdt) + sizeof(Header));
		animTab += cpt->o_dir;

		cpt->o_anim_resource = _resMan->getUint32(animTab->cdt);
		if (animTab->cdt)
			cpt->o_resource = _resMan->getUint32(animTab->spr);
		_resMan->resClose(cdt);
	} else {
		cpt->o_anim_resource = cdt;
		if (cdt)
			cpt->o_resource = spr;
	}
	cpt->o_anim_pc = 0; // start anim from first frame
	if (cpt->o_anim_resource) {
		if (!cpt->o_resource)
			error("ID %d: Can't run anim with cdt=%d, spr=%d", id, cdt, spr);

		FrameHeader *frameHead = _resMan->fetchFrame(_resMan->openFetchRes(cpt->o_resource), 0);
		if (frameHead->offsetX && frameHead->offsetY) { // is this a boxed mega?
			cpt->o_status |= STAT_SHRINK;
			cpt->o_anim_x = cpt->o_xcoord;
			cpt->o_anim_y = cpt->o_ycoord;
		} else
			cpt->o_status &= ~STAT_SHRINK;

		_resMan->resClose(cpt->o_resource);
	}
	if (SwordEngine::_systemVars.playSpeech)
		_speechRunning = _sound->startSpeech(textNo >> 16, textNo & 0xFFFF);
	else
		_speechRunning = false;
	_speechFinished = false;
	if (SwordEngine::_systemVars.showText || (!_speechRunning)) {
		_textRunning = true;

		char *text = _objMan->lockText(textNo);
		cpt->o_speech_time = strlen(text) + 5;
		uint32 textCptId = _textMan->lowTextManager((uint8 *)text, cpt->o_speech_width, (uint8)cpt->o_speech_pen);
		_objMan->unlockText(textNo);

		Object *textCpt = _objMan->fetchObject(textCptId);
		textCpt->o_screen = cpt->o_screen;
		textCpt->o_target = textCptId;

		// the graphic is a property of Text, so we don't lock/unlock it.
		uint16 textSpriteWidth  = _resMan->getUint16(_textMan->giveSpriteData(textCpt->o_target)->width);
		uint16 textSpriteHeight = _resMan->getUint16(_textMan->giveSpriteData(textCpt->o_target)->height);

		cpt->o_text_id = textCptId;

		// now set text coords, above the player, usually

#define TEXT_MARGIN 3 // distance kept from edges of screen
#define ABOVE_HEAD 20 // distance kept above talking sprite
		uint16 textX, textY;
		if (((id == GEORGE) || ((id == NICO) && (_scriptVars[SCREEN] == 10))) && (!cpt->o_anim_resource)) {
			// if George is doing Voice-Over text (centered at the bottom of the screen)
			textX = _scriptVars[SCROLL_OFFSET_X] + 128 + (640 / 2) - textSpriteWidth / 2;
			textY = _scriptVars[SCROLL_OFFSET_Y] + 128 + 400;
		} else {
			if ((id == GEORGE) && (_scriptVars[SCREEN] == 79))
				textX = cpt->o_mouse_x2; // move it off george's head
			else
				textX = (cpt->o_mouse_x1 + cpt->o_mouse_x2) / 2 - textSpriteWidth / 2;

			textY = cpt->o_mouse_y1 - textSpriteHeight - ABOVE_HEAD;
		}
		// now ensure text is within visible screen
		uint16 textLeftMargin, textRightMargin, textTopMargin, textBottomMargin;
		textLeftMargin   = SCREEN_LEFT_EDGE   + TEXT_MARGIN + _scriptVars[SCROLL_OFFSET_X];
		textRightMargin  = SCREEN_RIGHT_EDGE  - TEXT_MARGIN + _scriptVars[SCROLL_OFFSET_X] - textSpriteWidth;
		textTopMargin    = SCREEN_TOP_EDGE    + TEXT_MARGIN + _scriptVars[SCROLL_OFFSET_Y];
		textBottomMargin = SCREEN_BOTTOM_EDGE - TEXT_MARGIN + _scriptVars[SCROLL_OFFSET_Y] - textSpriteHeight;

		textCpt->o_anim_x = textCpt->o_xcoord = CLIP<uint16>(textX, textLeftMargin, textRightMargin);
		textCpt->o_anim_y = textCpt->o_ycoord = CLIP<uint16>(textY, textTopMargin, textBottomMargin);
	}
	return SCRIPT_STOP;
}

//send instructions to mega in conversation with player
//the instruction is interpreted by the script mega_interact
int Logic::fnTheyDo(Object *cpt, int32 id, int32 tar, int32 instruc, int32 param1, int32 param2, int32 param3, int32 x) {
	Object *target;
	target = _objMan->fetchObject(tar);
	target->o_down_flag = instruc; // instruction for the mega
	target->o_ins1 = param1;
	target->o_ins2 = param2;
	target->o_ins3 = param3;
	return SCRIPT_CONT;
}

//send an instruction to mega we're talking to and wait
//until it has finished before returning to script
int Logic::fnTheyDoWeWait(Object *cpt, int32 id, int32 tar, int32 instruc, int32 param1, int32 param2, int32 param3, int32 x) {
	// workaround for scriptbug #928791: Freeze at hospital
	// in at least one game version, a script forgets to set sam_returning back to zero
	if ((tar == SAM) && (instruc == INS_talk) && (param2 == 2162856))
		_scriptVars[SAM_RETURNING] = 0;
	Object *target = _objMan->fetchObject(tar);
	target->o_down_flag = instruc; // instruction for the mega
	target->o_ins1 = param1;
	target->o_ins2 = param2;
	target->o_ins3 = param3;
	target->o_status &= ~STAT_TALK_WAIT;

	cpt->o_logic = LOGIC_wait_for_talk;
	cpt->o_down_flag = tar;
	return SCRIPT_STOP;
}

int Logic::fnWeWait(Object *cpt, int32 id, int32 tar, int32 d, int32 e, int32 f, int32 z, int32 x) {
	Object *target = _objMan->fetchObject(tar);
	target->o_status &= ~STAT_TALK_WAIT;

	cpt->o_logic = LOGIC_wait_for_talk;
	cpt->o_down_flag = tar;

	return SCRIPT_STOP;
}

int Logic::fnChangeSpeechText(Object *cpt, int32 id, int32 tar, int32 width, int32 pen, int32 f, int32 z, int32 x) {
	Object *target = _objMan->fetchObject(tar);
	target->o_speech_width = width;
	target->o_speech_pen = pen;
	return SCRIPT_STOP;
}

//mega_interact has received an instruction it does not understand -
//The game is halted for debugging. Maybe we'll remove this later.
int Logic::fnTalkError(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	error("fnTalkError for id %d, instruction %d", id, cpt->o_down_flag);
	return SCRIPT_STOP; // for compilers that don't support NORETURN
}

int Logic::fnStartTalk(Object *cpt, int32 id, int32 target, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_down_flag = target;
	cpt->o_logic = LOGIC_start_talk;
	return SCRIPT_STOP;
}

int Logic::fnCheckForTextLine(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_scriptVars[RETURN_VALUE] = _objMan->fnCheckForTextLine(id);
	return SCRIPT_CONT;
}

int Logic::fnAddTalkWaitStatusBit(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status |= STAT_TALK_WAIT;
	return SCRIPT_CONT;
}

int Logic::fnRemoveTalkWaitStatusBit(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status &= ~STAT_TALK_WAIT;
	return SCRIPT_CONT;
}

int Logic::fnNoHuman(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnNoHuman();
	return SCRIPT_CONT;
}

int Logic::fnAddHuman(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnAddHuman();
	return SCRIPT_CONT;
}

int Logic::fnBlankMouse(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnBlankMouse();
	return SCRIPT_CONT;
}

int Logic::fnNormalMouse(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnNormalMouse();
	return SCRIPT_CONT;
}

int Logic::fnLockMouse(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnLockMouse();
	return SCRIPT_CONT;
}

int Logic::fnUnlockMouse(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_mouse->fnUnlockMouse();
	return SCRIPT_CONT;
}

int Logic::fnSetMousePointer(Object *cpt, int32 id, int32 tag, int32 rate, int32 e, int32 f, int32 z, int32 x) {
	_mouse->setPointer(tag, rate);
	return SCRIPT_CONT;
}

int Logic::fnSetMouseLuggage(Object *cpt, int32 id, int32 tag, int32 rate, int32 e, int32 f, int32 z, int32 x) {
	_mouse->setLuggage(tag, rate);
	return SCRIPT_CONT;
}

int Logic::fnMouseOn(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status |= STAT_MOUSE;
	return SCRIPT_CONT;
}

int Logic::fnMouseOff(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_status &= ~STAT_MOUSE;
	return SCRIPT_CONT;
}

int Logic::fnChooser(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->fnChooser(cpt);
	return SCRIPT_STOP;
}

int Logic::fnEndChooser(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->fnEndChooser();
	return SCRIPT_CONT;
}

int Logic::fnStartMenu(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->fnStartMenu();
	return SCRIPT_CONT;
}

int Logic::fnEndMenu(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->fnEndMenu();
	return SCRIPT_CONT;
}

int Logic::cfnReleaseMenu(Object *cpt, int32 id, int32 c, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->cfnReleaseMenu();
	return SCRIPT_STOP;
}

int Logic::fnAddSubject(Object *cpt, int32 id, int32 sub, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_menu->fnAddSubject(sub);
	return SCRIPT_CONT;
}

int Logic::fnAddObject(Object *cpt, int32 id, int32 objectNo, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_scriptVars[POCKET_1 + objectNo - 1] = 1; // basically means: carrying object objectNo = true;
	return SCRIPT_CONT;
}

int Logic::fnRemoveObject(Object *cpt, int32 id, int32 objectNo, int32 d, int32 e, int32 f, int32 z, int32 x) {
	_scriptVars[POCKET_1 + objectNo - 1] = 0;
	return SCRIPT_CONT;
}

int Logic::fnEnterSection(Object *cpt, int32 id, int32 screen, int32 d, int32 e, int32 f, int32 z, int32 x) {
	if (screen >= TOTAL_SECTIONS)
		error("mega %d tried entering section %d", id, screen);

	/* if (cpt->o_type == TYPE_PLAYER)
	   ^= this was the original condition from the game sourcecode.
	   not sure why it doesn't work*/
	if (id == PLAYER)
		_scriptVars[NEW_SCREEN] = screen;
	else
		cpt->o_screen = screen; // move the mega
	_objMan->megaEntering(screen);
	return SCRIPT_CONT;
}

int Logic::fnLeaveSection(Object *cpt, int32 id, int32 oldScreen, int32 d, int32 e, int32 f, int32 z, int32 x) {
	if (oldScreen >= TOTAL_SECTIONS)
		error("mega %d leaving section %d", id, oldScreen);
	_objMan->megaLeaving(oldScreen, id);
	return SCRIPT_CONT;
}

int Logic::fnChangeFloor(Object *cpt, int32 id, int32 floor, int32 d, int32 e, int32 f, int32 z, int32 x) {
	cpt->o_place = floor;
	Object *floorCpt = _objMan->fetchObject(floor);
	cpt->o_scale_a = floorCpt->o_scale_a;
	cpt->o_scale_b = floorCpt->o_scale_b;
	return SCRIPT_CONT;
}

int Logic::fnWalk(Object *cpt, int32 id, int32 x, int32 y, int32 dir, int32 stance, int32 a, int32 b) {
	if (stance > 0)
		dir = 9;
	cpt->o_walk_pc = 0;
	cpt->o_route[1].frame = 512; // end of sequence
	if (id == PLAYER)
		_router->setPlayerTarget(x, y, dir, stance);

	int32 routeRes = _router->routeFinder(id, cpt, x, y, dir);

	if (id == PLAYER) {
		if ((routeRes == 1) || (routeRes == 2)) {
			_scriptVars[MEGA_ON_GRID] = 0;
			_scriptVars[REROUTE_GEORGE] = 0;
		}
	}
	if ((routeRes == 1) || (routeRes == 2)) {
		cpt->o_down_flag = 1; // 1 means okay.
		// if both mouse buttons were pressed on an exit => skip george's walk
		if ((id == GEORGE) && (_mouse->testEvent() == MOUSE_BOTH_BUTTONS)) {
			int32 target = _scriptVars[CLICK_ID];
			// exceptions: compacts that use hand pointers but are not actually exits
			if ((target != LEFT_SCROLL_POINTER) && (target != RIGHT_SCROLL_POINTER) &&
			        (target != FLOOR_63) && (target != ROOF_63) && (target != GUARD_ROOF_63) &&
			        (target != LEFT_TREE_POINTER_71) && (target != RIGHT_TREE_POINTER_71)) {

				target = _objMan->fetchObject(_scriptVars[CLICK_ID])->o_mouse_on;
				if ((target >= SCR_exit0) && (target <= SCR_exit9)) {
					fnStandAt(cpt, id, x, y, dir, stance, 0, 0);
					return SCRIPT_STOP;
				}
			}
		}
		cpt->o_logic = LOGIC_AR_animate;
		return SCRIPT_STOP;
	} else if (routeRes == 3)
		cpt->o_down_flag = 1; // pretend it was successful
	else
		cpt->o_down_flag = 0; // 0 means error

	return SCRIPT_CONT;
}

int Logic::fnTurn(Object *cpt, int32 id, int32 dir, int32 stance, int32 c, int32 d, int32 a, int32 b) {
	if (stance > 0)
		dir = 9;
	int route = _router->routeFinder(id, cpt, cpt->o_xcoord, cpt->o_ycoord, dir);

	if (route)
		cpt->o_down_flag = 1;       //1 means ok
	else
		cpt->o_down_flag = 0;       //0 means error

	cpt->o_logic = LOGIC_AR_animate;
	cpt->o_walk_pc = 0;                     //reset

	return SCRIPT_STOP;
}

int Logic::fnStand(Object *cpt, int32 id, int32 dir, int32 stance, int32 c, int32 d, int32 a, int32 b) {
	if ((dir < 0) || (dir > 8)) {
		warning("fnStand:: invalid direction %d", dir);
		return SCRIPT_CONT;
	}
	if (dir == 8)
		dir = cpt->o_dir;
	cpt->o_resource = cpt->o_walk_resource;
	cpt->o_status |= STAT_SHRINK;
	cpt->o_anim_x = cpt->o_xcoord;
	cpt->o_anim_y = cpt->o_ycoord;
	cpt->o_frame = 96 + dir;
	cpt->o_dir = dir;
	return SCRIPT_STOP;
}

int Logic::fnStandAt(Object *cpt, int32 id, int32 x, int32 y, int32 dir, int32 stance, int32 a, int32 b) {
	if ((dir < 0) || (dir > 8)) {
		warning("fnStandAt:: invalid direction %d", dir);
		return SCRIPT_CONT;
	}
	if (dir == 8)
		dir = cpt->o_dir;
	cpt->o_xcoord = x;
	cpt->o_ycoord = y;
	return fnStand(cpt, id, dir, stance, 0, 0, 0, 0);
}

int Logic::fnFace(Object *cpt, int32 id, int32 targetId, int32 b, int32 c, int32 d, int32 a, int32 z) {
	Object *target = _objMan->fetchObject(targetId);
	int32 x, y;
	if ((target->o_type == TYPE_MEGA) || (target->o_type == TYPE_PLAYER)) {
		x = target->o_xcoord;
		y = target->o_ycoord;
	} else {
		x = (target->o_mouse_x1 + target->o_mouse_x2) / 2;
		y = target->o_mouse_y2;
	}
	int32 megaTarDir = whatTarget(cpt->o_xcoord, cpt->o_ycoord, x, y);
	fnTurn(cpt, id, megaTarDir, 0, 0, 0, 0, 0);
	return SCRIPT_STOP;
}

int Logic::fnFaceXy(Object *cpt, int32 id, int32 x, int32 y, int32 c, int32 d, int32 a, int32 b) {
	int megaTarDir = whatTarget(cpt->o_xcoord, cpt->o_ycoord, x, y);
	fnTurn(cpt, id, megaTarDir, 0, 0, 0, 0, 0);
	return SCRIPT_STOP;
}

int Logic::fnIsFacing(Object *cpt, int32 id, int32 targetId, int32 b, int32 c, int32 d, int32 a, int32 z) {
	Object *target = _objMan->fetchObject(targetId);
	int32 x, y, dir;
	if ((target->o_type == TYPE_MEGA) || (target->o_type == TYPE_PLAYER)) {
		x = target->o_xcoord;
		y = target->o_ycoord;
		dir = target->o_dir;
	} else
		error("fnIsFacing:: Target isn't a mega");

	int32 lookDir = whatTarget(x, y, cpt->o_xcoord, cpt->o_ycoord);
	lookDir -= dir;
	lookDir = ABS(lookDir);

	if (lookDir > 4)
		lookDir = 8 - lookDir;

	_scriptVars[RETURN_VALUE] = lookDir;
	return SCRIPT_STOP;
}

int Logic::fnGetTo(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	Object *place = _objMan->fetchObject(cpt->o_place);

	cpt->o_tree.o_script_level++;
	cpt->o_tree.o_script_pc[cpt->o_tree.o_script_level] = place->o_get_to_script;
	cpt->o_tree.o_script_id[cpt->o_tree.o_script_level] = place->o_get_to_script;
	return SCRIPT_STOP;
}

int Logic::fnGetToError(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	debug(1, "fnGetToError: compact %d at place %d no get-to for target %d, click_id %d\n", id, cpt->o_place, cpt->o_target, _scriptVars[CLICK_ID]);
	return SCRIPT_CONT;
}

int Logic::fnRandom(Object *compact, int32 id, int32 min, int32 max, int32 e, int32 f, int32 z, int32 x) {
	_scriptVars[RETURN_VALUE] = _rnd.getRandomNumberRng(min, max);
	return SCRIPT_CONT;
}

int Logic::fnGetPos(Object *cpt, int32 id, int32 targetId, int32 b, int32 c, int32 d, int32 z, int32 x) {
	Object *target = _objMan->fetchObject(targetId);
	if ((target->o_type == TYPE_MEGA) || (target->o_type == TYPE_PLAYER)) {
		_scriptVars[RETURN_VALUE]   = target->o_xcoord;
		_scriptVars[RETURN_VALUE_2] = target->o_ycoord;
	} else {
		_scriptVars[RETURN_VALUE]   = (target->o_mouse_x1 + target->o_mouse_x2) / 2;
		_scriptVars[RETURN_VALUE_2] = target->o_mouse_y2;
	}
	_scriptVars[RETURN_VALUE_3] = target->o_dir;

	int32 megaSeperation;
	if (targetId == DUANE)
		megaSeperation = 70; // George & Duane stand with feet 70 pixels apart when at full scale
	else if (targetId == BENOIR)
		megaSeperation = 61; // George & Benoir
	else
		megaSeperation = 42; // George & Nico/Goinfre stand with feet 42 pixels apart when at full scale

	if (target->o_status & STAT_SHRINK) {
		int32 scale = (target->o_scale_a * target->o_ycoord + target->o_scale_b) / 256;
		_scriptVars[RETURN_VALUE_4] = (megaSeperation * scale) / 256;
	} else
		_scriptVars[RETURN_VALUE_4] = megaSeperation;
	return SCRIPT_CONT;
}

int Logic::fnGetGamepadXy(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	// playstation only
	return SCRIPT_CONT;
}

int Logic::fnPlayFx(Object *cpt, int32 id, int32 fxNo, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_scriptVars[RETURN_VALUE] = _sound->addToQueue(fxNo);
	return SCRIPT_CONT;
}

int Logic::fnStopFx(Object *cpt, int32 id, int32 fxNo, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_sound->fnStopFx(fxNo);
	//_sound->removeFromQueue(fxNo);
	return SCRIPT_CONT;
}

int Logic::fnPlayMusic(Object *cpt, int32 id, int32 tuneId, int32 loopFlag, int32 c, int32 d, int32 z, int32 x) {
	if (tuneId == 153)
		return SCRIPT_CONT;
	if (loopFlag == LOOPED)
		_scriptVars[CURRENT_MUSIC] = tuneId; // so it gets restarted when saving & reloading
	else
		_scriptVars[CURRENT_MUSIC] = 0;

	_music->startMusic(tuneId, loopFlag);
	return SCRIPT_CONT;
}

int Logic::fnStopMusic(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_scriptVars[CURRENT_MUSIC] = 0;
	_music->fadeDown();
	return SCRIPT_CONT;
}

int Logic::fnInnerSpace(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	error("fnInnerSpace() not working");
	return SCRIPT_STOP; // for compilers that don't support NORETURN
}

int Logic::fnSetScreen(Object *cpt, int32 id, int32 target, int32 screen, int32 c, int32 d, int32 z, int32 x) {
	_objMan->fetchObject(target)->o_screen = screen;
	return SCRIPT_CONT;
}

int Logic::fnPreload(Object *cpt, int32 id, int32 resId, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_resMan->resOpen(resId);
	_resMan->resClose(resId);
	return SCRIPT_CONT;
}

int Logic::fnCheckCD(Object *cpt, int32 id, int32 screen, int32 b, int32 c, int32 d, int32 z, int32 x) {
	// only a dummy, here.
	// the check is done in the mainloop
	return SCRIPT_CONT;
}

int Logic::fnRestartGame(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	SwordEngine::_systemVars.forceRestart = true;
	cpt->o_logic = LOGIC_quit;
	return SCRIPT_STOP;
}

int Logic::fnQuitGame(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	if (SwordEngine::_systemVars.isDemo) {
		GUI::MessageDialog dialog(_("This is the end of the Broken Sword 1 Demo"), _("OK"), NULL);
		dialog.runModal();
		Engine::quitGame();
	} else
		error("fnQuitGame() called");
	return fnQuit(cpt, id, 0, 0, 0, 0, 0, 0);
}

int Logic::fnDeathScreen(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {

	if (_scriptVars[FINALE_OPTION_FLAG] == 4) // successful end of game!
		SwordEngine::_systemVars.controlPanelMode = CP_THEEND;
	else
		SwordEngine::_systemVars.controlPanelMode = CP_DEATHSCREEN;

	cpt->o_logic = LOGIC_quit;
	return SCRIPT_STOP;
}

int Logic::fnSetParallax(Object *cpt, int32 id, int32 screen, int32 resId, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnSetParallax(screen, resId);
	return SCRIPT_CONT;
}

int Logic::fnTdebug(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	debug(1, "Script TDebug id %d code %d, %d", id, a, b);
	return SCRIPT_CONT;
}

int Logic::fnRedFlash(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(FLASH_RED);
	return SCRIPT_CONT;
}

int Logic::fnBlueFlash(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(FLASH_BLUE);
	return SCRIPT_CONT;
}

int Logic::fnYellow(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(BORDER_YELLOW);
	return SCRIPT_CONT;
}

int Logic::fnGreen(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(BORDER_GREEN);
	return SCRIPT_CONT;
}

int Logic::fnPurple(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(BORDER_PURPLE);
	return SCRIPT_CONT;
}

int Logic::fnBlack(Object *cpt, int32 id, int32 a, int32 b, int32 c, int32 d, int32 z, int32 x) {
	_screen->fnFlash(BORDER_BLACK);
	return SCRIPT_CONT;
}

void Logic::startPosCallFn(uint8 fnId, uint32 param1, uint32 param2, uint32 param3) {
	Object *obj = NULL;
	switch (fnId) {
	case opcPlaySequence:
		fnPlaySequence(NULL, 0, param1, 0, 0, 0, 0, 0);
		break;
	case opcAddObject:
		fnAddObject(NULL, 0, param1, 0, 0, 0, 0, 0);
		break;
	case opcRemoveObject:
		fnRemoveObject(NULL, 0, param1, 0, 0, 0, 0, 0);
		break;
	case opcMegaSet:
		obj = _objMan->fetchObject(param1);
		fnMegaSet(obj, param1, param2, param3, 0, 0, 0, 0);
		break;
	case opcNoSprite:
		obj = _objMan->fetchObject(param1);
		fnNoSprite(obj, param1, param2, param3, 0, 0, 0, 0);
		break;
	default:
		error("Illegal fnCallfn argument %d", fnId);
	}
}

void Logic::runStartScript(const uint8 *data) {
	// Here data is a static resource defined in staticres.cpp
	// It is always in little endian
	uint16 varId = 0;
	uint8 fnId = 0;
	uint32 param1 = 0;
	while (*data != opcSeqEnd) {
		switch (*data++) {
		case opcCallFn:
			fnId = *data++;
			param1 = *data++;
			startPosCallFn(fnId, param1, 0, 0);
			break;
		case opcCallFnLong:
			fnId = *data++;
			startPosCallFn(fnId, READ_LE_UINT32(data), READ_LE_UINT32(data + 4), READ_LE_UINT32(data + 8));
			data += 12;
			break;
		case opcSetVar8:
			varId = READ_LE_UINT16(data);
			_scriptVars[varId] = data[2];
			data += 3;
			break;
		case opcSetVar16:
			varId = READ_LE_UINT16(data);
			_scriptVars[varId] = READ_LE_UINT16(data + 2);
			data += 4;
			break;
		case opcSetVar32:
			varId = READ_LE_UINT16(data);
			_scriptVars[varId] = READ_LE_UINT32(data + 2);
			data += 6;
			break;
		case opcGeorge:
			_scriptVars[CHANGE_X]     = READ_LE_UINT16(data + 0);
			_scriptVars[CHANGE_Y]     = READ_LE_UINT16(data + 2);
			_scriptVars[CHANGE_DIR]   = data[4];
			_scriptVars[CHANGE_PLACE] = READ_LE_UINT24(data + 5);
			data += 8;
			break;
		case opcRunStart:
			data = _startData[*data];
			break;
		case opcRunHelper:
			data = _helperData[*data];
			break;
		default:
			error("Unexpected opcode in StartScript");
		}
	}
}

void Logic::startPositions(uint32 pos) {
	bool spainVisit2 = false;
	if ((pos >= 956) && (pos <= 962)) {
		spainVisit2 = true;
		pos -= 900;
	}
	if ((pos > 80) || (_startData[pos] == NULL))
		error("Starting in Section %d is not supported", pos);

	Logic::_scriptVars[CHANGE_STANCE] = STAND;
	Logic::_scriptVars[GEORGE_CDT_FLAG] = GEO_TLK_TABLE;

	runStartScript(_startData[pos]);
	if (spainVisit2)
		runStartScript(_helperData[HELP_SPAIN2]);

	if (pos == 0)
		pos = 1;
	Object *compact = _objMan->fetchObject(PLAYER);
	fnEnterSection(compact, PLAYER, pos, 0, 0, 0, 0, 0);    // (automatically opens the compact resource for that section)
	SwordEngine::_systemVars.controlPanelMode = CP_NORMAL;
	SwordEngine::_systemVars.wantFade = true;
}

const uint32 Logic::_scriptVarInit[NON_ZERO_SCRIPT_VARS][2] = {
	{  42,  448}, {  43,  378}, {  51,    1}, {  92,    1}, { 147,   71}, { 201,   1},
	{ 209,    1}, { 215,    1}, { 242,    2}, { 244,    1}, { 246,    3}, { 247,   1},
	{ 253,    1}, { 297,    1}, { 398,    1}, { 508,    1}, { 605,    1}, { 606,   1},
	{ 701,    1}, { 709,    1}, { 773,    1}, { 843,    1}, { 907,    1}, { 923,   1},
	{ 966,    1}, { 988,    2}, {1058,    1}, {1059,    2}, {1060,    3}, {1061,   4},
	{1062,    5}, {1063,    6}, {1064,    7}, {1065,    8}, {1066,    9}, {1067,  10},
	{1068,   11}, {1069,   12}, {1070,   13}, {1071,   14}, {1072,   15}, {1073,  16},
	{1074,   17}, {1075,   18}, {1076,   19}, {1077,   20}, {1078,   21}, {1079,  22},
	{1080,   23}, {1081,   24}, {1082,   25}, {1083,   26}, {1084,   27}, {1085,  28},
	{1086,   29}, {1087,   30}, {1088,   31}, {1089,   32}, {1090,   33}, {1091,  34},
	{1092,   35}, {1093,   36}, {1094,   37}, {1095,   38}, {1096,   39}, {1097,  40},
	{1098,   41}, {1099,   42}, {1100,   43}, {1101,   44}, {1102,   48}, {1103,  45},
	{1104,   47}, {1105,   49}, {1106,   50}, {1107,   52}, {1108,   54}, {1109,  56},
	{1110,   57}, {1111,   58}, {1112,   59}, {1113,   60}, {1114,   61}, {1115,  62},
	{1116,   63}, {1117,   64}, {1118,   65}, {1119,   66}, {1120,   67}, {1121,  68},
	{1122,   69}, {1123,   71}, {1124,   72}, {1125,   73}, {1126,   74}
};

} // End of namespace Sword1
