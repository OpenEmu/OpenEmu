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
 * Handles timers.
 *
 * Note: As part of the transition to ScummVM, the ticks field of a timer has been changed
 * to a millisecond value, rather than ticks at 24Hz. Most places should be able to use
 * the timers without change, since the ONE_SECOND constant has been set to be in MILLISECONDS
 */

#include "tinsel/timers.h"
#include "tinsel/dw.h"
#include "common/serializer.h"
#include "common/textconsole.h"
#include "common/system.h"

namespace Tinsel {

//----------------- LOCAL DEFINES --------------------

#define MAX_TIMERS 16

struct TIMER {
	int	tno;		/**< Timer number */
	int		ticks;		/**< Tick count */
	int		secs;		/**< Second count */
	int		delta;		/**< Increment/decrement value */
	bool	frame;		/**< If set, in ticks, otherwise in seconds */
};



//----------------- LOCAL GLOBAL DATA --------------------

static TIMER g_timers[MAX_TIMERS];	// FIXME: Avoid non-const global vars


//--------------------------------------------------------

/**
 * Gets the current time in number of ticks.
 *
 * DOS timer ticks is the number of 54.9254ms since midnight. Converting the
 * millisecond count won't give the exact same 'since midnight' count, but I
 * figure that as long as the timing interval is more or less accurate, it
 * shouldn't be a problem.
 */

uint32 DwGetCurrentTime() {
	return g_system->getMillis() * 55 / 1000;
}

/**
 * Resets all of the timer slots
 */

void RebootTimers() {
	memset(g_timers, 0, sizeof(g_timers));
}

/**
 * (Un)serialize the timer data for save/restore game.
 */
void syncTimerInfo(Common::Serializer &s) {
	for (int i = 0; i < MAX_TIMERS; i++) {
		s.syncAsSint32LE(g_timers[i].tno);
		s.syncAsSint32LE(g_timers[i].ticks);
		s.syncAsSint32LE(g_timers[i].secs);
		s.syncAsSint32LE(g_timers[i].delta);
		s.syncAsSint32LE(g_timers[i].frame);
	}
}

/**
 * Find the timer numbered thus, if one is thus numbered.
 * @param num	number of the timer
 * @return the timer with the specified number, or NULL if there is none
 */
static TIMER *findTimer(int num) {
	for (int i = 0; i < MAX_TIMERS; i++) {
		if (g_timers[i].tno == num)
			return &g_timers[i];
	}
	return NULL;
}

/**
 * Find an empty timer slot.
 */
static TIMER *allocateTimer(int num) {
	assert(num); // zero is not allowed as a timer number
	assert(!findTimer(num)); // Allocating already existant timer

	for (int i = 0; i < MAX_TIMERS; i++) {
		if (!g_timers[i].tno) {
			g_timers[i].tno = num;
			return &g_timers[i];
		}
	}

	error("Too many timers");
}

/**
 * Update all timers, as appropriate.
 */
void FettleTimers() {
	for (int i = 0; i < MAX_TIMERS; i++) {
		if (!g_timers[i].tno)
			continue;

		g_timers[i].ticks += g_timers[i].delta;	// Update tick value

		if (g_timers[i].frame) {
			if (g_timers[i].ticks < 0)
				g_timers[i].ticks = 0;	// Have reached zero
		} else {
			if (g_timers[i].ticks < 0) {
				g_timers[i].ticks = ONE_SECOND;
				g_timers[i].secs--;
				if (g_timers[i].secs < 0)
					g_timers[i].secs = 0;	// Have reached zero
			} else if (g_timers[i].ticks == ONE_SECOND) {
				g_timers[i].ticks = 0;
				g_timers[i].secs++;		// Another second has passed
			}
		}
	}
}

/**
 * Start a timer up.
 */
void StartTimer(int num, int sval, bool up, bool frame) {
	TIMER *pt;

	assert(num); // zero is not allowed as a timer number

	pt = findTimer(num);
	if (pt == NULL)
		pt = allocateTimer(num);

	pt->delta = up ? 1 : -1;	// Increment/decrement value
	pt->frame = frame;

	if (frame) {
		pt->secs = 0;
		pt->ticks = sval;
	} else {
		pt->secs = sval;
		pt->ticks = 0;
	}
}

/**
 * Return the current count of a timer.
 */
int Timer(int num) {
	TIMER *pt;

	pt = findTimer(num);

	if (pt == NULL)
		return -1;

	if (pt->frame)
		return pt->ticks;
	else
		return pt->secs;
}

} // End of namespace Tinsel
