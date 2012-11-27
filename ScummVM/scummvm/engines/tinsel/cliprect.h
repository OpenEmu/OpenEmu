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
 * Clipping rectangle defines
 */

#ifndef TINSEL_CLIPRECT_H     // prevent multiple includes
#define TINSEL_CLIPRECT_H

#include "common/list.h"
#include "common/rect.h"

namespace Tinsel {

struct OBJECT;

typedef Common::List<Common::Rect> RectList;

/*----------------------------------------------------------------------*\
|*			Clip Rect Function Prototypes			*|
\*----------------------------------------------------------------------*/

void ResetClipRect();	// Resets the clipping rectangle allocator

void AddClipRect(		// Allocate a clipping rectangle from the free list
	const Common::Rect &pClip);		// clip rectangle dimensions to allocate

const RectList &GetClipRects();

bool IntersectRectangle(	// Creates the intersection of two rectangles
	Common::Rect &pDest,		// pointer to destination rectangle that is to receive the intersection
	const Common::Rect &pSrc1,		// pointer to a source rectangle
	const Common::Rect &pSrc2);		// pointer to a source rectangle

bool UnionRectangle(		// Creates the union of two rectangles
	Common::Rect &pDest,		// destination rectangle that is to receive the new union
	const Common::Rect &pSrc1,		// a source rectangle
	const Common::Rect &pSrc2);		// a source rectangle

void FindMovingObjects(		// Creates clipping rectangles for all the objects that have moved on the specified object list
	OBJECT **pObjList,	// playfield display list to draw
	Common::Point *pWin,		// playfield window top left position
	Common::Rect *pClip,		// playfield clipping rectangle
	bool bVelocity,		// when set, objects pos is updated with velocity
	bool bScrolled);	// when set, playfield has scrolled

void MergeClipRect();	// Merges any clipping rectangles that overlap

void UpdateClipRect(		// Redraws all objects within this clipping rectangle
	OBJECT **pObjList,	// object list to draw
	Common::Point *pWin,		// window top left position
	Common::Rect *pClip);		// pointer to clip rectangle

} // End of namespace Tinsel

#endif	// TINSEL_CLIPRECT_H
