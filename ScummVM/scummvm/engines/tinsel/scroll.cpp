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
 * Handles scrolling
 */

#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/graphics.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/scroll.h"
#include "tinsel/sched.h"
#include "tinsel/sysvar.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

//----------------- LOCAL DEFINES --------------------

#define LEFT	'L'
#define RIGHT	'R'
#define UP	'U'
#define DOWN	'D'



//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars


static int g_LeftScroll = 0, g_DownScroll = 0;	// Number of iterations outstanding

static int g_scrollActor = 0;
static PMOVER g_pScrollMover = 0;
static int g_oldx = 0, g_oldy = 0;

/** Boundaries and numbers of boundaries */
static SCROLLDATA g_sd = {
		{
			{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
			{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}
		},
		{
			{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0},
			{0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}
		},
		0,
		0,
		// DW2 fields
		0,
		0,
		0,
		0,
		0,
		0,
		0
	};

static int g_ImageH = 0, g_ImageW = 0;

static bool g_ScrollCursor = 0;	// If a TAG or EXIT polygon is clicked on,
				// the cursor is kept over that polygon
				// whilst scrolling

static int g_scrollPixelsX = SCROLLPIXELS;
static int g_scrollPixelsY = SCROLLPIXELS;


/**
 * Reset the ScrollCursor flag
 */
void DontScrollCursor() {
	g_ScrollCursor = false;
}

/**
 * Set the ScrollCursor flag
 */
void DoScrollCursor() {
	g_ScrollCursor = true;
}

/**
 * Configure a no-scroll boundary for a scene.
 */
void SetNoScroll(int x1, int y1, int x2, int y2) {
	if (x1 == x2) {
		/* Vertical line */
		assert(g_sd.NumNoH < MAX_HNOSCROLL);

		g_sd.NoHScroll[g_sd.NumNoH].ln = x1;	// X pos of vertical line
		g_sd.NoHScroll[g_sd.NumNoH].c1 = y1;
		g_sd.NoHScroll[g_sd.NumNoH].c2 = y2;
		g_sd.NumNoH++;
	} else if (y1 == y2) {
		/* Horizontal line */
		assert(g_sd.NumNoV < MAX_VNOSCROLL);

		g_sd.NoVScroll[g_sd.NumNoV].ln = y1;	// Y pos of horizontal line
		g_sd.NoVScroll[g_sd.NumNoV].c1 = x1;
		g_sd.NoVScroll[g_sd.NumNoV].c2 = x2;
		g_sd.NumNoV++;
	} else {
		/* No-scroll lines must be horizontal or vertical */
	}
}

/**
 * Called from scroll process when it thinks that a scroll is in order.
 * Checks for no-scroll boundaries and sets off a scroll if allowed.
 */
static void NeedScroll(int direction) {
	uint	i;
	int	BottomLine, RightCol;
	int	Loffset, Toffset;

	// get background offsets
	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	switch (direction) {
	case LEFT:	  /* Picture will go left, 'camera' right */

		BottomLine = Toffset + (SCREEN_HEIGHT - 1);
		RightCol = Loffset + (SCREEN_WIDTH - 1);

		for (i = 0; i < g_sd.NumNoH; i++) {
			if (RightCol >= g_sd.NoHScroll[i].ln - 1 && RightCol <= g_sd.NoHScroll[i].ln + 1 &&
					((g_sd.NoHScroll[i].c1 >= Toffset && g_sd.NoHScroll[i].c1 <= BottomLine) ||
					(g_sd.NoHScroll[i].c2 >= Toffset && g_sd.NoHScroll[i].c2 <= BottomLine) ||
					(g_sd.NoHScroll[i].c1 < Toffset && g_sd.NoHScroll[i].c2 > BottomLine)))
				return;
		}

		if (g_LeftScroll <= 0) {
			if (TinselV2) {
				g_scrollPixelsX = g_sd.xSpeed;
				g_LeftScroll += g_sd.xDistance;
			} else {
				g_scrollPixelsX = SCROLLPIXELS;
				g_LeftScroll = RLSCROLL;
			}
		}
		break;

	case RIGHT:	 /* Picture will go right, 'camera' left */

		BottomLine = Toffset + (SCREEN_HEIGHT - 1);

		for (i = 0; i < g_sd.NumNoH; i++) {
			if (Loffset >= g_sd.NoHScroll[i].ln - 1 && Loffset <= g_sd.NoHScroll[i].ln + 1 &&
					((g_sd.NoHScroll[i].c1 >= Toffset && g_sd.NoHScroll[i].c1 <= BottomLine) ||
					(g_sd.NoHScroll[i].c2 >= Toffset && g_sd.NoHScroll[i].c2 <= BottomLine) ||
					(g_sd.NoHScroll[i].c1 < Toffset && g_sd.NoHScroll[i].c2 > BottomLine)))
				return;
		}

		if (g_LeftScroll >= 0) {
			if (TinselV2) {
				g_scrollPixelsX = g_sd.xSpeed;
				g_LeftScroll -= g_sd.xDistance;
			} else {
				g_scrollPixelsX = SCROLLPIXELS;
				g_LeftScroll = -RLSCROLL;
			}
		}
		break;

	case UP:		/* Picture will go upwards, 'camera' downwards  */

		BottomLine = Toffset + (SCREEN_HEIGHT - 1);
		RightCol = Loffset + (SCREEN_WIDTH - 1);

		for (i = 0; i < g_sd.NumNoV; i++) {
			if ((BottomLine >= g_sd.NoVScroll[i].ln - 1 && BottomLine <= g_sd.NoVScroll[i].ln + 1) &&
					((g_sd.NoVScroll[i].c1 >= Loffset && g_sd.NoVScroll[i].c1 <= RightCol) ||
					(g_sd.NoVScroll[i].c2 >= Loffset && g_sd.NoVScroll[i].c2 <= RightCol) ||
					(g_sd.NoVScroll[i].c1 < Loffset && g_sd.NoVScroll[i].c2 > RightCol)))
				return;
			}

		if (g_DownScroll <= 0) {
			if (TinselV2) {
				g_scrollPixelsY = g_sd.ySpeed;
				g_DownScroll += g_sd.yDistance;
			} else {
				g_scrollPixelsY = SCROLLPIXELS;
				g_DownScroll = UDSCROLL;
			}
		}
		break;

	case DOWN:	  /* Picture will go downwards, 'camera' upwards  */

		RightCol = Loffset + (SCREEN_WIDTH - 1);

		for (i = 0; i < g_sd.NumNoV; i++) {
			if (Toffset >= g_sd.NoVScroll[i].ln - 1  && Toffset <= g_sd.NoVScroll[i].ln + 1  &&
					((g_sd.NoVScroll[i].c1 >= Loffset && g_sd.NoVScroll[i].c1 <= RightCol) ||
					(g_sd.NoVScroll[i].c2 >= Loffset && g_sd.NoVScroll[i].c2 <= RightCol) ||
					(g_sd.NoVScroll[i].c1 < Loffset && g_sd.NoVScroll[i].c2 > RightCol)))
				return;
		}

		if (g_DownScroll >= 0) {
			if (TinselV2) {
				g_scrollPixelsY = g_sd.ySpeed;
				g_DownScroll -= g_sd.yDistance;
			} else {
				g_scrollPixelsY = SCROLLPIXELS;
				g_DownScroll = -UDSCROLL;
			}
		}
		break;
	}
}

/**
 * Called from scroll process - Scrolls the image as appropriate.
 */
static void ScrollImage() {
	int OldLoffset = 0, OldToffset = 0;	// Used when keeping cursor on a tag
	int Loffset, Toffset;
	int curX, curY;

	// get background offsets
	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	/*
	 * Keeping cursor on a tag?
	 */
	if (g_ScrollCursor) {
		GetCursorXYNoWait(&curX, &curY, true);
		if (InPolygon(curX, curY, TAG) != NOPOLY || InPolygon(curX, curY, EXIT) != NOPOLY) {
			OldLoffset = Loffset;
			OldToffset = Toffset;
		} else
			g_ScrollCursor = false;
	}

	/*
	 * Horizontal scrolling
	 */
	if (g_LeftScroll > 0) {
		g_LeftScroll -= g_scrollPixelsX;
		if (g_LeftScroll < 0) {
			Loffset += g_LeftScroll;
			g_LeftScroll = 0;
		}
		Loffset += g_scrollPixelsX;		// Move right
		if (Loffset > g_ImageW - SCREEN_WIDTH)
			Loffset = g_ImageW - SCREEN_WIDTH;// Now at extreme right

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MaximumXoffset) &&  (Loffset > SysVar(SV_MaximumXoffset)))
			Loffset = SysVar(SV_MaximumXoffset);

	} else if (g_LeftScroll < 0) {
		g_LeftScroll += g_scrollPixelsX;
		if (g_LeftScroll > 0) {
			Loffset += g_LeftScroll;
			g_LeftScroll = 0;
		}
		Loffset -= g_scrollPixelsX;	// Move left
		if (Loffset < 0)
			Loffset = 0;		// Now at extreme left

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MinimumXoffset) &&  (Loffset < SysVar(SV_MinimumXoffset)))
			Loffset = SysVar(SV_MinimumXoffset);
	}

	/*
	 * Vertical scrolling
	 */
	if (g_DownScroll > 0) {
		g_DownScroll -= g_scrollPixelsY;
		if (g_DownScroll < 0) {
			Toffset += g_DownScroll;
			g_DownScroll = 0;
		}
		Toffset += g_scrollPixelsY;		// Move down

		if (Toffset > g_ImageH - SCREEN_HEIGHT)
			Toffset = g_ImageH - SCREEN_HEIGHT;// Now at extreme bottom

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MaximumYoffset) &&  Toffset > SysVar(SV_MaximumYoffset))
			Toffset = SysVar(SV_MaximumYoffset);

	} else if (g_DownScroll < 0) {
		g_DownScroll += g_scrollPixelsY;
		if (g_DownScroll > 0) {
			Toffset += g_DownScroll;
			g_DownScroll = 0;
		}
		Toffset -= g_scrollPixelsY;		// Move up

		if (Toffset < 0)
			Toffset = 0;			// Now at extreme top

		/*** New feature to prop up rickety scroll boundaries ***/
		if (TinselV2 && SysVar(SV_MinimumYoffset) &&  Toffset < SysVar(SV_MinimumYoffset))
			Toffset = SysVar(SV_MinimumYoffset);
	}

	/*
	 * Move cursor if keeping cursor on a tag.
	 */
	if (g_ScrollCursor)
		AdjustCursorXY(OldLoffset - Loffset, OldToffset - Toffset);

	PlayfieldSetPos(FIELD_WORLD, Loffset, Toffset);
}


/**
 * See if the actor on whom the camera is is approaching an edge.
 * Request a scroll if he is.
 */
static void MonitorScroll() {
	int newx, newy;
	int Loffset, Toffset;

	/*
	 * Only do it if the actor is there and is visible
	 */
	if (!g_pScrollMover || MoverHidden(g_pScrollMover) || !MoverIs(g_pScrollMover))
		return;

	GetActorPos(g_scrollActor, &newx, &newy);

	if (g_oldx == newx && g_oldy == newy)
		return;

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);

	/*
	 * Approaching right side or left side of the screen?
	 */
	if (newx > Loffset+SCREEN_WIDTH - RLDISTANCE && Loffset < g_ImageW - SCREEN_WIDTH) {
		if (newx > g_oldx)
				NeedScroll(LEFT);
	} else if (newx < Loffset + RLDISTANCE  &&  Loffset) {
		if (newx < g_oldx)
				NeedScroll(RIGHT);
	}

	/*
	 * Approaching bottom or top of the screen?
	 */
	if (newy > Toffset+SCREEN_HEIGHT-DDISTANCE && Toffset < g_ImageH-SCREEN_HEIGHT) {
		if (newy > g_oldy)
				NeedScroll(UP);
	} else if (Toffset && newy < Toffset + UDISTANCE + GetActorBottom(g_scrollActor) - GetActorTop(g_scrollActor)) {
		if (newy < g_oldy)
				NeedScroll(DOWN);
	}

	g_oldx = newx;
	g_oldy = newy;
}

static void RestoreScrollDefaults() {
	g_sd.xTrigger		= SysVar(SV_SCROLL_XTRIGGER);
	g_sd.xDistance	= SysVar(SV_SCROLL_XDISTANCE);
	g_sd.xSpeed		= SysVar(SV_SCROLL_XSPEED);
	g_sd.yTriggerTop	= SysVar(SV_SCROLL_YTRIGGERTOP);
	g_sd.yTriggerBottom= SysVar(SV_SCROLL_YTRIGGERBOT);
	g_sd.yDistance	= SysVar(SV_SCROLL_YDISTANCE);
	g_sd.ySpeed		= SysVar(SV_SCROLL_YSPEED);
}

/**
 * Does the obvious - called at the end of a scene.
 */
void DropScroll() {
	g_sd.NumNoH = g_sd.NumNoV = 0;
	if (TinselV2) {
		g_LeftScroll = g_DownScroll = 0;		// No iterations outstanding
		g_oldx = g_oldy = 0;
		g_scrollPixelsX = g_sd.xSpeed;
		g_scrollPixelsY = g_sd.ySpeed;
		RestoreScrollDefaults();
	}
}

/**
 * Decide when to scroll and scroll when decided to.
 */
void ScrollProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// In Tinsel v2, scenes may play movies, so the background may not always
	// already be initialized like it is in v1
	while (!GetBgObject())
		CORO_SLEEP(1);

	g_ImageH = BgHeight();		// Dimensions
	g_ImageW = BgWidth();		//  of this scene.

	// Give up if there'll be no purpose in this process
	if (g_ImageW == SCREEN_WIDTH  &&  g_ImageH == SCREEN_HEIGHT)
		CORO_KILL_SELF();

	if (!TinselV2) {
		g_LeftScroll = g_DownScroll = 0;		// No iterations outstanding
		g_oldx = g_oldy = 0;
		g_scrollPixelsX = g_scrollPixelsY = SCROLLPIXELS;
	}

	if (!g_scrollActor)
		g_scrollActor = GetLeadId();

	g_pScrollMover = GetMover(g_scrollActor);

	while (1) {
		MonitorScroll();		// Set scroll requirement

		if (g_LeftScroll || g_DownScroll)	// Scroll if required
			ScrollImage();

		CORO_SLEEP(1);		// allow re-scheduling
	}

	CORO_END_CODE;
}

/**
 * Change which actor the camera is following.
 */
void ScrollFocus(int ano) {
	if (g_scrollActor != ano) {
		g_oldx = g_oldy = 0;
		g_scrollActor = ano;

		g_pScrollMover = ano ? GetMover(g_scrollActor) : NULL;
	}
}

/**
 * Returns the actor which the camera is following
 */
int GetScrollFocus() {
	return g_scrollActor;
}


/**
 * Scroll to abslote position.
 */
void ScrollTo(int x, int y, int xIter, int yIter) {
	int Loffset, Toffset;		// for background offsets

	g_scrollPixelsX = xIter != 0 ? xIter : (TinselV2 ? g_sd.xSpeed : SCROLLPIXELS);
	g_scrollPixelsY = yIter != 0 ? yIter : (TinselV2 ? g_sd.ySpeed : SCROLLPIXELS);

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);	// get background offsets

	g_LeftScroll = x - Loffset;
	g_DownScroll = y - Toffset;
}

/**
 * Kill of any current scroll.
 */
void KillScroll() {
	g_LeftScroll = g_DownScroll = 0;
}


void GetNoScrollData(SCROLLDATA *ssd) {
	memcpy(ssd, &g_sd, sizeof(SCROLLDATA));
}

void RestoreNoScrollData(SCROLLDATA *ssd) {
	memcpy(&g_sd, ssd, sizeof(SCROLLDATA));
}

/**
 * SetScrollParameters
 */
void SetScrollParameters(int xTrigger, int xDistance, int xSpeed, int yTriggerTop,
		int yTriggerBottom, int yDistance, int ySpeed) {
	if (xTrigger == 0 && xDistance == 0 && xSpeed == 0
	 && yTriggerTop == 0 && yTriggerBottom && yDistance == 0 && ySpeed == 0) {
		// Restore defaults
		RestoreScrollDefaults();
	} else {
		if (xTrigger)
			g_sd.xTrigger = xTrigger;
		if (xDistance)
			g_sd.xDistance = xDistance;
		if (xSpeed)
			g_sd.xSpeed = xSpeed;
		if (yTriggerTop)
			g_sd.yTriggerTop = yTriggerTop;
		if (yTriggerBottom)
			g_sd.yTriggerBottom = yTriggerBottom;
		if (yDistance)
			g_sd.yDistance = yDistance;
		if (ySpeed)
			g_sd.ySpeed = ySpeed;
	}
}

bool IsScrolling() {
	return (g_LeftScroll || g_DownScroll);
}

} // End of namespace Tinsel
