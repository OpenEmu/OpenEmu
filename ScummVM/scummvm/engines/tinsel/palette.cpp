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
 * Palette Allocator for IBM PC.
 */

#include "tinsel/dw.h"		// TBLUE1 definition
#include "tinsel/graphics.h"
#include "tinsel/handle.h"	// LockMem definition
#include "tinsel/palette.h"	// palette allocator structures etc.
#include "tinsel/sysvar.h"
#include "tinsel/tinsel.h"

#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/palette.h"

namespace Tinsel {

//----------------- LOCAL DEFINES --------------------

/** video DAC transfer Q structure */
struct VIDEO_DAC_Q {
	union {
		SCNHANDLE hRGBarray;	///< handle of palette or
		COLORREF *pRGBarray;	///< list of palette colors
		COLORREF  singleRGB;
	} pal;
	bool bHandle;		///< when set - use handle of palette
	int destDACindex;	///< start index of palette in video DAC
	int numColors;		///< number of colors in "hRGBarray"
};


//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

/** palette allocator data */
static PALQ g_palAllocData[NUM_PALETTES];


/** video DAC transfer Q length */
#define VDACQLENGTH (NUM_PALETTES+2)

/** video DAC transfer Q */
static VIDEO_DAC_Q g_vidDACdata[VDACQLENGTH];

/** video DAC transfer Q head pointer */
static VIDEO_DAC_Q *g_pDAChead;

/** color index of the 4 colors used for the translucent palette */
#define COL_HILIGHT	TBLUE1

/** the translucent palette lookup table */
uint8 g_transPalette[MAX_COLORS];	// used in graphics.cpp

uint8 g_ghostPalette[MAX_COLORS];

static int g_translucentIndex	= 228;

static int g_talkIndex		= 233;

static COLORREF g_talkColRef;

static COLORREF g_tagColRef;


#ifdef DEBUG
// diagnostic palette counters
static int numPals = 0;
static int maxPals = 0;
static int maxDACQ = 0;
#endif

/**
 * Map PSX palettes to original palette from resource file
 */
void psxPaletteMapper(PALQ *originalPal, uint8 *psxClut, byte *mapperTable) {
	PALETTE *pal = (PALETTE *)LockMem(originalPal->hPal);
	bool colorFound = false;
	uint16 clutEntry = 0;

	// Empty the table with color correspondences
	memset(mapperTable, 0, 16);

	for (int j = 1; j < 16; j++) {
		clutEntry = READ_LE_UINT16(psxClut + (sizeof(uint16) * j));
		if (clutEntry) {
			if (clutEntry == 0x7EC0) { // This is an already known value, used by the in-game text
				mapperTable[j] = 232;
				continue;
			}

			// Check for correspondent color
			for (uint i = 0; (i < FROM_LE_32(pal->numColors)) && !colorFound; i++) {
				// get R G B values in the same way as psx format converters
				uint16 psxEquivalent = TINSEL_PSX_RGB(TINSEL_GetRValue(pal->palRGB[i]) >> 3, TINSEL_GetGValue(pal->palRGB[i]) >> 3, TINSEL_GetBValue(pal->palRGB[i]) >> 3);

				if (psxEquivalent == clutEntry) {
					mapperTable[j] = i + 1; // Add entry in the table for the found color
					colorFound = true;
				}
			}
			colorFound = false;
		} else { // The rest of the entries are zeroes
			return;
		}
	}
}

/**
 * Transfer palettes in the palette Q to Video DAC.
 */
void PalettesToVideoDAC() {
	PALQ *pPalQ;				// palette Q iterator
	VIDEO_DAC_Q *pDACtail = g_vidDACdata;	// set tail pointer
	byte pal[768];

	// while Q is not empty
	while (g_pDAChead != pDACtail) {
		const PALETTE *pPalette;	// pointer to hardware palette
		const COLORREF *pColors;	// pointer to list of RGB triples

#ifdef	DEBUG
		// make sure palette does not overlap
		assert(pDACtail->destDACindex + pDACtail->numColors <= MAX_COLORS);
#else
		// make sure palette does not overlap
		if (pDACtail->destDACindex + pDACtail->numColors > MAX_COLORS)
			pDACtail->numColors = MAX_COLORS - pDACtail->destDACindex;
#endif

		if (pDACtail->bHandle) {
			// we are using a palette handle

			// get hardware palette pointer
			pPalette = (const PALETTE *)LockMem(pDACtail->pal.hRGBarray);

			// get RGB pointer
			pColors = pPalette->palRGB;
		} else if (pDACtail->numColors == 1) {
			// we are using a single color palette
			pColors = &pDACtail->pal.singleRGB;
		} else {
			// we are using a palette pointer
			pColors = pDACtail->pal.pRGBarray;
		}

		for (int i = 0; i < pDACtail->numColors; ++i) {
			pal[i * 3 + 0] = TINSEL_GetRValue(pColors[i]);
			pal[i * 3 + 1] = TINSEL_GetGValue(pColors[i]);
			pal[i * 3 + 2] = TINSEL_GetBValue(pColors[i]);
		}

		// update the system palette
		g_system->getPaletteManager()->setPalette(pal, pDACtail->destDACindex, pDACtail->numColors);

		// update tail pointer
		pDACtail++;

	}

	// reset video DAC transfer Q head pointer
	g_pDAChead = g_vidDACdata;

	// clear all palette moved bits
	for (pPalQ = g_palAllocData; pPalQ < g_palAllocData + NUM_PALETTES; pPalQ++)
		pPalQ->posInDAC &= ~PALETTE_MOVED;
}

/**
 * Commpletely reset the palette allocator.
 */
void ResetPalAllocator() {
#ifdef DEBUG
	// clear number of palettes in use
	numPals = 0;
#endif

	// wipe out the palette allocator data
	memset(g_palAllocData, 0, sizeof(g_palAllocData));

	// reset video DAC transfer Q head pointer
	g_pDAChead = g_vidDACdata;
}

#ifdef	DEBUG
/**
 * Shows the maximum number of palettes used at once.
 */
void PaletteStats() {
	debug("%i palettes of %i used", maxPals, NUM_PALETTES);
	debug("%i DAC queue entries of %i used", maxDACQ, VDACQLENGTH);
}
#endif

/**
 * Places a palette in the video DAC queue.
 * @param posInDAC			Position in video DAC
 * @param numColors		Number of colors in palette
 * @param hPalette			Handle to palette
 */
void UpdateDACqueueHandle(int posInDAC, int numColors, SCNHANDLE hPalette) {
	// check Q overflow
	assert(g_pDAChead < g_vidDACdata + VDACQLENGTH);

	g_pDAChead->destDACindex = posInDAC & ~PALETTE_MOVED;	// set index in video DAC
	g_pDAChead->numColors = numColors;	// set number of colors
	g_pDAChead->pal.hRGBarray = hPalette;	// set handle of palette
	g_pDAChead->bHandle = true;		// we are using a palette handle

	// update head pointer
	++g_pDAChead;

#ifdef DEBUG
	if ((g_pDAChead-g_vidDACdata) > maxDACQ)
		maxDACQ = g_pDAChead-g_vidDACdata;
#endif
}

/**
 * Places a palette in the video DAC queue.
 * @param posInDAC			Position in video DAC
 * @param numColors		Number of colors in palette
 * @param pColors			List of RGB triples
 */
void UpdateDACqueue(int posInDAC, int numColors, COLORREF *pColors) {
	// check Q overflow
	assert(g_pDAChead < g_vidDACdata + NUM_PALETTES);

	g_pDAChead->destDACindex = posInDAC & ~PALETTE_MOVED;	// set index in video DAC
	g_pDAChead->numColors = numColors;	// set number of colors
	if (numColors == 1)
		g_pDAChead->pal.singleRGB = *pColors;	// set single color of which the "palette" consists
	else
		g_pDAChead->pal.pRGBarray = pColors;	// set addr of palette
	g_pDAChead->bHandle = false;		// we are not using a palette handle

	// update head pointer
	++g_pDAChead;

#ifdef DEBUG
	if ((g_pDAChead-g_vidDACdata) > maxDACQ)
		maxDACQ = g_pDAChead-g_vidDACdata;
#endif
}


/**
 * Places a "palette" consisting of a single color in the video DAC queue.
 * @param posInDAC			Position in video DAC
 * @param color				Single RGB triple
 */
void UpdateDACqueue(int posInDAC, COLORREF color) {
	// check Q overflow
	assert(g_pDAChead < g_vidDACdata + NUM_PALETTES);

	g_pDAChead->destDACindex = posInDAC & ~PALETTE_MOVED;	// set index in video DAC
	g_pDAChead->numColors = 1;	// set number of colors
	g_pDAChead->pal.singleRGB = color;	// set single color of which the "palette" consists
	g_pDAChead->bHandle = false;		// we are not using a palette handle

	// update head pointer
	++g_pDAChead;

#ifdef DEBUG
	if ((g_pDAChead-g_vidDACdata) > maxDACQ)
		maxDACQ = g_pDAChead-g_vidDACdata;
#endif
}

/**
 * Allocate a palette.
 * @param hNewPal			Palette to allocate
 */
PALQ *AllocPalette(SCNHANDLE hNewPal) {
	PALQ *pPrev, *p;		// walks palAllocData
	int iDAC;		// color index in video DAC
	PALQ *pNxtPal;		// next PALQ struct in palette allocator
	PALETTE *pNewPal;

	// get pointer to new palette
	pNewPal = (PALETTE *)LockMem(hNewPal);

	// search all structs in palette allocator - see if palette already allocated
	for (p = g_palAllocData; p < g_palAllocData + NUM_PALETTES; p++) {
		if (p->hPal == hNewPal) {
			// found the desired palette in palette allocator
			p->objCount++;	// update number of objects using palette
			return p;	// return palette queue position
		}
	}

	// search all structs in palette allocator - find a free slot
	iDAC = FGND_DAC_INDEX;	// init DAC index to first available foreground color

	for (p = g_palAllocData; p < g_palAllocData + NUM_PALETTES; p++) {
		if (p->hPal == 0) {
			// found a free slot in palette allocator
			p->objCount = 1;	// init number of objects using palette
			p->posInDAC = iDAC;	// set palettes start pos in video DAC
			p->hPal = hNewPal;	// set hardware palette data
			p->numColors = FROM_LE_32(pNewPal->numColors);	// set number of colors in palette

			if (TinselV2)
				// Copy all the colors
				memcpy(p->palRGB, pNewPal->palRGB, p->numColors * sizeof(COLORREF));

#ifdef DEBUG
			// one more palette in use
			if (++numPals > maxPals)
				maxPals = numPals;
#endif

			// Q the change to the video DAC
			if (TinselV2)
				UpdateDACqueue(p->posInDAC, p->numColors, p->palRGB);
			else
				UpdateDACqueueHandle(p->posInDAC, p->numColors, p->hPal);

			// move all palettes after this one down (if necessary)
			for (pPrev = p, pNxtPal = pPrev + 1; pNxtPal < g_palAllocData + NUM_PALETTES; pNxtPal++) {
				if (pNxtPal->hPal != 0) {
					// palette slot is in use
					if (pNxtPal->posInDAC >= pPrev->posInDAC + pPrev->numColors)
						// no need to move palettes down
						break;

					// move palette down - indicate change
					pNxtPal->posInDAC = (pPrev->posInDAC
						+ pPrev->numColors) | PALETTE_MOVED;

					// Q the palette change in position to the video DAC
					if (!TinselV2)
						UpdateDACqueueHandle(pNxtPal->posInDAC,
							pNxtPal->numColors,
							pNxtPal->hPal);
					else if (!pNxtPal->bFading)
						UpdateDACqueue(pNxtPal->posInDAC,
							pNxtPal->numColors,
							pNxtPal->palRGB);

					// update previous palette to current palette
					pPrev = pNxtPal;
				}
			}

			// return palette pointer
			return p;
		}

		// set new DAC index
		iDAC = p->posInDAC + p->numColors;
	}

	// no free palettes
	error("AllocPalette(): formally 'assert(0)!'");
}

/**
 * Free a palette allocated with "AllocPalette".
 * @param pFreePal			Palette queue entry to free
 */
void FreePalette(PALQ *pFreePal) {
	// validate palette Q pointer
	assert(pFreePal >= g_palAllocData && pFreePal <= g_palAllocData + NUM_PALETTES - 1);

	// reduce the palettes object reference count
	pFreePal->objCount--;

	// make sure palette has not been deallocated too many times
	assert(pFreePal->objCount >= 0);

	if (pFreePal->objCount == 0) {
		pFreePal->hPal = 0;	// palette is no longer in use

#ifdef DEBUG
		// one less palette in use
		--numPals;
		assert(numPals >= 0);
#endif
	}
}

/**
 * Find the specified palette.
 * @param hSrchPal			Hardware palette to search for
 */
PALQ *FindPalette(SCNHANDLE hSrchPal) {
	PALQ *pPal;		// palette allocator iterator

	// search all structs in palette allocator
	for (pPal = g_palAllocData; pPal < g_palAllocData + NUM_PALETTES; pPal++) {
		if (pPal->hPal == hSrchPal)
			// found palette in palette allocator
			return pPal;
	}

	// palette not found
	return NULL;
}

/**
 * Swaps the palettes at the specified palette queue position.
 * @param pPalQ			Palette queue position
 * @param hNewPal		New palette
 */
void SwapPalette(PALQ *pPalQ, SCNHANDLE hNewPal) {
	// convert handle to palette pointer
	PALETTE *pNewPal = (PALETTE *)LockMem(hNewPal);

	// validate palette Q pointer
	assert(pPalQ >= g_palAllocData && pPalQ <= g_palAllocData + NUM_PALETTES - 1);

	if (pPalQ->numColors >= (int)FROM_LE_32(pNewPal->numColors)) {
		// new palette will fit the slot

		// install new palette
		pPalQ->hPal = hNewPal;

		if (TinselV2) {
			pPalQ->numColors = FROM_LE_32(pNewPal->numColors);

			// Copy all the colors
			memcpy(pPalQ->palRGB, pNewPal->palRGB, FROM_LE_32(pNewPal->numColors) * sizeof(COLORREF));

			if (!pPalQ->bFading)
				// Q the change to the video DAC
				UpdateDACqueue(pPalQ->posInDAC, FROM_LE_32(pNewPal->numColors), pPalQ->palRGB);
		} else {
			// Q the change to the video DAC
			UpdateDACqueueHandle(pPalQ->posInDAC, FROM_LE_32(pNewPal->numColors), hNewPal);
		}
	} else {
		// # colors are different - will have to update all following palette entries
		assert(!TinselV2); // Fatal error for Tinsel 2

		PALQ *pNxtPalQ;		// next palette queue position

		for (pNxtPalQ = pPalQ + 1; pNxtPalQ < g_palAllocData + NUM_PALETTES; pNxtPalQ++) {
			if (pNxtPalQ->posInDAC >= pPalQ->posInDAC + pPalQ->numColors)
				// no need to move palettes down
				break;

			// move palette down
			pNxtPalQ->posInDAC = (pPalQ->posInDAC
				+ pPalQ->numColors) | PALETTE_MOVED;

			// Q the palette change in position to the video DAC
			UpdateDACqueueHandle(pNxtPalQ->posInDAC,
				pNxtPalQ->numColors,
				pNxtPalQ->hPal);

			// update previous palette to current palette
			pPalQ = pNxtPalQ;
		}
	}
}

/**
 * Statless palette iterator. Returns the next palette in the list
 * @param pStrtPal			Palette to start from - when NULL will start from beginning of list
 */
PALQ *GetNextPalette(PALQ *pStrtPal) {
	if (pStrtPal == NULL) {
		// start of palette iteration - return 1st palette
		return (g_palAllocData[0].objCount) ? g_palAllocData : NULL;
	}

	// validate palette Q pointer
	assert(pStrtPal >= g_palAllocData && pStrtPal <= g_palAllocData + NUM_PALETTES - 1);

	// return next active palette in list
	while (++pStrtPal < g_palAllocData + NUM_PALETTES) {
		if (pStrtPal->objCount)
			// active palette found
			return pStrtPal;
	}

	// non found
	return NULL;
}

/**
 * Sets the current background color.
 * @param color			Color to set the background to
 */
void SetBgndColor(COLORREF color) {
	// update background color struct by queuing the change to the video DAC
	UpdateDACqueue(BGND_DAC_INDEX, color);
}

/**
 * Note whether a palette is being faded.
 * @param pPalQ			Palette queue position
 * @param bFading		Whether it is fading
 */
void FadingPalette(PALQ *pPalQ, bool bFading) {
	// validate palette Q pointer
	assert(pPalQ >= g_palAllocData && pPalQ <= g_palAllocData + NUM_PALETTES - 1);

	// validate that this is a change
	assert(pPalQ->bFading != bFading);

	pPalQ->bFading = bFading;
}

/**
 * All fading processes have just been killed, so none of the
 * palettes are fading.
 */
void NoFadingPalettes() {
	PALQ *pPalQ;

	for (pPalQ = g_palAllocData; pPalQ <= g_palAllocData + NUM_PALETTES - 1; pPalQ++) {
		pPalQ->bFading = false;
	}
}

/**
 * Builds the translucent palette from the current backgrounds palette.
 * @param hPalette			Handle to current background palette
 */
void CreateTranslucentPalette(SCNHANDLE hPalette) {
	// get a pointer to the palette
	PALETTE *pPal = (PALETTE *)LockMem(hPalette);

	// leave background color alone
	g_transPalette[0] = 0;

	for (uint i = 0; i < FROM_LE_32(pPal->numColors); i++) {
		// get the RGB color model values
		uint8 red   = TINSEL_GetRValue(pPal->palRGB[i]);
		uint8 green = TINSEL_GetGValue(pPal->palRGB[i]);
		uint8 blue  = TINSEL_GetBValue(pPal->palRGB[i]);

		// calculate the Value field of the HSV color model
		unsigned val = (red > green) ? red : green;
		val = (val > blue) ? val : blue;

		// map the Value field to one of the 4 colors reserved for the translucent palette
		val /= 63;
		g_transPalette[i + 1] = (uint8)((val == 0) ? 0 : val +
			(TinselV2 ? TranslucentColor() : COL_HILIGHT) - 1);
	}
}

/**
 * Creates the ghost palette
 */
void CreateGhostPalette(SCNHANDLE hPalette) {
	// get a pointer to the palette
	PALETTE *pPal = (PALETTE *)LockMem(hPalette);
	int i;

	// leave background color alone
	g_ghostPalette[0] = 0;

	for (i = 0; i < (int)FROM_LE_32(pPal->numColors); i++) {
		// get the RGB color model values
		uint8 red   = TINSEL_GetRValue(pPal->palRGB[i]);
		uint8 green = TINSEL_GetGValue(pPal->palRGB[i]);
		uint8 blue  = TINSEL_GetBValue(pPal->palRGB[i]);

		// calculate the Value field of the HSV color model
		unsigned val = (red > green) ? red : green;
		val = (val > blue) ? val : blue;

		// map the Value field to one of the 4 colors reserved for the translucent palette
		val /= 64;
		assert(/*val >= 0 &&*/ val <= 3);
		g_ghostPalette[i + 1] = (uint8)(val + SysVar(ISV_GHOST_BASE));
	}
}


/**
 * Returns an adjusted color RGB
 * @param color		Color to scale
 */
static COLORREF DimColor(COLORREF color, int factor) {
	uint32 red, green, blue;

	if (factor == 10) {
		// No change
		return color;
	} else if (factor == 0) {
		// No brightness
		return 0;
	} else {
		// apply multiplier to RGB components
		red   = TINSEL_GetRValue(color) * factor / 10;
		green = TINSEL_GetGValue(color) * factor / 10;
		blue  = TINSEL_GetBValue(color) * factor / 10;

		// return new color
		return TINSEL_RGB(red, green, blue);
	}
}

/**
 * DimPartPalette
 */
void DimPartPalette(SCNHANDLE hDimPal, int startColor, int length, int brightness) {
	PALQ *pPalQ;
	PALETTE *pDimPal;
	int iColor;

	pPalQ = FindPalette(hDimPal);
	assert(pPalQ);

	// get pointer to dim palette
	pDimPal = (PALETTE *)LockMem(hDimPal);

	// Adjust for the fact that palettes don't contain color 0
	startColor -= 1;

	// Check some other things
	if (startColor + length > pPalQ->numColors)
		error("DimPartPalette(): color overrun");

	for (iColor = startColor; iColor < startColor + length; iColor++) {
		pPalQ->palRGB[iColor] = DimColor(pDimPal->palRGB[iColor], brightness);
	}

	if (!pPalQ->bFading) {
		// Q the change to the video DAC
		UpdateDACqueue(pPalQ->posInDAC + startColor, length, &pPalQ->palRGB[startColor]);
	}
}

int TranslucentColor() {
	return g_translucentIndex;
}

int HighlightColor() {
	UpdateDACqueue(g_talkIndex, (COLORREF)SysVar(SYS_HighlightRGB));

	return g_talkIndex;
}

int TalkColor() {
	return TinselV2 ? g_talkIndex : TALKFONT_COL;
}

void SetTalkColorRef(COLORREF colRef) {
	g_talkColRef = colRef;
}

COLORREF GetTalkColorRef() {
	return g_talkColRef;
}

void SetTagColorRef(COLORREF colRef) {
	g_tagColRef = colRef;
}

COLORREF GetTagColorRef() {
	return g_tagColRef;
}

void SetTranslucencyOffset(int offset) {
	g_translucentIndex = offset;
}

void SetTalkTextOffset(int offset) {
	g_talkIndex = offset;
}

} // End of namespace Tinsel
