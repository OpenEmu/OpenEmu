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

#include "mt32emu.h"

namespace MT32Emu {

Poly::Poly(Part *usePart) {
	part = usePart;
	key = 255;
	velocity = 255;
	sustain = false;
	activePartialCount = 0;
	for (int i = 0; i < 4; i++) {
		partials[i] = NULL;
	}
	state = POLY_Inactive;
}

void Poly::reset(unsigned int newKey, unsigned int newVelocity, bool newSustain, Partial **newPartials) {
	if (isActive()) {
		// FIXME: Throw out some big ugly debug output with a lot of exclamation marks - we should never get here
		terminate();
	}

	key = newKey;
	velocity = newVelocity;
	sustain = newSustain;

	activePartialCount = 0;
	for (int i = 0; i < 4; i++) {
		partials[i] = newPartials[i];
		if (newPartials[i] != NULL) {
			activePartialCount++;
			state = POLY_Playing;
		}
	}
}

bool Poly::noteOff(bool pedalHeld) {
	// Generally, non-sustaining instruments ignore note off. They die away eventually anyway.
	// Key 0 (only used by special cases on rhythm part) reacts to note off even if non-sustaining or pedal held.
	if (state == POLY_Inactive || state == POLY_Releasing) {
		return false;
	}
	if (pedalHeld) {
		if (state == POLY_Held) {
			return false;
		}
		state = POLY_Held;
	} else {
		startDecay();
	}
	return true;
}

bool Poly::stopPedalHold() {
	if (state != POLY_Held) {
		return false;
	}
	return startDecay();
}

bool Poly::startDecay() {
	if (state == POLY_Inactive || state == POLY_Releasing) {
		return false;
	}
	state = POLY_Releasing;

	for (int t = 0; t < 4; t++) {
		Partial *partial = partials[t];
		if (partial != NULL) {
			partial->startDecayAll();
		}
	}
	return true;
}

bool Poly::startAbort() {
	if (state == POLY_Inactive) {
		return false;
	}
	for (int t = 0; t < 4; t++) {
		Partial *partial = partials[t];
		if (partial != NULL) {
			partial->startAbort();
		}
	}
	return true;
}

void Poly::terminate() {
	if (state == POLY_Inactive) {
		return;
	}
	for (int t = 0; t < 4; t++) {
		Partial *partial = partials[t];
		if (partial != NULL) {
			partial->deactivate();
		}
	}
	if (state != POLY_Inactive) {
		// FIXME: Throw out lots of debug output - this should never happen
		// (Deactivating the partials above should've made them each call partialDeactivated(), ultimately changing the state to POLY_Inactive)
		state = POLY_Inactive;
	}
}

void Poly::backupCacheToPartials(PatchCache cache[4]) {
	for (int partialNum = 0; partialNum < 4; partialNum++) {
		Partial *partial = partials[partialNum];
		if (partial != NULL && partial->patchCache == &cache[partialNum]) {
			partial->cachebackup = cache[partialNum];
			partial->patchCache = &partial->cachebackup;
		}
	}
}

/**
 * Returns the internal key identifier.
 * For non-rhythm, this is within the range 12 to 108.
 * For rhythm on MT-32, this is 0 or 1 (special cases) or within the range 24 to 87.
 * For rhythm on devices with extended PCM sounds (e.g. CM-32L), this is 0, 1 or 24 to 108
 */
unsigned int Poly::getKey() const {
	return key;
}

unsigned int Poly::getVelocity() const {
	return velocity;
}

bool Poly::canSustain() const {
	return sustain;
}

PolyState Poly::getState() const {
	return state;
}

unsigned int Poly::getActivePartialCount() const {
	return activePartialCount;
}

bool Poly::isActive() const {
	return state != POLY_Inactive;
}

// This is called by Partial to inform the poly that the Partial has deactivated
void Poly::partialDeactivated(Partial *partial) {
	for (int i = 0; i < 4; i++) {
		if (partials[i] == partial) {
			partials[i] = NULL;
			activePartialCount--;
		}
	}
	if (activePartialCount == 0) {
		state = POLY_Inactive;
	}
	part->partialDeactivated(this);
}

}
