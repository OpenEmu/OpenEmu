/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sci/engine/state.h"
#include "sci/engine/kernel.h"

namespace Sci {

reg_t kRandom(EngineState *s, int argc, reg_t *argv) {
	switch (argc) {
	case 1: // set seed to argv[0]
		// SCI0/SCI01 just reset the seed to 0 instead of using argv[0] at all
		return NULL_REG;

	case 2: { // get random number
		// numbers are definitely unsigned, for example lsl5 door code in k rap radio is random
		//  and 5-digit - we get called kRandom(10000, 65000)
		//  some codes in sq4 are also random and 5 digit (if i remember correctly)
		const uint16 fromNumber = argv[0].toUint16();
		const uint16 toNumber = argv[1].toUint16();
		// Some scripts may request a range in the reverse order (from largest
		// to smallest). An example can be found in Longbow, room 710, where a
		// random number is requested from 119 to 83. In this case, we're
		// supposed to return toNumber (determined by the KQ5CD disasm).
		// Fixes bug #3413020.
		if (fromNumber > toNumber)
			return make_reg(0, toNumber);

		uint16 range = toNumber - fromNumber + 1;
		// calculating range is exactly how sierra sci did it and is required for hoyle 4
		//  where we get called with kRandom(0, -1) and we are supposed to give back values from 0 to 0
		//  the returned value will be used as displace-offset for a background cel
		//  note: i assume that the hoyle4 code is actually buggy and it was never fixed because of
		//         the way sierra sci handled it - "it just worked". It should have called kRandom(0, 0)
		if (range)
			range--; // the range value was never returned, our random generator gets 0->range, so fix it

		const int randomNumber = fromNumber + (int)g_sci->getRNG().getRandomNumber(range);
		return make_reg(0, randomNumber);
	}

	case 3: // get seed
		// SCI0/01 did not support this at all
		// Actually we would have to return the previous seed
		error("kRandom: scripts asked for previous seed");
		break;

	default:
		error("kRandom: unsupported argc");
	}
}

reg_t kAbs(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, ABS(argv[0].toSint16()));
}

reg_t kSqrt(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, (int16) sqrt((float) ABS(argv[0].toSint16())));
}

/**
 * Returns the angle (in degrees) between the two points determined by (x1, y1)
 * and (x2, y2). The angle ranges from 0 to 359 degrees.
 * What this function does is pretty simple but apparently the original is not
 * accurate.
 */
uint16 kGetAngleWorker(int16 x1, int16 y1, int16 x2, int16 y2) {
	// SCI1 games (QFG2 and newer) use a simple atan implementation. SCI0 games
	// use a somewhat less accurate calculation (below).
	if (getSciVersion() >= SCI_VERSION_1_EGA_ONLY)
		return (int16)(360 - atan2((double)(x1 - x2), (double)(y1 - y2)) * 57.2958) % 360;

	int16 xRel = x2 - x1;
	int16 yRel = y1 - y2; // y-axis is mirrored.
	int16 angle;

	// Move (xrel, yrel) to first quadrant.
	if (y1 < y2)
		yRel = -yRel;
	if (x2 < x1)
		xRel = -xRel;

	// Compute angle in grads.
	if (yRel == 0 && xRel == 0)
		return 0;
	else
		angle = 100 * xRel / (xRel + yRel);

	// Fix up angle for actual quadrant of (xRel, yRel).
	if (y1 < y2)
		angle = 200 - angle;
	if (x2 < x1)
		angle = 400 - angle;

	// Convert from grads to degrees by merging grad 0 with grad 1,
	// grad 10 with grad 11, grad 20 with grad 21, etc. This leads to
	// "degrees" that equal either one or two grads.
	angle -= (angle + 9) / 10;
	return angle;
}

reg_t kGetAngle(EngineState *s, int argc, reg_t *argv) {
	// Based on behavior observed with a test program created with
	// SCI Studio.
	int x1 = argv[0].toSint16();
	int y1 = argv[1].toSint16();
	int x2 = argv[2].toSint16();
	int y2 = argv[3].toSint16();

	return make_reg(0, kGetAngleWorker(x1, y1, x2, y2));
}

reg_t kGetDistance(EngineState *s, int argc, reg_t *argv) {
	int xdiff = (argc > 3) ? argv[3].toSint16() : 0;
	int ydiff = (argc > 2) ? argv[2].toSint16() : 0;
	int angle = (argc > 5) ? argv[5].toSint16() : 0;
	int xrel = (int)(((float) argv[1].toSint16() - xdiff) / cos(angle * M_PI / 180.0)); // This works because cos(0)==1
	int yrel = argv[0].toSint16() - ydiff;
	return make_reg(0, (int16)sqrt((float) xrel*xrel + yrel*yrel));
}

reg_t kTimesSin(EngineState *s, int argc, reg_t *argv) {
	int angle = argv[0].toSint16();
	int factor = argv[1].toSint16();

	return make_reg(0, (int16)(factor * sin(angle * M_PI / 180.0)));
}

reg_t kTimesCos(EngineState *s, int argc, reg_t *argv) {
	int angle = argv[0].toSint16();
	int factor = argv[1].toSint16();

	return make_reg(0, (int16)(factor * cos(angle * M_PI / 180.0)));
}

reg_t kCosDiv(EngineState *s, int argc, reg_t *argv) {
	int angle = argv[0].toSint16();
	int value = argv[1].toSint16();
	double cosval = cos(angle * M_PI / 180.0);

	if ((cosval < 0.0001) && (cosval > -0.0001)) {
		error("kCosDiv: Attempted division by zero");
		return SIGNAL_REG;
	} else
		return make_reg(0, (int16)(value / cosval));
}

reg_t kSinDiv(EngineState *s, int argc, reg_t *argv) {
	int angle = argv[0].toSint16();
	int value = argv[1].toSint16();
	double sinval = sin(angle * M_PI / 180.0);

	if ((sinval < 0.0001) && (sinval > -0.0001)) {
		error("kSinDiv: Attempted division by zero");
		return SIGNAL_REG;
	} else
		return make_reg(0, (int16)(value / sinval));
}

reg_t kTimesTan(EngineState *s, int argc, reg_t *argv) {
	int param = argv[0].toSint16();
	int scale = (argc > 1) ? argv[1].toSint16() : 1;

	param -= 90;
	if ((param % 90) == 0) {
		error("kTimesTan: Attempted tan(pi/2)");
		return SIGNAL_REG;
	} else
		return make_reg(0, (int16) - (tan(param * M_PI / 180.0) * scale));
}

reg_t kTimesCot(EngineState *s, int argc, reg_t *argv) {
	int param = argv[0].toSint16();
	int scale = (argc > 1) ? argv[1].toSint16() : 1;

	if ((param % 90) == 0) {
		error("kTimesCot: Attempted tan(pi/2)");
		return SIGNAL_REG;
	} else
		return make_reg(0, (int16)(tan(param * M_PI / 180.0) * scale));
}

#ifdef ENABLE_SCI32

reg_t kMulDiv(EngineState *s, int argc, reg_t *argv) {
	int16 multiplicant = argv[0].toSint16();
	int16 multiplier = argv[1].toSint16();
	int16 denominator = argv[2].toSint16();

	// Sanity check...
	if (!denominator) {
		error("kMulDiv: attempt to divide by zero (%d * %d / %d", multiplicant, multiplier, denominator);
		return NULL_REG;
	}

	return make_reg(0, multiplicant * multiplier / denominator);
}

#endif

} // End of namespace Sci
