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
 * Multi-part object definitions
 */

#ifndef TINSEL_MULTIOBJ_H     // prevent multiple includes
#define TINSEL_MULTIOBJ_H

#include "tinsel/dw.h"
#include "tinsel/object.h"

namespace Tinsel {

struct OBJECT;

#include "common/pack-start.h"	// START STRUCT PACKING

/**
 * multi-object initialisation structure (parallels OBJ_INIT struct)
 */
struct MULTI_INIT {
	SCNHANDLE hMulFrame;	///< multi-objects shape - NULL terminated list of IMAGE structures
	int32 mulFlags;			///< multi-objects flags
	int32 mulID;			///< multi-objects id
	int32 mulX;				///< multi-objects initial x ani position
	int32 mulY;				///< multi-objects initial y ani position
	int32 mulZ;				///< multi-objects initial z position
	uint32 otherFlags;		///< multi-objects Tinsel 2 - other flags
} PACKED_STRUCT;
typedef MULTI_INIT *PMULTI_INIT;

#include "common/pack-end.h"	// END STRUCT PACKING

/*----------------------------------------------------------------------*\
|*			Multi Object Function Prototypes		*|
\*----------------------------------------------------------------------*/

OBJECT *MultiInitObject(	// Initialize a multi-part object
	const MULTI_INIT *pInitTbl);	// pointer to multi-object initialisation table

void MultiInsertObject(		// Insert a multi-part object onto a object list
	OBJECT **pObjList,	// list to insert multi-part object onto
	OBJECT *pInsObj);	// head of multi-part object to insert

void MultiDeleteObject(		// Delete all the pieces of a multi-part object
	OBJECT **pObjList,	// list to delete multi-part object from
	OBJECT *pMultiObj);	// multi-part object to be deleted

void MultiHideObject(		// Hide a multi-part object
	OBJECT *pMultiObj);	// multi-part object to be hidden

void MultiHorizontalFlip(	// Hortizontally flip a multi-part object
	OBJECT *pFlipObj);	// head of multi-part object to flip

void MultiVerticalFlip(		// Vertically flip a multi-part object
	OBJECT *pFlipObj);	// head of multi-part object to flip

void MultiAdjustXY(		// Adjust coords of a multi-part object. Takes into account the orientation
	OBJECT *pMultiObj,	// multi-part object to be adjusted
	int deltaX,		// x adjustment
	int deltaY);		// y adjustment

void MultiMoveRelXY(		// Move multi-part object relative. Does not take into account the orientation
	OBJECT *pMultiObj,	// multi-part object to be moved
	int deltaX,		// x movement
	int deltaY);		// y movement

void MultiSetAniXY(		// Set the x & y anim position of a multi-part object
	OBJECT *pMultiObj,	// multi-part object whose position is to be changed
	int newAniX,		// new x animation position
	int newAniY);		// new y animation position

void MultiSetAniX(		// Set the x anim position of a multi-part object
	OBJECT *pMultiObj,	// multi-part object whose x position is to be changed
	int newAniX);		// new x animation position

void MultiSetAniY(		// Set the y anim position of a multi-part object
	OBJECT *pMultiObj,	// multi-part object whose y position is to be adjusted
	int newAniY);		// new y animation position

void MultiSetZPosition(		// Sets the z position of a multi-part object
	OBJECT *pMultiObj,	// multi-part object to be adjusted
	int newZ);		// new Z order

void MultiMatchAniPoints(	// Matches a multi-parts pos and orientation to be the same as a reference object
	OBJECT *pMoveObj,	// multi-part object to be moved
	OBJECT *pRefObj);	// multi-part object to match with

void MultiReshape(		// Reshape a multi-part object
	OBJECT *pMultiObj);	// multi-part object to re-shape

int MultiLeftmost(		// Returns the left-most point of a multi-part object
	OBJECT *pMulti);	// multi-part object

int MultiRightmost(		// Returns the right-most point of a multi-part object
	OBJECT *pMulti);	// multi-part object

int MultiHighest(		// Returns the highest point of a multi-part object
	OBJECT *pMulti);	// multi-part object

int MultiLowest(		// Returns the lowest point of a multi-part object
	OBJECT *pMulti);	// multi-part object

bool MultiHasShape(		// Returns TRUE if the object currently has an image
	POBJECT pMulti);	// multi-part object

void MultiForceRedraw(
	POBJECT pMultiObj);	// multi-part object to be forced

} // End of namespace Tinsel

#endif	// TINSEL_MULTIOBJ_H
