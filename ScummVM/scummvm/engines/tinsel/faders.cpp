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
 * Palette Fader and Flasher processes.
 */

#include "tinsel/actors.h"
#include "tinsel/faders.h"	// fader defs
#include "tinsel/handle.h"
#include "tinsel/palette.h"	// Palette Manager defs
#include "tinsel/pid.h"	// list of all process IDs
#include "tinsel/sched.h"	// scheduler defs
#include "tinsel/sysvar.h"
#include "tinsel/tinsel.h"

namespace Tinsel {

/** structure used by the "FadeProcess" process */
struct FADE {
	const long *pColorMultTable;	// list of fixed point color multipliers - terminated with negative entry
	PALQ *pPalQ;		// palette queue entry to fade
};

// fixed point fade multiplier tables
//const long fadeout[] = {0xf000, 0xd000, 0xb000, 0x9000, 0x7000, 0x5000, 0x3000, 0x1000, 0, -1};
//const long fadein[] = {0, 0x1000, 0x3000, 0x5000, 0x7000, 0x9000, 0xb000, 0xd000, 0x10000L, -1};

/**
 * Scale 'color' by the fixed point color multiplier 'colorMult'
 * @param color			Color to scale
 * @param colorMult		Fixed point multiplier
 */
static COLORREF ScaleColor(COLORREF color, uint32 colorMult)	{
	// apply multiplier to RGB components
	uint32 red   = ((TINSEL_GetRValue(color) * colorMult) << 8) >> 24;
	uint32 green = ((TINSEL_GetGValue(color) * colorMult) << 8) >> 24;
	uint32 blue  = ((TINSEL_GetBValue(color) * colorMult) << 8) >> 24;

	// return new color
	return TINSEL_RGB(red, green, blue);
}

/**
 * Applies the fixed point multiplier 'mult' to all colors in
 * 'pOrig' to produce 'pNew'. Each color in the palette will be
 * multiplied by 'mult'.
 * @param pNew				Pointer to new palette
 * @param pOrig				Pointer to original palette
 * @param numColors		Number of colors in the above palettes
 * @param mult				Fixed point multiplier
 */
static void FadePalette(COLORREF *pNew, COLORREF *pOrig, int numColors, uint32 mult) {
	for (int i = 0; i < numColors; i++, pNew++, pOrig++) {
		if (!TinselV2)
			// apply multiplier to RGB components
			*pNew = ScaleColor(*pOrig, mult);
		else if (i == (TalkColor() - 1)) {
			*pNew = GetTalkColorRef();
			*pNew = ScaleColor(*pNew, mult);
		} else if (SysVar(SV_TAGCOLOR) && i == (SysVar(SV_TAGCOLOR) - 1)) {
			*pNew = GetTagColorRef();
			*pNew = ScaleColor(*pNew, mult);
		} else {
			*pNew = ScaleColor(*pOrig, mult);
		}
	}
}

/**
 * Process to fade one palette.
 * A pointer to a 'FADE' structure must be passed to this process when
 * it is created.
 */
static void FadeProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		COLORREF fadeRGB[MAX_COLORS];	// local copy of palette
		const long *pColMult;			// pointer to color multiplier table
		PALETTE *pPalette;		// pointer to palette
	CORO_END_CONTEXT(_ctx);

	// get the fade data structure - copied to process when it was created
	const FADE *pFade = (const FADE *)param;

	CORO_BEGIN_CODE(_ctx);

	if (TinselV2)
		// Note that this palette is being faded
		FadingPalette(pFade->pPalQ, true);

	// get pointer to palette - reduce pointer indirection a bit
	_ctx->pPalette = (PALETTE *)LockMem(pFade->pPalQ->hPal);

	for (_ctx->pColMult = pFade->pColorMultTable; *_ctx->pColMult >= 0; _ctx->pColMult++) {
		// go through all multipliers in table - until a negative entry

		// fade palette using next multiplier
		if (TinselV2)
			FadePalette(_ctx->fadeRGB, pFade->pPalQ->palRGB,
				pFade->pPalQ->numColors, (uint32) *_ctx->pColMult);
		else
			FadePalette(_ctx->fadeRGB, _ctx->pPalette->palRGB,
				FROM_LE_32(_ctx->pPalette->numColors), (uint32) *_ctx->pColMult);

		// send new palette to video DAC
		UpdateDACqueue(pFade->pPalQ->posInDAC, FROM_LE_32(_ctx->pPalette->numColors), _ctx->fadeRGB);

		// allow time for video DAC to be updated
		CORO_SLEEP(1);
	}

	if (TinselV2)
		// Note that this palette is being faded
		FadingPalette(pFade->pPalQ, false);

	CORO_END_CODE;
}

/**
 * Generic palette fader/unfader. Creates a 'FadeProcess' process
 * for each palette that is to fade.
 * @param multTable			Fixed point color multiplier table
 * @param noFadeTable		List of palettes not to fade
 */
static void Fader(const long multTable[], SCNHANDLE noFadeTable[]) {
	PALQ *pPal;	// palette manager iterator

	if (TinselV2) {
		// The is only ever one cuncurrent fade
		// But this could be a fade out and the fade in is still going!
		CoroScheduler.killMatchingProcess(PID_FADER);
		NoFadingPalettes();
	}

	// create a process for each palette in the palette queue
	for (pPal = GetNextPalette(NULL); pPal != NULL; pPal = GetNextPalette(pPal)) {
		bool bFade = true;
			// assume we want to fade this palette

		// is palette in the list of palettes not to fade
		if (noFadeTable != NULL) {
			// there is a list of palettes not to fade
			for (int i = 0; noFadeTable[i] != 0; i++) {
				if (pPal->hPal == noFadeTable[i]) {
					// palette is in the list - dont fade it
					bFade = false;

					// leave loop prematurely
					break;
				}
			}
		}

		if (bFade) {
			FADE fade;

			// fill in FADE struct
			fade.pColorMultTable	= multTable;
			fade.pPalQ		= pPal;

			// create a fader process for this palette
			CoroScheduler.createProcess(PID_FADER, FadeProcess, (void *)&fade, sizeof(FADE));
		}
	}
}

/**
 * Fades a list of palettes down to black.
 * 'noFadeTable' is a NULL terminated list of palettes not to fade.
 */
void FadeOutMedium(SCNHANDLE noFadeTable[]) {
	// Fixed point fade multiplier table
	static const long fadeout[] = {0xea00, 0xd000, 0xb600, 0x9c00,
		0x8200, 0x6800, 0x4e00, 0x3400, 0x1a00, 0, -1};

	// call generic fader
	Fader(fadeout, noFadeTable);
}

/**
 * Fades a list of palettes down to black.
 * @param noFadeTable		A NULL terminated list of palettes not to fade.
 */
void FadeOutFast(SCNHANDLE noFadeTable[]) {
	// Fixed point fade multiplier table
	static const long fadeout[] = {0xd000, 0xa000, 0x7000, 0x4000, 0x1000, 0, -1};

	// call generic fader
	Fader(fadeout, noFadeTable);
}

/**
 * Fades a list of palettes from black to their current colors.
 * 'noFadeTable' is a NULL terminated list of palettes not to fade.
 */
void FadeInMedium(SCNHANDLE noFadeTable[]) {
	// Fade multiplier table
	static const long fadein[] = {0, 0x1a00, 0x3400, 0x4e00, 0x6800,
		0x8200, 0x9c00, 0xb600, 0xd000, 0xea00, 0x10000L, -1};

	// call generic fader
	Fader(fadein, noFadeTable);
}

/**
 * Fades a list of palettes from black to their current colors.
 * @param noFadeTable		A NULL terminated list of palettes not to fade.
 */
void FadeInFast(SCNHANDLE noFadeTable[]) {
	// Fade multiplier table
	static const long fadein[] = {0, 0x1000, 0x4000, 0x7000, 0xa000, 0xd000, 0x10000L, -1};

	// call generic fader
	Fader(fadein, noFadeTable);
}

void PokeInTagColor() {
	if (SysVar(SV_TAGCOLOR)) {
		const COLORREF c = GetActorRGB(-1);
		UpdateDACqueue(SysVar(SV_TAGCOLOR), c);
	}
}

} // End of namespace Tinsel
