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
 * Plays the background film of a scene.
 */

#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/dw.h"
#include "tinsel/faders.h"
#include "tinsel/film.h"
#include "tinsel/font.h"
#include "tinsel/handle.h"
#include "tinsel/multiobj.h"
#include "tinsel/object.h"
#include "tinsel/pcode.h"		// CONTROL_STARTOFF
#include "tinsel/pid.h"
#include "tinsel/sched.h"
#include "tinsel/timers.h"		// For ONE_SECOND constant
#include "tinsel/tinlib.h"		// For Control()
#include "tinsel/tinsel.h"

#include "common/textconsole.h"
#include "common/util.h"

namespace Tinsel {

//----------------- LOCAL GLOBAL DATA --------------------

#define MAX_BG	10

// FIXME: Avoid non-const global vars
static SCNHANDLE g_hBgPal = 0;	// Background's palette
static POBJECT g_pBG[MAX_BG];
static ANIM	g_thisAnim[MAX_BG];	// used by BGmainProcess()
static int g_BGspeed = 0;
static SCNHANDLE g_hBackground = 0;	// Current scene handle - stored in case of Save_Scene()
static bool g_bDoFadeIn = false;
static int g_bgReels;

/**
 * GetBgObject
 */
OBJECT *GetBgObject() {
	return g_pBG[0];
}

/**
 * BackPal
 */
SCNHANDLE BgPal() {
	return g_hBgPal;
}

/**
 * SetDoFadeIn
*/
void SetDoFadeIn(bool tf) {
	g_bDoFadeIn = tf;
}

/**
 * Called before scene change.
 */
void DropBackground() {
	g_pBG[0] = NULL;	// No background

	if (!TinselV2)
		g_hBgPal = 0;	// No background palette
}

/**
 * Return the width of the current background.
 */
int BgWidth() {
	assert(g_pBG[0]);
	return MultiRightmost(g_pBG[0]) + 1;
}

/**
 * Return the height of the current background.
 */
int BgHeight() {
	assert(g_pBG[0]);
	return MultiLowest(g_pBG[0]) + 1;
}

/**
 * Run main animation that comprises the scene background.
 */
static void BGmainProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FILM *pFilm;
	const FREEL *pReel;
	const MULTI_INIT *pmi;

	// get the stuff copied to process when it was created
	if (g_pBG[0] == NULL) {
		/*** At start of scene ***/

		if (!TinselV2) {
			pReel = (const FREEL *)param;

			// Get the MULTI_INIT structure
			pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(pReel->mobj));

			// Initialize and insert the object, and initialize its script.
			g_pBG[0] = MultiInitObject(pmi);
			MultiInsertObject(GetPlayfieldList(FIELD_WORLD), g_pBG[0]);
			InitStepAnimScript(&g_thisAnim[0], g_pBG[0], FROM_LE_32(pReel->script), g_BGspeed);
			g_bgReels = 1;
		} else {
			/*** At start of scene ***/
			pFilm = (const FILM *)LockMem(g_hBackground);
			g_bgReels = FROM_LE_32(pFilm->numreels);

			int i;
			for (i = 0; i < g_bgReels; i++) {
				// Get the MULTI_INIT structure
				pmi = (PMULTI_INIT) LockMem(FROM_LE_32(pFilm->reels[i].mobj));

				// Initialize and insert the object, and initialize its script.
				g_pBG[i] = MultiInitObject(pmi);
				MultiInsertObject(GetPlayfieldList(FIELD_WORLD), g_pBG[i]);
				MultiSetZPosition(g_pBG[i], 0);
				InitStepAnimScript(&g_thisAnim[i], g_pBG[i], FROM_LE_32(pFilm->reels[i].script), g_BGspeed);

				if (i > 0)
					g_pBG[i-1]->pSlave = g_pBG[i];
			}
		}

		if (g_bDoFadeIn) {
			FadeInFast(NULL);
			g_bDoFadeIn = false;
		} else if (TinselV2)
			PokeInTagColor();

		for (;;) {
			for (int i = 0; i < g_bgReels; i++) {
				if (StepAnimScript(&g_thisAnim[i]) == ScriptFinished)
					error("Background animation has finished");
			}

			CORO_SLEEP(1);
		}
	} else {
		// New background during scene
		if (!TinselV2) {
			pReel = (const FREEL *)param;
			InitStepAnimScript(&g_thisAnim[0], g_pBG[0], FROM_LE_32(pReel->script), g_BGspeed);
			StepAnimScript(&g_thisAnim[0]);
		} else {
			pFilm = (const FILM *)LockMem(g_hBackground);
			assert(g_bgReels == (int32)FROM_LE_32(pFilm->numreels));

			// Just re-initialize the scripts.
			for (int i = 0; i < g_bgReels; i++) {
				InitStepAnimScript(&g_thisAnim[i], g_pBG[i], pFilm->reels[i].script, g_BGspeed);
				StepAnimScript(&g_thisAnim[i]);
			}
		}
	}

	CORO_END_CODE;
}

/**
 * Runs secondary reels for a scene background
 */
static void BGotherProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		OBJECT *pObj;
		ANIM anim;
	CORO_END_CONTEXT(_ctx);

	const FREEL *pReel = (const FREEL *)param;
	const MULTI_INIT *pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(pReel->mobj));

	CORO_BEGIN_CODE(_ctx);

	// Initialize and insert the object, and initialize its script.
	_ctx->pObj = MultiInitObject(pmi);
	MultiInsertObject(GetPlayfieldList(FIELD_WORLD), _ctx->pObj);

	InitStepAnimScript(&_ctx->anim, g_pBG[0], FROM_LE_32(pReel->script), g_BGspeed);

	while (StepAnimScript(&_ctx->anim) != ScriptFinished)
		CORO_SLEEP(1);

	CORO_END_CODE;
}

/**
 * AetBgPal()
 */
void SetBackPal(SCNHANDLE hPal) {
	g_hBgPal = hPal;

	FettleFontPal(g_hBgPal);
	CreateTranslucentPalette(g_hBgPal);
}

void ChangePalette(SCNHANDLE hPal) {
	SwapPalette(FindPalette(g_hBgPal), hPal);

	SetBackPal(hPal);
}

/**
 * Given the scene background film, extracts the palette handle for
 * everything else's use, then starts a display process for each reel
 * in the film.
 * @param hFilm			Scene background film
 */
void StartupBackground(CORO_PARAM, SCNHANDLE hFilm) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FILM *pfilm;
	IMAGE *pim;

	g_hBackground = hFilm;		// Save handle in case of Save_Scene()

	pim = GetImageFromFilm(hFilm, 0, NULL, NULL, &pfilm);

	SetBackPal(FROM_LE_32(pim->hImgPal));

	// Extract the film speed
	g_BGspeed = ONE_SECOND / FROM_LE_32(pfilm->frate);

	// Start display process for each reel in the film
	CoroScheduler.createProcess(PID_REEL, BGmainProcess, &pfilm->reels[0], sizeof(FREEL));

	if (TinselV0) {
		for (uint i = 1; i < FROM_LE_32(pfilm->numreels); ++i)
			CoroScheduler.createProcess(PID_REEL, BGotherProcess, &pfilm->reels[i], sizeof(FREEL));
	}

	if (g_pBG[0] == NULL)
		ControlStartOff();

	if (TinselV2 && (coroParam != Common::nullContext))
		CORO_GIVE_WAY;

	CORO_END_CODE;
}

/**
 * Return the current scene handle.
 */
SCNHANDLE GetBgroundHandle() {
	return g_hBackground;
}

} // End of namespace Tinsel
