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

#include "common/scummsys.h"

#if defined(SDL_BACKEND) && defined(USE_OPENGL)

#include "backends/graphics/openglsdl/openglsdl-graphics.h"
#include "backends/platform/sdl/sdl.h"
#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/translation.h"

OpenGLSdlGraphicsManager::OpenGLSdlGraphicsManager(SdlEventSource *eventSource)
	:
	SdlGraphicsManager(eventSource),
	_hwscreen(0),
	_screenResized(false),
	_activeFullscreenMode(-2),
	_lastFullscreenModeWidth(0),
	_lastFullscreenModeHeight(0),
	_desktopWidth(0),
	_desktopHeight(0),
	_ignoreResizeFrames(0) {

	// Initialize SDL video subsystem
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1) {
		error("Could not initialize SDL: %s", SDL_GetError());
	}

	// This is also called in initSDL(), but initializing graphics
	// may reset it.
	SDL_EnableUNICODE(1);

	// Disable OS cursor
	SDL_ShowCursor(SDL_DISABLE);

	// Get desktop resolution
	// TODO: In case the OpenGL manager is created *after* a plain SDL manager
	// has been used, this will return the last setup graphics mode rather
	// than the desktop resolution. We should really look into a way to
	// properly retrieve the desktop resolution.
	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	if (videoInfo->current_w > 0 && videoInfo->current_h > 0) {
		_desktopWidth = videoInfo->current_w;
		_desktopHeight = videoInfo->current_h;
	}

	if (ConfMan.hasKey("last_fullscreen_mode_width") && ConfMan.hasKey("last_fullscreen_mode_height")) {
		_lastFullscreenModeWidth = ConfMan.getInt("last_fullscreen_mode_width");
		_lastFullscreenModeHeight = ConfMan.getInt("last_fullscreen_mode_height");
	}
}

OpenGLSdlGraphicsManager::~OpenGLSdlGraphicsManager() {
	// Unregister the event observer
	if (g_system->getEventManager()->getEventDispatcher() != NULL)
		g_system->getEventManager()->getEventDispatcher()->unregisterObserver(this);
}

void OpenGLSdlGraphicsManager::initEventObserver() {
	// Register the graphics manager as a event observer
	g_system->getEventManager()->getEventDispatcher()->registerObserver(this, 10, false);
}

bool OpenGLSdlGraphicsManager::hasFeature(OSystem::Feature f) {
	return
		(f == OSystem::kFeatureFullscreenMode) ||
		(f == OSystem::kFeatureIconifyWindow) ||
		OpenGLGraphicsManager::hasFeature(f);
}

void OpenGLSdlGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureIconifyWindow:
		if (enable)
			SDL_WM_IconifyWindow();
		break;
	default:
		OpenGLGraphicsManager::setFeatureState(f, enable);
	}
}

#ifdef USE_RGB_COLOR

Common::List<Graphics::PixelFormat> OpenGLSdlGraphicsManager::getSupportedFormats() const {
	assert(!_supportedFormats.empty());
	return _supportedFormats;
}

void OpenGLSdlGraphicsManager::detectSupportedFormats() {

	// Clear old list
	_supportedFormats.clear();

	// Some tables with standard formats that we always list
	// as "supported". If frontend code tries to use one of
	// these, we will perform the necessary format
	// conversion in the background. Of course this incurs a
	// performance hit, but on desktop ports this should not
	// matter. We still push the currently active format to
	// the front, so if frontend code just uses the first
	// available format, it will get one that is "cheap" to
	// use.
	const Graphics::PixelFormat RGBList[] = {
#if defined(ENABLE_32BIT)
		Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0), // RGBA8888
#ifndef USE_GLES
		Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24), // ARGB8888
#endif
		Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0),  // RGB888
#endif
		Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0),  // RGB565
		Graphics::PixelFormat(2, 5, 5, 5, 1, 11, 6, 1, 0),  // RGB5551
		Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0),  // RGB555
		Graphics::PixelFormat(2, 4, 4, 4, 4, 12, 8, 4, 0),  // RGBA4444
#ifndef USE_GLES
		Graphics::PixelFormat(2, 4, 4, 4, 4, 8, 4, 0, 12)   // ARGB4444
#endif
	};
#ifndef USE_GLES
	const Graphics::PixelFormat BGRList[] = {
#ifdef ENABLE_32BIT
		Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24), // ABGR8888
		Graphics::PixelFormat(4, 8, 8, 8, 8, 8, 16, 24, 0), // BGRA8888
		Graphics::PixelFormat(3, 8, 8, 8, 0, 0, 8, 16, 0),  // BGR888
#endif
		Graphics::PixelFormat(2, 5, 6, 5, 0, 0, 5, 11, 0),  // BGR565
		Graphics::PixelFormat(2, 5, 5, 5, 1, 1, 6, 11, 0),  // BGRA5551
		Graphics::PixelFormat(2, 4, 4, 4, 4, 0, 4, 8, 12),  // ABGR4444
		Graphics::PixelFormat(2, 4, 4, 4, 4, 4, 8, 12, 0)   // BGRA4444
	};
#endif

	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	if (_hwscreen) {
		// Get our currently set hardware format
		format = Graphics::PixelFormat(_hwscreen->format->BytesPerPixel,
			8 - _hwscreen->format->Rloss, 8 - _hwscreen->format->Gloss,
			8 - _hwscreen->format->Bloss, 8 - _hwscreen->format->Aloss,
			_hwscreen->format->Rshift, _hwscreen->format->Gshift,
			_hwscreen->format->Bshift, _hwscreen->format->Ashift);

		// Workaround to SDL not providing an accurate Aloss value on Mac OS X.
		if (_hwscreen->format->Amask == 0)
			format.aLoss = 8;

		// Push it first, as the prefered format if available
		for (int i = 0; i < ARRAYSIZE(RGBList); i++) {
			if (RGBList[i] == format) {
				_supportedFormats.push_back(format);
				break;
			}
		}
#ifndef USE_GLES
		for (int i = 0; i < ARRAYSIZE(BGRList); i++) {
			if (BGRList[i] == format) {
				_supportedFormats.push_back(format);
				break;
			}
		}
#endif
	}

	// Push some RGB formats
	for (int i = 0; i < ARRAYSIZE(RGBList); i++) {
		if (_hwscreen && (RGBList[i].bytesPerPixel > format.bytesPerPixel))
			continue;
		if (RGBList[i] != format)
			_supportedFormats.push_back(RGBList[i]);
	}
#ifndef USE_GLES
	// Push some BGR formats
	for (int i = 0; i < ARRAYSIZE(BGRList); i++) {
		if (_hwscreen && (BGRList[i].bytesPerPixel > format.bytesPerPixel))
			continue;
		if (BGRList[i] != format)
			_supportedFormats.push_back(BGRList[i]);
	}
#endif
	_supportedFormats.push_back(Graphics::PixelFormat::createFormatCLUT8());
}

#endif

void OpenGLSdlGraphicsManager::setInternalMousePosition(int x, int y) {
	SDL_WarpMouse(x, y);
}

void OpenGLSdlGraphicsManager::updateScreen() {
	if (_ignoreResizeFrames)
		_ignoreResizeFrames -= 1;

	OpenGLGraphicsManager::updateScreen();
}

//
// Intern
//

bool OpenGLSdlGraphicsManager::setupFullscreenMode() {
	SDL_Rect const* const*availableModes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_OPENGL);

	// SDL_ListModes() returns -1 in case any dimension is okay. In that
	// case we'll reuse the current desktop resolution for fullscreen.
	if (availableModes == (void *)-1) {
		_videoMode.hardwareWidth = _desktopWidth;
		_videoMode.hardwareHeight = _desktopHeight;
		_activeFullscreenMode = -2;
		return true;
	}

	// If -2, autodetect the fullscreen mode
	// The last used fullscreen mode will be prioritized, if there is no last fullscreen
	// mode, the desktop resolution will be used, and in case the desktop resolution
	// is not available as a fullscreen mode, the one with smallest metric will be selected.
	if (_activeFullscreenMode == -2) {
		// Desktop resolution
		int desktopModeIndex = -1;

		// Best metric mode
		const SDL_Rect *bestMode = availableModes[0];
		int bestModeIndex = 0;
		uint bestMetric = (uint)-1;

		// Iterate over all available fullscreen modes
		for (int i = 0; const SDL_Rect *mode = availableModes[i]; i++) {
			// Try to setup the last used fullscreen mode
			if (mode->w == _lastFullscreenModeWidth && mode->h == _lastFullscreenModeHeight) {
				_videoMode.hardwareWidth = _lastFullscreenModeWidth;
				_videoMode.hardwareHeight = _lastFullscreenModeHeight;
				_activeFullscreenMode = i;
				return true;
			}

			if (mode->w == _desktopWidth && mode->h == _desktopHeight)
				desktopModeIndex = i;

			if (mode->w < _videoMode.overlayWidth)
				continue;
			if (mode->h < _videoMode.overlayHeight)
				continue;

			uint metric = mode->w * mode->h - _videoMode.overlayWidth * _videoMode.overlayHeight;
			if (metric < bestMetric) {
				bestMode = mode;
				bestMetric = metric;
				bestModeIndex = i;
			}
		}

		if (desktopModeIndex >= 0) {
			_videoMode.hardwareWidth = _desktopWidth;
			_videoMode.hardwareHeight = _desktopHeight;

			_activeFullscreenMode = desktopModeIndex;
			return true;
		} else if (bestMode) {
			_videoMode.hardwareWidth = bestMode->w;
			_videoMode.hardwareHeight = bestMode->h;

			_activeFullscreenMode = bestModeIndex;
			return true;
		}
	} else {
		// Use last fullscreen mode if looping backwards from the first mode
		if (_activeFullscreenMode == -1) {
			do {
				_activeFullscreenMode++;
			} while(availableModes[_activeFullscreenMode]);
			_activeFullscreenMode--;
		}

		// Use first fullscreen mode if looping from last mode
		if (!availableModes[_activeFullscreenMode])
			_activeFullscreenMode = 0;

		// Check if the fullscreen mode is valid
		if (availableModes[_activeFullscreenMode]) {
			_videoMode.hardwareWidth = availableModes[_activeFullscreenMode]->w;
			_videoMode.hardwareHeight = availableModes[_activeFullscreenMode]->h;
			return true;
		}
	}

	// Could not find any suiting fullscreen mode, return false.
	return false;
}

bool OpenGLSdlGraphicsManager::loadGFXMode() {
	// If the screen was resized, do not change its size
	if (!_screenResized) {
		const int scaleFactor = getScale();
		_videoMode.overlayWidth = _videoMode.hardwareWidth = _videoMode.screenWidth * scaleFactor;
		_videoMode.overlayHeight = _videoMode.hardwareHeight = _videoMode.screenHeight * scaleFactor;

		// The only modes where we need to adapt the aspect ratio are 320x200
		// and 640x400. That is since our aspect ratio correction in fact is
		// only used to ensure that the original pixel size aspect for these
		// modes is used.
		// (Non-square pixels on old monitors vs square pixel on new ones).
		if (_videoMode.aspectRatioCorrection) {
		 	if (_videoMode.screenWidth == 320 && _videoMode.screenHeight == 200)
				_videoMode.overlayHeight = _videoMode.hardwareHeight = 240 * scaleFactor;
			else if (_videoMode.screenWidth == 640 && _videoMode.screenHeight == 400)
				_videoMode.overlayHeight = _videoMode.hardwareHeight = 480 * scaleFactor;
		}
	}

	_screenResized = false;

	// Setup OpenGL attributes for SDL
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	const bool isFullscreen = getFullscreenMode();

	// In case we have an fullscreen mode and we are not in a rollback, detect
	// a proper mode to use. In case we are in a rollback, we already detected
	// a proper mode when setting up that mode, thus there is no need to run
	// the detection again.
	if (isFullscreen && _transactionMode != kTransactionRollback) {
		if (!setupFullscreenMode())
			// Failed setuping a fullscreen mode
			return false;
	}

	_videoMode.overlayWidth = _videoMode.hardwareWidth;
	_videoMode.overlayHeight = _videoMode.hardwareHeight;

	uint32 flags = SDL_OPENGL;

	if (isFullscreen)
		flags |= SDL_FULLSCREEN;
	else
		flags |= SDL_RESIZABLE;

	// Create our window
	_hwscreen = SDL_SetVideoMode(_videoMode.hardwareWidth, _videoMode.hardwareHeight, 32, flags);
#ifdef USE_RGB_COLOR
	detectSupportedFormats();
#endif

	if (_hwscreen == NULL) {
		// DON'T use error(), as this tries to bring up the debug
		// console, which WON'T WORK now that _hwscreen is hosed.

		if (!_oldVideoMode.setup) {
			warning("SDL_SetVideoMode says we can't switch to that mode (%s)", SDL_GetError());
			g_system->quit();
		} else
			// Cancel GFX load, and go back to last mode
			return false;
	}

	// Check if the screen is BGR format
	setFormatIsBGR(_hwscreen->format->Rshift != 0);

	if (isFullscreen) {
		_lastFullscreenModeWidth = _videoMode.hardwareWidth;
		_lastFullscreenModeHeight = _videoMode.hardwareHeight;
		ConfMan.setInt("last_fullscreen_mode_width", _lastFullscreenModeWidth);
		ConfMan.setInt("last_fullscreen_mode_height", _lastFullscreenModeHeight);
	}

	// Call and return parent implementation of this method
	return OpenGLGraphicsManager::loadGFXMode();
}

void OpenGLSdlGraphicsManager::unloadGFXMode() {
	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen);
		_hwscreen = NULL;
	}
}

void OpenGLSdlGraphicsManager::internUpdateScreen() {
	// Call to parent implementation of this method
	OpenGLGraphicsManager::internUpdateScreen();

	// Swap OpenGL buffers
	SDL_GL_SwapBuffers();
}

#ifdef USE_OSD
void OpenGLSdlGraphicsManager::displayModeChangedMsg() {
	const char *newModeName = getCurrentModeName();
	if (newModeName) {
		const int scaleFactor = getScale();

		Common::String osdMessage = Common::String::format(
			"%s: %s\n%d x %d -> %d x %d",
			_("Current display mode"),
			newModeName,
			_videoMode.screenWidth * scaleFactor,
			_videoMode.screenHeight * scaleFactor,
			_hwscreen->w, _hwscreen->h
			);
		displayMessageOnOSD(osdMessage.c_str());
	}
}
void OpenGLSdlGraphicsManager::displayScaleChangedMsg() {
	const int scaleFactor = getScale();
	Common::String osdMessage = Common::String::format(
		"%s: x%d\n%d x %d -> %d x %d",
		_("Current scale"),
		scaleFactor,
		_videoMode.screenWidth, _videoMode.screenHeight,
		_videoMode.overlayWidth, _videoMode.overlayHeight
		);
	displayMessageOnOSD(osdMessage.c_str());
}
#endif

bool OpenGLSdlGraphicsManager::isHotkey(const Common::Event &event) {
	if ((event.kbd.flags & (Common::KBD_CTRL|Common::KBD_ALT)) == (Common::KBD_CTRL|Common::KBD_ALT)) {
		if (event.kbd.keycode == Common::KEYCODE_PLUS || event.kbd.keycode == Common::KEYCODE_MINUS ||
			event.kbd.keycode == Common::KEYCODE_KP_PLUS || event.kbd.keycode == Common::KEYCODE_KP_MINUS ||
			event.kbd.keycode == 'a' || event.kbd.keycode == 'f')
			return true;
	} else if ((event.kbd.flags & (Common::KBD_CTRL|Common::KBD_SHIFT)) == (Common::KBD_CTRL|Common::KBD_SHIFT)) {
		if (event.kbd.keycode == 'a' || event.kbd.keycode == 'f')
			return true;
	} else if ((event.kbd.flags & (Common::KBD_ALT)) == (Common::KBD_ALT) && event.kbd.keycode == 's') {
		return true;
	}
	return false;
}

void OpenGLSdlGraphicsManager::toggleFullScreen(int loop) {
	beginGFXTransaction();
		const bool isFullscreen = getFullscreenMode();

		if (isFullscreen && loop) {
			_activeFullscreenMode += loop;
			setFullscreenMode(true);
		} else {
			_activeFullscreenMode = -2;
			setFullscreenMode(!isFullscreen);
		}
	endGFXTransaction();

	// Ignore resize events for the next 10 frames
	_ignoreResizeFrames = 10;

#ifdef USE_OSD
	Common::String osdMessage;
	if (getFullscreenMode())
		osdMessage = Common::String::format("%s\n%d x %d",
			_("Fullscreen mode"),
			_hwscreen->w, _hwscreen->h
			);
	else
		osdMessage = Common::String::format("%s\n%d x %d",
			_("Windowed mode"),
			_hwscreen->w, _hwscreen->h
			);
	displayMessageOnOSD(osdMessage.c_str());
#endif
}

bool OpenGLSdlGraphicsManager::notifyEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		if (event.kbd.hasFlags(Common::KBD_ALT)) {
			// Alt-Return and Alt-Enter toggle full screen mode
			if (event.kbd.keycode == Common::KEYCODE_RETURN ||
				event.kbd.keycode == (Common::KeyCode)SDLK_KP_ENTER) {
				toggleFullScreen(0);
				return true;
			}

			// Alt-S create a screenshot
			if (event.kbd.keycode == 's') {
				Common::String filename;

				for (int n = 0;; n++) {
					SDL_RWops *file;

					filename = Common::String::format("scummvm%05d.bmp", n);
					file = SDL_RWFromFile(filename.c_str(), "r");
					if (!file)
						break;
					SDL_RWclose(file);
				}
				if (saveScreenshot(filename.c_str()))
					debug("Saved screenshot '%s'", filename.c_str());
				else
					warning("Could not save screenshot");
				return true;
			}
		}

		if (event.kbd.hasFlags(Common::KBD_CTRL|Common::KBD_ALT)) {
			// Ctrl-Alt-Return and Ctrl-Alt-Enter switch between full screen modes
			if (event.kbd.keycode == Common::KEYCODE_RETURN ||
				event.kbd.keycode == (Common::KeyCode)SDLK_KP_ENTER) {
				toggleFullScreen(1);
				return true;
			}

			// Ctrl-Alt-a switch between display modes
			if (event.kbd.keycode == 'a') {
				beginGFXTransaction();
					setFeatureState(OSystem::kFeatureAspectRatioCorrection, !getFeatureState(OSystem::kFeatureAspectRatioCorrection));
				endGFXTransaction();
#ifdef USE_OSD
			Common::String osdMessage;
			if (getFeatureState(OSystem::kFeatureAspectRatioCorrection))
				osdMessage = Common::String::format("%s\n%d x %d -> %d x %d",
				        _("Enabled aspect ratio correction"),
				        _videoMode.screenWidth, _videoMode.screenHeight,
				        _hwscreen->w, _hwscreen->h);
			else
				osdMessage = Common::String::format("%s\n%d x %d -> %d x %d",
				        _("Disabled aspect ratio correction"),
				        _videoMode.screenWidth, _videoMode.screenHeight,
				        _hwscreen->w, _hwscreen->h);
			displayMessageOnOSD(osdMessage.c_str());
#endif
				internUpdateScreen();
				return true;
			}

			// Ctrl-Alt-f toggles antialiasing
			if (event.kbd.keycode == 'f') {
				beginGFXTransaction();
					toggleAntialiasing();
				endGFXTransaction();

#ifdef USE_OSD
				// TODO: This makes guesses about what internal antialiasing
				// modes we use, we might want to consider a better way of
				// displaying information to the user.
				if (getAntialiasingState())
					displayMessageOnOSD(_("Active filter mode: Linear"));
				else
					displayMessageOnOSD(_("Active filter mode: Nearest"));
#endif
				return true;
			}

			SDLKey sdlKey = (SDLKey)event.kbd.keycode;

			// Ctrl+Alt+Plus/Minus Increase/decrease the scale factor
			if ((sdlKey == SDLK_EQUALS || sdlKey == SDLK_PLUS || sdlKey == SDLK_MINUS ||
				sdlKey == SDLK_KP_PLUS || sdlKey == SDLK_KP_MINUS)) {
				int factor = getScale();
				factor += (sdlKey == SDLK_MINUS || sdlKey == SDLK_KP_MINUS) ? -1 : +1;
				if (0 < factor && factor < 4) {
					// Check if the desktop resolution has been detected
					if (_desktopWidth > 0 && _desktopHeight > 0)
						// If the new scale factor is too big, do not scale
						if (_videoMode.screenWidth * factor > _desktopWidth ||
							_videoMode.screenHeight * factor > _desktopHeight)
							return false;

					beginGFXTransaction();
						setScale(factor);
					endGFXTransaction();
#ifdef USE_OSD
					displayScaleChangedMsg();
#endif
					return true;
				}
			}

			const bool isNormalNumber = (SDLK_1 <= sdlKey && sdlKey <= SDLK_3);
			const bool isKeypadNumber = (SDLK_KP1 <= sdlKey && sdlKey <= SDLK_KP3);

			// Ctrl-Alt-<number key> will change the GFX mode
			if (isNormalNumber || isKeypadNumber) {
				if (sdlKey - (isNormalNumber ? SDLK_1 : SDLK_KP1) <= 3) {
#ifdef USE_OSD
					int lastMode = _videoMode.mode;
#endif
					// We need to query the scale and set it up, because
					// setGraphicsMode sets the default scale to 2
					int oldScale = getScale();
					beginGFXTransaction();
						setGraphicsMode(sdlKey - (isNormalNumber ? SDLK_1 : SDLK_KP1));
						setScale(oldScale);
					endGFXTransaction();
#ifdef USE_OSD
					if (lastMode != _videoMode.mode)
						displayModeChangedMsg();
#endif
					internUpdateScreen();
				}
			}
		}

		if (event.kbd.hasFlags(Common::KBD_CTRL|Common::KBD_SHIFT)) {
			// Ctrl-Shift-Return and Ctrl-Shift-Enter switch backwards between full screen modes
			if (event.kbd.keycode == Common::KEYCODE_RETURN ||
				event.kbd.keycode == (Common::KeyCode)SDLK_KP_ENTER) {
				toggleFullScreen(-1);
				return true;
			}
		}
		break;

	case Common::EVENT_KEYUP:
		return isHotkey(event);

	default:
		break;
	}

	return false;
}

void OpenGLSdlGraphicsManager::notifyVideoExpose() {
}

void OpenGLSdlGraphicsManager::notifyResize(const uint width, const uint height) {
	// Do not resize if ignoring resize events.
	if (!_ignoreResizeFrames && !getFullscreenMode()) {
		bool scaleChanged = false;
		beginGFXTransaction();
			_videoMode.hardwareWidth = width;
			_videoMode.hardwareHeight = height;

			_screenResized = true;

			int scale = MIN(_videoMode.hardwareWidth / _videoMode.screenWidth,
			                _videoMode.hardwareHeight / _videoMode.screenHeight);

			if (getScale() != scale) {
				scaleChanged = true;
				setScale(MAX(MIN(scale, 3), 1));
			}

			_transactionDetails.sizeChanged = true;
		endGFXTransaction();
#ifdef USE_OSD
		if (scaleChanged)
			displayScaleChangedMsg();
#endif
	}
}

void OpenGLSdlGraphicsManager::transformMouseCoordinates(Common::Point &point) {
	adjustMousePosition(point.x, point.y);
}

void OpenGLSdlGraphicsManager::notifyMousePos(Common::Point mouse) {
	setMousePosition(mouse.x, mouse.y);
}

#endif
