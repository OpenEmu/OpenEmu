// FB Alpha Pooyan Driver Module
// Based on MAME driver by Allard van der Bas, Mike Cuddy, Nicola Salmoria, Martin Binder, and Marco Cassili

#include "tiles_generic.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *Mem, *MemEnd;
static UINT8 *Rom0, *Rom1, *Gfx0, *Gfx1, *Prom;
static UINT8 DrvJoy1[8], DrvJoy2[8], DrvJoy3[8], DrvDips[2], DrvReset;
static UINT32 *Palette, *DrvPalette;
static UINT8 DrvRecalcPalette;

static INT16 *pFMBuffer;
static INT16 *pAY8910Buffer[6];

static UINT8 irqtrigger;
static UINT8 irq_enable;
static UINT8 flipscreen;
static UINT8 soundlatch;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy1 + 0,	"p1 coin"  },
	{"P1 Start"     , BIT_DIGITAL  , DrvJoy1 + 3,	"p1 start" },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy2 + 2, "p1 up"    },
	{"P1 Down"      ,	BIT_DIGITAL  , DrvJoy2 + 3, "p1 down"  },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p1 fire 1"},

	{"P2 Coin"      , BIT_DIGITAL  , DrvJoy1 + 1,	"p2 coin"  },
	{"P2 Start"     , BIT_DIGITAL  , DrvJoy1 + 4,	"p2 start" },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy3 + 2, "p2 up"    },
	{"P2 Down"      ,	BIT_DIGITAL  , DrvJoy3 + 3, "p2 down"  },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 4,	"p2 fire 1"},

	{"Service"      ,	BIT_DIGITAL  , DrvJoy1 + 2,	"service"  },

	{"Reset"        ,	BIT_DIGITAL  , &DrvReset  ,	"reset"    },
	{"Dip 1"        ,	BIT_DIPSWITCH, DrvDips + 0,	"dip"	     },
	{"Dip 2"        ,	BIT_DIPSWITCH, DrvDips + 1,	"dip"	     },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0xff, NULL },

	{0   , 0xfe, 0   , 16  , "Coin A" },
	{0x0c, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"  },
	{0x0c, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"  },
	{0x0c, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"  },
	{0x0c, 0x01, 0x0f, 0x04, "3 Coins 2 Credits" },
	{0x0c, 0x01, 0x0f, 0x01, "4 Coins 3 Credits" },
	{0x0c, 0x01, 0x0f, 0x0f, "1 Coin 1 Credit"   },
	{0x0c, 0x01, 0x0f, 0x03, "3 Coins 4 Credits" },
	{0x0c, 0x01, 0x0f, 0x07, "2 Coins 3 Credits" },
	{0x0c, 0x01, 0x0f, 0x0e, "1 Coin 2 Credits"  },
	{0x0c, 0x01, 0x0f, 0x06, "2 Coins 5 Credits" },
	{0x0c, 0x01, 0x0f, 0x0d, "1 Coin 3 Credits"  },
	{0x0c, 0x01, 0x0f, 0x0c, "1 Coin 4 Credits"  },
	{0x0c, 0x01, 0x0f, 0x0b, "1 Coin 5 Credits"  },
	{0x0c, 0x01, 0x0f, 0x0a, "1 Coin 6 Credits"  },
	{0x0c, 0x01, 0x0f, 0x09, "1 Coin 7 Credits"  },
	{0x0c, 0x01, 0x0f, 0x00, "Free Play"   	     },

	{0   , 0xfe, 0   , 15  , "Coin B" },
	{0x0c, 0x82, 0xf0, 0x20, "4 Coins 1 Credit"  },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0x50, "3 Coins 1 Credit"  },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0x80, "2 Coins 1 Credit"  },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0x40, "3 Coins 2 Credits" },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0x10, "4 Coins 3 Credits" },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0xf0, "1 Coin 1 Credit"   },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0x30, "3 Coins 4 Credits" },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0x70, "2 Coins 3 Credits" },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0xe0, "1 Coin 2 Credits"  },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0x60, "2 Coins 5 Credits" },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0xd0, "1 Coin 3 Credits"  },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0xc0, "1 Coin 4 Credits"  },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0xb0, "1 Coin 5 Credits"  },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0xa0, "1 Coin 6 Credits"  },
	{0x0c, 0x00, 0x0f, 0x00, NULL },
	{0x0c, 0x82, 0xf0, 0x90, "1 Coin 7 Credits"  },
	{0x0c, 0x00, 0x0f, 0x00, NULL },

	// Default Values
	{0x0d, 0xff, 0xff, 0x7b, NULL },

	{0   , 0xfe, 0   , 4   , "Lives" },
	{0x0d, 0x01, 0x03, 0x03, "3"   },
	{0x0d, 0x01, 0x03, 0x02, "4"   },
	{0x0d, 0x01, 0x03, 0x01, "5"   },
	{0x0d, 0x01, 0x03, 0x00, "256" },

	{0   , 0xfe, 0   , 2   , "Cabinet" },
	{0x0d, 0x01, 0x04, 0x00, "Upright"  },
	{0x0d, 0x01, 0x04, 0x04, "Cocktail" },

	{0   , 0xfe, 0   , 2   , "Bonus Life"	},
	{0x0d, 0x01, 0x08, 0x08, "50K 80K+" },
	{0x0d, 0x01, 0x08, 0x00, "30K 70K+"	},

	{0   , 0xfe, 0   , 8   , "Difficulty" },
	{0x0d, 0x01, 0x70, 0x70, "1 (Easy)" },
	{0x0d, 0x01, 0x70, 0x60, "2"    		},
	{0x0d, 0x01, 0x70, 0x50, "3"    		},
	{0x0d, 0x01, 0x70, 0x40, "4"    		},
	{0x0d, 0x01, 0x70, 0x30, "5"    		},
	{0x0d, 0x01, 0x70, 0x20, "6"    		},
	{0x0d, 0x01, 0x70, 0x10, "7"    		},
	{0x0d, 0x01, 0x70, 0x00, "8 (Hard)" },

	{0   , 0xfe, 0   , 2   , "Demo Sounds" },
	{0x0d, 0x01, 0x80, 0x80, "Off" },
	{0x0d, 0x01, 0x80, 0x00, "On"  },
};

STDDIPINFO(Drv)

UINT8 __fastcall pooyan_cpu0_read(UINT16 address)
{
	UINT8 ret = 0xff;

	switch (address)
	{
		case 0xa000:
			return DrvDips[1];

		case 0xa080:
		{
			for (INT32 i = 0; i < 8; i++) {
				ret ^= DrvJoy1[i] << i;
			}

			return ret;
		}

		case 0xa0a0:
		{
			for (INT32 i = 0; i < 8; i++) {
				ret ^= DrvJoy2[i] << i;
			}

			return ret;
		}

		case 0xa0c0:
		{
			for (INT32 i = 0; i < 8; i++) {
				ret ^= DrvJoy3[i] << i;
			}

			return ret;
		}

		case 0xa0e0:
			return DrvDips[0];

	}

	return 0;
}

void __fastcall pooyan_cpu0_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xa000: 
			// watchdog
		break;

		case 0xa100:
			soundlatch = data;
		break;

		case 0xa180:
			irq_enable = data & 1;

			if (!irq_enable)
				ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		break;

		case 0xa181:
		{
			if (irqtrigger == 0 && data) {
				ZetClose();
				ZetOpen(1);
				ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
				ZetClose();
				ZetOpen(0);
			}

			irqtrigger = data;
		}
		break;

		case 0xa183:
		break;

		case 0xa187:
			flipscreen = ~data & 1;
		break;
	}
}

UINT8 __fastcall pooyan_cpu1_read(UINT16 address)
{
	switch (address & 0xf000)
	{
		case 0x4000:
			return AY8910Read(0);

		case 0x6000:
			return AY8910Read(1);
	}

	return 0;
}

void __fastcall pooyan_cpu1_write(UINT16 address, UINT8 data)
{
	switch (address & 0xf000)
	{
		case 0x4000:
			AY8910Write(0, 1, data);
		break;

		case 0x5000:
			AY8910Write(0, 0, data);
		break;

		case 0x6000:
			AY8910Write(1, 1, data);
		break;

		case 0x7000:
			AY8910Write(1, 0, data);
		break;

		case 0x8000:
		case 0x9000:
		case 0xa000:
		case 0xb000:
		case 0xc000:
		case 0xd000:
		case 0xe000:
		case 0xf000:
		// timeplt_filter_w
		break;
	}
}

static UINT8 AY8910_0_port0(UINT32)
{
	return soundlatch;
}

static UINT8 AY8910_0_port1(UINT32)
{
	static INT32 timeplt_timer[10] =
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x90, 0xa0, 0xb0, 0xa0, 0xd0
	};

	return timeplt_timer[(ZetTotalCycles() >> 9) % 10];
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	for (INT32 i = 0; i < 2; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}

	AY8910Reset(0);
	AY8910Reset(1);

	memset (Rom0 + 0x8000, 0, 0x1800);
	memset (Rom1 + 0x3000, 0, 0x0400);

	irqtrigger = 0;
	flipscreen = 0;
	soundlatch = 0;
	irq_enable = 0;

	return 0;
}

static void DrvPaletteInit()
{
	INT32 i;
	UINT32 c_pal[0x20];

	for (i = 0; i < 0x20; i++)
	{
		INT32 r, g, b;
		INT32 bit0, bit1, bit2;

		bit0 = (Prom[i] >> 0) & 0x01;
		bit1 = (Prom[i] >> 1) & 0x01;
		bit2 = (Prom[i] >> 2) & 0x01;
		r = 33 * bit0 + 71 * bit1 + 151 * bit2;

		bit0 = (Prom[i] >> 3) & 0x01;
		bit1 = (Prom[i] >> 4) & 0x01;
		bit2 = (Prom[i] >> 5) & 0x01;
		g = 33 * bit0 + 71 * bit1 + 151 * bit2;

		bit0 = (Prom[i] >> 6) & 0x01;
		bit1 = (Prom[i] >> 7) & 0x01;
		b = 80 * bit0 + 171 * bit1;

		c_pal[i] = (r << 16) | (g << 8) | b;
	}

	for (i = 0; i < 0x100; i++)
	{
		Palette[i + 0x000] = c_pal[(Prom[i + 0x020] & 0x0f) | 0x10];
		Palette[i + 0x100] = c_pal[(Prom[i + 0x120] & 0x0f) | 0x00];
	}
}

static INT32 DrvGfxDecode()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x2000);
	if (!tmp) return 1;

	static INT32 Planes[4] = { 0x8004, 0x8000, 4, 0 };
	static INT32 XOffs[16] = { 0, 1, 2, 3, 0x40, 0x41, 0x42, 0x43, 0x80, 0x81, 0x82, 0x83, 0xc0, 0xc1, 0xc2, 0xc3 };
	static INT32 YOffs[16] = { 0, 8, 16, 24, 32, 40, 48, 56, 256, 264, 272, 280, 288, 296, 304, 312 };

	memcpy (tmp, Gfx0, 0x2000);

	GfxDecode(0x100, 4,  8,  8, Planes, XOffs, YOffs, 0x080, tmp, Gfx0);

	memcpy (tmp, Gfx1, 0x2000);

	GfxDecode(0x40,  4, 16, 16, Planes, XOffs, YOffs, 0x200, tmp, Gfx1);

	BurnFree(tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Rom0           = Next; Next += 0x10000;
	Rom1           = Next; Next += 0x04000;
	Gfx0           = Next; Next += 0x04000;
	Gfx1           = Next; Next += 0x04000;
	Prom	       = Next; Next += 0x00220;

	Palette	       = (UINT32*)Next; Next += 0x00200 * sizeof(UINT32);
	DrvPalette     = (UINT32*)Next; Next += 0x00200 * sizeof(UINT32);

	pFMBuffer      = (INT16 *)Next; Next += nBurnSoundLen * 6 * sizeof(INT16);

	MemEnd         = Next;

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;

	{
		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(Rom0 + i * 0x2000, 0  + i, 1)) return 1;
		}

		for (INT32 i = 0; i < 2; i++) {
			if (BurnLoadRom(Rom1 + i * 0x1000, 4  + i, 1)) return 1;
			if (BurnLoadRom(Gfx0 + i * 0x1000, 6  + i, 1)) return 1;
			if (BurnLoadRom(Gfx1 + i * 0x1000, 8  + i, 1)) return 1;
		}

		if (BurnLoadRom(Prom + 0x000, 10, 1)) return 1;
		if (BurnLoadRom(Prom + 0x020, 11, 1)) return 1;
		if (BurnLoadRom(Prom + 0x120, 12, 1)) return 1;

		if (DrvGfxDecode()) return 1;
		DrvPaletteInit();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, Rom0 + 0x0000);
	ZetMapArea(0x0000, 0x7fff, 2, Rom0 + 0x0000);
	ZetMapArea(0x8000, 0x8fff, 0, Rom0 + 0x8000);
	ZetMapArea(0x8000, 0x8fff, 1, Rom0 + 0x8000);
	ZetMapArea(0x8000, 0x8fff, 2, Rom0 + 0x8000);
	ZetMapArea(0x9000, 0x90ff, 0, Rom0 + 0x9000);
	ZetMapArea(0x9000, 0x90ff, 1, Rom0 + 0x9000);
	ZetMapArea(0x9000, 0x90ff, 2, Rom0 + 0x9000);
	ZetMapArea(0x9400, 0x94ff, 0, Rom0 + 0x9400);
	ZetMapArea(0x9400, 0x94ff, 1, Rom0 + 0x9400);
	ZetMapArea(0x9400, 0x94ff, 2, Rom0 + 0x9400);
	ZetSetWriteHandler(pooyan_cpu0_write);
	ZetSetReadHandler(pooyan_cpu0_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMemEnd();
	ZetMapArea(0x0000, 0x1fff, 0, Rom1 + 0x0000);
	ZetMapArea(0x0000, 0x1fff, 2, Rom1 + 0x0000);
	ZetMapArea(0x3000, 0x33ff, 0, Rom1 + 0x3000);
	ZetMapArea(0x3000, 0x33ff, 1, Rom1 + 0x3000);
	ZetMapArea(0x3000, 0x33ff, 2, Rom1 + 0x3000);
	ZetSetWriteHandler(pooyan_cpu1_write);
	ZetSetReadHandler(pooyan_cpu1_read);
	ZetClose();

	GenericTilesInit();

	AY8910Init(0, 1789773, nBurnSoundRate, &AY8910_0_port0, &AY8910_0_port1, NULL, NULL);
	AY8910Init(1, 1789773, nBurnSoundRate, NULL, NULL, NULL, NULL);

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	ZetExit();
	AY8910Exit(0);
	AY8910Exit(1);

	GenericTilesExit();

	BurnFree (Mem);

	Mem = MemEnd = NULL;
	Rom0 = Rom1 = Gfx0 = Gfx1 = Prom = NULL;

	Palette = DrvPalette = NULL;
	pFMBuffer = NULL;

	for (INT32 i = 0; i < 6; i++) {
		pAY8910Buffer[i] = NULL;
	}

	irqtrigger = 0;
	irq_enable = 0;
	flipscreen = 0;
	soundlatch = 0;

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalcPalette) {
		for (INT32 i = 0; i < 0x200; i++) {
			UINT32 col = Palette[i];
			DrvPalette[i] = BurnHighCol(col >> 16, col >> 8, col, 0);
		}
	}

	for (INT32 i = 0; i < 0x0400; i++)
	{
		INT32 attr = Rom0[0x8000 + i];
		INT32 code = Rom0[0x8400 + i];
		INT32 color = attr & 0x0f;
		INT32 flipx = (attr >> 6) & 1;
		INT32 flipy = (attr >> 7) & 1;

		INT32 sx = (i << 3) & 0xf8;
		INT32 sy = (i >> 2) & 0xf8;

		if (flipscreen) {
			sx ^= 0xf8;
			sy ^= 0xf8;
			flipx ^= 1;
			flipy ^= 1;
		}

		if (sy > 239 || sy < 16) continue;
		sy -= 16;

		if (flipy) {
			if (flipx) {
				Render8x8Tile_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, Gfx0);
			} else {
				Render8x8Tile_FlipY(pTransDraw, code, sx, sy, color, 4, 0, Gfx0);
			}
		} else {
			if (flipx) {
				Render8x8Tile_FlipX(pTransDraw, code, sx, sy, color, 4, 0, Gfx0);
			} else {
				Render8x8Tile(pTransDraw, code, sx, sy, color, 4, 0, Gfx0);
			}
		}
	}

	for (INT32 i = 0x10; i < 0x40; i += 2)
	{
		INT32 sx = Rom0[0x9000 + i];
		INT32 sy = 240 - Rom0[0x9401 + i];

		INT32 code = Rom0[0x9001 + i] & 0x3f;
		INT32 color = (Rom0[0x9400 + i] & 0x0f) | 0x10;
		INT32 flipx = ~Rom0[0x9400 + i] & 0x40;
		INT32 flipy = Rom0[0x9400 + i] & 0x80;

		if (sy == 0 || sy >= 240) continue;

		sy -= 16;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, Gfx1);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw,  code, sx, sy, color, 4, 0, 0, Gfx1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw,  code, sx, sy, color, 4, 0, 0, Gfx1);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw,        code, sx, sy, color, 4, 0, 0, Gfx1);
			}
		}
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	INT32 nInterleave = 10;

	INT32 nCyclesSegment;
	INT32 nCyclesTotal[2], nCyclesDone[2];

	nCyclesTotal[0] = 3072000 / 60;
	nCyclesTotal[1] = 1789773 / 60; 

	nCyclesDone[0] = nCyclesDone[1] = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nCurrentCPU, nNext;

		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (irq_enable && i == (nInterleave - 1)) ZetNmi();
		ZetClose();

		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		ZetClose();
	}

	if (pBurnSoundOut) {
		INT32 nSample;
		INT32 nSegmentLength = nBurnSoundLen;
		INT16* pSoundBuf = pBurnSoundOut;
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n] >> 2;
				nSample += pAY8910Buffer[1][n] >> 2;
				nSample += pAY8910Buffer[2][n] >> 2;
				nSample += pAY8910Buffer[3][n] >> 2;
				nSample += pAY8910Buffer[4][n] >> 2;
				nSample += pAY8910Buffer[5][n] >> 2;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
			}
		}
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029521;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = Rom0 + 0x8000;
		ba.nLen	  = 0x1800;
		ba.szName = "Cpu #0 Ram";
		BurnAcb(&ba);

		ba.Data	  = Rom1 + 0x3000;
		ba.nLen	  = 0x0400;
		ba.szName = "Cpu #1 Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		AY8910Scan(nAction, pnMin);

		SCAN_VAR(soundlatch);
		SCAN_VAR(irqtrigger);
		SCAN_VAR(irq_enable);
		SCAN_VAR(flipscreen);
	}

	return 0;
}


// Pooyan

static struct BurnRomInfo pooyanRomDesc[] = {
	{ "1.4a",         0x2000, 0xbb319c63, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "2.5a",         0x2000, 0xa1463d98, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3.6a",         0x2000, 0xfe1a9e08, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "4.7a",         0x2000, 0x9e0f9bcc, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "xx.7a",        0x1000, 0xfbe2b368, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 #1 Code
	{ "xx.8a",        0x1000, 0xe1795b3d, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "8.10g",        0x1000, 0x931b29eb, 3 | BRF_GRA },	       //  6 Characters
	{ "7.9g",         0x1000, 0xbbe6d6e4, 3 | BRF_GRA },	       //  7 

	{ "6.9a",         0x1000, 0xb2d8c121, 4 | BRF_GRA },	       //  8 Sprites
	{ "5.8a",         0x1000, 0x1097c2b6, 4 | BRF_GRA },	       //  9

	{ "pooyan.pr1",   0x0020, 0xa06a6d0e, 5 | BRF_GRA },	       // 10 Color Proms
	{ "pooyan.pr3",   0x0100, 0x8cd4cd60, 5 | BRF_GRA },	       // 11
	{ "pooyan.pr2",   0x0100, 0x82748c0b, 5 | BRF_GRA },	       // 12
};

STD_ROM_PICK(pooyan)
STD_ROM_FN(pooyan)

struct BurnDriver BurnDrvPooyan = {
	"pooyan", NULL, NULL, NULL, "1982",
	"Pooyan\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_MISC, 0,
	NULL, pooyanRomInfo, pooyanRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalcPalette, 0x200,
	224, 256, 3, 4
};


// Pooyan (Stern)

static struct BurnRomInfo pooyansRomDesc[] = {
	{ "ic22_a4.cpu",  0x2000, 0x916ae7d7, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "ic23_a5.cpu",  0x2000, 0x8fe38c61, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ic24_a6.cpu",  0x2000, 0x2660218a, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ic25_a7.cpu",  0x2000, 0x3d2a10ad, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "xx.7a",        0x1000, 0xfbe2b368, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 #1 Code
	{ "xx.8a",        0x1000, 0xe1795b3d, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "ic13_g10.cpu", 0x1000, 0x7433aea9, 3 | BRF_GRA },	       //  6 Characters
	{ "ic14_g9.cpu",  0x1000, 0x87c1789e, 3 | BRF_GRA },	       //  7

	{ "6.9a",         0x1000, 0xb2d8c121, 4 | BRF_GRA },	       //  8 Sprites
	{ "5.8a",         0x1000, 0x1097c2b6, 4 | BRF_GRA },	       //  9

	{ "pooyan.pr1",   0x0020, 0xa06a6d0e, 5 | BRF_GRA },	       // 10 Color Proms
	{ "pooyan.pr3",   0x0100, 0x8cd4cd60, 5 | BRF_GRA },	       // 11
	{ "pooyan.pr2",   0x0100, 0x82748c0b, 5 | BRF_GRA },	       // 12
};

STD_ROM_PICK(pooyans)
STD_ROM_FN(pooyans)

struct BurnDriver BurnDrvPooyans = {
	"pooyans", "pooyan", NULL, NULL, "1982",
	"Pooyan (Stern)\0", NULL, "[Konami] (Stern license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_MISC, 0,
	NULL, pooyansRomInfo, pooyansRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalcPalette, 0x200,
	224, 256, 3, 4
};


// Pootan

static struct BurnRomInfo pootanRomDesc[] = {
	{ "poo_ic22.bin", 0x2000, 0x41b23a24, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "poo_ic23.bin", 0x2000, 0xc9d94661, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3.6a",         0x2000, 0xfe1a9e08, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "poo_ic25.bin", 0x2000, 0x8ae459ef, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "xx.7a",        0x1000, 0xfbe2b368, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 #1 Code
	{ "xx.8a",        0x1000, 0xe1795b3d, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "poo_ic13.bin", 0x1000, 0x0be802e4, 3 | BRF_GRA },	       //  6 Characters
	{ "poo_ic14.bin", 0x1000, 0xcba29096, 3 | BRF_GRA },	       //  7

	{ "6.9a",         0x1000, 0xb2d8c121, 4 | BRF_GRA },	       //  8 Sprites
	{ "5.8a",         0x1000, 0x1097c2b6, 4 | BRF_GRA },	       //  9

	{ "pooyan.pr1",   0x0020, 0xa06a6d0e, 5 | BRF_GRA },	       // 10 Color Proms
	{ "pooyan.pr3",   0x0100, 0x8cd4cd60, 5 | BRF_GRA },	       // 11
	{ "pooyan.pr2",   0x0100, 0x82748c0b, 5 | BRF_GRA },	       // 12
};

STD_ROM_PICK(pootan)
STD_ROM_FN(pootan)

struct BurnDriver BurnDrvPootan = {
	"pootan", "pooyan", NULL, NULL, "1982",
	"Pootan\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_MISC, 0,
	NULL, pootanRomInfo, pootanRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalcPalette, 0x200,
	224, 256, 3, 4
};

