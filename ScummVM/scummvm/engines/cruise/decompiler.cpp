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

#include "cruise/cruise_main.h"

namespace Cruise {

#ifdef DUMP_SCRIPT

#define numMaxLignes 100000
#define lineMaxSize 10000

int currentLineType = 0;

struct decompileLineStruct {
	int lineOffset;
	char line[lineMaxSize];
	int indent;
	int type;
	int pendingElse;
};

struct decompileLineStruct decompileLineTable[numMaxLignes];

int positionInDecompileLineTable;

int failed;

char *currentDecompScript;
scriptInstanceStruct dummy;
scriptInstanceStruct *currentDecompScriptPtr = &dummy;

uint8 *getDataFromData3(ovlData3Struct * ptr, int param);

opcodeTypeFunction decompOpcodeTypeTable[64];

int currentLineIdx = 0;

unsigned long int currentOffset;

unsigned long int dumpIdx = 0;

FILE *fHandle = NULL;	// FIXME: Use Common::DumpFile instead of FILE

#define DECOMPILER_STACK_DEPTH 100
#define DECOMPILER_STACK_ENTRY_SIZE 5000

char tempbuffer[5000];

char decompileStack[DECOMPILER_STACK_DEPTH][DECOMPILER_STACK_ENTRY_SIZE];

unsigned long int decompileStackPosition = 0;

uint8 stringName[256];

ovlData3Struct *currentScript;

ovlDataStruct *currentDecompOvl;
int currentDecompScriptIdx;

char decompSaveOpcodeVar[256];

uint8 *getStringNameFromIdx(uint16 stringTypeIdx, char *offset) {
	switch (stringTypeIdx & 7) {
	case 2: {
		sprintf(stringName, "\"%s\"",
		        currentScript->dataPtr +
		        currentScript->offsetToSubData3 + atoi(offset));
		break;
	}
	case 5: {
		sprintf(stringName, "vars[%s]", offset);
		break;
	}
	default: {
		sprintf(stringName, "string[%d][%s]",
		        stringTypeIdx & 7, offset);
		break;
	}
	}

	return stringName;
}

char *resolveMessage(char *messageIdxString) {
	char buffer[500];
	int variable;

	variable = atoi(messageIdxString);
	sprintf(buffer, "%d", variable);

	if (strcmp(buffer, messageIdxString)) {
		return messageIdxString;
	} else {
		return currentDecompOvl->stringTable[atoi(messageIdxString)].
		       string;
	}
}

void pushDecomp(char *string, ...) {
	va_list va;

	va_start(va, string);
	vsprintf(decompileStack[decompileStackPosition], string, va);
	va_end(va);

	// fprintf(fHandle, "----> %s\n",decompileStack[decompileStackPosition]);

	decompileStackPosition++;
}

void resolveDecompShort(char *buffer) {
	ovlData3Struct *data3Ptr = currentScript;
	int i;

	importScriptStruct *importEntry =
	    (importScriptStruct *)(data3Ptr->dataPtr +
	                           data3Ptr->offsetToImportData);

	for (i = 0; i < data3Ptr->numRelocGlob; i++) {
		switch (importEntry->type) {
		case 20:	// script
		case 30:
		case 40:
		case 50: {
			if (importEntry->offset == currentDecompScriptPtr->var4 - 3) {	// param1
				sprintf(buffer,
				        data3Ptr->dataPtr +
				        data3Ptr->
				        offsetToImportName +
				        importEntry->offsetToName);
				return;
			}
			if (importEntry->offset == currentDecompScriptPtr->var4 - 6) {	// param2
				sprintf(buffer, "linkedIdx");
				return;
			}
			break;
		}
		default: {
			if (importEntry->offset ==
			        currentDecompScriptPtr->var4 - 4) {
				sprintf(buffer,
				        data3Ptr->dataPtr +
				        data3Ptr->
				        offsetToImportName +
				        importEntry->offsetToName);
				return;
			}
		}
		}
		importEntry++;
	}

	buffer[0] = 0;

}

void resolveDecompChar(char *buffer) {
	ovlData3Struct *data3Ptr = currentScript;
	int i;

	importScriptStruct *importEntry =
	    (importScriptStruct *)(data3Ptr->dataPtr +
	                           data3Ptr->offsetToImportData);

	for (i = 0; i < data3Ptr->numRelocGlob; i++) {
		switch (importEntry->type) {
		default: {
			if (importEntry->offset ==
			        currentDecompScriptPtr->var4 - 2) {
				sprintf(buffer,
				        data3Ptr->dataPtr +
				        data3Ptr->
				        offsetToImportName +
				        importEntry->offsetToName);
				return;
			}
		}
		}
		importEntry++;
	}

	buffer[0] = 0;

}

char *popDecomp() {
	//  printf("<----\n");

	if (!decompileStackPosition) {
		return ("");
	}

	decompileStackPosition--;

	return decompileStack[decompileStackPosition];
}

void getByteFromDecompScript(char *buffer) {
	short int var = currentDecompScript[currentDecompScriptPtr->var4];

	currentDecompScriptPtr->var4 = currentDecompScriptPtr->var4 + 1;

	if (var == -1) {
		resolveDecompChar(buffer);

		if (buffer[0])
			return;
	}

	sprintf(buffer, "%d", var);
}

char getByteFromDecompScriptReal() {
	short int var = currentDecompScript[currentDecompScriptPtr->var4];

	currentDecompScriptPtr->var4 = currentDecompScriptPtr->var4 + 1;

	return var;
}

void getShortFromDecompScript(char *buffer) {
	int16 var = (int16)READ_BE_UINT16(currentDecompScript + currentDecompScriptPtr->var4);

	currentDecompScriptPtr->var4 = currentDecompScriptPtr->var4 + 2;

	if (var == -1) {
		resolveDecompShort(buffer);

		if (buffer[0])
			return;
	}

	sprintf(buffer, "%d", var);
}

int16 getShortFromDecompScriptReal() {
	int16 var = (int16)READ_BE_UINT16(currentDecompScript + currentDecompScriptPtr->var4);

	currentDecompScriptPtr->var4 = currentDecompScriptPtr->var4 + 2;

	return var;
}

void addDecomp(char *string, ...) {
	va_list va;

	/* fprintf(fHandle,"%d: ",currentLineIdx);
	 *
	 * va_start(va,string);
	 * vfprintf(fHandle,string,va);
	 * va_end(va);
	 *
	 * fprintf(fHandle,"\n"); */

	struct decompileLineStruct *pLineStruct =
				    &decompileLineTable[positionInDecompileLineTable++];

	pLineStruct->lineOffset = currentLineIdx;
	pLineStruct->indent = 0;
	pLineStruct->type = currentLineType;
	pLineStruct->line[0] = 0;
	pLineStruct->pendingElse = 0;

	va_start(va, string);
	vsprintf(pLineStruct->line, string, va);
	va_end(va);

	currentLineIdx = currentDecompScriptPtr->var4;
	currentLineType = 0;

	/*printf("%d: ",currentOffset);
	 *
	 * va_start(va,string);
	 * vprintf(string,va);
	 * va_end(va);
	 *
	 * printf("\n"); */
}

void resolveVarName(char *ovlIdxString, int varType, char *varIdxString,
                    char *outputName) {
	int varIdx = atoi(varIdxString);

	strcpy(outputName, "");

	if (varType == 2) {
		strcpy(outputName, getStringNameFromIdx(varType,
		                                        varIdxString));
		return;
	}
	if (varType == 1) {
		sprintf(outputName, "localVar_%s", varIdxString);
		return;
	}

	if (!strcmp(ovlIdxString, "0")) {
		int i;

		for (i = 0; i < currentDecompOvl->numSymbGlob; i++) {
			if (varIdx == currentDecompOvl->arraySymbGlob[i].idx) {
				if (((currentDecompOvl->arraySymbGlob[i].var4 & 0xF0) == 0) && varType != 0x20) {	// var
					strcpy(outputName,
					       currentDecompOvl->arrayNameSymbGlob +
					       currentDecompOvl->arraySymbGlob[i].
					       offsetToName);
					return;
				}
				if ((currentDecompOvl->arraySymbGlob[i].var4) == 20 && varType == 0x20) {	// script
					strcpy(outputName,
					       currentDecompOvl->arrayNameSymbGlob +
					       currentDecompOvl->arraySymbGlob[i].
					       offsetToName);
					return;
				}
			}
		}
		sprintf(outputName, "ovl(%s).[%d][%s]", ovlIdxString, varType,
		        varIdxString);
	} else {
		strcpy(outputName, ovlIdxString);
	}
}

int decompLoadVar() {
	switch (currentScriptOpcodeType) {
	case 0: {
		char buffer[256];

		getShortFromDecompScript(buffer);

		pushDecomp(buffer);

		return (0);
	}
	// string
	case 1: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];
		char varName[256];

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		resolveVarName(buffer2, atoi(buffer1) & 7, buffer3,
		               varName);

		pushDecomp("%s", varName);
		return (0);
	}
	case 2: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];
		char varName[256];

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		resolveVarName(buffer2, atoi(buffer1) & 7, buffer3,
		               varName);

		pushDecomp("%s", varName);
		return (0);
	}
	case 5: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];
		char varName[256];

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		resolveVarName(buffer2, atoi(buffer1) & 7, buffer3,
		               varName);

		pushDecomp("%s[%s]", varName, decompSaveOpcodeVar);
		return (0);
	}
	default: {
		debug("Unsupported type %d in opcodeType0",
		       currentScriptOpcodeType);
		failed = 1;
	}
	}
}

int decompSaveVar() {
//      int var = popVar();

	switch (currentScriptOpcodeType) {
	case 0: {
		addDecomp(popDecomp());
		return (0);
	}
	// modify string
	case 1: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];
		char varName[256];
		uint8 type;

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		type = atoi(buffer1) & 7;

		resolveVarName(buffer2, type, buffer3, varName);

		addDecomp("%s = %s", varName, popDecomp());
		break;
	}
	case 2: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		addDecomp("ovl(%s).setVar(%s,%s) = %s", buffer2,
		          buffer1, buffer3, popDecomp());
		break;
	}
	case 4: {
		strcpy(decompSaveOpcodeVar, popDecomp());
		break;
	}
	case 5: {
		char buffer1[256];
		char buffer2[256];
		char buffer3[256];
		char varName[256];
		uint8 type;

		getByteFromDecompScript(buffer1);
		getByteFromDecompScript(buffer2);

		getShortFromDecompScript(buffer3);

		type = atoi(buffer1) & 7;

		resolveVarName(buffer2, type, buffer3, varName);

		addDecomp("%s[%s] = %s", varName, decompSaveOpcodeVar,
		          popDecomp());
		break;
	}
	default: {
		debug("Unsupported type %d in opcodeType1",
		       currentScriptOpcodeType);
		failed = 1;
	}
	}

	return (0);
}

int decompOpcodeType2() {
	switch (currentScriptOpcodeType) {
	case 1: {
		char buffer3[256];
		char varName[256];
		int byte1 = getByteFromDecompScriptReal();
		int byte2 = getByteFromDecompScriptReal();
		getShortFromDecompScript(buffer3);

		resolveVarName("0", byte1 & 7, buffer3, varName);

		pushDecomp(varName);

		break;
	}
	case 5: {
		int byte1 = getByteFromDecompScriptReal();
		int byte2 = getByteFromDecompScriptReal();
		short int short1 = getShortFromDecompScriptReal();

		int8 *ptr = scriptDataPtrTable[byte1 & 7] + short1;

		if ((byte1 & 7) == 2) {
			pushDecomp("\"%s\"[%s]", ptr,
			           decompSaveOpcodeVar);
		} else if ((byte1 & 7) == 5) {
			pushDecomp("freeString[%d][%s]", short1,
			           decompSaveOpcodeVar);
		} else {
			debug("Unsupported type %d in opcodeType2",
			       byte1 & 7);
			failed = 1;
		}
		break;
	}
	default: {
		debug("Unsupported type %d in opcodeType2",
		       currentScriptOpcodeType);
		failed = 1;
	}
	}
	return (0);
}

int decompMath() {
	char *param1 = popDecomp();
	char *param2 = popDecomp();

	switch (currentScriptOpcodeType) {
	case 0: {
		sprintf(tempbuffer, "%s+%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 1: {
		sprintf(tempbuffer, "%s/%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 2: {
		sprintf(tempbuffer, "%s-%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 3: {
		sprintf(tempbuffer, "%s*%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 4: {
		sprintf(tempbuffer, "%s\%%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 5:
	case 7: {
		sprintf(tempbuffer, "%s|%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	case 6: {
		sprintf(tempbuffer, "%s&%s", param1, param2);
		pushDecomp(tempbuffer);
		break;
	}

	default: {
		sprintf(tempbuffer, "decompMath(%d,%s,%s)",
		        currentScriptOpcodeType, param1, param2);
		pushDecomp(tempbuffer);
		break;
	}
	}
	return (0);
}

int decompBoolCompare() {
	char *param1;
	char *param2;

	param1 = popDecomp();
	param2 = popDecomp();

	sprintf(tempbuffer, "compare(%s,%s)", param1, param2);
	pushDecomp(tempbuffer);

	return 0;
}

int decompTest() {
	unsigned long int oldOffset = currentDecompScriptPtr->var4;
	short int offset = getShortFromDecompScriptReal();

	switch (currentScriptOpcodeType) {
	case 0: {
		currentLineType = 1;
		addDecomp("test '!(bitMask & 1)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 1: {
		currentLineType = 1;
		addDecomp("test '(bitMask & 1)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 2: {
		currentLineType = 1;
		addDecomp("test '(bitMask & 2)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 3: {
		currentLineType = 1;
		addDecomp("test '(bitMask & 3)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 4: {
		currentLineType = 1;
		addDecomp("test '(bitMask & 4)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 5: {
		currentLineType = 1;
		addDecomp("test '(bitMask & 5)' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 6: {
		currentLineType = 2;
		addDecomp("test 'never' and goto %d",
		          offset + oldOffset);
		break;
	}
	case 7: {
		currentLineType = 3;
		addDecomp("goto %d", offset + oldOffset);
		break;
	}

	}

	return 0;
}

int decompCompare() {
	char *param;

	param = popDecomp();

	addDecomp("sign(%s)", param);

	/*
	  if (!pop)
	    si = 1;

	  if (pop<0) {
	    si |= 4;
	  }

	  if (pop>0) {
	    si |= 2;
	  }

	  currentScriptPtr->bitMask = si;
	*/

	return 0;
}

int decompSwapStack() {
	char *stack1;
	char *stack2;
	char buffer1[4000];
	char buffer2[4000];

	stack1 = popDecomp();
	stack2 = popDecomp();

	strcpy(buffer1, stack1);
	strcpy(buffer2, stack2);

	pushDecomp(buffer1);
	pushDecomp(buffer2);

	return 0;
}

int decompFunction() {
	currentScriptOpcodeType = getByteFromDecompScriptReal();
//    addDecomp("OP_%X", currentScriptOpcodeType);
	switch (currentScriptOpcodeType) {
	case 0x1: {
		pushDecomp("_setdoFade()");
		break;
	}
	case 0x2: {
		pushDecomp("_prepareFade()");
		break;
	}
	case 0x3: {
		sprintf(tempbuffer, "_loadBackground(%s,%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x4: {
		sprintf(tempbuffer, "_loadFullBundle(%s,%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x5: {
		sprintf(tempbuffer, "_addCell(%s,%s,%s)", popDecomp(),
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x6: {
		unsigned long int numArg = atoi(popDecomp());
		char *ovlStr;
		char *idxStr;
		int i;
		char functionName[100];

		idxStr = popDecomp();
		ovlStr = popDecomp();

		resolveVarName(ovlStr, 0x20, idxStr, functionName);

		sprintf(tempbuffer, "_startASync(%s", functionName);

		for (i = 0; i < numArg; i++) {
			strcatuint8(tempbuffer, ",");
			strcatuint8(tempbuffer, popDecomp());
		}

		strcatuint8(tempbuffer, ")");

		pushDecomp(tempbuffer);
		break;
	}
	case 0x7: {
		char *var1;
		char *objIdxStr;
		char *ovlStr;

		var1 = popDecomp();
		objIdxStr = popDecomp();
		ovlStr = popDecomp();

		sprintf(tempbuffer,
		        "_createObjectFromOvlData(ovl:%s,dataIdx:%s,%s)",
		        ovlStr, objIdxStr, var1);
		pushDecomp(tempbuffer);
		break;
	}
	case 0x8: {
		sprintf(tempbuffer, "_removeCell(%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x9: {
		pushDecomp("_freeobjectList()");
		break;
	}
	case 0xA: {
		sprintf(tempbuffer, "_removeScript(ovl(%s),%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0xB: {
		sprintf(tempbuffer, "_resetFilesEntries(%s,%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0xC: {
		sprintf(tempbuffer, "_loadOverlay(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0xD: {
		sprintf(tempbuffer, "_palManipulation(%s,%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0xE: {
		sprintf(tempbuffer, "_playSample(%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x10: {
		sprintf(tempbuffer, "_releaseScript2(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x11: {
		sprintf(tempbuffer, "_getOverlayIdx(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x13: {
		sprintf(tempbuffer,
		        "_displayMessage(%s,\"%s\",%s,%s,%s,%s)",
		        popDecomp(), resolveMessage(popDecomp()),
		        popDecomp(), popDecomp(), popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x14: {
		sprintf(tempbuffer, "_removeObject(ovl(%s),%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x15: {
		pushDecomp("_pauseScript()");
		break;
	}
	case 0x16: {
		sprintf(tempbuffer,
		        "_Op_FreezeCell(%s,%s,%s,%s,%s,%s)", popDecomp(),
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x17: {
		sprintf(tempbuffer, "_loadCtp(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x18: {
		sprintf(tempbuffer,
		        "_Op_AddAnimation(%s,%s,%s,%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x19: {
		sprintf(tempbuffer, "_Op_RemoveAnimation(%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x1A: {
		sprintf(tempbuffer, "_setupScaleFormula(%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x1E: {
		sprintf(tempbuffer, "_Op_TrackAnim(%s,%s,%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x21: {
		sprintf(tempbuffer, "_isActorLoaded(%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x22: {
		sprintf(tempbuffer, "_computeScale(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x23: {
		sprintf(tempbuffer, "_convertToScale(%s,%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x24: {
		sprintf(tempbuffer, "_op_24(%s,%s,%s,%s)", popDecomp(),
		        popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x27: {
		sprintf(tempbuffer, "_getWalkBoxCollision(%s,%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x28: {
		sprintf(tempbuffer, "_changeSaveAllowedState(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x29: {
		pushDecomp("_freeAllPerso()");
		break;
	}
	case 0x2B: {
		sprintf(tempbuffer, "_getProcIdx(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x2C: {
		sprintf(tempbuffer, "_setObjectPosition(%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x2E: {
		sprintf(tempbuffer, "_releaseScript(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x2F: {
		sprintf(tempbuffer, "_addBackgroundIncrust(%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x30: {
		sprintf(tempbuffer, "_removeBackgroundIncrust(%s,%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x31: {
		sprintf(tempbuffer, "_op_31(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x32: {
		pushDecomp("_freeBackgroundInscrustList()");
		break;
	}
	case 0x35: {
		sprintf(tempbuffer, "_op35(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x37: {
		sprintf(tempbuffer, "_op37(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x38: {
		sprintf(tempbuffer, "_removeBackground(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x39: {
		sprintf(tempbuffer, "_SetActiveBackgroundPlane(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x3A: {
		sprintf(tempbuffer, "_setVar49(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x3B: {
		pushDecomp("_op3B()");
		break;
	}
	case 0x3C: {
		sprintf(tempbuffer, "_rand(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x3D: {
		sprintf(tempbuffer, "_loadMusic(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x3E: {
		pushDecomp("_op_3E()");
		break;
	}
	case 0x3F: {
		pushDecomp("_op_3F()");
		break;
	}
	case 0x40: {
		pushDecomp("_op_40()");
		break;
	}
	case 0x41: {
		sprintf(tempbuffer, "_isFileLoaded2(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x45: {
		pushDecomp("_stopSound()");
		break;
	}
	case 0x49: {
		sprintf(tempbuffer, "_op49(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x54: {
		sprintf(tempbuffer, "_setFontVar(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x56: {
		sprintf(tempbuffer, "_changeCutSceneState(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x57: {
		pushDecomp("_getMouseX()");
		break;
	}
	case 0x58: {
		pushDecomp("_getMouseY()");
		break;
	}
	case 0x59: {
		pushDecomp("_getMouse3()");
		break;
	}
	case 0x5A: {
		sprintf(tempbuffer, "_isFileLoaded(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x5B: {
		pushDecomp("_regenerateBackgroundIncrust()");
		break;
	}
	case 0x5C: {
		sprintf(tempbuffer, "_Op_AddCellC(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x5E: {
		sprintf(tempbuffer, "_Op_AddCellE(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x60: {
		sprintf(tempbuffer, "_op_60(%s,%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x61: {
		sprintf(tempbuffer, "_op61(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x62: {
		pushDecomp("_pauseCallingScript()");
		break;
	}
	case 0x63: {
		pushDecomp("_resumeScript()");
		break;
	}
	case 0x64: {
		unsigned long int numArg = atoi(popDecomp());
		char *ovlStr;
		char *idxStr;
		int i;
		char functionName[256];

		idxStr = popDecomp();
		ovlStr = popDecomp();

		resolveVarName(ovlStr, 0x20, idxStr, functionName);

		sprintf(tempbuffer, "%s(", functionName);

		for (i = 0; i < numArg; i++) {
			if (i)
				strcatuint8(tempbuffer, ",");
			strcatuint8(tempbuffer, popDecomp());
		}

		strcatuint8(tempbuffer, ")");

		pushDecomp(tempbuffer);
		break;
	}
	case 0x65: {
		sprintf(tempbuffer,
		        "_addWaitObject(%s,%s,%s,%s,%s,%s,%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x66: {
		sprintf(tempbuffer, "_op_66(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x67: {
		sprintf(tempbuffer, "_loadAudioResource(%s,%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x68: {
		pushDecomp("_freeMediumVar()");
		break;
	}
	case 0x6A: {
		sprintf(tempbuffer, "_op_6A(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x6B: {
		sprintf(tempbuffer, "_loadData(%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x6C: {
		sprintf(tempbuffer, "_op_6C(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x6D: {
		sprintf(tempbuffer, "_strcpy(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x6E: {
		sprintf(tempbuffer, "_op_6E(%s,%s)", popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x6F: {
		unsigned long int numArg = atoi(popDecomp());
		char *ovlStr;
		char *idxStr;
		int i;

		idxStr = popDecomp();
		ovlStr = popDecomp();

		sprintf(tempbuffer, "_op_6F(%s,%s", idxStr, ovlStr);

		for (i = 0; i < numArg; i++) {
			strcatuint8(tempbuffer, ",");
			strcatuint8(tempbuffer, popDecomp());
		}

		strcatuint8(tempbuffer, ")");

		pushDecomp(tempbuffer);
		break;
	}
	case 0x70: {
		sprintf(tempbuffer, "_comment(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x71: {
		sprintf(tempbuffer, "_op71(%s,%s,%s,%s,%s)",
		        popDecomp(), popDecomp(), popDecomp(), popDecomp(),
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x72: {
		sprintf(tempbuffer, "_op72(%s,%s)", popDecomp(),
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x73: {
		sprintf(tempbuffer, "_op73(%s)", popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x74: {
		sprintf(tempbuffer, "_getlowMemory()");
		pushDecomp(tempbuffer);
		break;
	}
	case 0x76: {
		sprintf(tempbuffer, "_Op_InitializeState6(%s,%s)",
		        popDecomp(), popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x77: {
		sprintf(tempbuffer, "_Op_InitializeState7(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x78: {
		sprintf(tempbuffer, "_Op_InitializeState8(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x79: {
		sprintf(tempbuffer, "_EnterPlayerMenu(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x7B: {
		sprintf(tempbuffer, "_Op_InitializeStateB(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x7C: {
		sprintf(tempbuffer, "_Op_InitializeStateC(%s)",
		        popDecomp());
		pushDecomp(tempbuffer);
		break;
	}
	case 0x7D: {
		pushDecomp("_freeAllMenu()");
		break;
	}
	default: {
		addDecomp("OP_%X", currentScriptOpcodeType);
		debug("OPCODE: %X", currentScriptOpcodeType);
		failed = 1;
		break;
	}
	}

//    pushDecomp("functionDummyPush");

	return (0);
}

uint8 stop = 0;

int decompStop() {
	stop = 1;
	addDecomp("stop\n");
	return 0;
}

int decompBreak() {
	addDecomp("break");
	return 0;
}

void generateIndentation() {
	int i, j;

	for (i = 0; i < positionInDecompileLineTable; i++) {
		if (decompileLineTable[i].type != 0) {
			char *gotoStatement;
			int destLine;
			int destLineIdx;

			gotoStatement =
			    strstr(decompileLineTable[i].line, "goto");
			assert(gotoStatement);
			gotoStatement = strchr(gotoStatement, ' ') + 1;

			destLine = atoi(gotoStatement);
			destLineIdx = -1;

			for (j = 0; j < positionInDecompileLineTable; j++) {
				if (decompileLineTable[j].lineOffset == destLine) {
					destLineIdx = j;
					break;
				}
			}

			assert(destLineIdx != -1);

			if (destLineIdx > i) {
				for (j = i + 1; j < destLineIdx; j++) {
					decompileLineTable[j].indent++;
				}

				if (strstr(decompileLineTable[destLineIdx - 1].line, "goto") ==
				        decompileLineTable[destLineIdx - 1].line) {
					//decompileLineTable[destLineIdx-1].pendingElse = 1;
				}
			}
		}
	}
}

void dumpScript(uint8 *ovlName, ovlDataStruct *ovlData, int idx) {
	uint8 opcodeType;
	char buffer[256];
	int i;

	char temp[256];
	char scriptName[256];

	sprintf(temp, "%d", idx);

	failed = 0;

	currentScript = &ovlData->arrayProc[idx];

	currentDecompScript = currentScript->dataPtr;
	currentDecompScriptPtr->var4 = 0;

	currentDecompOvl = ovlData;
	currentDecompScriptIdx = idx;

	currentLineIdx = 0;
	positionInDecompileLineTable = 0;
	currentLineType = 0;

	resolveVarName("0", 0x20, temp, scriptName);

	debug("decompiling script %d - %s", idx, scriptName);

	// return;

//      scriptDataPtrTable[1] = *(char**)(ptr+0x6);
	scriptDataPtrTable[2] = getDataFromData3(currentScript, 1);	// strings
	scriptDataPtrTable[5] = ovlData->data4Ptr;	// free strings
	scriptDataPtrTable[6] = ovlData->ptr8;

	stop = 0;

	sprintf(buffer, "%s-%02d-%s.txt", ovlName, idx, scriptName);
	fHandle = fopen(buffer, "w+");

	decompileStackPosition = 0;

	for (i = 0; i < 64; i++) {
		decompOpcodeTypeTable[i] = NULL;
	}

	decompOpcodeTypeTable[1] = decompLoadVar;
	decompOpcodeTypeTable[2] = decompSaveVar;
	decompOpcodeTypeTable[3] = decompOpcodeType2;
	decompOpcodeTypeTable[4] = decompMath;
	decompOpcodeTypeTable[5] = decompBoolCompare;
	decompOpcodeTypeTable[6] = decompTest;
	decompOpcodeTypeTable[7] = decompCompare;
	decompOpcodeTypeTable[8] = decompSwapStack;
	decompOpcodeTypeTable[9] = decompFunction;
	decompOpcodeTypeTable[10] = decompStop;
	decompOpcodeTypeTable[12] = decompBreak;

	do {
		currentOffset = currentDecompScriptPtr->var4;

		opcodeType = getByteFromDecompScriptReal();

		currentScriptOpcodeType = opcodeType & 7;

		if (!decompOpcodeTypeTable[(opcodeType & 0xFB) >> 3]) {
			debug("Unsupported opcode type %d in decomp",
			       (opcodeType & 0xFB) >> 3);
			return;
		}

		//debug("Optype: %d",(opcodeType&0xFB)>>3);

		decompOpcodeTypeTable[(opcodeType & 0xFB) >> 3]();

		if (failed) {
			debug("Aborting decompilation..");
			fclose(fHandle);
			return;
		}

	} while (!stop);

	dumpIdx++;

	generateIndentation();

	for (i = 0; i < positionInDecompileLineTable; i++) {
		int j;

		if (decompileLineTable[i].pendingElse) {
			fprintf(fHandle, "%05d:\t",
			        decompileLineTable[i].lineOffset);
			fprintf(fHandle, "else", decompileLineTable[i].line);
			fprintf(fHandle, "\n");
		}

		fprintf(fHandle, "%05d:\t", decompileLineTable[i].lineOffset);
		for (j = 0; j < decompileLineTable[i].indent; j++) {
			fprintf(fHandle, "\t");
		}
		fprintf(fHandle, "%s", decompileLineTable[i].line);
		fprintf(fHandle, "\n");
	}

	fclose(fHandle);
}

#endif

} // End of namespace Cruise
