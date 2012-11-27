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

#if defined(DINGUX)

#include "backends/graphics/dinguxsdl/dinguxsdl-graphics.h"
#include "backends/events/dinguxsdl/dinguxsdl-events.h"
#include "graphics/scaler/aspect.h"
#include "common/mutex.h"
#include "common/textconsole.h"

static const OSystem::GraphicsMode s_supportedGraphicsModes[] = {
	{"1x", "Standard", GFX_NORMAL},
	{0, 0, 0}
};

DINGUXSdlGraphicsManager::DINGUXSdlGraphicsManager(SdlEventSource *boss)
	: SurfaceSdlGraphicsManager(boss) {
}

const OSystem::GraphicsMode *DINGUXSdlGraphicsManager::getSupportedGraphicsModes() const {
	return s_supportedGraphicsModes;
}

int DINGUXSdlGraphicsManager::getDefaultGraphicsMode() const {
	return GFX_NORMAL;
}

bool DINGUXSdlGraphicsManager::setGraphicsMode(int mode) {
	Common::StackLock lock(_graphicsMutex);

	assert(_transactionMode == kTransactionActive);

	if (_oldVideoMode.setup && _oldVideoMode.mode == mode)
		return true;

	int newScaleFactor = 1;

	switch (mode) {
	case GFX_NORMAL:
		newScaleFactor = 1;
		break;
	case GFX_HALF:
		newScaleFactor = 1;
		break;
	default:
		warning("unknown gfx mode %d", mode);
		return false;
	}

	_transactionDetails.normal1xScaler = (mode == GFX_NORMAL);
	if (_oldVideoMode.setup && _oldVideoMode.scaleFactor != newScaleFactor)
		_transactionDetails.needHotswap = true;

	_transactionDetails.needUpdatescreen = true;

	_videoMode.mode = mode;
	_videoMode.scaleFactor = newScaleFactor;

	return true;
}

void DINGUXSdlGraphicsManager::setGraphicsModeIntern() {
	Common::StackLock lock(_graphicsMutex);
	ScalerProc *newScalerProc = 0;

	switch (_videoMode.mode) {
	case GFX_NORMAL:
		newScalerProc = Normal1x;
		break;
	case GFX_HALF:
		newScalerProc = DownscaleAllByHalf;
		break;

	default:
		error("Unknown gfx mode %d", _videoMode.mode);
	}

	_scalerProc = newScalerProc;

	if (!_screen || !_hwscreen)
		return;

	// Blit everything to the screen
	_forceFull = true;

	// Even if the old and new scale factors are the same, we may have a
	// different scaler for the cursor now.
	blitCursor();
}

void DINGUXSdlGraphicsManager::initSize(uint w, uint h) {
	assert(_transactionMode == kTransactionActive);

	// Avoid redundant res changes
	if ((int)w == _videoMode.screenWidth && (int)h == _videoMode.screenHeight)
		return;

	_videoMode.screenWidth = w;
	_videoMode.screenHeight = h;
	if (w > 320 || h > 240) {
		setGraphicsMode(GFX_HALF);
		setGraphicsModeIntern();
		_eventSource->toggleMouseGrab();
	}

	_transactionDetails.sizeChanged = true;
}

void DINGUXSdlGraphicsManager::drawMouse() {
	if (!_mouseVisible || !_mouseSurface) {
		_mouseBackup.x = _mouseBackup.y = _mouseBackup.w = _mouseBackup.h = 0;
		return;
	}

	SDL_Rect dst;
	int scale;
	int hotX, hotY;

	if (_videoMode.mode == GFX_HALF && !_overlayVisible) {
		dst.x = _mouseCurState.x / 2;
		dst.y = _mouseCurState.y / 2;
	} else {
		dst.x = _mouseCurState.x;
		dst.y = _mouseCurState.y;
	}

	if (!_overlayVisible) {
		scale = _videoMode.scaleFactor;
		dst.w = _mouseCurState.vW;
		dst.h = _mouseCurState.vH;
		hotX = _mouseCurState.vHotX;
		hotY = _mouseCurState.vHotY;
	} else {
		scale = 1;
		dst.w = _mouseCurState.rW;
		dst.h = _mouseCurState.rH;
		hotX = _mouseCurState.rHotX;
		hotY = _mouseCurState.rHotY;
	}

	// The mouse is undrawn using virtual coordinates, i.e. they may be
	// scaled and aspect-ratio corrected.

	_mouseBackup.x = dst.x - hotX;
	_mouseBackup.y = dst.y - hotY;
	_mouseBackup.w = dst.w;
	_mouseBackup.h = dst.h;

	// We draw the pre-scaled cursor image, so now we need to adjust for
	// scaling, shake position and aspect ratio correction manually.

	if (!_overlayVisible) {
		dst.y += _currentShakePos;
	}

	if (_videoMode.aspectRatioCorrection && !_overlayVisible)
		dst.y = real2Aspect(dst.y);

	dst.x = scale * dst.x - _mouseCurState.rHotX;
	dst.y = scale * dst.y - _mouseCurState.rHotY;
	dst.w = _mouseCurState.rW;
	dst.h = _mouseCurState.rH;

	// Note that SDL_BlitSurface() and addDirtyRect() will both perform any
	// clipping necessary

	if (SDL_BlitSurface(_mouseSurface, NULL, _hwscreen, &dst) != 0)
		error("SDL_BlitSurface failed: %s", SDL_GetError());

	// The screen will be updated using real surface coordinates, i.e.
	// they will not be scaled or aspect-ratio corrected.
	addDirtyRect(dst.x, dst.y, dst.w, dst.h, true);
}

void DINGUXSdlGraphicsManager::undrawMouse() {
	const int x = _mouseBackup.x;
	const int y = _mouseBackup.y;

	// When we switch bigger overlay off mouse jumps. Argh!
	// This is intended to prevent undrawing offscreen mouse
	if (!_overlayVisible && (x >= _videoMode.screenWidth || y >= _videoMode.screenHeight))
		return;

	if (_mouseBackup.w != 0 && _mouseBackup.h != 0) {
		if (_videoMode.mode == GFX_HALF && !_overlayVisible) {
			addDirtyRect(x*2, y*2, _mouseBackup.w*2, _mouseBackup.h*2);
		} else {
			addDirtyRect(x, y, _mouseBackup.w, _mouseBackup.h);
		}
	}
}

void DINGUXSdlGraphicsManager::internUpdateScreen() {
	SDL_Surface *srcSurf, *origSurf;
	int height, width;
	ScalerProc *scalerProc;
	int scale1;

#if defined(DEBUG) && ! defined(_WIN32_WCE) // definitions not available for non-DEBUG here. (needed this to compile in SYMBIAN32 & linux?)
	assert(_hwscreen != NULL);
	assert(_hwscreen->map->sw_data != NULL);
#endif

	// If the shake position changed, fill the dirty area with blackness
	if (_currentShakePos != _newShakePos) {
		SDL_Rect blackrect = {0, 0, _videoMode.screenWidth * _videoMode.scaleFactor, _newShakePos * _videoMode.scaleFactor};

		if (_videoMode.aspectRatioCorrection && !_overlayVisible)
			blackrect.h = real2Aspect(blackrect.h - 1) + 1;

		SDL_FillRect(_hwscreen, &blackrect, 0);

		_currentShakePos = _newShakePos;

		_forceFull = true;
	}

	// Check whether the palette was changed in the meantime and update the
	// screen surface accordingly.
	if (_screen && _paletteDirtyEnd != 0) {
		SDL_SetColors(_screen, _currentPalette + _paletteDirtyStart,
		              _paletteDirtyStart,
		              _paletteDirtyEnd - _paletteDirtyStart);

		_paletteDirtyEnd = 0;

		_forceFull = true;
	}

#ifdef USE_OSD
	// OSD visible (i.e. non-transparent)?
	if (_osdAlpha != SDL_ALPHA_TRANSPARENT) {
		// Updated alpha value
		const int diff = SDL_GetTicks() - _osdFadeStartTime;
		if (diff > 0) {
			if (diff >= kOSDFadeOutDuration) {
				// Back to full transparency
				_osdAlpha = SDL_ALPHA_TRANSPARENT;
			} else {
				// Do a linear fade out...
				const int startAlpha = SDL_ALPHA_TRANSPARENT + kOSDInitialAlpha * (SDL_ALPHA_OPAQUE - SDL_ALPHA_TRANSPARENT) / 100;
				_osdAlpha = startAlpha + diff * (SDL_ALPHA_TRANSPARENT - startAlpha) / kOSDFadeOutDuration;
			}
			SDL_SetAlpha(_osdSurface, SDL_RLEACCEL | SDL_SRCCOLORKEY | SDL_SRCALPHA, _osdAlpha);
			_forceFull = true;
		}
	}
#endif

	if (!_overlayVisible) {
		origSurf = _screen;
		srcSurf = _tmpscreen;
		width = _videoMode.screenWidth;
		height = _videoMode.screenHeight;
		scalerProc = _scalerProc;
		scale1 = _videoMode.scaleFactor;
	} else {
		origSurf = _overlayscreen;
		srcSurf = _tmpscreen2;
		width = _videoMode.overlayWidth;
		height = _videoMode.overlayHeight;
		scalerProc = Normal1x;
		scale1 = 1;
	}

	// Add the area covered by the mouse cursor to the list of dirty rects if
	// we have to redraw the mouse.
	if (_mouseNeedsRedraw)
		undrawMouse();

	// Force a full redraw if requested
	if (_forceFull) {
		_numDirtyRects = 1;
		_dirtyRectList[0].x = 0;
		_dirtyRectList[0].y = 0;
		_dirtyRectList[0].w = width;
		_dirtyRectList[0].h = height;
	}

	// Only draw anything if necessary
	if (_numDirtyRects > 0 || _mouseNeedsRedraw) {
		SDL_Rect *r;
		SDL_Rect dst;
		uint32 srcPitch, dstPitch;
		SDL_Rect *lastRect = _dirtyRectList + _numDirtyRects;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			dst = *r;
			dst.x++;	// Shift rect by one since 2xSai needs to access the data around
			dst.y++;	// any pixel to scale it, and we want to avoid mem access crashes.

			if (SDL_BlitSurface(origSurf, r, srcSurf, &dst) != 0)
				error("SDL_BlitSurface failed: %s", SDL_GetError());
		}

		SDL_LockSurface(srcSurf);
		SDL_LockSurface(_hwscreen);

		srcPitch = srcSurf->pitch;
		dstPitch = _hwscreen->pitch;

		for (r = _dirtyRectList; r != lastRect; ++r) {
			register int dst_y = r->y + _currentShakePos;
			register int dst_h = 0;
			register int dst_w = r->w;
			register int orig_dst_y = 0;
			register int dst_x = r->x;
			register int src_y;
			register int src_x;

			if (dst_y < height) {
				dst_h = r->h;
				if (dst_h > height - dst_y)
					dst_h = height - dst_y;

				orig_dst_y = dst_y;
				src_x = dst_x;
				src_y = dst_y;

				if (_videoMode.aspectRatioCorrection && !_overlayVisible)
					dst_y = real2Aspect(dst_y);

				assert(scalerProc != NULL);

				if ((_videoMode.mode == GFX_HALF) && (scalerProc == DownscaleAllByHalf)) {
					if (dst_x % 2 == 1) {
						dst_x--;
						dst_w++;
					}
					if (dst_y % 2 == 1) {
						dst_y--;
						dst_h++;
					}
					src_x = dst_x;
					src_y = dst_y;
					dst_x = dst_x / 2;
					dst_y = dst_y / 2;

					scalerProc((byte *)srcSurf->pixels + (src_x * 2 + 2) + (src_y + 1) * srcPitch, srcPitch,
					           (byte *)_hwscreen->pixels + dst_x * 2 + dst_y * dstPitch, dstPitch, dst_w, dst_h);

				} else {
					scalerProc((byte *)srcSurf->pixels + (r->x * 2 + 2) + (r->y + 1) * srcPitch, srcPitch,
					           (byte *)_hwscreen->pixels + r->x * 2 + dst_y * dstPitch, dstPitch, r->w, dst_h);
				}
			}

			if (_videoMode.mode == GFX_HALF && scalerProc == DownscaleAllByHalf) {
				r->w = r->w / 2;
				r->h = dst_h / 2;
			} else {
				r->w = r->w;
				r->h = dst_h;
			}

			r->x = dst_x;
			r->y = dst_y;


#ifdef USE_SCALERS
			if (_videoMode.aspectRatioCorrection && orig_dst_y < height && !_overlayVisible)
				r->h = stretch200To240((uint8 *) _hwscreen->pixels, dstPitch, r->w, r->h, r->x, r->y, orig_dst_y * scale1);
#endif
		}
		SDL_UnlockSurface(srcSurf);
		SDL_UnlockSurface(_hwscreen);

		// Readjust the dirty rect list in case we are doing a full update.
		// This is necessary if shaking is active.
		if (_forceFull) {
			_dirtyRectList[0].y = 0;
			_dirtyRectList[0].h = (_videoMode.mode == GFX_HALF) ? effectiveScreenHeight() / 2 : effectiveScreenHeight();
		}

		drawMouse();

#ifdef USE_OSD
		if (_osdAlpha != SDL_ALPHA_TRANSPARENT) {
			SDL_BlitSurface(_osdSurface, 0, _hwscreen, 0);
		}
#endif
		// Finally, blit all our changes to the screen
		SDL_UpdateRects(_hwscreen, _numDirtyRects, _dirtyRectList);
	}

	_numDirtyRects = 0;
	_forceFull = false;
	_mouseNeedsRedraw = false;
}

void DINGUXSdlGraphicsManager::showOverlay() {
	if (_videoMode.mode == GFX_HALF) {
		_mouseCurState.x = _mouseCurState.x / 2;
		_mouseCurState.y = _mouseCurState.y / 2;
	}
	SurfaceSdlGraphicsManager::showOverlay();
}

void DINGUXSdlGraphicsManager::hideOverlay() {
	if (_videoMode.mode == GFX_HALF) {
		_mouseCurState.x = _mouseCurState.x * 2;
		_mouseCurState.y = _mouseCurState.y * 2;
	}
	SurfaceSdlGraphicsManager::hideOverlay();
}

bool DINGUXSdlGraphicsManager::loadGFXMode() {
	debug("Game ScreenMode = %d*%d", _videoMode.screenWidth, _videoMode.screenHeight);

	// Forcefully disable aspect ratio correction for games
	// which starts with a native 240px height resolution.
	// This fixes games with weird resolutions, like MM Nes (256x240)
	if (_videoMode.screenHeight == 240) {
		_videoMode.aspectRatioCorrection = false;
	}

	if (_videoMode.screenWidth > 320 || _videoMode.screenHeight > 240) {
		_videoMode.aspectRatioCorrection = false;
		setGraphicsMode(GFX_HALF);
		debug("GraphicsMode set to HALF");
	} else {
		setGraphicsMode(GFX_NORMAL);
		debug("GraphicsMode set to NORMAL");
	}

	if ((_videoMode.mode == GFX_HALF) && !_overlayVisible) {
		_videoMode.overlayWidth = _videoMode.screenWidth / 2;
		_videoMode.overlayHeight = _videoMode.screenHeight / 2;
		_videoMode.fullscreen = true;
	} else {

		_videoMode.overlayWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
		_videoMode.overlayHeight = _videoMode.screenHeight * _videoMode.scaleFactor;

		if (_videoMode.aspectRatioCorrection)
			_videoMode.overlayHeight = real2Aspect(_videoMode.overlayHeight);

		_videoMode.hardwareWidth = _videoMode.screenWidth * _videoMode.scaleFactor;
		_videoMode.hardwareHeight = effectiveScreenHeight();
	}


	return SurfaceSdlGraphicsManager::loadGFXMode();
}

bool DINGUXSdlGraphicsManager::hasFeature(OSystem::Feature f) {
	return
	    (f == OSystem::kFeatureAspectRatioCorrection) ||
	    (f == OSystem::kFeatureCursorPalette);
}

void DINGUXSdlGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
	switch (f) {
	case OSystem::kFeatureAspectRatioCorrection:
		setAspectRatioCorrection(enable);
		break;
	case OSystem::kFeatureCursorPalette:
		_cursorPaletteDisabled = !enable;
		blitCursor();
		break;
	default:
		break;
	}
}

bool DINGUXSdlGraphicsManager::getFeatureState(OSystem::Feature f) {
	assert(_transactionMode == kTransactionNone);

	switch (f) {
	case OSystem::kFeatureAspectRatioCorrection:
			return _videoMode.aspectRatioCorrection;
	case OSystem::kFeatureCursorPalette:
		return !_cursorPaletteDisabled;
	default:
		return false;
	}
}

SurfaceSdlGraphicsManager::MousePos *DINGUXSdlGraphicsManager::getMouseCurState() {
	return &_mouseCurState;
}

SurfaceSdlGraphicsManager::VideoState *DINGUXSdlGraphicsManager::getVideoMode() {
	return &_videoMode;
}

void DINGUXSdlGraphicsManager::warpMouse(int x, int y) {
	if (_mouseCurState.x != x || _mouseCurState.y != y) {
		if (_videoMode.mode == GFX_HALF && !_overlayVisible) {
			x = x / 2;
			y = y / 2;
		}
	}
	SurfaceSdlGraphicsManager::warpMouse(x, y);
}

void DINGUXSdlGraphicsManager::transformMouseCoordinates(Common::Point &point) {
	if (!_overlayVisible) {
		if (_videoMode.mode == GFX_HALF) {
			point.x *= 2;
			point.y *= 2;
		}
		point.x /= _videoMode.scaleFactor;
		point.y /= _videoMode.scaleFactor;
		if (_videoMode.aspectRatioCorrection)
			point.y = aspect2Real(point.y);
	}
}

#endif
