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

#ifndef SCI_GRAPHICS_SCREEN_H
#define SCI_GRAPHICS_SCREEN_H

#include "sci/sci.h"
#include "sci/graphics/helpers.h"
#include "sci/graphics/view.h"

#include "graphics/sjis.h"

namespace Sci {

#define SCI_SCREEN_UPSCALEDMAXHEIGHT 200

enum GfxScreenUpscaledMode {
	GFX_SCREEN_UPSCALED_DISABLED	= 0,
	GFX_SCREEN_UPSCALED_640x400		= 1,
	GFX_SCREEN_UPSCALED_640x440		= 2,
	GFX_SCREEN_UPSCALED_640x480		= 3
};

enum GfxScreenMasks {
	GFX_SCREEN_MASK_VISUAL		= 1,
	GFX_SCREEN_MASK_PRIORITY	= 2,
	GFX_SCREEN_MASK_CONTROL		= 4,
	GFX_SCREEN_MASK_DISPLAY		= 8, // not official sierra sci, only used internally
	GFX_SCREEN_MASK_ALL			= GFX_SCREEN_MASK_VISUAL|GFX_SCREEN_MASK_PRIORITY|GFX_SCREEN_MASK_CONTROL
};

enum {
	DITHERED_BG_COLORS_SIZE = 256
};

/**
 * Screen class, actually creates 3 (4) screens internally:
 * - visual/display (for the user),
 * - priority (contains priority information) and
 * - control (contains control information).
 * Handles all operations to it and copies parts of visual/display screen to
 * the actual screen, so the user can really see it.
 */
class GfxScreen {
public:
	GfxScreen(ResourceManager *resMan);
	~GfxScreen();

	uint16 getWidth() { return _width; }
	uint16 getHeight() { return _height; }
	uint16 getDisplayWidth() { return _displayWidth; }
	uint16 getDisplayHeight() { return _displayHeight; }
	byte getColorWhite() { return _colorWhite; }
	byte getColorDefaultVectorData() { return _colorDefaultVectorData; }

	void copyToScreen();
	void copyFromScreen(byte *buffer);
	void kernelSyncWithFramebuffer();
	void copyRectToScreen(const Common::Rect &rect);
	void copyDisplayRectToScreen(const Common::Rect &rect);
	void copyRectToScreen(const Common::Rect &rect, int16 x, int16 y);

	byte getDrawingMask(byte color, byte prio, byte control);
	void putPixel(int x, int y, byte drawMask, byte color, byte prio, byte control);
	void putFontPixel(int startingY, int x, int y, byte color);
	void putPixelOnDisplay(int x, int y, byte color);
	void drawLine(Common::Point startPoint, Common::Point endPoint, byte color, byte prio, byte control);
	void drawLine(int16 left, int16 top, int16 right, int16 bottom, byte color, byte prio, byte control) {
		drawLine(Common::Point(left, top), Common::Point(right, bottom), color, prio, control);
	}

	GfxScreenUpscaledMode getUpscaledHires() const {
		return _upscaledHires;
	}

	bool isUnditheringEnabled() const {
		return _unditheringEnabled;
	}
	void enableUndithering(bool flag);

	void putKanjiChar(Graphics::FontSJIS *commonFont, int16 x, int16 y, uint16 chr, byte color);
	byte getVisual(int x, int y);
	byte getPriority(int x, int y);
	byte getControl(int x, int y);
	byte isFillMatch(int16 x, int16 y, byte drawMask, byte t_color, byte t_pri, byte t_con, bool isEGA);

	int bitsGetDataSize(Common::Rect rect, byte mask);
	void bitsSave(Common::Rect rect, byte mask, byte *memoryPtr);
	void bitsGetRect(byte *memoryPtr, Common::Rect *destRect);
	void bitsRestore(byte *memoryPtr);

	void scale2x(const byte *src, byte *dst, int16 srcWidth, int16 srcHeight, byte bytesPerPixel = 1);

	void adjustToUpscaledCoordinates(int16 &y, int16 &x, Sci32ViewNativeResolution viewScalingType = SCI_VIEW_NATIVERES_NONE);
	void adjustBackUpscaledCoordinates(int16 &y, int16 &x, Sci32ViewNativeResolution viewScalingType = SCI_VIEW_NATIVERES_NONE);

	void dither(bool addToFlag);

	// Force a color combination as a dithered color
	void ditherForceDitheredColor(byte color);
	int16 *unditherGetDitheredBgColors();

	void debugShowMap(int mapNo);

	int _picNotValid; // possible values 0, 1 and 2
	int _picNotValidSci11; // another variable that is used by kPicNotValid in sci1.1

	int16 kernelPicNotValid(int16 newPicNotValid);
	void kernelShakeScreen(uint16 shakeCount, uint16 direction);

	void setFontIsUpscaled(bool isUpscaled) { _fontIsUpscaled = isUpscaled; }
	bool fontIsUpscaled() const { return _fontIsUpscaled; }

private:
	uint16 _width;
	uint16 _pitch;
	uint16 _height;
	uint _pixels;
	uint16 _displayWidth;
	uint16 _displayHeight;
	uint _displayPixels;

	byte _colorWhite;
	byte _colorDefaultVectorData;

	void bitsRestoreScreen(Common::Rect rect, byte *&memoryPtr, byte *screen, uint16 screenWidth);
	void bitsRestoreDisplayScreen(Common::Rect rect, byte *&memoryPtr);
	void bitsSaveScreen(Common::Rect rect, byte *screen, uint16 screenWidth, byte *&memoryPtr);
	void bitsSaveDisplayScreen(Common::Rect rect, byte *&memoryPtr);

	void setVerticalShakePos(uint16 shakePos);

	/**
	 * If this flag is true, undithering is enabled, otherwise disabled.
	 */
	bool _unditheringEnabled;
	int16 _ditheredPicColors[DITHERED_BG_COLORS_SIZE];

	// These screens have the real resolution of the game engine (320x200 for
	// SCI0/SCI1/SCI11 games, 640x480 for SCI2 games). SCI0 games will be
	// dithered in here at any time.
	byte *_visualScreen;
	byte *_priorityScreen;
	byte *_controlScreen;

	/**
	 * This screen is the one that is actually displayed to the user. It may be
	 * 640x400 for japanese SCI1 games. SCI0 games may be undithered in here.
	 * Only read from this buffer for Save/ShowBits usage.
	 */
	byte *_displayScreen;

	ResourceManager *_resMan;

	/**
	 * Pointer to the currently active screen (changing it only required for
	 * debug purposes).
	 */
	byte *_activeScreen;

	/**
	 * This variable defines, if upscaled hires is active and what upscaled mode
	 * is used.
	 */
	GfxScreenUpscaledMode _upscaledHires;

	/**
	 * This here holds a translation for vertical coordinates between native
	 * (visual) and actual (display) screen.
	 */
	int _upscaledMapping[SCI_SCREEN_UPSCALEDMAXHEIGHT + 1];

	/**
	 * This defines whether or not the font we're drawing is already scaled
	 * to the screen size (and we therefore should not upscale it ourselves).
	 */
	bool _fontIsUpscaled;

	uint16 getLowResScreenHeight();
};

} // End of namespace Sci

#endif
