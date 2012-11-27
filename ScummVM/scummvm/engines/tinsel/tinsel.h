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

#ifndef TINSEL_H
#define TINSEL_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/random.h"
#include "common/util.h"

#include "engines/engine.h"
#include "tinsel/debugger.h"
#include "tinsel/graphics.h"
#include "tinsel/sound.h"
#include "tinsel/dw.h"

/**
 * This is the namespace of the Tinsel engine.
 *
 * Status of this engine: Complete
 *
 * Games using this engine:
 * - Discworld
 * - Discworld 2: Missing Presumed ...!?
 */
namespace Tinsel {

class BMVPlayer;
class Config;
class MidiDriver;
class MidiMusicPlayer;
class PCMMusicPlayer;
class SoundManager;

typedef Common::List<Common::Rect> RectList;

enum TinselGameID {
	GID_DW1 = 0,
	GID_DW2 = 1
};

enum TinselGameFeatures {
	GF_SCNFILES = 1 << 0,
	GF_ENHANCED_AUDIO_SUPPORT = 1 << 1,
	GF_ALT_MIDI = 1 << 2,		// Alternate sequence in midi.dat file

	// The GF_USE_?FLAGS values specify how many country flags are displayed
	// in the subtitles options dialog.
	// None of these defined -> 1 language, in ENGLISH.TXT
	GF_USE_3FLAGS = 1 << 3,	// French, German, Spanish
	GF_USE_4FLAGS = 1 << 4,	// French, German, Spanish, Italian
	GF_USE_5FLAGS = 1 << 5	// All 5 flags
};

/**
 * The following is the ScummVM definitions of the various Tinsel versions:
 * TINSEL_V0 - This was an early engine version that was only used in the Discworld 1
 *			demo. It is not currently supported.
 * TINSEL_V1 - This was the engine version used by Discworld 1. Note that there were two
 *			major releases: an earlier version that used *.gra files, and a later one that
 *			used *.scn files, and contained certain script and engine bugfixes. In ScummVM,
 *			we treat both releases as 'Tinsel 1', since the engine fixes from the later
 *			version work equally well the earlier version data.
 * TINSEL_V2 - This is the engine used for the Discworld 2 game.
 */
enum TinselEngineVersion {
	TINSEL_V0 = 0,
	TINSEL_V1 = 1,
	TINSEL_V2 = 2
};

enum {
	kTinselDebugAnimations = 1 << 0,
	kTinselDebugActions = 1 << 1,
	kTinselDebugSound = 1 << 2,
	kTinselDebugMusic = 2 << 3
};

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

struct TinselGameDescription;

enum TinselKeyDirection {
	MSK_LEFT = 1, MSK_RIGHT = 2, MSK_UP = 4, MSK_DOWN = 8,
	MSK_DIRECTION = MSK_LEFT | MSK_RIGHT | MSK_UP | MSK_DOWN
};

typedef bool (*KEYFPTR)(const Common::KeyState &);

#define	SCREEN_WIDTH	(_vm->screen().w)	// PC screen dimensions
#define	SCREEN_HEIGHT	(_vm->screen().h)
#define	SCRN_CENTER_X	((SCREEN_WIDTH  - 1) / 2)	// screen center x
#define	SCRN_CENTER_Y	((SCREEN_HEIGHT - 1) / 2)	// screen center y
#define UNUSED_LINES	48
#define EXTRA_UNUSED_LINES	3
//#define	SCREEN_BOX_HEIGHT1	(SCREEN_HEIGHT - UNUSED_LINES)
//#define	SCREEN_BOX_HEIGHT2	(SCREEN_BOX_HEIGHT1 - EXTRA_UNUSED_LINES)
#define	SCREEN_BOX_HEIGHT1	SCREEN_HEIGHT
#define	SCREEN_BOX_HEIGHT2	SCREEN_HEIGHT

#define GAME_FRAME_DELAY (1000 / ONE_SECOND)

#define TinselVersion (_vm->getVersion())
#define TinselV0 (TinselVersion == TINSEL_V0)
#define TinselV1 (TinselVersion == TINSEL_V1)
#define TinselV2 (TinselVersion == TINSEL_V2)
#define TinselV2Demo (TinselVersion == TINSEL_V2 && _vm->getIsADGFDemo())
#define TinselV1PSX (TinselVersion == TINSEL_V1 && _vm->getPlatform() == Common::kPlatformPSX)
#define TinselV1Mac (TinselVersion == TINSEL_V1 && _vm->getPlatform() == Common::kPlatformMacintosh)

#define READ_16(v) (TinselV1Mac ? READ_BE_UINT16(v) : READ_LE_UINT16(v))
#define READ_32(v) (TinselV1Mac ? READ_BE_UINT32(v) : READ_LE_UINT32(v))

// Global reference to the TinselEngine object
extern TinselEngine *_vm;

class TinselEngine : public Engine {
	int _gameId;
	Common::KeyState _keyPressed;
	Common::RandomSource _random;
	Graphics::Surface _screenSurface;
	Common::Point _mousePos;
	uint8 _dosPlayerDir;
	Console *_console;

	static const char *const _sampleIndices[][3];
	static const char *const _sampleFiles[][3];
	static const char *const _textFiles[][3];

protected:

	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
	Common::Error loadGameState(int slot);
#if 0
	Common::Error saveGameState(int slot, const Common::String &desc);
#endif
	bool canLoadGameStateCurrently();
#if 0
	bool canSaveGameStateCurrently();
#endif

public:
	TinselEngine(OSystem *syst, const TinselGameDescription *gameDesc);
	virtual ~TinselEngine();
	int getGameId() {
		return _gameId;
	}

	const TinselGameDescription *_gameDescription;
	uint32 getGameID() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	uint16 getVersion() const;
	uint32 getFlags() const;
	Common::Platform getPlatform() const;
	bool getIsADGFDemo() const;
	bool isCD() const;

	const char *getSampleIndex(LANGUAGE lang);
	const char *getSampleFile(LANGUAGE lang);
	const char *getTextFile(LANGUAGE lang);

	MidiDriver *_driver;
	SoundManager *_sound;
	MidiMusicPlayer *_midiMusic;
	PCMMusicPlayer *_pcmMusic;
	BMVPlayer *_bmv;

	Config *_config;

	KEYFPTR _keyHandler;

	/** Stack of pending mouse button events. */
	Common::List<Common::EventType> _mouseButtons;

	/** Stack of pending keypresses. */
	Common::List<Common::Event> _keypresses;


	/** List of all clip rectangles. */
	RectList _clipRects;

private:
	void NextGameCycle();
	void CreateConstProcesses();
	void RestartGame();
	void RestartDrivers();
	void ChopDrivers();
	void ProcessKeyEvent(const Common::Event &event);
	bool pollEvent();

public:
	const Common::String getTargetName() const { return _targetName; }
	Common::String getSavegameFilename(int16 saveNum) const;
	Common::SaveFileManager *getSaveFileMan() { return _saveFileMan; }
	Graphics::Surface &screen() { return _screenSurface; }

	Common::Point getMousePosition() const { return _mousePos; }
	void setMousePosition(const Common::Point &pt) {
		int yOffset = TinselV2 ? (g_system->getHeight() - _screenSurface.h) / 2 : 0;
		g_system->warpMouse(pt.x, pt.y + yOffset);
		_mousePos = pt;
	}
	void divertKeyInput(KEYFPTR fptr) { _keyHandler = fptr; }
	int getRandomNumber(int maxNumber) { return _random.getRandomNumber(maxNumber); }
	uint8 getKeyDirection() const { return _dosPlayerDir; }
};

// Externally available methods
void CuttingScene(bool bCutting);
void CDChangeForRestore(int cdNumber);
void CdHasChanged();

} // End of namespace Tinsel

#endif /* TINSEL_H */
