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

#ifndef CRUISE_ACTOR_H
#define CRUISE_ACTOR_H

namespace Cruise {

enum animPhase {
	ANIM_PHASE_WAIT = 0,
	ANIM_PHASE_STATIC = 1,
	ANIM_PHASE_MOVE = 2,
	ANIM_PHASE_STATIC_END = 3,
	ANIM_PHASE_END = 4
};

enum ATP {
	ATP_MOUSE = 0,
	ATP_TRACK = 1
};

struct actorStruct {
	struct actorStruct *next;
	struct actorStruct *prev;

	int16 idx;
	int16 type;
	int16 overlayNumber;
	int16 x_dest;
	int16 y_dest;
	int16 x;
	int16 y;
	int16 startDirection;
	int16 nextDirection;
	int16 endDirection;
	int16 stepX;
	int16 stepY;
	int16 pathId;
	animPhase phase;
	int16 counter;
	int16 poly;
	int16 flag;
	int16 start;
	int16 freeze;
};

bool isAnimFinished(int overlayIdx, int idx, actorStruct *pStartEntry, int objType);
actorStruct *findActor(actorStruct *pStartEntry, int overlayIdx, int objIdx, int type);
void processAnimation();
void getPixel(int x, int y);

} // End of namespace Cruise

#endif
