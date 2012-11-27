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

#include "common/scummsys.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "tony/tony.h"
#include "tony/mpal/lzo.h"
#include "tony/mpal/mpal.h"
#include "tony/mpal/mpaldll.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Internal functions
\****************************************************************************/

/**
 * Locks the variables for access
 */
void lockVar() {
	GLOBALS._lpmvVars = (LpMpalVar)globalLock(GLOBALS._hVars);
}

/**
 * Unlocks variables after use
 */
void unlockVar() {
	globalUnlock(GLOBALS._hVars);
}

/**
 * Locks the messages for access
 */
static void LockMsg() {
#ifdef NEED_LOCK_MSGS
	GLOBALS._lpmmMsgs = (LpMpalMsg)globalLock(GLOBALS._hMsgs);
#endif
}

/**
 * Unlocks the messages after use
 */
static void UnlockMsg() {
#ifdef NEED_LOCK_MSGS
	globalUnlock(GLOBALS._hMsgs);
#endif
}

/**
 * Locks the dialogs for access
 */
static void lockDialogs() {
	GLOBALS._lpmdDialogs = (LpMpalDialog)globalLock(GLOBALS._hDialogs);
}

/**
 * Unlocks the dialogs after use
 */
static void unlockDialogs() {
	globalUnlock(GLOBALS._hDialogs);
}

/**
 * Locks the location data structures for access
 */
static void lockLocations() {
	GLOBALS._lpmlLocations = (LpMpalLocation)globalLock(GLOBALS._hLocations);
}

/**
 * Unlocks the location structures after use
 */
static void unlockLocations() {
	globalUnlock(GLOBALS._hLocations);
}

/**
 * Locks the items structures for use
 */
static void lockItems() {
	GLOBALS._lpmiItems = (LpMpalItem)globalLock(GLOBALS._hItems);
}

/**
 * Unlocks the items structures after use
 */
static void unlockItems() {
	globalUnlock(GLOBALS._hItems);
}

/**
 * Locks the script data structures for use
 */
static void LockScripts() {
	GLOBALS._lpmsScripts = (LpMpalScript)globalLock(GLOBALS._hScripts);
}

/**
 * Unlocks the script data structures after use
 */
static void unlockScripts() {
	globalUnlock(GLOBALS._hScripts);
}

/**
 * Returns the current value of a global variable
 *
 * @param lpszVarName		Name of the variable
 * @returns		Current value
 * @remarks		Before using this method, you must call lockVar() to
 * lock the global variablves for use. Then afterwards, you will
 * need to remember to call UnlockVar()
 */
int32 varGetValue(const char *lpszVarName) {
	LpMpalVar v = GLOBALS._lpmvVars;

	for (int i = 0; i < GLOBALS._nVars; v++, i++)
		if (strcmp(lpszVarName, v->_lpszVarName) == 0)
			return v->_dwVal;

	GLOBALS._mpalError = 1;
	return 0;
}

/**
 * Sets the value of a MPAL global variable
 * @param lpszVarName       Name of the variable
 * @param val				Value to set
 */
void varSetValue(const char *lpszVarName, int32 val) {
	LpMpalVar v = GLOBALS._lpmvVars;

	for (uint i = 0; i < GLOBALS._nVars; v++, i++)
		if (strcmp(lpszVarName, v->_lpszVarName) == 0) {
			v->_dwVal = val;
			if (GLOBALS._lpiifCustom != NULL && strncmp(v->_lpszVarName, "Pattern.", 8) == 0) {
				i = 0;
				sscanf(v->_lpszVarName, "Pattern.%u", &i);
				GLOBALS._lpiifCustom(i, val, -1);
			} else if (GLOBALS._lpiifCustom != NULL && strncmp(v->_lpszVarName, "Status.", 7) == 0) {
				i = 0;
				sscanf(v->_lpszVarName,"Status.%u", &i);
				GLOBALS._lpiifCustom(i, -1, val);
			}
			return;
		}

	GLOBALS._mpalError = 1;
	return;
}

/**
 * Find the index of a location within the location array. Remember to call LockLoc() beforehand.
 *
 * @param nLoc				Location number to search for
 * @returns		Index, or -1 if the location is not present
 * @remarks		This function requires the location list to have
 * first been locked with a call to LockLoc().
 */
static int locGetOrderFromNum(uint32 nLoc) {
	LpMpalLocation loc = GLOBALS._lpmlLocations;

	for (int i = 0; i < GLOBALS._nLocations; i++, loc++)
		if (loc->_nObj == nLoc)
			return i;

	return -1;
}

/**
 * Find the index of a message within the messages array
 * @param nMsg				Message number to search for
 * @returns		Index, or -1 if the message is not present
 * @remarks		This function requires the message list to have
 * first been locked with a call to LockMsg()
 */
static int msgGetOrderFromNum(uint32 nMsg) {
	LpMpalMsg msg = GLOBALS._lpmmMsgs;

	for (int i = 0; i < GLOBALS._nMsgs; i++, msg++) {
		if (msg->_wNum == nMsg)
			return i;
	}

	return -1;
}

/**
 * Find the index of an item within the items array
 * @param nItem				Item number to search for
 * @returns		Index, or -1 if the item is not present
 * @remarks		This function requires the item list to have
 * first been locked with a call to LockItems()
 */
static int itemGetOrderFromNum(uint32 nItem) {
	LpMpalItem item = GLOBALS._lpmiItems;

	for (int i = 0; i < GLOBALS._nItems; i++, item++) {
		if (item->_nObj == nItem)
			return i;
	}

	return -1;
}

/**
 * Find the index of a script within the scripts array
 * @param nScript			Script number to search for
 * @returns		Index, or -1 if the script is not present
 * @remarks		This function requires the script list to have
 * first been locked with a call to LockScripts()
 */
static int scriptGetOrderFromNum(uint32 nScript) {
	LpMpalScript script = GLOBALS._lpmsScripts;

	for (int i = 0; i < GLOBALS._nScripts; i++, script++) {
		if (script->_nObj == nScript)
			return i;
	}

	return -1;
}

/**
 * Find the index of a dialog within the dialogs array
 * @param nDialog			Dialog number to search for
 * @returns		Index, or -1 if the dialog is not present
 * @remarks		This function requires the dialog list to have
 * first been locked with a call to LockDialogs()
 */
static int dialogGetOrderFromNum(uint32 nDialog) {
	LpMpalDialog dialog = GLOBALS._lpmdDialogs;

	for (int i = 0; i < GLOBALS._nDialogs; i++, dialog++) {
		if (dialog->_nObj == nDialog)
			return i;
	}

	return -1;
}

/**
 * Duplicates a message
 * @param nMsgOrd			Index of the message inside the messages array
 * @returns		Pointer to the duplicated message.
 * @remarks		Remember to free the duplicated message when done with it.
 */
static char *DuplicateMessage(uint32 nMsgOrd) {
	const char *origmsg;
	char *clonemsg;

	if (nMsgOrd == (uint32)-1)
		return NULL;

	origmsg = (const char *)globalLock(GLOBALS._lpmmMsgs[nMsgOrd]._hText);

	int j = 0;
	while (origmsg[j] != '\0' || origmsg[j + 1] != '\0')
		j++;
	j += 2;

	clonemsg = (char *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, j);
	if (clonemsg == NULL)
		return NULL;

	memcpy(clonemsg, origmsg, j);
	globalUnlock(GLOBALS._lpmmMsgs[nMsgOrd]._hText);

	return clonemsg;
}

/**
 * Duplicate a sentence of a dialog
 * @param nDlgOrd			Index of the dialog in the dialogs array
 * @param nPeriod           Sentence number to be duplicated.
 * @returns		Pointer to the duplicated phrase. Remember to free it
 * when done with it.
 */
static char *duplicateDialogPeriod(uint32 nPeriod) {
	const char *origmsg;
	char *clonemsg;
	LpMpalDialog dialog = GLOBALS._lpmdDialogs + GLOBALS._nExecutingDialog;

	for (int j = 0; dialog->_periods[j] != NULL; j++) {
		if (dialog->_periodNums[j] == nPeriod) {
			// Found the phrase, it should be duplicated
			origmsg = (const char *)globalLock(dialog->_periods[j]);

			// Calculate the length and allocate memory
			int i = 0;
			while (origmsg[i] != '\0')
				i++;

			clonemsg = (char *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, i + 1);
			if (clonemsg == NULL)
				return NULL;

			memcpy(clonemsg, origmsg, i);

			globalUnlock(dialog->_periods[j]);

			return clonemsg;
		}
	}

	return NULL;
}

/**
 * Load a resource from the MPR file
 *
 * @param dwId				ID of the resource to load
 * @returns		Handle to the loaded resource
 */
MpalHandle resLoad(uint32 dwId) {
	MpalHandle h;
	char head[4];
	byte *temp, *buf;

	for (int i = 0; i < GLOBALS._nResources; i++)
		if (GLOBALS._lpResources[i * 2] == dwId) {
			GLOBALS._hMpr.seek(GLOBALS._lpResources[i * 2 + 1]);
			uint32 nBytesRead = GLOBALS._hMpr.read(head, 4);
			if (nBytesRead != 4)
				return NULL;
			if (head[0] != 'R' || head[1] != 'E' || head[2] != 'S' || head[3] != 'D')
				return NULL;

			uint32 nSizeDecomp = GLOBALS._hMpr.readUint32LE();
			if (GLOBALS._hMpr.err())
				return NULL;

			uint32 nSizeComp = GLOBALS._hMpr.readUint32LE();
			if (GLOBALS._hMpr.err())
				return NULL;

			h = globalAllocate(GMEM_MOVEABLE | GMEM_ZEROINIT, nSizeDecomp + (nSizeDecomp / 1024) * 16);
			buf = (byte *)globalLock(h);
			temp = (byte *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, nSizeComp);

			nBytesRead = GLOBALS._hMpr.read(temp, nSizeComp);
			if (nBytesRead != nSizeComp)
				return NULL;

			lzo1x_decompress(temp, nSizeComp, buf, &nBytesRead);
			if (nBytesRead != nSizeDecomp)
				return NULL;

			globalDestroy(temp);
			globalUnlock(h);
			return h;
		}

	return NULL;
}

static uint32 *getSelectList(uint32 i) {
	uint32 *sl;
	LpMpalDialog dialog = GLOBALS._lpmdDialogs + GLOBALS._nExecutingDialog;

	// Count how many are active selects
	int num = 0;
	for (int j = 0; dialog->_choice[i]._select[j]._dwData != 0; j++) {
		if (dialog->_choice[i]._select[j]._curActive)
			num++;
	}

	// If there are 0, it's a mistake
	if (num == 0)
		return NULL;

	sl = (uint32 *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(uint32) * (num + 1));
	if (sl == NULL)
		return NULL;

	// Copy all the data inside the active select list
	int k = 0;
	for (int j = 0; dialog->_choice[i]._select[j]._dwData != 0; j++) {
		if (dialog->_choice[i]._select[j]._curActive)
			sl[k++] = dialog->_choice[i]._select[j]._dwData;
	}

	sl[k] = (uint32)NULL;
	return sl;
}

static uint32 *GetItemList(uint32 nLoc) {
	uint32 *il;
	LpMpalVar v = GLOBALS._lpmvVars;

	uint32 num = 0;
	for (uint32 i = 0; i < GLOBALS._nVars; i++, v++) {
		if (strncmp(v->_lpszVarName, "Location", 8) == 0 && v->_dwVal == nLoc)
			num++;
	}

	il = (uint32 *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(uint32) * (num + 1));
	if (il == NULL)
		return NULL;

	v = GLOBALS._lpmvVars;
	uint32 j = 0;
	for (uint32 i = 0; i < GLOBALS._nVars; i++, v++) {
		if (strncmp(v->_lpszVarName, "Location", 8) == 0 && v->_dwVal == nLoc) {
			sscanf(v->_lpszVarName, "Location.%u", &il[j]);
			j++;
		}
	}

	il[j] = (uint32)NULL;
	return il;
}

static LpItem getItemData(uint32 nOrdItem) {
	LpMpalItem curitem = GLOBALS._lpmiItems + nOrdItem;
	char *dat;
	char *patlength;

	// Zeroing out the allocated memory is required!!!
	LpItem ret = (LpItem)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(Item));
	if (ret == NULL)
		return NULL;
	ret->_speed = 150;

	MpalHandle hDat = resLoad(curitem->_dwRes);
	dat = (char *)globalLock(hDat);

	if (dat[0] == 'D' && dat[1] == 'A' && dat[2] == 'T') {
		int i = dat[3]; // For version 1.0!!
		dat += 4;

		if (i >= 0x10) { // From 1.0, there's a destination point for each object
			ret->_destX = (int16)READ_LE_UINT16(dat);
			ret->_destY = (int16)READ_LE_UINT16(dat + 2);
			dat += 4;
		}

		if (i >= 0x11) { // From 1.1, there's animation speed
			ret->_speed = READ_LE_UINT16(dat);
			dat += 2;
		} else
			ret->_speed = 150;
	}

	ret->_numframe = *dat++;
	ret->_numpattern = *dat++;
	ret->_destZ = *dat++;

	// Upload the left & top co-ordinates of each frame
	for (int i = 0; i < ret->_numframe; i++) {
		ret->_frameslocations[i].left = (int16)READ_LE_UINT16(dat);
		ret->_frameslocations[i].top = (int16)READ_LE_UINT16(dat + 2);
		dat += 4;
	}

	// Upload the size of each frame and calculate the right & bottom
	for (int i = 0; i < ret->_numframe; i++) {
		ret->_frameslocations[i].right = (int16)READ_LE_UINT16(dat) + ret->_frameslocations[i].left;
		ret->_frameslocations[i].bottom = (int16)READ_LE_UINT16(dat + 2) + ret->_frameslocations[i].top;
		dat += 4;
	}

	// Upload the bounding boxes of each frame
	for (int i = 0; i < ret->_numframe; i++) {
		ret->_bbox[i].left = (int16)READ_LE_UINT16(dat);
		ret->_bbox[i].top = (int16)READ_LE_UINT16(dat + 2);
		ret->_bbox[i].right = (int16)READ_LE_UINT16(dat + 4);
		ret->_bbox[i].bottom = (int16)READ_LE_UINT16(dat + 6);
		dat += 8;
	}

	// Load the animation pattern
	patlength = dat;
	dat += ret->_numpattern;

	for (int i = 1; i < ret->_numpattern; i++) {
		for (int j = 0; j < patlength[i]; j++)
			ret->_pattern[i][j] = dat[j];
		ret->_pattern[i][(int)patlength[i]] = 255; // Terminate pattern
		dat += patlength[i];
	}

	// Upload the individual frames of animations
	for (int i = 1; i < ret->_numframe; i++) {
		uint32 dim = (uint32)(ret->_frameslocations[i].right - ret->_frameslocations[i].left) *
			(uint32)(ret->_frameslocations[i].bottom - ret->_frameslocations[i].top);
		ret->_frames[i] = (char *)globalAlloc(GMEM_FIXED, dim);

		if (ret->_frames[i] == NULL)
			return NULL;
		memcpy(ret->_frames[i], dat, dim);
		dat += dim;
	}

	// Check if we've got to the end of the file
	int i = READ_LE_UINT16(dat);
	if (i != 0xABCD)
		return NULL;

	globalUnlock(hDat);
	globalFree(hDat);

	return ret;
}

/**
 * Thread that calls a custom function. It is used in scripts, so that each script
 * function is executed without delaying the others.
 *
 * @param param				pointer to a pointer to the structure that defines the call.
 * @remarks		The passed structure is freed when the process finishes.
 */
void CustomThread(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		LpCfCall p;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->p = *(const LpCfCall *)param;

	CORO_INVOKE_4(GLOBALS._lplpFunctions[_ctx->p->_nCf], _ctx->p->_arg1, _ctx->p->_arg2, _ctx->p->_arg3, _ctx->p->_arg4);

	globalFree(_ctx->p);

	CORO_END_CODE;
}

/**
 * Main process for running a script.
 *
 * @param param				Pointer to a pointer to a structure containing the script data.
 * @remarks		The passed structure is freed when the process finishes.
 */
void ScriptThread(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		uint i, j, k;
		uint32 dwStartTime;
		uint32 dwCurTime;
		uint32 dwId;
		int numHandles;
		LpCfCall p;
	CORO_END_CONTEXT(_ctx);

	static uint32 cfHandles[MAX_COMMANDS_PER_MOMENT];
	LpMpalScript s = *(const LpMpalScript *)param;

	CORO_BEGIN_CODE(_ctx);

	_ctx->dwStartTime = g_vm->getTime();
	_ctx->numHandles = 0;

	//debugC(DEBUG_BASIC, kTonyDebugMPAL, "PlayScript(): Moments: %u\n", s->_nMoments);
	for (_ctx->i = 0; _ctx->i < s->_nMoments; _ctx->i++) {
		// Sleep for the required time
		if (s->_moment[_ctx->i]._dwTime == -1) {
			CORO_INVOKE_4(CoroScheduler.waitForMultipleObjects, _ctx->numHandles, cfHandles, true, CORO_INFINITE);
			_ctx->dwStartTime = g_vm->getTime();
		} else {
			_ctx->dwCurTime = g_vm->getTime();
			if (_ctx->dwCurTime < _ctx->dwStartTime + (s->_moment[_ctx->i]._dwTime * 100)) {
				//debugC(DEBUG_BASIC, kTonyDebugMPAL, "PlayScript(): Sleeping %lums\n",_ctx->dwStartTime + (s->_moment[_ctx->i]._dwTime*100) - _ctx->dwCurTime);
				CORO_INVOKE_1(CoroScheduler.sleep, _ctx->dwStartTime + (s->_moment[_ctx->i]._dwTime * 100) - _ctx->dwCurTime);
			}
		}

		_ctx->numHandles = 0;
		for (_ctx->j = 0; _ctx->j < s->_moment[_ctx->i]._nCmds; _ctx->j++) {
			_ctx->k = s->_moment[_ctx->i]._cmdNum[_ctx->j];

			if (s->_command[_ctx->k]._type == 1) {
				_ctx->p = (LpCfCall)globalAlloc(GMEM_FIXED, sizeof(CfCall));
				if (_ctx->p == NULL) {
					GLOBALS._mpalError = 1;

					CORO_KILL_SELF();
					return;
				}

				_ctx->p->_nCf  = s->_command[_ctx->k]._nCf;
				_ctx->p->_arg1 = s->_command[_ctx->k]._arg1;
				_ctx->p->_arg2 = s->_command[_ctx->k]._arg2;
				_ctx->p->_arg3 = s->_command[_ctx->k]._arg3;
				_ctx->p->_arg4 = s->_command[_ctx->k]._arg4;

				// !!! New process management
				if ((cfHandles[_ctx->numHandles++] = CoroScheduler.createProcess(CustomThread, &_ctx->p, sizeof(LpCfCall))) == 0) {
					GLOBALS._mpalError = 1;

					CORO_KILL_SELF();
					return;
				}
			} else if (s->_command[_ctx->k]._type == 2) {
				lockVar();
				varSetValue(
					s->_command[_ctx->k]._lpszVarName,
					evaluateExpression(s->_command[_ctx->k]._expr)
				);
				unlockVar();

			} else {
				GLOBALS._mpalError = 1;
				globalFree(s);

				CORO_KILL_SELF();
				return;
			}

			// WORKAROUND: Wait for events to pulse.
			CORO_SLEEP(1);
		}
	}

	globalFree(s);

	CORO_KILL_SELF();

	CORO_END_CODE;
}

/**
 * Thread that performs an action on an item. the thread always executes the action,
 * so it should create a new item in which the action is the one required.
 * Furthermore, the expression is not checked, but it is always performed the action.
 *
 * @param param				Pointer to a pointer to a structure containing the action.
 */
void ActionThread(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		int j, k;
		LpMpalItem item;

		~CoroContextTag() {
			if (item)
				globalDestroy(item);
		}
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// The ActionThread owns the data block pointed to, so we need to make sure it's
	// freed when the process exits
	_ctx->item = *(const LpMpalItem *)param;

	GLOBALS._mpalError = 0;
	for (_ctx->j = 0; _ctx->j < _ctx->item->_action[_ctx->item->_dwRes]._nCmds; _ctx->j++) {
		_ctx->k = _ctx->item->_action[_ctx->item->_dwRes]._cmdNum[_ctx->j];

		if (_ctx->item->_command[_ctx->k]._type == 1) {
			// Custom function
			debugC(DEBUG_DETAILED, kTonyDebugActions, "Action Process %d Call=%s params=%d,%d,%d,%d",
				CoroScheduler.getCurrentPID(), GLOBALS._lplpFunctionStrings[_ctx->item->_command[_ctx->k]._nCf].c_str(),
				_ctx->item->_command[_ctx->k]._arg1, _ctx->item->_command[_ctx->k]._arg2,
				_ctx->item->_command[_ctx->k]._arg3, _ctx->item->_command[_ctx->k]._arg4
			);

			CORO_INVOKE_4(GLOBALS._lplpFunctions[_ctx->item->_command[_ctx->k]._nCf],
				_ctx->item->_command[_ctx->k]._arg1,
				_ctx->item->_command[_ctx->k]._arg2,
				_ctx->item->_command[_ctx->k]._arg3,
				_ctx->item->_command[_ctx->k]._arg4

			);
		} else if (_ctx->item->_command[_ctx->k]._type == 2) {
			// Variable assign
			debugC(DEBUG_DETAILED, kTonyDebugActions, "Action Process %d Variable=%s",
				CoroScheduler.getCurrentPID(), _ctx->item->_command[_ctx->k]._lpszVarName);

			lockVar();
			varSetValue(_ctx->item->_command[_ctx->k]._lpszVarName, evaluateExpression(_ctx->item->_command[_ctx->k]._expr));
			unlockVar();

		} else {
			GLOBALS._mpalError = 1;
			break;
		}

		// WORKAROUND: Wait for events to pulse.
		CORO_SLEEP(1);
	}

	globalDestroy(_ctx->item);
	_ctx->item = NULL;

	debugC(DEBUG_DETAILED, kTonyDebugActions, "Action Process %d ended", CoroScheduler.getCurrentPID());

	CORO_END_CODE;
}

/**
 * This thread monitors a created action to detect when it ends.
 * @remarks		Since actions can spawn sub-actions, this needs to be a
 * separate thread to determine when the outer action is done
 */
void ShutUpActionThread(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		int slotNumber;
	CORO_END_CONTEXT(_ctx);

	uint32 pid = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, pid, CORO_INFINITE);

	GLOBALS._bExecutingAction = false;

	if (g_vm->_initialLoadSlotNumber != -1) {
		_ctx->slotNumber = g_vm->_initialLoadSlotNumber;
		g_vm->_initialLoadSlotNumber = -1;

		CORO_INVOKE_1(g_vm->loadState, _ctx->slotNumber);
	}

	CORO_END_CODE;
}

/**
 * Polls one location (starting point of a process)
 *
 * @param param				Pointer to an index in the array of polling locations.
 */
void LocationPollThread(CORO_PARAM, const void *param) {
	typedef struct {
		uint32 _nItem, _nAction;

		uint16 _wTime;
		byte _perc;
		MpalHandle _when;
		byte _nCmds;
		uint16 _cmdNum[MAX_COMMANDS_PER_ACTION];
		uint32 _dwLastTime;
	} MYACTION;

	typedef struct {
		uint32 _nItem;
		uint32 _hThread;
	} MYTHREAD;

	CORO_BEGIN_CONTEXT;
		uint32 *il;
		int i, j, k;
		int numitems;
		int nRealItems;
		LpMpalItem curItem, newItem;
		int nIdleActions;
		uint32 curTime;
		uint32 dwSleepTime;
		uint32 dwId;
		int ord;
		bool delayExpired;
		bool expired;

		MYACTION *myActions;
		MYTHREAD *myThreads;

		~CoroContextTag() {
			// Free data blocks
			if (myThreads)
				globalDestroy(myThreads);
			if (myActions)
				globalDestroy(myActions);
		}
	CORO_END_CONTEXT(_ctx);

	uint32 id = *((const uint32 *)param);

	CORO_BEGIN_CODE(_ctx);

	// Initialize data pointers
	_ctx->myActions = NULL;
	_ctx->myThreads = NULL;

	// To begin with, we need to request the item list from the location
	_ctx->il = mpalQueryItemList(GLOBALS._nPollingLocations[id]);

	// Count the items
	for (_ctx->numitems = 0; _ctx->il[_ctx->numitems] != 0; _ctx->numitems++)
		;

	// We look for items without idle actions, and eliminate them from the list
	lockItems();
	_ctx->nIdleActions = 0;
	_ctx->nRealItems = 0;
	for (_ctx->i = 0; _ctx->i < _ctx->numitems; _ctx->i++) {
		_ctx->ord = itemGetOrderFromNum(_ctx->il[_ctx->i]);

		if (_ctx->ord == -1)
			continue;

		_ctx->curItem = GLOBALS._lpmiItems + _ctx->ord;

		_ctx->k = 0;
		for (_ctx->j = 0; _ctx->j < _ctx->curItem->_nActions; _ctx->j++) {
			if (_ctx->curItem->_action[_ctx->j]._num == 0xFF)
				_ctx->k++;
		}

		_ctx->nIdleActions += _ctx->k;

		if (_ctx->k == 0)
			// We can remove this item from the list
			_ctx->il[_ctx->i] = (uint32)NULL;
		else
			_ctx->nRealItems++;
	}
	unlockItems();

	// If there is nothing left, we can exit
	if (_ctx->nRealItems == 0) {
		globalDestroy(_ctx->il);
		CORO_KILL_SELF();
		return;
	}

	_ctx->myThreads = (MYTHREAD *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, _ctx->nRealItems * sizeof(MYTHREAD));
	if (_ctx->myThreads == NULL) {
		globalDestroy(_ctx->il);
		CORO_KILL_SELF();
		return;
	}

	// We have established that there is at least one item that contains idle actions.
	// Now we created the mirrored copies of the idle actions.
	_ctx->myActions = (MYACTION *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, _ctx->nIdleActions * sizeof(MYACTION));
	if (_ctx->myActions == NULL) {
		globalDestroy(_ctx->myThreads);
		globalDestroy(_ctx->il);
		CORO_KILL_SELF();
		return;
	}

	lockItems();
	_ctx->k = 0;

	for (_ctx->i = 0; _ctx->i < _ctx->numitems; _ctx->i++) {
		if (_ctx->il[_ctx->i] == 0)
			continue;

		_ctx->curItem = GLOBALS._lpmiItems + itemGetOrderFromNum(_ctx->il[_ctx->i]);

		for (_ctx->j = 0; _ctx->j < _ctx->curItem->_nActions; _ctx->j++) {
			if (_ctx->curItem->_action[_ctx->j]._num == 0xFF) {
				_ctx->myActions[_ctx->k]._nItem = _ctx->il[_ctx->i];
				_ctx->myActions[_ctx->k]._nAction = _ctx->j;

				_ctx->myActions[_ctx->k]._wTime = _ctx->curItem->_action[_ctx->j]._wTime;
				_ctx->myActions[_ctx->k]._perc = _ctx->curItem->_action[_ctx->j]._perc;
				_ctx->myActions[_ctx->k]._when = _ctx->curItem->_action[_ctx->j]._when;
				_ctx->myActions[_ctx->k]._nCmds = _ctx->curItem->_action[_ctx->j]._nCmds;
				memcpy(_ctx->myActions[_ctx->k]._cmdNum, _ctx->curItem->_action[_ctx->j]._cmdNum,
				MAX_COMMANDS_PER_ACTION * sizeof(uint16));

				_ctx->myActions[_ctx->k]._dwLastTime = g_vm->getTime();
				_ctx->k++;
			}
		}
	}

	unlockItems();

	// We don't need the item list anymore
	globalDestroy(_ctx->il);

	// Here's the main loop
	while (1) {
		// Searching for idle actions requiring time to execute
		_ctx->curTime = g_vm->getTime();
		_ctx->dwSleepTime = (uint32)-1L;

		for (_ctx->k = 0;_ctx->k<_ctx->nIdleActions;_ctx->k++) {
			if (_ctx->curTime >= _ctx->myActions[_ctx->k]._dwLastTime + _ctx->myActions[_ctx->k]._wTime) {
				_ctx->dwSleepTime = 0;
				break;
			} else
				_ctx->dwSleepTime = MIN(_ctx->dwSleepTime, _ctx->myActions[_ctx->k]._dwLastTime + _ctx->myActions[_ctx->k]._wTime - _ctx->curTime);
		}

		// We fall alseep, but always checking that the event is set when prompted for closure
		CORO_INVOKE_3(CoroScheduler.waitForSingleObject, GLOBALS._hEndPollingLocations[id], _ctx->dwSleepTime, &_ctx->expired);

		//if (_ctx->k == WAIT_OBJECT_0)
		if (!_ctx->expired)
			break;

		for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++) {
			if (_ctx->myThreads[_ctx->i]._nItem != 0) {
				CORO_INVOKE_3(CoroScheduler.waitForSingleObject, _ctx->myThreads[_ctx->i]._hThread, 0, &_ctx->delayExpired);

				// if result == WAIT_OBJECT_0)
				if (!_ctx->delayExpired)
					_ctx->myThreads[_ctx->i]._nItem = 0;
			}
		}

		_ctx->curTime = g_vm->getTime();

		// Loop through all the necessary idle actions
		for (_ctx->k = 0; _ctx->k < _ctx->nIdleActions; _ctx->k++) {
			if (_ctx->curTime >= _ctx->myActions[_ctx->k]._dwLastTime + _ctx->myActions[_ctx->k]._wTime) {
				_ctx->myActions[_ctx->k]._dwLastTime += _ctx->myActions[_ctx->k]._wTime;

				// It's time to check to see if fortune is on the side of the idle action
				byte randomVal = (byte)g_vm->_randomSource.getRandomNumber(99);
				if (randomVal < _ctx->myActions[_ctx->k]._perc) {
					// Check if there is an action running on the item
					if ((GLOBALS._bExecutingAction) && (GLOBALS._nExecutingAction == _ctx->myActions[_ctx->k]._nItem))
						continue;

					// Check to see if there already another idle funning running on the item
					for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++) {
						if (_ctx->myThreads[_ctx->i]._nItem == _ctx->myActions[_ctx->k]._nItem)
							break;
					}

					if (_ctx->i < _ctx->nRealItems)
						continue;

					// Ok, we are the only ones :)
					lockItems();
					_ctx->curItem = GLOBALS._lpmiItems + itemGetOrderFromNum(_ctx->myActions[_ctx->k]._nItem);

					// Check if there is a WhenExecute expression
					_ctx->j=_ctx->myActions[_ctx->k]._nAction;
					if (_ctx->curItem->_action[_ctx->j]._when != NULL) {
						if (!evaluateExpression(_ctx->curItem->_action[_ctx->j]._when)) {
							unlockItems();
							continue;
						}
					}

					// Ok, we can perform the action. For convenience, we do it in a new process
					_ctx->newItem = (LpMpalItem)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(MpalItem));
					if (_ctx->newItem == false) {
						globalDestroy(_ctx->myThreads);
						globalDestroy(_ctx->myActions);

						CORO_KILL_SELF();
						return;
					}

					memcpy(_ctx->newItem,_ctx->curItem, sizeof(MpalItem));
					unlockItems();

					// We copy the action in #0
					//_ctx->newItem->Action[0].nCmds = _ctx->curItem->Action[_ctx->j].nCmds;
					//memcpy(_ctx->newItem->Action[0].CmdNum,_ctx->curItem->Action[_ctx->j].CmdNum,_ctx->newItem->Action[0].nCmds*sizeof(_ctx->newItem->Action[0].CmdNum[0]));
					_ctx->newItem->_dwRes = _ctx->j;

					// We will create an action, and will provide the necessary details
					for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++) {
						if (_ctx->myThreads[_ctx->i]._nItem == 0)
							break;
					}

					_ctx->myThreads[_ctx->i]._nItem = _ctx->myActions[_ctx->k]._nItem;

					// Create the process
					if ((_ctx->myThreads[_ctx->i]._hThread = CoroScheduler.createProcess(ActionThread, &_ctx->newItem, sizeof(LpMpalItem))) == CORO_INVALID_PID_VALUE) {
					//if ((_ctx->myThreads[_ctx->i]._hThread = (void*)_beginthread(ActionThread, 10240, (void *)_ctx->newItem)) == (void*)-1)
						globalDestroy(_ctx->newItem);
						globalDestroy(_ctx->myThreads);
						globalDestroy(_ctx->myActions);

						CORO_KILL_SELF();
						return;
					}

					// Skip all idle actions of the same item
				}
			}
		}
	}

	// Set idle skip on
	CORO_INVOKE_4(GLOBALS._lplpFunctions[200], 0, 0, 0, 0);

	for (_ctx->i = 0; _ctx->i < _ctx->nRealItems; _ctx->i++) {
		if (_ctx->myThreads[_ctx->i]._nItem != 0) {
			CORO_INVOKE_3(CoroScheduler.waitForSingleObject, _ctx->myThreads[_ctx->i]._hThread, 5000, &_ctx->delayExpired);

			//if (result != WAIT_OBJECT_0)
			//if (_ctx->delayExpired)
			//	TerminateThread(_ctx->MyThreads[_ctx->i].hThread, 0);

			CoroScheduler.killMatchingProcess(_ctx->myThreads[_ctx->i]._hThread);
		}
	}

	// Set idle skip off
	CORO_INVOKE_4(GLOBALS._lplpFunctions[201], 0, 0, 0, 0);

	CORO_END_CODE;
}

/**
 * Wait for the end of the dialog execution thread, and then restore global
 * variables indicating that the dialogue has finished.
 *
 * @param param				Pointer to a handle to the dialog
 * @remarks		This additional process is used, instead of clearing variables
 * within the same dialog thread, because due to the recursive nature of a dialog,
 * it would be difficult to know within it when the dialog is actually ending.
 */
void ShutUpDialogThread(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	uint32 pid = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, pid, CORO_INFINITE);

	GLOBALS._bExecutingDialog = false;
	GLOBALS._nExecutingDialog = 0;
	GLOBALS._nExecutingChoice = 0;

	CoroScheduler.setEvent(GLOBALS._hAskChoice);

	CORO_KILL_SELF();

	CORO_END_CODE;
}

void doChoice(CORO_PARAM, uint32 nChoice);

/**
 * Executes a group of the current dialog. Can 'be the Starting point of a process.
 * @parm nGroup				Number of the group to perform
 */
void GroupThread(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		LpMpalDialog dialog;
		int i, j, k;
		int type;
	CORO_END_CONTEXT(_ctx);

	uint32 nGroup = *(const uint32 *)param;

	CORO_BEGIN_CODE(_ctx);

	// Lock the _ctx->dialog
	lockDialogs();

	// Find the pointer to the current _ctx->dialog
	_ctx->dialog = GLOBALS._lpmdDialogs + GLOBALS._nExecutingDialog;

	// Search inside the group requesting the _ctx->dialog
	for (_ctx->i = 0; _ctx->dialog->_group[_ctx->i]._num != 0; _ctx->i++) {
		if (_ctx->dialog->_group[_ctx->i]._num == nGroup) {
			// Cycle through executing the commands of the group
			for (_ctx->j = 0; _ctx->j < _ctx->dialog->_group[_ctx->i]._nCmds; _ctx->j++) {
				_ctx->k = _ctx->dialog->_group[_ctx->i]._cmdNum[_ctx->j];

				_ctx->type = _ctx->dialog->_command[_ctx->k]._type;
				if (_ctx->type == 1) {
					// Call custom function
					CORO_INVOKE_4(GLOBALS._lplpFunctions[_ctx->dialog->_command[_ctx->k]._nCf],
						_ctx->dialog->_command[_ctx->k]._arg1,
						_ctx->dialog->_command[_ctx->k]._arg2,
						_ctx->dialog->_command[_ctx->k]._arg3,
						_ctx->dialog->_command[_ctx->k]._arg4
					);

				} else if (_ctx->type == 2) {
					// Set a variable
					lockVar();
					varSetValue(_ctx->dialog->_command[_ctx->k]._lpszVarName, evaluateExpression(_ctx->dialog->_command[_ctx->k]._expr));
					unlockVar();

				} else if (_ctx->type == 3) {
					// DoChoice: call the chosen function
					CORO_INVOKE_1(doChoice, (uint32)_ctx->dialog->_command[_ctx->k]._nChoice);

				} else {
					GLOBALS._mpalError = 1;
					unlockDialogs();

					CORO_KILL_SELF();
					return;
				}

				// WORKAROUND: Wait for events to pulse.
				CORO_SLEEP(1);
			}

			// The gruop is finished, so we can return to the calling function.
			// If the group was the first called, then the process will automatically
			// end. Otherwise it returns to the caller method

			return;
		}
	}

	// If we are here, it means that we have not found the requested group
	GLOBALS._mpalError = 1;
	unlockDialogs();

	CORO_KILL_SELF();

	CORO_END_CODE;
}

/**
 * Make a choice in the current dialog.
 *
 * @param nChoice			Number of choice to perform
 */
void doChoice(CORO_PARAM, uint32 nChoice) {
	CORO_BEGIN_CONTEXT;
		LpMpalDialog dialog;
		int i, j, k;
		uint32 nGroup;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Lock the dialogs
	lockDialogs();

	// Get a pointer to the current dialog
	_ctx->dialog = GLOBALS._lpmdDialogs + GLOBALS._nExecutingDialog;

	// Search the choice between those required in the dialog
	for (_ctx->i = 0; _ctx->dialog->_choice[_ctx->i]._nChoice != 0; _ctx->i++) {
		if (_ctx->dialog->_choice[_ctx->i]._nChoice == nChoice)
			break;
	}

	// If nothing has been found, exit with an error
	if (_ctx->dialog->_choice[_ctx->i]._nChoice == 0) {
		// If we're here, we did not find the required choice
		GLOBALS._mpalError = 1;
		unlockDialogs();

		CORO_KILL_SELF();
		return;
	}

	// We've found the requested choice. Remember what in global variables
	GLOBALS._nExecutingChoice = _ctx->i;

	while (1) {
		GLOBALS._nExecutingChoice = _ctx->i;

		_ctx->k = 0;
		// Calculate the expression of each selection, to see if they're active or inactive
		for (_ctx->j = 0; _ctx->dialog->_choice[_ctx->i]._select[_ctx->j]._dwData != 0; _ctx->j++) {
			if (_ctx->dialog->_choice[_ctx->i]._select[_ctx->j]._when == NULL) {
				_ctx->dialog->_choice[_ctx->i]._select[_ctx->j]._curActive = 1;
				_ctx->k++;
			} else if (evaluateExpression(_ctx->dialog->_choice[_ctx->i]._select[_ctx->j]._when)) {
				_ctx->dialog->_choice[_ctx->i]._select[_ctx->j]._curActive = 1;
				_ctx->k++;
			} else
				_ctx->dialog->_choice[_ctx->i]._select[_ctx->j]._curActive = 0;
		}

		// If there are no choices activated, then the dialog is finished.
		if (_ctx->k == 0) {
			unlockDialogs();
			break;
		}

		// There are choices available to the user, so wait for them to make one
		CoroScheduler.resetEvent(GLOBALS._hDoneChoice);
		CoroScheduler.setEvent(GLOBALS._hAskChoice);
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, GLOBALS._hDoneChoice, CORO_INFINITE);

		// Now that the choice has been made, we can run the groups associated with the choice tbontbtitq
		_ctx->j = GLOBALS._nSelectedChoice;
		for (_ctx->k = 0; _ctx->dialog->_choice[_ctx->i]._select[_ctx->j]._wPlayGroup[_ctx->k] != 0; _ctx->k++) {
			_ctx->nGroup = _ctx->dialog->_choice[_ctx->i]._select[_ctx->j]._wPlayGroup[_ctx->k];
			CORO_INVOKE_1(GroupThread, &_ctx->nGroup);
		}

		// Control attribute
		if (_ctx->dialog->_choice[_ctx->i]._select[_ctx->j]._attr & (1 << 0)) {
			// Bit 0 set: the end of the choice
			unlockDialogs();
			break;
		}

		if (_ctx->dialog->_choice[_ctx->i]._select[_ctx->j]._attr & (1 << 1)) {
			// Bit 1 set: the end of the dialog
			unlockDialogs();

			CORO_KILL_SELF();
			return;
		}

		// End of choic ewithout attributes. We must do it again
	}

	// If we're here, we found an end choice. Return to the caller group
	return;

	CORO_END_CODE;
}

/**
 * Perform an action on a certain item.
 *
 * @param nAction			Action number
 * @param ordItem           Index of the item in the items list
 * @param dwParam			Any parameter for the action.
 * @returns		Id of the process that was launched to perform the action, or
 * CORO_INVALID_PID_VALUE if the action was not defined, or the item was inactive.
 * @remarks		You can get the index of an item from its number by using
 * the itemGetOrderFromNum() function. The items list must first be locked
 * by calling LockItem().
 */
static uint32 doAction(uint32 nAction, uint32 ordItem, uint32 dwParam) {
	LpMpalItem item = GLOBALS._lpmiItems;
	LpMpalItem newitem;

	item+=ordItem;
	Common::String buf = Common::String::format("Status.%u", item->_nObj);
	if (varGetValue(buf.c_str()) <= 0)
		return CORO_INVALID_PID_VALUE;

	for (int i = 0; i < item->_nActions; i++) {
		if (item->_action[i]._num != nAction)
			continue;

		if (item->_action[i]._wParm != dwParam)
			continue;

		if (item->_action[i]._when != NULL) {
			if (!evaluateExpression(item->_action[i]._when))
				continue;
		}

		// Now we find the right action to be performed
		// Duplicate the item and copy the current action in #i into #0
		newitem = (LpMpalItem)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(MpalItem));
		if (newitem == NULL)
			return CORO_INVALID_PID_VALUE;

		// In the new version number of the action in writing dwRes
		Common::copy((byte *)item, (byte *)item + sizeof(MpalItem), (byte *)newitem);

		//newitem->_action[0]._nCmds=item->_action[i]._nCmds;
		//memcpy(newitem->_action[0]._cmdNum, item->_action[i]._cmdNum, newitem->Action[0].nCmds * sizeof(newitem->_action[0]._cmdNum[0]));

		newitem->_dwRes = i;

		// And finally we can laucnh the process that will execute the action,
		// and a second process to free up the memory when the action is finished.

		// !!! New process management
		uint32 h;
		if ((h = CoroScheduler.createProcess(ActionThread, &newitem, sizeof(LpMpalItem))) == CORO_INVALID_PID_VALUE)
			return CORO_INVALID_PID_VALUE;

		if (CoroScheduler.createProcess(ShutUpActionThread, &h, sizeof(uint32)) == CORO_INVALID_PID_VALUE)
			return CORO_INVALID_PID_VALUE;

		GLOBALS._nExecutingAction = item->_nObj;
		GLOBALS._bExecutingAction = true;

		return h;
	}

	return CORO_INVALID_PID_VALUE;
}

/**
 * Shows a dialog in a separate process.
 *
 * @param nDlgOrd				The index of the dialog in the dialog list
 * @param nGroup				Number of the group to perform
 * @returns						The process Id of the process running the dialog
 *								or CORO_INVALID_PID_VALUE on error
 * @remarks						The dialogue runs in a thread created on purpose,
 * so that must inform through an event and when 'necessary to you make a choice.
 * The data on the choices may be obtained through various queries.
 */
static uint32 doDialog(uint32 nDlgOrd, uint32 nGroup) {
	// Store the running dialog in a global variable
	GLOBALS._nExecutingDialog = nDlgOrd;

	// Enables the flag to indicate that there is' a running dialogue
	GLOBALS._bExecutingDialog = true;

	CoroScheduler.resetEvent(GLOBALS._hAskChoice);
	CoroScheduler.resetEvent(GLOBALS._hDoneChoice);

	// Create a thread that performs the dialogue group

	// Create the process
	uint32 h;
	if ((h = CoroScheduler.createProcess(GroupThread, &nGroup, sizeof(uint32))) == CORO_INVALID_PID_VALUE)
		return CORO_INVALID_PID_VALUE;

	// Create a thread that waits until the end of the dialog process, and will restore the global variables
	if (CoroScheduler.createProcess(ShutUpDialogThread, &h, sizeof(uint32)) == CORO_INVALID_PID_VALUE) {
		// Something went wrong, so kill the previously started dialog process
		CoroScheduler.killMatchingProcess(h);
		return CORO_INVALID_PID_VALUE;
	}

	return h;
}

/**
 * Takes note of the selection chosen by the user, and warns the process that was running
 * the box that it can continue.
 *
 * @param nChoice           Number of choice that was in progress
 * @param dwData			Since combined with select selection
 * @returns		True if everything is OK, false on failure
 */
bool doSelection(uint32 i, uint32 dwData) {
	LpMpalDialog dialog = GLOBALS._lpmdDialogs + GLOBALS._nExecutingDialog;
	int j;

	for (j = 0; dialog->_choice[i]._select[j]._dwData != 0; j++) {
		if (dialog->_choice[i]._select[j]._dwData == dwData && dialog->_choice[i]._select[j]._curActive != 0)
			break;
	}

	if (dialog->_choice[i]._select[j]._dwData == 0)
		return false;

	GLOBALS._nSelectedChoice = j;
	CoroScheduler.setEvent(GLOBALS._hDoneChoice);
	return true;
}

/**
 * @defgroup Exported functions
 */
//@{

/**
 * Initializes the MPAL library and opens the .MPC file, which will be used for all queries.
 *
 * @param lpszMpcFileName   Name of the MPC file
 * @param lpszMprFileName   Name of the MPR file
 * @param lplpcfArray		Array of pointers to custom functions.
 * @returns		True if everything is OK, false on failure
 */
bool mpalInit(const char *lpszMpcFileName, const char *lpszMprFileName,
			  LPLPCUSTOMFUNCTION lplpcfArray, Common::String *lpcfStrings) {
	byte buf[5];
	byte *cmpbuf;

	// Save the array of custom functions
	GLOBALS._lplpFunctions = lplpcfArray;
	GLOBALS._lplpFunctionStrings = lpcfStrings;

	// OPen the MPC file for reading
	Common::File hMpc;
	if (!hMpc.open(lpszMpcFileName))
		return false;

	// Read and check the header
	uint32 nBytesRead = hMpc.read(buf, 5);
	if (nBytesRead != 5)
		return false;

	if (buf[0] != 'M' || buf[1] != 'P' || buf[2] != 'C' || buf[3] != 0x20)
		return false;

	bool bCompress = buf[4];

	// Reads the size of the uncompressed file, and allocate memory
	uint32 dwSizeDecomp = hMpc.readUint32LE();
	if (hMpc.err())
		return false;

	byte *lpMpcImage = (byte *)globalAlloc(GMEM_FIXED, dwSizeDecomp + 16);
	if (lpMpcImage == NULL)
		return false;

	if (bCompress) {
		// Get the compressed size and read the data in
		uint32 dwSizeComp = hMpc.readUint32LE();
		if (hMpc.err())
			return false;

		cmpbuf = (byte *)globalAlloc(GMEM_FIXED, dwSizeComp);
		if (cmpbuf == NULL)
			return false;

		nBytesRead = hMpc.read(cmpbuf, dwSizeComp);
		if (nBytesRead != dwSizeComp)
			return false;

		// Decompress the data
		lzo1x_decompress(cmpbuf, dwSizeComp, lpMpcImage, &nBytesRead);
		if (nBytesRead != dwSizeDecomp)
			return false;

		globalDestroy(cmpbuf);
	} else {
		// If the file is not compressed, we directly read in the data
		nBytesRead = hMpc.read(lpMpcImage, dwSizeDecomp);
		if (nBytesRead != dwSizeDecomp)
			return false;
	}

	// Close the file
	hMpc.close();

	// Process the data
	if (parseMpc(lpMpcImage) == false)
		return false;

	globalDestroy(lpMpcImage);

	// Open the MPR file
	if (!GLOBALS._hMpr.open(lpszMprFileName))
		return false;

	// Seek to the end of the file to read overall information
	GLOBALS._hMpr.seek(-12, SEEK_END);

	uint32 dwSizeComp = GLOBALS._hMpr.readUint32LE();
	if (GLOBALS._hMpr.err())
		return false;

	GLOBALS._nResources = GLOBALS._hMpr.readUint32LE();
	if (GLOBALS._hMpr.err())
		return false;

	nBytesRead = GLOBALS._hMpr.read(buf, 4);
	if (GLOBALS._hMpr.err())
		return false;

	if (buf[0] !='E' || buf[1] != 'N' || buf[2] != 'D' || buf[3] != '0')
		return false;

	// Move to the start of the resources header
	GLOBALS._hMpr.seek(-(12 + (int)dwSizeComp), SEEK_END);

	GLOBALS._lpResources = (uint32 *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, GLOBALS._nResources * 8);
	if (GLOBALS._lpResources == NULL)
		return false;

	cmpbuf = (byte *)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, dwSizeComp);
	if (cmpbuf == NULL)
		return false;

	nBytesRead = GLOBALS._hMpr.read(cmpbuf, dwSizeComp);
	if (nBytesRead != dwSizeComp)
		return false;

	lzo1x_decompress((const byte *)cmpbuf, dwSizeComp, (byte *)GLOBALS._lpResources, (uint32 *)&nBytesRead);
	if (nBytesRead != (uint32)GLOBALS._nResources * 8)
		return false;
	for (int i = 0; i < 2*GLOBALS._nResources; ++i)
		GLOBALS._lpResources[i] = FROM_LE_32(GLOBALS._lpResources[i]);

	globalDestroy(cmpbuf);

	// Reset back to the start of the file, leaving it open
	GLOBALS._hMpr.seek(0, SEEK_SET);

	// There is no action or dialog running by default
	GLOBALS._bExecutingAction = false;
	GLOBALS._bExecutingDialog = false;

	// There's no polling location
	Common::fill(GLOBALS._nPollingLocations, GLOBALS._nPollingLocations + MAXPOLLINGLOCATIONS, 0);

	// Create the event that will be used to co-ordinate making choices and choices finishing
	GLOBALS._hAskChoice = CoroScheduler.createEvent(true, false);
	GLOBALS._hDoneChoice = CoroScheduler.createEvent(true, false);

	return true;
}

/**
 * Frees resources allocated by the MPAL subsystem
 */
void mpalFree() {
	// Free the resource list
	globalDestroy(GLOBALS._lpResources);
}

/**
 * This is a general function to communicate with the library, to request information
 * about what is in the .MPC file
 *
 * @param wQueryType		Type of query. The list is in the QueryTypes enum.
 * @returns		4 bytes depending on the type of query
 * @remarks		This is the specialised version of the original single mpalQuery
 * method that returns numeric results.
 */
uint32 mpalQueryDWORD(uint16 wQueryType, ...) {
	Common::String buf;
	uint32 dwRet = 0;
	char *n;

	va_list v;
	va_start(v, wQueryType);

	GLOBALS._mpalError = OK;

	if (wQueryType == MPQ_VERSION) {

		/*
		 *  uint32 mpalQuery(MPQ_VERSION);
		 */
		dwRet = HEX_VERSION;

	} else if (wQueryType == MPQ_GLOBAL_VAR) {
		/*
		 *  uint32 mpalQuery(MPQ_GLOBAL_VAR, char * lpszVarName);
		 */
		lockVar();
		dwRet = (uint32)varGetValue(GETARG(char *));
		unlockVar();

	} else if (wQueryType == MPQ_MESSAGE) {
		/*
		 *  char * mpalQuery(MPQ_MESSAGE, uint32 nMsg);
		 */
		error("mpalQuery(MPQ_MESSAGE, uint32 nMsg) used incorrect method variant");


	} else if (wQueryType == MPQ_ITEM_PATTERN) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_PATTERN, uint32 nItem);
		 */
		lockVar();
		buf = Common::String::format("Pattern.%u", GETARG(uint32));
		dwRet = (uint32)varGetValue(buf.c_str());
		unlockVar();

	} else if (wQueryType == MPQ_LOCATION_SIZE) {
		/*
		 *  uint32 mpalQuery(MPQ_LOCATION_SIZE, uint32 nLoc, uint32 dwCoord);
		 */
		lockLocations();
		int x = locGetOrderFromNum(GETARG(uint32));
		int y = GETARG(uint32);
		if (x != -1) {
			if (y == MPQ_X)
				dwRet = GLOBALS._lpmlLocations[x]._dwXlen;
			else if (y == MPQ_Y)
				dwRet = GLOBALS._lpmlLocations[x]._dwYlen;
			else
				GLOBALS._mpalError = 1;
		} else
			GLOBALS._mpalError = 1;

		unlockLocations();

	} else if (wQueryType == MPQ_LOCATION_IMAGE) {
		/*
		 *  HGLOBAL mpalQuery(MPQ_LOCATION_IMAGE, uint32 nLoc);
		 */
		error("mpalQuery(MPQ_LOCATION_IMAGE, uint32 nLoc) used incorrect variant");

	} else if (wQueryType == MPQ_RESOURCE) {
		/*
		 *  HGLOBAL mpalQuery(MPQ_RESOURCE, uint32 dwRes);
		 */
		error("mpalQuery(MPQ_RESOURCE, uint32 dwRes) used incorrect variant");

	} else if (wQueryType == MPQ_ITEM_LIST) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_LIST, uint32 nLoc);
		 */
		error("mpalQuery(MPQ_ITEM_LIST, uint32 nLoc) used incorrect variant");

	} else if (wQueryType == MPQ_ITEM_DATA) {
		/*
		 *  LpItem mpalQuery(MPQ_ITEM_DATA, uint32 nItem);
		 */
		error("mpalQuery(MPQ_ITEM_DATA, uint32 nItem) used incorrect variant");

	} else if (wQueryType == MPQ_ITEM_IS_ACTIVE) {
		/*
		 *  bool mpalQuery(MPQ_ITEM_IS_ACTIVE, uint32 nItem);
		 */
		lockVar();
		int x = GETARG(uint32);
		buf = Common::String::format("Status.%u", x);
		if (varGetValue(buf.c_str()) <= 0)
			dwRet = (uint32)false;
		else
			dwRet = (uint32)true;

		unlockVar();

	} else if (wQueryType == MPQ_ITEM_NAME) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_NAME, uint32 nItem, char * lpszName);
		 */
		lockVar();
		int x = GETARG(uint32);
		n = GETARG(char *);
		buf = Common::String::format("Status.%u", x);
		if (varGetValue(buf.c_str()) <= 0)
			n[0]='\0';
		else {
			lockItems();
			int y = itemGetOrderFromNum(x);
			memcpy(n, (char *)(GLOBALS._lpmiItems + y)->_lpszDescribe, MAX_DESCRIBE_SIZE);
			unlockItems();
		}

		unlockVar();

	} else if (wQueryType == MPQ_DIALOG_PERIOD) {
		/*
		 *  char *mpalQuery(MPQ_DIALOG_PERIOD, uint32 nDialog, uint32 nPeriod);
		 */
		error("mpalQuery(MPQ_DIALOG_PERIOD, uint32 nDialog, uint32 nPeriod) used incorrect variant");

	} else if (wQueryType == MPQ_DIALOG_WAITFORCHOICE) {
		/*
		 *  void mpalQuery(MPQ_DIALOG_WAITFORCHOICE);
		 */
		error("mpalQuery(MPQ_DIALOG_WAITFORCHOICE) used incorrect variant");

	} else if (wQueryType == MPQ_DIALOG_SELECTLIST) {
		/*
		 *  uint32 *mpalQuery(MPQ_DIALOG_SELECTLIST, uint32 nChoice);
		 */
		error("mpalQuery(MPQ_DIALOG_SELECTLIST, uint32 nChoice) used incorrect variant");

	} else if (wQueryType == MPQ_DIALOG_SELECTION) {
		/*
		 *  bool mpalQuery(MPQ_DIALOG_SELECTION, uint32 nChoice, uint32 dwData);
		 */
		lockDialogs();
		int x = GETARG(uint32);
		int y = GETARG(uint32);
		dwRet = (uint32)doSelection(x, y);

		unlockDialogs();

	} else if (wQueryType == MPQ_DO_ACTION) {
		/*
		 *  int mpalQuery(MPQ_DO_ACTION, uint32 nAction, uint32 nItem, uint32 dwParam);
		 */
		lockItems();
		lockVar();
		int x = GETARG(uint32);
		int z = GETARG(uint32);
		int y = itemGetOrderFromNum(z);
		if (y != -1) {
			dwRet = doAction(x, y, GETARG(uint32));
		} else {
			dwRet = CORO_INVALID_PID_VALUE;
			GLOBALS._mpalError = 1;
		}

		unlockVar();
		unlockItems();

	} else if (wQueryType == MPQ_DO_DIALOG) {
		/*
		 *  int mpalQuery(MPQ_DO_DIALOG, uint32 nDialog, uint32 nGroup);
		 */
		if (!GLOBALS._bExecutingDialog) {
			lockDialogs();

			int x = dialogGetOrderFromNum(GETARG(uint32));
			int y = GETARG(uint32);
			dwRet = doDialog(x, y);
			unlockDialogs();
		}
	} else {
		/*
		 *  DEFAULT -> ERROR
		 */
		GLOBALS._mpalError = 1;
	}

	va_end(v);
	return dwRet;
}

/**
 * This is a general function to communicate with the library, to request information
 * about what is in the .MPC file
 *
 * @param wQueryType		Type of query. The list is in the QueryTypes enum.
 * @returns		4 bytes depending on the type of query
 * @remarks		This is the specialised version of the original single mpalQuery
 * method that returns a pointer or handle.
 */
MpalHandle mpalQueryHANDLE(uint16 wQueryType, ...) {
	char *n;
	Common::String buf;
	va_list v;
	va_start(v, wQueryType);
	void *hRet = NULL;

	GLOBALS._mpalError = OK;

	if (wQueryType == MPQ_VERSION) {
		/*
		 *  uint32 mpalQuery(MPQ_VERSION);
		 */
		error("mpalQuery(MPQ_VERSION) used incorrect variant");

	} else if (wQueryType == MPQ_GLOBAL_VAR) {
		/*
		 *  uint32 mpalQuery(MPQ_GLOBAL_VAR, char * lpszVarName);
		 */
		error("mpalQuery(MPQ_GLOBAL_VAR, char * lpszVarName) used incorrect variant");

	} else if (wQueryType == MPQ_MESSAGE) {
		/*
		 *  char * mpalQuery(MPQ_MESSAGE, uint32 nMsg);
		 */
		LockMsg();
		hRet = DuplicateMessage(msgGetOrderFromNum(GETARG(uint32)));
		UnlockMsg();

	} else if (wQueryType == MPQ_ITEM_PATTERN) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_PATTERN, uint32 nItem);
		 */
		error("mpalQuery(MPQ_ITEM_PATTERN, uint32 nItem) used incorrect variant");

	} else if (wQueryType == MPQ_LOCATION_SIZE) {
		/*
		 *  uint32 mpalQuery(MPQ_LOCATION_SIZE, uint32 nLoc, uint32 dwCoord);
		 */
		error("mpalQuery(MPQ_LOCATION_SIZE, uint32 nLoc, uint32 dwCoord) used incorrect variant");

	} else if (wQueryType == MPQ_LOCATION_IMAGE) {
		/*
		 *  HGLOBAL mpalQuery(MPQ_LOCATION_IMAGE, uint32 nLoc);
		 */
		lockLocations();
		int x = locGetOrderFromNum(GETARG(uint32));
		hRet = resLoad(GLOBALS._lpmlLocations[x]._dwPicRes);
		unlockLocations();

	} else if (wQueryType == MPQ_RESOURCE) {
		/*
		 *  HGLOBAL mpalQuery(MPQ_RESOURCE, uint32 dwRes);
		 */
		hRet = resLoad(GETARG(uint32));

	} else if (wQueryType == MPQ_ITEM_LIST) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_LIST, uint32 nLoc);
		 */
		lockVar();
		hRet = GetItemList(GETARG(uint32));
		lockVar();

	} else if (wQueryType == MPQ_ITEM_DATA) {
		/*
		 *  LpItem mpalQuery(MPQ_ITEM_DATA, uint32 nItem);
		 */
		lockItems();
		hRet = getItemData(itemGetOrderFromNum(GETARG(uint32)));
		unlockItems();

	} else if (wQueryType == MPQ_ITEM_IS_ACTIVE) {
		/*
		 *  bool mpalQuery(MPQ_ITEM_IS_ACTIVE, uint32 nItem);
		 */
		error("mpalQuery(MPQ_ITEM_IS_ACTIVE, uint32 nItem) used incorrect variant");

	} else if (wQueryType == MPQ_ITEM_NAME) {
		/*
		 *  uint32 mpalQuery(MPQ_ITEM_NAME, uint32 nItem, char *lpszName);
		 */
		lockVar();
		int x = GETARG(uint32);
		n = GETARG(char *);
		buf = Common::String::format("Status.%u", x);
		if (varGetValue(buf.c_str()) <= 0)
			n[0] = '\0';
		else {
			lockItems();
			int y = itemGetOrderFromNum(x);
			memcpy(n, (char *)(GLOBALS._lpmiItems + y)->_lpszDescribe, MAX_DESCRIBE_SIZE);
			unlockItems();
		}

		unlockVar();

	} else if (wQueryType == MPQ_DIALOG_PERIOD) {
		/*
		 *  char * mpalQuery(MPQ_DIALOG_PERIOD, uint32 nDialog, uint32 nPeriod);
		 */
		lockDialogs();
		int y = GETARG(uint32);
		hRet = duplicateDialogPeriod(y);
		unlockDialogs();

	} else if (wQueryType == MPQ_DIALOG_WAITFORCHOICE) {
		/*
		 *  void mpalQuery(MPQ_DIALOG_WAITFORCHOICE);
		 */
		error("mpalQuery(MPQ_DIALOG_WAITFORCHOICE) used incorrect variant");

	} else if (wQueryType == MPQ_DIALOG_SELECTLIST) {
		/*
		 *  uint32 *mpalQuery(MPQ_DIALOG_SELECTLIST, uint32 nChoice);
		 */
		lockDialogs();
		hRet = getSelectList(GETARG(uint32));
		unlockDialogs();

	} else if (wQueryType == MPQ_DIALOG_SELECTION) {
		/*
		 *  bool mpalQuery(MPQ_DIALOG_SELECTION, uint32 nChoice, uint32 dwData);
		 */
		error("mpalQuery(MPQ_DIALOG_SELECTION, uint32 nChoice, uint32 dwData) used incorrect variant");

	} else if (wQueryType == MPQ_DO_ACTION) {
		/*
		 *  int mpalQuery(MPQ_DO_ACTION, uint32 nAction, uint32 nItem, uint32 dwParam);
		 */
		error("mpalQuery(MPQ_DO_ACTION, uint32 nAction, uint32 nItem, uint32 dwParam) used incorrect variant");

	} else if (wQueryType == MPQ_DO_DIALOG) {
		/*
		 *  int mpalQuery(MPQ_DO_DIALOG, uint32 nDialog, uint32 nGroup);
		 */
		error("mpalQuery(MPQ_DO_DIALOG, uint32 nDialog, uint32 nGroup) used incorrect variant");
	} else {
		/*
		 *  DEFAULT -> ERROR
		 */
		GLOBALS._mpalError = 1;
	}

	va_end(v);
	return hRet;
}

/**
 * This is a general function to communicate with the library, to request information
 * about what is in the .MPC file
 *
 * @param wQueryType		Type of query. The list is in the QueryTypes enum.
 * @returns		4 bytes depending on the type of query
 * @remarks		This is the specialised version of the original single mpalQuery
 * method that needs to run within a co-routine context.
 */
void mpalQueryCORO(CORO_PARAM, uint16 wQueryType, uint32 *dwRet, ...) {
	CORO_BEGIN_CONTEXT;
		uint32 dwRet;
	CORO_END_CONTEXT(_ctx);

	va_list v;
	va_start(v, dwRet);

	CORO_BEGIN_CODE(_ctx);

	if (wQueryType == MPQ_DIALOG_WAITFORCHOICE) {
		/*
		 *  void mpalQuery(MPQ_DIALOG_WAITFORCHOICE);
		 */
		CORO_INVOKE_2(CoroScheduler.waitForSingleObject, GLOBALS._hAskChoice, CORO_INFINITE);

		// WORKAROUND: Introduce a single frame delay so that if there are multiple actions running,
		// they all have time to be signalled before resetting the event. This fixes a problem where
		// if you try to use the 'shrimp' on the parrot a second time after trying to first use it
		// whilst the parrot was talking, the cursor wouldn't be re-enabled afterwards
		CORO_SLEEP(1);

		CoroScheduler.resetEvent(GLOBALS._hAskChoice);

		if (GLOBALS._bExecutingDialog)
			*dwRet = (uint32)GLOBALS._nExecutingChoice;
		else
			*dwRet = (uint32)((int)-1);
	} else {
		error("mpalQueryCORO called with unsupported query type");
	}

	CORO_END_CODE;

	va_end(v);
}

/**
 * Returns the current MPAL error code
 *
 * @returns		Error code
 */
uint32 mpalGetError() {
	return GLOBALS._mpalError;
}

/**
 * Execute a script. The script runs on multitasking by a thread.
 *
 * @param nScript			Script number to run
 * @returns		TRUE if the script 'was launched, FALSE on failure
 */
bool mpalExecuteScript(int nScript) {
	LockScripts();
	int n = scriptGetOrderFromNum(nScript);
	LpMpalScript s = (LpMpalScript)globalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(MpalScript));
	if (s == NULL)
		return false;

	memcpy(s, GLOBALS._lpmsScripts + n, sizeof(MpalScript));
	unlockScripts();

	// !!! New process management
	if (CoroScheduler.createProcess(ScriptThread, &s, sizeof(LpMpalScript)) == CORO_INVALID_PID_VALUE)
 		return false;

	return true;
}

/**
 * Install a custom routine That will be called by MPAL every time the pattern
 * of an item has been changed.
 *
 * @param lpiifCustom		Custom function to install
 */
void mpalInstallItemIrq(LPITEMIRQFUNCTION lpiifCus) {
	GLOBALS._lpiifCustom = lpiifCus;
}

/**
 * Process the idle actions of the items on one location.
 *
 * @param nLoc				Number of the location whose items must be processed
 * for idle actions.
 * @returns		TRUE if all OK, and FALSE if it exceeded the maximum limit.
 * @remarks		The maximum number of locations that can be polled
 * simultaneously is defined defined by MAXPOLLINGFUNCIONS
 */
bool mpalStartIdlePoll(int nLoc) {
	for (uint32 i = 0; i < MAXPOLLINGLOCATIONS; i++) {
		if (GLOBALS._nPollingLocations[i] == (uint32)nLoc)
			return false;
	}

	for (uint32 i = 0; i < MAXPOLLINGLOCATIONS; i++) {
		if (GLOBALS._nPollingLocations[i] == 0) {
			GLOBALS._nPollingLocations[i] = nLoc;

			GLOBALS._hEndPollingLocations[i] = CoroScheduler.createEvent(true, false);
// !!! New process management
			if ((GLOBALS._pollingThreads[i] = CoroScheduler.createProcess(LocationPollThread, &i, sizeof(uint32))) == CORO_INVALID_PID_VALUE)
//			 if ((GLOBALS.hEndPollingLocations[i] = (void*)_beginthread(LocationPollThread, 10240, (void *)i))= = (void*)-1)
				return false;

			return true;
		}
	}

	return false;
}

/**
 * Stop processing the idle actions of the items on one location.
 *
 * @param nLo				Number of the location
 * @returns		TRUE if all OK, FALSE if the specified location was not
 * in the process of polling
 */
void mpalEndIdlePoll(CORO_PARAM, int nLoc, bool *result) {
	CORO_BEGIN_CONTEXT;
		int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	for (_ctx->i = 0; _ctx->i < MAXPOLLINGLOCATIONS; _ctx->i++) {
		if (GLOBALS._nPollingLocations[_ctx->i] == (uint32)nLoc) {
			CoroScheduler.setEvent(GLOBALS._hEndPollingLocations[_ctx->i]);

			CORO_INVOKE_2(CoroScheduler.waitForSingleObject, GLOBALS._pollingThreads[_ctx->i], CORO_INFINITE);

			CoroScheduler.closeEvent(GLOBALS._hEndPollingLocations[_ctx->i]);
			GLOBALS._nPollingLocations[_ctx->i] = 0;

			if (result)
				*result = true;
			return;
		}
	}

	if (result)
		*result = false;

	CORO_END_CODE;
}

/**
 * Retrieve the length of a save state
 *
 * @returns		Length in bytes
 */
int mpalGetSaveStateSize() {
	return GLOBALS._nVars * sizeof(MpalVar) + 4;
}

/**
 * Store the save state into a buffer. The buffer must be
 * length at least the size specified with mpalGetSaveStateSize
 *
 * @param buf				Buffer where to store the state
 */
void mpalSaveState(byte *buf) {
	lockVar();
	WRITE_LE_UINT32(buf, GLOBALS._nVars);
	memcpy(buf + 4, (byte *)GLOBALS._lpmvVars, GLOBALS._nVars * sizeof(MpalVar));
	unlockVar();
}

/**
 * Load a save state from a buffer.
 *
 * @param buf				Buffer where to store the state
 * @returns		Length of the state buffer in bytes
 */
int mpalLoadState(byte *buf) {
	// We must destroy and recreate all the variables
	globalFree(GLOBALS._hVars);

	GLOBALS._nVars = READ_LE_UINT32(buf);

	GLOBALS._hVars = globalAllocate(GMEM_ZEROINIT | GMEM_MOVEABLE, GLOBALS._nVars * sizeof(MpalVar));
	lockVar();
	memcpy((byte *)GLOBALS._lpmvVars, buf + 4, GLOBALS._nVars * sizeof(MpalVar));
	unlockVar();

	return GLOBALS._nVars * sizeof(MpalVar) + 4;
}

} // end of namespace MPAL

} // end of namespace Tony
