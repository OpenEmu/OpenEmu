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

#ifndef KYRA_SCREEN_EOB_H
#define KYRA_SCREEN_EOB_H

#ifdef ENABLE_EOB

#include "kyra/screen.h"

namespace Kyra {

class EoBCoreEngine;
class Screen_EoB : public Screen {
public:
	Screen_EoB(EoBCoreEngine *vm, OSystem *system);
	virtual ~Screen_EoB();

	bool init();

	void setClearScreenDim(int dim);
	void clearCurDim();

	void setMouseCursor(int x, int y, const byte *shape);
	void setMouseCursor(int x, int y, const byte *shape, const uint8 *ovl);

	void loadFileDataToPage(Common::SeekableReadStream *s, int pageNum, uint32 size);

	void printShadedText(const char *string, int x, int y, int col1, int col2);

	void loadEoBBitmap(const char *file, const uint8 *cgaMapping, int tempPage, int destPage, int convertToPage);
	void loadShapeSetBitmap(const char *file, int tempPage, int destPage);

	void convertPage(int srcPage, int dstPage, const uint8 *cgaMapping);

	void setScreenPalette(const Palette &pal);
	void getRealPalette(int num, uint8 *dst);

	uint8 *encodeShape(uint16 x, uint16 y, uint16 w, uint16 h, bool encode8bit = false, const uint8 *cgaMapping = 0);
	void drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd = -1, int flags = 0, ...);
	const uint8 *scaleShape(const uint8 *shapeData, int blockDistance);
	const uint8 *scaleShapeStep(const uint8 *shp);
	const uint8 *generateShapeOverlay(const uint8 *shp, int paletteOverlayIndex);

	void setShapeFrame(int x1, int y1, int x2, int y2);
	void setShapeFadeMode(uint8 i, bool b);

	void setGfxParameters(int x, int y, int col);
	void drawExplosion(int scale, int radius, int numElements, int stepSize, int aspectRatio, const uint8 *colorTable, int colorTableSize);
	void drawVortex(int numElements, int radius, int stepSize, int, int disorder, const uint8 *colorTable, int colorTableSize);

	void fadeTextColor(Palette *pal, int color1, int fadeTextColor);
	bool delayedFadePalStep(Palette *fadePal, Palette *destPal, int rate);

	void setTextColorMap(const uint8 *cmap) {}
	int getRectSize(int w, int h);

	void setFadeTableIndex(int index);
	void createFadeTable(uint8 *palData, uint8 *dst, uint8 rootColor, uint8 weight);
	uint8 *getFadeTable(int index);

	const uint16 *getCGADitheringTable(int index);
	const uint8 *getEGADitheringTable();

private:
	void updateDirtyRects();
	void ditherRect(const uint8 *src, uint8 *dst, int dstPitch, int srcW, int srcH, int colorKey = -1);

	void drawShapeSetPixel(uint8 *dst, uint8 col);
	void scaleShapeProcessLine2Bit(uint8 *&shpDst, const uint8 *&shpSrc, uint32 transOffsetDst, uint32 transOffsetSrc);
	void scaleShapeProcessLine4Bit(uint8 *&dst, const uint8 *&src);
	bool posWithinRect(int posX, int posY, int x1, int y1, int x2, int y2);

	void generateEGADitheringTable(const Palette &pal);
	void generateCGADitheringTables(const uint8 *mappingData);

	int _dsDiv, _dsRem, _dsScaleTrans;
	uint8 *_cgaScaleTable;
	int16 _gfxX, _gfxY;
	uint8 _gfxCol;
	const uint8 *_gfxMaxY;

	int16 _dsX1, _dsX2, _dsY1, _dsY2;
	bool _shapeFadeMode[2];
	uint16 _shapeFadeInternal;
	uint8 *_fadeData;
	int _fadeDataIndex;
	uint8 _shapeOverlay[16];

	uint8 *_dsTempPage;

	uint16 *_cgaDitheringTables[2];
	const uint8 *_cgaMappingDefault;

	uint8 *_egaDitheringTable;
	uint8 *_egaDitheringTempPage;

	static const uint8 _egaMatchTable[];
	static const ScreenDim _screenDimTable[];
	static const int _screenDimTableCount;
};

} // End of namespace Kyra

#endif // ENABLE_EOB

#endif
