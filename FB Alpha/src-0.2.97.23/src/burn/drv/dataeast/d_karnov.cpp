// FB Alpha Chelnov / Karnov / Wonder Planet driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "sek.h"
#include "m6502_intf.h"
#include "burn_ym2203.h"
#include "burn_ym3526.h"

static UINT8 *AllMem;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *MemEnd;
static UINT8 *Drv68KROM;
static UINT8 *Drv6502ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvColPROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvPfRAM;
static UINT8 *Drv6502RAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprBuf;

static UINT32 *Palette;
static UINT32 *DrvPalette;
static UINT8  DrvRecalc;

static UINT16 *DrvScroll;

static UINT8 *soundlatch;
static UINT8 *flipscreen;

static UINT16 i8751_return;
static UINT16 i8751_needs_ack;
static UINT16 i8751_coin_pending;
static UINT16 i8751_command_queue;
static INT32 i8751_level;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT16 DrvInput[3];
static UINT8 DrvDip[2];
static UINT8 DrvReset;

static bool bUseAsm68KCoreOldValue = false;

enum { KARNOV=0, KARNOVJ, CHELNOV, CHELNOVJ, CHELNOVW, WNDRPLNT };
static INT32 microcontroller_id;
static INT32 coin_mask;
static INT32 vblank;

static struct BurnInputInfo KarnovInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 14,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 0,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDip + 1,	"dip"		},
};

STDINPUTINFO(Karnov)

static struct BurnInputInfo ChelnovInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 6,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 14,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDip + 1,	"dip"		},
};

STDINPUTINFO(Chelnov)

static struct BurnDIPInfo KarnovDIPList[]=
{
	{0x14, 0xff, 0xff, 0xaf, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x14, 0x01, 0x03, 0x00, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x03, 0x01, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x14, 0x01, 0x0c, 0x00, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x0c, 0x04, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x20, 0x20, "Off"			},
	{0x14, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x14, 0x01, 0x40, 0x00, "Upright"		},
	{0x14, 0x01, 0x40, 0x40, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x01, "1"			},
	{0x15, 0x01, 0x03, 0x03, "3"			},
	{0x15, 0x01, 0x03, 0x02, "5"			},
	{0x15, 0x01, 0x03, 0x00, "Infinite (Cheat)"	},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x15, 0x01, 0x0c, 0x0c, "50 K"			},
	{0x15, 0x01, 0x0c, 0x08, "70 K"			},
	{0x15, 0x01, 0x0c, 0x04, "90 K"			},
	{0x15, 0x01, 0x0c, 0x00, "100 K"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x30, 0x30, "Easy"			},
	{0x15, 0x01, 0x30, 0x10, "Normal"		},
	{0x15, 0x01, 0x30, 0x20, "Hard"			},
	{0x15, 0x01, 0x30, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x40, 0x00, "Off"			},
	{0x15, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Timer Speed"		},
	{0x15, 0x01, 0x80, 0x80, "Normal"		},
	{0x15, 0x01, 0x80, 0x00, "Fast"			},
};

STDDIPINFO(Karnov)

static struct BurnDIPInfo WndrplntDIPList[]=
{
	{0x14, 0xff, 0xff, 0x6f, NULL			},
	{0x15, 0xff, 0xff, 0xd3, NULL			},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x14, 0x01, 0x03, 0x00, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x03, 0x01, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x14, 0x01, 0x0c, 0x00, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x0c, 0x04, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x20, 0x00, "Off"			},
	{0x14, 0x01, 0x20, 0x20, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x14, 0x01, 0x80, 0x00, "Upright"		},
	{0x14, 0x01, 0x80, 0x80, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x01, "1"			},
	{0x15, 0x01, 0x03, 0x03, "3"			},
	{0x15, 0x01, 0x03, 0x02, "5"			},
	{0x15, 0x01, 0x03, 0x00, "Infinite (Cheat)"	},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x15, 0x01, 0x10, 0x00, "No"			},
	{0x15, 0x01, 0x10, 0x10, "Yes"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0xc0, 0x80, "Easy"			},
	{0x15, 0x01, 0xc0, 0xc0, "Normal"		},
	{0x15, 0x01, 0xc0, 0x40, "Hard"			},
	{0x15, 0x01, 0xc0, 0x00, "Hardest"		},
};

STDDIPINFO(Wndrplnt)

static struct BurnDIPInfo ChelnovDIPList[]=
{
	{0x14, 0xff, 0xff, 0x6f, NULL			},
	{0x15, 0xff, 0xff, 0xdf, NULL			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x14, 0x01, 0x03, 0x03, "1 Coin 2 Credits"	},
	{0x14, 0x01, 0x03, 0x02, "1 Coin 3 Credits"	},
	{0x14, 0x01, 0x03, 0x01, "1 Coin 4 Credits"	},
	{0x14, 0x01, 0x03, 0x00, "1 Coin 6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x14, 0x01, 0x0c, 0x00, "4 Coins 1 Credits"	},
	{0x14, 0x01, 0x0c, 0x04, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x0c, 0x08, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x20, 0x00, "Off"			},
	{0x14, 0x01, 0x20, 0x20, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x14, 0x01, 0x80, 0x00, "Upright"		},
	{0x14, 0x01, 0x80, 0x80, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x01, "1"			},
	{0x15, 0x01, 0x03, 0x03, "3"			},
	{0x15, 0x01, 0x03, 0x02, "5"			},
	{0x15, 0x01, 0x03, 0x00, "Infinite (Cheat)"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x0c, 0x04, "Easy"			},
	{0x15, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x15, 0x01, 0x0c, 0x08, "Hard"			},
	{0x15, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x15, 0x01, 0x10, 0x00, "No"			},
	{0x15, 0x01, 0x10, 0x10, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x15, 0x01, 0x40, 0x40, "Off"			},
	{0x15, 0x01, 0x40, 0x00, "On"			},
};

STDDIPINFO(Chelnov)

static struct BurnDIPInfo ChelnovuDIPList[]=
{
	{0x14, 0xff, 0xff, 0x6f, NULL			},
	{0x15, 0xff, 0xff, 0xdf, NULL			},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x14, 0x01, 0x03, 0x00, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x03, 0x01, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x14, 0x01, 0x0c, 0x00, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x0c, 0x04, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x20, 0x00, "Off"			},
	{0x14, 0x01, 0x20, 0x20, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x14, 0x01, 0x80, 0x00, "Upright"		},
	{0x14, 0x01, 0x80, 0x80, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x01, "1"			},
	{0x15, 0x01, 0x03, 0x03, "3"			},
	{0x15, 0x01, 0x03, 0x02, "5"			},
	{0x15, 0x01, 0x03, 0x00, "Infinite (Cheat)"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x0c, 0x04, "Easy"			},
	{0x15, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x15, 0x01, 0x0c, 0x08, "Hard"			},
	{0x15, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x15, 0x01, 0x10, 0x00, "No"			},
	{0x15, 0x01, 0x10, 0x10, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x15, 0x01, 0x40, 0x40, "Off"			},
	{0x15, 0x01, 0x40, 0x00, "On"			},
};

STDDIPINFO(Chelnovu)

//------------------------------------------------------------------------------------------------
// These are pretty much ripped straight from MAME

static void karnov_i8751_w(INT32 data)
{
	if (i8751_needs_ack)
	{
		i8751_command_queue=data;
		return;
	}

	i8751_return=0;
	if (data==0x100 && microcontroller_id==KARNOVJ) i8751_return=0x56a; /* Japan version */
	if (data==0x100 && microcontroller_id==KARNOV) i8751_return=0x56b; /* USA version */
	if ((data&0xf00)==0x300) i8751_return=(data&0xff)*0x12; /* Player sprite mapping */

	if (data==0x400) i8751_return=0x4000; /* Get The Map... */
	if (data==0x402) i8751_return=0x40a6; /* Ancient Ruins */
	if (data==0x403) i8751_return=0x4054; /* Forest... */
	if (data==0x404) i8751_return=0x40de; /* ^Rocky hills */
	if (data==0x405) i8751_return=0x4182; /* Sea */
	if (data==0x406) i8751_return=0x41ca; /* Town */
	if (data==0x407) i8751_return=0x421e; /* Desert */
	if (data==0x401) i8751_return=0x4138; /* ^Whistling wind */
	if (data==0x408) i8751_return=0x4276; /* ^Heavy Gates */
	
	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO); /* Signal main cpu task is complete */
	i8751_needs_ack=1;
}

static void wndrplnt_i8751_w(INT32 data)
{
	if (i8751_needs_ack)
	{
		i8751_command_queue=data;
		return;
	}

	i8751_return=0;
	if (data==0x100) i8751_return=0x67a;
	if (data==0x200) i8751_return=0x214;
	if (data==0x300) i8751_return=0x17; /* Copyright text on title screen */
//  if (data==0x300) i8751_return=0x1; /* (USA) Copyright text on title screen */

	if ((data&0x600)==0x600)
	{
		switch (data&0x18)
		{
			case 0x00: 	i8751_return=0x4d53; break;
			case 0x08:	i8751_return=0x4b54; break;
			case 0x10: 	i8751_return=0x5453; break;
			case 0x18:	i8751_return=0x5341; break;
		}
	}

	if (data==0x400) i8751_return=0x594;
	if (data==0x401) i8751_return=0x5ea;
	if (data==0x402) i8751_return=0x628;
	if (data==0x403) i8751_return=0x66c;
	if (data==0x404) i8751_return=0x6a4;
	if (data==0x405) i8751_return=0x6a4;
	if (data==0x406) i8751_return=0x6a4;

	if (data==0x50c) i8751_return=0x13fc;
	if (data==0x50b) i8751_return=0x00ff;
	if (data==0x50a) i8751_return=0x0006;
	if (data==0x509) i8751_return=0x0000;
	if (data==0x508) i8751_return=0x4a39;
	if (data==0x507) i8751_return=0x0006;
	if (data==0x506) i8751_return=0x0000;
	if (data==0x505) i8751_return=0x66f8;
	if (data==0x504) i8751_return=0x4a39;
	if (data==0x503) i8751_return=0x000c;
	if (data==0x502) i8751_return=0x0003;
	if (data==0x501) i8751_return=0x6bf8;
	if (data==0x500) i8751_return=0x4e75;

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	i8751_needs_ack=1;
}


static void chelnov_i8751_w(INT32 data)
{

	if (i8751_needs_ack)
	{
		i8751_command_queue=data;
		return;
	}

	i8751_return=0;
	if (data==0x200 && microcontroller_id==CHELNOVJ) i8751_return=0x7734; /* Japan version */
	if (data==0x200 && microcontroller_id==CHELNOV)  i8751_return=0x783e; /* USA version */
	if (data==0x200 && microcontroller_id==CHELNOVW)  i8751_return=0x7736; /* World version */
	if (data==0x100 && microcontroller_id==CHELNOVJ) i8751_return=0x71a; /* Japan version */
	if (data==0x100 && microcontroller_id==CHELNOV)  i8751_return=0x71b; /* USA version */
	if (data==0x100 && microcontroller_id==CHELNOVW)  i8751_return=0x71c; /* World version */

	if (data>=0x6000 && data<0x8000) i8751_return=1;  /* patched */
	if ((data&0xf000)==0x1000) i8751_level=1; /* Level 1 */
	if ((data&0xf000)==0x2000) i8751_level++; /* Level Increment */

	if ((data&0xf000)==0x3000)
	{        /* Sprite table mapping */
		INT32 b=data&0xff;
		switch (i8751_level)
		{
			case 1: /* Level 1, Sprite mapping tables */
				if (microcontroller_id==CHELNOV)
				{ /* USA */
					if (b<2) i8751_return=0;
					else if (b<6) i8751_return=1;
					else if (b<0xb) i8751_return=2;
					else if (b<0xf) i8751_return=3;
					else if (b<0x13) i8751_return=4;
					else i8751_return=5;
				}
				else
				{ /* Japan, World */
					if (b<3) i8751_return=0;
					else if (b<8) i8751_return=1;
					else if (b<0x0c) i8751_return=2;
					else if (b<0x10) i8751_return=3;
					else if (b<0x19) i8751_return=4;
					else if (b<0x1b) i8751_return=5;
					else if (b<0x22) i8751_return=6;
					else if (b<0x28) i8751_return=7;
					else i8751_return=8;
				}
				break;
			case 2: /* Level 2, Sprite mapping tables, USA & Japan are the same */
				if (b<3) i8751_return=0;
				else if (b<9) i8751_return=1;
				else if (b<0x11) i8751_return=2;
				else if (b<0x1b) i8751_return=3;
				else if (b<0x21) i8751_return=4;
				else if (b<0x28) i8751_return=5;
				else i8751_return=6;
				break;
			case 3: /* Level 3, Sprite mapping tables, USA & Japan are the same */
				if (b<5) i8751_return=0;
				else if (b<9) i8751_return=1;
				else if (b<0x0d) i8751_return=2;
				else if (b<0x11) i8751_return=3;
				else if (b<0x1b) i8751_return=4;
				else if (b<0x1c) i8751_return=5;
				else if (b<0x22) i8751_return=6;
				else if (b<0x27) i8751_return=7;
				else i8751_return=8;
				break;
			case 4: /* Level 4, Sprite mapping tables, USA & Japan are the same */
				if (b<4) i8751_return=0;
				else if (b<0x0c) i8751_return=1;
				else if (b<0x0f) i8751_return=2;
				else if (b<0x19) i8751_return=3;
				else if (b<0x1c) i8751_return=4;
				else if (b<0x22) i8751_return=5;
				else if (b<0x29) i8751_return=6;
				else i8751_return=7;
				break;
			case 5: /* Level 5, Sprite mapping tables */
				if (b<7) i8751_return=0;
				else if (b<0x0e) i8751_return=1;
				else if (b<0x14) i8751_return=2;
				else if (b<0x1a) i8751_return=3;
				else if (b<0x23) i8751_return=4;
				else if (b<0x27) i8751_return=5;
				else i8751_return=6;
				break;
			case 6: /* Level 6, Sprite mapping tables */
				if (b<3) i8751_return=0;
				else if (b<0x0b) i8751_return=1;
				else if (b<0x11) i8751_return=2;
				else if (b<0x17) i8751_return=3;
				else if (b<0x1d) i8751_return=4;
				else if (b<0x24) i8751_return=5;
				else i8751_return=6;
				break;
			case 7: /* Level 7, Sprite mapping tables */
				if (b<5) i8751_return=0;
				else if (b<0x0b) i8751_return=1;
				else if (b<0x11) i8751_return=2;
				else if (b<0x1a) i8751_return=3;
				else if (b<0x21) i8751_return=4;
				else if (b<0x27) i8751_return=5;
				else i8751_return=6;
				break;
		}
	}

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	i8751_needs_ack=1;
}

static void karnov_control_w(INT32 offset, INT32 data)
{
	switch (offset<<1)
	{
		case 0:
			SekSetIRQLine(6, SEK_IRQSTATUS_NONE);

			if (i8751_needs_ack)
			{
				if (i8751_coin_pending)
				{
					i8751_return=i8751_coin_pending;
					SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
					i8751_coin_pending=0;
				}
				else if (i8751_command_queue)
				{
					i8751_needs_ack=0;
					karnov_control_w(3,i8751_command_queue);
					i8751_command_queue=0;
				}
				else
				{
					i8751_needs_ack=0;
				}
			}
			return;

		case 2:
			*soundlatch = data;
			M6502SetIRQ(M6502_INPUT_LINE_NMI, M6502_IRQSTATUS_AUTO);			
			break;

		case 4:
			memcpy (DrvSprBuf, DrvSprRAM, 0x1000); 
			break;

		case 6:
			if (microcontroller_id==KARNOV  || microcontroller_id==KARNOVJ) karnov_i8751_w(data);
			if (microcontroller_id==CHELNOV || microcontroller_id==CHELNOVJ || microcontroller_id==CHELNOVW) chelnov_i8751_w(data);
			if (microcontroller_id==WNDRPLNT) wndrplnt_i8751_w(data);
			break;

		case 8:
			DrvScroll[0] = data;
			*flipscreen = data >> 15;
			break;

		case 0xa:
			DrvScroll[1] = data;
			break;

		case 0xc:
			i8751_needs_ack=0;
			i8751_coin_pending=0;
			i8751_command_queue=0;
			i8751_return=0;
			break;

		case 0xe:
			SekSetIRQLine(7, SEK_IRQSTATUS_NONE);
			break;
	}
}

static UINT16 karnov_control_r(INT32 offset)
{
	switch (offset<<1)
	{
		case 0:
			return DrvInput[0];
		case 2:
			return DrvInput[1] ^ vblank;
		case 4:
			return (DrvDip[1] << 8) | DrvDip[0];
		case 6:
			return i8751_return;
	}

	return ~0;
}

//------------------------------------------------------------------------------------------

void __fastcall karnov_main_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfff800) == 0x0a1800) {
		UINT16 *ptr = (UINT16*)DrvPfRAM;

		INT32 offset = (address >> 1) & 0x3ff;
		offset = ((offset & 0x1f) << 5) | ((offset & 0x3e0) >> 5);

		ptr[offset] = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}

	if ((address & 0xfffff0) == 0x0c0000) {
		karnov_control_w((address >> 1) & 0x07, data);
		return;
	}
}

void __fastcall karnov_main_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xfff800) == 0x0a1800) {
		INT32 offset = (address >> 1) & 0x3ff;
		offset = ((offset & 0x1f) << 5) | ((offset & 0x3e0) >> 5);

		DrvPfRAM[(offset << 1) | (~address & 1)] = data;
		return;
	}

	if ((address & 0xfffff0) == 0x0c0000) {
		karnov_control_w((address >> 1) & 0x07, data);

	}
}

UINT16 __fastcall karnov_main_read_word(UINT32 address)
{
	if ((address & 0xfffff0) == 0x0c0000) {
		return karnov_control_r((address >> 1) & 7);
	}

	return 0;
}

UINT8 __fastcall karnov_main_read_byte(UINT32 address)
{
	if ((address & 0xfffff0) == 0x0c0000) {
		return karnov_control_r((address >> 1) & 7) >> ((~address & 1) << 3);
	}

	return 0;
}

void karnov_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x1000:
		case 0x1001:
			BurnYM2203Write(0, address & 1, data);
		return;

		case 0x1800:
		case 0x1801:
			BurnYM3526Write(address & 1, data);
		return;
	}
}

UINT8 karnov_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x0800:
//			m6502SetIRQ(M6502_CLEAR);
			return *soundlatch;
	}

	return 0;
}

static void DrvYM3526FMIRQHandler(INT32, INT32 nStatus)
{	
	if (nStatus) {
		M6502SetIRQ(M6502_IRQ_LINE, M6502_IRQSTATUS_ACK);
	} else {
		M6502SetIRQ(M6502_IRQ_LINE, M6502_IRQSTATUS_NONE);
	}
}

static INT32 DrvYM3526SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)M6502TotalCycles() * nSoundRate / 1500000;
}

static INT32 DrvYM2203SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)SekTotalCycles() * nSoundRate / 10000000;
}

static double DrvYM2203GetTime()
{
	return (double)SekTotalCycles() / 10000000;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	M6502Open(0);
	M6502Reset();
	M6502Close();

	BurnYM3526Reset();
	BurnYM2203Reset();

	i8751_return = 0;
	i8751_needs_ack = 0;
	i8751_coin_pending = 0;
	i8751_command_queue = 0;
	i8751_level = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x060000;
	Drv6502ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x020000;
	DrvGfxROM1		= Next; Next += 0x080000;
	DrvGfxROM2		= Next; Next += 0x100000;

	DrvColPROM		= Next; Next += 0x000800;

	Palette			= (UINT32*)Next; Next += 0x0300 * sizeof(UINT32);
	DrvPalette		= (UINT32*)Next; Next += 0x0300 * sizeof(UINT32);

	AllRam			= Next;

	Drv68KRAM		= Next; Next += 0x004000;
	DrvPfRAM		= Next; Next += 0x000800;
	Drv6502RAM		= Next; Next += 0x000800;
	DrvVidRAM		= Next; Next += 0x000800;
	DrvSprRAM		= Next; Next += 0x001000;
	DrvSprBuf		= Next; Next += 0x001000;

	soundlatch		= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;

	DrvScroll		= (UINT16*)Next; Next += 0x0002 * sizeof(UINT16);

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 0x300; i++)
	{
		INT32 bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (DrvColPROM[0x000 + i] >> 0) & 0x01;
		bit1 = (DrvColPROM[0x000 + i] >> 1) & 0x01;
		bit2 = (DrvColPROM[0x000 + i] >> 2) & 0x01;
		bit3 = (DrvColPROM[0x000 + i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[0x000 + i] >> 4) & 0x01;
		bit1 = (DrvColPROM[0x000 + i] >> 5) & 0x01;
		bit2 = (DrvColPROM[0x000 + i] >> 6) & 0x01;
		bit3 = (DrvColPROM[0x000 + i] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[0x400 + i] >> 0) & 0x01;
		bit1 = (DrvColPROM[0x400 + i] >> 1) & 0x01;
		bit2 = (DrvColPROM[0x400 + i] >> 2) & 0x01;
		bit3 = (DrvColPROM[0x400 + i] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		Palette[i] = (r << 16) | (g << 8) | b;
		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[3] = { 0x6000*8,0x4000*8,0x2000*8 };
	INT32 Plane1[4] = { 0x60000*8,0x00000*8,0x20000*8,0x40000*8 };
	INT32 XOffs[16] = { 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8), 0,1,2,3,4,5,6,7 };
	INT32 YOffs[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0400, 3,  8,  8, Plane0, XOffs + 8, YOffs, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x80000);

	GfxDecode(0x0800, 4, 16, 16, Plane1, XOffs + 0, YOffs, 0x100, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x80000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs + 0, YOffs, 0x100, tmp, DrvGfxROM2);

	BurnFree (tmp);

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
		if (BurnLoadRom(Drv68KROM + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x000000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x020001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x020000,  3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x040001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x040000,  5, 2)) return 1;

		if (BurnLoadRom(Drv6502ROM + 0x08000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,  7, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x20000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x40000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x60000, 11, 1)) return 1;

		if (microcontroller_id == CHELNOVJ || microcontroller_id == CHELNOVW || microcontroller_id == CHELNOV) {
			if (BurnLoadRom(DrvGfxROM2 + 0x00000, 12, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x20000, 13, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x40000, 14, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x60000, 15, 1)) return 1;
	
			if (BurnLoadRom(DrvColPROM + 0x00000, 16, 1)) return 1;
			if (BurnLoadRom(DrvColPROM + 0x00400, 17, 1)) return 1;
		} else {
			if (BurnLoadRom(DrvGfxROM2 + 0x00000, 12, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x10000, 13, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x20000, 14, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x30000, 15, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x40000, 16, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x50000, 17, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x60000, 18, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x70000, 19, 1)) return 1;
	
			if (BurnLoadRom(DrvColPROM + 0x00000, 20, 1)) return 1;
			if (BurnLoadRom(DrvColPROM + 0x00400, 21, 1)) return 1;
		}

		DrvPaletteInit();
		DrvGfxDecode();
	}

	// These games really don't like the ASM core, so disable it for now
	// and restore it on exit.
	if (bBurnUseASMCPUEmulation) {
		bUseAsm68KCoreOldValue = bBurnUseASMCPUEmulation;
		bBurnUseASMCPUEmulation = false;
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x05ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x060000, 0x063fff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x080000, 0x080fff, SM_RAM);
	SekMapMemory(DrvVidRAM,		0x0a0000, 0x0a07ff, SM_RAM);
	SekMapMemory(DrvVidRAM,		0x0a0800, 0x0a0fff, SM_RAM);
	SekMapMemory(DrvPfRAM,		0x0a1000, 0x0a17ff, SM_WRITE);
	SekSetWriteWordHandler(0,	karnov_main_write_word);
	SekSetWriteByteHandler(0,	karnov_main_write_byte);
	SekSetReadWordHandler(0,	karnov_main_read_word);
	SekSetReadByteHandler(0,	karnov_main_read_byte);
	SekClose();

	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(Drv6502RAM,		0x0000, 0x05ff, M6502_RAM);
	M6502MapMemory(Drv6502ROM + 0x8000,	0x8000, 0xffff, M6502_ROM);
	M6502SetReadByteHandler(karnov_sound_read);
	M6502SetWriteByteHandler(karnov_sound_write);
	M6502Close();

	BurnYM3526Init(3000000, &DrvYM3526FMIRQHandler, &DrvYM3526SynchroniseStream, 0);
	BurnTimerAttachM6502YM3526(1500000);

	BurnYM2203Init(1, 1500000, NULL, DrvYM2203SynchroniseStream, DrvYM2203GetTime, 1);
	BurnTimerAttachSek(10000000);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	SekExit();
	M6502Exit();

	BurnYM3526Exit();
	BurnYM2203Exit();

	BurnFree (AllMem);

	if (bUseAsm68KCoreOldValue) {
		bBurnUseASMCPUEmulation = true;
	}

	return 0;
}

static void draw_txt_layer(INT32 swap)
{
	UINT16 *vram = (UINT16*)DrvVidRAM;
	for (INT32 offs = 0x20; offs < 0x3e0; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		if (swap) {
			INT32 t = sx;
			sx = sy;
			sy = t;
		}

		if (*flipscreen) {
			sy ^= 0xf8;
			sx ^= 0xf8;
		}

		sy -= 8;

		INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[offs]) & 0x0fff;
		INT32 color = BURN_ENDIAN_SWAP_INT16(vram[offs]) >> 14;

		if (code == 0) continue;

		if (*flipscreen) {
			Render8x8Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 3, 0, 0, DrvGfxROM0);
		} else {
			Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 3, 0, 0, DrvGfxROM0);
		}
	}
}

static void draw_bg_layer()
{
	INT32 scrollx = DrvScroll[0] & 0x1ff;
	INT32 scrolly = DrvScroll[1] & 0x1ff;

	UINT16 *vram = (UINT16*)DrvPfRAM;

	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		sy -= scrolly;
		if (sy < -15) sy+=512;
		sx -= scrollx;
		if (sx < -15) sx+=512;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 attr = BURN_ENDIAN_SWAP_INT16(vram[offs]);
		INT32 code = attr & 0x7ff;
		INT32 color= attr >> 12;	

		if (*flipscreen) {
			Render16x16Tile_FlipXY_Clip(pTransDraw, code, 240 - sx, (240 - sy) - 8, color, 4, 0x200, DrvGfxROM1);
		} else {
			Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0x200, DrvGfxROM1);
		}
	}
}

static inline void sprite_routine(INT32 code, INT32 sx, INT32 sy, INT32 color, INT32 fy, INT32 fx)
{
	if (fy) {
		if (fx) {
			Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x100, DrvGfxROM2);
		} else {
			Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x100, DrvGfxROM2);
		}
	} else {
		if (fx) {
			Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x100, DrvGfxROM2);
		} else {
			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x100, DrvGfxROM2);
		}
	}
}

static void draw_sprites()
{
	UINT16 *ram = (UINT16*)DrvSprBuf;

	for (INT32 offs = 0; offs < 0x800; offs+=4)
	{
		INT32 y = BURN_ENDIAN_SWAP_INT16(ram[offs]);
		INT32 x = BURN_ENDIAN_SWAP_INT16(ram[offs + 2]) & 0x1ff;
		if (~y & 0x8000) continue;
		y &= 0x1ff;

		INT32 sprite = BURN_ENDIAN_SWAP_INT16(ram[offs + 3]);
		INT32 color = sprite >> 12;
		sprite &= 0xfff;

		INT32 flipx = BURN_ENDIAN_SWAP_INT16(ram[offs + 1]);
		INT32 flipy = flipx & 0x02;
		INT32 extra = flipx & 0x10;
		flipx &= 0x04;

		x = (x + 16) & 0x1ff;
		y = (y + 16 + extra) & 0x1ff;
		x = 256 - x;
		y = 256 - y;

		if (*flipscreen) {
			y = 240 - y;
			x = 240 - x;
			flipx ^= 0x04;
			flipy ^= 0x02;
			if (extra) y -= 16;
			y -= 8;
		}

		INT32 sprite2 = sprite + 1;
		if (extra && flipy) {
			sprite2--;
			sprite++;
		}

		sprite_routine(sprite, x, y, color, flipy, flipx);

    		if (extra) sprite_routine(sprite2, x, y + 16, color, flipy, flipx);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		UINT8 r,g,b;
		for (INT32 i = 0; i < 0x300; i++) {
			INT32 d = Palette[i];

			r = d >> 16;
			g = d >> 8;
			b = d >> 0;

			DrvPalette[i] = BurnHighCol(r, g, b, 0);
		}
		DrvRecalc = 0;
	}

	draw_bg_layer();
	draw_sprites();
	draw_txt_layer(microcontroller_id == WNDRPLNT);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static void DrvInterrupt()
{
	static INT32 latch;

	if (DrvInput[2] == coin_mask) latch=1;
	if (DrvInput[2] != coin_mask && latch)
	{
		if (i8751_needs_ack)
		{
			i8751_coin_pending = DrvInput[2] | 0x8000;
		}
		else
		{
			i8751_return = DrvInput[2] | 0x8000;
			SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
			i8751_needs_ack=1;
		}
		latch=0;
	}

	SekSetIRQLine(7, SEK_IRQSTATUS_AUTO);
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	SekNewFrame();
	M6502NewFrame();

	{
		memset (DrvInput, 0xff, 2 * sizeof(INT16));
		DrvInput[2] = coin_mask;
		for (INT32 i = 0; i < 16; i++) {
			DrvInput[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInput[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInput[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	INT32 nInterleave = 32;
	INT32 nCyclesTotal[2] = { 10000000 / 60, 1500000 / 60 };

	M6502Open(0);
	SekOpen(0);

	vblank = 0x80;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		if (i == 1) vblank = 0x80;
		if (i == 31) {
			vblank = 0;
			DrvInterrupt();
		}

		BurnTimerUpdate(i * (nCyclesTotal[0] / nInterleave));
		
		BurnTimerUpdateYM3526(i * (nCyclesTotal[1] / nInterleave));
	}

	BurnTimerEndFrame(nCyclesTotal[0]);
	BurnTimerEndFrameYM3526(nCyclesTotal[1]);
	
	if (pBurnSoundOut) {
		BurnYM3526Update(pBurnSoundOut, nBurnSoundLen);	
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
	}


	SekClose();
	M6502Close();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029707;
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
		M6502Scan(nAction);

		BurnYM3526Scan(nAction, pnMin);
		BurnYM2203Scan(nAction, pnMin);

		SCAN_VAR(i8751_return);
		SCAN_VAR(i8751_needs_ack);
		SCAN_VAR(i8751_coin_pending);
		SCAN_VAR(i8751_command_queue);
		SCAN_VAR(i8751_level);
	}

	return 0;
}


// Karnov (US)

static struct BurnRomInfo karnovRomDesc[] = {
	{ "dn08-5",		0x10000, 0xdb92c264, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "dn11-5",		0x10000, 0x05669b4b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "dn07-",		0x10000, 0xfc14291b, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "dn10-",		0x10000, 0xa4a34e37, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "dn06-5",		0x10000, 0x29d64e42, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "dn09-5",		0x10000, 0x072d7c49, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "dn05-5",		0x08000, 0xfa1a31a8, 2 | BRF_PRG | BRF_ESS }, //  6 m6502 Code

	{ "dn00-",		0x08000, 0x0ed77c6d, 3 | BRF_GRA},            //  7 Characters

	{ "dn04-",		0x10000, 0xa9121653, 4 | BRF_GRA},            //  8 Tiles
	{ "dn01-",		0x10000, 0x18697c9e, 4 | BRF_GRA},            //  9
	{ "dn03-",		0x10000, 0x90d9dd9c, 4 | BRF_GRA},            // 10
	{ "dn02-",		0x10000, 0x1e04d7b9, 4 | BRF_GRA},            // 11

	{ "dn12-",		0x10000, 0x9806772c, 5 | BRF_GRA},            // 12 Sprites
	{ "dn14-5",		0x08000, 0xac9e6732, 5 | BRF_GRA},            // 13
	{ "dn13-",		0x10000, 0xa03308f9, 5 | BRF_GRA},            // 14
	{ "dn15-5",		0x08000, 0x8933fcb8, 5 | BRF_GRA},            // 15
	{ "dn16-",		0x10000, 0x55e63a11, 5 | BRF_GRA},            // 16
	{ "dn17-5",		0x08000, 0xb70ae950, 5 | BRF_GRA},            // 17
	{ "dn18-",		0x10000, 0x2ad53213, 5 | BRF_GRA},            // 18
	{ "dn19-5",		0x08000, 0x8fd4fa40, 5 | BRF_GRA},            // 19

	{ "karnprom.21",	0x00400, 0xaab0bb93, 6 | BRF_GRA},            // 20 Color Color Proms
	{ "karnprom.20",	0x00400, 0x02f78ffb, 6 | BRF_GRA},            // 21
	
	{ "karnov_i8751",  0x01000, 0x00000000, BRF_OPT | BRF_NODUMP},
};

STD_ROM_PICK(karnov)
STD_ROM_FN(karnov)

static INT32 KarnovInit()
{
	microcontroller_id = KARNOV;
	coin_mask = 0;

	return DrvInit();
}

struct BurnDriver BurnDrvKarnov = {
	"karnov", NULL, NULL, NULL, "1987",
	"Karnov (US)\0", NULL, "Data East USA", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM | GBF_HORSHOOT, 0,
	NULL, karnovRomInfo, karnovRomName, NULL, NULL, KarnovInputInfo, KarnovDIPInfo,
	KarnovInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x300, 256, 240, 4, 3
};



// Karnov (Japan)

static struct BurnRomInfo karnovjRomDesc[] = {
	{ "kar8",		0x10000, 0x3e17e268, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "kar11",		0x10000, 0x417c936d, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "dn07-",		0x10000, 0xfc14291b, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "dn10-",		0x10000, 0xa4a34e37, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "kar6",		0x10000, 0xc641e195, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "kar9",		0x10000, 0xd420658d, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "kar5",		0x08000, 0x7c9158f1, 2 | BRF_PRG | BRF_ESS }, //  6 m6502 Code

	{ "dn00-",		0x08000, 0x0ed77c6d, 3 | BRF_GRA},            //  7 Characters

	{ "dn04-",		0x10000, 0xa9121653, 4 | BRF_GRA},            //  8 Tiles
	{ "dn01-",		0x10000, 0x18697c9e, 4 | BRF_GRA},            //  9
	{ "dn03-",		0x10000, 0x90d9dd9c, 4 | BRF_GRA},            // 10
	{ "dn02-",		0x10000, 0x1e04d7b9, 4 | BRF_GRA},            // 11

	{ "dn12-",		0x10000, 0x9806772c, 5 | BRF_GRA},            // 12 Sprites
	{ "kar14",		0x08000, 0xc6b39595, 5 | BRF_GRA},            // 13
	{ "dn13-",		0x10000, 0xa03308f9, 5 | BRF_GRA},            // 14
	{ "kar15",		0x08000, 0x2f72cac0, 5 | BRF_GRA},            // 15
	{ "dn16-",		0x10000, 0x55e63a11, 5 | BRF_GRA},            // 16
	{ "kar17",		0x08000, 0x7851c70f, 5 | BRF_GRA},            // 17
	{ "dn18-",		0x10000, 0x2ad53213, 5 | BRF_GRA},            // 18
	{ "kar19",		0x08000, 0x7bc174bb, 5 | BRF_GRA},            // 19

	{ "karnprom.21",	0x00400, 0xaab0bb93, 6 | BRF_GRA},            // 20 Color Proms
	{ "karnprom.20",	0x00400, 0x02f78ffb, 6 | BRF_GRA},            // 21
	
	{ "karnovj_i8751",  0x01000, 0x00000000, BRF_OPT | BRF_NODUMP},
};

STD_ROM_PICK(karnovj)
STD_ROM_FN(karnovj)

static INT32 KarnovjInit()
{
	microcontroller_id = KARNOVJ;
	coin_mask = 0;

	return DrvInit();
}

struct BurnDriver BurnDrvKarnovj = {
	"karnovj", "karnov", NULL, NULL, "1987",
	"Karnov (Japan)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM | GBF_HORSHOOT, 0,
	NULL, karnovjRomInfo, karnovjRomName, NULL, NULL, KarnovInputInfo, KarnovDIPInfo,
	KarnovjInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x300, 256, 240, 4, 3
};


// Wonder Planet (Japan)

static struct BurnRomInfo wndrplntRomDesc[] = {
	{ "ea08.bin",		0x10000, 0xb0578a14, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ea11.bin",		0x10000, 0x271edc6c, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ea07.bin",		0x10000, 0x7095a7d5, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ea10.bin",		0x10000, 0x81a96475, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "ea06.bin",		0x10000, 0x5951add3, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "ea09.bin",		0x10000, 0xc4b3cb1e, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "ea05.bin",		0x08000, 0x8dbb6231, 2 | BRF_PRG | BRF_ESS }, //  6 m6502 Code

	{ "ea00.bin",		0x08000, 0x9f3cac4c, 3 | BRF_GRA},            //  7 Characters

	{ "ea04.bin",		0x10000, 0x7d701344, 4 | BRF_GRA},            //  8 Tiles
	{ "ea01.bin",		0x10000, 0x18df55fb, 4 | BRF_GRA},            //  9
	{ "ea03.bin",		0x10000, 0x922ef050, 4 | BRF_GRA},            // 10
	{ "ea02.bin",		0x10000, 0x700fde70, 4 | BRF_GRA},            // 11

	{ "ea12.bin",		0x10000, 0xa6d4e99d, 5 | BRF_GRA},            // 12 Sprites
	{ "ea14.bin",		0x10000, 0x915ffdc9, 5 | BRF_GRA},            // 13
	{ "ea13.bin",		0x10000, 0xcd839f3a, 5 | BRF_GRA},            // 14
	{ "ea15.bin",		0x10000, 0xa1f14f16, 5 | BRF_GRA},            // 15
	{ "ea16.bin",		0x10000, 0x7a1d8a9c, 5 | BRF_GRA},            // 16
	{ "ea17.bin",		0x10000, 0x21a3223d, 5 | BRF_GRA},            // 17
	{ "ea18.bin",		0x10000, 0x3fb2cec7, 5 | BRF_GRA},            // 18
	{ "ea19.bin",		0x10000, 0x87cf03b5, 5 | BRF_GRA},            // 19

	{ "ea21.prm",		0x00400, 0xc8beab49, 6 | BRF_GRA},            // 20 Color Proms
	{ "ea20.prm",		0x00400, 0x619f9d1e, 6 | BRF_GRA},            // 21
	
	{ "wndrplnt_i8751",  0x01000, 0x00000000, BRF_OPT | BRF_NODUMP},
};

STD_ROM_PICK(wndrplnt)
STD_ROM_FN(wndrplnt)

static INT32 WndrplntInit()
{
	microcontroller_id = WNDRPLNT;
	coin_mask = 0;

	return DrvInit();
}

struct BurnDriver BurnDrvWndrplnt = {
	"wndrplnt", NULL, NULL, NULL, "1987",
	"Wonder Planet (Japan)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PREFIX_DATAEAST, GBF_VERSHOOT, 0,
	NULL, wndrplntRomInfo, wndrplntRomName, NULL, NULL, KarnovInputInfo, WndrplntDIPInfo,
	WndrplntInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x300, 240, 256, 3, 4
};


// Chelnov - Atomic Runner (World)

static struct BurnRomInfo chelnovRomDesc[] = {
	{ "ee08-e.j16",		0x10000, 0x8275cc3a, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ee11-e.j19",		0x10000, 0x889e40a0, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "a-j14.bin",		0x10000, 0x51465486, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "a-j18.bin",		0x10000, 0xd09dda33, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "ee06-e.j13",		0x10000, 0x55acafdb, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "ee09-e.j17",		0x10000, 0x303e252c, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "ee05-.f3",		0x08000, 0x6a8936b4, 2 | BRF_PRG | BRF_ESS }, //  6 m6502 Code

	{ "ee00-e.c5",		0x08000, 0xe06e5c6b, 3 | BRF_GRA},            //  7 Characters

	{ "ee04-.d18",		0x10000, 0x96884f95, 4 | BRF_GRA},            //  8 Tiles
	{ "ee01-.c15",		0x10000, 0xf4b54057, 4 | BRF_GRA},            //  9
	{ "ee03-.d15",		0x10000, 0x7178e182, 4 | BRF_GRA},            // 10
	{ "ee02-.c18",		0x10000, 0x9d7c45ae, 4 | BRF_GRA},            // 11

	{ "ee12-.f8",		0x10000, 0x9b1c53a5, 5 | BRF_GRA},            // 12 Sprites
	{ "ee13-.f9",		0x10000, 0x72b8ae3e, 5 | BRF_GRA},            // 13
	{ "ee14-.f13",		0x10000, 0xd8f4bbde, 5 | BRF_GRA},            // 14
	{ "ee15-.f15",		0x10000, 0x81e3e68b, 5 | BRF_GRA},            // 15

	{ "ee21.k8",		0x00400, 0xb1db6586, 6 | BRF_GRA},            // 16 Color Proms
	{ "ee20.l6",		0x00400, 0x41816132, 6 | BRF_GRA},            // 17
	
	{ "chelnov_i8751",  0x01000, 0x00000000, BRF_OPT | BRF_NODUMP},
};

STD_ROM_PICK(chelnov)
STD_ROM_FN(chelnov)

static INT32 ChelnovInit()
{
	microcontroller_id = CHELNOVW;
	coin_mask = 0xe0;

	INT32 nRet = DrvInit();

	if (nRet == 0) {
		*((UINT16*)(Drv68KROM + 0x0A26)) = BURN_ENDIAN_SWAP_INT16(0x4E71);
		*((UINT16*)(Drv68KROM + 0x062a)) = BURN_ENDIAN_SWAP_INT16(0x4E71);
	}

	return nRet;
}

struct BurnDriver BurnDrvChelnov = {
	"chelnov", NULL, NULL, NULL, "1988",
	"Chelnov - Atomic Runner (World)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM | GBF_HORSHOOT, 0,
	NULL, chelnovRomInfo, chelnovRomName, NULL, NULL, ChelnovInputInfo, ChelnovDIPInfo,
	ChelnovInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x300, 256, 240, 4, 3
};


// Chelnov - Atomic Runner (US)

static struct BurnRomInfo chelnovuRomDesc[] = {
	{ "ee08-a.j15",		0x10000, 0x2f2fb37b, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ee11-a.j20",		0x10000, 0xf306d05f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ee07-a.j14",		0x10000, 0x9c69ed56, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ee10-a.j18",		0x10000, 0xd5c5fe4b, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "ee06-e.j13",		0x10000, 0x55acafdb, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "ee09-e.j17",		0x10000, 0x303e252c, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "ee05-.f3",		0x08000, 0x6a8936b4, 2 | BRF_PRG | BRF_ESS }, //  6 m6502 Code

	{ "ee00-e.c5",		0x08000, 0xe06e5c6b, 3 | BRF_GRA},            //  7 Characters

	{ "ee04-.d18",		0x10000, 0x96884f95, 4 | BRF_GRA},            //  8 Tiles
	{ "ee01-.c15",		0x10000, 0xf4b54057, 4 | BRF_GRA},            //  9
	{ "ee03-.d15",		0x10000, 0x7178e182, 4 | BRF_GRA},            // 10
	{ "ee02-.c18",		0x10000, 0x9d7c45ae, 4 | BRF_GRA},            // 11

	{ "ee12-.f8",		0x10000, 0x9b1c53a5, 5 | BRF_GRA},            // 12 Sprites
	{ "ee13-.f9",		0x10000, 0x72b8ae3e, 5 | BRF_GRA},            // 13
	{ "ee14-.f13",		0x10000, 0xd8f4bbde, 5 | BRF_GRA},            // 14
	{ "ee15-.f15",		0x10000, 0x81e3e68b, 5 | BRF_GRA},            // 15

	{ "ee21.k8",		0x00400, 0xb1db6586, 6 | BRF_GRA},            // 16 Color Proms
	{ "ee20.l6",		0x00400, 0x41816132, 6 | BRF_GRA},            // 17
	
	{ "chelnovu_i8751",  0x01000, 0x00000000, BRF_OPT | BRF_NODUMP},
};

STD_ROM_PICK(chelnovu)
STD_ROM_FN(chelnovu)

static INT32 ChelnovuInit()
{
	microcontroller_id = CHELNOV;
	coin_mask = 0xe0;

	INT32 nRet = DrvInit();

	if (nRet == 0) {
		*((UINT16*)(Drv68KROM + 0x0A26)) = BURN_ENDIAN_SWAP_INT16(0x4E71);
		*((UINT16*)(Drv68KROM + 0x062a)) = BURN_ENDIAN_SWAP_INT16(0x4E71);
	}

	return nRet;
}

struct BurnDriver BurnDrvChelnovu = {
	"chelnovu", "chelnov", NULL, NULL, "1988",
	"Chelnov - Atomic Runner (US)\0", NULL, "Data East USA", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM | GBF_HORSHOOT, 0,
	NULL, chelnovuRomInfo, chelnovuRomName, NULL, NULL, ChelnovInputInfo, ChelnovuDIPInfo,
	ChelnovuInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x300, 256, 240, 4, 3
};


// Chelnov - Atomic Runner (Japan)

static struct BurnRomInfo chelnovjRomDesc[] = {
	{ "a-j15.bin",		0x10000, 0x1978cb52, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "a-j20.bin",		0x10000, 0xe0ed3d99, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "a-j14.bin",		0x10000, 0x51465486, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "a-j18.bin",		0x10000, 0xd09dda33, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "a-j13.bin",		0x10000, 0xcd991507, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "a-j17.bin",		0x10000, 0x977f601c, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "ee05-.f3",		0x08000, 0x6a8936b4, 2 | BRF_PRG | BRF_ESS }, //  6 m6502 Code

	{ "a-c5.bin",		0x08000, 0x1abf2c6d, 3 | BRF_GRA},            //  7 Characters

	{ "ee04-.d18",		0x10000, 0x96884f95, 4 | BRF_GRA},            //  8 Tiles
	{ "ee01-.c15",		0x10000, 0xf4b54057, 4 | BRF_GRA},            //  9
	{ "ee03-.d15",		0x10000, 0x7178e182, 4 | BRF_GRA},            // 10
	{ "ee02-.c18",		0x10000, 0x9d7c45ae, 4 | BRF_GRA},            // 11

	{ "ee12-.f8",		0x10000, 0x9b1c53a5, 5 | BRF_GRA},            // 12 Sprites
	{ "ee13-.f9",		0x10000, 0x72b8ae3e, 5 | BRF_GRA},            // 13
	{ "ee14-.f13",		0x10000, 0xd8f4bbde, 5 | BRF_GRA},            // 14
	{ "ee15-.f15",		0x10000, 0x81e3e68b, 5 | BRF_GRA},            // 15

	{ "a-k7.bin",		0x00400, 0x309c49d8, 6 | BRF_GRA},            // 16 Color Proms
	{ "ee20.l6",		0x00400, 0x41816132, 6 | BRF_GRA},            // 17
	
	{ "chelnovj_i8751",  0x01000, 0x00000000, BRF_OPT | BRF_NODUMP},
};

STD_ROM_PICK(chelnovj)
STD_ROM_FN(chelnovj)

static INT32 ChelnovjInit()
{
	microcontroller_id = CHELNOVJ;
	coin_mask = 0xe0;

	INT32 nRet = DrvInit();

	if (nRet == 0) {
		*((UINT16*)(Drv68KROM + 0x0A2e)) = BURN_ENDIAN_SWAP_INT16(0x4E71);
		*((UINT16*)(Drv68KROM + 0x062a)) = BURN_ENDIAN_SWAP_INT16(0x4E71);
	}

	return nRet;
}

struct BurnDriver BurnDrvChelnovj = {
	"chelnovj", "chelnov", NULL, NULL, "1988",
	"Chelnov - Atomic Runner (Japan)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM | GBF_HORSHOOT, 0,
	NULL, chelnovjRomInfo, chelnovjRomName, NULL, NULL, ChelnovInputInfo, ChelnovuDIPInfo,
	ChelnovjInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x300, 256, 240, 4, 3
};
