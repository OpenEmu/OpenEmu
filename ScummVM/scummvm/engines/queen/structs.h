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

#ifndef QUEEN_STRUCTS_H
#define QUEEN_STRUCTS_H

#include "queen/defs.h"
#include "common/endian.h"

namespace Queen {

struct Box {
	int16 x1, y1, x2, y2;

	Box()
		: x1(0), y1(0), x2(0), y2(0) {
	}

	Box(int16 xx1, int16 yy1, int16 xx2, int16 yy2)
		: x1(xx1), y1(yy1), x2(xx2), y2(yy2) {
	}

	void readFromBE(byte *&ptr) {
		x1 = (int16)READ_BE_UINT16(ptr); ptr += 2;
		y1 = (int16)READ_BE_UINT16(ptr); ptr += 2;
		x2 = (int16)READ_BE_UINT16(ptr); ptr += 2;
		y2 = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}

	void writeToBE(byte *&ptr) {
		WRITE_BE_UINT16(ptr, x1); ptr += 2;
		WRITE_BE_UINT16(ptr, y1); ptr += 2;
		WRITE_BE_UINT16(ptr, x2); ptr += 2;
		WRITE_BE_UINT16(ptr, y2); ptr += 2;
	}

	int16 xDiff() const {
		return x2 - x1;
	}

	int16 yDiff() const {
		return y2 - y1;
	}

	bool intersects(int16 x, int16 y, uint16 w, uint16 h) const {
		return (x + w > x1) && (y + h > y1) && (x <= x2) && (y <= y2);
	}

	bool contains(int16 x, int16 y) const {
		return (x >= x1) && (x <= x2) && (y >= y1) && (y <= y2);
	}

	bool operator==(const Box &b) const {
		return (x1 == b.x1) && (x2 == b.x2) && (y1 == b.y1) && (y2 == b.y2);
	}
};


struct Area {
	//! bitmask of connected areas
	int16 mapNeighbors;
	//! coordinates defining area limits
	Box box;
	//! scaling factors for bobs actors
	uint16 bottomScaleFactor, topScaleFactor;
	//! entry in ObjectData, object lying in this area
	uint16 object;

	void readFromBE(byte *&ptr) {
		mapNeighbors = (int16)READ_BE_UINT16(ptr); ptr += 2;
		box.readFromBE(ptr);
		bottomScaleFactor = READ_BE_UINT16(ptr); ptr += 2;
		topScaleFactor = READ_BE_UINT16(ptr); ptr += 2;
		object = READ_BE_UINT16(ptr); ptr += 2;
	}

	void writeToBE(byte *&ptr) {
		WRITE_BE_UINT16(ptr, mapNeighbors); ptr += 2;
		box.writeToBE(ptr);
		WRITE_BE_UINT16(ptr, bottomScaleFactor); ptr += 2;
		WRITE_BE_UINT16(ptr, topScaleFactor); ptr += 2;
		WRITE_BE_UINT16(ptr, object); ptr += 2;
	}

	uint16 calcScale(int16 y) const {
		uint16 dy = box.yDiff();
		int16 ds = scaleDiff();
		uint16 scale = 0;

		if (dy)	// Prevent division-by-zero
			scale = ((((y - box.y1) * 100) / dy) * ds) / 100 + bottomScaleFactor;

		if (scale == 0)
			scale = 100;

		return scale;
	}

	int16 scaleDiff() const {
		return (int16)(topScaleFactor - bottomScaleFactor);
	}
};


struct WalkOffData {
	//! entry in ObjectData
	int16 entryObj;
	//! coordinates to reach
	uint16 x, y;

	void readFromBE(byte *&ptr) {
		entryObj = (int16)READ_BE_UINT16(ptr); ptr += 2;
		x = READ_BE_UINT16(ptr); ptr += 2;
		y = READ_BE_UINT16(ptr); ptr += 2;
	}

	void writeToBE(byte *&ptr) {
		WRITE_BE_UINT16(ptr, entryObj); ptr += 2;
		WRITE_BE_UINT16(ptr, x); ptr += 2;
		WRITE_BE_UINT16(ptr, y); ptr += 2;
	}
};


struct GraphicData {
	//! coordinates of object
	uint16 x, y;
	//! bank bobframes
	/*!
		<table>
			<tr>
				<td>lastFrame == 0</td>
				<td>non-animated bob (one frame)</td>
			</tr>
			<tr>
				<td>lastFrame < 0</td>
				<td>rebound animation</td>
			</tr>
			<tr>
				<td>firstFrame < 0</td>
				<td>BobSlot::animString (animation is described by a string)</td>
			</tr>
			<tr>
				<td>firstFrame > 0</td>
				<td>BobSlot::animNormal (animation is a sequence of frames)</td>
			</tr>
		</table>
	*/
	int16 firstFrame, lastFrame;
	//! moving speed of object
	uint16 speed;

	void readFromBE(byte *&ptr) {
		x = READ_BE_UINT16(ptr); ptr += 2;
		y = READ_BE_UINT16(ptr); ptr += 2;
		firstFrame = (int16)READ_BE_UINT16(ptr); ptr += 2;
		lastFrame = (int16)READ_BE_UINT16(ptr); ptr += 2;
		speed = READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct ObjectData {
	//! entry in OBJECT_NAME (<0: object is hidden, 0: object has been deleted)
	int16 name;
	//! coordinates of object
	uint16 x, y;
	//! entry in OBJECT_DESCR
	uint16 description;
	//! associated object
	int16 entryObj;
	//! room in which this object is available
	uint16 room;
	//! state of the object (grab direction, on/off, default command...)
	uint16 state;
	//! entry in GraphicData
	/*!
		<table>
			<tr>
				<td>value</td>
				<td>description</td>
			</tr>
			<tr>
				<td>]-4000..-10]</td>
				<td>graphic image turned off</td>
			</tr>
			<tr>
				<td>-4</td>
				<td>person object (right facing)</td>
			</tr>
			<tr>
				<td>-3</td>
				<td>person object (left facing)</td>
			</tr>
			<tr>
				<td>-2</td>
				<td>animated bob (off)</td>
			</tr>
			<tr>
				<td>-1</td>
				<td>static bob (off)</td>
			</tr>
			<tr>
				<td>0</td>
				<td>object deleted</td>
			</tr>
			<tr>
				<td>]0..5000]</td>
				<td>static or animated bob (on)</td>
			</tr>
			<tr>
				<td>]5000.. [</td>
				<td>'paste down' bob</td>
			</tr>
		</table>
	*/
	int16 image;

	void readFromBE(byte *&ptr) {
		name = (int16)READ_BE_UINT16(ptr); ptr += 2;
		x = READ_BE_UINT16(ptr); ptr += 2;
		y = READ_BE_UINT16(ptr); ptr += 2;
		description = READ_BE_UINT16(ptr); ptr += 2;
		entryObj = (int16)READ_BE_UINT16(ptr); ptr += 2;
		room = READ_BE_UINT16(ptr); ptr += 2;
		state = READ_BE_UINT16(ptr); ptr += 2;
		image = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}

	void writeToBE(byte *&ptr) {
		WRITE_BE_UINT16(ptr, name); ptr += 2;
		WRITE_BE_UINT16(ptr, x); ptr += 2;
		WRITE_BE_UINT16(ptr, y); ptr += 2;
		WRITE_BE_UINT16(ptr, description); ptr += 2;
		WRITE_BE_UINT16(ptr, entryObj); ptr += 2;
		WRITE_BE_UINT16(ptr, room); ptr += 2;
		WRITE_BE_UINT16(ptr, state); ptr += 2;
		WRITE_BE_UINT16(ptr, image); ptr += 2;
	}
};


struct ObjectDescription {
	//! entry in ObjectData or ItemData
	uint16 object;
	//! type of the description
	/*!
		refer to select.c l.75-101
		<table>
			<tr>
				<td>value</td>
				<td>description</td>
			</tr>
			<tr>
				<td>0</td>
				<td>random but starts at first description</td>
			<tr>
				<td>1</td>
				<td>random</td>
			</tr>
			<tr>
				<td>2</td>
				<td>sequential with loop</td>
			</tr>
			<tr>
				<td>3</td>
				<td>sequential and set description to last</td>
			</tr>
		</table>
	*/
	uint16 type;
	//! last entry possible in OBJECT_DESCR for this object
	uint16 lastDescription;
	//! last description number used (in order to avoid re-using it)
	uint16 lastSeenNumber;

	void readFromBE(byte *&ptr) {
		object = READ_BE_UINT16(ptr); ptr += 2;
		type = READ_BE_UINT16(ptr); ptr += 2;
		lastDescription = READ_BE_UINT16(ptr); ptr += 2;
		lastSeenNumber = READ_BE_UINT16(ptr); ptr += 2;
	}

	void writeToBE(byte *&ptr) {
		WRITE_BE_UINT16(ptr, object); ptr += 2;
		WRITE_BE_UINT16(ptr, type); ptr += 2;
		WRITE_BE_UINT16(ptr, lastDescription); ptr += 2;
		WRITE_BE_UINT16(ptr, lastSeenNumber); ptr += 2;
	}
};


struct ItemData {
	//! entry in OBJECT_NAME
	int16 name;
	//! entry in OBJECT_DESCR
	uint16 description;
	//! state of the object
	uint16 state;
	//! bank bobframe
	uint16 frame;
	//! entry in OBJECT_DESCR (>0 if available)
	int16 sfxDescription;

	void readFromBE(byte *&ptr) {
		name = (int16)READ_BE_UINT16(ptr); ptr += 2;
		description = READ_BE_UINT16(ptr); ptr += 2;
		state = READ_BE_UINT16(ptr); ptr += 2;
		frame = READ_BE_UINT16(ptr); ptr += 2;
		sfxDescription = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}

	void writeToBE(byte *&ptr) {
		WRITE_BE_UINT16(ptr, name); ptr += 2;
		WRITE_BE_UINT16(ptr, description); ptr += 2;
		WRITE_BE_UINT16(ptr, state); ptr += 2;
		WRITE_BE_UINT16(ptr, frame); ptr += 2;
		WRITE_BE_UINT16(ptr, sfxDescription); ptr += 2;
	}
};


struct ActorData {
	//! room in which the actor is
	int16 room;
	//! bob number associated to this actor
	int16 bobNum;
	//! entry in ACTOR_NAME
	uint16 name;
	//! gamestate entry/value, actor is valid if GAMESTATE[slot] == value
	int16 gsSlot, gsValue;
	//! spoken text color
	uint16 color;
	//! bank bobframe for standing position of the actor
	uint16 bobFrameStanding;
	//! initial coordinates in the room
	uint16 x, y;
	//! entry in ACTOR_ANIM
	uint16 anim;
	//! bank to use to load the actor file
	uint16 bankNum;
	//! entry in ACTOR_FILE
	uint16 file;

	void readFromBE(byte *&ptr) {
		room = (int16)READ_BE_UINT16(ptr); ptr += 2;
		bobNum = (int16)READ_BE_UINT16(ptr); ptr += 2;
		name = READ_BE_UINT16(ptr); ptr += 2;
		gsSlot = (int16)READ_BE_UINT16(ptr); ptr += 2;
		gsValue = (int16)READ_BE_UINT16(ptr); ptr += 2;
		color = READ_BE_UINT16(ptr); ptr += 2;
		bobFrameStanding = READ_BE_UINT16(ptr); ptr += 2;
		x = READ_BE_UINT16(ptr); ptr += 2;
		y = READ_BE_UINT16(ptr); ptr += 2;
		anim = READ_BE_UINT16(ptr); ptr += 2;
		bankNum = READ_BE_UINT16(ptr); ptr += 2;
		file = READ_BE_UINT16(ptr); ptr += 2;
		// Fix the actor data (see queen.c - l.1518-1519). When there is no
		// valid actor file, we must load the data from the objects room bank.
		// This bank has number 15 (not 10 as in the data files).
		if (file == 0) {
			bankNum = 15;
		}
	}
};


struct CmdListData {
	//! action to perform
	Verb verb;
	//! first object used in the action
	int16 nounObj1;
	//! second object used in the action
	int16 nounObj2;
	//! song to play (>0: playbefore, <0: playafter)
	int16 song;
	//! if set, P2_SET_AREAS must be called (using CmdArea)
	bool setAreas;
	//! if set, P3_SET_OBJECTS must be called (using CmdObject)
	bool setObjects;
	//! if set, P4_SET_ITEMS must be called (using CmdInventory)
	bool setItems;
	//! if set, P1_SET_CONDITIONS must be called (using CmdGameState)
	bool setConditions;
	//! graphic image order
	int16 imageOrder;
	//! special section to execute (refer to execute.c l.423-451)
	int16 specialSection;

	void readFromBE(byte *&ptr) {
		verb = (Verb)READ_BE_UINT16(ptr); ptr += 2;
		nounObj1 = (int16)READ_BE_UINT16(ptr); ptr += 2;
		nounObj2 = (int16)READ_BE_UINT16(ptr); ptr += 2;
		song = (int16)READ_BE_UINT16(ptr); ptr += 2;
		setAreas = READ_BE_UINT16(ptr) != 0; ptr += 2;
		setObjects = READ_BE_UINT16(ptr) != 0; ptr += 2;
		setItems = READ_BE_UINT16(ptr) != 0; ptr += 2;
		setConditions = READ_BE_UINT16(ptr) != 0; ptr += 2;
		imageOrder = (int16)READ_BE_UINT16(ptr); ptr += 2;
		specialSection = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}

	bool match(const Verb& v, int16 obj1, int16 obj2) const {
		return verb == v && nounObj1 == obj1 && nounObj2 == obj2;
	}
};


struct CmdArea {
	//! CmdListData number
	int16 id;
	//! area to turn off/on (<0: off, >0: on)
	int16 area;
	//! room in which the area must be changed
	uint16 room;

	void readFromBE(byte *&ptr) {
		id = (int16)READ_BE_UINT16(ptr); ptr += 2;
		area = (int16)READ_BE_UINT16(ptr); ptr += 2;
		room = READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct CmdObject {
	//! CmdListData number
	int16 id;
	//! >0: show, <0: hide
	int16 dstObj;
	//! >0: copy from srcObj, 0: nothing, -1: delete dstObj
	int16 srcObj;

	void readFromBE(byte *&ptr) {
		id = (int16)READ_BE_UINT16(ptr); ptr += 2;
		dstObj = (int16)READ_BE_UINT16(ptr); ptr += 2;
		srcObj = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct CmdInventory {
	//! CmdListData number
	int16 id;
	//! <0: delete, >0: add
	int16 dstItem;
	//! >0: valid
	int16 srcItem;

	void readFromBE(byte *&ptr) {
		id = (int16)READ_BE_UINT16(ptr); ptr += 2;
		dstItem = (int16)READ_BE_UINT16(ptr); ptr += 2;
		srcItem = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct CmdGameState {
	//! CmdListData number
	int16 id;
	int16 gameStateSlot;
	int16 gameStateValue;
	uint16 speakValue;

	void readFromBE(byte *&ptr) {
		id = (int16)READ_BE_UINT16(ptr); ptr += 2;
		gameStateSlot = (int16)READ_BE_UINT16(ptr); ptr += 2;
		gameStateValue = (int16)READ_BE_UINT16(ptr); ptr += 2;
		speakValue = READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct FurnitureData {
	//! room in which the furniture are
	int16 room;
	//! furniture object number
	/*!
		<table>
			<tr>
				<td>range</td>
				<td>type</td>
			</tr>
			<tr>
				<td>]0..5000]</td>
				<td>static or animated</td>
			</tr>
			<tr>
				<td>]5000..[</td>
				<td>paste down</td>
			</tr>
		</table>
	*/
	int16 objNum;

	void readFromBE(byte *&ptr) {
		room = (int16)READ_BE_UINT16(ptr); ptr += 2;
		objNum = (int16)READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct GraphicAnim {
	int16 keyFrame;
	int16 frame;
	uint16 speed;

	void readFromBE(byte *&ptr) {
		keyFrame = (int16)READ_BE_UINT16(ptr); ptr += 2;
		frame = (int16)READ_BE_UINT16(ptr); ptr += 2;
		speed = READ_BE_UINT16(ptr); ptr += 2;
	}
};


struct AnimFrame {
	uint16 frame;
	uint16 speed;
};


struct Person {
	//! actor settings to use
	const ActorData *actor;
	//! actor name
	const char *name;
	//! animation string
	const char *anim;
	//! current frame
	uint16 bobFrame;
};


struct TalkSelected {
	bool hasTalkedTo;
	int16 values[4];

	void readFromBE(byte *&ptr) {
		hasTalkedTo = READ_BE_UINT16(ptr) != 0; ptr += 2;
		for (int i = 0; i < 4; i++) {
			values[i] = (int16)READ_BE_UINT16(ptr); ptr += 2;
		}
	}

	void writeToBE(byte *&ptr) {
		WRITE_BE_UINT16(ptr, (uint16)hasTalkedTo); ptr += 2;
		for (int i = 0; i < 4; i++) {
			WRITE_BE_UINT16(ptr, values[i]); ptr += 2;
		}
	}
};


struct BobFrame {
	uint16 width, height;
	uint16 xhotspot, yhotspot;
	uint8 *data;
};


} // End of namespace Queen

#endif
