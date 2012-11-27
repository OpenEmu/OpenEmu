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

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

int AGOSEngine_Elvira2::canPlace(Item *x, Item *y) {
	Item *z = derefItem(x->parent);

	SubObject *o = (SubObject *)findChildOfType(y, kObjectType);
	int ct;
	int cap = 0;

	if (o == NULL)
		return 0;	/* Fits Fine */

	xPlace(x,NULL);		/* Avoid disturbing figures */
	if (o)
		cap = sizeContents(y);

	xPlace(x, z);
	if ((o) && (o->objectFlags & kOFVolume)) {
		ct = getOffsetOfChild2Param(o, kOFVolume);
		cap = o->objectFlagValue[ct] - cap;
		cap -= sizeOfRec(x, 0);	/* - size of item going in */
		if (cap < 0)
			return -1;	/* Too big to fit */
	}

	return 0;
}

int AGOSEngine::canPlace(Item *x, Item *y) {
	Item *z = derefItem(x->parent);

	SubPlayer *p = (SubPlayer *)findChildOfType(y, kPlayerType);
	SubContainer *c = (SubContainer *)findChildOfType(y, kContainerType);
	int cap = 0;
	int wt;

	if ((c == NULL) && (p == NULL))
		return 0;		/* Fits Fine */

	xPlace(x, NULL);		/* Avoid disturbing figures */
	if (c)
		cap = sizeContents(y);

	wt = weightOf(y);
	xPlace(x, z);
	if (c) {
		cap = c->volume - cap;
		cap -= sizeOfRec(x, 0);	/* - size of item going in */
		if (cap < 0)
			return -1;	/* Too big to fit */
	}
	if (p) {
		if (wt + weightOf(x) > p->strength * 10)
			return -2;	/* Too heavy */
	}

	return 0;
}

void AGOSEngine::xPlace(Item *x, Item *y) {
	if (derefItem(x->parent))
		unlinkItem(x);

	linkItem(x, y);
}

int AGOSEngine::contains(Item *a, Item *b) {
	while (derefItem(b->parent)) {
		if (derefItem(b->parent) == a)
			return 1;
		b = derefItem(b->parent);
	}

	return 0;
}

int AGOSEngine::sizeContents(Item *x) {
	return sizeRec(x, 0);
}

int AGOSEngine::sizeRec(Item *x, int d) {
	Item *o;
	int n = 0;

	o = derefItem(x->child);

	if (d > 32)
		return(0);
	while (o) {
		n += sizeOfRec(o,d);
		o = derefItem(o->child);
	}

	return n;
}

int AGOSEngine_Elvira2::sizeOfRec(Item *i, int d) {
	SubObject *o = (SubObject *)findChildOfType(i, kObjectType);

	int ct;
	if ((o) && (o->objectFlags & kOFSoft)) {
		if (o->objectFlags & kOFSize) {
			ct = getOffsetOfChild2Param(o, kOFSize);
			return o->objectFlagValue[ct] + sizeRec(i, d + 1);
		}
		return sizeRec(i, d + 1);
	}
	if ((o) && (o->objectFlags & kOFSize)) {
		ct = getOffsetOfChild2Param(o, kOFSize);
		return o->objectFlagValue[ct];
	}

	return 0;
}

int AGOSEngine::sizeOfRec(Item *i, int d) {
	SubObject *o = (SubObject *)findChildOfType(i, kObjectType);

	SubPlayer *p = (SubPlayer *)findChildOfType(i, kPlayerType);
	SubContainer *c = (SubContainer *)findChildOfType(i, kContainerType);

	if ((c) && (c->flags & 1)) {
		if (o)
			return (o->objectSize + sizeRec(i, d + 1));
		if (p)
			return (p->size + sizeRec(i, d + 1));
		return (sizeRec(i, d + 1));
	}
	if (o)
		return (o->objectWeight);
	if (p)
		return (p->weight);

	return 0;
}

int AGOSEngine::weighUp(Item *x) {
	return weightRec(x, 0);
}

int AGOSEngine::weightRec(Item *x, int d) {
	int n = weightOf(x);
	Item *o;

	if (d > 32)
		return 0;
	o = derefItem(x->child);
	while (o) {
		n += weightRec(o, d + 1);
		o = derefItem(o->next);
	}

	return n;
}

int AGOSEngine_Elvira2::weightOf(Item *x) {
	SubObject *o = (SubObject *)findChildOfType(x, kObjectType);

	if ((o) && (o->objectFlags & kOFWeight)) {
		int ct = getOffsetOfChild2Param(o, kOFWeight);
		return (o->objectFlagValue[ct]);
	}

	return 0;
}

int AGOSEngine::weightOf(Item *x) {
	SubObject *o = (SubObject *)findChildOfType(x, kObjectType);

	SubPlayer *p = (SubPlayer *)findChildOfType(x, kPlayerType);
	if (o)
		return o->objectWeight;
	if (p)
		return p->weight;

	return 0;
}

} // End of namespace AGOS
