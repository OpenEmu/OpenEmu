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

#ifndef KYRA_KYRA_V1_H
#define KYRA_KYRA_V1_H

#include "engines/engine.h"

#include "common/array.h"
#include "common/error.h"
#include "common/events.h"
#include "common/hashmap.h"
#include "common/random.h"
#include "common/rendermode.h"

#include "audio/mixer.h"

#include "kyra/script.h"
#include "kyra/item.h"

namespace Common {
class SeekableReadStream;
class WriteStream;
} // End of namespace Common

namespace Graphics {
struct Surface;
}

class KyraMetaEngine;

/**
 * This is the namespace of the Kyra engine.
 *
 * Status of this engine:
 *
 * The KYRA engine supports all three Kyrandia games by Westwood. It also
 * supports Westwood's Lands of Lore. There are various platform ports of
 * the different games, almost all of them are fully supported. Only the
 * Macintosh port of Kyrandia 1 makes a difference here, which lacks support
 * for sound effects and music.
 *
 * The different translations of the games are mostly supported, since every
 * translation requires some work for kyra.dat for example, it is almost
 * impossible to support translations, without owning them. There a currently
 * a few reported unsupported translations:
 *
 * - Official translations
 * None known.
 * - Probably official translations (currently no sources are known to verify this)
 * Kyrandia 2 Spanish (feature request #2499966 "KYRA2: Add support for Spanish floppy version")
 * - Doubtful official translations (no sources here either, but less likely to be official)
 * Kyrandia 1 Korean (feature request #1758252 "KYRA1: Add support for Korean/DOS version")
 * Kyrandia 2 Polish (feature request #2146192 "KYRA2: Add support for Polish floppy version")
 * - Fan translations:
 * Kyrandia 3 Russian (feature request #2812792 "Kyrandia3 Russian")
 *
 * The primary maintainer for the engine is LordHoto, although some parts are
 * maintained by _athrxx. If you have questions about parts of the code, the
 * following rough description might help in determining who you should ask:
 * _athrxx is the maintainer for the Lands of Lore subengine, he also
 * maintains most of the FM-TOWNS and PC98 specific code (especially the sound
 * code, also some ingame code) and the Kyrandia 2 sequence player code.
 * LordHoto is responsible for the rest of the codebase, he also worked on the
 * graphics output for 16 color PC98 games.
 *
 * Other people who worked on this engine include cyx, who initially started
 * to work on Kyrandia 1 support. Vinterstum, who did various things for
 * Kyrandia 1 and started to work on the Kyrandia 2 sequence player code and
 * also on the TIM script code. Eriktorbjorn, who helped out naming our AdLib
 * player code and also contributed a work around for a music bug in the
 * "Pool of Sorrow" scene of Kyrandia 1, which is also present in the original.
 * He also contributed the VQA player for Kyrandia 3.
 *
 * The engine is mostly finished code wise. A possible remaining task is
 * proper refactoring, which might help in reducing binary size and along with
 * it runtime memory use, but of course might lead to regressions (since the
 * current code makes no problems on our low end ports, it is pretty minor
 * priority though, since the benefit would be mostly nicer code). The biggest
 * task left is the kyra.dat handling.
 *
 * Games using this engine:
 * - The Legend of Kyrandia (fully supported, except for Macintosh port, which lacks sound)
 * - (The) Hand of Fate (fully supported)
 * - Malcolm's Revenge (fully supported)
 * - Lands of Lore: The Throne of Chaos (fully supported)
 */
namespace Kyra {

struct GameFlags {
	Common::Language lang;

	// language overwrites of fan translations (only needed for multilingual games)
	Common::Language fanLang;
	Common::Language replacedLang;

	Common::Platform platform;

	bool isDemo               : 1;
	bool useAltShapeHeader    : 1;    // alternative shape header (uses 2 bytes more, those are unused though)
	bool isTalkie             : 1;
	bool isOldFloppy          : 1;
	bool useHiRes             : 1;
	bool use16ColorMode       : 1;
	bool useDigSound          : 1;
	bool useInstallerPackage  : 1;

	byte gameID;
};

struct KeyCodeHash : public Common::UnaryFunction<Common::KeyCode, uint> {
	uint operator()(Common::KeyCode val) const { return (uint)val; }
};

enum {
	GI_KYRA1 = 0,
	GI_KYRA2 = 1,
	GI_KYRA3 = 2,
	GI_LOL = 4,
	GI_EOB1 = 5,
	GI_EOB2 = 6
};

// TODO: this is just the start of makeing the debug output of the kyra engine a bit more useable
// in the future we maybe merge some flags  and/or create new ones
enum DebugLevels {
	kDebugLevelScriptFuncs = 1 <<  0, ///< debug level for o#_* functions
	kDebugLevelScript      = 1 <<  1, ///< debug level for "EMCInterpreter" functions
	kDebugLevelSprites     = 1 <<  2, ///< debug level for "Sprites" functions
	kDebugLevelScreen      = 1 <<  3, ///< debug level for "Screen" functions
	kDebugLevelSound       = 1 <<  4, ///< debug level for "Sound" functions
	kDebugLevelAnimator    = 1 <<  5, ///< debug level for "ScreenAnimator" functions
	kDebugLevelMain        = 1 <<  6, ///< debug level for common "KyraEngine(_v#)" functions && "TextDisplayer" functions
	kDebugLevelGUI         = 1 <<  7, ///< debug level for "KyraEngine*" gui functions
	kDebugLevelSequence    = 1 <<  8, ///< debug level for "SeqPlayer" functions
	kDebugLevelMovie       = 1 <<  9, ///< debug level for movie specific funtions
	kDebugLevelTimer       = 1 << 10  ///< debug level for "TimerManager" functions
};

enum AudioResourceSet {
	kMusicIntro = 0,
	kMusicIngame,
	kMusicFinale
};

class Screen;
class Resource;
class Sound;
class TextDisplayer;
class StaticResource;
class TimerManager;
class Debugger;
class GUI;

struct Button;

class KyraEngine_v1 : public Engine {
friend class Debugger;
friend class ::KyraMetaEngine;
friend class GUI;
friend class GUI_v1;
friend class GUI_EoB;
friend class SoundMidiPC;    // For _eventMan
friend class SeqPlayer_HOF; // For skipFlag()
friend class TransferPartyWiz; // For save state API
public:
	KyraEngine_v1(OSystem *system, const GameFlags &flags);
	virtual ~KyraEngine_v1();

	uint8 game() const { return _flags.gameID; }
	const GameFlags &gameFlags() const { return _flags; }

	// access to Kyra specific functionallity
	Resource *resource() { return _res; }
	virtual Screen *screen() = 0;
	virtual TextDisplayer *text() { return _text; }
	virtual GUI *gui() const { return 0; }
	Sound *sound() { return _sound; }
	StaticResource *staticres() { return _staticres; }
	TimerManager *timer() { return _timer; }

	uint32 tickLength() const { return _tickLength; }

	Common::RandomSource _rnd;

	// input
	void setMousePos(int x, int y);
	Common::Point getMousePos();

	// config specific
	bool speechEnabled();
	bool textEnabled();

	enum kVolumeEntry {
		kVolumeMusic = 0,
		kVolumeSfx = 1,
		kVolumeSpeech = 2
	};

	// volume reaches per default from 2 to 97
	void setVolume(kVolumeEntry vol, uint8 value);
	uint8 getVolume(kVolumeEntry vol);

	virtual void syncSoundSettings();

	// game flag handling
	int setGameFlag(int flag);
	int queryGameFlag(int flag) const;
	int resetGameFlag(int flag);

	// sound
	virtual void snd_playTheme(int file, int track);
	virtual void snd_playSoundEffect(int id, int volume=0xFF);
	virtual void snd_playWanderScoreViaMap(int command, int restart);
	virtual void snd_playVoiceFile(int id) = 0;
	virtual bool snd_voiceIsPlaying();
	virtual void snd_stopVoice();

	// delay functionallity
	virtual void delayUntil(uint32 timestamp, bool updateGameTimers = false, bool update = false, bool isMainLoop = false);
	virtual void delay(uint32 millis, bool update = false, bool isMainLoop = false);
	virtual void delayWithTicks(int ticks);

protected:
	// Engine APIs
	virtual Common::Error init();
	virtual Common::Error go() = 0;

	virtual Common::Error run() {
		Common::Error err;
		registerDefaultSettings();
		err = init();
		if (err.getCode() != Common::kNoError)
			return err;
		return go();
	}

	virtual ::GUI::Debugger *getDebugger();
	virtual bool hasFeature(EngineFeature f) const;
	virtual void pauseEngineIntern(bool pause);

	// intern
	Resource *_res;
	Sound *_sound;
	TextDisplayer *_text;
	StaticResource *_staticres;
	TimerManager *_timer;
	EMCInterpreter *_emc;
	Debugger *_debugger;

	// input
	void setupKeyMap();
	void updateInput();
	int checkInput(Button *buttonList, bool mainLoop = false, int eventFlag = 0x8000);
	void removeInputTop();

	int _mouseX, _mouseY;

	struct Event {
		Common::Event event;
		bool causedSkip;

		Event() : event(), causedSkip(false) {}
		Event(Common::Event e) : event(e), causedSkip(false) {}
		Event(Common::Event e, bool skip) : event(e), causedSkip(skip) {}

		operator Common::Event() const { return event; }
	};
	Common::List<Event> _eventList;
	typedef Common::HashMap<Common::KeyCode, int16, KeyCodeHash> KeyMap;
	KeyMap _keyMap;

	// config specific
	virtual void registerDefaultSettings();
	virtual void readSettings();
	virtual void writeSettings();

	uint8 _configWalkspeed;

	int _configMusic;
	bool _configSounds;
	uint8 _configVoice;

	Common::RenderMode _configRenderMode;

	// game speed
	virtual bool skipFlag() const;
	virtual void resetSkipFlag(bool removeEvent = true);

	uint16 _tickLength;
	uint16 _gameSpeed;

	// run
	int8 _deathHandler;

	// timer
	virtual void setupTimers() = 0;

	virtual void setWalkspeed(uint8 speed) = 0;

	// detection
	GameFlags _flags;

	// opcode
	virtual void setupOpcodeTable() = 0;
	Common::Array<const Opcode *> _opcodes;

	int o1_queryGameFlag(EMCState *script);
	int o1_setGameFlag(EMCState *script);
	int o1_resetGameFlag(EMCState *script);
	int o1_getRand(EMCState *script);
	int o1_hideMouse(EMCState *script);
	int o1_showMouse(EMCState *script);
	int o1_setMousePos(EMCState *script);
	int o1_setHandItem(EMCState *script);
	int o1_removeHandItem(EMCState *script);
	int o1_getMouseState(EMCState *script);
	int o1_setDeathHandler(EMCState *script);
	int o1_playWanderScoreViaMap(EMCState *script);
	int o1_fillRect(EMCState *script);
	int o1_blockInWalkableRegion(EMCState *script);
	int o1_blockOutWalkableRegion(EMCState *script);
	int o1_playSoundEffect(EMCState *script);

	// items
	int _mouseState;

	virtual void setHandItem(Item item) = 0;
	virtual void removeHandItem() = 0;

	// game flags
	uint8 _flagsTable[100]; // TODO: check this value

	// sound
	Audio::SoundHandle _speechHandle;

	int _curMusicTheme;
	int _curSfxFile;
	int16 _lastMusicCommand;

	const int8 *_trackMap;
	int _trackMapSize;

	virtual int convertVolumeToMixer(int value);
	virtual int convertVolumeFromMixer(int value);

	// pathfinder
	virtual int findWay(int x, int y, int toX, int toY, int *moveTable, int moveTableSize);
	int findSubPath(int x, int y, int toX, int toY, int *moveTable, int start, int end);
	int getFacingFromPointToPoint(int x, int y, int toX, int toY);
	int getOppositeFacingDirection(int dir);
	void changePosTowardsFacing(int &x, int &y, int facing);
	int getMoveTableSize(int *moveTable);
	virtual bool lineIsPassable(int x, int y) = 0;

	static const int8 _addXPosTable[];
	static const int8 _addYPosTable[];

	// Character

	static const int8 _charAddXPosTable[];
	static const int8 _charAddYPosTable[];

	// save/load
	int _gameToLoad;

	uint32 _lastAutosave;
	void checkAutosave();

	bool _isSaveAllowed;

	bool canLoadGameStateCurrently() { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() { return _isSaveAllowed; }

	const char *getSavegameFilename(int num);
	Common::String _savegameFilename;
	static Common::String getSavegameFilename(const Common::String &target, int num);
	bool saveFileLoadable(int slot);

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;

		bool originalSave;  // savegame from original interpreter
		bool oldHeader;     // old scummvm save header

		Graphics::Surface *thumbnail;
	};

	enum ReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	static ReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *file, bool loadThumbnail, SaveHeader &header);

	void loadGameStateCheck(int slot);
	virtual Common::Error loadGameState(int slot) = 0;
	Common::Error saveGameState(int slot, const Common::String &desc) { return saveGameStateIntern(slot, desc.c_str(), 0); }
	virtual Common::Error saveGameStateIntern(int slot, const char *saveName, const Graphics::Surface *thumbnail) = 0;

	Common::SeekableReadStream *openSaveForReading(const char *filename, SaveHeader &header, bool checkID = true);
	Common::WriteStream *openSaveForWriting(const char *filename, const char *saveName, const Graphics::Surface *thumbnail) const;

	// TODO: Consider moving this to Screen
	virtual Graphics::Surface *generateSaveThumbnail() const { return 0; }
};

} // End of namespace Kyra

#endif
