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

#include "scumm/actor.h"
#include "scumm/bomp.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm_v0.h"
#include "scumm/scumm_v8.h"
#include "scumm/usage_bits.h"
#include "scumm/util.h"

namespace Scumm {

void ScummEngine::addObjectToInventory(uint obj, uint room) {
	int idx, slot;
	uint32 size;
	const byte *ptr;
	byte *dst;
	FindObjectInRoom foir;

	debug(1, "Adding object %d from room %d into inventory", obj, room);

	if (whereIsObject(obj) == WIO_FLOBJECT) {
		idx = getObjectIndex(obj);
		assert(idx >= 0);
		ptr = getResourceAddress(rtFlObject, _objs[idx].fl_object_index) + 8;
		size = READ_BE_UINT32(ptr + 4);
	} else {
		findObjectInRoom(&foir, foCodeHeader, obj, room);
		if (_game.features & GF_OLD_BUNDLE)
			size = READ_LE_UINT16(foir.obcd);
		else if (_game.features & GF_SMALL_HEADER)
			size = READ_LE_UINT32(foir.obcd);
		else
			size = READ_BE_UINT32(foir.obcd + 4);
		ptr = foir.obcd;
	}

	slot = getInventorySlot();
	_inventory[slot] = obj;
	dst = _res->createResource(rtInventory, slot, size);
	assert(dst);
	memcpy(dst, ptr, size);
}

int ScummEngine::getInventorySlot() {
	int i;
	for (i = 0; i < _numInventory; i++) {
		if (_inventory[i] == 0)
			return i;
	}
	error("Inventory full, %d max items", _numInventory);
	return -1;
}

int ScummEngine::findInventory(int owner, int idx) {
	int count = 1, i, obj;
	for (i = 0; i < _numInventory; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj) == owner && count++ == idx)
			return obj;
	}
	return 0;
}

int ScummEngine::getInventoryCount(int owner) {
	int i, obj;
	int count = 0;
	for (i = 0; i < _numInventory; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj) == owner)
			count++;
	}
	return count;
}

void ScummEngine::setOwnerOf(int obj, int owner) {
	ScriptSlot *ss;

	// In Sam & Max this is necessary, or you won't get your stuff back
	// from the Lost and Found tent after riding the Cone of Tragedy. But
	// it probably applies to all V6+ games. See bugs #493153 and #907113.
	// FT disassembly is checked, behavior is correct. [sev]

	int arg = (_game.version >= 6) ? obj : 0;

	// WORKAROUND for bug #1917981: Game crash when finishing Indy3 demo.
	// Script 94 tries to empty the inventory but does so in a bogus way.
	// This causes it to try to remove object 0 from the inventory.
	if (_game.id == GID_PASS && obj == 0 && vm.slot[_currentScript].number == 94)
		return;
	assert(obj > 0);

	if (owner == 0) {
		clearOwnerOf(obj);

		// FIXME: See bug #1535358 and many others. Essentially, the following
		// code, while matching disasm of various versions of the SCUMM engine,
		// is total bullocks, and leads to odd crashes due to out-of-bounds
		// array (read) access. Three "famous" crashes were caused by this:
		//   Monkey Island 1: Using meat with flower
		//   FOA: Using ribcage with another item
		//   DOTT: Using stamp with contract
		//
		// The bad code:
		//   if (ss->where == WIO_INVENTORY && _inventory[ss->number] == obj) {
		// That check makes no sense at all: _inventory only contains 80 items,
		// which are in the order the player picked up items. We can only
		// guess that the SCUMM coders meant to write
		//   if (ss->where == WIO_INVENTORY && ss->number == obj) {
		// which would ensure that an object script that nukes itself gets
		// stopped. Alas, we can't just make that change, since it could
		// lead to new regressions.
		// Another fix would be to completely remove this check, which should
		// not cause much problems, since it'll only succeed by pure chance.
		//
		// For now we follow a more defensive route: We perform the check
		// if ss->number is small enough.

		ss = &vm.slot[_currentScript];
		if (ss->where == WIO_INVENTORY) {
			if (ss->number < _numInventory && _inventory[ss->number] == obj) {
				error("Odd setOwnerOf case #1: Please report to Fingolfin where you encountered this");
				putOwner(obj, 0);
				runInventoryScript(arg);
				stopObjectCode();
				return;
			}
			if (ss->number == obj)
				error("Odd setOwnerOf case #2: Please report to Fingolfin where you encountered this");
		}
	}

	putOwner(obj, owner);
	runInventoryScript(arg);
}

void ScummEngine::clearOwnerOf(int obj) {
	int i;

	// Stop the associated object script code (else crashes might occurs)
	stopObjectScript(obj);

	// If the object is "owned" by a the current room, we scan the
	// object list and (only if it's a floating object) nuke it.
	if (getOwner(obj) == OF_OWNER_ROOM) {
		for (i = 0; i < _numLocalObjects; i++)  {
			if (_objs[i].obj_nr == obj && _objs[i].fl_object_index) {
				// Removing an flObject from a room means we can nuke it
				_res->nukeResource(rtFlObject, _objs[i].fl_object_index);
				_objs[i].obj_nr = 0;
				_objs[i].fl_object_index = 0;
			}
		}
	} else {

		// Alternatively, scan the inventory to see if the object is in there...
		for (i = 0; i < _numInventory; i++) {
			if (_inventory[i] == obj) {
				assert(WIO_INVENTORY == whereIsObject(obj));

				// Found the object! Nuke it from the inventory.
				_res->nukeResource(rtInventory, i);
				_inventory[i] = 0;

				// Now fill up the gap removing the object from the inventory created.
				for (i = 0; i < _numInventory - 1; i++) {
					if (!_inventory[i] && _inventory[i+1]) {
						_inventory[i] = _inventory[i+1];
						_inventory[i+1] = 0;
						// FIXME FIXME FIXME: This is incomplete, as we do not touch flags, status... BUG
						_res->_types[rtInventory][i]._address = _res->_types[rtInventory][i + 1]._address;
						_res->_types[rtInventory][i]._size = _res->_types[rtInventory][i + 1]._size;
						_res->_types[rtInventory][i + 1]._address = NULL;
						_res->_types[rtInventory][i + 1]._size = 0;
					}
				}
				break;
			}
		}
	}
}

bool ScummEngine::getClass(int obj, int cls) const {
	if (_game.version == 0)
		return false;

	assertRange(0, obj, _numGlobalObjects - 1, "object");
	cls &= 0x7F;
	assertRange(1, cls, 32, "class");

	if (_game.features & GF_SMALL_HEADER) {
		// Translate the new (V5) object classes to the old classes
		// (for those which differ).
		switch (cls) {
		case kObjectClassUntouchable:
			cls = 24;
			break;
		case kObjectClassPlayer:
			cls = 23;
			break;
		case kObjectClassXFlip:
			cls = 19;
			break;
		case kObjectClassYFlip:
			cls = 18;
			break;
		}
	}

	return (_classData[obj] & (1 << (cls - 1))) != 0;
}

void ScummEngine::putClass(int obj, int cls, bool set) {
	if (_game.version == 0)
		return;

	assertRange(0, obj, _numGlobalObjects - 1, "object");
	cls &= 0x7F;
	assertRange(1, cls, 32, "class");

	if (_game.features & GF_SMALL_HEADER) {
		// Translate the new (V5) object classes to the old classes
		// (for those which differ).
		switch (cls) {
		case kObjectClassUntouchable:
			cls = 24;
			break;
		case kObjectClassPlayer:
			cls = 23;
			break;
		case kObjectClassXFlip:
			cls = 19;
			break;
		case kObjectClassYFlip:
			cls = 18;
			break;
		}
	}

	if (set)
		_classData[obj] |= (1 << (cls - 1));
	else
		_classData[obj] &= ~(1 << (cls - 1));

	if (_game.version <= 4 && obj >= 1 && obj < _numActors) {
		_actors[obj]->classChanged(cls, set);
	}
}

int ScummEngine::getOwner(int obj) const {
	assertRange(0, obj, _numGlobalObjects - 1, "object");
	return _objectOwnerTable[obj];
}

void ScummEngine::putOwner(int obj, int owner) {
	assertRange(0, obj, _numGlobalObjects - 1, "object");
	assertRange(0, owner, 0xFF, "owner");
	_objectOwnerTable[obj] = owner;
}

int ScummEngine::getState(int obj) {
	assertRange(0, obj, _numGlobalObjects - 1, "object");

	if (!_copyProtection) {
		// I knew LucasArts sold cracked copies of the original Maniac Mansion,
		// at least as part of Day of the Tentacle. Apparently they also sold
		// cracked versions of the enhanced version. At least in Germany.
		//
		// This will keep the security door open at all times. I can only
		// assume that 182 and 193 each correspond to one particular side of
		// it. Fortunately this does not prevent frustrated players from
		// blowing up the mansion, should they feel the urge to.

		if (_game.id == GID_MANIAC && _game.version != 0 && (obj == 182 || obj == 193))
			_objectStateTable[obj] |= kObjectState_08;
	}

	return _objectStateTable[obj];
}

void ScummEngine::putState(int obj, int state) {
	assertRange(0, obj, _numGlobalObjects - 1, "object");
	assertRange(0, state, 0xFF, "state");
	_objectStateTable[obj] = state;
}

int ScummEngine::getObjectRoom(int obj) const {
	assertRange(0, obj, _numGlobalObjects - 1, "object");
	return _objectRoomTable[obj];
}

int ScummEngine::getObjectIndex(int object) const {
	int i;

	if (object < 1)
		return -1;

	for (i = (_numLocalObjects-1); i > 0; i--) {
		if (_objs[i].obj_nr == object)
			return i;
	}
	return -1;
}

int ScummEngine::whereIsObject(int object) const {
	int i;

	// Note: in MM v0 bg objects are greater _numGlobalObjects
	if (_game.version != 0 && object >= _numGlobalObjects)
		return WIO_NOT_FOUND;

	if (object < 1)
		return WIO_NOT_FOUND;

	if ((_game.version != 0 || OBJECT_V0_TYPE(object) == 0) &&
		 _objectOwnerTable[object] != OF_OWNER_ROOM)
	{
		for (i = 0; i < _numInventory; i++)
			if (_inventory[i] == object)
				return WIO_INVENTORY;
		return WIO_NOT_FOUND;
	}

	for (i = (_numLocalObjects-1); i > 0; i--) {
		if (_objs[i].obj_nr == object) {
			if (_objs[i].fl_object_index)
				return WIO_FLOBJECT;
			return WIO_ROOM;
		}
	}

	return WIO_NOT_FOUND;
}

int ScummEngine::getObjectOrActorXY(int object, int &x, int &y) {
	Actor *act;

	if (objIsActor(object)) {
		act = derefActorSafe(objToActor(object), "getObjectOrActorXY");
		if (act && act->isInCurrentRoom()) {
			x = act->getRealPos().x;
			y = act->getRealPos().y;
			return 0;
		} else
			return -1;
	}

	switch (whereIsObject(object)) {
	case WIO_NOT_FOUND:
		return -1;
	case WIO_INVENTORY:
		if (objIsActor(_objectOwnerTable[object])) {
			act = derefActor(_objectOwnerTable[object], "getObjectOrActorXY(2)");
			if (act && act->isInCurrentRoom()) {
				x = act->getRealPos().x;
				y = act->getRealPos().y;
				return 0;
			}
		}
		return -1;
	}
	getObjectXYPos(object, x, y);
	return 0;
}

/**
 * Return the position of an object.
 * Returns X, Y and direction in angles
 */
void ScummEngine::getObjectXYPos(int object, int &x, int &y, int &dir) {
	int idx = getObjectIndex(object);
	assert(idx >= 0);
	ObjectData &od = _objs[idx];
	int state;
	const byte *ptr;
	const ImageHeader *imhd;

	if (_game.version >= 6) {
		state = getState(object) - 1;
		if (state < 0)
			state = 0;

		ptr = getOBIMFromObjectData(od);
		if (!ptr) {
			// FIXME: We used to assert here, but it seems that in the nexus
			// in The Dig, this can happen, at least with old savegames, and
			// it's safe to continue...
			debug(0, "getObjectXYPos: Can't find object %d", object);
			return;
		}
		imhd = (const ImageHeader *)findResourceData(MKTAG('I','M','H','D'), ptr);
		assert(imhd);
		if (_game.version == 8) {
			switch (FROM_LE_32(imhd->v8.version)) {
			case 800:
				x = od.x_pos + (int32)READ_LE_UINT32((const byte *)imhd + 8 * state + 0x44);
				y = od.y_pos + (int32)READ_LE_UINT32((const byte *)imhd + 8 * state + 0x48);
				break;
			case 801:
				x = od.x_pos + (int32)READ_LE_UINT32(&imhd->v8.hotspot[state].x);
				y = od.y_pos + (int32)READ_LE_UINT32(&imhd->v8.hotspot[state].y);
				break;
			default:
				error("Unsupported image header version %d", FROM_LE_32(imhd->v8.version));
			}
		} else if (_game.version == 7) {
			x = od.x_pos + (int16)READ_LE_UINT16(&imhd->v7.hotspot[state].x);
			y = od.y_pos + (int16)READ_LE_UINT16(&imhd->v7.hotspot[state].y);
		} else {
			x = od.x_pos + (int16)READ_LE_UINT16(&imhd->old.hotspot[state].x);
			y = od.y_pos + (int16)READ_LE_UINT16(&imhd->old.hotspot[state].y);
		}
	} else if (_game.version <= 2) {
		x = od.walk_x;
		y = od.walk_y;

		// Adjust x, y when no actor direction is set, but only perform this
		// adjustment for V0 games (e.g. MM C64), otherwise certain scenes in
		// newer games are affected as well (e.g. the interior of the Shuttle
		// Bus scene in Zak V2, where no actor is present). Refer to bug #3526089.
		if (!od.actordir && _game.version == 0) {
			x = od.x_pos + od.width / 2;
			y = od.y_pos + od.height / 2;
		}
		x = x >> V12_X_SHIFT;
		y = y >> V12_Y_SHIFT;
	} else {
		x = od.walk_x;
		y = od.walk_y;
	}
	if (_game.version == 8)
		dir = fromSimpleDir(1, od.actordir);
	else
		dir = oldDirToNewDir(od.actordir & 3);
}

int ScummEngine::getDist(int x, int y, int x2, int y2) {
	int a = ABS(y - y2);
	int b = ABS(x - x2);
	return MAX(a, b);
}

int ScummEngine::getObjActToObjActDist(int a, int b) {
	int x, y, x2, y2;
	Actor *acta = NULL;
	Actor *actb = NULL;

	if (objIsActor(a))
		acta = derefActorSafe(objToActor(a), "getObjActToObjActDist");

	if (objIsActor(b))
		actb = derefActorSafe(objToActor(b), "getObjActToObjActDist(2)");

	if (acta && actb && acta->getRoom() == actb->getRoom() && acta->getRoom() && !acta->isInCurrentRoom())
		return 0;

	if (getObjectOrActorXY(a, x, y) == -1)
		return 0xFF;

	if (getObjectOrActorXY(b, x2, y2) == -1)
		return 0xFF;

	// Perform adjustXYToBeInBox() *only* if the first item is an
	// actor and the second is an object. This used to not check
	// whether the second item is a non-actor, which caused bug
	// #853874).
	if (acta && !actb) {
		AdjustBoxResult r = acta->adjustXYToBeInBox(x2, y2);
		x2 = r.x;
		y2 = r.y;
	}


	// Now compute the distance between the two points
	return getDist(x, y, x2, y2);
}

int ScummEngine::findObject(int x, int y) {
	int i, b;
	byte a;
	const int mask = (_game.version <= 2) ? kObjectState_08 : 0xF;

	for (i = 1; i < _numLocalObjects; i++) {
		if ((_objs[i].obj_nr < 1) || getClass(_objs[i].obj_nr, kObjectClassUntouchable))
			continue;

		if ((_game.version == 0 && OBJECT_V0_TYPE(_objs[i].obj_nr) == kObjectV0TypeFG) ||
			(_game.version > 0 && _game.version <= 2)) {
			if (_objs[i].state & kObjectStateUntouchable)
				continue;
		}

		b = i;
		do {
			a = _objs[b].parentstate;
			b = _objs[b].parent;
			if (b == 0) {
#ifdef ENABLE_HE
				if (_game.heversion >= 71) {
					if (((ScummEngine_v71he *)this)->_wiz->polygonHit(_objs[i].obj_nr, x, y))
						return _objs[i].obj_nr;
				}
#endif
				if (_objs[i].x_pos <= x && _objs[i].width + _objs[i].x_pos > x &&
				    _objs[i].y_pos <= y && _objs[i].height + _objs[i].y_pos > y)
					return _objs[i].obj_nr;
				break;
			}
		} while ((_objs[b].state & mask) == a);
	}

	return 0;
}

void ScummEngine::drawRoomObject(int i, int arg) {
	ObjectData *od;
	byte a;
	const int mask = (_game.version <= 2) ? kObjectState_08 : 0xF;

	od = &_objs[i];
	if ((i < 1) || (od->obj_nr < 1) || !od->state)
		return;

	do {
		a = od->parentstate;
		if (!od->parent) {
			if (_game.version <= 6 || od->fl_object_index == 0)
				drawObject(i, arg);
			break;
		}
		od = &_objs[od->parent];
	} while ((od->state & mask) == a);
}

void ScummEngine::drawRoomObjects(int arg) {
	int i;
	const int mask = (_game.version <= 2) ? kObjectState_08 : 0xF;

	if (_game.heversion >= 60) {
		// In HE games, normal objects are drawn, followed by FlObjects.
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr > 0 && (_objs[i].state & mask) && _objs[i].fl_object_index == 0)
				drawRoomObject(i, arg);
		}
		for (i = (_numLocalObjects-1); i > 0; i--) {
			if (_objs[i].obj_nr > 0 && (_objs[i].state & mask) && _objs[i].fl_object_index != 0)
				drawRoomObject(i, arg);
		}
	} else if (_game.id == GID_SAMNMAX) {
		// In Sam & Max, objects are drawn in reverse order.
		for (i = 1; i < _numLocalObjects; i++)
			if (_objs[i].obj_nr > 0)
				drawRoomObject(i, arg);
	} else {
		for (i = (_numLocalObjects-1); i > 0; i--)
			if (_objs[i].obj_nr > 0 && (_objs[i].state & mask)) {
				drawRoomObject(i, arg);
			}
	}
}

void ScummEngine::drawObject(int obj, int arg) {
	if (_skipDrawObject)
		return;

	ObjectData &od = _objs[obj];
	int height, width;
	const byte *ptr;
	int x, a, numstrip;
	int tmp;

	if (_bgNeedsRedraw)
		arg = 0;

	if (od.obj_nr == 0)
		return;

	assertRange(0, od.obj_nr, _numGlobalObjects - 1, "object");

	const int xpos = od.x_pos / 8;
	const int ypos = od.y_pos;

	width = od.width / 8;
	height = od.height &= 0xFFFFFFF8;	// Mask out last 3 bits

	// Short circuit for objects which aren't visible at all.
	if (width == 0 || xpos > _screenEndStrip || xpos + width < _screenStartStrip)
		return;

	// For objects without image in Apple II & Commodore 64 versions of Maniac Mansion
	if (_game.version == 0 && od.OBIMoffset == 0)
		return;

	ptr = getObjectImage(getOBIMFromObjectData(od), getState(od.obj_nr));
	if (!ptr)
		return;

	x = 0xFFFF;

	for (a = numstrip = 0; a < width; a++) {
		tmp = xpos + a;
		if (tmp < _screenStartStrip || _screenEndStrip < tmp)
			continue;
		if (arg > 0 && _screenStartStrip + arg <= tmp)
			continue;
		if (arg < 0 && tmp <= _screenEndStrip + arg)
			continue;
		setGfxUsageBit(tmp, USAGE_BIT_DIRTY);
		if (tmp < x)
			x = tmp;
		numstrip++;
	}

	if (numstrip != 0) {
		byte flags = od.flags | Gdi::dbObjectMode;

		// Sam & Max needs this to fix object-layering problems with
		// the inventory and conversation icons.
		if ((_game.id == GID_SAMNMAX && getClass(od.obj_nr, kObjectClassIgnoreBoxes)) ||
		    (_game.id == GID_FT && getClass(od.obj_nr, kObjectClassPlayer)))
			flags |= Gdi::dbDrawMaskOnAll;

#ifdef ENABLE_HE
		if (_game.heversion >= 70 && findResource(MKTAG('S','M','A','P'), ptr) == NULL)
			_gdi->drawBMAPObject(ptr, &_virtscr[kMainVirtScreen], obj, od.x_pos, od.y_pos, od.width, od.height);
		else
#endif
			_gdi->drawBitmap(ptr, &_virtscr[kMainVirtScreen], x, ypos, width * 8, height, x - xpos, numstrip, flags);
	}
}

void ScummEngine::clearRoomObjects() {
	int i;

	if (_game.features & GF_SMALL_HEADER) {
		for (i = 0; i < _numLocalObjects; i++) {
			_objs[i].obj_nr = 0;
		}
	} else {
		for (i = 0; i < _numLocalObjects; i++) {
			if (_objs[i].obj_nr < 1)	// Optimise codepath
				continue;

			// Nuke all non-flObjects (flObjects are nuked in script.cpp)
			if (_objs[i].fl_object_index == 0) {
				_objs[i].obj_nr = 0;
			} else {
				// Nuke all unlocked flObjects
				if (!_res->isLocked(rtFlObject, _objs[i].fl_object_index)) {
					_res->nukeResource(rtFlObject, _objs[i].fl_object_index);
					_objs[i].obj_nr = 0;
					_objs[i].fl_object_index = 0;
				}
			}
		}
	}
}

void ScummEngine_v70he::resetRoomObjects() {
	ScummEngine_v60he::resetRoomObjects();
	restoreFlObjects();
}

void ScummEngine_v70he::clearRoomObjects() {
	_numStoredFlObjects = 0;

	for (int i = 0; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr < 1)	// Optimise codepath
			continue;

		if (_objs[i].fl_object_index != 0) {
			if (!_res->isLocked(rtFlObject, _objs[i].fl_object_index)) {
				_res->nukeResource(rtFlObject, _objs[i].fl_object_index);
			} else {
				storeFlObject(i);
			}
		}
		_objs[i].fl_object_index = 0;
		_objs[i].obj_nr = 0;
	}

	if (_currentRoom == 0)
		restoreFlObjects();
}

void ScummEngine_v70he::storeFlObject(int slot) {
	memcpy(&_storedFlObjects[_numStoredFlObjects], &_objs[slot], sizeof(_objs[slot]));
	_numStoredFlObjects++;
	if (_numStoredFlObjects > 100)
		error("Too many flobjects saved on room transition");
}

void ScummEngine_v70he::restoreFlObjects() {
	int i, slot;

	for (i = 0; i < _numStoredFlObjects; i++) {
		slot = findLocalObjectSlot();
		memcpy(&_objs[slot], &_storedFlObjects[i], sizeof(_objs[slot]));
	}

	_numStoredFlObjects = 0;
}

void ScummEngine::resetRoomObjects() {
	int i, j;
	ObjectData *od;
	const byte *ptr;
	uint16 obim_id;
	const byte *room, *searchptr, *rootptr;
	const CodeHeader *cdhd;

	room = getResourceAddress(rtRoom, _roomResource);

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	if (_game.version == 8)
		searchptr = rootptr = getResourceAddress(rtRoomScripts, _roomResource);
	else
		searchptr = rootptr = room;
	assert(searchptr);

	// Load in new room objects
	ResourceIterator	obcds(searchptr, false);
	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		ptr = obcds.findNext(MKTAG('O','B','C','D'));
		if (ptr == NULL)
			error("Room %d missing object code block(s)", _roomResource);

		od->OBCDoffset = ptr - rootptr;
		cdhd = (const CodeHeader *)findResourceData(MKTAG('C','D','H','D'), ptr);

		if (_game.version >= 7)
			od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));
		else if (_game.version == 6)
			od->obj_nr = READ_LE_UINT16(&(cdhd->v6.obj_id));
		else
			od->obj_nr = READ_LE_UINT16(&(cdhd->v5.obj_id));

		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			ptr = findResource(MKTAG('V','E','R','B'), ptr);
			dumpResource(buf, od->obj_nr, ptr);
		}

	}

	searchptr = room;
	ResourceIterator	obims(room, false);
	for (i = 0; i < _numObjectsInRoom; i++) {
		ptr = obims.findNext(MKTAG('O','B','I','M'));
		if (ptr == NULL)
			error("Room %d missing image blocks(s)", _roomResource);

		obim_id = getObjectIdFromOBIM(ptr);

		for (j = 1; j < _numLocalObjects; j++) {
			if (_objs[j].obj_nr == obim_id)
				_objs[j].OBIMoffset = ptr - room;
		}
	}

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr && !_objs[i].fl_object_index)
			resetRoomObject(&_objs[i], room);
	}
}

void ScummEngine_v3old::resetRoomObjects() {
	int i;
	ObjectData *od;
	const byte *room, *ptr;

	room = getResourceAddress(rtRoom, _roomResource);

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	if (_game.version <= 2)
		ptr = room + 28;
	else
		ptr = room + 29;

	// Default pointer of objects without image, in v0 version of Maniac Mansion
	int defaultPtr = READ_LE_UINT16(ptr + 2 * _numObjectsInRoom);

	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		if (_game.version == 0 && READ_LE_UINT16(ptr) == defaultPtr)
			od->OBIMoffset = 0;
		else
			od->OBIMoffset = READ_LE_UINT16(ptr);

		od->OBCDoffset = READ_LE_UINT16(ptr + 2 * _numObjectsInRoom);
		resetRoomObject(od, room);

		ptr += 2;

		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			dumpResource(buf, od->obj_nr, room + od->OBCDoffset);
		}
	}
}

void ScummEngine_v4::resetRoomObjects() {
	int i, j;
	ObjectData *od;
	const byte *ptr;
	uint16 obim_id;
	const byte *room;

	room = getResourceAddress(rtRoom, _roomResource);

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	ResourceIterator	obcds(room, true);
	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		ptr = obcds.findNext(MKTAG('O','B','C','D'));
		if (ptr == NULL)
			error("Room %d missing object code block(s)", _roomResource);

		od->OBCDoffset = ptr - room;
		od->obj_nr = READ_LE_UINT16(ptr + 6);
		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			dumpResource(buf, od->obj_nr, ptr);
		}
	}

	ResourceIterator	obims(room, true);
	for (i = 0; i < _numObjectsInRoom; i++) {
		// In the PC Engine version of Loom, there aren't image blocks
		// for all objects.
		ptr = obims.findNext(MKTAG('O','B','I','M'));
		if (ptr == NULL)
			break;

		obim_id = READ_LE_UINT16(ptr + 6);

		for (j = 1; j < _numLocalObjects; j++) {
			if (_objs[j].obj_nr == obim_id)
				_objs[j].OBIMoffset = ptr - room;
		}
	}

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr && !_objs[i].fl_object_index) {
			resetRoomObject(&_objs[i], room);
		}
	}
}

void ScummEngine_v0::resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr) {
	assert(room);
	const byte *ptr = room + od->OBCDoffset;
	ptr -= 2;

	od->obj_nr = OBJECT_V0(*(ptr + 6), *(ptr + 7));

	od->x_pos = *(ptr + 8) * 8;
	od->y_pos = ((*(ptr + 9)) & 0x7F) * 8;

	od->parentstate = (*(ptr + 9) & 0x80) ? 1 : 0;
	od->parentstate *= 8;

	od->width = *(ptr + 10) * 8;

	od->parent = *(ptr + 11);

	od->walk_x = *(ptr + 12) * 8;
	od->walk_y = (*(ptr + 13) & 0x1f) * 8;
	od->actordir = (*(ptr + 14)) & 7;
	od->height = *(ptr + 14) & 0xf8;
}

void ScummEngine_v4::resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr) {
	assert(room);
	const byte *ptr = room + od->OBCDoffset;

	if (_game.features & GF_OLD_BUNDLE)
		ptr -= 2;

	od->obj_nr = READ_LE_UINT16(ptr + 6);

	if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine) {
		od->x_pos = *(ptr + 8) * 8;
		od->y_pos = ((*(ptr + 9)) & 0x7F) * 8;

		od->parentstate = (*(ptr + 9) & 0x80) ? 1 : 0;
		od->width = *(ptr + 10) * 8;

		// TODO: Where is parent data?
		od->parent = 0;
		od->walk_x = READ_LE_UINT16(ptr + 11);
		od->walk_y = READ_LE_UINT16(ptr + 13);
		od->actordir = (*(ptr + 15)) & 7;
		od->height = *(ptr + 15) & 0xf8;
	} else {
		od->x_pos = *(ptr + 9) * 8;
		od->y_pos = ((*(ptr + 10)) & 0x7F) * 8;

		od->parentstate = (*(ptr + 10) & 0x80) ? 1 : 0;
		if (_game.version <= 2)
			od->parentstate *= 8;

		od->width = *(ptr + 11) * 8;

		od->parent = *(ptr + 12);

		if (_game.version <= 2) {
			od->walk_x = *(ptr + 13) * 8;
			od->walk_y = (*(ptr + 14) & 0x1f) * 8;
			od->actordir = (*(ptr + 15)) & 7;
			od->height = *(ptr + 15) & 0xf8;
		} else {
			od->walk_x = READ_LE_UINT16(ptr + 13);
			od->walk_y = READ_LE_UINT16(ptr + 15);
			od->actordir = (*(ptr + 17)) & 7;
			od->height = *(ptr + 17) & 0xf8;
		}
	}
}

void ScummEngine::resetRoomObject(ObjectData *od, const byte *room, const byte *searchptr) {
	const CodeHeader *cdhd = NULL;
	const ImageHeader *imhd = NULL;

	assert(room);

	if (searchptr == NULL) {
		if (_game.version == 8)
			searchptr = getResourceAddress(rtRoomScripts, _roomResource);
		else
			searchptr = room;
	}

	cdhd = (const CodeHeader *)findResourceData(MKTAG('C','D','H','D'), searchptr + od->OBCDoffset);
	if (cdhd == NULL)
		error("Room %d missing CDHD blocks(s)", _roomResource);
	if (od->OBIMoffset)
		imhd = (const ImageHeader *)findResourceData(MKTAG('I','M','H','D'), room + od->OBIMoffset);

	od->flags = Gdi::dbAllowMaskOr;

	if (_game.version == 8) {
		assert(imhd);
		od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));

		od->parent = cdhd->v7.parent;
		od->parentstate = cdhd->v7.parentstate;

		od->x_pos = (int)READ_LE_UINT32(&imhd->v8.x_pos);
		od->y_pos = (int)READ_LE_UINT32(&imhd->v8.y_pos);
		od->width = (uint)READ_LE_UINT32(&imhd->v8.width);
		od->height = (uint)READ_LE_UINT32(&imhd->v8.height);
		// HACK: This is done since an angle doesn't fit into a byte (360 > 256)
		od->actordir = toSimpleDir(1, READ_LE_UINT32(&imhd->v8.actordir));
		if (FROM_LE_32(imhd->v8.version) == 801)
			od->flags = ((((byte)READ_LE_UINT32(&imhd->v8.flags)) & 16) == 0) ? Gdi::dbAllowMaskOr : 0;

	} else if (_game.version == 7) {
		assert(imhd);
		od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));

		od->parent = cdhd->v7.parent;
		od->parentstate = cdhd->v7.parentstate;

		od->x_pos = READ_LE_UINT16(&imhd->v7.x_pos);
		od->y_pos = READ_LE_UINT16(&imhd->v7.y_pos);
		od->width = READ_LE_UINT16(&imhd->v7.width);
		od->height = READ_LE_UINT16(&imhd->v7.height);
		od->actordir = (byte)READ_LE_UINT16(&imhd->v7.actordir);

	} else if (_game.version == 6) {
		assert(imhd);
		od->obj_nr = READ_LE_UINT16(&(cdhd->v6.obj_id));

		od->width = READ_LE_UINT16(&cdhd->v6.w);
		od->height = READ_LE_UINT16(&cdhd->v6.h);
		od->x_pos = ((int16)READ_LE_UINT16(&cdhd->v6.x));
		od->y_pos = ((int16)READ_LE_UINT16(&cdhd->v6.y));
		if (cdhd->v6.flags == 0x80) {
			od->parentstate = 1;
		} else {
			od->parentstate = (cdhd->v6.flags & 0xF);
		}
		od->parent = cdhd->v6.parent;
		od->actordir = cdhd->v6.actordir;

		if (_game.heversion >= 60 && imhd)
			od->flags = ((imhd->old.flags & 1) != 0) ? Gdi::dbAllowMaskOr : 0;

	} else {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v5.obj_id));

		od->width = cdhd->v5.w * 8;
		od->height = cdhd->v5.h * 8;
		od->x_pos = cdhd->v5.x * 8;
		od->y_pos = cdhd->v5.y * 8;
		if (cdhd->v5.flags == 0x80) {
			od->parentstate = 1;
		} else {
			od->parentstate = (cdhd->v5.flags & 0xF);
		}
		od->parent = cdhd->v5.parent;
		od->walk_x = READ_LE_UINT16(&cdhd->v5.walk_x);
		od->walk_y = READ_LE_UINT16(&cdhd->v5.walk_y);
		od->actordir = cdhd->v5.actordir;
	}

	od->fl_object_index = 0;
}

void ScummEngine::updateObjectStates() {
	int i;
	ObjectData *od = &_objs[1];
	for (i = 1; i < _numLocalObjects; i++, od++) {
		// V0 MM, objects with type == 1 are room objects (room specific objects, non-pickup)
		if (_game.version == 0 && OBJECT_V0_TYPE(od->obj_nr) == kObjectV0TypeBG)
			continue;

		if (od->obj_nr > 0)
			od->state = getState(od->obj_nr);
	}
}

void ScummEngine::processDrawQue() {
	int i, j;
	for (i = 0; i < _drawObjectQueNr; i++) {
		j = _drawObjectQue[i];
		if (j)
			drawObject(j, 0);
	}
	_drawObjectQueNr = 0;
}

void ScummEngine::addObjectToDrawQue(int object) {
	if ((unsigned int)_drawObjectQueNr >= ARRAYSIZE(_drawObjectQue))
		error("Draw Object Que overflow");
	_drawObjectQue[_drawObjectQueNr++] = object;
}

void ScummEngine::removeObjectFromDrawQue(int object) {
	if (_drawObjectQueNr <= 0)
		return;

	int i;
	for (i = 0; i < _drawObjectQueNr; i++) {
		if (_drawObjectQue[i] == object)
			_drawObjectQue[i] = 0;
	}
}

void ScummEngine::clearDrawObjectQueue() {
	_drawObjectQueNr = 0;
}

void ScummEngine::clearDrawQueues() {
	clearDrawObjectQueue();
}

void ScummEngine_v6::clearDrawQueues() {
	ScummEngine::clearDrawQueues();

	_blastObjectQueuePos = 0;
}

#ifdef ENABLE_HE
void ScummEngine_v71he::clearDrawQueues() {
	ScummEngine_v6::clearDrawQueues();

	_wiz->polygonClear();
}

void ScummEngine_v80he::clearDrawQueues() {
	ScummEngine_v71he::clearDrawQueues();

	_wiz->clearWizBuffer();
}
#endif

/**
 * Mark the rectangle covered by the given object as dirty, thus eventually
 * ensuring a redraw of that area. This function is typically invoked when an
 * object gets removed from the current room, or when its state changed.
 */
void ScummEngine::markObjectRectAsDirty(int obj) {
	int i, strip;

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr == (uint16)obj) {
			if (_objs[i].width != 0) {
				const int minStrip = MAX(_screenStartStrip, _objs[i].x_pos / 8);
				const int maxStrip = MIN(_screenEndStrip+1, _objs[i].x_pos / 8 + _objs[i].width / 8);
				for (strip = minStrip; strip < maxStrip; strip++) {
					setGfxUsageBit(strip, USAGE_BIT_DIRTY);
				}
			}
			_bgNeedsRedraw = true;
			return;
		}
	}
}

const byte *ScummEngine::getObjOrActorName(int obj) {
	byte *objptr;
	int i;

	if (objIsActor(obj))
		return derefActor(objToActor(obj), "getObjOrActorName")->getActorName();

	for (i = 0; i < _numNewNames; i++) {
		if (_newNames[i] == obj) {
			debug(5, "Found new name for object %d at _newNames[%d]", obj, i);
			return getResourceAddress(rtObjectName, i);
		}
	}

	objptr = getOBCDFromObject(obj, true);
	if (objptr == NULL)
		return NULL;

	if (_game.features & GF_SMALL_HEADER) {
		byte offset = 0;

		if (_game.version == 0)
			offset = *(objptr + 13);
		else if (_game.version <= 2)
			offset = *(objptr + 14);
		else if (_game.features & GF_OLD_BUNDLE)
			offset = *(objptr + 16);
		else if (_game.id == GID_LOOM && _game.platform == Common::kPlatformPCEngine)
			offset = *(objptr + 16) + 17;
		else
			offset = *(objptr + 18);

		return (objptr + offset);
	}

	return findResourceData(MKTAG('O','B','N','A'), objptr);
}

void ScummEngine::setObjectName(int obj) {
	int i;

	if (objIsActor(obj))
		error("Can't set actor %d name with new-name-of", obj);

	for (i = 0; i < _numNewNames; i++) {
		if (_newNames[i] == obj) {
			_res->nukeResource(rtObjectName, i);
			_newNames[i] = 0;
			break;
		}
	}

	for (i = 0; i < _numNewNames; i++) {
		if (_newNames[i] == 0) {
			loadPtrToResource(rtObjectName, i, NULL);
			_newNames[i] = obj;
			runInventoryScript(0);
			return;
		}
	}

	error("New name of %d overflows name table (max = %d)", obj, _numNewNames);
}

uint32 ScummEngine::getOBCDOffs(int object) const {
	int i;

	if ((_game.version != 0 || OBJECT_V0_TYPE(object) == 0) &&
		_objectOwnerTable[object] != OF_OWNER_ROOM)
		return 0;

	for (i = (_numLocalObjects-1); i > 0; i--) {
		if (_objs[i].obj_nr == object) {
			if (_objs[i].fl_object_index != 0)
				return 8;
			return _objs[i].OBCDoffset;
		}
	}
	return 0;
}

byte *ScummEngine::getOBCDFromObject(int obj, bool v0CheckInventory) {
	int i;
	byte *ptr;

	if ((_game.version != 0 || OBJECT_V0_TYPE(obj) == 0) &&
		_objectOwnerTable[obj] != OF_OWNER_ROOM)
	{
		if (_game.version == 0 && !v0CheckInventory)
			return 0;
		for (i = 0; i < _numInventory; i++) {
			if (_inventory[i] == obj)
				return getResourceAddress(rtInventory, i);
		}
	} else {
		for (i = (_numLocalObjects-1); i > 0; --i) {
			if (_objs[i].obj_nr == obj) {
				if (_objs[i].fl_object_index) {
					assert(_objs[i].OBCDoffset == 8);
					ptr = getResourceAddress(rtFlObject, _objs[i].fl_object_index);
				} else if (_game.version == 8)
					ptr = getResourceAddress(rtRoomScripts, _roomResource);
				else
					ptr = getResourceAddress(rtRoom, _roomResource);
				assert(ptr);
				return ptr + _objs[i].OBCDoffset;
			}
		}
	}
	return 0;
}

const byte *ScummEngine::getOBIMFromObjectData(const ObjectData &od) {
	const byte *ptr;

	if (od.fl_object_index) {
		ptr = getResourceAddress(rtFlObject, od.fl_object_index);
		ptr = findResource(MKTAG('O','B','I','M'), ptr);
	} else {
		ptr = getResourceAddress(rtRoom, _roomResource);
		if (ptr)
			ptr += od.OBIMoffset;
	}
	return ptr;
}

static const uint32 IMxx_tags[] = {
	MKTAG('I','M','0','0'),
	MKTAG('I','M','0','1'),
	MKTAG('I','M','0','2'),
	MKTAG('I','M','0','3'),
	MKTAG('I','M','0','4'),
	MKTAG('I','M','0','5'),
	MKTAG('I','M','0','6'),
	MKTAG('I','M','0','7'),
	MKTAG('I','M','0','8'),
	MKTAG('I','M','0','9'),
	MKTAG('I','M','0','A'),
	MKTAG('I','M','0','B'),
	MKTAG('I','M','0','C'),
	MKTAG('I','M','0','D'),
	MKTAG('I','M','0','E'),
	MKTAG('I','M','0','F'),
	MKTAG('I','M','1','0')
};

const byte *ScummEngine::getObjectImage(const byte *ptr, int state) {
	assert(ptr);
	if (_game.features & GF_OLD_BUNDLE)
		ptr += 0;
	else if (_game.features & GF_SMALL_HEADER) {
		ptr += 8;
	} else if (_game.version == 8) {
		// The OBIM contains an IMAG, which in turn contains a WRAP, which contains
		// an OFFS chunk and multiple BOMP/SMAP chunks. To find the right BOMP/SMAP,
		// we use the offsets in the OFFS chunk,
		ptr = findResource(MKTAG('I','M','A','G'), ptr);
		if (!ptr)
			return 0;

		ptr = findResource(MKTAG('W','R','A','P'), ptr);
		if (!ptr)
			return 0;

		ptr = findResource(MKTAG('O','F','F','S'), ptr);
		if (!ptr)
			return 0;

		// Get the address of the specified SMAP (corresponding to IMxx)
		ptr += READ_LE_UINT32(ptr + 4 + 4*state);
	} else {
		ptr = findResource(IMxx_tags[state], ptr);
	}

	return ptr;
}

int ScummEngine::getObjectImageCount(int object) {
	const byte *ptr;
	const ImageHeader *imhd;
	int objnum;

	objnum = getObjectIndex(object);
	if (objnum == -1)
		return 0;

	ptr = getOBIMFromObjectData(_objs[objnum]);
	imhd = (const ImageHeader *)findResourceData(MKTAG('I','M','H','D'), ptr);
	if (!imhd)
		return 0;

	if (_game.version == 8) {
		return (READ_LE_UINT32(&imhd->v8.image_count));
	} else if (_game.version == 7) {
		return(READ_LE_UINT16(&imhd->v7.image_count));
	} else {
		return (READ_LE_UINT16(&imhd->old.image_count));
	}
}

#ifdef ENABLE_SCUMM_7_8
int ScummEngine_v8::getObjectIdFromOBIM(const byte *obim) {
	// In V8, IMHD has no obj_id, but rather a name string. We map the name
	// back to an object id using a table derived from the DOBJ resource.
	const ImageHeader *imhd = (const ImageHeader *)findResourceData(MKTAG('I','M','H','D'), obim);
	ObjectNameId *found = (ObjectNameId *)bsearch(imhd->v8.name, _objectIDMap, _objectIDMapSize,
					sizeof(ObjectNameId), (int (*)(const void*, const void*))strcmp);
	assert(found);
	return found->id;
}

int ScummEngine_v7::getObjectIdFromOBIM(const byte *obim) {
	const ImageHeader *imhd = (const ImageHeader *)findResourceData(MKTAG('I','M','H','D'), obim);
	return READ_LE_UINT16(&imhd->v7.obj_id);
}
#endif

int ScummEngine::getObjectIdFromOBIM(const byte *obim) {
	if (_game.features & GF_SMALL_HEADER)
		return READ_LE_UINT16(obim + 6);

	const ImageHeader *imhd = (const ImageHeader *)findResourceData(MKTAG('I','M','H','D'), obim);
	return READ_LE_UINT16(&imhd->old.obj_id);
}

void ScummEngine::findObjectInRoom(FindObjectInRoom *fo, byte findWhat, uint id, uint room) {

	const CodeHeader *cdhd;
	int i, numobj;
	const byte *roomptr, *obcdptr, *obimptr, *searchptr;
	int id2;
	int obim_id;

	id2 = getObjectIndex(id);
	if (findWhat & foCheckAlreadyLoaded && id2 != -1) {
		assert(_game.version >= 6);
		if (findWhat & foCodeHeader) {
			fo->obcd = obcdptr = getOBCDFromObject(id);
			assert(obcdptr);
			fo->cdhd = (const CodeHeader *)findResourceData(MKTAG('C','D','H','D'), obcdptr);
		}
		if (findWhat & foImageHeader) {
			fo->obim = obimptr = getOBIMFromObjectData(_objs[id2]);
			assert(obimptr);
		}
		return;
	}

	fo->roomptr = roomptr = getResourceAddress(rtRoom, room);
	if (!roomptr)
		error("findObjectInRoom: failed getting roomptr to %d", room);

	if (_game.features & GF_OLD_BUNDLE) {
		numobj = roomptr[20];
	} else {
		const RoomHeader *roomhdr = (const RoomHeader *)findResourceData(MKTAG('R','M','H','D'), roomptr);

		if (_game.version == 8)
			numobj = READ_LE_UINT32(&(roomhdr->v8.numObjects));
		else if (_game.version == 7)
			numobj = READ_LE_UINT16(&(roomhdr->v7.numObjects));
		else
			numobj = READ_LE_UINT16(&(roomhdr->old.numObjects));
	}

	if (numobj == 0)
		error("findObjectInRoom: No object found in room %d", room);
	if (numobj > _numLocalObjects)
		error("findObjectInRoom: More (%d) than %d objects in room %d", numobj, _numLocalObjects, room);

	if (_game.features & GF_OLD_BUNDLE) {
		if (_game.version <= 2)
			searchptr = roomptr + 28;
		else
			searchptr = roomptr + 29;

		for (i = 0; i < numobj; i++) {
			obimptr = roomptr + READ_LE_UINT16(searchptr);
			obcdptr = roomptr + READ_LE_UINT16(searchptr + 2 * numobj);
			id2 = READ_LE_UINT16(obcdptr + 4);

			if (id2 == (uint16)id) {
				if (findWhat & foCodeHeader) {
					fo->obcd = obcdptr;
					// We assume that the code header starts at a fixed offset.
					// A bit hackish, but works reasonably well.
					fo->cdhd = (const CodeHeader *)(obcdptr + 10);
				}
				if (findWhat & foImageHeader) {
					fo->obim = obimptr;
				}
				break;
			}
			searchptr += 2;
		}
		return;
	}

	if (findWhat & foCodeHeader) {
		if (_game.version == 8)
			searchptr = getResourceAddress(rtRoomScripts, room);
		else
			searchptr = roomptr;
		assert(searchptr);
		ResourceIterator	obcds(searchptr, (_game.features & GF_SMALL_HEADER) != 0);
		for (i = 0; i < numobj; i++) {
			obcdptr = obcds.findNext(MKTAG('O','B','C','D'));
			if (obcdptr == NULL)
				error("findObjectInRoom: Not enough code blocks in room %d", room);
			cdhd = (const CodeHeader *)findResourceData(MKTAG('C','D','H','D'), obcdptr);

			if (_game.features & GF_SMALL_HEADER)
				id2 = READ_LE_UINT16(obcdptr + 6);
			else if (_game.version >= 7)
				id2 = READ_LE_UINT16(&(cdhd->v7.obj_id));
			else if (_game.version == 6)
				id2 = READ_LE_UINT16(&(cdhd->v6.obj_id));
			else
				id2 = READ_LE_UINT16(&(cdhd->v5.obj_id));

			if (id2 == (uint16)id) {
				fo->obcd = obcdptr;
				fo->cdhd = cdhd;
				break;
			}
		}
		if (i == numobj)
			error("findObjectInRoom: Object %d not found in room %d", id, room);
	}

	roomptr = fo->roomptr;
	if (findWhat & foImageHeader) {
		ResourceIterator	obims(roomptr, (_game.features & GF_SMALL_HEADER) != 0);
		for (i = 0; i < numobj; i++) {
			obimptr = obims.findNext(MKTAG('O','B','I','M'));
			if (obimptr == NULL)
				error("findObjectInRoom: Not enough image blocks in room %d", room);
			obim_id = getObjectIdFromOBIM(obimptr);

			if (obim_id == (uint16)id) {
				fo->obim = obimptr;
				break;
			}
		}
		if (i == numobj)
			error("findObjectInRoom: Object %d image not found in room %d", id, room);
	}
}

bool ScummEngine_v0::objIsActor(int obj) {
	// object IDs < _numActors are used in v0 for objects too (e.g. hamster)
	return OBJECT_V0_TYPE(obj) == kObjectV0TypeActor;
}

int ScummEngine_v0::objToActor(int obj) {
	return OBJECT_V0_ID(obj);
}

int ScummEngine_v0::actorToObj(int actor) {
	return OBJECT_V0(actor, kObjectV0TypeActor);
}

bool ScummEngine::objIsActor(int obj) {
	return obj < _numActors;
}

int ScummEngine::objToActor(int obj) {
	return obj;
}

int ScummEngine::actorToObj(int actor) {
	return actor;
}

int ScummEngine::getObjX(int obj) {
	if (obj < 1)
		return 0;									/* fix for indy4's map */

	if (objIsActor(obj)) {
		return derefActor(objToActor(obj), "getObjX")->getRealPos().x;
	} else {
		if (whereIsObject(obj) == WIO_NOT_FOUND)
			return -1;
		int x, y;
		getObjectOrActorXY(obj, x, y);
		return x;
	}
}

int ScummEngine::getObjY(int obj) {
	if (obj < 1)
		return 0;									/* fix for indy4's map */

	if (objIsActor(obj)) {
		return derefActor(objToActor(obj), "getObjY")->getRealPos().y;
	} else {
		if (whereIsObject(obj) == WIO_NOT_FOUND)
			return -1;
		int x, y;
		getObjectOrActorXY(obj, x, y);
		return y;
	}
}

int ScummEngine::getObjOldDir(int obj) {
	return newDirToOldDir(getObjNewDir(obj));
}

int ScummEngine::getObjNewDir(int obj) {
	int dir;
	if (objIsActor(obj)) {
		dir = derefActor(objToActor(obj), "getObjNewDir")->getFacing();
	} else {
		int x, y;
		getObjectXYPos(obj, x, y, dir);
	}
	return dir;
}

void ScummEngine::setObjectState(int obj, int state, int x, int y) {
	int i;

	i = getObjectIndex(obj);
	if (i == -1) {
		debug(0, "setObjectState: no such object %d", obj);
		return;
	}

	if (x != -1 && x != 0x7FFFFFFF) {
		_objs[i].x_pos = x * 8;
		_objs[i].y_pos = y * 8;
	}

	addObjectToDrawQue(i);
	if (_game.version >= 7) {
		int imagecount;
		if (state == 0xFF) {
			state = getState(obj);
			imagecount = getObjectImageCount(obj);

			if (state < imagecount)
				state++;
			else
				state = 1;
		}

		if (state == 0xFE)
			state = _rnd.getRandomNumber(getObjectImageCount(obj));
	}
	putState(obj, state);
}

int ScummEngine_v6::getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f) {
	int i, j;
	int x, y;
	int x2, y2;

	j = i = 0xFF;

	if (is_obj_1) {
		if (getObjectOrActorXY(b, x, y) == -1)
			return -1;
		if (b < _numActors)
			i = derefActor(b, "getDistanceBetween_is_obj_1")->_scalex;
	} else {
		x = b;
		y = c;
	}

	if (is_obj_2) {
		if (getObjectOrActorXY(e, x2, y2) == -1)
			return -1;
		if (e < _numActors)
			j = derefActor(e, "getDistanceBetween_is_obj_2")->_scalex;
	} else {
		x2 = e;
		y2 = f;
	}

	return getDist(x, y, x2, y2) * 0xFF / ((i + j) / 2);
}

void ScummEngine::nukeFlObjects(int min, int max) {
	ObjectData *od;
	int i;

	debug(0, "nukeFlObjects(%d,%d)", min, max);

	for (i = (_numLocalObjects-1), od = _objs; --i >= 0; od++)
		if (od->fl_object_index && od->obj_nr >= min && od->obj_nr <= max) {
			_res->nukeResource(rtFlObject, od->fl_object_index);
			od->obj_nr = 0;
			od->fl_object_index = 0;
		}
}

void ScummEngine_v6::enqueueObject(int objectNumber, int objectX, int objectY, int objectWidth,
								int objectHeight, int scaleX, int scaleY, int image, int mode) {
	BlastObject *eo;

	if (_blastObjectQueuePos >= (int)ARRAYSIZE(_blastObjectQueue)) {
		error("enqueueObject: overflow");
	}

	int idx = getObjectIndex(objectNumber);
	assert(idx >= 0);

	eo = &_blastObjectQueue[_blastObjectQueuePos++];
	eo->number = objectNumber;
	eo->rect.left = objectX;
	eo->rect.top = objectY + _screenTop;
	if (objectWidth == 0) {
		eo->rect.right = eo->rect.left + _objs[idx].width;
	} else {
		eo->rect.right = eo->rect.left + objectWidth;
	}
	if (objectHeight == 0) {
		eo->rect.bottom = eo->rect.top + _objs[idx].height;
	} else {
		eo->rect.bottom = eo->rect.top + objectHeight;
	}

	eo->scaleX = scaleX;
	eo->scaleY = scaleY;
	eo->image = image;

	eo->mode = mode;
}

void ScummEngine_v6::drawBlastObjects() {
	BlastObject *eo;
	int i;

	eo = _blastObjectQueue;
	for (i = 0; i < _blastObjectQueuePos; i++, eo++) {
		drawBlastObject(eo);
	}
}

void ScummEngine_v6::drawBlastObject(BlastObject *eo) {
	VirtScreen *vs;
	const byte *bomp, *ptr;
	int objnum;
	BompDrawData bdd;

	vs = &_virtscr[kMainVirtScreen];

	assertRange(30, eo->number, _numGlobalObjects - 1, "blast object");

	objnum = getObjectIndex(eo->number);
	if (objnum == -1)
		error("drawBlastObject: getObjectIndex on BlastObject %d failed", eo->number);

	ptr = getOBIMFromObjectData(_objs[objnum]);
	if (!ptr)
		error("BlastObject object %d image not found", eo->number);

	const byte *img = getObjectImage(ptr, eo->image);
	if (_game.version == 8) {
		assert(img);
		bomp = img + 8;
	} else {
		if (!img)
			img = getObjectImage(ptr, 1);	// Backward compatibility with samnmax blast objects
		assert(img);
		bomp = findResourceData(MKTAG('B','O','M','P'), img);
	}

	if (!bomp)
		error("object %d is not a blast object", eo->number);

	bdd.dst = *vs;
	bdd.dst.pixels = vs->getPixels(0, 0);
	bdd.x = eo->rect.left;
	bdd.y = eo->rect.top;

	// Skip the bomp header
	if (_game.version == 8) {
		bdd.src = bomp + 8;
	} else {
		bdd.src = bomp + 10;
	}
	if (_game.version == 8) {
		bdd.srcwidth = READ_LE_UINT32(bomp);
		bdd.srcheight = READ_LE_UINT32(bomp+4);
	} else {
		bdd.srcwidth = READ_LE_UINT16(bomp+2);
		bdd.srcheight = READ_LE_UINT16(bomp+4);
	}

	bdd.scale_x = (byte)eo->scaleX;
	bdd.scale_y = (byte)eo->scaleY;

	bdd.maskPtr = NULL;
	bdd.numStrips = _gdi->_numStrips;

	if ((bdd.scale_x != 255) || (bdd.scale_y != 255)) {
		bdd.shadowMode = 0;
	} else {
		bdd.shadowMode = eo->mode;
	}
	bdd.shadowPalette = _shadowPalette;

	bdd.actorPalette = 0;
	bdd.mirror = false;

	drawBomp(bdd);

	markRectAsDirty(vs->number, bdd.x, bdd.x + bdd.srcwidth, bdd.y, bdd.y + bdd.srcheight);
}

void ScummEngine_v6::removeBlastObjects() {
	BlastObject *eo;
	int i;

	eo = _blastObjectQueue;
	for (i = 0; i < _blastObjectQueuePos; i++, eo++) {
		removeBlastObject(eo);
	}
	_blastObjectQueuePos = 0;
}

void ScummEngine_v6::removeBlastObject(BlastObject *eo) {
	VirtScreen *vs = &_virtscr[kMainVirtScreen];

	Common::Rect r;
	int left_strip, right_strip;
	int i;

	r = eo->rect;

	r.clip(Common::Rect(vs->w, vs->h));

	if (r.width() <= 0 || r.height() <= 0)
		return;

	left_strip = r.left / 8;
	right_strip = (r.right + (vs->xstart % 8)) / 8;

	if (left_strip < 0)
		left_strip = 0;
	if (right_strip > _gdi->_numStrips - 1)
		right_strip = _gdi->_numStrips - 1;
	for (i = left_strip; i <= right_strip; i++)
		_gdi->resetBackground(r.top, r.bottom, i);

	markRectAsDirty(kMainVirtScreen, r, USAGE_BIT_RESTORED);
}

int ScummEngine::findLocalObjectSlot() {
	int i;

	for (i = 1; i < _numLocalObjects; i++) {
		if (!_objs[i].obj_nr) {
			memset(&_objs[i], 0, sizeof(_objs[i]));
			return i;
		}
	}

	return -1;
}

int ScummEngine::findFlObjectSlot() {
	int i;
	for (i = 1; i < _numFlObject; i++) {
		if (_res->_types[rtFlObject][i]._address == NULL)
			return i;
	}
	error("findFlObjectSlot: Out of FLObject slots");
	return -1;
}

void ScummEngine_v70he::loadFlObject(uint object, uint room) {
	// Don't load an already stored object
	for (int i = 0; i < _numStoredFlObjects; i++) {
		if (_storedFlObjects[i].obj_nr == object)
			return;
	}

	ScummEngine_v60he::loadFlObject(object, room);
}

void ScummEngine::loadFlObject(uint object, uint room) {
	FindObjectInRoom foir;
	int slot, objslot;
	ObjectData *od;
	byte *flob;
	uint32 obcd_size, obim_size, flob_size;
	bool isRoomLocked, isRoomScriptsLocked;

	// Don't load an already loaded object
	if (getObjectIndex(object) != -1)
		return;

	// Locate the object in the room resource
	findObjectInRoom(&foir, foImageHeader | foCodeHeader, object, room);

	// Add an entry for the new floating object in the local object table
	objslot = findLocalObjectSlot();
	if (objslot == -1)
		error("loadFlObject: Local Object Table overflow");

	od = &_objs[objslot];

	// Dump object script
	if (_dumpScripts) {
		char buf[32];
		const byte *ptr = foir.obcd;
		sprintf(buf, "roomobj-%u-", room);
		ptr = findResource(MKTAG('V','E','R','B'), ptr);
		dumpResource(buf, object, ptr);
	}

	// Setup sizes
	obcd_size = READ_BE_UINT32(foir.obcd + 4);
	od->OBCDoffset = 8;
	od->OBIMoffset = obcd_size + 8;
	obim_size = READ_BE_UINT32(foir.obim + 4);
	flob_size = obcd_size + obim_size + 8;

	// Lock room/roomScripts for the given room. They contains the OBCD/OBIM
	// data, and a call to createResource might expire them, hence we lock them.
	isRoomLocked = _res->isLocked(rtRoom, room);
	isRoomScriptsLocked = _res->isLocked(rtRoomScripts, room);
	if (!isRoomLocked)
		_res->lock(rtRoom, room);
	if (_game.version == 8 && !isRoomScriptsLocked)
		_res->lock(rtRoomScripts, room);

	// Allocate slot & memory for floating object
	slot = findFlObjectSlot();
	flob = _res->createResource(rtFlObject, slot, flob_size);
	assert(flob);

	// Copy object code + object image to floating object
	WRITE_UINT32(flob, MKTAG('F','L','O','B'));
	WRITE_BE_UINT32(flob + 4, flob_size);
	memcpy(flob + 8, foir.obcd, obcd_size);
	memcpy(flob + 8 + obcd_size, foir.obim, obim_size);

	// Unlock room/roomScripts
	if (!isRoomLocked)
		_res->unlock(rtRoom, room);
	if (_game.version == 8 && !isRoomScriptsLocked)
		_res->unlock(rtRoomScripts, room);

	// Setup local object flags
	resetRoomObject(od, flob, flob);

	od->fl_object_index = slot;
}

} // End of namespace Scumm
