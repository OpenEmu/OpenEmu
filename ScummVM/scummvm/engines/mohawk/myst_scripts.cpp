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

#include "mohawk/cursors.h"
#include "mohawk/myst.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_scripts.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "common/system.h"
#include "common/textconsole.h"
#include "gui/message.h"

namespace Mohawk {

MystScriptEntry::MystScriptEntry() {
	type = kMystScriptNone;
	var = 0;
	argc = 0;
	argv = 0;
	resourceId = 0;
	u1 = 0;
}

MystScriptEntry::~MystScriptEntry() {
	delete[] argv;
}

const uint8 MystScriptParser::_stackMap[11] = {
	kSeleniticStack,
	kStoneshipStack,
	kMystStack,
	kMechanicalStack,
	kChannelwoodStack,
	kIntroStack,
	kDniStack,
	kMystStack,
	kCreditsStack,
	kMystStack,
	kMystStack
};

const uint16 MystScriptParser::_startCard[11] = {
	1282,
	2029,
	4396,
	6122,
	3137,
	1,
	5038,
	4134,
	10000,
	4739,
	4741
};

// NOTE: Credits Start Card is 10000

MystScriptParser::MystScriptParser(MohawkEngine_Myst *vm) :
		_vm(vm),
		_globals(vm->_gameState->_globals) {
	setupCommonOpcodes();
	_invokingResource = NULL;
	_savedCardId = 0;
	_savedCursorId = 0;
	_tempVar = 0;
}

MystScriptParser::~MystScriptParser() {
	for (uint32 i = 0; i < _opcodes.size(); i++)
		delete _opcodes[i];
}

#define OPCODE(op, x) _opcodes.push_back(new MystOpcode(op, &MystScriptParser::x, #x))

void MystScriptParser::setupCommonOpcodes() {
	// These opcodes are common to each stack

	// "Standard" Opcodes
	OPCODE(0, o_toggleVar);
	OPCODE(1, o_setVar);
	OPCODE(2, o_changeCardSwitch);
	OPCODE(3, o_takePage);
	OPCODE(4, o_redrawCard);
	// Opcode 5 Not Present
	OPCODE(6, o_goToDest);
	OPCODE(7, o_goToDest);
	OPCODE(8, o_goToDest);
	OPCODE(9, o_triggerMovie);
	OPCODE(10, o_toggleVarNoRedraw);
	// Opcode 11 Not Present
	OPCODE(12, o_changeCardSwitch);
	OPCODE(13, o_changeCardSwitch);
	OPCODE(14, o_drawAreaState);
	OPCODE(15, o_redrawAreaForVar);
	OPCODE(16, o_changeCardDirectional);
	OPCODE(17, o_changeCardPush);
	OPCODE(18, o_changeCardPop);
	OPCODE(19, o_enableAreas);
	OPCODE(20, o_disableAreas);
	OPCODE(21, o_directionalUpdate);
	OPCODE(22, o_goToDest);
	OPCODE(23, o_toggleAreasActivation);
	OPCODE(24, o_playSound);
	// Opcode 25 is unused; original calls replaceSoundMyst
	OPCODE(26, o_stopSoundBackground);
	OPCODE(27, o_playSoundBlocking);
	OPCODE(28, o_copyBackBufferToScreen);
	OPCODE(29, o_copyImageToBackBuffer);
	OPCODE(30, o_changeBackgroundSound);
	OPCODE(31, o_soundPlaySwitch);
	OPCODE(32, o_soundResumeBackground);
	OPCODE(33, o_copyImageToScreen);
	OPCODE(34, o_changeCard);
	OPCODE(35, o_drawImageChangeCard);
	OPCODE(36, o_changeMainCursor);
	OPCODE(37, o_hideCursor);
	OPCODE(38, o_showCursor);
	OPCODE(39, o_delay);
	OPCODE(40, o_changeStack);
	OPCODE(41, o_changeCardPlaySoundDirectional);
	OPCODE(42, o_directionalUpdatePlaySound);
	OPCODE(43, o_saveMainCursor);
	OPCODE(44, o_restoreMainCursor);
	// Opcode 45 Not Present
	OPCODE(46, o_soundWaitStop);
	OPCODE(51, o_exitMap);
	// Opcodes 47 to 99 Not Present

	OPCODE(0xFFFF, NOP);
}

#undef OPCODE

void MystScriptParser::runScript(MystScript script, MystResource *invokingResource) {
	debugC(kDebugScript, "Script Size: %d", script->size());

	// Scripted drawing takes more time to simulate older hardware
	// This way opcodes can't overwrite what the previous ones drew too quickly
	_vm->_gfx->enableDrawingTimeSimulation(true);

	for (uint16 i = 0; i < script->size(); i++) {
		MystScriptEntry &entry = (*script)[i];
		debugC(kDebugScript, "\tOpcode %d: %d", i, entry.opcode);

		if (entry.type == kMystScriptNormal)
			_invokingResource = invokingResource;
		else
			_invokingResource = _vm->_resources[entry.resourceId];

		runOpcode(entry.opcode, entry.var, entry.argc, entry.argv);
	}

	_vm->_gfx->enableDrawingTimeSimulation(false);
}

void MystScriptParser::runOpcode(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	bool ranOpcode = false;

	for (uint16 i = 0; i < _opcodes.size(); i++)
		if (_opcodes[i]->op == op) {
			(this->*(_opcodes[i]->proc)) (op, var, argc, argv);
			ranOpcode = true;
			break;
		}

	if (!ranOpcode)
		warning("Trying to run invalid opcode %d", op);
}

const Common::String MystScriptParser::getOpcodeDesc(uint16 op) {
	for (uint16 i = 0; i < _opcodes.size(); i++)
		if (_opcodes[i]->op == op)
			return _opcodes[i]->desc;

	return Common::String::format("%d", op);
}

MystScript MystScriptParser::readScript(Common::SeekableReadStream *stream, MystScriptType type) {
	assert(stream);
	assert(type != kMystScriptNone);

	MystScript script = MystScript(new Common::Array<MystScriptEntry>());

	uint16 opcodeCount = stream->readUint16LE();
	script->resize(opcodeCount);

	for (uint16 i = 0; i < opcodeCount; i++) {
		MystScriptEntry &entry = (*script)[i];
		entry.type = type;

		// Resource ID only exists in INIT and EXIT scripts
		if (type != kMystScriptNormal)
			entry.resourceId = stream->readUint16LE();

		entry.opcode = stream->readUint16LE();
		entry.var = stream->readUint16LE();
		entry.argc = stream->readUint16LE();

		if (entry.argc > 0) {
			entry.argv = new uint16[entry.argc];
			for (uint16 j = 0; j < entry.argc; j++)
				entry.argv[j] = stream->readUint16LE();
		}

		// u1 exists only in EXIT scripts
		if (type == kMystScriptExit)
			entry.u1 = stream->readUint16LE();
	}

	return script;
}

uint16 MystScriptParser::getVar(uint16 var) {
	switch(var) {
	case 105:
		return _tempVar;
	case 106:
		return _globals.ending;
	default:
		warning("Unimplemented var getter 0x%02x (%d)", var, var);
		return 0;
	}
}

void MystScriptParser::toggleVar(uint16 var) {
	warning("Unimplemented var toggle 0x%02x (%d)", var, var);
}

bool MystScriptParser::setVarValue(uint16 var, uint16 value) {
	if (var == 105) {
		if (_tempVar != value)
			_tempVar = value;
	} else {
		warning("Unimplemented var setter 0x%02x (%d)", var, var);
	}

	return false;
}

// NOTE: Check to be used on Opcodes where var is thought
// not to be used. This emits a warning if var is nonzero.
// It is possible that the opcode does use var 0 in this case,
// but this will catch the majority of missed cases.
void MystScriptParser::varUnusedCheck(uint16 op, uint16 var) {
	if (var != 0)
		warning("Opcode %d: Unused Var %d", op, var);
}

void MystScriptParser::animatedUpdate(uint16 argc, uint16 *argv, uint16 delay) {
	uint16 argsRead = 0;

	while (argsRead < argc) {
		Common::Rect rect = Common::Rect(argv[argsRead], argv[argsRead + 1], argv[argsRead + 2], argv[argsRead + 3]);
		uint16 kind = argv[argsRead + 4];
		uint16 steps = argv[argsRead + 5];

		debugC(kDebugScript, "\trect.left: %d", rect.left);
		debugC(kDebugScript, "\trect.top: %d", rect.top);
		debugC(kDebugScript, "\trect.right: %d", rect.right);
		debugC(kDebugScript, "\trect.bottom: %d", rect.bottom);

		debugC(kDebugScript, "\tkind / direction: %d", kind);
		debugC(kDebugScript, "\tsteps: %d", steps);

		_vm->_gfx->runTransition(kind, rect, steps, delay);

		argsRead += 6;
	}
}

void MystScriptParser::unknown(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	warning("Unimplemented opcode 0x%02x (%d)", op, op);
	warning("\tUses var %d", var);
	warning("\tArg count = %d", argc);

	if (argc) {
		Common::String str;
		str += Common::String::format("%d", argv[0]);

		for (uint16 i = 1; i < argc; i++)
			str += Common::String::format(", %d", argv[i]);

		warning("\tArgs: %s\n", str.c_str());
	}
}

void MystScriptParser::NOP(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
}

void MystScriptParser::o_toggleVar(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Toggle var %d", op, var);

	toggleVar(var);
	_vm->redrawArea(var);
}

void MystScriptParser::o_setVar(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Set var %d: %d", op, var, argv[0]);

	if (setVarValue(var, argv[0]))
		_vm->redrawArea(var);
}

void MystScriptParser::o_changeCardSwitch(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Opcodes 2, 12, and 13 are the same
	uint16 value = getVar(var);

	debugC(kDebugScript, "Opcode %d: changeCardSwitch var %d: %d", op, var, value);

	if (value)
		_vm->changeToCard(argv[value -1 ], true);
	else if (_invokingResource != NULL)
		_vm->changeToCard(_invokingResource->getDest(), true);
	else
		warning("Missing invokingResource in altDest call");
}

void MystScriptParser::o_takePage(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 cursorId = argv[0];
	uint16 oldPage = _globals.heldPage;

	debugC(kDebugScript, "Opcode %d: takePage Var %d CursorId %d", op, var, cursorId);

	// Take / drop page
	toggleVar(var);

	if (oldPage != _globals.heldPage) {
		_vm->_cursor->hideCursor();
		_vm->redrawArea(var);

		// Set new cursor
		if (_globals.heldPage)
			_vm->setMainCursor(cursorId);
		else
			_vm->setMainCursor(kDefaultMystCursor);

		_vm->_cursor->showCursor();
	}
}

void MystScriptParser::o_redrawCard(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Redraw card", op);

	_vm->drawCardBackground();
	_vm->drawResourceImages();
	_vm->_gfx->copyBackBufferToScreen(Common::Rect(544, 333));
	_vm->_system->updateScreen();
}

void MystScriptParser::o_goToDest(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Change To Dest of Invoking Resource", op);

	if (_invokingResource != NULL)
		_vm->changeToCard(_invokingResource->getDest(), true);
	else
		warning("Opcode %d: Missing invokingResource", op);
}
void MystScriptParser::o_triggerMovie(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Trigger Type 6 Resource Movie..", op);
	// TODO: If movie has sound, pause background music

	int16 direction = 1;
	if (argc == 1)
		direction = argv[0];

	debugC(kDebugScript, "\tDirection: %d", direction);

	// Trigger resource 6 movie overriding play direction
	MystResourceType6 *resource = static_cast<MystResourceType6 *>(_invokingResource);
	resource->setDirection(direction);
	resource->playMovie();

	// TODO: If movie has sound, resume background music
}

void MystScriptParser::o_toggleVarNoRedraw(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: toggleVarNoRedraw", op);

	toggleVar(var);
}

void MystScriptParser::o_drawAreaState(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: drawAreaState, state: %d", op, argv[0]);
	debugC(kDebugScript, "\tVar: %d", var);

	MystResourceType8 *parent = static_cast<MystResourceType8 *>(_invokingResource->_parent);
	parent->drawConditionalDataToScreen(argv[0]);
}

void MystScriptParser::o_redrawAreaForVar(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: redraw area", op);
	debugC(kDebugScript, "\tvar: %d", var);

	_vm->redrawArea(var);
}

void MystScriptParser::o_changeCardDirectional(uint16 op, uint16 var, uint16 argc, uint16 *argv) {

	// Used by Channelwood Card 3262 (In Elevator)
	debugC(kDebugScript, "Opcode %d: Change Card with optional directional update", op);

	uint16 cardId = argv[0];
	uint16 directionalUpdateDataSize = argv[1];

	debugC(kDebugScript, "\tcardId: %d", cardId);
	debugC(kDebugScript, "\tdirectonal update data size: %d", directionalUpdateDataSize);

	_vm->changeToCard(cardId, false);

	animatedUpdate(directionalUpdateDataSize, &argv[2], 0);
}

// NOTE: Opcode 17 and 18 form a pair, where Opcode 17 jumps to a card,
// but with the current cardId stored.
// Opcode 18 then "pops" this stored CardId and returns to that card.

void MystScriptParser::o_changeCardPush(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Jump to Card Id, Storing Current Card Id", op);

	_savedCardId = _vm->getCurCard();
	uint16 cardId = argv[0];

	// argv[1] is not used in the original engine

	debugC(kDebugScript, "\tCurrent CardId: %d", _savedCardId);
	debugC(kDebugScript, "\tJump to CardId: %d", cardId);

	_vm->changeToCard(cardId, true);
}

void MystScriptParser::o_changeCardPop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Return To Stored Card Id", op);
	debugC(kDebugScript, "\tCardId: %d", _savedCardId);

	// argv[0] is not used in the original engine

	_vm->changeToCard(_savedCardId, true);
}

void MystScriptParser::o_enableAreas(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Enable Hotspots", op);

	uint16 count = argv[0];

	if (argc == count + 1) {
		for (uint16 i = 0; i < count; i++) {
			debugC(kDebugScript, "Enable hotspot index %d", argv[i + 1]);

			MystResource *resource = 0;
			if (argv[i + 1] == 0xFFFF)
				resource = _invokingResource;
			else
				resource = _vm->_resources[argv[i + 1]];

			if (resource)
				resource->setEnabled(true);
			else
				warning("Unknown Resource in enableAreas script Opcode");
		}
	} else {
		error("Invalid arguments for opcode %d", op);
	}
}

void MystScriptParser::o_disableAreas(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Disable Hotspots", op);

	uint16 count = argv[0];

	if (argc == count + 1) {
		for (uint16 i = 0; i < count; i++) {
			debugC(kDebugScript, "Disable hotspot index %d", argv[i + 1]);

			MystResource *resource = 0;
			if (argv[i + 1] == 0xFFFF)
				resource = _invokingResource;
			else
				resource = _vm->_resources[argv[i + 1]];

			if (resource)
				resource->setEnabled(false);
			else
				warning("Unknown Resource in disableAreas script Opcode");
		}
	} else {
		error("Invalid arguments for opcode %d", op);
	}
}

void MystScriptParser::o_directionalUpdate(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Transition / Directional update", op);

	animatedUpdate(argc, argv, 0);
}

void MystScriptParser::o_toggleAreasActivation(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Toggle areas activation", op);

	uint16 count = argv[0];

	if (argc == count + 1) {
		for (uint16 i = 0; i < count; i++) {
			debugC(kDebugScript, "Enable/Disable hotspot index %d", argv[i + 1]);

			MystResource *resource = 0;
			if (argv[i + 1] == 0xFFFF)
				resource = _invokingResource;
			else
				resource = _vm->_resources[argv[i + 1]];

			if (resource)
				resource->setEnabled(!resource->isEnabled());
			else
				warning("Unknown Resource in toggleAreasActivation script Opcode");
		}
	} else {
		error("Invalid arguments for opcode %d", op);
	}
}

void MystScriptParser::o_playSound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 soundId = argv[0];

	debugC(kDebugScript, "Opcode %d: playSound", op);
	debugC(kDebugScript, "\tsoundId: %d", soundId);

	_vm->_sound->replaceSoundMyst(soundId);
}

void MystScriptParser::o_stopSoundBackground(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: stopSoundBackground", op);
	_vm->_sound->stopBackgroundMyst();
}

void MystScriptParser::o_playSoundBlocking(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 soundId = argv[0];

	debugC(kDebugScript, "Opcode %d: playSoundBlocking", op);
	debugC(kDebugScript, "\tsoundId: %d", soundId);

	_vm->_sound->stopSound();
	_vm->_sound->playSoundBlocking(soundId);
}

void MystScriptParser::o_copyBackBufferToScreen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Copy back buffer to screen", op);

	Common::Rect rect;
	if (argv[0] == 0xFFFF) {
		// Used in Stoneship Card 2111 (Compass Rose)
		// Used in Mechanical Card 6267 (Code Lock)
		rect = _invokingResource->getRect();
	} else {
		rect = Common::Rect(argv[0], argv[1], argv[2], argv[3]);
	}

	debugC(kDebugScript, "\trect.left: %d", rect.left);
	debugC(kDebugScript, "\trect.top: %d", rect.top);
	debugC(kDebugScript, "\trect.right: %d", rect.right);
	debugC(kDebugScript, "\trect.bottom: %d", rect.bottom);

	_vm->_gfx->copyBackBufferToScreen(rect);
	_vm->_system->updateScreen();
}

void MystScriptParser::o_copyImageToBackBuffer(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 imageId = argv[0];

	// WORKAROUND wrong image id in mechanical staircase
	if (imageId == 7158)
		imageId = 7178;

	Common::Rect srcRect = Common::Rect(argv[1], argv[2], argv[3], argv[4]);

	Common::Rect dstRect = Common::Rect(argv[5], argv[6], 544, 333);

	if (dstRect.left == -1 || dstRect.top == -1) {
		// Interpreted as full screen
		dstRect.left = 0;
		dstRect.top = 0;
	}

	dstRect.right = dstRect.left + srcRect.width();
	dstRect.bottom = dstRect.top + srcRect.height();

	debugC(kDebugScript, "Opcode %d: Copy image to back buffer", op);
	debugC(kDebugScript, "\timageId: %d", imageId);
	debugC(kDebugScript, "\tsrcRect.left: %d", srcRect.left);
	debugC(kDebugScript, "\tsrcRect.top: %d", srcRect.top);
	debugC(kDebugScript, "\tsrcRect.right: %d", srcRect.right);
	debugC(kDebugScript, "\tsrcRect.bottom: %d", srcRect.bottom);
	debugC(kDebugScript, "\tdstRect.left: %d", dstRect.left);
	debugC(kDebugScript, "\tdstRect.top: %d", dstRect.top);
	debugC(kDebugScript, "\tdstRect.right: %d", dstRect.right);
	debugC(kDebugScript, "\tdstRect.bottom: %d", dstRect.bottom);

	_vm->_gfx->copyImageSectionToBackBuffer(imageId, srcRect, dstRect);
}

// TODO: Implement common engine function for read and processing of sound blocks
//       for use by this opcode and VIEW sound block.
// TODO: Though the playSound and PlaySoundBlocking opcodes play sounds immediately,
//       this opcode changes the main background sound playing..
//       Current behavior here and with VIEW sound block is not right as demonstrated
//       by Channelwood Card 3280 (Tank Valve) and water flow sound behavior in pipe
//       on cards leading from shed...
void MystScriptParser::o_changeBackgroundSound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	int16 *soundList = NULL;
	uint16 *soundListVolume = NULL;

	// Used on Stoneship Card 2080
	// Used on Channelwood Card 3225 with argc = 8 i.e. Conditional Sound List
	if (argc == 1 || argc == 2 || argc == 8) {
		debugC(kDebugScript, "Opcode %d: Process Sound Block", op);
		uint16 decodeIdx = 0;

		int16 soundAction = argv[decodeIdx++];
		uint16 soundVolume = 65535;
		if (soundAction == kMystSoundActionChangeVolume || soundAction > 0) {
			soundVolume = argv[decodeIdx++];
		} else if (soundAction == kMystSoundActionConditional) {
			debugC(kDebugScript, "Conditional sound list");
			uint16 condVar = argv[decodeIdx++];
			uint16 condVarValue = getVar(condVar);
			uint16 condCount = argv[decodeIdx++];

			debugC(kDebugScript, "\tcondVar: %d = %d", condVar, condVarValue);
			debugC(kDebugScript, "\tcondCount: %d", condCount);

			soundList = new int16[condCount];
			soundListVolume = new uint16[condCount];

			if (condVarValue >= condCount)
				warning("Opcode %d: Conditional sound variable outside range",  op);
			else {
				for (uint16 i = 0; i < condCount; i++) {
					soundList[i] = argv[decodeIdx++];
					debugC(kDebugScript, "\t\tCondition %d: Action %d", i, soundList[i]);
					if (soundAction == kMystSoundActionChangeVolume || soundAction > 0) {
						soundListVolume[i] = argv[decodeIdx++];
					} else
						soundListVolume[i] = 65535;
					debugC(kDebugScript, "\t\tCondition %d: Volume %d", i, soundListVolume[i]);
				}

				soundAction = soundList[condVarValue];
				soundVolume = soundListVolume[condVarValue];
			}
		}

		if (soundAction == kMystSoundActionContinue)
			debugC(kDebugScript, "Continue current sound");
		else if (soundAction == kMystSoundActionChangeVolume) {
			debugC(kDebugScript, "Continue current sound, change volume");
			debugC(kDebugScript, "\tVolume: %d", soundVolume);
			_vm->_sound->changeBackgroundVolumeMyst(soundVolume);
		} else if (soundAction == kMystSoundActionStop) {
			debugC(kDebugScript, "Stop sound");
			_vm->_sound->stopBackgroundMyst();
		} else if (soundAction > 0) {
			debugC(kDebugScript, "Play new Sound, change volume");
			debugC(kDebugScript, "\tSound: %d", soundAction);
			debugC(kDebugScript, "\tVolume: %d", soundVolume);
			_vm->_sound->replaceBackgroundMyst(soundAction, soundVolume);
		} else {
			debugC(kDebugScript, "Unknown");
			warning("Unknown sound control value in opcode %d", op);
		}
	} else
		warning("Unknown arg count in opcode %d", op);

	delete[] soundList;
	soundList = NULL;
	delete[] soundListVolume;
	soundListVolume = NULL;
}

void MystScriptParser::o_soundPlaySwitch(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Switch Choice of Play Sound", op);

	uint16 value = getVar(var);

	if (value < argc) {
		uint16 soundId = argv[value];
		debugC(kDebugScript, "\tvar: %d", var);
		debugC(kDebugScript, "\tsoundId: %d", soundId);

		if (soundId)
			_vm->_sound->replaceSoundMyst(soundId);
	}
}

void MystScriptParser::o_soundResumeBackground(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: soundResumeBackground", op);
	_vm->_sound->resumeBackgroundMyst();
}

void MystScriptParser::o_copyImageToScreen(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	uint16 imageId = argv[0];

	Common::Rect srcRect = Common::Rect(argv[1], argv[2], argv[3], argv[4]);

	Common::Rect dstRect = Common::Rect(argv[5], argv[6], 544, 333);

	if (dstRect.left == -1 || dstRect.top == -1) {
		// Interpreted as full screen
		dstRect.left = 0;
		dstRect.top = 0;
	}

	dstRect.right = dstRect.left + srcRect.width();
	dstRect.bottom = dstRect.top + srcRect.height();

	debugC(kDebugScript, "Opcode %d: Copy image to screen", op);
	debugC(kDebugScript, "\timageId: %d", imageId);
	debugC(kDebugScript, "\tsrcRect.left: %d", srcRect.left);
	debugC(kDebugScript, "\tsrcRect.top: %d", srcRect.top);
	debugC(kDebugScript, "\tsrcRect.right: %d", srcRect.right);
	debugC(kDebugScript, "\tsrcRect.bottom: %d", srcRect.bottom);
	debugC(kDebugScript, "\tdstRect.left: %d", dstRect.left);
	debugC(kDebugScript, "\tdstRect.top: %d", dstRect.top);
	debugC(kDebugScript, "\tdstRect.right: %d", dstRect.right);
	debugC(kDebugScript, "\tdstRect.bottom: %d", dstRect.bottom);

	_vm->_gfx->copyImageSectionToScreen(imageId, srcRect, dstRect);
	_vm->_system->updateScreen();
}

void MystScriptParser::o_changeCard(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Change Card", op);

	uint16 cardId = argv[0];

	// Argument 1 if present is not used
	// uint16 u0 = argv[1];

	debugC(kDebugScript, "\tTarget Card: %d", cardId);
	//debugC(kDebugScript, "\tu0: %d", u0); // Unused data

	_vm->changeToCard(cardId, true);
}

void MystScriptParser::o_drawImageChangeCard(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
		debugC(kDebugScript, "Opcode %d: Draw Full Screen Image, Delay then Change Card", op);

		uint16 imageId = argv[0];
		uint16 cardId = argv[1];
		// argv[2] is not used in the original engine

		debugC(kDebugScript, "\timageId: %d", imageId);
		debugC(kDebugScript, "\tcardId: %d", cardId);

		_vm->_gfx->copyImageToScreen(imageId, Common::Rect(0, 0, 544, 333));
		_vm->_system->updateScreen();

		_vm->changeToCard(cardId, true);
}

void MystScriptParser::o_changeMainCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Change main cursor", op);

	uint16 cursorId = argv[0];

	debugC(kDebugScript, "Cursor: %d", cursorId);

	_vm->setMainCursor(cursorId);
	_vm->_cursor->setCursor(cursorId);
}

void MystScriptParser::o_hideCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Hide Cursor", op);
	_vm->_cursor->hideCursor();
}

void MystScriptParser::o_showCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Show Cursor", op);
	_vm->_cursor->showCursor();
}

void MystScriptParser::o_delay(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Mechanical Card 6327 (Elevator)
	debugC(kDebugScript, "Opcode %d: Delay", op);

	uint16 time = argv[0];

	debugC(kDebugScript, "\tTime: %d", time);

	_vm->_system->delayMillis(time);
}

void MystScriptParser::o_changeStack(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: changeStack", op);

	uint16 targetStack = argv[0];
	uint16 soundIdLinkSrc = argv[1];
	uint16 soundIdLinkDst = argv[2];

	debugC(kDebugScript, "\tTarget Stack: %d", targetStack);
	debugC(kDebugScript, "\tSource Stack Link Sound: %d", soundIdLinkSrc);
	debugC(kDebugScript, "\tDestination Stack Link Sound: %d", soundIdLinkDst);

	_vm->_sound->stopSound();

	if (_vm->getFeatures() & GF_DEMO) {
		// No need to have a table for just this data...
		if (targetStack == 1)
			_vm->changeToStack(kDemoSlidesStack, 1000, soundIdLinkSrc, soundIdLinkDst);
		else if (targetStack == 2)
			_vm->changeToStack(kDemoPreviewStack, 3000, soundIdLinkSrc, soundIdLinkDst);
	} else {
		_vm->changeToStack(_stackMap[targetStack], _startCard[targetStack], soundIdLinkSrc, soundIdLinkDst);
	}
}

void MystScriptParser::o_changeCardPlaySoundDirectional(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play Sound, Change Card and Directional Update Screen Region", op);

	uint16 cardId = argv[0];
	uint16 soundId = argv[1];
	uint16 delayBetweenSteps = argv[2];
	uint16 dataSize = argv[3];

	debugC(kDebugScript, "\tcard: %d", cardId);
	debugC(kDebugScript, "\tsound: %d", soundId);
	debugC(kDebugScript, "\tdelay between steps: %d", delayBetweenSteps);
	debugC(kDebugScript, "\tanimated update data size: %d", dataSize);

	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);

	_vm->changeToCard(cardId, false);

	animatedUpdate(dataSize, &argv[4], delayBetweenSteps);
}

void MystScriptParser::o_directionalUpdatePlaySound(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Play Sound and Directional Update Screen Region", op);

	uint16 soundId = argv[0];
	uint16 delayBetweenSteps = argv[1];
	uint16 dataSize = argv[2];

	debugC(kDebugScript, "\tsound: %d", soundId);
	debugC(kDebugScript, "\tdelay between steps: %d", delayBetweenSteps);
	debugC(kDebugScript, "\tanimated update data size: %d", dataSize);

	if (soundId)
		_vm->_sound->replaceSoundMyst(soundId);

	animatedUpdate(dataSize, &argv[3], delayBetweenSteps);
}

void MystScriptParser::o_saveMainCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Save main cursor", op);

	_savedCursorId = _vm->getMainCursor();
}

void MystScriptParser::o_restoreMainCursor(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	debugC(kDebugScript, "Opcode %d: Restore main cursor", op);

	_vm->setMainCursor(_savedCursorId);
}

void MystScriptParser::o_soundWaitStop(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	// Used on Selenitic Card 1191 (Maze Runner)
	// Used on Mechanical Card 6267 (Code Lock)
	// Used when Button is pushed...
	debugC(kDebugScript, "Opcode %d: Wait for foreground sound to finish", op);

	while (_vm->_sound->isPlaying())
		_vm->_system->delayMillis(10);
}

void MystScriptParser::o_quit(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_vm->quitGame();
}

void MystScriptParser::showMap() {
	if (_vm->getCurCard() != getMap()) {
		_savedMapCardId = _vm->getCurCard();
		_vm->changeToCard(getMap(), true);
	}
}

void MystScriptParser::o_exitMap(uint16 op, uint16 var, uint16 argc, uint16 *argv) {
	_vm->changeToCard(_savedMapCardId, true);
}

} // End of namespace Mohawk
