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

#ifndef MT32EMU_PARTIALMANAGER_H
#define MT32EMU_PARTIALMANAGER_H

namespace MT32Emu {

class Synth;

class PartialManager {
private:
	Synth *synth; // Only used for sending debug output
	Part **parts;

	Partial *partialTable[MT32EMU_MAX_PARTIALS];
	Bit8u numReservedPartialsForPart[9];

	bool abortFirstReleasingPolyWhereReserveExceeded(int minPart);
	bool abortFirstPolyPreferHeldWhereReserveExceeded(int minPart);

public:

	PartialManager(Synth *synth, Part **parts);
	~PartialManager();
	Partial *allocPartial(int partNum);
	unsigned int getFreePartialCount(void);
	void getPerPartPartialUsage(unsigned int perPartPartialUsage[9]);
	bool freePartials(unsigned int needed, int partNum);
	unsigned int setReserve(Bit8u *rset);
	void deactivateAll();
	bool produceOutput(int i, float *leftBuf, float *rightBuf, Bit32u bufferLength);
	bool shouldReverb(int i);
	void clearAlreadyOutputed();
	const Partial *getPartial(unsigned int partialNum) const;
};

}

#endif
