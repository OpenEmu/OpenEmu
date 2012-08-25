// FB Alpha The Main Event / Devastators driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "z80_intf.h"
#include "hd6309_intf.h"
#include "konamiic.h"
#include "burn_ym2151.h"
#include "upd7759.h"
#include "k007232.h"

static UINT8 *AllMem;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *MemEnd;
static UINT8 *DrvHD6309ROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *DrvHD6309RAM;
static UINT8 *DrvZ80RAM;
static UINT8 *soundlatch;
static UINT8 *irq_enable;
static UINT8 *nmi_enable;

static UINT8 *nDrvBank;

static UINT32  *DrvPalette;
static UINT8  DrvRecalc;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvJoy4[8];
static UINT8 DrvJoy5[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[5];
static UINT8 DrvReset;

static INT32 nGame;

static struct BurnInputInfo MainevtInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 2"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy1 + 2,	"p3 coin"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy4 + 2,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy4 + 3,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy4 + 0,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy4 + 1,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy4 + 5,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy4 + 4,	"p3 fire 2"	},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy1 + 3,	"p4 coin"	},
	{"P4 Up",		BIT_DIGITAL,	DrvJoy5 + 2,	"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy5 + 3,	"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy5 + 0,	"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	DrvJoy5 + 1,	"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy5 + 5,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy5 + 4,	"p4 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"service"	},
	{"Service 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"service2"	},
	{"Service 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"service3"	},
	{"Service 4",		BIT_DIGITAL,	DrvJoy1 + 7,	"service4"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Mainevt)

static struct BurnInputInfo Mainevt2pInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"service"	},
	{"Service 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"service2"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Mainevt2p)

static struct BurnInputInfo DevstorsInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 4,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Devstors)

static struct BurnDIPInfo MainevtDIPList[]=
{
	{0x21, 0xff, 0xff, 0xff, NULL			},
	{0x22, 0xff, 0xff, 0x57, NULL			},
	{0x23, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    16, "Coinage"		},
	{0x21, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 	},
	{0x21, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 	},
	{0x21, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 	},
	{0x21, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x21, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x21, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 	},
	{0x21, 0x01, 0x0f, 0x00, "4 Coins 5 Credits"	},
	{0x21, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x21, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x21, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x21, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x21, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x21, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x21, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x21, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x21, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Bonus Energy"		},
	{0x22, 0x01, 0x18, 0x00, "60"			},
	{0x22, 0x01, 0x18, 0x08, "70"			},
	{0x22, 0x01, 0x18, 0x10, "80"			},
	{0x22, 0x01, 0x18, 0x18, "90"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x22, 0x01, 0x60, 0x60, "Easy"			},
	{0x22, 0x01, 0x60, 0x40, "Normal"		},
	{0x22, 0x01, 0x60, 0x20, "Hard"			},
	{0x22, 0x01, 0x60, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x22, 0x01, 0x80, 0x80, "Off"			},
	{0x22, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x23, 0x01, 0x01, 0x01, "Off"			},
	{0x23, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x23, 0x01, 0x04, 0x04, "Off"			},
	{0x23, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Mainevt)

static struct BurnDIPInfo Mainevt2pDIPList[]=
{
	{0x15, 0xff, 0xff, 0xff, NULL			},
	{0x16, 0xff, 0xff, 0x57, NULL			},
	{0x17, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,   16, "Coinage"		},
	{0x15, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x15, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x15, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x00, "4 Coins 5 Credits"	},
	{0x15, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x15, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x15, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x15, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x15, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x15, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,   16, "Coin B"		},
	{0x15, 0x01, 0xf0, 0x20, "4 Coins 1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x50, "3 Coins 1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x80, "2 Coins 1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x15, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x15, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x00, "4 Coins 5 Credits"	},
	{0x15, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x15, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x15, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x15, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x15, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x15, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Bonus Energy"		},
	{0x16, 0x01, 0x18, 0x00, "60"			},
	{0x16, 0x01, 0x18, 0x08, "70"			},
	{0x16, 0x01, 0x18, 0x10, "80"			},
	{0x16, 0x01, 0x18, 0x18, "90"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x60, 0x60, "Easy"			},
	{0x16, 0x01, 0x60, 0x40, "Normal"		},
	{0x16, 0x01, 0x60, 0x20, "Hard"			},
	{0x16, 0x01, 0x60, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x16, 0x01, 0x80, 0x80, "Off"			},
	{0x16, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x17, 0x01, 0x01, 0x01, "Off"			},
	{0x17, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x17, 0x01, 0x04, 0x04, "Off"			},
	{0x17, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Mainevt2p)

static struct BurnDIPInfo DevstorsDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x5e, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,   16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,   15, "Coin B"		},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x03, 0x03, "2"			},
	{0x13, 0x01, 0x03, 0x02, "3"			},
	{0x13, 0x01, 0x03, 0x01, "5"			},
	{0x13, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0x18, 0x18, "150 and every 200"	},
	{0x13, 0x01, 0x18, 0x10, "150 and every 250"	},
	{0x13, 0x01, 0x18, 0x08, "150 Only"		},
	{0x13, 0x01, 0x18, 0x00, "200 Only"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x60, 0x60, "Easy"			},
	{0x13, 0x01, 0x60, 0x40, "Normal"		},
	{0x13, 0x01, 0x60, 0x20, "Hard"			},
	{0x13, 0x01, 0x60, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Devstors)

static struct BurnDIPInfo Devstors2DIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x52, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,   16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,   15, "Coin B"		},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x03, 0x03, "2"			},
	{0x13, 0x01, 0x03, 0x02, "3"			},
	{0x13, 0x01, 0x03, 0x01, "5"			},
	{0x13, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x13, 0x01, 0x04, 0x00, "Upright"		},
	{0x13, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0x18, 0x18, "150 and every 200"	},
	{0x13, 0x01, 0x18, 0x10, "150 and every 250"	},
	{0x13, 0x01, 0x18, 0x08, "150 Only"		},
	{0x13, 0x01, 0x18, 0x00, "200 Only"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x60, 0x60, "Easy"			},
	{0x13, 0x01, 0x60, 0x40, "Normal"		},
	{0x13, 0x01, 0x60, 0x20, "Hard"			},
	{0x13, 0x01, 0x60, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Devstors2)

static void bankswitch(INT32 data)
{
	nDrvBank[0] = data & 3;

	INT32 nBank = 0x10000 + nDrvBank[0] * 0x2000;

	HD6309MapMemory(DrvHD6309ROM + nBank, 0x6000, 0x7fff, HD6309_ROM);

	K052109RMRDLine = data & 0x40;
}

UINT8 mainevt_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x1f94:
			return DrvInputs[0];

		case 0x1f95:
			return DrvInputs[1];

		case 0x1f96:
			return DrvInputs[2];

		case 0x1f97:
			return DrvDips[0];

		case 0x1f98:
			return DrvDips[2];

		case 0x1f99:
			return DrvInputs[3];

		case 0x1f9a:
			return DrvInputs[4];

		case 0x1f9b:
			return DrvDips[1];
	}

	if ((address & 0xffe0) == 0x1fa0 && nGame) {
		return K051733Read(address);
	}

	if ((address & 0xc000) == 0x0000) {
		return K052109_051960_r(address & 0x3fff);
	}

	return 0;
}

void mainevt_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x1f80:
			bankswitch(data);
		return;

		case 0x1f84:
			*soundlatch = data;
		return;

		case 0x1f88:
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;

		case 0x1f90:
		return;

		case 0x1fb2:
			*nmi_enable = data;
		return;
	}

	if ((address & 0xffe0) == 0x1fa0 && nGame) {
		K051733Write(address, data);
		return;
	}

	if ((address & 0xc000) == 0x0000) {
		K052109_051960_w(address & 0x3fff, data);
		return;
	}
}

static void soundbankswitch(INT32 data)
{
	INT32 bank_A,bank_B,bank_C;

	nDrvBank[1] = data;

	bank_A =  data & 3;
	bank_B = (data >> 2) & 3;
	bank_C = 0x20000 + ((data >> 4) & 0x03) * 0x20000;

	k007232_set_bank(0, bank_A, bank_B );

	memcpy (DrvSndROM1, DrvSndROM1 + bank_C, 0x20000);
}

void __fastcall mainevt_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x9000:
			UPD7759PortWrite(0, data);
		return;

		case 0xc000:
			BurnYM2151SelectRegister(data);
		return;

		case 0xc001:
			BurnYM2151WriteRegister(data);
		return;

		case 0xe000:
			UPD7759ResetWrite(0, data & 2);
			UPD7759StartWrite(0, data & 1);

			*irq_enable = data & 4;
		return;

		case 0xf000:
			soundbankswitch(data);
		return;
	}

	if ((address & 0xfff0) == 0xb000) {
		K007232WriteReg(0, address & 0x0f, data);
		return;
	}
}

UINT8 __fastcall mainevt_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xa000:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;

		case 0xc000:
		case 0xc001:
			return BurnYM2151ReadStatus();

		case 0xd000:
			return UPD7759BusyRead(0);

	}

	if ((address & 0xfff0) == 0xb000) {
		return K007232ReadReg(0, address & 0x0f);
	}

	return 0;
}

static void K052109Callback(INT32 layer, INT32 , INT32 *code, INT32 *color, INT32 *flipx, INT32 *priority)
{
	*flipx = *color & 0x02;

	if (layer == 2) *priority = (*color >> 5) & 1;

	*code |= ((*color & 0x01) << 8) | ((*color & 0x1c) << 7);
	*color = ((layer & 2) << 1) + ((layer & 1) << 2) + ((*color & 0xc0) >> 6);
}

static void DvK052109Callback(INT32 layer, INT32, INT32 *code, INT32 *color, INT32 *, INT32 *)
{
	*code |= ((*color & 0x01) << 8) | ((*color & 0x3c) << 7);
	*color = ((layer & 2) << 1) + ((*color & 0xc0) >> 6);
}

static void K051960Callback(INT32 *, INT32 *color, INT32 *priority, INT32 *)
{
	if (*color & 0x20)	*priority = 0;
	else if (*color & 0x40)	*priority = 1;
	else			*priority = 2;

	*color = 0x0c + (*color & 0x03);
}

static void DvK051960Callback(INT32 *, INT32 *color, INT32 *, INT32 *)
{
	*color = 0x08 + (*color & 0x07);
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

	HD6309Open(0);
	HD6309Reset();
	HD6309Close();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2151Reset();

	UPD7759Reset();

	KonamiICReset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvHD6309ROM		= Next; Next += 0x020000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x040000;
	DrvGfxROM1		= Next; Next += 0x100000;
	DrvGfxROMExp0		= Next; Next += 0x080000;
	DrvGfxROMExp1		= Next; Next += 0x200000;

	DrvSndROM0		= Next; Next += 0x080000;
	DrvSndROM1		= Next; Next += 0x0a0000;

	DrvPalette		= (UINT32*)Next; Next += 0x100 * sizeof(UINT32);

	AllRam			= Next;

	DrvHD6309RAM		= Next; Next += 0x002000;
	DrvZ80RAM		= Next; Next += 0x000400;

	soundlatch		= Next; Next += 0x000001;

	irq_enable		= Next; Next += 0x000001;
	nmi_enable		= Next; Next += 0x000001;

	nDrvBank		= Next; Next += 0x000002;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode(INT32 gfxlen0)
{
	INT32 Plane0[4] = { 0x018, 0x010, 0x008, 0x000 };
	INT32 Plane1[4] = { 0x000, 0x008, 0x010, 0x018 };
	INT32 XOffs[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			  0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107 };
	INT32 YOffs[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
			  0x200, 0x220, 0x240, 0x260, 0x280, 0x2a0, 0x2c0, 0x2e0 };

	konami_rom_deinterleave_2(DrvGfxROM0, gfxlen0);
	konami_rom_deinterleave_2(DrvGfxROM1, 0x100000);

	GfxDecode(gfxlen0/0x20, 4,  8,  8, Plane0, XOffs, YOffs, 0x100, DrvGfxROM0, DrvGfxROMExp0);
	GfxDecode(0x02000,      4, 16, 16, Plane1, XOffs, YOffs, 0x400, DrvGfxROM1, DrvGfxROMExp1);

	return 0;
}

static INT32 DrvInit(INT32 type)
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	nGame = type;

	INT32 gfx0_offset = 0x10000 << nGame;

	{
		if (BurnLoadRom(DrvHD6309ROM  + 0x010000,  0, 1)) return 1;
		memcpy (DrvHD6309ROM + 0x08000, DrvHD6309ROM + 0x18000, 0x8000);

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  1, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  2, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001,  3, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + gfx0_offset + 0, 4, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + gfx0_offset + 1, 5, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000,  7, 1)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000,  8, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x020000,  9, 1)) return 1;
		memcpy (DrvSndROM1, DrvSndROM1 + 0x20000, 0x20000);

		DrvGfxDecode(gfx0_offset * 2);
	}

	HD6309Init(1);
	HD6309Open(0);
	HD6309MapMemory(DrvHD6309RAM,		0x4000, 0x5fff, HD6309_RAM);
	HD6309MapMemory(DrvHD6309ROM + 0x10000, 0x6000, 0x7fff, HD6309_ROM);
	HD6309MapMemory(DrvHD6309ROM + 0x08000, 0x8000, 0xffff, HD6309_ROM);
	HD6309SetWriteHandler(mainevt_main_write);
	HD6309SetReadHandler(mainevt_main_read);
	HD6309Close();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x83ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x83ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x83ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(mainevt_sound_write);
	ZetSetReadHandler(mainevt_sound_read);
	ZetMemEnd();
	ZetClose();

	K052109Init(DrvGfxROM0, (gfx0_offset * 2) - 1);
	K052109SetCallback(nGame ? DvK052109Callback : K052109Callback);
	K052109AdjustScroll(nGame ? 0 : 8, 0);

	K051960Init(DrvGfxROM1, 0xfffff);
	K051960SetCallback(nGame ? DvK051960Callback : K051960Callback);
	K051960SetSpriteOffset(nGame ? 0 : 8, 0);

	K007232Init(0, 3579545, DrvSndROM0, 0x80000);
	K007232SetPortWriteHandler(0, DrvK007232VolCallback);
	K007232PCMSetAllRoutes(0, 0.20, BURN_SND_ROUTE_BOTH);

	BurnYM2151Init(3579545);
	BurnYM2151SetAllRoutes(0.30, BURN_SND_ROUTE_BOTH);

	UPD7759Init(0, UPD7759_STANDARD_CLOCK, DrvSndROM1);
	UPD7759SetRoute(0, 0.50, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	KonamiICExit();

	HD6309Exit();
	ZetExit();

	K007232Exit();
	UPD7759Exit();
	BurnYM2151Exit();

	BurnFree (AllMem);

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		KonamiRecalcPal(DrvHD6309RAM + 0x1e00, DrvPalette, 0x200);
	}

	K052109UpdateScroll();

	if (nGame)
	{
		K052109RenderLayer(1, 1, DrvGfxROMExp0); 
		K052109RenderLayer(2, 0, DrvGfxROMExp0);

		K051960SpritesRender(DrvGfxROMExp1, 0);
	
		K052109RenderLayer(0, 0, DrvGfxROMExp0);
	}
	else
	{
		if (nBurnLayer & 1) K052109RenderLayer(1, 1, DrvGfxROMExp0); 

		if (nBurnLayer & 2) K052109RenderLayer(2 | 0x10, 0, DrvGfxROMExp0);

		if (nSpriteEnable & 2) K051960SpritesRender(DrvGfxROMExp1, 1);

		if (nBurnLayer & 4) K052109RenderLayer(2 | 0x00, 0, DrvGfxROMExp0);

		if (nSpriteEnable & 4) K051960SpritesRender(DrvGfxROMExp1, 0); // makes sense...

		if (nSpriteEnable & 1) K051960SpritesRender(DrvGfxROMExp1, 2);

		if (nBurnLayer & 8) K052109RenderLayer(0, 0, DrvGfxROMExp0);
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
		memset (DrvInputs, 0xff, 5);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
		}

	  // Clear Opposites
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[2] & 0x0c) == 0) DrvInputs[2] |= 0x0c;
		if ((DrvInputs[2] & 0x03) == 0) DrvInputs[2] |= 0x03;
		if ((DrvInputs[3] & 0x0c) == 0) DrvInputs[3] |= 0x0c;
		if ((DrvInputs[3] & 0x03) == 0) DrvInputs[3] |= 0x03;
		if ((DrvInputs[4] & 0x0c) == 0) DrvInputs[4] |= 0x0c;
		if ((DrvInputs[4] & 0x03) == 0) DrvInputs[4] |= 0x03;
	}

	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = nBurnSoundLen;
	INT32 nCyclesTotal[2] = { 12000000 / 60, 3579545 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	ZetOpen(0);
	HD6309Open(0);

	INT32 nCyclesSoundIrq = nInterleave / (8 >> nGame);
	INT32 nCyclesSoundIrqTrigger = nCyclesSoundIrq;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nSegment = (nCyclesTotal[0] / nInterleave) * (i + 1);

		nCyclesDone[0] += HD6309Run(nSegment - nCyclesDone[0]);

		nSegment = (nCyclesTotal[1] / nInterleave) * (i + 1);

		nCyclesDone[1] += ZetRun(nSegment - nCyclesDone[1]);

		if (i == nCyclesSoundIrqTrigger-1) {
			nCyclesSoundIrqTrigger+=nCyclesSoundIrq;
			if (*irq_enable) {
				if (nGame) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
				else ZetNmi();
			}
		}

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			UPD7759Update(0, pSoundBuf, nSegmentLength);
			K007232Update(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (nGame) {
		if (nmi_enable[0]) HD6309SetIRQLine(0x20, HD6309_IRQSTATUS_AUTO); // nmi
	} else {
		if (K052109_irq_enabled) HD6309SetIRQLine(HD6309_IRQ_LINE, HD6309_IRQSTATUS_AUTO);
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

	HD6309Close();
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	return 1; // hd6309 isn't hooked up...

	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029704;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		HD6309Scan(nAction);
		ZetScan(nAction);

		UPD7759Scan(0, nAction, pnMin);
		BurnYM2151Scan(nAction);
		K007232Scan(nAction, pnMin);

		KonamiICScan(nAction);
	}

	if (nAction & ACB_WRITE) {
		HD6309Open(0);
		bankswitch(nDrvBank[0]);
		HD6309Close();

		soundbankswitch(nDrvBank[1]);
	}

	return 0;
}


// The Main Event (4 Players ver. Y)

static struct BurnRomInfo mainevtRomDesc[] = {
	{ "799c02.k11",		0x10000, 0xe2e7dbd5, 1 | BRF_PRG | BRF_ESS }, //  0 HD6309 Code

	{ "799c01.f7",		0x08000, 0x447c4c5c, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "799c06.f22",		0x08000, 0xf839cb58, 3 | BRF_GRA },           //  2 K052109 Tiles
	{ "799c07.h22",		0x08000, 0x176df538, 3 | BRF_GRA },           //  3
	{ "799c08.j22",		0x08000, 0xd01e0078, 3 | BRF_GRA },           //  4
	{ "799c09.k22",		0x08000, 0x9baec75e, 3 | BRF_GRA },           //  5

	{ "799b04.h4",		0x80000, 0x323e0c2b, 4 | BRF_GRA },           //  6 K051960 Tiles
	{ "799b05.k4",		0x80000, 0x571c5831, 4 | BRF_GRA },           //  7

	{ "799b03.d4",		0x80000, 0xf1cfd342, 5 | BRF_SND },           //  8 K007232 Samples

	{ "799b06.c22",		0x80000, 0x2c8c47d7, 6 | BRF_GRA },           //  9 UPD7759

	{ "63s141n.k14",	0x00100, 0x61f6c8d1, 7 | BRF_OPT },           // 10 Priority Prom
};

STD_ROM_PICK(mainevt)
STD_ROM_FN(mainevt)

static INT32 mainevtInit()
{
	return DrvInit(0);
}


struct BurnDriver BurnDrvMainevt = {
	"mainevt", NULL, NULL, NULL, "1988",
	"The Main Event (4 Players ver. Y)\0", NULL, "Konami", "GX799",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, mainevtRomInfo, mainevtRomName, NULL, NULL, MainevtInputInfo, MainevtDIPInfo,
	mainevtInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	288, 224, 4, 3
};


// The Main Event (4 Players ver. F)

static struct BurnRomInfo mainevtoRomDesc[] = {
	{ "799_02.k11",		0x10000, 0xc143596b, 1 | BRF_PRG | BRF_ESS }, //  0 HD6309 Code

	{ "799c01.f7",		0x08000, 0x447c4c5c, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "799c06.f22",		0x08000, 0xf839cb58, 3 | BRF_GRA },           //  2 K052109 Tiles
	{ "799c07.h22",		0x08000, 0x176df538, 3 | BRF_GRA },           //  3
	{ "799c08.j22",		0x08000, 0xd01e0078, 3 | BRF_GRA },           //  4
	{ "799c09.k22",		0x08000, 0x9baec75e, 3 | BRF_GRA },           //  5

	{ "799b04.h4",		0x80000, 0x323e0c2b, 4 | BRF_GRA },           //  6 K051960 Tiles
	{ "799b05.k4",		0x80000, 0x571c5831, 4 | BRF_GRA },           //  7

	{ "799b03.d4",		0x80000, 0xf1cfd342, 5 | BRF_SND },           //  8 K007232 Samples

	{ "799b06.c22",		0x80000, 0x2c8c47d7, 6 | BRF_GRA },           //  9 UPD7759

	{ "63s141n.k14",	0x00100, 0x61f6c8d1, 7 | BRF_OPT },           // 10 Priority Prom
};

STD_ROM_PICK(mainevto)
STD_ROM_FN(mainevto)

struct BurnDriver BurnDrvMainevto = {
	"mainevto", "mainevt", NULL, NULL, "1988",
	"The Main Event (4 Players ver. F)\0", NULL, "Konami", "GX799",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, mainevtoRomInfo, mainevtoRomName, NULL, NULL, MainevtInputInfo, MainevtDIPInfo,
	mainevtInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	288, 224, 4, 3
};


// The Main Event (2 Players ver. X)

static struct BurnRomInfo mainevt2pRomDesc[] = {
	{ "799x02.k11",		0x10000, 0x42cfc650, 1 | BRF_PRG | BRF_ESS }, //  0 HD6309 Code

	{ "799c01.f7",		0x08000, 0x447c4c5c, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "799c06.f22",		0x08000, 0xf839cb58, 3 | BRF_GRA },           //  2 K052109 Tiles
	{ "799c07.h22",		0x08000, 0x176df538, 3 | BRF_GRA },           //  3
	{ "799c08.j22",		0x08000, 0xd01e0078, 3 | BRF_GRA },           //  4
	{ "799c09.k22",		0x08000, 0x9baec75e, 3 | BRF_GRA },           //  5

	{ "799b04.h4",		0x80000, 0x323e0c2b, 4 | BRF_GRA },           //  6 K051960 Tiles
	{ "799b05.k4",		0x80000, 0x571c5831, 4 | BRF_GRA },           //  7

	{ "799b03.d4",		0x80000, 0xf1cfd342, 5 | BRF_SND },           //  8 K007232 Samples

	{ "799b06.c22",		0x80000, 0x2c8c47d7, 6 | BRF_GRA },           //  9 UPD7759

	{ "63s141n.k14",	0x00100, 0x61f6c8d1, 7 | BRF_OPT },           // 10 Priority Prom
};

STD_ROM_PICK(mainevt2p)
STD_ROM_FN(mainevt2p)

struct BurnDriver BurnDrvMainevt2p = {
	"mainevt2p", "mainevt", NULL, NULL, "1988",
	"The Main Event (2 Players ver. X)\0", NULL, "Konami", "GX799",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, mainevt2pRomInfo, mainevt2pRomName, NULL, NULL, Mainevt2pInputInfo, Mainevt2pDIPInfo,
	mainevtInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	288, 224, 4, 3
};


// Ring no Ohja (Japan 2 Players ver. N)

static struct BurnRomInfo ringohjaRomDesc[] = {
	{ "799n02.k11",		0x10000, 0xf9305dd0, 1 | BRF_PRG | BRF_ESS }, //  0 HD6309 Code

	{ "799c01.f7",		0x08000, 0x447c4c5c, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "799c06.f22",		0x08000, 0xf839cb58, 3 | BRF_GRA },           //  2 K052109 Tiles
	{ "799c07.h22",		0x08000, 0x176df538, 3 | BRF_GRA },           //  3
	{ "799c08.j22",		0x08000, 0xd01e0078, 3 | BRF_GRA },           //  4
	{ "799c09.k22",		0x08000, 0x9baec75e, 3 | BRF_GRA },           //  5

	{ "799b04.h4",		0x80000, 0x323e0c2b, 4 | BRF_GRA },           //  6 K051960 Tiles
	{ "799b05.k4",		0x80000, 0x571c5831, 4 | BRF_GRA },           //  7

	{ "799b03.d4",		0x80000, 0xf1cfd342, 5 | BRF_SND },           //  8 K007232 Samples

	{ "799b06.c22",		0x80000, 0x2c8c47d7, 6 | BRF_GRA },           //  9 UPD7759

	{ "63s141n.k14",	0x00100, 0x61f6c8d1, 7 | BRF_OPT },           // 10 Priority Prom
};

STD_ROM_PICK(ringohja)
STD_ROM_FN(ringohja)

struct BurnDriver BurnDrvRingohja = {
	"ringohja", "mainevt", NULL, NULL, "1988",
	"Ring no Ohja (Japan 2 Players ver. N)\0", NULL, "Konami", "GX799",
	L"\u30EA\u30F3\u30B0\u306E \u738B\u8005 (Japan 2 Players ver. N)\0Ring no Ohja\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, ringohjaRomInfo, ringohjaRomName, NULL, NULL, Mainevt2pInputInfo, Mainevt2pDIPInfo,
	mainevtInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	288, 224, 4, 3
};


// Devastators (ver. Z)

static struct BurnRomInfo devstorsRomDesc[] = {
	{ "890z02.k11",		0x10000, 0xebeb306f, 1 | BRF_PRG | BRF_ESS }, //  0 HD6309 Code

	{ "890k01.f7",		0x08000, 0xd44b3eb0, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "890f06.f22",		0x10000, 0x26592155, 3 | BRF_GRA },           //  2 K052109 Tiles
	{ "890f07.h22",		0x10000, 0x6c74fa2e, 3 | BRF_GRA },           //  3
	{ "890f08.j22",		0x10000, 0x29e12e80, 3 | BRF_GRA },           //  4
	{ "890f09.k22",		0x10000, 0x67ca40d5, 3 | BRF_GRA },           //  5

	{ "890f04.h4",		0x80000, 0xf16cd1fa, 4 | BRF_GRA },           //  6 K051960 Tiles
	{ "890f05.k4",		0x80000, 0xda37db05, 4 | BRF_GRA },           //  7

	{ "890f03.d4",		0x80000, 0x19065031, 6 | BRF_GRA },           //  8 K007232 Samples

	{ "63s141n.k14",	0x00100, 0xd3620106, 5 | BRF_OPT },           //  9 Priority Prom
};

STD_ROM_PICK(devstors)
STD_ROM_FN(devstors)

static INT32 devstorsInit()
{
	return DrvInit(1);
}

struct BurnDriver BurnDrvDevstors = {
	"devstors", NULL, NULL, NULL, "1988",
	"Devastators (ver. Z)\0", NULL, "Konami", "GX890",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_SHOOT, 0,
	NULL, devstorsRomInfo, devstorsRomName, NULL, NULL, DevstorsInputInfo, DevstorsDIPInfo,
	devstorsInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	224, 304, 3, 4
};


// Devastators (ver. X)

static struct BurnRomInfo devstors2RomDesc[] = {
	{ "890x02.k11",		0x10000, 0xe58ebb35, 1 | BRF_PRG | BRF_ESS }, //  0 HD6309 Code

	{ "890k01.f7",		0x08000, 0xd44b3eb0, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "890f06.f22",		0x10000, 0x26592155, 3 | BRF_GRA },           //  2 K052109 Tiles
	{ "890f07.h22",		0x10000, 0x6c74fa2e, 3 | BRF_GRA },           //  3
	{ "890f08.j22",		0x10000, 0x29e12e80, 3 | BRF_GRA },           //  4
	{ "890f09.k22",		0x10000, 0x67ca40d5, 3 | BRF_GRA },           //  5

	{ "890f04.h4",		0x80000, 0xf16cd1fa, 4 | BRF_GRA },           //  6 K051960 Tiles
	{ "890f05.k4",		0x80000, 0xda37db05, 4 | BRF_GRA },           //  7

	{ "890f03.d4",		0x80000, 0x19065031, 6 | BRF_GRA },           //  8 K007232 Samples

	{ "63s141n.k14",	0x00100, 0xd3620106, 5 | BRF_OPT },           //  9 Priority Prom
};

STD_ROM_PICK(devstors2)
STD_ROM_FN(devstors2)

struct BurnDriver BurnDrvDevstors2 = {
	"devstors2", "devstors", NULL, NULL, "1988",
	"Devastators (ver. X)\0", NULL, "Konami", "GX890",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_SHOOT, 0,
	NULL, devstors2RomInfo, devstors2RomName, NULL, NULL, DevstorsInputInfo, Devstors2DIPInfo,
	devstorsInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	224, 304, 3, 4
};


// Devastators (ver. V)

static struct BurnRomInfo devstors3RomDesc[] = {
	{ "890k02.k11",		0x10000, 0x52f4ccdd, 1 | BRF_PRG | BRF_ESS }, //  0 HD6309 Code

	{ "890k01.f7",		0x08000, 0xd44b3eb0, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "890f06.f22",		0x10000, 0x26592155, 3 | BRF_GRA },           //  2 K052109 Tiles
	{ "890f07.h22",		0x10000, 0x6c74fa2e, 3 | BRF_GRA },           //  3
	{ "890f08.j22",		0x10000, 0x29e12e80, 3 | BRF_GRA },           //  4
	{ "890f09.k22",		0x10000, 0x67ca40d5, 3 | BRF_GRA },           //  5

	{ "890f04.h4",		0x80000, 0xf16cd1fa, 4 | BRF_GRA },           //  6 K051960 Tiles
	{ "890f05.k4",		0x80000, 0xda37db05, 4 | BRF_GRA },           //  7

	{ "890f03.d4",		0x80000, 0x19065031, 6 | BRF_GRA },           //  8 K007232 Samples

	{ "63s141n.k14",	0x00100, 0xd3620106, 5 | BRF_OPT },           //  9 Priority Prom
};

STD_ROM_PICK(devstors3)
STD_ROM_FN(devstors3)

struct BurnDriver BurnDrvDevstors3 = {
	"devstors3", "devstors", NULL, NULL, "1988",
	"Devastators (ver. V)\0", NULL, "Konami", "GX890",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_SHOOT, 0,
	NULL, devstors3RomInfo, devstors3RomName, NULL, NULL, DevstorsInputInfo, DevstorsDIPInfo,
	devstorsInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	224, 304, 3, 4
};


// Garuka (Japan ver. W)

static struct BurnRomInfo garukaRomDesc[] = {
	{ "890w02.k11",		0x10000, 0xb2f6f538, 1 | BRF_PRG | BRF_ESS }, //  0 HD6309 Code

	{ "890k01.f7",		0x08000, 0xd44b3eb0, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "890f06.f22",		0x10000, 0x26592155, 3 | BRF_GRA },           //  2 K052109 Tiles
	{ "890f07.h22",		0x10000, 0x6c74fa2e, 3 | BRF_GRA },           //  3
	{ "890f08.j22",		0x10000, 0x29e12e80, 3 | BRF_GRA },           //  4
	{ "890f09.k22",		0x10000, 0x67ca40d5, 3 | BRF_GRA },           //  5

	{ "890f04.h4",		0x80000, 0xf16cd1fa, 4 | BRF_GRA },           //  6 K051960 Tiles
	{ "890f05.k4",		0x80000, 0xda37db05, 4 | BRF_GRA },           //  7

	{ "890f03.d4",		0x80000, 0x19065031, 6 | BRF_GRA },           //  8 K007232 Samples

	{ "63s141n.k14",	0x00100, 0xd3620106, 5 | BRF_OPT },           //  9 Priority Prom
};

STD_ROM_PICK(garuka)
STD_ROM_FN(garuka)

struct BurnDriver BurnDrvGaruka = {
	"garuka", "devstors", NULL, NULL, "1988",
	"Garuka (Japan ver. W)\0", NULL, "Konami", "GX890",
	L"\u9913\u6D41\u798D (Japan ver. W)\0Garuka\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_SHOOT, 0,
	NULL, garukaRomInfo, garukaRomName, NULL, NULL, DevstorsInputInfo, Devstors2DIPInfo,
	devstorsInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	224, 304, 3, 4
};
