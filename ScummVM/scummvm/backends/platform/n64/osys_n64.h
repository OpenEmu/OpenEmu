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

#ifndef __OSYS_N64_H__
#define __OSYS_N64_H__

#include "common/rect.h"
#include "common/config-manager.h"

#include "backends/base-backend.h"

#include "base/main.h"

#include "graphics/surface.h"
#include "graphics/colormasks.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"

#include "audio/mixer_intern.h"

#include <libn64.h>
#include <n64utils.h>

#define DEFAULT_SOUND_SAMPLE_RATE 8000 // 8 kHz
//#define DEFAULT_SOUND_SAMPLE_RATE 11025 // 11 kHz

#define N64_PAL_FPS 25
#define N64_NTSC_FPS 30

typedef int (*TimerProc)(int interval);

// Interrupt callback functions
void vblCallback(void);
void sndCallback(void);
void refillAudioBuffers(void);

// External utility functions
void enableAudioPlayback(void);
void disableAudioPlayback(void);
void checkTimers(void);
int timer_handler(int t);

static volatile bool _audioEnabled = false; // Used by interrupt callbacks

/* Graphic mode identifiers */
enum GraphicModeID {
	OVERS_NTSC_340X240,
	NORM_NTSC_320X240,
	NORM_PAL_320X240,
	OVERS_PAL_340X240,
	NORM_MPAL_320X240,
	OVERS_MPAL_340X240
};

class OSystem_N64 : public EventsBaseBackend, public PaletteManager {
protected:
	Audio::MixerImpl *_mixer;

	struct display_context * _dc; // Display context for N64 on screen buffer switching

	Graphics::Surface _framebuffer;

	uint16 *_offscreen_hic; // Offscreen converted to 16bit surface
	uint8  *_offscreen_pal; // Offscreen with palette indexes
	uint16 *_overlayBuffer; // Offscreen for the overlay (16 bit)

	uint16 *_screenPalette; // Array for palette entries (256 colors max)

#ifndef N64_EXTREME_MEMORY_SAVING
	uint8 *_screenExactPalette; // Array for palette entries, as received by setPalette(), no precision loss
#endif
	uint16 _cursorPalette[256]; // Palette entries for the cursor

	int _graphicMode; // Graphic mode
	uint16 _screenWidth, _screenHeight;
	uint16 _gameWidth, _gameHeight;
	uint16 _frameBufferWidth; // Width of framebuffer in N64 memory
	uint8 _offscrPixels; // Pixels to skip on each line before start drawing, used to center image
	uint8 _maxFps; // Max frames-per-second which can be shown on screen

	int _shakeOffset;

	uint8 *_cursor_pal; // Cursor buffer, palettized
	uint16 *_cursor_hic; // Cursor buffer, 16bit
	bool _cursorPaletteDisabled;
	bool _dirtyPalette;

	uint _cursorWidth, _cursorHeight;

	int _cursorKeycolor;

	uint16	_overlayHeight, _overlayWidth;
	bool	_overlayVisible;

	bool	_disableFpsLimit; // When this is enabled, the system doesn't limit screen updates

	bool _mouseVisible;
	volatile int _mouseX, _mouseY;
	volatile float _tempMouseX, _tempMouseY;
	volatile int _mouseMaxX, _mouseMaxY;
	int _mouseHotspotX, _mouseHotspotY;

	int8 _controllerPort;
	int8 _mousePort;
	bool _controllerHasRumble; // Gets enabled if rumble-pak is detected

	bool _dirtyOffscreen;

public:

	/* These have to be accessed by interrupt callbacks */
	uint16 _audioBufferSize;
	uint32 _viClockRate; // Clock rate of video system, depending on VI mode

	uint32 _timerCallbackNext;
	uint32 _timerCallbackTimer;
	TimerProc _timerCallback;
	/* *** */

	OSystem_N64();
	virtual ~OSystem_N64();

	virtual void initBackend();

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	bool setGraphicsMode(const char *name);
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format);
	virtual int16 getHeight();
	virtual int16 getWidth();

	virtual PaletteManager *getPaletteManager() { return this; }
protected:
	// PaletteManager API
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num);

public:
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h);
	virtual void updateScreen();
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	virtual void setShakePos(int shakeOffset);

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(void *buf, int pitch);
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	virtual Graphics::PixelFormat getOverlayFormat() const {
		return Graphics::createPixelFormat<555>();
	}

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format);
	virtual void setCursorPalette(const byte *colors, uint start, uint num);

	virtual bool pollEvent(Common::Event &event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual void quit();

	virtual Audio::Mixer *getMixer();
	virtual void getTimeAndDate(TimeDate &t) const;
	virtual void setTimerCallback(TimerProc callback, int interval);
	virtual void logMessage(LogMessageType::Type type, const char *message);

	void rebuildOffscreenGameBuffer(void);
	void rebuildOffscreenMouseBuffer(void);
	void switchGraphicModeId(int mode);

	void setupMixer(void);

	void detectControllers(void);
	void readControllerAnalogInput(void); // read controller analog nub position
};

#endif /* __OSYS_N64_H__ */
