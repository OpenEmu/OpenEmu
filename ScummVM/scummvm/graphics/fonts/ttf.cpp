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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Since FreeType2 includes files, which contain forbidden symbols, we need to
// allow all symbols here.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"
#ifdef USE_FREETYPE2

#include "graphics/fonts/ttf.h"
#include "graphics/font.h"
#include "graphics/surface.h"

#include "common/singleton.h"
#include "common/stream.h"
#include "common/hashmap.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

namespace Graphics {

namespace {

inline int ftCeil26_6(FT_Pos x) {
	return (x + 63) / 64;
}

} // End of anonymous namespace

class TTFLibrary : public Common::Singleton<TTFLibrary> {
public:
	TTFLibrary();
	~TTFLibrary();

	/**
	 * Check whether FreeType2 is initialized properly.
	 */
	bool isInitialized() const { return _initialized; }

	bool loadFont(const uint8 *file, const uint32 size, FT_Face &face);
	void closeFont(FT_Face &face);
private:
	FT_Library _library;
	bool _initialized;
};

void shutdownTTF() {
	TTFLibrary::destroy();
}

#define g_ttf ::Graphics::TTFLibrary::instance()

TTFLibrary::TTFLibrary() : _library(), _initialized(false) {
	if (!FT_Init_FreeType(&_library))
		_initialized = true;
}

TTFLibrary::~TTFLibrary() {
	if (_initialized) {
		FT_Done_FreeType(_library);
		_initialized = false;
	}
}

bool TTFLibrary::loadFont(const uint8 *file, const uint32 size, FT_Face &face) {
	assert(_initialized);

	return (FT_New_Memory_Face(_library, file, size, 0, &face) == 0);
}

void TTFLibrary::closeFont(FT_Face &face) {
	assert(_initialized);

	FT_Done_Face(face);
}

class TTFFont : public Font {
public:
	TTFFont();
	virtual ~TTFFont();

	bool load(Common::SeekableReadStream &stream, int size, uint dpi, bool monochrome, const uint32 *mapping);

	virtual int getFontHeight() const;

	virtual int getMaxCharWidth() const;

	virtual int getCharWidth(byte chr) const;

	virtual int getKerningOffset(byte left, byte right) const;

	virtual void drawChar(Surface *dst, byte chr, int x, int y, uint32 color) const;
private:
	bool _initialized;
	FT_Face _face;

	uint8 *_ttfFile;
	uint32 _size;

	int _width, _height;
	int _ascent, _descent;

	struct Glyph {
		Surface image;
		int xOffset, yOffset;
		int advance;
	};

	bool cacheGlyph(Glyph &glyph, FT_UInt &slot, uint chr);
	typedef Common::HashMap<byte, Glyph> GlyphCache;
	GlyphCache _glyphs;

	FT_UInt _glyphSlots[256];

	bool _monochrome;
	bool _hasKerning;
};

TTFFont::TTFFont()
    : _initialized(false), _face(), _ttfFile(0), _size(0), _width(0), _height(0), _ascent(0),
      _descent(0), _glyphs(), _glyphSlots(), _monochrome(false), _hasKerning(false) {
}

TTFFont::~TTFFont() {
	if (_initialized) {
		g_ttf.closeFont(_face);

		delete[] _ttfFile;
		_ttfFile = 0;

		for (GlyphCache::iterator i = _glyphs.begin(), end = _glyphs.end(); i != end; ++i)
			i->_value.image.free();

		_initialized = false;
	}
}

bool TTFFont::load(Common::SeekableReadStream &stream, int size, uint dpi, bool monochrome, const uint32 *mapping) {
	if (!g_ttf.isInitialized())
		return false;

	_size = stream.size();
	if (!_size)
		return false;

	_ttfFile = new uint8[_size];
	assert(_ttfFile);

	if (stream.read(_ttfFile, _size) != _size) {
		delete[] _ttfFile;
		_ttfFile = 0;

		return false;
	}

	if (!g_ttf.loadFont(_ttfFile, _size, _face)) {
		delete[] _ttfFile;
		_ttfFile = 0;

		return false;
	}

	// We only support scalable fonts.
	if (!FT_IS_SCALABLE(_face)) {
		delete[] _ttfFile;
		_ttfFile = 0;

		g_ttf.closeFont(_face);

		return false;
	}

	// Check whether we have kerning support
	_hasKerning = (FT_HAS_KERNING(_face) != 0);

	if (FT_Set_Char_Size(_face, 0, size * 64, dpi, dpi)) {
		delete[] _ttfFile;
		_ttfFile = 0;

		return false;
	}

	_monochrome = monochrome;

	FT_Fixed yScale = _face->size->metrics.y_scale;
	_ascent = ftCeil26_6(FT_MulFix(_face->ascender, yScale));
	_descent = ftCeil26_6(FT_MulFix(_face->descender, yScale));

	_width = ftCeil26_6(FT_MulFix(_face->max_advance_width, _face->size->metrics.x_scale));
	_height = _ascent - _descent + 1;

	if (!mapping) {
		// Load all ISO-8859-1 characters.
		for (uint i = 0; i < 256; ++i) {
			if (!cacheGlyph(_glyphs[i], _glyphSlots[i], i))
				_glyphSlots[i] = 0;
		}
	} else {
		for (uint i = 0; i < 256; ++i) {
			const uint32 unicode = mapping[i] & 0x7FFFFFFF;
			const bool isRequired = (mapping[i] & 0x80000000) != 0;
			// Check whether loading an important glyph fails and error out if
			// that is the case.
			if (!cacheGlyph(_glyphs[i], _glyphSlots[i], unicode)) {
				_glyphSlots[i] = 0;
				if (isRequired)
					return false;
			}
		}
	}

	_initialized = (_glyphs.size() != 0);
	return _initialized;
}

int TTFFont::getFontHeight() const {
	return _height;
}

int TTFFont::getMaxCharWidth() const {
	return _width;
}

int TTFFont::getCharWidth(byte chr) const {
	GlyphCache::const_iterator glyphEntry = _glyphs.find(chr);
	if (glyphEntry == _glyphs.end())
		return 0;
	else
		return glyphEntry->_value.advance;
}

int TTFFont::getKerningOffset(byte left, byte right) const {
	if (!_hasKerning)
		return 0;

	FT_UInt leftGlyph = _glyphSlots[left];
	FT_UInt rightGlyph = _glyphSlots[right];

	if (!leftGlyph || !rightGlyph)
		return 0;

	FT_Vector kerningVector;
	FT_Get_Kerning(_face, leftGlyph, rightGlyph, FT_KERNING_DEFAULT, &kerningVector);
	return (kerningVector.x / 64);
}

namespace {

template<typename ColorType>
void renderGlyph(uint8 *dstPos, const int dstPitch, const uint8 *srcPos, const int srcPitch, const int w, const int h, ColorType color, const PixelFormat &dstFormat) {
	uint8 sR, sG, sB;
	dstFormat.colorToRGB(color, sR, sG, sB);

	for (int y = 0; y < h; ++y) {
		ColorType *rDst = (ColorType *)dstPos;
		const uint8 *src = srcPos;

		for (int x = 0; x < w; ++x) {
			if (*src == 255) {
				*rDst = color;
			} else if (*src) {
				const uint8 a = *src;

				uint8 dR, dG, dB;
				dstFormat.colorToRGB(*rDst, dR, dG, dB);

				dR = ((255 - a) * dR + a * sR) / 255;
				dG = ((255 - a) * dG + a * sG) / 255;
				dB = ((255 - a) * dB + a * sB) / 255;

				*rDst = dstFormat.RGBToColor(dR, dG, dB);
			}

			++rDst;
			++src;
		}

		dstPos += dstPitch;
		srcPos += srcPitch;
	}
}

} // End of anonymous namespace

void TTFFont::drawChar(Surface *dst, byte chr, int x, int y, uint32 color) const {
	GlyphCache::const_iterator glyphEntry = _glyphs.find(chr);
	if (glyphEntry == _glyphs.end())
		return;

	const Glyph &glyph = glyphEntry->_value;

	x += glyph.xOffset;
	y += glyph.yOffset;

	if (x > dst->w)
		return;
	if (y > dst->h)
		return;

	int w = glyph.image.w;
	int h = glyph.image.h;

	const uint8 *srcPos = (const uint8 *)glyph.image.getBasePtr(0, 0);

	// Make sure we are not drawing outside the screen bounds
	if (x < 0) {
		srcPos -= x;
		w += x;
		x = 0;
	}

	if (x + w > dst->w)
		w = dst->w - x;

	if (w <= 0)
		return;

	if (y < 0) {
		srcPos -= y * glyph.image.pitch;
		h += y;
		y = 0;
	}

	if (y + h > dst->h)
		h = dst->h - y;

	if (h <= 0)
		return;

	uint8 *dstPos = (uint8 *)dst->getBasePtr(x, y);

	if (dst->format.bytesPerPixel == 1) {
		for (int cy = 0; cy < h; ++cy) {
			uint8 *rDst = dstPos;
			const uint8 *src = srcPos;

			for (int cx = 0; cx < w; ++cx) {
				// We assume a 1Bpp mode is a color indexed mode, thus we can
				// not take advantage of anti-aliasing here.
				if (*src >= 0x80)
					*rDst = color;

				++rDst;
				++src;
			}

			dstPos += dst->pitch;
			srcPos += glyph.image.pitch;
		}
	} else if (dst->format.bytesPerPixel == 2) {
		renderGlyph<uint16>(dstPos, dst->pitch, srcPos, glyph.image.pitch, w, h, color, dst->format);
	} else if (dst->format.bytesPerPixel == 4) {
		renderGlyph<uint32>(dstPos, dst->pitch, srcPos, glyph.image.pitch, w, h, color, dst->format);
	}
}

bool TTFFont::cacheGlyph(Glyph &glyph, FT_UInt &slot, uint chr) {
	slot = FT_Get_Char_Index(_face, chr);
	if (!slot)
		return false;

	// We use the light target and render mode to improve the looks of the
	// glyphs. It is most noticable in FreeSansBold.ttf, where otherwise the
	// 't' glyph looks like it is cut off on the right side.
	if (FT_Load_Glyph(_face, slot, (_monochrome ? FT_LOAD_TARGET_MONO : FT_LOAD_TARGET_LIGHT)))
		return false;

	if (FT_Render_Glyph(_face->glyph, (_monochrome ? FT_RENDER_MODE_MONO : FT_RENDER_MODE_LIGHT)))
		return false;

	if (_face->glyph->format != FT_GLYPH_FORMAT_BITMAP)
		return false;

	FT_Glyph_Metrics &metrics = _face->glyph->metrics;

	glyph.xOffset = _face->glyph->bitmap_left;
	int xMax = glyph.xOffset + ftCeil26_6(metrics.width);
	glyph.yOffset = _ascent - _face->glyph->bitmap_top;

	glyph.advance = ftCeil26_6(_face->glyph->advance.x);

	// In case we got a negative xMin we adjust that, this might make some
	// characters make a bit odd, but it's the only way we can assure no
	// invalid memory writes with the current font API
	if (glyph.xOffset < 0) {
		xMax -= glyph.xOffset;
		glyph.xOffset = 0;

		if (xMax > glyph.advance)
			glyph.advance = xMax;
	}

	const FT_Bitmap &bitmap = _face->glyph->bitmap;
	glyph.image.create(bitmap.width, bitmap.rows, PixelFormat::createFormatCLUT8());

	const uint8 *src = bitmap.buffer;
	int srcPitch = bitmap.pitch;
	if (srcPitch < 0) {
		src += (bitmap.rows - 1) * srcPitch;
		srcPitch = -srcPitch;
	}

	uint8 *dst = (uint8 *)glyph.image.getBasePtr(0, 0);
	memset(dst, 0, glyph.image.h * glyph.image.pitch);

	switch (bitmap.pixel_mode) {
	case FT_PIXEL_MODE_MONO:
		for (int y = 0; y < bitmap.rows; ++y) {
			const uint8 *curSrc = src;
			uint8 mask = 0;

			for (int x = 0; x < bitmap.width; ++x) {
				if ((x % 8) == 0)
					mask = *curSrc++;

				if (mask & 0x80)
					*dst = 255;

				mask <<= 1;
				++dst;
			}

			src += srcPitch;
		}
		break;

	case FT_PIXEL_MODE_GRAY:
		for (int y = 0; y < bitmap.rows; ++y) {
			memcpy(dst, src, bitmap.width);
			dst += glyph.image.pitch;
			src += srcPitch;
		}
		break;

	default:
		warning("TTFFont::cacheGlyph: Unsupported pixel mode %d", bitmap.pixel_mode);
		return false;
	}

	return true;
}

Font *loadTTFFont(Common::SeekableReadStream &stream, int size, uint dpi, bool monochrome, const uint32 *mapping) {
	TTFFont *font = new TTFFont();

	if (!font->load(stream, size, dpi, monochrome, mapping)) {
		delete font;
		return 0;
	}

	return font;
}

} // End of namespace Graphics

namespace Common {
DECLARE_SINGLETON(Graphics::TTFLibrary);
} // End of namespace Common

#endif

