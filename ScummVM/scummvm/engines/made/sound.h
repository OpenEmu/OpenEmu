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

#ifndef MADE_SOUND_H
#define MADE_SOUND_H

#include "common/array.h"

namespace Made {

class ManholeEgaSoundDecompressor {
public:
	void decompress(byte *source, byte *dest, uint32 size);
protected:
	byte *_source, *_dest;
	uint32 _size;
	uint16 _bitBuffer;
	int _bitsLeft;
	int32 _sample1, _sample2, _sample3, _sample4;
	bool _writeFlag;
	bool _eof;
	int _mode;
	int getBit();
	void update0();
	void update1();
	void update2();
	void update3();
};

struct SoundEnergyItem {
	uint32 position;
	byte energy;
};

typedef Common::Array<SoundEnergyItem> SoundEnergyArray;

void decompressSound(byte *source, byte *dest, uint16 chunkSize, uint16 chunkCount, SoundEnergyArray *soundEnergyArray = NULL);

} // End of namespace Made

#endif /* MADE_H */
