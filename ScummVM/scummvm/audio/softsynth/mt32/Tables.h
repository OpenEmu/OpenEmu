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

#ifndef MT32EMU_TABLES_H
#define MT32EMU_TABLES_H

namespace MT32Emu {

// Sample rate to use in mixing
const unsigned int SAMPLE_RATE = 32000;

const int MIDDLEC = 60;

class Synth;

class Tables {
private:
	Tables();
	Tables(Tables &);

public:
	static const Tables &getInstance();

	// Constant LUTs

	// CONFIRMED: This is used to convert several parameters to amp-modifying values in the TVA envelope:
	// - PatchTemp.outputLevel
	// - RhythmTemp.outlevel
	// - PartialParam.tva.level
	// - expression
	// It's used to determine how much to subtract from the amp envelope's target value
	Bit8u levelToAmpSubtraction[101];

	// CONFIRMED: ...
	Bit8u envLogarithmicTime[256];

	// CONFIRMED: ...
	Bit8u masterVolToAmpSubtraction[101];

	// CONFIRMED:
	Bit8u pulseWidth100To255[101];

	float exp2[4096];
	float pulseLenFactor[128];
	float resAmpMax[32];
	float resAmpFadeFactor[8];
	float sinf10[5120];
};

}

#endif
