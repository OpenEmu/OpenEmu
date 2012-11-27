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
//#include <cstring>

#include "mt32emu.h"
#include "mmath.h"

namespace MT32Emu {

const Tables &Tables::getInstance() {
	static const Tables instance;
	return instance;
}

Tables::Tables() {
	int lf;
	for (lf = 0; lf <= 100; lf++) {
		// CONFIRMED:KG: This matches a ROM table found by Mok
		float fVal = (2.0f - LOG10F((float)lf + 1.0f)) * 128.0f;
		int val = (int)(fVal + 1.0);
		if (val > 255) {
			val = 255;
		}
		levelToAmpSubtraction[lf] = (Bit8u)val;
	}

	envLogarithmicTime[0] = 64;
	for (lf = 1; lf <= 255; lf++) {
		// CONFIRMED:KG: This matches a ROM table found by Mok
		envLogarithmicTime[lf] = (Bit8u)ceil(64.0f + LOG2F((float)lf) * 8.0f);
	}

#ifdef EMULATE_LAPC_I // Dummy #ifdef - we'll have runtime emulation mode selection in future.
	// CONFIRMED: Based on a table found by Mok in the LAPC-I control ROM
	// Note that this matches the MT-32 table, but with the values clamped to a maximum of 8.
	memset(masterVolToAmpSubtraction, 8, 71);
	memset(masterVolToAmpSubtraction + 71, 7, 3);
	memset(masterVolToAmpSubtraction + 74, 6, 4);
	memset(masterVolToAmpSubtraction + 78, 5, 3);
	memset(masterVolToAmpSubtraction + 81, 4, 4);
	memset(masterVolToAmpSubtraction + 85, 3, 3);
	memset(masterVolToAmpSubtraction + 88, 2, 4);
	memset(masterVolToAmpSubtraction + 92, 1, 4);
	memset(masterVolToAmpSubtraction + 96, 0, 5);
#else
	// CONFIRMED: Based on a table found by Mok in the MT-32 control ROM
	masterVolToAmpSubtraction[0] = 255;
	for (int masterVol = 1; masterVol <= 100; masterVol++) {
		masterVolToAmpSubtraction[masterVol] = (int)(106.31 - 16.0f * LOG2F((float)masterVol));
	}
#endif

	for (int i = 0; i <= 100; i++) {
		pulseWidth100To255[i] = (int)(i * 255 / 100.0f + 0.5f);
		//synth->printDebug("%d: %d", i, pulseWidth100To255[i]);
	}

	// Ratio of negative segment to wave length
	for (int i = 0; i < 128; i++) {
		// Formula determined from sample analysis.
		float pt = 0.5f / 127.0f * i;
		pulseLenFactor[i] = (1.241857812f - pt) * pt;    // seems to be 2 ^ (5 / 16) = 1.241857812f
	}

	// The LA32 chip presumably has such a table inside as the internal computaions seem to be performed using fixed point math with 12-bit fractions
	for (int i = 0; i < 4096; i++) {
		exp2[i] = EXP2F(i / 4096.0f);
	}

	// found from sample analysis
	for (int i = 0; i < 32; i++) {
		resAmpMax[i] = EXP2F(1.0f - (32 - i) / 4.0f);
	}

	// found from sample analysis
	resAmpFadeFactor[7] = 1.0f / 8.0f;
	resAmpFadeFactor[6] = 2.0f / 8.0f;
	resAmpFadeFactor[5] = 3.0f / 8.0f;
	resAmpFadeFactor[4] = 5.0f / 8.0f;
	resAmpFadeFactor[3] = 8.0f / 8.0f;
	resAmpFadeFactor[2] = 12.0f / 8.0f;
	resAmpFadeFactor[1] = 16.0f / 8.0f;
	resAmpFadeFactor[0] = 31.0f / 8.0f;

	for (int i = 0; i < 5120; i++) {
		sinf10[i] = sin(FLOAT_PI * i / 2048.0f);
	}
}

}
