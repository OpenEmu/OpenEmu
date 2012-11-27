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

#ifndef MT32EMU_TVP_H
#define MT32EMU_TVP_H

namespace MT32Emu {

class TVP {
private:
	const Partial * const partial;
	const MemParams::System * const system_; // FIXME: Only necessary because masterTune calculation is done in the wrong place atm.

	const Part *part;
	const TimbreParam::PartialParam *partialParam;
	const MemParams::PatchTemp *patchTemp;

	int maxCounter;
	int processTimerIncrement;
	int counter;
	Bit32u timeElapsed;

	int phase;
	Bit32u basePitch;
	Bit32s targetPitchOffsetWithoutLFO;
	Bit32s currentPitchOffset;

	Bit16s lfoPitchOffset;
	// In range -12 - 36
	Bit8s timeKeyfollowSubtraction;

	Bit16s pitchOffsetChangePerBigTick;
	Bit16u targetPitchOffsetReachedBigTick;
	unsigned int shifts;

	Bit16u pitch;

	void updatePitch();
	void setupPitchChange(int targetPitchOffset, Bit8u changeDuration);
	void targetPitchOffsetReached();
	void nextPhase();
	void process();
public:
	TVP(const Partial *partial);
	void reset(const Part *part, const TimbreParam::PartialParam *partialParam);
	Bit32u getBasePitch() const;
	Bit16u nextPitch();
	void startDecay();
};

}

#endif
