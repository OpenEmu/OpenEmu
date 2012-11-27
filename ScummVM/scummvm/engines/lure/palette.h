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

#ifndef LURE_PALETTE_H
#define LURE_PALETTE_H

#include "lure/luredefs.h"
#include "lure/disk.h"
#include "lure/memory.h"

namespace Lure {

enum PaletteSource {DEFAULT, RGB, RGB64, EGA};

class Palette {
private:
	MemoryBlock *_palette;
	uint16 _numEntries;

	void convertRgb64Palette(const byte *srcPalette, uint16 srcNumEntries);
	void convertEGAPalette(const byte *srcPalette);
public:
	Palette();
	Palette(uint16 srcNumEntries, const byte *srcData, PaletteSource paletteSource);
	Palette(Palette &src);
	Palette(uint16 resourceId, PaletteSource paletteSource = DEFAULT);
	~Palette();

	uint8 *data() { return _palette->data(); }
	MemoryBlock *palette() { return _palette; }
	uint16 numEntries() { return _palette->size() / 4; }
	void setEntry(uint8 index, uint32 value);
	uint32 getEntry(uint8 index);
	void copyFrom(Palette *src);
};

class PaletteCollection {
private:
	Palette **_palettes;
	uint8 _numPalettes;
public:
	PaletteCollection(uint16 resourceId);
	~PaletteCollection();

	uint8 numPalettes() { return _numPalettes; }
	Palette &getPalette(uint8 paletteNum);
};

} // end of namspace Lure

#endif
