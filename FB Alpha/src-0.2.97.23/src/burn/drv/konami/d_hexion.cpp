// FB Alpha Hexion driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "z80_intf.h"
#include "k051649.h"
#include "msm6295.h"

static UINT8 *AllMem;
static UINT8 *AllRam;
static UINT8 *MemEnd;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM;
static UINT8 *DrvGfxROMExp;
static UINT8 *DrvSndROM;
static UINT8 *DrvColPROM;
static UINT8 *DrvUnkRAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvZ80RAM;
static UINT8 *flipscreen;

static UINT32  *DrvPalette;
static UINT32  *Palette;
static UINT8 DrvRecalc;

static INT32 cpubank;
static INT32 bankctrl;
static INT32 rambank;
static INT32 pmcbank;
static INT32 gfxrom_select;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static struct BurnInputInfo HexionInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Hexion)

static struct BurnDIPInfo HexionDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x4f, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    16, "Coin B"		},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0xf0, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    8, "Difficulty"		},
	{0x13, 0x01, 0x70, 0x70, "Easiest"		},
	{0x13, 0x01, 0x70, 0x60, "Very Easy"		},
	{0x13, 0x01, 0x70, 0x50, "Easy"			},
	{0x13, 0x01, 0x70, 0x40, "Medium"		},
	{0x13, 0x01, 0x70, 0x30, "Medium Hard"		},
	{0x13, 0x01, 0x70, 0x20, "Hard"			},
	{0x13, 0x01, 0x70, 0x10, "Very Hard"		},
	{0x13, 0x01, 0x70, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x14, 0x01, 0x04, 0x04, "Off"				},
	{0x14, 0x01, 0x04, 0x00, "On"				},
};

STDDIPINFO(Hexion)

static void bankswitch(INT32 data)
{
	cpubank = data & 0x0f;

	ZetMapArea(0x8000, 0x9fff, 0, DrvZ80ROM + (cpubank << 13));
	ZetMapArea(0x8000, 0x9fff, 2, DrvZ80ROM + (cpubank << 13));
}

void __fastcall hexion_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xdfff:
			bankctrl = data;
		return;

		case 0xf200:
			MSM6295Command(0, data);
		return;

		case 0xf480:
		{
			if (data & 0x40) {
				memset (DrvVidRAM + ((DrvUnkRAM[0] & 1) << 13), DrvUnkRAM[1], 0x2000);
			}

			pmcbank = data & 0x80;

			bankswitch(data);
		}
		return;

		case 0xf4c0:
			*flipscreen = data & 0x20;
		return;

		case 0xf500:
			gfxrom_select = data;
		return;
	}

	if ((address & 0xe000) == 0xc000) {
		if (bankctrl == 3 && address == 0xc000 && (data & 0xfe) == 0) {
			rambank = data & 1;
			return;
		}
		if (pmcbank) {
			if (bankctrl == 0) {
				DrvVidRAM[(rambank << 13) + (address & 0x1fff)] = data;
				return;
			}
			if (bankctrl == 2 && address < 0xc800) {
				DrvUnkRAM[address & 0x7ff] = data;
				return;
			}
		}
		return;
	}

	if ((address & 0xff80) == 0xe800) {
		K051649WaveformWrite(address & 0x7f, data);
		return;
	}

	if ((address & 0xfff0) == 0xe880) {
		if (address <= 0xe889) {
			K051649FrequencyWrite(address & 0x0f, data);
			return;
		}

		if (address == 0xe88f) {
			K051649KeyonoffWrite(data);
			return;
		}

		if (address >= 0xe88a) {
			K051649VolumeWrite(address - 0xe88a, data);
			return;
		}

		return;
	}
}

UINT8 __fastcall hexion_read(UINT16 address)
{
	switch (address)
	{
		case 0xf400:
			return DrvDips[0];

		case 0xf401:
			return DrvDips[1];

		case 0xf402:
			return DrvInputs[0];

		case 0xf403:
			return DrvInputs[1];

		case 0xf440:
			return DrvDips[2];

		case 0xf441:
			return DrvInputs[2] & 0xf7;

		case 0xf540:
			return 0; // watchdog
	}

	if ((address & 0xe000) == 0xc000) {
		if (gfxrom_select && address < 0xd000)
			return DrvGfxROM[((gfxrom_select & 0x7f) << 12) + (address & 0xfff)];

		if (bankctrl == 0)
			return DrvVidRAM[(rambank << 13) + (address & 0x1fff)];

		if (bankctrl == 2 && address < 0xd800)
			return DrvUnkRAM[address & 0x7ff];

		return 0;
	}

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	bankswitch(4);
	ZetClose();

	K051649Reset();

	cpubank = 0;
	bankctrl = 0;
	rambank = 0;
	pmcbank = 0;
	gfxrom_select = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM	= Next; Next += 0x020000;

	DrvGfxROM	= Next; Next += 0x080000;
	DrvGfxROMExp	= Next; Next += 0x100000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x040000;

	DrvColPROM	= Next; Next += 0x000300;

	DrvPalette	= (UINT32*)Next; Next += 0x0100 * sizeof(UINT32);
	Palette		= (UINT32*)Next; Next += 0x0100 * sizeof(UINT32);

	AllRam		= Next;

	DrvUnkRAM	= Next; Next += 0x000800;
	DrvVidRAM	= Next; Next += 0x006000;
	DrvZ80RAM	= Next; Next += 0x002000;

	flipscreen	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4] = { 0x000000, 0x000001, 0x000002, 0x000003 };
	INT32 XOffs[8] = { 0x200000, 0x200004, 0x000000, 0x000004, 0x200008, 0x20000c, 0x000008, 0x00000c };
	INT32 YOffs[8] = { 0x000000, 0x000010, 0x000020, 0x000030, 0x000040, 0x000050, 0x000060, 0x000070 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM, 0x80000);

	GfxDecode(0x4000, 4, 8, 8, Plane, XOffs, YOffs, 0x080, tmp, DrvGfxROMExp);

	BurnFree (tmp);

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 0x100; i++)
	{
		INT32 bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (DrvColPROM[i + 0x000] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + 0x000] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + 0x000] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + 0x000] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[i + 0x100] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + 0x100] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + 0x100] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + 0x100] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[i + 0x200] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + 0x200] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + 0x200] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + 0x200] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		Palette[i] = (r << 16) | (g << 8) | b;
	}
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
		if (BurnLoadRom(DrvZ80ROM,		0, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM,		1, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM + 0x40000,	2, 1)) return 1;

		if (BurnLoadRom(DrvSndROM,		3, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x000,	4, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x100,	5, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x200,	6, 1)) return 1;

		DrvGfxDecode();
		DrvPaletteInit();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0xa000, 0xbfff, 0, DrvZ80RAM);
	ZetMapArea(0xa000, 0xbfff, 1, DrvZ80RAM);
	ZetMapArea(0xa000, 0xbfff, 2, DrvZ80RAM);
	ZetSetWriteHandler(hexion_write);
	ZetSetReadHandler(hexion_read);
	ZetMemEnd();
	ZetClose();

	MSM6295Init(0, 1056000 / 132, 0);
	MSM6295SetRoute(0, 0.50, BURN_SND_ROUTE_BOTH);

	K051649Init(1500000);
	K051649SetRoute(0.50, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	MSM6295Exit(0);
	K051649Exit();

	ZetExit();

	BurnFree (AllMem);

	MSM6295ROM = NULL;

	return 0;
}

static void draw_layer(INT32 offset, INT32 transparent, INT32 pos)
{
	UINT8 *vram = DrvVidRAM + offset;

	for (INT32 offs = 0; offs < 64 * 32; offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 6) << 3;

		INT32 code  = vram[0] | ((vram[1] & 0x3f) << 8);
		INT32 color = vram[2] & 0x0f;
		vram += 4;

		if (*flipscreen) {
			sx ^= 0x1f8;
			sy ^= 0x0f8;

			if (transparent) {
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx-pos, sy+pos, color, 4, 0, 0, DrvGfxROMExp);
			} else {
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx-pos, sy+pos, color, 4, 0, DrvGfxROMExp);
			}

			// wrap
			if (pos ==  0) continue;
			if (sx ==   0) Render8x8Tile_Clip(pTransDraw, code, sx + 508, sy, color, 4, 0, DrvGfxROMExp);
			if (sy == 252) Render8x8Tile_Clip(pTransDraw, code, sx, sy - 256, color, 4, 0, DrvGfxROMExp);
		} else {
			if (transparent) {
				Render8x8Tile_Mask_Clip(pTransDraw, code, sx+pos, sy-pos, color, 4, 0, 0, DrvGfxROMExp);
			} else {
				Render8x8Tile_Clip(pTransDraw, code, sx+pos, sy-pos, color, 4, 0, DrvGfxROMExp);
			}

			// wrap
			if (pos ==  0) continue;
			if (sy ==   0) Render8x8Tile_Clip(pTransDraw, code, sx, sy + 252, color, 4, 0, DrvGfxROMExp);
			if (sx == 504) Render8x8Tile_Clip(pTransDraw, code, sx - 508, sy, color, 4, 0, DrvGfxROMExp);
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x100; i++) {
			INT32 rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	draw_layer(0x2000, 0, 4);
	draw_layer(0x0000, 1, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	INT32 nInterleave = nBurnSoundLen;
	INT32 nSoundBufferPos = 0;
	
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		// Clear opposites
		if ((DrvInputs[0] & 0x03) == 0) DrvInputs[0] |= 0x03;
		if ((DrvInputs[0] & 0x0c) == 0) DrvInputs[0] |= 0x0c;
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
	}
	
	ZetNewFrame();
	
	INT32 nCyclesTotal[1] = { 6000000 / 60 };
	INT32 nCyclesDone[1] = { 0 };

	ZetOpen(0);
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nNext, nCyclesSegment;

		nNext = (i + 1) * nCyclesTotal[0] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[0];
		nCyclesSegment = ZetRun(nCyclesSegment);
		if (i == (nInterleave / 3)) ZetNmi();
		if (i == ((nInterleave / 3) * 2)) ZetNmi();
		if (i == nInterleave - 1) ZetRaiseIrq(0);
		nCyclesDone[0] += nCyclesSegment;

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			K051649Update(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}
	ZetClose();
	
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			K051649Update(pSoundBuf, nSegmentLength);
		}
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
		*pnMin = 0x029705;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);

		MSM6295Scan(0, nAction);
		K051649Scan(nAction, pnMin);

		SCAN_VAR(cpubank);
		SCAN_VAR(bankctrl);
		SCAN_VAR(rambank);
		SCAN_VAR(pmcbank);
		SCAN_VAR(gfxrom_select);
	}

	ZetOpen(0);
	bankswitch(cpubank);
	ZetClose();

	return 0;
}


// Hexion (Japan ver. JAB)

static struct BurnRomInfo hexionRomDesc[] = {
	{ "122jab01.bin",	0x20000, 0xeabc6dd1, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code

	{ "122a07.bin",		0x40000, 0x22ae55e3, 2 | BRF_GRA },           //  1 Tiles
	{ "122a06.bin",		0x40000, 0x438f4388, 2 | BRF_GRA },           //  2

	{ "122a05.bin",		0x40000, 0xbcc831bf, 3 | BRF_SND },           //  3 M6296 Samples

	{ "122a04.10b",		0x00100, 0x506eb8c6, 4 | BRF_GRA },           //  4 Color PROMs
	{ "122a03.11b",		0x00100, 0x590c4f64, 4 | BRF_GRA },           //  5
	{ "122a02.13b",		0x00100, 0x5734305c, 4 | BRF_GRA },           //  6
};

STD_ROM_PICK(hexion)
STD_ROM_FN(hexion)

struct BurnDriver BurnDrvHexion = {
	"hexion", NULL, NULL, NULL, "1992",
	"Hexion (Japan ver. JAB)\0", NULL, "Konami", "GX122",
	L"Hexion\0\u30D8\u30AF\u30B7\u30AA\u30F3 (Japan ver. JAB)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_PUZZLE, 0,
	NULL, hexionRomInfo, hexionRomName, NULL, NULL, HexionInputInfo, HexionDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	512, 256, 4, 3
};
