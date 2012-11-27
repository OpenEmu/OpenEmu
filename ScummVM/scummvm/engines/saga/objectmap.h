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

// Object map / Object click-area module header file

#ifndef SAGA_OBJECTMAP_H
#define SAGA_OBJECTMAP_H

namespace Saga {


class HitZone {
private:
	typedef Common::Array<Point> ClickArea;
	typedef Common::Array<ClickArea> ClickAreas;
public:
	void load(SagaEngine *vm, Common::MemoryReadStreamEndian *readStream, int index, int sceneNumber);

	int getIndex() const {
		return _index;
	}
	int getNameIndex() const {
		return _nameIndex;
	}
	int getSceneNumber() const {
		return _nameIndex;
	}
	int getActorsEntrance() const {
		return _scriptNumber;
	}
	int getScriptNumber() const {
		return _scriptNumber;
	}
	int getRightButtonVerb() const {
		return _rightButtonVerb;
	}
	int getFlags() const {
		return _flags;
	}
	void setFlag(HitZoneFlags flag) {
		_flags |= flag;
	}
	void clearFlag(HitZoneFlags flag) {
		_flags &= ~flag;
	}
	int getDirection() const {
		return ((_flags >> 4) & 0xF);
	}
	uint16 getHitZoneId() const {
		return objectIndexToId(kGameObjectHitZone, _index);
	}
	uint16 getStepZoneId() const {
		return objectIndexToId(kGameObjectStepZone, _index);
	}
	bool getSpecialPoint(Point &specialPoint) const;
#ifdef SAGA_DEBUG
	void draw(SagaEngine *vm, int color);	// for debugging
#endif
	bool hitTest(const Point &testPoint);

private:
	int _flags;				// Saga::HitZoneFlags
	int _rightButtonVerb;
	int _nameIndex;
	int _scriptNumber;
	int _index;

	ClickAreas _clickAreas;
};

typedef Common::Array<HitZone> HitZoneArray;

class ObjectMap {
public:
	ObjectMap(SagaEngine *vm) : _vm(vm) {
	}
	void load(const ByteArray &resourceData);
	void clear();
#ifdef SAGA_DEBUG
	void draw(const Point& testPoint, int color, int color2);	// for debugging
#endif
	int hitTest(const Point& testPoint);
	HitZone *getHitZone(int16 index) {
		if (uint(index) >= _hitZoneList.size()) {
			return NULL;
		}
		return &_hitZoneList[index];
	}

	void cmdInfo();

private:
	SagaEngine *_vm;

	HitZoneArray _hitZoneList;
};

} // End of namespace Saga

#endif
