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


#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "queen/queen.h"
#include "queen/input.h"

namespace Queen {

const char *const Input::_commandKeys[] = {
	"ocmglptu", // English
	"osbgpnre", // German
	"ofdnepau", // French
	"acsdgpqu", // Italian
	"ocmglptu", // Hebrew
	"acodmthu"  // Spanish
};

const Verb Input::_verbKeys[] = {
	VERB_OPEN,
	VERB_CLOSE,
	VERB_MOVE,
	VERB_GIVE,
	VERB_LOOK_AT,
	VERB_PICK_UP,
	VERB_TALK_TO,
	VERB_USE
};

Input::Input(Common::Language language, OSystem *system, QueenEngine *vm) :
	_system(system), _eventMan(system->getEventManager()), _fastMode(false),
	_keyVerb(VERB_NONE), _cutawayRunning(false), _canQuit(false),
	_cutawayQuit(false), _dialogueRunning(false), _talkQuit(false),
	_quickSave(false), _quickLoad(false), _debugger(false), _inKey(Common::KEYCODE_INVALID),
	_mouseButton(0), _idleTime(0) , _vm(vm) {

	switch (language) {
	case Common::EN_ANY:
	case Common::GR_GRE:
	case Common::RU_RUS:
		_currentCommandKeys = _commandKeys[0];
		break;
	case Common::DE_DEU:
		_currentCommandKeys = _commandKeys[1];
		break;
	case Common::FR_FRA:
		_currentCommandKeys = _commandKeys[2];
		break;
	case Common::IT_ITA:
		_currentCommandKeys = _commandKeys[3];
		break;
	case Common::HE_ISR:
		_currentCommandKeys = _commandKeys[4];
		break;
	case Common::ES_ESP:
		_currentCommandKeys = _commandKeys[5];
		break;
	default:
		error("Unknown language");
		break;
	}
}

void Input::delay(uint amount) {
	if (_fastMode && amount > DELAY_SHORT) {
		amount = DELAY_SHORT;
	}
	if (_idleTime < DELAY_SCREEN_BLANKER) {
		_idleTime += amount;
	}
	uint32 end = _system->getMillis() + amount;
	do {
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			_idleTime = 0;
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.hasFlags(Common::KBD_CTRL)) {
					if (event.kbd.keycode == Common::KEYCODE_d) {
						_debugger = true;
					} else if (event.kbd.keycode == Common::KEYCODE_f) {
						_fastMode = !_fastMode;
					}
				} else {
					_inKey = event.kbd.keycode;
				}
				break;

			case Common::EVENT_LBUTTONDOWN:
				_mouseButton |= MOUSE_LBUTTON;
				break;

			case Common::EVENT_RBUTTONDOWN:
				_mouseButton |= MOUSE_RBUTTON;
				break;
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
				if (_cutawayRunning)
					_cutawayQuit = true;
				return;

			default:
				break;
			}
		}

		_system->updateScreen();

		if (amount == 0)
			break;

		_system->delayMillis((amount > 10) ? 10 : amount);
	} while (_system->getMillis() < end);
}

void Input::checkKeys() {

	if (_inKey)
		debug(6, "[Input::checkKeys] _inKey = %i", _inKey);

	switch (_inKey) {
	case Common::KEYCODE_SPACE:
		_keyVerb = VERB_SKIP_TEXT;
		break;
	case Common::KEYCODE_COMMA:
		_keyVerb = VERB_SCROLL_UP;
		break;
	case Common::KEYCODE_PERIOD:
		_keyVerb = VERB_SCROLL_DOWN;
		break;
	case Common::KEYCODE_1:
		_keyVerb = VERB_DIGIT_1;
		break;
	case Common::KEYCODE_2:
		_keyVerb = VERB_DIGIT_2;
		break;
	case Common::KEYCODE_3:
		_keyVerb = VERB_DIGIT_3;
		break;
	case Common::KEYCODE_4:
		_keyVerb = VERB_DIGIT_4;
		break;
	case Common::KEYCODE_ESCAPE: // skip cutaway / dialogue
		if (_canQuit) {
			if (_cutawayRunning) {
				debug(6, "[Input::checkKeys] Setting _cutawayQuit to true");
				_cutawayQuit = true;
			}
			if (_dialogueRunning)
				_talkQuit = true;
		}
		break;
	case Common::KEYCODE_F1: // use Journal
	case Common::KEYCODE_F5:
		if (_cutawayRunning) {
			if (_canQuit) {
				_keyVerb = VERB_USE_JOURNAL;
				_cutawayQuit = _talkQuit = true;
			}
		} else {
			_keyVerb = VERB_USE_JOURNAL;
			if (_canQuit)
				_talkQuit = true;
		}
		break;
	case Common::KEYCODE_F11: // quicksave
		_quickSave = true;
		break;
	case Common::KEYCODE_F12: // quickload
		_quickLoad = true;
		break;
	default:
		for (int i = 0; i < ARRAYSIZE(_verbKeys); ++i) {
			if (_inKey == _currentCommandKeys[i]) {
				_keyVerb = _verbKeys[i];
				break;
			}
		}
		break;
	}

	_inKey = Common::KEYCODE_INVALID;	// reset
}

Common::Point Input::getMousePos() const {
	return _eventMan->getMousePos();
}

} // End of namespace Queen
