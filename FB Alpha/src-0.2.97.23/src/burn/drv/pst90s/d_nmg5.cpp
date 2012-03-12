// FB Alpha NMG5 driver module
// Based on MAME driver by Pierpaolo Prazzoli

#include "tiles_generic.h"
#include "burn_ym3812.h"
#include "msm6295.h"

static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvSndROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvBmpRAM;
static UINT8 *DrvVidRAM0;
static UINT8 *DrvVidRAM1;
static UINT16 *DrvScrRAM;
static UINT32 *DrvPalette;
static UINT32 *Palette;
static UINT8 DrvRecalc;

static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvDips[ 2];
static UINT8 DrvReset;
static UINT16 DrvInputs[3];

static UINT8 DrvOkiBank;
static UINT8 soundlatch;
static UINT8 prot_val;
static UINT8 input_data;
static UINT8 priority_reg;
static UINT8 gfx_bank;

static struct BurnInputInfo Nmg5InputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy2 + 0,	 "p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy2 + 1,	 "p2 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy3 + 7,	 "p1 start" },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy3 + 0,    "p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy3 + 1,    "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy3 + 2,    "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy3 + 3,    "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 4,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 5,	 "p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 6,	 "p1 fire 3"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy3 + 15,	 "p2 start" },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy3 + 8,    "p2 up"    },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy3 + 9,    "p2 down"  },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 10,   "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 11,   "p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 12,	 "p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 13,	 "p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 14,	 "p2 fire 3"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	 "reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	 "dip"	    },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	 "dip"	    },
};

STDINPUTINFO(Nmg5)

static struct BurnInputInfo SearcheyInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy2 + 0,	 "p1 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy3 + 7,	 "p1 start" },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy3 + 0,    "p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy3 + 1,    "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy3 + 2,    "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy3 + 3,    "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 4,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 5,	 "p1 fire 2"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy3 + 15,	 "p2 start" },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy3 + 8,    "p2 up"    },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy3 + 9,    "p2 down"  },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 10,   "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 11,   "p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 12,	 "p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 13,	 "p2 fire 2"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	 "reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	 "dip"	    },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	 "dip"	    },
};

STDINPUTINFO(Searchey)

static struct BurnInputInfo PclubysInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy2 + 0,	 "p1 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy3 + 7,	 "p1 start" },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy3 + 0,    "p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy3 + 1,    "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy3 + 2,    "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy3 + 3,    "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 4,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 5,	 "p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 6,	 "p1 fire 3"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy3 + 15,	 "p2 start" },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy3 + 8,    "p2 up"    },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy3 + 9,    "p2 down"  },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 10,   "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 11,   "p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 12,	 "p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 13,	 "p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 14,	 "p2 fire 3"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	 "reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	 "dip"	    },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	 "dip"	    },
};

STDINPUTINFO(Pclubys)

static struct BurnInputInfo GarogunInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy2 + 0,	 "p1 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy3 + 7,	 "p1 start" },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy3 + 0,    "p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy3 + 1,    "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy3 + 2,    "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy3 + 3,    "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 4,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 5,	 "p1 fire 2"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	 "reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	 "dip"	    },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	 "dip"	    },
};

STDINPUTINFO(Garogun)

static struct BurnInputInfo Ordi7InputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy2 + 0,	 "p1 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy3 + 7,	 "p1 start" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 2,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 3,	 "p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 4,	 "p1 fire 3"},
	{"P1 Button 4"  , BIT_DIGITAL  , DrvJoy3 + 1,	 "p1 fire 4"},
	{"P1 Button 5"  , BIT_DIGITAL  , DrvJoy3 + 0,	 "p1 fire 5"},
	{"P1 Button 6"  , BIT_DIGITAL  , DrvJoy3 + 10,	 "p1 fire 6"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	 "reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	 "dip"	    },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	 "dip"	    },
};

STDINPUTINFO(Ordi7)

static struct BurnDIPInfo Nmg5DIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL				},
	{0x14, 0xff, 0xff, 0xdf, NULL				},

	{0x13, 0xfe,    0,    2, "Game Title"			},
	{0x13, 0x01, 0x01, 0x01, "Multi 5"			},
	{0x13, 0x01, 0x01, 0x00, "New Multi Game 5"		},

	{0x13, 0xfe,    0,    4, "Crocodiles (Tong Boy)"	},
	{0x13, 0x01, 0x06, 0x06, "Pattern 1"			},
	{0x13, 0x01, 0x06, 0x02, "Pattern 2"			},
	{0x13, 0x01, 0x06, 0x04, "Pattern 3"			},
	{0x13, 0x01, 0x06, 0x00, "Pattern 4"			},

	{0x13, 0xfe,    0,    4, "License" },
	{0x13, 0x01, 0x18, 0x00, "New Impeuropex Corp. S.R.L."	},
	{0x13, 0x01, 0x18, 0x08, "BNS Enterprises"		},
	{0x13, 0x01, 0x18, 0x10, "Nova Games"			},
	{0x13, 0x01, 0x18, 0x18, "None"				},

	{0x13, 0xfe,    0,    2, "1P Vs 2P Rounds (Bubble Gum)"	},
	{0x13, 0x01, 0x20, 0x20, "Best of 1"			},
	{0x13, 0x01, 0x20, 0x00, "Best of 3"			},

	{0x13, 0xfe,    0,    4, "Difficulty"			},
	{0x13, 0x01, 0xc0, 0x40, "Easy"				},
	{0x13, 0x01, 0xc0, 0xc0, "Normal"			},
	{0x13, 0x01, 0xc0, 0x80, "Hard"				},
	{0x13, 0x01, 0xc0, 0x00, "Hardest"			},

	{0x14, 0xfe,    0,    4, "Coin B"			},
	{0x14, 0x01, 0x03, 0x02, "2C 1C"			},
	{0x14, 0x01, 0x03, 0x03, "1C 1C"			},
	{0x14, 0x01, 0x03, 0x00, "2C 3C"			},
	{0x14, 0x01, 0x03, 0x01, "1C 2C"			},
//	{0x14, 0x01, 0x03, 0x03, "1C 1C"			},
//	{0x14, 0x01, 0x03, 0x01, "1C 3C"			},
//	{0x14, 0x01, 0x03, 0x02, "1C 4C"			},
//	{0x14, 0x01, 0x03, 0x00, "1C 6C"			},

	{0x14, 0xfe,    0,    4, "Coin A"			},
	{0x14, 0x01, 0x0c, 0x08, "2C 1C"			},
	{0x14, 0x01, 0x0c, 0x0c, "1C 1C"			},
	{0x14, 0x01, 0x0c, 0x00, "2C 3C"			},
	{0x14, 0x01, 0x0c, 0x04, "1C 2C"			},
//	{0x14, 0x01, 0x0c, 0x00, "4C 1C"			},
//	{0x14, 0x01, 0x0c, 0x08, "3C 1C"			},
//	{0x14, 0x01, 0x0c, 0x04, "2C 1C"			},
//	{0x14, 0x01, 0x0c, 0x0c, "1C 1C"			},

	{0x14, 0xfe,    0,    2, "Demo Sounds"			},
	{0x14, 0x01, 0x20, 0x20, "Off"				},
	{0x14, 0x01, 0x20, 0x00, "On"				},

	{0x14, 0xfe,    0,    2, "Coin Type"			},
	{0x14, 0x01, 0x40, 0x40, "1"				},
	{0x14, 0x01, 0x40, 0x00, "2"				},

	{0x14, 0xfe,    0,    2, "Service Mode"			},
	{0x14, 0x01, 0x80, 0x00, "Off"				},
	{0x14, 0x01, 0x80, 0x80, "On"				},
};

STDDIPINFO(Nmg5)

static struct BurnDIPInfo SearcheyDIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL				},
	{0x11, 0xff, 0xff, 0xdd, NULL				},

	{0   , 0xfe,    0,    4, "Timer Speed"			},
	{0x10, 0x01, 0x03, 0x03, "Slowest"			},
	{0x10, 0x01, 0x03, 0x02, "Slow"				},
	{0x10, 0x01, 0x03, 0x01, "Fast"				},
	{0x10, 0x01, 0x03, 0x00, "Fastest"			},

	{0x10, 0xfe,    0,    4, "Helps"			},
	{0x10, 0x01, 0x0c, 0x0c, "1"				},
	{0x10, 0x01, 0x0c, 0x08, "2"				},
	{0x10, 0x01, 0x0c, 0x04, "3"				},
	{0x10, 0x01, 0x0c, 0x00, "4"				},

	{0x10, 0xfe,    0,    4, "Lives"			},
	{0x10, 0x01, 0x30, 0x20, "3"				},
	{0x10, 0x01, 0x30, 0x10, "4"				},
	{0x10, 0x01, 0x30, 0x30, "5"				},
	{0x10, 0x01, 0x30, 0x00, "6"				},

	{0x10, 0xfe,    0,    4, "Coinage"			},
	{0x10, 0x01, 0xc0, 0x00, "3C 1C"			},
	{0x10, 0x01, 0xc0, 0x40, "2C 1C"			},
	{0x10, 0x01, 0xc0, 0xc0, "1C 1C"			},
	{0x10, 0x01, 0xc0, 0x80, "1C 2C"			},

	{0x11, 0xfe,    0,    2, "Service Mode"			},
	{0x11, 0x01, 0x01, 0x01, "Off"				},
	{0x11, 0x01, 0x01, 0x00, "On"				},

	{0x11, 0xfe,    0,    2, "Demo Sounds"			},
	{0x11, 0x01, 0x02, 0x02, "Off"				},
	{0x11, 0x01, 0x02, 0x00, "On"				},

	{0x11, 0xfe,    0,    2, "Language"			},
	{0x11, 0x01, 0x20, 0x00, "English"			},
	{0x11, 0x01, 0x20, 0x20, "Korean"			},

	{0x11, 0xfe,    0,    2, "Items to find"		},
	{0x11, 0x01, 0x80, 0x00, "Less"				},
	{0x11, 0x01, 0x80, 0x80, "More"				},
};

STDDIPINFO(Searchey)

static struct BurnDIPInfo Searchp2DIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL				},
	{0x11, 0xff, 0xff, 0xf7, NULL				},

	{0x10, 0xfe,    0,    4, "Timer Speed"			},
	{0x10, 0x01, 0x03, 0x03, "Slowest"			},
	{0x10, 0x01, 0x03, 0x02, "Slow"				},
	{0x10, 0x01, 0x03, 0x01, "Fast"				},
	{0x10, 0x01, 0x03, 0x00, "Fastest"			},

	{0x10, 0xfe,    0,    4, "Helps"			},
	{0x10, 0x01, 0x0c, 0x0c, "1"				},
	{0x10, 0x01, 0x0c, 0x08, "2"				},
	{0x10, 0x01, 0x0c, 0x04, "3"				},
	{0x10, 0x01, 0x0c, 0x00, "4"				},

	{0x10, 0xfe,    0,    4, "Lives"			},
	{0x10, 0x01, 0x30, 0x20, "3"				},
	{0x10, 0x01, 0x30, 0x10, "4"				},
	{0x10, 0x01, 0x30, 0x30, "5"				},
	{0x10, 0x01, 0x30, 0x00, "6"				},

	{0x10, 0xfe,    0,    4, "Coinage"			},
	{0x10, 0x01, 0xc0, 0x00, "3C 1C"			},
	{0x10, 0x01, 0xc0, 0x40, "2C 1C"			},
	{0x10, 0x01, 0xc0, 0xc0, "1C 1C"			},
	{0x10, 0x01, 0xc0, 0x80, "1C 2C"			},

	{0x11, 0xfe,    0,    2, "Service Mode"			},
	{0x11, 0x01, 0x01, 0x01, "Off"				},
	{0x11, 0x01, 0x01, 0x00, "On"				},

	{0x11, 0xfe,    0,    4, "Lucky Chance"			},
	{0x11, 0x01, 0x06, 0x06, "Table 1"			},
	{0x11, 0x01, 0x06, 0x04, "Table 2"			},
	{0x11, 0x01, 0x06, 0x02, "Table 3"			},
	{0x11, 0x01, 0x06, 0x00, "Table 4"			},

	{0x11, 0xfe,    0,    8, "Language"			},
	{0x11, 0x01, 0x38, 0x38, "Korean"			},
	{0x11, 0x01, 0x38, 0x30, "English"			},
	{0x11, 0x01, 0x38, 0x28, "Japanese"			},
	{0x11, 0x01, 0x38, 0x20, "Chinese"			},
	{0x11, 0x01, 0x38, 0x18, "Italian"			},
	{0x11, 0x01, 0x38, 0x10, "Korean"			},
	{0x11, 0x01, 0x38, 0x08, "Korean"			},
	{0x11, 0x01, 0x38, 0x00, "Korean"			},

	{0x11, 0xfe,    0,    2, "Lucky Timer"			},
	{0x11, 0x01, 0x40, 0x00, "Less"				},
	{0x11, 0x01, 0x40, 0x40, "More"				},

	{0x11, 0xfe,    0,    2, "Items to find"		},
	{0x11, 0x01, 0x80, 0x00, "Less"				},
	{0x11, 0x01, 0x80, 0x80, "More"				},
};

STDDIPINFO(Searchp2)

static struct BurnDIPInfo PclubysDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL				},
	{0x13, 0xff, 0xff, 0xff, NULL				},

	{0x12, 0xfe,    0,    2, "Lives (Mad Ball)"		},
	{0x12, 0x01, 0x01, 0x00, "3"				},
	{0x12, 0x01, 0x01, 0x01, "4"				},

	{0x12, 0xfe,    0,    2, "Demo Sounds"			},
	{0x12, 0x01, 0x02, 0x00, "Off"				},
	{0x12, 0x01, 0x02, 0x02, "On"				},

	{0x12, 0xfe,    0,    2, "1P Vs 2P Rounds"		},
	{0x12, 0x01, 0x04, 0x00, "Best of 1"			},
	{0x12, 0x01, 0x04, 0x04, "Best of 3"			},

	{0x12, 0xfe,    0,    2, "Nudity"			},
	{0x12, 0x01, 0x08, 0x08, "Soft only"			},
	{0x12, 0x01, 0x08, 0x00, "Soft and Hard"		},

	{0x12, 0xfe,    0,    4, "Difficulty"			},
	{0x12, 0x01, 0x30, 0x30, "Easy"				},
	{0x12, 0x01, 0x30, 0x20, "Normal"			},
	{0x12, 0x01, 0x30, 0x10, "Hard"				},
	{0x12, 0x01, 0x30, 0x00, "Hardest"			},

	{0x12, 0xfe,    0,    4, "Coinage"			},
	{0x12, 0x01, 0xc0, 0x00, "3C 1C"			},
	{0x12, 0x01, 0xc0, 0x40, "2C 1C"			},
	{0x12, 0x01, 0xc0, 0xc0, "1C 1C"			},
	{0x12, 0x01, 0xc0, 0x80, "1C 2C"			},

	{0x13, 0xfe,    0,    2, "Service Mode" 		},
	{0x13, 0x01, 0x01, 0x01, "Off"				},
	{0x13, 0x01, 0x01, 0x00, "On"				},

	{0x13, 0xfe,    0,    4, "Timer Speed (Magic Eye)"	},
	{0x13, 0x01, 0x30, 0x30, "Slowest"			},
	{0x13, 0x01, 0x30, 0x20, "Slow"				},
	{0x13, 0x01, 0x30, 0x10, "Fast"				},
	{0x13, 0x01, 0x30, 0x00, "Fastest"			},

	{0x13, 0xfe,    0,    2, "Lives (Magic Eye)"		},
	{0x13, 0x01, 0x40, 0x00, "3"				},
	{0x13, 0x01, 0x40, 0x40, "4"				},

	{0x13, 0xfe,    0,    2, "Timer Speed (Box Logic)"	},
	{0x13, 0x01, 0x80, 0x80, "Slow"				},
	{0x13, 0x01, 0x80, 0x00, "Fast"				},
};

STDDIPINFO(Pclubys)

static struct BurnDIPInfo GarogunDIPList[]=
{
	{0x09, 0xff, 0xff, 0xff, NULL				},
	{0x0a, 0xff, 0xff, 0xfd, NULL				},

	{0x09, 0xfe,    0,    4, "Helps"			},
	{0x09, 0x01, 0x03, 0x00, "1"				},
	{0x09, 0x01, 0x03, 0x01, "2"				},
	{0x09, 0x01, 0x03, 0x02, "3"				},
	{0x09, 0x01, 0x03, 0x03, "4"				},

	{0x09, 0xfe,    0,    4, "Timer Speed (Bonus Levels)"	},
	{0x09, 0x01, 0x0c, 0x0c, "Slowest"			},
	{0x09, 0x01, 0x0c, 0x08, "Slow"				},
	{0x09, 0x01, 0x0c, 0x04, "Fast"				},
	{0x09, 0x01, 0x0c, 0x00, "Fastest"			},

	{0x09, 0xfe,    0,    4, "Timer Speed (Main Levels)"	},
	{0x09, 0x01, 0x30, 0x30, "Slowest"			},
	{0x09, 0x01, 0x30, 0x20, "Slow"				},
	{0x09, 0x01, 0x30, 0x10, "Fast"				},
	{0x09, 0x01, 0x30, 0x00, "Fastest"			},

	{0x09, 0xfe,    0,    4, "Coinage"			},
	{0x09, 0x01, 0xc0, 0x00, "3C 1C"			},
	{0x09, 0x01, 0xc0, 0x40, "2C 1C"			},
	{0x09, 0x01, 0xc0, 0xc0, "1C 1C"			},
	{0x09, 0x01, 0xc0, 0x80, "1C 2C"			},

	{0x0a, 0xfe,    0,    2, "Service Mode"			},
	{0x0a, 0x01, 0x01, 0x01, "Off"				},
	{0x0a, 0x01, 0x01, 0x00, "On"				},

	{0x0a, 0xfe,    0,    2, "Demo Sounds"			},
	{0x0a, 0x01, 0x02, 0x02, "Off"				},
	{0x0a, 0x01, 0x02, 0x00, "On"				},
};

STDDIPINFO(Garogun)

static struct BurnDIPInfo Ordi7DIPList[]=
{
	{0x09, 0xff, 0xff, 0xff, NULL				},
	{0x0a, 0xff, 0xff, 0xff, NULL				},

	{0x09, 0xfe,    0,    2, "High-Low Error"		},
	{0x09, 0x01, 0x01, 0x01, "-500"				},
	{0x09, 0x01, 0x01, 0x00, "Lose All"			},

	{0x09, 0xfe,    0,    2, "Minimum Credits"		},
	{0x09, 0x01, 0x02, 0x02, "300"				},
	{0x09, 0x01, 0x02, 0x00, "500"				},

	{0x09, 0xfe,    0,    4, "Credit Limit"			},
	{0x09, 0x01, 0x0c, 0x0c, "10000"			},
	{0x09, 0x01, 0x0c, 0x08, "30000"			},
	{0x09, 0x01, 0x0c, 0x04, "50000"			},
	{0x09, 0x01, 0x0c, 0x00, "100000"			},

	{0x09, 0xfe,    0,    4, "Bet"				},
	{0x09, 0x01, 0x30, 0x30, "50 Credits"			},
	{0x09, 0x01, 0x30, 0x20, "100 Credits"			},
	{0x09, 0x01, 0x30, 0x10, "150 Credits"			},
	{0x09, 0x01, 0x30, 0x00, "200 Credits"			},

	{0x09, 0xfe,    0,    4, "Coinage"			},
	{0x09, 0x01, 0xc0, 0xc0, "1 Coin 50 Credits"		},
	{0x09, 0x01, 0xc0, 0x80, "1 Coin 100 Credits"		},
	{0x09, 0x01, 0xc0, 0x40, "1 Coin 500 Credits"		},
	{0x09, 0x01, 0xc0, 0x00, "1 Coin 1000 Credits"		},

	{0x0a, 0xfe,    0,    2, "Service Mode"			},
	{0x0a, 0x01, 0x01, 0x01, "Off"				},
	{0x0a, 0x01, 0x01, 0x00, "On"				},

	{0x0a, 0xfe,    0,    2, "Winnings"			},
	{0x0a, 0x01, 0x04, 0x04, "Medals and Credits"		},
	{0x0a, 0x01, 0x04, 0x00, "Credits only"			},

	{0x0a, 0xfe,    0,    4, "Medals Table"			},
	{0x0a, 0x01, 0x18, 0x18, "x1.0"				},
	{0x0a, 0x01, 0x18, 0x10, "x1.4"				},
	{0x0a, 0x01, 0x18, 0x08, "x2.0"				},
	{0x0a, 0x01, 0x18, 0x00, "x3.0"				},

	{0x0a, 0xfe,    0,    8, "Payout %"			},
	{0x0a, 0x01, 0xe0, 0xe0, "90%"				},
	{0x0a, 0x01, 0xe0, 0xc0, "85%"				},
	{0x0a, 0x01, 0xe0, 0xa0, "80%"				},
	{0x0a, 0x01, 0xe0, 0x80, "75%"				},
	{0x0a, 0x01, 0xe0, 0x60, "70%"				},
	{0x0a, 0x01, 0xe0, 0x40, "65%"				},
	{0x0a, 0x01, 0xe0, 0x20, "60%"				},
	{0x0a, 0x01, 0xe0, 0x00, "55%"				},
};

STDDIPINFO(Ordi7)

static struct BurnDIPInfo WondstckDIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL				},
	{0x11, 0xff, 0xff, 0xfd, NULL				},

	{0x10, 0xfe,    0,    2, "Helps"			},
	{0x10, 0x01, 0x10, 0x10, "3"				},
	{0x10, 0x01, 0x10, 0x00, "5"				},

	{0x10, 0xfe,    0,    2, "Lives"			},
	{0x10, 0x01, 0x20, 0x20, "3"				},
	{0x10, 0x01, 0x20, 0x00, "5"				},

	{0x10, 0xfe,    0,    4, "Coinage"			},
	{0x10, 0x01, 0xc0, 0x00, "3C 1C"			},
	{0x10, 0x01, 0xc0, 0x40, "2C 1C"			},
	{0x10, 0x01, 0xc0, 0xc0, "1C 1C"			},
	{0x10, 0x01, 0xc0, 0x80, "1C 2C"			},

	{0x11, 0xfe,    0,    2, "Service Mode"			},
	{0x11, 0x01, 0x01, 0x01, "Off"				},
	{0x11, 0x01, 0x01, 0x00, "On"				},

	{0x11, 0xfe,    0,    2, "Demo Sounds"			},
	{0x11, 0x01, 0x02, 0x02, "Off"				},
	{0x11, 0x01, 0x02, 0x00, "On"				},

	{0x11, 0xfe,    0,    4, "Difficulty"			},
	{0x11, 0x01, 0xc0, 0x80, "Easy"				},
	{0x11, 0x01, 0xc0, 0xc0, "Normal"			},
	{0x11, 0x01, 0xc0, 0x40, "Hard"				},
	{0x11, 0x01, 0xc0, 0x00, "Hardest"			},
};

STDDIPINFO(Wondstck)

//-----------------------------------------------------------------------------------------------

static void palette_write(INT32 offset)
{
	INT32 data = *((UINT16*)(DrvPalRAM + offset));

	INT32 r = (data >>  0) & 0x1f;
	INT32 g = (data >>  5) & 0x1f;
	INT32 b = (data >> 10) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	Palette[offset>>1] = (r << 16) | (g << 8) | b;
	DrvPalette[offset>>1] = BurnHighCol(r, g, b, 0);
}

void __fastcall nmg5_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xfffff800) == 0x140000) {
		DrvPalRAM[address & 0x7ff] = data;
		palette_write(address & 0x7ff);
		return;
	}

	switch (address)
	{
		case 0x180000:
		case 0x180001:
			soundlatch = data;
			ZetNmi();
		return;

		case 0x180004:
		case 0x180005:
			input_data = data & 0xf;
		return;

		case 0x180006:
		case 0x180007:
			gfx_bank = data & 3;
		return;

		case 0x18000e:
		case 0x18000f:
			priority_reg = data & 7;
		return;

		case 0x300002:
		case 0x300003:
		case 0x300004:
		case 0x300005:
		case 0x300006:
		case 0x300007:
		case 0x300008:
		case 0x300009:
			*((UINT8*)(DrvScrRAM + ((address - 2) & 7))) = data;
		return;
	}

	return;
}

void __fastcall nmg5_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffff800) == 0x140000) {
		*((UINT16*)(DrvPalRAM + (address & 0x7ff))) = data;
		palette_write(address & 0x7ff);
		return;
	}

	switch (address)
	{
		case 0x180000:
			soundlatch = data;
			ZetNmi();
		return;

		case 0x180004:
			input_data = data & 0xf;
		return;

		case 0x180006:
			gfx_bank = data & 3;
		return;

		case 0x18000e:
			priority_reg = data & 7;
		return;

		case 0x300002:
		case 0x300004:
		case 0x300006:
		case 0x300008:
			DrvScrRAM[(address - 0x300002) >> 1] = data & 0x1ff;
		return;
	}

	return;
}

UINT8 __fastcall nmg5_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x180004:
		case 0x180005:
			return prot_val | input_data;

		case 0x180008:
			return DrvInputs[0] >> 8;

		case 0x180009:
			return DrvInputs[0];

		case 0x18000a:
			return DrvInputs[1] >> 8;

		case 0x18000b:
			return DrvInputs[1] & ~0x50;

		case 0x18000c:
			return DrvInputs[2] >> 8;

		case 0x18000d:
			return DrvInputs[2];
	}

	return 0;
}

UINT16 __fastcall nmg5_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x180004:
			return prot_val | input_data;

		case 0x180008:
			return DrvInputs[0];

		case 0x18000a:
			return DrvInputs[1] & ~0x50;

		case 0x18000c:
			return DrvInputs[2];
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------

void __fastcall pclubys_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xfffff800) == 0x440000) {
		DrvPalRAM[address & 0x7ff] = data;
		palette_write(address & 0x7ff);
		return;
	}

	switch (address)
	{
		case 0x480000:
		case 0x480001:
			soundlatch = data;
			ZetNmi();
		return;

		case 0x480004:
		case 0x480005:
			input_data = data & 0xf;
		return;

		case 0x480006:
		case 0x480007:
			gfx_bank = data & 3;
		return;

		case 0x48000e:
		case 0x48000f:
			priority_reg = data & 7;
		return;

		case 0x500002:
		case 0x500003:
		case 0x500004:
		case 0x500005:
		case 0x500006:
		case 0x500007:
		case 0x500008:
		case 0x500009:
			*((UINT8*)(DrvScrRAM + ((address - 0x500002) & 7))) = data;
		return;
	}

	return;
}

void __fastcall pclubys_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffff800) == 0x440000) {
		*((UINT16*)(DrvPalRAM + (address & 0x7ff))) = data;
		palette_write(address & 0x7ff);
		return;
	}

	switch (address)
	{
		case 0x480000:
			soundlatch = data;
			ZetNmi();
		return;

		case 0x480004:
			input_data = data & 0xf;
		return;

		case 0x480006:
			gfx_bank = data & 3;
		return;

		case 0x48000e:
			priority_reg = data & 7;
		return;

		case 0x500002:
		case 0x500004:
		case 0x500006:
		case 0x500008:
			DrvScrRAM[(address - 0x500002) >> 1] = data;
		return;
	}

	return;
}

UINT8 __fastcall pclubys_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x480004:
		case 0x480005:
			return prot_val | input_data;

		case 0x480008:
			return DrvInputs[0] >> 8;

		case 0x480009:
			return DrvInputs[0];

		case 0x48000a:
			return DrvInputs[1] >> 8;

		case 0x48000b:
			return DrvInputs[1];

		case 0x48000c:
			return DrvInputs[2] >> 8;

		case 0x48000d:
			return DrvInputs[2];
	}

	return 0;
}

UINT16 __fastcall pclubys_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x480004:
			return prot_val | input_data;

		case 0x480008:
			return DrvInputs[0];

		case 0x48000a:
			return DrvInputs[1];

		case 0x48000c:
			return DrvInputs[2];
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------

void __fastcall nmg5_write_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			DrvOkiBank = data & 1;
			MSM6295ROM = DrvSndROM + (DrvOkiBank ? 0x40000 : 0);
		return;

		case 0x10:
			BurnYM3812Write(0, data);
		return;

		case 0x11:
			BurnYM3812Write(1, data);
		return;

		case 0x1c:
			MSM6295Command(0, data);
		return;
	}

	return;
}

UINT8 __fastcall nmg5_read_port(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x10:
			return BurnYM3812Read(0);

		case 0x18:
			return soundlatch;

		case 0x1c:
			return MSM6295ReadStatus(0);
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x0100000;
	DrvZ80ROM	= Next; Next += 0x0010000;

	DrvGfxROM0	= Next; Next += 0x1000000;
	DrvGfxROM1	= Next; Next += 0x0400000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x0080000;

	DrvPalette	= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);
	
	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x0010000;
	DrvZ80RAM	= Next; Next += 0x0000800;

	DrvPalRAM	= Next; Next += 0x0000800;

	DrvSprRAM	= Next; Next += 0x0000800;

	DrvVidRAM0	= Next; Next += 0x0002000;
	DrvVidRAM1	= Next; Next += 0x0002000;

	DrvBmpRAM	= Next; Next += 0x0010000;

	DrvScrRAM	= (UINT16*)Next; Next += 0x0000004 * sizeof(UINT16);

	Palette		= (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	priority_reg = 7;

	soundlatch = 0;
	input_data = 0;
	gfx_bank = 0;
	DrvOkiBank = 0;
	MSM6295ROM = DrvSndROM;

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	MSM6295Reset(0);
	BurnYM3812Reset();

	return 0;
}

static INT32 DrvGfxDecode(INT32 type)
{
	static INT32 Planes0[8] = {
		0x1C00000, 0x1800000, 0x1400000, 0x1000000,
		0x0c00000, 0x0800000, 0x0400000, 0x0000000
	};

	static INT32 XOffs0[8] = {
		0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007
	};

	static INT32 YOffs0[8] = {
		0x000, 0x008, 0x010, 0x018, 0x020, 0x028, 0x030, 0x038
	};

	static INT32 Planes0a[8] = {
		0x2000008, 0x4000000, 0x0000008, 0x0000000,
		0x6000008, 0x6000000, 0x4000008, 0x2000000
	};

	static INT32 XOffs0a[8] = {
		0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007
	};

	static INT32 YOffs0a[8] = {
		0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070
	};

	static INT32 Planes1[5] = {
		0x0800000, 0x0c00000, 0x0400000, 0x1000000, 0x0000000
	};

	static INT32 XOffs1[16] = {
		0x007, 0x006, 0x005, 0x004, 0x003, 0x002, 0x001, 0x000,
		0x087, 0x086, 0x085, 0x084, 0x083, 0x082, 0x081, 0x080
	};

	static INT32 YOffs1[16] = {
		0x000, 0x008, 0x010, 0x018, 0x020, 0x028, 0x030, 0x038,
		0x040, 0x048, 0x050, 0x058, 0x060, 0x068, 0x070, 0x078
	};

	UINT8 *tmp = (UINT8*)BurnMalloc(0x1000000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x1000000);

	if (type) {
		GfxDecode(0x40000, 8,  8,  8, Planes0a, XOffs0a, YOffs0a, 0x080, tmp, DrvGfxROM0);
	} else {
		GfxDecode(0x10000, 8,  8,  8, Planes0,  XOffs0,  YOffs0,  0x040, tmp, DrvGfxROM0);
	}

	memcpy (tmp, DrvGfxROM1, 0x400000);

	GfxDecode(0x04000, 5, 16, 16, Planes1, XOffs1, YOffs1, 0x100, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static void DrvFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvInit(INT32 loadtype, INT32 sektype, INT32 zettype) // 0 nmg, 1 pclubys
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 1, 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0, 1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,      2, 1)) return 1;

		if (loadtype) {
			for (INT32 i = 0; i < 4; i++) {
				if (BurnLoadRom(DrvGfxROM0 + i * 0x400000, i +  3, 1)) return 1;
			}

			for (INT32 i = 0; i < 5; i++) {
				if (BurnLoadRom(DrvGfxROM1 + i * 0x080000, i +  7, 1)) return 1;
			}

			if (BurnLoadRom(DrvSndROM,     12, 1)) return 1;
		} else {
			for (INT32 i = 0; i < 8; i++) {
				if (BurnLoadRom(DrvGfxROM0 + i * 0x80000, i +  3, 1)) return 1;
			}

			for (INT32 i = 0; i < 5; i++) {
				if (BurnLoadRom(DrvGfxROM1 + i * 0x80000, i + 11, 1)) return 1;
			}

			if (BurnLoadRom(DrvSndROM,     16, 1)) return 1;
		}

		DrvGfxDecode(loadtype);
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(DrvBmpRAM,		0x800000, 0x80ffff, SM_RAM);
	if (sektype) {
		SekMapMemory(Drv68KRAM,		0x200000, 0x20ffff, SM_RAM);
		SekMapMemory(DrvPalRAM,		0x440000, 0x4407ff, SM_ROM);
		SekMapMemory(DrvSprRAM,		0x460000, 0x4607ff, SM_RAM);
		SekMapMemory(DrvVidRAM0,	0x520000, 0x521fff, SM_RAM);
		SekMapMemory(DrvVidRAM1,	0x522000, 0x523fff, SM_RAM);
		SekSetWriteByteHandler(0,	pclubys_write_byte);
		SekSetWriteWordHandler(0,	pclubys_write_word);
		SekSetReadByteHandler(0,	pclubys_read_byte);
		SekSetReadWordHandler(0,	pclubys_read_word);
	} else {
		SekMapMemory(Drv68KRAM,		0x120000, 0x12ffff, SM_RAM);
		SekMapMemory(DrvPalRAM,		0x140000, 0x1407ff, SM_ROM);
		SekMapMemory(DrvSprRAM,		0x160000, 0x1607ff, SM_RAM);
		SekMapMemory(DrvVidRAM0,	0x320000, 0x321fff, SM_RAM);
		SekMapMemory(DrvVidRAM1,	0x322000, 0x323fff, SM_RAM);
		SekSetWriteByteHandler(0,	nmg5_write_byte);
		SekSetWriteWordHandler(0,	nmg5_write_word);
		SekSetReadByteHandler(0,	nmg5_read_byte);
		SekSetReadWordHandler(0,	nmg5_read_word);
	}
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	if (zettype) {
		ZetMapArea(0x0000, 0xf7ff, 0, DrvZ80ROM);
		ZetMapArea(0x0000, 0xf7ff, 2, DrvZ80ROM);
		ZetMapArea(0xf800, 0xffff, 0, DrvZ80RAM);
		ZetMapArea(0xf800, 0xffff, 1, DrvZ80RAM);
		ZetMapArea(0xf800, 0xffff, 2, DrvZ80RAM);
	} else {
		ZetMapArea(0x0000, 0xdfff, 0, DrvZ80ROM);
		ZetMapArea(0x0000, 0xdfff, 2, DrvZ80ROM);
		ZetMapArea(0xe000, 0xe7ff, 0, DrvZ80RAM);
		ZetMapArea(0xe000, 0xe7ff, 1, DrvZ80RAM);
		ZetMapArea(0xe000, 0xe7ff, 2, DrvZ80RAM);
	}
	ZetSetInHandler(nmg5_read_port);
	ZetSetOutHandler(nmg5_write_port);
	ZetMemEnd();
	ZetClose();

	BurnYM3812Init(4000000, &DrvFMIRQHandler, &DrvSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(4000000);

	MSM6295Init(0, 1000000 / 132, 100.0, 1);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	MSM6295Exit(0);
	BurnYM3812Exit();

	GenericTilesExit();
	ZetExit();
	SekExit();

	BurnFree (AllMem);

	return 0;
}

//-----------------------------------------------------------------------------------------------

static void draw_bitmap()
{
	UINT16 x,y,count;
	INT32 pix;

	UINT16 *bitmap = (UINT16*)DrvBmpRAM;

	for (y = 0, count = 0; y < 256; y++)
	{
		for(x = 0; x < 128; x++, count++)
		{
			if (!bitmap[count]) continue;

			INT32 ofst = ((y - 9) * nScreenWidth) + ((x << 2) - 12);

			pix = bitmap[count] >> 12;
			if (pix) pTransDraw[ofst    ] = pix | 0x300;

			pix = (bitmap[count]&0x0f00)>>8;
			if (pix) pTransDraw[ofst | 1] = pix | 0x300;

			pix = (bitmap[count]&0x00f0)>>4;
			if (pix) pTransDraw[ofst | 2] = pix | 0x300;

			pix = bitmap[count] & 0x000f;
			if (pix) pTransDraw[ofst | 3] = pix | 0x300;
		}
	}
}

static void DrawTiles(UINT8 *vidram, INT32 color, INT32 transp, INT32 scrollx, INT32 scrolly)
{
	UINT16 *vram = (UINT16*)vidram;

	for (INT32 offs = 0; offs < 0x2000 / 2; offs++)
	{
		INT32 sx    = (offs & 0x3f) << 3;
		INT32 sy    = (offs >> 6) << 3;

		INT32 code  = vram[offs] | (gfx_bank << 16);

		sx -= scrollx;
		sy -= scrolly;
		if (sx < -7) sx += 0x200;
		if (sy < -7) sy += 0x200;

		if (transp) {
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 8, 0, 0, DrvGfxROM0);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 8, 0, DrvGfxROM0);
		}
	}
}

static void draw_sprites()
{
	UINT16 *spriteram16 = (UINT16*)DrvSprRAM;

	for (INT32 offs = 0;offs < 0x800/2;offs += 4)
	{
		INT32 sx     = spriteram16[offs + 2] & 0x1ff;
		INT32 sy     = spriteram16[offs];
		INT32 code   = spriteram16[offs + 1];
		INT32 color  = (spriteram16[offs + 2] >> 9) & 0xf;
		INT32 height = 1 << ((sy & 0x0600) >> 9);
		INT32 flipx  = sy & 0x2000;
		INT32 flipy  = sy & 0x4000;

		for (INT32 y = 0;y < height;y++)
		{
			INT32 yy = 248 - ((sy + ((height - y) << 4)) & 0x1ff);

			INT32 tile = code + (flipy ? height-1-y : y);

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, tile, sx, yy, color, 5, 0, 0x200, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, tile, sx, yy, color, 5, 0, 0x200, DrvGfxROM1);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, tile, sx, yy, color, 5, 0, 0x200, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, tile, sx, yy, color, 5, 0, 0x200, DrvGfxROM1);
				}
			}

			if (sx < 0x1f1) continue;

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, tile, sx - 512, yy, color, 5, 0, 0x200, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, tile, sx - 512, yy, color, 5, 0, 0x200, DrvGfxROM1);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, tile, sx - 512, yy, color, 5, 0, 0x200, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, tile, sx - 512, yy, color, 5, 0, 0x200, DrvGfxROM1);
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

	DrawTiles(DrvVidRAM0, 1, 0, DrvScrRAM[2]+3, DrvScrRAM[3]+9);

	if(priority_reg == 0)
	{
		draw_sprites();
		DrawTiles(DrvVidRAM1, 0, 1, DrvScrRAM[0]-1, DrvScrRAM[1]+9);
		draw_bitmap();
	}
	else if(priority_reg == 1)
	{
		draw_bitmap();
		draw_sprites();
		DrawTiles(DrvVidRAM1, 0, 1, DrvScrRAM[0]-1, DrvScrRAM[1]+9);
	}
	else if(priority_reg == 2)
	{
		draw_sprites();
		draw_bitmap();
		DrawTiles(DrvVidRAM1, 0, 1, DrvScrRAM[0]-1, DrvScrRAM[1]+9);
	}
	else if(priority_reg == 3)
	{
		DrawTiles(DrvVidRAM1, 0, 1, DrvScrRAM[0]-1, DrvScrRAM[1]+9);
		draw_sprites();
		draw_bitmap();
	}
	else if(priority_reg == 7)
	{
		DrawTiles(DrvVidRAM1, 0, 1, DrvScrRAM[0]-1, DrvScrRAM[1]+9);
		draw_bitmap();
		draw_sprites();
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}



static INT32 DrvFrame()
{
	INT32 nTotalCycles[2] = { 16000000 / 60, 4000000 / 60 };

	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 6);

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[1] ^= DrvJoy2[i] << i;
			DrvInputs[2] ^= DrvJoy3[i] << i;
		}

		DrvInputs[0] = (DrvDips[1] << 8) | DrvDips[0];
	}

	SekNewFrame();
	ZetNewFrame();

	SekOpen(0);
	ZetOpen(0);

	SekRun(nTotalCycles[0]);

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		BurnTimerEndFrameYM3812(nTotalCycles[1]);
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------

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
		SekScan(nAction);
		ZetScan(nAction);
		BurnYM3812Scan(nAction, pnMin);
		MSM6295Scan(0, nAction);
//		BurnTimerScan(nAction, pnMin);

		SCAN_VAR(soundlatch);
		SCAN_VAR(prot_val);
		SCAN_VAR(input_data);
		SCAN_VAR(priority_reg);
		SCAN_VAR(gfx_bank);
		SCAN_VAR(DrvOkiBank);
	}
	
	if (nAction & ACB_WRITE) {
		nmg5_write_port(0, DrvOkiBank);
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------

// Multi 5 / New Multi Game 5

static struct BurnRomInfo nmg5RomDesc[] = {
	{ "ub15.bin",	0x80000, 0x36af3e2f, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "ub16.bin",	0x80000, 0x2d9923d4, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "xh15.bin",	0x10000, 0x12d047c4, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "srom1.bin",	0x80000, 0x6771b694, 3 | BRF_GRA },	      //  3 - Tiles
	{ "srom2.bin",	0x80000, 0x362d33af, 3 | BRF_GRA },	      //  4
	{ "srom3.bin",	0x80000, 0x8bad69d1, 3 | BRF_GRA },	      //  5
	{ "srom4.bin",	0x80000, 0xe73a7fcb, 3 | BRF_GRA },	      //  6
	{ "srom5.bin",	0x80000, 0x7300494e, 3 | BRF_GRA },	      //  7
	{ "srom6.bin",	0x80000, 0x74b5fdf9, 3 | BRF_GRA },	      //  8
	{ "srom7.bin",	0x80000, 0xbd2b9036, 3 | BRF_GRA },	      //  9
	{ "srom8.bin",	0x80000, 0xdd38360e, 3 | BRF_GRA },	      // 10

	{ "uf1.bin",	0x40000, 0x9a9fb6f4, 4 | BRF_GRA },	      // 11 - Sprites
	{ "uf2.bin",	0x40000, 0x66954d63, 4 | BRF_GRA },	      // 12 
	{ "ufa1.bin",	0x40000, 0xba73ed2d, 4 | BRF_GRA },	      // 13 
	{ "uh1.bin",	0x40000, 0xf7726e8e, 4 | BRF_GRA },	      // 14 
	{ "uj1.bin",	0x40000, 0x54f7486e, 4 | BRF_GRA },	      // 15 

	{ "xra1.bin",	0x20000, 0xc74a4f3e, 5 | BRF_SND },	      // 16 - Samples
};

STD_ROM_PICK(nmg5)
STD_ROM_FN(nmg5)

static INT32 Nmg5Init()
{
	prot_val = 0x10;

	return DrvInit(0,0,0);
}

struct BurnDriver BurnDrvNmg5 = {
	"nmg5", NULL, NULL, NULL, "1998",
	"Multi 5 / New Multi Game 5\0", NULL, "Yun Sung", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, nmg5RomInfo, nmg5RomName, NULL, NULL, Nmg5InputInfo, Nmg5DIPInfo,
	Nmg5Init, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x400, 320, 240, 4, 3
};


// Multi 5 / New Multi Game 5 (earlier)

static struct BurnRomInfo nmg5eRomDesc[] = {
	{ "ub15.rom",	0x80000, 0x578516e2, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "ub16.rom",	0x80000, 0x12fab483, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "xh15.bin",	0x10000, 0x12d047c4, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "srom1.rom",	0x80000, 0x6df3e0c2, 3 | BRF_GRA },	      //  3 - Tiles
	{ "srom2.rom",	0x80000, 0x3caf65f9, 3 | BRF_GRA },	      //  4
	{ "srom3.rom",	0x80000, 0x812f3f87, 3 | BRF_GRA },	      //  5
	{ "srom4.rom",	0x80000, 0xedb8299d, 3 | BRF_GRA },	      //  6
	{ "srom5.rom",	0x80000, 0x79821f18, 3 | BRF_GRA },	      //  7
	{ "srom6.rom",	0x80000, 0x7e37abaf, 3 | BRF_GRA },	      //  8
	{ "srom7.rom",	0x80000, 0xb7a9c660, 3 | BRF_GRA },	      //  9
	{ "srom8.rom",	0x80000, 0xd7ba6058, 3 | BRF_GRA },	      // 10

	{ "uf1.rom",	0x40000, 0x502dbd65, 4 | BRF_GRA },	      // 11 - Sprites
	{ "uf2.rom",	0x40000, 0x6744cca0, 4 | BRF_GRA },	      // 12
	{ "ufa1.rom",	0x40000, 0x7110677f, 4 | BRF_GRA },	      // 13
	{ "uh1.rom",	0x40000, 0xf6a3ef4d, 4 | BRF_GRA },	      // 14
	{ "uj1.rom",	0x40000, 0x0595d8ef, 4 | BRF_GRA },	      // 15

	{ "xra1.bin",	0x20000, 0xc74a4f3e, 5 | BRF_SND },	      // 16 - Samples
};

STD_ROM_PICK(nmg5e)
STD_ROM_FN(nmg5e)

struct BurnDriver BurnDrvNmg5e = {
	"nmg5e", "nmg5", NULL, NULL, "1997",
	"Multi 5 / New Multi Game 5 (earlier)\0", NULL, "Yun Sung", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, nmg5eRomInfo, nmg5eRomName, NULL, NULL, Nmg5InputInfo, Nmg5DIPInfo,
	Nmg5Init, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x400, 320, 240, 4, 3
};


// Search Eye

static struct BurnRomInfo searcheyRomDesc[] = {
	{ "u7.bin",	0x40000, 0x287ce3dd, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "u2.bin",	0x40000, 0xb574f033, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u128.bin",	0x10000, 0x85bae10c, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "u63.bin",	0x80000, 0x1b0b7b7d, 3 | BRF_GRA },	      //  3 - Tiles
	{ "u68.bin",	0x80000, 0xae18b2aa, 3 | BRF_GRA },	      //  4
	{ "u73.bin",	0x80000, 0xab7f8716, 3 | BRF_GRA },	      //  5
	{ "u79.bin",	0x80000, 0x7f2c8b83, 3 | BRF_GRA },	      //  6
	{ "u64.bin",	0x80000, 0x322a903c, 3 | BRF_GRA },	      //  7
	{ "u69.bin",	0x80000, 0xd546eaf8, 3 | BRF_GRA },	      //  8
	{ "u74.bin",	0x80000, 0xe6134d84, 3 | BRF_GRA },	      //  9
	{ "u80.bin",	0x80000, 0x9a160918, 3 | BRF_GRA },	      // 10

	{ "u83.bin",	0x20000, 0xc5a1c647, 4 | BRF_GRA },	      // 11 - Sprites
	{ "u82.bin",	0x20000, 0x25b2ae62, 4 | BRF_GRA },	      // 12
	{ "u105.bin",	0x20000, 0xb4207ef0, 4 | BRF_GRA },	      // 13
	{ "u96.bin",	0x20000, 0x8c40818a, 4 | BRF_GRA },	      // 14
	{ "u97.bin",	0x20000, 0x5dc7f231, 4 | BRF_GRA },	      // 15

	{ "u137.bin",	0x40000, 0x49105e23, 5 | BRF_SND },	      // 16 - Samples
};

STD_ROM_PICK(searchey)
STD_ROM_FN(searchey)

struct BurnDriver BurnDrvSearchey = {
	"searchey", NULL, NULL, NULL, "1999",
	"Search Eye\0", NULL, "Yun Sung", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, searcheyRomInfo, searcheyRomName, NULL, NULL, SearcheyInputInfo, SearcheyDIPInfo,
	Nmg5Init, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x400, 320, 240, 4, 3
};


// Search Eye Plus V2.0

static struct BurnRomInfo searchp2RomDesc[] = {
	{ "u7",		0x080000, 0x37fe9e18, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "u2",		0x080000, 0x8278513b, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u128",	0x010000, 0x85bae10c, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "0.u1",	0x400000, 0x28a50dcf, 3 | BRF_GRA },	       //  3 - Tiles
	{ "2.u3",	0x400000, 0x30d46e19, 3 | BRF_GRA },	       //  4
	{ "1.u2",	0x400000, 0xf9c4e824, 3 | BRF_GRA },	       //  5
	{ "3.u4",	0x400000, 0x619f142f, 3 | BRF_GRA },	       //  6

	{ "u83",	0x040000, 0x2bae34cb, 4 | BRF_GRA },	       //  7 - Sprites
	{ "u82",	0x040000, 0x5cb773f0, 4 | BRF_GRA },	       //  8
	{ "u105",	0x040000, 0xe8adb15e, 4 | BRF_GRA },	       //  9
	{ "u96",	0x040000, 0x67efb536, 4 | BRF_GRA },	       // 10
	{ "u97",	0x040000, 0xf7b63826, 4 | BRF_GRA },	       // 11

	{ "u137", 	0x040000, 0xcd037524, 5 | BRF_SND },	       // 12 - Samples
};

STD_ROM_PICK(searchp2)
STD_ROM_FN(searchp2)

static INT32 Searchp2Init()
{
	prot_val = 0x10;

	BurnSetRefreshRate(55);

	return DrvInit(1,0,0);
}

struct BurnDriver BurnDrvSearchp2 = {
	"searchp2", NULL, NULL, NULL, "1999",
	"Search Eye Plus V2.0\0", NULL, "Yun Sung", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, searchp2RomInfo, searchp2RomName, NULL, NULL, SearcheyInputInfo, Searchp2DIPInfo,
	Searchp2Init, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x400, 320, 240, 4, 3
};


// Puzzle Club (Yun Sung, set 1)

static struct BurnRomInfo pclubysRomDesc[] = {
	{ "rom3.7",	0x080000, 0x62e28e6d, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "rom4.2",	0x080000, 0xb51dab41, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "rom1.128",	0x010000, 0x25cd27f8, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "rom10.167",	0x400000, 0xd67e8e84, 3 | BRF_GRA },	       //  3 - Tiles
	{ "rom12.165",	0x400000, 0x6be8b733, 3 | BRF_GRA },	       //  4
	{ "rom11.166",	0x400000, 0x672501a4, 3 | BRF_GRA },	       //  5
	{ "rom13.164",	0x400000, 0xfc725ce7, 3 | BRF_GRA },	       //  6

	{ "rom8.83",	0x080000, 0x651af101, 4 | BRF_GRA },	       //  7 - Sprites
	{ "rom9.82",	0x080000, 0x2535b4d6, 4 | BRF_GRA },	       //  8
	{ "rom7.105",	0x080000, 0xf7536c52, 4 | BRF_GRA },	       //  9
	{ "rom6.95",	0x080000, 0x3c078a52, 4 | BRF_GRA },	       // 10
	{ "rom5.97",	0x080000, 0x20eae2f8, 4 | BRF_GRA },	       // 11

	{ "rom2.137",	0x080000, 0x4ff97ad1, 4 | BRF_SND },	       // 12 - Samples
};

STD_ROM_PICK(pclubys)
STD_ROM_FN(pclubys)

static INT32 PclubysInit()
{
	prot_val = 0x10;

	return DrvInit(1,1,1);
}

struct BurnDriver BurnDrvPclubys = {
	"pclubys", NULL, NULL, NULL, "2000",
	"Puzzle Club (Yun Sung, set 1)\0", NULL, "Yun Sung", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, pclubysRomInfo, pclubysRomName, NULL, NULL, PclubysInputInfo, PclubysDIPInfo,
	PclubysInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x400, 320, 240, 4, 3
};


// Puzzle Club (Yun Sung, set 2)

static struct BurnRomInfo pclubysaRomDesc[] = {
	{ "rom3a.7",	0x080000, 0x885aa07a, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "rom4a.2",	0x080000, 0x9bfbdeac, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "rom1.128",	0x010000, 0x25cd27f8, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "rom10.167",	0x400000, 0xd67e8e84, 3 | BRF_GRA },	       //  3 - Tiles
	{ "rom12.165",	0x400000, 0x6be8b733, 3 | BRF_GRA },	       //  4
	{ "rom11.166",	0x400000, 0x672501a4, 3 | BRF_GRA },	       //  5
	{ "rom13.164",	0x400000, 0xfc725ce7, 3 | BRF_GRA },	       //  6

	{ "rom8.83",	0x080000, 0x651af101, 4 | BRF_GRA },	       //  7 - Sprites
	{ "rom9.82",	0x080000, 0x2535b4d6, 4 | BRF_GRA },	       //  8
	{ "rom7.105",	0x080000, 0xf7536c52, 4 | BRF_GRA },	       //  9
	{ "rom6.95",	0x080000, 0x3c078a52, 4 | BRF_GRA },	       // 10
	{ "rom5.97",	0x080000, 0x20eae2f8, 4 | BRF_GRA },	       // 11

	{ "rom2.137",	0x080000, 0x4ff97ad1, 4 | BRF_SND },	       // 12 - Samples
};

STD_ROM_PICK(pclubysa)
STD_ROM_FN(pclubysa)

struct BurnDriver BurnDrvPclubysa = {
	"pclubysa", "pclubys", NULL, NULL, "2000",
	"Puzzle Club (Yun Sung, set 2)\0", NULL, "Yun Sung", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, pclubysaRomInfo, pclubysaRomName, NULL, NULL, PclubysInputInfo, PclubysDIPInfo,
	PclubysInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x400, 320, 240, 4, 3
};


// Garogun Seroyang (Korea)

static struct BurnRomInfo garogunRomDesc[] = {
	{ "p1.u7",	0x80000, 0x9b5627f8, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "p2.u2",	0x80000, 0x1d2ff271, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "rom.u128",	0x10000, 0x117b31ce, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "8.u63",	0x80000, 0x2d152d32, 3 | BRF_GRA },	      //  3 - Tiles
	{ "11.u68",	0x80000, 0x60ec7f67, 3 | BRF_GRA },	      //  4
	{ "9.u73",	0x80000, 0xa4b16319, 3 | BRF_GRA },	      //  5
	{ "13.u79",	0x80000, 0x2dc14fb6, 3 | BRF_GRA },	      //  6
	{ "6.u64",	0x80000, 0xa0fc7547, 3 | BRF_GRA },	      //  7
	{ "10.u69",	0x80000, 0xe5dc36c3, 3 | BRF_GRA },	      //  8
	{ "7.u74",	0x80000, 0xa0574f8d, 3 | BRF_GRA },	      //  9
	{ "12.u80",	0x80000, 0x94d66169, 3 | BRF_GRA },	      // 10

	{ "4.u83",	0x40000, 0x3d1d46ff, 4 | BRF_GRA },	      // 11 - Sprites
	{ "5.u82",	0x40000, 0x2a7b2fb5, 4 | BRF_GRA },	      // 12
	{ "3.u105",	0x40000, 0xcd20e39c, 4 | BRF_GRA },	      // 13
	{ "2.u96",	0x40000, 0x4df3b502, 4 | BRF_GRA },	      // 14
	{ "1.u97",	0x40000, 0x591b3efe, 4 | BRF_GRA },	      // 15

	{ "s.u137",	0x80000, 0x3eadc21a, 5 | BRF_SND },	      // 16 - Samples
};

STD_ROM_PICK(garogun)
STD_ROM_FN(garogun)

static INT32 GarogunInit()
{
	prot_val = 0x40;

	return DrvInit(0,1,1);
}

struct BurnDriver BurnDrvGarogun = {
	"garogun", NULL, NULL, NULL, "2000",
	"Garogun Seroyang (Korea)\0", NULL, "Yun Sung", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_MISC_POST90S, GBF_QUIZ, 0,
	NULL, garogunRomInfo, garogunRomName, NULL, NULL, GarogunInputInfo, GarogunDIPInfo,
	GarogunInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x400, 320, 240, 4, 3
};


// 7 Ordi (Korea)

static struct BurnRomInfo ordi7RomDesc[] = {
	{ "p1.u7",	0x20000, 0xebf21862, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "p2.u2",	0x20000, 0xf7943a6a, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "4.u128",	0x10000, 0xed73b565, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "8.u63",	0x80000, 0xed8dfe5d, 3 | BRF_GRA },	      //  3 - Tiles
	{ "11.u68",	0x80000, 0x742764a7, 3 | BRF_GRA },	      //  4
	{ "9.u73",	0x80000, 0x2b76efd0, 3 | BRF_GRA },	      //  5
	{ "13.u79", 	0x80000, 0x3892b356, 3 | BRF_GRA },	      //  6
	{ "6.u64",	0x80000, 0x5c0b0838, 3 | BRF_GRA },	      //  7
	{ "10.u69",	0x80000, 0xc15db1a4, 3 | BRF_GRA },	      //  8
	{ "7.u74",	0x80000, 0x6910f754, 3 | BRF_GRA },	      //  9
	{ "12.u80",	0x80000, 0x4c5dd9ef, 3 | BRF_GRA },	      // 10

	{ "4.u83",	0x80000, 0xa2569cf4, 4 | BRF_GRA },	      // 11 - Sprites
	{ "5.u82",	0x80000, 0x045e548e, 4 | BRF_GRA },	      // 12
	{ "3.u105",	0x80000, 0x04c1dbf9, 4 | BRF_GRA },	      // 13
	{ "2.u96",	0x80000, 0x11fa7de8, 4 | BRF_GRA },	      // 14
	{ "1.u97",	0x80000, 0xcd1ffe88, 4 | BRF_GRA },	      // 15

	{ "3.u137", 	0x20000, 0x669ed310, 5 | BRF_SND },	      // 16 - Samples
};

STD_ROM_PICK(ordi7)
STD_ROM_FN(ordi7)

static INT32 Ordi7Init()
{
	INT32 nRet;

	prot_val = 0x20;

	nRet = DrvInit(0,0,1);

	if (nRet == 0) {
		memcpy (DrvSndROM + 0x20000, DrvSndROM, 0x20000);
		memcpy (DrvSndROM + 0x40000, DrvSndROM, 0x20000);
		memcpy (DrvSndROM + 0x50000, DrvSndROM, 0x20000);
	}

	return nRet;
}

struct BurnDriver BurnDrvOrdi7 = {
	"7ordi", NULL, NULL, NULL, "2002",
	"7 Ordi (Korea)\0", NULL, "Yun Sung", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_MISC_POST90S, GBF_CASINO, 0,
	NULL, ordi7RomInfo, ordi7RomName, NULL, NULL, Ordi7InputInfo, Ordi7DIPInfo,
	Ordi7Init, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x400, 320, 240, 4, 3
};


// Wonder Stick

static struct BurnRomInfo wondstckRomDesc[] = {
	{ "u4.bin",	0x20000, 0x46a3e9f6, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "u2.bin",	0x20000, 0x9995b743, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u128.bin",	0x10000, 0x86dba085, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "u63.bin",	0x80000, 0xc6cf09b4, 3 | BRF_GRA },	      //  3 - Tiles
	{ "u68.bin",	0x80000, 0x2e9e9a5e, 3 | BRF_GRA },	      //  4
	{ "u73.bin",	0x80000, 0x3a828604, 3 | BRF_GRA },	      //  5
	{ "u79.bin",	0x80000, 0x0cca46af, 3 | BRF_GRA },	      //  6
	{ "u64.bin",	0x80000, 0xdcec9ac5, 3 | BRF_GRA },	      //  7
	{ "u69.bin",	0x80000, 0x27b9d708, 3 | BRF_GRA },	      //  8
	{ "u74.bin", 	0x80000, 0x7eff8e2f, 3 | BRF_GRA },	      //  9
	{ "u80.bin", 	0x80000, 0x1160a0c2, 3 | BRF_GRA },	      // 10

	{ "u83.bin",	0x80000, 0xf51cf9c6, 4 | BRF_GRA },	      // 11 - Sprites
	{ "u82.bin",	0x80000, 0xddd3c60c, 4 | BRF_GRA },	      // 12
	{ "u105.bin",	0x80000, 0xa7fc624d, 4 | BRF_GRA },	      // 13
	{ "u96.bin",	0x80000, 0x2369d8a3, 4 | BRF_GRA },	      // 14
	{ "u97.bin",	0x80000, 0xaba1bd94, 4 | BRF_GRA },	      // 15

	{ "u137.bin",	0x40000, 0x294b6cbd, 5 | BRF_SND },	      // 16 - Samples
};

STD_ROM_PICK(wondstck)
STD_ROM_FN(wondstck)

static INT32 WondstckInit()
{
	prot_val = 0x00;

	return DrvInit(0,0,0);
}

struct BurnDriver BurnDrvWondstck = {
	"wondstck", NULL, NULL, NULL, "????",
	"Wonder Stick\0", NULL, "Yun Sung", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, wondstckRomInfo, wondstckRomName, NULL, NULL, SearcheyInputInfo, WondstckDIPInfo,
	WondstckInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x400, 320, 240, 4, 3
};
