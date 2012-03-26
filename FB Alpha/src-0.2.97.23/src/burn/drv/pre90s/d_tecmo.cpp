// FB Alpha Tecmo driver Module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "zet.h"
#include "burn_ym3812.h"
#include "msm5205.h"

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvJoy4[8];
static UINT8 DrvJoy5[8];
static UINT8 DrvJoy6[8];
static UINT8 DrvJoy11[8];
static UINT8 DrvReset;
static UINT8 DrvInputs[11];

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvSndROM;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvPalRAM;
static UINT8 *DrvTextRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvForeRAM;
static UINT8 *DrvBackRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT16 *DrvBgScroll;
static UINT16 *DrvFgScroll;

static INT32 tecmo_video_type;

static UINT32 adpcm_pos;
static UINT32 adpcm_end;
static UINT32 adpcm_size;
static INT32 adpcm_data;
static UINT8 DrvHasADPCM;

static UINT32 DrvZ80Bank;
static UINT8 soundlatch;
static UINT8 flipscreen;
static UINT8 DrvEnableNmi;

static struct BurnInputInfo RygarInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy5 + 3,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy5 + 2,	"p2 coin"  },

	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy5 + 1,	"p1 start" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 1, 	"p1 right" },
	{"P1 Down",	  BIT_DIGITAL  , DrvJoy1 + 2,   "p1 down", },
	{"P1 Up",	  BIT_DIGITAL  , DrvJoy1 + 3,   "p1 up",   },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 0,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 1,	"p1 fire 2"},

	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy5 + 0,	"p2 start" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 0, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 1, 	"p2 right" },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy3 + 2,   "p2 down", },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy3 + 3,   "p2 up",   },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy4 + 0,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy4 + 1,	"p2 fire 2"},

	{"Service",	  BIT_DIGITAL  , DrvJoy2 + 2,   "diag"     },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvInputs + 6,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvInputs + 7,	"dip"	   },
	{"Dip 3",	  BIT_DIPSWITCH, DrvInputs + 8,	"dip"	   },
	{"Dip 4",	  BIT_DIPSWITCH, DrvInputs + 9,	"dip"	   },
};

STDINPUTINFO(Rygar)

static struct BurnInputInfo BackfirtInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy6 + 2,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy6 + 3,	"p2 coin"  },

	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy6 + 1,	"p1 start" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 1, 	"p1 right" },
	{"P1 Down",	  BIT_DIGITAL  , DrvJoy1 + 2,   "p1 down", },
	{"P1 Up",	  BIT_DIGITAL  , DrvJoy1 + 3,   "p1 up",   },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 0,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 1,	"p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy2 + 2,	"p1 fire 3"},

	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy6 + 0,	"p2 start" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 0, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 1, 	"p2 right" },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy3 + 2,   "p2 down", },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy3 + 3,   "p2 up",   },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy4 + 0,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy4 + 1,	"p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy4 + 2,	"p2 fire 3"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvInputs + 6,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvInputs + 7,	"dip"	   },
	{"Dip 3",	  BIT_DIPSWITCH, DrvInputs + 8,	"dip"	   },
	{"Dip 4",	  BIT_DIPSWITCH, DrvInputs + 9,	"dip"	   },
};

STDINPUTINFO(Backfirt)

static struct BurnInputInfo GeminiInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy6 + 2,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy6 + 3,	"p2 coin"  },

	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy6 + 0,	"p1 start" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 1, 	"p1 right" },
	{"P1 Down",	  BIT_DIGITAL  , DrvJoy1 + 2,   "p1 down", },
	{"P1 Up",	  BIT_DIGITAL  , DrvJoy1 + 3,   "p1 up",   },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 1,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 0,	"p1 fire 2"},

	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy6 + 1,	"p2 start" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 0, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 1, 	"p2 right" },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy3 + 2,   "p2 down", },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy3 + 3,   "p2 up",   },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy4 + 1,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy4 + 0,	"p2 fire 2"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvInputs + 6,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvInputs + 7,	"dip"	   },
	{"Dip 3",	  BIT_DIPSWITCH, DrvInputs + 8,	"dip"	   },
	{"Dip 4",	  BIT_DIPSWITCH, DrvInputs + 9,	"dip"	   },
};

STDINPUTINFO(Gemini)

static struct BurnInputInfo SilkwormInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy11 + 2,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy11 + 3,	"p2 coin"  },

	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy11 + 0,	"p1 start" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 1, 	"p1 right" },
	{"P1 Down",	  BIT_DIGITAL  , DrvJoy1 + 2,   "p1 down", },
	{"P1 Up",	  BIT_DIGITAL  , DrvJoy1 + 3,   "p1 up",   },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 1,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 0,	"p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy2 + 2,	"p1 fire 3"},

	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy11 + 1,	"p2 start" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 0, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 1, 	"p2 right" },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy3 + 2,   "p2 down", },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy3 + 3,   "p2 up",   },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy4 + 1,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy4 + 0,	"p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy4 + 2,	"p2 fire 3"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvInputs + 6,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvInputs + 7,	"dip"	   },
	{"Dip 3",	  BIT_DIPSWITCH, DrvInputs + 8,	"dip"	   },
	{"Dip 4",	  BIT_DIPSWITCH, DrvInputs + 9,	"dip"	   },

};

STDINPUTINFO(Silkworm)

static struct BurnDIPInfo RygarDIPList[]=
{
	{0x12, 0xff, 0xff, 0x00, NULL },
	{0x13, 0xff, 0xff, 0x00, NULL },
	{0x14, 0xff, 0xff, 0x00, NULL },
	{0x15, 0xff, 0xff, 0x00, NULL },

	{0x12, 0xfe, 0,       4, "Coin A" },
	{0x12, 0x01, 0x03, 0x01, "2C 1C" },
	{0x12, 0x01, 0x03, 0x00, "1C 1C" },
	{0x12, 0x01, 0x03, 0x02, "1C 2C" },
	{0x12, 0x01, 0x03, 0x03, "1C 3C" },

	{0x12, 0xfe, 0,       4, "Coin B" },
	{0x12, 0x01, 0x0C, 0x04, "2C 1C" },
	{0x12, 0x01, 0x0C, 0x00, "1C 1C" },
	{0x12, 0x01, 0x0C, 0x08, "1C 2C" },
	{0x12, 0x01, 0x0C, 0x0C, "1C 3C" },

	{0x13, 0xfe, 0,       4, "Lives" },
	{0x13, 0x01, 0x03, 0x03, "2" },
	{0x13, 0x01, 0x03, 0x00, "3" },
	{0x13, 0x01, 0x03, 0x01, "4" },
	{0x13, 0x01, 0x03, 0x02, "5" },

	{0x13, 0xfe, 0,       2, "Cabinet" },
	{0x13, 0x01, 0x04, 0x04, "Upright" },
	{0x13, 0x01, 0x04, 0x00, "Cocktail" },

	{0x14, 0xfe, 0,       4, "Bonus Life" },
	{0x14, 0x01, 0x03, 0x00, "50000 200000 500000" },
	{0x14, 0x01, 0x03, 0x01, "100000 300000 600000" },
	{0x14, 0x01, 0x03, 0x02, "200000 500000" },
	{0x14, 0x01, 0x03, 0x03, "100000" },

	{0x15, 0xfe, 0,       4, "Difficulty" },
	{0x15, 0x01, 0x03, 0x00, "Easy" },
	{0x15, 0x01, 0x03, 0x01, "Normal" },
	{0x15, 0x01, 0x03, 0x02, "Hard" },
	{0x15, 0x01, 0x03, 0x03, "Hardest" },

	{0x15, 0xfe, 0,       2, "2P Can Start Anytime" },
	{0x15, 0x01, 0x04, 0x00, "No" },
	{0x15, 0x01, 0x04, 0x04, "Yes" },

	{0x15, 0xfe, 0,       2, "Allow Continue" },
	{0x15, 0x01, 0x08, 0x00, "No" },
	{0x15, 0x01, 0x08, 0x08, "Yes" },
};

STDDIPINFO(Rygar)

static struct BurnDIPInfo GeminiDIPList[]=
{
	{0x11, 0xff, 0xff, 0x00, NULL },
	{0x12, 0xff, 0xff, 0x00, NULL },
	{0x13, 0xff, 0xff, 0x00, NULL },
	{0x14, 0xff, 0xff, 0x00, NULL },

	{0x11, 0xfe, 0,       8, "Coin A" },
	{0x11, 0x01, 0x07, 0x06, "2C 1C" },
	{0x11, 0x01, 0x07, 0x00, "1C 1C" },
	{0x11, 0x01, 0x07, 0x07, "2C 3C" },
	{0x11, 0x01, 0x07, 0x01, "1C 2C" },
	{0x11, 0x01, 0x07, 0x02, "1C 3C" },
	{0x11, 0x01, 0x07, 0x03, "1C 4C" },
	{0x11, 0x01, 0x07, 0x04, "1C 5C" },
	{0x11, 0x01, 0x07, 0x05, "1C 6C" },

	{0x11, 0xfe, 0,       2, "Final Round Continuation" },
	{0x11, 0x01, 0x08, 0x00, "Round 6" },
	{0x11, 0x01, 0x08, 0x08, "Round 7" },

	{0x12, 0xfe, 0,       8, "Coin B" },
	{0x12, 0x01, 0x07, 0x06, "2C 1C" },
	{0x12, 0x01, 0x07, 0x00, "1C 1C" },
	{0x12, 0x01, 0x07, 0x07, "2C 3C" },
	{0x12, 0x01, 0x07, 0x01, "1C 2C" },
	{0x12, 0x01, 0x07, 0x02, "1C 3C" },
	{0x12, 0x01, 0x07, 0x03, "1C 4C" },
	{0x12, 0x01, 0x07, 0x04, "1C 5C" },
	{0x12, 0x01, 0x07, 0x05, "1C 6C" },

	{0x12, 0xfe, 0,       2, "Buy in During Final Round" },
	{0x12, 0x01, 0x08, 0x00, "No" },
	{0x12, 0x01, 0x08, 0x08, "Yes" },

	{0x13, 0xfe, 0,       4, "Lives" },
	{0x13, 0x01, 0x03, 0x03, "2" },
	{0x13, 0x01, 0x03, 0x00, "3" },
	{0x13, 0x01, 0x03, 0x01, "4" },
	{0x13, 0x01, 0x03, 0x02, "5" },

	{0x13, 0xfe, 0,       4, "Difficulty" },
	{0x13, 0x01, 0x0c, 0x00, "Easy" },
	{0x13, 0x01, 0x0c, 0x04, "Normal" },
	{0x13, 0x01, 0x0c, 0x08, "Hard" },
	{0x13, 0x01, 0x0c, 0x0c, "Hardest" },

	{0x14, 0xfe, 0,       8, "Bonus Life" },
	{0x14, 0x01, 0x07, 0x00, "50000 200000" },
	{0x14, 0x01, 0x07, 0x01, "50000 300000" },
	{0x14, 0x01, 0x07, 0x02, "100000 500000" },
	{0x14, 0x01, 0x07, 0x03, "50000" },
	{0x14, 0x01, 0x07, 0x04, "100000" },
	{0x14, 0x01, 0x07, 0x05, "200000" },
	{0x14, 0x01, 0x07, 0x06, "300000" },
	{0x14, 0x01, 0x07, 0x07, "None" },

	{0x14, 0xfe, 0,       2, "Demo Sounds" },
	{0x14, 0x01, 0x08, 0x08, "Off" },
	{0x14, 0x01, 0x08, 0x00, "On" },
};

STDDIPINFO(Gemini)

static struct BurnDIPInfo SilkwormDIPList[]=
{
	{0x13, 0xff, 0xff, 0x00, NULL },
	{0x14, 0xff, 0xff, 0x00, NULL },
	{0x15, 0xff, 0xff, 0x00, NULL },
	{0x16, 0xff, 0xff, 0x00, NULL },

	{0x13, 0xfe, 0,      4, "Coin A" },
	{0x13, 0x01, 0x3, 0x01, "2C 1C" },
	{0x13, 0x01, 0x3, 0x00, "1C 1C" },
	{0x13, 0x01, 0x3, 0x02, "1C 2C" },
	{0x13, 0x01, 0x3, 0x03, "1C 3C" },

	{0x13, 0xfe, 0,      4, "Coin B" },
	{0x13, 0x01, 0xC, 0x04, "2C 1C" },
	{0x13, 0x01, 0xC, 0x00, "1C 1C" },
	{0x13, 0x01, 0xC, 0x08, "1C 2C" },
	{0x13, 0x01, 0xC, 0x0C, "1C 3C" },

	{0x14, 0xfe, 0,      4 , "Lives" },
	{0x14, 0x01, 0x3, 0x03, "2" },
	{0x14, 0x01, 0x3, 0x00, "3" },
	{0x14, 0x01, 0x3, 0x01, "4" },
	{0x14, 0x01, 0x3, 0x02, "5" },

	{0x14, 0xfe, 0,      2 , "Demo Sounds" },
	{0x14, 0x01, 0x8, 0x00, "Off" },
	{0x14, 0x01, 0x8, 0x08, "On" },

	{0x15, 0xfe, 0,      8, "Bonus Life" },
	{0x15, 0x01, 0x7, 0x00, "50000 200000 500000" },
	{0x15, 0x01, 0x7, 0x01, "100000 300000 800000" },
	{0x15, 0x01, 0x7, 0x02, "50000 200000" },
	{0x15, 0x01, 0x7, 0x03, "100000 300000" },
	{0x15, 0x01, 0x7, 0x04, "50000" },
	{0x15, 0x01, 0x7, 0x05, "100000" },
	{0x15, 0x01, 0x7, 0x06, "200000" },
	{0x15, 0x01, 0x7, 0x07, "None" },

	{0x16, 0xfe, 0,      6, "Difficulty" },
	{0x16, 0x01, 0x7, 0x00, "0" },
	{0x16, 0x01, 0x7, 0x01, "1" },
	{0x16, 0x01, 0x7, 0x02, "2" },
	{0x16, 0x01, 0x7, 0x03, "3" },
	{0x16, 0x01, 0x7, 0x04, "4" },
	{0x16, 0x01, 0x7, 0x05, "5" },

	{0x16, 0xfe, 0,      2, "Allow Continue" },
	{0x16, 0x01, 0x8, 0x08, "No" },
	{0x16, 0x01, 0x8, 0x00, "Yes" },
};

STDDIPINFO(Silkworm)

static struct BurnDIPInfo BackfirtDIPList[]=
{
	{0x13, 0xff, 0xff, 0x00, NULL },
	{0x14, 0xff, 0xff, 0x10, NULL },
	{0x15, 0xff, 0xff, 0x00, NULL },
	{0x16, 0xff, 0xff, 0x00, NULL },

	{0x13, 0xfe, 0,      4, "Coin A" },
	{0x13, 0x01, 0x3, 0x00, "1C 1C" },
	{0x13, 0x01, 0x3, 0x01, "1C 2C" },
	{0x13, 0x01, 0x3, 0x02, "1C 3C" },
	{0x13, 0x01, 0x3, 0x03, "1C 6C" },

	{0x13, 0xfe, 0,      4, "Coin B" },
	{0x13, 0x01, 0xC, 0x04, "2C 1C" },
	{0x13, 0x01, 0xC, 0x00, "1C 1C" },
	{0x13, 0x01, 0xC, 0x08, "1C 2C" },
	{0x13, 0x01, 0xC, 0x0C, "1C 3C" },

	{0x14, 0xfe, 0,       2, "Cabinet" },
	{0x14, 0x01, 0x10, 0x10, "Upright" },
	{0x14, 0x01, 0x10, 0x00, "Cocktail" },

	{0x15, 0xfe, 0,      8, "Bonus Life" },
	{0x15, 0x01, 0x07, 0x00, "50000  200000 500000" },
	{0x15, 0x01, 0x07, 0x01, "100000 300000 800000" },
	{0x15, 0x01, 0x07, 0x02, "50000  200000" },
	{0x15, 0x01, 0x07, 0x03, "100000 300000" },
	{0x15, 0x01, 0x07, 0x04, "50000" },
	{0x15, 0x01, 0x07, 0x05, "100000" },
	{0x15, 0x01, 0x07, 0x06, "200000" },
	{0x15, 0x01, 0x07, 0x07, "None" },

	// not verified
	{0x15, 0xfe, 0,       6, "Difficulty" },
	{0x15, 0x01, 0x38, 0x00, "0" },
	{0x15, 0x01, 0x38, 0x08, "1" },
	{0x15, 0x01, 0x38, 0x10, "2" },
	{0x15, 0x01, 0x38, 0x18, "3" },
	{0x15, 0x01, 0x38, 0x20, "4" },
	{0x15, 0x01, 0x38, 0x28, "5" },

	{0x16, 0xfe, 0,      2, "Allow Continue" },
	{0x16, 0x01, 0x04, 0x04, "No" },
	{0x16, 0x01, 0x04, 0x00, "Yes" },

	{0x16, 0xfe, 0,      2, "Invincibility (Cheat)" },
	{0x16, 0x01, 0x08, 0x08, "No" },
	{0x16, 0x01, 0x08, 0x00, "Yes" },
};

STDDIPINFO(Backfirt)

UINT8 __fastcall rygar_main_read(UINT16 address)
{
	switch (address)
	{
		case 0xf800:
		case 0xf801:
		case 0xf802:
		case 0xf803:
		case 0xf804:
		case 0xf805:
		case 0xf806:
		case 0xf807:
		case 0xf808:
		case 0xf809:
			return DrvInputs[address & 0x0f];

		case 0xf80f:
			return DrvInputs[10];
	}

	return 0;
}

static void bankswitch_w(INT32 data)
{
	DrvZ80Bank = 0x10000 + ((data & 0xf8) << 8);

	ZetMapArea(0xf000, 0xf7ff, 0, DrvZ80ROM0 + DrvZ80Bank);
	ZetMapArea(0xf000, 0xf7ff, 2, DrvZ80ROM0 + DrvZ80Bank);
}

static inline void palette_write(INT32 offset)
{
	UINT16 data;
	UINT8 r,g,b;

	data = *((UINT16*)(DrvPalRAM + (offset & ~1)));
	data = (data << 8) | (data >> 8);

	r = (data >> 4) & 0x0f;
	g = (data >> 0) & 0x0f;
	b = (data >> 8) & 0x0f;

	r |= r << 4;
	g |= g << 4;
	b |= b << 4;

	DrvPalette[offset >> 1] = BurnHighCol(r, g, b, 0);
}

void __fastcall rygar_main_write(UINT16 address, UINT8 data)
{
	if ((address & 0xf000) == 0xe000) {
		DrvPalRAM[address & 0x7ff] = data;
		palette_write(address & 0x7ff);
		return;
	}

	switch (address)
	{
		case 0xf800:
			DrvFgScroll[0] = (DrvFgScroll[0] & 0xff00) | data;
		return;

		case 0xf801:
			DrvFgScroll[0] = (DrvFgScroll[0] & 0x00ff) | (data << 8);
		return;

		case 0xf802:
			DrvFgScroll[1] = data;
		return;

		case 0xf803:
			DrvBgScroll[0] = (DrvBgScroll[0] & 0xff00) | data;
		return;

		case 0xf804:
			DrvBgScroll[0] = (DrvBgScroll[0] & 0x00ff) | (data << 8);
		return;

		case 0xf805:
			DrvBgScroll[1] = data;
		return;

		case 0xf806:
			soundlatch = data;
			DrvEnableNmi = 1;
		return;

		case 0xf807:
			flipscreen = data & 1;
		return;

		case 0xf808:
			bankswitch_w(data);
		return;

		case 0xf80b:
			// watchdog reset
		return;
	}

	return;
}

UINT8 __fastcall rygar_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xc000:
			return soundlatch;
	}

	return 0;
}

void __fastcall rygar_sound_write(UINT16 address, UINT8 data)
{
	if ((address & 0xff80) == 0x2000) {
		DrvZ80ROM1[address] = data;
		return;
	}

	switch (address)
	{
		case 0x8000:
		case 0xa000:
			BurnYM3812Write(0, data);
		return;

		case 0x8001:
		case 0xa001:
			BurnYM3812Write(1, data);
		return;

		case 0xc000:
			if (DrvHasADPCM) {
				adpcm_pos = data << 8;
				MSM5205ResetWrite(0, 0);
			}
		return;

		case 0xc400:
		case 0xd000:
			adpcm_end = (data + 1) << 8;
		return;

		case 0xc800:
		case 0xe000:
			if (DrvHasADPCM) MSM5205SetVolume(0, (data & 0x0f) * 100 / 15);
		return;

		case 0xf000:
		return;
	}

	return;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x20000;
	DrvZ80ROM1	= Next; Next += 0x08000;

	DrvSndROM	= Next; Next += adpcm_size;

	DrvGfxROM0	= Next; Next += 0x10000;
	DrvGfxROM1	= Next; Next += 0x80000;
	DrvGfxROM2	= Next; Next += 0x80000;
	DrvGfxROM3	= Next; Next += 0x80000;

	AllRam		= Next;

	DrvZ80RAM0	= Next; Next += 0x01000;
	DrvZ80RAM1	= Next; Next += 0x00800;

	DrvPalRAM	= Next; Next += 0x00800;
	DrvTextRAM	= Next; Next += 0x00800;
	DrvBackRAM	= Next; Next += 0x00400;
	DrvForeRAM	= Next; Next += 0x00400;
	DrvSprRAM	= Next; Next += 0x00800;

	DrvBgScroll	= (UINT16*)Next; Next += 0x00002 * sizeof(UINT16);
	DrvFgScroll	= (UINT16*)Next; Next += 0x00002 * sizeof(UINT16);

	DrvPalette	= (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);

	RamEnd		= Next;
	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x40000);
	if (tmp == NULL) {
		return 1;
	}

	static INT32 Planes[4] = {
		0x000, 0x001, 0x002, 0x003
	};

	static INT32 XOffs[16] = {
		0x000, 0x004, 0x008, 0x00c, 0x010, 0x014, 0x018, 0x01c,
		0x100, 0x104, 0x108, 0x10c, 0x110, 0x114, 0x118, 0x11c
	};

	static INT32 YOffs[16] = {
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
		0x200, 0x220, 0x240, 0x260, 0x280, 0x2a0, 0x2c0, 0x2e0
	};

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0400, 4,  8,  8, Planes, XOffs, YOffs, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x40000);

	GfxDecode(0x2000, 4,  8,  8, Planes, XOffs, YOffs, 0x100, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x40000);

	GfxDecode(0x0800, 4, 16, 16, Planes, XOffs, YOffs, 0x400, tmp, DrvGfxROM2);

	memcpy (tmp, DrvGfxROM3, 0x40000);

	GfxDecode(0x0800, 4, 16, 16, Planes, XOffs, YOffs, 0x400, tmp, DrvGfxROM3);

	BurnFree (tmp);

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	bankswitch_w(0);
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	if (DrvHasADPCM) MSM5205Reset();
	BurnYM3812Reset();

	if (tecmo_video_type) {
		memset (DrvZ80ROM1 + 0x2000, 0, 0x80);
	}

	soundlatch = 0;
	flipscreen = 0;

	adpcm_pos = 0;
	adpcm_end = 0;
	adpcm_data = -1;

	return 0;
}

static void TecmoFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 TecmoSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static void TecmoMSM5205Vck()
{
	if (adpcm_pos >= adpcm_end || adpcm_pos >= adpcm_size) {
		MSM5205ResetWrite(0, 1);
	} else {
		if (adpcm_data != -1) {
			MSM5205DataWrite(0, adpcm_data & 0x0f);
			adpcm_data = -1;
		} else {
			adpcm_data = DrvSndROM[adpcm_pos++ & (adpcm_size - 1)];
			MSM5205DataWrite(0, adpcm_data >> 4);
		}
	}
}

static INT32 RygarInit()
{
	tecmo_video_type = 0;
	DrvHasADPCM = 1;
	adpcm_size = 0x4000;

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xcfff, 0, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xcfff, 1, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xcfff, 2, DrvZ80RAM0);
	ZetMapArea(0xd000, 0xd7ff, 0, DrvTextRAM);
	ZetMapArea(0xd000, 0xd7ff, 1, DrvTextRAM);
	ZetMapArea(0xd800, 0xdbff, 0, DrvForeRAM);
	ZetMapArea(0xd800, 0xdbff, 1, DrvForeRAM);
	ZetMapArea(0xdc00, 0xdfff, 0, DrvBackRAM);
	ZetMapArea(0xdc00, 0xdfff, 1, DrvBackRAM);
	ZetMapArea(0xe000, 0xe7ff, 0, DrvSprRAM);
	ZetMapArea(0xe000, 0xe7ff, 1, DrvSprRAM);
	ZetMapArea(0xe800, 0xefff, 0, DrvPalRAM);
	ZetSetWriteHandler(rygar_main_write);
	ZetSetReadHandler(rygar_main_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80ROM1);
	ZetMapArea(0x4000, 0x47ff, 0, DrvZ80RAM1);
	ZetMapArea(0x4000, 0x47ff, 1, DrvZ80RAM1);
	ZetMapArea(0x4000, 0x47ff, 2, DrvZ80RAM1);
	ZetSetWriteHandler(rygar_sound_write);
	ZetSetReadHandler(rygar_sound_read);
	ZetMemEnd();
	ZetClose();

	{
		for (INT32 i = 0; i < 3; i++) {
			if (BurnLoadRom(DrvZ80ROM0 + i * 0x8000, i +  0, 1)) return 1;
		}

		if (BurnLoadRom(DrvZ80ROM1,	3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0,	4, 1)) return 1;

		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(DrvGfxROM1 + i * 0x8000, i +  5, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + i * 0x8000, i +  9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM3 + i * 0x8000, i + 13, 1)) return 1;
		}

		if (BurnLoadRom(DrvSndROM,	17, 1)) return 1;

		DrvGfxDecode();
	}

	BurnYM3812Init(4000000, &TecmoFMIRQHandler, &TecmoSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(4000000);

	MSM5205Init(0, TecmoSynchroniseStream, 400000, TecmoMSM5205Vck, MSM5205_S48_4B, 100, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}


static INT32 SilkwormInit()
{
	tecmo_video_type = 1;
	DrvHasADPCM = 1;
	adpcm_size = 0x8000;

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xc3ff, 0, DrvBackRAM);
	ZetMapArea(0xc000, 0xc3ff, 1, DrvBackRAM);
	ZetMapArea(0xc400, 0xc7ff, 0, DrvForeRAM);
	ZetMapArea(0xc400, 0xc7ff, 1, DrvForeRAM);
	ZetMapArea(0xc800, 0xcfff, 0, DrvTextRAM);
	ZetMapArea(0xc800, 0xcfff, 1, DrvTextRAM);
	ZetMapArea(0xd000, 0xdfff, 0, DrvZ80RAM0);
	ZetMapArea(0xd000, 0xdfff, 1, DrvZ80RAM0);
	ZetMapArea(0xd000, 0xdfff, 2, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xe7ff, 0, DrvSprRAM);
	ZetMapArea(0xe000, 0xe7ff, 1, DrvSprRAM);
	ZetMapArea(0xe800, 0xefff, 0, DrvPalRAM);
	ZetSetWriteHandler(rygar_main_write);
	ZetSetReadHandler(rygar_main_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM1);
	ZetSetWriteHandler(rygar_sound_write);
	ZetSetReadHandler(rygar_sound_read);
	ZetMemEnd();
	ZetClose();

	{
		for (INT32 i = 0; i < 2; i++) {
			if (BurnLoadRom(DrvZ80ROM0 + i * 0x10000, i +  0, 1)) return 1;
		}

		if (BurnLoadRom(DrvZ80ROM1,	2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0,	3, 1)) return 1;

		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(DrvGfxROM1 + i * 0x10000, i +  4, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + i * 0x10000, i +  8, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM3 + i * 0x10000, i + 12, 1)) return 1;
		}

		if (BurnLoadRom(DrvSndROM,	16, 1)) return 1;

		DrvGfxDecode();
	}

	BurnYM3812Init(4000000, &TecmoFMIRQHandler, &TecmoSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(4000000);

	MSM5205Init(0, TecmoSynchroniseStream, 400000, TecmoMSM5205Vck, MSM5205_S48_4B, 100, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 GeminiInit()
{
	tecmo_video_type = 2;
	DrvHasADPCM = 1;
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "backfirt")) DrvHasADPCM = 0;
	if (DrvHasADPCM) adpcm_size = 0x8000;

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xcfff, 0, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xcfff, 1, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xcfff, 2, DrvZ80RAM0);
	ZetMapArea(0xd000, 0xd7ff, 0, DrvTextRAM);
	ZetMapArea(0xd000, 0xd7ff, 1, DrvTextRAM);
	ZetMapArea(0xd800, 0xdbff, 0, DrvForeRAM);
	ZetMapArea(0xd800, 0xdbff, 1, DrvForeRAM);
	ZetMapArea(0xdc00, 0xdfff, 0, DrvBackRAM);
	ZetMapArea(0xdc00, 0xdfff, 1, DrvBackRAM);
	ZetMapArea(0xe000, 0xe7ff, 0, DrvPalRAM);
	ZetMapArea(0xe800, 0xefff, 0, DrvSprRAM);
	ZetMapArea(0xe800, 0xefff, 1, DrvSprRAM);
	ZetSetWriteHandler(rygar_main_write);
	ZetSetReadHandler(rygar_main_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM1);
	ZetSetWriteHandler(rygar_sound_write);
	ZetSetReadHandler(rygar_sound_read);
	ZetMemEnd();
	ZetClose();

	{
		for (INT32 i = 0; i < 2; i++) {
			if (BurnLoadRom(DrvZ80ROM0 + i * 0x10000, i +  0, 1)) return 1;
		}

		if (BurnLoadRom(DrvZ80ROM1,	2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0,	3, 1)) return 1;

		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(DrvGfxROM1 + i * 0x10000, i +  4, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + i * 0x10000, i +  8, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM3 + i * 0x10000, i + 12, 1)) return 1;
		}

		BurnLoadRom(DrvSndROM,	16, 1);

		DrvGfxDecode();
	}

	BurnYM3812Init(4000000, &TecmoFMIRQHandler, &TecmoSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(4000000);

	if (DrvHasADPCM) MSM5205Init(0, TecmoSynchroniseStream, 400000, TecmoMSM5205Vck, MSM5205_S48_4B, 100, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	if (DrvHasADPCM) MSM5205Exit();
	BurnYM3812Exit();

	GenericTilesExit();

	ZetExit();

	BurnFree (AllMem);
	
	DrvHasADPCM = 0;

	return 0;
}

static inline INT32 calc_sprite_offset(INT32 code, INT32 x, INT32 y)
{
	INT32 ofst = 0;
	if (x & 0x001) ofst |= 0x01;
	if (y & 0x001) ofst |= 0x02;
	if (x & 0x002) ofst |= 0x04;
	if (y & 0x002) ofst |= 0x08;
	if (x & 0x004) ofst |= 0x10;
	if (y & 0x004) ofst |= 0x20;

	return (ofst + code) & 0x1fff;
}

static void draw_sprites(INT32 priority)
{
	for (INT32 offs = 0; offs < 0x800; offs += 8)
	{
		INT32 flags = DrvSprRAM[offs+3];
		if (priority != (flags >> 6)) continue;

		INT32 bank = DrvSprRAM[offs+0];

		if (bank & 4)
		{
			INT32 which = DrvSprRAM[offs+1];
			INT32 code, xpos, ypos, flipx, flipy, x, y;
			INT32 size = DrvSprRAM[offs + 2] & 3;

			if (tecmo_video_type)
				code = which + ((bank & 0xf8) << 5);
			else
				code = which + ((bank & 0xf0) << 4);

			code &= ~((1 << (size << 1)) - 1);
			size = 1 << size;

			xpos = DrvSprRAM[offs + 5] - ((flags & 0x10) << 4);
			ypos = DrvSprRAM[offs + 4] - ((flags & 0x20) << 3);
			flipx = bank & 1;
			flipy = bank & 2;

			for (y = 0; y < size; y++)
			{
				for (x = 0; x < size; x++)
				{
					INT32 sx = xpos + ((flipx ? (size - 1 - x) : x) << 3);
					INT32 sy = ypos + ((flipy ? (size - 1 - y) : y) << 3);
					    sy -= 16;

					if (sy < -7 || sx < -7 || sx > 255 || sy > 223) continue;

					if (flipy) {
						if (flipx) {
							Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, calc_sprite_offset(code, x, y), sx, sy, flags & 0x0f, 4, 0, 0, DrvGfxROM1);
						} else {
							Render8x8Tile_Mask_FlipY_Clip(pTransDraw, calc_sprite_offset(code, x, y), sx, sy, flags & 0x0f, 4, 0, 0, DrvGfxROM1);
						}
					} else {
						if (flipx) {
							Render8x8Tile_Mask_FlipX_Clip(pTransDraw, calc_sprite_offset(code, x, y), sx, sy, flags & 0x0f, 4, 0, 0, DrvGfxROM1);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, calc_sprite_offset(code, x, y), sx, sy, flags & 0x0f, 4, 0, 0, DrvGfxROM1);
						}
					}
				}
			}
		}
	}
}
 
static INT32 draw_layer(UINT8 *vidram, UINT8 *gfx_base, INT32 paloffs, UINT16 *scroll)
{
	for (INT32 offs = 0; offs < 32 * 16; offs++)
	{
		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		    sx -= scroll[0] & 0x1ff;

		    if (flipscreen) {
			sx += 48 + 256;
		    } else {
			sx -= 48;
		    }

		if (sx <   -15) sx += 0x200;
		if (sx >   511) sx -= 0x200;
		    sy -= scroll[1] + 16;
		if (sy <   -15) sy += 0x100;

		if (sx > nScreenWidth || sy > nScreenHeight) continue;

		UINT8 color = vidram[0x200 | offs];
		INT32 code  = vidram[offs];

		if (tecmo_video_type == 2) {
			color = (color << 4) | (color >> 4);
		}

		    code  |= ((color & 7) << 8);
		    color >>= 4;

		if (sx < 0 || sy < 0 || sx > nScreenWidth - 16 || sy > nScreenHeight - 16) {
			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, paloffs, gfx_base);
		} else {
			Render16x16Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, paloffs, gfx_base);
		}
	}

	return 0;
}

static void draw_text_layer()
{
	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		INT32 color = DrvTextRAM[offs | 0x400];

		INT32 code = DrvTextRAM[offs] | ((color & 3) << 8);

		color >>= 4;

		if (sy < 16 || sy > 239) continue;

		Render8x8Tile_Mask(pTransDraw, code, sx, sy-16, color, 4, 0, 0x100, DrvGfxROM0);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x800; i+=2) {
			palette_write(i);
		}
		DrvRecalc = 0;
	}

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x100;
	}
 
	draw_sprites(3);

	draw_layer(DrvBackRAM, DrvGfxROM3, 0x300, DrvBgScroll);

	draw_sprites(2);

	draw_layer(DrvForeRAM, DrvGfxROM2, 0x200, DrvFgScroll);

	draw_sprites(1);
	draw_sprites(0);

	draw_text_layer();

	if (flipscreen) {
		INT32 nSize = (nScreenWidth * nScreenHeight) - 1;
		for (INT32 i = 0; i < nSize >> 1; i++) {
			INT32 n = pTransDraw[i];
			pTransDraw[i] = pTransDraw[nSize - i];
			pTransDraw[nSize - i] = n;
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
		memset (DrvInputs, 0, 6);
		DrvInputs[10] = 0;

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[ 0] ^= DrvJoy1[i] << i;
			DrvInputs[ 1] ^= DrvJoy2[i] << i;
			DrvInputs[ 2] ^= DrvJoy3[i] << i;
			DrvInputs[ 3] ^= DrvJoy4[i] << i;
			DrvInputs[ 4] ^= DrvJoy5[i] << i;
			DrvInputs[ 5] ^= DrvJoy6[i] << i;
			DrvInputs[10] ^= DrvJoy11[i] << i;
		}
	}

	ZetNewFrame();

	INT32 nSegment;
	INT32 nInterleave = 10;
	if (DrvHasADPCM) nInterleave = MSM5205CalcInterleave(0, 4000000);
	INT32 nTotalCycles[2] = { 6000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nSegment = (nTotalCycles[0] - nCyclesDone[0]) / (nInterleave - i);

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nSegment);
		if (i == (nInterleave-1)) ZetRaiseIrq(0);
		ZetClose();

		ZetOpen(1);
		if (DrvEnableNmi) {
			ZetNmi();
			DrvEnableNmi = 0;
		}
		BurnTimerUpdateYM3812(i * (nTotalCycles[1] / nInterleave));
		if (DrvHasADPCM)  MSM5205Update();
		ZetClose();
	}

	ZetOpen(1);
	if (pBurnSoundOut) {
		BurnTimerEndFrameYM3812(nTotalCycles[1]);
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		if (DrvHasADPCM) MSM5205Render(0, pBurnSoundOut, nBurnSoundLen);
	}
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
		*pnMin = 0x029622;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ba.Data   = DrvZ80ROM1 + 0x2000;
		ba.nLen	  = 0x80;
		ba.szName = "Sound Z80 RAM";
		BurnAcb(&ba);

		ZetScan(nAction);
		BurnYM3812Scan(nAction, pnMin);
		if (DrvHasADPCM) MSM5205Scan(nAction, pnMin);

		SCAN_VAR(flipscreen);
		SCAN_VAR(soundlatch);
		SCAN_VAR(DrvZ80Bank);

		SCAN_VAR(adpcm_pos);
		SCAN_VAR(adpcm_end);
	}

	ZetOpen(0);
	ZetMapArea(0xf000, 0xf7ff, 0, DrvZ80ROM0 + DrvZ80Bank);
	ZetMapArea(0xf000, 0xf7ff, 2, DrvZ80ROM0 + DrvZ80Bank);
	ZetClose();

	return 0;
}


// Rygar (US set 1)

static struct BurnRomInfo rygarRomDesc[] = {
	{ "5.5p",      	0x08000, 0x062cd55d, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 Code
	{ "cpu_5m.bin",	0x04000, 0x7ac5191b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "cpu_5j.bin",	0x08000, 0xed76d606, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "cpu_4h.bin",	0x02000, 0xe4a2fa87, 2 | BRF_PRG | BRF_ESS }, //  3 - Z80 Code

	{ "cpu_8k.bin",	0x08000, 0x4d482fb6, 3 | BRF_GRA },	      //  4 - Characters

	{ "vid_6k.bin",	0x08000, 0xaba6db9e, 4 | BRF_GRA },	      //  5 - Sprites
	{ "vid_6j.bin",	0x08000, 0xae1f2ed6, 4 | BRF_GRA },	      //  6
	{ "vid_6h.bin",	0x08000, 0x46d9e7df, 4 | BRF_GRA },	      //  7
	{ "vid_6g.bin",	0x08000, 0x45839c9a, 4 | BRF_GRA },	      //  8

	{ "vid_6p.bin",	0x08000, 0x9eae5f8e, 5 | BRF_GRA },	      //  9 - Foreground Tiles
	{ "vid_6o.bin",	0x08000, 0x5a10a396, 5 | BRF_GRA },	      // 10
	{ "vid_6n.bin",	0x08000, 0x7b12cf3f, 5 | BRF_GRA },	      // 11
	{ "vid_6l.bin",	0x08000, 0x3cea7eaa, 5 | BRF_GRA },	      // 12

	{ "vid_6f.bin",	0x08000, 0x9840edd8, 6 | BRF_GRA },	      // 13 - Background Tiles
	{ "vid_6e.bin",	0x08000, 0xff65e074, 6 | BRF_GRA },	      // 14 
	{ "vid_6c.bin",	0x08000, 0x89868c85, 6 | BRF_GRA },	      // 15 
	{ "vid_6b.bin",	0x08000, 0x35389a7b, 6 | BRF_GRA },	      // 16 

	{ "cpu_1f.bin",	0x04000, 0x3cc98c5a, 7 | BRF_SND },	      // 17 - Samples
};

STD_ROM_PICK(rygar)
STD_ROM_FN(rygar)

struct BurnDriver BurnDrvRygar = {
	"rygar", NULL, NULL, NULL, "1986",
	"Rygar (US set 1)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, rygarRomInfo, rygarRomName, NULL, NULL, RygarInputInfo, RygarDIPInfo,
	RygarInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};


// Rygar (US set 2)

static struct BurnRomInfo rygar2RomDesc[] = {
	{ "5p.bin",     0x08000, 0x151ffc0b, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 Code
	{ "cpu_5m.bin",	0x04000, 0x7ac5191b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "cpu_5j.bin",	0x08000, 0xed76d606, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "cpu_4h.bin",	0x02000, 0xe4a2fa87, 2 | BRF_PRG | BRF_ESS }, //  3 - Z80 Code

	{ "cpu_8k.bin",	0x08000, 0x4d482fb6, 3 | BRF_GRA },	      //  4 - Characters

	{ "vid_6k.bin",	0x08000, 0xaba6db9e, 4 | BRF_GRA },	      //  5 - Sprites
	{ "vid_6j.bin",	0x08000, 0xae1f2ed6, 4 | BRF_GRA },	      //  6
	{ "vid_6h.bin",	0x08000, 0x46d9e7df, 4 | BRF_GRA },	      //  7
	{ "vid_6g.bin",	0x08000, 0x45839c9a, 4 | BRF_GRA },	      //  8

	{ "vid_6p.bin",	0x08000, 0x9eae5f8e, 5 | BRF_GRA },	      //  9 - Foreground Tiles
	{ "vid_6o.bin",	0x08000, 0x5a10a396, 5 | BRF_GRA },	      // 10
	{ "vid_6n.bin",	0x08000, 0x7b12cf3f, 5 | BRF_GRA },	      // 11
	{ "vid_6l.bin",	0x08000, 0x3cea7eaa, 5 | BRF_GRA },	      // 12

	{ "vid_6f.bin",	0x08000, 0x9840edd8, 6 | BRF_GRA },	      // 13 - Background Tiles
	{ "vid_6e.bin",	0x08000, 0xff65e074, 6 | BRF_GRA },	      // 14 
	{ "vid_6c.bin",	0x08000, 0x89868c85, 6 | BRF_GRA },	      // 15 
	{ "vid_6b.bin",	0x08000, 0x35389a7b, 6 | BRF_GRA },	      // 16 

	{ "cpu_1f.bin",	0x04000, 0x3cc98c5a, 7 | BRF_SND },	      // 17 - Samples
};

STD_ROM_PICK(rygar2)
STD_ROM_FN(rygar2)

struct BurnDriver BurnDrvRygar2 = {
	"rygar2", "rygar", NULL, NULL, "1986",
	"Rygar (US set 2)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, rygar2RomInfo, rygar2RomName, NULL, NULL, RygarInputInfo, RygarDIPInfo,
	RygarInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};


// Rygar (US set 3, old version)

static struct BurnRomInfo rygar3RomDesc[] = {
	{ "cpu_5p.bin", 0x08000, 0xe79c054a, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 Code
	{ "cpu_5m.bin",	0x04000, 0x7ac5191b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "cpu_5j.bin",	0x08000, 0xed76d606, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "cpu_4h.bin",	0x02000, 0xe4a2fa87, 2 | BRF_PRG | BRF_ESS }, //  3 - Z80 Code

	{ "cpu_8k.bin",	0x08000, 0x4d482fb6, 3 | BRF_GRA },	      //  4 - Characters

	{ "vid_6k.bin",	0x08000, 0xaba6db9e, 4 | BRF_GRA },	      //  5 - Sprites
	{ "vid_6j.bin",	0x08000, 0xae1f2ed6, 4 | BRF_GRA },	      //  6
	{ "vid_6h.bin",	0x08000, 0x46d9e7df, 4 | BRF_GRA },	      //  7
	{ "vid_6g.bin",	0x08000, 0x45839c9a, 4 | BRF_GRA },	      //  8

	{ "vid_6p.bin",	0x08000, 0x9eae5f8e, 5 | BRF_GRA },	      //  9 - Foreground Tiles
	{ "vid_6o.bin",	0x08000, 0x5a10a396, 5 | BRF_GRA },	      // 10
	{ "vid_6n.bin",	0x08000, 0x7b12cf3f, 5 | BRF_GRA },	      // 11
	{ "vid_6l.bin",	0x08000, 0x3cea7eaa, 5 | BRF_GRA },	      // 12

	{ "vid_6f.bin",	0x08000, 0x9840edd8, 6 | BRF_GRA },	      // 13 - Background Tiles
	{ "vid_6e.bin",	0x08000, 0xff65e074, 6 | BRF_GRA },	      // 14 
	{ "vid_6c.bin",	0x08000, 0x89868c85, 6 | BRF_GRA },	      // 15 
	{ "vid_6b.bin",	0x08000, 0x35389a7b, 6 | BRF_GRA },	      // 16 

	{ "cpu_1f.bin",	0x04000, 0x3cc98c5a, 7 | BRF_SND },	      // 17 - Samples
};

STD_ROM_PICK(rygar3)
STD_ROM_FN(rygar3)

struct BurnDriver BurnDrvRygar3 = {
	"rygar3", "rygar", NULL, NULL, "1986",
	"Rygar (US set 3, old version)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, rygar3RomInfo, rygar3RomName, NULL, NULL, RygarInputInfo, RygarDIPInfo,
	RygarInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};


// Argus no Senshi (Japan)

static struct BurnRomInfo rygarjRomDesc[] = {
	{ "cpuj_5p.bin",0x08000, 0xb39698ba, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 Code
	{ "cpuj_5m.bin",0x04000, 0x3f180979, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "cpuj_5j.bin",0x08000, 0x69e44e8f, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "cpu_4h.bin",	0x02000, 0xe4a2fa87, 2 | BRF_PRG | BRF_ESS }, //  3 - Z80 Code

	{ "cpuj_8k.bin",0x08000, 0x45047707, 3 | BRF_GRA },	      //  4 - Characters

	{ "vid_6k.bin",	0x08000, 0xaba6db9e, 4 | BRF_GRA },	      //  5 - Sprites
	{ "vid_6j.bin",	0x08000, 0xae1f2ed6, 4 | BRF_GRA },	      //  6
	{ "vid_6h.bin",	0x08000, 0x46d9e7df, 4 | BRF_GRA },	      //  7
	{ "vid_6g.bin",	0x08000, 0x45839c9a, 4 | BRF_GRA },	      //  8

	{ "vid_6p.bin",	0x08000, 0x9eae5f8e, 5 | BRF_GRA },	      //  9 - Foreground Tiles
	{ "vid_6o.bin",	0x08000, 0x5a10a396, 5 | BRF_GRA },	      // 10
	{ "vid_6n.bin",	0x08000, 0x7b12cf3f, 5 | BRF_GRA },	      // 11
	{ "vid_6l.bin",	0x08000, 0x3cea7eaa, 5 | BRF_GRA },	      // 12

	{ "vid_6f.bin",	0x08000, 0x9840edd8, 6 | BRF_GRA },	      // 13 - Background Tiles
	{ "vid_6e.bin",	0x08000, 0xff65e074, 6 | BRF_GRA },	      // 14 
	{ "vid_6c.bin",	0x08000, 0x89868c85, 6 | BRF_GRA },	      // 15 
	{ "vid_6b.bin",	0x08000, 0x35389a7b, 6 | BRF_GRA },	      // 16 

	{ "cpu_1f.bin",	0x04000, 0x3cc98c5a, 7 | BRF_SND },	      // 17 - Samples
};

STD_ROM_PICK(rygarj)
STD_ROM_FN(rygarj)

struct BurnDriver BurnDrvRygarj = {
	"rygarj", "rygar", NULL, NULL, "1986",
	"Argus no Senshi (Japan)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, rygarjRomInfo, rygarjRomName, NULL, NULL, RygarInputInfo, RygarDIPInfo,
	RygarInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};


// Silk Worm (set 1)

static struct BurnRomInfo silkwormRomDesc[] = {
	{ "silkworm.4",		0x10000, 0xa5277cce, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 Code
	{ "silkworm.5",		0x10000, 0xa6c7bb51, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "silkworm.3",		0x08000, 0xb589f587, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "silkworm.2",		0x08000, 0xe80a1cd9, 3 | BRF_GRA },	      //  3 - Characters

	{ "silkworm.6",		0x10000, 0x1138d159, 4 | BRF_GRA },	      //  4 - Sprites
	{ "silkworm.7",		0x10000, 0xd96214f7, 4 | BRF_GRA },	      //  5
	{ "silkworm.8",		0x10000, 0x0494b38e, 4 | BRF_GRA },	      //  6
	{ "silkworm.9",		0x10000, 0x8ce3cdf5, 4 | BRF_GRA },	      //  7

	{ "silkworm.10",	0x10000, 0x8c7138bb, 5 | BRF_GRA },	      //  8 - Foreground Tiles
	{ "silkworm.11",	0x10000, 0x6c03c476, 5 | BRF_GRA },	      //  9
	{ "silkworm.12",	0x10000, 0xbb0f568f, 5 | BRF_GRA },	      // 10
	{ "silkworm.13",	0x10000, 0x773ad0a4, 5 | BRF_GRA },	      // 11

	{ "silkworm.14",	0x10000, 0x409df64b, 6 | BRF_GRA },	      // 12 - Background Tiles
	{ "silkworm.15",	0x10000, 0x6e4052c9, 6 | BRF_GRA },	      // 13
	{ "silkworm.16",	0x10000, 0x9292ed63, 6 | BRF_GRA },	      // 14
	{ "silkworm.17",	0x10000, 0x3fa4563d, 6 | BRF_GRA },	      // 15

	{ "silkworm.1",		0x08000, 0x5b553644, 7 | BRF_SND },	      // 16 - Samples
};

STD_ROM_PICK(silkworm)
STD_ROM_FN(silkworm)

struct BurnDriver BurnDrvSilkworm = {
	"silkworm", NULL, NULL, NULL, "1988",
	"Silk Worm\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, silkwormRomInfo, silkwormRomName, NULL, NULL, SilkwormInputInfo, SilkwormDIPInfo,
	SilkwormInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};


// Silk Worm (set 1)

static struct BurnRomInfo silkwrmjRomDesc[] = {
	{ "silkwormj.4",	0x10000, 0x6df3df22, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 Code
	{ "silkworm.5",		0x10000, 0xa6c7bb51, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "silkwormj.3",	0x08000, 0xb79848d0, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "silkworm.2",		0x08000, 0xe80a1cd9, 3 | BRF_GRA },	      //  3 - Characters

	{ "silkworm.6",		0x10000, 0x1138d159, 4 | BRF_GRA },	      //  4 - Sprites
	{ "silkworm.7",		0x10000, 0xd96214f7, 4 | BRF_GRA },	      //  5
	{ "silkworm.8",		0x10000, 0x0494b38e, 4 | BRF_GRA },	      //  6
	{ "silkworm.9",		0x10000, 0x8ce3cdf5, 4 | BRF_GRA },	      //  7

	{ "silkworm.10",	0x10000, 0x8c7138bb, 5 | BRF_GRA },	      //  8 - Foreground Tiles
	{ "silkworm.11",	0x10000, 0x6c03c476, 5 | BRF_GRA },	      //  9
	{ "silkworm.12",	0x10000, 0xbb0f568f, 5 | BRF_GRA },	      // 10
	{ "silkworm.13",	0x10000, 0x773ad0a4, 5 | BRF_GRA },	      // 11

	{ "silkworm.14",	0x10000, 0x409df64b, 6 | BRF_GRA },	      // 12 - Background Tiles
	{ "silkworm.15",	0x10000, 0x6e4052c9, 6 | BRF_GRA },	      // 13
	{ "silkworm.16",	0x10000, 0x9292ed63, 6 | BRF_GRA },	      // 14
	{ "silkworm.17",	0x10000, 0x3fa4563d, 6 | BRF_GRA },	      // 15

	{ "silkworm.1",		0x08000, 0x5b553644, 7 | BRF_SND },	      // 16 - Samples
};

STD_ROM_PICK(silkwrmj)
STD_ROM_FN(silkwrmj)

struct BurnDriver BurnDrvSilkwrmj = {
	"silkwormj", "silkworm", NULL, NULL, "1988",
	"Silk Worm (Japan)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, silkwrmjRomInfo, silkwrmjRomName, NULL, NULL, SilkwormInputInfo, SilkwormDIPInfo,
	SilkwormInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};

// Silk Worm (bootleg)

static struct BurnRomInfo silkwormbRomDesc[] = {
	{ "e3.4",		    0x10000, 0x3d86fd58, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 Code
	{ "e4.5",		    0x10000, 0xa6c7bb51, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "e2.3",		    0x08000, 0xb7a3fb80, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "e1.2",		    0x08000, 0xe80a1cd9, 3 | BRF_GRA },	      //  3 - Characters

	{ "e5.6",		    0x10000, 0x1138d159, 4 | BRF_GRA },	      //  4 - Sprites
	{ "e6.7",		    0x10000, 0xd96214f7, 4 | BRF_GRA },	      //  5
	{ "e7.8",		    0x10000, 0x0494b38e, 4 | BRF_GRA },	      //  6
	{ "e8.9",		    0x10000, 0x8ce3cdf5, 4 | BRF_GRA },	      //  7

	{ "e9.10",	        0x10000, 0x8c7138bb, 5 | BRF_GRA },	      //  8 - Foreground Tiles
	{ "e10.11",	        0x08000, 0xc0c4687d, 5 | BRF_GRA },	      //  9
	{ "e11.12",	        0x10000, 0xbb0f568f, 5 | BRF_GRA },	      // 10
	{ "e12.13",	        0x08000, 0xfc472811, 5 | BRF_GRA },	      // 11

	{ "e13.14",	        0x10000, 0x409df64b, 6 | BRF_GRA },	      // 12 - Background Tiles
	{ "e14.15",	        0x08000, 0xb02acdb6, 6 | BRF_GRA },	      // 13
	{ "e15.16",	        0x08000, 0xcaf7b25e, 6 | BRF_GRA },	      // 14
	{ "e16.17",	        0x08000, 0x7ec93873, 6 | BRF_GRA },	      // 15

};

STD_ROM_PICK(silkwormb)
STD_ROM_FN(silkwormb)

struct BurnDriver BurnDrvSilkwormb = {
	"silkwormb", "silkworm", NULL, NULL, "1988",
	"Silk Worm (bootleg)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, silkwormbRomInfo, silkwormbRomName, NULL, NULL, SilkwormInputInfo, SilkwormDIPInfo,
	SilkwormInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};

// Back Fire (Tecmo) (Japan, Bootleg, Prototype?)

static struct BurnRomInfo backfirtRomDesc[] = {
	{ "b5-e3.bin",		0x10000, 0x0ab3bd4d, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 Code
	{ "b4-f3.bin",		0x10000, 0x150B6949, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "b2-e10.bin",		0x08000, 0x9b2ac54f, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "b3-c10.bin",		0x08000, 0x08ce729f, 3 | BRF_GRA },	      //  3 - Characters

	{ "b6-c2.bin",		0x10000, 0xc8c25e45, 4 | BRF_GRA },	      //  4 - Sprites
	{ "b7-d2.bin",		0x10000, 0x25fb6a57, 4 | BRF_GRA },	      //  5
	{ "b8-e2.bin",		0x10000, 0x6bccac4e, 4 | BRF_GRA },	      //  6
	{ "b9-h2.bin",		0x10000, 0x566a99b8, 4 | BRF_GRA },	      //  7

	{ "b13-p1.bin",		0x10000, 0x8c7138bb, 5 | BRF_GRA },	      //  8 - Foreground Tiles
	{ "b12-p2.bin",		0x10000, 0x6c03c476, 5 | BRF_GRA },	      //  9
	{ "b11-p2.bin",		0x10000, 0x0bc84b4b, 5 | BRF_GRA },	      // 10
	{ "b10-p3.bin",		0x10000, 0xec149ec3, 5 | BRF_GRA },	      // 11

	{ "b17-s1.bin",		0x10000, 0x409df64b, 6 | BRF_GRA },	      // 12 - Background Tiles
	{ "b16-s2.bin",		0x10000, 0x6e4052c9, 6 | BRF_GRA },	      // 13
	{ "b15-s2.bin",		0x10000, 0x2b6cc20e, 6 | BRF_GRA },	      // 14
	{ "b14-s3.bin",		0x08000, 0x4d29637a, 6 | BRF_GRA },	      // 15
};

STD_ROM_PICK(backfirt)
STD_ROM_FN(backfirt)

struct BurnDriver BurnDrvbackfirt = {
	"backfirt", NULL, NULL, NULL, "1988",
	"Back Fire (Tecmo) (Japan, Bootleg, Prototype?)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, backfirtRomInfo, backfirtRomName, NULL, NULL, BackfirtInputInfo, BackfirtDIPInfo,
	GeminiInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};


// Gemini Wing

static struct BurnRomInfo geminiRomDesc[] = {
	{ "gw04-5s.rom",	0x10000, 0xff9de855, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 Code
	{ "gw05-6s.rom",	0x10000, 0x5a6947a9, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "gw03-5h.rom",	0x08000, 0x9bc79596, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "gw02-3h.rom",	0x08000, 0x7acc8d35, 3 | BRF_GRA },	      //  3 - Characters

	{ "gw06-1c.rom",	0x10000, 0x4ea51631, 4 | BRF_GRA },	      //  4 - Sprites
	{ "gw07-1d.rom",	0x10000, 0xda42637e, 4 | BRF_GRA },	      //  5
	{ "gw08-1f.rom",	0x10000, 0x0b4e8d70, 4 | BRF_GRA },	      //  6
	{ "gw09-1h.rom",	0x10000, 0xb65c5e4c, 4 | BRF_GRA },	      //  7

	{ "gw10-1n.rom",	0x10000, 0x5e84cd4f, 5 | BRF_GRA },	      //  8 - Foreground Tiles
	{ "gw11-2na.rom",	0x10000, 0x08b458e1, 5 | BRF_GRA },	      //  9
	{ "gw12-2nb.rom",	0x10000, 0x229c9714, 5 | BRF_GRA },	      // 10
	{ "gw13-3n.rom",	0x10000, 0xc5dfaf47, 5 | BRF_GRA },	      // 11

	{ "gw14-1r.rom",	0x10000, 0x9c10e5b5, 6 | BRF_GRA },	      // 12 - Background Tiles
	{ "gw15-2ra.rom",	0x10000, 0x4cd18cfa, 6 | BRF_GRA },	      // 13
	{ "gw16-2rb.rom",	0x10000, 0xf911c7be, 6 | BRF_GRA },	      // 14
	{ "gw17-3r.rom",	0x10000, 0x79a9ce25, 6 | BRF_GRA },	      // 15

	{ "gw01-6a.rom",	0x08000, 0xd78afa05, 7 | BRF_SND },	      // 16 - Samples
};

STD_ROM_PICK(gemini)
STD_ROM_FN(gemini)

struct BurnDriver BurnDrvGemini = {
	"gemini", NULL, NULL, NULL, "1987",
	"Gemini Wing\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, geminiRomInfo, geminiRomName, NULL, NULL, GeminiInputInfo, GeminiDIPInfo,
	GeminiInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	224, 256, 3, 4
};
