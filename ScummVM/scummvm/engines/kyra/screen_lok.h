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

#ifndef KYRA_SCREEN_LOK_H
#define KYRA_SCREEN_LOK_H

#include "kyra/screen.h"

namespace Kyra {

class KyraEngine_LoK;

class Screen_LoK : public Screen {
public:
	Screen_LoK(KyraEngine_LoK *vm, OSystem *system);
	virtual ~Screen_LoK();

	bool init();

	int getRectSize(int w, int h);

	void setTextColorMap(const uint8 *cmap);

	void fadeSpecialPalette(int palIndex, int startIndex, int size, int fadeTime);

	void savePageToDisk(const char *file, int page);
	void loadPageFromDisk(const char *file, int page);
	void queryPageFromDisk(const char *file, int page, uint8 *buffer);
	void deletePageFromDisk(int page);

	void copyBackgroundBlock(int x, int page, int flag);
	void copyBackgroundBlock2(int x);

	void addBitBlitRect(int x, int y, int w, int h);
	void bitBlitRects();

	// AMIGA specific
	virtual void postProcessCursor(uint8 *data, int width, int height, int pitch);

protected:
	enum {
		kNumBitBlitRects = 10
	};

	KyraEngine_LoK *_vm;

	static const ScreenDim _screenDimTable[];
	static const int _screenDimTableCount;

	Common::Rect _bitBlitRects[kNumBitBlitRects];
	int _bitBlitNum;
	uint8 *_unkPtr1, *_unkPtr2;

	uint8 *_saveLoadPage[8];
	uint8 *_saveLoadPageOvl[8];
};

class Screen_LoK_16 : public Screen_LoK {
public:
	Screen_LoK_16(KyraEngine_LoK *vm, OSystem *system);

	void setScreenPalette(const Palette &pal);

	void fadePalette(const Palette &pal, int delay, const UpdateFunctor *upFunc = 0);
	void getFadeParams(const Palette &pal, int delay, int &delayInc, int &diff);
	int fadePalStep(const Palette &pal, int diff);
private:
	void updateDirtyRectsOvl();

	void convertTo16Colors(uint8 *page, int w, int h, int pitch, int keyColor = -1);
	void postProcessCursor(uint8 *data, int width, int height, int pitch) {
		convertTo16Colors(data, width, height, pitch, _cursorColorKey);
	}
	void mergeOverlay(int x, int y, int w, int h);

	void set16ColorPalette(const uint8 *pal);

	void paletteMap(uint8 idx, int r, int g, int b);

	struct PaletteDither {
		uint8 bestMatch;
		uint8 invertMatch;
	};

	PaletteDither _paletteDither[256];

	static const uint8 _palette16[48];
};

} // End of namespace Kyra

#endif
