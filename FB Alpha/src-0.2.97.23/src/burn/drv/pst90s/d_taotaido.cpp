// FB Alpha "Tao Taido" driver module
// Based on MAME driver by David Haywood

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "burn_ym2610.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *DrvPalRAM;
static UINT8 *DrvBgRAM;
static UINT8 *DrvSprRAM0;
static UINT8 *DrvSprRAM1;
static UINT8 *DrvSpr0Buf0;
static UINT8 *DrvSpr1Buf0;
static UINT8 *DrvSpr0Buf1;
static UINT8 *DrvSpr1Buf1;
static UINT8 *DrvScrollRAM;
static UINT8 *taotaido_tileregs;
static UINT8 *taotaido_spritebank;
static UINT32 *DrvPalette;

static UINT8 DrvRecalc;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvJoy4[8];
static UINT8 DrvJoy5[8];
static UINT8 DrvDip[4];
static UINT16 DrvInps[5];
static UINT8 DrvReset;

static UINT8 *nDrvZ80Bank;
static UINT8 *pending_command;
static UINT8 *soundlatch;

static INT32 nCyclesTotal[2];

static struct BurnInputInfo TaotaidoInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 6,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 5,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDip + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDip + 2,	"dip"		},
	{"Region",		BIT_DIPSWITCH,	DrvDip + 3,	"dip"		},
};

STDINPUTINFO(Taotaido)

static struct BurnInputInfo TaotaidoaInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Weak Punch",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Medium Punch",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Strong Punch",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},
	{"P1 Weak Kick",	BIT_DIGITAL,	DrvJoy4 + 0,	"p1 fire 4"	},
	{"P1 Medium Kick",	BIT_DIGITAL,	DrvJoy4 + 1,	"p1 fire 5"	},
	{"P1 Strong Kick",	BIT_DIGITAL,	DrvJoy4 + 2,	"p1 fire 6"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Weak Punch",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Medium Punch",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Strong Punch",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},
	{"P2 Weak Kick",	BIT_DIGITAL,	DrvJoy5 + 0,	"p2 fire 4"	},
	{"P2 Medium Kick",	BIT_DIGITAL,	DrvJoy5 + 1,	"p2 fire 5"	},
	{"P2 Strong Kick",	BIT_DIGITAL,	DrvJoy5 + 2,	"p2 fire 6"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 6,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 5,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDip + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDip + 2,	"dip"		},
	{"Region",		BIT_DIPSWITCH,	DrvDip + 3,	"dip"		},
};

STDINPUTINFO(Taotaidoa)

static struct BurnDIPInfo TaotaidoDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL					},
	{0x16, 0xff, 0xff, 0xff, NULL					},
	{0x17, 0xff, 0xff, 0xff, NULL					},
	{0x18, 0xff, 0xff, 0x08, NULL					},

	{0   , 0xfe, 0   ,    2, "Coin Slot"	},
	{0x15, 0x01, 0x01, 0x01, "Same"				},
	{0x15, 0x01, 0x01, 0x00, "Individual"	},

	// Coinage condition: Coin Slot Individual
	{0   , 0xfe, 0   ,    8, "Coinage"				},
	{0x15, 0x02, 0x0e, 0x0a, "3 Coins 1 Credit"			},
	{0x15, 0x00, 0x01, 0x00, NULL},
	{0x15, 0x02, 0x0e, 0x0c, "2 Coins 1 Credit"			},
	{0x15, 0x00, 0x01, 0x00, NULL},
	{0x15, 0x02, 0x0e, 0x0e, "1 Coin 1 Credit"			},
	{0x15, 0x00, 0x01, 0x00, NULL},
	{0x15, 0x02, 0x0e, 0x08, "1 Coin 2 Credits"			},
	{0x15, 0x00, 0x01, 0x00, NULL},
	{0x15, 0x02, 0x0e, 0x06, "1 Coin 3 Credits"			},
	{0x15, 0x00, 0x01, 0x00, NULL},
	{0x15, 0x02, 0x0e, 0x04, "1 Coin 4 Credits"			},
	{0x15, 0x00, 0x01, 0x00, NULL},
	{0x15, 0x02, 0x0e, 0x02, "1 Coin 5 Credits"			},
	{0x15, 0x00, 0x01, 0x00, NULL},
	{0x15, 0x02, 0x0e, 0x00, "1 Coin 6 Credits"			},
	{0x15, 0x00, 0x01, 0x00, NULL},

	// Coin 1 condition: Coin Slot Same
	{0   , 0xfe, 0   ,    8, "Coin 1"				},
	{0x15, 0x02, 0x0e, 0x0a, "3 Coins 1 Credit"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x0e, 0x0c, "2 Coins 1 Credit"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x0e, 0x0e, "1 Coin 1 Credit"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x0e, 0x08, "1 Coin 2 Credits"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x0e, 0x06, "1 Coin 3 Credits"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x0e, 0x04, "1 Coin 4 Credits"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x0e, 0x02, "1 Coin 5 Credits"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x0e, 0x00, "1 Coin 6 Credits"			},
	{0x15, 0x00, 0x01, 0x01, NULL},

	// Coin 2 condition: Coin Slot Same
	{0   , 0xfe, 0   ,    8, "Coin 2"				},
	{0x15, 0x02, 0x70, 0x50, "3 Coins 1 Credit"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x70, 0x60, "2 Coins 1 Credit"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x70, 0x70, "1 Coin 1 Credit"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x70, 0x40, "1 Coin 2 Credits"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x70, 0x30, "1 Coin 3 Credits"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x70, 0x20, "1 Coin 4 Credits"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x70, 0x10, "1 Coin 5 Credits"			},
	{0x15, 0x00, 0x01, 0x01, NULL},
	{0x15, 0x02, 0x70, 0x00, "1 Coin 6 Credits"			},
	{0x15, 0x00, 0x01, 0x01, NULL},

	{0   , 0xfe, 0   ,    2, "2 Coins to Start, 1 to Continue"	},
	{0x15, 0x01, 0x80, 0x80, "Off"				},
	{0x15, 0x01, 0x80, 0x00, "On"					},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
//	{0x16, 0x01, 0x01, 0x01, "Off"				},
//	{0x16, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"	},
	{0x16, 0x01, 0x02, 0x00, "Off"				},
	{0x16, 0x01, 0x02, 0x02, "On"					},

	{0   , 0xfe, 0   ,    2, "Free Play"	},
	{0x16, 0x01, 0x04, 0x04, "Off"				},
	{0x16, 0x01, 0x04, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x18, 0x10, "Easy"					},
	{0x16, 0x01, 0x18, 0x18, "Normal"				},
	{0x16, 0x01, 0x18, 0x08, "Hard"					},
	{0x16, 0x01, 0x18, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"	},
	{0x16, 0x01, 0x20, 0x20, "Off"					},
	{0x16, 0x01, 0x20, 0x00, "On"					  },

	{0   , 0xfe, 0   ,    2, "Join In (1 Credit)"	},
	{0x17, 0x01, 0x01, 0x00, "Off"					},
	{0x17, 0x01, 0x01, 0x01, "On"					  },

	{0   , 0xfe, 0   ,    2, "Buy In"				},
	{0x17, 0x01, 0x02, 0x00, "No"					  },
	{0x17, 0x01, 0x02, 0x02, "Yes"					},

	{0   , 0xfe, 0   ,    5, "Country"			},
	{0x18, 0x01, 0x0f, 0x00, "US"					  },
	{0x18, 0x01, 0x0f, 0x01, "Japan"				},
	{0x18, 0x01, 0x0f, 0x02, "Hong-Kong/Taiwan"	},
	{0x18, 0x01, 0x0f, 0x04, "Korea"				},
	{0x18, 0x01, 0x0f, 0x08, "World"				},
};

STDDIPINFO(Taotaido)

static struct BurnDIPInfo TaotaidoaDIPList[]=
{
	// Default Values
	{0x1b, 0xff, 0xff, 0xff, NULL					},
	{0x1c, 0xff, 0xff, 0xff, NULL					},
	{0x1d, 0xff, 0xff, 0xff, NULL					},
	{0x1e, 0xff, 0xff, 0x08, NULL					},

	{0   , 0xfe, 0   ,    2, "Coin Slot"		},
	{0x1b, 0x01, 0x01, 0x01, "Same"					},
	{0x1b, 0x01, 0x01, 0x00, "Individual"		},

	// Coinage condition: Coin Slot Individual
	{0   , 0xfe, 0   ,    8, "Coinage"				},
	{0x1b, 0x02, 0x0e, 0x0a, "3 Coins 1 Credit"			},
	{0x1b, 0x00, 0x01, 0x00, NULL},
	{0x1b, 0x02, 0x0e, 0x0c, "2 Coins 1 Credit"			},
	{0x1b, 0x00, 0x01, 0x00, NULL},
	{0x1b, 0x02, 0x0e, 0x0e, "1 Coin 1 Credit"			},
	{0x1b, 0x00, 0x01, 0x00, NULL},
	{0x1b, 0x02, 0x0e, 0x08, "1 Coin 2 Credits"			},
	{0x1b, 0x00, 0x01, 0x00, NULL},
	{0x1b, 0x02, 0x0e, 0x06, "1 Coin 3 Credits"			},
	{0x1b, 0x00, 0x01, 0x00, NULL},
	{0x1b, 0x02, 0x0e, 0x04, "1 Coin 4 Credits"			},
	{0x1b, 0x00, 0x01, 0x00, NULL},
	{0x1b, 0x02, 0x0e, 0x02, "1 Coin 5 Credits"			},
	{0x1b, 0x00, 0x01, 0x00, NULL},
	{0x1b, 0x02, 0x0e, 0x00, "1 Coin 6 Credits"			},
	{0x1b, 0x00, 0x01, 0x00, NULL},

	// Coin 1 condition: Coin Slot Same
	{0   , 0xfe, 0   ,    8, "Coin 1"				},
	{0x1b, 0x02, 0x0e, 0x0a, "3 Coins 1 Credit"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x0e, 0x0c, "2 Coins 1 Credit"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x0e, 0x0e, "1 Coin 1 Credit"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x0e, 0x08, "1 Coin 2 Credits"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x0e, 0x06, "1 Coin 3 Credits"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x0e, 0x04, "1 Coin 4 Credits"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x0e, 0x02, "1 Coin 5 Credits"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x0e, 0x00, "1 Coin 6 Credits"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},

	// Coin 2 condition: Coin Slot Same
	{0   , 0xfe, 0   ,    8, "Coin 2"				},
	{0x1b, 0x02, 0x70, 0x50, "3 Coins 1 Credit"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x70, 0x60, "2 Coins 1 Credit"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x70, 0x70, "1 Coin 1 Credit"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x70, 0x40, "1 Coin 2 Credits"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x70, 0x30, "1 Coin 3 Credits"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x70, 0x20, "1 Coin 4 Credits"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x70, 0x10, "1 Coin 5 Credits"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},
	{0x1b, 0x02, 0x70, 0x00, "1 Coin 6 Credits"			},
	{0x1b, 0x00, 0x01, 0x01, NULL},

	{0   , 0xfe, 0   ,    2, "2 Coins to Start, 1 to Continue" },
	{0x1b, 0x01, 0x80, 0x80, "Off"				},
	{0x1b, 0x01, 0x80, 0x00, "On"					},

//	{0   , 0xfe, 0   ,    2, "Flip Screen" },
//	{0x1c, 0x01, 0x01, 0x01, "Off"				},
//	{0x1c, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"	},
	{0x1c, 0x01, 0x02, 0x00, "Off"				},
	{0x1c, 0x01, 0x02, 0x02, "On"					},

	{0   , 0xfe, 0   ,    2, "Free Play"	},
	{0x1c, 0x01, 0x04, 0x04, "Off"				},
	{0x1c, 0x01, 0x04, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x1c, 0x01, 0x18, 0x10, "Easy"					},
	{0x1c, 0x01, 0x18, 0x18, "Normal"				},
	{0x1c, 0x01, 0x18, 0x08, "Hard"					},
	{0x1c, 0x01, 0x18, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"	},
	{0x1c, 0x01, 0x20, 0x20, "Off"					},
	{0x1c, 0x01, 0x20, 0x00, "On"					  },

	{0   , 0xfe, 0   ,    2, "Join In (1 Credit)"	},
	{0x1d, 0x01, 0x01, 0x01, "Off"					},
	{0x1d, 0x01, 0x01, 0x00, "On"					  },

	{0   , 0xfe, 0   ,    2, "Buy In"				},
	{0x1d, 0x01, 0x02, 0x00, "No"					  },
	{0x1d, 0x01, 0x02, 0x02, "Yes"					},

	{0   , 0xfe, 0   ,    5, "Country"			},
	{0x1e, 0x01, 0x0f, 0x00, "US"					  },
	{0x1e, 0x01, 0x0f, 0x01, "Japan"				},
	{0x1e, 0x01, 0x0f, 0x02, "Hong-Kong/Taiwan"	},
	{0x1e, 0x01, 0x0f, 0x04, "Korea"				},
	{0x1e, 0x01, 0x0f, 0x08, "World"				},
};

STDDIPINFO(Taotaidoa)

static void palette_write(INT32 offset)
{
	UINT8 r, g, b;
	UINT16 data = *((UINT16*)(DrvPalRAM + offset));

	r = (data >> 10) & 0x1f;
	r = (r << 3) | (r >> 2);
	g = (data >>  5) & 0x1f;
	g = (g << 3) | (g >> 2);
	b = (data >>  0) & 0x1f;
	b = (b << 3) | (b >> 2);

	DrvPalette[offset/2] = BurnHighCol(r, g, b, 0);
}

UINT8 __fastcall taotaido_read_byte(UINT32 address)
{
	if (address < 0xffff00) return 0;

	switch (address)
	{
		case 0xffff80:
		case 0xffff81:
			return DrvInps[0] >> ((~address & 1) << 3);

		case 0xffff82:
		case 0xffff83:
			return DrvInps[1] >> ((~address & 1) << 3);

		case 0xffff84:
		case 0xffff85:
			return DrvInps[2] >> ((~address & 1) << 3);

		case 0xffff87:
			return DrvDip[0];

		case 0xffff89:
			return DrvDip[1];

		case 0xffff8b:
			return DrvDip[2];

		case 0xffff8f:
			return DrvDip[3];

		case 0xffffa0: // p3
		case 0xffffa1:
			return DrvInps[3] >> ((~address & 1) << 3); 

		case 0xffffa2: // p4
		case 0xffffa3:
			return DrvInps[4] >> ((~address & 1) << 3);

		case 0xffffe1:
			return *pending_command;
	}

	return 0;
}

UINT16 __fastcall taotaido_read_word(UINT32 address)
{
	if (address < 0xffff00) return 0;

	switch (address)
	{
		case 0xffff80:
			return DrvInps[0];

		case 0xffff82:
			return DrvInps[1];

		case 0xffff84:
			return DrvInps[2];

		case 0xffffa0: // p3
			return DrvInps[3];

		case 0xffffa2: // p4
			return DrvInps[4];
	}

	return 0;
}

void __fastcall taotaido_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xfff000) == 0xffc000) {
		DrvPalRAM[(address & 0xfff) ^ 1] = data;
		palette_write(address & 0xffe);
		return;
	}

	if (address < 0xffff00) return;

	switch (address)
	{
		case 0xffff08:
		case 0xffff09:
		case 0xffff0a:
		case 0xffff0b:
		case 0xffff0c:
		case 0xffff0d:
		case 0xffff0e:
		case 0xffff0f:
			taotaido_tileregs[address & 0x07] = data;
		return;

		case 0xffff40:
		case 0xffff41:
		case 0xffff42:
		case 0xffff43:
		case 0xffff44:
		case 0xffff45:
		case 0xffff46:
		case 0xffff47:
			taotaido_spritebank[address & 7] = data;
		return;

		case 0xffffc1:
		{
			INT32 nCycles = ((INT64)SekTotalCycles() * nCyclesTotal[1] / nCyclesTotal[0]);
			if (nCycles <= ZetTotalCycles()) return;

			BurnTimerUpdate(nCycles);

			*pending_command = 1;
			*soundlatch = data;
			ZetNmi();
		}
		return;
	}
}

void __fastcall taotaido_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfff000) == 0xffc000) {
		*((UINT16*)(DrvPalRAM + (address & 0xffe))) = data;
		palette_write(address & 0xffe);
		return;
	}		

	if (address < 0xffff00) return;

	switch (address)
	{
		case 0xffff08:
		case 0xffff0a:
		case 0xffff0c:
		case 0xffff0e:
			*((UINT16*)(taotaido_tileregs + (address & 0x06))) = data;
		return;

		case 0xffff40:
		case 0xffff42:
		case 0xffff44:
		case 0xffff46:
			*((UINT16*)(taotaido_spritebank + (address & 6))) = data;
		return;
	}
}

static void taotaido_sound_bankswitch(INT32 data)
{
	*nDrvZ80Bank = data & 3;

	ZetMapArea(0x8000, 0xffff, 0, DrvZ80ROM + 0x8000 * *nDrvZ80Bank);
	ZetMapArea(0x8000, 0xffff, 2, DrvZ80ROM + 0x8000 * *nDrvZ80Bank);
}

void __fastcall taotaido_sound_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			BurnYM2610Write(port & 3, data);
		return;

		case 0x04:
			if (*nDrvZ80Bank != (data & 3)) {
				taotaido_sound_bankswitch(data);
			}
		return;

		case 0x08:
			*pending_command = 0;
		return;
	}
}

UINT8 __fastcall taotaido_sound_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return BurnYM2610Read(0);


		case 0x02:
			return BurnYM2610Read(2);

		case 0x0c:
			return *soundlatch;
	}

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[ 4] = {
		0x000, 0x001, 0x002, 0x003
	};

	INT32 XOffs[16] = {
		0x004, 0x000, 0x00c, 0x008, 0x014, 0x010, 0x01c, 0x018,
		0x024, 0x020, 0x02c, 0x028, 0x034, 0x030, 0x03c, 0x038
	};

	INT32 YOffs[16] = {
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0
	};

	UINT8 *tmp = (UINT8*)BurnMalloc(0x600000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x600000);

	GfxDecode(0xc000, 4, 16, 16, Plane, XOffs, YOffs, 0x400, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x200000);

	GfxDecode(0x4000, 4, 16, 16, Plane, XOffs, YOffs, 0x400, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	taotaido_sound_bankswitch(1);
	ZetClose();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x100000;
	DrvZ80ROM		= Next; Next += 0x020000;

	DrvGfxROM0		= Next; Next += 0xc00000;
	DrvGfxROM1		= Next; Next += 0x400000;

	DrvSndROM0		= Next; Next += 0x100000;
	DrvSndROM1		= Next; Next += 0x200000;

	DrvPalette		= (UINT32*)Next; Next += 0x0800 * sizeof(UINT32);

	AllRam			= Next;

	Drv68KRAM		= Next; Next += 0x010000;
	DrvPalRAM		= Next; Next += 0x001000;
	DrvBgRAM		= Next; Next += 0x004000;
	DrvSprRAM0		= Next; Next += 0x002000;
	DrvSprRAM1		= Next; Next += 0x010000;
	DrvScrollRAM		= Next; Next += 0x000400;
	DrvSpr0Buf0		= Next; Next += 0x002000;
	DrvSpr1Buf0		= Next; Next += 0x010000;
	DrvSpr0Buf1		= Next; Next += 0x002000;
	DrvSpr1Buf1		= Next; Next += 0x010000;

	taotaido_tileregs 	= Next; Next += 0x000010;
	taotaido_spritebank 	= Next; Next += 0x000010;

	pending_command 	= Next; Next += 0x000001;
	soundlatch		= Next; Next += 0x000001;
	nDrvZ80Bank		= Next; Next += 0x000001;

	DrvZ80RAM		= Next; Next += 0x000800;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static void taotaidoFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 taotaidoSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 5000000;
}

static double taotaidoGetTime()
{
	return (double)ZetTotalCycles() / 5000000.0;
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
		if (BurnLoadRom(Drv68KROM  + 0x000000,	0, 1)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080000,	1, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM,		2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,	3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x200000,	4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x400000,	5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,	6, 1)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000,	7, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x000000,	8, 1)) return 1;


		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(DrvBgRAM,		0x800000, 0x803fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,	0xa00000, 0xa01fff, SM_RAM);
	SekMapMemory(DrvSprRAM1,	0xc00000, 0xc0ffff, SM_RAM);
	SekMapMemory(Drv68KRAM,		0xfe0000, 0xfeffff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0xffc000, 0xffcfff, SM_ROM);
	SekMapMemory(DrvScrollRAM,	0xffe000, 0xffe3ff, SM_RAM); 
	SekSetWriteByteHandler(0,	taotaido_write_byte);
	SekSetWriteWordHandler(0,	taotaido_write_word);
	SekSetReadByteHandler(0,	taotaido_read_byte);
	SekSetReadWordHandler(0,	taotaido_read_word);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x77ff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x77ff, 2, DrvZ80ROM);
	ZetMapArea(0x7800, 0x7fff, 0, DrvZ80RAM);
	ZetMapArea(0x7800, 0x7fff, 1, DrvZ80RAM);
	ZetMapArea(0x7800, 0x7fff, 2, DrvZ80RAM);
	ZetSetInHandler(taotaido_sound_in);
	ZetSetOutHandler(taotaido_sound_out);
	ZetMemEnd();
	ZetClose();

	INT32 nDrvSndROM0Size = 0x100000;
	INT32 nDrvSndROM1Size = 0x200000;
	BurnYM2610Init(8000000, DrvSndROM1, &nDrvSndROM1Size, DrvSndROM0, &nDrvSndROM0Size, &taotaidoFMIRQHandler, taotaidoSynchroniseStream, taotaidoGetTime, 0);
	BurnTimerAttachZet(5000000);
	BurnYM2610SetRoute(BURN_SND_YM2610_YM2610_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	BurnYM2610SetRoute(BURN_SND_YM2610_YM2610_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);
	BurnYM2610SetRoute(BURN_SND_YM2610_AY8910_ROUTE, 0.25, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	BurnYM2610Exit();

	SekExit();
	ZetExit();

	BurnFree (AllMem);

	return 0;
}

static void draw_layer()
{
	UINT16 *vram = (UINT16*)DrvBgRAM;
	UINT16 *scroll = (UINT16*)DrvScrollRAM;

	for (INT32 line = 0; line < 224; line++)
	{
		INT32 scrolly = ((scroll[(line << 1) | 1] >> 4) +  0) & 0x3ff;
		INT32 scrollx = ((scroll[(line << 1) | 0] >> 4) + 30) & 0x7ff;

		INT32 sy = scrolly >> 4;

		for (INT32 col = 0; col < (320 / 16) + 1; col++)
		{
			INT32 sx = (scrollx >> 4) + col;
			INT32 ofst = (sy << 6) + (sx & 0x3f) + ((sx & 0x40) << 6);
		
			INT32 code = vram[ofst];
			INT32 bank = (code >> 9) & 7;
			INT32 color = ((code >> 12) << 4) | 0x300;
	
			code = (code & 0x1ff) | (taotaido_tileregs[bank] << 9);
			
			UINT8 *src = DrvGfxROM1 + (code << 8) + ((scrolly & 0x0f) << 4);
		
			for (INT32 x = 0; x < 16; x++) {
				INT32 xx = (x + (col << 4)) - (scrollx & 0x0f);
				if (xx < 0 || xx >= nScreenWidth) continue;
				pTransDraw[(line * nScreenWidth) + xx] = src[x] | color;
			}			
		}
	}
}

static void draw_sprite(INT32 spriteno)
{
	UINT16 *source = (UINT16*)DrvSpr0Buf1;
	UINT16 *source1 = (UINT16*)DrvSpr1Buf1;
	source += spriteno*4;

	INT32 yzoom = (source[0] & 0xf000) >> 12;
	INT32 xzoom = (source[1] & 0xf000) >> 12;

	INT32 ysize = (source[0] & 0x0e00) >> 9;
	INT32 xsize = (source[1] & 0x0e00) >> 9;

	INT32 ypos = source[0] & 0x01ff;
	INT32 xpos = source[1] & 0x01ff;

	INT32 yflip = source[2] & 0x8000;
	INT32 xflip = source[2] & 0x4000;
	INT32 color = (source[2] & 0x1f00) >> 8;

	INT32 tile = source[3] & 0xffff;

	xpos += (xsize*xzoom+2)/4;
	ypos += (ysize*yzoom+2)/4;

	xzoom = 32 - xzoom;
	yzoom = 32 - yzoom;

	for (INT32 y = 0;y <= ysize;y++)
	{
		INT32 sx,sy;

		if (yflip) sy = ((ypos + yzoom * (ysize - y)/2 + 16) & 0x1ff) - 16;
			else sy = ((ypos + yzoom * y / 2 + 16) & 0x1ff) - 16;

		for (INT32 x = 0;x <= xsize;x++)
		{
			INT32 realtile = source1[tile & 0x7fff];

			if (realtile > 0x3fff)
			{
				INT32 block;

				block = (realtile & 0x3800)>>11;

				realtile &= 0x07ff;
				realtile |= taotaido_spritebank[block] * 0x800;
			}

			if (xflip) sx = ((xpos + xzoom * (xsize - x) / 2 + 16) & 0x1ff) - 16;
				else sx = ((xpos + xzoom * x / 2 + 16) & 0x1ff) - 16;

			if (yflip) {
				if (xflip) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, realtile, sx, sy, color, 4, 15, 0, DrvGfxROM0);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, realtile, sx, sy, color, 4, 15, 0, DrvGfxROM0);
				}
			} else {
				if (xflip) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, realtile, sx, sy, color, 4, 15, 0, DrvGfxROM0);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, realtile, sx, sy, color, 4, 15, 0, DrvGfxROM0);
				}
			}

			tile++;
		}
	}
}

static void draw_sprites()
{
	UINT16 *source = (UINT16*)DrvSpr0Buf1;
	UINT16 *finish = source + 0x2000/2;

	while (source < finish)
	{
		if (source[0] == 0x4000) break;

		draw_sprite(source[0] & 0x3ff);

		source++;
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x1000; i+=2) {
			palette_write(i);
		}
	}

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

	SekNewFrame();
	ZetNewFrame();

	{
		memset (DrvInps, 0xff, 5 * sizeof(UINT16));
		for (INT32 i = 0; i < 8; i++) {
			DrvInps[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInps[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInps[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInps[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInps[4] ^= (DrvJoy5[i] & 1) << i;
		}
	}

	nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = (INT32)(5000000.0 / 60.0);

	SekOpen(0);
	ZetOpen(0);

	SekRun(nCyclesTotal[0]);
	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
	
	BurnTimerEndFrame(nCyclesTotal[1]);
	BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);

	ZetClose();
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	memcpy (DrvSpr0Buf1, DrvSpr0Buf0, 0x02000);
	memcpy (DrvSpr1Buf1, DrvSpr1Buf0, 0x10000);
	memcpy (DrvSpr0Buf0, DrvSprRAM0,  0x02000);
	memcpy (DrvSpr1Buf0, DrvSprRAM1,  0x10000);

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029698;
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

		BurnYM2610Scan(nAction, pnMin);
	}

	ZetOpen(0);
	taotaido_sound_bankswitch(*nDrvZ80Bank);
	ZetClose();

	return 0;
}


// Tao Taido (set 1)

static struct BurnRomInfo taotaidoRomDesc[] = {
	{ "1-u90.bin",	0x080000, 0xa3ee30da, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "2-u91.bin",	0x080000, 0x30b7e4fb, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "3-u113.bin",	0x020000, 0xa167c4e4, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "u86.bin",	0x200000, 0x908e251e, 3 | BRF_GRA },           //  3 Sprites
	{ "u87.bin",	0x200000, 0xc4290ba6, 3 | BRF_GRA },           //  4
	{ "u88.bin",	0x200000, 0x407d9aeb, 3 | BRF_GRA },           //  5

	{ "u15.bin",	0x200000, 0xe95823e9, 4 | BRF_GRA },           //  6 Background Tiles

	{ "u104.bin",	0x100000, 0xe89387a9, 5 | BRF_SND },           //  7 Samples
	{ "u127.bin",	0x200000, 0x0cf0cb23, 6 | BRF_SND },           //  8
};

STD_ROM_PICK(taotaido)
STD_ROM_FN(taotaido)

struct BurnDriver BurnDrvTaotaido = {
	"taotaido", NULL, NULL, NULL, "1993",
	"Tao Taido (set 1)\0", NULL, "Video System Co.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MISC, 0,
	NULL, taotaidoRomInfo, taotaidoRomName, NULL, NULL, TaotaidoInputInfo, TaotaidoDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 224, 4, 3
};


// Tao Taido (set 2)

static struct BurnRomInfo taotaidoaRomDesc[] = {
	{ "tt0-u90.bin",	0x080000, 0x69d4cca7, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "tt1-u91.bin",	0x080000, 0x41025469, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "3-u113.bin",		0x020000, 0xa167c4e4, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "u86.bin",		0x200000, 0x908e251e, 3 | BRF_GRA },           //  3 Sprites
	{ "u87.bin",		0x200000, 0xc4290ba6, 3 | BRF_GRA },           //  4
	{ "u88.bin",		0x200000, 0x407d9aeb, 3 | BRF_GRA },           //  5

	{ "u15.bin",		0x200000, 0xe95823e9, 4 | BRF_GRA },           //  6 Background Tiles

	{ "u104.bin",		0x100000, 0xe89387a9, 5 | BRF_SND },           //  7 Samples
	{ "u127.bin",		0x200000, 0x0cf0cb23, 6 | BRF_SND },           //  8
};

STD_ROM_PICK(taotaidoa)
STD_ROM_FN(taotaidoa)

struct BurnDriver BurnDrvTaotaidoa = {
	"taotaidoa", "taotaido", NULL, NULL, "1993",
	"Tao Taido (set 2)\0", NULL, "Video System Co.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, 0,
	NULL, taotaidoaRomInfo, taotaidoaRomName, NULL, NULL, TaotaidoaInputInfo, TaotaidoaDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 224, 4, 3
};
