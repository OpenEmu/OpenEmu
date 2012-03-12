#include "toaplan.h"

INT32 ToaOpaquePriority;

static UINT8* pTile;
static UINT32* pTileData;
static UINT32* pTilePalette;

INT32 Rallybik = 0;
INT32 Hellfire = 0;

typedef void (*RenderTileFunction)();
static RenderTileFunction* RenderTile;

static INT32 nTileXPos, nTileYPos;

// Include the tile rendering functions
#include "toa_gp9001_func.h"

static INT32 nLastBPP = 0;

struct ToaTile {
	UINT32 nTileAttrib;
	INT16 nTileXPos;
	INT16 nTileYPos;
};

extern UINT32* ToaPalette2;

// ----------------------------------------------------------------------------
// Rally Bike custom sprite function

static void rallybik_draw_sprites(INT32 priority)
{
	UINT16 *sprite = (UINT16*)FCU2RAM; 

	for (INT32 offs = 0; offs < (0x1000/2); offs += 4)
	{
		INT32 attrib = sprite[offs + 1];

		if ((attrib & 0x0c00) == priority)
		{
			INT32 sy = (sprite[offs + 3] >> 7) & 0x1ff;

			if (sy != 0x0100)
			{
				INT32 code	= sprite[offs] & 0x7ff;
				INT32 color	= attrib & 0x3f;
				INT32 sx		= (sprite[offs + 2] >> 7) & 0x1ff;
				INT32 flipx	= attrib & 0x100;
				INT32 flipy	= attrib & 0x200;
				if (flipx) sx -= 15;

				sx -= 31;
				sy -= 16;

				if (sy < -15 || sx < -15 || sy >= 240 || sx >= 320) continue;

				{
					INT32 flip = 0;
					if (flipx) flip |= 0x0f;
					if (flipy) flip |= 0xf0;
					UINT8 *gfx = FCU2ROM + (code * 0x100);

					pTilePalette = &ToaPalette2[color << 4];

					for (INT32 y = 0; y < 16; y++, sy++) {
						if (sy < 0 || sy >= 240) continue;

						for (INT32 x = 0; x < 16; x++, sx++) {
							if (sx < 0 || sx >= 320) continue;

							INT32 pxl = gfx[((y * 16) + x) ^ flip];

							if (pxl) {
								PutPix(pBurnDraw + ((sy * 320) + sx) * nBurnBpp, pTilePalette[pxl]);
							}
						}
						sx -= 16;
					}
				}
			}
		}
	}
}


// ----------------------------------------------------------------------------
// FCU-2 functions

UINT8* FCU2ROM;
UINT32 nFCU2ROMSize;
static UINT8* FCU2TileAttrib;

static UINT32 nFCU2MaxSprite;

UINT8* FCU2RAM;
UINT8* FCU2RAMSize;
UINT32 FCU2Pointer;

static UINT8* pFCU2SpriteBuffer;

static UINT16** pFCU2SpriteQueue[16];
static UINT16** pFCU2SpriteQueueData = NULL;

//static INT32 nFCU2SpriteXOffset, nFCU2SpriteYOffset;

static void FCU2PrepareSprites()
{
	UINT16* pSpriteInfo;
	INT32 nSprite;

	for (INT32 nPriority = 0; nPriority < 16; nPriority++) {
		pFCU2SpriteQueue[nPriority] = &pFCU2SpriteQueueData[(nPriority << 8) + nPriority];
	}

	for (nSprite = 0, pSpriteInfo = (UINT16*)pFCU2SpriteBuffer; nSprite < 0x0100; nSprite++, pSpriteInfo += 4) {
		if (!(pSpriteInfo[0] & 0x8000)) {				// Sprite is enabled
			*pFCU2SpriteQueue[pSpriteInfo[1] >> 12]++ = pSpriteInfo;
		}
	}
}

static void FCU2RenderSpriteQueue(INT32 nPriority)
{
	UINT16* pSpriteInfo;
	UINT8* pSpriteData;
	INT32 nSpriteXPos, nSpriteYPos;
	INT32 nSpriteXSize, nSpriteYSize;
	UINT32 nSpriteNumber;
	INT32 x, y, s;

	*pFCU2SpriteQueue[nPriority] = NULL;
	pFCU2SpriteQueue[nPriority] = &pFCU2SpriteQueueData[(nPriority << 8) + nPriority];

	while ((pSpriteInfo = *pFCU2SpriteQueue[nPriority]++) != NULL) {

		pTilePalette = &ToaPalette2[(pSpriteInfo[1] & 0x3F) << 4];
		nSpriteNumber = pSpriteInfo[0] & nFCU2MaxSprite;

		pSpriteData = FCU2ROM + (nSpriteNumber << 5);

		s = (pSpriteInfo[1] & 0x0FC0) >> 6;

		nSpriteXSize = ((((UINT16*)FCU2RAMSize)[s] >> 0) & 0x0F);
		nSpriteYSize = ((((UINT16*)FCU2RAMSize)[s] >> 4) & 0x0F);

		nSpriteXPos = (pSpriteInfo[2] >> 7) + 0;//nFCU2SpriteXOffset;
		nSpriteXPos &= 0x01FF;
		nSpriteYPos = (pSpriteInfo[3] >> 7) + 0;//nFCU2SpriteYOffset;
		nSpriteYPos &= 0x01FF;

		if (Hellfire) nSpriteYPos -= 16;

		if (nSpriteXPos > 384) {
			nSpriteXPos -= 0x0200;
		}
		if (nSpriteYPos > 384) {
			nSpriteYPos -= 0x0200;
		}
		
//		if (nSpriteNumber) {
//			printf("Sprite\t0x%04X\n", nSpriteNumber);
//			printf("xp: %4i, yp: %4i\n", nSpriteXPos, nSpriteYPos);
//			printf("xs: %4i, ys: %4i\n", nSpriteXSize, nSpriteYSize);
//		}

		for (y = 0, nTileYPos = nSpriteYPos; y < nSpriteYSize; y++, nTileYPos += 8) {
			for (x = 0, nTileXPos = nSpriteXPos; x < nSpriteXSize; x++, nTileXPos += 8, nSpriteNumber++, pSpriteData += 32) {
				if (nSpriteNumber > nFCU2MaxSprite) {
					break;
				}
				if (FCU2TileAttrib[nSpriteNumber]) {
					// Skip tile if it's completely off the screen
					if (!(nTileXPos <= -8 || nTileXPos >= 320 || nTileYPos <= -8 || nTileYPos >= 240)) {
						pTileData = (UINT32*)pSpriteData;
						pTile = pBurnBitmap + (nTileXPos * nBurnColumn) + (nTileYPos * nBurnRow);
						if (nTileXPos < 0 || nTileXPos > 312 || nTileYPos < 0 || nTileYPos > 232) {
							RenderTile[1]();
						} else {
							RenderTile[0]();
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
// BCU-2 functions

UINT8* BCU2ROM;
UINT32 nBCU2ROMSize;
static UINT8* BCU2TileAttrib;

static UINT32 nBCU2MaxTile;

UINT8* BCU2RAM;

UINT16 BCU2Reg[8];

UINT32 BCU2Pointer;

static ToaTile* pBCU2TileQueue[16];
static ToaTile* pBCU2TileQueueData = NULL;

INT32 nBCU2TileXOffset = 0, nBCU2TileYOffset = 0;

static void BCU2QueueLayer(UINT16* pTilemap, INT32 nXPos, INT32 nYPos)
{
	if (Hellfire) nYPos += 16;

	INT32 x, y;
	INT32 nTileRow, nTileColumn;
	UINT32 nTileNumber, nTileAttrib;

	for (y = 0; y < 31; y++) {

		nTileRow = (((nYPos >> 3) + y) << 7) & 0x1F80;

		for (x = 0; x < 41; x++) {
			nTileColumn = (((nXPos >> 3) + x) << 1) & 0x7E;
			nTileNumber = pTilemap[nTileRow + nTileColumn + 1];
			nTileAttrib = pTilemap[nTileRow + nTileColumn];

			if (!(nTileNumber & 0x8000) && (nTileAttrib & 0xF000)) {
				pBCU2TileQueue[nTileAttrib >> 12]->nTileAttrib = (nTileAttrib << 16) | nTileNumber;
				pBCU2TileQueue[nTileAttrib >> 12]->nTileXPos = (x << 3) - (nXPos & 7);
				pBCU2TileQueue[nTileAttrib >> 12]->nTileYPos = (y << 3) - (nYPos & 7);
				pBCU2TileQueue[nTileAttrib >> 12]++;
			}
		}
	}
}

static void BCU2PrepareTiles()
{
	for (INT32 nPriority = 0; nPriority < 16; nPriority++) {
		pBCU2TileQueue[nPriority] = &pBCU2TileQueueData[nPriority * 1280 * 4];
	}

	BCU2QueueLayer((UINT16*)(BCU2RAM + 0xC000), (BCU2Reg[6] >> 7) - nBCU2TileXOffset + nLayer3XOffset, (BCU2Reg[7] >> 7) - nBCU2TileYOffset + nLayer3YOffset);
	BCU2QueueLayer((UINT16*)(BCU2RAM + 0x8000), (BCU2Reg[4] >> 7) - nBCU2TileXOffset + nLayer2XOffset, (BCU2Reg[5] >> 7) - nBCU2TileYOffset + nLayer2YOffset);
	BCU2QueueLayer((UINT16*)(BCU2RAM + 0x4000), (BCU2Reg[2] >> 7) - nBCU2TileXOffset + nLayer1XOffset, (BCU2Reg[3] >> 7) - nBCU2TileYOffset + nLayer1YOffset);
	BCU2QueueLayer((UINT16*)(BCU2RAM + 0x0000), (BCU2Reg[0] >> 7) - nBCU2TileXOffset + nLayer0XOffset, (BCU2Reg[1] >> 7) - nBCU2TileYOffset + nLayer0YOffset);
}

static void BCU2RenderTileQueue(INT32 nPriority)
{
	UINT32 nTileNumber, nTileAttrib;
	UINT8* pTileStart;
	UINT8 nOpacity;

	pBCU2TileQueue[nPriority]->nTileAttrib = 0;
	pBCU2TileQueue[nPriority] = &pBCU2TileQueueData[nPriority * 1280 * 4];

	while ((nTileAttrib = pBCU2TileQueue[nPriority]->nTileAttrib) != 0) {
		nTileXPos = pBCU2TileQueue[nPriority]->nTileXPos;
		nTileYPos = pBCU2TileQueue[nPriority]->nTileYPos;
		pBCU2TileQueue[nPriority]++;

		nTileNumber = nTileAttrib & nBCU2MaxTile;

		pTileStart = BCU2ROM + (nTileNumber << 5);
		pTilePalette = &ToaPalette[(nTileAttrib >> 12) & 0x03F0];

		pTile = pBurnBitmap + (nTileXPos * nBurnColumn) + (nTileYPos * nBurnRow);
		
		if ((nOpacity = BCU2TileAttrib[nTileNumber]) != 0 || nPriority < ToaOpaquePriority) {
			if (nPriority < ToaOpaquePriority) nOpacity = 9;

			pTileData = (UINT32*)pTileStart;
			if (nTileXPos >= 0 && nTileXPos < 312 && nTileYPos >= 0 && nTileYPos < 232) {
				RenderTile[nOpacity - 1]();
			} else {
				if (nTileXPos > -8 && nTileXPos < 320 && nTileYPos > -8 && nTileYPos < 240) {
					RenderTile[nOpacity]();
				}
			}
		}
	}
}

void ToaBufferFCU2Sprites()
{
	memcpy(pFCU2SpriteBuffer, FCU2RAM, 0x0800);
}

INT32 ToaRenderBCU2()
{
	if (nLastBPP != nBurnBpp ) {
		nLastBPP = nBurnBpp;

#ifdef DRIVER_ROTATION
		if (bRotatedScreen) {
			RenderTile = RenderTile_ROT270[nBurnBpp - 2];
		} else {
			RenderTile = RenderTile_ROT0[nBurnBpp - 2];
		}
#else
		RenderTile = RenderTile_ROT0[nBurnBpp - 2];
#endif
	}

	BCU2PrepareTiles();
	if (!Rallybik) {
		FCU2PrepareSprites();
	}

	for (INT32 nPriority = 0; nPriority < 16; nPriority++) {
		if (Rallybik) {
			rallybik_draw_sprites(nPriority << 8);
		} else {
			FCU2RenderSpriteQueue(nPriority);
		}
		BCU2RenderTileQueue(nPriority);
	}

	return 0;
}

INT32 ToaInitBCU2()
{
	INT32 nSize;

	nLastBPP = 0;

	nBCU2MaxTile = (nBCU2ROMSize - 1) >> 5;

	nSize = 1280 * 4 * 0x10 * sizeof(ToaTile);
	pBCU2TileQueueData = (ToaTile*)BurnMalloc(nSize);
	memset(pBCU2TileQueueData, 0, nSize);

	BCU2TileAttrib = (UINT8*)BurnMalloc(32768);
	memset(BCU2TileAttrib, 0, 32768);
	for (UINT32 j = 0; j < (nBCU2ROMSize >> 5); j++) {
		bool bTransparent = true, bSolid = true;
		INT32 nTwoPixels;
		for (UINT32 k = (j << 5); k < ((j << 5) + 32); k++) {
			if ((nTwoPixels = BCU2ROM[k]) != 0) {
				bTransparent = false;
			}
			if ((nTwoPixels & 0xF0) == 0 || (nTwoPixels & 0x0F) == 0) {
				bSolid = false;
			}
		}
		if (bTransparent) {
			BCU2TileAttrib[j] = 0;
		} else {
			if (bSolid) {
				BCU2TileAttrib[j] = 9;
			} else {
				BCU2TileAttrib[j] = 1;
			}
		}
	}

	nFCU2MaxSprite = (nFCU2ROMSize - 1) >> 5;

	nSize = 0x10 * 0x101 * sizeof(UINT16*);
	pFCU2SpriteQueueData = (UINT16**)BurnMalloc(nSize);
	memset(pFCU2SpriteQueueData, 0, nSize);

	pFCU2SpriteBuffer = (UINT8*)BurnMalloc(0x0800);

	FCU2TileAttrib = (UINT8*)BurnMalloc(32768);
	memset(FCU2TileAttrib, 0, 32768);
	for (UINT32 j = 0; j < (nFCU2ROMSize >> 5); j++) {
		bool bTransparent = true, bSolid = true;
		INT32 nTwoPixels;
		for (UINT32 k = (j << 5); k < ((j << 5) + 32); k++) {
			if ((nTwoPixels = FCU2ROM[k]) != 0) {
				bTransparent = false;
			}
			if ((nTwoPixels & 0xF0) == 0 || (nTwoPixels & 0x0F) == 0) {
				bSolid = false;
			}
		}
		if (bTransparent) {
			FCU2TileAttrib[j] = 0;
		} else {
			if (bSolid) {
				FCU2TileAttrib[j] = 9;
			} else {
				FCU2TileAttrib[j] = 1;
			}
		}
	}

	if (!nLayer0XOffset) {
		nLayer0XOffset = 0x01EF + 6;
	}
	if (!nLayer1XOffset) {
		nLayer1XOffset = 0x01EF + 4;
	}
	if (!nLayer2XOffset) {
		nLayer2XOffset = 0x01EF + 2;
	}
	if (!nLayer3XOffset) {
		nLayer3XOffset = 0x01EF + 0;
	}

	if (!nLayer0YOffset) {
		nLayer0YOffset = 0x0101;
	}
	if (!nLayer1YOffset) {
		nLayer1YOffset = 0x0101;
	}
	if (!nLayer2YOffset) {
		nLayer2YOffset = 0x0101;
	}
	if (!nLayer3YOffset) {
		nLayer3YOffset = 0x0101;
	}

	ToaOpaquePriority = 0;

	return 0;
}

INT32 ToaExitBCU2()
{
	nLayer0XOffset = 0;
	nLayer1XOffset = 0;
	nLayer2XOffset = 0;
	nLayer3XOffset = 0;
	nLayer0YOffset = 0;
	nLayer1YOffset = 0;
	nLayer2YOffset = 0;
	nLayer3YOffset = 0;

	BurnFree(pBCU2TileQueueData);
	BurnFree(BCU2TileAttrib);
	BurnFree(pFCU2SpriteQueueData);
	BurnFree(pFCU2SpriteBuffer);
	BurnFree(FCU2TileAttrib);

	return 0;
}

