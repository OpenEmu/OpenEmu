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

#ifndef LASTEXPRESS_H
#define LASTEXPRESS_H

#include "lastexpress/debug.h"
#include "lastexpress/eventhandler.h"

#include "common/random.h"

#include "engines/engine.h"

#include "graphics/pixelformat.h"

struct ADGameDescription;

/**
 * This is the namespace of the LastExpress engine.
 *
 * Status of this engine:
 *  The game is playable but still very buggy and missing crucial functionality:
 *    - Resources: classes for the resource formats used by the game are mostly
 *      complete (subtitles integration/cursor transparency are missing)
 *    - Display: basic graphic manager functionality is implemented (transitions
 *      and dirty rects handling are missing)
 *    - Menu/Navigation: menu is done and navigation/hotspot handling are also
 *      mostly implemented (with remaining bugs)
 *    - Logic: all the hardcoded AI logic has been implemented, as well as the
 *      shared entity code for drawing/handling of entities.
 *    - Sound: most of the sound queue functionality is still missing
 *    - Savegame: almost all the savegame code is still missing.
 *
 * Maintainers:
 *  littleboy, jvprat, clone2727
 *
 * Supported games:
 *  - The Last Express
 */
namespace LastExpress {

class Cursor;
class Font;
class GraphicsManager;
class Logic;
class Menu;
class ResourceManager;
class SceneManager;
class SoundManager;

class LastExpressEngine : public Engine {
protected:
	// Engine APIs
	Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
	virtual Debugger *getDebugger() { return _debugger; }

public:
	LastExpressEngine(OSystem *syst, const ADGameDescription *gd);
	~LastExpressEngine();

	// Misc
	Common::RandomSource getRandom() const {return _random; }

	// Game
	Cursor          *getCursor()          const { return _cursor; }
	Font            *getFont()            const { return _font; }
	Logic           *getGameLogic()       const { return _logic; }
	Menu            *getGameMenu()        const { return _menu; }

	// Managers
	GraphicsManager *getGraphicsManager() const { return _graphicsMan; }
	ResourceManager *getResourceManager() const { return _resMan; }
	SceneManager    *getSceneManager()    const { return _sceneMan; }
	SoundManager    *getSoundManager()    const { return _soundMan; }

	// Event handling
	bool handleEvents();
	void pollEvents();

	void backupEventHandlers();
	void restoreEventHandlers();
	void setEventHandlers(EventHandler::EventFunction *eventMouse, EventHandler::EventFunction *eventTick);

	bool isDemo() const;

	// Frame Counter
	uint32 getFrameCounter() { return _frameCounter; }
	void setFrameCounter(uint32 count) { _frameCounter = count; }

protected:
	// Sound Timer
	static void soundTimer(void *ptr);
	void handleSoundTimer();

private:
	const ADGameDescription *_gameDescription;
	Graphics::PixelFormat _pixelFormat;

	// Misc
	Debugger *_debugger;
	Common::RandomSource _random;

	// Game
	Cursor *_cursor;
	Font   *_font;
	Logic  *_logic;
	Menu   *_menu;

	// Frame counter
	uint32 _frameCounter;
	uint32 _lastFrameCount;

	// Managers
	GraphicsManager *_graphicsMan;
	ResourceManager *_resMan;
	SceneManager    *_sceneMan;
	SoundManager    *_soundMan;

	// Event handlers
	EventHandler::EventFunction *_eventMouse;
	EventHandler::EventFunction *_eventTick;

	EventHandler::EventFunction *_eventMouseBackup;
	EventHandler::EventFunction *_eventTickBackup;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_H
