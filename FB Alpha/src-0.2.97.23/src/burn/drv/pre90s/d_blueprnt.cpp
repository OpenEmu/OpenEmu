// FB Alpha Blue Print driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "zet.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *RamEnd;
static UINT8 *AllRam;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvColRAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvScrollRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT32 *DrvPalette;
static INT16 *pAY8910Buffer[6];

static UINT8 *watchdog;
static UINT8 *dipsw;
static UINT8 *soundlatch;
static UINT8 *flipscreen;
static UINT8 *gfx_bank;

static UINT8 DrvReset;
static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[2];

static UINT8 DrvRecalc;

static struct BurnInputInfo BlueprntInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy1 + 2,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Blueprnt)

static struct BurnDIPInfo BlueprntDIPList[]=
{
	{0x11, 0xff, 0xff, 0xc3, NULL				},
	{0x12, 0xff, 0xff, 0xd5, NULL				},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x11, 0x01, 0x06, 0x00, "20K"				},
	{0x11, 0x01, 0x06, 0x02, "30K"				},
	{0x11, 0x01, 0x06, 0x04, "40K"				},
	{0x11, 0x01, 0x06, 0x06, "50K"				},

	{0   , 0xfe, 0   ,    2, "Free Play"			},
	{0x11, 0x01, 0x08, 0x00, "Off"				},
	{0x11, 0x01, 0x08, 0x08, "On"				},

	{0   , 0xfe, 0   ,    2, "Maze Monster Appears In"	},
	{0x11, 0x01, 0x10, 0x00, "2nd Maze"			},
	{0x11, 0x01, 0x10, 0x10, "3rd Maze"			},

	{0   , 0xfe, 0   ,    2, "Coin A"			},
	{0x11, 0x01, 0x20, 0x20, "2 Coins 1 Credits "		},
	{0x11, 0x01, 0x20, 0x00, "1 Coin 1 Credits "		},

	{0   , 0xfe, 0   ,    2, "Coin B"			},
	{0x11, 0x01, 0x40, 0x40, "1 Coin 3 Credits "		},
	{0x11, 0x01, 0x40, 0x00, "1 Coin 5 Credits "		},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x12, 0x01, 0x03, 0x00, "2"				},
	{0x12, 0x01, 0x03, 0x01, "3"				},
	{0x12, 0x01, 0x03, 0x02, "4"				},
	{0x12, 0x01, 0x03, 0x03, "5"				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x12, 0x01, 0x08, 0x00, "Upright"			},
	{0x12, 0x01, 0x08, 0x08, "Cocktail"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x12, 0x01, 0x30, 0x00, "Level 1"			},
	{0x12, 0x01, 0x30, 0x10, "Level 2"			},
	{0x12, 0x01, 0x30, 0x20, "Level 3"			},
	{0x12, 0x01, 0x30, 0x30, "Level 4"			},
};

STDDIPINFO(Blueprnt)

static struct BurnInputInfo SaturnInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Saturn)

static struct BurnDIPInfo SaturnDIPList[]=
{
	{0x11, 0xff, 0xff, 0x00, NULL				},
	{0x12, 0xff, 0xff, 0x04, NULL				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x11, 0x01, 0x02, 0x00, "Upright"			},
	{0x11, 0x01, 0x02, 0x02, "Cocktail"			},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x11, 0x01, 0xc0, 0x00, "3"				},
	{0x11, 0x01, 0xc0, 0x40, "4"				},
	{0x11, 0x01, 0xc0, 0x80, "5"				},
	{0x11, 0x01, 0xc0, 0xc0, "6"				},

	{0   , 0xfe, 0   ,    2, "Coinage"			},
	{0x12, 0x01, 0x02, 0x02, "A 2/1 B 1/3"			},
	{0x12, 0x01, 0x02, 0x00, "A 1/1 B 1/6"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x12, 0x01, 0x04, 0x00, "Off"				},
	{0x12, 0x01, 0x04, 0x04, "On"				},
};

STDDIPINFO(Saturn)

void __fastcall blueprint_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xd000:
		{
			*soundlatch = data;
			ZetClose();
			ZetOpen(1);
			ZetNmi();
			ZetClose();
			ZetOpen(0);
		}
		return;

		case 0xe000:
		{
			*flipscreen = ~data & 2;
			*gfx_bank   = (data >> 2) & 1;
		}

		return;
	}
}

UINT8 __fastcall blueprint_read(UINT16 address)
{
	switch (address)
	{
		case 0xc000:
		case 0xc001:
			return DrvInputs[address & 1];

		case 0xc003:
			return *dipsw;

		case 0xe000:
			*watchdog = 0;
			return 0;
	}

	return 0;
}

void __fastcall blueprint_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x6000:
			AY8910Write(0, 0, data);
		return;

		case 0x6001:
			AY8910Write(0, 1, data);
		return;

		case 0x8000:
			AY8910Write(1, 0, data);
		return;

		case 0x8001:
			AY8910Write(1, 1, data);
		return;
	}
}

UINT8 __fastcall blueprint_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x6002:
			return AY8910Read(0); 

		case 0x8002:
			return AY8910Read(1); 
	}

	return 0;
}

UINT8 ay8910_0_read_port_1(UINT32)
{
	return *soundlatch;
}

void ay8910_0_write_port_0(UINT32, UINT32 data)
{
	*dipsw = data & 0xff;
}

UINT8 ay8910_1_read_port_0(UINT32)
{
	return DrvDips[0];
}

UINT8 ay8910_1_read_port_1(UINT32)
{
	return DrvDips[1];
}

static void palette_init()
{
	for (INT32 i = 0; i < 512 + 8; i++)
	{
		UINT8 pen;

		if (i < 0x200)
			pen = ((i & 0x100) >> 5) | ((i & 0x002) ? ((i & 0x0e0) >> 5) : 0) | ((i & 0x001) ? ((i & 0x01c) >> 2) : 0);
		else
			pen = i - 0x200;

		INT32 r = ((pen >> 0) & 1) * (((pen & 8) >> 1) ^ 0xff);
		INT32 g = ((pen >> 2) & 1) * (((pen & 8) >> 1) ^ 0xff);
		INT32 b = ((pen >> 1) & 1) * (((pen & 8) >> 1) ^ 0xff);

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[3] = { (0x1000 * 8) * 2, (0x1000 * 8) * 1, (0x1000 * 8) * 0 };
	INT32 XOffs[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 YOffs[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x03000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x02000);

	GfxDecode(0x0200, 2,  8,  8, Plane + 1, XOffs, YOffs, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x03000);

	GfxDecode(0x0100, 3,  8, 16, Plane + 0, XOffs, YOffs, 0x080, tmp, DrvGfxROM1);

	BurnFree (tmp);

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

	AY8910Reset(0);
	AY8910Reset(1);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x010000;
	DrvZ80ROM1	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x008000;
	DrvGfxROM1	= Next; Next += 0x008000;

	DrvPalette	= (UINT32*)Next; Next += 0x0208 * sizeof(UINT32);

	pAY8910Buffer[0] = (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[1] = (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[2] = (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[3] = (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[4] = (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[5] = (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);

	AllRam		= Next;

	DrvColRAM	= Next; Next += 0x000400;
	DrvVidRAM	= Next; Next += 0x000400;
	DrvScrollRAM	= Next; Next += 0x000100;
	DrvSprRAM	= Next; Next += 0x000100;

	DrvZ80RAM0	= Next; Next += 0x000800;
	DrvZ80RAM1	= Next; Next += 0x000800;

	dipsw		= Next; Next += 0x000001;
	soundlatch	= Next; Next += 0x000001;
	flipscreen	= Next; Next += 0x000001;
	gfx_bank	= Next; Next += 0x000001;
	watchdog	= Next; Next += 0x000001;

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
		INT32 lofst = 0;

		if (BurnLoadRom(DrvZ80ROM0 + 0x0000,	0, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x1000,	1, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x2000,	2, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x3000,	3, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x4000,	4, 1)) return 1;

		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "saturnzi")) {
			if (BurnLoadRom(DrvZ80ROM0 + 0x5000,	5, 1)) return 1;
			lofst = 1;
		}

		if (BurnLoadRom(DrvZ80ROM1 + 0x0000,	5+lofst, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM1 + 0x2000,	6+lofst, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x0000,	7+lofst, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x1000,	8+lofst, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x0000,	9+lofst, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x1000,	10+lofst,1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x2000,	11+lofst,1)) return 1;

		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM0);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM0);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM0);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM0);
	ZetMapArea(0x9000, 0x93ff, 0, DrvVidRAM);
	ZetMapArea(0x9000, 0x93ff, 1, DrvVidRAM);
	ZetMapArea(0x9000, 0x93ff, 2, DrvVidRAM);
	ZetMapArea(0x9400, 0x97ff, 0, DrvVidRAM);
	ZetMapArea(0x9400, 0x97ff, 1, DrvVidRAM);
	ZetMapArea(0x9400, 0x97ff, 2, DrvVidRAM);
	ZetMapArea(0xa000, 0xa0ff, 0, DrvScrollRAM);
	ZetMapArea(0xa000, 0xa0ff, 1, DrvScrollRAM);
	ZetMapArea(0xa000, 0xa0ff, 2, DrvScrollRAM);
	ZetMapArea(0xb000, 0xb0ff, 0, DrvSprRAM);
	ZetMapArea(0xb000, 0xb0ff, 1, DrvSprRAM);
	ZetMapArea(0xb000, 0xb0ff, 2, DrvSprRAM);
	ZetMapArea(0xf000, 0xf3ff, 0, DrvColRAM);
	ZetMapArea(0xf000, 0xf3ff, 1, DrvColRAM);
	ZetMapArea(0xf000, 0xf3ff, 2, DrvColRAM);
	ZetSetWriteHandler(blueprint_write);
	ZetSetReadHandler(blueprint_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x2fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x2fff, 2, DrvZ80ROM1);
	ZetMapArea(0x4000, 0x43ff, 0, DrvZ80RAM1);
	ZetMapArea(0x4000, 0x43ff, 1, DrvZ80RAM1);
	ZetMapArea(0x4000, 0x43ff, 2, DrvZ80RAM1);
	ZetSetWriteHandler(blueprint_sound_write);
	ZetSetReadHandler(blueprint_sound_read);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 625000, nBurnSoundRate, NULL, &ay8910_0_read_port_1, &ay8910_0_write_port_0, NULL);
	AY8910Init(1, 625000, nBurnSoundRate, &ay8910_1_read_port_0, &ay8910_1_read_port_1, NULL, NULL);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();

	AY8910Exit(0);
	AY8910Exit(1);

	BurnFree (AllMem);

	return 0;
}

static void draw_layer(INT32 prio)
{
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 attr = DrvColRAM[offs];
		if ((attr >> 7) != prio) continue;

		INT32 code = DrvVidRAM[offs] | (*gfx_bank << 8);
		INT32 color = attr & 0x7f;

		INT32 sx = (~offs & 0x3e0) >> 2;
		INT32 sy = (offs & 0x1f) << 3;

		sy -= DrvScrollRAM[(30 + *flipscreen) - (sx >> 3)];
		if (sy < -7) sy += 0x100;

		if (*flipscreen) {
			Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx ^ 0xf8, (248 - sy) - 16, color, 2, 0, 0, DrvGfxROM0);
		} else {
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy - 16, color, 2, 0, 0, DrvGfxROM0);
		}
	}
}

static void draw_8x16_tile(INT32 code, INT32 sx, INT32 sy, INT32 fx, INT32 fy)
{
	UINT8 *src = DrvGfxROM1 + (code << 7);

	for (INT32 y = 0; y < 16; y++)
	{
		INT32 yy = sy + y;

		for (INT32 x = 0; x < 8; x++)
		{
			INT32 xx = sx + x;
			INT32 d = src[((y ^ fy) << 3) | (x ^ fx)];

			if (yy < 0 || xx < 0 || yy >= nScreenHeight || xx >= nScreenWidth || !d) continue;

			pTransDraw[(yy * nScreenWidth) + xx] = d | 0x200;
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0; offs < 0x100; offs += 4)
	{
		INT32 code  = DrvSprRAM[offs + 1];
		INT32 sx    = DrvSprRAM[offs + 3];
		INT32 sy    = 240 - DrvSprRAM[offs];
		INT32 flipx = (DrvSprRAM[offs + 2] >> 6) & 1;
		INT32 flipy = (DrvSprRAM[offs + 2 - 4] >> 7) & 1;

		if (*flipscreen)
		{
			sx = 248 - sx;
			sy = 240 - sy;
			flipx ^= 1;
			flipy ^= 1;
		}

		draw_8x16_tile(code, sx+2, sy-17, flipx * 0x07, flipy * 0x0f);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		palette_init();
	}

	memset (pTransDraw, 0, nScreenWidth * nScreenHeight * 2);

	draw_layer(0);
	draw_sprites();
	draw_layer(1);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		if (*watchdog > 180) {
			DrvDoReset();
		}

		*watchdog = *watchdog + 1;
	}

	{
		DrvInputs[0] = DrvInputs[1] = 0;
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] |= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] |= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nSegment;
	INT32 nInterleave = 256;
	INT32 nTotalCycles[2] = { 3500000 / 60, 625000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nSegment = (nTotalCycles[0] - nCyclesDone[0]) / (nInterleave - i);

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nSegment);
		if (i == 255) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		ZetClose();

		nSegment = (nTotalCycles[1] - nCyclesDone[1]) / (nInterleave - i);

		ZetOpen(1);
		nCyclesDone[1] += ZetRun(nSegment);
		if ((i & 0x3f) == 0x3f) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
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
		*pnMin = 0x029698;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		AY8910Scan(nAction, pnMin);
	}

	return 0;
}


// Blue Print (Midway)

static struct BurnRomInfo blueprntRomDesc[] = {
	{ "bp-1.1m",	0x1000, 0xb20069a6, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "bp-2.1n",	0x1000, 0x4a30302e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "bp-3.1p",	0x1000, 0x6866ca07, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "bp-4.1r",	0x1000, 0x5d3cfac3, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "bp-5.1s",	0x1000, 0xa556cac4, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "snd-1.3u",	0x1000, 0xfd38777a, 2 | BRF_PRG | BRF_ESS }, //  5 Z80 Code
	{ "snd-2.3v",	0x1000, 0x33d5bf5b, 2 | BRF_PRG | BRF_ESS }, //  6

	{ "bg-1.3c",	0x1000, 0xac2a61bc, 3 | BRF_GRA           }, //  7 Background Tiles
	{ "bg-2.3d",	0x1000, 0x81fe85d7, 3 | BRF_GRA           }, //  8

	{ "red.17d",	0x1000, 0xa73b6483, 4 | BRF_GRA           }, //  9 Sprites
	{ "blue.18d",	0x1000, 0x7d622550, 4 | BRF_GRA           }, // 10
	{ "green.20d",	0x1000, 0x2fcb4f26, 4 | BRF_GRA           }, // 11
};

STD_ROM_PICK(blueprnt)
STD_ROM_FN(blueprnt)

struct BurnDriver BurnDrvBlueprnt = {
	"blueprnt", NULL, NULL, NULL, "1982",
	"Blue Print (Midway)\0", NULL, "[Zilec Electronics] Bally Midway", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, blueprntRomInfo, blueprntRomName, NULL, NULL, BlueprntInputInfo, BlueprntDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x208,
	224, 256, 3, 4
};


// Blue Print (Jaleco)

static struct BurnRomInfo blueprnjRomDesc[] = {
	{ "bp-1j.1m",	0x1000, 0x2e746693, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "bp-2j.1n",	0x1000, 0xa0eb0b8e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "bp-3j.1p",	0x1000, 0xc34981bb, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "bp-4j.1r",	0x1000, 0x525e77b5, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "bp-5j.1s",	0x1000, 0x431a015f, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "snd-1.3u",	0x1000, 0xfd38777a, 2 | BRF_PRG | BRF_ESS }, //  5 Z80 Code
	{ "snd-2.3v",	0x1000, 0x33d5bf5b, 2 | BRF_PRG | BRF_ESS }, //  6

	{ "bg-1j.3c",	0x0800, 0x43718c34, 3 | BRF_GRA           }, //  7 Background Tiles
	{ "bg-2j.3d",	0x0800, 0xd3ce077d, 3 | BRF_GRA           }, //  8

	{ "redj.17d",	0x1000, 0x83da108f, 4 | BRF_GRA           }, //  9 Sprites
	{ "bluej.18d",	0x1000, 0xb440f32f, 4 | BRF_GRA           }, // 10
	{ "greenj.20d",	0x1000, 0x23026765, 4 | BRF_GRA           }, // 11
};

STD_ROM_PICK(blueprnj)
STD_ROM_FN(blueprnj)

struct BurnDriver BurnDrvBlueprnj = {
	"blueprntj", "blueprnt", NULL, NULL, "1982",
	"Blue Print (Jaleco)\0", NULL, "[Zilec Electronics] Jaleco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, blueprnjRomInfo, blueprnjRomName, NULL, NULL, BlueprntInputInfo, BlueprntDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x208,
	224, 256, 3, 4
};


// Saturn

static struct BurnRomInfo saturnziRomDesc[] = {
	{ "r1",		0x1000, 0x18a6d68e, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "r2",		0x1000, 0xa7dd2665, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "r3",		0x1000, 0xb9cfa791, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "r4",		0x1000, 0xc5a997e7, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "r5",		0x1000, 0x43444d00, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "r6",		0x1000, 0x4d4821f6, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "r7",		0x1000, 0xdd43e02f, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 Code
	{ "r8",		0x1000, 0x7f9d0877, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "r10",	0x1000, 0x35987d61, 3 | BRF_GRA           }, //  8 Background Tiles
	{ "r9",		0x1000, 0xca6a7fda, 3 | BRF_GRA           }, //  9

	{ "r11",	0x1000, 0x6e4e6e5d, 4 | BRF_GRA           }, // 10 Sprites
	{ "r12",	0x1000, 0x46fc049e, 4 | BRF_GRA           }, // 11
	{ "r13",	0x1000, 0x8b3e8c32, 4 | BRF_GRA           }, // 12
};

STD_ROM_PICK(saturnzi)
STD_ROM_FN(saturnzi)

struct BurnDriver BurnDrvSaturnzi = {
	"saturnzi", NULL, NULL, NULL, "1983",
	"Saturn\0", NULL, "[Zilec Electronics] Jaleco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, saturnziRomInfo, saturnziRomName, NULL, NULL, SaturnInputInfo, SaturnDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x208,
	224, 256, 3, 4
};
