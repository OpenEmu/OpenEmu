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
 * Should really be called "moving actors.c"
 */

#include "tinsel/actors.h"
#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/config.h"
#include "tinsel/dw.h"
#include "tinsel/film.h"
#include "tinsel/handle.h"
#include "tinsel/dialogs.h"
#include "tinsel/mareels.h"
#include "tinsel/move.h"
#include "tinsel/multiobj.h"	// multi-part object defintions etc.
#include "tinsel/object.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"
#include "tinsel/play.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/sched.h"
#include "tinsel/sysvar.h"
#include "tinsel/timers.h"
#include "tinsel/tinsel.h"
#include "tinsel/token.h"

#include "common/textconsole.h"
#include "common/util.h"

namespace Tinsel {

//----------------- LOCAL GLOBAL DATA --------------------

static MOVER g_Movers[MAX_MOVERS];	// FIXME: Avoid non-const global vars

//----------------- FUNCTIONS ----------------------------

/**
 * Called from ActorPalette(), normally once just after the beginning of time.
 */
void StoreMoverPalette(PMOVER pMover, int startColor, int length) {
	pMover->startColor = startColor;
	pMover->paletteLength = length;
}

/**
 * Called from the moving actor's main loop.
 */
static void CheckBrightness(PMOVER pMover) {
	int brightness;

	if (pMover->hCpath == NOPOLY || pMover->bHidden)
		return;

	brightness = GetBrightness(pMover->hCpath, pMover->objY);

	if (brightness != pMover->brightness) {
		// Do it all immediately on first appearance,
		// otherwise do it iteratively

		if (pMover->brightness == BOGUS_BRIGHTNESS)
			pMover->brightness = brightness;	// all the way
		else if (brightness > pMover->brightness)
			pMover->brightness++;			// ramp up
		else
			pMover->brightness--;			// ramp down

		DimPartPalette(BgPal(),
				pMover->startColor,
				pMover->paletteLength,
				pMover->brightness);
	}
}

/**
 * Called from ActorBrightness() Glitter call.
 * Typically called before the moving actor is created
 * at the start of a scene to cover a walk-in Play().
 */
void MoverBrightness(PMOVER pMover, int brightness) {
	// Note: Like with some of the Tinsel1 code, this routine original had a process yield
	// if BgPal is NULL, and has been changed for ScummVM to a simple assert

	// This is changed from a ProcessGiveWay in DW2 to an assert in ScummVM
	assert(BgPal());

	// Do it all immediately
	DimPartPalette(BgPal(), pMover->startColor, pMover->paletteLength, brightness);

	// The actor is probably hidden at this point,
	pMover->brightness = brightness;
}

/**
 * RebootMovers
 */
void RebootMovers() {
	memset(g_Movers, 0, sizeof(g_Movers));
}

/**
 * Given an actor number, return pointer to its moving actor structure,
 * if it is a moving actor.
 */
PMOVER GetMover(int ano) {
	int i;

	// Slot 0 is reserved for lead actor
	if (ano == GetLeadId() || ano == LEAD_ACTOR)
		return &g_Movers[0];

	for (i = 1; i < MAX_MOVERS; i++)
		if (g_Movers[i].actorID == ano)
			return &g_Movers[i];

	return NULL;
}

/**
 * Register an actor as being a moving one.
 */
PMOVER RegisterMover(int ano) {
	int i;

	// Slot 0 is reserved for lead actor
	if (ano == GetLeadId() || ano == LEAD_ACTOR) {
		g_Movers[0].actorToken = TOKEN_LEAD;
		g_Movers[0].actorID = GetLeadId();
		return &g_Movers[0];
	}

	// Check it hasn't already been declared
	for (i = 1; i < MAX_MOVERS; i++) {
		if (g_Movers[i].actorID == ano) {
			// Actor is already a moving actor
			return &g_Movers[i];
		}
	}

	// Find an empty slot
	for (i = 1; i < MAX_MOVERS; i++)
		if (!g_Movers[i].actorID) {
			g_Movers[i].actorToken = TOKEN_LEAD + i;
			g_Movers[i].actorID = ano;
			return &g_Movers[i];
		}

	error("Too many moving actors");
}

/**
 * Given an index, returns the associated moving actor.
 *
 * At the time of writing, used by the effect process.
 */
PMOVER GetLiveMover(int index) {
	assert(index >= 0 && index < MAX_MOVERS); // out of range

	if (g_Movers[index].bActive)
		return &g_Movers[index];
	else
		return NULL;
}

bool IsMAinEffectPoly(int index) {
	assert(index >= 0 && index < MAX_MOVERS); // out of range

	return g_Movers[index].bInEffect;
}

void SetMoverInEffect(int index, bool tf) {
	assert(index >= 0 && index < MAX_MOVERS); // out of range

	g_Movers[index].bInEffect = tf;
}

/**
 * Remove a moving actor from the current scene.
 */
void KillMover(PMOVER pMover) {
	if (pMover->bActive) {
		pMover->bActive = false;
		MultiDeleteObject(GetPlayfieldList(FIELD_WORLD), pMover->actorObj);
		pMover->actorObj = NULL;
		assert(CoroScheduler.getCurrentProcess() != pMover->pProc);
		CoroScheduler.killProcess(pMover->pProc);
	}
}

/**
 * getMActorState
 */
bool getMActorState(PMOVER pActor) {
	return pActor->bActive;
}

/**
 * If the actor's object exists, move it behind the background.
 * MultiHideObject() is deliberately not used, as StepAnimScript() calls
 * cause the object to re-appear.
 */
void HideMover(PMOVER pMover, int sf) {
	assert(pMover); // Hiding null moving actor

	pMover->bHidden = true;

	if (!TinselV2) {
		// sf is only passed in Tinsel v1
		pMover->SlowFactor = sf;
	} else {
		// Tinsel 2 specific code
		if (IsTaggedActor(pMover->actorID)) {
			// It may be pointed to
			SetActorPointedTo(pMover->actorID, false);
			SetActorTagWanted(pMover->actorID, false, false, 0);
		}
	}

	if (pMover->actorObj)
		MultiSetZPosition(pMover->actorObj, -1);
}

/**
 * MoverHidden
 */
bool MoverHidden(PMOVER pMover) {
	if (pMover)
		return pMover->bHidden;
	else
		return false;
}

/**
 * To be or not to be? If it be, then it is.
 */
bool MoverIs(PMOVER pMover) {
	if (TinselV2)
		return pMover->actorObj ? true : false;
	else
		return getMActorState(pMover);
}

/**
 * To be SWalk()ing or not to be SWalk()ing?
 */
bool MoverIsSWalking(PMOVER pMover) {
	return (MoverMoving(pMover) && pMover->bIgPath);
}

/**
 * MoverMoving()
 */
bool MoverMoving(PMOVER pMover) {
	if (!TinselV2)
		return pMover->bMoving;

	if (pMover->UtargetX == -1 && pMover->UtargetY == -1)
		return false;
	else
		return true;
}

/**
 * Return an actor's walk ticket.
 */
int GetWalkNumber(PMOVER pMover) {
	return pMover->walkNumber;
}

/**
 * GetMoverId
 */
int GetMoverId(PMOVER pMover) {
	return pMover->actorID;
}

/**
 * Sets the mover Z position
 */
void SetMoverZ(PMOVER pMover, int y, uint32 zFactor) {
	if (!pMover->bHidden) {
		if (MoverIsSWalking(pMover) && pMover->zOverride != -1) {
			// Special for SWalk()
			MultiSetZPosition(pMover->actorObj, (pMover->zOverride << ZSHIFT) + y);
		} else {
			// Normal case
			MultiSetZPosition(pMover->actorObj, (zFactor << ZSHIFT) + y);
		}
	}
}

void SetMoverZoverride(PMOVER pMover, uint32 zFactor) {
	pMover->zOverride = zFactor;
}

/**
 * UnHideMover
 */
void UnHideMover(PMOVER pMover) {
	assert(pMover); // unHiding null moving actor

	if (!TinselV2 || pMover->bHidden) {
		pMover->bHidden = false;

		// Make visible on the screen
		if (pMover->actorObj) {
			// If no path, just use first path in the scene
			if (pMover->hCpath != NOPOLY)
				SetMoverZ(pMover, pMover->objY, GetPolyZfactor(pMover->hCpath));
			else
				SetMoverZ(pMover, pMover->objY, GetPolyZfactor(FirstPathPoly()));
		}
	}
}

/**
 * Clear everything out at actor start-up time.
 */
static void InitMover(PMOVER pMover) {
	pMover->bActive = false;
	pMover->actorObj = NULL;
	pMover->objX = pMover->objY = 0;

	pMover->hRpath = NOPOLY;

	pMover->targetX = pMover->targetY = -1;
	pMover->ItargetX = pMover->ItargetY = -1;
	pMover->hIpath = NOPOLY;
	pMover->UtargetX = pMover->UtargetY = -1;
	pMover->hUpath = NOPOLY;
	pMover->hCpath = NOPOLY;

	pMover->over = false;
	pMover->InDifficulty = NO_PROB;

	pMover->hFnpath = NOPOLY;
	pMover->npstatus = NOT_IN;
	pMover->line = 0;

	pMover->Tline = 0;

	if (pMover->direction != FORWARD && pMover->direction != AWAY
			&& pMover->direction != LEFTREEL && pMover->direction != RIGHTREEL)
		pMover->direction = FORWARD;

	if (pMover->scale < 0 || pMover->scale > TOTAL_SCALES)
		pMover->scale = 1;

	pMover->brightness = BOGUS_BRIGHTNESS;	// Force initial setup

	pMover->bNoPath = false;
	pMover->bIgPath = false;
	pMover->bHidden = false;	// 20/2/95
	pMover->bStop = false;

	pMover->walkNumber= 0;
	pMover->stepCount = 0;

	pMover->bWalkReel = false;
	pMover->bSpecReel = false;
	pMover->hLastFilm = 0;
	pMover->hPushedFilm = 0;

	pMover->bInEffect = false;

	pMover->walkedFromX = pMover->walkedFromY = 0;
}

/**
 * Get it into our heads that there's nothing doing.
 * Called at the end of a scene.
 */
void DropMovers() {
	for (int i = 0; i < MAX_MOVERS; i++)
		InitMover(&g_Movers[i]);
}


/**
 * Reposition a moving actor.
 */
void PositionMover(PMOVER pMover, int x, int y) {
	int	z;
	int	node;
	HPOLYGON hPath;

	assert(pMover); // Moving null moving actor
	assert(pMover->actorObj);

	pMover->objX = x;
	pMover->objY = y;
	MultiSetAniXY(pMover->actorObj, x, y);

	hPath = InPolygon(x, y, PATH);
	if (hPath != NOPOLY) {
		pMover->hCpath = hPath;
		if (PolySubtype(hPath) == NODE) {
			node = NearestNodeWithin(hPath, x, y);
			getNpathNode(hPath, node, &pMover->objX, &pMover->objY);
			pMover->hFnpath = hPath;
			pMover->line = node;
			pMover->npstatus = GOING_UP;
		} else {
			pMover->hFnpath = NOPOLY;
			pMover->npstatus = NOT_IN;
		}

		z = GetScale(hPath, pMover->objY);
		pMover->scale = z;
		SetMoverStanding(pMover);
	} else {
		pMover->bNoPath = true;

		pMover->hFnpath = NOPOLY;	// Ain't in one
		pMover->npstatus = NOT_IN;

		// Ensure legal reel and scale
		if (pMover->direction < 0 || pMover->direction > 3)
			pMover->direction = FORWARD;
		if (pMover->scale < 0 || pMover->scale > TOTAL_SCALES)
			pMover->scale = 1;
	}
}

/**
 * Get position of a moving actor.
 */
void GetMoverPosition(PMOVER pMover, int *paniX, int *paniY) {
	assert(pMover); // Getting null moving actor's position

	if (pMover->actorObj != NULL)
		GetAniPosition(pMover->actorObj, paniX, paniY);
	else {
		*paniX = 0;
		*paniY = 0;
	}
}

/**
 * Moving actor's mid-top position.
 */
void GetMoverMidTop(PMOVER pMover, int *aniX, int *aniY) {
	assert(pMover); // Getting null moving actor's mid-top position
	assert(pMover->actorObj); // Getting null moving actor's mid-top position

	*aniX = (MultiLeftmost(pMover->actorObj) + MultiRightmost(pMover->actorObj)) / 2;
	*aniY = MultiHighest(pMover->actorObj);
}

/**
 * Moving actor's left-most co-ordinate.
 */
int GetMoverLeft(PMOVER pMover) {
	assert(pMover); // Getting null moving actor's leftmost position
	assert(pMover->actorObj); // Getting null moving actor's leftmost position

	return MultiLeftmost(pMover->actorObj);
}

/**
 * Moving actor's right-most co-ordinate.
 */
int GetMoverRight(PMOVER pMover) {
	assert(pMover); // Getting null moving actor's rightmost position
	assert(pMover->actorObj); // Getting null moving actor's rightmost position

	return MultiRightmost(pMover->actorObj);
}

/**
 * Moving actor's top co-ordinate.
 */
int GetMoverTop(PMOVER pMover) {
	assert(pMover); // Getting null moving actor's topmost position
	assert(pMover->actorObj); // Getting null moving actor's topmost position

	return MultiHighest(pMover->actorObj);
}

/**
 * Moving actor's bottom co-ordinate.
 */
int GetMoverBottom(PMOVER pMover) {
	assert(pMover); // Getting null moving actor's bottommost position
	assert(pMover->actorObj); // Getting null moving actor's bottommost position

	return MultiLowest(pMover->actorObj);
}

/**
 * See if moving actor is stood within a polygon.
 */
bool MoverIsInPolygon(PMOVER pMover, HPOLYGON hp) {
	assert(pMover); // Checking if null moving actor is in polygon
	assert(pMover->actorObj); // Checking if null moving actor is in polygon

	int aniX, aniY;
	GetAniPosition(pMover->actorObj, &aniX, &aniY);

	return IsInPolygon(aniX, aniY, hp);
}

/**
 * Change which reel is playing for a moving actor.
 */
void AlterMover(PMOVER pMover, SCNHANDLE film, AR_FUNCTION fn) {
	const FILM *pfilm;

	assert(pMover->actorObj); // Altering null moving actor's animation script

	if (fn == AR_POPREEL) {
		// Use the saved film
		film = pMover->hPushedFilm;
	}
	if (fn == AR_PUSHREEL) {
		// Save the one we're replacing
		pMover->hPushedFilm = (pMover->bSpecReel) ? pMover->hLastFilm : 0;
	}

	if (film == 0) {
		if (pMover->bSpecReel) {
			// Revert to 'normal' actor
			SetMoverWalkReel(pMover, pMover->direction, pMover->scale, true);
			pMover->bSpecReel = false;
		}
	} else {
		// Remember this one in case the actor talks
		pMover->hLastFilm = film;

		pfilm = (const FILM *)LockMem(film);
		assert(pfilm != NULL);

		InitStepAnimScript(&pMover->actorAnim, pMover->actorObj, FROM_LE_32(pfilm->reels[0].script), ONE_SECOND / FROM_LE_32(pfilm->frate));
		if (!TinselV2)
			pMover->stepCount = 0;

		// If no path, just use first path in the scene
		if (pMover->hCpath != NOPOLY)
			SetMoverZ(pMover, pMover->objY, GetPolyZfactor(pMover->hCpath));
		else
			SetMoverZ(pMover, pMover->objY, GetPolyZfactor(FirstPathPoly()));

		if (fn == AR_WALKREEL) {
			pMover->bSpecReel = false;
			pMover->bWalkReel = true;
		} else {
			pMover->bSpecReel = true;
			pMover->bWalkReel = false;

#ifdef DEBUG
			assert(StepAnimScript(&pMover->actorAnim) != ScriptFinished); // Actor reel has finished!
#else
			StepAnimScript(&pMover->actorAnim);	// 04/01/95
#endif
		}

		// Hang on, we may not want him yet! 04/01/95
		if (pMover->bHidden)
			MultiSetZPosition(pMover->actorObj, -1);
	}
}

/**
 * Return the actor's direction.
 */
DIRECTION GetMoverDirection(PMOVER pMover) {
	return pMover->direction;
}

/**
 * Return the actor's scale.
 */
int GetMoverScale(PMOVER pMover) {
	return pMover->scale;
}

/**
 * Point actor in specified derection
 */
void SetMoverDirection(PMOVER pMover, DIRECTION dirn) {
	pMover->direction = dirn;
}

/**
 * Get actor to adopt its appropriate standing reel.
 */
void SetMoverStanding(PMOVER pMover) {
	assert(pMover->actorObj);
	AlterMover(pMover, pMover->standReels[pMover->scale - 1][pMover->direction], AR_NORMAL);
}

/**
 * Get actor to adopt its appropriate walking reel.
 */
void SetMoverWalkReel(PMOVER pMover, DIRECTION reel, int scale, bool force) {
	SCNHANDLE	whichReel;
	const FILM *pfilm;

	// Kill off any play that may be going on for this actor
	// and restore the real actor
	storeActorReel(pMover->actorID, NULL, 0, NULL, 0, 0, 0);
	UnHideMover(pMover);

	// Don't do it if using a special walk reel
	if (pMover->bWalkReel)
		return;

	if (force || pMover->scale != scale || pMover->direction != reel) {
		assert(reel >= 0 && reel <= 3 && scale > 0 && scale <= TOTAL_SCALES); // out of range scale or reel

		// If scale change and both are regular scales
		// and there's a scaling reel in the right direction
		if (pMover->scale != scale
				&& scale <= NUM_MAINSCALES && pMover->scale <= NUM_MAINSCALES
				&& (whichReel = ScalingReel(pMover->actorID, pMover->scale, scale, reel)) != 0) {
//			error("Cripes");
			;	// Use what is now in 'whichReel'
		} else {
			whichReel = pMover->walkReels[scale-1][reel];
			assert(whichReel); // no reel
		}

		pfilm = (const FILM *)LockMem(whichReel);
		assert(pfilm != NULL); // no film

		InitStepAnimScript(&pMover->actorAnim, pMover->actorObj, FROM_LE_32(pfilm->reels[0].script), 1);

		// Synchronised walking reels
		assert(pMover->stepCount >= 0);
		SkipFrames(&pMover->actorAnim, pMover->stepCount);

		pMover->scale = scale;
		pMover->direction = reel;
	}
}

/**
 * Sort some stuff out at actor start-up time.
 */
static void InitialPathChecks(PMOVER pMover, int xpos, int ypos) {
	HPOLYGON hPath;
	int	node;
	int	z;

	pMover->objX = xpos;
	pMover->objY = ypos;

	/*--------------------------------------
	| If Actor is in a follow nodes path,	|
	| position it at the nearest node.	|
	 --------------------------------------*/
	hPath = InPolygon(xpos, ypos, PATH);

	if (hPath != NOPOLY) {
		pMover->hCpath = hPath;
		if (PolySubtype(hPath) == NODE) {
			node = NearestNodeWithin(hPath, xpos, ypos);
			getNpathNode(hPath, node, &pMover->objX, &pMover->objY);
			pMover->hFnpath = hPath;
			pMover->line = node;
			pMover->npstatus = GOING_UP;
		}

		z = GetScale(hPath, pMover->objY);
	} else {
		pMover->bNoPath = true;

		z = GetScale(FirstPathPoly(), pMover->objY);
	}
	SetMoverWalkReel(pMover, FORWARD, z, false);
}

static void MoverProcessHelper(int X, int Y, int id, PMOVER pMover) {
	const FILM *pfilm;
	const MULTI_INIT *pmi;
	const FRAME *pFrame;
	IMAGE *pim;


	assert(BgPal()); // Can't start actor without a background palette
	assert(pMover->walkReels[0][FORWARD]); // Starting actor process without walk reels

	InitMover(pMover);
	InitialPathChecks(pMover, X, Y);

	pfilm = (const FILM *)LockMem(pMover->walkReels[0][FORWARD]);
	pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(pfilm->reels[0].mobj));

//---
	pFrame = (const FRAME *)LockMem(FROM_LE_32(pmi->hMulFrame));

	// get pointer to image
	pim = (IMAGE *)LockMem(READ_LE_UINT32(pFrame));	// handle to image
	pim->hImgPal = TO_LE_32(BgPal());
//---
	pMover->actorObj = MultiInitObject(pmi);

/**/	assert(pMover->actorID == id);
	pMover->actorID = id;

	// add it to display list
	MultiInsertObject(GetPlayfieldList(FIELD_WORLD), pMover->actorObj);
	storeActorReel(id, NULL, 0, pMover->actorObj, 0, 0, 0);

	InitStepAnimScript(&pMover->actorAnim, pMover->actorObj, FROM_LE_32(pfilm->reels[0].script), ONE_SECOND / FROM_LE_32(pfilm->frate));
	pMover->stepCount = 0;

	MultiSetAniXY(pMover->actorObj, pMover->objX, pMover->objY);

	// If no path, just use first path in the scene
	if (pMover->hCpath != NOPOLY)
		SetMoverZ(pMover, pMover->objY, GetPolyZfactor(pMover->hCpath));
	else
		SetMoverZ(pMover, pMover->objY, GetPolyZfactor(FirstPathPoly()));

	// Make him the right size
	SetMoverStanding(pMover);

//**** if added 18/11/94, am
	if (X != MAGICX && Y != MAGICY) {
		HideMover(pMover, 0);		// Allows a play to come in before this appears
		pMover->bHidden = false;	// ...but don't stay hidden
	}

	pMover->bActive = true;
}

/**
 * Moving actor process - 1 per moving actor in current scene.
 */
void T1MoverProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	const PMOVER pActor = *(const PMOVER *)param;

	CORO_BEGIN_CODE(_ctx);

	while (1) {
		if (pActor->bSpecReel) {
			if (!pActor->bHidden)
#ifdef DEBUG
			assert(StepAnimScript(&pActor->actorAnim) != ScriptFinished); // Actor reel has finished!
#else
			StepAnimScript(&pActor->actorAnim);
#endif
		} else
			DoMoveActor(pActor);

		CORO_SLEEP(1);		// allow rescheduling

	}

	CORO_END_CODE;
}

/**
 * Tinsel 2 Moving actor process
 * - 1 per moving actor in current scene.
 */
void T2MoverProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	// Get the co-ordinates - copied to process when it was created
	const MAINIT *rpos = (const MAINIT *)param;
	PMOVER pMover = rpos->pMover;
	int i;
	FILM *pFilm;
	PMULTI_INIT pmi;

	CORO_BEGIN_CODE(_ctx);

	for (i = 0; i < TOTAL_SCALES; i++) {
		if (pMover->walkReels[i][FORWARD])
			break;
	}
	assert(i < TOTAL_SCALES);

	InitMover(pMover);
	InitialPathChecks(pMover, rpos->X, rpos->Y);

	pFilm = (FILM *)LockMem(pMover->walkReels[i][FORWARD]);	// Any old reel
	pmi = (PMULTI_INIT)LockMem(FROM_LE_32(pFilm->reels[0].mobj));

	// Poke in the background palette
	PokeInPalette(pmi);

	pMover->actorObj = MultiInitObject(pmi);
	pMover->actorID = pMover->actorID;
	pMover->bActive = true;

	// add it to display list
	MultiInsertObject( GetPlayfieldList(FIELD_WORLD), pMover->actorObj );

	InitStepAnimScript(&pMover->actorAnim, pMover->actorObj, pFilm->reels[0].script, ONE_SECOND/pFilm->frate);
	pMover->stepCount = 0;

	MultiSetAniXY(pMover->actorObj, pMover->objX, pMover->objY);

	// If no path, just use first path in the scene
	if (pMover->hCpath != NOPOLY)
		SetMoverZ(pMover, pMover->objY, GetPolyZfactor(pMover->hCpath));
	else
		SetMoverZ(pMover, pMover->objY, GetPolyZfactor(FirstPathPoly()));

	// Make him the right size
	SetMoverStanding(pMover);

	HideMover(pMover);		// Allows a play to come in before this appears
	pMover->bHidden = false;	// ...but don't stay hidden

	for (;;) {
		if (pMover->bSpecReel) {
			if (!pMover->bHidden)
				StepAnimScript(&pMover->actorAnim);
		} else
			DoMoveActor(pMover);

		CheckBrightness(pMover);

		CORO_SLEEP(1);
	}

	CORO_END_CODE;
}

/**
 * Creates a handling process for a moving actor
 */
void MoverProcessCreate(int X, int Y, int id, PMOVER pMover) {
	if (TinselV2) {
		MAINIT iStruct;
		iStruct.X = X;
		iStruct.Y = Y;
		iStruct.pMover = pMover;

		CoroScheduler.createProcess(PID_MOVER, T2MoverProcess, &iStruct, sizeof(MAINIT));
	} else {
		MoverProcessHelper(X, Y, id, pMover);
		pMover->pProc = CoroScheduler.createProcess(PID_MOVER, T1MoverProcess, &pMover, sizeof(PMOVER));
	}
}

/**
 * Check for moving actor collision.
 */
PMOVER InMoverBlock(PMOVER pMover, int x, int y) {
	int	caX;		// Calling actor's pos'n
	int	caL, caR;	// Calling actor's left and right
	int	taX, taY;	// Test actor's pos'n
	int	taL, taR;	// Test actor's left and right

	caX = pMover->objX;
	if (pMover->hFnpath != NOPOLY || GetNoBlocking())
		return NULL;

	caL = GetMoverLeft(pMover) + x - caX;
	caR = GetMoverRight(pMover) + x - caX;

	for (int i = 0; i < MAX_MOVERS; i++) {
		if (pMover == &g_Movers[i] ||
				(TinselV2 && (g_Movers[i].actorObj == NULL)) ||
				(!TinselV2 && !g_Movers[i].bActive))
			continue;

		// At around the same height?
		GetMoverPosition(&g_Movers[i], &taX, &taY);
		if (g_Movers[i].hFnpath != NOPOLY)
			continue;

		if (ABS(y - taY) > 2)	// 2 was 8
			continue;

		// To the left?
		taL = GetMoverLeft(&g_Movers[i]);
		if (caR <= taL)
			continue;

		// To the right?
		taR = GetMoverRight(&g_Movers[i]);
		if (caL >= taR)
			continue;

		return &g_Movers[i];
	}
	return NULL;
}

/**
 * Copies key information for savescn.c to store away.
 */
void SaveMovers(SAVED_MOVER *sMoverInfo) {
	for (int i = 0; i < MAX_MOVERS; i++) {
		sMoverInfo[i].bActive = !TinselV2 ? g_Movers[i].bActive : g_Movers[i].actorObj != NULL;
		sMoverInfo[i].actorID	= g_Movers[i].actorID;
		sMoverInfo[i].objX	= g_Movers[i].objX;
		sMoverInfo[i].objY	= g_Movers[i].objY;
		sMoverInfo[i].hLastfilm	= g_Movers[i].hLastFilm;

		if (TinselV2) {
			sMoverInfo[i].bHidden = g_Movers[i].bHidden;
			sMoverInfo[i].brightness = g_Movers[i].brightness;
			sMoverInfo[i].startColor = g_Movers[i].startColor;
			sMoverInfo[i].paletteLength = g_Movers[i].paletteLength;
		}

		memcpy(sMoverInfo[i].walkReels, g_Movers[i].walkReels, TOTAL_SCALES * 4 * sizeof(SCNHANDLE));
		memcpy(sMoverInfo[i].standReels, g_Movers[i].standReels, TOTAL_SCALES * 4 * sizeof(SCNHANDLE));
		memcpy(sMoverInfo[i].talkReels, g_Movers[i].talkReels, TOTAL_SCALES * 4 * sizeof(SCNHANDLE));
	}
}

void RestoreAuxScales(SAVED_MOVER *sMoverInfo) {
	for (int i = 0; i < MAX_MOVERS; i++) {
		if (TinselV2)
			g_Movers[i].actorID = sMoverInfo[i].actorID;

		memcpy(g_Movers[i].walkReels, sMoverInfo[i].walkReels, TOTAL_SCALES * 4 * sizeof(SCNHANDLE));
		memcpy(g_Movers[i].standReels, sMoverInfo[i].standReels, TOTAL_SCALES * 4 * sizeof(SCNHANDLE));
		memcpy(g_Movers[i].talkReels, sMoverInfo[i].talkReels, TOTAL_SCALES * 4 * sizeof(SCNHANDLE));
	}
}


PMOVER NextMover(PMOVER pMover) {
	int next;

	if (pMover == NULL)
		next = 0;
	else
		next = pMover - g_Movers + 1;

	if (g_Movers[next].actorID)
		return &g_Movers[next];
	else
		return NULL;
}

void StopMover(PMOVER pMover) {
	pMover->bStop = true;
	DoMoveActor(pMover);
}

} // End of namespace Tinsel
