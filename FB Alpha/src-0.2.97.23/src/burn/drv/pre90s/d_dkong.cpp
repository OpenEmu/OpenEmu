#include "tiles_generic.h"
#include "z80_intf.h"
#include "s2650_intf.h"
#include "samples.h"
#include "eeprom.h"
#include "bitswap.h"
#include "8257dma.h"
#include "i8039.h"
#include "dac.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv2650ROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvColPROM;
static UINT8 *DrvMapROM;
static UINT8 *DrvZ80RAM;
static UINT8 *Drv2650RAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvVidRAM;
static INT32 *DrvRevMap;

static UINT32 *DrvPalette;
static UINT8  DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *gfx_bank;
static UINT8 *sprite_bank;
static UINT8 *palette_bank;
static UINT8 *grid_enable;
static UINT8 *grid_color;
static UINT8 *flipscreen;
static UINT8 *nmi_mask;

static UINT8  DrvJoy1[8];
static UINT8  DrvJoy2[8];
static UINT8  DrvJoy3[8];
static UINT8  DrvDips[2];
static UINT8  DrvInputs[3];
static UINT8  DrvReset;

static INT32 vblank;
static void (*DrvPaletteUpdate)();

static INT32 s2650_protection = 0;
static int dkongjr_walk = 0;
static int page = 0,mcustatus;
static int p[8] = { 255,255,255,255,255,255,255,255 };
static int t[2] = { 1,1 };

static struct BurnInputInfo DkongInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 7,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
};

STDINPUTINFO(Dkong)

static struct BurnInputInfo Dkong3InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 coin"},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
};

STDINPUTINFO(Dkong3)

static struct BurnInputInfo RadarscpInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 7,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 start"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
};

STDINPUTINFO(Radarscp)

static struct BurnInputInfo PestplceInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 7,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
};

STDINPUTINFO(Pestplce)

static struct BurnInputInfo HerodkInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 7,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
};

STDINPUTINFO(Herodk)

static struct BurnDIPInfo DkongDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x80, NULL			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x0e, 0x01, 0x03, 0x00, "3"			},
	{0x0e, 0x01, 0x03, 0x01, "4"			},
	{0x0e, 0x01, 0x03, 0x02, "5"			},
	{0x0e, 0x01, 0x03, 0x03, "6"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x0e, 0x01, 0x0c, 0x00, "7000"			},
	{0x0e, 0x01, 0x0c, 0x04, "10000"		},
	{0x0e, 0x01, 0x0c, 0x08, "15000"		},
	{0x0e, 0x01, 0x0c, 0x0c, "20000"		},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x0e, 0x01, 0x70, 0x70, "5 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x50, "4 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x30, "3 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x10, "2 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x00, "1 Coin  1 Credits"	},
	{0x0e, 0x01, 0x70, 0x20, "1 Coin  2 Credits"	},
	{0x0e, 0x01, 0x70, 0x40, "1 Coin  3 Credits"	},
	{0x0e, 0x01, 0x70, 0x60, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x0e, 0x01, 0x80, 0x80, "Upright"		},
	{0x0e, 0x01, 0x80, 0x00, "Cocktail"		},
};

STDDIPINFO(Dkong)

static struct BurnDIPInfo DkongfDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x80, NULL			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x0e, 0x01, 0x03, 0x00, "3"			},
	{0x0e, 0x01, 0x03, 0x01, "4"			},
	{0x0e, 0x01, 0x03, 0x02, "5"			},
	{0x0e, 0x01, 0x03, 0x03, "6"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x0e, 0x01, 0x0c, 0x00, "7000"			},
	{0x0e, 0x01, 0x0c, 0x04, "10000"		},
	{0x0e, 0x01, 0x0c, 0x08, "15000"		},
	{0x0e, 0x01, 0x0c, 0x0c, "20000"		},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x0e, 0x01, 0x70, 0x70, "Free Play"		},
	{0x0e, 0x01, 0x70, 0x50, "4 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x30, "3 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x10, "2 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x00, "1 Coin  1 Credits"	},
	{0x0e, 0x01, 0x70, 0x20, "1 Coin  2 Credits"	},
	{0x0e, 0x01, 0x70, 0x40, "1 Coin  3 Credits"	},
	{0x0e, 0x01, 0x70, 0x60, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x0e, 0x01, 0x80, 0x80, "Upright"		},
	{0x0e, 0x01, 0x80, 0x00, "Cocktail"		},
};

STDDIPINFO(Dkongf)

static struct BurnDIPInfo Dkong3bDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x00, NULL			},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x0e, 0x01, 0x07, 0x02, "3 Coins 1 Credit"	},
	{0x0e, 0x01, 0x07, 0x04, "2 Coins 1 Credit"	},
	{0x0e, 0x01, 0x07, 0x00, "1 Coin  1 Credits"	},
	{0x0e, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x0e, 0x01, 0x07, 0x01, "1 Coin  3 Credits"	},
	{0x0e, 0x01, 0x07, 0x03, "1 Coin  4 Credits"	},
	{0x0e, 0x01, 0x07, 0x05, "1 Coin  5 Credits"	},
	{0x0e, 0x01, 0x07, 0x07, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x0e, 0x01, 0x08, 0x00, "Upright"		},
	{0x0e, 0x01, 0x08, 0x08, "Cocktail"		},
};

STDDIPINFO(Dkong3b)

static struct BurnDIPInfo Dkong3DIPList[]=
{
	{0x0f, 0xff, 0xff, 0x00, NULL			},
	{0x10, 0xff, 0xff, 0x00, NULL			},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x0f, 0x01, 0x07, 0x02, "3 Coins 1 Credits"	},
	{0x0f, 0x01, 0x07, 0x04, "2 Coins 1 Credits"	},
	{0x0f, 0x01, 0x07, 0x00, "1 Coin  1 Credits"	},
	{0x0f, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x0f, 0x01, 0x07, 0x01, "1 Coin  3 Credits"	},
	{0x0f, 0x01, 0x07, 0x03, "1 Coin  4 Credits"	},
	{0x0f, 0x01, 0x07, 0x05, "1 Coin  5 Credits"	},
	{0x0f, 0x01, 0x07, 0x07, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0f, 0x01, 0x40, 0x00, "Off"			},
	{0x0f, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x0f, 0x01, 0x80, 0x00, "Upright"		},
	{0x0f, 0x01, 0x80, 0x80, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x10, 0x01, 0x03, 0x00, "3"			},
	{0x10, 0x01, 0x03, 0x01, "4"			},
	{0x10, 0x01, 0x03, 0x02, "5"			},
	{0x10, 0x01, 0x03, 0x03, "6"			},
	
	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x10, 0x01, 0x0c, 0x00, "30000"		},
	{0x10, 0x01, 0x0c, 0x04, "40000"		},
	{0x10, 0x01, 0x0c, 0x08, "50000"		},
	{0x10, 0x01, 0x0c, 0x0c, "None"			},

	{0   , 0xfe, 0   ,    4, "Additional Bonus"	},
	{0x10, 0x01, 0x30, 0x00, "30000"		},
	{0x10, 0x01, 0x30, 0x10, "40000"		},
	{0x10, 0x01, 0x30, 0x20, "50000"		},
	{0x10, 0x01, 0x30, 0x30, "None"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x10, 0x01, 0xc0, 0x00, "Easy"			},
	{0x10, 0x01, 0xc0, 0x40, "Medium"		},
	{0x10, 0x01, 0xc0, 0x80, "Hard"			},
	{0x10, 0x01, 0xc0, 0xc0, "Hardest"		},
};

STDDIPINFO(Dkong3)

static struct BurnDIPInfo RadarscpDIPList[]=
{
	{0x0a, 0xff, 0xff, 0x80, NULL			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x0a, 0x01, 0x03, 0x00, "3"			},
	{0x0a, 0x01, 0x03, 0x01, "4"			},
	{0x0a, 0x01, 0x03, 0x02, "5"			},
	{0x0a, 0x01, 0x03, 0x03, "6"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x0a, 0x01, 0x0c, 0x00, "7000"			},
	{0x0a, 0x01, 0x0c, 0x04, "10000"		},
	{0x0a, 0x01, 0x0c, 0x08, "15000"		},
	{0x0a, 0x01, 0x0c, 0x0c, "20000"		},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x0a, 0x01, 0x70, 0x70, "5 Coins 1 Credits"	},
	{0x0a, 0x01, 0x70, 0x50, "4 Coins 1 Credits"	},
	{0x0a, 0x01, 0x70, 0x30, "3 Coins 1 Credits"	},
	{0x0a, 0x01, 0x70, 0x10, "2 Coins 1 Credits"	},
	{0x0a, 0x01, 0x70, 0x00, "1 Coin  1 Credits"	},
	{0x0a, 0x01, 0x70, 0x20, "1 Coin  2 Credits"	},
	{0x0a, 0x01, 0x70, 0x40, "1 Coin  3 Credits"	},
	{0x0a, 0x01, 0x70, 0x60, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x0a, 0x01, 0x80, 0x80, "Upright"		},
	{0x0a, 0x01, 0x80, 0x00, "Cocktail"		},
};

STDDIPINFO(Radarscp)

static struct BurnDIPInfo PestplceDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x20, NULL			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x0e, 0x01, 0x03, 0x00, "3"			},
	{0x0e, 0x01, 0x03, 0x01, "4"			},
	{0x0e, 0x01, 0x03, 0x02, "5"			},
	{0x0e, 0x01, 0x03, 0x03, "6"			},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x0e, 0x01, 0x1c, 0x08, "3 Coins 1 Credits"	},
	{0x0e, 0x01, 0x1c, 0x10, "2 Coins 1 Credits"	},
	{0x0e, 0x01, 0x1c, 0x00, "1 Coin  1 Credits"	},
	{0x0e, 0x01, 0x1c, 0x18, "1 Coin  2 Credits"	},
	{0x0e, 0x01, 0x1c, 0x04, "1 Coin  3 Credits"	},
	{0x0e, 0x01, 0x1c, 0x0c, "1 Coin  4 Credits"	},
	{0x0e, 0x01, 0x1c, 0x14, "1 Coin  5 Credits"	},
	{0x0e, 0x01, 0x1c, 0x1c, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "2 Players Game"	},
	{0x0e, 0x01, 0x20, 0x00, "1 Credit"		},
	{0x0e, 0x01, 0x20, 0x20, "2 Credits"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x0e, 0x01, 0xc0, 0x00, "20000"		},
	{0x0e, 0x01, 0xc0, 0x40, "30000"		},
	{0x0e, 0x01, 0xc0, 0x80, "40000"		},
	{0x0e, 0x01, 0xc0, 0xc0, "×"			},
};

STDDIPINFO(Pestplce)

static struct BurnDIPInfo HerbiedkDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x80, NULL			},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x0e, 0x01, 0x70, 0x70, "5 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x50, "4 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x30, "3 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x10, "2 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x00, "1 Coin  1 Credits"	},
	{0x0e, 0x01, 0x70, 0x20, "1 Coin  2 Credits"	},
	{0x0e, 0x01, 0x70, 0x40, "1 Coin  3 Credits"	},
	{0x0e, 0x01, 0x70, 0x60, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x0e, 0x01, 0x80, 0x80, "Upright"		},
	{0x0e, 0x01, 0x80, 0x00, "Cocktail"		},
};

STDDIPINFO(Herbiedk)

static struct BurnDIPInfo HunchbkdDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x80, NULL			},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x0e, 0x01, 0x02, 0x00, "3"			},
	{0x0e, 0x01, 0x02, 0x02, "5"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x0e, 0x01, 0x0c, 0x00, "10000"		},
	{0x0e, 0x01, 0x0c, 0x04, "20000"		},
	{0x0e, 0x01, 0x0c, 0x08, "40000"		},
	{0x0e, 0x01, 0x0c, 0x0c, "80000"		},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x0e, 0x01, 0x70, 0x70, "5 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x50, "4 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x30, "3 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x10, "2 Coins 1 Credits"	},
	{0x0e, 0x01, 0x70, 0x00, "1 Coin  1 Credits"	},
	{0x0e, 0x01, 0x70, 0x20, "1 Coin  2 Credits"	},
	{0x0e, 0x01, 0x70, 0x40, "1 Coin  3 Credits"	},
	{0x0e, 0x01, 0x70, 0x60, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x0e, 0x01, 0x80, 0x80, "Upright"		},
	{0x0e, 0x01, 0x80, 0x00, "Cocktail"		},
};

STDDIPINFO(Hunchbkd)

static struct BurnDIPInfo HerodkDIPList[]=
{
	{0x10, 0xff, 0xff, 0x81, NULL			},

	{0   , 0xfe, 0   ,    0, "Lives"		},
	{0x10, 0x01, 0x02, 0x00, "3"			},
	{0x10, 0x01, 0x02, 0x02, "5"			},

	{0   , 0xfe, 0   ,    2, "Difficulty?"		},
	{0x10, 0x01, 0x0c, 0x00, "0"			},
	{0x10, 0x01, 0x0c, 0x04, "1"			},
	{0x10, 0x01, 0x0c, 0x08, "2"			},
	{0x10, 0x01, 0x0c, 0x0c, "3"			},

	{0   , 0xfe, 0   ,    4, "Coinage"		},
	{0x10, 0x01, 0x70, 0x70, "5 Coins 1 Credits"	},
	{0x10, 0x01, 0x70, 0x50, "4 Coins 1 Credits"	},
	{0x10, 0x01, 0x70, 0x30, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0x70, 0x10, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0x70, 0x00, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0x70, 0x20, "1 Coin  2 Credits"	},
	{0x10, 0x01, 0x70, 0x40, "1 Coin  3 Credits"	},
	{0x10, 0x01, 0x70, 0x60, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    8, "Cabinet"		},
	{0x10, 0x01, 0x80, 0x80, "Upright"		},
	{0x10, 0x01, 0x80, 0x00, "Cocktail"		},
};

STDDIPINFO(Herodk)

static struct BurnDIPInfo DraktonDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x1d, NULL		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x0e, 0x01, 0x01, 0x00, "Off"		},
	{0x0e, 0x01, 0x01, 0x01, "On"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x0e, 0x01, 0x06, 0x00, "3"		},
	{0x0e, 0x01, 0x06, 0x02, "4"		},
	{0x0e, 0x01, 0x06, 0x04, "5"		},
	{0x0e, 0x01, 0x06, 0x06, "6"		},

	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x0e, 0x01, 0x08, 0x00, "Easy"		},
	{0x0e, 0x01, 0x08, 0x08, "Normal"		},

	{0   , 0xfe, 0   ,    8, "Bonus Life"		},
	{0x0e, 0x01, 0x70, 0x00, "10000"		},
	{0x0e, 0x01, 0x70, 0x10, "20000"		},
	{0x0e, 0x01, 0x70, 0x20, "30000"		},
	{0x0e, 0x01, 0x70, 0x30, "40000"		},
	{0x0e, 0x01, 0x70, 0x40, "50000"		},
	{0x0e, 0x01, 0x70, 0x50, "60000"		},
	{0x0e, 0x01, 0x70, 0x60, "70000"		},
	{0x0e, 0x01, 0x70, 0x70, "80000"		},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x0e, 0x01, 0x80, 0x00, "Upright"		},
	{0x0e, 0x01, 0x80, 0x80, "Cocktail"		},
};

STDDIPINFO(Drakton)


static void dkong_sh1_write(INT32 offset, UINT8 data)
{
	static INT32 state[8];
	static INT32 count = 0;
	INT32 sample_order[7] = {1,2,1,2,0,1,0};

	if (state[offset] != data)
	{
		if (data) {
			if (offset) {
				BurnSamplePlay(offset+2);
			} else {
				BurnSamplePlay(sample_order[count]);
				count++;
				if (count == 7) count = 0;
			}
		}

		state[offset] = data;
	}
}

void __fastcall dkong_main_write(UINT16 address, UINT8 data)
{
	if ((address & 0xfff0) == 0x7800) {
		i8257Write(address,data);
		return;
	}

	switch (address)
	{
		case 0x7c00:	// AM_LATCH8_WRITE("ls175.3d")
			*soundlatch = data ^ 0x0f;
		return;

		case 0x7c80:
			*gfx_bank = data & 1; // inverted for dkong3
		return;

#if 0
		case 0x7d00:
		case 0x7d01:
		case 0x7d02:
		case 0x7d03:
		case 0x7d04:
		case 0x7d05:
		case 0x7d06:
		case 0x7d07:	// AM_DEVWRITE("ls259.6h", latch8_bit0_w)     		/* Sound signals */
		return;
#else
		case 0x7d00:
		case 0x7d01:
		case 0x7d02:
			dkong_sh1_write(address & 3, data);
		return;

		case 0x7d03:
			p[2] = (p[2] & ~0x20) | ((~data & 1) << 5);
		return;

		case 0x7d04:
			t[1] = ~data & 1;
		return;

		case 0x7d05:
			t[0] = ~data & 1;
		return;
#endif

		case 0x7d80:
			I8039SetIrqState(data ? 1 : 0);
		return;

		case 0x7d82:
			*flipscreen = ~data & 0x01;
		return;

		case 0x7d83:
			*sprite_bank = data & 0x01;
		return;

		case 0x7d84:
			*nmi_mask = data & 0x01;
		return;

		case 0x7d85:
			i8257_drq_write(0, data & 0x01);
			i8257_drq_write(1, data & 0x01);
			i8257_do_transfer(data & 0x01);
		return;

		case 0x7d86:
		case 0x7d87:
			if (data & 0x01) {
				*palette_bank |=  (1 << (address & 1));
			} else {
				*palette_bank &= ~(1 << (address & 1));
			}
		return;
	}
}

UINT8 __fastcall dkong_main_read(UINT16 address)
{
	if ((address & 0xfff0) == 0x7800) {
		return i8257Read(address);
	}

	switch (address)
	{
		case 0x7c00:
			return DrvInputs[0];

		case 0x7c80:
			return DrvInputs[1];

		case 0x7d00:
			{
				UINT8 ret = DrvInputs[2] & 0xbf;
				if (ret & 0x10) ret = (ret & ~0x10) | 0x80;
				ret |= mcustatus << 6;
				return ret;
			}
			return 0;

		case 0x7d80:
			return DrvDips[0];
	}

	return 0;
}

static inline void dkongjr_climb_write(UINT8 data)
{
	static INT32 climb = 0;
	static INT32 count;
	INT32 sample_order[7] = {1,2,1,2,0,1,0};

	if (climb != data)
	{
		if (data && dkongjr_walk == 0)
		{
			BurnSamplePlay(sample_order[count]+3);
			count++;
			if (count == 7) count = 0;
		}
		else if (data && dkongjr_walk == 1)
		{
			BurnSamplePlay(sample_order[count]+8);
			count++;
			if (count == 7) count = 0;
		}
		climb = data;
	}
}

static inline void dkongjr_sample_play(INT32 offs, UINT8 data, INT32 stop) // jump, land[s], roar, snapjaw[s], death[s], drop
{
	static INT32 select[8];
	UINT8 sample[8] = { 0, 1, 2, 11, 6, 7 };

	if (select[offs] != data)
	{
		if (stop) {
			if (data) BurnSampleStop(7);
			BurnSamplePlay(sample[offs]);
		} else {
			if (data) BurnSamplePlay(sample[offs]);
		}

		select[offs] = data;
	}
}

void __fastcall dkongjr_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x7c00:
			*soundlatch = data;
		return;

		case 0x7c81:
			p[2] = (p[2] & ~0x40) | ((~data & 1) << 6);
		return;

		case 0x7d00:
			dkongjr_climb_write(data);
		return;

		case 0x7d01:
			dkongjr_sample_play(0, data, 0);
		return;

		case 0x7d02:
			dkongjr_sample_play(1, data, 1);
		return;

		case 0x7d03:
			dkongjr_sample_play(2, data, 0);
		return;

		case 0x7d06:
			dkongjr_sample_play(3, data, 1);
		return;

		case 0x7d07:
			dkongjr_walk = data;
		return;

		case 0x7d80:
			dkongjr_sample_play(4, data, 1);
		return;

		case 0x7d81:
			dkongjr_sample_play(5, data, 0);
		return;
	}

	dkong_main_write(address, data);
}

void __fastcall radarscp_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x7c80:
			*grid_color = data; // ?
		return;

		case 0x7d81:
			*grid_enable = data & 0x01;
		return;
	}

	dkong_main_write(address, data);
}

void __fastcall dkong3_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x7c00:
			// latch1
		return;

		case 0x7c80:
			// latch2
		return;

		case 0x7d00:
			// latch3
		return;

		case 0x7d80:
			// dkong_2a03_reset_w
		return;

		case 0x7e80:
			// coin_counter
		return;

		case 0x7e81:
			*gfx_bank = data & 0x01;
		return;

		case 0x7e82:
			*flipscreen = ~data & 0x01;
		return;

		case 0x7e83:
			*sprite_bank = data & 0x01;
		return;

		case 0x7e84:
			*nmi_mask = data & 0x01;
		return;

		case 0x7e85: // dma
		return;

		case 0x7e86:
		case 0x7e87:
			if (data & 0x01) {
				*palette_bank |=  (1 << (address & 1));
			} else {
				*palette_bank &= ~(1 << (address & 1));
			}
		return;
	}
}

UINT8 __fastcall dkong3_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x7c00:
			return DrvInputs[0];

		case 0x7c80:
			return DrvInputs[1];

		case 0x7d00:
			return DrvDips[0];

		case 0x7d80:
			return DrvDips[1];
	}

	return 0;
}


static INT32 braze_bank = 0;

static void braze_bankswitch(INT32 data)
{
	braze_bank = (data & 0x01) * 0x8000;

	ZetMapArea(0x0000, 0x5fff, 0, DrvZ80ROM + braze_bank);
	ZetMapArea(0x0000, 0x5fff, 2, DrvZ80ROM + braze_bank);

	// work-around for eeprom reading
	ZetMapArea(0x8000, 0xc7ff, 0, DrvZ80ROM + braze_bank);
	ZetMapArea(0xc900, 0xffff, 0, DrvZ80ROM + 0x4900 + braze_bank);

	ZetMapArea(0x8000, 0xffff, 2, DrvZ80ROM + braze_bank);
}

void __fastcall braze_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xc800:
			EEPROMWrite((data & 0x02), (data & 0x04), (data & 0x01));
		return;

		case 0xe000:
			braze_bankswitch(data);
		return;
	}

	dkong_main_write(address, data);
}

UINT8 __fastcall braze_main_read(UINT16 address)
{
	// work-around for eeprom reading
	if ((address & 0xff00) == 0xc800)
	{
		if (address == 0xc800) return (EEPROMRead() & 1);

		return DrvZ80ROM[braze_bank + (address & 0x7fff)];
	}

	return dkong_main_read(address);
}

static const eeprom_interface braze_eeprom_intf =
{
	7,		// address bits
	8,		// data bits
	"*110",		// read command
	"*101",		// write command
	0,		// erase command
	"*10000xxxxx",	// lock command
	"*10011xxxxx",	// unlock command
	0,0
};

static void braze_decrypt_rom()
{
	UINT8 *tmp = BurnMalloc(0x10000);

	for (INT32 i = 0; i < 0x10000; i++) {
		tmp[(BITSWAP08(i >> 8, 7,2,3,1,0,6,4,5) << 8) | (i & 0xff)] = BITSWAP08(DrvZ80ROM[i], 1,4,5,7,6,0,3,2);
	}

	memcpy (DrvZ80ROM, tmp, 0x10000);

	BurnFree (tmp);
}



static INT32 hunch_prot_ctr = 0;
static UINT8 hunchloopback = 0;
static UINT8 main_fo = 0;

static void s2650_main_write(UINT16 address, UINT8 data)
{
//	bprintf (0, _T("mw %4.4x, %2.2x\n"), address, data);

	if (address >= 0x2000) { // mirrors
		s2650_write(address & 0x1fff, data);
		return;
	}

	if ((address & 0xff80) == 0x1f00) {
		DrvSprRAM[0x400 + (address & 0x3ff)] = data;
		return;
	}

	if ((address & 0xfff0) == 0x1f80) {
		i8257Write(address,data);
		return;
	}

	switch (address)
	{
		case 0x1400:
			*soundlatch = data ^ 0x0f; // ?
		return;

		case 0x1500:
		case 0x1501:
		case 0x1502:
		case 0x1503:
		case 0x1504:
		case 0x1505:
		case 0x1506:
		case 0x1507:
		return;		// latch8_bit0_w

		case 0x1580:
			I8039SetIrqState(data ? 1 : 0);
		return;

		case 0x1582:
			*flipscreen = ~data & 0x01;
		return;

		case 0x1583:
			*sprite_bank = data & 0x01;
		return;

		case 0x1584:
		return; // nop

		case 0x1585:
			i8257_drq_write(0, data & 0x01);
			i8257_drq_write(1, data & 0x01);
			i8257_do_transfer(data & 0x01);
		return;

		case 0x1586:
		case 0x1587:
			if (data & 0x01) {
				*palette_bank |=  (1 << (address & 1));
			} else {
				*palette_bank &= ~(1 << (address & 1));
			}
		return;
	}
}

static UINT8 s2650_main_read(UINT16 address)
{
//	bprintf (0, _T("mr %4.4x\n"), address);

	if (address >= 0x2000) { // mirrors
		return s2650_read(address & 0x1fff);
	}

	if ((address & 0xff80) == 0x1f00) {
		return DrvSprRAM[0x400 + (address & 0x3ff)];
	}

	if ((address & 0xfff0) == 0x1f80) {
		return i8257Read(address);
	}

	if ((address & 0xfe80) == 0x1400) address &= ~0x7f; // mirrored

	switch (address)
	{
		case 0x1400:
			return DrvInputs[0];

		case 0x1480:
			return DrvInputs[1];

		case 0x1500:
			{
				UINT8 ret = DrvInputs[2] & 0xbf;
				if (ret & 0x10) ret = (ret & ~0x10) | 0x80;
				ret |= mcustatus << 6;
				return ret;
			}
			return 0;

		case 0x1580:
			return DrvDips[0];
	}

	return 0;
}

static void s2650_main_write_port(UINT16 port, UINT8 data)
{
//	bprintf (0, _T("pw %4.4x, %2.2x\n"), port, data);

	switch (port)
	{
		case 0x101:
			hunchloopback = data;
		return;

		case 0x0103:
		{
			main_fo = data;

			if (data) hunchloopback = 0xfb;
		}
		return;
	}
}

static UINT8 s2650_main_read_port(UINT16 port)
{
//	bprintf (0, _T("pr %4.4x\n"), port);

	switch (port)
	{
		case 0x00:
			switch (s2650_protection)
			{
				case 0x02:
			        	if (main_fo)
			        		return hunchloopback;
			        	else
			        		return hunchloopback--;

				default:
			        	if (!main_fo)
			        		return hunchloopback;
			        	else
			        		return hunchloopback--;
			}

		case 0x01:
			switch (s2650_protection)
			{
				case 0x01:
			        	if (hunchloopback & 0x80)
			        		return hunch_prot_ctr;
			        	else
			        		return ++hunch_prot_ctr;

				case 0x02:
					return hunchloopback--;
			}

		case S2650_SENSE_PORT:
			return vblank^0x80;
	}

	return 0;
}

UINT8 __fastcall i8039_sound_read(UINT32 address)
{
	return DrvSndROM0[address & 0x0fff];
}

UINT8 __fastcall i8039_sound_read_port(UINT32 port)
{
	if (port < 0x100) {
		if ((page & 0x40) && port == 0x20) return *soundlatch;

		return DrvSndROM0[0x1000 + (page & 7) * 0x100 + (port & 0xff)];
	}

	switch (port)
	{
		case I8039_p1:
			return p[1];

		case I8039_p2:
			return p[2];

		case I8039_t0:
			return t[0];

		case I8039_t1:
			return t[1];
	}
	
	return 0;
}

static INT32 DkongDACSync()
{
	return (INT32)(float)(nBurnSoundLen * (I8039TotalCycles() / ((6000000.000 / 15) / (nBurnFPS / 100.000))));
}

#if 1
#include <math.h>

static double envelope,tt;
static INT32 decay;

static void dkong_sh_p1_write(UINT8 data)
{
	envelope=exp(-tt);
	DACWrite(0,(INT32)(data*envelope));
	if (decay) tt+=0.001;
	else tt=0;
}
#endif

static void __fastcall i8039_sound_write_port(UINT32 port, UINT8 data)
{
//bprintf (0, _T("i8039 wp %x %x\n"), port,data);

	switch (port)
	{
		case I8039_p1:
			dkong_sh_p1_write(data);
		return;

		case I8039_p2:
			decay = !(data & 0x80);
			page = (data & 0x47);
			mcustatus = ((~data & 0x10) >> 4);
		return;
	}
}

static UINT8 dma_latch = 0;

static void p8257ControlWrite(UINT16,UINT8 data)
{
	dma_latch = data;
}

static UINT8 p8257ControlRead(UINT16)
{
	return dma_latch;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	I8039Reset();

	BurnSampleReset();
	DACReset();

	i8257Reset();

	EEPROMReset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv2650ROM		= Next;
	DrvZ80ROM		= Next; Next += 0x020000;
	DrvSndROM0		= Next; Next += 0x002000;
	DrvSndROM1		= Next; Next += 0x002000;

	DrvGfxROM0		= Next; Next += 0x008000;
	DrvGfxROM1		= Next; Next += 0x010000;
	DrvGfxROM2		= Next; Next += 0x000800;

	DrvColPROM		= Next; Next += 0x000300;
	DrvMapROM		= Next; Next += 0x000200; // for s2650 sets

	DrvRevMap		= (INT32*)Next; Next += 0x000200 * sizeof(INT32);

	DrvPalette		= (UINT32*)Next; Next += 0x0102 * sizeof(UINT32);

	AllRam			= Next;

	Drv2650RAM		= Next;
	DrvZ80RAM		= Next; Next += 0x001000;
	DrvSprRAM		= Next; Next += 0x000b00;
	DrvVidRAM		= Next; Next += 0x000400;

	soundlatch		= Next; Next += 0x000001;
	gfx_bank		= Next; Next += 0x000001;
	sprite_bank		= Next; Next += 0x000001;
	palette_bank		= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;
	nmi_mask		= Next; Next += 0x000001;

	grid_color		= Next; Next += 0x000001;
	grid_enable		= Next; Next += 0x000001;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static void dkongPaletteInit()
{
	for (INT32 i = 0; i < 256; i++)
	{
		INT32 bit0 = (DrvColPROM[i+256] >> 1) & 1;
		INT32 bit1 = (DrvColPROM[i+256] >> 2) & 1;
		INT32 bit2 = (DrvColPROM[i+256] >> 3) & 1;
		INT32 r = 255 - (0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		bit0 = (DrvColPROM[i+0] >> 2) & 1;
		bit1 = (DrvColPROM[i+0] >> 3) & 1;
		bit2 = (DrvColPROM[i+256] >> 0) & 1;
		INT32 g = 255 - (0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2);

		bit0 = (DrvColPROM[i+0] >> 0) & 1;
		bit1 = (DrvColPROM[i+0] >> 1) & 1;
		INT32 b = 255 - (0x55 * bit0 + 0xaa * bit1);

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void dkong3PaletteInit()
{
	for (INT32 i = 0; i < 256; i++)
	{
		INT32 bit0 = (DrvColPROM[i+0] >> 4) & 0x01;
		INT32 bit1 = (DrvColPROM[i+0] >> 5) & 0x01;
		INT32 bit2 = (DrvColPROM[i+0] >> 6) & 0x01;
		INT32 bit3 = (DrvColPROM[i+0] >> 7) & 0x01;
		INT32 r = 255 - (0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3);

		bit0 = (DrvColPROM[i+0] >> 0) & 0x01;
		bit1 = (DrvColPROM[i+0] >> 1) & 0x01;
		bit2 = (DrvColPROM[i+0] >> 2) & 0x01;
		bit3 = (DrvColPROM[i+0] >> 3) & 0x01;
		INT32 g = 255 - (0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3);

		bit0 = (DrvColPROM[i+256] >> 0) & 0x01;
		bit1 = (DrvColPROM[i+256] >> 1) & 0x01;
		bit2 = (DrvColPROM[i+256] >> 2) & 0x01;
		bit3 = (DrvColPROM[i+256] >> 3) & 0x01;
		INT32 b = 255 - (0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3);

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[2]  = { 0x2000/2*8, 0 };
	INT32 Plane1[2]  = { 0x4000/2*8, 0 };
	INT32 XOffs0[16] = { STEP8(0,1), STEP8((0x2000/4)*8,1) };
	INT32 XOffs1[16] = { STEP8(0,1), STEP8((0x4000/4)*8,1) };
	INT32 YOffs[16] = { STEP16(0,8) };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x4000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x2000);

	GfxDecode(0x0200, 2,  8,  8, Plane0, XOffs0, YOffs, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x4000);

	GfxDecode(0x0100, 2, 16, 16, Plane1, XOffs1, YOffs, 0x080, tmp, DrvGfxROM1);

	BurnFree(tmp);

	return 0;
}

static ior_in_functs dkong_dma_read_functions[4] = { NULL, p8257ControlRead, NULL, NULL };
static ior_out_functs dkong_dma_write_functions[4] = { p8257ControlWrite, NULL, NULL, NULL };

static INT32 DrvInit(INT32 (*pRomLoadCallback)(), void (*pPaletteUpdate)(), UINT32 map_flags)
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x5fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x5fff, 2, DrvZ80ROM);

	if (map_flags & 2) { // hack
		ZetMapArea(0x6000, 0x68ff, 0, DrvZ80RAM);
		ZetMapArea(0x6000, 0x68ff, 1, DrvZ80RAM);
		ZetMapArea(0x6000, 0x68ff, 2, DrvZ80RAM);
		ZetMapArea(0x6900, 0x73ff, 0, DrvSprRAM); // 900-a7f
		ZetMapArea(0x6900, 0x73ff, 1, DrvSprRAM);
		ZetMapArea(0x6900, 0x73ff, 2, DrvSprRAM);
	} else {
		ZetMapArea(0x6000, 0x6fff, 0, DrvZ80RAM);
		ZetMapArea(0x6000, 0x6fff, 1, DrvZ80RAM);
		ZetMapArea(0x6000, 0x6fff, 2, DrvZ80RAM);
		ZetMapArea(0x7000, 0x73ff, 0, DrvSprRAM);
		ZetMapArea(0x7000, 0x73ff, 1, DrvSprRAM);
		ZetMapArea(0x7000, 0x73ff, 2, DrvSprRAM);
	}

	ZetMapArea(0x7400, 0x77ff, 0, DrvVidRAM);
	ZetMapArea(0x7400, 0x77ff, 1, DrvVidRAM);
	ZetMapArea(0x7400, 0x77ff, 2, DrvVidRAM);

	if (map_flags & 1) {
		ZetMapArea(0x8000, 0xffff, 0, DrvZ80ROM + 0x8000);
		ZetMapArea(0x8000, 0xffff, 2, DrvZ80ROM + 0x8000);
	}

	ZetSetWriteHandler(dkong_main_write);
	ZetSetReadHandler(dkong_main_read);
	ZetMemEnd();
	ZetClose();

	I8039Init(NULL);
	I8039SetIOReadHandler(i8039_sound_read_port);
	I8039SetIOWriteHandler(i8039_sound_write_port);
	I8039SetProgramReadHandler(i8039_sound_read);
	I8039SetCPUOpReadHandler(i8039_sound_read);
	I8039SetCPUOpReadArgHandler(i8039_sound_read);

	DACInit(0, 0, 0, DkongDACSync);
	DACSetRoute(0, 0.75, BURN_SND_ROUTE_BOTH);

	BurnSampleInit(1);
	BurnSampleSetAllRoutesAllSamples(0.75, BURN_SND_ROUTE_BOTH);

	i8257Init();
	i8257Config(ZetReadByte, ZetWriteByte, ZetIdle, dkong_dma_read_functions, dkong_dma_write_functions);

	EEPROMInit(&braze_eeprom_intf);

	{
		if (pRomLoadCallback) {
			if (pRomLoadCallback()) return 1;
		}

		if (pPaletteUpdate) {
			DrvPaletteUpdate = pPaletteUpdate;

			DrvPaletteUpdate();
		}

		DrvGfxDecode();
	}

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();
	I8039Exit();

	BurnSampleExit();
	DACExit();

	EEPROMExit();

	BurnFree(AllMem);

	return 0;
}

static INT32 s2650DkongDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	s2650Open(0);
	s2650Reset();
	s2650Close();

	I8039Reset();

	BurnSampleReset();
	DACReset();

	i8257Reset();

	hunchloopback = 0;

	return 0;
}

static UINT8 hb_dma_read_byte(UINT16 offset)
{
	offset = ((DrvRevMap[(offset >> 10) & 0x1ff] << 7) & 0x7c00) | (offset & 0x3ff);

	return s2650_read(offset);
}

static void hb_dma_write_byte(UINT16 offset, UINT8 data)
{
	offset = ((DrvRevMap[(offset >> 10) & 0x1ff] << 7) & 0x7c00) | (offset & 0x3ff);

	s2650_write(offset, data);
}

static INT32 s2650_irq_callback(INT32)
{
	return 0x03;
}

static void s2650RevMapConvert()
{
	for (INT32 i = 0; i < 0x200; i++) {
		DrvRevMap[DrvMapROM[i]] = i;
	}
}

static INT32 s2650DkongInit(INT32 (*pRomLoadCallback)())
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (pRomLoadCallback) {
			if (pRomLoadCallback()) return 1;
		}

		DrvPaletteUpdate = dkongPaletteInit;

		s2650RevMapConvert();
		DrvGfxDecode();
	}

	s2650Init(1);
	s2650Open(0);
	s2650MapMemory(Drv2650ROM + 0x0000, 0x0000, 0x0fff, S2650_ROM);
	s2650MapMemory(Drv2650RAM + 0x0000, 0x1000, 0x13ff, S2650_RAM); // sprite ram (after dma)
	s2650MapMemory(DrvSprRAM  + 0x0000, 0x1600, 0x17ff, S2650_RAM);
	s2650MapMemory(DrvVidRAM  + 0x0000, 0x1800, 0x1bff, S2650_RAM);
	s2650MapMemory(DrvSprRAM  + 0x0400, 0x1c00, 0x1eff, S2650_RAM);
	s2650MapMemory(Drv2650ROM + 0x2000, 0x2000, 0x2fff, S2650_ROM);
	s2650MapMemory(Drv2650ROM + 0x4000, 0x4000, 0x4fff, S2650_ROM);
	s2650MapMemory(Drv2650ROM + 0x6000, 0x6000, 0x6fff, S2650_ROM);
	s2650SetIrqCallback(s2650_irq_callback);
	s2650SetWriteHandler(s2650_main_write);
	s2650SetReadHandler(s2650_main_read);
	s2650SetOutHandler(s2650_main_write_port);
	s2650SetInHandler(s2650_main_read_port);
	s2650Close();

	I8039Init(NULL);
	I8039SetIOReadHandler(i8039_sound_read_port);
	I8039SetIOWriteHandler(i8039_sound_write_port);
	I8039SetProgramReadHandler(i8039_sound_read);
	I8039SetCPUOpReadHandler(i8039_sound_read);
	I8039SetCPUOpReadArgHandler(i8039_sound_read);

	DACInit(0, 0, 0, DkongDACSync);
	DACSetRoute(0, 0.75, BURN_SND_ROUTE_BOTH);

	BurnSampleInit(1);
	BurnSampleSetAllRoutesAllSamples(0.75, BURN_SND_ROUTE_BOTH);

	i8257Init();
	i8257Config(hb_dma_read_byte, hb_dma_write_byte, /*s2650Idle*/NULL, dkong_dma_read_functions, dkong_dma_write_functions);

	GenericTilesInit();

	s2650DkongDoReset();

	return 0;
}

static INT32 s2650DkongExit()
{
	GenericTilesExit();

	s2650Exit();
	I8039Exit();

	BurnSampleExit();
	DACExit();

	BurnFree(AllMem);

	s2650_protection = 0;

	return 0;
}

static void draw_grid()
{
	DrvPalette[0x100] = BurnHighCol(0xff, 0, 0, 0); // red
	DrvPalette[0x101] = BurnHighCol(0, 0, 0xff, 0); // blue

	const UINT8 *table = DrvGfxROM2;
	INT32 x,y,counter;

	counter = 0x400; //flip_screen ? 0x000 : 0x400;

	x = 0;
	y = 16;

	while (y <= 240)
	{
		x = 4 * (table[counter] & 0x7f);

		if (x >= 0 && x <= 255)
		{
			if (table[counter] & 0x80)	/* star */
			{
				if (rand() & 1)	/* noise coming from sound board */
					pTransDraw[(y - 16) * nScreenWidth + x] = 0x100;
			}
			else if (*grid_enable)			/* radar */
				pTransDraw[(y - 16) * nScreenWidth + x] = 0x101;
		}

		counter++;

		if (x >= 4 * (table[counter] & 0x7f))
			y++;
	}
}

static void draw_sprites(UINT32 code_mask, UINT32 mask_bank, UINT32 shift_bits, UINT32 swap)
{
	INT32 bank = *sprite_bank * 0x200;
	INT32 yoffset = (swap) ? -15 : -16;

	for (INT32 offs = bank; offs < bank + 0x200; offs += 4)
	{
		if (DrvSprRAM[offs + 0])
		{
			INT32 sx    = DrvSprRAM[offs + 3] - 8;
			INT32 attr  = DrvSprRAM[offs + ((swap) ? 1 : 2)];
			INT32 code  = (DrvSprRAM[offs + ((swap) ? 2 : 1)] & code_mask) + ((attr & mask_bank) << shift_bits);
			INT32 sy    =(240 - DrvSprRAM[offs + 0] + 7) + yoffset;
			INT32 color = (attr & 0x0f) + (*palette_bank * 0x10);
			INT32 flipx = attr & 0x80;
			INT32 flipy = DrvSprRAM[offs + 1] & ((swap) ? 0x40 : 0x80);

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				}
			}

			// wraparound
			if (sx < 0) {
				if (flipy) {
					if (flipx) {
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx+256, sy, color, 2, 0, 0, DrvGfxROM1);
					} else {
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx+256, sy, color, 2, 0, 0, DrvGfxROM1);
					}
				} else {
					if (flipx) {
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx+256, sy, color, 2, 0, 0, DrvGfxROM1);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, code, sx+256, sy, color, 2, 0, 0, DrvGfxROM1);
					}
				}
			}
		}
	}
}

static void draw_layer()
{
	for (INT32 offs = (32 * 2); offs < (32 * 32) - (32 * 2); offs++)
	{
		INT32 sx = (offs & 0x1f) * 8;
		INT32 sy = (offs / 0x20) * 8;

		INT32 code  = DrvVidRAM[offs] + (*gfx_bank * 256);
		INT32 color =(DrvColPROM[0x200 + (offs & 0x1f) + ((offs / 0x80) * 0x20)] & 0x0f) + (*palette_bank * 0x10);

		Render8x8Tile(pTransDraw, code, sx, sy - 16, color, 2, 0, DrvGfxROM0);
	}
}

static INT32 dkongDraw()
{
	if (DrvRecalc) {
		DrvPaletteUpdate();
		DrvRecalc = 0;
	}

	draw_layer();
	draw_sprites(0x7f, 0x40, 1, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 radarscpDraw()
{
	if (DrvRecalc) {
		DrvPaletteUpdate();
		DrvRecalc = 0;
	}

	draw_layer();
	draw_grid();
	draw_sprites(0x7f, 0x40, 1, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 pestplceDraw()
{
	if (DrvRecalc) {
		DrvPaletteUpdate();
		DrvRecalc = 0;
	}

	draw_layer();
	draw_sprites(0xff, 0x00, 0, 1);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	I8039NewFrame();

	{
		memset (DrvInputs, 0, 3);

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	ZetOpen(0);

	for (INT32 i = 0; i < 10; i++) {
		ZetRun(3072000 / 60 / 10);

		I8039Run(6000000 / 15 / 60 / 10);
	}

	if (*nmi_mask) ZetNmi();
	ZetClose();

	if (pBurnSoundOut) {
		DACUpdate(pBurnSoundOut, nBurnSoundLen);
		BurnSampleRender(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		BurnDrvRedraw();
	}

	return 0;
}

static INT32 s2650DkongFrame()
{
	if (DrvReset) {
		s2650DkongDoReset();
	}

	I8039NewFrame();

	{
		memset (DrvInputs, 0, 3);

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	s2650Open(0);

	vblank = 0;

	for (INT32 i = 0; i < 32; i++)
	{
		s2650Run(3072000 / 2 / 60 / 32);

		I8039Run(6000000 / 15 / 60 / 32);

		if (i == 30) {
			vblank = 0x80;
	
			s2650_set_irq_line(0, 1);
			s2650Run(10);
			s2650_set_irq_line(0, 0);
		}
	}

	s2650Close();

	if (pBurnSoundOut) {
		DACUpdate(pBurnSoundOut, nBurnSoundLen);
		BurnSampleRender(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		BurnDrvRedraw();
	}

	return 0;
}



// Radar Scope

static struct BurnRomInfo radarscpRomDesc[] = {
	{ "trs2c5fc",	0x1000, 0x40949e0d, 1 }, //  0 maincpu
	{ "trs2c5gc",	0x1000, 0xafa8c49f, 1 }, //  1
	{ "trs2c5hc",	0x1000, 0x51b8263d, 1 }, //  2
	{ "trs2c5kc",	0x1000, 0x1f0101f7, 1 }, //  3

	{ "trs2s3i",	0x0800, 0x78034f14, 2 }, //  4 soundcpu

	{ "trs2v3gc",	0x0800, 0xf095330e, 3 }, //  5 gfx1
	{ "trs2v3hc",	0x0800, 0x15a316f0, 3 }, //  6

	{ "trs2v3dc",	0x0800, 0xe0bb0db9, 4 }, //  7 gfx2
	{ "trs2v3cc",	0x0800, 0x6c4e7dad, 4 }, //  8
	{ "trs2v3bc",	0x0800, 0x6fdd63f1, 4 }, //  9
	{ "trs2v3ac",	0x0800, 0xbbf62755, 4 }, // 10

	{ "rs2-x.xxx",	0x0100, 0x54609d61, 6 }, // 11 proms
	{ "rs2-c.xxx",	0x0100, 0x79a7d831, 6 }, // 12
	{ "rs2-v.1hc",	0x0100, 0x1b828315, 6 }, // 13

	{ "trs2v3ec",	0x0800, 0x0eca8d6b, 5 }, // 14 gfx3
};

STD_ROM_PICK(radarscp)
STD_ROM_FN(radarscp)

static INT32 radarscpRomLoad()
{
	if (BurnLoadRom(DrvZ80ROM  + 0x0000,  0, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x1000,  1, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x2000,  2, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x3000,  3, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0  + 0x0000,  4, 1)) return 1;
	memcpy (DrvSndROM0 + 0x0800, DrvSndROM0 + 0x0000, 0x0800); // re-load

	if (BurnLoadRom(DrvGfxROM0 + 0x0000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x1000,  6, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x0000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x1000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x2000,  9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x3000, 10, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x0000, 11, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0100, 12, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0200, 13, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x0000, 14, 1)) return 1;

	return 0;
}

static INT32 radarscpInit()
{
	INT32 ret = DrvInit(radarscpRomLoad, dkongPaletteInit, 0);

	if (ret == 0)
	{
		ZetOpen(0);
		ZetSetWriteHandler(radarscp_main_write);
		ZetClose();
	}

	return ret;
}

struct BurnDriver BurnDrvRadarscp = {
	"radarscp", NULL, NULL, NULL, "1980",
	"Radar Scope\0", NULL, "Nintendo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, radarscpRomInfo, radarscpRomName, NULL, NULL, RadarscpInputInfo, RadarscpDIPInfo,
	radarscpInit, DrvExit, DrvFrame, radarscpDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Radar Scope (TRS01)

static struct BurnRomInfo radarscp1RomDesc[] = {
	{ "trs01_5f",		0x1000, 0x40949e0d, 1 }, //  0 maincpu
	{ "trs01_5g",		0x1000, 0xafa8c49f, 1 }, //  1
	{ "trs01_5h",		0x1000, 0x51b8263d, 1 }, //  2
	{ "trs01_5k",		0x1000, 0x1f0101f7, 1 }, //  3

	{ "trs015aa.bin",	0x0800, 0x5166554c, 2 }, //  4 soundcpu

	{ "trs01v3f",		0x0800, 0xf095330e, 4 }, //  5 gfx1
	{ "trs01v3g",		0x0800, 0x15a316f0, 4 }, //  6

	{ "trs01v3d",		0x0800, 0xe0bb0db9, 5 }, //  7 gfx2
	{ "trs01v3c",		0x0800, 0x6c4e7dad, 5 }, //  8
	{ "trs01v3b",		0x0800, 0x6fdd63f1, 5 }, //  9
	{ "trs01v3a",		0x0800, 0xbbf62755, 5 }, // 10

	{ "trs01c2j.bin",	0x0100, 0x2a087c87, 8 }, // 11 proms
	{ "trs01c2k.bin",	0x0100, 0x650c5daf, 8 }, // 12
	{ "trs01c2l.bin",	0x0100, 0x23087910, 8 }, // 13

	{ "trs011ha.bin",	0x0800, 0xdbcc50c2, 6 }, // 14 gfx3

	{ "trs01e3k.bin",	0x0100, 0x6c6f989c, 7 }, // 15 gfx4

	{ "trs014ha.bin",	0x0800, 0xd1f1b48c, 3 }, // 16 m58819 speech

	{ "trs01v1d.bin",	0x0100, 0x1b828315, 8 }, // 17 unused proms
};

STD_ROM_PICK(radarscp1)
STD_ROM_FN(radarscp1)

static INT32 radarscp1RomLoad()
{
	if (radarscpRomLoad()) return 1;

	// load gfx4
	// load speech

	return 0;
}

static INT32 radarscp1Init()
{
	INT32 ret = DrvInit(radarscp1RomLoad, dkongPaletteInit, 0);

	if (ret == 0)
	{
		ZetOpen(0);
		ZetSetWriteHandler(radarscp_main_write);
		ZetClose();
	}

	return ret;
}

struct BurnDriver BurnDrvRadarscp1 = {
	"radarscp1", "radarscp", NULL, NULL, "1980",
	"Radar Scope (TRS01)\0", "No sound", "Nintendo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, radarscp1RomInfo, radarscp1RomName, NULL, NULL, RadarscpInputInfo, RadarscpDIPInfo,
	radarscp1Init, DrvExit, DrvFrame, radarscpDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong (US set 1)

static struct BurnRomInfo dkongRomDesc[] = {
	{ "c_5et_g.bin",	0x1000, 0xba70b88b, 1 }, //  0 maincpu
	{ "c_5ct_g.bin",	0x1000, 0x5ec461ec, 1 }, //  1
	{ "c_5bt_g.bin",	0x1000, 0x1c97d324, 1 }, //  2
	{ "c_5at_g.bin",	0x1000, 0xb9005ac0, 1 }, //  3

	{ "s_3i_b.bin",		0x0800, 0x45a4ed06, 2 }, //  4 soundcpu
	{ "s_3j_b.bin",		0x0800, 0x4743fe92, 2 }, //  5

	{ "v_5h_b.bin",		0x0800, 0x12c8c95d, 3 }, //  6 gfx1
	{ "v_3pt.bin",		0x0800, 0x15e9c5e9, 3 }, //  7

	{ "l_4m_b.bin",		0x0800, 0x59f8054d, 4 }, //  8 gfx2
	{ "l_4n_b.bin",		0x0800, 0x672e4714, 4 }, //  9
	{ "l_4r_b.bin",		0x0800, 0xfeaa59ee, 4 }, // 10
	{ "l_4s_b.bin",		0x0800, 0x20f2ef7e, 4 }, // 11

	{ "c-2k.bpr",		0x0100, 0xe273ede5, 5 }, // 12 proms
	{ "c-2j.bpr",		0x0100, 0xd6412358, 5 }, // 13
	{ "v-5e.bpr",		0x0100, 0xb869b8f5, 5 }, // 14
};

STD_ROM_PICK(dkong)
STD_ROM_FN(dkong)

static struct BurnSampleInfo DkongSampleDesc[] = {
#if !defined (ROM_VERIFY)
	{ "run01.wav",   SAMPLE_NOLOOP },
	{ "run02.wav",   SAMPLE_NOLOOP },
	{ "run03.wav",   SAMPLE_NOLOOP },
	{ "jump.wav",    SAMPLE_NOLOOP },
	{ "dkstomp.wav", SAMPLE_NOLOOP },
#endif
	{ "",            0             }
};

STD_SAMPLE_PICK(Dkong)
STD_SAMPLE_FN(Dkong)

static INT32 dkongRomLoad()
{
	if (BurnLoadRom(DrvZ80ROM  + 0x0000,  0, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x1000,  1, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x2000,  2, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x3000,  3, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0  + 0x0000,  4, 1)) return 1;
	memcpy (DrvSndROM0 + 0x0800, DrvSndROM0 + 0x0000, 0x0800); // re-load
	if (BurnLoadRom(DrvSndROM0  + 0x1000,  5, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x0000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x1000,  7, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x0000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x1000,  9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x2000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x3000, 11, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x0000, 12, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0100, 13, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0200, 14, 1)) return 1;

	return 0;
}

static INT32 dkongInit()
{
	return DrvInit(dkongRomLoad, dkongPaletteInit, 0);
}

struct BurnDriver BurnDrvDkong = {
	"dkong", NULL, NULL, "dkong", "1981",
	"Donkey Kong (US set 1)\0", NULL, "Nintendo of America", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongRomInfo, dkongRomName, DkongSampleInfo, DkongSampleName, DkongInputInfo, DkongDIPInfo,
	dkongInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong (US set 2)

static struct BurnRomInfo dkongoRomDesc[] = {
	{ "c_5f_b.bin",		0x1000, 0x424f2b11, 1 }, //  0 maincpu
	{ "c_5ct_g.bin",	0x1000, 0x5ec461ec, 1 }, //  1
	{ "c_5h_b.bin",		0x1000, 0x1d28895d, 1 }, //  2
	{ "tkg3c.5k",		0x1000, 0x553b89bb, 1 }, //  3

	{ "s_3i_b.bin",		0x0800, 0x45a4ed06, 2 }, //  4 soundcpu
	{ "s_3j_b.bin",		0x0800, 0x4743fe92, 2 }, //  5

	{ "v_5h_b.bin",		0x0800, 0x12c8c95d, 3 }, //  6 gfx1
	{ "v_3pt.bin",		0x0800, 0x15e9c5e9, 3 }, //  7

	{ "l_4m_b.bin",		0x0800, 0x59f8054d, 4 }, //  8 gfx2
	{ "l_4n_b.bin",		0x0800, 0x672e4714, 4 }, //  9
	{ "l_4r_b.bin",		0x0800, 0xfeaa59ee, 4 }, // 10
	{ "l_4s_b.bin",		0x0800, 0x20f2ef7e, 4 }, // 11

	{ "c-2k.bpr",		0x0100, 0xe273ede5, 5 }, // 12 proms
	{ "c-2j.bpr",		0x0100, 0xd6412358, 5 }, // 13
	{ "v-5e.bpr",		0x0100, 0xb869b8f5, 5 }, // 14
};

STD_ROM_PICK(dkongo)
STD_ROM_FN(dkongo)

struct BurnDriver BurnDrvDkongo = {
	"dkongo", "dkong", NULL, "dkong", "1981",
	"Donkey Kong (US set 2)\0", NULL, "Nintendo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongoRomInfo, dkongoRomName, DkongSampleInfo, DkongSampleName, DkongInputInfo, DkongDIPInfo,
	dkongInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong (Japan set 1)

static struct BurnRomInfo dkongjRomDesc[] = {
	{ "c_5f_b.bin",	0x1000, 0x424f2b11, 1 }, //  0 maincpu
	{ "5g.cpu",	0x1000, 0xd326599b, 1 }, //  1
	{ "5h.cpu",	0x1000, 0xff31ac89, 1 }, //  2
	{ "c_5k_b.bin",	0x1000, 0x394d6007, 1 }, //  3

	{ "s_3i_b.bin",	0x0800, 0x45a4ed06, 2 }, //  4 soundcpu
	{ "s_3j_b.bin",	0x0800, 0x4743fe92, 2 }, //  5

	{ "v_5h_b.bin",	0x0800, 0x12c8c95d, 3 }, //  6 gfx1
	{ "v_5k_b.bin",	0x0800, 0x3684f914, 3 }, //  7

	{ "l_4m_b.bin",	0x0800, 0x59f8054d, 4 }, //  8 gfx2
	{ "l_4n_b.bin",	0x0800, 0x672e4714, 4 }, //  9
	{ "l_4r_b.bin",	0x0800, 0xfeaa59ee, 4 }, // 10
	{ "l_4s_b.bin",	0x0800, 0x20f2ef7e, 4 }, // 11

	{ "c-2k.bpr",	0x0100, 0xe273ede5, 5 }, // 12 proms
	{ "c-2j.bpr",	0x0100, 0xd6412358, 5 }, // 13
	{ "v-5e.bpr",	0x0100, 0xb869b8f5, 5 }, // 14
};

STD_ROM_PICK(dkongj)
STD_ROM_FN(dkongj)

struct BurnDriver BurnDrvDkongj = {
	"dkongj", "dkong", NULL, "dkong", "1981",
	"Donkey Kong (Japan set 1)\0", NULL, "Nintendo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongjRomInfo, dkongjRomName, DkongSampleInfo, DkongSampleName, DkongInputInfo, DkongDIPInfo,
	dkongInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong (Japan set 2)

static struct BurnRomInfo dkongjoRomDesc[] = {
	{ "c_5f_b.bin",	0x1000, 0x424f2b11, 1 }, //  0 maincpu
	{ "c_5g_b.bin",	0x1000, 0x3b2a6635, 1 }, //  1
	{ "c_5h_b.bin",	0x1000, 0x1d28895d, 1 }, //  2
	{ "c_5k_b.bin",	0x1000, 0x394d6007, 1 }, //  3

	{ "s_3i_b.bin",	0x0800, 0x45a4ed06, 2 }, //  4 soundcpu
	{ "s_3j_b.bin",	0x0800, 0x4743fe92, 2 }, //  5

	{ "v_5h_b.bin",	0x0800, 0x12c8c95d, 3 }, //  6 gfx1
	{ "v_5k_b.bin",	0x0800, 0x3684f914, 3 }, //  7

	{ "l_4m_b.bin",	0x0800, 0x59f8054d, 4 }, //  8 gfx2
	{ "l_4n_b.bin",	0x0800, 0x672e4714, 4 }, //  9
	{ "l_4r_b.bin",	0x0800, 0xfeaa59ee, 4 }, // 10
	{ "l_4s_b.bin",	0x0800, 0x20f2ef7e, 4 }, // 11

	{ "c-2k.bpr",	0x0100, 0xe273ede5, 5 }, // 12 proms
	{ "c-2j.bpr",	0x0100, 0xd6412358, 5 }, // 13
	{ "v-5e.bpr",	0x0100, 0xb869b8f5, 5 }, // 14
};

STD_ROM_PICK(dkongjo)
STD_ROM_FN(dkongjo)

struct BurnDriver BurnDrvDkongjo = {
	"dkongjo", "dkong", NULL, "dkong", "1981",
	"Donkey Kong (Japan set 2)\0", NULL, "Nintendo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongjoRomInfo, dkongjoRomName, DkongSampleInfo, DkongSampleName, DkongInputInfo, DkongDIPInfo,
	dkongInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong (Japan set 3)

static struct BurnRomInfo dkongjo1RomDesc[] = {
	{ "c_5f_b.bin",	0x1000, 0x424f2b11, 1 }, //  0 maincpu
	{ "5g.cpu",	0x1000, 0xd326599b, 1 }, //  1
	{ "c_5h_b.bin",	0x1000, 0x1d28895d, 1 }, //  2
	{ "5k.bin",	0x1000, 0x7961599c, 1 }, //  3

	{ "s_3i_b.bin",	0x0800, 0x45a4ed06, 2 }, //  4 soundcpu
	{ "s_3j_b.bin",	0x0800, 0x4743fe92, 2 }, //  5

	{ "v_5h_b.bin",	0x0800, 0x12c8c95d, 3 }, //  6 gfx1
	{ "v_5k_b.bin",	0x0800, 0x3684f914, 3 }, //  7

	{ "l_4m_b.bin",	0x0800, 0x59f8054d, 4 }, //  8 gfx2
	{ "l_4n_b.bin",	0x0800, 0x672e4714, 4 }, //  9
	{ "l_4r_b.bin",	0x0800, 0xfeaa59ee, 4 }, // 10
	{ "l_4s_b.bin",	0x0800, 0x20f2ef7e, 4 }, // 11

	{ "c-2k.bpr",	0x0100, 0xe273ede5, 5 }, // 12 proms
	{ "c-2j.bpr",	0x0100, 0xd6412358, 5 }, // 13
	{ "v-5e.bpr",	0x0100, 0xb869b8f5, 5 }, // 14
};

STD_ROM_PICK(dkongjo1)
STD_ROM_FN(dkongjo1)

struct BurnDriver BurnDrvDkongjo1 = {
	"dkongjo1", "dkong", NULL, "dkong", "1981",
	"Donkey Kong (Japan set 3)\0", NULL, "Nintendo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongjo1RomInfo, dkongjo1RomName, DkongSampleInfo, DkongSampleName, DkongInputInfo, DkongDIPInfo,
	dkongInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong Foundry (hack)

static struct BurnRomInfo dkongfRomDesc[] = {
	{ "dk_f.5et",	0x1000, 0x00b7efaf, 1 }, //  0 maincpu
	{ "dk_f.5ct",	0x1000, 0x88af9b69, 1 }, //  1
	{ "dk_f.5bt",	0x1000, 0xde74ad91, 1 }, //  2
	{ "dk_f.5at",	0x1000, 0x6a6bd420, 1 }, //  3

	{ "s_3i_b.bin",	0x0800, 0x45a4ed06, 2 }, //  4 soundcpu
	{ "s_3j_b.bin",	0x0800, 0x4743fe92, 2 }, //  5

	{ "v_5h_b.bin",	0x0800, 0x12c8c95d, 3 }, //  6 gfx1
	{ "v_3pt.bin",	0x0800, 0x15e9c5e9, 3 }, //  7

	{ "l_4m_b.bin",	0x0800, 0x59f8054d, 4 }, //  8 gfx2
	{ "l_4n_b.bin",	0x0800, 0x672e4714, 4 }, //  9
	{ "l_4r_b.bin",	0x0800, 0xfeaa59ee, 4 }, // 10
	{ "l_4s_b.bin",	0x0800, 0x20f2ef7e, 4 }, // 11

	{ "c-2k.bpr",	0x0100, 0xe273ede5, 5 }, // 12 proms
	{ "c-2j.bpr",	0x0100, 0xd6412358, 5 }, // 13
	{ "v-5e.bpr",	0x0100, 0xb869b8f5, 5 }, // 14
};

STD_ROM_PICK(dkongf)
STD_ROM_FN(dkongf)

struct BurnDriver BurnDrvDkongf = {
	"dkongf", "dkong", NULL, "dkong", "2004",
	"Donkey Kong Foundry (hack)\0", NULL, "hack", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongfRomInfo, dkongfRomName, DkongSampleInfo, DkongSampleName, DkongInputInfo, DkongfDIPInfo,
	dkongInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong II - Jumpman Returns (V1.2) (hack)

static struct BurnRomInfo dkongxRomDesc[] = {
	{ "c_5et_g.bin",	0x01000, 0xba70b88b, 1 }, //  0 maincpu
	{ "c_5ct_g.bin",	0x01000, 0x5ec461ec, 1 }, //  1
	{ "c_5bt_g.bin",	0x01000, 0x1c97d324, 1 }, //  2
	{ "c_5at_g.bin",	0x01000, 0xb9005ac0, 1 }, //  3

	{ "d2k12.bin",		0x10000, 0x6e95ca0d, 2 }, //  4 braze

	{ "s_3i_b.bin",		0x00800, 0x45a4ed06, 3 }, //  5 soundcpu
	{ "s_3j_b.bin",		0x00800, 0x4743fe92, 3 }, //  6

	{ "v_5h_b.bin",		0x00800, 0x12c8c95d, 4 }, //  7 gfx1
	{ "v_3pt.bin",		0x00800, 0x15e9c5e9, 4 }, //  8

	{ "l_4m_b.bin",		0x00800, 0x59f8054d, 5 }, //  9 gfx2
	{ "l_4n_b.bin",		0x00800, 0x672e4714, 5 }, // 10
	{ "l_4r_b.bin",		0x00800, 0xfeaa59ee, 5 }, // 11
	{ "l_4s_b.bin",		0x00800, 0x20f2ef7e, 5 }, // 12

	{ "c-2k.bpr",		0x00100, 0xe273ede5, 6 }, // 13 proms
	{ "c-2j.bpr",		0x00100, 0xd6412358, 6 }, // 14
	{ "v-5e.bpr",		0x00100, 0xb869b8f5, 6 }, // 15
};

STD_ROM_PICK(dkongx)
STD_ROM_FN(dkongx)

static INT32 dkongxRomLoad()
{
	if (BurnLoadRom(DrvZ80ROM  + 0x00000,  4, 1)) return 1; // "braze"

	if (BurnLoadRom(DrvSndROM0 + 0x00000,  5, 1)) return 1;
	memcpy (DrvSndROM0 + 0x0800, DrvSndROM0 + 0x0000, 0x0800); // re-load
	if (BurnLoadRom(DrvSndROM0 + 0x01000,  6, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x0000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x1000,  8, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x0000,  9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x1000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x2000, 11, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x3000, 12, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x0000, 13, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0100, 14, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0200, 15, 1)) return 1;

	braze_decrypt_rom();

	return 0;
}

static INT32 dkongxInit()
{
	INT32 ret = DrvInit(dkongxRomLoad, dkongPaletteInit, 0);

	if (ret == 0)
	{
		ZetOpen(0);
		ZetSetWriteHandler(braze_main_write);
		ZetSetReadHandler(braze_main_read);
		braze_bankswitch(0);
		ZetClose();
	}

	return ret;
}

struct BurnDriver BurnDrvDkongx = {
	"dkongx", "dkong", NULL, "dkong", "2006",
	"Donkey Kong II - Jumpman Returns (V1.2) (hack)\0", NULL, "hack", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongxRomInfo, dkongxRomName, DkongSampleInfo, DkongSampleName, DkongInputInfo, NULL,
	dkongxInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong II - Jumpman Returns (V1.1) (hack)

static struct BurnRomInfo dkongx11RomDesc[] = {
	{ "c_5et_g.bin",	0x01000, 0xba70b88b, 1 }, //  0 maincpu
	{ "c_5ct_g.bin",	0x01000, 0x5ec461ec, 1 }, //  1
	{ "c_5bt_g.bin",	0x01000, 0x1c97d324, 1 }, //  2
	{ "c_5at_g.bin",	0x01000, 0xb9005ac0, 1 }, //  3

	{ "d2k11.bin",		0x10000, 0x2048fc42, 2 }, //  4 braze

	{ "s_3i_b.bin",		0x00800, 0x45a4ed06, 3 }, //  5 soundcpu
	{ "s_3j_b.bin",		0x00800, 0x4743fe92, 3 }, //  6

	{ "v_5h_b.bin",		0x00800, 0x12c8c95d, 4 }, //  7 gfx1
	{ "v_3pt.bin",		0x00800, 0x15e9c5e9, 4 }, //  8

	{ "l_4m_b.bin",		0x00800, 0x59f8054d, 5 }, //  9 gfx2
	{ "l_4n_b.bin",		0x00800, 0x672e4714, 5 }, // 10
	{ "l_4r_b.bin",		0x00800, 0xfeaa59ee, 5 }, // 11
	{ "l_4s_b.bin",		0x00800, 0x20f2ef7e, 5 }, // 12

	{ "c-2k.bpr",		0x00100, 0xe273ede5, 6 }, // 13 proms
	{ "c-2j.bpr",		0x00100, 0xd6412358, 6 }, // 14
	{ "v-5e.bpr",		0x00100, 0xb869b8f5, 6 }, // 15
};

STD_ROM_PICK(dkongx11)
STD_ROM_FN(dkongx11)

struct BurnDriver BurnDrvDkongx11 = {
	"dkongx11", "dkong", NULL, "dkong", "2006",
	"Donkey Kong II - Jumpman Returns (V1.1) (hack)\0", NULL, "hack", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongx11RomInfo, dkongx11RomName, DkongSampleInfo, DkongSampleName, DkongInputInfo, NULL,
	dkongxInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong Junior (US)

static struct BurnRomInfo dkongjrRomDesc[] = {
	{ "dkj.5b",	0x2000, 0xdea28158, 1 }, //  0 maincpu
	{ "dkj.5c",	0x2000, 0x6fb5faf6, 1 }, //  1
	{ "dkj.5e",	0x2000, 0xd042b6a8, 1 }, //  2

	{ "c_3h.bin",	0x1000, 0x715da5f8, 2 }, //  3 soundcpu

	{ "dkj.3n",	0x1000, 0x8d51aca9, 3 }, //  4 gfx1
	{ "dkj.3p",	0x1000, 0x4ef64ba5, 3 }, //  5

	{ "v_7c.bin",	0x0800, 0xdc7f4164, 4 }, //  6 gfx2
	{ "v_7d.bin",	0x0800, 0x0ce7dcf6, 4 }, //  7
	{ "v_7e.bin",	0x0800, 0x24d1ff17, 4 }, //  8
	{ "v_7f.bin",	0x0800, 0x0f8c083f, 4 }, //  9

	{ "c-2e.bpr",	0x0100, 0x463dc7ad, 5 }, // 10 proms
	{ "c-2f.bpr",	0x0100, 0x47ba0042, 5 }, // 11
	{ "v-2n.bpr",	0x0100, 0xdbf185bf, 5 }, // 12
};

STD_ROM_PICK(dkongjr)
STD_ROM_FN(dkongjr)

static struct BurnSampleInfo DkongjrSampleDesc[] = {
#if !defined (ROM_VERIFY)
	{ "jump.wav", SAMPLE_NOLOOP },
	{ "land.wav", SAMPLE_NOLOOP },
	{ "roar.wav", SAMPLE_NOLOOP },
	{ "climb0.wav", SAMPLE_NOLOOP },
	{ "climb1.wav", SAMPLE_NOLOOP },
	{ "climb2.wav", SAMPLE_NOLOOP },
	{ "death.wav", SAMPLE_NOLOOP },
	{ "drop.wav", SAMPLE_NOLOOP },
  	{ "walk0.wav", SAMPLE_NOLOOP },
 	{ "walk1.wav", SAMPLE_NOLOOP },
	{ "walk2.wav", SAMPLE_NOLOOP },
	{ "snapjaw.wav", SAMPLE_NOLOOP },
#endif
	{ "", 0 }
};

STD_SAMPLE_PICK(Dkongjr)
STD_SAMPLE_FN(Dkongjr)

static INT32 dkongjrRomLoad()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x2000);

	if (BurnLoadRom(tmp,  0, 1)) return 1;

	memcpy (DrvZ80ROM + 0x0000, tmp + 0x0000, 0x1000);
	memcpy (DrvZ80ROM + 0x3000, tmp + 0x1000, 0x1000);

	if (BurnLoadRom(tmp,  1, 1)) return 1;

	memcpy (DrvZ80ROM + 0x2000, tmp + 0x0000, 0x0800);
	memcpy (DrvZ80ROM + 0x4800, tmp + 0x0800, 0x0800);
	memcpy (DrvZ80ROM + 0x1000, tmp + 0x1000, 0x0800);
	memcpy (DrvZ80ROM + 0x5800, tmp + 0x1800, 0x0800);

	if (BurnLoadRom(tmp,  2, 1)) return 1;

	memcpy (DrvZ80ROM + 0x4000, tmp + 0x0000, 0x0800);
	memcpy (DrvZ80ROM + 0x2800, tmp + 0x0800, 0x0800);
	memcpy (DrvZ80ROM + 0x5000, tmp + 0x1000, 0x0800);
	memcpy (DrvZ80ROM + 0x1800, tmp + 0x1800, 0x0800);

	BurnFree (tmp);

	if (BurnLoadRom(DrvSndROM0  + 0x0000,  3, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x0000,  4, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x1000,  5, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x0000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x1000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x2000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x3000,  9, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x0000, 10, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0100, 11, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0200, 12, 1)) return 1;

	ZetOpen(0);
	ZetSetWriteHandler(dkongjr_main_write);
	ZetClose();

	return 0;
}

static INT32 dkongjrInit()
{
	return DrvInit(dkongjrRomLoad, dkongPaletteInit, 0);
}

struct BurnDriver BurnDrvDkongjr = {
	"dkongjr", NULL, NULL, "dkongjr", "1982",
	"Donkey Kong Junior (US)\0", NULL, "Nintendo of America", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongjrRomInfo, dkongjrRomName, DkongjrSampleInfo, DkongjrSampleName, DkongInputInfo, DkongDIPInfo,
	dkongjrInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong Jr. (Japan)

static struct BurnRomInfo dkongjrjRomDesc[] = {
	{ "c_5ba.bin",	0x2000, 0x50a015ce, 1 }, //  0 maincpu
	{ "c_5ca.bin",	0x2000, 0xc0a18f0d, 1 }, //  1
	{ "c_5ea.bin",	0x2000, 0xa81dd00c, 1 }, //  2

	{ "c_3h.bin",	0x1000, 0x715da5f8, 2 }, //  3 soundcpu

	{ "v_3na.bin",	0x1000, 0xa95c4c63, 3 }, //  4 gfx1
	{ "v_3pa.bin",	0x1000, 0x4974ffef, 3 }, //  5

	{ "v_7c.bin",	0x0800, 0xdc7f4164, 4 }, //  6 gfx2
	{ "v_7d.bin",	0x0800, 0x0ce7dcf6, 4 }, //  7
	{ "v_7e.bin",	0x0800, 0x24d1ff17, 4 }, //  8
	{ "v_7f.bin",	0x0800, 0x0f8c083f, 4 }, //  9

	{ "c-2e.bpr",	0x0100, 0x463dc7ad, 5 }, // 10 proms
	{ "c-2f.bpr",	0x0100, 0x47ba0042, 5 }, // 11
	{ "v-2n.bpr",	0x0100, 0xdbf185bf, 5 }, // 12
};

STD_ROM_PICK(dkongjrj)
STD_ROM_FN(dkongjrj)

struct BurnDriver BurnDrvDkongjrj = {
	"dkongjrj", "dkongjr", NULL, "dkongjr", "1982",
	"Donkey Kong Jr. (Japan)\0", NULL, "Nintendo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongjrjRomInfo, dkongjrjRomName, DkongjrSampleInfo, DkongjrSampleName, DkongInputInfo, DkongDIPInfo,
	dkongjrInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong Junior (Japan?)

static struct BurnRomInfo dkongjnrjRomDesc[] = {
	{ "dkjp.5b",	0x2000, 0x7b48870b, 1 }, //  0 maincpu
	{ "dkjp.5c",	0x2000, 0x12391665, 1 }, //  1
	{ "dkjp.5e",	0x2000, 0x6c9f9103, 1 }, //  2

	{ "c_3h.bin",	0x1000, 0x715da5f8, 2 }, //  3 soundcpu

	{ "dkj.3n",	0x1000, 0x8d51aca9, 3 }, //  4 gfx1
	{ "dkj.3p",	0x1000, 0x4ef64ba5, 3 }, //  5

	{ "v_7c.bin",	0x0800, 0xdc7f4164, 4 }, //  6 gfx2
	{ "v_7d.bin",	0x0800, 0x0ce7dcf6, 4 }, //  7
	{ "v_7e.bin",	0x0800, 0x24d1ff17, 4 }, //  8
	{ "v_7f.bin",	0x0800, 0x0f8c083f, 4 }, //  9

	{ "c-2e.bpr",	0x0100, 0x463dc7ad, 5 }, // 10 proms
	{ "c-2f.bpr",	0x0100, 0x47ba0042, 5 }, // 11
	{ "v-2n.bpr",	0x0100, 0xdbf185bf, 5 }, // 12
};

STD_ROM_PICK(dkongjnrj)
STD_ROM_FN(dkongjnrj)

struct BurnDriver BurnDrvDkongjnrj = {
	"dkongjnrj", "dkongjr", NULL, "dkongjr", "1982",
	"Donkey Kong Junior (Japan?)\0", NULL, "Nintendo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongjnrjRomInfo, dkongjnrjRomName, DkongjrSampleInfo, DkongjrSampleName, DkongInputInfo, DkongDIPInfo,
	dkongjrInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong Jr. (bootleg)

static struct BurnRomInfo dkongjrbRomDesc[] = {
	{ "dkjr1",	0x2000, 0xec7e097f, 1 }, //  0 maincpu
	{ "c_5ca.bin",	0x2000, 0xc0a18f0d, 1 }, //  1
	{ "c_5ea.bin",	0x2000, 0xa81dd00c, 1 }, //  2

	{ "c_3h.bin",	0x1000, 0x715da5f8, 2 }, //  3 soundcpu

	{ "v_3na.bin",	0x1000, 0xa95c4c63, 3 }, //  4 gfx1
	{ "dkjr10",	0x1000, 0xadc11322, 3 }, //  5

	{ "v_7c.bin",	0x0800, 0xdc7f4164, 4 }, //  6 gfx2
	{ "v_7d.bin",	0x0800, 0x0ce7dcf6, 4 }, //  7
	{ "v_7e.bin",	0x0800, 0x24d1ff17, 4 }, //  8
	{ "v_7f.bin",	0x0800, 0x0f8c083f, 4 }, //  9

	{ "c-2e.bpr",	0x0100, 0x463dc7ad, 5 }, // 10 proms
	{ "c-2f.bpr",	0x0100, 0x47ba0042, 5 }, // 11
	{ "v-2n.bpr",	0x0100, 0xdbf185bf, 5 }, // 12
};

STD_ROM_PICK(dkongjrb)
STD_ROM_FN(dkongjrb)

struct BurnDriver BurnDrvDkongjrb = {
	"dkongjrb", "dkongjr", NULL, "dkongjr", "1982",
	"Donkey Kong Jr. (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongjrbRomInfo, dkongjrbRomName, DkongjrSampleInfo, DkongjrSampleName, DkongInputInfo, DkongDIPInfo,
	dkongjrInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Junior King (bootleg of Donkey Kong Jr.)

static struct BurnRomInfo jrkingRomDesc[] = {
	{ "b5.bin",	0x2000, 0xec7e097f, 1 }, //  0 maincpu
	{ "c5.bin",	0x2000, 0xc0a18f0d, 1 }, //  1
	{ "e5.bin",	0x2000, 0xa81dd00c, 1 }, //  2

	{ "h3.bin",	0x1000, 0x715da5f8, 2 }, //  3 soundcpu

	{ "n3.bin",	0x1000, 0x7110715d, 3 }, //  4 gfx1
	{ "p3.bin",	0x1000, 0x46476016, 3 }, //  5

	{ "c7.bin",	0x1000, 0x9f531527, 4 }, //  6 gfx2
	{ "d7.bin",	0x1000, 0x32fbd41b, 4 }, //  7
	{ "e7.bin",	0x1000, 0x2286bf8e, 4 }, //  8
	{ "f7.bin",	0x1000, 0x627007a0, 4 }, //  9

	{ "c-2e.bpr",	0x0100, 0x463dc7ad, 5 }, // 10 proms
	{ "c-2f.bpr",	0x0100, 0x47ba0042, 5 }, // 11
	{ "v-2n.bpr",	0x0100, 0xdbf185bf, 5 }, // 12
};

STD_ROM_PICK(jrking)
STD_ROM_FN(jrking)

struct BurnDriver BurnDrvJrking = {
	"jrking", "dkongjr", NULL, "dkongjr", "1982",
	"Junior King (bootleg of Donkey Kong Jr.)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, jrkingRomInfo, jrkingRomName, DkongjrSampleInfo, DkongjrSampleName, DkongInputInfo, DkongDIPInfo,
	dkongjrInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey King Jr. (bootleg of Donkey Kong Jr.)

static struct BurnRomInfo dkingjrRomDesc[] = {
	{ "1.7g",	0x2000, 0xbd07bb8d, 1 }, //  0 maincpu
	{ "2.7h",	0x2000, 0x01fbec11, 1 }, //  1
	{ "3.7k",	0x2000, 0xa81dd00c, 1 }, //  2

	{ "4.7l",	0x1000, 0x715da5f8, 2 }, //  3 soundcpu

	{ "5.6g",	0x1000, 0xcf14669d, 3 }, //  4 gfx1
	{ "6.6e",	0x1000, 0xcefed15e, 3 }, //  5

	{ "7.2t",	0x0800, 0xdc7f4164, 4 }, //  6 gfx2
	{ "8.2r",	0x0800, 0x0ce7dcf6, 4 }, //  7
	{ "9.2p",	0x0800, 0x24d1ff17, 4 }, //  8
	{ "10.2m",	0x0800, 0x0f8c083f, 4 }, //  9

	{ "mb7052.9k",	0x0100, 0x49f2d444, 5 }, // 10 proms
	{ "mb7052.9l",	0x0100, 0x487513ab, 5 }, // 11
	{ "mb7052.6b",	0x0100, 0xdbf185bf, 5 }, // 12

	{ "mb7051.8j",	0x0020, 0xa5a6f2ca, 5 }, // 13
};

STD_ROM_PICK(dkingjr)
STD_ROM_FN(dkingjr)

static INT32 dkingjrRomLoad()
{
	INT32 ret = dkongjrRomLoad();

	for (INT32 i = 0; i < 0x200; i++) {
		DrvColPROM[i] ^= 0xff;
	}

	return ret;
}

static INT32 dkingjrInit()
{
	return DrvInit(dkingjrRomLoad, dkongPaletteInit, 0);
}

struct BurnDriver BurnDrvDkingjr = {
	"dkingjr", "dkongjr", NULL, "dkongjr", "1982",
	"Donkey King Jr. (bootleg of Donkey Kong Jr.)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkingjrRomInfo, dkingjrRomName, DkongjrSampleInfo, DkongjrSampleName, DkongInputInfo, DkongDIPInfo,
	dkingjrInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong Junior (Easy)

static struct BurnRomInfo dkongjreRomDesc[] = {
	{ "djr1-c.5b",	0x2000, 0xffe9e1a5, 1 }, //  0 maincpu
	{ "djr1-c.5c",	0x2000, 0x982e30e8, 1 }, //  1
	{ "djr1-c.5e",	0x2000, 0x24c3d325, 1 }, //  2

	{ "c_3h.bin",	0x1000, 0x715da5f8, 2 }, //  3 soundcpu

	{ "dkj.3n",	0x1000, 0x8d51aca9, 3 }, //  4 gfx1
	{ "dkj.3p",	0x1000, 0x4ef64ba5, 3 }, //  5

	{ "v_7c.bin",	0x0800, 0xdc7f4164, 4 }, //  6 gfx2
	{ "v_7d.bin",	0x0800, 0x0ce7dcf6, 4 }, //  7
	{ "v_7e.bin",	0x0800, 0x24d1ff17, 4 }, //  8
	{ "v_7f.bin",	0x0800, 0x0f8c083f, 4 }, //  9

	{ "c-2e.bpr",	0x0100, 0x463dc7ad, 5 }, // 10 proms
	{ "c-2f.bpr",	0x0100, 0x47ba0042, 5 }, // 11
	{ "v-2n.bpr",	0x0100, 0xdbf185bf, 5 }, // 12

	{ "djr1-c.5a",	0x1000, 0xbb5f5180, 1 }, // 13 extra cpu rom
};

STD_ROM_PICK(dkongjre)
STD_ROM_FN(dkongjre)

static INT32 dkongjreRomLoad()
{
	INT32 ret = dkongjrRomLoad();

	if (BurnLoadRom(DrvZ80ROM + 0x8000, 13, 1)) return 1;

	return ret;
}

static INT32 dkongjreInit()
{
	return DrvInit(dkongjreRomLoad, dkongPaletteInit, 0);
}

struct BurnDriverD BurnDrvDkongjre = {
	"dkongjre", "dkongjr", NULL, "dkongjr", "1982",
	"Donkey Kong Junior (Easy)\0", NULL, "Nintendo of America", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkongjreRomInfo, dkongjreRomName, DkongjrSampleInfo, DkongjrSampleName, DkongInputInfo, DkongDIPInfo,
	dkongjreInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Pest Place

static struct BurnRomInfo pestplceRomDesc[] = {
	{ "pest.1p",		0x2000, 0x80d50721, 1 }, //  0 maincpu
	{ "pest.2p",		0x2000, 0x9c3681cc, 1 }, //  1
	{ "pest.3p",		0x2000, 0x49853922, 1 }, //  2

	{ "pest.4",		0x1000, 0x715da5f8, 2 }, //  3 soundcpu

	{ "pest.o",		0x1000, 0x03939ece, 3 }, //  4 gfx1
	{ "pest.k",		0x1000, 0x2acacedf, 3 }, //  5

	{ "pest.b",		0x1000, 0xe760073e, 4 }, //  6 gfx2
	{ "pest.a",		0x1000, 0x1958346e, 4 }, //  7
	{ "pest.d",		0x1000, 0x3a993c17, 4 }, //  8
	{ "pest.c",		0x1000, 0xbf08f2a3, 4 }, //  9

	{ "n82s129a.bin",	0x0100, 0x0330f35f, 5 }, // 10 proms
	{ "n82s129b.bin",	0x0100, 0xba88311b, 5 }, // 11
	{ "sn74s288n.bin",	0x0020, 0xa5a6f2ca, 5 }, // 12

	{ "pest.0",		0x1000, 0x28952b56, 1 }, // 13
};

STD_ROM_PICK(pestplce)
STD_ROM_FN(pestplce)

static INT32 pestplceRomLoad()
{
	INT32 ret = dkongjrRomLoad();

	if (BurnLoadRom(DrvZ80ROM + 0xb000, 13, 1)) return 1;

	for (INT32 i = 0; i < 0x300; i++) { // invert colors
		DrvColPROM[i] ^= 0xff;
	}

	return ret;
}

static INT32 pestplceInit()
{
	return DrvInit(pestplceRomLoad, dkongPaletteInit, 1);
}

struct BurnDriver BurnDrvPestplce = {
	"pestplce", "mario", NULL, NULL, "1983",
	"Pest Place\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, pestplceRomInfo, pestplceRomName, NULL, NULL, PestplceInputInfo, PestplceDIPInfo,
	pestplceInit, DrvExit, DrvFrame, pestplceDraw, NULL, &DrvRecalc, 0x100,
	256, 224, 4, 3
};


// Donkey Kong 3 (US)

static struct BurnRomInfo dkong3RomDesc[] = {
	{ "dk3c.7b",	0x2000, 0x38d5f38e, 1 }, //  0 maincpu
	{ "dk3c.7c",	0x2000, 0xc9134379, 1 }, //  1
	{ "dk3c.7d",	0x2000, 0xd22e2921, 1 }, //  2
	{ "dk3c.7e",	0x2000, 0x615f14b7, 1 }, //  3

	{ "dk3c.5l",	0x2000, 0x7ff88885, 2 }, //  4 n2a03a

	{ "dk3c.6h",	0x2000, 0x36d7200c, 3 }, //  5 n2a03b

	{ "dk3v.3n",	0x1000, 0x415a99c7, 4 }, //  6 gfx1
	{ "dk3v.3p",	0x1000, 0x25744ea0, 4 }, //  7

	{ "dk3v.7c",	0x1000, 0x8ffa1737, 5 }, //  8 gfx2
	{ "dk3v.7d",	0x1000, 0x9ac84686, 5 }, //  9
	{ "dk3v.7e",	0x1000, 0x0c0af3fb, 5 }, // 10
	{ "dk3v.7f",	0x1000, 0x55c58662, 5 }, // 11

	{ "dkc1-c.1d",	0x0200, 0xdf54befc, 6 }, // 12 proms
	{ "dkc1-c.1c",	0x0200, 0x66a77f40, 6 }, // 13
	{ "dkc1-v.2n",	0x0100, 0x50e33434, 6 }, // 14

	{ "dkc1-v.5e",	0x0020, 0xd3e2eaf8, 7 }, // 15 adrdecode
};

STD_ROM_PICK(dkong3)
STD_ROM_FN(dkong3)

static INT32 dkong3RomLoad()
{
	if (BurnLoadRom(DrvZ80ROM  + 0x0000,  0, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x2000,  1, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x4000,  2, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x8000,  3, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0 + 0x0000,  4, 1)) return 1;
	if (BurnLoadRom(DrvSndROM1 + 0x0000,  5, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x0000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x1000,  7, 1)) return 1;

	memcpy (DrvGfxROM0 + 0x0000, DrvGfxROM1 + 0x0800, 0x0800);
	memcpy (DrvGfxROM0 + 0x0800, DrvGfxROM1 + 0x0000, 0x0800);
	memcpy (DrvGfxROM0 + 0x1000, DrvGfxROM1 + 0x1800, 0x0800);
	memcpy (DrvGfxROM0 + 0x1800, DrvGfxROM1 + 0x1000, 0x0800);

	if (BurnLoadRom(DrvGfxROM1 + 0x0000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x1000,  9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x2000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x3000, 11, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x0000, 12, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0100, 13, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0200, 14, 1)) return 1;

	return 0;
}

static INT32 dkong3Init()
{
	INT32 ret = DrvInit(dkong3RomLoad, dkong3PaletteInit, 2|1);

	if (ret == 0)
	{
		ZetOpen(0);
		ZetSetWriteHandler(dkong3_main_write);
		ZetSetReadHandler(dkong3_main_read);
		ZetClose();
	}

	return ret;
}

struct BurnDriver BurnDrvDkong3 = {
	"dkong3", NULL, NULL, NULL, "1983",
	"Donkey Kong 3 (US)\0", "No sound", "Nintendo of America", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkong3RomInfo, dkong3RomName, NULL, NULL, Dkong3InputInfo, Dkong3DIPInfo,
	dkong3Init, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong 3 (Japan)

static struct BurnRomInfo dkong3jRomDesc[] = {
	{ "dk3c.7b",	0x2000, 0x38d5f38e, 1 }, //  0 maincpu
	{ "dk3c.7c",	0x2000, 0xc9134379, 1 }, //  1
	{ "dk3c.7d",	0x2000, 0xd22e2921, 1 }, //  2
	{ "dk3cj.7e",	0x2000, 0x25b5be23, 1 }, //  3

	{ "dk3c.5l",	0x2000, 0x7ff88885, 2 }, //  4 n2a03a

	{ "dk3c.6h",	0x2000, 0x36d7200c, 3 }, //  5 n2a03b

	{ "dk3v.3n",	0x1000, 0x415a99c7, 4 }, //  6 gfx1
	{ "dk3v.3p",	0x1000, 0x25744ea0, 4 }, //  7

	{ "dk3v.7c",	0x1000, 0x8ffa1737, 5 }, //  8 gfx2
	{ "dk3v.7d",	0x1000, 0x9ac84686, 5 }, //  9
	{ "dk3v.7e",	0x1000, 0x0c0af3fb, 5 }, // 10
	{ "dk3v.7f",	0x1000, 0x55c58662, 5 }, // 11

	{ "dkc1-c.1d",	0x0200, 0xdf54befc, 6 }, // 12 proms
	{ "dkc1-c.1c",	0x0200, 0x66a77f40, 6 }, // 13
	{ "dkc1-v.2n",	0x0100, 0x50e33434, 6 }, // 14

	{ "dkc1-v.5e",	0x0020, 0xd3e2eaf8, 7 }, // 15 adrdecode
};

STD_ROM_PICK(dkong3j)
STD_ROM_FN(dkong3j)

struct BurnDriver BurnDrvDkong3j = {
	"dkong3j", "dkong3", NULL, NULL, "1983",
	"Donkey Kong 3 (Japan)\0", "No sound", "Nintendo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkong3jRomInfo, dkong3jRomName, NULL, NULL, Dkong3InputInfo, Dkong3DIPInfo,
	dkong3Init, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Donkey Kong 3 (bootleg on Donkey Kong Jr. hardware)

static struct BurnRomInfo dkong3bRomDesc[] = {
	{ "5b.bin",	0x2000, 0x549979bc, 1 }, //  0 maincpu
	{ "5c-2.bin",	0x2000, 0xb9dcbae6, 1 }, //  1
	{ "5e-2.bin",	0x2000, 0x5a61868f, 1 }, //  2

	{ "3h.bin",	0x1000, 0x715da5f8, 2 }, //  3 soundcpu

	{ "3n.bin",	0x1000, 0xfed67d35, 3 }, //  4 gfx1
	{ "3p.bin",	0x1000, 0x3d1b87ce, 3 }, //  5

	{ "7c.bin",	0x1000, 0x8ffa1737, 4 }, //  6 gfx2
	{ "7d.bin",	0x1000, 0x9ac84686, 4 }, //  7
	{ "7e.bin",	0x1000, 0x0c0af3fb, 4 }, //  8
	{ "7f.bin",	0x1000, 0x55c58662, 4 }, //  9

	{ "dk3b-c.1d",	0x0200, 0xdf54befc, 5 }, // 10 proms
	{ "dk3b-c.1c",	0x0200, 0x66a77f40, 5 }, // 11
	{ "dk3b-v.2n",	0x0100, 0x50e33434, 5 }, // 12

	{ "5c-1.bin",	0x1000, 0x77a012d6, 1 }, // 13
	{ "5e-1.bin",	0x1000, 0x745ed767, 1 }, // 14
};

STD_ROM_PICK(dkong3b)
STD_ROM_FN(dkong3b)

static INT32 dkong3bRomLoad()
{
	INT32 ret = dkongjrRomLoad();

	if (BurnLoadRom(DrvGfxROM1 + 0x0000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x1000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x2000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x3000,  9, 1)) return 1;

	if (BurnLoadRom(DrvZ80ROM  + 0x9000, 13, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0xd000, 14, 1)) return 1;

	return ret;
}

static INT32 dkong3bInit()
{
	return DrvInit(dkong3bRomLoad, dkong3PaletteInit, 1);
}

struct BurnDriver BurnDrvDkong3b = {
	"dkong3b", "dkong3", NULL, NULL, "1984",
	"Donkey Kong 3 (bootleg on Donkey Kong Jr. hardware)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, dkong3bRomInfo, dkong3bRomName, NULL, NULL, DkongInputInfo, Dkong3bDIPInfo,
	dkong3bInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};




// Herbie at the Olympics (DK conversion)

static struct BurnRomInfo herbiedkRomDesc[] = {
	{ "5f.cpu",		0x1000, 0xc7ab3ac6, 1 }, //  0 maincpu
	{ "5g.cpu",		0x1000, 0xd1031aa6, 1 }, //  1
	{ "5h.cpu",		0x1000, 0xc0daf551, 1 }, //  2
	{ "5k.cpu",		0x1000, 0x67442242, 1 }, //  3

	{ "3i.snd",		0x0800, 0x20e30406, 2 }, //  4 soundcpu

	{ "5h.vid",		0x0800, 0xea2a2547, 3 }, //  5 gfx1
	{ "5k.vid",		0x0800, 0xa8d421c9, 3 }, //  6

	{ "7c.clk",		0x0800, 0xaf646166, 4 }, //  7 gfx2
	{ "7d.clk",		0x0800, 0xd8e15832, 4 }, //  8
	{ "7e.clk",		0x0800, 0x2f7e65fa, 4 }, //  9
	{ "7f.clk",		0x0800, 0xad32d5ae, 4 }, // 10

	{ "74s287.2k",		0x0100, 0x7dc0a381, 5 }, // 11 proms
	{ "74s287.2j",		0x0100, 0x0a440c00, 5 }, // 12
	{ "74s287.vid",		0x0100, 0x5a3446cc, 5 }, // 13

	{ "82s147.hh",		0x0200, 0x46e5bc92, 6 }, // 14 user1

	{ "pls153h.bin",	0x00eb, 0xd6a04bcc, 7 }, // 15 plds
};

STD_ROM_PICK(herbiedk)
STD_ROM_FN(herbiedk)

static INT32 herbiedkRomLoad()
{
	if (BurnLoadRom(Drv2650ROM  + 0x0000,  0, 1)) return 1;
	if (BurnLoadRom(Drv2650ROM  + 0x2000,  1, 1)) return 1;
	if (BurnLoadRom(Drv2650ROM  + 0x4000,  2, 1)) return 1;
	if (BurnLoadRom(Drv2650ROM  + 0x6000,  3, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0  + 0x0000,  4, 1)) return 1;
	memcpy (DrvSndROM0 + 0x0800, DrvSndROM0 + 0x0000, 0x0800); // re-load
	memset (DrvSndROM0 + 0x1000, 0xff, 0x0800);

	if (BurnLoadRom(DrvGfxROM0 + 0x0000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x1000,  6, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x0000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x1000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x2000,  9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x3000, 10, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x0000, 11, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0100, 12, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0200, 13, 1)) return 1;

	if (BurnLoadRom(DrvMapROM  + 0x0000, 14, 1)) return 1;

	return 0;
}

static INT32 herbiedkInit()
{
	s2650_protection = 1;
	return s2650DkongInit(herbiedkRomLoad);
}

struct BurnDriver BurnDrvHerbiedk = {
	"herbiedk", "huncholy", NULL, NULL, "1984",
	"Herbie at the Olympics (DK conversion)\0", "No sound", "Century Electronics / Seatongrove Ltd", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, herbiedkRomInfo, herbiedkRomName, NULL, NULL, DkongInputInfo, HerbiedkDIPInfo,
	herbiedkInit, s2650DkongExit, s2650DkongFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Hunchback (DK conversion)

static struct BurnRomInfo hunchbkdRomDesc[] = {
	{ "hb.5e",		0x1000, 0x4c3ac070, 1 }, //  0 maincpu
	{ "hbsc-1.5c",		0x1000, 0x9b0e6234, 1 }, //  1
	{ "hb.5b",		0x1000, 0x4cde80f3, 1 }, //  2
	{ "hb.5a",		0x1000, 0xd60ef5b2, 1 }, //  3

	{ "hb.3h",		0x0800, 0xa3c240d4, 2 }, //  4 soundcpu

	{ "hb.3n",		0x0800, 0x443ed5ac, 3 }, //  5 gfx1
	{ "hb.3p",		0x0800, 0x073e7b0c, 3 }, //  6

	{ "hb.7c",		0x0800, 0x3ba71686, 4 }, //  7 gfx2
	{ "hb.7d",		0x0800, 0x5786948d, 4 }, //  8
	{ "hb.7e",		0x0800, 0xf845e8ca, 4 }, //  9
	{ "hb.7f",		0x0800, 0x52d20fea, 4 }, // 10

	{ "hbprom.2e",		0x0100, 0x37aab98f, 5 }, // 11 proms
	{ "hbprom.2f",		0x0100, 0x845b8dcc, 5 }, // 12
	{ "hbprom.2n",		0x0100, 0xdff9070a, 5 }, // 13

	{ "82s147.prm",		0x0200, 0x46e5bc92, 6 }, // 14 user1

	{ "pls153h.bin",	0x00eb, 0x00000000, 7 | BRF_NODUMP }, // 15 plds
};

STD_ROM_PICK(hunchbkd)
STD_ROM_FN(hunchbkd)

static INT32 hunchbkdInit()
{
	s2650_protection = 2;
	return s2650DkongInit(herbiedkRomLoad);
}

struct BurnDriverD BurnDrvHunchbkd = {
	"hunchbkd", "hunchbak", NULL, NULL, "1983",
	"Hunchback (DK conversion)\0", "No sound", "Century Electronics", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, hunchbkdRomInfo, hunchbkdRomName, NULL, NULL, DkongInputInfo, HunchbkdDIPInfo,
	hunchbkdInit, s2650DkongExit, s2650DkongFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};

/*

// Super Bike (DK conversion)

static struct BurnRomInfo sbdkRomDesc[] = {
	{ "sb-dk.ap",		0x1000, 0xfef0ef9c, 1 }, //  0 maincpu
	{ "sb-dk.ay",		0x1000, 0x2e9dade2, 1 }, //  1
	{ "sb-dk.as",		0x1000, 0xe6d200f3, 1 }, //  2
	{ "sb-dk.5a",		0x1000, 0xca41ca56, 1 }, //  3

	{ "sb-dk.3h",		0x0800, 0x13e60b6e, 2 }, //  4 soundcpu

	{ "sb-dk.3n",		0x0800, 0xb1d76b59, 3 }, //  5 gfx1
	{ "sb-dk.3p",		0x0800, 0xea5f9f88, 3 }, //  6

	{ "sb-dk.7c",		0x0800, 0xc12c18f2, 4 }, //  7 gfx2
	{ "sb-dk.7d",		0x0800, 0xf7a32d23, 4 }, //  8
	{ "sb-dk.7e",		0x0800, 0x8e48b13e, 4 }, //  9
	{ "sb-dk.7f",		0x0800, 0x989969f3, 4 }, // 10

	{ "sb.2e",		0x0100, 0x4f06f789, 5 }, // 11 proms
	{ "sb.2f",		0x0100, 0x2c15b1b2, 5 }, // 12
	{ "sb.2n",		0x0100, 0xdff9070a, 5 }, // 13

	{ "82s147.prm",		0x0200, 0x46e5bc92, 6 }, // 14 user1

	{ "pls153h.bin",	0x00eb, 0x00000000, 7 | BRF_NODUMP }, // 15 plds
};

STD_ROM_PICK(sbdk)
STD_ROM_FN(sbdk)

struct BurnDriverD BurnDrvSbdk = {
	"sbdk", "superbik", NULL, NULL, "1984",
	"Super Bike (DK conversion)\0", NULL, "Century Electronics", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, sbdkRomInfo, sbdkRomName, NULL, NULL, SbdkInputInfo, SbdkDIPInfo,
	hunchbkdInit, s2650DkongExit, s2650DkongFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};
*/

// Hero in the Castle of Doom (DK conversion)

static struct BurnRomInfo herodkRomDesc[] = {
	{ "red-dot.rgt",	0x2000, 0x9c4af229, 1 }, //  0 maincpu
	{ "wht-dot.lft",	0x2000, 0xc10f9235, 1 }, //  1

	{ "silver.3h",		0x0800, 0x67863ce9, 2 }, //  2 soundcpu

	{ "pnk.3n",		0x0800, 0x574dfd7a, 3 }, //  3 gfx1
	{ "blk.3p",		0x0800, 0x16f7c040, 3 }, //  4

	{ "gold.7c",		0x0800, 0x5f5282ed, 4 }, //  5 gfx2
	{ "orange.7d",		0x0800, 0x075d99f5, 4 }, //  6
	{ "yellow.7e",		0x0800, 0xf6272e96, 4 }, //  7
	{ "violet.7f",		0x0800, 0xca020685, 4 }, //  8

	{ "82s129.2e",		0x0100, 0xda4b47e6, 5 }, //  9 proms
	{ "82s129.2f",		0x0100, 0x96e213a4, 5 }, // 10
	{ "82s126.2n",		0x0100, 0x37aece4b, 5 }, // 11

	{ "82s147.prm",		0x0200, 0x46e5bc92, 6 }, // 12 user1

	{ "pls153h.bin",	0x00eb, 0x00000000, 7 | BRF_NODUMP }, // 13 plds
};

STD_ROM_PICK(herodk)
STD_ROM_FN(herodk)

static INT32 herodkRomLoad()
{
	if (BurnLoadRom(Drv2650ROM + 0x0000,  0, 1)) return 1;
	if (BurnLoadRom(Drv2650ROM + 0x2000,  1, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0  + 0x0000, 2, 1)) return 1;
	memcpy (DrvSndROM0 + 0x0800, DrvSndROM0 + 0x0000, 0x0800); // re-load
	memset (DrvSndROM0 + 0x1000, 0xff, 0x0800);

	if (BurnLoadRom(DrvGfxROM0 + 0x0000,  3, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x1000,  4, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x0000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x1000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x2000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x3000,  8, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x0000,  9, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0100, 10, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0200, 11, 1)) return 1;

	if (BurnLoadRom(DrvMapROM  + 0x0000, 12, 1)) return 1;

	return 0;
}

static INT32 herodkLoad()
{
	if (herodkRomLoad()) return 1;

	UINT8 *tmp = (UINT8*)BurnMalloc(0x4000);

	memcpy (tmp, Drv2650ROM, 0x4000);

	memcpy (Drv2650ROM + 0x0c00, tmp + 0x0000, 0x0400);
	memcpy (Drv2650ROM + 0x0800, tmp + 0x0400, 0x0400);
	memcpy (Drv2650ROM + 0x0400, tmp + 0x0800, 0x0400);
	memcpy (Drv2650ROM + 0x0000, tmp + 0x0c00, 0x0400);
	memcpy (Drv2650ROM + 0x2000, tmp + 0x1000, 0x0e00);
	memcpy (Drv2650ROM + 0x6e00, tmp + 0x1e00, 0x0200);
	memcpy (Drv2650ROM + 0x4000, tmp + 0x2000, 0x1000);
	memcpy (Drv2650ROM + 0x6000, tmp + 0x3000, 0x0e00);
	memcpy (Drv2650ROM + 0x2e00, tmp + 0x3e00, 0x0200);

	BurnFree (tmp);

	for (INT32 i = 0; i < 0x8000; i++)
	{
 		if ((i & 0x1000) == 0)
		{
			INT32 v = Drv2650ROM[i];
			Drv2650ROM[i] = (v & 0xe7) | ((v & 0x10) >> 1) | ((v & 0x08) << 1);
		}
	}

	return 0;
}

static INT32 herodkInit()
{
	s2650_protection = 2;

	return s2650DkongInit(herodkLoad);
}

struct BurnDriver BurnDrvHerodk = {
	"herodk", "hero", NULL, NULL, "1984",
	"Hero in the Castle of Doom (DK conversion)\0", "No sound", "Seatongrove Ltd (Crown license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, herodkRomInfo, herodkRomName, NULL, NULL, HerodkInputInfo, HerodkDIPInfo,
	herodkInit, s2650DkongExit, s2650DkongFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Hero in the Castle of Doom (DK conversion not encrypted)

static struct BurnRomInfo herodkuRomDesc[] = {
	{ "2764.8h",		0x2000, 0x989ce053, 1 }, //  0 maincpu
	{ "2764.8f",		0x2000, 0x835e0074, 1 }, //  1

	{ "2716.3h",		0x0800, 0xcaf57bef, 2 }, //  2 soundcpu

	{ "pnk.3n",		0x0800, 0x574dfd7a, 3 }, //  3 gfx1
	{ "blk.3p",		0x0800, 0x16f7c040, 3 }, //  4

	{ "gold.7c",		0x0800, 0x5f5282ed, 4 }, //  5 gfx2
	{ "orange.7d",		0x0800, 0x075d99f5, 4 }, //  6
	{ "yellow.7e",		0x0800, 0xf6272e96, 4 }, //  7
	{ "violet.7f",		0x0800, 0xca020685, 4 }, //  8

	{ "82s129.2e",		0x0100, 0xda4b47e6, 5 }, //  9 proms
	{ "82s129.2f",		0x0100, 0x96e213a4, 5 }, // 10
	{ "82s126.2n",		0x0100, 0x37aece4b, 5 }, // 11

	{ "82s147.prm",		0x0200, 0x46e5bc92, 6 }, // 12 user1

	{ "pls153h.bin",	0x00eb, 0x00000000, 7 | BRF_NODUMP }, // 13 plds
};

STD_ROM_PICK(herodku)
STD_ROM_FN(herodku)

static INT32 herodkuLoad()
{
	if (herodkRomLoad()) return 1;

	UINT8 *tmp = (UINT8*)BurnMalloc(0x4000);

	memcpy (tmp, Drv2650ROM, 0x4000);

	memcpy (Drv2650ROM + 0x0c00, tmp + 0x0000, 0x0400);
	memcpy (Drv2650ROM + 0x0800, tmp + 0x0400, 0x0400);
	memcpy (Drv2650ROM + 0x0400, tmp + 0x0800, 0x0400);
	memcpy (Drv2650ROM + 0x0000, tmp + 0x0c00, 0x0400);
	memcpy (Drv2650ROM + 0x2000, tmp + 0x1000, 0x1000);
	memcpy (Drv2650ROM + 0x4000, tmp + 0x2000, 0x1000);
	memcpy (Drv2650ROM + 0x6000, tmp + 0x3000, 0x1000);

	BurnFree (tmp);

	return 0;
}

static INT32 herodkuInit()
{
	s2650_protection = 2;

	return s2650DkongInit(herodkuLoad);
}

struct BurnDriver BurnDrvHerodku = {
	"herodku", "hero", NULL, NULL, "1984",
	"Hero in the Castle of Doom (DK conversion not encrypted)\0", NULL, "Seatongrove Ltd (Crown license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, herodkuRomInfo, herodkuRomName, NULL, NULL, HerodkInputInfo, HerodkDIPInfo,
	herodkuInit, s2650DkongExit, s2650DkongFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};



static UINT8 decrypt_counter = 0;

static void epos_bankswitch(INT32 bank)
{
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80ROM + 0x10000 + (bank * 0x4000));
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80ROM + 0x10000 + (bank * 0x4000));
}

UINT8 __fastcall epos_main_read_port(UINT16 port)
{
	if (port & 0x01)
	{
		decrypt_counter = (decrypt_counter - 1) & 0x0f;
	}
	else
	{
		decrypt_counter = (decrypt_counter + 1) & 0x0f;
	}

	if (decrypt_counter >= 8 && decrypt_counter <= 0x0b) {
		epos_bankswitch(decrypt_counter & 3);
	}

	return 0;
}


static void epos_decrypt_rom(UINT8 mod, INT32 offs, INT32 *bs)
{
    UINT8 oldbyte,newbyte;
    UINT8 *ROM;
    INT32 mem;

    ROM = DrvZ80ROM;

    for (mem=0;mem<0x4000;mem++)
    {
        oldbyte = ROM[mem];

        newbyte = (oldbyte & mod) | (~oldbyte & ~mod);
        newbyte = BITSWAP08(newbyte, bs[0], bs[1], bs[2], bs[3], bs[4], bs[5], bs[6], bs[7]);

        ROM[mem + offs] = newbyte;
    }
}


static INT32 eposRomLoad()
{
	if (BurnLoadRom(DrvZ80ROM  + 0x0000,  0, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x2000,  1, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0  + 0x0000, 2, 1)) return 1;
	memcpy (DrvSndROM0 + 0x0800, DrvSndROM0 + 0x0000, 0x0800); // re-load
	memset (DrvSndROM0 + 0x1000, 0x00, 0x0800);

	if (BurnLoadRom(DrvGfxROM0 + 0x0000,  3, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x1000,  4, 1)) return 1;
	memcpy (DrvGfxROM0 + 0x0800, DrvGfxROM0 + 0x0000, 0x0800); // re-load
	memcpy (DrvGfxROM0 + 0x1800, DrvGfxROM0 + 0x1000, 0x0800);

	if (BurnLoadRom(DrvGfxROM1 + 0x0000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x1000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x2000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x3000,  8, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x0000,  9, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0100, 10, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x0200, 11, 1)) return 1;

	return 0;
}




// Drakton (DK conversion)

static struct BurnRomInfo draktonRomDesc[] = {
	{ "2764.u2",	0x2000, 0xd9a33205, 1 }, //  0 maincpu
	{ "2764.u3",	0x2000, 0x69583a35, 1 }, //  1

	{ "2716.3h",	0x0800, 0x3489a35b, 2 }, //  2 soundcpu

	{ "2716.3n",	0x0800, 0xea0e7f9a, 3 }, //  3 gfx1
	{ "2716.3p",	0x0800, 0x46f51b68, 3 }, //  4

	{ "2716.7c",	0x0800, 0x2925dc2d, 4 }, //  5 gfx2
	{ "2716.7d",	0x0800, 0xbdf6b1b4, 4 }, //  6
	{ "2716.7e",	0x0800, 0x4d62e62f, 4 }, //  7
	{ "2716.7f",	0x0800, 0x81d200e5, 4 }, //  8

	{ "82s126.2e",	0x0100, 0x3ff45f76, 5 }, //  9 proms
	{ "82s126.2f",	0x0100, 0x38f905be, 5 }, // 10
	{ "82s126.2n",	0x0100, 0x3c343b9b, 5 }, // 11
};

STD_ROM_PICK(drakton)
STD_ROM_FN(drakton)

static INT32 draktonLoad()
{
	INT32 bs[4][8] = {
		{7,6,1,3,0,4,2,5},
		{7,1,4,3,0,6,2,5},
		{7,6,1,0,3,4,2,5},
		{7,1,4,0,3,6,2,5},
	};

	if (eposRomLoad()) return 1;

	epos_decrypt_rom(0x02, 0x10000, bs[0]);
	epos_decrypt_rom(0x40, 0x14000, bs[1]);
	epos_decrypt_rom(0x8a, 0x18000, bs[2]);
	epos_decrypt_rom(0xc8, 0x1c000, bs[3]);

	return 0;
}

static INT32 draktonInit()
{
	decrypt_counter = 0x09;

	INT32 ret = DrvInit(draktonLoad, dkongPaletteInit, 0);

	if (ret == 0)
	{
		ZetOpen(0);
		ZetSetInHandler(epos_main_read_port);
		epos_bankswitch(1);
		ZetReset(); // bankswitch changed vectors
		ZetClose();
	}

	return ret;
}

struct BurnDriver BurnDrvDrakton = {
	"drakton", NULL, NULL, NULL, "1984",
	"Drakton (DK conversion)\0", "No sound", "Epos Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, draktonRomInfo, draktonRomName, NULL, NULL, DkongInputInfo, DraktonDIPInfo,
	draktonInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};


// Drakton (DKJr conversion)

static struct BurnRomInfo drktnjrRomDesc[] = {
	{ "2764.u2",	0x2000, 0xd9a33205, 1 }, //  0 maincpu
	{ "2764.u3",	0x2000, 0x69583a35, 1 }, //  1

	{ "2716.3h1",	0x0800, 0x2a6ec016, 2 }, //  2 soundcpu

	{ "2716.3n",	0x0800, 0xea0e7f9a, 3 }, //  3 gfx1
	{ "2716.3p",	0x0800, 0x46f51b68, 3 }, //  4

	{ "2716.7c",	0x0800, 0x2925dc2d, 4 }, //  5 gfx2
	{ "2716.7d",	0x0800, 0xbdf6b1b4, 4 }, //  6
	{ "2716.7e",	0x0800, 0x4d62e62f, 4 }, //  7
	{ "2716.7f",	0x0800, 0x81d200e5, 4 }, //  8

	{ "82s126.2e",	0x0100, 0x3ff45f76, 5 }, //  9 proms
	{ "82s126.2f",	0x0100, 0x38f905be, 5 }, // 10
	{ "82s126.2n",	0x0100, 0x3c343b9b, 5 }, // 11
};

STD_ROM_PICK(drktnjr)
STD_ROM_FN(drktnjr)

static INT32 drktnjrInit()
{
	decrypt_counter = 0x09;

	INT32 ret = DrvInit(draktonLoad, dkongPaletteInit, 0);

	if (ret == 0)
	{
		ZetOpen(0);
		ZetSetWriteHandler(dkongjr_main_write);
		ZetSetInHandler(epos_main_read_port);
		epos_bankswitch(1);
		ZetReset(); // bankswitch changed vectors
		ZetClose();
	}

	return ret;
}

struct BurnDriver BurnDrvDrktnjr = {
	"drktnjr", "drakton", NULL, NULL, "1984",
	"Drakton (DKJr conversion)\0", "No sound", "Epos Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, drktnjrRomInfo, drktnjrRomName, NULL, NULL, DkongInputInfo, DraktonDIPInfo,
	drktnjrInit, DrvExit, DrvFrame, dkongDraw, NULL, &DrvRecalc, 0x100,
	224, 256, 3, 4
};







#if 0



/* 2650 based */
GAME( 1984, sbdk,     superbik, s2650,    sbdk,           0,  ROT90, "Century Electronics", "Super Bike (DK conversion)", GAME_SUPPORTS_SAVE )
GAME( 1984, 8ballact, 0,        s2650,    8ballact,       0,  ROT90, "Seatongrove Ltd (Magic Eletronics USA license)", "Eight Ball Action (DK conversion)", GAME_SUPPORTS_SAVE )
GAME( 1984, 8ballact2,8ballact, s2650,    8ballact,       0,  ROT90, "Seatongrove Ltd (Magic Eletronics USA license)", "Eight Ball Action (DKJr conversion)", GAME_SUPPORTS_SAVE )
GAME( 1984, shootgal, 0,        s2650,    shootgal,       0,  ROT180,"Seatongrove Ltd (Zaccaria license)", "Shooting Gallery", GAME_IMPERFECT_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1985, spclforc, 0,        spclforc, spclforc,       0,  ROT90, "Senko Industries (Magic Eletronics Inc. license)", "Special Forces", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )
GAME( 1985, spcfrcii, 0,        spclforc, spclforc,       0,  ROT90, "Senko Industries (Magic Eletronics Inc. license)", "Special Forces II", GAME_NO_SOUND | GAME_SUPPORTS_SAVE )

/* EPOS */
GAME( 1985, strtheat, 0,        strtheat, strtheat, strtheat, ROT90, "Epos Corporation", "Street Heat", GAME_SUPPORTS_SAVE ) // distributed by Cardinal Amusements Products (a division of Epos Corporation

#endif



