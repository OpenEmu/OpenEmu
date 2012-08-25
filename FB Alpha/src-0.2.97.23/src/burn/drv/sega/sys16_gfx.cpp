#include "sys16.h"

INT32 System16SpriteBanks[16];
INT32 System16TileBanks[8];
INT32 System16OldTileBanks[8];
INT32 System16Page[4];
INT32 System16OldPage[4];
UINT8 BootlegFgPage[4];
UINT8 BootlegBgPage[4];
INT32 System16ScrollX[4] = {0, 0, 0, 0};
INT32 System16ScrollY[4] = {0, 0, 0, 0};
INT32 System16VideoEnable;
INT32 System18VdpEnable;
INT32 System18VdpMixing;
INT32 System16ScreenFlip;
INT32 System16SpriteShadow;
INT32 System16SpriteXOffset = 0;
INT32 System16ColScroll = 0;
INT32 System16RowScroll = 0;
INT32 System16RoadControl = 0;
INT32 System16RoadColorOffset1 = 0;
INT32 System16RoadColorOffset2 = 0;
INT32 System16RoadColorOffset3 = 0;
INT32 System16RoadXOffset = 0;
INT32 System16RoadPriority = 0;
INT32 System16PaletteEntries = 0;
INT32 System16TilemapColorOffset = 0;
INT32 System16TileBankSize = 0;
INT32 System16RecalcBgTileMap = 0;
INT32 System16RecalcBgAltTileMap = 0;
INT32 System16RecalcFgTileMap = 0;
INT32 System16RecalcFgAltTileMap = 0;
INT32 System16CreateOpaqueTileMaps = 0;
INT32 System16IgnoreVideoEnable = 0;

bool bSystem16BootlegRender;

UINT16 *pTempDraw = NULL;
static UINT16 *pSys16BgTileMapOpaque = NULL;
static UINT16 *pSys16BgAltTileMapOpaque = NULL;
static UINT16 *pSys16BgTileMapPri0 = NULL;
static UINT16 *pSys16BgTileMapPri1 = NULL;
static UINT16 *pSys16FgTileMapPri0 = NULL;
static UINT16 *pSys16FgTileMapPri1 = NULL;
static UINT16 *pSys16BgAltTileMapPri0 = NULL;
static UINT16 *pSys16BgAltTileMapPri1 = NULL;
static UINT16 *pSys16FgAltTileMapPri0 = NULL;
static UINT16 *pSys16FgAltTileMapPri1 = NULL;

/*====================================================
Scan Function
====================================================*/

void System16GfxScan(INT32 nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		if (nAction & ACB_WRITE) {		
			if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM16A) || ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_HANGON))  {
				System16RecalcBgTileMap = 1;
				System16RecalcFgTileMap = 1;
			}
		
			if (((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM16B) || ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEM18) || ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_OUTRUN) || ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_SYSTEMX)) {
				System16RecalcFgAltTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcBgTileMap = 1;
				System16RecalcBgAltTileMap = 1;
				System16RecalcFgTileMap = 1;
				System16RecalcFgAltTileMap = 1;
			}
		}
	}
}

/*====================================================
Decode Functions
====================================================*/

void System16Decode8x8Tiles(UINT8 *pTile, INT32 Num, INT32 offs1, INT32 offs2, INT32 offs3)
{
	INT32 c, y, x, Dat1, Dat2, Dat3, Col;

	for (c = 0; c < Num; c++) {
		for (y = 0; y < 8; y++) {
			Dat1 = System16TempGfx[offs1 + (c * 8) + y];
			Dat2 = System16TempGfx[offs2 + (c * 8) + y];
			Dat3 = System16TempGfx[offs3 + (c * 8) + y];
			for (x = 0; x < 8; x++) {
				Col = 0;
				if (Dat1 & 1) { Col |= 4;}
				if (Dat2 & 1) { Col |= 2;}
				if (Dat3 & 1) { Col |= 1;}
				pTile[(c * 64) + (y * 8) + (7 - x)] = Col;
				Dat1 >>= 1;
				Dat2 >>= 1;
				Dat3 >>= 1;
			}
		}
	}
}

void OutrunDecodeRoad()
{
	for (INT32 y = 0; y < 256 * 2; y++) {
		UINT8 *src = System16TempGfx + ((y & 0xff) * 0x40 + (y >> 8) * 0x8000) % System16RoadRomSize;
		UINT8 *dst = System16Roads + y * 512;

		/* loop over columns */
		for (INT32 x = 0; x < 512; x++)
		{
			dst[x] = (((src[x/8] >> (~x & 7)) & 1) << 0) | (((src[x/8 + 0x4000] >> (~x & 7)) & 1) << 1);

			/* pre-mark road data in the "stripe" area with a high bit */
			if (x >= 256-8 && x < 256 && dst[x] == 3)
				dst[x] |= 4;
		}
	}

	/* set up a dummy road in the last entry */
	memset(System16Roads + 256 * 2 * 512, 3, 512);
}

void HangonDecodeRoad()
{
	INT32 x, y;

	/* loop over rows */
	for (y = 0; y < 256; y++)
	{
		UINT8 *src = System16TempGfx + ((y & 0xff) * 0x40) % System16RoadRomSize;
		UINT8 *dst = System16Roads + y * 512;

		/* loop over columns */
		for (x = 0; x < 512; x++)
			dst[x] = (((src[x/8] >> (~x & 7)) & 1) << 0) | (((src[x/8 + 0x4000] >> (~x & 7)) & 1) << 1);
	}
}

/*====================================================
Tile Layer Rendering
====================================================*/

#define PLOTPIXEL(x, po) pPixel[x] = nPalette | pTileData[x] | po;
#define PLOTPIXEL_MASK(x, mc, po) if (pTileData[x] != mc) {pPixel[x] = nPalette | pTileData[x] | po;}

static inline void RenderTile(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, UINT8 *pTile)
{
	UINT32 nPalette = nTilePalette << nColourDepth;
	pTileData = pTile + (nTileNumber << 6);

	UINT16* pPixel = pDestDraw + (StartY * 1024) + StartX;

	for (INT32 y = 0; y < 8; y++, pPixel += 1024, pTileData += 8) {
		PLOTPIXEL(0, nPaletteOffset);
		PLOTPIXEL(1, nPaletteOffset);
		PLOTPIXEL(2, nPaletteOffset);
		PLOTPIXEL(3, nPaletteOffset);
		PLOTPIXEL(4, nPaletteOffset);
		PLOTPIXEL(5, nPaletteOffset);
		PLOTPIXEL(6, nPaletteOffset);
		PLOTPIXEL(7, nPaletteOffset);
	}
}

static void RenderTile_Mask(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile)
{
	UINT32 nPalette = nTilePalette << nColourDepth;
	pTileData = pTile + (nTileNumber << 6);

	UINT16* pPixel = pDestDraw + (StartY * 1024) + StartX;

	for (INT32 y = 0; y < 8; y++, pPixel += 1024, pTileData += 8) {
		PLOTPIXEL_MASK(0, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK(1, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK(2, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK(3, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK(4, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK(5, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK(6, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK(7, nMaskColour, nPaletteOffset);
	}
}

#undef PLOTPIXEL
#undef PLOTPIXEL_MASK

void System16ATileMapsInit(INT32 bOpaque)
{
	if (bOpaque) {
		pSys16BgTileMapOpaque = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	}
	
	pSys16BgTileMapPri0 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	pSys16BgTileMapPri1 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	pSys16FgTileMapPri0 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	pSys16FgTileMapPri1 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
}

void System16BTileMapsInit(INT32 bOpaque)
{
	if (bOpaque) {
		pSys16BgTileMapOpaque = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
		pSys16BgAltTileMapOpaque = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	}
	
	pSys16BgTileMapPri0 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	pSys16BgTileMapPri1 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	pSys16FgTileMapPri0 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	pSys16FgTileMapPri1 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	
	pSys16BgAltTileMapPri0 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	pSys16BgAltTileMapPri1 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	pSys16FgAltTileMapPri0 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	pSys16FgAltTileMapPri1 = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
}

void System16TileMapsExit()
{
	BurnFree(pSys16BgTileMapOpaque);
	BurnFree(pSys16BgAltTileMapOpaque);
	BurnFree(pSys16BgTileMapPri0);
	BurnFree(pSys16BgTileMapPri1);
	BurnFree(pSys16FgTileMapPri0);
	BurnFree(pSys16FgTileMapPri1);
	BurnFree(pSys16BgAltTileMapPri0);
	BurnFree(pSys16BgAltTileMapPri1);
	BurnFree(pSys16FgAltTileMapPri0);
	BurnFree(pSys16FgAltTileMapPri1);
}

static void System16ACreateBgTileMaps()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex, Priority, ColourOff, TilePage, i, xOffs, yOffs;
	UINT16 *VideoRam = (UINT16*)System16TileRam;
	UINT16 EffPage, ActPage;
	UINT16 *pDest = NULL;
	
	if (System16CreateOpaqueTileMaps) memset(pSys16BgTileMapOpaque, 0, 1024 * 512 * sizeof(UINT16));
	EffPage = System16Page[1];
	EffPage = ((EffPage >> 4) & 0x0707) | ((EffPage << 4) & 0x7070);
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_HANGON) EffPage &= 0x3333;
	for (i = 0; i < 2; i++) {
		if (i == 0) pDest = pSys16BgTileMapPri0;
		if (i == 1) pDest = pSys16BgTileMapPri1;
		memset(pDest, 0, 1024 * 512 * sizeof(UINT16));
		for (TilePage = 0; TilePage < 4; TilePage++) {
			ActPage = (EffPage >> 0) & 0x0f;
			xOffs = 0;
			yOffs = 0;
			if (TilePage == 1) { ActPage = (EffPage >> 4) & 0x0f; xOffs = 512; }
			if (TilePage == 2) { ActPage = (EffPage >> 8) & 0x0f; yOffs = 256; }
			if (TilePage == 3) { ActPage = (EffPage >> 12) & 0x0f; xOffs = 512; yOffs = 256; }
			for (my = 0; my < 32; my++) {
				y = 8 * my;
				y += yOffs;
				for (mx = 0; mx < 64; mx++) {
					TileIndex = (ActPage * 64 * 32) + (my * 64) + mx;
						
					Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
					Priority = (Attr >> 12) & 1;
			
					if (Priority != i) continue;
					
					x = 8 * mx;
					x += xOffs;
								
					Code = ((Attr >> 1) & 0x1000) | (Attr & 0xfff);
					Code &= (System16NumTiles - 1);
					Colour = (Attr >> 5) & 0x7f;
				
					ColourOff = System16TilemapColorOffset;
					if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
					if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
					if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
			
					if (System16CreateOpaqueTileMaps) RenderTile(pSys16BgTileMapOpaque, Code, x, y, Colour, 3, ColourOff, System16Tiles);
					RenderTile_Mask(pDest, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
				}
			}
		}
	}
}

static void System16ACreateFgTileMaps()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex, Priority, ColourOff, TilePage, xOffs, yOffs;
	UINT16 *VideoRam = (UINT16*)System16TileRam;
	UINT16 EffPage, ActPage;
	
	memset(pSys16FgTileMapPri0, 0, 1024 * 512 * sizeof(UINT16));
	memset(pSys16FgTileMapPri1, 0, 1024 * 512 * sizeof(UINT16));
		
	EffPage = System16Page[0];
	EffPage = ((EffPage >> 4) & 0x0707) | ((EffPage << 4) & 0x7070);
	if ((BurnDrvGetHardwareCode() & HARDWARE_PUBLIC_MASK) == HARDWARE_SEGA_HANGON) EffPage &= 0x3333;
	for (TilePage = 0; TilePage < 4; TilePage++) {
		ActPage = (EffPage >> 0) & 0x0f;
		xOffs = 0;
		yOffs = 0;
		if (TilePage == 1) { ActPage = (EffPage >> 4) & 0x0f; xOffs = 512; }
		if (TilePage == 2) { ActPage = (EffPage >> 8) & 0x0f; yOffs = 256; }
		if (TilePage == 3) { ActPage = (EffPage >> 12) & 0x0f; xOffs = 512; yOffs = 256; }
		for (my = 0; my < 32; my++) {
			y = 8 * my;
			y += yOffs;
			for (mx = 0; mx < 64; mx++) {
				TileIndex = (ActPage * 64 * 32) + (my * 64) + mx;
					
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
				Priority = (Attr >> 12) & 1;
			
				x = 8 * mx;
				x += xOffs;
			
				Code = ((Attr >> 1) & 0x1000) | (Attr & 0xfff);
				Code &= (System16NumTiles - 1);
				Colour = (Attr >> 5) & 0x7f;
				
				ColourOff = System16TilemapColorOffset;
				if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
				if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
				if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
		
				if (Priority == 0) RenderTile_Mask(pSys16FgTileMapPri0, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
				if (Priority == 1) RenderTile_Mask(pSys16FgTileMapPri1, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
			}
		}
	}
}

static void System16BCreateBgTileMaps()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex, Priority, ColourOff, TilePage, i, xOffs, yOffs;
	UINT16 *VideoRam = (UINT16*)System16TileRam;
	UINT16 EffPage, ActPage;
	UINT16 *pDest = NULL;
		
	if (System16CreateOpaqueTileMaps) memset(pSys16BgTileMapOpaque, 0, 1024 * 512 * sizeof(UINT16));
	EffPage = System16Page[1];
	for (i = 0; i < 2; i++) {
		if (i == 0) pDest = pSys16BgTileMapPri0;
		if (i == 1) pDest = pSys16BgTileMapPri1;
		memset(pDest, 0, 1024 * 512 * sizeof(UINT16));
		for (TilePage = 0; TilePage < 4; TilePage++) {
			ActPage = (EffPage >> 0) & 0x0f;
			xOffs = 0;
			yOffs = 0;
			if (TilePage == 1) { ActPage = (EffPage >> 4) & 0x0f; xOffs = 512; }
			if (TilePage == 2) { ActPage = (EffPage >> 8) & 0x0f; yOffs = 256; }
			if (TilePage == 3) { ActPage = (EffPage >> 12) & 0x0f; xOffs = 512; yOffs = 256; }
			for (my = 0; my < 32; my++) {
				y = 8 * my;
				y += yOffs;
				for (mx = 0; mx < 64; mx++) {
					TileIndex = (ActPage * 64 * 32) + (my * 64) + mx;
						
					Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
					Priority = (Attr >> 15) & 1;
			
					if (Priority != i) continue;
					
					x = 8 * mx;
					x += xOffs;
								
					Code = Attr & 0x1fff;
					Code = System16TileBanks[Code / System16TileBankSize] * System16TileBankSize + Code % System16TileBankSize;
					Code &= (System16NumTiles - 1);
					Colour = (Attr >> 6) & 0x7f;
				
					ColourOff = System16TilemapColorOffset;
					if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
					if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
					if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
			
					if (System16CreateOpaqueTileMaps) RenderTile(pSys16BgTileMapOpaque, Code, x, y, Colour, 3, ColourOff, System16Tiles);
					RenderTile_Mask(pDest, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
				}
			}
		}
	}
}

static void System16BCreateBgAltTileMaps()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex, Priority, ColourOff, TilePage, i, xOffs, yOffs;
	UINT16 *VideoRam = (UINT16*)System16TileRam;
	UINT16 EffPage, ActPage;
	UINT16 *pDest = NULL;
	
	if (System16CreateOpaqueTileMaps) memset(pSys16BgAltTileMapOpaque, 0, 1024 * 512 * sizeof(UINT16));
	EffPage = System16Page[3];
	for (i = 0; i < 2; i++) {
		if (i == 0) pDest = pSys16BgAltTileMapPri0;
		if (i == 1) pDest = pSys16BgAltTileMapPri1;
		if (System16RecalcBgAltTileMap) memset(pDest, 0,1024 * 512 * sizeof(UINT16));
		for (TilePage = 0; TilePage < 4; TilePage++) {
			ActPage = (EffPage >> 0) & 0x0f;
			xOffs = 0;
			yOffs = 0;
			if (TilePage == 1) { ActPage = (EffPage >> 4) & 0x0f; xOffs = 512; }
			if (TilePage == 2) { ActPage = (EffPage >> 8) & 0x0f; yOffs = 256; }
			if (TilePage == 3) { ActPage = (EffPage >> 12) & 0x0f; xOffs = 512; yOffs = 256; }
			for (my = 0; my < 32; my++) {
				y = 8 * my;
				y += yOffs;
				for (mx = 0; mx < 64; mx++) {
					TileIndex = (ActPage * 64 * 32) + (my * 64) + mx;
						
					Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
					Priority = (Attr >> 15) & 1;
			
					if (Priority != i) continue;
					
					x = 8 * mx;
					x += xOffs;
			
					Code = Attr & 0x1fff;
					Code = System16TileBanks[Code / System16TileBankSize] * System16TileBankSize + Code % System16TileBankSize;
					Code &= (System16NumTiles - 1);
					Colour = (Attr >> 6) & 0x7f;
				
					ColourOff = System16TilemapColorOffset;
					if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
					if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
					if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
			
					if (System16CreateOpaqueTileMaps) RenderTile(pSys16BgAltTileMapOpaque, Code, x, y, Colour, 3, ColourOff, System16Tiles);
					if (System16RecalcBgAltTileMap) RenderTile_Mask(pDest, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
				}
			}
		}
	}
}

static void System16BCreateFgTileMaps()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex, Priority, ColourOff, TilePage, xOffs, yOffs;
	UINT16 *VideoRam = (UINT16*)System16TileRam;
	UINT16 EffPage, ActPage;
	
	memset(pSys16FgTileMapPri0, 0, 1024 * 512 * sizeof(UINT16));
	memset(pSys16FgTileMapPri1, 0, 1024 * 512 * sizeof(UINT16));
		
	EffPage = System16Page[0];
	for (TilePage = 0; TilePage < 4; TilePage++) {
		ActPage = (EffPage >> 0) & 0x0f;
		xOffs = 0;
		yOffs = 0;
		if (TilePage == 1) { ActPage = (EffPage >> 4) & 0x0f; xOffs = 512; }
		if (TilePage == 2) { ActPage = (EffPage >> 8) & 0x0f; yOffs = 256; }
		if (TilePage == 3) { ActPage = (EffPage >> 12) & 0x0f; xOffs = 512; yOffs = 256; }
		for (my = 0; my < 32; my++) {
			y = 8 * my;
			y += yOffs;
			for (mx = 0; mx < 64; mx++) {
				TileIndex = (ActPage * 64 * 32) + (my * 64) + mx;
					
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
				Priority = (Attr >> 15) & 1;
			
				x = 8 * mx;
				x += xOffs;
			
				Code = Attr & 0x1fff;
				Code = System16TileBanks[Code / System16TileBankSize] * System16TileBankSize + Code % System16TileBankSize;
				Code &= (System16NumTiles - 1);
				Colour = (Attr >> 6) & 0x7f;
			
				ColourOff = System16TilemapColorOffset;
				if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
				if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
				if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
		
				if (Priority == 0) RenderTile_Mask(pSys16FgTileMapPri0, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
				if (Priority == 1) RenderTile_Mask(pSys16FgTileMapPri1, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
			}
		}
	}
}

static void System16BCreateFgAltTileMaps()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex, Priority, ColourOff, TilePage, xOffs, yOffs;
	UINT16 *VideoRam = (UINT16*)System16TileRam;
	UINT16 EffPage, ActPage;
	
	memset(pSys16FgAltTileMapPri0, 0, 1024 * 512 * sizeof(UINT16));
	memset(pSys16FgAltTileMapPri1, 0, 1024 * 512 * sizeof(UINT16));
		
	EffPage = System16Page[2];
	for (TilePage = 0; TilePage < 4; TilePage++) {
		ActPage = (EffPage >> 0) & 0x0f;
		xOffs = 0;
		yOffs = 0;
		if (TilePage == 1) { ActPage = (EffPage >> 4) & 0x0f; xOffs = 512; }
		if (TilePage == 2) { ActPage = (EffPage >> 8) & 0x0f; yOffs = 256; }
		if (TilePage == 3) { ActPage = (EffPage >> 12) & 0x0f; xOffs = 512; yOffs = 256; }
		for (my = 0; my < 32; my++) {
			y = 8 * my;
			y += yOffs;
			for (mx = 0; mx < 64; mx++) {
				TileIndex = (ActPage * 64 * 32) + (my * 64) + mx;
					
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
				Priority = (Attr >> 15) & 1;
		
				x = 8 * mx;
				x += xOffs;
			
				Code = Attr & 0x1fff;
				Code = System16TileBanks[Code / System16TileBankSize] * System16TileBankSize + Code % System16TileBankSize;
				Code &= (System16NumTiles - 1);
				Colour = (Attr >> 6) & 0x7f;
			
				ColourOff = System16TilemapColorOffset;
				if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
				if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
				if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
		
				if (Priority == 0) RenderTile_Mask(pSys16FgAltTileMapPri0, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
				if (Priority == 1) RenderTile_Mask(pSys16FgAltTileMapPri1, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
			}
		}
	}
}

static void System16BAltCreateBgTileMaps()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex, Priority, ColourOff, TilePage, i, xOffs, yOffs;
	UINT16 *VideoRam = (UINT16*)System16TileRam;
	UINT16 EffPage, ActPage;
	UINT16 *pDest = NULL;
		
	if (System16CreateOpaqueTileMaps) memset(pSys16BgTileMapOpaque, 0, 1024 * 512 * sizeof(UINT16));
	EffPage = System16Page[1];
	for (i = 0; i < 2; i++) {
		if (i == 0) pDest = pSys16BgTileMapPri0;
		if (i == 1) pDest = pSys16BgTileMapPri1;
		memset(pDest, 0, 1024 * 512 * sizeof(UINT16));
		for (TilePage = 0; TilePage < 4; TilePage++) {
			ActPage = (EffPage >> 0) & 0x0f;
			xOffs = 0;
			yOffs = 0;
			if (TilePage == 1) { ActPage = (EffPage >> 4) & 0x0f; xOffs = 512; }
			if (TilePage == 2) { ActPage = (EffPage >> 8) & 0x0f; yOffs = 256; }
			if (TilePage == 3) { ActPage = (EffPage >> 12) & 0x0f; xOffs = 512; yOffs = 256; }
			for (my = 0; my < 32; my++) {
				y = 8 * my;
				y += yOffs;
				for (mx = 0; mx < 64; mx++) {
					TileIndex = (ActPage * 64 * 32) + (my * 64) + mx;
						
					Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
					Priority = (Attr >> 15) & 1;
			
					if (Priority != i) continue;
					
					x = 8 * mx;
					x += xOffs;
								
					Code = Attr & 0x1fff;
					Code = System16TileBanks[Code / System16TileBankSize] * System16TileBankSize + Code % System16TileBankSize;
					Code &= (System16NumTiles - 1);
					Colour = (Attr >> 5) & 0x7f;
				
					ColourOff = System16TilemapColorOffset;
					if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
					if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
					if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
			
					if (System16CreateOpaqueTileMaps) RenderTile(pSys16BgTileMapOpaque, Code, x, y, Colour, 3, ColourOff, System16Tiles);
					RenderTile_Mask(pDest, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
				}
			}
		}
	}
}

static void System16BAltCreateBgAltTileMaps()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex, Priority, ColourOff, TilePage, i, xOffs, yOffs;
	UINT16 *VideoRam = (UINT16*)System16TileRam;
	UINT16 EffPage, ActPage;
	UINT16 *pDest = NULL;
	
	if (System16CreateOpaqueTileMaps) memset(pSys16BgAltTileMapOpaque, 0, 1024 * 512 * sizeof(UINT16));
	EffPage = System16Page[3];
	for (i = 0; i < 2; i++) {
		if (i == 0) pDest = pSys16BgAltTileMapPri0;
		if (i == 1) pDest = pSys16BgAltTileMapPri1;
		if (System16RecalcBgAltTileMap) memset(pDest, 0,1024 * 512 * sizeof(UINT16));
		for (TilePage = 0; TilePage < 4; TilePage++) {
			ActPage = (EffPage >> 0) & 0x0f;
			xOffs = 0;
			yOffs = 0;
			if (TilePage == 1) { ActPage = (EffPage >> 4) & 0x0f; xOffs = 512; }
			if (TilePage == 2) { ActPage = (EffPage >> 8) & 0x0f; yOffs = 256; }
			if (TilePage == 3) { ActPage = (EffPage >> 12) & 0x0f; xOffs = 512; yOffs = 256; }
			for (my = 0; my < 32; my++) {
				y = 8 * my;
				y += yOffs;
				for (mx = 0; mx < 64; mx++) {
					TileIndex = (ActPage * 64 * 32) + (my * 64) + mx;
						
					Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
					Priority = (Attr >> 15) & 1;
			
					if (Priority != i) continue;
					
					x = 8 * mx;
					x += xOffs;
			
					Code = Attr & 0x1fff;
					Code = System16TileBanks[Code / System16TileBankSize] * System16TileBankSize + Code % System16TileBankSize;
					Code &= (System16NumTiles - 1);
					Colour = (Attr >> 5) & 0x7f;
				
					ColourOff = System16TilemapColorOffset;
					if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
					if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
					if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
			
					if (System16CreateOpaqueTileMaps) RenderTile(pSys16BgAltTileMapOpaque, Code, x, y, Colour, 3, ColourOff, System16Tiles);
					if (System16RecalcBgAltTileMap) RenderTile_Mask(pDest, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
				}
			}
		}
	}
}

static void System16BAltCreateFgTileMaps()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex, Priority, ColourOff, TilePage, xOffs, yOffs;
	UINT16 *VideoRam = (UINT16*)System16TileRam;
	UINT16 EffPage, ActPage;
	
	memset(pSys16FgTileMapPri0, 0, 1024 * 512 * sizeof(UINT16));
	memset(pSys16FgTileMapPri1, 0, 1024 * 512 * sizeof(UINT16));
		
	EffPage = System16Page[0];
	for (TilePage = 0; TilePage < 4; TilePage++) {
		ActPage = (EffPage >> 0) & 0x0f;
		xOffs = 0;
		yOffs = 0;
		if (TilePage == 1) { ActPage = (EffPage >> 4) & 0x0f; xOffs = 512; }
		if (TilePage == 2) { ActPage = (EffPage >> 8) & 0x0f; yOffs = 256; }
		if (TilePage == 3) { ActPage = (EffPage >> 12) & 0x0f; xOffs = 512; yOffs = 256; }
		for (my = 0; my < 32; my++) {
			y = 8 * my;
			y += yOffs;
			for (mx = 0; mx < 64; mx++) {
				TileIndex = (ActPage * 64 * 32) + (my * 64) + mx;
					
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
				Priority = (Attr >> 15) & 1;
			
				x = 8 * mx;
				x += xOffs;
			
				Code = Attr & 0x1fff;
				Code = System16TileBanks[Code / System16TileBankSize] * System16TileBankSize + Code % System16TileBankSize;
				Code &= (System16NumTiles - 1);
				Colour = (Attr >> 5) & 0x7f;
			
				ColourOff = System16TilemapColorOffset;
				if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
				if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
				if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
		
				if (Priority == 0) RenderTile_Mask(pSys16FgTileMapPri0, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
				if (Priority == 1) RenderTile_Mask(pSys16FgTileMapPri1, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
			}
		}
	}
}

static void System16BAltCreateFgAltTileMaps()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex, Priority, ColourOff, TilePage, xOffs, yOffs;
	UINT16 *VideoRam = (UINT16*)System16TileRam;
	UINT16 EffPage, ActPage;
	
	memset(pSys16FgAltTileMapPri0, 0, 1024 * 512 * sizeof(UINT16));
	memset(pSys16FgAltTileMapPri1, 0, 1024 * 512 * sizeof(UINT16));
		
	EffPage = System16Page[2];
	for (TilePage = 0; TilePage < 4; TilePage++) {
		ActPage = (EffPage >> 0) & 0x0f;
		xOffs = 0;
		yOffs = 0;
		if (TilePage == 1) { ActPage = (EffPage >> 4) & 0x0f; xOffs = 512; }
		if (TilePage == 2) { ActPage = (EffPage >> 8) & 0x0f; yOffs = 256; }
		if (TilePage == 3) { ActPage = (EffPage >> 12) & 0x0f; xOffs = 512; yOffs = 256; }
		for (my = 0; my < 32; my++) {
			y = 8 * my;
			y += yOffs;
			for (mx = 0; mx < 64; mx++) {
				TileIndex = (ActPage * 64 * 32) + (my * 64) + mx;
					
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
				Priority = (Attr >> 15) & 1;
		
				x = 8 * mx;
				x += xOffs;
			
				Code = Attr & 0x1fff;
				Code = System16TileBanks[Code / System16TileBankSize] * System16TileBankSize + Code % System16TileBankSize;
				Code &= (System16NumTiles - 1);
				Colour = (Attr >> 5) & 0x7f;
			
				ColourOff = System16TilemapColorOffset;
				if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
				if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
				if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
		
				if (Priority == 0) RenderTile_Mask(pSys16FgAltTileMapPri0, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
				if (Priority == 1) RenderTile_Mask(pSys16FgAltTileMapPri1, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
			}
		}
	}
}

static void System16ACreateTileMaps()
{
	if (System16RecalcBgTileMap) {
		System16ACreateBgTileMaps();
		System16RecalcBgTileMap = 0;
	}
	
	if (System16RecalcFgTileMap) {
		System16ACreateFgTileMaps();
		System16RecalcFgTileMap = 0;
	}
}

static void System16BCreateTileMaps()
{
	UINT16 *TextRam = (UINT16*)System16TextRam;
	INT32 i;
	
	// Check if we need the alt tilemaps
	if (System16RecalcFgAltTileMap) {
		System16RecalcFgAltTileMap = 0;
		for (i = 0xf80/2 + 0 * 0x40/2; i < 0xf80/2 + 0 * 0x40/2 + 224/8; i++) {
			if (TextRam[i] & 0x8000) System16RecalcFgAltTileMap = 1;
		}
	}
	
	if (System16RecalcBgAltTileMap) {
		System16RecalcBgAltTileMap = 0;
		for (i = 0xf80/2 + 1 * 0x40/2; i < 0xf80/2 + 1 * 0x40/2 + 224/8; i++) {
			if (TextRam[i] & 0x8000) System16RecalcBgAltTileMap = 1;
		}
	}

	if (System16RecalcBgTileMap) {
		System16BCreateBgTileMaps();
		System16RecalcBgTileMap = 0;
	}
	
	if (System16RecalcBgAltTileMap) {
		System16BCreateBgAltTileMaps();
		System16RecalcBgAltTileMap = 0;
	}
	
	if (System16RecalcFgTileMap) {
		System16BCreateFgTileMaps();
		System16RecalcFgTileMap = 0;
	}
	
	if (System16RecalcFgAltTileMap) {
		System16BCreateFgAltTileMaps();
		System16RecalcFgAltTileMap = 0;
	}
}

static void System16BAltCreateTileMaps()
{
	UINT16 *TextRam = (UINT16*)System16TextRam;
	INT32 i;
	
	// Check if we need the alt tilemaps
	if (System16RecalcFgAltTileMap) {
		System16RecalcFgAltTileMap = 0;
		for (i = 0xf80/2 + 0 * 0x40/2; i < 0xf80/2 + 0 * 0x40/2 + 224/8; i++) {
			if (TextRam[i] & 0x8000) System16RecalcFgAltTileMap = 1;
		}
	}
	
	if (System16RecalcBgAltTileMap) {
		System16RecalcBgAltTileMap = 0;
		for (i = 0xf80/2 + 1 * 0x40/2; i < 0xf80/2 + 1 * 0x40/2 + 224/8; i++) {
			if (TextRam[i] & 0x8000) System16RecalcBgAltTileMap = 1;
		}
	}

	if (System16RecalcBgTileMap) {
		System16BAltCreateBgTileMaps();
		System16RecalcBgTileMap = 0;
	}
	
	if (System16RecalcBgAltTileMap) {
		System16BAltCreateBgAltTileMaps();
		System16RecalcBgAltTileMap = 0;
	}
	
	if (System16RecalcFgTileMap) {
		System16BAltCreateFgTileMaps();
		System16RecalcFgTileMap = 0;
	}
	
	if (System16RecalcFgAltTileMap) {
		System16BAltCreateFgAltTileMaps();
		System16RecalcFgAltTileMap = 0;
	}
}

void System16ATileWordWrite(UINT32 Offset, UINT16 d)
{
	UINT16 *TileRam = (UINT16*)System16TileRam;
	
	UINT32 FgPage1Addr = (((System16Page[0] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage2Addr = (((System16Page[0] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage3Addr = (((System16Page[0] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage4Addr = (((System16Page[0] >> 12) & 0xf) * (32 * 64)) << 1;
	
	UINT32 BgPage1Addr = (((System16Page[1] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage2Addr = (((System16Page[1] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage3Addr = (((System16Page[1] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage4Addr = (((System16Page[1] >> 12) & 0xf) * (32 * 64)) << 1;
	
	if ((Offset >= FgPage1Addr && Offset <= (FgPage1Addr + 0xfff)) || (Offset >= FgPage2Addr && Offset <= (FgPage2Addr + 0xfff)) || (Offset >= FgPage3Addr && Offset <= (FgPage3Addr + 0xfff)) || (Offset >= FgPage4Addr && Offset <= (FgPage4Addr + 0xfff))) {
		if (TileRam[Offset >> 1] != BURN_ENDIAN_SWAP_INT16(d)) System16RecalcFgTileMap = 1;
	}
	
	if ((Offset >= BgPage1Addr && Offset <= (BgPage1Addr + 0xfff)) || (Offset >= BgPage2Addr && Offset <= (BgPage2Addr + 0xfff)) || (Offset >= BgPage3Addr && Offset <= (BgPage3Addr + 0xfff)) || (Offset >= BgPage4Addr && Offset <= (BgPage4Addr + 0xfff))) {
		if (TileRam[Offset >> 1] != BURN_ENDIAN_SWAP_INT16(d)) System16RecalcBgTileMap = 1;
	}
	
	TileRam[Offset >> 1] = BURN_ENDIAN_SWAP_INT16(d);
}

void System16ATileByteWrite(UINT32 Offset, UINT8 d)
{
	UINT32 FgPage1Addr = (((System16Page[0] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage2Addr = (((System16Page[0] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage3Addr = (((System16Page[0] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage4Addr = (((System16Page[0] >> 12) & 0xf) * (32 * 64)) << 1;
	
	UINT32 BgPage1Addr = (((System16Page[1] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage2Addr = (((System16Page[1] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage3Addr = (((System16Page[1] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage4Addr = (((System16Page[1] >> 12) & 0xf) * (32 * 64)) << 1;
	
	if ((Offset >= FgPage1Addr && Offset <= (FgPage1Addr + 0xfff)) || (Offset >= FgPage2Addr && Offset <= (FgPage2Addr + 0xfff)) || (Offset >= FgPage3Addr && Offset <= (FgPage3Addr + 0xfff)) || (Offset >= FgPage4Addr && Offset <= (FgPage4Addr + 0xfff))) {
		if (System16TileRam[Offset] != d) System16RecalcFgTileMap = 1;
	}
	
	if ((Offset >= BgPage1Addr && Offset <= (BgPage1Addr + 0xfff)) || (Offset >= BgPage2Addr && Offset <= (BgPage2Addr + 0xfff)) || (Offset >= BgPage3Addr && Offset <= (BgPage3Addr + 0xfff)) || (Offset >= BgPage4Addr && Offset <= (BgPage4Addr + 0xfff))) {
		if (System16TileRam[Offset] != d) System16RecalcBgTileMap = 1;
	}
	
	System16TileRam[Offset] = d;
}

void System16BTileWordWrite(UINT32 Offset, UINT16 d)
{
	UINT16 *TileRam = (UINT16*)System16TileRam;
	
	UINT32 FgPage1Addr = (((System16Page[0] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage2Addr = (((System16Page[0] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage3Addr = (((System16Page[0] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage4Addr = (((System16Page[0] >> 12) & 0xf) * (32 * 64)) << 1;
	
	UINT32 BgPage1Addr = (((System16Page[1] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage2Addr = (((System16Page[1] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage3Addr = (((System16Page[1] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage4Addr = (((System16Page[1] >> 12) & 0xf) * (32 * 64)) << 1;
	
	UINT32 FgAltPage1Addr = (((System16Page[2] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 FgAltPage2Addr = (((System16Page[2] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 FgAltPage3Addr = (((System16Page[2] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 FgAltPage4Addr = (((System16Page[2] >> 12) & 0xf) * (32 * 64)) << 1;
	
	UINT32 BgAltPage1Addr = (((System16Page[3] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 BgAltPage2Addr = (((System16Page[3] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 BgAltPage3Addr = (((System16Page[3] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 BgAltPage4Addr = (((System16Page[3] >> 12) & 0xf) * (32 * 64)) << 1;
	
	if ((Offset >= FgPage1Addr && Offset <= (FgPage1Addr + 0xfff)) || (Offset >= FgPage2Addr && Offset <= (FgPage2Addr + 0xfff)) || (Offset >= FgPage3Addr && Offset <= (FgPage3Addr + 0xfff)) || (Offset >= FgPage4Addr && Offset <= (FgPage4Addr + 0xfff))) {
		if (TileRam[Offset >> 1] != BURN_ENDIAN_SWAP_INT16(d)) System16RecalcFgTileMap = 1;
	}
	
	if ((Offset >= BgPage1Addr && Offset <= (BgPage1Addr + 0xfff)) || (Offset >= BgPage2Addr && Offset <= (BgPage2Addr + 0xfff)) || (Offset >= BgPage3Addr && Offset <= (BgPage3Addr + 0xfff)) || (Offset >= BgPage4Addr && Offset <= (BgPage4Addr + 0xfff))) {
		if (TileRam[Offset >> 1] != BURN_ENDIAN_SWAP_INT16(d)) System16RecalcBgTileMap = 1;
	}
	
	if ((Offset >= FgAltPage1Addr && Offset <= (FgAltPage1Addr + 0xfff)) || (Offset >= FgAltPage2Addr && Offset <= (FgAltPage2Addr + 0xfff)) || (Offset >= FgAltPage3Addr && Offset <= (FgAltPage3Addr + 0xfff)) || (Offset >= FgAltPage4Addr && Offset <= (FgAltPage4Addr + 0xfff))) {
		if (TileRam[Offset >> 1] != BURN_ENDIAN_SWAP_INT16(d)) System16RecalcFgAltTileMap = 1;
	}
	
	if ((Offset >= BgAltPage1Addr && Offset <= (BgAltPage1Addr + 0xfff)) || (Offset >= BgAltPage2Addr && Offset <= (BgAltPage2Addr + 0xfff)) || (Offset >= BgAltPage3Addr && Offset <= (BgAltPage3Addr + 0xfff)) || (Offset >= BgAltPage4Addr && Offset <= (BgAltPage4Addr + 0xfff))) {
		if (TileRam[Offset >> 1] != BURN_ENDIAN_SWAP_INT16(d)) System16RecalcBgAltTileMap = 1;
	}	
	
	TileRam[Offset >> 1] = BURN_ENDIAN_SWAP_INT16(d);
}

void System16BTileByteWrite(UINT32 Offset, UINT8 d)
{
	UINT32 FgPage1Addr = (((System16Page[0] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage2Addr = (((System16Page[0] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage3Addr = (((System16Page[0] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 FgPage4Addr = (((System16Page[0] >> 12) & 0xf) * (32 * 64)) << 1;
	
	UINT32 BgPage1Addr = (((System16Page[1] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage2Addr = (((System16Page[1] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage3Addr = (((System16Page[1] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 BgPage4Addr = (((System16Page[1] >> 12) & 0xf) * (32 * 64)) << 1;
	
	UINT32 FgAltPage1Addr = (((System16Page[2] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 FgAltPage2Addr = (((System16Page[2] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 FgAltPage3Addr = (((System16Page[2] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 FgAltPage4Addr = (((System16Page[2] >> 12) & 0xf) * (32 * 64)) << 1;
	
	UINT32 BgAltPage1Addr = (((System16Page[3] >> 0) & 0xf) * (32 * 64)) << 1;
	UINT32 BgAltPage2Addr = (((System16Page[3] >> 4) & 0xf) * (32 * 64)) << 1;
	UINT32 BgAltPage3Addr = (((System16Page[3] >> 8) & 0xf) * (32 * 64)) << 1;
	UINT32 BgAltPage4Addr = (((System16Page[3] >> 12) & 0xf) * (32 * 64)) << 1;
	
	if ((Offset >= FgPage1Addr && Offset <= (FgPage1Addr + 0xfff)) || (Offset >= FgPage2Addr && Offset <= (FgPage2Addr + 0xfff)) || (Offset >= FgPage3Addr && Offset <= (FgPage3Addr + 0xfff)) || (Offset >= FgPage4Addr && Offset <= (FgPage4Addr + 0xfff))) {
		if (System16TileRam[Offset] != d) System16RecalcFgTileMap = 1;
	}
	
	if ((Offset >= BgPage1Addr && Offset <= (BgPage1Addr + 0xfff)) || (Offset >= BgPage2Addr && Offset <= (BgPage2Addr + 0xfff)) || (Offset >= BgPage3Addr && Offset <= (BgPage3Addr + 0xfff)) || (Offset >= BgPage4Addr && Offset <= (BgPage4Addr + 0xfff))) {
		if (System16TileRam[Offset] != d) System16RecalcBgTileMap = 1;
	}
	
	if ((Offset >= FgAltPage1Addr && Offset <= (FgAltPage1Addr + 0xfff)) || (Offset >= FgAltPage2Addr && Offset <= (FgAltPage2Addr + 0xfff)) || (Offset >= FgAltPage3Addr && Offset <= (FgAltPage3Addr + 0xfff)) || (Offset >= FgAltPage4Addr && Offset <= (FgAltPage4Addr + 0xfff))) {
		if (System16TileRam[Offset] != d) System16RecalcFgAltTileMap = 1;
	}
	
	if ((Offset >= BgAltPage1Addr && Offset <= (BgAltPage1Addr + 0xfff)) || (Offset >= BgAltPage2Addr && Offset <= (BgAltPage2Addr + 0xfff)) || (Offset >= BgAltPage3Addr && Offset <= (BgAltPage3Addr + 0xfff)) || (Offset >= BgAltPage4Addr && Offset <= (BgAltPage4Addr + 0xfff))) {
		if (System16TileRam[Offset] != d) System16RecalcBgAltTileMap = 1;
	}	
	
	System16TileRam[Offset] = d;
}

static void System16ARenderTileLayer(INT32 Page, INT32 PriorityDraw, INT32 Transparent)
{
	INT32 xScroll, yScroll, x, y;
	UINT16 *TextRam = (UINT16*)System16TextRam;
	
	xScroll = System16ScrollX[Page];
	yScroll = System16ScrollY[Page];
	UINT16 Pix;
	
	UINT16 *pTileMapSrc = NULL;
	UINT16 *pTileMapDest = NULL;
	INT32 xSrcOff, ySrcOff, RowScrollIndex, xEffScroll, yEffScroll;
	
	if (System16ColScroll && System16RowScroll) {
		for (y = 0; y < nScreenHeight; y++) {
			pTileMapDest = pTransDraw + (y * nScreenWidth);
		
			RowScrollIndex = y / 8;
		
			xEffScroll = BURN_ENDIAN_SWAP_INT16(TextRam[0xf80/2 + RowScrollIndex * 2 + Page]) & 0x1ff;
			
			for (x = 0; x < nScreenWidth; x++) {
				yEffScroll = BURN_ENDIAN_SWAP_INT16(TextRam[0xf30/2 + (x/16) * 2 + Page]) & 0xff;
		
				ySrcOff = (y + yEffScroll) & 0x1ff;
				if (Page == 0 && PriorityDraw == 0) pTileMapSrc = pSys16FgTileMapPri0 + (ySrcOff * 1024);
				if (Page == 0 && PriorityDraw == 1) pTileMapSrc = pSys16FgTileMapPri1 + (ySrcOff * 1024);
				if (Page == 1 && PriorityDraw == 0 && Transparent == 1) pTileMapSrc = pSys16BgTileMapPri0 + (ySrcOff * 1024);
				if (Page == 1 && PriorityDraw == 1 && Transparent == 1) pTileMapSrc = pSys16BgTileMapPri1 + (ySrcOff * 1024);
				if (Page == 1 && Transparent == 0) pTileMapSrc = pSys16BgTileMapOpaque + (ySrcOff * 1024);
							
				xSrcOff = (x - xEffScroll + 200) & 0x3ff;
				Pix = pTileMapSrc[xSrcOff];

				if (Transparent) {
					if (Pix) pTileMapDest[x] = Pix;
				} else {
					pTileMapDest[x] = Pix;
				}
			}
		}
	} else {
		if (System16ColScroll) {
			for (y = 0; y < nScreenHeight; y++) {
				pTileMapDest = pTransDraw + (y * nScreenWidth);
				
				xEffScroll = xScroll;
		
				for (x = 0; x < nScreenWidth; x++) {
					yEffScroll = BURN_ENDIAN_SWAP_INT16(TextRam[0xf30/2 + (x/16) * 2 + Page]) & 0xff;
		
					ySrcOff = (y + yEffScroll) & 0x1ff;
					if (Page == 0 && PriorityDraw == 0) pTileMapSrc = pSys16FgTileMapPri0 + (ySrcOff * 1024);
					if (Page == 0 && PriorityDraw == 1) pTileMapSrc = pSys16FgTileMapPri1 + (ySrcOff * 1024);
					if (Page == 1 && PriorityDraw == 0 && Transparent == 1) pTileMapSrc = pSys16BgTileMapPri0 + (ySrcOff * 1024);
					if (Page == 1 && PriorityDraw == 1 && Transparent == 1) pTileMapSrc = pSys16BgTileMapPri1 + (ySrcOff * 1024);
					if (Page == 1 && Transparent == 0) pTileMapSrc = pSys16BgTileMapOpaque + (ySrcOff * 1024);
							
					xSrcOff = (x - xEffScroll + 200) & 0x3ff;
					Pix = pTileMapSrc[xSrcOff];

					if (Transparent) {
						if (Pix) pTileMapDest[x] = Pix;
					} else {
						pTileMapDest[x] = Pix;
					}
				}
			}
		} else {
			if (System16RowScroll) {
				for (y = 0; y < nScreenHeight; y++) {
					pTileMapDest = pTransDraw + (y * nScreenWidth);
		
					RowScrollIndex = y / 8;
		
					xEffScroll = BURN_ENDIAN_SWAP_INT16(TextRam[0xf80/2 + RowScrollIndex * 2 + Page]) & 0x1ff;
			
					for (x = 0; x < nScreenWidth; x++) {
						yEffScroll = yScroll;
		
						ySrcOff = (y + yEffScroll) & 0x1ff;
						if (Page == 0 && PriorityDraw == 0) pTileMapSrc = pSys16FgTileMapPri0 + (ySrcOff * 1024);
						if (Page == 0 && PriorityDraw == 1) pTileMapSrc = pSys16FgTileMapPri1 + (ySrcOff * 1024);
						if (Page == 1 && PriorityDraw == 0 && Transparent == 1) pTileMapSrc = pSys16BgTileMapPri0 + (ySrcOff * 1024);
						if (Page == 1 && PriorityDraw == 1 && Transparent == 1) pTileMapSrc = pSys16BgTileMapPri1 + (ySrcOff * 1024);
						if (Page == 1 && Transparent == 0) pTileMapSrc = pSys16BgTileMapOpaque + (ySrcOff * 1024);
							
						xSrcOff = (x - xEffScroll + 200) & 0x3ff;
						Pix = pTileMapSrc[xSrcOff];

						if (Transparent) {
							if (Pix) pTileMapDest[x] = Pix;
						} else {
							pTileMapDest[x] = Pix;
						}
					}
				}	
			} else {
				for (y = 0; y < nScreenHeight; y++) {
					pTileMapDest = pTransDraw + (y * nScreenWidth);
			
					ySrcOff = (y + yScroll) & 0x1ff;
					if (Page == 0 && PriorityDraw == 0) pTileMapSrc = pSys16FgTileMapPri0 + (ySrcOff * 1024);
					if (Page == 0 && PriorityDraw == 1) pTileMapSrc = pSys16FgTileMapPri1 + (ySrcOff * 1024);
					if (Page == 1 && PriorityDraw == 0) pTileMapSrc = pSys16BgTileMapPri0 + (ySrcOff * 1024);
					if (Page == 1 && PriorityDraw == 1) pTileMapSrc = pSys16BgTileMapPri1 + (ySrcOff * 1024);
					if (Page == 1 && Transparent == 0) pTileMapSrc = pSys16BgTileMapOpaque + (ySrcOff * 1024);
		
					for (x = 0; x < nScreenWidth; x++) {	
						xSrcOff = (x - xScroll + 200) & 0x3ff;
						Pix = pTileMapSrc[xSrcOff];

						if (Transparent) {
							if (Pix) pTileMapDest[x] = Pix;
						} else {
							pTileMapDest[x] = Pix;
						}
					}
				}
			}
		}
	}
}

static void System16BRenderTileLayer(INT32 Page, INT32 PriorityDraw, INT32 Transparent)
{
	INT32 xScroll, yScroll, x, y;
	UINT16 *TextRam = (UINT16*)System16TextRam;
	
	xScroll = System16ScrollX[Page];
	yScroll = System16ScrollY[Page];
	UINT16 Pix;
	
	UINT16 *pTileMapSrc = NULL;
	UINT16 *pTileMapDest = NULL;
	INT32 xSrcOff, ySrcOff, RowScrollIndex, RowScroll, xEffScroll, yEffScroll;
	
	if (yScroll & 0x8000) {
		for (y = 0; y < nScreenHeight; y++) {
			pTileMapDest = pTransDraw + (y * nScreenWidth);
		
			RowScrollIndex = y / 8;
		
			RowScroll = BURN_ENDIAN_SWAP_INT16(TextRam[0xf80/2 + 0x40/2 * Page + RowScrollIndex]);
			xEffScroll = (xScroll & 0x8000) ? RowScroll : xScroll;
			
			for (x = 0; x < nScreenWidth; x++) {
				yEffScroll = BURN_ENDIAN_SWAP_INT16(TextRam[0xf16/2 + 0x40/2 * Page + (x+9)/16]);
		
				if (RowScroll & 0x8000) {
					xEffScroll = System16ScrollX[Page + 2];
					yEffScroll = System16ScrollY[Page + 2];
					
					ySrcOff = (y + yEffScroll) & 0x1ff;	
					if (Page == 0 && PriorityDraw == 0) pTileMapSrc = pSys16FgAltTileMapPri0 + (ySrcOff * 1024);
					if (Page == 0 && PriorityDraw == 1) pTileMapSrc = pSys16FgAltTileMapPri1 + (ySrcOff * 1024);
					if (Page == 1 && PriorityDraw == 0) pTileMapSrc = pSys16BgAltTileMapPri0 + (ySrcOff * 1024);
					if (Page == 1 && PriorityDraw == 1) pTileMapSrc = pSys16BgAltTileMapPri1 + (ySrcOff * 1024);
					if (Page == 1 && Transparent == 0) pTileMapSrc = pSys16BgAltTileMapOpaque + (ySrcOff * 1024);
				} else {
					ySrcOff = (y + yEffScroll) & 0x1ff;	
					if (Page == 0 && PriorityDraw == 0) pTileMapSrc = pSys16FgTileMapPri0 + (ySrcOff * 1024);
					if (Page == 0 && PriorityDraw == 1) pTileMapSrc = pSys16FgTileMapPri1 + (ySrcOff * 1024);
					if (Page == 1 && PriorityDraw == 0 && Transparent == 1) pTileMapSrc = pSys16BgTileMapPri0 + (ySrcOff * 1024);
					if (Page == 1 && PriorityDraw == 1 && Transparent == 1) pTileMapSrc = pSys16BgTileMapPri1 + (ySrcOff * 1024);
					if (Page == 1 && Transparent == 0) pTileMapSrc = pSys16BgTileMapOpaque + (ySrcOff * 1024);
				}
							
				xSrcOff = (x - xEffScroll + 192) & 0x3ff;
				Pix = pTileMapSrc[xSrcOff];

				if (Transparent) {
					if (Pix) pTileMapDest[x] = Pix;
				} else {
					pTileMapDest[x] = Pix;
				}
			}
		}
	} else {
		for (y = 0; y < nScreenHeight; y++) {
			pTileMapDest = pTransDraw + (y * nScreenWidth);
		
			RowScrollIndex = y / 8;
		
			RowScroll = BURN_ENDIAN_SWAP_INT16(TextRam[0xf80/2 + 0x40/2 * Page + RowScrollIndex]);
			xEffScroll = (xScroll & 0x8000) ? RowScroll : xScroll;
			yEffScroll = yScroll;
		
			if (RowScroll & 0x8000) {
				xEffScroll = System16ScrollX[Page + 2];
				yEffScroll = System16ScrollY[Page + 2];
				
				ySrcOff = (y + yEffScroll) & 0x1ff;	
				if (Page == 0 && PriorityDraw == 0) pTileMapSrc = pSys16FgAltTileMapPri0 + (ySrcOff * 1024);
				if (Page == 0 && PriorityDraw == 1) pTileMapSrc = pSys16FgAltTileMapPri1 + (ySrcOff * 1024);
				if (Page == 1 && PriorityDraw == 0) pTileMapSrc = pSys16BgAltTileMapPri0 + (ySrcOff * 1024);
				if (Page == 1 && PriorityDraw == 1) pTileMapSrc = pSys16BgAltTileMapPri1 + (ySrcOff * 1024);
				if (Page == 1 && Transparent == 0) pTileMapSrc = pSys16BgAltTileMapOpaque + (ySrcOff * 1024);
			} else {
				ySrcOff = (y + yEffScroll) & 0x1ff;		
				if (Page == 0 && PriorityDraw == 0) pTileMapSrc = pSys16FgTileMapPri0 + (ySrcOff * 1024);
				if (Page == 0 && PriorityDraw == 1) pTileMapSrc = pSys16FgTileMapPri1 + (ySrcOff * 1024);
				if (Page == 1 && PriorityDraw == 0) pTileMapSrc = pSys16BgTileMapPri0 + (ySrcOff * 1024);
				if (Page == 1 && PriorityDraw == 1) pTileMapSrc = pSys16BgTileMapPri1 + (ySrcOff * 1024);
				if (Page == 1 && Transparent == 0) pTileMapSrc = pSys16BgTileMapOpaque + (ySrcOff * 1024);
			}
		
			for (x = 0; x < nScreenWidth; x++) {
				xSrcOff = (x - xEffScroll + 192) & 0x3ff;
				Pix = pTileMapSrc[xSrcOff];

				if (Transparent) {
					if (Pix) pTileMapDest[x] = Pix;
				} else {
					pTileMapDest[x] = Pix;
				}
			}
		}
	}
}

static void BootlegRenderTileLayer(INT32 Page, INT32 PriorityDraw, INT32 Transparent)
{
	INT32 mx, my, Code, Colour, x, y, Priority, TileIndex, ColourOff;
	
	UINT16 EffPage, ActPage = 0, xScroll, yScroll;
	
	EffPage = System16Page[Page];
	xScroll = System16ScrollX[Page];
	yScroll = System16ScrollY[Page];
	
	INT32 yStartPosArray[28 * 21];
	
	float yPos = float(yScroll % 8);
	INT32 yStartPos = INT32(yPos) * -1;

	if (yScroll & 0x8000) {
		for (y = 0; y < 224; y += 8) {
			for (x = -8; x < 320; x += 16) {
				INT32 yColScrollTemp = (System16TextRam[0xf16 + (0x40 * Page) +  (2 * (x + 8) / 16) + 1] << 8) | System16TextRam[0xf16 + (0x40 * Page) +  (2 * (x + 8) / 16) + 0];
				float yTempPos = float(yColScrollTemp % 8);
				INT32 yTempStartPos = INT32(yTempPos) * -1;
				yStartPosArray[(y / 8) + (28 * ((x + 8) / 16))] = yTempStartPos + y;
			}
		}
		
		for (INT32 ny = 0; ny < 28; ny++) {
			INT32 RowScrollIndex = ny;
			
			INT32 RowScroll = (System16TextRam[0xf80 + (0x40 * Page) +  (2 * RowScrollIndex) + 1] << 8) | System16TextRam[0xf80 + (0x40 * Page) +  (2 * RowScrollIndex) + 0];
			
			INT32 xScrollRow = ((xScroll & 0x8000) ? RowScroll : xScroll);
			
			INT32 yScrollCol;
			
			if (RowScroll & 0x8000) {
				EffPage = System16Page[Page + 2];
				xScrollRow = System16ScrollX[Page + 2];
				yScrollCol = System16ScrollY[Page + 2];
			}
				
			float xPos = float(xScrollRow % 8);
			INT32 xStartPos = (INT32)xPos;
			
			if (xStartPos == 1) xStartPos = -7;
			if (xStartPos == 2) xStartPos = -6;
			if (xStartPos == 3) xStartPos = -5;
			if (xStartPos == 4) xStartPos = -4;
			if (xStartPos == 5) xStartPos = -3;
			if (xStartPos == 6) xStartPos = -2;
			if (xStartPos == 7) xStartPos = -1;
			if (xStartPos == 0) xStartPos = 0;
			
			for (x = -8; x < 320; x += 16) {
				INT32 x2 = xStartPos + x;
				
				yScrollCol = (System16TextRam[0xf16 + (0x40 * Page) +  (2 * (x + 8) / 16) + 1] << 8) | System16TextRam[0xf16 + (0x40 * Page) +  (2 * (x + 8) / 16) + 0];
				
				my = ((ny * 8) + (yScrollCol & 0x1ff)) / 8;
		
				if (my > 63) my -= 64;
	
				y = yStartPosArray[ny + (28 * ((x + 8) / 16))];
					
				mx = (x2 - (xScrollRow & 0x3ff) + 192) / 8;
				if (mx < 0) mx += 128;
				if (Page == 1) {
					if (my < 32 && mx < 64) ActPage = BootlegBgPage[0];
					if (my < 32 && mx >= 64) ActPage = BootlegBgPage[1];
					if (my >= 32 && mx < 64) ActPage = BootlegBgPage[2];
					if (my >= 32 && mx >= 64) ActPage = BootlegBgPage[3];
				}
				if (Page == 0) {
					if (my < 32 && mx < 64) ActPage = BootlegFgPage[0];
					if (my < 32 && mx >= 64) ActPage = BootlegFgPage[1];
					if (my >= 32 && mx < 64) ActPage = BootlegFgPage[2];
					if (my >= 32 && mx >= 64) ActPage = BootlegFgPage[3];
				}
				
				TileIndex = 64 * 32 * 2 * ActPage + ((2 * 64 * my) & 0xfff) + ((2 * mx) & 0x7f);
				INT32 Data = (System16TileRam[TileIndex + 1] << 8) | System16TileRam[TileIndex + 0];
				INT32 Data2 = (System16TileRam[TileIndex + 3] << 8) | System16TileRam[TileIndex + 2];
				
				Priority = (Data >> 15) & 1;
				
				if (Priority == PriorityDraw) {
					Code = Data & 0x1fff;
					Code = System16TileBanks[Code / System16TileBankSize] * System16TileBankSize + Code % System16TileBankSize;
					Code &= (System16NumTiles - 1);
					Colour = (Data >> 6) & 0x7f;
			
					ColourOff = System16TilemapColorOffset;
					if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
					if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
					if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
			
					if (Transparent) {
						if (x2 > 7 && x2 < 312 && y > 7 && y < 216) {
							Render8x8Tile_Mask(pTransDraw, Code, x2, y, Colour, 3, 0, ColourOff, System16Tiles);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, Code, x2, y, Colour, 3, 0, ColourOff, System16Tiles);
						}
					} else {
						if (x2 > 7 && x2 < 312 && y > 7 && y < 216) {
							Render8x8Tile(pTransDraw, Code, x2, y, Colour, 3, ColourOff, System16Tiles);
						} else {
							Render8x8Tile_Clip(pTransDraw, Code, x2, y, Colour, 3, ColourOff, System16Tiles);
						}
					}
					
					Code = Data2 & 0x1fff;
					Code = System16TileBanks[Code / System16TileBankSize] * System16TileBankSize + Code % System16TileBankSize;
					Code &= (System16NumTiles - 1);
					Colour = (Data2 >> 6) & 0x7f;
			
					ColourOff = System16TilemapColorOffset;
					if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
					if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
					if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
					
					INT32 x1 = x2 + 8;
					
					if (Transparent) {
						if (x1 > 7 && x1 < 312 && y > 7 && y < 216) {
							Render8x8Tile_Mask(pTransDraw, Code, x1, y, Colour, 3, 0, ColourOff, System16Tiles);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, Code, x1, y, Colour, 3, 0, ColourOff, System16Tiles);
						}
					} else {
						if (x1 > 7 && x1 < 312 && y > 7 && y < 216) {
							Render8x8Tile(pTransDraw, Code, x1, y, Colour, 3, ColourOff, System16Tiles);
						} else {
							Render8x8Tile_Clip(pTransDraw, Code, x1, y, Colour, 3, ColourOff, System16Tiles);
						}
					}
				}
			}
		}		
	} else {
		for (y = yStartPos; y < 224; y += 8) {
			INT32 RowScrollIndex = y / 8;
			INT32 RowScroll = (System16TextRam[0xf80 + (0x40 * Page) +  (2 * RowScrollIndex) + 1] << 8) | System16TextRam[0xf80 + (0x40 * Page) +  (2 * RowScrollIndex) + 0];
		
			INT32 xScrollRow = ((xScroll & 0x8000) ? RowScroll : xScroll);
		
			if (RowScroll & 0x8000) {
				EffPage = System16Page[Page + 2];
				xScrollRow = System16ScrollX[Page + 2];
				yScroll = System16ScrollY[Page + 2];
			}
		
			float xPos = float(xScrollRow % 8);
			INT32 xStartPos = (INT32)xPos;
		
			if (xStartPos == 1) xStartPos = -7;
			if (xStartPos == 2) xStartPos = -6;
			if (xStartPos == 3) xStartPos = -5;
			if (xStartPos == 4) xStartPos = -4;
			if (xStartPos == 5) xStartPos = -3;
			if (xStartPos == 6) xStartPos = -2;
			if (xStartPos == 7) xStartPos = -1;
			
			my = (y + (yScroll & 0x1ff)) / 8;
			
			if (my > 63) my -= 64;
		
			for (x = xStartPos; x < 320; x += 8) {
				mx = (x - (xScrollRow & 0x3ff) + 192) / 8;
				if (mx < 0) mx += 128;
				if (Page == 1) {
					if (my < 32 && mx < 64) ActPage = BootlegBgPage[0];
					if (my < 32 && mx >= 64) ActPage = BootlegBgPage[1];
					if (my >= 32 && mx < 64) ActPage = BootlegBgPage[2];
					if (my >= 32 && mx >= 64) ActPage = BootlegBgPage[3];
				}
				if (Page == 0) {
					if (my < 32 && mx < 64) ActPage = BootlegFgPage[0];
					if (my < 32 && mx >= 64) ActPage = BootlegFgPage[1];
					if (my >= 32 && mx < 64) ActPage = BootlegFgPage[2];
					if (my >= 32 && mx >= 64) ActPage = BootlegFgPage[3];
				}
				
				TileIndex = 64 * 32 * 2 * ActPage + ((2 * 64 * my) & 0xfff) + ((2 * mx) & 0x7f);
				INT32 Data = (System16TileRam[TileIndex + 1] << 8) | System16TileRam[TileIndex + 0];
			
				Priority = (Data >> 15) & 1;
			
				if (Priority == PriorityDraw) {
					Code = Data & 0x1fff;
					Code = System16TileBanks[Code / System16TileBankSize] * System16TileBankSize + Code % System16TileBankSize;
					Code &= (System16NumTiles - 1);
					Colour = (Data >> 6) & 0x7f;
			
					ColourOff = System16TilemapColorOffset;
					if (Colour >= 0x20) ColourOff = 0x100 | System16TilemapColorOffset;
					if (Colour >= 0x40) ColourOff = 0x200 | System16TilemapColorOffset;
					if (Colour >= 0x60) ColourOff = 0x300 | System16TilemapColorOffset;
			
					if (Transparent) {
						if (x > 7 && x < 312 && y > 7 && y < 216) {
							Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 3, 0, ColourOff, System16Tiles);
						}
					} else {
						if (x > 7 && x < 312 && y > 7 && y < 216) {
							Render8x8Tile(pTransDraw, Code, x, y, Colour, 3, ColourOff, System16Tiles);
						} else {
							Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 3, ColourOff, System16Tiles);
						}
					}
				}
			}
		}
	}
}

static void System16ARenderTextLayer(INT32 PriorityDraw)
{
	INT32 mx, my, Code, Colour, x, y, Priority, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) { 
			Code = (System16TextRam[TileIndex + 1] << 8) | System16TextRam[TileIndex + 0];
			
			Priority = (Code >> 11) & 1;
			
			if (Priority == PriorityDraw) {
				Colour = (Code >> 8) & 0x07;
				Code &= 0xff;
			
				x = 8 * mx;
				y = 8 * my;
				
				x -= 192;
			
				if (x > 7 && x < 312 && y > 7 && y < 216) {
					Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 3, 0, System16TilemapColorOffset, System16Tiles);
				} else {
					Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 3, 0, System16TilemapColorOffset, System16Tiles);
				}
			}

			TileIndex +=2;
		}
	}
}

static void System16BRenderTextLayer(INT32 PriorityDraw)
{
	INT32 mx, my, Code, Colour, x, y, Priority, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = (System16TextRam[TileIndex + 1] << 8) | System16TextRam[TileIndex + 0];
			Priority = (Code >> 15) & 1;
			
			if (Priority == PriorityDraw) {			
				Colour = (Code >> 9) & 0x07;
				Code &= 0x1ff;
			
				Code += System16TileBanks[0] * 0x1000;
				
				Code &= (System16NumTiles - 1);
			
				x = 8 * mx;
				y = 8 * my;
			
				x -= 192;

				if (x > 7 && x < 312 && y > 7 && y < 216) {
					Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 3, 0, System16TilemapColorOffset, System16Tiles);
				} else {
					Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 3, 0, System16TilemapColorOffset, System16Tiles);
				}
			}

			TileIndex +=2 ;
		}
	}
}

static void System16BAltRenderTextLayer(INT32 PriorityDraw)
{
	INT32 mx, my, Code, Colour, x, y, Priority, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = (System16TextRam[TileIndex + 1] << 8) | System16TextRam[TileIndex + 0];
			Priority = (Code >> 15) & 1;
			
			if (Priority == PriorityDraw) {			
				Colour = (Code >> 8) & 0x07;
				Code &= 0xff;
			
				Code += System16TileBanks[0] * 0x1000;
				
				Code &= (System16NumTiles - 1);
			
				x = 8 * mx;
				y = 8 * my;
			
				x -= 192;

				if (x > 7 && x < 312 && y > 7 && y < 216) {
					Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 3, 0, System16TilemapColorOffset, System16Tiles);
				} else {
					Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 3, 0, System16TilemapColorOffset, System16Tiles);
				}
			}

			TileIndex +=2 ;
		}
	}
}

/*====================================================
Sprite Rendering
====================================================*/

static void System16DrawPixel(INT32 x, INT32 pix, INT32 colour, UINT16* pPixel)
{
	x += System16SpriteXOffset;
	if (x >= 0 && x <= 319 && pix != 0 && pix != 15) {
		if (colour == (0x3f << 4)) {
			pPixel[x] &= (System16PaletteEntries - 1);
			pPixel[x] += ((((System16PaletteRam[pPixel[x] + 1] << 8) | System16PaletteRam[pPixel[x] + 0]) & 0x8000) ? (System16PaletteEntries * 2) : System16PaletteEntries);
		} else {
			pPixel[x] = (pix | colour | 0x400) & (System16PaletteEntries - 1);
		}
	}
}

static void System16ARenderSpriteLayer(INT32 Priority)
{
	UINT8 numbanks = System16SpriteRomSize / 0x10000;

	const UINT16 *spritebase = (const UINT16*)System16Sprites;
	UINT16 *data;
		
	for (data = (UINT16*)System16SpriteRam; data < (UINT16*)System16SpriteRam + System16SpriteRamSize / 2; data += 8) {
		if ((BURN_ENDIAN_SWAP_INT16(data[0]) >> 8) > 0xf0) break;
		INT32 sprpri  = 1 << ((BURN_ENDIAN_SWAP_INT16(data[4]) >> 0) & 0x3);
				
#if 1 && defined FBA_DEBUG
		if (sprpri != 1 && sprpri != 2 && sprpri != 4 && sprpri != 8) bprintf(PRINT_IMPORTANT, _T("Unknown Sprite Priority - %x\n"), sprpri);
#endif
		
		if (sprpri != Priority) continue;
		
		INT32 bottom  = (BURN_ENDIAN_SWAP_INT16(data[0]) >> 8) + 1;
		INT32 top     = (BURN_ENDIAN_SWAP_INT16(data[0]) & 0xff) + 1;
		INT32 xpos    = (BURN_ENDIAN_SWAP_INT16(data[1]) & 0x1ff) - 0xbd;
		INT32 pitch   = (INT16)BURN_ENDIAN_SWAP_INT16(data[2]);
		UINT16 addr = BURN_ENDIAN_SWAP_INT16(data[3]);
		INT32 color   = ((BURN_ENDIAN_SWAP_INT16(data[4]) >> 8) & 0x3f) << 4;
		INT32 bank    = System16SpriteBanks[(BURN_ENDIAN_SWAP_INT16(data[4]) >> 4) & 0x7];
		const UINT16 *spritedata;
		INT32 x, y, pix, xdelta = 1;
		
		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if ((top >= bottom) || bank == 255)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x8000 * bank;
		
		if (!System16ScreenFlip) {
			for (y = top; y < bottom; y++) {
				addr += pitch;
				if (y >= 0 && y <= 223) {
					UINT16* pPixel = pTransDraw + (y * 320);
					if (!(addr & 0x8000)) {
						/* start at the word before because we preincrement below */
						data[7] = addr - 1;
						for (x = xpos; ((xpos - x) & 0x1ff) != 1; ) {
							UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[++data[7] & 0x7fff]);
					
							pix = (pixels >> 12) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >>  8) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >>  4) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >>  0) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
					
							if (pix == 15) break;
						}
					} else {
						data[7] = addr + 1;
						for (x = xpos; ((xpos - x) & 0x1ff) != 1; ) {
							UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[--data[7] & 0x7fff]);

							/* draw four pixels */
							pix = (pixels >>  0) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >>  4) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >>  8) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >> 12) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;

							/* stop if the last pixel in the group was 0xf */
							if (pix == 15) break;
						}
					}
				}
			}
		} else {
			for (y = bottom - 3; y > top - 3; y--) {
				addr += pitch;
				if (y >= 0 && y <= 223) {
					UINT16* pPixel = pTransDraw + (y * 320);
					if (!(addr & 0x8000)) {
						/* start at the word before because we preincrement below */
						data[7] = addr - 1;
						for (x = xpos; ((xpos - x) & 0x1ff) != 1; ) {
							UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[++data[7] & 0x7fff]);
					
							pix = (pixels >> 12) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >>  8) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >>  4) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >>  0) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
					
							if (pix == 15) break;
						}
					} else {
						data[7] = addr + 1;
						for (x = xpos; ((xpos - x) & 0x1ff) != 1; ) {
							UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[--data[7] & 0x7fff]);

							/* draw four pixels */
							pix = (pixels >>  0) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >>  4) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >>  8) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;
							pix = (pixels >> 12) & 0xf; System16DrawPixel(x, pix, color, pPixel); x += xdelta;

							/* stop if the last pixel in the group was 0xf */
							if (pix == 15) break;
						}
					}
				}
			}
		}
	}
}

static void System16BRenderSpriteLayer(INT32 Priority)
{
	UINT8 numbanks;
	const UINT16 *spritebase;
  	UINT16 *data;

	spritebase = (const UINT16 *)System16Sprites;
	numbanks = System16SpriteRomSize / 0x20000;

	for (data = (UINT16*)System16SpriteRam; data < (UINT16*)System16SpriteRam + System16SpriteRamSize / 2; data += 8) {
		if (BURN_ENDIAN_SWAP_INT16(data[2]) & 0x8000) break;
		INT32 sprpri  = 1 << ((BURN_ENDIAN_SWAP_INT16(data[4]) >> 6) & 0x3);
				
#if 1 && defined FBA_DEBUG
		if (sprpri != 1 && sprpri != 2 && sprpri != 4 && sprpri != 8) bprintf(PRINT_IMPORTANT, _T("Unknown Sprite Priority - %x\n"), sprpri);
#endif
		
		if (sprpri != Priority) continue;
		
		INT32 bottom  = BURN_ENDIAN_SWAP_INT16(data[0]) >> 8;
		INT32 top     = BURN_ENDIAN_SWAP_INT16(data[0]) & 0xff;
		INT32 xpos    = (BURN_ENDIAN_SWAP_INT16(data[1]) & 0x1ff) - 0xb8;
		INT32 hide    = BURN_ENDIAN_SWAP_INT16(data[2]) & 0x4000;
		INT32 flip    = BURN_ENDIAN_SWAP_INT16(data[2]) & 0x100;
		INT32 pitch   = (INT8)(BURN_ENDIAN_SWAP_INT16(data[2]) & 0xff);
		UINT16 addr = BURN_ENDIAN_SWAP_INT16(data[3]);
		INT32 bank    = System16SpriteBanks[(BURN_ENDIAN_SWAP_INT16(data[4]) >> 8) & 0xf];
		INT32 color   = ((BURN_ENDIAN_SWAP_INT16(data[4]) & 0x3f) << 4);
		INT32 vzoom   = (BURN_ENDIAN_SWAP_INT16(data[5]) >> 5) & 0x1f;
		INT32 hzoom   = BURN_ENDIAN_SWAP_INT16(data[5]) & 0x1f;
		const UINT16 *spritedata;
		INT32 x, y, pix, xdelta = 1;

		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if (hide || (top >= bottom) || bank == 255)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x10000 * bank;

		/* reset the yzoom counter */
		data[5] &= 0x03ff;
		
		if (!System16ScreenFlip) {
			/* loop from top to bottom */
			for (y = top; y < bottom; y++) {
				/* advance a row */
				addr += pitch;

				/* accumulate zoom factors; if we carry into the high bit, skip an extra row */
				data[5] += vzoom << 10;
				if (data[5] & 0x8000) {
					addr += pitch;
					data[5] &= ~0x8000;
				}
				
				/* skip drawing if not within the cliprect */
				if (y >= 0 && y <= 223) {
					UINT16* pPixel = pTransDraw + (y * 320);
					int xacc;

					/* compute the initial X zoom accumulator; this is verified on the real PCB */
					xacc = 4 * hzoom;

					/* non-flipped case */
					if (!flip) {
						/* start at the word before because we preincrement below */
						data[7] = addr - 1;
						for (x = xpos; ((xpos - x) & 0x1ff) != 1; ) {
							UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[++data[7]]);

							/* draw four pixels */
							pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }

							/* stop if the last pixel in the group was 0xf */
							if (pix == 15) break;
						}
					} else {
						/* start at the word after because we predecrement below */
						data[7] = addr + 1;
						for (x = xpos; ((xpos - x) & 0x1ff) != 1; ) {
							UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[--data[7]]);

							/* draw four pixels */
							pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }

							/* stop if the last pixel in the group was 0xf */
							if (pix == 15) break;
						}
					}
				}
			}
		} else {
			/* loop from top to bottom */
			for (y = bottom; y > top; y--) {
				/* advance a row */
				addr += pitch;

				/* accumulate zoom factors; if we carry into the high bit, skip an extra row */
				data[5] += vzoom << 10;
				if (data[5] & 0x8000) {
					addr += pitch;
					data[5] &= ~0x8000;
				}

				/* skip drawing if not within the cliprect */
				if (y >= 0 && y <= 223) {
					UINT16* pPixel = pTransDraw + (y * 320);
					int xacc;

					/* compute the initial X zoom accumulator; this is verified on the real PCB */
					xacc = 4 * hzoom;

					/* non-flipped case */
					if (!flip) {
						/* start at the word before because we preincrement below */
						data[7] = addr - 1;
						for (x = xpos; ((xpos - x) & 0x1ff) != 1; ) {
							UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[++data[7]]);

							/* draw four pixels */
							pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }

							/* stop if the last pixel in the group was 0xf */
							if (pix == 15) break;
						}
					} else {
						/* start at the word after because we predecrement below */
						data[7] = addr + 1;
						for (x = xpos; ((xpos - x) & 0x1ff) != 1; ) {
							UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[--data[7]]);

							/* draw four pixels */
							pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }
							pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { System16DrawPixel(x, pix, color, pPixel); x += xdelta; }

							/* stop if the last pixel in the group was 0xf */
							if (pix == 15) break;
						}
					}
				}
			}

		}
	}
}

inline static void OutrunDrawPixel(INT32 x, INT32 pix, INT32 colour, INT32 shadow, UINT16* pPixel)
{
	if (x >= 0 && x <= 319 && pix != 0 && pix != 15) {
		if (shadow && pix == 0xa) {
			pPixel[x] &= (System16PaletteEntries - 1);
			pPixel[x] += ((((System16PaletteRam[pPixel[x] + 1] << 8) | System16PaletteRam[pPixel[x] + 0]) & 0x8000) ? System16PaletteEntries * 2 : System16PaletteEntries);
		} else {
			pPixel[x] = (pix | colour | 0x800) & (System16PaletteEntries - 1);
		}
	}
}

static void OutrunRenderSpriteLayer(INT32 Priority)
{
	UINT8 numbanks = System16SpriteRomSize / 0x40000;
	const UINT32 *spritebase = (const UINT32 *)System16Sprites;
	UINT16 *data;

	for (data = (UINT16*)System16SpriteRamBuff; data < (UINT16*)System16SpriteRamBuff + System16SpriteRamSize / 2; data += 8) {
		if (BURN_ENDIAN_SWAP_INT16(data[0]) & 0x8000) break;
		INT32 sprpri  = 1 << ((BURN_ENDIAN_SWAP_INT16(data[3]) >> 12) & 3);
				
#if 1 && defined FBA_DEBUG
		if (sprpri != 1 && sprpri != 2 && sprpri != 4 && sprpri != 8) bprintf(PRINT_IMPORTANT, _T("Unknown Sprite Priority - %x\n"), sprpri);
#endif
		if (sprpri != Priority) continue;
		
		INT32 hide    = (BURN_ENDIAN_SWAP_INT16(data[0]) & 0x5000);
		INT32 bank    = (BURN_ENDIAN_SWAP_INT16(data[0]) >> 9) & 7;
		INT32 top     = (BURN_ENDIAN_SWAP_INT16(data[0]) & 0x1ff) - 0x100;
		UINT16 addr = BURN_ENDIAN_SWAP_INT16(data[1]);
		INT32 pitch   = (INT16)((BURN_ENDIAN_SWAP_INT16(data[2]) >> 1) | ((BURN_ENDIAN_SWAP_INT16(data[4]) & 0x1000) << 3)) >> 8;
		INT32 xpos    = BURN_ENDIAN_SWAP_INT16(data[2]) & 0x1ff;
		INT32 shadow  = (BURN_ENDIAN_SWAP_INT16(data[3]) >> 14) & 1;
		INT32 vzoom   = BURN_ENDIAN_SWAP_INT16(data[3]) & 0x7ff;
		INT32 ydelta  = (BURN_ENDIAN_SWAP_INT16(data[4]) & 0x8000) ? 1 : -1;
		INT32 flip    = (~(BURN_ENDIAN_SWAP_INT16(data[4])) >> 14) & 1;
		INT32 xdelta  = (BURN_ENDIAN_SWAP_INT16(data[4]) & 0x2000) ? 1 : -1;
		INT32 hzoom   = BURN_ENDIAN_SWAP_INT16(data[4]) & 0x7ff;
		INT32 height  = (BURN_ENDIAN_SWAP_INT16(data[5]) >> 8) + 1;
		INT32 color   = (BURN_ENDIAN_SWAP_INT16(data[5]) & 0x7f) << 4;
		INT32 x, y, ytarget, yacc = 0, pix;
		const UINT32 *spritedata;

		/* adjust X coordinate */
		/* note: the threshhold below is a guess. If it is too high, rachero will draw garbage */
		/* If it is too low, smgp won't draw the bottom part of the road */
		if (xpos < 0x80 && xdelta < 0)
			xpos += 0x200;
		xpos -= 0xbe;

		/* initialize the end address to the start address */
		data[7] = BURN_ENDIAN_SWAP_INT16(addr);

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if (hide || height == 0)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x10000 * bank;

		/* clamp to a maximum of 8x (not 100% confirmed) */
		if (vzoom < 0x40) vzoom = 0x40;
		if (hzoom < 0x40) hzoom = 0x40;

		/* loop from top to bottom */
		ytarget = top + ydelta * height;
		for (y = top; y != ytarget; y += ydelta)
		{
			/* skip drawing if not within the cliprect */
			if (y >= 0 && y <= 223) {
				UINT16* pPixel = pTransDraw + (y * 320);
				INT32 xacc = 0;

				/* non-flipped case */
				if (!flip)
				{
					/* start at the word before because we preincrement below */
					data[7] = addr - 1;
					for (x = xpos; (xdelta > 0 && x <= 319) || (xdelta < 0 && x >= 0); )
					{
						UINT32 pixels = BURN_ENDIAN_SWAP_INT32(spritedata[++data[7]]);

						/* draw four pixels */
						pix = (pixels >> 28) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 24) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 20) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 16) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 12) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  8) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  4) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  0) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;

						/* stop if the second-to-last pixel in the group was 0xf */
						if ((pixels & 0x000000f0) == 0x000000f0)
							break;
					}
				}

				/* flipped case */
				else
				{
					/* start at the word after because we predecrement below */
					data[7] = addr + 1;
					for (x = xpos; (xdelta > 0 && x <= 319) || (xdelta < 0 && x >= 0); )
					{
						UINT32 pixels = BURN_ENDIAN_SWAP_INT32(spritedata[--data[7]]);

						/* draw four pixels */
						pix = (pixels >>  0) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  4) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  8) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 12) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 16) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 20) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 24) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 28) & 0xf; while (xacc < 0x200) { OutrunDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;

						/* stop if the second-to-last pixel in the group was 0xf */
						if ((pixels & 0x0f000000) == 0x0f000000)
							break;
					}
				}
			}

			/* accumulate zoom factors; if we carry into the high bit, skip an extra row */
			yacc += vzoom;
			addr += pitch * (yacc >> 9);
			yacc &= 0x1ff;
		}
	}
}

inline static void HangonDrawPixel(INT32 x, INT32 pix, INT32 colour, INT32 shadow, UINT16* pPixel)
{
	if (x >= 0 && x <= 319 && pix != 0 && pix != 15) {
		if (shadow && pix == 0xa) {
			pPixel[x] &= (System16PaletteEntries - 1);
			pPixel[x] += ((((System16PaletteRam[pPixel[x] + 1] << 8) | System16PaletteRam[pPixel[x] + 0]) & 0x8000) ? System16PaletteEntries * 2 : System16PaletteEntries);
		} else {
			pPixel[x] = (pix | colour | 0x400) & (System16PaletteEntries - 1);
		}
	}
}

static void HangonRenderSpriteLayer(INT32 Priority)
{
	UINT8 numbanks = System16SpriteRomSize / 0x20000;
	const UINT32 *spritebase = (UINT32*)System16Sprites;
	const UINT8 *zoom = System16Prom;
	UINT16 *data;

	for (data = (UINT16*)System16SpriteRam; data < (UINT16*)System16SpriteRam + System16SpriteRamSize / 2; data += 8) {
		if ((BURN_ENDIAN_SWAP_INT16(data[0]) >> 8) > 0xf0) break;	
		INT32 sprpri  = ((BURN_ENDIAN_SWAP_INT16(data[2]) >> 14) & 1) ? (1<<3) : (1<<1);
		
#if 1 && defined FBA_DEBUG
		if (sprpri != 1 && sprpri != 2 && sprpri != 4 && sprpri != 8) bprintf(PRINT_IMPORTANT, _T("Unknown Sprite Priority - %x\n"), sprpri);
#endif
		
		if (sprpri != Priority) continue;
		
		INT32 bottom  = (BURN_ENDIAN_SWAP_INT16(data[0]) >> 8) + 1;
		INT32 top     = (BURN_ENDIAN_SWAP_INT16(data[0]) & 0xff) + 1;
		INT32 bank    = System16SpriteBanks[(BURN_ENDIAN_SWAP_INT16(data[1]) >> 12) & 0x7];
		INT32 xpos    = (BURN_ENDIAN_SWAP_INT16(data[1]) & 0x1ff) - 0xbd;
		INT32 shadow  = (~(BURN_ENDIAN_SWAP_INT16(data[2])) >> 15) & 1;
		INT32 color   = (((BURN_ENDIAN_SWAP_INT16(data[2]) >> 8) & 0x3f) << 4);
		INT32 pitch   = (INT16)(BURN_ENDIAN_SWAP_INT16(data[2]) << 9) >> 9;
		UINT16 addr = BURN_ENDIAN_SWAP_INT16(data[3]);
		INT32 hzoom   = ((BURN_ENDIAN_SWAP_INT16(data[4]) >> 8) & 0x3f) << 1;
		INT32 vzoom   = (BURN_ENDIAN_SWAP_INT16(data[4]) >> 0) & 0x3f;
		INT32 x, y, pix, zaddr, zmask;
		const UINT32 *spritedata;
		
		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if ((top >= bottom) || bank == 255)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x8000 * bank;
		
		/* determine the starting zoom address and mask */
		zaddr = (vzoom & 0x38) << 5;
		zmask = 1 << (vzoom & 7);
		
		for (y = top; y < bottom; y++) {
			addr += pitch;
			
			/* if the zoom bit says so, add pitch a second time */
			if (zoom[zaddr++] & zmask)
				addr += pitch;
				
			if (y >= 0 && y <= 223) {
				UINT16* pPixel = pTransDraw + (y * 320);
				INT32 xacc = 0x00;
				
				if (!(addr & 0x8000)) {
					/* start at the word before because we preincrement below */
					data[7] = addr - 1;
					for (x = xpos; x <= 319; ) {
						UINT32 pixels = BURN_ENDIAN_SWAP_INT32(spritedata[++data[7] & 0x7fff]);
					
						pix = (pixels >> 28) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >> 24) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >> 20) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >> 16) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
					
						if (pix == 15) break;
					}
				} else {
					data[7] = addr + 1;
					for (x = xpos; x <= 319; ) {
						UINT32 pixels = BURN_ENDIAN_SWAP_INT32(spritedata[--data[7] & 0x7fff]);

						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >> 16) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >> 20) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >> 24) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }
						pix = (pixels >> 28) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonDrawPixel(x, pix, color, shadow, pPixel); x++; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15) break;
					}
				}
			}
		}
	}
}

inline static void HangonAltDrawPixel(INT32 x, INT32 pix, INT32 colour, UINT16* pPixel)
{
	if (x >= 0 && x <= 319 && pix != 0 && pix != 15) {
		if (colour == (0x3f << 4)) {
			pPixel[x] &= (System16PaletteEntries - 1);
			pPixel[x] += ((System16SpriteShadow) ? System16PaletteEntries * 2 : System16PaletteEntries);
		} else {
			pPixel[x] = (pix | colour | 0x400) & (System16PaletteEntries - 1);
		}
	}
}

static void HangonAltRenderSpriteLayer(INT32 Priority)
{
	UINT8 numbanks = System16SpriteRomSize / 0x10000;
	const UINT16 *spritebase = (UINT16*)System16Sprites;
	const UINT8 *zoom = System16Prom;
	UINT16 *data;
	
	for (data = (UINT16*)System16SpriteRam; data < (UINT16*)System16SpriteRam + System16SpriteRamSize / 2; data += 8) {
		if ((BURN_ENDIAN_SWAP_INT16(data[0]) >> 8) > 0xf0) break;
		INT32 sprpri  = 1 << ((BURN_ENDIAN_SWAP_INT16(data[4]) >> 0) & 0x3);
		
#if 1 && defined FBA_DEBUG
		if (sprpri != 1 && sprpri != 2 && sprpri != 4 && sprpri != 8) bprintf(PRINT_IMPORTANT, _T("Unknown Sprite Priority - %x\n"), sprpri);
#endif
		
		if (sprpri != Priority) continue;
		
		INT32 bottom  = (BURN_ENDIAN_SWAP_INT16(data[0]) >> 8) + 1;
		INT32 top     = (BURN_ENDIAN_SWAP_INT16(data[0]) & 0xff) + 1;
		INT32 bank    = System16SpriteBanks[(BURN_ENDIAN_SWAP_INT16(data[1]) >> 12) & 0xf];
		INT32 xpos    = (BURN_ENDIAN_SWAP_INT16(data[1]) & 0x1ff) - 0xbd;
		INT32 pitch   = (INT16)BURN_ENDIAN_SWAP_INT16(data[2]);
		UINT16 addr = BURN_ENDIAN_SWAP_INT16(data[3]);
		INT32 color   = (((BURN_ENDIAN_SWAP_INT16(data[4]) >> 8) & 0x3f) << 4);
		INT32 vzoom   = (BURN_ENDIAN_SWAP_INT16(data[4]) >> 2) & 0x3f;
		INT32 hzoom   = vzoom << 1;
		
		INT32 x, y, pix, zaddr, zmask;
		const UINT16 *spritedata;

		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if ((top >= bottom) || bank == 255)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x8000 * bank;

		/* determine the starting zoom address and mask */
		zaddr = (vzoom & 0x38) << 5;
		zmask = 1 << (vzoom & 7);

		/* loop from top to bottom */
		for (y = top; y < bottom; y++)
		{
			/* advance a row */
			addr += pitch;

			/* if the zoom bit says so, add pitch a second time */
			if (zoom[zaddr++] & zmask)
				addr += pitch;

			/* skip drawing if not within the cliprect */
			if (y >= 0 && y <= 223)
			{
				UINT16* pPixel = pTransDraw + (y * 320);
				INT32 xacc = 0x00;

				/* note that the System 16A sprites have a design flaw that allows the address */
				/* to carry into the flip flag, which is the topmost bit -- it is very important */
				/* to emulate this as the games compensate for it */

				/* non-flipped case */
				if (!(addr & 0x8000))
				{
					/* start at the word before because we preincrement below */
					data[7] = addr - 1;
					for (x = xpos; x <= 319; )
					{
						UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[++data[7] & 0x7fff]);

						/* draw four pixels */
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonAltDrawPixel(x, pix, color, pPixel); x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonAltDrawPixel(x, pix, color, pPixel); x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonAltDrawPixel(x, pix, color, pPixel); x++; }
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonAltDrawPixel(x, pix, color, pPixel); x++; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}

				/* flipped case */
				else
				{
					/* start at the word after because we predecrement below */
					data[7] = addr + 1;
					for (x = xpos; x <= 319; )
					{
						UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[--data[7] & 0x7fff]);

						/* draw four pixels */
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonAltDrawPixel(x, pix, color, pPixel); x++; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonAltDrawPixel(x, pix, color, pPixel); x++; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonAltDrawPixel(x, pix, color, pPixel); x++; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0xff) + hzoom; if (xacc < 0x100) { if (x >= 0) HangonAltDrawPixel(x, pix, color, pPixel); x++; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15)
							break;
					}
				}
			}
		}
	}
}

inline static void BoardXDrawPixel(INT32 x, INT32 pix, INT32 colour, INT32 shadow, UINT16* pPixel)
{
	if (x >= 0 && x <= 319 && pix != 0 && pix != 15) {
		if (shadow && pix == 0xa) {
			pPixel[x] &= (System16PaletteEntries - 1);
			pPixel[x] += ((((System16PaletteRam[pPixel[x] + 1] << 8) | System16PaletteRam[pPixel[x] + 0]) & 0x8000) ? System16PaletteEntries * 2 : System16PaletteEntries);
		} else {
			pPixel[x] = (pix | colour) & (System16PaletteEntries - 1);
		}
	}
}

static void XBoardRenderSpriteLayer(INT32 Priority)
{
	UINT8 numbanks = System16SpriteRomSize / 0x40000;
	const UINT32 *spritebase = (const UINT32 *)System16Sprites;
	UINT16 *data;

	for (data = (UINT16*)System16SpriteRamBuff; data < (UINT16*)System16SpriteRamBuff + System16SpriteRamSize / 2; data += 8) {
		if (BURN_ENDIAN_SWAP_INT16(data[0]) & 0x8000) break;
		INT32 sprpri  = 1 << ((BURN_ENDIAN_SWAP_INT16(data[3]) >> 12) & 3);
		
#if 1 && defined FBA_DEBUG
		if (sprpri != 1 && sprpri != 2 && sprpri != 4 && sprpri != 8) bprintf(PRINT_IMPORTANT, _T("Unknown Sprite Priority - %x\n"), sprpri);
#endif
		if (sprpri != Priority) continue;
		
		INT32 hide    = (BURN_ENDIAN_SWAP_INT16(data[0]) & 0x5000);
		INT32 bank    = (BURN_ENDIAN_SWAP_INT16(data[0]) >> 9) & 7;
		INT32 top     = (BURN_ENDIAN_SWAP_INT16(data[0]) & 0x1ff) - 0x100;
		UINT16 addr = BURN_ENDIAN_SWAP_INT16(data[1]);
		INT32 pitch   = (INT16)((BURN_ENDIAN_SWAP_INT16(data[2]) >> 1) | ((BURN_ENDIAN_SWAP_INT16(data[4]) & 0x1000) << 3)) >> 8;
		INT32 xpos    = BURN_ENDIAN_SWAP_INT16(data[2]) & 0x1ff;
		INT32 shadow  = (BURN_ENDIAN_SWAP_INT16(data[3]) >> 14) & 1;
		INT32 vzoom   = BURN_ENDIAN_SWAP_INT16(data[3]) & 0x7ff;
		INT32 ydelta  = (BURN_ENDIAN_SWAP_INT16(data[4]) & 0x8000) ? 1 : -1;
		INT32 flip    = (~(BURN_ENDIAN_SWAP_INT16(data[4])) >> 14) & 1;
		INT32 xdelta  = (BURN_ENDIAN_SWAP_INT16(data[4]) & 0x2000) ? 1 : -1;
		INT32 hzoom   = BURN_ENDIAN_SWAP_INT16(data[4]) & 0x7ff;
		INT32 height  = (BURN_ENDIAN_SWAP_INT16(data[5]) & 0xfff) + 1;
		INT32 color   = (BURN_ENDIAN_SWAP_INT16(data[6]) & 0xff) << 4;
		INT32 x, y, ytarget, yacc = 0, pix;
		const UINT32 *spritedata;

		/* adjust X coordinate */
		/* note: the threshhold below is a guess. If it is too high, rachero will draw garbage */
		/* If it is too low, smgp won't draw the bottom part of the road */
		if (xpos < 0x80 && xdelta < 0)
			xpos += 0x200;
		xpos -= 0xbe;

		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if (hide || height == 0)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x10000 * bank;

		/* clamp to a maximum of 8x (not 100% confirmed) */
		if (vzoom < 0x40) vzoom = 0x40;
		if (hzoom < 0x40) hzoom = 0x40;

		/* loop from top to bottom */
		ytarget = top + ydelta * height;
		for (y = top; y != ytarget; y += ydelta)
		{
			/* skip drawing if not within the cliprect */
			if (y >= 0 && y <= 223) {
				UINT16* pPixel = pTransDraw + (y * 320);
				INT32 xacc = 0;

				/* non-flipped case */
				if (!flip)
				{
					/* start at the word before because we preincrement below */
					data[7] = addr - 1;
					for (x = xpos; (xdelta > 0 && x <= 319) || (xdelta < 0 && x >= 0); )
					{
						UINT32 pixels = BURN_ENDIAN_SWAP_INT32(spritedata[++data[7]]);
						
						/* draw four pixels */
						pix = (pixels >> 28) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 24) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 20) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 16) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 12) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  8) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  4) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  0) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;

						/* stop if the second-to-last pixel in the group was 0xf */
						if ((pixels & 0x000000f0) == 0x000000f0)
							break;
					}
				}

				/* flipped case */
				else
				{
					/* start at the word after because we predecrement below */
					data[7] = addr + 1;
					for (x = xpos; (xdelta > 0 && x <= 319) || (xdelta < 0 && x >= 0); )
					{
						UINT32 pixels = BURN_ENDIAN_SWAP_INT32(spritedata[--data[7]]);
						
						/* draw four pixels */
						pix = (pixels >>  0) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  4) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  8) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 12) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 16) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 20) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 24) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 28) & 0xf; while (xacc < 0x200) { BoardXDrawPixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;

						/* stop if the second-to-last pixel in the group was 0xf */
						if ((pixels & 0x0f000000) == 0x0f000000)
							break;
					}
				}
			}

			/* accumulate zoom factors; if we carry into the high bit, skip an extra row */
			yacc += vzoom;
			addr += pitch * (yacc >> 9);
			yacc &= 0x1ff;
		}
	}
}

inline static void YBoardSystem16BDrawPixel(INT32 x, INT32 pix, INT32 colour, UINT16* pPixel)
{
	if (x >= 0 && x <= 319 && pix != 0 && pix != 15) {
		if (pix == 14) {
			pPixel[x] &= (System16PaletteEntries - 1);
			pPixel[x] += ((((System16PaletteRam[pPixel[x] + 1] << 8) | System16PaletteRam[pPixel[x] + 0]) & 0x8000) ? (System16PaletteEntries * 2) : System16PaletteEntries);
		} else {
			pPixel[x] = (pix | colour | 0x800) & (System16PaletteEntries - 1);
		}
	}
}

static void YBoardSystem16BRenderSpriteLayer()
{
	UINT8 numbanks;
	const UINT16 *spritebase;
  	UINT16 *data;

	spritebase = (const UINT16 *)System16Sprites;
	numbanks = System16SpriteRomSize / 0x20000;

	for (data = (UINT16*)System16SpriteRam; data < (UINT16*)System16SpriteRam + System16SpriteRamSize / 2; data += 8) {
		if (BURN_ENDIAN_SWAP_INT16(data[2]) & 0x8000) break;
		//INT32 sprpri  = (data[1] >> 8) & 0x1e;
				
		INT32 bottom  = BURN_ENDIAN_SWAP_INT16(data[0]) >> 8;
		INT32 top     = BURN_ENDIAN_SWAP_INT16(data[0]) & 0xff;
		INT32 xpos    = (BURN_ENDIAN_SWAP_INT16(data[1]) & 0x1ff) - 0xb8;
		INT32 hide    = BURN_ENDIAN_SWAP_INT16(data[2]) & 0x4000;
		INT32 flip    = BURN_ENDIAN_SWAP_INT16(data[2]) & 0x100;
		INT32 pitch   = (INT8)(BURN_ENDIAN_SWAP_INT16(data[2]) & 0xff);
		UINT16 addr = BURN_ENDIAN_SWAP_INT16(data[3]);
		INT32 bank    = System16SpriteBanks[(BURN_ENDIAN_SWAP_INT16(data[4]) >> 8) & 0xf];
		INT32 color   = ((BURN_ENDIAN_SWAP_INT16(data[4]) & 0x7f) << 4);
		INT32 vzoom   = (BURN_ENDIAN_SWAP_INT16(data[5]) >> 5) & 0x1f;
		INT32 hzoom   = BURN_ENDIAN_SWAP_INT16(data[5]) & 0x1f;
		const UINT16 *spritedata;
		INT32 x, y, pix, xdelta = 1;
		
		/* initialize the end address to the start address */
		data[7] = addr;

		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if (hide || (top >= bottom) || bank == 255)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x10000 * bank;

		/* reset the yzoom counter */
		data[5] &= 0x03ff;

		/* loop from top to bottom */
		for (y = top; y < bottom; y++) {
			/* advance a row */
			addr += pitch;

			/* accumulate zoom factors; if we carry into the high bit, skip an extra row */
			data[5] += vzoom << 10;
			if (data[5] & 0x8000) {
				addr += pitch;
				data[5] &= ~0x8000;
			}

			/* skip drawing if not within the cliprect */
			if (y >= 0 && y <= 223) {
				UINT16* pPixel = pTransDraw + (y * 320);
				INT32 xacc;

				/* compute the initial X zoom accumulator; this is verified on the real PCB */
				xacc = 4 * hzoom;

				/* non-flipped case */
				if (!flip) {
					/* start at the word before because we preincrement below */
					data[7] = addr - 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; ) {
						UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[++data[7]]);

						/* draw four pixels */
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { YBoardSystem16BDrawPixel(x, pix, color, pPixel); x += xdelta; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { YBoardSystem16BDrawPixel(x, pix, color, pPixel); x += xdelta; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { YBoardSystem16BDrawPixel(x, pix, color, pPixel); x += xdelta; }
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { YBoardSystem16BDrawPixel(x, pix, color, pPixel); x += xdelta; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15) break;
					}
				} else {
					/* start at the word after because we predecrement below */
					data[7] = addr + 1;
					for (x = xpos; ((xpos - x) & 0x1ff) != 1; ) {
						UINT16 pixels = BURN_ENDIAN_SWAP_INT16(spritedata[--data[7]]);

						/* draw four pixels */
						pix = (pixels >>  0) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { YBoardSystem16BDrawPixel(x, pix, color, pPixel); x += xdelta; }
						pix = (pixels >>  4) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { YBoardSystem16BDrawPixel(x, pix, color, pPixel); x += xdelta; }
						pix = (pixels >>  8) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { YBoardSystem16BDrawPixel(x, pix, color, pPixel); x += xdelta; }
						pix = (pixels >> 12) & 0xf; xacc = (xacc & 0x3f) + hzoom; if (xacc < 0x40) { YBoardSystem16BDrawPixel(x, pix, color, pPixel); x += xdelta; }

						/* stop if the last pixel in the group was 0xf */
						if (pix == 15) break;
					}
				}
			}
		}
	}
}

inline static void YBoardDrawPixel(INT32 x, UINT32 ind, UINT32 colourpri, UINT16* pPixel)
{
	if (x >= 0 && x <= 511 && ind < 0x1fe) {
		pPixel[x] = (ind | colourpri | 0x1000)/* & (System16PaletteEntries - 1)*/;
	}
}

static void YBoardRenderSpriteLayer()
{
	UINT8 numbanks = System16Sprite2RomSize / 0x80000;
	const UINT64 *spritebase = (const UINT64 *)System16Sprites2;
	const UINT16 *rotatebase = (UINT16*)System16RotateRamBuff;
	UINT8 visited[0x1000];
	UINT16 *data;
	INT32 next = 0;

	/* reset the visited list */
	memset(visited, 0, sizeof(visited));

	for (INT32 y = 0; y <= 511; y++)
		if (!(BURN_ENDIAN_SWAP_INT16(rotatebase[y & ~1]) & 0xc000))
			memset(pTempDraw + (y * 512), 0xff, 512 * sizeof(UINT16));

	for (data = (UINT16*)System16SpriteRam2; !(BURN_ENDIAN_SWAP_INT16(data[0]) & 0x8000) && !visited[next]; data = (UINT16*)System16SpriteRam2 + next * 8)
	{
		INT32 hide    = (BURN_ENDIAN_SWAP_INT16(data[0]) & 0x5000);
		UINT16 *indirect = (UINT16*)System16SpriteRam2 + ((BURN_ENDIAN_SWAP_INT16(data[0]) & 0x7ff) << 4);
		INT32 bank    = ((BURN_ENDIAN_SWAP_INT16(data[1]) >> 8) & 0x10) | ((BURN_ENDIAN_SWAP_INT16(data[2]) >> 12) & 0x0f);
		INT32 xpos    = (BURN_ENDIAN_SWAP_INT16(data[1]) & 0xfff) - 0x600;
		INT32 top     = (BURN_ENDIAN_SWAP_INT16(data[2]) & 0xfff) - 0x600;
		UINT16 addr = BURN_ENDIAN_SWAP_INT16(data[3]);
		INT32 height  = BURN_ENDIAN_SWAP_INT16(data[4]);
		INT32 ydelta  = (BURN_ENDIAN_SWAP_INT16(data[5]) & 0x4000) ? 1 : -1;
		INT32 flip    = (~(BURN_ENDIAN_SWAP_INT16(data[5])) >> 13) & 1;
		INT32 xdelta  = (BURN_ENDIAN_SWAP_INT16(data[5]) & 0x1000) ? 1 : -1;
		INT32 zoom    = BURN_ENDIAN_SWAP_INT16(data[5]) & 0x7ff;
		INT32 colorpri= (BURN_ENDIAN_SWAP_INT16(data[6]) << 1) & 0xfe00;
		INT32 pitch   = (INT8)BURN_ENDIAN_SWAP_INT16(data[6]);
		INT32 x, y, ytarget, yacc = 0, pix, ind;
		const UINT64 *spritedata;
		UINT16 offs;

		/* note that we've visited this entry and get the offset of the next one */
		visited[next] = 1;
		next = BURN_ENDIAN_SWAP_INT16(data[7]) & 0xfff;
		
		/* if hidden, or top greater than/equal to bottom, or invalid bank, punt */
		if (hide || height == 0)
			continue;

		/* clamp to within the memory region size */
		if (numbanks)
			bank %= numbanks;
		spritedata = spritebase + 0x10000 * bank;

		/* clamp to a maximum of 8x (not 100% confirmed) */
		if (zoom == 0) zoom = 1;
		
		/* loop from top to bottom */
		ytarget = top + ydelta * height;
		for (y = top; y != ytarget; y += ydelta)
		{
			/* skip drawing if not within the cliprect */
			if (y >= 0 && y <= 511)
			{
				//UINT16* pPixel = pTransDraw + (y * 320);
				UINT16* pPixel = pTempDraw + (y * 512);
				INT32 minx = BURN_ENDIAN_SWAP_INT16(rotatebase[y & ~1]);
				INT32 maxx = BURN_ENDIAN_SWAP_INT16(rotatebase[y |  1]);
				INT32 xacc = 0;

				/* bit 0x8000 from rotate RAM means that Y is above the top of the screen */
				if ((minx & 0x8000) && ydelta < 0)
					break;

				/* bit 0x4000 from rotate RAM means that Y is below the bottom of the screen */
				if ((minx & 0x4000) && ydelta > 0)
					break;

				/* if either bit is set, skip the rest for this scanline */
				if (!(minx & 0xc000))
				{
					/* clamp min/max to the cliprect */
					minx -= 0x600;
					maxx -= 0x600;
					if (minx < 0)
						minx = 0;
					if (maxx > 511)
						maxx = 511;

					/* non-flipped case */
					if (!flip)
					{
						/* start at the word before because we preincrement below */
						offs = addr - 1;
						for (x = xpos; (xdelta > 0 && x <= maxx) || (xdelta < 0 && x >= minx); )
						{
							UINT64 pixels = BURN_ENDIAN_SWAP_INT64(spritedata[++offs]);

							/* draw four pixels */
							pix = (pixels >> 60) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 56) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 52) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 48) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 44) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 40) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 36) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 32) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 28) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 24) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 20) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 16) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 12) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  8) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  4) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  0) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							
							/* stop if the second-to-last pixel in the group was 0xf */
							if (pix == 0x0f)
								break;
						}
					}

					/* flipped case */
					else
					{
						/* start at the word after because we predecrement below */
						offs = addr + 1;
						for (x = xpos; (xdelta > 0 && x <= maxx) || (xdelta < 0 && x >= minx); )
						{
							UINT64 pixels = BURN_ENDIAN_SWAP_INT64(spritedata[--offs]);

							/* draw four pixels */
							pix = (pixels >>  0) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  4) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >>  8) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 12) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 16) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 20) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 24) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 28) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 32) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 36) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 40) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 44) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 48) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 52) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 56) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							pix = (pixels >> 60) & 0xf; ind = BURN_ENDIAN_SWAP_INT16(indirect[pix]); while (xacc < 0x200) { YBoardDrawPixel(x, ind, colorpri, pPixel); x += xdelta; xacc += zoom; } xacc -= 0x200;
							
							/* stop if the second-to-last pixel in the group was 0xf */
							if (pix == 0x0f)
								break;
						}
					}
				}
			}

			/* accumulate zoom factors; if we carry into the high bit, skip an extra row */
			yacc += zoom;
			addr += pitch * (yacc >> 9);
			yacc &= 0x1ff;
		}
	}
}

/*====================================================
Road Rendering
====================================================*/

static void OutrunRenderRoadBackgroundLayer()
{
	UINT16 *roadram = (UINT16*)System16RoadRam;
	INT32 x, y;
	
	for (y = 0; y < 224; y++) {
		UINT16* pPixel = pTransDraw + (y * 320);
		INT32 data0 = BURN_ENDIAN_SWAP_INT16(roadram[0x000 + y]);
		INT32 data1 = BURN_ENDIAN_SWAP_INT16(roadram[0x100 + y]);
		
		INT32 color = -1;
		
		switch (System16RoadControl & 3) {
			case 0: {
				if (data0 & 0x800) color = data0 & 0x7f;
				break;
			}

			case 1: {
				if (data0 & 0x800) color = data0 & 0x7f;
				else if (data1 & 0x800) color = data1 & 0x7f;
				break;
			}

			case 2: {
				if (data1 & 0x800) color = data1 & 0x7f;
				else if (data0 & 0x800)	color = data0 & 0x7f;
				break;
			}

			case 3: {
				if (data1 & 0x800) color = data1 & 0x7f;
				break;
			}
		}

		if (color != -1) {
			for (x = 0; x < 320; x++) {
				pPixel[x] = color | System16RoadColorOffset3;
			}
		}
	}
}

static void OutrunRenderRoadForegroundLayer()
{
	UINT16 *roadram = (UINT16*)System16RoadRam;
	INT32 x, y;
	
	for (y = 0; y < 224; y++) {
		static const UINT8 priority_map[2][8] =	{
			{ 0x80,0x81,0x81,0x87,0,0,0,0x00 },
			{ 0x81,0x81,0x81,0x8f,0,0,0,0x80 }
		};
	
		UINT16* pPixel = pTransDraw + (y * 320);
		INT32 data0 = BURN_ENDIAN_SWAP_INT16(roadram[0x000 + y]);
		INT32 data1 = BURN_ENDIAN_SWAP_INT16(roadram[0x100 + y]);
		
		/* if both roads are low priority, skip */
		if ((data0 & 0x800) && (data1 & 0x800))	continue;
		
		INT32 hpos0, hpos1, color0, color1;
		INT32 control = System16RoadControl & 3;
		UINT16 color_table[32];
		UINT8 *src0, *src1;
		UINT8 bgcolor;

		/* get road 0 data */
		src0 = (data0 & 0x800) ? System16Roads + 256 * 2 * 512 : (System16Roads + (0x000 + ((data0 >> 1) & 0xff)) * 512);
		hpos0 = BURN_ENDIAN_SWAP_INT16((roadram[0x200 + ((System16RoadControl & 4) ? y : (data0 & 0x1ff))])) & 0xfff;
		color0 = BURN_ENDIAN_SWAP_INT16(roadram[0x600 + ((System16RoadControl & 4) ? y : (data0 & 0x1ff))]);

		/* get road 1 data */
		src1 = (data1 & 0x800) ? System16Roads + 256 * 2 * 512 : (System16Roads + (0x100 + ((data1 >> 1) & 0xff)) * 512);
		hpos1 = BURN_ENDIAN_SWAP_INT16((roadram[0x400 + ((System16RoadControl & 4) ? (0x100 + y) : (data1 & 0x1ff))])) & 0xfff;
		color1 = BURN_ENDIAN_SWAP_INT16(roadram[0x600 + ((System16RoadControl & 4) ? (0x100 + y) : (data1 & 0x1ff))]);

		/* determine the 5 colors for road 0 */
		color_table[0x00] = System16RoadColorOffset1 ^ 0x00 ^ ((color0 >> 0) & 1);
		color_table[0x01] = System16RoadColorOffset1 ^ 0x02 ^ ((color0 >> 1) & 1);
		color_table[0x02] = System16RoadColorOffset1 ^ 0x04 ^ ((color0 >> 2) & 1);
		bgcolor = (color0 >> 8) & 0xf;
		color_table[0x03] = (data0 & 0x200) ? color_table[0x00] : (System16RoadColorOffset2 ^ 0x00 ^ bgcolor);
		color_table[0x07] = System16RoadColorOffset1 ^ 0x06 ^ ((color0 >> 3) & 1);

		/* determine the 5 colors for road 1 */
		color_table[0x10] = System16RoadColorOffset1 ^ 0x08 ^ ((color1 >> 4) & 1);
		color_table[0x11] = System16RoadColorOffset1 ^ 0x0a ^ ((color1 >> 5) & 1);
		color_table[0x12] = System16RoadColorOffset1 ^ 0x0c ^ ((color1 >> 6) & 1);
		bgcolor = (color1 >> 8) & 0xf;
		color_table[0x13] = (data1 & 0x200) ? color_table[0x10] : (System16RoadColorOffset2 ^ 0x10 ^ bgcolor);
		color_table[0x17] = System16RoadColorOffset1 ^ 0x0e ^ ((color1 >> 7) & 1);

		/* draw the road */
		switch (control) {
			case 0: {
				if (data0 & 0x800) continue;
				hpos0 = (hpos0 - (0x5f8 + System16RoadXOffset)) & 0xfff;
				for (x = 0; x < 320; x++) {
					INT32 pix0 = (hpos0 < 0x200) ? src0[hpos0] : 3;
					pPixel[x] = color_table[0x00 + pix0];
					hpos0 = (hpos0 + 1) & 0xfff;
				}
				break;
			}

			case 1: {
				hpos0 = (hpos0 - (0x5f8 + System16RoadXOffset)) & 0xfff;
				hpos1 = (hpos1 - (0x5f8 + System16RoadXOffset)) & 0xfff;
				for (x = 0; x < 320; x++) {
					INT32 pix0 = (hpos0 < 0x200) ? src0[hpos0] : 3;
					INT32 pix1 = (hpos1 < 0x200) ? src1[hpos1] : 3;
					if ((priority_map[0][pix0] >> pix1) & 1) {
						pPixel[x] = color_table[0x10 + pix1];
					} else {
						pPixel[x] = color_table[0x00 + pix0];
					}
					hpos0 = (hpos0 + 1) & 0xfff;
					hpos1 = (hpos1 + 1) & 0xfff;
				}
				break;
			}

			case 2: {
				hpos0 = (hpos0 - (0x5f8 + System16RoadXOffset)) & 0xfff;
				hpos1 = (hpos1 - (0x5f8 + System16RoadXOffset)) & 0xfff;
				for (x = 0; x < 320; x++) {
					INT32 pix0 = (hpos0 < 0x200) ? src0[hpos0] : 3;
					INT32 pix1 = (hpos1 < 0x200) ? src1[hpos1] : 3;
					if ((priority_map[1][pix0] >> pix1) & 1) {
						pPixel[x] = color_table[0x10 + pix1];
					} else {
						pPixel[x] = color_table[0x00 + pix0];
					}
					hpos0 = (hpos0 + 1) & 0xfff;
					hpos1 = (hpos1 + 1) & 0xfff;
				}
				break;
			}

			case 3: {
				if (data1 & 0x800) continue;
				hpos1 = (hpos1 - (0x5f8 + System16RoadXOffset)) & 0xfff;
				for (x = 0; x < 320; x++) {
					INT32 pix1 = (hpos1 < 0x200) ? src1[hpos1] : 3;
					pPixel[x] = color_table[0x10 + pix1];
					hpos1 = (hpos1 + 1) & 0xfff;
				}
				break;
			}
		}
	}	
}

static void HangonRenderRoadLayer(INT32 priority)
{
	UINT16 *roadram = (UINT16*)System16RoadRam;
	INT32 x, y;
	
	for (y = 0; y <= 223; y++)
	{
		UINT16* pPixel = pTransDraw + (y * 320);
		INT32 control = BURN_ENDIAN_SWAP_INT16(roadram[0x000 + y]);
		INT32 ff9j1 = 0, ff9j2 = 0, ctr9m = 0, ctr9n9p = 0, ctr9n9p_ena = 0, ss8j = 0, plycont = 0;
		
		/* the PLYCONT signal controls the road layering */
		plycont = (control >> 10) & 3;
		
		if (plycont != priority) continue;
		
		INT32 hpos = BURN_ENDIAN_SWAP_INT16(roadram[0x100 + (control & 0xff)]);
		INT32 color0 = BURN_ENDIAN_SWAP_INT16(roadram[0x200 + (control & 0xff)]);
		INT32 color1 = BURN_ENDIAN_SWAP_INT16(roadram[0x300 + (control & 0xff)]);
		UINT8 *src;

		/* compute the offset of the road graphics for this line */
		src = System16Roads + (0x000 + (control & 0xff)) * 512;

		/* initialize the 4-bit counter at 9M, which counts bits within each road byte */
		ctr9m = hpos & 7;

		/* initialize the two 4-bit counters at 9P (low) and 9N (high), which count road data bytes */
		ctr9n9p = (hpos >> 3) & 0xff;

		/* initialize the flip-flop at 9J (lower half), which controls the counting direction */
		ff9j1 = (hpos >> 11) & 1;

		/* initialize the flip-flop at 9J (upper half), which controls the background color */
		ff9j2 = 1;

		/* initialize the serial shifter at 8S, which delays several signals after we flip */
		ss8j = 0;

		/* draw this scanline from the beginning */
		for (x = -24; x <= 319; x++)
		{
			INT32 md, color, select;

			/* ---- the following logic all happens constantly ---- */

			/* the enable is controlled by the value in the counter at 9M */
			ctr9n9p_ena = (ctr9m == 7);

			/* if we carried out of the 9P/9N counters, we will forcibly clear the flip-flop at 9J (lower half) */
			if ((ctr9n9p & 0xff) == 0xff)
				ff9j1 = 0;

			/* if the control word bit 8 is clear, we will forcibly set the flip-flop at 9J (lower half) */
			if (!(control & 0x100))
				ff9j1 = 1;

			/* for the Hang On/Super Hang On case only: if the control word bit 9 is clear, we will forcibly */
			/* set the flip-flip at 9J (upper half) */
			if (Hangon && !(control & 0x200))
				ff9j2 = 1;

			/* ---- now process the pixel ---- */
			md = 3;

			/* the Space Harrier/Enduro Racer hardware has a tweak that maps the control word bit 9 to the */
			/* /CE line on the road ROM; use this to effectively disable the road data */
			if (Hangon || !(control & 0x200))

				/* the /OE line on the road ROM is linked to the AND of bits 2 & 3 of the counter at 9N */
				if ((ctr9n9p & 0xc0) == 0xc0)
				{
					/* note that the pixel logic is hidden in a custom at 9S; this is just a guess */
					if (ss8j & 1)
						md = src[((ctr9n9p & 0x3f) << 3) | ctr9m];
					else
						md = src[((ctr9n9p & 0x3f) << 3) | (ctr9m ^ 7)];
				}

			/* "select" is a made-up signal that comes from bit 3 of the serial shifter and is */
			/* used in several places for color selection */
			select = (ss8j >> 3) & 1;

			/* check the flip-flop at 9J (upper half) to determine if we should use the background color; */
			/* the output of this is ANDed with M0 and M1 so it only affects pixels with a value of 3; */
			/* this is done by the AND gates at 9L and 7K */
			if (ff9j2 && md == 3)
			{
				/* in this case, the "select" signal is used to select which background color to use */
				/* since the color0 control word contains two selections */
				color = (color0 >> (select ? 0 : 8)) & 0x3f;
				color |= 0x7c0;
			}

			/* if we're not using the background color, we select pixel data from an alternate path */
			else
			{
				/* the AND gates at 7L, 9K, and 7K clamp the pixel value to 0 if bit 7 of the color 1 */
				/* signal is 1 and if the pixel value is 3 (both M0 and M1 == 1) */
				if ((color1 & 0x80) && md == 3)
					md = 0;

				/* the pixel value plus the "select" line combine to form a mux into the low 8 bits of color1 */
				color = (color1 >> ((md << 1) | select)) & 1;

				/* this value becomes the low bit of the final color; the "select" line itself and the pixel */
				/* value form the other bits */
				color |= select << 3;
				color |= md << 1;
				color |= 0x038;
			}

			/* write the pixel if we're past the minimum clip */
			if (x >= 0)
				pPixel[x] = color;

			/* ---- the following logic all happens on the 6M clock ---- */

			/* clock the counter at 9M */
			ctr9m = (ctr9m + 1) & 7;

			/* if enabled, clock on the two cascaded 4-bit counters at 9P and 9N */
			if (ctr9n9p_ena)
			{
				if (ff9j1)
					ctr9n9p++;
				else
					ctr9n9p--;
			}

			/* clock the flip-flop at 9J (upper half) */
			ff9j2 = !(!ff9j1 && (ss8j & 0x80));

			/* clock the serial shift register at 8J */
			ss8j = (ss8j << 1) | ff9j1;
		}
	}
}

/*====================================================
Rotation Layer Rendering
====================================================*/

void System16RotateDraw()
{
	UINT16 *pRotateBuff = (UINT16*)System16RotateRamBuff;

	INT32 currx = (BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3f0]) << 16) | BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3f1]);
	INT32 curry = (BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3f2]) << 16) | BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3f3]);
	INT32 dyy = (BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3f4]) << 16) | BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3f5]);
	INT32 dxx = (BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3f6]) << 16) | BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3f7]);
	INT32 dxy = (BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3f8]) << 16) | BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3f9]);
	INT32 dyx = (BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3fa]) << 16) | BURN_ENDIAN_SWAP_INT16(pRotateBuff[0x3fb]);
	INT32 x, y;

	/* advance forward based on the clip rect */
	currx += dxx * (0 + 27) + dxy * 0;
	curry += dyx * (0 + 27) + dyy * 0;

	/* loop over screen Y coordinates */
	for (y = 0; y <= 223; y++)
	{
		UINT16* pPixel = pTransDraw + (y * 320);
		UINT16* pSrc = pTempDraw;
		INT32 tx = currx;
		INT32 ty = curry;

		/* loop over screen X coordinates */
		for (x = 0; x <= 319; x++)
		{
			/* fetch the pixel from the source bitmap */
			INT32 sx = (tx >> 14) & 0x1ff;
			INT32 sy = (ty >> 14) & 0x1ff;
			INT32 pix = pSrc[sy * 512 + (sx & 0x1ff)];

			/* non-zero pixels get written; everything else is the scanline color */
			if (pix != 0xffff)
			{
				*pPixel++ = ((pix & 0x1ff) | ((pix >> 6) & 0x200) | ((pix >> 3) & 0xc00) | 0x1000) & (System16PaletteEntries - 1);
			}
			else
			{
				*pPixel++ = /*info->colorbase +*/ sy;
			}

			/* advance the source X/Y pointers */
			tx += dxx;
			ty += dyx;
		}

		/* advance the source X/Y pointers */
		currx += dxy;
		curry += dyy;
	}
}

/*====================================================
Genesis VDP Rendering
====================================================*/

static void System18DrawVDP()
{
	INT32 x, y;

	for (y = 0; y < 224; y++)
	{
		UINT16 *src = pTempDraw + (y * 320);
		UINT16 *dst = pTransDraw + (y * 320);

		for (x = 0; x < 320; x++)
		{
			UINT16 pix = src[x];
			if (pix != 0xffff)
			{
				dst[x] = pix;
			}
		}
	}
}

void UpdateSystem18VDP()
{
	for (INT32 y = 0; y < 224; y++) {
		UINT16* pPixel = pTempDraw + (y * 320);
		vdp_drawline(pPixel, y, 0xffff);
	}
}

/*====================================================
Palette Generation
====================================================*/

static INT32 System16CalcPalette()
{
	INT32 i;

	for (i = 0; i < System16PaletteEntries * 2; i +=2) {
		INT32 r, g, b;
		INT32 nColour = (System16PaletteRam[i + 1] << 8) | System16PaletteRam[i + 0];
	
		r = (nColour & 0x00f) << 1;
		g = (nColour & 0x0f0) >> 2;
		b = (nColour & 0xf00) >> 7;

		if (nColour & 0x1000) r |= 1;
		if (nColour & 0x2000) g |= 2;
		if (nColour & 0x8000) g |= 1;
		if (nColour & 0x4000) b |= 1;
	
		r = (r << 3) | (r >> 2);
		g = (g << 2) | (g >> 4);
		b = (b << 3) | (b >> 2);
		
		System16Palette[i / 2] = BurnHighCol(r, g, b, 0);
		
		r = r * 160 / 256;
		g = g * 160 / 256;
		b = b * 160 / 256;
	
		System16Palette[(i / 2) + System16PaletteEntries] = BurnHighCol(r, g, b, 0);
		System16Palette[(i / 2) + (System16PaletteEntries * 2)] = BurnHighCol(r, g, b, 0);
	}
	
	return 0;
}

/*====================================================
Frame Rendering
====================================================*/

inline static void System16AUpdateTileValues()
{
	UINT16 *TextRam = (UINT16*)System16TextRam;
	INT32 i;
	
	for (i = 0; i < 2; i++) {
		System16OldPage[i] = System16Page[i];
		System16Page[i] = (System16ScreenFlip) ? BURN_ENDIAN_SWAP_INT16(TextRam[0xe8e/2 - i]) : BURN_ENDIAN_SWAP_INT16(TextRam[0xe9e/2 - i]);
		System16ScrollX[i] = BURN_ENDIAN_SWAP_INT16(TextRam[0xff8/2 + i]) & 0x1ff;
		System16ScrollY[i] = BURN_ENDIAN_SWAP_INT16(TextRam[0xf24/2 + i]) & 0x0ff;
	}
	
	if (System16OldPage[0] != System16Page[0]) {
		System16RecalcFgTileMap = 1;
	}
	
	if (System16OldPage[1] != System16Page[1]) {
		System16RecalcBgTileMap = 1;
	}
}

void System16ARender()
{
	if (!System16VideoEnable) {
		BurnTransferClear();
		return;
	}
	
	System16AUpdateTileValues();
	System16ACreateTileMaps();
	
	System16CalcPalette();
	System16ARenderTileLayer(1, 0, 0);
	System16ARenderSpriteLayer(1);
	System16ARenderTileLayer(1, 0, 1);
	System16ARenderSpriteLayer(2);
	System16ARenderTileLayer(1, 1, 1);
	System16ARenderTileLayer(0, 0, 1);
	System16ARenderSpriteLayer(4);
	System16ARenderTileLayer(0, 1, 1);
	System16ARenderTextLayer(0);
	System16ARenderSpriteLayer(8);
	System16ARenderTextLayer(1);
	BurnTransferCopy(System16Palette);
}

void System16BootlegRender()
{
	if (!System16VideoEnable) {
		BurnTransferClear();
		return;
	}

	System16CalcPalette();
	BootlegRenderTileLayer(1, 0, 0);
	BootlegRenderTileLayer(1, 1, 0);
	System16BRenderSpriteLayer(1);
	BootlegRenderTileLayer(1, 0, 1);
	System16BRenderSpriteLayer(2);
	BootlegRenderTileLayer(1, 1, 1);
	BootlegRenderTileLayer(0, 0, 1);
	System16BRenderSpriteLayer(4);
	BootlegRenderTileLayer(0, 1, 1);
	System16BRenderTextLayer(0);
	System16BRenderSpriteLayer(8);
	System16BRenderTextLayer(1);
	BurnTransferCopy(System16Palette);
}

inline static void System16BUpdateTileValues()
{
	UINT16 *TextRam = (UINT16*)System16TextRam;
	
	for (INT32 i = 0; i < 4; i++) {
		System16OldPage[i] = System16Page[i];
		System16Page[i] = BURN_ENDIAN_SWAP_INT16(TextRam[(0xe80 >> 1) + i]);
		System16ScrollX[i] = BURN_ENDIAN_SWAP_INT16(TextRam[(0xe98 >> 1) + i]);
		System16ScrollY[i] = BURN_ENDIAN_SWAP_INT16(TextRam[(0xe90 >> 1) + i]);
	}
	
	if (System16OldPage[0] != System16Page[0]) {
		System16RecalcFgTileMap = 1;
	}
	
	if (System16OldPage[1] != System16Page[1]) {
		System16RecalcBgTileMap = 1;
	}
	
	if (System16OldPage[2] != System16Page[2]) {
		System16RecalcFgAltTileMap = 1;
	}
	
	if (System16OldPage[3] != System16Page[3]) {
		System16RecalcBgAltTileMap = 1;
	}
}

void System16BRender()
{
	if (!System16IgnoreVideoEnable) {
		if (!System16VideoEnable) {
			BurnTransferClear();
			return;
		}
	}
	
	System16BUpdateTileValues();
	System16BCreateTileMaps();
	
	System16CalcPalette();	
	System16BRenderTileLayer(1, 0, 0);
	System16BRenderSpriteLayer(1);
	System16BRenderTileLayer(1, 0, 1);
	System16BRenderSpriteLayer(2);
	System16BRenderTileLayer(1, 1, 1);
	System16BRenderTileLayer(0, 0, 1);
	System16BRenderSpriteLayer(4);
	System16BRenderTileLayer(0, 1, 1);
	System16BRenderTextLayer(0);
	System16BRenderSpriteLayer(8);
	System16BRenderTextLayer(1);
	BurnTransferCopy(System16Palette);
}

void System16BAltRender()
{
	if (!System16VideoEnable) {
		BurnTransferClear();
		return;
	}

	System16BUpdateTileValues();
	System16BAltCreateTileMaps();
	
	System16CalcPalette();
	System16BRenderTileLayer(1, 0, 0);
	System16BRenderSpriteLayer(1);
	System16BRenderTileLayer(1, 0, 1);
	System16BRenderSpriteLayer(2);
	System16BRenderTileLayer(1, 1, 1);
	System16BRenderTileLayer(0, 0, 1);
	System16BRenderSpriteLayer(4);
	System16BRenderTileLayer(0, 1, 1);
	System16BAltRenderTextLayer(0);
	System16BRenderSpriteLayer(8);
	System16BAltRenderTextLayer(1);
	BurnTransferCopy(System16Palette);
}

void System18Render()
{
	if (!System16VideoEnable) {
		BurnTransferClear();
		return;
	}
	
	INT32 VDPLayer = (System18VdpMixing >> 1) & 3;
	INT32 VDPPri = (System18VdpMixing & 1) ? (1 << VDPLayer) : 0;
	
	System16BUpdateTileValues();
	System16BCreateTileMaps();
	
	System16CalcPalette();
	
	if (System18VdpEnable) UpdateSystem18VDP();
	
	System16BRenderTileLayer(1, 0, 0);
	if (System18VdpEnable && VDPLayer == 0 && !VDPPri) System18DrawVDP();
	System16BRenderSpriteLayer(1);
	if (System18VdpEnable && VDPLayer == 0 && VDPPri) System18DrawVDP();
	System16BRenderTileLayer(1, 0, 1);
	if (System18VdpEnable && VDPLayer == 1 && !VDPPri) System18DrawVDP();
	System16BRenderSpriteLayer(2);
	if (System18VdpEnable && VDPLayer == 1 && VDPPri) System18DrawVDP();
	System16BRenderTileLayer(1, 1, 1);
	System16BRenderTileLayer(0, 0, 1);
	if (System18VdpEnable && VDPLayer == 2 && !VDPPri) System18DrawVDP();
	System16BRenderSpriteLayer(4);
	if (System18VdpEnable && VDPLayer == 2 && VDPPri) System18DrawVDP();
	System16BRenderTileLayer(0, 1, 1);
	System16BRenderTextLayer(0);
	if (System18VdpEnable && VDPLayer == 3 && !VDPPri) System18DrawVDP();
	System16BRenderSpriteLayer(8);
	if (System18VdpEnable && VDPLayer == 3 && VDPPri) System18DrawVDP();
	System16BRenderTextLayer(1);
	BurnTransferCopy(System16Palette);

	for (INT32 i = 0; i < nBurnGunNumPlayers; i++) {
		BurnGunDrawTarget(i, BurnGunX[i] >> 8, BurnGunY[i] >> 8);
	}
}

void HangonRender()
{
	if (!System16VideoEnable) {
		BurnTransferClear();
		return;
	}
	
	System16CalcPalette();
	
	System16AUpdateTileValues();
	System16ACreateTileMaps();
	
	HangonRenderRoadLayer(0);
	HangonRenderSpriteLayer(1);
	System16ARenderTileLayer(1, 0, 1);
	HangonRenderSpriteLayer(2);
	System16ARenderTileLayer(1, 1, 1);
	System16ARenderTileLayer(0, 0, 1);
	HangonRenderSpriteLayer(4);
	System16ARenderTileLayer(0, 1, 1);
	HangonRenderRoadLayer(1);
	HangonRenderSpriteLayer(8);
	System16ARenderTextLayer(0);
	System16ARenderTextLayer(1);
	BurnTransferCopy(System16Palette);
}

void HangonAltRender()
{
	if (!System16VideoEnable) {
		BurnTransferClear();
		return;
	}
	
	System16CalcPalette();
	
	System16AUpdateTileValues();
	System16ACreateTileMaps();
	
	HangonRenderRoadLayer(0);
	HangonAltRenderSpriteLayer(1);
	System16ARenderTileLayer(1, 0, 1);
	HangonAltRenderSpriteLayer(2);
	System16ARenderTileLayer(1, 1, 1);
	System16ARenderTileLayer(0, 0, 1);
	HangonAltRenderSpriteLayer(4);
	System16ARenderTileLayer(0, 1, 1);
	HangonRenderRoadLayer(1);
	HangonAltRenderSpriteLayer(8);
	System16ARenderTextLayer(0);
	System16ARenderTextLayer(1);
	BurnTransferCopy(System16Palette);
}

void OutrunRender()
{
	if (!System16VideoEnable) {
		BurnTransferClear();
		return;
	}

	System16BUpdateTileValues();
	System16BCreateTileMaps();
	
	System16CalcPalette();
	OutrunRenderRoadBackgroundLayer();
	OutrunRenderSpriteLayer(1);
	System16BRenderTileLayer(1, 0, 1);
	OutrunRenderSpriteLayer(2);
	System16BRenderTileLayer(1, 1, 1);
	System16BRenderTileLayer(0, 0, 1);
	OutrunRenderSpriteLayer(4);
	System16BRenderTileLayer(0, 1, 1);
	OutrunRenderRoadForegroundLayer();
	System16BRenderTextLayer(0);
	OutrunRenderSpriteLayer(8);
	System16BRenderTextLayer(1);
	BurnTransferCopy(System16Palette);
}

void ShangonRender()
{
	if (!System16VideoEnable) {
		BurnTransferClear();
		return;
	}

	System16BUpdateTileValues();
	System16BAltCreateTileMaps();
	
	System16CalcPalette();
	OutrunRenderRoadBackgroundLayer();
	System16BRenderSpriteLayer(1);
	System16BRenderTileLayer(1, 0, 1);
	System16BRenderSpriteLayer(2);
	System16BRenderTileLayer(1, 1, 1);
	System16BRenderTileLayer(0, 0, 1);
	System16BRenderSpriteLayer(4);
	System16BRenderTileLayer(0, 1, 1);
	OutrunRenderRoadForegroundLayer();
	System16BRenderSpriteLayer(8);
	System16BAltRenderTextLayer(0);
	System16BAltRenderTextLayer(1);
	BurnTransferCopy(System16Palette);
}

void XBoardRender()
{
	if (!System16VideoEnable) {
		BurnTransferClear();
		return;
	}

	System16BUpdateTileValues();
	System16BCreateTileMaps();
	
	System16CalcPalette();
	OutrunRenderRoadBackgroundLayer();
	if (!System16RoadPriority) OutrunRenderRoadForegroundLayer();
	XBoardRenderSpriteLayer(1);
	System16BRenderTileLayer(1, 0, 1);
	XBoardRenderSpriteLayer(2);
	System16BRenderTileLayer(1, 1, 1);
	System16BRenderTileLayer(0, 0, 1);
	XBoardRenderSpriteLayer(4);
	System16BRenderTileLayer(0, 1, 1);
	if (System16RoadPriority) OutrunRenderRoadForegroundLayer();
	System16BRenderTextLayer(0);
	XBoardRenderSpriteLayer(8);
	System16BRenderTextLayer(1);
	BurnTransferCopy(System16Palette);
	
	for (INT32 i = 0; i < nBurnGunNumPlayers; i++) {
		BurnGunDrawTarget(i, BurnGunX[i] >> 8, BurnGunY[i] >> 8);
	}
}

void YBoardRender()
{
	if (!System16VideoEnable) {
		BurnTransferClear();
		return;
	}
	
	System16CalcPalette();
	YBoardRenderSpriteLayer();
	System16RotateDraw();
	YBoardSystem16BRenderSpriteLayer();
	BurnTransferCopy(System16Palette);
	
	for (INT32 i = 0; i < nBurnGunNumPlayers; i++) {
		BurnGunDrawTarget(i, BurnGunX[i] >> 8, BurnGunY[i] >> 8);
	}
}
