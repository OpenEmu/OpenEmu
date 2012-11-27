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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Object Manager data structures
 */

#ifndef TINSEL_OBJECT_H	// prevent multiple includes
#define TINSEL_OBJECT_H

#include "tinsel/dw.h"
#include "common/frac.h"
#include "common/rect.h"

namespace Tinsel {

struct PALQ;

enum {
	/** the maximum number of objects */
	NUM_OBJECTS	= 256,

	// object flags
	DMA_WNZ		= 0x0001,	///< write non-zero data
	DMA_CNZ		= 0x0002,	///< write constant on non-zero data
	DMA_CONST	= 0x0004,	///< write constant on both zero & non-zero data
	DMA_WA		= 0x0008,	///< write all data
	DMA_FLIPH	= 0x0010,	///< flip object horizontally
	DMA_FLIPV	= 0x0020,	///< flip object vertically
	DMA_CLIP	= 0x0040,	///< clip object
	DMA_TRANS	= 0x0084,	///< translucent rectangle object
	DMA_ABS		= 0x0100,	///< position of object is absolute
	DMA_CHANGED	= 0x0200,	///< object has changed in some way since the last frame
	DMA_USERDEF	= 0x0400,	///< user defined flags start here
	DMA_GHOST	= 0x0080,


	/** flags that effect an objects appearance */
	DMA_HARDFLAGS	= (DMA_WNZ | DMA_CNZ | DMA_CONST | DMA_WA | DMA_FLIPH | DMA_FLIPV | DMA_TRANS)
};

/** structure for image */
#include "common/pack-start.h"	// START STRUCT PACKING
struct IMAGE {
	short imgWidth;		///< image width
	unsigned short imgHeight;	///< image height
	short anioffX;		///< image x animation offset
	short anioffY;		///< image y animation offset
	SCNHANDLE hImgBits;	///< image bitmap handle
	SCNHANDLE hImgPal;	///< image palette handle
} PACKED_STRUCT;
#include "common/pack-end.h"	// END STRUCT PACKING

/** a multi-object animation frame is a list of multi-image handles */
typedef uint32 FRAME;


// object structure
struct OBJECT {
	OBJECT *pNext;	///< pointer to next object in list
	OBJECT *pSlave;	///< pointer to slave object (multi-part objects)
//	char *pOnDispList;	///< pointer to display list byte for background objects
//	frac_t xVel;		///< x velocity of object
//	frac_t yVel;		///< y velocity of object
	frac_t xPos;		///< x position of object
	frac_t yPos;		///< y position of object
	int zPos;			///< z position of object
	Common::Rect rcPrev;		///< previous screen coordinates of object bounding rectangle
	int flags;			///< object flags - see above for list
	PALQ *pPal;			///< objects palette Q position
	int constant;		///< which color in palette for monochrome objects
	int width;			///< width of object
	int height;			///< height of object
	SCNHANDLE hBits;	///< image bitmap handle
	SCNHANDLE hImg;		///< handle to object image definition
	SCNHANDLE hShape;	///< objects current animation frame
	SCNHANDLE hMirror;	///< objects previous animation frame
	int oid;			///< object identifier
};
typedef OBJECT *POBJECT;

#include "common/pack-start.h"	// START STRUCT PACKING

// object initialisation structure
struct OBJ_INIT {
	SCNHANDLE hObjImg;	// objects shape - handle to IMAGE structure
	int32 objFlags;		// objects flags
	int32 objID;		// objects id
	int32 objX;		// objects initial x position
	int32 objY;		// objects initial y position
	int32 objZ;		// objects initial z position
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING


/*----------------------------------------------------------------------*\
|*			Object Function Prototypes			*|
\*----------------------------------------------------------------------*/

void KillAllObjects();	// kill all objects and place them on free list

void FreeObjectList();	// free the object list

#ifdef	DEBUG
void ObjectStats();		// Shows the maximum number of objects used at once
#endif

OBJECT *AllocObject();	// allocate a object from the free list

void FreeObject(		// place a object back on the free list
	OBJECT *pFreeObj);	// object to free

bool isValidObject(OBJECT *obj);

void CopyObject(		// copy one object to another
	OBJECT *pDest,		// destination object
	OBJECT *pSrc);		// source object

void InsertObject(		// insert a object onto a sorted object list
	OBJECT **pObjList,	// list to insert object onto
	OBJECT *pInsObj);	// object to insert

void DelObject(			// delete a object from a object list and add to free list
	OBJECT **pObjList,	// list to delete object from
	OBJECT *pDelObj);	// object to delete

void SortObjectList(		// re-sort an object list
	OBJECT **pObjList);	// list to sort

void GetAniOffset(	// returns the anim offsets of a image, takes into account orientation
	SCNHANDLE hImg,	// image to get animation offset of
	int flags,	// images current flags
	int *pAniX,	// gets set to new X animation offset
	int *pAniY);	// gets set to new Y animation offset

void GetAniPosition(	// Returns a objects x,y animation point
	OBJECT *pObj,	// pointer to object
	int *pPosX,	// gets set to objects X animation position
	int *pPosY);	// gets set to objects Y animation position

OBJECT *InitObject(		// Init a object using a OBJ_INIT struct
	const OBJ_INIT *pInitTbl);	// pointer to object initialisation table

void AnimateObjectFlags(	// Give a object a new image and new orientation flags
	OBJECT *pAniObj,	// object to be updated
	int newflags,		// objects new flags
	SCNHANDLE hNewImg);	// objects new image

void AnimateObject(		// give a object a new image
	OBJECT *pAniObj,	// object to animate
	SCNHANDLE hNewImg);	// objects new image

void HideObject(		// Hides a object by giving it a "NullImage" image pointer
	OBJECT *pObj);		// object to be hidden

OBJECT *RectangleObject(	// create a rectangle object of the given dimensions
	SCNHANDLE hPal,		// palette for the rectangle object
	int color,		// which color offset from the above palette
	int width,		// width of rectangle
	int height);		// height of rectangle

OBJECT *TranslucentObject(	// create a translucent rectangle object of the given dimensions
	int width,		// width of rectangle
	int height);		// height of rectangle

void ResizeRectangle(		// resizes a rectangle object
	OBJECT *pRect,		// rectangle object pointer
	int width,		// new width of rectangle
	int height);		// new height of rectangle


// FIXME: This does not belong here
struct FILM;
struct FREEL;
struct MULTI_INIT;
IMAGE *GetImageFromReel(const FREEL *pfreel, const MULTI_INIT **ppmi = 0);
IMAGE *GetImageFromFilm(SCNHANDLE hFilm, int reel, const FREEL **ppfr = 0,
					const MULTI_INIT **ppmi = 0, const FILM **ppfilm = 0);


} // End of namespace Tinsel

#endif	// TINSEL_OBJECT_H
