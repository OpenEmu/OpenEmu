// Psikyo hardware tilemaps

#include "psikyo.h"

UINT8* PsikyoTileROM;
UINT8* PsikyoTileRAM[3] = { NULL, };

bool bPsikyoClearBackground;

static UINT32 PsikyoLayerAttrib[3] = { 0, };
static INT32 PsikyoLayerXOffset[2] = { 0, };
static INT32 PsikyoLayerYOffset[2] = { 0, };

static INT8* PsikyoTileAttrib = { NULL, };
static INT32 PsikyoTileMask;

static UINT32 PsikyoTileBank[2];

static UINT8* pTile;
static UINT8* pTileData;
static UINT32* pTilePalette;
static INT16* pTileRowInfo;

typedef void (*RenderTileFunction)();

static INT32 nTilemapWith, nTileXPos, nTileYPos;

// Include the tile rendering functions
#include "psikyo_tile_func.h"

static void PsikyoRenderLayer(INT32 nLayer)
{
	static INT32 nLayerXSize[] = { 0x0040, 0x0080, 0x0100, 0x0020 };
//	static INT32 nLayerYSize[] = { 0x0040, 0x0020, 0x0010, 0x0080 };

	INT32 x, y;
	INT32 ax, bx, ay, by, cx, cy, mx;
	INT32 minx, maxx;
	INT32 nTileColumn, nTileRow;
	UINT32 nTileNumber, nTilePalette, nTileBank;
	INT32 nRenderFunction, nTransColour;
	bool bClip;

    UINT16* pTileRAM = (UINT16*)(PsikyoTileRAM[nLayer]);

	UINT32* LayerPalette = PsikyoPalette + (nLayer ? 0x0C00 : 0x0800);

	nTileBank = PsikyoTileBank[nLayer];

	mx = nLayerXSize[(PsikyoLayerAttrib[nLayer] & 0xC0) >> 6];

	pTileRowInfo = (INT16*)(PsikyoTileRAM[2] + (nLayer << 9));
	nTilemapWith = (mx << 4) - 1;

	nRenderFunction = 8;
	nTransColour = 99;
	if ((bPsikyoClearBackground || nLayer) && (PsikyoLayerAttrib[nLayer] & 0x02) == 0) {
		if (PsikyoLayerAttrib[nLayer] & 0x08) {
			nRenderFunction = 0;
			nTransColour = 0;
		} else {
			nRenderFunction = 4;
			nTransColour = 15;
		}
	}

	// Compute the tile offsets
	ax = PsikyoLayerXOffset[nLayer];
	bx = 16 - (ax & 0x0F);
	cx = (ax >> 4) + 1;

	ay = PsikyoLayerYOffset[nLayer];
	by = 16 - (ay & 0x0F);
	cy = ((ay >> 4) + 1) * mx;

	for (nTileYPos = -16 + by, y = -mx; nTileYPos < 224; nTileYPos += 16, y += mx) {

		if (nTileYPos <= -16) {
			continue;
		}

		nTileRow = (cy + y) & 0x0FFF;

		bClip = (nTileYPos < 0 || nTileYPos > 208);

		if (PsikyoLayerAttrib[nLayer] & 0x0300) {
			if (PsikyoLayerAttrib[nLayer] & 0x0200) {

				// per-tile rowscroll; each tilerow has it's own offset to the tilemap x offset

				minx = (INT16)BURN_ENDIAN_SWAP_INT16(pTileRowInfo[(nTileYPos >> 4) & 0x0F]);
			} else {

				// Rowscroll enabled

				minx = 32767; maxx = -32768;

				// Compute the min/max row offsets
				for (INT32 i = 0; i < 16; i++) {
					if ((INT16)BURN_ENDIAN_SWAP_INT16(pTileRowInfo[(nTileYPos + i) & 0xFF]) > maxx) {
						maxx = (INT16)BURN_ENDIAN_SWAP_INT16(pTileRowInfo[(nTileYPos + i) & 0xFF]);
					}
					if ((INT16)BURN_ENDIAN_SWAP_INT16(pTileRowInfo[(nTileYPos + i) & 0xFF]) < minx) {
						minx = (INT16)BURN_ENDIAN_SWAP_INT16(pTileRowInfo[(nTileYPos + i) & 0xFF]);
					}
				}

				// If rowscroll is enabled, but each line of the tilerow has the
				// same offset, we can use the normal tilemap rendering
				// Otherwise, use specific rowscroll rendering
				if (minx != maxx) {

					// Compute the tile offsets
					ax = PsikyoLayerXOffset[nLayer];
					bx = 16 - (ax & 0x0F);
					cx = (ax >> 4) + 1;

					maxx = 21 + (maxx >> 4);
					minx = (minx - 16) >> 4;

					for (x = minx, nTileXPos = (minx << 4) + bx; x < maxx; x++, nTileXPos += 16) {

						nTileColumn = (cx + x) & (mx - 1);
						nTileNumber = BURN_ENDIAN_SWAP_INT16(pTileRAM[nTileRow + nTileColumn]);
						nTilePalette = nTileNumber >> 13;
						nTileNumber = (nTileNumber & 0x1FFF) + nTileBank;

						if (PsikyoTileAttrib[nTileNumber] == nTransColour) {
							continue;
						}

						pTile = pBurnDraw + (nTileYPos * nBurnPitch);
						pTileData = PsikyoTileROM + (nTileNumber << 8);
						pTilePalette = LayerPalette + (nTilePalette << 4);

						if (bClip) {
							RenderTile[nRenderFunction + 3]();
						} else {
							RenderTile[nRenderFunction + 2]();
						}
					}

					continue;
				}
			}

			// Compute the tile offsets
			ax = PsikyoLayerXOffset[nLayer] + minx;
			bx = 16 - (ax & 0x0F);
			cx = (ax >> 4) + 1;
		}

		for (nTileXPos = -16 + bx, x = -1; nTileXPos < 320; nTileXPos += 16, x++) {

			if (nTileXPos <= -16) {
				continue;
			}

			nTileColumn = (cx + x) & (mx - 1);
			nTileNumber = BURN_ENDIAN_SWAP_INT16(pTileRAM[nTileRow + nTileColumn]);
			nTilePalette = nTileNumber >> 13;
			nTileNumber = (nTileNumber & 0x1FFF) + nTileBank;

			if (PsikyoTileAttrib[nTileNumber] == nTransColour) {
				continue;
			}

			pTile = pBurnDraw + (nTileYPos * nBurnPitch) + (nTileXPos * nBurnBpp);
			pTileData = PsikyoTileROM + (nTileNumber << 8);
			pTilePalette = LayerPalette + (nTilePalette << 4);

			if (bClip || nTileXPos < 0 || nTileXPos > 304) {
				RenderTile[nRenderFunction + 1]();
			} else {
				RenderTile[nRenderFunction + 0]();
			}
		}
	}

	return;
}

void PsikyoSetTileBank(INT32 nLayer, INT32 nBank)
{
	PsikyoTileBank[nLayer] = nBank << 13;
}

INT32 PsikyoTileRender()
{
	INT32 nPriority, nLowPriority = 0;

	PsikyoLayerAttrib[0]  = BURN_ENDIAN_SWAP_INT16(*((INT16*)(PsikyoTileRAM[2] + 0x0412)));
	PsikyoLayerXOffset[0] = BURN_ENDIAN_SWAP_INT16(*((INT16*)(PsikyoTileRAM[2] + 0x0406)));
	PsikyoLayerYOffset[0] = BURN_ENDIAN_SWAP_INT16(*((INT16*)(PsikyoTileRAM[2] + 0x0402)));

	PsikyoLayerAttrib[1]  = BURN_ENDIAN_SWAP_INT16(*((INT16*)(PsikyoTileRAM[2] + 0x0416)));
	PsikyoLayerXOffset[1] = BURN_ENDIAN_SWAP_INT16(*((INT16*)(PsikyoTileRAM[2] + 0x040E)));
	PsikyoLayerYOffset[1] = BURN_ENDIAN_SWAP_INT16(*((INT16*)(PsikyoTileRAM[2] + 0x040A)));

	if (PsikyoHardwareVersion == PSIKYO_HW_GUNBIRD) {
		PsikyoTileBank[0] = (PsikyoLayerAttrib[0] & 0x0400) << 3;
		PsikyoTileBank[1] = (PsikyoLayerAttrib[1] & 0x0400) << 3;
	}

	if (bPsikyoClearBackground || ((PsikyoLayerAttrib[0] & 0x01) && (PsikyoLayerAttrib[1] & 0x02)) || (nBurnLayer & 0x0C) != 0x0C) {
		BurnClearScreen();
	}

	for (nPriority = 1; nPriority < 4; nPriority++) {

		INT32 nLayer = nPriority - 1;

		if ((PsikyoLayerAttrib[nLayer] & 1) == 0 || nPriority == 3) {
			PsikyoSpriteRender(nLowPriority, nPriority);
			nLowPriority = nPriority + 1;
		}

		if (nLayer < 2 && (PsikyoLayerAttrib[nLayer] & 1) == 0 && (PsikyoLayerAttrib[nLayer + 1] & 0x02) == 0 && (nBurnLayer & (4 << nLayer))) {
			PsikyoRenderLayer(nLayer);
		}
	}

	return 0;
}

void PsikyoTileExit()
{
	BurnFree(PsikyoTileAttrib);

	return;
}

INT32 PsikyoTileInit(UINT32 nROMSize)
{
	const INT32 nTileSize = 256;
	INT32 nNumTiles = nROMSize / nTileSize;

	for (PsikyoTileMask = 1; PsikyoTileMask < nNumTiles; PsikyoTileMask <<= 1) { }
	PsikyoTileMask--;

	if (PsikyoTileAttrib) {
		BurnFree(PsikyoTileAttrib);
	}
	PsikyoTileAttrib = (INT8*)BurnMalloc(PsikyoTileMask + 1);
	if (PsikyoTileAttrib == NULL) {
		return 1;
	}

	for (INT32 i = 0; i < nNumTiles; i++) {
		bool bTransparent0 = true;
		bool bTransparent15 = true;
		for (INT32 j = i * nTileSize; j < (i + 1) * nTileSize; j++) {
			if (PsikyoTileROM[j] != 0x00) {
				bTransparent0 = false;
				if (!bTransparent15) {
					break;
				}
			}
			if (PsikyoTileROM[j] != 0xFF) {
				bTransparent15 = false;
				if (!bTransparent0) {
					break;
				}
			}
		}
		PsikyoTileAttrib[i] = (INT8)0xFF;
		if (bTransparent0) {
			PsikyoTileAttrib[i] = 0;
		}
		if (bTransparent15) {
			PsikyoTileAttrib[i] = 15;
		}
	}

	for (INT32 i = nNumTiles; i <= PsikyoTileMask; i++) {
		PsikyoTileAttrib[i] = (INT8)0xFF;
	}

	PsikyoTileBank[0] = 0x0000;
	PsikyoTileBank[1] = 0x2000;

	return 0;
}
