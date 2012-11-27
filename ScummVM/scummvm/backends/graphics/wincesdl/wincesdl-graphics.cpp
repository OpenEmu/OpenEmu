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

#ifdef _WIN32_WCE

#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "common/mutex.h"

#include "graphics/scaler/downscaler.h"
#include "graphics/scaler/aspect.h"
#include "backends/graphics/wincesdl/wincesdl-graphics.h"
#include "backends/events/wincesdl/wincesdl-events.h"
#include "backends/platform/wince/wince-sdl.h"

#include "backends/platform/wince/resource.h"
#include "backends/platform/wince/CEActionsPocket.h"
#include "backends/platform/wince/CEActionsSmartphone.h"
#include "backends/platform/wince/CEDevice.h"
#include "backends/platform/wince/CEScaler.h"
#include "backends/platform/wince/CEgui/ItemAction.h"

WINCESdlGraphicsManager::WINCESdlGraphicsManager(SdlEventSource *sdlEventSource)
	: SurfaceSdlGraphicsManager(sdlEventSource),
	  _panelInitialized(false), _noDoubleTapRMB(false), _noDoubleTapPT(false),
	  _toolbarHighDrawn(false), _newOrientation(0), _orientationLandscape(0),
	  _panelVisible(true), _saveActiveToolbar(NAME_MAIN_PANEL), _panelStateForced(false),
	  _canBeAspectScaled(false), _scalersChanged(false), _saveToolbarState(false),
	  _mouseBackupOld(NULL), _mouseBackupDim(0), _mouseBackupToolbar(NULL),
	  _usesEmulatedMouse(false), _forceHideMouse(false), _freeLook(false),
	  _hasfocus(true), _zoomUp(false), _zoomDown(false) {
	memset(&_mouseCurState, 0, sizeof(_mouseCurState));
	if (_isSmartphone) {
		_mouseCurState.x = 20;
		_mouseCurState.y = 20;
	}

	loadDeviceConfigurationElement("repeatTrigger", _keyRepeatTrigger, 200);
	loadDeviceConfigurationElement("repeatX", _repeatX, 4);
	loadDeviceConfigurationElement("repeatY", _repeatY, 4);
	loadDeviceConfigurationElement("stepX1", _stepX1, 2);
	loadDeviceConfigurationElement("stepX2", _stepX2, 10);
	loadDeviceConfigurationElement("stepX3", _stepX3, 40);
	loadDeviceConfigurationElement("stepY1", _stepY1, 2);
	loadDeviceConfigurationElement("stepY2", _stepY2, 10);
	loadDeviceConfigurationElement("stepY3", _stepY3, 20);
	ConfMan.flushToDisk();

	_isSmartphone = CEDevice::isSmartphone();

	// Query SDL for screen size and init screen dependent stuff
	OSystem_WINCE3::initScreenInfos();
	create_toolbar();
	_hasSmartphoneResolution = CEDevice::hasSmartphoneResolution() || CEDevice::isSmartphone();
	if (_hasSmartphoneResolution)
		_panelVisible = false;  // init correctly in smartphones

	_screen = NULL;
}

// Graphics mode consts

// Low end devices 240x320

static const OSystem::GraphicsMode s_supportedGraphicsModesLow[] = {
	{"1x", _s("Normal (no scaling)"), GFX_NORMAL},
	{0, 0, 0}
};

// High end device 480x640

static const OSystem::GraphicsMode s_supportedGraphicsModesHigh[] = {
	{"1x", _s("Normal (no scaling)"), GFX_NORMAL},
	{"2x", "2x", GFX_DOUBLESIZE},
#ifndef _MSC_VER // EVC breaks template functions, and I'm tired of fixing them :)
	{"2xsai", "2xSAI", GFX_2XSAI},
	{"super2xsai", "Super2xSAI", GFX_SUPER2XSAI},
	{"supereagle", "SuperEagle", GFX_SUPEREAGLE},
#endif
	{"advmame2x", "AdvMAME2x", GFX_ADVMAME2X},
#ifndef _MSC_VER
	{"hq2x", "HQ2x", GFX_HQ2X},
	{"tv2x", "TV2x", GFX_TV2X},
#endif
	{"dotmatrix", "DotMatrix", GFX_DOTMATRIX},
	{0, 0, 0}
};

const OSystem::GraphicsMode *WINCESdlGraphicsManager::getSupportedGraphicsModes() const {
	if (CEDevice::hasWideResolution())
		return s_supportedGraphicsModesHigh;
	else
		return s_supportedGraphicsModesLow;
}

bool WINCESdlGraphicsManager::hasFeature(OSystem::Feature f) {
	return (f == OSystem::kFeatureVirtualKeyboard);
}

void WINCESdlGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		return;

	case OSystem::kFeatureVirtualKeyboard:
		if (_hasSmartphoneResolution)
			return;
		_toolbarHighDrawn = false;
		if (enable) {
			_panelStateForced = true;
			if (!_toolbarHandler.visible()) swap_panel_visibility();
			//_saveToolbarState = _toolbarHandler.visible();
			_saveActiveToolbar = _toolbarHandler.activeName();
			_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);
			_toolbarHandler.setVisible(true);
		} else if (_panelStateForced) {
			_panelStateForced = false;
			_toolbarHandler.setActive(_saveActiveToolbar);
			//_toolbarHandler.setVisible(_saveToolbarState);
		}
		return;

	case OSystem::kFeatureDisableKeyFiltering:
		if (_hasSmartphoneResolution) {
			GUI::Actions::Instance()->beginMapping(enable);
		}
		return;

	default:
		SurfaceSdlGraphicsManager::setFeatureState(f, enable);
	}
}

bool WINCESdlGraphicsManager::getFeatureState(OSystem::Feature f) {
	switch (f) {
	case OSystem::kFeatureFullscreenMode:
		return false;
	case OSystem::kFeatureVirtualKeyboard:
		return (_panelStateForced);
	default:
		return SurfaceSdlGraphicsManager::getFeatureState(f);
	}
}

int WINCESdlGraphicsManager::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

void WINCESdlGraphicsManager::initSize(uint w, uint h, const Graphics::PixelFormat *format) {
	if (_hasSmartphoneResolution && h == 240)
		h = 200;  // mainly for the launcher

	if (_isSmartphone && !ConfMan.hasKey("landscape")) {
		ConfMan.setInt("landscape", 1);
		ConfMan.flushToDisk();
	}

	_canBeAspectScaled = false;
	if (w == 320 && h == 200 && !_hasSmartphoneResolution) {
		_canBeAspectScaled = true;
		h = 240; // use the extra 40 pixels height for the toolbar
	}

	if (h == 400)   // touche engine fixup
		h += 80;

	if (!_hasSmartphoneResolution) {
		if (h == 240)
			_toolbarHandler.setOffset(200);
		else
			_toolbarHandler.setOffset(400);
	} else {
		if (h == 240)
			_toolbarHandler.setOffset(200);
		else    // 176x220
			_toolbarHandler.setOffset(0);
	}

	if (w != (uint) _videoMode.screenWidth || h != (uint) _videoMode.screenHeight)
		_scalersChanged = false;

	_videoMode.overlayWidth = w;
	_videoMode.overlayHeight = h;

	SurfaceSdlGraphicsManager::initSize(w, h, format);

	if (_scalersChanged) {
		unloadGFXMode();
		loadGFXMode();
		_scalersChanged = false;
	}

	update_game_settings();
}

void WINCESdlGraphicsManager::loadDeviceConfigurationElement(Common::String element, int &value, int defaultValue) {
	value = ConfMan.getInt(element, ConfMan.kApplicationDomain);
	if (!value) {
		value = defaultValue;
		ConfMan.setInt(element, value, ConfMan.kApplicationDomain);
	}
}

void WINCESdlGraphicsManager::move_cursor_up() {
	int x, y;
	_usesEmulatedMouse = true;
	retrieve_mouse_location(x, y);
	if (_keyRepeat > _repeatY)
		y -= _stepY3;
	else if (_keyRepeat)
		y -= _stepY2;
	else
		y -= _stepY1;

	if (y < 0)
		y = 0;

	EventsBuffer::simulateMouseMove(x, y);
}

void WINCESdlGraphicsManager::move_cursor_down() {
	int x, y;
	_usesEmulatedMouse = true;
	retrieve_mouse_location(x, y);
	if (_keyRepeat > _repeatY)
		y += _stepY3;
	else if (_keyRepeat)
		y += _stepY2;
	else
		y += _stepY1;

	if (y > _videoMode.screenHeight * _scaleFactorYm / _scaleFactorYd)
		y = _videoMode.screenHeight * _scaleFactorYm / _scaleFactorYd;

	EventsBuffer::simulateMouseMove(x, y);
}

void WINCESdlGraphicsManager::move_cursor_left() {
	int x, y;
	_usesEmulatedMouse = true;
	retrieve_mouse_location(x, y);
	if (_keyRepeat > _repeatX)
		x -= _stepX3;
	else if (_keyRepeat)
		x -= _stepX2;
	else
		x -= _stepX1;

	if (x < 0)
		x = 0;

	EventsBuffer::simulateMouseMove(x, y);
}

void WINCESdlGraphicsManager::move_cursor_right() {
	int x, y;
	_usesEmulatedMouse = true;
	retrieve_mouse_location(x, y);
	if (_keyRepeat > _repeatX)
		x += _stepX3;
	else if (_keyRepeat)
		x += _stepX2;
	else
		x += _stepX1;

	if (x > _videoMode.screenWidth * _scaleFactorXm / _scaleFactorXd)
		x = _videoMode.screenWidth * _scaleFactorXm / _scaleFactorXd;

	EventsBuffer::simulateMouseMove(x, y);
}

void WINCESdlGraphicsManager::retrieve_mouse_location(int &x, int &y) {
	x = _mouseCurState.x;
	y = _mouseCurState.y;

	x = x * _scaleFactorXm / _scaleFactorXd;
	y = y * _scaleFactorYm / _scaleFactorYd;

	if (_zoomDown)
		y -= 240;
}

void WINCESdlGraphicsManager::switch_zone() {
	int x, y;
	int i;
	retrieve_mouse_location(x, y);

	for (i = 0; i < TOTAL_ZONES; i++)
		if (x >= _zones[i].x && y >= _zones[i].y &&
		        x <= _zones[i].x + _zones[i].width && y <= _zones[i].y + _zones[i].height) {
			_mouseXZone[i] = x;
			_mouseYZone[i] = y;
			break;
		}
	_currentZone = i + 1;
	if (_currentZone >= TOTAL_ZONES)
		_currentZone = 0;

	EventsBuffer::simulateMouseMove(_mouseXZone[_currentZone], _mouseYZone[_currentZone]);
}

void WINCESdlGraphicsManager::add_right_click(bool pushed) {
	int x, y;
	retrieve_mouse_location(x, y);
	EventsBuffer::simulateMouseRightClick(x, y, pushed);
}

void WINCESdlGraphicsManager::add_left_click(bool pushed) {
	int x, y;
	retrieve_mouse_location(x, y);
	EventsBuffer::simulateMouseLeftClick(x, y, pushed);
}

bool WINCESdlGraphicsManager::update_scalers() {
	_videoMode.aspectRatioCorrection = false;

	if (CEDevice::hasPocketPCResolution()) {
		if (_videoMode.mode != GFX_NORMAL)
			return false;

		if ((!_orientationLandscape && (_videoMode.screenWidth == 320 || !_videoMode.screenWidth))
		        || CEDevice::hasSquareQVGAResolution()) {
			if (OSystem_WINCE3::getScreenWidth() != 320) {
				_scaleFactorXm = 3;
				_scaleFactorXd = 4;
				_scaleFactorYm = 1;
				_scaleFactorYd = 1;
				_scalerProc = DownscaleHorizByThreeQuarters;
			} else {
				_scaleFactorXm = 1;
				_scaleFactorXd = 1;
				_scaleFactorYm = 1;
				_scaleFactorYd = 1;
				_scalerProc = Normal1x;
			}
		} else if (_orientationLandscape && (_videoMode.screenWidth == 320 || !_videoMode.screenWidth)) {
			if (!_panelVisible && !_hasSmartphoneResolution  && !_overlayVisible && _canBeAspectScaled) {
				_scaleFactorXm = 1;
				_scaleFactorXd = 1;
				_scaleFactorYm = 6;
				_scaleFactorYd = 5;
				_scalerProc = Normal1xAspect;
				_videoMode.aspectRatioCorrection = true;
			} else {
				_scaleFactorXm = 1;
				_scaleFactorXd = 1;
				_scaleFactorYm = 1;
				_scaleFactorYd = 1;
				_scalerProc = Normal1x;
			}
		} else if (_videoMode.screenWidth == 640 && !(OSystem_WINCE3::isOzone() && (OSystem_WINCE3::getScreenWidth() >= 640 || OSystem_WINCE3::getScreenHeight() >= 640))) {
			_scaleFactorXm = 1;
			_scaleFactorXd = 2;
			_scaleFactorYm = 1;
			_scaleFactorYd = 2;
			_scalerProc = DownscaleAllByHalf;
		} else if (_videoMode.screenWidth == 640 && (OSystem_WINCE3::isOzone() && (OSystem_WINCE3::getScreenWidth() >= 640 || OSystem_WINCE3::getScreenHeight() >= 640))) {
			_scaleFactorXm = 1;
			_scaleFactorXd = 1;
			_scaleFactorYm = 1;
			_scaleFactorYd = 1;
			_scalerProc = Normal1x;
		}

		return true;
	} else if (CEDevice::hasWideResolution()) {
#ifdef USE_ARM_SCALER_ASM
		if (_videoMode.mode == GFX_DOUBLESIZE && (_videoMode.screenWidth == 320 || !_videoMode.screenWidth)) {
			if (!_panelVisible && !_overlayVisible && _canBeAspectScaled) {
				_scaleFactorXm = 2;
				_scaleFactorXd = 1;
				_scaleFactorYm = 12;
				_scaleFactorYd = 5;
				_scalerProc = Normal2xAspect;
				_videoMode.aspectRatioCorrection = true;
			} else if ((_panelVisible || _overlayVisible) && _canBeAspectScaled) {
				_scaleFactorXm = 2;
				_scaleFactorXd = 1;
				_scaleFactorYm = 2;
				_scaleFactorYd = 1;
				_scalerProc = Normal2x;
			}
			return true;
		}
#endif
	} else if (CEDevice::hasSmartphoneResolution()) {
		if (_videoMode.mode != GFX_NORMAL)
			return false;

		if (_videoMode.screenWidth > 320)
			error("Game resolution not supported on Smartphone");
#ifdef ARM
		_scaleFactorXm = 11;
		_scaleFactorXd = 16;
#else
		_scaleFactorXm = 2;
		_scaleFactorXd = 3;
#endif
		_scaleFactorYm = 7;
		_scaleFactorYd = 8;
		_scalerProc = SmartphoneLandscape;
		initZones();
		return true;
	}

	return false;
}

void WINCESdlGraphicsManager::update_game_settings() {
	Common::String gameid(ConfMan.get("gameid"));

	// Finish panel initialization
	if (!_panelInitialized && !gameid.empty()) {
		CEGUI::Panel *panel;
		_panelInitialized = true;
		// Add the main panel
		panel = new CEGUI::Panel(0, 32);
		panel->setBackground(IMAGE_PANEL);

		// Save
		panel->add(NAME_ITEM_OPTIONS, new CEGUI::ItemAction(ITEM_OPTIONS, POCKET_ACTION_SAVE));
		// Skip
		panel->add(NAME_ITEM_SKIP, new CEGUI::ItemAction(ITEM_SKIP, POCKET_ACTION_SKIP));
		// sound
		panel->add(NAME_ITEM_SOUND, new CEGUI::ItemSwitch(ITEM_SOUND_OFF, ITEM_SOUND_ON, &OSystem_WINCE3::_soundMaster));

		// bind keys
		panel->add(NAME_ITEM_BINDKEYS, new CEGUI::ItemAction(ITEM_BINDKEYS, POCKET_ACTION_BINDKEYS));
		// portrait/landscape - screen dependent
		// FIXME : will still display the portrait/landscape icon when using a scaler (but will be disabled)
		if (ConfMan.hasKey("landscape")) {
			if (ConfMan.get("landscape")[0] > 57) {
				_newOrientation = _orientationLandscape = ConfMan.getBool("landscape");
				//ConfMan.removeKey("landscape", "");
				ConfMan.setInt("landscape", _orientationLandscape);
			} else
				_newOrientation = _orientationLandscape = ConfMan.getInt("landscape");
		} else {
			_newOrientation = _orientationLandscape = 0;
		}
		panel->add(NAME_ITEM_ORIENTATION, new CEGUI::ItemSwitch(ITEM_VIEW_LANDSCAPE, ITEM_VIEW_PORTRAIT, &_newOrientation, 2));
		_toolbarHandler.add(NAME_MAIN_PANEL, *panel);
		_toolbarHandler.setActive(NAME_MAIN_PANEL);
		_toolbarHandler.setVisible(true);

		if (_videoMode.mode == GFX_NORMAL && ConfMan.hasKey("landscape") && ConfMan.getInt("landscape")) {
			setGraphicsMode(GFX_NORMAL);
			hotswapGFXMode();
		}

		if (_hasSmartphoneResolution)
			panel->setVisible(false);

		_saveToolbarState = true;
	}

	if (ConfMan.hasKey("no_doubletap_rightclick"))
		_noDoubleTapRMB = ConfMan.getBool("no_doubletap_rightclick");

	if (ConfMan.hasKey("no_doubletap_paneltoggle"))
		_noDoubleTapPT = ConfMan.getBool("no_doubletap_paneltoggle");
}

void WINCESdlGraphicsManager::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	static bool old_overlayVisible = false;
	int numRectsOut = 0;
	int16 routx, routy, routw, routh, stretch, shakestretch;

	assert(_hwscreen != NULL);

	// bail if the application is minimized, be nice to OS
	if (!_hasfocus) {
		Sleep(20);
		return;
	}

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, 0, _videoMode.screenWidth *_scaleFactorXm / _scaleFactorXd, _newShakePos *_scaleFactorYm / _scaleFactorYd};
		if (_videoMode.aspectRatioCorrection)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;
		SDL_FillRect(_hwscreen, &blackrect, 0);
		_currentShakePos = _newShakePos;
		_forceFull = true;
	}

	// Make sure the mouse is drawn, if it should be drawn.
	drawMouse();

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart, _paletteDirtyStart, _paletteDirtyEnd - _paletteDirtyStart);
		_paletteDirtyEnd = 0;
		_forceFull = true;
	}

	if (!_overlayVisible) {
		origSurf = _screen;
		srcSurf = _tmpscreen;
	} else {
		origSurf = _overlayscreen;
		srcSurf = _tmpscreen2;
	}

	if (old_overlayVisible != _overlayVisible) {
		old_overlayVisible = _overlayVisible;
		update_scalers();
	}

	// Force a full redraw if requested
	if (_forceFull) {
		_numDirtyRects = 1;

		_dirtyRectList[0].x = 0;
		if (!_zoomDown)
			_dirtyRectList[0].y = 0;
		else
			_dirtyRectList[0].y = _videoMode.screenHeight / 2;
		_dirtyRectList[0].w = _videoMode.screenWidth;
		if (!_zoomUp && !_zoomDown)
			_dirtyRectList[0].h = _videoMode.screenHeight;
		else
			_dirtyRectList[0].h = _videoMode.screenHeight / 2;

		_toolbarHandler.forceRedraw();
	}

	// Only draw anything if necessary
	if (_numDirtyRects > 0) {

		SDL_Rect *r, *rout;
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *last_rect = _dirtyRectList + _numDirtyRects;
		bool toolbarVisible = _toolbarHandler.visible();
		int toolbarOffset = _toolbarHandler.getOffset();

		for (r = _dirtyRectList; r != last_rect; ++r) {
			dst = *r;
			dst.x++;    // Shift rect by one since 2xSai needs to access the data around
			dst.y++;    // any pixel to scale it, and we want to avoid mem access crashes.
			// NOTE: This is also known as BLACK MAGIC, copied from the sdl backend
			if (SDL_BlitSurface(origSurf, r, srcSurf, &dst) != 0)
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

		SDL_LockSurface(srcSurf);
		SDL_LockSurface(_hwscreen);

		srcPitch = srcSurf->pitch;
		dstPitch = _hwscreen->pitch;

		for (r = _dirtyRectList, rout = _dirtyRectOut; r != last_rect; ++r) {

			// always clamp to enclosing, downsampled-grid-aligned rect in the downscaled image
			if (_scaleFactorXd != 1) {
				stretch = r->x % _scaleFactorXd;
				r->x -= stretch;
				r->w += stretch;
				r->w = (r->x + r->w + _scaleFactorXd - 1) / _scaleFactorXd * _scaleFactorXd - r->x;
			}
			if (_scaleFactorYd != 1) {
				stretch = r->y % _scaleFactorYd;
				r->y -= stretch;
				r->h += stretch;
				r->h = (r->y + r->h + _scaleFactorYd - 1) / _scaleFactorYd * _scaleFactorYd - r->y;
			}

			// transform
			shakestretch = _currentShakePos * _scaleFactorYm / _scaleFactorYd;
			routx = r->x * _scaleFactorXm / _scaleFactorXd;                 // locate position in scaled screen
			routy = r->y * _scaleFactorYm / _scaleFactorYd + shakestretch;  // adjust for shake offset
			routw = r->w * _scaleFactorXm / _scaleFactorXd;
			routh = r->h * _scaleFactorYm / _scaleFactorYd - shakestretch;

			// clipping destination rectangle inside device screen (more strict, also more tricky but more stable)
			// note that all current scalers do not make dst rect exceed left/right, unless chosen badly (FIXME)
			if (_zoomDown)  routy -= 240;           // adjust for zoom position
			if (routy + routh < 0)  continue;
			if (routy < 0) {
				routh += routy;
				r->y -= routy * _scaleFactorYd / _scaleFactorYm;
				routy = 0;
				r->h = routh * _scaleFactorYd / _scaleFactorYm;
			}
			if (_orientationLandscape) {
				if (routy > OSystem_WINCE3::getScreenWidth())   continue;
				if (routy + routh > OSystem_WINCE3::getScreenWidth()) {
					routh = OSystem_WINCE3::getScreenWidth() - routy;
					r->h = routh * _scaleFactorYd / _scaleFactorYm;
				}
			} else {
				if (routy > OSystem_WINCE3::getScreenHeight())  continue;
				if (routy + routh > OSystem_WINCE3::getScreenHeight()) {
					routh = OSystem_WINCE3::getScreenHeight() - routy;
					r->h = routh * _scaleFactorYd / _scaleFactorYm;
				}
			}

			// check if the toolbar is overwritten
			if (toolbarVisible && r->y + r->h >= toolbarOffset)
				_toolbarHandler.forceRedraw();

			// blit it (with added voodoo from the sdl backend, shifting the source rect again)
			_scalerProc((byte *)srcSurf->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
			            (byte *)_hwscreen->pixels + routx * 2 + routy * dstPitch, dstPitch,
			            r->w, r->h - _currentShakePos);

			// add this rect to output
			rout->x = routx;
			rout->y = routy - shakestretch;
			rout->w = routw;
			rout->h = routh + shakestretch;
			numRectsOut++;
			rout++;

		}
		SDL_UnlockSurface(srcSurf);
		SDL_UnlockSurface(_hwscreen);
	}
	// Add the toolbar if needed
	SDL_Rect toolbar_rect[1];
	if (_panelVisible && _toolbarHandler.draw(_toolbarLow, &toolbar_rect[0])) {
		// It can be drawn, scale it
		uint32 srcPitch, dstPitch;
		SDL_Surface *toolbarSurface;
		ScalerProc *toolbarScaler;

		if (_videoMode.screenHeight > 240) {
			if (!_toolbarHighDrawn) {
				// Resize the toolbar
				SDL_LockSurface(_toolbarLow);
				SDL_LockSurface(_toolbarHigh);
				Normal2x((byte *)_toolbarLow->pixels, _toolbarLow->pitch, (byte *)_toolbarHigh->pixels, _toolbarHigh->pitch, toolbar_rect[0].w, toolbar_rect[0].h);
				SDL_UnlockSurface(_toolbarHigh);
				SDL_UnlockSurface(_toolbarLow);
				_toolbarHighDrawn = true;
			}
			toolbar_rect[0].w *= 2;
			toolbar_rect[0].h *= 2;
			toolbarSurface = _toolbarHigh;
		} else
			toolbarSurface = _toolbarLow;

		drawToolbarMouse(toolbarSurface, true);     // draw toolbar mouse if applicable

		// Apply the appropriate scaler
		SDL_LockSurface(toolbarSurface);
		SDL_LockSurface(_hwscreen);
		srcPitch = toolbarSurface->pitch;
		dstPitch = _hwscreen->pitch;

		toolbarScaler = _scalerProc;
		if (_videoMode.scaleFactor == 2)
			toolbarScaler = Normal2x;
		else if (_videoMode.scaleFactor == 3)
			toolbarScaler = Normal3x;
		toolbarScaler((byte *)toolbarSurface->pixels, srcPitch,
		              (byte *)_hwscreen->pixels + (_toolbarHandler.getOffset() * _scaleFactorYm / _scaleFactorYd * dstPitch),
		              dstPitch, toolbar_rect[0].w, toolbar_rect[0].h);
		SDL_UnlockSurface(toolbarSurface);
		SDL_UnlockSurface(_hwscreen);

		// And blit it
		toolbar_rect[0].y = _toolbarHandler.getOffset();
		toolbar_rect[0].x = toolbar_rect[0].x * _scaleFactorXm / _scaleFactorXd;
		toolbar_rect[0].y = toolbar_rect[0].y * _scaleFactorYm / _scaleFactorYd;
		toolbar_rect[0].w = toolbar_rect[0].w * _scaleFactorXm / _scaleFactorXd;
		toolbar_rect[0].h = toolbar_rect[0].h * _scaleFactorYm / _scaleFactorYd;

		SDL_UpdateRects(_hwscreen, 1, toolbar_rect);

		drawToolbarMouse(toolbarSurface, false);    // undraw toolbar mouse
	}

	// Finally, blit all our changes to the screen
	if (numRectsOut > 0)
		SDL_UpdateRects(_hwscreen, numRectsOut, _dirtyRectOut);

	_numDirtyRects = 0;
	_forceFull = false;
}

bool WINCESdlGraphicsManager::setGraphicsMode(int mode) {

	Common::StackLock lock(_graphicsMutex);
	int oldScaleFactorXm = _scaleFactorXm;
	int oldScaleFactorXd = _scaleFactorXd;
	int oldScaleFactorYm = _scaleFactorYm;
	int oldScaleFactorYd = _scaleFactorYd;

	_scaleFactorXm = -1;
	_scaleFactorXd = -1;
	_scaleFactorYm = -1;
	_scaleFactorYd = -1;

	if (ConfMan.hasKey("landscape"))
		if (ConfMan.get("landscape")[0] > 57) {
			_newOrientation = _orientationLandscape = ConfMan.getBool("landscape");
			ConfMan.setInt("landscape", _orientationLandscape);
		} else
			_newOrientation = _orientationLandscape = ConfMan.getInt("landscape");
	else
		_newOrientation = _orientationLandscape = 0;

	if (OSystem_WINCE3::isOzone() && (OSystem_WINCE3::getScreenWidth() >= 640 || OSystem_WINCE3::getScreenHeight() >= 640) && mode)
		_scaleFactorXm = -1;

	if (CEDevice::hasPocketPCResolution() && !CEDevice::hasWideResolution() && _orientationLandscape)
		_videoMode.mode = GFX_NORMAL;
	else
		_videoMode.mode = mode;

	if (_scaleFactorXm < 0) {
		/* Standard scalers, from the SDL backend */
		switch (_videoMode.mode) {
		case GFX_NORMAL:
			_videoMode.scaleFactor = 1;
			_scalerProc = Normal1x;
			break;
		case GFX_DOUBLESIZE:
			_videoMode.scaleFactor = 2;
			_scalerProc = Normal2x;
			break;
		case GFX_TRIPLESIZE:
			_videoMode.scaleFactor = 3;
			_scalerProc = Normal3x;
			break;
		case GFX_2XSAI:
			_videoMode.scaleFactor = 2;
			_scalerProc = _2xSaI;
			break;
		case GFX_SUPER2XSAI:
			_videoMode.scaleFactor = 2;
			_scalerProc = Super2xSaI;
			break;
		case GFX_SUPEREAGLE:
			_videoMode.scaleFactor = 2;
			_scalerProc = SuperEagle;
			break;
		case GFX_ADVMAME2X:
			_videoMode.scaleFactor = 2;
			_scalerProc = AdvMame2x;
			break;
		case GFX_ADVMAME3X:
			_videoMode.scaleFactor = 3;
			_scalerProc = AdvMame3x;
			break;
#ifdef USE_HQ_SCALERS
		case GFX_HQ2X:
			_videoMode.scaleFactor = 2;
			_scalerProc = HQ2x;
			break;
		case GFX_HQ3X:
			_videoMode.scaleFactor = 3;
			_scalerProc = HQ3x;
			break;
#endif
		case GFX_TV2X:
			_videoMode.scaleFactor = 2;
			_scalerProc = TV2x;
			break;
		case GFX_DOTMATRIX:
			_videoMode.scaleFactor = 2;
			_scalerProc = DotMatrix;
			break;

		default:
			error("unknown gfx mode %d", mode);
		}
	}

	// Check if the scaler can be accepted, if not get back to normal scaler
	if (_videoMode.scaleFactor && ((_videoMode.scaleFactor * _videoMode.screenWidth > OSystem_WINCE3::getScreenWidth() && _videoMode.scaleFactor * _videoMode.screenWidth > OSystem_WINCE3::getScreenHeight())
	                               || (_videoMode.scaleFactor * _videoMode.screenHeight > OSystem_WINCE3::getScreenWidth() && _videoMode.scaleFactor * _videoMode.screenHeight > OSystem_WINCE3::getScreenHeight()))) {
		_videoMode.scaleFactor = 1;
		_scalerProc = Normal1x;
	}

	// Common scaler system was used
	if (_scaleFactorXm < 0) {
		_scaleFactorXm = _videoMode.scaleFactor;
		_scaleFactorXd = 1;
		_scaleFactorYm = _videoMode.scaleFactor;
		_scaleFactorYd = 1;
	}

	_forceFull = true;

	if (oldScaleFactorXm != _scaleFactorXm ||
	        oldScaleFactorXd != _scaleFactorXd ||
	        oldScaleFactorYm != _scaleFactorYm ||
	        oldScaleFactorYd != _scaleFactorYd) {
		_scalersChanged = true;
	} else
		_scalersChanged = false;


	return true;

}

bool WINCESdlGraphicsManager::loadGFXMode() {
	int displayWidth;
	int displayHeight;
	unsigned int flags = SDL_FULLSCREEN | SDL_SWSURFACE;

	_videoMode.fullscreen = true; // forced
	_forceFull = true;

	_tmpscreen = NULL;

	// Recompute scalers if necessary
	update_scalers();

	// Create the surface that contains the 8 bit game data
	_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth, _videoMode.screenHeight, 8, 0, 0, 0, 0);
	if (_screen == NULL)
		error("_screen failed (%s)", SDL_GetError());

	// Create the surface that contains the scaled graphics in 16 bit mode
	// Always use full screen mode to have a "clean screen"
	if (!_videoMode.aspectRatioCorrection) {
		displayWidth = _videoMode.screenWidth * _scaleFactorXm / _scaleFactorXd;
		displayHeight = _videoMode.screenHeight * _scaleFactorYm / _scaleFactorYd;
	} else {
		displayWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
		displayHeight = _videoMode.screenHeight * _videoMode.scaleFactor;
	}

	switch (_orientationLandscape) {
	case 1:
		flags |= SDL_LANDSCVIDEO;
		break;
	case 2:
		flags |= SDL_INVLNDVIDEO;
		break;
	default:
		flags |= SDL_PORTRTVIDEO;
	}
	_hwscreen = SDL_SetVideoMode(displayWidth, displayHeight, 16, flags);

	if (_hwscreen == NULL) {
		warning("SDL_SetVideoMode says we can't switch to that mode (%s)", SDL_GetError());
		g_system->quit();
	}

	// see what orientation sdl finally accepted
	if (_hwscreen->flags & SDL_PORTRTVIDEO)
		_orientationLandscape = _newOrientation = 0;
	else if (_hwscreen->flags & SDL_LANDSCVIDEO)
		_orientationLandscape = _newOrientation = 1;
	else
		_orientationLandscape = _newOrientation = 2;

	// Create the surface used for the graphics in 16 bit before scaling, and also the overlay
	// Distinguish 555 and 565 mode
	if (_hwscreen->format->Rmask == 0x7C00)
		InitScalers(555);
	else
		InitScalers(565);
	_overlayFormat.bytesPerPixel = _hwscreen->format->BytesPerPixel;
	_overlayFormat.rLoss = _hwscreen->format->Rloss;
	_overlayFormat.gLoss = _hwscreen->format->Gloss;
	_overlayFormat.bLoss = _hwscreen->format->Bloss;
	_overlayFormat.aLoss = _hwscreen->format->Aloss;
	_overlayFormat.rShift = _hwscreen->format->Rshift;
	_overlayFormat.gShift = _hwscreen->format->Gshift;
	_overlayFormat.bShift = _hwscreen->format->Bshift;
	_overlayFormat.aShift = _hwscreen->format->Ashift;

	// Need some extra bytes around when using 2xSaI
	_tmpscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.screenWidth + 3, _videoMode.screenHeight + 3, 16, _hwscreen->format->Rmask, _hwscreen->format->Gmask, _hwscreen->format->Bmask, _hwscreen->format->Amask);

	if (_tmpscreen == NULL)
		error("_tmpscreen creation failed (%s)", SDL_GetError());

	// Overlay
	if (CEDevice::hasDesktopResolution()) {
		_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth * _scaleFactorXm / _scaleFactorXd, _videoMode.overlayHeight * _scaleFactorYm / _scaleFactorYd, 16, 0, 0, 0, 0);
		if (_overlayscreen == NULL)
			error("_overlayscreen failed (%s)", SDL_GetError());
		_tmpscreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth * _scaleFactorXm / _scaleFactorXd + 3, _videoMode.overlayHeight * _scaleFactorYm / _scaleFactorYd + 3, 16, 0, 0, 0, 0);
		if (_tmpscreen2 == NULL)
			error("_tmpscreen2 failed (%s)", SDL_GetError());
	} else {
		_overlayscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth, _videoMode.overlayHeight, 16, 0, 0, 0, 0);
		if (_overlayscreen == NULL)
			error("_overlayscreen failed (%s)", SDL_GetError());
		_tmpscreen2 = SDL_CreateRGBSurface(SDL_SWSURFACE, _videoMode.overlayWidth + 3, _videoMode.overlayHeight + 3, 16, 0, 0, 0, 0);
		if (_tmpscreen2 == NULL)
			error("_tmpscreen2 failed (%s)", SDL_GetError());
	}

	// Toolbar
	_toolbarHighDrawn = false;
	uint16 *toolbar_screen = (uint16 *)calloc(320 * 40, sizeof(uint16));    // *not* leaking memory here
	_toolbarLow = SDL_CreateRGBSurfaceFrom(toolbar_screen, 320, 40, 16, 320 * 2, _hwscreen->format->Rmask, _hwscreen->format->Gmask, _hwscreen->format->Bmask, _hwscreen->format->Amask);

	if (_toolbarLow == NULL)
		error("_toolbarLow failed (%s)", SDL_GetError());

	if (_videoMode.screenHeight > 240) {
		uint16 *toolbar_screen_high = (uint16 *)calloc(640 * 80, sizeof(uint16));
		_toolbarHigh = SDL_CreateRGBSurfaceFrom(toolbar_screen_high, 640, 80, 16, 640 * 2, _hwscreen->format->Rmask, _hwscreen->format->Gmask, _hwscreen->format->Bmask, _hwscreen->format->Amask);

		if (_toolbarHigh == NULL)
			error("_toolbarHigh failed (%s)", SDL_GetError());
	} else
		_toolbarHigh = NULL;

	// keyboard cursor control, some other better place for it?
	_eventSource->resetKeyboadEmulation(_videoMode.screenWidth * _scaleFactorXm / _scaleFactorXd - 1, _videoMode.screenHeight * _scaleFactorXm / _scaleFactorXd - 1);

	return true;
}

void WINCESdlGraphicsManager::unloadGFXMode() {
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = NULL;
	}

	if (_hwscreen) {
		SDL_FreeSurface(_hwscreen);
		_hwscreen = NULL;
	}

	if (_tmpscreen) {
		SDL_FreeSurface(_tmpscreen);
		_tmpscreen = NULL;
	}
}

bool WINCESdlGraphicsManager::hotswapGFXMode() {
	if (!_screen)
		return false;

	// Keep around the old _screen & _tmpscreen so we can restore the screen data
	// after the mode switch. (also for the overlay)
	SDL_Surface *old_screen = _screen;
	SDL_Surface *old_tmpscreen = _tmpscreen;
	SDL_Surface *old_overlayscreen = _overlayscreen;
	SDL_Surface *old_tmpscreen2 = _tmpscreen2;

	// Release the HW screen surface
	SDL_FreeSurface(_hwscreen);

	// Release toolbars
	free(_toolbarLow->pixels);
	SDL_FreeSurface(_toolbarLow);
	if (_toolbarHigh) {
		free(_toolbarHigh->pixels);
		SDL_FreeSurface(_toolbarHigh);
	}

	// Setup the new GFX mode
	if (!loadGFXMode()) {
		unloadGFXMode();

		_screen = old_screen;
		_overlayscreen = old_overlayscreen;

		return false;
	}

	// reset palette
	SDL_SetColors(_screen, _currentPalette, 0, 256);

	// Restore old screen content
	SDL_BlitSurface(old_screen, NULL, _screen, NULL);
	SDL_BlitSurface(old_tmpscreen, NULL, _tmpscreen, NULL);
	if (_overlayVisible) {
		SDL_BlitSurface(old_overlayscreen, NULL, _overlayscreen, NULL);
		SDL_BlitSurface(old_tmpscreen2, NULL, _tmpscreen2, NULL);
	}

	// Free the old surfaces
	SDL_FreeSurface(old_screen);
	SDL_FreeSurface(old_tmpscreen);
	SDL_FreeSurface(old_overlayscreen);
	SDL_FreeSurface(old_tmpscreen2);

	// Blit everything back to the screen
	_toolbarHighDrawn = false;
	internUpdateScreen();

	// Make sure that a Common::EVENT_SCREEN_CHANGED gets sent later -> FIXME this crashes when no game has been loaded.
//	_modeChanged = true;

	return true;
}

bool WINCESdlGraphicsManager::saveScreenshot(const char *filename) {
	assert(_hwscreen != NULL);

	Common::StackLock lock(_graphicsMutex); // Lock the mutex until this function ends
	SDL_SaveBMP(_hwscreen, filename);
	return true;
}

void WINCESdlGraphicsManager::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	assert(_transactionMode == kTransactionNone);

	if (_overlayscreen == NULL)
		return;

	const byte *src = (const byte *)buf;

	// Clip the coordinates
	if (x < 0) {
		w += x;
		src -= x * 2;
		x = 0;
	}

	if (y < 0) {
		h += y;
		src -= y * pitch;
		y = 0;
	}

	if (w > _videoMode.overlayWidth - x) {
		w = _videoMode.overlayWidth - x;
	}

	if (h > _videoMode.overlayHeight - y) {
		h = _videoMode.overlayHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	// Mark the modified region as dirty
	addDirtyRect(x, y, w, h);

	undrawMouse();

	if (SDL_LockSurface(_overlayscreen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_overlayscreen->pixels + y * _overlayscreen->pitch + x * 2;
	do {
		memcpy(dst, src, w * 2);
		dst += _overlayscreen->pitch;
		src += pitch;
	} while (--h);

	SDL_UnlockSurface(_overlayscreen);
}

void WINCESdlGraphicsManager::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	assert(_transactionMode == kTransactionNone);
	assert(buf);

	if (_screen == NULL)
		return;

	Common::StackLock lock(_graphicsMutex); // Lock the mutex until this function ends

	const byte *src = (const byte *)buf;
	/* Clip the coordinates */
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}

	if (y < 0) {
		h += y;
		src -= y * pitch;
		y = 0;
	}

	if (w > _videoMode.screenWidth - x) {
		w = _videoMode.screenWidth - x;
	}

	if (h > _videoMode.screenHeight - y) {
		h = _videoMode.screenHeight - y;
	}

	if (w <= 0 || h <= 0)
		return;

	addDirtyRect(x, y, w, h);

	undrawMouse();

	// Try to lock the screen surface
	if (SDL_LockSurface(_screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	byte *dst = (byte *)_screen->pixels + y * _videoMode.screenWidth + x;

	if (_videoMode.screenWidth == pitch && pitch == w) {
		memcpy(dst, src, h * w);
	} else {
		do {
			memcpy(dst, src, w);
			src += pitch;
			dst += _videoMode.screenWidth;
		} while (--h);
	}

	// Unlock the screen surface
	SDL_UnlockSurface(_screen);
}

void WINCESdlGraphicsManager::setMouseCursor(const void *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {

	undrawMouse();
	if (w == 0 || h == 0)
		return;

	_mouseCurState.w = w;
	_mouseCurState.h = h;

	_mouseHotspotX = hotspot_x;
	_mouseHotspotY = hotspot_y;

	_mouseKeyColor = keycolor;

	free(_mouseData);

	_mouseData = (byte *) malloc(w * h);
	memcpy(_mouseData, buf, w * h);

	if (w > _mouseBackupDim || h > _mouseBackupDim) {
		// mouse has been undrawn, adjust sprite backup area
		free(_mouseBackupOld);
		free(_mouseBackupToolbar);
		uint16 tmp = (w > h) ? w : h;
		_mouseBackupOld = (byte *) malloc(tmp * tmp * 2);   // can hold 8bpp (playfield) or 16bpp (overlay) data
		_mouseBackupToolbar = (uint16 *) malloc(tmp * tmp * 2); // 16 bpp
		_mouseBackupDim = tmp;
	}
}

void WINCESdlGraphicsManager::transformMouseCoordinates(Common::Point &point) {
	/*
	if (!_overlayVisible) {
		point.x = point.x * _scaleFactorXd / _scaleFactorXm;
		point.y = point.y * _scaleFactorYd / _scaleFactorYm;
		point.x /= _videoMode.scaleFactor;
		point.y /= _videoMode.scaleFactor;
		if (_videoMode.aspectRatioCorrection)
			point.y = aspect2Real(point.y);
	}
	*/
}

void WINCESdlGraphicsManager::setMousePos(int x, int y) {
	if (x != _mouseCurState.x || y != _mouseCurState.y) {
		undrawMouse();
		_mouseCurState.x = x;
		_mouseCurState.y = y;
		updateScreen();
	}
}

Graphics::Surface *WINCESdlGraphicsManager::lockScreen() {
	// Make sure mouse pointer is not painted over the playfield at the time of locking
	undrawMouse();
	return SurfaceSdlGraphicsManager::lockScreen();
}

void WINCESdlGraphicsManager::showOverlay() {
	assert(_transactionMode == kTransactionNone);

	if (_overlayVisible)
		return;

	undrawMouse();
	_overlayVisible = true;
	update_scalers();
	clearOverlay();
}

void WINCESdlGraphicsManager::hideOverlay() {
	assert(_transactionMode == kTransactionNone);

	if (!_overlayVisible)
		return;

	undrawMouse();
	_overlayVisible = false;
	clearOverlay();
	_forceFull = true;
}

void WINCESdlGraphicsManager::blitCursor() {
}

void WINCESdlGraphicsManager::drawToolbarMouse(SDL_Surface *surf, bool draw) {

	if (!_mouseData || !_usesEmulatedMouse)
		return;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY - _toolbarHandler.getOffset();
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;
	byte color;
	const byte *src = _mouseData;
	int width;

	// clip
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _mouseCurState.w;
		y = 0;
	}
	if (w > surf->w - x)
		w = surf->w - x;
	if (h > surf->h - y)
		h = surf->h - y;
	if (w <= 0 || h <= 0)
		return;

	if (SDL_LockSurface(surf) == -1)
		error("SDL_LockSurface failed at internDrawToolbarMouse: %s", SDL_GetError());

	uint16 *bak = _mouseBackupToolbar;  // toolbar surfaces are 16bpp
	uint16 *dst;
	dst = (uint16 *)surf->pixels + y * surf->w + x;

	if (draw) {     // blit it
		while (h > 0) {
			width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != _mouseKeyColor)    // transparent color
					*dst = 0xFFFF;
				dst++;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += _mouseBackupDim - w;
			dst += surf->w - w;
			h--;
		}
	} else {        // restore bg
		for (y = 0; y < h; ++y, bak += _mouseBackupDim, dst += surf->w)
			memcpy(dst, bak, w << 1);
	}

	SDL_UnlockSurface(surf);
}

void WINCESdlGraphicsManager::warpMouse(int x, int y) {
	if (_mouseCurState.x != x || _mouseCurState.y != y) {
		SDL_WarpMouse(x * _scaleFactorXm / _scaleFactorXd, y * _scaleFactorYm / _scaleFactorYd);

		// SDL_WarpMouse() generates a mouse movement event, so
		// set_mouse_pos() would be called eventually. However, the
		// cannon script in CoMI calls this function twice each time
		// the cannon is reloaded. Unless we update the mouse position
		// immediately the second call is ignored, causing the cannon
		// to change its aim.

		setMousePos(x, y);
	}
}

void WINCESdlGraphicsManager::unlockScreen() {
	SurfaceSdlGraphicsManager::unlockScreen();
}

void WINCESdlGraphicsManager::internDrawMouse() {
	if (!_mouseNeedsRedraw || !_mouseVisible || !_mouseData)
		return;

	int x = _mouseCurState.x - _mouseHotspotX;
	int y = _mouseCurState.y - _mouseHotspotY;
	int w = _mouseCurState.w;
	int h = _mouseCurState.h;
	byte color;
	const byte *src = _mouseData;       // Image representing the mouse
	int width;

	// clip the mouse rect, and adjust the src pointer accordingly
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		src -= y * _mouseCurState.w;
		y = 0;
	}

	if (w > _videoMode.screenWidth - x)
		w = _videoMode.screenWidth - x;
	if (h > _videoMode.screenHeight - y)
		h = _videoMode.screenHeight - y;

	// Quick check to see if anything has to be drawn at all
	if (w <= 0 || h <= 0)
		return;

	// Draw the mouse cursor; backup the covered area in "bak"
	if (SDL_LockSurface(_overlayVisible ? _overlayscreen : _screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	// Mark as dirty
	addDirtyRect(x, y, w, h);

	if (!_overlayVisible) {
		byte *bak = _mouseBackupOld;        // Surface used to backup the area obscured by the mouse
		byte *dst;                  // Surface we are drawing into

		dst = (byte *)_screen->pixels + y * _videoMode.screenWidth + x;
		while (h > 0) {
			width = w;
			while (width > 0) {
				*bak++ = *dst;
				color = *src++;
				if (color != _mouseKeyColor)    // transparent, don't draw
					*dst = color;
				dst++;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += _mouseBackupDim - w;
			dst += _videoMode.screenWidth - w;
			h--;
		}

	} else {
		uint16 *bak = (uint16 *)_mouseBackupOld;    // Surface used to backup the area obscured by the mouse
		byte *dst;                  // Surface we are drawing into

		dst = (byte *)_overlayscreen->pixels + (y + 1) * _overlayscreen->pitch + (x + 1) * 2;
		while (h > 0) {
			width = w;
			while (width > 0) {
				*bak++ = *(uint16 *)dst;
				color = *src++;
				if (color != 0xFF)  // 0xFF = transparent, don't draw
					*(uint16 *)dst = SDL_MapRGB(_overlayscreen->format, _currentPalette[color].r, _currentPalette[color].g, _currentPalette[color].b);
				dst += 2;
				width--;
			}
			src += _mouseCurState.w - w;
			bak += _mouseBackupDim - w;
			dst += _overlayscreen->pitch - w * 2;
			h--;
		}
	}

	SDL_UnlockSurface(_overlayVisible ? _overlayscreen : _screen);

	// Finally, set the flag to indicate the mouse has been drawn
	_mouseNeedsRedraw = false;
}

void WINCESdlGraphicsManager::undrawMouse() {
	assert(_transactionMode == kTransactionNone);

	if (_mouseNeedsRedraw)
		return;

	int old_mouse_x = _mouseCurState.x - _mouseHotspotX;
	int old_mouse_y = _mouseCurState.y - _mouseHotspotY;
	int old_mouse_w = _mouseCurState.w;
	int old_mouse_h = _mouseCurState.h;

	// clip the mouse rect, and adjust the src pointer accordingly
	if (old_mouse_x < 0) {
		old_mouse_w += old_mouse_x;
		old_mouse_x = 0;
	}
	if (old_mouse_y < 0) {
		old_mouse_h += old_mouse_y;
		old_mouse_y = 0;
	}

	if (old_mouse_w > _videoMode.screenWidth - old_mouse_x)
		old_mouse_w = _videoMode.screenWidth - old_mouse_x;
	if (old_mouse_h > _videoMode.screenHeight - old_mouse_y)
		old_mouse_h = _videoMode.screenHeight - old_mouse_y;

	// Quick check to see if anything has to be drawn at all
	if (old_mouse_w <= 0 || old_mouse_h <= 0)
		return;


	if (SDL_LockSurface(_overlayVisible ? _overlayscreen : _screen) == -1)
		error("SDL_LockSurface failed: %s", SDL_GetError());

	int y;
	if (!_overlayVisible) {
		byte *dst, *bak = _mouseBackupOld;

		// No need to do clipping here, since drawMouse() did that already
		dst = (byte *)_screen->pixels + old_mouse_y * _videoMode.screenWidth + old_mouse_x;
		for (y = 0; y < old_mouse_h; ++y, bak += _mouseBackupDim, dst += _videoMode.screenWidth)
			memcpy(dst, bak, old_mouse_w);
	} else {
		byte *dst;
		uint16 *bak = (uint16 *)_mouseBackupOld;

		// No need to do clipping here, since drawMouse() did that already
		dst = (byte *)_overlayscreen->pixels + (old_mouse_y + 1) * _overlayscreen->pitch + (old_mouse_x + 1) * 2;
		for (y = 0; y < old_mouse_h; ++y, bak += _mouseBackupDim, dst += _overlayscreen->pitch)
			memcpy(dst, bak, old_mouse_w << 1);
	}

	addDirtyRect(old_mouse_x, old_mouse_y, old_mouse_w, old_mouse_h);

	SDL_UnlockSurface(_overlayVisible ? _overlayscreen : _screen);

	_mouseNeedsRedraw = true;
}

void WINCESdlGraphicsManager::drawMouse() {
	if (!(_toolbarHandler.visible() && _mouseCurState.y >= _toolbarHandler.getOffset() && !_usesEmulatedMouse) && !_forceHideMouse)
		internDrawMouse();
}

bool WINCESdlGraphicsManager::showMouse(bool visible) {
	if (_mouseVisible == visible)
		return visible;

	if (visible == false)
		undrawMouse();

	bool last = _mouseVisible;
	_mouseVisible = visible;
	_mouseNeedsRedraw = true;

	return last;
}

void WINCESdlGraphicsManager::addDirtyRect(int x, int y, int w, int h, bool mouseRect) {

	if (_forceFull || _paletteDirtyEnd)
		return;

	SurfaceSdlGraphicsManager::addDirtyRect(x, y, w, h, false);
}

void WINCESdlGraphicsManager::swap_panel_visibility() {
	//if (!_forcePanelInvisible && !_panelStateForced) {
	if (_zoomDown || _zoomUp)
		return;

	if (_panelVisible) {
		if (_toolbarHandler.activeName() == NAME_PANEL_KEYBOARD)
			_panelVisible = !_panelVisible;
		else
			_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);
	} else {
		_toolbarHandler.setActive(NAME_MAIN_PANEL);
		_panelVisible = !_panelVisible;
	}
	_toolbarHandler.setVisible(_panelVisible);
	_toolbarHighDrawn = false;

	if (_videoMode.screenHeight > 240)
		addDirtyRect(0, 400, 640, 80);
	else
		addDirtyRect(0, 200, 320, 40);

	if (_toolbarHandler.activeName() == NAME_PANEL_KEYBOARD && _panelVisible)
		internUpdateScreen();
	else {
		update_scalers();
		hotswapGFXMode();
	}
	//}
}

void WINCESdlGraphicsManager::swap_panel() {
	_toolbarHighDrawn = false;
	//if (!_panelStateForced) {
	if (_toolbarHandler.activeName() == NAME_PANEL_KEYBOARD && _panelVisible)
		_toolbarHandler.setActive(NAME_MAIN_PANEL);
	else
		_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);

	if (_videoMode.screenHeight > 240)
		addDirtyRect(0, 400, 640, 80);
	else
		addDirtyRect(0, 200, 320, 40);

	_toolbarHandler.setVisible(true);
	if (!_panelVisible) {
		_panelVisible = true;
		update_scalers();
		hotswapGFXMode();
	}
	//}
}

void WINCESdlGraphicsManager::swap_smartphone_keyboard() {
	_toolbarHandler.setActive(NAME_PANEL_KEYBOARD);
	_panelVisible = !_panelVisible;
	_toolbarHandler.setVisible(_panelVisible);
	if (_videoMode.screenHeight > 240)
		addDirtyRect(0, 0, 640, 80);
	else
		addDirtyRect(0, 0, 320, 40);
	internUpdateScreen();
}

void WINCESdlGraphicsManager::swap_zoom_up() {
	if (_zoomUp) {
		// restore visibility
		_toolbarHandler.setVisible(_saveToolbarZoom);
		// restore scaler
		_scaleFactorYd = 2;
		_scalerProc = DownscaleAllByHalf;
		_zoomUp = false;
		_zoomDown = false;
	} else {
		// only active if running on a PocketPC
		if (_scalerProc != DownscaleAllByHalf && _scalerProc != DownscaleHorizByHalf)
			return;
		if (_scalerProc == DownscaleAllByHalf) {
			_saveToolbarZoom = _toolbarHandler.visible();
			_toolbarHandler.setVisible(false);
			// set zoom scaler
			_scaleFactorYd = 1;
			_scalerProc = DownscaleHorizByHalf;
		}

		_zoomDown = false;
		_zoomUp = true;
	}
	// redraw whole screen
	addDirtyRect(0, 0, 640, 480);
	internUpdateScreen();
}

void WINCESdlGraphicsManager::swap_zoom_down() {
	if (_zoomDown) {
		// restore visibility
		_toolbarHandler.setVisible(_saveToolbarZoom);
		// restore scaler
		_scaleFactorYd = 2;
		_scalerProc = DownscaleAllByHalf;
		_zoomDown = false;
		_zoomUp = false;
	} else {
		// only active if running on a PocketPC
		if (_scalerProc != DownscaleAllByHalf && _scalerProc != DownscaleHorizByHalf)
			return;
		if (_scalerProc == DownscaleAllByHalf) {
			_saveToolbarZoom = _toolbarHandler.visible();
			_toolbarHandler.setVisible(false);
			// set zoom scaler
			_scaleFactorYd = 1;
			_scalerProc = DownscaleHorizByHalf;
		}

		_zoomUp = false;
		_zoomDown = true;
	}
	// redraw whole screen
	addDirtyRect(0, 0, 640, 480);
	internUpdateScreen();
}

void WINCESdlGraphicsManager::swap_mouse_visibility() {
	_forceHideMouse = !_forceHideMouse;
	if (_forceHideMouse)
		undrawMouse();
}

void WINCESdlGraphicsManager::init_panel() {
	_panelVisible = true;
	if (_panelInitialized) {
		_toolbarHandler.setVisible(true);
		_toolbarHandler.setActive(NAME_MAIN_PANEL);
	}
}

void WINCESdlGraphicsManager::reset_panel() {
	_panelVisible = false;
	_toolbarHandler.setVisible(false);
}

// Smartphone actions
void WINCESdlGraphicsManager::initZones() {
	int i;

	_currentZone = 0;
	for (i = 0; i < TOTAL_ZONES; i++) {
		_mouseXZone[i] = (_zones[i].x + (_zones[i].width / 2)) * _scaleFactorXm / _scaleFactorXd;
		_mouseYZone[i] = (_zones[i].y + (_zones[i].height / 2)) * _scaleFactorYm / _scaleFactorYd;
	}
}

void WINCESdlGraphicsManager::smartphone_rotate_display() {
	_orientationLandscape = _newOrientation = _orientationLandscape == 1 ? 2 : 1;
	ConfMan.setInt("landscape", _orientationLandscape);
	ConfMan.flushToDisk();
	hotswapGFXMode();
}

void WINCESdlGraphicsManager::create_toolbar() {
	CEGUI::PanelKeyboard *keyboard;

	// Add the keyboard
	keyboard = new CEGUI::PanelKeyboard(PANEL_KEYBOARD);
	_toolbarHandler.add(NAME_PANEL_KEYBOARD, *keyboard);
	_toolbarHandler.setVisible(false);
}

void WINCESdlGraphicsManager::swap_freeLook() {
	_freeLook = !_freeLook;
}

bool WINCESdlGraphicsManager::getFreeLookState() {
	return _freeLook;
}

WINCESdlGraphicsManager::zoneDesc WINCESdlGraphicsManager::_zones[TOTAL_ZONES] = {
	{ 0, 0, 320, 145 },
	{ 0, 145, 150, 55 },
	{ 150, 145, 170, 55 }
};

#endif /* _WIN32_WCE */
