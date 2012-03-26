// FB Alpha Tiger Road driver module
// Based on MAME driver by Phil Stroffolino
// F1 Dream protection code by Eric Hustvedt

#include "tiles_generic.h"
#include "sek.h"
#include "zet.h"
#include "burn_ym2203.h"
#include "msm5205.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvSndROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *Drv68KRAM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprBuf;
static UINT8 *DrvVidRAM;
static UINT8 *DrvScrollRAM;
static UINT32 *DrvPalette;
static UINT8 *DrvTransMask;

static UINT8 DrvRecalc;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDip[2];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static UINT8 *soundlatch;
static UINT8 *soundlatch2;
static UINT8 *flipscreen;
static UINT8 *bgcharbank;
static UINT8 *coin_lockout;

static INT32 nF1dream = 0;
static INT32 toramich = 0;

static struct BurnInputInfo TigeroadInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 14,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 8,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 15,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 9,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDip + 1,	"dip"		},
};

STDINPUTINFO(Tigeroad)

static struct BurnDIPInfo TigeroadDIPList[] =
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0xfb, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x11, 0x01, 0x07, 0x00, "4 Coins 1 Credits "	},
	{0x11, 0x01, 0x07, 0x01, "3 Coins 1 Credits "	},
	{0x11, 0x01, 0x07, 0x02, "2 Coins 1 Credits "	},
	{0x11, 0x01, 0x07, 0x07, "1 Coin 1 Credits "	},
	{0x11, 0x01, 0x07, 0x06, "1 Coin 2 Credits "	},
	{0x11, 0x01, 0x07, 0x05, "1 Coin 3 Credits "	},
	{0x11, 0x01, 0x07, 0x04, "1 Coin 4 Credits "	},
	{0x11, 0x01, 0x07, 0x03, "1 Coin 6 Credits "	},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x11, 0x01, 0x38, 0x00, "4 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x08, "3 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x10, "2 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x38, "1 Coin 1 Credits "	},
	{0x11, 0x01, 0x38, 0x30, "1 Coin 2 Credits "	},
	{0x11, 0x01, 0x38, 0x28, "1 Coin 3 Credits "	},
	{0x11, 0x01, 0x38, 0x20, "1 Coin 4 Credits "	},
	{0x11, 0x01, 0x38, 0x18, "1 Coin 6 Credits "	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x11, 0x01, 0x40, 0x00, "On"			},
	{0x11, 0x01, 0x40, 0x40, "Off"			},


	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x11, 0x01, 0x80, 0x80, "Off"			},
	{0x11, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x12, 0x01, 0x03, 0x03, "3"			},
	{0x12, 0x01, 0x03, 0x02, "4"			},
	{0x12, 0x01, 0x03, 0x01, "5"			},
	{0x12, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x04, 0x00, "Upright"		},
	{0x12, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x12, 0x01, 0x18, 0x18, "20000 70000 70000"	},
	{0x12, 0x01, 0x18, 0x10, "20000 80000 80000"	},
	{0x12, 0x01, 0x18, 0x08, "30000 80000 80000"	},
	{0x12, 0x01, 0x18, 0x00, "30000 90000 90000"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x60, 0x20, "Very_Easy"		},
	{0x12, 0x01, 0x60, 0x40, "Easy"			},
	{0x12, 0x01, 0x60, 0x60, "Normal"		},
	{0x12, 0x01, 0x60, 0x00, "Difficult"		},

	{0   , 0xfe, 0   ,    2, "Allow_Continue"	},
	{0x12, 0x01, 0x80, 0x00, "No"			},
	{0x12, 0x01, 0x80, 0x80, "Yes"			},
};

STDDIPINFO(Tigeroad)

static struct BurnDIPInfo ToramichDIPList[] =
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0xfb, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x11, 0x01, 0x07, 0x00, "4 Coins 1 Credits "	},
	{0x11, 0x01, 0x07, 0x01, "3 Coins 1 Credits "	},
	{0x11, 0x01, 0x07, 0x02, "2 Coins 1 Credits "	},
	{0x11, 0x01, 0x07, 0x07, "1 Coin 1 Credits "	},
	{0x11, 0x01, 0x07, 0x06, "1 Coin 2 Credits "	},
	{0x11, 0x01, 0x07, 0x05, "1 Coin 3 Credits "	},
	{0x11, 0x01, 0x07, 0x04, "1 Coin 4 Credits "	},
	{0x11, 0x01, 0x07, 0x03, "1 Coin 6 Credits "	},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x11, 0x01, 0x38, 0x00, "4 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x08, "3 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x10, "2 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x38, "1 Coin 1 Credits "	},
	{0x11, 0x01, 0x38, 0x30, "1 Coin 2 Credits "	},
	{0x11, 0x01, 0x38, 0x28, "1 Coin 3 Credits "	},
	{0x11, 0x01, 0x38, 0x20, "1 Coin 4 Credits "	},
	{0x11, 0x01, 0x38, 0x18, "1 Coin 6 Credits "	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x11, 0x01, 0x40, 0x00, "On"			},
	{0x11, 0x01, 0x40, 0x40, "Off"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x11, 0x01, 0x80, 0x80, "Off"			},
	{0x11, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x12, 0x01, 0x03, 0x03, "3"			},
	{0x12, 0x01, 0x03, 0x02, "4"			},
	{0x12, 0x01, 0x03, 0x01, "5"			},
	{0x12, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x04, 0x00, "Upright"		},
	{0x12, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Bonus Life"		},
	{0x12, 0x01, 0x08, 0x08, "20000 70000 70000"	},
	{0x12, 0x01, 0x08, 0x00, "20000 80000 80000"	},

	{0   , 0xfe, 0   ,    2, "Allow Level Select"	},
	{0x12, 0x01, 0x10, 0x10, "No"			},
	{0x12, 0x01, 0x10, 0x00, "Yes"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x60, 0x40, "Easy"			},
	{0x12, 0x01, 0x60, 0x60, "Normal"		},
	{0x12, 0x01, 0x60, 0x20, "Difficult"		},
	{0x12, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x12, 0x01, 0x80, 0x00, "No"			},
	{0x12, 0x01, 0x80, 0x80, "Yes"			},
};

STDDIPINFO(Toramich)

static struct BurnDIPInfo F1dreamDIPList[] =
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0xbb, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x11, 0x01, 0x07, 0x00, "4 Coins 1 Credits "	},
	{0x11, 0x01, 0x07, 0x01, "3 Coins 1 Credits "	},
	{0x11, 0x01, 0x07, 0x02, "2 Coins 1 Credits "	},
	{0x11, 0x01, 0x07, 0x07, "1 Coin 1 Credits "	},
	{0x11, 0x01, 0x07, 0x06, "1 Coin 2 Credits "	},
	{0x11, 0x01, 0x07, 0x05, "1 Coin 3 Credits "	},
	{0x11, 0x01, 0x07, 0x04, "1 Coin 4 Credits "	},
	{0x11, 0x01, 0x07, 0x03, "1 Coin 6 Credits "	},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x11, 0x01, 0x38, 0x00, "4 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x08, "3 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x10, "2 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x38, "1 Coin 1 Credits "	},
	{0x11, 0x01, 0x38, 0x30, "1 Coin 2 Credits "	},
	{0x11, 0x01, 0x38, 0x28, "1 Coin 3 Credits "	},
	{0x11, 0x01, 0x38, 0x20, "1 Coin 4 Credits "	},
	{0x11, 0x01, 0x38, 0x18, "1 Coin 6 Credits "	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x11, 0x01, 0x40, 0x00, "On"			},
	{0x11, 0x01, 0x40, 0x40, "Off"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x11, 0x01, 0x80, 0x80, "Off"			},
	{0x11, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x12, 0x01, 0x03, 0x03, "3"			},
	{0x12, 0x01, 0x03, 0x02, "4"			},
	{0x12, 0x01, 0x03, 0x01, "5"			},
	{0x12, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x04, 0x00, "Upright"		},
	{0x12, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "F1 Up Point"		},
	{0x12, 0x01, 0x18, 0x18, "12"			},
	{0x12, 0x01, 0x18, 0x10, "16"			},
	{0x12, 0x01, 0x18, 0x08, "18"			},
	{0x12, 0x01, 0x18, 0x00, "20"			},

	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x12, 0x01, 0x20, 0x20, "Normal"		},
	{0x12, 0x01, 0x20, 0x00, "Difficult"		},

	{0   , 0xfe, 0   ,    2, "Version"		},
	{0x12, 0x01, 0x40, 0x00, "International"	},
	{0x12, 0x01, 0x40, 0x40, "Japan"		},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x12, 0x01, 0x80, 0x00, "No"			},
	{0x12, 0x01, 0x80, 0x80, "Yes"			},
};

STDDIPINFO(F1dream)

static void palette_write(INT32 offset)
{
	UINT16 data = *((UINT16 *)(DrvPalRAM + offset + 0x200));

	UINT8 r,g,b;

	r = (data >> 8) & 0x0f;
	g = (data >> 4) & 0x0f;
	b = (data >> 0) & 0x0f;

	r |= r << 4;
	g |= g << 4;
	b |= b << 4;

	DrvPalette[offset / 2] = BurnHighCol(r, g, b, 0);
}

static const UINT16 f1dream_613ea_lookup[16] = {
0x0052, 0x0031, 0x00a7, 0x0043, 0x0007, 0x008a, 0x00b1, 0x0066, 0x009f, 0x00cc, 0x0009, 0x004d, 0x0033, 0x0028, 0x00d0, 0x0025};

static const UINT16 f1dream_613eb_lookup[256] = {
0x0001, 0x00b5, 0x00b6, 0x00b6, 0x00b6, 0x00b6, 0x00b6, 0x00b6, 0x00b7, 0x0001, 0x00b8, 0x002f, 0x002f, 0x002f, 0x002f, 0x00b9,
0x00aa, 0x0031, 0x00ab, 0x00ab, 0x00ab, 0x00ac, 0x00ad, 0x00ad, 0x00ae, 0x00af, 0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x0091,
0x009c, 0x009d, 0x009e, 0x009f, 0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9, 0x009b, 0x0091,
0x00bc, 0x0092, 0x000b, 0x0009, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097, 0x0073, 0x0001, 0x0098, 0x0099, 0x009a, 0x009b, 0x0091,
0x00bc, 0x007b, 0x000b, 0x0008, 0x0087, 0x0088, 0x0089, 0x008a, 0x007f, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f, 0x0090, 0x0091,
0x00bd, 0x007b, 0x000b, 0x0007, 0x007c, 0x007d, 0x007e, 0x0001, 0x007f, 0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086,
0x00bc, 0x0070, 0x000b, 0x0006, 0x0071, 0x0072, 0x0073, 0x0001, 0x0074, 0x000d, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a,
0x00bc, 0x00ba, 0x000a, 0x0005, 0x0065, 0x0066, 0x0067, 0x0068, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
0x00bc, 0x0059, 0x0001, 0x0004, 0x005a, 0x005b, 0x0001, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063, 0x0064,
0x0014, 0x004d, 0x0001, 0x0003, 0x004e, 0x004f, 0x0050, 0x0051, 0x0052, 0x0001, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058,
0x0014, 0x0043, 0x0001, 0x0002, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x00bb, 0x004a, 0x004b, 0x004c, 0x0001, 0x0001,
0x0014, 0x002b, 0x0001, 0x0038, 0x0039, 0x003a, 0x003b, 0x0031, 0x003c, 0x003d, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0001,
0x0014, 0x002d, 0x0001, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0001, 0x0014, 0x0037, 0x0001,
0x0014, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x0001, 0x0001, 0x0001, 0x002a, 0x002b, 0x002c,
0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001e, 0x001e, 0x001e, 0x001f, 0x0020,
0x000c, 0x000d, 0x000e, 0x0001, 0x000f, 0x0010, 0x0011, 0x0012, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x000d, 0x0013 };

static const UINT16 f1dream_17b74_lookup[128] = {
0x0003, 0x0040, 0x0005, 0x0080, 0x0003, 0x0080, 0x0005, 0x00a0, 0x0003, 0x0040, 0x0005, 0x00c0, 0x0003, 0x0080, 0x0005, 0x00e0,
0x0003, 0x0040, 0x0006, 0x0000, 0x0003, 0x0080, 0x0006, 0x0020, 0x0003, 0x0040, 0x0006, 0x0040, 0x0003, 0x0080, 0x0006, 0x0060,
0x0000, 0x00a0, 0x0009, 0x00e0, 0x0000, 0x00e0, 0x000a, 0x0000, 0x0000, 0x00a0, 0x000a, 0x0020, 0x0000, 0x00e0, 0x000a, 0x0040,
0x0000, 0x00a0, 0x000a, 0x0060, 0x0000, 0x00e0, 0x000a, 0x0080, 0x0000, 0x00a0, 0x000a, 0x00a0, 0x0000, 0x00e0, 0x000a, 0x00c0,
0x0003, 0x0040, 0x0005, 0x0080, 0x0003, 0x0080, 0x0005, 0x00a0, 0x0003, 0x0040, 0x0005, 0x00c0, 0x0003, 0x0080, 0x0005, 0x00e0,
0x0003, 0x0040, 0x0006, 0x0000, 0x0003, 0x0080, 0x0006, 0x0020, 0x0003, 0x0040, 0x0006, 0x0040, 0x0003, 0x0080, 0x0006, 0x0060,
0x0000, 0x00a0, 0x0009, 0x00e0, 0x0000, 0x00e0, 0x000a, 0x0000, 0x0000, 0x00a0, 0x000a, 0x0020, 0x0000, 0x00e0, 0x000a, 0x0040,
0x0000, 0x00a0, 0x000a, 0x0060, 0x0000, 0x00e0, 0x000a, 0x0080, 0x0000, 0x00a0, 0x000a, 0x00a0, 0x0000, 0x00e0, 0x000a, 0x00c0 };

static const UINT16 f1dream_2450_lookup[32] = {
0x0003, 0x0080, 0x0006, 0x0060, 0x0000, 0x00e0, 0x000a, 0x00c0, 0x0003, 0x0080, 0x0006, 0x0060, 0x0000, 0x00e0, 0x000a, 0x00c0,
0x0003, 0x0080, 0x0006, 0x0060, 0x0000, 0x00e0, 0x000a, 0x00c0, 0x0003, 0x0080, 0x0006, 0x0060, 0x0000, 0x00e0, 0x000a, 0x00c0 };

static void f1dream_protection_w()
{
	INT32 indx;
	INT32 value = 255;
	INT32 prevpc = SekGetPC(0)-8;
	UINT16* ram16 = (UINT16*)Drv68KRAM;

	if (prevpc == 0x244c)
	{
		indx = ram16[0x3ff0/2];
		ram16[0x3fe6/2] = f1dream_2450_lookup[indx];
		ram16[0x3fe8/2] = f1dream_2450_lookup[++indx];
		ram16[0x3fea/2] = f1dream_2450_lookup[++indx];
		ram16[0x3fec/2] = f1dream_2450_lookup[++indx];
	}
	else if (prevpc == 0x613a)
	{
		if (ram16[0x3ff6/2] < 15)
		{
			indx = f1dream_613ea_lookup[ram16[0x3ff6/2]] - ram16[0x3ff4/2];
			if (indx > 255)
			{
				indx <<= 4;
				indx += ram16[0x3ff6/2] & 0x00ff;
				value = f1dream_613eb_lookup[indx];
			}
		}

		ram16[0x3ff2/2] = value;
	}
	else if (prevpc == 0x17b70)
	{
		if (ram16[0x3ff0/2] >= 0x04) indx = 128;
		else if (ram16[0x3ff0/2] > 0x02) indx = 96;
		else if (ram16[0x3ff0/2] == 0x02) indx = 64;
		else if (ram16[0x3ff0/2] == 0x01) indx = 32;
		else indx = 0;

		indx += ram16[0x3fee/2];
		if (indx < 128)
		{
			ram16[0x3fe6/2] = f1dream_17b74_lookup[indx];
			ram16[0x3fe8/2] = f1dream_17b74_lookup[++indx];
			ram16[0x3fea/2] = f1dream_17b74_lookup[++indx];
			ram16[0x3fec/2] = f1dream_17b74_lookup[++indx];
		}
		else
		{
			ram16[0x3fe6/2] = 0x00ff;
			ram16[0x3fe8/2] = 0x00ff;
			ram16[0x3fea/2] = 0x00ff;
			ram16[0x3fec/2] = 0x00ff;
		}
	}
	else if ((prevpc == 0x27f8) || (prevpc == 0x511a) || (prevpc == 0x5142) || (prevpc == 0x516a))
	{
		*soundlatch = ram16[0x3ffc/2] & 0xff;
	}
}

void __fastcall tigeroad_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0xfe4000:
			*flipscreen =  data & 0x02;
			*bgcharbank = (data & 0x04) >> 2;
			*coin_lockout = (~data & 0x30) << 1;
		return;

		case 0xfe4002:
			if (nF1dream) {
				f1dream_protection_w();
			} else {
				*soundlatch = data;
			}
		return;
	}
}

void __fastcall tigeroad_write_word(UINT32 address, UINT16 data)
{
	if (address >= 0xff8200 && address <= 0xff867f) {
		*((UINT16*)(DrvPalRAM + (address - 0xff8000))) = data;

		palette_write(address - 0xff8200);

		return;
	}

	switch (address)
	{
		case 0xfe8000:
		case 0xfe8002:
			*((UINT16 *)(DrvScrollRAM + (address & 2))) = data;
		return;

		case 0xfe800e:
			// watchdog
		return;
	}
}

UINT8 __fastcall tigeroad_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0xfe4000:
		case 0xfe4001:
			return DrvInputs[0] >> ((~address & 1) << 3);

		case 0xfe4002:
		case 0xfe4003:
			return DrvInputs[1] >> ((~address & 1) << 3);

		case 0xfe4004:
		case 0xfe4005:
			return DrvDip[~address & 1];
	}

	return 0;
}

UINT16 __fastcall tigeroad_read_word(UINT32 address)
{
	switch (address)
	{
		case 0xfe4000:
			return DrvInputs[0];

		case 0xfe4002:
			return DrvInputs[1];

		case 0xfe4004:
			return ((DrvDip[1] << 8) | DrvDip[0]);
	}

	return 0;
}

void __fastcall tigeroad_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x8000:
			BurnYM2203Write(0, 0, data);
		return;

		case 0x8001:
			BurnYM2203Write(0, 1, data);
		return;

		case 0xa000:
			BurnYM2203Write(1, 0, data);
		return;

		case 0xa001:
			BurnYM2203Write(1, 1, data);
		return;
	}
}

void __fastcall tigeroad_sound_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x7f:
			*soundlatch2 = data;
		return;
	}
}

UINT8 __fastcall tigeroad_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x8000:
			return BurnYM2203Read(0, 0);

		case 0xa000:
			return BurnYM2203Read(1, 0);

		case 0xe000:
			return *soundlatch;
	}

	return 0;
}

void __fastcall tigeroad_sample_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x01:
			SekOpen(0);
			MSM5205ResetWrite(0, (data >> 7) & 1);
			MSM5205DataWrite(0, data);
			MSM5205VCLKWrite(0, 1);
			MSM5205VCLKWrite(0, 0);
			SekClose();
		return;
	}
}

UINT8 __fastcall tigeroad_sample_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return *soundlatch2;
	}

	return 0;
}

static void TigeroadIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 TigeroadSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 3579545;
}

static double TigeroadGetTime()
{
	return (double)ZetTotalCycles() / 3579545;
}

inline static INT32 DrvMSM5205SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)(SekTotalCycles() * nSoundRate / 10000000);
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

	BurnYM2203Reset();
	
	if (toramich) {
		ZetOpen(1);
		ZetReset();
		ZetClose();
		
		MSM5205Reset();
	}

	if (pBurnSoundOut) { // fix ym2203 junk..
		memset (pBurnSoundOut, 0, nBurnSoundLen);
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x040000;
	DrvZ80ROM	= Next; Next += 0x008000;
	DrvSndROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x020000;
	DrvGfxROM1	= Next; Next += 0x200000;
	DrvGfxROM2	= Next; Next += 0x100000;
	DrvGfxROM3	= Next; Next += 0x008000;

	DrvPalette	= (UINT32*)Next; Next += 0x0240 * sizeof(UINT32);

	DrvTransMask	= Next; Next += 0x000010;

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x004000;
	DrvPalRAM	= Next; Next += 0x000800;
	DrvVidRAM	= Next; Next += 0x000800;
	DrvSprRAM	= Next; Next += 0x001400;
	DrvSprBuf	= Next; Next += 0x000500;

	DrvZ80RAM	= Next; Next += 0x000800;

	DrvScrollRAM	= Next; Next += 0x000004;

	soundlatch	= Next; Next += 0x000001;
	soundlatch2	= Next; Next += 0x000001;
	flipscreen	= Next; Next += 0x000001;
	bgcharbank	= Next; Next += 0x000001;
	coin_lockout	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[4]  = { ((0x20000 * 8) * 4) + 4, ((0x20000 * 8) * 4) + 0, 4, 0 };
	INT32 XOffs0[32] = { 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			   64*8+0, 64*8+1, 64*8+2, 64*8+3, 64*8+8+0, 64*8+8+1, 64*8+8+2, 64*8+8+3,
			   2*64*8+0, 2*64*8+1, 2*64*8+2, 2*64*8+3, 2*64*8+8+0, 2*64*8+8+1, 2*64*8+8+2, 2*64*8+8+3,
			   3*64*8+0, 3*64*8+1, 3*64*8+2, 3*64*8+3, 3*64*8+8+0, 3*64*8+8+1, 3*64*8+8+2, 3*64*8+8+3 };
	INT32 YOffs0[32] = { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			   8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16,
		   	   16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16,
			   24*16, 25*16, 26*16, 27*16, 28*16, 29*16, 30*16, 31*16 };

	INT32 Plane1[4]  = { (0x20000*8)*3, (0x20000*8)*2, (0x20000*8)*1, (0x20000*8)*0 };
	INT32 XOffs1[16] = { 0, 1, 2, 3, 4, 5, 6, 7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 };
	INT32 YOffs1[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 };


	UINT8 *tmp = (UINT8*)BurnMalloc(0x100000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x008000);

	GfxDecode(0x0800, 2,  8,  8, Plane0 + 2, XOffs0, YOffs0, 0x080, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x100000);

	GfxDecode(0x0800, 4, 32, 32, Plane0 + 0, XOffs0, YOffs0, 0x800, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x080000);

	GfxDecode(0x1000, 4, 16, 16, Plane1 + 0, XOffs1, YOffs1, 0x100, tmp, DrvGfxROM2);

	for (INT32 i = 0; i < 16; i++) {
		DrvTransMask[i] = (0xfe00 >> i) & 1;
	}

	BurnFree (tmp);

	return 0;
}

static INT32 DrvInit(INT32 (*pInitCallback)())
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (pInitCallback()) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(DrvSprRAM,		0xfe0800, 0xfe1bff, SM_RAM);
	SekMapMemory(DrvVidRAM,		0xfec000, 0xfec7ff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0xff8000, 0xff87ff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0xffc000, 0xffffff, SM_RAM);
	SekSetWriteByteHandler(0,	tigeroad_write_byte);
	SekSetWriteWordHandler(0,	tigeroad_write_word);
	SekSetReadByteHandler(0,	tigeroad_read_byte);
	SekSetReadWordHandler(0,	tigeroad_read_word);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80RAM);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(tigeroad_sound_write);
	ZetSetReadHandler(tigeroad_sound_read);
	ZetSetOutHandler(tigeroad_sound_out);
	ZetMemEnd();
	ZetClose();

	if (toramich) {
		ZetInit(1);

		ZetOpen(1);
		ZetMapArea(0x0000, 0xffff, 0, DrvSndROM);
		ZetMapArea(0x0000, 0xffff, 2, DrvSndROM);
		ZetSetOutHandler(tigeroad_sample_out);
		ZetSetInHandler(tigeroad_sample_in);
		ZetMemEnd();
		ZetClose();
	}

	BurnYM2203Init(2, 3579545, &TigeroadIRQHandler, TigeroadSynchroniseStream, TigeroadGetTime, 0);
	BurnYM2203SetVolumeShift(2);
	BurnTimerAttachZet(3579545);
	
	if (toramich) MSM5205Init(0, DrvMSM5205SynchroniseStream, 384000, NULL, MSM5205_SEX_4B, 100, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	BurnYM2203Exit();
	if (toramich) MSM5205Exit();

	GenericTilesExit();

	SekExit();
	ZetExit();

	BurnFree (AllMem);

	nF1dream = 0;
	toramich = 0;

	return 0;
}

static void draw_sprites()
{
	UINT16 *source = (UINT16*)DrvSprBuf;

	for (INT32 offs = (0x500 - 8) / 2; offs >= 0; offs -=4)
	{
		INT32 tile_number = source[offs + 0];

		if (tile_number != 0xfff) {
			INT32 attr = source[offs + 1];
			INT32 sy = source[offs + 2] & 0x1ff;
			INT32 sx = source[offs + 3] & 0x1ff;

			INT32 flipx = attr & 0x02;
			INT32 flipy = attr & 0x01;
			INT32 color = (attr >> 2) & 0x0f;

			if (sx > 0x100) sx -= 0x200;
			if (sy > 0x100) sy -= 0x200;

			if (*flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			sy = (240 - sy) - 16;

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, tile_number, sx, sy, color, 4, 15, 0x100, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, tile_number, sx, sy, color, 4, 15, 0x100, DrvGfxROM2);	
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, tile_number, sx, sy, color, 4, 15, 0x100, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, tile_number, sx, sy, color, 4, 15, 0x100, DrvGfxROM2);	
				}
			}
		}
	}
}

static void draw_32x32_mask_tile(INT32 sx, INT32 sy, INT32 code, INT32 color, INT32 flipx, INT32 flipy)
{
	UINT8 *src = DrvGfxROM1 + (code * 0x400) + (flipy * 0x3e0);
	UINT16 *dst;

	INT32 increment = flipy ? -32 : 32;

	for (INT32 y = 0; y < 32; y++, sy++)
	{
		if (sy >= nScreenHeight) break;

		if (sy >= 0)
		{
			dst = pTransDraw + (sy * nScreenWidth);

			if (flipx) {
				for (INT32 x = 31; x >= 0; x--)
				{
					if ((sx+x) < 0 || (sx+x) >= nScreenWidth) continue;
		
					if (DrvTransMask[src[x^0x1f]])
						dst[sx+x] = src[x^0x1f] | color;
				}
			} else {
				for (INT32 x = 0; x < 32; x++)
				{
					if ((sx+x) < 0 || (sx+x) >= nScreenWidth) continue;
		
					if (DrvTransMask[src[x]])
						dst[sx+x] = src[x] | color;
				}
			}
		}

		src += increment;
	}
}

static void draw_background(INT32 priority)
{
	INT32 scrollx = *((UINT16*)(DrvScrollRAM + 0));
	INT32 scrolly = *((UINT16*)(DrvScrollRAM + 2));

	scrollx &= 0xfff;

	scrolly = 0 - scrolly;
	scrolly -= 0x100;
	scrolly &= 0xfff;

	for (INT32 y = 0; y < 8+1; y++)
	{
		for (INT32 x = 0; x < 8+1; x++)
		{
			INT32 sx = x + (scrollx >> 5);
			INT32 sy = y + (scrolly >> 5);

			INT32 ofst = ((sx & 7) << 1) + (((127 - sy) & 7) << 4) + ((sx >> 3) << 7) + (((127 - sy) >> 3) << 11);

			INT32 attr = DrvGfxROM3[ofst + 1];
			INT32 prio = attr & 0x10;
			if (priority && !prio) continue;
	
			INT32 data = DrvGfxROM3[ofst];
			INT32 code = data + ((attr & 0xc0) << 2) + (*bgcharbank << 10);
			INT32 color = attr & 0x0f;
			INT32 flipx = attr & 0x20;
			INT32 flipy = 0;

			sx = (x << 5) - (scrollx & 0x1f);
			sy = (y << 5) - (scrolly & 0x1f);

			if (*flipscreen) {
				flipx ^= 0x20;
				flipy ^= 1;

				sx = 224 - sx;
				sy = 224 - sy;
			}

			if (!priority) {
				if (flipy) {
					if (flipx) {
						Render32x32Tile_FlipXY_Clip(pTransDraw, code, sx, sy - 16, color, 4, 0, DrvGfxROM1);
					} else {
						Render32x32Tile_FlipY_Clip(pTransDraw, code, sx, sy - 16, color, 4, 0, DrvGfxROM1);
					}
				} else {
					if (flipx) {
						Render32x32Tile_FlipX_Clip(pTransDraw, code, sx, sy - 16, color, 4, 0, DrvGfxROM1);
					} else {
						Render32x32Tile_Clip(pTransDraw, code, sx, sy - 16, color, 4, 0, DrvGfxROM1);
					}
				}
			} else {
				draw_32x32_mask_tile(sx, sy - 16, code, color << 4, flipx, flipy);
			}
		}
	}
}

static void draw_text_layer()
{
	UINT16 *vram = (UINT16*)DrvVidRAM;

	for (INT32 offs = 0x40; offs < 0x3c0; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		INT32 data = vram[offs];
		INT32 attr = data >> 8;
		INT32 code = (data & 0xff) + ((attr & 0xc0) << 2) + ((attr & 0x20) << 5);
		if (code == 0x400) continue;

		INT32 color = attr & 0x0f;
		INT32 flipx = attr & 0x10;
		INT32 flipy = 0;

		if (*flipscreen) {
			sx ^= 0xf8;
			sy ^= 0xf8;
			flipx ^= 0x10;
			flipy ^= 0x01;
		}

		if (flipy) {
			if (flipx) {
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy - 16, color, 2, 3, 0x200, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy - 16, color, 2, 3, 0x200, DrvGfxROM0);
			}
		} else {
			if (flipx) {
				Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code, sx - 16, sy, color, 2, 3, 0x200, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy - 16, color, 2, 3, 0x200, DrvGfxROM0);
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x240 * 2; i+=2) {
			palette_write(i);
		}
	}

	memset (pTransDraw, 0, nScreenWidth * nScreenHeight * 2);

	draw_background(0);
	draw_sprites();
	draw_background(1);
	draw_text_layer();

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
		DrvInputs[1] = ~0;

		for (INT32 i = 0; i < 16; i++)
		{
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}

		DrvInputs[1] |= *coin_lockout << 8;
	}

	INT32 nCyclesTotal[3] = { 10000000 / 60, 3579545 / 60, 3579545 / 60 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	SekNewFrame();
	ZetNewFrame();
	
	INT32 nInterleave = 10;
	INT32 MSMIRQSlice[67];
	
	if (toramich) {
		nInterleave = MSM5205CalcInterleave(0, 10000000);
	
		for (INT32 i = 0; i < 67; i++) {
			MSMIRQSlice[i] = (INT32)((double)((nInterleave * (i + 1)) / 68));
		}
	}
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext, nCyclesSegment;

		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == (nInterleave - 1)) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		if (toramich) MSM5205Update();
		SekClose();
		
		ZetOpen(0);
		BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));
		ZetClose();
		
		if (toramich) {
			nCurrentCPU = 2;
			ZetOpen(1);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
			for (INT32 j = 0; j < 67; j++) {
				if (i == MSMIRQSlice[j]) {
					ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
					nCyclesDone[nCurrentCPU] += ZetRun(1000);
				}
			}
			ZetClose();
		}
	}
	
	ZetOpen(0);
	BurnTimerEndFrame(nCyclesTotal[1]);
	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		if (toramich) {
			SekOpen(0);
			MSM5205Render(0, pBurnSoundOut, nBurnSoundLen);
			SekClose();
		}
	}
	ZetClose();	

	if (pBurnDraw) {
		DrvDraw();
	}

	memcpy (DrvSprBuf, DrvSprRAM, 0x500);

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029698;
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
		ZetScan(nAction);

		BurnYM2203Scan(nAction, pnMin);
	}

	return 0;
}


// Tiger Road (US)

static struct BurnRomInfo tigeroadRomDesc[] = {
	{ "tru02.bin",	0x20000, 0x8d283a95, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "tru04.bin",	0x20000, 0x72e2ef20, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "tru05.bin",	0x08000, 0xf9a7c9bf, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "tr01.bin",	0x08000, 0x74a9f08c, 3 | BRF_GRA },           //  3 Character Tiles

	{ "tr-01a.bin",	0x20000, 0xa8aa2e59, 4 | BRF_GRA },           //  4 Background Tiles
	{ "tr-04a.bin",	0x20000, 0x8863a63c, 4 | BRF_GRA },           //  5
	{ "tr-02a.bin",	0x20000, 0x1a2c5f89, 4 | BRF_GRA },           //  6
	{ "tr05.bin",	0x20000, 0x5bf453b3, 4 | BRF_GRA },           //  7
	{ "tr-03a.bin",	0x20000, 0x1e0537ea, 4 | BRF_GRA },           //  8
	{ "tr-06a.bin",	0x20000, 0xb636c23a, 4 | BRF_GRA },           //  9
	{ "tr-07a.bin",	0x20000, 0x5f907d4d, 4 | BRF_GRA },           // 10
	{ "tr08.bin",	0x20000, 0xadee35e2, 4 | BRF_GRA },           // 11

	{ "tr-09a.bin",	0x20000, 0x3d98ad1e, 5 | BRF_GRA },           // 12 Sprites
	{ "tr-10a.bin",	0x20000, 0x8f6f03d7, 5 | BRF_GRA },           // 13
	{ "tr-11a.bin",	0x20000, 0xcd9152e5, 5 | BRF_GRA },           // 14
	{ "tr-12a.bin",	0x20000, 0x7d8a99d0, 5 | BRF_GRA },           // 15

	{ "tr13.bin",	0x08000, 0xa79be1eb, 6 | BRF_GRA },           // 16 Background Tilemaps

	{ "trprom.bin",	0x00100, 0xec80ae36, 7 | BRF_GRA | BRF_OPT }, // 17 Priority Proms (unused)
};

STD_ROM_PICK(tigeroad)
STD_ROM_FN(tigeroad)

static INT32 TigeroadLoadRoms()
{
	if (BurnLoadRom(Drv68KROM + 0x000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x000000,  1, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM + 0x000000,  2, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x00000,  3, 1)) return 1;

	for (INT32 i = 0; i < 8; i++) {
		if (BurnLoadRom(DrvGfxROM1 + i * 0x20000,  4 + i, 1)) return 1;
	}

	for (INT32 i = 0; i < 4; i++) {
		if (BurnLoadRom(DrvGfxROM2 + i * 0x20000, 12 + i, 1)) return 1;
	}

	if (BurnLoadRom(DrvGfxROM3 + 0x00000, 16, 1)) return 1;

	if (toramich) if (BurnLoadRom(DrvSndROM + 0x000000, 18, 1)) return 1;

	return 0;
}

static INT32 TigeroadInit()
{
	return DrvInit(TigeroadLoadRoms);
}

struct BurnDriver BurnDrvTigeroad = {
	"tigeroad", NULL, NULL, NULL, "1987",
	"Tiger Road (US)\0", NULL, "Capcom (Romstar license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARWARE_CAPCOM_MISC, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, tigeroadRomInfo, tigeroadRomName, NULL, NULL, TigeroadInputInfo, TigeroadDIPInfo,
	TigeroadInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x240,
	256, 224, 4, 3
};


// Tora-he no Michi (Japan)

static struct BurnRomInfo toramichRomDesc[] = {
	{ "tr_02.bin",	0x20000, 0xb54723b1, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "tr_04.bin",	0x20000, 0xab432479, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "tr_05.bin",	0x08000, 0x3ebe6e62, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "tr01.bin",	0x08000, 0x74a9f08c, 3 | BRF_GRA },           //  3 Character Tiles

	{ "tr-01a.bin",	0x20000, 0xa8aa2e59, 4 | BRF_GRA },           //  4 Background Tiles
	{ "tr-04a.bin",	0x20000, 0x8863a63c, 4 | BRF_GRA },           //  5
	{ "tr-02a.bin",	0x20000, 0x1a2c5f89, 4 | BRF_GRA },           //  6
	{ "tr05.bin",	0x20000, 0x5bf453b3, 4 | BRF_GRA },           //  7
	{ "tr-03a.bin",	0x20000, 0x1e0537ea, 4 | BRF_GRA },           //  8
	{ "tr-06a.bin",	0x20000, 0xb636c23a, 4 | BRF_GRA },           //  9
	{ "tr-07a.bin",	0x20000, 0x5f907d4d, 4 | BRF_GRA },           // 10
	{ "tr08.bin",	0x20000, 0xadee35e2, 4 | BRF_GRA },           // 11

	{ "tr-09a.bin",	0x20000, 0x3d98ad1e, 5 | BRF_GRA },           // 12 Sprites
	{ "tr-10a.bin",	0x20000, 0x8f6f03d7, 5 | BRF_GRA },           // 13
	{ "tr-11a.bin",	0x20000, 0xcd9152e5, 5 | BRF_GRA },           // 14
	{ "tr-12a.bin",	0x20000, 0x7d8a99d0, 5 | BRF_GRA },           // 15

	{ "tr13.bin",	0x08000, 0xa79be1eb, 6 | BRF_GRA },           // 16 Background Tilemaps

	{ "trprom.bin",	0x00100, 0xec80ae36, 7 | BRF_GRA | BRF_OPT }, // 17 Priority Proms (unused)

	{ "tr_03.bin",	0x10000, 0xea1807ef, 8 | BRF_PRG | BRF_ESS }, // 18 Sample Z80 Code
};

STD_ROM_PICK(toramich)
STD_ROM_FN(toramich)

static INT32 ToramichInit()
{
	toramich = 1;

	return DrvInit(TigeroadLoadRoms);
}

struct BurnDriver BurnDrvToramich = {
	"toramich", "tigeroad", NULL, NULL, "1987",
	"Tora-he no Michi (Japan)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, toramichRomInfo, toramichRomName, NULL, NULL, TigeroadInputInfo, ToramichDIPInfo,
	ToramichInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x240,
	256, 224, 4, 3
};


// Tiger Road (US bootleg)

static struct BurnRomInfo tigerodbRomDesc[] = {
	{ "tgrroad.3",	0x10000, 0x14c87e07, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "tgrroad.5",	0x10000, 0x0904254c, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "tgrroad.2",	0x10000, 0xcedb1f46, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "tgrroad.4",	0x10000, 0xe117f0b1, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "tru05.bin",	0x08000, 0xf9a7c9bf, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "tr01.bin",	0x08000, 0x74a9f08c, 3 | BRF_GRA },           //  5 Character Tiles

	{ "tr-01a.bin",	0x20000, 0xa8aa2e59, 4 | BRF_GRA },           //  6 Background Tiles
	{ "tr-04a.bin",	0x20000, 0x8863a63c, 4 | BRF_GRA },           //  7
	{ "tr-02a.bin",	0x20000, 0x1a2c5f89, 4 | BRF_GRA },           //  8
	{ "tr05.bin",	0x20000, 0x5bf453b3, 4 | BRF_GRA },           //  9
	{ "tr-03a.bin",	0x20000, 0x1e0537ea, 4 | BRF_GRA },           // 10
	{ "tr-06a.bin",	0x20000, 0xb636c23a, 4 | BRF_GRA },           // 11
	{ "tr-07a.bin",	0x20000, 0x5f907d4d, 4 | BRF_GRA },           // 12
	{ "tgrroad.17",	0x10000, 0x3f7539cc, 4 | BRF_GRA },           // 13
	{ "tgrroad.18",	0x10000, 0xe2e053cb, 4 | BRF_GRA },           // 14

	{ "tr-09a.bin",	0x20000, 0x3d98ad1e, 5 | BRF_GRA },           // 15 Sprites
	{ "tr-10a.bin",	0x20000, 0x8f6f03d7, 5 | BRF_GRA },           // 16
	{ "tr-11a.bin",	0x20000, 0xcd9152e5, 5 | BRF_GRA },           // 17
	{ "tr-12a.bin",	0x20000, 0x7d8a99d0, 5 | BRF_GRA },           // 18

	{ "tr13.bin",	0x08000, 0xa79be1eb, 6 | BRF_GRA },           // 19 Background Tilemaps

	{ "trprom.bin",	0x00100, 0xec80ae36, 7 | BRF_GRA | BRF_OPT }, // 20 Priority Proms (unused)
};

STD_ROM_PICK(tigerodb)
STD_ROM_FN(tigerodb)

static INT32 TigerodbLoadRoms()
{
	if (BurnLoadRom(Drv68KROM + 0x000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x000000,  1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020001,  2, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020000,  3, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM + 0x000000,  4, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x00000,  5, 1)) return 1;

	for (INT32 i = 0; i < 8; i++) {
		if (BurnLoadRom(DrvGfxROM1 + i * 0x20000,  6 + i, 1)) return 1;
	}

	if (BurnLoadRom(DrvGfxROM1 + 0xf0000, 14, 1)) return 1;

	for (INT32 i = 0; i < 4; i++) {
		if (BurnLoadRom(DrvGfxROM2 + i * 0x20000, 15 + i, 1)) return 1;
	}

	if (BurnLoadRom(DrvGfxROM3 + 0x00000, 19, 1)) return 1;

	return 0;
}

static INT32 TigerodbInit()
{
	return DrvInit(TigerodbLoadRoms);
}

struct BurnDriver BurnDrvTigerodb = {
	"tigeroadb", "tigeroad", NULL, NULL, "1987",
	"Tiger Road (US bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARWARE_CAPCOM_MISC, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, tigerodbRomInfo, tigerodbRomName, NULL, NULL, TigeroadInputInfo, TigeroadDIPInfo,
	TigerodbInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x240,
	256, 224, 4, 3
};


// F-1 Dream

static struct BurnRomInfo f1dreamRomDesc[] = {
	{ "06j_02.bin",	0x20000, 0x3c2ec697, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "06k_03.bin",	0x20000, 0x85ebad91, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "12k_04.bin",	0x08000, 0x4b9a7524, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "10d_01.bin",	0x08000, 0x361caf00, 3 | BRF_GRA },           //  3 Character Tiles

	{ "03f_12.bin",	0x10000, 0xbc13e43c, 4 | BRF_GRA },           //  4 Background Tiles
	{ "01f_10.bin",	0x10000, 0xf7617ad9, 4 | BRF_GRA },           //  5
	{ "03h_14.bin",	0x10000, 0xe33cd438, 4 | BRF_GRA },           //  6
	{ "02f_11.bin",	0x10000, 0x4aa49cd7, 4 | BRF_GRA },           //  7
	{ "17f_09.bin",	0x10000, 0xca622155, 4 | BRF_GRA },           //  8
	{ "02h_13.bin",	0x10000, 0x2a63961e, 4 | BRF_GRA },           //  9

	{ "03b_06.bin",	0x10000, 0x5e54e391, 5 | BRF_GRA },           // 10 Sprites
	{ "02b_05.bin",	0x10000, 0xcdd119fd, 5 | BRF_GRA },           // 11
	{ "03d_08.bin",	0x10000, 0x811f2e22, 5 | BRF_GRA },           // 12
	{ "02d_07.bin",	0x10000, 0xaa9a1233, 5 | BRF_GRA },           // 13

	{ "07l_15.bin",	0x08000, 0x978758b7, 6 | BRF_GRA },           // 14 Background Tilemaps

	{ "09e_tr.bin",	0x00100, 0xec80ae36, 7 | BRF_GRA | BRF_OPT }, // 15 Priority Proms (unused)
	
	{ "c8751h-88",  0x01000, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },
};

STD_ROM_PICK(f1dream)
STD_ROM_FN(f1dream)

static INT32 F1dreamLoadRoms()
{
	if (BurnLoadRom(Drv68KROM + 0x000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x000000,  1, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM + 0x000000,  2, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x00000,  3, 1)) return 1;

	for (INT32 i = 0; i < 3; i++) {
		if (BurnLoadRom(DrvGfxROM1 + 0x00000 + i * 0x10000,  4 + i, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x80000 + i * 0x10000,  7 + i, 1)) return 1;
	}

	for (INT32 i = 0; i < 4; i++) {
		if (BurnLoadRom(DrvGfxROM2 + i * 0x20000, 10 + i, 1)) return 1;
	}

	if (BurnLoadRom(DrvGfxROM3 + 0x00000, 14, 1)) return 1;

	return 0;
}

static INT32 F1dreamInit()
{
	nF1dream = 1;

	return DrvInit(F1dreamLoadRoms);
}

struct BurnDriver BurnDrvF1dream = {
	"f1dream", NULL, NULL, NULL, "1988",
	"F-1 Dream\0", NULL, "Capcom (Romstar license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARWARE_CAPCOM_MISC, GBF_RACING, 0,
	NULL, f1dreamRomInfo, f1dreamRomName, NULL, NULL, TigeroadInputInfo, F1dreamDIPInfo,
	F1dreamInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x240,
	256, 224, 4, 3
};


// F-1 Dream (bootleg)

static struct BurnRomInfo f1dreambRomDesc[] = {
	{ "f1d_04.bin",	0x10000, 0x903febad, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "f1d_05.bin",	0x10000, 0x666fa2a7, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "f1d_02.bin",	0x10000, 0x98973c4c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "f1d_03.bin",	0x10000, 0x3d21c78a, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "12k_04.bin",	0x08000, 0x4b9a7524, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "10d_01.bin",	0x08000, 0x361caf00, 3 | BRF_GRA },           //  5 Character Tiles

	{ "03f_12.bin",	0x10000, 0xbc13e43c, 4 | BRF_GRA },           //  6 Background Tiles
	{ "01f_10.bin",	0x10000, 0xf7617ad9, 4 | BRF_GRA },           //  7
	{ "03h_14.bin",	0x10000, 0xe33cd438, 4 | BRF_GRA },           //  8
	{ "02f_11.bin",	0x10000, 0x4aa49cd7, 4 | BRF_GRA },           //  9
	{ "17f_09.bin",	0x10000, 0xca622155, 4 | BRF_GRA },           // 10
	{ "02h_13.bin",	0x10000, 0x2a63961e, 4 | BRF_GRA },           // 11

	{ "03b_06.bin",	0x10000, 0x5e54e391, 5 | BRF_GRA },           // 12 Sprites
	{ "02b_05.bin",	0x10000, 0xcdd119fd, 5 | BRF_GRA },           // 13
	{ "03d_08.bin",	0x10000, 0x811f2e22, 5 | BRF_GRA },           // 14
	{ "02d_07.bin",	0x10000, 0xaa9a1233, 5 | BRF_GRA },           // 15

	{ "07l_15.bin",	0x08000, 0x978758b7, 6 | BRF_GRA },           // 16 Background Tilemaps

	{ "09e_tr.bin",	0x00100, 0xec80ae36, 7 | BRF_GRA | BRF_OPT }, // 17 Priority Proms (unused)
};

STD_ROM_PICK(f1dreamb)
STD_ROM_FN(f1dreamb)

static INT32 F1dreambLoadRoms()
{
	if (BurnLoadRom(Drv68KROM + 0x000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x000000,  1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020001,  2, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x020000,  3, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM + 0x000000,  4, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x00000,  5, 1)) return 1;

	for (INT32 i = 0; i < 3; i++) {
		if (BurnLoadRom(DrvGfxROM1 + 0x00000 + i * 0x10000,  6 + i, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x80000 + i * 0x10000,  9 + i, 1)) return 1;
	}

	for (INT32 i = 0; i < 4; i++) {
		if (BurnLoadRom(DrvGfxROM2 + i * 0x20000, 12 + i, 1)) return 1;
	}

	if (BurnLoadRom(DrvGfxROM3 + 0x00000, 16, 1)) return 1;

	return 0;
}

static INT32 F1dreambInit()
{
	return DrvInit(F1dreambLoadRoms);
}

struct BurnDriver BurnDrvF1dreamb = {
	"f1dreamb", "f1dream", NULL, NULL, "1988",
	"F-1 Dream (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARWARE_CAPCOM_MISC, GBF_RACING, 0,
	NULL, f1dreambRomInfo, f1dreambRomName, NULL, NULL, TigeroadInputInfo, F1dreamDIPInfo,
	F1dreambInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x240,
	256, 224, 4, 3
};
