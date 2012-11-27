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

#ifndef PARALLACTION_GUI_H
#define PARALLACTION_GUI_H

#include "common/system.h"
#include "common/hashmap.h"

#include "parallaction/input.h"
#include "parallaction/parallaction.h"
#include "parallaction/sound.h"


namespace Parallaction {

class MenuInputState;

class MenuInputHelper {
	typedef	Common::HashMap<Common::String, MenuInputState *> StateMap;

	StateMap	_map;
	MenuInputState	*_state;
	MenuInputState *_newState;

public:
	MenuInputHelper() : _state(0) {
	}

	~MenuInputHelper();

	void setState(const Common::String &name) {
		// bootstrap routine
		_newState = getState(name);
		assert(_newState);
	}

	void addState(const Common::String &name, MenuInputState *state) {
		_map.setVal(name, state);
	}

	MenuInputState *getState(const Common::String &name) {
		return _map[name];
	}

	bool run();
};

class MenuInputState {

protected:
	MenuInputHelper *_helper;

public:
	MenuInputState(const Common::String &name, MenuInputHelper *helper) : _helper(helper), _name(name) {
		debugC(3, kDebugExec, "MenuInputState(%s)", name.c_str());
		_helper->addState(name, this);
	}

	Common::String	_name;

	virtual ~MenuInputState() { }

	virtual MenuInputState* run() = 0;
	virtual void enter() = 0;
};


} // namespace Parallaction

#endif
