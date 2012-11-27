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

#ifndef PEGASUS_NEIGHBORHOOD_VIEW_H
#define PEGASUS_NEIGHBORHOOD_VIEW_H

#include "common/array.h"
#include "common/endian.h"

#include "pegasus/constants.h"

namespace Common {
	class SeekableReadStream;
}

namespace Pegasus {

class ViewTable {
public:
	ViewTable() {}
	~ViewTable() {}

	static uint32 getResTag() { return MKTAG('V', 'i', 'e', 'w'); }

	void loadFromStream(Common::SeekableReadStream *stream);
	void clear();

	struct Entry {
		Entry() { time = 0xffffffff; }
		bool isEmpty() { return time == 0xffffffff; }

		RoomID	room;
		DirectionConstant direction;
		AlternateID altCode;
		TimeValue time;
	};

	Entry findEntry(RoomID room, DirectionConstant direction, AlternateID altCode);

private:
	Common::Array<Entry> _entries;
};

} // End of namespace Pegasus

#endif
