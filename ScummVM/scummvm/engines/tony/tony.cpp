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

#include "common/scummsys.h"
#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/installshield_cab.h"
#include "tony/tony.h"
#include "tony/custom.h"
#include "tony/debugger.h"
#include "tony/game.h"
#include "tony/mpal/mpal.h"

namespace Tony {

TonyEngine *g_vm;

TonyEngine::TonyEngine(OSystem *syst, const TonyGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("tony") {
	g_vm = this;
	_loadSlotNumber = -1;

	// Set the up the debugger
	_debugger = new Debugger();
	DebugMan.addDebugChannel(kTonyDebugAnimations, "animations", "Animations debugging");
	DebugMan.addDebugChannel(kTonyDebugActions, "actions", "Actions debugging");
	DebugMan.addDebugChannel(kTonyDebugSound, "sound", "Sound debugging");
	DebugMan.addDebugChannel(kTonyDebugMusic, "music", "Music debugging");

	// Add folders to the search directory list
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "Voices");
	SearchMan.addSubDirectoryMatching(gameDataDir, "Roasted");
	SearchMan.addSubDirectoryMatching(gameDataDir, "Music");
	SearchMan.addSubDirectoryMatching(gameDataDir, "Music/utilsfx");
	SearchMan.addSubDirectoryMatching(gameDataDir, "Music/Layer");

	// Set up load slot number
	_initialLoadSlotNumber = -1;
	if (ConfMan.hasKey("save_slot")) {
		int slotNumber = ConfMan.getInt("save_slot");
		if (slotNumber >= 0 && slotNumber <= 99)
			_initialLoadSlotNumber = slotNumber;
	}

	// Load the ScummVM sound settings
	syncSoundSettings();

	_hEndOfFrame = 0;
	for (int i = 0; i < 6; i++)
		_stream[i] = NULL;
	for (int i = 0; i < MAX_SFX_CHANNELS; i++) {
		_sfx[i] = NULL;
		_utilSfx[i] = NULL;
	}
	_bPaused = false;
	_bDrawLocation = false;
	_startTime = 0;
	_curThumbnail = NULL;
	_bQuitNow = false;
	_bTimeFreezed = false;
	_nTimeFreezed = 0;
}

TonyEngine::~TonyEngine() {
	// Close the voice database
	closeVoiceDatabase();

	// Reset the coroutine scheduler
	CoroScheduler.reset();
	CoroScheduler.setResourceCallback(NULL);

	delete _debugger;
}

/**
 * Run the game
 */
Common::Error TonyEngine::run() {
	Common::ErrorCode result = init();
	if (result != Common::kNoError)
		return result;

	play();
	close();

	return Common::kNoError;
}

/**
 * Initialize the game
 */
Common::ErrorCode TonyEngine::init() {
	// Load DAT file (used by font manager)
	if (!loadTonyDat())
		return Common::kUnknownError;

	if (isCompressed()) {
		Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember("data1.cab");
		if (!stream)
			error("Failed to open data1.cab");

		Common::Archive *cabinet = Common::makeInstallShieldArchive(stream);
		if (!cabinet)
			error("Failed to parse data1.cab");

		SearchMan.add("data1.cab", cabinet);
	}

	_hEndOfFrame = CoroScheduler.createEvent(false, false);

	_bPaused = false;
	_bDrawLocation = true;
	_startTime = g_system->getMillis();

	// Init static class fields
	RMText::initStatics();
	RMTony::initStatics();

	// Reset the scheduler
	CoroScheduler.reset();

	// Initialize the graphics window
	_window.init();

	// Initialize the function list
	Common::fill(_funcList, _funcList + 300, (LPCUSTOMFUNCTION)NULL);
	initCustomFunctionMap();

	// Initializes MPAL system, passing the custom functions list
	Common::File f;
	if (!f.open("ROASTED.MPC"))
		return Common::kReadingFailed;
	f.close();

	if (!mpalInit("ROASTED.MPC", "ROASTED.MPR", _funcList, _funcListStrings))
		return Common::kUnknownError;

	// Initialize the update resources
	_resUpdate.init("ROASTED.MPU");

	// Initialize the music
	initMusic();

	// Initialize the voices database
	if (!openVoiceDatabase())
		return Common::kReadingFailed;

	// Initialize the boxes
	_theBoxes.init();

	// Link to the custom graphics engine
	_theEngine.initCustomDll();
	_theEngine.init();

	// Allocate space for thumbnails when saving the game
	_curThumbnail = new uint16[160 * 120];

	_bQuitNow = false;

	return Common::kNoError;
}

bool TonyEngine::loadTonyDat() {
	Common::String msg;
	Common::File in;

	in.open("tony.dat");

	if (!in.isOpen()) {
		msg = "You're missing the 'tony.dat' file. Get it from the ScummVM website";
		GUIErrorMessage(msg);
		warning("%s", msg.c_str());
		return false;
	}

	// Read header
	char buf[4+1];
	in.read(buf, 4);
	buf[4] = '\0';

	if (strcmp(buf, "TONY")) {
		msg = "File 'tony.dat' is corrupt. Get it from the ScummVM website";
		GUIErrorMessage(msg);
		warning("%s", msg.c_str());
		return false;
	}

	int majVer = in.readByte();
	int minVer = in.readByte();

	if ((majVer != TONY_DAT_VER_MAJ) || (minVer != TONY_DAT_VER_MIN)) {
		msg = Common::String::format("File 'tony.dat' is wrong version. Expected %d.%d but got %d.%d. Get it from the ScummVM website", TONY_DAT_VER_MAJ, TONY_DAT_VER_MIN, majVer, minVer);
		GUIErrorMessage(msg);
		warning("%s", msg.c_str());

		return false;
	}

	int expectedLangVariant = -1;
	switch (g_vm->getLanguage()) {
	case Common::IT_ITA:
	case Common::EN_ANY:
		expectedLangVariant = 0;
		break;
	case Common::PL_POL:
		expectedLangVariant = 1;
		break;
	case Common::RU_RUS:
		expectedLangVariant = 2;
		break;
	case Common::CZ_CZE:
		expectedLangVariant = 3;
		break;
	case Common::FR_FRA:
		expectedLangVariant = 4;
		break;
	case Common::DE_DEU:
		expectedLangVariant = 5;
		break;
	default:
		warning("Unhandled language, falling back to English/Italian fonts.");
		expectedLangVariant = 0;
		break;
	}

	int numVariant = in.readUint16BE();
	if (expectedLangVariant > numVariant - 1) {
		msg = Common::String::format("Font variant not present in 'tony.dat'. Get it from the ScummVM website");
		GUIErrorMessage(msg);
		warning("%s", msg.c_str());

		return false;
	}

	in.seek(in.pos() + (2 * 256 * 8 * expectedLangVariant));
	for (int i = 0; i < 256; i++) {
		_cTableDialog[i] = in.readSint16BE();
		_lTableDialog[i] = in.readSint16BE();
		_cTableMacc[i] = in.readSint16BE();
		_lTableMacc[i] = in.readSint16BE();
		_cTableCred[i] = in.readSint16BE();
		_lTableCred[i] = in.readSint16BE();
		_cTableObj[i] = in.readSint16BE();
		_lTableObj[i] = in.readSint16BE();
	}

	return true;
}

void TonyEngine::initCustomFunctionMap() {
	INIT_CUSTOM_FUNCTION(_funcList, _funcListStrings);
}

/**
 * Display an error message
 */
void TonyEngine::GUIError(const Common::String &msg) {
	GUIErrorMessage(msg);
}

void TonyEngine::playMusic(int nChannel, const Common::String &fname, int nFX, bool bLoop, int nSync) {
	if (nChannel < 4) {
		if (GLOBALS._flipflop)
			nChannel = nChannel + 1;
	}

	switch (nFX) {
	case 0:
	case 1:
	case 2:
		_stream[nChannel]->stop();
		_stream[nChannel]->unloadFile();
		break;

	case 22:
		break;
	}

	if (nFX == 22) { // Sync a tempo
		GLOBALS._curChannel = nChannel;
		GLOBALS._nextLoop = bLoop;
		GLOBALS._nextSync = nSync;
		GLOBALS._nextMusic = fname;

		if (GLOBALS._flipflop)
			GLOBALS._nextChannel = nChannel - 1;
		else
			GLOBALS._nextChannel = nChannel + 1;

		uint32 hThread = CoroScheduler.createProcess(doNextMusic, NULL, 0);
		assert(hThread != CORO_INVALID_PID_VALUE);

	} else if (nFX == 44) { // Change the channel and let the first finish
		if (GLOBALS._flipflop)
			GLOBALS._nextChannel = nChannel - 1;
		else
			GLOBALS._nextChannel = nChannel + 1;

		_stream[GLOBALS._nextChannel]->stop();
		_stream[GLOBALS._nextChannel]->unloadFile();

		if (!getIsDemo()) {
			if (!_stream[GLOBALS._nextChannel]->loadFile(fname, FPCODEC_ADPCM, nSync))
				error("failed to open music file '%s'", fname.c_str());
		} else {
			_stream[GLOBALS._nextChannel]->loadFile(fname, FPCODEC_ADPCM, nSync);
		}

		_stream[GLOBALS._nextChannel]->setLoop(bLoop);
		_stream[GLOBALS._nextChannel]->play();

		GLOBALS._flipflop = 1 - GLOBALS._flipflop;
	} else {
		if (!getIsDemo()) {
			if (!_stream[nChannel]->loadFile(fname, FPCODEC_ADPCM, nSync))
				error("failed to open music file '%s'", fname.c_str());
		} else {
			_stream[nChannel]->loadFile(fname, FPCODEC_ADPCM, nSync);
		}

		_stream[nChannel]->setLoop(bLoop);
		_stream[nChannel]->play();
	}
}

void TonyEngine::doNextMusic(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	Common::String fn;
	CORO_END_CONTEXT(_ctx);

	FPStream **streams = g_vm->_stream;

	CORO_BEGIN_CODE(_ctx);

	if (!g_vm->getIsDemo()) {
		if (!streams[GLOBALS._nextChannel]->loadFile(GLOBALS._nextMusic, FPCODEC_ADPCM, GLOBALS._nextSync))
			error("failed to open next music file '%s'", GLOBALS._nextMusic.c_str());
	} else {
		streams[GLOBALS._nextChannel]->loadFile(GLOBALS._nextMusic, FPCODEC_ADPCM, GLOBALS._nextSync);
	}

	streams[GLOBALS._nextChannel]->setLoop(GLOBALS._nextLoop);
	//streams[GLOBALS._nextChannel]->prefetch();

	streams[GLOBALS._curChannel]->waitForSync(streams[GLOBALS._nextChannel]);

	streams[GLOBALS._curChannel]->unloadFile();

	GLOBALS._flipflop = 1 - GLOBALS._flipflop;

	CORO_END_CODE;
}

void TonyEngine::playSFX(int nChannel, int nFX) {
	if (_sfx[nChannel] == NULL)
		return;

	switch (nFX) {
	case 0:
		_sfx[nChannel]->setLoop(false);
		break;

	case 1:
		_sfx[nChannel]->setLoop(true);
		break;
	}

	_sfx[nChannel]->play();
}

void TonyEngine::stopMusic(int nChannel) {
	if (nChannel < 4)
		_stream[nChannel + GLOBALS._flipflop]->stop();
	else
		_stream[nChannel]->stop();
}

void TonyEngine::stopSFX(int nChannel) {
	_sfx[nChannel]->stop();
}

void TonyEngine::playUtilSFX(int nChannel, int nFX) {
	if (_utilSfx[nChannel] == NULL)
		return;

	switch (nFX) {
	case 0:
		_utilSfx[nChannel]->setLoop(false);
		break;

	case 1:
		_utilSfx[nChannel]->setLoop(true);
		break;
	}

	_utilSfx[nChannel]->setVolume(52);
	_utilSfx[nChannel]->play();
}

void TonyEngine::stopUtilSFX(int nChannel) {
	_utilSfx[nChannel]->stop();
}

void TonyEngine::preloadSFX(int nChannel, const char *fn) {
	if (_sfx[nChannel] != NULL) {
		_sfx[nChannel]->stop();
		_sfx[nChannel]->release();
		_sfx[nChannel] = NULL;
	}

	_theSound.createSfx(&_sfx[nChannel]);

	_sfx[nChannel]->loadFile(fn, FPCODEC_ADPCM);
}

FPSfx *TonyEngine::createSFX(Common::SeekableReadStream *stream) {
	FPSfx *sfx;

	_theSound.createSfx(&sfx);
	sfx->loadWave(stream);
	return sfx;
}

void TonyEngine::preloadUtilSFX(int nChannel, const char *fn) {
	if (_utilSfx[nChannel] != NULL) {
		_utilSfx[nChannel]->stop();
		_utilSfx[nChannel]->release();
		_utilSfx[nChannel] = NULL;
	}

	_theSound.createSfx(&_utilSfx[nChannel]);

	_utilSfx[nChannel]->loadFile(fn, FPCODEC_ADPCM);
	_utilSfx[nChannel]->setVolume(63);
}

void TonyEngine::unloadAllSFX() {
	for (int i = 0; i < MAX_SFX_CHANNELS; i++) {
		if (_sfx[i] != NULL) {
			_sfx[i]->stop();
			_sfx[i]->release();
			_sfx[i] = NULL;
		}
	}
}

void TonyEngine::unloadAllUtilSFX() {
	for (int i = 0; i < MAX_SFX_CHANNELS; i++) {
		if (_utilSfx[i] != NULL) {
			_utilSfx[i]->stop();
			_utilSfx[i]->release();
			_utilSfx[i] = NULL;
		}
	}
}

void TonyEngine::initMusic() {
	int i;

	_theSound.init();
	_theSound.setMasterVolume(63);

	for (i = 0; i < 6; i++)
		_theSound.createStream(&_stream[i]);

	for (i = 0; i < MAX_SFX_CHANNELS; i++) {
		_sfx[i] = _utilSfx[i] = NULL;
	}

	// Preload sound effects
	preloadUtilSFX(0, "U01.ADP"); // Reversed!!
	preloadUtilSFX(1, "U02.ADP");

	// Start check processes for sound
	CoroScheduler.createProcess(FPSfx::soundCheckProcess, NULL);
}

void TonyEngine::closeMusic() {
	for (int i = 0; i < 6; i++) {
		_stream[i]->stop();
		_stream[i]->unloadFile();
		_stream[i]->release();
	}

	unloadAllSFX();
	unloadAllUtilSFX();
}

void TonyEngine::pauseSound(bool bPause) {
	_theEngine.pauseSound(bPause);

	for (uint i = 0; i < 6; i++)
		if (_stream[i])
			_stream[i]->setPause(bPause);

	for (uint i = 0; i < MAX_SFX_CHANNELS; i++) {
		if (_sfx[i])
			_sfx[i]->setPause(bPause);
		if (_utilSfx[i])
			_utilSfx[i]->setPause(bPause);
	}
}

void TonyEngine::setMusicVolume(int nChannel, int volume) {
	_stream[nChannel + GLOBALS._flipflop]->setVolume(volume);
}

int TonyEngine::getMusicVolume(int nChannel) {
	int volume;
	_stream[nChannel + GLOBALS._flipflop]->getVolume(&volume);
	return volume;
}

Common::String TonyEngine::getSaveStateFileName(int n) {
	return Common::String::format("tony.%03d", n);
}

void TonyEngine::autoSave(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	Common::String buf;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	grabThumbnail();
	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);
	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, g_vm->_hEndOfFrame, CORO_INFINITE);
	_ctx->buf = getSaveStateFileName(0);
	_theEngine.saveState(_ctx->buf, (byte *)_curThumbnail, "Autosave");

	CORO_END_CODE;
}

void TonyEngine::saveState(int n, const char *name) {
	Common::String buf = getSaveStateFileName(n);
	_theEngine.saveState(buf.c_str(), (byte *)_curThumbnail, name);
}

void TonyEngine::loadState(CORO_PARAM, int n) {
	CORO_BEGIN_CONTEXT;
	Common::String buf;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->buf = getSaveStateFileName(n);
	CORO_INVOKE_1(_theEngine.loadState, _ctx->buf.c_str());

	CORO_END_CODE;
}

bool TonyEngine::openVoiceDatabase() {
	char id[4];
	uint32 numfiles;

	// Open the voices database
	if (!_vdbFP.open("voices.vdb"))
		return false;

	_vdbFP.seek(-8, SEEK_END);
	numfiles = _vdbFP.readUint32LE();
	_vdbFP.read(id, 4);

	if (id[0] != 'V' || id[1] != 'D' || id[2] != 'B' || id[3] != '1') {
		_vdbFP.close();
		return false;
	}

	// Read in the index
	_vdbFP.seek(-8 - (numfiles * VOICE_HEADER_SIZE), SEEK_END);

	for (uint32 i = 0; i < numfiles; ++i) {
		VoiceHeader vh;
		vh._offset = _vdbFP.readUint32LE();
		vh._code = _vdbFP.readUint32LE();
		vh._parts = _vdbFP.readUint32LE();

		_voices.push_back(vh);
	}

	return true;
}

void TonyEngine::closeVoiceDatabase() {
	if (_vdbFP.isOpen())
		_vdbFP.close();

	if (_voices.size() > 0)
		_voices.clear();
}

void TonyEngine::grabThumbnail() {
	_window.grabThumbnail(_curThumbnail);
}

uint16 *TonyEngine::getThumbnail() {
	return _curThumbnail;
}

void TonyEngine::quitGame() {
	_bQuitNow = true;
}

void TonyEngine::openInitLoadMenu(CORO_PARAM) {
	_theEngine.openOptionScreen(coroParam, 1);
}

void TonyEngine::openInitOptions(CORO_PARAM) {
	_theEngine.openOptionScreen(coroParam, 2);
}

/**
 * Main process for playing the game.
 *
 * @remarks     This needs to be in a separate process, since there are some things that can briefly
 * block the execution of process. For now, all ScummVm event handling is dispatched to within the context of this
 * process. If it ever proves a problem, we may have to look into whether it's feasible to have it still remain
 * in the outer 'main' process.
 */
void TonyEngine::playProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	Common::String fn;
	CORO_END_CONTEXT(_ctx);


	CORO_BEGIN_CODE(_ctx);

	// Game loop. We rely on the outer main process to detect if a shutdown is required,
	// and kill the scheudler and all the processes, including this one
	for (;;) {
		// If a savegame needs to be loaded, then do so
		if (g_vm->_loadSlotNumber != -1 && GLOBALS._gfxEngine != NULL) {
			_ctx->fn = getSaveStateFileName(g_vm->_loadSlotNumber);
			CORO_INVOKE_1(GLOBALS._gfxEngine->loadState, _ctx->fn);
			g_vm->_loadSlotNumber = -1;
		}

		// Wait for the next frame
		CORO_INVOKE_1(CoroScheduler.sleep, 50);

		// Call the engine to handle the next frame
		CORO_INVOKE_1(g_vm->_theEngine.doFrame, g_vm->_bDrawLocation);

		// Warns that a frame is finished
		CoroScheduler.pulseEvent(g_vm->_hEndOfFrame);

		// Handle drawing the frame
		if (!g_vm->_bPaused) {
			if (!g_vm->_theEngine._bWiping)
				g_vm->_window.getNewFrame(g_vm->_theEngine, NULL);
			else
				g_vm->_window.getNewFrame(g_vm->_theEngine, &g_vm->_theEngine._rcWipeEllipse);
		}

		// Paint the frame onto the screen
		g_vm->_window.repaint();

		// Signal the ScummVM debugger
		g_vm->_debugger->onFrame();
	}

	CORO_END_CODE;
}

/**
 * Play the game
 */
void TonyEngine::play() {
	// Create the game player process
	CoroScheduler.createProcess(playProcess, NULL);

	// Loop through calling the scheduler until it's time for the game to quit
	while (!shouldQuit() && !_bQuitNow) {
		// Delay for a brief amount
		g_system->delayMillis(10);

		// Call any scheduled processes
		CoroScheduler.schedule();
	}
}

void TonyEngine::close() {
	closeMusic();
	CoroScheduler.closeEvent(_hEndOfFrame);
	_theBoxes.close();
	_theEngine.close();
	_window.close();
	mpalFree();
	freeMpc();
	delete[] _curThumbnail;
}

void TonyEngine::freezeTime() {
	_bTimeFreezed = true;
	_nTimeFreezed = getTime() - _startTime;
}

void TonyEngine::unfreezeTime() {
	_bTimeFreezed = false;
}

/**
 * Returns the millisecond timer
 */
uint32 TonyEngine::getTime() {
	return g_system->getMillis();
}

bool TonyEngine::canLoadGameStateCurrently() {
	return GLOBALS._gfxEngine != NULL && GLOBALS._gfxEngine->canLoadSave();
}

bool TonyEngine::canSaveGameStateCurrently() {
	return GLOBALS._gfxEngine != NULL && GLOBALS._gfxEngine->canLoadSave();
}

Common::Error TonyEngine::loadGameState(int slot) {
	_loadSlotNumber = slot;
	return Common::kNoError;
}

Common::Error TonyEngine::saveGameState(int slot, const Common::String &desc) {
	if (!GLOBALS._gfxEngine)
		return Common::kUnknownError;

	RMGfxTargetBuffer &bigBuf = *GLOBALS._gfxEngine;
	RMSnapshot s;
	s.grabScreenshot(bigBuf, 4, _curThumbnail);

	GLOBALS._gfxEngine->saveState(getSaveStateFileName(slot), (byte *)_curThumbnail, desc);
	return Common::kNoError;
}

void TonyEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	GLOBALS._bCfgDubbing = !ConfMan.getBool("mute") && !ConfMan.getBool("speech_mute");
	GLOBALS._bCfgSFX = !ConfMan.getBool("mute") && !ConfMan.getBool("sfx_mute");
	GLOBALS._bCfgMusic = !ConfMan.getBool("mute") && !ConfMan.getBool("music_mute");

	GLOBALS._nCfgDubbingVolume = ConfMan.getInt("speech_volume") * 10 / 256;
	GLOBALS._nCfgSFXVolume = ConfMan.getInt("sfx_volume") * 10 / 256;
	GLOBALS._nCfgMusicVolume = ConfMan.getInt("music_volume") * 10 / 256;

	GLOBALS._bShowSubtitles = ConfMan.getBool("subtitles");
	GLOBALS._nCfgTextSpeed = ConfMan.getInt("talkspeed") * 10 / 256;
}

void TonyEngine::saveSoundSettings() {
	ConfMan.setBool("speech_mute", !GLOBALS._bCfgDubbing);
	ConfMan.setBool("sfx_mute", !GLOBALS._bCfgSFX);
	ConfMan.setBool("music_mute", !GLOBALS._bCfgMusic);

	ConfMan.setInt("speech_volume", GLOBALS._nCfgDubbingVolume * 256 / 10);
	ConfMan.setInt("sfx_volume", GLOBALS._nCfgSFXVolume * 256 / 10);
	ConfMan.setInt("music_volume", GLOBALS._nCfgMusicVolume * 256 / 10);

	ConfMan.setBool("subtitles", GLOBALS._bShowSubtitles);
	ConfMan.setInt("talkspeed", GLOBALS._nCfgTextSpeed * 256 / 10);
}

void TonyEngine::showLocation() {
	_bDrawLocation = true;
}

void TonyEngine::hideLocation() {
	_bDrawLocation = false;
}

} // End of namespace Tony
