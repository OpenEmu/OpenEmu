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

#include "lure/lure.h"
#include "lure/palette.h"
#include "common/util.h"

namespace Lure {

// Constructor
// Defaults the palette to a full 256 entry palette

Palette::Palette() {
	_numEntries = GAME_COLORS;
	_palette = Memory::allocate(_numEntries * 4);
	_palette->empty();
}

// Consructor
// Sets up a palette with the given number of entries and a copy of the passed data

Palette::Palette(uint16 srcNumEntries, const byte *srcData, PaletteSource paletteSource) {
	_numEntries = srcNumEntries;
	_palette = Memory::allocate(_numEntries * 4);

	if (srcData) {
		if (paletteSource == RGB64)
			convertRgb64Palette(srcData, _numEntries);
		else if (paletteSource == EGA) {
			assert((srcNumEntries == 16) || (srcNumEntries == 17));
			convertEGAPalette(srcData);
		} else
			_palette->copyFrom(srcData, 0, 0, _numEntries * 4);

	} else {
		// No data provided, set a null palette
		_palette->empty();
	}
}

// Constructor
// Makes a copy of a passed palette object

Palette::Palette(Palette &src) {
	_numEntries = src.numEntries();
	_palette = Memory::duplicate(src._palette);
}

// Constructor
// Loads a palette from a resource

Palette::Palette(uint16 resourceId, PaletteSource paletteSource) {
	Disk &disk = Disk::getReference();
	bool isEGA = LureEngine::getReference().isEGA();
	MemoryBlock *srcData = disk.getEntry(resourceId);

	if (paletteSource == DEFAULT)
		paletteSource = isEGA ? EGA : RGB64;

	switch (paletteSource) {
	case EGA:
		// Handle EGA palette
		if ((srcData->size() != 16) && (srcData->size() != 17))
			error("Specified resource %d is not a palette", resourceId);

		_numEntries = 16;
		_palette = Memory::allocate(_numEntries * 4);
		convertEGAPalette(srcData->data());
		break;

	case RGB64:
		if (((srcData->size() % 3) != 0) || ((srcData->size() / 3) > GAME_COLORS))
			error("Specified resource %d is not a palette", resourceId);

		_numEntries = srcData->size() / 3;
		_palette = Memory::allocate(_numEntries * 4);
		convertRgb64Palette(srcData->data(), _numEntries);
		break;

	default:
		error("Invalid palette type specified for palette resource");
	}

	delete srcData;
}

// Destructor

Palette::~Palette() {
	delete _palette;
}

void Palette::convertRgb64Palette(const byte *srcPalette, uint16 srcNumEntries) {
	byte *pDest = _palette->data();
	const byte *pSrc = srcPalette;

	while (srcNumEntries-- > 0) {
		*pDest++ = (pSrc[0] << 2) + (pSrc[0] >> 4);
		*pDest++ = (pSrc[1] << 2) + (pSrc[1] >> 4);
		*pDest++ = (pSrc[2] << 2) + (pSrc[2] >> 4);
		*pDest++ = 0;
		pSrc += 3;
	}
}

// EGA palette definition copied from DOSBox 0.72
static byte ega_palette[64][3] =
{
  {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
  {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
  {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
  {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
  {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
  {0x00,0x00,0x00}, {0x00,0x00,0x2a}, {0x00,0x2a,0x00}, {0x00,0x2a,0x2a}, {0x2a,0x00,0x00}, {0x2a,0x00,0x2a}, {0x2a,0x15,0x00}, {0x2a,0x2a,0x2a},
  {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f},
  {0x15,0x15,0x15}, {0x15,0x15,0x3f}, {0x15,0x3f,0x15}, {0x15,0x3f,0x3f}, {0x3f,0x15,0x15}, {0x3f,0x15,0x3f}, {0x3f,0x3f,0x15}, {0x3f,0x3f,0x3f}
};

void Palette::convertEGAPalette(const byte *srcPalette) {
	byte *pDest = _palette->data();
	const byte *pSrc = srcPalette;

	for (int index = 0; index < 16; ++index, ++pSrc) {
		// Handle RGB components of entry
		assert(*pSrc < 64);
		byte *v = &ega_palette[*pSrc][0];
		*pDest++ = *v++ * 4;
		*pDest++ = *v++ * 4;
		*pDest++ = *v++ * 4;
		*pDest++ = 0;
	}
}

void Palette::setEntry(uint8 index, uint32 value) {
	if (index >= numEntries()) error("Invalid palette index: %d", index);
	uint32 *entry = (uint32 *) (data() + index * 4);
	*entry = value;
}

uint32 Palette::getEntry(uint8 index) {
	if (index >= numEntries()) error("Invalid palette index: %d", index);
	uint32 *entry = (uint32 *) (data() + index * 4);
	return *entry;
}

void Palette::copyFrom(Palette *src) {
	_palette->copyFrom(src->palette());
}

/*--------------------------------------------------------------------------*/

PaletteCollection::PaletteCollection(uint16 resourceId) {
	Disk &d = Disk::getReference();
	MemoryBlock *resource = d.getEntry(resourceId);
	bool isEGA = LureEngine::getReference().isEGA();
	uint32 palSize;
	uint8 *data = resource->data();

	if (isEGA) {
		// EGA Palette collection - only has 1 sub-palette
		if ((resource->size() != 16) && (resource->size() != 17))
			error("Resource #%d is not a valid palette set", resourceId);

		_numPalettes = 1;
		_palettes = (Palette **) Memory::alloc(1 * sizeof(Palette *));
		_palettes[0] = new Palette(16, data, EGA);

	} else {
		// VGA Palette collection
		if (resource->size() % (SUB_PALETTE_SIZE * 3) != 0)
			error("Resource #%d is not a valid palette set", resourceId);

		palSize = SUB_PALETTE_SIZE * 3;
		_numPalettes = resource->size() / palSize;

		_palettes = (Palette **) Memory::alloc(_numPalettes * sizeof(Palette *));
		for (uint8 paletteCtr = 0; paletteCtr < _numPalettes; ++paletteCtr, data += palSize)
			_palettes[paletteCtr] = new Palette(SUB_PALETTE_SIZE, data, RGB64);
	}

	delete resource;
}

PaletteCollection::~PaletteCollection() {
	for (int paletteCtr = 0; paletteCtr < _numPalettes; ++paletteCtr)
		delete _palettes[paletteCtr];
	free(_palettes);
}

Palette &PaletteCollection::getPalette(uint8 paletteNum) {
	if (paletteNum >= _numPalettes)
		error("Invalid palette index specified");
	return *_palettes[paletteNum];
}

} // End of namespace Lure
