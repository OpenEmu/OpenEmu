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

#ifndef DREAMWEB_STRUCTS_H
#define DREAMWEB_STRUCTS_H

#include "common/endian.h"
#include "common/rect.h"

namespace DreamWeb {

struct GraphicsFile;
struct SetObject;

struct Sprite {
	bool _mainManCallback;
	const GraphicsFile *_frameData;
	uint8  x;
	uint8  y;
	uint8  frameNumber;
	uint8  delay;
	uint8  animFrame; // index into SetObject::frames
	SetObject *_objData;
	uint8  speed;
	uint8  priority;
	uint8  walkFrame;
	uint8  type;
	uint8  hidden;
};

struct RectWithCallback {
	uint16 _xMin, _xMax;
	uint16 _yMin, _yMax;
	void (DreamWebEngine::*_callback)();

	bool contains(uint16 x, uint16 y) const {
		return (x >= _xMin) && (x < _xMax) && (y >= _yMin) && (y < _yMax);
	}
};



#include "common/pack-start.h"	// START STRUCT PACKING

struct SetObject {
	uint8 b0;
	uint8 b1;
	uint8 b2;
	uint8 slotSize;
	uint8 slotCount;
	uint8 priority;
	uint8 b6;
	uint8 delay;
	uint8 type;
	uint8 b9;
	uint8 b10;
	uint8 b11;
	uint8 objId[4];
	uint8 b16;
	uint8 index;
	uint8 frames[13]; // Table mapping animFrame to sprite frame number
	                  // NB: Don't know the size yet
	uint8 b31;
	uint8 b32;
	uint8 b33;
	uint8 b34;
	uint8 b35;
	uint8 b36;
	uint8 b37;
	uint8 b38;
	uint8 b39;
	uint8 b40;
	uint8 b41;
	uint8 b42;
	uint8 b43;
	uint8 b44;
	uint8 b45;
	uint8 b46;
	uint8 b47;
	uint8 b48;
	uint8 b49;
	uint8 b50;
	uint8 b51;
	uint8 b52;
	uint8 b53;
	uint8 b54;
	uint8 b55;
	uint8 b56;
	uint8 b57;
	uint8 mapad[5];
	uint8 b63;
} PACKED_STRUCT;

struct DynObject {
	uint8 currentLocation;
	uint8 index;
	uint8 mapad[5];
	uint8 slotSize;		// the size of an object's slots
	uint8 slotCount;	// the number of slots of an object
	uint8 objectSize;	// the size of an object
	uint8 turnedOn;
	uint8 initialLocation;
	uint8 objId[4];
} PACKED_STRUCT;

struct ObjPos {
	uint8 xMin;
	uint8 yMin;
	uint8 xMax;
	uint8 yMax;
	uint8 index;
	bool contains(uint8 x, uint8 y) const {
		return (x >= xMin) && (x < xMax) && (y >= yMin) && (y < yMax);
	}
} PACKED_STRUCT;

struct Frame {
	uint8  width;
	uint8  height;
	uint16 _ptr;
	uint16 ptr() const { return READ_LE_UINT16(&_ptr); }
	void setPtr(uint16 v) { WRITE_LE_UINT16(&_ptr, v); }
	uint8  x;
	uint8  y;
} PACKED_STRUCT;

struct Reel {
	uint8 frame_lo;
	uint8 frame_hi;
	uint16 frame() const { return READ_LE_UINT16(&frame_lo); }
	void setFrame(uint16 v) { WRITE_LE_UINT16(&frame_lo, v); }
	uint8 x;
	uint8 y;
	uint8 b4;
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING



struct ReelRoutine {
	uint8 reallocation;
	uint8 mapX;
	uint8 mapY;
	uint16 _reelPointer;
	uint16 reelPointer() const { return _reelPointer; }
	void setReelPointer(uint16 v) { _reelPointer = v; }
	void incReelPointer() { _reelPointer++; }
	uint8 period;
	uint8 counter;
	uint8 b7;
};

struct People {
	uint16 _reelPointer;
	ReelRoutine *_routinePointer;
	uint8 b4;
};



#include "common/pack-start.h"	// START STRUCT PACKING

struct Room {
	char  name[13];
	uint8 roomsSample;
	uint8 b14;
	uint8 mapX;
	uint8 mapY;
	uint8 b17;
	uint8 b18;
	uint8 b19;
	uint8 liftFlag;
	uint8 b21;
	uint8 facing;
	uint8 countToOpen;
	uint8 liftPath;
	uint8 doorPath;
	uint8 b26;
	uint8 b27;
	uint8 b28;
	uint8 b29;
	uint8 b30;
	uint8 realLocation;
} PACKED_STRUCT;

extern const Room g_roomData[];

struct Rain {
	uint8 x;
	uint8 y;
	uint8 size;
	uint16 w3;
	uint8 b5;
} PACKED_STRUCT;

struct Change {
	uint8 index;
	uint8 location;
	uint8 value;
	uint8 type;
} PACKED_STRUCT;

struct PathNode {
	uint8 x;
	uint8 y;
	uint8 x1;
	uint8 y1;
	uint8 x2;
	uint8 y2;
	uint8 on;
	uint8 dir;
} PACKED_STRUCT;

struct PathSegment {
	uint8 b0;
	uint8 b1;
} PACKED_STRUCT;

struct RoomPaths {
	PathNode    nodes[12];
	PathSegment segments[24];
} PACKED_STRUCT;

struct FileHeader {
	char _desc[50];
	uint16 _len[20];
	uint8 _padding[6];

	uint16 len(unsigned int i) const {
		assert(i < 20);
		return READ_LE_UINT16(&_len[i]);
	}
	void setLen(unsigned int i, uint16 length) {
		assert(i < 20);
		WRITE_LE_UINT16(&_len[i], length);
	}
} PACKED_STRUCT;

struct Atmosphere {
	uint8 _location;
	uint8 _mapX;
	uint8 _mapY;
	uint8 _sound;
	uint8 _repeat;
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING



enum ObjectTypes {
	kSetObjectType1 = 1,
	kFreeObjectType = 2,
	kSetObjectType3 = 3,
	kExObjectType = 4
};

struct ObjectRef {
	uint8 _index;
	uint8 _type; // enum ObjectTypes

	bool operator==(const ObjectRef &r) const {
		return _index == r._index && _type == r._type;
	}
	bool operator!=(const ObjectRef &r) const {
		return _index != r._index || _type != r._type;
	}
};



#include "common/pack-start.h"	// START STRUCT PACKING

struct BackdropMapFlag {
	uint8 _flag;
	uint8 _flagEx;
} PACKED_STRUCT;

struct MapFlag {
	uint8 _flag;
	uint8 _flagEx;
	uint8 _type;
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING





struct TextFile {
	TextFile(unsigned int size = 66) : _size(size), _text(0) { _offsetsLE = new uint16[_size]; }

	~TextFile() {
		delete[] _offsetsLE;
		_offsetsLE = 0;
		_size = 0;
		clear();
	}

	uint16 *_offsetsLE;
	unsigned int _size;
	char *_text;

	const char *getString(unsigned int i) const {
		assert(i < _size);
		return _text + getOffset(i);
	}
	void setOffset(unsigned int i, uint16 offset) {
		WRITE_LE_UINT16(&_offsetsLE[i], offset);
	}
	uint16 getOffset(unsigned int i) const {
		return READ_LE_UINT16(&_offsetsLE[i]);
	}
	void clear() {
		delete[] _text;
		_text = 0;
	}
};

struct GraphicsFile {
	GraphicsFile() : _data(0), _frames(0) { }

	Frame *_frames;
	uint8 *_data;

	const uint8 *getFrameData(unsigned int i) const {
		// There is 2080 bytes of Frame data, but that is between 346 and 347
		// frames
		assert(i < 346);
		return _data + _frames[i].ptr();
	}
	void clear() {
		delete[] _frames;
		_frames = 0;
		delete[] _data;
		_data = 0;
	}
};

struct GameVars {
	uint8 _startVars;
	uint8 _progressPoints;
	uint8 _watchOn;
	uint8 _shadesOn;
	uint8 _secondCount;
	uint8 _minuteCount;
	uint8 _hourCount;
	uint8 _zoomOn;
	uint8 _location;
	uint8 _exPos;
	uint16 _exFramePos;
	uint16 _exTextPos;
	uint16 _card1Money;
	uint16 _listPos;
	uint8 _ryanPage;
	uint16 _watchingTime;
	uint16 _reelToWatch; // reel plays from here in mode 0
	uint16 _endWatchReel; // and stops here. Mode set to 1
	uint8 _speedCount;
	uint8 _watchSpeed;
	uint16 _reelToHold; // if mode is 1 hold on this reel
	uint16 _endOfHoldReel; // if mode is 2 then play to endOfHoldReel and reset mode to -1
	uint8 _watchMode;
	uint8 _destAfterHold; // set walking destination
	uint8 _newsItem;
	uint8 _liftFlag;
	uint8 _liftPath;
	uint8 _lockStatus;
	uint8 _doorPath;
	uint8 _countToOpen;
	uint8 _countToClose;
	uint8 _rockstarDead;
	uint8 _generalDead;
	uint8 _sartainDead;
	uint8 _aideDead;
	uint8 _beenMugged;
	uint8 _gunPassFlag;
	uint8 _canMoveAltar;
	uint8 _talkedToAttendant;
	uint8 _talkedToSparky;
	uint8 _talkedToBoss;
	uint8 _talkedToRecep;
	uint8 _cardPassFlag;
	uint8 _madmanFlag;
	uint8 _keeperFlag;
	uint8 _lastTrigger;
	uint8 _manDead;
	uint8 _seed1;
	uint8 _seed2;
	uint8 _seed3;
	uint8 _needToTravel;
	uint8 _throughDoor;
	uint8 _newObs;
	uint8 _ryanOn;
	uint8 _combatCount;
	uint8 _lastWeapon;
	uint8 _dreamNumber;
	uint8 _roomAfterDream;
	uint8 _shakeCounter;
};

struct TimedTemp {
	TimedTemp() : _timeCount(0), _string(0) { }

	uint8 _x;
	uint8 _y;

	uint16 _timeCount;
	uint16 _countToTimed;

	const char *_string;
};

} // End of namespace DreamWeb

#endif

