#include "tiles_generic.h"
#include "taito_ic.h"
#include "taito.h"

UINT8 *TC0180VCURAM;
UINT8 *TC0180VCUScrollRAM;
UINT8 *TC0180VCUFbRAM; // framebuffer ram

static UINT16 *TC0180VCUFramebuffer[2];

static INT32 TC0180VCU_y_offset;
static INT32 TC0180VCU_x_offset;

static UINT8 TC0180VCUControl[0x10];

static UINT8 *tiledata[2];
static UINT32 tilemask[2];
static UINT8 *transtiletab[2];

static UINT8 *dummy_tile = NULL;

static INT32 *TC0180VCU_scrollx[2];
static INT32 *TC0180VCU_scrolly[2];

static INT32 flipscreen;
static INT32 framebuffer_page;

void TC0180VCUFramebufferWrite(INT32 offset)
{
	offset &= 0x3fffe;
	INT32 data = *((UINT16*)(TC0180VCUFbRAM + offset));

	INT32 fb = (offset >> 17) & 1;

	offset &= 0x1fffe;

	TC0180VCUFramebuffer[fb][offset + 0] = data >> 8;
	TC0180VCUFramebuffer[fb][offset + 1] = data & 0xff;
}

UINT16 TC0180VCUFramebufferRead(INT32 offset)
{
	offset &= 0x3fffe;

	INT32 fb = (offset >> 17) & 1;

	offset &= 0x1fffe;

	return (TC0180VCUFramebuffer[fb][offset + 0] << 8) | (TC0180VCUFramebuffer[fb][offset + 1] & 0xff);
}

UINT8 TC0180VCUReadControl()
{
	return TC0180VCUControl[7];
}

UINT8 TC0180VCUReadRegs(INT32 offset)
{
	offset >>= 1;
	offset &= 0x0f;

	return TC0180VCUControl[offset];
}

void TC0180VCUWriteRegs(INT32 offset, INT32 data)
{
	offset >>= 1;
	offset &= 0x0f;

	TC0180VCUControl[offset] = data;

	if (offset == 7) {
		if (data & 0x80) {
			framebuffer_page = (data & 0x40) ? 0 : 1;
		}
	}
}

void TC0180VCUReset()
{
	for (INT32 i = 0; i < 2; i++) {

		memset (TC0180VCUFramebuffer[i], 0, 512 * 256 * sizeof(UINT16));
		memset (TC0180VCU_scrollx[i], 0, 256 * sizeof(INT32));
		memset (TC0180VCU_scrolly[i], 0, 256 * sizeof(INT32));
	}

	memset (TC0180VCUControl, 	0, 16);
	memset (TC0180VCURAM,		0, 0x010000);
	memset (TC0180VCUScrollRAM,	0, 0x000800);
	memset (TC0180VCUFbRAM,		0, 0x040000);

	flipscreen = 0;
	framebuffer_page = 0;
}

static void create_transtile_table(INT32 tile)
{
	INT32 size = (tile) ? (16 * 16) : (8 * 8);

	if (tilemask[tile]) {
		INT32 len = (tilemask[tile] + 1);

		transtiletab[tile] = (UINT8*)BurnMalloc(len);

		memset (transtiletab[tile], 1, len);

		for (INT32 i = 0; i < len * size; i++)
		{
			if (tiledata[tile][i]) {
				transtiletab[tile][i / size] = 0;
				i|=(size-1);
			}
		}
	}
}

void TC0180VCUInit(UINT8 *gfx0, INT32 mask0, UINT8 *gfx1, INT32 mask1, INT32 global_x, INT32 global_y)
{
	TaitoIC_TC0180VCUInUse = 1;

	for (INT32 i = 0; i < 2; i++)
	{
		TC0180VCUFramebuffer[i] = (UINT16*)BurnMalloc(512 * 256 * sizeof(UINT16));
		TC0180VCU_scrollx[i] = (INT32*)BurnMalloc(257 * sizeof(INT32));
		TC0180VCU_scrolly[i] = (INT32*)BurnMalloc(257 * sizeof(INT32));
	}

	TC0180VCURAM		= (UINT8*)BurnMalloc(0x010000);
	TC0180VCUScrollRAM	= (UINT8*)BurnMalloc(0x000800);
	TC0180VCUFbRAM		= (UINT8*)BurnMalloc(0x040000);

	tilemask[0] = mask0;
	tilemask[1] = mask1;
	tiledata[0] = gfx0;
	tiledata[1] = gfx1;

	if (mask0) create_transtile_table(0);
	if (mask1) create_transtile_table(1);

	if (mask0 == 0) {
		dummy_tile = (UINT8*)BurnMalloc(0x100);
		transtiletab[1] = (UINT8*)BurnMalloc(1);
		tiledata[1] = dummy_tile;
	}

	TC0180VCU_y_offset = global_y;
	TC0180VCU_x_offset = global_x;

	TC0180VCUReset();
}

void TC0180VCUExit()
{
	for (INT32 i = 0; i < 2; i++)
	{
		BurnFree (TC0180VCU_scrollx[i]);
		BurnFree (TC0180VCU_scrolly[i]);

		tilemask[i] = ~0;
		tiledata[i] = NULL;

		BurnFree (TC0180VCUFramebuffer[i]);
		BurnFree (transtiletab[i]);
	}

	BurnFree (dummy_tile);
	BurnFree (TC0180VCURAM);
	BurnFree (TC0180VCUScrollRAM);
	BurnFree (TC0180VCUFbRAM);

	TC0180VCU_y_offset = 0;
	TC0180VCU_x_offset = 0;
}

static void update_scroll(INT32 plane)
{
	flipscreen = TC0180VCUReadControl() & 0x10;

	UINT16 *scrollram = (UINT16*)TC0180VCUScrollRAM;

	INT32 lines_per_block = 256 - TC0180VCUControl[2 + plane];
	INT32 number_of_blocks = 256 / lines_per_block;

	for (INT32 i = 0; i < number_of_blocks; i++)
	{
		INT32 scrollx = scrollram[plane * 0x200 + i * 2 * lines_per_block + 0];
		INT32 scrolly = scrollram[plane * 0x200 + i * 2 * lines_per_block + 1];

		INT32 min_y = (i + 0) * lines_per_block - 0;
		INT32 max_y = (i + 1) * lines_per_block - 1;

		if (min_y <= max_y)
		{
			for (INT32 y = min_y; y <= max_y; y++) {
				TC0180VCU_scrollx[plane][y] = -(scrollx & 0x3ff);
				TC0180VCU_scrolly[plane][y] = -(scrolly & 0x3ff);
			}
		}
	}
}

void TC0180VCUDrawLayer(INT32 colorbase, INT32 ctrl_offset, INT32 transparent) // 0, -1
{
	update_scroll(ctrl_offset);

	UINT16 *ram = (UINT16*)TC0180VCURAM;

	INT32 bank0 = (TC0180VCUControl[ctrl_offset] << 12) & 0xf000; // tile bank
	INT32 bank1 = (TC0180VCUControl[ctrl_offset] <<  8) & 0xf000; // color bank

	INT32 *scroll_x = TC0180VCU_scrollx[ctrl_offset];
	INT32 *scroll_y = TC0180VCU_scrolly[ctrl_offset];

	INT32 lines = TC0180VCUControl[2 + ctrl_offset];

	if (lines)
	{
		INT32 screen_width = nScreenWidth - 1;
		INT32 screen_height = nScreenHeight - 1;

		UINT16 *dest;

		for (INT32 sy = 0; sy < nScreenHeight; sy++)
		{
			if (flipscreen) {
				dest = pTransDraw + (screen_height - sy) * nScreenWidth;
			} else {
				dest = pTransDraw + sy * nScreenWidth;
			}

			INT32 scly = (sy + scroll_y[(sy + TC0180VCU_y_offset) & 0xff] + TC0180VCU_y_offset) & 0x3ff;

			INT32 scly_off = (scly >> 4) << 6;
			INT32 scly_ts  = (scly & 0x0f) << 4;

			INT32 sclx_base = scroll_x[(sy + TC0180VCU_y_offset) & 0xff] + TC0180VCU_x_offset;

			for (INT32 sx = 0; sx < nScreenWidth + 16; sx+=16)
			{
				INT32 sclx = (sx + sclx_base) & 0x3ff;

				INT32 offs = scly_off | (sclx >> 4);

				INT32 attr = ram[offs + bank1];
				INT32 code = ram[offs + bank0];
				INT32 color = (attr & 0x003f) + colorbase;
				code &= tilemask[1];

				if (!transparent) {
					if (transtiletab[1][code]) continue;
				}

				{
					INT32 sx4 = sx - (sclx & 0x0f);

					color <<= 4;
					UINT8 *src = tiledata[1] + code * 256;

					if (attr & 0x80) {			// flipy
						src += (scly_ts ^ 0xf0);
					} else {
						src += (scly_ts);
					}

					INT32 flipx = ((attr & 0x40) >> 6) * 0x0f;

					if (flipscreen) {
						if (!transparent) { // transparency
							for (INT32 sxx = 0; sxx < 16; sxx++, sx4++) {
								if (sx4 >= nScreenWidth || sx4 < 0) continue;
			
								INT32 pxl = src[sxx ^ flipx];
		
								if (pxl != transparent) {
									dest[(screen_width - sx4)] = pxl | color;
								}
							}
						} else {
							for (INT32 sxx = 0; sxx < 16; sxx++, sx4++) {
								if (sx4 >= nScreenWidth || sx4 < 0) continue;

								dest[(screen_width - sx4)] = src[sxx ^ flipx] | color;
							}
						}
					} else {
						if (!transparent) { // transparency
							for (INT32 sxx = 0; sxx < 16; sxx++, sx4++) {
								if (sx4 >= nScreenWidth || sx4 < 0) continue;
			
								INT32 pxl = src[sxx ^ flipx];
		
								if (pxl != transparent) {
									dest[sx4] = pxl | color;
								}
							}
						} else {
							for (INT32 sxx = 0; sxx < 16; sxx++, sx4++) {
								if (sx4 >= nScreenWidth || sx4 < 0) continue;

								dest[sx4] = src[sxx ^ flipx] | color;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		for (INT32 offs = 0; offs < 64 * 64; offs++)
		{
			INT32 sx = (offs & 0x3f) << 4;
			INT32 sy = (offs >> 6) << 4;
	
			sy -= scroll_y[(sy + TC0180VCU_y_offset) & 0xff];
			if (sy >= 0x400-15) sy -= 0x400;

			sx -= scroll_x[(sy + TC0180VCU_y_offset) & 0xff];
			if (sx >= 0x400-15) sx -= 0x400;

			INT32 attr  = ram[offs + bank1];
			INT32 code  = ram[offs + bank0];
	
			INT32 color = (attr & 0x003f) + colorbase;
			INT32 flipx = (attr & 0x0040);
			INT32 flipy = (attr & 0x0080);
	
			code &= tilemask[1];

			if (flipscreen) {
				sx = (nScreenWidth - 16) - sx;
				sy = 240 - sy;
				flipx ^= 0x40;
				flipy ^= 0x80;
			}

			sx -= TC0180VCU_x_offset;
			sy -= TC0180VCU_y_offset;

			if (!transparent) {
				if (transtiletab[1][code]) continue;

				if (sy >= 0 && sy < (nScreenHeight - 15) && sx >= 0 && sx < (nScreenWidth - 15)) {
					if (flipy) {
						if (flipx) {
							Render16x16Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						} else {
							Render16x16Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						}
					} else {
						if (flipx) {
							Render16x16Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						} else {
							Render16x16Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						}
					}
				} else {
					if (flipy) {
						if (flipx) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						} else {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						}
					} else {
						if (flipx) {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, tiledata[1]);
						}
					}
				}
			} else {
				if (sy >= 0 && sy < (nScreenHeight - 15) && sx >= 0 && sx < (nScreenWidth - 15)) {
					if (flipy) {
						if (flipx) {
							Render16x16Tile_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						} else {
							Render16x16Tile_FlipY(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						}
					} else {
						if (flipx) {
							Render16x16Tile_FlipX(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						} else {
							Render16x16Tile(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						}
					}
				} else {
					if (flipy) {
						if (flipx) {
							Render16x16Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						} else {
							Render16x16Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						}
					} else {
						if (flipx) {
							Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						} else {
							Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, tiledata[1]);
						}
					}
				}
			}
		}
	}
}

void TC0180VCUDrawCharLayer(INT32 colorbase)
{
	if (tilemask[0] == 0) return;

	UINT16 *ram = (UINT16*)TC0180VCURAM;

	INT32 bank0 = (TC0180VCUControl[6] & 0x0f) << 11; // tile bank

	for (INT32 offs = 0; offs < 64 * 32; offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 6) << 3;

		INT32 code  = ram[offs + bank0];
		INT32 color = (code >> 12) + colorbase;

		code = (code & 0x07ff) | (TC0180VCUControl[4 + ((code & 0x800) >> 11)] << 11);

		code &= tilemask[0];

		if (transtiletab[0][code]) continue;

		if (flipscreen) {
			sx = (nScreenWidth - 8) - sx;
			sy = 248 - sy;

			Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx - TC0180VCU_x_offset, sy - TC0180VCU_y_offset, color, 4, 0, 0, tiledata[0]);
		} else {
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx - TC0180VCU_x_offset, sy - TC0180VCU_y_offset, color, 4, 0, 0, tiledata[0]);
		}
	}
}

void TC0180VCUFramebufferDraw(INT32 priority, INT32 color_base)
{
	priority <<= 4;

	INT32 ctrl = TC0180VCUReadControl();

	if (ctrl & 0x08)
	{
		if (ctrl & 0x10)	// flip screen
		{
			for (INT32 y = 0; y < nScreenHeight; y++)
			{
				UINT16 *src = TC0180VCUFramebuffer[framebuffer_page & 1] + (y + TC0180VCU_y_offset) * 512 + TC0180VCU_x_offset;
				UINT16 *dst = pTransDraw + ((nScreenHeight - 1) - y) * nScreenWidth + (nScreenWidth - 1);

				for (INT32 x = 0; x < nScreenWidth; x++)
				{
					INT32 c = *src++;

					if (c != 0) *dst = color_base + c;
					dst--;
				}
			}
		}
		else
		{
			for (INT32 y = 0; y < nScreenHeight; y++)
			{
				UINT16 *src = TC0180VCUFramebuffer[framebuffer_page & 1] + (y + TC0180VCU_y_offset) * 512 + TC0180VCU_x_offset;
				UINT16 *dst = pTransDraw + y * nScreenWidth;

				for (INT32 x = 0; x < nScreenWidth; x++)
				{
					INT32 c = *src++;

					if (c != 0) *dst = color_base + c;
					dst++;
				}
			}
		}
	}
	else
	{
		if (ctrl & 0x10)   // flip screen
		{
			for (INT32 y = 0; y < nScreenHeight; y++)
			{
				UINT16 *src = TC0180VCUFramebuffer[framebuffer_page & 1] + (y + TC0180VCU_y_offset) * 512 + TC0180VCU_x_offset;
				UINT16 *dst = pTransDraw + ((nScreenHeight - 1) - y) * nScreenWidth + (nScreenWidth - 1);

				for (INT32 x = 0; x < nScreenWidth; x++)
				{
					INT32 c = *src++;

					if (c != 0 && (c & 0x10) == priority)
						*dst = color_base + c;
					dst--;
				}
			}
		}
    		else
		{
			for (INT32 y = 0; y < nScreenHeight; y++)
			{
				UINT16 *src = TC0180VCUFramebuffer[framebuffer_page & 1] + (y + TC0180VCU_y_offset) * 512 + TC0180VCU_x_offset;
				UINT16 *dst = pTransDraw + y * nScreenWidth;

				for (INT32 x = 0; x < nScreenWidth; x++)
				{
					INT32 c = *src++;

					if (c != 0 && (c & 0x10) == priority)
						*dst = color_base + c;
					dst++;
				}
			}
		}
	}
}

void TC0180VCUDrawSprite(UINT16 *dest)
{
	INT32 t_swide = nScreenWidth;  nScreenWidth  = 512; // hack to allow use of generic tile routines
	INT32 t_shigh = nScreenHeight; nScreenHeight = 256;

	INT32 xlatch = 0;
	INT32 ylatch = 0;
	INT32 x_no = 0;
	INT32 y_no = 0;
	INT32 x_num = 0;
	INT32 y_num = 0;
	INT32 big_sprite = 0;
	UINT32 zoomx;
	UINT32 zoomy;
	UINT32 zx;
	UINT32 zy;
	UINT32 zoomxlatch = 0;
	UINT32 zoomylatch = 0;

	UINT16 *ram = (UINT16*)TaitoSpriteRam;

	for (INT32 offs = (0x1980 - 16) / 2; offs >=0; offs -= 8)
	{
		INT32 code  = ram[offs + 0] & tilemask[1];
		INT32 color = ram[offs + 1];
		INT32 x     = ram[offs + 2] & 0x03ff;
		INT32 y     = ram[offs + 3] & 0x03ff;

		INT32 data  = ram[offs + 5];

		INT32 flipx = color & 0x4000;
		INT32 flipy = color & 0x8000;

		if (x >= 0x200) x -= 0x400;
		if (y >= 0x200) y -= 0x400;

		if (data)
		{
			if (!big_sprite)
			{
				x_num  = (data >> 8) & 0xff;
				y_num  = (data >> 0) & 0xff;
				x_no   = 0;
				y_no   = 0;
				xlatch = x;
				ylatch = y;
				data   = ram[offs + 4];
				zoomxlatch = (data >> 8) & 0xff;
				zoomylatch = (data >> 0) & 0xff;
				big_sprite = 1;
			}
		}

		data = ram[offs + 4];
		zoomx = (data >> 8) & 0xff;
		zoomy = (data >> 0) & 0xff;
		zx = (0x100 - zoomx) / 16;
		zy = (0x100 - zoomy) / 16;

		if (big_sprite)
		{
			zoomx = zoomxlatch;
			zoomy = zoomylatch;

			x = xlatch + x_no * (0x100 - zoomx) / 16;
			y = ylatch + y_no * (0x100 - zoomy) / 16;
			zx = xlatch + (x_no + 1) * (0x100 - zoomx) / 16 - x;
			zy = ylatch + (y_no + 1) * (0x100 - zoomy) / 16 - y;
			y_no++;

			if (y_no > y_num)
			{
				y_no = 0;
				x_no++;

				if (x_no > x_num) big_sprite = 0;
			}
		}

		if (zoomx || zoomy )
		{
			RenderZoomedTile(dest, tiledata[1], code, (color & 0x3f) << 4, 0, x, y, flipx, flipy, 16, 16, zx << 12, zy << 12);
		}
		else
		{
			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(dest, code, x, y, color & 0x3f, 4, 0, 0, tiledata[1]);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(dest, code, x, y, color & 0x3f, 4, 0, 0, tiledata[1]);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(dest, code, x, y, color & 0x3f, 4, 0,0, tiledata[1]);
				} else {
					Render16x16Tile_Mask_Clip(dest, code, x, y, color & 0x3f, 4, 0, 0, tiledata[1]);
				}
			}
		}
	}

	nScreenWidth  = t_swide;
	nScreenHeight = t_shigh;
}

void TC0180VCUBufferSprites()
{
	INT32 ctrl = TC0180VCUReadControl();

	if (~ctrl & 0x01) {
		memset (TC0180VCUFbRAM + framebuffer_page * 0x20000, 0, 512 * 256);
		memset (TC0180VCUFramebuffer[framebuffer_page], 0, 512 * 256 * sizeof(UINT16));
	}

	if (~ctrl & 0x80) {
		framebuffer_page ^= 1;
	}

	if (tilemask[1]) {
		TC0180VCUDrawSprite(TC0180VCUFramebuffer[framebuffer_page]);
	}
}

void TC0180VCUScan(INT32 nAction)
{
	struct BurnArea ba;

	if (nAction & ACB_VOLATILE)
	{
	bprintf (0, _T("yup\n"));

		ba.Data	  = (UINT8*)TC0180VCUFramebuffer[0];
		ba.nLen	  = 512 * 256 * sizeof(UINT16);
		ba.szName = "Framebuffer 0";
		BurnAcb(&ba);

		ba.Data	  = (UINT8*)TC0180VCUFramebuffer[1];
		ba.nLen	  = 512 * 256 * sizeof(UINT16);
		ba.szName = "Framebuffer 1";
		BurnAcb(&ba);

		ba.Data	  = TC0180VCURAM;
		ba.nLen	  = 0x10000;
		ba.szName = "Tilemap RAM";
		BurnAcb(&ba);

		ba.Data	  = TC0180VCUScrollRAM;
		ba.nLen	  = 0x00800;
		ba.szName = "Scroll RAM";
		BurnAcb(&ba);

		ba.Data	  = TC0180VCUFbRAM;
		ba.nLen	  = 0x00800;
		ba.szName = "Framebuffer RAM";
		BurnAcb(&ba);

		ba.Data	  = TC0180VCUControl;
		ba.nLen	  = 0x00010;
		ba.szName = "Control RAM";
		BurnAcb(&ba);

		SCAN_VAR(framebuffer_page);
	}
}
