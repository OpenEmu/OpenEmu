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

#ifndef MOUSE_H
#define MOUSE_H

#include "backends/platform/psp/default_display_client.h"

class Cursor : public DefaultDisplayClient {
private:
	int	_hotspotX, _hotspotY;
	uint32 _keyColor;
	int _cursorTargetScale;
	bool _useCursorPalette;
	bool _hasCursorPalette;
	uint32	_mouseLimitWidth;
	uint32	_mouseLimitHeight;
	int32	_x, _y;
	Palette _screenPalette;			// separate palette for screen. default 'palette' is cursor palette.

	void updateRendererOffset();

public:
	Cursor() : _hotspotX(0), _hotspotY(0), _keyColor(0), _cursorTargetScale(0),
			_useCursorPalette(false), _hasCursorPalette(false), _mouseLimitWidth(0),
			_mouseLimitHeight(0), _x(0), _y(0) { }
	virtual ~Cursor() { deallocate(); }

	void setKeyColor(uint32 color);
	void setCursorTargetScale(int scale) { _cursorTargetScale = scale; }
	void setScreenPalette(const byte *colors, uint start, uint num);
	void copyFromArray(const byte *array);
	Palette &palette() { return _palette; }
	Buffer &buffer() { return _buffer; }
	void setCursorPalette(const byte *colors, uint start, uint num);
	void enableCursorPalette(bool enable);
	bool isCursorPaletteEnabled() const { return _useCursorPalette; }
	void setLimits(uint32 width, uint32 height);
	void setXY(int x, int y);
	int32 getX() const { return _x; }
	int32 getY() const { return _y; }
	bool increaseXY(int32 incX, int32 incY); // returns true if there's a change in x or y
	void adjustXYForScreenSize(int32 &x, int32 &y);
	void init();
	void setHotspot(int32 x, int32 y);
	void setScreenPaletteScummvmPixelFormat(const Graphics::PixelFormat *format);
	void setSizeAndScummvmPixelFormat(uint32 widht, uint32 height, const Graphics::PixelFormat *format);
	void clearKeyColor();
	void useGlobalScaler(bool val) { _renderer.setUseGlobalScaler(val); }
	bool allocate();
	void deallocate();

private:
	void setSize(uint32 width, uint32 height);
	void getPixelFormatsFromScummvmPixelFormat(const Graphics::PixelFormat *format,
	        PSPPixelFormat::Type &bufferFormat,
	        PSPPixelFormat::Type &paletteFormat,
	        uint32 &numOfEntries);
	void setRendererModePalettized(bool palettized);
};

#endif /* MOUSE_H */
