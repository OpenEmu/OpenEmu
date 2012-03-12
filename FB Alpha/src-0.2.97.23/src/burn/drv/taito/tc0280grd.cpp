// TC0280GRD

#include "tiles_generic.h"
#include "taito_ic.h"

UINT8 *TC0280GRDRam;
static UINT16 TC0280GRDCtrl[8];
INT32 TC0280GRDBaseColour;
static INT32 TC0280GRDxMultiply;
static INT32 TC0280GRDXOffset;
static INT32 TC0280GRDYOffset;
static UINT8 *pTC0280GRDSrc = NULL;

static UINT16 *pRozTileMapData = NULL;

#define PLOTPIXEL_MASK(x, mc, po) if (pTileData[x] != mc) {pPixel[x] = nPalette | pTileData[x] | po;}

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

#undef PLOTPIXEL_MASK

static void RozRender(UINT32 xStart, UINT32 yStart, INT32 xxInc, INT32 xyInc, INT32 yxInc, INT32 yyInc)
{
	UINT32 cx;
	UINT32 cy;
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0, Pix;
	UINT16 *Dest = NULL;	
	UINT16 *VideoRam = (UINT16*)TC0280GRDRam;
	
	if (xxInc == (1 << 16) && xyInc == 0 && yxInc == 0 && yyInc == (1 << 16)) {
		for (my = 0; my < 64; my++) {
			for (mx = 0; mx < 64; mx++) {
				Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
				Code = (Attr & 0x3fff);
				Colour = ((Attr & 0xc000) >> 14) + TC0280GRDBaseColour;
			
				x = 8 * mx;
				y = 8 * my;
				
				x -= (xStart >> 16) & 0x1ff;
				y -= (yStart >> 16) & 0x1ff;
				
				if (x < -8) x += 512;
				if (y < -8) y += 512;

				if (x >= 0 && x < (nScreenWidth - 8) && y >= 0 && y < (nScreenHeight - 8)) {
					Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, pTC0280GRDSrc);
				} else {
					Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pTC0280GRDSrc);
				}
			
				TileIndex++;
			}
		}
		
		return;
	}
	
	memset(pRozTileMapData, 0, 512 * 512 * sizeof(UINT16));
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < 64; mx++) {
			Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
			Code = (Attr & 0x3fff);
			Colour = ((Attr & 0xc000) >> 14) + TC0280GRDBaseColour;
			
			x = 8 * mx;
			y = 8 * my;

			RenderTile_Mask(pRozTileMapData, Code, x, y, Colour, 4, 0, 0, pTC0280GRDSrc);

			TileIndex++;
		}
	}
	
	y = 0;
	while (y < nScreenHeight) {
		x = 0;
		cx = xStart;
		cy = yStart;

		Dest = pTransDraw + (y * nScreenWidth);
	
		while (x < nScreenWidth) {
			Pix = pRozTileMapData[(((cy >> 16) & 0x1ff) * 512) + ((cx >> 16) & 0x1ff)];
			if (Pix) {
				*Dest++ = Pix;
			} else {
				Dest++;
			}
					
			cx += xxInc;
			cy += xyInc;
			x++;
		}
			
		xStart += yxInc;
		yStart += yyInc;
		y++;
	}
}

void TC0280GRDRenderLayer()
{
	UINT32 xStart, yStart;
	INT32 xxInc, xyInc, yxInc, yyInc;

	xStart = ((TC0280GRDCtrl[0] & 0xff) << 16) + TC0280GRDCtrl[1];
	if (xStart & 0x800000) xStart -= 0x1000000;
	xxInc = (INT16)TC0280GRDCtrl[2];
	xxInc *= TC0280GRDxMultiply;
	yxInc = (INT16)TC0280GRDCtrl[3];

	yStart = ((TC0280GRDCtrl[4] & 0xff) << 16) + TC0280GRDCtrl[5];
	if (yStart & 0x800000) yStart -= 0x1000000;
	xyInc = (INT16)TC0280GRDCtrl[6];
	xyInc *= TC0280GRDxMultiply;
	yyInc = (INT16)TC0280GRDCtrl[7];

	xStart -= TC0280GRDXOffset * xxInc + TC0280GRDYOffset * yxInc;
	yStart -= TC0280GRDXOffset * xyInc + TC0280GRDYOffset * yyInc;
	
	RozRender(xStart << 4, yStart << 4, xxInc << 4, xyInc << 4, yxInc << 4, yyInc << 4);
}

void TC0280GRDCtrlWordWrite(UINT32 Offset, UINT16 Data)
{
	TC0280GRDCtrl[Offset] = Data;
}

void TC0280GRDReset()
{
	memset(TC0280GRDCtrl, 0, 8);
	
	TC0280GRDBaseColour = 0;
}

void TC0280GRDInit(INT32 xOffs, INT32 yOffs, UINT8 *pSrc)
{
	TC0280GRDRam = (UINT8*)BurnMalloc(0x2000);
	memset(TC0280GRDRam, 0, 0x2000);
	
	pRozTileMapData = (UINT16*)BurnMalloc(512 * 512 * sizeof(UINT16));
	memset(pRozTileMapData, 0, 512 * 512 * sizeof(UINT16));
	
	TC0280GRDXOffset = xOffs;
	TC0280GRDYOffset = yOffs;
	
	pTC0280GRDSrc = pSrc;
	
	TC0280GRDxMultiply = 2;
	
	TaitoIC_TC0280GRDInUse = 1;
}

void TC0430GRWInit(INT32 xOffs, INT32 yOffs, UINT8 *pSrc)
{
	TC0280GRDInit(xOffs, yOffs, pSrc);
	
	TC0280GRDxMultiply = 1;
	
	TaitoIC_TC0280GRDInUse = 0;
	TaitoIC_TC0430GRWInUse = 1;
}

void TC0280GRDExit()
{
	BurnFree(TC0280GRDRam);
	BurnFree(pRozTileMapData);
	
	memset(TC0280GRDCtrl, 0, 8);
	
	TC0280GRDBaseColour = 0;
	
	TC0280GRDxMultiply = 0;
	TC0280GRDXOffset = 0;
	TC0280GRDYOffset = 0;
	
	pTC0280GRDSrc = NULL;
}

void TC0280GRDScan(INT32 nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = TC0280GRDRam;
		ba.nLen	  = 0x2000;
		ba.szName = "TC0280GRD Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(TC0280GRDCtrl);
		SCAN_VAR(TC0280GRDBaseColour);
	}
}
