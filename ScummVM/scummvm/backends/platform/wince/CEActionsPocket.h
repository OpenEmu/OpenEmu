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

#ifndef CEACTIONSPOCKET_H
#define CEACTIONSPOCKET_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/str.h"
#include "gui/Key.h"
#include "gui/Actions.h"
#include "backends/graphics/wincesdl/wincesdl-graphics.h"

#define POCKET_ACTION_VERSION 5

enum pocketActionType {
	POCKET_ACTION_PAUSE = 0,
	POCKET_ACTION_SAVE,
	POCKET_ACTION_QUIT,
	POCKET_ACTION_SKIP,
	POCKET_ACTION_HIDE,
	POCKET_ACTION_KEYBOARD,
	POCKET_ACTION_SOUND,
	POCKET_ACTION_RIGHTCLICK,
	POCKET_ACTION_CURSOR,
	POCKET_ACTION_FREELOOK,
	POCKET_ACTION_ZOOM_UP,
	POCKET_ACTION_ZOOM_DOWN,
	POCKET_ACTION_MULTI,
	POCKET_ACTION_BINDKEYS,
	POCKET_ACTION_UP,
	POCKET_ACTION_DOWN,
	POCKET_ACTION_LEFT,
	POCKET_ACTION_RIGHT,
	POCKET_ACTION_LEFTCLICK,

	POCKET_ACTION_LAST
};

class OSystem_WINCE3;

class CEActionsPocket : public GUI::Actions {
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

	// Utility
	bool needsRightClickMapping();
	bool needsHideToolbarMapping();
	bool needsZoomMapping();

	~CEActionsPocket();
private:
	CEActionsPocket(const Common::String &gameid);
	WINCESdlGraphicsManager *_graphicsMan;
	bool _right_click_needed;
	bool _hide_toolbar_needed;
	bool _zoom_needed;
	OSystem_WINCE3 *_CESystem;
};

#endif
