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

#ifndef BACKENDS_PLATFORM_IPHONE_OSYS_MAIN_H
#define BACKENDS_PLATFORM_IPHONE_OSYS_MAIN_H

#include "graphics/surface.h"
#include "iphone_common.h"
#include "backends/base-backend.h"
#include "common/events.h"
#include "audio/mixer_intern.h"
#include "backends/fs/posix/posix-fs-factory.h"
#include "graphics/colormasks.h"
#include "graphics/palette.h"

#include <AudioToolbox/AudioQueue.h>

#define AUDIO_BUFFERS 3
#define WAVE_BUFFER_SIZE 2048
#define AUDIO_SAMPLE_RATE 44100

#define SCUMMVM_ROOT_PATH "/var/mobile/Library/ScummVM"
#define SCUMMVM_SAVE_PATH SCUMMVM_ROOT_PATH "/Savegames"
#define SCUMMVM_PREFS_PATH SCUMMVM_ROOT_PATH "/Preferences"

typedef void (*SoundProc)(void *param, byte *buf, int len);
typedef int (*TimerProc)(int interval);

struct AQCallbackStruct {
	AudioQueueRef queue;
	uint32 frameCount;
	AudioQueueBufferRef buffers[AUDIO_BUFFERS];
	AudioStreamBasicDescription dataFormat;
};

class OSystem_IPHONE : public EventsBaseBackend, public PaletteManager {
protected:
	static const OSystem::GraphicsMode s_supportedGraphicsModes[];
	static AQCallbackStruct s_AudioQueue;
	static SoundProc s_soundCallback;
	static void *s_soundParam;

	Audio::MixerImpl *_mixer;

	VideoContext *_videoContext;

	Graphics::Surface _framebuffer;

	// For signaling that screen format set up might have failed.
	TransactionError _gfxTransactionError;

	// For use with the game texture
	uint16  _gamePalette[256];
	// For use with the mouse texture
	uint16  _gamePaletteRGBA5551[256];

	struct timeval _startTime;
	uint32 _timeSuspended;

	bool _mouseCursorPaletteEnabled;
	uint16 _mouseCursorPalette[256];
	Graphics::Surface _mouseBuffer;
	uint16 _mouseKeyColor;
	bool _mouseDirty;
	bool _mouseNeedTextureUpdate;

	long _lastMouseDown;
	long _lastMouseTap;
	long _queuedEventTime;
	Common::Event _queuedInputEvent;
	bool _secondaryTapped;
	long _lastSecondaryDown;
	long _lastSecondaryTap;
	int _gestureStartX, _gestureStartY;
	bool _mouseClickAndDragEnabled;
	bool _touchpadModeEnabled;
	int _lastPadX;
	int _lastPadY;
	int _lastDragPosX;
	int _lastDragPosY;

	int _timerCallbackNext;
	int _timerCallbackTimer;
	TimerProc _timerCallback;

	Common::Array<Common::Rect> _dirtyRects;
	Common::Array<Common::Rect> _dirtyOverlayRects;
	ScreenOrientation _screenOrientation;
	bool _fullScreenIsDirty;
	bool _fullScreenOverlayIsDirty;
	int _screenChangeCount;

public:

	OSystem_IPHONE();
	virtual ~OSystem_IPHONE();

	virtual void initBackend();

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format);

	virtual void beginGFXTransaction();
	virtual TransactionError endGFXTransaction();

	virtual int16 getHeight();
	virtual int16 getWidth();

#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const { return _framebuffer.format; }
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
#endif

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
	virtual Graphics::PixelFormat getOverlayFormat() const { return Graphics::createPixelFormat<5551>(); }

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor = 255, bool dontScale = false, const Graphics::PixelFormat *format = NULL);
	virtual void setCursorPalette(const byte *colors, uint start, uint num);

	virtual bool pollEvent(Common::Event &event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	static void mixCallback(void *sys, byte *samples, int len);
	virtual void setupMixer(void);
	virtual void setTimerCallback(TimerProc callback, int interval);
	virtual int getScreenChangeID() const { return _screenChangeCount; }
	virtual void quit();

	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);
	virtual void getTimeAndDate(TimeDate &t) const;

	virtual Audio::Mixer *getMixer();

	void startSoundsystem();
	void stopSoundsystem();

	virtual Common::String getDefaultConfigFileName();

	virtual void logMessage(LogMessageType::Type type, const char *message);

protected:
	void initVideoContext();
	void updateOutputSurface();

	void internUpdateScreen();
	void dirtyFullScreen();
	void dirtyFullOverlayScreen();
	void suspendLoop();
	void drawDirtyRect(const Common::Rect &dirtyRect);
	void updateMouseTexture();
	static void AQBufferCallback(void *in, AudioQueueRef inQ, AudioQueueBufferRef outQB);
	static int timerHandler(int t);

	bool handleEvent_swipe(Common::Event &event, int direction);
	void handleEvent_keyPressed(Common::Event &event, int keyPressed);
	void handleEvent_orientationChanged(int orientation);

	bool handleEvent_mouseDown(Common::Event &event, int x, int y);
	bool handleEvent_mouseUp(Common::Event &event, int x, int y);

	bool handleEvent_secondMouseDown(Common::Event &event, int x, int y);
	bool handleEvent_secondMouseUp(Common::Event &event, int x, int y);

	bool handleEvent_mouseDragged(Common::Event &event, int x, int y);
	bool handleEvent_mouseSecondDragged(Common::Event &event, int x, int y);
};

#endif
