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

#ifndef TONY_H
#define TONY_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/array.h"
#include "common/coroutines.h"
#include "common/error.h"
#include "common/random.h"
#include "common/util.h"
#include "engines/engine.h"

#include "tony/mpal/mpal.h"
#include "tony/mpal/memory.h"
#include "tony/debugger.h"
#include "tony/gfxengine.h"
#include "tony/loc.h"
#include "tony/utils.h"
#include "tony/window.h"
#include "tony/globals.h"

/**
 * This is the namespace of the Tony engine.
 *
 * Status of this engine: In Development
 *
 * Games using this engine:
 * - Tony Tough
 */
namespace Tony {

using namespace MPAL;

class Globals;

enum {
	kTonyDebugAnimations = 1 << 0,
	kTonyDebugActions = 1 << 1,
	kTonyDebugSound = 1 << 2,
	kTonyDebugMusic = 1 << 3,
	kTonyDebugMPAL = 1 << 4
};

#define DEBUG_BASIC 1
#define DEBUG_INTERMEDIATE 2
#define DEBUG_DETAILED 3

struct TonyGameDescription;

#define MAX_SFX_CHANNELS 32
#define TONY_DAT_VER_MAJ 0
#define TONY_DAT_VER_MIN 3

struct VoiceHeader {
	int _offset;
	int _code;
	int _parts;
};
#define VOICE_HEADER_SIZE 12

class TonyEngine : public Engine {
private:
	Common::ErrorCode init();
	bool loadTonyDat();
	void initMusic();
	void closeMusic();
	bool openVoiceDatabase();
	void closeVoiceDatabase();
	void initCustomFunctionMap();
	static void playProcess(CORO_PARAM, const void *param);
	static void doNextMusic(CORO_PARAM, const void *param);

protected:
	// Engine APIs
	virtual Common::Error run();
	virtual bool hasFeature(EngineFeature f) const;
public:
	LPCUSTOMFUNCTION _funcList[300];
	Common::String _funcListStrings[300];
	Common::RandomSource _randomSource;
	RMResUpdate _resUpdate;
	uint32 _hEndOfFrame;
	Common::File _vdbFP;
	Common::Array<VoiceHeader> _voices;
	FPSound _theSound;
	Common::List<FPSfx *> _activeSfx;
	Globals _globals;
	Debugger *_debugger;

	int16 _cTableDialog[256];
	int16 _lTableDialog[256];
	int16 _cTableMacc[256];
	int16 _lTableMacc[256];
	int16 _cTableCred[256];
	int16 _lTableCred[256];
	int16 _cTableObj[256];
	int16 _lTableObj[256];

	enum DataDir {
		DD_BASE = 1,
		DD_SAVE,
		DD_SHOTS,
		DD_MUSIC,
		DD_LAYER,
		DD_UTILSFX,
		DD_VOICES,
		DD_BASE2
	};

	FPStream *_stream[6];
	FPSfx *_sfx[MAX_SFX_CHANNELS];
	FPSfx *_utilSfx[MAX_SFX_CHANNELS];
	bool _bPaused;
	bool _bDrawLocation;
	int _startTime;
	uint16 *_curThumbnail;
	int _initialLoadSlotNumber;
	int _loadSlotNumber;

	// Bounding box list manager
	RMGameBoxes _theBoxes;
	RMWindow _window;
	RMGfxEngine _theEngine;

	bool _bQuitNow;
	bool _bTimeFreezed;
	int  _nTimeFreezed;
public:
	TonyEngine(OSystem *syst, const TonyGameDescription *gameDesc);
	virtual ~TonyEngine();

	const TonyGameDescription *_gameDescription;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;
	uint16 getVersion() const;
	bool getIsDemo() const;
	bool isCompressed() const;
	RMGfxEngine *getEngine() {
		return &_theEngine;
	}
	void GUIError(const Common::String &msg);

	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);

	void play();
	void close();

	void getDataDirectory(DataDir dir, char *path);

	void showLocation();
	void hideLocation();

	/**
	 * Reads the time
	 */
	uint32 getTime();
	void freezeTime();
	void unfreezeTime();

	// Music
	// ******
	void playMusic(int nChannel, const Common::String &fn, int nFX, bool bLoop, int nSync);
	void stopMusic(int nChannel);

	void playSFX(int nSfx, int nFX = 0);
	void stopSFX(int nSfx);

	void playUtilSFX(int nSfx, int nFX = 0);
	void stopUtilSFX(int nSfx);

	FPSfx *createSFX(Common::SeekableReadStream *stream);

	void preloadSFX(int nSfx, const char *fn);
	void unloadAllSFX();

	void preloadUtilSFX(int nSfx, const char *fn);
	void unloadAllUtilSFX();

	/**
	 * Stop all the audio
	 */
	void pauseSound(bool bPause);

	void setMusicVolume(int nChannel, int volume);
	int getMusicVolume(int nChannel);

	/**
	 * Handle saving
	 */
	void autoSave(CORO_PARAM);
	void saveState(int n, const char *name);
	void loadState(CORO_PARAM, int n);
	static Common::String getSaveStateFileName(int n);

	/**
	 * Get a thumbnail
	 */
	void grabThumbnail();
	uint16 *getThumbnail();

	void quitGame();

	void openInitLoadMenu(CORO_PARAM);
	void openInitOptions(CORO_PARAM);

	virtual void syncSoundSettings();
	void saveSoundSettings();
};

// Global reference to the TonyEngine object
extern TonyEngine *g_vm;

#define GLOBALS g_vm->_globals

} // End of namespace Tony

#endif /* TONY_H */
