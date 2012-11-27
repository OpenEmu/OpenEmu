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


#ifndef _OSYSTEM_DS_H_
#define _OSYSTEM_DS_H_

#include "backends/base-backend.h"
#include "common/events.h"
#include "nds.h"
#include "gbampsave.h"
#include "backends/saves/default/default-saves.h"
#include "audio/mixer_intern.h"
#include "graphics/surface.h"
#include "graphics/colormasks.h"
#include "graphics/palette.h"

class OSystem_DS : public EventsBaseBackend, public PaletteManager {
protected:

	int eventNum;
	int lastPenFrame;

	Common::Event eventQueue[96];
	int queuePos;

	GBAMPSaveFileManager mpSaveManager;
	Audio::MixerImpl *_mixer;
	Graphics::Surface _framebuffer;
	bool _frameBufferExists;
	bool _graphicsEnable;

	static OSystem_DS *_instance;

	u16 _palette[256];
	u16 _cursorPalette[256];

	u8 _cursorImage[64 * 64];
	uint _cursorW;
	uint _cursorH;
	int _cursorHotX;
	int _cursorHotY;
	byte _cursorKey;
	int _cursorScale;


	Graphics::Surface *createTempFrameBuffer();
	bool _disableCursorPalette;

	int _gammaValue;

public:
	typedef void (*SoundProc)(byte *buf, int len);
	typedef int  (*TimerProc)(int interval);

	OSystem_DS();
	virtual ~OSystem_DS();

	static OSystem_DS *instance() { return _instance; }

	virtual bool hasFeature(Feature f);
	virtual void setFeatureState(Feature f, bool enable);
	virtual bool getFeatureState(Feature f);
	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	bool setGraphicsMode(const char *name);
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
	void restoreHardwarePalette();

	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h);
	virtual void updateScreen();
	virtual void setShakePos(int shakeOffset);

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(void *buf, int pitch);
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayHeight();
	virtual int16 getOverlayWidth();
	virtual Graphics::PixelFormat getOverlayFormat() const { return Graphics::createPixelFormat<1555>(); }

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, u32 keycolor, bool dontScale, const Graphics::PixelFormat *format);

	virtual bool pollEvent(Common::Event &event);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);
	virtual void getTimeAndDate(TimeDate &t) const;

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);


	// FIXME/TODO: The CD API as follows is *obsolete*
	// and should be replaced by an AudioCDManager subclass,
	// see backends/audiocd/ and common/system.h
	virtual bool openCD(int drive);
	virtual bool pollCD();
	virtual void playCD(int track, int num_loops, int start_frame, int duration);
	virtual void stopCD();
	virtual void updateCD();

	virtual void quit();

	void addEvent(const Common::Event& e);
	bool isEventQueueEmpty() const { return queuePos == 0; }

	virtual void setFocusRectangle(const Common::Rect& rect);

	virtual void clearFocusRectangle();

	virtual void initBackend();

	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();

	virtual Audio::Mixer *getMixer() { return _mixer; }
	Audio::MixerImpl *getMixerImpl() { return _mixer; }

	static int timerHandler(int t);


	virtual void addAutoComplete(const char *word);
	virtual void clearAutoComplete();
	virtual void setCharactersEntered(int count);

	u16 getDSPaletteEntry(u32 entry) const { return _palette[entry]; }
	u16 getDSCursorPaletteEntry(u32 entry) const { return !_disableCursorPalette? _cursorPalette[entry]: _palette[entry]; }

	virtual void setCursorPalette(const byte *colors, uint start, uint num);

	virtual FilesystemFactory *getFilesystemFactory();

	void refreshCursor();

	virtual Common::String getDefaultConfigFileName();

	virtual void logMessage(LogMessageType::Type type, const char *message);

	u16 applyGamma(u16 color);
	void setGammaValue(int gamma) { _gammaValue = gamma; }

	void engineDone();
};

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{0, 0, 0},
};

#endif
