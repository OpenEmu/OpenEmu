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
 * Text utility defines
 */

#ifndef TINSEL_TEXT_H     // prevent multiple includes
#define TINSEL_TEXT_H

#include "common/coroutines.h"
#include "tinsel/object.h"	// object manager defines

namespace Tinsel {

/** text mode flags - defaults to left justify */
enum {
	TXT_CENTER		= 0x0001,	///< center justify text
	TXT_RIGHT		= 0x0002,	///< right justify text
	TXT_SHADOW		= 0x0004,	///< shadow each character
	TXT_ABSOLUTE	= 0x0008	///< position of text is absolute (only for object text)
};

/** maximum number of characters in a font */
#define	MAX_FONT_CHARS	256

#define C16_240		0x4000
#define C16_224		0x8000
#define C16_MAP		0xC000
#define C16_FLAG_MASK	(C16_240 | C16_224 | C16_MAP)

#include "common/pack-start.h"	// START STRUCT PACKING

/**
 * Text font data structure.
 * @note only the pointer is used so the size of fontDef[] is not important.
 * It is currently set at 300 because it suited me for debugging.
 */
struct FONT {
	int xSpacing;			///< x spacing between characters
	int ySpacing;			///< y spacing between characters
	int xShadow;			///< x shadow offset
	int yShadow;			///< y shadow offset
	int spaceSize;			///< x spacing to use for a space character
	OBJ_INIT fontInit;		///< structure used to init text objects
	SCNHANDLE fontDef[300];	///< image handle array for all characters in the font
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING


/** structure for passing the correct parameters to ObjectTextOut */
struct TEXTOUT {
	OBJECT *pList;		///< object list to add text to
	char *szStr;		///< string to output
	int color;			///< color for monochrome text
	int xPos;			///< x position of string
	int yPos;			///< y position of string
	SCNHANDLE hFont;	///< which font to use
	int mode;			///< mode flags for the string
	int sleepTime;		///< sleep time between each character (if non-zero)
};


/*----------------------------------------------------------------------*\
|*			Text Function Prototypes			*|
\*----------------------------------------------------------------------*/

/**
 * Main text outputting routine. If a object list is specified a
 * multi-object is created for the whole text and a pointer to the head
 * of the list is returned.
 * @param pList			object list to add text to
 * @param szStr			string to output
 * @param color		color for monochrome text
 * @param xPos			x position of string
 * @param yPos			y position of string
 * @param hFont			which font to use
 * @param mode			mode flags for the string
 * @param sleepTime		Sleep time between each character (if non-zero)
 */
OBJECT *ObjectTextOut(OBJECT **pList, char *szStr, int color,
					int xPos, int yPos, SCNHANDLE hFont, int mode, int sleepTime = 0);

OBJECT *ObjectTextOutIndirect(	// output a string of text
	TEXTOUT *pText);	// pointer to TextOut struct with all parameters

bool IsCharImage(		// Is there an image for this character in this font?
	SCNHANDLE hFont,	// which font to use
	char c);		// character to test

} // End of namespace Tinsel

#endif	// TINSEL_TEXT_H
