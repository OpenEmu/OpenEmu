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

#ifndef CRUISE_CRUISE_H
#define CRUISE_CRUISE_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/random.h"

#include "engines/engine.h"

#include "cruise/cruise_main.h"
#include "cruise/debugger.h"
#include "cruise/sound.h"

/**
 * This is the namespace of the Cruise engine.
 *
 * Status of this engine: Game is completable, engine needs objectifying
 *
 * Games using this engine:
 * - Cruise for a Corpse
 */
namespace Cruise {

enum CruiseGameType {
	GType_CRUISE = 1
};

#define GAME_FRAME_DELAY_1 50
#define GAME_FRAME_DELAY_2 100

#define MAX_LANGUAGE_STRINGS 25

enum LangStringId { ID_PAUSED = 0, ID_INVENTORY = 5, ID_SPEAK_ABOUT = 6, ID_PLAYER_MENU = 7,
	ID_SAVE = 9, ID_LOAD = 10, ID_RESTART = 11, ID_QUIT = 12};

struct CRUISEGameDescription;

class CruiseEngine: public Engine {
private:
	bool _preLoad;
	Debugger *_debugger;
	PCSound *_sound;
	Common::StringArray _langStrings;
	CursorType _savedCursor;
	uint32 lastTick, lastTickDebug;
	int _gameSpeed;
	bool _speedFlag;

	void initialize();
	void deinitialize();
	bool loadLanguageStrings();
	void mainLoop();
	int processInput();
protected:
	// Engine APIs
	virtual Common::Error run();

	void shutdown();

	bool initGame();

public:
	CruiseEngine(OSystem * syst, const CRUISEGameDescription *gameDesc);
	virtual ~ CruiseEngine();
	virtual bool hasFeature(EngineFeature f) const;

	int getGameType() const;
	const char *getGameId() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	PCSound &sound() { return *_sound; }
	virtual GUI::Debugger *getDebugger() { return _debugger; }
	virtual void pauseEngine(bool pause);
	const char *langString(LangStringId langId) { return _langStrings[(int)langId].c_str(); }

	static const char *getSavegameFile(int saveGameIdx);
	virtual Common::Error loadGameState(int slot);
	virtual bool canLoadGameStateCurrently();
	virtual Common::Error saveGameState(int slot, const Common::String &desc);
	virtual bool canSaveGameStateCurrently();
	virtual void syncSoundSettings();

	const CRUISEGameDescription *_gameDescription;
	void initAllData();

	Common::RandomSource _rnd;

	struct MemInfo {
		int32  lineNum;
		char   fname[64];
		uint32 magic;

		static uint32 const cookie = 0x41424344;
	};

	Common::List<MemInfo *> _memList;

	typedef Common::List<Common::Rect> RectList;

	RectList _dirtyRects;
	RectList _priorFrameRects;

	Common::File _currentVolumeFile;

	Common::Array<CtStruct> _polyStructNorm;
	Common::Array<CtStruct> _polyStructExp;
	Common::Array<CtStruct> *_polyStructs;
	Common::Array<CtStruct> *_polyStruct;

	Common::File _PAL_file;
};

extern CruiseEngine *_vm;

#define BOOT_PRC_NAME "AUTO00.PRC"

enum {
	VAR_MOUSE_X_MODE = 253,
	VAR_MOUSE_X_POS = 249,
	VAR_MOUSE_Y_MODE = 251,
	VAR_MOUSE_Y_POS = 250
};

enum {
	MOUSE_CURSOR_NORMAL = 0,
	MOUSE_CURSOR_DISK,
	MOUSE_CURSOR_CROSS
};

enum {
	kCruiseDebugScript = 1 << 0,
	kCruiseDebugSound = 1 << 1
};

enum {
	kCmpEQ = (1 << 0),
	kCmpGT = (1 << 1),
	kCmpLT = (1 << 2)
};

} // End of namespace Cruise

#endif
