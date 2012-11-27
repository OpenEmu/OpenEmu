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

#ifndef OSYS_PSP_H
#define OSYS_PSP_H

#include "common/scummsys.h"
#include "graphics/surface.h"
#include "graphics/colormasks.h"
#include "graphics/palette.h"
#include "audio/mixer_intern.h"
#include "backends/base-backend.h"
#include "backends/fs/psp/psp-fs-factory.h"

#include "backends/platform/psp/display_client.h"
#include "backends/platform/psp/default_display_client.h"
#include "backends/platform/psp/cursor.h"
#include "backends/platform/psp/pspkeyboard.h"
#include "backends/platform/psp/image_viewer.h"
#include "backends/platform/psp/display_manager.h"
#include "backends/platform/psp/input.h"
#include "backends/platform/psp/audio.h"
#include "backends/timer/psp/timer.h"
#include "backends/platform/psp/thread.h"

class OSystem_PSP : public EventsBaseBackend, public PaletteManager {
private:

	Audio::MixerImpl *_mixer;
	bool _pendingUpdate;  			// save an update we couldn't perform
	uint32 _pendingUpdateCounter;	// prevent checking for pending update too often, in a cheap way

	// All needed sub-members
	Screen _screen;
	Overlay _overlay;
	Cursor _cursor;
	DisplayManager _displayManager;
	PSPKeyboard _keyboard;
	InputHandler _inputHandler;
	PspAudio _audio;
	PspTimer _pspTimer;
	ImageViewer _imageViewer;

public:
	OSystem_PSP() : _mixer(0), _pendingUpdate(false), _pendingUpdateCounter(0) {}
	~OSystem_PSP();

	static OSystem *instance();

	void initBackend();

	// Feature related
	bool hasFeature(Feature f);
	void setFeatureState(Feature f, bool enable);
	bool getFeatureState(Feature f);

	// Graphics related
	const GraphicsMode *getSupportedGraphicsModes() const;
	int getDefaultGraphicsMode() const;
	bool setGraphicsMode(int mode);
	bool setGraphicsMode(const char *name);
	int getGraphicsMode() const;
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
#endif

	// Screen size
	void initSize(uint width, uint height, const Graphics::PixelFormat *format);
	int16 getWidth();
	int16 getHeight();

	// Palette related
	PaletteManager *getPaletteManager() { return this; }
protected:
	// PaletteManager API
	void setPalette(const byte *colors, uint start, uint num);
	void grabPalette(byte *colors, uint start, uint num);
public:
	void setCursorPalette(const byte *colors, uint start, uint num);

	// Screen related
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h);
	Graphics::Surface *lockScreen();
	void unlockScreen();
	void updateScreen();
	void setShakePos(int shakeOffset);

	// Overlay related
	void showOverlay();
	void hideOverlay();
	void clearOverlay();
	void grabOverlay(void *buf, int pitch);
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h);
	int16 getOverlayHeight();
	int16 getOverlayWidth();
	Graphics::PixelFormat getOverlayFormat() const { return Graphics::createPixelFormat<4444>(); }

	// Mouse related
	bool showMouse(bool visible);
	void warpMouse(int x, int y);
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format);

	// Events and input
	bool pollEvent(Common::Event &event);
	bool processInput(Common::Event &event);

	// Time
	uint32 getMillis();
	void delayMillis(uint msecs);

	// Timer
	typedef int (*TimerProc)(int interval);
	void setTimerCallback(TimerProc callback, int interval);

	// Mutex
	MutexRef createMutex(void);
	void lockMutex(MutexRef mutex);
	void unlockMutex(MutexRef mutex);
	void deleteMutex(MutexRef mutex);

	// Sound
	static void mixCallback(void *sys, byte *samples, int len);
	void setupMixer(void);
	Audio::Mixer *getMixer() { return _mixer; }

	// Misc
	FilesystemFactory *getFilesystemFactory() { return &PSPFilesystemFactory::instance(); }
	void getTimeAndDate(TimeDate &t) const;
	virtual void engineDone();

	void quit();

	void logMessage(LogMessageType::Type type, const char *message);

	virtual Common::String getDefaultConfigFileName();
};

#endif /* OSYS_PSP_H */
