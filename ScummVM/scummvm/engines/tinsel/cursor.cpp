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
 * Cursor and cursor trails.
 */

#include "tinsel/cursor.h"

#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/events.h"		// For EventsManager class
#include "tinsel/film.h"
#include "tinsel/graphics.h"
#include "tinsel/handle.h"
#include "tinsel/dialogs.h"
#include "tinsel/multiobj.h"	// multi-part object defintions etc.
#include "tinsel/object.h"
#include "tinsel/pid.h"
#include "tinsel/play.h"
#include "tinsel/sched.h"
#include "tinsel/sysvar.h"
#include "tinsel/text.h"
#include "tinsel/timers.h"		// For ONE_SECOND constant
#include "tinsel/tinlib.h"		// resetidletime()
#include "tinsel/tinsel.h"		// For engine access


namespace Tinsel {

//----------------- LOCAL DEFINES --------------------

#define ITERATION_BASE		FRAC_ONE
#define ITER_ACCELERATION	(10L << (FRAC_BITS - 4))


//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

static OBJECT *g_McurObj = NULL;		// Main cursor object
static OBJECT *g_AcurObj = NULL;		// Auxiliary cursor object

static ANIM g_McurAnim = {0,0,0,0,0};		// Main cursor animation structure
static ANIM g_AcurAnim = {0,0,0,0,0};		// Auxiliary cursor animation structure

static bool g_bHiddenCursor = false;		// Set when cursor is hidden
static bool g_bTempNoTrailers = false;	// Set when cursor trails are hidden
static bool g_bTempHide = false;			// Set when cursor is hidden

static bool g_bFrozenCursor = false;	// Set when cursor position is frozen

static frac_t g_IterationSize = 0;

static SCNHANDLE g_hCursorFilm = 0;	// Handle to cursor reel data

static int g_numTrails = 0;
static int g_nextTrail = 0;

static bool g_bWhoa = false;		// Set by DropCursor() at the end of a scene
				// - causes cursor processes to do nothing
				// Reset when main cursor has re-initialized

static uint16 g_restart = 0;	// When main cursor has been bWhoa-ed, it waits
							// for this to be set to 0x8000.
							// Main cursor sets all the bits after a re-start
							// - each cursor trail examines it's own bit
							// to trigger a trail restart.

static short g_ACoX = 0, g_ACoY = 0;	// Auxillary cursor image's animation offsets



#define MAX_TRAILERS	10

static struct {

	ANIM	trailAnim;	// Animation structure
	OBJECT *trailObj;	// This trailer's object

} g_ntrailData [MAX_TRAILERS];

static int g_lastCursorX = 0, g_lastCursorY = 0;


//----------------- FORWARD REFERENCES --------------------

static void DoCursorMove();

/**
 * Initialize and insert a cursor trail object, set its Z-pos, and hide
 * it. Also initialize its animation script.
 */
static void InitCurTrailObj(int i, int x, int y) {
	const FREEL *pfr;		// pointer to reel
	IMAGE *pim;		// pointer to image
	const MULTI_INIT *pmi;		// MULTI_INIT structure

	const FILM *pfilm;

	if (!g_numTrails)
		return;

	// Get rid of old object
	if (g_ntrailData[i].trailObj != NULL)
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), g_ntrailData[i].trailObj);

	pim = GetImageFromFilm(g_hCursorFilm, i+1, &pfr, &pmi, &pfilm);// Get pointer to image
	assert(BgPal()); // No background palette
	pim->hImgPal = TO_LE_32(BgPal());

	// Initialize and insert the object, set its Z-pos, and hide it
	g_ntrailData[i].trailObj = MultiInitObject(pmi);
	MultiInsertObject(GetPlayfieldList(FIELD_STATUS), g_ntrailData[i].trailObj);
	MultiSetZPosition(g_ntrailData[i].trailObj, Z_CURSORTRAIL);
	MultiSetAniXY(g_ntrailData[i].trailObj, x, y);

	// Initialize the animation script
	InitStepAnimScript(&g_ntrailData[i].trailAnim, g_ntrailData[i].trailObj, FROM_LE_32(pfr->script), ONE_SECOND / FROM_LE_32(pfilm->frate));
	StepAnimScript(&g_ntrailData[i].trailAnim);
}

/**
 * Get the cursor position from the mouse driver.
 */
static bool GetDriverPosition(int *x, int *y) {
	Common::Point ptMouse = _vm->getMousePosition();
	*x = ptMouse.x;
	*y = ptMouse.y;

	return(*x >= 0 && *x <= SCREEN_WIDTH - 1 &&
		*y >= 0 && *y <= SCREEN_HEIGHT - 1);
}

/**
 * Move the cursor relative to current position.
 */
void AdjustCursorXY(int deltaX, int deltaY) {
	int x, y;

	if (deltaX || deltaY) {
		if (GetDriverPosition(&x, &y))
			_vm->setMousePosition(Common::Point(x + deltaX, y + deltaY));
	}
	DoCursorMove();
}

/**
 * Move the cursor to an absolute position.
 */
void SetCursorXY(int newx, int newy) {
	int	x, y;
	int	Loffset, Toffset;	// Screen offset

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
	newx -= Loffset;
	newy -= Toffset;

	if (GetDriverPosition(&x, &y))
		_vm->setMousePosition(Common::Point(newx, newy));
	DoCursorMove();
}

/**
 * Move the cursor to a screen position.
 */
void SetCursorScreenXY(int newx, int newy) {
	int	x, y;

	if (GetDriverPosition(&x, &y))
		_vm->setMousePosition(Common::Point(newx, newy));
	DoCursorMove();
}

/**
 * Called by the world and his brother.
 * Returns the cursor's animation position in (x,y).
 * Returns false if there is no cursor object.
 */
bool GetCursorXYNoWait(int *x, int *y, bool absolute) {
	if (g_McurObj == NULL) {
		*x = *y = 0;
		return false;
	}

	GetAniPosition(g_McurObj, x, y);

	if (absolute) {
		int	Loffset, Toffset;	// Screen offset
		PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
		*x += Loffset;
		*y += Toffset;
	}

	return true;
}

/**
 * Called by the world and his brother.
 * Returns the cursor's animation position.
 * If called while there is no cursor object, the calling process ends
 * up waiting until there is.
 */
void GetCursorXY(int *x, int *y, bool absolute) {
	//while (McurObj == NULL)
	//	ProcessSleepSelf();
	assert(g_McurObj);
	GetCursorXYNoWait(x, y, absolute);
}

/**
 * Re-initialize the main cursor to use the main cursor reel.
 * Called from TINLIB.C to restore cursor after hiding it.
 * Called from INVENTRY.C to restore cursor after customising it.
 */
void RestoreMainCursor() {
	const FILM *pfilm;

	if (g_McurObj != NULL) {
		pfilm = (const FILM *)LockMem(g_hCursorFilm);

		InitStepAnimScript(&g_McurAnim, g_McurObj, FROM_LE_32(pfilm->reels->script), ONE_SECOND / FROM_LE_32(pfilm->frate));
		StepAnimScript(&g_McurAnim);
	}
	g_bHiddenCursor = false;
	g_bFrozenCursor = false;
}

/**
 * Called from INVENTRY.C to customise the main cursor.
 */
void SetTempCursor(SCNHANDLE pScript) {
	if (g_McurObj != NULL)
		InitStepAnimScript(&g_McurAnim, g_McurObj, pScript, 2);
}

/**
 * Hide the cursor.
 */
void DwHideCursor() {
	int i;

	g_bHiddenCursor = true;

	if (g_McurObj)
		MultiHideObject(g_McurObj);
	if (g_AcurObj)
		MultiHideObject(g_AcurObj);

	for (i = 0; i < g_numTrails; i++) {
		if (g_ntrailData[i].trailObj != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), g_ntrailData[i].trailObj);
			g_ntrailData[i].trailObj = NULL;
		}
	}
}

/**
 * Unhide the cursor.
 */
void UnHideCursor() {
	g_bHiddenCursor = false;
}

/**
 * Freeze the cursor.
 */
void FreezeCursor() {
	g_bFrozenCursor = true;
}

/**
 * Freeze the cursor, or not.
 */
void DoFreezeCursor(bool bFreeze) {
	g_bFrozenCursor = bFreeze;
}

/**
 * HideCursorTrails
 */
void HideCursorTrails() {
	int i;

	g_bTempNoTrailers = true;

	for (i = 0; i < g_numTrails; i++)	{
		if (g_ntrailData[i].trailObj != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), g_ntrailData[i].trailObj);
			g_ntrailData[i].trailObj = NULL;
		}
	}
}

/**
 * UnHideCursorTrails
 */
void UnHideCursorTrails() {
	g_bTempNoTrailers = false;
}

/**
 * Get pointer to image from a film reel. And the rest.
 */
IMAGE *GetImageFromReel(const FREEL *pfr, const MULTI_INIT **ppmi) {
	const MULTI_INIT *pmi;
	const FRAME *pFrame;

	pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(pfr->mobj));
	if (ppmi)
		*ppmi = pmi;

	pFrame = (const FRAME *)LockMem(FROM_LE_32(pmi->hMulFrame));

	// get pointer to image
	return (IMAGE *)LockMem(READ_LE_UINT32(pFrame));
}

/**
 * Get pointer to image from a film. And the rest.
 */
IMAGE *GetImageFromFilm(SCNHANDLE hFilm, int reel, const FREEL **ppfr, const MULTI_INIT **ppmi, const FILM **ppfilm) {
	const FILM *pfilm;
	const FREEL *pfr;

	pfilm = (const FILM *)LockMem(hFilm);
	if (ppfilm)
		*ppfilm = pfilm;

	pfr = &pfilm->reels[reel];
	if (ppfr)
		*ppfr = pfr;

	return GetImageFromReel(pfr, ppmi);
}

/**
 * Delete auxillary cursor. Restore animation offsets in the image.
 */
void DelAuxCursor() {
	if (g_AcurObj != NULL) {
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), g_AcurObj);
		g_AcurObj = NULL;
	}
}

/**
 * Set auxillary cursor.
 * Save animation offsets from the image if required.
 */
void SetAuxCursor(SCNHANDLE hFilm) {
	IMAGE *pim;		// Pointer to auxillary cursor's image
	const FREEL *pfr;
	const MULTI_INIT *pmi;
	const FILM *pfilm;
	int	x, y;		// Cursor position

	DelAuxCursor();		// Get rid of previous

	GetCursorXY(&x, &y, false);	// Note: also waits for cursor to appear

	pim = GetImageFromFilm(hFilm, 0, &pfr, &pmi, &pfilm);// Get pointer to image
	assert(BgPal()); // no background palette
	pim->hImgPal = TO_LE_32(BgPal());			// Poke in the background palette

	g_ACoX = (short)(FROM_LE_16(pim->imgWidth)/2 - ((int16) FROM_LE_16(pim->anioffX)));
	g_ACoY = (short)((FROM_LE_16(pim->imgHeight) & ~C16_FLAG_MASK)/2 -
		((int16) FROM_LE_16(pim->anioffY)));

	// Initialize and insert the auxillary cursor object
	g_AcurObj = MultiInitObject(pmi);
	MultiInsertObject(GetPlayfieldList(FIELD_STATUS), g_AcurObj);

	// Initialize the animation and set its position
	InitStepAnimScript(&g_AcurAnim, g_AcurObj, FROM_LE_32(pfr->script), ONE_SECOND / FROM_LE_32(pfilm->frate));
	MultiSetAniXY(g_AcurObj, x - g_ACoX, y - g_ACoY);
	MultiSetZPosition(g_AcurObj, Z_ACURSOR);

	if (g_bHiddenCursor)
		MultiHideObject(g_AcurObj);
}

/**
 * MoveCursor
 */
static void DoCursorMove() {
	int	startX, startY;
	Common::Point ptMouse;
	frac_t newX, newY;
	unsigned dir;

	// get cursors start animation position
	GetCursorXYNoWait(&startX, &startY, false);

	// get mouse drivers current position
	ptMouse = _vm->getMousePosition();

	// convert to fixed point
	newX = intToFrac(ptMouse.x);
	newY = intToFrac(ptMouse.y);

	// modify mouse driver position depending on cursor keys
	dir = _vm->getKeyDirection();
	if (dir != 0) {
		if (dir & MSK_LEFT)
			newX -= g_IterationSize;

		if (dir & MSK_RIGHT)
			newX += g_IterationSize;

		if (dir & MSK_UP)
			newY -= g_IterationSize;

		if (dir & MSK_DOWN)
			newY += g_IterationSize;

		g_IterationSize += ITER_ACCELERATION;

		// set new mouse driver position
		_vm->setMousePosition(Common::Point(fracToInt(newX), fracToInt(newY)));
	} else

		g_IterationSize = ITERATION_BASE;

	// get new mouse driver position - could have been modified
	ptMouse = _vm->getMousePosition();

	if (g_lastCursorX != ptMouse.x || g_lastCursorY != ptMouse.y) {
		resetUserEventTime();

		if (!g_bTempNoTrailers && !g_bHiddenCursor) {
			InitCurTrailObj(g_nextTrail++, g_lastCursorX, g_lastCursorY);
			if (g_nextTrail == g_numTrails)
				g_nextTrail = 0;
		}
	}

	// adjust cursor to new mouse position
	if (g_McurObj)
		MultiSetAniXY(g_McurObj, ptMouse.x, ptMouse.y);
	if (g_AcurObj != NULL)
		MultiSetAniXY(g_AcurObj, ptMouse.x - g_ACoX, ptMouse.y - g_ACoY);

	if (InventoryActive() && g_McurObj) {
		// Notify the inventory
		Xmovement(ptMouse.x - startX);
		Ymovement(ptMouse.y - startY);
	}

	g_lastCursorX = ptMouse.x;
	g_lastCursorY = ptMouse.y;
}

/**
 * Initialize cursor object.
 */
static void InitCurObj() {
	const FILM *pFilm;
	const FREEL *pfr;
	const MULTI_INIT *pmi;
	IMAGE *pim;

	if (TinselV2) {
		pFilm = (const FILM *)LockMem(g_hCursorFilm);
		pfr = (const FREEL *)&pFilm->reels[0];
		pmi = (MULTI_INIT *)LockMem(FROM_LE_32(pfr->mobj));

		PokeInPalette(pmi);
	} else {
		assert(BgPal()); // no background palette

		pim = GetImageFromFilm(g_hCursorFilm, 0, &pfr, &pmi, &pFilm);// Get pointer to image
		pim->hImgPal = TO_LE_32(BgPal());

		g_AcurObj = NULL;		// No auxillary cursor
	}

	g_McurObj = MultiInitObject(pmi);
	MultiInsertObject(GetPlayfieldList(FIELD_STATUS), g_McurObj);

	InitStepAnimScript(&g_McurAnim, g_McurObj, FROM_LE_32(pfr->script), ONE_SECOND / FROM_LE_32(pFilm->frate));
}

/**
 * Initialize the cursor position.
 */
static void InitCurPos() {
	Common::Point ptMouse = _vm->getMousePosition();
	g_lastCursorX = ptMouse.x;
	g_lastCursorY = ptMouse.y;

	MultiSetZPosition(g_McurObj, Z_CURSOR);
	DoCursorMove();
	MultiHideObject(g_McurObj);

	g_IterationSize = ITERATION_BASE;
}

/**
 * CursorStoppedCheck
 */
static void CursorStoppedCheck(CORO_PARAM) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// If scene is closing down
	if (g_bWhoa) {
		// ...wait for next scene start-up
		while (g_restart != 0x8000)
			CORO_SLEEP(1);

		// Re-initialize
		InitCurObj();
		InitCurPos();
		InventoryIconCursor(false);	// May be holding something

		// Re-start the cursor trails
		g_restart = (uint16)-1;		// set all bits
		g_bWhoa = false;
	}
	CORO_END_CODE;
}

/**
 * The main cursor process.
 */
void CursorProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	while (!g_hCursorFilm || !BgPal())
		CORO_SLEEP(1);

	InitCurObj();
	InitCurPos();
	InventoryIconCursor(false);		// May be holding something

	g_bWhoa = false;
	g_restart = 0;

	while (1) {
		// allow rescheduling
		CORO_SLEEP(1);

		// Stop/start between scenes
		CORO_INVOKE_0(CursorStoppedCheck);

		// Step the animation script(s)
		StepAnimScript(&g_McurAnim);
		if (g_AcurObj != NULL)
			StepAnimScript(&g_AcurAnim);
		for (int i = 0; i < g_numTrails; i++) {
			if (g_ntrailData[i].trailObj != NULL) {
				if (StepAnimScript(&g_ntrailData[i].trailAnim) == ScriptFinished) {
					MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), g_ntrailData[i].trailObj);
					g_ntrailData[i].trailObj = NULL;
				}
			}
		}

		// Move the cursor as appropriate
		if (!g_bFrozenCursor)
			DoCursorMove();

		// If the cursor should be hidden...
		if (g_bHiddenCursor || g_bTempHide) {
			// ...hide the cursor object(s)
			MultiHideObject(g_McurObj);
			if (g_AcurObj)
				MultiHideObject(g_AcurObj);

			for (int i = 0; i < g_numTrails; i++) {
				if (g_ntrailData[i].trailObj != NULL)
					MultiHideObject(g_ntrailData[i].trailObj);
			}

			// Wait 'til cursor is again required.
			while (g_bHiddenCursor) {
				CORO_SLEEP(1);

				// Stop/start between scenes
				CORO_INVOKE_0(CursorStoppedCheck);
			}
		}
	}
	CORO_END_CODE;
}

/**
 * Called from dec_cursor() Glitter function.
 * Register the handle to cursor reel data.
 */
void DwInitCursor(SCNHANDLE bfilm) {
	const FILM *pfilm;

	g_hCursorFilm = bfilm;

	pfilm = (const FILM *)LockMem(g_hCursorFilm);
	g_numTrails = FROM_LE_32(pfilm->numreels) - 1;

	assert(g_numTrails <= MAX_TRAILERS);
}

/**
 * DropCursor is called when a scene is closing down.
 */
void DropCursor() {
	if (TinselV2) {
		if (g_AcurObj)
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), g_AcurObj);
		if (g_McurObj)
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), g_McurObj);

		g_restart = 0;
	}

	g_AcurObj = NULL;		// No auxillary cursor
	g_McurObj = NULL;		// No cursor object (imminently deleted elsewhere)
	g_bHiddenCursor = false;	// Not hidden in next scene
	g_bTempNoTrailers = false;	// Trailers not hidden in next scene
	g_bWhoa = true;		// Suspend cursor processes

	for (int i = 0; i < g_numTrails; i++) {
		if (g_ntrailData[i].trailObj != NULL)		{
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), g_ntrailData[i].trailObj);
			g_ntrailData[i].trailObj = NULL;
		}
	}
}

/**
 * RestartCursor is called when a new scene is starting up.
 */
void RestartCursor() {
	g_restart = 0x8000;	// Get the main cursor to re-initialize
}

/**
 * Called when restarting the game, ensures correct re-start with NULL
 * pointers etc.
 */
void RebootCursor() {
	g_McurObj = g_AcurObj = NULL;
	for (int i = 0; i < MAX_TRAILERS; i++)
		g_ntrailData[i].trailObj = NULL;

	g_bHiddenCursor = g_bTempNoTrailers = g_bFrozenCursor = false;

	g_hCursorFilm = 0;

	g_bWhoa = false;
	g_restart = 0;
}

void StartCursorFollowed() {
	DelAuxCursor();

	if (!SysVar(SV_ENABLEPRINTCURSOR))
		g_bTempHide = true;
}

void EndCursorFollowed() {
	InventoryIconCursor(false);	// May be holding something
	g_bTempHide = false;
}

bool isCursorShown() {
	return !(g_bTempHide || g_bHiddenCursor);
}

} // End of namespace Tinsel
