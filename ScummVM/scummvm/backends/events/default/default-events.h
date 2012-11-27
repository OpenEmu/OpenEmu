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

#if !defined(BACKEND_EVENTS_DEFAULT_H) && !defined(DISABLE_DEFAULT_EVENTMANAGER)
#define BACKEND_EVENTS_DEFAULT_H

#include "common/events.h"
#include "common/queue.h"

namespace Common {
#ifdef ENABLE_KEYMAPPER
class Keymapper;
#endif
#ifdef ENABLE_VKEYBD
class VirtualKeyboard;
#endif
}


class DefaultEventManager : public Common::EventManager, Common::EventObserver {
#ifdef ENABLE_VKEYBD
	Common::VirtualKeyboard *_vk;
#endif

#ifdef ENABLE_KEYMAPPER
	Common::Keymapper *_keymapper;
	bool _remap;
#endif

	Common::ArtificialEventSource _artificialEventSource;

	Common::Queue<Common::Event> _eventQueue;
	bool notifyEvent(const Common::Event &ev) {
		_eventQueue.push(ev);
		return true;
	}

	Common::Point _mousePos;
	int _buttonState;
	int _modifierState;
	bool _shouldQuit;
	bool _shouldRTL;
	bool _confirmExitDialogActive;

	// for continuous events (keyDown)
	enum {
		kKeyRepeatInitialDelay = 400,
		kKeyRepeatSustainDelay = 100
	};

	struct {
		uint16 ascii;
		byte flags;
		int keycode;
	} _currentKeyDown;
	uint32 _keyRepeatTime;
public:
	DefaultEventManager(Common::EventSource *boss);
	~DefaultEventManager();

	virtual void init();
	virtual bool pollEvent(Common::Event &event);
	virtual void pushEvent(const Common::Event &event);

	virtual Common::Point getMousePos() const { return _mousePos; }
	virtual int getButtonState() const { return _buttonState; }
	virtual int getModifierState() const { return _modifierState; }
	virtual int shouldQuit() const { return _shouldQuit; }
	virtual int shouldRTL() const { return _shouldRTL; }
	virtual void resetRTL() { _shouldRTL = false; }
#ifdef FORCE_RTL
	virtual void resetQuit() { _shouldQuit = false; }
#endif

#ifdef ENABLE_KEYMAPPER
	 // IMPORTANT NOTE: This is part of the WIP Keymapper. If you plan to use
	 // this, please talk to tsoliman and/or LordHoto.
	virtual Common::Keymapper *getKeymapper() { return _keymapper; }
#endif
};

#endif
