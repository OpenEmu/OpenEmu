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
 * Process scheduler.
 */

#include "tinsel/handle.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/sched.h"

#include "common/textconsole.h"
#include "common/util.h"

namespace Tinsel {

#include "common/pack-start.h"	// START STRUCT PACKING

struct PROCESS_STRUC {
	uint32 processId;		// ID of process
	SCNHANDLE hProcessCode;	// handle to actor script
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

static uint32 g_numSceneProcess;
static SCNHANDLE g_hSceneProcess;

static uint32 g_numGlobalProcess;
static PROCESS_STRUC *g_pGlobalProcess;


/**************************************************************************\
|***********    Stuff to do with scene and global processes    ************|
\**************************************************************************/

/**
 * The code for for restored scene processes.
 */
static void RestoredProcessProcess(CORO_PARAM, const void *param) {
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// get the stuff copied to process when it was created
	_ctx->pic = *(const PINT_CONTEXT *)param;

	_ctx->pic = RestoreInterpretContext(_ctx->pic);
	AttachInterpret(_ctx->pic, CoroScheduler.getCurrentProcess());

	CORO_INVOKE_1(Interpret, _ctx->pic);

	CORO_END_CODE;
}

/**
 * Process Tinsel Process
 */
static void ProcessTinselProcess(CORO_PARAM, const void *param) {
	const PINT_CONTEXT *pPic = (const PINT_CONTEXT *)param;

	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// get the stuff copied to process when it was created
	CORO_INVOKE_1(Interpret, *pPic);

	CORO_KILL_SELF();
	CORO_END_CODE;
}


/**************************************************************************\
|*****************    Stuff to do with scene processes    *****************|
\**************************************************************************/

/**
 * Called to restore a scene process.
 */
void RestoreSceneProcess(INT_CONTEXT *pic) {
	uint32 i;
	PROCESS_STRUC	*pStruc;

	pStruc = (PROCESS_STRUC *)LockMem(g_hSceneProcess);
	for (i = 0; i < g_numSceneProcess; i++) {
		if (FROM_LE_32(pStruc[i].hProcessCode) == pic->hCode) {
			CoroScheduler.createProcess(PID_PROCESS + i, RestoredProcessProcess,
					 &pic, sizeof(pic));
			break;
		}
	}

	assert(i < g_numSceneProcess);
}

/**
 * Run a scene process with the given event.
 */
void SceneProcessEvent(CORO_PARAM, uint32 procID, TINSEL_EVENT event, bool bWait, int myEscape,
						bool *result) {
	uint32 i;		// Loop counter
	if (result) *result = false;

	CORO_BEGIN_CONTEXT;
		PROCESS_STRUC *pStruc;
		Common::PPROCESS pProc;
		PINT_CONTEXT pic;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->pStruc = (PROCESS_STRUC *)LockMem(g_hSceneProcess);
	for (i = 0; i < g_numSceneProcess; i++) {
		if (FROM_LE_32(_ctx->pStruc[i].processId) == procID) {
			assert(_ctx->pStruc[i].hProcessCode);		// Must have some code to run

			_ctx->pic = InitInterpretContext(GS_PROCESS,
				FROM_LE_32(_ctx->pStruc[i].hProcessCode),
				event,
				NOPOLY,			// No polygon
				0,			// No actor
				NULL,			// No object
				myEscape);
			if (_ctx->pic == NULL)
				return;

			_ctx->pProc = CoroScheduler.createProcess(PID_PROCESS + i, ProcessTinselProcess,
				&_ctx->pic, sizeof(_ctx->pic));
			AttachInterpret(_ctx->pic, _ctx->pProc);
			break;
		}
	}

	if (i == g_numSceneProcess)
		return;

	if (bWait) {
		CORO_INVOKE_2(WaitInterpret, _ctx->pProc, result);
	}

	CORO_END_CODE;
}

/**
 * Kill all instances of a scene process.
 */
void KillSceneProcess(uint32 procID) {
	uint32 i;		// Loop counter
	PROCESS_STRUC	*pStruc;

	pStruc = (PROCESS_STRUC *) LockMem(g_hSceneProcess);
	for (i = 0; i < g_numSceneProcess; i++) {
		if (FROM_LE_32(pStruc[i].processId) == procID) {
			CoroScheduler.killMatchingProcess(PID_PROCESS + i, -1);
			break;
		}
	}
}

/**
 * Register the scene processes in a scene.
 */
void SceneProcesses(uint32 numProcess, SCNHANDLE hProcess) {
	g_numSceneProcess = numProcess;
	g_hSceneProcess = hProcess;
}


/**************************************************************************\
|*****************    Stuff to do with global processes    ****************|
\**************************************************************************/

/**
 * Called to restore a global process.
 */
void RestoreGlobalProcess(INT_CONTEXT *pic) {
	uint32 i;		// Loop counter

	for (i = 0; i < g_numGlobalProcess; i++) {
		if (g_pGlobalProcess[i].hProcessCode == pic->hCode) {
			CoroScheduler.createProcess(PID_GPROCESS + i, RestoredProcessProcess,
					 &pic, sizeof(pic));
			break;
		}
	}

	assert(i < g_numGlobalProcess);
}

/**
 * Kill them all (restore game).
 */
void KillGlobalProcesses() {

	for (uint32 i = 0; i < g_numGlobalProcess; ++i)	{
		CoroScheduler.killMatchingProcess(PID_GPROCESS + i, -1);
	}
}

/**
 * Run a global process with the given event.
 */
bool GlobalProcessEvent(CORO_PARAM, uint32 procID, TINSEL_EVENT event, bool bWait, int myEscape) {
	CORO_BEGIN_CONTEXT;
		PINT_CONTEXT	pic;
		Common::PPROCESS	pProc;
	CORO_END_CONTEXT(_ctx);

	bool result = false;

	CORO_BEGIN_CODE(_ctx);

	uint32	i;		// Loop counter
	_ctx->pProc = NULL;

	for (i = 0; i < g_numGlobalProcess; ++i)	{
		if (g_pGlobalProcess[i].processId == procID) {
			assert(g_pGlobalProcess[i].hProcessCode);		// Must have some code to run

			_ctx->pic = InitInterpretContext(GS_GPROCESS,
				g_pGlobalProcess[i].hProcessCode,
				event,
				NOPOLY,			// No polygon
				0,			// No actor
				NULL,			// No object
				myEscape);

			if (_ctx->pic != NULL) {

				_ctx->pProc = CoroScheduler.createProcess(PID_GPROCESS + i, ProcessTinselProcess,
					&_ctx->pic, sizeof(_ctx->pic));
				AttachInterpret(_ctx->pic, _ctx->pProc);
			}
			break;
		}
	}

	if ((i == g_numGlobalProcess) || (_ctx->pic == NULL))
		result = false;
	else if (bWait)
		CORO_INVOKE_ARGS_V(WaitInterpret, false, (CORO_SUBCTX, _ctx->pProc, &result));

	CORO_END_CODE;
	return result;
}

/**
 * Kill all instances of a global process.
 */
void xKillGlobalProcess(uint32 procID) {
	uint32 i;		// Loop counter

	for (i = 0; i < g_numGlobalProcess; ++i) {
		if (g_pGlobalProcess[i].processId == procID) {
			CoroScheduler.killMatchingProcess(PID_GPROCESS + i, -1);
			break;
		}
	}
}

/**
 * Register the global processes list
 */
void GlobalProcesses(uint32 numProcess, byte *pProcess) {
	g_pGlobalProcess = new PROCESS_STRUC[numProcess];
	g_numGlobalProcess = numProcess;
	byte *p = pProcess;

	for (uint i = 0; i < numProcess; ++i, p += 8) {
		g_pGlobalProcess[i].processId = READ_LE_UINT32(p);
		g_pGlobalProcess[i].hProcessCode = READ_LE_UINT32(p + 4);
	}
}

/**
 * Frees the global processes list
 */
void FreeGlobalProcesses() {
	delete[] g_pGlobalProcess;
	g_pGlobalProcess = 0;
	g_numGlobalProcess = 0;
}

} // End of namespace Tinsel
