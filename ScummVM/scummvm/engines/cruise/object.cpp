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

#include "common/textconsole.h"

#include "cruise/cruise_main.h"

namespace Cruise {

//10 values:
/*

0 = X
1 = Y
3 = fileIdx
4 = scale

*/

objDataStruct *getObjectDataFromOverlay(int ovlIdx, int objIdx) {
	objDataStruct *var_6;

	if (ovlIdx < 1 || objIdx < 0)
		return NULL;

	if (!overlayTable[ovlIdx].ovlData)
		return NULL;

	if (overlayTable[ovlIdx].ovlData->numObj <= objIdx)
		return NULL;

	var_6 = overlayTable[ovlIdx].ovlData->arrayObject;

	if (!var_6)
		return NULL;

	return (&var_6[objIdx]);
}

int16 getMultipleObjectParam(int16 overlayIdx, int16 objectIdx, objectParamsQuery *returnParam) {
	objectParams *ptr2 = 0;
	objDataStruct *ptr;
	ovlDataStruct *ovlData;
//  int16 type;

	int state = 0;
	int state2 = 0;

	ptr = getObjectDataFromOverlay(overlayIdx, objectIdx);

	if (!ptr)
		return -11;

	ovlData = overlayTable[overlayIdx].ovlData;

	switch (ptr->_class) {
	case THEME:
	case MULTIPLE: {
		state = globalVars[overlayTable[overlayIdx].state + ptr->_stateTableIdx];

		ptr2 = &ovlData->arrayStates[ptr->_firstStateIdx + state];

		if (ptr->_firstStateIdx + state < 0) {
			debug(0, "Invalid Negative arrayState index in getMultipleObjectParam(overlayIdx: %d, objectIdx: %d)... Forcing to 0", overlayIdx, objectIdx);
			ptr2 = &ovlData->arrayStates[0];
		}

		state2 = ptr2->state;
		break;
	}
	case VARIABLE: {
		ptr2 = &ovlData->arrayObjVar[ptr->_varTableIdx];

		state = ptr2->state;
		state2 = state;
		if (ptr->_firstStateIdx + state < ovlData->size8) {
			objectParams *ptr3 = &ovlData->arrayStates[ptr->_firstStateIdx + state];
			state2 = ptr3->state;
		}
		break;
	}
	default: {
		error("Unsupported case %d in getMultipleObjectParam", ptr->_class);
	}
	}

	returnParam->X = ptr2->X;
	returnParam->Y = ptr2->Y;
	returnParam->baseFileIdx = ptr2->Z;
	returnParam->fileIdx = ptr2->frame;
	returnParam->scale = ptr2->scale;
	returnParam->state = state;
	returnParam->state2 = state2;
	returnParam->nbState = ptr->_numStates;

	return 0;
}

void setObjectPosition(int16 ovlIdx, int16 objIdx, int16 param3, int16 param4) {
	objDataStruct *ptr;
	objectParams *ptr2;

	ptr = getObjectDataFromOverlay(ovlIdx, objIdx);

	if (!ptr) {
		return;
	}
	//overlayTable[param1].ovlData

	switch (ptr->_class) {
	case THEME:
	case MULTIPLE: {
		if (param3 != 5)
			return;
		globalVars[overlayTable[ovlIdx].state + ptr->_stateTableIdx] = param4;
		sortCells(ovlIdx, objIdx, &cellHead);
		break;
	}
	case UNIQUE:
		return;
	case VARIABLE: {
		ptr2 =  &overlayTable[ovlIdx].ovlData->arrayObjVar[ptr->_varTableIdx];

		switch (param3) {
		case 0: {	// x
			ptr2->X = param4;
			break;
		}
		case 1: {	// y
			ptr2->Y = param4;
			break;
		}
		case 2: {	// z
			ptr2->Z = param4;
			sortCells(ovlIdx, objIdx, &cellHead);
			break;
		}
		case 3: {
			ptr2->frame = param4;
			break;
		}
		case 4: {	// scale
			ptr2->scale = param4;
			break;
		}
		case 5: {	// state
			ptr2->state = param4;
			break;
		}
		default: {
			ASSERT(0);
		}
		}

		break;
	}
	default: {
		ASSERT(0);
	}
	}
}

int16 objInit(int ovlIdx, int objIdx, int newState) {
	objDataStruct *ptr;
//  uint16 param;
	ovlDataStruct *ovlData;

	ptr = getObjectDataFromOverlay(ovlIdx, objIdx);

	if (!ptr)
		return -11;

	ovlData = overlayTable[ovlIdx].ovlData;

	switch (ptr->_class) {
	case THEME:
	case MULTIPLE: {
		globalVars[overlayTable[ovlIdx].state + ptr->_stateTableIdx] = newState;
		sortCells(ovlIdx, objIdx, &cellHead);
		break;
	}
	case UNIQUE:
		break;
	case VARIABLE: {
		objectParams *destEntry;
		objectParams *sourceEntry;

		if (ptr->_firstStateIdx + newState > ovlData->size8) {
			return 0;
		}

		destEntry = &ovlData->arrayObjVar[ptr->_varTableIdx];
		sourceEntry = &ovlData->arrayStates[ptr->_firstStateIdx + newState];

		memcpy(destEntry, sourceEntry, sizeof(objectParams));

		destEntry->state = newState;

		sortCells(ovlIdx, objIdx, &cellHead);
		break;
	}
	default: {
		warning("Unsupported param = %d in objInit", ptr->_class);
		// exit(1);
	}
	}

	return 0;
}

int16 getSingleObjectParam(int16 overlayIdx, int16 param2, int16 param3, int16 *returnParam) {
	int state = 0;
	//char* ptr3 = NULL;
	objDataStruct *ptr;
	ovlDataStruct *ovlData;
	objectParams *ptr2 = 0;

	ptr = getObjectDataFromOverlay(overlayIdx, param2);

	if (!ptr)
		return -11;

	ovlData = overlayTable[overlayIdx].ovlData;

	switch (ptr->_class) {
	case THEME:
	case MULTIPLE: {
		state = globalVars[overlayTable[overlayIdx].state + ptr->_stateTableIdx];

		ptr2 = &ovlData->arrayStates[ptr->_firstStateIdx + state];

		if (ptr->_firstStateIdx + state < 0) {
			debug(0, "Invalid Negative arrayState index in getSingleObjectParam(overlayIdx: %d, param2: %d, param3: %d)... Forcing to 0", overlayIdx, param2, param3);
			ptr2 = &ovlData->arrayStates[0];
		}
		break;
	}
	case VARIABLE: {
		ptr2 = &ovlData->arrayObjVar[ptr->_varTableIdx];

		state = ptr2->state;
		break;
	}
	default: {
		error("Unsupported case %d in getSingleObjectParam", ptr->_class);
	}
	}

	switch (param3) {
	case 0: {
		*returnParam = ptr2->X;
		break;
	}
	case 1: {
		*returnParam = ptr2->Y;
		break;
	}
	case 2: {
		*returnParam = ptr2->Z;
		break;
	}
	case 3: {
		*returnParam = ptr2->frame;
		break;
	}
	case 4: {
		*returnParam = ptr2->scale;
		break;
	}
	case 5: {
		*returnParam = state;
		break;
	}
	default: {
		error("Unsupported case %d in getSingleObjectParam case 1", param3);
	}
	}

	return 0;
}

void objectReset() {
	for (int i = 1; i < numOfLoadedOverlay; i++)	{
		if (overlayTable[i].alreadyLoaded && overlayTable[i].ovlData) {
			if (overlayTable[i].ovlData->arrayObject) {
				for (int j = 0; j < overlayTable[i].ovlData->numObj; j++) {
					int stateIdx = overlayTable[i].state + overlayTable[i].ovlData->arrayObject[j]._stateTableIdx;
					if ((overlayTable[i].ovlData->arrayObject[j]._class == THEME) && (globalVars[stateIdx] == -2))
						globalVars[stateIdx] = 0;
				}
			}
		}
	}
}

const char *getObjectName(int index, const char *string) {
	const char *ptr = string;

	if (!string)
		return NULL;

	int i = 0;
//	int j = 0;

	while (i < index) {
		ptr += strlen(ptr) + 1;
		i++;
	}
	return ptr;
}

int getObjectClass(int overlayIdx, int objIdx) {
	objDataStruct *pObjectData = getObjectDataFromOverlay(overlayIdx, objIdx);

	if (pObjectData) {
		return pObjectData->_class;
	} else {
		return -11;
	}
}

} // End of namespace Cruise
