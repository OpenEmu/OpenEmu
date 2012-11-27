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

#ifndef GOB_SURFACE_H
#define GOB_SURFACE_H

#include "common/scummsys.h"
#include "common/ptr.h"
#include "common/rational.h"
#include "common/iff_container.h"

#include "graphics/iff.h"

namespace Common {
class SeekableReadStream;
}

namespace Gob {

enum ImageType {
	kImageTypeNone = -1,
	kImageTypeTGA  =  0,
	kImageTypeLBM,
	kImageTypeBRC,
	kImageTypeBMP,
	kImageTypeJPEG
};

class LBMLoader {
public:
	LBMLoader(Common::SeekableReadStream &stream);

	bool loadHeader (Graphics::BMHD &header);
	bool loadPalette(byte *palette);
	bool loadImage  (byte *image);

private:
	Common::IFFParser _parser;

	bool _hasHeader;

	Graphics::ILBMDecoder _decoder;

	byte *_palette;
	byte *_image;

	bool callbackHeader (Common::IFFChunk &chunk);
	bool callbackPalette(Common::IFFChunk &chunk);
	bool callbackImage  (Common::IFFChunk &chunk);

	bool readHeader();
};

/** An iterator over a surface's image data, automatically handles different color depths. */
class Pixel {
public:
	Pixel(byte *vidMem, uint8 bpp, byte *min, byte *max);

	Pixel &operator++();
	Pixel operator++(int x);

	Pixel &operator--();
	Pixel operator--(int x);

	Pixel &operator+=(int x);
	Pixel &operator-=(int x);

	uint32 get() const;
	void set(uint32 p);

	bool isValid() const;

private:
	byte *_vidMem;
	byte *_min, *_max;
	uint8 _bpp;
};

/** A const iterator over a surface's image data, automatically handles different color depths. */
class ConstPixel {
public:
	ConstPixel(const byte *vidMem, uint8 bpp, const byte *min, const byte *max);

	ConstPixel &operator++();
	ConstPixel operator++(int x);

	ConstPixel &operator--();
	ConstPixel operator--(int x);

	ConstPixel &operator+=(int x);
	ConstPixel &operator-=(int x);

	uint32 get() const;

	bool isValid() const;

private:
	const byte *_vidMem;
	const byte *_min, *_max;
	uint8 _bpp;
};

class Surface {
public:
	Surface(uint16 width, uint16 height, uint8 bpp, byte *vidMem = 0);
	Surface(uint16 width, uint16 height, uint8 bpp, const byte *vidMem);
	~Surface();

	uint16 getWidth () const;
	uint16 getHeight() const;
	uint8  getBPP   () const;

	byte *getData(uint16 x = 0, uint16 y = 0);
	const byte *getData(uint16 x = 0, uint16 y = 0) const;

	void resize(uint16 width, uint16 height);

	void setBPP(uint8 bpp);

	Pixel get(uint16 x = 0, uint16 y = 0);
	ConstPixel get(uint16 x = 0, uint16 y = 0) const;

	void blit(const Surface &from, int16 left, int16 top, int16 right, int16 bottom,
	          int16 x, int16 y, int32 transp = -1);
	void blit(const Surface &from, int16 x, int16 y, int32 transp = -1);
	void blit(const Surface &from, int32 transp = -1);

	void blitScaled(const Surface &from, int16 left, int16 top, int16 right, int16 bottom,
	                int16 x, int16 y, Common::Rational scale, int32 transp = -1);
	void blitScaled(const Surface &from, int16 x, int16 y, Common::Rational scale, int32 transp = -1);
	void blitScaled(const Surface &from, Common::Rational scale, int32 transp = -1);

	void fillRect(uint16 left, uint16 top, uint16 right, uint16 bottom, uint32 color);
	void fill(uint32 color);
	void clear();

	void shadeRect(uint16 left, uint16 top, uint16 right, uint16 bottom,
			uint32 color, uint8 strength);

	void recolor(uint8 from, uint8 to);

	void putPixel(uint16 x, uint16 y, uint32 color);
	void drawLine(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint32 color);
	void drawRect(uint16 left, uint16 top, uint16 right, uint16 bottom, uint32 color);
	void drawCircle(uint16 x0, uint16 y0, uint16 radius, uint32 color, int16 pattern = 0);

	void blitToScreen(uint16 left, uint16 top, uint16 right, uint16 bottom, uint16 x, uint16 y) const;

	bool loadImage(Common::SeekableReadStream &stream);
	bool loadImage(Common::SeekableReadStream &stream, ImageType type);

	static ImageType identifyImage(Common::SeekableReadStream &stream);

private:
	uint16 _width;
	uint16 _height;
	uint8  _bpp;

	bool  _ownVidMem;
	byte *_vidMem;

	static bool clipBlitRect(int16 &left, int16 &top, int16 &right, int16 &bottom, int16 &x, int16 &y,
	                         uint16 dWidth, uint16 dHeight, uint16 sWidth, uint16 sHeight);

	bool loadTGA (Common::SeekableReadStream &stream);
	bool loadLBM (Common::SeekableReadStream &stream);
	bool loadBRC (Common::SeekableReadStream &stream);
	bool loadBMP (Common::SeekableReadStream &stream);
	bool loadJPEG(Common::SeekableReadStream &stream);
};

typedef Common::SharedPtr<Surface> SurfacePtr;

} // End of namespace Gob

#endif // GOB_SURFACE_H
