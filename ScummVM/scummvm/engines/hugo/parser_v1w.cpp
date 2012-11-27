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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// parser.c - handles all keyboard/command input

#include "common/debug.h"

#include "hugo/hugo.h"
#include "hugo/parser.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/route.h"
#include "hugo/display.h"
#include "hugo/util.h"
#include "hugo/sound.h"
#include "hugo/object.h"
#include "hugo/text.h"
#include "hugo/inventory.h"

namespace Hugo {
Parser_v1w::Parser_v1w(HugoEngine *vm) : Parser_v3d(vm) {
}

Parser_v1w::~Parser_v1w() {
}

/**
 * Parse the user's line of text input.  Generate events as necessary
 */
void Parser_v1w::lineHandler() {
	debugC(1, kDebugParser, "lineHandler()");

	Status &gameStatus = _vm->getGameStatus();

	// Toggle God Mode
	if (!strncmp(_vm->_line, "PPG", 3)) {
		_vm->_sound->playSound(!_vm->_soundTest, kSoundPriorityHigh);
		gameStatus._godModeFl = !gameStatus._godModeFl;
		return;
	}

	Utils::strlwr(_vm->_line);                      // Convert to lower case

	// God Mode cheat commands:
	// goto <screen>                                Takes hero to named screen
	// fetch <object name>                          Hero carries named object
	// fetch all                                    Hero carries all possible objects
	// find <object name>                           Takes hero to screen containing named object
	if (gameStatus._godModeFl) {
		// Special code to allow me to go straight to any screen
		if (strstr(_vm->_line, "goto")) {
			for (int i = 0; i < _vm->_numScreens; i++) {
				if (!scumm_stricmp(&_vm->_line[strlen("goto") + 1], _vm->_text->getScreenNames(i))) {
					_vm->_scheduler->newScreen(i);
					return;
				}
			}
		}

		// Special code to allow me to get objects from anywhere
		if (strstr(_vm->_line, "fetch all")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (_vm->_object->_objects[i]._genericCmd & TAKE)
					takeObject(&_vm->_object->_objects[i]);
			}
			return;
		}

		if (strstr(_vm->_line, "fetch")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (!scumm_stricmp(&_vm->_line[strlen("fetch") + 1], _vm->_text->getNoun(_vm->_object->_objects[i]._nounIndex, 0))) {
					takeObject(&_vm->_object->_objects[i]);
					return;
				}
			}
		}

		// Special code to allow me to goto objects
		if (strstr(_vm->_line, "find")) {
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				if (!scumm_stricmp(&_vm->_line[strlen("find") + 1], _vm->_text->getNoun(_vm->_object->_objects[i]._nounIndex, 0))) {
					_vm->_scheduler->newScreen(_vm->_object->_objects[i]._screenIndex);
					return;
				}
			}
		}
	}

	// Special meta commands
	// EXIT/QUIT
	if (!strcmp("exit", _vm->_line) || strstr(_vm->_line, "quit")) {
		if (Utils::yesNoBox(_vm->_text->getTextParser(kTBExit_1d)))
			_vm->endGame();
		return;
	}

	// SAVE/RESTORE
	if (!strcmp("save", _vm->_line) && gameStatus._viewState == kViewPlay) {
		_vm->_file->saveGame(-1, Common::String());
		return;
	}

	if (!strcmp("restore", _vm->_line) && (gameStatus._viewState == kViewPlay || gameStatus._viewState == kViewIdle)) {
		_vm->_file->restoreGame(-1);
		return;
	}

	// Empty line
	if (*_vm->_line == '\0')                        // Empty line
		return;
	if (strspn(_vm->_line, " ") == strlen(_vm->_line)) // Nothing but spaces!
		return;

	if (gameStatus._gameOverFl) {
		// No commands allowed!
		_vm->gameOverMsg();
		return;
	}

	char farComment[kCompLineSize * 5] = "";        // hold 5 line comment if object not nearby

	// Test for nearby objects referenced explicitly
	for (int i = 0; i < _vm->_object->_numObj; i++) {
		Object *obj = &_vm->_object->_objects[i];
		if (isWordPresent(_vm->_text->getNounArray(obj->_nounIndex))) {
			if (isObjectVerb_v3(obj, farComment) || isGenericVerb_v3(obj, farComment))
				return;
		}
	}

	// Test for nearby objects that only require a verb
	// Note comment is unused if not near.
	for (int i = 0; i < _vm->_object->_numObj; i++) {
		Object *obj = &_vm->_object->_objects[i];
		if (obj->_verbOnlyFl) {
			char contextComment[kCompLineSize * 5] = ""; // Unused comment for context objects
			if (isObjectVerb_v3(obj, contextComment) || isGenericVerb_v3(obj, contextComment))
				return;
		}
	}

	// No objects match command line, try background and catchall commands
	if (isBackgroundWord_v3(_backgroundObjects[*_vm->_screenPtr]))
		return;
	if (isCatchallVerb_v3(_backgroundObjects[*_vm->_screenPtr]))
		return;

	if (isBackgroundWord_v3(_catchallList))
		return;
	if (isCatchallVerb_v3(_catchallList))
		return;

	// If a not-near comment was generated, print it
	if (*farComment != '\0') {
		Utils::notifyBox(farComment);
		return;
	}

	// Nothing matches.  Report recognition success to user.
	const char *verb = findVerb();
	const char *noun = findNoun();
	if (verb == _vm->_text->getVerb(_vm->_look, 0) && _vm->_maze._enabledFl) {
		Utils::notifyBox(_vm->_text->getTextParser(kTBMaze));
		_vm->_object->showTakeables();
	} else if (verb && noun) {                      // A combination I didn't think of
		Utils::notifyBox(_vm->_text->getTextParser(kTBNoPoint));
	} else if (noun) {
		Utils::notifyBox(_vm->_text->getTextParser(kTBNoun));
	} else if (verb) {
		Utils::notifyBox(_vm->_text->getTextParser(kTBVerb));
	} else {
		Utils::notifyBox(_vm->_text->getTextParser(kTBEh));
	}
}

void Parser_v1w::showInventory() const {
	Status &gameStatus = _vm->getGameStatus();
	Istate inventState = _vm->_inventory->getInventoryState();
	if (gameStatus._gameOverFl) {
		_vm->gameOverMsg();
	} else if ((inventState == kInventoryOff) && (gameStatus._viewState == kViewPlay)) {
		_vm->_inventory->setInventoryState(kInventoryDown);
		gameStatus._viewState = kViewInvent;
	} else if (inventState == kInventoryActive) {
		_vm->_inventory->setInventoryState(kInventoryUp);
		gameStatus._viewState = kViewInvent;
	}
}

} // End of namespace Hugo
