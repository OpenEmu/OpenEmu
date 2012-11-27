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

#ifndef SYSTEMPS2_H
#define SYSTEMPS2_H

#include "common/system.h"
#include "backends/base-backend.h"
#include "graphics/palette.h"

class Gs2dScreen;
class Ps2Input;
// class Ps2FilesystemFactory;
struct IrxReference;

#define MAX_MUTEXES 16

struct Ps2Mutex {
	int sema;
	int owner;
	int count;
};

namespace Audio {
class MixerImpl;
};

class OSystem_PS2 : public EventsBaseBackend, public PaletteManager {
public:
	OSystem_PS2(const char *elfPath);
	virtual ~OSystem_PS2(void);
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format);

	void init(void);

	virtual int16 getHeight(void);
	virtual int16 getWidth(void);

	virtual PaletteManager *getPaletteManager() { return this; }
protected:
	// PaletteManager API
	virtual void setPalette(const byte *colors, uint start, uint num);
	virtual void grabPalette(byte *colors, uint start, uint num);
public:

	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h);
	virtual void setShakePos(int shakeOffset);
	virtual Graphics::Surface *lockScreen();
	virtual void unlockScreen();
	virtual void updateScreen();
	virtual void displayMessageOnOSD(const char *msg);

	virtual void showOverlay();
	virtual void hideOverlay();
	virtual void clearOverlay();
	virtual void grabOverlay(void *buf, int pitch);
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h);
	virtual int16 getOverlayWidth(void);
	virtual int16 getOverlayHeight(void);

	virtual bool showMouse(bool visible);

	virtual void warpMouse(int x, int y);
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = 0);

	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);
	virtual bool pollEvent(Common::Event &event);

	virtual Audio::Mixer *getMixer();

	virtual MutexRef createMutex(void);
	virtual void lockMutex(MutexRef mutex);
	virtual void unlockMutex(MutexRef mutex);
	virtual void deleteMutex(MutexRef mutex);

	virtual const GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
	virtual int getScreenChangeID() const { return _screenChangeCount; }

	virtual void quit();

	virtual Common::String getDefaultConfigFileName();

	virtual void logMessage(LogMessageType::Type type, const char *message);

	virtual Graphics::PixelFormat getOverlayFormat() const;
	virtual FilesystemFactory *getFilesystemFactory();

	virtual void getTimeAndDate(TimeDate &t) const;

	void timerThreadCallback(void);
	void soundThreadCallback(void);
	void msgPrintf(int millis, const char *format, ...) GCC_PRINTF(3, 4);
	void makeConfigPath(void);
	bool prepMC();

	void powerOffCallback(void);

	bool mcPresent(void);
	bool hddPresent(void);
	bool usbMassPresent(void);
	bool netPresent(void);

	bool runningFromHost(void);
	int getBootDevice() { return _bootDevice; }

private:
	void startIrxModules(int numModules, IrxReference *modules);

	void initMutexes(void);
	void initTimer(void);
	void readRtcTime(void);

	Audio::MixerImpl *_scummMixer;

	bool _mouseVisible;
	bool _useMouse, _useKbd, _useHdd, _usbMassLoaded, _useNet;

	Gs2dScreen	*_screen;
	Ps2Input	*_input;
	uint16		_oldMouseX, _oldMouseY;
	uint32		_msgClearTime;
	uint16		_printY;
	bool _modeChanged;
	int _screenChangeCount;

	int			_mutexSema;
	Ps2Mutex	_mutex[MAX_MUTEXES];

	uint8		*_timerStack, *_soundStack;
	int			_timerTid, _soundTid;
	int			_intrId;
	volatile bool _systemQuit;
	static const GraphicsMode _graphicsMode;

	int			_bootDevice;
	char		*_bootPath;
	char		*_configFile;
};

#endif // SYSTEMPS2_H
