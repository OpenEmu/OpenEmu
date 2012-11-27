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

#include "cruise/cruise.h"
#include "cruise/cruise_main.h"
#include "common/endian.h"
#include "common/textconsole.h"

namespace Cruise {

scriptInstanceStruct relHead;
scriptInstanceStruct procHead;

scriptInstanceStruct *currentScriptPtr;

int8 getByteFromScript() {
	int8 var = *(int8 *)(currentData3DataPtr + currentScriptPtr->scriptOffset);
	++currentScriptPtr->scriptOffset;

	return (var);
}

short int getShortFromScript() {
	short int var = (int16)READ_BE_UINT16(currentData3DataPtr + currentScriptPtr->scriptOffset);
	currentScriptPtr->scriptOffset += 2;

	return (var);
}

// load opcode
int32 opcodeType0() {
	int index = 0;

	switch (currentScriptOpcodeType) {
	case 0: {
		pushVar(getShortFromScript());
		return (0);
	}
	case 5:
		index = saveOpcodeVar;
	case 1: {
		uint8 *address = 0;
		int type = getByteFromScript();
		int ovl = getByteFromScript();
		short int offset = getShortFromScript();
		offset += index;

		int typ7 = type & 7;

		if (!typ7) {
			return (-10); // unresloved link
		}

		if (!ovl) {
			address = scriptDataPtrTable[typ7];
		} else	{ // TODO:
			if (!overlayTable[ovl].alreadyLoaded) {
				return (-7);
			}

			if (!overlayTable[ovl].ovlData) {
				return (-4);
			}

			if (typ7 == 5) {
				address = overlayTable[ovl].ovlData->data4Ptr;
			} else {
				assert(0);
			}
		}

		address += offset;

		int size = (type >> 3) & 3;

		if (size == 1) {
			address += index;
			pushVar((int16)READ_BE_UINT16(address));
			return 0;
		} else if (size == 2) {
			pushVar(*address);
			return 0;
		} else {
			error("Unsupported code in opcodeType0 case 1");
		}
	}
	case 2: {
		int16 var_16;
		int di = getByteFromScript();
		int si = getByteFromScript();
		int var_2 = getShortFromScript();

		if (!si) {
			si = currentScriptPtr->overlayNumber;
		}

		if (getSingleObjectParam(si, var_2, di, &var_16)) {
			return -10;
		}

		pushVar(var_16);
		return 0;
	}
	default:
		error("Unsupported type %d in opcodeType0", currentScriptOpcodeType);
	}
}

// save opcode
int32 opcodeType1()	{
	int var = popVar();
	int offset = 0;

	switch (currentScriptOpcodeType) {
	case 0: {
		return (0);	// strange, but happens also in original interpreter
	}
	case 5: {
		offset = saveOpcodeVar;
	}
	case 1: {
		int var_A = 0;

		int byte1 = getByteFromScript();
		int byte2 = getByteFromScript();

		int short1 = getShortFromScript();

		int var_6 = byte1 & 7;

		int var_C = short1;

		uint8 *ptr = 0;
		int type2;

		if (!var_6)
			return (-10);

		var_C = short1;

		if (byte2) {
			if (!overlayTable[byte2].alreadyLoaded) {
				return (-7);
			}

			if (!overlayTable[byte2].ovlData) {
				return (-4);
			}

			if (var_6 == 5) {
				ptr = overlayTable[byte2].ovlData->data4Ptr + var_C;
			} else {
				ASSERT(0);
			}
		} else {
			ptr = scriptDataPtrTable[var_6] + var_C;
		}

		type2 = ((byte1 & 0x18) >> 3);

		switch (type2) {
		case 1: {
			WRITE_BE_UINT16(ptr + var_A + offset * 2, var);
			return 0;
		}
		case 2: {
			assert (ptr);
			*(ptr + var_A + offset) = var;
			return 0;
		}
		default:
			error("Unsupported code in opcodeType1 case 1");
		}

		break;
	}
	case 2: {
		int mode = getByteFromScript();
		int di = getByteFromScript();
		int var_4 = getShortFromScript();

		if (!di) {
			di = currentScriptPtr->overlayNumber;
		}

		if ((var == 0x85) && !strcmp((char *)currentCtpName, "S26.CTP") && !di && mode == 1) { // patch in bar
			var = 0x87;
		}

		setObjectPosition(di, var_4, mode, var);

		break;
	}
	case 4: {
		saveOpcodeVar = var;
		break;
	}
	default:
		error("Unsupported type %d in opcodeType1", currentScriptOpcodeType);
	}

	return (0);
}

int32 opcodeType2() {
	int index = 0;
	switch (currentScriptOpcodeType) {
	case 5:
		index = saveOpcodeVar;
	case 1: {
		uint8* adresse = NULL;
		int type = getByteFromScript();
		int overlay = getByteFromScript();

		int offset = getShortFromScript();
		offset += index;

		int typ7 = type & 7;
		if (!typ7) {
			return (-10);
		}
		if (!overlay) {
			adresse = scriptDataPtrTable[typ7];
		} else {
			if (!overlayTable[overlay].alreadyLoaded) {
				return (-7);
			}
			if (!overlayTable[overlay].ovlData) {
				return (-4);
			}
			ASSERT(0);
		}

		adresse += offset;
		int size = (type >> 3) & 3;

		if (size == 1) {
			adresse += index;
			pushPtr(adresse);
		} else if (size == 2) {
			pushPtr(adresse);
		}

	}
	}

	return 0;
}

int32 opcodeType10() {	// break
	return (0);
}

int32 opcodeType11() {	// break
	return (1);
}

int32 opcodeType4() {		// test
	int boolVar = 0;

	int var1 = popVar();
	int var2 = popVar();

	switch (currentScriptOpcodeType) {
	case 0: {
		if (var2 != var1)
			boolVar = 1;
		break;
	}
	case 1: {
		if (var2 == var1)
			boolVar = 1;
		break;
	}
	case 2: {
		if (var2 < var1)
			boolVar = 1;
		break;
	}
	case 3: {
		if (var2 <= var1)
			boolVar = 1;
		break;
	}
	case 4: {
		if (var2 > var1)
			boolVar = 1;
		break;
	}
	case 5: {
		if (var2 >= var1)
			boolVar = 1;
		break;
	}

	}

	pushVar(boolVar);

	return (0);
}

int32 opcodeType6() {
	int si = 0;

	int pop = popVar();

	if (!pop)
		si = 1;

	if (pop < 0) {
		si |= 4;
	}

	if (pop > 0) {
		si |= 2;
	}

	currentScriptPtr->ccr = si;

	return (0);
}

int32 opcodeType7() {
	int var1 = popVar();
	int var2 = popVar();

	pushVar(var1);
	pushVar(var2);

	return (0);
}

int32 opcodeType5() {
	int offset = currentScriptPtr->scriptOffset;
	int short1 = getShortFromScript();
	int newSi = short1 + offset;
	int bitMask = currentScriptPtr->ccr;

	switch (currentScriptOpcodeType) {
	case 0: {
		if (!(bitMask & 1)) {
			currentScriptPtr->scriptOffset = newSi;
		}
		break;
	}
	case 1: {
		if (bitMask & 1) {
			currentScriptPtr->scriptOffset = newSi;
		}
		break;
	}
	case 2: {
		if (bitMask & 2) {
			currentScriptPtr->scriptOffset = newSi;
		}
		break;
	}
	case 3: {
		if (bitMask & 3) {
			currentScriptPtr->scriptOffset = newSi;
		}
		break;
	}
	case 4: {
		if (bitMask & 4) {
			currentScriptPtr->scriptOffset = newSi;
		}
		break;
	}
	case 5: {
		if (bitMask & 5) {
			currentScriptPtr->scriptOffset = newSi;
		}
		break;
	}
	case 6: {
		break;	// never
	}
	case 7: {
		currentScriptPtr->scriptOffset = newSi;	//always
		break;
	}
	}

	return (0);
}

int32 opcodeType3()	{	// math
	int pop1 = popVar();
	int pop2 = popVar();

	switch (currentScriptOpcodeType) {
	case 0: {
		pushVar(pop1 + pop2);
		return (0);
	}
	case 1: {
		pushVar(pop1 / pop2);
		return (0);
	}
	case 2: {
		pushVar(pop1 - pop2);
		return (0);
	}
	case 3: {
		pushVar(pop1 * pop2);
		return (0);
	}
	case 4: {
		pushVar(pop1 % pop2);
		return (0);
	}
	case 7:
	case 5: {
		pushVar(pop2 | pop1);
		return (0);
	}
	case 6: {
		pushVar(pop2 & pop1);
		return (0);
	}
	}

	return 0;
}

int32 opcodeType9() {		// stop script
	//debug("Stop a script of overlay %s", overlayTable[currentScriptPtr->overlayNumber].overlayName);
	currentScriptPtr->scriptNumber = -1;
	return (1);
}

void setupFuncArray() {
	int i;

	for (i = 0; i < 64; i++) {
		opcodeTypeTable[i] = NULL;
	}

	opcodeTypeTable[1] = opcodeType0;
	opcodeTypeTable[2] = opcodeType1;
	opcodeTypeTable[3] = opcodeType2;
	opcodeTypeTable[4] = opcodeType3;
	opcodeTypeTable[5] = opcodeType4;
	opcodeTypeTable[6] = opcodeType5;
	opcodeTypeTable[7] = opcodeType6;
	opcodeTypeTable[8] = opcodeType7;
	opcodeTypeTable[9] = opcodeType8;
	opcodeTypeTable[10] = opcodeType9;
	opcodeTypeTable[11] = opcodeType10;
	opcodeTypeTable[12] = opcodeType11;
}

int removeScript(int overlay, int idx, scriptInstanceStruct *headPtr) {
	scriptInstanceStruct *scriptPtr;

	scriptPtr = headPtr->nextScriptPtr;

	if (scriptPtr) {
		do {
			if (scriptPtr->overlayNumber == overlay
			        && (scriptPtr->scriptNumber == idx || idx == -1)) {
				scriptPtr->scriptNumber = -1;
			}

			scriptPtr = scriptPtr->nextScriptPtr;
		} while (scriptPtr);
	}

	return (0);
}

uint8 *attacheNewScriptToTail(scriptInstanceStruct *scriptHandlePtr, int16 overlayNumber, int16 param, int16 arg0, int16 arg1, int16 arg2, scriptTypeEnum scriptType) {
	int useArg3Neg = 0;
	ovlData3Struct *data3Ptr;
	scriptInstanceStruct *tempPtr;
	int var_C;
	scriptInstanceStruct *oldTail;

	//debug("Starting script %d of overlay %s", param,overlayTable[overlayNumber].overlayName);

	if (scriptType < 0) {
		useArg3Neg = 1;
		scriptType = (scriptTypeEnum) - scriptType;
	}

	if (scriptType == 20) {
		data3Ptr = getOvlData3Entry(overlayNumber, param);
	} else {
		if (scriptType == 30) {
			data3Ptr = scriptFunc1Sub2(overlayNumber, param);
		} else {
			return (NULL);
		}
	}

	if (!data3Ptr) {
		return (NULL);
	}

	if (!data3Ptr->dataPtr) {
		return (NULL);
	}

	var_C = data3Ptr->sysKey;

	oldTail = scriptHandlePtr;

	while (oldTail->nextScriptPtr) {	// go to the end of the list
		oldTail = oldTail->nextScriptPtr;
	}

	tempPtr =
	    (scriptInstanceStruct *)
	    mallocAndZero(sizeof(scriptInstanceStruct));

	if (!tempPtr)
		return (NULL);

	tempPtr->data = NULL;

	if (var_C) {
		tempPtr->data = (uint8 *) mallocAndZero(var_C);
	}

	tempPtr->dataSize = var_C;
	tempPtr->nextScriptPtr = NULL;
	tempPtr->scriptOffset = 0;

	tempPtr->scriptNumber = param;
	tempPtr->overlayNumber = overlayNumber;

	if (scriptType == 20) {	// Obj or not ?
		tempPtr->sysKey = useArg3Neg;
	} else {
		tempPtr->sysKey = 1;
	}

	tempPtr->freeze = 0;
	tempPtr->type = scriptType;
	tempPtr->var18 = arg2;
	tempPtr->var16 = arg1;
	tempPtr->var1A = arg0;
	tempPtr->nextScriptPtr = oldTail->nextScriptPtr;	// should always be NULL as it's the tail

	oldTail->nextScriptPtr = tempPtr;	// attache the new node to the list

	return (tempPtr->data);
}

int executeScripts(scriptInstanceStruct *ptr) {
	int numScript2;
	ovlData3Struct *ptr2;
	ovlDataStruct *ovlData;
	uint8 opcodeType;

	numScript2 = ptr->scriptNumber;

	if (ptr->type == 20) {
		ptr2 = getOvlData3Entry(ptr->overlayNumber, numScript2);

		if (!ptr2) {
			return (-4);
		}
	} else {
		if (ptr->type == 30) {
			ptr2 = scriptFunc1Sub2(ptr->overlayNumber, numScript2);

			if (!ptr2) {
				return (-4);
			}
		} else {
			return (-6);
		}
	}

	if (!overlayTable[ptr->overlayNumber].alreadyLoaded) {
		return (-7);
	}

	ovlData = overlayTable[ptr->overlayNumber].ovlData;

	if (!ovlData)
		return (-4);

	currentData3DataPtr = ptr2->dataPtr;

	scriptDataPtrTable[1] = (uint8 *) ptr->data;
	scriptDataPtrTable[2] = getDataFromData3(ptr2, 1);
	scriptDataPtrTable[5] = ovlData->data4Ptr;	// free strings
	scriptDataPtrTable[6] = ovlData->ptr8;

	currentScriptPtr = ptr;

	positionInStack = 0;

	do {
#ifdef SKIP_INTRO
		if (currentScriptPtr->scriptOffset == 290
		        && currentScriptPtr->overlayNumber == 4
		        && currentScriptPtr->scriptNumber == 0) {
			currentScriptPtr->scriptOffset = 923;
		}
#endif
		opcodeType = getByteFromScript();

		debugC(5, kCruiseDebugScript, "Script %s/%d ip=%d opcode=%d",
			overlayTable[currentScriptPtr->overlayNumber].overlayName,
			currentScriptPtr->scriptNumber,
			currentScriptPtr->scriptOffset,
			(opcodeType & 0xFB) >> 3);

		currentScriptOpcodeType = opcodeType & 7;

		if (!opcodeTypeTable[(opcodeType & 0xFB) >> 3]) {
			error("Unsupported opcode type %d", (opcodeType & 0xFB) >> 3);
		}
	} while (!opcodeTypeTable[(opcodeType & 0xFB) >> 3]());

	currentScriptPtr = NULL;

	return (0);
}

void manageScripts(scriptInstanceStruct *scriptHandle) {
	scriptInstanceStruct *ptr = scriptHandle;

	if (ptr) {
		do {
			if (!overlayTable[ptr->overlayNumber].executeScripts) {
				if ((ptr->scriptNumber != -1) && (ptr->freeze == 0) && (ptr->sysKey != 0)) {
					executeScripts(ptr);
				}

				if (ptr->sysKey == 0) {
					ptr->sysKey = 1;
				}
			}

			ptr = ptr->nextScriptPtr;

		} while (ptr);
	}
}

} // End of namespace Cruise
