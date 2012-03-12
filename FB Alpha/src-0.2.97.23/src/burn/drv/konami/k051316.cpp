#include "tiles_generic.h"
#include "konamiic.h"

static UINT16 *K051316TileMap[3];
static void (*K051316Callback[3])(INT32 *code,INT32 *color,INT32 *flags);
static INT32 K051316Depth[3];
static INT32 K051316TransColor[3];
static UINT8 *K051316Gfx[3];
static UINT8 *K051316GfxExp[3];
static INT32 K051316Mask[3];
static INT32 K051316Offs[3][2];

static UINT8 *K051316Ram[3];
static UINT8 K051316Ctrl[3][16];
static UINT8 K051316Wrap[3];

// Decode 4bpp graphics
static void K051316GfxExpand(UINT8 *src, UINT8 *dst, INT32 len)
{
	for (INT32 i = 0; i < len; i++) {
		INT32 d = src[i];
		dst[i * 2 + 0] = d >> 4;
		dst[i * 2 + 1] = d & 0x0f;
	}
}

void K051316Init(INT32 chip, UINT8 *gfx, UINT8 *gfxexp, INT32 mask, void (*callback)(INT32 *code,INT32 *color,INT32 *flags), INT32 bpp, INT32 transp)
{
	K051316Ram[chip] = (UINT8*)BurnMalloc(0x800);
	K051316TileMap[chip] = (UINT16*)BurnMalloc(((32 * 16) * (32 * 16)) * sizeof(UINT16));

	K051316Callback[chip] = callback;	

	K051316Depth[chip] = bpp;
	K051316Gfx[chip] = gfx;
	K051316GfxExp[chip] = gfxexp;

	K051316Mask[chip] = mask;

	if (bpp == 4) {
		K051316GfxExpand(gfx, gfxexp, mask+1);
	}

	KonamiIC_K051316InUse = 1;

	K051316Offs[chip][0] = K051316Offs[chip][1] = 0;

	K051316TransColor[chip] = transp;
}

void K051316Reset()
{
	for (INT32 i = 0; i < 3; i++)
	{
		if (K051316Ram[i]) {
			memset (K051316Ram[i], 0, 0x800);
		}

		memset (K051316Ctrl[i], 0, 16);

		K051316Wrap[i] = 0;

		if (K051316TileMap[i]) {
			memset (K051316TileMap[i], 0, (32 * 16) * (32 * 16) * sizeof(INT16));
		}
	}
}

void K051316Exit()
{
	for (INT32 i = 0; i < 3; i++)
	{
		BurnFree (K051316Ram[i]);
		BurnFree (K051316TileMap[i]);
		K051316Callback[i] = NULL;
	}
}

void K051316SetOffset(INT32 chip, INT32 xoffs, INT32 yoffs)
{
	K051316Offs[chip][0] = xoffs;
	K051316Offs[chip][1] = yoffs;
}

UINT8 K051316ReadRom(INT32 chip, INT32 offset)
{
	if ((K051316Ctrl[chip][0x0e] & 0x01) == 0)
	{
		INT32 addr = offset + (K051316Ctrl[chip][0x0c] << 11) + (K051316Ctrl[chip][0x0d] << 19);
		if (K051316Depth[chip] <= 4) addr /= 2;
		addr &= K051316Mask[chip];

		return K051316Gfx[chip][addr];
	}

	return 0;
}

UINT8 K051316Read(INT32 chip, INT32 offset)
{
	return K051316Ram[chip][offset];
}

static inline void K051316_write_tile(INT32 offset, INT32 chip)
{
	offset &= 0x3ff;

	INT32 sx = (offset & 0x1f) << 4;
	INT32 sy = (offset >> 5) << 4;

	INT32 code = K051316Ram[chip][offset];
	INT32 color = K051316Ram[chip][offset + 0x400];
	INT32 flags = 0;

	(*K051316Callback[chip])(&code,&color,&flags);

	UINT8 *src = K051316GfxExp[chip] + (code * 16 * 16);
	UINT16 *dst;

	color <<= K051316Depth[chip];

	INT32 flipx = flags & 1;
	INT32 flipy = flags & 2;
	if (flipx) flipx = 0x0f;
	if (flipy) flipy = 0x0f;

	for (INT32 y = 0; y < 16; y++, sy++)
	{
		dst = K051316TileMap[chip] + ((sy << 9) + sx);

		for (INT32 x = 0; x < 16; x++)
		{
			INT32 pxl = src[((y^flipy) << 4) | (x ^ flipx)];

			if (pxl != K051316TransColor[chip]) {
				dst[x] = color | pxl;
			} else {
				dst[x] = 0x8000 | color | pxl;
			}
		}
	}
}

void K051316Write(INT32 chip, INT32 offset, INT32 data)
{
	K051316Ram[chip][offset] = data;
	K051316_write_tile(offset & 0x3ff, chip);
}

void K051316WriteCtrl(INT32 chip, INT32 offset, INT32 data)
{
	K051316Ctrl[chip][offset & 0x0f] = data;
}

void K051316WrapEnable(INT32 chip, INT32 status)
{
	K051316Wrap[chip] = status;
}

static inline void copy_roz(INT32 chip, UINT32 startx, UINT32 starty, INT32 incxx, INT32 incxy, INT32 incyx, INT32 incyy, INT32 wrap, INT32 transp)
{
	UINT16 *dst = pTransDraw;
	UINT16 *src = K051316TileMap[chip];

	UINT32 hshift = 512 << 16;
	UINT32 wshift = 512 << 16;

	for (INT32 sy = 0; sy < nScreenHeight; sy++, startx+=incyx, starty+=incyy)
	{
		UINT32 cx = startx;
		UINT32 cy = starty;

		if (wrap) {
			if (transp) {
				for (INT32 x = 0; x < nScreenWidth; x++, cx+=incxx, cy+=incxy, dst++)
				{
					INT32 pxl = src[(((cy >> 16) & 0x1ff) << 9) | ((cx >> 16) & 0x1ff)];
			
					if (!(pxl & 0x8000)) {
						*dst = pxl;
					}
				}
			} else {
				for (INT32 x = 0; x < nScreenWidth; x++, cx+=incxx, cy+=incxy, dst++) {
					*dst = src[(((cy >> 16) & 0x1ff) << 9) | ((cx >> 16) & 0x1ff)] & 0x7fff;
				}
			}
		} else {
			if (transp) {
				for (INT32 x = 0; x < nScreenWidth; x++, cx+=incxx, cy+=incxy, dst++)
				{
					if (cx < wshift && cy < hshift) {
						INT32 pxl = src[(((cy >> 16) & 0x1ff) << 9) | ((cx >> 16) & 0x1ff)];
						if (!(pxl & 0x8000)) {
							*dst = pxl;
						}
					}
				}
			} else {
				for (INT32 x = 0; x < nScreenWidth; x++, cx+=incxx, cy+=incxy, dst++)
				{
					UINT32 pos = ((cy >> 16) << 9) | (cx >> 16);

					if (pos >= 0x40000) continue;

					*dst = src[pos] & 0x7fff;
				}
			}
		}
	}
}

void K051316_zoom_draw(INT32 chip, INT32 /*flags*/)
{
	UINT32 startx,starty;
	INT32 incxx,incxy,incyx,incyy;

	startx = 256 * ((INT16)(256 * K051316Ctrl[chip][0x00] + K051316Ctrl[chip][0x01]));
	incxx  =        (INT16)(256 * K051316Ctrl[chip][0x02] + K051316Ctrl[chip][0x03]);
	incyx  =        (INT16)(256 * K051316Ctrl[chip][0x04] + K051316Ctrl[chip][0x05]);
	starty = 256 * ((INT16)(256 * K051316Ctrl[chip][0x06] + K051316Ctrl[chip][0x07]));
	incxy  =        (INT16)(256 * K051316Ctrl[chip][0x08] + K051316Ctrl[chip][0x09]);
	incyy  =        (INT16)(256 * K051316Ctrl[chip][0x0a] + K051316Ctrl[chip][0x0b]);

	startx -= (16 + K051316Offs[chip][1]) * incyx;
	starty -= (16 + K051316Offs[chip][1]) * incyy;

	startx -= (89 + K051316Offs[chip][0]) * incxx;
	starty -= (89 + K051316Offs[chip][0]) * incxy;

	copy_roz(chip, startx << 5,starty << 5,incxx << 5,incxy << 5,incyx << 5,incyy << 5, K051316Wrap[chip], K051316TransColor[chip]+1); // transp..
}

void K051316RedrawTiles(INT32 chip)
{
	if (K051316Ram[chip]) {
		for (INT32 j = 0; j < 0x400; j++) {
			K051316_write_tile(j, chip);
		}
	}
}

void K051316Scan(INT32 nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		for (INT32 i = 0; i < 3; i++) {
			if (K051316Ram[i]) {
				memset(&ba, 0, sizeof(ba));
				ba.Data	  = K051316Ram[i];
				ba.nLen	  = 0x800;
				ba.szName = "K052109 Ram";
				BurnAcb(&ba);
			}

			memset(&ba, 0, sizeof(ba));
			ba.Data	  = K051316Ctrl[i];
			ba.nLen	  = 0x010;
			ba.szName = "K052109 Control";
			BurnAcb(&ba);	
		}
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(K051316Wrap[0]);
		SCAN_VAR(K051316Wrap[1]);
		SCAN_VAR(K051316Wrap[2]);
	}

	if (nAction & ACB_WRITE) {
		K051316RedrawTiles(0);
		K051316RedrawTiles(1);
		K051316RedrawTiles(2);
	}
}
