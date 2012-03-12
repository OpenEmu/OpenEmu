#include "toaplan.h"

/*
 * Some games write to addresses outside of VRAM. It looks like the hardware masks bit 15-12.
 * Note that they only seem to set the VRAM pointer too high (tekipaki does read/write too far
 * during POST -- 0x3800 = 2 * 0x1C00). Batsugun in particular sets the pointer too high often.
 *
 * Some games need 1 to be added to the sprite priorities (e.g the 2nd level boss in mahoudai).
 * This is clearly wrong for some other games.
 *
 * For batsugun, controller 1 has higher priority then controller 0.
 * Tiles with priority 0 are special -- they are always in the background.
 * How to determine which controller has the highest priority is unknown.
 * The code to accomplish this for Batsugun is a hack that checks the X positions of the layers.
 *
 */

UINT8* GP9001ROM[2];
UINT32 nGP9001ROMSize[2];
static UINT8* GP9001TileAttrib[2];

static UINT32 nMaxTile[2];
static UINT32 nMaxSprite[2];

static INT32 nControllers;
static INT32 nMode;

UINT32 GP9001TileBank[8];

UINT8* GP9001RAM[2];
UINT16* GP9001Reg[2];

UINT8* GP9001Pointer[2];
INT32 GP9001Regnum[2];

static UINT32* pTileQueue[32];
static UINT32* pTileQueueData[2] = {NULL, };

static UINT8** pSpriteQueue[32];
static UINT8** pSpriteQueueData[2] = {NULL, };

static UINT8* pSpriteBuffer[2];
static UINT8* pSpriteBufferData[2] = {NULL, };

static INT32 nSpriteBuffer = 0;

INT32 nSpriteXOffset = 0, nSpriteYOffset = 0, nSpritePriority = 0;

INT32 nLayer0XOffset = 0, nLayer0YOffset = 0;
INT32 nLayer1XOffset = 0, nLayer1YOffset = 0;
INT32 nLayer2XOffset = 0, nLayer2YOffset = 0;
INT32 nLayer3XOffset = 0, nLayer3YOffset = 0;

static UINT8* pTile;
static UINT32* pTileData;
static UINT32* pTilePalette;

typedef void (*RenderTileFunction)();
static RenderTileFunction* RenderTile;

static INT32 nTileXPos, nTileYPos;

static INT32 nLastBPP = 0;

// Include the tile rendering functions
#include "toa_gp9001_func.h"

static void PrepareSprites()
{
	UINT8* pSpriteInfo;
	INT32 nSprite, nAttrib;

	for (INT32 i = 0; i < nControllers; i++) {
		UINT8*** pMySpriteQueue = &pSpriteQueue[i << 4];

		for (INT32 nPriority = 0; nPriority < 16; nPriority++) {
			pMySpriteQueue[nPriority] = &pSpriteQueueData[i][(nPriority << 8) + nPriority];
		}

		for (nSprite = 0, pSpriteInfo = pSpriteBuffer[i]; nSprite < 0x0100; nSprite++, pSpriteInfo += 8) {
			nAttrib = pSpriteInfo[1];
			if (nAttrib & 0x80) {				// Sprite is enabled
				*pMySpriteQueue[nAttrib & 0x0F]++ = pSpriteInfo;
			}
		}
	}
}

static void RenderSpriteQueue(INT32 i, INT32 nPriority)
{
	UINT8* pSpriteInfo;
	UINT8* pSpriteData;
	INT32 nSpriteXPos, nSpriteYPos;
	INT32 nSpriteXSize, nSpriteYSize;
	UINT32 nSpriteNumber;
	INT32 x, y, xoff, yoff;
	INT32 nFlip;

	UINT8*** pMySpriteQueue = &pSpriteQueue[i << 4];

	*pMySpriteQueue[nPriority] = NULL;
	pMySpriteQueue[nPriority] = &pSpriteQueueData[i][(nPriority << 8) + nPriority];

	while ((pSpriteInfo = *pMySpriteQueue[nPriority]++) != NULL) {
		nFlip = ((pSpriteInfo[1] & 0x30) >> 3);

		pTilePalette = &ToaPalette[((pSpriteInfo[0] & 0xFC) << 2)];
		nSpriteNumber = (((pSpriteInfo[3] << 8) | pSpriteInfo[2]) & 0x7FFF);
		nSpriteNumber += GP9001TileBank[(((pSpriteInfo[0] & 3) << 1) | (pSpriteInfo[3] >> 7))];

		pSpriteData = GP9001ROM[i] + (nSpriteNumber << 5);

		nSpriteXSize = pSpriteInfo[4] & 0x0F;
		nSpriteXPos = ((pSpriteInfo[5] << 1) | (pSpriteInfo[4] >> 7)) + GP9001Reg[i][6] + nSpriteXOffset;
		nSpriteXPos &= 0x01FF;
		nSpriteYSize = pSpriteInfo[6] & 0x0F;
		nSpriteYPos = ((pSpriteInfo[7] << 1) | (pSpriteInfo[6] >> 7)) + GP9001Reg[i][7] + nSpriteYOffset;
		nSpriteYPos &= 0x01FF;

		if (nFlip & 2) {
			xoff = -8;
			nSpriteXPos -= 7;
			if (nSpriteXPos > (320 + 128)) {
				nSpriteXPos -= 0x0200;
			}
		} else {
			xoff = 8;
			if (nSpriteXPos > (512 - 128)) {
				nSpriteXPos -= 0x0200;
			}
		}
		if (nFlip & 4) {
			yoff = -8;
			nSpriteYPos -= 7;
		} else {
			yoff = 8;
		}

		if (nSpriteYPos > 384) {
			nSpriteYPos -= 0x0200;
		}

		for (y = 0, nTileYPos = nSpriteYPos; y <= nSpriteYSize; y++, nTileYPos += yoff) {
			for (x = 0, nTileXPos = nSpriteXPos; x <= nSpriteXSize; x++, nTileXPos += xoff, nSpriteNumber++, pSpriteData += 32) {
				if (nSpriteNumber > nMaxSprite[i]) {
					break;
				}
				if (GP9001TileAttrib[i][nSpriteNumber]) {
					// Skip tile if it's completely off the screen
					if (!(nTileXPos <= -8 || nTileXPos >= 320 || nTileYPos <= -8 || nTileYPos >= 240)) {
						pTileData = (UINT32*)pSpriteData;
						pTile = pBurnBitmap + (nTileXPos * nBurnColumn) + (nTileYPos * nBurnRow);
						if (nTileXPos < 0 || nTileXPos > 312 || nTileYPos < 0 || nTileYPos > 232) {
							RenderTile[nFlip + 1]();
						} else {
							RenderTile[nFlip]();
						}
					}
				}
			}
		}
	}
}

static void QueueLayer(INT32 i, UINT16* pTilemap, INT32 nXPos, INT32 nYPos)
{
	INT32 x, y;
	INT32 nTileRow, nTileColumn;
	UINT32 nTileNumber, nTileAttrib;

	UINT32** pMyTileQueue = &pTileQueue[i << 4];

	for (y = 0; y < 16; y++) {

		nTileRow = (((nYPos >> 4) + y) << 6) & 0x7C0;

		for (x = 0; x < 21; x++) {
			nTileColumn = (((nXPos >> 4) + x) << 1) & 0x3E;
			nTileNumber = BURN_ENDIAN_SWAP_INT16(pTilemap[nTileRow + nTileColumn + 1]);

			if (nTileNumber > 0 && nTileNumber <= nMaxTile[i]) {
				nTileAttrib = BURN_ENDIAN_SWAP_INT16(pTilemap[nTileRow + nTileColumn]);
				*pMyTileQueue[(nTileAttrib >> 8) & 0x0F]++ = (nTileAttrib << 16) | nTileNumber;
				nTileXPos = (x << 4) - (nXPos & 15);
				nTileYPos = (y << 4) - (nYPos & 15);
				*pMyTileQueue[(nTileAttrib >> 8) & 0x0F]++ = (nTileXPos << 16) | (nTileYPos & 0xFFFF);
#if 0
				printf("%X ", nTileAttrib >> 8);
#endif
			}
		}
	}
}

static void QueueLayer2(INT32 i, UINT16* pTilemap, INT32 nXPos, INT32 nYPos)
{
	INT32 x, y;
	INT32 nTileRow, nTileColumn;
	UINT32 nTileNumber, nTileAttrib;

	UINT32** pMyTileQueue = &pTileQueue[i << 4];

	for (y = 0; y < 16; y++) {

		nTileRow = (((nYPos >> 4) + y) << 6) & 0x7C0;

		for (x = 0; x < 21; x++) {
			nTileColumn = (((nXPos >> 4) + x) << 1) & 0x3E;
			nTileNumber = BURN_ENDIAN_SWAP_INT16(pTilemap[nTileRow + nTileColumn + 1]);

			if (nTileNumber > 0 && nTileNumber <= nMaxTile[i]) {
				nTileAttrib = BURN_ENDIAN_SWAP_INT16(pTilemap[nTileRow + nTileColumn]);
				if ((nTileAttrib & 0x0F00) == 0) {
					nTileAttrib |= 0x0100;
				}
				*pMyTileQueue[(nTileAttrib >> 8) & 0x0F]++ = (nTileAttrib << 16) | nTileNumber;
				nTileXPos = (x << 4) - (nXPos & 15);
				nTileYPos = (y << 4) - (nYPos & 15);
				*pMyTileQueue[(nTileAttrib >> 8) & 0x0F]++ = (nTileXPos << 16) | (nTileYPos & 0xFFFF);
			}
		}
	}
}

static void PrepareTiles()
{
	for (INT32 i = 0; i < nControllers; i++) {
		UINT32** pMyTileQueue = &pTileQueue[i << 4];

		for (INT32 nPriority = 0; nPriority < 16; nPriority++) {
			pMyTileQueue[nPriority] = &pTileQueueData[i][nPriority * 512 * 3 * 2];
		}
	}

	if (nControllers == 1) {
		QueueLayer(0, (UINT16*)(GP9001RAM[0] + 0x0000), GP9001Reg[0][0] + nLayer0XOffset, GP9001Reg[0][1] + nLayer0YOffset);
		QueueLayer(0, (UINT16*)(GP9001RAM[0] + 0x1000), GP9001Reg[0][2] + nLayer1XOffset, GP9001Reg[0][3] + nLayer1YOffset);
		QueueLayer(0, (UINT16*)(GP9001RAM[0] + 0x2000), GP9001Reg[0][4] + nLayer2XOffset, GP9001Reg[0][5] + nLayer2YOffset);
	} else {
		if (nMode == 2) {
			QueueLayer(0, (UINT16*)(GP9001RAM[0] + 0x0000), GP9001Reg[0][0] + nLayer0XOffset, GP9001Reg[0][1] + nLayer0YOffset);
			QueueLayer(0, (UINT16*)(GP9001RAM[0] + 0x1000), GP9001Reg[0][2] + nLayer1XOffset, GP9001Reg[0][3] + nLayer1YOffset);
			QueueLayer(0, (UINT16*)(GP9001RAM[0] + 0x2000), GP9001Reg[0][4] + nLayer2XOffset, GP9001Reg[0][5] + nLayer2YOffset);
			QueueLayer(1, (UINT16*)(GP9001RAM[1] + 0x0000), GP9001Reg[1][0] + nLayer0XOffset, GP9001Reg[1][1] + nLayer0YOffset);
			QueueLayer(1, (UINT16*)(GP9001RAM[1] + 0x1000), GP9001Reg[1][2] + nLayer1XOffset, GP9001Reg[1][3] + nLayer1YOffset);
			QueueLayer(1, (UINT16*)(GP9001RAM[1] + 0x2000), GP9001Reg[1][4] + nLayer2XOffset, GP9001Reg[1][5] + nLayer2YOffset);
		} else {
			QueueLayer(0, (UINT16*)(GP9001RAM[0] + 0x0000), GP9001Reg[0][0] + nLayer0XOffset, GP9001Reg[0][1] + nLayer0YOffset);
			QueueLayer(0, (UINT16*)(GP9001RAM[0] + 0x1000), GP9001Reg[0][2] + nLayer1XOffset, GP9001Reg[0][3] + nLayer1YOffset);
			if (GP9001Reg[0][4] + nLayer2XOffset == 0) {
				QueueLayer(0, (UINT16*)(GP9001RAM[0] + 0x2000), GP9001Reg[0][4] + nLayer2XOffset, GP9001Reg[0][5] + nLayer2YOffset);
			} else {
				QueueLayer2(0, (UINT16*)(GP9001RAM[0] + 0x2000), GP9001Reg[0][4] + nLayer2XOffset, GP9001Reg[0][5] + nLayer2YOffset);
			}
			QueueLayer(1, (UINT16*)(GP9001RAM[1] + 0x0000), GP9001Reg[1][0] + nLayer0XOffset, GP9001Reg[1][1] + nLayer0YOffset);
			QueueLayer(1, (UINT16*)(GP9001RAM[1] + 0x1000), GP9001Reg[1][2] + nLayer1XOffset, GP9001Reg[1][3] + nLayer1YOffset);
			QueueLayer(1, (UINT16*)(GP9001RAM[1] + 0x2000), GP9001Reg[1][4] + nLayer2XOffset, GP9001Reg[1][5] + nLayer2YOffset);
		}
	}
}

static void RenderTileQueue(INT32 i, INT32 nPriority)
{
	UINT32 nTileNumber, nTileAttrib;
	UINT8* pTileStart;
	UINT8 nOpacity;

	UINT32** pMyTileQueue = &pTileQueue[i << 4];

	*pMyTileQueue[nPriority] = 0;
	pMyTileQueue[nPriority] = &pTileQueueData[i][nPriority * 512 * 3 * 2];

	while ((nTileNumber = *pMyTileQueue[nPriority]++) != 0) {
		nTileXPos = (INT16)(*pMyTileQueue[nPriority] >> 16);
		nTileYPos = (INT16)(*pMyTileQueue[nPriority]++ & 0xFFFF);
		nTileAttrib = nTileNumber;
		nTileNumber = ((nTileNumber & 0x1FFF) << 2) + GP9001TileBank[(nTileNumber >> 13) & 7];

		pTileStart = GP9001ROM[i] + (nTileNumber << 5);
		pTilePalette = &ToaPalette[(nTileAttrib >> 12) & 0x07F0];

		if (nTileXPos >= 0 && nTileXPos < 304 && nTileYPos >= 0 && nTileYPos < 224) {
			INT32 nTileWidth = 8 * nBurnColumn;
			pTile = pBurnBitmap + (nTileXPos * nBurnColumn) + (nTileYPos * nBurnRow);

			if ((nOpacity = GP9001TileAttrib[i][nTileNumber]) != 0) {
				pTileData = (UINT32*)pTileStart;
				RenderTile[nOpacity - 1]();
			}
			if ((nOpacity = GP9001TileAttrib[i][nTileNumber + 1]) != 0) {
				pTile += nTileWidth;
				pTileData = (UINT32*)(pTileStart + 32);
				RenderTile[nOpacity - 1]();
				pTile -= nTileWidth;
			}
			pTile += 8 * nBurnRow;
			if ((nOpacity = GP9001TileAttrib[i][nTileNumber + 2]) != 0) {
				pTileData = (UINT32*)(pTileStart + 64);
				RenderTile[nOpacity - 1]();
			}
			if ((nOpacity = GP9001TileAttrib[i][nTileNumber + 3]) != 0) {
				pTile += nTileWidth;
				pTileData = (UINT32*)(pTileStart + 96);
				RenderTile[nOpacity - 1]();
			}
		} else {
			INT32 nTileWidth = 8 * nBurnColumn;
			pTile = pBurnBitmap + (nTileXPos * nBurnColumn) + (nTileYPos * nBurnRow);

			if ((nOpacity = GP9001TileAttrib[i][nTileNumber]) != 0) {
				if (nTileXPos > -8 && nTileXPos < 320 && nTileYPos > -8 && nTileYPos < 240) {
					pTileData = (UINT32*)pTileStart;
					if (nTileXPos > 0 && nTileXPos <= 312 && nTileYPos > 0 && nTileYPos <= 232) {
						RenderTile[nOpacity - 1]();
					} else {
						RenderTile[nOpacity]();
					}
				}
			}
			if ((nOpacity = GP9001TileAttrib[i][nTileNumber + 1]) != 0) {
				pTile += nTileWidth;
				nTileXPos += 8;
				if (nTileXPos > -8 && nTileXPos < 320 && nTileYPos > -8 && nTileYPos < 240) {
					pTileData = (UINT32*)(pTileStart + 32);
					if (nTileXPos > 0 && nTileXPos <= 312 && nTileYPos > 0 && nTileYPos <= 232) {
						RenderTile[nOpacity - 1]();
					} else {
						RenderTile[nOpacity]();
					}
				}
				nTileXPos -= 8;
				pTile -= nTileWidth;
			}
			nTileYPos += 8;
			pTile += 8 * nBurnRow;
			if ((nOpacity = GP9001TileAttrib[i][nTileNumber + 2]) != 0) {
				if (nTileXPos > -8 && nTileXPos < 320 && nTileYPos > -8 && nTileYPos < 240) {
					pTileData = (UINT32*)(pTileStart + 64);
					if (nTileXPos > 0 && nTileXPos <= 312 && nTileYPos > 0 && nTileYPos <= 232) {
						RenderTile[nOpacity - 1]();
					} else {
						RenderTile[nOpacity]();
					}
				}
			}
			if ((nOpacity = GP9001TileAttrib[i][nTileNumber + 3]) != 0) {
				nTileXPos += 8;
				pTile += nTileWidth;
				if (nTileXPos > -8 && nTileXPos < 320 && nTileYPos > -8 && nTileYPos < 240) {
					pTileData = (UINT32*)(pTileStart + 96);
					if (nTileXPos > 0 && nTileXPos <= 312 && nTileYPos > 0 && nTileYPos <= 232) {
						RenderTile[nOpacity - 1]();
					} else {
						RenderTile[nOpacity]();
					}
				}
			}
		}
	}
}

INT32 ToaBufferGP9001Sprites()
{

#if 0
	pSpriteBuffer[0] = GP9001RAM[0] + 0x3000;
	if (nControllers > 1) {
		pSpriteBuffer[1] = GP9001RAM[1] + 0x3000;
	}
#else

	pSpriteBuffer[0] = pSpriteBufferData[0] + 0x0800 * nSpriteBuffer;
	if (nControllers > 1) {
		pSpriteBuffer[1] = pSpriteBufferData[1] + 0x0800 * nSpriteBuffer;
	}

	nSpriteBuffer ^= 1;

	memcpy(pSpriteBufferData[0] + 0x0800 * nSpriteBuffer, GP9001RAM[0] + 0x3000, 0x0800);
	if (nControllers > 1) {
		memcpy(pSpriteBufferData[1] + 0x0800 * nSpriteBuffer, GP9001RAM[1] + 0x3000, 0x0800);
	}
#endif

	return 0;
}

INT32 ToaRenderGP9001()
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

	PrepareTiles();
	PrepareSprites();

	if (nControllers > 1) {
		if (nMode == 2) {						// Dogyuun
			for (INT32 nPriority = 0; nPriority < 16; nPriority++) {
				RenderTileQueue(1, nPriority);
				RenderSpriteQueue(1, nPriority);

			}
			for (INT32 nPriority = 0; nPriority < 16; nPriority++) {
				RenderTileQueue(0, nPriority);
				RenderSpriteQueue(0, nPriority);

			}
		} else {								// Batsugun
//			printf("%04X %04X %04X\n", GP9001Reg[1][0], GP9001Reg[1][2], GP9001Reg[1][4]);

			if ((((GP9001Reg[1][0] + nLayer0XOffset) && (GP9001Reg[1][2] + nLayer1XOffset)) || (GP9001Reg[0][4] + nLayer1XOffset < 0)) && (GP9001Reg[1][4] + nLayer2XOffset == 0)) {
				RenderTileQueue(0, 0);
				RenderTileQueue(1, 0);
			} else {
				RenderTileQueue(1, 0);
				RenderTileQueue(0, 0);
			}

			for (INT32 nPriority = 1; nPriority < 16; nPriority++) {
				RenderTileQueue(0, nPriority);
				if (nPriority < 4) {
					RenderTileQueue(1, nPriority);
				}
				RenderSpriteQueue(0, nPriority - 1);
			}
			RenderSpriteQueue(0, 15);

			RenderSpriteQueue(1, 0);
			for (INT32 nPriority = 1; nPriority < 16; nPriority++) {
				if (nPriority >= 4) {
					RenderTileQueue(1, nPriority);
				}
				RenderSpriteQueue(1, nPriority);
			}
		}
	} else {
		if (nSpritePriority) {
			for (INT32 nPriority = 0; nPriority < nSpritePriority; nPriority++) {
				RenderTileQueue(0, nPriority);
			}
		}
		for (INT32 nPriority = nSpritePriority; nPriority < 16; nPriority++) {
			RenderTileQueue(0, nPriority );
			RenderSpriteQueue(0, nPriority - nSpritePriority);
		}
		if (nSpritePriority) {
			for (INT32 nPriority = 16 - nSpritePriority; nPriority < 16; nPriority++) {
				RenderSpriteQueue(0, nPriority);
			}
		}
	}

	return 0;
}

INT32 ToaInitGP9001(INT32 n)
{
	INT32 nSize;

	if (n < 1 || n > 3) {
		return 1;
	} else {
		nMode = n;
	}

	if (n > 1) {
		nControllers = 2;
	} else {
		nControllers = 1;
	}

	nLastBPP = 0;

	for (INT32 i = 0; i < nControllers; i++) {

		nMaxTile[i] = (nGP9001ROMSize[i] - 1) >> 7;
		nMaxSprite[i] = (nGP9001ROMSize[i] - 1) >> 5;

		nSize = 512 * 3 * 0x10 * 2 * sizeof(INT32);
		pTileQueueData[i] = (UINT32*)BurnMalloc(nSize);
		memset(pTileQueueData[i], 0, nSize);

		nSize = 0x10 * 0x101 * sizeof(UINT8*);
		pSpriteQueueData[i] = (UINT8**)BurnMalloc(nSize);
		memset(pSpriteQueueData[i], 0, nSize);

		nSize = 0x0800 * 2;
		pSpriteBufferData[i] = (UINT8*)BurnMalloc(nSize);
		memset(pSpriteBufferData[i], 0, nSize);

		GP9001TileAttrib[i] = (UINT8*)BurnMalloc(nGP9001ROMSize[i] >> 5);
		for (UINT32 j = 0; j < (nGP9001ROMSize[i] >> 5); j++) {
			bool bTransparent = true, bSolid = true;
			INT32 nTwoPixels;
			for (UINT32 k = (j << 5); k < ((j << 5) + 32); k++) {
				if ((nTwoPixels = GP9001ROM[i][k]) != 0) {
					bTransparent = false;
				}
				if ((nTwoPixels & 0xF0) == 0 || (nTwoPixels & 0x0F) == 0) {
					bSolid = false;
				}
			}
			if (bTransparent) {
				GP9001TileAttrib[i][j] = 0;
			} else {
				if (bSolid) {
					GP9001TileAttrib[i][j] = 9;
				} else {
					GP9001TileAttrib[i][j] = 1;
				}
			}
		}
	}

	// Mark the rubbish tiles that appear on level 1 of Batsugun transparent
	if (nMode == 3) {
		for (INT32 i = 0; i < 16; i++) {
			GP9001TileAttrib[1][(0x225C << 2) + i] = 0;
		}
	}

	for (INT32 i = 0; i < 8; i++) {
		GP9001TileBank[i] = i << 15;
	}

	nSpriteBuffer = 0;

	ToaBufferGP9001Sprites();
	ToaBufferGP9001Sprites();

	if (!nSpriteXOffset){
		nSpriteXOffset = 0x024;
	}
	if (!nSpriteYOffset){
		nSpriteYOffset = -0x01;
	}

	if (!nLayer0XOffset) {
		nLayer0XOffset = -0x01D5;
	}
	if (!nLayer1XOffset) {
		nLayer1XOffset = -0x01D7;
	}
	if (!nLayer2XOffset) {
		nLayer2XOffset = -0x01D9;
	}

	if (!nLayer0YOffset) {
		nLayer0YOffset = -0x01EF;
	}
	if (!nLayer1YOffset) {
		nLayer1YOffset = -0x01EF;
	}
	if (!nLayer2YOffset) {
		nLayer2YOffset = -0x01EF;
	}

	return 0;
}

INT32 ToaExitGP9001()
{
	nSpriteXOffset = 0;
	nSpriteYOffset = 0;
	nSpritePriority = 0;

	nLayer0XOffset = 0;
	nLayer1XOffset = 0;
	nLayer2XOffset = 0;
	nLayer0YOffset = 0;
	nLayer1YOffset = 0;
	nLayer2YOffset = 0;

	for (INT32 i = 0; i < nControllers; i++) {
		BurnFree(pSpriteBufferData[i]);
		BurnFree(pSpriteQueueData[i]);
		BurnFree(pTileQueueData[i]);
		BurnFree(GP9001TileAttrib[i]);
	}

	return 0;
}

INT32 ToaScanGP9001(INT32 nAction, INT32* pnMin)
{
	if (nAction & ACB_VOLATILE) {		// Scan volatile data

		if (pnMin) {
			*pnMin = 0x029496;
		}

		SCAN_VAR(nSpriteBuffer);

		SCAN_VAR(GP9001Pointer);
		SCAN_VAR(GP9001Regnum);
		SCAN_VAR(GP9001TileBank);
	}

	return 0;
}
