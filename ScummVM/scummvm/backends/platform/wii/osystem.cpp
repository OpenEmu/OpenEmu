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
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <unistd.h>

#include <ogc/conf.h>
#include <ogc/mutex.h>
#include <ogc/lwp_watchdog.h>

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "backends/fs/wii/wii-fs-factory.h"
#include "backends/saves/default/default-saves.h"
#include "backends/timer/default/default-timer.h"

#include "osystem.h"
#include "options.h"

OSystem_Wii::OSystem_Wii() :
	_startup_time(0),

	_cursorDontScale(true),
	_cursorPaletteDisabled(true),
	_cursorPalette(NULL),
	_cursorPaletteDirty(false),

	_gameRunning(false),
	_gameWidth(0),
	_gameHeight(0),
	_gamePixels(NULL),
	_gameDirty(false),

	_overlayVisible(true),
	_overlayWidth(0),
	_overlayHeight(0),
	_overlaySize(0),
	_overlayPixels(NULL),
	_overlayDirty(false),

	_lastScreenUpdate(0),
	_currentWidth(0),
	_currentHeight(0),
	_currentXScale(1),
	_currentYScale(1),

	_configGraphicsMode(0),
	_actualGraphicsMode(0),
	_bilinearFilter(false),
#ifdef USE_RGB_COLOR
	_pfRGB565(Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)),
	_pfRGB3444(Graphics::PixelFormat(2, 4, 4, 4, 3, 8, 4, 0, 12)),
	_pfGame(Graphics::PixelFormat::createFormatCLUT8()),
	_pfGameTexture(Graphics::PixelFormat::createFormatCLUT8()),
	_pfCursor(Graphics::PixelFormat::createFormatCLUT8()),
#endif

	_optionsDlgActive(false),
	_consoleVisible(false),
	_fullscreen(false),
	_arCorrection(false),

	_mouseVisible(false),
	_mouseX(0),
	_mouseY(0),
	_mouseHotspotX(0),
	_mouseHotspotY(0),
	_mouseKeyColor(0),

	_kbd_active(false),

	_event_quit(false),

	_lastPadCheck(0),
	_padSensitivity(16),
	_padAcceleration(4),

	_mixer(NULL) {
}

OSystem_Wii::~OSystem_Wii() {
	delete _mixer;
	_mixer = NULL;
}

void OSystem_Wii::initBackend() {
	_startup_time = gettime();

	ConfMan.registerDefault("fullscreen", true);
	ConfMan.registerDefault("aspect_ratio", true);
	ConfMan.registerDefault("wii_video_default_underscan_x", 16);
	ConfMan.registerDefault("wii_video_default_underscan_y", 16);
	ConfMan.registerDefault("wii_video_ds_underscan_x", 16);
	ConfMan.registerDefault("wii_video_ds_underscan_y", 16);
	ConfMan.registerDefault("wii_pad_sensitivity", 48);
	ConfMan.registerDefault("wii_pad_acceleration", 5);
	ConfMan.registerDefault("wii_smb_server", "");
	ConfMan.registerDefault("wii_smb_share", "");
	ConfMan.registerDefault("wii_smb_username", "");
	ConfMan.registerDefault("wii_smb_password", "");

	WiiFilesystemFactory &fsf = WiiFilesystemFactory::instance();

#ifdef USE_WII_SMB
	fsf.setSMBLoginData(ConfMan.get("wii_smb_server"),
						ConfMan.get("wii_smb_share"),
						ConfMan.get("wii_smb_username"),
						ConfMan.get("wii_smb_password"));
#endif

	fsf.asyncInit();

	char buf[MAXPATHLEN];
	if (!getcwd(buf, MAXPATHLEN))
		strcpy(buf, "/");

	_savefileManager = new DefaultSaveFileManager(buf);
	_timerManager = new DefaultTimerManager();

	initGfx();
	initSfx();
	initEvents();

	EventsBaseBackend::initBackend();
}

void OSystem_Wii::quit() {
	deinitEvents();
	deinitSfx();
	deinitGfx();

	WiiFilesystemFactory::instance().asyncDeinit();
}

void OSystem_Wii::engineInit() {
	_gameRunning = true;
	WiiFilesystemFactory::instance().umountUnused(ConfMan.get("path"));
}

void OSystem_Wii::engineDone() {
	_gameRunning = false;
	switchVideoMode(gmStandard);
	gfx_set_ar(4.0 / 3.0);
}

bool OSystem_Wii::hasFeature(Feature f) {
	return (f == kFeatureFullscreenMode) ||
			(f == kFeatureAspectRatioCorrection) ||
			(f == kFeatureCursorPalette) ||
			(f == kFeatureOverlaySupportsAlpha);
}

void OSystem_Wii::setFeatureState(Feature f, bool enable) {
	switch (f) {
	case kFeatureFullscreenMode:
		_fullscreen = enable;
		gfx_set_pillarboxing(!enable);
		break;
	case kFeatureAspectRatioCorrection:
		_arCorrection = enable;
		break;
	case kFeatureCursorPalette:
		_cursorPaletteDisabled = !enable;
		if (_texMouse.palette && !enable) {
			memcpy(_texMouse.palette, _cursorPalette, 256 * 2);
			_cursorPaletteDirty = true;
		}
		break;
	default:
		break;
	}
}

bool OSystem_Wii::getFeatureState(Feature f) {
	switch (f) {
	case kFeatureFullscreenMode:
		return _fullscreen;
	case kFeatureAspectRatioCorrection:
		return _arCorrection;
	case kFeatureCursorPalette:
		return !_cursorPaletteDisabled;
	default:
		return false;
	}
}

uint32 OSystem_Wii::getMillis() {
	return ticks_to_millisecs(diff_ticks(_startup_time, gettime()));
}

void OSystem_Wii::delayMillis(uint msecs) {
	usleep(msecs * 1000);
}

OSystem::MutexRef OSystem_Wii::createMutex() {
	mutex_t *mutex = (mutex_t *) malloc(sizeof(mutex_t));
	s32 res = LWP_MutexInit(mutex, true);

	if (res) {
		printf("ERROR creating mutex\n");
		free(mutex);
		return NULL;
	}

	return (MutexRef) mutex;
}

void OSystem_Wii::lockMutex(MutexRef mutex) {
	s32 res = LWP_MutexLock(*(mutex_t *)mutex);

	if (res)
		printf("ERROR locking mutex %p (%d)\n", mutex, res);
}

void OSystem_Wii::unlockMutex(MutexRef mutex) {
	s32 res = LWP_MutexUnlock(*(mutex_t *)mutex);

	if (res)
		printf("ERROR unlocking mutex %p (%d)\n", mutex, res);
}

void OSystem_Wii::deleteMutex(MutexRef mutex) {
	s32 res = LWP_MutexDestroy(*(mutex_t *)mutex);

	if (res)
		printf("ERROR destroying mutex %p (%d)\n", mutex, res);

	free(mutex);
}

void OSystem_Wii::setWindowCaption(const char *caption) {
	printf("window caption: %s\n", caption);
}

Audio::Mixer *OSystem_Wii::getMixer() {
	assert(_mixer);
	return _mixer;
}

FilesystemFactory *OSystem_Wii::getFilesystemFactory() {
	return &WiiFilesystemFactory::instance();
}

void OSystem_Wii::getTimeAndDate(TimeDate &td) const {
	time_t curTime = time(0);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
	td.tm_wday = t.tm_wday;
}

void OSystem_Wii::showOptionsDialog() {
	if (_optionsDlgActive)
		return;

	bool ds = (_actualGraphicsMode == gmDoubleStrike) ||
				(_actualGraphicsMode == gmDoubleStrikeFiltered);

	_optionsDlgActive = true;
	WiiOptionsDialog dlg(ds);
	dlg.runModal();
	_optionsDlgActive = false;

	_padSensitivity = 64 - ConfMan.getInt("wii_pad_sensitivity");
	_padAcceleration = 9 - ConfMan.getInt("wii_pad_acceleration");
}

void OSystem_Wii::logMessage(LogMessageType::Type type, const char *message) {
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	fputs(message, output);
	fflush(output);
}

#ifndef GAMECUBE
Common::String OSystem_Wii::getSystemLanguage() const {
	const char *wiiCountries[] = {
		"JP", // CONF_AREA_JPN Japan
		"US", // CONF_AREA_USA United States of America
		"",   // CONF_AREA_EUR Europe?
		"AU", // CONF_AREA_AUS Australia, Commonwealth of
		"BR", // CONF_AREA_BRA Brazil, Federative Republic of
		"TW", // CONF_AREA_TWN Taiwan, Province of China
		"",   // CONF_AREA_ROC Republic of China (Taiwan)
		"KR", // CONF_AREA_KOR Korea, Republic of
		"HK", // CONF_AREA_HKG Hong Kong, Special Administrative Region of China
		"",   // CONF_AREA_ASI Asia?
		"",   // CONF_AREA_LTN Lithuania?
		"",   // CONF_AREA_SAF South-Africa?
		"CN"  // CONF_AREA_CHN China, People's Republic of
	};

	// Start by detecting the country, since we can deduce some languages not
	// supported on the Wii from it.
	Common::String country;
	// TODO: Can we get more fine-grained country setting?
	int32 areaID = CONF_GetArea();
	if ((areaID >= CONF_AREA_JPN) && (areaID <= CONF_AREA_CHN)) {
		// It's a known area.
		if (areaID == CONF_AREA_BRA) {
			// Portuguese isn't available on the Wii, but we know it's the
			// official language in Brazil, so we handle it separately.
			return "pt_BR";
		} else {
			// Let's use our manual area to country mapping.
			country = wiiCountries[areaID];
		}
	} else {
		// This will only happen when new areas are added to the API.
		warning("WII: Unknown system area: %d", areaID);
	}


	const char *wiiLanguages[] = {
		"ja", // CONF_LANG_JAPANESE     Japanese
		"en", // CONF_LANG_ENGLISH      English
		"de", // CONF_LANG_GERMAN       German
		"fr", // CONF_LANG_FRENCH       French
		"es", // CONF_LANG_SPANISH      Spanish
		"it", // CONF_LANG_ITALIAN      Italian
		"nl", // CONF_LANG_DUTCH        Dutch
		"zh", // CONF_LANG_SIMP_CHINESE Simplified Chinese
		"zh", // CONF_LANG_TRAD_CHINESE Traditional Chinese
		"ko"  // CONF_LANG_KOREAN       Korean
	};

	// Now let's read the system language.
	Common::String lang;
	int32 langID = CONF_GetLanguage();
	if ((langID >= CONF_LANG_JAPANESE) && (langID <= CONF_LANG_KOREAN)) {
		// It's a known language, let's use our manual language mapping.
		lang = wiiLanguages[langID];

		if (country.empty()) {
			// We don't know how to improve the detection,
			// let's return the language alone.
			return lang;
		} else {
			// Return the complete language_country string.
			return lang + "_" + country;
		}
	} else {
		// This will only happen when new languages are added to the API.
		warning("WII: Unknown system language: %d", langID);
		return EventsBaseBackend::getSystemLanguage();
	}
}
#endif // !GAMECUBE
