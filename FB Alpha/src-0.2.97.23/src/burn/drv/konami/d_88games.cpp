// FB Alpha '88 Games driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "z80_intf.h"
#include "konami_intf.h"
#include "konamiic.h"
#include "burn_ym2151.h"
#include "upd7759.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvKonROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvGfxROMExp2;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *DrvBankRAM;
static UINT8 *DrvKonRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvNVRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *nDrvBank;

static INT32 videobank;
static INT32 zoomreadroms;
static INT32 k88games_priority;
static INT32 UPD7759Device;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static struct BurnInputInfo games88InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"P3 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p3 start"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy3 + 0,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	DrvJoy3 + 2,	"p3 fire 3"	},

	{"P4 Start",		BIT_DIGITAL,	DrvJoy3 + 7,	"p4 start"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p4 fire 2"	},
	{"P4 Button 3",		BIT_DIGITAL,	DrvJoy3 + 6,	"p4 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(games88)

static struct BurnDIPInfo games88DIPList[]=
{
	{0x14, 0xff, 0xff, 0xf0, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},
	{0x16, 0xff, 0xff, 0x7b, NULL			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x14, 0x01, 0x10, 0x10, "Off"			},
//	{0x14, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "World Records"	},
	{0x14, 0x01, 0x20, 0x20, "Don't Erase"		},
	{0x14, 0x01, 0x20, 0x00, "Erase on Reset"	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,   16, "Coin A"		},
	{0x15, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"	  },
	{0x15, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"	  },
	{0x15, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"	  },
	{0x15, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x15, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x15, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"	  },
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
	{0x15, 0x01, 0xf0, 0x20, "4 Coins 1 Credit"	  },
	{0x15, 0x01, 0xf0, 0x50, "3 Coins 1 Credit"	  },
	{0x15, 0x01, 0xf0, 0x80, "2 Coins 1 Credit"	  },
	{0x15, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x15, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x15, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"	  },
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

//	{0   , 0xfe, 0   ,    4, "Cabinet"		},
//	{0x16, 0x01, 0x06, 0x06, "Cocktail"		},
//	{0x16, 0x01, 0x06, 0x04, "Cocktail (A)"		},
//	{0x16, 0x01, 0x06, 0x02, "Upright"		},
//	{0x16, 0x01, 0x06, 0x00, "Upright (D)"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x60, 0x60, "Easy"			},
	{0x16, 0x01, 0x60, 0x40, "Normal"		},
	{0x16, 0x01, 0x60, 0x20, "Hard"			},
	{0x16, 0x01, 0x60, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x16, 0x01, 0x80, 0x80, "Off"			},
	{0x16, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(games88)

void games88_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x5f84:
			zoomreadroms = data & 0x04;
		return;

		case 0x5f88:
			// watchdog
		return;

		case 0x5f8c:
			*soundlatch = data;
		return;

		case 0x5f90:
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;
	}

	if ((address & 0xf800) == 0x3800)
	{
		if (videobank)
			DrvBankRAM[address & 0x7ff] = data;
		else
			K051316Write(0, address & 0x7ff, data);

		return;
	}

	if ((address & 0xfff0) == 0x5fc0) {
		K051316WriteCtrl(0, address & 0x0f, data);
		return;
	}

	if ((address & 0xc000) == 0x4000) {
		K052109_051960_w(address & 0x3fff, data);
		return;
	}
}

UINT8 games88_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x5f94:
			return (DrvInputs[0] & 0x0f) | (DrvDips[0] & 0xf0);

		case 0x5f95:
			return DrvInputs[1];

		case 0x5f96:
			return DrvInputs[2];

		case 0x5f97:
			return DrvDips[1];

		case 0x5f9b:
			return DrvDips[2];
	}

	if ((address & 0xf800) == 0x3800)
	{
		if (videobank) {
			return DrvBankRAM[address & 0x7ff];
		} else {
			if (zoomreadroms) {
				return K051316ReadRom(0, address & 0x7ff); // k051316_rom_0
			} else {
				return K051316Read(0, address & 0x7ff); // k051316_0
			}
		}
	}

	if ((address & 0xc000) == 0x4000) {
		return K052109_051960_r(address & 0x3fff);
	}

	return 0;
}

void __fastcall games88_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x9000:
			UPD7759PortWrite(UPD7759Device, data);
		return;

		case 0xc000:
			BurnYM2151SelectRegister(data);
		return;

		case 0xc001:
			BurnYM2151WriteRegister(data);
		return;

		case 0xe000:
			UPD7759Device = (data & 4) >> 2;
			UPD7759ResetWrite(UPD7759Device, data & 2);
			UPD7759StartWrite(UPD7759Device, data & 1);
		return;
	}
}

UINT8 __fastcall games88_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xa000:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;

		case 0xc000:
		case 0xc001:
			return BurnYM2151ReadStatus();
	}

	return 0;
}

static void games88_set_lines(INT32 lines)
{
	nDrvBank[0] = lines;

	INT32 nBank = 0x10000 + (lines & 0x07) * 0x2000;

	konamiMapMemory(DrvKonROM + nBank, 0x0000, 0x0fff, KON_ROM);

	if (lines & 8) {
		konamiMapMemory(DrvPalRAM, 0x1000, 0x1fff, KON_RAM);
	} else {
		konamiMapMemory(DrvKonROM + nBank + 0x1000, 0x1000, 0x1fff, KON_ROM);
		konamiMapMemory(DrvKonROM +         0x0000, 0x1000, 0x1fff, KON_WRITE); // unmap writes
	}

	videobank = lines & 0x10;
	K052109RMRDLine = lines & 0x20;
	k88games_priority = lines & 0x80;
}

static void K052109Callback(INT32 layer, INT32 bank, INT32 *code, INT32 *color, INT32 *, INT32 *)
{
	INT32 layer_colorbase[3] = { 64, 0, 16 };

	*code |= ((*color & 0x0f) << 8) | (bank << 12);
	*color = layer_colorbase[layer] + ((*color & 0xf0) >> 4);
}

static void K051960Callback(INT32 *, INT32 *color, INT32 *priority, INT32 *)
{
	*priority = (*color & 0x20) >> 5;
	*color = 32 + (*color & 0x0f);
}

static void K051316Callback(INT32 *code,INT32 *color,INT32 *flags)
{
	*flags = *color & 0x40;
	*code |= ((*color & 0x07) << 8);
	*color = 48 + ((*color & 0x38) >> 3) + ((*color & 0x80) >> 4);
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	konamiOpen(0);
	konamiReset();
	konamiClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2151Reset();

	KonamiICReset();

	UPD7759Reset();

	videobank = 0;
	zoomreadroms = 0;
	k88games_priority = 0;
	UPD7759Device = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvKonROM		= Next; Next += 0x020000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x080000;
	DrvGfxROM1		= Next; Next += 0x100000;
	DrvGfxROM2		= Next; Next += 0x040000;
	DrvGfxROMExp0		= Next; Next += 0x100000;
	DrvGfxROMExp1		= Next; Next += 0x200000;
	DrvGfxROMExp2		= Next; Next += 0x080000;

	DrvSndROM0		= Next; Next += 0x020000;
	DrvSndROM1		= Next; Next += 0x020000;

	DrvPalette		= (UINT32*)Next; Next += 0x800 * sizeof(UINT32);

	AllRam			= Next;

	DrvBankRAM		= Next; Next += 0x000800;
	DrvKonRAM		= Next; Next += 0x001000;
	DrvPalRAM		= Next; Next += 0x001000;
	DrvNVRAM		= Next; Next += 0x000800;

	DrvZ80RAM		= Next; Next += 0x000800;

	soundlatch		= Next; Next += 0x000001;
	nDrvBank		= Next; Next += 0x000002;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[4]  = { 0x018, 0x010, 0x008, 0x000 };
	INT32 Plane1[4]  = { 0x000, 0x008, 0x010, 0x018 };
	INT32 XOffs0[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			   0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107 };
	INT32 YOffs0[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
			   0x200, 0x220, 0x240, 0x260, 0x280, 0x2a0, 0x2c0, 0x2e0 };

	konami_rom_deinterleave_2(DrvGfxROM0, 0x080000);
	konami_rom_deinterleave_2(DrvGfxROM1, 0x100000);

	GfxDecode(0x04000, 4,  8,  8, Plane0, XOffs0, YOffs0, 0x100, DrvGfxROM0, DrvGfxROMExp0);
	GfxDecode(0x02000, 4, 16, 16, Plane1, XOffs0, YOffs0, 0x400, DrvGfxROM1, DrvGfxROMExp1);

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
		if (BurnLoadRom(DrvKonROM  + 0x008000,  0, 1)) return 1;
		if (BurnLoadRom(DrvKonROM  + 0x010000,  1, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;


		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001,  4, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x020000,  5, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x020001,  6, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x040000,  7, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x040001,  8, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x060000,  9, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x060001, 10, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000, 11, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x000001, 12, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x020000, 13, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x020001, 14, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040000, 15, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040001, 16, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x060000, 17, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x060001, 18, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000, 19, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080001, 20, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0a0000, 21, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0a0001, 22, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0c0000, 23, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0c0001, 24, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0e0000, 25, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0e0001, 26, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000, 27, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x010000, 28, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x020000, 29, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x030000, 30, 1)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000, 31, 1)) return 1;
		if (BurnLoadRom(DrvSndROM0 + 0x010000, 32, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x000000, 33, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x010000, 34, 1)) return 1;

		DrvGfxDecode();
	}

	konamiInit(1);
	konamiOpen(0);
	konamiMapMemory(DrvPalRAM + 0x0000,	0x1000, 0x1fff, KON_RAM);
	konamiMapMemory(DrvKonRAM,		0x2000, 0x2fff, KON_RAM); 
	konamiMapMemory(DrvNVRAM,		0x3000, 0x37ff, KON_RAM);
	konamiMapMemory(DrvKonROM + 0x8000,	0x8000, 0xffff, KON_ROM);
	konamiSetWriteHandler(games88_main_write);
	konamiSetReadHandler(games88_main_read);
	konamiSetlinesCallback(games88_set_lines);
	konamiClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(games88_sound_write);
	ZetSetReadHandler(games88_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(3579545);
	BurnYM2151SetAllRoutes(0.75, BURN_SND_ROUTE_BOTH);

	UPD7759Init(0, UPD7759_STANDARD_CLOCK, DrvSndROM0);
	UPD7759Init(1, UPD7759_STANDARD_CLOCK, DrvSndROM1);
	UPD7759SetRoute(0, 0.30, BURN_SND_ROUTE_BOTH);
	UPD7759SetRoute(1, 0.30, BURN_SND_ROUTE_BOTH);

	K052109Init(DrvGfxROM0, 0x7ffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(0, 0);

	K051960Init(DrvGfxROM1, 0xfffff);
	K051960SetCallback(K051960Callback);
	K051960SetSpriteOffset(0, 0);

	K051316Init(0, DrvGfxROM2, DrvGfxROMExp2, 0x3ffff, K051316Callback, 4, 0);
	K051316SetOffset(0, -104, -16);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	KonamiICExit();

	konamiExit();
	ZetExit();

	BurnYM2151Exit();
	UPD7759Exit();

	BurnFree (AllMem);

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		KonamiRecalcPal(DrvPalRAM, DrvPalette, 0x1000);
	}

	K052109UpdateScroll();

	if (k88games_priority)
	{
		K052109RenderLayer(0, 1, DrvGfxROMExp0);
		K051960SpritesRender(DrvGfxROMExp1, 1);
		K052109RenderLayer(2, 0, DrvGfxROMExp0);
		K052109RenderLayer(1, 0, DrvGfxROMExp0);
		K051960SpritesRender(DrvGfxROMExp1, 0);
		K051316_zoom_draw(0, 4);
	}
	else
	{
		K052109RenderLayer(2, 1, DrvGfxROMExp0);
		K051316_zoom_draw(0, 4);
		K051960SpritesRender(DrvGfxROMExp1, 0);
		K052109RenderLayer(1, 0, DrvGfxROMExp0);
		K051960SpritesRender(DrvGfxROMExp1, 1);
		K052109RenderLayer(0, 0, DrvGfxROMExp0);
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
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
	}

	konamiNewFrame();
	ZetNewFrame();

	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = 100;
	INT32 nCyclesTotal[2] = { (((3000000 / 60) * 133) / 100) /* 33% overclock */, 3579545 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	ZetOpen(0);
	konamiOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nSegment = (nCyclesTotal[0] / nInterleave) * (i + 1);

		nCyclesDone[0] += konamiRun(nSegment - nCyclesDone[0]);

		nSegment = (nCyclesTotal[1] / nInterleave) * (i + 1);

		nCyclesDone[1] += ZetRun(nSegment - nCyclesDone[1]);

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			UPD7759Update(0, pSoundBuf, nSegmentLength);
			UPD7759Update(1, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (K052109_irq_enabled) konamiSetIrqLine(KONAMI_IRQ_LINE, KONAMI_HOLD_LINE);

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		if (nSegmentLength) {
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			UPD7759Update(0, pSoundBuf, nSegmentLength);
			UPD7759Update(1, pSoundBuf, nSegmentLength);
		}
	}

	konamiClose();
	ZetClose();

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

		konamiCpuScan(nAction, pnMin);
		ZetScan(nAction);

		BurnYM2151Scan(nAction);
		UPD7759Scan(0, nAction, pnMin);
		UPD7759Scan(0, nAction, pnMin);

		KonamiICScan(nAction);

		SCAN_VAR(videobank);
		SCAN_VAR(zoomreadroms);
		SCAN_VAR(k88games_priority);
		SCAN_VAR(UPD7759Device);
	}

	if (nAction & ACB_WRITE) {
		konamiOpen(0);
		games88_set_lines(nDrvBank[0]);
		konamiClose();
	}

	return 0;
}


// '88 Games

static struct BurnRomInfo games88RomDesc[] = {
	{ "861m01.k18",	0x08000, 0x4a4e2959, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Cpu
	{ "861m02.k16",	0x10000, 0xe19f15f6, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "861d01.d9",	0x08000, 0x0ff1dec0, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "861a08.a",	0x10000, 0x77a00dd6, 3 | BRF_GRA },           //  3 K052109 Tiles
	{ "861a08.c",	0x10000, 0xb422edfc, 3 | BRF_GRA },           //  4
	{ "861a08.b",	0x10000, 0x28a8304f, 3 | BRF_GRA },           //  5
	{ "861a08.d",	0x10000, 0xe01a3802, 3 | BRF_GRA },           //  6
	{ "861a09.a",	0x10000, 0xdf8917b6, 3 | BRF_GRA },           //  7
	{ "861a09.c",	0x10000, 0xf577b88f, 3 | BRF_GRA },           //  8
	{ "861a09.b",	0x10000, 0x4917158d, 3 | BRF_GRA },           //  9
	{ "861a09.d",	0x10000, 0x2bb3282c, 3 | BRF_GRA },           // 10

	{ "861a05.a",	0x10000, 0xcedc19d0, 4 | BRF_GRA },           // 11 K051960 Tiles
	{ "861a05.e",	0x10000, 0x725af3fc, 4 | BRF_GRA },           // 12
	{ "861a05.b",	0x10000, 0xdb2a8808, 4 | BRF_GRA },           // 13
	{ "861a05.f",	0x10000, 0x32d830ca, 4 | BRF_GRA },           // 14
	{ "861a05.c",	0x10000, 0xcf03c449, 4 | BRF_GRA },           // 15
	{ "861a05.g",	0x10000, 0xfd51c4ea, 4 | BRF_GRA },           // 16
	{ "861a05.d",	0x10000, 0x97d78c77, 4 | BRF_GRA },           // 17
	{ "861a05.h",	0x10000, 0x60d0c8a5, 4 | BRF_GRA },           // 18
	{ "861a06.a",	0x10000, 0x85e2e30e, 4 | BRF_GRA },           // 19
	{ "861a06.e",	0x10000, 0x6f96651c, 4 | BRF_GRA },           // 20
	{ "861a06.b",	0x10000, 0xce17eaf0, 4 | BRF_GRA },           // 21
	{ "861a06.f",	0x10000, 0x88310bf3, 4 | BRF_GRA },           // 22
	{ "861a06.c",	0x10000, 0xa568b34e, 4 | BRF_GRA },           // 23
	{ "861a06.g",	0x10000, 0x4a55beb3, 4 | BRF_GRA },           // 24
	{ "861a06.d",	0x10000, 0xbc70ab39, 4 | BRF_GRA },           // 25
	{ "861a06.h",	0x10000, 0xd906b79b, 4 | BRF_GRA },           // 26

	{ "861a04.a",	0x10000, 0x092a8b15, 5 | BRF_GRA },           // 27 K051316 Tiles
	{ "861a04.b",	0x10000, 0x75744b56, 5 | BRF_GRA },           // 28
	{ "861a04.c",	0x10000, 0xa00021c5, 5 | BRF_GRA },           // 29
	{ "861a04.d",	0x10000, 0xd208304c, 5 | BRF_GRA },           // 30

	{ "861a07.a",	0x10000, 0x5d035d69, 6 | BRF_SND },           // 31 UPD7759 #0 Samples
	{ "861a07.b",	0x10000, 0x6337dd91, 6 | BRF_SND },           // 32

	{ "861a07.c",	0x10000, 0x5067a38b, 7 | BRF_SND },           // 33 UPD7759 #1 Samples
	{ "861a07.d",	0x10000, 0x86731451, 7 | BRF_SND },           // 34

	{ "861.g3",	0x00100, 0x429785db, 0 | BRF_OPT },           // 31 Priority Prom
};

STD_ROM_PICK(games88)
STD_ROM_FN(games88)

struct BurnDriver BurnDrvgames88 = {
	"88games", NULL, NULL, NULL, "1988",
	"'88 Games\0", NULL, "Konami", "GX861",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, games88RomInfo, games88RomName, NULL, NULL, games88InputInfo, games88DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	304, 224, 4, 3
};


// Konami '88

static struct BurnRomInfo konami88RomDesc[] = {
	{ "861.e03",	0x08000, 0x55979bd9, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Cpu
	{ "861.e02",	0x10000, 0x5b7e98a6, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "861d01.d9",	0x08000, 0x0ff1dec0, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "861a08.a",	0x10000, 0x77a00dd6, 3 | BRF_GRA },           //  3 K052109 Tiles
	{ "861a08.c",	0x10000, 0xb422edfc, 3 | BRF_GRA },           //  4
	{ "861a08.b",	0x10000, 0x28a8304f, 3 | BRF_GRA },           //  5
	{ "861a08.d",	0x10000, 0xe01a3802, 3 | BRF_GRA },           //  6
	{ "861a09.a",	0x10000, 0xdf8917b6, 3 | BRF_GRA },           //  7
	{ "861a09.c",	0x10000, 0xf577b88f, 3 | BRF_GRA },           //  8
	{ "861a09.b",	0x10000, 0x4917158d, 3 | BRF_GRA },           //  9
	{ "861a09.d",	0x10000, 0x2bb3282c, 3 | BRF_GRA },           // 10

	{ "861a05.a",	0x10000, 0xcedc19d0, 4 | BRF_GRA },           // 11 K051960 Tiles
	{ "861a05.e",	0x10000, 0x725af3fc, 4 | BRF_GRA },           // 12
	{ "861a05.b",	0x10000, 0xdb2a8808, 4 | BRF_GRA },           // 13
	{ "861a05.f",	0x10000, 0x32d830ca, 4 | BRF_GRA },           // 14
	{ "861a05.c",	0x10000, 0xcf03c449, 4 | BRF_GRA },           // 15
	{ "861a05.g",	0x10000, 0xfd51c4ea, 4 | BRF_GRA },           // 16
	{ "861a05.d",	0x10000, 0x97d78c77, 4 | BRF_GRA },           // 17
	{ "861a05.h",	0x10000, 0x60d0c8a5, 4 | BRF_GRA },           // 18
	{ "861a06.a",	0x10000, 0x85e2e30e, 4 | BRF_GRA },           // 19
	{ "861a06.e",	0x10000, 0x6f96651c, 4 | BRF_GRA },           // 20
	{ "861a06.b",	0x10000, 0xce17eaf0, 4 | BRF_GRA },           // 21
	{ "861a06.f",	0x10000, 0x88310bf3, 4 | BRF_GRA },           // 22
	{ "861a06.c",	0x10000, 0xa568b34e, 4 | BRF_GRA },           // 23
	{ "861a06.g",	0x10000, 0x4a55beb3, 4 | BRF_GRA },           // 24
	{ "861a06.d",	0x10000, 0xbc70ab39, 4 | BRF_GRA },           // 25
	{ "861a06.h",	0x10000, 0xd906b79b, 4 | BRF_GRA },           // 26

	{ "861a04.a",	0x10000, 0x092a8b15, 5 | BRF_GRA },           // 27 K051316 Tiles
	{ "861a04.b",	0x10000, 0x75744b56, 5 | BRF_GRA },           // 28
	{ "861a04.c",	0x10000, 0xa00021c5, 5 | BRF_GRA },           // 29
	{ "861a04.d",	0x10000, 0xd208304c, 5 | BRF_GRA },           // 30

	{ "861a07.a",	0x10000, 0x5d035d69, 6 | BRF_SND },           // 31 UPD7759 #0 Samples
	{ "861a07.b",	0x10000, 0x6337dd91, 6 | BRF_SND },           // 32

	{ "861a07.c",	0x10000, 0x5067a38b, 7 | BRF_SND },           // 33 UPD7759 #1 Samples
	{ "861a07.d",	0x10000, 0x86731451, 7 | BRF_SND },           // 34

	{ "861.g3",	0x00100, 0x429785db, 0 | BRF_OPT },           // 31 Priority Prom
};

STD_ROM_PICK(konami88)
STD_ROM_FN(konami88)

struct BurnDriver BurnDrvKonami88 = {
	"konami88", "88games", NULL, NULL, "1988",
	"Konami '88\0", NULL, "Konami", "GX861",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, konami88RomInfo, konami88RomName, NULL, NULL, games88InputInfo, games88DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	304, 224, 4, 3
};


// Hyper Sports Special (Japan)

static struct BurnRomInfo hypsptspRomDesc[] = {
	{ "861f03.k18",	0x08000, 0x8c61aebd, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Cpu
	{ "861f02.k16",	0x10000, 0xd2460c28, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "861d01.d9",	0x08000, 0x0ff1dec0, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "861a08.a",	0x10000, 0x77a00dd6, 3 | BRF_GRA },           //  3 K052109 Tiles
	{ "861a08.c",	0x10000, 0xb422edfc, 3 | BRF_GRA },           //  4
	{ "861a08.b",	0x10000, 0x28a8304f, 3 | BRF_GRA },           //  5
	{ "861a08.d",	0x10000, 0xe01a3802, 3 | BRF_GRA },           //  6
	{ "861a09.a",	0x10000, 0xdf8917b6, 3 | BRF_GRA },           //  7
	{ "861a09.c",	0x10000, 0xf577b88f, 3 | BRF_GRA },           //  8
	{ "861a09.b",	0x10000, 0x4917158d, 3 | BRF_GRA },           //  9
	{ "861a09.d",	0x10000, 0x2bb3282c, 3 | BRF_GRA },           // 10

	{ "861a05.a",	0x10000, 0xcedc19d0, 4 | BRF_GRA },           // 11 K051960 Tiles
	{ "861a05.e",	0x10000, 0x725af3fc, 4 | BRF_GRA },           // 12
	{ "861a05.b",	0x10000, 0xdb2a8808, 4 | BRF_GRA },           // 13
	{ "861a05.f",	0x10000, 0x32d830ca, 4 | BRF_GRA },           // 14
	{ "861a05.c",	0x10000, 0xcf03c449, 4 | BRF_GRA },           // 15
	{ "861a05.g",	0x10000, 0xfd51c4ea, 4 | BRF_GRA },           // 16
	{ "861a05.d",	0x10000, 0x97d78c77, 4 | BRF_GRA },           // 17
	{ "861a05.h",	0x10000, 0x60d0c8a5, 4 | BRF_GRA },           // 18
	{ "861a06.a",	0x10000, 0x85e2e30e, 4 | BRF_GRA },           // 19
	{ "861a06.e",	0x10000, 0x6f96651c, 4 | BRF_GRA },           // 20
	{ "861a06.b",	0x10000, 0xce17eaf0, 4 | BRF_GRA },           // 21
	{ "861a06.f",	0x10000, 0x88310bf3, 4 | BRF_GRA },           // 22
	{ "861a06.c",	0x10000, 0xa568b34e, 4 | BRF_GRA },           // 23
	{ "861a06.g",	0x10000, 0x4a55beb3, 4 | BRF_GRA },           // 24
	{ "861a06.d",	0x10000, 0xbc70ab39, 4 | BRF_GRA },           // 25
	{ "861a06.h",	0x10000, 0xd906b79b, 4 | BRF_GRA },           // 26

	{ "861a04.a",	0x10000, 0x092a8b15, 5 | BRF_GRA },           // 27 K051316 Tiles
	{ "861a04.b",	0x10000, 0x75744b56, 5 | BRF_GRA },           // 28
	{ "861a04.c",	0x10000, 0xa00021c5, 5 | BRF_GRA },           // 29
	{ "861a04.d",	0x10000, 0xd208304c, 5 | BRF_GRA },           // 30

	{ "861a07.a",	0x10000, 0x5d035d69, 6 | BRF_SND },           // 31 UPD7759 #0 Samples
	{ "861a07.b",	0x10000, 0x6337dd91, 6 | BRF_SND },           // 32

	{ "861a07.c",	0x10000, 0x5067a38b, 7 | BRF_SND },           // 33 UPD7759 #1 Samples
	{ "861a07.d",	0x10000, 0x86731451, 7 | BRF_SND },           // 34

	{ "861.g3",	0x00100, 0x429785db, 8 | BRF_OPT },           // 31 Priority Prom
};

STD_ROM_PICK(hypsptsp)
STD_ROM_FN(hypsptsp)

struct BurnDriver BurnDrvHypsptsp = {
	"hypsptsp", "88games", NULL, NULL, "1988",
	"Hyper Sports Special (Japan)\0", NULL, "Konami", "GX861",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, hypsptspRomInfo, hypsptspRomName, NULL, NULL, games88InputInfo, games88DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	304, 224, 4, 3
};
