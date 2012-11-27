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

#ifndef BACKENDS_GRAPHICS_OPENGLSDL_H
#define BACKENDS_GRAPHICS_OPENGLSDL_H

#include "backends/platform/sdl/sdl-sys.h"
#if defined(ARRAYSIZE) && !defined(_WINDOWS_)
#undef ARRAYSIZE
#endif
#include "backends/graphics/sdl/sdl-graphics.h"
#include "backends/graphics/opengl/opengl-graphics.h"

#include "common/events.h"

/**
 * SDL OpenGL graphics manager
 */
class OpenGLSdlGraphicsManager : public OpenGLGraphicsManager, public SdlGraphicsManager, public Common::EventObserver {
public:
	OpenGLSdlGraphicsManager(SdlEventSource *eventSource);
	virtual ~OpenGLSdlGraphicsManager();

	virtual bool hasFeature(OSystem::Feature f);
	virtual void setFeatureState(OSystem::Feature f, bool enable);

#ifdef USE_RGB_COLOR
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const;
#endif

	virtual void initEventObserver();
	virtual bool notifyEvent(const Common::Event &event);

	virtual void updateScreen();

	// SdlGraphicsManager interface
	virtual void notifyVideoExpose();
	virtual void notifyResize(const uint width, const uint height);
	virtual void transformMouseCoordinates(Common::Point &point);
	virtual void notifyMousePos(Common::Point mouse);

protected:
	virtual void internUpdateScreen();

	virtual bool loadGFXMode();
	virtual void unloadGFXMode();
	virtual bool isHotkey(const Common::Event &event);

#ifdef USE_RGB_COLOR
	Common::List<Graphics::PixelFormat> _supportedFormats;

	/**
	 * Update the list of supported pixel formats.
	 * This method is invoked by loadGFXMode().
	 */
	void detectSupportedFormats();
#endif

	/**
	 * Toggles fullscreen.
	 * @loop loop direction for switching fullscreen mode, if 0 toggles it.
	 */
	virtual void toggleFullScreen(int loop);

	int _activeFullscreenMode;

	/**
	 * Setup the fullscreen mode.
	 * @return false if failed finding a mode, true otherwise.
	 */
	virtual bool setupFullscreenMode();

	virtual void setInternalMousePosition(int x, int y);

	int _lastFullscreenModeWidth;
	int _lastFullscreenModeHeight;
	int _desktopWidth;
	int _desktopHeight;

	// Hardware screen
	SDL_Surface *_hwscreen;

	// If screen was resized by the user
	bool _screenResized;

	// Ignore resize events for the number of updateScreen() calls.
	// Normaly resize events are user generated when resizing the window
	// from its borders, but in some cases a resize event can be generated
	// after a fullscreen change.
	int _ignoreResizeFrames;

#ifdef USE_OSD
	/**
	 * Displays a mode change message in OSD
	 */
	void displayModeChangedMsg();

	/**
	 * Displays a scale change message in OSD
	 */
	void displayScaleChangedMsg();
#endif
};

#endif
