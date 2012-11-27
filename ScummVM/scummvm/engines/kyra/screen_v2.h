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

#ifndef KYRA_SCREEN_V2_H
#define KYRA_SCREEN_V2_H

#include "kyra/screen.h"
#include "kyra/kyra_v2.h"

namespace Kyra {

class Screen_v2 : public Screen {
public:
	Screen_v2(KyraEngine_v1 *vm, OSystem *system, const ScreenDim *dimTable, const int dimTableSize);
	~Screen_v2();

	// screen page handling
	void checkedPageUpdate(int srcPage, int dstPage);

	// palette handling
	uint8 *generateOverlay(const Palette &pal, uint8 *buffer, int color, uint weight, int maxColor = -1);
	void applyOverlay(int x, int y, int w, int h, int pageNum, const uint8 *overlay);
	int findLeastDifferentColor(const uint8 *paletteEntry, const Palette &pal, uint8 firstColor, uint16 numColors, bool skipSpecialColors = false);

	virtual void getFadeParams(const Palette &pal, int delay, int &delayInc, int &diff);

	bool timedPaletteFadeStep(uint8 *pal1, uint8 *pal2, uint32 elapsedTime, uint32 totalTime);

	// shape handling
	uint8 *getPtrToShape(uint8 *shpFile, int shape);
	const uint8 *getPtrToShape(const uint8 *shpFile, int shape);

	int getShapeScaledWidth(const uint8 *shpFile, int scale);
	int getShapeScaledHeight(const uint8 *shpFile, int scale);

	uint16 getShapeSize(const uint8 *shp);

	uint8 *makeShapeCopy(const uint8 *src, int index);

	// rect handling
	int getRectSize(int w, int h);
	bool calcBounds(int w0, int h0, int &x1, int &y1, int &w1, int &h1, int &x2, int &y2, int &w2);

	// text display
	void setTextColorMap(const uint8 *cmap);

	// layer handling
	virtual int getLayer(int x, int y);

	// special WSA handling
	void wsaFrameAnimationStep(int x1, int y1, int x2, int y2, int w1, int h1, int w2, int h2, int srcPage, int dstPage, int dim);

	// used in non-interactive HoF/LoL demos
	void copyPageMemory(int srcPage, int srcPos, int dstPage, int dstPos, int numBytes);
	void copyRegionEx(int srcPage, int srcW, int srcH, int dstPage, int dstX,int dstY, int dstW, int dstH, const ScreenDim *d, bool flag = false);
protected:
	uint8 *_wsaFrameAnimBuffer;
};

} // End of namespace Kyra

#endif
