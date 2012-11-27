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

#ifndef LURE_GAME_H
#define LURE_GAME_H


#include "common/config-manager.h"
#include "engines/engine.h"
#include "lure/luredefs.h"
#include "lure/menu.h"
#include "lure/palette.h"
#include "lure/disk.h"
#include "lure/memory.h"
#include "lure/screen.h"
#include "lure/events.h"
#include "lure/debugger.h"

namespace Lure {

enum GameState {GS_ERROR = 1, GS_TICK = 2, GS_TOCK = 4, GS_PROT = 8, GS_RESTART = 0x10,
	GS_CAUGHT = 0x20, GS_RESTORE = 0x40, GS_FLOPPY = 0x80,
	GS_RESTORE_RESTART = 0x50};

class Game {
private:
	Debugger *_debugger;
	bool _fastTextFlag, _soundFlag;
	uint8 _state;
	uint16 _tellCommands[MAX_TELL_COMMANDS * 3 + 1];
	int _numTellCommands;
	bool _preloadFlag;
	bool _debugFlag;

	void handleMenuResponse(uint8 selection);
	void handleClick();
	void handleRightClickMenu();
	void handleLeftClick();
	bool GetTellActions();
	void doAction(Action action, uint16 hotspotId, uint16 usedId);

	void playerChangeRoom();
	void displayChuteAnimation();
	void displayBarrelAnimation();
	void handleBootParam(int value);
	bool getYN();
	bool isMenuAvailable();
public:
	Game();
	virtual ~Game();

	static bool isCreated();
	static Game &getReference();
	void saveToStream(Common::WriteStream *stream);
	void loadFromStream(Common::ReadStream *stream);

	void tick();
	void tickCheck();
	void nextFrame();
	void execute();
	void setState(uint8 flags) { _state = flags; }
	bool &preloadFlag() { return _preloadFlag; }
	bool &debugFlag() { return _debugFlag; }
	bool fastTextFlag() { return _fastTextFlag; }
	bool soundFlag() { return _soundFlag; }
	Debugger &debugger() { return *_debugger; }

	// Menu item support methods
	void doDebugMenu();
	void doShowCredits();
	void doQuit();
	void doRestart();
	void doTextSpeed();
	void doSound();
};

} // End of namespace Lure

#endif
