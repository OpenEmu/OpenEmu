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
 *
 */

#include "graphics/palette.h"

#include "toltecs/toltecs.h"
#include "toltecs/palette.h"
#include "toltecs/resource.h"

namespace Toltecs {

Palette::Palette(ToltecsEngine *vm) : _vm(vm) {
	clearFragments();

	memset(_colorTransTable, 0, sizeof(_colorTransTable));
}

Palette::~Palette() {
}

void Palette::setFullPalette(byte *palette) {
	byte colors[768];
	for (int i = 0; i < 256; i++) {
		colors[i * 3 + 0] = palette[i * 3 + 0] << 2;
		colors[i * 3 + 1] = palette[i * 3 + 1] << 2;
		colors[i * 3 + 2] = palette[i * 3 + 2] << 2;
	}
	_vm->_system->getPaletteManager()->setPalette((const byte *)colors, 0, 256);
	_vm->_system->updateScreen();
}

void Palette::getFullPalette(byte *palette) {
	byte colors[768];
	_vm->_system->getPaletteManager()->grabPalette(colors, 0, 256);
	for (int i = 0; i < 256; i++) {
		palette[i * 3 + 0] = colors[i * 3 + 0] >> 2;
		palette[i * 3 + 1] = colors[i * 3 + 1] >> 2;
		palette[i * 3 + 2] = colors[i * 3 + 2] >> 2;
	}
}

void Palette::setDeltaPalette(byte *palette, byte mask, int8 deltaValue, int16 count, int16 startIndex) {
	byte colors[768];

	byte *palPtr = palette + startIndex * 3;
	int16 index = startIndex, colorCount = count;
	byte rgb;

	count++;

	_vm->_system->getPaletteManager()->grabPalette(colors, 0, 256);

	deltaValue *= -1;

	while (count--) {
		rgb = *palPtr++;
		if (mask & 1) colors[index * 3 + 0] = CLIP<int>(rgb + deltaValue, 0, 63) << 2;
		rgb = *palPtr++;
		if (mask & 2) colors[index * 3 + 1] = CLIP<int>(rgb + deltaValue, 0, 63) << 2;
		rgb = *palPtr++;
		if (mask & 4) colors[index * 3 + 2] = CLIP<int>(rgb + deltaValue, 0, 63) << 2;
		index++;
	}

	debug(0, "startIndex = %d; colorCount = %d", startIndex, colorCount);

	_vm->_system->getPaletteManager()->setPalette((const byte *)colors, 0, 256);
}

void Palette::loadAddPalette(uint resIndex, byte startIndex) {
	Resource *paletteResource = _vm->_res->load(resIndex);
	memcpy(&_mainPalette[startIndex * 3], paletteResource->data, paletteResource->size);
}

void Palette::loadAddPaletteFrom(byte *source, byte startIndex, byte count) {
	memcpy(&_mainPalette[startIndex * 3], source, count * 3);
}

void Palette::addFragment(uint resIndex, int16 id) {
	debug(0, "Palette::addFragment(%d, %d)", resIndex, id);

	Resource *fragmentResource = _vm->_res->load(resIndex);
	byte count = fragmentResource->size / 3;

	memcpy(&_mainPalette[_fragmentIndex * 3], fragmentResource->data, count * 3);

	PaletteFragment fragment;
	fragment.id = id;
	fragment.index = _fragmentIndex;
	fragment.count = count;
	_fragments.push_back(fragment);

	debug(0, "Palette::addFragment() index = %02X; count = %02X", fragment.index, fragment.count);

	_fragmentIndex += count;
}

uint16 Palette::findFragment(int16 id) {
	debug(0, "Palette::findFragment(%d)", id);

	uint16 result = 0;
	for (PaletteFragmentArray::iterator iter = _fragments.begin(); iter != _fragments.end(); iter++) {
		PaletteFragment fragment = *iter;
		if (fragment.id == id) {
			result = (fragment.count << 8) | fragment.index;
			break;
		}
	}

	debug(0, "Palette::findFragment() result = %04X", result);

	return result;
}

void Palette::clearFragments() {
	debug(0, "Palette::clearFragments()");
	_fragmentIndex = 128;
	_fragments.clear();
}

void Palette::buildColorTransTable(byte limit, int8 deltaValue, byte mask) {
	byte r = 0, g = 0, b = 0;

	mask &= 7;

	for (int i = 0; i < 256; i++) {

		if (deltaValue < 0) {
			// TODO (probably unused)
			warning("Palette::buildColorTransTable(%d, %d, %02X) not yet implemented!", limit, deltaValue, mask);
		} else {
			r = _mainPalette[i * 3 + 0];
			g = _mainPalette[i * 3 + 1];
			b = _mainPalette[i * 3 + 2];
			if (MAX(r, MAX(b, g)) >= limit) {
				if ((mask & 1) && r >= deltaValue)
					r -= deltaValue;
				if ((mask & 2) && g >= deltaValue)
					g -= deltaValue;
				if ((mask & 4) && b >= deltaValue)
					b -= deltaValue;
			}
		}

		int bestIndex = 0;
		uint16 bestMatch = 0xFFFF;

		for (int j = 0; j < 256; j++) {
			byte distance = ABS(_mainPalette[j * 3 + 0] - r) + ABS(_mainPalette[j * 3 + 1] - g) + ABS(_mainPalette[j * 3 + 2] - b);
			byte maxColor = MAX(_mainPalette[j * 3 + 0], MAX(_mainPalette[j * 3 + 1], _mainPalette[j * 3 + 2]));
			uint16 match = (distance << 8) | maxColor;
			if (match < bestMatch) {
				bestMatch = match;
				bestIndex = j;
			}
		}

		_colorTransTable[i] = bestIndex;

	}
}

void Palette::buildColorTransTable2(byte limit, int8 deltaValue, byte mask) {
	// TODO
}

void Palette::saveState(Common::WriteStream *out) {
	// Save currently active palette
	byte palette[768];
	getFullPalette(palette);
	out->write(palette, 768);

	out->write(_mainPalette, 768);
	out->write(_animPalette, 768);
	out->write(_colorTransTable, 256);

	uint16 fragmentCount = _fragments.size();
	out->writeUint16LE(fragmentCount);
	for (PaletteFragmentArray::iterator iter = _fragments.begin(); iter != _fragments.end(); iter++) {
		PaletteFragment fragment = *iter;
		out->writeUint16LE(fragment.id);
		out->writeByte(fragment.index);
		out->writeByte(fragment.count);
	}
	out->writeByte(_fragmentIndex);
}

void Palette::loadState(Common::ReadStream *in) {
	// Save currently active palette
	byte palette[768];
	in->read(palette, 768);
	setFullPalette(palette);

	in->read(_mainPalette, 768);
	in->read(_animPalette, 768);
	in->read(_colorTransTable, 256);

	uint16 fragmentCount = in->readUint16LE();
	_fragments.clear();
	for (uint16 i = 0; i < fragmentCount; i++) {
		PaletteFragment fragment;
		fragment.id = in->readUint16LE();
		fragment.index = in->readByte();
		fragment.count = in->readByte();
		_fragments.push_back(fragment);
	}
	_fragmentIndex = in->readByte();
}


} // End of namespace Toltecs
