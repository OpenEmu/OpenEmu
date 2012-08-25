// FB Alpha "Legendary Wings" driver module
// Based on MAME driver by Paul Leaman

#include "tiles_generic.h"
#include "z80_intf.h"
#include "burn_ym2203.h"
#include "msm5205.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvZ80ROM2;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvTileMap;
static UINT8 *DrvGfxMask;
static UINT32 *DrvPalette;
static UINT32 *Palette;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvPalRAM;
static UINT8 *DrvFgRAM;
static UINT8 *DrvBgRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprBuf;
static UINT8 *ScrollX;
static UINT8 *ScrollY;
static UINT8 DrvRecalc;

static UINT8 DrvReset;
static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDip[2];
static UINT8 DrvInp[3];

static UINT8 interrupt_enable;
static UINT8 soundlatch;
static UINT8 soundlatch2;
static UINT8 flipscreen;
static UINT8 DrvZ80Bank;

static UINT8 avengers_param[4];
static UINT32 avengers_palette_pen;
static UINT8 avengers_soundlatch2;
static UINT8 avengers_soundstate;

static UINT8 trojan_bg2_scrollx;
static UINT8 trojan_bg2_image;

static INT32 avengers = 0;
static INT32 MSM5205InUse = 0;

static INT32 nCyclesTotal[3];

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDip + 1,	"dip"		},
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo LwingsDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfe, NULL			},
	{0x12, 0xff, 0xff, 0xfe, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip_Screen"		},
	{0x11, 0x01, 0x02, 0x02, "Off"			},
	{0x11, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x11, 0x01, 0x0c, 0x0c, "3"			},
	{0x11, 0x01, 0x0c, 0x04, "4"			},
	{0x11, 0x01, 0x0c, 0x08, "5"			},
	{0x11, 0x01, 0x0c, 0x00, "6"			},

	{0   , 0xfe, 0   ,    4, "Coin_B"		},
	{0x11, 0x01, 0x30, 0x00, "4C_1C"		},
	{0x11, 0x01, 0x30, 0x20, "3C_1C"		},
	{0x11, 0x01, 0x30, 0x10, "2C_1C"		},
	{0x11, 0x01, 0x30, 0x30, "1C_1C"		},

	{0   , 0xfe, 0   ,    4, "Coin_A"		},
	{0x11, 0x01, 0xc0, 0xc0, "1C_1C"		},
	{0x11, 0x01, 0xc0, 0x00, "2C_4C"		},
	{0x11, 0x01, 0xc0, 0x40, "1C_2C"		},
	{0x11, 0x01, 0xc0, 0x80, "1C_3C"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x06, 0x02, "Easy"			},
	{0x12, 0x01, 0x06, 0x06, "Medium"		},
	{0x12, 0x01, 0x06, 0x04, "Hard"			},
	{0x12, 0x01, 0x06, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Demo_Sounds"		},
	{0x12, 0x01, 0x08, 0x00, "Off"			},
	{0x12, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow_Continue"	},
	{0x12, 0x01, 0x10, 0x00, "No"			},
	{0x12, 0x01, 0x10, 0x10, "Yes"			},

	{0   , 0xfe, 0   ,    8, "Bonus_Life"		},
	{0x12, 0x01, 0xe0, 0xe0, "20000 and every 50000"},
	{0x12, 0x01, 0xe0, 0x60, "20000 and every 60000"},
	{0x12, 0x01, 0xe0, 0xa0, "20000 and every 70000"},
	{0x12, 0x01, 0xe0, 0x20, "30000 and every 60000"},
	{0x12, 0x01, 0xe0, 0xc0, "30000 and every 70000"},
	{0x12, 0x01, 0xe0, 0x40, "30000 and every 80000"},
	{0x12, 0x01, 0xe0, 0x80, "40000 and every 100000"},
	{0x12, 0x01, 0xe0, 0x00, "None"			},
};

STDDIPINFO(Lwings)

static struct BurnDIPInfo LwingsbDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfe, NULL		},
	{0x12, 0xff, 0xff, 0xfe, NULL		},

	{0   , 0xfe, 0   ,    2, "Flip_Screen"		},
	{0x11, 0x01, 0x02, 0x02, "Off"		},
	{0x11, 0x01, 0x02, 0x00, "On"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x11, 0x01, 0x0c, 0x0c, "2"		},
	{0x11, 0x01, 0x0c, 0x04, "3"		},
	{0x11, 0x01, 0x0c, 0x08, "4"		},
	{0x11, 0x01, 0x0c, 0x00, "5"		},

	{0   , 0xfe, 0   ,    4, "Coin_B"		},
	{0x11, 0x01, 0x30, 0x00, "4C_1C"		},
	{0x11, 0x01, 0x30, 0x20, "3C_1C"		},
	{0x11, 0x01, 0x30, 0x10, "2C_1C"		},
	{0x11, 0x01, 0x30, 0x30, "1C_1C"		},

	{0   , 0xfe, 0   ,    4, "Coin_A"		},
	{0x11, 0x01, 0xc0, 0xc0, "1C_1C"		},
	{0x11, 0x01, 0xc0, 0x00, "2C_4C"		},
	{0x11, 0x01, 0xc0, 0x40, "1C_2C"		},
	{0x11, 0x01, 0xc0, 0x80, "1C_3C"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x06, 0x02, "Easy"		},
	{0x12, 0x01, 0x06, 0x06, "Medium"		},
	{0x12, 0x01, 0x06, 0x04, "Hard"		},
	{0x12, 0x01, 0x06, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Demo_Sounds"		},
	{0x12, 0x01, 0x08, 0x00, "Off"		},
	{0x12, 0x01, 0x08, 0x08, "On"		},

	{0   , 0xfe, 0   ,    2, "Allow_Continue"		},
	{0x12, 0x01, 0x10, 0x00, "No"		},
	{0x12, 0x01, 0x10, 0x10, "Yes"		},

	{0   , 0xfe, 0   ,    8, "Bonus_Life"		},
	{0x12, 0x01, 0xe0, 0xe0, "20000 and every 50000"		},
	{0x12, 0x01, 0xe0, 0x60, "20000 and every 60000"		},
	{0x12, 0x01, 0xe0, 0xa0, "20000 and every 70000"		},
	{0x12, 0x01, 0xe0, 0x20, "30000 and every 60000"		},
	{0x12, 0x01, 0xe0, 0xc0, "30000 and every 70000"		},
	{0x12, 0x01, 0xe0, 0x40, "30000 and every 80000"		},
	{0x12, 0x01, 0xe0, 0x80, "40000 and every 100000"		},
	{0x12, 0x01, 0xe0, 0x00, "None"		},
};

STDDIPINFO(Lwingsb)

static struct BurnDIPInfo SectionzDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL		},
	{0x12, 0xff, 0xff, 0x3f, NULL		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x11, 0x01, 0x01, 0x01, "Off"		},
	{0x11, 0x01, 0x01, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Flip_Screen"		},
	{0x11, 0x01, 0x02, 0x02, "Off"		},
	{0x11, 0x01, 0x02, 0x00, "On"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x11, 0x01, 0x0c, 0x04, "2"		},
	{0x11, 0x01, 0x0c, 0x0c, "3"		},
	{0x11, 0x01, 0x0c, 0x08, "4"		},
	{0x11, 0x01, 0x0c, 0x00, "5"		},

	{0   , 0xfe, 0   ,    4, "Coin_A"		},
	{0x11, 0x01, 0x30, 0x00, "4C_1C"		},
	{0x11, 0x01, 0x30, 0x20, "3C_1C"		},
	{0x11, 0x01, 0x30, 0x10, "2C_1C"		},
	{0x11, 0x01, 0x30, 0x30, "1C_1C"		},

	{0   , 0xfe, 0   ,    4, "Coin_B"		},
	{0x11, 0x01, 0xc0, 0x00, "2C_1C"		},
	{0x11, 0x01, 0xc0, 0xc0, "1C_1C"		},
	{0x11, 0x01, 0xc0, 0x40, "1C_2C"		},
	{0x11, 0x01, 0xc0, 0x80, "1C_3C"		},

	{0   , 0xfe, 0   ,    2, "Allow_Continue"		},
	{0x12, 0x01, 0x01, 0x00, "No"		},
	{0x12, 0x01, 0x01, 0x01, "Yes"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x06, 0x02, "Easy"		},
	{0x12, 0x01, 0x06, 0x06, "Normal"		},
	{0x12, 0x01, 0x06, 0x04, "Hard"		},
	{0x12, 0x01, 0x06, 0x00, "Very_Hard"		},

	{0   , 0xfe, 0   ,    8, "Bonus_Life"		},
	{0x12, 0x01, 0x38, 0x38, "20000 50000"		},
	{0x12, 0x01, 0x38, 0x18, "20000 60000"		},
	{0x12, 0x01, 0x38, 0x28, "20000 70000"		},
	{0x12, 0x01, 0x38, 0x08, "30000 60000"		},
	{0x12, 0x01, 0x38, 0x30, "30000 70000"		},
	{0x12, 0x01, 0x38, 0x10, "30000 80000"		},
	{0x12, 0x01, 0x38, 0x20, "40000 100000"		},
	{0x12, 0x01, 0x38, 0x00, "None"		},

	{0   , 0xfe, 0   ,    3, "Cabinet"		},
	{0x12, 0x01, 0xc0, 0x00, "Upright One Player"		},
	{0x12, 0x01, 0xc0, 0x40, "Upright Two Players"		},
	{0x12, 0x01, 0xc0, 0xc0, "Cocktail"		},
};

STDDIPINFO(Sectionz)

static struct BurnDIPInfo TrojanlsDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x1c, NULL		},
	{0x12, 0xff, 0xff, 0xff, NULL		},

	{0   , 0xfe, 0   ,    3, "Cabinet"		},
	{0x11, 0x01, 0x03, 0x00, "Upright 1 Player"		},
	{0x11, 0x01, 0x03, 0x02, "Upright 2 Players"		},
	{0x11, 0x01, 0x03, 0x03, "Cocktail"		},

	{0   , 0xfe, 0   ,    8, "Bonus_Life"		},
	{0x11, 0x01, 0x1c, 0x10, "20000 60000"		},
	{0x11, 0x01, 0x1c, 0x0c, "20000 70000"		},
	{0x11, 0x01, 0x1c, 0x08, "20000 80000"		},
	{0x11, 0x01, 0x1c, 0x1c, "30000 60000"		},
	{0x11, 0x01, 0x1c, 0x18, "30000 70000"		},
	{0x11, 0x01, 0x1c, 0x14, "30000 80000"		},
	{0x11, 0x01, 0x1c, 0x04, "40000 80000"		},
	{0x11, 0x01, 0x1c, 0x00, "None"		},

	{0   , 0xfe, 0   ,    4, "Coin_A"		},
	{0x12, 0x01, 0x03, 0x00, "2C_1C"		},
	{0x12, 0x01, 0x03, 0x03, "1C_1C"		},
	{0x12, 0x01, 0x03, 0x02, "1C_2C"		},
	{0x12, 0x01, 0x03, 0x01, "1C_3C"		},

	{0   , 0xfe, 0   ,    4, "Coin_B"		},
	{0x12, 0x01, 0x0c, 0x00, "4C_1C"		},
	{0x12, 0x01, 0x0c, 0x04, "3C_1C"		},
	{0x12, 0x01, 0x0c, 0x08, "2C_1C"		},
	{0x12, 0x01, 0x0c, 0x0c, "1C_1C"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x12, 0x01, 0x30, 0x20, "2"		},
	{0x12, 0x01, 0x30, 0x30, "3"		},
	{0x12, 0x01, 0x30, 0x10, "4"		},
	{0x12, 0x01, 0x30, 0x00, "5"		},

	{0   , 0xfe, 0   ,    2, "Flip_Screen"		},
	{0x12, 0x01, 0x40, 0x40, "Off"		},
	{0x12, 0x01, 0x40, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Allow_Continue"		},
	{0x12, 0x01, 0x80, 0x00, "No"		},
	{0x12, 0x01, 0x80, 0x80, "Yes"		},
};

STDDIPINFO(Trojanls)

static struct BurnDIPInfo TrojanDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfc, NULL		},
	{0x12, 0xff, 0xff, 0xff, NULL		},

	{0   , 0xfe, 0   ,    3, "Cabinet"		},
	{0x11, 0x01, 0x03, 0x00, "Upright 1 Player"		},
	{0x11, 0x01, 0x03, 0x02, "Upright 2 Players"		},
	{0x11, 0x01, 0x03, 0x03, "Cocktail"		},

	{0   , 0xfe, 0   ,    8, "Bonus_Life"		},
	{0x11, 0x01, 0x1c, 0x10, "20000 60000"		},
	{0x11, 0x01, 0x1c, 0x0c, "20000 70000"		},
	{0x11, 0x01, 0x1c, 0x08, "20000 80000"		},
	{0x11, 0x01, 0x1c, 0x1c, "30000 60000"		},
	{0x11, 0x01, 0x1c, 0x18, "30000 70000"		},
	{0x11, 0x01, 0x1c, 0x14, "30000 80000"		},
	{0x11, 0x01, 0x1c, 0x04, "40000 80000"		},
	{0x11, 0x01, 0x1c, 0x00, "None"		},

	{0   , 0xfe, 0   ,    6, "Starting Level"		},
	{0x11, 0x01, 0xe0, 0xe0, "1"		},
	{0x11, 0x01, 0xe0, 0xc0, "2"		},
	{0x11, 0x01, 0xe0, 0xa0, "3"		},
	{0x11, 0x01, 0xe0, 0x80, "4"		},
	{0x11, 0x01, 0xe0, 0x60, "5"		},
	{0x11, 0x01, 0xe0, 0x40, "6"		},

	{0   , 0xfe, 0   ,    4, "Coin_A"		},
	{0x12, 0x01, 0x03, 0x00, "2C_1C"		},
	{0x12, 0x01, 0x03, 0x03, "1C_1C"		},
	{0x12, 0x01, 0x03, 0x02, "1C_2C"		},
	{0x12, 0x01, 0x03, 0x01, "1C_3C"		},

	{0   , 0xfe, 0   ,    4, "Coin_B"		},
	{0x12, 0x01, 0x0c, 0x00, "4C_1C"		},
	{0x12, 0x01, 0x0c, 0x04, "3C_1C"		},
	{0x12, 0x01, 0x0c, 0x08, "2C_1C"		},
	{0x12, 0x01, 0x0c, 0x0c, "1C_1C"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x12, 0x01, 0x30, 0x20, "2"		},
	{0x12, 0x01, 0x30, 0x30, "3"		},
	{0x12, 0x01, 0x30, 0x10, "4"		},
	{0x12, 0x01, 0x30, 0x00, "5"		},

	{0   , 0xfe, 0   ,    2, "Flip_Screen"		},
	{0x12, 0x01, 0x40, 0x40, "Off"		},
	{0x12, 0x01, 0x40, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Allow_Continue"		},
	{0x12, 0x01, 0x80, 0x00, "No"		},
	{0x12, 0x01, 0x80, 0x80, "Yes"		},
};

STDDIPINFO(Trojan)

static struct BurnDIPInfo AvengersDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL		},
	{0x12, 0xff, 0xff, 0xff, NULL		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x01, 0x01, "Off"		},
	{0x12, 0x01, 0x01, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Flip_Screen"		},
	{0x12, 0x01, 0x02, 0x02, "Off"		},
	{0x12, 0x01, 0x02, 0x00, "On"		},

	{0   , 0xfe, 0   ,    8, "Coin_B"		},
	{0x12, 0x01, 0x1c, 0x00, "4C_1C"		},
	{0x12, 0x01, 0x1c, 0x10, "3C_1C"		},
	{0x12, 0x01, 0x1c, 0x08, "2C_1C"		},
	{0x12, 0x01, 0x1c, 0x1c, "1C_1C"		},
	{0x12, 0x01, 0x1c, 0x0c, "1C_2C"		},
	{0x12, 0x01, 0x1c, 0x14, "1C_3C"		},
	{0x12, 0x01, 0x1c, 0x04, "1C_4C"		},
	{0x12, 0x01, 0x1c, 0x18, "1C_6C"		},

	{0   , 0xfe, 0   ,    8, "Coin_A"		},
	{0x12, 0x01, 0xe0, 0x00, "4C_1C"		},
	{0x12, 0x01, 0xe0, 0x80, "3C_1C"		},
	{0x12, 0x01, 0xe0, 0x40, "2C_1C"		},
	{0x12, 0x01, 0xe0, 0xe0, "1C_1C"		},
	{0x12, 0x01, 0xe0, 0x60, "1C_2C"		},
	{0x12, 0x01, 0xe0, 0xa0, "1C_3C"		},
	{0x12, 0x01, 0xe0, 0x20, "1C_4C"		},
	{0x12, 0x01, 0xe0, 0xc0, "1C_6C"		},

	{0   , 0xfe, 0   ,    2, "Allow_Continue"		},
	{0x11, 0x01, 0x01, 0x00, "No"		},
	{0x11, 0x01, 0x01, 0x01, "Yes"		},

	{0   , 0xfe, 0   ,    2, "Demo_Sounds"		},
	{0x11, 0x01, 0x02, 0x00, "Off"		},
	{0x11, 0x01, 0x02, 0x02, "On"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x11, 0x01, 0x0c, 0x04, "Easy"		},
	{0x11, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x11, 0x01, 0x0c, 0x08, "Hard"		},
	{0x11, 0x01, 0x0c, 0x00, "Very_Hard"		},

	{0   , 0xfe, 0   ,    4, "Bonus_Life"		},
	{0x11, 0x01, 0x30, 0x30, "20k 60k"		},
	{0x11, 0x01, 0x30, 0x10, "20k 70k"		},
	{0x11, 0x01, 0x30, 0x20, "20k 80k"		},
	{0x11, 0x01, 0x30, 0x00, "30k 80k"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x11, 0x01, 0xc0, 0xc0, "3"		},
	{0x11, 0x01, 0xc0, 0x40, "4"		},
	{0x11, 0x01, 0xc0, 0x80, "5"		},
	{0x11, 0x01, 0xc0, 0x00, "6"		},
};

STDDIPINFO(Avengers)



static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x020000;
	DrvZ80ROM1	= Next; Next += 0x008000;
	DrvZ80ROM2	= Next; Next += 0x010000;

	DrvTileMap	= Next; Next += 0x008000;

	DrvGfxROM0	= Next; Next += 0x020000;
	DrvGfxROM1	= Next; Next += 0x080000;
	DrvGfxROM2	= Next; Next += 0x080000;
	DrvGfxROM3	= Next; Next += 0x020000;

	DrvGfxMask	= Next; Next += 0x000020;

	DrvPalette	= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	AllRam		= Next;

	DrvZ80RAM0	= Next; Next += 0x002000;
	DrvZ80RAM1	= Next; Next += 0x000800;
	DrvPalRAM	= Next; Next += 0x000800;
	DrvFgRAM	= Next; Next += 0x000800;
	DrvBgRAM	= Next; Next += 0x000800;
	DrvSprRAM	= Next; Next += 0x000200;
	DrvSprBuf	= Next; Next += 0x000200;

	Palette		= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	ScrollX		= Next; Next += 0x000002;
	ScrollY		= Next; Next += 0x000002;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

// Avengers protection code ripped directly from MAME
static void avengers_protection_w(UINT8 data)
{
	INT32 pc = ZetPc(-1);

	if (pc == 0x2eeb)
	{
		avengers_param[0] = data;
	}
	else if (pc == 0x2f09)
	{
		avengers_param[1] = data;
	}
	else if (pc == 0x2f26)
	{
		avengers_param[2] = data;
	}
	else if (pc == 0x2f43)
	{
		avengers_param[3] = data;
	}
	else if (pc == 0x0445)
	{
		avengers_soundstate = 0x80;
		soundlatch = data;
	}
}

static INT32 avengers_fetch_paldata()
{
	static const char pal_data[] =
	// page 1: 0x03,0x02,0x01,0x00
	"0000000000000000" "A65486A6364676D6" "C764C777676778A7" "A574E5E5C5756AE5"
	"0000000000000000" "F51785D505159405" "A637B6A636269636" "F45744E424348824"
	"0000000000000000" "A33263B303330203" "4454848454440454" "A27242C232523632"
	"0000000000000000" "1253327202421102" "3386437373631373" "41A331A161715461"
	"0000000000000000" "1341715000711203" "4442635191622293" "5143D48383D37186"
	"0000000000000000" "2432423000412305" "6633343302333305" "7234A565A5A4A2A8"
	"0000000000000000" "46232422A02234A7" "88241624A21454A7" "A3256747A665D3AA"
	"0000000000000000" "070406020003050B" "0A05090504050508" "05060A090806040C"

	// page2: 0x07,0x06,0x05,0x04
	"0000000000000000" "2472030503230534" "6392633B23433B53" "0392846454346423"
	"0000000000000000" "1313052405050423" "3223754805354832" "323346A38686A332"
	"0000000000000000" "72190723070723D2" "81394776070776D1" "A15929F25959F2F1"
	"0000000000000000" "650706411A2A1168" "770737C43A3A3466" "87071F013C0C3175"
	"0000000000000000" "2001402727302020" "4403048F4A484344" "4A050B074E0E4440"
	"0000000000000000" "3003800C35683130" "5304035C587C5453" "5607080C5B265550"
	"0000000000000000" "4801D00043854245" "6C020038669A6569" "6604050A69446764"
	"0000000000000000" "0504000001030504" "0A05090504060307" "04090D0507010403"

	// page3: 0x0b,0x0a,0x09,0x08
	"0000000000000000" "685A586937F777F7" "988A797A67A7A7A7" "B8CA898DC737F787"
	"0000000000000000" "4738A61705150505" "8797672835250535" "7777072A25350525"
	"0000000000000000" "3525642404340404" "6554453554440454" "5544053634540434"
	"0000000000000000" "2301923203430303" "4333834383630373" "3324034473730363"
	"0000000000000000" "3130304000762005" "5352525291614193" "6463635483D06581"
	"0000000000000000" "4241415100483107" "6463631302335304" "76757415A5A077A3"
	"0000000000000000" "53525282A02A43AA" "76747424A31565A5" "88888536A66089A4"
	"0000000000000000" "05040304000D050C" "0806050604070707" "0A0A060808000C06"

	// page4: 0x0f,0x0e,0x0d,0x0c
	"0000000000000000" "3470365956342935" "5590578997554958" "73C078A8C573687A"
	"0000000000000000" "5355650685030604" "2427362686042607" "010A070584010508"
	"0000000000000000" "0208432454022403" "737A243455733406" "000D050353000307"
	"0000000000000000" "000A023233003202" "424C134234424204" "000F241132001105"
	"0000000000000000" "3031113030300030" "5152215252512051" "7273337374723272"
	"0000000000000000" "4141214041411041" "6263326363623162" "8385448585834383"
	"0000000000000000" "5153225152512051" "7375437475734273" "9598559697946495"
	"0000000000000000" "0205020303020102" "0407040606040304" "060A060809060506"

	// page5: 0x13,0x12,0x11,0x10
	"0000000000000000" "4151D141D3D177F7" "5454C44482C4A7A7" "0404D45491D4F787"
	"0000000000000000" "0303032374230505" "9696962673560535" "0505054502850525"
	"0000000000000000" "0303030355030404" "7777770754470454" "0606060603760434"
	"0000000000000000" "0505053547050303" "4949492945390373" "0808083804580363"
	"0000000000000000" "0B0C444023442005" "3D3F333433334193" "0000043504046581"
	"0000000000000000" "0809565085863107" "0B6A352374455304" "00700644050677A3"
	"0000000000000000" "06073879C8C843AA" "09492739A58765A5" "0050084A060889A4"
	"0000000000000000" "05060B070B0B050C" "0707090707090707" "00000B08070B0C06"

	// page6: 0x17,0x16,0x15,0x14
	"0000000000000000" "0034308021620053" "0034417042512542" "0034526064502E31"
	"0000000000000000" "0106412032733060" "11A6522053628350" "22A6632072620D42"
	"0000000000000000" "1308223052242080" "2478233071235170" "3578243090230960"
	"0000000000000000" "2111334333331404" "3353324232324807" "45B5314131310837"
	"0000000000000000" "3232445444445302" "445443534343B725" "567642524242B745"
	"0000000000000000" "4343556555550201" "5575546454540524" "6787536353537554"
	"0000000000000000" "6474667676660100" "7696657575650423" "88A8647474645473"
	"0000000000000000" "0001070701050004" "0003060603040303" "0005050505040302";

	INT32 bank = avengers_palette_pen/64;
	INT32 offs = avengers_palette_pen%64;
	INT32 page = bank/4;				// 0..7
	INT32 base = (3-(bank&3));			// 0..3
	INT32 row = offs&0xf;				// 0..15
	INT32 col = offs/16 + base*4;			// 0..15
	INT32 digit0 = pal_data[page*256*2 + (31-row*2)*16+col];
	INT32 digit1 = pal_data[page*256*2 + (30-row*2)*16+col];
	INT32 result;

	if( digit0>='A' ) digit0 += 10 - 'A'; else digit0 -= '0';
	if( digit1>='A' ) digit1 += 10 - 'A'; else digit1 -= '0';
	result = digit0 * 16 + digit1;

	if( (avengers_palette_pen&0x3f)!=0x3f ) avengers_palette_pen++;

	return result;
}

static UINT8 avengers_protection_r()
{
	static const INT32 xpos[8] = { 10, 7,  0, -7, -10, -7,   0,  7 };
	static const INT32 ypos[8] = {  0, 7, 10,  7,   0, -7, -10, -7 };
	INT32 best_dist = 0;
	INT32 best_dir = 0;
	INT32 x,y;
	INT32 dx,dy,dist,dir;

	if(ZetPc(-1) == 0x7c7 )
	{
		// palette data
		return avengers_fetch_paldata();
	}

	//  int to Angle Function
	//
        //	Input: two cartesian points
        //	Output: direction code (north,northeast,east,...)
	//
	x = avengers_param[0] - avengers_param[2];
	y = avengers_param[1] - avengers_param[3];
	for( dir=0; dir<8; dir++ )
	{
		dx = xpos[dir]-x;
		dy = ypos[dir]-y;
		dist = dx*dx+dy*dy;
		if( dist < best_dist || dir==0 )
		{
			best_dir = dir;
			best_dist = dist;
		}
	}
	return best_dir<<5;
}



UINT8 __fastcall lwings_main_read(UINT16 address)
{
	switch (address)
	{
		case 0xf808:
		case 0xf809:
		case 0xf80a:
			return DrvInp[address - 0xf808];

		case 0xf80b:
		case 0xf80c:
			return DrvDip[address - 0xf80b];

		case 0xf80d:
			return avengers_protection_r();
	}

	return 0;
}

static void lwings_bankswitch_w(UINT8 data)
{
	DrvZ80Bank = data;

	INT32 bankaddress = 0x10000 + ((data >> 1) & 3) * 0x4000;

	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM0 + bankaddress);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM0 + bankaddress);
}

void __fastcall lwings_main_write(UINT16 address, UINT8 data)
{
	if ((address & 0xf800) == 0xf000) {
		DrvPalRAM[address & 0x7ff] = data;

		UINT8 r, g, b;
		UINT16 coldata = DrvPalRAM[(address & 0x3ff) | 0x400] | (DrvPalRAM[address & 0x3ff] << 8);

		r = (coldata >> 8) & 0xf0;
		g = (coldata >> 4) & 0xf0;
		b = (coldata >> 0) & 0xf0;

		r |= r >> 4;
		g |= g >> 4;
		b |= b >> 4;

		Palette[address & 0x3ff] = (r << 16) | (g << 8) | b;
		DrvPalette[address & 0x3ff] = BurnHighCol(r, g, b, 0);

		return;
	}

	// hack
	if (avengers && (address & 0xfff8) == 0xf808) address += 0x10;

	switch (address)
	{
		case 0xf800:
		case 0xf801:
		case 0xf808:
		case 0xf809:
			ScrollX[address & 1] = data;
		return;

		case 0xf802:
		case 0xf803:
		case 0xf80a:
		case 0xf80b:
			ScrollY[address & 1] = data;
		return;

		case 0xf804:
			trojan_bg2_scrollx = data;
		return;

		case 0xf805:
			trojan_bg2_image = data;
		return;

		case 0xf80c:
			soundlatch = data;
		return;

		case 0xf80d:
			soundlatch2 = data;
		return;

		case (0xf80e + 0x10):
		case 0xf80e:
			lwings_bankswitch_w(data);

			flipscreen = ~data & 0x01;

			interrupt_enable = data & 0x08;
		return;

		case (0xf809 + 0x10):
			avengers_protection_w(data);
		return;

		case (0xf80c + 0x10):
			avengers_palette_pen = data << 6;
		return;

		case (0xf80d + 0x10):
			soundlatch2 = data;
		return;
	}
}

void __fastcall lwings_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe000:
		case 0xe001:
		case 0xe002:
		case 0xe003:
			BurnYM2203Write((address & 2) >> 1, address & 1, data);
		return;

		case 0xe006:
			avengers_soundlatch2 = data;
		return;
	}
}

UINT8 __fastcall lwings_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xc800:
			return soundlatch;

		case 0xe006:
			UINT8 Data = avengers_soundlatch2 | avengers_soundstate;
			avengers_soundstate = 0;
			return Data;
	}

	return 0;
}

void __fastcall trojan_adpcm_out(UINT16 port, UINT8 data)
{
	if ((port & 0xff) == 0x01) {
		MSM5205ResetWrite(0, (data >> 7) & 1);
		MSM5205DataWrite(0, data);
		MSM5205VCLKWrite(0, 1);
		MSM5205VCLKWrite(0, 0);
	}
}

UINT8 __fastcall trojan_adpcm_in(UINT16 port)
{
	port &= 0xff;

	UINT8 ret = 0;

	if (port == 0x00) {
			ret = soundlatch2;
	}

	return ret;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	for (INT32 i = 0; i < 2; i++) {
		ZetOpen(i);
		ZetReset();
		if (i == 0) lwings_bankswitch_w(0);
		ZetClose();
	}
	
	if (MSM5205InUse) {
		ZetOpen(2);
		ZetReset();
		ZetClose();
	}

	BurnYM2203Reset();

	if (MSM5205InUse) MSM5205Reset();

	trojan_bg2_scrollx = 0;
	trojan_bg2_image = 0;

	memset(avengers_param, 0, 4);
	avengers_palette_pen = 0;

	avengers_soundlatch2 = 0;
	avengers_soundstate = 0;

	DrvZ80Bank = 0;
	flipscreen = 0;
	interrupt_enable = 0;
	soundlatch = 0;
	soundlatch2 = 0;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[2]  = { 0x000000, 0x000004 };
	INT32 Plane1[4]  = { 0x080004, 0x080000, 0x000004, 0x000000 };
	INT32 Plane1a[4] = { 0x100004, 0x100000, 0x000004, 0x000000 };
	INT32 Plane2[4]  = { 0x180000, 0x100000, 0x080000, 0x000000 };
	INT32 Plane3[4]  = { 0x040000, 0x040004, 0x000000, 0x000004 };

	// sprite, char
	INT32 XOffs0[16] = { 0, 1, 2, 3, 8, 9, 10, 11, 256, 257, 258, 259, 264, 265, 266, 267 };
	INT32 YOffs0[16] = { 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240 };

	// background
	INT32 XOffs1[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
	INT32 YOffs1[16] = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x40000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0800, 2,  8,  8, Plane0, XOffs0, YOffs0, 0x080, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x40000);

	GfxDecode(0x0800, 4, 16, 16, Plane2, XOffs1, YOffs1, 0x100, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x40000);

	if (DrvTileMap != NULL) {
		GfxDecode(0x0800, 4, 16, 16, Plane1a, XOffs0, YOffs0, 0x200, tmp, DrvGfxROM2);

		memcpy (tmp, DrvGfxROM3, 0x10000);

		GfxDecode(0x0200, 4, 16, 16, Plane3, XOffs0, YOffs0, 0x200, tmp, DrvGfxROM3);
	} else {
		GfxDecode(0x0400, 4, 16, 16, Plane1, XOffs0, YOffs0, 0x200, tmp, DrvGfxROM2);
	}

	BurnFree (tmp);

	return 0;
}

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)(ZetTotalCycles() * nSoundRate / 3000000);
}

inline static INT32 DrvMSM5205SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)((double)ZetTotalCycles() * nSoundRate / (nCyclesTotal[0] * 60));
}

inline static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 3000000;
}

static void lwings_main_cpu_init()
{
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM0);
	// 8000 - bfff banked
	ZetMapArea(0xc000, 0xddff, 0, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xddff, 1, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xddff, 2, DrvZ80RAM0);

	ZetMapArea(0xde00, 0xdfff, 0, DrvSprRAM);
	ZetMapArea(0xde00, 0xdfff, 1, DrvSprRAM);
	ZetMapArea(0xde00, 0xdfff, 2, DrvSprRAM);

	ZetMapArea(0xe000, 0xe7ff, 0, DrvFgRAM);
	ZetMapArea(0xe000, 0xe7ff, 1, DrvFgRAM);
	ZetMapArea(0xe000, 0xe7ff, 2, DrvFgRAM);

	ZetMapArea(0xe800, 0xefff, 0, DrvBgRAM);
	ZetMapArea(0xe800, 0xefff, 1, DrvBgRAM);
	ZetMapArea(0xe800, 0xefff, 2, DrvBgRAM);

	ZetMapArea(0xf000, 0xf7ff, 0, DrvPalRAM);
//	ZetMapArea(0xf000, 0xf7ff, 1, DrvPalRAM); // write handler
	ZetMapArea(0xf000, 0xf7ff, 2, DrvPalRAM);
	ZetSetReadHandler(lwings_main_read);
	ZetSetWriteHandler(lwings_main_write);
	ZetMemEnd();
	ZetClose();
}

static void lwings_sound_init()
{
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM1);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80RAM1);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80RAM1);
	ZetSetReadHandler(lwings_sound_read);
	ZetSetWriteHandler(lwings_sound_write);
	ZetMemEnd();
	ZetClose();

	BurnYM2203Init(2, 1500000, NULL, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(3000000);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.10, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_YM2203_ROUTE, 0.10, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_1, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_2, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_3, 0.20, BURN_SND_ROUTE_BOTH);
}

static INT32 DrvInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	DrvTileMap = NULL;

	{
		if (BurnLoadRom(DrvZ80ROM0 + 0x00000, 0, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM0 + 0x10000, 1, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x18000, 2, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM1 + 0x00000, 3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000, 4, 1)) return 1;

		for (INT32 i = 0; i < 8; i++) {
			if (BurnLoadRom(DrvGfxROM1 + i * 0x8000, i + 5, 1)) return 1;
		}

		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(DrvGfxROM2 + i * 0x8000, i + 13, 1)) return 1;
		}

		DrvGfxDecode();
	}

	ZetInit(0);
	lwings_main_cpu_init();

	ZetInit(1);
	lwings_sound_init();

	GenericTilesInit();
	
	nCyclesTotal[0] = 6000000 / 60;
	nCyclesTotal[1] = 3000000 / 60;
	nCyclesTotal[2] = 0;

	DrvDoReset();

	return 0;
}

static INT32 TrojanInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvZ80ROM0 + 0x00000, 0, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x10000, 1, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x18000, 2, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM1 + 0x00000, 3, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM2 + 0x00000, 4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000, 5, 1)) return 1;

		for (INT32 i = 0; i < 8; i++) {
			if (BurnLoadRom(DrvGfxROM1 + i * 0x8000, i + 6, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + i * 0x8000, i + 14, 1)) return 1;
		}

		if (BurnLoadRom(DrvGfxROM3 + 0x0000, 22, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x8000, 23, 1)) return 1;

		if (BurnLoadRom(DrvTileMap, 24, 1)) return 1;

		DrvGfxDecode();

		{
			for (INT32 i = 0; i < 32; i++) {
				DrvGfxMask[i] = (0xf07f0001 & (1 << i)) ? 1 : 0;
			}
		}
	}

	ZetInit(0);
	lwings_main_cpu_init();

	ZetInit(1);
	lwings_sound_init();

	ZetInit(2);
	ZetOpen(2);
	ZetMapArea(0x0000, 0xffff, 0, DrvZ80ROM2);
	ZetMapArea(0x0000, 0xffff, 2, DrvZ80ROM2);
	ZetSetInHandler(trojan_adpcm_in);
	ZetSetOutHandler(trojan_adpcm_out);
	ZetMemEnd();
	ZetClose();
	
	MSM5205Init(0, DrvMSM5205SynchroniseStream, 455000, NULL, MSM5205_SEX_4B, 1);
	MSM5205SetRoute(0, 0.50, BURN_SND_ROUTE_BOTH);
	MSM5205InUse = 1;

	GenericTilesInit();
	
	nCyclesTotal[0] = 3000000 / 60;
	nCyclesTotal[1] = 3000000 / 60;
	nCyclesTotal[2] = 3000000 / 60;

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();
	BurnYM2203Exit();

	BurnFree (AllMem);

	avengers = 0;
	MSM5205InUse = 0;

	return 0;
}

static INT32 TrojanExit()
{
	MSM5205Exit();
	return DrvExit();
}

static void draw_foreground(INT32 colbase)
{
	for (INT32 offs = 0x20; offs < 0x3e0; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		INT32 color = DrvFgRAM[offs | 0x400];
		INT32 code = DrvFgRAM[offs] | ((color & 0xc0) << 2);

		INT32 flipx = color & 0x10;
		INT32 flipy = color & 0x20;

		color &= 0x0f;

		sy -= 8;

		if (flipy) {
			if (flipx) {
				Render8x8Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 2, 0x03, colbase, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 2, 0x03, colbase, DrvGfxROM0);
			}
		} else {
			if (flipx) {
				Render8x8Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 2, 0x03, colbase, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 2, 0x03, colbase, DrvGfxROM0);
			}
		}
	}
}

static void draw_background()
{
	INT32 scrollx = (ScrollX[0] | (ScrollX[1] << 8)) & 0x1ff;
	INT32 scrolly = (ScrollY[0] | (ScrollY[1] << 8)) & 0x1ff;

	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 sy = (offs & 0x1f) << 4;
		INT32 sx = (offs >> 5) << 4;
		    sy -= 8;

		sx -= scrollx;
		sy -= scrolly;
		if (sx < -15) sx += 512;
		if (sy < -15) sy += 512;

		if (sy < -15 || sx < -15 || sy >= nScreenHeight || sx >= nScreenWidth)
			continue;

		INT32 color = DrvBgRAM[offs | 0x400];
		INT32 code = DrvBgRAM[offs] | (color & 0xe0) << 3;;

		INT32 flipx = color & 0x08;
		INT32 flipy = color & 0x10;

		color &= 0x07;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM1);
			} else {
				Render16x16Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM1);
			} else {
				Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM1);
			}
		}
	}
}

static void lwings_draw_sprites()
{
	for (INT32 offs = 0x200 - 4; offs >= 0; offs -= 4)
	{
		INT32 sx = DrvSprBuf[offs + 3] - 0x100 * (DrvSprBuf[offs + 1] & 0x01);
		INT32 sy = DrvSprBuf[offs + 2];

		if (sy && sx)
		{
			INT32 code,color,flipx,flipy;

			if (sy > 0xf8) sy-=0x100;

			code  = DrvSprBuf[offs] | ((DrvSprBuf[offs + 1] & 0xc0) << 2);
			color = (DrvSprBuf[offs + 1] & 0x38) >> 3;
			flipx = DrvSprBuf[offs + 1] & 0x02;
			flipy = DrvSprBuf[offs + 1] & 0x04;

			color += 0x18;

		    	sy -= 8;

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0, DrvGfxROM2);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0, DrvGfxROM2);
				}
			}
		}
	}
}


static void draw_16x16_with_mask(INT32 sx, INT32 sy, INT32 code, INT32 color, UINT8 *gfxbase, UINT8 *mask, INT32 flipx, INT32 flipy)
{
	UINT8 *src = gfxbase + (code << 8);

	color = (color << 4) | 0x100;

	if (flipy) {
		src += 0xf0;

		if (flipx) {
			for (INT32 y = 15; y >= 0; y--, src-=16)
			{
				INT32 yy = sy + y;
				if (yy < 0) break;
				if (yy >= nScreenHeight) continue;
		
				for (INT32 x = 15; x >= 0; x--)
				{
					INT32 xx = sx + x;
					if (xx < 0) break;
					if (xx >= nScreenWidth) continue;	
		
					INT32 o = color | src[15-x];
					if (mask[src[15-x]]) continue;
		
					pTransDraw[(yy * nScreenWidth) + xx] = o;
				}
			}
		} else {
			for (INT32 y = 15; y >= 0; y--, src-=16)
			{
				INT32 yy = sy + y;
				if (yy < 0) break;
				if (yy >= nScreenHeight) continue;
		
				for (INT32 x = 0; x < 16; x++)
				{
					INT32 xx = sx + x;
					if (xx < 0) continue;
					if (xx >= nScreenWidth) break;	
		
					INT32 o = color | src[x];
					if (mask[src[x]]) continue;
		
					pTransDraw[(yy * nScreenWidth) + xx] = o;
				}
			}
		}
	} else {
		if (flipx) {
			for (INT32 y = 0; y < 16; y++, src+=16)
			{
				INT32 yy = sy + y;
				if (yy < 0) continue;
				if (yy >= nScreenHeight) break;

				for (INT32 x = 15; x >= 0; x--)
				{
					INT32 xx = sx + x;
					if (xx < 0) break;
					if (xx >= nScreenWidth) continue;	
		
					INT32 o = color | src[15-x];
					if (mask[src[15-x]]) continue;

					pTransDraw[(yy * nScreenWidth) + xx] = o;
				}
			}
		} else {
			for (INT32 y = 0; y < 16; y++, src+=16)
			{
				INT32 yy = sy + y;
				if (yy < 0) continue;
				if (yy >= nScreenHeight) break;

				for (INT32 x = 0; x < 16; x++)
				{
					INT32 xx = sx + x;
					if (xx < 0) continue;
					if (xx >= nScreenWidth) break;	
		
					INT32 o = color | src[x];
					if (mask[src[x]]) continue;

					pTransDraw[(yy * nScreenWidth) + xx] = o;
				}
			}
		}
	}
}

static void trojan_draw_background(INT32 priority)
{
	INT32 scrollx = (ScrollX[0] | (ScrollX[1] << 8)) & 0x1ff;
	INT32 scrolly = (ScrollY[0] | (ScrollY[1] << 8)) & 0x1ff;

	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 color = DrvBgRAM[offs | 0x400];
		if (priority && ((~color >> 3) & 1)) continue;

		INT32 sy = (offs & 0x1f) << 4;
		INT32 sx = (offs >> 5) << 4;
		    sy -= 8;

		sx -= scrollx;
		sy -= scrolly;
		if (sx < -15) sx += 512;
		if (sy < -15) sy += 512;

		if (sy < -15 || sx < -15 || sy >= nScreenHeight || sx >= nScreenWidth)
			continue;

		INT32 code = DrvBgRAM[offs] | ((color & 0xe0) << 3);
		INT32 flipx = color & 0x10;
		INT32 flipy = 0;

		color &= 0x07;

		if (avengers) color ^= 6;

		draw_16x16_with_mask(sx, sy, code, color, DrvGfxROM1, DrvGfxMask + priority * 16, flipx, flipy);
/*
		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0x00, 0x100, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0x00, 0x100, DrvGfxROM1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0x00, 0x100, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0x00, 0x100, DrvGfxROM1);
			}
		}
*/
	}
}

static void trojan_draw_background2()
{
	for (INT32 offs = 0; offs < 32 * 16; offs++)
	{
		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		sx -= trojan_bg2_scrollx;
		if (sx < -15) sx += 512;
		    sy -= 8;

		if (sy < -15 || sx < -15 || sy >= nScreenHeight || sx >= nScreenWidth)
			continue;

		INT32 offset = ((((offs << 6) & 0x7800) | ((offs << 1) & 0x3e)) + (trojan_bg2_image << 5)) & 0x7fff;

		INT32 color = DrvTileMap[offset + 1];
		INT32 code  = DrvTileMap[offset + 0] | ((color & 0x80) << 1);
		INT32 flipx = color & 0x10;
		INT32 flipy = color & 0x20;

		color &= 7;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM3);
			} else {
				Render16x16Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM3);
			}
		} else {
			if (flipx) {
				Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM3);
			} else {
				Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM3);
			}
		}
	}
}

static void trojan_draw_sprites()
{
	for (INT32 offs = 0x180 - 4; offs >= 0; offs -= 4)
	{
		INT32 sx = DrvSprBuf[offs + 3] - 0x100 * (DrvSprBuf[offs + 1] & 0x01);
		INT32 sy = DrvSprBuf[offs + 2];

		if (sy && sx)
		{
			INT32 flipx, flipy;

			if (sy > 0xf8) sy-=0x100;

			INT32 color = DrvSprBuf[offs + 1];
			INT32 code  = DrvSprBuf[offs] | ((color & 0x20) << 4) | ((color & 0x40) << 2) | ((color & 0x80) << 3);

			if (avengers)
			{
				flipx = 0;
				flipy = ~color & 0x10;
			}
			else
			{
				flipx = color & 0x10;
				flipy = 1;
			}

			color = ((color >> 1) & 7) + 0x28;

		   	sy -= 8;

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0, DrvGfxROM2);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0, DrvGfxROM2);
				}
			}
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

	if (DrvTileMap == NULL) {
		draw_background();
		lwings_draw_sprites();
		draw_foreground(0x200);
	} else {
		trojan_draw_background2();
		trojan_draw_background(0);
		trojan_draw_sprites();
		trojan_draw_background(1);
		draw_foreground(0x300);
	}

	if (flipscreen) {
		UINT16 *ptr = pTransDraw + (nScreenWidth * nScreenHeight) - 1;

		for (INT32 i = 0; i < nScreenWidth * nScreenHeight / 2; i++, ptr--)
		{
			INT32 n = pTransDraw[i];
			pTransDraw[i] = *ptr;
			*ptr = n;
		}
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
		memset (DrvInp, 0xff, 3);

		for (INT32 i = 0; i < 8; i++) {
			DrvInp[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInp[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInp[2] ^= (DrvJoy3[i] & 1) << i;
		}

		if ((DrvInp[1] & 0x03) == 0) DrvInp[1] |= 0x03;
		if ((DrvInp[1] & 0x0c) == 0) DrvInp[1] |= 0x0c;
		if ((DrvInp[2] & 0x03) == 0) DrvInp[2] |= 0x03;
		if ((DrvInp[2] & 0x0c) == 0) DrvInp[2] |= 0x0c;
	}

	INT32 nInterleave;
	INT32 MSMIRQSlice[67];
	
	if (MSM5205InUse) {
		nInterleave = MSM5205CalcInterleave(0, 6000000);
	
		for (INT32 i = 0; i < 67; i++) {
			MSMIRQSlice[i] = (INT32)((double)((nInterleave * (i + 1)) / 68));
		}
	} else {
		nInterleave = 16;
	}

	INT32 nCyclesDone[3] = { 0, 0, 0 };
	
	ZetNewFrame();

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nCurrentCPU, nNext, nCyclesSegment;

		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (interrupt_enable && i == (nInterleave - 1)) {
			if (avengers & 1) {
				ZetNmi();
			} else {
				ZetSetVector(0xd7);
				ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
			}
		}
		if (MSM5205InUse) MSM5205Update();
		ZetClose();
		
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		BurnTimerUpdate(i * (nCyclesTotal[nCurrentCPU] / nInterleave));
		if ((i % (nInterleave / 4)) == ((nInterleave / 4) - 1)) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		ZetClose();

		if (MSM5205InUse) {
			nCurrentCPU = 2;
			ZetOpen(nCurrentCPU);
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
	
	ZetOpen(1);
	BurnTimerEndFrame(nCyclesTotal[1]);
	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		if (MSM5205InUse) MSM5205Render(0, pBurnSoundOut, nBurnSoundLen);
	}
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	memcpy (DrvSprBuf, DrvSprRAM, 0x200);

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029692;
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

		BurnYM2203Scan(nAction, pnMin);
		if (MSM5205InUse) MSM5205Scan(nAction, pnMin);

		SCAN_VAR(interrupt_enable);
		SCAN_VAR(soundlatch);
		SCAN_VAR(soundlatch2);
		SCAN_VAR(flipscreen);
		SCAN_VAR(DrvZ80Bank);

		SCAN_VAR(*((UINT32*)avengers_param));
		SCAN_VAR(avengers_palette_pen);
		SCAN_VAR(avengers_soundlatch2);
		SCAN_VAR(avengers_soundstate);
		SCAN_VAR(trojan_bg2_scrollx);
		SCAN_VAR(trojan_bg2_image);
	}

	ZetOpen(0);
	lwings_bankswitch_w(DrvZ80Bank);
	ZetClose();

	return 0;
}


// Section Z (set 1)

static struct BurnRomInfo sectionzRomDesc[] = {
	{ "6c_sz01.bin",	0x8000, 0x69585125, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "7c_sz02.bin",	0x8000, 0x22f161b8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "9c_sz03.bin",	0x8000, 0x4c7111ed, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "11e_sz04.bin",	0x8000, 0xa6073566, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "9h_sz05.bin",	0x4000, 0x3173ba2e, 4 | BRF_GRA },           //  4 Characters

	{ "3e_sz14.bin",	0x8000, 0x63782e30, 5 | BRF_GRA },           //  5 Background Layer 1 Tiles
	{ "1e_sz08.bin",	0x8000, 0xd57d9f13, 5 | BRF_GRA },           //  6
	{ "3d_sz13.bin",	0x8000, 0x1b3d4d7f, 5 | BRF_GRA },           //  7
	{ "1d_sz07.bin",	0x8000, 0xf5b3a29f, 5 | BRF_GRA },           //  8
	{ "3b_sz12.bin",	0x8000, 0x11d47dfd, 5 | BRF_GRA },           //  9
	{ "1b_sz06.bin",	0x8000, 0xdf703b68, 5 | BRF_GRA },           // 10
	{ "3f_sz15.bin",	0x8000, 0x36bb9bf7, 5 | BRF_GRA },           // 11
	{ "1f_sz09.bin",	0x8000, 0xda8f06c9, 5 | BRF_GRA },           // 12

	{ "3j_sz17.bin",	0x8000, 0x8df7b24a, 6 | BRF_GRA },           // 13 Sprites
	{ "1j_sz11.bin",	0x8000, 0x685d4c54, 6 | BRF_GRA },           // 14
	{ "3h_sz16.bin",	0x8000, 0x500ff2bb, 6 | BRF_GRA },           // 15
	{ "1h_sz10.bin",	0x8000, 0x00b3d244, 6 | BRF_GRA },           // 16

	{ "mb7114e.15g",	0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 17 Proms (not used)
};

STD_ROM_PICK(sectionz)
STD_ROM_FN(sectionz)

struct BurnDriver BurnDrvSectionz = {
	"sectionz", NULL, NULL, NULL, "1985",
	"Section Z (set 1)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARWARE_CAPCOM_MISC, GBF_HORSHOOT, 0,
	NULL, sectionzRomInfo, sectionzRomName, NULL, NULL, DrvInputInfo, SectionzDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// Section Z (set 2)

static struct BurnRomInfo sctionzaRomDesc[] = {
	{ "sz-01a.bin",		0x8000, 0x98df49fd, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "7c_sz02.bin",	0x8000, 0x22f161b8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sz-03j.bin",		0x8000, 0x94547abf, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "11e_sz04.bin",	0x8000, 0xa6073566, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "9h_sz05.bin",	0x4000, 0x3173ba2e, 4 | BRF_GRA },           //  4 Characters

	{ "3e_sz14.bin",	0x8000, 0x63782e30, 5 | BRF_GRA },           //  5 Background Layer 1 Tiles
	{ "1e_sz08.bin",	0x8000, 0xd57d9f13, 5 | BRF_GRA },           //  6
	{ "3d_sz13.bin",	0x8000, 0x1b3d4d7f, 5 | BRF_GRA },           //  7
	{ "1d_sz07.bin",	0x8000, 0xf5b3a29f, 5 | BRF_GRA },           //  8
	{ "3b_sz12.bin",	0x8000, 0x11d47dfd, 5 | BRF_GRA },           //  9
	{ "1b_sz06.bin",	0x8000, 0xdf703b68, 5 | BRF_GRA },           // 10
	{ "3f_sz15.bin",	0x8000, 0x36bb9bf7, 5 | BRF_GRA },           // 11
	{ "1f_sz09.bin",	0x8000, 0xda8f06c9, 5 | BRF_GRA },           // 12

	{ "3j_sz17.bin",	0x8000, 0x8df7b24a, 6 | BRF_GRA },           // 13 Sprites
	{ "1j_sz11.bin",	0x8000, 0x685d4c54, 6 | BRF_GRA },           // 14
	{ "3h_sz16.bin",	0x8000, 0x500ff2bb, 6 | BRF_GRA },           // 15
	{ "1h_sz10.bin",	0x8000, 0x00b3d244, 6 | BRF_GRA },           // 16

	{ "mb7114e.15g",	0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 17 Proms (not used)
};

STD_ROM_PICK(sctionza)
STD_ROM_FN(sctionza)

struct BurnDriver BurnDrvSctionza = {
	"sectionza", "sectionz", NULL, NULL, "1985",
	"Section Z (set 2)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_HORSHOOT, 0,
	NULL, sctionzaRomInfo, sctionzaRomName, NULL, NULL, DrvInputInfo, SectionzDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// Legendary Wings (US set 1)

static struct BurnRomInfo lwingsRomDesc[] = {
	{ "6c_lw01.bin",	0x8000, 0xb55a7f60, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "7c_lw02.bin",	0x8000, 0xa5efbb1b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "9c_lw03.bin",	0x8000, 0xec5cc201, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "11e_lw04.bin",	0x8000, 0xa20337a2, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "9h_lw05.bin",	0x4000, 0x091d923c, 4 | BRF_GRA },           //  4 Characters

	{ "3e_lw14.bin",	0x8000, 0x5436392c, 5 | BRF_GRA },           //  5 Background Layer 1 Tiles
	{ "1e_lw08.bin",	0x8000, 0xb491bbbb, 5 | BRF_GRA },           //  6
	{ "3d_lw13.bin",	0x8000, 0xfdd1908a, 5 | BRF_GRA },           //  7
	{ "1d_lw07.bin",	0x8000, 0x5c73d406, 5 | BRF_GRA },           //  8
	{ "3b_lw12.bin",	0x8000, 0x32e17b3c, 5 | BRF_GRA },           //  9
	{ "1b_lw06.bin",	0x8000, 0x52e533c1, 5 | BRF_GRA },           // 10
	{ "3f_lw15.bin",	0x8000, 0x99e134ba, 5 | BRF_GRA },           // 11
	{ "1f_lw09.bin",	0x8000, 0xc8f28777, 5 | BRF_GRA },           // 12

	{ "3j_lw17.bin",	0x8000, 0x5ed1bc9b, 6 | BRF_GRA },           // 13 Sprites
	{ "1j_lw11.bin",	0x8000, 0x2a0790d6, 6 | BRF_GRA },           // 14
	{ "3h_lw16.bin",	0x8000, 0xe8834006, 6 | BRF_GRA },           // 15
	{ "1h_lw10.bin",	0x8000, 0xb693f5a5, 6 | BRF_GRA },           // 16

	{ "63s141.15g",		0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 17 Proms (not used)
};

STD_ROM_PICK(lwings)
STD_ROM_FN(lwings)

struct BurnDriver BurnDrvLwings = {
	"lwings", NULL, NULL, NULL, "1986",
	"Legendary Wings (US set 1)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, lwingsRomInfo, lwingsRomName, NULL, NULL, DrvInputInfo, LwingsDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Legendary Wings (US set 2)

static struct BurnRomInfo lwings2RomDesc[] = {
	{ "u13-l",		0x8000, 0x3069c01c, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "u14-k",		0x8000, 0x5d91c828, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "9c_lw03.bin",	0x8000, 0xec5cc201, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "11e_lw04.bin",	0x8000, 0xa20337a2, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "9h_lw05.bin",	0x4000, 0x091d923c, 4 | BRF_GRA },           //  4 Characters

	{ "b_03e.rom",		0x8000, 0x176e3027, 5 | BRF_GRA },           //  5 Background Layer 1 Tiles
	{ "b_01e.rom",		0x8000, 0xf5d25623, 5 | BRF_GRA },           //  6
	{ "b_03d.rom",		0x8000, 0x001caa35, 5 | BRF_GRA },           //  7
	{ "b_01d.rom",		0x8000, 0x0ba008c3, 5 | BRF_GRA },           //  8
	{ "b_03b.rom",		0x8000, 0x4f8182e9, 5 | BRF_GRA },           //  9
	{ "b_01b.rom",		0x8000, 0xf1617374, 5 | BRF_GRA },           // 10
	{ "b_03f.rom",		0x8000, 0x9b374dcc, 5 | BRF_GRA },           // 11
	{ "b_01f.rom",		0x8000, 0x23654e0a, 5 | BRF_GRA },           // 12

	{ "b_03j.rom",		0x8000, 0x8f3c763a, 6 | BRF_GRA },           // 13 Sprites
	{ "b_01j.rom",		0x8000, 0x7cc90a1d, 6 | BRF_GRA },           // 14
	{ "b_03h.rom",		0x8000, 0x7d58f532, 6 | BRF_GRA },           // 15
	{ "b_01h.rom",		0x8000, 0x3e396eda, 6 | BRF_GRA },           // 16

	{ "63s141.15g",		0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 17 Proms (not used)
};

STD_ROM_PICK(lwings2)
STD_ROM_FN(lwings2)

struct BurnDriver BurnDrvLwings2 = {
	"lwings2", "lwings", NULL, NULL, "1986",
	"Legendary Wings (US set 2)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, lwings2RomInfo, lwings2RomName, NULL, NULL, DrvInputInfo, LwingsDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Ares no Tsubasa (Japan)

static struct BurnRomInfo lwingsjpRomDesc[] = {
	{ "a_06c.rom",		0x8000, 0x2068a738, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "a_07c.rom",		0x8000, 0xd6a2edc4, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "9c_lw03.bin",	0x8000, 0xec5cc201, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "11e_lw04.bin",	0x8000, 0xa20337a2, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "9h_lw05.bin",	0x4000, 0x091d923c, 4 | BRF_GRA },           //  4 Characters

	{ "b_03e.rom",		0x8000, 0x176e3027, 5 | BRF_GRA },           //  5 Background Layer 1 Tiles
	{ "b_01e.rom",		0x8000, 0xf5d25623, 5 | BRF_GRA },           //  6
	{ "b_03d.rom",		0x8000, 0x001caa35, 5 | BRF_GRA },           //  7
	{ "b_01d.rom",		0x8000, 0x0ba008c3, 5 | BRF_GRA },           //  8
	{ "b_03b.rom",		0x8000, 0x4f8182e9, 5 | BRF_GRA },           //  9
	{ "b_01b.rom",		0x8000, 0xf1617374, 5 | BRF_GRA },           // 10
	{ "b_03f.rom",		0x8000, 0x9b374dcc, 5 | BRF_GRA },           // 11
	{ "b_01f.rom",		0x8000, 0x23654e0a, 5 | BRF_GRA },           // 12

	{ "b_03j.rom",		0x8000, 0x8f3c763a, 6 | BRF_GRA },           // 13 Sprites
	{ "b_01j.rom",		0x8000, 0x7cc90a1d, 6 | BRF_GRA },           // 14
	{ "b_03h.rom",		0x8000, 0x7d58f532, 6 | BRF_GRA },           // 15
	{ "b_01h.rom",		0x8000, 0x3e396eda, 6 | BRF_GRA },           // 16

	{ "63s141.15g",		0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 17 Proms (not used)
};

STD_ROM_PICK(lwingsjp)
STD_ROM_FN(lwingsjp)

struct BurnDriver BurnDrvLwingsjp = {
	"lwingsj", "lwings", NULL, NULL, "1986",
	"Ares no Tsubasa (Japan)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, lwingsjpRomInfo, lwingsjpRomName, NULL, NULL, DrvInputInfo, LwingsDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Legendary Wings (bootleg)

static struct BurnRomInfo lwingsbRomDesc[] = {
	{ "ic17.bin",		0x8000, 0xfe8a8823, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "ic18.bin",		0x8000, 0x2a00cde8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ic19.bin",		0x8000, 0xec5cc201, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "ic37.bin",		0x8000, 0xa20337a2, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "ic60.bin",		0x4000, 0x091d923c, 4 | BRF_GRA },           //  4 Characters

	{ "ic50.bin",		0x8000, 0x5436392c, 5 | BRF_GRA },           //  5 Background Layer 1 Tiles
	{ "ic49.bin",		0x8000, 0xffdbdd69, 5 | BRF_GRA },           //  6
	{ "ic26.bin",		0x8000, 0xfdd1908a, 5 | BRF_GRA },           //  7
	{ "ic25.bin",		0x8000, 0x5c73d406, 5 | BRF_GRA },           //  8
	{ "ic2.bin",		0x8000, 0x32e17b3c, 5 | BRF_GRA },           //  9
	{ "ic1.bin",		0x8000, 0x52e533c1, 5 | BRF_GRA },           // 10
	{ "ic63.bin",		0x8000, 0x99e134ba, 5 | BRF_GRA },           // 11
	{ "ic62.bin",		0x8000, 0xc8f28777, 5 | BRF_GRA },           // 12

	{ "ic99.bin",		0x8000, 0x163946da, 6 | BRF_GRA },           // 13 Sprites
	{ "ic98.bin",		0x8000, 0x7cc90a1d, 6 | BRF_GRA },           // 14
	{ "ic87.bin",		0x8000, 0xbca275ac, 6 | BRF_GRA },           // 15
	{ "ic86.bin",		0x8000, 0x3e396eda, 6 | BRF_GRA },           // 16

	{ "63s141.15g",		0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 17 Proms (not used)
};

STD_ROM_PICK(lwingsb)
STD_ROM_FN(lwingsb)

struct BurnDriver BurnDrvLwingsb = {
	"lwingsb", "lwings", NULL, NULL, "1986",
	"Legendary Wings (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, lwingsbRomInfo, lwingsbRomName, NULL, NULL, DrvInputInfo, LwingsbDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Trojan (US set 1)

static struct BurnRomInfo trojanRomDesc[] = {
	{ "t4.10n",			0x8000, 0xc1bbeb4e, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "t6.13n",			0x8000, 0xd49592ef, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "tb05.12n",		0x8000, 0x9273b264, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "tb02.15h",		0x8000, 0x21154797, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "tb01.6d",		0x4000, 0x1c0f91b2, 3 | BRF_PRG | BRF_ESS }, //  4 Z80 #2 Code

	{ "tb03.8k",		0x4000, 0x581a2b4c, 4 | BRF_GRA },           //  5 Characters

	{ "tb13.6b",		0x8000, 0x285a052b, 5 | BRF_GRA },           //  6 Background Layer 1 Tiles
	{ "tb09.6a",		0x8000, 0xaeb693f7, 5 | BRF_GRA },           //  7
	{ "tb12.4b",		0x8000, 0xdfb0fe5c, 5 | BRF_GRA },           //  8
	{ "tb08.4a",		0x8000, 0xd3a4c9d1, 5 | BRF_GRA },           //  9
	{ "tb11.3b",		0x8000, 0x00f0f4fd, 5 | BRF_GRA },           // 10
	{ "tb07.3a",		0x8000, 0xdff2ee02, 5 | BRF_GRA },           // 11
	{ "tb14.8b",		0x8000, 0x14bfac18, 5 | BRF_GRA },           // 12
	{ "tb10.8a",		0x8000, 0x71ba8a6d, 5 | BRF_GRA },           // 13

	{ "tb18.7l",		0x8000, 0x862c4713, 6 | BRF_GRA },           // 14 Sprites
	{ "tb16.3l",		0x8000, 0xd86f8cbd, 6 | BRF_GRA },           // 15
	{ "tb17.5l",		0x8000, 0x12a73b3f, 6 | BRF_GRA },           // 16
	{ "tb15.2l",		0x8000, 0xbb1a2769, 6 | BRF_GRA },           // 17
	{ "tb22.7n",		0x8000, 0x39daafd4, 6 | BRF_GRA },           // 18
	{ "tb20.3n",		0x8000, 0x94615d2a, 6 | BRF_GRA },           // 19
	{ "tb21.5n",		0x8000, 0x66c642bd, 6 | BRF_GRA },           // 20
	{ "tb19.2n",		0x8000, 0x81d5ab36, 6 | BRF_GRA },           // 21

	{ "tb25.15n",		0x8000, 0x6e38c6fa, 7 | BRF_GRA },           // 22 Background Layer 2 Tiles
	{ "tb24.13n",		0x8000, 0x14fc6cf2, 7 | BRF_GRA },           // 23

	{ "tb23.9n",		0x8000, 0xeda13c0e, 8 | BRF_GRA },           // 24 Background Layer 2 Tile Map

	{ "tbp24s10.7j",	0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 25 Proms (not used)
	{ "mb7114e.1e",		0x0100, 0x5052fa9d, 0 | BRF_OPT },           // 26
};

STD_ROM_PICK(trojan)
STD_ROM_FN(trojan)

struct BurnDriver BurnDrvTrojan = {
	"trojan", NULL, NULL, NULL, "1986",
	"Trojan (US set 1)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM | GBF_SCRFIGHT, 0,
	NULL, trojanRomInfo, trojanRomName, NULL, NULL, DrvInputInfo, TrojanlsDIPInfo,
	TrojanInit, TrojanExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// Trojan (US set 2)

static struct BurnRomInfo trojanaRomDesc[] = {
	{ "tb4.10n",		0x8000, 0x0113a551, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "tb6.13n",		0x8000, 0xaa127a5b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "tb05.12n",		0x8000, 0x9273b264, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "tb02.15h",		0x8000, 0x21154797, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "tb01.6d",		0x4000, 0x1c0f91b2, 3 | BRF_PRG | BRF_ESS }, //  4 Z80 #2 Code

	{ "tb03.8k",		0x4000, 0x581a2b4c, 4 | BRF_GRA },           //  5 Characters

	{ "tb13.6b",		0x8000, 0x285a052b, 5 | BRF_GRA },           //  6 Background Layer 1 Tiles
	{ "tb09.6a",		0x8000, 0xaeb693f7, 5 | BRF_GRA },           //  7
	{ "tb12.4b",		0x8000, 0xdfb0fe5c, 5 | BRF_GRA },           //  8
	{ "tb08.4a",		0x8000, 0xd3a4c9d1, 5 | BRF_GRA },           //  9
	{ "tb11.3b",		0x8000, 0x00f0f4fd, 5 | BRF_GRA },           // 10
	{ "tb07.3a",		0x8000, 0xdff2ee02, 5 | BRF_GRA },           // 11
	{ "tb14.8b",		0x8000, 0x14bfac18, 5 | BRF_GRA },           // 12
	{ "tb10.8a",		0x8000, 0x71ba8a6d, 5 | BRF_GRA },           // 13

	{ "tb18.7l",		0x8000, 0x862c4713, 6 | BRF_GRA },           // 14 Sprites
	{ "tb16.3l",		0x8000, 0xd86f8cbd, 6 | BRF_GRA },           // 15
	{ "tb17.5l",		0x8000, 0x12a73b3f, 6 | BRF_GRA },           // 16
	{ "tb15.2l",		0x8000, 0xbb1a2769, 6 | BRF_GRA },           // 17
	{ "tb22.7n",		0x8000, 0x39daafd4, 6 | BRF_GRA },           // 18
	{ "tb20.3n",		0x8000, 0x94615d2a, 6 | BRF_GRA },           // 19
	{ "tb21.5n",		0x8000, 0x66c642bd, 6 | BRF_GRA },           // 20
	{ "tb19.2n",		0x8000, 0x81d5ab36, 6 | BRF_GRA },           // 21

	{ "tb25.15n",		0x8000, 0x6e38c6fa, 7 | BRF_GRA },           // 22 Background Layer 2 Tiles
	{ "tb24.13n",		0x8000, 0x14fc6cf2, 7 | BRF_GRA },           // 23

	{ "tb23.9n",		0x8000, 0xeda13c0e, 8 | BRF_GRA },           // 24 Background Layer 2 Tile Map

	{ "tbp24s10.7j",	0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 25 Proms (not used)
	{ "mb7114e.1e",		0x0100, 0x5052fa9d, 0 | BRF_OPT },           // 26
};

STD_ROM_PICK(trojana)
STD_ROM_FN(trojana)

struct BurnDriver BurnDrvTrojana = {
	"trojana", "trojan", NULL, NULL, "1986",
	"Trojan (US set 2)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM | GBF_SCRFIGHT, 0,
	NULL, trojanaRomInfo, trojanaRomName, NULL, NULL, DrvInputInfo, TrojanlsDIPInfo,
	TrojanInit, TrojanExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// Trojan (Romstar)

static struct BurnRomInfo trojanrRomDesc[] = {
	{ "tb04.10n",		0x8000, 0x92670f27, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "tb06.13n",		0x8000, 0xa4951173, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "tb05.12n",		0x8000, 0x9273b264, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "tb02.15h",		0x8000, 0x21154797, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "tb01.6d",		0x4000, 0x1c0f91b2, 3 | BRF_PRG | BRF_ESS }, //  4 Z80 #2 Code

	{ "tb03.8k",		0x4000, 0x581a2b4c, 4 | BRF_GRA },           //  5 Characters

	{ "tb13.6b",		0x8000, 0x285a052b, 5 | BRF_GRA },           //  6 Background Layer 1 Tiles
	{ "tb09.6a",		0x8000, 0xaeb693f7, 5 | BRF_GRA },           //  7
	{ "tb12.4b",		0x8000, 0xdfb0fe5c, 5 | BRF_GRA },           //  8
	{ "tb08.4a",		0x8000, 0xd3a4c9d1, 5 | BRF_GRA },           //  9
	{ "tb11.3b",		0x8000, 0x00f0f4fd, 5 | BRF_GRA },           // 10
	{ "tb07.3a",		0x8000, 0xdff2ee02, 5 | BRF_GRA },           // 11
	{ "tb14.8b",		0x8000, 0x14bfac18, 5 | BRF_GRA },           // 12
	{ "tb10.8a",		0x8000, 0x71ba8a6d, 5 | BRF_GRA },           // 13

	{ "tb18.7l",		0x8000, 0x862c4713, 6 | BRF_GRA },           // 14 Sprites
	{ "tb16.3l",		0x8000, 0xd86f8cbd, 6 | BRF_GRA },           // 15
	{ "tb17.5l",		0x8000, 0x12a73b3f, 6 | BRF_GRA },           // 16
	{ "tb15.2l",		0x8000, 0xbb1a2769, 6 | BRF_GRA },           // 17
	{ "tb22.7n",		0x8000, 0x39daafd4, 6 | BRF_GRA },           // 18
	{ "tb20.3n",		0x8000, 0x94615d2a, 6 | BRF_GRA },           // 19
	{ "tb21.5n",		0x8000, 0x66c642bd, 6 | BRF_GRA },           // 20
	{ "tb19.2n",		0x8000, 0x81d5ab36, 6 | BRF_GRA },           // 21

	{ "tb25.15n",		0x8000, 0x6e38c6fa, 7 | BRF_GRA },           // 22 Background Layer 2 Tiles
	{ "tb24.13n",		0x8000, 0x14fc6cf2, 7 | BRF_GRA },           // 23

	{ "tb23.9n",		0x8000, 0xeda13c0e, 8 | BRF_GRA },           // 24 Background Layer 2 Tile Map

	{ "tbp24s10.7j",	0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 25 Proms (not used)
	{ "mb7114e.1e",		0x0100, 0x5052fa9d, 0 | BRF_OPT },           // 26
};

STD_ROM_PICK(trojanr)
STD_ROM_FN(trojanr)

struct BurnDriver BurnDrvTrojanr = {
	"trojanr", "trojan", NULL, NULL, "1986",
	"Trojan (Romstar)\0", NULL, "Capcom (Romstar license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM | GBF_SCRFIGHT, 0,
	NULL, trojanrRomInfo, trojanrRomName, NULL, NULL, DrvInputInfo, TrojanDIPInfo,
	TrojanInit, TrojanExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// Tatakai no Banka (Japan)

static struct BurnRomInfo trojanjRomDesc[] = {
	{ "troj-04.10n",	0x8000, 0x0b5a7f49, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "troj-06.13n",	0x8000, 0xdee6ed92, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "tb05.12n",		0x8000, 0x9273b264, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "tb02.15h",		0x8000, 0x21154797, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "tb01.6d",		0x4000, 0x1c0f91b2, 3 | BRF_PRG | BRF_ESS }, //  4 Z80 #2 Code

	{ "tb03.8k",		0x4000, 0x581a2b4c, 4 | BRF_GRA },           //  5 Characters

	{ "tb13.6b",		0x8000, 0x285a052b, 5 | BRF_GRA },           //  6 Background Layer 1 Tiles
	{ "tb09.6a",		0x8000, 0xaeb693f7, 5 | BRF_GRA },           //  7
	{ "tb12.4b",		0x8000, 0xdfb0fe5c, 5 | BRF_GRA },           //  8
	{ "tb08.4a",		0x8000, 0xd3a4c9d1, 5 | BRF_GRA },           //  9
	{ "tb11.3b",		0x8000, 0x00f0f4fd, 5 | BRF_GRA },           // 10
	{ "tb07.3a",		0x8000, 0xdff2ee02, 5 | BRF_GRA },           // 11
	{ "tb14.8b",		0x8000, 0x14bfac18, 5 | BRF_GRA },           // 12
	{ "tb10.8a",		0x8000, 0x71ba8a6d, 5 | BRF_GRA },           // 13

	{ "tb18.7l",		0x8000, 0x862c4713, 6 | BRF_GRA },           // 14 Sprites
	{ "tb16.3l",		0x8000, 0xd86f8cbd, 6 | BRF_GRA },           // 15
	{ "tb17.5l",		0x8000, 0x12a73b3f, 6 | BRF_GRA },           // 16
	{ "tb15.2l",		0x8000, 0xbb1a2769, 6 | BRF_GRA },           // 17
	{ "tb22.7n",		0x8000, 0x39daafd4, 6 | BRF_GRA },           // 18
	{ "tb20.3n",		0x8000, 0x94615d2a, 6 | BRF_GRA },           // 19
	{ "tb21.5n",		0x8000, 0x66c642bd, 6 | BRF_GRA },           // 20
	{ "tb19.2n",		0x8000, 0x81d5ab36, 6 | BRF_GRA },           // 21

	{ "tb25.15n",		0x8000, 0x6e38c6fa, 7 | BRF_GRA },           // 22 Background Layer 2 Tiles
	{ "tb24.13n",		0x8000, 0x14fc6cf2, 7 | BRF_GRA },           // 23

	{ "tb23.9n",		0x8000, 0xeda13c0e, 8 | BRF_GRA },           // 24 Background Layer 2 Tile Map

	{ "tbp24s10.7j",	0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 25 Proms (not used)
	{ "mb7114e.1e",		0x0100, 0x5052fa9d, 0 | BRF_OPT },           // 26
};

STD_ROM_PICK(trojanj)
STD_ROM_FN(trojanj)

struct BurnDriver BurnDrvTrojanj = {
	"trojanj", "trojan", NULL, NULL, "1986",
	"Tatakai no Banka (Japan)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM | GBF_SCRFIGHT, 0,
	NULL, trojanjRomInfo, trojanjRomName, NULL, NULL, DrvInputInfo, TrojanDIPInfo,
	TrojanInit, TrojanExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// Avengers (US set 1)

static struct BurnRomInfo avengersRomDesc[] = {
	{ "04.10n",		0x8000, 0xa94aadcc, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "06.13n",		0x8000, 0x39cd80bd, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "05.12n",		0x8000, 0x06b1cec9, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "02.15h",		0x8000, 0x107a2e17, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "01.6d",		0x8000, 0xc1e5d258, 3 | BRF_PRG | BRF_ESS }, //  4 Z80 #2 Code

	{ "03.8k",		0x8000, 0xefb5883e, 4 | BRF_GRA },           //  5 Characters

	{ "13.6b",		0x8000, 0x9b5ff305, 5 | BRF_GRA },           //  6 Background Layer 1 Tiles
	{ "09.6a",		0x8000, 0x08323355, 5 | BRF_GRA },           //  7
	{ "12.4b",		0x8000, 0x6d5261ba, 5 | BRF_GRA },           //  8
	{ "08.4a",		0x8000, 0xa13d9f54, 5 | BRF_GRA },           //  9
	{ "11.3b",		0x8000, 0xa2911d8b, 5 | BRF_GRA },           // 10
	{ "07.3a",		0x8000, 0xcde78d32, 5 | BRF_GRA },           // 11
	{ "14.8b",		0x8000, 0x44ac2671, 5 | BRF_GRA },           // 12
	{ "10.8a",		0x8000, 0xb1a717cb, 5 | BRF_GRA },           // 13

	{ "18.7l",		0x8000, 0x3c876a17, 6 | BRF_GRA },           // 14 Sprites
	{ "16.3l",		0x8000, 0x4b1ff3ac, 6 | BRF_GRA },           // 15
	{ "17.5l",		0x8000, 0x4eb543ef, 6 | BRF_GRA },           // 16
	{ "15.2l",		0x8000, 0x8041de7f, 6 | BRF_GRA },           // 17
	{ "22.7n",		0x8000, 0xbdaa8b22, 6 | BRF_GRA },           // 18
	{ "20.3n",		0x8000, 0x566e3059, 6 | BRF_GRA },           // 19
	{ "21.5n",		0x8000, 0x301059aa, 6 | BRF_GRA },           // 20
	{ "19.2n",		0x8000, 0xa00485ec, 6 | BRF_GRA },           // 21

	{ "25.15n",		0x8000, 0x230d9e30, 7 | BRF_GRA },           // 22 Background Layer 2 Tiles
	{ "24.13n",		0x8000, 0xa6354024, 7 | BRF_GRA },           // 23

	{ "23.9n",		0x8000, 0xc0a93ef6, 8 | BRF_GRA },           // 24 Background Layer 2 Tile Map

	{ "tbb_2bpr.7j",	0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 25 Proms (not used)
	{ "tbb_1bpr.1e",	0x0100, 0x5052fa9d, 0 | BRF_OPT },           // 26
};

STD_ROM_PICK(avengers)
STD_ROM_FN(avengers)

static INT32 AvengersInit()
{
	avengers = 1;

	return TrojanInit();
}

struct BurnDriver BurnDrvAvengers = {
	"avengers", NULL, NULL, NULL, "1987",
	"Avengers (US set 1)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARWARE_CAPCOM_MISC, GBF_SCRFIGHT, 0,
	NULL, avengersRomInfo, avengersRomName, NULL, NULL, DrvInputInfo, AvengersDIPInfo,
	AvengersInit, TrojanExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Avengers (US set 2)

static struct BurnRomInfo avenger2RomDesc[] = {
	{ "avg4.bin",		0x8000, 0x0fea7ac5, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "av_06a.13n",		0x8000, 0x491a712c, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "av_05.12n",		0x8000, 0x9a214b42, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "02.15h",		0x8000, 0x107a2e17, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "01.6d",		0x8000, 0xc1e5d258, 3 | BRF_PRG | BRF_ESS }, //  4 Z80 #2 Code

	{ "03.8k",		0x8000, 0xefb5883e, 4 | BRF_GRA },           //  5 Characters

	{ "13.6b",		0x8000, 0x9b5ff305, 5 | BRF_GRA },           //  6 Background Layer 1 Tiles
	{ "09.6a",		0x8000, 0x08323355, 5 | BRF_GRA },           //  7
	{ "12.4b",		0x8000, 0x6d5261ba, 5 | BRF_GRA },           //  8
	{ "08.4a",		0x8000, 0xa13d9f54, 5 | BRF_GRA },           //  9
	{ "11.3b",		0x8000, 0xa2911d8b, 5 | BRF_GRA },           // 10
	{ "07.3a",		0x8000, 0xcde78d32, 5 | BRF_GRA },           // 11
	{ "14.8b",		0x8000, 0x44ac2671, 5 | BRF_GRA },           // 12
	{ "10.8a",		0x8000, 0xb1a717cb, 5 | BRF_GRA },           // 13

	{ "18.7l",		0x8000, 0x3c876a17, 6 | BRF_GRA },           // 14 Sprites
	{ "16.3l",		0x8000, 0x4b1ff3ac, 6 | BRF_GRA },           // 15
	{ "17.5l",		0x8000, 0x4eb543ef, 6 | BRF_GRA },           // 16
	{ "15.2l",		0x8000, 0x8041de7f, 6 | BRF_GRA },           // 17
	{ "22.7n",		0x8000, 0xbdaa8b22, 6 | BRF_GRA },           // 18
	{ "20.3n",		0x8000, 0x566e3059, 6 | BRF_GRA },           // 19
	{ "21.5n",		0x8000, 0x301059aa, 6 | BRF_GRA },           // 20
	{ "19.2n",		0x8000, 0xa00485ec, 6 | BRF_GRA },           // 21

	{ "25.15n",		0x8000, 0x230d9e30, 7 | BRF_GRA },           // 22 Background Layer 2 Tiles
	{ "24.13n",		0x8000, 0xa6354024, 7 | BRF_GRA },           // 23

	{ "23.9n",		0x8000, 0xc0a93ef6, 8 | BRF_GRA },           // 24 Background Layer 2 Tile Map

	{ "tbb_2bpr.7j",	0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 25 Proms (not used)
	{ "tbb_1bpr.1e",	0x0100, 0x5052fa9d, 0 | BRF_OPT },           // 26
};

STD_ROM_PICK(avenger2)
STD_ROM_FN(avenger2)

struct BurnDriver BurnDrvAvenger2 = {
	"avengers2", "avengers", NULL, NULL, "1987",
	"Avengers (US set 2)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARWARE_CAPCOM_MISC, GBF_SCRFIGHT, 0,
	NULL, avenger2RomInfo, avenger2RomName, NULL, NULL, DrvInputInfo, AvengersDIPInfo,
	AvengersInit, TrojanExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Hissatsu Buraiken (Japan)

static struct BurnRomInfo buraikenRomDesc[] = {
	{ "av_04a.10n",		0x8000, 0x361fc614, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "av_06a.13n",		0x8000, 0x491a712c, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "av_05.12n",		0x8000, 0x9a214b42, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "02.15h",		0x8000, 0x107a2e17, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "01.6d",		0x8000, 0xc1e5d258, 3 | BRF_PRG | BRF_ESS }, //  4 Z80 #2 Code

	{ "03.8k",		0x8000, 0xefb5883e, 4 | BRF_GRA },           //  5 Characters

	{ "13.6b",		0x8000, 0x9b5ff305, 5 | BRF_GRA },           //  6 Background Layer 1 Tiles
	{ "09.6a",		0x8000, 0x08323355, 5 | BRF_GRA },           //  7
	{ "12.4b",		0x8000, 0x6d5261ba, 5 | BRF_GRA },           //  8
	{ "08.4a",		0x8000, 0xa13d9f54, 5 | BRF_GRA },           //  9
	{ "11.3b",		0x8000, 0xa2911d8b, 5 | BRF_GRA },           // 10
	{ "07.3a",		0x8000, 0xcde78d32, 5 | BRF_GRA },           // 11
	{ "14.8b",		0x8000, 0x44ac2671, 5 | BRF_GRA },           // 12
	{ "10.8a",		0x8000, 0xb1a717cb, 5 | BRF_GRA },           // 13

	{ "18.7l",		0x8000, 0x3c876a17, 6 | BRF_GRA },           // 14 Sprites
	{ "16.3l",		0x8000, 0x4b1ff3ac, 6 | BRF_GRA },           // 15
	{ "17.5l",		0x8000, 0x4eb543ef, 6 | BRF_GRA },           // 16
	{ "15.2l",		0x8000, 0x8041de7f, 6 | BRF_GRA },           // 17
	{ "22.7n",		0x8000, 0xbdaa8b22, 6 | BRF_GRA },           // 18
	{ "20.3n",		0x8000, 0x566e3059, 6 | BRF_GRA },           // 19
	{ "21.5n",		0x8000, 0x301059aa, 6 | BRF_GRA },           // 20
	{ "19.2n",		0x8000, 0xa00485ec, 6 | BRF_GRA },           // 21

	{ "av_25.15n",		0x8000, 0x88a505a7, 7 | BRF_GRA },           // 22 Background Layer 2 Tiles
	{ "av_24.13n",		0x8000, 0x1f4463c8, 7 | BRF_GRA },           // 23

	{ "23.9n",		0x8000, 0xc0a93ef6, 8 | BRF_GRA },           // 24 Background Layer 2 Tile Map

	{ "tbb_2bpr.7j",	0x0100, 0xd96bcc98, 0 | BRF_OPT },           // 25 Proms (not used)
	{ "tbb_1bpr.1e",	0x0100, 0x5052fa9d, 0 | BRF_OPT },           // 26
};

STD_ROM_PICK(buraiken)
STD_ROM_FN(buraiken)

struct BurnDriver BurnDrvBuraiken = {
	"buraiken", "avengers", NULL, NULL, "1987",
	"Hissatsu Buraiken (Japan)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARWARE_CAPCOM_MISC, GBF_SCRFIGHT, 0,
	NULL, buraikenRomInfo, buraikenRomName, NULL, NULL, DrvInputInfo, AvengersDIPInfo,
	AvengersInit, TrojanExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};
