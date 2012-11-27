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
 *
 */
/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "mpal.h"
#include "mpaldll.h"
#include "memory.h"
#include "tony/tony.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Static functions
\****************************************************************************/

static bool compareCommands(struct Command *cmd1, struct Command *cmd2) {
	if (cmd1->_type == 2 && cmd2->_type == 2) {
		if (strcmp(cmd1->_lpszVarName, cmd2->_lpszVarName) == 0 &&
			compareExpressions(cmd1->_expr, cmd2->_expr))
			return true;
		else
			return false;
	} else
		return (memcmp(cmd1, cmd2, sizeof(struct Command)) == 0);
}

/**
 * Parses a script from the MPC file, and inserts its data into a structure
 *
 * @param lpBuf				Buffer containing the compiled script.
 * @param lpmsScript		Pointer to a structure that will be filled with the
 * data of the script.
 * @returns		Pointer to the buffer after the item, or NULL on failure.
 */
static const byte *ParseScript(const byte *lpBuf, LpMpalScript lpmsScript) {
	lpmsScript->_nObj = (int32)READ_LE_UINT32(lpBuf);
	lpBuf += 4;

	lpmsScript->_nMoments = READ_LE_UINT16(lpBuf);
	lpBuf += 2;

	int curCmd = 0;

	for (uint i = 0; i < lpmsScript->_nMoments; i++) {
		lpmsScript->_moment[i]._dwTime = (int32)READ_LE_UINT32(lpBuf);
		lpBuf += 4;
		lpmsScript->_moment[i]._nCmds = *lpBuf;
		lpBuf++;

		for (int j = 0; j < lpmsScript->_moment[i]._nCmds; j++) {
			lpmsScript->_command[curCmd]._type = *lpBuf;
			lpBuf++;
			switch (lpmsScript->_command[curCmd]._type) {
			case 1:
				lpmsScript->_command[curCmd]._nCf = READ_LE_UINT16(lpBuf);
				lpBuf += 2;
				lpmsScript->_command[curCmd]._arg1 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmsScript->_command[curCmd]._arg2 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmsScript->_command[curCmd]._arg3 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmsScript->_command[curCmd]._arg4 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				break;

			case 2: { // Variable assign
				int len = *lpBuf;
				lpBuf++;
				lpmsScript->_command[curCmd]._lpszVarName = (char *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, len + 1);
				if (lpmsScript->_command[curCmd]._lpszVarName == NULL)
					return NULL;
				memcpy(lpmsScript->_command[curCmd]._lpszVarName, lpBuf, len);
				lpBuf += len;

				lpBuf = parseExpression(lpBuf, &lpmsScript->_command[curCmd]._expr);
				if (lpBuf == NULL)
					return NULL;
				break;
			}
			default:
				return NULL;
			}

			lpmsScript->_moment[i]._cmdNum[j] = curCmd;
			curCmd++;
		}
	}
	return lpBuf;
}

/**
 * Frees a script allocated via a previous call to ParseScript
 *
 * @param lpmsScript		Pointer to a script structure
 */
static void FreeScript(LpMpalScript lpmsScript) {
	for (int i = 0; i < MAX_COMMANDS_PER_SCRIPT && (lpmsScript->_command[i]._type); ++i, ++lpmsScript) {
		if (lpmsScript->_command[i]._type == 2) {
			// Variable Assign
			globalDestroy(lpmsScript->_command[i]._lpszVarName);
			freeExpression(lpmsScript->_command[i]._expr);
		}
	}
}

/**
 * Parses a dialog from the MPC file, and inserts its data into a structure
 *
 * @param lpBuf				Buffer containing the compiled dialog.
 * @param lpmdDialog		Pointer to a structure that will be filled with the
 * data of the dialog.
 * @returns		Pointer to the buffer after the item, or NULL on failure.
 */
static const byte *parseDialog(const byte *lpBuf, LpMpalDialog lpmdDialog) {
	byte *lpLock;

	lpmdDialog->_nObj = READ_LE_UINT32(lpBuf);
	lpBuf += 4;

	// Periods
	uint32 num = READ_LE_UINT16(lpBuf);
	lpBuf += 2;

	if (num >= MAX_PERIODS_PER_DIALOG - 1)
		error("Too much periods in dialog #%d", lpmdDialog->_nObj);

	uint32 i;
	for (i = 0; i < num; i++) {
		lpmdDialog->_periodNums[i] = READ_LE_UINT16(lpBuf);
		lpBuf += 2;
		lpmdDialog->_periods[i] = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, *lpBuf + 1);
		lpLock = (byte *)globalLock(lpmdDialog->_periods[i]);
		Common::copy(lpBuf + 1, lpBuf + 1 + *lpBuf, lpLock);
		globalUnlock(lpmdDialog->_periods[i]);
		lpBuf += (*lpBuf) + 1;
	}

	lpmdDialog->_periodNums[i] = 0;
	lpmdDialog->_periods[i] = NULL;

	// Groups
	num = READ_LE_UINT16(lpBuf);
	lpBuf += 2;
	uint32 curCmd = 0;

	if (num >= MAX_GROUPS_PER_DIALOG)
		error("Too much groups in dialog #%d", lpmdDialog->_nObj);

	for (i = 0; i < num; i++) {
		lpmdDialog->_group[i]._num = READ_LE_UINT16(lpBuf);
		lpBuf += 2;
		lpmdDialog->_group[i]._nCmds = *lpBuf; lpBuf++;

		if (lpmdDialog->_group[i]._nCmds >= MAX_COMMANDS_PER_GROUP)
			error("Too much commands in group #%d in dialog #%d", lpmdDialog->_group[i]._num, lpmdDialog->_nObj);

		for (uint32 j = 0; j < lpmdDialog->_group[i]._nCmds; j++) {
			lpmdDialog->_command[curCmd]._type = *lpBuf;
			lpBuf++;

			switch (lpmdDialog->_command[curCmd]._type) {
			// Call custom function
			case 1:
				lpmdDialog->_command[curCmd]._nCf = READ_LE_UINT16(lpBuf);
				lpBuf += 2;
				lpmdDialog->_command[curCmd]._arg1 = READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmdDialog->_command[curCmd]._arg2 = READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmdDialog->_command[curCmd]._arg3 = READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmdDialog->_command[curCmd]._arg4 = READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				break;

			// Variable assign
			case 2: {
				uint32 len = *lpBuf;
				lpBuf++;
				lpmdDialog->_command[curCmd]._lpszVarName = (char *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, len + 1);
				if (lpmdDialog->_command[curCmd]._lpszVarName == NULL)
					return NULL;

				Common::copy(lpBuf, lpBuf + len, lpmdDialog->_command[curCmd]._lpszVarName);
				lpBuf += len;

				lpBuf = parseExpression(lpBuf, &lpmdDialog->_command[curCmd]._expr);
				if (lpBuf == NULL)
					return NULL;
				break;
			}

			// Do Choice
			case 3:
				lpmdDialog->_command[curCmd]._nChoice = READ_LE_UINT16(lpBuf);
				lpBuf += 2;
				break;

			default:
				return NULL;
			}

			uint32 kk;
			for (kk = 0;kk < curCmd; kk++) {
				if (compareCommands(&lpmdDialog->_command[kk], &lpmdDialog->_command[curCmd])) {
					lpmdDialog->_group[i]._cmdNum[j] = kk;

					// Free any data allocated for the duplictaed command
					if (lpmdDialog->_command[curCmd]._type == 2) {
						globalDestroy(lpmdDialog->_command[curCmd]._lpszVarName);
						freeExpression(lpmdDialog->_command[curCmd]._expr);

						lpmdDialog->_command[curCmd]._lpszVarName = NULL;
						lpmdDialog->_command[curCmd]._expr = 0;
						lpmdDialog->_command[curCmd]._type = 0;
					}
					break;
				}
			}

			if (kk == curCmd) {
				lpmdDialog->_group[i]._cmdNum[j] = curCmd;
				curCmd++;
			}
		}
	}

	if (curCmd >= MAX_COMMANDS_PER_DIALOG)
		error("Too much commands in dialog #%d", lpmdDialog->_nObj);

	// Choices
	num = READ_LE_UINT16(lpBuf);
	lpBuf += 2;

	if (num >= MAX_CHOICES_PER_DIALOG)
		error("Too much choices in dialog #%d", lpmdDialog->_nObj);

	for (i = 0; i < num; i++) {
		lpmdDialog->_choice[i]._nChoice = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		uint32 num2 = *lpBuf++;

		if (num2 >= MAX_SELECTS_PER_CHOICE)
			error("Too much selects in choice #%d in dialog #%d", lpmdDialog->_choice[i]._nChoice, lpmdDialog->_nObj);

		for (uint32 j = 0; j < num2; j++) {
			// When
			switch (*lpBuf++) {
			case 0:
				lpmdDialog->_choice[i]._select[j]._when = NULL;
				break;

			case 1:
				lpBuf = parseExpression(lpBuf, &lpmdDialog->_choice[i]._select[j]._when);
				if (lpBuf == NULL)
					return NULL;
				break;

			case 2:
				return NULL;
			}

			// Attrib
			lpmdDialog->_choice[i]._select[j]._attr = *lpBuf++;

			// Data
			lpmdDialog->_choice[i]._select[j]._dwData = READ_LE_UINT32(lpBuf);
			lpBuf += 4;

			// PlayGroup
			uint32 num3 = *lpBuf++;

			if (num3 >= MAX_PLAYGROUPS_PER_SELECT)
				error("Too much playgroups in select #%d in choice #%d in dialog #%d", j, lpmdDialog->_choice[i]._nChoice, lpmdDialog->_nObj);

			for (uint32 z = 0; z < num3; z++) {
				lpmdDialog->_choice[i]._select[j]._wPlayGroup[z] = READ_LE_UINT16(lpBuf);
				lpBuf += 2;
			}

			lpmdDialog->_choice[i]._select[j]._wPlayGroup[num3] = 0;
		}

		// Mark the last selection
		lpmdDialog->_choice[i]._select[num2]._dwData = 0;
	}

	lpmdDialog->_choice[num]._nChoice = 0;

	return lpBuf;
}

/**
 * Parses an item from the MPC file, and inserts its data into a structure
 *
 * @param lpBuf				Buffer containing the compiled dialog.
 * @param lpmiItem			Pointer to a structure that will be filled with the
 * data of the item.
 * @returns		Pointer to the buffer after the item, or NULL on failure.
 * @remarks		It's necessary that the structure that is passed  has been
 * completely initialized to 0 beforehand.
 */
static const byte *parseItem(const byte *lpBuf, LpMpalItem lpmiItem) {
	lpmiItem->_nObj = (int32)READ_LE_UINT32(lpBuf);
	lpBuf += 4;

	byte len = *lpBuf;
	lpBuf++;
	memcpy(lpmiItem->_lpszDescribe, lpBuf, MIN((byte)127, len));
	lpBuf += len;

	if (len >= MAX_DESCRIBE_SIZE)
		error("Describe too long in item #%d", lpmiItem->_nObj);

	lpmiItem->_nActions=*lpBuf;
	lpBuf++;

	// Allocation action
	if (lpmiItem->_nActions > 0)
		lpmiItem->_action = (ItemAction *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(struct ItemAction) * (int)lpmiItem->_nActions);

	uint32 curCmd = 0;

	for (uint32 i = 0; i < lpmiItem->_nActions; i++) {
		lpmiItem->_action[i]._num = *lpBuf;
		lpBuf++;

		lpmiItem->_action[i]._wParm = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		if (lpmiItem->_action[i]._num == 0xFF) {
			lpmiItem->_action[i]._wTime = READ_LE_UINT16(lpBuf);
			lpBuf += 2;

			lpmiItem->_action[i]._perc = *lpBuf;
			lpBuf++;
		}

		if (*lpBuf == 0) {
			lpBuf++;
			lpmiItem->_action[i]._when = NULL;
		} else {
			lpBuf++;
			lpBuf = parseExpression(lpBuf,&lpmiItem->_action[i]._when);
			if (lpBuf == NULL)
				return NULL;
		}

		lpmiItem->_action[i]._nCmds=*lpBuf;
		lpBuf++;

		if (lpmiItem->_action[i]._nCmds >= MAX_COMMANDS_PER_ACTION)
			error("Too much commands in action #%d in item #%d", lpmiItem->_action[i]._num, lpmiItem->_nObj);

		for (uint32 j = 0; j < lpmiItem->_action[i]._nCmds; j++) {
			lpmiItem->_command[curCmd]._type = *lpBuf;
			lpBuf++;
			switch (lpmiItem->_command[curCmd]._type) {
			case 1: // Call custom function
				lpmiItem->_command[curCmd]._nCf  = READ_LE_UINT16(lpBuf);
				lpBuf += 2;
				lpmiItem->_command[curCmd]._arg1 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmiItem->_command[curCmd]._arg2 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmiItem->_command[curCmd]._arg3 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				lpmiItem->_command[curCmd]._arg4 = (int32)READ_LE_UINT32(lpBuf);
				lpBuf += 4;
				break;

			case 2: // Variable assign
				len = *lpBuf;
				lpBuf++;
				lpmiItem->_command[curCmd]._lpszVarName = (char *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, len + 1);
				if (lpmiItem->_command[curCmd]._lpszVarName == NULL)
					return NULL;
				memcpy(lpmiItem->_command[curCmd]._lpszVarName, lpBuf, len);
				lpBuf += len;

				lpBuf = parseExpression(lpBuf, &lpmiItem->_command[curCmd]._expr);
				if (lpBuf == NULL)
					return NULL;
				break;

			default:
				return NULL;
			}

			uint32 kk;
			for (kk = 0; kk < curCmd; kk++) {
				if (compareCommands(&lpmiItem->_command[kk], &lpmiItem->_command[curCmd])) {
					lpmiItem->_action[i]._cmdNum[j] = kk;

					// Free any data allocated for the duplictaed command
					if (lpmiItem->_command[curCmd]._type == 2) {
						globalDestroy(lpmiItem->_command[curCmd]._lpszVarName);
						freeExpression(lpmiItem->_command[curCmd]._expr);

						lpmiItem->_command[curCmd]._lpszVarName = NULL;
						lpmiItem->_command[curCmd]._expr = 0;
						lpmiItem->_command[curCmd]._type = 0;
					}
					break;
				}
			}

			if (kk == curCmd) {
				lpmiItem->_action[i]._cmdNum[j] = curCmd;
				curCmd++;

				if (curCmd >= MAX_COMMANDS_PER_ITEM) {
					error("Too much commands in item #%d", lpmiItem->_nObj);
					//curCmd=0;
				}
			}
		}
	}

	lpmiItem->_dwRes = READ_LE_UINT32(lpBuf);
	lpBuf += 4;

	return lpBuf;
}

/**
 * Frees an item parsed from a prior call to ParseItem
 *
 * @param lpmiItem			Pointer to an item structure
 */
static void freeItem(LpMpalItem lpmiItem) {
	// Free the actions
	if (lpmiItem->_action) {
		for (int i = 0; i < lpmiItem->_nActions; ++i) {
			if (lpmiItem->_action[i]._when != 0)
				freeExpression(lpmiItem->_action[i]._when);
		}

		globalDestroy(lpmiItem->_action);
	}

	// Free the commands
	for (int i = 0; i < MAX_COMMANDS_PER_ITEM && (lpmiItem->_command[i]._type); ++i) {
		if (lpmiItem->_command[i]._type == 2) {
			// Variable Assign
			globalDestroy(lpmiItem->_command[i]._lpszVarName);
			freeExpression(lpmiItem->_command[i]._expr);
		}
	}
}

/**
 * Parses a location from the MPC file, and inserts its data into a structure
 *
 * @param lpBuf				Buffer containing the compiled location.
 * @param lpmiLocation		Pointer to a structure that will be filled with the
 * data of the location.
 * @returns		Pointer to the buffer after the location, or NULL on failure.
 */
static const byte *ParseLocation(const byte *lpBuf, LpMpalLocation lpmlLocation) {
	lpmlLocation->_nObj = (int32)READ_LE_UINT32(lpBuf);
	lpBuf += 4;
	lpmlLocation->_dwXlen = READ_LE_UINT16(lpBuf);
	lpBuf += 2;
	lpmlLocation->_dwYlen = READ_LE_UINT16(lpBuf);
	lpBuf += 2;
	lpmlLocation->_dwPicRes = READ_LE_UINT32(lpBuf);
	lpBuf += 4;

	return lpBuf;
}

/****************************************************************************\
*       Exported functions
\****************************************************************************/
/**
 * @defgroup Exported functions
 */
//@{

/**
 * Reads and interprets the MPC file, and create structures for various directives
 * in the global variables
 *
 * @param lpBuf				Buffer containing the MPC file data, excluding the header.
 * @returns		True if succeeded OK, false if failure.
 */
bool parseMpc(const byte *lpBuf) {
	byte *lpTemp;

	// 1. Variables
	if (lpBuf[0] != 'V' || lpBuf[1] != 'A' || lpBuf[2] != 'R' || lpBuf[3] != 'S')
		return false;

	lpBuf += 4;
	GLOBALS._nVars = READ_LE_UINT16(lpBuf);
	lpBuf += 2;

	GLOBALS._hVars = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(MpalVar) * (uint32)GLOBALS._nVars);
	if (GLOBALS._hVars == NULL)
		return false;

	GLOBALS._lpmvVars = (LpMpalVar)globalLock(GLOBALS._hVars);

	for (uint16 i = 0; i < GLOBALS._nVars; i++) {
		uint16 wLen = *(const byte *)lpBuf;
		lpBuf++;
		memcpy(GLOBALS._lpmvVars->_lpszVarName, lpBuf, MIN(wLen, (uint16)32));
		lpBuf += wLen;
		GLOBALS._lpmvVars->_dwVal = READ_LE_UINT32(lpBuf);
		lpBuf += 4;

		lpBuf++; // Skip 'ext'
		GLOBALS._lpmvVars++;
	}

	globalUnlock(GLOBALS._hVars);

	// 2. Messages
	if (lpBuf[0] != 'M' || lpBuf[1] != 'S' || lpBuf[2] != 'G' || lpBuf[3] != 'S')
		return false;

	lpBuf += 4;
	GLOBALS._nMsgs = READ_LE_UINT16(lpBuf);
	lpBuf += 2;

#ifdef NEED_LOCK_MSGS
	GLOBALS._hMsgs = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(MpalMsg) * (uint32)GLOBALS._nMsgs);
	if (GLOBALS._hMsgs == NULL)
		return false;

	GLOBALS._lpmmMsgs = (LpMpalMsg)globalLock(GLOBALS._hMsgs);
#else
	GLOBALS._lpmmMsgs=(LPMPALMSG)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(MPALMSG) * (uint32)GLOBALS._nMsgs);
	if (GLOBALS._lpmmMsgs==NULL)
		return false;
#endif

	for (uint16 i = 0; i < GLOBALS._nMsgs; i++) {
		GLOBALS._lpmmMsgs->_wNum = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		uint16 j;
		for (j = 0; lpBuf[j] != 0;)
			j += lpBuf[j] + 1;

		GLOBALS._lpmmMsgs->_hText = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, j + 1);
		lpTemp = (byte *)globalLock(GLOBALS._lpmmMsgs->_hText);

		for (j = 0; lpBuf[j] != 0;) {
			memcpy(lpTemp, &lpBuf[j + 1], lpBuf[j]);
			lpTemp += lpBuf[j];
			*lpTemp ++= '\0';
			j += lpBuf[j] + 1;
		}

		lpBuf += j + 1;
		*lpTemp = '\0';

		globalUnlock(GLOBALS._lpmmMsgs->_hText);
		GLOBALS._lpmmMsgs++;
	}

#ifdef NEED_LOCK_MSGS
	globalUnlock(GLOBALS._hMsgs);
#endif

	// 3. Objects
	if (lpBuf[0] != 'O' || lpBuf[1] != 'B' || lpBuf[2] != 'J' || lpBuf[3] != 'S')
		return false;

	lpBuf += 4;
	GLOBALS._nObjs = READ_LE_UINT16(lpBuf);
	lpBuf += 2;

	// Check out the dialogs
	GLOBALS._nDialogs = 0;
	GLOBALS._hDialogs = GLOBALS._lpmdDialogs = NULL;
	if (*((const byte *)lpBuf + 2) == 6 && strncmp((const char *)lpBuf + 3, "Dialog", 6) == 0) {
		GLOBALS._nDialogs = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		GLOBALS._hDialogs = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, (uint32)GLOBALS._nDialogs * sizeof(MpalDialog));
		if (GLOBALS._hDialogs == NULL)
			return false;

		GLOBALS._lpmdDialogs = (LpMpalDialog)globalLock(GLOBALS._hDialogs);

		for (uint16 i = 0; i < GLOBALS._nDialogs; i++) {
			if ((lpBuf = parseDialog(lpBuf + 7, &GLOBALS._lpmdDialogs[i])) == NULL)
				return false;
		}

		globalUnlock(GLOBALS._hDialogs);
	}

	// Check the items
	GLOBALS._nItems = 0;
	GLOBALS._hItems = GLOBALS._lpmiItems = NULL;
	if (*(lpBuf + 2) == 4 && strncmp((const char *)lpBuf + 3, "Item", 4) == 0) {
		GLOBALS._nItems = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		// Allocate memory and read them in
		GLOBALS._hItems = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, (uint32)GLOBALS._nItems * sizeof(MpalItem));
		if (GLOBALS._hItems == NULL)
			return false;

		GLOBALS._lpmiItems = (LpMpalItem)globalLock(GLOBALS._hItems);

		for (uint16 i = 0; i < GLOBALS._nItems; i++) {
			if ((lpBuf = parseItem(lpBuf + 5, &GLOBALS._lpmiItems[i])) == NULL)
				return false;
		}

		globalUnlock(GLOBALS._hItems);
	}

	// Check the locations
	GLOBALS._nLocations = 0;
	GLOBALS._hLocations = GLOBALS._lpmlLocations = NULL;
	if (*(lpBuf + 2) == 8 && strncmp((const char *)lpBuf + 3, "Location", 8) == 0) {
		GLOBALS._nLocations = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		// Allocate memory and read them in
		GLOBALS._hLocations = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, (uint32)GLOBALS._nLocations * sizeof(MpalLocation));
		if (GLOBALS._hLocations == NULL)
			return false;

		GLOBALS._lpmlLocations = (LpMpalLocation)globalLock(GLOBALS._hLocations);

		for (uint16 i = 0; i < GLOBALS._nLocations; i++) {
			if ((lpBuf = ParseLocation(lpBuf + 9, &GLOBALS._lpmlLocations[i])) == NULL)
				return false;
		}

		globalUnlock(GLOBALS._hLocations);
	}

	// Check the scripts
	GLOBALS._nScripts = 0;
	GLOBALS._hScripts = GLOBALS._lpmsScripts = NULL;
	if (*(lpBuf + 2) == 6 && strncmp((const char *)lpBuf + 3, "Script", 6) == 0) {
		GLOBALS._nScripts = READ_LE_UINT16(lpBuf);
		lpBuf += 2;

		// Allocate memory
		GLOBALS._hScripts = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, (uint32)GLOBALS._nScripts * sizeof(MpalScript));
		if (GLOBALS._hScripts == NULL)
			return false;

		GLOBALS._lpmsScripts = (LpMpalScript)globalLock(GLOBALS._hScripts);

		for (uint16 i = 0; i < GLOBALS._nScripts; i++) {
			if ((lpBuf = ParseScript(lpBuf + 7, &GLOBALS._lpmsScripts[i])) == NULL)
				return false;

			// Sort the various moments of the script
			//qsort(
			//GLOBALS.lpmsScripts[i].Moment,
			//GLOBALS.lpmsScripts[i].nMoments,
			//sizeof(GLOBALS.lpmsScripts[i].Moment[0]),
			//(int (*)(const void *, const void *))CompareMoments
			//);
		}

		globalUnlock(GLOBALS._hScripts);
	}

	if (lpBuf[0] != 'E' || lpBuf[1] != 'N' || lpBuf[2] != 'D' || lpBuf[3] != '0')
		return false;

	return true;
}

/**
 * Free the given dialog
 */
static void freeDialog(LpMpalDialog lpmdDialog) {
	// Free the periods
	for (int i = 0; i < MAX_PERIODS_PER_DIALOG && (lpmdDialog->_periods[i]); ++i)
		globalFree(lpmdDialog->_periods[i]);

	for (int i = 0; i < MAX_COMMANDS_PER_DIALOG && (lpmdDialog->_command[i]._type); i++) {
		if (lpmdDialog->_command[i]._type == 2) {
			// Variable assign
			globalDestroy(lpmdDialog->_command[i]._lpszVarName);
			freeExpression(lpmdDialog->_command[i]._expr);
		}
	}

	// Free the choices
	for (int i = 0; i < MAX_CHOICES_PER_DIALOG; ++i) {
		for (int j = 0; j < MAX_SELECTS_PER_CHOICE; j++) {
			if (lpmdDialog->_choice[i]._select[j]._when)
				freeExpression(lpmdDialog->_choice[i]._select[j]._when);
		}
	}
}

/**
 * Frees any data allocated from the parsing of the MPC file
 */
void freeMpc() {
	// Free variables
	globalFree(GLOBALS._hVars);

	// Free messages
	LpMpalMsg lpmmMsgs = (LpMpalMsg)globalLock(GLOBALS._hMsgs);
	for (int i = 0; i < GLOBALS._nMsgs; i++, ++lpmmMsgs)
		globalFree(lpmmMsgs->_hText);

	globalUnlock(GLOBALS._hMsgs);
	globalFree(GLOBALS._hMsgs);

	// Free objects
	if (GLOBALS._hDialogs) {
		LpMpalDialog lpmdDialogs = (LpMpalDialog)globalLock(GLOBALS._hDialogs);

		for (int i = 0; i < GLOBALS._nDialogs; i++, ++lpmdDialogs)
			freeDialog(lpmdDialogs);

		globalFree(GLOBALS._hDialogs);
	}

	// Free items
	if (GLOBALS._hItems) {
		LpMpalItem lpmiItems = (LpMpalItem)globalLock(GLOBALS._hItems);

		for (int i = 0; i < GLOBALS._nItems; ++i, ++lpmiItems)
			freeItem(lpmiItems);

		globalUnlock(GLOBALS._hItems);
		globalFree(GLOBALS._hItems);
	}

	// Free the locations
	if (GLOBALS._hLocations) {
		globalFree(GLOBALS._hLocations);
	}

	// Free the scripts
	if (GLOBALS._hScripts) {
		LpMpalScript lpmsScripts = (LpMpalScript)globalLock(GLOBALS._hScripts);

		for (int i = 0; i < GLOBALS._nScripts; ++i, ++lpmsScripts) {
			FreeScript(lpmsScripts);
		}

		globalUnlock(GLOBALS._hScripts);
	}
}

//@}

} // end of namespace MPAL

} // end of namespace Tony
