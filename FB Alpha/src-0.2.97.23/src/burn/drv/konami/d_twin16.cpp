// FB Alpha Twin16 driver module
// Based on MAME driver by Phil Stroffolino

#include "tiles_generic.h"
#include "burn_ym2151.h"
#include "upd7759.h"
#include "k007232.h"

static UINT8 *AllMem;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *MemEnd;
static UINT8 *Drv68KROM0;
static UINT8 *Drv68KROM1;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *DrvShareRAM;
static UINT8 *Drv68KRAM0;
static UINT8 *DrvPalRAM;
static UINT8 *DrvVidRAM2;
static UINT8 *DrvVidRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprBuf;
static UINT8 *DrvNvRAM;
static UINT8 *DrvNvRAMBank;
static UINT8 *Drv68KRAM1;
static UINT8 *DrvFgRAM;
static UINT8 *DrvTileRAM;
static UINT8 *DrvSprGfxRAM;
static UINT8 *DrvGfxExp;

static UINT8 *DrvZ80RAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT16 *scrollx;
static UINT16 *scrolly;

static UINT8 *soundlatch;
static UINT8 *soundlatch2;

static INT32 video_register;
static INT32 gfx_bank;

static INT32 twin16_CPUA_register;
static INT32 twin16_CPUB_register;

static INT32 need_process_spriteram;
static INT32 twin16_custom_video;
static INT32 is_vulcan = 0;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvJoy4[16];
static UINT8 DrvInputs[4];
static UINT8 DrvDips[3];
static UINT8 DrvReset;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 6,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Drv)

static struct BurnInputInfo DevilwInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 6,	"service"	},
	{"Map Button",		BIT_DIGITAL,	DrvJoy1 + 3,	"service2"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Devilw)

static struct BurnInputInfo DarkadvInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 fire 3"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy1 + 2,	"p3 coin"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy4 + 2,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy4 + 3,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy4 + 0,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy4 + 1,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy4 + 4,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy4 + 5,	"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	DrvJoy4 + 6,	"p3 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service 1",		BIT_DIGITAL,	DrvJoy1 + 6,	"service"	},
	{"Service 2",		BIT_DIGITAL,	DrvJoy1 + 7,	"service2"	},
	{"Service 3",		BIT_DIGITAL,	DrvJoy2 + 7,	"service3"	},
	{"Map Button",		BIT_DIGITAL,	DrvJoy1 + 3,	"service4"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Darkadv)

#define coinage_dips(offs)					\
	{0   , 0xfe, 0   ,    16, "Coin A"		},	\
	{offs, 0x01, 0x0f, 0x02, "4 Coins 1 Credit "	},	\
	{offs, 0x01, 0x0f, 0x05, "3 Coins 1 Credit "	},	\
	{offs, 0x01, 0x0f, 0x08, "2 Coins 1 Credit "	},	\
	{offs, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},	\
	{offs, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},	\
	{offs, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit "	},	\
	{offs, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},	\
	{offs, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},	\
	{offs, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},	\
	{offs, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},	\
	{offs, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},	\
	{offs, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},	\
	{offs, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},	\
	{offs, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},	\
	{offs, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},	\
	{offs, 0x01, 0x0f, 0x00, "Free Play"		},	\
								\
	{0   , 0xfe, 0   ,    16, "Coin B"		},	\
	{offs, 0x01, 0xf0, 0x20, "4 Coins 1 Credit "	},	\
	{offs, 0x01, 0xf0, 0x50, "3 Coins 1 Credit "	},	\
	{offs, 0x01, 0xf0, 0x80, "2 Coins 1 Credit "	},	\
	{offs, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},	\
	{offs, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},	\
	{offs, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit "	},	\
	{offs, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},	\
	{offs, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},	\
	{offs, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},	\
	{offs, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},	\
	{offs, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},	\
	{offs, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},	\
	{offs, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},	\
	{offs, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},	\
	{offs, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},	\
	{offs, 0x01, 0xf0, 0x00, "Disabled"		},

static struct BurnDIPInfo DevilwDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0x5e, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	coinage_dips(0x13)

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x03, 0x03, "2"			},
	{0x14, 0x01, 0x03, 0x02, "3"			},
	{0x14, 0x01, 0x03, 0x01, "5"			},
	{0x14, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x60, 0x60, "Easy"			},
	{0x14, 0x01, 0x60, 0x40, "Normal"		},
	{0x14, 0x01, 0x60, 0x20, "Difficult"		},
	{0x14, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x80, 0x80, "Off"			},
	{0x14, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x15, 0x01, 0x01, 0x01, "Off"			},
	{0x15, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x15, 0x01, 0x04, 0x04, "Off"			},
	{0x15, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Devilw)

static struct BurnDIPInfo DarkadvDIPList[]=
{
	{0x1d, 0xff, 0xff, 0xff, NULL			},
	{0x1e, 0xff, 0xff, 0x5e, NULL			},
	{0x1f, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    16, "Coinage"		},
	{0x1d, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 	},
	{0x1d, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 	},
	{0x1d, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 	},
	{0x1d, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x1d, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x1d, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 	},
	{0x1d, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x1d, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x1d, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x1d, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x1d, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x1d, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x1d, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x1d, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x1d, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x1d, 0x01, 0x0f, 0x00, "Invalid"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x1e, 0x01, 0x03, 0x03, "2"			},
	{0x1e, 0x01, 0x03, 0x02, "3"			},
	{0x1e, 0x01, 0x03, 0x01, "5"			},
	{0x1e, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x1e, 0x01, 0x60, 0x60, "Easy"			},
	{0x1e, 0x01, 0x60, 0x40, "Normal"		},
	{0x1e, 0x01, 0x60, 0x20, "Difficult"		},
	{0x1e, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x1e, 0x01, 0x80, 0x80, "Off"			},
	{0x1e, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x1f, 0x01, 0x01, 0x01, "Off"			},
	{0x1f, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x1f, 0x01, 0x04, 0x04, "Off"			},
	{0x1f, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Darkadv)

static struct BurnDIPInfo VulcanDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0x5a, NULL			},
	{0x16, 0xff, 0xff, 0xfd, NULL			},

	coinage_dips(0x14)

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x03, "2"			},
	{0x15, 0x01, 0x03, 0x02, "3"			},
	{0x15, 0x01, 0x03, 0x01, "4"			},
	{0x15, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x15, 0x01, 0x04, 0x00, "Upright"		},
	{0x15, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x15, 0x01, 0x18, 0x18, "20K 70K"		},
	{0x15, 0x01, 0x18, 0x10, "30K 80K"		},
	{0x15, 0x01, 0x18, 0x08, "20K"			},
	{0x15, 0x01, 0x18, 0x00, "70K"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x60, 0x60, "Easy"			},
	{0x15, 0x01, 0x60, 0x40, "Normal"		},
	{0x15, 0x01, 0x60, 0x20, "Difficult"		},
	{0x15, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x16, 0x01, 0x01, 0x01, "Off"			},
	{0x16, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Upright Controls"	},
	{0x16, 0x01, 0x02, 0x02, "Single"		},
	{0x16, 0x01, 0x02, 0x00, "Dual"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x16, 0x01, 0x04, 0x04, "Off"			},
	{0x16, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Vulcan)

static struct BurnDIPInfo Gradius2DIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0x5a, NULL			},
	{0x16, 0xff, 0xff, 0xfd, NULL			},

	coinage_dips(0x14)

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x03, "2"			},
	{0x15, 0x01, 0x03, 0x02, "3"			},
	{0x15, 0x01, 0x03, 0x01, "4"			},
	{0x15, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x15, 0x01, 0x04, 0x00, "Upright"		},
	{0x15, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x15, 0x01, 0x18, 0x18, "20K 150K"		},
	{0x15, 0x01, 0x18, 0x10, "30K 200K"		},
	{0x15, 0x01, 0x18, 0x08, "20K"			},
	{0x15, 0x01, 0x18, 0x00, "70K"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x60, 0x60, "Easy"			},
	{0x15, 0x01, 0x60, 0x40, "Normal"		},
	{0x15, 0x01, 0x60, 0x20, "Difficult"		},
	{0x15, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x16, 0x01, 0x01, 0x01, "Off"			},
	{0x16, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Upright Controls"	},
	{0x16, 0x01, 0x02, 0x02, "Single"		},
	{0x16, 0x01, 0x02, 0x00, "Dual"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x16, 0x01, 0x04, 0x04, "Off"			},
	{0x16, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Gradius2)

static struct BurnDIPInfo FroundDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0x42, NULL			},
	{0x16, 0xff, 0xff, 0xfd, NULL			},

	coinage_dips(0x14)

	{0   , 0xfe, 0   ,    4, "Energy"		},
	{0x15, 0x01, 0x03, 0x03, "18"			},
	{0x15, 0x01, 0x03, 0x02, "20"			},
	{0x15, 0x01, 0x03, 0x01, "22"			},
	{0x15, 0x01, 0x03, 0x00, "24"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x60, 0x60, "Easy"			},
	{0x15, 0x01, 0x60, 0x40, "Normal"		},
	{0x15, 0x01, 0x60, 0x20, "Difficult"		},
	{0x15, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x16, 0x01, 0x01, 0x01, "Off"			},
	{0x16, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x16, 0x01, 0x04, 0x04, "Off"			},
	{0x16, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Fround)

static struct BurnDIPInfo MiajDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0x52, NULL			},
	{0x16, 0xff, 0xff, 0xff, NULL			},

	coinage_dips(0x14)

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x03, "2"			},
	{0x15, 0x01, 0x03, 0x02, "3"			},
	{0x15, 0x01, 0x03, 0x01, "5"			},
	{0x15, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x15, 0x01, 0x18, 0x18, "30K 80K"		},
	{0x15, 0x01, 0x18, 0x10, "50K 100K"		},
	{0x15, 0x01, 0x18, 0x08, "50K"			},
	{0x15, 0x01, 0x18, 0x00, "100K"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x60, 0x60, "Easy"			},
	{0x15, 0x01, 0x60, 0x40, "Normal"		},
	{0x15, 0x01, 0x60, 0x20, "Difficult"		},
	{0x15, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x16, 0x01, 0x01, 0x01, "Off"			},
	{0x16, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "VRAM Character Check"	},
	{0x16, 0x01, 0x02, 0x02, "Off"			},
	{0x16, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x16, 0x01, 0x04, 0x04, "Off"			},
	{0x16, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Miaj)

static struct BurnDIPInfo CuebrckjDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0x5a, NULL			},
	{0x16, 0xff, 0xff, 0xff, NULL			},

	coinage_dips(0x14)

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x03, "1"			},
	{0x15, 0x01, 0x03, 0x02, "2"			},
	{0x15, 0x01, 0x03, 0x01, "3"			},
	{0x15, 0x01, 0x03, 0x00, "4"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x15, 0x01, 0x04, 0x00, "Upright"		},
	{0x15, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Machine Name"		},
	{0x15, 0x01, 0x18, 0x18, "None"			},
	{0x15, 0x01, 0x18, 0x10, "Lewis"		},
	{0x15, 0x01, 0x18, 0x08, "Johnson"		},
	{0x15, 0x01, 0x18, 0x00, "George"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x60, 0x60, "Easy"			},
	{0x15, 0x01, 0x60, 0x40, "Normal"		},
	{0x15, 0x01, 0x60, 0x20, "Hard"			},
	{0x15, 0x01, 0x60, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x16, 0x01, 0x01, 0x01, "Off"			},
	{0x16, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Upright Controls"	},
	{0x16, 0x01, 0x02, 0x02, "Single"		},
	{0x16, 0x01, 0x02, 0x00, "Dual"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x16, 0x01, 0x04, 0x04, "Off"			},
	{0x16, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Mode"			},
	{0x16, 0x01, 0x08, 0x08, "3"			},
	{0x16, 0x01, 0x08, 0x00, "4"			},
};

STDDIPINFO(Cuebrckj)

static void twin16_spriteram_process()
{
	INT32 dx = BURN_ENDIAN_SWAP_INT16(scrollx[0]);
	INT32 dy = BURN_ENDIAN_SWAP_INT16(scrolly[0]);

	UINT16 *spriteram16 = (UINT16*)DrvSprRAM;
	UINT16 *source = spriteram16;
	UINT16 *finish = spriteram16 + 0x1800;

	memset(&spriteram16[0x1800], 0, 0x800);

	while (source < finish)
	{
		INT32 priority = BURN_ENDIAN_SWAP_INT16(source[0]);
		if (priority & 0x8000 )
		{
			UINT16 *dest = &spriteram16[0x1800 + ((priority & 0xff) << 2)];

			INT32 xpos = (BURN_ENDIAN_SWAP_INT16(source[4]) << 16) | BURN_ENDIAN_SWAP_INT16(source[5]);
			INT32 ypos = (BURN_ENDIAN_SWAP_INT16(source[6]) << 16) | BURN_ENDIAN_SWAP_INT16(source[7]);

			INT32 attributes = BURN_ENDIAN_SWAP_INT16(source[2])&0x03ff;
			if (priority & 0x0200) attributes |= 0x4000;

			attributes |= 0x8000;

			dest[0] = source[3];
			dest[1] = BURN_ENDIAN_SWAP_INT16(((xpos >> 8) - dx) & 0xffff);
			dest[2] = BURN_ENDIAN_SWAP_INT16(((ypos >> 8) - dy) & 0xffff);
			dest[3] = BURN_ENDIAN_SWAP_INT16(attributes);
		}

		source += 0x50/2;
	}

	need_process_spriteram = 0;
}

static void fround_CPU_register_w(INT32 data)
{
	INT32 old = twin16_CPUA_register;
	twin16_CPUA_register = data;
	if (twin16_CPUA_register != old)
	{
		if ((old & 0x08) == 0 && (twin16_CPUA_register & 0x08)) {
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		}
	}
}

static void twin16_CPUA_register_w(INT32 data)
{
	if (twin16_CPUA_register != data)
	{
		if ((twin16_CPUA_register & 0x08) == 0 && (data & 0x08)) {
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		}

		if ((twin16_CPUA_register & 0x40) && (data & 0x40) == 0)
			twin16_spriteram_process();

		if ((twin16_CPUA_register & 0x10) == 0 && (data & 0x10)) {
			SekClose();
			SekOpen(1);
			SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
			SekClose();
			SekOpen(0);
		}

		twin16_CPUA_register = data;
	}
}

static void twin16_CPUB_register_w(INT32 data)
{
	INT32 old = twin16_CPUB_register;
	twin16_CPUB_register = data;
	if (twin16_CPUB_register != old)
	{
		if ((old & 0x01) == 0 && (twin16_CPUB_register & 0x01))
		{
			SekClose();
			SekOpen(0);
			SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
			SekClose();
			SekOpen(1);
		}

		INT32 offset = (twin16_CPUB_register & 4) << 17;
		SekMapMemory(DrvGfxROM1 + 0x100000 + offset, 0x700000, 0x77ffff, SM_ROM);
	}
}

void __fastcall twin16_main_write_word(UINT32 address, UINT16 data)
{
	switch (address)
	{
		case 0xc0002:
		case 0xc0006:
		case 0xc000a:
			scrollx[((address - 2) & 0x0c) >> 2] = BURN_ENDIAN_SWAP_INT16(data);
		return;

		case 0xc0004:
		case 0xc0008:
		case 0xc000c:
			scrolly[((address - 4) & 0x0c) >> 2] = BURN_ENDIAN_SWAP_INT16(data);
		return;

		case 0xe0000:
			gfx_bank = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}
}

void __fastcall twin16_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0xa0001:
			if (twin16_custom_video == 1) {
				fround_CPU_register_w(data);
			} else {
				twin16_CPUA_register_w(data);
			}
		return;

		case 0xa0008:
		case 0xa0009:
			*soundlatch = data;
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;

		case 0xa0011: // watchdog
		return;

		case 0xb0400:
		{
			*DrvNvRAMBank = data & 0x1f;
			int offset = data & 0x1f;
			SekMapMemory(DrvNvRAM + offset * 0x400,	0x0b0000, 0x0b03ff, SM_RAM);
		}
		return;

		case 0xc0001:
			video_register = data;
		return;
	}
}

UINT16 __fastcall twin16_main_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x0c000e:
		case 0x0c000f: {
			static INT32 ret = 0;
			ret = 1-ret;
			return ret;
		}

		case 0x0a0000:
		case 0x0a0002:
		case 0x0a0004:
		case 0x0a0006:
			return DrvInputs[(address - 0xa0000)/2];

		case 0x0a0010:
			return DrvDips[1];

		case 0x0a0012:
			return DrvDips[0];

		case 0x0a0018:
			return DrvDips[2];
	}

	return 0;
}

UINT8 __fastcall twin16_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x0c000e:
		case 0x0c000f: {
			static INT32 ret = 0;
			ret = 1-ret;
			return ret;
		}

		case 0x0a0000:
		case 0x0a0001:
		case 0x0a0002:
		case 0x0a0003:
		case 0x0a0004:
		case 0x0a0005:
		case 0x0a0006:
		case 0x0a0007:
			return DrvInputs[(address - 0xa0000)/2];

		case 0x0a0010:
		case 0x0a0011:
			return DrvDips[1];

		case 0x0a0012:
		case 0x0a0013:
			return DrvDips[0];

		case 0x0a0018:
		case 0x0a0019:
			return DrvDips[2];
	}

	return 0;
}

static inline void twin16_tile_write(INT32 offset)
{
	DrvGfxExp[(offset << 1) + 2] = DrvTileRAM[offset + 0] >> 4;
	DrvGfxExp[(offset << 1) + 3] = DrvTileRAM[offset + 0] & 0x0f;
	DrvGfxExp[(offset << 1) + 0] = DrvTileRAM[offset + 1] >> 4;
	DrvGfxExp[(offset << 1) + 1] = DrvTileRAM[offset + 1] & 0x0f;
}

void __fastcall twin16_sub_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfc0000) == 0x500000) {
		INT32 offset = address & 0x3ffff;
		*((UINT16*)(DrvTileRAM + offset)) = BURN_ENDIAN_SWAP_INT16(data);	
		twin16_tile_write(offset);
		return;
	}
}

void __fastcall twin16_sub_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0xa0001:
			twin16_CPUB_register_w(data);
		return;
	}

	if ((address & 0xfc0000) == 0x500000) {
		INT32 offset = address & 0x3ffff;
		DrvTileRAM[offset^1] = data;	
		twin16_tile_write(offset & ~1);
		return;
	}
}

void __fastcall twin16_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x9000:
			*soundlatch2 = data;
			UPD7759ResetWrite(0, data & 2);
		return;

		case 0xc000:
			BurnYM2151SelectRegister(data);
		return;

		case 0xc001:
			BurnYM2151WriteRegister(data);
		return;

		case 0xd000:
			UPD7759PortWrite(0, data);
		return;

		case 0xe000:
			UPD7759StartWrite(0, data & 1);
		return;
	}

	if ((address & 0xfff0) == 0xb000) {
		K007232WriteReg(0, address & 0x0f, data);
		return;
	}
}

UINT8 __fastcall twin16_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x9000:
			return *soundlatch2;

		case 0xa000:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;

		case 0xc000:
		case 0xc001:
			return BurnYM2151ReadStatus();

		case 0xf000:
			return UPD7759BusyRead(0) ? 1 : 0;
	}

	if ((address & 0xfff0) == 0xb000) {
		return K007232ReadReg(0, address & 0x0f);
	}

	return 0;
}

static void DrvK007232VolCallback(INT32 v)
{
	K007232SetVolume(0, 0, (v >> 0x4) * 0x11, 0);
	K007232SetVolume(0, 1, 0, (v & 0x0f) * 0x11);
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	SekOpen(1);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2151Reset();

	UPD7759Reset();

	gfx_bank = 0x3210; // for other than fround
	video_register = 0;

	twin16_CPUA_register = 0;
	twin16_CPUB_register = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM0	= Next; Next += 0x040000;
	Drv68KROM1	= Next; Next += 0x040000;
	DrvZ80ROM	= Next; Next += 0x008000;

	DrvGfxROM0	= Next; Next += 0x008000;
	DrvGfxROM1	= Next; Next += 0x200000;
	DrvGfxROM2	= Next; Next += 0x020000;

	DrvSndROM0	= Next; Next += 0x020000;
	DrvSndROM1	= Next; Next += 0x020000;

	DrvGfxExp	= Next; Next += 0x400000;
	DrvNvRAM	= Next; Next += 0x008000;

	DrvPalette	= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	AllRam		= Next;

	DrvSprRAM	= Next; Next += 0x004000;
	DrvSprBuf	= Next; Next += 0x004000;
	DrvShareRAM	= Next; Next += 0x010000;
	Drv68KRAM0	= Next; Next += 0x004000;
	DrvPalRAM	= Next; Next += 0x001000;
	DrvVidRAM2	= Next; Next += 0x006000;
	DrvVidRAM	= Next; Next += 0x004000;
	Drv68KRAM1	= Next; Next += 0x004000;
	DrvFgRAM	= Next; Next += 0x004000;
	DrvTileRAM	= Next; Next += 0x040000;
	DrvSprGfxRAM	= Next; Next += 0x020000;

	DrvZ80RAM	= Next; Next += 0x001000;

	DrvNvRAMBank	= Next; Next += 0x000001;

	scrollx		= (UINT16*)Next; Next += 0x00004 * sizeof(UINT16);
	scrolly		= (UINT16*)Next; Next += 0x00004 * sizeof(UINT16);

	soundlatch	= Next; Next += 0x000001;
	soundlatch2	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static void expand4bpp(UINT8 *src, UINT8 *dst, INT32 len)
{
	for (INT32 i = len-1; i > 0; i--) {
		dst[i * 2 + 0] = src[i] >> 4;
		dst[i * 2 + 1] = src[i] & 0x0f;
	}
}

static void gfxdecode()
{
	UINT16 *src = (UINT16*)DrvGfxROM1;
	UINT16 *dst = (UINT16*)BurnMalloc(0x200000);
	for (INT32 i = 0; i < 0x80000; i++)
	{
		dst[i * 2 + 0] = src[i + 0x80000];
		dst[i * 2 + 1] = src[i + 0x00000];
	}
	memcpy (src, dst, 0x200000);
	BurnFree(dst);
}

static INT32 load68k(UINT8 *rom, INT32 offs)
{
	if (BurnLoadRom(rom + 0x000001,  offs+0, 2)) return 1;
	if (BurnLoadRom(rom + 0x000000,  offs+1, 2)) return 1;
	if (BurnLoadRom(rom + 0x020001,  offs+2, 2)) return 1;
	if (BurnLoadRom(rom + 0x020000,  offs+3, 2)) return 1;

	return 0;
}

static INT32 DrvInit(INT32 (pLoadCallback)())
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (pLoadCallback != NULL) {
			if (pLoadCallback()) return 1;
		}
	
		gfxdecode();
		expand4bpp(DrvGfxROM0, DrvGfxROM0, 0x4000);
		
		if (twin16_custom_video == 1)
		{
			BurnByteswap(DrvGfxROM1, 0x200000);
			expand4bpp(DrvGfxROM1, DrvGfxExp, 0x200000);
			BurnByteswap(DrvGfxROM1, 0x200000);
		}
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM0,		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(DrvShareRAM,		0x040000, 0x04ffff, SM_RAM);
	SekMapMemory(Drv68KRAM0,		0x060000, 0x063fff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x080000, 0x080fff, SM_RAM);
	SekMapMemory(DrvNvRAM,			0x0b0000, 0x0b03ff, SM_RAM);
	SekMapMemory(DrvVidRAM2,		0x100000, 0x105fff, SM_RAM);
	SekMapMemory(DrvVidRAM,			0x120000, 0x123fff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x140000, 0x143fff, SM_RAM);
	SekMapMemory(DrvGfxROM1,		0x500000, 0x6fffff, SM_ROM);
	SekSetWriteWordHandler(0,		twin16_main_write_word);
	SekSetWriteByteHandler(0,		twin16_main_write_byte);
	SekSetReadWordHandler(0,		twin16_main_read_word);
	SekSetReadByteHandler(0,		twin16_main_read_byte);
	SekClose();

	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Drv68KROM1,		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(DrvShareRAM,		0x040000, 0x04ffff, SM_RAM);
	SekMapMemory(Drv68KRAM1,		0x060000, 0x063fff, SM_RAM);
	SekMapMemory(DrvGfxROM2,		0x080000, 0x09ffff, SM_ROM);
	SekMapMemory(DrvSprRAM,			0x400000, 0x403fff, SM_RAM);
	SekMapMemory(DrvVidRAM,			0x480000, 0x483fff, SM_RAM);
	SekMapMemory(DrvTileRAM,		0x500000, 0x53ffff, SM_ROM);
	SekMapMemory(DrvGfxROM1,		0x600000, 0x6fffff, SM_ROM);
	SekMapMemory(DrvGfxROM1 + 0x100000,	0x700000, 0x77ffff, SM_ROM);
	SekMapMemory(DrvSprGfxRAM,		0x780000, 0x79ffff, SM_RAM);
	SekSetWriteWordHandler(0,		twin16_sub_write_word);
	SekSetWriteByteHandler(0,		twin16_sub_write_byte);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x8fff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x8fff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x8fff, 2, DrvZ80RAM);
	ZetSetWriteHandler(twin16_sound_write);
	ZetSetReadHandler(twin16_sound_read);
	ZetMemEnd();
	ZetClose();

	K007232Init(0, 3579545, DrvSndROM0, 0x20000);
	K007232SetPortWriteHandler(0, DrvK007232VolCallback);

	BurnYM2151Init(3579580, 75.0);

	UPD7759Init(0, UPD7759_STANDARD_CLOCK, DrvSndROM1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	SekExit();
	ZetExit();

	K007232Exit();
	UPD7759Exit();
	BurnYM2151Exit();

	BurnFree (AllMem);

	is_vulcan = 0;
	twin16_custom_video = 0;

	return 0;
}

static inline void DrvRecalcPal()
{
	UINT8 r,g,b;
	UINT16 *p = (UINT16*)DrvPalRAM;
	for (INT32 i = 0; i < 0x1000 / 2; i+=2)
	{
		INT32 d = ((BURN_ENDIAN_SWAP_INT16(p[i+0]) & 0xff) << 8) | (BURN_ENDIAN_SWAP_INT16(p[i+1]) & 0xff);

		r = (d >>  0) & 0x1f;
		g = (d >>  5) & 0x1f;
		b = (d >> 10) & 0x1f;

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		DrvPalette[i/2] = BurnHighCol(r, g, b, 0);
	}
}

static void draw_fg_layer()
{
	INT32 flipx = (video_register & 2) ? 0x1f8 : 0;
	INT32 flipy = (video_register & 1) ? 0x0f8 : 0;

	UINT16 *vram = (UINT16*)DrvVidRAM2;

	for (INT32 offs = 0x80; offs < 0x780; offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 6) << 3;

		sx ^= flipx;
		sy ^= flipy;

		if (flipx) sx -= 192;
		sy -= 16;

		if (sx >= nScreenWidth) continue;

		INT32 attr  = BURN_ENDIAN_SWAP_INT16(vram[offs]);
		INT32 code  = attr & 0x1ff;
		INT32 color = (attr >> 9) & 0x0f;

		if (code == 0) continue;

		if (sx >= 0 && sx < nScreenWidth-7 && sy >= 0 && sy < nScreenHeight-7) {
			if (flipx) {
				Render8x8Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
			}
		} else {
			if (flipx) {
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
			}
		}
	}
}

static void draw_layer(INT32 opaque)
{
	INT32 o = 0;
	INT32 banks[4];
	if (((video_register & 8) >> 3) != opaque) o = 1;

	UINT16 *vram = (UINT16*)(DrvVidRAM + (o << 13));
	banks[3] =  gfx_bank >> 12;
	banks[2] = (gfx_bank >>  8) & 0x0f;
	banks[1] = (gfx_bank >>  4) & 0x0f;
	banks[0] =  gfx_bank & 0xf;

	INT32 dx = BURN_ENDIAN_SWAP_INT16(scrollx[1+o]);
	INT32 dy = BURN_ENDIAN_SWAP_INT16(scrolly[1+o]);

	INT32 flipx = 0;
	INT32 flipy = 0;

	if (video_register & 2) {
		dx = 256-dx-64;
		flipx = 1;
	}

	if (video_register & 1) {
		dy = 256-dy;
		flipy = 1;
	}
	if (is_vulcan) flipy ^= 1;

	INT32 palette = o;

	for (INT32 offs = 0; offs < 64 * 64; offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 6) << 3;
		if (video_register & 2) sx ^= 0x1f8;
		if (video_register & 1) sy ^= 0x1f8;

		sx = (sx - dx) & 0x1ff;
		sy = (sy - dy) & 0x1ff;
		if (sx >= 320) sx-=512;
		if (sy >= 256) sy-=512;

		sy -= 16;

		INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[offs]);
		INT32 color = (0x20 + (code >> 13) + 8 * palette);
		    code = (code & 0x7ff) | (banks[(code >> 11) & 3] << 11);

		if (sx >= 0 && sx < nScreenWidth-7 && sy >= 0 && sy < nScreenHeight-7) {
			if (opaque) {
				if (flipy) {
					if (flipx) {
						Render8x8Tile_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxExp);
					} else {
						Render8x8Tile_FlipY(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxExp);
					}
				} else {
					if (flipx) {
						Render8x8Tile_FlipX(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxExp);
					} else {
						Render8x8Tile(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxExp);
					}
				}
			} else {
				if (code == 0) continue;

				if (flipy) {
					if (flipx) {
						Render8x8Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxExp);
					} else {
						Render8x8Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxExp);
					}
				} else {
					if (flipx) {
						Render8x8Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxExp);
					} else {
						Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxExp);
					}
				}
			}
		} else {
			if (sy < -7 || sy >= nScreenHeight || sx >= nScreenWidth) continue;

			if (opaque) {
				if (flipy) {
					if (flipx) {
						Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxExp);
					} else {
						Render8x8Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxExp);
					}
				} else {
					if (flipx) {
						Render8x8Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxExp);
					} else {
						Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxExp);
					}
				}
			} else {
				if (code == 0) continue;

				if (flipy) {
					if (flipx) {
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxExp);
					} else {
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxExp);
					}
				} else {
					if (flipx) {
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxExp);
					} else {
						Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxExp);
					}
				}
			}
		}
	}
}

static void draw_sprite(UINT16 *pen_data, INT32 pal_base, INT32 xpos, INT32 ypos, INT32 width, INT32 height, INT32 flipx, INT32 flipy)
{
	if (xpos >= 320) xpos -= 0x10000;
	if (ypos >= 256) ypos -= 0x10000;

	for (INT32 y = 0; y < height; y++)
	{
		INT32 sy = (flipy) ? (ypos + height - 1 - y) : (ypos + y);

		if (sy >= 16 && sy < 240)
		{
			UINT16 *dest = pTransDraw + (sy - 16) * nScreenWidth;

			for (INT32 x = 0; x < width; x++)
			{
				INT32 sx = (flipx) ? (xpos + width - 1 - x) : (xpos + x);

				if (sx >= 0 && sx < 320)
				{
					INT32 pen = BURN_ENDIAN_SWAP_INT16(pen_data[x/4]);

					pen = (pen >> ((~x & 3) << 2)) & 0x0f;

					if (pen) dest[sx] = pal_base + pen;
				}
			}
		}

		pen_data += width/4;
	}
}

static void draw_sprites(INT32 priority)
{
	UINT16 *twin16_sprite_gfx_ram = (UINT16*)DrvSprGfxRAM;
	UINT16 *twin16_gfx_rom = (UINT16*)DrvGfxROM1;
	UINT16 *buffered_spriteram16 = (UINT16*)DrvSprBuf;

	UINT16 *source = 0x1800+buffered_spriteram16;
	UINT16 *finish = 0x1800+buffered_spriteram16 + 0x800 - 4;

	for (; source < finish; source += 4)
	{
		INT32 attributes = BURN_ENDIAN_SWAP_INT16(source[3]);
		INT32 prio = (attributes&0x4000) >> 14;
		if (prio != priority) continue;

		INT32 code = BURN_ENDIAN_SWAP_INT16(source[0]);

		if (code != 0xffff && attributes & 0x8000)
		{
			INT32 xpos 	= BURN_ENDIAN_SWAP_INT16(source[1]);
			INT32 ypos 	= BURN_ENDIAN_SWAP_INT16(source[2]);

			INT32 pal_base	= ((attributes&0xf)+0x10)*16;
			INT32 height	= 16<<((attributes>>6)&0x3);
			INT32 width	= 16<<((attributes>>4)&0x3);
			INT32 flipy 	= attributes&0x0200;
			INT32 flipx 	= attributes&0x0100;
			UINT16 *pen_data = 0;

			if( twin16_custom_video == 1 )
			{
				pen_data = twin16_gfx_rom + 0x80000;
			}
			else
			{
				switch( (code>>12)&0x3 )
				{
					case 0:
					pen_data = twin16_gfx_rom;
					break;

					case 1:
					pen_data = twin16_gfx_rom + 0x40000;
					break;

					case 2:
					pen_data = twin16_gfx_rom + 0x80000;
					if( code&0x4000 ) pen_data += 0x40000;
					break;

					case 3:
					pen_data = twin16_sprite_gfx_ram;
					break;
				}

				code &= 0xfff;
			}

			if (height == 64 && width == 64)
			{
				code &= ~8;
			}
			else if (height == 32 && width == 32)
			{
				code &= ~3;
			}
			else if (height == 32 && width == 16)
			{
				code &= ~1;
			}
			else if (height == 16 && width == 32)
			{
				code &= ~1;
			}

			pen_data += code << 6;

			if( video_register&1 )
			{
				if (ypos>65000) ypos=ypos-65536;
				ypos = 256-ypos-height;
				flipy = !flipy;
			}
			if( video_register&2 )
			{
				if (xpos>65000) xpos=xpos-65536;
				xpos = 320-xpos-width;
				flipx = !flipx;
			}

			draw_sprite(pen_data, pal_base, xpos, ypos, width, height, flipx, flipy);
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvRecalcPal();
	}

	if (nBurnLayer & 1) draw_layer(1);
	if (nSpriteEnable & 1) draw_sprites(1);
	if (nBurnLayer & 2) draw_layer(0);
	if (nSpriteEnable & 2) draw_sprites(0);
	if (nBurnLayer & 4) draw_fg_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 5);
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
		}

	  // Clear Opposites
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[2] & 0x0c) == 0) DrvInputs[2] |= 0x0c;
		if ((DrvInputs[2] & 0x03) == 0) DrvInputs[2] |= 0x03;
		if ((DrvInputs[3] & 0x0c) == 0) DrvInputs[3] |= 0x0c;
		if ((DrvInputs[3] & 0x03) == 0) DrvInputs[3] |= 0x03;
	}

	INT32 nSegment;
	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = 100;
	if (twin16_custom_video == 0 && is_vulcan == 0) nInterleave = 1000; // devilw

	INT32 nTotalCycles[3] = { (twin16_custom_video == 1) ? 10000000 / 60 : 9216000 / 60, 9216000 / 60, 3579545 / 60 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		SekOpen(0);
		nSegment = (nTotalCycles[0] - nCyclesDone[0]) / (nInterleave - i);
		nCyclesDone[0] += SekRun(nSegment);
		if ((twin16_CPUA_register & 0x20) && i == nInterleave-1) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
		SekClose();

		if (twin16_custom_video != 1) {
			SekOpen(1);
			nCyclesDone[1] += SekRun(nSegment);
			if ((twin16_CPUB_register & 0x02) && i == nInterleave-1) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
			SekClose();
		}

		nSegment = (nTotalCycles[2] - nCyclesDone[2]) / (nInterleave - i);

		nCyclesDone[1] += ZetRun(nSegment);

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			UPD7759Update(0, pSoundBuf, nSegmentLength);
			K007232Update(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		if (nSegmentLength) {
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			UPD7759Update(0, pSoundBuf, nSegmentLength);
			K007232Update(0, pSoundBuf, nSegmentLength);
		}
	}	

	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	if(~twin16_CPUA_register & 0x40 && need_process_spriteram)
		twin16_spriteram_process();

	need_process_spriteram = 1;

	memcpy (DrvSprBuf, DrvSprRAM, 0x4000);

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin =  0x029702;
	}

	if (nAction & ACB_MEMORY_RAM) {	
		ba.Data		= AllRam;
		ba.nLen		= RamEnd - AllRam;
		ba.nAddress 	= 0x000000;
		ba.szName	= "All RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_NVRAM) {
		ba.Data		= DrvNvRAM;
		ba.nLen		= 0x008000;
		ba.nAddress	= 0xb00000;
		ba.szName	= "Cue Brick NV RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		UPD7759Scan(0, nAction, pnMin);
		BurnYM2151Scan(nAction);
	//	K007232Scan(nAction, pnMin); // crash...

		SCAN_VAR(gfx_bank);
		SCAN_VAR(video_register);
		SCAN_VAR(twin16_CPUA_register);
		SCAN_VAR(twin16_CPUB_register);
	}

	if (nAction & ACB_WRITE) {
		if (twin16_custom_video != 1) {
			for (INT32 i = 0; i < 0x40000; i+=2) {
				twin16_tile_write(i);
			}
		}

		SekOpen(0);
		SekMapMemory(DrvNvRAM + (*DrvNvRAMBank * 0x400),	0x0b0000, 0x0b03ff, SM_RAM);
		SekClose();

		SekOpen(1);
		INT32 offset = (twin16_CPUB_register & 4) << 17;
		SekMapMemory(DrvGfxROM1 + 0x100000 + offset, 0x700000, 0x77ffff, SM_ROM);
		SekClose();
	}

 	return 0;
}


// Devil World

static struct BurnRomInfo devilwRomDesc[] = {
	{ "687_t05.6n",		0x10000, 0x8ab7dc61, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "687_t04.4n",		0x10000, 0xc69924da, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "687_t09.6r",		0x10000, 0xfae97de0, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "687_t08.4r",		0x10000, 0x8c898d67, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "687_q07.10n",	0x10000, 0x53110c0b, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "687_q06.8n",		0x10000, 0x9c53a0c5, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "687_q13.10s",	0x10000, 0x36ae6014, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "687_q12.8s",		0x10000, 0x6d012167, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "687_l03.10a",	0x08000, 0x7201983c, 3 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "687_m14.d8",		0x04000, 0xd7338557, 4 | BRF_GRA },           //  9 Characters

	{ "687i17.p16",		0x80000, 0x66cb3923, 5 | BRF_GRA },           // 10 Sprites / Bg Tiles
	{ "687i18.p18",		0x80000, 0xa1c7d0db, 5 | BRF_GRA },           // 11
	{ "687i15.p13",		0x80000, 0xeec8c5b2, 5 | BRF_GRA },           // 12
	{ "687i16.p15",		0x80000, 0x746cf48b, 5 | BRF_GRA },           // 13

	{ "687_l11.10r",	0x10000, 0x399deee8, 6 | BRF_GRA },           // 14 Sprites / Bg Tiles
	{ "687_l10.8r",		0x10000, 0x117c91ee, 6 | BRF_GRA },           // 15

	{ "687_i01.5a",		0x20000, 0xd4992dfb, 7 | BRF_SND },           // 16 K007232 Samples

	{ "687_i02.7c",		0x20000, 0xe5947501, 8 | BRF_SND },           // 17 UPD7759 Samples
};

STD_ROM_PICK(devilw)
STD_ROM_FN(devilw)

static INT32 devilwCallback()
{
	if (load68k(Drv68KROM0,  0)) return 1;
	if (load68k(Drv68KROM1,  4)) return 1;
	
	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  8, 1)) return 1;
	
	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  9, 1)) return 1;	

	if (BurnLoadRom(DrvGfxROM1 + 0x000000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x080000, 11, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x100000, 12, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x180000, 13, 1)) return 1;
	
	if (BurnLoadRom(DrvGfxROM2 + 0x000001, 14, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x000000, 15, 2)) return 1;
		
	if (BurnLoadRom(DrvSndROM0 + 0x000000, 16, 1)) return 1;
	if (BurnLoadRom(DrvSndROM1 + 0x000000, 17, 1)) return 1;

	return 0;
}

static INT32 devilwInit()
{
	twin16_custom_video = 0;

	return DrvInit(devilwCallback);
}

struct BurnDriver BurnDrvDevilw = {
	"devilw", NULL, NULL, NULL, "1987",
	"Devil World\0", NULL, "Konami", "GX687",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_SHOOT, 0,
	NULL, devilwRomInfo, devilwRomName, NULL, NULL, DevilwInputInfo, DevilwDIPInfo,
	devilwInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// Majuu no Ohkoku

static struct BurnRomInfo majuuRomDesc[] = {
	{ "687_s05.6n",		0x10000, 0xbd99b434, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "687_s04.4n",		0x10000, 0x3df732e2, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "687_s09.6r",		0x10000, 0x1f6efec3, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "687_s08.4r",		0x10000, 0x8a16c8c6, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "687_q07.10n",	0x10000, 0x53110c0b, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "687_q06.8n",		0x10000, 0x9c53a0c5, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "687_q13.10s",	0x10000, 0x36ae6014, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "687_q12.8s",		0x10000, 0x6d012167, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "687_l03.10a",	0x08000, 0x7201983c, 3 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "687_l14.d8",		0x04000, 0x20ecccd6, 4 | BRF_GRA },           //  9 Characters

	{ "687i17.p16",		0x80000, 0x66cb3923, 5 | BRF_GRA },           // 10 Sprites / Bg Tiles
	{ "687i18.p18",		0x80000, 0xa1c7d0db, 5 | BRF_GRA },           // 11
	{ "687i15.p13",		0x80000, 0xeec8c5b2, 5 | BRF_GRA },           // 12
	{ "687i16.p15",		0x80000, 0x746cf48b, 5 | BRF_GRA },           // 13

	{ "687_l11.10r",	0x10000, 0x399deee8, 6 | BRF_GRA },           // 14 Sprites / Bg Tiles
	{ "687_l10.8r",		0x10000, 0x117c91ee, 6 | BRF_GRA },           // 15

	{ "687_i01.5a",		0x20000, 0xd4992dfb, 7 | BRF_SND },           // 16 K007232 Samples

	{ "687_i02.7c",		0x20000, 0xe5947501, 8 | BRF_SND },           // 17 UPD7759 Samples
};

STD_ROM_PICK(majuu)
STD_ROM_FN(majuu)

struct BurnDriver BurnDrvMajuu = {
	"majuu", "devilw", NULL, NULL, "1987",
	"Majuu no Ohkoku\0", NULL, "Konami", "GX687",
	L"\u9B54\u7363\u306E\u738B\u56FD\0Majuu no Ohkoku\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SHOOT, 0,
	NULL, majuuRomInfo, majuuRomName, NULL, NULL, DevilwInputInfo, DevilwDIPInfo,
	devilwInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// Dark Adventure

static struct BurnRomInfo darkadvRomDesc[] = {
	{ "687_n05.6n",		0x10000, 0xa9195b0b, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "687_n04.4n",		0x10000, 0x65b55105, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "687_n09.6r",		0x10000, 0x1c6b594c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "687_n08.4r",		0x10000, 0xa9603196, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "687_n07.10n",	0x10000, 0x6154322a, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "687_n06.8n",		0x10000, 0x37a72e8b, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "687_n13.10s",	0x10000, 0xf1c252af, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "687_n12.8s",		0x10000, 0xda221944, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "687_n03.10a",	0x08000, 0xa24c682f, 3 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "687_n14.d8",		0x04000, 0xc76ac6d2, 4 | BRF_GRA },           //  9 Characters

	{ "687i17.p16",		0x80000, 0x66cb3923, 5 | BRF_GRA },           // 10 Sprites / Bg Tiles
	{ "687i18.p18",		0x80000, 0xa1c7d0db, 5 | BRF_GRA },           // 11
	{ "687i15.p13",		0x80000, 0xeec8c5b2, 5 | BRF_GRA },           // 12
	{ "687i16.p15",		0x80000, 0x746cf48b, 5 | BRF_GRA },           // 13

	{ "687_l11.10r",	0x10000, 0x399deee8, 6 | BRF_GRA },           // 14 Sprites / Bg Tiles
	{ "687_l10.8r",		0x10000, 0x117c91ee, 6 | BRF_GRA },           // 15

	{ "687_i01.5a",		0x20000, 0xd4992dfb, 7 | BRF_SND },           // 16 K007232 Samples

	{ "687_i02.7c",		0x20000, 0xe5947501, 8 | BRF_SND },           // 17 UPD7759 Samples
};

STD_ROM_PICK(darkadv)
STD_ROM_FN(darkadv)

struct BurnDriver BurnDrvDarkadv = {
	"darkadv", "devilw", NULL, NULL, "1987",
	"Dark Adventure\0", NULL, "Konami", "GX687",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_PREFIX_KONAMI, GBF_SHOOT, 0,
	NULL, darkadvRomInfo, darkadvRomName, NULL, NULL, DarkadvInputInfo, DarkadvDIPInfo,
	devilwInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// Vulcan Venture (New)

static struct BurnRomInfo vulcanRomDesc[] = {
	{ "785_w05.6n",		0x10000, 0x6e0e99cd, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "785_w04.4n",		0x10000, 0x23ec74ca, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "785_w09.6r",		0x10000, 0x377e4f28, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "785_w08.4r",		0x10000, 0x813d41ea, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "785_p07.10n",	0x10000, 0x686d549d, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "785_p06.8n",		0x10000, 0x70c94bee, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "785_p13.10s",	0x10000, 0x478fdb0a, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "785_p12.8s",		0x10000, 0x38ea402a, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "785_g03.10a",	0x08000, 0x67a3b50d, 3 | BRF_PRG | BRF_ESS }, //  0 Z80 Code

	{ "785_h14.d8",		0x04000, 0x02f4b16f, 4 | BRF_GRA },           //  9 Characters

	{ "785f17.p16",		0x80000, 0x8fbec1a4, 5 | BRF_GRA },           // 10 Sprites / Bg Tiles
	{ "785f18.p18",		0x80000, 0x50d61e38, 5 | BRF_GRA },           // 11
	{ "785f15.p13",		0x80000, 0xaf96aef3, 5 | BRF_GRA },           // 12
	{ "785f16.p15",		0x80000, 0xb858df1f, 5 | BRF_GRA },           // 13

	{ "785_f01.5a",		0x20000, 0xa0d8d69e, 7 | BRF_SND },           // 14 K007232 Samples

	{ "785_f02.7c",		0x20000, 0xc39f5ca4, 8 | BRF_SND },           // 15 UPD7759 Samples
};

STD_ROM_PICK(vulcan)
STD_ROM_FN(vulcan)

static INT32 vulcanCallback()
{
	if (load68k(Drv68KROM0,  0)) return 1;
	if (load68k(Drv68KROM1,  4)) return 1;
	
	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  8, 1)) return 1;
	
	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  9, 1)) return 1;	

	if (BurnLoadRom(DrvGfxROM1 + 0x000000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x080000, 11, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x100000, 12, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x180000, 13, 1)) return 1;

	BurnByteswap(DrvGfxROM1, 0x200000);

	if (BurnLoadRom(DrvSndROM0 + 0x000000, 14, 1)) return 1;
	if (BurnLoadRom(DrvSndROM1 + 0x000000, 15, 1)) return 1;

	return 0;
}

static INT32 vulcanInit()
{
	is_vulcan = 1;
	twin16_custom_video = 0;

	return DrvInit(vulcanCallback);
}

struct BurnDriver BurnDrvVulcan = {
	"vulcan", NULL, NULL, NULL, "1988",
	"Vulcan Venture (New)\0", NULL, "Konami", "GX785",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, vulcanRomInfo, vulcanRomName, NULL, NULL, DrvInputInfo, VulcanDIPInfo,
	vulcanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// Vulcan Venture (Old)

static struct BurnRomInfo vulcanaRomDesc[] = {
	{ "785_r05.6n",		0x10000, 0x1777a57b, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "785_r04.4n",		0x10000, 0x790cbdf3, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "785_u09.6r",		0x10000, 0x1aba7010, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "785_k08.4r",		0x10000, 0x36bb1be1, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "785_p07.10n",	0x10000, 0x686d549d, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "785_p06.8n",		0x10000, 0x70c94bee, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "785_p13.10s",	0x10000, 0x478fdb0a, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "785_p12.8s",		0x10000, 0x38ea402a, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "785_g03.10a",	0x08000, 0x67a3b50d, 3 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "785_h14.d8",		0x04000, 0x02f4b16f, 4 | BRF_GRA },           //  9 Characters

	{ "785f17.p16",		0x80000, 0x8fbec1a4, 5 | BRF_GRA },           // 10 Sprites / Bg Tiles
	{ "785f18.p18",		0x80000, 0x50d61e38, 5 | BRF_GRA },           // 11
	{ "785f15.p13",		0x80000, 0xaf96aef3, 5 | BRF_GRA },           // 12
	{ "785f16.p15",		0x80000, 0xb858df1f, 5 | BRF_GRA },           // 13

	{ "785_f01.5a",		0x20000, 0xa0d8d69e, 7 | BRF_SND },           // 14 K007232 Samples

	{ "785_f02.7c",		0x20000, 0xc39f5ca4, 8 | BRF_SND },           // 15 UPD7759 Samples
};

STD_ROM_PICK(vulcana)
STD_ROM_FN(vulcana)

struct BurnDriver BurnDrvVulcana = {
	"vulcana", "vulcan", NULL, NULL, "1988",
	"Vulcan Venture (Old)\0", NULL, "Konami", "GX785",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, vulcanaRomInfo, vulcanaRomName, NULL, NULL, DrvInputInfo, VulcanDIPInfo,
	vulcanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// Vulcan Venture (Oldest)

static struct BurnRomInfo vulcanbRomDesc[] = {
	{ "785_n05.6n",		0x10000, 0x6371d475, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "785_n04.4n",		0x10000, 0xb6f24b9e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "785_r09.6r",		0x10000, 0xfbfd1543, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "785_r08.4r",		0x10000, 0x36bb1be1, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "785_g07.10n",	0x10000, 0xee09dd5d, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "785_g06.8n",		0x10000, 0x85ab7af7, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "785_g13.10s",	0x10000, 0x274f325d, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "785_g12.8s",		0x10000, 0x1625f933, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "785_g03.10a",	0x08000, 0x67a3b50d, 3 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "785_h14.d8",		0x04000, 0x02f4b16f, 4 | BRF_GRA },           //  9 Characters

	{ "785f17.p16",		0x80000, 0x8fbec1a4, 5 | BRF_GRA },           // 10 Sprites / Bg Tiles
	{ "785f18.p18",		0x80000, 0x50d61e38, 5 | BRF_GRA },           // 11
	{ "785f15.p13",		0x80000, 0xaf96aef3, 5 | BRF_GRA },           // 12
	{ "785f16.p15",		0x80000, 0xb858df1f, 5 | BRF_GRA },           // 13

	{ "785_f01.5a",		0x20000, 0xa0d8d69e, 7 | BRF_SND },           // 14 K007232 Samples

	{ "785_f02.7c",		0x20000, 0xc39f5ca4, 8 | BRF_SND },           // 15 UPD7759 Samples
};

STD_ROM_PICK(vulcanb)
STD_ROM_FN(vulcanb)

struct BurnDriver BurnDrvVulcanb = {
	"vulcanb", "vulcan", NULL, NULL, "1988",
	"Vulcan Venture (Oldest)\0", NULL, "Konami", "GX785",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, vulcanbRomInfo, vulcanbRomName, NULL, NULL, DrvInputInfo, VulcanDIPInfo,
	vulcanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// Gradius II - GOFER no Yabou (Japan New ver.)

static struct BurnRomInfo gradius2RomDesc[] = {
	{ "785_x05.6n",		0x10000, 0x8a23a7b8, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "785_x04.4n",		0x10000, 0x88e466ce, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "785_x09.6r",		0x10000, 0x3f3d7d7a, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "785_x08.4r",		0x10000, 0xc39c8efd, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "785_p07.10n",	0x10000, 0x686d549d, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "785_p06.8n",		0x10000, 0x70c94bee, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "785_p13.10s",	0x10000, 0x478fdb0a, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "785_p12.8s",		0x10000, 0x38ea402a, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "785_g03.10a",	0x08000, 0x67a3b50d, 3 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "785_g14.d8",		0x04000, 0x9dcdad9d, 4 | BRF_GRA },           //  9 Characters

	{ "gr2.p16",		0x80000, 0x4e7a7b82, 5 | BRF_GRA },           // 10 Sprites / Bg Tiles
	{ "gr2.p18",		0x80000, 0x3f604e9a, 5 | BRF_GRA },           // 11
	{ "gr2.p13",		0x80000, 0x5bd239ac, 5 | BRF_GRA },           // 12
	{ "gr2.p15",		0x80000, 0x95c6b8a3, 5 | BRF_GRA },           // 13

	{ "785_f01.5a",		0x20000, 0xa0d8d69e, 7 | BRF_SND },           // 14 K007232 Samples

	{ "785_f02.7c",		0x20000, 0xc39f5ca4, 8 | BRF_SND },           // 15 UPD7759 Samples
};

STD_ROM_PICK(gradius2)
STD_ROM_FN(gradius2)

static INT32 gradius2Callback()
{
	if (load68k(Drv68KROM0,  0)) return 1;
	if (load68k(Drv68KROM1,  4)) return 1;
	
	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  8, 1)) return 1;
	
	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  9, 1)) return 1;	

	if (BurnLoadRom(DrvGfxROM1 + 0x000000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x080000, 11, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x100000, 12, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x180000, 13, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0 + 0x000000, 14, 1)) return 1;
	if (BurnLoadRom(DrvSndROM1 + 0x000000, 15, 1)) return 1;

	return 0;
}

static INT32 gradius2Init()
{
	is_vulcan = 1;
	twin16_custom_video = 0;

	return DrvInit(gradius2Callback);
}

struct BurnDriver BurnDrvGradius2 = {
	"gradius2", "vulcan", NULL, NULL, "1988",
	"Gradius II - GOFER no Yabou (Japan New ver.)\0", NULL, "Konami", "GX785",
	L"Gradius II - GOFER \u306E\u91CE\u671B (Japan New ver.)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, gradius2RomInfo, gradius2RomName, NULL, NULL, DrvInputInfo, Gradius2DIPInfo,
	gradius2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// Gradius II - GOFER no Yabou (Japan Old ver.)

static struct BurnRomInfo gradius2aRomDesc[] = {
	{ "785_p05.6n",		0x10000, 0x4db0e736, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "785_p04.4n",		0x10000, 0x765b99e6, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "785_t09.6r",		0x10000, 0x4e3f4965, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "785_j08.4r",		0x10000, 0x2b1c9108, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "785_p07.10n",	0x10000, 0x686d549d, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "785_p06.8n",		0x10000, 0x70c94bee, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "785_p13.10s",	0x10000, 0x478fdb0a, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "785_p12.8s",		0x10000, 0x38ea402a, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "785_g03.10a",	0x08000, 0x67a3b50d, 3 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "785_g14.d8",		0x04000, 0x9dcdad9d, 4 | BRF_GRA },           //  9 Characters

	{ "785f17.p16",		0x80000, 0x8fbec1a4, 5 | BRF_GRA },           // 10 Sprites / Bg Tiles
	{ "785f18.p18",		0x80000, 0x50d61e38, 5 | BRF_GRA },           // 11
	{ "785f15.p13",		0x80000, 0xaf96aef3, 5 | BRF_GRA },           // 12
	{ "785f16.p15",		0x80000, 0xb858df1f, 5 | BRF_GRA },           // 13

	{ "785_f01.5a",		0x20000, 0xa0d8d69e, 7 | BRF_SND },           // 14 K007232 Samples

	{ "785_f02.7c",		0x20000, 0xc39f5ca4, 8 | BRF_SND },           // 15 UPD7759 Samples
};

STD_ROM_PICK(gradius2a)
STD_ROM_FN(gradius2a)

struct BurnDriver BurnDrvGradius2a = {
	"gradius2a", "vulcan", NULL, NULL, "1988",
	"Gradius II - GOFER no Yabou (Japan Old ver.)\0", NULL, "Konami", "GX785",
	L"Gradius II - GOFER \u306E\u91CE\u671B (Japan Old ver.)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, gradius2aRomInfo, gradius2aRomName, NULL, NULL, DrvInputInfo, VulcanDIPInfo,
	vulcanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// Gradius II - GOFER no Yabou (Japan Older ver.)

static struct BurnRomInfo gradius2bRomDesc[] = {
	{ "785_p05.6n",		0x10000, 0x4db0e736, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "785_p04.4n",		0x10000, 0x765b99e6, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "785_j09.6r",		0x10000, 0x6d96a7e3, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "785_j08.4r",		0x10000, 0x2b1c9108, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "785_p07.10n",	0x10000, 0x686d549d, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "785_p06.8n",		0x10000, 0x70c94bee, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "785_p13.10s",	0x10000, 0x478fdb0a, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "785_p12.8s",		0x10000, 0x38ea402a, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "785_g03.10a",	0x08000, 0x67a3b50d, 3 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "785_g14.d8",		0x04000, 0x9dcdad9d, 4 | BRF_GRA },           //  9 Characters

	{ "785f17.p16",		0x80000, 0x8fbec1a4, 5 | BRF_GRA },           // 10 Sprites / Bg Tiles
	{ "785f18.p18",		0x80000, 0x50d61e38, 5 | BRF_GRA },           // 11
	{ "785f15.p13",		0x80000, 0xaf96aef3, 5 | BRF_GRA },           // 12
	{ "785f16.p15",		0x80000, 0xb858df1f, 5 | BRF_GRA },           // 13

	{ "785_f01.5a",		0x20000, 0xa0d8d69e, 7 | BRF_GRA },           // 14 K007232 Samples

	{ "785_f02.7c",		0x20000, 0xc39f5ca4, 8 | BRF_GRA },           // 15 UPD7759 Samples
};

STD_ROM_PICK(gradius2b)
STD_ROM_FN(gradius2b)

struct BurnDriver BurnDrvGradius2b = {
	"gradius2b", "vulcan", NULL, NULL, "1988",
	"Gradius II - GOFER no Yabou (Japan Older ver.)\0", NULL, "Konami", "GX785",
	L"Gradius II - GOFER \u306E\u91CE\u671B (Japan Older ver.)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, gradius2bRomInfo, gradius2bRomName, NULL, NULL, DrvInputInfo, VulcanDIPInfo,
	vulcanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// The Final Round (ver. M)

static struct BurnRomInfo froundRomDesc[] = {
	{ "870_m21.bin",	0x20000, 0x436dbffb, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "870_m20.bin",	0x20000, 0xb1c79d6a, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "870_f03.10a",	0x08000, 0xa645c727, 3 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "870_f14.d8",		0x04000, 0xc9b46615, 4 | BRF_PRG | BRF_ESS }, //  3 Characters

	{ "870c18.p18",		0x80000, 0x07927fe8, 5 | BRF_GRA },           //  4 Sprites / Bg Tiles
	{ "870c17.p16",		0x80000, 0x2bc99ff8, 5 | BRF_GRA },           //  5
	{ "870c16.p15",		0x80000, 0x41df6a1b, 5 | BRF_GRA },           //  6
	{ "870c15.p13",		0x80000, 0x8c9281df, 5 | BRF_GRA },           //  7

	{ "870_c01.5a",		0x20000, 0x6af96546, 7 | BRF_GRA },           //  8 K007232 Samples

	{ "870_c02.7c",		0x20000, 0x54e12c6d, 8 | BRF_GRA },           //  9 UPD7759 Samples
};

STD_ROM_PICK(fround)
STD_ROM_FN(fround)

static INT32 froundCallback()
{
	if (BurnLoadRom(Drv68KROM0 + 0x000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM0 + 0x000000,  1, 2)) return 1;
	
	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;
	
	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 1)) return 1;	

	if (BurnLoadRom(DrvGfxROM1 + 0x000000,  4, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x080000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x100000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x180000,  7, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0 + 0x000000,  8, 1)) return 1;
	if (BurnLoadRom(DrvSndROM1 + 0x000000,  9, 1)) return 1;

	return 0;
}

static INT32 froundInit()
{
	twin16_custom_video = 1;

	return DrvInit(froundCallback);
}

struct BurnDriver BurnDrvFround = {
	"fround", NULL, NULL, NULL, "1988",
	"The Final Round (ver. M)\0", NULL, "Konami", "GX870",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, froundRomInfo, froundRomName, NULL, NULL, DrvInputInfo, FroundDIPInfo,
	froundInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// The Final Round (ver. L)

static struct BurnRomInfo froundlRomDesc[] = {
	{ "870_l21.bin",	0x20000, 0xe21a3a19, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "870_l20.bin",	0x20000, 0x0ce9786f, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "870_f03.10a",	0x08000, 0xa645c727, 3 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "870_f14.d8",		0x04000, 0xc9b46615, 4 | BRF_PRG | BRF_ESS }, //  3 Characters

	{ "870c18.p18",		0x80000, 0x07927fe8, 5 | BRF_GRA },           //  4 Sprites / Bg Tiles
	{ "870c17.p16",		0x80000, 0x2bc99ff8, 5 | BRF_GRA },           //  5
	{ "870c16.p15",		0x80000, 0x41df6a1b, 5 | BRF_GRA },           //  6
	{ "870c15.p13",		0x80000, 0x8c9281df, 5 | BRF_GRA },           //  7

	{ "870_c01.5a",		0x20000, 0x6af96546, 7 | BRF_GRA },           //  8 K007232 Samples

	{ "870_c02.7c",		0x20000, 0x54e12c6d, 8 | BRF_GRA },           //  9 UPD7759 Samples
};

STD_ROM_PICK(froundl)
STD_ROM_FN(froundl)

struct BurnDriver BurnDrvFroundl = {
	"froundl", "fround", NULL, NULL, "1988",
	"The Final Round (ver. L)\0", NULL, "Konami", "GX870",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, froundlRomInfo, froundlRomName, NULL, NULL, DrvInputInfo, FroundDIPInfo,
	froundInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// Hard Puncher (Japan)

static struct BurnRomInfo hpuncherRomDesc[] = {
	{ "870_h05.6n",		0x10000, 0x2bcfeef3, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "870_h04.4n",		0x10000, 0xb9f97fd3, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "870_h09.6r",		0x10000, 0x96a4f8b1, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "870_h08.4r",		0x10000, 0x46d65156, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "870_h07.10n",	0x10000, 0xb4dda612, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "870_h06.8n",		0x10000, 0x696ba702, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "870_g03.10a",	0x08000, 0xdb9c10c8, 3 | BRF_PRG | BRF_ESS }, //  6 Z80 Code

	{ "870_f14.d8",		0x04000, 0xc9b46615, 4 | BRF_GRA },           //  7 Characters

	{ "870c17.p16",		0x80000, 0x2bc99ff8, 5 | BRF_GRA },           //  8 Sprites / Bg Tiles
	{ "870c18.p18",		0x80000, 0x07927fe8, 5 | BRF_GRA },           //  9
	{ "870c15.p13",		0x80000, 0x8c9281df, 5 | BRF_GRA },           // 10
	{ "870c16.p15",		0x80000, 0x41df6a1b, 5 | BRF_GRA },           // 11

	{ "870_c01.5a",		0x20000, 0x6af96546, 7 | BRF_GRA },           // 12 K007232 Samples

	{ "870_c02.7c",		0x20000, 0x54e12c6d, 8 | BRF_GRA },           // 13 UPD7759 Samples
};

STD_ROM_PICK(hpuncher)
STD_ROM_FN(hpuncher)

static INT32 hpuncherCallback()
{
	if (load68k(Drv68KROM0,  0)) return 1;
	
	if (BurnLoadRom(Drv68KROM1 + 0x000001,  4, 2)) return 1;
	if (BurnLoadRom(Drv68KROM1 + 0x000000,  5, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  6, 1)) return 1;
	
	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  7, 1)) return 1;	

	if (BurnLoadRom(DrvGfxROM1 + 0x000000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x080000,  9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x100000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x180000, 11, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0 + 0x000000, 12, 1)) return 1;
	if (BurnLoadRom(DrvSndROM1 + 0x000000, 13, 1)) return 1;

	return 0;
}

static INT32 hpuncherInit()
{
	twin16_custom_video = 2;

	return DrvInit(hpuncherCallback);
}

struct BurnDriver BurnDrvHpuncher = {
	"hpuncher", "fround", NULL, NULL, "1988",
	"Hard Puncher (Japan)\0", NULL, "Konami", "GX870",
	L"Hard Puncher \uFF0D\u8840\u307E\u307F\u308C\u306E\u6804\u5149\uFF0D (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, hpuncherRomInfo, hpuncherRomName, NULL, NULL, DrvInputInfo, FroundDIPInfo,
	hpuncherInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// M.I.A. - Missing in Action (Japan)

static struct BurnRomInfo miajRomDesc[] = {
	{ "808_r05.6n",		0x10000, 0x91fd83f4, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "808_r04.4n",		0x10000, 0xf1c8c597, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "808_r09.6r",		0x10000, 0xf74d4467, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "808_r08.4r",		0x10000, 0x26f21704, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "808_e07.10n",	0x10000, 0x297bdcea, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "808_e06.8n",		0x10000, 0x8f576b33, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "808_e13.10s",	0x10000, 0x1fa708f4, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "808_e12.8s",		0x10000, 0xd62f1fde, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "808_e03.10a",	0x08000, 0x3d93a7cd, 3 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "808_e14.d8",		0x04000, 0xb9d36525, 4 | BRF_GRA },           //  9 Characters

	{ "808d17.p16",		0x80000, 0xd1299082, 5 | BRF_GRA },           // 10 Sprites / Bg Tiles
	{ "808d15.p13",		0x80000, 0x2b22a6b6, 5 | BRF_GRA },           // 11

	{ "808_d01.5a",		0x20000, 0xfd4d37c0, 7 | BRF_GRA },           // 12 K007232 Samples
};

STD_ROM_PICK(miaj)
STD_ROM_FN(miaj)

static INT32 miajCallback()
{
	if (load68k(Drv68KROM0,  0)) return 1;
	if (load68k(Drv68KROM1,  4)) return 1;

	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  8, 1)) return 1;
	
	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  9, 1)) return 1;	

	if (BurnLoadRom(DrvGfxROM1 + 0x000000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x100000, 11, 1)) return 1;

	if (BurnLoadRom(DrvSndROM0 + 0x000000, 12, 1)) return 1;

	return 0;
}

static INT32 miajInit()
{
	twin16_custom_video = 2;

	return DrvInit(miajCallback);
}

struct BurnDriver BurnDrvMiaj = {
	"miaj", "mia", NULL, NULL, "1989",
	"M.I.A. - Missing in Action (Japan)\0", NULL, "Konami", "GX808",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM, 0,
	NULL, miajRomInfo, miajRomName, NULL, NULL, DrvInputInfo, MiajDIPInfo,
	miajInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};


// Cue Brick (Japan)

static struct BurnRomInfo cuebrckjRomDesc[] = {
	{ "903_e05.6n",		0x10000, 0x8b556220, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "903_e04.4n",		0x10000, 0xbf9c7927, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "903_e09.6r",		0x10000, 0x2a77554d, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "903_e08.4r",		0x10000, 0xc0a430c1, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "903_d07.10n",	0x10000, 0xfc0edce7, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "903_d06.8n",		0x10000, 0xb2cef6fe, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "903_e13.10s",	0x10000, 0x4fb5fb80, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "903_e12.8s",		0x10000, 0x883e3097, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "903_d03.10a",	0x08000, 0x455e855a, 3 | BRF_PRG | BRF_ESS }, //  8 Z80 Code

	{ "903_e14.d8",		0x04000, 0xddbebbd5, 4 | BRF_GRA },           //  9 Characters

	{ "903_e11.10r",	0x10000, 0x5c41faf8, 6 | BRF_GRA },           // 10 Sprites / Bg Tiles
	{ "903_e10.8r",		0x10000, 0x417576d4, 6 | BRF_GRA },           // 11
};

STD_ROM_PICK(cuebrckj)
STD_ROM_FN(cuebrckj)

static INT32 cuebrckjCallback()
{
	if (load68k(Drv68KROM0,  0)) return 1;
	if (load68k(Drv68KROM1,  4)) return 1;
	
	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  8, 1)) return 1;
	
	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  9, 1)) return 1;	

	if (BurnLoadRom(DrvGfxROM2 + 0x000001, 10, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x000000, 11, 2)) return 1;

	return 0;
}

static INT32 cuebrckjInit()
{
	twin16_custom_video = 2;

	return DrvInit(cuebrckjCallback);
}

struct BurnDriver BurnDrvCuebrckj = {
	"cuebrickj", "cuebrick", NULL, NULL, "1989",
	"Cue Brick (Japan)\0", NULL, "Konami", "GX903",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_PUZZLE, 0,
	NULL, cuebrckjRomInfo, cuebrckjRomName, NULL, NULL, DrvInputInfo, CuebrckjDIPInfo,
	cuebrckjInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 224, 4, 3
};
