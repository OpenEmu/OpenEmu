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

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

namespace Scumm {

/**
 * Logic code for:
 *     Freddi Fish's One-Stop Fun Shop
 *     Pajama Sam's One-Stop Fun Shop
 *     Putt-Putt's One-Stop Fun Shop
 */
class LogicHEfunshop : public LogicHE {
public:
	LogicHEfunshop(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);

private:
	void op_1004(int32 *args);
	void op_1005(int32 *args);
	int checkShape(int32 data0, int32 data1, int32 data4, int32 data5, int32 data2, int32 data3, int32 data6, int32 data7, int32 *x, int32 *y);
};

int LogicHEfunshop::versionID() {
	return 1;
}

int32 LogicHEfunshop::dispatch(int op, int numArgs, int32 *args) {
	switch (op) {
	case 1004:
		op_1004(args);
		break;

	case 1005:
		op_1005(args);
		break;

	default:
		break;
	}

	return 0;
}

void LogicHEfunshop::op_1004(int32 *args) {
	double data[8], at, sq;
	int32 x, y;
	int i=0;

	for (i = 0; i <= 6; i += 2) {
		data[i] = getFromArray(args[0], 0, 519 + i);
		data[i + 1] = getFromArray(args[0], 0, 519 + i + 1);
	}
	int s = checkShape((int32)data[0], (int32)data[1], (int32)data[4], (int32)data[5],
		(int32)data[2], (int32)data[3], (int32)data[6], (int32)data[7], &x, &y);

	if (s != 1) {
		error("LogicHEfunshop::op_1004: Your shape has defied the laws of physics");
		return;
	}

	for (i = 0; i <= 6; i += 2) {
		data[i] -= (double)x;
		data[i + 1] -= (double)y;
	}

	double a1 = (double)args[1] * DEG2RAD;

	for (i = 0; i <= 6; i += 2) {
		at = atan2(data[i + 1], data[i]);
		sq = sqrt(data[i + 1] * data[i + 1] + data[i] * data[i]);

		if (at <= 0)
			at += 2 * M_PI;

		data[i] = cos(at + a1) * sq;
		data[i + 1] = sin(at + a1) * sq;
	}

	double minx = data[0];
	double miny = data[1];

	for (i = 0; i <= 6; i += 2) {
		if (data[i] < minx)
			minx = data[i];
		if (data[i + 1] < miny)
			miny = data[i + 1];
	}

	for (i = 0; i <= 6; i += 2) {
		data[i] -= minx;
		data[i + 1] -= miny;

		putInArray(args[0], 0, 519 + i, scummRound(data[i]));
		putInArray(args[0], 0, 519 + i + 1, scummRound(data[i + 1]));
	}
}

void LogicHEfunshop::op_1005(int32 *args) {
	double data[8];
	double args1, args2;
	int i;
	for (i = 520; i <= 526; i += 2) {
		data[i - 520] = getFromArray(args[0], 0, i - 1);
		data[i - 520 + 1] = getFromArray(args[0], 0, i);
	}

	args1 = (double)args[1] * 0.01 + 1;
	args2 = (double)args[2] * 0.01 + 1;

	for (i = 0; i < 4; i++) {
		data[2 * i] *= args1;
		data[2 * i + 1] *= args2;
	}

	for (i = 520; i <= 526; i += 2) {
		putInArray(args[0], 0, i - 1, scummRound(data[i - 520]));
		putInArray(args[0], 0, i, scummRound(data[i - 520 + 1]));
	}
}

int LogicHEfunshop::checkShape(int32 data0, int32 data1, int32 data4, int32 data5, int32 data2, int32 data3, int32 data6, int32 data7, int32 *x, int32 *y) {
	int32 diff5_1, diff0_4, diff7_3, diff2_6;
	int32 diff1, diff2;
	int32 delta, delta2;
	int32 sum1, sum2;

	diff0_4 = data0 - data4;
	diff5_1 = data5 - data1;
	diff1 = data1 * data4 - data0 * data5;
	sum1 = diff0_4 * data3 + diff1 + diff5_1 * data2;
	sum2 = diff0_4 * data7 + diff1 + diff5_1 * data6;

	if (sum1 != 0 && sum2 != 0) {
		sum2 ^= sum1;

		if (sum2 >= 0)
			return 0;
	}

	diff2_6 = data2 - data6;
	diff7_3 = data7 - data3;
	diff2 = data3 * data6 - data2 * data7;
	sum1 = diff2_6 * data1 + diff2 + diff7_3 * data0;
	sum2 = diff2_6 * data5 + diff2 + diff7_3 * data4;

	if (sum1 != 0 && sum2 != 0) {
		sum2 ^= sum1;

		if (sum2 >= 0)
			return 0;
	}

	delta = diff2_6 * diff5_1 - diff0_4 * diff7_3;

	if (delta == 0) {
		return 2;
	}

	if (delta < 0) {
		data7 = -((delta + 1) >> 1);
	} else {
		data7 = delta >> 1;
	}

	delta2 = diff2 * diff0_4 - diff1 * diff2_6;

	if (delta2 < 0) {
		delta2 -= data7;
	} else {
		delta2 += data7;
	}

	*x = delta2 / delta;

	delta2 = diff1 * diff7_3 - diff2 * diff5_1;

	if (delta2 < 0) {
		delta2 -= data7;
	} else {
		delta2 += data7;
	}

	*y = delta2 / delta;

	return 1;
}

LogicHE *makeLogicHEfunshop(ScummEngine_v90he *vm) {
	return new LogicHEfunshop(vm);
}

} // End of namespace Scumm
