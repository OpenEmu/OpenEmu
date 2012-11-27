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
 * This class emulates the calculations performed by the 8095 microcontroller in order to configure the LA-32's amplitude ramp for a single partial at each stage of its TVA envelope.
 * Unless we introduced bugs, it should be pretty much 100% accurate according to Mok's specifications.
*/
//#include <cmath>

#include "mt32emu.h"
#include "mmath.h"

namespace MT32Emu {

// CONFIRMED: Matches a table in ROM - haven't got around to coming up with a formula for it yet.
static Bit8u biasLevelToAmpSubtractionCoeff[13] = {255, 187, 137, 100, 74, 54, 40, 29, 21, 15, 10, 5, 0};

TVA::TVA(const Partial *usePartial, LA32Ramp *useAmpRamp) :
	partial(usePartial), ampRamp(useAmpRamp), system_(&usePartial->getSynth()->mt32ram.system) {
}

void TVA::startRamp(Bit8u newTarget, Bit8u newIncrement, int newPhase) {
	target = newTarget;
	phase = newPhase;
	ampRamp->startRamp(newTarget, newIncrement);
#if MT32EMU_MONITOR_TVA >= 1
	partial->getSynth()->printDebug("[+%lu] [Partial %d] TVA,ramp,%d,%d,%d,%d", partial->debugGetSampleNum(), partial->debugGetPartialNum(), (newIncrement & 0x80) ? -1 : 1, (newIncrement & 0x7F), newPhase);
#endif
}

void TVA::end(int newPhase) {
	phase = newPhase;
	playing = false;
#if MT32EMU_MONITOR_TVA >= 1
	partial->getSynth()->printDebug("[+%lu] [Partial %d] TVA,end,%d", partial->debugGetSampleNum(), partial->debugGetPartialNum(), newPhase);
#endif
}

static int multBias(Bit8u biasLevel, int bias) {
	return (bias * biasLevelToAmpSubtractionCoeff[biasLevel]) >> 5;
}

static int calcBiasAmpSubtraction(Bit8u biasPoint, Bit8u biasLevel, int key) {
	if ((biasPoint & 0x40) == 0) {
		int bias = biasPoint + 33 - key;
		if (bias > 0) {
			return multBias(biasLevel, bias);
		}
	} else {
		int bias = biasPoint - 31 - key;
		if (bias < 0) {
			bias = -bias;
			return multBias(biasLevel, bias);
		}
	}
	return 0;
}

static int calcBiasAmpSubtractions(const TimbreParam::PartialParam *partialParam, int key) {
	int biasAmpSubtraction1 = calcBiasAmpSubtraction(partialParam->tva.biasPoint1, partialParam->tva.biasLevel1, key);
	if (biasAmpSubtraction1 > 255) {
		return 255;
	}
	int biasAmpSubtraction2 = calcBiasAmpSubtraction(partialParam->tva.biasPoint2, partialParam->tva.biasLevel2, key);
	if (biasAmpSubtraction2 > 255) {
		return 255;
	}
	int biasAmpSubtraction = biasAmpSubtraction1 + biasAmpSubtraction2;
	if (biasAmpSubtraction > 255) {
		return 255;
	}
	return biasAmpSubtraction;
}

static int calcVeloAmpSubtraction(Bit8u veloSensitivity, unsigned int velocity) {
	// FIXME:KG: Better variable names
	int velocityMult = veloSensitivity - 50;
	int absVelocityMult = velocityMult < 0 ? -velocityMult : velocityMult;
	velocityMult = (signed)((unsigned)(velocityMult * ((signed)velocity - 64)) << 2);
	return absVelocityMult - (velocityMult >> 8); // PORTABILITY NOTE: Assumes arithmetic shift
}

static int calcBasicAmp(const Tables *tables, const Partial *partial, const MemParams::System *system_, const TimbreParam::PartialParam *partialParam, const MemParams::PatchTemp *patchTemp, const MemParams::RhythmTemp *rhythmTemp, int biasAmpSubtraction, int veloAmpSubtraction, Bit8u expression) {
	int amp = 155;

	if (!partial->isRingModulatingSlave()) {
		amp -= tables->masterVolToAmpSubtraction[system_->masterVol];
		if (amp < 0) {
			return 0;
		}
		amp -= tables->levelToAmpSubtraction[patchTemp->outputLevel];
		if (amp < 0) {
			return 0;
		}
		amp -= tables->levelToAmpSubtraction[expression];
		if (amp < 0) {
			return 0;
		}
		if (rhythmTemp != NULL) {
			amp -= tables->levelToAmpSubtraction[rhythmTemp->outputLevel];
			if (amp < 0) {
				return 0;
			}
		}
	}
	amp -= biasAmpSubtraction;
	if (amp < 0) {
		return 0;
	}
	amp -= tables->levelToAmpSubtraction[partialParam->tva.level];
	if (amp < 0) {
		return 0;
	}
	amp -= veloAmpSubtraction;
	if (amp < 0) {
		return 0;
	}
	if (amp > 155) {
		amp = 155;
	}
	amp -= partialParam->tvf.resonance >> 1;
	if (amp < 0) {
		return 0;
	}
	return amp;
}

int calcKeyTimeSubtraction(Bit8u envTimeKeyfollow, int key) {
	if (envTimeKeyfollow == 0) {
		return 0;
	}
	return (key - 60) >> (5 - envTimeKeyfollow); // PORTABILITY NOTE: Assumes arithmetic shift
}

void TVA::reset(const Part *newPart, const TimbreParam::PartialParam *newPartialParam, const MemParams::RhythmTemp *newRhythmTemp) {
	part = newPart;
	partialParam = newPartialParam;
	patchTemp = newPart->getPatchTemp();
	rhythmTemp = newRhythmTemp;

	playing = true;

	const Tables *tables = &Tables::getInstance();

	int key = partial->getPoly()->getKey();
	int velocity = partial->getPoly()->getVelocity();

	keyTimeSubtraction = calcKeyTimeSubtraction(partialParam->tva.envTimeKeyfollow, key);

	biasAmpSubtraction = calcBiasAmpSubtractions(partialParam, key);
	veloAmpSubtraction = calcVeloAmpSubtraction(partialParam->tva.veloSensitivity, velocity);

	int newTarget = calcBasicAmp(tables, partial, system_, partialParam, patchTemp, newRhythmTemp, biasAmpSubtraction, veloAmpSubtraction, part->getExpression());
	int newPhase;
	if (partialParam->tva.envTime[0] == 0) {
		// Initially go to the TVA_PHASE_ATTACK target amp, and spend the next phase going from there to the TVA_PHASE_2 target amp
		// Note that this means that velocity never affects time for this partial.
		newTarget += partialParam->tva.envLevel[0];
		newPhase = TVA_PHASE_ATTACK; // The first target used in nextPhase() will be TVA_PHASE_2
	} else {
		// Initially go to the base amp determined by TVA level, part volume, etc., and spend the next phase going from there to the full TVA_PHASE_ATTACK target amp.
		newPhase = TVA_PHASE_BASIC; // The first target used in nextPhase() will be TVA_PHASE_ATTACK
	}

	ampRamp->reset();//currentAmp = 0;

	// "Go downward as quickly as possible".
	// Since the current value is 0, the LA32Ramp will notice that we're already at or below the target and trying to go downward,
	// and therefore jump to the target immediately and raise an interrupt.
	startRamp((Bit8u)newTarget, 0x80 | 127, newPhase);
}

void TVA::startAbort() {
	startRamp(64, 0x80 | 127, TVA_PHASE_RELEASE);
}

void TVA::startDecay() {
	if (phase >= TVA_PHASE_RELEASE) {
		return;
	}
	Bit8u newIncrement;
	if (partialParam->tva.envTime[4] == 0) {
		newIncrement = 1;
	} else {
		newIncrement = -partialParam->tva.envTime[4];
	}
	// The next time nextPhase() is called, it will think TVA_PHASE_RELEASE has finished and the partial will be aborted
	startRamp(0, newIncrement, TVA_PHASE_RELEASE);
}

void TVA::handleInterrupt() {
	nextPhase();
}

void TVA::recalcSustain() {
	// We get pinged periodically by the pitch code to recalculate our values when in sustain.
	// This is done so that the TVA will respond to things like MIDI expression and volume changes while it's sustaining, which it otherwise wouldn't do.

	// The check for envLevel[3] == 0 strikes me as slightly dumb. FIXME: Explain why
	if (phase != TVA_PHASE_SUSTAIN || partialParam->tva.envLevel[3] == 0) {
		return;
	}
	// We're sustaining. Recalculate all the values
	const Tables *tables = &Tables::getInstance();
	int newTarget = calcBasicAmp(tables, partial, system_, partialParam, patchTemp, rhythmTemp, biasAmpSubtraction, veloAmpSubtraction, part->getExpression());
	newTarget += partialParam->tva.envLevel[3];
	// Since we're in TVA_PHASE_SUSTAIN at this point, we know that target has been reached and an interrupt fired, so we can rely on it being the current amp.
	int targetDelta = newTarget - target;

	// Calculate an increment to get to the new amp value in a short, more or less consistent amount of time
	Bit8u newIncrement;
	if (targetDelta >= 0) {
		newIncrement = tables->envLogarithmicTime[(Bit8u)targetDelta] - 2;
	} else {
		newIncrement = (tables->envLogarithmicTime[(Bit8u)-targetDelta] - 2) | 0x80;
	}
	// Configure so that once the transition's complete and nextPhase() is called, we'll just re-enter sustain phase (or decay phase, depending on parameters at the time).
	startRamp(newTarget, newIncrement, TVA_PHASE_SUSTAIN - 1);
}

bool TVA::isPlaying() const {
	return playing;
}

int TVA::getPhase() const {
	return phase;
}

void TVA::nextPhase() {
	const Tables *tables = &Tables::getInstance();

	if (phase >= TVA_PHASE_DEAD || !playing) {
		partial->getSynth()->printDebug("TVA::nextPhase(): Shouldn't have got here with phase %d, playing=%s", phase, playing ? "true" : "false");
		return;
	}
	int newPhase = phase + 1;

	if (newPhase == TVA_PHASE_DEAD) {
		end(newPhase);
		return;
	}

	bool allLevelsZeroFromNowOn = false;
	if (partialParam->tva.envLevel[3] == 0) {
		if (newPhase == TVA_PHASE_4) {
			allLevelsZeroFromNowOn = true;
		} else if (partialParam->tva.envLevel[2] == 0) {
			if (newPhase == TVA_PHASE_3) {
				allLevelsZeroFromNowOn = true;
			} else if (partialParam->tva.envLevel[1] == 0) {
				if (newPhase == TVA_PHASE_2) {
					allLevelsZeroFromNowOn = true;
				} else if (partialParam->tva.envLevel[0] == 0) {
					if (newPhase == TVA_PHASE_ATTACK)  { // this line added, missing in ROM - FIXME: Add description of repercussions
						allLevelsZeroFromNowOn = true;
					}
				}
			}
		}
	}

	int newTarget;
	int newIncrement = 0;
	int envPointIndex = phase;

	if (!allLevelsZeroFromNowOn) {
		newTarget = calcBasicAmp(tables, partial, system_, partialParam, patchTemp, rhythmTemp, biasAmpSubtraction, veloAmpSubtraction, part->getExpression());

		if (newPhase == TVA_PHASE_SUSTAIN || newPhase == TVA_PHASE_RELEASE) {
			if (partialParam->tva.envLevel[3] == 0) {
				end(newPhase);
				return;
			}
			if (!partial->getPoly()->canSustain()) {
				newPhase = TVA_PHASE_RELEASE;
				newTarget = 0;
				newIncrement = -partialParam->tva.envTime[4];
				if (newIncrement == 0) {
					// We can't let the increment be 0, or there would be no emulated interrupt.
					// So we do an "upward" increment, which should set the amp to 0 extremely quickly
					// and cause an "interrupt" to bring us back to nextPhase().
					newIncrement = 1;
				}
			} else {
				newTarget += partialParam->tva.envLevel[3];
				newIncrement = 0;
			}
		} else {
			newTarget += partialParam->tva.envLevel[envPointIndex];
		}
	} else {
		newTarget = 0;
	}

	if ((newPhase != TVA_PHASE_SUSTAIN && newPhase != TVA_PHASE_RELEASE) || allLevelsZeroFromNowOn) {
		int envTimeSetting = partialParam->tva.envTime[envPointIndex];

		if (newPhase == TVA_PHASE_ATTACK) {
			envTimeSetting -= ((signed)partial->getPoly()->getVelocity() - 64) >> (6 - partialParam->tva.envTimeVeloSensitivity); // PORTABILITY NOTE: Assumes arithmetic shift

			if (envTimeSetting <= 0 && partialParam->tva.envTime[envPointIndex] != 0) {
				envTimeSetting = 1;
			}
		} else {
			envTimeSetting -= keyTimeSubtraction;
		}
		if (envTimeSetting > 0) {
			int targetDelta = newTarget - target;
			if (targetDelta <= 0) {
				if (targetDelta == 0) {
					// target and newTarget are the same.
					// We can't have an increment of 0 or we wouldn't get an emulated interrupt.
					// So instead make the target one less than it really should be and set targetDelta accordingly.
					targetDelta = -1;
					newTarget--;
					if (newTarget < 0) {
						// Oops, newTarget is less than zero now, so let's do it the other way:
						// Make newTarget one more than it really should've been and set targetDelta accordingly.
						// FIXME (apparent bug in real firmware):
						// This means targetDelta will be positive just below here where it's inverted, and we'll end up using envLogarithmicTime[-1], and we'll be setting newIncrement to be descending later on, etc..
						targetDelta = 1;
						newTarget = -newTarget;
					}
				}
				targetDelta = -targetDelta;
				newIncrement = tables->envLogarithmicTime[(Bit8u)targetDelta] - envTimeSetting;
				if (newIncrement <= 0) {
					newIncrement = 1;
				}
				newIncrement = newIncrement | 0x80;
			} else {
				// FIXME: The last 22 or so entries in this table are 128 - surely that fucks things up, since that ends up being -128 signed?
				newIncrement = tables->envLogarithmicTime[(Bit8u)targetDelta] - envTimeSetting;
				if (newIncrement <= 0) {
					newIncrement = 1;
				}
			}
		} else {
			newIncrement = newTarget >= target ? (0x80 | 127) : 127;
		}

		// FIXME: What's the point of this? It's checked or set to non-zero everywhere above
		if (newIncrement == 0) {
			newIncrement = 1;
		}
	}

	startRamp((Bit8u)newTarget, (Bit8u)newIncrement, newPhase);
}

}
