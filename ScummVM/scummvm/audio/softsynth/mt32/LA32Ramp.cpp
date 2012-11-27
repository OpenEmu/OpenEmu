/* Copyright (C) 2003, 2004, 2005, 2006, 2008, 2009 Dean Beeler, Jerome Fisher
 * Copyright (C) 2011 Dean Beeler, Jerome Fisher, Sergey V. Mikayev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
Some notes on this class:

This emulates the LA-32's implementation of "ramps". A ramp in this context is a smooth transition from one value to another, handled entirely within the LA-32.
The LA-32 provides this feature for amplitude and filter cutoff values.

The 8095 starts ramps on the LA-32 by setting two values in memory-mapped registers:

(1) The target value (between 0 and 255) for the ramp to end on. This is represented by the "target" argument to startRamp().
(2) The speed at which that value should be approached. This is represented by the "increment" argument to startRamp().

Once the ramp target value has been hit, the LA-32 raises an interrupt.

Note that the starting point of the ramp is whatever internal value the LA-32 had when the registers were set. This is usually the end point of a previously completed ramp.

Our handling of the "target" and "increment" values is based on sample analysis and a little guesswork.
Here's what we're pretty confident about:
 - The most significant bit of "increment" indicates the direction that the LA32's current internal value ("current" in our emulation) should change in.
   Set means downward, clear means upward.
 - The lower 7 bits of "increment" indicate how quickly "current" should be changed.
 - If "increment" is 0, no change to "current" is made and no interrupt is raised. [SEMI-CONFIRMED by sample analysis]
 - Otherwise, if the MSb is set:
    - If "current" already corresponds to a value <= "target", "current" is set immediately to the equivalent of "target" and an interrupt is raised.
    - Otherwise, "current" is gradually reduced (at a rate determined by the lower 7 bits of "increment"), and once it reaches the equivalent of "target" an interrupt is raised.
 - Otherwise (the MSb is unset):
    - If "current" already corresponds to a value >= "target", "current" is set immediately to the equivalent of "target" and an interrupt is raised.
    - Otherwise, "current" is gradually increased (at a rate determined by the lower 7 bits of "increment"), and once it reaches the equivalent of "target" an interrupt is raised.

We haven't fully explored:
 - Values when ramping between levels (though this is probably correct).
 - Transition timing (may not be 100% accurate, especially for very fast ramps).
*/
//#include <cmath>

#include "mt32emu.h"
#include "LA32Ramp.h"
#include "mmath.h"

namespace MT32Emu {

// SEMI-CONFIRMED from sample analysis.
const int TARGET_MULT = 0x40000;
const unsigned int MAX_CURRENT = 0xFF * TARGET_MULT;

// We simulate the delay in handling "target was reached" interrupts by waiting
// this many samples before setting interruptRaised.
// FIXME: This should vary with the sample rate, but doesn't.
// SEMI-CONFIRMED: Since this involves asynchronous activity between the LA32
// and the 8095, a good value is hard to pin down.
// This one matches observed behaviour on a few digital captures I had handy,
// and should be double-checked. We may also need a more sophisticated delay
// scheme eventually.
const int INTERRUPT_TIME = 7;

LA32Ramp::LA32Ramp() :
	current(0),
	largeTarget(0),
	largeIncrement(0),
	interruptCountdown(0),
	interruptRaised(false) {
}

void LA32Ramp::startRamp(Bit8u target, Bit8u increment) {
	// CONFIRMED: From sample analysis, this appears to be very accurate.
	if (increment == 0) {
		largeIncrement = 0;
	} else {
		// Using integer argument here, no precision loss:
		// (unsigned int)(EXP2F(((increment & 0x7F) + 24) / 8.0f) + 0.125f)
		largeIncrement = (unsigned int)(EXP2I(((increment & 0x7F) + 24) << 9) + 0.125f);
	}
	descending = (increment & 0x80) != 0;
	if (descending) {
		// CONFIRMED: From sample analysis, descending increments are slightly faster
		largeIncrement++;
	}

	largeTarget = target * TARGET_MULT;
	interruptCountdown = 0;
	interruptRaised = false;
}

Bit32u LA32Ramp::nextValue() {
	if (interruptCountdown > 0) {
		if (--interruptCountdown == 0) {
			interruptRaised = true;
		}
	} else if (largeIncrement != 0) {
		// CONFIRMED from sample analysis: When increment is 0, the LA32 does *not* change the current value at all (and of course doesn't fire an interrupt).
		if (descending) {
			// Lowering current value
			if (largeIncrement > current) {
				current = largeTarget;
				interruptCountdown = INTERRUPT_TIME;
			} else {
				current -= largeIncrement;
				if (current <= largeTarget) {
					current = largeTarget;
					interruptCountdown = INTERRUPT_TIME;
				}
			}
		} else {
			// Raising current value
			if (MAX_CURRENT - current < largeIncrement) {
				current = largeTarget;
				interruptCountdown = INTERRUPT_TIME;
			} else {
				current += largeIncrement;
				if (current >= largeTarget) {
					current = largeTarget;
					interruptCountdown = INTERRUPT_TIME;
				}
			}
		}
	}
	return current;
}

bool LA32Ramp::checkInterrupt() {
	bool wasRaised = interruptRaised;
	interruptRaised = false;
	return wasRaised;
}

void LA32Ramp::reset() {
	current = 0;
	largeTarget = 0;
	largeIncrement = 0;
	descending = false;
	interruptCountdown = 0;
	interruptRaised = false;
}

}
