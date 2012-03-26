// FB Alpha Ultraman driver module
// Based on MAME driver by Manuel Abadia

#include "tiles_generic.h"
#include "sek.h"
#include "zet.h"
#include "burn_ym2151.h"
#include "msm6295.h"
#include "konamiic.h"

static UINT8 *AllMem;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvGfxROMExp2;
static UINT8 *DrvGfxROMExp3;
static UINT8 *DrvSndROM;
static UINT8 *AllRam;
static UINT8 *Drv68KRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;
static UINT8 *soundlatch;
static UINT8 *RamEnd;
static UINT8 *MemEnd;

static UINT32  *DrvPalette;
static UINT8 DrvRecalc;

static INT32 bank0;
static INT32 bank1;
static INT32 bank2;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT8 DrvInputs[3];

static struct BurnInputInfo UltramanInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy3 + 7,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Diagnostics",		BIT_DIGITAL,	DrvJoy1 + 4,	"diag"	},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 5,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Ultraman)

static struct BurnDIPInfo UltramanDIPList[]=
{
	{0x15, 0xff, 0xff, 0xff, NULL			},
	{0x16, 0xff, 0xff, 0x74, NULL			},

	{0   , 0xfe, 0   ,   16, "Coin A"		},
	{0x15, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x15, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x15, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x15, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x15, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x15, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x15, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x15, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x15, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,   16, "Coin B"		},
	{0x15, 0x01, 0xf0, 0x20, "4 Coins 1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x50, "3 Coins 1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x80, "2 Coins 1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x15, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x15, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x15, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x15, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x15, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x15, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x15, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},
	{0x15, 0x01, 0xf0, 0x00, "No Coin B"		},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x16, 0x01, 0x04, 0x00, "No"			},
	{0x16, 0x01, 0x04, 0x04, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x16, 0x01, 0x08, 0x08, "Off"			},
	{0x16, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x30, 0x10, "Easy"			},
	{0x16, 0x01, 0x30, 0x30, "Normal"		},
	{0x16, 0x01, 0x30, 0x20, "Hard"			},
	{0x16, 0x01, 0x30, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    2, "Upright Controls"	},
	{0x16, 0x01, 0x40, 0x40, "Single"		},
	{0x16, 0x01, 0x40, 0x00, "Dual"			},

//	{0   , 0xfe, 0   ,    2, "Cabinet"		},
//	{0x16, 0x01, 0x80, 0x00, "Upright"		},
//	{0x16, 0x01, 0x80, 0x80, "Cocktail"		},
};

STDDIPINFO(Ultraman)

void __fastcall ultraman_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x1c0019:
		{
			if (((data & 0x02) >> 1) != bank0) {
				bank0 = (data & 0x02) >> 1;
				K051316RedrawTiles(0);
			}

			if (((data & 0x08) >> 3) != bank1) {
				bank1 = (data & 0x08) >> 3;
				K051316RedrawTiles(1);
			}

			if (((data & 0x20) >> 5) != bank2) {
				bank2 = (data & 0x20) >> 5;
				K051316RedrawTiles(2);
			}

			K051316WrapEnable(0,data & 0x01);
			K051316WrapEnable(1,data & 0x04);
			K051316WrapEnable(2,data & 0x10);
		}
		return;

		case 0x1c0021:
			ZetNmi();
		return;

		case 0x1c0029:
			*soundlatch = data;
		return;

		case 0x1c0031: // watchdog
		return;
	}

	if ((address & 0xfff001) == 0x204001) {
		K051316Write(0, (address >> 1) & 0x7ff, data);
		return;
	}

	if ((address & 0xfff001) == 0x205001) {
		K051316Write(1, (address >> 1) & 0x7ff, data);
		return;
	}

	if ((address & 0xfff001) == 0x206001) {
		K051316Write(2, (address >> 1) & 0x7ff, data);
		return;
	}

	if ((address & 0xffffe1) == 0x207f81) {
		K051316WriteCtrl(0, (address >> 1) & 0x0f, data);
		return;
	}

	if ((address & 0xffffe1) == 0x207fa1) {
		K051316WriteCtrl(1, (address >> 1) & 0x0f, data);
		return;
	}

	if ((address & 0xffffe1) == 0x207fc1) {
		K051316WriteCtrl(2, (address >> 1) & 0x0f, data);
		return;
	}

	if ((address & 0xfffff1) == 0x304001) {
		K051937Write((address >> 1) & 0x007, data);
		return;
	}

	if ((address & 0xfff801) == 0x304801) {
		K051960Write((address >> 1) & 0x3ff, data);
		return;
	}

}

UINT8 __fastcall ultraman_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x1c0001:
			return DrvInputs[0];

		case 0x1c0003:
			return DrvInputs[1];

		case 0x1c0005:
			return DrvInputs[2];

		case 0x1c0007:
			return DrvDips[0];

		case 0x1c0009:
			return DrvDips[1];
	}

	if ((address & 0xfff000) == 0x204000) {
		return K051316Read(0, (address >> 1) & 0x7ff);
	}

	if ((address & 0xfff000) == 0x205000) {
		return K051316Read(1, (address >> 1) & 0x7ff);
	}

	if ((address & 0xfff000) == 0x206000) {
		return K051316Read(2, (address >> 1) & 0x7ff);
	}

	if ((address & 0xfffff0) == 0x304000) {
		return K051937Read((address >> 1) & 0x007);
	}

	if ((address & 0xfff800) == 0x304800) {
		return K051960Read((address >> 1) & 0x3ff);
	}

	return 0;
}

void __fastcall ultraman_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xd000:
		return;

		case 0xe000:
			MSM6295Command(0, data);
		return;

		case 0xf000:
			BurnYM2151SelectRegister(data);
		return;

		case 0xf001:
			BurnYM2151WriteRegister(data);
		return;
	}
}

UINT8 __fastcall ultraman_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xc000:
			return *soundlatch;

		case 0xe000:
			return MSM6295ReadStatus(0);

		case 0xf000:
		case 0xf001:
			return BurnYM2151ReadStatus();
	}

	return 0;
}

static void K051960Callback(INT32 *, INT32 *color, INT32 *priority, INT32 *shadow)
{
	*priority = (*color & 0x80) >> 7;
	*color = 0xc0 + ((*color & 0x7e) >> 1);
	*shadow = 0;
}

static void K051316Callback0(INT32 *code, INT32 *color, INT32 *)
{
	*code |= ((*color & 0x07) << 8) | (bank0 << 11);
	*color = ((*color & 0xf8) >> 3);
}

static void K051316Callback1(INT32 *code, INT32 *color, INT32 *)
{
	*code |= ((*color & 0x07) << 8) | (bank1 << 11);
	*color = 0x40 + ((*color & 0xf8) >> 3);
}

static void K051316Callback2(INT32 *code, INT32 *color, INT32 *)
{
	*code |= ((*color & 0x07) << 8) | (bank2 << 11);
	*color = 0x80 + ((*color & 0xf8) >> 3);
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2151Reset();
	MSM6295Reset(0);

	KonamiICReset();

	bank0 = bank1 = bank2 = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x040000;
	DrvZ80ROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x100000;
	DrvGfxROM1	= Next; Next += 0x080000;
	DrvGfxROM2	= Next; Next += 0x080000;
	DrvGfxROM3	= Next; Next += 0x080000;
	DrvGfxROMExp0	= Next; Next += 0x200000;
	DrvGfxROMExp1	= Next; Next += 0x100000;
	DrvGfxROMExp2	= Next; Next += 0x100000;
	DrvGfxROMExp3	= Next; Next += 0x100000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x040000;

	DrvPalette	= (UINT32*)Next; Next += 0x2000 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvPalRAM	= Next; Next += 0x004000;

	DrvZ80RAM	= Next; Next += 0x004000;

	soundlatch	= Next; Next += 0x000001;

	RamEnd		= Next;
	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[4]  = { 0x000, 0x008, 0x010, 0x018 };
	INT32 XOffs0[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			   0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107 };
	INT32 YOffs0[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
			   0x200, 0x220, 0x240, 0x260, 0x280, 0x2a0, 0x2c0, 0x2e0 };
	INT32 Plane1[4]  = { 0x000, 0x001, 0x002, 0x003 };
	INT32 XOffs1[16] = { 0x000, 0x004, 0x008, 0x00c, 0x010, 0x014, 0x018, 0x01c,
			   0x020, 0x024, 0x028, 0x02c, 0x030, 0x034, 0x038, 0x03c };
	INT32 YOffs1[16] = { 0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
			   0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0 };

	konami_rom_deinterleave_2(DrvGfxROM0, 0x100000);

	GfxDecode(0x02000, 4, 16, 16, Plane0, XOffs0, YOffs0, 0x400, DrvGfxROM0, DrvGfxROMExp0);
	GfxDecode(0x01000, 4, 16, 16, Plane1, XOffs1, YOffs1, 0x400, DrvGfxROM1, DrvGfxROMExp1);
	GfxDecode(0x01000, 4, 16, 16, Plane1, XOffs1, YOffs1, 0x400, DrvGfxROM2, DrvGfxROMExp2);
	GfxDecode(0x01000, 4, 16, 16, Plane1, XOffs1, YOffs1, 0x400, DrvGfxROM3, DrvGfxROMExp3);

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
		if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x080000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x020000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x060000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x020000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x040000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x060000, 12, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x020000, 14, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x040000, 15, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x060000, 16, 1)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000, 17, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x080000, 0x08ffff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x180000, 0x183fff, SM_RAM);
	SekSetWriteByteHandler(0,	ultraman_write_byte);
	SekSetReadByteHandler(0,	ultraman_read_byte);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0xbfff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80RAM);
	ZetSetWriteHandler(ultraman_sound_write);
	ZetSetReadHandler(ultraman_sound_read);
	ZetMemEnd();
	ZetClose();

	K051960Init(DrvGfxROM0, 0xfffff);
	K051960SetCallback(K051960Callback);
	K051960SetSpriteOffset(9, 0);

	K051316Init(0, DrvGfxROM1, DrvGfxROMExp1, 0x7ffff, K051316Callback0, 4, 0);
	K051316SetOffset(0, -105, -16);

	K051316Init(1, DrvGfxROM2, DrvGfxROMExp2, 0x7ffff, K051316Callback1, 4, 0);
	K051316SetOffset(1, -105, -16);

	K051316Init(2, DrvGfxROM3, DrvGfxROMExp3, 0x7ffff, K051316Callback2, 4, 0);
	K051316SetOffset(2, -105, -16);

	BurnYM2151Init(4000000, 100.0);

	MSM6295Init(0, 1056000 / 132, 50.0, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	KonamiICExit();

	SekExit();
	ZetExit();

	BurnYM2151Exit();
	MSM6295Exit(0);

	BurnFree (AllMem);

	return 0;
}

static inline void DrvRecalcPalette()
{
	UINT8 r,g,b;
	UINT16 *p = (UINT16*)DrvPalRAM;
	for (INT32 i = 0; i < 0x4000 / 2; i++) {
		r = (p[i] >> 10) & 0x1f;
		g = (p[i] >>  5) & 0x1f;
		b = (p[i] >>  0) & 0x1f;

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvRecalcPalette();
	}

	BurnTransferClear();

	K051316_zoom_draw(2, 0);
	K051316_zoom_draw(1, 0);
	K051960SpritesRender(DrvGfxROMExp0, 0);
	K051316_zoom_draw(0, 0);
	K051960SpritesRender(DrvGfxROMExp0, 1);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	SekNewFrame();
	ZetNewFrame();

	{
		memset (DrvInputs, 0xff, 3);
		for (INT32 i = 0; i < 8; i++) { 
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

	  // Clear Opposites
		if ((DrvInputs[1] & 0x18) == 0) DrvInputs[1] |= 0x18;
		if ((DrvInputs[1] & 0x06) == 0) DrvInputs[1] |= 0x06;
		if ((DrvInputs[2] & 0x18) == 0) DrvInputs[2] |= 0x18;
		if ((DrvInputs[2] & 0x06) == 0) DrvInputs[2] |= 0x06;
	}

	INT32 nInterleave = 100;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 12000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCyclesSegment = nCyclesTotal[0] / nInterleave;

		nCyclesDone[0] += SekRun(nCyclesSegment);

		nCyclesSegment = nCyclesTotal[1] / nInterleave;

		nCyclesDone[1] += ZetRun(nCyclesSegment);

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		if (nSegmentLength) {
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
		}
	}

	ZetClose();
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029705;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);

		BurnYM2151Scan(nAction);
		MSM6295Scan(0, nAction);

		KonamiICScan(nAction);

		SCAN_VAR(bank0);
		SCAN_VAR(bank1);
		SCAN_VAR(bank2);
	}

	return 0;
}


// Ultraman (Japan)

static struct BurnRomInfo ultramanRomDesc[] = {
	{ "910-b01.c11",	0x20000, 0x3d9e4323, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "910-b02.d11",	0x20000, 0xd24c82e9, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "910-a05.d05",	0x08000, 0xebaef189, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "910-a19.l04",	0x80000, 0x2dc9ffdc, 3 | BRF_GRA },           //  3 K051960 Sprites
	{ "910-a20.l01",	0x80000, 0xa4298dce, 3 | BRF_GRA },           //  4

	{ "910-a07.j15",	0x20000, 0x8b43a64e, 4 | BRF_GRA },           //  5 K051316 #0 Tiles
	{ "910-a08.j16",	0x20000, 0xc3829826, 4 | BRF_GRA },           //  6
	{ "910-a09.j18",	0x20000, 0xee10b519, 4 | BRF_GRA },           //  7
	{ "910-a10.j19",	0x20000, 0xcffbb0c3, 4 | BRF_GRA },           //  8

	{ "910-a11.l15",	0x20000, 0x17a5581d, 5 | BRF_GRA },           //  9 K051316 #1 Tiles
	{ "910-a12.l16",	0x20000, 0x39763fb5, 5 | BRF_GRA },           // 10
	{ "910-a13.l18",	0x20000, 0x66b25a4f, 5 | BRF_GRA },           // 11
	{ "910-a14.l19",	0x20000, 0x09fbd412, 5 | BRF_GRA },           // 12

	{ "910-a15.m15",	0x20000, 0x6d5bfbb7, 6 | BRF_GRA },           // 13 K051316 #2 Tiles
	{ "910-a16.m16",	0x20000, 0x5f6f8c3d, 6 | BRF_GRA },           // 14
	{ "910-a17.m18",	0x20000, 0x1f3ec4ff, 6 | BRF_GRA },           // 15
	{ "910-a18.m19",	0x20000, 0xfdc42929, 6 | BRF_GRA },           // 16

	{ "910-a06.c06",	0x40000, 0x28fa99c9, 7 | BRF_SND },           // 17 MSM6295 Samples

	{ "910-a21.f14",	0x00100, 0x64460fbc, 0 | BRF_OPT },           // 18 Priority Prom
};

STD_ROM_PICK(ultraman)
STD_ROM_FN(ultraman)

struct BurnDriver BurnDrvUltraman = {
	"ultraman", NULL, NULL, NULL, "1991",
	"Ultraman (Japan)\0", NULL, "Banpresto / Bandai", "GX910",
	L"\uFEFF\u30A6\u30EB\u30c8\u30E9\u30DE\u30f3  \u7A7A\u60F3\u7279\u64AE\u30B7\u30EA\u30FC\u30BA (Japan)\0Ultraman\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_VSFIGHT, 0,
	NULL, ultramanRomInfo, ultramanRomName, NULL, NULL, UltramanInputInfo, UltramanDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x2000,
	288, 224, 4, 3
};
