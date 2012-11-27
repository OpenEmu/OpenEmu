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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/wince/wince-sdl.h"

#include "CEActionsPocket.h"
#include "EventsBuffer.h"
#include "gui/message.h"
#include "common/config-manager.h"
#include "gui/KeysDialog.h"

#include "common/translation.h"


#ifdef _WIN32_WCE
#define     KEY_ALL_SKIP    3457
#endif

const Common::String pocketActionNames[] = {
	_s("Pause"),
	_s("Save"),
	_s("Quit"),
	_s("Skip"),
	_s("Hide Toolbar"),
	_s("Show Keyboard"),
	_s("Sound on/off"),
	_s("Right click"),
	_s("Show/Hide Cursor"),
	_s("Free look"),
	_s("Zoom up"),
	_s("Zoom down"),
	_s("Multi Function"),
	_s("Bind Keys"),
	_s("Cursor Up"),
	_s("Cursor Down"),
	_s("Cursor Left"),
	_s("Cursor Right"),
	_s("Left Click")
};

void CEActionsPocket::init() {
	_instance = new CEActionsPocket(ConfMan.get("gameid"));
}


Common::String CEActionsPocket::actionName(GUI::ActionType action) {
	return _(pocketActionNames[action]);
}

int CEActionsPocket::size() {
	return POCKET_ACTION_LAST;
}

Common::String CEActionsPocket::domain() {
	return ConfMan.kApplicationDomain;
}

int CEActionsPocket::version() {
	return POCKET_ACTION_VERSION;
}

CEActionsPocket::CEActionsPocket(const Common::String &gameid) :
	GUI::Actions() {
	int i;

	_right_click_needed = false;
	_hide_toolbar_needed = false;
	_zoom_needed = false;

	for (i = 0; i < POCKET_ACTION_LAST; i++) {
		_action_mapping[i] = 0;
		_action_enabled[i] = false;
	}

	// apply some default settings for emulated mouse
	_action_enabled[POCKET_ACTION_LEFTCLICK] = true;
	_action_enabled[POCKET_ACTION_UP] = true;
	_action_enabled[POCKET_ACTION_DOWN] = true;
	_action_enabled[POCKET_ACTION_LEFT] = true;
	_action_enabled[POCKET_ACTION_RIGHT] = true;
	_action_mapping[POCKET_ACTION_LEFTCLICK] = SDLK_F1;
	_action_mapping[POCKET_ACTION_UP] = SDLK_UP;
	_action_mapping[POCKET_ACTION_DOWN] = SDLK_DOWN;
	_action_mapping[POCKET_ACTION_LEFT] = SDLK_LEFT;
	_action_mapping[POCKET_ACTION_RIGHT] = SDLK_RIGHT;
}

void CEActionsPocket::initInstanceMain(OSystem *mainSystem) {
	// Nothing generic to do for Pocket PC
	_CESystem = static_cast<OSystem_WINCE3 *>(mainSystem);
	GUI_Actions::initInstanceMain(mainSystem);
}

void CEActionsPocket::initInstanceGame() {
	Common::String gameid(ConfMan.get("gameid"));
	bool is_simon = (strncmp(gameid.c_str(), "simon", 5) == 0);
	bool is_sword1 = (gameid == "sword1");
	bool is_sword2 = (strcmp(gameid.c_str(), "sword2") == 0);
	bool is_queen = (gameid == "queen");
	bool is_sky = (gameid == "sky");
	bool is_comi = (strncmp(gameid.c_str(), "comi", 4) == 0);
	bool is_gob = (strncmp(gameid.c_str(), "gob", 3) == 0);
	bool is_saga = (gameid == "saga");
	bool is_kyra = (strncmp(gameid.c_str(), "kyra", 4) == 0);
	bool is_samnmax = (gameid == "samnmax");
	bool is_cine = (gameid == "cine");
	bool is_touche = (gameid == "touche");
	bool is_agi = (gameid == "agi");
	bool is_parallaction = (gameid == "parallaction");
	bool is_lure = (gameid == "lure");
	bool is_feeble = (gameid == "feeble");
	bool is_drascula = (strncmp(gameid.c_str(), "drascula", 8) == 0);
	bool is_tucker = (gameid == "tucker");
	bool is_groovie = (gameid == "groovie");
	bool is_tinsel = (gameid == "tinsel");
	bool is_cruise = (gameid == "cruise");
	bool is_made = (gameid == "made");
	bool is_sci = (gameid == "sci");

	GUI_Actions::initInstanceGame();

	// See if a right click mapping could be needed
	if (is_sword1 || is_sword2 || is_sky || is_queen || is_comi || is_gob || is_tinsel ||
	        is_samnmax || is_cine || is_touche || is_parallaction || is_drascula || is_cruise)
		_right_click_needed = true;

	// See if a "hide toolbar" mapping could be needed
	if (is_sword1 || is_sword2 || is_comi || is_groovie)
		_hide_toolbar_needed = true;

	// Initialize keys for different actions
	// Pause
	_key_action[POCKET_ACTION_PAUSE].setKey(VK_SPACE);
	_action_enabled[POCKET_ACTION_PAUSE] = true;
	// Save
	if (is_simon || is_sword2 || is_gob || is_kyra || is_feeble || is_tucker || is_groovie)
		_action_enabled[POCKET_ACTION_SAVE] = false;
	else if (is_queen) {
		_action_enabled[POCKET_ACTION_SAVE] = true;
		_key_action[POCKET_ACTION_SAVE].setKey(Common::ASCII_F5, SDLK_F5); // F1 key for FOTAQ
	} else if (is_sky) {
		_action_enabled[POCKET_ACTION_SAVE] = true;
		_key_action[POCKET_ACTION_SAVE].setKey(Common::ASCII_F5, SDLK_F5);
	} else if (is_cine || is_drascula || is_cruise) {
		_action_enabled[POCKET_ACTION_SAVE] = true;
		_key_action[POCKET_ACTION_SAVE].setKey(Common::ASCII_F10, SDLK_F10); // F10
	} else if (is_agi || is_made) {
		_action_enabled[POCKET_ACTION_SAVE] = true;
		_key_action[POCKET_ACTION_SAVE].setKey(Common::ASCII_ESCAPE, SDLK_ESCAPE);
	} else if (is_parallaction) {
		_action_enabled[POCKET_ACTION_SAVE] = true;
		_key_action[POCKET_ACTION_SAVE].setKey('s', SDLK_s);
	} else if (is_tinsel) {
		_action_enabled[POCKET_ACTION_SAVE] = true;
		_key_action[POCKET_ACTION_SAVE].setKey(Common::ASCII_F1, SDLK_F1);
	} else {
		_action_enabled[POCKET_ACTION_SAVE] = true;
		_key_action[POCKET_ACTION_SAVE].setKey(Common::ASCII_F5, SDLK_F5); // F5 key
	}
	// Quit
	_action_enabled[POCKET_ACTION_QUIT] = true;
	// Skip
	if (!is_cine && !is_parallaction && !is_groovie && !is_cruise && !is_made)
		_action_enabled[POCKET_ACTION_SKIP] = true;
	if (is_simon || is_sky || is_sword2 || is_queen || is_sword1 || is_gob || is_tinsel ||
	        is_saga || is_kyra || is_touche || is_lure || is_feeble || is_drascula || is_tucker)
		_key_action[POCKET_ACTION_SKIP].setKey(VK_ESCAPE);
	else
		_key_action[POCKET_ACTION_SKIP].setKey(KEY_ALL_SKIP);
	// Hide
	_action_enabled[POCKET_ACTION_HIDE] = true;
	// Keyboard
	_action_enabled[POCKET_ACTION_KEYBOARD] = true;
	// Sound
	_action_enabled[POCKET_ACTION_SOUND] = true;
	// RightClick
	_action_enabled[POCKET_ACTION_RIGHTCLICK] = true;
	// Cursor
	_action_enabled[POCKET_ACTION_CURSOR] = true;
	// Freelook
	_action_enabled[POCKET_ACTION_FREELOOK] = true;
	// Zoom
	if (is_sword1 || is_sword2 || is_comi || is_touche) {
		_zoom_needed = true;
		_action_enabled[POCKET_ACTION_ZOOM_UP] = true;
		_action_enabled[POCKET_ACTION_ZOOM_DOWN] = true;
	}
	// Multi function key
	_action_enabled[POCKET_ACTION_MULTI] = true;
	if (is_agi)
		_key_action[POCKET_ACTION_MULTI].setKey(SDLK_PAUSE); // agi: show predictive dialog
	else if (is_gob)
		_key_action[POCKET_ACTION_MULTI].setKey(Common::ASCII_F1, SDLK_F1); // bargon : F1 to start
	else if (gameid == "atlantis")
		_key_action[POCKET_ACTION_MULTI].setKey(0, SDLK_KP0); // fate of atlantis : Ins to sucker-punch
	else if (is_simon)
		_key_action[POCKET_ACTION_MULTI].setKey(Common::ASCII_F10, SDLK_F10); // F10
	else
		_key_action[POCKET_ACTION_MULTI].setKey('V', SDLK_v, KMOD_SHIFT); // FT cheat : shift-V
	// Key bind method
	_action_enabled[POCKET_ACTION_BINDKEYS] = true;
	// Disable double-tap right-click for convenience
	if (is_tinsel || is_cruise || is_sci)
		if (!ConfMan.hasKey("no_doubletap_rightclick")) {
			ConfMan.setBool("no_doubletap_rightclick", true);
			ConfMan.flushToDisk();
		}
}


CEActionsPocket::~CEActionsPocket() {
}

bool CEActionsPocket::perform(GUI::ActionType action, bool pushed) {
	static bool keydialogrunning = false, quitdialog = false;

	_graphicsMan = ((WINCESdlGraphicsManager *)((OSystem_SDL *)g_system)->getGraphicsManager());

	if (!pushed) {
		switch (action) {
		case POCKET_ACTION_RIGHTCLICK:
			_graphicsMan->add_right_click(false);
			return true;
		case POCKET_ACTION_LEFTCLICK:
			_graphicsMan->add_left_click(false);
			return true;
		case POCKET_ACTION_PAUSE:
		case POCKET_ACTION_SAVE:
		case POCKET_ACTION_SKIP:
		case POCKET_ACTION_MULTI:
			EventsBuffer::simulateKey(&_key_action[action], false);
			return true;
		}
		return false;
	}

	switch (action) {
	case POCKET_ACTION_PAUSE:
	case POCKET_ACTION_SAVE:
	case POCKET_ACTION_SKIP:
	case POCKET_ACTION_MULTI:
		if (action == POCKET_ACTION_SAVE && ConfMan.get("gameid") == "parallaction") {
			// FIXME: This is a temporary solution. The engine should handle its own menus.
			// Note that the user can accomplish this via the virtual keyboard as well, this is just for convenience
			GUI::MessageDialog alert(_("Do you want to load or save the game?"), _("Load"), _("Save"));
			if (alert.runModal() == GUI::kMessageOK)
				_key_action[action].setKey(SDLK_l);
			else
				_key_action[action].setKey(SDLK_s);
		}
		if (action == POCKET_ACTION_SKIP && ConfMan.get("gameid") == "agi") {
			// In several AGI games (for example SQ2) it is needed to press F10 to exit from
			// a screen. But we still want be able to skip normally with the skip button.
			// Because of this, we inject a F10 keystroke here (this works and doesn't seem
			// to have side-effects)
			_key_action[action].setKey(Common::ASCII_F10, SDLK_F10); // F10
			EventsBuffer::simulateKey(&_key_action[action], true);
			_key_action[action].setKey(KEY_ALL_SKIP);
		}
		EventsBuffer::simulateKey(&_key_action[action], true);
		return true;
	case POCKET_ACTION_KEYBOARD:
		_graphicsMan->swap_panel();
		return true;
	case POCKET_ACTION_HIDE:
		_graphicsMan->swap_panel_visibility();
		return true;
	case POCKET_ACTION_SOUND:
		_CESystem->swap_sound_master();
		return true;
	case POCKET_ACTION_RIGHTCLICK:
		_graphicsMan->add_right_click(true);
		return true;
	case POCKET_ACTION_CURSOR:
		_graphicsMan->swap_mouse_visibility();
		return true;
	case POCKET_ACTION_FREELOOK:
		_graphicsMan->swap_freeLook();
		return true;
	case POCKET_ACTION_ZOOM_UP:
		_graphicsMan->swap_zoom_up();
		return true;
	case POCKET_ACTION_ZOOM_DOWN:
		_graphicsMan->swap_zoom_down();
		return true;
	case POCKET_ACTION_LEFTCLICK:
		_graphicsMan->add_left_click(true);
		return true;
	case POCKET_ACTION_UP:
		_graphicsMan->move_cursor_up();
		return true;
	case POCKET_ACTION_DOWN:
		_graphicsMan->move_cursor_down();
		return true;
	case POCKET_ACTION_LEFT:
		_graphicsMan->move_cursor_left();
		return true;
	case POCKET_ACTION_RIGHT:
		_graphicsMan->move_cursor_right();
		return true;
	case POCKET_ACTION_QUIT:
		if (!quitdialog) {
			quitdialog = true;
			GUI::MessageDialog alert(_("   Are you sure you want to quit ?   "), _("Yes"), _("No"));
			if (alert.runModal() == GUI::kMessageOK)
				_mainSystem->quit();
			quitdialog = false;
		}
		return true;
	case POCKET_ACTION_BINDKEYS:
		if (!keydialogrunning) {
			keydialogrunning = true;
			GUI::KeysDialog *keysDialog = new GUI::KeysDialog();
			keysDialog->runModal();
			delete keysDialog;
			keydialogrunning = false;
		}
		return true;
	}
	return false;
}

bool CEActionsPocket::needsRightClickMapping() {
	if (!_right_click_needed)
		return false;
	else
		return (_action_mapping[POCKET_ACTION_RIGHTCLICK] == 0);
}

bool CEActionsPocket::needsHideToolbarMapping() {
	if (!_hide_toolbar_needed)
		return false;
	else
		return (_action_mapping[POCKET_ACTION_HIDE] == 0);
}


bool CEActionsPocket::needsZoomMapping() {
	if (!_zoom_needed)
		return false;
	else
		return (_action_mapping[POCKET_ACTION_ZOOM_UP] == 0 || _action_mapping[POCKET_ACTION_ZOOM_DOWN] == 0);
}
