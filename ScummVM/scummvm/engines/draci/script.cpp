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
#include "common/debug.h"
#include "common/memstream.h"
#include "common/stack.h"

#include "draci/draci.h"
#include "draci/animation.h"
#include "draci/barchive.h"
#include "draci/game.h"
#include "draci/mouse.h"
#include "draci/music.h"
#include "draci/screen.h"
#include "draci/script.h"
#include "draci/sound.h"
#include "draci/surface.h"

namespace Draci {

void Script::setupCommandList() {
	/** A table of all the commands the game player uses */
	static const GPL2Command gplCommands[] = {
		{ 0,  0, "gplend",		0, { },					NULL },
		{ 0,  1, "exit",		0, { },					NULL },
		{ 1,  1, "goto", 		1, { kGPL2Ident }, 			&Script::c_Goto },
		{ 2,  1, "Let", 		2, { kGPL2Ident, kGPL2Math }, 		&Script::c_Let },
		{ 3,  1, "if", 			2, { kGPL2Math, kGPL2Ident }, 		&Script::c_If },
		{ 4,  1, "Start", 		2, { kGPL2Ident, kGPL2Str }, 		&Script::start },
		{ 5,  1, "Load", 		2, { kGPL2Ident, kGPL2Str }, 		&Script::load },
		{ 5,  2, "StartPlay", 		2, { kGPL2Ident, kGPL2Str }, 		&Script::startPlay },
		{ 5,  3, "JustTalk", 		0, { }, 				&Script::justTalk },
		{ 5,  4, "JustStay", 		0, { }, 				&Script::justStay },
		{ 6,  1, "Talk", 		2, { kGPL2Ident, kGPL2Str }, 		&Script::talk },
		{ 7,  1, "ObjStat", 		2, { kGPL2Ident, kGPL2Ident }, 		&Script::objStat },
		{ 7,  2, "ObjStat_On", 		2, { kGPL2Ident, kGPL2Ident }, 		&Script::objStatOn },
		{ 8,  1, "IcoStat", 		2, { kGPL2Ident, kGPL2Ident }, 		&Script::icoStat },
		{ 9,  1, "Dialogue", 		1, { kGPL2Str }, 			&Script::dialogue },
		{ 9,  2, "ExitDialogue", 	0, { }, 				&Script::exitDialogue },
		{ 9,  3, "ResetDialogue", 	0, { }, 				&Script::resetDialogue },
		{ 9,  4, "ResetDialogueFrom", 	0, { }, 				&Script::resetDialogueFrom },
		{ 9,  5, "ResetBlock", 		1, { kGPL2Ident }, 			&Script::resetBlock },
		{ 10, 1, "WalkOn", 		3, { kGPL2Num, kGPL2Num, kGPL2Ident }, 	&Script::walkOn },
		{ 10, 2, "StayOn", 		3, { kGPL2Num, kGPL2Num, kGPL2Ident }, 	&Script::stayOn },
		{ 10, 3, "WalkOnPlay", 		3, { kGPL2Num, kGPL2Num, kGPL2Ident }, 	&Script::walkOnPlay },
		{ 11, 1, "LoadPalette", 	1, { kGPL2Str }, 			&Script::loadPalette },
		{ 12, 1, "SetPalette", 		0, { }, 				&Script::setPalette },
		{ 12, 2, "BlackPalette", 	0, { }, 				&Script::blackPalette },
		{ 13, 1, "FadePalette", 	3, { kGPL2Num, kGPL2Num, kGPL2Num }, 	&Script::fadePalette },
		{ 13, 2, "FadePalettePlay", 	3, { kGPL2Num, kGPL2Num, kGPL2Num }, 	&Script::fadePalettePlay },
		{ 14, 1, "NewRoom", 		2, { kGPL2Ident, kGPL2Num }, 		&Script::newRoom },
		{ 15, 1, "ExecInit", 		1, { kGPL2Ident }, 			&Script::execInit },
		{ 15, 2, "ExecLook", 		1, { kGPL2Ident }, 			&Script::execLook },
		{ 15, 3, "ExecUse", 		1, { kGPL2Ident }, 			&Script::execUse },
		{ 18, 1, "LoadMusic", 		1, { kGPL2Str }, 			&Script::loadMusic },
		{ 18, 2, "StartMusic", 		0, { }, 				&Script::startMusic },
		{ 18, 3, "StopMusic", 		0, { }, 				&Script::stopMusic },
		{ 19, 1, "Mark", 		0, { }, 				&Script::mark },
		{ 19, 2, "Release", 		0, { }, 				&Script::release },
		{ 20, 1, "Play", 		0, { }, 				&Script::play },
		{ 21, 1, "LoadMap", 		1, { kGPL2Str }, 			&Script::loadMap },
		{ 21, 2, "RoomMap", 		0, { }, 				&Script::roomMap },
		{ 22, 1, "DisableQuickHero", 	0, { }, 				&Script::disableQuickHero },
		{ 22, 2, "EnableQuickHero", 	0, { }, 				&Script::enableQuickHero },
		{ 23, 1, "DisableSpeedText", 	0, { }, 				&Script::disableSpeedText },
		{ 23, 2, "EnableSpeedText", 	0, { }, 				&Script::enableSpeedText },
		{ 24, 1, "QuitGame", 		0, { }, 				&Script::quitGame },
		{ 25, 1, "PushNewRoom", 	0, { }, 				&Script::pushNewRoom },
		{ 25, 2, "PopNewRoom", 		0, { }, 				&Script::popNewRoom },
		// The following commands are not used in the original game files.
		{ 16, 1, "RepaintInventory", 	0, { },					NULL },
		{ 16, 2, "ExitInventory", 	0, { },					NULL },
		{ 17, 1, "ExitMap", 		0, { },					NULL },
		{ 18, 4, "FadeOutMusic",	1, { kGPL2Num },			NULL },
		{ 18, 5, "FadeInMusic", 	1, { kGPL2Num },			NULL },
		// The following commands are not even defined in the game
		// sources, but their numbers are allocated for internal
		// purposes of the old player.
		{ 26, 1, "ShowCheat", 		0, { },					NULL },
		{ 26, 2, "HideCheat", 		0, { },					NULL },
		{ 26, 3, "ClearCheat", 		1, { kGPL2Num },			NULL },
		{ 27, 1, "FeedPassword", 	3, { kGPL2Num, kGPL2Num, kGPL2Num },	NULL }
	};

	/** Operators used by the mathematical evaluator */
	static const GPL2Operator gplOperators[] = {
		{ &Script::operAnd,            "&" },
		{ &Script::operOr,             "|" },
		{ &Script::operXor,            "^" },
		{ &Script::operEqual,          "==" },
		{ &Script::operNotEqual,       "!=" },
		{ &Script::operLess,           "<" },
		{ &Script::operGreater,        ">" },
		{ &Script::operLessOrEqual,    "<=" },
		{ &Script::operGreaterOrEqual, ">=" },
		{ &Script::operMul,            "*" },
		{ &Script::operDiv,            "/" },
		{ &Script::operMod,            "%" },
		{ &Script::operAdd,            "+" },
		{ &Script::operSub,            "-" }
	};

	/** Functions used by the mathematical evaluator */
	static const GPL2Function gplFunctions[] = {
		{ &Script::funcNot,       "Not" },
		{ &Script::funcRandom,    "Random" },
		{ &Script::funcIsIcoOn,   "IsIcoOn" },
		{ &Script::funcIsIcoAct,  "IsIcoAct" },
		{ &Script::funcIcoStat,   "IcoStat" },
		{ &Script::funcActIco,    "ActIco" },
		{ &Script::funcIsObjOn,   "IsObjOn" },
		{ &Script::funcIsObjOff,  "IsObjOff" },
		{ &Script::funcIsObjAway, "IsObjAway" },
		{ &Script::funcObjStat,   "ObjStat" },
		{ &Script::funcLastBlock, "LastBlock" },
		{ &Script::funcAtBegin,   "AtBegin" },
		{ &Script::funcBlockVar,  "BlockVar" },
		{ &Script::funcHasBeen,   "HasBeen" },
		{ &Script::funcMaxLine,   "MaxLine" },
		{ &Script::funcActPhase,  "ActPhase" },
		// The following function is not even defined in the game
		// sources, but its number is allocated for internal purposes
		// of the old player.
		{ NULL, "Cheat" },
	};

	_commandList = gplCommands;
	_operatorList = gplOperators;
	_functionList = gplFunctions;
}

/** Type of mathematical object */
enum mathExpressionObject {
	kMathEnd,
	kMathNumber,
	kMathOperator,
	kMathFunctionCall,
	kMathVariable
};

/* GPL operators */

int Script::operAnd(int op1, int op2) const {
	return op1 & op2;
}

int Script::operOr(int op1, int op2) const {
	return op1 | op2;
}

int Script::operXor(int op1, int op2) const {
	return op1 ^ op2;
}

int Script::operEqual(int op1, int op2) const {
	return op1 == op2;
}

int Script::operNotEqual(int op1, int op2) const {
	return op1 != op2;
}

int Script::operLess(int op1, int op2) const {
	return op1 < op2;
}

int Script::operGreater(int op1, int op2) const {
	return op1 > op2;
}

int Script::operGreaterOrEqual(int op1, int op2) const {
	return op1 >= op2;
}

int Script::operLessOrEqual(int op1, int op2) const {
	return op1 <= op2;
}

int Script::operMul(int op1, int op2) const {
	return op1 * op2;
}

int Script::operAdd(int op1, int op2) const {
	return op1 + op2;
}

int Script::operSub(int op1, int op2) const {
	return op1 - op2;
}

int Script::operDiv(int op1, int op2) const {
	return op1 / op2;
}

int Script::operMod(int op1, int op2) const {
	return op1 % op2;
}

/* GPL functions */

int Script::funcRandom(int n) const {
// The function needs to return numbers in the [0..n-1] range so we need to deduce 1
// (RandomSource::getRandomNumber returns a number in the range [0..n])

	n -= 1;
	return _vm->_rnd.getRandomNumber(n);
}

int Script::funcAtBegin(int yesno) const {
	return _vm->_game->isDialogueBegin() == (bool)yesno;
}

int Script::funcLastBlock(int blockID) const {
	blockID -= 1;

	return _vm->_game->getDialogueLastBlock() == blockID;
}

int Script::funcBlockVar(int blockID) const {
	blockID -= 1;

	const int currentOffset = _vm->_game->getCurrentDialogueOffset();
	return _vm->_game->getDialogueVar(currentOffset + blockID);
}

int Script::funcHasBeen(int blockID) const {
	blockID -= 1;

	const int currentOffset = _vm->_game->getCurrentDialogueOffset();
	return _vm->_game->getDialogueVar(currentOffset + blockID) > 0;
}

int Script::funcMaxLine(int lines) const {
	return _vm->_game->getDialogueLinesNum() < lines;
}

int Script::funcNot(int n) const {
	return !n;
}

int Script::funcIsIcoOn(int itemID) const {
	itemID -= 1;

	return _vm->_game->getItemStatus(itemID) == 1;
}

int Script::funcIcoStat(int itemID) const {
	itemID -= 1;

	int status = _vm->_game->getItemStatus(itemID);
	return (status == 1) ? 1 : 2;
}

int Script::funcIsIcoAct(int itemID) const {
	itemID -= 1;

	return _vm->_game->getCurrentItem() == _vm->_game->getItem(itemID);
}

int Script::funcActIco(int itemID) const {
	// The parameter seems to be an omission in the original player since it's not
	// used in the implementation of the function. It's possible that the functions were
	// implemented in such a way that they had to have a single parameter so this is only
	// passed as a dummy.

	const GameItem *item = _vm->_game->getCurrentItem();
	return item ? item->_absNum + 1 : 0;
}

int Script::funcIsObjOn(int objID) const {
	objID -= 1;

	const GameObject *obj = _vm->_game->getObject(objID);

	return obj->_visible;
}

int Script::funcIsObjOff(int objID) const {
	objID -= 1;

	const GameObject *obj = _vm->_game->getObject(objID);

	// We index locations from 0 (as opposed to the original player where it was from 1)
	// That's why the "away" location 0 from the data files is converted to -1
	return !obj->_visible && obj->_location != -1;
}

int Script::funcObjStat(int objID) const {
	objID -= 1;

	const GameObject *obj = _vm->_game->getObject(objID);

	if (obj->_location == _vm->_game->getRoomNum()) {
		if (obj->_visible) {
			return 1;   // object is ON (in the room and visible)
		} else {
			return 2;   // object is OFF (in the room, not visible)
		}
	} else {
		return 3;       // object is AWAY (not in the room)
	}
}

int Script::funcIsObjAway(int objID) const {
	objID -= 1;

	const GameObject *obj = _vm->_game->getObject(objID);

	// see Script::funcIsObjOff
	return !obj->_visible && obj->_location == -1;
}

int Script::funcActPhase(int objID) const {
	objID -= 1;

	// Default return value
	int ret = 0;

	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return ret;
	}

	const GameObject *obj = _vm->_game->getObject(objID);

	const bool visible = (obj->_location == _vm->_game->getRoomNum() && obj->_visible);
	if (objID == kDragonObject || visible) {
		const int i = obj->playingAnim();
		if (i >= 0) {
			Animation *anim = obj->_anim[i];
			ret = anim->currentFrameNum();
		}
	}

	return ret;
}

/* GPL commands */

void Script::play(const Common::Array<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	// Runs just one phase of the loop and exits.  Used when waiting for a
	// particular animation phase to come.
	_vm->_game->loop(kInnerUntilExit, true);
}

void Script::load(const Common::Array<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params[0] - 1;
	int animID = params[1] - 1;

	// If the animation is already loaded, return
	GameObject *obj = _vm->_game->getObject(objID);
	if (obj->getAnim(animID) >= 0) {
		return;
	}

	// We don't test here whether an animation is loaded in the
	// AnimationManager while not being registered in the object's array of
	// animations.  This cannot legally happen and an assertion will be
	// thrown by AnimationManager::load().
	obj->addAnim(_vm->_anims->load(animID));
}

void Script::start(const Common::Array<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params[0] - 1;
	int animID = params[1] - 1;

	GameObject *obj = _vm->_game->getObject(objID);
	obj->stopAnim();

	int index = obj->getAnim(animID);
	if (index < 0) {
		// WORKAROUND:
		//
		// The original game files seem to contain errors, which I have
		// verified by inspecting their source code.  They try to load
		// each animation before starting it, but fail to anticipate
		// all possible code paths when game loading comes into play.
		//
		// In particular, if I load the game at the stump location,
		// apply a hedgehog on them, and then talk to them, one of the
		// animations is not loaded.  This animation would have been
		// loaded had I talked to them before applying the hedgehog
		// (because a different dialog init code is run before the
		// application).  Talking to the stumps is necessary to be able
		// to apply the hedgehog, so normal game-play is safe.
		// However, if I save the game after talking to them and load
		// it later, then the game variables are set so as to allow me
		// to apply the hedgehog, but there is no way that the game
		// player would load the requested animation by itself.
		// See objekty:5077 and parezy.txt:27.
		index = obj->addAnim(_vm->_anims->load(animID));
		debugC(1, kDraciBytecodeDebugLevel, "start(%d=%s) cannot find animation %d.  Loading.",
			objID, obj->_title.c_str(), animID);
	}
	Animation *anim = obj->_anim[index];
	anim->registerCallback(&Animation::stop);

	if (objID == kDragonObject) {
		_vm->_game->playHeroAnimation(index);
	} else {
		const bool visible = (obj->_location == _vm->_game->getRoomNum() && obj->_visible);
		if (visible) {
			obj->playAnim(index);
		}
	}
}

void Script::startPlay(const Common::Array<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params[0] - 1;
	int animID = params[1] - 1;

	GameObject *obj = _vm->_game->getObject(objID);
	obj->stopAnim();

	int index = obj->getAnim(animID);
	if (index < 0) {
		index = obj->addAnim(_vm->_anims->load(animID));
		debugC(1, kDraciBytecodeDebugLevel, "startPlay(%d=%s) cannot find animation %d.  Loading.",
			objID, obj->_title.c_str(), animID);
	}
	Animation *anim = obj->_anim[index];
	anim->registerCallback(&Animation::exitGameLoop);

	if (objID == kDragonObject) {
		_vm->_game->playHeroAnimation(index);
	} else {
		const bool visible = (obj->_location == _vm->_game->getRoomNum() && obj->_visible);
		if (visible) {
			obj->playAnim(index);
		}
	}

	// Runs an inner loop until the animation ends.
	_vm->_game->loop(kInnerUntilExit, false);
	obj->stopAnim();

	anim->registerCallback(&Animation::doNothing);
}

void Script::justTalk(const Common::Array<int> &params) {
	const GameObject *dragon = _vm->_game->getObject(kDragonObject);
	const int last_anim = static_cast<Movement> (dragon->playingAnim());
	const int new_anim = (last_anim == kSpeakRight || last_anim == kStopRight) ? kSpeakRight : kSpeakLeft;
	_vm->_game->playHeroAnimation(new_anim);
}

void Script::justStay(const Common::Array<int> &params) {
	const GameObject *dragon = _vm->_game->getObject(kDragonObject);
	const int last_anim = static_cast<Movement> (dragon->playingAnim());
	const int new_anim = (last_anim == kSpeakRight || last_anim == kStopRight) ? kStopRight : kStopLeft;
	_vm->_game->playHeroAnimation(new_anim);
}

void Script::c_If(const Common::Array<int> &params) {
	int expression = params[0];
	int jump = params[1];

	if (expression)
		_jump = jump;
}

void Script::c_Goto(const Common::Array<int> &params) {
	int jump = params[0];

	_jump = jump;
}

void Script::c_Let(const Common::Array<int> &params) {
	int var = params[0] - 1;
	int value = params[1];

	_vm->_game->setVariable(var, value);
}

void Script::loadMusic(const Common::Array<int> &params) {
	int track = params[0];
	_vm->_game->setMusicTrack(track);
}

void Script::startMusic(const Common::Array<int> &params) {
	// If already playing this track, nothing happens.
	_vm->_music->playSMF(_vm->_game->getMusicTrack(), true);
}

void Script::stopMusic(const Common::Array<int> &params) {
	_vm->_music->stop();
	_vm->_game->setMusicTrack(0);
}

void Script::mark(const Common::Array<int> &params) {
	_vm->_game->setMarkedAnimationIndex(_vm->_anims->getLastIndex());
}

void Script::release(const Common::Array<int> &params) {
	int markedIndex = _vm->_game->getMarkedAnimationIndex();

	_vm->_game->deleteAnimationsAfterIndex(markedIndex);
}

void Script::icoStat(const Common::Array<int> &params) {
	int status = params[0];
	int itemID = params[1] - 1;
	GameItem *item = _vm->_game->getItem(itemID);

	_vm->_game->setItemStatus(itemID, status == 1);

	if (!_vm->_game->getItemStatus(itemID)) {
		// Remove the item from the inventory and release its animations.
		_vm->_game->removeItem(item);
		item->_anim->del();
		item->_anim = NULL;

		// If the item was in the hand, remove it from the hands and,
		// if the cursor was set to this item (as opposed to, say, an
		// arrow leading outside a location), set it to standard.
		if (_vm->_game->getCurrentItem() == item) {
			_vm->_game->setCurrentItem(NULL);
			_vm->_game->setPreviousItemPosition(-1);
			if (_vm->_mouse->getCursorType() >= kItemCursor) {
				_vm->_mouse->setCursorType(kNormalCursor);
			}
		}

	} else {
		_vm->_game->loadItemAnimation(item);
		_vm->_game->setCurrentItem(item);
		_vm->_game->setPreviousItemPosition(0);		// next time, try to place the item from the beginning
		_vm->_mouse->loadItemCursor(item, false);
	}
}

void Script::objStatOn(const Common::Array<int> &params) {
	int objID = params[0] - 1;
	int roomID = params[1] - 1;

	GameObject *obj = _vm->_game->getObject(objID);

	obj->_location = roomID;
	obj->_visible = true;
}

void Script::objStat(const Common::Array<int> &params) {
	int status = params[0];
	int objID = params[1] - 1;

	GameObject *obj = _vm->_game->getObject(objID);

	if (status == 1) {
		return;
	} else if (status == 2) {
		obj->_visible = false;
	} else {
		obj->_visible = false;
		obj->_location = -1;
	}

	obj->stopAnim();
}

void Script::execInit(const Common::Array<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params[0] - 1;

	const GameObject *obj = _vm->_game->getObject(objID);
	run(obj->_program, obj->_init);
}

void Script::execLook(const Common::Array<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params[0] - 1;

	const GameObject *obj = _vm->_game->getObject(objID);

	// We don't have to use runWrapper(), because the has already been
	// wrapped due to the fact that these commands are only run from a GPL2
	// program but never from the core player.
	run(obj->_program, obj->_look);
}

void Script::execUse(const Common::Array<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int objID = params[0] - 1;

	const GameObject *obj = _vm->_game->getObject(objID);
	run(obj->_program, obj->_use);
}

void Script::stayOn(const Common::Array<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int x = params[0];
	int y = params[1];
	SightDirection dir = static_cast<SightDirection> (params[2]);

	// Jumps into the given position regardless of the walking map.
	Common::Point heroPos(_vm->_game->findNearestWalkable(x, y));
	Common::Point mousePos(_vm->_mouse->getPosX(), _vm->_mouse->getPosY());
	const GameObject *dragon = _vm->_game->getObject(kDragonObject);
	Movement startingDirection = static_cast<Movement> (dragon->playingAnim());

	_vm->_game->stopWalking();
	_vm->_game->setHeroPosition(heroPos);
	_vm->_game->playHeroAnimation(WalkingState::animationForSightDirection(
		  dir, heroPos, mousePos, WalkingPath(), startingDirection));
}

void Script::walkOn(const Common::Array<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int x = params[0];
	int y = params[1];
	SightDirection dir = static_cast<SightDirection> (params[2]);

	// Constructs an optimal path and starts walking there.  No callback
	// will be called at the end nor will the loop-body exit.
	_vm->_game->stopWalking();
	_vm->_game->walkHero(x, y, dir);
}

void Script::walkOnPlay(const Common::Array<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int x = params[0];
	int y = params[1];
	SightDirection dir = static_cast<SightDirection> (params[2]);

	_vm->_game->stopWalking();
	_vm->_game->walkHero(x, y, dir);

	// Walk in an inner loop until the hero has arrived at the target
	// point.  Then the loop-body will exit.
	_vm->_game->loop(kInnerUntilExit, false);
}

void Script::newRoom(const Common::Array<int> &params) {
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		return;
	}

	int room = params[0] - 1;
	int gate = params[1] - 1;

	_vm->_game->scheduleEnteringRoomUsingGate(room, gate);
}

void Script::talk(const Common::Array<int> &params) {
	int personID = params[0] - 1;
	int sentenceID = params[1] - 1;

	Surface *surface = _vm->_screen->getSurface();

	// Fetch string
	const BAFile *f = _vm->_stringsArchive->getFile(sentenceID);

	// Fetch frame for the speech text
	Animation *speechAnim = _vm->_anims->getAnimation(kSpeechText);
	Text *speechFrame = reinterpret_cast<Text *>(speechAnim->getCurrentFrame());

	// Fetch person info
	const Person *person = _vm->_game->getPerson(personID);

	// Fetch the dubbing
	SoundSample *sample = _vm->_sound->isMutedVoice()
		? NULL : _vm->_dubbingArchive->getSample(sentenceID, 0);

	// Set the string and text color
	surface->markDirtyRect(speechFrame->getRect(kNoDisplacement));
	if (_vm->_sound->showSubtitles() || !sample) {
		speechFrame->setText(Common::String((const char *)f->_data+1, f->_length-1));
	} else {
		speechFrame->setText("");
	}
	speechFrame->setColor(person->_fontColor);
	speechFrame->repeatedlySplitLongLines(kScreenWidth);

	// Speak the dubbing if possible
	uint dubbingDuration = 0;
	if (sample) {
		dubbingDuration = _vm->_sound->playVoice(sample);
		debugC(3, kDraciSoundDebugLevel, "Playing sentence %d: %d+%d with duration %dms",
			sentenceID, sample->_offset, sample->_length, dubbingDuration);
		dubbingDuration += 500;
	}

	// Record time
	int talkSpeed = _vm->_sound->talkSpeed();
	if (!_vm->_game->getEnableSpeedText() && talkSpeed > kStandardSpeed) {
		talkSpeed = kStandardSpeed;
	}
	if (talkSpeed <= 0) {
		talkSpeed = 1;
	}
	uint subtitleDuration;
	if (talkSpeed >= 255) {
		subtitleDuration = 0;
	} else {
		subtitleDuration = (kBaseSpeechDuration + speechFrame->getLength() * kSpeechTimeUnit) / talkSpeed;
	}
	const uint duration = MAX(subtitleDuration, dubbingDuration);
	_vm->_game->setSpeechTiming(_vm->_system->getMillis(), duration);

	// Set speech text coordinates
	int x, y;
	if (_vm->_game->getLoopStatus() == kStatusInventory) {
		x = surface->centerOnX(160, speechFrame->getWidth());
		y = 4;
	} else {
		x = surface->centerOnX(person->_x, speechFrame->getWidth());
		y = surface->putAboveY(person->_y, speechFrame->getHeight());
	}

	speechFrame->setX(x);
	speechFrame->setY(y);

	// Call the game loop to enable interactivity until the text expires.
	_vm->_game->loop(kInnerWhileTalk, false);

	// Delete the text
	_vm->_screen->getSurface()->markDirtyRect(speechFrame->getRect(kNoDisplacement));
	speechFrame->setText("");

	// Stop the playing sample and deallocate it.  Stopping should only be
	// necessary if the user interrupts the playback.
	if (sample) {
		_vm->_sound->stopVoice();
		sample->close();
	}
}

void Script::dialogue(const Common::Array<int> &params) {
	int dialogueID = params[0] - 1;

	_vm->_game->dialogueMenu(dialogueID);
}

void Script::loadMap(const Common::Array<int> &params) {
	int mapID = params[0] - 1;

	_vm->_game->loadWalkingMap(mapID);
}

void Script::resetDialogue(const Common::Array<int> &params) {
	const int currentOffset = _vm->_game->getCurrentDialogueOffset();

	for (int i = 0; i < _vm->_game->getDialogueBlockNum(); ++i) {
		_vm->_game->setDialogueVar(currentOffset + i, 0);
	}
}

void Script::resetDialogueFrom(const Common::Array<int> &params) {
	const int currentOffset = _vm->_game->getCurrentDialogueOffset();

	for (int i = _vm->_game->getDialogueCurrentBlock(); i < _vm->_game->getDialogueBlockNum(); ++i) {
		_vm->_game->setDialogueVar(currentOffset + i, 0);
	}
}

void Script::resetBlock(const Common::Array<int> &params) {
	int blockID = params[0] - 1;

	const int currentOffset = _vm->_game->getCurrentDialogueOffset();

	_vm->_game->setDialogueVar(currentOffset + blockID, 0);
}

void Script::exitDialogue(const Common::Array<int> &params) {
	_vm->_game->setDialogueExit(true);
}

void Script::roomMap(const Common::Array<int> &params) {
	// Load the default walking map for the room
	_vm->_game->loadWalkingMap(_vm->_game->getMapID());
}

void Script::disableQuickHero(const Common::Array<int> &params) {
	_vm->_game->setEnableQuickHero(false);
}

void Script::enableQuickHero(const Common::Array<int> &params) {
	_vm->_game->setEnableQuickHero(true);
}

void Script::disableSpeedText(const Common::Array<int> &params) {
	_vm->_game->setEnableSpeedText(false);
}

void Script::enableSpeedText(const Common::Array<int> &params) {
	_vm->_game->setEnableSpeedText(true);
}

void Script::loadPalette(const Common::Array<int> &params) {
	int palette = params[0] - 1;

	_vm->_game->schedulePalette(palette);
}

void Script::blackPalette(const Common::Array<int> &params) {

	_vm->_game->schedulePalette(kBlackPalette);
}

void Script::fadePalette(const Common::Array<int> &params) {
	// Unused first and last
	int phases = params[2];

	// Let the palette fade in the background while the game continues.
	// Since we don't set substatus to kInnerWhileFade, the outer loop will
	// just continue rather than exit.
	_vm->_game->initializeFading(phases);
}

void Script::fadePalettePlay(const Common::Array<int> &params) {
	// Unused first and last
	int phases = params[2];
	_vm->_game->initializeFading(phases);

	// Call the game loop to enable interactivity until the fading is done.
	_vm->_game->loop(kInnerWhileFade, false);
}

void Script::setPalette(const Common::Array<int> &params) {
	if (_vm->_game->getScheduledPalette() == -1) {
		_vm->_screen->setPalette(NULL, 0, kNumColors);
	} else {
		const BAFile *f;
		f = _vm->_paletteArchive->getFile(_vm->_game->getScheduledPalette());
		_vm->_screen->setPalette(f->_data, 0, kNumColors);
	}
	// Immediately update the palette
	_vm->_screen->copyToScreen();
	_vm->_system->delayMillis(kTimeUnit);
}

void Script::quitGame(const Common::Array<int> &params) {
	_vm->_game->setQuit(true);
}

void Script::pushNewRoom(const Common::Array<int> &params) {
	_vm->_game->pushNewRoom();
}

void Script::popNewRoom(const Common::Array<int> &params) {
	_vm->_game->popNewRoom();
}

/**
 * @brief Evaluates mathematical expressions
 * @param reader Stream reader set to the beginning of the expression
 */
int Script::handleMathExpression(Common::ReadStream *reader) const {
	Common::Stack<int> stk;
	mathExpressionObject obj;
	GPL2Operator oper;
	GPL2Function func;

	debugC(4, kDraciBytecodeDebugLevel, "\t<MATHEXPR>");

	// Read in initial math object
	obj = (mathExpressionObject)reader->readSint16LE();

	int value;
	int arg1, arg2, res;

	while (1) {
		if (obj == kMathEnd) {
			// Check whether the expression was evaluated correctly
			// The stack should contain only one value after the evaluation
			// i.e. the result of the expression
			assert(stk.size() == 1 && "Mathematical expression error");
			break;
		}

		switch (obj) {

		// If the object type is not known, assume that it's a number
		default:
		case kMathNumber:
			value = reader->readSint16LE();
			stk.push(value);
			debugC(4, kDraciBytecodeDebugLevel, "\t\tnumber: %d", value);
			break;

		case kMathOperator:
			value = reader->readSint16LE();
			arg2 = stk.pop();
			arg1 = stk.pop();

			// Fetch operator
			oper = _operatorList[value-1];

			// Calculate result
			res = (this->*(oper._handler))(arg1, arg2);

			// Push result
			stk.push(res);

			debugC(4, kDraciBytecodeDebugLevel, "\t\t%d %s %d (res: %d)",
				arg1, oper._name, arg2, res);
			break;

		case kMathVariable:
			value = reader->readSint16LE() - 1;

			stk.push(_vm->_game->getVariable(value));

			debugC(4, kDraciBytecodeDebugLevel, "\t\tvariable: %d (%d)", value,
				_vm->_game->getVariable(value));
			break;

		case kMathFunctionCall:
			value = reader->readSint16LE();

			// Fetch function
			func = _functionList[value-1];

			// If not yet implemented
			if (func._handler == 0) {
				stk.pop();

				// Pushing dummy value
				stk.push(0);

				debugC(4, kDraciBytecodeDebugLevel, "\t\tcall: %s (not implemented)",
				       func._name);
			} else {
				arg1 = stk.pop();

				// Calculate result
				res = (this->*(func._handler))(arg1);

				// Push the result on the evaluation stack
				stk.push(res);

				debugC(4, kDraciBytecodeDebugLevel, "\t\tcall: %s(%d) (res: %d)",
				       func._name, arg1, res);
			}

			break;
		}

		obj = (mathExpressionObject) reader->readSint16LE();
	}

	return stk.pop();
}

/**
 * @brief Evaluates a GPL mathematical expression on a given offset and returns
 * the result (which is normally a boolean-like value)
 *
 * @param program   A GPL2Program instance of the program containing the expression
 * @param offset    Offset of the expression inside the program (in multiples of 2 bytes)
 *
 * @return The result of the expression converted to a bool.
 *
 * Reference: the function equivalent to this one is called "Can()" in the original engine.
 */
bool Script::testExpression(const GPL2Program &program, uint16 offset) const {
	// Initialize program reader
	Common::MemoryReadStream reader(program._bytecode, program._length);

	// Offset is given as number of 16-bit integers so we need to convert
	// it to a number of bytes
	offset -= 1;
	offset *= 2;

	// Seek to the expression
	reader.seek(offset);

	debugC(4, kDraciBytecodeDebugLevel,
	       "Evaluating (standalone) GPL expression at offset %d:", offset);

	return (bool)handleMathExpression(&reader);
}

/**
 * @brief Find the current command in the internal table
 *
 * @param num       Command number
 * @param subnum    Command subnumer
 *
 * @return NULL if command is not found. Otherwise, a pointer to a GPL2Command
 *         struct representing the command.
 */
const GPL2Command *Script::findCommand(byte num, byte subnum) const {
	uint i = 0;
	while (1) {

		// Command not found
		if (i >= kNumCommands) {
			break;
		}

		// Return found command
		if (_commandList[i]._number == num &&
			_commandList[i]._subNumber == subnum) {
			return &_commandList[i];
		}

		++i;
	}

	return NULL;
}

/**
 * @brief GPL2 bytecode interpreter
 * @param program GPL program in the form of a GPL2Program struct
 *        offset Offset into the program where execution should begin
 *
 * GPL2 is short for Game Programming Language 2 which is the script language
 * used by Draci Historie. This is the interpreter for the language.
 *
 * A compiled GPL2 program consists of a stream of bytes representing commands
 * and their parameters. The syntax is as follows:
 *
 * Syntax of a command:
 *  <name of the command> <number> <sub-number> <list of parameters...>
 *
 * Syntax of a parameter:
 *  - 1: integer number literally passed to the program
 *  - 2-1: string stored in the reservouir of game strings (i.e. something to be
 *    displayed) and stored as an index in this list
 *  - 2-2: string resolved by the compiler (i.e., a path to another file) and
 *    replaced by an integer index of this entity in the appropriate namespace
 *    (e.g., the index of the palette, location, ...)
 *  - 3-0: relative jump to a label defined in this code.  Each label must be
 *    first declared in the beginning of the program.
 *  - 3-1 .. 3-9: index of an entity in several namespaces, defined in file ident
 *  - 4: mathematical expression compiled into a postfix format
 *
 *  In the compiled program, parameters of type 1..3 are represented by a single
 *  16-bit integer.  The called command knows by its definition what namespace the
 *  value comes from.
 */

void Script::run(const GPL2Program &program, uint16 offset) {
	if (shouldEndProgram()) {
		// This might get set by some GPL commands via Script::endCurrentProgram()
		// if they need a program to stop midway.  This flag is sticky until cleared
		// at the top level.
		return;
	}

	int oldJump = _jump;

	// Mark the last animation index before we do anything so a Release command
	// doesn't unload too many animations if we forget to use a Mark command first
	_vm->_game->setMarkedAnimationIndex(_vm->_anims->getLastIndex());

	// Stream reader for the whole program
	Common::MemoryReadStream reader(program._bytecode, program._length);

	// Parameter queue that is passed to each command
	Common::Array<int> params;

	// Offset is given as number of 16-bit integers so we need to convert
	// it to a number of bytes
	offset -= 1;
	offset *= 2;

	// Seek to the requested part of the program
	reader.seek(offset);

	debugC(3, kDraciBytecodeDebugLevel,
		"Starting GPL program at offset %d (program length: %d)", offset, program._length);

	const GPL2Command *cmd;
	do {

		// Account for GPL jump that some commands set
		if (_jump != 0) {
			debugC(3, kDraciBytecodeDebugLevel,
				"Jumping from offset %d to %d (%d bytes)",
				reader.pos(), reader.pos() + _jump, _jump);
			reader.seek(_jump, SEEK_CUR);
		}

		// Reset jump
		_jump = 0;

		// Clear any parameters left on the stack from the previous command
		// This likely won't be needed once all commands are implemented
		params.clear();

		// read in command pair
		uint16 cmdpair = reader.readUint16BE();

		// extract high byte, i.e. the command number
		byte num = (cmdpair >> 8) & 0xFF;

		// extract low byte, i.e. the command subnumber
		byte subnum = cmdpair & 0xFF;

		if ((cmd = findCommand(num, subnum))) {
			int tmp;

			// Print command name
			debugC(1, kDraciBytecodeDebugLevel, "%s", cmd->_name);

			for (int i = 0; i < cmd->_numParams; ++i) {
				if (cmd->_paramTypes[i] == kGPL2Math) {
					debugC(3, kDraciBytecodeDebugLevel,
					    "Evaluating (in-script) GPL expression at offset %d: ", offset);
					params.push_back(handleMathExpression(&reader));
				} else {
					tmp = reader.readSint16LE();
					params.push_back(tmp);
					debugC(2, kDraciBytecodeDebugLevel, "\t%d", tmp);
				}
			}
		} else {
			error("Unknown opcode %d, %d", num, subnum);
		}

		GPLHandler handler = cmd->_handler;

		if (handler != 0) {
			// Call the handler for the current command
			(this->*(cmd->_handler))(params);
		}

	} while (cmd->_number != 0 && !shouldEndProgram());    // 0 = gplend and exit

	_jump = oldJump;

	// Reset the flags which may have temporarily been altered inside the script.
	_vm->_game->setEnableQuickHero(true);
	_vm->_game->setEnableSpeedText(true);
}

void Script::runWrapper(const GPL2Program &program, uint16 offset, bool disableCursor, bool releaseAnims) {
	if (disableCursor) {
		// Fetch the dedicated objects' title animation / current frame
		Animation *titleAnim = _vm->_anims->getAnimation(kTitleText);
		titleAnim->markDirtyRect(_vm->_screen->getSurface());
		Text *title = reinterpret_cast<Text *>(titleAnim->getCurrentFrame());
		title->setText("");

		_vm->_mouse->cursorOff();
	}

	// Mark last animation
	int lastAnimIndex = _vm->_anims->getLastIndex();

	run(program, offset);

	if (releaseAnims) {
			_vm->_game->deleteAnimationsAfterIndex(lastAnimIndex);
	}

	if (disableCursor) {
		_vm->_mouse->cursorOn();
	}
}

} // End of namespace Draci
