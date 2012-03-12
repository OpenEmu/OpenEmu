#include "cave.h"

INT32 nCaveXSize = 0, nCaveYSize = 0;
INT32 nCaveXOffset = 0, nCaveYOffset = 0;
INT32 nCaveExtraXOffset = 0, nCaveExtraYOffset = 0;
INT32 nCaveRowModeOffset = 0;

INT32 CaveScanGraphics()
{
	SCAN_VAR(nCaveXOffset);
	SCAN_VAR(nCaveYOffset);

	SCAN_VAR(nCaveTileBank);

	SCAN_VAR(nCaveSpriteBank);
	SCAN_VAR(nCaveSpriteBankDelay);

	for (INT32 i = 0; i < 4; i++) {
		SCAN_VAR(CaveTileReg[i][0]);
		SCAN_VAR(CaveTileReg[i][1]);
		SCAN_VAR(CaveTileReg[i][2]);
	}

	return 0;
}

// This function fills the screen with the background colour
void CaveClearScreen(UINT32 nColour)
{
	if (nColour) {
		UINT32* pClear = (UINT32*)pBurnDraw;
		nColour = nColour | (nColour << 16);
		for (INT32 i = nCaveXSize * nCaveYSize / 16; i > 0 ; i--) {
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
			*pClear++ = nColour;
		}
	} else {
		memset(pBurnDraw, 0, nCaveXSize * nCaveYSize * sizeof(INT16));
	}
}

