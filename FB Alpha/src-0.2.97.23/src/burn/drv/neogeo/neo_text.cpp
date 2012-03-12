#include "neogeo.h"

UINT8* NeoTextROMBIOS;
UINT8* NeoTextROM[MAX_SLOT];

INT32 nNeoTextROMSize[MAX_SLOT] = { 0, };
bool bBIOSTextROMEnabled;

static UINT8* NeoTextROMCurrent;

static INT8* NeoTextTileAttrib[MAX_SLOT] = { NULL, };
static INT8* NeoTextTileAttribBIOS = NULL;
static INT8* NeoTextTileAttribActive = NULL;
static INT32 nBankswitch[MAX_SLOT] = { 0, };

static INT32 nBankLookupAddress[40];
static INT32 nBankLookupShift[40];

static UINT8* pTile;
static UINT8* pTileData;
static UINT32* pTilePalette;

typedef void (*RenderTileFunction)();
static RenderTileFunction RenderTile;

static INT32 nLastBPP = 0;

static INT32 nMinX, nMaxX;

#define BPP 16
 #include "neo_text_render.h"
#undef BPP

#define BPP 24
 #include "neo_text_render.h"
#undef BPP

#define BPP 32
 #include "neo_text_render.h"
#undef BPP

INT32 NeoRenderText()
{
	INT32 x, y;
	UINT8* pTextROM;
	INT8* pTileAttrib;
	UINT8* pCurrentRow = pBurnDraw;
	UINT32* pTextPalette = NeoPalette;
	UINT32 nTileDown = nBurnPitch << 3;
	UINT32 nTileLeft = nBurnBpp << 3;
	UINT16* pTileRow = (UINT16*)(NeoGraphicsRAM + 0xE000);

	if (!(nBurnLayer & 2)) {
		return 0;
	}

	if (nLastBPP != nBurnBpp ) {
		nLastBPP = nBurnBpp;

		switch (nBurnBpp) {
			case 2:
				RenderTile = *RenderTile16;
				break;
			case 3:
				RenderTile = *RenderTile24;
				break;
			case 4:
				RenderTile = *RenderTile32;
				break;
			default:
				return 1;
		}
	}

	if (!bBIOSTextROMEnabled && nBankswitch[nNeoActiveSlot]) {

		if (!NeoTextROMCurrent) {
			return 0;
		}

		if (nBankswitch[nNeoActiveSlot] == 1) {

			// Garou, Metal Slug 3, Metal Slug 4

			INT32 nOffset[32];
			INT32 nBank = (3 << 12);
			INT32 z = 0;

			y = 0;
			while (y < 32) {
				if (*((UINT16*)(NeoGraphicsRAM + 0xEA00 + z)) == 0x0200 && (*((UINT16*)(NeoGraphicsRAM + 0xEB00 + z)) & 0xFF00) == 0xFF00) {
					nBank = ((*((UINT16*)(NeoGraphicsRAM + 0xEB00 + z)) & 3) ^ 3) << 12;
					nOffset[y++] = nBank;
				}
				nOffset[y++] = nBank;
				z += 4;
			}

			for (y = 2, pTileRow += 2; y < 30; y++, pCurrentRow += nTileDown, pTileRow++) {
				pTextROM    = NeoTextROMCurrent        + (nOffset[y - 2] << 5);
				pTileAttrib = NeoTextTileAttribActive +  nOffset[y - 2];
				for (x = nMinX, pTile = pCurrentRow; x < nMaxX; x++, pTile += nTileLeft) {
					UINT32 nTile = pTileRow[x << 5];
					INT32 nPalette = nTile & 0xF000;
					nTile &= 0x0FFF;
					if (pTileAttrib[nTile] == 0) {
						pTileData = pTextROM + (nTile << 5);
						pTilePalette = &pTextPalette[nPalette >> 8];
						RenderTile();
					}
				}
			}
		} else {

			// KOF2000

			UINT16* pBankInfo = (UINT16*)(NeoGraphicsRAM + 0xEA00) + 1;
			pTextROM    = NeoTextROMCurrent;
			pTileAttrib = NeoTextTileAttribActive;

			for (y = 2, pTileRow += 2; y < 30; y++, pCurrentRow += nTileDown, pTileRow++, pBankInfo++) {
 				for (x = nMinX, pTile = pCurrentRow; x < nMaxX; x++, pTile += nTileLeft) {
					UINT32 nTile = pTileRow[x << 5];
					INT32 nPalette = nTile & 0xF000;
					nTile &= 0x0FFF;
					nTile += (((pBankInfo[nBankLookupAddress[x]] >> nBankLookupShift[x]) & 3) ^ 3) << 12;
					if (pTileAttrib[nTile] == 0) {
						pTileData = pTextROM + (nTile << 5);
						pTilePalette = &pTextPalette[nPalette >> 8];
						RenderTile();
					}
				}
			}
		}
	} else {
		if (bBIOSTextROMEnabled) {
			pTextROM    = NeoTextROMBIOS;
			pTileAttrib = NeoTextTileAttribBIOS;
		} else {
			pTextROM    = NeoTextROMCurrent;
			pTileAttrib = NeoTextTileAttribActive;
		}
		if (!pTextROM) {
			return 0;
		}

		for (y = 2, pTileRow += 2; y < 30; y++, pCurrentRow += nTileDown, pTileRow++) {
			for (x = nMinX, pTile = pCurrentRow; x < nMaxX; x++, pTile += nTileLeft) {
				UINT32 nTile = pTileRow[x << 5];
				INT32 nPalette = nTile & 0xF000;
				nTile &= 0xFFF;
				if (pTileAttrib[nTile] == 0) {
					pTileData = pTextROM + (nTile << 5);
					pTilePalette = &pTextPalette[nPalette >> 8];
					RenderTile();
				}
			}
		}
	}

	return 0;
}

void NeoExitText(INT32 nSlot)
{
	BurnFree(NeoTextTileAttribBIOS);
	BurnFree(NeoTextTileAttrib[nSlot]);
	NeoTextTileAttribActive = NULL;
}

static void NeoUpdateTextAttribBIOS(INT32 nOffset, INT32 nSize)
{
	for (INT32 i = nOffset & ~31; i < nOffset + nSize; i += 32) {
		NeoTextTileAttribBIOS[i >> 5] = (((INT64*)NeoTextROMBIOS)[(i >> 3) + 0] ||
										 ((INT64*)NeoTextROMBIOS)[(i >> 3) + 1] ||
										 ((INT64*)NeoTextROMBIOS)[(i >> 3) + 2] ||
										 ((INT64*)NeoTextROMBIOS)[(i >> 3) + 3])
									  ? 0 : 1;
	}
}

static inline void NeoUpdateTextAttribOne(const INT32 nOffset)
{
	NeoTextTileAttribActive[nOffset >> 5] = 1;

	for (INT32 i = nOffset; i < nOffset + 32; i += 4) {
		if (*((UINT32*)(NeoTextROMCurrent + i))) {
			NeoTextTileAttribActive[nOffset >> 5] = 0;
			break;
		}
	}
}

static void NeoUpdateTextAttrib(INT32 nOffset, INT32 nSize)
{
	nOffset &= ~0x1F;

	for (INT32 i = nOffset; i < nOffset + nSize; i += 32) {
		NeoUpdateTextAttribOne(i);
	}
}

void NeoUpdateTextOne(INT32 nOffset, const UINT8 byteValue)
{
	nOffset = (nOffset & ~0x1F) | (((nOffset ^ 0x10) & 0x18) >> 3) | ((nOffset & 0x07) << 2);

	if (byteValue) {
		NeoTextTileAttribActive[nOffset >> 5] = 0;
	} else {
		if (NeoTextTileAttribActive[nOffset >> 5] == 0 && NeoTextROMCurrent[nOffset]) {
			NeoTextTileAttribActive[nOffset >> 5] = 1;
			NeoUpdateTextAttribOne(nOffset);
		}
	}

	NeoTextROMCurrent[nOffset] = byteValue;
}

static inline void NeoTextDecodeTile(const UINT8* pData, UINT8* pDest)
{
	UINT8 nBuffer[32];

	for (INT32 i = 0; i < 8; i++) {
		nBuffer[0 + i * 4] = pData[16 + i];
		nBuffer[1 + i * 4] = pData[24 + i];
		nBuffer[2 + i * 4] = pData[ 0 + i];
		nBuffer[3 + i * 4] = pData[ 8 + i];
	}

	for (INT32 i = 0; i < 32; i++) {
		pDest[i]  = nBuffer[i] << 4;
		pDest[i] |= nBuffer[i] >> 4;
	}
}

void NeoDecodeTextBIOS(INT32 nOffset, const INT32 nSize, UINT8* pData)
{
	UINT8* pEnd = pData + nSize;

	for (UINT8* pDest = NeoTextROMBIOS + (nOffset & ~0x1F); pData < pEnd; pData += 32, pDest += 32) {
		NeoTextDecodeTile(pData, pDest);
	}

//	if (NeoTextTileAttribBIOS) {
//		NeoUpdateTextAttribBIOS(0, nSize);
//	}	
}

void NeoDecodeText(INT32 nOffset, const INT32 nSize, UINT8* pData, UINT8* pDest)
{
	UINT8* pEnd = pData + nSize;

	for (pData += (nOffset & ~0x1F); pData < pEnd; pData += 32, pDest += 32) {
		NeoTextDecodeTile(pData, pDest);
	}
}

void NeoUpdateText(INT32 nOffset, const INT32 nSize, UINT8* pData, UINT8* pDest)
{
	NeoDecodeText(nOffset, nSize, pData, pDest);
	if (NeoTextTileAttribActive) {
		NeoUpdateTextAttrib((nOffset & ~0x1F), nSize);
	}	
}

void NeoSetTextSlot(INT32 nSlot)
{
	NeoTextROMCurrent       = NeoTextROM[nSlot];
	NeoTextTileAttribActive = NeoTextTileAttrib[nSlot];
}

INT32 NeoInitText(INT32 nSlot)
{
	if (nSlot < 0) {
		NeoTextTileAttribBIOS    = (INT8*)BurnMalloc(0x1000);
		for (INT32 i = 0; i < 0x1000; i++) {
			NeoTextTileAttribBIOS[i] = 1;
		}
		NeoUpdateTextAttribBIOS(0, 0x020000);

		return 0;
	}
		
	INT32 nTileNum = nNeoTextROMSize[nSlot] >> 5;

//	NeoExitText(nSlot);

	NeoTextTileAttrib[nSlot] = (INT8*)BurnMalloc((nTileNum < 0x1000) ? 0x1000 : nTileNum);

	if (nNeoScreenWidth == 304) {
		nMinX = 1;
		nMaxX = 39;
	} else {
		nMinX = 0;
		nMaxX = 40;
	}

	// Set up tile attributes

	NeoTextROMCurrent       = NeoTextROM[nSlot];
	NeoTextTileAttribActive = NeoTextTileAttrib[nSlot];
	for (INT32 i = 0; i < ((nTileNum < 0x1000) ? 0x1000 : nTileNum); i++) {
		NeoTextTileAttribActive[i] = 1;
	}
	NeoUpdateTextAttrib(0, nNeoTextROMSize[nSlot]);

	// Set up tile bankswitching

	nBankswitch[nSlot] = 0;
	if (nNeoTextROMSize[nSlot] > 0x040000) {
//		if (BurnDrvGetHardwareCode() & HARDWARE_SNK_CMC50) {
		if (BurnDrvGetHardwareCode() & HARDWARE_SNK_ALTERNATE_TEXT) {
			nBankswitch[nSlot] = 2;

			// Precompute lookup-tables
			for (INT32 x = nMinX; x < nMaxX; x++) {
				nBankLookupAddress[x] = (x / 6) << 5;
				nBankLookupShift[x] = (5 - (x % 6)) << 1;
			}

		} else {
			nBankswitch[nSlot] = 1;
		}
	}

	return 0;
}
