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
 * Functions to set up moving actors' reels.
 */

#include "tinsel/handle.h"
#include "tinsel/pcode.h"	// For D_UP, D_DOWN
#include "tinsel/rince.h"

#include "common/textconsole.h"
#include "common/util.h"

namespace Tinsel {

//----------------- LOCAL GLOBAL DATA --------------------

enum {
	NUM_INTERVALS = REQ_MAIN_SCALES - 1,

	// 2 for up and down, 3 allow enough entries for 3 fully subscribed moving actors' worth
	MAX_SCRENTRIES = NUM_INTERVALS*2*3
};

struct SCIdataStruct {
	int	actor;
	int	scale;
	int	direction;
	SCNHANDLE reels[4];
};

// FIXME: Avoid non-const global vars

static SCIdataStruct g_SCIdata[MAX_SCRENTRIES];

static int g_scrEntries = 0;

/**
 * Sets an actor's walk reels
 */

void SetWalkReels(PMOVER pMover, int scale,
		SCNHANDLE al, SCNHANDLE ar, SCNHANDLE af, SCNHANDLE aa) {
	assert(scale > 0 && scale <= TOTAL_SCALES);

	pMover->walkReels[scale-1][LEFTREEL] = al;
	pMover->walkReels[scale-1][RIGHTREEL] = ar;
	pMover->walkReels[scale-1][FORWARD] = af;
	pMover->walkReels[scale-1][AWAY] = aa;
}


/**
 * Sets an actor's stand reels
 */

void SetStandReels(PMOVER pMover, int scale,
		SCNHANDLE al, SCNHANDLE ar, SCNHANDLE af, SCNHANDLE aa) {
	assert(scale > 0 && scale <= TOTAL_SCALES);

	pMover->standReels[scale-1][LEFTREEL] = al;
	pMover->standReels[scale-1][RIGHTREEL] = ar;
	pMover->standReels[scale-1][FORWARD] = af;
	pMover->standReels[scale-1][AWAY] = aa;
}


/**
 * Sets an actor's talk reels
 */

void SetTalkReels(PMOVER pMover, int scale,
		SCNHANDLE al, SCNHANDLE ar, SCNHANDLE af, SCNHANDLE aa) {
	assert(scale > 0 && scale <= TOTAL_SCALES);

	pMover->talkReels[scale-1][LEFTREEL] = al;
	pMover->talkReels[scale-1][RIGHTREEL] = ar;
	pMover->talkReels[scale-1][FORWARD] = af;
	pMover->talkReels[scale-1][AWAY] = aa;
}

/**
 * Return handle to actor's talk reel at present scale and direction.
 */
SCNHANDLE GetMoverTalkReel(PMOVER pActor, TFTYPE dirn) {
	assert(1 <= pActor->scale && pActor->scale <= TOTAL_SCALES);
	switch (dirn) {
	case TF_NONE:
		return pActor->talkReels[pActor->scale-1][pActor->direction];

	case TF_UP:
		return pActor->talkReels[pActor->scale-1][AWAY];

	case TF_DOWN:
		return pActor->talkReels[pActor->scale-1][FORWARD];

	case TF_LEFT:
		return pActor->talkReels[pActor->scale-1][LEFTREEL];

	case TF_RIGHT:
		return pActor->talkReels[pActor->scale-1][RIGHTREEL];

	default:
		error("GetMoverTalkReel() - illegal direction");
	}
}

/**
 * scalingreels
 */
void SetScalingReels(int actor, int scale, int direction,
		SCNHANDLE left, SCNHANDLE right, SCNHANDLE forward, SCNHANDLE away) {
	assert(scale >= 1 && scale <= NUM_MAINSCALES); // invalid scale
	assert(!(scale == 1 && direction == D_UP) &&
		!(scale == NUM_MAINSCALES && direction == D_DOWN)); // illegal direction from scale

	assert(g_scrEntries < MAX_SCRENTRIES); // Scaling reels limit reached!

	g_SCIdata[g_scrEntries].actor = actor;
	g_SCIdata[g_scrEntries].scale = scale;
	g_SCIdata[g_scrEntries].direction = direction;
	g_SCIdata[g_scrEntries].reels[LEFTREEL]	= left;
	g_SCIdata[g_scrEntries].reels[RIGHTREEL]	= right;
	g_SCIdata[g_scrEntries].reels[FORWARD]	= forward;
	g_SCIdata[g_scrEntries].reels[AWAY]		= away;
	g_scrEntries++;
}

/**
 * ScalingReel
 */
SCNHANDLE ScalingReel(int ano, int scale1, int scale2, DIRECTION reel) {
	int d;	// Direction

	// The smaller the number, the bigger the scale
	if (scale1 < scale2)
		d = D_DOWN;
	else
		d = D_UP;

	for (int i = 0; i < g_scrEntries; i++)	{
		if (g_SCIdata[i].actor == ano && g_SCIdata[i].scale == scale1 && g_SCIdata[i].direction == d) {
			if (g_SCIdata[i].reels[reel] == TF_NONE)
				return 0;
			else
				return g_SCIdata[i].reels[reel];
		}
	}
	return 0;
}

/**
 * RebootScalingReels
 */
void RebootScalingReels() {
	g_scrEntries = 0;
	memset(g_SCIdata, 0, sizeof(g_SCIdata));
}

/**
 * Discourage them from being ditched.
 */
void TouchMoverReels() {
	PMOVER	pMover;
	int	scale;

	pMover = NextMover(NULL);

	do {
		for (scale = 0; scale < TOTAL_SCALES; scale++) {
			TouchMem(pMover->walkReels[scale][LEFTREEL]);
		}
	} while ((pMover = NextMover(pMover)) != NULL);
}

} // End of namespace Tinsel
