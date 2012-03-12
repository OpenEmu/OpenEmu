#include "burnint.h"
#include "burn_sound.h"

INT16 Precalc[4096 *4];

// Routine used to precalculate the table used for interpolation
INT32 cmc_4p_Precalc()
{
	INT32 a, x, x2, x3;

	for (a = 0; a < 4096; a++) {
		x  = a  * 4;			// x = 0..16384
		x2 = x  * x / 16384;	// pow(x, 2);
		x3 = x2 * x / 16384;	// pow(x, 3);

		Precalc[a * 4 + 0] = (INT16)(-x / 3 + x2 / 2 - x3 / 6);
		Precalc[a * 4 + 1] = (INT16)(-x / 2 - x2     + x3 / 2 + 16384);
		Precalc[a * 4 + 2] = (INT16)( x     + x2 / 2 - x3 / 2);
		Precalc[a * 4 + 3] = (INT16)(-x / 6 + x3 / 6);
	}

	return 0;
}
