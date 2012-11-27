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

#ifndef SCUMM_CHARSET_H
#define SCUMM_CHARSET_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "graphics/sjis.h"
#include "scumm/scumm.h"
#include "scumm/gfx.h"
#include "scumm/saveload.h"

namespace Scumm {

class ScummEngine;
class NutRenderer;
struct VirtScreen;

static inline bool checkSJISCode(byte c) {
	if ((c >= 0x80 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfd))
		return true;
	return false;
}


class CharsetRenderer {
public:

	Common::Rect _str;

	int _top;
	int _left, _startLeft;
	int _right;

protected:
	byte _color;

public:
	bool _center;

	bool _hasMask;	// True if "removable" text is visible somewhere (should be called _hasText or so)
	VirtScreenNumber _textScreenID;	// ID of the virtual screen on which the text is visible.

	bool _blitAlso;
	bool _firstChar;
	bool _disableOffsX;

protected:
	ScummEngine *_vm;
	int32 _curId;

public:
	CharsetRenderer(ScummEngine *vm);
	virtual ~CharsetRenderer();

	virtual void printChar(int chr, bool ignoreCharsetMask) = 0;
	virtual void drawChar(int chr, Graphics::Surface &s, int x, int y) {}

	int getStringWidth(int a, const byte *str);
	void addLinebreaks(int a, byte *str, int pos, int maxwidth);
	void translateColor();

	virtual void setCurID(int32 id) = 0;
	int getCurID() { return _curId; }

	virtual int getFontHeight() = 0;
	virtual int getCharHeight(byte chr) { return getFontHeight(); }
	virtual int getCharWidth(uint16 chr) = 0;

	virtual void setColor(byte color) { _color = color; translateColor(); }

	void saveLoadWithSerializer(Serializer *ser);
};

class CharsetRendererCommon : public CharsetRenderer {
protected:
	const byte *_fontPtr;
	int _bytesPerPixel;
	int _fontHeight;
	int _numChars;

	byte _shadowColor;
	bool _shadowMode;

public:
	CharsetRendererCommon(ScummEngine *vm);

	void setCurID(int32 id);

	virtual int getFontHeight();
};

class CharsetRendererClassic : public CharsetRendererCommon {
protected:
	virtual void drawBitsN(const Graphics::Surface &s, byte *dst, const byte *src, byte bpp, int drawTop, int width, int height);
	void printCharIntern(bool is2byte, const byte *charPtr, int origWidth, int origHeight, int width, int height, VirtScreen *vs, bool ignoreCharsetMask);
	virtual bool prepareDraw(uint16 chr);

	int _width, _height, _origWidth, _origHeight;
	int _offsX, _offsY;
	const byte *_charPtr;

	// On which virtual screen will be drawn right now
	VirtScreenNumber _drawScreen;

public:
	CharsetRendererClassic(ScummEngine *vm) : CharsetRendererCommon(vm) {}

	void printChar(int chr, bool ignoreCharsetMask);
	void drawChar(int chr, Graphics::Surface &s, int x, int y);

	int getCharWidth(uint16 chr);
};

#ifdef USE_RGB_COLOR
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
class CharsetRendererTownsClassic : public CharsetRendererClassic {
public:
	CharsetRendererTownsClassic(ScummEngine *vm);

	int getCharWidth(uint16 chr);
	int getFontHeight();

private:
	void drawBitsN(const Graphics::Surface &s, byte *dst, const byte *src, byte bpp, int drawTop, int width, int height);
	bool prepareDraw(uint16 chr);
	void setupShadowMode();
	bool useFontRomCharacter(uint16 chr);
	void processCharsetColors();

	uint16 _sjisCurChar;
};
#endif
#endif

class CharsetRendererNES : public CharsetRendererCommon {
protected:
	byte *_trTable;

	void drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height);

public:
	CharsetRendererNES(ScummEngine *vm) : CharsetRendererCommon(vm) {}

	void setCurID(int32 id) {}
	void printChar(int chr, bool ignoreCharsetMask);
	void drawChar(int chr, Graphics::Surface &s, int x, int y);

	int getFontHeight() { return 8; }
	int getCharWidth(uint16 chr) { return 8; }
};

class CharsetRendererV3 : public CharsetRendererCommon {
protected:
	virtual void enableShadow(bool enable);
	virtual void drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height);
	virtual int getDrawWidthIntern(uint16 chr);
	virtual int getDrawHeightIntern(uint16 chr);
	virtual void setDrawCharIntern(uint16 chr) {}

	const byte *_widthTable;

public:
	CharsetRendererV3(ScummEngine *vm) : CharsetRendererCommon(vm) {}

	void printChar(int chr, bool ignoreCharsetMask);
	void drawChar(int chr, Graphics::Surface &s, int x, int y);
	void setCurID(int32 id);
	void setColor(byte color);
	virtual int getCharWidth(uint16 chr);
};

class CharsetRendererTownsV3 : public CharsetRendererV3 {
public:
	CharsetRendererTownsV3(ScummEngine *vm);

	int getCharWidth(uint16 chr);
	int getFontHeight();

private:
	void enableShadow(bool enable);
	void drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height);
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
	int getDrawWidthIntern(uint16 chr);
	int getDrawHeightIntern(uint16 chr);
	void setDrawCharIntern(uint16 chr);
#endif
	uint16 _sjisCurChar;
};

#ifdef USE_RGB_COLOR
class CharsetRendererPCE : public CharsetRendererV3 {
private:
	void drawBits1(Graphics::Surface &dest, int x, int y, const byte *src, int drawTop, int width, int height);

	int getDrawWidthIntern(uint16 chr);
	int getDrawHeightIntern(uint16 chr);
	void setDrawCharIntern(uint16 chr);

	uint16 _sjisCurChar;

public:
	CharsetRendererPCE(ScummEngine *vm) : CharsetRendererV3(vm), _sjisCurChar(0) {}

	void setColor(byte color);
};
#endif

class CharsetRendererV2 : public CharsetRendererV3 {
protected:
	bool _deleteFontPtr;

public:
	CharsetRendererV2(ScummEngine *vm, Common::Language language);
	~CharsetRendererV2();

	void setCurID(int32 id) {}
	int getCharWidth(uint16 chr) { return 8; }
};

#ifdef ENABLE_SCUMM_7_8
class CharsetRendererNut : public CharsetRenderer {
protected:
	NutRenderer *_fr[5];
	NutRenderer *_current;

public:
	CharsetRendererNut(ScummEngine *vm);
	~CharsetRendererNut();

	void printChar(int chr, bool ignoreCharsetMask);

	void setCurID(int32 id);

	int getFontHeight();
	int getCharHeight(byte chr);
	int getCharWidth(uint16 chr);
};
#endif

} // End of namespace Scumm


#endif
