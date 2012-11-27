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

#ifndef PLATFORM_SDL_H
#define PLATFORM_SDL_H

#include "backends/platform/sdl/sdl-sys.h"

#include "backends/modular-backend.h"
#include "backends/mixer/sdl/sdl-mixer.h"
#include "backends/events/sdl/sdl-events.h"
#include "backends/log/log.h"

/**
 * Base OSystem class for all SDL ports.
 */
class OSystem_SDL : public ModularBackend {
public:
	OSystem_SDL();
	virtual ~OSystem_SDL();

	/**
	 * Pre-initialize backend. It should be called after
	 * instantiating the backend. Early needed managers are
	 * created here.
	 */
	virtual void init();

	/**
	 * Get the Mixer Manager instance. Not to confuse with getMixer(),
	 * that returns Audio::Mixer. The Mixer Manager is a SDL wrapper class
	 * for the Audio::Mixer. Used by other managers.
	 */
	virtual SdlMixerManager *getMixerManager();

	// Override functions from ModularBackend and OSystem
	virtual void initBackend();
#if defined(USE_TASKBAR)
	virtual void engineInit();
	virtual void engineDone();
#endif
	virtual void quit();
	virtual void fatalError();

	// Logging
	virtual void logMessage(LogMessageType::Type type, const char *message);

	virtual Common::String getSystemLanguage() const;

	virtual void setWindowCaption(const char *caption);
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);
	virtual uint32 getMillis();
	virtual void delayMillis(uint msecs);
	virtual void getTimeAndDate(TimeDate &td) const;
	virtual Audio::Mixer *getMixer();

protected:
	bool _inited;
	bool _initedSDL;

	/**
	 * Mixer manager that configures and setups SDL for
	 * the wrapped Audio::Mixer, the true mixer.
	 */
	SdlMixerManager *_mixerManager;

	/**
	 * The event source we use for obtaining SDL events.
	 */
	SdlEventSource *_eventSource;

	virtual Common::EventSource *getDefaultEventSource() { return _eventSource; }

	/**
	 * Initialze the SDL library.
	 */
	virtual void initSDL();

	/**
	 * Setup the window icon.
	 */
	virtual void setupIcon();

	// Logging
	virtual Common::WriteStream *createLogFile() { return 0; }
	Backends::Log::Log *_logger;

#ifdef USE_OPENGL
	OSystem::GraphicsMode *_graphicsModes;
	int _graphicsMode;
	int _sdlModesCount;
	int _glModesCount;

	/**
	 * Creates the merged graphics modes list
	 */
	virtual void setupGraphicsModes();

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const;
	virtual int getDefaultGraphicsMode() const;
	virtual bool setGraphicsMode(int mode);
	virtual int getGraphicsMode() const;
#endif
};

#endif
