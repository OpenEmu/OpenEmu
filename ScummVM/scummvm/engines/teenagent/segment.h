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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TEENAGENT_SEGMENT_H
#define TEENAGENT_SEGMENT_H

#include "common/stream.h"
#include "common/endian.h"

namespace TeenAgent {

class Segment {
	uint32 _size;
	byte *_data;

public:
	Segment() : _size(0), _data(0) {}
	~Segment();

	void read(Common::ReadStream *s, uint32 _size);

	inline byte get_byte(uint32 offset) const {
		assert(offset < _size);
		return _data[offset];
	}

	inline uint16 get_word(uint32 offset) const {
		assert(offset + 1 < _size);
		return READ_LE_UINT16(_data + offset);
	}

	inline void set_byte(uint32 offset, byte v) const {
		assert(offset < _size);
		_data[offset] = v;
	}

	inline void set_word(uint32 offset, uint16 v) const {
		assert(offset + 1 < _size);
		return WRITE_LE_UINT16(_data + offset, v);
	}

	const byte *ptr(uint32 addr) const {
		assert(addr < _size);
		return _data + addr;
	}

	byte *ptr(uint32 addr) {
		assert(addr < _size);
		return _data + addr;
	}

	uint size() const { return _size; }
};

} // End of namespace TeenAgent

#endif
