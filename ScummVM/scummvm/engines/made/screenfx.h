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

#ifndef MADE_SCREENFX_H
#define MADE_SCREENFX_H

#include "common/scummsys.h"

namespace Graphics {
struct Surface;
}

namespace Made {

class Screen;

struct BlendedPaletteStatus {
	bool _active;
	byte *_palette, *_newPalette;
	int _colorCount;
	int16 _value, _maxValue, _incr;
	int cnt;
};

class ScreenEffects {
public:
	ScreenEffects(Screen *screen);
	~ScreenEffects();
	void run(int16 effectNum, Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void flash(int count, byte *palette, int colorCount);
private:
	Screen *_screen;
	byte *_fxPalette;
	static const byte vfxOffsTable[64];
	static const byte vfxOffsIndexTable[8];
	const byte *vfxOffsTablePtr;
	int16 vfxX1, vfxY1, vfxWidth, vfxHeight;
	BlendedPaletteStatus _blendedPaletteStatus;

	void setPalette(byte *palette);
	void setBlendedPalette(byte *palette, byte *newPalette, int colorCount, int16 value, int16 maxValue);
	void startBlendedPalette(byte *palette, byte *newPalette, int colorCount, int16 maxValue);
	void stepBlendedPalette();
	void copyFxRect(Graphics::Surface *surface, int16 x1, int16 y1, int16 x2, int16 y2);

	void vfx00(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx01(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx02(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx03(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx04(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx05(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx06(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx07(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx08(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx09(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx10(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx11(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx12(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx13(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx14(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx15(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx16(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx17(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx18(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx19(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
	void vfx20(Graphics::Surface *surface, byte *palette, byte *newPalette, int colorCount);
};

} // End of namespace Made

#endif /* MADE_H */
