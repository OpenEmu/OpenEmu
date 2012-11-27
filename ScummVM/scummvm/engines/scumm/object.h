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

#ifndef SCUMM_OBJECT_H
#define SCUMM_OBJECT_H

namespace Scumm {

static inline int OBJECT_V0(int id, byte type) {
	assert(id < 256);
	return (type << 8 | id);
}
#define OBJECT_V0_ID(obj)	(obj & 0xFF)
#define OBJECT_V0_TYPE(obj)	((obj >> 8) & 0xFF)

enum ObjectV0Type {
	kObjectV0TypeFG = 0,    // foreground object
	                        //   - with owner/state, might (but has not to) be pickupable
	                        //     -> with entry in _objectOwner/StateTable
	                        //     -> all objects in _inventory have this type
	                        //   - image can be exchanged (background overlay)
	kObjectV0TypeBG = 1,    // background object
	                        //   - without owner/state, not pickupable  (room only)
	                        //     -> without entry in _objectOwner/StateTable
	                        //   - image cannot be exchanged (part of background image)
	kObjectV0TypeActor = 2  // object is an actor
};

enum ObjectClass {
	kObjectClassNeverClip = 20,
	kObjectClassAlwaysClip = 21,
	kObjectClassIgnoreBoxes = 22,
	kObjectClassYFlip = 29,
	kObjectClassXFlip = 30,
	kObjectClassPlayer = 31,	// Actor is controlled by the player
	kObjectClassUntouchable = 32
};

enum ObjectStateV2 {
	kObjectStatePickupable = 1,
	kObjectStateUntouchable = 2,
	kObjectStateLocked = 4,

	// FIXME: Not quite sure how to name state 8. It seems to mark some kind
	// of "activation state" for the given object. E.g. is a door open?
	// Is a drawer extended? In addition it is used to toggle the look
	// of objects that the user can "pick up" (i.e. it is set in
	// o2_pickupObject together with kObjectStateUntouchable). So in a sense,
	// it can also mean "invisible" in some situations.
	kObjectState_08 = 8
};

struct ObjectData {
	uint32 OBIMoffset;
	uint32 OBCDoffset;
	int16 walk_x, walk_y;
	uint16 obj_nr;
	int16 x_pos;
	int16 y_pos;
	uint16 width;
	uint16 height;
	byte actordir;
	byte parent;
	byte parentstate;
	byte state;
	byte fl_object_index;
	byte flags;
};

#include "common/pack-start.h"	// START STRUCT PACKING

struct RoomHeader {
	union {
		struct {
			uint16 width, height;
			uint16 numObjects;
		} old;

		struct {
			uint32 version;
			uint16 width, height;
			uint16 numObjects;
		} v7;

		struct {
			uint32 version;
			uint32 width, height;
			uint32 numObjects;
			uint32 numZBuffer;
			uint32 transparency;
		} v8;
	};
} PACKED_STRUCT;

struct CodeHeader {
	union {
		struct {
			uint16 obj_id;
			byte x, y, w, h;
			byte flags;
			byte parent;
			int16 walk_x;
			int16 walk_y;
			byte actordir;
		} v5;

		struct {
			uint16 obj_id;
			int16 x, y;
			uint16 w, h;
			byte flags, parent;
			uint16 unk1;
			uint16 unk2;
			byte actordir;
		} v6;

		struct {
			uint32 version;
			uint16 obj_id;
			byte parent;
			byte parentstate;
		} v7;

	};
} PACKED_STRUCT;

struct ImageHeader { /* file format */
	union {
		struct {
			uint16 obj_id;
			uint16 image_count;
			uint16 unk[1];
			byte flags;
			byte unk1;
			uint16 unk2[2];
			uint16 width;
			uint16 height;
			uint16 hotspot_num;
			struct {
				int16 x, y;
			} hotspot[15];
		} old;

		struct {
			uint32 version;
			uint16 obj_id;
			uint16 image_count;
			int16 x_pos, y_pos;
			uint16 width, height;
			byte unk2[3];
			byte actordir;
			uint16 hotspot_num;
			struct {
				int16 x, y;
			} hotspot[15];
		} v7;

		struct {
			char name[32];
			uint32 unk_1[2];
			uint32 version;		// 801 in COMI, 800 in the COMI demo
			uint32 image_count;
			uint32 x_pos;
			uint32 y_pos;
			uint32 width;
			uint32 height;
			uint32 actordir;
			uint32 flags;	// This field is missing in the COMI demo (version == 800) !
			struct {
				int32 x, y;
			} hotspot[15];
		} v8;
	};
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

struct FindObjectInRoom {
	const CodeHeader *cdhd;
	const byte *obcd;
	const byte *obim;
	const byte *roomptr;
};

enum FindObjectWhat {
	foCodeHeader = 1,
	foImageHeader = 2,
	foCheckAlreadyLoaded = 4
};

} // End of namespace Scumm


#endif
