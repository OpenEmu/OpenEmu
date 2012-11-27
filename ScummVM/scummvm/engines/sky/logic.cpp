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
#include "common/rect.h"
#include "common/textconsole.h"

#include "sky/autoroute.h"
#include "sky/compact.h"
#include "sky/control.h"
#include "sky/debug.h"
#include "sky/disk.h"
#include "sky/grid.h"
#include "sky/logic.h"
#include "sky/mouse.h"
#include "sky/music/musicbase.h"
#include "sky/text.h"
#include "sky/screen.h"
#include "sky/sky.h"
#include "sky/sound.h"
#include "sky/struc.h"

namespace Sky {

uint32 Logic::_scriptVariables[NUM_SKY_SCRIPTVARS];

void Logic::setupLogicTable() {
	static const LogicTable logicTable[] = {
		&Logic::nop,
		&Logic::logicScript,	 // 1  script processor
		&Logic::autoRoute,	 // 2  Make a route
		&Logic::arAnim,	 // 3  Follow a route
		&Logic::arTurn,	 // 4  Mega turns araound
		&Logic::alt,		 // 5  Set up new get-to script
		&Logic::anim,	 // 6  Follow a sequence
		&Logic::turn,	 // 7  Mega turning
		&Logic::cursor,	 // 8  id tracks the pointer
		&Logic::talk,	 // 9  count down and animate
		&Logic::listen,	 // 10 player waits for talking id
		&Logic::stopped,	 // 11 wait for id to move
		&Logic::choose,	 // 12 wait for player to click
		&Logic::frames,	 // 13 animate just frames
		&Logic::pause,	 // 14 Count down to 0 and go
		&Logic::waitSync,	 // 15 Set to l_script when sync!=0
		&Logic::simpleAnim,	 // 16 Module anim without x,y's
	};

	_logicTable = logicTable;
}

Logic::Logic(SkyCompact *skyCompact, Screen *skyScreen, Disk *skyDisk, Text *skyText, MusicBase *skyMusic, Mouse *skyMouse, Sound *skySound)
	: _rnd("sky") {

	_skyCompact = skyCompact;
	_skyScreen = skyScreen;
	_skyDisk = skyDisk;
	_skyText = skyText;
	_skyMusic = skyMusic;
	_skySound = skySound;
	_skyMouse = skyMouse;
	_skyGrid = new Grid(_skyDisk, _skyCompact);
	_skyAutoRoute = new AutoRoute(_skyGrid, _skyCompact);

	setupLogicTable();
	setupMcodeTable();

	memset(_objectList, 0, 30 * sizeof(uint32));

	for (int i = 0; i < ARRAYSIZE(_moduleList); i++)
		_moduleList[i] = 0;
	_stackPtr = 0;

	_currentSection = 0xFF; //force music & sound reload
	initScriptVariables();
}

Logic::~Logic() {
	delete _skyGrid;
	delete _skyAutoRoute;

	for (int i = 0; i < ARRAYSIZE(_moduleList); i++)
		if (_moduleList[i])
			free(_moduleList[i]);
}

void Logic::initScreen0() {
	fnEnterSection(0, 0, 0);
	_skyMusic->startMusic(2);
	SkyEngine::_systemVars.currentMusic = 2;
}

void Logic::parseSaveData(uint32 *data) {
	if (!SkyEngine::isDemo())
		fnLeaveSection(_scriptVariables[CUR_SECTION], 0, 0);
	for (uint16 cnt = 0; cnt < NUM_SKY_SCRIPTVARS; cnt++)
		_scriptVariables[cnt] = READ_LE_UINT32(data++);
	fnEnterSection(_scriptVariables[CUR_SECTION], 0, 0);
}

bool Logic::checkProtection() {
	if (_scriptVariables[ENTER_DIGITS]) {
		if (_scriptVariables[CONSOLE_TYPE] == 5) // reactor code
			_scriptVariables[FS_COMMAND] = 240;
		else									 // copy protection
			_scriptVariables[FS_COMMAND] = 337;
		_scriptVariables[ENTER_DIGITS] = 0;
		return true;
	} else
		return false;
}

void Logic::engine() {
	do {
		uint16 *logicList = (uint16 *)_skyCompact->fetchCpt(_scriptVariables[LOGIC_LIST_NO]);

		while (uint16 id = *logicList++) { // 0 means end of list
			if (id == 0xffff) {
				// Change logic data address
				logicList = (uint16 *)_skyCompact->fetchCpt(*logicList);
				continue;
			}

			_scriptVariables[CUR_ID] = id;
			_compact = _skyCompact->fetchCpt(id);

			// check the id actually wishes to be processed
			if (!(_compact->status & (1 << 6)))
				continue;

			// ok, here we process the logic bit system

			if (_compact->status & (1 << 7))
				_skyGrid->removeObjectFromWalk(_compact);

			Debug::logic(_compact->logic);
			(this->*_logicTable[_compact->logic]) ();

			if (_compact->status & (1 << 7))
				_skyGrid->objectToWalk(_compact);

			// a sync sent to the compact is available for one cycle
			// only. that cycle has just ended so remove the sync.
			// presumably the mega has just reacted to it.
			_compact->sync = 0;
		}
		// usually this loop is run only once, it'll only be run a second time if the game
		// script just asked the user to enter a copy protection code.
		// this is done to prevent the copy protection screen from flashing up.
		// (otherwise it would be visible for 1/50 second)
	} while (checkProtection());
}

void Logic::nop() {}

/**
 * This function is basicly a wrapper around the real script engine. It runs
 * the script engine until a script has finished.
 * @see script()
 */
void Logic::logicScript() {
	/// Process the current mega's script
	/// If the script finishes then drop back a level

	for (;;) {
		uint16 mode = _compact->mode; // get pointer to current script
		uint16 *scriptNo = SkyCompact::getSub(_compact, mode);
		uint16 *offset   = SkyCompact::getSub(_compact, mode + 2);

		*offset = script(*scriptNo, *offset);

		if (!*offset) // script finished
			_compact->mode -= 4;
		else if (_compact->mode == mode)
			return;
	}
}

void Logic::autoRoute() {

	_compact->downFlag = _skyAutoRoute->autoRoute(_compact);
	if ((_compact->downFlag == 2) && _skyCompact->cptIsId(_compact, CPT_JOEY) &&
	   (_compact->mode == 0) && (_compact->baseSub == JOEY_OUT_OF_LIFT)) {
		   // workaround for script bug #1064113. Details unclear...
		   _compact->downFlag = 0;
	}
	if (_compact->downFlag != 1) { // route ok
		_compact->grafixProgId = _compact->animScratchId;
		_compact->grafixProgPos = 0;
	}

	_compact->logic = L_SCRIPT; // continue the script

	logicScript();
	return;
}

void Logic::arAnim() {
	/// Follow a route
	/// Mega should be in getToMode

	// only check collisions on character boundaries
	if ((_compact->xcood & 7) || (_compact->ycood & 7)) {
		mainAnim();
		return;
	}

	// On character boundary. Have we been told to wait?
	// if not - are WE colliding?

	if (_compact->waitingFor == 0xffff) { // 1st cycle of re-route does not require collision checks
		mainAnim();
		return;
	}

	if (_compact->waitingFor) {
		// ok, we've been told we've hit someone
		// we will wait until we are no longer colliding
		// with them. here we check to see if we are (still) colliding.
		// if we are then run the stop script. if not clear the flag
		// and continue.

		// remember - this could be the first ar cycle for some time,
		// we might have been told to wait months ago. if we are
		// waiting for one person then another hits us then
		// c_waiting_for will be replaced by the new mega - this is
		// fine because the later collision will almost certainly
		// take longer to clear than the earlier one.

		if (collide(_skyCompact->fetchCpt(_compact->waitingFor))) {
			stopAndWait();
			return;
		}

		// we are not in fact hitting this person so clr & continue
		// it must have registered some time ago

		_compact->waitingFor = 0; // clear id flag
	}

	// ok, our turn to check for collisions

	uint16 *logicList = (uint16 *)_skyCompact->fetchCpt(_scriptVariables[LOGIC_LIST_NO]);
	Compact *cpt = 0;

	while (uint16 id = *logicList++) { // get an id

		if (id == 0xffff) { // address change?
			logicList = (uint16 *)_skyCompact->fetchCpt(*logicList); // get new logic list
			continue;
		}

		if (id == (uint16)(_scriptVariables[CUR_ID] & 0xffff)) // is it us?
			continue;

		_scriptVariables[HIT_ID] = id; // save target id for any possible c_mini_bump
		cpt = _skyCompact->fetchCpt(id); // let's have a closer look

		if (!(cpt->status & (1 << ST_COLLISION_BIT))) // can it collide?
			continue;

		if (cpt->screen != _compact->screen) // is it on our screen?
			continue;

		if (collide(cpt)) { // check for a hit
			// ok, we've hit a mega
			// is it moving... or something else?

			if (cpt->logic != L_AR_ANIM) { // check for following route
				// it is doing something else
				// we restart our get-to script
				// first tell it to wait for us - in case it starts moving
				// ( *it may have already hit us and stopped to wait )

				_compact->waitingFor = 0xffff; // effect 1 cycle collision skip
				// tell it it is waiting for us
				cpt->waitingFor = (uint16)(_scriptVariables[CUR_ID] & 0xffff);
				// restart current script
				*SkyCompact::getSub(_compact, _compact->mode + 2) = 0;
				_compact->logic = L_SCRIPT;
				logicScript();
				return;
			}

			script(_compact->miniBump, 0);
			return;
		}
	}

	// ok, there was no collisions
	// now check for interaction request
	// *note: the interaction is always set up as an action script

	if (_compact->request) {
		_compact->mode = C_ACTION_MODE; // put into action mode
		_compact->actionSub = _compact->request;
		_compact->actionSub_off = 0;
		_compact->request = 0; // trash request
		_compact->logic = L_SCRIPT;
		logicScript();
		return;
	}

	// any flag? - or any change?
	// if change then re-run the current script, which must be
	// a position independent get-to		 ----

	if (!_compact->atWatch) { // any flag set?
		mainAnim();
		return;
	}

	// ok, there is an at watch - see if it's changed

	if (_compact->atWas == _scriptVariables[_compact->atWatch/4]) { // still the same?
		mainAnim();
		return;
	}

	// changed so restart the current script
	// *not suitable for base initiated ARing
	*SkyCompact::getSub(_compact, _compact->mode + 2) = 0;

	_compact->logic = L_SCRIPT;
	logicScript();
}

void Logic::mainAnim() {
	/// Extension of arAnim()
	_compact->waitingFor = 0; // clear possible zero-zero skip

	uint16 *sequence = _skyCompact->getGrafixPtr(_compact);
	if (!*sequence) {
		// ok, move to new anim segment
		sequence += 2;
		_compact->grafixProgPos += 2;
		if (!*sequence) { // end of route?
			// ok, sequence has finished

			// will start afresh if new sequence continues in last direction
			_compact->arAnimIndex = 0;

			_compact->downFlag = 0; // pass back ok to script
			_compact->logic = L_SCRIPT;
			logicScript();
			return;
		}

		_compact->arAnimIndex = 0; // reset position
	}

	uint16 dir;
	while ((dir = _compact->dir) != *(sequence + 1)) {
		// ok, setup turning
		_compact->dir = *(sequence + 1);

		uint16 *tt = _skyCompact->getTurnTable(_compact, dir);
		if (tt[_compact->dir]) {
			_compact->turnProgId = tt[_compact->dir];
			_compact->turnProgPos = 0;
			_compact->logic = L_AR_TURNING;
			arTurn();
			return;
		}
	};

	uint16 animId = *(uint16 *)_skyCompact->getCompactElem(_compact, C_ANIM_UP + _compact->megaSet + dir * 4);
	uint16 *animList = (uint16 *)_skyCompact->fetchCpt(animId);

	uint16 arAnimIndex = _compact->arAnimIndex;
	if (!animList[arAnimIndex / 2]) {
		 arAnimIndex = 0;
		_compact->arAnimIndex = 0; // reset
	}

	_compact->arAnimIndex += S_LENGTH;

	*sequence       -= animList[(S_COUNT + arAnimIndex)/2]; // reduce the distance to travel
	_compact->frame  = animList[(S_FRAME + arAnimIndex)/2]; // new graphic frame
	_compact->xcood += animList[(S_AR_X  + arAnimIndex)/2]; // update x coordinate
	_compact->ycood += animList[(S_AR_Y  + arAnimIndex)/2]; // update y coordinate
}

void Logic::arTurn() {
	uint16 *turnData = (uint16 *)_skyCompact->fetchCpt(_compact->turnProgId) + _compact->turnProgPos;
	_compact->frame = *turnData++;
	_compact->turnProgPos++;

	if (!*turnData) { // turn done?
		// Back to ar mode
		_compact->arAnimIndex = 0;
		_compact->logic = L_AR_ANIM;
	}
}

void Logic::alt() {
	/// change the current script
	_compact->logic = L_SCRIPT;
	*SkyCompact::getSub(_compact, _compact->mode) = _compact->alt;
	*SkyCompact::getSub(_compact, _compact->mode + 2) = 0;
	logicScript();
}

void Logic::anim() {
	/// Follow an animation sequence
	uint16 *grafixProg = _skyCompact->getGrafixPtr(_compact);

	while (*grafixProg) {
		_compact->grafixProgPos += 3; // all types are 3 words.
		if (*grafixProg == LF_START_FX) { // do fx
			grafixProg++;
			uint16 sound = *grafixProg++;
			uint16 volume = *grafixProg++;

			// channel 0
			fnStartFx(sound, 0, volume);
		} else if (*grafixProg >= LF_START_FX) { // do sync
			grafixProg++;

			Compact *cpt = _skyCompact->fetchCpt(*grafixProg++);

			cpt->sync = *grafixProg++;
		} else { // put coordinates and frame in
			_compact->xcood = *grafixProg++;
			_compact->ycood = *grafixProg++;

			_compact->frame = *grafixProg++ | _compact->offset;
			return;
		}
	}

	_compact->downFlag = 0;
	_compact->logic = L_SCRIPT;
	logicScript();
}

void Logic::turn() {
	uint16 *turnData = (uint16 *)_skyCompact->fetchCpt(_compact->turnProgId) + _compact->turnProgPos;
	if (*turnData) {
		_compact->frame = *turnData;
		_compact->turnProgPos++;
		return;
	}

	// turn_to_script:
	_compact->arAnimIndex = 0;
	_compact->logic = L_SCRIPT;

	logicScript();
}

void Logic::cursor() {
	_skyText->logicCursor(_compact, _skyMouse->giveMouseX(), _skyMouse->giveMouseY());
}

static uint16 clickTable[46] = {
	ID_FOSTER,
	ID_JOEY,
	ID_JOBS,
	ID_LAMB,
	ID_ANITA,
	ID_SON,
	ID_DAD,
	ID_MONITOR,
	ID_SHADES,
	MINI_SS,
	FULL_SS,
	ID_FOREMAN,
	ID_RADMAN,
	ID_GALLAGER_BEL,
	ID_BURKE,
	ID_BODY,
	ID_HOLO,
	ID_TREVOR,
	ID_ANCHOR,
	ID_WRECK_GUARD,
	ID_SKORL_GUARD,

	// BASE LEVEL
	ID_SC30_HENRI,
	ID_SC31_GUARD,
	ID_SC32_VINCENT,
	ID_SC32_GARDENER,
	ID_SC32_BUZZER,
	ID_SC36_BABS,
	ID_SC36_BARMAN,
	ID_SC36_COLSTON,
	ID_SC36_GALLAGHER,
	ID_SC36_JUKEBOX,
	ID_DANIELLE,
	ID_SC42_JUDGE,
	ID_SC42_CLERK,
	ID_SC42_PROSECUTION,
	ID_SC42_JOBSWORTH,

	// UNDERWORLD
	ID_MEDI,
	ID_WITNESS,
	ID_GALLAGHER,
	ID_KEN,
	ID_SC76_ANDROID_2,
	ID_SC76_ANDROID_3,
	ID_SC81_FATHER,
	ID_SC82_JOBSWORTH,

	// LINC WORLD
	ID_HOLOGRAM_B,
	12289
};

void Logic::talk() {
	// first count through the frames
	// just frames - nothing tweeky
	// the speech finishes when the timer runs out &
	// not when the animation finishes
	// this routine is very task specific

	// TODO: Check for mouse clicking

	// Are we allowed to click

	if (_skyMouse->wasClicked())
		for (int i = 0; i < ARRAYSIZE(clickTable); i++)
			if (clickTable[i] == (uint16)_scriptVariables[CUR_ID]) {
				if ((SkyEngine::_systemVars.systemFlags & SF_ALLOW_SPEECH) && (!_skySound->speechFinished()))
					_skySound->stopSpeech();
				if ((_compact->spTextId > 0) &&
					(_compact->spTextId < 0xFFFF)) {

					_skyCompact->fetchCpt(_compact->spTextId)->status = 0;
				}
				if (_skyCompact->getGrafixPtr(_compact)) {
					_compact->frame = _compact->getToFlag; // set character to stand
					_compact->grafixProgId = 0;
				}

				_compact->logic = L_SCRIPT;
				logicScript();
				return;
			}

	// If speech is allowed then check for it to finish before finishing animations

	if ((_compact->spTextId == 0xFFFF) && // is this a voc file?
		(_skySound->speechFinished())) { // finished?

		_compact->logic = L_SCRIPT; // restart character control

		if (_skyCompact->getGrafixPtr(_compact)) {
			_compact->frame = _compact->getToFlag; // set character to stand
			_compact->grafixProgId = 0;
		}

		logicScript();
		return;
	}

	uint16 *graphixProg = _skyCompact->getGrafixPtr(_compact);
	if (graphixProg) {
		if ((*graphixProg) && ((_compact->spTime != 3) || (!_skySound->speechFinished()))) {
			// we will force the animation to finish 3 game cycles
			// before the speech actually finishes - because it looks good.

			_compact->frame = *(graphixProg + 2) + _compact->offset;
			graphixProg += 3;
			_compact->grafixProgPos += 3;
		} else {
			// we ran out of frames or finished speech, let actor stand still.
			_compact->frame = _compact->getToFlag;
			_compact->grafixProgId = 0;
		}
	}

	if (_skySound->speechFinished()) _compact->spTime--;

	if (_compact->spTime == 0) {

		// ok, speech has finished

		if (_compact->spTextId) {
			Compact *cpt = _skyCompact->fetchCpt(_compact->spTextId); // get text id to kill
			cpt->status = 0; // kill the text
		}

		_compact->logic = L_SCRIPT;
		logicScript();
	}
}

void Logic::listen() {
	/// Stay in this mode until id in getToFlag leaves L_TALK mode

	Compact *cpt = _skyCompact->fetchCpt(_compact->flag);

	if (cpt->logic == L_TALK)
		return;

	_compact->logic = L_SCRIPT;
	logicScript();
}

void Logic::stopped() {
	/// waiting for another mega to move or give-up trying
	///
	/// this mode will always be set up from a special script
	/// that will be one level higher than the script we
	/// would wish to restart from

	Compact *cpt = _skyCompact->fetchCpt(_compact->waitingFor);

	if (cpt)
		if (!cpt->mood && collide(cpt))
			return;

	// we are free, continue processing the script

	// restart script one level below
	*SkyCompact::getSub(_compact, _compact->mode - 2) = 0;
	_compact->waitingFor = 0xffff;

	_compact->logic = L_SCRIPT;
	logicScript();
}

void Logic::choose() {
	// Remain in this mode until player selects some text
	if (!_scriptVariables[THE_CHOSEN_ONE])
		return;

	fnNoHuman(0, 0, 0); // kill mouse again

	SkyEngine::_systemVars.systemFlags &= ~SF_CHOOSING; // restore save/restore

	_compact->logic = L_SCRIPT; // and continue script
	logicScript();
}

void Logic::frames() {
	if (!_compact->sync)
		simpleAnim();
	else {
		_compact->downFlag = 0; // return 'ok' to script
		_compact->logic = L_SCRIPT;
		logicScript();
	}
}

void Logic::pause() {
	if (--_compact->flag)
		return;

	_compact->logic = L_SCRIPT;
	logicScript();
	return;
}

void Logic::waitSync() {
	/// checks c_sync, when its non 0
	/// the id is put back into script mode
	// use this instead of loops in the script

	if (!_compact->sync)
		return;

	_compact->logic = L_SCRIPT;
	logicScript();
}

void Logic::simpleAnim() {
	/// follow an animation sequence module whilst ignoring the coordinate data

	uint16 *grafixProg = _skyCompact->getGrafixPtr(_compact);

	// *grafix_prog: command
	while (*grafixProg) {
		_compact->grafixProgPos += 3;
		if (*grafixProg != SEND_SYNC) {
			grafixProg++;
			grafixProg++; // skip coordinates

			// *grafix_prog: frame
			if (*grafixProg >= 64)
				_compact->frame = *grafixProg;
			else
				_compact->frame = *grafixProg + _compact->offset;

			return;
		}

		grafixProg++;
		// *grafix_prog: id to sync
		Compact *compact2 = _skyCompact->fetchCpt(*grafixProg);
		grafixProg++;

		// *grafix_prog: sync
		compact2->sync = *grafixProg;
		grafixProg++;
	}

	_compact->downFlag = 0; // return 'ok' to script
	_compact->logic = L_SCRIPT;
	logicScript();
}

bool Logic::collide(Compact *cpt) {
	MegaSet *m1 = SkyCompact::getMegaSet(_compact);
	MegaSet *m2 = SkyCompact::getMegaSet(cpt);

	// target's base coordinates
	uint16 x = cpt->xcood & 0xfff8;
	uint16 y = cpt->ycood & 0xfff8;

	// The collision is direction dependent
	switch (_compact->dir) {
	case 0: // looking up
		x -= m1->colOffset; // compensate for inner x offsets
		x += m2->colOffset;

		if ((x + m2->colWidth) < _compact->xcood) // their rightmost
			return false;

		x -= m1->colWidth; // our left, their right
		if (x >= _compact->xcood)
			return false;

		y += 8; // bring them down a line
		if (y == _compact->ycood)
			return true;

		y += 8; // bring them down a line
		if (y == _compact->ycood)
			return true;

		return false;
	case 1: // looking down
		x -= m1->colOffset; // compensate for inner x offsets
		x += m2->colOffset;

		if ((x + m2->colWidth) < _compact->xcood) // their rightmoast
			return false;

		x -= m1->colWidth; // our left, their right
		if (x >= _compact->xcood)
			return false;

		y -= 8; // bring them up a line
		if (y == _compact->ycood)
			return true;

		y -= 8; // bring them up a line
		if (y == _compact->ycood)
			return true;

		return false;
	case 2: // looking left

		if (y != _compact->ycood)
			return false;

		x += m2->lastChr;
		if (x == _compact->xcood)
			return true;

		x -= 8; // out another one
		if (x == _compact->xcood)
			return true;

		return false;
	case 3: // looking right
	case 4: // talking (not sure if this makes sense...)

		if (y != _compact->ycood)
			return false;

		x -= m1->lastChr; // last block
		if (x == _compact->xcood)
			return true;

		x -= 8; // out another block
		if (x != _compact->xcood)
			return false;

		return true;
	default:
		error("Unknown Direction: %d", _compact->dir);
	}
}

void Logic::runGetOff() {
	uint32 getOff = _scriptVariables[GET_OFF];
	_scriptVariables[GET_OFF] = 0;
	if (getOff)
		script((uint16)(getOff & 0xffff), (uint16)(getOff >> 16));
}

void Logic::stopAndWait() {
	_compact->mode += 4;

	uint16 *scriptNo = SkyCompact::getSub(_compact, _compact->mode);
	uint16 *offset   = SkyCompact::getSub(_compact, _compact->mode + 2);

	*scriptNo = _compact->stopScript;
	*offset   = 0;

	_compact->logic = L_SCRIPT;
	logicScript();
}

void Logic::checkModuleLoaded(uint16 moduleNo) {
	if (!_moduleList[moduleNo])
		_moduleList[moduleNo] = (uint16 *)_skyDisk->loadFile((uint16)moduleNo + F_MODULE_0);
}

void Logic::push(uint32 a) {
	if (_stackPtr > ARRAYSIZE(_stack) - 2)
		error("Stack overflow");
	_stack[_stackPtr++] = a;
}

uint32 Logic::pop() {
	if (_stackPtr < 1 || _stackPtr > ARRAYSIZE(_stack) - 1)
		error("No items on Stack to pop");
	return _stack[--_stackPtr];
}

void Logic::setupMcodeTable() {
	static const McodeTable mcodeTable[] = {
		&Logic::fnCacheChip,
		&Logic::fnCacheFast,
		&Logic::fnDrawScreen,
		&Logic::fnAr,
		&Logic::fnArAnimate,
		&Logic::fnIdle,
		&Logic::fnInteract,
		&Logic::fnStartSub,
		&Logic::fnTheyStartSub,
		&Logic::fnAssignBase,
		&Logic::fnDiskMouse,
		&Logic::fnNormalMouse,
		&Logic::fnBlankMouse,
		&Logic::fnCrossMouse,
		&Logic::fnCursorRight,
		&Logic::fnCursorLeft,
		&Logic::fnCursorDown,
		&Logic::fnOpenHand,
		&Logic::fnCloseHand,
		&Logic::fnGetTo,
		&Logic::fnSetToStand,
		&Logic::fnTurnTo,
		&Logic::fnArrived,
		&Logic::fnLeaving,
		&Logic::fnSetAlternate,
		&Logic::fnAltSetAlternate,
		&Logic::fnKillId,
		&Logic::fnNoHuman,
		&Logic::fnAddHuman,
		&Logic::fnAddButtons,
		&Logic::fnNoButtons,
		&Logic::fnSetStop,
		&Logic::fnClearStop,
		&Logic::fnPointerText,
		&Logic::fnQuit,
		&Logic::fnSpeakMe,
		&Logic::fnSpeakMeDir,
		&Logic::fnSpeakWait,
		&Logic::fnSpeakWaitDir,
		&Logic::fnChooser,
		&Logic::fnHighlight,
		&Logic::fnTextKill,
		&Logic::fnStopMode,
		&Logic::fnWeWait,
		&Logic::fnSendSync,
		&Logic::fnSendFastSync,
		&Logic::fnSendRequest,
		&Logic::fnClearRequest,
		&Logic::fnCheckRequest,
		&Logic::fnStartMenu,
		&Logic::fnUnhighlight,
		&Logic::fnFaceId,
		&Logic::fnForeground,
		&Logic::fnBackground,
		&Logic::fnNewBackground,
		&Logic::fnSort,
		&Logic::fnNoSpriteEngine,
		&Logic::fnNoSpritesA6,
		&Logic::fnResetId,
		&Logic::fnToggleGrid,
		&Logic::fnPause,
		&Logic::fnRunAnimMod,
		&Logic::fnSimpleMod,
		&Logic::fnRunFrames,
		&Logic::fnAwaitSync,
		&Logic::fnIncMegaSet,
		&Logic::fnDecMegaSet,
		&Logic::fnSetMegaSet,
		&Logic::fnMoveItems,
		&Logic::fnNewList,
		&Logic::fnAskThis,
		&Logic::fnRandom,
		&Logic::fnPersonHere,
		&Logic::fnToggleMouse,
		&Logic::fnMouseOn,
		&Logic::fnMouseOff,
		&Logic::fnFetchX,
		&Logic::fnFetchY,
		&Logic::fnTestList,
		&Logic::fnFetchPlace,
		&Logic::fnCustomJoey,
		&Logic::fnSetPalette,
		&Logic::fnTextModule,
		&Logic::fnChangeName,
		&Logic::fnMiniLoad,
		&Logic::fnFlushBuffers,
		&Logic::fnFlushChip,
		&Logic::fnSaveCoods,
		&Logic::fnPlotGrid,
		&Logic::fnRemoveGrid,
		&Logic::fnEyeball,
		&Logic::fnCursorUp,
		&Logic::fnLeaveSection,
		&Logic::fnEnterSection,
		&Logic::fnRestoreGame,
		&Logic::fnRestartGame,
		&Logic::fnNewSwingSeq,
		&Logic::fnWaitSwingEnd,
		&Logic::fnSkipIntroCode,
		&Logic::fnBlankScreen,
		&Logic::fnPrintCredit,
		&Logic::fnLookAt,
		&Logic::fnLincTextModule,
		&Logic::fnTextKill2,
		&Logic::fnSetFont,
		&Logic::fnStartFx,
		&Logic::fnStopFx,
		&Logic::fnStartMusic,
		&Logic::fnStopMusic,
		&Logic::fnFadeDown,
		&Logic::fnFadeUp,
		&Logic::fnQuitToDos,
		&Logic::fnPauseFx,
		&Logic::fnUnPauseFx,
		&Logic::fnPrintf
	};

	_mcodeTable = mcodeTable;
}

static const uint32 forwardList1b[] = {
	JOBS_SPEECH,
	JOBS_S4,
	JOBS_ALARMED,
	JOEY_RECYCLE,
	SHOUT_SSS,
	JOEY_MISSION,
	TRANS_MISSION,
	SLOT_MISSION,
	CORNER_MISSION,
	JOEY_LOGIC,
	GORDON_SPEECH,
	JOEY_BUTTON_MISSION,
	LOB_DAD_SPEECH,
	LOB_SON_SPEECH,
	GUARD_SPEECH,
	MANTRACH_SPEECH,
	WRECK_SPEECH,
	ANITA_SPEECH,
	LAMB_FACTORY,
	FORE_SPEECH,
	JOEY_42_MISS,
	JOEY_JUNCTION_MISS,
	WELDER_MISSION,
	JOEY_WELD_MISSION,
	RADMAN_SPEECH,
	LINK_7_29,
	LINK_29_7,
	LAMB_TO_3,
	LAMB_TO_2,
	BURKE_SPEECH,
	BURKE_1,
	BURKE_2,
	DR_BURKE_1,
	JASON_SPEECH,
	JOEY_BELLEVUE,
	ANCHOR_SPEECH,
	ANCHOR_MISSION,
	JOEY_PC_MISSION,
	HOOK_MISSION,
	TREVOR_SPEECH,
	JOEY_FACTORY,
	HELGA_SPEECH,
	JOEY_HELGA_MISSION,
	GALL_BELLEVUE,
	GLASS_MISSION,
	LAMB_FACT_RETURN,
	LAMB_LEAVE_GARDEN,
	LAMB_START_29,
	LAMB_BELLEVUE,
	CABLE_MISSION,
	FOSTER_TOUR,
	LAMB_TOUR,
	FOREMAN_LOGIC,
	LAMB_LEAVE_FACTORY,
	LAMB_BELL_LOGIC,
	LAMB_FACT_2,
	START90,
	0,
	0,
	LINK_28_31,
	LINK_31_28,
	EXIT_LINC,
	DEATH_SCRIPT
};

static uint32 forwardList1b288[] = {
	JOBS_SPEECH,
	JOBS_S4,
	JOBS_ALARMED,
	JOEY_RECYCLE,
	SHOUT_SSS,
	JOEY_MISSION,
	TRANS_MISSION,
	SLOT_MISSION,
	CORNER_MISSION,
	JOEY_LOGIC,
	GORDON_SPEECH,
	JOEY_BUTTON_MISSION,
	LOB_DAD_SPEECH,
	LOB_SON_SPEECH,
	GUARD_SPEECH,
	0x68,
	WRECK_SPEECH,
	ANITA_SPEECH,
	LAMB_FACTORY,
	FORE_SPEECH,
	JOEY_42_MISS,
	JOEY_JUNCTION_MISS,
	WELDER_MISSION,
	JOEY_WELD_MISSION,
	RADMAN_SPEECH,
	LINK_7_29,
	LINK_29_7,
	LAMB_TO_3,
	LAMB_TO_2,
	0x3147,
	0x3100,
	0x3101,
	0x3102,
	0x3148,
	0x3149,
	0x314A,
	0x30C5,
	0x30C6,
	0x30CB,
	0x314B,
	JOEY_FACTORY,
	0x314C,
	0x30E2,
	0x314D,
	0x310C,
	LAMB_FACT_RETURN,
	0x3139,
	0x313A,
	0x004F,
	CABLE_MISSION,
	FOSTER_TOUR,
	LAMB_TOUR,
	FOREMAN_LOGIC,
	LAMB_LEAVE_FACTORY,
	0x3138,
	LAMB_FACT_2,
	0x004D,
	0,
	0,
	LINK_28_31,
	LINK_31_28,
	0x004E,
	DEATH_SCRIPT
};

static const uint32 forwardList2b[] = {
	STD_ON,
	STD_EXIT_LEFT_ON,
	STD_EXIT_RIGHT_ON,
	ADVISOR_188,
	SHOUT_ACTION,
	MEGA_CLICK,
	MEGA_ACTION
};

static const uint32 forwardList3b[] = {
	DANI_SPEECH,
	DANIELLE_GO_HOME,
	SPUNKY_GO_HOME,
	HENRI_SPEECH,
	BUZZER_SPEECH,
	FOSTER_VISIT_DANI,
	DANIELLE_LOGIC,
	JUKEBOX_SPEECH,
	VINCENT_SPEECH,
	EDDIE_SPEECH,
	BLUNT_SPEECH,
	DANI_ANSWER_PHONE,
	SPUNKY_SEE_VIDEO,
	SPUNKY_BARK_AT_FOSTER,
	SPUNKY_SMELLS_FOOD,
	BARRY_SPEECH,
	COLSTON_SPEECH,
	GALL_SPEECH,
	BABS_SPEECH,
	CHUTNEY_SPEECH,
	FOSTER_ENTER_COURT
};

static const uint32 forwardList4b[] = {
	WALTER_SPEECH,
	JOEY_MEDIC,
	JOEY_MED_LOGIC,
	JOEY_MED_MISSION72,
	KEN_LOGIC,
	KEN_SPEECH,
	KEN_MISSION_HAND,
	SC70_IRIS_OPENED,
	SC70_IRIS_CLOSED,
	FOSTER_ENTER_BOARDROOM,
	BORED_ROOM,
	FOSTER_ENTER_NEW_BOARDROOM,
	HOBS_END,
	SC82_JOBS_SSS
};

static const uint32 forwardList5b[] = {
	SET_UP_INFO_WINDOW,
	SLAB_ON,
	UP_MOUSE,
	DOWN_MOUSE,
	LEFT_MOUSE,
	RIGHT_MOUSE,
	DISCONNECT_FOSTER
};

void Logic::fnExec(uint16 num, uint32 a, uint32 b, uint32 c) {
	(this->*_mcodeTable[num])(a, b, c);
}

void Logic::initScriptVariables() {
	for (int i = 0; i < ARRAYSIZE(_scriptVariables); i++)
		_scriptVariables[i] = 0;

	_scriptVariables[LOGIC_LIST_NO] = 141;
	_scriptVariables[LAMB_GREET] = 62;
	_scriptVariables[JOEY_SECTION] = 1;
	_scriptVariables[LAMB_SECTION] = 2;
	_scriptVariables[S15_FLOOR] = 8371;
	_scriptVariables[GUARDIAN_THERE] = 1;
	_scriptVariables[DOOR_67_68_FLAG] = 1;
	_scriptVariables[SC70_IRIS_FLAG] = 3;
	_scriptVariables[DOOR_73_75_FLAG] = 1;
	_scriptVariables[SC76_CABINET1_FLAG] = 1;
	_scriptVariables[SC76_CABINET2_FLAG] = 1;
	_scriptVariables[SC76_CABINET3_FLAG] = 1;
	_scriptVariables[DOOR_77_78_FLAG] = 1;
	_scriptVariables[SC80_EXIT_FLAG] = 1;
	_scriptVariables[SC31_LIFT_FLAG] = 1;
	_scriptVariables[SC32_LIFT_FLAG] = 1;
	_scriptVariables[SC33_SHED_DOOR_FLAG] = 1;
	_scriptVariables[BAND_PLAYING] = 1;
	_scriptVariables[COLSTON_AT_TABLE] = 1;
	_scriptVariables[SC36_NEXT_DEALER] = 16731;
	_scriptVariables[SC36_DOOR_FLAG] = 1;
	_scriptVariables[SC37_DOOR_FLAG] = 2;
	_scriptVariables[SC40_LOCKER_1_FLAG] = 1;
	_scriptVariables[SC40_LOCKER_2_FLAG] = 1;
	_scriptVariables[SC40_LOCKER_3_FLAG] = 1;
	_scriptVariables[SC40_LOCKER_4_FLAG] = 1;
	_scriptVariables[SC40_LOCKER_5_FLAG] = 1;

	if (SkyEngine::_systemVars.gameVersion == 288)
		memcpy(_scriptVariables + 352, forwardList1b288, sizeof(forwardList1b288));
	else
		memcpy(_scriptVariables + 352, forwardList1b, sizeof(forwardList1b));

	memcpy(_scriptVariables + 656, forwardList2b, sizeof(forwardList2b));
	memcpy(_scriptVariables + 721, forwardList3b, sizeof(forwardList3b));
	memcpy(_scriptVariables + 663, forwardList4b, sizeof(forwardList4b));
	memcpy(_scriptVariables + 505, forwardList5b, sizeof(forwardList5b));
}

uint16 Logic::mouseScript(uint32 scrNum, Compact *scriptComp) {
	Compact *tmpComp = _compact;
	_compact = scriptComp;
	uint16 retVal = script((uint16)(scrNum & 0xFFFF), (uint16)(scrNum >> 16));
	_compact = tmpComp;

	if (scrNum == MENU_SELECT || (scrNum >= LINC_MENU_SELECT && scrNum <= DOC_MENU_SELECT)) {
		// HACK: See patch #1689516 for details. The short story:
		// The user has clicked on an inventory item.  We update the
		// mouse cursor instead of waiting for the script to update it.
		// In the original game the cursor is just updated when the mouse
		// moves away the item, but it's unintuitive.
		fnCrossMouse(0, 0, 0);
	}

	return retVal;
}

/**
 * This is the actual script engine.  It interprets script \a scriptNo starting at \a offset
 *
 * @param scriptNo The script to interpret.
 *	 @arg Bits 0-11 - Script number
 *	 @arg Bits 12-15 - Module number
 * @param offset At which offset to start interpreting the script.
 *
 * @return 0 if script finished. Else offset where to continue.
 */
uint16 Logic::script(uint16 scriptNo, uint16 offset) {
	do {
		bool restartScript = false;

		/// process a script
		/// low level interface to interpreter

		uint16 moduleNo = scriptNo >> 12;
		uint16 *scriptData = _moduleList[moduleNo]; // get module address

		if (!scriptData) { // We need to load the script module
			_moduleList[moduleNo] = _skyDisk->loadScriptFile(moduleNo + F_MODULE_0);
			 scriptData = _moduleList[moduleNo]; // module has been loaded
		}

		uint16 *moduleStart = scriptData;

		debug(3, "Doing Script: %d:%d:%x", moduleNo, scriptNo & 0xFFF, offset ? (offset - moduleStart[scriptNo & 0xFFF]) : 0);

		// WORKAROUND for bug #3149412: "Invalid Mode when giving shades to travel agent"
		// Using the dark glasses on Trevor (travel agent) multiple times in succession would
		// wreck the trevor compact's mode, as the script in question doesn't account for using
		// this item at this point in the game (you will only have it here if you play the game
		// in an unusual way) and thus would loop indefinitely / never drop out.
		// To prevent this, we trigger the generic response by pretending we're using an item
		// which the script /does/ handle.
		if (scriptNo == TREVOR_SPEECH && _scriptVariables[OBJECT_HELD] == IDO_SHADES)
			_scriptVariables[OBJECT_HELD] = IDO_GLASS;


		// Check whether we have an offset or what
		if (offset)
			scriptData = moduleStart + offset;
		else
			scriptData += scriptData[scriptNo & 0x0FFF];

		uint32 a = 0, b = 0, c = 0;
		uint16 command, s;

		while(!restartScript) {
			command = *scriptData++; // get a command
			Debug::script(command, scriptData);

			switch (command) {
			case 0: // push_variable
				push( _scriptVariables[*scriptData++ / 4] );
				break;
			case 1: // less_than
				a = pop();
				b = pop();
				if (a > b)
					push(1);
				else
					push(0);
				break;
			case 2: // push_number
				push(*scriptData++);
				break;
			case 3: // not_equal
				a = pop();
				b = pop();
				if (a != b)
					push(1);
				else
					push(0);
				break;
			case 4: // if_and
				a = pop();
				b = pop();
				if (a && b)
					push(1);
				else
					push(0);
				break;
			case 5: // skip_zero
				s = *scriptData++;

				a = pop();
				if (!a)
					scriptData += s / 2;
				break;
			case 6: // pop_var
				b = _scriptVariables[*scriptData++ / 4] = pop();
				break;
			case 7: // minus
				a = pop();
				b = pop();
				push(b-a);
				break;
			case 8: // plus
				a = pop();
				b = pop();
				push(b+a);
				break;
			case 9: // skip_always
				s = *scriptData++;
				scriptData += s / 2;
				break;
			case 10: // if_or
				a = pop();
				b = pop();
				if (a || b)
					push(1);
				else
					push(0);
				break;
			case 11: // call_mcode
				{
					a = *scriptData++;
					assert(a <= 3);
					// No, I did not forget the "break"s
					switch (a) {
					case 3:
						c = pop();
					case 2:
						b = pop();
					case 1:
						a = pop();
					}

					uint16 mcode = *scriptData++ / 4; // get mcode number
					Debug::mcode(mcode, a, b, c);

					Compact *saveCpt = _compact;
					bool ret = (this->*_mcodeTable[mcode]) (a, b, c);
					_compact = saveCpt;

					if (!ret)
						return (scriptData - moduleStart);
				}
				break;
			case 12: // more_than
				a = pop();
				b = pop();
				if (a < b)
					push(1);
				else
					push(0);
				break;
			case 14: // switch
				c = s = *scriptData++; // get number of cases

				a = pop(); // and value to switch on

				do {
					if (a == *scriptData) {
						scriptData += scriptData[1] / 2;
						scriptData++;
						break;
					}
					scriptData += 2;
				} while (--s);

				if (s == 0)
					scriptData += *scriptData / 2; // use the default
				break;
			case 15: // push_offset
				push( *(uint16 *)_skyCompact->getCompactElem(_compact, *scriptData++) );
				break;
			case 16: // pop_offset
				// pop a value into a compact
				*(uint16 *)_skyCompact->getCompactElem(_compact, *scriptData++) = (uint16)pop();
				break;
			case 17: // is_equal
				a = pop();
				b = pop();
				if (a == b)
					push(1);
				else
					push(0);
				break;
			case 18: { // skip_nz
					int16 t = *scriptData++;
					a = pop();
					if (a)
						scriptData += t / 2;
					break;
				}
			case 13:
			case 19: // script_exit
				return 0;
			case 20: // restart_script
				offset = 0;
				restartScript = true;
				break;
			default:
				error("Unknown script command: %d", command);
			}
		}
	} while (true);
}

bool Logic::fnCacheChip(uint32 a, uint32 b, uint32 c) {
	_skySound->fnStopFx();
	_skyDisk->fnCacheChip((uint16 *)_skyCompact->fetchCpt((uint16)a));
	return true;
}

bool Logic::fnCacheFast(uint32 a, uint32 b, uint32 c) {
	_skyDisk->fnCacheFast((uint16 *)_skyCompact->fetchCpt((uint16)a));
	return true;
}

bool Logic::fnDrawScreen(uint32 a, uint32 b, uint32 c) {
	debug(5, "Call: fnDrawScreen(%X, %X)",a,b);
	SkyEngine::_systemVars.currentPalette = a;
	_skyScreen->fnDrawScreen(a, b);

	if (Logic::_scriptVariables[SCREEN] == 32) {
		/* workaround for script bug #786482
		    Under certain circumstances, which never got completely cleared,
		    the gardener can get stuck in an animation, waiting for a sync
		    signal from foster.
			This is most probably caused by foster leaving the screen before
			sending the sync.
			To work around that, we simply send a sync to the gardener every time
			we enter the screen. If he isn't stuck (and thus not waiting for sync)
			it will be ignored anyways */

		debug(1, "sending gardener sync");
		fnSendSync(ID_SC32_GARDENER, 1, 0);
	}
	return true;
}

bool Logic::fnAr(uint32 x, uint32 y, uint32 c) {
	_compact->downFlag = 1; // assume failure in-case logic is interupted by speech (esp Joey)

	_compact->arTargetX = (uint16)x;
	_compact->arTargetY = (uint16)y;
	_compact->logic = L_AR; // Set to AR mode

	_compact->xcood &= 0xfff8;
	_compact->ycood &= 0xfff8;

	return false; // drop out of script
}

bool Logic::fnArAnimate(uint32 a, uint32 b, uint32 c) {
	_compact->mood = 0; // high level 'not stood still'
	_compact->logic = L_AR_ANIM;
	return false; // drop out of script
}

bool Logic::fnIdle(uint32 a, uint32 b, uint32 c) {
	// set the player idling
	_compact->logic = 0;
	return true;
}

bool Logic::fnInteract(uint32 targetId, uint32 b, uint32 c) {
	_compact->mode += 4; // next level up
	_compact->logic = L_SCRIPT;
	Compact *cpt = _skyCompact->fetchCpt(targetId);

	*SkyCompact::getSub(_compact, _compact->mode) = cpt->actionScript;
	*SkyCompact::getSub(_compact, _compact->mode + 2) = 0;

	return false;
}

bool Logic::fnStartSub(uint32 scr, uint32 b, uint32 c) {
	_compact->mode += 4;
	*SkyCompact::getSub(_compact, _compact->mode) = (uint16)(scr & 0xffff);
	*SkyCompact::getSub(_compact, _compact->mode + 2) = (uint16)(scr >> 16);
	return false;
}

bool Logic::fnTheyStartSub(uint32 mega, uint32 scr, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(mega);
	cpt->mode += 4;
	*SkyCompact::getSub(cpt, cpt->mode) = (uint16)(scr & 0xffff);
	*SkyCompact::getSub(cpt, cpt->mode + 2) = (uint16)(scr >> 16);
	return true;
}

bool Logic::fnAssignBase(uint32 id, uint32 scr, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(id);
	cpt->mode = C_BASE_MODE;
	cpt->logic = L_SCRIPT;
	cpt->baseSub     = (uint16)(scr & 0xffff);
	cpt->baseSub_off = (uint16)(scr >> 16);
	return true;
}

bool Logic::fnDiskMouse(uint32 a, uint32 b, uint32 c) {
	_skyMouse->spriteMouse(MOUSE_DISK, 11, 11);
	return true;
}

bool Logic::fnNormalMouse(uint32 a, uint32 b, uint32 c) {
	_skyMouse->spriteMouse(MOUSE_NORMAL, 0, 0);
	return true;
}

bool Logic::fnBlankMouse(uint32 a, uint32 b, uint32 c) {
	_skyMouse->spriteMouse(MOUSE_BLANK, 0, 0);
	return true;
}

bool Logic::fnCrossMouse(uint32 a, uint32 b, uint32 c) {
	if (_scriptVariables[OBJECT_HELD])
		_skyMouse->fnOpenCloseHand(false);
	else
		_skyMouse->spriteMouse(MOUSE_CROSS, 4, 4);
	return true;
}

bool Logic::fnCursorRight(uint32 a, uint32 b, uint32 c) {
	_skyMouse->spriteMouse(MOUSE_RIGHT, 9, 4);
	return true;
}

bool Logic::fnCursorLeft(uint32 a, uint32 b, uint32 c) {
	_skyMouse->spriteMouse(MOUSE_LEFT, 0, 5);
	return true;
}

bool Logic::fnCursorDown(uint32 a, uint32 b, uint32 c) {
	_skyMouse->spriteMouse(MOUSE_DOWN, 9, 4);
	return true;
}

bool Logic::fnCursorUp(uint32 a, uint32 b, uint32 c) {
	_skyMouse->spriteMouse(MOUSE_UP, 9, 4);
	return true;
}

bool Logic::fnOpenHand(uint32 a, uint32 b, uint32 c) {
	_skyMouse->fnOpenCloseHand(true);
	return true;
}

bool Logic::fnCloseHand(uint32 a, uint32 b, uint32 c) {
	_skyMouse->fnOpenCloseHand(false);
	return true;
}

bool Logic::fnGetTo(uint32 targetPlaceId, uint32 mode, uint32 c) {
	_compact->upFlag = (uint16)mode; // save mode for action script
	_compact->mode += 4; // next level up
	Compact *cpt = _skyCompact->fetchCpt(_compact->place);
	if (!cpt) {
		warning("can't find _compact's getToTable. Place compact is NULL");
		return false;
	}
	uint16 *getToTable = (uint16 *)_skyCompact->fetchCpt(cpt->getToTableId);
	if (!getToTable) {
		warning("Place compact's getToTable is NULL");
		return false;
	}

	while (*getToTable != targetPlaceId)
		getToTable += 2;

	// get new script
	*SkyCompact::getSub(_compact, _compact->mode) = *(getToTable + 1);
	*SkyCompact::getSub(_compact, _compact->mode + 2) = 0;

	return false; // drop out of script
}

bool Logic::fnSetToStand(uint32 a, uint32 b, uint32 c) {
	_compact->mood = 1; // high level stood still

	_compact->grafixProgId = *(uint16 *)_skyCompact->getCompactElem(_compact, C_STAND_UP + _compact->megaSet + _compact->dir * 4);
	_compact->grafixProgPos = 0;

	uint16 *standList = _skyCompact->getGrafixPtr(_compact);

	_compact->offset = *standList; // get frames offset
	_compact->logic = L_SIMPLE_MOD;
	_compact->grafixProgPos++;
	simpleAnim();
	return false; // drop out of script
}

bool Logic::fnTurnTo(uint32 dir, uint32 b, uint32 c) {
	/// turn compact to direction dir

	uint16 curDir = _compact->dir; // get current direction
	_compact->dir = (uint16)(dir & 0xffff); // set new direction

	uint16 *tt = _skyCompact->getTurnTable(_compact, curDir);

	if (!tt[dir])
		return true; // keep going

	_compact->turnProgId = tt[dir]; // put turn program in
	_compact->turnProgPos = 0;
	_compact->logic = L_TURNING;

	turn();

	return false; // drop out of script
}

bool Logic::fnArrived(uint32 scriptVar, uint32 b, uint32 c) {
	_compact->leaving = (uint16)(scriptVar & 0xffff);
	_scriptVariables[scriptVar/4]++;
	return true;
}

bool Logic::fnLeaving(uint32 a, uint32 b, uint32 c) {
	_compact->atWatch = 0;

	if (_compact->leaving) {
		_scriptVariables[_compact->leaving/4]--;
		_compact->leaving = 0; // I shall do this only once
	}

	return true; // keep going
}

bool Logic::fnSetAlternate(uint32 scr, uint32 b, uint32 c) {
	_compact->alt = (uint16)(scr & 0xffff);
	_compact->logic = L_ALT;
	return false;
}

bool Logic::fnAltSetAlternate(uint32 target, uint32 scr, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(target);
	cpt->alt = (uint16)(scr & 0xffff);
	cpt->logic = L_ALT;
	return false;
}

bool Logic::fnKillId(uint32 id, uint32 b, uint32 c) {
	if (id) {
		Compact *cpt = _skyCompact->fetchCpt(id);
		if (cpt->status & (1 << 7))
			_skyGrid->removeObjectFromWalk(cpt);
		cpt->status = 0;
	}
	return true;
}

bool Logic::fnNoHuman(uint32 a, uint32 b, uint32 c) {
	if (!_scriptVariables[MOUSE_STOP]) {
		_scriptVariables[MOUSE_STATUS] &= 1;
		runGetOff();
		fnBlankMouse(0, 0, 0);
	}
	return true;
}

bool Logic::fnAddHuman(uint32 a, uint32 b, uint32 c) {
	return _skyMouse->fnAddHuman();
}

bool Logic::fnAddButtons(uint32 a, uint32 b, uint32 c) {
	_scriptVariables[MOUSE_STATUS] |= 4;
	return true;
}

bool Logic::fnNoButtons(uint32 a, uint32 b, uint32 c) {
	//remove the mouse buttons
	_scriptVariables[MOUSE_STATUS] &= 0xFFFFFFFB;
	return true;
}

bool Logic::fnSetStop(uint32 a, uint32 b, uint32 c) {
	_scriptVariables[MOUSE_STOP] |= 1;
	return true;
}

bool Logic::fnClearStop(uint32 a, uint32 b, uint32 c) {
	_scriptVariables[MOUSE_STOP] = 0;
	return true;
}

bool Logic::fnPointerText(uint32 a, uint32 b, uint32 c) {
	_skyText->fnPointerText(a, _skyMouse->giveMouseX(), _skyMouse->giveMouseY());
	return true;
}

bool Logic::fnQuit(uint32 a, uint32 b, uint32 c) {
	return false;
}

bool Logic::fnSpeakMe(uint32 targetId, uint32 mesgNum, uint32 animNum) {
	/* WORKAROUND for #2687172: When Mrs. Piermont is talking
	   on the phone in her apartment, ignore her fnSpeakMe calls
	   on other screens, as the lack of speech files for these lines
	   will cause Foster's speech to be aborted if the timing is bad.
	*/
	if (targetId == 0x4039 && animNum == 0x9B && Logic::_scriptVariables[SCREEN] != 38) {
		return false;
	}

	stdSpeak(_skyCompact->fetchCpt(targetId), mesgNum, animNum, 0);
	return false;	//drop out of script
}

bool Logic::fnSpeakMeDir(uint32 targetId, uint32 mesgNum, uint32 animNum) {
	//must be player so don't cause script to drop out
	//this function sets the directional option whereby
	//the anim chosen is linked to c_dir
	animNum += _compact->dir << 1;	//2 sizes (large and small)
	return fnSpeakMe(targetId, mesgNum, animNum);
}

bool Logic::fnSpeakWait(uint32 id, uint32 message, uint32 animation) {
	// non player mega char speaks
	// player will wait for it to finish before continuing script processing
	_compact->flag = (uint16)(id & 0xffff);
	_compact->logic = L_LISTEN;
	return fnSpeakMe(id, message, animation);
}

bool Logic::fnSpeakWaitDir(uint32 a, uint32 b, uint32 c) {
	/* non player mega chr$ speaks	S2(20Jan93tw)
	the player will wait for it to finish
	before continuing script processing
	this function sets the directional option whereby
	the anim chosen is linked to c_dir -

	_compact is player
	a is ID to speak (not us)
	b is text message number
	c is base of mini table within anim_talk_table */

#ifdef __DC__
	__builtin_alloca(4); // Works around a gcc bug (wrong-code/11736)
#endif

	_compact->flag = (uint16)a;
	_compact->logic = L_LISTEN;

	Compact *speaker = _skyCompact->fetchCpt(a);
	if (c) {
		c += speaker->dir << 1;
		stdSpeak(speaker, b, c, speaker->dir << 1);
	} else
		stdSpeak(speaker, b, c, 0);

	return false;
}

bool Logic::fnChooser(uint32 a, uint32 b, uint32 c) {

	// setup the text questions to be clicked on
	// read from TEXT1 until 0

	SkyEngine::_systemVars.systemFlags |= SF_CHOOSING; // can't save/restore while choosing

	_scriptVariables[THE_CHOSEN_ONE] = 0; // clear result

	uint32 *p = _scriptVariables + TEXT1;
	uint16 ycood = TOP_LEFT_Y; // rolling coordinate

	while (*p) {
		uint32 textNum = *p++;

		DisplayedText lowText = _skyText->lowTextManager(textNum, GAME_SCREEN_WIDTH, 0, 241, 0);

		uint8 *data = lowText.textData;

		// stipple the text

		uint32 size = ((DataFileHeader *)data)->s_height * ((DataFileHeader *)data)->s_width;
		uint32 index = 0;
		uint32 width = ((DataFileHeader *)data)->s_width;
		uint32 height = ((DataFileHeader *)data)->s_height;

		data += sizeof(DataFileHeader);

		while (index < size) {
			if (index % width <= 1)
				index ^= 1; //index++;
			if (!data[index])
				data[index] = 1;
			index += 2;
		}

		Compact *textCompact = _skyCompact->fetchCpt(lowText.compactNum);

		textCompact->getToFlag = (uint16)textNum;
		textCompact->downFlag = (uint16)*p++; // get animation number

		textCompact->status |= ST_MOUSE; // mouse detects

		textCompact->xcood = TOP_LEFT_X; // set coordinates
		textCompact->ycood = ycood;
		ycood += height;
	}

	if (p == _scriptVariables + TEXT1)
		return true;

	_compact->logic = L_CHOOSE; // player frozen until choice made
	fnAddHuman(0, 0, 0); // bring back mouse

	return false;
}

bool Logic::fnHighlight(uint32 itemNo, uint32 pen, uint32 c) {
	pen -= 11;
	pen ^= 1;
	pen += 241;
	Compact *textCompact = _skyCompact->fetchCpt(itemNo);
	uint8 *sprData = (uint8 *)SkyEngine::fetchItem(textCompact->flag);
	_skyText->changeTextSpriteColor(sprData, (uint8)pen);
	return true;
}

bool Logic::fnTextKill(uint32 a, uint32 b, uint32 c) {
	/// Kill of text items that are mouse detectable

	uint32 id = FIRST_TEXT_COMPACT;

	for (int i = 10; i > 0; i--) {
		Compact *cpt = _skyCompact->fetchCpt(id);
		if (cpt->status & (1 << 4))
			cpt->status = 0;
		id++;
	}
	return true;
}

bool Logic::fnStopMode(uint32 a, uint32 b, uint32 c) {
	_compact->logic = L_STOPPED;
	return false;
}

bool Logic::fnWeWait(uint32 id, uint32 b, uint32 c) {
	/// We have hit another mega
	/// we are going to wait for it to move

	_compact->waitingFor = (uint16) id;
	stopAndWait();
	return true; // not sure about this
}

bool Logic::fnSendSync(uint32 mega, uint32 sync, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(mega);
	cpt->sync = (uint16)(sync & 0xffff);
	return false;
}

bool Logic::fnSendFastSync(uint32 mega, uint32 sync, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(mega);
	cpt->sync = (uint16)(sync & 0xffff);
	return true;
}

bool Logic::fnSendRequest(uint32 target, uint32 scr, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(target);
	cpt->request = (uint16)(scr & 0xffff);
	return false;
}

bool Logic::fnClearRequest(uint32 target, uint32 b, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(target);
	cpt->request = 0;
	return true;
}

bool Logic::fnCheckRequest(uint32 a, uint32 b, uint32 c) {
	/// check for interaction request

	if (!_compact->request)
		return true;

	_compact->mode = C_ACTION_MODE; // into action mode

	_compact->actionSub = _compact->request;
	_compact->actionSub_off = 0;

	_compact->request = 0; // trash request
	return false; // drop from script
}

bool Logic::fnStartMenu(uint32 firstObject, uint32 b, uint32 c) {
	/// initialize the top menu bar
	// firstObject is o0 for game menu, k0 for linc

	uint i;
	firstObject /= 4;

	// (1) FIRST, SET UP THE 2 ARROWS SO THEY APPEAR ON SCREEN

	Compact *cpt = _skyCompact->fetchCpt(47);
	cpt->status = ST_MOUSE + ST_FOREGROUND + ST_LOGIC + ST_RECREATE;
	cpt->screen = (uint16)(_scriptVariables[SCREEN] & 0xffff);

	cpt = _skyCompact->fetchCpt(48);
	cpt->status = ST_MOUSE + ST_FOREGROUND + ST_LOGIC + ST_RECREATE;
	cpt->screen = (uint16)(_scriptVariables[SCREEN] & 0xffff);

	// (2) COPY OBJECTS FROM NON-ZERO INVENTORY VARIABLES INTO OBJECT DISPLAY LIST (& COUNT THEM)

	// sort the objects and pad with blanks

	uint32 menuLength = 0;
	for (i = firstObject; i < firstObject + ARRAYSIZE(_objectList); i++) {
		if (_scriptVariables[i])
			_objectList[menuLength++] = _scriptVariables[i];
	}
	_scriptVariables[MENU_LENGTH] = menuLength;

	// (3) OK, NOW TOP UP THE LIST WITH THE REQUIRED NO. OF BLANK OBJECTS (for min display length 11)

	uint32 blankID = 51;
	for (i = menuLength; i < 11; i++)
		_objectList[i] = blankID++;

	// (4) KILL ID's OF ALL 20 OBJECTS SO UNWANTED ICONS (SCROLLED OFF) DON'T REMAIN ON SCREEN
	// (There should be a better way of doing this - only kill id of 12th item when menu has scrolled right)

	for (i = 0; i < ARRAYSIZE(_objectList); i++) {
		if (_objectList[i])
			(_skyCompact->fetchCpt(_objectList[i]))->status = ST_LOGIC;
		else break;
	}

	// (5) NOW FIND OUT WHICH OBJECT TO START THE DISPLAY FROM (depending on scroll offset)

	if (menuLength < 11) // check we can scroll
		_scriptVariables[SCROLL_OFFSET] = 0;
	else if (menuLength < _scriptVariables[SCROLL_OFFSET] + 11)
		_scriptVariables[SCROLL_OFFSET] = menuLength - 11;

	// (6) AND FINALLY, INITIALIZE THE 11 OBJECTS SO THEY APPEAR ON SCREEEN

	uint16 rollingX = TOP_LEFT_X + 28;
	for (i = 0; i < 11; i++) {
		cpt = _skyCompact->fetchCpt(
				_objectList[_scriptVariables[SCROLL_OFFSET] + i]);

		cpt->status = ST_MOUSE + ST_FOREGROUND + ST_LOGIC + ST_RECREATE;
		cpt->screen = (uint16)(_scriptVariables[SCREEN] & 0xffff);

		cpt->xcood = rollingX;
		rollingX += 24;

		if (_scriptVariables[MENU] == 2)
			cpt->ycood = 136;
		else
			cpt->ycood = 112;
	}

	return true;
}

bool Logic::fnUnhighlight(uint32 item, uint32 b, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(item);
	cpt->frame--;
	cpt->getToFlag = 0;
	return true;
}

bool Logic::fnFaceId(uint32 otherId, uint32 b, uint32 c) {
	/// return the direction to turn to face another id
	/// pass back result in c_just_flag

	Compact *cpt = _skyCompact->fetchCpt(otherId);

	int16 x = _compact->xcood - cpt->xcood;

	if (x < 0) { // we're to the left
		x = -x;
		_compact->getToFlag = 3;
	} else { // it's to the left
		_compact->getToFlag = 2;
	}

	// now check y

	// we must find the true bottom of the sprite
	// it is not enough to use y coord because changing
	// sprite offsets can ruin the formula - instead we
	// will use the bottom of the mouse collision area

	int16 y = _compact->ycood - (cpt->ycood + cpt->mouseRelY + cpt->mouseSizeY);

	if (y < 0) { // it's below
		y = -y;
		if (y >= x)
			_compact->getToFlag = 1;
	} else { // it's above
		if (y >= x)
			_compact->getToFlag = 0;
	}
	return true;
}

bool Logic::fnForeground(uint32 sprite, uint32 b, uint32 c) {
	/// Make sprite a foreground sprite
	Compact *cpt = _skyCompact->fetchCpt(sprite);
	cpt->status &= 0xfff8;
	cpt->status |= ST_FOREGROUND;
	return true;
}

bool Logic::fnBackground(uint32 a, uint32 b, uint32 c) {
	/// Make us a background sprite
	_compact->status &= 0xfff8;
	_compact->status |= ST_BACKGROUND;
	return true;
}

bool Logic::fnNewBackground(uint32 sprite, uint32 b, uint32 c) {
	/// Make sprite a background sprite
	Compact *cpt = _skyCompact->fetchCpt(sprite);
	cpt->status &= 0xfff8;
	cpt->status |= ST_BACKGROUND;
	return true;
}

bool Logic::fnSort(uint32 mega, uint32 b, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(mega);
	cpt->status &= 0xfff8;
	cpt->status |= ST_SORT;
	return true;
}

bool Logic::fnNoSpriteEngine(uint32 a, uint32 b, uint32 c) {
	/// stop the compact printing
	/// remove foreground, background & sort
	_compact->status &= 0xfff8;
	return true;
}

bool Logic::fnNoSpritesA6(uint32 us, uint32 b, uint32 c) {
	/// stop the compact printing
	/// remove foreground, background & sort
	Compact *cpt = _skyCompact->fetchCpt(us);
	cpt->status &= 0xfff8;
	return true;
}

bool Logic::fnResetId(uint32 id, uint32 resetBlock, uint32 c) {
	/// used when a mega is to be restarted
	/// eg - when a smaller mega turn to larger
	/// - a mega changes rooms...

	Compact *cpt = _skyCompact->fetchCpt(id);
	uint16 *rst = (uint16 *)_skyCompact->fetchCpt(resetBlock);

	if (!cpt) {
		warning("fnResetId(): Compact %d (id) == NULL", id);
		return true;
	}
	if (!rst) {
		warning("fnResetId(): Compact %d (resetBlock) == NULL", resetBlock);
		return true;
	}

	uint16 off;
	while ((off = *rst++) != 0xffff)
		*(uint16 *)_skyCompact->getCompactElem(cpt, off) = *rst++;
	return true;
}

bool Logic::fnToggleGrid(uint32 a, uint32 b, uint32 c) {
	/// Toggle a mega's grid plotting
	_compact->status ^= ST_GRID_PLOT;
	return true;
}

bool Logic::fnPause(uint32 cycles, uint32 b, uint32 c) {
	/// Set mega to L_PAUSE
	_compact->flag = (uint16)(cycles & 0xffff);
	_compact->logic = L_PAUSE;
	return false; // drop out of script
}

bool Logic::fnRunAnimMod(uint32 animNo, uint32 b, uint32 c) {
	_compact->grafixProgId = animNo;
	_compact->grafixProgPos = 0;

	_compact->offset = *_skyCompact->getGrafixPtr(_compact);
	_compact->grafixProgPos++;
	_compact->logic = L_MOD_ANIMATE;
	anim();
	return false; // drop from script
}

bool Logic::fnSimpleMod(uint32 animSeqNo, uint32 b, uint32 c) {
	_compact->grafixProgId = animSeqNo;
	_compact->grafixProgPos = 0;

	_compact->logic = L_SIMPLE_MOD;
	_compact->offset = *_skyCompact->getGrafixPtr(_compact);
	_compact->grafixProgPos++;
	simpleAnim();
	return false;
}

bool Logic::fnRunFrames(uint32 sequenceNo, uint32 b, uint32 c) {
	_compact->grafixProgId = sequenceNo;
	_compact->grafixProgPos = 0;

	_compact->logic = L_FRAMES;
	_compact->offset = *_skyCompact->getGrafixPtr(_compact);
	_compact->grafixProgPos++;
	simpleAnim();
	return false;
}

bool Logic::fnAwaitSync(uint32 a, uint32 b, uint32 c) {
	if (_compact->sync)
		return true;

	_compact->logic = L_WAIT_SYNC;
	return false;
}

bool Logic::fnIncMegaSet(uint32 a, uint32 b, uint32 c) {
	_compact->megaSet += NEXT_MEGA_SET;
	return true;
}

bool Logic::fnDecMegaSet(uint32 a, uint32 b, uint32 c) {
	_compact->megaSet -= NEXT_MEGA_SET;
	return true;
}

bool Logic::fnSetMegaSet(uint32 mega, uint32 setNo, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(mega);
	cpt->megaSet = (uint16) (setNo * NEXT_MEGA_SET);
	return true;
}

bool Logic::fnMoveItems(uint32 listNo, uint32 screenNo, uint32 c) {
	// Move a list of id's to another screen
	uint16 *p = (uint16 *)_skyCompact->fetchCpt(CPT_MOVE_LIST);
	p = (uint16 *)_skyCompact->fetchCpt(p[listNo]);
	for (int i = 0; i < 2; i++) {
		if (!*p)
			return true;
		Compact *cpt = _skyCompact->fetchCpt(*p++);
		cpt->screen = (uint16)(screenNo & 0xffff);
	}
	return true;
}

bool Logic::fnNewList(uint32 a, uint32 b, uint32 c) {
	/// Reset the chooser list
	for (int i = 0; i < 16; i++)
		_scriptVariables[TEXT1 + i] = 0;
	return true;
}

bool Logic::fnAskThis(uint32 textNo, uint32 animNo, uint32 c) {
	// find first free position
	uint32 *p = _scriptVariables + TEXT1;
	while (*p)
		p += 2;
	*p++ = textNo;
	*p = animNo;
	return true;
}

bool Logic::fnRandom(uint32 a, uint32 b, uint32 c) {
	_scriptVariables[RND] = _rnd.getRandomNumber(65536) & a;
	return true;
}

bool Logic::fnPersonHere(uint32 id, uint32 room, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(id);
	_scriptVariables[RESULT] = cpt->screen == room ? 1 : 0;
	return true;
}

bool Logic::fnToggleMouse(uint32 a, uint32 b, uint32 c) {
	_skyCompact->fetchCpt(a)->status ^= ST_MOUSE;
	return true;
}

bool Logic::fnMouseOn(uint32 a, uint32 b, uint32 c) {
	//switch on the mouse highlight
	Compact *cpt = _skyCompact->fetchCpt(a);
	cpt->status |= ST_MOUSE;
	return true;
}

bool Logic::fnMouseOff(uint32 a, uint32 b, uint32 c) {
	//switch off the mouse highlight
	Compact *cpt = _skyCompact->fetchCpt(a);
	cpt->status &= ~ST_MOUSE;
	return true;
}

bool Logic::fnFetchX(uint32 id, uint32 b, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(id);
	_scriptVariables[RESULT] = cpt->xcood;
	return true;
}

bool Logic::fnFetchY(uint32 id, uint32 b, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(id);
	_scriptVariables[RESULT] = cpt->ycood;
	return true;
}

bool Logic::fnTestList(uint32 id, uint32 x, uint32 y) {
	_scriptVariables[RESULT] = 0; // assume fail
	uint16 *list = (uint16 *)_skyCompact->fetchCpt(id);

	while (*list) {
		if ((x >= list[0]) && (x < list[1]) && (y >= list[2]) && (y < list[3]))
			_scriptVariables[RESULT] = list[4];
		list += 5;
	}
	return true;
}

bool Logic::fnFetchPlace(uint32 id, uint32 b, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(id);
	_scriptVariables[RESULT] = cpt->place;
	return true;
}

bool Logic::fnCustomJoey(uint32 id, uint32 b, uint32 c) {
	/// return id's x & y coordinate & c_mood (i.e. stood still yes/no)
	/// used by Joey-Logic - done in code like this because scripts can't
	/// get access to another megas compact as easily

	Compact *cpt = _skyCompact->fetchCpt(id);

	_scriptVariables[PLAYER_X] = cpt->xcood;
	_scriptVariables[PLAYER_Y] = cpt->ycood;
	_scriptVariables[PLAYER_MOOD] = cpt->mood;
	_scriptVariables[PLAYER_SCREEN] = cpt->screen;
	return true;
}

bool Logic::fnSetPalette(uint32 a, uint32 b, uint32 c) {
	_skyScreen->setPaletteEndian((uint8 *)_skyCompact->fetchCpt(a));
	SkyEngine::_systemVars.currentPalette = a;
	return true;
}

bool Logic::fnTextModule(uint32 a, uint32 b, uint32 c) {
	_skyText->fnTextModule(a, b);
	return true;
}

bool Logic::fnChangeName(uint32 id, uint32 textNo, uint32 c) {
	Compact *cpt = _skyCompact->fetchCpt(id);
	cpt->cursorText = (uint16) textNo;
	return true;
}

bool Logic::fnMiniLoad(uint32 a, uint32 b, uint32 c) {
	_skyDisk->fnMiniLoad((uint16)a);
	return true;
}

bool Logic::fnFlushBuffers(uint32 a, uint32 b, uint32 c) {
	_skyDisk->fnFlushBuffers();
	return true;
}

bool Logic::fnFlushChip(uint32 a, uint32 b, uint32 c) {
	// this should be done automatically
	return true;
}

bool Logic::fnSaveCoods(uint32 a, uint32 b, uint32 c) {
	_skyMouse->fnSaveCoods();
	return true;
}

bool Logic::fnPlotGrid(uint32 x, uint32 y, uint32 width) {
	_skyGrid->plotGrid(x, y, width, _compact);
	return true;
}

bool Logic::fnRemoveGrid(uint32 x, uint32 y, uint32 width) {
	_skyGrid->removeGrid(x, y, width, _compact);
	return true;
}

bool Logic::fnEyeball(uint32 id, uint32 b, uint32 c) {
	// set 'result' to frame no. pointing to foster, according to table used
	// eg. FN_eyeball (id_eye_90_table);

	uint16 *eyeTable = (uint16 *)_skyCompact->fetchCpt(id);
	Compact *cpt = _skyCompact->fetchCpt(ID_BLUE_FOSTER);

	uint32 x = cpt->xcood; // 168 < x < 416
	x -= 168;
	x >>= 3;

	uint32 y = cpt->ycood; // 256 < y < 296
	y -= 256;
	y <<= 2;

	_scriptVariables[RESULT] = eyeTable[x + y] + S91;
	return true;
}

bool Logic::fnLeaveSection(uint32 sectionNo, uint32 b, uint32 c) {
	if (SkyEngine::isDemo())
		Engine::quitGame();

	if (sectionNo == 5) //linc section - has different mouse icons
		_skyMouse->replaceMouseCursors(60301);

	return true;
}

bool Logic::fnEnterSection(uint32 sectionNo, uint32 b, uint32 c) {
	if (SkyEngine::isDemo() && (sectionNo > 2))
		_skyControl->showGameQuitMsg();

	_scriptVariables[CUR_SECTION] = sectionNo;
	SkyEngine::_systemVars.currentMusic = 0;

	if (sectionNo == 5) //linc section - has different mouse icons
		_skyMouse->replaceMouseCursors(60302);

	if ((sectionNo != _currentSection) || (SkyEngine::_systemVars.systemFlags & SF_GAME_RESTORED)) {
		_currentSection = sectionNo;

		sectionNo++;
		_skyMusic->loadSection((byte)sectionNo);
		_skySound->loadSection((byte)sectionNo);
		_skyGrid->loadGrids();
		SkyEngine::_systemVars.systemFlags &= ~SF_GAME_RESTORED;
	}

	return true;
}

bool Logic::fnRestoreGame(uint32 a, uint32 b, uint32 c) {
	_skyControl->doLoadSavePanel();
	return false;
}

bool Logic::fnRestartGame(uint32 a, uint32 b, uint32 c) {
	_skyControl->restartGame();
	return false;
}

bool Logic::fnNewSwingSeq(uint32 a, uint32 b, uint32 c) {
	// only certain files work on pc. (huh?! something we should take care of?)
	if ((a == 85) || (a == 106) || (a == 75) || (a == 15)) {
		_skyScreen->startSequenceItem((uint16)a);
	} else {
		debug(1,"Logic::fnNewSwingSeq: ignored seq %d",a);
	}
	return true;
}

bool Logic::fnWaitSwingEnd(uint32 a, uint32 b, uint32 c) {
	_skyScreen->waitForSequence();
	return true;
}

bool Logic::fnSkipIntroCode(uint32 a, uint32 b, uint32 c) {
	SkyEngine::_systemVars.pastIntro = true;
	return true;
}

bool Logic::fnBlankScreen(uint32 a, uint32 b, uint32 c) {
	_skyScreen->clearScreen();
	return true;
}

bool Logic::fnPrintCredit(uint32 a, uint32 b, uint32 c) {
	DisplayedText creditText = _skyText->lowTextManager(a, 240, 0, 248, true);
	Compact *credCompact = _skyCompact->fetchCpt(creditText.compactNum);
	credCompact->xcood = 168;
	if ((a == 558) && (c == 215))
		credCompact->ycood = 211;
	else
		credCompact->ycood = (uint16)c;
	_scriptVariables[RESULT] = creditText.compactNum;
	return true;
}

bool Logic::fnLookAt(uint32 a, uint32 b, uint32 c) {
	DisplayedText textInfo = _skyText->lowTextManager(a, 240, 0, 248, true);
	Compact *textCpt = _skyCompact->fetchCpt(textInfo.compactNum);
	textCpt->xcood = 168;
	textCpt->ycood = (uint16)c;

	_skyScreen->recreate();
	_skyScreen->spriteEngine();
	_skyScreen->flip();

	fnNoHuman(0, 0, 0);
	_skyMouse->lockMouse();

	_skyMouse->waitMouseNotPressed(800);

	_skyMouse->unlockMouse();
	fnAddHuman(0, 0, 0);
	textCpt->status = 0;

	return true;
}

bool Logic::fnLincTextModule(uint32 textPos, uint32 textNo, uint32 buttonAction) {
	uint16 cnt;
	if (buttonAction & 0x8000)
		for (cnt = LINC_DIGIT_0; cnt <= LINC_DIGIT_9; cnt++)
			_scriptVariables[cnt] = 0;
	buttonAction &= 0x7FFF;
	if (buttonAction < 10)
		_scriptVariables[LINC_DIGIT_0 + buttonAction] = textNo;

	DisplayedText text = _skyText->lowTextManager(textNo, 220, 0, 215, false);

	Compact *textCpt = _skyCompact->fetchCpt(text.compactNum);

	if (textPos < 20) { // line number (for text)
		textCpt->xcood = 152;
		textCpt->ycood = (uint16)textPos * 13 + 170;
	} else if (textPos > 20) { // x coordinate (for numbers)
		textCpt->xcood = (uint16)textPos;
		textCpt->ycood = 214;
	} else warning("::fnLincTextModule: textPos == 20");
	textCpt->getToFlag = (uint16)textNo;
	return true;
}

bool Logic::fnTextKill2(uint32 a, uint32 b, uint32 c) {
	/// Kill all text items

	uint32 id = FIRST_TEXT_COMPACT;

	for (int i = 10; i > 0; i--) {
		Compact *cpt = _skyCompact->fetchCpt(id);
		cpt->status = 0;
		id++;
	}
	return true;
}

bool Logic::fnSetFont(uint32 font, uint32 b, uint32 c) {
	_skyText->fnSetFont(font);
	return true;
}

bool Logic::fnStartFx(uint32 sound, uint32 b, uint32 c) {
	_skySound->fnStartFx(sound, (uint8)(b & 1));
	return true;
}

bool Logic::fnStopFx(uint32 a, uint32 b, uint32 c) {
	_skySound->fnStopFx();
	return true;
}

bool Logic::fnStartMusic(uint32 a, uint32 b, uint32 c) {
	if (!(SkyEngine::_systemVars.systemFlags & SF_MUS_OFF))
		_skyMusic->startMusic((uint16)a);
	SkyEngine::_systemVars.currentMusic = (uint16)a;
	return true;
}

bool Logic::fnStopMusic(uint32 a, uint32 b, uint32 c) {
	_skyMusic->startMusic(0);
	SkyEngine::_systemVars.currentMusic = 0;
	return true;
}

bool Logic::fnFadeDown(uint32 a, uint32 b, uint32 c) {
	_skyScreen->fnFadeDown(a);
	return true;
}

bool Logic::fnFadeUp(uint32 a, uint32 b, uint32 c) {
	SkyEngine::_systemVars.currentPalette = a;
	_skyScreen->fnFadeUp(a,b);
	return true;
}

bool Logic::fnQuitToDos(uint32 a, uint32 b, uint32 c) {
	Engine::quitGame();
	return false;
}

bool Logic::fnPauseFx(uint32 a, uint32 b, uint32 c) {
	_skySound->fnPauseFx();
	return true;
}

bool Logic::fnUnPauseFx(uint32 a, uint32 b, uint32 c) {
	_skySound->fnUnPauseFx();
	return true;
}

bool Logic::fnPrintf(uint32 a, uint32 b, uint32 c) {
	debug("fnPrintf(%d, %d, %d)", a, b, c);
	return true;
}

void Logic::stdSpeak(Compact *target, uint32 textNum, uint32 animNum, uint32 base) {
	animNum += target->megaSet / NEXT_MEGA_SET;
	animNum &= 0xFF;

	uint16 *talkTable = (uint16 *)_skyCompact->fetchCpt(CPT_TALK_TABLE_LIST);
	target->grafixProgId = talkTable[animNum];
	target->grafixProgPos = 0;
	uint16 *animPtr = _skyCompact->getGrafixPtr(target);

	if (animPtr) {
		target->offset = *animPtr++;
		target->getToFlag = *animPtr++;
		target->grafixProgPos += 2;
	} else
		target->grafixProgId = 0;

	bool speechFileFound = false;
	if (SkyEngine::isCDVersion())
		speechFileFound = _skySound->startSpeech((uint16)textNum);


	// Set the focus region to that area
	// Calculate the point where the character is
	int x = target->xcood - TOP_LEFT_X;
	int y = target->ycood - TOP_LEFT_Y;
	// TODO: Make the box size change based on the object that has the focus
	_skyScreen->setFocusRectangle(Common::Rect::center(x, y, 192, 128));


	if ((SkyEngine::_systemVars.systemFlags & SF_ALLOW_TEXT) || !speechFileFound) {
		// form the text sprite, if player wants subtitles or
		// if we couldn't find the speech file
		DisplayedText textInfo;
		textInfo = _skyText->lowTextManager(textNum, FIXED_TEXT_WIDTH, 0, (uint8)target->spColor, true);
		Compact *textCompact = _skyCompact->fetchCpt(textInfo.compactNum);
		target->spTextId = textInfo.compactNum;	//So we know what text to kill
		byte *textGfx = textInfo.textData;

		textCompact->screen = target->screen;	//put it on our screen

		if (_scriptVariables[SCREEN] == target->screen) { // Only use coordinates if we are on the current screen
			//talking on-screen
			//create the x coordinate for the speech text
			//we need the talkers sprite information
			byte *targetGfx = (byte *)SkyEngine::fetchItem(target->frame >> 6);
			uint16 xPos = target->xcood + ((DataFileHeader *)targetGfx)->s_offset_x;
			uint16 width = (((DataFileHeader *)targetGfx)->s_width >> 1);

			xPos += width - (FIXED_TEXT_WIDTH / 2);	//middle of talker

			if (xPos < TOP_LEFT_X)
				xPos = TOP_LEFT_X;

			width = xPos + FIXED_TEXT_WIDTH;
			if ((TOP_LEFT_X + FULL_SCREEN_WIDTH) <= width) {
				xPos = TOP_LEFT_X + FULL_SCREEN_WIDTH;
				xPos -= FIXED_TEXT_WIDTH;
			}

			textCompact->xcood = xPos;
			uint16 yPos = target->ycood + ((DataFileHeader *)targetGfx)->s_offset_y - 6 - ((DataFileHeader *)textGfx)->s_height;

			if (yPos < TOP_LEFT_Y)
				yPos = TOP_LEFT_Y;

			textCompact->ycood = yPos;

		} else {
			//talking off-screen
			target->spTextId = 0;	//don't kill any text 'cos none was made
			textCompact->status = 0;	//don't display text
		}
		// In CD version, we're doing the timing by checking when the VOC has stopped playing.
		// Setting spTime to 10 thus means that we're doing a pause of 10 gamecycles between
		// each sentence.
		if (speechFileFound)
			target->spTime = 10;
		else
			target->spTime = (uint16)_skyText->_numLetters + 5;
	} else {
		target->spTime = 10;
		target->spTextId = 0;
	}
	target->logic = L_TALK;
}

} // End of namespace Sky
