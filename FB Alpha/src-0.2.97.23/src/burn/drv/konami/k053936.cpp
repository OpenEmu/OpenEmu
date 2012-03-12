#include "tiles_generic.h"
#include "konamiic.h"

#define MAX_K053936	2

static INT32 nRamLen[MAX_K053936] = { 0, 0 };
static INT32 nWidth[MAX_K053936] = { 0, 0 };
static INT32 nHeight[MAX_K053936] = { 0, 0 };
static UINT16 *tscreen[MAX_K053936] = { NULL, NULL };
static UINT8 *ramptr[MAX_K053936] = { NULL, NULL };
static UINT8 *rambuf[MAX_K053936] = { NULL, NULL };

static INT32 K053936Wrap[MAX_K053936] = { 0, 0 };
static INT32 K053936Offset[MAX_K053936][2] = { { 0, 0 }, { 0, 0 } };

static void (*pTileCallback0)(INT32 offset, UINT16 *ram, INT32 *code, INT32 *color, INT32 *sx, INT32 *sy, INT32 *fx, INT32 *fy);
static void (*pTileCallback1)(INT32 offset, UINT16 *ram, INT32 *code, INT32 *color, INT32 *sx, INT32 *sy, INT32 *fx, INT32 *fy);

void K053936Reset()
{
	for (INT32 i = 0; i < MAX_K053936; i++) {
		if (rambuf[i]) {
			memset (rambuf[i], 0, nRamLen[i]);
		}
	}
}

void K053936Init(INT32 chip, UINT8 *ram, INT32 len, INT32 w, INT32 h, void (*pCallback)(INT32 offset, UINT16 *ram, INT32 *code, INT32 *color, INT32 *sx, INT32 *sy, INT32 *fx, INT32 *fy))
{
	ramptr[chip] = ram;

	nRamLen[chip] = len;

	if (rambuf[chip] == NULL) {
		rambuf[chip] = (UINT8*)BurnMalloc(len);
	}

	nWidth[chip] = w;
	nHeight[chip] = h;

	if (tscreen[chip] == NULL) {
		tscreen[chip] = (UINT16*)BurnMalloc(w * h * 2);
	}

	if (chip == 0) {
		pTileCallback0 = pCallback;
	}
	if (chip == 1) {
		pTileCallback1 = pCallback;
	}

	KonamiIC_K053936InUse = 1;
}

void K053936Exit()
{
	for (INT32 i = 0; i < MAX_K053936; i++) {
		nRamLen[i] = 0;
		nWidth[i] = 0;
		nHeight[i] = 0;
		BurnFree (tscreen[i]);
		ramptr[i] = NULL;
		BurnFree (rambuf[i]);
		K053936Wrap[i] = 0;
		K053936Offset[i][0] = K053936Offset[i][1] = 0;
	}
}

void K053936PredrawTiles(INT32 chip, UINT8 *gfx, INT32 transparent, INT32 tcol)
{
	INT32 twidth = nWidth[chip];
	UINT16 *ram = (UINT16*)ramptr[chip];
	UINT16 *buf = (UINT16*)rambuf[chip];

	for (INT32 i = 0; i < nRamLen[chip] / 2; i++)
	{
		if (ram[i] != buf[i]) {
			INT32 sx;
			INT32 sy;
			INT32 code;
			INT32 color;
			INT32 fx;
			INT32 fy;
		
			if (chip) {
				pTileCallback1(i, ram, &code, &color, &sx, &sy, &fx, &fy);
			} else {
				pTileCallback0(i, ram, &code, &color, &sx, &sy, &fx, &fy);
			}
		
			// draw tile
			{
				if (fy) fy  = 0xf0;
				if (fx) fy |= 0x0f;
		
				UINT8 *src = gfx + (code * 16 * 16);
				UINT16 *sdst = tscreen[chip] + (sy * twidth) + sx;
		
				for (INT32 y = 0; y < 16; y++) {
					for (INT32 x = 0; x < 16; x++) {
						INT32 pxl = src[((y << 4) | x) ^ fy];
						if (transparent) {
							if (pxl == tcol) pxl |= 0x8000;
						}
		
						sdst[x] = pxl | color;
					}
					sdst += twidth;
				}
			}
		}
		buf[i] = ram[i];
	}
}

static inline void copy_roz(INT32 chip, INT32 minx, INT32 maxx, INT32 miny, INT32 maxy, UINT32 startx, UINT32 starty, INT32 incxx, INT32 incxy, INT32 incyx, INT32 incyy, INT32 transp)
{
	UINT16 *dst = pTransDraw;
	UINT16 *src = tscreen[chip];

	INT32 hmask = nHeight[chip] - 1;
	INT32 wmask = nWidth[chip] - 1;

	INT32 wrap = K053936Wrap[chip];

	for (INT32 sy = miny; sy < maxy; sy++, startx+=incyx, starty+=incyy)
	{
		UINT32 cx = startx;
		UINT32 cy = starty;

		if (transp) {
			if (wrap) {
				for (INT32 x = minx; x < maxx; x++, cx+=incxx, cy+=incxy, dst++)
				{
					INT32 pxl = src[(((cy >> 16) & hmask) << 10) + ((cx >> 16) & wmask)];
		
					if (!(pxl & 0x8000)) {
						*dst = pxl;
					}
				}
			} else {
				for (INT32 x = minx; x < maxx; x++, cx+=incxx, cy+=incxy, dst++)
				{
					INT32 yy = cy >> 16;
					if (yy > hmask || yy < 0) continue;
					INT32 xx = cx >> 16;
					if (xx > wmask || xx < 0) continue;

					INT32 pxl = src[(yy << 10) + xx];

					if (!(pxl & 0x8000)) {
						*dst = pxl;
					}
				}
			}
		} else {
			if (wrap) {
				for (INT32 x = minx; x < maxx; x++, cx+=incxx, cy+=incxy, dst++) {
					*dst = src[(((cy >> 16) & hmask) << 10) + ((cx >> 16) & wmask)] & 0x7fff;
				}
			} else {
				for (INT32 x = minx; x < maxx; x++, cx+=incxx, cy+=incxy, dst++) {
					INT32 yy = cy >> 16;
					if (yy > hmask || yy < 0) continue;
					INT32 xx = cx >> 16;
					if (xx > wmask || xx < 0) continue;

					*dst = src[(yy << 10) + xx] & 0x7fff;
				}
			}
		}
	}
}

void K053936Draw(INT32 chip, UINT16 *ctrl, UINT16 *linectrl, INT32 transp)
{
	if (ctrl[0x07] & 0x0040 && linectrl)	// Super!
	{
		UINT32 startx,starty;
		INT32 incxx,incxy;

		INT32 minx, maxx, maxy, miny, y;

		if ((ctrl[0x07] & 0x0002) && ctrl[0x09])	// glfgreat
		{
			minx = ctrl[0x08] + K053936Offset[chip][0]+2;
			maxx = ctrl[0x09] + K053936Offset[chip][0]+2 - 1;
			if (minx < 0) minx = 0;
			if (maxx > nScreenWidth) maxx = nScreenWidth;

			y = ctrl[0x0a] + K053936Offset[chip][1]-2;
			if (y < 0) y = 0;
			maxy = ctrl[0x0b] + K053936Offset[chip][1]-2 - 1;
			if (maxy > nScreenHeight) maxy = nScreenHeight;
		}
		else
		{
			minx = 0;
			maxx = nScreenWidth;

			y = 0;
			maxy = nScreenHeight;
		}

		while (y <= maxy)
		{
			UINT16 *lineaddr = linectrl + 4*((y - K053936Offset[chip][1]) & 0x1ff);
			miny = maxy = y;

			startx = 256 * (INT16)(lineaddr[0] + ctrl[0x00]);
			starty = 256 * (INT16)(lineaddr[1] + ctrl[0x01]);
			incxx  =       (INT16)(lineaddr[2]);
			incxy  =       (INT16)(lineaddr[3]);

			if (ctrl[0x06] & 0x8000) incxx *= 256;
			if (ctrl[0x06] & 0x0080) incxy *= 256;

			startx -= K053936Offset[chip][0] * incxx;
			starty -= K053936Offset[chip][0] * incxy;

			copy_roz(chip, minx, maxx, miny, maxy, startx << 5, starty << 5, incxx << 5, incxy << 5, 0, 0, transp);

			y++;
		}
	}
	else	// simple
	{
		UINT32 startx,starty;
		INT32 incxx,incxy,incyx,incyy;

		startx = 256 * (INT16)(ctrl[0x00]);
		starty = 256 * (INT16)(ctrl[0x01]);
		incyx  =       (INT16)(ctrl[0x02]);
		incyy  =       (INT16)(ctrl[0x03]);
		incxx  =       (INT16)(ctrl[0x04]);
		incxy  =       (INT16)(ctrl[0x05]);

		if (ctrl[0x06] & 0x4000) { incyx *= 256; incyy *= 256; }
		if (ctrl[0x06] & 0x0040) { incxx *= 256; incxy *= 256; }

		startx -= K053936Offset[chip][1] * incyx;
		starty -= K053936Offset[chip][1] * incyy;

		startx -= K053936Offset[chip][0] * incxx;
		starty -= K053936Offset[chip][0] * incxy;

		copy_roz(chip, 0, nScreenWidth, 0, nScreenHeight, startx << 5, starty << 5, incxx << 5, incxy << 5, incyx << 5, incyy << 5, transp);
	}
}

void K053936EnableWrap(INT32 chip, INT32 status)
{
	K053936Wrap[chip] = status;
}

void K053936SetOffset(INT32 chip, INT32 xoffs, INT32 yoffs)
{
	K053936Offset[chip][0] = xoffs;
	K053936Offset[chip][1] = yoffs;
}

void K053936Scan(INT32 nAction)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(K053936Wrap[0]);
		SCAN_VAR(K053936Wrap[1]);
	}
}

