/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
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
 */

#ifndef	SWORD2_H
#define	SWORD2_H

#define FRAMES_PER_SECOND 12

// Enable this to make it possible to clear the mouse cursor luggage by
// right-clicking. The original didn't do this, but it feels natural to me.
// However, I'm afraid that it'll interfer badly with parts of the game, so
// for now I'll keep it disabled.

#define RIGHT_CLICK_CLEARS_LUGGAGE 0

#include "engines/engine.h"

#include "common/events.h"
#include "common/util.h"
#include "common/random.h"

#define	MAX_starts	100
#define	MAX_description	100

class OSystem;

/**
 * This is the namespace of the Sword2 engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Broken Sword II: The Smoking Mirror
 */
namespace Sword2 {

enum {
	GF_DEMO	= 1 << 0
};

class MemoryManager;
class ResourceManager;
class Sound;
class Screen;
class Mouse;
class Logic;
class FontRenderer;
class Gui;
class Debugger;

enum {
	RD_LEFTBUTTONDOWN		= 0x01,
	RD_LEFTBUTTONUP			= 0x02,
	RD_RIGHTBUTTONDOWN		= 0x04,
	RD_RIGHTBUTTONUP		= 0x08,
	RD_WHEELUP			= 0x10,
	RD_WHEELDOWN			= 0x20,
	RD_KEYDOWN			= 0x40
};

struct MouseEvent {
	bool pending;
	uint16 buttons;
};

struct KeyboardEvent {
	bool pending;
	Common::KeyState kbd;
};

struct StartUp {
	char description[MAX_description];

	// id of screen manager object
	uint32 start_res_id;

	// Tell the manager which startup you want (if there are more than 1)
	// (i.e more than 1 entrance to a screen and/or separate game boots)
	uint32 key;
};

class Sword2Engine : public Engine {
private:
	uint32 _inputEventFilter;

	// The event "buffers"
	MouseEvent _mouseEvent;
	KeyboardEvent _keyboardEvent;

	uint32 _bootParam;
	int32 _saveSlot;

	void getPlayerStructures();
	void putPlayerStructures();

	uint32 saveData(uint16 slotNo, byte *buffer, uint32 bufferSize);
	uint32 restoreData(uint16 slotNo, byte *buffer, uint32 bufferSize);

	uint32 calcChecksum(byte *buffer, uint32 size);

	uint32 _totalStartups;
	uint32 _totalScreenManagers;
	uint32 _startRes;

	bool _useSubtitles;
	int _gameSpeed;

	// Used to trigger GMM Loading
	int _gmmLoadSlot;

	StartUp _startList[MAX_starts];

	// We need these to fetch data from SCREENS.CLU, which is
	// a resource file with custom format keeping background and
	// parallax data (which is removed from multiscreen files).
	byte *fetchPsxBackground(uint32 location);
	byte *fetchPsxParallax(uint32 location, uint8 level); // level: 0 -> bg, 1 -> fg

	// Original game platform (PC/PSX)
	static Common::Platform _platform;

protected:
	// Engine APIs
	virtual Common::Error run();
	virtual GUI::Debugger *getDebugger();
	virtual bool hasFeature(EngineFeature f) const;
	virtual void syncSoundSettings();
	virtual void pauseEngineIntern(bool pause);

public:
	Sword2Engine(OSystem *syst);
	~Sword2Engine();

	int getFramesPerSecond();

	void registerDefaultSettings();
	void readSettings();
	void writeSettings();

	void setupPersistentResources();

	bool getSubtitles() { return _useSubtitles; }
	void setSubtitles(bool b) { _useSubtitles = b; }

	// GMM Loading/Saving
	Common::Error saveGameState(int slot, const Common::String &desc);
	bool canSaveGameStateCurrently();
	Common::Error loadGameState(int slot);
	bool canLoadGameStateCurrently();

	uint32 _features;

	MemoryManager *_memory;
	ResourceManager	*_resman;
	Sound *_sound;
	Screen *_screen;
	Mouse *_mouse;
	Logic *_logic;
	FontRenderer *_fontRenderer;

	Debugger *_debugger;

	Common::RandomSource _rnd;

	uint32 _speechFontId;
	uint32 _controlsFontId;
	uint32 _redFontId;

	uint32 setInputEventFilter(uint32 filter);

	void parseInputEvents();

	bool checkForMouseEvents();
	MouseEvent *mouseEvent();
	KeyboardEvent *keyboardEvent();

	bool _wantSfxDebug;

	int32 _gameCycle;

#if RIGHT_CLICK_CLEARS_LUGGAGE
	bool heldIsInInventory();
#endif

	void fetchPalette(byte *screenFile, byte *palBuffer);
	byte *fetchScreenHeader(byte *screenFile);
	byte *fetchLayerHeader(byte *screenFile, uint16 layerNo);
	byte *fetchShadingMask(byte *screenFile);

	byte *fetchAnimHeader(byte *animFile);
	byte *fetchCdtEntry(byte *animFile, uint16 frameNo);
	byte *fetchFrameHeader(byte *animFile, uint16 frameNo);
	byte *fetchBackgroundParallaxLayer(byte *screenFile, int layer);
	byte *fetchBackgroundLayer(byte *screenFile);
	byte *fetchForegroundParallaxLayer(byte *screenFile, int layer);
	byte *fetchTextLine(byte *file, uint32 text_line);
	bool checkTextLine(byte *file, uint32 text_line);
	byte *fetchPaletteMatchTable(byte *screenFile);

	uint32 saveGame(uint16 slotNo, const byte *description);
	uint32 restoreGame(uint16 slotNo);
	uint32 getSaveDescription(uint16 slotNo, byte *description);
	bool saveExists();
	bool saveExists(uint16 slotNo);
	uint32 restoreFromBuffer(byte *buffer, uint32 size);
	Common::String getSaveFileName(uint16 slotNo);
	uint32 findBufferSize();

	void startGame();
	void gameCycle();
	void restartGame();

	void sleepUntil(uint32 time);

	void initializeFontResourceFlags();
	void initializeFontResourceFlags(uint8 language);

	bool initStartMenu();
	void registerStartPoint(int32 key, char *name);

	uint32 getNumStarts() { return _totalStartups; }
	uint32 getNumScreenManagers() { return _totalScreenManagers; }
	StartUp *getStartList() { return _startList; }

	void runStart(int start);

	// Convenience alias for OSystem::getMillis().
	// This is a bit hackish, of course :-).
	uint32 getMillis();

	//Used to check wether we are running PSX version
	static bool isPsx() { return _platform == Common::kPlatformPSX; }
};

} // End of namespace Sword2

#endif
