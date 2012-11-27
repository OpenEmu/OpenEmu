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

#ifndef BACKENDS_GRAPHICS_NULL_H
#define BACKENDS_GRAPHICS_NULL_H

#include "backends/graphics/graphics.h"

static const OSystem::GraphicsMode s_noGraphicsModes[] = { {0, 0, 0} };

class NullGraphicsManager : public GraphicsManager {
public:
	virtual ~NullGraphicsManager() {}

	bool hasFeature(OSystem::Feature f) { return false; }
	void setFeatureState(OSystem::Feature f, bool enable) {}
	bool getFeatureState(OSystem::Feature f) { return false; }

	const OSystem::GraphicsMode *getSupportedGraphicsModes() const { return s_noGraphicsModes; }
	int getDefaultGraphicsMode() const { return 0; }
	bool setGraphicsMode(int mode) { return true; }
	void resetGraphicsScale(){}
	int getGraphicsMode() const { return 0; }
	inline Graphics::PixelFormat getScreenFormat() const {
		return Graphics::PixelFormat::createFormatCLUT8();
	}
	inline Common::List<Graphics::PixelFormat> getSupportedFormats() const {
		Common::List<Graphics::PixelFormat> list;
		list.push_back(Graphics::PixelFormat::createFormatCLUT8());
		return list;
	}
	void initSize(uint width, uint height, const Graphics::PixelFormat *format = NULL) {}
	virtual int getScreenChangeID() const { return 0; }

	void beginGFXTransaction() {}
	OSystem::TransactionError endGFXTransaction() { return OSystem::kTransactionSuccess; }

	int16 getHeight() { return 0; }
	int16 getWidth() { return 0; }
	void setPalette(const byte *colors, uint start, uint num) {}
	void grabPalette(byte *colors, uint start, uint num) {}
	void copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {}
	Graphics::Surface *lockScreen() { return NULL; }
	void unlockScreen() {}
	void fillScreen(uint32 col) {}
	void updateScreen() {}
	void setShakePos(int shakeOffset) {}
	void setFocusRectangle(const Common::Rect& rect) {}
	void clearFocusRectangle() {}

	void showOverlay() {}
	void hideOverlay() {}
	Graphics::PixelFormat getOverlayFormat() const { return Graphics::PixelFormat(); }
	void clearOverlay() {}
	void grabOverlay(void *buf, int pitch) {}
	void copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {}
	int16 getOverlayHeight() { return 0; }
	int16 getOverlayWidth() { return 0; }

	bool showMouse(bool visible) { return !visible; }
	void warpMouse(int x, int y) {}
	void setMouseCursor(const void *buf, uint w, uint h, int hotspotX, int hotspotY, uint32 keycolor, bool dontScale = false, const Graphics::PixelFormat *format = NULL) {}
	void setCursorPalette(const byte *colors, uint start, uint num) {}
};

#endif
