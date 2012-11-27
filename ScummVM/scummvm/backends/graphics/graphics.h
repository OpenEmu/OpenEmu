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

#ifndef BACKENDS_GRAPHICS_ABSTRACT_H
#define BACKENDS_GRAPHICS_ABSTRACT_H

#include "common/system.h"
#include "common/noncopyable.h"
#include "common/keyboard.h"

#include "graphics/palette.h"

/**
 * Abstract class for graphics manager. Subclasses
 * implement the real functionality.
 */
class GraphicsManager : public PaletteManager {
public:
	virtual ~GraphicsManager() {}

	virtual bool hasFeature(OSystem::Feature f) = 0;
	virtual void setFeatureState(OSystem::Feature f, bool enable) = 0;
	virtual bool getFeatureState(OSystem::Feature f) = 0;

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const = 0;
	virtual int getDefaultGraphicsMode() const = 0;
	virtual bool setGraphicsMode(int mode) = 0;
	virtual void resetGraphicsScale() = 0;
	virtual int getGraphicsMode() const = 0;
#ifdef USE_RGB_COLOR
	virtual Graphics::PixelFormat getScreenFormat() const = 0;
	virtual Common::List<Graphics::PixelFormat> getSupportedFormats() const = 0;
#endif
	virtual void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) = 0;
	virtual int getScreenChangeID() const = 0;

	virtual void beginGFXTransaction() = 0;
	virtual OSystem::TransactionError endGFXTransaction() = 0;

	virtual int16 getHeight() = 0;
	virtual int16 getWidth() = 0;
	virtual void setPalette(const byte *colors, uint start, uint num) = 0;
	virtual void grabPalette(byte *colors, uint start, uint num) = 0;
	virtual void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) = 0;
	virtual Graphics::Surface *lockScreen() = 0;
	virtual void unlockScreen() = 0;
	virtual void fillScreen(uint32 col) = 0;
	virtual void updateScreen() = 0;
	virtual void setShakePos(int shakeOffset) = 0;
	virtual void setFocusRectangle(const Common::Rect& rect) = 0;
	virtual void clearFocusRectangle() = 0;

	virtual void showOverlay() = 0;
	virtual void hideOverlay() = 0;
	virtual Graphics::PixelFormat getOverlayFormat() const = 0;
	virtual void clearOverlay() = 0;
	virtual void grabOverlay(void *buf, int pitch) = 0;
	virtual void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h)= 0;
	virtual int16 getOverlayHeight() = 0;
	virtual int16 getOverlayWidth() = 0;

	virtual bool showMouse(bool visible) = 0;
	virtual void warpMouse(int x, int y) = 0;
	virtual void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) = 0;
	virtual void setCursorPalette(const byte *colors, uint start, uint num) = 0;

	virtual void displayMessageOnOSD(const char *msg) {}

	// Graphics::PaletteManager interface
	//virtual void setPalette(const byte *colors, uint start, uint num) = 0;
	//virtual void grabPalette(byte *colors, uint start, uint num) = 0;
};

#endif
