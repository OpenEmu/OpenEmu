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

#ifndef WINTERMUTE_BASE_RENDERER_H
#define WINTERMUTE_BASE_RENDERER_H

#include "engines/wintermute/math/rect32.h"
#include "engines/wintermute/base/base.h"
#include "common/rect.h"
#include "common/array.h"

namespace Wintermute {

class BaseImage;
class BaseActiveRect;
class BaseObject;
class BaseSurface;
class BasePersistenceManager;

/**
 * @class BaseRenderer a common interface for the rendering portion of WME
 * this interface is mainly intended to wrap away any differencies between
 * software-rendering/hardware-rendering.
 */
class BaseRenderer: public BaseClass {
public:
	int _realWidth;
	int _realHeight;
	int _drawOffsetX;
	int _drawOffsetY;

	virtual void dumpData(const char *filename) {}
	/**
	 * Take a screenshot of the current screenstate
	 *
	 * @return a BaseImage containing the current screen-buffer.
	 */
	virtual BaseImage *takeScreenshot() = 0;
	virtual bool setViewport(int left, int top, int right, int bottom);
	virtual bool setViewport(Rect32 *rect);
	virtual Rect32 getViewPort() = 0;
	virtual bool setScreenViewport();

	virtual Graphics::PixelFormat getPixelFormat() const = 0;
	/**
	 * Fade the screen to black
	 *
	 * @param alpha amount to fade by (alpha value of black)
	 * @return
	 */
	virtual void fade(uint16 alpha) = 0;
	/**
	 * Fade a portion of the screen to a specific color
	 *
	 * @param r the red component to fade too.
	 * @param g the green component to fade too.
	 * @param b the blue component to fade too.
	 * @param a the alpha component to fade too.
	 * @param rect the portion of the screen to fade (if NULL, the entire screen will be faded).
	 */
	virtual void fadeToColor(byte r, byte g, byte b, byte a, Common::Rect *rect = NULL) = 0;
	virtual bool drawLine(int x1, int y1, int x2, int y2, uint32 color);
	virtual bool drawRect(int x1, int y1, int x2, int y2, uint32 color, int width = 1);
	BaseRenderer(BaseGame *inGame = NULL);
	virtual ~BaseRenderer();
	virtual bool setProjection() {
		return STATUS_OK;
	};

	virtual bool windowedBlt();
	/**
	 * Fill a portion of the screen with a specified color
	 *
	 * @param r the red component to fill with.
	 * @param g the green component to fill with.
	 * @param b the blue component to fill with.
	 */
	virtual bool fill(byte r, byte g, byte b, Common::Rect *rect = NULL) = 0;
	virtual void onWindowChange();
	virtual bool initRenderer(int width, int height, bool windowed);
	/**
	 * Flip the backbuffer onto the screen-buffer
	 * The screen will NOT be updated before calling this function.
	 *
	 * @return true if successfull, false on error.
	 */
	virtual bool flip() = 0;
	/**
	 * Special flip for the indicator drawn during save/load
	 * essentially, just copies the region defined by the _indicator-variables.
	 */
	virtual bool indicatorFlip() = 0;
	virtual void initLoop();
	virtual bool setup2D(bool force = false);
	virtual bool setupLines();

	/**
	 * Get the name of the current renderer
	 *
	 * @return the name of the renderer.
	 */
	virtual Common::String getName() const = 0;
	virtual bool displayDebugInfo() {
		return STATUS_FAILED;
	};
	virtual bool drawShaderQuad() {
		return STATUS_FAILED;
	}

	virtual float getScaleRatioX() const {
		return 1.0f;
	}
	virtual float getScaleRatioY() const {
		return 1.0f;
	}

	/**
	 * Create a Surface fit for use with the renderer.
	 * As diverse implementations of BaseRenderer might have different solutions for storing surfaces
	 * this allows for a common interface for creating surface-handles. (Mostly usefull to ease future
	 * implementation of hw-accelerated rendering, or readding 3D-support at some point).
	 *
	 * @return a surface that can be used with this renderer
	 */
	virtual BaseSurface *createSurface() = 0;

	bool clipCursor();
	bool unclipCursor();

	BaseObject *getObjectAt(int x, int y);
	void deleteRectList();

	virtual bool startSpriteBatch() {
		return STATUS_OK;
	};
	virtual bool endSpriteBatch() {
		return STATUS_OK;
	};
	bool pointInViewport(Point32 *P);
	bool _active;
	bool _ready;
	bool _windowed;
	int _bPP;
	int _height;
	int _width;
	uint32 _window;
	uint32 _forceAlphaColor;

	void addRectToList(BaseActiveRect *rect);

	// Indicator & Save/Load-related functions
	void initIndicator();
	void setIndicatorVal(int value);
	void setIndicator(int width, int height, int x, int y, uint32 color);
	void persistSaveLoadImages(BasePersistenceManager *persistMgr);
	void initSaveLoad(bool isSaving, bool quickSave = false);
	void endSaveLoad();
	void setLoadingScreen(const char *filename, int x, int y);
	void setSaveImage(const char *filename, int x, int y);

	bool displayIndicator();
protected:
	Common::String _loadImageName;
	Common::String _saveImageName;
	int _saveImageX;
	int _saveImageY;
	int _loadImageX;
	int _loadImageY;
	BaseSurface *_saveLoadImage;
	bool _hasDrawnSaveLoadImage;

	int _indicatorWidthDrawn;
	uint32 _indicatorColor;
	int _indicatorX;
	int _indicatorY;
	int _indicatorWidth;
	int _indicatorHeight;
	bool _loadInProgress;
	bool _indicatorDisplay;
	int _indicatorProgress;
protected:
	uint32 _clipperWindow;

	Rect32 _windowRect;
	Rect32 _viewportRect;
	Rect32 _screenRect;
	Rect32 _monitorRect;
private:
	Common::Array<BaseActiveRect *> _rectList;
};

BaseRenderer *makeOSystemRenderer(BaseGame *inGame); // Implemented in BRenderSDL.cpp

} // end of namespace Wintermute

#endif
