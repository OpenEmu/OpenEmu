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

#ifndef SWORD1_COLLISION_H
#define SWORD1_COLLISION_H

/*#include "objectman.h"

namespace Sword1 {

class Logic;

class Collision {
public:
	Collision(ObjectMan *pObjMan, Logic *pLogic);
	~Collision();
	void checkCollisions();
	void fnBumpOff();
	void fnBumpOn();
private:
	int32 getIntersect(int32 x0, int32 y0, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3);
	int noCol;
	ObjectMan *_objMan;
	Logic *_logic; // for CFN_preset_script
};

} // End of namespace Sword1
*/
// maybe it's better to make this part of Router

#endif // BSCOLLISION_H
