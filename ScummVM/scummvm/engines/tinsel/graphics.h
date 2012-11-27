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
 * Low level graphics interface.
 */

#ifndef TINSEL_GRAPHICS_H		// prevent multiple includes
#define TINSEL_GRAPHICS_H

#include "tinsel/dw.h"

#include "common/rect.h"
#include "common/system.h"
#include "graphics/surface.h"

namespace Tinsel {

struct PALQ;

/** draw object structure - only used when drawing objects */
struct DRAWOBJECT {
	char *charBase;		// character set base address
	int transOffset;	// transparent character offset
	int flags;		// object flags - see above for list
	PALQ *pPal;		// objects palette Q position
	int constant;		// which color in palette for monochrome objects
	int width;		// width of object
	int height;		// height of object
	SCNHANDLE hBits;	// image bitmap handle
	int lineoffset;		// offset to next line
	int leftClip;		// amount to clip off object left
	int rightClip;		// amount to clip off object right
	int topClip;		// amount to clip off object top
	int botClip;		// amount to clip off object bottom
	short xPos;			// x position of object
	short yPos;			// y position of object
	uint32 baseCol;		// For 4-bit stuff
};


/*----------------------------------------------------------------------*\
|*			    Function Prototypes				*|
\*----------------------------------------------------------------------*/

void ClearScreen();
void DrawObject(DRAWOBJECT *pObj);

// called to update a rectangle on the video screen from a video page
void UpdateScreenRect(const Common::Rect &pClip);

} // End of namespace Tinsel

#endif
