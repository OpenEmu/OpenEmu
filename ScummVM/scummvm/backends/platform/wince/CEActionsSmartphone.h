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

#ifndef CEACTIONSSMARTPHONE_H
#define CEACTIONSSMARTPHONE_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/str.h"
#include "gui/Key.h"
#include "gui/Actions.h"
#include "backends/graphics/wincesdl/wincesdl-graphics.h"

#define SMARTPHONE_ACTION_VERSION 5

enum smartphoneActionType {
	SMARTPHONE_ACTION_UP = 0,
	SMARTPHONE_ACTION_DOWN,
	SMARTPHONE_ACTION_LEFT,
	SMARTPHONE_ACTION_RIGHT,
	SMARTPHONE_ACTION_LEFTCLICK,
	SMARTPHONE_ACTION_RIGHTCLICK,
	SMARTPHONE_ACTION_SAVE,
	SMARTPHONE_ACTION_SKIP,
	SMARTPHONE_ACTION_ZONE,
	SMARTPHONE_ACTION_MULTI,
	SMARTPHONE_ACTION_BINDKEYS,
	SMARTPHONE_ACTION_KEYBOARD,
	SMARTPHONE_ACTION_ROTATE,
	SMARTPHONE_ACTION_QUIT,

	SMARTPHONE_ACTION_LAST
};


class CEActionsSmartphone : public GUI::Actions {
public:
	// Actions
	bool perform(GUI::ActionType action, bool pushed = true);
	Common::String actionName(GUI::ActionType action);
	int size();
	static void init();
	void initInstanceMain(OSystem *mainSystem);
	void initInstanceGame();

	// Action domain
	Common::String domain();
	int version();

	~CEActionsSmartphone();
private:
	CEActionsSmartphone();
	WINCESdlGraphicsManager *_graphicsMan;
	bool _right_click_needed;
	OSystem_WINCE3 *_CESystem;
};

#endif
