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
 * This file contains utilities to handle multi-part objects.
 */

#include "tinsel/multiobj.h"
#include "tinsel/handle.h"
#include "tinsel/object.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

/**
 * Initialize a multi-part object using a list of images to init
 * each object piece. One object is created for each image in the list.
 * All objects are given the same palette as the first image. A pointer
 * to the first (master) object created is returned.
 * @param pInitTbl			Pointer to multi-object initialisation table
 */
OBJECT *MultiInitObject(const MULTI_INIT *pInitTbl) {
	OBJ_INIT obj_init;	// object init table
	OBJECT *pFirst, *pObj;	// object pointers
	FRAME *pFrame;		// list of images for the multi-part object

	if (FROM_LE_32(pInitTbl->hMulFrame)) {
		// we have a frame handle
		pFrame = (FRAME *)LockMem(FROM_LE_32(pInitTbl->hMulFrame));

		obj_init.hObjImg  = READ_LE_UINT32(pFrame);	// first objects shape
	} else {	// this must be a animation list for a NULL object
		pFrame = NULL;
		obj_init.hObjImg = 0;	// first objects shape
	}

	// init the object init table
	obj_init.objFlags = (int)FROM_LE_32(pInitTbl->mulFlags);	// all objects have same flags
	obj_init.objID    = (int)FROM_LE_32(pInitTbl->mulID);	// all objects have same ID
	obj_init.objX     = (int)FROM_LE_32(pInitTbl->mulX);	// all objects have same X ani pos
	obj_init.objY     = (int)FROM_LE_32(pInitTbl->mulY);	// all objects have same Y ani pos
	obj_init.objZ     = (int)FROM_LE_32(pInitTbl->mulZ);	// all objects have same Z pos

	// create and init the first object
	pObj = pFirst = InitObject(&obj_init);

	if (pFrame) {
		// if we have any animation frames

		pFrame++;

		while (READ_LE_UINT32(pFrame) != 0) {
			// set next objects shape
			obj_init.hObjImg = READ_LE_UINT32(pFrame);

			// create next object and link to previous
			pObj = pObj->pSlave = InitObject(&obj_init);

			pFrame++;
		}
	}

	// null end of list for final object
	pObj->pSlave = NULL;

	// return master object
	return pFirst;
}

/**
 * Inserts the multi-part object onto the specified object list.
 * @param pObjList			List to insert multi-part object onto
* @param  pInsObj			Head of multi-part object to insert

 */

void MultiInsertObject(OBJECT **pObjList, OBJECT *pInsObj) {
	// validate object pointer
	assert(isValidObject(pInsObj));

	// for all the objects that make up this multi-part
	do {
		// add next part to the specified list
		InsertObject(pObjList, pInsObj);

		// next obj in list
		pInsObj = pInsObj->pSlave;
	} while (pInsObj != NULL);
}

/**
 * Deletes all the pieces of a multi-part object from the
 * specified object list.
 * @param pObjList			List to delete multi-part object from
 * @param pMultiObj			Multi-part object to be deleted
 */

void MultiDeleteObject(OBJECT **pObjList, OBJECT *pMultiObj) {
	// validate object pointer
	assert(isValidObject(pMultiObj));

	// for all the objects that make up this multi-part
	do {
		// delete object
		DelObject(pObjList, pMultiObj);

		// next obj in list
		pMultiObj = pMultiObj->pSlave;
	}
	while (pMultiObj != NULL);
}

/**
 * Hides a multi-part object by giving each object a "NullImage"
 * image pointer.
 * @param pMultiObj			Multi-part object to be hidden
 */

void MultiHideObject(OBJECT *pMultiObj) {
	// validate object pointer
	assert(isValidObject(pMultiObj));

	// set master shape to null animation frame
	pMultiObj->hShape = 0;

	// change all objects
	MultiReshape(pMultiObj);
}

/**
 * Horizontally flip a multi-part object.
 * @param pFlipObj			Head of multi-part object to flip
 */

void MultiHorizontalFlip(OBJECT *pFlipObj) {
	// validate object pointer
	assert(isValidObject(pFlipObj));

	// for all the objects that make up this multi-part
	do {
		// horizontally flip the next part
		AnimateObjectFlags(pFlipObj, pFlipObj->flags ^ DMA_FLIPH,
			pFlipObj->hImg);

		// next obj in list
		pFlipObj = pFlipObj->pSlave;
	} while (pFlipObj != NULL);
}

/**
 * Vertically flip a multi-part object.
 * @param pFlipObj			Head of multi-part object to flip
 */

void MultiVerticalFlip(OBJECT *pFlipObj) {
	// validate object pointer
	assert(isValidObject(pFlipObj));

	// for all the objects that make up this multi-part
	do {
		// vertically flip the next part
		AnimateObjectFlags(pFlipObj, pFlipObj->flags ^ DMA_FLIPV,
			pFlipObj->hImg);

		// next obj in list
		pFlipObj = pFlipObj->pSlave;
	}
	while (pFlipObj != NULL);
}

/**
 * Adjusts the coordinates of a multi-part object.	The adjustments
 * take into account the orientation of the object.
 * @param pMultiObj			Multi-part object to be adjusted
 * @param deltaX			X adjustment
 * @param deltaY			Y adjustment
 */

void MultiAdjustXY(OBJECT *pMultiObj, int deltaX, int deltaY) {
	// validate object pointer
	assert(isValidObject(pMultiObj));

	if (deltaX == 0 && deltaY == 0)
		return;		// ignore no change

	if (!TinselV2) {
		// *** This may be wrong!!!
		if (pMultiObj->flags & DMA_FLIPH) {
			// image is flipped horizontally - flip the x direction
			deltaX = -deltaX;
		}

		if (pMultiObj->flags & DMA_FLIPV) {
			// image is flipped vertically - flip the y direction
			deltaY = -deltaY;
		}
	}

	// for all the objects that make up this multi-part
	do {
		// signal a change in the object
		pMultiObj->flags |= DMA_CHANGED;

		// adjust the x position
		pMultiObj->xPos += intToFrac(deltaX);

		// adjust the y position
		pMultiObj->yPos += intToFrac(deltaY);

		// next obj in list
		pMultiObj = pMultiObj->pSlave;

	} while (pMultiObj != NULL);
}

/**
 * Moves all the pieces of a multi-part object by the specified
 * amount. Does not take into account the objects orientation.
 * @param pMultiObj			Multi-part object to be adjusted
 * @param deltaX			X movement
 * @param deltaY			Y movement
 */

void MultiMoveRelXY(OBJECT *pMultiObj, int deltaX, int deltaY) {
	// validate object pointer
	assert(isValidObject(pMultiObj));

	if (deltaX == 0 && deltaY == 0)
		return;		// ignore no change

	// for all the objects that make up this multi-part
	do {
		// signal a change in the object
		pMultiObj->flags |= DMA_CHANGED;

		// adjust the x position
		pMultiObj->xPos += intToFrac(deltaX);

		// adjust the y position
		pMultiObj->yPos += intToFrac(deltaY);

		// next obj in list
		pMultiObj = pMultiObj->pSlave;

	} while (pMultiObj != NULL);
}

/**
 * Sets the x & y anim position of all pieces of a multi-part object.
 * @param pMultiObj			Multi-part object whose position is to be changed
 * @param newAniX			New x animation position
 * @param newAniY			New y animation position
 */

void MultiSetAniXY(OBJECT *pMultiObj, int newAniX, int newAniY) {
	int curAniX, curAniY;	// objects current animation position

	// validate object pointer
	assert(isValidObject(pMultiObj));

	// get master objects current animation position
	GetAniPosition(pMultiObj, &curAniX, &curAniY);

	// calc difference between current and new positions
	newAniX -= curAniX;
	newAniY -= curAniY;

	// move all pieces by the difference
	MultiMoveRelXY(pMultiObj, newAniX, newAniY);
}

/**
 * Sets the x anim position of all pieces of a multi-part object.
 * @param pMultiObj			Multi-part object whose x position is to be changed
 * @param newAniX			New x animation position
 */

void MultiSetAniX(OBJECT *pMultiObj, int newAniX) {
	int curAniX, curAniY;	// objects current animation position

	// validate object pointer
	assert(isValidObject(pMultiObj));

	// get master objects current animation position
	GetAniPosition(pMultiObj, &curAniX, &curAniY);

	// calc x difference between current and new positions
	newAniX -= curAniX;
	curAniY = 0;

	// move all pieces by the difference
	MultiMoveRelXY(pMultiObj, newAniX, curAniY);
}

/**
 * Sets the y anim position of all pieces of a multi-part object.
 * @param pMultiObj			Multi-part object whose x position is to be changed
 * @param newAniX			New y animation position
 */

void MultiSetAniY(OBJECT *pMultiObj, int newAniY) {
	int curAniX, curAniY;	// objects current animation position

	// validate object pointer
	assert(isValidObject(pMultiObj));

	// get master objects current animation position
	GetAniPosition(pMultiObj, &curAniX, &curAniY);

	// calc y difference between current and new positions
	curAniX = 0;
	newAniY -= curAniY;

	// move all pieces by the difference
	MultiMoveRelXY(pMultiObj, curAniX, newAniY);
}

/**
 * Sets the Z position of all pieces of a multi-part object.
 * @param pMultiObj		Multi-part object to be adjusted
 * @param newZ			New Z order
 */

void MultiSetZPosition(OBJECT *pMultiObj, int newZ) {
	// validate object pointer
	assert(isValidObject(pMultiObj));

	// for all the objects that make up this multi-part
	do {
		// signal a change in the object
		pMultiObj->flags |= DMA_CHANGED;

		// set the new z position
		pMultiObj->zPos = newZ;

		// next obj in list
		pMultiObj = pMultiObj->pSlave;
	}
	while (pMultiObj != NULL);
}

/**
 * Reshape a multi-part object.
 * @param pMultiObj			Multi-part object to re-shape
 */

void MultiReshape(OBJECT *pMultiObj) {
	SCNHANDLE hFrame;

	// validate object pointer
	assert(isValidObject(pMultiObj));

	// get objects current anim frame
	hFrame = pMultiObj->hShape;

	if (hFrame != 0 && hFrame != pMultiObj->hMirror) {
		// a valid shape frame which is different from previous

		// get pointer to frame
		const FRAME *pFrame = (const FRAME *)LockMem(hFrame);

		// update previous
		pMultiObj->hMirror = hFrame;

		while (READ_LE_UINT32(pFrame) != 0 && pMultiObj != NULL) {
			// a normal image - update the current object with this image
			AnimateObject(pMultiObj, READ_LE_UINT32(pFrame));

			// move to next image for this frame
			pFrame++;

			// move to next part of object
			pMultiObj = pMultiObj->pSlave;
		}

		// null the remaining object parts
		while (pMultiObj != NULL) {
			// set a null image for this object part
			AnimateObject(pMultiObj, 0);

			// move to next part of object
			pMultiObj = pMultiObj->pSlave;
		}
	} else if (hFrame == 0) {
		// update previous
		pMultiObj->hMirror = hFrame;

		// null all the object parts
		while (pMultiObj != NULL) {
			// set a null image for this object part
			AnimateObject(pMultiObj, 0);

			// move to next part of object
			pMultiObj = pMultiObj->pSlave;
		}
	}
}

/**
 * Returns the left-most point of a multi-part object.
 * @param pMulti			Multi-part object
 */

int MultiLeftmost(OBJECT *pMulti) {
	int left;

	// validate object pointer
	assert(isValidObject(pMulti));

	// init leftmost point to first object
	left = fracToInt(pMulti->xPos);

	// for all the objects in this multi
	while ((pMulti = pMulti->pSlave) != NULL) {
		if (pMulti->hImg != 0) {
			// non null object part

			if (fracToInt(pMulti->xPos) < left)
				// this object is further left
				left = fracToInt(pMulti->xPos);
		}
	}

	// return left-most point
	return left;
}

/**
 * Returns the right-most point of a multi-part object.
 * @param pMulti			Multi-part object
 */

int MultiRightmost(OBJECT *pMulti) {
	int right;

	// validate object pointer
	assert(isValidObject(pMulti));

	// init right-most point to first object
	right = fracToInt(pMulti->xPos) + pMulti->width;

	// for all the objects in this multi
	while ((pMulti = pMulti->pSlave) != NULL) {
		if (pMulti->hImg != 0) {
			// non null object part

			if (fracToInt(pMulti->xPos) + pMulti->width > right)
				// this object is further right
				right = fracToInt(pMulti->xPos) + pMulti->width;
		}
	}

	// return right-most point
	return right - 1;
}

/**
 * Returns the highest point of a multi-part object.
 * @param pMulti			Multi-part object
 */

int MultiHighest(OBJECT *pMulti) {
	int highest;

	// validate object pointer
	assert(isValidObject(pMulti));

	// init highest point to first object
	highest = fracToInt(pMulti->yPos);

	// for all the objects in this multi
	while ((pMulti = pMulti->pSlave) != NULL) {
		if (pMulti->hImg != 0) {
			// non null object part

			if (fracToInt(pMulti->yPos) < highest)
				// this object is higher
				highest = fracToInt(pMulti->yPos);
		}
	}

	// return highest point
	return highest;
}

/**
 * Returns the lowest point of a multi-part object.
 * @param pMulti			Multi-part object
 */

int MultiLowest(OBJECT *pMulti) {
	int lowest;

	// validate object pointer
	assert(isValidObject(pMulti));

	// init lowest point to first object
	lowest = fracToInt(pMulti->yPos) + pMulti->height;

	// for all the objects in this multi
	while ((pMulti = pMulti->pSlave) != NULL) {
		if (pMulti->hImg != 0) {
			// non null object part

			if (fracToInt(pMulti->yPos) + pMulti->height > lowest)
				// this object is lower
				lowest = fracToInt(pMulti->yPos) + pMulti->height;
		}
	}

	// return lowest point
	return lowest - 1;
}

/**
 * Returns TRUE if the object currently has an image.
 * @param pMulti		Multi-part object
 */

bool MultiHasShape(POBJECT pMulti) {
	return (pMulti->hShape != 0);
}

/**
 * Bodge for text on movies. Makes sure it appears for it's lifetime.
 * @param pMultiObj			Multi-part object to be adjusted
 */

void MultiForceRedraw(POBJECT pMultiObj) {
	// validate object pointer
	assert(isValidObject(pMultiObj));

	// for all the objects that make up this multi-part
	do {
		// signal a change in the object
		pMultiObj->flags |= DMA_CHANGED;

		// next obj in list
		pMultiObj = pMultiObj->pSlave;
	} while (pMultiObj != NULL);
}

} // End of namespace Tinsel
