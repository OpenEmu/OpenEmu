// FB Alpha Caveman Ninja driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "h6280_intf.h"
#include "bitswap.h"
#include "deco16ic.h"
#include "burn_ym2203.h"
#include "burn_ym2151.h"
#include "msm6295.h"
#include "timer.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvHucROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvGfxROM4;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *Drv68KRAM;
static UINT8 *DrvHucRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprRAM1;
static UINT8 *DrvSprBuf;
static UINT8 *DrvSprBuf1;
static UINT8 *DrvPrtRAM;
static UINT8 *DrvZ80RAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static INT16 *SoundBuffer;

static UINT8 *soundlatch;
static UINT8 *flipscreen;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[3];
static UINT8 DrvReset;
static UINT16 DrvInputs[3];

static INT32 scanline;
static INT32 irq_mask;
static INT32 irq_timer;

static INT32 DrvOkiBank;

static INT32 has_z80 = 0;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Drv)

static struct BurnInputInfo Robocop2InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 14,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Robocop2)

static struct BurnDIPInfo CninjaDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x12, 0x01, 0x07, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x01, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x07, 0x02, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x12, 0x01, 0x38, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x08, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x38, 0x18, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x38, 0x10, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x12, 0x01, 0x40, 0x40, "Off"			},
	{0x12, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x03, 0x01, "1"			},
	{0x13, 0x01, 0x03, 0x00, "2"			},
	{0x13, 0x01, 0x03, 0x03, "3"			},
	{0x13, 0x01, 0x03, 0x02, "4"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x0c, 0x08, "Easy"			},
	{0x13, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x13, 0x01, 0x0c, 0x04, "Hard"			},
	{0x13, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Restore Life Meter"	},
	{0x13, 0x01, 0x10, 0x10, "Off"			},
	{0x13, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Cninja)

static struct BurnDIPInfo CninjauDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x12, 0x01, 0x07, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x01, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x07, 0x02, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x12, 0x01, 0x38, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x08, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x38, 0x18, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x38, 0x10, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x12, 0x01, 0x40, 0x40, "Off"			},
	{0x12, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Credit(s) to Start"	},
	{0x12, 0x01, 0x80, 0x80, "1"			},
	{0x12, 0x01, 0x80, 0x00, "2"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x03, 0x01, "1"			},
	{0x13, 0x01, 0x03, 0x00, "2"			},
	{0x13, 0x01, 0x03, 0x03, "3"			},
	{0x13, 0x01, 0x03, 0x02, "4"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x0c, 0x08, "Easy"			},
	{0x13, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x13, 0x01, 0x0c, 0x04, "Hard"			},
	{0x13, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Restore Life Meter"	},
	{0x13, 0x01, 0x10, 0x10, "Off"			},
	{0x13, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Cninjau)

static struct BurnDIPInfo MutantfDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x12, 0x01, 0x07, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x01, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x07, 0x02, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x12, 0x01, 0x38, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x08, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x38, 0x18, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x38, 0x10, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x12, 0x01, 0x40, 0x40, "Off"			},
	{0x12, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Credit(s) to Start"	},
	{0x12, 0x01, 0x80, 0x80, "1"			},
	{0x12, 0x01, 0x80, 0x00, "2"			},

	{0   , 0xfe, 0   ,    4, "Timer Decrement"	},
	{0x13, 0x01, 0x03, 0x01, "Slow"			},
	{0x13, 0x01, 0x03, 0x03, "Normal"		},
	{0x13, 0x01, 0x03, 0x02, "Fast"			},
	{0x13, 0x01, 0x03, 0x00, "Very Fast"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x0c, 0x08, "Easy"			},
	{0x13, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x13, 0x01, 0x0c, 0x04, "Hard"			},
	{0x13, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Life Per Stage"	},
	{0x13, 0x01, 0x30, 0x00, "Least"		},
	{0x13, 0x01, 0x30, 0x10, "Little"		},
	{0x13, 0x01, 0x30, 0x20, "Less"			},
	{0x13, 0x01, 0x30, 0x30, "Normal"		},

	{0   , 0xfe, 0   ,    2, "Continues"		},
	{0x13, 0x01, 0x40, 0x00, "Off"			},
	{0x13, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Mutantf)

static struct BurnDIPInfo EdrandyDIPList[]=
{
	{0x12, 0xff, 0xff, 0xbf, NULL			},
	{0x13, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x12, 0x01, 0x07, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x01, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x07, 0x02, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x12, 0x01, 0x38, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x08, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x38, 0x18, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x38, 0x10, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x12, 0x01, 0x40, 0x00, "Off"			},
	{0x12, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Credit(s) to Start"	},
	{0x12, 0x01, 0x80, 0x80, "1"			},
	{0x12, 0x01, 0x80, 0x00, "2"			},

	{0   , 0xfe, 0   ,    4, "Player's Power"	},
	{0x13, 0x01, 0x03, 0x01, "Very Low"		},
	{0x13, 0x01, 0x03, 0x00, "Low"			},
	{0x13, 0x01, 0x03, 0x03, "Medium"		},
	{0x13, 0x01, 0x03, 0x02, "High"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x0c, 0x08, "Easy"			},
	{0x13, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x13, 0x01, 0x0c, 0x04, "Hard"			},
	{0x13, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Continues"		},
	{0x13, 0x01, 0x40, 0x00, "Off"			},
	{0x13, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Edrandy)

static struct BurnDIPInfo EdrandcDIPList[]=
{
	{0x12, 0xff, 0xff, 0xbf, NULL			},
	{0x13, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x12, 0x01, 0x07, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x01, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x07, 0x02, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x12, 0x01, 0x38, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x08, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x38, 0x18, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x38, 0x10, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x12, 0x01, 0x40, 0x00, "Off"			},
	{0x12, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Unknown"		},
	{0x12, 0x01, 0x80, 0x80, "Off"			},
	{0x12, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Player's Power"	},
	{0x13, 0x01, 0x03, 0x01, "Very Low"		},
	{0x13, 0x01, 0x03, 0x00, "Low"			},
	{0x13, 0x01, 0x03, 0x03, "Medium"		},
	{0x13, 0x01, 0x03, 0x02, "High"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x0c, 0x08, "Easy"			},
	{0x13, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x13, 0x01, 0x0c, 0x04, "Hard"			},
	{0x13, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Continues"		},
	{0x13, 0x01, 0x40, 0x00, "Off"			},
	{0x13, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Edrandc)

static struct BurnDIPInfo Robocop2DIPList[]=
{
	{0x14, 0xff, 0xff, 0xbf, NULL		},
	{0x15, 0xff, 0xff, 0x7f, NULL			},
	{0x16, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x14, 0x01, 0x07, 0x00, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x07, 0x01, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0x07, 0x02, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x14, 0x01, 0x38, 0x00, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x38, 0x08, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0x38, 0x18, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0x38, 0x10, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x01, "1"			},
	{0x15, 0x01, 0x03, 0x00, "2"			},
	{0x15, 0x01, 0x03, 0x03, "3"			},
	{0x15, 0x01, 0x03, 0x02, "4"			},

	{0   , 0xfe, 0   ,    4, "Time"			},
	{0x15, 0x01, 0x0c, 0x08, "400 Seconds"		},
	{0x15, 0x01, 0x0c, 0x0c, "300 Seconds"		},
	{0x15, 0x01, 0x0c, 0x04, "200 Seconds"		},
	{0x15, 0x01, 0x0c, 0x00, "100 Seconds"		},

	{0   , 0xfe, 0   ,    4, "Health"		},
	{0x15, 0x01, 0x30, 0x00, "17"			},
	{0x15, 0x01, 0x30, 0x10, "24"			},
	{0x15, 0x01, 0x30, 0x30, "33"			},
	{0x15, 0x01, 0x30, 0x20, "40"			},

	{0   , 0xfe, 0   ,    2, "Continues"		},
	{0x15, 0x01, 0x40, 0x00, "Off"			},
	{0x15, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Bullets"		},
	{0x16, 0x01, 0x03, 0x00, "Least"		},
	{0x16, 0x01, 0x03, 0x01, "Less"			},
	{0x16, 0x01, 0x03, 0x03, "Normal"		},
	{0x16, 0x01, 0x03, 0x02, "More"			},

	{0   , 0xfe, 0   ,    4, "Enemy Movement"	},
	{0x16, 0x01, 0x0c, 0x08, "Slow"			},
	{0x16, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x16, 0x01, 0x0c, 0x04, "Fast"			},
	{0x16, 0x01, 0x0c, 0x00, "Fastest"		},

	{0   , 0xfe, 0   ,    4, "Enemy Strength"	},
	{0x16, 0x01, 0x30, 0x20, "Less"			},
	{0x16, 0x01, 0x30, 0x30, "Normal"		},
	{0x16, 0x01, 0x30, 0x10, "More"			},
	{0x16, 0x01, 0x30, 0x00, "Most"			},

	{0   , 0xfe, 0   ,    2, "Enemy Weapon Speed"	},
	{0x16, 0x01, 0x40, 0x40, "Normal"		},
	{0x16, 0x01, 0x40, 0x00, "Fast"			},

	{0   , 0xfe, 0   ,    2, "Game Over Message"	},
	{0x16, 0x01, 0x80, 0x80, "Off"			},
	{0x16, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Robocop2)

void __fastcall cninja_main_write_word(UINT32 address, UINT16 data)
{
	deco16_write_control_word(0, address, 0x140000, data)
	deco16_write_control_word(1, address, 0x150000, data)

	switch (address)
	{
		case 0x190000:
		case 0x1a4000:
			irq_mask = data & 0xff;
		return;

		case 0x190002:
		case 0x1a4002:
		{
			scanline = data & 0xff;

			if ((!BIT(scanline,1)) && (scanline > 0) && (scanline < 240)) {
				irq_timer = scanline;
			} else {
				irq_timer = -1;
			}
		}
		return;

		case 0x1ac000:
		case 0x1b4000:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;

		case 0x17ff2a:
		case 0x198064:
		case 0x1bc0a8:
			if (has_z80) {
				*soundlatch = data & 0xff;
				ZetNmi();
			} else {
				deco16_soundlatch = data & 0xff;
				h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
			}
		break;
	}

	if ((address & 0xfffff800) == 0x198000) {
		deco16_66_prot_w(address, data, 0xffff);
		return;
	}

	if ((address & 0xffffff00) == 0x1bc000) {
		UINT16 *ProtRam = (UINT16*)deco16_prot_ram;
		ProtRam[(address - 0x1bc000) >> 1] = data;
		return;
	}
}

void __fastcall cninja_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
	//	case 0x190000:
		case 0x190001:
	//	case 0x1a4000:
		case 0x1a4001:
			irq_mask = data & 0xff;
		return;

	//	case 0x190002:
		case 0x190003:
	//	case 0x1a4002:
		case 0x1a4003:
		{
			scanline = data & 0xff;
			if ((~irq_mask & 0x02) && (scanline > 0) && (scanline < 240)) {
				irq_timer = scanline;
			} else {
				irq_timer = -1;
			}
		}
		return;

	//	case 0x1ac000:
		case 0x1ac001:
	//	case 0x1b4000:
		case 0x1b4001:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;

		case 0x198065:
		case 0x1bc0a9:
			if (has_z80) {
				*soundlatch = data;
				ZetNmi();
			} else {
				deco16_soundlatch = data;
				h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
			}
		break;
	}

	if ((address & 0xfffff800) == 0x198000) { 
		deco16_66_prot_w(address, data, 0x00ff << ((address & 1) << 3));
		return;
	}

	if ((address & 0xffffff00) == 0x1bc000) {
		DrvPrtRAM[(address & 0xff)^1] = data;

		return;
	}
	
	bprintf(PRINT_NORMAL, _T("Write Byte %x, %x\n"), address, data);
}

UINT16 __fastcall cninja_main_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x17ff22:
			return (DrvDips[1] << 8) | (DrvDips[0] << 0);

		case 0x17ff28:
			return (DrvInputs[1] & 0x07) | (deco16_vblank & 0x08);

		case 0x17ff2c:
			return DrvInputs[0];

		case 0x1a4002:
		case 0x190002:
			return scanline;

		case 0x1a4004:
		case 0x190004:
			SekSetIRQLine(3, SEK_IRQSTATUS_NONE);
			SekSetIRQLine(4, SEK_IRQSTATUS_NONE);
			return 0;
	}

	if ((address & 0xfffffc00) == 0x1bc000) {
		return deco16_104_cninja_prot_r(address);
	}

	if ((address & 0xfffff800) == 0x198000) {
		return deco16_60_prot_r(address);
	}
	
	bprintf(PRINT_NORMAL, _T("Read Word %x, %x\n"), address);

	return 0;
}

UINT8 __fastcall cninja_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x17ff22:
			return DrvDips[1]; 

		case 0x17ff23:
			return DrvDips[0];

	//	case 0x17ff28:
		case 0x17ff29:
			return (DrvInputs[1] & 0x07) | (deco16_vblank & 0x08);

		case 0x17ff2c:
			return DrvInputs[0] >> 8;

		case 0x17ff2d:
			return DrvInputs[0];

	//	case 0x1a4002:
		case 0x1a4003:
	//	case 0x190002:
		case 0x190003:
			return scanline;

	//	case 0x1a4004:
		case 0x1a4005:
	//	case 0x190004:
		case 0x190005:
			SekSetIRQLine(3, SEK_IRQSTATUS_NONE);
			SekSetIRQLine(4, SEK_IRQSTATUS_NONE);
			return 0;
	}

	if ((address & 0xfffff800) == 0x198000) {
		return deco16_60_prot_r(address) >> ((~address & 1) << 3);
	}

	if ((address & 0xfffffc00) == 0x1bc000) {
		return deco16_104_cninja_prot_r(address) >> ((~address & 1) << 3);
	}
	
	bprintf(PRINT_NORMAL, _T("Read Byte %x, %x\n"), address);

	return 0;
}

void __fastcall mutantf_main_write_word(UINT32 address, UINT16 data)
{
	deco16_write_control_word(0, address, 0x300000, data)
	deco16_write_control_word(1, address, 0x310000, data)

	switch (address)
	{
		case 0x180000:
			deco16_priority = data;
		return;

		case 0x1a0064:
			deco16_soundlatch = data & 0xff;
			h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
		break;

		case 0x1c0000:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;

		case 0x1e0000:
			memcpy (DrvSprBuf1, DrvSprRAM1, 0x800);
		return;
	}

	if ((address & 0xfffff800) == 0x1a0000) {
		deco16_66_prot_w(address, data, 0xffff);
		return;
	}
}

void __fastcall mutantf_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x180000:
		case 0x180001:
			deco16_priority = data;
		return;

		case 0x1c0000:
		case 0x1c0001:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;

		case 0x1e0000:
		case 0x1e0001:
			memcpy (DrvSprBuf1, DrvSprRAM1, 0x800);
		return;

		case 0x1a0065:
			deco16_soundlatch = data;
			h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
		break;
	}

	if ((address & 0xfffff800) == 0x1a0000) {
		deco16_66_prot_w(address, data, 0x00ff << ((address & 1) << 3));
		return;
	}
}

UINT16 __fastcall mutantf_main_read_word(UINT32 address)
{
	if ((address & 0xfffff800) == 0x1a0000) {
		return deco16_66_prot_r(address);
	}

	return 0;
}

UINT8 __fastcall mutantf_main_read_byte(UINT32 address)
{
	if (address == 0x1c0001) return deco16ic_71_read() & 0xff;

	if ((address & 0xfffff800) == 0x1a0000) {
		return deco16_66_prot_r(address) >> ((~address & 1) << 3);
	}

	return 0;
}

void __fastcall robocop2_main_write_word(UINT32 address, UINT16 data)
{
	deco16_write_control_word(0, address, 0x140000, data)
	deco16_write_control_word(1, address, 0x150000, data)

	switch (address)
	{
		case 0x1b0000:
			irq_mask = data & 0xff;
		return;

		case 0x1b0002: {
			scanline = data & 0xff;

			if ((!BIT(scanline,1)) && (scanline > 0) && (scanline < 240)) {
				irq_timer = scanline;
			} else {
				irq_timer = -1;
			}
		}
		return;

		case 0x198000:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;

		case 0x18c064:
			deco16_soundlatch = data & 0xff;
			h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
		return;;

		case 0x1f0000:
			deco16_priority = data;
		return;
	}
}

void __fastcall robocop2_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x1b0000:
		case 0x1b0001:

			irq_mask = data & 0xff;
		return;

		case 0x1b0002:
		case 0x1b0003: {
			scanline = data & 0xff;
			if ((~irq_mask & 0x02) && (scanline > 0) && (scanline < 240)) {
				irq_timer = scanline;
			} else {
				irq_timer = -1;
			}
		}
		return;

		case 0x198000:
		case 0x198001:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;

		case 0x18c065:
			deco16_soundlatch = data;
			h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
		break;

		case 0x1f0000:
		case 0x1f0001:
			deco16_priority = data;
		return;
	}
}

UINT16 __fastcall robocop2_main_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x18c41a:
			return DrvInputs[0];

		case 0x18c320:
			return (DrvInputs[1] & 0x07) | (deco16_vblank & 0x08);

		case 0x18c4e6:
			return (DrvDips[1] << 8) | (DrvDips[0] << 0);

		case 0x18c504:
			return 0x0084;

		case 0x1b0002:
			return scanline;

		case 0x1b0004:
			SekSetIRQLine(3, SEK_IRQSTATUS_NONE);
			SekSetIRQLine(4, SEK_IRQSTATUS_NONE);
			return 0;

		case 0x1f8000:
			return DrvDips[2];
	}

	return 0;
}

UINT8 __fastcall robocop2_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x18c41a:
			return DrvInputs[0] >> 8;

		case 0x18c41b:
			return DrvInputs[0] >> 0;

		case 0x18c320:
		case 0x18c321:
			return (DrvInputs[1] & 0x07) | (deco16_vblank & 0x08);

		case 0x18c4e6:
			return DrvDips[0];

		case 0x18c4e7:
			return DrvDips[1];

		case 0x18c504:
		case 0x18c505:
			return 0x0084;

		case 0x1b0002:
		case 0x1b0003:
			return scanline;

		case 0x1b0004:
		case 0x1b0005:
			SekSetIRQLine(3, SEK_IRQSTATUS_NONE);
			SekSetIRQLine(4, SEK_IRQSTATUS_NONE);
			return 0;

		case 0x1f8000:
		case 0x1f8001:
			return DrvDips[2];
	}

	return 0;
}

void __fastcall stoneage_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x8800:
			BurnYM2151SelectRegister(data);
		return;

		case 0x8801:
			BurnYM2151WriteRegister(data);
		return;

		case 0x9800:
			MSM6295Command(0, data);
		return;
	}
}

UINT8 __fastcall stoneage_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x8800:
		case 0x8801:
			return BurnYM2151ReadStatus();

		case 0x9800:
			return MSM6295ReadStatus(0);

		case 0xa000:
			ZetSetIRQLine(0x20, ZET_IRQSTATUS_NONE);
			return *soundlatch;
	}

	return 0;
}

static void DrvYM2151IrqHandler(INT32 state)
{
	ZetSetIRQLine(0, state ? ZET_IRQSTATUS_ACK : ZET_IRQSTATUS_NONE);
}

static void DrvYM2151WritePort(UINT32, UINT32 data)
{
	DrvOkiBank = data & 1;

	memcpy (DrvSndROM1, DrvSndROM1 + 0x40000 + (data & 1) * 0x40000, 0x40000);
}

static INT32 cninja_bank_callback(const INT32 bank)
{
	if ((bank >> 4) & 0xf) return 0x0000;
	return 0x1000;
}

static INT32 mutantf_1_bank_callback(const INT32 bank)
{
	return ((bank >> 4) & 0x3) << 12;
}

static INT32 mutantf_2_bank_callback(const INT32 bank)
{
	return ((bank >> 5) & 0x1) << 14;
}

static INT32 robocop2_bank_callback(const INT32 bank)
{
	return (bank & 0x30) << 8;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	if (has_z80) {
		ZetOpen(0);
		ZetReset();
		ZetClose();
		
		MSM6295Reset(0);
		MSM6295Reset(1);
		BurnYM2151Reset();
	} else {
		deco16SoundReset();
	}

	DrvYM2151WritePort(0, 0); // set initial oki bank

	deco16Reset();

	scanline = 0;
	irq_mask = 0;
	irq_timer = -1;

	return 0;
}

static void DrvBootlegCharDecode(UINT8 *gfx, INT32 len)
{
	UINT8 *dst = (UINT8*)BurnMalloc(len);

	memcpy (dst, gfx, len);

	for (INT32 r = 0; r < len; r+=4) {
		for (INT32 i = 0; i < 8; i++) {	
			INT32 t0 = (dst[r + 3] >> (7 - (i & 7))) & 1;
			INT32 t1 = (dst[r + 1] >> (7 - (i & 7))) & 1;
			INT32 t2 = (dst[r + 2] >> (7 - (i & 7))) & 1;
			INT32 t3 = (dst[r + 0] >> (7 - (i & 7))) & 1;
	
			gfx[(r * 2) + i] = (t0 << 3) | (t1 << 2) | (t2 << 1) | (t3 << 0);
		}
	}

	BurnFree (dst);
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x100000;
	DrvZ80ROM	= Next;
	DrvHucROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x200000;
	DrvGfxROM1	= Next; Next += 0x200000;
	DrvGfxROM2	= Next; Next += 0x300000;
	DrvGfxROM3	= Next; Next += 0xa00000;
	DrvGfxROM4	= Next; Next += 0x100000;

	MSM6295ROM	= Next;
	DrvSndROM0	= Next; Next += 0x100000;
	DrvSndROM1	= Next; Next += 0x0c0000;

	DrvPalette	= (UINT32*)Next; Next += 0x0800 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x008000;
	DrvHucRAM	= Next; Next += 0x002000;
	DrvSprRAM	= Next; Next += 0x000800;
	DrvSprBuf	= Next; Next += 0x000800;
	DrvSprRAM1	= Next; Next += 0x000800;
	DrvSprBuf1	= Next; Next += 0x000800;
	DrvPalRAM	= Next; Next += 0x002000;

	deco16_prot_ram	= (UINT16*)Next;
	DrvPrtRAM	= Next; Next += 0x000800;

	DrvZ80RAM	= Next; Next += 0x000800;

	soundlatch	= Next; Next += 0x000001;
	flipscreen	= Next; Next += 0x000001;

	RamEnd		= Next;
	
	SoundBuffer = (INT16*)Next; Next += nBurnSoundLen * 2 * sizeof(INT16);

	MemEnd		= Next;

	return 0;
}

static void cninja_patch()
{
	UINT16 *rom = (UINT16 *)Drv68KROM;

	for (INT32 i = 0; i < 0x80000 / 2; i++)
	{
		if (rom[i] == BURN_ENDIAN_SWAP_INT16(0x66ff) || rom[i] == BURN_ENDIAN_SWAP_INT16(0x67ff))
		{
			if (rom[i - 4] == BURN_ENDIAN_SWAP_INT16(0x0c39) || rom[i - 4] == BURN_ENDIAN_SWAP_INT16(0x0839))
			{
				rom[i - 0] = BURN_ENDIAN_SWAP_INT16(0x4e71);
				rom[i - 1] = BURN_ENDIAN_SWAP_INT16(0x4e71);
				rom[i - 2] = BURN_ENDIAN_SWAP_INT16(0x4e71);
				rom[i - 3] = BURN_ENDIAN_SWAP_INT16(0x4e71);
				rom[i - 4] = BURN_ENDIAN_SWAP_INT16(0x4e71);
			}
		}
	}
}

static INT32 CninjaInit()
{
	BurnSetRefreshRate(58.00);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x00001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x00000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x40001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x40000,  3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x80001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x80000,  5, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x00000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00001,  7, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x00000,  8, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  9, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x00000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x80000, 11, 1)) return 1;

		for (INT32 i = 0; i < 0x40000; i++) {
			INT32 n = DrvGfxROM2[i + 0x40000];
			DrvGfxROM2[i + 0x40000] = DrvGfxROM2[i + 0x80000];
			DrvGfxROM2[i + 0x80000] = n;
		}

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 12, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x000001, 13, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100000, 14, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100001, 15, 2)) return 1;

		BurnByteswap(DrvGfxROM3, 0x200000);

		if (BurnLoadRom(DrvSndROM0 + 0x00000, 16, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x40000, 17, 1)) return 1;

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x020000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x080000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x100000, 0);

		deco16_sprite_decode(DrvGfxROM3, 0x200000); // 16x16

		cninja_patch();
	}

	deco16Init(0, 1, 1);
	deco16_set_graphics(DrvGfxROM0, 0x20000 * 2, DrvGfxROM1, 0x080000 * 2, DrvGfxROM2, 0x100000 * 2);
	deco16_set_global_offsets(0, 8);

	deco16_set_color_base(2, 0x200 + 0x000);
	deco16_set_color_base(3, 0x200 + 0x300);
	deco16_set_bank_callback(2, cninja_bank_callback);
	deco16_set_bank_callback(3, cninja_bank_callback);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x0bffff, SM_ROM);
	SekMapMemory(deco16_pf_ram[0],		0x144000, 0x144fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x146000, 0x146fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x14c000, 0x14c7ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x14e000, 0x14e7ff, SM_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x154000, 0x154fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x156000, 0x156fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x15c000, 0x15c7ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x15e000, 0x15e7ff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0x184000, 0x187fff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x19c000, 0x19dfff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x1a4000, 0x1a47ff, SM_RAM);
	SekSetWriteWordHandler(0,		cninja_main_write_word);
	SekSetWriteByteHandler(0,		cninja_main_write_byte);
	SekSetReadWordHandler(0,		cninja_main_read_word);
	SekSetReadByteHandler(0,		cninja_main_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 8055000, 1, DrvYM2151WritePort, 0.45, 1006875, 0.75, 2013750, 0.60);
	BurnYM2203SetAllRoutes(0, 0.60, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 EdrandyInit()
{
	BurnSetRefreshRate(58.00);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x00001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x00000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x40001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x40000,  3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x80001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x80000,  5, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0xc0001,  6, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0xc0000,  7, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x00000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00001,  9, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x00000, 10, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000, 11, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x00000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x80000, 13, 1)) return 1;

		for (INT32 i = 0; i < 0x40000; i++) {
			INT32 n = DrvGfxROM2[i + 0x40000];
			DrvGfxROM2[i + 0x40000] = DrvGfxROM2[i + 0x80000];
			DrvGfxROM2[i + 0x80000] = n;
		}

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 14, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x000001, 15, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100000, 16, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100001, 17, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200000, 18, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200001, 19, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x300000, 20, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x300001, 21, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x400000, 22, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x400001, 23, 2)) return 1;

		BurnByteswap(DrvGfxROM3, 0x500000);

		if (BurnLoadRom(DrvSndROM0 + 0x00000, 24, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x40000, 25, 1)) return 1;

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x020000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x080000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x100000, 0);

		deco16_sprite_decode(DrvGfxROM3, 0x500000); // 16x16
	}	

	deco16Init(0, 0, 1);
	deco16_set_graphics(DrvGfxROM0, 0x20000 * 2, DrvGfxROM1, 0x080000 * 2, DrvGfxROM2, 0x100000 * 2);
	deco16_set_global_offsets(0, 8);

	deco16_set_color_base(2, 0x200 + 0x000);
	deco16_set_color_base(3, 0x200 + 0x300);
	deco16_set_bank_callback(2, cninja_bank_callback);
	deco16_set_bank_callback(3, cninja_bank_callback);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(deco16_pf_ram[0],		0x144000, 0x144fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x146000, 0x146fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x14c000, 0x14c7ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x14e000, 0x14e7ff, SM_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x154000, 0x154fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x156000, 0x156fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x15c000, 0x15c7ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x15e000, 0x15e7ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x188000, 0x189fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0x194000, 0x197fff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x1bc000, 0x1bc7ff, SM_RAM);
	SekSetWriteWordHandler(0,		cninja_main_write_word);
	SekSetWriteByteHandler(0,		cninja_main_write_byte);
	SekSetReadWordHandler(0,		cninja_main_read_word);
	SekSetReadByteHandler(0,		cninja_main_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 8055000, 1, DrvYM2151WritePort, 0.45, 1006875, 0.75, 2013750, 0.60);
	BurnYM2203SetAllRoutes(0, 0.60, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 MutantfInit()
{
	BurnSetRefreshRate(58.00);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x00001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x00000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x40001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x40000,  3, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x00000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,  5, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x00001,  6, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  7, 1)) return 1;

		memcpy (DrvGfxROM0 + 0x50000, DrvGfxROM0 + 0x10000, 0x10000);
		memcpy (DrvGfxROM0 + 0x10000, DrvGfxROM1 + 0x00000, 0x40000);
		memcpy (DrvGfxROM0 + 0x60000, DrvGfxROM1 + 0x40000, 0x40000);

		if (BurnLoadRom(DrvGfxROM2 + 0x00000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x80000,  9, 1)) return 1;

		for (INT32 i = 0; i < 0x40000; i++) {
			INT32 n = DrvGfxROM2[i + 0x40000];
			DrvGfxROM2[i + 0x40000] = DrvGfxROM2[i + 0x80000];
			DrvGfxROM2[i + 0x80000] = n;
		}

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 10, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x000001, 13, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200000, 11, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200001, 14, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x400000, 12, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x400001, 15, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM4 + 0x000001, 16, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x000003, 17, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x000000, 18, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x000002, 19, 4)) return 1;

		BurnByteswap(DrvGfxROM3, 0x500000);
		BurnByteswap(DrvGfxROM4, 0x040000);

		if (BurnLoadRom(DrvSndROM0 + 0x00000, 20, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x40000, 21, 1)) return 1;

		deco56_decrypt_gfx(DrvGfxROM0, 0xa0000);
		deco56_decrypt_gfx(DrvGfxROM1, 0x80000);

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x0a0000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x080000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x100000, 0);

		deco16_sprite_decode(DrvGfxROM3, 0x500000); // 16x16
		deco16_sprite_decode(DrvGfxROM4, 0x040000);
	}	

	deco16Init(0, 0, 1);
	deco16_set_graphics(DrvGfxROM0, 0xa0000 * 2, DrvGfxROM1, 0x080000 * 2, DrvGfxROM2, 0x100000 * 2);
	deco16_set_global_offsets(0, 8);

	deco16_set_color_base(0, 0x000);
	deco16_set_color_base(1, 0x300);
	deco16_set_color_base(2, 0x200);
	deco16_set_color_base(3, 0x400);
	deco16_set_bank_callback(0, mutantf_1_bank_callback);
	deco16_set_bank_callback(1, mutantf_2_bank_callback);
	deco16_set_bank_callback(2, mutantf_1_bank_callback);
	deco16_set_bank_callback(3, mutantf_1_bank_callback);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x100000, 0x103fff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x120000, 0x1207ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0x140000, 0x1407ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x160000, 0x161fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[0],		0x304000, 0x305fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x306000, 0x307fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x308000, 0x3087ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x30a000, 0x30a7ff, SM_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x314000, 0x315fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x316000, 0x317fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x318000, 0x3187ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x31a000, 0x31a7ff, SM_RAM);
	SekSetWriteWordHandler(0,		mutantf_main_write_word);
	SekSetWriteByteHandler(0,		mutantf_main_write_byte);
	SekSetReadWordHandler(0,		mutantf_main_read_word);
	SekSetReadByteHandler(0,		mutantf_main_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 8055000, 0, DrvYM2151WritePort, 0.45, 1006875, 0.75, 2013750, 0.60);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.45, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.45, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 CninjablInit()
{
	BurnSetRefreshRate(58.00);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x00000,  0, 1)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x80000,  1, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM  + 0x00000,  2, 1)) return 1;

		UINT8 *tmp = (UINT8*)malloc(0x400000);

		if (BurnLoadRom(tmp + 0x00000,  3, 2)) return 1;
		if (BurnLoadRom(tmp + 0x00001,  4, 2)) return 1;
		BurnByteswap(tmp, 0x400000);

		for (INT32 i = 0; i < 0x200000; i++) tmp[i] ^= 0xff;

		memcpy (DrvGfxROM0 + 0x000000, tmp + 0x000000, 0x020000);
		memcpy (DrvGfxROM1 + 0x000000, tmp + 0x080000, 0x080000);
		memcpy (DrvGfxROM2 + 0x000000, tmp + 0x180000, 0x080000);
		memcpy (DrvGfxROM2 + 0x080000, tmp + 0x100000, 0x080000);
		memcpy (DrvGfxROM3 + 0x000000, tmp + 0x200000, 0x200000);

		if (tmp) {
			free (tmp);
			tmp = NULL;
		}

		if (BurnLoadRom(DrvSndROM0  + 0x00000,  5, 1)) return 1;

		DrvBootlegCharDecode(DrvGfxROM0, 0x020000);
		deco16_sprite_decode(DrvGfxROM1, 0x080000);
		deco16_sprite_decode(DrvGfxROM2, 0x100000);
		deco16_sprite_decode(DrvGfxROM3, 0x200000);
	}

	deco16Init(0, 1, 1);
	deco16_set_graphics(DrvGfxROM0, 0x20000 * 2, DrvGfxROM1, 0x080000 * 2, DrvGfxROM2, 0x100000 * 2);
	deco16_set_global_offsets(0, 8);
	deco16_set_scroll_offs(3, 1,  2, 0);
	deco16_set_scroll_offs(2, 1,  2, 0);
	deco16_set_color_base(2, 0x200 + 0x000);
	deco16_set_color_base(3, 0x200 + 0x300);
	deco16_set_bank_callback(2, cninja_bank_callback);
	deco16_set_bank_callback(3, cninja_bank_callback);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x0bffff, SM_ROM);
	SekMapMemory(DrvSprRAM,			0x138000, 0x1387ff, SM_RAM);
	SekMapMemory(deco16_pf_ram[0],		0x144000, 0x144fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x146000, 0x146fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x14c000, 0x14c7ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x14e000, 0x14e7ff, SM_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x154000, 0x154fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x156000, 0x156fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x15c000, 0x15c7ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x15e000, 0x15e7ff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0x180000, 0x187fff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x19c000, 0x19dfff, SM_RAM);
	SekSetWriteWordHandler(0,		cninja_main_write_word);
	SekSetWriteByteHandler(0,		cninja_main_write_byte);
	SekSetReadWordHandler(0,		cninja_main_read_word);
	SekSetReadByteHandler(0,		cninja_main_read_byte);
	SekClose();

	has_z80 = 1;

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(stoneage_sound_write);
	ZetSetReadHandler(stoneage_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(3580000);
	BurnYM2151SetIrqHandler(&DrvYM2151IrqHandler);
	BurnYM2151SetAllRoutes(0.45, BURN_SND_ROUTE_BOTH);

	MSM6295Init(0, 1006875 / 132, 1);
	MSM6295Init(1, 2013750 / 132, 1);
	MSM6295SetRoute(0, 0.75, BURN_SND_ROUTE_BOTH);
	MSM6295SetRoute(1, 0.60, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 StoneageInit()
{
	BurnSetRefreshRate(58.00);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x00001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x00000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x40001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x40000,  3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x80001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x80000,  5, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM  + 0x00000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00001,  7, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x00000,  8, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  9, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x00000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x80000, 11, 1)) return 1;

		for (INT32 i = 0; i < 0x40000; i++) {
			INT32 n = DrvGfxROM2[i + 0x40000];
			DrvGfxROM2[i + 0x40000] = DrvGfxROM2[i + 0x80000];
			DrvGfxROM2[i + 0x80000] = n;
		}


		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 12, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x000001, 13, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100000, 14, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100001, 15, 2)) return 1;

		BurnByteswap(DrvGfxROM3, 0x200000);

		if (BurnLoadRom(DrvSndROM0 + 0x00000, 16, 1)) return 1;

		memset (DrvSndROM1, 0xff, 0x80000);

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x020000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x080000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x100000, 0);

		deco16_sprite_decode(DrvGfxROM3, 0x200000); // 16x16
	}	

	deco16Init(0, 1, 1);
	deco16_set_graphics(DrvGfxROM0, 0x20000 * 2, DrvGfxROM1, 0x080000 * 2, DrvGfxROM2, 0x100000 * 2);
	deco16_set_global_offsets(0, 8);
	deco16_set_scroll_offs(3, 1, 10, 0);
	deco16_set_scroll_offs(1, 1, 10, 0);
	deco16_set_scroll_offs(0, 1, -2, 0);
	deco16_set_color_base(2, 0x200 + 0x000);
	deco16_set_color_base(3, 0x200 + 0x300);
	deco16_set_bank_callback(2, cninja_bank_callback);
	deco16_set_bank_callback(3, cninja_bank_callback);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x0bffff, SM_ROM);
	SekMapMemory(deco16_pf_ram[0],		0x144000, 0x144fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x146000, 0x146fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x14c000, 0x14c7ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x14e000, 0x14e7ff, SM_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x154000, 0x154fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x156000, 0x156fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x15c000, 0x15c7ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x15e000, 0x15e7ff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0x184000, 0x187fff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x19c000, 0x19dfff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x1a4000, 0x1a47ff, SM_RAM);
	SekSetWriteWordHandler(0,		cninja_main_write_word);
	SekSetWriteByteHandler(0,		cninja_main_write_byte);
	SekSetReadWordHandler(0,		cninja_main_read_word);
	SekSetReadByteHandler(0,		cninja_main_read_byte);
	SekClose();

	has_z80 = 1;

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(stoneage_sound_write);
	ZetSetReadHandler(stoneage_sound_read);
	ZetMemEnd();
	ZetClose();

	MSM6295Init(0, 1006875 / 132, 1);
	MSM6295Init(1, 2013750 / 132, 1);
	MSM6295SetRoute(0, 0.75, BURN_SND_ROUTE_BOTH);
	MSM6295SetRoute(1, 0.60, BURN_SND_ROUTE_BOTH);

	BurnYM2151Init(3580000);
	BurnYM2151SetAllRoutes(0.45, BURN_SND_ROUTE_BOTH);
	BurnYM2151SetIrqHandler(&DrvYM2151IrqHandler);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 Robocop2Init()
{
	BurnSetRefreshRate(58.00);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x00001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x00000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x40001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x40000,  3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x80001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x80000,  5, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0xc0001,  6, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0xc0000,  7, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x00000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00001,  9, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x00000, 10, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x80000, 12, 1)) return 1;

		for (INT32 i = 0; i < 0x40000; i++) {
			INT32 n = DrvGfxROM1[i + 0x40000];
			DrvGfxROM1[i + 0x40000] = DrvGfxROM1[i + 0x80000];
			DrvGfxROM1[i + 0x80000] = n;
		}

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 13, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x000000, DrvGfxROM3 + 0x000000, 0x040000);
		memcpy (DrvGfxROM2 + 0x0c0000, DrvGfxROM3 + 0x040000, 0x040000);

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 14, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x040000, DrvGfxROM3 + 0x000000, 0x040000);
		memcpy (DrvGfxROM2 + 0x100000, DrvGfxROM3 + 0x040000, 0x040000);

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 15, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x080000, DrvGfxROM3 + 0x000000, 0x040000);
		memcpy (DrvGfxROM2 + 0x140000, DrvGfxROM3 + 0x040000, 0x040000);

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 16, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x000001, 17, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100000, 18, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100001, 19, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200000, 20, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200001, 21, 2)) return 1;

		BurnByteswap(DrvGfxROM3, 0x300000);

		if (BurnLoadRom(DrvSndROM0 + 0x00000, 22, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x40000, 23, 1)) return 1;

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x020000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x100000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM4, 0x180000, 2);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x180000, 0);

		deco16_sprite_decode(DrvGfxROM3, 0x300000); // 16x16
	}	

	deco16Init(0, 0, 1);
	deco16_set_graphics(DrvGfxROM0, 0x20000 * 2, DrvGfxROM1, 0x100000 * 2, DrvGfxROM2, 0x180000 * 2);
	deco16_set_global_offsets(0, 8);

	deco16_set_color_base(2, 0x200 + 0x000);
	deco16_set_color_base(3, 0x200 + 0x300);
	deco16_set_bank_callback(1, robocop2_bank_callback);
	deco16_set_bank_callback(2, robocop2_bank_callback);
	deco16_set_bank_callback(3, robocop2_bank_callback);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(deco16_pf_ram[0],		0x144000, 0x144fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x146000, 0x146fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x14c000, 0x14c7ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x14e000, 0x14e7ff, SM_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x154000, 0x154fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x156000, 0x156fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x15c000, 0x15c7ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x15e000, 0x15e7ff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x180000, 0x1807ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x1a8000, 0x1a9fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0x1b8000, 0x1bbfff, SM_RAM);
	SekSetWriteWordHandler(0,		robocop2_main_write_word);
	SekSetWriteByteHandler(0,		robocop2_main_write_byte);
	SekSetReadWordHandler(0,		robocop2_main_read_word);
	SekSetReadByteHandler(0,		robocop2_main_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 8055000, 1, DrvYM2151WritePort, 0.45, 1006875, 0.75, 2013750, 0.60);
	BurnYM2203SetAllRoutes(0, 0.60, BURN_SND_ROUTE_BOTH);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.45, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.45, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();
	deco16Exit();

	SekExit();

	if (has_z80) {
		ZetExit();
		has_z80 = 0;
		MSM6295Exit(0);
		MSM6295Exit(1);
		BurnYM2151Exit();
	} else {
		deco16SoundExit();
	}

	BurnFree (AllMem);

	MSM6295ROM = NULL;

	return 0;
}

static void cninja_draw_sprites()
{
	UINT16 *buffered_spriteram = (UINT16*)DrvSprBuf;

	for (INT32 offs = 0x400 - 4; offs >=0 ; offs -= 4)
	{
		INT32 x, y, sprite, color, multi, flipx, flipy, inc, flash, mult, pri = 0;
		sprite = BURN_ENDIAN_SWAP_INT16(buffered_spriteram[offs + 1]);
		if (!sprite)
			continue;

		x = BURN_ENDIAN_SWAP_INT16(buffered_spriteram[offs + 2]);

		switch (x & 0xc000)
		{
			case 0x0000: pri = 0; break;
			case 0x4000: pri = 0xf0; break;
			case 0x8000: pri = 0xf0 | 0xcc; break;
			case 0xc000: pri = 0xf0 | 0xcc; break;
		}

		y = BURN_ENDIAN_SWAP_INT16(buffered_spriteram[offs]);
		flash = y & 0x1000;
		if (flash && (nCurrentFrame & 1))
			continue;

		color = (x >> 9) & 0x1f;

		flipx = y & 0x2000;
		flipy = y & 0x4000;
		multi = (1 << ((y & 0x0600) >> 9)) - 1;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= nScreenWidth) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		sprite &= ~multi;
		if (flipy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (*flipscreen)
		{
			y = 240 - y;
			x = (nScreenWidth - 16) - x;
			if (flipx) flipx = 0; else flipx = 1;
			if (flipy) flipy = 0; else flipy = 1;
			mult = 16;
		}
		else
			mult = -16;

		while (multi >= 0)
		{
			deco16_draw_prio_sprite(pTransDraw, DrvGfxROM3, sprite - multi * inc, (color << 4) + 0x300, x, y + mult * multi, flipx, flipy, pri);

			multi--;
		}
	}
}

static void cninjabl_draw_sprites()
{
	UINT16 *buffered_spriteram = (UINT16*)DrvSprBuf;
	INT32 offs;
	INT32 endoffs;

	endoffs = 0x400 - 4;
	for (offs = 0; offs < 0x400 - 4 ; offs += 4)
	{
		INT32 y = BURN_ENDIAN_SWAP_INT16(buffered_spriteram[offs + 1]);

		if (y == 0x180)
		{
			endoffs = offs;
			offs = 0x400 - 4;
		}
	}

	for (offs = endoffs; offs >=0 ; offs -= 4)
	{
		INT32 x, y, sprite, colour, multi, fx, fy, inc, flash, mult, pri = 0;

		sprite = BURN_ENDIAN_SWAP_INT16(buffered_spriteram[offs + 0]);
		y = BURN_ENDIAN_SWAP_INT16(buffered_spriteram[offs + 1]);

		if (!sprite)
			continue;

		x = BURN_ENDIAN_SWAP_INT16(buffered_spriteram[offs + 2]);

		switch (x & 0xc000)
		{
			case 0x0000: pri = 0; break;
			case 0x4000: pri = 0xf0; break;
			case 0x8000: pri = 0xf0 | 0xcc; break;
			case 0xc000: pri = 0xf0 | 0xcc; break;
		}

		flash = y & 0x1000;
		if (flash && (nCurrentFrame & 1))
			continue;

		colour = (x >> 9) & 0x1f;

		fx = y & 0x2000;
		fy = y & 0x4000;

		multi = (1 << ((y & 0x0600) >> 9)) - 1;

		y -= multi * 16;
		y += 4;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (*flipscreen)
		{
			y = 240 - y;
			x = 240 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else
			mult = -16;

		while (multi >= 0)
		{
			deco16_draw_prio_sprite(pTransDraw, DrvGfxROM3, sprite - multi * inc, (colour << 4) + 0x300, x, y + mult * multi, fx, fy, pri);

			multi--;
		}
	}
}

static void mutantf_draw_sprites(UINT8 *ram, UINT8 *gfx, INT32 colbank, INT32 gfxbank)
{
	UINT16 *spriteptr = (UINT16*)ram;

	INT32 offs, end, inc;

	if (gfxbank == 4)
	{
		offs = 0;
		end = 0x400;
		inc = 4;
	}
	else
	{
		offs = 0x3fc;
		end = -4;
		inc = -4;
	}

	while (offs != end)
	{
		INT32 x, y, sprite, colour, fx, fy, w, h, sx, sy, x_mult, y_mult;
		INT32 alpha = 0xff;

		sprite = BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 3]);
		if (!sprite)
		{
			offs += inc;
			continue;
		}

		sx = BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 1]);

		h = (BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 2]) & 0xf000) >> 12;
		w = (BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 2]) & 0x0f00) >>  8;

		sy = BURN_ENDIAN_SWAP_INT16(spriteptr[offs]);
		if ((sy & 0x2000) && (nCurrentFrame & 1))
		{
			offs += inc;
			continue;
		}

		colour = (BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 2]) >> 0) & 0x1f;

		if (gfxbank == 4)
		{
			alpha = 0x80;
			colour &= 0xf;
		}

		fx = (BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 0]) & 0x4000);
		fy = (BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 0]) & 0x8000);

		if (*flipscreen)
		{
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;

			sx = sx & 0x01ff;
			sy = sy & 0x01ff;
			if (sx > 0x180) sx = -(0x200 - sx);
			if (sy > 0x180) sy = -(0x200 - sy);

			if (fx) { x_mult = -16; sx += 16 * w; } else { x_mult = 16; sx -= 16; }
			if (fy) { y_mult = -16; sy += 16 * h; } else { y_mult = 16; sy -= 16; }
		}
		else
		{
			sx = sx & 0x01ff;
			sy = sy & 0x01ff;
			if (sx & 0x100) sx = -(0x100 - (sx & 0xff));
			if (sy & 0x100) sy = -(0x100 - (sy & 0xff));
			sx = 304 - sx;
			sy = 240 - sy;
			if (sx >= 432) sx -= 512;
			if (sy >= 384) sy -= 512;
			if (fx) { x_mult = -16; sx += 16; } else { x_mult = 16; sx -= 16*w; }
			if (fy) { y_mult = -16; sy += 16; } else { y_mult = 16; sy -= 16*h; }
		}

		for (x = 0; x < w; x++)
		{
			for (y = 0; y < h; y++)
			{
				// needs alpha blending... 
				deco16_draw_prio_sprite(pTransDraw, gfx, sprite + y + h * x, (colour << 4) + colbank, sx + x_mult * (w-x), sy + y_mult * (h-y), fx, fy, 0);
			}
		}
		offs += inc;
	}
}

static INT32 CninjaDraw()
{
//	if (DrvRecalc) {
		deco16_palette_recalculate(DrvPalette, DrvPalRAM);
		DrvRecalc = 0;
//	}

	deco16_pf12_update();
	deco16_pf34_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x200;
	}

	deco16_clear_prio_map();

	if (nSpriteEnable &  1) deco16_draw_layer(3, pTransDraw, DECO16_LAYER_PRIORITY(0x01) | DECO16_LAYER_OPAQUE);
	if (nSpriteEnable &  2) deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x02));
	if (nSpriteEnable &  4) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x02) | DECO16_LAYER_TRANSMASK1);
	if (nSpriteEnable &  8) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x04) | DECO16_LAYER_TRANSMASK0);
 
	cninja_draw_sprites();

	if (nSpriteEnable & 16) deco16_draw_layer(0, pTransDraw, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 CninjablDraw()
{
//	if (DrvRecalc) {
		deco16_palette_recalculate(DrvPalette, DrvPalRAM);
		DrvRecalc = 0;
//	}

	deco16_pf12_update();
	deco16_pf34_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x200;
	}

	deco16_clear_prio_map();

	if (nSpriteEnable &  1) deco16_draw_layer(3, pTransDraw, DECO16_LAYER_PRIORITY(0x01) | DECO16_LAYER_OPAQUE);
	if (nSpriteEnable &  2) deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x02));
	if (nSpriteEnable &  4) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x02) | DECO16_LAYER_TRANSMASK1);
	if (nSpriteEnable &  8) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x04) | DECO16_LAYER_TRANSMASK0);
 
	cninjabl_draw_sprites();

	if (nSpriteEnable & 16) deco16_draw_layer(0, pTransDraw, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 EdrandyDraw()
{
//	if (DrvRecalc) {
		deco16_palette_recalculate(DrvPalette, DrvPalRAM);
		DrvRecalc = 0;
//	}

	deco16_pf12_update();
	deco16_pf34_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0;
	}

	deco16_clear_prio_map();

	if (nSpriteEnable &  1) deco16_draw_layer(3, pTransDraw, DECO16_LAYER_PRIORITY(0x01) | DECO16_LAYER_OPAQUE);
	if (nSpriteEnable &  2) deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x02));
	if (nSpriteEnable &  4) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x04));
 
	cninja_draw_sprites();

	if (nSpriteEnable &  8) deco16_draw_layer(0, pTransDraw, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 Robocop2Draw()
{
//	if (DrvRecalc) {
		deco16_palette_recalculate(DrvPalette, DrvPalRAM);
		DrvRecalc = 0;
//	}

	deco16_pf12_update();
	deco16_pf34_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x200;
	}

	deco16_clear_prio_map();

	INT32 layer_8bpp = 0;

	if (deco16_priority & 4)
	{
		deco16_set_color_mask(2, 0);
		deco16_set_color_mask(3, 0);
		deco16_set_graphics(2, DrvGfxROM4, 0x100000, 16);
		layer_8bpp = DECO16_LAYER_8BITSPERPIXEL;
	}
	else
	{
		deco16_set_color_mask(2, 0xf);
		deco16_set_color_mask(3, 0xf);
		deco16_set_graphics(2, DrvGfxROM2, 0x300000, 16);

		if (nSpriteEnable &  1) deco16_draw_layer(3, pTransDraw, DECO16_LAYER_OPAQUE | DECO16_LAYER_PRIORITY(0x01));
	}

	if (deco16_priority & 8) {
		if (nSpriteEnable &  2) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x02));
		if (nSpriteEnable &  4) deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x04) | layer_8bpp);
	} else {
		if (nSpriteEnable &  2) deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x02) | layer_8bpp);
		if (nSpriteEnable &  4) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x04));
	}

	cninja_draw_sprites();

	if (nSpriteEnable &  8) deco16_draw_layer(0, pTransDraw, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}


static INT32 MutantfDraw()
{
//	if (DrvRecalc) {
		deco16_palette_recalculate(DrvPalette, DrvPalRAM);
		DrvRecalc = 0;
//	}

	deco16_pf12_update();
	deco16_pf34_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x400;
	}

	if (nSpriteEnable &  1) deco16_draw_layer(3, pTransDraw, DECO16_LAYER_OPAQUE);
	if (nSpriteEnable &  2) deco16_draw_layer(1, pTransDraw, 0);
	if (nSpriteEnable &  4) deco16_draw_layer(2, pTransDraw, 0);
 
	if (deco16_priority & 1)
	{
		deco16_clear_prio_map();
		mutantf_draw_sprites(DrvSprBuf,  DrvGfxROM3, 0x100, 3);
		deco16_clear_prio_map();
		mutantf_draw_sprites(DrvSprBuf1, DrvGfxROM4, 0x700, 4);
	}
	else
	{
		deco16_clear_prio_map();
		mutantf_draw_sprites(DrvSprBuf1, DrvGfxROM4, 0x700, 4);
		deco16_clear_prio_map();
		mutantf_draw_sprites(DrvSprBuf,  DrvGfxROM3, 0x100, 3);
	}

	if (nSpriteEnable & 8) deco16_draw_layer(0, pTransDraw, 0); 

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 CninjaFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		deco16_prot_inputs = DrvInputs;
		memset (DrvInputs, 0xff, 2 * sizeof(INT16)); 
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
		DrvInputs[2] = (DrvDips[1] << 8) | (DrvDips[0] << 0);
	}

	INT32 nInterleave = 256;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 12000000 / 58, 8055000 / 58 };
	INT32 nCyclesDone[2] = { 0, 0 };

	h6280NewFrame();
	
	SekOpen(0);
	h6280Open(0);

	deco16_vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);
		nCyclesDone[1] += h6280Run(nCyclesTotal[1] / nInterleave);

		if (irq_timer == i) {
			SekSetIRQLine((irq_mask & 0x10) ? 3 : 4, SEK_IRQSTATUS_ACK);
			irq_timer = -1;
		}
		if (i == 248) deco16_vblank = 0x08;
		
		INT32 nSegmentLength = nBurnSoundLen / nInterleave;
		INT16* pSoundBuf = SoundBuffer + (nSoundBufferPos << 1);
		deco16SoundUpdate(pSoundBuf, nSegmentLength);
		nSoundBufferPos += nSegmentLength;
	}

	SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
	BurnTimerEndFrame(nCyclesTotal[1]);

	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = SoundBuffer + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			deco16SoundUpdate(pSoundBuf, nSegmentLength);
		}
		
		for (INT32 i = 0; i < nBurnSoundLen; i++) {
			pBurnSoundOut[(i << 1) + 0] += SoundBuffer[(i << 1) + 0];
			pBurnSoundOut[(i << 1) + 1] += SoundBuffer[(i << 1) + 1];
		}
	}

	h6280Close();
	SekClose();

	if (pBurnDraw) {
		BurnDrvRedraw();
	}

	return 0;
}

static INT32 MutantfFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		deco16_prot_inputs = DrvInputs;
		memset (DrvInputs, 0xff, 2 * sizeof(INT16)); 
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
		DrvInputs[2] = (DrvDips[1] << 8) | (DrvDips[0] << 0);
	}

	INT32 nInterleave = 256;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 14000000 / 58, 8055000 / 58 };
	INT32 nCyclesDone[2] = { 0, 0 };

	h6280NewFrame();
	
	SekOpen(0);
	h6280Open(0);

	deco16_vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);
		nCyclesDone[1] += h6280Run(nCyclesTotal[1] / nInterleave);

		if (i == 240) deco16_vblank = 0x08;
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			deco16SoundUpdate(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			deco16SoundUpdate(pSoundBuf, nSegmentLength);
		}
	}
	
	h6280Close();
	SekClose();

	if (pBurnDraw) {
		MutantfDraw();
	}

	return 0;
}

static INT32 StoneageFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		deco16_prot_inputs = DrvInputs;
		memset (DrvInputs, 0xff, 2 * sizeof(INT16)); 
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
		DrvInputs[2] = (DrvDips[1] << 8) | (DrvDips[0] << 0);
	}

	INT32 nInterleave = 256;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 12000000 / 58, 3579545 / 58 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	deco16_vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);
		nCyclesDone[1] += ZetRun(nCyclesTotal[1] / nInterleave);

		if (irq_timer == i) {
			SekSetIRQLine((irq_mask & 0x10) ? 3 : 4, SEK_IRQSTATUS_ACK);
			irq_timer = -1;
		}
		if (i == 248) deco16_vblank = 0x08;
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			MSM6295Render(1, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			MSM6295Render(1, pSoundBuf, nSegmentLength);
		}
	}

	ZetClose();
	SekClose();

	if (pBurnDraw) {
		BurnDrvRedraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029722;
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
	
		deco16SoundScan(nAction, pnMin);

		deco16Scan();

		SCAN_VAR(scanline);
		SCAN_VAR(irq_mask);
		SCAN_VAR(irq_timer);

		SCAN_VAR(DrvOkiBank);
		DrvYM2151WritePort(0, DrvOkiBank);
	}

	return 0;
}

static INT32 StoneageScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029722;
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
		BurnYM2151Scan(nAction);
		MSM6295Scan(0, nAction);
		MSM6295Scan(1, nAction);
	
		deco16Scan();

		SCAN_VAR(scanline);
		SCAN_VAR(irq_mask);
		SCAN_VAR(irq_timer);

		SCAN_VAR(DrvOkiBank);
	}

	return 0;
}


// Caveman Ninja (World ver 4)

static struct BurnRomInfo cninjaRomDesc[] = {
	{ "gn-02-3.1k",		0x020000, 0x39aea12a, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gn-05-2.3k",		0x020000, 0x0f4360ef, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "gn-01-2.1j",		0x020000, 0xf740ef7e, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "gn-04-2.3j",		0x020000, 0xc98fcb62, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "gn-00.rom",		0x020000, 0x0b110b16, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "gn-03.rom",		0x020000, 0x1e28e697, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "gl-07.rom",		0x010000, 0xca8bef96, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "gl-08.rom",		0x010000, 0x33a2b400, 3 | BRF_GRA }, 	       //  7 Characters
	{ "gl-09.rom",		0x010000, 0x5a2d4752, 3 | BRF_GRA }, 	       //  8

	{ "mag-02.rom",		0x080000, 0xde89c69a, 4 | BRF_GRA }, 	       //  9 Foreground Tiles

	{ "mag-00.rom",		0x080000, 0xa8f05d33, 5 | BRF_GRA }, 	       // 10 Background Tiles
	{ "mag-01.rom",		0x080000, 0x5b399eed, 5 | BRF_GRA }, 	       // 11

	{ "mag-03.rom",		0x080000, 0x2220eb9f, 6 | BRF_GRA }, 	       // 12 Sprites
	{ "mag-05.rom",		0x080000, 0x56a53254, 6 | BRF_GRA }, 	       // 13
	{ "mag-04.rom",		0x080000, 0x144b94cc, 6 | BRF_GRA }, 	       // 14
	{ "mag-06.rom",		0x080000, 0x82d44749, 6 | BRF_GRA }, 	       // 15

	{ "gl-06.rom",		0x020000, 0xd92e519d, 7 | BRF_SND }, 	       // 16 OKI M6295 Samples 0

	{ "mag-07.rom",		0x080000, 0x08eb5264, 8 | BRF_SND }, 	       // 17 OKI M6295 Samples 1

	{ "mb7122h.7v",		0x000400, 0xa1267336, 0 | BRF_OPT }, 	       // 18 Unused PROMs
};

STD_ROM_PICK(cninja)
STD_ROM_FN(cninja)

struct BurnDriver BurnDrvCninja = {
	"cninja", NULL, NULL, NULL, "1991",
	"Caveman Ninja (World ver 4)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM, 0,
	NULL, cninjaRomInfo, cninjaRomName, NULL, NULL, DrvInputInfo, CninjaDIPInfo,
	CninjaInit, DrvExit, CninjaFrame, CninjaDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};

// Caveman Ninja (alternate)

static struct BurnRomInfo cninjaaRomDesc[] = {
	{ "1.1k",			0x020000, 0xa6c40959, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "4.3k",			0x020000, 0x2e01d1fd, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "2.2j",			0x020000, 0x18f0527c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "5.3j",			0x020000, 0xea4b6d53, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "gn-00.rom",		0x020000, 0x0b110b16, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "gn-03.rom",		0x020000, 0x1e28e697, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "gl-07.rom",		0x010000, 0xca8bef96, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "gl-08.rom",		0x010000, 0x33a2b400, 3 | BRF_GRA }, 	       //  7 Characters
	{ "gl-09.rom",		0x010000, 0x5a2d4752, 3 | BRF_GRA }, 	       //  8

	{ "mag-02.rom",		0x080000, 0xde89c69a, 4 | BRF_GRA }, 	       //  9 Foreground Tiles
 
	{ "mag-00.rom",		0x080000, 0xa8f05d33, 5 | BRF_GRA }, 	       // 10 Background Tiles
	{ "mag-01.rom",		0x080000, 0x5b399eed, 5 | BRF_GRA }, 	       // 11

	{ "mag-03.rom",		0x080000, 0x2220eb9f, 6 | BRF_GRA }, 	       // 12 Sprites
	{ "mag-05.rom",		0x080000, 0x56a53254, 6 | BRF_GRA }, 	       // 13
	{ "mag-04.rom",		0x080000, 0x144b94cc, 6 | BRF_GRA }, 	       // 14
	{ "mag-06.rom",		0x080000, 0x82d44749, 6 | BRF_GRA }, 	       // 15

	{ "gl-06.rom",		0x020000, 0xd92e519d, 7 | BRF_SND }, 	       // 16 OKI M6295 Samples 0

	{ "mag-07.rom",		0x080000, 0x08eb5264, 8 | BRF_SND }, 	       // 17 OKI M6295 Samples 1

	{ "mb7122h.7v",		0x000400, 0xa1267336, 0 | BRF_OPT }, 	       // 18 Unused PROMs
};

STD_ROM_PICK(cninjaa)
STD_ROM_FN(cninjaa)

struct BurnDriver BurnDrvCninjaa = {
	"cninjaa", "cninja", NULL, NULL, "1991",
	"Caveman Ninja (alternate)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM, 0,
	NULL, cninjaaRomInfo, cninjaaRomName, NULL, NULL, DrvInputInfo, CninjaDIPInfo,
	CninjaInit, DrvExit, CninjaFrame, CninjaDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Caveman Ninja (World ver 1)

static struct BurnRomInfo cninja1RomDesc[] = {
	{ "gn-02.1k",		0x020000, 0xccc59524, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gn-05.3k",		0x020000, 0xa002cbe4, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "gn-01.1j",		0x020000, 0x18f0527c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "gn-04.3j",		0x020000, 0xea4b6d53, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "gn-00.rom",		0x020000, 0x0b110b16, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "gn-03.rom",		0x020000, 0x1e28e697, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "gl-07.rom",		0x010000, 0xca8bef96, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "gl-08.rom",		0x010000, 0x33a2b400, 3 | BRF_GRA }, 	       //  7 Characters
	{ "gl-09.rom",		0x010000, 0x5a2d4752, 3 | BRF_GRA }, 	       //  8

	{ "mag-02.rom",		0x080000, 0xde89c69a, 4 | BRF_GRA }, 	       //  9 Foreground Tiles
 
	{ "mag-00.rom",		0x080000, 0xa8f05d33, 5 | BRF_GRA }, 	       // 10 Background Tiles
	{ "mag-01.rom",		0x080000, 0x5b399eed, 5 | BRF_GRA }, 	       // 11

	{ "mag-03.rom",		0x080000, 0x2220eb9f, 6 | BRF_GRA }, 	       // 12 Sprites
	{ "mag-05.rom",		0x080000, 0x56a53254, 6 | BRF_GRA }, 	       // 13
	{ "mag-04.rom",		0x080000, 0x144b94cc, 6 | BRF_GRA }, 	       // 14
	{ "mag-06.rom",		0x080000, 0x82d44749, 6 | BRF_GRA }, 	       // 15

	{ "gl-06.rom",		0x020000, 0xd92e519d, 7 | BRF_SND }, 	       // 16 OKI M6295 Samples 0

	{ "mag-07.rom",		0x080000, 0x08eb5264, 8 | BRF_SND }, 	       // 17 OKI M6295 Samples 1

	{ "mb7122h.7v",		0x000400, 0xa1267336, 0 | BRF_OPT }, 	       // 18 Unused PROMs
};

STD_ROM_PICK(cninja1)
STD_ROM_FN(cninja1)

struct BurnDriver BurnDrvCninja1 = {
	"cninja1", "cninja", NULL, NULL, "1991",
	"Caveman Ninja (World ver 1)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM, 0,
	NULL, cninja1RomInfo, cninja1RomName, NULL, NULL, DrvInputInfo, CninjaDIPInfo,
	CninjaInit, DrvExit, CninjaFrame, CninjaDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Caveman Ninja (US ver 4)

static struct BurnRomInfo cninjauRomDesc[] = {
	{ "gm-02-3.1k",		0x020000, 0xd931c3b1, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gm-05-2.3k",		0x020000, 0x7417d3fb, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "gm-01-2.1j",		0x020000, 0x72041f7e, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "gm-04-2.3j",		0x020000, 0x2104d005, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "gn-00.rom",		0x020000, 0x0b110b16, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "gn-03.rom",		0x020000, 0x1e28e697, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "gl-07.rom",		0x010000, 0xca8bef96, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "gl-08.rom",		0x010000, 0x33a2b400, 3 | BRF_GRA }, 	       //  7 Characters
	{ "gl-09.rom",		0x010000, 0x5a2d4752, 3 | BRF_GRA }, 	       //  8

	{ "mag-02.rom",		0x080000, 0xde89c69a, 4 | BRF_GRA }, 	       //  9 Foreground Tiles

	{ "mag-00.rom",		0x080000, 0xa8f05d33, 5 | BRF_GRA }, 	       // 10 Background Tiles
	{ "mag-01.rom",		0x080000, 0x5b399eed, 5 | BRF_GRA }, 	       // 11

	{ "mag-03.rom",		0x080000, 0x2220eb9f, 6 | BRF_GRA }, 	       // 12 Sprites
	{ "mag-05.rom",		0x080000, 0x56a53254, 6 | BRF_GRA }, 	       // 13
	{ "mag-04.rom",		0x080000, 0x144b94cc, 6 | BRF_GRA }, 	       // 14
	{ "mag-06.rom",		0x080000, 0x82d44749, 6 | BRF_GRA }, 	       // 15

	{ "gl-06.rom",		0x020000, 0xd92e519d, 7 | BRF_SND }, 	       // 16 OKI M6295 Samples 0

	{ "mag-07.rom",		0x080000, 0x08eb5264, 8 | BRF_SND }, 	       // 17 OKI M6295 Samples 1

	{ "mb7122h.7v",		0x000400, 0xa1267336, 0 | BRF_OPT }, 	       // 18 Unused PROMs
};

STD_ROM_PICK(cninjau)
STD_ROM_FN(cninjau)

struct BurnDriver BurnDrvCninjau = {
	"cninjau", "cninja", NULL, NULL, "1991",
	"Caveman Ninja (US ver 4)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM, 0,
	NULL, cninjauRomInfo, cninjauRomName, NULL, NULL, DrvInputInfo, CninjauDIPInfo,
	CninjaInit, DrvExit, CninjaFrame, CninjaDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Tatakae Genshizin Joe & Mac (Japan ver 1)

static struct BurnRomInfo joemacRomDesc[] = {
	{ "gl-02-2.1k",		0x020000, 0x80da12e2, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gl-05-2.3k",		0x020000, 0xfe4dbbbb, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "gl-01-2.1j",		0x020000, 0x0b245307, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "gl-04-2.3j",		0x020000, 0x1b331f61, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "gn-00.rom",		0x020000, 0x0b110b16, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "gn-03.rom",		0x020000, 0x1e28e697, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "gl-07.rom",		0x010000, 0xca8bef96, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "gl-08.rom",		0x010000, 0x33a2b400, 3 | BRF_GRA }, 	       //  7 Characters
	{ "gl-09.rom",		0x010000, 0x5a2d4752, 3 | BRF_GRA }, 	       //  8

	{ "mag-02.rom",		0x080000, 0xde89c69a, 4 | BRF_GRA }, 	       //  9 Foreground Tiles

	{ "mag-00.rom",		0x080000, 0xa8f05d33, 5 | BRF_GRA }, 	       // 10 Background Tiles
	{ "mag-01.rom",		0x080000, 0x5b399eed, 5 | BRF_GRA }, 	       // 11

	{ "mag-03.rom",		0x080000, 0x2220eb9f, 6 | BRF_GRA }, 	       // 12 Sprites
	{ "mag-05.rom",		0x080000, 0x56a53254, 6 | BRF_GRA }, 	       // 13
	{ "mag-04.rom",		0x080000, 0x144b94cc, 6 | BRF_GRA }, 	       // 14
	{ "mag-06.rom",		0x080000, 0x82d44749, 6 | BRF_GRA }, 	       // 15

	{ "gl-06.rom",		0x020000, 0xd92e519d, 7 | BRF_SND }, 	       // 16 OKI M6295 Samples 0

	{ "mag-07.rom",		0x080000, 0x08eb5264, 8 | BRF_SND }, 	       // 17 OKI M6295 Samples 1

	{ "mb7122h.7v",		0x000400, 0xa1267336, 0 | BRF_OPT }, 	       // 18 Unused PROMs
};

STD_ROM_PICK(joemac)
STD_ROM_FN(joemac)

struct BurnDriver BurnDrvJoemac = {
	"joemac", "cninja", NULL, NULL, "1991",
	"Tatakae Genshizin Joe & Mac (Japan ver 1)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM, 0,
	NULL, joemacRomInfo, joemacRomName, NULL, NULL, DrvInputInfo, CninjaDIPInfo,
	CninjaInit, DrvExit, CninjaFrame, CninjaDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Stoneage (bootleg of Caveman Ninja)

static struct BurnRomInfo stoneageRomDesc[] = {
	{ "sa_1_019.bin",	0x020000, 0x7fb8c44f, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "sa_1_033.bin",	0x020000, 0x961c752b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sa_1_018.bin",	0x020000, 0xa4043022, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "sa_1_032.bin",	0x020000, 0xf52a3286, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "sa_1_017.bin",	0x020000, 0x08d6397a, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "sa_1_031.bin",	0x020000, 0x103079f5, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "sa_1_012.bin",	0x010000, 0x56058934, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "gl-08.rom",		0x010000, 0x33a2b400, 3 | BRF_GRA }, 	       //  7 Characters
	{ "gl-09.rom",		0x010000, 0x5a2d4752, 3 | BRF_GRA }, 	       //  8

	{ "mag-02.rom",		0x080000, 0xde89c69a, 4 | BRF_GRA }, 	       //  9 Foreground Tiles

	{ "mag-00.rom",		0x080000, 0xa8f05d33, 5 | BRF_GRA }, 	       // 10 Background Tiles
	{ "mag-01.rom",		0x080000, 0x5b399eed, 5 | BRF_GRA }, 	       // 11

	{ "mag-03.rom",		0x080000, 0x2220eb9f, 6 | BRF_GRA }, 	       // 12 Sprites
	{ "mag-05.rom",		0x080000, 0x56a53254, 6 | BRF_GRA }, 	       // 13
	{ "mag-04.rom",		0x080000, 0x144b94cc, 6 | BRF_GRA }, 	       // 14
	{ "mag-06.rom",		0x080000, 0x82d44749, 6 | BRF_GRA }, 	       // 15

	{ "sa_1_069.bin",	0x040000, 0x2188f3ca, 7 | BRF_SND }, 	       // 16 OKI M6295 Samples
};

STD_ROM_PICK(stoneage)
STD_ROM_FN(stoneage)

struct BurnDriver BurnDrvStoneage = {
	"stoneage", "cninja", NULL, NULL, "1991",
	"Stoneage (bootleg of Caveman Ninja)\0", NULL, "bootleg", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM, 0,
	NULL, stoneageRomInfo, stoneageRomName, NULL, NULL, DrvInputInfo, CninjaDIPInfo,
	StoneageInit, DrvExit, StoneageFrame, CninjaDraw, StoneageScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Caveman Ninja (bootleg)

static struct BurnRomInfo cninjablRomDesc[] = {
	{ "joe mac 3.68k",	0x080000, 0xdc931d80, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "joe mac 4.68k",	0x080000, 0xe8dfe0b5, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "joe mac 5.z80",	0x010000, 0xd791b9d7, 2 | BRF_PRG | BRF_ESS }, //  2 Huc6280 Code

	{ "joe mac 1.gfx",	0x200000, 0x17ea5931, 3 | BRF_GRA }, 	       //  3 Graphics (Expanded on init)
	{ "joe mac 2.gfx",	0x200000, 0xcc95317b, 3 | BRF_GRA }, 	       //  4

	{ "joe mac 6.samples",	0x080000, 0xdbecad83, 4 | BRF_SND }, 	       //  5 OKI M6295 Samples
};

STD_ROM_PICK(cninjabl)
STD_ROM_FN(cninjabl)

struct BurnDriver BurnDrvCninjabl = {
	"cninjabl", "cninja", NULL, NULL, "1991",
	"Caveman Ninja (bootleg)\0", NULL, "bootleg", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM, 0,
	NULL, cninjablRomInfo, cninjablRomName, NULL, NULL, DrvInputInfo, CninjaDIPInfo,
	CninjablInit, DrvExit, StoneageFrame, CninjablDraw, StoneageScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Mutant Fighter (World ver EM-5)

static struct BurnRomInfo mutantfRomDesc[] = {
	{ "hd-03-4.2c",		0x020000, 0x94859545, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "hd-00-4.2a",		0x020000, 0x3cdb648f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "hd-04-1.4c",		0x020000, 0xfd2ea8d7, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "hd-01-1.4a",		0x020000, 0x48a247ac, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "hd-12.21j",		0x010000, 0x13d55f11, 2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "hd-06-1.8d",		0x010000, 0x8b7a558b, 3 | BRF_GRA }, 	       //  5 Characters
	{ "hd-07-1.9d",		0x010000, 0xd2a3d449, 3 | BRF_GRA }, 	       //  6

	{ "maf-00.8a",		0x080000, 0xe56f528d, 4 | BRF_GRA }, 	       //  7 Foreground Tiles

	{ "maf-01.9a",		0x080000, 0xc3d5173d, 5 | BRF_GRA }, 	       //  8 Background Tiles
	{ "maf-02.11a",		0x080000, 0x0b37d849, 5 | BRF_GRA }, 	       //  9

	{ "maf-03.18a",		0x100000, 0xf4366d2c, 6 | BRF_GRA }, 	       // 10 Sprite Bank A
	{ "maf-04.20a",		0x100000, 0x0c8f654e, 6 | BRF_GRA }, 	       // 11
	{ "maf-05.21a",		0x080000, 0xb0cfeb80, 6 | BRF_GRA }, 	       // 12
	{ "maf-06.18d",		0x100000, 0xf5c7a9b5, 6 | BRF_GRA }, 	       // 13
	{ "maf-07.20d",		0x100000, 0xfd6008a3, 6 | BRF_GRA }, 	       // 14
	{ "maf-08.21d",		0x080000, 0xe41cf1e7, 6 | BRF_GRA }, 	       // 15

	{ "hf-08.15a",		0x010000, 0x93b7279f, 7 | BRF_GRA }, 	       // 16 Sprite Bank B
	{ "hf-09.17a",		0x010000, 0x05e2c074, 7 | BRF_GRA }, 	       // 17
	{ "hf-10.15c",		0x010000, 0x9b06f418, 7 | BRF_GRA }, 	       // 18
	{ "hf-11.17c",		0x010000, 0x3859a531, 7 | BRF_GRA }, 	       // 19

	{ "maf-10.20l",		0x040000, 0x7c57f48b, 8 | BRF_SND }, 	       // 20 OKI M6295 Samples 0

	{ "maf-09.18l",		0x080000, 0x28e7ed81, 9 | BRF_SND }, 	       // 21 OKI M6295 Samples 1
};

STD_ROM_PICK(mutantf)
STD_ROM_FN(mutantf)

struct BurnDriver BurnDrvMutantf = {
	"mutantf", NULL, NULL, NULL, "1992",
	"Mutant Fighter (World ver EM-5)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_VSFIGHT, 0,
	NULL, mutantfRomInfo, mutantfRomName, NULL, NULL, DrvInputInfo, MutantfDIPInfo,
	MutantfInit, DrvExit, MutantfFrame, MutantfDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Mutant Fighter (World ver EM-4)

static struct BurnRomInfo mutantf4RomDesc[] = {
	{ "hd-03-3.2c",		0x020000, 0xe6f53574, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "hd-00-3.2a",		0x020000, 0xd3055454, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "hd-04-1.4c",		0x020000, 0xfd2ea8d7, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "hd-01-1.4a",		0x020000, 0x48a247ac, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "hd-12.21j",		0x010000, 0x13d55f11, 2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "hd-06-1.8d",		0x010000, 0x8b7a558b, 3 | BRF_GRA }, 	       //  5 Characters
	{ "hd-07-1.9d",		0x010000, 0xd2a3d449, 3 | BRF_GRA }, 	       //  6

	{ "maf-00.8a",		0x080000, 0xe56f528d, 4 | BRF_GRA }, 	       //  7 Foreground Tiles

	{ "maf-01.9a",		0x080000, 0xc3d5173d, 5 | BRF_GRA }, 	       //  8 Background Tiles
	{ "maf-02.11a",		0x080000, 0x0b37d849, 5 | BRF_GRA }, 	       //  9

	{ "maf-03.18a",		0x100000, 0xf4366d2c, 6 | BRF_GRA }, 	       // 10 Sprite Bank A
	{ "maf-04.20a",		0x100000, 0x0c8f654e, 6 | BRF_GRA }, 	       // 11
	{ "maf-05.21a",		0x080000, 0xb0cfeb80, 6 | BRF_GRA }, 	       // 12
	{ "maf-06.18d",		0x100000, 0xf5c7a9b5, 6 | BRF_GRA }, 	       // 13
	{ "maf-07.20d",		0x100000, 0xfd6008a3, 6 | BRF_GRA }, 	       // 14
	{ "maf-08.21d",		0x080000, 0xe41cf1e7, 6 | BRF_GRA }, 	       // 15

	{ "hf-08.15a",		0x010000, 0x93b7279f, 7 | BRF_GRA }, 	       // 16 Sprite Bank B
	{ "hf-09.17a",		0x010000, 0x05e2c074, 7 | BRF_GRA }, 	       // 17
	{ "hf-10.15c",		0x010000, 0x9b06f418, 7 | BRF_GRA }, 	       // 18
	{ "hf-11.17c",		0x010000, 0x3859a531, 7 | BRF_GRA }, 	       // 19

	{ "maf-10.20l",		0x040000, 0x7c57f48b, 8 | BRF_SND }, 	       // 20 OKI M6295 Samples 0

	{ "maf-09.18l",		0x080000, 0x28e7ed81, 9 | BRF_SND }, 	       // 21 OKI M6295 Samples 1
};

STD_ROM_PICK(mutantf4)
STD_ROM_FN(mutantf4)

struct BurnDriver BurnDrvMutantf4 = {
	"mutantf4", "mutantf", NULL, NULL, "1992",
	"Mutant Fighter (World ver EM-4)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_VSFIGHT, 0,
	NULL, mutantf4RomInfo, mutantf4RomName, NULL, NULL, DrvInputInfo, MutantfDIPInfo,
	MutantfInit, DrvExit, MutantfFrame, MutantfDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Mutant Fighter (World ver EM-3)

static struct BurnRomInfo mutantf3RomDesc[] = {
	{ "hd-03-2.2c",		0x020000, 0x0586c4fa, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "hd-00-2.2a",		0x020000, 0x6f8ec48e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "hd-04-1.4c",		0x020000, 0xfd2ea8d7, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "hd-01-1.4a",		0x020000, 0x48a247ac, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "hd-12.21j",		0x010000, 0x13d55f11, 2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "hd-06-1.8d",		0x010000, 0x8b7a558b, 3 | BRF_GRA }, 	       //  5 Characters
	{ "hd-07-1.9d",		0x010000, 0xd2a3d449, 3 | BRF_GRA }, 	       //  6

	{ "maf-00.8a",		0x080000, 0xe56f528d, 4 | BRF_GRA }, 	       //  7 Foreground Tiles

	{ "maf-01.9a",		0x080000, 0xc3d5173d, 5 | BRF_GRA }, 	       //  8 Background Tiles
	{ "maf-02.11a",		0x080000, 0x0b37d849, 5 | BRF_GRA }, 	       //  9

	{ "maf-03.18a",		0x100000, 0xf4366d2c, 6 | BRF_GRA }, 	       // 10 Sprite Bank A
	{ "maf-04.20a",		0x100000, 0x0c8f654e, 6 | BRF_GRA }, 	       // 11
	{ "maf-05.21a",		0x080000, 0xb0cfeb80, 6 | BRF_GRA }, 	       // 12
	{ "maf-06.18d",		0x100000, 0xf5c7a9b5, 6 | BRF_GRA }, 	       // 13
	{ "maf-07.20d",		0x100000, 0xfd6008a3, 6 | BRF_GRA }, 	       // 14
	{ "maf-08.21d",		0x080000, 0xe41cf1e7, 6 | BRF_GRA }, 	       // 15

	{ "hf-08.15a",		0x010000, 0x93b7279f, 7 | BRF_GRA }, 	       // 16 Sprite Bank B
	{ "hf-09.17a",		0x010000, 0x05e2c074, 7 | BRF_GRA }, 	       // 17
	{ "hf-10.15c",		0x010000, 0x9b06f418, 7 | BRF_GRA }, 	       // 18
	{ "hf-11.17c",		0x010000, 0x3859a531, 7 | BRF_GRA }, 	       // 19

	{ "maf-10.20l",		0x040000, 0x7c57f48b, 8 | BRF_SND }, 	       // 20 OKI M6295 Samples 0

	{ "maf-09.18l",		0x080000, 0x28e7ed81, 9 | BRF_SND }, 	       // 21 OKI M6295 Samples 1
};

STD_ROM_PICK(mutantf3)
STD_ROM_FN(mutantf3)

struct BurnDriver BurnDrvMutantf3 = {
	"mutantf3", "mutantf", NULL, NULL, "1992",
	"Mutant Fighter (World ver EM-3)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_VSFIGHT, 0,
	NULL, mutantf3RomInfo, mutantf3RomName, NULL, NULL, DrvInputInfo, MutantfDIPInfo,
	MutantfInit, DrvExit, MutantfFrame, MutantfDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Death Brade (Japan ver JM-3)

static struct BurnRomInfo deathbrdRomDesc[] = {
	{ "hf-03-2.2c",		0x020000, 0xfb86fff3, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "hf-00-2.2a",		0x020000, 0x099aa422, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "hd-04-1.4c",		0x020000, 0xfd2ea8d7, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "hd-01-1.4a",		0x020000, 0x48a247ac, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "hd-12.21j",		0x010000, 0x13d55f11, 2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "hd-06-1.8d",		0x010000, 0x8b7a558b, 3 | BRF_GRA }, 	       //  5 Characters
	{ "hd-07-1.9d",		0x010000, 0xd2a3d449, 3 | BRF_GRA }, 	       //  6

	{ "maf-00.8a",		0x080000, 0xe56f528d, 4 | BRF_GRA }, 	       //  7 Foreground Tiles

	{ "maf-01.9a",		0x080000, 0xc3d5173d, 5 | BRF_GRA }, 	       //  8 Background Tiles
	{ "maf-02.11a",		0x080000, 0x0b37d849, 5 | BRF_GRA }, 	       //  9

	{ "maf-03.18a",		0x100000, 0xf4366d2c, 6 | BRF_GRA }, 	       // 10 Sprite Bank A
	{ "maf-04.20a",		0x100000, 0x0c8f654e, 6 | BRF_GRA }, 	       // 11
	{ "maf-05.21a",		0x080000, 0xb0cfeb80, 6 | BRF_GRA }, 	       // 12
	{ "maf-06.18d",		0x100000, 0xf5c7a9b5, 6 | BRF_GRA }, 	       // 13
	{ "maf-07.20d",		0x100000, 0xfd6008a3, 6 | BRF_GRA }, 	       // 14
	{ "maf-08.21d",		0x080000, 0xe41cf1e7, 6 | BRF_GRA }, 	       // 15

	{ "hf-08.15a",		0x010000, 0x93b7279f, 7 | BRF_GRA }, 	       // 16 Sprite Bank B
	{ "hf-09.17a",		0x010000, 0x05e2c074, 7 | BRF_GRA }, 	       // 17
	{ "hf-10.15c",		0x010000, 0x9b06f418, 7 | BRF_GRA }, 	       // 18
	{ "hf-11.17c",		0x010000, 0x3859a531, 7 | BRF_GRA }, 	       // 19

	{ "maf-10.20l",		0x040000, 0x7c57f48b, 8 | BRF_SND }, 	       // 20 OKI M6295 Samples 0

	{ "maf-09.18l",		0x080000, 0x28e7ed81, 9 | BRF_SND }, 	       // 21 OKI M6295 Samples 1
};

STD_ROM_PICK(deathbrd)
STD_ROM_FN(deathbrd)

struct BurnDriver BurnDrvDeathbrd = {
	"deathbrd", "mutantf", NULL, NULL, "1992",
	"Death Brade (Japan ver JM-3)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_VSFIGHT, 0,
	NULL, deathbrdRomInfo, deathbrdRomName, NULL, NULL, DrvInputInfo, MutantfDIPInfo,
	MutantfInit, DrvExit, MutantfFrame, MutantfDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// The Cliffhanger - Edward Randy (World ver 3)

static struct BurnRomInfo edrandyRomDesc[] = {
	{ "gg-00-2.k1",		0x020000, 0xce1ba964, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gg-04-2.k3",		0x020000, 0x24caed19, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "gg-01-2.j1",		0x020000, 0x33677b80, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "gg-05-2.j3",		0x020000, 0x79a68ca6, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "ge-02.h1",		0x020000, 0xc2969fbb, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "ge-06.h3",		0x020000, 0x5c2e6418, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "ge-03.f1",		0x020000, 0x5e7b19a8, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "ge-07.f3",		0x020000, 0x5eb819a1, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "ge-09.k13",		0x010000, 0x9f94c60b, 2 | BRF_PRG | BRF_ESS }, //  8 Huc6280 Code

	{ "gg-10.y6",		0x010000, 0xb96c6cbe, 3 | BRF_GRA }, 	       //  9 Characters
	{ "gg-11.z6",		0x010000, 0xee567448, 3 | BRF_GRA }, 	       // 10

	{ "mad-02",		0x080000, 0x6c76face, 4 | BRF_GRA }, 	       // 11 Foreground Tiles

	{ "mad-00",		0x080000, 0x3735b22d, 5 | BRF_GRA }, 	       // 12 Background Tiles
	{ "mad-01",		0x080000, 0x7bb13e1c, 5 | BRF_GRA }, 	       // 13

	{ "mad-03",		0x080000, 0xc0bff892, 6 | BRF_GRA }, 	       // 14 Sprites
	{ "mad-05",		0x080000, 0x3f2ccf95, 6 | BRF_GRA }, 	       // 15
	{ "mad-04",		0x080000, 0x464f3eb9, 6 | BRF_GRA }, 	       // 16
	{ "mad-06",		0x080000, 0x60871f77, 6 | BRF_GRA }, 	       // 17
	{ "mad-07",		0x080000, 0xac03466e, 6 | BRF_GRA }, 	       // 18
	{ "mad-08",		0x080000, 0x1b420ec8, 6 | BRF_GRA }, 	       // 19
	{ "mad-10",		0x080000, 0x42da8ef0, 6 | BRF_GRA }, 	       // 20
	{ "mad-11",		0x080000, 0x03c1f982, 6 | BRF_GRA }, 	       // 21
	{ "mad-09",		0x080000, 0x930f4900, 6 | BRF_GRA }, 	       // 22
	{ "mad-12",		0x080000, 0xa0bd62b6, 6 | BRF_GRA }, 	       // 23

	{ "ge-08.j13",		0x020000, 0xdfe28c7b, 7 | BRF_SND }, 	       // 24 OKI M6295 Samples 0

	{ "mad-13",		0x080000, 0x6ab28eba, 8 | BRF_SND }, 	       // 25 OKI M6295 Samples 1

	{ "ge-12.v7",		0x000400, 0x278f674f, 0 | BRF_OPT }, 	       // 26 Unused PROMs
};

STD_ROM_PICK(edrandy)
STD_ROM_FN(edrandy)

struct BurnDriver BurnDrvEdrandy = {
	"edrandy", NULL, NULL, NULL, "1990",
	"The Cliffhanger - Edward Randy (World ver 3)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, edrandyRomInfo, edrandyRomName, NULL, NULL, DrvInputInfo, EdrandyDIPInfo,
	EdrandyInit, DrvExit, CninjaFrame, EdrandyDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// The Cliffhanger - Edward Randy (World ver 2)

static struct BurnRomInfo edrandy2RomDesc[] = {
	{ "gg00-1.k1",		0x020000, 0xa029cc4a, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gg04-1.k3",		0x020000, 0x8b7928a4, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "gg01-1.j1",		0x020000, 0x84360123, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "gg05-1.j3",		0x020000, 0x0bf85d9d, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "ge-02.h1",		0x020000, 0xc2969fbb, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "ge-06.h3",		0x020000, 0x5c2e6418, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "ge-03.f1",		0x020000, 0x5e7b19a8, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "ge-07.f3",		0x020000, 0x5eb819a1, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "ge-09.k13",		0x010000, 0x9f94c60b, 2 | BRF_PRG | BRF_ESS }, //  8 Huc6280 Code

	{ "gg-10.y6",		0x010000, 0xb96c6cbe, 3 | BRF_GRA }, 	       //  9 Characters
	{ "gg-11.z6",		0x010000, 0xee567448, 3 | BRF_GRA }, 	       // 10

	{ "mad-02",		0x080000, 0x6c76face, 4 | BRF_GRA }, 	       // 11 Foreground Tiles

	{ "mad-00",		0x080000, 0x3735b22d, 5 | BRF_GRA }, 	       // 12 Background Tiles
	{ "mad-01",		0x080000, 0x7bb13e1c, 5 | BRF_GRA }, 	       // 13

	{ "mad-03",		0x080000, 0xc0bff892, 6 | BRF_GRA }, 	       // 14 Sprites
	{ "mad-05",		0x080000, 0x3f2ccf95, 6 | BRF_GRA }, 	       // 15
	{ "mad-04",		0x080000, 0x464f3eb9, 6 | BRF_GRA }, 	       // 16
	{ "mad-06",		0x080000, 0x60871f77, 6 | BRF_GRA }, 	       // 17
	{ "mad-07",		0x080000, 0xac03466e, 6 | BRF_GRA }, 	       // 18
	{ "mad-08",		0x080000, 0x1b420ec8, 6 | BRF_GRA }, 	       // 19
	{ "mad-10",		0x080000, 0x42da8ef0, 6 | BRF_GRA }, 	       // 20
	{ "mad-11",		0x080000, 0x03c1f982, 6 | BRF_GRA }, 	       // 21
	{ "mad-09",		0x080000, 0x930f4900, 6 | BRF_GRA }, 	       // 22
	{ "mad-12",		0x080000, 0xa0bd62b6, 6 | BRF_GRA }, 	       // 23

	{ "ge-08.j13",		0x020000, 0xdfe28c7b, 7 | BRF_SND }, 	       // 24 OKI M6295 Samples 0

	{ "mad-13",		0x080000, 0x6ab28eba, 8 | BRF_SND }, 	       // 25 OKI M6295 Samples 1

	{ "ge-12.v7",		0x000400, 0x278f674f, 0 | BRF_OPT }, 	       // 26 Unused PROMs
};

STD_ROM_PICK(edrandy2)
STD_ROM_FN(edrandy2)

struct BurnDriver BurnDrvEdrandy2 = {
	"edrandy2", "edrandy", NULL, NULL, "1990",
	"The Cliffhanger - Edward Randy (World ver 2)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, edrandy2RomInfo, edrandy2RomName, NULL, NULL, DrvInputInfo, EdrandcDIPInfo,
	EdrandyInit, DrvExit, CninjaFrame, EdrandyDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// The Cliffhanger - Edward Randy (World ver 1)

static struct BurnRomInfo edrandy1RomDesc[] = {
	{ "1.k1",		0x020000, 0xf184cdaa, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "5.k3",		0x020000, 0x7e3a4b81, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "2.j1",		0x020000, 0x212cd593, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "6.j3",		0x020000, 0x4a96fb07, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "ge-02.h1",		0x020000, 0xc2969fbb, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "ge-06.h3",		0x020000, 0x5c2e6418, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "ge-03.f1",		0x020000, 0x5e7b19a8, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "ge-07.f3",		0x020000, 0x5eb819a1, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "ge-09.k13",		0x010000, 0x9f94c60b, 2 | BRF_PRG | BRF_ESS }, //  8 Huc6280 Code

	{ "ge-10.y6",		0x010000, 0x2528d795, 3 | BRF_GRA }, 	       //  9 Characters
	{ "ge-11.z6",		0x010000, 0xe34a931e, 3 | BRF_GRA }, 	       // 10

	{ "mad-02",		0x080000, 0x6c76face, 4 | BRF_GRA }, 	       // 11 Foreground Tiles

	{ "mad-00",		0x080000, 0x3735b22d, 5 | BRF_GRA }, 	       // 12 Background Tiles
	{ "mad-01",		0x080000, 0x7bb13e1c, 5 | BRF_GRA }, 	       // 13

	{ "mad-03",		0x080000, 0xc0bff892, 6 | BRF_GRA }, 	       // 14 Sprites
	{ "mad-05",		0x080000, 0x3f2ccf95, 6 | BRF_GRA }, 	       // 15
	{ "mad-04",		0x080000, 0x464f3eb9, 6 | BRF_GRA }, 	       // 16
	{ "mad-06",		0x080000, 0x60871f77, 6 | BRF_GRA }, 	       // 17
	{ "mad-07",		0x080000, 0xac03466e, 6 | BRF_GRA }, 	       // 18
	{ "mad-08",		0x080000, 0x1b420ec8, 6 | BRF_GRA }, 	       // 19
	{ "mad-10",		0x080000, 0x42da8ef0, 6 | BRF_GRA }, 	       // 20
	{ "mad-11",		0x080000, 0x03c1f982, 6 | BRF_GRA }, 	       // 21
	{ "mad-09",		0x080000, 0x930f4900, 6 | BRF_GRA }, 	       // 22
	{ "mad-12",		0x080000, 0xa0bd62b6, 6 | BRF_GRA }, 	       // 23

	{ "ge-08.j13",		0x020000, 0xdfe28c7b, 7 | BRF_SND }, 	       // 24 OKI M6295 Samples 0

	{ "mad-13",		0x080000, 0x6ab28eba, 8 | BRF_SND }, 	       // 25 OKI M6295 Samples 1

	{ "ge-12.v7",		0x000400, 0x278f674f, 0 | BRF_OPT }, 	       // 26 Unused PROMs
};

STD_ROM_PICK(edrandy1)
STD_ROM_FN(edrandy1)

struct BurnDriver BurnDrvEdrandy1 = {
	"edrandy1", "edrandy", NULL, NULL, "1990",
	"The Cliffhanger - Edward Randy (World ver 1)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, edrandy1RomInfo, edrandy1RomName, NULL, NULL, DrvInputInfo, EdrandcDIPInfo,
	EdrandyInit, DrvExit, CninjaFrame, EdrandyDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// The Cliffhanger - Edward Randy (Japan ver 3)

static struct BurnRomInfo edrandyjRomDesc[] = {
	{ "ge-00-2.k1",		0x020000, 0xb3d2403c, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ge-04-2.k3",		0x020000, 0x8a9624d6, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ge-01-2.j1",		0x020000, 0x84360123, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ge-05-2.j3",		0x020000, 0x0bf85d9d, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "ge-02.h1",		0x020000, 0xc2969fbb, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "ge-06.h3",		0x020000, 0x5c2e6418, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "ge-03.f1",		0x020000, 0x5e7b19a8, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "ge-07.f3",		0x020000, 0x5eb819a1, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "ge-09.k13",		0x010000, 0x9f94c60b, 2 | BRF_PRG | BRF_ESS }, //  8 Huc6280 Code

	{ "ge-10.y6",		0x010000, 0x2528d795, 3 | BRF_GRA }, 	       //  9 Characters
	{ "ge-11.z6",		0x010000, 0xe34a931e, 3 | BRF_GRA }, 	       // 10

	{ "mad-02",		0x080000, 0x6c76face, 4 | BRF_GRA }, 	       // 11 Foreground Tiles

	{ "mad-00",		0x080000, 0x3735b22d, 5 | BRF_GRA }, 	       // 12 Background Tiles
	{ "mad-01",		0x080000, 0x7bb13e1c, 5 | BRF_GRA }, 	       // 13

	{ "mad-03",		0x080000, 0xc0bff892, 6 | BRF_GRA }, 	       // 14 Sprites
	{ "mad-05",		0x080000, 0x3f2ccf95, 6 | BRF_GRA }, 	       // 15
	{ "mad-04",		0x080000, 0x464f3eb9, 6 | BRF_GRA }, 	       // 16
	{ "mad-06",		0x080000, 0x60871f77, 6 | BRF_GRA }, 	       // 17
	{ "mad-07",		0x080000, 0xac03466e, 6 | BRF_GRA }, 	       // 18
	{ "mad-08",		0x080000, 0x1b420ec8, 6 | BRF_GRA }, 	       // 19
	{ "mad-10",		0x080000, 0x42da8ef0, 6 | BRF_GRA }, 	       // 20
	{ "mad-11",		0x080000, 0x03c1f982, 6 | BRF_GRA }, 	       // 21
	{ "mad-09",		0x080000, 0x930f4900, 6 | BRF_GRA }, 	       // 22
	{ "mad-12",		0x080000, 0xa0bd62b6, 6 | BRF_GRA }, 	       // 23

	{ "ge-08.j13",		0x020000, 0xdfe28c7b, 7 | BRF_SND }, 	       // 24 OKI M6295 Samples 0

	{ "mad-13",		0x080000, 0x6ab28eba, 8 | BRF_SND }, 	       // 25 OKI M6295 Samples 1

	{ "ge-12.v7",		0x000400, 0x278f674f, 0 | BRF_OPT }, 	       // 26 Unused PROMs
};

STD_ROM_PICK(edrandyj)
STD_ROM_FN(edrandyj)

struct BurnDriver BurnDrvEdrandyj = {
	"edrandyj", "edrandy", NULL, NULL, "1990",
	"The Cliffhanger - Edward Randy (Japan ver 3)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, edrandyjRomInfo, edrandyjRomName, NULL, NULL, DrvInputInfo, EdrandcDIPInfo,
	EdrandyInit, DrvExit, CninjaFrame, EdrandyDraw, DrvScan, &DrvRecalc, 0x800,
	256, 240, 4, 3
};


// Robocop 2 (Euro/Asia v0.10)

static struct BurnRomInfo robocop2RomDesc[] = {
	{ "gq-03.k1",		0x020000, 0xa7e90c28, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gq-07.k3",		0x020000, 0xd2287ec1, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "gq-02.j1",		0x020000, 0x6777b8a0, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "gq-06.j3",		0x020000, 0xe11e27b5, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "go-01-1.h1",		0x020000, 0xab5356c0, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "go-05-1.h3",		0x020000, 0xce21bda5, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "go-00.f1",		0x020000, 0xa93369ea, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "go-04.f3",		0x020000, 0xee2f6ad9, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "gp-09.k13",		0x010000, 0x4a4e0f8d, 2 | BRF_PRG | BRF_ESS }, //  8 Huc6280 Code

	{ "gp10-1.y6",		0x010000, 0xd25d719c, 3 | BRF_GRA }, 	       //  9 Characters
	{ "gp11-1.z6",		0x010000, 0x030ded47, 3 | BRF_GRA }, 	       // 10

	{ "mah-04.z4",		0x080000, 0x9b6ca18c, 4 | BRF_GRA }, 	       // 11 Foreground Tiles
	{ "mah-03.y4",		0x080000, 0x37894ddc, 4 | BRF_GRA }, 	       // 12

	{ "mah-01.z1",		0x080000, 0x26e0dfff, 5 | BRF_GRA }, 	       // 13 Background Tiles
	{ "mah-00.y1",		0x080000, 0x7bd69e41, 5 | BRF_GRA }, 	       // 14
	{ "mah-02.a1",		0x080000, 0x328a247d, 5 | BRF_GRA }, 	       // 15

	{ "mah-05.y9",		0x080000, 0x6773e613, 6 | BRF_GRA }, 	       // 16 Sprites
	{ "mah-08.y12",		0x080000, 0x88d310a5, 6 | BRF_GRA }, 	       // 17
	{ "mah-06.z9",		0x080000, 0x27a8808a, 6 | BRF_GRA }, 	       // 18
	{ "mah-09.z12",		0x080000, 0xa58c43a7, 6 | BRF_GRA }, 	       // 19
	{ "mah-07.a9",		0x080000, 0x526f4190, 6 | BRF_GRA }, 	       // 20
	{ "mah-10.a12",		0x080000, 0x14b770da, 6 | BRF_GRA }, 	       // 21

	{ "gp-08.j13",		0x020000, 0x365183b1, 7 | BRF_SND }, 	       // 22 OKI M6295 Samples 0

	{ "mah-11.f13",		0x080000, 0x642bc692, 8 | BRF_SND }, 	       // 23 OKI M6295 Samples 1

	{ "go-12.v7",		0x000400, 0x278f674f, 0 | BRF_OPT }, 	       // 24 Unused PROMs
};

STD_ROM_PICK(robocop2)
STD_ROM_FN(robocop2)

struct BurnDriver BurnDrvRobocop2 = {
	"robocop2", NULL, NULL, NULL, "1991",
	"Robocop 2 (Euro/Asia v0.10)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, robocop2RomInfo, robocop2RomName, NULL, NULL, Robocop2InputInfo, Robocop2DIPInfo,
	Robocop2Init, DrvExit, CninjaFrame, Robocop2Draw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Robocop 2 (US v0.05)

static struct BurnRomInfo robocop2uRomDesc[] = {
	{ "robo03.k1",		0x020000, 0xf4c96cc9, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "robo07.k3",		0x020000, 0x11e53a7c, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "robo02.j1",		0x020000, 0xfa086a0d, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "robo06.j3",		0x020000, 0x703b49d0, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "go-01-1.h1",		0x020000, 0xab5356c0, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "go-05-1.h3",		0x020000, 0xce21bda5, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "go-00.f1",		0x020000, 0xa93369ea, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "go-04.f3",		0x020000, 0xee2f6ad9, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "gp-09.k13",		0x010000, 0x4a4e0f8d, 2 | BRF_PRG | BRF_ESS }, //  8 Huc6280 Code

	{ "gp10-1.y6",		0x010000, 0xd25d719c, 3 | BRF_GRA }, 	       //  9 Characters
	{ "gp11-1.z6",		0x010000, 0x030ded47, 3 | BRF_GRA }, 	       // 10

	{ "mah-04.z4",		0x080000, 0x9b6ca18c, 4 | BRF_GRA }, 	       // 11 Foreground Tiles
	{ "mah-03.y4",		0x080000, 0x37894ddc, 4 | BRF_GRA }, 	       // 12

	{ "mah-01.z1",		0x080000, 0x26e0dfff, 5 | BRF_GRA }, 	       // 13 Background Tiles
	{ "mah-00.y1",		0x080000, 0x7bd69e41, 5 | BRF_GRA }, 	       // 14
	{ "mah-02.a1",		0x080000, 0x328a247d, 5 | BRF_GRA }, 	       // 15

	{ "mah-05.y9",		0x080000, 0x6773e613, 6 | BRF_GRA }, 	       // 16 Sprites
	{ "mah-08.y12",		0x080000, 0x88d310a5, 6 | BRF_GRA }, 	       // 17
	{ "mah-06.z9",		0x080000, 0x27a8808a, 6 | BRF_GRA }, 	       // 18
	{ "mah-09.z12",		0x080000, 0xa58c43a7, 6 | BRF_GRA }, 	       // 19
	{ "mah-07.a9",		0x080000, 0x526f4190, 6 | BRF_GRA }, 	       // 20
	{ "mah-10.a12",		0x080000, 0x14b770da, 6 | BRF_GRA }, 	       // 21

	{ "gp-08.j13",		0x020000, 0x365183b1, 7 | BRF_SND }, 	       // 22 OKI M6295 Samples 0

	{ "mah-11.f13",		0x080000, 0x642bc692, 8 | BRF_SND }, 	       // 23 OKI M6295 Samples 1

	{ "go-12.v7",		0x000400, 0x278f674f, 0 | BRF_OPT }, 	       // 24 Unused PROMs
};

STD_ROM_PICK(robocop2u)
STD_ROM_FN(robocop2u)

struct BurnDriver BurnDrvRobocop2u = {
	"robocop2u", "robocop2", NULL, NULL, "1991",
	"Robocop 2 (US v0.05)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, robocop2uRomInfo, robocop2uRomName, NULL, NULL, Robocop2InputInfo, Robocop2DIPInfo,
	Robocop2Init, DrvExit, CninjaFrame, Robocop2Draw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Robocop 2 (Japan v0.11)

static struct BurnRomInfo robocop2jRomDesc[] = {
	{ "go-03-1.k1",		0x020000, 0x52506608, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "go-07-1.k3",		0x020000, 0x739cda17, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "go-02-1.j1",		0x020000, 0x48c0ace9, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "go-06-1.j3",		0x020000, 0x41abec87, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "go-01-1.h1",		0x020000, 0xab5356c0, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "go-05-1.h3",		0x020000, 0xce21bda5, 1 | BRF_PRG | BRF_ESS }, //  5
	{ "go-00.f1",		0x020000, 0xa93369ea, 1 | BRF_PRG | BRF_ESS }, //  6
	{ "go-04.f3",		0x020000, 0xee2f6ad9, 1 | BRF_PRG | BRF_ESS }, //  7

	{ "gp-09.k13",		0x010000, 0x4a4e0f8d, 2 | BRF_PRG | BRF_ESS }, //  8 Huc6280 Code

	{ "gp10-1.y6",		0x010000, 0xd25d719c, 3 | BRF_GRA }, 	       //  9 Characters
	{ "gp11-1.z6",		0x010000, 0x030ded47, 3 | BRF_GRA }, 	       // 10

	{ "mah-04.z4",		0x080000, 0x9b6ca18c, 4 | BRF_GRA }, 	       // 11 Foreground Tiles
	{ "mah-03.y4",		0x080000, 0x37894ddc, 4 | BRF_GRA }, 	       // 12

	{ "mah-01.z1",		0x080000, 0x26e0dfff, 5 | BRF_GRA }, 	       // 13 Background Tiles
	{ "mah-00.y1",		0x080000, 0x7bd69e41, 5 | BRF_GRA }, 	       // 14
	{ "mah-02.a1",		0x080000, 0x328a247d, 5 | BRF_GRA }, 	       // 15

	{ "mah-05.y9",		0x080000, 0x6773e613, 6 | BRF_GRA }, 	       // 16 Sprites
	{ "mah-08.y12",		0x080000, 0x88d310a5, 6 | BRF_GRA }, 	       // 17
	{ "mah-06.z9",		0x080000, 0x27a8808a, 6 | BRF_GRA }, 	       // 18
	{ "mah-09.z12",		0x080000, 0xa58c43a7, 6 | BRF_GRA }, 	       // 19
	{ "mah-07.a9",		0x080000, 0x526f4190, 6 | BRF_GRA }, 	       // 20
	{ "mah-10.a12",		0x080000, 0x14b770da, 6 | BRF_GRA }, 	       // 21

	{ "gp-08.j13",		0x020000, 0x365183b1, 7 | BRF_SND }, 	       // 22 OKI M6295 Samples 0

	{ "mah-11.f13",		0x080000, 0x642bc692, 8 | BRF_SND }, 	       // 23 OKI M6295 Samples 1

	{ "go-12.v7",		0x000400, 0x278f674f, 0 | BRF_OPT }, 	       // 24 Unused PROMs
};

STD_ROM_PICK(robocop2j)
STD_ROM_FN(robocop2j)

struct BurnDriver BurnDrvRobocop2j = {
	"robocop2j", "robocop2", NULL, NULL, "1991",
	"Robocop 2 (Japan v0.11)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, robocop2jRomInfo, robocop2jRomName, NULL, NULL, Robocop2InputInfo, Robocop2DIPInfo,
	Robocop2Init, DrvExit, CninjaFrame, Robocop2Draw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};
