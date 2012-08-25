// FB Alpha Gumbo driver module
// Based on MAME driver by David Haywood

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "msm6295.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvSndROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *Drv68KRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvFgRAM;
static UINT8 *DrvBgRAM;
static UINT32 *DrvPalette;

static UINT8 DrvRecalc;

static UINT8 DrvJoy1[16];
static UINT8 DrvDips[1];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static struct BurnInputInfo GumboInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 2"	},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 14,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Gumbo)

static struct BurnInputInfo DblpointInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 3"	},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 14,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Dblpoint)

static struct BurnDIPInfo GumboDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x0e, 0x01, 0x03, 0x00, "3 Coins 1 Credits"	},
	{0x0e, 0x01, 0x03, 0x01, "2 Coins 1 Credits"	},
	{0x0e, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x0e, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Helps"		},
	{0x0e, 0x01, 0x04, 0x00, "0"			},
	{0x0e, 0x01, 0x04, 0x04, "1"			},

	{0   , 0xfe, 0   ,    2, "Bonus Bar Level"	},
	{0x0e, 0x01, 0x08, 0x08, "Normal"		},
	{0x0e, 0x01, 0x08, 0x00, "High"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x0e, 0x01, 0x30, 0x20, "Easy"			},
	{0x0e, 0x01, 0x30, 0x30, "Normal"		},
	{0x0e, 0x01, 0x30, 0x10, "Hard"			},
	{0x0e, 0x01, 0x30, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Picture View"		},
	{0x0e, 0x01, 0x40, 0x40, "Off"			},
	{0x0e, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0e, 0x01, 0x80, 0x80, "Off"			},
	{0x0e, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Gumbo)

static struct BurnDIPInfo MsbingoDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x0e, 0x01, 0x03, 0x00, "3 Coins 1 Credits"	},
	{0x0e, 0x01, 0x03, 0x01, "2 Coins 1 Credits"	},
	{0x0e, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x0e, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Chance Count"		},
	{0x0e, 0x01, 0x0c, 0x0c, "0"			},
	{0x0e, 0x01, 0x0c, 0x08, "1"			},
	{0x0e, 0x01, 0x0c, 0x04, "2"			},
	{0x0e, 0x01, 0x0c, 0x00, "3"			},

	{0   , 0xfe, 0   ,    2, "Play Level"		},
	{0x0e, 0x01, 0x10, 0x10, "Normal"		},
	{0x0e, 0x01, 0x10, 0x00, "Easy"			},

	{0   , 0xfe, 0   ,    2, "Play Speed"		},
	{0x0e, 0x01, 0x20, 0x20, "Normal"		},
	{0x0e, 0x01, 0x20, 0x00, "High"			},

	{0   , 0xfe, 0   ,    2, "Left Count"		},
	{0x0e, 0x01, 0x40, 0x40, "Normal"		},
	{0x0e, 0x01, 0x40, 0x00, "Low"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0e, 0x01, 0x80, 0x80, "Off"			},
	{0x0e, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Msbingo)

static struct BurnDIPInfo MspuzzleDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xfe, NULL			},

	{0   , 0xfe, 0   ,    4, "Time Mode"		},
	{0x0e, 0x01, 0x03, 0x03, "0"			},
	{0x0e, 0x01, 0x03, 0x02, "1"			},
	{0x0e, 0x01, 0x03, 0x01, "2"			},
	{0x0e, 0x01, 0x03, 0x00, "3"			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x0e, 0x01, 0x0c, 0x00, "2 Coins 1 Credits"	},
	{0x0e, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"	},
	{0x0e, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"	},
	{0x0e, 0x01, 0x0c, 0x04, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    2, "Sound Test"		},
	{0x0e, 0x01, 0x10, 0x10, "Off"			},
	{0x0e, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "View Staff Credits"	},
	{0x0e, 0x01, 0x20, 0x20, "Off"			},
	{0x0e, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Picture View"		},
	{0x0e, 0x01, 0x40, 0x40, "Off"			},
	{0x0e, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0e, 0x01, 0x80, 0x80, "Off"			},
	{0x0e, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Mspuzzle)

static struct BurnDIPInfo DblpointDIPList[]=
{
	{0x10, 0xff, 0xff, 0xfb, NULL			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x10, 0x01, 0x03, 0x00, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0x03, 0x01, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x10, 0x01, 0x0c, 0x0c, "Easy"			},
	{0x10, 0x01, 0x0c, 0x08, "Normal"		},
	{0x10, 0x01, 0x0c, 0x04, "Hard"			},
	{0x10, 0x01, 0x0c, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    2, "Sound Test"		},
	{0x10, 0x01, 0x10, 0x10, "Off"			},
	{0x10, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Picture View"		},
	{0x10, 0x01, 0x20, 0x20, "Off"			},
	{0x10, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x10, 0x01, 0x80, 0x80, "Off"			},
	{0x10, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Dblpoint)

void __fastcall gumbo_write_word(UINT32 address, UINT16 data)
{
	switch (address)
	{
		case 0x1b0300:
		case 0x1c0300:
			MSM6295Command(0, data);
		return;
	}
}

UINT16 __fastcall gumbo_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x1b0100:
		case 0x1c0100:
			return DrvInputs[0];

		case 0x1b0200:
		case 0x1c0200:
			return DrvInputs[1];

		case 0x1b0300:
		case 0x1c0300:
			return MSM6295ReadStatus(0);
	}

	return 0;
}

void __fastcall gumbo_write_byte(UINT32, UINT8)
{
}

UINT8 __fastcall gumbo_read_byte(UINT32)
{
	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	MSM6295Reset(0);

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[8]  = { 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 XOffs0[8] = { 0, 0x800000, 8, 0x800008, 16, 0x800010, 24, 0x800018 };
	INT32 XOffs1[4] = { 0, 0x200000, 8, 0x200008 };
	INT32 YOffs0[8] = { 0, 32, 64, 96, 128, 160, 192, 224 };
	INT32 YOffs1[4] = { 0, 16, 32, 48 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x200000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x200000);

	GfxDecode(0x8000, 8, 8, 8, Plane, XOffs0, YOffs0, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x080000);

	GfxDecode(0x8000, 8, 4, 4, Plane, XOffs1, YOffs1, 0x040, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x080000;

	DrvGfxROM0	= Next; Next += 0x200000;
	DrvGfxROM1	= Next; Next += 0x080000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x040000;

	DrvPalette	= (UINT32*)Next; Next += 0x0200 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x004000;
	DrvPalRAM	= Next; Next += 0x000400;
	DrvBgRAM	= Next; Next += 0x002000;
	DrvFgRAM	= Next; Next += 0x008000;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 GumboInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	INT32 dblpoin = strncmp(BurnDrvGetTextA(DRV_NAME), "dblpoin", 7) ? 0 : 0x10000;

	{
		if (BurnLoadRom(Drv68KROM + 0x000000,	 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x000001,	 1, 2)) return 1;

		if (BurnLoadRom(DrvSndROM,		 2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,	 3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040000,	 4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,	 5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x100000,	 6, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x080000, 0x083fff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x1b0000, 0x1b03ff, SM_RAM);
	SekMapMemory(DrvBgRAM,		0x1e0000^dblpoin, 0x1e0fff^dblpoin, SM_RAM);
	SekMapMemory(DrvFgRAM,		0x1f0000^dblpoin, 0x1f3fff^dblpoin, SM_RAM);
	SekSetWriteByteHandler(0,	gumbo_write_byte);
	SekSetReadByteHandler(0,	gumbo_read_byte);
	SekSetWriteWordHandler(0,	gumbo_write_word);
	SekSetReadWordHandler(0,	gumbo_read_word);
	SekClose();

	MSM6295Init(0, 1122000 / 132, 0);
	MSM6295SetRoute(0, 0.47, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 MspuzzleInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM + 0x000000,	 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x000001,	 1, 2)) return 1;

		if (BurnLoadRom(DrvSndROM,		 2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,	 3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040000,	 4, 1)) return 1;

		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "msbingo")) {
			if (BurnLoadRom(DrvGfxROM0 + 0x000000,	 5, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x100000,	 6, 1)) return 1;
		} else {
			if (BurnLoadRom(DrvGfxROM0 + 0x000000,	 5, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x080000,	 6, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x100000,	 7, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x180000,	 8, 1)) return 1;
		}

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x100000, 0x103fff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x1a0000, 0x1a03ff, SM_RAM);
	SekMapMemory(DrvFgRAM,		0x190000, 0x197fff, SM_RAM);
	SekMapMemory(DrvBgRAM,		0x1c0000, 0x1c1fff, SM_RAM);
	SekSetWriteByteHandler(0,	gumbo_write_byte);
	SekSetReadByteHandler(0,	gumbo_read_byte);
	SekSetWriteWordHandler(0,	gumbo_write_word);
	SekSetReadWordHandler(0,	gumbo_read_word);
	SekClose();

	MSM6295Init(0, 1122000 / 132, 0);
	MSM6295SetRoute(0, 0.47, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	MSM6295Exit(0);
	SekExit();

	BurnFree (AllMem);

	MSM6295ROM = NULL;

	return 0;
}

static void draw_background()
{
	UINT16 *vram = (UINT16*)DrvBgRAM;

	for (INT32 offs = 0x80; offs < 0x780; offs++)
	{
		INT32 sx = ((offs & 0x3f) << 3) - 64;
		INT32 sy = ((offs >> 6) << 3) - 16;
		if (sx < 0 || sx >= nScreenWidth) continue;

		INT32 code = vram[offs] & 0x7fff;

		Render8x8Tile(pTransDraw, code, sx, sy, 0, 8, 0, DrvGfxROM0);
	}
}

static void draw_foreground()
{
	UINT16 *vram = (UINT16*)DrvFgRAM;

	for (INT32 offs = 0x200; offs < 0x1e00; offs++)
	{
		INT32 sx = ((offs & 0x7f) << 2) - 64;
		INT32 sy = ((offs >> 7) << 2) - 16;
		INT32 code = vram[offs] & 0x7fff;

		if (sx < 0 || sx >= nScreenWidth) continue;

		UINT16 *pDest = pTransDraw + (sy * nScreenWidth) + sx;
		UINT8  *pSrc  = DrvGfxROM1 + (code << 4);

		for (INT32 y = 0; y < 4; y++)
		{
			if (pSrc[0] != 0xff) pDest[0] = pSrc[0] | 0x100;
			if (pSrc[1] != 0xff) pDest[1] = pSrc[1] | 0x100;
			if (pSrc[2] != 0xff) pDest[2] = pSrc[2] | 0x100;
			if (pSrc[3] != 0xff) pDest[3] = pSrc[3] | 0x100;

			pDest += nScreenWidth;
			pSrc += 4;
		}
	}
}

static inline void DrvRecalcPalette()
{
	UINT8 r,g,b;
	UINT16 *pal = (UINT16*)DrvPalRAM;
	for (INT32 i = 0; i < 0x200; i++) {
		INT32 d = pal[i];

		r = (d >> 7) & 0xf8;
		g = (d >> 2) & 0xf8;
		b = (d << 3) & 0xf8;

		r |= (r >> 5);
		g |= (g >> 5);
		b |= (b >> 5);

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvRecalcPalette();
	}

	draw_background();
	draw_foreground();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = ~0;

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		}
		DrvInputs[1] = (DrvDips[0] << 8) | 0x00ff;
	}

	SekOpen(0);
	SekRun(7159090 / 60);
	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
	SekClose();

	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029702;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);

		MSM6295Scan(0, nAction);
	}

	return 0;
}


// Gumbo

static struct BurnRomInfo gumboRomDesc[] = {
	{ "u1.bin",	0x20000, 0xe09899e4, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "u2.bin",	0x20000, 0x60e59acb, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u210.bin",	0x40000, 0x16fbe06b, 2 | BRF_SND },           //  2 Samples

	{ "u512.bin",	0x20000, 0x97741798, 3 | BRF_GRA },           //  3 Foreground Tiles
	{ "u511.bin",	0x20000, 0x1411451b, 3 | BRF_GRA },           //  4

	{ "u421.bin",	0x80000, 0x42445132, 4 | BRF_GRA },           //  5 Background Tiles
	{ "u420.bin",	0x80000, 0xde1f0e2f, 4 | BRF_GRA },           //  6
};

STD_ROM_PICK(gumbo)
STD_ROM_FN(gumbo)

struct BurnDriver BurnDrvGumbo = {
	"gumbo", NULL, NULL, NULL, "1994",
	"Gumbo\0", NULL, "Min Corp.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, gumboRomInfo, gumboRomName, NULL, NULL, GumboInputInfo, GumboDIPInfo,
	GumboInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	320, 224, 4, 3
};


// Miss Puzzle (Clone of Gumbo)

static struct BurnRomInfo mspuzzlgRomDesc[] = {
	{ "u1",		0x20000, 0x95218ff1, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "u2",		0x20000, 0x7ea7d96c, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u210.bin",	0x40000, 0x16fbe06b, 2 | BRF_SND },           //  2 Samples

	{ "u512.bin",	0x20000, 0x97741798, 3 | BRF_GRA },           //  3 Foreground Tiles
	{ "u511.bin",	0x20000, 0x1411451b, 3 | BRF_GRA },           //  4

	{ "u420",	0x80000, 0x2b387153, 4 | BRF_GRA },           //  5 Background Tiles
	{ "u421",	0x80000, 0xafa06a93, 4 | BRF_GRA },           //  6
};

STD_ROM_PICK(mspuzzlg)
STD_ROM_FN(mspuzzlg)

struct BurnDriver BurnDrvMspuzzlg = {
	"mspuzzleg", "gumbo", NULL, NULL, "1994",
	"Miss Puzzle (Clone of Gumbo)\0", NULL, "Min Corp.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, mspuzzlgRomInfo, mspuzzlgRomName, NULL, NULL, GumboInputInfo, GumboDIPInfo,
	GumboInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	320, 224, 4, 3
};


// Miss Bingo

static struct BurnRomInfo msbingoRomDesc[] = {
	{ "u1.bin",	0x20000, 0x6eeb6d89, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "u2.bin",	0x20000, 0xf15dd4b5, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u210.bin",	0x40000, 0x55011f69, 2 | BRF_SND },           //  2 Samples

	{ "u512.bin",	0x40000, 0x8a46d467, 3 | BRF_GRA },           //  3 Foreground Tiles
	{ "u511.bin",	0x40000, 0xd5fd3e2e, 3 | BRF_GRA },           //  4

	{ "u421.bin",	0x80000, 0xb73f21ab, 4 | BRF_GRA },           //  5 Background Tiles
	{ "u420.bin",	0x80000, 0xc2fe9175, 4 | BRF_GRA },           //  6
};

STD_ROM_PICK(msbingo)
STD_ROM_FN(msbingo)

struct BurnDriver BurnDrvMsbingo = {
	"msbingo", NULL, NULL, NULL, "1994",
	"Miss Bingo\0", NULL, "Min Corp.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, msbingoRomInfo, msbingoRomName, NULL, NULL, GumboInputInfo, MsbingoDIPInfo,
	MspuzzleInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	320, 224, 4, 3
};


// Miss Puzzle

static struct BurnRomInfo mspuzzleRomDesc[] = {
	{ "u1.bin",	0x40000, 0xd9e63f12, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "u2.bin",	0x40000, 0x9c3fc677, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u210.bin",	0x40000, 0x0a223a38, 2 | BRF_SND },           //  2 Samples

	{ "u512.bin",	0x40000, 0x505ee3c2, 3 | BRF_GRA },           //  3 Foreground Tiles
	{ "u511.bin",	0x40000, 0x3d6b6c78, 3 | BRF_GRA },           //  4

	{ "u421.bin",	0x80000, 0x5387ab3a, 4 | BRF_GRA },           //  5 Background Tiles
	{ "u425.bin",	0x80000, 0xf53a9042, 4 | BRF_GRA },           //  6
	{ "u420.bin",	0x80000, 0xc3f892e6, 4 | BRF_GRA },           //  7
	{ "u426.bin",	0x80000, 0xc927e8da, 4 | BRF_GRA },           //  8
};

STD_ROM_PICK(mspuzzle)
STD_ROM_FN(mspuzzle)

struct BurnDriver BurnDrvMspuzzle = {
	"mspuzzle", NULL, NULL, NULL, "1994",
	"Miss Puzzle\0", NULL, "Min Corp.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, mspuzzleRomInfo, mspuzzleRomName, NULL, NULL, GumboInputInfo, MspuzzleDIPInfo,
	MspuzzleInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 320, 3, 4
};


// Miss Puzzle (Nudes)

static struct BurnRomInfo mspuzzlenRomDesc[] = {
	{ "u1.rom",	0x20000, 0xec940df4, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "u2.rom",	0x20000, 0x7b9cac82, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u210.rom",	0x40000, 0x8826b018, 2 | BRF_SND },  		           //  2 Samples

	{ "u512.bin",	0x40000, 0x505ee3c2, 3 | BRF_GRA },		           //  3 Foreground Tiles
	{ "u511.bin",	0x40000, 0x3d6b6c78, 3 | BRF_GRA },   		           //  4

	{ "u421.rom",	0x80000, 0x3c567c55, 4 | BRF_GRA },     		   //  5 Background Tiles
	{ "u425.rom",	0x80000, 0x1c4c8fc1, 4 | BRF_GRA },           //  6
	{ "u420.rom",	0x80000, 0xf52ab7fd, 4 | BRF_GRA }, 		           //  7
	{ "u426.rom",	0x80000, 0xc28b2743, 4 | BRF_GRA },		           //  8
};

STD_ROM_PICK(mspuzzlen)
STD_ROM_FN(mspuzzlen)

struct BurnDriverD BurnDrvMspuzzlen = {
	"mspuzzlen", "mspuzzle", NULL, NULL, "1994",
	"Miss Puzzle (Nudes)\0", NULL, "Min Corp.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, mspuzzlenRomInfo, mspuzzlenRomName, NULL, NULL, GumboInputInfo, MspuzzleDIPInfo,
	MspuzzleInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 320, 3, 4
};


// Double Point

static struct BurnRomInfo dblpointRomDesc[] = {
	{ "u1.bin",	0x20000, 0xb05c9e02, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "u2.bin",	0x20000, 0xcab35cbe, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u210.rom",	0x40000, 0xd35f975c, 2 | BRF_SND },           //  2 Samples

	{ "u512.bin",	0x40000, 0xb57b8534, 3 | BRF_GRA },           //  3 Foreground Tiles
	{ "u511.bin",	0x40000, 0x74ed13ff, 3 | BRF_GRA },           //  4

	{ "u421.bin",	0x80000, 0xb0e9271f, 4 | BRF_GRA },           //  5 Background Tiles
	{ "u420.bin",	0x80000, 0x252789e8, 4 | BRF_GRA },           //  6
};

STD_ROM_PICK(dblpoint)
STD_ROM_FN(dblpoint)

struct BurnDriver BurnDrvDblpoint = {
	"dblpoint", NULL, NULL, NULL, "1995",
	"Double Point\0", NULL, "Min Corp.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, dblpointRomInfo, dblpointRomName, NULL, NULL, DblpointInputInfo, DblpointDIPInfo,
	GumboInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	320, 224, 4, 3
};


// Double Point (Dong Bang Electron, bootleg?)

static struct BurnRomInfo dblpoindRomDesc[] = {
	{ "d12.bin",	0x20000, 0x44bc1bd9, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "d13.bin",	0x20000, 0x625a311b, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "d11.bin",	0x40000, 0xd35f975c, 2 | BRF_SND },           //  2 Samples

	{ "d14.bin",	0x40000, 0x41943db5, 3 | BRF_GRA },           //  3 Foreground Tiles
	{ "d15.bin",	0x40000, 0x6b899a51, 3 | BRF_GRA },           //  4

	{ "d16.bin",	0x80000, 0xafea0158, 4 | BRF_GRA },           //  5 Background Tiles
	{ "d17.bin",	0x80000, 0xc971dcb5, 4 | BRF_GRA },           //  6
};

STD_ROM_PICK(dblpoind)
STD_ROM_FN(dblpoind)

struct BurnDriver BurnDrvDblpoind = {
	"dblpointd", "dblpoint", NULL, NULL, "1995",
	"Double Point (Dong Bang Electron, bootleg?)\0", NULL, "Dong Bang Electron", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, dblpoindRomInfo, dblpoindRomName, NULL, NULL, DblpointInputInfo, DblpointDIPInfo,
	GumboInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	320, 224, 4, 3
};

