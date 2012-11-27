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

#include "backends/platform/symbian/src/SymbianActions.h"

#include "gui/message.h"
#include "scumm/scumm.h"
#include "common/config-manager.h"
#include "common/translation.h"

#include <sdl.h>

namespace GUI {

// SumthinWicked says: we either split our Actions like WinCE did with Pocket/Smartphone
// or we put them in this file separated by #ifdefs, this one is up to you, AnotherGuest :)

const Common::String actionNames[] = {
	_s("Up"),
	_s("Down"),
	_s("Left"),
	_s("Right"),
	_s("Left Click"),
	_s("Right Click"),
	_s("Save"),
	_s("Skip"),
	_s("Zone"),
	_s("Multi Function"),
	_s("Swap character"),
	_s("Skip text"),
	_s("Pause"),
	_s("Fast mode"),
	_s("Quit"),
	_s("Debugger"),
	_s("Global menu"),
	_s("Virtual keyboard"),
	_s("Key mapper")
};

#ifdef UIQ
static const int ACTIONS_DEFAULT[ACTION_LAST] = { SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_F1, SDLK_F2, SDLK_F5, SDLK_PAGEDOWN, '9', 0, 0, SDLK_PAGEUP, 0, 0, 0, 0, 0, 0, 0};
#elif defined(S60)
const int ACTIONS_DEFAULT[ACTION_LAST] = { 0, 0, 0, 0, 0, 0, '*', '#', '9', 0, 0, 0, 0, 0, 0, 0, '0', 0, 0};
#elif defined(S90)
const int ACTIONS_DEFAULT[ACTION_LAST] = { SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, 0, 0, SDLK_MENU, SDLK_ESCAPE, 0, 0 , 0, 0, 0, 0, 0, 0, 0, 0 ,0};
#else
const int ACTIONS_DEFAULT[ACTION_LAST] = { SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_F1, SDLK_F2, SDLK_MENU, SDLK_ESCAPE, 0, 0, 0, 0, 0, 0, 0, 0, '1', 0 ,0};
#endif

// creator function according to Factory Pattern
void SymbianActions::init() {
	_instance = new SymbianActions();
}


Common::String SymbianActions::actionName(ActionType action) {
	return _(actionNames[action]);
}

int SymbianActions::size() {
	return ACTION_LAST;
}

Common::String SymbianActions::domain() {
	return Common::ConfigManager::kApplicationDomain;
}

int SymbianActions::version() {
	return ACTION_VERSION;
}

SymbianActions::SymbianActions()
 : Actions() {
	int i;

	for (i = 0; i < ACTION_LAST; i++) {
		_action_mapping[i] = ACTIONS_DEFAULT[i];
		_action_enabled[i] = false;
	}

}

void SymbianActions::initInstanceMain(OSystem *mainSystem) {
	int i;

	// Need to do this since all old mappings are reset after engineDone
	_initialized = false;
	Actions::initInstanceMain(mainSystem);

	// Disable all mappings before setting main mappings again
	for (i = 0; i < ACTION_LAST; i++) {
		_action_enabled[i] = false;
	}

	// Mouse Up
	_action_enabled[ACTION_UP] = true;

	// Mouse Down
	_action_enabled[ACTION_DOWN] = true;

	// Mouse Left
	_action_enabled[ACTION_LEFT] = true;

	// Mouse Right
	_action_enabled[ACTION_RIGHT] = true;

	// Left Click
	_action_enabled[ACTION_LEFTCLICK] = true;

	// Right Click
	_action_enabled[ACTION_RIGHTCLICK] = true;

	// Skip
	_action_enabled[ACTION_SKIP] = true;
	_key_action[ACTION_SKIP].setKey(SDLK_ESCAPE);

	// Enable keymappings
	_action_enabled[ACTION_KEYMAPPER] = true;
	_key_action[ACTION_KEYMAPPER].setKey(Common::ASCII_F8, Common::KEYCODE_F8);

	// Enable VKB
	_action_enabled[ACTION_VKB] = true;
	_key_action[ACTION_VKB].setKey(Common::ASCII_F7, Common::KEYCODE_F7);
}

void SymbianActions::initInstanceGame() {
	Common::String gameid(ConfMan.get("gameid"));
	bool is_simon = (strncmp(gameid.c_str(), "simon", 5) == 0);
	bool is_sword1 = (gameid == "sword1");
	bool is_sword2 = (strcmp(gameid.c_str(), "sword2") == 0);
	bool is_queen = (gameid == "queen");
	bool is_sky = (gameid == "sky");
	bool is_comi = (strncmp(gameid.c_str(), "comi", 4) == 0);
	bool is_gob = (strncmp(gameid.c_str(), "gob", 3) == 0);
	bool is_saga = (gameid == "saga");
	bool is_kyra = (strncmp(gameid.c_str(), "kyra",4) == 0);
	bool is_samnmax = (gameid == "samnmax");
	bool is_cine = (gameid == "cine");
	bool is_touche = (gameid == "touche");
	bool is_agi = (gameid == "agi");
	bool is_parallaction = (gameid == "parallaction");
	bool is_lure = (gameid == "lure");
	bool is_feeble = (gameid == "feeble");
	bool is_drascula = (strncmp(gameid.c_str(), "drascula",8) == 0);
	bool is_tucker = (gameid == "tucker");
	bool is_groovie = (gameid == "groovie");
	bool is_tinsel = (gameid == "tinsel");
	bool is_cruise = (gameid == "cruise");
	bool is_made = (gameid == "made");

	Actions::initInstanceGame();

	// Initialize keys for different actions
	// Pause
	if(is_cruise) {
		_key_action[ACTION_PAUSE].setKey('P');
	}
	else {
		_key_action[ACTION_PAUSE].setKey(' ');
	}
	_action_enabled[ACTION_PAUSE] = true;


	// Save
	if (is_simon || is_sword2 || is_gob || is_kyra || is_feeble || is_tucker || is_groovie)
		_action_enabled[ACTION_SAVE] = false;
	else {
		_action_enabled[ACTION_SAVE] = true;

		if (is_queen) {
			_key_action[ACTION_SAVE].setKey(Common::ASCII_F1, Common::KEYCODE_F1); // F1 key for FOTAQ
		} else if (is_sky) {
			_key_action[ACTION_SAVE].setKey(Common::ASCII_F5, Common::KEYCODE_F5);
		} else if (is_cine || is_drascula || is_cruise) {
			_key_action[ACTION_SAVE].setKey(Common::ASCII_F10, Common::KEYCODE_F10); // F10
		} else if (is_agi) {
			_key_action[ACTION_SAVE].setKey(Common::ASCII_ESCAPE, Common::KEYCODE_ESCAPE);
		} else if (is_parallaction) {
			_key_action[ACTION_SAVE].setKey('s', Common::KEYCODE_s);
		} else if (is_tinsel) {
			_key_action[ACTION_SAVE].setKey(Common::ASCII_F1, SDLK_F1);
		} else {
			_key_action[ACTION_SAVE].setKey(Common::ASCII_F5, Common::KEYCODE_F5); // F5 key
		}
	}
	// Quit
	_action_enabled[ACTION_QUIT] = true;

	// Skip text
	if (!is_cine && !is_parallaction && !is_groovie && !is_cruise && !is_made)
		_action_enabled[ACTION_SKIP_TEXT] = true;
	if (is_simon || is_sky || is_sword2 || is_queen || is_sword1 || is_gob || is_tinsel ||
			is_saga || is_kyra || is_touche || is_lure || is_feeble || is_drascula || is_tucker)
		_key_action[ACTION_SKIP_TEXT].setKey(Common::KEYCODE_ESCAPE, Common::KEYCODE_ESCAPE); // Escape key
	else {
		_key_action[ACTION_SKIP_TEXT].setKey(SDLK_PERIOD);
	}

	// Enable fast mode
	_action_enabled[ACTION_FASTMODE] = true;
	_key_action[ACTION_FASTMODE].setKey('f', Common::KEYCODE_f, KMOD_CTRL);

	// Swap character
	_action_enabled[ACTION_SWAPCHAR] = true;
	_key_action[ACTION_SWAPCHAR].setKey('b'); // b

	// Zone
	_action_enabled[ACTION_ZONE] = true;

	// Multi function key
	_action_enabled[ACTION_MULTI] = true;
	if (is_agi)
		_key_action[ACTION_MULTI].setKey(SDLK_PAUSE); // agi: show predictive dialog
	else if (is_gob)
		_key_action[ACTION_MULTI].setKey(Common::ASCII_F1, Common::KEYCODE_F1); // bargon : F1 to start
	else if (gameid == "atlantis")
		_key_action[ACTION_MULTI].setKey('0', Common::KEYCODE_KP0); // fate of atlantis : Ins to sucker-punch
	else
		_key_action[ACTION_MULTI].setKey('V', SDLK_v, KMOD_SHIFT); // FT cheat : shift-V

	// Enable debugger
	_action_enabled[ACTION_DEBUGGER] = true;
	_key_action[ACTION_DEBUGGER].setKey('d', Common::KEYCODE_d, KMOD_CTRL);

	// Enable global menu
	_action_enabled[ACTION_MAINMENU] = true;
	_key_action[ACTION_MAINMENU].setKey(Common::ASCII_F5, Common::KEYCODE_F5, KMOD_CTRL);

}


SymbianActions::~SymbianActions() {
}

bool SymbianActions::perform(ActionType /*action*/, bool /*pushed*/) {

	return false;
}

} // namespace GUI
