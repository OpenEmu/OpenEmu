// FB Alpha Irem M90 driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "zet.h"
#include "burn_ym2151.h"
#include "vez.h"
#include "irem_cpu.h"
#include "dac.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvV30ROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvSndROM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvV30RAM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvPalRAM;

static UINT8 *m90_video_control;

static UINT8 *RamPrioBitmap;

static UINT8 *soundlatch;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 irqvector;
static INT32 sample_address;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvJoy4[8];
static UINT8 DrvJoy5[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[5];
static UINT8 DrvReset;

static INT32 vblank;
static INT32 code_mask[2];
static INT32 video_offsets[2] = { 0, 0 };

enum { VECTOR_INIT, YM2151_ASSERT, YM2151_CLEAR, Z80_ASSERT, Z80_CLEAR };

static struct BurnInputInfo p4commonInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy4 + 5,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	DrvJoy4 + 4,	"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy4 + 3,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy4 + 2,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy4 + 1,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy4 + 0,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy4 + 7,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy4 + 6,	"p3 fire 2"	},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy5 + 5,	"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	DrvJoy5 + 4,	"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	DrvJoy5 + 3,	"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy5 + 2,	"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy5 + 1,	"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	DrvJoy5 + 0,	"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy5 + 7,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy5 + 6,	"p4 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 4,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(p4common)

static struct BurnInputInfo p2commonInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 4,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(p2common)

static struct BurnInputInfo Quizf1InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 4"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 4,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Quizf1)

static struct BurnInputInfo Matchit2InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 4,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Matchit2)

static struct BurnDIPInfo DynablstDIPList[]=
{
	{0x22, 0xff, 0xff, 0xbf, NULL					},
	{0x23, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x22, 0x01, 0x03, 0x02, "2"					},
	{0x22, 0x01, 0x03, 0x03, "3"					},
	{0x22, 0x01, 0x03, 0x01, "4"					},
	{0x22, 0x01, 0x03, 0x00, "5"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x22, 0x01, 0x0c, 0x08, "Easy"					},
	{0x22, 0x01, 0x0c, 0x0c, "Medium"				},
	{0x22, 0x01, 0x0c, 0x04, "Hard"					},
	{0x22, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Game Title"				},
	{0x22, 0x01, 0x10, 0x10, "Dynablaster"				},
	{0x22, 0x01, 0x10, 0x00, "Bomber Man"				},

	{0   , 0xfe, 0   ,    2, "Allow Continue"			},
	{0x22, 0x01, 0x20, 0x00, "No"					},
	{0x22, 0x01, 0x20, 0x20, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x22, 0x01, 0x40, 0x40, "Off"					},
	{0x22, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x22, 0x01, 0x80, 0x80, "Off"					},
	{0x22, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x23, 0x01, 0x01, 0x01, "Off"					},
	{0x23, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Cabinet"				},
	{0x23, 0x01, 0x06, 0x04, "2 Player Upright"			},
	{0x23, 0x01, 0x06, 0x06, "4 Player Upright A"			},
	{0x23, 0x01, 0x06, 0x02, "4 Player Upright B"			},
	{0x23, 0x01, 0x06, 0x00, "Cocktail"				},

	{0   , 0xfe, 0   ,    2, "Coin Mode"				},
	{0x23, 0x01, 0x08, 0x08, "1"					},
	{0x23, 0x01, 0x08, 0x00, "2"					},

	{0   , 0xfe, 0   ,    16, "Coinage"				},
	{0x23, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0x10, "2 Coins to Start/1 to Continue"	},
	{0x23, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"			},
	{0x23, 0x01, 0xf0, 0x20, "4 Coins 3 Credits"			},
	{0x23, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x23, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"			},
	{0x23, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"			},
	{0x23, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"			},
	{0x23, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"			},
	{0x23, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"			},
	{0x23, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"			},
	{0x23, 0x01, 0xf0, 0x00, "Free Play"				},
};

STDDIPINFO(Dynablst)

static struct BurnDIPInfo AtompunkDIPList[]=
{
	{0x22, 0xff, 0xff, 0xbf, NULL					},
	{0x23, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x22, 0x01, 0x03, 0x02, "2"					},
	{0x22, 0x01, 0x03, 0x03, "3"					},
	{0x22, 0x01, 0x03, 0x01, "4"					},
	{0x22, 0x01, 0x03, 0x00, "5"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x22, 0x01, 0x0c, 0x08, "Easy"					},
	{0x22, 0x01, 0x0c, 0x0c, "Medium"				},
	{0x22, 0x01, 0x0c, 0x04, "Hard"					},
	{0x22, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Unknown"				},
	{0x22, 0x01, 0x10, 0x10, "Off"					},
	{0x22, 0x01, 0x10, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Allow Continue"			},
	{0x22, 0x01, 0x20, 0x00, "No"					},
	{0x22, 0x01, 0x20, 0x20, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x22, 0x01, 0x40, 0x40, "Off"					},
	{0x22, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x22, 0x01, 0x80, 0x80, "Off"					},
	{0x22, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x23, 0x01, 0x01, 0x01, "Off"					},
	{0x23, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Cabinet"				},
	{0x23, 0x01, 0x06, 0x04, "2 Player Upright"			},
	{0x23, 0x01, 0x06, 0x06, "4 Player Upright A"			},
	{0x23, 0x01, 0x06, 0x02, "4 Player Upright B"			},
	{0x23, 0x01, 0x06, 0x00, "Cocktail"				},

	{0   , 0xfe, 0   ,    2, "Coin Mode"				},
	{0x23, 0x01, 0x08, 0x08, "1"					},
	{0x23, 0x01, 0x08, 0x00, "2"					},

	{0   , 0xfe, 0   ,    16, "Coinage"				},
	{0x23, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0x10, "2 Coins to Start/1 to Continue"	},
	{0x23, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"			},
	{0x23, 0x01, 0xf0, 0x20, "4 Coins 3 Credits"			},
	{0x23, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x23, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"			},
	{0x23, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"			},
	{0x23, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"			},
	{0x23, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"			},
	{0x23, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"			},
	{0x23, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"			},
	{0x23, 0x01, 0xf0, 0x00, "Free Play"				},
};

STDDIPINFO(Atompunk)

static struct BurnDIPInfo BombrmanDIPList[]=
{
	{0x12, 0xff, 0xff, 0xaf, NULL					},
	{0x13, 0xff, 0xff, 0xf9, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x12, 0x01, 0x03, 0x02, "2"					},
	{0x12, 0x01, 0x03, 0x03, "3"					},
	{0x12, 0x01, 0x03, 0x01, "4"					},
	{0x12, 0x01, 0x03, 0x00, "5"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x12, 0x01, 0x0c, 0x08, "Easy"					},
	{0x12, 0x01, 0x0c, 0x0c, "Medium"				},
	{0x12, 0x01, 0x0c, 0x04, "Hard"					},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Allow Continue"			},
	{0x12, 0x01, 0x20, 0x00, "No"					},
	{0x12, 0x01, 0x20, 0x20, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x12, 0x01, 0x40, 0x40, "Off"					},
	{0x12, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x12, 0x01, 0x80, 0x80, "Off"					},
	{0x12, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x13, 0x01, 0x01, 0x01, "Off"					},
	{0x13, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Coin Mode"				},
	{0x13, 0x01, 0x08, 0x08, "1"					},
	{0x13, 0x01, 0x08, 0x00, "2"					},

	{0   , 0xfe, 0   ,    16, "Coinage"				},
	{0x13, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0x10, "2 Coins to Start/1 to Continue"	},
	{0x13, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"			},
	{0x13, 0x01, 0xf0, 0x20, "4 Coins 3 Credits"			},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x13, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"			},
	{0x13, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"			},
	{0x13, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"			},
	{0x13, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"			},
	{0x13, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"			},
	{0x13, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"			},
	{0x13, 0x01, 0xf0, 0x00, "Free Play"				},
};

STDDIPINFO(Bombrman)

static struct BurnDIPInfo HasamuDIPList[]=
{
	{0x12, 0xff, 0xff, 0xbf, NULL					},
	{0x13, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    2, "Allow Continue"			},
	{0x12, 0x01, 0x20, 0x00, "No"					},
	{0x12, 0x01, 0x20, 0x20, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x12, 0x01, 0x40, 0x40, "Off"					},
	{0x12, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x12, 0x01, 0x80, 0x80, "Off"					},
	{0x12, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x13, 0x01, 0x01, 0x01, "Off"					},
	{0x13, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Unknown"				},
	{0x13, 0x01, 0x02, 0x02, "Off"					},
	{0x13, 0x01, 0x02, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Coin Mode"				},
	{0x13, 0x01, 0x08, 0x08, "1"					},
	{0x13, 0x01, 0x08, 0x00, "2"					},

	{0   , 0xfe, 0   ,    16, "Coinage"				},
	{0x13, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0x10, "2 Coins to Start/1 to Continue"	},
	{0x13, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"			},
	{0x13, 0x01, 0xf0, 0x20, "4 Coins 3 Credits"			},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x13, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"			},
	{0x13, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"			},
	{0x13, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"			},
	{0x13, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"			},
	{0x13, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"			},
	{0x13, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"			},
	{0x13, 0x01, 0xf0, 0x00, "Free Play"				},
};

STDDIPINFO(Hasamu)

static struct BurnDIPInfo RiskchalDIPList[]=
{
	{0x12, 0xff, 0xff, 0xbf, NULL					},
	{0x13, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x12, 0x01, 0x03, 0x02, "2"					},
	{0x12, 0x01, 0x03, 0x03, "3"					},
	{0x12, 0x01, 0x03, 0x01, "4"					},
	{0x12, 0x01, 0x03, 0x00, "5"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x12, 0x01, 0x0c, 0x08, "Easy"					},
	{0x12, 0x01, 0x0c, 0x0c, "Medium"				},
	{0x12, 0x01, 0x0c, 0x04, "Hard"					},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x12, 0x01, 0x40, 0x40, "Off"					},
	{0x12, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x12, 0x01, 0x80, 0x80, "Off"					},
	{0x12, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x13, 0x01, 0x01, 0x01, "Off"					},
	{0x13, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Coin Slots"				},
	{0x13, 0x01, 0x04, 0x04, "Common"				},
	{0x13, 0x01, 0x04, 0x00, "Separate"				},

	{0   , 0xfe, 0   ,    2, "Coin Mode"				},
	{0x13, 0x01, 0x08, 0x08, "1"					},
	{0x13, 0x01, 0x08, 0x00, "2"					},

	{0   , 0xfe, 0   ,    16, "Coinage"				},
	{0x13, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"			},
	{0x13, 0x01, 0xf0, 0x10, "2 Coins to Start/1 to Continue"	},
	{0x13, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"			},
	{0x13, 0x01, 0xf0, 0x20, "4 Coins 3 Credits"			},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x13, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"			},
	{0x13, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"			},
	{0x13, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"			},
	{0x13, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"			},
	{0x13, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"			},
	{0x13, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"			},
	{0x13, 0x01, 0xf0, 0x00, "Free Play"				},
};

STDDIPINFO(Riskchal)

static struct BurnDIPInfo BbmanwDIPList[]=
{
	{0x22, 0xff, 0xff, 0xbf, NULL					},
	{0x23, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x22, 0x01, 0x03, 0x02, "2"					},
	{0x22, 0x01, 0x03, 0x03, "3"					},
	{0x22, 0x01, 0x03, 0x01, "4"					},
	{0x22, 0x01, 0x03, 0x00, "5"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x22, 0x01, 0x0c, 0x08, "Easy"					},
	{0x22, 0x01, 0x0c, 0x0c, "Medium"				},
	{0x22, 0x01, 0x0c, 0x04, "Hard"					},
	{0x22, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Game Title"				},
	{0x22, 0x01, 0x10, 0x10, "Bomber Man World"			},
	{0x22, 0x01, 0x10, 0x00, "New Dyna Blaster Global Quest"	},

	{0   , 0xfe, 0   ,    2, "Allow Continue"			},
	{0x22, 0x01, 0x20, 0x00, "No"					},
	{0x22, 0x01, 0x20, 0x20, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x22, 0x01, 0x40, 0x40, "Off"					},
	{0x22, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x22, 0x01, 0x80, 0x80, "Off"					},
	{0x22, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x23, 0x01, 0x01, 0x01, "Off"					},
	{0x23, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Cabinet"				},
	{0x23, 0x01, 0x06, 0x04, "2 Player"				},
	{0x23, 0x01, 0x06, 0x06, "4 Player Seprate Coins"		},
	{0x23, 0x01, 0x06, 0x02, "4 Player Shared Coins"		},
	{0x23, 0x01, 0x06, 0x00, "4 Player 1&2 3&4 Share Coins"		},

	{0   , 0xfe, 0   ,    2, "Coin Mode"				},
	{0x23, 0x01, 0x08, 0x08, "1"					},
	{0x23, 0x01, 0x08, 0x00, "2"					},

	{0   , 0xfe, 0   ,    16, "Coinage"				},
	{0x23, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0x10, "2 Coins to Start/1 to Continue"	},
	{0x23, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"			},
	{0x23, 0x01, 0xf0, 0x20, "4 Coins 3 Credits"			},
	{0x23, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x23, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"			},
	{0x23, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"			},
	{0x23, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"			},
	{0x23, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"			},
	{0x23, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"			},
	{0x23, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"			},
	{0x23, 0x01, 0xf0, 0x00, "Free Play"				},
};

STDDIPINFO(Bbmanw)

static struct BurnDIPInfo BbmanwjDIPList[]=
{
	{0x22, 0xff, 0xff, 0xbf, NULL					},
	{0x23, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x22, 0x01, 0x03, 0x02, "2"					},
	{0x22, 0x01, 0x03, 0x03, "3"					},
	{0x22, 0x01, 0x03, 0x01, "4"					},
	{0x22, 0x01, 0x03, 0x00, "5"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x22, 0x01, 0x0c, 0x08, "Easy"					},
	{0x22, 0x01, 0x0c, 0x0c, "Medium"				},
	{0x22, 0x01, 0x0c, 0x04, "Hard"					},
	{0x22, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Allow Continue"			},
	{0x22, 0x01, 0x20, 0x00, "No"					},
	{0x22, 0x01, 0x20, 0x20, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x22, 0x01, 0x40, 0x40, "Off"					},
	{0x22, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x22, 0x01, 0x80, 0x80, "Off"					},
	{0x22, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x23, 0x01, 0x01, 0x01, "Off"					},
	{0x23, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Cabinet"				},
	{0x23, 0x01, 0x06, 0x04, "2 Player"				},
	{0x23, 0x01, 0x06, 0x06, "4 Player Seprate Coins"		},
	{0x23, 0x01, 0x06, 0x02, "4 Player Shared Coins"		},
	{0x23, 0x01, 0x06, 0x00, "4 Player 1&2 3&4 Share Coins"		},

	{0   , 0xfe, 0   ,    2, "Coin Mode"				},
	{0x23, 0x01, 0x08, 0x08, "1"					},
	{0x23, 0x01, 0x08, 0x00, "2"					},

	{0   , 0xfe, 0   ,    16, "Coinage"				},
	{0x23, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"			},
	{0x23, 0x01, 0xf0, 0x10, "2 Coins to Start/1 to Continue"	},
	{0x23, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"			},
	{0x23, 0x01, 0xf0, 0x20, "4 Coins 3 Credits"			},
	{0x23, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x23, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"			},
	{0x23, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"			},
	{0x23, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"			},
	{0x23, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"			},
	{0x23, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"			},
	{0x23, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"			},
	{0x23, 0x01, 0xf0, 0x00, "Free Play"				},
};

STDDIPINFO(Bbmanwj)

static struct BurnDIPInfo Quizf1DIPList[]=
{
	{0x16, 0xff, 0xff, 0x9f, NULL					},
	{0x17, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x16, 0x01, 0x03, 0x02, "2"					},
	{0x16, 0x01, 0x03, 0x03, "3"					},
	{0x16, 0x01, 0x03, 0x01, "4"					},
	{0x16, 0x01, 0x03, 0x00, "5"					},

	{0   , 0xfe, 0   ,    2, "Input Device"				},
	{0x16, 0x01, 0x20, 0x20, "Joystick"				},
	{0x16, 0x01, 0x20, 0x00, "Buttons"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x16, 0x01, 0x40, 0x40, "Off"					},
	{0x16, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x16, 0x01, 0x80, 0x80, "Off"					},
	{0x16, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x17, 0x01, 0x01, 0x01, "Off"					},
	{0x17, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Coin Mode"				},
	{0x17, 0x01, 0x08, 0x08, "1"					},
	{0x17, 0x01, 0x08, 0x00, "2"					},

	{0   , 0xfe, 0   ,    16, "Coinage"				},
	{0x17, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"			},
	{0x17, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"			},
	{0x17, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"			},
	{0x17, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"			},
	{0x17, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"			},
	{0x17, 0x01, 0xf0, 0x10, "2 Coins to Start/1 to Continue"	},
	{0x17, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"			},
	{0x17, 0x01, 0xf0, 0x20, "4 Coins 3 Credits"			},
	{0x17, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x17, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"			},
	{0x17, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"			},
	{0x17, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"			},
	{0x17, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"			},
	{0x17, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"			},
	{0x17, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"			},
	{0x17, 0x01, 0xf0, 0x00, "Free Play"				},
};

STDDIPINFO(Quizf1)

static struct BurnDIPInfo Matchit2DIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL					},
	{0x15, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    2, "Girls Mode"				},
	{0x14, 0x01, 0x01, 0x00, "Off"					},
	{0x14, 0x01, 0x01, 0x01, "On"					},

	{0   , 0xfe, 0   ,    2, "China Tiles"				},
	{0x14, 0x01, 0x02, 0x02, "Mahjong"				},
	{0x14, 0x01, 0x02, 0x00, "Alpha-Numeric"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x14, 0x01, 0x0c, 0x00, "Very Hard"				},
	{0x14, 0x01, 0x0c, 0x04, "Hard"					},
	{0x14, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x14, 0x01, 0x0c, 0x08, "Easy"					},

	{0   , 0xfe, 0   ,    4, "Timer Speed"				},
	{0x14, 0x01, 0x30, 0x00, "Very Hard"				},
	{0x14, 0x01, 0x30, 0x10, "Hard"					},
	{0x14, 0x01, 0x30, 0x30, "Normal"				},
	{0x14, 0x01, 0x30, 0x20, "Easy"					},

	{0   , 0xfe, 0   ,    2, "Title Screen"				},
	{0x14, 0x01, 0x40, 0x40, "Match It II"				},
	{0x14, 0x01, 0x40, 0x00, "Shisensho II"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x14, 0x01, 0x80, 0x80, "Off"					},
	{0x14, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x15, 0x01, 0x01, 0x01, "Off"					},
	{0x15, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    3, "Language"				},
	{0x15, 0x01, 0x06, 0x06, "English"				},
	{0x15, 0x01, 0x06, 0x04, "German"				},
	{0x15, 0x01, 0x06, 0x02, "Korean"				},

	{0   , 0xfe, 0   ,    2, "Coin Mode"				},
	{0x15, 0x01, 0x08, 0x08, "1"					},
	{0x15, 0x01, 0x08, 0x00, "2"					},

	{0   , 0xfe, 0   ,    16, "Coinage"				},
	{0x15, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"			},
	{0x15, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"			},
	{0x15, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"			},
	{0x15, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"			},
	{0x15, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"			},
	{0x15, 0x01, 0xf0, 0x10, "2 Coins to Start/1 to Continue"	},
	{0x15, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"			},
	{0x15, 0x01, 0xf0, 0x20, "4 Coins 3 Credits"			},
	{0x15, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x15, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"			},
	{0x15, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"			},
	{0x15, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"			},
	{0x15, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"			},
	{0x15, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"			},
	{0x15, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"			},
	{0x15, 0x01, 0xf0, 0x00, "Free Play"				},
};

STDDIPINFO(Matchit2)

static struct BurnDIPInfo Shisen2DIPList[]=
{
	{0x14, 0xff, 0xff, 0xbf, NULL					},
	{0x15, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    2, "Girls Mode"				},
	{0x14, 0x01, 0x01, 0x00, "Off"					},
	{0x14, 0x01, 0x01, 0x01, "On"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x14, 0x01, 0x0c, 0x00, "Very Hard"				},
	{0x14, 0x01, 0x0c, 0x04, "Hard"					},
	{0x14, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x14, 0x01, 0x0c, 0x08, "Easy"					},

	{0   , 0xfe, 0   ,    4, "Timer Speed"				},
	{0x14, 0x01, 0x30, 0x00, "Very Hard"				},
	{0x14, 0x01, 0x30, 0x10, "Hard"					},
	{0x14, 0x01, 0x30, 0x30, "Normal"				},
	{0x14, 0x01, 0x30, 0x20, "Easy"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x14, 0x01, 0x40, 0x40, "Off"					},
	{0x14, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x14, 0x01, 0x80, 0x80, "Off"					},
	{0x14, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x15, 0x01, 0x01, 0x01, "Off"					},
	{0x15, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Coin Mode"				},
	{0x15, 0x01, 0x08, 0x08, "1"					},
	{0x15, 0x01, 0x08, 0x00, "2"					},

	{0   , 0xfe, 0   ,    16, "Coinage"				},
	{0x15, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"			},
	{0x15, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"			},
	{0x15, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"			},
	{0x15, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"			},
	{0x15, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"			},
	{0x15, 0x01, 0xf0, 0x10, "2 Coins to Start/1 to Continue"	},
	{0x15, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"			},
	{0x15, 0x01, 0xf0, 0x20, "4 Coins 3 Credits"			},
	{0x15, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x15, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"			},
	{0x15, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"			},
	{0x15, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"			},
	{0x15, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"			},
	{0x15, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"			},
	{0x15, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"			},
	{0x15, 0x01, 0xf0, 0x00, "Free Play"				},
};

STDDIPINFO(Shisen2)

static void setvector_callback(INT32 param)
{
	switch (param)
	{
		case VECTOR_INIT:   irqvector  = 0xff; break;
		case YM2151_ASSERT: irqvector &= 0xef; break;
		case YM2151_CLEAR:  irqvector |= 0x10; break;
		case Z80_ASSERT:    irqvector &= 0xdf; break;
		case Z80_CLEAR:     irqvector |= 0x20; break;
	}

	ZetSetVector(irqvector);
	ZetSetIRQLine(0, (irqvector == 0xff) ? ZET_IRQSTATUS_NONE : ZET_IRQSTATUS_ACK);
}

static inline void update_palette_entry(INT32 entry)
{
	UINT16 d = *((UINT16*)(DrvPalRAM + entry));

#ifndef LSB_FIRST
	d = BURN_ENDIAN_SWAP_INT16(d);
#endif

	UINT8 r = (d >>  0) & 0x1f;
	UINT8 g = (d >>  5) & 0x1f;
	UINT8 b = (d >> 10) & 0x1f;
	
	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	DrvPalette[entry / 2] = BurnHighCol(r, g, b, 0);
}

void __fastcall m90_main_write(UINT32 address, UINT8 data)
{
	if ((address & 0xffc00) == 0xe0000) {
		DrvPalRAM[address & 0x3ff] = data;
		update_palette_entry(address & 0x3fe);
		return;
	}
}

UINT8 __fastcall m90_main_read(UINT32 /*address*/)
{
	return 0;
}

void __fastcall m90_main_write_port(UINT32 port, UINT8 data)
{
	if ((port & ~0x0f) == 0x80) {
		m90_video_control[port & 0x0f] = data;
		return;
	}

	switch (port)
	{
		case 0x00:
	//	case 0x01:
			*soundlatch = data;
			setvector_callback(Z80_ASSERT);
		return;

		case 0x02:
	//	case 0x03:
			// coin counter
		return;

		case 0x04:
		//case 0x05:
		{
			INT32 bankaddress = 0x100000 + 0x10000 * (data & 0x0f);
			VezMapArea(0x80000, 0x8ffff, 0, DrvV30ROM + bankaddress);
			VezMapArea(0x80000, 0x8ffff, 2, DrvV30ROM + bankaddress);
		}
		return;

		case 0x90:
	//	case 0x91: // nop dynablsb
		return; 

		//default:
		//	bprintf (0, _T("Writing %2.2x to port %2.2x wp\n"), data, port);
		//	break;
	}
}

UINT8 __fastcall m90_main_read_port(UINT32 port)
{
	switch (port)
	{
		case 0x00: return DrvInputs[0];
		case 0x01: return DrvInputs[1];
		case 0x02: return (DrvInputs[2] & 0x7f) | vblank; // system
	//	case 0x03: return 0xff; // ?
		case 0x04: return DrvDips[0];
		case 0x05: return DrvDips[1];
		case 0x06: return DrvInputs[3];
		case 0x07: return DrvInputs[4];

		//default:
		//	bprintf (0, _T("Reading port: %2.2x\n"), port);
		//	break;
	}

	return 0;
}

void __fastcall m90_sound_write_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x40: // bbmanw
			BurnYM2151SelectRegister(data);
		return;

		case 0x01:
		case 0x41: // bbmanw
			BurnYM2151WriteRegister(data);
		return;

		case 0x80:
			sample_address >>= 5;
			sample_address = (sample_address & 0x00ff) | (data << 8);
			sample_address <<= 5;
		return;

		case 0x81:
			sample_address >>= 5;
			sample_address = (sample_address & 0xff00) | (data << 0);
			sample_address <<= 5;
		return;

		case 0x82:
			DACSignedWrite(0, data);
			sample_address = (sample_address + 1) & 0x3ffff;
		return;

		case 0x42: // bbmanw
		case 0x83:
			setvector_callback(Z80_CLEAR);
		return;
	}
}

UINT8 __fastcall m90_sound_read_port(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
		case 0x40: // bbmanw
		case 0x41:
			return BurnYM2151ReadStatus();

		case 0x42: // bbmanw
		case 0x80:
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;
	
		case 0x84:
			return DrvSndROM[sample_address & 0x3fff];
	}

	return 0;
}

static void m72YM2151IRQHandler(INT32 nStatus)
{
	setvector_callback(nStatus ? YM2151_ASSERT : YM2151_CLEAR);
}

static INT32 m90SyncDAC()
{
	return (INT32)(float)(nBurnSoundLen * (ZetTotalCycles() / (3579545.000 / (nBurnFPS / 100.000))));
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	VezOpen(0);
	VezReset();
	VezClose();

	ZetOpen(0);
	ZetReset();
	setvector_callback(VECTOR_INIT);
	ZetClose();

	BurnYM2151Reset();
	DACReset();

	sample_address = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvV30ROM 	= Next; Next += 0x200000;
	DrvZ80ROM	= Next; Next += 0x010000;
	DrvGfxROM0	= Next; Next += 0x400000;
	DrvGfxROM1	= Next; Next += 0x400000;
	DrvSndROM	= Next; Next += 0x180000;

	RamPrioBitmap	= Next; Next += nScreenWidth * nScreenHeight;

	AllRam	= Next;

	DrvSprRAM	= Next; Next += 0x000800;
	DrvVidRAM	= Next; Next += 0x010000;
	DrvV30RAM	= Next; Next += 0x004000;
	DrvPalRAM	= Next; Next += 0x000800;
	DrvZ80RAM	= Next; Next += 0x001000;

	soundlatch	= Next; Next += 0x000001;

	m90_video_control	= Next; Next += 0x000010;

	RamEnd		= Next;

	DrvPalette	= (UINT32 *) Next; Next += 0x0200 * sizeof(UINT32);

	MemEnd		= Next;

	return 0;
}

static INT32 DrvLoadRoms(INT32 codesize, INT32 samples, INT32 bank)
{
	if (BurnLoadRom(DrvV30ROM + 1,  0, 2)) return 1;
	if (BurnLoadRom(DrvV30ROM + 0,  1, 2)) return 1;

	if (codesize == 0x40000) {
		memcpy (DrvV30ROM + 0x7fff0, DrvV30ROM + 0x3fff0, 0x10);
	}

	if (BurnLoadRom(DrvZ80ROM,  2, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x000000,  3, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x080000,  4, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x100000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x180000,  6, 1)) return 1;

	if (samples) {
		if (BurnLoadRom(DrvSndROM,  7, 1)) return 1;

		if (samples < 0x40000) {
			memcpy (DrvSndROM + 0x20000, DrvSndROM + 0x00000, 0x20000);
		}
	}

	if (bank) {
		if (BurnLoadRom(DrvV30ROM + 0x100001,  8, 2)) return 1;
		if (BurnLoadRom(DrvV30ROM + 0x100000,  9, 2)) return 1;
	}

	return 0;
}

static void map_main_cpu(UINT8 *decrypt_table, INT32 codesize, INT32 spriteram)
{
	VezInit(0, V35_TYPE, 32000000/2);

	VezOpen(0);
	if (decrypt_table != NULL) {
		VezSetDecode(decrypt_table);
	}

	VezMapArea(0x00000, (codesize-1), 0, DrvV30ROM + 0x000000);
	VezMapArea(0x00000, (codesize-1), 2, DrvV30ROM + 0x000000);
	VezMapArea(0x80000, 0x8ffff, 0, DrvV30ROM + 0x100000);
	VezMapArea(0x80000, 0x8ffff, 2, DrvV30ROM + 0x100000);
	VezMapArea(0xa0000, 0xa3fff, 0, DrvV30RAM);
	VezMapArea(0xa0000, 0xa3fff, 1, DrvV30RAM);
	VezMapArea(0xa0000, 0xa3fff, 2, DrvV30RAM);
	VezMapArea(0xd0000, 0xdffff, 0, DrvVidRAM);
	VezMapArea(0xd0000, 0xdffff, 1, DrvVidRAM);
	VezMapArea(0xd0000, 0xdffff, 2, DrvVidRAM);
	VezMapArea(0xe0000, 0xe07ff, 0, DrvPalRAM);
//	VezMapArea(0xe0000, 0xe07ff, 1, DrvPalRAM);
	VezMapArea(0xe0000, 0xe07ff, 2, DrvPalRAM);
	VezMapArea(0xff800, 0xfffff, 0, DrvV30ROM + 0x07f800);
	VezMapArea(0xff800, 0xfffff, 2, DrvV30ROM + 0x07f800);

	if (spriteram) {
		VezMapArea(0x00000 + spriteram, 0x00fff + spriteram, 0, DrvSprRAM); // 00e - fff // ??
		VezMapArea(0x00000 + spriteram, 0x00fff + spriteram, 1, DrvSprRAM);
		VezMapArea(0x00000 + spriteram, 0x00fff + spriteram, 2, DrvSprRAM);	
	}

	VezSetReadHandler(m90_main_read);
	VezSetWriteHandler(m90_main_write);
	VezSetReadPort(m90_main_read_port);
	VezSetWritePort(m90_main_write_port);
	VezClose();
}

static void DrvGfxDecode()
{
	INT32 Plane[4]  = { 0x180000 * 8, 0x100000 * 8, 0x080000 * 8, 0x000000 * 8 };
	INT32 XOffs[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 };
	INT32 YOffs[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x200000);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, DrvGfxROM1, 0x200000);

	GfxDecode(0x10000, 4,  8,  8, Plane, XOffs, YOffs, 0x040, tmp, DrvGfxROM0);
	GfxDecode(0x04000, 4, 16, 16, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM1);

	BurnFree (tmp);
}

static INT32 DrvInit(INT32 codesize, INT32 gfxlen, INT32 samples, INT32 bank, INT32 spriteram, const UINT8 *decrypt_table)
{
	GenericTilesInit();

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (DrvLoadRoms(codesize, samples, bank)) return 1; 

	DrvGfxDecode();

	map_main_cpu((UINT8*)decrypt_table, codesize, spriteram);

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xefff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0xefff, 2, DrvZ80ROM);
	ZetMapArea(0xf000, 0xffff, 0, DrvZ80RAM);
	ZetMapArea(0xf000, 0xffff, 1, DrvZ80RAM);
	ZetMapArea(0xf000, 0xffff, 2, DrvZ80RAM);
	ZetSetOutHandler(m90_sound_write_port);
	ZetSetInHandler(m90_sound_read_port);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(3579545, 15.0);
	YM2151SetIrqHandler(0, &m72YM2151IRQHandler);

	DACInit(0, 0, 1, m90SyncDAC);
	DACSetVolShift(0, 4); // 1/16th of max

	code_mask[0] = ((gfxlen * 2) - 1) / (8 * 8);
	code_mask[1] = ((gfxlen * 2) - 1) / (16 * 16);

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	BurnYM2151Exit();
	DACExit();

	ZetExit();
	VezExit();

	BurnFree(AllMem);

	video_offsets[0] = video_offsets[1] = 0;

	return 0;
}

static void RenderTilePrio(UINT16 *dest, UINT8 *gfx, INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 width, INT32 height, UINT8 *pri, INT32 prio)
{
	if (sx <= (0-width) || sx >= nScreenWidth || sy <= (0-height) || sy >= nScreenHeight) return;

	INT32 flip = 0;
	if (flipy) flip |= (height - 1) * width;
	if (flipx) flip |= width - 1;

	gfx += code * width * height;

	for (INT32 y = 0; y < height; y++, sy++) {
		if (sy < 0 || sy >= nScreenHeight) continue;

		for (INT32 x = 0; x < width; x++, sx++) {
			if (sx < 0 || sx >= nScreenWidth) continue;

			INT32 pxl = gfx[((y * width) + x) ^ flip];

			if (pxl == 0) continue;

			if ((prio & (1 << pri[sy * nScreenWidth + sx])) == 0) {
				dest[sy * nScreenWidth + sx] = pxl | color;
				pri[sy * nScreenWidth + sx] = 7;
			}
		}

		sx -= width;
	}
}

static void draw_sprites()
{
	INT32 pri_ctrl = m90_video_control[14] & 0x03;
	UINT16 *sram = (UINT16*)(DrvVidRAM + 0xee00);

	for (INT32 offs = 0x1f2/2; offs >= 0; offs -= 3)
	{
		INT32 sy    = BURN_ENDIAN_SWAP_INT16(sram[offs + 0]);
		INT32 code  = BURN_ENDIAN_SWAP_INT16(sram[offs + 1]);
		INT32 sx    = BURN_ENDIAN_SWAP_INT16(sram[offs + 2]);

		INT32 flipy = sy & 0x8000;
		INT32 flipx = sx & 0x0200;
		INT32 multi = 1 << ((sy & 0x6000) >> 13);
		INT32 color = (sy >> 9) & 0x0f;

		sx = ((sx - video_offsets[0]) & 0x1ff) - 16;
		sy = (512 - ((sy + video_offsets[1]) & 0x1ff)) - (16 * multi);

		INT32 prio = 0x82;
		if (pri_ctrl & 0x01) {
			if ((color & 0x08) == 0x08) prio = 0x80;
		} else if (pri_ctrl & 0x02) {
			if ((color & 0x0c) == 0x0c) prio = 0x80;
		}

		for (INT32 i = 0; i < multi; i++) {
			RenderTilePrio(pTransDraw, DrvGfxROM1, (code + (flipy ? multi-1 - i : i)) & code_mask[1], (color << 4)|0x100, sx, sy+i*16, flipx, flipy, 16, 16, RamPrioBitmap, prio);
		}
	}
}

static void draw_layer(INT32 layer)
{
	INT32 control = m90_video_control[10 + (layer * 2)];

	if (control & 0x10) return; // disable layer

	INT32 enable_rowscroll = control & 0x20;
	INT32 enable_colscroll = control & 0x40;

	INT32 wide = (control & 0x04) ? 128 : 64;

	INT32 trans = layer ? 0xff : 0;
	
	INT32 pmask = (wide == 128) ? 2 : 3;
	UINT16 *vram = (UINT16*)(DrvVidRAM + (control & pmask) * 0x4000);

	UINT16  scrollx = ((m90_video_control[(layer*4)+2] << 0) | (m90_video_control[(layer*4)+3] << 8)) + video_offsets[0];
	UINT16  scrolly = ((m90_video_control[(layer*4)+0] << 0) | (m90_video_control[(layer*4)+1] << 8)) + video_offsets[1];

	scrollx += layer ? -2 : 2;
	scrollx += ((wide & 0x80) * 2);

	UINT16 *xscroll = (UINT16*)(DrvVidRAM + 0xf000 + (layer * 0x400));
	UINT16 *yscroll = (UINT16*)(DrvVidRAM + 0xf800 + (layer * 0x400));

	for (INT32 sy = 0; sy < nScreenHeight; sy++)
	{
		INT32 scrollx_1 = scrollx;
		INT32 scrolly_1 = 0;
		UINT16 *dest = pTransDraw + (sy * nScreenWidth);
		UINT8 *pri = RamPrioBitmap + (sy * nScreenWidth);

		if (enable_rowscroll) scrollx_1 += BURN_ENDIAN_SWAP_INT16(xscroll[sy]);

		if (enable_colscroll) {
			scrolly_1 += (scrolly + sy + BURN_ENDIAN_SWAP_INT16(yscroll[sy]) + 128) & 0x1ff;
		} else {
			scrolly_1 += (scrolly + sy) & 0x1ff;
		}

		INT32 romoff_1 = (scrolly_1 & 0x07) << 3;
		
		for (INT32 sx = 0; sx < nScreenWidth + 8; sx+=8)
		{
			INT32 scrollx_2 = (scrollx_1 + sx) & ((wide * 8) - 1);

			INT32 offs = ((scrolly_1 / 8) * wide) | (scrollx_2 / 8);

			INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[offs * 2 + 0]) & code_mask[0];
			INT32 color = BURN_ENDIAN_SWAP_INT16(vram[offs * 2 + 1]);

			INT32 flipy = color & 0x80;
			INT32 flipx = color & 0x40;
			INT32 group =(color & 0x30) ? 0 : 1;
			color &= 0x0f;	

			{
				color <<= 4;
				INT32 x_xor = 0;
				INT32 romoff = romoff_1;
				if (flipy) romoff ^= 0x38;
				if (flipx) x_xor = 7;

				UINT8 *rom = DrvGfxROM0 + (code * 0x40) + romoff;

				INT32 xx = sx - (scrollx_2&0x7);

				for (INT32 x = 0; x < 8; x++, xx++) {
					if (xx < 0 || xx >= nScreenWidth) continue;

					INT32 pxl = rom[x ^ x_xor];
					if (pxl == trans) continue;

					dest[xx] = pxl | color;
					pri[xx] = 1 << group;
				}
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x400; i+=2) {
			update_palette_entry(i);
		}
		DrvRecalc = 0;
	}

	if ((m90_video_control[14] & 0x04) == 0) {
		if (m90_video_control[12] & 0x10) {
			memset (RamPrioBitmap, 0, nScreenWidth * nScreenHeight);
			BurnTransferClear();
		} else {
			if (nBurnLayer & 1) draw_layer(1);
		}

		if (nBurnLayer & 2) draw_layer(0);

		if (nBurnLayer & 4) draw_sprites();
	} else {
		BurnTransferClear();
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static void compile_inputs()
{
	memset (DrvInputs, 0xff, 5);

	for (INT32 i = 0; i < 8; i++) {
		DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
		DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
	}
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	compile_inputs();
	
	INT32 nInterleave = 128; // nmi pulses for sound cpu
	INT32 nCyclesTotal[2];
	INT32 nCyclesDone[2];

	// overclocking...
	nCyclesTotal[0] = (INT32)((INT64)(8000000 / 60) * nBurnCPUSpeedAdjust / 0x0100);
	nCyclesTotal[1] = (INT32)((INT64)(3579545 / 60) * nBurnCPUSpeedAdjust / 0x0100);
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	VezNewFrame();
	ZetNewFrame();

	VezOpen(0);
	ZetOpen(0);

	vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += VezRun(nCyclesTotal[0] / nInterleave);

		if (i == (nInterleave - 1))
		{
			VezSetIRQLineAndVector(NEC_INPUT_LINE_INTP0, 0xff, VEZ_IRQSTATUS_ACK);
			VezRun(0);
			VezSetIRQLineAndVector(NEC_INPUT_LINE_INTP0, 0xff, VEZ_IRQSTATUS_NONE);
		}

		nCyclesDone[1] += ZetRun(nCyclesTotal[1] / nInterleave);
		ZetNmi();

		if (i == 124) vblank = 0x80;
	}

	if (pBurnSoundOut) {
		BurnYM2151Render(pBurnSoundOut, nBurnSoundLen);
		DACUpdate(pBurnSoundOut, nBurnSoundLen);
	}

	VezClose();
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}



// Hasamu (Japan)

static struct BurnRomInfo hasamuRomDesc[] = {
	{ "hasc-p1.bin",	0x20000, 0x53df9834, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "hasc-p0.bin",	0x20000, 0xdff0ba6e, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "hasc-sp.bin",	0x10000, 0x259b1687, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "hasc-c0.bin",	0x20000, 0xdd5a2174, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "hasc-c1.bin",	0x20000, 0x76b8217c, 3 | BRF_GRA },           //  4
	{ "hasc-c2.bin",	0x20000, 0xd90f9a68, 3 | BRF_GRA },           //  5
	{ "hasc-c3.bin",	0x20000, 0x6cfe0d39, 3 | BRF_GRA },           //  6
};

STD_ROM_PICK(hasamu)
STD_ROM_FN(hasamu)

static INT32 hasamuInit()
{
	video_offsets[0] = 48;
	video_offsets[1] = 136;
	return DrvInit(0x40000, 0x80000, 0x00000, 0, 0, gunforce_decryption_table);
}

struct BurnDriver BurnDrvHasamu = {
	"hasamu", NULL, NULL, NULL, "1991",
	"Hasamu (Japan)\0", NULL, "Irem", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, hasamuRomInfo, hasamuRomName, NULL, NULL, p2commonInputInfo, HasamuDIPInfo,
	hasamuInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	384, 240, 4, 3
};


// Dynablaster / Bomber Man

static struct BurnRomInfo dynablstRomDesc[] = {
	{ "bbm-cp1e.62",	0x20000, 0x27667681, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "bbm-cp0e.65",	0x20000, 0x95db7a67, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "bbm-sp.23",		0x10000, 0x251090cd, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "bbm-c0.66",		0x40000, 0x695d2019, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "bbm-c1.67",		0x40000, 0x4c7c8bbc, 3 | BRF_GRA },           //  4
	{ "bbm-c2.68",		0x40000, 0x0700d406, 3 | BRF_GRA },           //  5
	{ "bbm-c3.69",		0x40000, 0x3c3613af, 3 | BRF_GRA },           //  6

	{ "bbm-v0.20",		0x20000, 0x0fa803fe, 4 | BRF_SND },           //  7 Samples
};

STD_ROM_PICK(dynablst)
STD_ROM_FN(dynablst)

static INT32 dynablstInit()
{
	video_offsets[0] = 80;
	video_offsets[1] = 136;
	return DrvInit(0x40000, 0x100000, 0x20000, 0, 0, bomberman_decryption_table);
}

struct BurnDriver BurnDrvDynablst = {
	"dynablst", NULL, NULL, NULL, "1991",
	"Dynablaster / Bomber Man\0", NULL, "Irem (licensed from Hudson Soft)", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, dynablstRomInfo, dynablstRomName, NULL, NULL, p4commonInputInfo, DynablstDIPInfo,
	dynablstInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	320, 240, 4, 3
};


// Bomber Man (Japan)

static struct BurnRomInfo bombrmanRomDesc[] = {
	{ "bbm-p1.62",		0x20000, 0x982bd166, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "bbm-p0.65",		0x20000, 0x0a20afcc, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "bbm-sp.23",		0x10000, 0x251090cd, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "bbm-c0.66",		0x40000, 0x695d2019, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "bbm-c1.67",		0x40000, 0x4c7c8bbc, 3 | BRF_GRA },           //  4
	{ "bbm-c2.68",		0x40000, 0x0700d406, 3 | BRF_GRA },           //  5
	{ "bbm-c3.69",		0x40000, 0x3c3613af, 3 | BRF_GRA },           //  6

	{ "bbm-v0.20",		0x20000, 0x0fa803fe, 4 | BRF_SND },           //  7 Samples
};

STD_ROM_PICK(bombrman)
STD_ROM_FN(bombrman)

struct BurnDriver BurnDrvBombrman = {
	"bombrman", "dynablst", NULL, NULL, "1991",
	"Bomber Man (Japan)\0", NULL, "Irem (licensed from Hudson Soft)", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, bombrmanRomInfo, bombrmanRomName, NULL, NULL, p2commonInputInfo, BombrmanDIPInfo,
	dynablstInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	320, 240, 4, 3
};


// Atomic Punk (US)

static struct BurnRomInfo atompunkRomDesc[] = {
	{ "bbm-cp0d.65",	0x20000, 0x860c0479, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "bbm-cp1d.62",	0x20000, 0xbe57bf74, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "bbm-sp.23",		0x10000, 0x251090cd, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "bbm-c0.66",		0x40000, 0x695d2019, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "bbm-c1.67",		0x40000, 0x4c7c8bbc, 3 | BRF_GRA },           //  4
	{ "bbm-c2.68",		0x40000, 0x0700d406, 3 | BRF_GRA },           //  5
	{ "bbm-c3.69",		0x40000, 0x3c3613af, 3 | BRF_GRA },           //  6

	{ "bbm-v0.20",		0x20000, 0x0fa803fe, 4 | BRF_SND },           //  7 Samples
};

STD_ROM_PICK(atompunk)
STD_ROM_FN(atompunk)

struct BurnDriver BurnDrvAtompunk = {
	"atompunk", "dynablst", NULL, NULL, "1991",
	"Atomic Punk (US)\0", NULL, "Irem America (licensed from Hudson Soft)", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, atompunkRomInfo, atompunkRomName, NULL, NULL, p4commonInputInfo, AtompunkDIPInfo,
	dynablstInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	320, 240, 4, 3
};


// Bomber Man World / New Dyna Blaster - Global Quest

static struct BurnRomInfo bbmanwRomDesc[] = {
	{ "bbm2-h0-b.77",	0x40000, 0x567d3709, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "bbm2-l0-b.79",	0x40000, 0xe762c22b, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "bbm2-sp.33",		0x10000, 0x6bc1689e, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "bbm2-c0.81",		0x40000, 0xe7ce058a, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "bbm2-c1.82",		0x40000, 0x636a78a9, 3 | BRF_GRA },           //  4
	{ "bbm2-c2.83",		0x40000, 0x9ac2142f, 3 | BRF_GRA },           //  5
	{ "bbm2-c3.84",		0x40000, 0x47af1750, 3 | BRF_GRA },           //  6

	{ "bbm2-v0.30",		0x20000, 0x4ad889ed, 4 | BRF_SND },           //  7 Samples
};

STD_ROM_PICK(bbmanw)
STD_ROM_FN(bbmanw)

static INT32 bbmanwInit()
{
	video_offsets[0] = 80;
	video_offsets[1] = 136;
	return DrvInit(0x80000, 0x100000, 0x20000, 0, 0, dynablaster_decryption_table);
}

struct BurnDriver BurnDrvBbmanw = {
	"bbmanw", NULL, NULL, NULL, "1992",
	"Bomber Man World / New Dyna Blaster - Global Quest\0", NULL, "Irem", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, bbmanwRomInfo, bbmanwRomName, NULL, NULL, p4commonInputInfo, BbmanwDIPInfo,
	bbmanwInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	320, 240, 4, 3
};


// Bomber Man World (Japan)

static struct BurnRomInfo bbmanwjRomDesc[] = {
	{ "bbm2-h0.77",		0x40000, 0xe1407b91, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "bbm2-l0.79",		0x40000, 0x20873b49, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "bbm2-sp-b.bin",	0x10000, 0xb8d8108c, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "bbm2-c0.81",		0x40000, 0xe7ce058a, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "bbm2-c1.82",		0x40000, 0x636a78a9, 3 | BRF_GRA },           //  4
	{ "bbm2-c2.83",		0x40000, 0x9ac2142f, 3 | BRF_GRA },           //  5
	{ "bbm2-c3.84",		0x40000, 0x47af1750, 3 | BRF_GRA },           //  6

	{ "bbm2-v0-b.30",	0x20000, 0x0ae655ff, 4 | BRF_SND },           //  7 Samples
};

STD_ROM_PICK(bbmanwj)
STD_ROM_FN(bbmanwj)

struct BurnDriver BurnDrvBbmanwj = {
	"bbmanwj", "bbmanw", NULL, NULL, "1992",
	"Bomber Man World (Japan)\0", NULL, "Irem", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, bbmanwjRomInfo, bbmanwjRomName, NULL, NULL, p4commonInputInfo, BbmanwjDIPInfo,
	bbmanwInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	320, 240, 4, 3
};


// New Atomic Punk - Global Quest (US)

static struct BurnRomInfo newapunkRomDesc[] = {
	{ "bbm2-h0-a.77",	0x40000, 0x7d858682, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "bbm2-l0-a.79",	0x40000, 0xc7568031, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "bbm2-sp.33",		0x10000, 0x6bc1689e, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "bbm2-c0.81",		0x40000, 0xe7ce058a, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "bbm2-c1.82",		0x40000, 0x636a78a9, 3 | BRF_GRA },           //  4
	{ "bbm2-c2.83",		0x40000, 0x9ac2142f, 3 | BRF_GRA },           //  5
	{ "bbm2-c3.84",		0x40000, 0x47af1750, 3 | BRF_GRA },           //  6

	{ "bbm2-v0.30",		0x20000, 0x4ad889ed, 4 | BRF_SND },           //  7 Samples
};

STD_ROM_PICK(newapunk)
STD_ROM_FN(newapunk)

struct BurnDriver BurnDrvNewapunk = {
	"newapunk", "bbmanw", NULL, NULL, "1992",
	"New Atomic Punk - Global Quest (US)\0", NULL, "Irem America", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, newapunkRomInfo, newapunkRomName, NULL, NULL, p4commonInputInfo, BbmanwjDIPInfo,
	bbmanwInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	320, 240, 4, 3
};


// Quiz F1 1-2 Finish (Japan)

static struct BurnRomInfo quizf1RomDesc[] = {
	{ "qf1-h0-.77",		0x40000, 0x280e3049, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "qf1-l0-.79",		0x40000, 0x94588a6f, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "qf1-sp-.33",		0x10000, 0x0664fa9f, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "qf1-c0-.81",		0x80000, 0xc26b521e, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "qf1-c1-.82",		0x80000, 0xdb9d7394, 3 | BRF_GRA },           //  4
	{ "qf1-c2-.83",		0x80000, 0x0b1460ae, 3 | BRF_GRA },           //  5
	{ "qf1-c3-.84",		0x80000, 0x2d32ff37, 3 | BRF_GRA },           //  6

	{ "qf1-v0-.30",		0x40000, 0xb8d16e7c, 4 | BRF_SND },           //  7 Samples

	{ "qf1-h1-.78",		0x80000, 0xc6c2eb2b, 5 | BRF_PRG | BRF_ESS }, //  8 V30 banked code
	{ "qf1-l1-.80",		0x80000, 0x3132c144, 5 | BRF_PRG | BRF_ESS }, //  9
};

STD_ROM_PICK(quizf1)
STD_ROM_FN(quizf1)

static INT32 quizf1Init()
{
	video_offsets[0] = 48;
	video_offsets[1] = 128;
	return DrvInit(0x80000, 0x200000, 0x40000, 0x100000, 0, lethalth_decryption_table);
}

struct BurnDriver BurnDrvQuizf1 = {
	"quizf1", NULL, NULL, NULL, "1992",
	"Quiz F1 1-2 Finish (Japan)\0", NULL, "Irem", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, quizf1RomInfo, quizf1RomName, NULL, NULL, Quizf1InputInfo, Quizf1DIPInfo,
	quizf1Init, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	384, 240, 4, 3
};


// Risky Challenge

static struct BurnRomInfo riskchalRomDesc[] = {
	{ "rc_h0.ic77",		0x40000, 0x4c9b5344, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "rc_l0.ic79",		0x40000, 0x0455895a, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "l4_a-sp.ic33",	0x10000, 0xbb80094e, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "rc_c0.rom",		0x80000, 0x84d0b907, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "rc_c1.rom",		0x80000, 0xcb3784ef, 3 | BRF_GRA },           //  4
	{ "rc_c2.rom",		0x80000, 0x687164d7, 3 | BRF_GRA },           //  5
	{ "rc_c3.rom",		0x80000, 0xc86be6af, 3 | BRF_GRA },           //  6

	{ "rc_v0.rom",		0x40000, 0xcddac360, 4 | BRF_SND },           //  7 Samples
};

STD_ROM_PICK(riskchal)
STD_ROM_FN(riskchal)

static INT32 riskchalInit()
{
	video_offsets[0] = 80;
	video_offsets[1] = 136;
	return DrvInit(0x80000, 0x200000, 0x40000, 0, 0, gussun_decryption_table);
}

struct BurnDriver BurnDrvRiskchal = {
	"riskchal", NULL, NULL, NULL, "1993",
	"Risky Challenge\0", "Unemulated CPU functions", "Irem", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, riskchalRomInfo, riskchalRomName, NULL, NULL, p2commonInputInfo, RiskchalDIPInfo,
	riskchalInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	320, 240, 4, 3
};


// Gussun Oyoyo (Japan)

static struct BurnRomInfo gussunRomDesc[] = {
	{ "l4_a-h0.ic77",	0x40000, 0x9d585e61, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "l4_a-l0.ic79",	0x40000, 0xc7b4c519, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "l4_a-sp.ic33",	0x10000, 0xbb80094e, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "rc_c0.rom",		0x80000, 0x84d0b907, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "rc_c1.rom",		0x80000, 0xcb3784ef, 3 | BRF_GRA },           //  4
	{ "rc_c2.rom",		0x80000, 0x687164d7, 3 | BRF_GRA },           //  5
	{ "rc_c3.rom",		0x80000, 0xc86be6af, 3 | BRF_GRA },           //  6

	{ "rc_v0.rom",		0x40000, 0xcddac360, 4 | BRF_SND },           //  7 Samples
};

STD_ROM_PICK(gussun)
STD_ROM_FN(gussun)

struct BurnDriver BurnDrvGussun = {
	"gussun", "riskchal", NULL, NULL, "1993",
	"Gussun Oyoyo (Japan)\0", "Unemulated CPU functions", "Irem", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, gussunRomInfo, gussunRomName, NULL, NULL, p2commonInputInfo, RiskchalDIPInfo,
	riskchalInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	320, 240, 4, 3
};


// Match It II

static struct BurnRomInfo matchit2RomDesc[] = {
	{ "sis2-h0-b.bin",	0x40000, 0x9a2556ac, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "sis2-l0-b.bin",	0x40000, 0xd35d948a, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "sis2-sp-.rom",	0x10000, 0x6fc0ff3a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "ic81.rom",		0x80000, 0x5a7cb88f, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "ic82.rom",		0x80000, 0x54a7852c, 3 | BRF_GRA },           //  4
	{ "ic83.rom",		0x80000, 0x2bd65dc6, 3 | BRF_GRA },           //  5
	{ "ic84.rom",		0x80000, 0x876d5fdb, 3 | BRF_GRA },           //  6
};

STD_ROM_PICK(matchit2)
STD_ROM_FN(matchit2)

static INT32 matchit2Init()
{
	video_offsets[0] = 48;
	video_offsets[1] = 128;
	return DrvInit(0x80000, 0x200000, 0, 0, 0, matchit2_decryption_table);
}

struct BurnDriver BurnDrvMatchit2 = {
	"matchit2", NULL, NULL, NULL, "1993",
	"Match It II\0", NULL, "Tamtex", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, matchit2RomInfo, matchit2RomName, NULL, NULL, Matchit2InputInfo, Matchit2DIPInfo,
	matchit2Init, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	384, 240, 4, 3
};


// Shisensho II

static struct BurnRomInfo shisen2RomDesc[] = {
	{ "sis2-h0-.rom",	0x40000, 0x6fae0aea, 1 | BRF_PRG | BRF_ESS }, //  0 V30 Code
	{ "sis2-l0-.rom",	0x40000, 0x2af25182, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "sis2-sp-.rom",	0x10000, 0x6fc0ff3a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "ic81.rom",		0x80000, 0x5a7cb88f, 3 | BRF_GRA },           //  3 Tiles and sprites
	{ "ic82.rom",		0x80000, 0x54a7852c, 3 | BRF_GRA },           //  4
	{ "ic83.rom",		0x80000, 0x2bd65dc6, 3 | BRF_GRA },           //  5
	{ "ic84.rom",		0x80000, 0x876d5fdb, 3 | BRF_GRA },           //  6
};

STD_ROM_PICK(shisen2)
STD_ROM_FN(shisen2)

struct BurnDriver BurnDrvShisen2 = {
	"shisen2", "matchit2", NULL, NULL, "1993",
	"Shisensho II\0", NULL, "Tamtex", "M90",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M90, GBF_MISC, 0,
	NULL, shisen2RomInfo, shisen2RomName, NULL, NULL, Matchit2InputInfo, Shisen2DIPInfo,
	matchit2Init, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	384, 240, 4, 3
};
