 // FB Alpha Markham driver module
// Based on MAME driver by Uki

#include "tiles_generic.h"
#include "z80_intf.h"
#include "sn76496.h"

static UINT8 *AllMem;
static UINT8 *AllRam;
static UINT8 *MemEnd;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvColPROM;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvVidRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvShareRAM;

static UINT32 *Palette;
static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *scroll;

static UINT8 flipscreen;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static struct BurnInputInfo MarkhamInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,  DrvDips + 2,    "dip"		},
};

STDINPUTINFO(Markham)

static struct BurnDIPInfo MarkhamDIPList[]=
{
	{0x12, 0xff, 0xff, 0x02, NULL			},
	{0x13, 0xff, 0xff, 0x7e, NULL			},
	{0x14, 0xff, 0xff, 0x00, NULL			},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x12, 0x01, 0x01, 0x00, "3"			},
	{0x12, 0x01, 0x01, 0x01, "5"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x02, 0x02, "Upright"		},
	{0x12, 0x01, 0x02, 0x00, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x12, 0x01, 0x04, 0x00, "Off"			},
	{0x12, 0x01, 0x04, 0x04, "On"			},

	{0   , 0xfe, 0   ,    2, "Coin Chutes"		},
	{0x12, 0x01, 0x08, 0x00, "Individual"		},
	{0x12, 0x01, 0x08, 0x08, "Common"		},

	{0   , 0xfe, 0   ,   16, "Coin1 / Coin2"	},
	{0x12, 0x01, 0xf0, 0x00, "1C 1C / 1C 1C"	},
	{0x12, 0x01, 0xf0, 0x10, "2C 1C / 2C 1C"	},
	{0x12, 0x01, 0xf0, 0x20, "2C 1C / 1C 3C"	},
	{0x12, 0x01, 0xf0, 0x30, "1C 1C / 1C 2C"	},
	{0x12, 0x01, 0xf0, 0x40, "1C 1C / 1C 3C"	},
	{0x12, 0x01, 0xf0, 0x50, "1C 1C / 1C 4C"	},
	{0x12, 0x01, 0xf0, 0x60, "1C 1C / 1C 5C"	},
	{0x12, 0x01, 0xf0, 0x70, "1C 1C / 1C 6C"	},
	{0x12, 0x01, 0xf0, 0x80, "1C 2C / 1C 2C"	},
	{0x12, 0x01, 0xf0, 0x90, "1C 2C / 1C 4C"	},
	{0x12, 0x01, 0xf0, 0xa0, "1C 2C / 1C 5C"	},
	{0x12, 0x01, 0xf0, 0xb0, "1C 2C / 1C 10C"	},
	{0x12, 0x01, 0xf0, 0xc0, "1C 2C / 1C 11C"	},
	{0x12, 0x01, 0xf0, 0xd0, "1C 2C / 1C 12C"	},
	{0x12, 0x01, 0xf0, 0xe0, "1C 2C / 1C 6C"	},
	{0x12, 0x01, 0xf0, 0xf0, "Free Play"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0x03, 0x00, "None"			},
	{0x13, 0x01, 0x03, 0x01, "20000"		},
	{0x13, 0x01, 0x03, 0x02, "20000, Every 50000"	},
	{0x13, 0x01, 0x03, 0x03, "20000, Every 80000"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x04, 0x00, "Off"			},
	{0x13, 0x01, 0x04, 0x04, "On"			},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x13, 0x01, 0x80, 0x00, "Off"			},
	{0x13, 0x01, 0x80, 0x80, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x10, 0x00, "Off"			},
	{0x14, 0x01, 0x10, 0x10, "On"			},
};

STDDIPINFO(Markham)

void __fastcall markham_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe00c:
			scroll[0] = data;
		return;

		case 0xe00d:
			scroll[1] = data;
		return;

		case 0xe00e:
			flipscreen = data & 1;
		return;
	}
}

UINT8 __fastcall markham_main_read(UINT16 address)
{
	switch (address)
	{
		case 0xe000:
			return DrvDips[1];

		case 0xe001:
			return DrvDips[0];

		case 0xe002:
			return DrvInputs[0];

		case 0xe003:
			return DrvInputs[1];

		case 0xe005:
			return (DrvInputs[2] & ~0x10) | (DrvDips[2] & 0x10);
	}

	return 0;
}

void __fastcall markham_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xc000:
			SN76496Write(0, data);
		return;

		case 0xc001:
			SN76496Write(1, data);
		return;
	}
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	flipscreen = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0		= Next; Next += 0x006000;
	DrvZ80ROM1		= Next; Next += 0x006000;

	DrvGfxROM0		= Next; Next += 0x010000;
	DrvGfxROM1		= Next; Next += 0x010000;

	DrvColPROM		= Next; Next += 0x000700;

	Palette			= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);
	DrvPalette		= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	AllRam			= Next;

	DrvZ80RAM0		= Next; Next += 0x000800;
	DrvVidRAM		= Next; Next += 0x000800;
	DrvSprRAM		= Next; Next += 0x000800;
	DrvShareRAM		= Next; Next += 0x000800;

	scroll			= Next; Next += 0x000002;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[3]  = { 0x00000, 0x10000, 0x20000 };
	INT32 XOffs[16] = { 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x87, 0x86, 0x85, 0x84, 0x83, 0x82, 0x81, 0x80 };
	INT32 YOffs[16] = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x78 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x6000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x6000);

	GfxDecode(0x0100, 3, 16, 16, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x6000);

	GfxDecode(0x0400, 3,  8,  8, Plane, XOffs, YOffs, 0x040, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 0x100; i++)
	{
		INT32 r = DrvColPROM[i + 0x000] & 0x0f;
		INT32 g = DrvColPROM[i + 0x100] & 0x0f;
		INT32 b = DrvColPROM[i + 0x200] & 0x0f;

		DrvPalette[i] = (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | (b << 0);
	}

	for (INT32 i = 0; i < 0x400; i++) {
		Palette[i] = DrvPalette[DrvColPROM[0x300 + i]];
	}

	DrvRecalc = 1;
}

static INT32 DrvInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvZ80ROM0 + 0x00000,  0, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x02000,  1, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x04000,  2, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM1 + 0x00000,  3, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM1 + 0x04000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x02000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x04000,  5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x02000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x04000, 10, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x00000, 11, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x00100, 12, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x00200, 13, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x00300, 14, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x00500, 15, 1)) return 1;

		DrvGfxDecode();
		DrvPaletteInit();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x5fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x5fff, 2, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80RAM0);
	ZetMapArea(0xc800, 0xcfff, 0, DrvSprRAM);
	ZetMapArea(0xc800, 0xcfff, 1, DrvSprRAM);
	ZetMapArea(0xc800, 0xcfff, 2, DrvSprRAM);
	ZetMapArea(0xd000, 0xd7ff, 0, DrvVidRAM);
	ZetMapArea(0xd000, 0xd7ff, 1, DrvVidRAM);
	ZetMapArea(0xd000, 0xd7ff, 2, DrvVidRAM);
	ZetMapArea(0xd800, 0xdfff, 0, DrvShareRAM);
	ZetMapArea(0xd800, 0xdfff, 1, DrvShareRAM);
	ZetMapArea(0xd800, 0xdfff, 2, DrvShareRAM);
	ZetSetWriteHandler(markham_main_write);
	ZetSetReadHandler(markham_main_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x5fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x5fff, 2, DrvZ80ROM1);
	ZetMapArea(0x8000, 0x87ff, 0, DrvShareRAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvShareRAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvShareRAM);
	ZetSetWriteHandler(markham_sound_write);
	ZetMemEnd();
	ZetClose();

	SN76496Init(0, 4000000, 0);
	SN76496Init(1, 4000000, 1);
	SN76496SetRoute(0, 0.75, BURN_SND_ROUTE_BOTH);
	SN76496SetRoute(1, 0.75, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();
	
	SN76496Exit();

	BurnFree (AllMem);

	return 0;
}

static void draw_background_layer()
{
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs >> 5) << 3;
		INT32 sy = (offs & 0x1f) << 3;

		if (sy >=  32 && sy < 128) sx -= scroll[0];
		if (sy >= 128 && sy < 256) sx -= scroll[1];

		if (sx < -7) sx += 256;

		INT32 attr  = DrvVidRAM[offs * 2 + 0];
		INT32 code  = DrvVidRAM[offs * 2 + 1] | ((attr & 0x60) << 3);
		INT32 color = (attr & 0x1f) | ((attr & 0x80) >> 2);

		if (flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, (248 - sx) - 8, (248 - sy) - 16, color, 3, 0x200, DrvGfxROM1);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx - 8, sy - 16, color, 3, 0x200, DrvGfxROM1);
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0x60; offs < 0x100; offs +=4)
	{
		INT32 sy    = DrvSprRAM[offs + 0];
		INT32 code  = DrvSprRAM[offs + 1];
		INT32 color = DrvSprRAM[offs + 2] & 0x3f;
		INT32 sx    = DrvSprRAM[offs + 3];

		if (flipscreen)	{
			sx = (240 - sx) & 0xff;
		} else {
			sx = (sx - 2) & 0xff;
			sy = 240 - sy;
		}

		if (sx > 248) sx -= 256;

		RenderTileTranstab(pTransDraw, DrvGfxROM0, code, color << 3, 0, sx - 8, sy - 16, flipscreen, flipscreen, 16, 16, DrvColPROM + 0x300);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x400; i++) {
			INT32 d = Palette[i];
			DrvPalette[i] = BurnHighCol(d >> 16, (d >> 8) & 0xff, d & 0xff, 0);
		}
		DrvRecalc = 0;
	}

	draw_background_layer();
	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();

	{
		memset (DrvInputs, 0, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	INT32 nInterleave = 10;
	INT32 nCyclesTotal[2] = { 4000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nSegment = nCyclesTotal[0] / nInterleave;

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nSegment);
		if (i == (nInterleave - 1)) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		ZetClose();

		ZetOpen(1);
		nCyclesDone[1] += ZetRun(nSegment);
		if (i == (nInterleave - 1)) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		ZetClose();
	}

	if (pBurnSoundOut) {
		SN76496Update(0, pBurnSoundOut, nBurnSoundLen);
		SN76496Update(1, pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029707;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);

		SCAN_VAR(flipscreen);
	}

	return 0;
}


// Markham

static struct BurnRomInfo markhamRomDesc[] = {
	{ "tv3.9",	0x2000, 0x59391637, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "tvg4.10",	0x2000, 0x1837bcce, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "tvg5.11",	0x2000, 0x651da602, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "tvg1.5",	0x2000, 0xc5299766, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code
	{ "tvg2.6",	0x2000, 0xb216300a, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "tvg6.84",	0x2000, 0xab933ae5, 3 | BRF_GRA },           //  5 Sprites
	{ "tvg7.85",	0x2000, 0xce8edda7, 3 | BRF_GRA },           //  6
	{ "tvg8.86",	0x2000, 0x74d1536a, 3 | BRF_GRA },           //  7

	{ "tvg9.87",	0x2000, 0x42168675, 4 | BRF_GRA },           //  8 Tiles
	{ "tvg10.88",	0x2000, 0xfa9feb67, 4 | BRF_GRA },           //  9
	{ "tvg11.89",	0x2000, 0x71f3dd49, 4 | BRF_GRA },           // 10

	{ "14-3.99",	0x0100, 0x89d09126, 5 | BRF_GRA },           // 11 Color PROMs
	{ "14-4.100",	0x0100, 0xe1cafe6c, 5 | BRF_GRA },           // 12
	{ "14-5.101",	0x0100, 0x2d444fa6, 5 | BRF_GRA },           // 13
	{ "14-1.61",	0x0200, 0x3ad8306d, 5 | BRF_GRA },           // 14
	{ "14-2.115",	0x0200, 0x12a4f1ff, 5 | BRF_GRA },           // 15
};

STD_ROM_PICK(markham)
STD_ROM_FN(markham)

struct BurnDriver BurnDrvMarkham = {
	"markham", NULL, NULL, NULL, "1983",
	"Markham\0", NULL, "Sun Electronics", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, markhamRomInfo, markhamRomName, NULL, NULL, MarkhamInputInfo, MarkhamDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 224, 4, 3
};
