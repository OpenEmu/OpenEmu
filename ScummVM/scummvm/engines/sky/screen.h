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

#ifndef SKY_SCREEN_H
#define SKY_SCREEN_H


#include "common/scummsys.h"
#include "sky/skydefs.h"

class OSystem;

namespace Common {
struct Rect;
}

namespace Sky {

class Disk;
class SkyEngine;
class SkyCompact;
struct Compact;
struct DataFileHeader;

#define SCROLL_JUMP		16
#define VGA_COLORS		256
#define GAME_COLORS		240

#define FORE 1
#define BACK 0

typedef struct {
	uint16 yCood;
	Compact *compact;
	DataFileHeader *sprite;
} StSortList;

class Screen {
public:
	Screen(OSystem *pSystem, Disk *pDisk, SkyCompact *skyCompact);
	~Screen();
	void setPalette(uint8 *pal);
	void setPaletteEndian(uint8 *pal);
	void setPalette(uint16 fileNum);
	void paletteFadeUp(uint8 *pal);
	void paletteFadeUp(uint16 fileNr);

	void showScreen(uint16 fileNum);
	void showScreen(uint8 *pScreen);

	void handleTimer();
	void startSequence(uint16 fileNum);
	void startSequenceItem(uint16 itemNum);
	void stopSequence();
	bool sequenceRunning() { return _seqInfo.running; }
	void processSequence();
	void waitForSequence();
	uint32 seqFramesLeft() { return _seqInfo.framesLeft; }
	uint8 *giveCurrent() { return _currentScreen; }
	void halvePalette();

	//- regular screen.asm routines
	void forceRefresh() { memset(_gameGrid, 0x80, GRID_X * GRID_Y); }
	void fnFadeUp(uint32 palNum, uint32 scroll);
	void fnFadeDown(uint32 scroll);
	void fnDrawScreen(uint32 palette, uint32 scroll);
	void clearScreen();
	void setFocusRectangle(const Common::Rect& rect);

	void recreate();
	void flip(bool doUpdate = true);

	void spriteEngine();

	void paintBox(uint16 x, uint16 y);
	void showGrid(uint8 *gridBuf);

private:
	OSystem *_system;
	Disk *_skyDisk;
	SkyCompact *_skyCompact;
	static uint8 _top16Colors[16 * 3];
	uint8 _palette[VGA_COLORS * 3];
	uint32 _currentPalette;
	uint8 _seqGrid[20 * 12];

	void waitForTick();

	uint8 *_gameGrid;
	uint8 *_currentScreen;
	uint8 *_scrollScreen;
	struct {
		uint32 nextFrame;
		uint32 framesLeft;
		uint8 *seqData;
		uint8 *seqDataPos;
		volatile bool running;
		bool runningItem; // when playing an item, don't free it afterwards.
	} _seqInfo;

	//- more regular screen.asm + layer.asm routines
	void convertPalette(uint8 *inPal, uint8* outPal);
	void palette_fadedown_helper(uint8 *pal, uint num);

	//- sprite.asm routines
	// fixme: get rid of these globals
	uint32 _sprWidth, _sprHeight, _sprX, _sprY, _maskX1, _maskX2;
	void doSprites(uint8 layer);
	void sortSprites();
	void drawSprite(uint8 *spriteData, Compact *sprCompact);
	void verticalMask();
	void vertMaskSub(uint16 *grid, uint32 gridOfs, uint8 *screenPtr, uint32 layerId);
	void vectorToGame(uint8 gridVal);
};

} // End of namespace Sky

#endif //SKYSCREEN_H
