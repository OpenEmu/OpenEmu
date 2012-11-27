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
#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/parser.h"
#include "hugo/file.h"
#include "hugo/schedule.h"
#include "hugo/util.h"
#include "hugo/sound.h"
#include "hugo/object.h"
#include "hugo/text.h"

namespace Hugo {

Parser_v2d::Parser_v2d(HugoEngine *vm) : Parser_v1d(vm) {
}

Parser_v2d::~Parser_v2d() {
}

/**
 * Parse the user's line of text input.  Generate events as necessary
 */
void Parser_v2d::lineHandler() {
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

	if (!strcmp("exit", _vm->_line) || strstr(_vm->_line, "quit")) {
		if (Utils::yesNoBox(_vm->_text->getTextParser(kTBExit_1d)))
			_vm->endGame();
		return;
	}

	// SAVE/RESTORE
	if (!strcmp("save", _vm->_line)) {
		if (gameStatus._gameOverFl)
			_vm->gameOverMsg();
		else
			_vm->_file->saveGame(-1, Common::String());
		return;
	}

	if (!strcmp("restore", _vm->_line)) {
		_vm->_file->restoreGame(-1);
		return;
	}

	if (*_vm->_line == '\0')                        // Empty line
		return;

	if (strspn(_vm->_line, " ") == strlen(_vm->_line)) // Nothing but spaces!
		return;

	if (gameStatus._gameOverFl) {
		// No commands allowed!
		_vm->gameOverMsg();
		return;
	}

	// Find the first verb in the line
	const char *verb = findVerb();
	const char *noun = 0;                           // Noun not found yet
	char farComment[kCompLineSize * 5] = "";        // hold 5 line comment if object not nearby

	if (verb) {                                     // OK, verb found.  Try to match with object
		do {
			noun = findNextNoun(noun);              // Find a noun in the line
			// Must try at least once for objects allowing verb-context
			for (int i = 0; i < _vm->_object->_numObj; i++) {
				Object *obj = &_vm->_object->_objects[i];
				if (isNear_v1(verb, noun, obj, farComment)) {
					if (isObjectVerb_v1(verb, obj)  // Foreground object
					 || isGenericVerb_v1(verb, obj))// Common action type
						return;
				}
			}
			if ((*farComment != '\0') && isBackgroundWord_v1(noun, verb, _backgroundObjects[*_vm->_screenPtr]))
				return;
		} while (noun);
	}

	noun = findNextNoun(noun);
	if (   !isCatchallVerb_v1(true, noun, verb, _backgroundObjects[*_vm->_screenPtr])
		&& !isCatchallVerb_v1(true, noun, verb, _catchallList)
		&& !isCatchallVerb_v1(false, noun, verb, _backgroundObjects[*_vm->_screenPtr])
		&& !isCatchallVerb_v1(false, noun, verb, _catchallList)) {
		if (*farComment != '\0') {                  // An object matched but not near enough
			Utils::notifyBox(farComment);
		} else if (_vm->_maze._enabledFl && (verb == _vm->_text->getVerb(_vm->_look, 0))) {
			Utils::notifyBox(_vm->_text->getTextParser(kTBMaze));
			_vm->_object->showTakeables();
		} else if (verb && noun) {                  // A combination I didn't think of
			Utils::notifyBox(_vm->_text->getTextParser(kTBNoUse_2d));
		} else if (verb || noun) {
			Utils::notifyBox(_vm->_text->getTextParser(kTBNoun));
		} else {
			Utils::notifyBox(_vm->_text->getTextParser(kTBEh_2d));
		}
	}
}

} // End of namespace Hugo
