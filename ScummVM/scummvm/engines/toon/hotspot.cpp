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

#include "common/debug.h"

#include "toon/hotspot.h"
#include "toon/tools.h"

namespace Toon {

Hotspots::Hotspots(ToonEngine *vm) : _vm(vm) {
	_items = NULL;
	_numItems = 0;
}

Hotspots::~Hotspots() {
	delete[] _items;
}

void Hotspots::load(Common::ReadStream *Stream) {
	delete[] _items;

	_numItems = Stream->readSint16BE();
	_items = new HotspotData[_numItems];

	for (int32 i = 0; i < _numItems; i++) {
		for (int32 a = 0; a < 256; a++)
			_items[i].setData(a, Stream->readSint16BE());
	}
}

void Hotspots::save(Common::WriteStream *Stream) {
	Stream->writeSint16BE(_numItems);

	for (int32 i = 0; i < _numItems; i++) {
		for (int32 a = 0; a < 256; a++)
			Stream->writeSint16BE(_items[i].getData(a));
	}
}

int32 Hotspots::FindBasedOnCorner(int16 x, int16 y) {
	debugC(1, kDebugHotspot, "FindBasedOnCorner(%d, %d)", x, y);

	for (int32 i = 0; i < _numItems; i++) {
		if (x == _items[i].getX1()) {
			if (y == _items[i].getY1()) {
				if (_items[i].getMode() == -1)
					return _items[i].getRef();

				return i;
			}
		}
	}
	return -1;
}

int32 Hotspots::Find(int16 x, int16 y) {
	debugC(6, kDebugHotspot, "Find(%d, %d)", x, y);

	int32 priority = -1;
	int32 foundId = -1;
	int32 testId = -1;

	for (int i = 0; i < _numItems; i++) {
		if (x >= _items[i].getX1() && x <= _items[i].getX2() && y >= _items[i].getY1() && y <= _items[i].getY2()) {
			if (_items[i].getMode() == -1)
				testId = _items[i].getRef();
			else
				testId = i;

			if (_items[testId].getPriority() > priority) {
				foundId = testId;
				priority = _items[testId].getPriority();
			}
		}
	}
	return foundId;
}

bool Hotspots::LoadRif(const Common::String &rifName, const Common::String &additionalRifName) {
	debugC(1, kDebugHotspot, "LoadRif(%s, %s)", rifName.c_str(), additionalRifName.c_str());

	uint32 size = 0;
	uint8 *rifData = _vm->resources()->getFileData(rifName, &size);
	if (!rifData)
		return false;

	uint32 size2 = 0;
	uint8 *rifData2 = 0;
	if (additionalRifName.size())
		rifData2 = _vm->resources()->getFileData(additionalRifName, &size2);

	// figure out the number of hotspots based on file size
	int32 rifsize = READ_BE_UINT32(&rifData[4]);
	int32 rifsize2 = 0;

	if (size2)
		rifsize2 = READ_BE_UINT32(&rifData2[4]);

	_numItems = (rifsize + rifsize2) / 512;

	delete[] _items;
	_items = new HotspotData[_numItems];

	// RIFs are compressed in RNC1
	RncDecoder decoder;
	decoder.unpackM1(rifData, size, _items);
	if (rifsize2) {
		RncDecoder decoder2;
		decoder2.unpackM1(rifData2 , size2, _items + (rifsize >> 9));
		for (int32 i = 0; i < (rifsize2 >> 9); i++) {
			HotspotData *hot = _items + (rifsize >> 9) + i;
			hot->setData(0, hot->getX1() + 1280);
			hot->setData(2, hot->getX2() + 1280);
			if (hot->getMode() == -1)
				hot->setData(5, hot->getRef() + (rifsize >> 9));
		}
	}

	return true;
}

HotspotData *Hotspots::Get(int32 id) {
	debugC(5, kDebugHotspot, "Get(%d)", id);

	if (id < 0 || id >= _numItems)
		return 0;
	else
		return &_items[id];
}

} // End of namespace Toon
