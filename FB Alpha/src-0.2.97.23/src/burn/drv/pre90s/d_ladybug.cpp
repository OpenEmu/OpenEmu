// FB Alpha Lady Bug driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "sn76496.h"
#include "bitswap.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvZ80Ops0;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvColPROM;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvColRAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvGridRAM;

static UINT32 *Palette;
static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *flipscreen;
static UINT8 *soundlatch;
static UINT8 *gridcolor;
static UINT8 *enablestars;
static UINT8 *starspeed;

static INT32 stars_offset;
static INT32 stars_state;
static INT32 vblank;
static INT32 ladybug = 0;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvJoy4[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[4];
static UINT8 DrvReset;

static struct BurnInputInfo LadybugInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy4 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy4 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Tilt",		BIT_DIGITAL,	DrvJoy1 + 7,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Ladybug)

static struct BurnInputInfo SraiderInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Sraider)

static struct BurnDIPInfo LadybugDIPList[]=
{
	{0x12, 0xff, 0xff, 0xdf, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x03, 0x03, "Easy"			},
	{0x12, 0x01, 0x03, 0x02, "Medium"		},
	{0x12, 0x01, 0x03, 0x01, "Hard"			},
	{0x12, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "High Score Names"	},
	{0x12, 0x01, 0x04, 0x00, "3 Letters"		},
	{0x12, 0x01, 0x04, 0x04, "10 Letters"		},

	{0   , 0xfe, 0   ,    2, "Rack Test (Cheat)"	},
	{0x12, 0x01, 0x08, 0x08, "Off"			},
	{0x12, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x12, 0x01, 0x10, 0x10, "Off"			},
	{0x12, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x20, 0x00, "Upright"		},
	{0x12, 0x01, 0x20, 0x20, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x12, 0x01, 0x40, 0x40, "No"			},
	{0x12, 0x01, 0x40, 0x00, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x12, 0x01, 0x80, 0x80, "3"			},
	{0x12, 0x01, 0x80, 0x00, "5"			},

	{0   , 0xfe, 0   ,    10, "Coin B"		},
	{0x13, 0x01, 0x0f, 0x06, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x08, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x0a, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x07, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x0f, 0x09, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,    10, "Coin A"		},
	{0x13, 0x01, 0xf0, 0x60, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x80, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xa0, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x70, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xf0, 0x90, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
};

STDDIPINFO(Ladybug)

static struct BurnDIPInfo SnapjackDIPList[]=
{
	{0x12, 0xff, 0xff, 0xf7, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x03, 0x03, "Easy"			},
	{0x12, 0x01, 0x03, 0x02, "Medium"		},
	{0x12, 0x01, 0x03, 0x01, "Hard"			},
	{0x12, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "High Score Names"	},
	{0x12, 0x01, 0x04, 0x00, "3 Letters"		},
	{0x12, 0x01, 0x04, 0x04, "10 Letters"		},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x08, 0x00, "Upright"		},
	{0x12, 0x01, 0x08, 0x08, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x12, 0x01, 0xc0, 0x00, "2"			},
	{0x12, 0x01, 0xc0, 0xc0, "3"			},
	{0x12, 0x01, 0xc0, 0x80, "4"			},
	{0x12, 0x01, 0xc0, 0x40, "5"			},

	{0   , 0xfe, 0   ,    11, "Coin B"		},
	{0x13, 0x01, 0x0f, 0x05, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x07, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x0a, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x06, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0x0f, 0x09, "2 Coins 2 Credits"	},
	{0x13, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x0f, 0x08, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,    11, "Coin A"		},
	{0x13, 0x01, 0xf0, 0x50, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x70, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xa0, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x60, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0x90, "2 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xf0, 0x80, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
};

STDDIPINFO(Snapjack)

static struct BurnDIPInfo CavengerDIPList[]=
{
	{0x12, 0xff, 0xff, 0xc7, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x03, 0x03, "Easy"			},
	{0x12, 0x01, 0x03, 0x02, "Medium"		},
	{0x12, 0x01, 0x03, 0x01, "Hard"			},
	{0x12, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "High Score Names"	},
	{0x12, 0x01, 0x04, 0x00, "3 Letters"		},
	{0x12, 0x01, 0x04, 0x04, "10 Letters"		},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x08, 0x00, "Upright"		},
	{0x12, 0x01, 0x08, 0x08, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Initial High Score"	},
	{0x12, 0x01, 0x30, 0x00, "0"			},
	{0x12, 0x01, 0x30, 0x30, "5000"			},
	{0x12, 0x01, 0x30, 0x20, "8000"			},
	{0x12, 0x01, 0x30, 0x10, "10000"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x12, 0x01, 0xc0, 0x00, "2"			},
	{0x12, 0x01, 0xc0, 0xc0, "3"			},
	{0x12, 0x01, 0xc0, 0x80, "4"			},
	{0x12, 0x01, 0xc0, 0x40, "5"			},

	{0   , 0xfe, 0   ,    10, "Coin B"		},
	{0x13, 0x01, 0x0f, 0x06, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x08, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x0a, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x07, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x0f, 0x09, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,    10, "Coin A"		},
	{0x13, 0x01, 0xf0, 0x60, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x80, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xa0, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x70, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xf0, 0x90, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
};

STDDIPINFO(Cavenger)

static struct BurnDIPInfo DorodonDIPList[]=
{
	{0x12, 0xff, 0xff, 0xdf, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x03, 0x03, "Easy"			},
	{0x12, 0x01, 0x03, 0x02, "Medium"		},
	{0x12, 0x01, 0x03, 0x01, "Hard"			},
	{0x12, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Bonus Life"		},
	{0x12, 0x01, 0x04, 0x04, "20000"		},
	{0x12, 0x01, 0x04, 0x00, "30000"		},

	{0   , 0xfe, 0   ,    2, "Rack Test (Cheat)"	},
	{0x12, 0x01, 0x08, 0x08, "Off"			},
	{0x12, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x12, 0x01, 0x10, 0x10, "Off"			},
	{0x12, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x12, 0x01, 0x20, 0x00, "Upright"		},
	{0x12, 0x01, 0x20, 0x20, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x12, 0x01, 0x40, 0x40, "No"			},
	{0x12, 0x01, 0x40, 0x00, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x12, 0x01, 0x80, 0x80, "3"			},
	{0x12, 0x01, 0x80, 0x00, "5"			},

	{0   , 0xfe, 0   ,    10, "Coin B"		},
	{0x13, 0x01, 0x0f, 0x06, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x08, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x0a, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x07, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x0f, 0x09, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,    10, "Coin A"		},
	{0x13, 0x01, 0xf0, 0x60, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x80, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xa0, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x70, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xf0, 0x90, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
};

STDDIPINFO(Dorodon)

static struct BurnDIPInfo SraiderDIPList[]=
{
	{0x0f, 0xff, 0xff, 0xdf, NULL			},
	{0x10, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x0f, 0x01, 0x03, 0x03, "Easy"			},
	{0x0f, 0x01, 0x03, 0x02, "Medium"		},
	{0x0f, 0x01, 0x03, 0x01, "Hard"			},
	{0x0f, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "High Score Names"	},
	{0x0f, 0x01, 0x04, 0x00, "3 Letters"		},
	{0x0f, 0x01, 0x04, 0x04, "10 Letters"		},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x0f, 0x01, 0x08, 0x08, "No"			},
	{0x0f, 0x01, 0x08, 0x00, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x0f, 0x01, 0x20, 0x00, "Upright"		},
	{0x0f, 0x01, 0x20, 0x20, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x0f, 0x01, 0xc0, 0x00, "2"			},
	{0x0f, 0x01, 0xc0, 0xc0, "3"			},
	{0x0f, 0x01, 0xc0, 0x80, "4"			},
	{0x0f, 0x01, 0xc0, 0x40, "5"			},

	{0   , 0xfe, 0   ,    11, "Coin A"		},
	{0x10, 0x01, 0x0f, 0x06, "4 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x08, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x0a, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x07, "3 Coins 2 Credits"	},
	{0x10, 0x01, 0x0f, 0x09, "2 Coins 2 Credits"	},
	{0x10, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x10, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x10, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x10, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x10, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    11, "Coin B"		},
	{0x10, 0x01, 0xf0, 0x60, "4 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0x80, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0xa0, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0x70, "3 Coins 2 Credits"	},
	{0x10, 0x01, 0xf0, 0x90, "2 Coins 2 Credits"	},
	{0x10, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x10, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x10, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x10, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x10, 0x01, 0xf0, 0x00, "Free Play"		},
};

STDDIPINFO(Sraider)

void __fastcall ladybug_write(UINT16 address, UINT8 data)
{
	switch (address & 0xf000)
	{
		case 0xa000:
			*flipscreen = data & 1;
		return;

		case 0xb000:
			SN76496Write(0, data);
		return;

		case 0xc000:
			SN76496Write(1, data);
		return;
	}
}

UINT8 __fastcall ladybug_read(UINT16 address)
{
	switch (address)
	{
		case 0x8005:
			return 0x3e;

		case 0x9000:
			return DrvInputs[0];

		case 0x9001: {
			if (ladybug) {
				if (DrvDips[0] & 0x20) {
					return DrvInputs[1] ^ vblank;
				} else {
					return (DrvInputs[0] & 0x7f) ^ vblank;
				}
			}

			return DrvInputs[1] ^ vblank;
		}

		case 0x9002:
			return DrvDips[0];

		case 0x9003:
			return DrvDips[1];

		case 0xe000:
			return DrvInputs[2];
	}

	return 0;
}

void __fastcall sraider_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x8006:
			soundlatch[0] = data;
		return;

		case 0x8007:
			soundlatch[1] = data;
		return;
	}
}

void __fastcall sraider_sub_write(UINT16 address, UINT8 data)
{
	if (address == 0xe800) {
		*flipscreen 	= data & 0x80;
		*gridcolor  	= data & 0x70;
		*enablestars	= data & 0x08;
		*starspeed	=(data & 0x07) - 1;
	}
}

UINT8 __fastcall sraider_sub_read(UINT16 address)
{
	switch (address)
	{
		case 0x8000:
			return soundlatch[0];

		case 0xa000:
			return soundlatch[1];
	}

	return 0;
}

void __fastcall sraider_sub_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			SN76496Write(0, data);
		return;

		case 0x08:
			SN76496Write(1, data);
		return;

		case 0x10:
			SN76496Write(2, data);
		return;

		case 0x18:
			SN76496Write(3, data);
		return;

		case 0x20:
			SN76496Write(4, data);
		return;
	}
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

	stars_offset = 0;
	stars_state = 0;
	vblank = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x010000;
	DrvZ80ROM1	= Next;
	DrvZ80Ops0	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x008000;
	DrvGfxROM1	= Next; Next += 0x008000;
	DrvGfxROM2	= Next; Next += 0x008000;
	DrvGfxROM3	= Next; Next += 0x008000;

	DrvColPROM	= Next; Next += 0x000040;

	Palette		= (UINT32*)Next; Next += 0x0082 * sizeof(UINT32);
	DrvPalette	= (UINT32*)Next; Next += 0x0082 * sizeof(UINT32);

	AllRam		= Next;

	DrvZ80RAM0	= Next; Next += 0x001000;
	DrvZ80RAM1	= Next; Next += 0x000400;

	DrvColRAM	= Next; Next += 0x000400;
	DrvVidRAM	= Next; Next += 0x000400;
	DrvSprRAM	= Next; Next += 0x000400;
	DrvGridRAM	= Next; Next += 0x000100;

	flipscreen	= Next; Next += 0x000001;
	soundlatch	= Next; Next += 0x000002;
	gridcolor	= Next; Next += 0x000001;
	enablestars	= Next; Next += 0x000001;
	starspeed	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[2]  = { 0x0000, 0x8000 };
	INT32 Plane1[2]  = { 0x0001, 0x0000 };
	INT32 XOffs0[8]  = { 0x007, 0x006, 0x005, 0x004, 0x003, 0x002, 0x001, 0x000 };
	INT32 YOffs0[8]  = { 0x000, 0x008, 0x010, 0x018, 0x020, 0x028, 0x030, 0x038 };
	INT32 XOffs1[16] = { 0x000, 0x002, 0x004, 0x006, 0x008, 0x00a, 0x00c, 0x00e,
			   0x080, 0x082, 0x084, 0x086, 0x088, 0x08a, 0x08c, 0x08e };
	INT32 YOffs1[16] = { 0x170, 0x160, 0x150, 0x140, 0x130, 0x120, 0x110, 0x100,
			   0x070, 0x060, 0x050, 0x040, 0x030, 0x020, 0x010, 0x000 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x2000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x2000);

	GfxDecode(0x200, 2,  8,  8, Plane0, XOffs0, YOffs0 + 0, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x2000);

	GfxDecode(0x080, 2, 16, 16, Plane1, XOffs1, YOffs1 + 0, 0x200, tmp, DrvGfxROM1);
	GfxDecode(0x200, 2,  8,  8, Plane1, XOffs1, YOffs1 + 8, 0x080, tmp, DrvGfxROM2);

	memcpy (tmp, DrvGfxROM3, 0x1000);

	GfxDecode(0x200, 1,  8,  8, Plane0, XOffs0, YOffs0 + 0, 0x040, tmp, DrvGfxROM3);

	BurnFree (tmp);

	return 0;
}

static void DrvPaletteInit(INT32 sh0, INT32 sh1, INT32 sh2, INT32 sh3, INT32 sh4, INT32 sh5)
{
	for (INT32 i = 0; i < 0x20; i++)
	{
		INT32 bit0 = (~DrvColPROM[i] >> sh0) & 0x01;
		INT32 bit1 = (~DrvColPROM[i] >> sh1) & 0x01;
		INT32 r = bit0 * 82 + bit1 * 173;

		bit0 = (~DrvColPROM[i] >> sh2) & 0x01;
		bit1 = (~DrvColPROM[i] >> sh3) & 0x01;
		INT32 g = bit0 * 82 + bit1 * 173;

		bit0 = (~DrvColPROM[i] >> sh4) & 0x01;
		bit1 = (~DrvColPROM[i] >> sh5) & 0x01;
		INT32 b = bit0 * 82 + bit1 * 173;

		DrvPalette[i] = (r << 16) | (g << 8) | b;
	}

	for (INT32 i = 0; i < 0x20; i++)
	{
		Palette[i + 0x00] = DrvPalette[((i << 3) & 0x18) | ((i >> 2) & 0x07)];
		Palette[i + 0x20] = DrvPalette[BITSWAP08(DrvColPROM[i + 0x20] & 0x0f, 7,6,5,4,0,1,2,3)];
		Palette[i + 0x40] = DrvPalette[BITSWAP08(DrvColPROM[i + 0x20] >> 4,   7,6,5,4,0,1,2,3)];
	}

	DrvRecalc = 1;
}

static void SraiderPaletteInit()
{
	DrvPaletteInit(3, 0, 5, 4, 7, 6);

	for (INT32 i = 0; i < 0x20; i++)
	{
		INT32 bit0 = (i >> 3) & 0x01;
		INT32 bit1 = (i >> 4) & 0x01;
		INT32 b = 0x47 * bit0 + 0x97 * bit1;

		bit0 = (i >> 1) & 0x01;
		bit1 = (i >> 2) & 0x01;
		INT32 g = 0x47 * bit0 + 0x97 * bit1;

		bit0 = (i >> 0) & 0x01;
		INT32 r = 0x47 * bit0;

		Palette[i + 0x60] = (r << 16) | (g << 8) | b;
	}
}

static INT32 DorodonDecode()
{
	UINT8 *tmp = DrvZ80RAM0;

	if (BurnLoadRom(tmp + 0x000, 7, 1)) return 1;
	if (BurnLoadRom(tmp + 0x100, 8, 1)) return 1;
	for (INT32 i = 0; i < 0x100; i++) {
		tmp[i] = (tmp[i] << 4) | tmp[i + 0x100];
	}

	for (INT32 i = 0; i < 0x10000; i++) {
		DrvZ80Ops0[i] = tmp[DrvZ80ROM0[i]];
	}

	return 0;
}

static INT32 DrvInit(INT32 game_select)
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		switch (game_select)
		{
			case 0: // ladybug, snapjack
			{
				if (BurnLoadRom(DrvZ80ROM0 + 0x00000,  0, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x01000,  1, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x02000,  2, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x03000,  3, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x04000,  4, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x05000,  5, 1)) return 1;
				memcpy (DrvZ80Ops0, DrvZ80ROM0, 0x10000);
		
				if (BurnLoadRom(DrvGfxROM0 + 0x00000,  6, 1)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x01000,  7, 1)) return 1;
		
				if (BurnLoadRom(DrvGfxROM1 + 0x00000,  8, 1)) return 1;
				if (BurnLoadRom(DrvGfxROM1 + 0x01000,  9, 1)) return 1;
		
				if (BurnLoadRom(DrvColPROM + 0x00000, 10, 1)) return 1;
				if (BurnLoadRom(DrvColPROM + 0x00020, 11, 1)) return 1;

				ladybug = 1;
			}
			break;

			case 1: // cavenger
			{
				if (BurnLoadRom(DrvZ80ROM0 + 0x00000,  0, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x01000,  1, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x02000,  2, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x03000,  3, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x04000,  4, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x05000,  5, 1)) return 1;
				memcpy (DrvZ80Ops0, DrvZ80ROM0, 0x10000);
		
				if (BurnLoadRom(DrvGfxROM0 + 0x00000,  6, 1)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x01000,  7, 1)) return 1;
		
				if (BurnLoadRom(DrvGfxROM1 + 0x00000,  8, 1)) return 1;
		
				if (BurnLoadRom(DrvColPROM + 0x00000,  9, 1)) return 1;
				if (BurnLoadRom(DrvColPROM + 0x00020, 10, 1)) return 1;
			}
			break;

			case 2: // dorodon
			{
				if (BurnLoadRom(DrvZ80ROM0 + 0x00000,  0, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x02000,  1, 1)) return 1;
				if (BurnLoadRom(DrvZ80ROM0 + 0x04000,  2, 1)) return 1;
		
				if (BurnLoadRom(DrvGfxROM0 + 0x00000,  3, 1)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x01000,  4, 1)) return 1;
		
				if (BurnLoadRom(DrvGfxROM1 + 0x00000,  5, 1)) return 1;
				if (BurnLoadRom(DrvGfxROM1 + 0x01000,  6, 1)) return 1;

				if (BurnLoadRom(DrvColPROM + 0x00000,  9, 1)) return 1;
				if (BurnLoadRom(DrvColPROM + 0x00020, 10, 1)) return 1;

				DorodonDecode();
			}
			break;
		}

		DrvPaletteInit(0, 5, 2, 6, 4, 7);
		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x5fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x5fff, 2, DrvZ80Ops0, DrvZ80ROM0);
	ZetMapArea(0x6000, 0x6fff, 0, DrvZ80RAM0);
	ZetMapArea(0x6000, 0x6fff, 1, DrvZ80RAM0);
	ZetMapArea(0x6000, 0x6fff, 2, DrvZ80RAM0);
	ZetMapArea(0x7000, 0x73ff, 0, DrvSprRAM);
	ZetMapArea(0x7000, 0x73ff, 1, DrvSprRAM);
	ZetMapArea(0x7000, 0x73ff, 2, DrvSprRAM);
	ZetMapArea(0xd000, 0xd3ff, 0, DrvVidRAM);
	ZetMapArea(0xd000, 0xd3ff, 1, DrvVidRAM);
	ZetMapArea(0xd000, 0xd3ff, 2, DrvVidRAM);
	ZetMapArea(0xd400, 0xd7ff, 0, DrvColRAM);
	ZetMapArea(0xd400, 0xd7ff, 1, DrvColRAM);
	ZetMapArea(0xd400, 0xd7ff, 2, DrvColRAM);
	ZetSetWriteHandler(ladybug_write);
	ZetSetReadHandler(ladybug_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1); // So reset function can be shared

	SN76489Init(0, 4000000, 0);
	SN76489Init(1, 4000000, 1);
	SN76496SetVolShift(0, 2);
	SN76496SetVolShift(1, 2);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 SraiderInit()
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
		if (BurnLoadRom(DrvZ80ROM1 + 0x02000,  4, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM1 + 0x04000,  5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x01000,  7, 1)) return 1;
		
		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x01000,  9, 1)) return 1;
		
		if (BurnLoadRom(DrvGfxROM3 + 0x00000, 10, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x00000, 11, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x00020, 12, 1)) return 1;

		SraiderPaletteInit();
		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x5fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x5fff, 2, DrvZ80ROM0);
	ZetMapArea(0x6000, 0x6fff, 0, DrvZ80RAM0);
	ZetMapArea(0x6000, 0x6fff, 1, DrvZ80RAM0);
	ZetMapArea(0x6000, 0x6fff, 2, DrvZ80RAM0);
	ZetMapArea(0x7000, 0x73ff, 0, DrvSprRAM);
	ZetMapArea(0x7000, 0x73ff, 1, DrvSprRAM);
	ZetMapArea(0x7000, 0x73ff, 2, DrvSprRAM);
	ZetMapArea(0xd000, 0xd3ff, 0, DrvVidRAM);
	ZetMapArea(0xd000, 0xd3ff, 1, DrvVidRAM);
	ZetMapArea(0xd000, 0xd3ff, 2, DrvVidRAM);
	ZetMapArea(0xd400, 0xd7ff, 0, DrvColRAM);
	ZetMapArea(0xd400, 0xd7ff, 1, DrvColRAM);
	ZetMapArea(0xd400, 0xd7ff, 2, DrvColRAM);
	ZetSetWriteHandler(sraider_main_write);
	ZetSetReadHandler(ladybug_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x5fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x5fff, 2, DrvZ80ROM1);
	ZetMapArea(0x6000, 0x63ff, 0, DrvZ80RAM1);
	ZetMapArea(0x6000, 0x63ff, 1, DrvZ80RAM1);
	ZetMapArea(0x6000, 0x63ff, 2, DrvZ80RAM1);
	ZetMapArea(0xe000, 0xe0ff, 0, DrvGridRAM);
	ZetMapArea(0xe000, 0xe0ff, 1, DrvGridRAM);
	ZetMapArea(0xe000, 0xe0ff, 2, DrvGridRAM);
	ZetSetWriteHandler(sraider_sub_write);
	ZetSetReadHandler(sraider_sub_read);
	ZetSetOutHandler(sraider_sub_out);
	ZetMemEnd();
	ZetClose();

	SN76489Init(0, 4000000, 0);
	SN76489Init(1, 4000000, 1);
	SN76489Init(2, 4000000, 1);
	SN76489Init(3, 4000000, 1);
	SN76489Init(4, 4000000, 1);
	SN76496SetVolShift(0, 3);
	SN76496SetVolShift(1, 3);
	SN76496SetVolShift(2, 3);
	SN76496SetVolShift(3, 3);
	SN76496SetVolShift(4, 3);

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

	ladybug = 0;

	return 0;
}

static void draw_layer()
{
	for (INT32 offs = 4 * 32; offs < 32 * 28; offs++) {
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5);

		sx -= DrvVidRAM[((sy & 3) << 5) | (sy >> 2)] + 8;
		if (sx < -7) sx += 256;

		INT32 code  = DrvVidRAM[offs] | ((DrvColRAM[offs] & 0x08) << 5);
		INT32 color = DrvColRAM[offs] & 0x07;

		if (*flipscreen) {
			sx = 240 - sx;
			sy = 196 - sy;	

			Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, (sy * 8) - 32, color, 2, 0, 0, DrvGfxROM0);
		} else {
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx, (sy * 8) - 32, color, 2, 0, 0, DrvGfxROM0);
		}
	}
} 

static void draw_grid()
{
	for (INT32 offs = 4 * 32; offs < 32 * 28; offs++)
	{
		INT32 sx = ((offs & 0x1f) << 3) - 8;
		INT32 sy = ((offs >> 5) << 3) - 32;

		if (*flipscreen) {
			sx = 240 - sx;
			sy = 196 - sy;

			if (offs & 0x200) {
				Render8x8Tile_Mask_FlipX_Clip(pTransDraw, (offs & 0x1ff) ^ 0x1e0, sx, sy, 0, 1, 0, 0x80, DrvGfxROM3);
			} else {
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, offs, sx, sy, 0, 1, 0, 0x80, DrvGfxROM3);
			}
		} else {
			if (offs & 0x200) {
				Render8x8Tile_Mask_FlipY_Clip(pTransDraw, (offs & 0x1ff) ^ 0x1e0, sx, sy, 0, 1, 0, 0x80, DrvGfxROM3);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, offs, sx, sy, 0, 1, 0, 0x80, DrvGfxROM3);
			}
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0x400 - 2*0x40;offs >= 2*0x40;offs -= 0x40)
	{
		INT32 i = 0;

		while (i < 0x40 && DrvSprRAM[offs + i + 0] != 0)
			i += 4;

		while (i > 0)
		{
			i -= 4;

			if (DrvSprRAM[offs + i + 0] & 0x80)
			{
				INT32 size  = (DrvSprRAM[offs + i + 0] >> 6) & 0x01;
				INT32 code  = (DrvSprRAM[offs + i + 1] >> 2) + ((DrvSprRAM[offs + i + 2] & 0x10) << (4 >> size));
				INT32 color = (DrvSprRAM[offs + i + 2] & 0x0f) + 8;
				INT32 flipx =  DrvSprRAM[offs + i + 0] & 0x20;
				INT32 flipy =  DrvSprRAM[offs + i + 0] & 0x10;
				INT32 sx    =  DrvSprRAM[offs + i + 3] - 8;
				INT32 sy    = (DrvSprRAM[offs + i + 0] & 0x0f) + offs / 4 - 32;

				if (size) {
					if (flipy) {
						if (flipx) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy - 8, color, 2, 0, 0, DrvGfxROM1);
						} else {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy - 8, color, 2, 0, 0, DrvGfxROM1);
						}
					} else {
						if (flipx) {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy - 8, color, 2, 0, 0, DrvGfxROM1);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy - 8, color, 2, 0, 0, DrvGfxROM1);
						}
					}
				} else {
					if (flipy) {
						if (flipx) {
							Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
						} else {
							Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
						}
					} else {
						if (flipx) {
							Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
						}
					}
				}
			}
		}
	}
}

static void redclash_update_stars()
{
	static INT32 count = 0;

	if (*enablestars == 0)
		return;

	count = (count + 1) & 1;

	if (count == 0)
	{
		stars_offset += ((*starspeed << 1) - 0x09);
		stars_offset &= 0xffff;
		stars_state = 0;
	}
	else
	{
		stars_state = 0x1fc71;
	}
}

static void redclash_draw_stars(INT32 palette_offset, INT32 sraider, INT32 firstx, INT32 lastx)
{
	UINT8 tempbit, feedback, star_color, hcond,vcond;

	if (*enablestars == 0)
		return;

	UINT32 state = stars_state;

	for (INT32 i = 0; i < 256 * 256; i++)
	{
		INT32 xloc =  (stars_offset + i) & 0xff;
		INT32 yloc = ((stars_offset + i) >> 8) & 0xff;

		xloc -=  8;
		yloc -= 32;

		if ((state & 0x10000) == 0)
			tempbit = 1;
		else
			tempbit = 0;

		if ((state & 0x00020) != 0)
			feedback = tempbit ^ 1;
		else
			feedback = tempbit ^ 0;

		hcond = ((xloc + 8) & 0x10) >> 4;

		if (sraider)
			vcond = 1;
		else
			vcond = yloc & 0x01;

		if (xloc >= 0 && xloc < nScreenWidth &&
			yloc >= 0 && yloc < nScreenHeight)
		{
			if ((hcond ^ vcond) == 0)
			{
				if (((state & 0x000ff) == 0x000ff) && (feedback == 0))
				{
					if ((xloc >= firstx) && (xloc <= lastx))
					{
						star_color = (state >> 9) & 0x1f;
						pTransDraw[(yloc * nScreenWidth) + xloc] = palette_offset + star_color;
					}
				}
			}
		}

		state = ((state << 1) & 0x1fffe) | feedback;
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x60; i++) {
			INT32 d = Palette[i];
			DrvPalette[i] = BurnHighCol(d >> 16, d >> 8, d, 0);
		}
		DrvRecalc = 0;
	}

	BurnTransferClear();

	draw_layer();

	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 SraiderDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x80; i++) {
			INT32 d = Palette[i];
			DrvPalette[i] = BurnHighCol(d >> 16, d >> 8, d, 0);
		}
		DrvRecalc = 0;
	}

	DrvPalette[0x81] = BurnHighCol((*gridcolor & 0x40) ? 0xff : 0, (*gridcolor & 0x20) ? 0xff : 0, (*gridcolor & 0x10) ? 0xff : 0, 0);

	BurnTransferClear();

	if (*flipscreen) {
		redclash_draw_stars(0x60, 1, 0x27, 0xff);
	} else {
		redclash_draw_stars(0x60, 1, 0x00, 0xd8);
	}
	redclash_update_stars();

	draw_grid();

	draw_layer();

	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	INT32 coin = 0;

	{
		INT32 previous = DrvInputs[3] ^ 0xff;

		memset (DrvInputs, 0xff, 4);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
		}
		if ((previous & 1) != (~DrvInputs[3] & 1)) coin |= 1;
		if ((previous & 2) != (~DrvInputs[3] & 2)) coin |= 2;
		DrvInputs[1] &= 0x7f;
	}

	ZetOpen(0);
	if (coin & 1) Z80SetIrqLine(0x20, DrvJoy4[0] ? Z80_ASSERT_LINE : Z80_CLEAR_LINE);
	if (coin & 2) Z80SetIrqLine(0x00, DrvJoy4[1] ? Z80_ASSERT_LINE : Z80_CLEAR_LINE);

	vblank = 0xc0;

	for (INT32 i = 0; i < 32; i++) {
		if (i ==  5) vblank = 0;
		if (i == 31) vblank = 0xc0;

		INT32 nSegment = (4000000 / 60) / 32;

		ZetRun(nSegment);
	}

	ZetClose();

	SN76496Update(0, pBurnSoundOut, nBurnSoundLen);
	SN76496Update(1, pBurnSoundOut, nBurnSoundLen);

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 SraiderFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 2);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nInterleave = 100;
	INT32 nTotalCycles[2] = { 4000000 / 60, 4000000 / 60 };

	for (INT32 i = 0; i < nInterleave; i++) {
		ZetOpen(0);
		ZetRun(nTotalCycles[0] / nInterleave);
		if (i == (nInterleave - 1)) ZetRaiseIrq(0);
		ZetClose();

		ZetOpen(1);
		ZetRun(nTotalCycles[1] / nInterleave);
		if (i == (nInterleave - 1)) ZetRaiseIrq(0);
		ZetClose();
	}

	SN76496Update(0, pBurnSoundOut, nBurnSoundLen);
	SN76496Update(1, pBurnSoundOut, nBurnSoundLen);
	SN76496Update(2, pBurnSoundOut, nBurnSoundLen);
	SN76496Update(3, pBurnSoundOut, nBurnSoundLen);
	SN76496Update(4, pBurnSoundOut, nBurnSoundLen);

	if (pBurnDraw) {
		SraiderDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029706;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		SN76496Scan(nAction, pnMin);

		SCAN_VAR(stars_offset);
		SCAN_VAR(stars_state);
	}

	return 0;
}


// Lady Bug

static struct BurnRomInfo ladybugRomDesc[] = {
	{ "l1.c4",		0x1000, 0xd09e0adb, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "l2.d4",		0x1000, 0x88bc4a0a, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "l3.e4",		0x1000, 0x53e9efce, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "l4.h4",		0x1000, 0xffc424d7, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "l5.j4",		0x1000, 0xad6af809, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "l6.k4",		0x1000, 0xcf1acca4, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "l9.f7",		0x1000, 0x77b1da1e, 2 | BRF_GRA },           //  6 Characters
	{ "l0.h7",		0x1000, 0xaa82e00b, 2 | BRF_GRA },           //  7

	{ "l8.l7",		0x1000, 0x8b99910b, 3 | BRF_GRA },           //  8 Sprites
	{ "l7.m7",		0x1000, 0x86a5b448, 3 | BRF_GRA },           //  9

	{ "10-2.k1",		0x0020, 0xdf091e52, 4 | BRF_GRA },           // 10 Color PROMs
	{ "10-1.f4",		0x0020, 0x40640d8f, 4 | BRF_GRA },           // 11
	{ "10-3.c4",		0x0020, 0x27fa3a50, 4 | BRF_OPT },           // 12
};

STD_ROM_PICK(ladybug)
STD_ROM_FN(ladybug)

static INT32 LadybugInit()
{
	return DrvInit(0);
}

struct BurnDriver BurnDrvLadybug = {
	"ladybug", NULL, NULL, NULL, "1981",
	"Lady Bug\0", NULL, "Universal", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, ladybugRomInfo, ladybugRomName, NULL, NULL, LadybugInputInfo, LadybugDIPInfo,
	LadybugInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x60,
	196, 240, 3, 4
};


// Lady Bug (bootleg set 1)

static struct BurnRomInfo ladybugbRomDesc[] = {
	{ "lb1a.cpu",		0x1000, 0xec135e54, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "lb2a.cpu",		0x1000, 0x3049c5c6, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "lb3a.cpu",		0x1000, 0xb0fef837, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "l4.h4",		0x1000, 0xffc424d7, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "l5.j4",		0x1000, 0xad6af809, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "lb6a.cpu",		0x1000, 0x88c8002a, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "l9.f7",		0x1000, 0x77b1da1e, 2 | BRF_GRA },           //  6 Characters
	{ "l0.h7",		0x1000, 0xaa82e00b, 2 | BRF_GRA },           //  7

	{ "l8.l7",		0x1000, 0x8b99910b, 3 | BRF_GRA },           //  8 Sprites
	{ "l7.m7",		0x1000, 0x86a5b448, 3 | BRF_GRA },           //  9

	{ "10-2.k1",		0x0020, 0xdf091e52, 4 | BRF_GRA },           // 10 Color PROMs
	{ "10-1.f4",		0x0020, 0x40640d8f, 4 | BRF_GRA },           // 11
	{ "10-3.c4",		0x0020, 0x27fa3a50, 4 | BRF_GRA },           // 12
};

STD_ROM_PICK(ladybugb)
STD_ROM_FN(ladybugb)

struct BurnDriver BurnDrvLadybugb = {
	"ladybugb", "ladybug", NULL, NULL, "1981",
	"Lady Bug (bootleg set 1)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, ladybugbRomInfo, ladybugbRomName, NULL, NULL, LadybugInputInfo, LadybugDIPInfo,
	LadybugInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x60,
	196, 240, 3, 4
};


// Lady Bug (bootleg set 2)

static struct BurnRomInfo ladybgb2RomDesc[] = {
	{ "lb1b.cpu",		0x1000, 0x35d61e65, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "lb2b.cpu",		0x1000, 0xa13e0fe4, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "lb3b.cpu",		0x1000, 0xee8ac716, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "l4.h4",		0x1000, 0xffc424d7, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "l5.j4",		0x1000, 0xad6af809, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "lb6b.cpu",		0x1000, 0xdc906e89, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "l9.f7",		0x1000, 0x77b1da1e, 2 | BRF_GRA },           //  6 Characters
	{ "l0.h7",		0x1000, 0xaa82e00b, 2 | BRF_GRA },           //  7

	{ "l8.l7",		0x1000, 0x8b99910b, 3 | BRF_GRA },           //  8 Sprites
	{ "l7.m7",		0x1000, 0x86a5b448, 3 | BRF_GRA },           //  9

	{ "10-2.k1",		0x0020, 0xdf091e52, 4 | BRF_GRA },           // 10 Color PROMs
	{ "10-1.f4",		0x0020, 0x40640d8f, 4 | BRF_GRA },           // 11
	{ "10-3.c4",		0x0020, 0x27fa3a50, 4 | BRF_OPT },           // 12
};

STD_ROM_PICK(ladybgb2)
STD_ROM_FN(ladybgb2)

struct BurnDriver BurnDrvLadybgb2 = {
	"ladybgb2", "ladybug", NULL, NULL, "1981",
	"Lady Bug (bootleg set 2)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, ladybgb2RomInfo, ladybgb2RomName, NULL, NULL, LadybugInputInfo, LadybugDIPInfo,
	LadybugInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x60,
	196, 240, 3, 4
};


// Snap Jack

static struct BurnRomInfo snapjackRomDesc[] = {
	{ "sj1.c4",		0x1000, 0x6b30fcda, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "sj2.d4",		0x1000, 0x1f1088d1, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sj3.e4",		0x1000, 0xedd65f3a, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "sj4.h4",		0x1000, 0xf4481192, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "sj5.j4",		0x1000, 0x1bff7d05, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "sj6.k4",		0x1000, 0x21793edf, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "sj9.f7",		0x1000, 0xff2011c7, 2 | BRF_GRA },           //  6 Characters
	{ "sj0.h7",		0x1000, 0xf097babb, 2 | BRF_GRA },           //  7

	{ "sj8.l7",		0x1000, 0xb7f105b6, 3 | BRF_GRA },           //  8 Sprites
	{ "sj7.m7",		0x1000, 0x1cdb03a8, 3 | BRF_GRA },           //  9

	{ "10-2.k1",		0x0020, 0xcbbd9dd1, 4 | BRF_GRA },           // 10 Color PROMs
	{ "10-1.f4",		0x0020, 0x5b16fbd2, 4 | BRF_GRA },           // 11
	{ "10-3.c4",		0x0020, 0x27fa3a50, 4 | BRF_OPT },           // 12
};

STD_ROM_PICK(snapjack)
STD_ROM_FN(snapjack)

struct BurnDriver BurnDrvSnapjack = {
	"snapjack", NULL, NULL, NULL, "1982",
	"Snap Jack\0", NULL, "Universal", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, snapjackRomInfo, snapjackRomName, NULL, NULL, LadybugInputInfo, SnapjackDIPInfo,
	LadybugInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x60,
	240, 196, 4, 3
};


// Cosmic Avenger

static struct BurnRomInfo cavengerRomDesc[] = {
	{ "1.c4",		0x1000, 0x9e0cc781, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "2.d4",		0x1000, 0x5ce5b950, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3.e4",		0x1000, 0xbc28218d, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "4.h4",		0x1000, 0x2b32e9f5, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "5.j4",		0x1000, 0xd117153e, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "6.k4",		0x1000, 0xc7d366cb, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "9.f7",		0x1000, 0x63357785, 2 | BRF_GRA },           //  6 Characters
	{ "0.h7",		0x1000, 0x52ad1133, 2 | BRF_GRA },           //  7

	{ "8.l7",		0x1000, 0xb022bf2d, 3 | BRF_GRA },           //  8 Sprites

	{ "10-2.k1",		0x0020, 0x42a24dd5, 4 | BRF_GRA },           //  9 Color PROMs
	{ "10-1.f4",		0x0020, 0xd736b8de, 4 | BRF_GRA },           // 10
	{ "10-3.c4",		0x0020, 0x27fa3a50, 4 | BRF_OPT },           // 11
};

STD_ROM_PICK(cavenger)
STD_ROM_FN(cavenger)

static INT32 CavengerInit()
{
	return DrvInit(1);
}

struct BurnDriver BurnDrvCavenger = {
	"cavenger", NULL, NULL, NULL, "1981",
	"Cosmic Avenger\0", NULL, "Universal", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, cavengerRomInfo, cavengerRomName, NULL, NULL, LadybugInputInfo, CavengerDIPInfo,
	CavengerInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x60,
	240, 196, 4, 3
};


// Dorodon (set 1)

static struct BurnRomInfo dorodonRomDesc[] = {
	{ "dorodon.0",		0x2000, 0x460aaf26, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "dorodon.1",		0x2000, 0xd2451eb6, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "dorodon.2",		0x2000, 0xd3c6ee6c, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "dorodon.5",		0x1000, 0x5eee2b85, 2 | BRF_GRA },           //  3 Characters
	{ "dorodon.6",		0x1000, 0x395ac25a, 2 | BRF_GRA },           //  4

	{ "dorodon.4",		0x1000, 0xd70bb50a, 3 | BRF_GRA },           //  5 Sprites
	{ "dorodon.3",		0x1000, 0xe44e59e6, 3 | BRF_GRA },           //  6

	{ "dorodon.bp4",	0x0100, 0xf865c135, 4 | BRF_PRG | BRF_ESS }, //  7 Protection PROMs
	{ "dorodon.bp3",	0x0100, 0x47b2f0bb, 4 | BRF_PRG | BRF_ESS }, //  8

	{ "dorodon.bp0",	0x0020, 0x8fcf0bc8, 5 | BRF_GRA },           //  9 Color PROMs
	{ "dorodon.bp1",	0x0020, 0x3f209be4, 5 | BRF_GRA },           // 10
	{ "dorodon.bp2",	0x0020, 0x27fa3a50, 5 | BRF_OPT },           // 11
};

STD_ROM_PICK(dorodon)
STD_ROM_FN(dorodon)

static INT32 DorodonInit()
{
	return DrvInit(2);
}

struct BurnDriver BurnDrvDorodon = {
	"dorodon", NULL, NULL, NULL, "1982",
	"Dorodon (set 1)\0", NULL, "Falcon", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, dorodonRomInfo, dorodonRomName, NULL, NULL, LadybugInputInfo, DorodonDIPInfo,
	DorodonInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x60,
	196, 240, 3, 4
};


// Dorodon (set 2)

static struct BurnRomInfo dorodon2RomDesc[] = {
	{ "1.3fg",		0x2000, 0x4d05d6f8, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "2.3h",		0x2000, 0x27b43b09, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3.3k",		0x2000, 0x38d2f295, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "6.6a",		0x1000, 0x2a2d8b9c, 2 | BRF_GRA },           //  3 Characters
	{ "7.6bc",		0x1000, 0xd14f95fa, 2 | BRF_GRA },           //  4

	{ "5.3t",		0x1000, 0x54c04f58, 3 | BRF_GRA },           //  5 Sprites
	{ "4.3r",		0x1000, 0x1ebb6493, 3 | BRF_GRA },           //  6

	{ "dorodon.bp4",	0x0100, 0xf865c135, 4 | BRF_PRG | BRF_ESS }, //  7 Protection PROMs
	{ "dorodon.bp3",	0x0100, 0x47b2f0bb, 4 | BRF_PRG | BRF_ESS }, //  8

	{ "dorodon.bp0",	0x0020, 0x8fcf0bc8, 5 | BRF_GRA },           //  9 Color PROMs
	{ "dorodon.bp1",	0x0020, 0x3f209be4, 5 | BRF_GRA },           // 10
	{ "dorodon.bp2",	0x0020, 0x27fa3a50, 5 | BRF_OPT },           // 11
};

STD_ROM_PICK(dorodon2)
STD_ROM_FN(dorodon2)

struct BurnDriver BurnDrvDorodon2 = {
	"dorodon2", "dorodon", NULL, NULL, "1982",
	"Dorodon (set 2)\0", NULL, "Falcon", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, dorodon2RomInfo, dorodon2RomName, NULL, NULL, LadybugInputInfo, DorodonDIPInfo,
	DorodonInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x60,
	196, 240, 3, 4
};


// Space Raider

static struct BurnRomInfo sraiderRomDesc[] = {
	{ "sraid3.r4",		0x2000, 0x0f389774, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "sraid2.n4",		0x2000, 0x38a48db0, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sraid1.m4",		0x2000, 0x2f302a4e, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "sraid-s4.h6",	0x2000, 0x57173a12, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code
	{ "sraid-s5.j6",	0x2000, 0x5a459179, 2 | BRF_PRG | BRF_ESS }, //  4
	{ "sraid-s6.l6",	0x2000, 0xea3aa25d, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "sraid-s0.k6",	0x1000, 0xa0373909, 3 | BRF_GRA },           //  6 Characters
	{ "sraids11.l6",	0x1000, 0xba22d949, 3 | BRF_GRA },           //  7

	{ "sraid-s7.m2",	0x1000, 0x299f8e07, 4 | BRF_GRA },           //  8 Sprites
	{ "sraid-s8.n2",	0x1000, 0x57ba8888, 4 | BRF_GRA },           //  9

	{ "sraid-s9.f6",	0x1000, 0x2380b90f, 5 | BRF_GRA },           // 10 Grid Tiles

	{ "srpr10-1.a2",	0x0020, 0x121fdb99, 6 | BRF_GRA },           // 11 Color PROMs
	{ "srpr10-2.l3",	0x0020, 0x88b67e70, 6 | BRF_GRA },           // 12
	{ "srpr10-3.c1",	0x0020, 0x27fa3a50, 6 | BRF_OPT },           // 13
};

STD_ROM_PICK(sraider)
STD_ROM_FN(sraider)

struct BurnDriver BurnDrvSraider = {
	"sraider", NULL, NULL, NULL, "1982",
	"Space Raider\0", NULL, "Universal", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, sraiderRomInfo, sraiderRomName, NULL, NULL, SraiderInputInfo, SraiderDIPInfo,
	SraiderInit, DrvExit, SraiderFrame, SraiderDraw, DrvScan, &DrvRecalc, 0x82,
	196, 240, 3, 4
};
