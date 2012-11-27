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
 */

#include <time.h>
#include <psptypes.h>
#include <psprtc.h>

#include "common/scummsys.h"
#include "backends/platform/psp/rtc.h"

//#define __PSP_DEBUG_FUNCS__	/* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */

#include "backends/platform/psp/trace.h"


// Class PspRtc ---------------------------------------------------------------
namespace Common {
DECLARE_SINGLETON(PspRtc);
}

void PspRtc::init() {						// init our starting ticks
	uint32 ticks[2];
	sceRtcGetCurrentTick((u64 *)ticks);

	_startMillis = ticks[0]/1000;
	_startMicros = ticks[0];
	//_lastMillis = ticks[0]/1000;	//debug - only when we don't subtract startMillis
}

#define MS_LOOP_AROUND 4294967				/* We loop every 2^32 / 1000 = 71 minutes */
#define MS_LOOP_CHECK  60000				/* Threading can cause weird mixups without this */

// Note that after we fill up 32 bits ie 50 days we'll loop back to 0, which may cause
// unpredictable results
uint32 PspRtc::getMillis() {
	uint32 ticks[2];

	sceRtcGetCurrentTick((u64 *)ticks);		// can introduce weird thread delays

	uint32 millis = ticks[0]/1000;
	millis -= _startMillis;					// get ms since start of program

	if ((int)_lastMillis - (int)millis > MS_LOOP_CHECK) {		// we must have looped around
		if (_looped == false) {					// check to make sure threads do this once
			_looped = true;
			_milliOffset += MS_LOOP_AROUND;		// add the needed offset
			PSP_DEBUG_PRINT("looping around. last ms[%d], curr ms[%d]\n", _lastMillis, millis);
		}
	} else {
		_looped = false;
	}

	_lastMillis = millis;

	return millis + _milliOffset;
}

uint32 PspRtc::getMicros() {
	uint32 ticks[2];

	sceRtcGetCurrentTick((u64 *)ticks);
	ticks[0] -= _startMicros;

	return ticks[0];
}
