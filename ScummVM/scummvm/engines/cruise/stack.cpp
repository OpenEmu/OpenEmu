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

#include "cruise/cruise_main.h"

namespace Cruise {

// 4 type bigger than the old one, but much safer/cleaner
stackElementStruct scriptStack[SIZE_STACK];

// VAR

void pushVar(int16 var) {
	if (positionInStack < SIZE_STACK) {
		scriptStack[positionInStack].data.shortVar = var;
		scriptStack[positionInStack].type = STACK_SHORT;
		positionInStack++;
	}
}

int16 popVar() {
	if (positionInStack <= 0) {
		return (0);
	}

	positionInStack--;

	ASSERT(scriptStack[positionInStack].type == STACK_SHORT);

	return (scriptStack[positionInStack].data.shortVar);
}

//// PTR

void pushPtr(void *ptr) {
	if (positionInStack < SIZE_STACK) {
		scriptStack[positionInStack].data.ptrVar = ptr;
		scriptStack[positionInStack].type = STACK_PTR;
		positionInStack++;
	}
}

void *popPtr() {
	if (positionInStack <= 0) {
		return (0);
	}

	positionInStack--;

	ASSERT(scriptStack[positionInStack].type == STACK_PTR);

	return (scriptStack[positionInStack].data.ptrVar);
}

} // End of namespace Cruise
