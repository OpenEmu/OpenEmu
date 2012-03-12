#include "neogeo.h"

UINT8* NeoZoomROM;

UINT8* NeoSpriteROM[MAX_SLOT] = { NULL, };

UINT32 nNeoTileMask[MAX_SLOT];
INT32 nNeoMaxTile[MAX_SLOT];

static UINT8* NeoSpriteROMActive;
static UINT32 nNeoTileMaskActive;
static INT32 nNeoMaxTileActive;

static UINT8* NeoTileAttrib[MAX_SLOT] = { NULL, };
static UINT8* NeoTileAttribActive;

INT32 nSliceStart, nSliceEnd, nSliceSize;

static UINT32* pTileData;
static UINT32* pTilePalette;

static UINT16* pBank;

static INT32 nBankSize;
static INT32 nBankXPos, nBankYPos;
static INT32 nBankXZoom, nBankYZoom;

static INT32 nNeoSpriteFrame04, nNeoSpriteFrame08;

static INT32 nLastBPP = -1;

typedef void (*RenderBankFunction)();
static RenderBankFunction* RenderBank;

static 	UINT16 BankAttrib01, BankAttrib02, BankAttrib03;


// Include the tile rendering functions
#include "neo_sprite_func.h"

INT32 NeoRenderSprites()
{
	if (nLastBPP != nBurnBpp ) {
		nLastBPP = nBurnBpp;

		RenderBank = RenderBankNormal[nBurnBpp - 2];
	}

	if (!NeoSpriteROMActive || !(nBurnLayer & 1)) {
		return 0;
	}

//	UINT16 BankAttrib01, BankAttrib02, BankAttrib03;

	nNeoSpriteFrame04 = nNeoSpriteFrame & 3;
	nNeoSpriteFrame08 = nNeoSpriteFrame & 7;
	
	// ssrpg hack! - NeoCD/SDL
	INT32 nStart = 0;
	if (SekReadWord(0x108) == 0x0085) {
		UINT16 *vidram = (UINT16*)NeoGraphicsRAM;

	   	if ((vidram[0x8202] & 0x40) == 0 && (vidram[0x8203] & 0x40) != 0) {
			nStart = 3;

			while ((vidram[0x8200 + nStart] & 0x40) != 0) nStart++;

			if (nStart == 3) nStart = 0;
		}
	}

	for (INT32 nBank = 0; nBank < 0x17D; nBank++) {
		INT32 zBank = (nBank + nStart) % 0x17d;
		BankAttrib01 = *((UINT16*)(NeoGraphicsRAM + 0x010000 + (zBank << 1)));
		BankAttrib02 = *((UINT16*)(NeoGraphicsRAM + 0x010400 + (zBank << 1)));
		BankAttrib03 = *((UINT16*)(NeoGraphicsRAM + 0x010800 + (zBank << 1)));

		pBank = (UINT16*)(NeoGraphicsRAM + (zBank << 7));

		if (BankAttrib02 & 0x40) {
			nBankXPos += nBankXZoom + 1;
		} else {
			nBankYPos = (0x0200 - (BankAttrib02 >> 7)) & 0x01FF;
			nBankXPos = (BankAttrib03 >> 7);
			if (nNeoScreenWidth == 304) {
				nBankXPos -= 8;
			}

			nBankYZoom = BankAttrib01 & 0xFF;
			nBankSize  = BankAttrib02 & 0x3F;

//			if (nBankSize > 0x10 && nSliceStart == 0x10) bprintf(PRINT_NORMAL, _T("bank: %04X, x: %04X, y: %04X, zoom: %02X, size: %02X.\n"), zBank, nBankXPos, nBankYPos, nBankYZoom, nBankSize);
		}

		if (nBankSize) {

			nBankXZoom = (BankAttrib01 >> 8) & 0x0F;
			if (nBankXPos >= 0x01E0) {
				nBankXPos -= 0x200;
			}

			if (nBankXPos >= 0 && nBankXPos < (nNeoScreenWidth - nBankXZoom - 1)) {
				RenderBank[nBankXZoom]();
			} else {
				if (nBankXPos >= -nBankXZoom && nBankXPos < nNeoScreenWidth) {
					RenderBank[nBankXZoom + 16]();
				}
			}
		}
	}

//	bprintf(PRINT_NORMAL, _T("\n"));

	return 0;
}

void NeoUpdateSprites(INT32 nOffset, INT32 nSize)
{
	for (INT32 i = nOffset & ~127; i < nOffset + nSize; i += 128) {
		bool bTransparent = true;
		for (INT32 j = i; j < i + 128; j++) {
			if (NeoSpriteROMActive[j]) {
				bTransparent = false;
				break;
			}
		}
		if (bTransparent) {
			NeoTileAttribActive[i >> 7] = 1;
		} else {
			NeoTileAttribActive[i >> 7] = 0;
		}
	}
}

void NeoSetSpriteSlot(INT32 nSlot)
{
	NeoTileAttribActive = NeoTileAttrib[nSlot];
	NeoSpriteROMActive  = NeoSpriteROM[nSlot];
	nNeoTileMaskActive  = nNeoTileMask[nSlot];
	nNeoMaxTileActive   = nNeoMaxTile[nSlot];
}

INT32 NeoInitSprites(INT32 nSlot)
{
	// Create a table that indicates if a tile is transparent
	NeoTileAttrib[nSlot] = (UINT8*)BurnMalloc(nNeoTileMask[nSlot] + 1);

	for (INT32 i = 0; i < nNeoMaxTile[nSlot]; i++) {
		bool bTransparent = true;
		for (INT32 j = i << 7; j < (i + 1) << 7; j++) {
			if (NeoSpriteROM[nSlot][j]) {
				bTransparent = false;
				break;
			}
		}
		if (bTransparent) {
			NeoTileAttrib[nSlot][i] = 1;
		} else {
			NeoTileAttrib[nSlot][i] = 0;
		}
	}
	for (UINT32 i = nNeoMaxTile[nSlot]; i < nNeoTileMask[nSlot] + 1; i++) {
		NeoTileAttrib[nSlot][i] = 1;
	}

	NeoTileAttribActive = NeoTileAttrib[nSlot];
	NeoSpriteROMActive  = NeoSpriteROM[nSlot];
	nNeoTileMaskActive  = nNeoTileMask[nSlot];
	nNeoMaxTileActive   = nNeoMaxTile[nSlot];

	return 0;
}

void NeoExitSprites(INT32 nSlot)
{
	BurnFree(NeoTileAttrib[nSlot]);
	NeoTileAttribActive = NULL;
}
