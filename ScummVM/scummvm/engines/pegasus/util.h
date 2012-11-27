/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995-1997 Presto Studios, Inc.
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

#ifndef PEGASUS_UTIL_H
#define PEGASUS_UTIL_H

#include "common/stream.h"

#include "pegasus/types.h"

namespace Common {
	class RandomSource;
}

namespace Pegasus {

class IDObject {
public:
	IDObject(const int32 id);
	~IDObject();

	int32 getObjectID() const;

private:
	int32 _objectID;
};

#define NUM_FLAGS (sizeof(Unit) * 8)
#define BIT_INDEX_SHIFT (sizeof(Unit) + 2 - (sizeof(Unit)) / 3)
#define BIT_INDEX_MASK (NUM_FLAGS - 1)

template <typename Unit, uint32 kNumFlags>
class FlagsArray {
public:
	FlagsArray() { clearAllFlags(); }
	void clearAllFlags() { memset(_flags, 0, sizeof(_flags)); }
	void setAllFlags() { memset(_flags, ~((Unit)0), sizeof(_flags)); }
	void setFlag(uint32 flag) { _flags[flag >> BIT_INDEX_SHIFT] |= 1 << (flag & BIT_INDEX_MASK); }
	void clearFlag(uint32 flag) { _flags[flag >> BIT_INDEX_SHIFT] &= ~(1 << (flag & BIT_INDEX_MASK)); }
	void setFlag(uint32 flag, bool val) { if (val) setFlag(flag); else clearFlag(flag); }
	bool getFlag(uint32 flag) { return (_flags[flag >> BIT_INDEX_SHIFT] & (1 << (flag & BIT_INDEX_MASK))) != 0; }
	bool anyFlagSet() {
		for (uint32 i = 0; i < sizeof(_flags); i++)
			if (_flags[i] != 0)
				return true;
		return false;
	}

	void readFromStream(Common::ReadStream *stream) {
		// Shortcut
		if (sizeof(Unit) == 1) {
			stream->read(_flags, sizeof(_flags));
			return;
		}

		for (uint32 i = 0; i < ARRAYSIZE(_flags); i++) {
			if (sizeof(Unit) == 2)
				_flags[i] = stream->readUint16BE();
			else /* if (sizeof(Unit) == 4) */
				_flags[i] = stream->readUint32BE();
		}
	}

	void writeToStream(Common::WriteStream *stream) {
		// Shortcut
		if (sizeof(Unit) == 1) {
			stream->write(_flags, sizeof(_flags));
			return;
		}

		for (uint32 i = 0; i < ARRAYSIZE(_flags); i++) {
			if (sizeof(Unit) == 2)
				stream->writeUint16BE(_flags[i]);
			else /* if (sizeof(Unit) == 4) */
				stream->writeUint32BE(_flags[i]);
		}
	}

private:
	Unit _flags[(kNumFlags - 1) / NUM_FLAGS + 1];
};

#undef NUM_FLAGS
#undef BIT_INDEX_SHIFT
#undef BIT_INDEX_MASK

int32 linearInterp(const int32 start1, const int32 stop1, const int32 current1, const int32 start2, const int32 stop2);

int32 pegasusRound(const int32 a, const int32 b);

uint32 tickCount();

} // End of namespace Pegasus

#endif
