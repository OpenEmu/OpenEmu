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

#ifndef MT32EMU_TVA_H
#define MT32EMU_TVA_H

namespace MT32Emu {

class Part;

// Note that when entering nextPhase(), newPhase is set to phase + 1, and the descriptions/names below refer to
// newPhase's value.
enum {
	// In this phase, the base amp (as calculated in calcBasicAmp()) is targeted with an instant time.
	// This phase is entered by reset() only if time[0] != 0.
	TVA_PHASE_BASIC = 0,

	// In this phase, level[0] is targeted within time[0], and velocity potentially affects time
	TVA_PHASE_ATTACK = 1,

	// In this phase, level[1] is targeted within time[1]
	TVA_PHASE_2 = 2,

	// In this phase, level[2] is targeted within time[2]
	TVA_PHASE_3 = 3,

	// In this phase, level[3] is targeted within time[3]
	TVA_PHASE_4 = 4,

	// In this phase, immediately goes to PHASE_RELEASE unless the poly is set to sustain.
	// Aborts the partial if level[3] is 0.
	// Otherwise level[3] is continued, no phase change will occur until some external influence (like pedal release)
	TVA_PHASE_SUSTAIN = 5,

	// In this phase, 0 is targeted within time[4] (the time calculation is quite different from the other phases)
	TVA_PHASE_RELEASE = 6,

	// It's PHASE_DEAD, Jim.
	TVA_PHASE_DEAD = 7
};

class TVA {
private:
	const Partial * const partial;
	LA32Ramp *ampRamp;
	const MemParams::System * const system_;

	const Part *part;
	const TimbreParam::PartialParam *partialParam;
	const MemParams::PatchTemp *patchTemp;
	const MemParams::RhythmTemp *rhythmTemp;

	bool playing;

	int biasAmpSubtraction;
	int veloAmpSubtraction;
	int keyTimeSubtraction;

	Bit8u target;
	int phase;

	void startRamp(Bit8u newTarget, Bit8u newIncrement, int newPhase);
	void end(int newPhase);
	void nextPhase();

public:
	TVA(const Partial *partial, LA32Ramp *ampRamp);
	void reset(const Part *part, const TimbreParam::PartialParam *partialParam, const MemParams::RhythmTemp *rhythmTemp);
	void handleInterrupt();
	void recalcSustain();
	void startDecay();
	void startAbort();

	bool isPlaying() const;
	int getPhase() const;
};

}

#endif /* TVA_H_ */
