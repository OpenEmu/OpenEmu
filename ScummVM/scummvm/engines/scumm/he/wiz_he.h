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

#if !defined(SCUMM_HE_WIZ_HE_H) && defined(ENABLE_HE)
#define SCUMM_HE_WIZ_HE_H

#include "common/rect.h"

namespace Scumm {

struct WizPolygon {
	Common::Point vert[5];
	Common::Rect bound;
	int id;
	int numVerts;
	bool flag;
};

struct WizImage {
	int resNum;
	int x1;
	int y1;
	int zorder;
	int state;
	int flags;
	int shadow;
	int field_390;
	int palette;
};

struct WizParameters {
	int field_0;
	byte filename[260];
	Common::Rect box;
	int processFlags;
	int processMode;
	int field_11C;
	int field_120;
	int field_124;
	int field_128;
	int field_12C;
	int field_130;
	int field_134;
	int field_138;
	int compType;
	int fileWriteMode;
	int angle;
	int scale;
	int polygonId1;
	int polygonId2;
	int resDefImgW;
	int resDefImgH;
	int sourceImage;
	int params1;
	int params2;
	uint8 remapColor[256];
	uint8 remapIndex[256];
	int remapNum;
	int dstResNum;
	uint16 fillColor;
	byte string1[4096];
	byte string2[4096];
	int field_2399;
	int field_239D;
	int field_23A1;
	int field_23A5;
	int field_23A9;
	int field_23AD;
	int field_23B1;
	int field_23B5;
	int field_23B9;
	int field_23BD;
	int field_23C1;
	int field_23C5;
	int field_23C9;
	int field_23CD;
	Common::Rect box2;
	int field_23DE;
	int spriteId;
	int spriteGroup;
	int field_23EA;
	WizImage img;
};

enum WizImageFlags {
	kWIFHasPalette = 0x1,
	kWIFRemapPalette = 0x2,
	kWIFPrint = 0x4,
	kWIFBlitToFrontVideoBuffer = 0x8,
	kWIFMarkBufferDirty = 0x10,
	kWIFBlitToMemBuffer = 0x20,
	kWIFIsPolygon = 0x40,
	kWIFFlipX = 0x400,
	kWIFFlipY = 0x800
};

enum WizProcessFlags {
	kWPFSetPos = 0x1,
	kWPFShadow = 0x4,
	kWPFScaled = 0x8,
	kWPFRotate = 0x10,
	kWPFNewFlags = 0x20,
	kWPFRemapPalette = 0x40,
	kWPFClipBox = 0x200,
	kWPFNewState = 0x400,
	kWPFUseFile = 0x800,
	kWPFUseDefImgWidth = 0x2000,
	kWPFUseDefImgHeight = 0x4000,
	kWPFPaletteNum = 0x8000,
	kWPFDstResNum = 0x10000,
	kWPFFillColor = 0x20000,
	kWPFClipBox2 = 0x40000,
	kWPFMaskImg = 0x80000,
	kWPFParams = 0x100000
};

enum {
	kWizXMap = 0,
	kWizRMap,
	kWizCopy
};

enum DstSurface {
	kDstScreen   = 0,
	kDstMemory   = 1,
	kDstResource = 2,
 	kDstCursor   = 3
};

class ScummEngine_v71he;

class Wiz {
public:
	enum {
		NUM_POLYGONS = 200,
		NUM_IMAGES   = 255
	};

	WizImage _images[NUM_IMAGES];
	uint16 _imagesNum;
	WizPolygon _polygons[NUM_POLYGONS];

	Wiz(ScummEngine_v71he *vm);

	void clearWizBuffer();
	Common::Rect _rectOverride;
	bool _cursorImage;
	bool _rectOverrideEnabled;

	void polygonClear();
	void polygonLoad(const uint8 *polData);
	void polygonStore(int id, bool flag, int vert1x, int vert1y, int vert2x, int vert2y, int vert3x, int vert3y, int vert4x, int vert4y);
	void polygonCalcBoundBox(Common::Point *vert, int numVerts, Common::Rect & bound);
	void polygonErase(int fromId, int toId);
	int polygonHit(int id, int x, int y);
	bool polygonDefined(int id);
	bool polygonContains(const WizPolygon &pol, int x, int y);
	void polygonRotatePoints(Common::Point *pts, int num, int alpha);
	void polygonTransform(int resNum, int state, int po_x, int po_y, int angle, int zoom, Common::Point *vert);

	void createWizEmptyImage(int resNum, int x1, int y1, int width, int height);
	void fillWizRect(const WizParameters *params);
	void fillWizLine(const WizParameters *params);
	void fillWizPixel(const WizParameters *params);
	void fillWizFlood(const WizParameters *params);
	void remapWizImagePal(const WizParameters *params);

	void getWizImageDim(int resNum, int state, int32 &w, int32 &h);
	int getWizImageStates(int resnum);
	int isWizPixelNonTransparent(int resnum, int state, int x, int y, int flags);
	uint16 getWizPixelColor(int resnum, int state, int x, int y);
	int getWizImageData(int resNum, int state, int type);

	void flushWizBuffer();

	void getWizImageSpot(int resId, int state, int32 &x, int32 &y);
	void loadWizCursor(int resId, int palette);

	void captureWizImage(int resNum, const Common::Rect& r, bool frontBuffer, int compType);
	void captureImage(uint8 *src, int srcPitch, int srcw, int srch, int resNum, const Common::Rect& r, int compType);
	void captureWizPolygon(int resNum, int maskNum, int maskState, int id1, int id2, int compType);
	void displayWizComplexImage(const WizParameters *params);
	void displayWizImage(WizImage *pwi);
	void processWizImage(const WizParameters *params);

	uint8 *drawWizImage(int resNum, int state, int maskNum, int maskState, int x1, int y1, int zorder, int shadow, int field_390, const Common::Rect *clipBox, int flags, int dstResNum, const uint8 *palPtr);
	void drawWizPolygon(int resNum, int state, int id, int flags, int shadow, int dstResNum, int palette);
	void drawWizComplexPolygon(int resNum, int state, int po_x, int po_y, int shadow, int angle, int zoom, const Common::Rect *r, int flags, int dstResNum, int palette);
	void drawWizPolygonTransform(int resNum, int state, Common::Point *wp, int flags, int shadow, int dstResNum, int palette);
	void drawWizPolygonImage(uint8 *dst, const uint8 *src, const uint8 *mask, int dstpitch, int dstType, int dstw, int dsth, int wizW, int wizH, Common::Rect &bound, Common::Point *wp, uint8 bitDepth);

#ifdef USE_RGB_COLOR
	static void copyMaskWizImage(uint8 *dst, const uint8 *src, const uint8 *mask, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr);
#endif

	static void copyAuxImage(uint8 *dst1, uint8 *dst2, const uint8 *src, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, uint8 bitdepth);
	static void copyWizImageWithMask(uint8 *dst, const uint8 *src, int dstPitch, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int maskT, int maskP);
	static void copyWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitdepth);
	static void copyRawWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *palPtr, int transColor, uint8 bitdepth);
#ifdef USE_RGB_COLOR
	static void copy16BitWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, const uint8 *xmapPtr);
	static void copyRaw16BitWizImage(uint8 *dst, const uint8 *src, int dstPitch, int dstType, int dstw, int dsth, int srcx, int srcy, int srcw, int srch, const Common::Rect *rect, int flags, int transColor);
	template<int type> static void decompress16BitWizImage(uint8 *dst, int dstPitch, int dstType, const uint8 *src, const Common::Rect &srcRect, int flags, const uint8 *xmapPtr = NULL);
#endif
	template<int type> static void decompressWizImage(uint8 *dst, int dstPitch, int dstType, const uint8 *src, const Common::Rect &srcRect, int flags, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitdepth);
	template<int type> static void decompressRawWizImage(uint8 *dst, int dstPitch, int dstType, const uint8 *src, int srcPitch, int w, int h, int transColor, const uint8 *palPtr, uint8 bitdepth);

#ifdef USE_RGB_COLOR
	template<int type> static void write16BitColor(uint8 *dst, const uint8 *src, int dstType, const uint8 *xmapPtr);
#endif
	template<int type> static void write8BitColor(uint8 *dst, const uint8 *src, int dstType, const uint8 *palPtr, const uint8 *xmapPtr, uint8 bitDepth);
	static void writeColor(uint8 *dstPtr, int dstType, uint16 color);

	int isWizPixelNonTransparent(const uint8 *data, int x, int y, int w, int h, uint8 bitdepth);
	uint16 getWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 bitDepth, uint16 color);
	uint16 getRawWizPixelColor(const uint8 *data, int x, int y, int w, int h, uint8 bitDepth, uint16 color);
	void computeWizHistogram(uint32 *histogram, const uint8 *data, const Common::Rect& rCapt);
	void computeRawWizHistogram(uint32 *histogram, const uint8 *data, int srcPitch, const Common::Rect& rCapt);

private:
	ScummEngine_v71he *_vm;
};

} // End of namespace Scumm

#endif
