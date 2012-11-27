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
 * Background handling code.
 */

#include "tinsel/background.h"
#include "tinsel/cliprect.h"	// object clip rect defs
#include "tinsel/graphics.h"
#include "tinsel/sched.h"	// process sheduler defs
#include "tinsel/object.h"
#include "tinsel/pid.h"	// process identifiers
#include "tinsel/tinsel.h"

namespace Tinsel {

// FIXME: Avoid non-const global vars

// current background
const BACKGND *g_pCurBgnd = NULL;

/**
 * Called to initialize a background.
 * @param pBgnd			Pointer to data struct for current background
 */

void InitBackground(const BACKGND *pBgnd) {
	int i;			// playfield counter
	PLAYFIELD *pPlayfield;	// pointer to current playfield

	// set current background
	g_pCurBgnd = pBgnd;

	// init background sky color
	SetBgndColor(pBgnd->rgbSkyColor);

	// start of playfield array
	pPlayfield = pBgnd->fieldArray;

	// for each background playfield
	for (i = 0; i < pBgnd->numPlayfields; i++, pPlayfield++) {
		// init playfield pos
		pPlayfield->fieldX = intToFrac(pBgnd->ptInitWorld.x);
		pPlayfield->fieldY = intToFrac(pBgnd->ptInitWorld.y);

		// no scrolling
		pPlayfield->fieldXvel = intToFrac(0);
		pPlayfield->fieldYvel = intToFrac(0);

		// clear playfield display list
		pPlayfield->pDispList = NULL;

		// clear playfield moved flag
		pPlayfield->bMoved = false;
	}
}

/**
 * Sets the xy position of the specified playfield in the current background.
 * @param which			Which playfield
 * @param newXpos		New x position
 * @param newYpos		New y position
 */

void PlayfieldSetPos(int which, int newXpos, int newYpos) {
	PLAYFIELD *pPlayfield;	// pointer to relavent playfield

	// make sure there is a background
	assert(g_pCurBgnd != NULL);

	// make sure the playfield number is in range
	assert(which >= 0 && which < g_pCurBgnd->numPlayfields);

	// get playfield pointer
	pPlayfield = g_pCurBgnd->fieldArray + which;

	// set new integer position
	pPlayfield->fieldX = intToFrac(newXpos);
	pPlayfield->fieldY = intToFrac(newYpos);

	// set moved flag
	pPlayfield->bMoved = true;
}

/**
 * Returns the xy position of the specified playfield in the current background.
 * @param which			Which playfield
 * @param pXpos			Returns current x position
 * @param pYpos			Returns current y position
 */

void PlayfieldGetPos(int which, int *pXpos, int *pYpos) {
	PLAYFIELD *pPlayfield;	// pointer to relavent playfield

	// make sure there is a background
	assert(g_pCurBgnd != NULL);

	// make sure the playfield number is in range
	assert(which >= 0 && which < g_pCurBgnd->numPlayfields);

	// get playfield pointer
	pPlayfield = g_pCurBgnd->fieldArray + which;

	// get current integer position
	*pXpos = fracToInt(pPlayfield->fieldX);
	*pYpos = fracToInt(pPlayfield->fieldY);
}

/**
 * Returns the x position of the center of the specified playfield
 * @param which			Which playfield
 */

int PlayfieldGetCenterX(int which) {
	PLAYFIELD *pPlayfield; // pointer to relavent playfield

	// make sure there is a background
	assert(g_pCurBgnd != NULL);

	// make sure the playfield number is in range
	assert(which >= 0 && which < g_pCurBgnd->numPlayfields);

	// get playfield pointer
	pPlayfield = g_pCurBgnd->fieldArray + which;

	// get current integer position
	return fracToInt(pPlayfield->fieldX) + SCREEN_WIDTH/2;
}

/**
 * Returns the display list for the specified playfield.
 * @param which			Which playfield
 */

OBJECT **GetPlayfieldList(int which) {
	PLAYFIELD *pPlayfield;	// pointer to relavent playfield

	// make sure there is a background
	assert(g_pCurBgnd != NULL);

	// make sure the playfield number is in range
	assert(which >= 0 && which < g_pCurBgnd->numPlayfields);

	// get playfield pointer
	pPlayfield = g_pCurBgnd->fieldArray + which;

	// return the display list pointer for this playfield
	return &pPlayfield->pDispList;
}

/**
 * Draws all the playfield object lists for the current background.
 * The playfield velocity is added to the playfield position in order
 * to scroll each playfield before it is drawn.
 */

void DrawBackgnd() {
	int i;			// playfield counter
	PLAYFIELD *pPlay;	// playfield pointer
	int prevX, prevY;	// save interger part of position
	Common::Point ptWin;	// window top left

	if (g_pCurBgnd == NULL)
		return;		// no current background

	// scroll each background playfield
	for (i = 0; i < g_pCurBgnd->numPlayfields; i++) {
		// get pointer to correct playfield
		pPlay = g_pCurBgnd->fieldArray + i;

		// save integer part of position
		prevX = fracToInt(pPlay->fieldX);
		prevY = fracToInt(pPlay->fieldY);

		// update scrolling
		pPlay->fieldX += pPlay->fieldXvel;
		pPlay->fieldY += pPlay->fieldYvel;

		// convert fixed point window pos to a int
		ptWin.x = fracToInt(pPlay->fieldX);
		ptWin.y = fracToInt(pPlay->fieldY);

		// set the moved flag if the playfield has moved
		if (prevX != ptWin.x || prevY != ptWin.y)
			pPlay->bMoved = true;

		// sort the display list for this background - just in case somebody has changed object Z positions
		SortObjectList(&pPlay->pDispList);

		// generate clipping rects for all objects that have moved etc.
		FindMovingObjects(&pPlay->pDispList, &ptWin,
			&pPlay->rcClip,	false, pPlay->bMoved);

		// clear playfield moved flag
		pPlay->bMoved = false;
	}

	// merge the clipping rectangles
	MergeClipRect();

	// redraw all playfields within the clipping rectangles
	const RectList &clipRects = GetClipRects();
	for (RectList::const_iterator r = clipRects.begin(); r != clipRects.end(); ++r) {
		// clear the clip rectangle on the virtual screen
		// for each background playfield
		for (i = 0; i < g_pCurBgnd->numPlayfields; i++) {
			Common::Rect rcPlayClip;	// clip rect for this playfield

			// get pointer to correct playfield
			pPlay = g_pCurBgnd->fieldArray + i;

			// convert fixed point window pos to a int
			ptWin.x = fracToInt(pPlay->fieldX);
			ptWin.y = fracToInt(pPlay->fieldY);

			if (IntersectRectangle(rcPlayClip, pPlay->rcClip, *r))
				// redraw all objects within this clipping rect
				UpdateClipRect(&pPlay->pDispList, &ptWin,	&rcPlayClip);
		}
	}

	// transfer any new palettes to the video DAC
	PalettesToVideoDAC();

	// update the screen within the clipping rectangles
	for (RectList::const_iterator r = clipRects.begin(); r != clipRects.end(); ++r) {
		UpdateScreenRect(*r);
	}

	g_system->updateScreen();

	// delete all the clipping rectangles
	ResetClipRect();
}

} // End of namespace Tinsel
