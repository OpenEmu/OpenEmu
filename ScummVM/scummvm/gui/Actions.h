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

#ifndef GUI_ACTIONS_H
#define GUI_ACTIONS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "gui/Key.h"

namespace GUI {

typedef int ActionType;

class Actions {

public:
	static Actions* Instance();
	static void init();
	virtual void initInstanceMain(OSystem *mainSystem);
	virtual void initInstanceGame();
	bool initialized();

	// Actions
	virtual bool perform(ActionType action, bool pushed = true) = 0;
	bool isActive(ActionType action);
	bool isEnabled(ActionType action);
	virtual Common::String actionName(ActionType action) = 0;
	virtual int size() = 0;

	// Mapping
	void beginMapping(bool start);
	bool mappingActive();
	bool performMapped(unsigned int keyCode, bool pushed);
	bool loadMapping();
	bool saveMapping();
	unsigned int getMapping(ActionType action);
	void setMapping(ActionType action, unsigned int keyCode);
	Key& getKeyAction(ActionType action);

	// Action domain
	virtual Common::String domain() = 0;
	virtual int version() = 0;

	virtual ~Actions();

protected:
	Actions();

protected:
	enum {
		MAX_ACTIONS = 20
	};

	static Actions* _instance;
	OSystem *_mainSystem;
	Key _key_action[MAX_ACTIONS + 1];
	bool _action_enabled[MAX_ACTIONS + 1];
	unsigned int _action_mapping[MAX_ACTIONS + 1];
	bool _mapping_active;
	bool _initialized;
};

} // namespace GUI
typedef GUI::Actions GUI_Actions;
#endif
