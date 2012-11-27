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
#include "mohawk/riven.h"
#include "mohawk/riven_external.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_scripts.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "common/memstream.h"
#include "common/stream.h"
#include "common/system.h"

namespace Mohawk {

RivenScript::RivenScript(MohawkEngine_Riven *vm, Common::SeekableReadStream *stream, uint16 scriptType, uint16 parentStack, uint16 parentCard)
	: _vm(vm), _stream(stream), _scriptType(scriptType), _parentStack(parentStack), _parentCard(parentCard) {
	setupOpcodes();
	_isRunning = _continueRunning = false;
}

RivenScript::~RivenScript() {
	delete _stream;
}

uint32 RivenScript::calculateCommandSize(Common::SeekableReadStream* script) {
	uint16 command = script->readUint16BE();
	uint32 commandSize = 2;
	if (command == 8) {
		if (script->readUint16BE() != 2) // Arg count?
			warning ("if-then-else unknown value is not 2");
		script->readUint16BE();								// variable to check against
		uint16 logicBlockCount = script->readUint16BE();	// number of logic blocks
		commandSize += 6;									// 2 + variable + logicBlocks

		for (uint16 i = 0; i < logicBlockCount; i++) {
			script->readUint16BE(); // Block variable
			uint16 logicBlockLength = script->readUint16BE();
			commandSize += 4;
			for (uint16 j = 0; j < logicBlockLength; j++)
				commandSize += calculateCommandSize(script);
		}
	} else {
		uint16 argCount = script->readUint16BE();
		commandSize += 2;
		for (uint16 i = 0; i < argCount; i++) {
			script->readUint16BE();
			commandSize += 2;
		}
	}

	return commandSize;
}

uint32 RivenScript::calculateScriptSize(Common::SeekableReadStream* script) {
	uint32 oldPos = script->pos();
	uint16 commandCount = script->readUint16BE();
	uint16 scriptSize = 2; // 2 for command count

	for (uint16 i = 0; i < commandCount; i++)
		scriptSize += calculateCommandSize(script);

	script->seek(oldPos);
	return scriptSize;
}

#define OPCODE(x) { &RivenScript::x, #x }

void RivenScript::setupOpcodes() {
	static const RivenOpcode riven_opcodes[] = {
		// 0x00 (0 decimal)
		OPCODE(empty),
		OPCODE(drawBitmap),
		OPCODE(switchCard),
		OPCODE(playScriptSLST),
		// 0x04 (4 decimal)
		OPCODE(playSound),
		OPCODE(empty),						// Empty
		OPCODE(empty),						// Complex animation (not used)
		OPCODE(setVariable),
		// 0x08 (8 decimal)
		OPCODE(mohawkSwitch),
		OPCODE(enableHotspot),
		OPCODE(disableHotspot),
		OPCODE(empty),						// Empty
		// 0x0C (12 decimal)
		OPCODE(stopSound),
		OPCODE(changeCursor),
		OPCODE(delay),
		OPCODE(empty),						// Empty
		// 0x10 (16 decimal)
		OPCODE(empty),						// Empty
		OPCODE(runExternalCommand),
		OPCODE(transition),
		OPCODE(refreshCard),
		// 0x14 (20 decimal)
		OPCODE(disableScreenUpdate),
		OPCODE(enableScreenUpdate),
		OPCODE(empty),						// Empty
		OPCODE(empty),						// Empty
		// 0x18 (24 decimal)
		OPCODE(incrementVariable),
		OPCODE(empty),						// Empty
		OPCODE(empty),						// Empty
		OPCODE(changeStack),
		// 0x1C (28 decimal)
		OPCODE(disableMovie),
		OPCODE(disableAllMovies),
		OPCODE(empty),						// Set movie rate (not used)
		OPCODE(enableMovie),
		// 0x20 (32 decimal)
		OPCODE(playMovieBlocking),
		OPCODE(playMovie),
		OPCODE(stopMovie),
		OPCODE(empty),						// Start a water effect (not used)
		// 0x24 (36 decimal)
		OPCODE(unk_36),						// Unknown
		OPCODE(fadeAmbientSounds),
		OPCODE(storeMovieOpcode),
		OPCODE(activatePLST),
		// 0x28 (40 decimal)
		OPCODE(activateSLST),
		OPCODE(activateMLSTAndPlay),
		OPCODE(empty),						// Empty
		OPCODE(activateBLST),
		// 0x2C (44 decimal)
		OPCODE(activateFLST),
		OPCODE(zipMode),
		OPCODE(activateMLST),
		OPCODE(empty)                       // Activate an SLST with a volume parameter (not used)
	};

	_opcodes = riven_opcodes;
}

static void printTabs(byte tabs) {
	for (byte i = 0; i < tabs; i++)
		debugN("\t");
}

void RivenScript::dumpScript(const Common::StringArray &varNames, const Common::StringArray &xNames, byte tabs) {
	if (_stream->pos() != 0)
		_stream->seek(0);

	printTabs(tabs); debugN("Stream Type %d:\n", _scriptType);
	dumpCommands(varNames, xNames, tabs + 1);
}

void RivenScript::dumpCommands(const Common::StringArray &varNames, const Common::StringArray &xNames, byte tabs) {
	uint16 commandCount = _stream->readUint16BE();

	for (uint16 i = 0; i < commandCount; i++) {
		uint16 command = _stream->readUint16BE();

		if (command == 8) { // "Switch" Statement
			if (_stream->readUint16BE() != 2)
				warning ("if-then-else unknown value is not 2");
			uint16 var = _stream->readUint16BE();
			printTabs(tabs); debugN("switch (%s) {\n", varNames[var].c_str());
			uint16 logicBlockCount = _stream->readUint16BE();
			for (uint16 j = 0; j < logicBlockCount; j++) {
				uint16 varCheck = _stream->readUint16BE();
				printTabs(tabs + 1);
				if (varCheck == 0xFFFF)
					debugN("default:\n");
				else
					debugN("case %d:\n", varCheck);
				dumpCommands(varNames, xNames, tabs + 2);
				printTabs(tabs + 2); debugN("break;\n");
			}
			printTabs(tabs); debugN("}\n");
		} else if (command == 7) { // Use the variable name
			_stream->readUint16BE(); // Skip the opcode var count
			printTabs(tabs);
			uint16 var = _stream->readUint16BE();
			debugN("%s = %d;\n", varNames[var].c_str(), _stream->readUint16BE());
		} else if (command == 17) { // Use the external command name
			_stream->readUint16BE(); // Skip the opcode var count
			printTabs(tabs);
			debugN("%s(", xNames[_stream->readUint16BE()].c_str());
			uint16 varCount = _stream->readUint16BE();
			for (uint16 j = 0; j < varCount; j++) {
				debugN("%d", _stream->readUint16BE());
				if (j != varCount - 1)
					debugN(", ");
			}
			debugN(");\n");
		} else if (command == 24) { // Use the variable name
			_stream->readUint16BE(); // Skip the opcode var count
			printTabs(tabs);
			uint16 var = _stream->readUint16BE();
			debugN("%s += %d;\n", varNames[var].c_str(), _stream->readUint16BE());
		} else {
			printTabs(tabs);
			uint16 varCount = _stream->readUint16BE();
			debugN("%s(", _opcodes[command].desc);
			for (uint16 j = 0; j < varCount; j++) {
				debugN("%d", _stream->readUint16BE());
				if (j != varCount - 1)
					debugN(", ");
			}
			debugN(");\n");
		}
	}
}

void RivenScript::runScript() {
	_isRunning = _continueRunning = true;

	if (_stream->pos() != 0)
		_stream->seek(0);

	processCommands(true);
	_isRunning = false;
}

void RivenScript::processCommands(bool runCommands) {
	bool runBlock = true;

	uint16 commandCount = _stream->readUint16BE();

	for (uint16 j = 0; j < commandCount && !_vm->shouldQuit() && _stream->pos() < _stream->size() && _continueRunning; j++) {
		uint16 command = _stream->readUint16BE();

		if (command == 8) {
			// Command 8 contains a conditional branch, similar to switch statements
			if (_stream->readUint16BE() != 2)
				warning("if-then-else unknown value is not 2");

			uint16 var = _stream->readUint16BE();				// variable to check against
			uint16 logicBlockCount = _stream->readUint16BE();	// number of logic blocks
			bool anotherBlockEvaluated = false;

			for (uint16 k = 0; k < logicBlockCount; k++) {
				uint16 checkValue = _stream->readUint16BE();	// variable for this logic block

				// Run the following block if the block's variable is equal to the variable to check against
				// Don't run it if the parent block is not executed
				// And don't run it if another block has already evaluated to true (needed for the default case)
				runBlock = (_vm->getStackVar(var) == checkValue || checkValue == 0xffff) && runCommands && !anotherBlockEvaluated;
				processCommands(runBlock);

				if (runBlock)
					anotherBlockEvaluated = true;
			}
		} else {
			uint16 argCount = _stream->readUint16BE();
			uint16 *argValues = new uint16[argCount];

			for (uint16 k = 0; k < argCount; k++)
				argValues[k] = _stream->readUint16BE();

			if (runCommands) {
				debug (4, "Running opcode %04x, argument count %d", command, argCount);
				(this->*(_opcodes[command].proc)) (command, argCount, argValues);
			}

			delete[] argValues;
		}
	}
}

////////////////////////////////
// Opcodes
////////////////////////////////

// Command 1: draw tBMP resource (tbmp_id, left, top, right, bottom, u0, u1, u2, u3)
void RivenScript::drawBitmap(uint16 op, uint16 argc, uint16 *argv) {
	if (argc < 5) // Copy the image to the whole screen, ignoring the rest of the parameters
		_vm->_gfx->copyImageToScreen(argv[0], 0, 0, 608, 392);
	else          // Copy the image to a certain part of the screen
		_vm->_gfx->copyImageToScreen(argv[0], argv[1], argv[2], argv[3], argv[4]);

	// Now, update the screen
	_vm->_gfx->updateScreen();
}

// Command 2: go to card (card id)
void RivenScript::switchCard(uint16 op, uint16 argc, uint16 *argv) {
	_vm->changeToCard(argv[0]);

	// WORKAROUND: If we changed card on a mouse down event,
	// we want to ignore the next mouse up event so we don't
	// change card when lifting the mouse on the next card.
	if (_scriptType == kMouseDownScript)
		_vm->ignoreNextMouseUp();
}

// Command 3: play an SLST from the script
void RivenScript::playScriptSLST(uint16 op, uint16 argc, uint16 *argv) {
	SLSTRecord slstRecord;
	int offset = 0, j = 0;

	slstRecord.index = 0;		// not set by the scripts, so we set it to 0
	slstRecord.sound_count = argv[0];
	slstRecord.sound_ids = new uint16[slstRecord.sound_count];

	offset = slstRecord.sound_count;

	for (j = 0; j < slstRecord.sound_count; j++)
		slstRecord.sound_ids[j] = argv[offset++];
	slstRecord.fade_flags = argv[offset++];
	slstRecord.loop = argv[offset++];
	slstRecord.global_volume = argv[offset++];
	slstRecord.u0 = argv[offset++];
	slstRecord.u1 = argv[offset++];

	slstRecord.volumes = new uint16[slstRecord.sound_count];
	slstRecord.balances = new int16[slstRecord.sound_count];
	slstRecord.u2 = new uint16[slstRecord.sound_count];

	for (j = 0; j < slstRecord.sound_count; j++)
		slstRecord.volumes[j] = argv[offset++];

	for (j = 0; j < slstRecord.sound_count; j++)
		slstRecord.balances[j] = argv[offset++];	// negative = left, 0 = center, positive = right

	for (j = 0; j < slstRecord.sound_count; j++)
		slstRecord.u2[j] = argv[offset++];			// Unknown

	// Play the requested sound list
	_vm->_sound->playSLST(slstRecord);
	_vm->_activatedSLST = true;

	delete[] slstRecord.sound_ids;
	delete[] slstRecord.volumes;
	delete[] slstRecord.balances;
	delete[] slstRecord.u2;
}

// Command 4: play local tWAV resource (twav_id, volume, block)
void RivenScript::playSound(uint16 op, uint16 argc, uint16 *argv) {
	byte volume = Sound::convertRivenVolume(argv[1]);

	if (argv[2] == 1)
		_vm->_sound->playSoundBlocking(argv[0], volume);
	else
		_vm->_sound->playSound(argv[0], volume);
}

// Command 7: set variable value (variable, value)
void RivenScript::setVariable(uint16 op, uint16 argc, uint16 *argv) {
	_vm->getStackVar(argv[0]) = argv[1];
}

// Command 8: conditional branch
void RivenScript::mohawkSwitch(uint16 op, uint16 argc, uint16 *argv) {
	// dummy function, this opcode does logic checking in processCommands()
}

// Command 9: enable hotspot (blst_id)
void RivenScript::enableHotspot(uint16 op, uint16 argc, uint16 *argv) {
	for (uint16 i = 0; i < _vm->getHotspotCount(); i++) {
		if (_vm->_hotspots[i].blstID == argv[0]) {
			debug(2, "Enabling hotspot with BLST ID %d", argv[0]);
			_vm->_hotspots[i].enabled = true;
		}
	}

	// Recheck our current hotspot because it may have now changed
	_vm->updateCurrentHotspot();
}

// Command 10: disable hotspot (blst_id)
void RivenScript::disableHotspot(uint16 op, uint16 argc, uint16 *argv) {
	for (uint16 i = 0; i < _vm->getHotspotCount(); i++) {
		if (_vm->_hotspots[i].blstID == argv[0]) {
			debug(2, "Disabling hotspot with BLST ID %d", argv[0]);
			_vm->_hotspots[i].enabled = false;
		}
	}

	// Recheck our current hotspot because it may have now changed
	_vm->updateCurrentHotspot();
}

// Command 12: stop sounds (flags)
void RivenScript::stopSound(uint16 op, uint16 argc, uint16 *argv) {
	// WORKAROUND: The Play Riven/Visit Riven/Start New Game buttons
	// in the main menu call this function to stop ambient sounds
	// after the change stack call to Temple Island. However, this
	// would cause all ambient sounds not to play. An alternative
	// fix would be to stop all scripts on a stack change, but this
	// does fine for now.
	if (_vm->getCurStack() == tspit && (_vm->getCurCardRMAP() == 0x6e9a || _vm->getCurCardRMAP() == 0xfeeb))
		return;

	// The argument is a bitflag for the setting.
	// bit 0 is normal sound stopping
	// bit 1 is ambient sound stopping
	// Having no flags set means clear all
	if (argv[0] & 2 || argv[0] == 0)
		_vm->_sound->stopAllSLST();

	if (argv[0] & 1 || argv[0] == 0)
		_vm->_sound->stopSound();
}

// Command 13: set mouse cursor (cursor_id)
void RivenScript::changeCursor(uint16 op, uint16 argc, uint16 *argv) {
	debug(2, "Change to cursor %d", argv[0]);
	_vm->_cursor->setCursor(argv[0]);
	_vm->_system->updateScreen();
}

// Command 14: pause script execution (delay in ms, u1)
void RivenScript::delay(uint16 op, uint16 argc, uint16 *argv) {
	debug(2, "Delay %dms", argv[0]);
	if (argv[0] > 0)
		_vm->delayAndUpdate(argv[0]);
}

// Command 17: call external command
void RivenScript::runExternalCommand(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_externalScriptHandler->runCommand(argc, argv);
}

// Command 18: transition
// Note that this opcode has 1 or 5 parameters, depending on argc
// Parameter 0: transition type
// Parameters 1-4: transition rectangle
void RivenScript::transition(uint16 op, uint16 argc, uint16 *argv) {
	if (argc == 1)
		_vm->_gfx->scheduleTransition(argv[0]);
	else
		_vm->_gfx->scheduleTransition(argv[0], Common::Rect(argv[1], argv[2], argv[3], argv[4]));
}

// Command 19: reload card
void RivenScript::refreshCard(uint16 op, uint16 argc, uint16 *argv) {
	debug(2, "Refreshing card");
	_vm->refreshCard();
}

// Command 20: disable screen update
void RivenScript::disableScreenUpdate(uint16 op, uint16 argc, uint16 *argv) {
	debug(2, "Screen update disabled");
	_vm->_gfx->_updatesEnabled = false;
}

// Command 21: enable screen update
void RivenScript::enableScreenUpdate(uint16 op, uint16 argc, uint16 *argv) {
	debug(2, "Screen update enabled");
	_vm->_gfx->_updatesEnabled = true;
	_vm->_gfx->updateScreen();
}

// Command 24: increment variable (variable, value)
void RivenScript::incrementVariable(uint16 op, uint16 argc, uint16 *argv) {
	_vm->getStackVar(argv[0]) += argv[1];
}

// Command 27: go to stack (stack name, code high, code low)
void RivenScript::changeStack(uint16 op, uint16 argc, uint16 *argv) {
	Common::String stackName = _vm->getName(StackNames, argv[0]);
	int8 index = -1;

	for (byte i = 0; i < 8; i++)
		if (_vm->getStackName(i).equalsIgnoreCase(stackName)) {
			index = i;
			break;
		}

	if (index == -1)
		error ("'%s' is not a stack name!", stackName.c_str());

	_vm->changeToStack(index);
	uint32 rmapCode = (argv[1] << 16) + argv[2];
	uint16 cardID = _vm->matchRMAPToCard(rmapCode);
	_vm->changeToCard(cardID);
}

// Command 28: disable a movie
void RivenScript::disableMovie(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_video->disableMovieRiven(argv[0]);
}

// Command 29: disable all movies
void RivenScript::disableAllMovies(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_video->disableAllMovies();
}

// Command 31: enable a movie
void RivenScript::enableMovie(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_video->enableMovieRiven(argv[0]);
}

// Command 32: play foreground movie - blocking (movie_id)
void RivenScript::playMovieBlocking(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_cursor->hideCursor();
	_vm->_video->playMovieBlockingRiven(argv[0]);
	_vm->_cursor->showCursor();
}

// Command 33: play background movie - nonblocking (movie_id)
void RivenScript::playMovie(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_video->playMovieRiven(argv[0]);
}

// Command 34: stop a movie
void RivenScript::stopMovie(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_video->stopMovieRiven(argv[0]);
}

// Command 36: unknown
void RivenScript::unk_36(uint16 op, uint16 argc, uint16 *argv) {
	debug(0, "unk_36: Ignoring");
}

// Command 37: fade ambient sounds
void RivenScript::fadeAmbientSounds(uint16 op, uint16 argc, uint16 *argv) {
	// Similar to stopSound(), but does fading
	_vm->_sound->stopAllSLST(true);
}

// Command 38: Store an opcode for use when playing a movie (movie id, time high, time low, opcode, arguments...)
void RivenScript::storeMovieOpcode(uint16 op, uint16 argc, uint16 *argv) {
	// This opcode is used to delay an opcode's usage based on the elapsed
	// time of a specified movie. However, every use in the game is for
	// delaying an activateSLST opcode.

	uint32 scriptSize = 6 + (argc - 4) * 2;

	// Create our dummy script
	byte *scriptBuf = (byte *)malloc(scriptSize);
	WRITE_BE_UINT16(scriptBuf, 1);            // One command
	WRITE_BE_UINT16(scriptBuf + 2, argv[3]);  // One opcode
	WRITE_BE_UINT16(scriptBuf + 4, argc - 4); // argc - 4 args

	for (int i = 0; i < argc - 4; i++)
		WRITE_BE_UINT16(scriptBuf + 6 + (i * 2), argv[i + 4]);

	// Build a script out of 'er
	Common::SeekableReadStream *scriptStream = new Common::MemoryReadStream(scriptBuf, scriptSize, DisposeAfterUse::YES);
	RivenScript *script = new RivenScript(_vm, scriptStream, kStoredOpcodeScript, getParentStack(), getParentCard());

	uint32 delayTime = (argv[1] << 16) + argv[2];

	if (delayTime > 0) {
		// Store the script
		RivenScriptManager::StoredMovieOpcode storedOp;
		storedOp.script = script;
		storedOp.time = delayTime;
		storedOp.id = argv[0];

		// Store the opcode for later
		_vm->_scriptMan->setStoredMovieOpcode(storedOp);
	} else {
		// Run immediately if we have no delay
		script->runScript();
		delete script;
	}
}

// Command 39: activate PLST record (card picture lists)
void RivenScript::activatePLST(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_gfx->drawPLST(argv[0]);

	// An update is automatically sent here as long as it's not a load or update script and updates are enabled.
	if (_scriptType != kCardLoadScript && _scriptType != kCardUpdateScript)
		_vm->_gfx->updateScreen();
}

// Command 40: activate SLST record (card ambient sound lists)
void RivenScript::activateSLST(uint16 op, uint16 argc, uint16 *argv) {
	// WORKAROUND: Disable the SLST that is played during Riven's intro.
	// Riven X does this too (spoke this over with Jeff)
	if (_vm->getCurStack() == tspit && _vm->getCurCardRMAP() == 0x6e9a && argv[0] == 2)
		return;

	_vm->_sound->playSLST(argv[0], _vm->getCurCard());
	_vm->_activatedSLST = true;
}

// Command 41: activate MLST record and play
void RivenScript::activateMLSTAndPlay(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_video->activateMLST(argv[0], _vm->getCurCard());
	_vm->_video->playMovieRiven(argv[0]);
}

// Command 43: activate BLST record (card hotspot enabling lists)
void RivenScript::activateBLST(uint16 op, uint16 argc, uint16 *argv) {
	Common::SeekableReadStream* blst = _vm->getResource(ID_BLST, _vm->getCurCard());
	uint16 recordCount = blst->readUint16BE();

	for (uint16 i = 0; i < recordCount; i++) {
		uint16 index = blst->readUint16BE();	// record index
		uint16 enabled = blst->readUint16BE();
		uint16 hotspotID = blst->readUint16BE();

		if (argv[0] == index)
			for (uint16 j = 0; j < _vm->getHotspotCount(); j++)
				if (_vm->_hotspots[j].blstID == hotspotID)
					_vm->_hotspots[j].enabled = (enabled == 1);
	}

	delete blst;

	// Recheck our current hotspot because it may have now changed
	_vm->updateCurrentHotspot();
}

// Command 44: activate FLST record (information on which SFXE resource this card should use)
void RivenScript::activateFLST(uint16 op, uint16 argc, uint16 *argv) {
	Common::SeekableReadStream* flst = _vm->getResource(ID_FLST, _vm->getCurCard());
	uint16 recordCount = flst->readUint16BE();

	for (uint16 i = 0; i < recordCount; i++) {
		uint16 index = flst->readUint16BE();
		uint16 sfxeID = flst->readUint16BE();

		if (flst->readUint16BE() != 0)
			warning("FLST u0 non-zero");

		if (index == argv[0]) {
			_vm->_gfx->scheduleWaterEffect(sfxeID);
			break;
		}
	}

	delete flst;
}

// Command 45: do zip mode
void RivenScript::zipMode(uint16 op, uint16 argc, uint16 *argv) {
	// Check the ZIPS records to see if we have a match to the hotspot name
	Common::String hotspotName = _vm->getHotspotName(_vm->getCurHotspot());

	for (uint16 i = 0; i < _vm->_zipModeData.size(); i++)
		if (_vm->_zipModeData[i].name == hotspotName) {
			_vm->changeToCard(_vm->_zipModeData[i].id);
			return;
		}
}

// Command 46: activate MLST record (movie lists)
void RivenScript::activateMLST(uint16 op, uint16 argc, uint16 *argv) {
	_vm->_video->activateMLST(argv[0], _vm->getCurCard());
}

RivenScriptManager::RivenScriptManager(MohawkEngine_Riven *vm) {
	_vm = vm;
	_storedMovieOpcode.script = 0;
	_storedMovieOpcode.time = 0;
	_storedMovieOpcode.id = 0;
}

RivenScriptManager::~RivenScriptManager() {
	for (uint32 i = 0; i < _currentScripts.size(); i++)
		delete _currentScripts[i];

	clearStoredMovieOpcode();
}

RivenScriptList RivenScriptManager::readScripts(Common::SeekableReadStream *stream, bool garbageCollect) {
	if (garbageCollect)
		unloadUnusedScripts(); // Garbage collect!

	RivenScriptList scriptList;

	uint16 scriptCount = stream->readUint16BE();
	for (uint16 i = 0; i < scriptCount; i++) {
		uint16 scriptType = stream->readUint16BE();
		uint32 scriptSize = RivenScript::calculateScriptSize(stream);
		RivenScript *script = new RivenScript(_vm, stream->readStream(scriptSize), scriptType, _vm->getCurStack(), _vm->getCurCard());
		scriptList.push_back(script);

		// Only add it to the scripts that we will free later if it is requested.
		// (ie. we don't want to store scripts from the dumpScript console command)
		if (garbageCollect)
			_currentScripts.push_back(script);
	}

	return scriptList;
}

void RivenScriptManager::stopAllScripts() {
	for (uint32 i = 0; i < _currentScripts.size(); i++)
		_currentScripts[i]->stopRunning();
}

void RivenScriptManager::unloadUnusedScripts() {
	// Free any scripts that aren't part of the current card and aren't running
	for (uint32 i = 0; i < _currentScripts.size(); i++) {
		if ((_vm->getCurStack() != _currentScripts[i]->getParentStack() || _vm->getCurCard() != _currentScripts[i]->getParentCard()) && !_currentScripts[i]->isRunning()) {
			delete _currentScripts[i];
			_currentScripts.remove_at(i);
			i--;
		}
	}
}

void RivenScriptManager::setStoredMovieOpcode(const StoredMovieOpcode &op) {
	clearStoredMovieOpcode();
	_storedMovieOpcode.script = op.script;
	_storedMovieOpcode.id = op.id;
	_storedMovieOpcode.time = op.time;
}

void RivenScriptManager::runStoredMovieOpcode() {
	if (_storedMovieOpcode.script) {
		_storedMovieOpcode.script->runScript();
		clearStoredMovieOpcode();
	}
}

void RivenScriptManager::clearStoredMovieOpcode() {
	delete _storedMovieOpcode.script;
	_storedMovieOpcode.script = 0;
	_storedMovieOpcode.time = 0;
	_storedMovieOpcode.id = 0;
}

} // End of namespace Mohawk
