// FB Alpha Pirate Ship Higemaru Module
// Based on MAME driver by Mirko Buffoni

#include "tiles_generic.h"
#include "z80_intf.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *Mem, *MemEnd, *Rom, *Gfx0, *Gfx1, *Prom;
static INT16 *pAY8910Buffer[6], *pFMBuffer = NULL;
static UINT32 *DrvPalette, *Palette;
static UINT8 DrvRecalc = 0;

static UINT8 DrvJoy1[8], DrvJoy2[8], DrvJoy3[8], DrvDips[3], DrvReset;

static INT32 flipscreen;

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy3 + 7,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy3 + 6,	"p2 coin"  },
	{"P1 Start"     , BIT_DIGITAL  , DrvJoy3 + 5,	"p1 start" },
	{"P2 Start"     , BIT_DIGITAL  , DrvJoy3 + 4,	"p2 start" },

	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 right" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 1, 	"p1 left"  },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 2, 	"p1 down"  },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy1 + 3, 	"p1 up"    },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 3,	"p1 fire 1"},

	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 0, 	"p2 right" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 1, 	"p2 left"  },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy2 + 2, 	"p2 down"  },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy2 + 3, 	"p2 up"    },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 1,	"p2 fire 1"},

	{"Reset"        , BIT_DIGITAL  , &DrvReset  ,	"reset"    },
	{"Dip 1"        , BIT_DIPSWITCH, DrvDips + 0,   "dip 1"    },
	{"Dip 2"        , BIT_DIPSWITCH, DrvDips + 1,   "dip 2"    },
	{"Dip 3"        , BIT_DIPSWITCH, DrvDips + 2,   "dip 3"    },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0xff, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	{0x11, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0f, 0x01, 0x04, 0x04, "Off"		          },
	{0x0f, 0x01, 0x04, 0x00, "On"		          },
	
	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x10, 0x01, 0x07, 0x01, "5 Coins 1 Play"         },
	{0x10, 0x01, 0x07, 0x02, "4 Coins 1 Play"         },
	{0x10, 0x01, 0x07, 0x03, "3 Coins 1 Play"         },
	{0x10, 0x01, 0x07, 0x04, "1 Coins 1 Play"         },
	{0x10, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x10, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x10, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x10, 0x01, 0x07, 0x00, "Freeplay"               },

	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x10, 0x01, 0x38, 0x08, "5 Coins 1 Play"         },
	{0x10, 0x01, 0x38, 0x10, "4 Coins 1 Play"         },
	{0x10, 0x01, 0x38, 0x18, "3 Coins 1 Play"         },
	{0x10, 0x01, 0x38, 0x20, "1 Coins 1 Play"         },
	{0x10, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x10, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x10, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
	{0x10, 0x01, 0x38, 0x00, "Freeplay"               },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x10, 0x01, 0xc0, 0x80, "1"                      },
	{0x10, 0x01, 0xc0, 0x40, "2"                      },
	{0x10, 0x01, 0xc0, 0xc0, "3"                      },
	{0x10, 0x01, 0xc0, 0x00, "5"                      },

	// Dip 3
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x11, 0x01, 0x01, 0x00, "Upright"                },
	{0x11, 0x01, 0x01, 0x01, "Cocktail"               },
	
	{0   , 0xfe, 0   , 8   , "Bonus Life"             },
	{0x11, 0x01, 0x0e, 0x0e, "10k  50k  50k"          },
	{0x11, 0x01, 0x0e, 0x0c, "10k  60k  60k"          },
	{0x11, 0x01, 0x0e, 0x0a, "20k  60k  60k"          },
	{0x11, 0x01, 0x0e, 0x08, "20k  70k  70k"          },
	{0x11, 0x01, 0x0e, 0x06, "30k  70k  70k"          },
	{0x11, 0x01, 0x0e, 0x04, "30k  80k  80k"          },
	{0x11, 0x01, 0x0e, 0x02, "40k 100k 100k"          },
	{0x11, 0x01, 0x0e, 0x00, "None"                   },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x11, 0x01, 0x10, 0x00, "Off"                    },
	{0x11, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Music"             },
	{0x11, 0x01, 0x20, 0x00, "Off"                    },
	{0x11, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x11, 0x01, 0x40, 0x40, "Off"                    },
	{0x11, 0x01, 0x40, 0x00, "On"                     },
};

STDDIPINFO(Drv)

void __fastcall higemaru_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xc800:
			flipscreen = data >> 7;
		break;

		case 0xc801:
		case 0xc802:
			AY8910Write(0, (address - 1) & 1, data);
		break;

		case 0xc803:
		case 0xc804:
			AY8910Write(1, (address - 1) & 1, data);
		break;
	}
}

UINT8 __fastcall higemaru_read(UINT16 address)
{
	UINT8 ret;

	switch (address)
	{
		case 0xc000:
		{
			ret = 0xff;

			for (INT32 i = 0; i < 8; i++) {
				ret ^= DrvJoy1[i] << i;
			}

			return ret;
		}

		case 0xc001:
		{
			ret = 0xff;

			for (INT32 i = 0; i < 8; i++) {
				ret ^= DrvJoy2[i] << i;
			}

			return ret;
		}

		case 0xc002:
		{
			ret = DrvDips[0];

			for (INT32 i = 0; i < 8; i++)
				ret ^= DrvJoy3[i] << i;

			return ret;
		}

		case 0xc003:
			return DrvDips[1];

		case 0xc004:
			return DrvDips[2];
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Rom            = Next; Next += 0x10000;
	Gfx0           = Next; Next += 0x08000;
	Gfx1           = Next; Next += 0x08000;
	Prom           = Next; Next += 0x00300;

	Palette	       = (UINT32*)Next; Next += 0x00180 * sizeof(UINT32);
	DrvPalette     = (UINT32*)Next; Next += 0x00180 * sizeof(UINT32);

	pFMBuffer      = (INT16*)Next; Next += (nBurnSoundLen * 6 * sizeof(INT16));

	MemEnd         = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	memset (Rom + 0xd000, 0, 0x2000);

	AY8910Reset(0);
	AY8910Reset(1);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	flipscreen = 0;

	return 0;
}

static void DrvPaletteInit()
{
	UINT32 tmp[0x20];

	for (INT32 i = 0; i < 0x20; i++)
	{
		INT32 bit0, bit1, bit2;

		bit0 = (Prom[i] >> 0) & 1;
		bit1 = (Prom[i] >> 1) & 1;
		bit2 = (Prom[i] >> 2) & 1;
		INT32 r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (Prom[i] >> 3) & 1;
		bit1 = (Prom[i] >> 4) & 1;
		bit2 = (Prom[i] >> 5) & 1;
		INT32 g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit1 = (Prom[i] >> 6) & 1;
		bit2 = (Prom[i] >> 7) & 1;
		INT32 b = 0x47 * bit1 + 0x97 * bit2;

		tmp[i] = (r << 16) | (g << 8) | b;
	}

	Prom += 0x100;

	for (INT32 i = 0; i < 0x80; i++) {
		Palette[i] = tmp[Prom[i] & 0x0f];
	}

	for (INT32 i = 0x80; i < 0x180; i++) {
		Palette[i] = tmp[(Prom[i + 0x80] & 0x0f) | 0x10];
	}
}

static INT32 DrvGfxDecode()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x4000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, Gfx0, 0x2000);

	static INT32 Planes[4] = { 0x10004, 0x10000, 0x00004, 0x00000 };
	static INT32 XOffs[16] = { 0x000, 0x001, 0x002, 0x003, 0x008, 0x009, 0x00a, 0x00b,
				 0x100, 0x101, 0x102, 0x103, 0x108, 0x109, 0x10a, 0x10b };
	static INT32 YOffs[16] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
				 0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0 };

	GfxDecode(0x200, 2,  8,  8, Planes + 2, XOffs, YOffs, 0x080, tmp, Gfx0);

	memcpy (tmp, Gfx1, 0x4000);

	GfxDecode(0x080, 4, 16, 16, Planes,     XOffs, YOffs, 0x200, tmp, Gfx1);

	BurnFree (tmp);

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

	for (INT32 i = 0; i < 6; i++) {
		pAY8910Buffer[i] = pFMBuffer + nBurnSoundLen * i;
	}

	{
		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(Rom + i * 0x2000, i + 0, 1)) return 1;
		}

		if (BurnLoadRom(Gfx0 + 0x0000, 4, 1)) return 1;
		if (BurnLoadRom(Gfx1 + 0x0000, 5, 1)) return 1;
		if (BurnLoadRom(Gfx1 + 0x2000, 6, 1)) return 1;

		for (INT32 i = 0; i < 3; i++) {
			if (BurnLoadRom(Prom + i * 0x100, i + 7, 1)) return 1;
		}

		if (DrvGfxDecode()) return 1;
		DrvPaletteInit();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, Rom + 0x0000);
	ZetMapArea(0x0000, 0x7fff, 2, Rom + 0x0000);
	ZetMapArea(0xd000, 0xd7ff, 0, Rom + 0xd000);
	ZetMapArea(0xd000, 0xd9ff, 1, Rom + 0xd000);
	ZetMapArea(0xe000, 0xefff, 0, Rom + 0xe000);
	ZetMapArea(0xe000, 0xefff, 1, Rom + 0xe000);
	ZetMapArea(0xe000, 0xefff, 2, Rom + 0xe000);
	ZetSetWriteHandler(higemaru_write);
	ZetSetReadHandler(higemaru_read);
	ZetClose();

	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910SetAllRoutes(0, 0.25, BURN_SND_ROUTE_BOTH);
	AY8910SetAllRoutes(1, 0.25, BURN_SND_ROUTE_BOTH);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	ZetExit();
	AY8910Exit(0);
	AY8910Exit(1);
	GenericTilesExit();

	BurnFree (Mem);

	Mem = MemEnd = Rom = Gfx0 = Gfx1 = Prom = NULL;
	DrvPalette = Palette = NULL;
	pFMBuffer = NULL;

	for (INT32 i = 0; i < 6; i++) {
		pAY8910Buffer[i] = NULL;
	}

	flipscreen = 0;
	DrvRecalc = 0;

	return 0;
}


static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x100; i++) {
			INT32 color = Palette[i];
			DrvPalette[i] = BurnHighCol(color >> 16, color >> 8, color, 0);
		}
	}

	for (INT32 offs = 0x40; offs < 0x3c0; offs++)
	{
		INT32 sx = (offs << 3) & 0xf8;
		INT32 sy = (offs >> 2) & 0xf8;

		INT32 code = Rom[0xd000 + offs] | ((Rom[0xd400 + offs] & 0x80) << 1);
		INT32 color = Rom[0xd400 + offs] & 0x1f;

		sy -= 16;

		if (flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx ^ 0xf8, 216 - sy, color, 2, 0, Gfx0);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 2, 0, Gfx0);
		}
	}

	for (INT32 offs = 0x170; offs >= 0; offs -= 16)
	{
		INT32 code,color,sx,sy,flipx,flipy;

		code  = Rom[0xd880 + offs] & 0x7f;
		color = Rom[0xd884 + offs] & 0x0f;
		sx    = Rom[0xd88c + offs];
		sy    = Rom[0xd888 + offs];
		flipx = Rom[0xd884 + offs] & 0x10;
		flipy = Rom[0xd884 + offs] & 0x20;

		if (flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sy -= 16;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx,       sy, color, 4, 0x0f, 0x80, Gfx1);
				if (sx > 0xf0) Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx - 256, sy, color, 4, 0x0f, 0x80, Gfx1);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx,       sy, color, 4, 0x0f, 0x80, Gfx1);
				if (sx > 0xf0) Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx - 256, sy, color, 4, 0x0f, 0x80, Gfx1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx,       sy, color, 4, 0x0f, 0x80, Gfx1);
				if (sx > 0xf0) Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx - 256, sy, color, 4, 0x0f, 0x80, Gfx1);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx,       sy, color, 4, 0x0f, 0x80, Gfx1);
				if (sx > 0xf0) Render16x16Tile_Mask_Clip(pTransDraw, code, sx - 256, sy, color, 4, 0x0f, 0x80, Gfx1);
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

	ZetOpen(0);
	ZetRun(33333);
//	ZetRaiseIrq(0xd7);
	ZetSetVector(0xd7);
	ZetRaiseIrq(0);
	ZetRun(33334);
//	ZetRaiseIrq(0xcf);
	ZetSetVector(0xcf);
	ZetRaiseIrq(0);
	ZetClose();

	if (pBurnSoundOut) {
		AY8910Render(&pAY8910Buffer[0], pBurnSoundOut, nBurnSoundLen, 0);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029672;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = Rom + 0xd000;
		ba.nLen	  = 0x2000;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);
		AY8910Scan(nAction, pnMin);

		SCAN_VAR(flipscreen);
	}

	return 0;
}

	
// Pirate Ship Higemaru

static struct BurnRomInfo higemaruRomDesc[] = {
	{ "hg4.p12", 	0x2000, 0xdc67a7f9, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "hg5.m12", 	0x2000, 0xf65a4b68, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "hg6.p11", 	0x2000, 0x5f5296aa, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "hg7.m11", 	0x2000, 0xdc5d455d, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "hg3.m1", 	0x2000, 0xb37b88c8, 2 | BRF_GRA }, 	     //	 4 Characters

	{ "hg1.c14", 	0x2000, 0xef4c2f5d, 3 | BRF_GRA },	     //	 5 Sprites
	{ "hg2.e14", 	0x2000, 0x9133f804, 3 | BRF_GRA },	     //  6

	{ "hgb3.l6", 	0x0020, 0x629cebd8, 4 | BRF_GRA },	     //	 7 Color Proms
	{ "hgb5.m4", 	0x0100, 0xdbaa4443, 4 | BRF_GRA },	     //	 8
	{ "hgb1.h7", 	0x0100, 0x07c607ce, 4 | BRF_GRA },	     //	 9

	{ "hgb4.l9", 	0x0100, 0x712ac508, 0 | BRF_OPT },	     //	10 Misc. Proms (not used)
	{ "hgb2.k7", 	0x0100, 0x4921635c, 0 | BRF_OPT },	     //	11
};

STD_ROM_PICK(higemaru)
STD_ROM_FN(higemaru)

struct BurnDriver BurnDrvhigemaru = {
	"higemaru", NULL, NULL, NULL, "1984",
	"Pirate Ship Higemaru\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARWARE_CAPCOM_MISC, GBF_MAZE, 0,
	NULL, higemaruRomInfo, higemaruRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x180,
	256, 224, 4, 3
};

