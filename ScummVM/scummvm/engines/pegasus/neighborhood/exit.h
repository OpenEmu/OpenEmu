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

#ifndef PEGASUS_NEIGHBORHOOD_EXIT_H
#define PEGASUS_NEIGHBORHOOD_EXIT_H

#include "common/array.h"
#include "common/endian.h"

#include "pegasus/constants.h"

namespace Common {
	class SeekableReadStream;
}

namespace Pegasus {

class ExitTable {
public:
	ExitTable() {}
	~ExitTable() {}

	static uint32 getResTag() { return MKTAG('E', 'x', 'i', 't'); }

	void loadFromStream(Common::SeekableReadStream *stream);
	void clear();

	struct Entry {
		Entry() { clear(); }
		bool isEmpty() { return movieStart == 0xffffffff; }
		void clear() {
			room = kNoRoomID;
			direction = kNoDirection;
			altCode = kNoAlternateID;
			movieStart = 0xffffffff;
			movieEnd = 0xffffffff;
			exitEnd = 0xffffffff;
			originalEnd = 0xffffffff;
			exitLoop = 0xffffffff;
			exitRoom = kNoRoomID;
			exitDirection = kNoDirection;
		}

		RoomID room;
		DirectionConstant direction;
		AlternateID altCode;
		TimeValue movieStart;
		TimeValue movieEnd;
		// exitEnd is the end of the optimized run of walks.
		TimeValue exitEnd;
		TimeValue originalEnd;
		// exitLoop is the loop start time of the optimized run of walks if the run
		// loops back on itself (so far, only in TSA).
		TimeValue exitLoop;
		RoomID exitRoom;
		DirectionConstant exitDirection;
	};

	Entry findEntry(RoomID room, DirectionConstant direction, AlternateID altCode);

	typedef Common::Array<Entry>::iterator iterator;
	iterator begin() { return _entries.begin(); }
	iterator end() { return _entries.end(); }

private:
	Common::Array<Entry> _entries;
};

} // End of namespace Pegasus

#endif
