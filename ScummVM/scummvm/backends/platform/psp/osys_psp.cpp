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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <pspuser.h>
#include <pspgu.h>
#include <pspdisplay.h>

#include <time.h>
#include <zlib.h>

#include "common/config-manager.h"
#include "common/events.h"
#include "common/scummsys.h"

#include "backends/platform/psp/psppixelformat.h"
#include "backends/platform/psp/osys_psp.h"
#include "backends/platform/psp/powerman.h"
#include "backends/platform/psp/rtc.h"

#include "backends/saves/psp/psp-saves.h"
#include "backends/timer/default/default-timer.h"
#include "graphics/surface.h"
#include "audio/mixer_intern.h"

//#define __PSP_DEBUG_FUNCS__	/* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */

#include "backends/platform/psp/trace.h"

#define	SAMPLES_PER_SEC	44100

static int timer_handler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}

OSystem_PSP::~OSystem_PSP() {}

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272

void OSystem_PSP::initBackend() {
	DEBUG_ENTER_FUNC();

	// Instantiate real time clock
	PspRtc::instance();

	_cursor.enableCursorPalette(false);
	_cursor.setXY(PSP_SCREEN_WIDTH >> 1, PSP_SCREEN_HEIGHT >> 1);	// Mouse in the middle of the screen

	// Set pointers for display manager
	_displayManager.setCursor(&_cursor);
	_displayManager.setScreen(&_screen);
	_displayManager.setOverlay(&_overlay);
	_displayManager.setKeyboard(&_keyboard);
	_displayManager.setImageViewer(&_imageViewer);
	_displayManager.init();

	// Set pointers for input handler
	_inputHandler.setCursor(&_cursor);
	_inputHandler.setKeyboard(&_keyboard);
	_inputHandler.setImageViewer(&_imageViewer);
	_inputHandler.init();

	// Set pointers for image viewer
	_imageViewer.setInputHandler(&_inputHandler);
	_imageViewer.setDisplayManager(&_displayManager);

	_savefileManager = new PSPSaveFileManager;

	_timerManager = new DefaultTimerManager();

	PSP_DEBUG_PRINT("calling keyboard.load()\n");
	_keyboard.load();	// Load virtual keyboard files into memory

	setTimerCallback(&timer_handler, 10);

	setupMixer();

	EventsBaseBackend::initBackend();
}

// Let's us know an engine
void OSystem_PSP::engineDone() {
	// for now, all we need is to reset the image number on the viewer
	_imageViewer.resetOnEngineDone();
}

bool OSystem_PSP::hasFeature(Feature f) {
	return (f == kFeatureOverlaySupportsAlpha || f == kFeatureCursorPalette);
}

void OSystem_PSP::setFeatureState(Feature f, bool enable) {
	if (f == kFeatureCursorPalette) {
		_pendingUpdate = false;
		_cursor.enableCursorPalette(enable);
	}
}

bool OSystem_PSP::getFeatureState(Feature f) {
	if (f == kFeatureCursorPalette) {
		return _cursor.isCursorPaletteEnabled();
	}
	return false;
}

const OSystem::GraphicsMode* OSystem_PSP::getSupportedGraphicsModes() const {
	return _displayManager.getSupportedGraphicsModes();
}

int OSystem_PSP::getDefaultGraphicsMode() const {
	DEBUG_ENTER_FUNC();
	return _displayManager.getDefaultGraphicsMode();
}

bool OSystem_PSP::setGraphicsMode(int mode) {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;
	return _displayManager.setGraphicsMode(mode);
}

bool OSystem_PSP::setGraphicsMode(const char *name) {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;
	return _displayManager.setGraphicsMode(name);
}

int OSystem_PSP::getGraphicsMode() const {
	DEBUG_ENTER_FUNC();
	return _displayManager.getGraphicsMode();
}

#ifdef USE_RGB_COLOR

Graphics::PixelFormat OSystem_PSP::getScreenFormat() const {
	return _screen.getScummvmPixelFormat();
}

Common::List<Graphics::PixelFormat> OSystem_PSP::getSupportedFormats() const {
	return _displayManager.getSupportedPixelFormats();
}

#endif

void OSystem_PSP::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;
	_displayManager.setSizeAndPixelFormat(width, height, format);

	_cursor.setVisible(false);
	_cursor.setLimits(_screen.getWidth(), _screen.getHeight());
}

int16 OSystem_PSP::getWidth() {
	DEBUG_ENTER_FUNC();
	return (int16)_screen.getWidth();
}

int16 OSystem_PSP::getHeight() {
	DEBUG_ENTER_FUNC();
	return (int16)_screen.getHeight();
}

void OSystem_PSP::setPalette(const byte *colors, uint start, uint num) {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;
	_screen.setPartialPalette(colors, start, num);
	_cursor.setScreenPalette(colors, start, num);
	_cursor.clearKeyColor();
}

void OSystem_PSP::setCursorPalette(const byte *colors, uint start, uint num) {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;
	_cursor.setCursorPalette(colors, start, num);
	_cursor.enableCursorPalette(true);
	_cursor.clearKeyColor();	// Do we need this?
}

void OSystem_PSP::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;
	_screen.copyFromRect((const byte *)buf, pitch, x, y, w, h);
}

Graphics::Surface *OSystem_PSP::lockScreen() {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;
	return _screen.lockAndGetForEditing();
}

void OSystem_PSP::unlockScreen() {
	DEBUG_ENTER_FUNC();
	_pendingUpdate = false;
	// The screen is always completely updated anyway, so we don't have to force a full update here.
	_screen.unlock();
}

void OSystem_PSP::updateScreen() {
	DEBUG_ENTER_FUNC();
	_pendingUpdate = !_displayManager.renderAll();	// if we didn't update, we have a pending update
}

void OSystem_PSP::setShakePos(int shakeOffset) {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;
	_screen.setShakePos(shakeOffset);
}

void OSystem_PSP::showOverlay() {
	DEBUG_ENTER_FUNC();
	_pendingUpdate = false;
	_overlay.setVisible(true);
	_cursor.setLimits(_overlay.getWidth(), _overlay.getHeight());
	_cursor.useGlobalScaler(false);	// mouse with overlay is 1:1
}

void OSystem_PSP::hideOverlay() {
	DEBUG_ENTER_FUNC();
	_pendingUpdate = false;
	_overlay.setVisible(false);
	_cursor.setLimits(_screen.getWidth(), _screen.getHeight());
	_cursor.useGlobalScaler(true);	// mouse needs to be scaled with screen
}

void OSystem_PSP::clearOverlay() {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;
	_overlay.clearBuffer();
}

void OSystem_PSP::grabOverlay(void *buf, int pitch) {
	DEBUG_ENTER_FUNC();
	_overlay.copyToArray(buf, pitch);
}

void OSystem_PSP::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;
	_overlay.copyFromRect(buf, pitch, x, y, w, h);
}

int16 OSystem_PSP::getOverlayWidth() {
	return (int16)_overlay.getWidth();
}

int16 OSystem_PSP::getOverlayHeight() {
	return (int16)_overlay.getHeight();
}

void OSystem_PSP::grabPalette(byte *colors, uint start, uint num) {
	DEBUG_ENTER_FUNC();
	_screen.getPartialPalette(colors, start, num);
}

bool OSystem_PSP::showMouse(bool v) {
	DEBUG_ENTER_FUNC();
	_pendingUpdate = false;

	PSP_DEBUG_PRINT("%s\n", v ? "true" : "false");
	bool last = _cursor.isVisible();
	_cursor.setVisible(v);

	return last;
}

void OSystem_PSP::warpMouse(int x, int y) {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;
	_cursor.setXY(x, y);
}

void OSystem_PSP::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	DEBUG_ENTER_FUNC();
	_displayManager.waitUntilRenderFinished();
	_pendingUpdate = false;

	PSP_DEBUG_PRINT("pbuf[%p], w[%u], h[%u], hotspot:X[%d], Y[%d], keycolor[%d], scale[%d], pformat[%p]\n", buf, w, h, hotspotX, hotspotY, keycolor, cursorTargetScale, format);
	if (format) {
		PSP_DEBUG_PRINT("format: bpp[%d], rLoss[%d], gLoss[%d], bLoss[%d], aLoss[%d], rShift[%d], gShift[%d], bShift[%d], aShift[%d]\n", format->bytesPerPixel, format->rLoss, format->gLoss, format->bLoss, format->aLoss, format->rShift, format->gShift, format->bShift, format->aShift);
	}

	_cursor.setKeyColor(keycolor);
	// TODO: The old target scale was saved but never used. Should the new
	// "do not scale" logic be implemented?
	//_cursor.setCursorTargetScale(cursorTargetScale);
	_cursor.setSizeAndScummvmPixelFormat(w, h, format);
	_cursor.setHotspot(hotspotX, hotspotY);
	_cursor.clearKeyColor();
	_cursor.copyFromArray((const byte *)buf);
}

bool OSystem_PSP::pollEvent(Common::Event &event) {

	// If we're polling for events, we should check for pausing the engine
	// Pausing the engine is a necessary fix for games that use the timer for music synchronization
	// 	recovering many hours later causes the game to crash. We're polling without mutexes since it's not critical to
	//  get it right now.
	PowerMan.pollPauseEngine();

	// A hack:
	// Check if we have a pending update that we missed for some reason (FPS throttling for example)
	// Time between event polls is usually 5-10ms, so waiting for 4 calls before checking to update the screen should be fine
	if (_pendingUpdate) {
		_pendingUpdateCounter++;

		if (_pendingUpdateCounter >= 4) {
			PSP_DEBUG_PRINT("servicing pending update\n");
			updateScreen();
			if (!_pendingUpdate) 	// we handled the update
				_pendingUpdateCounter = 0;
		}
	} else
		_pendingUpdateCounter = 0;	// reset the counter, no pending

	return _inputHandler.getAllInputs(event);
}

uint32 OSystem_PSP::getMillis() {
	return PspRtc::instance().getMillis();
}

void OSystem_PSP::delayMillis(uint msecs) {
	PspThread::delayMillis(msecs);
}

void OSystem_PSP::setTimerCallback(TimerProc callback, int interval) {
	_pspTimer.setCallback((PspTimer::CallbackFunc)callback);
	_pspTimer.setIntervalMs(interval);
	_pspTimer.start();
}

OSystem::MutexRef OSystem_PSP::createMutex(void) {
	return (MutexRef) new PspMutex(true);	// start with a full mutex
}

void OSystem_PSP::lockMutex(MutexRef mutex) {
	((PspMutex *)mutex)->lock();
}

void OSystem_PSP::unlockMutex(MutexRef mutex) {
	((PspMutex *)mutex)->unlock();
}

void OSystem_PSP::deleteMutex(MutexRef mutex) {
	delete (PspMutex *)mutex;
}

void OSystem_PSP::mixCallback(void *sys, byte *samples, int len) {
	OSystem_PSP *this_ = (OSystem_PSP *)sys;
	assert(this_);

	if (this_->_mixer)
		this_->_mixer->mixCallback(samples, len);
}

void OSystem_PSP::setupMixer(void) {

	// Determine the desired output sampling frequency.
	uint32 samplesPerSec = 0;
	if (ConfMan.hasKey("output_rate"))
		samplesPerSec = ConfMan.getInt("output_rate");
	if (samplesPerSec <= 0)
		samplesPerSec = SAMPLES_PER_SEC;

	// Determine the sample buffer size. We want it to store enough data for
	// at least 1/16th of a second (though at most 8192 samples). Note
	// that it must be a power of two. So e.g. at 22050 Hz, we request a
	// sample buffer size of 2048.
	uint32 samples = 8192;
	while (samples * 16 > samplesPerSec * 2)
		samples >>= 1;

	assert(!_mixer);

	if (!_audio.open(samplesPerSec, 2, samples, mixCallback, this)) {
		PSP_ERROR("failed to open audio\n");
		return;
	}
	samplesPerSec = _audio.getFrequency();	// may have been changed by audio system
	_mixer = new Audio::MixerImpl(this, samplesPerSec);
	assert(_mixer);
	_mixer->setReady(true);
	_audio.unpause();
}

void OSystem_PSP::quit() {
	_audio.close();
	sceKernelExitGame();
}

void OSystem_PSP::logMessage(LogMessageType::Type type, const char *message) {
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	fputs(message, output);
	fflush(output);

	if (type == LogMessageType::kError)
		PspDebugTrace(false, "%s", message);	// write to file
}

void OSystem_PSP::getTimeAndDate(TimeDate &td) const {
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

Common::String OSystem_PSP::getDefaultConfigFileName() {
	return "ms0:/scummvm.ini";
}
