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

#ifndef LASTEXPRESS_LOGIC_H
#define LASTEXPRESS_LOGIC_H

#include "lastexpress/shared.h"

#include "lastexpress/eventhandler.h"

#include "common/events.h"

namespace LastExpress {

class LastExpressEngine;

class Action;
class Beetle;
class Debugger;
class Entities;
class Fight;
class SaveLoad;
class State;

class Logic : public EventHandler {
public:
	Logic(LastExpressEngine *engine);
	~Logic();

	void eventMouse(const Common::Event &ev);
	void eventTick(const Common::Event &ev);

	void resetState();
	void gameOver(SavegameType type, uint32 value, SceneIndex sceneIndex, bool showScene) const;
	void playFinalSequence() const;
	void updateCursor(bool redraw = true) const;

	Action 	   *getGameAction()   { return _action; }
	Beetle     *getGameBeetle()   { return _beetle; }
	Entities   *getGameEntities() { return _entities; }
	Fight      *getGameFight()    { return _fight; }
	SaveLoad   *getGameSaveLoad() { return _saveload; }
	State      *getGameState()    { return _state; }

private:
	LastExpressEngine *_engine;

	Action   *_action;          ///< Actions
	Beetle   *_beetle;          ///< Beetle catching
	Entities *_entities;        ///< Entities
	Fight    *_fight;           ///< Fight handling
	SaveLoad *_saveload;        ///< Save & loading
	State    *_state;           ///< Game state

	void switchChapter() const;
	void showCredits() const;
	void redrawCursor() const;

	// Flags & Members
	bool _flagActionPerformed;
	bool _ignoreFrameInterval;
	int _ticksSinceLastSavegame;

	friend class Debugger;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_LOGIC_H
