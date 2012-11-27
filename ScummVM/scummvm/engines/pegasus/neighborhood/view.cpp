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

#include "common/debug.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "pegasus/neighborhood/view.h"

namespace Pegasus {

void ViewTable::loadFromStream(Common::SeekableReadStream *stream) {
	uint32 count = stream->readUint32BE();
	_entries.resize(count);

	for (uint32 i = 0; i < count; i++) {
		_entries[i].room = stream->readUint16BE();
		_entries[i].direction = stream->readByte();
		_entries[i].altCode = stream->readByte();
		_entries[i].time = stream->readUint32BE();
		debug(0, "View[%d]: %d %d %d %d", i, _entries[i].room, _entries[i].direction,
				_entries[i].altCode, _entries[i].time);
	}
}

void ViewTable::clear() {
	_entries.clear();
}

ViewTable::Entry ViewTable::findEntry(RoomID room, DirectionConstant direction, AlternateID altCode) {
	for (uint32 i = 0; i < _entries.size(); i++)
		if (_entries[i].room == room && _entries[i].direction == direction && _entries[i].altCode == altCode)
			return _entries[i];

	return Entry();
}

} // End of namespace pegasus
