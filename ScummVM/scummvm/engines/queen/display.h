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

#ifndef QUEEN_DISPLAY_H
#define QUEEN_DISPLAY_H

#include "common/str.h"
#include "common/util.h"
#include "common/random.h"
#include "queen/defs.h"

class OSystem;

namespace Common {
struct Rect;
}

namespace Queen {

class QueenEngine;

class Display {
public:

	Display(QueenEngine *vm, OSystem *system);
	~Display();

	//! initialize dynalum for the specified room
	void dynalumInit(const char *roomName, uint16 roomNum);

	//! update dynalum for the current room
	void dynalumUpdate(int16 x, int16 y);

	//! update the palette
	void palSet(const uint8 *pal, int start, int end, bool updateScreen = false);

	//! setup palette for Joe's dress
	void palSetJoeDress();

	//! setup palette for Joe's normal clothes
	void palSetJoeNormal();

	//! setup palette for panel and inventory objects
	void palSetPanel();

	//! fade the current palette in
	void palFadeIn(uint16 roomNum, bool dynalum = false, int16 dynaX = 0, int16 dynaY = 0);

	//! fade the current palette out
	void palFadeOut(uint16 roomNum);

	//! grey the panel area (used when panel is disabled)
	void palGreyPanel();

	//! scroll some palette colors
	void palScroll(int start, int end);

	void palSetAmigaColor(uint8 color, uint16 rgb);

	//! custom palette effect for the specified room
	void palCustomColors(uint16 roomNum);

	//! custom palette scroll for the specified room
	void palCustomScroll(uint16 roomNum);

	//! process a 'palette flash' effect
	void palCustomFlash();

	void palCustomLightsOff(uint16 roomNum);
	void palCustomLightsOn(uint16 roomNum);

	//! mark all palette entries as dirty
	void palSetAllDirty() { _pal.dirtyMin = 0; _pal.dirtyMax = 255; }

	//! returns the number of colors used by the room
	int getNumColorsForRoom(uint16 room) const;

	//! returns true if we shouldn't fade the palette in the specified room
	bool isPalFadingDisabled(uint16 room) const;

	//! change fullscreen/panel mode
	void screenMode(int comPanel, bool inCutaway);

	void prepareUpdate();
	void update(bool dynalum = false, int16 dynaX = 0, int16 dynaY = 0);

	void setupPanel();
	void setupNewRoom(const char *name, uint16 room);

	void drawBobSprite(const uint8 *data, uint16 x, uint16 y, uint16 w, uint16 h, uint16 pitch, bool xflip);
	void drawBobPasteDown(const uint8 *data, uint16 x, uint16 y, uint16 w, uint16 h);
	void drawInventoryItem(const uint8 *data, uint16 x, uint16 y, uint16 w, uint16 h);

	void blit(uint8 *dstBuf, uint16 dstPitch, uint16 x, uint16 y, const uint8 *srcBuf, uint16 srcPitch, uint16 w, uint16 h, bool xflip, bool masked);
	void fill(uint8 *dstBuf, uint16 dstPitch, uint16 x, uint16 y, uint16 w, uint16 h, uint8 color);

	//! decode PCX picture data
	void decodePCX(const uint8 *src, uint32 srcSize, uint8 *dst, uint16 dstPitch, uint16 *w, uint16 *h, uint8 *pal, uint16 palStart, uint16 palEnd);

	//! decode ILBM picture data
	void decodeLBM(const uint8 *src, uint32 srcSize, uint8 *dst, uint16 dstPitch, uint16 *w, uint16 *h, uint8 *pal, uint16 palStart, uint16 palEnd, uint8 colorBase = 0);

	void horizontalScrollUpdate(int16 xCamera);
	void horizontalScroll(int16 scroll);
	int16 horizontalScroll() const { return _horizontalScroll; }

	void fullscreen(bool fs) { _fullRefresh = 2; _fullscreen = fs; }
	bool fullscreen() const { return _fullscreen; }

	//! mark the specified block as dirty
	void setDirtyBlock(uint16 x, uint16 y, uint16 w, uint16 h);

	//! force a full refresh (bypassing the dirtyblocks rendering), on next screen update
	void forceFullRefresh() { _fullRefresh = 2; }

	//! change mouse cursor bitmap
	void setMouseCursor(uint8 *buf, uint16 w, uint16 h);

	//! show/hide mouse cursor
	void showMouseCursor(bool show);

	//! initialize font, compute justification sizes
	void initFont();

	//! add the specified text to the texts list
	void setText(uint16 x, uint16 y, const char *text, bool outlined = true);

	//! add the specified text to the texts list
	void setTextCentered(uint16 y, const char *text, bool outlined = true);

	//! draw the text lists
	void drawTexts();

	//! remove entries from the texts list
	void clearTexts(uint16 y1, uint16 y2);

	void setupInkColors();

	uint8 getInkColor(InkColor color) const { return _inkColors[color]; }

	//! change the current text color
	void textCurrentColor(uint8 color) { _curTextColor = color; }

	//! change the text color for the specified texts list entry
	void textColor(uint16 y, uint8 color) { _texts[y].color = color; }

	//! Set the focus rectangle to the speaking character
	void setFocusRect(const Common::Rect& rect);

	int textCenterX(const char *text) const;
	uint16 textWidth(const char *text) const;
	uint16 textWidth(const char *text, uint16 len) const;
	void drawChar(uint16 x, uint16 y, uint8 color, const uint8 *chr);
	void drawText(uint16 x, uint16 y, uint8 color, const char *text, bool outlined = true);
	void drawBox(int16 x1, int16 y1, int16 x2, int16 y2, uint8 col);

	void shake(bool reset);

	void blankScreen();
	void blankScreenEffect1();
	void blankScreenEffect2();
	void blankScreenEffect3();

private:

	enum {
		FADE_SPEED = 16,
		D_BLOCK_W  =  8,
		D_BLOCK_H  =  8
	};

	enum BufferDimension {
		BACKDROP_W = 640,
		BACKDROP_H = 200,
		SCREEN_W   = 320,
		SCREEN_H   = 200,
		PANEL_W    = 320,
		PANEL_H    =  50
	};

	struct {
		uint8 *room;
		uint8 *screen;
		uint8 *panel;
		int dirtyMin, dirtyMax;
		bool scrollable;
	} _pal;

	struct Dynalum {
		bool valid;
		uint8 *mskBuf;
		uint32 mskSize;
		int8 *lumBuf;
		uint32 lumSize;
		uint8 prevColMask;
	};

	struct TextSlot {
		uint16 x;
		uint8 color;
		Common::String text;
		bool outlined;
	};

	uint8 *_screenBuf;
	uint8 *_panelBuf;
	uint8 *_backdropBuf;

	uint8 _fullRefresh;
	uint8 *_dirtyBlocks;
	uint16 _dirtyBlocksWidth, _dirtyBlocksHeight;

	bool _fullscreen;

	uint16 _horizontalScroll;
	uint16 _bdWidth, _bdHeight;

	const char *_imageExt;

	//! texts list
	TextSlot _texts[GAME_SCREEN_HEIGHT];

	//! current text color to use for display
	uint8 _curTextColor;

	//! font justification sizes
	uint8 _charWidth[256];

	uint8 _inkColors[INK_COUNT];

	Common::RandomSource _rnd;
	Dynalum _dynalum;
	OSystem *_system;
	QueenEngine *_vm;

	const uint8 *_font;

	static const uint8 _fontRegular[];
	static const uint8 _fontHebrew[];
	static const uint8 _fontRussian[];
	static const uint8 _fontGreek[];
	static const uint8 _palJoeClothes[];
	static const uint8 _palJoeDress[];
};


} // End of namespace Queen

#endif
