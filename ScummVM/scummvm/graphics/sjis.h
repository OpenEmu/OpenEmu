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
 */

// The code in this file is currently only used in KYRA and SCI.
// So if neither of those is enabled, we will skip compiling it.
// We also enable this code for ScummVM builds including support
// for dynamic engine plugins.
// If you plan to use this code in another engine, you will have
// to add the proper define check here.
#if !(defined(ENABLE_KYRA) || defined(ENABLE_SCI) || defined(ENABLE_SCUMM) || defined(DYNAMIC_MODULES))

// If neither of the above mentioned is enabled, do not include the SJIS code.

#else

#ifndef GRAPHICS_SJIS_H
#define GRAPHICS_SJIS_H

#ifdef __DS__
/* This disables the flipped mode which is used in FM-Towns versions
 * of Monkey Island 1 (and maybe other SCUMM 5 games). These are not supported
 * on the DS, so it makes sense to have a corresponding setting here.
 */
#define DISABLE_FLIPPED_MODE
#endif

#include "common/scummsys.h"
#include "common/platform.h"

namespace Graphics {

struct Surface;

/**
 * A font that is able to draw SJIS encoded characters.
 */
class FontSJIS {
public:
	virtual ~FontSJIS() {}

	/**
	 * Creates the first SJIS font, which ROM/font file is present.
	 * It will also call loadData, so the user can just start
	 * using the font.
	 *
	 * It'll prefer the platform specific ROM file, when platform
	 * is set to a value, which's font ROM is supported.
	 * So far that is only kPlatformFMTowns.
	 *
	 * The last file tried is ScummVM's SJIS.FNT file.
	 */
	static FontSJIS *createFont(const Common::Platform platform = Common::kPlatformUnknown);

	/**
	 * Load the font data.
	 */
	virtual bool loadData() = 0;

	/**
	 * Enable drawing with outline or shadow if supported by the Font.
	 *
	 * After changing outline state, getFontHeight and getMaxFontWidth / getCharWidth might return
	 * different values!
	 */
	enum DrawingMode {
		kDefaultMode,
		kOutlineMode,
		kShadowMode,
		kFMTownsShadowMode
	};

	virtual void setDrawingMode(DrawingMode mode) {}

	/**
	 * Enable flipped character drawing if supported by the Font (e.g. in the MI1 circus scene after Guybrush gets shot out of the cannon).
	 */
	virtual void toggleFlippedMode(bool enable) {}

	/**
	 * Set spacing between characters and lines. This affects font height / char width
	 */
	virtual void setCharSpacing(int spacing) {}
	virtual void setLineSpacing(int spacing) {}

	/**
	 * Returns the height of the font.
	 */
	virtual uint getFontHeight() const = 0;

	/**
	 * Returns the max. width of the font.
	 */
	virtual uint getMaxFontWidth() const = 0;

	/**
	 * Returns the width of a specific character.
	 */
	virtual uint getCharWidth(uint16 ch) const = 0;

	/**
	 * Draws a SJIS encoded character on the given surface.
	 */
	void drawChar(Graphics::Surface &dst, uint16 ch, int x, int y, uint32 c1, uint32 c2) const;

	/**
	 * Draws a SJIS char on the given raw buffer.
	 *
	 * @param dst   pointer to the destination
	 * @param ch    character to draw (in little endian)
	 * @param pitch pitch of the destination buffer (size in *bytes*)
	 * @param bpp   bytes per pixel of the destination buffer
	 * @param c1    forground color
	 * @param c2    outline color
	 * @param maxW  max draw width (to ensure that character drawing takes place within surface boundaries), -1 = no check
	 * @param maxH  max draw height (to ensure that character drawing takes place within surface boundaries), -1 = no check
	 */
	virtual void drawChar(void *dst, uint16 ch, int pitch, int bpp, uint32 c1, uint32 c2, int maxW, int maxH) const = 0;
};

/**
 * A base class to render monochrome SJIS fonts.
 */
class FontSJISBase : public FontSJIS {
public:
	FontSJISBase();

	virtual void setDrawingMode(DrawingMode mode);

	virtual void toggleFlippedMode(bool enable);

	virtual uint getFontHeight() const;

	virtual uint getMaxFontWidth() const;

	virtual uint getCharWidth(uint16 ch) const;

	virtual void drawChar(void *dst, uint16 ch, int pitch, int bpp, uint32 c1, uint32 c2, int maxW, int maxH) const;
private:
	template<typename Color>
	void blitCharacter(const uint8 *glyph, const int w, const int h, uint8 *dst, int pitch, Color c) const;
	void createOutline(uint8 *outline, const uint8 *glyph, const int w, const int h) const;

#ifndef DISABLE_FLIPPED_MODE
	// This is used in the FM-Towns version of Monkey Island 1
	// when Guybrush gets shot out of the cannon in the circus tent.
	const uint8 *flipCharacter(const uint8 *glyph, const int w) const;
	mutable uint8 _tempGlyph[32];
#endif
protected:
	DrawingMode _drawMode;
	bool _flippedMode;
	int _fontWidth, _fontHeight;
	uint8 _bitPosNewLineMask;

	bool isASCII(uint16 ch) const;

	virtual const uint8 *getCharData(uint16 c) const = 0;

	enum DrawingFeature {
		kFeatDefault		= 1 << 0,
		kFeatOutline		= 1 << 1,
		kFeatShadow			= 1 << 2,
		kFeatFMTownsShadow	= 1 << 3,
		kFeatFlipped		= 1 << 4
	};

	virtual bool hasFeature(int feat) const = 0;
};

/**
 * FM-TOWNS ROM based SJIS compatible font.
 *
 * This is used in KYRA, SCUMM and SCI.
 */
class FontTowns : public FontSJISBase {
public:
	/**
	 * Loads the ROM data from "FMT_FNT.ROM".
	 */
	bool loadData();
private:
	enum {
		kFont16x16Chars = 7808,
		kFont8x16Chars = 256
	};

	uint8 _fontData16x16[kFont16x16Chars * 32];
	uint8 _fontData8x16[kFont8x16Chars * 32];

	virtual const uint8 *getCharData(uint16 c) const;

	bool hasFeature(int feat) const;
};

/**
 * PC-Engine System Card based SJIS compatible font.
 *
 * This is used in LOOM.
 */
class FontPCEngine : public FontSJISBase {
public:
	/**
	 * Loads the ROM data from "pce.cdbios".
	 */
	bool loadData();
private:
	enum {
		kFont12x12Chars = 3418
	};

	uint8 _fontData12x12[kFont12x12Chars * 18];

	virtual const uint8 *getCharData(uint16 c) const;

	bool hasFeature(int feat) const;
};

/**
 * Our custom SJIS FNT.
 */
class FontSjisSVM : public FontSJISBase {
public:
	FontSjisSVM(const Common::Platform platform);
	~FontSjisSVM();

	/**
	 * Load the font data from "SJIS.FNT".
	 */
	bool loadData();
private:
	uint8 *_fontData16x16;
	uint _fontData16x16Size;

	uint8 *_fontData8x16;
	uint _fontData8x16Size;

	uint8 *_fontData12x12;
	uint _fontData12x12Size;

	virtual const uint8 *getCharData(uint16 c) const;

	bool hasFeature(int feat) const;

	const uint8 *getCharDataPCE(uint16 c) const;
	const uint8 *getCharDataDefault(uint16 c) const;

	void mapKANJIChar(const uint8 fB, const uint8 sB, int &base, int &index) const;

	enum {
		kSjisFontVersion = 3
	};
};

// TODO: Consider adding support for PC98 ROM

} // End of namespace Graphics

#endif

#endif // engine and dynamic plugins guard
