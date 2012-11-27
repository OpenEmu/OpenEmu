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
 *     Putt-Putt Enters the Race
 */
class LogicHErace : public LogicHE {
private:
	float *_userData;
	double *_userDataD;
public:
	LogicHErace(ScummEngine_v90he *vm);
	~LogicHErace();

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);

private:
	int32 op_1003(int32 *args);
	int32 op_1004(int32 *args);
	int32 op_1100(int32 *args);
	int32 op_1101(int32 *args);
	int32 op_1102(int32 *args);
	int32 op_1103(int32 *args);
	int32 op_1110();
	int32 op_1120(int32 *args);
	int32 op_1130(int32 *args);
	int32 op_1140(int32 *args);

	void op_sub1(float arg);
	void op_sub2(float arg);
	void op_sub3(float arg);
};

LogicHErace::LogicHErace(ScummEngine_v90he *vm) : LogicHE(vm) {
	// Originally it used 0x930 and stored both floats and doubles inside
	_userData = (float *)calloc(550, sizeof(float));
	_userDataD = (double *)calloc(30, sizeof(double));

	// FIXME: of the 550 entries in _userData, only 516 till 532 are used
	// FIXME: similarly, in _userDataD only 9 till 17 are used for computations
	//       (some of the other entries are also set, but never read, hence useless).
}

LogicHErace::~LogicHErace() {
	free(_userData);
	free(_userDataD);
}

int LogicHErace::versionID() {
	return 1;
}

int32 LogicHErace::dispatch(int op, int numArgs, int32 *args) {
	int32 res;

	switch (op) {
	case 1003:
		res = op_1003(args);
		break;

	case 1004:
		res = op_1004(args);
		break;

	case 1100:
		res = op_1100(args);
		break;

	case 1101:
		res = op_1101(args);
		break;

	case 1102:
		res = op_1102(args);
		break;

	case 1103:
		res = op_1103(args);
		break;

	case 1110:
		res = op_1110();
		break;

	case 1120:
		res = op_1120(args);
		break;

	case 1130:
		res = op_1130(args);
		break;

	case 1140:
		res = op_1140(args);
		break;

	default:
		res = 0;
		break;

	}

	return res;
}

#define RAD2DEG (180 / M_PI)
#define DEG2RAD (M_PI / 180)

int32 LogicHErace::op_1003(int32 *args) {
	int value = args[2] ? args[2] : 1;

	writeScummVar(108, (int32)(atan2((double)args[0], (double)args[1]) * RAD2DEG * value));

	return 1;
}

int32 LogicHErace::op_1004(int32 *args) {
	int value = args[1] ? args[1] : 1;

	writeScummVar(108, (int32)(sqrt((float)args[0]) * value));

	return 1;
}

int32 LogicHErace::op_1100(int32 *args) {
	// _userData 516,517,518 describe a 3D translation?
	_userData[516] = (float)args[0] / args[10];
	_userData[517] = (float)args[1] / args[10];
	_userData[518] = (float)args[2] / args[10];

	// _userData 519,520,521 describe rotation angles around the x,y,z axes?
	_userData[519] = (float)args[3] / args[10];
	_userData[520] = (float)args[4] / args[10];
	_userData[521] = (float)args[5] / args[10];

	op_sub1(_userData[520]);
	op_sub2(_userData[521]);

	// _userData[532] seems to be some kind of global scale factor
	_userData[532] = (float)args[10];

	_userData[524] = (float)args[8];	// not used
	_userData[525] = (float)args[9];	// not used
	_userData[522] = (float)args[6] / args[10];	// not used
	_userData[523] = (float)args[7] / args[10];	// only used to compute 528 and 529

	// The following two are some kind of scale factors
	_userData[526] = (float)args[6] / args[8] / args[10];
	_userData[527] = (float)args[7] / args[9] / args[10];

	// Set var 108 and 109 -- the value set here corresponds to the values
	// set by op_1110!
	writeScummVar(108, (int32)((float)args[6] / args[8] * args[10]));
	writeScummVar(109, (int32)((float)args[7] / args[9] * args[10]));

	_userData[528] = (float)(_userData[519] - _userData[523] * 0.5);
	_userData[529] = (float)(_userData[519] + _userData[523] * 0.5);

	writeScummVar(110, (int32)(_userData[528] * args[10]));
	writeScummVar(111, (int32)(_userData[529] * args[10]));

	// 530 and 531 are only used to set vars 112 and 113, so no need
	// to store them permanently
	_userData[530] = (float)(_userData[517] / tan(_userData[529] * DEG2RAD));
	_userData[531] = (float)(_userData[517] / tan(_userData[528] * DEG2RAD));

	writeScummVar(112, (int32)(_userData[530] * args[10]));
	writeScummVar(113, (int32)(_userData[531] * args[10]));

	return 1;
}

int32 LogicHErace::op_1101(int32 *args) {
	// Update rotation params?
	int32 retval;
	float temp;

	temp = args[0] / _userData[532];
	if (_userData[519] != temp) {
		_userData[519] = temp;
		op_sub3(temp);
		retval = 1;
	} else {
		retval = (int32)temp;
	}

	temp = args[1] / _userData[532];
	if (_userData[520] != temp) {
		_userData[520] = temp;
		op_sub1(temp);
		retval = 1;
	}

	temp = args[2] / _userData[532];
	if (_userData[521] != temp) {
		_userData[521] = temp;
		op_sub2(temp);
		retval = 1;
	}

	return retval;
}

int32 LogicHErace::op_1102(int32 *args) {
	// Update translation params?
	int32 retval;
	float temp;

	temp = args[0] / _userData[532];
	if (_userData[516] != temp) {
		_userData[516] = temp;
		retval = 1;
	} else {
		retval = (int32)_userData[532];
	}

	temp = args[1] / _userData[532];
	if (_userData[517] != temp) {
		_userData[517] = temp;
		retval = 1;
	}

	temp = args[2] / _userData[532];
	if (_userData[518] != temp) {
		_userData[518] = temp;
		retval = 1;
	}

	return retval;
}

int32 LogicHErace::op_1103(int32 *args) {
	double angle = args[0] / args[1] * DEG2RAD;

	writeScummVar(108, (int32)(sin(angle) * args[2]));
	writeScummVar(109, (int32)(cos(angle) * args[2]));

	return 1;
}

int32 LogicHErace::op_1110() {
	writeScummVar(108, (int32)(_userData[526] * _userData[532] * _userData[532]));
	writeScummVar(109, (int32)(_userData[527] * _userData[532] * _userData[532]));
	writeScummVar(110, (int32)(_userData[532]));

	return 1;
}

int32 LogicHErace::op_1120(int32 *args) {
	double a0, a1, a2;
	double b0, b1, b2;
	double res1, res2;

	a0 = args[0] / _userData[532] - _userData[516];
	a1 = args[1] / _userData[532] - _userData[517];
	a2 = args[2] / _userData[532] - _userData[518];

	// Perform matrix multiplication (multiplying by a rotation matrix)
	b2 = a2 * _userDataD[17] + a1 * _userDataD[14] + a0 * _userDataD[11];
	b1 = a2 * _userDataD[16] + a1 * _userDataD[13] + a0 * _userDataD[10];
	b0 = a2 * _userDataD[15] + a1 * _userDataD[12] + a0 * _userDataD[9];

	res1 = (atan2(b0, b2) * RAD2DEG) / _userData[526];
	res2 = (atan2(b1, b2) * RAD2DEG - _userData[528]) / _userData[527];

	writeScummVar(108, (int32)res1);
	writeScummVar(109, (int32)res2);

	return 1;
}

int32 LogicHErace::op_1130(int32 *args) {
	double cs = cos(args[0] / _userData[532] * DEG2RAD);
	double sn = sin(args[0] / _userData[532] * DEG2RAD);

	writeScummVar(108, (int32)(cs * args[1] + sn * args[2]));
	writeScummVar(109, (int32)(cs * args[2] - sn * args[1]));

	return 1;
}

int32 LogicHErace::op_1140(int32 *args) {
	// This functions seems to perform some kind of projection: We project
	// the vector (arg2,arg3) onto the vector (arg0,arg1), but also apply
	// some kind of distortion factor ?!?
	double x = args[2], y = args[3];

	// We start by normalizing the vector described by arg2 and arg3.
	// So compute its length and divide the x and y coordinates
	const double sq = sqrt(x*x + y*y);
	x /= sq;
	y /= sq;

	// Compute the scalar product of the vectors (arg0,arg1) and (x,y)
	const double scalarProduct = x * args[0] + y * args[1];

	// Finally compute the projection of (arg2,arg3) onto (arg0,arg1)
	double projX = args[0] - 2 * scalarProduct * x;
	double projY = args[1] - 2 * scalarProduct * y;

	projX = projX * 20.0 / 23.0;	// FIXME: Why is this here?

	writeScummVar(108, (int32)projX);

	if (args[3] >= 0)	// FIXME: Why is this here?
		projY = projY * 5.0 / 6.0;

	writeScummVar(109, (int32)projY);

	return 1;
}

void LogicHErace::op_sub1(float arg) {
	// Setup a rotation matrix
	_userDataD[10] = _userDataD[12] = _userDataD[14] = _userDataD[16] = 0;
	_userDataD[13] = 1;

	_userDataD[9] = cos(arg * DEG2RAD);
	_userDataD[15] = sin(arg * DEG2RAD);
	_userDataD[11] = -_userDataD[15];
	_userDataD[17] = _userDataD[9];
}

void LogicHErace::op_sub2(float arg) {
	// Setup a rotation matrix -- but it is NEVER USED!
	_userDataD[20] = _userDataD[21] = _userDataD[24] = _userDataD[25] = 0;
	_userDataD[26] = 1;

	_userDataD[19] = sin(arg * DEG2RAD);
	_userDataD[18] = cos(arg * DEG2RAD);
	_userDataD[21] = -_userDataD[19];
	_userDataD[22] = _userDataD[18];
}

void LogicHErace::op_sub3(float arg) {
	// Setup a rotation matrix -- but it is NEVER USED!
	_userDataD[1] = _userDataD[2] = _userDataD[3] = _userDataD[6] = 0;
	_userDataD[0] = 1;

	_userDataD[4] = cos(arg * DEG2RAD);
	_userDataD[5] = sin(arg * DEG2RAD);
	_userDataD[7] = -_userDataD[5];
	_userDataD[8] = _userDataD[4];
}

LogicHE *makeLogicHErace(ScummEngine_v90he *vm) {
	return new LogicHErace(vm);
}

} // End of namespace Scumm
