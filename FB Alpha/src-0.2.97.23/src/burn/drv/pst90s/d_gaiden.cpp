// FB Alpha Ninja Gaiden driver module
// Based on MAME driver by Alex Pasadyn, Phil Stroffolino, Nicola Salmoria, and various others

#include "tiles_generic.h"
#include "msm6295.h"
#include "burn_ym2151.h"
#include "burn_ym2203.h"
#include "bitswap.h"


static INT32 game = 0;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static UINT8 *Mem, *MemEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;

static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvGfx0Transp;
static UINT8 *DrvGfx1Transp;
static UINT8 *DrvGfx2Transp;

static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KRAM;
static UINT8 *DrvVidRAM0;
static UINT8 *DrvVidRAM1;
static UINT8 *DrvVidRAM2;
static UINT8 *DrvSprRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;

static UINT32 *DrvPalette;
static UINT32 *Palette;
static UINT8 DrvRecalc;

static INT32 flipscreen;
static UINT8 soundlatch;

static INT32 prot;
static INT32 jumpcode;
static INT32 jumppointer;

static INT32 tx_scroll_x;
static INT32 tx_scroll_y;
static INT32 fg_scroll_x;
static INT32 fg_scroll_y;
static INT32 bg_scroll_x;
static INT32 bg_scroll_y;


static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy1 + 6,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy1 + 7,	"p2 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy1 + 0,	"p1 start" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy2 + 0, "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy2 + 1, "p1 right" },
	{"P1 Down"      ,	BIT_DIGITAL  , DrvJoy2 + 2, "p1 down"  },
	{"P1 Up"        ,	BIT_DIGITAL  , DrvJoy2 + 3, "p1 up"    },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 5,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 6,	"p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p1 fire 3"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy1 + 1,	"p2 start" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 0, "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 1, "p2 right" },
	{"P2 Down"      ,	BIT_DIGITAL  , DrvJoy3 + 2, "p2 down"  },
	{"P2 Up"        ,	BIT_DIGITAL  , DrvJoy3 + 3, "p2 up"    },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 5,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 6,	"p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 4,	"p2 fire 3"},

	{"Reset"        ,	BIT_DIGITAL  , &DrvReset  ,	"reset"    },
	{"Dip 1"        ,	BIT_DIPSWITCH, DrvDips + 0,	"dip"	     },
	{"Dip 2"        ,	BIT_DIPSWITCH, DrvDips + 1,	"dip"	     },
};

STDINPUTINFO(Drv)

static struct BurnInputInfo RaigaInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy1 + 6,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy1 + 7,	"p2 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy1 + 0,	"p1 start" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy2 + 0, "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy2 + 1, "p1 right" },
	{"P1 Down"      ,	BIT_DIGITAL  , DrvJoy2 + 2, "p1 down"  },
	{"P1 Up"        ,	BIT_DIGITAL  , DrvJoy2 + 3, "p1 up"    },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 5,	"p1 fire 2"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy1 + 1,	"p2 start" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 0, "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 1, "p2 right" },
	{"P2 Down"      ,	BIT_DIGITAL  , DrvJoy3 + 2, "p2 down"  },
	{"P2 Up"        ,	BIT_DIGITAL  , DrvJoy3 + 3, "p2 up"    },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 4,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 5,	"p2 fire 2"},

	{"Reset"        ,	BIT_DIGITAL  , &DrvReset  ,	"reset"    },
	{"Dip 1"        ,	BIT_DIPSWITCH, DrvDips + 0,	"dip"	     },
	{"Dip 2"        ,	BIT_DIPSWITCH, DrvDips + 1,	"dip"	     },
};

STDINPUTINFO(Raiga)

static struct BurnDIPInfo GaidenDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL	},
	{0x14, 0xff, 0xff, 0xff, NULL },

	{0   , 0xfe, 0   , 2   , "Demo Sounds" },
	{0x13, 0x01, 0x01, 0x00, "Off" },
	{0x13, 0x01, 0x01, 0x01, "On"  },

	{0   , 0xfe, 0   , 2   , "Flip Screen" },
	{0x13, 0x01, 0x02, 0x02, "Off" },
	{0x13, 0x01, 0x02, 0x00, "On"  },

	{0   , 0xfe, 0   , 8   , "Coin A" },
	{0x13, 0x01, 0xe0, 0x00, "5 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0x80, "4 Coins 3 Credits" },
	{0x13, 0x01, 0xe0, 0x40, "3 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0x20, "2 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0xe0, "1 Coin  1 Credit"  },
	{0x13, 0x01, 0xe0, 0x60, "1 Coin  2 Credits" },
	{0x13, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits" },
	{0x13, 0x01, 0xe0, 0xc0, "1 Coin  4 Credits" },

	{0   , 0xfe, 0   , 8   , "Coin B" },
	{0x13, 0x01, 0x1c, 0x00, "5 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x10, "4 Coins 3 Credits" },
	{0x13, 0x01, 0x1c, 0x08, "3 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x04, "2 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x1c, "1 Coin  1 Credit"  },
	{0x13, 0x01, 0x1c, 0x0c, "1 Coin  2 Credits" },
	{0x13, 0x01, 0x1c, 0x14, "1 Coin  3 Credits" },
	{0x13, 0x01, 0x1c, 0x18, "1 Coin  4 Credits" },

	{0   , 0xfe, 0   , 4   , "Energy" },
	{0x14, 0x01, 0x30, 0x00, "2" },
	{0x14, 0x01, 0x30, 0x30, "3" },
	{0x14, 0x01, 0x30, 0x10, "4" },
	{0x14, 0x01, 0x30, 0x20, "5" },

	{0   , 0xfe, 0   , 4   , "Lives" },
	{0x14, 0x01, 0xc0, 0x00, "1" },
	{0x14, 0x01, 0xc0, 0xc0, "2" },
	{0x14, 0x01, 0xc0, 0x40, "3" },
	{0x14, 0x01, 0xc0, 0x80, "4" },
};

STDDIPINFO(Gaiden)

static struct BurnDIPInfo WildfangDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL },
	{0x14, 0xff, 0xff, 0xff, NULL },

	{0   , 0xfe, 0   , 2   , "Demo Sounds" },
	{0x13, 0x01, 0x01, 0x00, "Off" },
	{0x13, 0x01, 0x01, 0x01, "On"  },

	{0   , 0xfe, 0   , 2   , "Flip Screen" },
	{0x13, 0x01, 0x02, 0x02, "Off" },
	{0x13, 0x01, 0x02, 0x00, "On"  },

	{0   , 0xfe, 0   , 8   , "Coin A" },
	{0x13, 0x01, 0xe0, 0x00, "5 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0x80, "4 Coins 3 Credits" },
	{0x13, 0x01, 0xe0, 0x40, "3 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0x20, "2 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0xe0, "1 Coin  1 Credit"  },
	{0x13, 0x01, 0xe0, 0x60, "1 Coin  2 Credits" },
	{0x13, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits" },
	{0x13, 0x01, 0xe0, 0xc0, "1 Coin  4 Credits" },

	{0   , 0xfe, 0   , 8   , "Coin B" },
	{0x13, 0x01, 0x1c, 0x00, "5 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x10, "4 Coins 3 Credits" },
	{0x13, 0x01, 0x1c, 0x08, "3 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x04, "2 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x1c, "1 Coin  1 Credit"  },
	{0x13, 0x01, 0x1c, 0x0c, "1 Coin  2 Credits" },
	{0x13, 0x01, 0x1c, 0x14, "1 Coin  3 Credits" },
	{0x13, 0x01, 0x1c, 0x18, "1 Coin  4 Credits" },

	{0   , 0xfe, 0   , 2   , "Title" },
	{0x14, 0x01, 0x01, 0x01, "Wild Fang"  	},
	{0x14, 0x01, 0x01, 0x00, "Tecmo Knight" },

	{0   , 0xfe, 0   , 4   , "Difficulty" }, // Wild Fang
	{0x14, 0x02, 0x0c, 0x0c, "Easy"    },
	{0x14, 0x00, 0x01, 0x01, NULL },
	{0x14, 0x02, 0x0c, 0x04, "Normal"  },
	{0x14, 0x00, 0x01, 0x01, NULL },
	{0x14, 0x02, 0x0c, 0x08, "Hard"    },
	{0x14, 0x00, 0x01, 0x01, NULL },
	{0x14, 0x02, 0x0c, 0x00, "Hardest" },
	{0x14, 0x00, 0x01, 0x01, NULL },

	{0   , 0xfe, 0   , 4   , "Difficulty"	}, // Tecmo Knight
	{0x14, 0x02, 0x30, 0x30, "Easy"    },
	{0x14, 0x00, 0x01, 0x00, NULL },
	{0x14, 0x02, 0x30, 0x10, "Normal"  },
	{0x14, 0x00, 0x01, 0x00, NULL },
	{0x14, 0x02, 0x30, 0x20, "Hard"    },
	{0x14, 0x00, 0x01, 0x00, NULL },
	{0x14, 0x02, 0x30, 0x00, "Hardest" },
	{0x14, 0x00, 0x01, 0x00, NULL },

	{0   , 0xfe, 0   , 3   , "Lives" },
	{0x14, 0x01, 0xc0, 0x80, "1" },
	{0x14, 0x01, 0xc0, 0xc0, "2" },
	{0x14, 0x01, 0xc0, 0x40, "3" },
//	{0x14, 0x01, 0xc0, 0x00, "2" },
};

STDDIPINFO(Wildfang)

static struct BurnDIPInfo TknightDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL },
	{0x14, 0xff, 0xff, 0xff, NULL },

	{0   , 0xfe, 0   , 2   , "Demo Sounds" },
	{0x13, 0x01, 0x01, 0x00, "Off" },
	{0x13, 0x01, 0x01, 0x01, "On"  },

	{0   , 0xfe, 0   , 2   , "Flip Screen" },
	{0x13, 0x01, 0x02, 0x02, "Off" },
	{0x13, 0x01, 0x02, 0x00, "On"  },

	{0   , 0xfe, 0   , 8   , "Coin A" },
	{0x13, 0x01, 0xe0, 0x00, "5 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0x80, "4 Coins 3 Credits" },
	{0x13, 0x01, 0xe0, 0x40, "3 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0x20, "2 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0xe0, "1 Coin  1 Credit"  },
	{0x13, 0x01, 0xe0, 0x60, "1 Coin  2 Credits" },
	{0x13, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits" },
	{0x13, 0x01, 0xe0, 0xc0, "1 Coin  4 Credits" },

	{0   , 0xfe, 0   , 8   , "Coin B" },
	{0x13, 0x01, 0x1c, 0x00, "5 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x10, "4 Coins 3 Credits" },
	{0x13, 0x01, 0x1c, 0x08, "3 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x04, "2 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x1c, "1 Coin  1 Credit"  },
	{0x13, 0x01, 0x1c, 0x0c, "1 Coin  2 Credits" },
	{0x13, 0x01, 0x1c, 0x14, "1 Coin  3 Credits" },
	{0x13, 0x01, 0x1c, 0x18, "1 Coin  4 Credits" },

	{0   , 0xfe, 0   , 4   , "Difficulty" },
	{0x14, 0x01, 0x30, 0x30, "Easy"    },
	{0x14, 0x01, 0x30, 0x10, "Normal"  },
	{0x14, 0x01, 0x30, 0x20, "Hard"    },
	{0x14, 0x01, 0x30, 0x00, "Hardest" },

	{0   , 0xfe, 0   , 3   , "Lives" },
	{0x14, 0x01, 0xc0, 0x80, "1" },
	{0x14, 0x01, 0xc0, 0xc0, "2" },
	{0x14, 0x01, 0xc0, 0x40, "3" },
//	{0x14, 0x01, 0xc0, 0x00, "2" },
};

STDDIPINFO(Tknight)

static struct BurnDIPInfo DrgnbowlDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL },
	{0x14, 0xff, 0xff, 0xff, NULL },

	{0   , 0xfe, 0   , 2   , "Demo Sounds" },
	{0x13, 0x01, 0x01, 0x00, "Off" },
	{0x13, 0x01, 0x01, 0x01, "On"  },

	{0   , 0xfe, 0   , 8   , "Coin A" },
	{0x13, 0x01, 0xe0, 0x00, "5 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0x80, "4 Coins 3 Credits" },
	{0x13, 0x01, 0xe0, 0x40, "3 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0x20, "2 Coins 1 Credit"  },
	{0x13, 0x01, 0xe0, 0xe0, "1 Coin  1 Credit"  },
	{0x13, 0x01, 0xe0, 0x60, "1 Coin  2 Credits" },
	{0x13, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits" },
	{0x13, 0x01, 0xe0, 0xc0, "1 Coin  4 Credits" },

	{0   , 0xfe, 0   , 8   , "Coin B" },
	{0x13, 0x01, 0x1c, 0x00, "5 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x10, "4 Coins 3 Credits" },
	{0x13, 0x01, 0x1c, 0x08, "3 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x04, "2 Coins 1 Credit"  },
	{0x13, 0x01, 0x1c, 0x1c, "1 Coin  1 Credit"  },
	{0x13, 0x01, 0x1c, 0x0c, "1 Coin  2 Credits" },
	{0x13, 0x01, 0x1c, 0x14, "1 Coin  3 Credits" },
	{0x13, 0x01, 0x1c, 0x18, "1 Coin  4 Credits" },

	{0   , 0xfe, 0   , 4   , "Energy" },
	{0x14, 0x01, 0x30, 0x00, "2" },
	{0x14, 0x01, 0x30, 0x30, "3" },
	{0x14, 0x01, 0x30, 0x10, "4" },
	{0x14, 0x01, 0x30, 0x20, "5" },

	{0   , 0xfe, 0   , 4   , "Lives" },
	{0x14, 0x01, 0xc0, 0x00, "1" },
	{0x14, 0x01, 0xc0, 0xc0, "2" },
	{0x14, 0x01, 0xc0, 0x40, "3" },
	{0x14, 0x01, 0xc0, 0x80, "4" },
};

STDDIPINFO(Drgnbowl)

static struct BurnDIPInfo RaigaDIPList[]=
{
	{0x11, 0xff, 0xff, 0xff, NULL },
	{0x12, 0xff, 0xff, 0x5f, NULL },

	{0   , 0xfe, 0   , 16  , "Coin A" },
	{0x11, 0x01, 0xf0, 0x00, "5 Coins 1 Credit"  },
	{0x11, 0x01, 0xf0, 0x40, "4 Coins 1 Credit"  },
	{0x11, 0x01, 0xf0, 0xa0, "3 Coins 1 Credit"  },
	{0x11, 0x01, 0xf0, 0x10, "2 Coins 1 Credit"  },
	{0x11, 0x01, 0xf0, 0x20, "3 Coins 2 Credits" },
	{0x11, 0x01, 0xf0, 0x80, "4 Coins 3 Credits" },
	{0x11, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"  },
	{0x11, 0x01, 0xf0, 0xc0, "3 Coins 4 Credits" },
	{0x11, 0x01, 0xf0, 0xe0, "2 Coins 3 Credits" },
	{0x11, 0x01, 0xf0, 0x70, "1 Coin  2 Credits" },
	{0x11, 0x01, 0xf0, 0x60, "2 Coins 5 Credits" },
	{0x11, 0x01, 0xf0, 0xb0, "1 Coin  3 Credits" },
	{0x11, 0x01, 0xf0, 0x30, "1 Coin  4 Credits" },
	{0x11, 0x01, 0xf0, 0xd0, "1 Coin  5 Credits" },
	{0x11, 0x01, 0xf0, 0x50, "1 Coin  6 Credits" },
	{0x11, 0x01, 0xf0, 0x90, "1 Coin  7 Credits" },

	{0   , 0xfe, 0   , 16  , "Coin B" },
	{0x11, 0x01, 0x0f, 0x00, "5 Coins 1 Credit"  },
	{0x11, 0x01, 0x0f, 0x04, "4 Coins 1 Credit"  },
	{0x11, 0x01, 0x0f, 0x0a, "3 Coins 1 Credit"  },
	{0x11, 0x01, 0x0f, 0x01, "2 Coins 1 Credit"  },
	{0x11, 0x01, 0x0f, 0x02, "3 Coins 2 Credits" },
	{0x11, 0x01, 0x0f, 0x08, "4 Coins 3 Credits" },
	{0x11, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"  },
	{0x11, 0x01, 0x0f, 0x0c, "3 Coins 4 Credits" },
	{0x11, 0x01, 0x0f, 0x0e, "2 Coins 3 Credits" },
	{0x11, 0x01, 0x0f, 0x07, "1 Coin  2 Credits" },
	{0x11, 0x01, 0x0f, 0x06, "2 Coins 5 Credits" },
	{0x11, 0x01, 0x0f, 0x0b, "1 Coin  3 Credits" },
	{0x11, 0x01, 0x0f, 0x03, "1 Coin  4 Credits" },
	{0x11, 0x01, 0x0f, 0x0d, "1 Coin  5 Credits" },
	{0x11, 0x01, 0x0f, 0x05, "1 Coin  6 Credits" },
	{0x11, 0x01, 0x0f, 0x09, "1 Coin  7 Credits" },

	{0   , 0xfe, 0   , 4   , "Bonus Life" },
	{0x12, 0x01, 0x03, 0x03, "50k 200k"  },
	{0x12, 0x01, 0x03, 0x01, "100k 300k" },
	{0x12, 0x01, 0x03, 0x02, "50k only"  },
	{0x12, 0x01, 0x03, 0x00, "None"    	 },

	{0   , 0xfe, 0   , 4   , "Lives" },
	{0x12, 0x01, 0x0c, 0x00, "2" },
	{0x12, 0x01, 0x0c, 0x0c, "3" },
	{0x12, 0x01, 0x0c, 0x04, "4" },
	{0x12, 0x01, 0x0c, 0x08, "5" },

	{0   , 0xfe, 0   , 4   , "Difficulty" },
	{0x12, 0x01, 0x30, 0x30, "Easy"    },
	{0x12, 0x01, 0x30, 0x10, "Normal"  },
	{0x12, 0x01, 0x30, 0x20, "Hard"    },
	{0x12, 0x01, 0x30, 0x00, "Hardest" },

	{0   , 0xfe, 0   , 2   , "Demo Sounds" },
	{0x12, 0x01, 0x80, 0x80, "Off" },
	{0x12, 0x01, 0x80, 0x00, "On"  },
};

STDDIPINFO(Raiga)

static const INT32 jumppoints_00[0x100] =
{
	0x6669,	   -1,    -1,    -1,    -1,    -1,    -1,    -1,
	    -1,    -1,    -1,    -1,    -1,    -1,0x4a46,    -1,
	    -1,0x6704,    -2,    -1,    -1,    -1,    -1,    -1,
	    -1,    -2,    -1,    -1,    -1,    -1,    -1,    -1,
	    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
	    -2,    -1,    -1,    -1,    -1,0x4e75,    -1,    -1,
	    -1,    -2,    -1,0x4e71,0x60fc,    -1,0x7288,    -1,
	    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1
};

static const INT32 jumppoints_other[0x100] =
{
	0x5457,0x494e,0x5f4b,0x4149,0x5345,0x525f,0x4d49,0x5941,
	0x5241,0x5349,0x4d4f,0x4a49,    -1,    -1,    -1,    -1,
	    -1,    -1,    -2,0x594f,    -1,0x4e75,    -1,    -1,
	    -1,    -2,    -1,    -1,0x4e75,    -1,0x5349,    -1,
	    -1,    -1,    -1,0x4e75,    -1,0x4849,    -1,    -1,
	    -2,    -1,    -1,0x524f,    -1,    -1,    -1,    -1,
	    -1,    -2,    -1,    -1,    -1,    -1,    -1,    -1,
	    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1
};

static const INT32 wildfang_jumppoints[0x100] =
{
	0x0c0c,0x0cac,0x0d42,0x0da2,0x0eea,0x112e,0x1300,0x13fa,
	0x159a,0x1630,0x109a,0x1700,0x1750,0x1806,0x18d6,0x1a44,
	0x1b52,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
	    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
	    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
	    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
	    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
	    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1
};

static const INT32 *jumppoints;

static void protection_w(UINT8 data)
{
	switch (data & 0xf0)
	{
		case 0x00:	// init
			prot = 0x00;
			break;

		case 0x10:	// high 4 bits of jump code
			jumpcode = (data & 0x0f) << 4;
			prot = 0x10;
			break;

		case 0x20:	// low 4 bits of jump code
			jumpcode |= data & 0x0f;
			if (jumppoints[jumpcode] == -2) {
				jumppoints = jumppoints_other;
				jumppointer = 1;
			}

			if (jumppoints[jumpcode] == -1) {
				jumpcode = 0;
			}
			prot = 0x20;
			break;

		case 0x30:	// bits 12-15 of function address
			prot = 0x40 | ((jumppoints[jumpcode] >> 12) & 0x0f);
			break;

		case 0x40:	// bits 8-11 of function address
			prot = 0x50 | ((jumppoints[jumpcode] >>  8) & 0x0f);
			break;

		case 0x50:	// bits 4-7 of function address
			prot = 0x60 | ((jumppoints[jumpcode] >>  4) & 0x0f);
			break;

		case 0x60:	// bits 0-3 of function address
			prot = 0x70 | ((jumppoints[jumpcode] >>  0) & 0x0f);
			break;
	}
}

static UINT8 protection_r()
{
	return prot;
}


static void palette_write(INT32 offset, UINT16 pal)
{
	UINT8 b = (pal >> 8) & 0x0f;
	UINT8 g = (pal >> 4) & 0x0f;
	UINT8 r = (pal >> 0) & 0x0f;

	r = (r << 4) | r;
	g = (g << 4) | g;
	b = (b << 4) | b;

	Palette[offset] = (r << 16) | (g << 8) | b;
	DrvPalette[offset] = BurnHighCol(r, g, b, 0);
}

UINT8 __fastcall gaiden_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x07a001:
			return DrvInputs[0];

		case 0x07a002:
			return DrvInputs[2];

		case 0x07a003:
			return DrvInputs[1];

		case 0x07a004:
			return DrvDips[1];

		case 0x07a005:
			return DrvDips[0];

		case 0x07a007: // Raiga, Wild Fang
			return protection_r();
	}

	return 0;
}

UINT16 __fastcall gaiden_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x07a000:
			return DrvInputs[0];

		case 0x07a002:
			return (DrvInputs[2] << 8) | DrvInputs[1];

		case 0x07a004:
			return (DrvDips[1] << 8) | DrvDips[0];
	}

	return 0;
}

void __fastcall gaiden_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xffffe000) == 0x78000) {
		address &= 0x1fff;

		DrvPalRAM[address ^ 1] = data;

		palette_write(address>>1, *((UINT16*)(DrvPalRAM + (address & ~1))));

		return;
	}

	switch (address)
	{
		case 0x7a00e: // Dragon Bowl
			soundlatch = data;
			ZetRaiseIrq(0);
		return;

		case 0x7a802: // Tecmo Knight
		case 0x7a803: // Ninja Gaiden
			soundlatch = data;
			ZetNmi();
		return;

		case 0x7a804: // Raiga, Wild Fang 
			protection_w(data);
		return;

	}
}

void __fastcall gaiden_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xffffe000) == 0x78000) {
		address &= 0x1ffe;

		*((UINT16*)(DrvPalRAM + address)) = data;

		palette_write(address>>1, *((UINT16*)(DrvPalRAM + address)));

		return;
	}

	switch (address & 0xfffffffe)
	{
		case 0x7a104:
			tx_scroll_y = data & 0x1ff;
		return;

		case 0x7a10c:
			tx_scroll_x = data & 0x3ff;
		return;

		case 0x7a204:
			fg_scroll_y = data & 0x1ff;
		return;

		case 0x7a20c:
			fg_scroll_x = data & 0x3ff;
		return;

		case 0x7a304:
			bg_scroll_y = data & 0x1ff;
		return;

		case 0x7a30c:
			bg_scroll_x = data & 0x3ff;
		return;


		case 0x7a808:
			flipscreen = data & 1;
		return;

		// Dragon Bowl
		case 0x7f000:
			bg_scroll_y = data & 0x1ff;
		return;

		case 0x7f002:
			bg_scroll_x = (data + 248) & 0x3ff;
		return;

		case 0x7f004:
			fg_scroll_y = data & 0x1ff;
		return;

		case 0x7f006:
			fg_scroll_x = (data + 252) & 0x3ff;
		return;
	}
}


void __fastcall gaiden_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xf800:
			MSM6295Command(0, data);
		return;

		case 0xf810:
			BurnYM2203Write(0, 0, data);
		return;

		case 0xf811:
			BurnYM2203Write(0, 1, data);
		return;

		case 0xf820:
			BurnYM2203Write(1, 0, data);
		return;

		case 0xf821:
			BurnYM2203Write(1, 1, data);
		return;
	}
}

UINT8 __fastcall gaiden_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xf800:
			return MSM6295ReadStatus(0);

		case 0xfc00:
			return 0;

		case 0xfc20:
			return soundlatch;
	}

	return 0;
}


void __fastcall drgnbowl_sound_write(UINT16 address, UINT8 data)
{
	switch (address & 0xff)
	{
		case 0x00:
			BurnYM2151SelectRegister(data);
		return;

		case 0x01:
			BurnYM2151WriteRegister(data);
		return;

		case 0x80:
			MSM6295Command(0, data);
		return;
	}
}

UINT8 __fastcall drgnbowl_sound_read(UINT16 address)
{
	switch (address & 0xff)
	{
		case 0x01:
			return BurnYM2151ReadStatus();

		case 0x80:
			return MSM6295ReadStatus(0);

		case 0xc0:
			return soundlatch;
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Drv68KROM	= Next; Next += 0x0040000;
	DrvZ80ROM	= Next; Next += 0x0010000;

	DrvGfxROM0	= Next; Next += 0x0020000;
	DrvGfxROM1	= Next; Next += 0x0100000;
	DrvGfxROM2	= Next; Next += 0x0100000;
	DrvGfxROM3	= Next; Next += 0x0200000;

	DrvGfx0Transp	= Next; Next += 0x0000800;
	DrvGfx1Transp	= Next; Next += 0x0001000;
	DrvGfx2Transp	= Next; Next += 0x0001000;

	MSM6295ROM	= Next; Next += 0x0040000;

	DrvPalette	= (UINT32*)Next; Next += 0x1000 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x0004000;
	DrvVidRAM0	= Next; Next += 0x0001000;
	DrvVidRAM1	= Next; Next += 0x0002000;
	DrvVidRAM2	= Next; Next += 0x0002000;
	DrvSprRAM	= Next; Next += 0x0002000;
	DrvPalRAM	= Next; Next += 0x0002000;

	DrvZ80RAM	= Next; Next += 0x0000800;

	Palette		= (UINT32*)Next; Next += 0x01000 * sizeof(UINT32);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	soundlatch = 0;
	tx_scroll_x = 0;
	tx_scroll_y = 0;
	fg_scroll_x = 0;
	fg_scroll_y = 0;
	bg_scroll_x = 0;
	bg_scroll_y = 0;

	flipscreen = 0;

	prot = 0;
	jumpcode = 0;
	jumppointer = 0;

	if (game == 2) {
		jumppoints = jumppoints_00;
	} else {
		jumppoints = wildfang_jumppoints;
	}

	prot = 0;
	jumpcode = 0;
 
	DrvReset = 0;

	DrvRecalc = 1;

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	MSM6295Reset(0);
	if (game == 1) {
		BurnYM2151Reset();
	} else {
		BurnYM2203Reset();
	}

	SetCurrentFrame(0);

	return 0;
}

static INT32 DrvGfxDecode()
{
	static INT32 Planes[4]         = { 0x000, 0x001, 0x002, 0x003 };
	static INT32 TileXOffsets[16]  = { 0x000, 0x004, 0x008, 0x00c, 0x010, 0x014, 0x018, 0x01c,
					 0x100, 0x104, 0x108, 0x10c, 0x110, 0x114, 0x118, 0x11c };
	static INT32 TileYOffsets[16]  = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
					 0x200, 0x220, 0x240, 0x260, 0x280, 0x2a0, 0x2c0, 0x2e0 };

	static INT32 SpriteXOffsets[8] = { 0x000000, 0x000004, 0x400000, 0x400004,
					 0x000008, 0x00000c, 0x400008, 0x40000c };
	static INT32 SpriteYOffsets[8] = { 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x100000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x010000);

	GfxDecode(0x00800, 4,  8,  8, Planes, TileXOffsets, TileYOffsets, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x080000);

	GfxDecode(0x01000, 4, 16, 16, Planes, TileXOffsets, TileYOffsets, 0x400, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x080000);

	GfxDecode(0x01000, 4, 16, 16, Planes, TileXOffsets, TileYOffsets, 0x400, tmp, DrvGfxROM2);

	memcpy (tmp, DrvGfxROM3, 0x100000);

	GfxDecode(0x08000, 4,  8,  8, Planes, SpriteXOffsets, SpriteYOffsets, 0x080, tmp, DrvGfxROM3);

	BurnFree (tmp);

	// Make transparency tables
	{
		memset(DrvGfx0Transp, 2, 0x0800);
		for (INT32 i = 0; i < 0x20000; i++) {
			if (DrvGfxROM0[i] != 0) DrvGfx0Transp[i>>6] |= 1;
			if (DrvGfxROM0[i] == 0) DrvGfx0Transp[i>>6] &= ~2;
		}

		memset(DrvGfx1Transp, 2, 0x1000);
		memset(DrvGfx2Transp, 2, 0x1000);
		for (INT32 i = 0; i < 0x100000; i++) {
			if (DrvGfxROM1[i] != 0) DrvGfx1Transp[i>>8] |= 1;
			if (DrvGfxROM1[i] == 0) DrvGfx1Transp[i>>8] &= ~2;
			if (DrvGfxROM2[i] != 0) DrvGfx2Transp[i>>8] |= 1;
			if (DrvGfxROM2[i] == 0) DrvGfx2Transp[i>>8] &= ~2;	
		}
	}

	return 0;
}

static INT32 drgnbowlDecode()
{
	UINT8 *buf = (UINT8*)BurnMalloc(0x100000);
	if (buf == NULL) {
		return 1;
	}

	memcpy (buf, Drv68KROM, 0x40000);

	for (INT32 i = 0; i < 0x40000; i++) {
		Drv68KROM[i] = buf[BITSWAP24(i,23,22,21,20,19,18,17,15,16,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)];
	}

	for (INT32 i = 0; i < 0x100000; i++) {
		buf[i] = DrvGfxROM1[BITSWAP24(i,23,22,21,20,19,18,16,17,15,14,13,4,3,12,11,10,9,8,7,6,5,2,1,0)];
	}

	static INT32 CharPlanes[4]    = { 0, 1, 2, 3 };
	static INT32 CharXOffsets[8]  = { 0, 4, 8, 12, 16, 20, 24, 28 };
	static INT32 CharYOffsets[8]  = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0 };

	static INT32 Planes[4] = { 0x600000, 0x400000, 0x200000, 0x000000 };
	static INT32 XOffsets[16]  = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
	static INT32 YOffsets[16]  = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };

	GfxDecode(0x02000, 4, 16, 16, Planes, XOffsets, YOffsets, 0x100, buf, DrvGfxROM1);

	memcpy (buf, DrvGfxROM3, 0x100000);

	for (INT32 i = 0; i < 0x100000; i++) buf[i] ^= 0xff;

	GfxDecode(0x02000, 4, 16, 16, Planes, XOffsets, YOffsets, 0x100, buf, DrvGfxROM3);

	memcpy (buf, DrvGfxROM0, 0x010000);

	GfxDecode(0x00800, 4,  8,  8, CharPlanes, CharXOffsets, CharYOffsets, 0x100, buf, DrvGfxROM0);

	BurnFree (buf);

	// Make transparency tables
	{
		memset(DrvGfx0Transp, 2, 0x0800);
		for (INT32 i = 0; i < 0x20000; i++) {
			if (DrvGfxROM0[i] != 0xf) DrvGfx0Transp[i>>6] |= 1;
			if (DrvGfxROM0[i] == 0xf) DrvGfx0Transp[i>>6] &= ~2;
		}

		memset(DrvGfx1Transp, 3, 0x1000);
		memset(DrvGfx2Transp, 2, 0x1000);
		for (INT32 i = 0; i < 0x100000; i++) {
			if (DrvGfxROM2[i] != 0xf) DrvGfx2Transp[i>>8] |= 1;
			if (DrvGfxROM2[i] == 0xf) DrvGfx2Transp[i>>8] &= ~2;

		}
	}

	return 0;
}

static INT32 DrvGetRoms()
{
	char* pRomName;
	struct BurnRomInfo ri;
	UINT8 *Load68K = Drv68KROM;
	UINT8 *LoadZ80 = DrvZ80ROM;
	UINT8 *LoadG0  = DrvGfxROM0;
	UINT8 *LoadG1  = DrvGfxROM1;
	UINT8 *LoadG2  = DrvGfxROM2;
	UINT8 *LoadG3  = DrvGfxROM3;
	UINT8 *LoadSND = MSM6295ROM;

	for (INT32 i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {
		BurnDrvGetRomInfo(&ri, i);

		if ((ri.nType & 7) == 1) {
			if (BurnLoadRom(Load68K + 1, i + 0, 2)) return 1;
			if (BurnLoadRom(Load68K + 0, i + 1, 2)) return 1;
			Load68K += ri.nLen * 2;
			i++;
			continue;
		}

		if ((ri.nType & 7) == 2) {
			if (BurnLoadRom(LoadZ80, i, 1)) return 1;
			LoadZ80 += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 3) {
			if (BurnLoadRom(LoadG0, i, 1)) return 1;
			LoadG0 += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 4) {
			if (BurnLoadRom(LoadG1, i, 1)) return 1;
			LoadG1 += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 5) {
			if (BurnLoadRom(LoadG2, i, 1)) return 1;
			LoadG2 += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 6) {
			if (BurnLoadRom(LoadG3, i, 1)) return 1;
			LoadG3 += ri.nLen;
			continue;
		}

		if ((ri.nType & 7) == 7) {
			if (BurnLoadRom(LoadSND, i, 1)) return 1;
			LoadSND += ri.nLen;
			continue;
		}		
	}

	return 0;
}

inline static void DrvYM2203IRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

inline static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 4000000;
}


static INT32 DrvInit()
{
	INT32 nLen;

	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	if (DrvGetRoms()) return 1;
	if (game == 1) {
		if (drgnbowlDecode()) return 1;
	} else {
		if (DrvGfxDecode()) return 1;
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x060000, 0x063fff, SM_RAM);
	SekMapMemory(DrvVidRAM0,	0x070000, 0x070fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0x072000, 0x073fff, SM_RAM);
	SekMapMemory(DrvVidRAM2,	0x074000, 0x075fff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x076000, 0x077fff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x078000, 0x079fff, SM_ROM);
	SekSetWriteByteHandler(0,	gaiden_write_byte);
	SekSetWriteWordHandler(0,	gaiden_write_word);
	SekSetReadByteHandler(0,	gaiden_read_byte);
	SekSetReadWordHandler(0,	gaiden_read_word);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	if (game == 1) {
		ZetMapArea(0x0000, 0xf7ff, 0, DrvZ80ROM);
		ZetMapArea(0x0000, 0xf7ff, 2, DrvZ80ROM);
		ZetMapArea(0xf800, 0xffff, 0, DrvZ80RAM);
		ZetMapArea(0xf800, 0xffff, 1, DrvZ80RAM);
		ZetMapArea(0xf800, 0xffff, 2, DrvZ80RAM);
		ZetSetOutHandler(drgnbowl_sound_write);
		ZetSetInHandler(drgnbowl_sound_read);
	} else {
		ZetMapArea(0x0000, 0xefff, 0, DrvZ80ROM);
		ZetMapArea(0x0000, 0xefff, 2, DrvZ80ROM);
		ZetMapArea(0xf000, 0xf7ff, 0, DrvZ80RAM);
		ZetMapArea(0xf000, 0xf7ff, 1, DrvZ80RAM);
		ZetMapArea(0xf000, 0xf7ff, 2, DrvZ80RAM);
		ZetSetWriteHandler(gaiden_sound_write);
		ZetSetReadHandler(gaiden_sound_read);
	}
	ZetMemEnd();
	ZetClose();

	if (game == 1) {
		BurnYM2151Init(4000000, 100.0);
	} else {
		BurnYM2203Init(2, 4000000, &DrvYM2203IRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
		BurnTimerAttachZet(4000000);
	}

	MSM6295Init(0, 1000000 / 132, 80, 1);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	SekExit();
	ZetExit();

	MSM6295Exit(0);
	if (game == 1) {
		BurnYM2151Exit();
	} else {
		BurnYM2203Exit();
	}

	GenericTilesExit();

	BurnFree (Mem);

	game = 0;

	return 0;
}

static void gaiden_draw_sprites(INT32 prior)
{
	static const UINT8 layout[8][8] =
	{
		{ 0, 1, 4, 5,16,17,20,21},
		{ 2, 3, 6, 7,18,19,22,23},
		{ 8, 9,12,13,24,25,28,29},
		{10,11,14,15,26,27,30,31},
		{32,33,36,37,48,49,52,53},
		{34,35,38,39,50,51,54,55},
		{40,41,44,45,56,57,60,61},
		{42,43,46,47,58,59,62,63}
	};

	INT32 count = 0;
	const UINT16 *source = (UINT16*)DrvSprRAM;

	while (count < 256)
	{
		INT32 attributes = source[0];
		INT32 col,row;

		if (attributes & 0x04)
		{
			INT32 priority = (attributes >> 6) & 3;

			if (priority != prior) goto skip_sprite; 

			INT32 flipx = (attributes & 1);
			INT32 flipy = (attributes & 2);

			INT32 color = source[2];
			INT32 sizex = 1 << (color & 3);
			INT32 sizey = 1 << ((color >> (game & 2)) & 3);

			INT32 number = (source[1] & (sizex > 2 ? 0x7ff8 : 0x7ffc));	// raiga

			INT32 ypos = source[3] & 0x01ff;
			INT32 xpos = source[4] & 0x01ff;

			if ((attributes & 0x20) && (GetCurrentFrame() & 1))
				goto skip_sprite;

			color = (color >> 4) & 0x0f;

			if (xpos >= 256)
				xpos -= 512;

			if (ypos >= 256)
				ypos -= 512;

			if (flipscreen)
			{
				flipx = !flipx;
				flipy = !flipy;

				xpos = 256 - (sizex << 3) - xpos;
				ypos = 256 - (sizey << 3) - ypos;

				if (xpos <= -256)
					xpos += 512;

				if (ypos <= -256)
					ypos += 512;
			}

			for (row = 0; row < sizey; row++)
			{
				for (col = 0; col < sizex; col++)
				{
					INT32 sx = xpos + ((flipx ? (sizex - 1 - col) : col) << 3);
					INT32 sy = ypos + ((flipy ? (sizey - 1 - row) : row) << 3);

					sy -= 16;

					if (sx < -15 || sx > 255 || sy < - 15 || sy > 223) continue;

					if (sx >= 0 && sx <= 240 && sy >= 0 && sy <= 208) {
						if (flipy) {
							if (flipx) {
								Render8x8Tile_Mask_FlipXY(pTransDraw, number + layout[row][col], sx, sy, color, 4, 0, 0, DrvGfxROM3);
							} else {
								Render8x8Tile_Mask_FlipY(pTransDraw, number + layout[row][col], sx, sy, color, 4, 0, 0, DrvGfxROM3);
							}
						} else {
							if (flipx) {
								Render8x8Tile_Mask_FlipX(pTransDraw, number + layout[row][col], sx, sy, color, 4, 0, 0, DrvGfxROM3);
							} else {
								Render8x8Tile_Mask(pTransDraw, number + layout[row][col], sx, sy, color, 4, 0, 0, DrvGfxROM3);
							}
						}
					} else {
						if (flipy) {
							if (flipx) {
								Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, number + layout[row][col], sx, sy, color, 4, 0, 0, DrvGfxROM3);
							} else {
								Render8x8Tile_Mask_FlipY_Clip(pTransDraw, number + layout[row][col], sx, sy, color, 4, 0, 0, DrvGfxROM3);
							}
						} else {
							if (flipx) {
								Render8x8Tile_Mask_FlipX_Clip(pTransDraw, number + layout[row][col], sx, sy, color, 4, 0, 0, DrvGfxROM3);
							} else {
								Render8x8Tile_Mask_Clip(pTransDraw, number + layout[row][col], sx, sy, color, 4, 0, 0, DrvGfxROM3);
							}
						}
					}
				}
			}
		}
skip_sprite:
		source += 8;
		count++;
	}
}

static void drgnbowl_draw_sprites(INT32 priority)
{
	UINT16 *spriteram16 = (UINT16*)DrvSprRAM;

	INT32 i, code, color, x, y, flipx, flipy;

	for (i = 0x400 - 4; i >= 0; i -= 4)
	{
		if ((spriteram16[i + 3] & 0x20) != priority)
			continue;

		code = (spriteram16[i] & 0xff) | ((spriteram16[i + 3] & 0x1f) << 8);
		y = 256 - (spriteram16[i + 1] & 0xff) - 12;
		x = spriteram16[i + 2] & 0xff;
		color = spriteram16[0x400 + i] & 0x0f;
		flipx = spriteram16[i + 3] & 0x40;
		flipy = spriteram16[i + 3] & 0x80;

		if(spriteram16[0x400 + i] & 0x80)
			x -= 256;

		x += 256;

		y -= 16;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, x, y, color, 4, 0xf, 0x100, DrvGfxROM3);
				if (x >= 497) Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, x - 512, y, color, 4, 0xf, 0x100, DrvGfxROM3);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, x, y, color, 4, 0xf, 0x100, DrvGfxROM3);
				if (x >= 497) Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, x - 512, y, color, 4, 0xf, 0x100, DrvGfxROM3);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, x, y, color, 4, 0xf, 0x100, DrvGfxROM3);
				if (x >= 497) Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, x - 512, y, color, 4, 0xf, 0x100, DrvGfxROM3);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, x, y, color, 4, 0xf, 0x100, DrvGfxROM3);
				if (x >= 497) Render16x16Tile_Mask_Clip(pTransDraw, code, x - 512, y, color, 4, 0xf, 0x100, DrvGfxROM3);
			}
		}
	}
}

static void draw_layer(UINT16 *vidram, UINT8 *gfxbase, INT32 palette_offset, UINT16 scrollx, UINT16 scrolly, UINT8 *transptab, INT32 transp)
{
	for (INT32 offs = 0; offs < 0x800; offs++)
	{
		INT32 sx = (offs & 0x3f) << 4;
		INT32 sy = (offs >> 6) << 4;
		sx -= scrollx;
		sy -= scrolly;

		if (sx < -15) sx += 1024;
		if (sy < -15) sy +=  512;
		if (sy >= 0x100 || sx >= 0x100) continue;

		if (flipscreen) {
			sy = 0x1ff - sy;
			sx = 0x3ff - sx;
			sy -= 0x110;
			sx -= 0x310;
		}

		sy -= 16;

		INT32 code = vidram[0x0800 + offs] & 0x0fff;
		INT32 color = (vidram[offs] >> 4) & 0x0f;

		if (game == 2) { //raiga
			if (palette_offset == 0x300) {
				color |= (vidram[offs] & 0x08) << 4;
			}
		}

		if (sy >= 0 && sy <= 208 && sx >= 0 && sx <= 240) {
			if (transptab[code] & 1) {
				if (transptab[code] & 2) {
					if (flipscreen) {
						Render16x16Tile_FlipXY(pTransDraw, code, sx, sy, color, 4, palette_offset, gfxbase);
					} else {
						Render16x16Tile(pTransDraw, code, sx, sy, color, 4, palette_offset, gfxbase);
					}
				} else {
					if (flipscreen) {
						Render16x16Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, transp, palette_offset, gfxbase);
					} else {
						Render16x16Tile_Mask(pTransDraw, code, sx, sy, color, 4, transp, palette_offset, gfxbase);
					}
				}
			}
		} else {
			if (transptab[code] & 1) {
				if (transptab[code] & 2) {
					if (flipscreen) {
						Render16x16Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, palette_offset, gfxbase);
					} else {
						Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, palette_offset, gfxbase);
					}
				} else {
					if (flipscreen) {
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, transp, palette_offset, gfxbase);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, transp, palette_offset, gfxbase);
					}
				}
			}
		}
	}
}

static void draw_text(INT32 paloffset, INT32 transp)
{
	UINT16 *vidram = (UINT16*)DrvVidRAM0;

	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 2) & 0xf8;

		if (flipscreen) {
			sy = 0xf8 - sy;
			sx = 0xf8 - sx;
		}

		sx -= tx_scroll_x;
		sy -= tx_scroll_y + 16;

		// tx_scroll is used?

		INT32 code  = vidram[0x400 + offs] & 0x07ff;
		INT32 color = (vidram[0x000 + offs] >> 4) & 0x0f;

		if (!code) continue;

		if (sy >= 0 && sy <= 216 && sx >= 0 && sx <= 248) {
			if (DrvGfx0Transp[code] & 1) {
				if (DrvGfx0Transp[code] & 2) {
					if (flipscreen) {
						Render8x8Tile_FlipXY(pTransDraw, code, sx, sy, color, 4, paloffset, DrvGfxROM0);
					} else {
						Render8x8Tile(pTransDraw, code, sx, sy, color, 4, paloffset, DrvGfxROM0);
					}
				} else {
					if (flipscreen) {
						Render8x8Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, transp, paloffset, DrvGfxROM0);
					} else {
						Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, transp, paloffset, DrvGfxROM0);
					}
				}
			}
		} else {
			if (DrvGfx0Transp[code] & 1) {
				if (DrvGfx0Transp[code] & 2) {
					if (flipscreen) {
						Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, paloffset, DrvGfxROM0);
					} else {
						Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, paloffset, DrvGfxROM0);
					}
				} else {
					if (flipscreen) {
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, transp, paloffset, DrvGfxROM0);
					} else {
						Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, transp, paloffset, DrvGfxROM0);
					}
				}
			}
		}
	}
}

static INT32 DrgnbowlDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x1000; i++) {
			INT32 rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	{
		for (INT32 offs = 0; offs < nScreenWidth * nScreenHeight; offs++) {
			pTransDraw[offs] = 0;
		}
	}

	draw_layer((UINT16*)DrvVidRAM2, DrvGfxROM1, 0x300, bg_scroll_x, bg_scroll_y, DrvGfx1Transp, -1); // no transp
	drgnbowl_draw_sprites(0x20);
	draw_layer((UINT16*)DrvVidRAM1, DrvGfxROM2, 0x200, fg_scroll_x, fg_scroll_y, DrvGfx2Transp, 0xf);
	drgnbowl_draw_sprites(0x00);
	draw_text(0, 0x0f);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x1000; i++) {
			INT32 rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	{
		for (INT32 offs = 0; offs < nScreenWidth * nScreenHeight; offs++) {
			pTransDraw[offs] = 0x200;
		}
	}

	gaiden_draw_sprites(3);
	draw_layer((UINT16*)DrvVidRAM2, DrvGfxROM1, 0x300, bg_scroll_x, bg_scroll_y, DrvGfx1Transp, 0);
	gaiden_draw_sprites(2);
	draw_layer((UINT16*)DrvVidRAM1, DrvGfxROM2, 0x200, fg_scroll_x, fg_scroll_y, DrvGfx2Transp, 0);
	gaiden_draw_sprites(1);
	draw_text(0x100,0);
	gaiden_draw_sprites(0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();
	
	INT32 nInterleave = 10;
	INT32 nSoundBufferPos = 0;

	{
		DrvInputs[0] = DrvInputs[1] = DrvInputs[2] = 0xff;

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	INT32 nCyclesTotal[2] = { (game == 1 ? 10000000 : 9216000) / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext, nCyclesSegment;
		
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == (nInterleave - 1)) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
		
		nCurrentCPU = 1;
		if (game == 1) {
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
		} else {
			BurnTimerUpdate(i * (nCyclesTotal[nCurrentCPU] / nInterleave));
		}

		if (pBurnSoundOut && game == 1) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}		
	}
	
	if (game != 1) BurnTimerEndFrame(nCyclesTotal[1]);
	
	if (pBurnSoundOut) {
		if (game == 1) {
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

			if (nSegmentLength) {
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				MSM6295Render(0, pSoundBuf, nSegmentLength);
			}
		} else {
			BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
			MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
		}
	}		

	ZetClose();
	SekClose();

	if (pBurnDraw) {
		if (game == 1) {
			DrgnbowlDraw();
		} else {
			DrvDraw();
		}
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029523;
	}

	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		MSM6295Scan(0, nAction);

		if (game == 1) {
			BurnYM2151Scan(nAction);
		} else {
			BurnYM2203Scan(nAction, pnMin);
		}

		SCAN_VAR(prot);
		SCAN_VAR(jumpcode);
		SCAN_VAR(jumppointer);

		SCAN_VAR(tx_scroll_x);
		SCAN_VAR(tx_scroll_y);
		SCAN_VAR(fg_scroll_x);
		SCAN_VAR(fg_scroll_y);
		SCAN_VAR(bg_scroll_x);
		SCAN_VAR(bg_scroll_y);

		SCAN_VAR(soundlatch);
		SCAN_VAR(flipscreen);

		if (jumppointer) {
			jumppoints = jumppoints_other;
		}
	}

	return 0;
}


// Shadow Warriors (World, set 1)

static struct BurnRomInfo shadowwRomDesc[] = {
	{ "shadowa_1.3s",	  0x20000, 0x8290d567, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "shadowa_2.4s",	  0x20000, 0xf3f08921, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "gaiden_3.4b",    0x10000, 0x75fd3e6a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "gaiden_5.7a",	  0x10000, 0x8d4035f7, 3 | BRF_GRA },           //  3 Characters

	{ "14.3a",       	  0x20000, 0x1ecfddaa, 4 | BRF_GRA },           //  4 Foreground Tiles
	{ "15.3b",       	  0x20000, 0x1291a696, 4 | BRF_GRA },           //  5
	{ "16.1a",       	  0x20000, 0x140b47ca, 4 | BRF_GRA },           //  6
	{ "17.1b",       	  0x20000, 0x7638cccb, 4 | BRF_GRA },           //  7

	{ "18.6a",       	  0x20000, 0x3fadafd6, 5 | BRF_GRA },           //  8 Background Tiles
	{ "19.6b",       	  0x20000, 0xddae9d5b, 5 | BRF_GRA },           //  9
	{ "20.4b",       	  0x20000, 0x08cf7a93, 5 | BRF_GRA },           // 10
	{ "21.4b",       	  0x20000, 0x1ac892f5, 5 | BRF_GRA },           // 11 

	{ "6.3m",         	0x20000, 0xe7ccdf9f, 6 | BRF_GRA },           // 12 Sprites
	{ "8.3n",         	0x20000, 0x7ef7f880, 6 | BRF_GRA },           // 13
	{ "10.3r",        	0x20000, 0xa6451dec, 6 | BRF_GRA },           // 14
	{ "shadoww_12a.xx", 0x10000, 0x9bb07731, 6 | BRF_GRA },           // 15
	{ "shadoww_12b.xx", 0x10000, 0xa4a950a2, 6 | BRF_GRA },           // 16
	{ "7.1m",         	0x20000, 0x016bec95, 6 | BRF_GRA },           // 17
	{ "9.1n",         	0x20000, 0x6e9b7fd3, 6 | BRF_GRA },           // 18
	{ "11.1r",        	0x20000, 0x7fbfdf5e, 6 | BRF_GRA },           // 19
	{ "shadoww_13a.xx", 0x10000, 0x996d2fa5, 6 | BRF_GRA },           // 20
	{ "shadoww_13b.xx", 0x10000, 0xb8df8a34, 6 | BRF_GRA },           // 21

	{ "4.4a",     		  0x20000, 0xb0e0faf9, 7 | BRF_SND },           // 22 MSM6295 Samples
};

STD_ROM_PICK(shadoww)
STD_ROM_FN(shadoww)

struct BurnDriver BurnDrvShadoww = {
	"shadoww", NULL, NULL, NULL, "1988",
	"Shadow Warriors (World, set 1)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, shadowwRomInfo, shadowwRomName, NULL, NULL, DrvInputInfo, GaidenDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// Shadow Warriors (World, set 2)

static struct BurnRomInfo shadowwaRomDesc[] = {
	{ "shadoww_1.3s",	  0x20000, 0xfefba387, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "shadoww_2.4s",	  0x20000, 0x9b9d6b18, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "gaiden_3.4b",    0x10000, 0x75fd3e6a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "gaiden_5.7a",	  0x10000, 0x8d4035f7, 3 | BRF_GRA },           //  3 Characters

	{ "14.3a",       	  0x20000, 0x1ecfddaa, 4 | BRF_GRA },           //  4 Foreground Tiles
	{ "15.3b",       	  0x20000, 0x1291a696, 4 | BRF_GRA },           //  5
	{ "16.1a",       	  0x20000, 0x140b47ca, 4 | BRF_GRA },           //  6
	{ "17.1b",       	  0x20000, 0x7638cccb, 4 | BRF_GRA },           //  7

	{ "18.6a",       	  0x20000, 0x3fadafd6, 5 | BRF_GRA },           //  8 Background Tiles
	{ "19.6b",       	  0x20000, 0xddae9d5b, 5 | BRF_GRA },           //  9
	{ "20.4b",       	  0x20000, 0x08cf7a93, 5 | BRF_GRA },           // 10
	{ "21.4b",       	  0x20000, 0x1ac892f5, 5 | BRF_GRA },           // 11 

	{ "6.3m",         	0x20000, 0xe7ccdf9f, 6 | BRF_GRA },           // 12 Sprites
	{ "8.3n",         	0x20000, 0x7ef7f880, 6 | BRF_GRA },           // 13
	{ "10.3r",        	0x20000, 0xa6451dec, 6 | BRF_GRA },           // 14
	{ "shadoww_12a.xx", 0x10000, 0x9bb07731, 6 | BRF_GRA },           // 15
	{ "shadoww_12b.xx", 0x10000, 0xa4a950a2, 6 | BRF_GRA },           // 16
	{ "7.1m",         	0x20000, 0x016bec95, 6 | BRF_GRA },           // 17
	{ "9.1n",         	0x20000, 0x6e9b7fd3, 6 | BRF_GRA },           // 18
	{ "11.1r",        	0x20000, 0x7fbfdf5e, 6 | BRF_GRA },           // 19
	{ "shadoww_13a.xx", 0x10000, 0x996d2fa5, 6 | BRF_GRA },           // 20
	{ "shadoww_13b.xx", 0x10000, 0xb8df8a34, 6 | BRF_GRA },           // 21

	{ "4.4a",     		  0x20000, 0xb0e0faf9, 7 | BRF_SND },           // 22 MSM6295 Samples
};

STD_ROM_PICK(shadowwa)
STD_ROM_FN(shadowwa)

struct BurnDriver BurnDrvShadowwa = {
	"shadowwa", "shadoww", NULL, NULL, "1988",
	"Shadow Warriors (World, set 2)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, shadowwaRomInfo, shadowwaRomName, NULL, NULL, DrvInputInfo, GaidenDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// Ninja Gaiden (US)

static struct BurnRomInfo gaidenRomDesc[] = {
	{ "gaiden_1.3s",    0x20000, 0xe037ff7c, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "gaiden_2.4s",    0x20000, 0x454f7314, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "gaiden_3.4b",    0x10000, 0x75fd3e6a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "gaiden_5.7a",	  0x10000, 0x8d4035f7, 3 | BRF_GRA },           //  3 Characters

	{ "14.3a",       	  0x20000, 0x1ecfddaa, 4 | BRF_GRA },           //  4 Foreground Tiles
	{ "15.3b",       	  0x20000, 0x1291a696, 4 | BRF_GRA },           //  5
	{ "16.1a",       	  0x20000, 0x140b47ca, 4 | BRF_GRA },           //  6
	{ "17.1b",       	  0x20000, 0x7638cccb, 4 | BRF_GRA },           //  7

	{ "18.6a",       	  0x20000, 0x3fadafd6, 5 | BRF_GRA },           //  8 Background Tiles
	{ "19.6b",       	  0x20000, 0xddae9d5b, 5 | BRF_GRA },           //  9
	{ "20.4b",       	  0x20000, 0x08cf7a93, 5 | BRF_GRA },           // 10
	{ "21.4b",       	  0x20000, 0x1ac892f5, 5 | BRF_GRA },           // 11 

	{ "6.3m",         	0x20000, 0xe7ccdf9f, 6 | BRF_GRA },           // 12 Sprites
	{ "8.3n",         	0x20000, 0x7ef7f880, 6 | BRF_GRA },           // 13
	{ "10.3r",        	0x20000, 0xa6451dec, 6 | BRF_GRA },           // 14
	{ "gaiden_12.3s", 	0x20000, 0x90f1e13a, 6 | BRF_GRA },           // 15
	{ "7.1m",         	0x20000, 0x016bec95, 6 | BRF_GRA },           // 16
	{ "9.1n",         	0x20000, 0x6e9b7fd3, 6 | BRF_GRA },           // 17
	{ "11.1r",        	0x20000, 0x7fbfdf5e, 6 | BRF_GRA },           // 18
	{ "gaiden_13.1s", 	0x20000, 0x7d9f5c5e, 6 | BRF_GRA },           // 19

	{ "4.4a",     		  0x20000, 0xb0e0faf9, 7 | BRF_SND },           // 20 MSM6295 Samples
};

STD_ROM_PICK(gaiden)
STD_ROM_FN(gaiden)

struct BurnDriver BurnDrvGaiden = {
	"gaiden", "shadoww", NULL, NULL, "1988",
	"Ninja Gaiden (US)\0", NULL, "Tecmo", "Miscellaneous",
	L"Ninja \u5916\u4F1D Gaiden (US)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, gaidenRomInfo, gaidenRomName, NULL, NULL, DrvInputInfo, GaidenDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// Ninja Ryukenden (Japan, set 1)

static struct BurnRomInfo ryukendnRomDesc[] = {
	{ "ryukendn_1.3s",  0x20000, 0x6203a5e2, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ryukendn_2.4s",  0x20000, 0x9e99f522, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "3.4b",   		    0x10000, 0x6b686b69, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "hn27512p.7a",   	0x10000, 0x765e7baa, 3 | BRF_GRA },           //  3 Characters

	{ "14.3a",       	  0x20000, 0x1ecfddaa, 4 | BRF_GRA },           //  4 Foreground Tiles
	{ "15.3b",       	  0x20000, 0x1291a696, 4 | BRF_GRA },           //  5
	{ "16.1a",       	  0x20000, 0x140b47ca, 4 | BRF_GRA },           //  6
	{ "17.1b",       	  0x20000, 0x7638cccb, 4 | BRF_GRA },           //  7

	{ "18.6a",       	  0x20000, 0x3fadafd6, 5 | BRF_GRA },           //  8 Background Tiles
	{ "19.6b",       	  0x20000, 0xddae9d5b, 5 | BRF_GRA },           //  9
	{ "20.4b",       	  0x20000, 0x08cf7a93, 5 | BRF_GRA },           // 10
	{ "21.4b",       	  0x20000, 0x1ac892f5, 5 | BRF_GRA },           // 11 

	{ "6.3m",         	0x20000, 0xe7ccdf9f, 6 | BRF_GRA },           // 12 Sprites
	{ "8.3n",         	0x20000, 0x7ef7f880, 6 | BRF_GRA },           // 13
	{ "10.3r",        	0x20000, 0xa6451dec, 6 | BRF_GRA },           // 14
	{ "shadoww_12a.xx", 0x10000, 0x9bb07731, 6 | BRF_GRA },           // 15
	{ "ryukendn_12b.xx",0x10000, 0x1773628a, 6 | BRF_GRA },           // 16
	{ "7.1m",         	0x20000, 0x016bec95, 6 | BRF_GRA },           // 17
	{ "9.1n",         	0x20000, 0x6e9b7fd3, 6 | BRF_GRA },           // 18
	{ "11.1r",        	0x20000, 0x7fbfdf5e, 6 | BRF_GRA },           // 19
	{ "shadoww_13a.xx", 0x10000, 0x996d2fa5, 6 | BRF_GRA },           // 20
	{ "ryukendn_13b.xx",0x10000, 0x1f43c507, 6 | BRF_GRA },           // 21

	{ "4.4a",     		  0x20000, 0xb0e0faf9, 7 | BRF_SND },           // 22 MSM6295 Samples
};

STD_ROM_PICK(ryukendn)
STD_ROM_FN(ryukendn)

struct BurnDriver BurnDrvRyukendn = {
	"ryukendn", "shadoww", NULL, NULL, "1989",
	"Ninja Ryukenden (Japan, set 1)\0", NULL, "Tecmo", "Miscellaneous",
	L"\u5FCD\u8005 \u9F8D\u5263\u4F1D (Japan, set 1)\0Ninja Ryukenden\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, ryukendnRomInfo, ryukendnRomName, NULL, NULL, DrvInputInfo, GaidenDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// Ninja Ryukenden (Japan, set 2)

static struct BurnRomInfo ryukendaRomDesc[] = {
	{ "1.3s",  		      0x20000, 0x0ed5464c, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "2.4s",  		      0x20000, 0xa93a8256, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "3.4b",   		    0x10000, 0x6b686b69, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "hn27512p.7a",   	0x10000, 0x765e7baa, 3 | BRF_GRA },           //  3 Characters

	{ "14.3a",       	  0x20000, 0x1ecfddaa, 4 | BRF_GRA },           //  4 Foreground Tiles
	{ "15.3b",       	  0x20000, 0x1291a696, 4 | BRF_GRA },           //  5
	{ "16.1a",       	  0x20000, 0x140b47ca, 4 | BRF_GRA },           //  6
	{ "17.1b",       	  0x20000, 0x7638cccb, 4 | BRF_GRA },           //  7

	{ "18.6a",       	  0x20000, 0x3fadafd6, 5 | BRF_GRA },           //  8 Background Tiles
	{ "19.6b",       	  0x20000, 0xddae9d5b, 5 | BRF_GRA },           //  9
	{ "20.4b",       	  0x20000, 0x08cf7a93, 5 | BRF_GRA },           // 10
	{ "21.4b",       	  0x20000, 0x1ac892f5, 5 | BRF_GRA },           // 11 

	{ "6.3m",         	0x20000, 0xe7ccdf9f, 6 | BRF_GRA },           // 12 Sprites
	{ "8.3n",         	0x20000, 0x7ef7f880, 6 | BRF_GRA },           // 13
	{ "10.3r",        	0x20000, 0xa6451dec, 6 | BRF_GRA },           // 14
	{ "12.3s",    		  0x20000, 0x277204f0, 6 | BRF_GRA },           // 15
	{ "7.1m",         	0x20000, 0x016bec95, 6 | BRF_GRA },           // 16
	{ "9.1n",         	0x20000, 0x6e9b7fd3, 6 | BRF_GRA },           // 17
	{ "11.1r",        	0x20000, 0x7fbfdf5e, 6 | BRF_GRA },           // 18
	{ "13.1s",    		  0x20000, 0x4e56a508, 6 | BRF_GRA },           // 19

	{ "4.4a",     		  0x20000, 0xb0e0faf9, 7 | BRF_SND },           // 20 MSM6295 Samples
};

STD_ROM_PICK(ryukenda)
STD_ROM_FN(ryukenda)

struct BurnDriver BurnDrvRyukenda = {
	"ryukendna", "shadoww", NULL, NULL, "1989",
	"Ninja Ryukenden (Japan, set 2)\0", NULL, "Tecmo", "Miscellaneous",
	L"\u5FCD\u8005 \u9F8D\u5263\u4F1D (Japan, set 2)\0Ninja Ryukenden\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, ryukendaRomInfo, ryukendaRomName, NULL, NULL, DrvInputInfo, GaidenDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// Wild Fang / Tecmo Knight

static struct BurnRomInfo wildfangRomDesc[] = {
	{ "1.3st",		    0x20000, 0xab876c9b, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "2.5st",		    0x20000, 0x1dc74b3b, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "tkni3.bin",		0x10000, 0x15623ec7, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "tkni5.bin",		0x10000, 0x5ed15896, 3 | BRF_GRA },           //  3 Characters

	{ "14.3a",		    0x20000, 0x0d20c10c, 4 | BRF_GRA },           //  4 Foreground Tiles
	{ "15.3b",		    0x20000, 0x3f40a6b4, 4 | BRF_GRA },           //  5
	{ "16.1a",		    0x20000, 0x0f31639e, 4 | BRF_GRA },           //  6
	{ "17.1b",		    0x20000, 0xf32c158e, 4 | BRF_GRA },           //  7

	{ "tkni6.bin",		0x80000, 0xf68fafb1, 5 | BRF_GRA },           //  8

	{ "tkni9.bin",		0x80000, 0xd22f4239, 6 | BRF_GRA },           //  9 Sprites
	{ "tkni8.bin",		0x80000, 0x4931b184, 6 | BRF_GRA },           // 10

	{ "tkni4.bin",		0x20000, 0xa7a1dbcf, 7 | BRF_SND },           // 11 MSM6295 Samples
	
	{ "a-6v.mcu",           0x01000, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },
};

STD_ROM_PICK(wildfang)
STD_ROM_FN(wildfang)

struct BurnDriver BurnDrvWildfang = {
	"wildfang", NULL, NULL, NULL, "1989",
	"Wild Fang / Tecmo Knight\0", NULL, "Tecmo", "Miscellaneous",
	L"Wild Fang\0Tecmo Knight\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, wildfangRomInfo, wildfangRomName, NULL, NULL, DrvInputInfo, WildfangDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// Tecmo Knight

static struct BurnRomInfo tknightRomDesc[] = {
	{ "tkni1.bin",		0x20000, 0x9121daa8, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "tkni2.bin",		0x20000, 0x6669cd87, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "tkni3.bin",		0x10000, 0x15623ec7, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "tkni5.bin",		0x10000, 0x5ed15896, 3 | BRF_GRA },           //  3 Characters

	{ "tkni7.bin",		0x80000, 0x4b4d4286, 4 | BRF_GRA },           //  4 Foreground Tiles

	{ "tkni6.bin",		0x80000, 0xf68fafb1, 5 | BRF_GRA },           //  5

	{ "tkni9.bin",		0x80000, 0xd22f4239, 6 | BRF_GRA },           //  6 Sprites
	{ "tkni8.bin",		0x80000, 0x4931b184, 6 | BRF_GRA },           //  7

	{ "tkni4.bin",		0x20000, 0xa7a1dbcf, 7 | BRF_SND },           //  8 MSM6295 Samples
	
	{ "a-6v.mcu",           0x01000, 0x00000000, 0 | BRF_OPT | BRF_NODUMP },
};

STD_ROM_PICK(tknight)
STD_ROM_FN(tknight)

struct BurnDriver BurnDrvTknight = {
	"tknight", "wildfang", NULL, NULL, "1989",
	"Tecmo Knight\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, tknightRomInfo, tknightRomName, NULL, NULL, DrvInputInfo, TknightDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// Raiga - Strato Fighter (US)

static struct BurnRomInfo stratofRomDesc[] = {
	{ "1.3s",		    0x20000, 0x060822a4, 1 | BRF_PRG | BRF_ESS }, // 0 68k Code
	{ "2.4s",		    0x20000, 0x339358fa, 1 | BRF_PRG | BRF_ESS }, // 1

	{ "a-4b.3",		  0x10000, 0x18655c95, 2 | BRF_PRG | BRF_ESS }, // 2 Z80 Code

	{ "b-7a.5",		  0x10000, 0x6d2e4bf1, 3 | BRF_GRA },           // 3 Characters

	{ "b-1b",		    0x80000, 0x781d1bd2, 4 | BRF_GRA },           // 4 Foreground Tiles

	{ "b-4b",		    0x80000, 0x89468b84, 5 | BRF_GRA },           // 5

	{ "b-2m",		    0x80000, 0x5794ec32, 6 | BRF_GRA },           // 6 Sprites
	{ "b-1m",		    0x80000, 0xb0de0ded, 6 | BRF_GRA },           // 7

	{ "a-4a.4",		  0x20000, 0xef9acdcf, 7 | BRF_SND },           // 8 MSM6295 Samples

	{ "a-6v.mcu",		0x01000, 0x00000000, 0 | BRF_NODUMP },	      // 9 MCU
};

STD_ROM_PICK(stratof)
STD_ROM_FN(stratof)

static INT32 stratofInit()
{
	game = 2;

	return DrvInit();
}

struct BurnDriver BurnDrvStratof = {
	"stratof", NULL, NULL, NULL, "1991",
	"Raiga - Strato Fighter (US)\0", NULL, "Tecmo", "Miscellaneous",
	L"\u96F7\u7259 Strato Fighter (US)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_HORSHOOT, 0,
	NULL, stratofRomInfo, stratofRomName, NULL, NULL, RaigaInputInfo, RaigaDIPInfo,
	stratofInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// Raiga - Strato Fighter (Japan)

static struct BurnRomInfo raigaRomDesc[] = {
	{ "a-3s.1",		  0x20000, 0x303c2a6c, 1 | BRF_PRG | BRF_ESS }, // 0 68k Code
	{ "a-4s.2",		  0x20000, 0x5f31fecb, 1 | BRF_PRG | BRF_ESS }, // 1

	{ "a-4b.3",		  0x10000, 0x18655c95, 2 | BRF_PRG | BRF_ESS }, // 2 Z80 Code

	{ "b-7a.5",		  0x10000, 0x6d2e4bf1, 3 | BRF_GRA },           // 3 Characters

	{ "b-1b",		    0x80000, 0x781d1bd2, 4 | BRF_GRA },           // 4 Foreground Tiles

	{ "b-4b",		    0x80000, 0x89468b84, 5 | BRF_GRA },           // 5

	{ "b-2m",		    0x80000, 0x5794ec32, 6 | BRF_GRA },           // 6 Sprites
	{ "b-1m",		    0x80000, 0xb0de0ded, 6 | BRF_GRA },           // 7

	{ "a-4a.4",		  0x20000, 0xef9acdcf, 7 | BRF_SND },           // 8 MSM6295 Samples

	{ "a-6v.mcu",		0x01000, 0x00000000, 0 | BRF_NODUMP },	      // 9 MCU
};

STD_ROM_PICK(raiga)
STD_ROM_FN(raiga)

struct BurnDriver BurnDrvRaiga = {
	"raiga", "stratof", NULL, NULL, "1991",
	"Raiga - Strato Fighter (Japan)\0", NULL, "Tecmo", "Miscellaneous",
	L"\u96F7\u7259 Strato Fighter (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_HORSHOOT, 0,
	NULL, raigaRomInfo, raigaRomName, NULL, NULL, RaigaInputInfo, RaigaDIPInfo,
	stratofInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};


// Dragon Bowl

static struct BurnRomInfo drgnbowlRomDesc[] = {
	{ "4.3h",		  0x20000, 0x90730008, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "5.4h",		  0x20000, 0x193cc915, 1 | BRF_PRG | BRF_ESS }, //  1 

	{ "1.2r",		  0x10000, 0xd9cbf84a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "22.6m",		0x10000, 0x86e41198, 3 | BRF_GRA },           //  3 Characters

	{ "6.5a",		  0x20000, 0xb15759f7, 4 | BRF_GRA },           //  4 Foreground & Background Tiles
	{ "7.5b",		  0x20000, 0x2541d445, 4 | BRF_GRA },           //  5
	{ "8.6a",		  0x20000, 0x51a2f5c4, 4 | BRF_GRA },           //  6
	{ "9.6b",		  0x20000, 0xf4c8850f, 4 | BRF_GRA },           //  7
	{ "10.7a",		0x20000, 0x9e4b3c61, 4 | BRF_GRA },           //  8
	{ "11.7b",		0x20000, 0x0d33d083, 4 | BRF_GRA },           //  9
	{ "12.8a",		0x20000, 0x6c497ad3, 4 | BRF_GRA },           // 10
	{ "13.8b",		0x20000, 0x7a84adff, 4 | BRF_GRA },           // 11

	{ "21.8r",		0x20000, 0x0cee8711, 6 | BRF_GRA },           // 18 Sprites
	{ "20.8q",		0x20000, 0x9647e02a, 6 | BRF_GRA },           // 19
	{ "19.7r",		0x20000, 0x5082ceff, 6 | BRF_GRA },           // 16
	{ "18.7q",		0x20000, 0xd18a7ffb, 6 | BRF_GRA },           // 17
	{ "17.6r",		0x20000, 0x9088af09, 6 | BRF_GRA },           // 14
	{ "16.6q",		0x20000, 0x8ade4e01, 6 | BRF_GRA },           // 15
	{ "15.5r",		0x20000, 0x7429371c, 6 | BRF_GRA },           // 12
	{ "14.5q",		0x20000, 0x4301b97f, 6 | BRF_GRA },           // 13

	{ "3.3q",		  0x20000, 0x489c6d0e, 7 | BRF_SND },           // 20 MSM6295 Samples
	{ "2.3r",		  0x20000, 0x7710ce39, 7 | BRF_SND },           // 21
};

STD_ROM_PICK(drgnbowl)
STD_ROM_FN(drgnbowl)

static INT32 drgnbowlInit()
{
	game = 1;

	return DrvInit();
}

struct BurnDriver BurnDrvDrgnbowl = {
	"drgnbowl", NULL, NULL, NULL, "1992",
	"Dragon Bowl\0", NULL, "Nics", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_SCRFIGHT, 0,
	NULL, drgnbowlRomInfo, drgnbowlRomName, NULL, NULL, DrvInputInfo, DrgnbowlDIPInfo,
	drgnbowlInit, DrvExit, DrvFrame, DrgnbowlDraw, DrvScan, &DrvRecalc, 0x1000,
	256, 224, 4, 3
};

