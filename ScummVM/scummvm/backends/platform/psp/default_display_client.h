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

#ifndef PSP_DEF_DISPLAY_CLIENT_H
#define PSP_DEF_DISPLAY_CLIENT_H

/**
 *	Default display client that is useful for most purposes.
 */
class DefaultDisplayClient : public DisplayClient {
public:
	DefaultDisplayClient() : _visible(false), _dirty(true) {}

	bool isVisible() const { return _visible; }
	void setVisible(bool v) { _visible = v; setDirty(); }
	Buffer &buffer() { return _buffer; }
	Palette &palette() { return _palette; }
	void init();
	bool allocate(bool bufferInVram = false, bool paletteInVram = false);
	void deallocate();
	void clearBuffer();
	void clearPalette();
	void render() { _renderer.render(); }
	uint32 getWidth() { return _buffer.getSourceWidth(); }
	uint32 getHeight() { return _buffer.getSourceHeight(); }
	void setPartialPalette(const byte *colors, uint start, uint num) { setDirty(); return _palette.setPartial(colors, start, num); }
	void getPartialPalette(byte *colors, uint start, uint num) {
		return _palette.getPartial(colors, start, num);
	}
	void copyFromRect(const byte *buf, int pitch, int destX, int destY, int recWidth, int recHeight);
	void copyToArray(byte *dst, int pitch);
	void setDirty() { _dirty = true; }
	void setClean() { _dirty = false; }
	bool isDirty() const { return _dirty; }

protected:
	Buffer _buffer;
	Palette _palette;
	GuRenderer _renderer;
	bool _visible;
	bool _dirty;
};

/**
 *	Screen overlay class.
 */
class Overlay : public DefaultDisplayClient {
public:
	void init();
	bool allocate();
	void setBytesPerPixel(uint32 size);
	void setSize(uint32 width, uint32 height);
	void copyToArray(void *buf, int pitch);
	void copyFromRect(const void *buf, int pitch, int x, int y, int w, int h);
};

/**
 *	Screen class.
 */
class Screen : public DefaultDisplayClient {
public:
	Screen() : _shakePos(0) {
		memset(&_pixelFormat, 0, sizeof(_pixelFormat));
		memset(&_frameBuffer, 0, sizeof(_frameBuffer));
	}

	void init();
	bool allocate();
	void setShakePos(int pos);
	void setScummvmPixelFormat(const Graphics::PixelFormat *format);
	const Graphics::PixelFormat &getScummvmPixelFormat() const { return _pixelFormat; }
	Graphics::Surface *lockAndGetForEditing();
	void unlock() { setDirty(); } // set dirty here because of changes
	void setSize(uint32 width, uint32 height);

private:
	uint32 _shakePos;
	Graphics::PixelFormat _pixelFormat;
	Graphics::Surface _frameBuffer;
};

#endif /* PSP_DEF_DISPLAY_CLIENT_H */
