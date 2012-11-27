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

//#include <cstring>

#include "mt32emu.h"
#include "PartialManager.h"

namespace MT32Emu {

PartialManager::PartialManager(Synth *useSynth, Part **useParts) {
	synth = useSynth;
	parts = useParts;
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		partialTable[i] = new Partial(synth, i);
	}
}

PartialManager::~PartialManager(void) {
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		delete partialTable[i];
	}
}

void PartialManager::clearAlreadyOutputed() {
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		partialTable[i]->alreadyOutputed = false;
	}
}

bool PartialManager::shouldReverb(int i) {
	return partialTable[i]->shouldReverb();
}

bool PartialManager::produceOutput(int i, float *leftBuf, float *rightBuf, Bit32u bufferLength) {
	return partialTable[i]->produceOutput(leftBuf, rightBuf, bufferLength);
}

void PartialManager::deactivateAll() {
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		partialTable[i]->deactivate();
	}
}

unsigned int PartialManager::setReserve(Bit8u *rset) {
	unsigned int pr = 0;
	for (int x = 0; x <= 8; x++) {
		numReservedPartialsForPart[x] = rset[x];
		pr += rset[x];
	}
	return pr;
}

Partial *PartialManager::allocPartial(int partNum) {
	Partial *outPartial = NULL;

	// Get the first inactive partial
	for (int partialNum = 0; partialNum < MT32EMU_MAX_PARTIALS; partialNum++) {
		if (!partialTable[partialNum]->isActive()) {
			outPartial = partialTable[partialNum];
			break;
		}
	}
	if (outPartial != NULL) {
		outPartial->activate(partNum);
	}
	return outPartial;
}

unsigned int PartialManager::getFreePartialCount(void) {
	int count = 0;
	for (int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		if (!partialTable[i]->isActive()) {
			count++;
		}
	}
	return count;
}

// This function is solely used to gather data for debug output at the moment.
void PartialManager::getPerPartPartialUsage(unsigned int perPartPartialUsage[9]) {
	memset(perPartPartialUsage, 0, 9 * sizeof(unsigned int));
	for (unsigned int i = 0; i < MT32EMU_MAX_PARTIALS; i++) {
		if (partialTable[i]->isActive()) {
			perPartPartialUsage[partialTable[i]->getOwnerPart()]++;
		}
	}
}

// Finds the lowest-priority part that is exceeding its reserved partial allocation and has a poly
// in POLY_Releasing, then kills its first releasing poly.
// Parts with higher priority than minPart are not checked.
// Assumes that getFreePartials() has been called to make numReservedPartialsForPart up-to-date.
bool PartialManager::abortFirstReleasingPolyWhereReserveExceeded(int minPart) {
	if (minPart == 8) {
		// Rhythm is highest priority
		minPart = -1;
	}
	for (int partNum = 7; partNum >= minPart; partNum--) {
		int usePartNum = partNum == -1 ? 8 : partNum;
		if (parts[usePartNum]->getActivePartialCount() > numReservedPartialsForPart[usePartNum]) {
			// This part has exceeded its reserved partial count.
			// If it has any releasing polys, kill its first one and we're done.
			if (parts[usePartNum]->abortFirstPoly(POLY_Releasing)) {
				return true;
			}
		}
	}
	return false;
}

// Finds the lowest-priority part that is exceeding its reserved partial allocation and has a poly, then kills
// its first poly in POLY_Held - or failing that, its first poly in any state.
// Parts with higher priority than minPart are not checked.
// Assumes that getFreePartials() has been called to make numReservedPartialsForPart up-to-date.
bool PartialManager::abortFirstPolyPreferHeldWhereReserveExceeded(int minPart) {
	if (minPart == 8) {
		// Rhythm is highest priority
		minPart = -1;
	}
	for (int partNum = 7; partNum >= minPart; partNum--) {
		int usePartNum = partNum == -1 ? 8 : partNum;
		if (parts[usePartNum]->getActivePartialCount() > numReservedPartialsForPart[usePartNum]) {
			// This part has exceeded its reserved partial count.
			// If it has any polys, kill its first (preferably held) one and we're done.
			if (parts[usePartNum]->abortFirstPolyPreferHeld()) {
				return true;
			}
		}
	}
	return false;
}

bool PartialManager::freePartials(unsigned int needed, int partNum) {
	// CONFIRMED: Barring bugs, this matches the real LAPC-I according to information from Mok.

	// BUG: There's a bug in the LAPC-I implementation:
	// When allocating for rhythm part, or when allocating for a part that is using fewer partials than it has reserved,
	// held and playing polys on the rhythm part can potentially be aborted before releasing polys on the rhythm part.
	// This bug isn't present on MT-32.
	// I consider this to be a bug because I think that playing polys should always have priority over held polys,
	// and held polys should always have priority over releasing polys.

	// NOTE: This code generally aborts polys in parts (according to certain conditions) in the following order:
	// 7, 6, 5, 4, 3, 2, 1, 0, 8 (rhythm)
	// (from lowest priority, meaning most likely to have polys aborted, to highest priority, meaning least likely)

	if (needed == 0) {
		return true;
	}

	// Note that calling getFreePartialCount() also ensures that numReservedPartialsPerPart is up-to-date
	if (getFreePartialCount() >= needed) {
		return true;
	}

	// Note: These #ifdefs are temporary until we have proper "quirk" configuration.
	// Also, the MT-32 version isn't properly confirmed yet.
#ifdef MT32EMU_QUIRK_FREE_PARTIALS_MT32
	// On MT-32, we bail out before even killing releasing partials if the allocating part has exceeded its reserve and is configured for priority-to-earlier-polys.
	if (parts[partNum]->getActiveNonReleasingPartialCount() + needed > numReservedPartialsForPart[partNum] && (synth->getPart(partNum)->getPatchTemp()->patch.assignMode & 1)) {
		return false;
	}
#endif

	for (;;) {
#ifdef MT32EMU_QUIRK_FREE_PARTIALS_MT32
		// Abort releasing polys in parts that have exceeded their partial reservation (working backwards from part 7, with rhythm last)
		if (!abortFirstReleasingPolyWhereReserveExceeded(-1)) {
			break;
		}
#else
		// Abort releasing polys in non-rhythm parts that have exceeded their partial reservation (working backwards from part 7)
		if (!abortFirstReleasingPolyWhereReserveExceeded(0)) {
			break;
		}
#endif
		if (getFreePartialCount() >= needed) {
			return true;
		}
	}

	if (parts[partNum]->getActiveNonReleasingPartialCount() + needed > numReservedPartialsForPart[partNum]) {
		// With the new partials we're freeing for, we would end up using more partials than we have reserved.
		if (synth->getPart(partNum)->getPatchTemp()->patch.assignMode & 1) {
			// Priority is given to earlier polys, so just give up
			return false;
		}
		// Only abort held polys in the target part and parts that have a lower priority
		// (higher part number = lower priority, except for rhythm, which has the highest priority).
		for (;;) {
			if (!abortFirstPolyPreferHeldWhereReserveExceeded(partNum)) {
				break;
			}
			if (getFreePartialCount() >= needed) {
				return true;
			}
		}
		if (needed > numReservedPartialsForPart[partNum]) {
			return false;
		}
	} else {
		// At this point, we're certain that we've reserved enough partials to play our poly.
		// Check all parts from lowest to highest priority to see whether they've exceeded their
		// reserve, and abort their polys until until we have enough free partials or they're within
		// their reserve allocation.
		for (;;) {
			if (!abortFirstPolyPreferHeldWhereReserveExceeded(-1)) {
				break;
			}
			if (getFreePartialCount() >= needed) {
				return true;
			}
		}
	}

	// Abort polys in the target part until there are enough free partials for the new one
	for (;;) {
		if (!parts[partNum]->abortFirstPolyPreferHeld()) {
			break;
		}
		if (getFreePartialCount() >= needed) {
			return true;
		}
	}

	// Aww, not enough partials for you.
	return false;
}

const Partial *PartialManager::getPartial(unsigned int partialNum) const {
	if (partialNum > MT32EMU_MAX_PARTIALS - 1) {
		return NULL;
	}
	return partialTable[partialNum];
}

}
