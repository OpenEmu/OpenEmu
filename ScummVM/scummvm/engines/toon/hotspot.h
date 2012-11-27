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

#ifndef TOON_HOTSPOT_H
#define TOON_HOTSPOT_H

#include "toon/toon.h"
#include "toon/tools.h"

namespace Toon {

class HotspotData {
public:
	int16 getX1() const { return READ_LE_INT16(_data + 0); }
	int16 getY1() const { return READ_LE_INT16(_data + 1); }
	int16 getX2() const { return READ_LE_INT16(_data + 2); }
	int16 getY2() const { return READ_LE_INT16(_data + 3); }
	int16 getMode() const { return READ_LE_INT16(_data + 4); }
	int16 getRef() const { return READ_LE_INT16(_data + 5); }
	int16 getPriority() const { return READ_LE_INT16(_data + 7); }
	int16 getType() const { return READ_LE_INT16(_data + 8); }
	int16 getData(int32 id) const { return READ_LE_INT16(_data + id); }
	void setData(int32 id, int16 val) { WRITE_LE_UINT16(&_data[id], val); }

private:
	int16 _data[256];
};

class Hotspots {
public:
	Hotspots(ToonEngine *vm);
	~Hotspots();

	bool LoadRif(const Common::String &rifName, const Common::String &additionalRifName);
	int32 Find(int16 x, int16 y);
	int32 FindBasedOnCorner(int16 x, int16 y);
	HotspotData *Get(int32 id);
	int32 getCount() const { return _numItems; }

	void load(Common::ReadStream *Stream);
	void save(Common::WriteStream *Stream);

protected:
	HotspotData *_items;
	int32 _numItems;
	ToonEngine *_vm;
};

} // End of namespace Toon

#endif
