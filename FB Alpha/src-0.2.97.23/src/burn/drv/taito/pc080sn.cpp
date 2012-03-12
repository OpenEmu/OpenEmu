// PC080SN

#include "tiles_generic.h"
#include "taito_ic.h"

UINT8 *PC080SNRam[PC080SN_MAX_CHIPS] = { NULL, NULL };
static UINT16 PC080SNCtrl[PC080SN_MAX_CHIPS][8];
static INT32 BgScrollX[PC080SN_MAX_CHIPS];
static INT32 BgScrollY[PC080SN_MAX_CHIPS];
static INT32 FgScrollX[PC080SN_MAX_CHIPS];
static INT32 FgScrollY[PC080SN_MAX_CHIPS];
static INT32 PC080SNNumTiles[PC080SN_MAX_CHIPS];
static INT32 PC080SNXOffset[PC080SN_MAX_CHIPS];
static INT32 PC080SNYOffset[PC080SN_MAX_CHIPS];
static INT32 PC080SNFgTransparentPen[PC080SN_MAX_CHIPS];
static INT32 PC080SNYInvert[PC080SN_MAX_CHIPS];
static INT32 PC080SNDblWidth[PC080SN_MAX_CHIPS];
static INT32 PC080SNCols[PC080SN_MAX_CHIPS];
static INT32 PC080SNNum = 0;

void PC080SNDrawBgLayer(INT32 Chip, INT32 Opaque, UINT8 *pSrc, UINT16 *pDest)
{
	INT32 mx, my, Offset, Attr, Code, Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)PC080SNRam[Chip] + 0x0000;
	UINT16 *BgScrollRam = NULL;

	if (!PC080SNDblWidth[Chip]) BgScrollRam = (UINT16*)PC080SNRam[Chip] + 0x2000;
	
	INT32 BgScrollActive = 0;
	if (BgScrollRam) {
		for (INT32 i = 0; i < 512; i++) {
			if (BURN_ENDIAN_SWAP_INT16(BgScrollRam[i])) {
				BgScrollActive = 1;
				break;
			}
		}
	}
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < PC080SNCols[Chip]; mx++) {
			Offset = 2 * TileIndex;
			if (!PC080SNDblWidth[Chip]) {
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 0]);
				Code = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 1]) & (PC080SNNumTiles[Chip] - 1);
			} else {
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex + 0x0000]);
				Code = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex + 0x2000]) & 0x3fff;
			}
			Colour = Attr & 0x1ff;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= 16;
			
			x -= PC080SNXOffset[Chip];
			y -= PC080SNYOffset[Chip];
			
			if (BgScrollActive) {
				INT32 px, py;
			
				UINT32 nPalette = (Colour << 4);
			
				for (py = 0; py < 8; py++) {
					for (px = 0; px < 8; px++) {
						UINT8 c = pSrc[(Code * 64) + (py * 8) + px];
						if (xFlip) c = pSrc[(Code * 64) + (py * 8) + (7 - px)];
						if (yFlip) c = pSrc[(Code * 64) + ((7 - py) * 8) + px];
						if (xFlip && yFlip) c = pSrc[(Code * 64) + ((7 - py) * 8) + (7 - px)];
					
						if (c || Opaque) {
							INT32 xPos = x + px;
							INT32 yPos = y + py;
							yPos -= BgScrollY[Chip] & 0x1ff;
					
							if (yPos < -8) yPos += 512;
							if (yPos >= 512) yPos -= 512;			
					
							if (yPos >= 0 && yPos < nScreenHeight) {
								if (!PC080SNDblWidth[Chip]) {					
									xPos -= ((BgScrollX[Chip] - BURN_ENDIAN_SWAP_INT16(BgScrollRam[yPos + PC080SNYOffset[Chip]])) & 0x1ff);
									if (xPos < -8) xPos += 512;
									if (xPos >= 512) xPos -= 512;
								} else {
									xPos -= BgScrollX[Chip] & 0x3ff;
									if (xPos < -8) xPos += 1024;
									if (xPos >= 1024) xPos -= 1024;
								}							
							
								UINT16* pPixel = pDest + (yPos * nScreenWidth);
						
								if (xPos >= 0 && xPos < nScreenWidth) {
									pPixel[xPos] = c | nPalette;
								}
							}
						}
					}
				}
			} else {
				if (!PC080SNDblWidth[Chip]) {
					x -= BgScrollX[Chip] & 0x1ff;
					if (x < -8) x += 512;
					if (x >= 512) x -= 512;
				} else {
					x -= BgScrollX[Chip] & 0x3ff;
					if (x < -8) x += 1024;
					if (x >= 1024) x -= 1024;
				}
				
				y -= BgScrollY[Chip] & 0x1ff;
				if (y < -8) y += 512;
				if (y >= 512) y -= 512;
				
				if (Opaque) {
					if (x > 8 && x < (nScreenWidth - 8) && y > 8 && y < (nScreenHeight - 8)) {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_FlipXY(pDest, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile_FlipX(pDest, Code, x , y, Colour, 4, 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_FlipY(pDest, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile(pDest, Code, x , y, Colour, 4, 0, pSrc);
							}
						}
					} else {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_FlipXY_Clip(pDest, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile_FlipX_Clip(pDest, Code, x , y, Colour, 4, 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_FlipY_Clip(pDest, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile_Clip(pDest, Code, x , y, Colour, 4, 0, pSrc);
							}
						}
					}
				} else {
					if (x > 8 && x < (nScreenWidth - 8) && y > 8 && y < (nScreenHeight - 8)) {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_Mask_FlipXY(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
							} else {
								Render8x8Tile_Mask_FlipX(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_Mask_FlipY(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
							} else {
								Render8x8Tile_Mask(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
							}
						}
					} else {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_Mask_FlipXY_Clip(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
							} else {
								Render8x8Tile_Mask_FlipX_Clip(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_Mask_FlipY_Clip(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
							} else {
								Render8x8Tile_Mask_Clip(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
							}
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}

void PC080SNDrawFgLayer(INT32 Chip, INT32 Opaque, UINT8 *pSrc, UINT16 *pDest)
{
	INT32 mx, my, Offset, Attr, Code, Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)PC080SNRam[Chip] + 0x4000;
	UINT16 *FgScrollRam = NULL;

	if (!PC080SNDblWidth[Chip]) FgScrollRam = (UINT16*)PC080SNRam[Chip] + 0x6000;
	
	INT32 FgScrollActive = 0;
	if (FgScrollRam) {
		for (INT32 i = 0; i < 512; i++) {
			if (BURN_ENDIAN_SWAP_INT16(FgScrollRam[i])) {
				FgScrollActive = 1;
				break;
			}
		}
	}
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < PC080SNCols[Chip]; mx++) {
			Offset = 2 * TileIndex;
			if (!PC080SNDblWidth[Chip]) {
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 0]);
				Code = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 1]) & (PC080SNNumTiles[Chip] - 1);
			} else {
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex + 0x0000]);
				Code = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex + 0x2000]) & 0x3fff;
			}
			Colour = Attr & 0x1ff;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= 16;
			
			x -= PC080SNXOffset[Chip];
			y -= PC080SNYOffset[Chip];
			
			if (FgScrollActive) {
				INT32 px, py;
			
				UINT32 nPalette = (Colour << 4);
			
				for (py = 0; py < 8; py++) {
					for (px = 0; px < 8; px++) {
						UINT8 c = pSrc[(Code * 64) + (py * 8) + px];
						if (xFlip) c = pSrc[(Code * 64) + (py * 8) + (7 - px)];
						if (yFlip) c = pSrc[(Code * 64) + ((7 - py) * 8) + px];
						if (xFlip && yFlip) c = pSrc[(Code * 64) + ((7 - py) * 8) + (7 - px)];
					
						if (c != PC080SNFgTransparentPen[Chip] || Opaque) {
							INT32 xPos = x + px;
							INT32 yPos = y + py;
							yPos -= FgScrollY[Chip] & 0x1ff;
					
							if (yPos < -8) yPos += 512;
							if (yPos >= 512) yPos -= 512;			
					
							if (yPos >= 0 && yPos < nScreenHeight) {					
								if (!PC080SNDblWidth[Chip]) {
									xPos -= ((FgScrollX[Chip] - BURN_ENDIAN_SWAP_INT16(FgScrollRam[yPos + PC080SNYOffset[Chip]])) & 0x1ff);
									if (xPos < -8) xPos += 512;
									if (xPos >= 512) xPos -= 512;
								} else {
									xPos -= FgScrollX[Chip] & 0x3ff;
									if (xPos < -8) xPos += 1024;
									if (xPos >= 1024) xPos -= 1024;
								}
						
								UINT16* pPixel = pDest + (yPos * nScreenWidth);
							
								if (xPos >= 0 && xPos < nScreenWidth) {
									pPixel[xPos] = c | nPalette;
								}
							}
						}
					}
				}
			} else {
				if (!PC080SNDblWidth[Chip]) {
					x -= FgScrollX[Chip] & 0x1ff;
					if (x < -8) x += 512;
					if (x >= 512) x -= 512;
				} else {
					x -= FgScrollX[Chip] & 0x3ff;
					if (x < -8) x += 1024;
					if (x >= 1024) x -= 1024;
				}
				
				y -= FgScrollY[Chip] & 0x1ff;
				if (y < -8) y += 512;
				if (y >= 512) y -= 512;
				
				if (Opaque) {
					if (x > 8 && x < (nScreenWidth - 8) && y > 8 && y < (nScreenHeight - 8)) {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_FlipXY(pDest, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile_FlipX(pDest, Code, x , y, Colour, 4, 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_FlipY(pDest, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile(pDest, Code, x , y, Colour, 4, 0, pSrc);
							}
						}
					} else {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_FlipXY_Clip(pDest, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile_FlipX_Clip(pDest, Code, x , y, Colour, 4, 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_FlipY_Clip(pDest, Code, x , y, Colour, 4, 0, pSrc);
							} else {
								Render8x8Tile_Clip(pDest, Code, x , y, Colour, 4, 0, pSrc);
							}
						}
					}
				} else {
					if (x > 8 && x < (nScreenWidth - 8) && y > 8 && y < (nScreenHeight - 8)) {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_Mask_FlipXY(pDest, Code, x , y, Colour, 4, PC080SNFgTransparentPen[Chip], 0, pSrc);
							} else {
								Render8x8Tile_Mask_FlipX(pDest, Code, x , y, Colour, 4, PC080SNFgTransparentPen[Chip], 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_Mask_FlipY(pDest, Code, x , y, Colour, 4, PC080SNFgTransparentPen[Chip], 0, pSrc);
							} else {
								Render8x8Tile_Mask(pDest, Code, x , y, Colour, 4, PC080SNFgTransparentPen[Chip], 0, pSrc);
							}
						}
					} else {
						if (xFlip) {
							if (yFlip) {
								Render8x8Tile_Mask_FlipXY_Clip(pDest, Code, x , y, Colour, 4, PC080SNFgTransparentPen[Chip], 0, pSrc);
							} else {
								Render8x8Tile_Mask_FlipX_Clip(pDest, Code, x , y, Colour, 4, PC080SNFgTransparentPen[Chip], 0, pSrc);
							}
						} else {
							if (yFlip) {
								Render8x8Tile_Mask_FlipY_Clip(pDest, Code, x , y, Colour, 4, PC080SNFgTransparentPen[Chip], 0, pSrc);
							} else {
								Render8x8Tile_Mask_Clip(pDest, Code, x , y, Colour, 4, PC080SNFgTransparentPen[Chip], 0, pSrc);
							}
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}

void PC080SNSetScrollX(INT32 Chip, UINT32 Offset, UINT16 Data)
{
	PC080SNCtrl[Chip][Offset] = Data;
	
	switch (Offset) {
		case 0: {
			BgScrollX[Chip] = -Data;
			break;
		}
		
		case 1: {
			FgScrollX[Chip] = -Data;
			break;
		}
	}
}

void PC080SNSetScrollY(INT32 Chip, UINT32 Offset, UINT16 Data)
{
	PC080SNCtrl[Chip][Offset + 2] = Data;
	
	if (PC080SNYInvert[Chip]) Data = -Data;
	
	switch (Offset) {
		case 0: {
			BgScrollY[Chip] = -Data;
			break;
		}
		
		case 1: {
			FgScrollY[Chip] = -Data;
			break;
		}
	}
}

void PC080SNCtrlWrite(INT32 Chip, UINT32 Offset, UINT16 Data)
{
	PC080SNCtrl[Chip][Offset + 4] = Data;
	
	switch (Offset) {
		case 0: {
			if (Data & 1) bprintf(PRINT_NORMAL, _T("PC080SN Flipped\n"));
			return;
		}
	}
}

void PC080SNOverrideFgScroll(INT32 Chip, INT32 xScroll, INT32 yScroll)
{
	FgScrollX[Chip] = xScroll;
	FgScrollY[Chip] = yScroll;
}

void PC080SNReset()
{
	for (INT32 i = 0; i < PC080SNNum; i++) {
		memset(PC080SNCtrl[i], 0, 8 * sizeof(UINT16));
		BgScrollX[i] = 0;
		BgScrollY[i] = 0;
		FgScrollX[i] = 0;
		FgScrollY[i] = 0;
	}
}

void PC080SNInit(INT32 Chip, INT32 nNumTiles, INT32 xOffset, INT32 yOffset, INT32 yInvert, INT32 DblWidth)
{
	PC080SNRam[Chip] = (UINT8*)BurnMalloc(0x10000);
	memset(PC080SNRam[Chip], 0, 0x10000);
	
	PC080SNNumTiles[Chip] = nNumTiles;
	
	PC080SNXOffset[Chip] = xOffset;
	PC080SNYOffset[Chip] = yOffset;
	
	PC080SNFgTransparentPen[Chip] = 0;
	PC080SNYInvert[Chip] = yInvert;
	
	TaitoIC_PC080SNInUse = 1;
	
	PC080SNCols[Chip] = 64;
	if (DblWidth) PC080SNCols[Chip] = 128;
	
	PC080SNDblWidth[Chip] = DblWidth;
	PC080SNNum++;
}

void PC080SNSetFgTransparentPen(INT32 Chip, INT32 Pen)
{
	PC080SNFgTransparentPen[Chip] = Pen;
}

void PC080SNExit()
{
	for (INT32 i = 0; i < PC080SNNum; i++) {
		BurnFree(PC080SNRam[i]);

		memset(PC080SNCtrl[i], 0, 8 * sizeof(UINT16));
		BgScrollX[i] = 0;
		BgScrollY[i] = 0;
		FgScrollX[i] = 0;
		FgScrollY[i] = 0;
	
		PC080SNNumTiles[i] = 0;
		PC080SNXOffset[i] = 0;
		PC080SNYOffset[i] = 0;
		PC080SNFgTransparentPen[i] = 0;
		PC080SNYInvert[i] = 0;
		PC080SNDblWidth[i] = 0;
		PC080SNCols[i] = 0;
	}
	
	PC080SNNum = 0;
}

void PC080SNScan(INT32 nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		for (INT32 i = 0; i < PC080SNNum; i++) {
			memset(&ba, 0, sizeof(ba));
			ba.Data	  = PC080SNRam[i];
			ba.nLen	  = 0x10000;
			ba.szName = "PC080SN Ram";
			BurnAcb(&ba);
		}
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		for (INT32 i = 0; i < PC080SNNum; i++) {
			SCAN_VAR(PC080SNCtrl[i]);
			SCAN_VAR(BgScrollX[i]);
			SCAN_VAR(BgScrollY[i]);
			SCAN_VAR(FgScrollX[i]);
			SCAN_VAR(FgScrollY[i]);
		}
	}
}

// Top Speed Custom support

#define PLOTPIXEL_MASK(x, mc, po) if (pTileData[x] != mc) {pPixel[x] = nPalette | pTileData[x] | po;}
#define PLOTPIXEL_MASK_FLIPX(x, a, mc, po) if (pTileData[a] != mc) {pPixel[x] = nPalette | pTileData[a] | po;}

static void RenderTile_Mask(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile)
{
	UINT32 nPalette = nTilePalette << nColourDepth;
	pTileData = pTile + (nTileNumber << 6);
	
	UINT16* pPixel = pDestDraw + (StartY * 512) + StartX;

	for (INT32 y = 0; y < 8; y++, pPixel += 512, pTileData += 8) {
		PLOTPIXEL_MASK( 0, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 1, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 2, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 3, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 4, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 5, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 6, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 7, nMaskColour, nPaletteOffset);
	}
}

static void RenderTile_Mask_FlipX(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile)
{
	UINT32 nPalette = nTilePalette << nColourDepth;
	pTileData = pTile + (nTileNumber << 6);
	
	UINT16* pPixel = pDestDraw + (StartY * 512) + StartX;

	for (INT32 y = 0; y < 8; y++, pPixel += 512, pTileData += 8) {
		PLOTPIXEL_MASK_FLIPX( 7, 0, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 6, 1, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 5, 2, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 4, 3, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 3, 4, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 2, 5, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 1, 6, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 0, 7, nMaskColour, nPaletteOffset);
	}
}

static void RenderTile_Mask_FlipY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile)
{
	UINT32 nPalette = nTilePalette << nColourDepth;
	pTileData = pTile + (nTileNumber << 6);
	
	UINT16* pPixel = pDestDraw + ((StartY + 7) * 512) + StartX;

	for (INT32 y = 7; y >= 0; y--, pPixel -= 512, pTileData += 8) {
		PLOTPIXEL_MASK( 0, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 1, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 2, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 3, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 4, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 5, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 6, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK( 7, nMaskColour, nPaletteOffset);
	}
}

static void RenderTile_Mask_FlipXY(UINT16* pDestDraw, INT32 nTileNumber, INT32 StartX, INT32 StartY, INT32 nTilePalette, INT32 nColourDepth, INT32 nMaskColour, INT32 nPaletteOffset, UINT8 *pTile)
{
	UINT32 nPalette = nTilePalette << nColourDepth;
	pTileData = pTile + (nTileNumber << 6);
	
	UINT16* pPixel = pDestDraw + ((StartY + 7) * 512) + StartX;

	for (INT32 y = 7; y >= 0; y--, pPixel -= 512, pTileData += 8) {
		PLOTPIXEL_MASK_FLIPX( 7, 0, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 6, 1, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 5, 2, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 4, 3, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 3, 4, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 2, 5, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 1, 6, nMaskColour, nPaletteOffset);
		PLOTPIXEL_MASK_FLIPX( 0, 7, nMaskColour, nPaletteOffset);
	}
}

#undef PLOTPIXEL_MASK
#undef PLOTPIXEL_MASK_FLIPX

void TopspeedPC080SNDrawBgLayer(INT32 Chip, UINT8 *pSrc, UINT16 *pDest)
{
	INT32 mx, my, Offset, Attr, Code, Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)PC080SNRam[Chip] + 0x0000;
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < PC080SNCols[Chip]; mx++) {
			Offset = 2 * TileIndex;
			if (!PC080SNDblWidth[Chip]) {
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 0]);
				Code = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 1]) & (PC080SNNumTiles[Chip] - 1);
			} else {
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex + 0x0000]);
				Code = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex + 0x2000]) & 0x3fff;
			}
			Colour = Attr & 0x1ff;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			if (xFlip) {
				if (yFlip) {
					RenderTile_Mask_FlipXY(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
				} else {
					RenderTile_Mask_FlipX(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
				}
			} else {
				if (yFlip) {
					RenderTile_Mask_FlipY(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
				} else {
					RenderTile_Mask(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
				}
			}

			TileIndex++;
		}
	}
}

void TopspeedPC080SNDrawFgLayer(INT32 Chip, UINT8 *pSrc, UINT16 *pDest)
{
	INT32 mx, my, Offset, Attr, Code, Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)PC080SNRam[Chip] + 0x4000;
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < PC080SNCols[Chip]; mx++) {
			Offset = 2 * TileIndex;
			if (!PC080SNDblWidth[Chip]) {
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 0]);
				Code = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 1]) & (PC080SNNumTiles[Chip] - 1);
			} else {
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex + 0x0000]);
				Code = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex + 0x2000]) & 0x3fff;
			}
			Colour = Attr & 0x1ff;
			Flip = (Attr & 0xc000) >> 14;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			if (xFlip) {
				if (yFlip) {
					RenderTile_Mask_FlipXY(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
				} else {
					RenderTile_Mask_FlipX(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
				}
			} else {
				if (yFlip) {
					RenderTile_Mask_FlipY(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
				} else {
					RenderTile_Mask(pDest, Code, x , y, Colour, 4, 0, 0, pSrc);
				}
			}

			TileIndex++;
		}
	}
}

static inline void DrawScanLine(INT32 y, const UINT16 *src, INT32 Transparent, INT32 /*Pri*/)
{
	UINT16* pPixel;
	INT32 Length;
	
	pPixel = pTransDraw + (y * nScreenWidth);
	
	Length = nScreenWidth;
	
	if (Transparent) {
		while (Length--) {
			UINT16 sPixel = *src++;
			if (sPixel < 0x7fff) {
				*pPixel = sPixel;
			}
			pPixel++;
		}
	} else {
		while (Length--) {
			*pPixel++ = *src++;
		}
	}
}

static UINT16 TopspeedGetRoadPixelColour(UINT16 Pixel, UINT16 Colour)
{
	UINT16 RoadBodyColour, OffRoadColour, PixelType;

	PixelType = (Pixel % 0x10);
	RoadBodyColour = (Pixel & 0x7ff0) + 4;
	OffRoadColour = RoadBodyColour + 1;

	if ((Colour & 0xffe0) == 0xffe0) {
		Pixel += 10;
		RoadBodyColour += 10;
		OffRoadColour  += 10;
	} else {
		if (Colour & 0x10) RoadBodyColour += 5;
		if (Colour & 0x02) OffRoadColour  += 5;
	}

	switch (PixelType) {
		case 0x01: {
			if (Colour & 0x08) Pixel = RoadBodyColour;
			break;
		}
		
		case 0x02: {
			if (Colour & 0x08) Pixel = RoadBodyColour;
			break;
		}
		
		case 0x03: {
			if (Colour & 0x04) Pixel = RoadBodyColour;
			break;
		}
	
		case 0x04: {
			Pixel = RoadBodyColour;
			break;
		}
	
		case 0x05: {
			Pixel = OffRoadColour;
			break;
		}
	}
	
	return Pixel;
}

void TopspeedDrawBgLayer(INT32 Chip, UINT8 *pSrc, UINT16 *pDest, UINT16 *ColourCtrlRam)
{
	memset(pDest, 0, 512 * 512 * sizeof(UINT16));
	TopspeedPC080SNDrawBgLayer(Chip, pSrc, pDest);
	
	UINT16 *BgScrollRam = NULL;

	if (!PC080SNDblWidth[Chip]) BgScrollRam = (UINT16*)PC080SNRam[Chip] + 0x2000;
	
	UINT16 *Dst16, *Src16;
	UINT16 ScanLine[512];

	UINT16 a, Colour;
	INT32 sx, xIndex;
	INT32 i, y, yIndex, ySrcIndex, RowIndex;

	INT32 min_x = 0;
	INT32 max_x = nScreenWidth - 1;
	INT32 min_y = 0;
	INT32 max_y = nScreenHeight - 1;
	INT32 ScreenWidth = max_x - min_x + 1;
	INT32 WidthMask = 0x1ff;

	sx = (BgScrollX[Chip] & 0x1ff) + 16 - PC080SNXOffset[Chip];
	yIndex = (BgScrollY[Chip] & 0x1ff) + min_y + PC080SNYOffset[Chip];
	
	y = min_y;
	
	do {
		ySrcIndex = yIndex & 0x1ff;
		RowIndex = (ySrcIndex - (BgScrollY[Chip] & 0x1ff)) & 0x1ff;
		Colour = ColourCtrlRam[(RowIndex + PC080SNYOffset[Chip] - 2) & 0xff];

		xIndex = sx - (BURN_ENDIAN_SWAP_INT16(BgScrollRam[RowIndex]) & 0x1ff);
		
		Src16 = pDest + (ySrcIndex * 512);
		Dst16 = ScanLine;

		for (i = 0; i < ScreenWidth; i++) {
			if ((Src16[xIndex & WidthMask] & 0x0f) != 0) {
				a = Src16[xIndex & WidthMask];
				a = TopspeedGetRoadPixelColour(a, Colour);

				*Dst16++ = a;
			} else {
				*Dst16++ = 0x8000;
			}
			
			xIndex++;
		}

		DrawScanLine(y, ScanLine, 1, 0);
	
		yIndex++;
		y++;
	}
	while (y <= max_y);
}

void TopspeedDrawFgLayer(INT32 Chip, UINT8 *pSrc, UINT16 *pDest, UINT16 *ColourCtrlRam)
{
	memset(pDest, 0, 512 * 512 * sizeof(UINT16));
	TopspeedPC080SNDrawFgLayer(Chip, pSrc, pDest);
	
	UINT16 *FgScrollRam = NULL;

	if (!PC080SNDblWidth[Chip]) FgScrollRam = (UINT16*)PC080SNRam[Chip] + 0x6000;
	
	UINT16 *Dst16, *Src16;
	UINT16 ScanLine[512];

	UINT16 a, Colour;
	INT32 sx, xIndex;
	INT32 i, y, yIndex, ySrcIndex, RowIndex;

	INT32 min_x = 0;
	INT32 max_x = nScreenWidth - 1;
	INT32 min_y = 0;
	INT32 max_y = nScreenHeight - 1;
	INT32 ScreenWidth = max_x - min_x + 1;
	INT32 WidthMask = 0x1ff;

	sx = (FgScrollX[Chip] & 0x1ff) + 16 - PC080SNXOffset[Chip];
	yIndex = (FgScrollY[Chip] & 0x1ff) + min_y + PC080SNYOffset[Chip];

	y = min_y;
	
	do {
		ySrcIndex = yIndex & 0x1ff;
		RowIndex = (ySrcIndex - (FgScrollY[Chip] & 0x1ff)) & 0x1ff;
		Colour = ColourCtrlRam[(RowIndex + PC080SNYOffset[Chip] - 2) & 0xff];

		xIndex = sx - (BURN_ENDIAN_SWAP_INT16(FgScrollRam[RowIndex]) & 0x1ff);

		Src16 = pDest + (ySrcIndex * 512);
		Dst16 = ScanLine;

		for (i = 0; i < ScreenWidth; i++) {
			if ((Src16[xIndex & WidthMask] & 0x0f) != 0) {
				a = Src16[xIndex & WidthMask];
				a = TopspeedGetRoadPixelColour(a, Colour);

				*Dst16++ = a;
			} else {
				*Dst16++ = 0x8000;
			}
			
			xIndex++;
		}

		DrawScanLine(y, ScanLine, 1, 0);
	
		yIndex++;
		y++;
	}
	while (y <= max_y);
}
