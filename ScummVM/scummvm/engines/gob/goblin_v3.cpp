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

#include "gob/gob.h"
#include "gob/goblin.h"
#include "gob/mult.h"

namespace Gob {

Goblin_v3::Goblin_v3(GobEngine *vm) : Goblin_v2(vm) {
}

bool Goblin_v3::isMovement(int8 state) {
	if ((state >= 0) && (state < 8))
		return true;
	if ((state >= 40) && (state < 44))
		return true;
	if ((state == 26) || (state == 27))
		return true;
	if ((state == 38) || (state == 39))
		return true;

	return false;
}

void Goblin_v3::advMovement(Mult::Mult_Object *obj, int8 state) {
	switch (state) {
	case 0:
		obj->goblinX--;
		break;

	case 1:
		obj->goblinX--;
		obj->goblinY--;
		break;

	case 2:
	case 26:
	case 38:
		obj->goblinY--;
		break;

	case 3:
		obj->goblinX++;
		obj->goblinY--;
		break;

	case 4:
		obj->goblinX++;
		break;

	case 5:
		obj->goblinX++;
		obj->goblinY++;
		break;

	case 6:
	case 27:
	case 39:
		obj->goblinY++;
		break;

	case 7:
		obj->goblinX--;
		obj->goblinY++;
		break;

	case 40:
		obj->goblinX--;
		obj->goblinY -= 2;
		break;

	case 41:
		obj->goblinX--;
		obj->goblinY += 2;
		break;

	case 42:
		obj->goblinX++;
		obj->goblinY -= 2;
		break;

	case 43:
		obj->goblinX++;
		obj->goblinY += 2;
		break;
	}
}

void Goblin_v3::placeObject(Gob_Object *objDesc, char animated,
		int16 index, int16 x, int16 y, int16 state) {

	Mult::Mult_Object &obj = _vm->_mult->_objects[index];
	Mult::Mult_AnimData &objAnim = *(obj.pAnimData);

	if (!obj.goblinStates)
		return;

	if ((state != -1) && (obj.goblinStates[state] != 0)) {
		if (state == 8)
			objAnim.curLookDir = 0;
		else if (state == 9)
			objAnim.curLookDir = 4;
		else if (state == 28)
			objAnim.curLookDir = 6;
		else if (state == 29)
			objAnim.curLookDir = 2;
	}

	Goblin_v2::placeObject(objDesc, animated, index, x, y, state);

}

} // End of namespace Gob
