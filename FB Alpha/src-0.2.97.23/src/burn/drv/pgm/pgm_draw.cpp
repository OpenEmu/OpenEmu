#include "pgm.h"
#include "pgm_sprite.h"

static INT32 nTileMask = 0;
static UINT8 sprmsktab[0x100];
static UINT8  *SpritePrio;	// sprite priorities
static UINT16 *pTempScreen;	// sprites
static UINT16 *pTempDraw;	// pre-zoomed sprites
static UINT8  *tiletrans;	// tile transparency table
static UINT8  *texttrans;	// text transparency table

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour & 0x7C00) >> 7;  // Red 
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;	// Green
	g |= g >> 5;
	b = (nColour & 0x001F) << 3;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

static void pgm_prepare_sprite(INT32 wide, INT32 high, INT32 palt, INT32 boffset)
{
	UINT16* dest = pTempDraw;
	UINT8 * bdata = PGMSPRMaskROM;
	INT32 bdatasize = nPGMSPRMaskMaskLen;

	wide *= 16;
	palt *= 32;

	UINT32 aoffset = (bdata[(boffset+3) & bdatasize] << 24) | (bdata[(boffset+2) & bdatasize] << 16) | (bdata[(boffset+1) & bdatasize] << 8) | (bdata[(boffset) & bdatasize]);
	aoffset = (aoffset >> 2) * 3;

	boffset += 4;

	for (INT32 ycnt = 0; ycnt < high; ycnt++)
	{
		for (INT32 xcnt = 0; xcnt < wide; xcnt+=8)
		{
			aoffset+=zoom_draw_table[bdata[boffset & bdatasize]](dest + xcnt, PGMSPRColROM + (aoffset & nPGMSPRColMaskLen), palt);

			boffset++;
		}

		dest += wide;
	}
}

static inline void draw_sprite_line(INT32 wide, UINT16* dest, UINT8 *pdest, INT32 xzoom, INT32 xgrow, INT32 yoffset, INT32 flip, INT32 xpos, INT32 prio)
{
	INT32 xzoombit;
	INT32 xoffset;
	INT32 xcnt = 0, xcntdraw = 0;
	INT32 xdrawpos = 0;

	wide *= 16;
	flip &= 1;

	while (xcnt < wide)
	{
		if (flip) xoffset = wide - xcnt - 1;
		else	  xoffset = xcnt;

		UINT32 srcdat = pTempDraw[yoffset + xoffset];
		xzoombit = (xzoom >> (xcnt & 0x1f)) & 1;

		if (xzoombit == 1 && xgrow == 1)
		{
			xdrawpos = xpos + xcntdraw;

			if (!(srcdat & 0x8000))
			{
				if ((xdrawpos >= 0) && (xdrawpos < nScreenWidth)) {
					dest[xdrawpos] = srcdat;
					pdest[xdrawpos] = prio;
				}

				xdrawpos = xpos + xcntdraw + 1;

				if ((xdrawpos >= 0) && (xdrawpos < nScreenWidth)) {
					dest[xdrawpos] = srcdat;
					pdest[xdrawpos] = prio;
				}
			}

			xcntdraw+=2;
		}
		else if (xzoombit == 1 && xgrow == 0)
		{
			/* skip this column */
		}
		else //normal column
		{
			xdrawpos = xpos + xcntdraw;

			if (!(srcdat & 0x8000))
			{
				if ((xdrawpos >= 0) && (xdrawpos < nScreenWidth)) {
					dest[xdrawpos] = srcdat;
					pdest[xdrawpos] = prio;
				}
			}

			xcntdraw++;
		}

		xcnt++;

		if (xdrawpos == nScreenWidth) break;
	}
}

static void pgm_draw_sprite_nozoom(INT32 wide, INT32 high, INT32 palt, INT32 boffset, INT32 xpos, INT32 ypos, INT32 flipx, INT32 flipy, INT32 prio)
{
	UINT16 *dest = pTempScreen;
	UINT8 *pdest = SpritePrio;
	UINT8 * bdata = PGMSPRMaskROM;
	UINT8 * adata = PGMSPRColROM;
	INT32 bdatasize = nPGMSPRMaskMaskLen;
	INT32 adatasize = nPGMSPRColMaskLen;
	INT32 yoff, xoff;

	UINT16 msk;

	UINT32 aoffset = (bdata[(boffset+3) & bdatasize] << 24) | (bdata[(boffset+2) & bdatasize] << 16) | (bdata[(boffset+1) & bdatasize] << 8) | (bdata[boffset & bdatasize]);
	aoffset = (aoffset >> 2) * 3;
	aoffset &= adatasize;

	boffset += 4;
	wide <<= 4;

	palt <<= 5;

	sprite_draw_nozoom_function *drawsprite = nozoom_draw_table[flipx ? 1 : 0];

	for (INT32 ycnt = 0; ycnt < high; ycnt++) {
		if (flipy) {
			yoff = ypos + ((high-1) - ycnt);
			if (yoff < 0) break;
			if (yoff < nScreenHeight) {
				dest = pTempScreen + (yoff * nScreenWidth);
				pdest = SpritePrio + (yoff * nScreenWidth);
			}
		} else {
			yoff = ypos + ycnt;
			if (yoff >= nScreenHeight) break;
			if (yoff >= 0)  {
				dest = pTempScreen + (yoff * nScreenWidth);
				pdest = SpritePrio + (yoff * nScreenWidth);
			}
		}

		if (yoff >= 0 && yoff < nScreenHeight && xpos >= 0 && (xpos + wide) < nScreenWidth)
		{
			for (INT32 xcnt = 0; xcnt < wide; xcnt+=8)
			{
				if (flipx) {
					xoff = xpos + ((wide - 8) - xcnt);
				} else {
					xoff = xpos + xcnt;
				}

				aoffset += drawsprite[bdata[boffset & bdatasize]](dest + xoff, pdest + xoff, PGMSPRColROM + (aoffset & nPGMSPRColMaskLen), palt, prio);
				boffset++;
			}
		} else {
			for (INT32 xcnt = 0; xcnt < wide; xcnt+=8)
			{
				msk = bdata[boffset & bdatasize] ^ 0xff;
				boffset++;
				aoffset &= adatasize;

				if (yoff < 0 || yoff >= nScreenHeight || msk == 0) {
					aoffset += sprmsktab[msk];

					continue;
				}

				if (flipx) {
					xoff = xpos + (wide - xcnt) - 1;

					if (xoff < -7 || xoff >= nScreenWidth+8) {
						aoffset += sprmsktab[msk];
						continue;
					}

					for (INT32 x = 0; x < 8; x++, xoff--)
					{
						if (msk & 0x0001)
						{
							if (xoff >= 0 && xoff < nScreenWidth) {
								dest[xoff] = adata[aoffset] | palt;
								pdest[xoff] = prio;
							}
		
							aoffset++;
						}
		
						msk >>= 1;
						if (!msk) break;
					}
				} else {
					xoff = xpos + xcnt;

					if (xoff < -7 || xoff >= nScreenWidth) {
						aoffset += sprmsktab[msk];
						continue;
					}

					for (INT32 x = 0; x < 8; x++, xoff++)
					{
						if (msk & 0x0001)
						{
							if (xoff >= 0 && xoff < nScreenWidth) {
								dest[xoff] = adata[aoffset] | palt;
								pdest[xoff] = prio;
							}
		
							aoffset++;
						}
		
						msk >>= 1;
						if (!msk) break;
					}
				}
			}
		}
	}
}

static void draw_sprite_new_zoomed(INT32 wide, INT32 high, INT32 xpos, INT32 ypos, INT32 palt, INT32 boffset, INT32 flip, UINT32 xzoom, INT32 xgrow, UINT32 yzoom, INT32 ygrow, INT32 prio )
{
	if (!wide) return;

	if (yzoom == 0 && xzoom == 0) {
		pgm_draw_sprite_nozoom(wide, high, palt, boffset, xpos, ypos, flip & 1, flip & 2, prio);
		return;
	}

	INT32 ycnt;
	INT32 ydrawpos;
	UINT16 *dest;
	UINT8 *pdest;
	INT32 yoffset;
	INT32 ycntdraw;
	INT32 yzoombit;

	pgm_prepare_sprite(wide, high, palt, boffset);

	ycnt = 0;
	ycntdraw = 0;
	while (ycnt < high)
	{
		yzoombit = (yzoom >> (ycnt&0x1f))&1;

		if (yzoombit == 1 && ygrow == 1) // double this line
		{
			ydrawpos = ypos + ycntdraw;

			if (!(flip&0x02)) yoffset = (ycnt*(wide*16));
			else yoffset = ( (high-ycnt-1)*(wide*16));
			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = pTempScreen + ydrawpos * nScreenWidth;
				pdest = SpritePrio + ydrawpos * nScreenWidth;
				draw_sprite_line(wide, dest, pdest, xzoom, xgrow, yoffset, flip, xpos, prio);
			}
			ycntdraw++;

			ydrawpos = ypos + ycntdraw;
			if (!(flip&0x02)) yoffset = (ycnt*(wide*16));
			else yoffset = ( (high-ycnt-1)*(wide*16));
			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = pTempScreen + ydrawpos * nScreenWidth;
				pdest = SpritePrio + ydrawpos * nScreenWidth;
				draw_sprite_line(wide, dest, pdest, xzoom, xgrow, yoffset, flip, xpos, prio);
			}
			ycntdraw++;

			if (ydrawpos == 224) ycnt = high;
		}
		else if (yzoombit ==1 && ygrow == 0)
		{
			/* skip this line */
			/* we should process anyway if we don't do the pre-decode.. */
		}
		else /* normal line */
		{
			ydrawpos = ypos + ycntdraw;

			if (!(flip&0x02)) yoffset = (ycnt*(wide*16));
			else yoffset = ( (high-ycnt-1)*(wide*16));
			if ((ydrawpos >= 0) && (ydrawpos < 224))
			{
				dest = pTempScreen + ydrawpos * nScreenWidth;
				pdest = SpritePrio + ydrawpos * nScreenWidth;
				draw_sprite_line(wide, dest, pdest, xzoom, xgrow, yoffset, flip, xpos, prio);
			}
			ycntdraw++;

			if (ydrawpos == 224) ycnt = high;
		}

		ycnt++;
	}
}

static void pgm_drawsprites()
{
	UINT16 *source = PGMSprBuf;
	UINT16 *finish = PGMSprBuf + 0xa00/2;
	UINT16 *zoomtable = &PGMVidReg[0x1000/2];

	while (finish > source)
	{
		if (source[4] == 0) break; // right?

		INT32 xpos =  BURN_ENDIAN_SWAP_INT16(source[0]) & 0x07ff;
		INT32 ypos =  BURN_ENDIAN_SWAP_INT16(source[1]) & 0x03ff;
		INT32 xzom = (BURN_ENDIAN_SWAP_INT16(source[0]) & 0x7800) >> 11;
		INT32 xgrow= (BURN_ENDIAN_SWAP_INT16(source[0]) & 0x8000) >> 15;
		INT32 yzom = (BURN_ENDIAN_SWAP_INT16(source[1]) & 0x7800) >> 11;
		INT32 ygrow= (BURN_ENDIAN_SWAP_INT16(source[1]) & 0x8000) >> 15;
		INT32 palt = (BURN_ENDIAN_SWAP_INT16(source[2]) & 0x1f00) >> 8;
		INT32 flip = (BURN_ENDIAN_SWAP_INT16(source[2]) & 0x6000) >> 13;
		INT32 boff =((BURN_ENDIAN_SWAP_INT16(source[2]) & 0x007f) << 16) | (BURN_ENDIAN_SWAP_INT16(source[3]) & 0xffff);
		INT32 wide = (BURN_ENDIAN_SWAP_INT16(source[4]) & 0x7e00) >> 9;
		INT32 prio = (BURN_ENDIAN_SWAP_INT16(source[2]) & 0x0080) >> 7;
		INT32 high =  BURN_ENDIAN_SWAP_INT16(source[4]) & 0x01ff;

		if (xgrow) xzom = 0x10-xzom;
		if (ygrow) yzom = 0x10-yzom;

		UINT32 xzoom = (zoomtable[xzom*2]<<16)|zoomtable[xzom*2+1];
		UINT32 yzoom = (zoomtable[yzom*2]<<16)|zoomtable[yzom*2+1];

		if (xpos > 0x3ff) xpos -=0x800;
		if (ypos > 0x1ff) ypos -=0x400;

		draw_sprite_new_zoomed(wide, high, xpos, ypos, palt, boff * 2, flip, xzoom, xgrow, yzoom, ygrow, prio);

		source += 5;
	}
}

static void copy_sprite_priority(INT32 prio)
{
	UINT16 *dest = pTransDraw;
	UINT16 *src = pTempScreen;
	UINT8 *pri = SpritePrio;
	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++)
	{
		if (pri[i] == prio) {
			dest[i] = src[i];
		}
	}
}

static void draw_text()
{
	UINT16 *vram = (UINT16*)PGMTxtRAM;

	INT32 scrollx = ((INT16)BURN_ENDIAN_SWAP_INT16(PGMVidReg[0x6000 / 2])) & 0x1ff;
	INT32 scrolly = ((INT16)BURN_ENDIAN_SWAP_INT16(PGMVidReg[0x5000 / 2])) & 0x0ff;

	for (INT32 offs = 0; offs < 64 * 32; offs++)
	{
		INT32 code = BURN_ENDIAN_SWAP_INT16(vram[offs * 2]);
		if (texttrans[code] == 0) continue; // transparent

		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 6) << 3;

		sx -= scrollx;
		if (sx < -7) sx += 512;
		sy -= scrolly;
		if (sy < -7) sy += 256;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 attr  = BURN_ENDIAN_SWAP_INT16(vram[offs * 2 + 1]);
		INT32 color = ((attr & 0x3e) >> 1) | 0x80;
		INT32 flipx =  (attr & 0x40);
		INT32 flipy =  (attr & 0x80);

		if (sx < 0 || sy < 0 || sx >= nScreenWidth - 8 || sy >= nScreenHeight - 8)
		{
			if (texttrans[code] & 2) { // opaque
				if (flipy) {
					if (flipx) {
						Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					} else {
						Render8x8Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					}
				} else {
					if (flipx) {
						Render8x8Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					} else {
						Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					}
				}
			} else {
				if (flipy) {
					if (flipx) {
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					} else {
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					}
				} else {
					if (flipx) {
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					} else {
						Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					}
				}
			}
		}
		else
		{
			if (texttrans[code] & 2) { // opaque
				if (flipy) {
					if (flipx) {
						Render8x8Tile_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					} else {
						Render8x8Tile_FlipY(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					}
				} else {
					if (flipx) {
						Render8x8Tile_FlipX(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					} else {
						Render8x8Tile(pTransDraw, code, sx, sy, color, 4, 0, PGMTileROM);
					}
				}
			} else {
				if (flipy) {
					if (flipx) {
						Render8x8Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					} else {
						Render8x8Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					}
				} else {
					if (flipx) {
						Render8x8Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					} else {
						Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, 15, 0, PGMTileROM);
					}
				}
			}
		}
	}
}

static void draw_background()
{
	UINT16 *vram = (UINT16*)PGMBgRAM;
	UINT16 *dst   = pTransDraw;

	UINT16 *rowscroll = PGMRowRAM;
	INT32 yscroll = (INT16)BURN_ENDIAN_SWAP_INT16(PGMVidReg[0x2000 / 2]);
	INT32 xscroll = (INT16)BURN_ENDIAN_SWAP_INT16(PGMVidReg[0x3000 / 2]);

	// check to see if we need to do line scroll
	INT32 t = 0;
	{
		UINT16 *rs = rowscroll;
		for (INT32 i = 0; i < 224; i++) {
			if (BURN_ENDIAN_SWAP_INT16(rs[0]) != BURN_ENDIAN_SWAP_INT16(rs[i])) {
				t = 1;
				break;
			}
		}
	}

	// no line scroll (fast)
	if (t == 0)
	{
		yscroll &= 0x1ff;
		xscroll &= 0x7ff;

		for (INT32 offs = 0; offs < 64 * 16; offs++)
		{
			INT32 sx = (offs & 0x3f) << 5;
			INT32 sy = (offs >> 6) << 5;

			sx -= xscroll;
			if (sx < -31) sx += 2048;
			sy -= yscroll;
			if (sy < -31) sy += 512;

			if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

			INT32 code = BURN_ENDIAN_SWAP_INT16(vram[offs * 2]);
			if (code >= nTileMask) continue;
			if (tiletrans[code] == 0) continue; // transparent
			INT32 color = ((BURN_ENDIAN_SWAP_INT16(vram[offs*2+1]) & 0x3e) >> 1) | 0x20;
			INT32 flipy = BURN_ENDIAN_SWAP_INT16(vram[offs*2+1]) & 0x80;
			INT32 flipx = BURN_ENDIAN_SWAP_INT16(vram[offs*2+1]) & 0x40;

			if (sx < 0 || sy < 0 || sx >= nScreenWidth - 32 || sy >= nScreenHeight - 32)
			{
				if (tiletrans[code] & 2) { // opaque
					if (flipy) {
						if (flipx) {
							Render32x32Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						}
					} else {
						if (flipx) {
							Render32x32Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_Clip(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						}
					}
				} else {
					if (flipy) {
						if (flipx) {
							Render32x32Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						}
					} else {
						if (flipx) {
							Render32x32Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						}
					}
				}
			}
			else
			{
				if (tiletrans[code] & 2) { // opaque
					if (flipy) {
						if (flipx) {
							Render32x32Tile_FlipXY(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_FlipY(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						}
					} else {
						if (flipx) {
							Render32x32Tile_FlipX(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						} else {
							Render32x32Tile(pTransDraw, code, sx, sy, color, 5, 0, PGMTileROMExp);
						}
					}
				} else {
					if (flipy) {
						if (flipx) {
							Render32x32Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						}
					} else {
						if (flipx) {
							Render32x32Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						} else {
							Render32x32Tile_Mask(pTransDraw, code, sx, sy, color, 5, 0x1f, 0, PGMTileROMExp);
						}
					}
				}
			}
		}

		return;
	}

	// do line scroll (slow)
	for (INT32 y = 0; y < 224; y++, dst += nScreenWidth)
	{
		INT32 scrollx = (xscroll + BURN_ENDIAN_SWAP_INT16(rowscroll[y])) & 0x7ff;
		INT32 scrolly = (yscroll + y) & 0x7ff;

		for (INT32 x = 0; x < 480; x+=32)
		{
			INT32 sx = x - (scrollx & 0x1f);
			if (sx >= nScreenWidth) break;

			INT32 offs = ((scrolly & 0x1e0) << 2) | (((scrollx + x) & 0x7e0) >> 4);

			INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[offs]);
			if (code >= nTileMask) continue;
			if (tiletrans[code] == 0) continue;

			INT32 attr  = BURN_ENDIAN_SWAP_INT16(vram[offs + 1]);
			INT32 color = ((attr & 0x3e) << 4) | 0x400;
			INT32 flipx = ((attr & 0x40) >> 6) * 0x1f;
			INT32 flipy = ((attr & 0x80) >> 7) * 0x1f;

			UINT8 *src = PGMTileROMExp + (code * 1024) + (((scrolly ^ flipy) & 0x1f) << 5);

			if (sx >= 0 && sx <= 415) {
				for (INT32 xx = 0; xx < 32; xx++, sx++) {
					INT32 pxl = src[xx^flipx];
	
					if (pxl != 0x1f) {
						dst[sx] = pxl | color;
					}
				}
			} else {
				for (INT32 xx = 0; xx < 32; xx++, sx++) {
					if (sx < 0) continue;
					if (sx >= nScreenWidth) break;
	
					INT32 pxl = src[xx^flipx];
	
					if (pxl != 0x1f) {
						dst[sx] = pxl | color;
					}
				}
			}
		}
	}
}

INT32 pgmDraw()
{
	if (nPgmPalRecalc) {
		for (INT32 i = 0; i < 0x1200 / 2; i++) {
			RamCurPal[i] = CalcCol(PGMPalRAM[i]);
		}
		nPgmPalRecalc = 0;
	}

	{
		// black / magenta
		RamCurPal[0x1200/2] = (nBurnLayer & 1) ? RamCurPal[0x3ff] : BurnHighCol(0xff, 0, 0xff, 0);
	}

	// Fill in background color (0x1200/2)
	// also, clear buffers
	{
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
			pTransDraw[i] = 0x900;
			pTempScreen[i] = 0;
			SpritePrio[i] = 0xff;
		}
	}

	pgm_drawsprites();
	if (nSpriteEnable & 1) copy_sprite_priority(1);
	if (nBurnLayer & 1) draw_background();
	if (nSpriteEnable & 2) copy_sprite_priority(0);
	if (nBurnLayer & 2) draw_text();

	BurnTransferCopy(RamCurPal);

	return 0;
}

void pgmInitDraw() // preprocess some things...
{
	GenericTilesInit();

	pTempDraw = (UINT16*)BurnMalloc(0x400 * 0x200 * sizeof(INT16));
	SpritePrio = (UINT8*)BurnMalloc(nScreenWidth * nScreenHeight);
	pTempScreen = (UINT16*)BurnMalloc(nScreenWidth * nScreenHeight * sizeof(INT16));

	// Find transparent tiles so we can skip them
	{
		nTileMask = ((nPGMTileROMLen / 5) * 8) / 0x400; // also used to set max. tile

		// background tiles
		tiletrans = (UINT8*)BurnMalloc(nTileMask);
		memset (tiletrans, 0, nTileMask);
	
		for (INT32 i = 0; i < nTileMask << 10; i += 0x400)
		{
			INT32 k = 0x1f;
			for (INT32 j = 0; j < 0x400; j++)
			{
				if (PGMTileROMExp[i+j] != 0x1f) {
					tiletrans[i/0x400] = 1;
				}
				k &= (PGMTileROMExp[i+j] ^ 0x1f);
			}
			if (k) tiletrans[i/0x400] |= 2;
		}

		// character tiles
		texttrans = (UINT8*)BurnMalloc(0x10000);
		memset (texttrans, 0, 0x10000);

		for (INT32 i = 0; i < 0x400000; i += 0x40)
		{
			INT32 k = 0xf;
			for (INT32 j = 0; j < 0x40; j++)
			{
				if (PGMTileROM[i+j] != 0xf) {
					texttrans[i/0x40] = 1;
				}
				k &= (PGMTileROM[i+j] ^ 0xf);
			}
			if (k) texttrans[i/0x40] |= 2;
		}
	}

	// set up table to count bits in sprite mask data
	// gives a good speedup in sprite drawing. ^^
	{
		memset (sprmsktab, 0, 0x100);
		for (INT32 i = 0; i < 0x100; i++) {
			for (INT32 j = 0; j < 8; j++) {
				if (i & (1 << j)) {
					sprmsktab[i]++;
				}
			}
		}
	}
}

void pgmExitDraw()
{
	nTileMask = 0;
	
	BurnFree (pTempDraw);
	BurnFree (tiletrans);
	BurnFree (texttrans);
	BurnFree (pTempScreen);
	BurnFree (SpritePrio);

	GenericTilesExit();
}
