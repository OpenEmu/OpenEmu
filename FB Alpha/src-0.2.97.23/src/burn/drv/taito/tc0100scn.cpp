// TC0100SCN

#include "tiles_generic.h"
#include "taito_ic.h"

UINT8 *TC0100SCNRam[TC0100SCN_MAX_CHIPS];
UINT16 TC0100SCNCtrl[TC0100SCN_MAX_CHIPS][8];
UINT8 TC0100SCNBgLayerUpdate[TC0100SCN_MAX_CHIPS];
UINT8 TC0100SCNFgLayerUpdate[TC0100SCN_MAX_CHIPS];
static UINT8 *TC0100SCNChars[TC0100SCN_MAX_CHIPS];
static INT32 BgScrollX[TC0100SCN_MAX_CHIPS];
static INT32 BgScrollY[TC0100SCN_MAX_CHIPS];
static INT32 FgScrollX[TC0100SCN_MAX_CHIPS];
static INT32 FgScrollY[TC0100SCN_MAX_CHIPS];
static INT32 CharScrollX[TC0100SCN_MAX_CHIPS];
static INT32 CharScrollY[TC0100SCN_MAX_CHIPS];
static INT32 TC0100SCNXOffset[TC0100SCN_MAX_CHIPS];
static INT32 TC0100SCNYOffset[TC0100SCN_MAX_CHIPS];
static INT32 TC0100SCNFlipScreenX[TC0100SCN_MAX_CHIPS];
static INT32 TC0100SCNFlip[TC0100SCN_MAX_CHIPS];
static UINT8 *TC0100SCNPriorityMap[TC0100SCN_MAX_CHIPS] = { NULL, };
static INT32 TC0100SCNColourDepth[TC0100SCN_MAX_CHIPS];
static INT32 TC0100SCNGfxBank[TC0100SCN_MAX_CHIPS];
static INT32 TC0100SCNGfxMask[TC0100SCN_MAX_CHIPS];
static INT32 TC0100SCNDblWidth[TC0100SCN_MAX_CHIPS];
static INT32 TC0100SCNClipWidth[TC0100SCN_MAX_CHIPS];
static INT32 TC0100SCNClipHeight[TC0100SCN_MAX_CHIPS];
static INT32 TC0100SCNClipStartX[TC0100SCN_MAX_CHIPS];
static INT32 TC0100SCNPaletteOffset[TC0100SCN_MAX_CHIPS];
static UINT16 *pTC0100SCNBgTempDraw[TC0100SCN_MAX_CHIPS] = { NULL, };
static UINT16 *pTC0100SCNFgTempDraw[TC0100SCN_MAX_CHIPS] = { NULL, };
static INT32 TC0100SCNNum = 0;

#define PLOTPIXEL(x, po) pPixel[x] = nPalette | pTileData[x] | po;
#define PLOTPIXEL_FLIPX(x, a, po) pPixel[x] = nPalette | pTileData[a] | po;

static void RenderTile(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, INT32 nTilemapWidth, UINT8 *pTile)
{
	UINT32 nPalette = nTilePalette << nColourDepth;
	pTileData = pTile + (nTileNumber << 6);

	UINT16* pPixel = pDestDraw + (StartY * nTilemapWidth) + StartX;

	for (INT32 y = 0; y < 8; y++, pPixel += nTilemapWidth, pTileData += 8) {
		PLOTPIXEL( 0, nPaletteOffset);
		PLOTPIXEL( 1, nPaletteOffset);
		PLOTPIXEL( 2, nPaletteOffset);
		PLOTPIXEL( 3, nPaletteOffset);
		PLOTPIXEL( 4, nPaletteOffset);
		PLOTPIXEL( 5, nPaletteOffset);
		PLOTPIXEL( 6, nPaletteOffset);
		PLOTPIXEL( 7, nPaletteOffset);
	}
}

static void RenderTile_FlipX(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, INT32 nTilemapWidth, UINT8 *pTile)
{
	UINT32 nPalette = nTilePalette << nColourDepth;
	pTileData = pTile + (nTileNumber << 6);

	UINT16* pPixel = pDestDraw + (StartY * nTilemapWidth) + StartX;

	for (INT32 y = 0; y < 8; y++, pPixel += nTilemapWidth, pTileData += 8) {
		PLOTPIXEL_FLIPX( 7, 0, nPaletteOffset);
		PLOTPIXEL_FLIPX( 6, 1, nPaletteOffset);
		PLOTPIXEL_FLIPX( 5, 2, nPaletteOffset);
		PLOTPIXEL_FLIPX( 4, 3, nPaletteOffset);
		PLOTPIXEL_FLIPX( 3, 4, nPaletteOffset);
		PLOTPIXEL_FLIPX( 2, 5, nPaletteOffset);
		PLOTPIXEL_FLIPX( 1, 6, nPaletteOffset);
		PLOTPIXEL_FLIPX( 0, 7, nPaletteOffset);
	}
}

static void RenderTile_FlipY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, INT32 nTilemapWidth, UINT8 *pTile)
{
	UINT32 nPalette = nTilePalette << nColourDepth;
	pTileData = pTile + (nTileNumber << 6);

	UINT16* pPixel = pDestDraw + ((StartY + 7) * nTilemapWidth) + StartX;

	for (INT32 y = 7; y >= 0; y--, pPixel -= nTilemapWidth, pTileData += 8) {
		PLOTPIXEL( 0, nPaletteOffset);
		PLOTPIXEL( 1, nPaletteOffset);
		PLOTPIXEL( 2, nPaletteOffset);
		PLOTPIXEL( 3, nPaletteOffset);
		PLOTPIXEL( 4, nPaletteOffset);
		PLOTPIXEL( 5, nPaletteOffset);
		PLOTPIXEL( 6, nPaletteOffset);
		PLOTPIXEL( 7, nPaletteOffset);
	}
}

static void RenderTile_FlipXY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nPaletteOffset, INT32 nTilemapWidth, UINT8 *pTile)
{
	UINT32 nPalette = nTilePalette << nColourDepth;
	pTileData = pTile + (nTileNumber << 6);

	UINT16* pPixel = pDestDraw + ((StartY + 7) * nTilemapWidth) + StartX;

	for (INT32 y = 7; y >= 0; y--, pPixel -= nTilemapWidth, pTileData += 8) {
		PLOTPIXEL_FLIPX( 7, 0, nPaletteOffset);
		PLOTPIXEL_FLIPX( 6, 1, nPaletteOffset);
		PLOTPIXEL_FLIPX( 5, 2, nPaletteOffset);
		PLOTPIXEL_FLIPX( 4, 3, nPaletteOffset);
		PLOTPIXEL_FLIPX( 3, 4, nPaletteOffset);
		PLOTPIXEL_FLIPX( 2, 5, nPaletteOffset);
		PLOTPIXEL_FLIPX( 1, 6, nPaletteOffset);
		PLOTPIXEL_FLIPX( 0, 7, nPaletteOffset);
	}
}

#undef PLOTPIXEL
#undef PLOTPIXEL_FLIPX

void TC0100SCNCtrlWordWrite(INT32 Chip, UINT32 Offset, UINT16 Data)
{
	TC0100SCNCtrl[Chip][Offset] = Data;
	
	switch (Offset) {
		case 0x00: {
			BgScrollX[Chip] = -Data;
			return;
		}
		
		case 0x01: {
			FgScrollX[Chip] = -Data;
			return;
		}
		
		case 0x02: {
			CharScrollX[Chip] = -Data;
			return;
		}
		
		case 0x03: {
			BgScrollY[Chip] = -Data;
			return;
		}
		
		case 0x04: {
			FgScrollY[Chip] = -Data;
			return;
		}
		
		case 0x05: {
			CharScrollY[Chip] = -Data;
			return;
		}
		
		case 0x06: {
			TC0100SCNDblWidth[Chip] = (Data & 0x10) >> 4;
			if (TC0100SCNFlip[Chip]) bprintf(PRINT_NORMAL, _T("Double\n"));
			return;
		}
		
		case 0x07: {
			TC0100SCNFlip[Chip] = (Data & 0x01) ? 1: 0;
			if (TC0100SCNFlip[Chip]) bprintf(PRINT_NORMAL, _T("Flipped\n"));
			return;
		}
	}
	
	bprintf(PRINT_IMPORTANT, _T("TC0100 Ctrl Word Write %02X, %04X\n"), Offset, Data);
}

INT32 TC0100SCNBottomLayer(INT32 Chip)
{
	return (TC0100SCNCtrl[Chip][6] & 0x08) >> 3;
}

void TC0100SCNRenderBgLayer(INT32 Chip, INT32 Opaque, UINT8 *pSrc)
{
	INT32 mx, my, Attr, Code, Colour, x, y, xSrc = 0, ySrc = 0, TileIndex = 0, Offset, Flip, xFlip, yFlip, p, dxScroll, dyScroll;
	
	UINT16 *VideoRam = (UINT16*)TC0100SCNRam[Chip];
	UINT16 *ScrollRam = (UINT16*)TC0100SCNRam[Chip] + (0xc000 / 2);
	INT32 Columns = 64;
	INT32 WidthMask = 0x1ff;
	INT32 HeightMask = 0x1ff;
	
	if (TC0100SCNDblWidth[Chip]) {
		VideoRam = (UINT16*)TC0100SCNRam[Chip];
		ScrollRam = (UINT16*)TC0100SCNRam[Chip] + (0x10000 / 2);
		Columns = 128;
		WidthMask = 0x3ff;
	}

	// Render the tilemap to a buffer
	if (TC0100SCNBgLayerUpdate[Chip]) {
		memset(pTC0100SCNBgTempDraw[Chip], 0, Columns * 8 * 512 * sizeof(UINT16));
		for (my = 0; my < 64; my++) {
			for (mx = 0; mx < Columns; mx++) {
				Offset = 2 * TileIndex;
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset]);
				Code = (BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 1]) & TC0100SCNGfxMask[Chip]) + (TC0100SCNGfxBank[Chip] << 15);
				Colour = Attr & 0xff;
				Flip = (Attr & 0xc000) >> 14;
				xFlip = (Flip >> 0) & 0x01;
				yFlip = (Flip >> 1) & 0x01;
			
				x = 8 * mx;
				y = 8 * my;
			
				if (TC0100SCNFlipScreenX[Chip]) {
					xFlip = !xFlip;
					x = (Columns * 8) - 8 - x;
				}
			
				if (TC0100SCNFlip[Chip]) {
					xFlip = !xFlip;
					x = (Columns * 8) - 8 - x;
					yFlip = !yFlip;
					y = 512 - 8 - y;
				}
		
				if (xFlip) {
					if (yFlip) {
						RenderTile_FlipXY(pTC0100SCNBgTempDraw[Chip], Code, x, y, Colour, TC0100SCNColourDepth[Chip], TC0100SCNPaletteOffset[Chip], Columns * 8, pSrc);
					} else {
						RenderTile_FlipX(pTC0100SCNBgTempDraw[Chip], Code, x, y, Colour, TC0100SCNColourDepth[Chip], TC0100SCNPaletteOffset[Chip], Columns * 8, pSrc);
					}
				} else {
					if (yFlip) {
						RenderTile_FlipY(pTC0100SCNBgTempDraw[Chip], Code, x, y, Colour, TC0100SCNColourDepth[Chip], TC0100SCNPaletteOffset[Chip], Columns * 8, pSrc);
					} else {
						RenderTile(pTC0100SCNBgTempDraw[Chip], Code, x, y, Colour, TC0100SCNColourDepth[Chip], TC0100SCNPaletteOffset[Chip], Columns * 8, pSrc);
					}
				}

				TileIndex++;
			}
		}
		
		TC0100SCNBgLayerUpdate[Chip] = 0;
	}
	
	dxScroll = TC0100SCNXOffset[Chip] + 16 - TC0100SCNClipStartX[Chip];
	dyScroll = TC0100SCNYOffset[Chip];

	ySrc = (BgScrollY[Chip] + dyScroll) & HeightMask;
	if (TC0100SCNFlip[Chip]) ySrc = (256 + 16 - ySrc) & HeightMask;
	
	for (y = 0; y < TC0100SCNClipHeight[Chip]; y++) {
		xSrc = (BgScrollX[Chip] - BURN_ENDIAN_SWAP_INT16(ScrollRam[(y + dyScroll) & 0x1ff]) + dxScroll + TC0100SCNClipStartX[Chip]) & WidthMask;
		if(TC0100SCNFlip[Chip]) xSrc = (256 - 58 - xSrc) & WidthMask;
		if (TC0100SCNFlipScreenX[Chip]) xSrc = (256 - 64 - xSrc) & WidthMask;
		
		for (x = TC0100SCNClipStartX[Chip]; x < TC0100SCNClipStartX[Chip] + TC0100SCNClipWidth[Chip]; x++) {
			p = pTC0100SCNBgTempDraw[Chip][((ySrc & HeightMask) * Columns * 8) + xSrc];

			if ((p & 0x0f) != 0 || Opaque) {
				pTransDraw[(y * nScreenWidth) + x] = p;
//				if (TC0100SCNPriorityMap[Chip]) TC0100SCNPriorityMap[Chip][(y * TC0100SCNClipWidth[Chip]) + (x - TC0100SCNClipStartX[Chip])] = 1;
				if (TC0100SCNPriorityMap[Chip]) TC0100SCNPriorityMap[Chip][(y * nScreenWidth) + x] = 1;
			}
			xSrc = (xSrc + 1);
			xSrc &= WidthMask;
		}
		ySrc = (ySrc + 1);
		ySrc &= HeightMask;
	}
}

void TC0100SCNRenderFgLayer(INT32 Chip, INT32 Opaque, UINT8 *pSrc)
{
	INT32 mx, my, Attr, Code, Colour, x, y, xSrc = 0, ySrc = 0, TileIndex = 0, Offset, Flip, xFlip, yFlip, p, ColumnOffset, dxScroll, dyScroll;
	
	UINT16 *VideoRam = (UINT16*)TC0100SCNRam[Chip] + (0x8000 / 2);
	UINT16 *ScrollRam = (UINT16*)TC0100SCNRam[Chip] + (0xc400 / 2);
	UINT16 *ColumnRam = (UINT16*)TC0100SCNRam[Chip] + (0xe000 / 2);
	INT32 Columns = 64;
	INT32 WidthMask = 0x1ff;
	INT32 HeightMask = 0x1ff;
	
	if (TC0100SCNDblWidth[Chip]) {
		VideoRam = (UINT16*)TC0100SCNRam[Chip] + (0x8000 / 2);
		ScrollRam = (UINT16*)TC0100SCNRam[Chip] + (0x10400 / 2);
		ColumnRam = (UINT16*)TC0100SCNRam[Chip] + (0x10800 / 2);
		Columns = 128;
		WidthMask = 0x3ff;
	}

	// Render the tilemap to a buffer
	if (TC0100SCNFgLayerUpdate[Chip]) {
		memset(pTC0100SCNFgTempDraw[Chip], 0, Columns * 8 * 512 * sizeof(UINT16));
		for (my = 0; my < 64; my++) {
			for (mx = 0; mx < Columns; mx++) {
				Offset = 2 * TileIndex;
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset]);
				Code = (BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 1]) & TC0100SCNGfxMask[Chip]) + (TC0100SCNGfxBank[Chip] << 15);
				Colour = Attr & 0xff;
				Flip = (Attr & 0xc000) >> 14;
				xFlip = (Flip >> 0) & 0x01;
				yFlip = (Flip >> 1) & 0x01;
			
				x = 8 * mx;
				y = 8 * my;
			
				if (TC0100SCNFlipScreenX[Chip]) {
					xFlip = !xFlip;
					x = (Columns * 8) - 8 - x;
				}
			
				if (TC0100SCNFlip[Chip]) {
					xFlip = !xFlip;
					x = (Columns * 8) - 8 - x;
					yFlip = !yFlip;
					y = 512 - 8 -y;
				}
		
				if (xFlip) {
					if (yFlip) {
						RenderTile_FlipXY(pTC0100SCNFgTempDraw[Chip], Code, x, y, Colour, TC0100SCNColourDepth[Chip], TC0100SCNPaletteOffset[Chip], Columns * 8, pSrc);
					} else {
						RenderTile_FlipX(pTC0100SCNFgTempDraw[Chip], Code, x, y, Colour, TC0100SCNColourDepth[Chip], TC0100SCNPaletteOffset[Chip], Columns * 8, pSrc);
					}
				} else {
					if (yFlip) {
						RenderTile_FlipY(pTC0100SCNFgTempDraw[Chip], Code, x, y, Colour, TC0100SCNColourDepth[Chip], TC0100SCNPaletteOffset[Chip], Columns * 8, pSrc);
					} else {
						RenderTile(pTC0100SCNFgTempDraw[Chip], Code, x, y, Colour, TC0100SCNColourDepth[Chip], TC0100SCNPaletteOffset[Chip], Columns * 8, pSrc);
					}
				}
				
				TileIndex++;
			}
		}
		
		TC0100SCNFgLayerUpdate[Chip] = 0;
	}
	
	dxScroll = TC0100SCNXOffset[Chip] + 16 - TC0100SCNClipStartX[Chip];
	dyScroll = TC0100SCNYOffset[Chip];

	ySrc = (FgScrollY[Chip] + dyScroll) & HeightMask;
	if (TC0100SCNFlip[Chip]) ySrc = (256 + 16 - ySrc) & HeightMask;
	
	for (y = 0; y < TC0100SCNClipHeight[Chip]; y++) {
		xSrc = (FgScrollX[Chip] - BURN_ENDIAN_SWAP_INT16(ScrollRam[(y + dyScroll) & 0x1ff]) + dxScroll + TC0100SCNClipStartX[Chip]) & WidthMask;
		if (TC0100SCNFlip[Chip]) xSrc = (256 - 58 - xSrc) & WidthMask;
		if (TC0100SCNFlipScreenX[Chip]) xSrc = (256 - 64 - xSrc) & WidthMask;
		
		for (x = TC0100SCNClipStartX[Chip]; x < TC0100SCNClipStartX[Chip] + TC0100SCNClipWidth[Chip]; x++) {
			ColumnOffset = BURN_ENDIAN_SWAP_INT16(ColumnRam[(xSrc & 0x3ff) / 8]);
			p = pTC0100SCNFgTempDraw[Chip][(((ySrc - ColumnOffset) & HeightMask) * Columns * 8) + xSrc];

			if ((p & 0x0f) != 0 || Opaque) {
				pTransDraw[(y * nScreenWidth) + x] = p;
//				if (TC0100SCNPriorityMap[Chip]) TC0100SCNPriorityMap[Chip][(y * TC0100SCNClipWidth[Chip]) + (x - TC0100SCNClipStartX[Chip])] = 2;
				if (TC0100SCNPriorityMap[Chip]) TC0100SCNPriorityMap[Chip][(y * nScreenWidth) + x] = 2;
			}
			xSrc = (xSrc + 1);
			xSrc &= WidthMask;
		}
		ySrc = (ySrc + 1);
		ySrc &= HeightMask;
	}
}

static INT32 TC0100SCNPlaneOffsets[2] = { 8, 0 };
static INT32 TC0100SCNXOffsets[8]     = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 TC0100SCNYOffsets[8]     = { 0, 16, 32, 48, 64, 80, 96, 112 };

void TC0100SCNRenderCharLayer(INT32 Chip)
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)TC0100SCNRam[Chip] + (0x4000 / 2);
	UINT16 *CharRam = (UINT16*)TC0100SCNRam[Chip] + (0x6000 / 2);
	INT32 Columns = 64;
	INT32 Rows = 64;
	
	if (TC0100SCNDblWidth[Chip]) {
		VideoRam = (UINT16*)TC0100SCNRam[Chip] + (0x12000 / 2);
		CharRam = (UINT16*)TC0100SCNRam[Chip] + (0x11000 / 2);
		Columns = 128;
		Rows = 32;
	}
	
	GfxDecode(256, 2, 8, 8, TC0100SCNPlaneOffsets, TC0100SCNXOffsets, TC0100SCNYOffsets, 0x80, (UINT8*)CharRam, TC0100SCNChars[Chip]);
	
	for (my = 0; my < Rows; my++) {
		for (mx = 0; mx < Columns; mx++) {
			Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
			Code = Attr & 0xff;
			Colour = (Attr >> 6) & 0xfc;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= 16;
			if (!TC0100SCNFlip[Chip] && !TC0100SCNFlipScreenX[Chip]) x -= TC0100SCNXOffset[Chip];
			if (TC0100SCNFlip[Chip] || TC0100SCNFlipScreenX[Chip]) x += TC0100SCNXOffset[Chip];
			y -= TC0100SCNYOffset[Chip];
			
			if (TC0100SCNFlipScreenX[Chip]) {
				xFlip = !xFlip;
				x = TC0100SCNClipWidth[Chip] - x;
			}
			
			if (TC0100SCNFlip[Chip]) {
				xFlip = !xFlip;
				x = TC0100SCNClipWidth[Chip] - x;
				yFlip = !yFlip;
				y = TC0100SCNClipHeight[Chip] + 8 - y;
			}
			
			if (!TC0100SCNDblWidth[Chip]) {
				if (!TC0100SCNFlip[Chip] && !TC0100SCNFlipScreenX[Chip]) x -= CharScrollX[Chip] & 0x1ff;
				if (TC0100SCNFlip[Chip] || TC0100SCNFlipScreenX[Chip]) x += CharScrollX[Chip] & 0x1ff;
				
				if (x < -8) x += 512;
				if (x >= 512) x -= 512;
				
				if (!TC0100SCNFlip[Chip]) y -= CharScrollY[Chip] & 0x1ff;
				if (TC0100SCNFlip[Chip]) y += CharScrollY[Chip] & 0x1ff;
				
				if (y < -8) y += 512;
				if (y >= 512) y -= 512;
			} else {
				if (!TC0100SCNFlip[Chip] && !TC0100SCNFlipScreenX[Chip]) x -= CharScrollX[Chip] & 0x3ff;
				if (TC0100SCNFlip[Chip] || TC0100SCNFlipScreenX[Chip]) x += CharScrollX[Chip] & 0x3ff;
				
				if (x < -8) x += 1024;
				if (x >= 1024) x -= 1024;
				
				if (!TC0100SCNFlip[Chip]) y -= CharScrollY[Chip] & 0xff;
				if (TC0100SCNFlip[Chip]) y += CharScrollY[Chip] & 0xff;
				
				if (y < -8) y += 256;
				if (y >= 256) y -= 256;
			}
			
			INT32 px, py;
			
			UINT32 nPalette = Colour << 2;
			
			for (py = 0; py < 8; py++) {
				for (px = 0; px < 8; px++) {
					UINT8 c = TC0100SCNChars[Chip][(Code * 64) + (py * 8) + px];
					if (xFlip) c = TC0100SCNChars[Chip][(Code * 64) + (py * 8) + (7 - px)];
					if (yFlip) c = TC0100SCNChars[Chip][(Code * 64) + ((7 - py) * 8) + px];
					if (xFlip && yFlip) c = TC0100SCNChars[Chip][(Code * 64) + ((7 - py) * 8) + (7 - px)];
					
					if (c) {
						INT32 xPos = x + px + TC0100SCNClipStartX[Chip];
						INT32 yPos = y + py;

						if (yPos >= 0 && yPos < TC0100SCNClipHeight[Chip]) {
							UINT16* pPixel = pTransDraw + (yPos * nScreenWidth);
						
							if (xPos >= TC0100SCNClipStartX[Chip] && xPos < TC0100SCNClipStartX[Chip] + TC0100SCNClipWidth[Chip]) {
								pPixel[xPos] = c | nPalette | TC0100SCNPaletteOffset[Chip];
//								if (TC0100SCNPriorityMap[Chip]) TC0100SCNPriorityMap[Chip][(yPos * TC0100SCNClipWidth[Chip]) + xPos - TC0100SCNClipStartX[Chip]] = 4;
								if (TC0100SCNPriorityMap[Chip]) TC0100SCNPriorityMap[Chip][(yPos * nScreenWidth) + xPos] = 4;
							}
						}
					}
				}
			}			
			
			TileIndex++;
		}
	}
}

void TC0100SCNReset()
{
	for (INT32 i = 0;i < TC0100SCNNum; i++) {
		memset(TC0100SCNCtrl[i], 0, 8);	
		memset(TC0100SCNChars[i], 0, 256 * 8 * 8);
		
		BgScrollX[i] = 0;
		BgScrollY[i] = 0;
		FgScrollX[i] = 0;
		FgScrollY[i] = 0;
		CharScrollX[i] = 0;
		CharScrollY[i] = 0;
		TC0100SCNFlip[i] = 0;
		TC0100SCNGfxBank[i] = 0;
		TC0100SCNDblWidth[i] = 0;
		TC0100SCNBgLayerUpdate[i] = 1;
		TC0100SCNFgLayerUpdate[i] = 1;
	}
}

void TC0100SCNInit(INT32 Chip, INT32 nNumTiles, INT32 xOffset, INT32 yOffset, INT32 xFlip, UINT8 *PriorityMap)
{
	TC0100SCNRam[Chip] = (UINT8*)BurnMalloc(0x14000);
	memset(TC0100SCNRam[Chip], 0, 0x14000);
		
	TC0100SCNChars[Chip] = (UINT8*)BurnMalloc(256 * 8 * 8);
	memset(TC0100SCNChars[Chip], 0, 256 * 8 * 8);
	
	pTC0100SCNBgTempDraw[Chip] = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	memset(pTC0100SCNBgTempDraw[Chip], 0, 1024 * 512 * sizeof(UINT16));
	
	pTC0100SCNFgTempDraw[Chip] = (UINT16*)BurnMalloc(1024 * 512 * sizeof(UINT16));
	memset(pTC0100SCNFgTempDraw[Chip], 0, 1024 * 512 * sizeof(UINT16));
	
	TC0100SCNXOffset[Chip] = xOffset;
	TC0100SCNYOffset[Chip] = yOffset;
	TC0100SCNFlipScreenX[Chip] = xFlip;
	TC0100SCNPriorityMap[Chip] = PriorityMap;
	TC0100SCNColourDepth[Chip] = 4;
	TC0100SCNGfxMask[Chip] = nNumTiles - 1;
	TC0100SCNDblWidth[Chip] = 0;	
	TC0100SCNClipWidth[Chip] = nScreenWidth;
	TC0100SCNClipHeight[Chip] = nScreenHeight;
	TC0100SCNClipStartX[Chip] = 0;
	TC0100SCNPaletteOffset[Chip] = 0;
	TC0100SCNFgLayerUpdate[Chip] = 1;
	TC0100SCNBgLayerUpdate[Chip] = 1;
	
	if (!TC0100SCNClipWidth[Chip] || !TC0100SCNClipHeight[Chip]) bprintf(PRINT_IMPORTANT, _T("TC0100SCNInit called before GenericTilesInit\n"));
	
	TaitoIC_TC0100SCNInUse = 1;
	TC0100SCNNum++;
}

void TC0100SCNSetColourDepth(INT32 Chip, INT32 ColourDepth)
{
	TC0100SCNColourDepth[Chip] = ColourDepth;
}

void TC0100SCNSetGfxMask(INT32 Chip, INT32 Mask)
{
	TC0100SCNGfxMask[Chip] = Mask;
}

void TC0100SCNSetGfxBank(INT32 Chip, INT32 Bank)
{
	TC0100SCNGfxBank[Chip] = Bank & 0x01;
}

void TC0100SCNSetClipArea(INT32 Chip, INT32 ClipWidth, INT32 ClipHeight, INT32 ClipStartX)
{
	TC0100SCNClipWidth[Chip] = ClipWidth;
	TC0100SCNClipHeight[Chip] = ClipHeight;
	TC0100SCNClipStartX[Chip] = ClipStartX;
}

void TC0100SCNSetPaletteOffset(INT32 Chip, INT32 PaletteOffset)
{
	TC0100SCNPaletteOffset[Chip] = PaletteOffset;
}

void TC0100SCNExit()
{
	for (INT32 i = 0; i < TC0100SCNNum; i++) {
		BurnFree(TC0100SCNRam[i]);
		BurnFree(TC0100SCNChars[i]);
		BurnFree(pTC0100SCNBgTempDraw[i]);
		BurnFree(pTC0100SCNFgTempDraw[i]);
		
		memset(TC0100SCNCtrl[i], 0, 8);
		
		BgScrollX[i] = 0;
		BgScrollY[i] = 0;
		FgScrollX[i] = 0;
		FgScrollY[i] = 0;
		CharScrollX[i] = 0;
		CharScrollY[i] = 0;
		TC0100SCNXOffset[i] = 0;
		TC0100SCNYOffset[i] = 0;
		TC0100SCNFlipScreenX[i] = 0;
		TC0100SCNFlip[i] = 0;
		TC0100SCNPriorityMap[i] = NULL;
		TC0100SCNColourDepth[i] = 0;
		TC0100SCNGfxBank[i] = 0;
		TC0100SCNGfxMask[i] = 0;
		TC0100SCNDblWidth[i] = 0;
		TC0100SCNClipWidth[i] = 0;
		TC0100SCNClipHeight[i] = 0;
		TC0100SCNClipStartX[i] = 0;
		TC0100SCNPaletteOffset[i] = 0;
		TC0100SCNBgLayerUpdate[i] = 0;
		TC0100SCNFgLayerUpdate[i] = 0;
	}
	
	TC0100SCNNum = 0;
}

void TC0100SCNScan(INT32 nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = TC0100SCNRam[0];
		ba.nLen	  = 0x14000;
		ba.szName = "TC0100SCN Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(TC0100SCNCtrl[0]);
		SCAN_VAR(BgScrollX);
		SCAN_VAR(BgScrollY);
		SCAN_VAR(FgScrollX);
		SCAN_VAR(FgScrollY);
		SCAN_VAR(CharScrollX);
		SCAN_VAR(CharScrollY);
		SCAN_VAR(TC0100SCNFlip);
		SCAN_VAR(TC0100SCNGfxBank);
		SCAN_VAR(TC0100SCNDblWidth);
	}
}
