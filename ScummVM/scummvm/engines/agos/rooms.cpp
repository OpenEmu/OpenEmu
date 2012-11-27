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



#include "common/file.h"
#include "common/textconsole.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

uint16 AGOSEngine::getBackExit(int n) {
	switch (n) {
	case 0:
		return 2;
	case 1:
		return 3;
	case 2:
		return 0;
	case 3:
		return 1;
	case 4:
		return 5;
	case 5:
		return 4;
	}

	return 0;
}

uint16 AGOSEngine::getDoorState(Item *item, uint16 d) {
	uint16 mask = 3;
	uint16 n;

	SubRoom *subRoom = (SubRoom *)findChildOfType(item, kRoomType);
	if (subRoom == NULL)
		return 0;

	d <<= 1;
	mask <<= d;
	n = subRoom->roomExitStates & mask;
	n >>= d;

	return n;
}

uint16 AGOSEngine::getExitOf(Item *item, uint16 d) {
	SubRoom *subRoom;
	uint16 x;
	uint16 y = 0;

	subRoom = (SubRoom *)findChildOfType(item, kRoomType);
	if (subRoom == NULL)
		return 0;
	x = d;
	while (x > y) {
		if (getDoorState(item, y) == 0)
			d--;
		y++;
	}
	return subRoom->roomExit[d];
}

void AGOSEngine::changeDoorState(SubRoom *r, uint16 d, uint16 n) {
	uint16 mask=3;
	d <<= 1;
	mask <<= d;
	n <<= d;
	r->roomExitStates &= ~mask;
	r->roomExitStates |= n;
}

void AGOSEngine::setDoorState(Item *i, uint16 d, uint16 n) {
	Item *j;
	SubRoom *r, *r1;
	uint16 d1;
	uint16 y = 0;

	r = (SubRoom *)findChildOfType(i, kRoomType);
	if (r == NULL)
		return;
	d1 = d;
	while (d > y) {
		if (getDoorState(i, y) == 0)
			d1--;
		y++;
	}
	changeDoorState(r, d, n);

	j = derefItem(r->roomExit[d1]);
	if (j == NULL)
		return;
	r1 = (SubRoom *)findChildOfType(j, kRoomType);
	if (r1 == NULL)
		return;
	d = getBackExit(d);
	d1 = d;
	y = 0;
	while (d > y) {
		if (getDoorState(j, y) == 0)
			d1--;
		y++;
	}
	/* Check are a complete exit pair */
	if (derefItem(r1->roomExit[d1]) != i)
		return;
	/* Change state of exit coming back */
	changeDoorState(r1, d, n);
}

// Elvira 1 specific
Item *AGOSEngine::getDoorOf(Item *i, uint16 d) {
	SubGenExit *g;
	Item *x;

	g = (SubGenExit *)findChildOfType(i, kGenExitType);
	if (g == NULL)
		return 0;

	x = derefItem(g->dest[d]);
	if (x == NULL)
		return 0;
	if (isRoom(x))
		return 0;
	return x;
}

Item *AGOSEngine::getExitOf_e1(Item *item, uint16 d) {
	SubGenExit *g;
	Item *x;

	g = (SubGenExit *)findChildOfType(item, kGenExitType);
	if (g == NULL)
		return 0;

	x = derefItem(g->dest[d]);
	if (x == NULL)
		return 0;
	if (isRoom(x))
		return x;
	if (x->state != 0)
		return 0;
	return derefItem(x->parent);
}

void AGOSEngine_Waxworks::moveDirn(Item *i, uint x) {
	Item *d;
	uint16 n;

	if (i->parent == 0)
		return;

	n = getExitOf(derefItem(i->parent), x);
	if (derefItem(n) == NULL) {
		loadRoomItems(n);
		n = getExitOf(derefItem(i->parent), x);
	}

	d = derefItem(n);
	if (d) {
		n = getDoorState(derefItem(i->parent), x);
		if (n == 1) {
			if (!canPlace(i, d))
				setItemParent(i, d);
		}
	}
}

void AGOSEngine_Elvira2::moveDirn(Item *i, uint x) {
	SubSuperRoom *sr;
	Item *d, *p;
	uint16 a, n;

	if (i->parent == 0)
		return;

	p = derefItem(i->parent);
	if (findChildOfType(p, kSuperRoomType)) {
		n = getExitState(p, _superRoomNumber,x);
		if (n == 1) {
			sr = (SubSuperRoom *)findChildOfType(p, kSuperRoomType);
			switch (x) {
			case 0: a = -(sr->roomX); break;
			case 1: a = 1; break;
			case 2: a = sr->roomX; break;
			case 3: a = 0xFFFF; break;
			case 4: a = -(sr->roomX * sr->roomY); break;
			case 5: a = (sr->roomX * sr->roomY); break;
			default: return;
			}
			_superRoomNumber += a;
		}
		return;
	}

	n = getExitOf(derefItem(i->parent), x);

	d = derefItem(n);
	if (d) {
		n = getDoorState(derefItem(i->parent), x);
		if (n == 1) {
			if (!canPlace(i, d))
				setItemParent(i, d);
		}
	}
}

void AGOSEngine::moveDirn(Item *i, uint x) {
	Item *d, *p;

	p = derefItem(i->parent);
	if (p == 0)
		return;


	d = getExitOf_e1(p, x);
	if (d) {
		if (canPlace(i, d))
			return;

		setItemParent(i, d);
		return;
	}

	d = getDoorOf(p, x);
	if (d) {
		const byte *name = getStringPtrByID(d->itemName, true);
		if (d->state == 1)
			showMessageFormat("%s is closed.\n", name);
		else
			showMessageFormat("%s is locked.\n", name);
		return;
	}

	showMessageFormat("You can't go that way.\n");
}

// Elvira 2 specific
int AGOSEngine_Elvira2::changeExitStates(SubSuperRoom *sr, int n, int d, uint16 s) {
	int b, bd;
	uint16 mask;

	switch (d) {
	case 0:
		b = -(sr->roomX);
		bd = 2;
		if (((n % (sr->roomX * sr->roomY)) / sr->roomX) == 0)
			return 0;
		else
			break;
	case 1:
		b = 1;
		bd = 3;
		if (((n % (sr->roomX * sr->roomY)) % sr->roomX) == 0)
			return 0;
		else
			break;
	case 2:
		b = sr->roomX;
		bd = 0;
		if (((n % (sr->roomX * sr->roomY)) / sr->roomX) == (sr->roomY - 1))
			return 0;
		else
			break;
	case 3:
		b = -1;
		bd = 1;
		if (((n % (sr->roomX * sr->roomY)) % sr->roomX) == 1)
			return 0;
		else
			break;
	case 4:
		b = -(sr->roomX * sr->roomY);
		bd = 5;
		if (n < (sr->roomX * sr->roomY))
			return 0;
		else
			break;
	case 5:
		b = sr->roomX * sr->roomY;
		bd = 4;
		if (n > (sr->roomX * sr->roomY * (sr->roomZ - 1)))
			return 0;
		else
			break;
	default:
		return 0;
	}

	n--;
	d <<= 1;
	mask = (3 << d);
	sr->roomExitStates[n] &= ~mask;
	sr->roomExitStates[n] |= (s << d);

	bd <<= 1;
	mask = (3 << bd);
	sr->roomExitStates[n + b] &= ~mask;
	sr->roomExitStates[n + b] |= (s << bd);
	return 1;
}

uint16 AGOSEngine_Elvira2::getExitState(Item *i, uint16 x, uint16 d) {
	SubSuperRoom *sr;
	uint16 mask = 3;
	uint16 n;

	sr = (SubSuperRoom *)findChildOfType(i, kSuperRoomType);
	if (sr == NULL)
		return 0;

	d <<= 1;
	mask <<= d;
	n = sr->roomExitStates[x - 1] & mask;
	n >>= d;
	return n;
}

void AGOSEngine_Elvira2::setExitState(Item *i, uint16 n, uint16 d, uint16 s) {
	SubSuperRoom *sr = (SubSuperRoom *)findChildOfType(i, kSuperRoomType);
	if (sr)
		changeExitStates(sr, n, d, s);
}

void AGOSEngine_Elvira2::setSRExit(Item *i, int n, int d, uint16 s) {
	uint16 mask = 3;

	SubSuperRoom *sr = (SubSuperRoom *)findChildOfType(i, kSuperRoomType);
	if (sr) {
		n--;
		d <<= 1;
		mask <<= d;
		s <<= d;
		sr->roomExitStates[n] &= ~mask;
		sr->roomExitStates[n] |= s;
	}
}

// Waxworks specific
bool AGOSEngine::loadRoomItems(uint16 room) {
	byte *p;
	uint i, minNum, maxNum;
	char filename[30];
	Common::File in;
	Item *item, *itemTmp;

	if (_roomsList == NULL)
		return 0;

	_currentRoom = room;
	room -= 2;

	if (_roomsListPtr) {
		p = _roomsListPtr;
		for (;;) {
			minNum = READ_BE_UINT16(p); p += 2;
			if (minNum == 0)
				break;

			maxNum = READ_BE_UINT16(p); p += 2;

			 for (uint16 z = minNum; z <= maxNum; z++) {
				uint16 itemNum = z + 2;
				item = derefItem(itemNum);
				item->parent = 0;

				uint16 num = (itemNum - _itemArrayInited);
				_roomStates[num].state = item->state;
				_roomStates[num].classFlags = item->classFlags;
				SubRoom *subRoom = (SubRoom *)findChildOfType(item, kRoomType);
				_roomStates[num].roomExitStates = subRoom->roomExitStates;
			}
		}
	}

	p = _roomsList;
	while (*p) {
		for (i = 0; *p; p++, i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		_roomsListPtr = p;

		for (;;) {
			minNum = READ_BE_UINT16(p); p += 2;
			if (minNum == 0)
				break;

			maxNum = READ_BE_UINT16(p); p += 2;

			if (room >= minNum && room <= maxNum) {
				in.open(filename);
				if (in.isOpen() == false) {
					error("loadRoomItems: Can't load rooms file '%s'", filename);
				}

				while ((i = in.readUint16BE()) != 0) {
					uint16 itemNum = i + 2;

					_itemArrayPtr[itemNum] = (Item *)allocateItem(sizeof(Item));
					readItemFromGamePc(&in, _itemArrayPtr[itemNum]);

					item = derefItem(itemNum);
					item->parent = 0;
					item->child = 0;

					 for (uint16 z = _itemArrayInited; z; z--) {
						itemTmp = derefItem(z);

						if (!itemTmp)
							continue;
						if (itemTmp->parent != itemNum)
							continue;
						if (item->child == 0) {
							item->child = z;
							continue;
						}
						uint16 child = item->child;
						while (itemTmp->next != 0) {
							if (itemTmp->next == child) {
								item->child = z;
								break;
							}

							itemTmp = derefItem(itemTmp->next);
						}
					}

					uint16 num = (itemNum - _itemArrayInited);
					item->state = _roomStates[num].state;
					item->classFlags = _roomStates[num].classFlags;
					SubRoom *subRoom = (SubRoom *)findChildOfType(item, kRoomType);
					subRoom->roomExitStates = _roomStates[num].roomExitStates;
				}
				in.close();

				return 1;
			}
		}
	}

	debug(1,"loadRoomItems: didn't find %d", room);
	return 0;
}

} // End of namespace AGOS
