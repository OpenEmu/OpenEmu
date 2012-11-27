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

#define FORBIDDEN_SYMBOL_EXCEPTION_getcwd

#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
// winnt.h defines ARRAYSIZE, but we want our own one...
#undef ARRAYSIZE
#endif

#include "engines/engine.h"
#include "engines/dialogs.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"
#include "common/str.h"
#include "common/error.h"
#include "common/list.h"
#include "common/list_intern.h"
#include "common/scummsys.h"
#include "common/taskbar.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "backends/keymapper/keymapper.h"

#include "gui/debugger.h"
#include "gui/dialog.h"
#include "gui/message.h"

#include "audio/mixer.h"

#include "graphics/cursorman.h"
#include "graphics/pixelformat.h"

#ifdef _WIN32_WCE
extern bool isSmartphone();
#endif

// FIXME: HACK for error()
Engine *g_engine = 0;

// Output formatter for debug() and error() which invokes
// the errorString method of the active engine, if any.
static void defaultOutputFormatter(char *dst, const char *src, size_t dstSize) {
	if (g_engine) {
		g_engine->errorString(src, dst, dstSize);
	} else {
		Common::strlcpy(dst, src, dstSize);
	}
}

static void defaultErrorHandler(const char *msg) {
	// Unless this error -originated- within the debugger itself, we
	// now invoke the debugger, if available / supported.
	if (g_engine) {
		GUI::Debugger *debugger = g_engine->getDebugger();
#ifdef _WIN32_WCE
		if (isSmartphone())
			debugger = 0;
#endif

#if defined(USE_TASKBAR)
		g_system->getTaskbarManager()->notifyError();
#endif

		if (debugger && !debugger->isActive()) {
			debugger->attach(msg);
			debugger->onFrame();
		}


#if defined(USE_TASKBAR)
		g_system->getTaskbarManager()->clearError();
#endif

	}
}


Engine::Engine(OSystem *syst)
	: _system(syst),
		_mixer(_system->getMixer()),
		_timer(_system->getTimerManager()),
		_eventMan(_system->getEventManager()),
		_saveFileMan(_system->getSavefileManager()),
		_targetName(ConfMan.getActiveDomainName()),
		_pauseLevel(0),
		_pauseStartTime(0),
		_saveSlotToLoad(-1),
		_engineStartTime(_system->getMillis()),
		_mainMenuDialog(NULL) {

	g_engine = this;
	Common::setErrorOutputFormatter(defaultOutputFormatter);
	Common::setErrorHandler(defaultErrorHandler);

	// FIXME: Get rid of the following again. It is only here
	// temporarily. We really should never run with a non-working Mixer,
	// so ought to handle this at a much earlier stage. If we *really*
	// want to support systems without a working mixer, then we need
	// more work. E.g. we could modify the Mixer to immediately drop any
	// streams passed to it. This way, at least we don't crash because
	// heaps of (sound) memory get allocated but never freed. Of course,
	// there still would be problems with many games...
	if (!_mixer->isReady())
		warning("Sound initialization failed. This may cause severe problems in some games");

	// Setup a dummy cursor and palette, so that all engines can use
	// CursorMan.replace without having any headaches about memory leaks.
	//
	// If an engine only used CursorMan.replaceCursor and no cursor has
	// been setup before, then replaceCursor just uses pushCursor. This
	// means that that the engine's cursor is never again removed from
	// CursorMan. Hence we setup a fake cursor here and remove it again
	// in the destructor.
	CursorMan.pushCursor(NULL, 0, 0, 0, 0, 0);
	// Note: Using this dummy palette will actually disable cursor
	// palettes till the user enables it again.
	CursorMan.pushCursorPalette(NULL, 0, 0);
}

Engine::~Engine() {
	_mixer->stopAll();

	delete _mainMenuDialog;
	g_engine = NULL;

	// Remove our cursors again to prevent memory leaks
	CursorMan.popCursor();
	CursorMan.popCursorPalette();
}

void initCommonGFX(bool defaultTo1XScaler) {
	const Common::ConfigManager::Domain *transientDomain = ConfMan.getDomain(Common::ConfigManager::kTransientDomain);
	const Common::ConfigManager::Domain *gameDomain = ConfMan.getActiveDomain();

	assert(transientDomain);

	const bool useDefaultGraphicsMode =
		(!transientDomain->contains("gfx_mode") ||
		!scumm_stricmp(transientDomain->getVal("gfx_mode").c_str(), "normal") ||
		!scumm_stricmp(transientDomain->getVal("gfx_mode").c_str(), "default")
		)
		 &&
		(
		!gameDomain ||
		!gameDomain->contains("gfx_mode") ||
		!scumm_stricmp(gameDomain->getVal("gfx_mode").c_str(), "normal") ||
		!scumm_stricmp(gameDomain->getVal("gfx_mode").c_str(), "default")
		);

	// See if the game should default to 1x scaler
	if (useDefaultGraphicsMode && defaultTo1XScaler) {
		g_system->resetGraphicsScale();
	} else {
		// Override global scaler with any game-specific define
		if (ConfMan.hasKey("gfx_mode")) {
			g_system->setGraphicsMode(ConfMan.get("gfx_mode").c_str());
		}
	}

	// Note: The following code deals with the fullscreen / ASR settings. This
	// is a bit tricky, because there are three ways the user can affect these
	// settings: Via the config file, via the command line, and via in-game
	// hotkeys.
	// Any global or command line settings already have been applied at the time
	// we get here. Hence we only do something

	// (De)activate aspect-ratio correction as determined by the config settings
	if (gameDomain && gameDomain->contains("aspect_ratio"))
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio"));

	// (De)activate fullscreen mode as determined by the config settings
	if (gameDomain && gameDomain->contains("fullscreen"))
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen"));
}

void initGraphics(int width, int height, bool defaultTo1xScaler, const Graphics::PixelFormat *format) {

	g_system->beginGFXTransaction();

		initCommonGFX(defaultTo1xScaler);
#ifdef USE_RGB_COLOR
		if (format)
			g_system->initSize(width, height, format);
		else {
			Graphics::PixelFormat bestFormat = g_system->getSupportedFormats().front();
			g_system->initSize(width, height, &bestFormat);
		}
#else
		g_system->initSize(width, height);
#endif

	OSystem::TransactionError gfxError = g_system->endGFXTransaction();

	if (gfxError == OSystem::kTransactionSuccess)
		return;

	// Error out on size switch failure
	if (gfxError & OSystem::kTransactionSizeChangeFailed) {
		Common::String message;
		message = Common::String::format("Could not switch to resolution: '%dx%d'.", width, height);

		GUIErrorMessage(message);
		error("%s", message.c_str());
	}

	// Just show warnings then these occur:
#ifdef USE_RGB_COLOR
	if (gfxError & OSystem::kTransactionFormatNotSupported) {
		Common::String message = _("Could not initialize color format.");

		GUI::MessageDialog dialog(message);
		dialog.runModal();
	}
#endif

	if (gfxError & OSystem::kTransactionModeSwitchFailed) {
		Common::String message = _("Could not switch to video mode: '");
		message += ConfMan.get("gfx_mode");
		message += "'.";

		GUI::MessageDialog dialog(message);
		dialog.runModal();
	}

	if (gfxError & OSystem::kTransactionAspectRatioFailed) {
		GUI::MessageDialog dialog(_("Could not apply aspect ratio setting."));
		dialog.runModal();
	}

	if (gfxError & OSystem::kTransactionFullscreenFailed) {
		GUI::MessageDialog dialog(_("Could not apply fullscreen setting."));
		dialog.runModal();
	}
}


using Graphics::PixelFormat;

/**
 * Determines the first matching format between two lists.
 *
 * @param backend	The higher priority list, meant to be a list of formats supported by the backend
 * @param frontend	The lower priority list, meant to be a list of formats supported by the engine
 * @return			The first item on the backend list that also occurs on the frontend list
 *					or PixelFormat::createFormatCLUT8() if no matching formats were found.
 */
inline PixelFormat findCompatibleFormat(Common::List<PixelFormat> backend, Common::List<PixelFormat> frontend) {
#ifdef USE_RGB_COLOR
	for (Common::List<PixelFormat>::iterator i = backend.begin(); i != backend.end(); ++i) {
		for (Common::List<PixelFormat>::iterator j = frontend.begin(); j != frontend.end(); ++j) {
			if (*i == *j)
				return *i;
		}
	}
#endif
	return PixelFormat::createFormatCLUT8();
}


void initGraphics(int width, int height, bool defaultTo1xScaler, const Common::List<Graphics::PixelFormat> &formatList) {
	Graphics::PixelFormat format = findCompatibleFormat(g_system->getSupportedFormats(), formatList);
	initGraphics(width, height, defaultTo1xScaler, &format);
}

void initGraphics(int width, int height, bool defaultTo1xScaler) {
	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(width, height, defaultTo1xScaler, &format);
}

void GUIErrorMessage(const Common::String &msg) {
	g_system->setWindowCaption("Error");
	g_system->beginGFXTransaction();
		initCommonGFX(false);
		g_system->initSize(320, 200);
	if (g_system->endGFXTransaction() == OSystem::kTransactionSuccess) {
		GUI::MessageDialog dialog(msg);
		dialog.runModal();
	} else {
		error("%s", msg.c_str());
	}
}

void Engine::checkCD() {
#if defined(WIN32) && !defined(_WIN32_WCE) && !defined(__SYMBIAN32__)
	// It is a known bug under Windows that games that play CD audio cause
	// ScummVM to crash if the data files are read from the same CD. Check
	// if this appears to be the case and issue a warning.

	// If we can find a compressed audio track, then it should be ok even
	// if it's running from CD.

#ifdef USE_VORBIS
	if (Common::File::exists("track1.ogg") ||
	    Common::File::exists("track01.ogg"))
		return;
#endif
#ifdef USE_FLAC
	if (Common::File::exists("track1.fla") ||
            Common::File::exists("track1.flac") ||
	    Common::File::exists("track01.fla") ||
	    Common::File::exists("track01.flac"))
		return;
#endif
#ifdef USE_MAD
	if (Common::File::exists("track1.mp3") ||
	    Common::File::exists("track01.mp3"))
		return;
#endif

	char buffer[MAXPATHLEN];
	int i;

	const Common::FSNode gameDataDir(ConfMan.get("path"));

	if (gameDataDir.getPath().empty()) {
		// That's it! I give up!
		if (getcwd(buffer, MAXPATHLEN) == NULL)
			return;
	} else
		Common::strlcpy(buffer, gameDataDir.getPath().c_str(), sizeof(buffer));

	for (i = 0; i < MAXPATHLEN - 1; i++) {
		if (buffer[i] == '\\')
			break;
	}

	buffer[i + 1] = 0;

	if (GetDriveType(buffer) == DRIVE_CDROM) {
		GUI::MessageDialog dialog(
			_("You appear to be playing this game directly\n"
			"from the CD. This is known to cause problems,\n"
			"and it is therefore recommended that you copy\n"
			"the data files to your hard disk instead.\n"
			"See the README file for details."), _("OK"));
		dialog.runModal();
	} else {
		// If we reached here, the game has audio tracks,
		// it's not ran from the CD and the tracks have not
		// been ripped.
		GUI::MessageDialog dialog(
			_("This game has audio tracks in its disk. These\n"
			"tracks need to be ripped from the disk using\n"
			"an appropriate CD audio extracting tool in\n"
			"order to listen to the game's music.\n"
			"See the README file for details."), _("OK"));
		dialog.runModal();
	}
#endif
}

bool Engine::shouldPerformAutoSave(int lastSaveTime) {
	const int diff = _system->getMillis() - lastSaveTime;
	const int autosavePeriod = ConfMan.getInt("autosave_period");
	return autosavePeriod != 0 && diff > autosavePeriod * 1000;
}

void Engine::errorString(const char *buf1, char *buf2, int size) {
	Common::strlcpy(buf2, buf1, size);
}

void Engine::pauseEngine(bool pause) {
	assert((pause && _pauseLevel >= 0) || (!pause && _pauseLevel));

	if (pause)
		_pauseLevel++;
	else
		_pauseLevel--;

	if (_pauseLevel == 1 && pause) {
		_pauseStartTime = _system->getMillis();
		pauseEngineIntern(true);
	} else if (_pauseLevel == 0) {
		pauseEngineIntern(false);
		_engineStartTime += _system->getMillis() - _pauseStartTime;
		_pauseStartTime = 0;
	}
}

void Engine::pauseEngineIntern(bool pause) {
	// By default, just (un)pause all digital sounds
	_mixer->pauseAll(pause);
}

void Engine::openMainMenuDialog() {
	if (!_mainMenuDialog)
		_mainMenuDialog = new MainMenuDialog(this);

	setGameToLoadSlot(-1);

	runDialog(*_mainMenuDialog);

	// Load savegame after main menu execution
	// (not from inside the menu loop to avoid
	// mouse cursor glitches and simliar bugs,
	// e.g. #2822778).
	if (_saveSlotToLoad >= 0) {
		Common::Error status = loadGameState(_saveSlotToLoad);
		if (status.getCode() != Common::kNoError) {
			Common::String failMessage = Common::String::format(_("Gamestate load failed (%s)! "
				  "Please consult the README for basic information, and for "
				  "instructions on how to obtain further assistance."), status.getDesc().c_str());
			GUI::MessageDialog dialog(failMessage);
			dialog.runModal();
		}
	}

	syncSoundSettings();
}

bool Engine::warnUserAboutUnsupportedGame() {
	if (ConfMan.getBool("enable_unsupported_game_warning")) {
		GUI::MessageDialog alert(_("WARNING: The game you are about to start is"
			" not yet fully supported by ScummVM. As such, it is likely to be"
			" unstable, and any saves you make might not work in future"
			" versions of ScummVM."), _("Start anyway"), _("Cancel"));
		return alert.runModal() == GUI::kMessageOK;
	}
	return true;
}

uint32 Engine::getTotalPlayTime() const {
	if (!_pauseLevel)
		return _system->getMillis() - _engineStartTime;
	else
		return _pauseStartTime - _engineStartTime;
}

void Engine::setTotalPlayTime(uint32 time) {
	const uint32 currentTime = _system->getMillis();

	// We need to reset the pause start time here in case the engine is already
	// paused to avoid any incorrect play time counting.
	if (_pauseLevel > 0)
		_pauseStartTime = currentTime;

	_engineStartTime = currentTime - time;
}

int Engine::runDialog(GUI::Dialog &dialog) {
	pauseEngine(true);
	int result = dialog.runModal();
	pauseEngine(false);

	return result;
}

void Engine::setGameToLoadSlot(int slot) {
	_saveSlotToLoad = slot;
}

void Engine::syncSoundSettings() {
	// Sync the engine with the config manager
	int soundVolumeMusic = ConfMan.getInt("music_volume");
	int soundVolumeSFX = ConfMan.getInt("sfx_volume");
	int soundVolumeSpeech = ConfMan.getInt("speech_volume");

	bool mute = false;
	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	// We need to handle the speech mute separately here. This is because the
	// engine code should be able to rely on all speech sounds muted when the
	// user specified subtitles only mode, which results in "speech_mute" to
	// be set to "true". The global mute setting has precedence over the
	// speech mute setting though.
	bool speechMute = mute;
	if (!speechMute)
		speechMute = ConfMan.getBool("speech_mute");

	_mixer->muteSoundType(Audio::Mixer::kPlainSoundType, mute);
	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, mute);
	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, mute);
	_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, speechMute);

	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, Audio::Mixer::kMaxMixerVolume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, soundVolumeMusic);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, soundVolumeSFX);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, soundVolumeSpeech);
}

void Engine::deinitKeymap() {
#ifdef ENABLE_KEYMAPPER
	_eventMan->getKeymapper()->cleanupGameKeymaps();
#endif
}

void Engine::flipMute() {
	// Mute will be set to true by default here. This has two reasons:
	// - if the game already has an "mute" config entry, it will be overwritten anyway.
	// - if it does not have a "mute" config entry, the sound is unmuted currently and should be muted now.
	bool mute = true;

	if (ConfMan.hasKey("mute")) {
		mute = !ConfMan.getBool("mute");
	}

	ConfMan.setBool("mute", mute);

	syncSoundSettings();
}

Common::Error Engine::loadGameState(int slot) {
	// Do nothing by default
	return Common::kNoError;
}

bool Engine::canLoadGameStateCurrently() {
	// Do not allow loading by default
	return false;
}

Common::Error Engine::saveGameState(int slot, const Common::String &desc) {
	// Do nothing by default
	return Common::kNoError;
}

bool Engine::canSaveGameStateCurrently() {
	// Do not allow saving by default
	return false;
}

void Engine::quitGame() {
	Common::Event event;

	event.type = Common::EVENT_QUIT;
	g_system->getEventManager()->pushEvent(event);
}

bool Engine::shouldQuit() {
	Common::EventManager *eventMan = g_system->getEventManager();
	return (eventMan->shouldQuit() || eventMan->shouldRTL());
}

/*
EnginePlugin *Engine::getMetaEnginePlugin() const {

	const EnginePlugin *plugin = 0;
	Common::String gameid = ConfMan.get("gameid");
	gameid.toLowercase();
	EngineMan.findGame(gameid, &plugin);
	return plugin;
}

*/
