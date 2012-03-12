// PC090OJ

#include "tiles_generic.h"
#include "taito_ic.h"

UINT8 *PC090OJRam = NULL;
static INT32 PC090OJNumTiles;
static INT32 PC090OJXOffset;
static INT32 PC090OJYOffset;
static INT32 PC090OJUseBuffer;
static INT32 PC090OJPaletteOffset;
INT32 PC090OJSpriteCtrl;

void PC090OJDrawSprites(UINT8 *pSrc)
{
	UINT16 *VideoRam = (UINT16*)PC090OJRam;
	
	INT32 PC090OJCtrl = BURN_ENDIAN_SWAP_INT16(VideoRam[0xdff]);
	
	INT32 Offset;
	INT32 SpriteColBank = (PC090OJSpriteCtrl & 0x0f) << 4;
	
	for (Offset = 0x400 - 4; Offset >= 0; Offset -= 4)
	{
		INT32 xFlip, yFlip;
		INT32 x, y;
		INT32 Data, Code, Colour;

		Data = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 0]);
		yFlip = (Data & 0x8000) >> 15;
		xFlip = (Data & 0x4000) >> 14;
		Colour = (Data & 0x000f) | SpriteColBank | PC090OJPaletteOffset;

		Code = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 2]) & 0x1fff;
		if (Code >= PC090OJNumTiles) {
			Code %= PC090OJNumTiles; // ok? Mod seems to work fine for bonze.
		}

		x = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 3]) & 0x1ff;
		y = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 1]) & 0x1ff;

		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;
		
		if (!(PC090OJCtrl & 1))	{
			x = 320 - x - 16;
			y = 256 - y - 16;
			xFlip = !xFlip;
			yFlip = !yFlip;
		}

		x -= PC090OJXOffset;
		y -= PC090OJYOffset;

		if (x > 16 && x < (nScreenWidth - 16) && y > 16 && y < (nScreenHeight - 16)) {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				} else {
					Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				}
			}
		} else {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, pSrc);
				}
			}
		}
	}
}

void PC090OJReset()
{
	PC090OJSpriteCtrl = 0;
}

void PC090OJInit(INT32 nNumTiles, INT32 xOffset, INT32 yOffset, INT32 UseBuffer)
{
	PC090OJRam = (UINT8*)BurnMalloc(0x4000);
	memset(PC090OJRam, 0, 0x4000);
	
	PC090OJNumTiles = nNumTiles;
	
	PC090OJXOffset = xOffset;
	PC090OJYOffset = yOffset;
	PC090OJUseBuffer = UseBuffer;
	PC090OJPaletteOffset = 0;
	
	TaitoIC_PC090OJInUse = 1;
}

void PC090OJSetPaletteOffset(INT32 Offset)
{
	PC090OJPaletteOffset = Offset;
}

void PC090OJExit()
{
	BurnFree(PC090OJRam);
	
	PC090OJNumTiles = 0;
	PC090OJXOffset = 0;
	PC090OJYOffset = 0;
	PC090OJUseBuffer = 0;
	PC090OJPaletteOffset = 0;
	
	PC090OJSpriteCtrl = 0;
}

void PC090OJScan(INT32 nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = PC090OJRam;
		ba.nLen	  = 0x4000;
		ba.szName = "PC090OJ Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(PC090OJSpriteCtrl);
	}
}
