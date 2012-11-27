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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/osystem/base_surface_osystem.h"
#include "engines/wintermute/base/gfx/osystem/base_render_osystem.h"
#include "engines/wintermute/base/gfx/base_image.h"
#include "engines/wintermute/platform_osystem.h"
#include "graphics/decoders/png.h"
#include "graphics/decoders/bmp.h"
#include "graphics/decoders/jpeg.h"
#include "graphics/decoders/tga.h"
#include "engines/wintermute/graphics/transparent_surface.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "common/stream.h"
#include "common/system.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
BaseSurfaceOSystem::BaseSurfaceOSystem(BaseGame *inGame) : BaseSurface(inGame) {
	_surface = new Graphics::Surface();
	_alphaMask = NULL;
	_hasAlpha = true;
	_lockPixels = NULL;
	_lockPitch = 0;
	_loaded = false;
}

//////////////////////////////////////////////////////////////////////////
BaseSurfaceOSystem::~BaseSurfaceOSystem() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = NULL;
	}

	delete[] _alphaMask;
	_alphaMask = NULL;

	_gameRef->addMem(-_width * _height * 4);
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer);
	renderer->invalidateTicketsFromSurface(this);
}

bool hasTransparency(Graphics::Surface *surf) {
	if (surf->format.bytesPerPixel != 4) {
		warning("hasTransparency:: non 32 bpp surface passed as argument");
		return false;
	}
	uint8 r, g, b, a;
	for (int i = 0; i < surf->h; i++) {
		for (int j = 0; j < surf->w; j++) {
			uint32 pix = *(uint32 *)surf->getBasePtr(j, i);
			surf->format.colorToARGB(pix, a, r, g, b);
			if (a != 255) {
				return true;
			}
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::create(const Common::String &filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime, bool keepLoaded) {
	/*  BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer); */
	_filename = filename;
//	const Graphics::Surface *surface = image->getSurface();

	if (defaultCK) {
		ckRed   = 255;
		ckGreen = 0;
		ckBlue  = 255;
	}

	_ckDefault = defaultCK;
	_ckRed = ckRed;
	_ckGreen = ckGreen;
	_ckBlue = ckBlue;

	if (_lifeTime == 0 || lifeTime == -1 || lifeTime > _lifeTime) {
		_lifeTime = lifeTime;
	}

	_keepLoaded = keepLoaded;
	if (_keepLoaded) {
		_lifeTime = -1;
	}

	return STATUS_OK;
}

bool BaseSurfaceOSystem::finishLoad() {
	BaseImage *image = new BaseImage();
	if (!image->loadFile(_filename)) {
		return false;
	}

	_width = image->getSurface()->w;
	_height = image->getSurface()->h;

	bool isSaveGameGrayscale = scumm_strnicmp(_filename.c_str(), "savegame:", 9) == 0 && (_filename.c_str()[_filename.size() - 1] == 'g' || _filename.c_str()[_filename.size() - 1] == 'G');
	if (isSaveGameGrayscale) {
		warning("grayscaleConversion not yet implemented");
		// FIBITMAP *newImg = FreeImage_ConvertToGreyscale(img); TODO
	}

	// no alpha, set color key
	/*  if (surface->format.bytesPerPixel != 4)
	        SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, ck_red, ck_green, ck_blue));*/

	// convert 32-bit BMPs to 24-bit or they appear totally transparent (does any app actually write alpha in BMP properly?)
	// Well, actually, we don't convert via 24-bit as the color-key application overwrites the Alpha-channel anyhow.
	_surface->free();
	delete _surface;
	if (_filename.hasSuffix(".bmp") && image->getSurface()->format.bytesPerPixel == 4) {
		_surface = image->getSurface()->convertTo(g_system->getScreenFormat(), image->getPalette());
		TransparentSurface trans(*_surface);
		trans.applyColorKey(_ckRed, _ckGreen, _ckBlue);
	} else if (image->getSurface()->format.bytesPerPixel == 1 && image->getPalette()) {
		_surface = image->getSurface()->convertTo(g_system->getScreenFormat(), image->getPalette());
		TransparentSurface trans(*_surface);
		trans.applyColorKey(_ckRed, _ckGreen, _ckBlue, true);
	} else if (image->getSurface()->format.bytesPerPixel >= 3 && image->getSurface()->format != g_system->getScreenFormat()) {
		_surface = image->getSurface()->convertTo(g_system->getScreenFormat());
		if (image->getSurface()->format.bytesPerPixel == 3) {
			TransparentSurface trans(*_surface);
			trans.applyColorKey(_ckRed, _ckGreen, _ckBlue, true);
		}
	} else {
		_surface = new Graphics::Surface();
		_surface->copyFrom(*image->getSurface());
	}

	_hasAlpha = hasTransparency(_surface);
	_valid = true;

	_gameRef->addMem(_width * _height * 4);

	delete image;

	_loaded = true;

	return true;
}

//////////////////////////////////////////////////////////////////////////
void BaseSurfaceOSystem::genAlphaMask(Graphics::Surface *surface) {
	warning("BaseSurfaceOSystem::GenAlphaMask - Not ported yet");
	return;
	// TODO: Reimplement this
	delete[] _alphaMask;
	_alphaMask = NULL;
	if (!surface) {
		return;
	}

	bool hasColorKey;
	/* uint32 colorKey; */
	uint8 ckRed, ckGreen, ckBlue;
	/*  if (SDL_GetColorKey(surface, &colorKey) == 0) {
	        hasColorKey = true;
	        SDL_GetRGB(colorKey, surface->format, &ckRed, &ckGreen, &ckBlue);
	    } else hasColorKey = false;
	*/
	_alphaMask = new byte[surface->w * surface->h];

	bool hasTransparency = false;
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->w; x++) {
			uint32 pixel = getPixelAt(surface, x, y);

			uint8 r, g, b, a;
			surface->format.colorToARGB(pixel, a, r, g, b);
			//SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);

			if (hasColorKey && r == ckRed && g == ckGreen && b == ckBlue) {
				a = 0;
			}

			_alphaMask[y * surface->w + x] = a;
			if (a < 255) {
				hasTransparency = true;
			}
		}
	}

	if (!hasTransparency) {
		delete[] _alphaMask;
		_alphaMask = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
uint32 BaseSurfaceOSystem::getPixelAt(Graphics::Surface *surface, int x, int y) {
	warning("BaseSurfaceOSystem::GetPixel - Not ported yet");
	int bpp = surface->format.bytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	uint8 *p = (uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		return *p;
		break;

	case 2:
		return *(uint16 *)p;
		break;

	case 3:
#ifdef SCUMM_BIG_ENDIAN
		//  if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		return p[0] << 16 | p[1] << 8 | p[2];
#else
		//else
		return p[0] | p[1] << 8 | p[2] << 16;
#endif
		break;

	case 4:
		return *(uint32 *)p;
		break;

	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::create(int width, int height) {
	_width = width;
	_height = height;

	_gameRef->addMem(_width * _height * 4);

	_valid = true;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::isTransparentAt(int x, int y) {
	return isTransparentAtLite(x, y);
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::isTransparentAtLite(int x, int y) {
	if (x < 0 || x >= _surface->w || y < 0 || y >= _surface->h) {
		return true;
	}

	if (_surface->format.bytesPerPixel == 4) {
		uint32 pixel = *(uint32 *)_surface->getBasePtr(x, y);
		uint8 r, g, b, a;
		_surface->format.colorToARGB(pixel, a, r, g, b);
		if (a <= 128) {
			return true;
		} else {
			return false;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::startPixelOp() {
	//SDL_LockTexture(_texture, NULL, &_lockPixels, &_lockPitch);
	// Any pixel-op makes the caching useless:
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer);
	renderer->invalidateTicketsFromSurface(this);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::endPixelOp() {
	//SDL_UnlockTexture(_texture);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::display(int x, int y, Rect32 rect, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return drawSprite(x, y, &rect, 100, 100, 0xFFFFFFFF, true, blendMode, mirrorX, mirrorY);
}


//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTrans(int x, int y, Rect32 rect, uint32 alpha, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return drawSprite(x, y, &rect, 100, 100, alpha, false, blendMode, mirrorX, mirrorY);
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTransOffset(int x, int y, Rect32 rect, uint32 alpha, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX, int offsetY) {
	return drawSprite(x, y, &rect, 100, 100, alpha, false, blendMode, mirrorX, mirrorY, offsetX, offsetY);
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTransZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return drawSprite(x, y, &rect, zoomX, zoomY, alpha, false, blendMode, mirrorX, mirrorY);
}


//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha, bool transparent, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return drawSprite(x, y, &rect, zoomX, zoomY, alpha, !transparent, blendMode, mirrorX, mirrorY);
}


//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::displayTransform(int x, int y, int hotX, int hotY, Rect32 rect, float zoomX, float zoomY, uint32 alpha, float rotate, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY) {
	return drawSprite(x, y, &rect, zoomX, zoomY, alpha, false, blendMode, mirrorX, mirrorY);
}

//////////////////////////////////////////////////////////////////////////
bool BaseSurfaceOSystem::drawSprite(int x, int y, Rect32 *rect, float zoomX, float zoomY, uint32 alpha, bool alphaDisable, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX, int offsetY) {
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer);

	if (!_loaded) {
		finishLoad();
	}

	if (renderer->_forceAlphaColor != 0) {
		alpha = renderer->_forceAlphaColor;
	}

	byte r = RGBCOLGetR(alpha);
	byte g = RGBCOLGetG(alpha);
	byte b = RGBCOLGetB(alpha);
	byte a = RGBCOLGetA(alpha);

	renderer->setAlphaMod(a);
	renderer->setColorMod(r, g, b);

#if 0 // These are kept for reference if BlendMode is reimplemented at some point.
	if (alphaDisable) {
		SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_NONE);
	} else {
		SDL_SetTextureBlendMode(_texture, SDL_BLENDMODE_BLEND);
	}
#endif
	// TODO: This _might_ miss the intended behaviour by 1 in each direction
	// But I think it fits the model used in Wintermute.
	Common::Rect srcRect;
	srcRect.left = rect->left;
	srcRect.top = rect->top;
	srcRect.setWidth(rect->right - rect->left);
	srcRect.setHeight(rect->bottom - rect->top);

	Common::Rect position;
	position.left = x + offsetX;
	position.top = y + offsetY;

	// Crop off-by-ones:
	if (position.left == -1) {
		position.left = 0; // TODO: Something is wrong
	}
	if (position.top == -1) {
		position.top = 0; // TODO: Something is wrong
	}

	position.setWidth((int16)((float)srcRect.width() * zoomX / 100.f));
	position.setHeight((int16)((float)srcRect.height() * zoomX / 100.f));

	renderer->modTargetRect(&position);

	/*  position.left += offsetX;
	    position.top += offsetY;*/

	// TODO: This actually requires us to have the SAME source-offsets every time,
	// But no checking is in place for that yet.

	// TODO: Optimize by not doing alpha-blits if we lack or disable alpha
	bool hasAlpha;
	if (_hasAlpha && !alphaDisable) {
		hasAlpha = true;
	} else {
		hasAlpha = false;
	}
	if (alphaDisable) {
		warning("BaseSurfaceOSystem::drawSprite - AlphaDisable ignored");
	}

	renderer->drawSurface(this, _surface, &srcRect, &position, mirrorX, mirrorY, !hasAlpha);

	return STATUS_OK;
}

bool BaseSurfaceOSystem::putSurface(const Graphics::Surface &surface, bool hasAlpha) {
	_loaded = true;
	_surface->free();
	_surface->copyFrom(surface);
	_hasAlpha = hasAlpha;
	BaseRenderOSystem *renderer = static_cast<BaseRenderOSystem *>(_gameRef->_renderer);
	renderer->invalidateTicketsFromSurface(this);

	return STATUS_OK;
}

} // end of namespace Wintermute
