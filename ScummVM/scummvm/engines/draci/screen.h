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

#ifndef DRACI_SCREEN_H
#define DRACI_SCREEN_H

#include "common/scummsys.h"

namespace Draci {

enum ScreenParameters {
	kScreenWidth = 320,
	kScreenHeight = 200,
	kNumColors = 256,
	kDefaultTransparent = 255
};

class DraciEngine;
class Surface;

class Screen {

public:
	Screen(DraciEngine *vm);
	~Screen();

	void setPalette(const byte *data, uint16 start, uint16 num);
	void interpolatePalettes(const byte *first, const byte *second, uint16 start, uint16 num, int index, int number);
	const byte *getPalette() const { return _palette; }
	void copyToScreen();
	void clearScreen();
	Surface *getSurface() { return _surface; }

private:
	int interpolate(int first, int second, int index, int number);

	Surface *_surface;
	byte *_palette;
	byte *_blackPalette;
	DraciEngine *_vm;
};

} // End of namespace Draci

#endif // DRACI_SCREEN_H
