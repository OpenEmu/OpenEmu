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
 * Text utilities.
 */

#include "tinsel/dw.h"
#include "tinsel/graphics.h"	// object plotting
#include "tinsel/handle.h"
#include "tinsel/sched.h"	// process scheduler defines
#include "tinsel/strres.h"	// g_bMultiByte
#include "tinsel/text.h"	// text defines

namespace Tinsel {

/**
 * Returns the length of one line of a string in pixels.
 * @param szStr			String
 * @param pFont			Which font to use for dimensions
 */
int StringLengthPix(char *szStr, const FONT *pFont) {
	int strLen;	// accumulated length of string
	byte	c;
	SCNHANDLE	hImg;

	// while not end of string or end of line
	for (strLen = 0; (c = *szStr) != EOS_CHAR && c != LF_CHAR; szStr++) {
		if (g_bMultiByte) {
			if (c & 0x80)
				c = ((c & ~0x80) << 8) + *++szStr;
		}
		hImg = FROM_LE_32(pFont->fontDef[c]);

		if (hImg) {
			// there is a IMAGE for this character
			const IMAGE *pChar = (const IMAGE *)LockMem(hImg);

			// add width of font bitmap
			strLen += FROM_LE_16(pChar->imgWidth);
		} else
			// use width of space character
			strLen += FROM_LE_32(pFont->spaceSize);

		// finally add the inter-character spacing
		strLen += FROM_LE_32(pFont->xSpacing);
	}

	// return length of line in pixels - minus inter-char spacing for last character
	strLen -= FROM_LE_32(pFont->xSpacing);
	return (strLen > 0) ? strLen : 0;
}

/**
 * Returns the justified x start position of a line of text.
 * @param szStr			String to output
 * @param xPos			X position of string
 * @param pFont			Which font to use
 * @param mode			Mode flags for the string
 */
int JustifyText(char *szStr, int xPos, const FONT *pFont, int mode) {
	if (mode & TXT_CENTER) {
		// center justify the text

		// adjust x positioning by half the length of line in pixels
		xPos -= StringLengthPix(szStr, pFont) / 2;
	} else if (mode & TXT_RIGHT) {
		// right justify the text

		// adjust x positioning by length of line in pixels
		xPos -= StringLengthPix(szStr, pFont);
	}

	// return text line x start position
	return xPos;
}

/**
 * Main text outputting routine. If a object list is specified a
 * multi-object is created for the whole text and a pointer to the head
 * of the list is returned.
 * @param pList			Object list to add text to
 * @param szStr			String to output
 * @param color		Color for monochrome text
 * @param xPos			X position of string
 * @param yPos			Y position of string
 * @param hFont			Which font to use
 * @param mode			Mode flags for the string
 * @param sleepTime		Sleep time between each character (if non-zero)
 */
OBJECT *ObjectTextOut(OBJECT **pList, char *szStr, int color,
					  int xPos, int yPos, SCNHANDLE hFont, int mode, int sleepTime) {
	int xJustify;	// x position of text after justification
	int yOffset;	// offset to next line of text
	OBJECT *pFirst;	// head of multi-object text list
	OBJECT *pChar = 0;	// object ptr for the character
	byte c;
	SCNHANDLE hImg;
	const IMAGE *pImg;

	// make sure there is a linked list to add text to
	assert(pList);

	// get font pointer
	const FONT *pFont = (const FONT *)LockMem(hFont);

	// init head of text list
	pFirst = NULL;

	// get image for capital W
	assert(pFont->fontDef[(int)'W']);
	pImg = (const IMAGE *)LockMem(FROM_LE_32(pFont->fontDef[(int)'W']));

	// get height of capital W for offset to next line
	yOffset = FROM_LE_16(pImg->imgHeight) & ~C16_FLAG_MASK;

	while (*szStr) {
		// x justify the text according to the mode flags
		xJustify = JustifyText(szStr, xPos, pFont, mode);

		// repeat until end of string or end of line
		while ((c = *szStr) != EOS_CHAR && c != LF_CHAR) {
			if (g_bMultiByte) {
				if (c & 0x80)
					c = ((c & ~0x80) << 8) + *++szStr;
			}
			hImg = FROM_LE_32(pFont->fontDef[c]);

			if (hImg == 0) {
				// no image for this character

				// add font spacing for a space character
				xJustify += FROM_LE_32(pFont->spaceSize);
			} else {	// printable character

				int aniX, aniY;		// char image animation offsets

				OBJ_INIT oi;
				oi.hObjImg  = FROM_LE_32(pFont->fontInit.hObjImg);
				oi.objFlags = FROM_LE_32(pFont->fontInit.objFlags);
				oi.objID    = FROM_LE_32(pFont->fontInit.objID);
				oi.objX     = FROM_LE_32(pFont->fontInit.objX);
				oi.objY     = FROM_LE_32(pFont->fontInit.objY);
				oi.objZ     = FROM_LE_32(pFont->fontInit.objZ);

				// allocate and init a character object
				if (pFirst == NULL)
					// first time - init head of list
					pFirst = pChar = InitObject(&oi);	// FIXME: endian issue using fontInit!!!
				else
					// chain to multi-char list
					pChar = pChar->pSlave = InitObject(&oi);	// FIXME: endian issue using fontInit!!!

				// convert image handle to pointer
				pImg = (const IMAGE *)LockMem(hImg);

				// fill in character object
				pChar->hImg   = hImg;			// image def
				pChar->width  = FROM_LE_16(pImg->imgWidth);		// width of chars bitmap
				pChar->height = FROM_LE_16(pImg->imgHeight) & ~C16_FLAG_MASK;	// height of chars bitmap
				pChar->hBits  = FROM_LE_32(pImg->hImgBits);		// bitmap

				// check for absolute positioning
				if (mode & TXT_ABSOLUTE)
					pChar->flags |= DMA_ABS;

				// set characters color - only effective for mono fonts
				pChar->constant = color;

				// get Y animation offset
				GetAniOffset(hImg, pChar->flags, &aniX, &aniY);

				// set x position - ignore animation point
				pChar->xPos = intToFrac(xJustify);

				// set y position - adjust for animation point
				pChar->yPos = intToFrac(yPos - aniY);

				if (mode & TXT_SHADOW) {
					// we want to shadow the character
					OBJECT *pShad;

					// allocate a object for the shadow and chain to multi-char list
					pShad = pChar->pSlave = AllocObject();

					// copy the character for a shadow
					CopyObject(pShad, pChar);

					// add shadow offsets to characters position
					pShad->xPos += intToFrac(FROM_LE_32(pFont->xShadow));
					pShad->yPos += intToFrac(FROM_LE_32(pFont->yShadow));

					// shadow is behind the character
					pShad->zPos--;

					// shadow is always mono
					pShad->flags = DMA_CNZ | DMA_CHANGED;

					// check for absolute positioning
					if (mode & TXT_ABSOLUTE)
						pShad->flags |= DMA_ABS;

					// shadow always uses first palette entry
					// should really alloc a palette here also ????
					pShad->constant = 1;

					// add shadow to object list
					InsertObject(pList, pShad);
				}

				// add character to object list
				InsertObject(pList, pChar);

				// move to end of list
				if (pChar->pSlave)
					pChar = pChar->pSlave;

				// add character spacing
				xJustify += FROM_LE_16(pImg->imgWidth);
			}

			// finally add the inter-character spacing
			xJustify += FROM_LE_32(pFont->xSpacing);

			// next character in string
			++szStr;
		}

		// adjust the text y position and add the inter-line spacing
		yPos += yOffset + FROM_LE_32(pFont->ySpacing);

		// check for newline
		if (c == LF_CHAR)
			// next character in string
			++szStr;
	}

	// return head of list
	return pFirst;
}

/**
 * Is there an image for this character in this font?
 * @param hFont	which font to use
 * @param c		character to test
 */
bool IsCharImage(SCNHANDLE hFont, char c) {
	byte c2 = (byte)c;

	// Inventory save game name editor needs to be more clever for
	// multi-byte characters. This bodge will stop it erring.
	if (g_bMultiByte && (c2 & 0x80))
		return false;

	// get font pointer
	const FONT *pFont = (const FONT *)LockMem(hFont);

	return pFont->fontDef[c2] != 0;
}

} // End of namespace Tinsel
