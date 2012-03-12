// Video rendering module for Psikyo SH2 games
// Lots of code here and there ripped directly from MAME
// Thanks to David Haywood for the initial MAME driver
// as well as some other valuable pointers.

#include "tiles_generic.h" // nScreenWidth & nScreenHeight
#include "psikyosh_render.h" // contains loads of macros

UINT8 *pPsikyoshTiles;
UINT32  *pPsikyoshSpriteBuffer;
UINT32  *pPsikyoshBgRAM;
UINT32  *pPsikyoshVidRegs;
UINT32  *pPsikyoshPalRAM;
UINT32  *pPsikyoshZoomRAM;

static UINT8 *DrvTransTab;
static UINT8 alphatable[0x100];

static UINT16 *DrvPriBmp;
static UINT8 *DrvZoomBmp;
static INT32 nDrvZoomPrev = -1;
static UINT32  *DrvTmpDraw;
static UINT32  *DrvTmpDraw_ptr;

static INT32 nGraphicsMin0;  // minimum tile number 4bpp
static INT32 nGraphicsMin1;  // for 8bpp
static INT32 nGraphicsSize;  // normal
static INT32 nGraphicsSize0; // for 4bpp
static INT32 nGraphicsSize1; // for 8bpp

//--------------------------------------------------------------------------------

static inline UINT32 alpha_blend(UINT32 d, UINT32 s, UINT32 p)
{
	if (p == 0) return d;

	INT32 a = 256 - p;

	return (((((s & 0xff00ff) * p) + ((d & 0xff00ff) * a)) & 0xff00ff00) |
		((((s & 0x00ff00) * p) + ((d & 0x00ff00) * a)) & 0x00ff0000)) >> 8;
}

//--------------------------------------------------------------------------------

static void draw_blendy_tile(INT32 gfx, INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 fx, INT32 fy, INT32 alpha, INT32 z)
{
	color <<= 4;
	UINT32 *pal = pBurnDrvPalette + color;

	if (gfx == 0) {
		code &= 0x7ffff;
		code -= nGraphicsMin0;
		if (code < 0 || code > nGraphicsSize0) return;

		if (DrvTransTab[code >> 3] & (1 << (code & 7))) return;

		UINT8 *src = pPsikyoshTiles + (code << 7);
	
		INT32 inc = 8;
		if (fy) {
			inc = -8;
			src += 0x78;
		}

		if (sx >= 0 && sx < (nScreenWidth-15) && sy >= 0 && sy <= (nScreenHeight-15)) {
			if (z > 0) {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_PRIO_FLIPX()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_PRIO_FLIPX()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_PRIO_FLIPX()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_PRIO()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_PRIO()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_PRIO()
					}
				}
			} else {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_FLIPX()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_FLIPX()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_FLIPX()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA()
					} else {
						PUTPIXEL_4BPP_ALPHATAB()
					}
				}
			}
		} else {
			if (z > 0) {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_PRIO_FLIPX_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_PRIO_FLIPX_CLIP()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_PRIO_FLIPX_CLIP()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_PRIO_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_PRIO_CLIP()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_PRIO_CLIP()
					}
				}
			} else {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_FLIPX_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_FLIPX_CLIP()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_FLIPX_CLIP()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_4BPP_NORMAL_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_4BPP_ALPHA_CLIP()
					} else {
						PUTPIXEL_4BPP_ALPHATAB_CLIP()
					}
				}
			}
		}
	} else {
		code &= 0x3ffff;
		code -= nGraphicsMin1;
		if (code < 0 || code > nGraphicsSize0) return;

		if (DrvTransTab[(code >> 3) + 0x10000] & (1 << (code & 7))) return;

		UINT8 *src = pPsikyoshTiles + (code << 8);

		INT32 inc = 16;
		if (fy) {
			inc = -16;
			src += 0xf0;
		}

		if (sx >= 0 && sx < (nScreenWidth-15) && sy >= 0 && sy < (nScreenHeight-15)) {
			if (z > 0) {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_PRIO_FLIPX()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_PRIO_FLIPX()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_PRIO_FLIPX()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_PRIO()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_PRIO()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_PRIO()
					}
				}
			} else {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_FLIPX()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_FLIPX()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_FLIPX()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA()
					} else {
						PUTPIXEL_8BPP_ALPHATAB()
					}
				}
			}
		} else {
			if (z > 0) {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_PRIO_FLIPX_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_PRIO_FLIPX_CLIP()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_PRIO_FLIPX_CLIP()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_PRIO_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_PRIO_CLIP()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_PRIO_CLIP()
					}
				}
			} else {
				if (fx) {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_FLIPX_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_FLIPX_CLIP()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_FLIPX_CLIP()
					}
				} else {
					if (alpha == 0xff) {
						PUTPIXEL_8BPP_NORMAL_CLIP()
					} else if (alpha >= 0) {
						PUTPIXEL_8BPP_ALPHA_CLIP()
					} else {
						PUTPIXEL_8BPP_ALPHATAB_CLIP()
					}
				}
			}

		}
	}
}

static void draw_prezoom(INT32 gfx, INT32 code, INT32 high, INT32 wide)
{
	// these probably aren't the safest routines, but they should be pretty fast.

	if (gfx) {
		INT32 tileno = (code & 0x3ffff) - nGraphicsMin1;
		if (tileno < 0 || tileno > nGraphicsSize1) tileno = 0;
		if (nDrvZoomPrev == tileno) return;
		nDrvZoomPrev = tileno;
		UINT32 *gfxptr = (UINT32*)(pPsikyoshTiles + (tileno << 8));

		for (INT32 ytile = 0; ytile < high; ytile++)
		{
			for (INT32 xtile = 0; xtile < wide; xtile++)
			{
				UINT32 *dest = (UINT32*)(DrvZoomBmp + (ytile << 12) + (xtile << 4));

				for (INT32 ypixel = 0; ypixel < 16; ypixel++, gfxptr += 4) {

					dest[0] = gfxptr[0];
					dest[1] = gfxptr[1];
					dest[2] = gfxptr[2];
					dest[3] = gfxptr[3];

					dest += 64;
				}
			}
		}
	} else {
		INT32 tileno = (code & 0x7ffff) - nGraphicsMin0;
		if (tileno < 0 || tileno > nGraphicsSize0) tileno = 0;
		if (nDrvZoomPrev == tileno) return;
		nDrvZoomPrev = tileno;
		UINT8 *gfxptr = pPsikyoshTiles + (tileno << 7);
		for (INT32 ytile = 0; ytile < high; ytile++)
		{
			for (INT32 xtile = 0; xtile < wide; xtile++)
			{
				UINT8 *dest = DrvZoomBmp + (ytile << 12) + (xtile << 4);

				for (INT32 ypixel = 0; ypixel < 16; ypixel++, gfxptr += 8)
				{
					for (INT32 xpixel = 0; xpixel < 16; xpixel+=2)
					{
						INT32 c = gfxptr[xpixel>>1];
						dest[xpixel    ] = c >> 4;
						dest[xpixel + 1] = c & 0x0f;
					}

					dest += 256;
				}
			}
		}
	}
}

static void psikyosh_drawgfxzoom(INT32 gfx, UINT32 code, INT32 color, INT32 flipx, INT32 flipy, INT32 offsx, 
				 INT32 offsy, INT32 alpha, INT32 zoomx, INT32 zoomy, INT32 wide, INT32 high, INT32 z)
{
	if (~nBurnLayer & 8) return;
	if (!zoomx || !zoomy) return;

	if (zoomx == 0x400 && zoomy == 0x400)
	{
		INT32 xstart, ystart, xend, yend, xinc, yinc, code_offset = 0;

		if (flipx)	{ xstart = wide-1; xend = -1;   xinc = -1; }
		else		{ xstart = 0;      xend = wide; xinc = +1; }

		if (flipy)	{ ystart = high-1; yend = -1;   yinc = -1; }
		else		{ ystart = 0;      yend = high; yinc = +1; }

		for (INT32 ytile = ystart; ytile != yend; ytile += yinc )
		{
			for (INT32 xtile = xstart; xtile != xend; xtile += xinc )
			{
				INT32 sx = offsx + (xtile << 4);
				INT32 sy = offsy + (ytile << 4);

				draw_blendy_tile(gfx, code + code_offset++, color, sx, sy, flipx, flipy, alpha, z);
			}
		}
	}
	else
	{
		draw_prezoom(gfx, code, high, wide);

		{
			UINT32 *pal = pBurnDrvPalette + (color << 4);

			INT32 sprite_screen_height = ((high << 24) / zoomy + 0x200) >> 10;
			INT32 sprite_screen_width  = ((wide << 24) / zoomx + 0x200) >> 10;

			if (sprite_screen_width && sprite_screen_height)
			{
				INT32 sx = offsx;
				INT32 sy = offsy;
				INT32 ex = sx + sprite_screen_width;
				INT32 ey = sy + sprite_screen_height;

				INT32 x_index_base;
				INT32 y_index;

				INT32 dx, dy;

				if (flipx) { x_index_base = (sprite_screen_width-1)*zoomx; dx = -zoomx; }
				else	   { x_index_base = 0; dx = zoomx; }

				if (flipy) { y_index = (sprite_screen_height-1)*zoomy; dy = -zoomy; }
				else	   { y_index = 0; dy = zoomy; }

				{
					if (sx < 0) {
						INT32 pixels = 0-sx;
						sx += pixels;
						x_index_base += pixels*dx;
					}
					if (sy < 0 ) {
						INT32 pixels = 0-sy;
						sy += pixels;
						y_index += pixels*dy;
					}
					if (ex > nScreenWidth) {
						INT32 pixels = ex-(nScreenWidth-1)-1;
						ex -= pixels;
					}
					if (ey > nScreenHeight)	{
						INT32 pixels = ey-(nScreenHeight-1)-1;
						ey -= pixels;
					}
				}

				if (ex > sx)
				{
					if (alpha == 0xff) {
						if (z > 0) {
							PUTPIXEL_ZOOM_NORMAL_PRIO()
						} else {
							PUTPIXEL_ZOOM_NORMAL()
						}
					} else if (alpha >= 0) {
						if (z > 0) {
							PUTPIXEL_ZOOM_ALPHA_PRIO()
						} else {
							PUTPIXEL_ZOOM_ALPHA()
						}
					} else {
						if (z > 0) {
							PUTPIXEL_ZOOM_ALPHATAB_PRIO()
						} else {
							PUTPIXEL_ZOOM_ALPHATAB()
						}
					}
				}
			}
		}
	}
}

static void draw_sprites(UINT8 req_pri)
{
	UINT32   *src = pPsikyoshSpriteBuffer;
	UINT16 *list = (UINT16 *)src + 0x3800/2;
	UINT16 listlen = 0x800/2;
	UINT16 listcntr = 0;
	UINT16 *zoom_table = (UINT16 *)pPsikyoshZoomRAM;
	UINT8  *alpha_table = (UINT8 *)pPsikyoshVidRegs;

	while (listcntr < listlen)
	{
		UINT32 xpos, ypos, high, wide, flpx, flpy, zoomx, zoomy, tnum, colr, dpth, pri;
		INT32 alpha;

#ifdef LSB_FIRST
		UINT32 listdat = list[listcntr ^ 1];
#else
		UINT32 listdat = list[listcntr];
#endif
		UINT32 sprnum = (listdat & 0x03ff) << 2;

		pri = (src[sprnum+1] & 0x00003000) >> 12;
		pri = (pPsikyoshVidRegs[2] << (pri << 2)) >> 28;

		if (pri == req_pri)
		{
			ypos  = (src[sprnum+0] & 0x03ff0000) >> 16;
			xpos  = (src[sprnum+0] & 0x000003ff);
			high  =((src[sprnum+1] & 0x0f000000) >> 24) + 1;
			wide  =((src[sprnum+1] & 0x00000f00) >>  8) + 1;
			flpy  = (src[sprnum+1] & 0x80000000) >> 31;
			flpx  = (src[sprnum+1] & 0x00008000) >> 15;
			zoomy = (src[sprnum+1] & 0x00ff0000) >> 16;
			zoomx = (src[sprnum+1] & 0x000000ff);
			tnum  = (src[sprnum+2] & 0x0007ffff);
			dpth  = (src[sprnum+2] & 0x00800000) >> 23;
			colr  = (src[sprnum+2] & 0xff000000) >> 24;
			alpha = (src[sprnum+2] & 0x00700000) >> 20;

			if (ypos & 0x200) ypos -= 0x400;
			if (xpos & 0x200) xpos -= 0x400;

#ifdef LSB_FIRST
			alpha = alpha_table[alpha ^ 3];
#else
			alpha = alpha_table[alpha];
#endif

			if (alpha & 0x80) {
				alpha = -1;
			} else {
				alpha = alphatable[alpha | 0xc0];
			}

#ifdef LSB_FIRST
			if (zoom_table[zoomy ^ 1] && zoom_table[zoomx ^ 1])
#else
			if (zoom_table[zoomy] && zoom_table[zoomx])
#endif
			{
#ifdef LSB_FIRST
				psikyosh_drawgfxzoom(dpth, tnum, colr, flpx, flpy, xpos, ypos, alpha, 
					(UINT32)zoom_table[zoomx ^ 1],(UINT32)zoom_table[zoomy ^ 1], wide, high, listcntr);
#else
				psikyosh_drawgfxzoom(dpth, tnum, colr, flpx, flpy, xpos, ypos, alpha, 
					(UINT32)zoom_table[zoomx],(UINT32)zoom_table[zoomy], wide, high, listcntr);
#endif
			}
		}

		listcntr++;
		if (listdat & 0x4000) break;
	}
}

static void draw_layer(INT32 layer, INT32 bank, INT32 alpha, INT32 scrollx, INT32 scrolly)
{
	if ((bank < 0x0c) || (bank > 0x1f)) return;

	if (alpha & 0x80) {
		alpha = -1;
	} else {
		alpha = alphatable[alpha | 0xc0];
	}

	INT32 attr = pPsikyoshVidRegs[7] << (layer << 2);
	INT32 gfx  = attr & 0x00004000;
	INT32 size =(attr & 0x00001000) ? 32 : 16;
	INT32 wide = size * 16;

	for (INT32 offs = 0; offs < size * 32; offs++) {
		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		sx = (sx + scrollx) & 0x1ff;
		sy = (sy + scrolly) & (wide-1);
		if (sx >= nScreenWidth)  sx -= 0x200;
		if (sy >= nScreenHeight) sy -= wide;
		if (sx < -15 || sy < -15) continue;

		UINT32 code  = pPsikyoshBgRAM[(bank*0x800)/4 + offs - 0x4000/4];

		draw_blendy_tile(gfx, code & 0x7ffff, (code >> 24), sx, sy, 0, 0, alpha, 0);
	}
}

static void draw_bglayer(INT32 layer)
{
	if (!(nBurnLayer & 1)) return;

	INT32 scrollx, scrolly, bank, alpha;
	INT32 scrollbank = ((pPsikyoshVidRegs[6] << (layer << 3)) >> 24) & 0x7f;
	INT32 offset = (scrollbank == 0x0b) ? 0x200 : 0;

	bank    = (pPsikyoshBgRAM[0x17f0/4 + offset + layer] & 0x000000ff);
	alpha   = (pPsikyoshBgRAM[0x17f0/4 + offset + layer] & 0x0000bf00) >> 8;
	scrollx = (pPsikyoshBgRAM[0x13f0/4 + offset + layer] & 0x000001ff);
	scrolly = (pPsikyoshBgRAM[0x13f0/4 + offset + layer] & 0x03ff0000) >> 16;

	if (scrollbank == 0x0d) scrollx += 0x08;

	draw_layer(layer, bank, alpha, scrollx, scrolly);
}

static void draw_bglayertext(INT32 layer)
{
	if (~nBurnLayer & 2) return;

	INT32 scrollx, scrolly, bank, alpha;
	INT32 scrollbank = ((pPsikyoshVidRegs[6] << (layer << 3)) >> 24) & 0x7f;

	bank    = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 - 0x4000/4] & 0x000000ff);
	alpha   = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 - 0x4000/4] & 0x0000bf00) >> 8;
	scrollx = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4           ] & 0x000001ff);
	scrolly = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4           ] & 0x03ff0000) >> 16;

	draw_layer(layer, bank, alpha, scrollx, scrolly);

	bank    = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 + 0x20/4 - 0x4000/4] & 0x000000ff);
	alpha   = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 + 0x20/4 - 0x4000/4] & 0x0000bf00) >> 8;
	scrollx = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4 + 0x20/4           ] & 0x000001ff);
	scrolly = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4 + 0x20/4           ] & 0x03ff0000) >> 16;

	draw_layer(layer, bank, alpha, scrollx, scrolly);
}

static void draw_bglayerscroll(INT32 layer)
{
	if (!(nBurnLayer & 4)) return;

	INT32 scrollx, bank, alpha;
	INT32 scrollbank = ((pPsikyoshVidRegs[6] << (layer << 3)) >> 24) & 0x7f;

	bank    = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 - 0x4000/4] & 0x000000ff);
	alpha   = (pPsikyoshBgRAM[(scrollbank*0x800)/4 + 0x0400/4 - 0x4000/4] & 0x0000bf00) >> 8;
	scrollx = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4           ] & 0x000001ff);
//	scrolly = (pPsikyoshBgRAM[(scrollbank*0x800)/4 - 0x4000/4           ] & 0x03ff0000) >> 16;

	draw_layer(layer, bank, alpha, scrollx, 0);
}

static void draw_background(UINT8 req_pri)
{
	for (INT32 i = 0; i < 3; i++)
	{
		if (!((pPsikyoshVidRegs[7] << (i << 2)) & 0x8000))
			continue;

		INT32 bgtype = ((pPsikyoshVidRegs[6] << (i << 3)) >> 24) & 0x7f;

		switch (bgtype)
		{
			case 0x0a: // Normal
				if((pPsikyoshBgRAM[0x17f0/4 + (i*0x04)/4] >> 24) == req_pri)
					draw_bglayer(i);
				break;

			case 0x0b: // Alt / Normal
				if((pPsikyoshBgRAM[0x1ff0/4 + (i*0x04)/4] >> 24) == req_pri)
					draw_bglayer(i);
				break;

			case 0x0c: // Using normal for now
			case 0x0d: // Using normal for now
				if((pPsikyoshBgRAM[(bgtype*0x800)/4 + 0x400/4 - 0x4000/4] >> 24) == req_pri)
					draw_bglayertext(i);
				break;

			case 0x0e:
			case 0x10: case 0x11: case 0x12: case 0x13:
			case 0x14: case 0x15: case 0x16: case 0x17:
			case 0x18: case 0x19: case 0x1a: case 0x1b:
			case 0x1c: case 0x1d: case 0x1e: case 0x1f:
				if((pPsikyoshBgRAM[(bgtype*0x800)/4 + 0x400/4 - 0x4000/4] >> 24) == req_pri)
					draw_bglayerscroll(i);
				break;
		}
	}
}

static void prelineblend()
{
	UINT32 *linefill = pPsikyoshBgRAM;
	UINT32 *destline = DrvTmpDraw;

	for (INT32 y = 0; y < nScreenHeight; y++, destline+=nScreenWidth) {
		if (linefill[y] & 0xff) {
			for (INT32 x = 0; x < nScreenWidth; x++) {
				destline[x] = linefill[y] >> 8;
			}
		}
	}
}

static void postlineblend()
{
	UINT32 *lineblend = pPsikyoshBgRAM + 0x0400/4;
	UINT32 *destline = DrvTmpDraw;

	for (INT32 y = 0; y < nScreenHeight; y++, destline+=nScreenWidth) {
		if (lineblend[y] & 0x80) {
			for (INT32 x = 0; x < nScreenWidth; x++) {
				destline[x] = lineblend[y] >> 8;
			}
		}
		else if (lineblend[y] & 0x7f) {
			for (INT32 x = 0; x < nScreenWidth; x++) {
				destline[x] = alpha_blend(destline[x], lineblend[y] >> 8, (lineblend[y] & 0x7f) << 1);
			}
		}
	}
}

INT32 PsikyoshDraw()
{
	{
		for (INT32 i = 0; i < 0x5000 / 4; i++) {
			pBurnDrvPalette[i] = pPsikyoshPalRAM[i] >> 8;
		}
	}

	if (nBurnBpp == 4) {
		DrvTmpDraw = (UINT32*)pBurnDraw;
	} else {
		DrvTmpDraw = DrvTmpDraw_ptr;
	}

	memset (DrvTmpDraw, 0, nScreenWidth * nScreenHeight * sizeof(UINT32));
	memset (DrvPriBmp, 0, nScreenWidth * nScreenHeight * sizeof(INT16));

	UINT32 *psikyosh_vidregs = pPsikyoshVidRegs;

	prelineblend();

	for (UINT32 i = 0; i < 8; i++) {
		draw_sprites(i);
		draw_background(i);
		if ((psikyosh_vidregs[2] & 0x0f) == i) postlineblend();
	}

	if (nBurnBpp < 4) {
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
			INT32 d = DrvTmpDraw[i];
			PutPix(pBurnDraw + i * nBurnBpp, BurnHighCol(d>>16, d>>8, d, 0));
		}
	}

	return 0;
}

static void fill_alphatable()
{
	for (INT32 i = 0; i < 0xc0; i++)
		alphatable[i] = 0xff;

	for (INT32 i = 0; i < 0x40; i++) {
		alphatable[i | 0xc0] = ((0x3f - i) * 0xff) / 0x3f;
	}
}

static void calculate_transtab()
{
	DrvTransTab = (UINT8*)BurnMalloc(0x18000);

	memset (DrvTransTab, 0xff, 0x18000);

	// first calculate all 4bpp tiles
	for (INT32 i = 0; i < nGraphicsSize; i+= 0x80) {
		for (INT32 j = 0; j < 0x80; j++) {
			if (pPsikyoshTiles[i + j]) {
				DrvTransTab[(i>>10) + 0x00000] &= ~(1 << ((i >> 7) & 7));
				break;
			}
		}
	}

	// next, calculate all 8bpp tiles
	for (INT32 i = 0; i < nGraphicsSize; i+= 0x100) {
		for (INT32 j = 0; j < 0x100; j++) {
			if (pPsikyoshTiles[i + j]) {
				DrvTransTab[(i>>11) + 0x10000] &= ~(1 << ((i >> 8) & 7));
				break;
			}
		}
	}
}

void PsikyoshVideoInit(INT32 gfx_max, INT32 gfx_min)
{
	DrvZoomBmp	= (UINT8 *)BurnMalloc(16 * 16 * 16 * 16);
	DrvPriBmp	= (UINT16*)BurnMalloc(320 * 240 * sizeof(INT16));
	DrvTmpDraw_ptr	= (UINT32  *)BurnMalloc(320 * 240 * sizeof(UINT32));

	if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
		BurnDrvGetVisibleSize(&nScreenHeight, &nScreenWidth);
	} else {
		BurnDrvGetVisibleSize(&nScreenWidth, &nScreenHeight);
	}

	nGraphicsSize  = gfx_max - gfx_min;
	nGraphicsMin0  = (gfx_min / 128);
	nGraphicsMin1  = (gfx_min / 256);
	nGraphicsSize0 = (nGraphicsSize / 128) - 1;
	nGraphicsSize1 = (nGraphicsSize / 256) - 1;

	calculate_transtab();
	fill_alphatable();
}

void PsikyoshVideoExit()
{
	BurnFree (DrvZoomBmp);
	BurnFree (DrvPriBmp);
	BurnFree (DrvTmpDraw_ptr);
	DrvTmpDraw = NULL;
	BurnFree (DrvTransTab);
	
	nDrvZoomPrev		= -1;
	pPsikyoshTiles		= NULL;
	pPsikyoshSpriteBuffer	= NULL;
	pPsikyoshBgRAM		= NULL;
	pPsikyoshVidRegs	= NULL;
	pPsikyoshPalRAM		= NULL;
	pPsikyoshZoomRAM	= NULL;
	pBurnDrvPalette		= NULL;

	nScreenWidth = nScreenHeight = 0;
}
