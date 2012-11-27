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
 * This file contains the clipping rectangle code.
 */

#include "tinsel/cliprect.h"	// object clip rect defs
#include "tinsel/graphics.h"	// normal object drawing
#include "tinsel/object.h"
#include "tinsel/palette.h"
#include "tinsel/tinsel.h"		// for _vm

namespace Tinsel {

/**
 * Resets the clipping rectangle allocator.
 */
void ResetClipRect() {
	_vm->_clipRects.clear();
}

/**
 * Allocate a clipping rectangle from the free list.
 * @param pClip			clip rectangle dimensions to allocate
 */
void AddClipRect(const Common::Rect &pClip) {
	_vm->_clipRects.push_back(pClip);
}

const RectList &GetClipRects() {
	return _vm->_clipRects;
}

/**
 * Creates the intersection of two rectangles.
 * Returns True if there is a intersection.
 * @param pDest			Pointer to destination rectangle that is to receive the intersection
 * @param pSrc1			Pointer to a source rectangle
 * @param pSrc2			Pointer to a source rectangle
 */
bool IntersectRectangle(Common::Rect &pDest, const Common::Rect &pSrc1, const Common::Rect &pSrc2) {
	pDest.left   = MAX(pSrc1.left, pSrc2.left);
	pDest.top    = MAX(pSrc1.top, pSrc2.top);
	pDest.right  = MIN(pSrc1.right, pSrc2.right);
	pDest.bottom = MIN(pSrc1.bottom, pSrc2.bottom);

	return !pDest.isEmpty();
}

/**
 * Creates the union of two rectangles.
 * Returns True if there is a union.
 * @param pDest			destination rectangle that is to receive the new union
 * @param pSrc1			a source rectangle
 * @param pSrc2			a source rectangle
 */
bool UnionRectangle(Common::Rect &pDest, const Common::Rect &pSrc1, const Common::Rect &pSrc2) {
	pDest.left   = MIN(pSrc1.left, pSrc2.left);
	pDest.top    = MIN(pSrc1.top, pSrc2.top);
	pDest.right  = MAX(pSrc1.right, pSrc2.right);
	pDest.bottom = MAX(pSrc1.bottom, pSrc2.bottom);

	return !pDest.isEmpty();
}

/**
 * Check if the two rectangles are next to each other.
 * @param pSrc1			a source rectangle
 * @param pSrc2			a source rectangle
 */
static bool LooseIntersectRectangle(const Common::Rect &pSrc1, const Common::Rect &pSrc2) {
	Common::Rect pDest;

	pDest.left   = MAX(pSrc1.left, pSrc2.left);
	pDest.top    = MAX(pSrc1.top, pSrc2.top);
	pDest.right  = MIN(pSrc1.right, pSrc2.right);
	pDest.bottom = MIN(pSrc1.bottom, pSrc2.bottom);

	return pDest.isValidRect();
}

/**
 * Adds velocities and creates clipping rectangles for all the
 * objects that have moved on the specified object list.
 * @param pObjList			Playfield display list to draw
 * @param pWin				Playfield window top left position
 * @param pClip				Playfield clipping rectangle
 * @param bNoVelocity		When reset, objects pos is updated with velocity
 * @param bScrolled)		When set, playfield has scrolled
 */
void FindMovingObjects(OBJECT **pObjList, Common::Point *pWin, Common::Rect *pClip, bool bNoVelocity, bool bScrolled) {
	OBJECT *pObj;			// object list traversal pointer

	for (pObj = *pObjList; pObj != NULL; pObj = pObj->pNext) {
		if (!bNoVelocity) {
			// we want to add velocities to objects position

			if (bScrolled) {
				// this playfield has scrolled

				// indicate change
				pObj->flags |= DMA_CHANGED;
			}
		}

		if ((pObj->flags & DMA_CHANGED) ||	// object changed
			HasPalMoved(pObj->pPal)) {	// or palette moved
			// object has changed in some way

			Common::Rect rcClip;	// objects clipped bounding rectangle
			Common::Rect rcObj;	// objects bounding rectangle

			// calc intersection of objects previous bounding rectangle
			// NOTE: previous position is in screen co-ords
			if (IntersectRectangle(rcClip, pObj->rcPrev, *pClip)) {
				// previous position is within clipping rect
				AddClipRect(rcClip);
			}

			// calc objects current bounding rectangle
			if (pObj->flags & DMA_ABS) {
				// object position is absolute
				rcObj.left = fracToInt(pObj->xPos);
				rcObj.top  = fracToInt(pObj->yPos);
			} else {
				// object position is relative to window
				rcObj.left = fracToInt(pObj->xPos) - pWin->x;
				rcObj.top  = fracToInt(pObj->yPos) - pWin->y;
			}
			rcObj.right  = rcObj.left + pObj->width;
			rcObj.bottom = rcObj.top  + pObj->height;

			// calc intersection of object with clipping rect
			if (IntersectRectangle(rcClip, rcObj, *pClip)) {
				// current position is within clipping rect
				AddClipRect(rcClip);

				// update previous position
				pObj->rcPrev = rcClip;
			} else {
				// clear previous position
				pObj->rcPrev = Common::Rect();
			}

			// clear changed flag
			pObj->flags &= ~DMA_CHANGED;
		}
	}
}

/**
 * Merges any clipping rectangles that overlap to try and reduce
 * the total number of clip rectangles.
 */
void MergeClipRect() {
	RectList &s_rectList = _vm->_clipRects;

	if (s_rectList.size() <= 1)
		return;

	RectList::iterator rOuter, rInner;

	for (rOuter = s_rectList.begin(); rOuter != s_rectList.end(); ++rOuter) {
		rInner = rOuter;
		while (++rInner != s_rectList.end()) {

			if (LooseIntersectRectangle(*rOuter, *rInner)) {
				// these two rectangles overlap or
				// are next to each other - merge them

				UnionRectangle(*rOuter, *rOuter, *rInner);

				// remove the inner rect from the list
				s_rectList.erase(rInner);

				// move back to beginning of list
				rInner = rOuter;
			}
		}
	}
}

/**
 * Redraws all objects within this clipping rectangle.
 * @param pObjList		Object list to draw
 * @param pWin			Window top left position
 * @param pClip			Pointer to clip rectangle
 */
void UpdateClipRect(OBJECT **pObjList, Common::Point *pWin, Common::Rect *pClip) {
	int x, y, right, bottom;	// object corners
	int hclip, vclip;			// total size of object clipping
	DRAWOBJECT currentObj;		// filled in to draw the current object in list
	OBJECT *pObj;				// object list iterator

	// Initialize the fields of the drawing object to empty
	memset(&currentObj, 0, sizeof(DRAWOBJECT));

	for (pObj = *pObjList; pObj != NULL; pObj = pObj->pNext) {
		if (pObj->flags & DMA_ABS) {
			// object position is absolute
			x = fracToInt(pObj->xPos);
			y = fracToInt(pObj->yPos);
		} else {
			// object position is relative to window
			x = fracToInt(pObj->xPos) - pWin->x;
			y = fracToInt(pObj->yPos) - pWin->y;
		}

		// calc object right
		right = x + pObj->width;
		if (right < 0)
			// totally clipped if negative
			continue;

		// calc object bottom
		bottom = y + pObj->height;
		if (bottom < 0)
			// totally clipped if negative
			continue;

		// bottom clip = low right y - clip low right y
		currentObj.botClip = bottom - pClip->bottom;
		if (currentObj.botClip < 0) {
			// negative - object is not clipped
			currentObj.botClip = 0;
		}

		// right clip = low right x - clip low right x
		currentObj.rightClip = right - pClip->right;
		if (currentObj.rightClip < 0) {
			// negative - object is not clipped
			currentObj.rightClip = 0;
		}

		// top clip = clip top left y - top left y
		currentObj.topClip = pClip->top - y;
		if (currentObj.topClip < 0) {
			// negative - object is not clipped
			currentObj.topClip = 0;
		} else {	// clipped - adjust start position to top of clip rect
			y = pClip->top;
		}

		// left clip = clip top left x - top left x
		currentObj.leftClip = pClip->left - x;
		if (currentObj.leftClip < 0) {
			// negative - object is not clipped
			currentObj.leftClip = 0;
		} else {
			// NOTE: This else statement is disabled in tinsel v1
			// clipped - adjust start position to left of clip rect
			x = pClip->left;
		}

		// calc object total horizontal clipping
		hclip = currentObj.leftClip + currentObj.rightClip;

		// calc object total vertical clipping
		vclip = currentObj.topClip + currentObj.botClip;

		if (hclip + vclip != 0) {
			// object is clipped in some way

			if (pObj->width <= hclip)
				// object totally clipped horizontally - ignore
				continue;

			if (pObj->height <= vclip)
				// object totally clipped vertically - ignore
				continue;

			// set clip bit in objects flags
			currentObj.flags = pObj->flags | DMA_CLIP;
		} else {	// object is not clipped - copy flags
			currentObj.flags = pObj->flags;
		}

		// copy objects properties to local object
		currentObj.width    = pObj->width;
		currentObj.height   = pObj->height;
		currentObj.xPos     = (short)x;
		currentObj.yPos     = (short)y;
		currentObj.pPal     = pObj->pPal;
		currentObj.constant = pObj->constant;
		currentObj.hBits    = pObj->hBits;

		// draw the object
		DrawObject(&currentObj);
	}
}

} // End of namespace Tinsel
