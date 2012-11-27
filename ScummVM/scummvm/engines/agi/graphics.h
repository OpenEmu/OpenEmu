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

#ifndef AGI_GRAPHICS_H
#define AGI_GRAPHICS_H

#include "agi/font.h"

namespace Agi {

#define GFX_WIDTH	320
#define GFX_HEIGHT	200
#define CHAR_COLS	8
#define CHAR_LINES	8

class AgiEngine;

class GfxMgr {
private:
	AgiBase *_vm;

	uint8 _palette[256 * 3];
	uint8 *_agiScreen;
	unsigned char *_screen;

	uint8 *_shakeH, *_shakeV;

	uint8 _agipalPalette[16 * 3];
	int _agipalFileNum;
	int _currentCursorPalette;	// 0 - palette not set, 1 - PC, 2 - Amiga

private:
	void rawDrawButton(int x, int y, const char *s, int fgcolor, int bgcolor, bool border, int textOffset);

public:
	GfxMgr(AgiBase *vm);

	void gfxPutBlock(int x1, int y1, int x2, int y2);

	void putTextCharacter(int, int, int, unsigned char, int, int, bool checkerboard = false, const uint8 *font = curFont);
	void shakeScreen(int);
	void shakeStart();
	void shakeEnd();
	void saveScreen();
	void restoreScreen();

	int initVideo();
	int deinitVideo();
	void scheduleUpdate(int, int, int, int);
	void doUpdate();
	void putScreen();
	void flushBlock(int, int, int, int);
	void flushBlockA(int, int, int, int);
	void putPixelsA(int, int, int, uint8 *);
	void flushScreen();
	void clearScreen(int);
	void clearConsoleScreen(int);
	void drawBox(int, int, int, int, int, int, int);
	void drawDefaultStyleButton(int, int, const char *, int, int, int fgcolor = 0, int bgcolor = 0);
	void drawCurrentStyleButton(int x, int y, const char *label, bool hasFocus, bool pressed = false, bool positive = true);
	int testButton(int, int, const char *);
	void drawRectangle(int, int, int, int, int);
	void saveBlock(int, int, int, int, uint8 *);
	void restoreBlock(int, int, int, int, uint8 *);
	void initPalette(const uint8 *p, uint colorCount = 16, uint fromBits = 6, uint toBits = 8);
	void setAGIPal(int);
	int getAGIPalFileNum();
	void drawFrame(int x1, int y1, int x2, int y2, int c1, int c2);

	void putBlock(int x1, int y1, int x2, int y2);
	void gfxSetPalette();
	void setCursor(bool amigaStyleCursor = false, bool busy = false);
	void setCursorPalette(bool amigaStylePalette = false);

	void printCharacter(int, int, char, int, int);
	int initMachine();
	int deinitMachine();
};

} // End of namespace Agi

#endif /* AGI_GRAPHICS_H */
