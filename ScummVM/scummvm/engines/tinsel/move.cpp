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
 * Handles walking and use of the path system.
 *
 * Contains the dodgiest code in the whole system.
 */

#include "tinsel/actors.h"
#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/graphics.h"
#include "tinsel/move.h"
#include "tinsel/multiobj.h"	// multi-part object defintions etc.
#include "tinsel/object.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/scroll.h"
#include "tinsel/tinlib.h"	// For Stand()
#include "tinsel/tinsel.h"

namespace Tinsel {

//----------------- DEVELOPMENT OPTIONS --------------------

#define SLOW_RINCE_DOWN		0

//----------------- EXTERNAL FUNCTIONS ---------------------

// in POLYGONS.C
// Deliberatley defined here, and not in polygons.h
HPOLYGON InitExtraBlock(PMOVER ca, PMOVER ta);

//----------------- LOCAL DEFINES --------------------

#define XMDIST	(TinselV2 ? 6 : 4)
#define XHMDIST	(TinselV2 ? 3 : 2)
#define YMDIST	(TinselV2 ? 3 : 2)
#define YHMDIST	(TinselV2 ? 3 : 2)

#define XTHERE		1
#define XRESTRICT	2
#define YTHERE		4
#define YRESTRICT	8
#define STUCK		16

#define LEAVING_PATH	0x100
#define ENTERING_BLOCK	0x200
#define ENTERING_MBLOCK	0x400

#define ALL_SORTED	1
#define NOT_SORTED	0

#define STEPS_MAX (TinselV2 ? 12 : 6)

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

#if SLOW_RINCE_DOWN
static int g_Interlude = 0;	// For slowing down walking, for testing
static int g_BogusVar = 0;	// For slowing down walking, for testing
#endif

static int32 g_DefaultRefer = 0;
static int g_lastLeadXdest = 0, g_lastLeadYdest = 0;

static int g_hSlowVar = 0;	// used by MoveActor()


//----------------- FORWARD REFERENCES --------------------

static void NewCoOrdinates(int fromx, int fromy, int *targetX, int *targetY,
			int *newx, int *newy, int *s1, int *s2, HPOLYGON *hS2p,
			bool bOver, bool bBodge,
			PMOVER pActor, PMOVER *collisionActor = 0);


#if SLOW_RINCE_DOWN
/**
 * AddInterlude
 */

void AddInterlude(int n) {
	g_Interlude += n;
	if (g_Interlude < 0)
		g_Interlude = 0;
}
#endif

/**
 * Given (x, y) of a click within a path polygon, checks that the
 * co-ordinates are not within a blocking polygon. If it is not, the
 * destination is the click point, otherwise tries to find a legal point
 * below or above the click point.
 * Returns:
 *	NOT_SORTED - if a destination is worked out (movement required)
 *	ALL_SORTED - no destination found (so no movement required)
 */
static int ClickedOnPath(int clickX, int clickY, int *ptgtX, int *ptgtY) {
	int Loffset, Toffset;
	int i;

	/*--------------------------------------
	 Clicked within a path,
	 go to where requested unless blocked.
	 --------------------------------------*/
	if (InPolygon(clickX, clickY, BLOCK) == NOPOLY) {
		// Not in a blocking polygon - go to where requested.
		*ptgtX = clickX;
		*ptgtY = clickY;
	} else {
		/*------------------------------------------------------
		 In a Blocking polygon - try searching down and up.
		 If still nowhere (for now) give up!
		 ------------------------------------------------------*/
		PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

		for (i = clickY+1; i < SCREEN_HEIGHT + Toffset; i++) {
			// Don't leave the path system
			if (InPolygon(clickX, i, PATH) == NOPOLY) {
				i = SCREEN_HEIGHT;
				break;
			}
			if (InPolygon(clickX, i, BLOCK) == NOPOLY) {
				*ptgtX = clickX;
				*ptgtY = i;
				break;
			}
		}
		if (i == SCREEN_HEIGHT) {
			for (i = clickY-1; i >= Toffset; i--) {
				// Don't leave the path system
				if (InPolygon(clickX, i, PATH) == NOPOLY) {
					i = -1;
					break;
				}
				if (InPolygon(clickX, i, BLOCK) == NOPOLY) {
					*ptgtX = clickX;
					*ptgtY = i;
					break;
				}
			}
		}
		if (i < 0) {
			return ALL_SORTED;
		}
	}
	return NOT_SORTED;
}

/**
 * Given (x, y) of a click within a referral polygon, works out the
 * destination according to the referral type.
 * Returns:
 *   NOT_SORTED - if a destination is worked out (movement required)
 *   ALL_SORTED - no destination found (so no movement required)
 */
static int ClickedOnRefer(HPOLYGON hRefpoly, int clickX, int clickY, int *ptgtX, int *ptgtY) {
	int	i;
	int	end;		// Extreme of the scene
	int	Loffset, Toffset;

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
	*ptgtX = *ptgtY = -1;

	switch (PolySubtype(hRefpoly)) {
	case REF_POINT:				// Go to specified node
		GetPolyNode(hRefpoly, ptgtX, ptgtY);
		assert(InPolygon(*ptgtX, *ptgtY, PATH) != NOPOLY); // POINT Referral to illegal point
		break;

	case REF_DOWN:				// Search downwards
		end = BgHeight();
		for (i = clickY+1; i < end; i++)
			if (InPolygon(clickX, i, PATH) != NOPOLY
					&& InPolygon(clickX, i, BLOCK) == NOPOLY) {
				*ptgtX = clickX;
				*ptgtY = i;
				break;
			}
		break;

	case REF_UP:				// Search upwards
		for (i = clickY-1; i >= 0; i--)
			if (InPolygon(clickX, i, PATH) != NOPOLY
					&& InPolygon(clickX, i, BLOCK) == NOPOLY) {
				*ptgtX = clickX;
				*ptgtY = i;
				break;
			}
		break;

	case REF_RIGHT:				// Search to the right
		end = BgWidth();
		for (i = clickX+1; i < end; i++)
			if (InPolygon(i, clickY, PATH) != NOPOLY
			&& InPolygon(i, clickY, BLOCK) == NOPOLY) {
				*ptgtX = i;
				*ptgtY = clickY;
				break;
			}
		break;

	case REF_LEFT:				// Search to the left
		for (i = clickX-1; i >= 0; i--)
			if (InPolygon(i, clickY, PATH) != NOPOLY
			&& InPolygon(i, clickY, BLOCK) == NOPOLY) {
				*ptgtX = i;
				*ptgtY = clickY;
				break;
			}
		break;
	}
	if (*ptgtX != -1 && *ptgtY != -1) {
		return NOT_SORTED;
	} else
		return ALL_SORTED;
}

/**
 * Given (x, y) of a click, works out the destination according to the
 * default referral type.
 * Returns:
 *   NOT_SORTED - if a destination is worked out (movement required)
 *   ALL_SORTED - no destination found (so no movement required)
 */
static int ClickedOnNothing(int clickX, int clickY, int *ptgtX, int *ptgtY) {
	int	i;
	int	end;		// Extreme of the scene
	int	Loffset, Toffset;

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	switch (g_DefaultRefer) {
	case REF_DEFAULT:
		// Try searching down and up (onscreen).
		for (i = clickY+1; i < SCREEN_HEIGHT+Toffset; i++)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		for (i = clickY-1; i >= Toffset; i--)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		// Try searching down and up (offscreen).
		end = BgHeight();
		for (i = clickY+1; i < end; i++)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		for (i = clickY-1; i >= 0; i--)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		break;

	case REF_UP:
		for (i = clickY-1; i >= 0; i--)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		break;

	case REF_DOWN:
		end = BgHeight();
		for (i = clickY+1; i < end; i++)
			if (InPolygon(clickX, i, PATH) != NOPOLY) {
				return ClickedOnPath(clickX, i, ptgtX, ptgtY);
			}
		break;

	case REF_LEFT:
		for (i = clickX-1; i >= 0; i--)
			if (InPolygon(i, clickY, PATH) != NOPOLY) {
				return ClickedOnPath(i, clickY, ptgtX, ptgtY);
			}
		break;

	case REF_RIGHT:
		end = BgWidth();
		for (i = clickX + 1; i < end; i++)
			if (InPolygon(i, clickY, PATH) != NOPOLY) {
				return ClickedOnPath(i, clickY, ptgtX, ptgtY);
			}
		break;
	}

	// Going nowhere!
	return ALL_SORTED;
}

/**
 * Given (x, y) of the click, ascertains whether the click is within a
 * path, within a referral poly, or niether. The appropriate function
 * then gets called to give us a revised destination.
 * Returns:
 *   NOT_SORTED - if a destination is worked out (movement required)
 *   ALL_SORTED - no destination found (so no movement required)
 */
static int WorkOutDestination(int clickX, int clickY, int *ptgtX, int *ptgtY) {
	HPOLYGON hPoly;

	/*--------------------------------------
	 Clicked within a path?
	 if not, within a referral poly?
	 if not, try and sort something out.
	---------------------------------------*/
	if (InPolygon(clickX, clickY, PATH) != NOPOLY) {
		return ClickedOnPath(clickX, clickY, ptgtX, ptgtY);
	} else if ((hPoly = InPolygon(clickX, clickY, REFER)) != NOPOLY) {
		return ClickedOnRefer(hPoly, clickX, clickY, ptgtX, ptgtY);
	} else {
		return ClickedOnNothing(clickX, clickY, ptgtX, ptgtY);
	}
}

/**
 * Work out which reel to adopt for a section of movement.
 */
DIRECTION GetDirection(int fromx, int fromy, int tox, int toy, DIRECTION lastreel,
							  HPOLYGON hPath, YBIAS yBias) {
	int	xchange = 0, ychange = 0;
	enum {X_NONE, X_LEFT, X_RIGHT, X_NO} xdir;
	enum {Y_NONE, Y_UP, Y_DOWN, Y_NO} ydir;

	DIRECTION	reel = lastreel;	// Leave alone if can't decide

	/*
	 * Determine size and direction of X movement.
	 * i.e. left, right, none or not allowed.
	 */
	if (hPath != NOPOLY && (GetPolyReelType(hPath) == REEL_VERT))
		xdir = X_NO;
	else if (tox == -1)
		xdir = X_NONE;
	else {
		xchange = tox - fromx;
		if (xchange > 0)
			xdir = X_RIGHT;
		else if (xchange < 0) {
			xchange = -xchange;
			xdir = X_LEFT;
		} else
			xdir = X_NONE;
	}

	/*
	 * Determine size and direction of Y movement.
	 * i.e. up, down, none or not allowed.
	 */
	if (hPath != NOPOLY && (GetPolyReelType(hPath) == REEL_HORIZ))
		ydir = Y_NO;
	else if (toy == -1)
		ydir = Y_NONE;
	else {
		ychange = toy - fromy;
		if (ychange > 0)
			ydir = Y_DOWN;
		else if (ychange < 0) {
			ychange = -ychange;
			ydir = Y_UP;
		} else
			ydir = Y_NONE;
	}

	/*
	 * Some adjustment to allow for different x and y pixell sizes.
	 */
	switch (yBias) {
	case YB_X2:
		ychange += ychange;		// Double y distance to cover
		break;

	case YB_X1_5:
		ychange += ychange / 2;	// Double y distance to cover
		break;
	}

	/*
	 * Determine which reel to use.
	 */
	if (xdir == X_NO) {
		// Forced to be FORWARD or AWAY
		switch (ydir) {
		case Y_DOWN:
			reel = FORWARD;
			break;
		case Y_UP:
			reel = AWAY;
			break;
		default:
			if (reel != AWAY)	// No gratuitous turn
				reel = FORWARD;
			break;
		}
	} else if (ydir == Y_NO) {
		// Forced to be LEFTREEL or RIGHTREEL
		switch (xdir) {
		case X_LEFT:
			reel = LEFTREEL;
			break;
		case X_RIGHT:
			reel = RIGHTREEL;
			break;
		default:
			if (reel != LEFTREEL)	// No gratuitous turn
				reel = RIGHTREEL;
			break;
		}
	} else if (xdir != X_NONE || ydir != Y_NONE) {
		if (xdir == X_NONE)
			reel = (ydir == Y_DOWN) ? FORWARD : AWAY;
		else if (ydir == Y_NONE)
			reel = (xdir == X_LEFT) ? LEFTREEL : RIGHTREEL;
		else {
			bool DontBother = false;

			if (xchange <= 4 && ychange <= 4) {
				switch (reel) {
				case LEFTREEL:
					if (xdir == X_LEFT)
						DontBother = true;
					break;
				case RIGHTREEL:
					if (xdir == X_RIGHT)
						DontBother = true;
					break;
				case FORWARD:
					if (ydir == Y_DOWN)
						DontBother = true;
					break;
				case AWAY:
					if (ydir == Y_UP)
						DontBother = true;
					break;
				}
			}
			if (!DontBother) {
				if (xchange > ychange)
					reel = (xdir == X_LEFT) ? LEFTREEL : RIGHTREEL;
				else
					reel = (ydir == Y_DOWN) ? FORWARD : AWAY;
			}
		}
	}
	return reel;
}

/**
 * Haven't moved, look towards the cursor.
 */
static void GotThereWithoutMoving(PMOVER pActor) {
	int	curX, curY;
	DIRECTION	reel;

	if (!pActor->bSpecReel) {
		GetCursorXYNoWait(&curX, &curY, true);

		reel = GetDirection(pActor->objX, pActor->objY, curX, curY, pActor->direction, pActor->hCpath);

		if (reel != pActor->direction)
			SetMoverWalkReel(pActor, reel, pActor->scale, false);
	}
}

/**
 * Arrived at final destination.
 */
static void GotThere(PMOVER pMover) {
	pMover->targetX = pMover->targetY = -1;		// 4/1/95
	pMover->ItargetX = pMover->ItargetY = -1;
	pMover->UtargetX = pMover->UtargetY = -1;

	// Perhaps we have'nt moved.
	if (pMover->objX == (int)pMover->walkedFromX && pMover->objY == (int)pMover->walkedFromY) {
		// Got there without moving
		if (!TinselV2)
			GotThereWithoutMoving(pMover);
		else if (!pMover->bSpecReel) {
			// No tag reel, look at cursor
			int curX, curY;
			DIRECTION direction;

			GetCursorXY(&curX, &curY, true);
			direction = GetDirection(pMover->objX, pMover->objY,
						curX, curY,
						pMover->direction,
						pMover->hCpath,
						YB_X2);

			if (direction != pMover->direction)
				SetMoverWalkReel(pMover, direction, pMover->scale, false);
		}
	}

	if (!TinselV2)
		ReTagActor(pMover->actorID);	// Tag allowed while stationary

	SetMoverStanding(pMover);
	pMover->bMoving = false;

	if (TinselV2 && pMover->bIgPath && pMover->zOverride != -1
			&&  InPolygon(pMover->objX, pMover->objY, PATH) == NOPOLY)
		// New feature for end-of-scene walk-outs
		SetMoverZ(pMover, pMover->objY, pMover->zOverride);
}

enum cgt { GT_NOTL, GT_NOTB, GT_NOT2, GT_OK, GT_MAY };

/**
 * Can we get straight there?
 */
static cgt CanGetThere(PMOVER pActor, int tx, int ty) {
	int s1, s2;		// s2 not used here!
	HPOLYGON hS2p;		// nor is s2p!
	int nextx, nexty;

	int targetX = tx;
	int targetY = ty;		// Ultimate destination
	int x = pActor->objX;
	int y = pActor->objY;		// Present position

	while (targetX != -1 || targetY != -1) {
		NewCoOrdinates(x, y, &targetX, &targetY, &nextx, &nexty,
				&s1, &s2, &hS2p, pActor->over, false, pActor);

		if (s1 == (XTHERE | YTHERE)) {
			return GT_OK;	// Can get there directly.
		} else if (s1 == (XTHERE | YRESTRICT) || s1 == (YTHERE | XRESTRICT)) {
			return GT_MAY;	// Can't get there directly.
		} else if (s1 & STUCK) {
			if (s2 == LEAVING_PATH)
				return GT_NOTL;	// Can't get there.
			else
				return GT_NOTB;	// Can't get there.
		} else if (x == nextx && y == nexty) {
			return GT_NOT2;	// Can't get there.
		}
		x = nextx;
		y = nexty;
	}
	return GT_MAY;
}


/**
 * Set final destination.
 */
static void SetMoverUltDest(PMOVER pActor, int x, int y) {
	pActor->UtargetX = x;
	pActor->UtargetY = y;
	pActor->hUpath = InPolygon(x, y, PATH);

	assert(pActor->hUpath != NOPOLY || pActor->bIgPath); // Invalid ultimate destination
}

/**
 * Set intermediate destination.
 *
 * If in final destination path, go straight to target.
 * If in a neighboring path to the final destination, if the target path
 * is a follow nodes path, head for the end node, otherwise head straight
 * for the target.
 * Otherwise, head towards the pseudo-center or end node of the first
 * en-route path.
 */
static void SetMoverIntDest(PMOVER pMover, int x, int y) {
	HPOLYGON hIpath, hTpath;
	int	node;

	hTpath = InPolygon(x, y, PATH);		// Target path
#ifdef DEBUG
	if (!pMover->bIgPath)
		assert(hTpath != NOPOLY); // SetMoverIntDest() - target not in path
#endif

	if (pMover->hCpath == hTpath || pMover->bIgPath
		|| IsInPolygon(pMover->objX, pMover->objY, hTpath)) {
		// In destination path - head straight for the target.
		pMover->ItargetX = x;
		pMover->ItargetY = y;
		// make damn sure that Itarget is in hIpath
		pMover->hIpath = !TinselV2 ? hTpath : InPolygon(x, y, PATH);
	} else if (IsAdjacentPath(pMover->hCpath, hTpath)) {
		// In path adjacent to target
		if (PolySubtype(hTpath) != NODE) {
			// Target path is normal - head for target.
			// Added 26/01/95, innroom
			if (CanGetThere(pMover, x, y) == GT_NOTL) {
				NearestCorner(&x, &y, pMover->hCpath, hTpath);
			}
			pMover->ItargetX = x;
			pMover->ItargetY = y;
			if (TinselV2)
				// make damn sure that Itarget is in hIpath
				pMover->hIpath = InPolygon(x, y, PATH);
		} else {
			// Target path is node - head for end node.
			node = NearestEndNode(hTpath, pMover->objX, pMover->objY);
			getNpathNode(hTpath, node, &pMover->ItargetX, &pMover->ItargetY);
			if (TinselV2)
				// make damn sure that Itarget is in hIpath
				pMover->hIpath = InPolygon(pMover->ItargetX, pMover->ItargetY, PATH);
		}
		if (!TinselV2)
			pMover->hIpath = hTpath;
	} else {
		assert(hTpath != NOPOLY); // Error 701
		hIpath = GetPathOnTheWay(pMover->hCpath, hTpath);

		if (TinselV2 && (hIpath == NOPOLY)) {
			pMover->hIpath = NOPOLY;
		} else if (hIpath != NOPOLY) {
			/* Head for an en-route path */
			if (PolySubtype(hIpath) != NODE) {
				/* En-route path is normal - head for pseudo center. */
				if (CanGetThere(pMover, x, y) == GT_OK) {
					pMover->ItargetX = x;
					pMover->ItargetY = y;
					if (TinselV2)
						// make damn sure that Itarget is in hIpath
						pMover->hIpath = InPolygon(x, y, PATH);
				} else {
					pMover->ItargetX = PolyCenterX(hIpath);
					pMover->ItargetY = PolyCenterY(hIpath);
					if (TinselV2)
						// make damn sure that Itarget is in hIpath
						pMover->hIpath = InPolygon(pMover->ItargetX, pMover->ItargetY, PATH);
				}
			} else {
				/* En-route path is node - head for end node. */
				node = NearestEndNode(hIpath, pMover->objX, pMover->objY);
				getNpathNode(hIpath, node, &pMover->ItargetX, &pMover->ItargetY);
				if (TinselV2)
					// make damn sure that Itarget is in hIpath
					pMover->hIpath = InPolygon(pMover->ItargetX, pMover->ItargetY, PATH);
			}
			if (!TinselV2)
				pMover->hIpath = hIpath;
		}
	}

	pMover->InDifficulty = NO_PROB;
}

/**
 * Set short-term destination and adopt the appropriate reel.
 */
static void SetMoverDest(PMOVER pActor, int x, int y) {
	int	scale;
	DIRECTION	reel;

	// Set the co-ordinates requested.
	pActor->targetX = x;
	pActor->targetY = y;
	pActor->InDifficulty = NO_PROB;

	reel = GetDirection(pActor->objX, pActor->objY, x, y, pActor->direction, pActor->hCpath);
	scale = GetScale(pActor->hCpath, pActor->objY);
	if (scale != pActor->scale || reel != pActor->direction) {
		SetMoverWalkReel(pActor, reel, scale, false);
	}
}

/**
 * SetNextDest
 */
static void SetNextDest(PMOVER pMover) {
	int	targetX, targetY;		// Ultimate destination
	int	x, y;				// Present position
	int	nextx, nexty;
	int	s1, lstatus = 0;
	int	s2;
	HPOLYGON	hS2p;
	int	i;
	HPOLYGON	hNpoly;
	HPOLYGON	hPath;
	int	znode;
	int	nx, ny;
	int	sx, sy;
	HPOLYGON	hEb;

	int	ss1, ss2;
	HPOLYGON shS2p;
	PMOVER collisionActor;
#if 1
	int	sTargetX, sTargetY;
#endif

	/*
	 * Desired destination (Itarget) is already set
	 */
	x = pMover->objX;		// Current position
	y = pMover->objY;
	targetX = pMover->ItargetX;	// Desired position
	targetY = pMover->ItargetY;

	/*
	 * If we're where we're headed, end it all (the moving).
	 */
//	if (x == targetX && y == targetY)
	if (ABS(x - targetX) < XMDIST && ABS(y - targetY) < YMDIST) {
		if (targetX == pMover->UtargetX && targetY == pMover->UtargetY) {
			// Desired position
			GotThere(pMover);
			return;
		} else {
			assert(pMover->bIgPath || InPolygon(pMover->UtargetX, pMover->UtargetY, PATH) != NOPOLY); // Error 5001
			SetMoverIntDest(pMover, pMover->UtargetX, pMover->UtargetY);
		}
	}

	if (pMover->bNoPath || pMover->bIgPath) {
		/* Can get there directly. */
		SetMoverDest(pMover, targetX, targetY);
		pMover->over = false;
		return;
	}

	/*----------------------------------------------------------------------
	| Some work to do here if we're in a follow nodes polygon - basically
	| head for the next node.
	----------------------------------------------------------------------*/
	hNpoly = pMover->hFnpath;		// The node path we're in (if any)
	switch (pMover->npstatus) {
	case NOT_IN:
		break;

	case ENTERING:
		znode = NearestEndNode(hNpoly, x, y);
		/* Hang on, we're probably here already! */
		if (znode) {
			pMover->npstatus = GOING_DOWN;
			pMover->line = znode-1;
			getNpathNode(hNpoly, znode - 1, &nx, &ny);
		} else {
			pMover->npstatus = GOING_UP;
			pMover->line = znode;
			getNpathNode(hNpoly, 1, &nx, &ny);
		}
		SetMoverDest(pMover, nx, ny);

		// Test for pseudo-one-node npaths
		if (numNodes(hNpoly) == 2 &&
				ABS(pMover->objX - pMover->targetX) < XMDIST &&
				ABS(pMover->objY - pMover->targetY) < YMDIST) {
			// That's enough, we're leaving
			pMover->npstatus = LEAVING;
		} else {
			// Normal situation
			pMover->over = true;
			return;
		}
		// Fall through for LEAVING

	case LEAVING:
		assert(pMover->bIgPath || InPolygon(pMover->UtargetX, pMover->UtargetY, PATH) != NOPOLY); // Error 5002
		SetMoverIntDest(pMover, pMover->UtargetX, pMover->UtargetY);
		targetX = pMover->ItargetX;	// Desired position
		targetY = pMover->ItargetY;
		break;

	case GOING_UP:
		i = pMover->line;		// The line we're on

		// Is this the final target line?
		if (i+1 == pMover->Tline && hNpoly == pMover->hUpath) {
			// The final leg of the journey
			pMover->line = i+1;
			SetMoverDest(pMover, pMover->UtargetX, pMover->UtargetY);
			pMover->over = false;
			return;
		} else {
			// Go to the next node unless we're at the last one
			i++;				// The node we're at
			if (++i < numNodes(hNpoly)) {
				getNpathNode(hNpoly, i, &nx, &ny);
				SetMoverDest(pMover, nx, ny);
				pMover->line = i-1;
				if (ABS(pMover->UtargetX - pMover->targetX) < XMDIST &&
				   ABS(pMover->UtargetY - pMover->targetY) < YMDIST)
					pMover->over = false;
				else
					pMover->over = true;
				return;
			} else {
				// Last node - we're off
				pMover->npstatus = LEAVING;
				assert(pMover->bIgPath || InPolygon(pMover->UtargetX, pMover->UtargetY, PATH) != NOPOLY); // Error 5003
				SetMoverIntDest(pMover, pMover->UtargetX, pMover->UtargetY);
				targetX = pMover->ItargetX;	// Desired position
				targetY = pMover->ItargetY;
				break;
			}
		}

	case GOING_DOWN:
		i = pMover->line;		// The line we're on and the node we're at

		// Is this the final target line?
		if (i - 1 == pMover->Tline && hNpoly == pMover->hUpath) {
			// The final leg of the journey
			SetMoverDest(pMover, pMover->UtargetX, pMover->UtargetY);
			pMover->line = i-1;
			pMover->over = false;
			return;
		} else {
			// Go to the next node unless we're at the last one
			if (--i >= 0) {
				getNpathNode(hNpoly, i, &nx, &ny);
				SetMoverDest(pMover, nx, ny);
				pMover->line--;		/* The next node to head for */
				if (ABS(pMover->UtargetX - pMover->targetX) < XMDIST &&
				   ABS(pMover->UtargetY - pMover->targetY) < YMDIST)
					pMover->over = false;
				else
					pMover->over = true;
				return;
			} else {
				// Last node - we're off
				pMover->npstatus = LEAVING;
				assert(pMover->bIgPath || InPolygon(pMover->UtargetX, pMover->UtargetY, PATH) != NOPOLY); // Error 5004
				SetMoverIntDest(pMover, pMover->UtargetX, pMover->UtargetY);
				targetX = pMover->ItargetX;	// Desired position
				targetY = pMover->ItargetY;
				break;
			}
		}
	}




	/*------------------------------------------------------
	| See if it can get there directly. There may be an
	| intermediate destination to head for.
	------------------------------------------------------*/

	while (targetX != -1 || targetY != -1) {
#if 1
		// 'push' the target
		sTargetX = targetX;
		sTargetY = targetY;
#endif
		NewCoOrdinates(x, y, &targetX, &targetY, &nextx, &nexty,
					&s1, &s2, &hS2p, pMover->over, false, pMover, &collisionActor);

		if (s1 != (XTHERE | YTHERE) && x == nextx && y == nexty) {
			ss1 = s1;
			ss2 = s2;
			shS2p = hS2p;
#if 1
			// 'pop' the target
			targetX = sTargetX;
			targetY = sTargetY;
#endif
			// Note: this aint right - targetX/Y (may) have been
			// nobbled by that last call to NewCoOrdinates()
			// Re-instating them (can) leads to oscillation
			NewCoOrdinates(x, y, &targetX, &targetY, &nextx, &nexty,
						&s1, &s2, &hS2p, pMover->over, true, pMover, &collisionActor);

			if (x == nextx && y == nexty) {
				s1 = ss1;
				s2 = ss2;
				hS2p = shS2p;
			}
		}

		if (s1 == (XTHERE | YTHERE)) {
			/* Can get there directly. */
			SetMoverDest(pMover, nextx, nexty);
			pMover->over = false;
			break;
		} else if ((s1 & STUCK) || s1 == (XRESTRICT + YRESTRICT)
		     || s1 == (XTHERE | YRESTRICT) || s1 == (YTHERE | XRESTRICT)) {
			/*-------------------------------------------------
			 Can't go any further in this direction.	   |
			 If it's because of a blocking polygon, try to do |
			 something about it.				   |
			 -------------------------------------------------*/
			if (s2 & ENTERING_BLOCK) {
				x = pMover->objX;	// Current position
				y = pMover->objY;
				// Go to the nearest corner of the blocking polygon concerned
				BlockingCorner(hS2p, &x, &y, pMover->ItargetX, pMover->ItargetY);
				SetMoverDest(pMover, x, y);
				pMover->over = false;
			} else if (s2 & ENTERING_MBLOCK) {
				if (InMoverBlock(pMover, pMover->UtargetX, pMover->UtargetY)) {
					// The best we're going to achieve
					SetMoverUltDest(pMover, x, y);
					SetMoverDest(pMover, x, y);
				} else {
					sx = pMover->objX;
					sy = pMover->objY;
//					pMover->objX = x;
//					pMover->objY = y;

					hEb = InitExtraBlock(pMover, collisionActor);
					x = pMover->objX;
					y = pMover->objY;
					BlockingCorner(hEb, &x, &y, pMover->ItargetX, pMover->ItargetY);

					pMover->objX = sx;
					pMover->objY = sy;
					SetMoverDest(pMover, x, y);
					pMover->over = false;
				}
			} else {
				/*----------------------------------------
				 Currently, this is as far as we can go. |
				 Definitely room for improvement here!   |
				 ----------------------------------------*/
				hPath = InPolygon(pMover->ItargetX, pMover->ItargetY, PATH);
				if (hPath != pMover->hIpath) {
					if (IsInPolygon(pMover->ItargetX, pMover->ItargetY, pMover->hIpath))
						hPath = pMover->hIpath;
				}
				assert(hPath == pMover->hIpath);

				if (pMover->InDifficulty == NO_PROB) {
					x = PolyCenterX(hPath);
					y = PolyCenterY(hPath);
					SetMoverDest(pMover, x, y);
					pMover->InDifficulty = TRY_CENTER;
					pMover->over = false;
				} else if (pMover->InDifficulty == TRY_CENTER) {
					NearestCorner(&x, &y, pMover->hCpath, pMover->hIpath);
					SetMoverDest(pMover, x, y);
					pMover->InDifficulty = TRY_CORNER;
					pMover->over = false;
				} else if (pMover->InDifficulty == TRY_CORNER) {
					NearestCorner(&x, &y, pMover->hCpath, pMover->hIpath);
					SetMoverDest(pMover, x, y);
					pMover->InDifficulty = TRY_NEXTCORNER;
					pMover->over = false;
				}
			}
			break;
		}
		else if (((lstatus & YRESTRICT) && !(s1 & YRESTRICT))
		     ||  ((lstatus & XRESTRICT) && !(s1 & XRESTRICT))) {
			/*-----------------------------------------------
			 A restriction in a direction has been removed. |
			 Use this as an intermediate destination.	 |
			 -----------------------------------------------*/
			SetMoverDest(pMover, nextx, nexty);
			pMover->over = false;
			break;
		}

		x = nextx;
		y = nexty;

		if (!TinselV2) {
			/*-------------------------
			 Change of path polygon?  |
			 -------------------------*/
			hPath = InPolygon(x, y, PATH);
			if (pMover->hCpath != hPath &&
			   !IsInPolygon(x, y, pMover->hCpath) &&
			   !IsAdjacentPath(pMover->hCpath, pMover->hIpath)) {
				/*----------------------------------------------------------
				 If just entering a follow nodes polygon, go to first node.|
				 Else if just going to pass through, go to pseudo-center.  |
				 ----------------------------------------------------------*/
				if (PolySubtype(hPath) == NODE && pMover->hFnpath != hPath && pMover->npstatus != LEAVING) {
					int node = NearestEndNode(hPath, x, y);
					getNpathNode(hPath, node, &nx, &ny);
					SetMoverDest(pMover, nx, ny);
					pMover->over = true;
				} else if (!IsInPolygon(pMover->ItargetX, pMover->ItargetY, hPath) &&
					!IsInPolygon(pMover->ItargetX, pMover->ItargetY, pMover->hCpath)) {
					SetMoverDest(pMover, PolyCenterX(hPath), PolyCenterY(hPath));
					pMover->over = true;
				} else {
					SetMoverDest(pMover, pMover->ItargetX, pMover->ItargetY);
				}
				break;
			}

			lstatus = s1;
		}
	}
}

/**
 * Work out where the next position should be.
 * Check that it's in a path and not in a blocking polygon.
 */
static void NewCoOrdinates(int fromx, int fromy, int *targetX, int *targetY,
				int *newx, int *newy, int *s1, int *s2,
				HPOLYGON *hS2p, bool bOver, bool bBodge,
				PMOVER pMover, PMOVER *collisionActor) {
	HPOLYGON hPoly;
	int sidem, depthm;
	int sidesteps, depthsteps;
	PMOVER	ma;

	*s1 = *s2 = 0;

	/*------------------------------------------------
	 Don't overrun if this is the final destination. |
	 ------------------------------------------------*/
	if ((*targetX == pMover->UtargetX && (*targetY == -1 || *targetY == pMover->UtargetY)) ||
		(*targetY == pMover->UtargetY && (*targetX == -1 || *targetX == pMover->UtargetX)))
		bOver = false;

	/*----------------------------------------------------
	 Decide how big a step to attempt in each direction. |
	 ----------------------------------------------------*/
	sidesteps = *targetX == -1 ? 0 : *targetX - fromx;
	sidesteps = ABS(sidesteps);

	depthsteps = *targetY == -1 ? 0 : *targetY - fromy;
	depthsteps = ABS(depthsteps);

	if (sidesteps && depthsteps > sidesteps) {
		depthm = YMDIST;
		sidem = depthm * sidesteps/depthsteps;

		if (!sidem)
			sidem = 1;
	} else if (depthsteps && sidesteps > depthsteps) {
		sidem = XMDIST;
		depthm = sidem * depthsteps/sidesteps;

		if (!depthm) {
			if (bBodge)
				depthm = 1;
		} else if (depthm > YMDIST)
			depthm = YMDIST;
	} else {
		sidem = sidesteps ? XMDIST : 0;
		depthm = depthsteps ? YMDIST : 0;
	}

	*newx = fromx;
	*newy = fromy;

	/*------------------------------------------------------------
	 If Left-Right movement is required - then make the move,    |
	 but don't overshoot, and do notice when we're already there |
	 ------------------------------------------------------------*/
	if (*targetX == -1)
		*s1 |= XTHERE;
	else {
		if (*targetX > fromx) {		/* To the right?	*/
			*newx += sidem;		// Move to the right...
			if (*newx == *targetX)
				*s1 |= XTHERE;
			else if (*newx > *targetX) {	// ...but don't overshoot
				if (!bOver)
					*newx = *targetX;
				else
					*targetX = *newx;
				*s1 |= XTHERE;
			}
		} else if (*targetX < fromx) {	/* To the left?		*/
			*newx -= sidem;		// Move to the left...
			if (*newx == *targetX)
				*s1 |= XTHERE;
			else if (*newx < *targetX) {	// ...but don't overshoot
				if (!bOver)
					*newx = *targetX;
				else
					*targetX = *newx;
				*s1 |= XTHERE;
			}
		} else {
			*targetX = -1;		// We're already there!
			*s1 |= XTHERE;
		}
	}

	/*--------------------------------------------------------------
	 If Up-Down movement is required - then make the move,
	 but don't overshoot, and do notice when we're already there
	 --------------------------------------------------------------*/
	if (*targetY == -1)
		*s1 |= YTHERE;
	else {
		if (*targetY > fromy) {		/* Downwards?		*/
			*newy += depthm;	// Move down...
			if (*newy == *targetY)	// ...but don't overshoot
				*s1 |= YTHERE;
			else if (*newy > *targetY) {	// ...but don't overshoot
				if (!bOver)
					*newy = *targetY;
				else
					*targetY = *newy;
				*s1 |= YTHERE;
			}
		} else if (*targetY < fromy) {	/* Upwards?		*/
			*newy -= depthm;	// Move up...
			if (*newy == *targetY)	// ...but don't overshoot
				*s1 |= YTHERE;
			else if (*newy < *targetY) {	// ...but don't overshoot
				if (!bOver)
					*newy = *targetY;
				else
					*targetY = *newy;
				*s1 |= YTHERE;
			}
		} else {
			*targetY = -1;		// We're already there!
			*s1 |= YTHERE;
		}
	}

	/* Give over if this is it */
	if (*s1 == (XTHERE | YTHERE))
		return;

	/*------------------------------------------------------
	 Have worked out where an optimum step would take us.
	 Must now check if it's in a legal spot.
	 ------------------------------------------------------*/

	if (!pMover->bNoPath && !pMover->bIgPath) {
		/*------------------------------
		 Must stay in a path polygon.
		-------------------------------*/
		hPoly = InPolygon(*newx, *newy, PATH);
		if (hPoly == NOPOLY) {
			*s2 = LEAVING_PATH;	// Trying to leave the path polygons

			if (*newx != fromx && InPolygon(*newx, fromy, PATH) != NOPOLY && InPolygon(*newx, fromy, BLOCK) == NOPOLY) {
				*newy = fromy;
				*s1 |= YRESTRICT;
			} else if (*newy != fromy && InPolygon(fromx, *newy, PATH) != NOPOLY && InPolygon(fromx, *newy, BLOCK) == NOPOLY) {
				*newx = fromx;
				*s1 |= XRESTRICT;
			} else {
				*newx = fromx;
				*newy = fromy;
#if 1
				*targetX = *targetY = -1;
#endif
				*s1 |= STUCK;
				return;
			}
		}

		/*--------------------------------------
		 Must stay out of blocking polygons.
		 --------------------------------------*/
		hPoly = InPolygon(*newx, *newy, BLOCK);
		if (hPoly != NOPOLY) {
			*s2 = ENTERING_BLOCK;	// Trying to enter a blocking poly
			*hS2p = hPoly;

			if (*newx != fromx && InPolygon(*newx, fromy, BLOCK) == NOPOLY && InPolygon(*newx, fromy, PATH) != NOPOLY) {
				*newy = fromy;
				*s1 |= YRESTRICT;
			} else if (*newy != fromy && InPolygon(fromx, *newy, BLOCK) == NOPOLY && InPolygon(fromx, *newy, PATH) != NOPOLY) {
				*newx = fromx;
				*s1 |= XRESTRICT;
			} else {
				*newx = fromx;
				*newy = fromy;
#if 1
				*targetX = *targetY = -1;
#endif
				*s1 |= STUCK;
			}
		}
		/*------------------------------------------------------
		 Must stay out of moving actors' blocking polygons.
		 ------------------------------------------------------*/
		ma = InMoverBlock(pMover, *newx, *newy);
		if (ma != NULL) {
			// Ignore if already in it (it may have just appeared)
			if (!InMoverBlock(pMover, pMover->objX, pMover->objY)) {
				*s2 = ENTERING_MBLOCK;	// Trying to walk through an actor

				*hS2p = -1;
				if (collisionActor)
					*collisionActor = ma;

				if (*newx != fromx && InMoverBlock(pMover, *newx, fromy) == NULL
				    && InPolygon(*newx, fromy, BLOCK) == NOPOLY && InPolygon(*newx, fromy, PATH) != NOPOLY) {
					*newy = fromy;
					*s1 |= YRESTRICT;
				} else if (*newy != fromy && InMoverBlock(pMover, fromx, *newy) == NULL
				           && InPolygon(fromx, *newy, BLOCK) == NOPOLY && InPolygon(fromx, *newy, PATH) != NOPOLY) {
					*newx = fromx;
					*s1 |= XRESTRICT;
				} else {
					*newx = fromx;
					*newy = fromy;
#if 1
					*targetX = *targetY = -1;
#endif
					*s1 |= STUCK;
				}
			}
		}
	}
}

/**
 * SetOffWithinNodePath
 */
static void SetOffWithinNodePath(PMOVER pMover, HPOLYGON StartPath, HPOLYGON DestPath,
								 int targetX, int targetY) {
	int endnode;
	HPOLYGON hIpath;
	int	nx, ny;
	int	x, y;

	if (StartPath == DestPath) {
		if (pMover->line == pMover->Tline) {
			SetMoverDest(pMover, pMover->UtargetX, pMover->UtargetY);
			pMover->over = false;
		} else if (pMover->line < pMover->Tline) {
			getNpathNode(StartPath, pMover->line+1, &nx, &ny);
			SetMoverDest(pMover, nx, ny);
			pMover->npstatus = GOING_UP;
		} else if (pMover->line > pMover->Tline) {
			getNpathNode(StartPath, pMover->line, &nx, &ny);
			SetMoverDest(pMover, nx, ny);
			pMover->npstatus = GOING_DOWN;
		}
	} else {
		/*
		 * Leaving this path - work out
		 * which end of this path to head for.
		 */
		assert(DestPath != NOPOLY); // Error 702
		if ((hIpath = GetPathOnTheWay(StartPath, DestPath)) == NOPOLY) {
			// This should never happen!
			// It's the old code that didn't always work.
			endnode = NearestEndNode(StartPath, targetX, targetY);
		} else {
			if (PolySubtype(hIpath) != NODE) {
				x = PolyCenterX(hIpath);
				y = PolyCenterY(hIpath);
				endnode = NearestEndNode(StartPath, x, y);
			} else {
				endnode = NearEndNode(StartPath, hIpath);
			}
		}

#if 1
		if ((pMover->npstatus == LEAVING) &&
			endnode == NearestEndNode(StartPath, pMover->objX, pMover->objY)) {
			// Leave it be

			if (TinselV2) {
				// Yeah, but we need a destination
				// It's release night and there's this problem in the bar...
				if (hIpath)	// must be, but...
				{
					// could go for its end node if it's an NPATH
					// but we probably will when we hit it anyway!
					SetMoverDest(pMover, PolyCenterX(hIpath), PolyCenterY(hIpath));
				}
			}
		} else
#endif
		{
			if (endnode) {
				getNpathNode(StartPath, pMover->line+1, &nx, &ny);
				SetMoverDest(pMover, nx, ny);
				pMover->npstatus = GOING_UP;
			} else {
				getNpathNode(StartPath, pMover->line, &nx, &ny);
				SetMoverDest(pMover, nx, ny);
				pMover->npstatus = GOING_DOWN;
			}
		}
	}
}

/**
 * Restore a movement, called from restoreMovement() in ACTORS.CPP
 */
void SSetActorDest(PMOVER pActor) {
	if (pActor->UtargetX != -1 && pActor->UtargetY != -1) {
		Stand(Common::nullContext, pActor->actorID, pActor->objX, pActor->objY, 0);

		if (pActor->UtargetX != -1 && pActor->UtargetY != -1) {
			SetActorDest(pActor, pActor->UtargetX, pActor->UtargetY,
					pActor->bIgPath, 0);
		}
	} else {
		Stand(Common::nullContext, pActor->actorID, pActor->objX, pActor->objY, 0);
	}
}

/**
 * Initiate a movement, called from WalkTo_Event()
 */
int SetActorDest(PMOVER pMover, int clickX, int clickY, bool igPath, SCNHANDLE hFilm) {
	HPOLYGON StartPath, DestPath = 0;
	int targetX, targetY;

	if (TinselV2) {
		// No need to synchronise if not moving!
		// Hopefully will stop luggage flip in shades.
		if (!MoverMoving(pMover))
			pMover->stepCount = 0;

		// Fix interrupted-walking-to-wardrobe bug in mortuary
		StopMover(pMover);
	} else {
		if (pMover->actorID == GetLeadId())		// Now only for lead actor
			UnTagActor(pMover->actorID);	// Tag not allowed while moving
	}

	pMover->walkNumber++;
	pMover->bStop = false;
	pMover->over = false;
	pMover->walkedFromX = pMover->objX;
	pMover->walkedFromY = pMover->objY;
	pMover->bMoving = true;
	pMover->bIgPath = igPath;
	pMover->zOverride = -1;
	pMover->hRpath = NOPOLY;

	if (!TinselV2) {
		// Use the supplied reel or restore the normal actor.
		if (hFilm != 0)
			AlterMover(pMover, hFilm, AR_WALKREEL);
		else
			AlterMover(pMover, 0, AR_NORMAL);
	}

	if (igPath) {
		targetX = clickX;
		targetY = clickY;

		if (pMover->actorID == GetLeadId()) {
			g_lastLeadXdest = targetX;
			g_lastLeadYdest = targetY;
		}
	} else {
		int wodResult = WorkOutDestination(clickX, clickY, &targetX, &targetY);

		if (pMover->actorID == GetLeadId()) {
			g_lastLeadXdest = targetX;
			g_lastLeadYdest = targetY;
		}

		if (wodResult == ALL_SORTED) {
			GotThere(pMover);
			return 0;
		}
		assert(InPolygon(targetX, targetY, PATH) != NOPOLY); // illegal destination!
		assert(InPolygon(targetX, targetY, BLOCK) == NOPOLY); // illegal destination!
	}


	/***** Now have a destination to aim for. *****/

	/*----------------------------------
	| Don't move if it's not worth it.
	----------------------------------*/
	if (ABS(targetX - pMover->objX) < XMDIST && ABS(targetY - pMover->objY) < YMDIST) {
		GotThere(pMover);
		return 0;
	}

	/*------------------------------------------------------
	| If the destiation is within a follow nodes polygon,
	| set destination as the nearest node.
	------------------------------------------------------*/
	if (!igPath) {
		DestPath = InPolygon(targetX, targetY, PATH);
		if (PolySubtype(DestPath) == NODE) {
			// Find the nearest point on a line, or nearest node
			FindBestPoint(DestPath, &targetX, &targetY, &pMover->Tline);
		}
	}

	assert(pMover->bIgPath || InPolygon(targetX, targetY, PATH) != NOPOLY); // Error 5005
	SetMoverUltDest(pMover, targetX, targetY);
	SetMoverIntDest(pMover, targetX, targetY);

	if (TinselV2) {
		// No movement for unconnected paths
		if (pMover->hIpath == NOPOLY && !igPath) {
			GotThere(pMover);
			return 0;
		}

		// Use the supplied reel or restore the normal actor.
		if (hFilm != 0)
			AlterMover(pMover, hFilm, AR_WALKREEL);
		else
			AlterMover(pMover, 0, AR_NORMAL);
	}

	/*-------------------------------------------------------------------
	| If in a follow nodes path, need to set off in the right direction! |
	-------------------------------------------------------------------*/
	if ((StartPath = pMover->hFnpath) != NOPOLY && !igPath) {
		SetOffWithinNodePath(pMover, StartPath, DestPath, targetX, targetY);
	} else {
		// Set off!
		SetNextDest(pMover);
	}

	return pMover->walkNumber;
}

/**
 * Change scale if appropriate.
 */
static void CheckScale(PMOVER pActor, HPOLYGON hPath, int ypos) {
	int scale;

	scale = GetScale(hPath, ypos);
	if (scale != pActor->scale) {
		SetMoverWalkReel(pActor, pActor->direction, scale, false);
	}
}

/**
 * Not going anywhere - Kick off again if not at final destination.
 */
static void NotMoving(PMOVER pActor, int x, int y) {
	pActor->targetX = pActor->targetY = -1;

//	if (x == pActor->UtargetX && y == pActor->UtargetY)
	if (ABS(x - pActor->UtargetX) < XMDIST && ABS(y - pActor->UtargetY) < YMDIST) {
		GotThere(pActor);
		return;
	}

	if (pActor->ItargetX != -1 || pActor->ItargetY != -1) {
		SetNextDest(pActor);
	} else if (pActor->UtargetX != -1 || pActor->UtargetY != -1) {
		assert(pActor->bIgPath || InPolygon(pActor->UtargetX, pActor->UtargetY, PATH) != NOPOLY); // Error 5006
		SetMoverIntDest(pActor, pActor->UtargetX, pActor->UtargetY);
		SetNextDest(pActor);
	}
}

/**
 * Does the necessary business when entering a different path polygon.
 */
static void EnteringNewPath(PMOVER pMover, HPOLYGON hPath, int x, int y) {
	int	firstnode;	// First node to go to
	int	lastnode;	// Last node to go to
	HPOLYGON hIpath;
	HPOLYGON hLpath;	// one we're leaving
	int	nx, ny;
	int	nxl, nyl;

	hLpath = pMover->hCpath;
	pMover->hCpath = hPath;		// current path

	if (hPath == NOPOLY) {
		// Not proved this ever happens, but just in case
		pMover->hFnpath = NOPOLY;
		pMover->npstatus = NOT_IN;
		return;
	}

	// Is new path a node path?
	if (PolySubtype(hPath) == NODE) {
		// Node path - usually go to nearest end node
		firstnode = NearestEndNode(hPath, x, y);
		lastnode = -1;

		// If this is not the destination path,
		// find which end nodfe we wish to leave via
		if (hPath != pMover->hUpath) {
			if (pMover->bIgPath) {
				lastnode = NearestEndNode(hPath, pMover->UtargetX, pMover->UtargetY);
			} else {
				assert(pMover->hUpath != NOPOLY); // Error 703
				hIpath = GetPathOnTheWay(hPath, pMover->hUpath);
				assert(hIpath != NOPOLY); // No path on the way

				if (PolySubtype(hIpath) != NODE) {
					lastnode = NearestEndNode(hPath, PolyCenterX(hIpath), PolyCenterY(hIpath));
				} else {
					lastnode = NearEndNode(hPath, hIpath);
				}
			}
		}
		// Test for pseudo-one-node npaths
		if (lastnode != -1 && numNodes(hPath) == 2) {
			getNpathNode(hPath, firstnode, &nx, &ny);
			getNpathNode(hPath, lastnode, &nxl, &nyl);
			if (nxl == nx && nyl == ny)
				firstnode = lastnode;
		}

		// If leaving by same node as entering, don't bother.
		if (lastnode == firstnode) {
			pMover->hFnpath = NOPOLY;
			pMover->npstatus = NOT_IN;
			assert(pMover->bIgPath || InPolygon(pMover->UtargetX, pMover->UtargetY, PATH) != NOPOLY); // Error 5007
			SetMoverIntDest(pMover, pMover->UtargetX, pMover->UtargetY);
			SetNextDest(pMover);
		} else {
			// Head for first node
			pMover->over = true;
			pMover->npstatus = ENTERING;
			pMover->hFnpath = hPath;
			pMover->line = firstnode ? firstnode - 1 : firstnode;
			if (pMover->line == pMover->Tline && hPath == pMover->hUpath) {
				assert(pMover->bIgPath || InPolygon(pMover->UtargetX, pMover->UtargetY, PATH) != NOPOLY); // Error 5008
				SetMoverIntDest(pMover, pMover->UtargetX, pMover->UtargetY);
				SetMoverDest(pMover, pMover->UtargetX, pMover->UtargetY);
			} else {
				// This doesn't seem right
				getNpathNode(hPath, firstnode, &nx, &ny);
				if (ABS(pMover->objX - nx) < XMDIST
						&& ABS(pMover->objY - ny) < YMDIST) {
					pMover->npstatus = ENTERING;
					pMover->hFnpath = hPath;
					SetNextDest(pMover);
				} else {
					getNpathNode(hPath, firstnode, &nx, &ny);
					SetMoverDest(pMover, nx, ny);
				}
			}
		}
		return;
	} else {
		pMover->hFnpath = NOPOLY;
		pMover->npstatus = NOT_IN;
		assert(pMover->bIgPath || InPolygon(pMover->UtargetX, pMover->UtargetY, PATH) != NOPOLY); // Error 5009

		// Added 26/01/95
		if (IsPolyCorner(hPath, pMover->ItargetX, pMover->ItargetY))
			return;

		// Added 23/10/96
		if (TinselV2 && (pMover->hRpath == hPath))
			return;

		if (TinselV2)
			pMover->hRpath = hLpath;
		SetMoverIntDest(pMover, pMover->UtargetX, pMover->UtargetY);
		SetNextDest(pMover);
	}
}

/**
 * Move
 */
void Move(PMOVER pMover, int newx, int newy, HPOLYGON hPath) {
	pMover->objX = newx;
	pMover->objY = newy;

	MultiSetAniXY(pMover->actorObj, newx, newy);
	SetMoverZ(pMover, newy, GetPolyZfactor(hPath));
	if (StepAnimScript(&pMover->actorAnim) == ScriptFinished) {
		// The end of a scale-change reel
		// Revert to normal walking reel
		pMover->bWalkReel = false;
		pMover->stepCount = 0;
		SetMoverWalkReel(pMover, pMover->direction, pMover->scale, true);
	}

	// Synchronised walking reels
	if (++pMover->stepCount >= STEPS_MAX)
		pMover->stepCount = 0;
}

/**
 * Called from MActorProcess() on every tick.
 *
 * Moves the actor as appropriate.
 */
void MoveActor(PMOVER pMover) {
	int newx, newy;
	HPOLYGON hPath;
	int status, s2;		// s2 not used here!
	HPOLYGON hS2p;		// nor is s2p!
	HPOLYGON hEb;
	PMOVER ma;
	int	sTargetX, sTargetY;
	bool bNewPath = false;

	// Only do anything if the actor needs to move!
	if (pMover->targetX == -1 && pMover->targetY == -1)
		return;

	if (pMover->bStop) {
		GotThere(pMover);
		pMover->bStop = false;
		pMover->walkNumber++;
		SetMoverStanding(pMover);
		return;
	}

#if SLOW_RINCE_DOWN
/**/	if (g_BogusVar++ < g_Interlude)	// Temporary slow-down-the-action code
/**/		return;			//
/**/	g_BogusVar = 0;			//
#endif

	if (!TinselV2) {
		// During swalk()s, movement while hidden may be slowed down.
		if (pMover->bHidden) {
			if (++g_hSlowVar < pMover->SlowFactor)
				return;
			g_hSlowVar = 0;
		}
	}

	// 'push' the target
	sTargetX = pMover->targetX;
	sTargetY = pMover->targetY;

	NewCoOrdinates(pMover->objX, pMover->objY, &pMover->targetX, &pMover->targetY,
			&newx, &newy, &status, &s2, &hS2p, pMover->over, false, pMover);

	if (newx == pMover->objX && newy == pMover->objY) {
		// 'pop' the target
		pMover->targetX = sTargetX;
		pMover->targetY = sTargetY;

		NewCoOrdinates(pMover->objX, pMover->objY, &pMover->targetX, &pMover->targetY, &newx, &newy,
				&status, &s2, &hS2p, pMover->over, true, pMover);
		if (newx == pMover->objX && newy == pMover->objY) {
			NotMoving(pMover, newx, newy);
			return;
		}
	}

	// Find out which path we're in now
	hPath = InPolygon(newx, newy, PATH);
	if (hPath == NOPOLY) {
		if (pMover->bNoPath) {
			Move(pMover, newx, newy, pMover->hCpath);
			return;
		} else {
			// May be marginally outside!
			// OR bIgPath may be set.
			hPath = pMover->hCpath;
		}
	} else if (pMover->bNoPath) {
		pMover->bNoPath = false;
		bNewPath = true;
	} else if (hPath != pMover->hCpath) {
		if (IsInPolygon(newx, newy, pMover->hCpath))
			hPath = pMover->hCpath;
	}

	CheckScale(pMover, hPath, newy);

	/*
	* Must stay out of moving actors' blocking polygons.
	*/
	ma = InMoverBlock(pMover, newx, newy);
	if (ma != NULL) {
		// Stop if there's no chance of arriving
		if (InMoverBlock(pMover, pMover->UtargetX, pMover->UtargetY)) {
			GotThere(pMover);
			return;
		}

		if (InMoverBlock(pMover, pMover->objX, pMover->objY))
			;
		else {
			hEb = InitExtraBlock(pMover, ma);
			newx = pMover->objX;
			newy = pMover->objY;
			BlockingCorner(hEb, &newx, &newy, pMover->ItargetX, pMover->ItargetY);
			SetMoverDest(pMover, newx, newy);
			return;
		}
	}

	/*--------------------------------------
	 This is where it actually gets moved.
	 --------------------------------------*/
	Move(pMover, newx, newy, hPath);

	// Entering a new path polygon?
	if (hPath != pMover->hCpath || bNewPath)
		EnteringNewPath(pMover, hPath, newx, newy);
}

/**
 * Store the default refer type for the current scene.
 */
void SetDefaultRefer(int32 defRefer) {
	g_DefaultRefer = defRefer;
}

int GetLastLeadXdest() {
	return g_lastLeadXdest;
}

int GetLastLeadYdest() {
	return g_lastLeadYdest;
}




/**
 * DoMoveActor
 */
void DoMoveActor(PMOVER pActor) {
	int wasx, wasy;
	int i;

#define NUMBER 1

	wasx = pActor->objX;
	wasy = pActor->objY;

	MoveActor(pActor);

	if ((pActor->targetX != -1 || pActor->targetY != -1)
	&&  (wasx == pActor->objX && wasy == pActor->objY))	{
		for (i=0; i < NUMBER; i++) {
			MoveActor(pActor);
			if (wasx != pActor->objX || wasy != pActor->objY)
				break;
		}
//		assert(i<NUMBER);
	}
}

} // End of namespace Tinsel
