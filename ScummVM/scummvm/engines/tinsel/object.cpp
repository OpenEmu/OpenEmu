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
 * This file contains the Object Manager code.
 */

#include "tinsel/object.h"
#include "tinsel/background.h"
#include "tinsel/cliprect.h"	// object clip rect defs
#include "tinsel/graphics.h"	// low level interface
#include "tinsel/handle.h"
#include "tinsel/text.h"
#include "tinsel/tinsel.h"

#include "common/textconsole.h"

#define	OID_EFFECTS	0x2000			// generic special effects object id

namespace Tinsel {

// FIXME: Avoid non-const global vars

// list of all objects
static OBJECT *objectList = 0;

// pointer to free object list
static OBJECT *pFreeObjects = 0;

#ifdef DEBUG
// diagnostic object counters
static int numObj = 0;
static int maxObj = 0;
#endif

void FreeObjectList() {
	free(objectList);
	objectList = NULL;
}

/**
 * Kills all objects and places them on the free list.
 */

void KillAllObjects() {
	int i;

#ifdef DEBUG
	// clear number of objects in use
	numObj = 0;
#endif

	if (objectList == NULL) {
		// first time - allocate memory for object list
		objectList = (OBJECT *)calloc(NUM_OBJECTS, sizeof(OBJECT));

		// make sure memory allocated
		if (objectList == NULL) {
			error("Cannot allocate memory for object data");
		}
	}

	// place first object on free list
	pFreeObjects = objectList;

	// link all other objects after first
	for (i = 1; i < NUM_OBJECTS; i++) {
		objectList[i - 1].pNext = objectList + i;
	}

	// null the last object
	objectList[NUM_OBJECTS - 1].pNext = NULL;
}


#ifdef	DEBUG
/**
 * Shows the maximum number of objects used at once.
 */

void ObjectStats() {
	debug("%i objects of %i used", maxObj, NUM_OBJECTS);
}
#endif

/**
 * Allocate a object from the free list.
 */
OBJECT *AllocObject() {
	OBJECT *pObj = pFreeObjects;	// get a free object

	// check for no free objects
	assert(pObj != NULL);

	// a free object exists

	// get link to next free object
	pFreeObjects = pObj->pNext;

	// clear out object
	memset(pObj, 0, sizeof(OBJECT));

	// set default drawing mode and set changed bit
	pObj->flags = DMA_WNZ | DMA_CHANGED;

#ifdef DEBUG
	// one more object in use
	if (++numObj > maxObj)
		maxObj = numObj;
#endif

	// return new object
	return pObj;
}

bool isValidObject(OBJECT *obj) {
	return (obj >= objectList && obj <= objectList + NUM_OBJECTS - 1);
}

/**
 * Copy one object to another.
 * @param pDest			Destination object
 * @param pSrc			Source object
 */
void CopyObject(OBJECT *pDest, OBJECT *pSrc) {
	// save previous dimensions etc.
	Common::Rect rcSave = pDest->rcPrev;

	// make a copy
	memcpy(pDest, pSrc, sizeof(OBJECT));

	// restore previous dimensions etc.
	pDest->rcPrev = rcSave;

	// set changed flag in destination
	pDest->flags |= DMA_CHANGED;

	// null the links
	pDest->pNext = pDest->pSlave = NULL;
}

/**
 * Inserts an object onto the specified object list. The object
 * lists are sorted in Z Y order.
 * @param pObjList			List to insert object onto
 * @param pInsObj			Object to insert
 */

void InsertObject(OBJECT **pObjList, OBJECT *pInsObj) {
	OBJECT **pAnchor, *pObj;	// object list traversal pointers

	// validate object pointer
	assert(isValidObject(pInsObj));

	for (pAnchor = pObjList, pObj = *pAnchor; pObj != NULL; pAnchor = &pObj->pNext, pObj = *pAnchor) {
		// check Z order
		if (pInsObj->zPos < pObj->zPos) {
			// object Z is lower than list Z - insert here
			break;
		} else if (pInsObj->zPos == pObj->zPos) {
			// Z values are the same - sort on Y
			if (fracToDouble(pInsObj->yPos) <= fracToDouble(pObj->yPos)) {
				// object Y is lower than or same as list Y - insert here
				break;
			}
		}
	}

	// insert obj between pAnchor and pObj
	pInsObj->pNext = pObj;
	*pAnchor = pInsObj;
}


/**
 * Deletes an object from the specified object list and places it
 * on the free list.
 * @param pObjList			List to delete object from
 * @param pDelObj			Object to delete
 */
void DelObject(OBJECT **pObjList, OBJECT *pDelObj) {
	OBJECT **pAnchor, *pObj;	// object list traversal pointers
	const Common::Rect rcScreen(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// validate object pointer
	assert(isValidObject(pDelObj));

#ifdef DEBUG
	// one less object in use
	--numObj;
	assert(numObj >= 0);
#endif

	for (pAnchor = pObjList, pObj = *pAnchor; pObj != NULL; pAnchor = &pObj->pNext, pObj = *pAnchor) {
		if (pObj == pDelObj) {
			// found object to delete

			if (IntersectRectangle(pDelObj->rcPrev, pDelObj->rcPrev, rcScreen)) {
				// allocate a clipping rect for objects previous pos
				AddClipRect(pDelObj->rcPrev);
			}

			// make PREV next = OBJ next - removes OBJ from list
			*pAnchor = pObj->pNext;

			// place free list in OBJ next
			pObj->pNext = pFreeObjects;

			// add OBJ to top of free list
			pFreeObjects = pObj;

			// delete objects palette
			if (pObj->pPal)
				FreePalette(pObj->pPal);

			// quit
			return;
		}
	}

	// if we get to here - object has not been found on the list
	// This can be triggered in Act 3 in DW1 while talking to the guard,
	// so this has been turned to a warning instead of an error
	warning("DelObject(): formally 'assert(0)!'");
}


/**
 * Sort the specified object list in Z Y order.
 * @param pObjList			List to sort
 */
void SortObjectList(OBJECT **pObjList) {
	OBJECT *pPrev, *pObj;	// object list traversal pointers
	OBJECT head;		// temporary head of list - because pObjList is not usually a OBJECT

	// put at head of list
	head.pNext = *pObjList;

	// set head of list dummy OBJ Z Y values to lowest possible
	head.yPos = intToFrac(MIN_INT16);
	head.zPos = MIN_INT;

	for (pPrev = &head, pObj = head.pNext; pObj != NULL; pPrev = pObj, pObj = pObj->pNext) {
		// check Z order
		if (pObj->zPos < pPrev->zPos) {
			// object Z is lower than previous Z

			// remove object from list
			pPrev->pNext = pObj->pNext;

			// re-insert object on list
			InsertObject(pObjList, pObj);

			// back to beginning of list
			pPrev = &head;
			pObj  = head.pNext;
		} else if (pObj->zPos == pPrev->zPos) {
			// Z values are the same - sort on Y
			if (fracToDouble(pObj->yPos) < fracToDouble(pPrev->yPos)) {
				// object Y is lower than previous Y

				// remove object from list
				pPrev->pNext = pObj->pNext;

				// re-insert object on list
				InsertObject(pObjList, pObj);

				// back to beginning of list
				pPrev = &head;
				pObj  = head.pNext;
			}
		}
	}
}

/**
 * Returns the animation offsets of a image, dependent on the
 * images orientation flags.
 * @param hImg			Iimage to get animation offset of
 * @param flags			Images current flags
 * @param pAniX			Gets set to new X animation offset
 * @param pAniY			Gets set to new Y animation offset
 */
void GetAniOffset(SCNHANDLE hImg, int flags, int *pAniX, int *pAniY) {
	if (hImg) {
		const IMAGE *pImg = (const IMAGE *)LockMem(hImg);

		// set ani X
		*pAniX = (int16) FROM_LE_16(pImg->anioffX);

		// set ani Y
		*pAniY = (int16) FROM_LE_16(pImg->anioffY);

		if (flags & DMA_FLIPH) {
			// we are flipped horizontally

			// set ani X = -ani X + width - 1
			*pAniX = -*pAniX + FROM_LE_16(pImg->imgWidth) - 1;
		}

		if (flags & DMA_FLIPV) {
			// we are flipped vertically

			// set ani Y = -ani Y + height - 1
			*pAniY = -*pAniY + (FROM_LE_16(pImg->imgHeight) & ~C16_FLAG_MASK) - 1;
		}
	} else
		// null image
		*pAniX = *pAniY = 0;
}


/**
 * Returns the x,y position of an objects animation point.
 * @param pObj			Pointer to object
 * @param pPosX			Gets set to objects X animation position
 * @param pPosY			Gets set to objects Y animation position
 */
void GetAniPosition(OBJECT *pObj, int *pPosX, int *pPosY) {
	// validate object pointer
	assert(isValidObject(pObj));

	// get the animation offset of the object
	GetAniOffset(pObj->hImg, pObj->flags, pPosX, pPosY);

	// from animation offset and objects position - determine objects animation point
	*pPosX += fracToInt(pObj->xPos);
	*pPosY += fracToInt(pObj->yPos);
}

/**
 * Initialize a object using a OBJ_INIT structure to supply parameters.
 * @param pInitTbl			Pointer to object initialisation table
 */
OBJECT *InitObject(const OBJ_INIT *pInitTbl) {
	// allocate a new object
	OBJECT *pObj = AllocObject();

	// make sure object created
	assert(pObj != NULL);

	// set objects shape
	pObj->hImg = pInitTbl->hObjImg;

	// set objects ID
	pObj->oid = pInitTbl->objID;

	// set objects flags
	pObj->flags = DMA_CHANGED | pInitTbl->objFlags;

	// set objects Z position
	pObj->zPos = pInitTbl->objZ;

	// get pointer to image
	if (pInitTbl->hObjImg) {
		int aniX, aniY;		// objects animation offsets
		PALQ *pPalQ = NULL;	// palette queue pointer
		const IMAGE *pImg = (const IMAGE *)LockMem(pInitTbl->hObjImg);	// handle to image

		if (pImg->hImgPal) {
			// allocate a palette for this object
			pPalQ = AllocPalette(FROM_LE_32(pImg->hImgPal));

			// make sure palette allocated
			assert(pPalQ != NULL);
		}

		// assign palette to object
		pObj->pPal = pPalQ;

		// set objects size
		pObj->width  = FROM_LE_16(pImg->imgWidth);
		pObj->height = FROM_LE_16(pImg->imgHeight) & ~C16_FLAG_MASK;
		pObj->flags &= ~C16_FLAG_MASK;
		pObj->flags |= FROM_LE_16(pImg->imgHeight) & C16_FLAG_MASK;

		// set objects bitmap definition
		pObj->hBits = FROM_LE_32(pImg->hImgBits);

		// get animation offset of object
		GetAniOffset(pObj->hImg, pInitTbl->objFlags, &aniX, &aniY);

		// set objects X position - subtract ani offset
		pObj->xPos = intToFrac(pInitTbl->objX - aniX);

		// set objects Y position - subtract ani offset
		pObj->yPos = intToFrac(pInitTbl->objY - aniY);
	} else {	// no image handle - null image

		// set objects X position
		pObj->xPos = intToFrac(pInitTbl->objX);

		// set objects Y position
		pObj->yPos = intToFrac(pInitTbl->objY);
	}

	// return new object
	return pObj;
}

/**
 * Give a object a new image and new orientation flags.
 * @param pAniObj			Object to be updated
 * @param newflags			Objects new flags
 * @param hNewImg			Objects new image
 */
void AnimateObjectFlags(OBJECT *pAniObj, int newflags, SCNHANDLE hNewImg) {
	// validate object pointer
	assert(isValidObject(pAniObj));

	if (pAniObj->hImg != hNewImg
		|| (pAniObj->flags & DMA_HARDFLAGS) != (newflags & DMA_HARDFLAGS)) {
		// something has changed

		int oldAniX, oldAniY;	// objects old animation offsets
		int newAniX, newAniY;	// objects new animation offsets

		// get objects old animation offsets
		GetAniOffset(pAniObj->hImg, pAniObj->flags, &oldAniX, &oldAniY);

		// get objects new animation offsets
		GetAniOffset(hNewImg, newflags, &newAniX, &newAniY);

		if (hNewImg) {
			// get pointer to image
			const IMAGE *pNewImg = (IMAGE *)LockMem(hNewImg);

			// setup new shape
			pAniObj->width  = FROM_LE_16(pNewImg->imgWidth);
			pAniObj->height = FROM_LE_16(pNewImg->imgHeight) & ~C16_FLAG_MASK;
			newflags &= ~C16_FLAG_MASK;
			newflags |= FROM_LE_16(pNewImg->imgHeight) & C16_FLAG_MASK;

			// set objects bitmap definition
			pAniObj->hBits  = FROM_LE_32(pNewImg->hImgBits);
		} else {	// null image
			pAniObj->width  = 0;
			pAniObj->height = 0;
			pAniObj->hBits  = 0;
		}

		// set objects flags and signal a change
		pAniObj->flags = newflags | DMA_CHANGED;

		// set objects image
		pAniObj->hImg = hNewImg;

		// adjust objects position - subtract new from old for difference
		pAniObj->xPos += intToFrac(oldAniX - newAniX);
		pAniObj->yPos += intToFrac(oldAniY - newAniY);
	}
}

/**
 * Give an object a new image.
 * @param pAniObj			Object to animate
 * @param hNewImg			Objects new image
 */
void AnimateObject(OBJECT *pAniObj, SCNHANDLE hNewImg) {
	// dont change the objects flags
	AnimateObjectFlags(pAniObj, pAniObj->flags, hNewImg);
}

/**
 * Creates a rectangle object of the given dimensions and returns
 * a pointer to the object.
 * @param hPal			Palette for the rectangle object
 * @param color		Which color offset from the above palette
 * @param width			Width of rectangle
 * @param height		Height of rectangle
 */
OBJECT *RectangleObject(SCNHANDLE hPal, int color, int width, int height) {
	// template for initializing the rectangle object
	static const OBJ_INIT rectObj = {0, DMA_CONST, OID_EFFECTS, 0, 0, 0};
	PALQ *pPalQ;		// palette queue pointer

	// allocate and init a new object
	OBJECT *pRect = InitObject(&rectObj);

	// allocate a palette for this object
	pPalQ = AllocPalette(hPal);

	// make sure palette allocated
	assert(pPalQ != NULL);

	// assign palette to object
	pRect->pPal = pPalQ;

	// set color in the palette
	pRect->constant = color;

	// set rectangle width
	pRect->width = width;

	// set rectangle height
	pRect->height = height;

	// return pointer to rectangle object
	return pRect;
}

/**
 * Creates a translucent rectangle object of the given dimensions
 * and returns a pointer to the object.
 * @param width			Width of rectangle
 * @param height		Height of rectangle
 */
OBJECT *TranslucentObject(int width, int height) {
	// template for initializing the rectangle object
	static const OBJ_INIT rectObj = {0, DMA_TRANS, OID_EFFECTS, 0, 0, 0};

	// allocate and init a new object
	OBJECT *pRect = InitObject(&rectObj);

	// set rectangle width
	pRect->width = width;

	// set rectangle height
	pRect->height = height;

	// return pointer to rectangle object
	return pRect;
}

} // End of namespace Tinsel
