#include "toaplan.h"

UINT8* RomZ80;
UINT8* RamZ80;

// Used to keep track of emulated CPU cycles
INT32 nCyclesDone[2], nCyclesTotal[2];
INT32 nCyclesSegment;

INT32 nToaCyclesScanline;
INT32 nToaCyclesDisplayStart;
INT32 nToaCyclesVBlankStart;

// Variables for the graphics drawing
bool bRotatedScreen;
UINT8* pBurnBitmap;
INT32 nBurnColumn;
INT32 nBurnRow;

// This function loads interleaved code ROMs.
// All even roms should be first, followed by all odd ROMs.
INT32 ToaLoadCode(UINT8* pROM, INT32 nStart, INT32 nCount)
{
	nCount >>= 1;

	for (INT32 nOdd = 0; nOdd < 2; nOdd++) {
		UINT8* pLoad = pROM + (nOdd ^ 1);			// ^1 for byteswapped

		for (INT32 i = 0; i < nCount; i++) {
			struct BurnRomInfo ri;
			// Load these even/odd bytes
			if (BurnLoadRom(pLoad, nStart + i, 2)) {
				return 1;
			}

			// Increment position by the length of the rom * 2
			ri.nLen = 0;
			BurnDrvGetRomInfo(&ri, nStart + i);
			pLoad += ri.nLen << 1;
		}
		nStart += nCount;
	}
	return 0;
}

// This function decodes the tile data for the GP9001 chip in place.
INT32 ToaLoadGP9001Tiles(UINT8* pDest, INT32 nStart, INT32 nNumFiles, INT32 nROMSize, bool bSwap)	// bSwap = false
{
	UINT8* pTile;
	INT32 nSwap;

	for (INT32 i = 0; i < (nNumFiles >> 1); i++) {
		BurnLoadRom(pDest + (i * 2 * nROMSize / nNumFiles), nStart + i, 2);
		BurnLoadRom(pDest + (i * 2 * nROMSize / nNumFiles) + 1, nStart + (nNumFiles >> 1) + i, 2);
	}

	BurnUpdateProgress(0.0, _T("Decoding graphics..."), 0);

	if (bSwap) {
		nSwap = 2;
	} else {
		nSwap = 0;
	}

	for (pTile = pDest; pTile < (pDest + nROMSize); pTile += 4) {
		UINT8 data[4];
		for (INT32 n = 0; n < 4; n++) {
			INT32 m = 7 - (n << 1);
			UINT8 nPixels = ((pTile[0 ^ nSwap] >> m) & 1) << 0;
			nPixels |= ((pTile[2 ^ nSwap] >> m) & 1) << 1;
			nPixels |= ((pTile[1 ^ nSwap] >> m) & 1) << 2;
			nPixels |= ((pTile[3 ^ nSwap] >> m) & 1) << 3;
			nPixels |= ((pTile[0 ^ nSwap] >> (m - 1)) & 1) << 4;
			nPixels |= ((pTile[2 ^ nSwap] >> (m - 1)) & 1) << 5;
			nPixels |= ((pTile[1 ^ nSwap] >> (m - 1)) & 1) << 6;
			nPixels |= ((pTile[3 ^ nSwap] >> (m - 1)) & 1) << 7;

			data[n] = nPixels;
		}

		for (INT32 n = 0; n < 4; n++) {
			pTile[n] = data[n];
		}
	}
	return 0;
}

// This function fills the screen with the first palette entry
void ToaClearScreen(INT32 PalOffset)
{
	if (*ToaPalette) {
		switch (nBurnBpp) {
			case 4: {
				UINT32* pClear = (UINT32*)pBurnDraw;
				UINT32 nColour = ToaPalette[PalOffset];
				for (INT32 i = 0; i < 320 * 240 / 8; i++) {
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
				}
				break;
			}

			case 3: {
				UINT8* pClear = pBurnDraw;
				UINT32 nColour = ToaPalette[PalOffset];
				UINT8 r = nColour & 0xFF;
				UINT8 g = (nColour >> 8) & 0xFF;
				UINT8 b = (nColour >> 16) & 0xFF;
				r &= 0xFF;
				for (INT32 i = 0; i < 320 * 240; i++) {
					*pClear++ = r;
					*pClear++ = g;
					*pClear++ = b;
				}
				break;
			}

			case 2: {
				UINT32* pClear = (UINT32*)pBurnDraw;
				UINT32 nColour = ToaPalette[PalOffset] | ToaPalette[PalOffset] << 16;
				for (INT32 i = 0; i < 320 * 240 / 16; i++) {
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
					*pClear++ = nColour;
				}
				break;
			}
		}
	} else {
		memset(pBurnDraw, 0, 320 * 240 * nBurnBpp);
	}
}

void ToaZExit()
{
	ZetExit();
}
