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
 *     Backyard Soccer
 *     Backyard Soccer MLS Edition
 *     Backyard Soccer 2004
 */
class LogicHEsoccer : public LogicHE {
private:
	double *_userDataD;

public:
	LogicHEsoccer(ScummEngine_v90he *vm);
	~LogicHEsoccer();

	int versionID();
	int32 dispatch(int op, int numArgs, int32 *args);

	void beforeBootScript();
	void initOnce();
	int startOfFrame();

private:
	int op_1005(float x1, float y1, float z1, float x2, float y2, float z2, float *nextVelX, float *nextVelY, float *nextVelZ, float *a10);
	int op_1006(int32 a1, int32 a2, int32 a3, int32 a4);
	int op_1007(int32 *args);
	int op_1008(int outArray, int srcX, int srcY, int srcZ, int vecX, int vecY, int vecZ, int airResX, int airResY, int airResZ, int vecNumerator, int vecDenom, int gravityMult, int requiredSegments, int a15, int a16, int a17, int a18, int fieldType);
	int op_1011(int32 worldPosArray, int32 screenPosArray, int32 a3, int32 closestActorArray, int32 maxDistance, int32 fieldAreaArray);
	int op_1012(int32 *args);
	int op_1013(int32 a1, int32 a2, int32 a3);
	int op_1014(int32 srcX, int32 srcY, int32 srcZ, int32 velX, int32 velY, int32 velZ, int32 outArray, int32 dataArrayId, int32 indexArrayId, int32 requestType, int32 vecNumerator, int32 vecDenom, int32 a13, int32 a14);
	int op_1016(int32 *args);
	int op_1017(int32 *args);
	int op_1019(int32 *args);
	int op_1021(int32 inX, int32 inY, int32 inZ, int32 velX, int32 velY, int32 velZ, int32 internalUse);

	// op_1007 allocates some arrays
	// they're then filled by op_1019
	byte _collisionObjIds[4096], _collisionNodeEnabled[585];

	// op_1011 has a subfunction
	void calculateDistances(int32 worldPosArray, int32 a2, int32 closestActorArray, int32 maxDistance);

	// array containing collision detection tree
	bool _collisionTreeAllocated;
	uint32 *_collisionTree;
	int addCollisionTreeChild(int depth, int index, int parent);

	// op_1014 has several subops
	// ...and several sub-subops
	int generateCollisionObjectList(float srcX, float srcY, float srcZ, float velX, float velY, float velZ);
	int addFromCollisionTreeNode(int index, int parent, uint32 *indices, int objIndexBase);
	void addCollisionObj(byte objId);
	int findCollisionWith(int objId, float inX, float inY, float inZ, float inXVec, float inYVec, float inZVec, float &collideX, float &collideY, float &collideZ, int indexArrayId, int dataArrayId, float *nextVelX, float *nextVelY, float *nextVelZ, float *a15);
	void getPointsForFace(int faceId, float &x1, float &y1, float &z1, float &x2, float &y2, float &z2, float &x3, float &y3, float &z3, float &x4, float &y4, float &z4, const int *objPoints);
	void crossProduct(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, float &outX, float &outY, float &outZ);
	double dotProduct(float a1, float a2, float a3, float a4, float a5, float a6);
	void sortCollisionList(float *data, int numEntries, int entrySize, int compareOn);
	int setCollisionOutputData(float *collisionData, int entrySize, int dataArrayId, int indexArrayId, int startX, int startY, int startZ, float a8, int a9, int a10, int a11, int *out);

	// op_1014 sets an array optionally based upon
	// setCollisionOutputData; it is then used by op_1008
	int _internalCollisionOutData[10];
	Common::List<byte> _collisionObjs;

	// op_1021 can (optionally) set two variables for use in op_1008
	uint32 _var1021[2];
};

int LogicHEsoccer::versionID() {
	return 1;
}

LogicHEsoccer::LogicHEsoccer(ScummEngine_v90he *vm) : LogicHE(vm) {
	_userDataD = (double *)calloc(1732, sizeof(double));
	_collisionTree = 0;
	_collisionTreeAllocated = false;
}

LogicHEsoccer::~LogicHEsoccer() {
	free(_userDataD);
	delete[] _collisionTree;
}

int32 LogicHEsoccer::dispatch(int op, int numArgs, int32 *args) {
	int res = 0;

	switch (op) {
	case 1006:
		res = op_1006(args[0], args[1], args[2], args[3]);
		break;

	case 1007:
		res = op_1007(args);
		break;

	case 1008:
		res = op_1008(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13], args[14], args[15], args[16], args[17], args[18]);
		break;

	case 1011:
		res = op_1011(args[0], args[1], args[2], args[3], args[4], args[5]);
		break;

	case 1012:
		res = op_1012(args);
		break;

	case 1013:
		res = op_1013(args[0], args[1], args[2]);
		break;

	case 1014:
		res = op_1014(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13]);
		break;

	case 1016:
		res = op_1016(args);
		break;

	case 1017:
		res = op_1017(args);
		break;

	case 1019:
		res = op_1019(args);
		break;

	case 1021:
		res = op_1021(args[0], args[1], args[2], args[3], args[4], args[5], args[6]);
		break;

	case 1001: case 1002: case 1003: case 1005:
	case 1009: case 8221968:
		// In the u32, but unused by any of the soccer scripts
		// 1005 is called from another opcode, however
		error("Unused soccer u32 opcode %d called", op);

	case 1004: case 1010: case 1015: case 1018:
	case 1020:
		// Used only by the unaccessible in-game editor (so, fall through)

	default:
		LogicHE::dispatch(op, numArgs, args);
	}

	return res;
}

void LogicHEsoccer::beforeBootScript() {
	_userDataD[530] = 0;
}

void LogicHEsoccer::initOnce() {
	// The original sets some paths here that we don't need to worry about
	_collisionTreeAllocated = false;
	_userDataD[530] = 0;
}

int LogicHEsoccer::startOfFrame() {
	// This variable is some sort of flag that activates this mode
	int res = (int)_userDataD[530];

	if (res)
		res = op_1011((int)_userDataD[531], (int)_userDataD[532], (int)_userDataD[533], (int)_userDataD[534], (int)_userDataD[535], (int)_userDataD[536]);

	return res;
}

int LogicHEsoccer::op_1005(float x1, float y1, float z1, float x2, float y2, float z2, float *nextVelX, float *nextVelY, float *nextVelZ, float *a10) {
	// Called from op_1014

	double dot = x1 * x2 + y1 * y2 + z1 * z2;
	*nextVelX = x2 - 2 * dot * x1;
	*nextVelY = y2 - 2 * dot * y1;
	*nextVelZ = z2 - 2 * dot * z1;
	*a10 = 1.0f; // It always does this. How curious!

	return 1;
}

int LogicHEsoccer::op_1006(int32 a1, int32 a2, int32 a3, int32 a4) {
	double v1 = a1 * 0.01;
	double v2 = a2 * 0.01;
	double v3 = a3 * 0.01;
	double var108, var109;

	_userDataD[529] = a4;

	var108 = atan2(v1, v3) * _userDataD[523] - a4;
	var109 = _userDataD[526] - _userDataD[528] - (_userDataD[521] - atan2(_userDataD[524] - v2, v3)) * _userDataD[522];

	writeScummVar(108, (int32)var108);
	writeScummVar(109, (int32)var109);

	return 1;
}

int LogicHEsoccer::op_1007(int32 *args) {
	// Used when the HE logo is shown
	// This initializes the _userDataD fields that are used in op_1006/op_1011

	float y1 = (double)args[0] / 100.0;
	float x1 = (double)args[1] / 100.0;
	float x2 = (double)args[2] / 100.0;
	float y2 = (double)args[3] / 100.0;
	float x3 = (double)args[4] / 100.0;

	_userDataD[518] = x2;
	_userDataD[519] = 2 * atan2(y2, x2 - x3);
	_userDataD[520] = atan2(y1, x2);
	_userDataD[521] = atan2(y1, x1);
	_userDataD[524] = y1;
	_userDataD[525] = 2 * (_userDataD[521] - _userDataD[520]);
	_userDataD[526] = args[6];
	_userDataD[527] = args[5];
	_userDataD[528] = args[7];
	_userDataD[522] = _userDataD[526] / _userDataD[525];
	_userDataD[523] = _userDataD[527] / _userDataD[519];

	// Clear both byte arrays
	memset(_collisionObjIds, 0, 4096);
	memset(_collisionNodeEnabled, 0, 585);

	if (!_collisionTreeAllocated)
		op_1013(4, args[8], args[9]);

	return 1;
}

static inline double vectorLength(double x, double y, double z) {
	return sqrt(x * x + y * y + z * z);
}

int LogicHEsoccer::op_1008(int outArray, int srcX, int srcY, int srcZ, int vecX, int vecY, int vecZ, int airResX, int airResY, int airResZ, int vecNumerator, int vecDenom, int gravityMult, int requiredSegments, int a15, int a16, int a17, int a18, int fieldType) {
	// Calculate requiredSegments consecutive movement segments, and place
	// the associated data (positions, vectors, etc) into outArray.

	int loopsSoFar = 0;
	int segmentsSoFar = 1;
	int prevVecY = 500;
	int inX = srcX;
	int inZ = srcZ;
	int checkForCollisions = 0;

	while (segmentsSoFar <= requiredSegments) {
		if (fieldType == 1 && srcZ > 8819)
			checkForCollisions = 1;
		else if (fieldType == 2 && (srcX < -2350 || srcX > 2350))
			checkForCollisions = 1;
		else if (fieldType == 3 && (srcX < -2350 || srcX > 2350 || srcZ < 6119 || srcZ > 8819))
			checkForCollisions = 1;

		if (srcY > 0)
			vecY -= vecNumerator * gravityMult / vecDenom;

		int prevX = srcX;
		int prevY = srcY;
		int prevZ = srcZ;
		srcX += vecNumerator * vecX / vecDenom;
		srcY += vecNumerator * vecY / vecDenom;
		srcZ += vecNumerator * vecZ / vecDenom;

		if (srcY > 0) {
			if (checkForCollisions && op_1014(prevX, prevY, prevZ, vecX, vecY, vecZ, 0, a17, a18, 3, vecNumerator, vecDenom, a15, a16)) {
				srcX = _internalCollisionOutData[6];
				srcY = _internalCollisionOutData[7];
				srcZ = _internalCollisionOutData[8];
				vecX = _internalCollisionOutData[3];
				vecY = _internalCollisionOutData[4];
				vecZ = _internalCollisionOutData[5];
				putInArray(outArray, segmentsSoFar, 0, loopsSoFar);
				putInArray(outArray, segmentsSoFar, 1, (int)vectorLength((double)(_internalCollisionOutData[6] - inX), 0.0, (double)(_internalCollisionOutData[8] - inZ)));
				putInArray(outArray, segmentsSoFar, 2, _internalCollisionOutData[6]);
				putInArray(outArray, segmentsSoFar, 3, _internalCollisionOutData[7]);
				putInArray(outArray, segmentsSoFar, 4, _internalCollisionOutData[8]);
				putInArray(outArray, segmentsSoFar, 5, vecX);
				putInArray(outArray, segmentsSoFar, 6, vecY);
				putInArray(outArray, segmentsSoFar++, 7, vecZ);
			}
		} else {
			srcY = 0;
			int thisVecX = vecX;
			int thisVecZ = vecZ;
			vecX = vecX * airResX / 100;

			if (vecY) {
				int v18 = ABS(vecY);
				if (v18 > ABS(prevVecY))
					vecY = ABS(prevVecY);
				vecY = ABS(airResY * vecY) / 100;
			}

			vecZ = airResZ * vecZ / 100;

			if (prevVecY >= 0) {
				if (op_1014(prevX, prevY, prevZ, thisVecX, prevVecY, thisVecZ, 0, a17, a18, 3, vecNumerator, vecDenom, a15, a16)) {
					srcX = _internalCollisionOutData[6];
					srcY = _internalCollisionOutData[7];
					srcZ = _internalCollisionOutData[8];
					vecX = _internalCollisionOutData[3];
					vecY = _internalCollisionOutData[4];
					vecZ = _internalCollisionOutData[5];
				}
			} else {
				if (checkForCollisions) {
					op_1021(srcX, 0, srcZ, thisVecX, prevVecY, thisVecZ, 1);

					if (op_1014(prevX, prevY, prevZ, thisVecX, prevVecY, thisVecZ, 0, a17, a18, 3, vecNumerator, vecDenom, a15, a16)) {
						srcX = _internalCollisionOutData[6];
						srcY = _internalCollisionOutData[7];
						srcZ = _internalCollisionOutData[8];
						vecX = _internalCollisionOutData[3];
						vecY = _internalCollisionOutData[4];
						vecZ = _internalCollisionOutData[5];
					} else {
						// try it with the output of op_1021 instead
						int tmpVecZ = vecZ + prevZ - _var1021[1];
						int v20 = ABS(prevVecY);

						if (op_1014(_var1021[0], 0, _var1021[1], vecX + prevX - _var1021[0], v20 - prevY, tmpVecZ, 0, a17, a18, 3, vecNumerator, vecDenom, a15, a16)) {
							srcX = _internalCollisionOutData[6];
							srcY = _internalCollisionOutData[7];
							srcZ = _internalCollisionOutData[8];
							vecX = _internalCollisionOutData[3];
							vecY = _internalCollisionOutData[4];
							vecZ = _internalCollisionOutData[5];
						}
					}
				}
			}

			prevVecY = vecY;
			putInArray(outArray, segmentsSoFar, 0, loopsSoFar);
			putInArray(outArray, segmentsSoFar, 1, (int32)vectorLength(srcX - inX, 0.0, srcZ - inZ));
			putInArray(outArray, segmentsSoFar, 2, srcX);
			putInArray(outArray, segmentsSoFar, 3, srcY);
			putInArray(outArray, segmentsSoFar, 4, srcZ);
			putInArray(outArray, segmentsSoFar, 5, vecX);
			putInArray(outArray, segmentsSoFar, 6, vecY);
			putInArray(outArray, segmentsSoFar++, 7, vecZ);
		}

		loopsSoFar++;
	}

	return 1;
}

int LogicHEsoccer::op_1011(int32 worldPosArray, int32 screenPosArray, int32 a3, int32 closestActorArray, int32 maxDistance, int32 fieldAreaArray) {
	// This is called on each frame by startOfFrame() if activated by op_1012.

	float objY = 0.0;

	// First, iterate over the field objects and project them onto the screen.
	for (int i = 0; i < 18; i++) {
		int rawX = getFromArray(worldPosArray, i, 0);
		int rawY = getFromArray(worldPosArray, i, 1);
		int rawZ = getFromArray(worldPosArray, i, 2);

		float objX = (double)rawX / 100.0;
		objY = (double)rawY / 100.0;
		float objZ = (double)rawZ / 100.0;

		if (i < 13) {
			// For the players and the ball: work out the area of the field
			// this object is in, storing it in an array if provided.
			int areaX = (rawX + 2750) / 500;
			areaX = CLIP(areaX, 0, 10);

			int areaZ = (9219 - rawZ) / 500;
			areaZ = CLIP(areaZ, 0, 6);

			if (fieldAreaArray)
				putInArray(fieldAreaArray, 0, i, areaX + 11 * areaZ);
		}

		float v7 = atan2(_userDataD[524] - objY, (double)objZ);
		int screenY = (int)(_userDataD[526] - (_userDataD[521] - v7) * _userDataD[522] - 300.0);
		double v9 = _userDataD[523];

		// x/y position of objects
		putInArray(screenPosArray, i, 0, (int32)(atan2(objX, objZ) * v9 + 640.0));
		putInArray(screenPosArray, i, 1, screenY);

		double v10 = atan2(_userDataD[524], (double)objZ);
		int shadowScreenY = (int)(_userDataD[526] - (_userDataD[521] - (float)v10) * _userDataD[522] - 300.0);
		double v13 = _userDataD[523];

		// x/y position of shadows
		putInArray(screenPosArray, i + ((_vm->_game.id == GID_SOCCER) ? 20 : 22), 0, (int32)(atan2(objX, objZ) * v13 + 640.0));
		putInArray(screenPosArray, i + ((_vm->_game.id == GID_SOCCER) ? 20 : 22), 1, shadowScreenY);
	}

	// soccer only uses one array here
	// soccermls/soccer2004 use four
	int start = (_vm->_game.id == GID_SOCCER) ? 19 : 18;
	int end = (_vm->_game.id == GID_SOCCER) ? 19 : 21;

	// The following loop is doing cursor scaling
	// The further up on the screen, the smaller the cursor is
	for (int i = start; i <= end; i++) {
		int x = getFromArray(screenPosArray, i, 0);
		int y = getFromArray(screenPosArray, i, 1);

		// This retains objY from (i == 17)?
		float v16 = _userDataD[524] - objY;
		float scaledZ = v16 / tan((_userDataD[528] + y - _userDataD[526]) / _userDataD[522] + _userDataD[521]);
		double scaledX = tan((double)(x - ((_vm->_game.id == GID_SOCCER) ? 0 : 640)) / _userDataD[523]) * scaledZ;
		putInArray(worldPosArray, i, 0, (int)(scaledX * 100.0));
		putInArray(worldPosArray, i, 2, (int)(scaledZ * 100.0));
	}

	calculateDistances(worldPosArray, a3, closestActorArray, maxDistance);

	return 1;
}

static inline int distance(int a1, int a2, int a3, int a4) {
	return (int)sqrt((double)((a4 - a3) * (a4 - a3) + (a2 - a1) * (a2 - a1)));
}

void LogicHEsoccer::calculateDistances(int32 worldPosArray, int32 a2, int32 closestActorArray, int32 maxDistance) {
	// As you can guess, this is called from op_1011
	// This seems to be checking distances between the players and the ball
	// and which distance is the shortest.

	int closestActor[13];
	int objectX[13];
	int objectZ[13];
	int closestDistance[195];

	for (int i = 0; i < 13; i++) {
		closestActor[i] = 0;
		objectX[i] = getFromArray(worldPosArray, i, 0);
		objectZ[i] = getFromArray(worldPosArray, i, 2);
	}

	// 12 here, 13 up there
	// Probably 12 for players, 13 for players+ball
	for (int i = 0; i < 12; i++) {
		int bestDistance = maxDistance;
		for (int j = i + 1; j < 13; j++) {
			closestDistance[i * 15 + j] = distance(objectX[i], objectX[j], objectZ[i], objectZ[j]);
			putInArray(a2, i, j, closestDistance[i * 15 + j]);
			putInArray(a2, j, i, closestDistance[i * 15 + j]);
			if (closestDistance[i * 15 + j] < bestDistance) {
				bestDistance = closestDistance[i * 15 + j];
				closestActor[i] = j + 1;
				closestActor[j] = i + 1;
			}
		}
	}

	int v13 = getFromArray(worldPosArray, 18, 0);
	int v14 = getFromArray(worldPosArray, 18, 2);
	int v15 = getFromArray(worldPosArray, 19, 0);
	int v16 = getFromArray(worldPosArray, 19, 2);
	int v19[15];
	int v20[15];

	if (_vm->_game.id == GID_SOCCER) {
		// soccer gets to be different
		for (int i = 0; i < 13; i++)
			v20[i] = distance(v15, objectX[i], v16, objectZ[i]);

		for (int i = 0; i < 13; i++)
			v19[i] = distance(v13, objectX[i], v14, objectZ[i]);
	} else {
		// soccermls and soccer2004 use two other arrays here
		int v9 = getFromArray(worldPosArray, 20, 0);
		int v10 = getFromArray(worldPosArray, 20, 2);
		int v11 = getFromArray(worldPosArray, 21, 0);
		int v12 = getFromArray(worldPosArray, 21, 2);

		for (int i = 0; i < 6; i++) {
			v20[i] = distance(v9, objectX[i], v10, objectZ[i]);
			v19[i] = distance(v13, objectX[i], v14, objectZ[i]);
		}

		for (int i = 6; i < 13; i++) {
			v20[i] = distance(v11, objectX[i], v12, objectZ[i]);
			v19[i] = distance(v15, objectX[i], v16, objectZ[i]);
		}
	}

	for (int i = 0; i < 13; i++) {
		putInArray(a2, 14, i, v20[i]);
		putInArray(a2, i, 14, v20[i]);
		putInArray(a2, 13, i, v19[i]);
		putInArray(a2, i, 13, v19[i]);
		putInArray(closestActorArray, 0, i, closestActor[i]);
	}
}

int LogicHEsoccer::op_1012(int32 *args) {
	// Used after op_1019
	// This function activates startOfFrame() to call op_1011
	// (Possibly field parameters?)

	_userDataD[530] = (args[0] != 0) ? 1 : 0;
	_userDataD[531] = args[1];
	_userDataD[532] = args[2];
	_userDataD[533] = args[3];
	_userDataD[534] = args[4];
	_userDataD[535] = args[5];
	_userDataD[536] = args[6];

	return 1;
}

int LogicHEsoccer::addCollisionTreeChild(int depth, int index, int parent) {
	uint32 *dataPtr = _collisionTree + 11 * index;

	/*
	 * This sets up a node of the tree stored in _collisionTree. There are
	 * two sets of parents (at depth 1 and 2), then child nodes at depth
	 * 3 which represent a single collision object.
	 *
	 * 0 = this index, 1 = parent index,
	 * 2-9 = child indices (or all -1 if leaf),
	 * 10 = _collisionObjIds index (if leaf)
	 */
	dataPtr[0] = index;
	dataPtr[1] = parent;

	if (depth > 2) {
		// store the offset into _collisionObjIds (which holds collision object ids),
		// but subtract 585 first because there are already (8 + 8*8 + 8*8*8 = 584)
		// indexes at higher levels of the tree, and we want to start at 0
		dataPtr[10] = 8 * index - 585;
		for (int i = 0; i < 8; i++)
			dataPtr[i + 2] = 0xffffffff;
	} else {
		for (int i = 0; i < 8; i++)
			dataPtr[i + 2] = addCollisionTreeChild(depth + 1, i + 8 * index + 1, index);
	}

	return index;
}

int LogicHEsoccer::op_1013(int32 a1, int32 a2, int32 a3) {
	// Initialises _collisionTree, a tree used for collision detection.
	// It is used by op_1014 to work out which objects to check.

	_collisionTree = new uint32[585 * 11];
	_collisionTreeAllocated = true;
	for (int i = 0; i < 585 * 11; i++)
		_collisionTree[i] = 0;

	for (int i = 0; i < 8; i++)
		_collisionTree[i + 2] = addCollisionTreeChild(1, i + 1, 0);

	return 1;
}

int LogicHEsoccer::op_1014(int32 srcX, int32 srcY, int32 srcZ, int32 velX, int32 velY, int32 velZ, int32 outArray, int32 dataArrayId, int32 indexArrayId, int32 requestType, int32 vecNumerator, int32 vecDenom, int32 a13, int32 a14) {
	// Used many times during a match
	// And called from op_1008!
	// This seems to be doing collision handling

	double startX = (double)srcX;
	double startY = (double)srcY;
	double startZ = (double)srcZ;
	double adjustedVelZ = 0.0, adjustedVelY = 0.0, adjustedVelX = 0.0;

	writeScummVar(108, 0);
	writeScummVar(109, 0);

	switch (requestType) {
	case 1:
	case 3:
		adjustedVelX = (double)velX * (double)vecNumerator / (double)vecDenom / 100.0;
		adjustedVelY = (double)velY * (double)vecNumerator / (double)vecDenom / 100.0;
		adjustedVelZ = (double)velZ * (double)vecNumerator / (double)vecDenom / 100.0;
		break;
	case 2:
		// length of movement vector
		double v15 = vectorLength((double)velX * (double)vecNumerator / (double)vecDenom, (double)velY * (double)vecNumerator / (double)vecDenom, (double)velZ * (double)vecNumerator / (double)vecDenom);

		if (v15 != 0.0) {
			// add the (scaled) movement vector to the input
			double v26 = (double)ABS(velX) * (double)vecNumerator / (double)vecDenom * 50.0 / v15;
			srcX = (int)((double)srcX + v26);
			double v25 = (double)ABS(velY) * (double)vecNumerator / (double)vecDenom * 50.0 / v15;
			srcY = (int)((double)srcY + v25);
			double v24 = (double)ABS(velZ) * (double)vecNumerator / (double)vecDenom * 50.0 / v15;
			srcZ = (int)((double)srcZ + v24);
		}

		// srcX = (newX / newZ) * 3869
		startX = (double)srcX / (double)srcZ * 3869.0;
		// srcY = (newY - (+524 * 100)) / (newZ * 3869 + (+524 * 100)
		startY = ((double)srcY - _userDataD[524] * 100.0) / (double)srcZ * 3869.0 + _userDataD[524] * 100.0;
		// srcZ = 3869
		startZ = 3869.0;
		// vectorX = (newX - srcX) / 100
		adjustedVelX = ((double)srcX - startX) / 100.0;
		// vectorY = (newY - srcY) / 100
		adjustedVelY = ((double)srcY - startY) / 100.0;
		// vectorZ = (newZ - 3869 = srcZ) / 100
		adjustedVelZ = ((double)srcZ - 3869.0) / 100.0;
		break;
	}

	int foundCollision = 0;

	// work out which collision objects we might collide with (if any)
	if (generateCollisionObjectList(startX, startY, startZ, adjustedVelX, adjustedVelY, adjustedVelZ)) {
		int collisionId = 0;
		float v46; // always 1.0 after a collision due to op_1005

		float collisionInfo[42 * 8];
		memset(collisionInfo, 0, 42 * 8 * sizeof(float));

		// check each potential collision object for an actual collision,
		// add it to collisionInfo if there is one
		for (Common::List<byte>::const_iterator it = _collisionObjs.begin(); it != _collisionObjs.end(); it++) {
			float collideZ, collideY, collideX;
			float nextVelX, nextVelY, nextVelZ;

			if (findCollisionWith(*it, startX, startY, startZ, adjustedVelX * 100.0, adjustedVelY * 100.0, adjustedVelZ * 100.0, collideX, collideY, collideZ, indexArrayId, dataArrayId, &nextVelX, &nextVelY, &nextVelZ, &v46)) {
				collisionInfo[collisionId * 8] = *it;
				collisionInfo[collisionId * 8 + 1] = vectorLength(collideX - startX, collideY - startY, collideZ - startZ);
				collisionInfo[collisionId * 8 + 2] = collideX;
				collisionInfo[collisionId * 8 + 3] = collideY;
				collisionInfo[collisionId * 8 + 4] = collideZ;
				collisionInfo[collisionId * 8 + 5] = vecDenom * nextVelX / vecNumerator;
				collisionInfo[collisionId * 8 + 6] = vecDenom * nextVelY / vecNumerator;
				collisionInfo[collisionId * 8 + 7] = vecDenom * nextVelZ / vecNumerator;
				foundCollision = 1;
				collisionId++;
			}
		}

		if (foundCollision) {
			// if we have more than one collision, sort them by distance
			// to find the closest one
			if (collisionId != 1)
				sortCollisionList(collisionInfo, 42, 8, 1);

			int v22, v39, v42;
			float tmpData[8];
			int outData[10];

			// output the collision we found
			switch (requestType) {
				case 1:
					for (int i = 0; i < 8; i++)
						tmpData[i] = collisionInfo[i];
					v22 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0));
					v42 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0  + 1.0));
					v39 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0  + 2.0));
					setCollisionOutputData(tmpData, 8, dataArrayId, indexArrayId, (int)startX, (int)startY, (int)startZ, v46, v22, v42, v39, outData);
					for (int i = 0; i < 10; i++)
						putInArray(outArray, 0, i, outData[i]);
					break;
				case 2:
					// write the object id if collision happened (note that other case can't happen)
					if (collisionId)
						writeScummVar(109, (int)collisionInfo[(collisionId - 1) * 8]);
					else
						writeScummVar(109, 0);
					break;
				case 3:
					for (int i = 0; i < 8; i++)
						tmpData[i] = collisionInfo[i];
					v22 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0));
					v42 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0  + 1.0));
					v39 = getFromArray(indexArrayId, 0, (int)((tmpData[0] - 1.0) * 4.0  + 2.0));
					setCollisionOutputData(tmpData, 8, dataArrayId, indexArrayId, (int)startX, (int)startY, (int)startZ, v46, v22, v42, v39, outData);
					for (int i = 0; i < 10; i++)
						_internalCollisionOutData[i] = outData[i];
					break;
			}
		}
	}

	writeScummVar(108, foundCollision);

	_collisionObjs.clear();

	return foundCollision;
}

int LogicHEsoccer::generateCollisionObjectList(float srcX, float srcY, float srcZ, float velX, float velY, float velZ) {
	float v36 = srcX / 100.0;
	float v37 = v36 + 52.0;
	float destX = v37 + velX;

	int v33, v29;

	if (((int)destX / 52) ^ ((int)v37 / 52)) {
		v33 = 1;
		v29 = 1;
	} else if ((int)v37 / 52) {
		v29 = 0;
		v33 = 1;
	} else {
		v33 = 0;
		v29 = 1;
	}

	uint32 areaEnabled[8];
	for (int i = 0; i < 4; i++) {
		areaEnabled[i] = v29;
		areaEnabled[i + 4] = v33;
	}

	float v38 = srcY / 100.0;
	float destY = v38 + velY;

	if (((int)destY / 20) ^ ((int)v38 / 20)) {
		v33 = 1;
		v29 = 1;
	} else if ((int)v38 / 20) {
		v33 = 1;
		v29 = 0;
	} else {
		v29 = 1;
		v33 = 0;
	}

	for (int i = 0; i < 2; i++) {
		if (areaEnabled[i * 4 + 0])
			areaEnabled[i * 4 + 0] = v29;
		if (areaEnabled[i * 4 + 1])
			areaEnabled[i * 4 + 1] = v29;
		if (areaEnabled[i * 4 + 2])
			areaEnabled[i * 4 + 2] = v33;
		if (areaEnabled[i * 4 + 3])
			areaEnabled[i * 4 + 3] = v33;
	}

	float v39 = srcZ / 100.0;
	float v40 = v39 - 38.69;
	float destZ = v40 + velZ;

	if (((int)destZ / 36) ^ ((int)v40 / 36)) {
		v33 = 1;
		v29 = 1;
	} else if ((int)v40 / 36) {
		v29 = 0;
		v33 = 1;
	} else {
		v33 = 0;
		v29 = 1;
	}

	for (int i = 0; i <= 6; i += 2) {
		if (areaEnabled[i])
			areaEnabled[i] = v29;
		if (areaEnabled[i + 1])
			areaEnabled[i + 1] = v33;
	}

	int objCount = 0;

	for (int i = 0; i < 8; i++) {
		if (areaEnabled[i]) {
			uint32 *ptr = _collisionTree +  _collisionTree[i + 2] * 11;
			objCount += addFromCollisionTreeNode(ptr[0], ptr[1], &ptr[2], ptr[10]);
		}
	}

	writeScummVar(109, objCount);
	return objCount;
}

int LogicHEsoccer::addFromCollisionTreeNode(int index, int parent, uint32 *indices, int objIndexBase) {
	int objCount = 0;

	if (indices[0] == 0xffffffff) {
		for (int i = 0; i < 8; i++) {
			if (_collisionObjIds[i + objIndexBase]) {
				addCollisionObj(_collisionObjIds[i + objIndexBase]);
				objCount = 1;
			}
		}
	} else {
		if (_collisionNodeEnabled[index]) {
			for (int i = 0; i < 8; i++) {
				uint32 *ptr = _collisionTree + indices[i] * 11;
				objCount += addFromCollisionTreeNode(ptr[0], ptr[1], &ptr[2], ptr[10]);
			}
		}
	}

	return objCount;
}

void LogicHEsoccer::addCollisionObj(byte objId) {
	// Add objId to the list if not found
	for (Common::List<byte>::const_iterator it = _collisionObjs.begin(); it != _collisionObjs.end(); it++)
		if (*it == objId)
			return;

	_collisionObjs.push_back(objId);
}

int LogicHEsoccer::findCollisionWith(int objId, float inX, float inY, float inZ, float inXVec, float inYVec, float inZVec, float &collideX, float &collideY, float &collideZ, int indexArrayId, int dataArrayId, float *nextVelX, float *nextVelY, float *nextVelZ, float *a15) {
	int foundCollision = 0;
	float inY_plus1 = inY + 1.0;
	float destX = inX + inXVec;
	float destY = inY_plus1 + inYVec;
	float destZ = inZ + inZVec;

	// don't go below the ground!
	if (inY_plus1 <= 1.0001 && destY < 0.0) {
		destY = 0.0;
		inYVec = ABS((int)inYVec);
	}

	// get the 8 points which define the 6 faces of this object
	int objIndex = getFromArray(indexArrayId, 0, 4 * objId - 1);
	int objPoints[24];
	for (int i = 0; i < 24; i++)
		objPoints[i] = getFromArray(dataArrayId, 0, objIndex + i);

	for (int faceId = 0; faceId < 6; faceId++) {
		// This assigns variables from objPoints based on faceId
		float x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
		float faceCrossX, faceCrossY, faceCrossZ;
		getPointsForFace(faceId, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, objPoints);
		crossProduct(x1, y1, z1, x2, y2, z2, x1, y1, z1, x3, y3, z3, faceCrossX, faceCrossY, faceCrossZ);

		float faceArea = sqrt(faceCrossX * faceCrossX + faceCrossY * faceCrossY + faceCrossZ * faceCrossZ);

		// The original did not initialize these variables and would
		// use them uninitialized if faceArea == 0.0
		float xMult = 0.0, yMult = 0.0, zMult = 0.0;

		if (faceArea != 0.0) {
			// UnitCross = Cross/||Cross||
			xMult = faceCrossX / faceArea;
			yMult = faceCrossY / faceArea;
			zMult = faceCrossZ / faceArea;
		}
		double scalingMult = 5.0;

		float ZToFacePoint1 = z1 - inZ;
		float YToFacePoint1 = y1 - inY_plus1;
		float XToFacePoint1 = x1 - inX;
		// scalar component of UnitCross in direction of (start -> P1)
		double ToFacePoint1 = dotProduct(xMult, yMult, zMult, XToFacePoint1, YToFacePoint1, ZToFacePoint1);

		float ZToDest = destZ - inZ;
		float YToDest = destY - inY_plus1;
		float XToDest = destX - inX;
		// scalar component of UnitCross in direction of (start -> dest)
		double ToDest = dotProduct(xMult, yMult, zMult, XToDest, YToDest, ZToDest);

		if (fabs(ToDest) > 0.00000001)
			scalingMult = ToFacePoint1 / ToDest;

		if (scalingMult >= 0.0 && fabs(scalingMult) <= 1.0 && ToDest != 0.0) {
			// calculate where the collision would be, in the plane containing this face
			double collisionX = inX + (destX - inX) * scalingMult;
			double collisionY = inY_plus1 + (destY - inY_plus1) * scalingMult + 5.0;
			double collisionZ = inZ + (destZ - inZ) * scalingMult;

			// now we need to work out whether this point is actually inside the face
			double dot1 = dotProduct(x2 - x1, y2 - y1, z2 - z1, x3 - x1, y3 - y1, z3 - z1);
			double sqrt1 = vectorLength(x2 - x1, y2 - y1, z2 - z1);
			double num1 = dot1 / (vectorLength(x3 - x1, y3 - y1, z3 - z1) * sqrt1);
			num1 = CLIP<double>(num1, -1.0, 1.0);
			double faceAngle = acos(num1);

			double dot2 = dotProduct(x2 - x1, y2 - y1, z2 - z1, collisionX - x1, collisionY - y1, collisionZ - z1);
			double sqrt2 = vectorLength(x2 - x1, y2 - y1, z2 - z1);
			double num2 = dot2 / (vectorLength(collisionX - x1, collisionY - y1, collisionZ - z1) * sqrt2);
			num2 = CLIP<double>(num2, -1.0, 1.0);
			double angle1 = acos(num2);

			double dot3 = dotProduct(x3 - x1, y3 - y1, z3 - z1, collisionX - x1, collisionY - y1, collisionZ - z1);
			double sqrt3 = vectorLength(x3 - x1, y3 - y1, z3 - z1);
			double num3 = dot3 / (vectorLength(collisionX - x1, collisionY - y1, collisionZ - z1) * sqrt3);
			num3 = CLIP<double>(num3, -1.0, 1.0);
			double angle2 = acos(num3);

			if (angle1 + angle2 - 0.001 <= faceAngle) {
				double dot4 = dotProduct(x2 - x4, y2 - y4, z2 - z4, x3 - x4, y3 - y4, z3 - z4);
				double sqrt4 = vectorLength(x2 - x4, y2 - y4, z2 - z4);
				double num4 = dot4 / (vectorLength(x3 - x4, y3 - y4, z3 - z4) * sqrt4);
				num4 = CLIP<double>(num4, -1.0, 1.0);
				faceAngle = acos(num4);

				double dot5 = dotProduct(x2 - x4, y2 - y4, z2 - z4, collisionX - x4, collisionY - y4, collisionZ - z4);
				double sqrt5 = vectorLength(x2 - x4, y2 - y4, z2 - z4);
				double num5 = dot5 / (vectorLength(collisionX - x4, collisionY - y4, collisionZ - z4) * sqrt5);
				num5 = CLIP<double>(num5, -1.0, 1.0);
				double angle3 = acos(num5);

				double dot6 = dotProduct(x3 - x4, y3 - y4, z3 - z4, collisionX - x4, collisionY - y4, collisionZ - z4);
				double sqrt6 = vectorLength(x3 - x4, y3 - y4, z3 - z4);
				double num6 = dot6 / (vectorLength(collisionX - x4, collisionY - y4, collisionZ - z4) * sqrt6);
				num6 = CLIP<double>(num6, -1.0, 1.0);
				double angle4 = acos(num6);

				if (angle3 + angle4 - 0.001 <= faceAngle) {
					// found a collision with this face
					if (foundCollision) {
						// if we already found one, is the new one closer?
						// (except this don't adjust for the modification of collideX/Y/Z..)
						double ToCollide = vectorLength(inX - collisionX, inY_plus1 - collisionY, inZ - collisionZ);
						if (vectorLength(inX - collideX, inY_plus1 - collideY, inZ - collideZ) > ToCollide) {
							collideX = collisionX - xMult * 3.0;
							collideY = collisionY - yMult * 3.0;
							collideZ = collisionZ - zMult * 3.0;
							op_1005(xMult, yMult, zMult, inXVec, inYVec, inZVec, nextVelX, nextVelY, nextVelZ, a15);
						}
					} else {
						collideX = collisionX - xMult * 3.0;
						collideY = collisionY - yMult * 3.0;
						collideZ = collisionZ - zMult * 3.0;
						op_1005(xMult, yMult, zMult, inXVec, inYVec, inZVec, nextVelX, nextVelY, nextVelZ, a15);
					}

					foundCollision = 1;
				}
			}
		}
	}

	return foundCollision;
}

void LogicHEsoccer::getPointsForFace(int faceId, float &x1, float &y1, float &z1, float &x2, float &y2, float &z2, float &x3, float &y3, float &z3, float &x4, float &y4, float &z4, const int *objPoints) {
	// Note that this originally returned a value, but said value was never used
	// TODO: This can probably be shortened using a few tables...

	switch (faceId) {
	case 0:
		x1 = objPoints[0];
		y1 = objPoints[1];
		z1 = objPoints[2];
		x2 = objPoints[3];
		y2 = objPoints[4];
		z2 = objPoints[5];
		x3 = objPoints[6];
		y3 = objPoints[7];
		z3 = objPoints[8];
		x4 = objPoints[9];
		y4 = objPoints[10];
		z4 = objPoints[11];
		break;
	case 1:
		x1 = objPoints[0];
		y1 = objPoints[1];
		z1 = objPoints[2];
		x2 = objPoints[6];
		y2 = objPoints[7];
		z2 = objPoints[8];
		x3 = objPoints[12];
		y3 = objPoints[13];
		z3 = objPoints[14];
		x4 = objPoints[18];
		y4 = objPoints[19];
		z4 = objPoints[20];
		break;
	case 2:
		x1 = objPoints[3];
		y1 = objPoints[4];
		z1 = objPoints[5];
		x2 = objPoints[15];
		y2 = objPoints[16];
		z2 = objPoints[17];
		x3 = objPoints[9];
		y3 = objPoints[10];
		z3 = objPoints[11];
		x4 = objPoints[21];
		y4 = objPoints[22];
		z4 = objPoints[23];
		break;
	case 3:
		x1 = objPoints[0];
		y1 = objPoints[1];
		z1 = objPoints[2];
		x2 = objPoints[12];
		y2 = objPoints[13];
		z2 = objPoints[14];
		x3 = objPoints[3];
		y3 = objPoints[4];
		z3 = objPoints[5];
		x4 = objPoints[15];
		y4 = objPoints[16];
		z4 = objPoints[17];
		break;
	case 4:
		x1 = objPoints[6];
		y1 = objPoints[7];
		z1 = objPoints[8];
		x2 = objPoints[9];
		y2 = objPoints[10];
		z2 = objPoints[11];
		x3 = objPoints[18];
		y3 = objPoints[19];
		z3 = objPoints[20];
		x4 = objPoints[21];
		y4 = objPoints[22];
		z4 = objPoints[23];
		break;
	case 5:
		x1 = objPoints[15];
		y1 = objPoints[16];
		z1 = objPoints[17];
		x2 = objPoints[12];
		y2 = objPoints[13];
		z2 = objPoints[14];
		x3 = objPoints[21];
		y3 = objPoints[22];
		z3 = objPoints[23];
		x4 = objPoints[18];
		y4 = objPoints[19];
		z4 = objPoints[20];
		break;
	}
}

void LogicHEsoccer::crossProduct(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, float &outX, float &outY, float &outZ) {
	outX = (y2 - y1) * (z4 - z3) - (y4 - y3) * (z2 - z1);
	outY = ((x2 - x1) * (z4 - z3) - (x4 - x3) * (z2 - z1)) * -1.0;
	outZ = (x2 - x1) * (y4 - y3) - (x4 - x3) * (y2 - y1);
}

double LogicHEsoccer::dotProduct(float a1, float a2, float a3, float a4, float a5, float a6) {
	return a1 * a4 + a2 * a5 + a3 * a6;
}

void LogicHEsoccer::sortCollisionList(float *data, int numEntries, int entrySize, int compareOn) {
	// This takes an input array of collisions, and tries to sort it based on the distance
	// (index of compareOn, always 1), copying in groups of entrySize, which is always 8

	bool found = true;
	int entry = 0;

	while (found) {
		found = false;

		// while we still have entries, and there is an obj id set for the next entry
		while (entry <= numEntries - 2 && data[(entry + 1) * 8] != 0.0) {
			// if the current entry has distance 0, or the next entry is closer (distance is less)
			if (data[compareOn + entry * 8] == 0 || data[compareOn + entry * 8] > data[compareOn + (entry + 1) * 8]) {
				found = true;

				// swap all data with the next entry
				for (int i = 0; i < entrySize; i++) {
					float tmp = data[i + entry * 8];
					data[i + entry * 8] = data[i + (entry + 1) * 8];
					data[i + (entry + 1) * 8] = tmp;
				}
			}

			entry++;
		}
	}
}

int LogicHEsoccer::setCollisionOutputData(float *collisionData, int entrySize, int dataArrayId, int indexArrayId, int startX, int startY, int startZ, float a8, int a9, int a10, int a11, int *out) {
	// area-provided data
	out[0] = a9;
	out[1] = a10;
	out[2] = a11;
	// new velocity, slowed by area-provided value
	out[3] = (int)(collisionData[5] * (double)a10 / 100.0);
	out[4] = (int)(collisionData[6] * (double)a10 / 100.0 * a8); // Note: a8 should always be 1
	out[5] = (int)(collisionData[7] * (double)a10 / 100.0);
	// new position
	out[6] = (int)collisionData[2];
	out[7] = (int)collisionData[3];
	out[8] = (int)collisionData[4];
	// collision object id
	out[9] = (int)collisionData[0];
	return out[9];
}

int LogicHEsoccer::op_1016(int32 *args) {
	// Called when a goal is scored

	int result = 0;

	double v9 = (double)args[1] / 100.0;
	double v13 = (double)args[2] / 100.0;
	double v12 = (double)args[3] / 100.0;
	double v18 = v13 * v13;
	double v10 = (double)args[0] / 100.0 * (double)args[0] / 100.0;
	double v11 = v9 * v9;
	double v19 = (v9 * v9 * v12 * v12 + 2.0 * v9 * v12 * v18 + v18 * v18) * v10 * v10 - (v10 + v11) * v12 * v12 * v10 * v10;

	if (v19 >= 0.0) {
		double v6 = sqrt(v19);
		double v17 = ((v9 * v12 + v18) * v10 + v6) / (v10 + v11 + v10 + v11);
		double v16 = ((v9 * v12 + v18) * v10 - v6) / (v10 + v11 + v10 + v11);
		double v7, v14;

		if (v17 <= 0.0 || (v7 = sqrt(v17), v14 = acos(v7 / v13), v14 > 0.7853981633974475)) {
			double v8, v15;
			if (v16 <= 0.0 || (v8 = sqrt(v16), v15 = acos(v8 / v13), v15 > 0.7853981633974475)) {
				writeScummVar(108, -1);
			} else {
				writeScummVar(108, (int)(v15 / 0.01745329251994328 * 100.0));
				result = 1;
			}
		} else {
			writeScummVar(108, (int)(v14 / 0.01745329251994328 * 100.0));
			result = 1;
		}
	} else {
		writeScummVar(108, -1);
	}

	return result;
}

int LogicHEsoccer::op_1017(int32 *args) {
	// Used sporadically during a match (out of bounds?)
	if (!args[1])
		args[1] = 1;

	double v3 = asin((double)args[0] / (double)args[1]);
	writeScummVar(108, (int32)(v3 / 0.01745329251994328 * (double)args[1]));

	return 1;
}

int LogicHEsoccer::op_1019(int32 *args) {
	// Used at the beginning of a match
	// Initializes some arrays with field collision data

	// _collisionObjIds provides object ids for leaf nodes
	// of the collision tree (_collisionTree).
	for (int i = 0; i < 4096; i++)
		_collisionObjIds[i] = getFromArray(args[1], 0, i);

	// _collisionNodeEnabled enables or disables non-leaf nodes
	// of the collision tree (_collisionTree).
	for (int i = 0; i < 585; i++)
		_collisionNodeEnabled[i] = getFromArray(args[0], 0, i);

	// The remaining code of this function was used for the
	// built-in editor. However, it is incomplete in the
	// final product, so we do not need to have it.

	return 1;
}

int LogicHEsoccer::op_1021(int32 inX, int32 inY, int32 inZ, int32 velX, int32 velY, int32 velZ, int32 internalUse) {
	// Used during a match (ball movement?)
	// Also called from op_1008

	int outX;
	if (velX && velY)
		outX = (int)(((double)inY - (double)velY * (double)inX / (double)velX) * -1.0 * (double)velX / (double)velY);
	else
		outX = inX;

	int outZ;
	if (velZ && velY)
		outZ = (int)(((double)inY - (double)velY * (double)inZ / (double)velZ) * -1.0 * (double)velZ / (double)velY);
	else
		outZ = inZ;

	// The final argument chooses whether to store the results for op_1008 or
	// store them in SCUMM variables.
	if (internalUse) {
		_var1021[0] = outX;
		_var1021[1] = outZ;
	} else {
		writeScummVar(108, outX);
		writeScummVar(109, outZ);
	}

	return 1;
}

LogicHE *makeLogicHEsoccer(ScummEngine_v90he *vm) {
	return new LogicHEsoccer(vm);
}

} // End of namespace Scumm
