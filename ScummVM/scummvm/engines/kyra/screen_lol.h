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

#ifdef ENABLE_LOL

#ifndef KYRA_SCREEN_LOL_H
#define KYRA_SCREEN_LOL_H

#include "kyra/screen_v2.h"

namespace Kyra {

class LoLEngine;

class Screen_LoL : public Screen_v2 {
public:
	Screen_LoL(LoLEngine *vm, OSystem *system);
	~Screen_LoL();

	void fprintString(const char *format, int x, int y, uint8 col1, uint8 col2, uint16 flags, ...) GCC_PRINTF(2, 8);
	void fprintStringIntro(const char *format, int x, int y, uint8 c1, uint8 c2, uint8 c3, uint16 flags, ...) GCC_PRINTF(2, 9);

	void drawShadedBox(int x1, int y1, int x2, int y2, int color1, int color2);

	void drawGridBox(int x, int y, int w, int h, int col);
	void fadeClearSceneWindow(int delay);

	// smooth scrolling
	void backupSceneWindow(int srcPageNum, int dstPageNum);
	void restoreSceneWindow(int srcPageNum, int dstPageNum);
	void clearGuiShapeMemory(int pageNum);
	void copyGuiShapeFromSceneBackupBuffer(int srcPageNum, int dstPageNum);
	void copyGuiShapeToSurface(int srcPageNum, int dstPageNum);
	void smoothScrollZoomStepTop(int srcPageNum, int dstPageNum, int x, int y);
	void smoothScrollZoomStepBottom(int srcPageNum, int dstPageNum, int x, int y);
	void smoothScrollHorizontalStep(int pageNum, int x, int u2, int w);
	void smoothScrollTurnStep1(int srcPage1Num, int srcPage2Num, int dstPageNum);
	void smoothScrollTurnStep2(int srcPage1Num, int srcPage2Num, int dstPageNum);
	void smoothScrollTurnStep3(int srcPage1Num, int srcPage2Num, int dstPageNum);

	void copyRegionSpecial(int page1, int w1, int h1, int x1, int y1, int page2, int w2, int h2, int x2, int y2, int w3, int h3, int mode, ...);

	// palette stuff
	void fadeToBlack(int delay=0x54, const UpdateFunctor *upFunc = 0);
	void fadeToPalette1(int delay);
	void loadSpecialColors(Palette &dst);
	void copyColor(int dstColorIndex, int srcColorIndex);
	bool fadeColor(int dstColorIndex, int srcColorIndex, uint32 elapsedTicks, uint32 totalTicks);
	Palette **generateFadeTable(Palette **dst, Palette *src1, Palette *src2, int numTabs);

	void generateGrayOverlay(const Palette &Pal, uint8 *grayOverlay, int factor, int addR, int addG, int addB, int lastColor, bool skipSpecialColors);
	uint8 *getLevelOverlay(int index) { return _levelOverlays[index]; }

	void createTransparencyTablesIntern(const uint8 *ovl, int a, const uint8 *fxPal1, const uint8 *fxPal2, uint8 *outTable1, uint8 *outTable2, int b);

	void copyBlockAndApplyOverlay(int page1, int x1, int y1, int page2, int x2, int y2, int w, int h, int dim, uint8 *ovl);
	void applyOverlaySpecial(int page1, int x1, int y1, int page2, int x2, int y2, int w, int h, int dim, int flag, uint8 *ovl);

	void copyBlockAndApplyOverlayOutro(int srcPage, int dstPage, const uint8 *ovl);

	uint8 getShapePaletteSize(const uint8 *shp);

	uint8 *_paletteOverlay1;
	uint8 *_paletteOverlay2;
	uint8 *_grayOverlay;
	int _fadeFlag;

	// PC98 specific
	static void convertPC98Gfx(uint8 *data, int w, int h, int pitch);

private:
	static const ScreenDim _screenDimTable256C[];
	static const ScreenDim _screenDimTable16C[];
	static const int _screenDimTableCount;

	uint8 *_levelOverlays[8];

	void mergeOverlay(int x, int y, int w, int h);
	void postProcessCursor(uint8 *data, int width, int height, int pitch);
};

} // End of namespace Kyra

#endif

#endif // ENABLE_LOL
