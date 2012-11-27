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
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#ifndef COMMON_ACTION_H
#define COMMON_ACTION_H

#include "common/scummsys.h"

#ifdef ENABLE_KEYMAPPER

#include "common/events.h"
#include "common/func.h"
#include "common/list.h"
#include "common/str.h"

namespace Common {

struct HardwareInput;
class Keymap;

#define ACTION_ID_SIZE (4)

struct KeyActionEntry {
	const KeyState ks;
	const char *id;
	const char *description;
};

struct Action {
	/** unique id used for saving/loading to config */
	char id[ACTION_ID_SIZE];
	/** Human readable description */
	String description;

	/** Events to be sent when mapped key is pressed */
	List<Event> events;

private:
	/** Hardware input that is mapped to this Action */
	const HardwareInput *_hwInput;
	Keymap *_boss;

public:
	Action(Keymap *boss, const char *id, String des = "");

	void addEvent(const Event &evt) {
		events.push_back(evt);
	}

	void addEvent(const EventType evtType) {
		Event evt;

		evt.type = evtType;
		events.push_back(evt);
	}

	void addKeyEvent(const KeyState &ks) {
		Event evt;

		evt.type = EVENT_KEYDOWN;
		evt.kbd = ks;
		addEvent(evt);
	}

	void addLeftClickEvent() {
		addEvent(EVENT_LBUTTONDOWN);
	}

	void addMiddleClickEvent() {
		addEvent(EVENT_MBUTTONDOWN);
	}

	void addRightClickEvent() {
		addEvent(EVENT_RBUTTONDOWN);
	}

	Keymap *getParent() {
		return _boss;
	}

	void mapInput(const HardwareInput *input);
	const HardwareInput *getMappedInput() const;

};

} // End of namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

#endif // #ifndef COMMON_ACTION_H
