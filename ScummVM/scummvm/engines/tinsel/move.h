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

#ifndef TINSEL_MOVE_H	// prevent multiple includes
#define TINSEL_MOVE_H

#include "tinsel/dw.h"	// for SCNHANDLE
#include "tinsel/rince.h"

namespace Tinsel {

struct MOVER;

typedef enum { YB_X2, YB_X1_5 } YBIAS;

int SetActorDest(MOVER *pMover, int x, int y, bool igPath, SCNHANDLE film);
void SSetActorDest(MOVER *pActor);
void DoMoveActor(MOVER *pMover);

void SetDefaultRefer(int32 defRefer);
int GetLastLeadXdest();
int GetLastLeadYdest();

DIRECTION GetDirection(int fromx, int fromy, int tox, int toy, DIRECTION lastreel,
							  HPOLYGON hPath, YBIAS yBias = YB_X2);
} // End of namespace Tinsel

#endif /* TINSEL_MOVE_H */
