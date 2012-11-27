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


#include "common/system.h"
#include "common/file.h"
#include "common/textconsole.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/screen.h"
#include "sword2/console.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/mouse.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/sound.h"
#include "sword2/animation.h"

namespace Sword2 {

int32 Logic::fnTestFunction(int32 *params) {
	// params:	0 address of a flag
	return IR_CONT;
}

int32 Logic::fnTestFlags(int32 *params) {
	// params:	0 value of flag
	return IR_CONT;
}

int32 Logic::fnRegisterStartPoint(int32 *params) {
	// params:	0 id of startup script to call - key
	//		1 pointer to ascii message

	int32 key = params[0];
	char *name = (char *)decodePtr(params[1]);

	_vm->registerStartPoint(key, name);
	return IR_CONT;
}

int32 Logic::fnInitBackground(int32 *params) {
	// this screen defines the size of the back buffer

	// params:	0 res id of normal background layer - cannot be 0
	//		1 1 yes 0 no for a new palette

	if (Sword2Engine::isPsx())
		_vm->_screen->initPsxBackground(params[0], params[1]);
	else
		_vm->_screen->initBackground(params[0], params[1]);
	return IR_CONT;
}

/**
 * This function is used by start scripts.
 */

int32 Logic::fnSetSession(int32 *params) {
	// params:	0 id of new run list

	expressChangeSession(params[0]);
	return IR_CONT;
}

int32 Logic::fnBackSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	_router->setSpriteStatus(decodePtr(params[0]), BACK_SPRITE);
	return IR_CONT;
}

int32 Logic::fnSortSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	_router->setSpriteStatus(decodePtr(params[0]), SORT_SPRITE);
	return IR_CONT;
}

int32 Logic::fnForeSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	_router->setSpriteStatus(decodePtr(params[0]), FORE_SPRITE);
	return IR_CONT;
}

int32 Logic::fnRegisterMouse(int32 *params) {
	// this call would be made from an objects service script 0
	// the object would be one with no graphic but with a mouse - i.e. a
	// floor or one whose mouse area is manually defined rather than
	// intended to fit sprite shape

	// params:	0 pointer to ObjectMouse or 0 for no write to mouse
	//		  list

	_vm->_mouse->registerMouse(decodePtr(params[0]), NULL);
	return IR_CONT;
}

int32 Logic::fnAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 resource id of animation file

	// Normal forward animation
	return _router->doAnimate(
		decodePtr(params[0]),
		decodePtr(params[1]),
		params[2], false);
}

int32 Logic::fnRandom(int32 *params) {
	// params:	0 min
	//		1 max

	writeVar(RESULT, _vm->_rnd.getRandomNumberRng(params[0], params[1]));
	return IR_CONT;
}

int32 Logic::fnPreLoad(int32 *params) {
	// Forces a resource into memory before it's "officially" opened for
	// use. eg. if an anim needs to run on smoothly from another,
	// "preloading" gets it into memory in advance to avoid the cacheing
	// delay that normally occurs before the first frame.

	// params:	0 resource to preload

	_vm->_resman->openResource(params[0]);
	_vm->_resman->closeResource(params[0]);
	return IR_CONT;
}

int32 Logic::fnAddSubject(int32 *params) {
	// params:	0 id
	//		1 daves reference number
	_vm->_mouse->addSubject(params[0], params[1]);
	return IR_CONT;
}

int32 Logic::fnInteract(int32 *params) {
	// Run targets action on a subroutine. Called by player on his base
	// level 0 idle, for example.

	// params:	0 id of target from which we derive action script
	//		  reference

	writeVar(PLAYER_ACTION, 0);		// must clear this
	logicUp((params[0] << 16) | 2);		// 3rd script of clicked on id

	// Out, up and around again - pc is saved for current level to be
	// returned to.
	return IR_GOSUB;
}

int32 Logic::fnChoose(int32 *params) {
	// params:	none

	// This opcode is used to open the conversation menu. The human is
	// switched off so there will be no normal mouse engine.

	// The player's choice is piggy-backed on the standard opcode return
	// values, to be used with the CP_JUMP_ON_RETURNED opcode. As far as I
	// can tell, this is the only function that uses that feature.

	uint32 response = _vm->_mouse->chooseMouse();

	if (response == (uint32)-1)
		return IR_REPEAT;

	return IR_CONT | (response << 3);
}

/**
 * Walk mega to (x,y,dir). Set RESULT to 0 if it succeeded. Otherwise, set
 * RESULT to 1.
 */

int32 Logic::fnWalk(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target x-coord
	//		5 target y-coord
	//		6 target direction (8 means end walk on ANY direction)

	return _router->doWalk(
		decodePtr(params[0]),
		decodePtr(params[1]),
		decodePtr(params[2]),
		decodePtr(params[3]),
		params[4], params[5], params[6]);
}

/**
 * Walk mega to start position of anim
 */

int32 Logic::fnWalkToAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 anim resource id

	return _router->walkToAnim(
		decodePtr(params[0]),
		decodePtr(params[1]),
		decodePtr(params[2]),
		decodePtr(params[3]),
		params[4]);
}

/**
 * Turn mega to the specified direction.
 */

int32 Logic::fnTurn(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target direction

	return _router->doFace(
		decodePtr(params[0]),
		decodePtr(params[1]),
		decodePtr(params[2]),
		decodePtr(params[3]),
		params[4]);
}

/**
 * Stand mega at (x,y,dir)
 * Sets up the graphic object, but also needs to set the new 'current_dir' in
 * the mega object, so the router knows in future
 */

int32 Logic::fnStandAt(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 target x-coord
	//		3 target y-coord
	//		4 target direction

	_router->standAt(
		decodePtr(params[0]),
		decodePtr(params[1]),
		params[2], params[3], params[4]);
	return IR_CONT;
}

/**
 * Stand mega into the specified direction at current feet coords.
 * Just needs to call standAt() with current feet coords.
 */

int32 Logic::fnStand(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 target direction
	byte *ob_mega = decodePtr(params[1]);

	ObjectMega obMega(ob_mega);

	_router->standAt(
		decodePtr(params[0]),
		ob_mega, obMega.getFeetX(), obMega.getFeetY(), params[2]);
	return IR_CONT;
}

/**
 * stand mega at end position of anim
 */

int32 Logic::fnStandAfterAnim(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 anim resource id

	_router->standAfterAnim(
		decodePtr(params[0]),
		decodePtr(params[1]),
		params[2]);
	return IR_CONT;
}

int32 Logic::fnPause(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 number of game-cycles to pause

	// NB. Pause-value of 0 causes script to continue, 1 causes a 1-cycle
	// quit, 2 gives 2 cycles, etc.

	ObjectLogic obLogic(decodePtr(params[0]));

	if (obLogic.getLooping() == 0) {
		obLogic.setLooping(1);
		obLogic.setPause(params[1]);
	}

	if (obLogic.getPause()) {
		obLogic.setPause(obLogic.getPause() - 1);
		return IR_REPEAT;
	}

	obLogic.setLooping(0);
	return IR_CONT;
}

int32 Logic::fnMegaTableAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to animation table

	// Normal forward anim
	return _router->megaTableAnimate(
		decodePtr(params[0]),
		decodePtr(params[1]),
		decodePtr(params[2]),
		decodePtr(params[3]),
		false);
}

int32 Logic::fnAddMenuObject(int32 *params) {
	// params:	0 pointer to a MenuObject structure to copy down

	_vm->_mouse->addMenuObject(decodePtr(params[0]));
	return IR_CONT;
}

/**
 * Start a conversation.
 *
 * Note that fnStartConversation() might accidentally be called every time the
 * script loops back for another chooser, but we only want to reset the chooser
 * count flag the first time this function is called, i.e. when the talk flag
 * is zero.
 */

int32 Logic::fnStartConversation(int32 *params) {
	// params:	none

	_vm->_mouse->startConversation();
	return IR_CONT;
}

/**
 * End a conversation.
 */

int32 Logic::fnEndConversation(int32 *params) {
	// params:	none

	_vm->_mouse->endConversation();
	return IR_CONT;
}

int32 Logic::fnSetFrame(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 resource id of animation file
	//		2 frame flag (0=first 1=last)

	int32 res = params[1];
	assert(res);

	// open the resource (& check it's valid)
	byte *anim_file = _vm->_resman->openResource(res);

	assert(_vm->_resman->fetchType(res) == ANIMATION_FILE);

	// set up pointer to the animation header
	AnimHeader anim_head;

	anim_head.read(_vm->fetchAnimHeader(anim_file));

	// set up anim resource in graphic object
	ObjectGraphic obGraph(decodePtr(params[0]));

	obGraph.setAnimResource(res);
	obGraph.setAnimPc(params[2] ? anim_head.noAnimFrames - 1 : 0);

	// Close the anim file and drop out of script
	_vm->_resman->closeResource(obGraph.getAnimResource());
	return IR_CONT;
}

int32 Logic::fnRandomPause(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 minimum number of game-cycles to pause
	//		2 maximum number of game-cycles to pause

	ObjectLogic obLogic(decodePtr(params[0]));
	int32 pars[2];

	if (obLogic.getLooping() == 0) {
		pars[0] = params[1];
		pars[1] = params[2];
		fnRandom(pars);
		pars[1] = readVar(RESULT);
	}

	pars[0] = params[0];
	return fnPause(pars);
}

int32 Logic::fnRegisterFrame(int32 *params) {
	// this call would be made from an objects service script 0

	// params:	0 pointer to mouse structure or NULL for no write to
	//		  mouse list (non-zero means write sprite-shape to
	//		  mouse list)
	//		1 pointer to graphic structure
	//		2 pointer to mega structure or NULL if not a mega

	_vm->_screen->registerFrame(
		decodePtr(params[0]),
		decodePtr(params[1]),
		decodePtr(params[2]));
	return IR_CONT;
}

int32 Logic::fnNoSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	_router->setSpriteStatus(decodePtr(params[0]), NO_SPRITE);
	return IR_CONT;
}

int32 Logic::fnSendSync(int32 *params) {
	// params:	0 sync's recipient
	//		1 sync value

	sendSync(params[0], params[1]);
	return IR_CONT;
}

int32 Logic::fnUpdatePlayerStats(int32 *params) {
	// engine needs to know certain info about the player

	// params:	0 pointer to mega structure

	ObjectMega obMega(decodePtr(params[0]));

	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	screenInfo->player_feet_x = obMega.getFeetX();
	screenInfo->player_feet_y = obMega.getFeetY();

	// for the script
	writeVar(PLAYER_FEET_X, obMega.getFeetX());
	writeVar(PLAYER_FEET_Y, obMega.getFeetY());
	writeVar(PLAYER_CUR_DIR, obMega.getCurDir());
	writeVar(SCROLL_OFFSET_X, screenInfo->scroll_offset_x);

	debug(5, "fnUpdatePlayerStats: %d %d",
		obMega.getFeetX(), obMega.getFeetY());

	return IR_CONT;
}

int32 Logic::fnPassGraph(int32 *params) {
	// makes an engine local copy of passed ObjectGraphic - run script 4
	// of an object to request this used by fnTurnTo(id) etc
	//
	// remember, we cannot simply read a compact any longer but instead
	// must request it from the object itself

	// params:	0 pointer to an ObjectGraphic structure

	warning("fnPassGraph() is a no-op now");
	return IR_CONT;
}

int32 Logic::fnInitFloorMouse(int32 *params) {
	// params:	0 pointer to object's mouse structure

	byte *ob_mouse = decodePtr(params[0]);
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	// floor is always lowest priority

	ObjectMouse mouse;

	mouse.x1 = 0;
	mouse.y1 = 0;
	mouse.x2 = screenInfo->screen_wide - 1;
	mouse.y2 = screenInfo->screen_deep - 1;
	mouse.priority = 9;
	mouse.pointer = NORMAL_MOUSE_ID;

	mouse.write(ob_mouse);
	return IR_CONT;
}

int32 Logic::fnPassMega(int32 *params) {
	// makes an engine local copy of passed mega_structure - run script 4
	// of an object to request this used by fnTurnTo(id) etc
	//
	// remember, we cannot simply read a compact any longer but instead
	// must request it from the object itself

	// params:	0 pointer to a mega structure

	memcpy(_engineMega, decodePtr(params[0]), ObjectMega::size());
	return IR_CONT;
}

/**
 * Turn mega to face point (x,y) on the floor
 */

int32 Logic::fnFaceXY(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target x-coord
	//		5 target y-coord

	return _router->faceXY(
		decodePtr(params[0]),
		decodePtr(params[1]),
		decodePtr(params[2]),
		decodePtr(params[3]),
		params[4], params[5]);
}

/**
 * Causes no more objects in this logic loop to be processed. The logic engine
 * will restart at the beginning of the new list. The current screen will not
 * be drawn!
 */

int32 Logic::fnEndSession(int32 *params) {
	// params:	0 id of new run-list

	// terminate current and change to next run-list
	expressChangeSession(params[0]);

	// stop the script - logic engine will now go around and the new
	// screen will begin
	return IR_STOP;
}

int32 Logic::fnNoHuman(int32 *params) {
	// params:	none

	_vm->_mouse->noHuman();
	return IR_CONT;
}

int32 Logic::fnAddHuman(int32 *params) {
	// params:	none

	_vm->_mouse->addHuman();
	return IR_CONT;
}

/**
 * Wait for a target to become waiting, i.e. not busy.
 */

int32 Logic::fnWeWait(int32 *params) {
	// params:	0 target

	assert(_vm->_resman->fetchType(params[0]) == GAME_OBJECT);

	// Run the target's get-speech-state script
	runResScript(params[0], 5);

	if (readVar(RESULT) == 0) {
		// The target is busy. Try again.
		_vm->_debugger->_speechScriptWaiting = params[0];
		return IR_REPEAT;
	}

	// The target is waiting, i.e. not busy.

	_vm->_debugger->_speechScriptWaiting = 0;
	return IR_CONT;
}

/**
 * Wait for a target to become waiting, i.e. not busy, send a command to it,
 * then wait for it to finish.
 */

int32 Logic::fnTheyDoWeWait(int32 *params) {
	// params:	0 pointer to ob_logic
	//		1 target
	//		2 command
	//		3 ins1
	//		4 ins2
	//		5 ins3
	//		6 ins4
	//		7 ins5

	assert(_vm->_resman->fetchType(params[1]) == GAME_OBJECT);

	// Run the target's get-speech-state script
	runResScript(params[1], 5);

	ObjectLogic obLogic(decodePtr(params[0]));

	if (readVar(RESULT) == 1 && readVar(INS_COMMAND) == 0 && obLogic.getLooping() == 0) {
		// The target is waiting, i.e. not busy, and there is no other
		// command queued. We haven't sent the command yet, so do it.

		debug(5, "fnTheyDoWeWait: sending command to %d", params[1]);

		_vm->_debugger->_speechScriptWaiting = params[1];
		obLogic.setLooping(1);

		writeVar(SPEECH_ID, params[1]);
		writeVar(INS_COMMAND, params[2]);
		writeVar(INS1, params[3]);
		writeVar(INS2, params[4]);
		writeVar(INS3, params[5]);
		writeVar(INS4, params[6]);
		writeVar(INS5, params[7]);

		return IR_REPEAT;
	}

	if (obLogic.getLooping() == 0) {
		// The command has not been sent yet. Keep waiting.
		_vm->_debugger->_speechScriptWaiting = params[1];
		return IR_REPEAT;
	}

	if (readVar(RESULT) == 0) {
		// The command has been sent, and the target is busy doing it.
		// Wait for it to finish.

		debug(5, "fnTheyDoWeWait: Waiting for %d to finish", params[1]);

		_vm->_debugger->_speechScriptWaiting = params[1];
		return IR_REPEAT;
	}

	debug(5, "fnTheyDoWeWait: %d finished", params[1]);

	obLogic.setLooping(0);
	_vm->_debugger->_speechScriptWaiting = 0;
	return IR_CONT;
}

/**
 * Wait for a target to become waiting, i.e. not busy, then send a command to
 * it.
 */

int32 Logic::fnTheyDo(int32 *params) {
	// params:	0 target
	//		1 command
	//		2 ins1
	//		3 ins2
	//		4 ins3
	//		5 ins4
	//		6 ins5

	assert(_vm->_resman->fetchType(params[0]) == GAME_OBJECT);

	// Run the target's get-speech-state script
	runResScript(params[0], 5);

	if (readVar(RESULT) == 1 && !readVar(INS_COMMAND)) {
		// The target is waiting, i.e. not busy, and there is no other
		// command queued. Send the command.

		debug(5, "fnTheyDo: sending command to %d", params[0]);

		_vm->_debugger->_speechScriptWaiting = 0;

		writeVar(SPEECH_ID, params[0]);
		writeVar(INS_COMMAND, params[1]);
		writeVar(INS1, params[2]);
		writeVar(INS2, params[3]);
		writeVar(INS3, params[4]);
		writeVar(INS4, params[5]);
		writeVar(INS5, params[6]);

		return IR_CONT;
	}

	// The target is busy. Come back again next cycle.

	_vm->_debugger->_speechScriptWaiting = params[0];
	return IR_REPEAT;
}

/**
 * Route to the left or right hand side of target id, if possible.
 */

int32 Logic::fnWalkToTalkToMega(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 id of target mega to face
	//		5 separation

	return _router->walkToTalkToMega(
		decodePtr(params[0]),
		decodePtr(params[1]),
		decodePtr(params[2]),
		decodePtr(params[3]),
		params[4], params[5]);
}

int32 Logic::fnFadeDown(int32 *params) {
	// NONE means up! can only be called when screen is fully faded up -
	// multiple calls wont have strange effects

	// params:	none

	if (_vm->_screen->getFadeStatus() == RDFADE_NONE)
		_vm->_screen->fadeDown();

	return IR_CONT;
}

enum {
	S_OB_GRAPHIC	= 0,
	S_OB_SPEECH	= 1,
	S_OB_LOGIC	= 2,
	S_OB_MEGA	= 3,

	S_TEXT		= 4,
	S_WAV		= 5,
	S_ANIM		= 6,
	S_DIR_TABLE	= 7,
	S_ANIM_MODE	= 8
};

/**
 * It's the super versatile fnSpeak. Text and wavs can be selected in any
 * combination.
 *
 * @note We can assume no human - there should be no human, at least!
 */

int32 Logic::fnISpeak(int32 *params) {
	// params:	0 pointer to ob_graphic
	//		1 pointer to ob_speech
	//		2 pointer to ob_logic
	//		3 pointer to ob_mega
	//		4 encoded text number
	//		5 wav res id
	//		6 anim res id
	//		7 anim table res id
	//		8 animation mode	0 lip synced,
	//					1 just straight animation

	// Set up the pointers which we know we'll always need

	ObjectLogic obLogic(decodePtr(params[S_OB_LOGIC]));
	ObjectGraphic obGraph(decodePtr(params[S_OB_GRAPHIC]));

	// FIRST TIME ONLY: create the text, load the wav, set up the anim,
	// etc.

	if (obLogic.getLooping() == 0) {
		// New fudge to wait for smacker samples to finish
		// since they can over-run into the game

		if (_vm->_sound->getSpeechStatus() != RDSE_SAMPLEFINISHED)
			return IR_REPEAT;

		// New fudge for 'fx' subtitles: If subtitles switched off, and
		// we don't want to use a wav for this line either, then just
		// quit back to script right now!

		if (!_vm->getSubtitles() && !wantSpeechForLine(params[S_WAV]))
			return IR_CONT;

		// Drop out for 1st cycle to allow walks/anims to end and
		// display last frame before system locks while speech loaded

		if (!_cycleSkip) {
			_cycleSkip = true;
			return IR_REPEAT;
		}

		_cycleSkip = false;

		_vm->_debugger->_textNumber = params[S_TEXT];

		// Pull out the text line to get the official text number
		// (for wav id). Once the wav id's go into all script text
		// commands, we'll only need this for debugging.

		uint32 text_res = params[S_TEXT] / SIZE;
		uint32 local_text = params[S_TEXT] & 0xffff;

		// For testing all text & speech!
		//
		// A script loop can send any text number to fnISpeak and it
		// will only run the valid ones or return with 'result' equal
		// to '1' or '2' to mean 'invalid text resource' and 'text
		// number out of range' respectively
		//
		// See 'testing_routines' object in George's Player Character
		// section of linc

		if (readVar(SYSTEM_TESTING_TEXT)) {
			if (!_vm->_resman->checkValid(text_res)) {
				// Not a valid resource number - invalid (null
				// resource)
				writeVar(RESULT, 1);
				return IR_CONT;
			}

			if (_vm->_resman->fetchType(text_res) != TEXT_FILE) {
				// Invalid - not a text resource
				_vm->_resman->closeResource(text_res);
				writeVar(RESULT, 1);
				return IR_CONT;
			}

			if (!_vm->checkTextLine(_vm->_resman->openResource(text_res), local_text)) {
				// Line number out of range
				_vm->_resman->closeResource(text_res);
				writeVar(RESULT, 2);
				return IR_CONT;
			}

			_vm->_resman->closeResource(text_res);
			writeVar(RESULT, 0);
		}

		byte *text = _vm->fetchTextLine(_vm->_resman->openResource(text_res), local_text);
		_officialTextNumber = READ_LE_UINT16(text);
		_vm->_resman->closeResource(text_res);

		// Prevent dud lines from appearing while testing text & speech
		// since these will not occur in the game anyway

		if (readVar(SYSTEM_TESTING_TEXT)) {
			// If actor number is 0 and text line is just a 'dash'
			// character
			if (_officialTextNumber == 0 && text[2] == '-' && text[3] == 0) {
				writeVar(RESULT, 3);
				return IR_CONT;
			}
		}

		// Set the 'looping_flag' and the text-click-delays. We can
		// left-click past the text after half a second, and
		// right-click past it after a quarter of a second.

		obLogic.setLooping(1);
		_leftClickDelay = 6;
		_rightClickDelay = 3;

		if (readVar(PLAYER_ID) != CUR_PLAYER_ID)
			debug(5, "(%d) Nico: %s", _officialTextNumber, text + 2);
		else {
			debug(5, "(%d) %s: %s", _officialTextNumber, _vm->_resman->fetchName(readVar(ID)), text + 2);
		}

		// Set up the speech animation

		if (params[S_ANIM]) {
			// Just a straight anim.
			_animId = params[6];
		} else if (params[S_DIR_TABLE]) {
			// Use this direction table to derive the anim
			// NB. ASSUMES WE HAVE A MEGA OBJECT!!

			ObjectMega obMega(decodePtr(params[S_OB_MEGA]));
			byte *anim_table = decodePtr(params[S_DIR_TABLE]);

			_animId = READ_LE_UINT32(anim_table + 4 * obMega.getCurDir());
		} else {
			// No animation choosen
			_animId = 0;
		}

		if (_animId) {
			// Set the talker's graphic to the first frame of this
			// speech anim for now.

			_speechAnimType = readVar(SPEECHANIMFLAG);
			obGraph.setAnimResource(_animId);
			obGraph.setAnimPc(0);
		}

		// Default back to looped lip synced anims.
		writeVar(SPEECHANIMFLAG, 0);

		// Set up _textX and _textY for speech panning and/or text
		// sprite position.

		locateTalker(params);

		// If the speech is associated with a specific animation, and
		// not just a voice-over, set the focus area to the calculated
		// position.

		if (_animId) {
			_vm->_system->setFocusRectangle(Common::Rect::center(_textX, _textY, 192, 128));
		}

		// Is it to be speech or subtitles or both?

		// Assume not running until know otherwise
		_speechRunning = false;

		// New fudge for 'fx' subtitles: If speech is selected, and
		// this line is allowed speech (not if it's an fx subtitle!)

		if (!_vm->_sound->isSpeechMute() && wantSpeechForLine(_officialTextNumber)) {
			// If the wavId parameter is zero because not yet
			// compiled into speech command, we can still get it
			// from the 1st 2 chars of the text line.

			if (!params[S_WAV])
				params[S_WAV] = (int32)_officialTextNumber;

			// Panning goes from -16 (left) to 16 (right)
			int8 speech_pan = ((_textX - 320) * 16) / 320;

			if (speech_pan < -16)
				speech_pan = -16;
			else if (speech_pan > 16)
				speech_pan = 16;

			uint32 rv = _vm->_sound->playCompSpeech(params[S_WAV], 16, speech_pan);

			if (rv == RD_OK) {
				// Ok, we've got something to play. Set it
				// playing now. (We might want to do this the
				// next cycle, don't know yet.)

				_speechRunning = true;
				_vm->_sound->unpauseSpeech();
			} else {
				debug(5, "ERROR: PlayCompSpeech(wav=%d (res=%d pos=%d)) returned %.8x", params[S_WAV], text_res, local_text, rv);
			}
		}

		if (_vm->getSubtitles() || !_speechRunning) {
			// We want subtitles, or the speech failed to load.
			// Either way, we're going to show the text so create
			// the text sprite.

			formText(params);
		}
	}

	// EVERY TIME: run a cycle of animation, if there is one

	if (_animId) {
		// There is an animation - Increment the anim frame number.
		obGraph.setAnimPc(obGraph.getAnimPc() + 1);

		byte *anim_file = _vm->_resman->openResource(obGraph.getAnimResource());
		AnimHeader anim_head;

		anim_head.read(_vm->fetchAnimHeader(anim_file));

		if (!_speechAnimType) {
			// ANIM IS TO BE LIP-SYNC'ED & REPEATING

			if (obGraph.getAnimPc() == (int32)anim_head.noAnimFrames) {
				// End of animation - restart from frame 0
				obGraph.setAnimPc(0);
			} else if (_speechRunning && _vm->_sound->amISpeaking() == RDSE_QUIET) {
				// The speech is running, but we're at a quiet
				// bit. Restart from frame 0 (closed mouth).
				obGraph.setAnimPc(0);
			}
		} else {
			// ANIM IS TO PLAY ONCE ONLY
			if (obGraph.getAnimPc() == (int32)anim_head.noAnimFrames - 1) {
				// Reached the last frame of the anim. Hold
				// anim on this last frame
				_animId = 0;
			}
		}

		_vm->_resman->closeResource(obGraph.getAnimResource());
	} else if (_speechAnimType) {
		// Placed here so we actually display the last frame of the
		// anim.
		_speechAnimType = 0;
	}

	// EVERY TIME: FIND OUT IF WE NEED TO STOP THE SPEECH NOW...

	// If there is a wav then we're using that to end the speech naturally

	bool speechFinished = false;

	// If playing a sample

	if (_speechRunning) {
		// Has it finished?
		if (_vm->_sound->getSpeechStatus() == RDSE_SAMPLEFINISHED)
			speechFinished = true;
	} else if (!_speechRunning && _speechTime) {
		// Counting down text time because there is no sample - this
		// ends the speech

		// if no sample then we're using _speechTime to end speech
		// naturally

		_speechTime--;
		if (!_speechTime)
			speechFinished = true;
	}

	// Ok, all is running along smoothly - but a click means stop
	// unnaturally

	int mouseX, mouseY;

	_vm->_mouse->getPos(mouseX, mouseY);

	// So that we can go to the options panel while text & speech is
	// being tested
	if (readVar(SYSTEM_TESTING_TEXT) == 0 || mouseY > 0) {
		MouseEvent *me = _vm->mouseEvent();

		// Note that we now have TWO click-delays - one for LEFT
		// button, one for RIGHT BUTTON

		if ((!_leftClickDelay && me && (me->buttons & RD_LEFTBUTTONDOWN)) ||
		    (!_rightClickDelay && me && (me->buttons & RD_RIGHTBUTTONDOWN))) {
			// Mouse click, after click_delay has expired -> end
			// the speech.

			// if testing text & speech
			if (readVar(SYSTEM_TESTING_TEXT)) {
				// and RB used to click past text
				if (me->buttons & RD_RIGHTBUTTONDOWN) {
					// then we want the previous line again
					writeVar(SYSTEM_WANT_PREVIOUS_LINE, 1);
				} else {
					// LB just want next line again
					writeVar(SYSTEM_WANT_PREVIOUS_LINE, 0);
				}
			}

			speechFinished = true;

			// if speech sample playing, halt it prematurely
			if (_speechRunning)
				_vm->_sound->stopSpeech();
		}
	}

	// If we are finishing the speech this cycle, do the business

	// !speechAnimType, as we want an anim which is playing once to have
	// finished.

	if (speechFinished && !_speechAnimType) {
		_vm->_system->clearFocusRectangle();

		// If there is text, kill it
		if (_speechTextBlocNo) {
			_vm->_fontRenderer->killTextBloc(_speechTextBlocNo);
			_speechTextBlocNo = 0;
		}

		// if there is a speech anim, end it on closed mouth frame
		if (_animId) {
			_animId = 0;
			obGraph.setAnimPc(0);
		}

		_speechRunning = false;

		// no longer in a script function loop
		obLogic.setLooping(0);

		_vm->_debugger->_textNumber = 0;

		// reset to zero, in case text line not even extracted (since
		// this number comes from the text line)
		_officialTextNumber = 0;

		writeVar(RESULT, 0);
		return IR_CONT;
	}

	// Speech still going, so decrement the click_delay if it's still
	// active

	if (_leftClickDelay)
		_leftClickDelay--;

	if (_rightClickDelay)
		_rightClickDelay--;

	return IR_REPEAT;
}

/**
 * Reset the object and restart script 1 on level 0
 */

int32 Logic::fnTotalRestart(int32 *params) {
	// mega runs this to restart its base logic again - like being cached
	// in again

	// params:	none

	_curObjectHub.setLogicLevel(0);
	_curObjectHub.setScriptPc(0, 1);

	return IR_TERMINATE;
}

int32 Logic::fnSetWalkGrid(int32 *params) {
	// params:	none

	warning("fnSetWalkGrid() is no longer a valid opcode");
	return IR_CONT;
}

/**
 * Receive and sequence the commands sent from the conversation script. We have
 * to do this in a slightly tweeky manner as we can no longer have generic
 * scripts.
 */

enum {
	INS_talk		= 1,
	INS_anim		= 2,
	INS_reverse_anim	= 3,
	INS_walk		= 4,
	INS_turn		= 5,
	INS_face		= 6,
	INS_trace		= 7,
	INS_no_sprite		= 8,
	INS_sort		= 9,
	INS_foreground		= 10,
	INS_background		= 11,
	INS_table_anim		= 12,
	INS_reverse_table_anim	= 13,
	INS_walk_to_anim	= 14,
	INS_set_frame		= 15,
	INS_stand_after_anim	= 16,
	INS_quit		= 42
};

int32 Logic::fnSpeechProcess(int32 *params) {
	// params:	0 pointer to ob_graphic
	//		1 pointer to ob_speech
	//		2 pointer to ob_logic
	//		3 pointer to ob_mega
	//		4 pointer to ob_walkdata

	ObjectSpeech obSpeech(decodePtr(params[1]));

	while (1) {
		int32 pars[9];

		// Check which command we're waiting for, and call the
		// appropriate function. Once we're done, clear the command
		// and set wait_state to 1.
		//
		// Note: we could save a var and ditch wait_state and check
		// 'command' for non zero means busy
		//
		// Note: I can't see that we ever check the value of wait_state
		// but perhaps it accesses that memory location directly?

		switch (obSpeech.getCommand()) {
		case 0:
			break;
		case INS_talk:
			pars[0] = params[0];		// ob_graphic
			pars[1] = params[1];		// ob_speech
			pars[2] = params[2];		// ob_logic
			pars[3] = params[3];		// ob_mega
			pars[4] = obSpeech.getIns1();	// encoded text number
			pars[5] = obSpeech.getIns2();	// wav res id
			pars[6] = obSpeech.getIns3();	// anim res id
			pars[7] = obSpeech.getIns4();	// anim table res id
			pars[8] = obSpeech.getIns5();	// animation mode - 0 lip synced, 1 just straight animation

			if (fnISpeak(pars) != IR_REPEAT) {
				obSpeech.setCommand(0);
				obSpeech.setWaitState(1);
			}

			return IR_REPEAT;
		case INS_turn:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = params[4];		// ob_walkdata
			pars[4] = obSpeech.getIns1();	// direction to turn to

			if (fnTurn(pars) != IR_REPEAT) {
				obSpeech.setCommand(0);
				obSpeech.setWaitState(1);
			}

			return IR_REPEAT;
		case INS_face:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = params[4];		// ob_walkdata
			pars[4] = obSpeech.getIns1();	// target

			if (fnFaceMega(pars) != IR_REPEAT) {
				obSpeech.setCommand(0);
				obSpeech.setWaitState(1);
			}

			return IR_REPEAT;
		case INS_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = obSpeech.getIns1();	// anim res

			if (fnAnim(pars) != IR_REPEAT) {
				obSpeech.setCommand(0);
				obSpeech.setWaitState(1);
			}

			return IR_REPEAT;
		case INS_reverse_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = obSpeech.getIns1();	// anim res

			if (fnReverseAnim(pars) != IR_REPEAT) {
				obSpeech.setCommand(0);
				obSpeech.setWaitState(1);
			}

			return IR_REPEAT;
		case INS_table_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = obSpeech.getIns1();	// pointer to anim table

			if (fnMegaTableAnim(pars) != IR_REPEAT) {
				obSpeech.setCommand(0);
				obSpeech.setWaitState(1);
			}

			return IR_REPEAT;
		case INS_reverse_table_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = obSpeech.getIns1();	// pointer to anim table

			if (fnReverseMegaTableAnim(pars) != IR_REPEAT) {
				obSpeech.setCommand(0);
				obSpeech.setWaitState(1);
			}

			return IR_REPEAT;
		case INS_no_sprite:
			fnNoSprite(params);		// ob_graphic

			obSpeech.setCommand(0);
			obSpeech.setWaitState(1);
			return IR_REPEAT;
		case INS_sort:
			fnSortSprite(params);		// ob_graphic

			obSpeech.setCommand(0);
			obSpeech.setWaitState(1);
			return IR_REPEAT;
		case INS_foreground:
			fnForeSprite(params);		// ob_graphic

			obSpeech.setCommand(0);
			obSpeech.setWaitState(1);
			return IR_REPEAT;
		case INS_background:
			fnBackSprite(params);		// ob_graphic

			obSpeech.setCommand(0);
			obSpeech.setWaitState(1);
			return IR_REPEAT;
		case INS_walk:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = params[4];		// ob_walkdata
			pars[4] = obSpeech.getIns1();	// target x
			pars[5] = obSpeech.getIns2();	// target y
			pars[6] = obSpeech.getIns3();	// target direction

			if (fnWalk(pars) != IR_REPEAT) {
				obSpeech.setCommand(0);
				obSpeech.setWaitState(1);
			}

			return IR_REPEAT;
		case INS_walk_to_anim:
			pars[0] = params[2];		// ob_logic
			pars[1] = params[0];		// ob_graphic
			pars[2] = params[3];		// ob_mega
			pars[3] = params[4];		// ob_walkdata
			pars[4] = obSpeech.getIns1();	// anim resource

			if (fnWalkToAnim(pars) != IR_REPEAT) {
				obSpeech.setCommand(0);
				obSpeech.setWaitState(1);
			}

			return IR_REPEAT;
		case INS_stand_after_anim:
			pars[0] = params[0];		// ob_graphic
			pars[1] = params[3];		// ob_mega
			pars[2] = obSpeech.getIns1();	// anim resource

			fnStandAfterAnim(pars);

			obSpeech.setCommand(0);
			obSpeech.setWaitState(1);
			return IR_REPEAT;
		case INS_set_frame:
			pars[0] = params[0];		// ob_graphic
			pars[1] = obSpeech.getIns1();	// anim_resource
			pars[2] = obSpeech.getIns2();	// FIRST_FRAME or LAST_FRAME
			fnSetFrame(pars);

			obSpeech.setCommand(0);
			obSpeech.setWaitState(1);
			return IR_REPEAT;
		case INS_quit:
			// That's it - we're finished with this
			obSpeech.setCommand(0);
			// obSpeech.setWaitState(0);
			return IR_CONT;
		default:
			// Unimplemented command - just cancel
			obSpeech.setCommand(0);
			obSpeech.setWaitState(1);
			break;
		}

		if (readVar(SPEECH_ID) == readVar(ID)) {
			// There's a new command for us! Grab the command -
			// potentially we only have this cycle to do this - and
			// set things up so that the command will be picked up
			// on the next iteration of the while loop.

			debug(5, "fnSpeechProcess: Received new command %d", readVar(INS_COMMAND));

			writeVar(SPEECH_ID, 0);

			obSpeech.setCommand(readVar(INS_COMMAND));
			obSpeech.setIns1(readVar(INS1));
			obSpeech.setIns2(readVar(INS2));
			obSpeech.setIns3(readVar(INS3));
			obSpeech.setIns4(readVar(INS4));
			obSpeech.setIns5(readVar(INS5));
			obSpeech.setWaitState(0);

			writeVar(INS_COMMAND, 0);
		} else {
			// No new command. We could run a blink anim (or
			// something) here.

			obSpeech.setWaitState(1);
			return IR_REPEAT;
		}
	}
}

int32 Logic::fnSetScaling(int32 *params) {
	// params:	0 pointer to object's mega structure
	//		1 scale constant A
	//		2 scale constant B

	// 256 * s = A * y + B

	// Where s is system scale, which itself is (256 * actual_scale) ie.
	// s == 128 is half size

	ObjectMega obMega(decodePtr(params[0]));

	obMega.setScaleA(params[1]);
	obMega.setScaleB(params[2]);

	return IR_CONT;
}

int32 Logic::fnStartEvent(int32 *params) {
	// params:	none

	startEvent();
	return IR_TERMINATE;
}

int32 Logic::fnCheckEventWaiting(int32 *params) {
	// params:	none

	writeVar(RESULT, checkEventWaiting());
	return IR_CONT;
}

int32 Logic::fnRequestSpeech(int32 *params) {
	// change current script - must be followed by a TERMINATE script
	// directive

	// params:	0 id of target to catch the event and startup speech
	//		  servicing

	// Full script id to interact with - megas run their own 7th script
	sendEvent(params[0], (params[0] << 16) | 6);
	return IR_CONT;
}

int32 Logic::fnGosub(int32 *params) {
	// params:	0 id of script

	// Hurray, script subroutines. Logic goes up - pc is saved for current
	// level.
	logicUp(params[0]);
	return IR_GOSUB;
}

/**
 * Wait for a target to become waiting, i.e. not busy, or until we time out.
 * This is useful when clicking on a target to talk to it, and it doesn't
 * reply. This way, we won't lock up.
 *
 * If the target becomes waiting, RESULT is set to 0. If we time out, RESULT is
 * set to 1.
 */

int32 Logic::fnTimedWait(int32 *params) {
	// params:	0 ob_logic
	//		1 target
	//		2 number of cycles before give up

	assert(_vm->_resman->fetchType(params[1]) == GAME_OBJECT);

	ObjectLogic obLogic(decodePtr(params[0]));

	if (obLogic.getLooping() == 0) {
		// This is the first time, so set up the time-out.
		obLogic.setLooping(params[2]);
	}

	// Run the target's get-speech-state script
	runResScript(params[1], 5);

	if (readVar(RESULT) == 1) {
		// The target is waiting, i.e. not busy

		_vm->_debugger->_speechScriptWaiting = 0;

		obLogic.setLooping(0);
		writeVar(RESULT, 0);
		return IR_CONT;
	}

	obLogic.setLooping(obLogic.getLooping() - 1);

	if (obLogic.getLooping() == 0) {
		// Time's up.

		debug(5, "fnTimedWait: Timed out waiting for %d", params[1]);
		_vm->_debugger->_speechScriptWaiting = 0;

		// Clear the event that hasn't been picked up - in theory,
		// none of this should ever happen.

		killAllIdsEvents(params[1]);
		writeVar(RESULT, 1);
		return IR_CONT;
	}

	// Target is busy. Keep trying.

	_vm->_debugger->_speechScriptWaiting = params[1];
	return IR_REPEAT;
}

int32 Logic::fnPlayFx(int32 *params) {
	// params:	0 sample resource id
	//		1 type		(FX_SPOT, FX_RANDOM, FX_LOOP)
	//		2 delay		(0..65535)
	//		3 volume	(0..16)
	//		4 pan		(-16..16)

	// example script:
	//		fnPlayFx (FXWATER, FX_LOOP, 0, 10, 15);
	//		// fx_water is just a local script flag
	//		fx_water = result;
	//		.
	//		.
	//		.
	//		fnStopFx (fx_water);

	int32 res = params[0];
	int32 type = params[1];
	int32 delay = params[2];
	int32 volume = params[3];
	int32 pan = params[4];

	_vm->_sound->queueFx(res, type, delay, volume, pan);
	return IR_CONT;
}

int32 Logic::fnStopFx(int32 *params) {
	// params:	0 position in queue
	if (_vm->_sound->stopFx(params[0]) != RD_OK)
		debug(5, "SFX ERROR: Trying to stop an inactive sound slot");

	return IR_CONT;
}

/**
 * Start a tune playing, to play once or to loop until stopped or next one
 * played.
 */

int32 Logic::fnPlayMusic(int32 *params) {
	// params:	0 tune id
	//		1 loop flag (0 or 1)

	char filename[128];
	bool loopFlag;
	uint32 rv;

	loopFlag = (params[1] == FX_LOOP);

	rv = _vm->_sound->streamCompMusic(params[0], loopFlag);

	if (rv)
		debug(5, "ERROR: streamCompMusic(%s, %d, %d) returned error 0x%.8x", filename, params[0], loopFlag, rv);

	return IR_CONT;
}

int32 Logic::fnStopMusic(int32 *params) {
	// params:	none

	_vm->_sound->stopMusic(false);
	return IR_CONT;
}

int32 Logic::fnSetValue(int32 *params) {
	// temp. function!

	// used for setting far-referenced megaset resource field in mega
	// object, from start script

	// params:	0 pointer to object's mega structure
	//		1 value to set it to

	ObjectMega obMega(decodePtr(params[0]));

	obMega.setMegasetRes(params[1]);
	return IR_CONT;
}

int32 Logic::fnNewScript(int32 *params) {
	// change current script - must be followed by a TERMINATE script
	// directive

	// params:	0 id of script

	writeVar(PLAYER_ACTION, 0);		// must clear this
	logicReplace(params[0]);
	return IR_TERMINATE;
}

/**
 * Like getSync(), but called from scripts. Sets the RESULT variable to
 * the sync value, or 0 if none is found.
 */

int32 Logic::fnGetSync(int32 *params) {
	// params:	none

	int slot = getSync();

	writeVar(RESULT, (slot != -1) ? _syncList[slot].sync : 0);
	return IR_CONT;
}

/**
 * Wait for sync to happen. Sets the RESULT variable to the sync value, once
 * it has been found.
 */

int32 Logic::fnWaitSync(int32 *params) {
	// params:	none

	debug(6, "fnWaitSync: %d waits", readVar(ID));

	int slot = getSync();

	if (slot == -1)
		return IR_REPEAT;

	debug(5, "fnWaitSync: %d got sync %d", readVar(ID), _syncList[slot].sync);
	writeVar(RESULT, _syncList[slot].sync);
	return IR_CONT;
}

int32 Logic::fnRegisterWalkGrid(int32 *params) {
	// params:	none

	warning("fnRegisterWalkGrid() is no longer a valid opcode");
	return IR_CONT;
}

int32 Logic::fnReverseMegaTableAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to animation table

	// Reverse anim
	return _router->megaTableAnimate(
		decodePtr(params[0]),
		decodePtr(params[1]),
		decodePtr(params[2]),
		decodePtr(params[3]),
		true);
}

int32 Logic::fnReverseAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 resource id of animation file

	// Reverse anim
	return _router->doAnimate(
		decodePtr(params[0]),
		decodePtr(params[1]),
		params[2], true);
}

/**
 * Mark this object for killing - to be killed when player leaves this screen.
 * Object reloads and script restarts upon re-entry to screen, which causes
 * this object's startup logic to be re-run every time we enter the screen.
 * "Which is nice."
 *
 * @note Call ONCE from object's logic script, i.e. in startup code, so not
 * re-called every time script frops off and restarts!
 */

int32 Logic::fnAddToKillList(int32 *params) {
	// params:	none
	uint32 id = readVar(ID);

	// DON'T EVER KILL GEORGE!
	if (id == CUR_PLAYER_ID)
		return IR_CONT;

	// Scan the list to see if it's already included

	for (uint32 i = 0; i < _kills; i++) {
		if (_objectKillList[i] == id)
			return IR_CONT;
	}

	assert(_kills < OBJECT_KILL_LIST_SIZE);	// no room at the inn

	_objectKillList[_kills++] = id;

	// "another one bites the dust"

	// When we leave the screen, all these object resources are to be
	// cleaned out of memory and the kill list emptied by doing
	// '_kills = 0', ensuring that all resources are in fact still in
	// memory and, more importantly, closed before killing!

	return IR_CONT;
}

/**
 * Set the standby walk coords to be used by fnWalkToAnim() and
 * fnStandAfterAnim() when the anim header's start/end coords are zero.
 * Useful during development; can stay in final game anyway.
 */

int32 Logic::fnSetStandbyCoords(int32 *params) {
	// params:	0 x-coord
	//		1 y-coord
	//		2 direction (0..7)

	_router->setStandbyCoords(params[0], params[1], params[2]);
	return IR_CONT;
}

int32 Logic::fnBackPar0Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	_router->setSpriteStatus(decodePtr(params[0]), BGP0_SPRITE);
	return IR_CONT;
}

int32 Logic::fnBackPar1Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	_router->setSpriteStatus(decodePtr(params[0]), BGP1_SPRITE);
	return IR_CONT;
}

int32 Logic::fnForePar0Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	_router->setSpriteStatus(decodePtr(params[0]), FGP0_SPRITE);
	return IR_CONT;
}

int32 Logic::fnForePar1Sprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	_router->setSpriteStatus(decodePtr(params[0]), FGP1_SPRITE);
	return IR_CONT;
}

int32 Logic::fnSetPlayerActionEvent(int32 *params) {
	// we want to intercept the player character and have him interact
	// with an object - from script this code is the same as the mouse
	// engine calls when you click on an object - here, a third party
	// does the clicking IYSWIM

	// note - this routine used CUR_PLAYER_ID as the target

	// params:	0 id to interact with

	setPlayerActionEvent(CUR_PLAYER_ID, params[0]);
	return IR_CONT;
}

/**
 * Set the special scroll offset variables
 *
 * Call when starting screens and to change the camera within screens
 *
 * call AFTER fnInitBackground() to override the defaults
 */

int32 Logic::fnSetScrollCoordinate(int32 *params) {
	// params:	0 feet_x value
	//		1 feet_y value

	// Called feet_x and feet_y to retain intellectual compatibility with
	// Sword1!
	//
	// feet_x & feet_y refer to the physical screen coords where the
	// system will try to maintain George's feet

	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	screenInfo->feet_x = params[0];
	screenInfo->feet_y = params[1];
	return IR_CONT;
}

/**
 * Stand mega at start position of anim
 */

int32 Logic::fnStandAtAnim(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 anim resource id

	_router->standAtAnim(
		decodePtr(params[0]),
		decodePtr(params[1]),
		params[2]);
	return IR_CONT;
}

#define SCROLL_MOUSE_WIDTH 20

int32 Logic::fnSetScrollLeftMouse(int32 *params) {
	// params:	0 pointer to object's mouse structure

	byte *ob_mouse = decodePtr(params[0]);
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	// Highest priority

	ObjectMouse mouse;

	mouse.x1 = 0;
	mouse.y1 = 0;
	mouse.x2 = screenInfo->scroll_offset_x + SCROLL_MOUSE_WIDTH;
	mouse.y2 = screenInfo->screen_deep - 1;
	mouse.priority = 0;

	if (screenInfo->scroll_offset_x > 0) {
		// not fully scrolled to the left
		mouse.pointer = SCROLL_LEFT_MOUSE_ID;
	} else {
		// so the mouse area doesn't get registered
		mouse.pointer = 0;
	}

	mouse.write(ob_mouse);
	return IR_CONT;
}

int32 Logic::fnSetScrollRightMouse(int32 *params) {
	// params:	0 pointer to object's mouse structure

	byte *ob_mouse = decodePtr(params[0]);
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	// Highest priority

	ObjectMouse mouse;

	mouse.x1 = screenInfo->scroll_offset_x + _vm->_screen->getScreenWide() - SCROLL_MOUSE_WIDTH;
	mouse.y1 = 0;
	mouse.x2 = screenInfo->screen_wide - 1;
	mouse.y2 = screenInfo->screen_deep - 1;
	mouse.priority = 0;

	if (screenInfo->scroll_offset_x < screenInfo->max_scroll_offset_x) {
		// not fully scrolled to the right
		mouse.pointer = SCROLL_RIGHT_MOUSE_ID;
	} else {
		// so the mouse area doesn't get registered
		mouse.pointer = 0;
	}

	mouse.write(ob_mouse);
	return IR_CONT;
}

int32 Logic::fnColor(int32 *params) {
	// set border color - useful during script development
	// eg. set to color during a timer situation, then black when timed
	// out

	// params	0: color (see defines above)

#ifdef SWORD2_DEBUG
	// what color?
	switch (params[0]) {
	case BLACK:
		_vm->_screen->setPalette(0, 1, black, RDPAL_INSTANT);
		break;
	case WHITE:
		_vm->_screen->setPalette(0, 1, white, RDPAL_INSTANT);
		break;
	case RED:
		_vm->_screen->setPalette(0, 1, red, RDPAL_INSTANT);
		break;
	case GREEN:
		_vm->_screen->setPalette(0, 1, green, RDPAL_INSTANT);
		break;
	case BLUE:
		_vm->_screen->setPalette(0, 1, blue, RDPAL_INSTANT);
		break;
	}
#endif

	return IR_CONT;
}

#ifdef SWORD2_DEBUG
#define BLACK	0
#define WHITE	1
#define RED	2
#define GREEN	3
#define BLUE	4

static const uint8 black[3]	= {  0,    0,   0 };
static const uint8 white[3]	= { 255, 255, 255 };
static const uint8 red[3]	= { 255,   0,   0 };
static const uint8 green[3]	= {   0, 255,   0 };
static const uint8 blue[3]	= {   0,   0, 255 };
#endif

int32 Logic::fnFlash(int32 *params) {
	// flash color 0 (ie. border) - useful during script development
	// eg. fnFlash(BLUE) where a text line is missed; RED when some code
	// missing, etc

	// params:	0 color to flash

#ifdef SWORD2_DEBUG
	// what color?
	switch (params[0]) {
	case WHITE:
		_vm->_screen->setPalette(0, 1, white, RDPAL_INSTANT);
		break;
	case RED:
		_vm->_screen->setPalette(0, 1, red, RDPAL_INSTANT);
		break;
	case GREEN:
		_vm->_screen->setPalette(0, 1, green, RDPAL_INSTANT);
		break;
	case BLUE:
		_vm->_screen->setPalette(0, 1, blue, RDPAL_INSTANT);
		break;
	}

	// There used to be a busy-wait loop here, so I don't know how long
	// the delay was meant to be. Probably doesn't matter much.

	_vm->_screen->updateDisplay();
	_vm->_system->delayMillis(250);
	_vm->_screen->setPalette(0, 1, black, RDPAL_INSTANT);
#endif

	return IR_CONT;
}

int32 Logic::fnPreFetch(int32 *params) {
	// Go fetch resource in the background.

	// params:	0 resource to fetch [guess]

	return IR_CONT;
}

/**
 * Reverse of fnPassPlayerSaveData() - run script 8 of player object.
 */

int32 Logic::fnGetPlayerSaveData(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure

	byte *ob_logic = decodePtr(params[0]);
	byte *ob_graph = decodePtr(params[1]);
	byte *ob_mega = decodePtr(params[2]);

	// Copy from savegame buffers to player object

	memcpy(ob_logic, _saveLogic, ObjectLogic::size());
	memcpy(ob_graph, _saveGraphic, ObjectGraphic::size());
	memcpy(ob_mega, _saveMega, ObjectMega::size());

	// Any walk-data must be cleared - the player will be set to stand if
	// he was walking when saved.

	ObjectMega obMega(ob_mega);

	if (obMega.getIsWalking()) {
		ObjectLogic obLogic(ob_logic);

		obMega.setIsWalking(0);

		int32 pars[3];

		pars[0] = params[1];			// ob_graphic;
		pars[1] = params[2];			// ob_mega
		pars[2] = obMega.getCurDir();

		fnStand(pars);

		// Reset looping flag (which would have been 1 during fnWalk)
		obLogic.setLooping(0);
	}

	return IR_CONT;
}

/**
 * Copies the 4 essential player structures into the savegame header - run
 * script 7 of player object to request this.
 *
 * Remember, we cannot simply read a compact any longer but instead must
 * request it from the object itself.
 */

int32 Logic::fnPassPlayerSaveData(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure

	// Copy from player object to savegame buffers

	memcpy(_saveLogic, decodePtr(params[0]), ObjectLogic::size());
	memcpy(_saveGraphic, decodePtr(params[1]), ObjectGraphic::size());
	memcpy(_saveMega, decodePtr(params[2]), ObjectMega::size());

	return IR_CONT;
}

int32 Logic::fnSendEvent(int32 *params) {
	// we want to intercept the player character and have him interact
	// with an object - from script

	// params:	0 id to receive event
	//		1 script to run

	sendEvent(params[0], params[1]);
	return IR_CONT;
}

/**
 * Add this walkgrid resource to the list of those used for routing in this
 * location. Note that this is ignored if the resource is already in the list.
 */

int32 Logic::fnAddWalkGrid(int32 *params) {
	// params:	0 id of walkgrid resource

	// All objects that add walkgrids must be restarted whenever we
	// re-enter a location.

	// DON'T EVER KILL GEORGE!
	if (readVar(ID) != CUR_PLAYER_ID) {
		// Need to call this in case it wasn't called in script!
		fnAddToKillList(NULL);
	}

	_router->addWalkGrid(params[0]);
	fnPreLoad(params);
	return IR_CONT;
}

/**
 * Remove this walkgrid resource from the list of those used for routing in
 * this location. Note that this is ignored if the resource isn't actually
 * in the list.
 */

int32 Logic::fnRemoveWalkGrid(int32 *params) {
	// params:	0 id of walkgrid resource

	_router->removeWalkGrid(params[0]);
	return IR_CONT;
}

// like fnCheckEventWaiting, but starts the event rather than setting RESULT
// to 1

int32 Logic::fnCheckForEvent(int32 *params) {
	// params:	none

	if (checkEventWaiting()) {
		startEvent();
		return IR_TERMINATE;
	}

	return IR_CONT;
}

// combination of fnPause and fnCheckForEvent
// - ie. does a pause, but also checks for event each cycle

int32 Logic::fnPauseForEvent(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 number of game-cycles to pause

	ObjectLogic obLogic(decodePtr(params[0]));

	if (checkEventWaiting()) {
		obLogic.setLooping(0);
		startEvent();
		return IR_TERMINATE;
	}

	return fnPause(params);
}

int32 Logic::fnClearEvent(int32 *params) {
	// params:	none

	clearEvent(readVar(ID));
	return IR_CONT;
}

int32 Logic::fnFaceMega(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 id of target mega to face

	return _router->faceMega(
		decodePtr(params[0]),
		decodePtr(params[1]),
		decodePtr(params[2]),
		decodePtr(params[3]),
		params[4]);
}

int32 Logic::fnPlaySequence(int32 *params) {
	// params:	0 pointer to null-terminated ascii filename
	//		1 number of frames in the sequence, used for PSX.

	char filename[30];

	// The original code had some #ifdef blocks for skipping or muting the
	// cutscenes - fondly described as "the biggest fudge in the history
	// of computer games" - but at the very least we want to show the
	// cutscene subtitles, so I removed them.

	debug(5, "fnPlaySequence(\"%s\");", (const char *)decodePtr(params[0]));

	// add the appropriate file extension & play it

	strcpy(filename, (const char *)decodePtr(params[0]));

	// Write to walkthrough file (zebug0.txt)
	debug(5, "PLAYING SEQUENCE \"%s\"", filename);

	// don't want to carry on streaming game music when cutscene starts!
	fnStopMusic(NULL);

	// pause sfx during sequence
	_vm->_sound->pauseFx();

	uint32 frameCount = Sword2Engine::isPsx() ? params[1] : 0;

	_moviePlayer = makeMoviePlayer(filename, _vm, _vm->_system, frameCount);

	if (_moviePlayer && _moviePlayer->load(filename)) {
		_moviePlayer->play(_sequenceTextList, _sequenceTextLines, _smackerLeadIn, _smackerLeadOut);
	}

	_sequenceTextLines = 0;

	delete _moviePlayer;
	_moviePlayer = NULL;

	// unpause sound fx again, in case we're staying in same location
	_vm->_sound->unpauseFx();

	_smackerLeadIn = 0;
	_smackerLeadOut = 0;

	// now clear the screen in case the Sequence was quitted (using ESC)
	// rather than fading down to black

	_vm->_screen->clearScene();

	// zero the entire palette in case we're about to fade up!

	byte pal[3 * 256];

	memset(pal, 0, sizeof(pal));
	_vm->_screen->setPalette(0, 256, pal, RDPAL_INSTANT);

	debug(5, "fnPlaySequence FINISHED");
	return IR_CONT;
}

int32 Logic::fnShadedSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	_router->setSpriteShading(decodePtr(params[0]), SHADED_SPRITE);
	return IR_CONT;
}

int32 Logic::fnUnshadedSprite(int32 *params) {
	// params:	0 pointer to object's graphic structure
	_router->setSpriteShading(decodePtr(params[0]), UNSHADED_SPRITE);
	return IR_CONT;
}

int32 Logic::fnFadeUp(int32 *params) {
	// params:	none

	_vm->_screen->waitForFade();

	if (_vm->_screen->getFadeStatus() == RDFADE_BLACK)
		_vm->_screen->fadeUp();

	return IR_CONT;
}

int32 Logic::fnDisplayMsg(int32 *params) {
	// Display a message to the user on the screen.

	// params:	0 Text number of message to be displayed.

	uint32 local_text = params[0] & 0xffff;
	uint32 text_res = params[0] / SIZE;

	// Display message for three seconds.

	// +2 to skip the encoded text number in the first 2 chars; 3 is
	// duration in seconds

	_vm->_screen->displayMsg(_vm->fetchTextLine(_vm->_resman->openResource(text_res), local_text) + 2, 3);
	_vm->_resman->closeResource(text_res);

	return IR_CONT;
}

int32 Logic::fnSetObjectHeld(int32 *params) {
	// params:	0 luggage icon to set
	uint32 res = (uint32)params[0];

	_vm->_mouse->setObjectHeld(res);
	return IR_CONT;
}

int32 Logic::fnAddSequenceText(int32 *params) {
	// params:	0 text number
	//		1 frame number to start the text displaying
	//		2 frame number to stop the text dispalying

	if (!readVar(DEMO)) {
		assert(_sequenceTextLines < MAX_SEQUENCE_TEXT_LINES);

		_sequenceTextList[_sequenceTextLines].reset();
		_sequenceTextList[_sequenceTextLines]._textNumber = params[0];
		_sequenceTextList[_sequenceTextLines]._startFrame = params[1];
		_sequenceTextList[_sequenceTextLines]._endFrame = params[2];
		_sequenceTextLines++;
	}

	return IR_CONT;
}

int32 Logic::fnResetGlobals(int32 *params) {
	// fnResetGlobals is used by the demo - so it can loop back & restart
	// itself

	// params:	none

	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	byte *globals = _vm->_resman->openResource(1) + ResHeader::size();
	int32 size = _vm->_resman->fetchLen(1) - ResHeader::size();

	debug(5, "globals size: %d", size);

	// blank each global variable
	memset(globals, 0, size);

	_vm->_resman->closeResource(1);

	// all objects but george
	_vm->_resman->killAllObjects(false);

	// FOR THE DEMO - FORCE THE SCROLLING TO BE RESET!
	// - this is taken from fnInitBackground

	// switch on scrolling (2 means first time on screen)
	screenInfo->scroll_flag = 2;

	// Used to be IR_CONT, but that's a bad idea. We may just have killed
	// our own script resource -- continuing will cause a bad memory read
	// access.
	return IR_STOP;
}

int32 Logic::fnSetPalette(int32 *params) {
	// params:	0 resource number of palette file, or 0 if it's to be
	//		  the palette from the current screen

	_vm->_screen->setFullPalette(params[0]);
	return IR_CONT;
}

// use this in the object's service script prior to registering the mouse area
// ie. before fnRegisterMouse or fnRegisterFrame
// - best if kept at very top of service script

int32 Logic::fnRegisterPointerText(int32 *params) {
	// params:	0 local id of text line to use as pointer text

	_vm->_mouse->registerPointerText(params[0]);
	return IR_CONT;
}

int32 Logic::fnFetchWait(int32 *params) {
	// Fetches a resource in the background but prevents the script from
	// continuing until the resource is in memory.

	// params:	0 resource to fetch [guess]

	return IR_CONT;
}

int32 Logic::fnRelease(int32 *params) {
	// Releases a resource from memory. Used for freeing memory for
	// sprites that have just been used and will not be used again.
	// Sometimes it is better to kick out a sprite straight away so that
	// the memory can be used for more frequent animations.

	// params:	0 resource to release [guess]

	return IR_CONT;
}

int32 Logic::fnPrepareMusic(int32 *params) {
	// params:	1 id of music to prepare [guess]
	return IR_CONT;
}

int32 Logic::fnSoundFetch(int32 *params) {
	// params:	0 id of sound to fetch [guess]
	return IR_CONT;
}

int32 Logic::fnSmackerLeadIn(int32 *params) {
	// params:	0 id of lead-in music

	// ready for use in fnPlaySequence
	_smackerLeadIn = params[0];
	return IR_CONT;
}

int32 Logic::fnSmackerLeadOut(int32 *params) {
	// params:	0 id of lead-out music

	// ready for use in fnPlaySequence
	_smackerLeadOut = params[0];
	return IR_CONT;
}

/**
 * Stops all FX and clears the entire FX queue.
 */

int32 Logic::fnStopAllFx(int32 *params) {
	// params:	none

	_vm->_sound->clearFxQueue(false);
	return IR_CONT;
}

int32 Logic::fnCheckPlayerActivity(int32 *params) {
	// Used to decide when to trigger music cues described as "no player
	// activity for a while"

	// params:	0 threshold delay in seconds, ie. what we want to
	//		  check the actual delay against

	uint32 seconds = (uint32)params[0];

	_vm->_mouse->checkPlayerActivity(seconds);
	return IR_CONT;
}

int32 Logic::fnResetPlayerActivityDelay(int32 *params) {
	// Use if you want to deliberately reset the "no player activity"
	// counter for any reason

	// params:	none

	_vm->_mouse->resetPlayerActivityDelay();
	return IR_CONT;
}

int32 Logic::fnCheckMusicPlaying(int32 *params) {
	// params:	none

	// sets result to no. of seconds of current tune remaining
	// or 0 if no music playing

	// in seconds, rounded up to the nearest second
	writeVar(RESULT, _vm->_sound->musicTimeRemaining());
	return IR_CONT;
}

int32 Logic::fnPlayCredits(int32 *params) {
	// This function just quits the game if this is the playable demo, ie.
	// credits are NOT played in the demo any more!

	// params:	none

	if (readVar(DEMO)) {
		_vm->quitGame();
		return IR_STOP;
	}

	_vm->_screen->rollCredits();
	return IR_CONT;
}

int32 Logic::fnSetScrollSpeedNormal(int32 *params) {
	// params:	none

	_vm->_screen->setScrollFraction(16);
	return IR_CONT;
}

int32 Logic::fnSetScrollSpeedSlow(int32 *params) {
	// params:	none

	_vm->_screen->setScrollFraction(32);
	return IR_CONT;
}

// Called from speech scripts to remove the chooser bar when it's not
// appropriate to keep it displayed

int32 Logic::fnRemoveChooser(int32 *params) {
	// params:	none

	_vm->_mouse->hideMenu(RDMENU_BOTTOM);
	return IR_CONT;
}

/**
 * Alter the volume and pan of a currently playing FX
 */

int32 Logic::fnSetFxVolAndPan(int32 *params) {
	// params:	0 id of fx (ie. the id returned in 'result' from
	//		  fnPlayFx
	//		1 new volume (0..16)
	//		2 new pan (-16..16)

	debug(5, "fnSetFxVolAndPan(%d, %d, %d)", params[0], params[1], params[2]);

	_vm->_sound->setFxIdVolumePan(params[0], params[1], params[2]);
	return IR_CONT;
}

/**
 * Alter the volume of a currently playing FX
 */

int32 Logic::fnSetFxVol(int32 *params) {
	// params:	0 id of fx (ie. the id returned in 'result' from
	//		  fnPlayFx
	//		1 new volume (0..16)

	_vm->_sound->setFxIdVolumePan(params[0], params[1]);
	return IR_CONT;
}

int32 Logic::fnRestoreGame(int32 *params) {
	// params:	none
	return IR_CONT;
}

int32 Logic::fnRefreshInventory(int32 *params) {
	// Called from 'menu_look_or_combine' script in 'menu_master' object
	// to update the menu to display a combined object while George runs
	// voice-over. Note that 'object_held' must be set to the graphic of
	// the combined object

	// params:	none

	_vm->_mouse->refreshInventory();
	return IR_CONT;
}

int32 Logic::fnChangeShadows(int32 *params) {
	// params:	none
	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	// if last screen was using a shading mask (see below)
	if (screenInfo->mask_flag) {
		uint32 rv = _vm->_screen->closeLightMask();
		if (rv)
			error("Driver Error %.8x", rv);
		screenInfo->mask_flag = false;
	}

	return IR_CONT;
}

} // End of namespace Sword2
