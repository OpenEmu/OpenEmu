// Cave hardware tilemaps

#include "cave.h"

struct CaveTile { INT16 x; INT16 y; UINT32 tile; };

static CaveTile* CaveTileQueueMemory[4] = { NULL, };
static CaveTile* CaveTileQueue[4][4];

UINT32 CaveTileReg[4][3];

UINT8* CaveTileROM[4] = { NULL, };
UINT8* CaveTileRAM[4] = { NULL, };

static INT8* CaveTileAttrib[4] = { NULL, };

INT32 nCaveTileBank = 0;

// Used when row-select mode is enabled
static INT32* pRowScroll[4] = { NULL, };
static INT32* pRowSelect[4] = { NULL, };

static INT32 nLayerXOffset[4];
static INT32 nLayerYOffset;
static INT32 nPaletteOffset[4];
static INT32 nPaletteSize[4];

static INT32 CaveTileCount[4];
static INT32 CaveTileMax[4];
static INT32 nTileMask[4];

static UINT8* pTile;
static UINT32* pTileData;
static UINT32* pTilePalette;
static INT32* pTileRowInfo;

typedef void (*RenderTileFunction)();
static RenderTileFunction* RenderTile;

static INT32 nTileXPos, nTileYPos, nRowOffset;

static INT32 nClipX8, nClipX16;
static INT32 nClipY8, nClipY16;

// Include the tile rendering functions
#include "cave_tile_func.h"

static void CaveQueue8x8Layer_Normal(INT32 nLayer)
{
	INT32 x, y;
	INT32 bx, by, cx, cy, mx;
	INT32 nTileColumn, nTileRow;
	UINT32 nTileNumber;

	UINT8* pTileRAM = CaveTileRAM[nLayer];

	UINT32 nTileOffset = 0;
	if (nCaveTileBank) {
		nTileOffset = 0x040000;
	}

	mx = (nCaveXSize >> 3) << 2;

	bx = CaveTileReg[nLayer][0] - 0x0A + nLayerXOffset[nLayer];
	bx &= 0x01FF;
	cx = (bx >> 3) << 2;
	bx &= 0x0007;

	by = CaveTileReg[nLayer][1] + nLayerYOffset;
	by &= 0x01FF;
	cy = (by >> 3) << 8;
	by &= 0x0007;

	for (y = 0; y < (31 << 8); y += (1 << 8)) {
		nTileRow = (cy + y) & (0x3F << 8);
		nTileYPos = (y >> 5) - by;

		if (nTileYPos <= -8 || nTileYPos >= nCaveYSize) {
			continue;
		}

		for (x = mx; x >= 0; x -= (1 << 2)) {
			nTileColumn = (cx + x) & (0x3F << 2);
			nTileNumber = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x4000 + nTileRow + nTileColumn))) << 16;
			nTileNumber |= BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x4002 + nTileRow + nTileColumn)));

			nTileNumber |= nTileOffset;

			if (CaveTileAttrib[nLayer][nTileNumber & nTileMask[nLayer]]) {
				continue;
			}

			nTileXPos = (x << 1) - bx;

			if (nTileXPos <= -8 || nTileXPos >= nCaveXSize) {
				continue;
			}

			CaveTileQueue[nLayer][nTileNumber >> 30]->x = nTileXPos;
			CaveTileQueue[nLayer][nTileNumber >> 30]->y = nTileYPos;
			CaveTileQueue[nLayer][nTileNumber >> 30]->tile = nTileNumber;
			CaveTileQueue[nLayer][nTileNumber >> 30]++;
		}
	}

	return;
}

static void Cave8x8Layer_Normal(INT32 nLayer, UINT32 nPriority)
{
	UINT32 nTileNumber;

	UINT32* pPalette = CavePalette + nPaletteOffset[nLayer];
	UINT32 nPaletteMask = 0x3F000000;
	if (nPaletteSize[nLayer] == 6) {
		nPaletteMask = 0x0F000000;
	}
	INT32 nPaletteShift = 24 - nPaletteSize[nLayer];

	CaveTile* TileQueue = CaveTileQueue[nLayer][nPriority];

	while ((nTileXPos = TileQueue->x) < 9999) {
		nTileYPos = TileQueue->y;
		nTileNumber = TileQueue->tile;
		pTilePalette = pPalette + ((nTileNumber & nPaletteMask) >> nPaletteShift);
		nTileNumber &= nTileMask[nLayer];

		pTile = pBurnDraw + (nTileYPos * nBurnPitch) + (nTileXPos * nBurnBpp);

		pTileData = (UINT32*)(CaveTileROM[nLayer] + (nTileNumber << 6));

		if (nTileYPos < 0 || nTileYPos > nClipY8 || nTileXPos < 0 || nTileXPos > nClipX8) {
			RenderTile[1]();
		} else {
			RenderTile[0]();
		}

		TileQueue++;
	}

	return;
}

static void Cave8x8Layer_RowScroll(INT32 nLayer, UINT32 nPriority)
{
	INT32 x, y;
	INT32 bx, by, cy;
	INT32 nTileColumn, nTileRow;
	UINT32 nTileNumber;

    UINT8* pTileRAM = CaveTileRAM[nLayer];
	UINT32* pPalette = CavePalette + nPaletteOffset[nLayer];
	INT32 nPaletteShift = 24 - nPaletteSize[nLayer];

	INT32* rowscroll = pRowScroll[nLayer];

	INT32 count = CaveTileCount[nLayer];

	if (count >= (64 * 31)) {
		return;
	}

	UINT32 nTileOffset = 0;
	if (nCaveTileBank) {
		nTileOffset = 0x040000;
	}

	bx = CaveTileReg[nLayer][0] - 0x0A + nLayerXOffset[nLayer];
	bx &= 0x01FF;

	by = CaveTileReg[nLayer][1] + nLayerYOffset;
	by &= 0x01FF;
	cy = (by >> 3) << 8;
	by &= 0x0007;

	if (nPriority == 0) {
		INT32 dy = CaveTileReg[nLayer][1] + 0x12 - nCaveRowModeOffset;

		for (y = 0; y < nCaveYSize; y++) {
			rowscroll[y] = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x1000 + (((dy + y) & 0x01FF) << 2)))) + bx;
		}
	}

	for (y = 0; y < (31 << 8); y += (1 << 8)) {
		nTileYPos = (y >> 5) - by;

		nTileRow = (cy + y) & (0x3F << 8);
		pTileRowInfo =  rowscroll + nTileYPos;

		for (x = 0; x < (64 << 2); x += (1 << 2)) {
			nTileColumn = x & (0x3F << 2);
			nTileNumber = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x4000 + nTileRow + nTileColumn))) << 16;
			if ((nTileNumber >> 30) != nPriority) {
				continue;
			}
			nTileNumber |= BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x4002 + nTileRow + nTileColumn)));
			pTilePalette = pPalette + ((nTileNumber & 0x3F000000) >> nPaletteShift);

			nTileNumber |= nTileOffset;
			nTileNumber &= nTileMask[nLayer];

			count++;

			if (nTileYPos <= -8 || nTileYPos >= nCaveYSize) {
				continue;
			}

			if (*((UINT32*)(CaveTileAttrib[nLayer] + nTileNumber))) {
				continue;
			}

			nTileXPos = (x << 1);

			pTile = pBurnDraw + (nTileYPos * nBurnPitch);

			pTileData = (UINT32*)(CaveTileROM[nLayer] + (nTileNumber << 6));

			if (nTileYPos < 0 || nTileYPos > nClipY8 || nTileXPos < 0 || nTileXPos > nClipX8) {
				RenderTile[3]();
			} else {
				RenderTile[2]();
			}
		}
	}

	if (count >= (64 * 31)) {
		CaveTileMax[0] -= 0x0123;
		CaveTileMax[1] -= 0x0123;
		CaveTileMax[2] -= 0x0123;
		CaveTileMax[3] -= 0x0123;
	}
	CaveTileCount[nLayer] = count;

	return;
}

static void CaveQueue16x16Layer_Normal(INT32 nLayer)
{
	INT32 x, y;
	INT32 bx, by, cx, cy, mx;
	INT32 nTileColumn, nTileRow;
	UINT32 nTileNumber;

    UINT8* pTileRAM = CaveTileRAM[nLayer];

	mx = (nCaveXSize >> 4) << 2;

	bx = CaveTileReg[nLayer][0] - 0x12 + nLayerXOffset[nLayer];
	bx &= 0x01FF;
	cx = (bx >> 4) << 2;
	bx &= 0x000F;

	by = CaveTileReg[nLayer][1] + nLayerYOffset;
	by &= 0x01FF;
	cy = (by >> 4) << 7;
	by &= 0x000F;

	for (y = 0; y < (16 << 7); y += (1 << 7)) {
		nTileRow = (cy + y) & (0x1F << 7);
		nTileYPos = (y >> 3) - by;

		if (nTileYPos <= -16 || nTileYPos >= nCaveYSize) {
			continue;
		}

		for (x = mx; x >= 0; x -= (1 << 2)) {
			nTileColumn = (cx + x) & (0x1F << 2);
			nTileNumber = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x0000 + nTileRow + nTileColumn))) << 16;
			nTileNumber |= BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x0002 + nTileRow + nTileColumn)));

			if (*((UINT32*)(CaveTileAttrib[nLayer] + ((nTileNumber << 2) & nTileMask[nLayer]))) == 0x01010101) {
				continue;
			}

			nTileXPos = (x << 2) - bx;

			if (nTileXPos <= -16 || nTileXPos >= nCaveXSize) {
				continue;
			}

			CaveTileQueue[nLayer][nTileNumber >> 30]->x = nTileXPos;
			CaveTileQueue[nLayer][nTileNumber >> 30]->y = nTileYPos;
			CaveTileQueue[nLayer][nTileNumber >> 30]->tile = nTileNumber;
			CaveTileQueue[nLayer][nTileNumber >> 30]++;
		}
	}

	return;
}

static void Cave16x16Layer_Normal(INT32 nLayer, UINT32 nPriority)
{
	UINT32 nTileNumber, nAttrib;
	UINT32* pTileStart;

	UINT32* pPalette = CavePalette + nPaletteOffset[nLayer];
	INT32 nPaletteShift = 24 - nPaletteSize[nLayer];
	UINT32 nPaletteMask = 0x3F000000;
	if (nPaletteSize[nLayer] == 6) {
		nPaletteMask = 0x0F000000;
	}

	CaveTile* TileQueue = CaveTileQueue[nLayer][nPriority];

	while ((nTileXPos = TileQueue->x) < 9999) {
		nTileYPos = TileQueue->y;
		nTileNumber = TileQueue->tile;
		pTilePalette = pPalette + ((nTileNumber & nPaletteMask) >> nPaletteShift);
		nTileNumber <<= 2;
		nTileNumber &= nTileMask[nLayer];

		nAttrib = BURN_ENDIAN_SWAP_INT32(*((UINT32*)(CaveTileAttrib[nLayer] + nTileNumber)));

		pTile = pBurnDraw + (nTileYPos * nBurnPitch) + (nTileXPos * nBurnBpp);

		pTileStart = (UINT32*)(CaveTileROM[nLayer] + (nTileNumber << 6));

		if (nTileXPos < 0 || nTileXPos > nClipX16 || nTileYPos < 0 || nTileYPos > nClipY16) {

			if ((nAttrib & 0x000000FF) == 0) {
				pTileData = pTileStart;
				if (nTileXPos > -8 && nTileXPos < nCaveXSize && nTileYPos > -8 && nTileYPos < nCaveYSize) {
					if (nTileXPos >= 0 && nTileXPos <= nClipX8 && nTileYPos >= 0 && nTileYPos <= nClipY8) {
						RenderTile[0]();
					} else {
						RenderTile[1]();
					}
				}
			}

			nTileXPos += 8;
			pTile += 8 * nBurnBpp;
			if ((nAttrib & 0x0000FF00) == 0) {
				if (nTileXPos > -8 && nTileXPos < nCaveXSize && nTileYPos > -8 && nTileYPos < nCaveYSize) {
					pTileData = pTileStart + 16;
					if (nTileXPos >= 0 && nTileXPos <= nClipX8 && nTileYPos >= 0 && nTileYPos <= nClipY8) {
						RenderTile[0]();
					} else{
						RenderTile[1]();
					}
				}
			}

			nTileXPos -= 8;
			nTileYPos += 8;
			pTile = pBurnDraw + (nTileYPos * nBurnPitch) + (nTileXPos * nBurnBpp);
			if ((nAttrib & 0x00FF0000) == 0) {
				if (nTileXPos > -8 && nTileXPos < nCaveXSize && nTileYPos > -8 && nTileYPos < nCaveYSize) {
					pTileData = pTileStart + 32;
					if (nTileXPos >= 0 && nTileXPos <= nClipX8 && nTileYPos >= 0 && nTileYPos <= nClipY8) {
						RenderTile[0]();
					} else {
						RenderTile[1]();
					}
				}
			}

			nTileXPos += 8;
			pTile += 8 * nBurnBpp;
			if ((nAttrib & 0xFF000000) == 0) {
				if (nTileXPos > -8 && nTileXPos < nCaveXSize && nTileYPos > -8 && nTileYPos < nCaveYSize) {
					pTileData = pTileStart + 48;
					if (nTileXPos >= 0 && nTileXPos <= nClipX8 && nTileYPos >= 0 && nTileYPos <= nClipY8) {
						RenderTile[0]();
					} else {
						RenderTile[1]();
					}
				}
			}

		} else {
			if ((nAttrib & 0x000000FF) == 0) {
				pTileData = pTileStart;
				RenderTile[0]();
			}
			nTileXPos += 8;
			pTile += 8 * nBurnBpp;
			if ((nAttrib & 0x0000FF00) == 0) {
				pTileData = pTileStart + 16;
				RenderTile[0]();
			}
			nTileXPos -= 8;
			nTileYPos += 8;
			pTile = pBurnDraw + (nTileYPos * nBurnPitch) + (nTileXPos * nBurnBpp);
			if ((nAttrib & 0x00FF0000) == 0) {
				pTileData = pTileStart + 32;
				RenderTile[0]();
			}
			nTileXPos += 8;
			pTile += 8 * nBurnBpp;
			if ((nAttrib & 0xFF000000) == 0) {
				pTileData = pTileStart + 48;
				RenderTile[0]();
			}
		}

		TileQueue++;
	}

	return;
}

static void Cave16x16Layer_RowScroll(INT32 nLayer, UINT32 nPriority)
{
	INT32 x, y;
	INT32 bx, by, cy;
	INT32 nTileColumn, nTileRow;
	UINT32 nTileNumber, nAttrib;
	UINT32* pTileStart;

    UINT8* pTileRAM = CaveTileRAM[nLayer];
	UINT32* pPalette = CavePalette + nPaletteOffset[nLayer];
	INT32 nPaletteShift = 24 - nPaletteSize[nLayer];
	UINT32 nPaletteMask = 0x3F000000;
	if (nPaletteSize[nLayer] == 6) {
		nPaletteMask = 0x0F000000;
	}

	INT32* rowscroll = pRowScroll[nLayer];

	INT32 count = CaveTileCount[nLayer];

	if (count >= (32 * 16)) {
		return;
	}

	bx = CaveTileReg[nLayer][0] - 0x12 + nLayerXOffset[nLayer];
	bx &= 0x01FF;

	by = CaveTileReg[nLayer][1] + nLayerYOffset;
	by &= 0x01FF;
	cy = (by >> 4) << 7;
	by &= 0x000F;

	if (nPriority == 0) {
		INT32 dy = CaveTileReg[nLayer][1] + 0x12 - nCaveRowModeOffset;

		for (y = 0; y < nCaveYSize; y++) {
			rowscroll[y] = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x1000 + (((dy + y) & 0x01FF) << 2)))) + bx;
		}
	}

	for (y = 0; y < (16 << 7); y += (1 << 7)) {
		nTileRow = (cy + y) & (0x1F << 7);
		nTileYPos = (y >> 3) - by;

		pTileRowInfo = rowscroll + nTileYPos;

		for (x = 0; x < (32 << 2); x += (1 << 2)) {
			nTileColumn = x & (0x1F << 2);
			nTileNumber = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x0000 + nTileRow + nTileColumn))) << 16;
			if ((nTileNumber >> 30) != nPriority) {
				continue;
			}
			nTileNumber |= BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x0002 + nTileRow + nTileColumn)));
			pTilePalette = pPalette + ((nTileNumber & nPaletteMask) >> nPaletteShift);
			nTileNumber <<= 2;
			nTileNumber &= nTileMask[nLayer];

			count++;

			nAttrib = BURN_ENDIAN_SWAP_INT32(*((UINT32*)(CaveTileAttrib[nLayer] + nTileNumber)));
			if (nAttrib == 0x01010101) {
				continue;
			}

			nTileXPos = (x << 2);

			pTile = pBurnDraw + (nTileYPos * nBurnPitch);

			pTileStart = (UINT32*)(CaveTileROM[nLayer] + (nTileNumber << 6));

			if (nTileYPos < 0 || nTileYPos > nClipY16) {
				if ((nAttrib & 0x000000FF) == 0) {
					pTileData = pTileStart;
					if (nTileYPos > -8 && nTileYPos < nCaveYSize) {
						if (nTileYPos >= 0 && nTileYPos <= nClipY8) {
							RenderTile[2]();
						} else {
							RenderTile[3]();
						}
					}
				}
				nTileXPos += 8;
				if ((nAttrib & 0x0000FF00) == 0) {
					if (nTileYPos > -8 && nTileYPos < nCaveYSize) {
						pTileData = pTileStart + 16;
						if (nTileYPos >= 0 && nTileYPos <= nClipY8) {
							RenderTile[2]();
						} else {
							RenderTile[3]();
						}
					}
				}
				nTileXPos -= 8;
				nTileYPos += 8;

    			pTileRowInfo += 8;

				pTile = pBurnDraw + (nTileYPos * nBurnPitch);
				if ((nAttrib & 0x00FF0000) == 0) {
					if (nTileYPos > -8 && nTileYPos < nCaveYSize) {
						pTileData = pTileStart + 32;
						if (nTileYPos >= 0 && nTileYPos <= nClipY8) {
							RenderTile[2]();
						} else {
							RenderTile[3]();
						}
					}
				}
				nTileXPos += 8;
				if ((nAttrib & 0xFF000000) == 0) {
					if (nTileYPos > -8 && nTileYPos < nCaveYSize) {
						pTileData = pTileStart + 48;
						if (nTileYPos >= 0 && nTileYPos <= nClipY8) {
							RenderTile[2]();
						} else {
							RenderTile[3]();
						}
					}
				}
			} else {
				if ((nAttrib & 0x000000FF) == 0) {
					pTileData = pTileStart;
					RenderTile[2]();
				}
				nTileXPos += 8;
				if ((nAttrib & 0x0000FF00) == 0) {
					pTileData = pTileStart + 16;
					RenderTile[2]();
				}
				nTileXPos -= 8;
				nTileYPos += 8;

    			pTileRowInfo += 8;

				pTile = pBurnDraw + (nTileYPos * nBurnPitch);
				if ((nAttrib & 0x00FF0000) == 0) {
					pTileData = pTileStart + 32;
					RenderTile[2]();
				}
				nTileXPos += 8;
				if ((nAttrib & 0xFF000000) == 0) {
					pTileData = pTileStart + 48;
					RenderTile[2]();
				}
			}
			nTileYPos -= 8;

			pTileRowInfo -= 8;
		}
	}

	if (count >= (32 * 16)) {
		CaveTileMax[0] -= 0x0123;
		CaveTileMax[1] -= 0x0123;
		CaveTileMax[2] -= 0x0123;
		CaveTileMax[3] -= 0x0123;
	}
	CaveTileCount[nLayer] = count;

	return;
}

static void Cave16x16Layer_RowSelect(INT32 nLayer, UINT32 nPriority)
{
	INT32 x, y, ry, rx;
	INT32 bx, by, mx;
	INT32 nTileColumn, nTileRow;
	UINT32 nTileNumber;
	UINT32* pTileStart;

    UINT8* pTileRAM = CaveTileRAM[nLayer];
	UINT32* pPalette = CavePalette + nPaletteOffset[nLayer];
	INT32 nPaletteShift = 24 - nPaletteSize[nLayer];
	UINT32 nPaletteMask = 0x3F000000;
	if (nPaletteSize[nLayer] == 6) {
		nPaletteMask = 0x0F000000;
	}

	INT32* rowselect = pRowSelect[nLayer];
	INT32* rowscroll = pRowScroll[nLayer];

	INT32 count = CaveTileCount[nLayer];

	mx = nCaveXSize >> 4;

	if (count >= nCaveYSize * (mx + 1)) {
		return;
	}

	bx = CaveTileReg[nLayer][0] - 0x12 + nLayerXOffset[nLayer];
	bx &= 0x01FF;

	by = CaveTileReg[nLayer][1] + nLayerYOffset;
	by &= 0x01FF;

	if (nPriority == 0) {
		INT32 cy = CaveTileReg[nLayer][1] + 0x12 - nCaveRowModeOffset;

		if (CaveTileReg[nLayer][0] & 0x4000) {	// Row-scroll mode
			for (y = 0; y < nCaveYSize; y++) {
				rowselect[y] = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x1002 + (((cy + y) & 0x01FF) << 2))));
				rowscroll[y] = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x1000 + (((cy + y) & 0x01FF) << 2)))) + bx;
			}
		} else {
			for (y = 0; y < nCaveYSize; y++) {
				rowselect[y] = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x1002 + (((cy + y) & 0x01FF) << 2))));
			}
		}
	}

	for (y = 0; y < nCaveYSize; y++) {
		ry = rowselect[y];
		nTileRow = ((ry >> 4) & 0x1F) << 7;
		nTileYPos = y;

		if (CaveTileReg[nLayer][0] & 0x4000) {	// Row-scroll mode
			rx = rowscroll[y];
			ry = (((ry & 8) << 1) + (ry & 7)) << 1;

			for (x = 0; x <= mx; x++) {

				nTileColumn = (((rx >> 4) + x) & 0x1F) << 2;
				nTileNumber = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x0000 + nTileRow + nTileColumn))) << 16;
				if ((nTileNumber >> 30) != nPriority) {
					continue;
				}
				nTileNumber |= BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x0002 + nTileRow + nTileColumn)));
				pTilePalette = pPalette + ((nTileNumber & nPaletteMask) >> nPaletteShift);
				nTileNumber &= nTileMask[nLayer];

				count++;

				if (((UINT32*)CaveTileAttrib[nLayer])[nTileNumber] == 0x01010101) {
					continue;
				}

				nTileXPos = (x << 4) - (rx & 15);

				if (nTileXPos <= -16 || nTileXPos >= nCaveXSize) {
					continue;
				}

				pTile = pBurnDraw + (nTileYPos * nBurnPitch) + (nTileXPos * nBurnBpp);

				pTileStart = (UINT32*)(CaveTileROM[nLayer] + (nTileNumber << 8));
				pTileStart += ry;

				pTileData = pTileStart;
				if (nTileXPos >= 0 && nTileXPos <= nClipX8) {
					RenderTile[4]();
				} else {
					RenderTile[5]();
				}
				nTileXPos += 8;
				pTile += (nBurnBpp << 3);
				pTileData = pTileStart + 16;
				if (nTileXPos >= 0 && nTileXPos <= nClipX16) {
					RenderTile[4]();
				} else {
					RenderTile[5]();
				}
			}
		} else {
			nTileXPos = - (bx & 15);
			pTile = pBurnDraw + (nTileYPos * nBurnPitch) - ((bx & 15) * nBurnBpp);

			ry = (((ry & 8) << 1) + (ry & 7)) << 1;

			for (x = 0; x <= mx; x++) {
				nTileColumn = (((bx >> 4) + x) & 0x1F) << 2;
				nTileNumber = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x0000 + nTileRow + nTileColumn))) << 16;
				if ((nTileNumber >> 30) != nPriority) {
					nTileXPos += 16;
					pTile += (nBurnBpp << 4);
					continue;
				}
				nTileNumber |= BURN_ENDIAN_SWAP_INT16(*((UINT16*)(pTileRAM + 0x0002 + nTileRow + nTileColumn)));
				pTilePalette = pPalette + ((nTileNumber & nPaletteMask) >> nPaletteShift);
				nTileNumber &= nTileMask[nLayer];

				count++;

				if (((UINT32*)CaveTileAttrib[nLayer])[nTileNumber] == 0x01010101) {
					nTileXPos += 16;
					pTile += (nBurnBpp << 4);
					continue;
				}

				pTileStart = (UINT32*)(CaveTileROM[nLayer] + (nTileNumber << 8));
				pTileStart += ry;

				pTileData = pTileStart;

				if (nTileXPos >= 0 && nTileXPos <= nClipX8) {
					RenderTile[4]();
				} else {
					RenderTile[5]();
				}
				nTileXPos += 8;
				pTile += (nBurnBpp << 3);

				pTileData = pTileStart + 16;
				if (nTileXPos >= 0 && nTileXPos <= nClipX8) {
					RenderTile[4]();
				} else {
					RenderTile[5]();
				}
				nTileXPos += 8;
				pTile += (nBurnBpp << 3);
			}
		}
	}

	if (count >= nCaveYSize * (mx + 1)) {
		CaveTileMax[0] -= 0x0123;
		CaveTileMax[1] -= 0x0123;
		CaveTileMax[2] -= 0x0123;
		CaveTileMax[3] -= 0x0123;
	}
	CaveTileCount[nLayer] = count;

	return;
}

static void Cave16x16Layer(INT32 nLayer, UINT32 nPriority)
{
	if (CaveTileReg[nLayer][1] & 0x4000) {			// Row-select mode
		Cave16x16Layer_RowSelect(nLayer, nPriority);
//		bprintf(PRINT_NORMAL, "   Layer %d row-select mode enabled (16x16)\n", nLayer);
		return;
	}

	if (CaveTileReg[nLayer][0] & 0x4000) {			// Row-scroll mode
		Cave16x16Layer_RowScroll(nLayer, nPriority);
//		bprintf(PRINT_NORMAL, "   Layer %d row-scroll mode enabled (16x16)\n", nLayer);
		return;
	}

	Cave16x16Layer_Normal(nLayer, nPriority);

	return;
}

static void Cave8x8Layer(INT32 nLayer, UINT32 nPriority)
{
#if 0
	if (CaveTileReg[nLayer][1] & 0x4000) {			// Row-select mode
		Cave8x8Layer_RowSelect(nLayer, nPriority);
//		bprintf(PRINT_NORMAL, "   Layer %d row-select mode enabled ( 8x 8)\n", nLayer);
		return;
	}
#else
	if (CaveTileReg[nLayer][1] & 0x4000) {
//		bprintf(PRINT_ERROR, "   Layer %d row-select mode enabled ( 8x 8, UNSUPPORTED!)\n", nLayer);
	}
#endif

	if (CaveTileReg[nLayer][0] & 0x4000) {			// Row-scroll mode
		Cave8x8Layer_RowScroll(nLayer, nPriority);
//		bprintf(PRINT_NORMAL, "   Layer %d row-scroll mode enabled ( 8x 8)\n", nLayer);
		return;
	}

	Cave8x8Layer_Normal(nLayer, nPriority);

	return;
}

INT32 CaveTileRender(INT32 nMode)
{
	UINT32 nPriority;
	INT32 nLowPriority;
	INT32 nLayer;
	INT32 nOffset = 0;

	CaveTileCount[0] = CaveTileCount[1] = CaveTileCount[2] = CaveTileCount[3] = 0;
	CaveTileMax[0] = CaveTileMax[1] = CaveTileMax[2] = CaveTileMax[3] = 0;

	nLayerYOffset = 0x12 - nCaveYOffset - nCaveExtraYOffset;

	for (nLayer = 0; nLayer < 4; nLayer++) {

		nLayerXOffset[nLayer] = nLayer + nOffset - nCaveXOffset - nCaveExtraXOffset;
		
		if ((CaveTileReg[nLayer][2] & 0x0010) == 0) {

			for (nPriority = 0; nPriority < 4; nPriority++) {
				CaveTileQueue[nLayer][nPriority] = &CaveTileQueueMemory[nLayer][nPriority * 1536];
			}
			if (nBurnLayer & (8 >> nLayer)) {
				if (CaveTileReg[nLayer][1] & 0x2000) {
					if (nMode && ((CaveTileReg[nLayer][0] | CaveTileReg[nLayer][1]) & 0x4000)) {
						CaveTileMax[0] += 0x0123;
						CaveTileMax[1] += 0x0123;
						CaveTileMax[2] += 0x0123;
						CaveTileMax[3] += 0x0123;
					} else {
						CaveQueue16x16Layer_Normal(nLayer);
					}
				} else {
					if (nMode && (CaveTileReg[nLayer][0] & 0x4000)) {
						CaveTileMax[0] += 0x0123;
						CaveTileMax[1] += 0x0123;
						CaveTileMax[2] += 0x0123;
						CaveTileMax[3] += 0x0123;
					} else {
						CaveQueue8x8Layer_Normal(nLayer);
					}

//					nOffset += 8;
				}
			}

			for (nPriority = 0; nPriority < 4; nPriority++) {
				CaveTileQueue[nLayer][nPriority]->x = 9999;
				CaveTileQueue[nLayer][nPriority] = &CaveTileQueueMemory[nLayer][nPriority * 1536];
				if (CaveTileQueue[nLayer][nPriority]->x < 9999) {
					CaveTileMax[nPriority] += 0x123;
				}
			}
		}
	}

	nLowPriority = 0;

	for (nPriority = 0; nPriority < 4; nPriority++) {

		if (CaveTileMax[nPriority] || nPriority == 3) {
			CaveSpriteRender(nLowPriority, nPriority);
			nLowPriority = nPriority + 1;
		}

		for (UINT32 i = 0; i < 4; i++) {
			for (nLayer = 0; nLayer < 4; nLayer++) {
				if ((CaveTileReg[nLayer][2] & 0x0003) == i) {
					if ((CaveTileReg[nLayer][2] & 0x0010) || (nBurnLayer & (8 >> nLayer)) == 0) {
						continue;
					}

					if (CaveTileReg[nLayer][1] & 0x2000) {
						if (nMode) {
							Cave16x16Layer(nLayer, nPriority);
						} else {
							Cave16x16Layer_Normal(nLayer, nPriority);
						}
					} else {
						if (nMode) {
							Cave8x8Layer(nLayer, nPriority);
						} else {
							Cave8x8Layer_Normal(nLayer, nPriority);
						}
					}
				}
			}
		}
	}

	return 0;
}

void CaveTileExit()
{
	for (INT32 nLayer = 0; nLayer < 4; nLayer++) {
		BurnFree(CaveTileAttrib[nLayer]);
		BurnFree(CaveTileQueueMemory[nLayer]);
		BurnFree(pRowScroll[nLayer]);
		BurnFree(pRowSelect[nLayer]);
	}

	nCaveXOffset = nCaveYOffset = 0;
	nCaveRowModeOffset = 0;
	nCaveExtraXOffset = nCaveExtraYOffset = 0;

	return;
}

INT32 CaveTileInit()
{
	for (INT32 nLayer = 0; nLayer < 4; nLayer++) {
		CaveTileReg[nLayer][0] = 0x0000;
		CaveTileReg[nLayer][1] = 0x0000;
		CaveTileReg[nLayer][2] = 0x0010;							// Disable layer
	}

	nCaveTileBank = 0;

	BurnDrvGetFullSize(&nCaveXSize, &nCaveYSize);

	nClipX8  = nCaveXSize -  8;
	nClipX16 = nCaveXSize - 16;
	nClipY8  = nCaveYSize -  8;
	nClipY16 = nCaveYSize - 16;

	RenderTile = RenderTile_ROT0[(nCaveXSize == 320) ? 0 : 1];

	return 0;
}

INT32 CaveTileInitLayer(INT32 nLayer, INT32 nROMSize, INT32 nBitdepth, INT32 nOffset)
{
	INT32 nTileSize = (8 << 3);
	INT32 nNumTiles = nROMSize / nTileSize;

	for (nTileMask[nLayer] = 1; nTileMask[nLayer] < nNumTiles; nTileMask[nLayer] <<= 1) { }
	nTileMask[nLayer]--;

	if (CaveTileAttrib[nLayer]) {
		BurnFree(CaveTileAttrib[nLayer]);
	}
	CaveTileAttrib[nLayer] = (INT8*)BurnMalloc(nTileMask[nLayer] + 1);
	if (CaveTileAttrib[nLayer] == NULL) {
		return 1;
	}

	for (INT32 i = 0; i < nNumTiles; i++) {
		bool bTransparent = true;
		for (INT32 j = i * nTileSize; j < (i + 1) * nTileSize; j++) {
			if (CaveTileROM[nLayer][j]) {
				bTransparent = false;
				break;
			}
		}
		if (bTransparent) {
			CaveTileAttrib[nLayer][i] = 1;
		} else {
			CaveTileAttrib[nLayer][i] = 0;
		}
	}

	for (INT32 i = nNumTiles; i <= nTileMask[nLayer]; i++) {
		CaveTileAttrib[nLayer][i] = 1;
	}

	if (CaveTileQueueMemory[nLayer]) {
		BurnFree(CaveTileQueueMemory[nLayer]);
	}
	CaveTileQueueMemory[nLayer] = (CaveTile*)BurnMalloc(4 * 1536 * sizeof(CaveTile));
	if (CaveTileQueueMemory[nLayer] == NULL) {
		return 1;
	}

	if (pRowScroll[nLayer]) {
		BurnFree(pRowScroll[nLayer]);
	}
	pRowScroll[nLayer] = (INT32*)BurnMalloc(nCaveYSize * sizeof(INT32));
	if (pRowScroll[nLayer] == NULL) {
		return 1;
	}
	
	if (pRowSelect[nLayer]) {
		BurnFree(pRowSelect[nLayer]);
	}
	pRowSelect[nLayer] = (INT32*)BurnMalloc(nCaveYSize * sizeof(INT32));
	if (pRowSelect[nLayer] == NULL) {
		return 1;
	}

	nPaletteSize[nLayer] = nBitdepth;
	nPaletteOffset[nLayer] = nOffset;

	CaveTileReg[nLayer][2] = 0x0000;							// Enable layer

	return 0;
}
