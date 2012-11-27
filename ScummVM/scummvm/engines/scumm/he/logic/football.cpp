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

#include "common/savefile.h"

#include "scumm/he/intern_he.h"
#include "scumm/he/logic_he.h"

namespace Scumm {

/**
 * Logic code for:
 *     Backyard Football
 *     Backyard Football 2002
 */
class LogicHEfootball : public LogicHE {
public:
	LogicHEfootball(ScummEngine_v90he *vm) : LogicHE(vm) {}

	int versionID();
	virtual int32 dispatch(int op, int numArgs, int32 *args);

protected:
	int lineEquation3D(int32 *args);
	virtual int translateWorldToScreen(int32 *args);
	int fieldGoalScreenTranslation(int32 *args);
	virtual int translateScreenToWorld(int32 *args);
	int nextPoint(int32 *args);
	int computePlayerBallIntercepts(int32 *args);
	int computeTwoCircleIntercepts(int32 *args);
};

int LogicHEfootball::versionID() {
	return 1;
}

int32 LogicHEfootball::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
	case 1004:
		res = lineEquation3D(args);
		break;

	case 1006:
		res = translateWorldToScreen(args);
		break;

	case 1007:
		res = fieldGoalScreenTranslation(args);
		break;

	case 1010:
		res = translateScreenToWorld(args);
		break;

	case 1022:
		res = nextPoint(args);
		break;

	case 1023:
		res = computePlayerBallIntercepts(args);
		break;

	case 1024:
		res = computeTwoCircleIntercepts(args);
		break;

	case 8221968:
		// Someone had a fun and used his birthday as opcode number
		res = getFromArray(args[0], args[1], args[2]);
		break;

	case 1492: case 1493: case 1494: case 1495: case 1496:
	case 1497: case 1498: case 1499: case 1500: case 1501:
	case 1502: case 1503: case 1504: case 1505: case 1506:
	case 1507: case 1508: case 1509: case 1510: case 1511:
	case 1512: case 1513: case 1514: case 1555:
		// DirectPlay-related
		// 1513: initialize
		// 1555: set fake lag
		break;

	case 2200: case 2201: case 2202: case 2203: case 2204:
	case 2205: case 2206: case 2207: case 2208: case 2209:
	case 2210: case 2211: case 2212: case 2213: case 2214:
	case 2215: case 2216: case 2217: case 2218: case 2219:
	case 2220: case 2221: case 2222: case 2223: case 2224:
	case 2225: case 2226: case 2227: case 2228:
		// Boneyards-related
		break;

	case 3000: case 3001: case 3002: case 3003: case 3004:
		// Internet-related
		// 3000: check for updates
		// 3001: check network status
		// 3002: autoupdate
		// 3003: close connection
		break;

	default:
		LogicHE::dispatch(op, numArgs, args);
		warning("Tell sev how to reproduce it (%d)", op);
	}

	return res;
}

int LogicHEfootball::lineEquation3D(int32 *args) {
	// Identical to soccer's 1004 opcode
	double res, a2, a4, a5;

	a5 = ((double)args[4] - (double)args[1]) / ((double)args[5] - (double)args[2]);
	a4 = ((double)args[3] - (double)args[0]) / ((double)args[5] - (double)args[2]);
	a2 = (double)args[2] - (double)args[0] * a4 - args[1] * a5;

	res = (double)args[6] * a4 + (double)args[7] * a5 + a2;
	writeScummVar(108, (int32)res);

	writeScummVar(109, (int32)a2);
	writeScummVar(110, (int32)a5);
	writeScummVar(111, (int32)a4);

	return 1;
}

int LogicHEfootball::translateWorldToScreen(int32 *args) {
	// This is more or less the inverse of translateScreenToWorld
	const double a1 = args[1];
	double res;

	// 2.9411764e-4 = 1/3400
	// 5.3050399e-2 = 1/18.85 = 20/377
	// 1.1764706e-2 = 1/85 = 40/3400
	// 1.2360656e-1 = 377/3050
	res = (1.0 - a1 * 2.9411764e-4 * 5.3050399e-2) * 1.2360656e-1 * args[0] +
		a1 * 1.1764706e-2 + 46;

	// Shortened / optimized version of that formula:
	// res = (377.0 - a1 / 170.0) / 3050.0 * args[0] + a1 / 85.0 + 46;

	writeScummVar(108, (int32)res);

	// 1.2360656e-1 = 377/3050
	// 1.1588235e-1 = 197/1700 = 394/3400
	res = 640.0 - args[2] * 1.2360656e-1 - a1 * 1.1588235e-1 - 26;

	writeScummVar(109, (int32)res);

	return 1;
}

int LogicHEfootball::fieldGoalScreenTranslation(int32 *args) {
	double res, temp;

	temp = (double)args[1] * 0.32;

	if (temp > 304.0)
		res = -args[2] * 0.142;
	else
		res = args[2] * 0.142;

	res += temp;

	writeScummVar(108, (int32)res);

	res = (1000.0 - args[2]) * 0.48;

	writeScummVar(109, (int32)res);

	return 1;
}

int LogicHEfootball::translateScreenToWorld(int32 *args) {
	// This is more or less the inverse of translateWorldToScreen
	double a1 = (640.0 - (double)args[1] - 26.0) / 1.1588235e-1;

	// 2.9411764e-4 = 1/3400
	// 5.3050399e-2 = 1/18.85 = 20/377
	// 1.1764706e-2 = 1/85 = 40/3400
	// 1.2360656e-1 = 377/3050
	double a0 = ((double)args[0] - 46 - a1 * 1.1764706e-2) /
		((1.0 - a1 * 2.9411764e-4 * 5.3050399e-2) * 1.2360656e-1);

	writeScummVar(108, (int32)a0);
	writeScummVar(109, (int32)a1);

	return 1;
}

int LogicHEfootball::nextPoint(int32 *args) {
	double res;
	double var10 = args[4] - args[1];
	double var8 = args[5] - args[2];
	double var6 = args[3] - args[0];

	res = sqrt(var8 * var8 + var6 * var6 + var10 * var10);

	if (res >= (double)args[6]) {
		var8 = (double)args[6] * var8 / res;
		var10 = (double)args[6] * var10 / res;
		res = (double)args[6] * var6 / res;
	}

	writeScummVar(108, (int32)res);
	writeScummVar(109, (int32)var10);
	writeScummVar(110, (int32)var8);

	return 1;
}

int LogicHEfootball::computePlayerBallIntercepts(int32 *args) {
	double var10, var18, var20, var28, var30, var30_;
	double argf[7];

	for (int i = 0; i < 7; i++)
		argf[i] = args[i];

	var10 = (argf[3] - argf[1]) / (argf[2] - argf[0]);
	var28 = var10 * var10 + 1;
	var20 = argf[0] * var10;
	var18 = (argf[5] + argf[1] + var20) * argf[4] * var10 * 2 +
		argf[6] * argf[6] * var28 + argf[4] * argf[4] -
		argf[0] * argf[0] * var10 * var10 -
		argf[5] * argf[0] * var10 * 2 -
		argf[5] * argf[1] * 2 -
		argf[1] * argf[1] - argf[5] * argf[5];

	if (var18 >= 0) {
		var18 = sqrt(var18);

		var30_ = argf[4] + argf[5] * var10 + argf[1] * var10 + argf[0] * var10 * var10;
		var30 = (var30_ - var18) / var28;
		var18 = (var30_ + var18) / var28;

		if ((argf[0] - var30 < 0) && (argf[0] - var18 < 0)) {
			var30_ = var30;
			var30 = var18;
			var18 = var30_;
		}
		var28 = var18 * var10 - var20 - argf[1];
		var20 = var30 * var10 - var20 - argf[1];
	} else {
		var18 = 0;
		var20 = 0;
		var28 = 0;
		var30 = 0;
	}

	writeScummVar(108, (int32)var18);
	writeScummVar(109, (int32)var28);
	writeScummVar(110, (int32)var30);
	writeScummVar(111, (int32)var20);

	return 1;
}

int LogicHEfootball::computeTwoCircleIntercepts(int32 *args) {
	// Looks like this was just dummied out
	writeScummVar(108, 0);
	writeScummVar(109, 0);
	writeScummVar(110, 0);
	writeScummVar(111, 0);

	return 1;
}

class LogicHEfootball2002 : public LogicHEfootball {
public:
	LogicHEfootball2002(ScummEngine_v90he *vm) : LogicHEfootball(vm) {}

	int32 dispatch(int op, int numArgs, int32 *args);

private:
	int translateWorldToScreen(int32 *args);
	int translateScreenToWorld(int32 *args);
	int getDayOfWeek();
	int initScreenTranslations();
	int getPlaybookFiles(int32 *args);
	int largestFreeBlock();
};

int32 LogicHEfootball2002::dispatch(int op, int numArgs, int32 *args) {
	int32 res = 0;

	switch (op) {
	case 1025:
		res = getDayOfWeek();
		break;

	case 1026:
		res = initScreenTranslations();
		break;

	case 1027:
		res = getPlaybookFiles(args);
		break;

	case 1028:
		res = largestFreeBlock();
		break;

	case 1029:
		// Clean-up off heap
		// Dummied in the Windows U32
		res = 1;
		break;

	case 1516:
		// Start auto LAN game
		break;

	default:
		res = LogicHEfootball::dispatch(op, numArgs, args);
		break;
	}

	return res;
}

int LogicHEfootball2002::translateWorldToScreen(int32 *args) {
	// TODO: Implement modified 2002 version
	return LogicHEfootball::translateWorldToScreen(args);
}

int LogicHEfootball2002::translateScreenToWorld(int32 *args) {
	// TODO: Implement modified 2002 version
	return LogicHEfootball::translateScreenToWorld(args);
}

int LogicHEfootball2002::getDayOfWeek() {
	// Get day of week, store in var 108

	TimeDate time;
	_vm->_system->getTimeAndDate(time);
	writeScummVar(108, time.tm_wday);

	return 1;
}

int LogicHEfootball2002::initScreenTranslations() {
	// TODO: Set values used by translateWorldToScreen/translateScreenToWorld
	return 1;
}

int LogicHEfootball2002::getPlaybookFiles(int32 *args) {
	// Get the pattern and then skip over the directory prefix ("*\" or "*:")
	Common::String pattern = (const char *)_vm->getStringAddress(args[0] & ~0x33539000) + 2;

	// Prepare a buffer to hold the file names
	char buffer[1000];
	buffer[0] = 0;

	// Get the list of file names that match the pattern and iterate over it
	Common::StringArray fileList = _vm->getSaveFileManager()->listSavefiles(pattern);

	for (uint32 i = 0; i < fileList.size() && strlen(buffer) < 970; i++) {
		// Isolate the base part of the filename and concatenate it to our buffer
		Common::String fileName = Common::String(fileList[i].c_str(), fileList[i].size() - (pattern.size() - 1));
		strcat(buffer, fileName.c_str());
		strcat(buffer, ">"); // names separated by '>'
	}

	// Now store the result in an array
	int array = _vm->setupStringArray(strlen(buffer));
	strcpy((char *)_vm->getStringAddress(array), buffer);

	// And store the array index in variable 108
	writeScummVar(108, array);

	return 1;
}

int LogicHEfootball2002::largestFreeBlock() {
	// The Windows version always sets the variable to this
	// The Mac version actually checks for the largest free block
	writeScummVar(108, 100000000);
	return 1;
}

LogicHE *makeLogicHEfootball(ScummEngine_v90he *vm) {
	return new LogicHEfootball(vm);
}

LogicHE *makeLogicHEfootball2002(ScummEngine_v90he *vm) {
	return new LogicHEfootball2002(vm);
}

} // End of namespace Scumm
