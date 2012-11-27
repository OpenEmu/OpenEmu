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

//#include <cmath>
//#include <cstdlib>

#include "mt32emu.h"

namespace MT32Emu {

// FIXME: Add Explanation
static Bit16u lowerDurationToDivisor[] = {34078, 37162, 40526, 44194, 48194, 52556, 57312, 62499};

// These values represent unique options with no consistent pattern, so we have to use something like a table in any case.
// The table matches exactly what the manual claims (when divided by 8192):
// -1, -1/2, -1/4, 0, 1/8, 1/4, 3/8, 1/2, 5/8, 3/4, 7/8, 1, 5/4, 3/2, 2, s1, s2
// ...except for the last two entries, which are supposed to be "1 cent above 1" and "2 cents above 1", respectively. They can only be roughly approximated with this integer math.
static Bit16s pitchKeyfollowMult[] = {-8192, -4096, -2048, 0, 1024, 2048, 3072, 4096, 5120, 6144, 7168, 8192, 10240, 12288, 16384, 8198, 8226};

// Note: Keys < 60 use keyToPitchTable[60 - key], keys >= 60 use keyToPitchTable[key - 60].
// FIXME: This table could really be shorter, since we never use e.g. key 127.
static Bit16u keyToPitchTable[] = {
	    0,   341,   683,  1024,  1365,  1707,  2048,  2389,
	 2731,  3072,  3413,  3755,  4096,  4437,  4779,  5120,
	 5461,  5803,  6144,  6485,  6827,  7168,  7509,  7851,
	 8192,  8533,  8875,  9216,  9557,  9899, 10240, 10581,
	10923, 11264, 11605, 11947, 12288, 12629, 12971, 13312,
	13653, 13995, 14336, 14677, 15019, 15360, 15701, 16043,
	16384, 16725, 17067, 17408, 17749, 18091, 18432, 18773,
	19115, 19456, 19797, 20139, 20480, 20821, 21163, 21504,
	21845, 22187, 22528, 22869
};

TVP::TVP(const Partial *usePartial) :
	partial(usePartial), system_(&usePartial->getSynth()->mt32ram.system) {
	unsigned int sampleRate = usePartial->getSynth()->myProp.sampleRate;
	// We want to do processing 4000 times per second. FIXME: This is pretty arbitrary.
	maxCounter = sampleRate / 4000;
	// The timer runs at 500kHz. We only need to bother updating it every maxCounter samples, before we do processing.
	// This is how much to increment it by every maxCounter samples.
	processTimerIncrement = 500000 * maxCounter / sampleRate;
}

static Bit16s keyToPitch(unsigned int key) {
	// We're using a table to do: return round_to_nearest_or_even((key - 60) * (4096.0 / 12.0))
	// Banker's rounding is just slightly annoying to do in C++
	int k = (int)key;
	Bit16s pitch = keyToPitchTable[abs(k - 60)];
	return key < 60 ? -pitch : pitch;
}

static inline Bit32s coarseToPitch(Bit8u coarse) {
	return (coarse - 36) * 4096 / 12; // One semitone per coarse offset
}

static inline Bit32s fineToPitch(Bit8u fine) {
	return (fine - 50) * 4096 / 1200; // One cent per fine offset
}

static Bit32u calcBasePitch(const Partial *partial, const TimbreParam::PartialParam *partialParam, const MemParams::PatchTemp *patchTemp, unsigned int key) {
	Bit32s basePitch = keyToPitch(key);
	basePitch = (basePitch * pitchKeyfollowMult[partialParam->wg.pitchKeyfollow]) >> 13; // PORTABILITY NOTE: Assumes arithmetic shift
	basePitch += coarseToPitch(partialParam->wg.pitchCoarse);
	basePitch += fineToPitch(partialParam->wg.pitchFine);
	// NOTE:Mok: This is done on MT-32, but not LAPC-I:
	//pitch += coarseToPitch(patchTemp->patch.keyShift + 12);
	basePitch += fineToPitch(patchTemp->patch.fineTune);

	const ControlROMPCMStruct *controlROMPCMStruct = partial->getControlROMPCMStruct();
	if (controlROMPCMStruct != NULL) {
		basePitch += (Bit32s)((((Bit32s)controlROMPCMStruct->pitchMSB) << 8) | (Bit32s)controlROMPCMStruct->pitchLSB);
	} else {
		if ((partialParam->wg.waveform & 1) == 0) {
			basePitch += 37133; // This puts Middle C at around 261.64Hz (assuming no other modifications, masterTune of 64, etc.)
		} else {
			// Sawtooth waves are effectively double the frequency of square waves.
			// Thus we add 4096 less than for square waves here, which results in halving the frequency.
			basePitch += 33037;
		}
	}
	if (basePitch < 0) {
		basePitch = 0;
	}
	if (basePitch > 59392) {
		basePitch = 59392;
	}
	return (Bit32u)basePitch;
}

static Bit32u calcVeloMult(Bit8u veloSensitivity, unsigned int velocity) {
	if (veloSensitivity == 0 || veloSensitivity > 3) {
		// Note that on CM-32L/LAPC-I veloSensitivity is never > 3, since it's clipped to 3 by the max tables.
		return 21845; // aka floor(4096 / 12 * 64), aka ~64 semitones
	}
	// When velocity is 127, the multiplier is 21845, aka ~64 semitones (regardless of veloSensitivity).
	// The lower the velocity, the lower the multiplier. The veloSensitivity determines the amount decreased per velocity value.
	// The minimum multiplier (with velocity 0, veloSensitivity 3) is 170 (~half a semitone).
	Bit32u veloMult = 32768;
	veloMult -= (127 - velocity) << (5 + veloSensitivity);
	veloMult *= 21845;
	veloMult >>= 15;
	return veloMult;
}

static Bit32s calcTargetPitchOffsetWithoutLFO(const TimbreParam::PartialParam *partialParam, int levelIndex, unsigned int velocity) {
	int veloMult = calcVeloMult(partialParam->pitchEnv.veloSensitivity, velocity);
	int targetPitchOffsetWithoutLFO = partialParam->pitchEnv.level[levelIndex] - 50;
	targetPitchOffsetWithoutLFO = (Bit32s)(targetPitchOffsetWithoutLFO * veloMult) >> (16 - partialParam->pitchEnv.depth); // PORTABILITY NOTE: Assumes arithmetic shift
	return targetPitchOffsetWithoutLFO;
}

void TVP::reset(const Part *usePart, const TimbreParam::PartialParam *usePartialParam) {
	part = usePart;
	partialParam = usePartialParam;
	patchTemp = part->getPatchTemp();

	unsigned int key = partial->getPoly()->getKey();
	unsigned int velocity = partial->getPoly()->getVelocity();

	// FIXME: We're using a per-TVP timer instead of a system-wide one for convenience.
	timeElapsed = 0;

	basePitch = calcBasePitch(partial, partialParam, patchTemp, key);
	currentPitchOffset = calcTargetPitchOffsetWithoutLFO(partialParam, 0, velocity);
	targetPitchOffsetWithoutLFO = currentPitchOffset;
	phase = 0;

	if (partialParam->pitchEnv.timeKeyfollow) {
		timeKeyfollowSubtraction = (key - 60) >> (5 - partialParam->pitchEnv.timeKeyfollow); // PORTABILITY NOTE: Assumes arithmetic shift
	} else {
		timeKeyfollowSubtraction = 0;
	}
	lfoPitchOffset = 0;
	counter = 0;
	pitch = basePitch;

	// These don't really need to be initialised, but it aids debugging.
	pitchOffsetChangePerBigTick = 0;
	targetPitchOffsetReachedBigTick = 0;
	shifts = 0;
}

Bit32u TVP::getBasePitch() const {
	return basePitch;
}

void TVP::updatePitch() {
	Bit32s newPitch = basePitch + currentPitchOffset;
	if (!partial->isPCM() || (partial->getControlROMPCMStruct()->len & 0x01) == 0) { // FIXME: Use !partial->pcmWaveEntry->unaffectedByMasterTune instead
		// FIXME: masterTune recalculation doesn't really happen here, and there are various bugs not yet emulated
		// 171 is ~half a semitone.
		newPitch += ((system_->masterTune - 64) * 171) >> 6; // PORTABILITY NOTE: Assumes arithmetic shift.
	}
	if ((partialParam->wg.pitchBenderEnabled & 1) != 0) {
		newPitch += part->getPitchBend();
	}
	if (newPitch < 0) {
		newPitch = 0;
	}

// Note: Temporary #ifdef until we have proper "quirk" configuration
// This is about right emulation of MT-32 GEN0 quirk exploited in Colonel's Bequest timbre "Lightning"
#ifndef MT32EMU_QUIRK_PITCH_ENVELOPE_OVERFLOW_MT32
	if (newPitch > 59392) {
		newPitch = 59392;
	}
#endif
	pitch = (Bit16u)newPitch;

	// FIXME: We're doing this here because that's what the CM-32L does - we should probably move this somewhere more appropriate in future.
	partial->tva->recalcSustain();
}

void TVP::targetPitchOffsetReached() {
	currentPitchOffset = targetPitchOffsetWithoutLFO + lfoPitchOffset;

	switch (phase) {
	case 3:
	case 4:
	{
		int newLFOPitchOffset = (part->getModulation() * partialParam->pitchLFO.modSensitivity) >> 7;
		newLFOPitchOffset = (newLFOPitchOffset + partialParam->pitchLFO.depth) << 1;
		if (pitchOffsetChangePerBigTick > 0) {
			// Go in the opposite direction to last time
			newLFOPitchOffset = -newLFOPitchOffset;
		}
		lfoPitchOffset = newLFOPitchOffset;
		int targetPitchOffset = targetPitchOffsetWithoutLFO + lfoPitchOffset;
		setupPitchChange(targetPitchOffset, 101 - partialParam->pitchLFO.rate);
		updatePitch();
		break;
	}
	case 6:
		updatePitch();
		break;
	default:
		nextPhase();
	}
}

void TVP::nextPhase() {
	phase++;
	int envIndex = phase == 6 ? 4 : phase;

	targetPitchOffsetWithoutLFO = calcTargetPitchOffsetWithoutLFO(partialParam, envIndex, partial->getPoly()->getVelocity()); // pitch we'll reach at the end

	int changeDuration = partialParam->pitchEnv.time[envIndex - 1];
	changeDuration -= timeKeyfollowSubtraction;
	if (changeDuration > 0) {
		setupPitchChange(targetPitchOffsetWithoutLFO, changeDuration); // changeDuration between 0 and 112 now
		updatePitch();
	} else {
		targetPitchOffsetReached();
	}
}

// Shifts val to the left until bit 31 is 1 and returns the number of shifts
static Bit8u normalise(Bit32u &val) {
	Bit8u leftShifts;
	for (leftShifts = 0; leftShifts < 31; leftShifts++) {
		if ((val & 0x80000000) != 0) {
			break;
		}
		val = val << 1;
	}
	return leftShifts;
}

void TVP::setupPitchChange(int targetPitchOffset, Bit8u changeDuration) {
	bool negativeDelta = targetPitchOffset < currentPitchOffset;
	Bit32s pitchOffsetDelta = targetPitchOffset - currentPitchOffset;
	if (pitchOffsetDelta > 32767 || pitchOffsetDelta < -32768) {
		pitchOffsetDelta = 32767;
	}
	if (negativeDelta) {
		pitchOffsetDelta = -pitchOffsetDelta;
	}
	// We want to maximise the number of bits of the Bit16s "pitchOffsetChangePerBigTick" we use in order to get the best possible precision later
	Bit32u absPitchOffsetDelta = pitchOffsetDelta << 16;
	Bit8u normalisationShifts = normalise(absPitchOffsetDelta); // FIXME: Double-check: normalisationShifts is usually between 0 and 15 here, unless the delta is 0, in which case it's 31
	absPitchOffsetDelta = absPitchOffsetDelta >> 1; // Make room for the sign bit

	changeDuration--; // changeDuration's now between 0 and 111
	unsigned int upperDuration = changeDuration >> 3; // upperDuration's now between 0 and 13
	shifts = normalisationShifts + upperDuration + 2;
	Bit16u divisor = lowerDurationToDivisor[changeDuration & 7];
	Bit16s newPitchOffsetChangePerBigTick = ((absPitchOffsetDelta & 0xFFFF0000) / divisor) >> 1; // Result now fits within 15 bits. FIXME: Check nothing's getting sign-extended incorrectly
	if (negativeDelta) {
		newPitchOffsetChangePerBigTick = -newPitchOffsetChangePerBigTick;
	}
	pitchOffsetChangePerBigTick = newPitchOffsetChangePerBigTick;

	int currentBigTick = timeElapsed >> 8;
	int durationInBigTicks = divisor >> (12 - upperDuration);
	if (durationInBigTicks > 32767) {
		durationInBigTicks = 32767;
	}
	// The result of the addition may exceed 16 bits, but wrapping is fine and intended here.
	targetPitchOffsetReachedBigTick = currentBigTick + durationInBigTicks;
}

void TVP::startDecay() {
	phase = 5;
	lfoPitchOffset = 0;
	targetPitchOffsetReachedBigTick = timeElapsed >> 8; // FIXME: Afaict there's no good reason for this - check
}

Bit16u TVP::nextPitch() {
	// FIXME: Write explanation of counter and time increment
	if (counter == 0) {
		timeElapsed += processTimerIncrement;
		timeElapsed = timeElapsed & 0x00FFFFFF;
		process();
	}
	counter = (counter + 1) % maxCounter;
	return pitch;
}

void TVP::process() {
	if (phase == 0) {
		targetPitchOffsetReached();
		return;
	}
	if (phase == 5) {
		nextPhase();
		return;
	}
	if (phase > 7) {
		updatePitch();
		return;
	}

	Bit16s negativeBigTicksRemaining = (timeElapsed >> 8) - targetPitchOffsetReachedBigTick;
	if (negativeBigTicksRemaining >= 0) {
		// We've reached the time for a phase change
		targetPitchOffsetReached();
		return;
	}
	// FIXME: Write explanation for this stuff
	int rightShifts = shifts;
	if (rightShifts > 13) {
		rightShifts -= 13;
		negativeBigTicksRemaining = negativeBigTicksRemaining >> rightShifts; // PORTABILITY NOTE: Assumes arithmetic shift
		rightShifts = 13;
	}
	int newResult = ((Bit32s)(negativeBigTicksRemaining * pitchOffsetChangePerBigTick)) >> rightShifts; // PORTABILITY NOTE: Assumes arithmetic shift
	newResult += targetPitchOffsetWithoutLFO + lfoPitchOffset;
	currentPitchOffset = newResult;
	updatePitch();
}

}
