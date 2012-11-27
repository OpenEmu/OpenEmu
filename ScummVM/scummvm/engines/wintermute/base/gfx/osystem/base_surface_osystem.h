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

#ifndef WINTERMUTE_BASE_SURFACESDL_H
#define WINTERMUTE_BASE_SURFACESDL_H

#include "graphics/surface.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "common/list.h"

namespace Wintermute {
struct TransparentSurface;
class BaseImage;
class BaseSurfaceOSystem : public BaseSurface {
public:
	BaseSurfaceOSystem(BaseGame *inGame);
	~BaseSurfaceOSystem();

	bool create(const Common::String &filename, bool defaultCK, byte ckRed, byte ckGreen, byte ckBlue, int lifeTime = -1, bool keepLoaded = false);
	bool create(int width, int height);

	bool isTransparentAt(int x, int y);
	bool isTransparentAtLite(int x, int y);

	bool startPixelOp();
	bool endPixelOp();


	bool displayTransZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false);
	bool displayTrans(int x, int y, Rect32 rect, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false);
	bool displayTransOffset(int x, int y, Rect32 rect, uint32 alpha = 0xFFFFFFFF, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false, int offsetX = 0, int offsetY = 0);
	bool display(int x, int y, Rect32 rect, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false);
	bool displayZoom(int x, int y, Rect32 rect, float zoomX, float zoomY, uint32 alpha = 0xFFFFFFFF, bool transparent = false, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false);
	bool displayTransform(int x, int y, int hotX, int hotY, Rect32 Rect, float zoomX, float zoomY, uint32 alpha, float rotate, TSpriteBlendMode blendMode = BLEND_NORMAL, bool mirrorX = false, bool mirrorY = false);
	virtual bool putSurface(const Graphics::Surface &surface, bool hasAlpha = false);
	/*  static unsigned DLL_CALLCONV ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle);
	    static int DLL_CALLCONV SeekProc(fi_handle handle, long offset, int origin);
	    static long DLL_CALLCONV TellProc(fi_handle handle);*/
	virtual int getWidth() {
		if (!_loaded) {
			finishLoad();
		}
		if (_surface) {
			return _surface->w;
		}
		return _width;
	}
	virtual int getHeight() {
		if (!_loaded) {
			finishLoad();
		}
		if (_surface) {
			return _surface->h;
		}
		return _height;
	}

private:
	Graphics::Surface *_surface;
	bool _loaded;
	bool finishLoad();
	bool drawSprite(int x, int y, Rect32 *rect, float zoomX, float zoomY, uint32 alpha, bool alphaDisable, TSpriteBlendMode blendMode, bool mirrorX, bool mirrorY, int offsetX = 0, int offsetY = 0);
	void genAlphaMask(Graphics::Surface *surface);
	uint32 getPixelAt(Graphics::Surface *surface, int x, int y);

	bool _hasAlpha;
	void *_lockPixels;
	int _lockPitch;
	byte *_alphaMask;
};

} // end of namespace Wintermute

#endif
