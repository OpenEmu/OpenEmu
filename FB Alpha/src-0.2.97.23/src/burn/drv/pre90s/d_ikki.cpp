// FB Alpha Ikki / Farmers Rebellion driver module
// Based on MAME driver by Uki

#include "tiles_generic.h"
#include "z80_intf.h"
#include "sn76496.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvColPROM;
static UINT8 *DrvVidAttr;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvShareRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvVidRAM;
static UINT32 *DrvPalette;
static UINT32 *Palette;
static UINT8 *DrvTransMask;
static UINT8 DrvRecalc;

static UINT8 DrvReset;
static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[3];

static UINT8 *flipscreen;
static UINT8 *ikki_scroll;

static INT32 vblank;

static struct BurnInputInfo IkkiInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Ikki)

static struct BurnDIPInfo IkkiDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x00, NULL						},
	{0x11, 0xff, 0xff, 0x00, NULL						},

	{0   , 0xfe, 0   ,    2, "Lives"					},
	{0x10, 0x01, 0x01, 0x00, "3"						},
	{0x10, 0x01, 0x01, 0x01, "5"						},

	{0   , 0xfe, 0   ,    2, "2 Player Game"				},
	{0x10, 0x01, 0x02, 0x00, "2 Credits"					},
	{0x10, 0x01, 0x02, 0x02, "1 Credit"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"					},
	{0x10, 0x01, 0x04, 0x00, "Off"						},
	{0x10, 0x01, 0x04, 0x04, "On"						},

	{0   , 0xfe, 0   ,    16, "Coin1 / Coin2"				},
	{0x10, 0x01, 0xf0, 0x00, "1 Coin  1 Credit  / 1 Coin  1  Credit "	},
	{0x10, 0x01, 0xf0, 0x10, "2 Coins 1 Credit  / 2 Coins 1  Credit "	},
	{0x10, 0x01, 0xf0, 0x20, "2 Coins 1 Credit  / 1 Coin  3  Credits"	},
	{0x10, 0x01, 0xf0, 0x30, "1 Coin  1 Credit  / 1 Coin  2  Credits"	},
	{0x10, 0x01, 0xf0, 0x40, "1 Coin  1 Credit  / 1 Coin  3  Credits"	},
	{0x10, 0x01, 0xf0, 0x50, "1 Coin  1 Credit  / 1 Coin  4  Credits"	},
	{0x10, 0x01, 0xf0, 0x60, "1 Coin  1 Credit  / 1 Coin  5  Credits"	},
	{0x10, 0x01, 0xf0, 0x70, "1 Coin  1 Credit  / 1 Coin  6  Credits"	},
	{0x10, 0x01, 0xf0, 0x80, "1 Coin  2 Credits / 1 Coin  2  Credits"	},
	{0x10, 0x01, 0xf0, 0x90, "1 Coin  2 Credits / 1 Coin  4  Credits"	},
	{0x10, 0x01, 0xf0, 0xa0, "1 Coin  2 Credits / 1 Coin  5  Credits"	},
	{0x10, 0x01, 0xf0, 0xb0, "1 Coin  2 Credits / 1 Coin  10 Credits"	},
	{0x10, 0x01, 0xf0, 0xc0, "1 Coin  2 Credits / 1 Coin  11 Credits"	},
	{0x10, 0x01, 0xf0, 0xd0, "1 Coin  2 Credits / 1 Coin  12 Credits"	},
	{0x10, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits / 1 Coin  6  Credits"	},
	{0x10, 0x01, 0xf0, 0xf0, "Free_Play"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"					},
	{0x11, 0x01, 0x01, 0x01, "Off"						},
	{0x11, 0x01, 0x01, 0x00, "On"						},

	{0   , 0xfe, 0   ,    4, "Difficulty"					},
	{0x11, 0x01, 0x06, 0x00, "1 (Normal)"					},
	{0x11, 0x01, 0x06, 0x02, "2"						},
	{0x11, 0x01, 0x06, 0x04, "3"						},
	{0x11, 0x01, 0x06, 0x06, "4 (Difficult)"				},

	{0   , 0xfe, 0   ,    2, "Freeze"					},
	{0x11, 0x01, 0x80, 0x00, "Off"						},
	{0x11, 0x01, 0x80, 0x80, "On"						},
};

STDDIPINFO(Ikki)

void __fastcall ikki_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe008:
			*flipscreen = data & 4;
		return;

		case 0xe00a:
		case 0xe00b:
			ikki_scroll[address & 1] = data;
		return;
	}
}

UINT8 __fastcall ikki_main_read(UINT16 address)
{
	switch (address)
	{
		case 0xe000:
			return (vblank ? 2 : 0);

		case 0xe001:
			return DrvDips[0];

		case 0xe002:
			return DrvDips[1];

		case 0xe003:
			return DrvInputs[2];

		case 0xe004:
			return DrvInputs[0];

		case 0xe005:
			return DrvInputs[1];
	}

	return 0;
}

void __fastcall ikki_sub_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xd801:
			SN76496Write(0, data);
		return;

		case 0xd802:
			SN76496Write(1, data);
		return;
	}
}

UINT8 __fastcall ikki_sub_read(UINT16 /*address*/)
{
	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	return 0;
}

static void DrvPaletteInit()
{
	UINT32 *tmp = (UINT32*)BurnMalloc(0x100 * sizeof(UINT32));

	for (INT32 i = 0; i < 0x100; i++)
	{
		UINT8 r, g, b;

		r = DrvColPROM[i + 0x000] & 0x0f;
		g = DrvColPROM[i + 0x100] & 0x0f;
		b = DrvColPROM[i + 0x200] & 0x0f;

		tmp[i] = (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
	}

	DrvColPROM += 0x300;

	memset (DrvTransMask, 1, 0x200);

	for (INT32 i = 0; i < 0x200; i++)
	{
		UINT16 ctabentry = DrvColPROM[i] ^ 0xff;

		if ((i & 0x07) == 0x07 && ctabentry == 0) DrvTransMask[i] = 0;
		if ((i & 0x07) == 0x00) DrvTransMask[i] = 0; // Seems to work...

		Palette[i] = tmp[ctabentry];
	}

	for (INT32 i = 0x200; i < 0x400; i++) {
		Palette[i] = tmp[DrvColPROM[i]];
	}
	
	BurnFree(tmp);
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[3]  = { 16384*8*2,16384*8,0 };
	INT32 XOffs[16] = { 7,6,5,4,3,2,1,0,8*16+7,8*16+6,8*16+5,8*16+4,8*16+3,8*16+2,8*16+1,8*16+0 };
	INT32 YOffs[32] = { 8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7, 8*8,8*9,8*10,8*11,8*12,8*13,8*14,8*15,
		8*32,8*33,8*34,8*35,8*36,8*37,8*38,8*39, 8*40,8*41,8*42,8*43,8*44,8*45,8*46,8*47 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0xc000);

	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0xc000);

	GfxDecode(0x800, 3,  8,  8, Plane, XOffs, YOffs, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0xc000);

	GfxDecode(0x100, 3, 16, 32, Plane, XOffs, YOffs, 0x200, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x010000;
	DrvZ80ROM1	= Next; Next += 0x010000;

	DrvColPROM	= Next; Next += 0x000800;

	DrvVidAttr	= Next; Next += 0x000100;

	DrvPalette	= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);
	Palette		= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	DrvGfxROM0	= Next; Next += 0x020000;
	DrvGfxROM1	= Next; Next += 0x020000;

	DrvTransMask	= Next; Next += 0x000200;

	AllRam		= Next;

	DrvZ80RAM0	= Next; Next += 0x000800;
	DrvShareRAM	= Next; Next += 0x000800;
	DrvSprRAM	= Next; Next += 0x000800;
	DrvVidRAM	= Next; Next += 0x000800;

	ikki_scroll	= Next; Next += 0x000002;
	flipscreen	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
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
		if (BurnLoadRom(DrvZ80ROM0 + 0x0000,  0, 1)) return 1;
		memcpy (DrvZ80ROM0 + 0x8000, DrvZ80ROM0 + 0x2000, 0x2000);

		if (BurnLoadRom(DrvZ80ROM0 + 0x2000,  1, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x4000,  2, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x6000,  3, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM1 + 0x0000,  4, 1)) return 1;

		for (INT32 i = 0; i < 3; i++) {
			if (BurnLoadRom(DrvGfxROM1 + 0x4000 * i,  5 + i, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x4000 * i,  8 + i, 1)) return 1;
			if (BurnLoadRom(DrvColPROM + 0x0100 * i, 11 + i, 1)) return 1;
		}

		if (BurnLoadRom(DrvColPROM + 0x0300, 14, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x0500, 15, 1)) return 1;

		if (BurnLoadRom(DrvVidAttr + 0x0000, 16, 1)) return 1;

		DrvGfxDecode();
		DrvPaletteInit();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x9fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x9fff, 2, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80RAM0);
	ZetMapArea(0xc800, 0xcfff, 0, DrvShareRAM);
	ZetMapArea(0xc800, 0xcfff, 1, DrvShareRAM);
	ZetMapArea(0xc800, 0xcfff, 2, DrvShareRAM);
	ZetMapArea(0xd000, 0xd7ff, 0, DrvVidRAM);
	ZetMapArea(0xd000, 0xd7ff, 1, DrvVidRAM);
	ZetMapArea(0xd000, 0xd7ff, 2, DrvVidRAM);
	ZetSetWriteHandler(ikki_main_write);
	ZetSetReadHandler(ikki_main_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x1fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x1fff, 2, DrvZ80ROM1);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvSprRAM);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvSprRAM);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvSprRAM);
	ZetMapArea(0xc800, 0xcfff, 0, DrvShareRAM);
	ZetMapArea(0xc800, 0xcfff, 1, DrvShareRAM);
	ZetMapArea(0xc800, 0xcfff, 2, DrvShareRAM);
	ZetSetWriteHandler(ikki_sub_write);
	ZetSetReadHandler(ikki_sub_read);
	ZetMemEnd();
	ZetClose();

	SN76496Init(0, 8000000/4, 0);
	SN76496Init(1, 8000000/2, 1);
	SN76496SetRoute(0, 0.75, BURN_SND_ROUTE_BOTH);
	SN76496SetRoute(1, 0.75, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	SN76496Exit();
	ZetExit();

	BurnFree (AllMem);

	return 0;
}

static void draw_sprites()
{
	for (INT32 offs = 0; offs < 0x800; offs += 4)
	{
		INT32 code = (DrvSprRAM[offs + 2] & 0x80) | (DrvSprRAM[offs + 1] >> 1);
		INT32 color = DrvSprRAM[offs + 2] & 0x3f;

		INT32 sx = DrvSprRAM[offs + 3];
		INT32 sy = DrvSprRAM[offs + 0];

		if (*flipscreen)
			sx = 240 - sx;
		else
			sy = 224 - sy;

		sx &= 0xff;
		sy &= 0xff;
		if (sx > 248) sx -= 256;
		if (sy > 240) sy -= 256;

		sy -= 16;
		sx -= 8;

		{
			UINT8 *src = DrvGfxROM1 + (code << 9);
			color <<= 3;

			if (*flipscreen) {
				for (INT32 y = 31; y >= 0; y--)
				{
					INT32 yy = sy + y;

					for (INT32 x = 15; x >= 0; x--)
					{
						INT32 xx = sx + x;
						INT32 pxl = src[15-x] | color;

						if (xx < 0 || yy < 0 || xx >= nScreenWidth || yy >= nScreenHeight) continue;

						if (DrvTransMask[pxl])
							pTransDraw[(yy * nScreenWidth) + xx] = pxl;
					}

					src += 16;
				}
			} else {
				for (INT32 y = 0; y < 32; y++)
				{
					INT32 yy = sy + y;

					for (INT32 x = 0; x < 16; x++)
					{
						INT32 xx = sx + x;
						INT32 pxl = src[x] | color;

						if (xx < 0 || yy < 0 || xx >= nScreenWidth || yy >= nScreenHeight) continue;

						if (DrvTransMask[pxl])
							pTransDraw[(yy * nScreenWidth) + xx] = pxl;
					}

					src += 16;
				}
			}
		}
	}
}

static void draw_bg_layer(INT32 prio)
{
	for (INT32 offs = 0; offs < 0x800 / 2; offs++)
	{
		INT32 x = (offs >> 5) << 3;
		INT32 y = (offs & 0x1f) << 3;
		INT32 d = DrvVidAttr[x >> 3];

		if (d != 0 && d != 0x0d) {
			if (prio) continue;
		}

		INT32 color = DrvVidRAM[offs << 1];
		INT32 code = DrvVidRAM[(offs << 1) | 1] + ((color & 0xe0) << 3);
		color = (color & 0x1f) | ((color & 0x80) >> 2);

		if (d == 0x02 && prio == 0) {
			x -= ikki_scroll[1];
			if (x < 0) x += 176;

			y = (y + ~ikki_scroll[0]) & 0xff;
		}

		if (*flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, (248-x)-8, (248-y)-16, color, 3, 0x200, DrvGfxROM0);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, x-8, y-16, color, 3, 0x200, DrvGfxROM0);
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x400; i++) {
			INT32 rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	draw_bg_layer(0);
	draw_sprites();
	draw_bg_layer(1);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0x00, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		// clear opposites
		if (DrvJoy1[2] && DrvJoy1[3]) DrvInputs[0] &= ~0x0c;
		if (DrvJoy1[1] && DrvJoy1[0]) DrvInputs[0] &= ~0x03;
		if (DrvJoy2[3] && DrvJoy2[2]) DrvInputs[1] &= ~0x0c;
		if (DrvJoy2[1] && DrvJoy2[0]) DrvInputs[1] &= ~0x03;
	}

	INT32 nCycleSegment;
	INT32 nInterleave = 256;
	INT32 nCyclesTotal[2] = { 4000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	vblank = 1;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCycleSegment = (nCyclesTotal[0] - nCyclesDone[0]) / (nInterleave - i);

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nCycleSegment);
		if (i == 15) {
			vblank = 0;
			ZetRaiseIrq(0);
		}
		if (i == 239) {
			ZetRaiseIrq(0);
			vblank = 1;
		}
		ZetClose();

		nCycleSegment = (nCyclesTotal[1] - nCyclesDone[1]) / (nInterleave - i);

		ZetOpen(1);
		nCyclesDone[1] += ZetRun(nCycleSegment);
		if (i == 15) ZetRaiseIrq(0);
		if (i == 239)ZetRaiseIrq(0);
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
		*pnMin = 0x029698;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
	//	SN76496Scan(nAction, pnMin);
	}

	return 0;
}


// Ikki (Japan)

static struct BurnRomInfo ikkiRomDesc[] = {
	{ "tvg17_1",	0x4000, 0xcb28167c, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "tvg17_2",	0x2000, 0x756c7450, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "tvg17_3",	0x2000, 0x91f0a8b6, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "tvg17_4",	0x2000, 0x696fcf7d, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "tvg17_5",	0x2000, 0x22bdb40e, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 #1 Code

	{ "tvg17_6",	0x4000, 0xdc8aa269, 3 | BRF_GRA },           //  5 Sprites
	{ "tvg17_7",	0x4000, 0x0e9efeba, 3 | BRF_GRA },           //  6
	{ "tvg17_8",	0x4000, 0x45c9087a, 3 | BRF_GRA },           //  7

	{ "tvg17_9",	0x4000, 0xc594f3c5, 4 | BRF_GRA },           //  8 Background Tiles
	{ "tvg17_10",	0x4000, 0x2e510b4e, 4 | BRF_GRA },           //  9
	{ "tvg17_11",	0x4000, 0x35012775, 4 | BRF_GRA },           // 10

	{ "prom17_3",	0x0100, 0xdbcd3bec, 5 | BRF_GRA },           // 11 Color Proms
	{ "prom17_4",	0x0100, 0x9eb7b6cf, 5 | BRF_GRA },           // 12
	{ "prom17_5",	0x0100, 0x9b30a7f3, 5 | BRF_GRA },           // 13
	{ "prom17_6",	0x0200, 0x962e619d, 5 | BRF_GRA },           // 14
	{ "prom17_7",	0x0200, 0xb1f5148c, 5 | BRF_GRA },           // 15

	{ "prom17_1",	0x0100, 0xca0af30c, 6 | BRF_OPT },           // 16 Unused Proms
	{ "prom17_2",	0x0100, 0xf3c55174, 6 | BRF_OPT },           // 17
};

STD_ROM_PICK(ikki)
STD_ROM_FN(ikki)

struct BurnDriver BurnDrvIkki = {
	"ikki", NULL, NULL, NULL, "1985",
	"Ikki (Japan)\0", NULL, "Sun Electronics", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, ikkiRomInfo, ikkiRomName, NULL, NULL, IkkiInputInfo, IkkiDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 224, 4, 3
};


// Farmers Rebellion

static struct BurnRomInfo farmerRomDesc[] = {
	{ "tvg-1.10",	0x4000, 0x2c0bd392, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "tvg-2.9",	0x2000, 0xb86efe02, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "tvg-3.8",	0x2000, 0xfd686ff4, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "tvg-4.7",	0x2000, 0x1415355d, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "tvg-5.30",	0x2000, 0x22bdb40e, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 #1 Code

	{ "tvg-6.104",	0x4000, 0xdc8aa269, 3 | BRF_GRA },           //  5 Sprites
	{ "tvg-7.103",	0x4000, 0x0e9efeba, 3 | BRF_GRA },           //  6
	{ "tvg-8.102",	0x4000, 0x45c9087a, 3 | BRF_GRA },           //  7

	{ "tvg17_9",	0x4000, 0xc594f3c5, 4 | BRF_GRA },           //  8 Background Tiles
	{ "tvg17_10",	0x4000, 0x2e510b4e, 4 | BRF_GRA },           //  9
	{ "tvg17_11",	0x4000, 0x35012775, 4 | BRF_GRA },           // 10

	{ "prom17_3",	0x0100, 0xdbcd3bec, 5 | BRF_GRA },           // 11 Color Proms
	{ "prom17_4",	0x0100, 0x9eb7b6cf, 5 | BRF_GRA },           // 12
	{ "prom17_5",	0x0100, 0x9b30a7f3, 5 | BRF_GRA },           // 13
	{ "prom17_6",	0x0200, 0x962e619d, 5 | BRF_GRA },           // 14
	{ "prom17_7",	0x0200, 0xb1f5148c, 5 | BRF_GRA },           // 15

	{ "prom17_1",	0x0100, 0xca0af30c, 6 | BRF_OPT },           // 16 Unused Proms
	{ "prom17_2",	0x0100, 0xf3c55174, 6 | BRF_OPT },           // 17
};

STD_ROM_PICK(farmer)
STD_ROM_FN(farmer)

struct BurnDriver BurnDrvFarmer = {
	"farmer", "ikki", NULL, NULL, "1985",
	"Farmers Rebellion\0", NULL, "Sun Electronics", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE | GBF_SCRFIGHT, 0,
	NULL, farmerRomInfo, farmerRomName, NULL, NULL, IkkiInputInfo, IkkiDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 224, 4, 3
};
