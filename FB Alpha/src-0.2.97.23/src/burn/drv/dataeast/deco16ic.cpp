/*
TO DO!

// column scroll needs improved does 'size' (16/8) sized columns rather than 1pxl
// this affects boogie wings
// why is mutant fight's scrolling off in title?

1   Double Wings                MBE     102     52              141             104
*/



#include "tiles_generic.h"
#include "bitswap.h"

static INT32 deco16_layer_size[4];
static INT32 deco16_layer_size_select[4];

static INT32 (*deco16_bank_callback[4])(const INT32 bank);

static INT32 deco16_pf_colorbank[4];
static INT32 deco16_pf_colormask[4];
static INT32 deco16_pf_transmask[4];

static INT32 deco16_pf_bank[4];

static INT32 deco16_pf_gfx_bank[4]; // (1/2) 8x8, 16x16, (2/3) 8x8, 16x16

static UINT16 deco16_scroll_x[4][ 512]; // 512
static UINT16 deco16_scroll_y[4][1024]; // 1024

static INT32 deco16_enable_rowscroll[4];
static INT32 deco16_enable_colscroll[4];

static INT32 deco16_global_x_offset = 0;
static INT32 deco16_global_y_offset = 0;

static INT32 deco16_scroll_offset[4][2][2]; // tmap, size, x, y

static UINT16 transmask[4][2];

INT32 deco16_graphics_mask[3];

UINT8 *deco16_graphics[3];

UINT16 *deco16_pf_control[2];
UINT8 *deco16_pf_ram[4] = { NULL, NULL, NULL, NULL };
UINT8 *deco16_pf_rowscroll[4];//4 or 2?

UINT16 deco16_priority;

UINT8 *deco16_prio_map;
UINT8 *deco16_sprite_prio_map; // boogwing

INT32 deco16_vblank;

void deco16ProtScan();
void deco16ProtReset();

INT32 deco16_get_tilemap_size(INT32 tmap)
{
	return deco16_layer_size_select[tmap];
}

// painfully unfast.
void deco16_draw_prio_sprite(UINT16 *dest, UINT8 *gfx, INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 pri, INT32 spri)
{
	gfx += code * 0x100;

	INT32 flip = 0;
	if (flipx) flip |= 0x0f;
	if (flipy) flip |= 0xf0;

	sy -= deco16_global_y_offset;
	sx -= deco16_global_x_offset;

	for (INT32 yy = 0; yy < 16; yy++, sy++) {

		if (sy < 0 || sy >= nScreenHeight) continue;

		for (INT32 xx = 0; xx < 16; xx++, sx++) {
			if (sx < 0 || sx >= nScreenWidth) continue;

			INT32 pxl = gfx[((yy * 16) + xx) ^ flip];

			if (!pxl) continue;

			if (pri != -1) {
				INT32 bpriority = deco16_prio_map[(sy * 512) + sx];
	
				if (spri == -1) {
					if ((pri & (1 << (bpriority & 0x1f))) || (bpriority & 0x80)) continue;
					deco16_prio_map[sy * 512 + sx] |= 0x80; // right?
				} else {
					if (pri <= bpriority || spri <= deco16_sprite_prio_map[sy * 512 + sx]) continue;
					deco16_sprite_prio_map[sy * 512 + sx] = spri;
					deco16_prio_map[sy * 512 + sx] = pri; // right?
				}
			}

			dest[sy * nScreenWidth + sx] = pxl | color;

			deco16_prio_map[sy * 512 + sx] |= 0x80; // right?
		}

		sx -= 16;
	}
}

void deco16_draw_prio_sprite(UINT16 *dest, UINT8 *gfx, INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 pri)
{
	deco16_draw_prio_sprite(dest, gfx, code, color, sx, sy, flipx, flipy, pri, -1);
}

static inline UINT32 alpha_blend(UINT32 d, UINT32 s, UINT32 p)
{
	INT32 a = 256 - p;

	return (((((s & 0xff00ff) * p) + ((d & 0xff00ff) * a)) & 0xff00ff00) |
		((((s & 0x00ff00) * p) + ((d & 0x00ff00) * a)) & 0x00ff0000)) >> 8;
}

void deco16_draw_alphaprio_sprite(UINT32 *palette, UINT8 *gfx, INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 pri, INT32 spri, INT32 alpha)
{
	if (alpha == 0) return;

	gfx += code * 0x100;

	INT32 flip = 0;
	if (flipx) flip |= 0x0f;
	if (flipy) flip |= 0xf0;

	sy -= deco16_global_y_offset;
	sx -= deco16_global_x_offset;

	UINT32 *dest = (UINT32*)pBurnDraw;

	for (INT32 yy = 0; yy < 16; yy++, sy++) {

		if (sy < 0 || sy >= nScreenHeight) continue;

		for (INT32 xx = 0; xx < 16; xx++, sx++) {
			if (sx < 0 || sx >= nScreenWidth) continue;

			INT32 pxl = gfx[((yy * 16) + xx) ^ flip];

			if (!pxl) continue;

			INT32 bpriority = deco16_prio_map[(sy * 512) + sx];

			if (spri == -1) {
				if ((pri & (1 << (bpriority & 0x1f))) || (bpriority & 0x80)) continue;
				deco16_prio_map[sy * 512 + sx] |= 0x80; // right?
			} else {
				if (pri <= bpriority || spri <= deco16_sprite_prio_map[sy * 512 + sx]) continue;
				deco16_sprite_prio_map[sy * 512 + sx] = spri;
				deco16_prio_map[sy * 512 + sx] = pri; // right?
			}

			if (alpha == 0xff) {
				dest[sy * nScreenWidth + sx] = palette[pxl | color];
			} else {
				dest[sy * nScreenWidth + sx] = alpha_blend(dest[sy * nScreenWidth + sx], palette[pxl | color], alpha);
			}
		}

		sx -= 16;
	}
}

void deco16_palette_recalculate(UINT32 *palette, UINT8 *pal)
{
	UINT16 *p = (UINT16*)pal;

	for (INT32 i = 0; i < BurnDrvGetPaletteEntries() * 2; i+=2)
	{
		INT32 b = (BURN_ENDIAN_SWAP_INT16(p[i + 0]) >> 0) & 0xff;
		INT32 g = (BURN_ENDIAN_SWAP_INT16(p[i + 1]) >> 8) & 0xff;
		INT32 r = (BURN_ENDIAN_SWAP_INT16(p[i + 1]) >> 0) & 0xff;

		palette[i/2] = BurnHighCol(r, g, b, 0);
	}
}

void deco16_tile_decode(UINT8 *src, UINT8 *dst, INT32 len, INT32 type)
{
	INT32 Plane[4]  = { ((len / 2) * 8) + 8, ((len / 2) * 8) + 0, 0x00008, 0x00000 };
	INT32 XOffs[16] = { 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,	0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 YOffs[16] = { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 };

	INT32 Plane1[8] = { 0x100000*8+8, 0x100000*8, 0x40000*8+8, 0x40000*8, 0xc0000*8+8, 0xc0000*8, 8, 0 };

	UINT8 *tmp = (UINT8*)BurnMalloc(len);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, src, len);

	if (type == 2) { //16x16x8bpp
		GfxDecode(4096, 8, 16, 16, Plane1, XOffs + 0, YOffs, 0x200, tmp, dst);
	} else if (type == 1) { // 8x8
		GfxDecode((len * 2) / 0x040, 4,  8,  8, Plane, XOffs + 8, YOffs, 0x080, tmp, dst);
	} else {
		GfxDecode((len * 2) / 0x100, 4, 16, 16, Plane, XOffs + 0, YOffs, 0x200, tmp, dst);
	}

	BurnFree (tmp);
}

void deco16_sprite_decode(UINT8 *gfx, INT32 len)
{
	INT32 Plane[4] = { 24,8,16,0 };
	INT32 XOffs[16] = { 512,513,514,515,516,517,518,519, 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 YOffs[16] = { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,  8*32, 9*32,10*32,11*32,12*32,13*32,14*32,15*32};

	UINT8 *tmp = (UINT8*)BurnMalloc(len);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, gfx, len);

	GfxDecode((len * 2) / 0x100, 4, 16, 16, Plane, XOffs, YOffs, 0x400, tmp, gfx);

	BurnFree (tmp);
}
 
void deco16_draw_layer(INT32 tmap, UINT16 *dest, INT32 flags)
{
	INT32 size		= deco16_layer_size_select[tmap];
	if (size == -1) return;

	INT32 control		= deco16_pf_control[tmap / 2][6];
	if (tmap & 1) control >>= 8; 

//	INT32 control0		= deco16_pf_control[tmap / 2][5];
//	if (tmap & 1) control0 >>= 8; 
//	if ((control0 & 0x80) == 0) return; // layer disable bit

	INT32 select = (tmap & 2) + ((tmap < 2) ? size : 0);

	size = size ? 16 : 8;

	INT32 bpp = (flags & 0x100000) ? 8 : 4;
	if (flags & 0x200000) bpp = 5;

	UINT8 *gfx = deco16_graphics[select];
	INT32 gfxmask	= deco16_graphics_mask[select];

	UINT16 *vram	= (UINT16 *)deco16_pf_ram[tmap];

	INT32 tmask		= transmask[tmap][(flags & 0x00100) >> 8];
	if (flags & 0x10000) tmask = 0; // opaque!

	INT32 priority		= flags & 0x000ff;

	INT32 tilebank		= deco16_pf_bank[tmap];
	INT32 colmask		= deco16_pf_colormask[tmap];
	INT32 colbank		= deco16_pf_colorbank[tmap] >> bpp;

	INT32 hmask = (32 * size) - 1;
	INT32 wmask		= (deco16_layer_size[tmap] * size) - 1;
	INT32 shift = (wmask & 0x100) ? 6 : 5;
	INT32 smask = size - 1;

	for (INT32 y = 0; y < nScreenHeight; y++)
	{
		INT32 xoff = deco16_scroll_x[tmap][y] & wmask;

		for (INT32 x = 0; x < nScreenWidth + size; x+=size)
		{
			INT32 yoff = deco16_scroll_y[tmap][x] & hmask;

			INT32 yy = (y + yoff) & hmask;
			INT32 xx = (x + xoff) & wmask;

			INT32 col = xx / size;
			INT32 row = yy / size;

			INT32 ofst;
			if (size == 8) {
				ofst = (row << shift) | (col);
			} else {
				ofst = (col & 0x1f) + ((row & 0x1f) << 5) + ((col & 0x20) << 5) + ((row & 0x20) << 6);
			}

			INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[ofst]);
			INT32 color = code >> 12;
	
			INT32 flipx = 0;
			INT32 flipy = 0;
	
			if ((color & 0x0008) && (control & 0x03)) {
				flipx = control & 0x01;
				flipy = control & 0x02;
				color &= 0x07;
			}
	
			code = ((code & 0xfff) | tilebank) & gfxmask;
			color = (color & colmask) + colbank;
			
			{
				color <<= bpp;

				INT32 sx = x - (xx & smask);

				INT32 sy = yy & smask;
				if (flipy) sy ^= smask;

				UINT8 *src = gfx + (code * size * size) + (sy * size);

				if (flipx) flipx = smask;

				for (INT32 xxx = 0; xxx < size; xxx++) {
					if ((xxx + sx) < 0 || (xxx + sx) >= nScreenWidth) continue;

					INT32 pxl = src[xxx^flipx];

					if ((tmask & (1 << pxl))) continue;

					dest[y * nScreenWidth + xxx + sx] = pxl | color;
					deco16_prio_map[y * 512 + xxx + sx] = priority;
				}
			}
		}
	}
}

void deco16_set_bank_callback(INT32 tmap, INT32 (*callback)(const INT32 bank))
{
	deco16_bank_callback[tmap] = callback;
}

void deco16_set_color_base(INT32 tmap, INT32 base)
{
	deco16_pf_colorbank[tmap & 3] = base;
}

void deco16_set_color_mask(INT32 tmap, INT32 mask)
{
	deco16_pf_colormask[tmap & 3] = mask;
}

void deco16_set_transparency_mask(INT32 tmap, INT32 mask)
{
	deco16_pf_transmask[tmap & 3] = mask;
}

void deco16_set_gfxbank(INT32 tmap, INT32 small, INT32 big)
{
	deco16_pf_gfx_bank[0 + (tmap & 2)] = small;
	deco16_pf_gfx_bank[1 + (tmap & 2)] = big;
}

void deco16_set_global_offsets(INT32 x, INT32 y)
{
	deco16_global_x_offset = x;
	deco16_global_y_offset = y;
}

static void set_transmask(INT32 tmap, INT32 tmask0, INT32 tmask1)
{
	transmask[tmap][0] = tmask0;
	transmask[tmap][1] = tmask1;
}	

static void set_graphics_mask(INT32 gfx, INT32 len)
{
	INT32 b = 1;

	while (b < (len - 1)) {
		b <<= 1;
	}

	deco16_graphics_mask[gfx] = b - 1;
}

void deco16_set_graphics(INT32 num, UINT8 *gfx, INT32 len, INT32 size /*tile size*/)
{
	deco16_graphics[num] = gfx;

	set_graphics_mask(num, len / (size * size));
}

void deco16_set_graphics(UINT8 *gfx0, INT32 len0, UINT8 *gfx1, INT32 len1, UINT8 *gfx2, INT32 len2)
{
	deco16_graphics[0] = gfx0;
	deco16_graphics[1] = gfx1;
	deco16_graphics[2] = gfx2;

	set_graphics_mask(0, len0 / (8 * 8));
	set_graphics_mask(1, len1 / (16 * 16));
	set_graphics_mask(2, len2 / (16 * 16));
}

void deco16_clear_prio_map()
{
	memset(deco16_prio_map, 0, 512 * 256);
	memset(deco16_sprite_prio_map, 0, 512 * 256);
}

void deco16_set_scroll_offs(INT32 tmap, INT32 size, INT32 offsetx, INT32 offsety)
{
	deco16_scroll_offset[tmap][size][0] = offsetx;
	deco16_scroll_offset[tmap][size][1] = offsety;
}

void deco16Init(INT32 no_pf34, INT32 split, INT32 full_width)
{
	set_transmask(0, 0x0001, 0xffff);
	set_transmask(1, 0x0001, 0xffff);
	set_transmask(2, 0x0001, 0xffff);
	set_transmask(3, 0x0001, 0xffff);

	if (split) {
		set_transmask(1, 0xff01, 0x00ff);
	}

	memset (deco16_scroll_offset, 0, 4 * 2 * 2 * sizeof(INT32));

	deco16_pf_ram[0] = (UINT8*)BurnMalloc(0x2000); // ok
	deco16_pf_ram[1] = (UINT8*)BurnMalloc(0x2000);

	deco16_pf_rowscroll[0] = (UINT8*)BurnMalloc(0x1000);// plenty
	deco16_pf_rowscroll[1] = (UINT8*)BurnMalloc(0x1000);

	deco16_pf_control[0]	= (UINT16*)BurnMalloc(0x10); //1/2
	deco16_pf_control[1]	= (UINT16*)BurnMalloc(0x10); //3/4

	if (no_pf34 == 0) {
		deco16_pf_ram[2] = (UINT8*)BurnMalloc(0x2000); // right?
		deco16_pf_ram[3] = (UINT8*)BurnMalloc(0x2000);

		deco16_pf_rowscroll[2] = (UINT8*)BurnMalloc(0x1000); // right?
		deco16_pf_rowscroll[3] = (UINT8*)BurnMalloc(0x1000);
	}

	deco16_prio_map = (UINT8*)BurnMalloc(512 * 256); // priority map
	deco16_sprite_prio_map = (UINT8*)BurnMalloc(512 * 256);

	deco16_bank_callback[0] = NULL;
	deco16_bank_callback[1] = NULL;
	deco16_bank_callback[2] = NULL;
	deco16_bank_callback[3] = NULL;

	deco16_layer_size[0] = 64;
	deco16_layer_size[1] = full_width ? 64 : 32;
	deco16_layer_size[2] = (!no_pf34) ? (full_width ? 64 : 32) : 0;
	deco16_layer_size[3] = (!no_pf34) ? (full_width ? 64 : 32) : 0;

	deco16_pf_transmask[0] = 15;
	deco16_pf_transmask[1] = 15;
	deco16_pf_transmask[2] = 15;
	deco16_pf_transmask[3] = 15;

	deco16_pf_colormask[0] = 15;
	deco16_pf_colormask[1] = 15;
	deco16_pf_colormask[2] = 15;
	deco16_pf_colormask[3] = 15;

	deco16_pf_bank[0] = 0;
	deco16_pf_bank[1] = 0;
	deco16_pf_bank[2] = 0;
	deco16_pf_bank[3] = 0;

	deco16_pf_colorbank[0] = 0;
	deco16_pf_colorbank[1] = 16 << 4; // 2
	deco16_pf_colorbank[2] = 0;
	deco16_pf_colorbank[3] = 16 << 4; // 4

	deco16_pf_gfx_bank[0] = 0;
	deco16_pf_gfx_bank[1] = 1;
	deco16_pf_gfx_bank[2] = 2;

	deco16_layer_size_select[0] = 1; //16x16
	deco16_layer_size_select[1] = 1;
	deco16_layer_size_select[2] = 1;
	deco16_layer_size_select[3] = 1;

	deco16_global_x_offset = 0;
	deco16_global_y_offset = 0;

	deco16_priority = 0;
}

void deco16Reset()
{
	memset (deco16_pf_rowscroll[0], 0, 0x1000);
	memset (deco16_pf_rowscroll[1], 0, 0x1000);

	if (deco16_pf_rowscroll[2]) {
		memset (deco16_pf_rowscroll[2], 0, 0x1000);
		memset (deco16_pf_rowscroll[3], 0, 0x1000);
	}

	memset (deco16_pf_control[0], 0, 16);
	memset (deco16_pf_control[1], 0, 16);
	memset (deco16_pf_ram[0], 0, 0x2000);
	memset (deco16_pf_ram[1], 0, 0x2000);

	if (deco16_pf_ram[2]) {
		memset (deco16_pf_ram[2], 0, 0x2000);
		memset (deco16_pf_ram[3], 0, 0x2000);
	}

	deco16_priority = 0;

	deco16ProtReset();
}

void deco16Exit()
{
	BurnFree(deco16_prio_map);

	BurnFree (deco16_sprite_prio_map);

	for (INT32 i = 0; i < 4; i++) {
		BurnFree (deco16_pf_rowscroll[i]);
		BurnFree (deco16_pf_ram[i]);
	}
	
	BurnFree (deco16_pf_control[0]);	
	BurnFree (deco16_pf_control[1]);
}

static void pf_update(INT32 tmap, INT32 scrollx, INT32 scrolly, UINT16 *rowscroll, INT32 control0, INT32 control1)
{
	if (~tmap & 2) {
		if (control1 & 0x80) {
			deco16_layer_size_select[tmap] = (control0 & 0x80) ? 0 : -1; // 8x8
		} else {
			deco16_layer_size_select[tmap] = (control0 & 0x80) ? 1 : -1; // 16x16
		}
	}

	deco16_enable_rowscroll[tmap] = 0;
	deco16_enable_colscroll[tmap] = 0;

	if (rowscroll == NULL) return;

	if ((control1 & 0x40) == 0x40) // row scroll
	{
		INT32 size = deco16_layer_size_select[tmap] ? 16 : 8;

		INT32 rows = 1;
		INT32 row_sel = (control0 >> 3) & 0x0f;
		if (row_sel < 9) rows = 0x200 >> row_sel;

		INT32 rownum = 512;

		if (size == 8) {
			rows /= 2;
			rownum /= 2;
			if (rows == 0) rows = 1;
		}

		if (rows != 1) deco16_enable_rowscroll[tmap] = 1;

		INT32 rsize = rownum / rows;

		INT32 xscroll = scrollx + deco16_global_x_offset + deco16_scroll_offset[tmap][size/16][0];

		for (INT32 r = 0; r < rows; r++) {
			for (INT32 p = rsize * r; p < (rsize * r) + rsize; p++) {
				deco16_scroll_x[tmap][(p - deco16_global_y_offset) & 0x1ff] = xscroll + BURN_ENDIAN_SWAP_INT16(rowscroll[r]);
			}
		}

		if (~control1 & 0x20) {
			for (INT32 r = 0; r < 1024; r++) {
				deco16_scroll_y[tmap][r] = scrolly + deco16_global_y_offset;
			}
		}
	}

	if ((control1 & 0x20) == 0x20) // column scroll
	{
		INT32 size = deco16_layer_size_select[tmap] ? 16 : 8;

		INT32 mask = (0x40 >> (control0 & 0x07)) - 1;
		if (mask < 0) mask = 0;

		INT32 cols = 1024 / (8 << (control0 & 0x07));

		INT32 colnum = 1024;

		if (size == 8) {
			cols /= 2;
			colnum /= 2;
			if (cols == 0) cols = 1;
		}

		if (cols != 1) deco16_enable_rowscroll[tmap] = 1;

		INT32 rsize = colnum / cols;

		for (INT32 r = 0; r < cols; r++) {
			for (INT32 p = rsize * r; p < (rsize * r) + rsize; p++) {
				deco16_scroll_y[tmap][p] = scrolly + BURN_ENDIAN_SWAP_INT16(rowscroll[(r & mask) + 0x200]) + deco16_global_y_offset;
			}
		}

		if (~control1 & 0x40) {
			INT32 xscroll = scrollx + deco16_global_x_offset + deco16_scroll_offset[tmap][size/16][0];

			for (INT32 r = 0; r < 512; r++) {
				deco16_scroll_x[tmap][(r - deco16_global_y_offset) & 0x1ff] = xscroll;
			}
		}
	}

	if ((control1 & 0x60) == 0x00) // normal scroll
	{
		INT32 size = deco16_layer_size_select[tmap] ? 16 : 8;

		for (INT32 r = 0; r < 1024; r++) {
			deco16_scroll_y[tmap][r] = scrolly + deco16_global_y_offset;
		}

		INT32 xscroll = scrollx + deco16_global_x_offset + deco16_scroll_offset[tmap][size/16][0];

		for (INT32 r = 0; r < 512; r++) {
			deco16_scroll_x[tmap][(r - deco16_global_y_offset) & 0x1ff] = xscroll;
		}
	}	
}

void deco16_pf12_update()
{
	if (deco16_bank_callback[0]) deco16_pf_bank[0] = deco16_bank_callback[0](deco16_pf_control[0][7] & 0xff);
	if (deco16_bank_callback[1]) deco16_pf_bank[1] = deco16_bank_callback[1](deco16_pf_control[0][7] >> 8);

	pf_update(0, deco16_pf_control[0][1], deco16_pf_control[0][2], (UINT16 *)deco16_pf_rowscroll[0], deco16_pf_control[0][5] & 0xff, deco16_pf_control[0][6] & 0xff);
	pf_update(1, deco16_pf_control[0][3], deco16_pf_control[0][4], (UINT16 *)deco16_pf_rowscroll[1], deco16_pf_control[0][5] >> 8  , deco16_pf_control[0][6] >> 8);
}

void deco16_pf34_update()
{
	if (deco16_bank_callback[2]) deco16_pf_bank[2] = deco16_bank_callback[2](deco16_pf_control[1][7] & 0xff);
	if (deco16_bank_callback[3]) deco16_pf_bank[3] = deco16_bank_callback[3](deco16_pf_control[1][7] >> 8);

	pf_update(2, deco16_pf_control[1][1], deco16_pf_control[1][2], (UINT16 *)deco16_pf_rowscroll[2], deco16_pf_control[1][5] & 0xff, deco16_pf_control[1][6] & 0xff);
	pf_update(3, deco16_pf_control[1][3], deco16_pf_control[1][4], (UINT16 *)deco16_pf_rowscroll[3], deco16_pf_control[1][5] >> 8  , deco16_pf_control[1][6] >> 8);
}

void deco16Scan()
{
	struct BurnArea ba;

	{
		char name[128];

		for (INT32 i = 0; i < 4; i++) {
			if (deco16_pf_ram[i] == NULL) continue;

		//	memset(&ba, 0, sizeof(ba));
			ba.Data	  = deco16_pf_ram[i];
			ba.nLen	  = 0x2000;
			sprintf (name, "Deco16ic RAM %d", i);
			ba.szName = name;
			BurnAcb(&ba);

		//	memset(&ba, 0, sizeof(ba));
			ba.Data	  = deco16_pf_rowscroll[i];
			ba.nLen	  = 0x1000;
			sprintf (name, "Deco16ic Rowscroll %d", i);
			ba.szName = name;
			BurnAcb(&ba);
		}

		memset(&ba, 0, sizeof(ba));
		ba.Data	  = (UINT8*)deco16_pf_control[0];
		ba.nLen	  = 16;
		ba.szName = "Deco16ic Control 0";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  = (UINT8*)deco16_pf_control[1];
		ba.nLen	  = 16;
		ba.szName = "Deco16ic Control 1";
		BurnAcb(&ba);

		SCAN_VAR(deco16_priority);
		SCAN_VAR(deco16_vblank);
	}

	deco16ProtScan();
}


//---------------------------------------------------------------------------------------------------------------------------------------------------
// Data East common sound routines

#define ENABLE_HUC6280

#ifdef ENABLE_HUC6280
#include "h6280_intf.h"
#endif

#include "burn_ym2151.h"
#include "burn_ym2203.h"
#include "msm6295.h"

static INT32 deco16_sound_enable[4]; // ym2203, ym2151, msm6295 0, msm6295 1
static INT32 deco16_sound_cpuclock = 0;

INT32 deco16_soundlatch;

static void deco16YM2151IrqHandler(INT32 state)
{
#ifdef ENABLE_HUC6280
	h6280SetIRQLine(1, state ? H6280_IRQSTATUS_ACK : H6280_IRQSTATUS_NONE);
#else
	state = state;
#endif
}

static INT32 deco16SynchroniseStream(INT32 nSoundRate)
{
#ifdef ENABLE_HUC6280
	return (INT64)h6280TotalCycles() * nSoundRate / deco16_sound_cpuclock;
#else
	return 0 * nSoundRate;
#endif
}

static double deco16GetTime()
{
#ifdef ENABLE_HUC6280
	return (double)h6280TotalCycles() / (deco16_sound_cpuclock * 1.0);
#else
	return 0;
#endif
}

static void deco16_sound_write(UINT32 address, UINT8 data)
{
//bprintf (0, _T("%5.5x, %2.2x\n"), address, data);

	switch (address)
	{
		case 0x100000:
		case 0x100001:
			if (deco16_sound_enable[1]) {
				BurnYM2203Write(0, address & 1, data);
			}
		return;

		case 0x110000:
			BurnYM2151SelectRegister(data);
		return;

		case 0x110001:
			BurnYM2151WriteRegister(data);
		return;

		case 0x120000:
		case 0x120001:
			MSM6295Command(0, data);
		return;

		case 0x130000:
		case 0x130001:
			if (deco16_sound_enable[3]) {
				MSM6295Command(1, data);
			}
		return;

		case 0x1fec00:
		case 0x1fec01:
#ifdef ENABLE_HUC6280
			h6280_timer_w(address & 1, data);
#endif
		return;

		case 0x1ff400:
		case 0x1ff401:
		case 0x1ff402:
		case 0x1ff403:
#ifdef ENABLE_HUC6280
			h6280_irq_status_w(address & 3, data);
#endif
		return;
	}
}

static UINT8 deco16_sound_read(UINT32 address)
{
//bprintf (0, _T("%5.5x, rb\n"), address);


	switch (address)
	{
		case 0x100000:
		case 0x100001:
			if (deco16_sound_enable[1]) {
				return BurnYM2203Read(0, address & 1);
			}
			return 0x00;
		
		case 0x110000:
			return 0xff; 

		case 0x110001:
			return BurnYM2151ReadStatus();

		case 0x120000:
		case 0x120001:
			return MSM6295ReadStatus(0);

		case 0x130000:
		case 0x130001:
			if (deco16_sound_enable[3]) {
				return MSM6295ReadStatus(1);
			}
			return 0;

		case 0x140000:
		case 0x140001:
#ifdef ENABLE_HUC6280
			h6280SetIRQLine(0, H6280_IRQSTATUS_NONE);
#endif
			return deco16_soundlatch;
	}

	return 0;
}

void deco16SoundReset()
{
#ifdef ENABLE_HUC6280
	h6280Open(0);
	h6280Reset();
	h6280Close();
#endif

	if (deco16_sound_enable[0]) BurnYM2151Reset();
	if (deco16_sound_enable[1]) BurnYM2203Reset();
	if (deco16_sound_enable[2]) MSM6295Reset(0);
	if (deco16_sound_enable[3]) MSM6295Reset(1);

	deco16_soundlatch = 0;
}

void deco16SoundInit(UINT8 *rom, UINT8 *ram, INT32 huc_clock, INT32 ym2203, void (ym2151_port)(UINT32,UINT32), double ym2151vol, INT32 msmclk0, double msmvol0, INT32 msmclk1, double msmvol1)
{
#ifdef ENABLE_HUC6280
	h6280Init(0);
	h6280Open(0);
	h6280MapMemory(rom, 	0x000000, 0x00ffff, H6280_ROM);
	h6280MapMemory(ram,	0x1f0000, 0x1f1fff, H6280_RAM);
	h6280SetWriteHandler(deco16_sound_write);
	h6280SetReadHandler(deco16_sound_read);
	h6280Close();
#else
	rom = rom;
	ram = ram;
#endif

	deco16_sound_cpuclock = huc_clock;

	deco16_sound_enable[0] = 1;			// ym2151 always enabled
	deco16_sound_enable[1] = ym2203 ? 1 : 0;
	deco16_sound_enable[2] = 1;			// msm6295 #0 always enabled
	deco16_sound_enable[3] = msmclk1 ? 1 : 0;

	BurnYM2151Init(3580000, ym2151vol);
	BurnYM2151SetIrqHandler(&deco16YM2151IrqHandler);
	if (ym2151_port != NULL) {
		BurnYM2151SetPortHandler(ym2151_port);
	}

	if (ym2203) {
		BurnYM2203Init(1, 4027500, NULL, deco16SynchroniseStream, deco16GetTime, 0);
#ifdef ENABLE_HUC6280
		BurnTimerAttachH6280(deco16_sound_cpuclock);
#endif
	}

	MSM6295Init(0, msmclk0 / 132, msmvol0, 1);

	if (msmclk1) {
		MSM6295Init(1, msmclk1 / 132, msmvol1, 1);
	}
}

void deco16SoundExit()
{
#ifdef ENABLE_HUC6280
	h6280Exit();
#endif

	if (deco16_sound_enable[0]) BurnYM2151Exit();
	if (deco16_sound_enable[1]) BurnYM2203Exit();
	if (deco16_sound_enable[2]) MSM6295Exit(0);
	if (deco16_sound_enable[3]) MSM6295Exit(1);

	MSM6295ROM = NULL;

	deco16_sound_enable[0] = 0;
	deco16_sound_enable[1] = 0;
	deco16_sound_enable[2] = 0;
	deco16_sound_enable[3] = 0;

	deco16_sound_cpuclock = 0;
}

void deco16SoundUpdate(INT16 *buf, INT32 len)
{
	if (deco16_sound_enable[0]) BurnYM2151Render(buf, len);
//	if (deco16_sound_enable[1]) BurnYM2203Update(buf, len);
	if (deco16_sound_enable[2]) MSM6295Render(0, buf, len);
	if (deco16_sound_enable[3]) MSM6295Render(1, buf, len);
}

void deco16SoundScan(INT32 nAction, INT32 *pnMin)
{
	if (nAction & ACB_DRIVER_DATA) {
		h6280CpuScan(nAction);
	
		SCAN_VAR(deco16_soundlatch);
		
		if (deco16_sound_enable[0]) BurnYM2151Scan(nAction);
		if (deco16_sound_enable[1]) BurnYM2203Scan(nAction, pnMin);
		if (deco16_sound_enable[2]) MSM6295Scan(0, nAction);
		if (deco16_sound_enable[3]) MSM6295Scan(1, nAction);
	}
}


//---------------------------------------------------------------------------------------------------------------------------------------------------
// Data East decryption routines

static const UINT16 xor_masks[16] =
{
	0xd556,0x73cb,0x2963,0x4b9a,0xb3bc,0xbc73,0xcbc9,0xaeb5,
	0x1e6d,0xd5b5,0xe676,0x5cc5,0x395a,0xdaae,0x2629,0xe59e,
};

static const UINT8 swap_patterns[8][16] =
{
	{ 0xf,0x8,0x9,0xc,0xa,0xd,0xb,0xe, 0x2,0x7,0x4,0x3,0x1,0x5,0x6,0x0 },
	{ 0xc,0xa,0xb,0x9,0x8,0xf,0xe,0xd, 0x6,0x0,0x3,0x5,0x7,0x4,0x2,0x1 },
	{ 0x8,0xc,0xb,0x9,0xd,0xe,0xf,0xa, 0x4,0x6,0x5,0x0,0x3,0x1,0x7,0x2 },
	{ 0x8,0x9,0xa,0xd,0xb,0xf,0xe,0xc, 0x5,0x4,0x0,0x7,0x2,0x6,0x1,0x3 },
	{ 0xc,0xd,0xe,0xf,0x8,0x9,0xa,0xb, 0x1,0x5,0x0,0x3,0x2,0x7,0x6,0x4 },
	{ 0xe,0xf,0xd,0x8,0xc,0xa,0xb,0x9, 0x1,0x2,0x7,0x6,0x4,0x3,0x0,0x5 },
	{ 0xd,0xe,0xa,0xb,0x9,0x8,0xc,0xf, 0x3,0x1,0x7,0x4,0x5,0x0,0x2,0x6 },
	{ 0x9,0x8,0xe,0xa,0xf,0xb,0xd,0xc, 0x6,0x0,0x5,0x2,0x4,0x1,0x3,0x7 },
};

static const UINT8 deco56_xor_table[0x800] =
{
	 0, 1, 2, 3, 4, 5, 5, 6, 7, 8, 9,10, 7,11,11, 9,11, 3, 7,10,12,13, 3,14, 0, 0, 9, 0,15, 3, 4,14,
	 6, 3,14,14, 3,12, 3,13, 9, 5,13, 3, 8, 2, 7,14, 2, 5, 1, 2, 3, 4,14,11, 8, 0, 6, 7,10, 3, 4, 7,
	15, 1, 2, 7, 7,14,15, 0, 7, 9,15, 3,14,10, 9,12,11,12, 2, 8, 1,10, 9,14, 5, 6,15,10, 4,15, 5, 2,
	 3, 2, 4,15, 7, 7,10,13, 1,13, 6, 6, 7, 5,10, 9, 5,15,14, 2, 7,12, 7, 4, 2,15,12,10,11,15, 4, 6,
	 9, 3,11,13, 0, 3, 6, 4,15, 8, 3, 8, 0, 3,13, 8, 6, 0,11, 4, 9, 7,11, 1, 8, 1,10, 9, 2, 8,11,15,
	 2, 8,10, 5, 3, 8, 0,10, 0,15, 3, 1,15, 3, 8, 6,13,11,10, 4,15, 4, 4, 3, 7, 7,15, 3,11,15,15, 2,
	 3, 7,13, 4,10,12, 9, 8,11, 2, 5, 8, 7, 4,14, 0,12, 1,15,10, 3,10, 5,13, 1, 4, 0, 7, 7, 2, 8,15,
	12, 1, 9,14,12, 9,11, 5,13, 1,12,12,14, 8, 8,13, 4,13, 1,12,10, 1,10,15, 7, 5, 1,13,10,10,15, 1,
	 3,14,13, 0, 0, 1, 3, 9, 5, 0, 4,13, 6,14, 2, 8, 3, 3, 5, 8, 1,10, 7, 5,13, 4, 2,12, 5, 1, 0, 4,
	 4, 6,14,10, 6, 3,11,10, 0,10, 2, 3,11,11,15,13, 3,10, 3, 8, 3, 4, 7, 1, 5, 3, 9, 8,14,13,13, 4,
	 1, 0, 1,12,12,12,11,11, 3, 7, 4,10,10, 3,10, 3, 1, 7, 4, 6, 2,12, 3, 3, 0, 0,10, 0, 6, 8, 0, 7,
	 2,15,10,14, 1,14, 4,12, 4, 2, 3, 4, 8, 0,10,10,15, 1, 8, 4,11, 2,12, 6,12,12,12, 2, 4, 6, 3,12,
	 4, 7,14,11,10, 6,13, 3, 4,14, 7, 5, 7, 5,10, 8, 5, 0, 7,12, 3, 7,14, 3, 3, 8, 0, 7, 6, 9,11,15,
	10,15, 9, 7, 0,13, 7, 3, 7, 4,10, 1, 7, 4, 3,15, 2, 6, 1,14,12,12,10,10, 1,11,15, 0,10, 7,12,15,
	 8,10, 2,14,11, 0,10,12,13,15,12,15, 8,11, 5, 3, 0, 7,10, 6, 1, 9,11, 8, 4, 2, 4,12, 6, 3,15, 3,
	 2, 1,10, 2, 2, 7,10, 6,11,12, 6, 2,11, 7,15, 1,13, 9,12, 3, 9,12, 2,12, 6, 5,11, 8, 0, 7,14,10,
	 7, 8, 6, 5,12, 4, 1,12,12,13, 8, 4, 8,12, 8, 2, 1,15,13,14, 8, 6,11, 2,13, 5, 6,10, 9, 5, 7,11,
	 5,11,13, 2, 0, 8, 2,10, 2, 1, 6, 4, 8,14, 0, 4, 8,11, 8,15,12, 0,15, 6, 1,13,15,10, 3,13,14,13,
	 4, 5,12, 4, 3, 8, 9,15,13,11, 4, 8,10, 7,12, 7, 7, 9, 7,11,13, 0,15, 5,11, 5, 6, 2, 5, 1, 9, 2,
	 9,15, 9, 8,14, 6,11, 7,10, 4,14,12, 3,12, 3, 0, 4, 8,11, 9, 5, 7,14, 1,10, 3, 2,12,12,10,12, 8,
	 4, 5,10,13,11, 6,14, 4,13,14, 8, 3,10, 5, 6, 4, 4,10, 6, 5,12,11,10, 5, 9, 6, 7, 6, 4, 6, 3,15,
	 5,14,14,13, 7, 6,12,13,15, 5,15,10, 2, 1,15, 3,10,15, 7, 4, 5, 2,10,15,14,15, 1,13,12, 9, 8,14,
	15, 3, 4, 1, 9,13,13, 3,12,15, 8,15, 7, 1,10,12,10, 9,12,10,14, 6, 5, 5, 4, 0,12,14,15, 6, 5, 5,
	 2,14, 2, 6,14, 2,12, 7, 5, 1, 2, 3, 2,12,15, 4,10, 5,10,13, 0, 2, 2, 1, 5, 7, 5, 2, 0,13, 0, 4,
	13,13, 9,12, 5, 5,10,15, 6,11, 5, 2,11, 8, 0, 6, 9,13, 2, 6, 1,11, 5, 1,12, 8, 4, 7, 9,13, 4, 4,
	15, 3, 4, 5, 5, 0, 9, 4,10, 8, 1, 0,13, 8, 6,12,15,15,10, 7,10, 2, 6, 1, 4, 0, 2, 8, 3,10, 2,12,
	 4, 1,12,15, 0, 1,10, 0,10,11, 6, 1, 0, 3, 2, 3, 4, 4, 9,14, 7, 3,13, 7,13, 9,14,12,10, 5, 4,14,
	 0, 5,15, 8,12,12, 0, 5,11,15, 8, 7,15, 5,12, 5, 5, 3, 6,13,15,15, 7, 0, 3, 9, 1, 5, 7, 7,12, 5,
	 3,12,10,11, 2,11, 5,10, 1, 3, 6, 2, 7, 1, 3, 0, 6, 8,12,15,14, 6,10,13, 9, 1,15, 4, 6,13,14, 7,
	 2,13,12, 9, 4, 7, 9, 9, 6,10, 6,15,15, 2, 0, 4,12,12,10, 2, 7, 4,14,10, 2, 6, 5,12, 0,14, 1,15,
	13, 1, 4,13, 3,13, 6,15,11,15, 0,13,12, 5, 4,11,14,15, 6,13, 6, 7, 2, 2, 7,11, 6, 7,15, 0,13,10,
	 6,14, 9,13,12,14, 4, 8, 5,13, 3,14, 6, 6,13, 8, 7, 7, 5, 8,12, 2, 6, 0, 6, 8, 2,13, 9,14, 3,15,
	 1, 4,13, 3, 6, 8,15, 4, 8, 7, 4,10, 0, 9, 2,15, 2, 1, 3, 3, 1, 4, 0, 3, 1, 8, 5, 0,10, 0, 1, 6,
	11, 3,12, 0,14,11, 0, 5,15, 9, 1, 7,15,12,12,10, 2, 2, 3,15,13,13, 2,13,14,13,13, 8,11,10, 9, 9,
	 9, 7, 7, 4, 9, 1, 9,10, 9, 9, 9,15,13,14,12,13, 6,11, 4, 6, 9,14, 6, 6, 7, 0, 8, 6,10, 7, 9,13,
	 4, 8, 1, 8, 9, 9, 3,14,10,14,11, 7,15, 8,13,11, 9, 0, 5, 4, 4, 7, 6, 0,13, 9,13, 7, 9, 3, 2, 0,
	 7,11, 5, 8, 8, 0,10, 3, 1, 4, 2,15,12, 5, 2, 6,15, 5,13, 6,12, 8,10, 9,12, 9, 1, 2,15,14, 5,11,
	13, 3, 6, 8,15, 5, 8, 2,10, 2, 8,14,15, 5, 4, 0,10, 2,14,14, 8, 8, 9, 9,11,11,12,11, 5, 4, 2,12,
	15,12, 1,11, 5,13, 0,12, 2, 4, 0, 0,12,12, 4,15, 7, 1, 2,13,10,10, 6, 4, 8, 6, 0,14,13, 1, 8, 3,
	10, 9,11,12, 6, 0,11,14, 1,13, 7, 4,11, 3,10, 0, 2,10,14,13, 7, 8,10, 7, 7,12,10,11, 3, 9,13, 8,
	 0,10,10,13, 7,12, 8, 8, 5,15,10,15, 7, 1, 8, 2,12, 1,14,10,10, 4, 5, 7,11, 3, 7, 7, 1,12,12,12,
	 1,14, 4, 0,11,15, 8,10, 1,14, 2,11, 5, 5,10,12, 2, 2, 1, 0,10, 0, 6, 2,10,13, 1,11,14, 0,12,11,
	 8, 8,12, 4, 5, 5,14,14, 9, 8, 0,10, 1, 9,15,10,13, 5, 2, 7, 1, 2, 6, 5, 5,11, 0,13, 8, 0, 7, 2,
	15,11,15, 0, 5,15, 5, 6, 0, 2, 5, 3, 0,14,14, 4,11, 7, 7, 4,10,11, 1, 8,13, 9, 4,13, 3, 6, 1,10,
	 1, 0,15,15,12,10, 6,11, 3, 4, 7, 8, 7,14, 8, 7,12,15, 9, 8, 7,14,14, 9,15, 8, 9, 4,12, 3,12,12,
	 7, 8,11, 2, 4, 8, 4, 9, 4,11,15, 2,13, 6, 3,13, 1,13, 5, 0, 2,10, 5, 8, 1,13,14, 0,11,15, 8,12,
	10, 4, 5,10, 4,11,11, 2, 2,13, 2, 6, 0, 7, 4,11, 1,12, 6,13,14, 5, 9, 0, 5,13,14, 6,13, 3,13, 9,
	14,14, 0, 9,12,14, 9,10, 1, 2,12, 3, 3,15, 8, 4, 3, 1,13,13,13,13, 4, 3, 5,15, 8, 2,15,13,11, 7,
	 4, 0,15,14, 1,13, 6, 8, 2, 8, 7,14,11, 7, 4, 1, 8,12,13,11, 0, 9, 0, 3,12,11, 2, 3, 9, 1, 3,13,
	 4, 5,15,10, 5, 9, 0, 3, 9, 6,13, 7,15,13,11,13,12, 6,14,15, 3, 6,10, 3,11,10, 1, 4,13, 0,15,15,
	11, 0, 7,14, 9,15, 1,14, 2,13, 7,12,11, 7, 9, 6,11, 5,12, 4, 3, 6, 8, 3, 9, 0,15, 7, 1,11, 7,12,
	 2,13,13, 3, 0,11, 1,11, 6, 1, 5, 5, 4, 5,12, 3, 3,15, 3,10, 7,12,15, 0,13,10, 0, 2, 6,12, 9, 5,
	 3, 0, 0, 6, 3,13, 7,14,11,15, 7, 2, 2,11, 6,11, 9, 3,10,11, 1, 8, 1,13, 0,11, 2,10, 3,12, 4,11,
	 0, 0,10,10, 2, 3,15, 0, 0, 9,14, 5,14,10,14, 0,13, 1, 1,12, 3, 3, 1,15, 8, 8,11,13, 8, 4, 8, 5,
	 8, 7,13, 2, 1, 1,14,15, 1, 7, 7, 1,14, 1, 4,14, 0, 0, 7, 6, 5,11, 5, 8, 7, 1, 4, 3, 8,14, 5,11,
	13, 8, 1, 9, 6,12, 9, 6, 3, 3,11,12, 5,11, 4, 4, 3,11, 0, 0,10,10, 7,15,15,11, 1,11,13,11, 6, 8,
	 2,15,10, 7, 0, 6, 3, 5, 2,14,12, 8,12,10,15, 8, 4,11, 8,13,11, 2, 5,13,10, 5, 9,10,15, 6, 3,12,
	 3, 0,11,15, 5,13,12,13,12, 0, 8,12,10, 0, 5, 5, 3, 2,10, 5, 3,10, 7, 2,13,12,15,13, 1,10, 4,10,
	12,14,12, 1, 6,13, 6, 6, 0,13,11, 4, 5, 7, 1, 7, 7, 0, 4,13,13,12,14, 6, 1, 3,13,11, 1, 4, 9, 9,
	 2,12, 8, 5,10,15, 2, 0,14, 9, 5, 8,11, 5, 3, 4, 7, 1, 2,13, 3, 8, 1, 3,11,12,11, 5, 1, 5,10,12,
	10, 2,10,13,12, 3, 9, 2,12,14,13, 8, 2, 1, 5, 3,15, 2, 8, 2, 1,15,10, 6,10, 3, 8, 9, 6, 9, 2,10,
	 4,12,10,15,14,15, 1,15, 0,13, 6, 0, 5, 6,13,12, 2,14, 0,10, 6,15, 8, 8, 6, 8, 2,12,11, 3, 9, 5,
	 9,15,11, 3,11,14,15, 6,14, 6, 7,12, 7,10, 6,14, 6, 6, 3, 5, 9,14, 7, 5, 8, 6, 2,12,15, 1, 2, 4,
	11, 6, 3, 0, 5,11,14,14,14, 4,13, 6,11, 6, 4,12,15, 2, 4, 1, 1, 5, 1,15, 7, 0, 5, 0, 0, 0,12, 1,
};

static const UINT16 deco56_address_table[0x800] =
{
	0x527,0x1a1,0x2f6,0x523,0x297,0x005,0x141,0x3b4,0x539,0x794,0x6dd,0x498,0x59b,0x119,0x5db,0x631,
	0x2f1,0x565,0x409,0x158,0x2d6,0x16e,0x571,0x0b9,0x543,0x52b,0x668,0x0c0,0x5fd,0x216,0x5ea,0x24e,
	0x14b,0x09b,0x6d1,0x2f5,0x591,0x0b4,0x0a8,0x64a,0x39b,0x0d9,0x336,0x2a1,0x460,0x068,0x7b4,0x187,
	0x59a,0x2f0,0x5cd,0x181,0x7f7,0x733,0x223,0x581,0x4c5,0x1d4,0x11b,0x22a,0x688,0x738,0x130,0x202,
	0x12e,0x203,0x33e,0x4d0,0x2bd,0x0c4,0x042,0x614,0x6c2,0x385,0x3ed,0x7e2,0x2bb,0x2ef,0x3e1,0x654,
	0x73f,0x447,0x15f,0x41d,0x320,0x05a,0x2f4,0x116,0x1d6,0x0c2,0x4c1,0x0f3,0x694,0x28f,0x15c,0x125,
	0x343,0x291,0x33b,0x21e,0x031,0x4fa,0x76b,0x133,0x503,0x4e8,0x19c,0x3c6,0x24a,0x0b5,0x1df,0x71e,
	0x5ed,0x564,0x648,0x5d7,0x5c2,0x4a4,0x30e,0x750,0x474,0x11f,0x4a2,0x3bb,0x64c,0x258,0x4c7,0x586,
	0x071,0x362,0x593,0x252,0x2b0,0x5aa,0x053,0x047,0x6ef,0x669,0x6e8,0x011,0x093,0x443,0x33c,0x060,
	0x211,0x7d3,0x4c8,0x426,0x1c1,0x2e8,0x1fb,0x51a,0x402,0x49f,0x519,0x1c7,0x096,0x4e6,0x279,0x455,
	0x7ac,0x3a6,0x4af,0x736,0x1cb,0x45c,0x275,0x18c,0x776,0x6e0,0x5d4,0x59c,0x210,0x358,0x64f,0x2cf,
	0x0f4,0x307,0x577,0x7ba,0x296,0x0f6,0x1a3,0x09d,0x2c5,0x532,0x7bd,0x26c,0x171,0x727,0x5ba,0x2da,
	0x37b,0x707,0x366,0x10e,0x346,0x107,0x20a,0x4b1,0x12d,0x10b,0x603,0x788,0x2f7,0x6da,0x28b,0x08d,
	0x3ff,0x1e6,0x2a4,0x39f,0x454,0x112,0x2ec,0x441,0x0fa,0x054,0x3e5,0x3a2,0x1ec,0x322,0x53e,0x413,
	0x5b5,0x058,0x751,0x27a,0x3a7,0x2d1,0x16d,0x3f3,0x5f5,0x13e,0x122,0x1fe,0x069,0x7dd,0x213,0x424,
	0x36f,0x6f5,0x20e,0x431,0x5b2,0x25e,0x601,0x169,0x1a9,0x1cd,0x2e3,0x39e,0x338,0x7e5,0x5e5,0x7b1,
	0x3dc,0x464,0x065,0x255,0x246,0x71c,0x55c,0x3f7,0x5f6,0x0c8,0x1b4,0x148,0x284,0x69f,0x17f,0x4cd,
	0x6d4,0x63f,0x7ca,0x28a,0x75f,0x2df,0x708,0x442,0x00e,0x140,0x457,0x14f,0x63a,0x420,0x23e,0x672,
	0x440,0x716,0x3ca,0x1bb,0x622,0x367,0x018,0x109,0x3c1,0x206,0x7cb,0x42e,0x44c,0x613,0x61e,0x67c,
	0x396,0x06d,0x01b,0x0c9,0x40f,0x4b6,0x6b8,0x485,0x758,0x4d3,0x61b,0x34d,0x4fe,0x7a6,0x025,0x3eb,
	0x73a,0x79b,0x3db,0x312,0x190,0x35d,0x7b2,0x78b,0x248,0x010,0x075,0x12b,0x345,0x753,0x64b,0x163,
	0x374,0x557,0x4d8,0x25b,0x780,0x479,0x012,0x360,0x51c,0x3a1,0x0d0,0x020,0x137,0x5cf,0x7c5,0x189,
	0x679,0x352,0x7da,0x48a,0x18b,0x5fc,0x1fd,0x344,0x4dd,0x69e,0x761,0x542,0x65d,0x55b,0x755,0x4f9,
	0x427,0x501,0x329,0x6f2,0x2b4,0x1b6,0x319,0x459,0x3fb,0x6df,0x4ab,0x176,0x2fd,0x62f,0x1ca,0x1b3,
	0x1b7,0x2dc,0x674,0x56c,0x3ab,0x428,0x6c0,0x541,0x23d,0x283,0x6aa,0x5cb,0x231,0x713,0x014,0x79c,
	0x1af,0x3f2,0x32b,0x19b,0x0d2,0x25c,0x5be,0x1c5,0x383,0x50d,0x3e4,0x657,0x4bb,0x0e8,0x403,0x26b,
	0x14e,0x3b7,0x08f,0x31b,0x43f,0x375,0x2ca,0x6bc,0x678,0x13d,0x0e3,0x395,0x2ff,0x120,0x7eb,0x630,
	0x52d,0x369,0x36a,0x58c,0x538,0x04a,0x461,0x24f,0x717,0x73c,0x784,0x59f,0x0c3,0x340,0x476,0x207,
	0x6b7,0x31c,0x0da,0x729,0x2c0,0x7f0,0x3d5,0x67a,0x092,0x314,0x34f,0x2bc,0x6ee,0x32a,0x1a7,0x06b,
	0x40b,0x029,0x7fc,0x608,0x316,0x15a,0x113,0x3b8,0x22b,0x445,0x389,0x7a1,0x174,0x218,0x10d,0x29b,
	0x4a5,0x0e5,0x25f,0x250,0x75e,0x4c4,0x721,0x230,0x0d8,0x3c2,0x680,0x65f,0x25d,0x222,0x44d,0x57d,
	0x056,0x37f,0x4a9,0x30d,0x6de,0x7f5,0x423,0x2f9,0x3be,0x097,0x380,0x27c,0x5f1,0x2ba,0x1ea,0x2a5,
	0x162,0x0bd,0x4ff,0x5fa,0x3bf,0x01c,0x3ae,0x682,0x550,0x46a,0x13c,0x193,0x3cb,0x01e,0x6e9,0x03a,
	0x6d6,0x323,0x55a,0x74f,0x508,0x4f1,0x616,0x609,0x2b3,0x32c,0x0b7,0x584,0x77c,0x4c2,0x412,0x20d,
	0x21c,0x5f8,0x26f,0x69a,0x40d,0x50b,0x552,0x000,0x4e9,0x6e2,0x022,0x5d1,0x2f3,0x243,0x22d,0x706,
	0x304,0x76d,0x201,0x1e0,0x7be,0x3a9,0x074,0x1ac,0x57a,0x754,0x244,0x08a,0x425,0x561,0x6d3,0x48f,
	0x298,0x636,0x760,0x507,0x282,0x013,0x091,0x232,0x067,0x1ef,0x691,0x7e8,0x489,0x62e,0x289,0x487,
	0x3df,0x562,0x3e7,0x104,0x040,0x117,0x3d8,0x3f1,0x135,0x1b2,0x3cc,0x0e4,0x5e8,0x75c,0x595,0x2af,
	0x72f,0x361,0x0cf,0x5ca,0x3d1,0x049,0x002,0x347,0x287,0x337,0x799,0x404,0x7bc,0x0f1,0x101,0x467,
	0x1e2,0x544,0x095,0x37c,0x2fa,0x763,0x38f,0x376,0x722,0x035,0x2e1,0x28e,0x09e,0x4fb,0x26d,0x611,
	0x46c,0x5e7,0x563,0x6d2,0x724,0x2a0,0x475,0x24d,0x240,0x437,0x7c8,0x1dc,0x47a,0x356,0x72b,0x06a,
	0x765,0x499,0x1a8,0x7b0,0x73d,0x592,0x149,0x4cb,0x5a9,0x1e1,0x4ba,0x774,0x0b6,0x570,0x469,0x294,
	0x2d0,0x5df,0x5a3,0x16c,0x273,0x2b9,0x6c3,0x68a,0x5c6,0x041,0x768,0x4e3,0x0bc,0x077,0x21a,0x2db,
	0x0be,0x006,0x79a,0x17b,0x6ec,0x689,0x4b5,0x471,0x0f5,0x515,0x683,0x47d,0x576,0x5de,0x3b0,0x705,
	0x676,0x7d4,0x659,0x02c,0x49e,0x05e,0x02d,0x68c,0x3c8,0x4b9,0x5dd,0x390,0x0f0,0x42a,0x663,0x381,
	0x48b,0x06e,0x2be,0x0c7,0x42f,0x62c,0x568,0x5ee,0x212,0x559,0x3ec,0x3b9,0x180,0x4e2,0x1e9,0x303,
	0x446,0x387,0x6c9,0x52a,0x33d,0x7e4,0x325,0x5ff,0x554,0x290,0x675,0x7db,0x22f,0x578,0x5bf,0x3b6,
	0x4b7,0x7dc,0x5e2,0x18f,0x1ba,0x56a,0x78c,0x6af,0x5a2,0x749,0x4d6,0x208,0x400,0x43a,0x35f,0x286,
	0x251,0x332,0x030,0x6f6,0x0d3,0x786,0x3a5,0x214,0x087,0x667,0x378,0x29e,0x015,0x166,0x7ce,0x1f7,
	0x3e8,0x49c,0x787,0x6be,0x16f,0x0ba,0x7d6,0x458,0x488,0x783,0x7cf,0x588,0x5dc,0x6c4,0x70a,0x100,
	0x4fd,0x13a,0x078,0x715,0x1d2,0x633,0x32d,0x72a,0x6d5,0x6fd,0x78f,0x3d2,0x2c3,0x03e,0x17e,0x505,
	0x583,0x326,0x2ea,0x51b,0x44a,0x04d,0x3d7,0x1e4,0x0ab,0x3e6,0x124,0x3b1,0x368,0x2c2,0x6c6,0x68e,
	0x4ee,0x1d5,0x00c,0x430,0x71a,0x651,0x22e,0x20f,0x4ec,0x08e,0x292,0x086,0x589,0x435,0x6db,0x567,
	0x769,0x4f6,0x599,0x038,0x36d,0x448,0x4f7,0x3fa,0x082,0x3c0,0x021,0x4bf,0x5b9,0x277,0x023,0x422,
	0x0c6,0x6ab,0x4e5,0x044,0x00f,0x23a,0x664,0x36b,0x6fb,0x462,0x3d4,0x5ec,0x439,0x604,0x04c,0x546,
	0x0a2,0x14c,0x436,0x128,0x063,0x735,0x7cc,0x410,0x105,0x4d1,0x655,0x1b5,0x3a4,0x6ba,0x04b,0x634,
	0x7c6,0x1ce,0x3ef,0x11a,0x066,0x433,0x767,0x526,0x524,0x37a,0x5ad,0x0ec,0x4dc,0x38d,0x35b,0x4b2,
	0x54d,0x19d,0x172,0x495,0x692,0x545,0x6a8,0x5a0,0x164,0x407,0x742,0x145,0x1ab,0x7c4,0x285,0x299,
	0x1aa,0x66b,0x55d,0x594,0x69b,0x138,0x660,0x463,0x5b8,0x027,0x29d,0x57c,0x77a,0x481,0x797,0x7d9,
	0x732,0x7c1,0x6c1,0x416,0x701,0x78e,0x695,0x4a6,0x7ea,0x44f,0x7e6,0x106,0x5d9,0x57e,0x4f2,0x779,
	0x79f,0x509,0x2b5,0x22c,0x502,0x108,0x0b8,0x228,0x062,0x147,0x4fc,0x417,0x205,0x619,0x60b,0x308,
	0x7d7,0x1ee,0x555,0x0bf,0x398,0x04e,0x02e,0x061,0x4f3,0x699,0x182,0x48c,0x0e9,0x4b8,0x196,0x27e,
	0x1de,0x2b6,0x3d6,0x677,0x0d6,0x579,0x700,0x19a,0x3af,0x68d,0x51d,0x5d2,0x31e,0x4ae,0x617,0x1f8,
	0x47c,0x7b3,0x7e3,0x50f,0x470,0x3a3,0x6dc,0x0c1,0x46e,0x6b2,0x5ae,0x5e1,0x492,0x6b6,0x14d,0x178,
	0x1c9,0x798,0x781,0x0a5,0x23b,0x702,0x540,0x245,0x3d9,0x3b5,0x3ea,0x6ae,0x605,0x3de,0x0aa,0x179,
	0x2c9,0x2ae,0x0ee,0x6a0,0x262,0x4ac,0x0cc,0x60a,0x5b1,0x0cd,0x194,0x0ad,0x76c,0x73e,0x71d,0x5eb,
	0x7c0,0x486,0x0cb,0x200,0x551,0x1c6,0x3bc,0x686,0x696,0x12c,0x242,0x321,0x373,0x60f,0x635,0x56d,
	0x115,0x5e6,0x4da,0x704,0x48d,0x261,0x4f8,0x217,0x49a,0x6fa,0x085,0x56b,0x2ad,0x75a,0x5da,0x720,
	0x790,0x1f9,0x10f,0x55e,0x05d,0x21d,0x748,0x263,0x4d7,0x771,0x3bd,0x26e,0x07e,0x415,0x432,0x6eb,
	0x525,0x2ee,0x703,0x08b,0x274,0x310,0x533,0x1b0,0x21f,0x5f3,0x1b1,0x090,0x5a6,0x35e,0x7ad,0x6e5,
	0x330,0x4b0,0x175,0x4c6,0x70d,0x2d3,0x2b2,0x68b,0x5ef,0x650,0x003,0x081,0x1b9,0x46f,0x01f,0x026,
	0x7cd,0x2cb,0x642,0x0a6,0x764,0x7a3,0x58e,0x45a,0x63b,0x6ed,0x1db,0x5e4,0x78d,0x661,0x2e4,0x051,
	0x719,0x558,0x7fd,0x4d2,0x348,0x220,0x5e9,0x377,0x434,0x5d3,0x4a1,0x07a,0x301,0x7a9,0x33f,0x372,
	0x52e,0x582,0x70f,0x1d9,0x690,0x6f4,0x391,0x084,0x4a0,0x399,0x7af,0x6f0,0x5ac,0x7f6,0x1ae,0x392,
	0x77b,0x52c,0x7f9,0x2e7,0x0a9,0x5a7,0x3a0,0x6a1,0x5bc,0x6fc,0x32f,0x746,0x6bd,0x16b,0x42d,0x036,
	0x233,0x0f7,0x224,0x480,0x055,0x465,0x4aa,0x159,0x5fe,0x478,0x1c3,0x536,0x518,0x7c2,0x143,0x7e1,
	0x331,0x597,0x199,0x5ab,0x4d5,0x2de,0x0ed,0x0dc,0x317,0x229,0x4db,0x76e,0x351,0x5c1,0x618,0x684,
	0x54b,0x1f4,0x103,0x553,0x64e,0x2b1,0x271,0x775,0x1a5,0x239,0x521,0x51f,0x12f,0x418,0x466,0x134,
	0x468,0x0f9,0x15b,0x192,0x548,0x1bc,0x6b9,0x6e1,0x411,0x653,0x2c1,0x5f0,0x6cd,0x511,0x0af,0x7d1,
	0x5d6,0x5bd,0x3ad,0x34e,0x41a,0x0fd,0x4f4,0x33a,0x666,0x146,0x1dd,0x30f,0x5a8,0x254,0x69d,0x44e,
	0x42c,0x1f1,0x5fb,0x37d,0x3b2,0x080,0x7d8,0x2ac,0x221,0x1ad,0x66f,0x0eb,0x0e6,0x649,0x7fe,0x2a2,
	0x673,0x18e,0x535,0x5cc,0x644,0x293,0x65e,0x1f6,0x573,0x150,0x3da,0x237,0x238,0x3c3,0x45b,0x1f2,
	0x4d4,0x43d,0x1d0,0x63d,0x429,0x386,0x64d,0x46d,0x620,0x7ae,0x453,0x2c7,0x57b,0x697,0x6e7,0x2dd,
	0x5e3,0x09f,0x019,0x41b,0x6e3,0x43e,0x363,0x0db,0x62d,0x341,0x215,0x37e,0x3e9,0x1da,0x32e,0x647,
	0x21b,0x406,0x793,0x58f,0x560,0x0b2,0x569,0x607,0x2cd,0x640,0x38e,0x38a,0x388,0x4c9,0x652,0x185,
	0x219,0x7bf,0x7ab,0x1eb,0x472,0x4ad,0x226,0x2fe,0x53f,0x270,0x739,0x313,0x032,0x7f3,0x70c,0x3e0,
	0x2a9,0x259,0x5b7,0x08c,0x3c7,0x6ad,0x15d,0x2fb,0x10a,0x75d,0x66e,0x4e4,0x1fa,0x68f,0x643,0x2b8,
	0x6bf,0x497,0x6ff,0x405,0x490,0x170,0x50e,0x073,0x693,0x1d8,0x59d,0x53c,0x1e8,0x711,0x740,0x637,
	0x624,0x45e,0x483,0x6f3,0x09a,0x513,0x5bb,0x34c,0x414,0x54c,0x007,0x757,0x74a,0x2a3,0x355,0x017,
	0x72e,0x379,0x5a1,0x4a3,0x2c4,0x671,0x731,0x572,0x65c,0x5d0,0x034,0x1be,0x31f,0x300,0x7b8,0x2d2,
	0x0fe,0x57f,0x7fb,0x6a4,0x227,0x260,0x1c2,0x052,0x0ef,0x5ce,0x547,0x510,0x0f2,0x6ce,0x6ac,0x04f,
	0x2d7,0x0d5,0x5a5,0x29a,0x256,0x3ac,0x195,0x048,0x752,0x6d9,0x606,0x2e6,0x7ff,0x05c,0x3f6,0x0d4,
	0x623,0x602,0x5b3,0x58a,0x77e,0x6f8,0x3f5,0x698,0x587,0x78a,0x450,0x7c7,0x2e9,0x66c,0x49d,0x01d,
	0x2ce,0x046,0x1d1,0x357,0x628,0x718,0x0dd,0x13b,0x685,0x600,0x741,0x072,0x4eb,0x646,0x71f,0x165,
	0x05b,0x63e,0x043,0x136,0x66d,0x0fc,0x500,0x0f8,0x23f,0x31a,0x35a,0x7bb,0x45f,0x710,0x264,0x58d,
	0x2f2,0x079,0x656,0x154,0x444,0x03c,0x12a,0x7b9,0x60e,0x315,0x364,0x3f8,0x725,0x575,0x365,0x7aa,
	0x6e6,0x796,0x009,0x008,0x6a3,0x2ab,0x549,0x47b,0x2a7,0x778,0x730,0x627,0x7f2,0x743,0x3a8,0x39c,
	0x598,0x67f,0x1bd,0x65b,0x71b,0x615,0x6d8,0x27f,0x714,0x13f,0x7d0,0x0e0,0x126,0x118,0x2a6,0x7d2,
	0x4cf,0x2aa,0x60d,0x3dd,0x19f,0x24c,0x225,0x6a7,0x60c,0x7ee,0x6e4,0x41f,0x6ea,0x001,0x687,0x6cc,
	0x6f7,0x514,0x40c,0x4de,0x11c,0x39a,0x1e3,0x53d,0x327,0x050,0x276,0x632,0x11d,0x183,0x0c5,0x3fe,
	0x2a8,0x00b,0x30b,0x4f0,0x79d,0x257,0x3f0,0x15e,0x43c,0x6f1,0x110,0x596,0x2c6,0x5c5,0x0bb,0x318,
	0x77d,0x306,0x1cc,0x267,0x5b6,0x161,0x151,0x160,0x638,0x7f4,0x1d3,0x1e7,0x452,0x6b1,0x057,0x197,
	0x324,0x482,0x333,0x712,0x3ba,0x1a2,0x3f9,0x6ca,0x099,0x266,0x25a,0x17a,0x61d,0x1b8,0x556,0x4b3,
	0x6a6,0x0ac,0x67b,0x792,0x66a,0x0ca,0x098,0x74d,0x75b,0x522,0x629,0x512,0x53a,0x73b,0x529,0x156,
	0x65a,0x094,0x48e,0x737,0x123,0x129,0x1c0,0x18a,0x662,0x7ec,0x6cf,0x438,0x6cb,0x14a,0x4d9,0x728,
	0x534,0x658,0x06c,0x70b,0x184,0x3fc,0x46b,0x791,0x665,0x7df,0x744,0x3fd,0x3ee,0x0e1,0x394,0x56e,
	0x496,0x30a,0x7f1,0x55f,0x6a9,0x28d,0x770,0x2fc,0x41e,0x0ff,0x204,0x494,0x186,0x1c8,0x155,0x2eb,
	0x328,0x5d5,0x49b,0x50c,0x0e7,0x76a,0x235,0x47e,0x62a,0x0fb,0x3f4,0x7c9,0x280,0x4c3,0x7a4,0x756,
	0x0e2,0x5c3,0x24b,0x064,0x305,0x3e2,0x38b,0x7a8,0x31d,0x3d3,0x132,0x2ed,0x0b1,0x61f,0x02b,0x342,
	0x18d,0x00a,0x62b,0x4be,0x72c,0x1ff,0x5c7,0x7de,0x295,0x0b3,0x059,0x7d5,0x253,0x20b,0x1f5,0x5f7,
	0x27d,0x0a3,0x45d,0x7f8,0x1a0,0x088,0x43b,0x1ed,0x037,0x6f9,0x153,0x747,0x269,0x612,0x16a,0x265,
	0x370,0x574,0x504,0x354,0x7a7,0x2b7,0x2d9,0x54a,0x7a0,0x745,0x421,0x734,0x528,0x0d1,0x371,0x4cc,
	0x585,0x07b,0x477,0x58b,0x3cd,0x278,0x38c,0x03b,0x349,0x40a,0x7e7,0x6b4,0x6c5,0x19e,0x493,0x4e7,
	0x4a8,0x382,0x610,0x72d,0x566,0x36c,0x5c8,0x766,0x083,0x34b,0x789,0x7ed,0x0b0,0x173,0x139,0x144,
	0x28c,0x4e1,0x20c,0x3b3,0x41c,0x1bf,0x2e0,0x1a4,0x1f0,0x004,0x77f,0x7ef,0x114,0x63c,0x34a,0x27b,
	0x070,0x4e0,0x537,0x4b4,0x127,0x302,0x033,0x47f,0x5f4,0x3cf,0x3c4,0x07f,0x272,0x102,0x6b5,0x121,
	0x516,0x5f9,0x7b6,0x076,0x334,0x5c0,0x2e2,0x339,0x3e3,0x6b0,0x69c,0x0ea,0x56f,0x168,0x5e0,0x167,
	0x3c9,0x639,0x191,0x762,0x7b5,0x795,0x384,0x772,0x3ce,0x76f,0x02f,0x23c,0x773,0x249,0x2c8,0x03d,
	0x74c,0x06f,0x09c,0x0ae,0x6a2,0x131,0x4ea,0x1fc,0x309,0x2d8,0x3d0,0x51e,0x7e0,0x209,0x111,0x234,
	0x01a,0x782,0x2e5,0x506,0x35c,0x7a5,0x2cc,0x517,0x30c,0x0d7,0x759,0x26a,0x4ce,0x401,0x1f3,0x10c,
	0x449,0x29f,0x5b4,0x79e,0x6c7,0x028,0x530,0x681,0x397,0x00d,0x670,0x2d5,0x5af,0x520,0x0a4,0x6a5,
	0x0df,0x723,0x039,0x4a7,0x281,0x70e,0x1e5,0x0de,0x02a,0x5c4,0x777,0x456,0x142,0x451,0x785,0x17d,
	0x393,0x491,0x419,0x6d0,0x645,0x247,0x11e,0x0a1,0x580,0x473,0x3aa,0x709,0x17c,0x6bb,0x177,0x52f,
	0x268,0x0a7,0x4ca,0x626,0x7a2,0x54e,0x484,0x4f5,0x4bd,0x5f2,0x198,0x4df,0x1c4,0x5a4,0x2d4,0x157,
	0x4c0,0x241,0x531,0x40e,0x44b,0x2f8,0x0a0,0x1d7,0x350,0x152,0x67d,0x6b3,0x6c8,0x288,0x36e,0x7c3,
	0x621,0x67e,0x4ef,0x7b7,0x7fa,0x359,0x408,0x6d7,0x29c,0x5c9,0x590,0x045,0x3c5,0x5b0,0x59e,0x0ce,
	0x50a,0x236,0x1a6,0x641,0x54f,0x61c,0x05f,0x61a,0x74e,0x74b,0x016,0x6fe,0x311,0x39d,0x4ed,0x353,
	0x625,0x4bc,0x335,0x42b,0x5d8,0x07d,0x03f,0x089,0x7e9,0x07c,0x1cf,0x024,0x188,0x53b,0x2bf,0x726,
};

static const UINT8 deco56_swap_table[0x800] =
{
	4,0,7,2,0,0,7,2,0,1,6,3,2,2,2,2,0,5,6,3,7,0,0,5,1,1,1,7,1,0,0,0,
	2,5,6,6,2,0,4,1,5,0,1,7,4,7,0,4,6,2,1,5,3,1,2,4,6,4,2,0,4,3,3,7,
	2,7,0,6,0,5,0,2,2,6,4,0,3,5,5,7,0,4,7,5,4,5,6,3,5,5,3,6,2,0,0,4,
	2,3,7,5,6,4,7,4,3,2,0,0,0,1,5,4,0,3,7,3,1,6,4,1,2,6,7,4,1,3,0,7,
	1,7,2,6,5,6,5,1,6,7,0,3,4,6,5,0,6,2,2,6,4,6,7,3,2,3,2,4,0,0,4,5,
	6,5,0,3,0,6,0,6,0,6,7,1,3,5,0,4,5,3,7,4,3,3,5,1,2,1,6,7,1,6,7,6,
	0,5,2,4,5,1,0,6,3,3,4,1,0,1,1,6,3,4,7,0,4,1,6,5,5,2,2,3,5,3,0,6,
	6,7,4,4,7,3,3,2,0,6,3,0,4,7,7,6,1,1,1,7,1,5,3,7,6,4,6,2,2,4,1,5,
	4,6,5,3,5,5,4,2,7,4,7,4,3,3,5,7,4,0,3,0,4,3,2,2,2,5,0,3,5,7,0,7,
	2,7,7,7,2,3,3,6,4,0,3,7,2,5,1,2,1,0,5,4,3,5,5,3,6,3,4,6,2,2,1,4,
	2,5,4,3,0,4,0,3,6,4,7,1,7,2,3,7,6,2,3,3,4,1,5,7,0,0,6,7,5,2,2,7,
	1,5,7,4,6,3,0,4,4,3,6,6,5,1,1,6,6,1,1,5,1,5,1,4,1,6,5,6,2,6,6,0,
	7,7,3,4,0,3,5,7,4,5,1,6,1,7,5,6,4,6,7,0,7,6,5,2,4,6,2,5,5,3,6,6,
	5,6,2,5,1,7,6,1,3,5,1,4,6,6,6,4,6,0,4,2,5,6,5,0,4,5,1,1,2,2,1,3,
	4,7,3,3,2,0,2,7,6,3,3,3,2,6,4,5,6,4,0,0,6,5,2,5,1,6,1,5,0,2,4,0,
	0,5,6,0,5,1,7,1,5,7,6,5,7,3,2,1,2,0,4,5,7,4,3,5,0,6,1,6,1,6,2,6,
	3,5,6,3,1,1,6,6,4,0,0,7,5,3,2,4,4,3,4,3,3,2,3,3,6,0,1,6,0,7,7,7,
	6,7,5,5,7,0,6,7,4,7,3,5,4,5,0,1,1,7,0,4,1,0,0,3,2,6,2,6,4,5,4,6,
	3,3,4,4,7,4,4,5,3,3,0,0,3,5,2,5,3,4,4,0,5,2,6,5,6,3,0,5,2,3,3,2,
	5,7,0,4,1,7,0,5,7,5,3,6,1,5,2,4,3,4,7,7,5,7,7,5,5,2,5,4,7,2,5,5,
	2,0,3,3,6,4,7,1,7,5,5,0,6,3,4,3,7,7,1,0,2,2,3,0,2,0,6,5,4,0,4,5,
	6,0,3,7,5,0,3,4,0,6,7,0,2,3,6,3,5,6,3,4,6,0,1,1,2,2,1,6,6,1,6,1,
	2,6,0,6,5,4,5,5,5,4,7,3,6,6,5,6,5,5,0,6,4,2,2,4,2,7,5,7,7,0,1,5,
	5,3,4,1,6,4,3,0,1,5,0,2,0,7,4,1,6,4,5,3,4,4,4,6,5,4,7,6,5,1,6,3,
	6,6,0,1,2,2,7,6,5,1,0,2,7,4,3,6,5,5,4,5,6,5,5,0,6,7,0,5,3,0,7,0,
	6,0,3,3,4,6,0,3,5,7,4,5,0,1,1,5,3,4,3,5,2,6,6,1,1,2,7,1,2,1,5,7,
	3,2,3,6,7,5,4,7,1,7,2,1,7,0,5,5,1,2,6,5,0,6,5,6,3,0,5,3,0,5,5,1,
	0,3,0,5,1,5,6,3,2,7,4,1,0,5,6,3,7,5,1,3,0,3,1,0,0,1,1,7,4,0,7,4,
	4,5,4,7,3,7,6,6,1,1,2,5,7,6,4,1,1,5,6,7,1,1,2,7,1,3,6,4,1,3,6,3,
	5,3,5,6,2,0,4,5,3,5,1,1,0,4,2,6,6,5,3,2,5,5,1,0,4,3,5,7,6,3,4,4,
	7,1,6,2,2,2,0,3,3,4,5,1,6,2,0,5,3,0,7,3,6,3,6,7,4,0,0,3,7,4,6,1,
	1,1,6,0,0,0,7,1,2,3,5,5,2,3,0,3,7,2,5,1,3,5,4,0,4,6,2,2,0,7,7,7,
	2,5,3,4,7,7,3,2,3,5,4,1,6,7,0,7,3,7,0,6,0,2,7,6,2,6,1,6,5,0,0,0,
	2,2,2,2,5,5,5,6,5,0,4,1,1,2,2,0,3,5,5,5,6,3,5,1,3,0,1,4,1,0,0,3,
	1,5,3,1,0,5,5,6,4,5,3,0,3,3,1,4,4,4,4,2,7,7,7,0,1,6,1,1,1,7,7,5,
	6,4,7,5,3,6,6,1,1,6,7,1,4,4,7,7,6,0,6,4,6,7,6,6,3,3,1,1,3,7,3,1,
	0,4,4,3,4,2,2,4,7,1,5,1,5,0,3,4,7,3,7,4,5,3,5,4,0,2,0,7,6,0,4,0,
	6,0,7,1,3,4,7,0,0,6,7,5,7,7,2,2,0,7,2,4,7,1,6,1,1,5,5,5,2,5,3,5,
	1,1,5,0,5,4,3,2,2,0,3,1,2,2,6,7,2,7,4,1,4,5,6,0,1,0,6,4,2,1,7,2,
	2,2,4,1,3,0,3,5,7,2,1,1,3,6,2,5,5,7,4,2,7,5,5,5,1,4,4,0,7,2,0,6,
	0,6,2,5,4,7,6,2,1,2,4,5,1,2,3,5,1,6,2,3,0,0,7,5,0,1,0,5,7,7,0,7,
	2,6,0,0,3,6,1,0,4,0,5,1,5,7,4,3,1,5,7,7,6,4,7,3,3,4,6,5,0,4,7,6,
	4,1,2,4,0,4,3,1,5,6,3,1,7,0,2,4,0,2,6,0,7,0,3,3,4,5,2,2,0,6,6,0,
	3,7,7,1,5,0,2,1,5,1,1,5,2,4,5,3,1,6,4,7,6,3,4,3,5,3,6,7,7,4,4,3,
	2,4,0,6,7,4,1,6,2,6,0,2,3,5,3,0,2,2,5,5,2,3,1,5,1,0,0,0,6,3,6,3,
	3,5,6,4,0,4,2,6,3,5,1,2,7,1,3,0,3,3,0,2,3,2,7,0,7,7,4,6,2,4,7,0,
	7,5,6,7,2,2,0,2,5,0,0,3,2,7,3,3,5,7,5,4,3,6,2,2,6,4,3,7,2,6,5,7,
	7,7,6,0,2,6,7,2,4,0,1,0,3,5,3,4,3,1,7,7,5,4,0,1,6,3,0,7,6,4,5,4,
	7,3,0,5,3,5,0,6,2,3,5,0,4,2,5,3,1,1,3,7,4,1,2,7,7,4,1,6,3,7,7,7,
	6,6,4,1,1,3,1,3,4,7,5,6,2,7,2,5,1,1,6,7,0,7,6,3,4,1,4,0,1,2,6,0,
	5,2,2,2,7,7,1,7,0,0,4,5,3,3,7,5,0,4,4,4,0,5,7,0,0,6,7,7,4,1,1,6,
	1,1,0,1,4,5,2,6,0,4,4,5,7,0,3,7,4,6,6,2,1,7,6,4,7,1,7,2,5,5,6,6,
	5,5,0,5,2,0,3,4,4,7,3,7,1,2,1,2,4,0,3,3,3,4,2,3,0,1,7,0,0,0,4,0,
	7,2,1,5,6,3,4,6,2,6,6,3,5,4,3,4,0,4,7,4,7,7,0,2,1,6,7,0,6,7,5,7,
	4,0,3,4,5,4,6,7,1,7,5,3,6,7,1,7,1,0,5,5,7,3,2,6,7,4,4,2,6,7,3,4,
	1,7,4,1,5,3,3,5,3,4,6,4,4,6,0,4,2,6,0,4,3,1,7,2,7,7,1,7,1,0,4,6,
	4,7,1,7,6,3,3,3,6,0,3,6,2,3,6,2,6,4,5,3,1,2,1,6,7,5,0,0,4,0,7,0,
	6,1,1,2,2,7,6,2,5,4,4,6,3,5,4,7,3,1,4,0,0,2,4,1,4,6,3,6,0,7,2,3,
	0,7,7,1,7,0,1,2,7,3,7,2,7,5,1,3,6,2,7,2,0,1,3,5,2,3,4,4,2,4,4,3,
	4,4,0,3,7,2,6,6,6,0,0,7,1,0,1,5,0,1,7,1,4,3,7,7,6,1,0,4,3,3,1,2,
	0,5,6,3,1,2,5,6,0,0,0,4,2,0,1,6,2,4,0,1,5,4,7,1,2,2,7,7,5,3,6,3,
	3,5,4,0,5,3,0,1,5,5,4,0,5,0,1,1,3,6,4,2,3,1,5,5,7,4,0,5,7,0,6,6,
	6,6,7,3,2,2,5,4,0,6,1,1,6,0,4,3,2,0,4,0,3,4,7,3,3,5,7,1,3,5,2,3,
	6,5,6,0,1,1,6,0,6,1,7,2,4,5,6,5,0,4,0,3,2,3,7,7,0,3,5,0,4,1,1,1,
};

static const UINT8 deco74_xor_table[0x800] =
{
	13,13, 0,10, 8,15,12, 0,10, 8,13,15,12, 3,15, 2,11, 1, 8,10,13, 4,10,12,11, 2, 0, 3, 0,11, 8,11,
	 5,14,11, 2, 5, 3, 8,11, 8,13,14, 4, 3,13,11,10,14, 6, 9,11,11, 8, 0,15, 2, 4, 5, 7,15, 8,13,11,
	 7, 0,12, 3,13, 3, 1, 2, 6,14,10, 3,15,11,15, 7,13,14, 4, 6, 1,10,14, 8, 4,15, 2, 8,15, 9, 3,11,
	15,11, 1,14,11, 5, 6,14, 3, 9,12,14,15,12, 6,15,12,13, 3,15, 4, 0, 8, 0, 6, 8, 0, 1, 6,10,14, 9,
	12,15, 3, 5,11, 4, 9, 6, 9,13,12, 0,12,14, 9, 9,13,10,15, 8,15, 4, 9, 3,15,15,14, 8, 9, 0,10,11,
	15,15, 1,15,10, 6,11,15, 7,11, 8,15, 2,12,15,15, 1,14,10,13, 0,14, 4, 3,12,13, 3, 5, 7, 0, 4, 1,
	 3, 5,13, 2, 0, 7, 2,13, 3, 7, 9, 2,15, 0, 2, 2, 2, 8, 6, 6,14, 2,15,11, 8, 0, 1,15, 5, 4, 2,15,
	13, 1, 1,13, 4,10,15, 3,14, 2,12, 8, 9, 8,13, 2, 0,14, 2, 9, 0, 2,12, 3, 4,15,13, 8, 4,12, 3,11,
	13,15, 5, 8, 7, 7, 7,15, 5,11,10, 4, 2, 1, 4,11,11,11, 8,10, 1,13, 5, 3, 1, 5, 8,15,10, 3,10, 9,
	 4,14, 2, 8, 6,14,14, 9, 7, 2,15, 0,15, 5,10, 3, 5,12,10, 0, 7, 2, 1,11, 4,13,10,14, 3, 1, 1, 1,
	 9,10,11, 0,14, 0, 1,11, 1, 2, 4,14, 9, 1,12,15, 0,10,11, 6,10, 4, 9, 1, 1, 5, 4, 3,15, 9,11, 0,
	 7,12,10, 9, 3,14, 4,11,11, 3, 2,10, 1,12, 4, 3,11, 1, 9, 1,15, 9, 3,11,12, 7,11, 6,15, 8, 1, 3,
	10,12,10, 4, 2,13,13,11, 3,13, 1, 2, 7,13, 6, 6, 7,14,12,15, 9, 7,15, 7,15, 6, 5,11, 5, 1,15,11,
	11, 7,14,12,12, 4,11, 3, 8, 9, 8,15,12, 8,10, 6, 0,12, 5, 9, 6, 7,10, 6, 1, 6,13, 1, 3,10, 8, 2,
	 1,13, 9, 4, 6,11, 2, 6, 2,15, 4,13, 3, 0, 7, 0, 9, 0, 6,14, 2,12,11, 0, 5,12, 7, 2, 6, 0, 2,10,
	10, 5, 1,12, 5,14,12, 1,13, 2, 6,11, 7,10, 0, 0, 7, 1, 8,10, 2, 3, 6, 4,11,10,10,13,15, 0,14,10,
	 3, 8,14,10, 0, 7, 2, 4, 4, 4, 0, 9,15, 9,14, 3, 4, 3, 5, 4, 7,15, 6,13, 7,12, 1,12,10, 1, 8,15,
	 5,15,12,13,14,14, 0, 4, 0, 0, 5, 0, 6,14, 8,14,14, 1,11, 5, 5,11, 3, 0, 2,15,11, 2, 7, 1,14,10,
	14, 3, 1, 5, 5,10, 1,10,13,13, 9, 9, 2, 1, 6, 7, 6, 7,11,15, 6, 6, 4,12, 8, 4, 3, 0, 0,15, 8, 2,
	 2,14, 1, 8,11, 7,11,10, 3,10, 6,10,14,10,14, 6, 1, 3, 5, 2, 2, 5, 5, 5, 8, 3,12,15, 6, 9,12,10,
	 4, 8,11, 6, 1, 8, 9,15,15,10, 0, 6,11, 8, 2, 9,15, 2, 2,10, 0, 5, 6, 5,11, 0, 0, 6, 0, 1,13, 6,
	 4, 4, 4, 1, 4, 0, 9,12, 2,10,13, 5,10,12,15,10,12, 3, 4, 7,10,13, 3,15,12,11, 6, 8,13, 2, 8,10,
	 3,12,13, 9,10, 4, 2,11, 1, 4, 9, 1,11, 2,14, 5, 2, 0,10,12, 0, 6, 7, 0, 2,10,15, 9, 0, 0, 8, 1,
	 0, 9, 1,14, 0,13,12,13, 9, 6,10,12,10, 0,13, 3, 7, 4,12, 0,11, 0, 0,11, 5, 0,14,15,11, 8, 2, 5,
	 4, 6, 5, 9,13, 5, 4, 4,13, 4, 0,12,10, 3, 2, 2, 0,13,11,12, 4,12, 9, 7,13,14,14, 8, 5, 0,13, 0,
	 1, 4,15,15, 9, 8, 5,12,13, 8,12, 8, 8,10, 0,15, 8,11, 5, 6, 9, 8,10,12, 9,12, 5,14, 6, 9, 7,15,
	 1, 9,15, 0, 1, 9, 1, 2, 8, 1, 4,12, 1, 4, 9, 7, 2,10, 8, 1,13, 6, 0,12, 0, 6,12,13, 5, 9, 4,12,
	 7,14,10, 4, 6, 8,12, 4, 9, 1, 2, 2, 3,13,10, 2,15, 7, 1, 6, 0,12, 5,10, 9, 1, 0,14,15, 3, 7, 6,
	 9,12,13, 7, 0,13, 2, 5, 7,13, 1, 3, 8,15, 9,10, 3, 0,15, 2,11, 5, 7, 3,10, 7, 4, 0, 7, 2,10, 2,
	 0, 3, 6, 6,13, 0, 5, 9,13, 8,11, 9, 0, 4, 1,11,15, 5,15, 3, 1,15, 1,15,12, 8, 9, 4, 1, 5, 8,11,
	 5, 7, 6,11, 5, 7,15,15, 9, 4, 7,12,14,11, 8, 6, 7, 7,12, 3, 9,12, 6, 9, 5, 8,13,14, 6,14, 4, 7,
	12,12, 2,14,10, 2, 1,10, 3, 8,12,11,12, 7,12,15, 7,13, 9,15, 6, 3, 1,14,13,14, 8,11,14, 4, 9, 2,
	13,13, 0, 2, 9,11, 3, 2, 4,11, 6, 8, 2,14, 7,15, 8, 8, 9, 5, 4,15,11, 7,13, 7, 4,15,15, 4, 2, 1,
	 6, 4, 3,15, 0,11,14,15, 6,15,11,14,10, 4, 4, 0, 5, 4, 6,15, 8,10,15, 1, 3, 8,13, 3, 3, 0, 7, 7,
	 2, 3, 2,11,14, 1,12,13,10, 2, 4,13, 4,13, 9, 3, 2, 1,14, 9, 8,11,15, 4, 8, 9,13, 2,10,14, 2, 4,
	 8,13,13,14,12, 1, 5,15,13, 3,12, 8,11,12,12, 1, 0,15, 9, 2, 6, 9, 3,12, 5, 0,10, 8, 5, 6,11,14,
	 7, 7, 0,10, 0,14,10, 8, 4,12, 5, 3,11,10, 5,13,15, 9,14,12,12, 0,14, 3, 2,12, 9,14,11,11, 9, 2,
	 8, 9,13, 0,10, 3,14,14,13, 6, 4,14, 2,10,10, 9,10, 4, 5, 4, 5, 3, 4, 6,15, 5,14, 2, 6,10, 4, 8,
	 6, 9, 9, 7, 3, 3, 9, 5, 9, 8,10, 5, 0,13, 7, 8, 4, 1, 3,13, 4, 1, 8,10, 3,11,13, 3,12, 0, 2, 0,
	 3, 1, 3, 3, 1, 6,13, 2, 3,13,11, 5, 9, 0, 4,12, 6,11, 4, 4, 3, 3,12, 7,13, 2,15,12, 1,11,15, 7,
	10, 0, 9, 3, 3,13,12,12, 9,14, 2, 2, 0, 3,12, 2, 0,15, 0,10, 1, 9, 8, 3, 2, 3, 9, 1, 4, 5,12, 9,
	10,11, 0, 2, 2,11, 5,13, 8, 1, 7, 8, 4,12,11, 1, 2,11, 7, 7, 8, 6, 5, 8, 8,14, 2, 6,10, 6, 1, 9,
	 3, 4, 2, 9,11, 1, 4, 9, 8,13, 5, 0,15,13, 0, 9, 9,11, 5,11, 6, 4, 6, 9, 5, 1,13, 8, 8, 2,14,14,
	 2, 3,14,12, 4, 6,15, 4,11,14, 7, 2, 3,14, 4,15, 4, 1, 8,13, 5, 5, 7, 0, 6,14, 0, 1, 1,13, 5,10,
	10,14,14,10, 2, 9, 1,11, 5, 6, 8,12, 4, 2, 7, 7,11, 6,13,15, 1, 3, 9, 0,14,12, 9, 2, 3, 2, 1, 9,
	12, 7, 4,12,14, 3, 3, 2, 3,14,10, 5, 8, 4, 9, 9,11,14,13,15,14, 7, 3,13, 0, 9, 4, 9,12,12, 1, 2,
	10,11, 7,11, 1, 9, 2, 8, 1, 4, 5,14, 8,15, 9, 3, 8, 9, 6, 3, 7, 3, 1,10, 7, 5,13,14, 8, 7, 4,10,
	14, 4, 6, 9, 6, 2, 5, 3, 8, 0, 9, 7, 2, 7,12, 8,12, 2,15, 4, 5, 0,11, 8,11, 2,14, 8,15, 0, 8, 9,
	14, 5, 4,13,12,11, 2, 6,11, 9, 9, 7,14, 6, 6, 7, 4, 6, 3, 5,12, 5,11, 1,12,14, 0,13,15,13, 2, 2,
	 1, 8, 3,11, 3, 1, 6, 9,12,12, 5,10, 7,13,15, 4,12,10, 9,11,12,14,12,11,15, 2, 3, 2, 7, 8,15, 2,
	 2, 0,12,12, 9,11,13,14,13, 7, 3,11,11,14,12, 3, 2, 7, 4, 8, 9,11, 8,11, 5,10, 3, 7,15,11,12,15,
	10, 8, 5,13, 8, 3, 9, 8, 5, 7, 1,15,15, 0,13, 7, 8, 6, 8,10, 8, 6, 6, 3, 4,10,13, 1, 6, 6,10, 1,
	 2,11, 4,15,15, 3, 1, 1,14,12, 1,15, 1, 6, 8,10,14,14, 9, 4, 7, 8, 6, 5, 3, 1, 0, 5, 3, 8, 6, 1,
	 4, 2,13,15,14, 5,13,11,10,13,11,11, 6,10, 7, 2, 1,15, 2,10, 5, 0,11, 5,15, 8, 0,14, 3,12,11, 7,
	10, 1, 4, 0, 3,15, 7,15,12, 8, 9, 0, 1,15,13, 2, 3, 2,15,15,14,14, 7, 8,12,11,15,15,14, 2, 8, 8,
	 2, 0,15, 4, 2, 5,14,10, 4,12, 2, 1, 0,13, 7, 5, 4, 3,11, 6, 0, 2, 6,14, 9,14,10,13, 6,10, 5, 8,
	15, 2, 4, 7, 1, 8, 5, 3,11, 5,12, 1, 3, 1,14, 4, 8,12, 6,10,14, 7,14, 7,15,14, 5,10, 6, 9, 3, 6,
	12,15, 1,12, 0, 6, 1, 4,13,13, 7, 1, 7, 1, 9, 1, 8, 6, 9, 9, 8,12, 5, 2, 8,11, 9, 1, 3, 9, 0,11,
	14, 8, 5, 9, 7, 7,14, 4, 6, 1, 6,11, 4, 0,15, 5, 9, 8, 5,13, 2, 6,13,10,12, 3, 4, 3,15, 5, 6,14,
	 2,10,14, 1, 5, 9, 6, 1, 9, 2, 7,11,14, 7,14, 5, 8, 9,14,13,14, 8, 3, 8, 4,10, 3,14, 0,13,12,15,
	 9, 9, 0, 4, 6, 7,12,13,12, 1,15, 2,13,13, 8,10,13, 5,12, 5, 4, 0, 2, 1,13,12, 2,14, 9,15,13, 9,
	 7,13, 7, 0,14, 7,12,15, 3,11, 0, 5, 8, 1, 0,12, 9, 0,11,13, 6, 3, 3, 7, 2, 2, 3, 5,14,14, 0, 9,
	 2, 6, 4,12, 2, 6, 3,12, 3,13, 3,14, 7, 1, 8,15,14, 3, 0,10, 2,15,15, 9,14,11,10,12,13,13,13, 9,
	 7, 7,12, 5, 3,14, 1, 5, 1,10, 6, 7, 9, 8,15, 7,13,11, 4, 5, 4,10,14,14,12, 1,13,14,15,14,15,10,
};

static const UINT16 deco74_address_table[0x800] =
{
	0x526,0x684,0x15f,0x1ad,0x736,0x341,0x4c3,0x23c,0x3eb,0x01f,0x18e,0x375,0x029,0x227,0x707,0x506,
	0x5ce,0x0dc,0x5dc,0x5fd,0x15c,0x013,0x7b9,0x3c1,0x77d,0x1eb,0x53d,0x4a9,0x66e,0x0a9,0x4e2,0x361,
	0x32d,0x151,0x16a,0x5ed,0x4b0,0x78a,0x0d1,0x01e,0x4be,0x7dd,0x22d,0x125,0x401,0x118,0x051,0x784,
	0x48a,0x083,0x42f,0x56b,0x448,0x679,0x019,0x076,0x44c,0x663,0x3a1,0x740,0x095,0x242,0x4c5,0x13b,
	0x3c4,0x130,0x122,0x047,0x16b,0x162,0x0d9,0x1f4,0x0be,0x018,0x56c,0x494,0x0b0,0x185,0x3b3,0x257,
	0x764,0x05d,0x7d4,0x2bd,0x159,0x2fb,0x07a,0x365,0x4fd,0x451,0x452,0x52d,0x753,0x5c8,0x1a4,0x50e,
	0x5ff,0x536,0x45b,0x281,0x4b8,0x1ff,0x228,0x5f9,0x1ca,0x587,0x0fd,0x02a,0x0da,0x03f,0x6a4,0x671,
	0x2d9,0x377,0x39c,0x534,0x6c5,0x1b2,0x1db,0x263,0x5c3,0x0c7,0x474,0x4db,0x152,0x3de,0x47d,0x64a,
	0x41c,0x66c,0x3d9,0x769,0x0c1,0x4ec,0x46b,0x274,0x6aa,0x623,0x31a,0x2ef,0x270,0x1e7,0x4e5,0x2e3,
	0x1f9,0x203,0x150,0x4a3,0x7d3,0x0af,0x1c8,0x310,0x4cd,0x1a6,0x14e,0x636,0x3f8,0x04d,0x337,0x441,
	0x3c0,0x1d8,0x482,0x399,0x3ef,0x520,0x260,0x352,0x4f8,0x521,0x271,0x6e6,0x5da,0x763,0x595,0x4e0,
	0x2db,0x3c6,0x012,0x09b,0x14f,0x26b,0x512,0x0cb,0x616,0x605,0x63a,0x5e2,0x139,0x7cd,0x7d0,0x00c,
	0x198,0x234,0x4f7,0x37e,0x0a1,0x4bd,0x1a1,0x2f0,0x3b4,0x0a6,0x1c7,0x51e,0x166,0x567,0x719,0x5cb,
	0x3b5,0x31c,0x751,0x6a1,0x065,0x4fa,0x6e1,0x106,0x1a5,0x331,0x3a2,0x16d,0x756,0x090,0x554,0x241,
	0x19a,0x35c,0x0e9,0x388,0x1c9,0x11b,0x5c7,0x56a,0x168,0x5b2,0x47f,0x5a9,0x551,0x3bb,0x3d3,0x428,
	0x132,0x4d7,0x1d1,0x538,0x5d7,0x297,0x6df,0x0ce,0x728,0x2c5,0x6f6,0x60d,0x2a0,0x45f,0x46d,0x4f0,
	0x5ea,0x1e0,0x4d6,0x442,0x32f,0x243,0x30f,0x0c9,0x344,0x42d,0x22f,0x327,0x2b2,0x158,0x7af,0x3d2,
	0x127,0x6d3,0x411,0x7eb,0x632,0x277,0x60b,0x356,0x307,0x1c3,0x0f1,0x762,0x207,0x13e,0x25a,0x6d0,
	0x38e,0x0a7,0x79a,0x2a5,0x771,0x39e,0x325,0x160,0x014,0x115,0x698,0x712,0x2c8,0x3c3,0x1bc,0x696,
	0x0d6,0x692,0x49f,0x4de,0x5b3,0x0cc,0x2f3,0x667,0x318,0x52c,0x3e0,0x181,0x462,0x1f7,0x366,0x4f4,
	0x547,0x44e,0x1be,0x3ed,0x04e,0x490,0x661,0x703,0x320,0x42e,0x406,0x161,0x259,0x702,0x58b,0x619,
	0x51f,0x502,0x391,0x4ce,0x779,0x0f9,0x05b,0x45e,0x14a,0x66a,0x42b,0x35a,0x363,0x1b8,0x373,0x117,
	0x78f,0x34e,0x2e0,0x2d7,0x7ee,0x27b,0x2c6,0x103,0x7dc,0x23d,0x2ad,0x049,0x68a,0x41f,0x582,0x3d5,
	0x631,0x107,0x2be,0x28e,0x5af,0x119,0x694,0x744,0x546,0x19b,0x766,0x5cf,0x2ee,0x0e0,0x4a5,0x313,
	0x6a3,0x3b1,0x28a,0x104,0x415,0x19e,0x01c,0x009,0x167,0x266,0x397,0x056,0x525,0x416,0x64f,0x689,
	0x706,0x413,0x004,0x6c9,0x244,0x2f8,0x78b,0x510,0x02e,0x505,0x5a6,0x612,0x114,0x4d9,0x1cc,0x20f,
	0x7e9,0x13a,0x584,0x495,0x6d2,0x5f4,0x093,0x43f,0x0e3,0x382,0x746,0x71e,0x794,0x72d,0x3ec,0x7c1,
	0x03a,0x33d,0x1ec,0x128,0x27f,0x700,0x149,0x10c,0x553,0x2bc,0x492,0x080,0x5b0,0x015,0x1c4,0x691,
	0x037,0x59f,0x542,0x4cf,0x253,0x5ae,0x74d,0x407,0x6e8,0x354,0x7b4,0x49b,0x300,0x6f2,0x26e,0x3f4,
	0x086,0x37c,0x1ee,0x347,0x6b1,0x26d,0x105,0x55d,0x1ba,0x70b,0x236,0x3e3,0x0b6,0x2c2,0x33b,0x254,
	0x140,0x2e8,0x5c2,0x03b,0x04c,0x5aa,0x480,0x560,0x6ed,0x633,0x7a6,0x29f,0x789,0x0e4,0x08a,0x15b,
	0x57e,0x4fe,0x532,0x25c,0x215,0x709,0x5f7,0x197,0x586,0x2f1,0x5e0,0x477,0x61b,0x608,0x063,0x01b,
	0x212,0x016,0x795,0x205,0x1fa,0x742,0x777,0x40b,0x146,0x73b,0x503,0x792,0x10e,0x545,0x7f6,0x3f7,
	0x4ad,0x38d,0x3b9,0x516,0x2ac,0x3bd,0x31b,0x214,0x170,0x5d6,0x21b,0x1b1,0x3cd,0x213,0x565,0x40a,
	0x113,0x515,0x724,0x03d,0x5c0,0x334,0x3e9,0x2a8,0x1c5,0x089,0x24e,0x7cb,0x4d5,0x280,0x36c,0x2b0,
	0x430,0x0e6,0x0d5,0x3c5,0x662,0x275,0x60f,0x0de,0x248,0x07b,0x22c,0x7b5,0x340,0x739,0x2b8,0x65a,
	0x508,0x116,0x079,0x7a0,0x4c2,0x099,0x4bb,0x52a,0x4d1,0x389,0x3a3,0x1e6,0x2a4,0x10d,0x4ba,0x581,
	0x0b2,0x302,0x0f4,0x760,0x25d,0x4c6,0x39b,0x12b,0x046,0x637,0x38b,0x0f6,0x109,0x51c,0x40d,0x6e9,
	0x6e2,0x7da,0x4a0,0x487,0x67e,0x65f,0x265,0x439,0x189,0x208,0x4b5,0x250,0x4f1,0x16e,0x496,0x3fc,
	0x272,0x20c,0x7ae,0x292,0x5e3,0x472,0x458,0x66d,0x657,0x722,0x438,0x3b8,0x4e4,0x39a,0x598,0x539,
	0x4b4,0x290,0x18b,0x1de,0x65c,0x2ea,0x6ff,0x20a,0x232,0x058,0x2d3,0x0d3,0x1f1,0x61d,0x0d2,0x1d2,
	0x295,0x466,0x038,0x0ac,0x4c7,0x6c7,0x672,0x040,0x6a0,0x699,0x624,0x71f,0x120,0x10b,0x072,0x192,
	0x3ba,0x638,0x58d,0x530,0x00e,0x761,0x6dd,0x1cd,0x18d,0x142,0x2eb,0x55e,0x7b2,0x367,0x5d3,0x71c,
	0x2d2,0x0a0,0x6b2,0x134,0x286,0x12e,0x0cf,0x5dd,0x2e7,0x74b,0x261,0x082,0x726,0x14d,0x1e3,0x4b7,
	0x7e7,0x435,0x294,0x31f,0x154,0x2cf,0x5f5,0x330,0x400,0x13d,0x629,0x7d1,0x4a2,0x3e6,0x3e8,0x1f5,
	0x3f3,0x368,0x410,0x3ca,0x221,0x71b,0x669,0x195,0x7f2,0x70f,0x006,0x504,0x062,0x461,0x053,0x6f7,
	0x0a3,0x6fa,0x1f2,0x745,0x7df,0x12a,0x05a,0x23b,0x4da,0x568,0x665,0x30d,0x5f6,0x328,0x283,0x169,
	0x17c,0x601,0x201,0x6c3,0x6c0,0x491,0x63f,0x6f5,0x2b5,0x2c7,0x628,0x3f9,0x02b,0x38a,0x2e9,0x60c,
	0x2a9,0x102,0x39f,0x0c3,0x42c,0x7cf,0x3df,0x4f9,0x235,0x3a8,0x7e4,0x485,0x21d,0x349,0x0e8,0x0b8,
	0x345,0x683,0x18c,0x317,0x6ca,0x323,0x17d,0x36a,0x7c9,0x1a0,0x3e1,0x6f9,0x0a2,0x353,0x7b6,0x759,
	0x287,0x7d7,0x25e,0x523,0x092,0x427,0x5a7,0x396,0x136,0x75a,0x3d6,0x507,0x385,0x5de,0x55f,0x0ad,
	0x096,0x239,0x5b8,0x6cb,0x574,0x511,0x41e,0x537,0x074,0x148,0x7a4,0x70e,0x2cb,0x284,0x191,0x06e,
	0x2da,0x51a,0x374,0x6ba,0x41b,0x552,0x460,0x371,0x183,0x0b3,0x444,0x748,0x7f4,0x59e,0x5bd,0x381,
	0x0ab,0x6fd,0x4cc,0x3e5,0x5c5,0x09c,0x717,0x22e,0x708,0x1bb,0x70c,0x4a6,0x778,0x034,0x1d5,0x30b,
	0x7f5,0x743,0x6ae,0x2de,0x2a6,0x613,0x5d0,0x40c,0x0bf,0x209,0x1b7,0x4f5,0x72b,0x620,0x647,0x643,
	0x024,0x2ca,0x0bd,0x501,0x0a4,0x453,0x3ff,0x6b5,0x625,0x2f7,0x556,0x469,0x409,0x747,0x031,0x1f6,
	0x47c,0x2ab,0x1fd,0x533,0x74f,0x395,0x754,0x0d0,0x6b0,0x42a,0x635,0x1af,0x6a9,0x49c,0x77b,0x611,
	0x2ae,0x2b4,0x3a5,0x1b5,0x3fe,0x3ab,0x479,0x498,0x3aa,0x5c1,0x27e,0x26a,0x590,0x29d,0x02c,0x121,
	0x404,0x61c,0x650,0x518,0x33a,0x768,0x0f2,0x0b4,0x686,0x67b,0x301,0x04b,0x1c1,0x7d9,0x772,0x0c8,
	0x49a,0x68f,0x59b,0x419,0x73a,0x750,0x360,0x58e,0x264,0x52e,0x00d,0x46e,0x085,0x46f,0x788,0x164,
	0x3d8,0x6db,0x52b,0x473,0x332,0x073,0x440,0x028,0x35b,0x3f1,0x14b,0x7fb,0x03e,0x28c,0x1ea,0x3f5,
	0x6a8,0x53c,0x5b5,0x0fb,0x585,0x478,0x179,0x4ac,0x137,0x173,0x1e5,0x7c6,0x262,0x027,0x6d8,0x144,
	0x3e4,0x673,0x7b7,0x145,0x6b7,0x596,0x1c6,0x54b,0x110,0x155,0x64e,0x4f6,0x44b,0x7fa,0x422,0x5a4,
	0x06c,0x51d,0x023,0x0aa,0x459,0x5fc,0x11a,0x2dd,0x4e1,0x603,0x224,0x497,0x308,0x5c9,0x3dd,0x4e8,
	0x48d,0x4cb,0x548,0x36f,0x094,0x36e,0x58a,0x299,0x797,0x57d,0x7c8,0x75f,0x321,0x431,0x045,0x131,
	0x543,0x486,0x1ce,0x47b,0x57f,0x38f,0x775,0x1dd,0x34a,0x177,0x4d8,0x634,0x48c,0x2e5,0x5ee,0x255,
	0x644,0x456,0x653,0x1e9,0x0ca,0x72e,0x576,0x710,0x6ab,0x4aa,0x4fb,0x569,0x541,0x309,0x1d9,0x714,
	0x35d,0x757,0x278,0x7ef,0x2ec,0x668,0x6d7,0x335,0x62d,0x3b6,0x5b4,0x676,0x46c,0x0e2,0x5bc,0x7bd,
	0x07f,0x267,0x68e,0x649,0x721,0x53f,0x226,0x0cd,0x6c2,0x312,0x1b9,0x11d,0x53e,0x602,0x6bd,0x561,
	0x19c,0x62a,0x135,0x651,0x09e,0x3c8,0x237,0x204,0x610,0x798,0x690,0x5a5,0x5f3,0x0f8,0x540,0x614,
	0x14c,0x5d1,0x00f,0x402,0x4b9,0x583,0x5c6,0x21e,0x457,0x3f6,0x07e,0x216,0x4b1,0x369,0x06b,0x37f,
	0x091,0x580,0x61a,0x1cf,0x4b2,0x2c4,0x1bf,0x38c,0x0ff,0x314,0x7d8,0x26f,0x7a9,0x31e,0x2f2,0x7ca,
	0x12d,0x101,0x54e,0x32e,0x791,0x4a1,0x76d,0x6b9,0x34d,0x061,0x69c,0x6bb,0x41d,0x571,0x711,0x339,
	0x5bb,0x1e1,0x680,0x026,0x032,0x23f,0x524,0x329,0x488,0x2c9,0x7f8,0x043,0x0dd,0x5c4,0x589,0x6f1,
	0x4c8,0x00b,0x782,0x670,0x6b8,0x196,0x1d4,0x43d,0x10f,0x56e,0x030,0x165,0x559,0x6b6,0x4ef,0x67d,
	0x15d,0x56f,0x2ed,0x7a2,0x04a,0x4c1,0x7a3,0x2e4,0x111,0x5f8,0x725,0x685,0x3bc,0x1fb,0x5a3,0x178,
	0x293,0x3a6,0x3a9,0x036,0x7ab,0x07d,0x4c9,0x5a1,0x5b7,0x19d,0x13c,0x519,0x0ee,0x157,0x5a8,0x23e,
	0x2af,0x62e,0x25b,0x785,0x73d,0x172,0x3be,0x7ed,0x4e6,0x15e,0x5e7,0x211,0x44d,0x682,0x7db,0x5b6,
	0x40f,0x020,0x0f5,0x7c0,0x609,0x5f2,0x6cf,0x08b,0x256,0x514,0x799,0x5d2,0x1e2,0x727,0x7e5,0x279,
	0x30c,0x11c,0x30a,0x58f,0x0df,0x03c,0x577,0x5ef,0x7c2,0x55a,0x5b1,0x776,0x65d,0x2aa,0x225,0x3db,
	0x276,0x6a5,0x5ec,0x229,0x5ca,0x513,0x1a8,0x30e,0x783,0x3fd,0x124,0x535,0x13f,0x64c,0x1a3,0x607,
	0x02f,0x0e5,0x701,0x24c,0x45c,0x566,0x6d1,0x035,0x044,0x455,0x042,0x20e,0x7e8,0x767,0x2fa,0x476,
	0x003,0x2bf,0x37a,0x5d4,0x247,0x660,0x1f0,0x2d6,0x36d,0x564,0x3c2,0x37b,0x2f4,0x17a,0x4a7,0x176,
	0x3d4,0x59a,0x383,0x258,0x08e,0x098,0x528,0x62c,0x72f,0x35e,0x0b5,0x56d,0x6f0,0x44a,0x723,0x45d,
	0x4eb,0x055,0x285,0x0ed,0x182,0x429,0x393,0x470,0x555,0x3a4,0x078,0x1a2,0x43a,0x16c,0x65e,0x087,
	0x7f3,0x74e,0x273,0x386,0x6da,0x550,0x25f,0x206,0x039,0x2a3,0x2e6,0x199,0x1cb,0x445,0x4c0,0x06a,
	0x54c,0x316,0x0d8,0x741,0x194,0x05c,0x606,0x43e,0x100,0x21a,0x43c,0x4c4,0x693,0x5e1,0x1d3,0x240,
	0x20d,0x433,0x796,0x238,0x765,0x4af,0x0eb,0x7e1,0x050,0x4ee,0x2d5,0x75e,0x5a2,0x5be,0x70a,0x3cf,
	0x376,0x005,0x61e,0x002,0x5cd,0x069,0x0c6,0x041,0x084,0x64b,0x60e,0x268,0x51b,0x000,0x464,0x34b,
	0x615,0x3d7,0x5cc,0x1b0,0x233,0x7fe,0x641,0x53a,0x24d,0x00a,0x32a,0x18f,0x282,0x749,0x3dc,0x0b7,
	0x5b9,0x336,0x0c4,0x5eb,0x398,0x475,0x2d8,0x5bf,0x646,0x418,0x5a0,0x3cc,0x531,0x187,0x3ea,0x6bc,
	0x1ab,0x1e4,0x69d,0x63c,0x6f8,0x0f3,0x251,0x2dc,0x648,0x6a7,0x219,0x731,0x573,0x7f1,0x070,0x188,
	0x7d5,0x436,0x730,0x6de,0x27d,0x163,0x129,0x403,0x298,0x2b9,0x06d,0x357,0x249,0x4b3,0x296,0x2b6,
	0x6b4,0x697,0x770,0x484,0x303,0x7fc,0x1da,0x2d1,0x7f0,0x379,0x16f,0x222,0x48f,0x394,0x64d,0x68d,
	0x3b2,0x5ab,0x3e2,0x5f1,0x7ff,0x69b,0x50c,0x384,0x655,0x346,0x446,0x7ea,0x4f3,0x2fc,0x2b1,0x562,
	0x387,0x2ff,0x4ed,0x5e5,0x5d9,0x6fe,0x1ef,0x656,0x47e,0x171,0x067,0x600,0x6eb,0x246,0x4d2,0x73c,
	0x305,0x1d0,0x434,0x5d8,0x6e7,0x781,0x32c,0x2bb,0x4d4,0x4b6,0x493,0x0c0,0x3bf,0x4f2,0x7ba,0x774,
	0x6ad,0x378,0x7c4,0x63b,0x733,0x752,0x454,0x184,0x74c,0x3c9,0x618,0x688,0x054,0x414,0x6c4,0x07c,
	0x2b7,0x3c7,0x2fe,0x73e,0x351,0x4a4,0x3fa,0x6e4,0x6d9,0x4a8,0x1fe,0x517,0x010,0x4d3,0x6c8,0x153,
	0x180,0x593,0x7b1,0x7a5,0x24b,0x141,0x424,0x7e0,0x186,0x405,0x2ce,0x604,0x425,0x447,0x677,0x79b,
	0x76f,0x0ec,0x2cd,0x358,0x1fc,0x3b7,0x0f0,0x32b,0x2cc,0x138,0x735,0x09a,0x549,0x704,0x720,0x786,
	0x3f2,0x088,0x639,0x133,0x7b0,0x4ae,0x2f9,0x007,0x54d,0x68c,0x509,0x08d,0x67a,0x443,0x0ba,0x499,
	0x06f,0x71d,0x0bc,0x695,0x3b0,0x63e,0x734,0x057,0x033,0x7a1,0x068,0x355,0x49d,0x5ba,0x76c,0x675,
	0x54a,0x10a,0x417,0x3a0,0x658,0x1df,0x7aa,0x112,0x6c1,0x372,0x420,0x048,0x011,0x2df,0x202,0x622,
	0x483,0x5e6,0x28d,0x0a8,0x324,0x380,0x6ea,0x449,0x23a,0x6a2,0x066,0x021,0x6fb,0x1e8,0x48b,0x0d4,
	0x666,0x33f,0x077,0x50f,0x370,0x338,0x76a,0x2fd,0x1d6,0x2a2,0x77e,0x489,0x3ad,0x73f,0x7c5,0x17b,
	0x471,0x5df,0x1aa,0x2f5,0x0db,0x7d6,0x6e0,0x7cc,0x645,0x729,0x08c,0x097,0x790,0x41a,0x4dc,0x7a8,
	0x1bd,0x21f,0x01a,0x3ac,0x713,0x193,0x55b,0x59d,0x1c0,0x6ce,0x364,0x33c,0x4e9,0x57b,0x426,0x1a7,
	0x21c,0x1ed,0x343,0x69a,0x732,0x6ef,0x55c,0x72a,0x18a,0x7f9,0x432,0x29b,0x7b3,0x468,0x319,0x0fe,
	0x245,0x7ec,0x7be,0x24f,0x008,0x500,0x31d,0x33e,0x217,0x6be,0x50d,0x230,0x02d,0x15a,0x46a,0x1ac,
	0x220,0x0ae,0x1d7,0x50a,0x558,0x718,0x40e,0x621,0x594,0x52f,0x27a,0x758,0x29e,0x4d0,0x642,0x75b,
	0x156,0x20b,0x350,0x39d,0x0bb,0x08f,0x218,0x075,0x652,0x57c,0x6d5,0x0ea,0x678,0x052,0x5db,0x1b4,
	0x423,0x3a7,0x4df,0x50b,0x77f,0x49e,0x0c5,0x5ac,0x79e,0x3fb,0x591,0x5fa,0x0c2,0x0e1,0x755,0x67c,
	0x66b,0x60a,0x66f,0x481,0x467,0x7a7,0x61f,0x57a,0x17e,0x05f,0x5f0,0x7f7,0x0d7,0x773,0x05e,0x7e2,
	0x6e5,0x659,0x4e7,0x738,0x24a,0x12c,0x77c,0x4bf,0x45a,0x69f,0x7fd,0x69e,0x529,0x19f,0x2ba,0x0b1,
	0x627,0x412,0x65b,0x2f6,0x269,0x342,0x2b3,0x291,0x0fc,0x3da,0x12f,0x4e3,0x592,0x0fa,0x463,0x7bc,
	0x780,0x11f,0x4ff,0x7ac,0x28f,0x175,0x0ef,0x6d6,0x4ea,0x527,0x3ee,0x0f7,0x0b9,0x311,0x27c,0x025,
	0x2c3,0x71a,0x6af,0x190,0x11e,0x588,0x78e,0x6c6,0x147,0x3ae,0x578,0x6ac,0x4fc,0x252,0x7c7,0x43b,
	0x557,0x7e3,0x2d0,0x63d,0x79c,0x62b,0x640,0x123,0x6e3,0x306,0x77a,0x4ab,0x787,0x421,0x1dc,0x390,
	0x1ae,0x1f3,0x705,0x7d2,0x081,0x2e1,0x5e9,0x5fe,0x687,0x2a1,0x59c,0x0a5,0x223,0x78d,0x35f,0x7c3,
	0x326,0x575,0x348,0x715,0x6b3,0x793,0x6f4,0x7b8,0x064,0x210,0x2c0,0x579,0x022,0x3af,0x74a,0x200,
	0x1a9,0x75d,0x7ce,0x72c,0x79f,0x75c,0x09d,0x4dd,0x2e2,0x7e6,0x17f,0x716,0x570,0x7de,0x017,0x76e,
	0x54f,0x362,0x29a,0x522,0x1b3,0x22a,0x572,0x3d1,0x408,0x437,0x3cb,0x6dc,0x58c,0x6a6,0x37d,0x53b,
	0x1b6,0x7bf,0x5e8,0x143,0x289,0x392,0x22b,0x5fb,0x5ad,0x01d,0x2c1,0x26c,0x76b,0x70d,0x44f,0x6fc,
	0x79d,0x4ca,0x450,0x34f,0x3e7,0x1c2,0x664,0x3d0,0x001,0x7ad,0x6f3,0x359,0x544,0x67f,0x563,0x6d4,
	0x108,0x48e,0x3f0,0x04f,0x322,0x6cd,0x2d4,0x597,0x09f,0x62f,0x5e4,0x28b,0x681,0x231,0x630,0x315,
	0x68b,0x059,0x3ce,0x36b,0x654,0x47a,0x29c,0x6bf,0x071,0x6cc,0x2a7,0x333,0x304,0x1f8,0x0e7,0x599,
	0x174,0x060,0x674,0x6ec,0x7bb,0x288,0x6ee,0x737,0x34c,0x617,0x4bc,0x5d5,0x78c,0x465,0x126,0x626,
};

static const UINT8 deco74_swap_table[0x800] =
{
	2,7,5,7,1,1,4,4,7,5,6,7,7,3,4,3,0,2,1,3,0,4,7,7,2,5,3,1,4,4,6,7,
	6,7,4,5,1,3,0,3,7,2,5,0,1,5,2,1,4,0,6,1,1,2,4,2,1,0,6,5,2,6,4,6,
	4,4,2,2,0,6,0,4,6,0,1,7,0,6,1,2,5,0,1,5,0,2,6,2,2,7,0,7,2,4,6,3,
	3,5,1,1,2,3,4,1,4,6,1,4,0,6,6,3,5,4,1,1,3,0,5,7,5,4,5,4,3,6,2,0,
	4,5,7,3,1,3,4,7,7,4,2,7,1,6,4,1,1,4,0,4,2,0,4,0,1,5,6,4,7,7,3,7,
	7,0,1,2,6,7,7,0,5,1,2,4,2,0,4,4,7,4,0,3,7,1,7,4,2,5,7,4,3,7,3,6,
	7,3,0,5,1,7,5,3,0,1,6,1,7,4,7,6,5,1,2,7,3,3,1,4,6,2,3,7,0,4,5,6,
	3,2,3,1,7,6,0,1,5,7,4,0,5,1,4,1,2,0,6,4,3,5,7,4,3,0,0,6,5,1,7,6,
	0,2,4,0,0,5,4,7,4,3,3,0,2,3,5,7,0,3,2,3,7,2,0,0,7,6,3,5,0,1,3,2,
	4,7,2,3,4,7,1,5,6,7,3,6,4,1,7,7,4,4,3,4,0,7,5,3,2,5,7,7,0,4,0,7,
	7,2,5,1,2,4,7,6,0,6,7,0,5,0,1,6,5,0,2,4,5,1,6,7,6,0,7,0,6,0,1,4,
	3,2,0,2,4,6,3,2,3,3,0,5,7,2,3,7,1,2,6,3,7,2,0,5,7,1,2,6,4,3,4,6,
	4,0,1,4,2,3,3,2,3,7,0,7,4,3,2,1,0,3,5,2,5,3,0,0,0,4,6,4,4,6,1,0,
	3,1,1,5,7,1,5,2,0,1,5,7,2,4,5,4,2,6,3,1,4,0,3,0,1,7,3,5,1,1,1,2,
	2,3,4,1,2,0,0,7,1,6,1,5,2,2,2,1,6,6,0,5,5,4,2,3,7,4,6,6,6,3,1,3,
	3,0,1,7,4,6,3,7,0,1,4,2,3,3,1,7,6,0,6,4,1,3,6,1,6,2,3,1,6,5,3,4,
	2,1,3,3,1,6,6,3,6,6,5,0,1,7,1,0,4,6,0,6,0,0,3,6,5,6,4,0,4,2,6,5,
	0,4,2,7,6,2,4,6,6,4,6,6,0,5,6,6,5,2,3,0,4,4,6,4,1,6,6,0,7,0,1,6,
	1,2,2,4,6,4,2,5,4,2,2,5,4,1,3,3,4,5,3,4,5,6,3,5,2,3,7,6,1,0,4,4,
	7,6,0,5,2,6,2,2,3,7,1,5,4,2,6,7,0,5,5,2,5,7,5,6,2,3,0,5,4,5,2,1,
	0,5,3,6,7,2,3,4,0,3,5,7,6,6,5,6,7,2,2,2,3,5,0,2,0,1,2,2,4,6,7,2,
	5,4,4,3,5,7,3,4,5,3,4,7,1,2,3,5,3,3,6,0,4,4,2,5,4,1,7,2,7,7,4,5,
	2,1,0,3,4,1,3,6,2,4,3,3,3,0,0,1,2,0,3,3,6,5,6,7,5,2,0,4,2,0,1,6,
	7,1,0,4,1,7,5,2,3,0,2,7,2,2,4,6,7,5,7,5,0,6,5,3,2,7,2,3,3,6,0,4,
	1,5,2,2,5,6,4,1,3,6,2,7,4,3,4,1,6,0,5,6,0,0,3,1,1,2,4,3,4,1,4,2,
	0,3,0,7,6,3,2,0,2,3,6,5,3,3,6,6,2,4,5,4,1,5,1,6,0,7,5,5,4,7,3,7,
	4,3,5,0,7,4,4,3,5,5,7,7,2,4,6,6,7,0,7,6,1,0,2,4,0,2,3,5,4,6,3,5,
	2,4,2,4,2,4,5,2,6,5,2,4,6,2,2,2,4,4,2,1,4,0,1,1,1,4,7,0,3,1,7,5,
	7,5,5,4,1,0,2,3,5,3,3,7,2,6,1,2,7,4,1,5,2,4,2,5,4,0,5,0,5,4,1,3,
	5,1,7,4,2,2,1,3,3,7,6,3,2,1,7,5,3,2,4,6,6,5,6,2,0,2,5,3,4,2,3,4,
	0,2,6,3,4,6,6,7,3,0,3,0,1,0,2,7,4,6,0,3,5,2,5,5,5,3,6,4,7,3,5,3,
	7,1,1,3,1,5,7,3,0,4,6,5,3,3,2,3,0,7,0,5,1,1,7,2,2,0,0,6,7,6,7,6,
	3,4,7,3,7,6,6,0,7,6,3,0,0,6,2,7,1,0,2,7,6,2,2,2,7,1,3,2,3,4,1,3,
	1,7,5,2,0,7,7,0,6,1,6,0,2,2,6,7,0,6,7,0,6,4,0,1,6,6,6,0,1,3,4,4,
	4,1,2,2,7,3,7,2,4,4,3,4,0,3,1,0,0,7,3,3,6,5,0,4,3,5,5,0,1,7,2,3,
	3,0,6,6,5,6,4,0,6,0,1,7,5,4,2,6,3,7,4,4,5,6,7,6,1,7,2,5,5,5,1,4,
	3,0,6,3,2,3,0,2,0,0,0,1,0,6,3,7,7,4,3,2,3,4,6,4,2,0,5,3,3,6,4,3,
	5,1,4,0,0,1,0,5,2,1,6,3,3,1,1,6,2,0,3,0,7,0,2,0,5,3,6,6,0,6,2,4,
	0,3,0,1,2,1,6,4,5,2,3,4,1,2,0,6,7,0,5,5,0,0,3,7,1,4,1,2,7,3,4,7,
	4,4,5,5,7,0,4,2,0,7,4,4,7,2,7,2,1,7,4,5,5,7,7,4,0,5,2,2,7,0,5,3,
	3,2,0,5,2,1,1,7,0,5,4,3,1,3,2,5,3,4,5,3,4,6,2,2,2,5,4,2,1,3,1,4,
	7,5,5,7,3,2,0,4,6,0,4,1,6,5,6,1,3,0,1,1,0,4,5,1,5,0,2,3,7,2,5,6,
	0,6,4,2,6,1,5,6,3,3,5,0,3,2,0,0,1,2,1,0,2,7,5,1,6,4,1,7,6,5,3,2,
	3,4,3,2,7,1,6,2,3,2,4,3,4,7,0,5,2,0,6,7,2,0,3,7,6,7,4,0,4,3,7,7,
	5,2,6,6,7,6,3,6,6,1,6,2,3,1,5,1,0,5,1,0,3,7,4,5,0,3,2,1,3,5,3,1,
	1,7,4,2,1,2,3,0,4,7,1,1,6,3,6,4,0,2,7,1,0,5,4,6,1,2,7,1,3,6,5,2,
	3,0,5,7,3,5,5,2,2,6,1,6,0,4,5,7,3,4,0,7,0,5,4,3,6,5,7,0,1,0,0,7,
	3,3,4,4,1,4,7,1,2,0,0,7,0,5,3,4,6,3,4,6,4,2,5,0,7,5,5,6,6,7,6,7,
	3,4,5,0,5,5,7,5,4,4,1,4,3,1,4,3,3,2,5,7,6,3,0,4,1,2,5,4,4,3,0,2,
	7,5,5,7,3,7,3,3,7,0,0,5,3,1,4,4,6,0,1,3,1,4,7,2,1,1,4,1,0,7,4,5,
	2,7,2,4,5,7,3,3,7,6,4,4,5,3,2,4,0,3,5,5,5,4,3,6,7,7,5,1,3,0,2,6,
	3,2,0,4,6,7,4,6,7,2,0,6,7,2,5,0,1,3,0,2,4,7,5,1,4,7,7,4,2,3,6,4,
	3,2,0,4,0,6,0,0,5,7,0,4,0,4,6,3,6,0,0,3,1,0,1,2,6,2,1,0,5,7,6,4,
	5,4,1,2,3,6,6,1,3,1,1,2,4,3,1,4,1,0,4,2,0,3,2,1,1,1,1,6,2,1,3,0,
	6,6,2,5,5,5,2,6,7,6,1,1,3,1,1,6,0,2,6,0,3,6,0,6,3,4,3,3,0,2,0,1,
	5,6,5,0,5,3,4,2,7,5,6,4,4,1,1,4,7,7,1,2,3,6,1,6,4,5,3,6,5,1,7,1,
	0,6,6,2,6,2,6,0,2,2,6,1,0,2,5,6,6,7,4,4,7,6,1,0,1,4,3,1,2,1,4,1,
	4,6,2,5,1,2,0,2,3,4,3,0,3,7,7,6,5,1,7,3,1,0,6,1,1,3,0,7,4,3,1,3,
	7,6,1,5,4,3,5,4,7,0,2,5,4,5,1,6,3,7,4,5,6,7,3,6,1,2,3,3,6,6,1,5,
	2,0,7,2,5,4,1,7,6,6,0,1,7,5,2,4,1,2,0,2,2,2,7,4,3,0,6,0,7,2,1,7,
	5,1,7,1,2,2,4,3,1,3,6,5,5,0,2,6,2,6,0,1,4,1,1,4,3,2,6,0,6,3,7,5,
	6,0,7,7,7,2,0,4,2,5,6,3,4,3,2,0,3,1,6,2,3,3,6,7,1,5,6,6,4,0,6,1,
	1,6,2,0,7,6,3,2,5,6,0,4,2,4,4,2,5,7,5,4,4,1,6,3,4,6,5,5,6,0,0,4,
	4,7,2,2,1,3,4,4,1,7,0,2,5,4,7,3,7,6,1,5,6,0,7,4,1,1,5,2,2,6,7,2,
};

static void deco_decrypt(UINT8 *src, INT32 len, const UINT8 *xor_table,const UINT16 *address_table,const UINT8 *swap_table,INT32 remap_only)
{
	UINT16 *rom = (UINT16*)src;
	len/=2;
	UINT16 *buffer = (UINT16*)BurnMalloc(len * sizeof(INT16));
	INT32 i;

#if 0
	/* we work on 16-bit words but data is loaded as 8-bit, so swap bytes on LSB machines */
	if (ENDIANNESS_NATIVE == ENDIANNESS_LITTLE)
		for (i = 0;i < len;i++)
			rom[i] = BIG_ENDIANIZE_INT16(rom[i]);
#else
#ifdef LSB_FIRST
	for (i = 0; i < len; i++) {
		rom[i] = (rom[i] << 8) | (rom[i] >> 8);
	}
#endif
#endif

	memcpy(buffer,rom,len*2);

	for (i = 0;i < len;i++)
	{
		INT32 addr = (i & ~0x7ff) | address_table[i & 0x7ff];
		INT32 pat = swap_table[i & 0x7ff];

		if (remap_only)
			rom[i] = buffer[addr];
		else
			rom[i] = BITSWAP16(buffer[addr] ^ xor_masks[xor_table[addr & 0x7ff]],
						swap_patterns[pat][0],
						swap_patterns[pat][1],
						swap_patterns[pat][2],
						swap_patterns[pat][3],
						swap_patterns[pat][4],
						swap_patterns[pat][5],
						swap_patterns[pat][6],
						swap_patterns[pat][7],
						swap_patterns[pat][8],
						swap_patterns[pat][9],
						swap_patterns[pat][10],
						swap_patterns[pat][11],
						swap_patterns[pat][12],
						swap_patterns[pat][13],
						swap_patterns[pat][14],
						swap_patterns[pat][15]);
	}

	BurnFree(buffer);

#if 0
	/* we work on 16-bit words but data is loaded as 8-bit, so swap bytes on LSB machines */
	if (ENDIANNESS_NATIVE == ENDIANNESS_LITTLE)
		for (i = 0;i < len;i++)
			rom[i] = BIG_ENDIANIZE_INT16(rom[i]);
#else
#ifdef LSB_FIRST
	for (i = 0; i < len; i++) {
		rom[i] = (rom[i] << 8) | (rom[i] >> 8);
	}
#endif
#endif
}

void deco56_decrypt_gfx(UINT8 *rom, INT32 len)
{
	deco_decrypt(rom,len,deco56_xor_table,deco56_address_table,deco56_swap_table, 0);
}

void deco74_decrypt_gfx(UINT8 *rom, INT32 len)
{
	deco_decrypt(rom,len,deco74_xor_table,deco74_address_table,deco74_swap_table, 0);
}

void deco56_remap_gfx(UINT8 *rom, INT32 len)
{
	// Apply address remap, but not XOR/shift
	deco_decrypt(rom,len,deco56_xor_table,deco56_address_table,deco56_swap_table, 1);
}

static UINT16 decrypt(UINT16 data, INT32 address, INT32 select_xor)
{
	static const UINT16 xors[16] =
	{
		0xb52c,0x2458,0x139a,0xc998,0xce8e,0x5144,0x0429,0xaad4,0xa331,0x3645,0x69a3,0xac64,0x1a53,0x5083,0x4dea,0xd237
	};
	static const UINT8 bitswaps[16][16] =
	{
		{ 12,8,13,11,14,10,15,9, 3,2,1,0,4,5,6,7 }, { 10,11,14,12,15,13,8,9, 6,7,5,3,0,4,2,1 },
		{ 14,13,15,9,8,12,11,10, 7,4,1,5,6,0,3,2 }, { 15,14,8,9,10,11,13,12, 1,2,7,3,4,6,0,5 },
		{ 10,9,13,14,15,8,12,11, 5,2,1,0,3,4,7,6 }, { 8,9,15,14,10,11,13,12, 0,6,5,4,1,2,3,7 },
		{ 14,8,15,9,10,11,13,12, 4,5,3,0,2,7,6,1 }, { 13,11,12,10,15,9,14,8, 6,0,7,5,1,4,3,2 },
		{ 12,11,13,10,9,8,14,15, 0,2,4,6,7,5,3,1 }, { 15,13,9,8,10,11,12,14, 2,1,0,7,6,5,4,3 },
		{ 13,8,9,10,11,12,15,14, 6,0,1,2,3,7,4,5 }, { 12,11,10,8,9,13,14,15, 6,5,4,0,7,1,2,3 },
		{ 12,15,8,13,9,11,14,10, 6,5,4,3,2,1,0,7 }, { 11,12,13,14,15,8,9,10, 4,5,7,1,6,3,2,0 },
		{ 13,8,12,14,11,15,10,9, 7,6,5,4,3,2,1,0 }, { 15,14,13,12,11,10,9,8, 0,6,7,4,3,2,1,5 }
	};
	INT32 j, xorval;
	const UINT8 *bs;

	// calculate bitswap to use
	j = ((address ^ select_xor) & 0xf0) >> 4;
	if (address & 0x20000) j ^= 4;
	bs = bitswaps[j];

	// calculate xor to use
	j = (address ^ select_xor) & 0x0f;
	if (address & 0x40000) j ^= 2;	// boogwing
	xorval = xors[j];

	// decrypt
	return xorval ^ BITSWAP16(data,
				bs[0],bs[1],bs[2],bs[3],bs[4],bs[5],bs[6],bs[7],
				bs[8],bs[9],bs[10],bs[11],bs[12],bs[13],bs[14],bs[15]);
}

void deco102_decrypt_cpu(UINT8 *data, UINT8 *ops, INT32 size, INT32 address_xor, INT32 data_select_xor, INT32 opcode_select_xor)
{
	UINT16 *rom	= (UINT16*)data;
	UINT16 *opcodes = (UINT16*)ops;
	UINT16 *buf	= (UINT16*)BurnMalloc(size);

	memcpy(buf, rom, size);

//	memory_set_decrypted_region(space, 0, size - 1, opcodes);
//	m68k_set_encrypted_opcode_range(devtag_get_device(machine, cputag), 0, size);

	for (INT32 i = 0; i < size / 2; i++)
	{
		INT32 src;

		// calculate address of encrypted word in ROM
		src = i & 0xf0000;
		if (i & 0x0001) src ^= 0xbe0b;
		if (i & 0x0002) src ^= 0x5699;
		if (i & 0x0004) src ^= 0x1322;
		if (i & 0x0008) src ^= 0x0004;
		if (i & 0x0010) src ^= 0x08a0;
		if (i & 0x0020) src ^= 0x0089;
		if (i & 0x0040) src ^= 0x0408;
		if (i & 0x0080) src ^= 0x1212;
		if (i & 0x0100) src ^= 0x08e0;
		if (i & 0x0200) src ^= 0x5499;
		if (i & 0x0400) src ^= 0x9a8b;
		if (i & 0x0800) src ^= 0x1222;
		if (i & 0x1000) src ^= 0x1200;
		if (i & 0x2000) src ^= 0x0008;
		if (i & 0x4000) src ^= 0x1210;
		if (i & 0x8000) src ^= 0x00e0;
		src ^= address_xor;

		rom[i]     = BURN_ENDIAN_SWAP_INT16(decrypt(BURN_ENDIAN_SWAP_INT16(buf[src]), i, data_select_xor));
		opcodes[i] = BURN_ENDIAN_SWAP_INT16(decrypt(BURN_ENDIAN_SWAP_INT16(buf[src]), i, opcode_select_xor));
	}

	BurnFree(buf);
}


static void decrypt156(UINT32 *src, UINT32 *dst, INT32 length)
{
	INT32 a;

	for (a = 0; a < length/4; a++)
	{
		INT32 addr, dword;

		addr = (a & 0xff0000) | 0x92c6;

		if (a & 0x0001) addr ^= 0xce4a;
		if (a & 0x0002) addr ^= 0x4db2;
		if (a & 0x0004) addr ^= 0xef60;
		if (a & 0x0008) addr ^= 0x5737;
		if (a & 0x0010) addr ^= 0x13dc;
		if (a & 0x0020) addr ^= 0x4bd9;
		if (a & 0x0040) addr ^= 0xa209;
		if (a & 0x0080) addr ^= 0xd996;
		if (a & 0x0100) addr ^= 0xa700;
		if (a & 0x0200) addr ^= 0xeca0;
		if (a & 0x0400) addr ^= 0x7529;
		if (a & 0x0800) addr ^= 0x3100;
		if (a & 0x1000) addr ^= 0x33b4;
		if (a & 0x2000) addr ^= 0x6161;
		if (a & 0x4000) addr ^= 0x1eef;
		if (a & 0x8000) addr ^= 0xf5a5;

		dword = BURN_ENDIAN_SWAP_INT32(src[addr]);

		// note that each of the following lines affects exactly two bits

		if (a & 0x00004) dword ^= 0x04400000;
		if (a & 0x00008) dword ^= 0x40000004;
		if (a & 0x00010) dword ^= 0x00048000;
		if (a & 0x00020) dword ^= 0x00000280;
		if (a & 0x00040) dword ^= 0x00200040;
		if (a & 0x00080) dword ^= 0x09000000;
		if (a & 0x00100) dword ^= 0x00001100;
		if (a & 0x00200) dword ^= 0x20002000;
		if (a & 0x00400) dword ^= 0x00000022;
		if (a & 0x00800) dword ^= 0x000a0000;
		if (a & 0x01000) dword ^= 0x10004000;
		if (a & 0x02000) dword ^= 0x00010400;
		if (a & 0x04000) dword ^= 0x80000010;
		if (a & 0x08000) dword ^= 0x00000009;
		if (a & 0x10000) dword ^= 0x02100000;
		if (a & 0x20000) dword ^= 0x00800800;

		switch (a & 3)
		{
			case 0:
				dword = BITSWAP32( dword ^ 0xec63197a,
					 1,	 4,	 7,	28,	22,	18,	20,	 9,
					16,	10,	30,	 2,	31,	24,	19,	29,
					 6,	21,	23,	11,	12,	13,	 5,	 0,
					 8,	26,	27,	15,	14,	17,	25,	 3 );
				break;

			case 1:
				dword = BITSWAP32( dword ^ 0x58a5a55f,
					14,	23,	28,	29,	 6,	24,	10,	 1,
					 5,	16,	 7,	 2,	30,	 8,	18,	 3,
					31,	22,	25,	20,	17,	 0,	19,	27,
					 9,	12,	21,	15,	26,	13,	 4,	11 );
				break;

			case 2:
				dword = BITSWAP32( dword ^ 0xe3a65f16,
					19,	30,	21,	 4,	 2,	18,	15,	 1,
					12,	25,	 8,	 0,	24,	20,	17,	23,
					22,	26,	28,	16,	 9,	27,	 6,	11,
					31,	10,	 3,	13,	14,	 7,	29,	 5 );
				break;

			case 3:
				dword = BITSWAP32( dword ^ 0x28d93783,
					30,	 6,	15,	 0,	31,	18,	26,	22,
					14,	23,	19,	17,	10,	 8,	11,	20,
					 1,	28,	 2,	 4,	 9,	24,	25,	27,
					 7,	21,	13,	29,	 5,	 3,	16,	12 );
				break;
		}

		dst[a] = BURN_ENDIAN_SWAP_INT32(dword);
	}
}

void deco156_decrypt(UINT8 *src, INT32 len)
{
	UINT32 *rom = (UINT32*)src;
	UINT32 *buf = (UINT32*)BurnMalloc(len);

	memcpy (buf, rom, len);

	decrypt156(buf, rom, len);

	BurnFree(buf);
}



//---------------------------------------------------------------------------------------------------
// protection code 
//  should probably be in own file

UINT16 *deco16_prot_ram;
UINT16 *deco16_prot_inputs;
UINT16 *deco16_buffer_ram;

#define DECO_PORT(p) (prot_ram[p/2])

static INT32 deco16_buffer_ram_selected=0;
static INT32 deco16_xor=0;
static INT32 deco16_mask=0xffff;
static INT32 decoprot_last_write=0;
static INT32 decoprot_last_write_val=0;

static INT32 mutantf_port_0e_hack=0, mutantf_port_6a_hack=0,mutantf_port_e8_hack=0;

void deco16ProtReset()
{
	deco16_buffer_ram_selected = 0;
	deco16_xor = 0;
	deco16_mask = 0xffff;
	decoprot_last_write = 0;
	decoprot_last_write_val = 0;
	mutantf_port_0e_hack = 0;
	mutantf_port_6a_hack = 0;
	mutantf_port_e8_hack = 0;
}


void deco16ProtScan()
{
	SCAN_VAR(deco16_buffer_ram_selected);
	SCAN_VAR(deco16_xor);
	SCAN_VAR(deco16_mask);
	SCAN_VAR(decoprot_last_write);
	SCAN_VAR(deco16_vblank);
	SCAN_VAR(decoprot_last_write_val);
	SCAN_VAR(mutantf_port_0e_hack);
	SCAN_VAR(mutantf_port_6a_hack);
	SCAN_VAR(mutantf_port_e8_hack);
}

void deco16_66_prot_w(INT32 offset, UINT16 data, INT32 mask) /* Mutant Fighter */
{
	offset = (offset & 0x7ff) / 2;

	if (mask == 0xffff) {
		deco16_prot_ram[offset] = data;
	} else if (mask == 0xff00) {
		deco16_prot_ram[offset] = (deco16_prot_ram[offset] & mask) | (data & ~mask);
	} else {
		deco16_prot_ram[offset] = (deco16_prot_ram[offset] & mask) | ((data << 8) & ~mask);
	}

#if 0
	if (offset == (0x64 / 2))
	{
		soundlatch = data;
	//	cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE); // huc6280
		return;
	}
#endif

	/* See below */
	if (offset==(0xe/2))
		mutantf_port_0e_hack=data;
	else
		mutantf_port_0e_hack=0x800;

	if (offset==(0x6a/2))
		mutantf_port_6a_hack=data;
	else
		mutantf_port_6a_hack=0x2866;

	if (offset==(0xe8/2))
		mutantf_port_e8_hack=data;
	else
		mutantf_port_e8_hack=0x2401;
}

UINT16 deco16_66_prot_r(INT32 offset) /* Mutant Fighter */
{
	offset = (offset & 0x7ff) / 2;

	if (offset!=0xe/2)
		mutantf_port_0e_hack=0x0800;
	if (offset!=0x6a/2)
		mutantf_port_6a_hack=0x2866;

	switch (offset*2) {
		case 0xac: /* Dip switches */
			return deco16_prot_inputs[2];
		case 0xc2: /* Dip switches */
			return (deco16_prot_inputs[2]) ^ deco16_prot_ram[0x2c/2];
		case 0x46: /* Coins */
			return ((deco16_prot_inputs[1] & 0x07) | (deco16_vblank & 0x08)) ^ deco16_prot_ram[0x2c/2];
		case 0x50: /* Player 1 & 2 input ports */
			return deco16_prot_inputs[0];
		case 0x63c: /* Player 1 & 2 input ports */
			return deco16_prot_inputs[0] ^ deco16_prot_ram[0x2c/2];

		case 0x5f4:
			return deco16_prot_ram[0x18/2];
		case 0x7e8:
			return deco16_prot_ram[0x58/2];
		case 0x1c8:
			return deco16_prot_ram[0x6a/2];
		case 0x10:
			return deco16_prot_ram[0xc/2];
		case 0x672:
			return deco16_prot_ram[0x72/2];
		case 0x5ea:
			return deco16_prot_ram[0xb8/2];
		case 0x1e8:
			return deco16_prot_ram[0x2/2];
		case 0xf6:
			return deco16_prot_ram[0x42/2];
		case 0x692:
			return deco16_prot_ram[0x2e/2];
		case 0x63a:
			return deco16_prot_ram[0x88/2];
		case 0x7a:
			return deco16_prot_ram[0xe/2];
		case 0x40e:
			return deco16_prot_ram[0x7a/2];
		case 0x602:
			return deco16_prot_ram[0x92/2];
		case 0x5d4:
			return deco16_prot_ram[0x34/2];
		case 0x6fa:
			return deco16_prot_ram[0x4/2];
		case 0x3dc:
			return deco16_prot_ram[0xaa/2];
		case 0x444:
			return deco16_prot_ram[0xb0/2];
		case 0x102:
			return deco16_prot_ram[0xa2/2];
		case 0x458:
			return deco16_prot_ram[0xb6/2];
		case 0x2a6:
			return deco16_prot_ram[0xe8/2];
		case 0x626:
			return deco16_prot_ram[0xf4/2];
		case 0x762:
			return deco16_prot_ram[0x82/2];
		case 0x308:
			return deco16_prot_ram[0x38/2];
		case 0x1e6:
			return deco16_prot_ram[0x1e/2];
		case 0x566:
			return deco16_prot_ram[0xa4/2];
		case 0x5b6:
			return deco16_prot_ram[0xe4/2];
		case 0x77c:
			return deco16_prot_ram[0xfa/2];
		case 0x4ba:
			return deco16_prot_ram[0xdc/2];

		case 0x1e:
			return deco16_prot_ram[0xf4/2] ^ deco16_prot_ram[0x2c/2];
		case 0x18e:
			return ((deco16_prot_ram[0x1e/2]&0x000f)<<12) | ((deco16_prot_ram[0x1e/2]&0x0ff0)>>0) | ((deco16_prot_ram[0x1e/2]&0xf000)>>12);
		case 0x636:
			return ((deco16_prot_ram[0x18/2]&0x00ff)<<8) | ((deco16_prot_ram[0x18/2]&0x0f00)>>4) | ((deco16_prot_ram[0x18/2]&0xf000)>>12);
		case 0x7d4:
			return ((deco16_prot_ram[0xc/2]&0x0ff0)<<4) | ((deco16_prot_ram[0xc/2]&0x000c)<<2) | ((deco16_prot_ram[0xc/2]&0x0003)<<6);
		case 0x542:
			return ((deco16_prot_ram[0x92/2]&0x00ff)<<8) ^ deco16_prot_ram[0x2c/2];
		case 0xb0:
			return (((deco16_prot_ram[0xc/2]&0x000f)<<12) | ((deco16_prot_ram[0xc/2]&0x00f0)<<4) | ((deco16_prot_ram[0xc/2]&0xff00)>>8)) ^ deco16_prot_ram[0x2c/2];
		case 0x4:
			return (((deco16_prot_ram[0x18/2]&0x00f0)<<8) | ((deco16_prot_ram[0x18/2]&0x0003)<<10) | ((deco16_prot_ram[0x18/2]&0x000c)<<6)) & (~deco16_prot_ram[0x36/2]);

		case 0xe: /* On real hardware this value only seems to persist for 1 read or write, then reverts to 0800.  Hmm */
			{
				INT32 ret=mutantf_port_0e_hack;
				mutantf_port_0e_hack=0x800;
				return ret;
			}

		case 0x6a: /* On real hardware this value only seems to persist for 1 read or write, then reverts to 0x2866.  Hmm */
			{
				INT32 ret=mutantf_port_6a_hack;
				mutantf_port_6a_hack=0x2866;
				return ret;
			}

		case 0xe8: /* On real hardware this value only seems to persist for 1 read or write, then reverts to 0x2401.  Hmm */
			{
				INT32 ret=mutantf_port_e8_hack;
				mutantf_port_e8_hack=0x2401;
				return ret;
			}

		case 0xaa: /* ??? */
			return 0xc080;

		case 0x42: /* Strange, but consistent */
			return deco16_prot_ram[0x2c/2]^0x5302;

		case 0x48: /* Correct for test data, but I wonder if the 0x1800 is from an address, not a constant */
			return (0x1800) & (~deco16_prot_ram[0x36/2]);

		case 0x52:
			return (0x2188) & (~deco16_prot_ram[0x36/2]);

		case 0x82:
			return ((0x0022 ^ deco16_prot_ram[0x2c/2])) & (~deco16_prot_ram[0x36/2]);

		case 0xc:
			return 0x2000;
	}

	return 0;
}

void deco16_60_prot_w(INT32 offset, UINT16 data, INT32 mask)
{
	deco16_66_prot_w(offset, data, mask);
}

UINT16 deco16_60_prot_r(INT32 offset) /* Edward Randy */
{
	offset = (offset & 0x7ff)/2;

	switch (offset<<1) {
		/* Video registers */
		case 0x32a: /* Moved to 0x140006 on INT32 */
			return deco16_prot_ram[0x80/2];
		case 0x380: /* Moved to 0x140008 on INT32 */
			return deco16_prot_ram[0x84/2];
		case 0x63a: /* Moved to 0x150002 on INT32 */
			return deco16_prot_ram[0x88/2];
		case 0x42a: /* Moved to 0x150004 on INT32 */
			return deco16_prot_ram[0x8c/2];
		case 0x030: /* Moved to 0x150006 on INT32 */
			return deco16_prot_ram[0x90/2];
		case 0x6b2: /* Moved to 0x150008 on INT32 */
			return deco16_prot_ram[0x94/2];

		case 0x6fa:
			return deco16_prot_ram[0x4/2];
		case 0xe4:
			return (deco16_prot_ram[0x4/2]&0xf000)|((deco16_prot_ram[0x4/2]&0x00ff)<<4)|((deco16_prot_ram[0x4/2]&0x0f00)>>8);

		case 0x390:
			return deco16_prot_ram[0x2c/2];
		case 0x3b2:
			return deco16_prot_ram[0x3c/2];
		case 0x440:
			return deco16_prot_ram[0x3e/2];

		case 0x6fc:
			return deco16_prot_ram[0x66/2];

		case 0x15a:
			return deco16_prot_ram[0xa0/2];
		case 0x102:
			return deco16_prot_ram[0xa2/2];
		case 0x566:
			return deco16_prot_ram[0xa4/2];
		case 0xd2:
			return deco16_prot_ram[0xa6/2];
		case 0x4a6:
			return deco16_prot_ram[0xa8/2];
		case 0x3dc:
			return deco16_prot_ram[0xaa/2];
		case 0x2a0:
			return deco16_prot_ram[0xac/2];
		case 0x392:
			return deco16_prot_ram[0xae/2];
		case 0x444:
			return deco16_prot_ram[0xb0/2];

		case 0x5ea:
			return deco16_prot_ram[0xb8/2];
		case 0x358:
			return deco16_prot_ram[0xba/2];
		case 0x342:
			return deco16_prot_ram[0xbc/2];
		case 0x3c:
			return deco16_prot_ram[0xbe/2];
		case 0x656:
			return deco16_prot_ram[0xc0/2];
		case 0x18c:
			return deco16_prot_ram[0xc2/2];
		case 0x370:
			return deco16_prot_ram[0xc4/2];
		case 0x5c6:
			return deco16_prot_ram[0xc6/2];

			/* C8 written but not read */

		case 0x248:
			return deco16_prot_ram[0xd0/2];
		case 0x1ea:
			return deco16_prot_ram[0xd2/2];
		case 0x4cc:
			return deco16_prot_ram[0xd4/2];
		case 0x724:
			return deco16_prot_ram[0xd6/2];
		case 0x578:
			return deco16_prot_ram[0xd8/2];
		case 0x63e:
			return deco16_prot_ram[0xda/2];
		case 0x4ba:
			return deco16_prot_ram[0xdc/2];
		case 0x1a:
			return deco16_prot_ram[0xde/2];
		case 0x120:
			return deco16_prot_ram[0xe0/2];
		case 0x7c2: /* (Not checked for mask/xor but seems standard) */
			return deco16_prot_ram[0x50/2];

		/* memcpy selectors, transfer occurs in interrupt */
		case 0x32e: return deco16_prot_ram[4]; /* src msb */
		case 0x6d8: return deco16_prot_ram[5]; /* src lsb */
		case 0x010: return deco16_prot_ram[6]; /* dst msb */
		case 0x07a: return deco16_prot_ram[7]; /* src lsb */

		case 0x37c: return deco16_prot_ram[8]; /* src msb */
		case 0x250: return deco16_prot_ram[9];
		case 0x04e: return deco16_prot_ram[10];
		case 0x5ba: return deco16_prot_ram[11];
		case 0x5f4: return deco16_prot_ram[12]; /* length */

		case 0x38c: return deco16_prot_ram[13]; /* src msb */
		case 0x02c: return deco16_prot_ram[14];
		case 0x1e6: return deco16_prot_ram[15];
		case 0x3e4: return deco16_prot_ram[16];
		case 0x174: return deco16_prot_ram[17]; /* length */

		/* Player 1 & 2 controls, read in IRQ then written *back* to protection device */
		case 0x50: /* written to 9e byte */
			return deco16_prot_inputs[0];
		case 0x6f8: /* written to 76 byte */
			return (deco16_prot_inputs[0]>>8)|(deco16_prot_inputs[0]<<8); /* byte swap IN0 */

		case 0x5c: /* After coin insert, high 0x8000 bit set starts game */
			return deco16_prot_ram[0x3b];
		case 0x3a6: /* Top byte OR'd with above, masked to 7 */
			return deco16_prot_ram[0x9e/2];
		case 0xc6:
			return ((deco16_prot_ram[0x9e/2]&0xff00)>>8) | ((deco16_prot_ram[0x9e/2]&0x00ff)<<8);

		case 0xac: /* Dip switches */
			return deco16_prot_inputs[2];
		case 0xc2:
			return deco16_prot_inputs[2] ^ deco16_prot_ram[0x2c/2];

		case 0x5d4: /* The state of the dips last frame */
			return deco16_prot_ram[0x34/2];

		case 0x7bc:
			return ((deco16_prot_ram[0x76/2]&0xff00)>>8) | ((deco16_prot_ram[0x76/2]&0x00ff)<<8);

		case 0x2f6: /* Stage clear flag */
			return (((deco16_prot_ram[0]&0xfff0)>>0) | ((deco16_prot_ram[0]&0x000c)>>2) | ((deco16_prot_ram[0]&0x0003)<<2)) & (~deco16_prot_ram[0x36/2]);

		case 0x76a: /* Coins */
			return (deco16_prot_inputs[1] & 0x07) | (deco16_vblank & 0x08);

		case 0x284: /* Bit shifting with inverted mask register */
			return (((deco16_prot_ram[0x40/2]&0xfff0)>>0) | ((deco16_prot_ram[0x40/2]&0x0007)<<1) | ((deco16_prot_ram[0x40/2]&0x0008)>>3)) & (~deco16_prot_ram[0x36/2]);
		case 0x6c4: /* Bit shifting with inverted mask register */
			return (((deco16_prot_ram[0x54/2]&0xf000)>>4) | ((deco16_prot_ram[0x54/2]&0x0f00)>>4) | ((deco16_prot_ram[0x54/2]&0x00f0)>>4) | ((deco16_prot_ram[0x54/2]&0x0003)<<14) | ((deco16_prot_ram[0x54/2]&0x000c)<<10)) & (~deco16_prot_ram[0x36/2]);
		case 0x33e: /* Bit shifting with inverted mask register */
			return (((deco16_prot_ram[0x56/2]&0xff00)>>0) | ((deco16_prot_ram[0x56/2]&0x00f0)>>4) | ((deco16_prot_ram[0x56/2]&0x000f)<<4)) & (~deco16_prot_ram[0x36/2]);
		case 0x156: /* Bit shifting with inverted mask register */
			return (((deco16_prot_ram[0x58/2]&0xfff0)>>4) | ((deco16_prot_ram[0x58/2]&0x000e)<<11) | ((deco16_prot_ram[0x58/2]&0x0001)<<15)) & (~deco16_prot_ram[0x36/2]);
		case 0x286: /* Bit shifting with inverted mask register */
			return (((deco16_prot_ram[0x6a/2]&0x00f0)<<4) | ((deco16_prot_ram[0x6a/2]&0x0f00)<<4) | ((deco16_prot_ram[0x6a/2]&0x0007)<<5) | ((deco16_prot_ram[0x6a/2]&0x0008)<<1)) & (~deco16_prot_ram[0x36/2]);

		case 0x7d6: /* XOR IN0 */
			return deco16_prot_inputs[0] ^ deco16_prot_ram[0x2c/2];
		case 0x4b4:
			return ((deco16_prot_ram[0x32/2]&0x00f0)<<8) | ((deco16_prot_ram[0x32/2]&0x000e)<<7) | ((deco16_prot_ram[0x32/2]&0x0001)<<11);
	}

	return 0;
}

UINT16 deco16_104_cninja_prot_r(INT32 offset)
{
	switch (offset & 0x3fe) {
		case 0x80: /* Master level control */
			return deco16_prot_ram[0];

		case 0xde: /* Restart position control */
			return deco16_prot_ram[1];

		case 0xe6: /* The number of credits in the system. */
			return deco16_prot_ram[2];

		case 0x86: /* End of game check.  See 0x1814 */
			return deco16_prot_ram[3];

		/* Video registers */
		case 0x5a: /* Moved to 0x140000 on INT32 */
			return deco16_prot_ram[8];
		case 0x84: /* Moved to 0x14000a on INT32 */
			return deco16_prot_ram[9];
		case 0x20: /* Moved to 0x14000c on INT32 */
			return deco16_prot_ram[10];
		case 0x72: /* Moved to 0x14000e on INT32 */
			return deco16_prot_ram[11];
		case 0xdc: /* Moved to 0x150000 on INT32 */
			return deco16_prot_ram[12];
		case 0x6e: /* Moved to 0x15000a on INT32 */
			return deco16_prot_ram[13]; /* Not used on bootleg */
		case 0x6c: /* Moved to 0x15000c on INT32 */
			return deco16_prot_ram[14];
		case 0x08: /* Moved to 0x15000e on INT32 */
			return deco16_prot_ram[15];

		case 0x36: /* Dip switches */
			return deco16_prot_inputs[2];

		case 0x1c8: /* Coins */
			return (deco16_prot_inputs[1] & 0x07) | (deco16_vblank & 0x08);

		case 0x22c: /* Player 1 & 2 input ports */
			return deco16_prot_inputs[0];
	}

	return ~0;
}

UINT16 deco16_146_funkyjet_prot_r(INT32 offset)
{
	offset = (offset & 0x7fe) / 2;

	switch (offset)
	{
		case 0x00c >> 1:
			return deco16_prot_inputs[0];

		case 0x0be >> 1:
			return deco16_prot_ram[0x106>>1];

		case 0x11e >> 1:
			return deco16_prot_ram[0x500>>1];

		case 0x148 >> 1:
			return deco16_prot_ram[0x70e>>1];

		case 0x192 >>1:
			return ((deco16_prot_ram[0x78e>>1]<<0)&0xf000);

		case 0x1da >> 1:
			return deco16_prot_ram[0x100>>1];

		case 0x21c >> 1:
			return deco16_prot_ram[0x504>>1];

		case 0x226 >> 1:
			return deco16_prot_ram[0x58c>>1];

		case 0x24c >> 1:
			return deco16_prot_ram[0x78e>>1];

		case 0x250 >> 1:
			return deco16_prot_ram[0x304>>1];

		case 0x27c >>1:
			return ((deco16_prot_ram[0x70e>>1]>>4)&0x0fff) | ((deco16_prot_ram[0x70e>>1]&0x0001)<<15) | ((deco16_prot_ram[0x70e>>1]&0x000e)<<11);

		case 0x2d4 >> 1:
			return deco16_prot_ram[0x102>>1];

		case 0x2d8 >> 1:
			return deco16_prot_ram[0x502>>1];

		case 0x382 >> 1:
			return deco16_prot_inputs[2];

		case 0x3a6 >> 1:
			return deco16_prot_ram[0x104>>1];

		case 0x3a8 >> 1:
			return deco16_prot_ram[0x500>>1];

		case 0x3e8 >> 1:
			return (deco16_prot_ram[0x50c>>1] >> 8) ^ 0xffff;

		case 0x4e4 >> 1:
			return deco16_prot_ram[0x702>>1];

		case 0x562 >> 1:
			return deco16_prot_ram[0x18e>>1];

		case 0x56c >> 1:
			return deco16_prot_ram[0x50c>>1];

		case 0x5be >> 1:
			return ((deco16_prot_ram[0x70e>>1]<<4)&0xff00) | (deco16_prot_ram[0x70e>>1]&0x000f);

		case 0x5ca >> 1:
			return ((deco16_prot_ram[0x78e>>1]>>4)&0xff00) | (deco16_prot_ram[0x78e>>1]&0x000f) | ((deco16_prot_ram[0x78e>>1]<<8)&0xf000);

		case 0x688 >> 1:
			return deco16_prot_ram[0x300>>1];

		case 0x778 >> 1: {
			return (deco16_prot_inputs[1] & 0x07) | (deco16_vblank & 0x08);
		}

		case 0x788 >> 1:
			return deco16_prot_ram[0x700>>1];

		case 0x7d4 >> 1:
			return 0x10;
	}

	return ~0;
}




void deco16_104_rohga_prot_w(INT32 offset, UINT16 data, INT32 mask)
{
	offset = (offset & 0x7ff) / 2;

	if (deco16_buffer_ram_selected) {
		if (mask == 0xffff) {
			deco16_buffer_ram[offset] = data;
		} else if (mask == 0xff00) {
			deco16_buffer_ram[offset] = (deco16_buffer_ram[offset] & mask) | (data & ~mask);
		} else {
			deco16_buffer_ram[offset] = (deco16_buffer_ram[offset] & mask) | ((data << 8) & ~mask);
		}
	} else {
		if (mask == 0xffff) {
			deco16_prot_ram[offset] = data;
		} else if (mask == 0xff00) {
			deco16_prot_ram[offset] = (deco16_prot_ram[offset] & mask) | (data & ~mask);
		} else {
			deco16_prot_ram[offset] = (deco16_prot_ram[offset] & mask) | ((data << 8) & ~mask);
		}
	}

	if (offset==0x42/2)
		deco16_xor = data;

	if (offset==0xee/2)
		deco16_mask = data;
}

UINT16 deco16_104_rohga_prot_r(INT32 offset)
{
	offset = (offset & 0x7ff) / 2;

	const UINT16 * prot_ram=deco16_buffer_ram_selected ? deco16_buffer_ram : deco16_prot_ram;

	switch (offset) {
		case 0x88/2: /* Player 1 & 2 input ports */
			return deco16_prot_inputs[0];
		case 0x36c/2:
			return (deco16_prot_inputs[1] & 0x7)|(deco16_vblank & 0x08);
		case 0x44c/2:
			return ((deco16_prot_inputs[1] & 0x7)<<13)|((deco16_vblank & 0x8)<<9);
		case 0x292/2: /* Dips */
			return deco16_prot_inputs[2];

		case 0x44/2:
			return ((((DECO_PORT(0x2c)&0x000f)<<12)) ^ deco16_xor) & (~deco16_mask);
		case 0x282/2:
			return ((DECO_PORT(0x26)&0x000f)<<12) & (~deco16_mask);
		case 0xd4/2:
			return ((DECO_PORT(0x6e)&0x0ff0)<<4) | ((DECO_PORT(0x6e)&0x000e)<<3) | ((DECO_PORT(0x6e)&0x0001)<<7);
		case 0x5a2/2:
			return (((DECO_PORT(0x24)&0xff00)>>4) | ((DECO_PORT(0x24)&0x000f)<<0) | ((DECO_PORT(0x24)&0x00f0)<<8)) & (~deco16_mask);
		case 0x570/2:
			return (((DECO_PORT(0x24)&0xf0f0)>>0) | ((DECO_PORT(0x24)&0x000f)<<8)) ^ deco16_xor;
		case 0x32e/2:
			return (((DECO_PORT(0x46)&0xf000)>>0) | ((DECO_PORT(0x46)&0x00ff)<<4)) & (~deco16_mask);
		case 0x4dc/2:
			return ((DECO_PORT(0x62)&0x00ff)<<8);
		case 0x1be/2:
			return ((((DECO_PORT(0xc2)&0x0ff0)<<4) | ((DECO_PORT(0xc2)&0x0003)<<6) | ((DECO_PORT(0xc2)&0x000c)<<2)) ^ deco16_xor) & (~deco16_mask);

		case 0x420/2:
			return ((DECO_PORT(0x2e)&0xf000)>>4) | ((DECO_PORT(0x2e)&0x0f00)<<4) | ((DECO_PORT(0x2e)&0x00f0)>>4) | ((DECO_PORT(0x2e)&0x000f)<<4);

		case 0x390/2:
			return DECO_PORT(0x2c);

		case 0x756/2:
			return ((DECO_PORT(0x60)&0xfff0)>>4) | ((DECO_PORT(0x60)&0x0007)<<13) | ((DECO_PORT(0x60)&0x0008)<<9);
		case 0x424/2:
			return ((DECO_PORT(0x60)&0xf000)>>4) | ((DECO_PORT(0x60)&0x0f00)<<4) | ((DECO_PORT(0x60)&0x00f0)>>0) | ((DECO_PORT(0x60)&0x000f)<<0);

		case 0x156/2:
			return (((DECO_PORT(0xde)&0xff00)<<0) | ((DECO_PORT(0xde)&0x000f)<<4) | ((DECO_PORT(0xde)&0x00f0)>>4)) & (~deco16_mask);
		case 0xa8/2:
			return (((DECO_PORT(0xde)&0xff00)>>4) | ((DECO_PORT(0xde)&0x000f)<<0) | ((DECO_PORT(0xde)&0x00f0)<<8)) & (~deco16_mask);
		case 0x64a/2:
			return (((DECO_PORT(0xde)&0xfff0)>>4) | ((DECO_PORT(0xde)&0x000c)<<10) | ((DECO_PORT(0xde)&0x0003)<<14)) & (~deco16_mask);

		case 0x16e/2:
			return DECO_PORT(0x6a);

		case 0x39c/2:
			return (DECO_PORT(0x6a)&0x00ff) | ((DECO_PORT(0x6a)&0xf000)>>4) | ((DECO_PORT(0x6a)&0x0f00)<<4);
		case 0x212/2:
			return (((DECO_PORT(0x6e)&0xff00)>>4) | ((DECO_PORT(0x6e)&0x00f0)<<8) | ((DECO_PORT(0x6e)&0x000f)<<0)) ^ deco16_xor;

		case 0x70a/2:
			return (((DECO_PORT(0xde)&0x00f0)<<8) | ((DECO_PORT(0xde)&0x0007)<<9) | ((DECO_PORT(0xde)&0x0008)<<5)) ^ deco16_xor;

		case 0x7a0/2:
			return (DECO_PORT(0x6e)&0x00ff) | ((DECO_PORT(0x6e)&0xf000)>>4) | ((DECO_PORT(0x6e)&0x0f00)<<4);
		case 0x162/2:
			return DECO_PORT(0x6e);

		case 0x384/2:
			return ((DECO_PORT(0xdc)&0xf000)>>12) | ((DECO_PORT(0xdc)&0x0ff0)<<4) | ((DECO_PORT(0xdc)&0x000c)<<2) | ((DECO_PORT(0xdc)&0x0003)<<6);

		case 0x302/2:
			return DECO_PORT(0x24);
		case 0x334/2:
			return DECO_PORT(0x30);
		case 0x34c/2:
			return DECO_PORT(0x3c);

		case 0x514/2:
			return (((DECO_PORT(0x32)&0x0ff0)<<4) | ((DECO_PORT(0x32)&0x000c)<<2) | ((DECO_PORT(0x32)&0x0003)<<6)) & (~deco16_mask);

		case 0x34e/2:
			return ((DECO_PORT(0xde)&0x0ff0)<<4) | ((DECO_PORT(0xde)&0xf000)>>8) | ((DECO_PORT(0xde)&0x000f)<<0);
		case 0x722/2:
			return (((DECO_PORT(0xdc)&0x0fff)<<4) ^ deco16_xor) & (~deco16_mask);
		case 0x574/2:
			return ((((DECO_PORT(0xdc)&0xfff0)>>0) | ((DECO_PORT(0xdc)&0x0003)<<2) | ((DECO_PORT(0xdc)&0x000c)>>2)) ^ deco16_xor) & (~deco16_mask);

		case 0x5ae/2:
			return DECO_PORT(0xdc);
		case 0x410/2:
			return DECO_PORT(0xde);
		case 0x340/2:
			return ((DECO_PORT(0x90)&0xfff0) | ((DECO_PORT(0x90)&0x7)<<1) | ((DECO_PORT(0x90)&0x8)>>3)) ^ deco16_xor;
		case 0x4a4/2:
			return (((DECO_PORT(0xce)&0x0ff0) | ((DECO_PORT(0xce)&0xf000)>>12) | ((DECO_PORT(0xce)&0x000f)<<12)) ^ deco16_xor) & (~deco16_mask);
		case 0x256/2:
			return ((((DECO_PORT(0xce)&0xf000)>>12) | ((DECO_PORT(0xce)&0x0fff)<<4))) & (~deco16_mask);
		case 0x79a/2:
			return (((DECO_PORT(0xc8)&0xfff0)>>4) | ((DECO_PORT(0xc8)&0x0008)<<9) | ((DECO_PORT(0xc8)&0x0007)<<13)) ^ deco16_xor;

		case 0x65e/2:
			return DECO_PORT(0x9c);
		case 0x79c/2:
			return ((DECO_PORT(0xc6)&0xf000) | ((DECO_PORT(0xc6)&0x00ff)<<4) | ((DECO_PORT(0xc6)&0x0f00)>>8)) & (~deco16_mask);
		case 0x15e/2:
			return (((DECO_PORT(0x98)&0x0ff0)<<4) | ((DECO_PORT(0x98)&0xf000)>>12) | ((DECO_PORT(0x98)&0x0003)<<6) | ((DECO_PORT(0x98)&0x000c)<<2)) ^ deco16_xor;
		case 0x6e4/2:
			return DECO_PORT(0x98);
		case 0x1e/2:
			return ((((DECO_PORT(0xc4)&0xf000)>>4) | ((DECO_PORT(0xc4)&0x0f00)<<4) | ((DECO_PORT(0xc4)&0x00ff)<<0)) ^ deco16_xor) & (~deco16_mask);
		case 0x23a/2:
			return ((((DECO_PORT(0x86)&0xfff0)>>0) | ((DECO_PORT(0x86)&0x0003)<<2) | ((DECO_PORT(0x86)&0x000c)>>2)) ^ deco16_xor);
		case 0x6e/2:
			return ((((DECO_PORT(0x96)&0xf000)>>8) | ((DECO_PORT(0x96)&0x0f0f)<<0) | ((DECO_PORT(0x96)&0x00f0)<<8)) ^ deco16_xor);
		case 0x3a2/2:
			return ((((DECO_PORT(0x94)&0xf000)>>8) | ((DECO_PORT(0x94)&0x0f00)>>8) | ((DECO_PORT(0x94)&0x00f0)<<8) | ((DECO_PORT(0x94)&0x000e)<<7) | ((DECO_PORT(0x94)&0x0001)<<11)) ^ deco16_xor);// & (~deco16_mask);
		case 0x4a6/2:
			return ((DECO_PORT(0x8c)&0xff00)>>0) | ((DECO_PORT(0x8c)&0x00f0)>>4) | ((DECO_PORT(0x8c)&0x000f)<<4);
		case 0x7b0/2:
			return DECO_PORT(0x80);
		case 0x5aa/2:
			return ((((DECO_PORT(0x98)&0x0f00)>>8) | ((DECO_PORT(0x98)&0xf000)>>8) | ((DECO_PORT(0x98)&0x00f0)<<8) | ((DECO_PORT(0x98)&0x000e)<<7) | ((DECO_PORT(0x98)&0x0001)<<11)) ^ deco16_xor) & (~deco16_mask);
		case 0x662/2:
			return DECO_PORT(0x8c);
		case 0x624/2:
			return DECO_PORT(0x9a);
		case 0x2c/2:
			return (((DECO_PORT(0x82)&0x0f0f)>>0) | ((DECO_PORT(0x82)&0xf000)>>8) | ((DECO_PORT(0x82)&0x00f0)<<8)) & (~deco16_mask);

		case 0x1b4/2:
			return ((DECO_PORT(0xcc)&0x00f0)<<4) | ((DECO_PORT(0xcc)&0x000f)<<12);

		case 0x7ce/2:
			return ((DECO_PORT(0x80)&0x000e)<<11) | ((DECO_PORT(0x80)&0x0001)<<15);
		case 0x41a/2:
			return ((((DECO_PORT(0x84)&0x00f0)<<8) | ((DECO_PORT(0x84)&0xf000)>>8) | ((DECO_PORT(0x84)&0x0f00)>>8) | ((DECO_PORT(0x84)&0x0003)<<10) | ((DECO_PORT(0x84)&0x000c)<<6)) ^ deco16_xor);
		case 0x168/2:
			return ((((DECO_PORT(0x84)&0x0ff0)<<4) | ((DECO_PORT(0x84)&0x000e)<<3) | ((DECO_PORT(0x84)&0x0001)<<5))) & (~deco16_mask);
		case 0x314/2:
			return ((((DECO_PORT(0x84)&0x0ff0)<<4) | ((DECO_PORT(0x84)&0x000e)<<3) | ((DECO_PORT(0x84)&0x0001)<<5)));
		case 0x5e2/2:
			return ((((DECO_PORT(0x84)&0x00f0)<<8) | ((DECO_PORT(0x84)&0x000e)<<7) | ((DECO_PORT(0x84)&0x0001)<<9)));
		case 0x72a/2:
			return ((((DECO_PORT(0x86)&0xfff0)>>4) | ((DECO_PORT(0x86)&0x0003)<<14) | ((DECO_PORT(0x86)&0x000c)<<10)) ^ deco16_xor) & (~deco16_mask);
		case 0x178/2:
			return (((DECO_PORT(0x88)&0x00ff)<<8) | ((DECO_PORT(0x88)&0xff00)>>8)) & (~deco16_mask);
		case 0x40e/2:
			return ((((DECO_PORT(0x8a)&0xf000)>>0) | ((DECO_PORT(0x8a)&0x00ff)<<4)) ^ deco16_xor) & (~deco16_mask);
		case 0x248/2:
			return ((((DECO_PORT(0x8c)&0xff00)>>8) | ((DECO_PORT(0x8c)&0x00f0)<<4) | ((DECO_PORT(0x8c)&0x000f)<<12)) ^ deco16_xor) & (~deco16_mask);

		case 0x27e/2:
			return ((((DECO_PORT(0x94)&0x00f0)<<8)) ^ deco16_xor) & (~deco16_mask);

		case 0x22c/2:
			return ((DECO_PORT(0xc4)&0x00f0)<<8);
		case 0x77e/2:
			return ((DECO_PORT(0x62)&0xf000)>>12) | ((DECO_PORT(0x62)&0x0ff0)<<0) | ((DECO_PORT(0x62)&0x000f)<<12);
		case 0xc/2:
			return ((DECO_PORT(0xd6)&0xf000)>>12) | ((DECO_PORT(0xd6)&0x0fff)<<4);

		case 0x90/2:
			return DECO_PORT(0x44);
		case 0x246/2:
			return ((((DECO_PORT(0x48)&0xff00)>>8) | ((DECO_PORT(0x48)&0x00f0)<<8) | ((DECO_PORT(0x48)&0x0f00)>>8) | ((DECO_PORT(0x48)&0x0003)<<10) | ((DECO_PORT(0x48)&0x000c)<<6)) ^ deco16_xor);
		case 0x546/2:
			return (((DECO_PORT(0x62)&0xf0f0)>>0) | ((DECO_PORT(0x62)&0x000f)<<8)) & (~deco16_mask);
		case 0x2e2/2:
			return ((DECO_PORT(0xc6)&0x000e)<<11) | ((DECO_PORT(0xc6)&0x0001)<<15);
		case 0x3c0/2:
			return DECO_PORT(0x22);
		case 0x4b8/2:
			return (((DECO_PORT(0x46)&0xf000)>>12) | ((DECO_PORT(0x46)&0x0f00)>>4) | ((DECO_PORT(0x46)&0x00ff)<<8)) ^ deco16_xor;
		case 0x65c/2:
			return ((((DECO_PORT(0x44)&0xf000)>>12) | ((DECO_PORT(0x44)&0x0fff)<<4)) ^ deco16_xor) & (~deco16_mask);

		case 0x32a/2:
			return ((((DECO_PORT(0xc0)&0x0ff0)<<4) | ((DECO_PORT(0xc0)&0x000e)<<3) | ((DECO_PORT(0xc0)&0x0001)<<7))) & (~deco16_mask);// ^ deco16_xor;
		case 0x8/2:
			return ((((DECO_PORT(0x94)&0xfff0)<<0) | ((DECO_PORT(0x94)&0x000e)>>1) | ((DECO_PORT(0x94)&0x0001)<<3))) & (~deco16_mask);// ^ deco16_xor;
		case 0x456/2:
			return (((DECO_PORT(0x26)&0xfff0)<<0) | ((DECO_PORT(0x26)&0x0007)<<1) | ((DECO_PORT(0x26)&0x0008)>>3));// ^ deco16_xor;
		case 0x190/2:
			return ((((DECO_PORT(0x44)&0xf000)<<0) | ((DECO_PORT(0x44)&0x00ff)<<4))) & (~deco16_mask);// ^ deco16_xor;
		case 0x3f2/2:
			return ((((DECO_PORT(0x48)&0x000f)<<12) | ((DECO_PORT(0x48)&0x00f0)<<4))) & (~deco16_mask);// ^ deco16_xor;
		case 0x2be/2:
			return ((DECO_PORT(0x40)&0x00ff)<<8);

		case 0x19e/2:
			return ((((DECO_PORT(0x3c)&0xf000)>>12) | ((DECO_PORT(0x3c)&0x0f00)<<4) | ((DECO_PORT(0x3c)&0x00f0)>>0) | ((DECO_PORT(0x3c)&0x000f)<<8)) ^ deco16_xor) & (~deco16_mask);
		case 0x2a2/2:
			return ((((DECO_PORT(0x44)&0xff00)>>8) | ((DECO_PORT(0x44)&0x00f0)<<8) | ((DECO_PORT(0x44)&0x000e)<<7) | ((DECO_PORT(0x44)&0x0001)<<11)) ^ deco16_xor) & (~deco16_mask);
		case 0x748/2:
			return (((DECO_PORT(0x44)&0xfff0)<<0) | ((DECO_PORT(0x44)&0x000e)>>1) | ((DECO_PORT(0x44)&0x0001)<<3));// & (~deco16_mask);
		case 0x686/2:
			return (((DECO_PORT(0x46)&0xf000)>>4) | ((DECO_PORT(0x46)&0x0f00)>>8) | ((DECO_PORT(0x46)&0x00f0)<<8) | ((DECO_PORT(0x46)&0x000f)<<4));// & (~deco16_mask);
		case 0x4c4/2:
			return ((DECO_PORT(0x3c)&0x000f)<<12) & (~deco16_mask);
		case 0x538/2:
			return ((DECO_PORT(0x3c)&0x000f)<<12);
		case 0x63a/2:
			return ((DECO_PORT(0x3c)&0x000f)<<12);
		case 0x348/2:
			return ((((DECO_PORT(0x44)&0xf000)>>12) | ((DECO_PORT(0x44)&0x0ff0)<<4) | ((DECO_PORT(0x44)&0x000e)<<3) | ((DECO_PORT(0x44)&0x0001)<<7))) ^ deco16_xor;// & (~deco16_mask);
		case 0x200/2:
			return (((DECO_PORT(0xa0)&0xfff0)>>4) | ((DECO_PORT(0xa0)&0x0007)<<13) | ((DECO_PORT(0xa0)&0x0008)<<9));// & (~deco16_mask);
		case 0x254/2:
			return ((((DECO_PORT(0x7e)&0x0ff0)<<4) | ((DECO_PORT(0x7e)&0x000c)<<2) | ((DECO_PORT(0x7e)&0x0003)<<6))) ^ deco16_xor;// & (~deco16_mask);
		case 0x182/2:
			return ((DECO_PORT(0x46)&0xf000)<<0) | ((DECO_PORT(0x46)&0x0f00)>>8) | ((DECO_PORT(0x46)&0x00f0)>>0) | ((DECO_PORT(0x46)&0x000f)<<8);
		case 0x58/2:
			return DECO_PORT(0x46);
		case 0x48e/2:
			return ((((DECO_PORT(0x46)&0xf000)>>12) | ((DECO_PORT(0x46)&0x0f00)>>4) | ((DECO_PORT(0x46)&0x00f0)<<4) | ((DECO_PORT(0x46)&0x000f)<<12)));// /*^ deco16_xor*/) & (~deco16_mask);

		case 0x4ba/2:
			return (((DECO_PORT(0x24)&0xf000)>>12) | ((DECO_PORT(0x24)&0x0ff0)<<4) | ((DECO_PORT(0x24)&0x000c)<<2) | ((DECO_PORT(0x24)&0x0003)<<6)) & (~deco16_mask);
		case 0x92/2:
			return (((DECO_PORT(0x3c)&0xfff0)>>0) | ((DECO_PORT(0x3c)&0x0007)<<1) | ((DECO_PORT(0x3c)&0x0008)>>3));
		case 0x1f0/2:
			return ((((DECO_PORT(0xa2)&0xf000)>>12) | ((DECO_PORT(0xa2)&0x0f00)>>4) | ((DECO_PORT(0xa2)&0x00ff)<<8)) ^ deco16_xor) & (~deco16_mask);
		case 0x24e/2:
			return ((((DECO_PORT(0x46)&0xf000)>>8) | ((DECO_PORT(0x46)&0x0f00)>>0) | ((DECO_PORT(0x46)&0x00f0)>>4) | ((DECO_PORT(0x46)&0x000f)<<12)) ^ deco16_xor);// & (~deco16_mask);
		case 0x594/2:
			return ((((DECO_PORT(0x40)&0x00f0)<<8) | ((DECO_PORT(0x40)&0x000c)<<6) | ((DECO_PORT(0x40)&0x0003)<<10)) ^ deco16_xor);// & (~deco16_mask);

		case 0x7e2/2:
			return ((((DECO_PORT(0x96)&0xf000)<<0) | ((DECO_PORT(0x96)&0x00f0)<<4) | ((DECO_PORT(0x96)&0x000f)<<4))) ^ deco16_xor;// | ((DECO_PORT(0x96)&0x0001)<<7));// ^ deco16_xor);// & (~deco16_mask);
		case 0x18c/2:
			return (((DECO_PORT(0x22)&0xfff0)>>4) | ((DECO_PORT(0x22)&0x000e)<<11) | ((DECO_PORT(0x22)&0x0001)<<15));// ^ deco16_xor);// & (~deco16_mask);
		case 0x1fa/2:
			return ((((DECO_PORT(0x26)&0xf000)>>8) | ((DECO_PORT(0x26)&0x0f00)<<0) | ((DECO_PORT(0x26)&0x00f0)>>4) | ((DECO_PORT(0x26)&0x000f)<<12))) ^ deco16_xor;// & (~deco16_mask);
		case 0x70e/2:
			return ((((DECO_PORT(0x26)&0x0ff0)<<4) | ((DECO_PORT(0x26)&0x000c)<<2) | ((DECO_PORT(0x26)&0x0003)<<6))) ^ deco16_xor;// & (~deco16_mask);
		case 0x33a/2:
			return DECO_PORT(0x60) & (~deco16_mask);
		case 0x1e2/2:
			return ((DECO_PORT(0xd0)&0xf000)>>12) | ((DECO_PORT(0xd0)&0x0f00)>>4) | ((DECO_PORT(0xd0)&0x00ff)<<8);
		case 0x3f4/2:
			return DECO_PORT(0x6e)<<4;

		case 0x2ae/2:
			return ((DECO_PORT(0x9c)&0xf000)<<0) | ((DECO_PORT(0x9c)&0x0ff0)>>4) | ((DECO_PORT(0x9c)&0x000f)<<8);// & (~deco16_mask);
		case 0x96/2:
			return ((((DECO_PORT(0x22)&0xff00)>>8) | ((DECO_PORT(0x22)&0x00f0)<<8) | ((DECO_PORT(0x22)&0x000e)<<7) | ((DECO_PORT(0x22)&0x0001)<<11)) ^ deco16_xor) & (~deco16_mask);

		case 0x33e/2:
			return (((DECO_PORT(0x0)&0xf000)>>12) | ((DECO_PORT(0x0)&0x0f00)>>4) | ((DECO_PORT(0x0)&0x00f0)<<4) | ((DECO_PORT(0x0)&0x000f)<<12)) & (~deco16_mask);

		case 0x6c4/2: /* Reads from here flip buffers */
			deco16_buffer_ram_selected^=1;
			// Flip occurs AFTER this data has been calculated
			return ((DECO_PORT(0x66)&0xf0f0) | ((DECO_PORT(0x66)&0x000f)<<8)) & (~deco16_mask);
		case 0x700/2: /* Reads from here flip buffers */
			deco16_buffer_ram_selected^=1;
			return (((DECO_PORT(0x66)&0xf000)>>4) | ((DECO_PORT(0x66)&0x00f0)<<8)) ^ deco16_xor;
		case 0x444/2:
			deco16_buffer_ram_selected^=1;
			return ((DECO_PORT(0x66)&0x00f0)<<8) | ((DECO_PORT(0x66)&0x0007)<<9)  | ((DECO_PORT(0x66)&0x0008)<<5);
		case 0x2d0/2:
			deco16_buffer_ram_selected^=1;
			return (((DECO_PORT(0x66)&0xf000)>>4) | ((DECO_PORT(0x66)&0x00f0)<<8)) ^ deco16_xor;
		case 0x2b8/2:
			deco16_buffer_ram_selected^=1;
			return ((DECO_PORT(0x66)&0x00f0)<<8) ^ deco16_xor;
		case 0x294/2:
			deco16_buffer_ram_selected^=1;
			return ((DECO_PORT(0x66)&0x000f)<<12);
		case 0x1e8/2:
			deco16_buffer_ram_selected^=1;
			return 0; // todo

		case 0x49c/2:
			return (((DECO_PORT(0x6c)&0x00f0)<<8) ^ deco16_xor) & (~deco16_mask);

		case 0x44e/2:
			return (((DECO_PORT(0x44)&0x00f0)<<4) | ((DECO_PORT(0x44)&0x000f)<<12)) ^ deco16_xor;
		case 0x3ca/2:
			return (((DECO_PORT(0x1e)&0xfff0)>>4) | ((DECO_PORT(0x1e)&0x0003)<<14) | ((DECO_PORT(0x1e)&0x000c)<<10)) ^ deco16_xor;
		case 0x2ac/2:
			return DECO_PORT(0x1e);
		case 0x3c/2:
			return (((DECO_PORT(0x1e)&0x0003)<<14) | ((DECO_PORT(0x1e)&0x000c)<<10)) & (~deco16_mask);
		case 0x174/2:
			return (((DECO_PORT(0x1e)&0xff00)>>8) | ((DECO_PORT(0x1e)&0x00f0)<<8) | ((DECO_PORT(0x1e)&0x0007)<<9) | ((DECO_PORT(0x1e)&0x0008)<<5)) & (~deco16_mask);
		case 0x34a/2:
			return (((DECO_PORT(0x4)&0xff00)>>0) | ((DECO_PORT(0x4)&0x00f0)>>4) | ((DECO_PORT(0x4)&0x000f)<<4)) & (~deco16_mask);
		case 0x324/2:
			return (((DECO_PORT(0x6)&0xf000)>>12) | ((DECO_PORT(0x6)&0x0ff0)<<4) | ((DECO_PORT(0x6)&0x0007)<<5) | ((DECO_PORT(0x6)&0x0008)<<1));
		case 0x344/2:
			return (((DECO_PORT(0x8)&0xf000)>>8) | ((DECO_PORT(0x8)&0x0f00)>>8) | ((DECO_PORT(0x8)&0x00f0)<<4) | ((DECO_PORT(0x8)&0x000f)<<12));
		case 0x72/2:
			return ((((DECO_PORT(0xa)&0xf000)>>8) | ((DECO_PORT(0xa)&0x0ff0)<<4) | ((DECO_PORT(0xa)&0x000f)>>0))) & (~deco16_mask);
		case 0x36e/2:
			return ((((DECO_PORT(0xc)&0xf000)>>0) | ((DECO_PORT(0xc)&0x0ff0)>>4) | ((DECO_PORT(0xc)&0x000f)<<8))) & (~deco16_mask);

		case 0x590/2:
			return ((((DECO_PORT(0xe)&0xfff0)>>4) | ((DECO_PORT(0xe)&0x000e)<<11) | ((DECO_PORT(0xe)&0x0001)<<15))) ^ deco16_xor;
		case 0x7b6/2:
			return ((((DECO_PORT(0x2)&0xf000)>>8) | ((DECO_PORT(0x2)&0x0ff0)<<4) | ((DECO_PORT(0x2)&0x000f)<<0)) ^ deco16_xor) & (~deco16_mask);
		case 0x588/2:
			return ((((DECO_PORT(0x4)&0xff00)>>4) | ((DECO_PORT(0x4)&0x00f0)<<8) | ((DECO_PORT(0x4)&0x000f)<<0)) ^ deco16_xor) & (~deco16_mask);
		case 0x1f6/2:
			return (((DECO_PORT(0x6)&0xf000)>>12) | ((DECO_PORT(0x6)&0x0ff0)<<4) | ((DECO_PORT(0x6)&0x0007)<<5) | ((DECO_PORT(0x6)&0x0008)<<1)) ^ deco16_xor;
		case 0x4c0/2:
			return (((DECO_PORT(0x8)&0xf000)>>4) | ((DECO_PORT(0x8)&0x0f00)<<4) | ((DECO_PORT(0x8)&0x00f0)>>4) | ((DECO_PORT(0x8)&0x000f)<<4)) & (~deco16_mask);
		case 0x63e/2:
			return ((((DECO_PORT(0xa)&0x0ff0)<<4) | ((DECO_PORT(0xa)&0xf000)>>12) | ((DECO_PORT(0xa)&0x0003)<<6) | ((DECO_PORT(0xa)&0x000c)<<2)));
		case 0x7cc/2:
			return ((((DECO_PORT(0xc)&0xfff0)>>4) | ((DECO_PORT(0xc)&0x000e)<<11) | ((DECO_PORT(0xc)&0x0001)<<15)) ^ deco16_xor) & (~deco16_mask);
		case 0x1bc/2:
			return (((DECO_PORT(0xe)&0xf000)>>12) | ((DECO_PORT(0xe)&0x0f00)>>4) | ((DECO_PORT(0xe)&0x00ff)<<8)) & (~deco16_mask);

		case 0x780/2:
			return DECO_PORT(0xb8);

		case 0x454/2:
			return (((DECO_PORT(0x82)&0xf000)>>8) | ((DECO_PORT(0x82)&0x0f00)>>0) | ((DECO_PORT(0x82)&0x00f0)>>4) | ((DECO_PORT(0x82)&0x000f)<<12)) ^ deco16_xor;
		case 0x53e/2:
			return ((DECO_PORT(0x9e)&0x0003)<<14) | ((DECO_PORT(0x9e)&0x000c)<<10);
		case 0x250/2:
			return (((DECO_PORT(0x62)&0xf0f0)<<0) | ((DECO_PORT(0x62)&0x0f00)>>8)  | ((DECO_PORT(0x62)&0x000f)<<8)) & (~deco16_mask);


		case 0x150/2: /* Shared */
			return DECO_PORT(0x7e);
		case 0x10e/2: /* Schmeizr Robo only */
			return DECO_PORT(0x7c);
		case 0x56a/2: /* Schmeizr Robo only */
			return (((DECO_PORT(0x7c)&0xfff0)>>4) | ((DECO_PORT(0x7c)&0x000e)<<11) | ((DECO_PORT(0x7c)&0x0001)<<15)) & (~deco16_mask);
		case 0x39a/2: /* Schmeizr Robo only */
			return ((((DECO_PORT(0x7e)&0xfff0)>>4) | ((DECO_PORT(0x7e)&0x000e)<<11) | ((DECO_PORT(0x7e)&0x0001)<<15)) ^ deco16_xor) & (~deco16_mask);
		case 0x188/2: /* Schmeizr Robo only */
			return (((deco16_mask&0x0003)<<6) | ((deco16_mask&0x000c)<<2) | ((deco16_mask&0x00f0)<<4) | ((deco16_mask&0x0f00)<<4)) & (~deco16_mask);
		case 0x3cc/2: /* Schmeizr Robo only */
			return deco16_mask;
		case 0x4a/2: /* Schmeizr Robo only */
			return DECO_PORT(0x9e) & (~deco16_mask);
		case 0x7e8/2: /* Schmeizr Robo only */
			return DECO_PORT(0x4a) ^ deco16_xor;
		case 0xfc/2: /* Schmeizr Robo only */
			return DECO_PORT(0x4a);
		case 0x38c/2: /* Schmeizr Robo only */
			return DECO_PORT(0x28);
		case 0x28/2: /* Schmeizr Robo only  */
			return DECO_PORT(0x58);
	}

	return 0;
}

UINT16 deco16_104_prot_r(INT32 offset) /* Wizard Fire */
{
	offset = (offset & 0x7ff)/2;

	switch (offset<<1) {
		case 0x110: /* Player input */
			return deco16_prot_inputs[0];

		case 0x36c: /* Coins */
		case 0x334: /* Probably also, c6, 2c0, 2e0, 4b2, 46a, 4da, rohga is 44c */
			return (deco16_prot_inputs[1] & 0x7) | (deco16_vblank & 0x08);
		case 0x0dc:
			return ((deco16_prot_inputs[1] & 0x7) | (deco16_vblank & 0x08))<<4;

		case 0x494: /* Dips */
			return deco16_prot_inputs[2];

		case 0x244:
			return deco16_prot_ram[0];
		case 0x7cc:
			return ((deco16_prot_ram[0]&0x000f)<<12) | ((deco16_prot_ram[0]&0x00f0)<<4) | ((deco16_prot_ram[0]&0x0f00)>>4) | ((deco16_prot_ram[0]&0xf000)>>12);
		case 0x0c0:
			return (((deco16_prot_ram[0]&0x000e)>>1) | ((deco16_prot_ram[0]&0x0001)<<3))<<12;
		case 0x188:
			return (((deco16_prot_ram[0]&0x000e)>>1) | ((deco16_prot_ram[0]&0x0001)<<3))<<12;
		case 0x65e:
			return (((deco16_prot_ram[0]&0x000c)>>2) | ((deco16_prot_ram[0]&0x0003)<<2))<<12;
		case 0x5ce:
			return ((deco16_prot_ram[0]<<8)&0xf000) | ((deco16_prot_ram[0]&0xe)<<7) | ((deco16_prot_ram[0]&0x1)<<11);
		case 0x61a:
			return (deco16_prot_ram[0]<<8)&0xff00;

		case 0x496:
			return deco16_prot_ram[0x110/2];
		case 0x40a:
			return ((deco16_prot_ram[0x110/2]&0x000f)<<12) | ((deco16_prot_ram[0x110/2]&0x00f0)>>4) | ((deco16_prot_ram[0x110/2]&0x0f00)<<0) | ((deco16_prot_ram[0x110/2]&0xf000)>>8);
		case 0x1e8:
			return ((deco16_prot_ram[0x110/2]&0x00ff)<<8) | ((deco16_prot_ram[0x110/2]&0xff00)>>8);
		case 0x4bc:
			return ((deco16_prot_ram[0x110/2]&0x0ff0)<<4) | ((deco16_prot_ram[0x110/2]&0x0003)<<6) | ((deco16_prot_ram[0x110/2]&0x000c)<<2);
		case 0x46e:
			return ((deco16_prot_ram[0x110/2]&0xfff0)<<0) | ((deco16_prot_ram[0x110/2]&0x0007)<<1) | ((deco16_prot_ram[0x110/2]&0x0008)>>3);
		case 0x264:
			return ((deco16_prot_ram[0x110/2]&0x000f)<<8) | ((deco16_prot_ram[0x110/2]&0x00f0)>>0) | ((deco16_prot_ram[0x110/2]&0x0f00)<<4);
		case 0x172:
			return ((deco16_prot_ram[0x110/2]&0x000f)<<4) | ((deco16_prot_ram[0x110/2]&0x00f0)<<4) | ((deco16_prot_ram[0x110/2]&0xf000)<<0);

		case 0x214:
			return deco16_prot_ram[0x280/2];
		case 0x52e:
			return ((deco16_prot_ram[0x280/2]&0x000f)<<8) | ((deco16_prot_ram[0x280/2]&0x00f0)>>0) | ((deco16_prot_ram[0x280/2]&0x0f00)>>8) | ((deco16_prot_ram[0x280/2]&0xf000)>>0);
		case 0x07a:
			return ((deco16_prot_ram[0x280/2]&0x000f)<<8) | ((deco16_prot_ram[0x280/2]&0x00f0)>>0) | ((deco16_prot_ram[0x280/2]&0x0f00)>>8) | ((deco16_prot_ram[0x280/2]&0xf000)>>0);
		case 0x360:
			return ((deco16_prot_ram[0x280/2]&0x000f)<<8) | ((deco16_prot_ram[0x280/2]&0x00f0)>>0) | ((deco16_prot_ram[0x280/2]&0x0f00)>>8) | ((deco16_prot_ram[0x280/2]&0xf000)>>0);
		case 0x4dc:
			return ((deco16_prot_ram[0x280/2]&0x0ff0)<<4) | ((deco16_prot_ram[0x280/2]&0x0007)<<5) | ((deco16_prot_ram[0x280/2]&0x0008)<<1);
		case 0x3a8:
			return ((deco16_prot_ram[0x280/2]&0x000e)<<3) | ((deco16_prot_ram[0x280/2]&0x0001)<<7) | ((deco16_prot_ram[0x280/2]&0x0ff0)<<4) | ((deco16_prot_ram[0x280/2]&0xf000)>>12);
		case 0x2f6:
			return ((deco16_prot_ram[0x280/2]&0xff00)>>8) | ((deco16_prot_ram[0x280/2]&0x00f0)<<8) | ((deco16_prot_ram[0x280/2]&0x000c)<<6) | ((deco16_prot_ram[0x280/2]&0x0003)<<10);

		case 0x7e4:
			return (deco16_prot_ram[0x290/2]&0x00f0)<<8;

		case 0x536:
			return ((deco16_prot_ram[0x2b0/2]&0x000f)<<8) | ((deco16_prot_ram[0x2b0/2]&0x00f0)<<0) | ((deco16_prot_ram[0x2b0/2]&0x0f00)<<4) | ((deco16_prot_ram[0x2b0/2]&0xf000)>>12);

		case 0x0be:
			return ((deco16_prot_ram[0x370/2]&0x000f)<<4) | ((deco16_prot_ram[0x370/2]&0x00f0)<<4) | ((deco16_prot_ram[0x370/2]&0x0f00)>>8) | ((deco16_prot_ram[0x370/2]&0xf000)>>0);

		case 0x490:
			return (deco16_prot_ram[0x3c0/2]&0xfff0) | ((deco16_prot_ram[0x3c0/2]&0x0007)<<1) | ((deco16_prot_ram[0x3c0/2]&0x0008)>>3);

		case 0x710:
			return (deco16_prot_ram[0x430/2]&0xfff0) | ((deco16_prot_ram[0x430/2]&0x0007)<<1) | ((deco16_prot_ram[0x430/2]&0x0008)>>3);

		case 0x22a:
			return ((deco16_prot_ram[0x5a0/2]&0xff00)>>8) | ((deco16_prot_ram[0x5a0/2]&0x00f0)<<8) | ((deco16_prot_ram[0x5a0/2]&0x0001)<<11) | ((deco16_prot_ram[0x5a0/2]&0x000e)<<7);

		case 0x626:
			return ((deco16_prot_ram[0x5b0/2]&0x000f)<<8) | ((deco16_prot_ram[0x5b0/2]&0x00f0)<<8) | ((deco16_prot_ram[0x5b0/2]&0x0f00)>>4) | ((deco16_prot_ram[0x5b0/2]&0xf000)>>12);

		case 0x444:
			return deco16_prot_ram[0x604/2]; //rohga

		case 0x5ac:
			return ((deco16_prot_ram[0x6e0/2]&0xfff0)>>4) | ((deco16_prot_ram[0x6e0/2]&0x0007)<<13) | ((deco16_prot_ram[0x6e0/2]&0x0008)<<9);

		case 0x650:
			return ((deco16_prot_ram[0x7d0/2]&0xfff0)>>4) | ((deco16_prot_ram[0x7d0/2]&0x000f)<<12);

		case 0x4ac:
			return ((deco16_prot_ram[0x460/2]&0x0007)<<13) | ((deco16_prot_ram[0x460/2]&0x0008)<<9);
	}

	return 0;
}






static void deco16_146_core_prot_w(INT32 offset, INT32 data, INT32 mask)
{
	const INT32 writeport=offset;
	const INT32 sndport=0x260;
	const INT32 xorport=0x340;
	const INT32 maskport=0x6c0;
	if (writeport == sndport)
	{
//		soundlatch_w(space, 0, data & 0xff);
//		cputag_set_input_line(space->machine, "audiocpu", 0, HOLD_LINE);
		return;
	}

	if (writeport==xorport)
		deco16_xor = data; //COMBINE_DATA(&deco16_xor);
	if (writeport==maskport)
		deco16_mask = data; //COMBINE_DATA(&deco16_mask);

	offset >>= 1;

	if (deco16_buffer_ram_selected) {
		if (mask == 0xffff) {
			deco16_buffer_ram[offset] = data;
		} else if (mask == 0xff00) {
			deco16_buffer_ram[offset] = (deco16_buffer_ram[offset] & mask) | (data & ~mask);
		} else {
			deco16_buffer_ram[offset] = (deco16_buffer_ram[offset] & mask) | ((data << 8) & ~mask);
		}
	} else {
		if (mask == 0xffff) {
			deco16_prot_ram[offset] = data;
		} else if (mask == 0xff00) {
			deco16_prot_ram[offset] = (deco16_prot_ram[offset] & mask) | (data & ~mask);
		} else {
			deco16_prot_ram[offset] = (deco16_prot_ram[offset] & mask) | ((data << 8) & ~mask);
		}
	}
}

static UINT16 deco16_146_core_prot_r(INT32 offset)
{
	UINT16 val;
	const UINT16 * prot_ram=deco16_buffer_ram_selected ? deco16_buffer_ram : deco16_prot_ram;

	switch (offset)
	{
	case 0x582: /* Player 1 & Player 2 */
		return deco16_prot_inputs[0];
	case 0x04c: /* Coins/VBL */
		return (deco16_prot_inputs[1] & 0x07) | (deco16_vblank & 0x08);
	case 0x672: /* Dip switches */
		return deco16_prot_inputs[2];

	case 0x13a:
		return ((DECO_PORT(0x190)&0x00f0)<<8) | ((DECO_PORT(0x190)&0x0003)<<10) | ((DECO_PORT(0x190)&0x000c)<<6);

	case 0x53c:
		return ((DECO_PORT(0x30)&0x0ff0)<<4) | ((DECO_PORT(0x30)&0xf000)>>8);

	case 0x6c:
		return ((DECO_PORT(0x370)&0x00ff)<<8);

	case 0xa:
		return ((DECO_PORT(0x310)&0x0fff)<<4);

	case 0x4f6:
		return ((DECO_PORT(0x20)&0x00f0)<<8) | ((DECO_PORT(0x20)&0x0007)<<9) | ((DECO_PORT(0x20)&0x0008)<<5);

	case 0xea:
		return ((DECO_PORT(0x1c0)&0xf000)<<0) | ((DECO_PORT(0x1c0)&0x00ff)<<4);

	case 0x12e:
		return ((DECO_PORT(0x1f0)&0xf000)>>4) | ((DECO_PORT(0x1f0)&0x0f00)<<4) | ((DECO_PORT(0x1f0)&0x00f0)>>4) | ((DECO_PORT(0x1f0)&0x000f)<<4);

	case 0x316:
		return ((DECO_PORT(0x290)&0xf000)>>4) | ((DECO_PORT(0x290)&0x0f00)<<4) | ((DECO_PORT(0x290)&0x00ff)<<0);

	case 0x3c6:
		return ((DECO_PORT(0x170)&0xfff0)<<0) | ((DECO_PORT(0x170)&0x000e)>>1) | ((DECO_PORT(0x170)&0x0001)<<3);

	case 0x4d0:
		return ((DECO_PORT(0x20)&0x00f0)<<8) | ((DECO_PORT(0x20)&0x0007)<<9) | ((DECO_PORT(0x20)&0x0008)<<5);

	case 0x53a:
		return ((DECO_PORT(0x370)&0xffff)<<0);

	case 0x552:
		return ((DECO_PORT(0x240)&0xfff0)<<0) | ((DECO_PORT(0x240)&0x0007)<<1) | ((DECO_PORT(0x240)&0x0008)>>3);

	case 0x54c:
		return ((DECO_PORT(0x2f0)&0x00ff)<<8);

	case 0x5da:
		return ((DECO_PORT(0x130)&0x00f0)<<8) | ((DECO_PORT(0x130)&0x000e)<<7) | ((DECO_PORT(0x130)&0x0001)<<11);

	case 0x6be:
		return ((DECO_PORT(0x150)&0xf000)>>12) | ((DECO_PORT(0x150)&0x0ff0)<<0) | ((DECO_PORT(0x150)&0x000f)<<12);

	case 0x70a:
		return ((DECO_PORT(0x1d0)&0x0ff0)<<4) | ((DECO_PORT(0x1d0)&0x0003)<<6) | ((DECO_PORT(0x1d0)&0x000c)<<2);

	case 0x7e0:
		return ((DECO_PORT(0x2b0)&0xfff0)<<0) | ((DECO_PORT(0x2b0)&0x0003)<<2) | ((DECO_PORT(0x2b0)&0x000c)>>2);

	case 0x1de:
		return ((DECO_PORT(0x1b0)&0x0ff0)<<4) | ((DECO_PORT(0x1b0)&0x000e)<<3) | ((DECO_PORT(0x1b0)&0x0001)<<7);

	/*********************************************************************************/

//  case 0x582: return input_port_read(space->machine, "IN0"); /* IN0 */
//  case 0x672: return input_port_read(space->machine, "IN1"); /* IN1 */
//  case 0x04c: return eeprom_read_bit(devtag_get_device(space->machine, "eeprom"));

	case 0x468:
		val=DECO_PORT(0x570);
		val=((val&0x0003)<<6) | ((val&0x000c)<<2) | ((val&0x00f0)<<4) | ((val&0x0f00)<<4) | ((val&0xf000)>>12);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x1ea:
		val=DECO_PORT(0x570);
		val=((val&0x0003)<<10) | ((val&0x000c)<<6) | ((val&0x00f0)<<8) | ((val&0x0f00)>>8) | ((val&0xf000)>>8);
		return val ^ deco16_xor;

	case 0x7b6:
		val=((DECO_PORT(0))&0xffff);
		val=((val&0x000c)>>2) | ((val&0x0003)<<2) | ((val&0xfff0)<<0);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x01c:
		val=((DECO_PORT(0))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)<<4) | ((val&0x0f00)>>8) | ((val&0xf000)>>8);
		return val ^ deco16_xor;

	case 0x1e0:
		val=((DECO_PORT(0))&0xffff);
		val=((val&0x000e)<<3) | ((val&0x0001)<<7) | ((val&0x00f0)<<4) | ((val&0x0f00)<<4) | ((val&0xf000)>>12);
		return val ^ deco16_xor;

	case 0x1d4:
		val=((DECO_PORT(0))&0xffff);
		val=((val&0x000f)<<0) | ((val&0x00f0)<<4) | ((val&0x0f00)<<4) | ((val&0xf000)>>8);
		return val;

	case 0x0c0:
		val=((DECO_PORT(0x280))&0xffff);
		val=((val&0x000f)<<4) | ((val&0x00f0)>>4) | ((val&0x0f00)<<4) | ((val&0xf000)>>4);
		return val ^ deco16_xor;

	case 0x794:
		val=((DECO_PORT(0x280))&0xffff);
		val=((val&0x0007)<<1) | ((val&0xfff0)>>0) | ((val&0x0008)>>3);
		return val ^ deco16_xor;

	case 0x30:
		val=DECO_PORT(0x5e0);
		val=((val&0x0007)<<13) | ((val&0x0008)<<9); /* Bottom bits are masked out before XOR */
		return val ^ deco16_xor;

	case 0x422:
		val=((DECO_PORT(0x3d0))&0xffff);
		val=((val&0x0007)<<1) | ((val&0xfff0)>>0) | ((val&0x0008)>>3);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x558:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000f)<<0) | ((val&0x00f0)<<8) | ((val&0x0f00)>>0) | ((val&0xf000)>>8);
		return val;

	case 0x3e:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000f)<<4) | ((val&0x00f0)>>4) | ((val&0x0f00)<<4) | ((val&0xf000)>>4);
		return val & (~deco16_mask);

	case 0x328:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000e)<<3) | ((val&0x0001)<<7) | ((val&0x00f0)<<4) | ((val&0xf000)>>12) | ((val&0x0f00)<<4);
		return val ^ deco16_xor;

	case 0x476:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000f)<<0) | ((val&0x00f0)<<8) | ((val&0xff00)>>4);
		return val;

	case 0x50a:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)>>4) | ((val&0x0f00)<<0) | ((val&0xf000)>>8);
		return val;

	case 0x5ae:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)>>4) | ((val&0x0f00)>>0) | ((val&0xf000)>>8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x1ae:
		val=((DECO_PORT(0x3d0))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)<<4);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x7a4:
		val=((DECO_PORT(0x620))&0xffff);
		val=((val&0x000f)<<4) | ((val&0x00f0)<<4) | ((val&0x0f00)>>8) | ((val&0xf000)>>0);
		return val;

	case 0x2c4:
		val=((DECO_PORT(0x410))&0xffff);
		val=((val&0x00ff)<<8) | ((val&0xff00)>>8);
		return val ^ deco16_xor;

	case 0x76: /* Bitshifted XOR, with additional inverse mask on final output */
		val=((DECO_PORT(0x2a0))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)<<4) | ((val&0x0f00)>>8) | ((val&0xf000)>>8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x714: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x2a0))&0xffff);
		val=((val&0x0003)<<14) | ((val&0x000c)<<10) | ((val&0xfff0)>>4);
		return val & (~deco16_mask);

	case 0x642:
		val=((DECO_PORT(0x2a0))&0xffff);
		val=((val&0xf000)>>4) | ((val&0x0f00)>>8)| ((val&0x00f0)<<8) | ((val&0x000f)<<4);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x49a: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x580))&0xffff);
		val=((val&0x000f)<<4) | ((val&0x00f0)>>4) | ((val&0xff00)>>0);
		return val & (~deco16_mask);

	case 0x49c: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x580))&0xffff);
		val=((val&0x000e)<<7) | ((val&0x00f0)<<8) | ((val&0x0001)<<11);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x584: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x580))&0xffff);
		val=((val&0xff00)>>8) | ((val&0x00f0)<<8) | ((val&0x0008)<<5) | ((val&0x0007)<<9);
		return val & (~deco16_mask);

	case 0x614: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x580))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)<<4) | ((val&0x0f00)>>4) | ((val&0xf000)>>12);
		return val & (~deco16_mask);

	case 0x162: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0xe0))&0xffff);
		val=((val&0x0fff)<<4);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x18:
		val=((DECO_PORT(0x230))&0xffff);
		val=((val&0xfff0)>>4) | ((val&0x0007)<<13) | ((val&0x0008)<<9);
		return val ^ deco16_xor;

	case 0x7f6: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x230))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)<<4);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x1a0: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x230))&0xffff);
		val=((val&0xff00)>>8) | ((val&0x00f0)<<8) | ((val&0x0003)<<10) | ((val&0x000c)<<6);
		return val & (~deco16_mask);

	case 0x4f8:
		val=((DECO_PORT(0x2d0))&0xffff);
		val=((val&0x0fff)<<4);
		return val;

	case 0x1d6:
		val=((DECO_PORT(0xa0))&0xffff);
		val=((val&0x0fff)<<4);
		return val ^ deco16_xor;

	case 0x254:
		val=((DECO_PORT(0x320))&0xffff);
		val=((val&0x0f00)<<4) | ((val&0x00f0)<<0) | ((val&0x000f)<<8);
		return val & (~deco16_mask);

	case 0x2ea:
		val=((DECO_PORT(0x320))&0xffff);
		val=((val&0x00ff)<<8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x21e:
		val=((DECO_PORT(0x2f0))&0xffff);
		val=((val&0xfff0)<<0) | ((val&0x0007)<<1) | ((val&0x0008)>>3);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x7b0:
		val=((DECO_PORT(0x2f0))&0xffff);
		val=((val&0xfff0)>>4) | ((val&0x0007)<<13) | ((val&0x0008)<<9);
		return val ^ deco16_xor;

	case 0x7da:
		val=((DECO_PORT(0x2f0))&0xffff);
		val=((val&0xff00)>>8) | ((val&0x000f)<<12) | ((val&0x00f0)<<4);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x280:
		val=((DECO_PORT(0x2d0))&0xffff);
		val=((val&0x000f)<<8) | ((val&0x00f0)<<8) | ((val&0xf000)>>12) | ((val&0x0f00)>>4);
		return val ^ deco16_xor;

	case 0x416:
		val=((DECO_PORT(0x2e0))&0xffff);
		val=((val&0x000f)<<8) | ((val&0x00f0)>>4) | ((val&0xf000)>>0) | ((val&0x0f00)>>4);
		return val;


	case 0xac:
		val=((DECO_PORT(0x350))&0xffff);
		val=((val&0x000f)<<4) | ((val&0x00f0)<<4) | ((val&0xf000)>>0) | ((val&0x0f00)>>8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x2c2:
		val=((DECO_PORT(0x2e0))&0xffff);
		val=((val&0xf000)<<0) | ((val&0x0ff0)>>4) | ((val&0x000f)<<8);
		return val;

	case 0x450:
		val=((DECO_PORT(0x440))&0xffff);
		val=((val&0xff00)>>8) | ((val&0x00f0)<<4) | ((val&0x000f)<<12);
		return val;

	case 0x504:
		val=((DECO_PORT(0x440))&0xffff);
		val=((val&0x000c)<<2) | ((val&0x0003)<<6)| ((val&0x0ff0)<<4);
		return val ^ deco16_xor;

	case 0xfe:
		val=((DECO_PORT(0x440))&0xffff);
		val=((val&0x0fff)<<4);
		return val;

	// 1c0 swap address
	case 0x1c0:
		deco16_buffer_ram_selected^=1;
		return 0;

	case 0x0e2:
		deco16_buffer_ram_selected^=1;
		val=((DECO_PORT(0x6c0))&0xffff);
		return val ^ deco16_xor;

	case 0x444:
		val=((DECO_PORT(0xa0))&0xffff);
		val=((val&0xfff0)>>4) | ((val&0x0007)<<13) | ((val&0x0008)<<9);
		return val & (~deco16_mask);

	case 0x46a:
		val=((DECO_PORT(0x10))&0xffff);
		val=((val&0xff00)>>8) | ((val&0x00f0)<<8)| ((val&0x0007)<<9) | ((val&0x0008)<<5);
		return val;

	case 0x80:
		return DECO_PORT(0xe0);

	case 0xb2:
		val=((DECO_PORT(0x280))&0xffff);
		val=((val&0x00f0)<<8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x250:
		val=((DECO_PORT(0x160))&0xffff);
		val=((val&0xf000)>>12) | ((val&0x0f00)<<4)| ((val&0x00f0)<<4) | ((val&0x000e)<<3) | ((val&0x0001)<<7);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x306:
		val=((DECO_PORT(0x160))&0xffff);
		val=((val&0x00f0)<<8) | ((val&0xf000)>>4);
		return (val ^ deco16_xor);

	case 0x608:
		val=((DECO_PORT(0x160))&0xffff);
		val=((val&0xf000)>>4) | ((val&0x0f00)>>4)| ((val&0x00f0)<<8) | ((val&0x000f)<<0);
		return val & (~deco16_mask);

	case 0x52e:
		val=((DECO_PORT(0x160))&0xffff);
		val=((val&0xf000)>>4) | ((val&0x0f00)<<4)| ((val&0x00f0)<<0) | ((val&0x000f)<<0);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x67a:
		val=((DECO_PORT(0x390))&0xffff);
		val=((val&0xf000)>>8) | ((val&0x0ff0)<<4)| ((val&0x000f)<<0);
		return val;

	case 0x6c2:
		val=((DECO_PORT(0x390))&0xffff);
		val=((val&0x00f0)<<8) | ((val&0x000c)<<6)| ((val&0x0003)<<10);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x3d8:
		val=((DECO_PORT(0x7e0))&0xffff);
		val=((val&0xf000)>>8) | ((val&0x0ff0)<<4)| ((val&0x000f)<<0);
		return val & (~deco16_mask);

	case 0x244:
		val=((DECO_PORT(0x760))&0xffff);
		val=((val&0x0f00)<<4) | ((val&0x00f0)>>0)| ((val&0x000f)<<8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x7e8:
		val=((DECO_PORT(0x390))&0xffff);
		val=((val&0x0f00)<<4) | ((val&0xf000)>>4)| ((val&0x00ff)>>0);
		return (val ^ deco16_xor);

	case 0x276:
		val=((DECO_PORT(0x7e0))&0xffff);
		val=((val&0x00ff)<<8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x540:
		val=((DECO_PORT(0x530))&0xffff);
		val=((val&0x00f0)<<8) | ((val&0x0007)<<9) | ((val&0x0008)<<5);
		return val & (~deco16_mask);

	case 0x5c2:
		val=((DECO_PORT(0x7e0))&0xffff);
		val=((val&0xf000)>>12) | ((val&0x0ff0)<<4)| ((val&0x000c)<<2)| ((val&0x0003)<<6);
		return val;

	case 0x15c:
		val=((DECO_PORT(0x230))&0xffff);
		val=((val&0xff00)<<0) | ((val&0x000f)<<4) | ((val&0x00f0)>>4);
		return (val ^ deco16_xor);

	case 0x2c:
		val=((DECO_PORT(0x390))&0xffff);
		val=((val&0x00ff)<<8);
		return val & (~deco16_mask);
	}

	return 0;
}


void deco16_146_nitroball_prot_w(INT32 offset, UINT16 data, INT32 mask)
{
	offset = BITSWAP16((offset & 0x7fe), 0, 0, 0, 0, 0, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
	deco16_146_core_prot_w(offset, data, mask);
}

UINT16 deco16_146_nitroball_prot_r(INT32 offset)
{
	offset = BITSWAP16((offset & 0x7fe), 0, 0, 0, 0, 0, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
	return deco16_146_core_prot_r(offset);
}


void deco16_146_fghthist_prot_w(INT32 offset, UINT32 data, UINT32 mem_mask)
{
	offset = BITSWAP16(offset << 1, 0, 0, 0, 0, 0, 10, 1, 9, 2, 8, 3, 7, 4, 6, 5, 0);

	decoprot_last_write = offset;
	decoprot_last_write_val = data >> 16;

	deco16_146_core_prot_w(offset, data >> 16, mem_mask >> 16);
}

UINT32 deco16_146_fghthist_prot_r(INT32 offset)
{
	offset = BITSWAP16(offset << 1, 0, 0, 0, 0, 0, 10, 1, 9, 2, 8, 3, 7, 4, 6, 5, 0);

	if (decoprot_last_write==offset)
	{
		decoprot_last_write=-1;
		return (decoprot_last_write_val<<16)|0xffff;
	}

	decoprot_last_write=-1;

	return (deco16_146_core_prot_r(offset)<<16)|0xffff;
}




