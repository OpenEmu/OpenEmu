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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

#include "common/scummsys.h"
#include "common/system.h"

#include "common/util.h"
#include "common/rect.h"
#include "common/savefile.h"

#include "osystem_ds.h"
#include "nds.h"
#include "dsmain.h"
#include "nds/registers_alt.h"
#include "common/config-manager.h"
#include "common/str.h"
#include "cdaudio.h"
#include "graphics/surface.h"
#include "touchkeyboard.h"
#include "backends/fs/ds/ds-fs-factory.h"

#include "backends/audiocd/default/default-audiocd.h"
#include "backends/timer/default/default-timer.h"

#ifdef ENABLE_AGI
#include "wordcompletion.h"
#endif

#include <time.h>

#if defined(DS_BUILD_A)
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#elif defined(DS_BUILD_B)
#define DEFAULT_CONFIG_FILE "scummvmb.ini"
#elif defined(DS_BUILD_C)
#define DEFAULT_CONFIG_FILE "scummvmc.ini"
#elif defined(DS_BUILD_D)
#define DEFAULT_CONFIG_FILE "scummvmd.ini"
#elif defined(DS_BUILD_E)
#define DEFAULT_CONFIG_FILE "scummvme.ini"
#elif defined(DS_BUILD_F)
#define DEFAULT_CONFIG_FILE "scummvmf.ini"
#elif defined(DS_BUILD_G)
#define DEFAULT_CONFIG_FILE "scummvmg.ini"
#elif defined(DS_BUILD_H)
#define DEFAULT_CONFIG_FILE "scummvmh.ini"
#elif defined(DS_BUILD_I)
#define DEFAULT_CONFIG_FILE "scummvmi.ini"
#elif defined(DS_BUILD_J)
#define DEFAULT_CONFIG_FILE "scummvmj.ini"
#elif defined(DS_BUILD_K)
#define DEFAULT_CONFIG_FILE "scummvmk.ini"
#else
	// Use the "scummvm.ini" as config file if no build was specified. This
	// currently only happens with builds made using the regular ScummVM build
	// system (as opposed to the nds specific build system).
#define DEFAULT_CONFIG_FILE "scummvm.ini"
#endif

OSystem_DS *OSystem_DS::_instance = NULL;

OSystem_DS::OSystem_DS()
	: eventNum(0), lastPenFrame(0), queuePos(0), _mixer(NULL), _frameBufferExists(false),
	_disableCursorPalette(true), _graphicsEnable(true), _gammaValue(0)
{
//	eventNum = 0;
//	lastPenFrame = 0;
//	queuePos = 0;
	_instance = this;
//	_mixer = NULL;
	//_frameBufferExists = false;
}

OSystem_DS::~OSystem_DS() {
	delete _mixer;
	_mixer = 0;

	// If _savefileManager is not 0, then it points to the OSystem_DS
	// member variable mpSaveManager. Hence we set _savefileManager to
	// 0, to prevent the OSystem destructor from trying to delete it.
	_savefileManager = 0;
}

int OSystem_DS::timerHandler(int t) {
	DefaultTimerManager *tm = (DefaultTimerManager *)g_system->getTimerManager();
	tm->handler();
	return t;
}

void OSystem_DS::initBackend() {
	ConfMan.setInt("autosave_period", 0);
	ConfMan.setBool("FM_medium_quality", true);

	if (DS::isGBAMPAvailable()) {
		_savefileManager = &mpSaveManager;
	}

	_timerManager = new DefaultTimerManager();
    DS::setTimerCallback(&OSystem_DS::timerHandler, 10);

	if (ConfMan.hasKey("22khzaudio", "ds") && ConfMan.getBool("22khzaudio", "ds")) {
		DS::startSound(22050, 8192);
	} else {
		DS::startSound(11025, 4096);
	}

	_mixer = new Audio::MixerImpl(this, DS::getSoundFrequency());
	_mixer->setReady(true);

	/* TODO/FIXME: The NDS should use a custom AudioCD manager instance!
	if (!_audiocdManager)
		_audiocdManager = new DSAudioCDManager();
	*/

	EventsBaseBackend::initBackend();
}

bool OSystem_DS::hasFeature(Feature f) {
	return (f == kFeatureVirtualKeyboard) || (f == kFeatureCursorPalette);
}

void OSystem_DS::setFeatureState(Feature f, bool enable) {
	if (f == kFeatureVirtualKeyboard)
		DS::setKeyboardIcon(enable);
	else if (f == kFeatureCursorPalette) {
		_disableCursorPalette = !enable;
		refreshCursor();
	}
}

bool OSystem_DS::getFeatureState(Feature f) {
	if (f == kFeatureVirtualKeyboard)
		return DS::getKeyboardIcon();
	if (f == kFeatureCursorPalette)
		return !_disableCursorPalette;
	return false;
}

const OSystem::GraphicsMode *OSystem_DS::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}


int OSystem_DS::getDefaultGraphicsMode() const {
	return 0;
}

bool OSystem_DS::setGraphicsMode(int mode) {
	return true;
}

bool OSystem_DS::setGraphicsMode(const char *name) {
	consolePrintf("Set gfx mode %s\n", name);
	return true;
}

int OSystem_DS::getGraphicsMode() const {
	return -1;
}

void OSystem_DS::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	// For Lost in Time, the title screen is displayed in 640x400.
	// In order to support this game, the screen mode is set, but
	// all draw calls are ignored until the game switches to 320x200.
	if ((width == 640) && (height == 400)) {
		_graphicsEnable = false;
	} else {
		_graphicsEnable = true;
		DS::setGameSize(width, height);
	}
}

int16 OSystem_DS::getHeight() {
	return 200;
}

int16 OSystem_DS::getWidth() {
	return 320;
}

void OSystem_DS::setPalette(const byte *colors, uint start, uint num) {
//	consolePrintf("Setpal %d, %d\n", start, num);

	for (unsigned int r = start; r < start + num; r++) {
		int red = *colors;
		int green = *(colors + 1);
		int blue = *(colors + 2);

		red >>= 3;
		green >>= 3;
		blue >>= 3;

//		if (r != 255)
		{
			u16 paletteValue = red | (green << 5) | (blue << 10);

			if (DS::getIsDisplayMode8Bit()) {
				int col = applyGamma(paletteValue);
				BG_PALETTE[r] = col;

				if (!DS::getKeyboardEnable()) {
					BG_PALETTE_SUB[r] = col;
				}
			}

			_palette[r] = paletteValue;
		}
	//	if (num == 255) consolePrintf("pal:%d r:%d g:%d b:%d\n", r, red, green, blue);

		colors += 3;
	}
}

void OSystem_DS::restoreHardwarePalette() {
	// Set the hardware palette up based on the stored palette

	for (int r = 0; r < 255; r++) {
		int col = applyGamma(_palette[r]);
		BG_PALETTE[r] = col;
		if (!DS::getKeyboardEnable()) {
			BG_PALETTE_SUB[r] = col;
		}
	}
}

void OSystem_DS::setCursorPalette(const byte *colors, uint start, uint num) {

//	consolePrintf("Cursor palette set: start: %d, cols: %d\n", start, num);
	for (unsigned int r = start; r < start + num; r++) {
		int red = *colors;
		int green = *(colors + 1);
		int blue = *(colors + 2);

		red >>= 3;
		green >>= 3;
		blue >>= 3;

		u16 paletteValue = red | (green << 5) | (blue << 10);
		_cursorPalette[r] = paletteValue;

		colors += 3;
	}

	_disableCursorPalette = false;
	refreshCursor();
}

void OSystem_DS::grabPalette(unsigned char *colors, uint start, uint num) {
//	consolePrintf("Grabpalette");

	for (unsigned int r = start; r < start + num; r++) {
		*colors++ = (BG_PALETTE[r] & 0x001F) << 3;
		*colors++ = (BG_PALETTE[r] & 0x03E0) >> 5 << 3;
		*colors++ = (BG_PALETTE[r] & 0x7C00) >> 10 << 3;
	}
}


#define MISALIGNED16(ptr) (((u32) (ptr) & 1) != 0)

void OSystem_DS::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	if (!_graphicsEnable) return;
	if (w <= 1) return;
	if (h < 0) return;
	if (!DS::getIsDisplayMode8Bit()) return;

//	consolePrintf("CopyRectToScreen %d\n", w * h);

	u16 *bg;
	s32 stride;
	u16 *bgSub = (u16 *)BG_GFX_SUB;

	// The DS video RAM doesn't support 8-bit writes because Nintendo wanted
	// to save a few pennies/euro cents on the hardware.

	if (_frameBufferExists) {
		bg = (u16 *)_framebuffer.pixels;
		stride = _framebuffer.pitch;
	} else {
		bg = (u16 *)DS::get8BitBackBuffer();
		stride = DS::get8BitBackBufferStride();
	}


	if (((pitch & 1) != 0) || ((w & 1) != 0) || (((int) (buf) & 1) != 0)) {

		// Something is misaligned, so we have to use the slow but sure method

		int by = 0;

		if (DS::getKeyboardEnable()) {
			// When they keyboard is on screen, we don't update the subscreen because
			// the keyboard image uses the same VRAM addresses.

			for (int dy = y; dy < y + h; dy++) {
				u8 *dest = ((u8 *) (bg)) + (dy * stride) + x;
				const u8 *src = (const u8 *) buf + (pitch * by);

				u32 dx;

				u32 pixelsLeft = w;

				if (MISALIGNED16(dest)) {
					// Read modify write

					dest--;
					u16 mix = *((u16 *) dest);

					mix = (mix & 0x00FF) | (*src++ << 8);

					*dest = mix;

					dest += 2;
					pixelsLeft--;
				}

				// We can now assume dest is aligned
				u16 *dest16 = (u16 *) dest;

				for (dx = 0; dx < pixelsLeft; dx+=2) {
					u16 mix;

					mix = *src + (*(src + 1) << 8);
					*dest16++ = mix;
					src += 2;
				}

				pixelsLeft -= dx;

				// At the end we may have one pixel left over

				if (pixelsLeft != 0) {
					u16 mix = *dest16;

					mix = (mix & 0x00FF) | ((*src++) << 8);

					*dest16 = mix;
				}

				by++;
			}

		} else {
			// When they keyboard is not on screen, update both vram copies

			for (int dy = y; dy < y + h; dy++) {
				u8 *dest = ((u8 *) (bg)) + (dy * stride) + x;
				u8 *destSub = ((u8 *) (bgSub)) + (dy * 512) + x;
				const u8 *src = (const u8 *) buf + (pitch * by);

				u32 dx;

				u32 pixelsLeft = w;

				if (MISALIGNED16(dest)) {
					// Read modify write

					dest--;
					u16 mix = *((u16 *) dest);

					mix = (mix & 0x00FF) | (*src++ << 8);

					*dest = mix;
					*destSub = mix;

					dest += 2;
					destSub += 2;
					pixelsLeft--;
				}

				// We can now assume dest is aligned
				u16 *dest16 = (u16 *) dest;
				u16 *destSub16 = (u16 *) destSub;

				for (dx = 0; dx < pixelsLeft; dx+=2) {
					u16 mix;

					mix = *src + (*(src + 1) << 8);
					*dest16++ = mix;
					*destSub16++ = mix;
					src += 2;
				}

				pixelsLeft -= dx;

				// At the end we may have one pixel left over

				if (pixelsLeft != 0) {
					u16 mix = *dest16;

					mix = (mix & 0x00FF) | ((*src++) << 8);

					*dest16 = mix;
					*destSub16 = mix;
				}

				by++;

			}
		}

//		consolePrintf("Slow method used!\n");


	} else {

		// Stuff is aligned to 16-bit boundaries, so it's safe to do DMA.

		u16 *src = (u16 *) buf;

		if (DS::getKeyboardEnable()) {

			for (int dy = y; dy < y + h; dy++) {
				u16 *dest = bg + (dy * (stride >> 1)) + (x >> 1);

				DC_FlushRange(src, w << 1);
				DC_FlushRange(dest, w << 1);
				dmaCopyHalfWords(3, src, dest, w);

				while (dmaBusy(3));

				src += pitch >> 1;
			}

		} else {
			for (int dy = y; dy < y + h; dy++) {
				u16 *dest1 = bg + (dy * (stride >> 1)) + (x >> 1);
				u16 *dest2 = bgSub + (dy << 8) + (x >> 1);

				DC_FlushRange(src, w << 1);
				DC_FlushRange(dest1, w << 1);
				DC_FlushRange(dest2, w << 1);

				dmaCopyHalfWords(3, src, dest1, w);

				if ((!_frameBufferExists) || (buf == _framebuffer.pixels)) {
					dmaCopyHalfWords(2, src, dest2, w);
				}

				while (dmaBusy(2) || dmaBusy(3));

				src += pitch >> 1;
			}
		}
	}
//	consolePrintf("Done\n");
}

void OSystem_DS::updateScreen() {
//	static int cnt = 0;
//	consolePrintf("updatescr %d\n", cnt++);

	if ((_frameBufferExists) && (DS::getIsDisplayMode8Bit())) {
		_frameBufferExists = false;

		// Copy temp framebuffer back to screen
		copyRectToScreen((byte *)_framebuffer.pixels, _framebuffer.pitch, 0, 0, _framebuffer.w, _framebuffer.h);
	}

	DS::displayMode16BitFlipBuffer();
	DS::doSoundCallback();
//	DS::doTimerCallback();
	DS::addEventsToQueue();

	// FIXME: Evil game specific hack.
	// Force back buffer usage for Nippon Safes, as it doesn't double buffer it's output
	if (DS::getControlType() == DS::CONT_NIPPON) {
		lockScreen();
	}
}

void OSystem_DS::setShakePos(int shakeOffset) {
	DS::setShakePos(shakeOffset);
}

void OSystem_DS::showOverlay() {
//	consolePrintf("showovl\n");
	DS::displayMode16Bit();
}

void OSystem_DS::hideOverlay() {
	DS::displayMode8Bit();
}

void OSystem_DS::clearOverlay() {
	memset((u16 *) DS::get16BitBackBuffer(), 0, 512 * 256 * 2);
//	consolePrintf("clearovl\n");
}

void OSystem_DS::grabOverlay(void *buf, int pitch) {
//	consolePrintf("grabovl\n")
	u16 *start = DS::get16BitBackBuffer();

	for (int y = 0; y < 200; y++) {
		u16 *src = start + (y * 320);
		u16 *dest = (u16 *)((u8 *)buf + (y * pitch));

		for (int x = 0; x < 320; x++) {
			*dest++ =  *src++;
		}
	}

}

void OSystem_DS::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	u16 *bg = (u16 *) DS::get16BitBackBuffer();
	const u8 *source = (const u8 *)buf;

//	if (x + w > 256) w = 256 - x;
	//if (x + h > 256) h = 256 - y;

//	consolePrintf("Copy rect ovl %d, %d   %d, %d  %d\n", x, y, w, h, pitch);



	for (int dy = y; dy < y + h; dy++) {
		const u16 *src = (const u16 *)source;

		// Slow but save copy:
		for (int dx = x; dx < x + w; dx++) {

			*(bg + (dy * 512) + dx) = *src;
			//if ((*src) != 0) consolePrintf("%d,%d: %d   ", dx, dy, *src);
			//consolePrintf("%d,", *src);
			src++;
		}
		source += pitch;

		// Fast but broken copy: (why?)
		/*
		REG_IME = 0;
		dmaCopy(src, bg + (dy << 9) + x, w * 2);
		REG_IME = 1;

		src += pitch;*/
	}

//	consolePrintf("Copy rect ovl done");

}

int16 OSystem_DS::getOverlayHeight() {
//	consolePrintf("getovlheight\n");
	return getHeight();
}

int16 OSystem_DS::getOverlayWidth() {
//	consolePrintf("getovlwid\n");
	return getWidth();
}


bool OSystem_DS::showMouse(bool visible) {
	DS::setShowCursor(visible);
	return true;
}

void OSystem_DS::warpMouse(int x, int y) {
}

void OSystem_DS::setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, u32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	if ((w > 0) && (w < 64) && (h > 0) && (h < 64)) {
		memcpy(_cursorImage, buf, w * h);
		_cursorW = w;
		_cursorH = h;
		_cursorHotX = hotspotX;
		_cursorHotY = hotspotY;
		_cursorKey = keycolor;
		// TODO: The old target scales was saved, but never used. Should the
		// new "do not scale" logic be implemented?
		//_cursorScale = targetCursorScale;
		refreshCursor();
	}
}

void OSystem_DS::refreshCursor() {
	DS::setCursorIcon(_cursorImage, _cursorW, _cursorH, _cursorKey, _cursorHotX, _cursorHotY);
}

void OSystem_DS::addEvent(const Common::Event& e) {
	eventQueue[queuePos++] = e;
}

bool OSystem_DS::pollEvent(Common::Event &event) {

	if (lastPenFrame != DS::getMillis()) {

		if (eventNum == queuePos) {
			eventNum = 0;
			queuePos = 0;
			// Bodge - this last event seems to be processed sometimes and not others.
			// So we make it something harmless which won't cause any adverse effects.
			event.type = Common::EVENT_KEYUP;
			event.kbd.ascii = 0;
			event.kbd.keycode = Common::KEYCODE_INVALID;
			event.kbd.flags = 0;
//			consolePrintf("type: %d\n", event.type);
			return false;
		} else {
			event = eventQueue[eventNum++];
//			consolePrintf("type: %d\n", event.type);
			return true;
		}
	}

	return false;

/*	if (lastPenFrame != DS::getMillis()) {
		if ((eventNum == 0)) {
			event.type = Common::EVENT_MOUSEMOVE;
			event.mouse = Common::Point(DS::getPenX(), DS::getPenY());
			eventNum = 1;
			return true;
		}
		if (eventNum == 1) {
			eventNum = 0;
			lastPenFrame = DS::getMillis();
			if (DS::getPenDown()) {
				event.type = Common::EVENT_LBUTTONDOWN;
				event.mouse = Common::Point(DS::getPenX(), DS::getPenY());
				consolePrintf("Down %d, %d  ", event.mouse.x, event.mouse.y);
				return true;
			} else if (DS::getPenReleased()) {
				event.type = Common::EVENT_LBUTTONUP;
				event.mouse = Common::Point(DS::getPenX(), DS::getPenY());
				consolePrintf("Up %d, %d ", event.mouse.x, event.mouse.y);
				return true;
			} else {
				return false;
			}
		}
	}*/

	return false;
}

uint32 OSystem_DS::getMillis() {
	return DS::getMillis();
}

void OSystem_DS::delayMillis(uint msecs) {
	int st = getMillis();
	DS::addEventsToQueue();
	DS::CD::update();

	DS::doSoundCallback();
	while (st + msecs >= getMillis()) {
		DS::doSoundCallback();
	}

	DS::doTimerCallback();
	DS::checkSleepMode();
	DS::addEventsToQueue();
}


void OSystem_DS::getTimeAndDate(TimeDate &td) const {
	time_t curTime;
#if 0
	curTime = time(0);
#else
	curTime = 0xABCD1234 + DS::getMillis() / 1000;
#endif
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
	td.tm_wday = t.tm_wday;
}

FilesystemFactory *OSystem_DS::getFilesystemFactory() {
	return &DSFilesystemFactory::instance();
}

OSystem::MutexRef OSystem_DS::createMutex(void) {
	return NULL;
}

void OSystem_DS::lockMutex(MutexRef mutex) {
}

void OSystem_DS::unlockMutex(MutexRef mutex) {
}

void OSystem_DS::deleteMutex(MutexRef mutex) {
}

// FIXME/TODO: The CD API as follows is *obsolete*
// and should be replaced by an AudioCDManager subclass,
// see backends/audiocd/ and common/system.h

bool OSystem_DS::openCD(int drive) {
	return DS::CD::checkCD();
}

bool OSystem_DS::pollCD() {
	return DS::CD::isPlaying();
}

void OSystem_DS::playCD(int track, int num_loops, int start_frame, int duration) {
	DS::CD::playTrack(track, num_loops, start_frame, duration);
}

void OSystem_DS::stopCD() {
	DS::CD::stopTrack();
}

void OSystem_DS::updateCD() {
}

void OSystem_DS::quit() {
/*	consolePrintf("Soft resetting...");
	IPC->reset = 1;
	REG_IE = 0;

	asm("swi 0x26\n");
	swiSoftReset();*/
}

Graphics::Surface *OSystem_DS::createTempFrameBuffer() {

	// Ensure we copy using 16 bit quantities due to limitation of VRAM addressing

	// If the scaler is enabled, we can just return the 8 bit back buffer,
	// since it's in system memory anyway.  Otherwise, we need to copy the back
	// buffer into the memory normally used by the scaler buffer and then
	// return it.
	// We also must ensure that once the frame buffer is created, future calls
	// to copyRectToScreen() copy to this buffer.

	if (DS::isCpuScalerEnabled()) {

		_framebuffer.pixels = DS::getScalerBuffer();
		_framebuffer.w = DS::getGameWidth();
		_framebuffer.h = DS::getGameHeight();
		_framebuffer.pitch = DS::getGameWidth();
		_framebuffer.format = Graphics::PixelFormat::createFormatCLUT8();

	} else {

		s32 height = DS::getGameHeight();
		s32 width = DS::getGameWidth();
		s32 stride = DS::get8BitBackBufferStride();

		u16 *src = DS::get8BitBackBuffer();
		u16 *dest = DS::getScalerBuffer();

		for (int y = 0; y < height; y++) {

			u16 *destLine = dest + (y * (width / 2));
			u16 *srcLine = src + (y * (stride / 2));

			DC_FlushRange(srcLine, width);

			dmaCopyHalfWords(3, srcLine, destLine, width);
		}

		_framebuffer.pixels = dest;
		_framebuffer.w = width;
		_framebuffer.h = height;
		_framebuffer.pitch = width;
		_framebuffer.format = Graphics::PixelFormat::createFormatCLUT8();

	}

	_frameBufferExists = true;

/*
	size_t imageStrideInBytes = DS::get8BitBackBufferStride();
	size_t imageStrideInWords = imageStrideInBytes / 2;

	u16 *image = (u16 *) DS::get8BitBackBuffer();
	for (int y = 0; y <  DS::getGameHeight(); y++) {
		DC_FlushRange(image + (y * imageStrideInWords), DS::getGameWidth());
		for (int x = 0; x < DS::getGameWidth() >> 1; x++) {
			*(((u16 *) (_framebuffer.pixels)) + y * (DS::getGameWidth() >> 1) + x) = image[(y * imageStrideInWords) + x];
//			*(((u16 *) (surf->pixels)) + y * (DS::getGameWidth() >> 1) + x) = image[y * imageStrideInWords + x];
		}
	}*/

	return &_framebuffer;
}


Graphics::Surface *OSystem_DS::lockScreen() {
	if (!_frameBufferExists) {
		createTempFrameBuffer();
	}

	return &_framebuffer;
}

void OSystem_DS::unlockScreen() {
	// No need to do anything here.  The screen will be updated in updateScreen().
}

void OSystem_DS::setFocusRectangle(const Common::Rect& rect) {
	DS::setTalkPos(rect.left + rect.width() / 2, rect.top + rect.height() / 2);
}

void OSystem_DS::clearFocusRectangle() {

}


void OSystem_DS::addAutoComplete(const char *word) {
	DS::addAutoComplete(word);
}

void OSystem_DS::clearAutoComplete() {
	DS::clearAutoComplete();
}

void OSystem_DS::setCharactersEntered(int count) {
	DS::setCharactersEntered(count);
}

Common::String OSystem_DS::getDefaultConfigFileName() {
	return DEFAULT_CONFIG_FILE;
}

void OSystem_DS::logMessage(LogMessageType::Type type, const char *message) {
#ifndef DISABLE_TEXT_CONSOLE
	nocashMessage((char *)message);
//	consolePrintf((char *)message);
#endif
}

u16 OSystem_DS::applyGamma(u16 color) {
	// Attempt to do gamma correction (or something like it) to palette entries
	// to improve the contrast of the image on the original DS screen.

	// Split the color into it's component channels
	int r = color & 0x001F;
	int g = (color & 0x03E0) >> 5;
	int b = (color & 0x7C00) >> 10;

	// Caluclate the scaling factor for this color based on it's brightness
	int scale = ((23 - ((r + g + b) >> 2)) * _gammaValue) >> 3;

	// Scale the three components by the scaling factor, with clamping
	r = r + ((r * scale) >> 4);
	if (r > 31) r = 31;

	g = g + ((g * scale) >> 4);
	if (g > 31) g = 31;

	b = b + ((b * scale) >> 4);
	if (b > 31) b = 31;

	// Stick them back together into a 555 color value
	return 0x8000 | r | (g << 5) | (b << 10);
}

void OSystem_DS::engineDone() {
	// Scumm games appear not to stop their CD audio, so I stop the CD here.
	stopCD();
	DS::exitGame();

#ifdef ENABLE_AGI
	DS::clearAutoCompleteWordList();
#endif

}
