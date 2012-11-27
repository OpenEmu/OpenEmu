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

#ifndef KYRA_SCREEN_H
#define KYRA_SCREEN_H

#include "common/util.h"
#include "common/func.h"
#include "common/list.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/rendermode.h"
#include "common/stream.h"

class OSystem;

namespace Graphics {
class FontSJIS;
} // End of namespace Graphics

namespace Kyra {

typedef Common::Functor0<void> UpdateFunctor;

class KyraEngine_v1;
class Screen;

struct ScreenDim {
	uint16 sx;
	uint16 sy;
	uint16 w;
	uint16 h;
	uint16 unk8;
	uint16 unkA;
	uint16 unkC;
	uint16 unkE;
};

/**
 * A class that handles KYRA fonts.
 */
class Font {
public:
	virtual ~Font() {}

	/**
	 * Tries to load a file from the given stream
	 */
	virtual bool load(Common::SeekableReadStream &file) = 0;

	/**
	 * Whether the font draws on the overlay.
	 */
	virtual bool usesOverlay() const { return false; }

	/**
	 * The font height.
	 */
	virtual int getHeight() const = 0;

	/**
	 * The font width, this is the maximal character
	 * width.
	 */
	virtual int getWidth() const = 0;

	/**
	 * Gets the width of a specific character.
	 */
	virtual int getCharWidth(uint16 c) const = 0;

	/**
	 * Sets a text palette map. The map contains 16 entries.
	 */
	virtual void setColorMap(const uint8 *src) = 0;

	/**
	 * Draws a specific character.
	 *
	 * TODO/FIXME: Replace this with a nicer API. Currently
	 * the user has to assure that the character fits.
	 * We use this API, since it's hard to assure dirty rect
	 * handling from outside Screen.
	 */
	virtual void drawChar(uint16 c, byte *dst, int pitch) const = 0;
};

/**
 * Implementation of the Font interface for DOS fonts.
 *
 * TODO: Clean up the implementation. For example we might be able
 * to not to keep the whole font in memory.
 */
class DOSFont : public Font {
public:
	DOSFont();
	~DOSFont() { unload(); }

	bool load(Common::SeekableReadStream &file);
	int getHeight() const { return _height; }
	int getWidth() const { return _width; }
	int getCharWidth(uint16 c) const;
	void setColorMap(const uint8 *src) { _colorMap = src; }
	void drawChar(uint16 c, byte *dst, int pitch) const;

private:
	void unload();

	const uint8 *_colorMap;

	uint8 *_data;

	int _width, _height;

	int _numGlyphs;

	uint8 *_widthTable;
	uint8 *_heightTable;
	uint16 *_bitmapOffsets;
};

#ifdef ENABLE_EOB
/**
 * Implementation of the Font interface for old DOS fonts used
 * in EOB and EOB II.
 *
 */
class OldDOSFont : public Font {
public:
	OldDOSFont(Common::RenderMode mode);
	~OldDOSFont();

	bool load(Common::SeekableReadStream &file);
	int getHeight() const { return _height; }
	int getWidth() const { return _width; }
	int getCharWidth(uint16 c) const;
	void setColorMap(const uint8 *src) { _colorMap = src; }
	void drawChar(uint16 c, byte *dst, int pitch) const;

private:
	void unload();

	uint8 *_data;
	uint16 *_bitmapOffsets;

	int _width, _height;
	const uint8 *_colorMap;

	int _numGlyphs;

	Common::RenderMode _renderMode;

	static uint16 *_cgaDitheringTable;
	static int _numRef;
};
#endif // ENABLE_EOB

/**
 * Implementation of the Font interface for AMIGA fonts.
 */
class AMIGAFont : public Font {
public:
	AMIGAFont();
	~AMIGAFont() { unload(); }

	bool load(Common::SeekableReadStream &file);
	int getHeight() const { return _height; }
	int getWidth() const { return _width; }
	int getCharWidth(uint16 c) const;
	void setColorMap(const uint8 *src) {}
	void drawChar(uint16 c, byte *dst, int pitch) const;

private:
	void unload();

	int _width, _height;

	struct Character {
		uint8 yOffset, xOffset, width;

		struct Graphics {
			uint16 width, height;
			uint8 *bitmap;
		} graphics;
	};

	Character _chars[255];
};

/**
 * Implementation of the Font interface for FM-Towns/PC98 fonts
 */
class SJISFont : public Font {
public:
	SJISFont(Graphics::FontSJIS *font, const uint8 invisColor, bool is16Color, bool outlineSize);
	~SJISFont() { unload(); }

	bool usesOverlay() const { return true; }

	bool load(Common::SeekableReadStream &) { return true; }
	int getHeight() const;
	int getWidth() const;
	int getCharWidth(uint16 c) const;
	void setColorMap(const uint8 *src);
	void drawChar(uint16 c, byte *dst, int pitch) const;
private:
	void unload();

	const uint8 *_colorMap;

	Graphics::FontSJIS *_font;
	const uint8 _invisColor;
	const bool _is16Color;

	int _sjisWidth, _asciiWidth;
	int _fontHeight;
};

/**
 * A class that manages KYRA palettes.
 *
 * This class stores the palette data as VGA RGB internally.
 */
class Palette {
public:
	Palette(const int numColors);
	~Palette();

	enum {
		kVGABytesPerColor = 3,
		kPC98BytesPerColor = 3,
		kAmigaBytesPerColor = 2
	};

	/**
	 * Load a VGA palette from the given stream.
	 */
	void loadVGAPalette(Common::ReadStream &stream, int startIndex, int colors);

	/**
	 * Load a EGA palette from the given stream.
	 */
	void loadEGAPalette(Common::ReadStream &stream, int startIndex, int colors);

	/**
	 * Set default CGA palette. We only need the cyan/magenta/grey mode.
	 */
	enum CGAIntensity {
		kIntensityLow = 0,
		kIntensityHigh = 1
	};

	void setCGAPalette(int palIndex, CGAIntensity intensity);

	/**
	 * Load a AMIGA palette from the given stream.
	 */
	void loadAmigaPalette(Common::ReadStream &stream, int startIndex, int colors);

	/**
	 * Load a PC98 16 color palette from the given stream.
	 */
	void loadPC98Palette(Common::ReadStream &stream, int startIndex, int colors);

	/**
	 * Return the number of colors this palette manages.
	 */
	int getNumColors() const { return _numColors; }

	/**
	 * Set all palette colors to black.
	 */
	void clear();

	/**
	 * Fill the given indexes with the given component value.
	 *
	 * @param firstCol  the first color, which should be overwritten.
	 * @param numCols   number of colors, which schould be overwritten.
	 * @param value     color component value, which should be stored.
	 */
	void fill(int firstCol, int numCols, uint8 value);

	/**
	 * Copy data from another palette.
	 *
	 * @param source    palette to copy data from.
	 * @param firstCol  the first color of the source which should be copied.
	 * @param numCols   number of colors, which should be copied. -1 all remaining colors.
	 * @param dstStart  the first color, which should be ovewritten. If -1 firstCol will be used as start.
	 */
	void copy(const Palette &source, int firstCol = 0, int numCols = -1, int dstStart = -1);

	/**
	 * Copy data from a raw VGA palette.
	 *
	 * @param source    source buffer
	 * @param firstCol  the first color of the source which should be copied.
	 * @param numCols   number of colors, which should be copied.
	 * @param dstStart  the first color, which should be ovewritten. If -1 firstCol will be used as start.
	 */
	void copy(const uint8 *source, int firstCol, int numCols, int dstStart = -1);

	/**
	 * Fetch a RGB palette.
	 *
	 * @return a pointer to the RGB palette data, the client must delete[] it.
	 */
	uint8 *fetchRealPalette() const;

	//XXX
	uint8 &operator[](const int index) {
		assert(index >= 0 && index <= _numColors * 3);
		return _palData[index];
	}

	const uint8 &operator[](const int index) const {
		assert(index >= 0 && index <= _numColors * 3);
		return _palData[index];
	}

	/**
	 * Gets raw access to the palette.
	 *
	 * TODO: Get rid of this.
	 */
	uint8 *getData() { return _palData; }
	const uint8 *getData() const { return _palData; }

private:
	uint8 *_palData;
	const int _numColors;

	static const uint8 _egaColors[];
	static const int _egaNumColors;
	static const uint8 _cgaColors[4][12];
	static const int _cgaNumColors;
};

class Screen {
public:
	enum {
		SCREEN_W = 320,
		SCREEN_H = 200,
		SCREEN_PAGE_SIZE = 320 * 200 + 1024,
		SCREEN_OVL_SJIS_SIZE = 640 * 400,
		SCREEN_PAGE_NUM = 16,
		SCREEN_OVLS_NUM = 6
	};

	enum CopyRegionFlags {
		CR_NO_P_CHECK = 0x01
	};

	enum DrawShapeFlags {
		DSF_X_FLIPPED  = 0x01,
		DSF_Y_FLIPPED  = 0x02,
		DSF_SCALE      = 0x04,
		DSF_WND_COORDS = 0x10,
		DSF_CENTER     = 0x20
	};

	enum FontId {
		FID_6_FNT = 0,
		FID_8_FNT,
		FID_9_FNT,
		FID_CRED6_FNT,
		FID_CRED8_FNT,
		FID_BOOKFONT_FNT,
		FID_GOLDFONT_FNT,
		FID_INTRO_FNT,
		FID_SJIS_FNT,
		FID_NUM
	};

	Screen(KyraEngine_v1 *vm, OSystem *system, const ScreenDim *dimTable, const int dimTableSize);
	virtual ~Screen();

	// init
	virtual bool init();
	virtual void setResolution();

	void updateScreen();

	// debug functions
	bool queryScreenDebug() const { return _debugEnabled; }
	bool enableScreenDebug(bool enable);

	// page cur. functions
	int setCurPage(int pageNum);
	void clearCurPage();

	void copyWsaRect(int x, int y, int w, int h, int dimState, int plotFunc, const uint8 *src,
	                 int unk1, const uint8 *unkPtr1, const uint8 *unkPtr2);

	// page 0 functions
	void copyToPage0(int y, int h, uint8 page, uint8 *seqBuf);
	void shakeScreen(int times);

	// page functions
	void copyRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage, int flags=0);
	void copyPage(uint8 srcPage, uint8 dstPage);

	void copyRegionToBuffer(int pageNum, int x, int y, int w, int h, uint8 *dest);
	void copyBlockToPage(int pageNum, int x, int y, int w, int h, const uint8 *src);

	void shuffleScreen(int sx, int sy, int w, int h, int srcPage, int dstPage, int ticks, bool transparent);
	void fillRect(int x1, int y1, int x2, int y2, uint8 color, int pageNum = -1, bool xored = false);

	void clearPage(int pageNum);

	uint8 getPagePixel(int pageNum, int x, int y);
	void setPagePixel(int pageNum, int x, int y, uint8 color);

	const uint8 *getCPagePtr(int pageNum) const;
	uint8 *getPageRect(int pageNum, int x, int y, int w, int h);

	// palette handling
	void fadeFromBlack(int delay=0x54, const UpdateFunctor *upFunc = 0);
	void fadeToBlack(int delay=0x54, const UpdateFunctor *upFunc = 0);

	virtual void fadePalette(const Palette &pal, int delay, const UpdateFunctor *upFunc = 0);
	virtual void getFadeParams(const Palette &pal, int delay, int &delayInc, int &diff);
	virtual int fadePalStep(const Palette &pal, int diff);

	void setPaletteIndex(uint8 index, uint8 red, uint8 green, uint8 blue);
	virtual void setScreenPalette(const Palette &pal);

	// AMIGA version only
	bool isInterfacePaletteEnabled() const { return _interfacePaletteEnabled; }
	void enableInterfacePalette(bool e);
	void setInterfacePalette(const Palette &pal, uint8 r, uint8 g, uint8 b);

	virtual void getRealPalette(int num, uint8 *dst);
	Palette &getPalette(int num);
	void copyPalette(const int dst, const int src);

	// gui specific (processing on _curPage)
	void drawLine(bool vertical, int x, int y, int length, int color);
	void drawClippedLine(int x1, int y1, int x2, int y2, int color);
	virtual void drawShadedBox(int x1, int y1, int x2, int y2, int color1, int color2);
	void drawBox(int x1, int y1, int x2, int y2, int color);

	// font/text handling
	bool loadFont(FontId fontId, const char *filename);
	FontId setFont(FontId fontId);

	int getFontHeight() const;
	int getFontWidth() const;

	int getCharWidth(uint16 c) const;
	int getTextWidth(const char *str) const;

	void printText(const char *str, int x, int y, uint8 color1, uint8 color2);

	virtual void setTextColorMap(const uint8 *cmap) = 0;
	void setTextColor(const uint8 *cmap, int a, int b);

	const ScreenDim *getScreenDim(int dim) const;
	void modifyScreenDim(int dim, int x, int y, int w, int h);
	int screenDimTableCount() const { return _dimTableCount; }

	void setScreenDim(int dim);
	int curDimIndex() const { return _curDimIndex; }

	const ScreenDim *_curDim;

	// shape handling
	uint8 *encodeShape(int x, int y, int w, int h, int flags);

	int setNewShapeHeight(uint8 *shape, int height);
	int resetShapeHeight(uint8 *shape);

	virtual void drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, ...);

	// mouse handling
	void hideMouse();
	void showMouse();
	bool isMouseVisible() const;
	virtual void setMouseCursor(int x, int y, const byte *shape);

	// rect handling
	virtual int getRectSize(int w, int h) = 0;

	void rectClip(int &x, int &y, int w, int h);

	// misc
	void loadBitmap(const char *filename, int tempPage, int dstPage, Palette *pal, bool skip=false);

	virtual bool loadPalette(const char *filename, Palette &pal);
	bool loadPaletteTable(const char *filename, int firstPalette);
	virtual void loadPalette(const byte *data, Palette &pal, int bytes);

	void setAnimBlockPtr(int size);

	void setShapePages(int page1, int page2, int minY = -1, int maxY = 201);

	virtual byte getShapeFlag1(int x, int y);
	virtual byte getShapeFlag2(int x, int y);

	virtual int getDrawLayer(int x, int y);
	virtual int getDrawLayer2(int x, int y, int height);

	void blockInRegion(int x, int y, int width, int height);
	void blockOutRegion(int x, int y, int width, int height);

	int _charWidth;
	int _charOffset;
	int _curPage;
	uint8 *_shapePages[2];
	int _maskMinY, _maskMaxY;
	FontId _currentFont;

	// decoding functions
	static void decodeFrame1(const uint8 *src, uint8 *dst, uint32 size);
	static uint16 decodeEGAGetCode(const uint8 *&pos, uint8 &nib);

	static void decodeFrame3(const uint8 *src, uint8 *dst, uint32 size);
	static uint decodeFrame4(const uint8 *src, uint8 *dst, uint32 dstSize);
	static void decodeFrameDelta(uint8 *dst, const uint8 *src, bool noXor = false);
	static void decodeFrameDeltaPage(uint8 *dst, const uint8 *src, const int pitch, bool noXor);

	static void convertAmigaGfx(uint8 *data, int w, int h, int depth = 5, bool wsa = false, int bytesPerPlane = -1);
	static void convertAmigaMsc(uint8 *data);

	// RPG specific, this does not belong here
	void crossFadeRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage);

protected:
	uint8 *getPagePtr(int pageNum);
	virtual void updateDirtyRects();
	void updateDirtyRectsAmiga();
	void updateDirtyRectsOvl();

	void scale2x(byte *dst, int dstPitch, const byte *src, int srcPitch, int w, int h);
	virtual void mergeOverlay(int x, int y, int w, int h);

	// overlay specific
	byte *getOverlayPtr(int pageNum);
	void clearOverlayPage(int pageNum);
	void clearOverlayRect(int pageNum, int x, int y, int w, int h);
	void copyOverlayRegion(int x, int y, int x2, int y2, int w, int h, int srcPage, int dstPage);

	// font/text specific
	uint16 fetchChar(const char *&s) const;
	void drawChar(uint16 c, int x, int y);

	int16 encodeShapeAndCalculateSize(uint8 *from, uint8 *to, int size);

	template<bool noXor> static void wrapped_decodeFrameDelta(uint8 *dst, const uint8 *src);
	template<bool noXor> static void wrapped_decodeFrameDeltaPage(uint8 *dst, const uint8 *src, const int pitch);

	uint8 *_pagePtrs[16];
	uint8 *_sjisOverlayPtrs[SCREEN_OVLS_NUM];
	uint8 _pageMapping[SCREEN_PAGE_NUM];

	bool _useOverlays;
	bool _useSJIS;
	bool _use16ColorMode;
	bool _useHiResEGADithering;
	bool _isAmiga;
	Common::RenderMode _renderMode;

	uint8 _sjisInvisibleColor;

	Palette *_screenPalette;
	Common::Array<Palette *> _palettes;
	Palette *_internFadePalette;

	Font *_fonts[FID_NUM];
	uint8 _textColorsMap[16];

	uint8 *_decodeShapeBuffer;
	int _decodeShapeBufferSize;

	uint8 *_animBlockPtr;
	int _animBlockSize;

	// dimension handling
	const ScreenDim *const _dimTable;
	ScreenDim **_customDimTable;
	const int _dimTableCount;
	int _curDimIndex;

	// mouse handling
	int _mouseLockCount;
	const uint8 _cursorColorKey;

	virtual void postProcessCursor(uint8 *data, int w, int h, int pitch) {}

	enum {
		kMaxDirtyRects = 50
	};

	bool _forceFullUpdate;
	bool _paletteChanged;
	Common::List<Common::Rect> _dirtyRects;

	void addDirtyRect(int x, int y, int w, int h);

	OSystem *_system;
	KyraEngine_v1 *_vm;

	// shape
	int drawShapeMarginNoScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt);
	int drawShapeMarginNoScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt);
	int drawShapeMarginScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt);
	int drawShapeMarginScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt);
	int drawShapeSkipScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt);
	int drawShapeSkipScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt);
	void drawShapeProcessLineNoScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt, int16 scaleState);
	void drawShapeProcessLineNoScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt, int16 scaleState);
	void drawShapeProcessLineScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt, int16 scaleState);
	void drawShapeProcessLineScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt, int16 scaleState);

	void drawShapePlotType0(uint8 *dst, uint8 cmd);
	void drawShapePlotType1(uint8 *dst, uint8 cmd);
	void drawShapePlotType3_7(uint8 *dst, uint8 cmd);
	void drawShapePlotType4(uint8 *dst, uint8 cmd);
	void drawShapePlotType5(uint8 *dst, uint8 cmd);
	void drawShapePlotType6(uint8 *dst, uint8 cmd);
	void drawShapePlotType8(uint8 *dst, uint8 cmd);
	void drawShapePlotType9(uint8 *dst, uint8 cmd);
	void drawShapePlotType11_15(uint8 *dst, uint8 cmd);
	void drawShapePlotType12(uint8 *dst, uint8 cmd);
	void drawShapePlotType13(uint8 *dst, uint8 cmd);
	void drawShapePlotType14(uint8 *dst, uint8 cmd);
	void drawShapePlotType16(uint8 *dst, uint8 cmd);
	void drawShapePlotType20(uint8 *dst, uint8 cmd);
	void drawShapePlotType21(uint8 *dst, uint8 cmd);
	void drawShapePlotType33(uint8 *dst, uint8 cmd);
	void drawShapePlotType37(uint8 *dst, uint8 cmd);
	void drawShapePlotType48(uint8 *dst, uint8 cmd);
	void drawShapePlotType52(uint8 *dst, uint8 cmd);

	typedef int (Screen::*DsMarginSkipFunc)(uint8 *&dst, const uint8 *&src, int &cnt);
	typedef void (Screen::*DsLineFunc)(uint8 *&dst, const uint8 *&src, int &cnt, int16 scaleState);
	typedef void (Screen::*DsPlotFunc)(uint8 *dst, uint8 cmd);

	DsMarginSkipFunc _dsProcessMargin;
	DsMarginSkipFunc _dsScaleSkip;
	DsLineFunc _dsProcessLine;
	DsPlotFunc _dsPlot;

	const uint8 *_dsTable;
	int _dsTableLoopCount;
	const uint8 *_dsTable2;
	const uint8 *_dsTable3;
	const uint8 *_dsTable4;
	const uint8 *_dsTable5;
	int _dsDrawLayer;
	uint8 *_dsDstPage;
	int _dsTmpWidth;
	int _dsOffscreenLeft;
	int _dsOffscreenRight;
	int _dsScaleW;
	int _dsScaleH;
	int _dsOffscreenScaleVal1;
	int _dsOffscreenScaleVal2;
	int _drawShapeVar1;
	int _drawShapeVar3;
	int _drawShapeVar4;
	int _drawShapeVar5;

	// AMIGA version
	bool _interfacePaletteEnabled;

	// debug
	bool _debugEnabled;
};

} // End of namespace Kyra

#endif
