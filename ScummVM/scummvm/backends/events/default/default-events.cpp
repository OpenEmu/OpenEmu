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

#include "common/scummsys.h"

#if !defined(DISABLE_DEFAULT_EVENTMANAGER)

#include "common/system.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "backends/events/default/default-events.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/remap-dialog.h"
#include "backends/vkeybd/virtual-keyboard.h"

#include "engines/engine.h"
#include "gui/message.h"

DefaultEventManager::DefaultEventManager(Common::EventSource *boss) :
	_buttonState(0),
	_modifierState(0),
	_shouldQuit(false),
	_shouldRTL(false),
	_confirmExitDialogActive(false) {

	assert(boss);

	_dispatcher.registerSource(boss, false);
	_dispatcher.registerSource(&_artificialEventSource, false);

	_dispatcher.registerObserver(this, kEventManPriority, false);

	// Reset key repeat
	_currentKeyDown.keycode = 0;

#ifdef ENABLE_VKEYBD
	_vk = new Common::VirtualKeyboard();
#endif
#ifdef ENABLE_KEYMAPPER
	_keymapper = new Common::Keymapper(this);
	// EventDispatcher will automatically free the keymapper
	_dispatcher.registerMapper(_keymapper);
	_remap = false;
#else
	_dispatcher.registerMapper(new Common::DefaultEventMapper());
#endif
}

DefaultEventManager::~DefaultEventManager() {
#ifdef ENABLE_VKEYBD
	delete _vk;
#endif
}

void DefaultEventManager::init() {
#ifdef ENABLE_VKEYBD
	if (ConfMan.hasKey("vkeybd_pack_name")) {
		_vk->loadKeyboardPack(ConfMan.get("vkeybd_pack_name"));
	} else {
		_vk->loadKeyboardPack("vkeybd_default");
	}
#endif
}

bool DefaultEventManager::pollEvent(Common::Event &event) {
	uint32 time = g_system->getMillis();
	bool result = false;

	_dispatcher.dispatch();
	if (!_eventQueue.empty()) {
		event = _eventQueue.pop();
		result = true;
	}

	if (result) {
		event.synthetic = false;
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_modifierState = event.kbd.flags;
			// init continuous event stream
			_currentKeyDown.ascii = event.kbd.ascii;
			_currentKeyDown.keycode = event.kbd.keycode;
			_currentKeyDown.flags = event.kbd.flags;
			_keyRepeatTime = time + kKeyRepeatInitialDelay;

			if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
				// WORKAROUND: Some engines incorrectly attempt to use the
				// ascii value instead of the keycode to detect the backspace
				// key (a non-portable behavior). This fails at least on
				// Mac OS X, possibly also on other systems.
				// As a workaround, we force the ascii value for backspace
				// key pressed. A better fix would be for engines to stop
				// making invalid assumptions about ascii values.
				event.kbd.ascii = Common::KEYCODE_BACKSPACE;
				_currentKeyDown.ascii = Common::KEYCODE_BACKSPACE;
			}
			break;

		case Common::EVENT_KEYUP:
			_modifierState = event.kbd.flags;
			if (event.kbd.keycode == _currentKeyDown.keycode) {
				// Only stop firing events if it's the current key
				_currentKeyDown.keycode = 0;
			}
			break;

		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;

		case Common::EVENT_LBUTTONDOWN:
			_mousePos = event.mouse;
			_buttonState |= LBUTTON;
			break;

		case Common::EVENT_LBUTTONUP:
			_mousePos = event.mouse;
			_buttonState &= ~LBUTTON;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_mousePos = event.mouse;
			_buttonState |= RBUTTON;
			break;

		case Common::EVENT_RBUTTONUP:
			_mousePos = event.mouse;
			_buttonState &= ~RBUTTON;
			break;

		case Common::EVENT_MAINMENU:
			if (g_engine && !g_engine->isPaused())
				g_engine->openMainMenuDialog();

			if (_shouldQuit)
				event.type = Common::EVENT_QUIT;
			else if (_shouldRTL)
				event.type = Common::EVENT_RTL;
			break;
#ifdef ENABLE_VKEYBD
		case Common::EVENT_VIRTUAL_KEYBOARD:
			if (_vk->isDisplaying()) {
				_vk->close(true);
			} else {
				if (g_engine)
					g_engine->pauseEngine(true);
				_vk->show();
				if (g_engine)
					g_engine->pauseEngine(false);
				result = false;
			}
			break;
#endif
#ifdef ENABLE_KEYMAPPER
		case Common::EVENT_KEYMAPPER_REMAP:
			if (!_remap) {
				_remap = true;
				Common::RemapDialog _remapDialog;
				if (g_engine)
					g_engine->pauseEngine(true);
				_remapDialog.runModal();
				if (g_engine)
					g_engine->pauseEngine(false);
				_remap = false;
			}
			break;
#endif
		case Common::EVENT_RTL:
			if (ConfMan.getBool("confirm_exit")) {
				if (g_engine)
					g_engine->pauseEngine(true);
				GUI::MessageDialog alert(_("Do you really want to return to the Launcher?"), _("Launcher"), _("Cancel"));
				result = _shouldRTL = (alert.runModal() == GUI::kMessageOK);
				if (g_engine)
					g_engine->pauseEngine(false);
			} else
				_shouldRTL = true;
			break;

		case Common::EVENT_MUTE:
			if (g_engine)
				g_engine->flipMute();
			break;

		case Common::EVENT_QUIT:
			if (ConfMan.getBool("confirm_exit")) {
				if (_confirmExitDialogActive) {
					result = false;
					break;
				}
				_confirmExitDialogActive = true;
				if (g_engine)
					g_engine->pauseEngine(true);
				GUI::MessageDialog alert(_("Do you really want to quit?"), _("Quit"), _("Cancel"));
				result = _shouldQuit = (alert.runModal() == GUI::kMessageOK);
				if (g_engine)
					g_engine->pauseEngine(false);
				_confirmExitDialogActive = false;
			} else
				_shouldQuit = true;

			break;

		default:
			break;
		}
	} else {
		// Check if event should be sent again (keydown)
		if (_currentKeyDown.keycode != 0 && _keyRepeatTime < time) {
			// fire event
			event.type = Common::EVENT_KEYDOWN;
			event.synthetic = true;
			event.kbd.ascii = _currentKeyDown.ascii;
			event.kbd.keycode = (Common::KeyCode)_currentKeyDown.keycode;
			event.kbd.flags = _currentKeyDown.flags;
			_keyRepeatTime = time + kKeyRepeatSustainDelay;
			result = true;
		}
	}

	return result;
}

void DefaultEventManager::pushEvent(const Common::Event &event) {
	// If already received an EVENT_QUIT, don't add another one
	if (event.type == Common::EVENT_QUIT) {
		if (!_shouldQuit)
			_artificialEventSource.addEvent(event);
	} else
		_artificialEventSource.addEvent(event);
}

#endif // !defined(DISABLE_DEFAULT_EVENTMANAGER)
