#include "tiles_generic.h"
#include "burn_ym2151.h"
#include "msm6295.h"

static UINT8 DrvInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort3[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[4]        = {0, 0, 0, 0};
static UINT8 DrvInput[4]      = {0x00, 0x00, 0x00, 0x00};
static UINT8 DrvReset         = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *Drv68KRom           = NULL;
static UINT8 *Drv68KRam           = NULL;
static UINT8 *DrvMSM6295ROMSrc    = NULL;
static UINT8 *DrvFgVideoRam       = NULL;
static UINT8 *DrvBgVideoRam       = NULL;
static UINT8 *DrvZ80Rom           = NULL;
static UINT8 *DrvZ80Ram           = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvPaletteRam       = NULL;
static UINT8 *DrvTiles            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette         = NULL;

static UINT16 DrvBgTileBase;
static UINT16 DrvBgScrollX;
static UINT16 DrvBgScrollY;
static UINT16 DrvFgScrollX;
static UINT16 DrvFgScrollY;
static UINT16 DrvVReg;
static UINT8 DrvSoundLatch;
static UINT8 DrvOkiBank;
static UINT8 DrvVBlank;

typedef void (*Render)();
static Render DrawFunction;
static void DrvDraw();
static void CtribeDraw();

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort2 + 7, "p2 start"  },
	{"Start 3"           , BIT_DIGITAL  , DrvInputPort3 + 7, "p3 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 fire 3" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , DrvInputPort2 + 6, "p2 fire 3" },
	
	{"P3 Up"             , BIT_DIGITAL  , DrvInputPort3 + 2, "p3 up"     },
	{"P3 Down"           , BIT_DIGITAL  , DrvInputPort3 + 3, "p3 down"   },
	{"P3 Left"           , BIT_DIGITAL  , DrvInputPort3 + 1, "p3 left"   },
	{"P3 Right"          , BIT_DIGITAL  , DrvInputPort3 + 0, "p3 right"  },
	{"P3 Fire 1"         , BIT_DIGITAL  , DrvInputPort3 + 4, "p3 fire 1" },
	{"P3 Fire 2"         , BIT_DIGITAL  , DrvInputPort3 + 5, "p3 fire 2" },
	{"P3 Fire 3"         , BIT_DIGITAL  , DrvInputPort3 + 6, "p3 fire 3" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort1 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
	{"Dip 4"             , BIT_DIPSWITCH, DrvDip + 3       , "dip"       },
};

STDINPUTINFO(Drv)

static inline void DrvClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

static inline void DrvMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = DrvInput[1] = DrvInput[2] = DrvInput[3] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] |= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] |= (DrvInputPort2[i] & 1) << i;
		DrvInput[3] |= (DrvInputPort3[i] & 1) << i;
	}

	// Clear Opposites
	DrvClearOpposites(&DrvInput[0]);
	DrvClearOpposites(&DrvInput[2]);
	DrvClearOpposites(&DrvInput[3]);
}

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x1c, 0xff, 0xff, 0xff, NULL                     },
	{0x1d, 0xff, 0xff, 0xff, NULL                     },
	{0x1e, 0xff, 0xff, 0xff, NULL                     },
	{0x1f, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x1c, 0x01, 0x03, 0x00, "3 Coins 1 Credit"       },
	{0x1c, 0x01, 0x03, 0x01, "2 Coins 1 Credit"       },
	{0x1c, 0x01, 0x03, 0x03, "1 Coin  1 Credit"       },
	{0x1c, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Continue Discount"      },
	{0x1c, 0x01, 0x10, 0x10, "Off"                    },
	{0x1c, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x1c, 0x01, 0x20, 0x00, "Off"                    },
	{0x1c, 0x01, 0x20, 0x20, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1c, 0x01, 0x40, 0x40, "Off"                    },
	{0x1c, 0x01, 0x40, 0x00, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x1d, 0x01, 0x03, 0x02, "Easy"                   },
	{0x1d, 0x01, 0x03, 0x03, "Normal"                 },
	{0x1d, 0x01, 0x03, 0x01, "Hard"                   },
	{0x1d, 0x01, 0x03, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Player vs Player Damage"},
	{0x1d, 0x01, 0x04, 0x04, "Off"                    },
	{0x1d, 0x01, 0x04, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Test Mode"              },
	{0x1d, 0x01, 0x10, 0x10, "Off"                    },
	{0x1d, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Stage Clear Energy"     },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "50"                     },
	
	{0   , 0xfe, 0   , 2   , "Starting Energy"        },
	{0x1d, 0x01, 0x40, 0x00, "200"                    },
	{0x1d, 0x01, 0x40, 0x40, "230"                    },
	
	{0   , 0xfe, 0   , 2   , "Number of Players"      },
	{0x1d, 0x01, 0x80, 0x80, "2"                      },
	{0x1d, 0x01, 0x80, 0x00, "3"                      },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo DrvbDIPList[]=
{
	// Default Values
	{0x1c, 0xff, 0xff, 0x10, NULL                     },
	{0x1d, 0xff, 0xff, 0xff, NULL                     },
	{0x1e, 0xff, 0xff, 0xff, NULL                     },
	{0x1f, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1c, 0x01, 0x10, 0x10, "Off"                    },
	{0x1c, 0x01, 0x10, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x1d, 0x01, 0x03, 0x00, "3 Coins 1 Credit"       },
	{0x1d, 0x01, 0x03, 0x01, "2 Coins 1 Credit"       },
	{0x1d, 0x01, 0x03, 0x03, "1 Coin  1 Credit"       },
	{0x1d, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Continue Discount"      },
	{0x1d, 0x01, 0x10, 0x10, "Off"                    },
	{0x1d, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x1e, 0x01, 0x03, 0x02, "Easy"                   },
	{0x1e, 0x01, 0x03, 0x03, "Normal"                 },
	{0x1e, 0x01, 0x03, 0x01, "Hard"                   },
	{0x1e, 0x01, 0x03, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Player vs Player Damage"},
	{0x1e, 0x01, 0x04, 0x04, "Off"                    },
	{0x1e, 0x01, 0x04, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Test Mode"              },
	{0x1e, 0x01, 0x10, 0x10, "Off"                    },
	{0x1e, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Stage Clear Energy"     },
	{0x1e, 0x01, 0x20, 0x00, "Off"                    },
	{0x1e, 0x01, 0x20, 0x20, "50"                     },
	
	// Dip 4
	{0   , 0xfe, 0   , 2   , "Starting Energy"        },
	{0x1f, 0x01, 0x01, 0x00, "200"                    },
	{0x1f, 0x01, 0x01, 0x01, "230"                    },
	
	{0   , 0xfe, 0   , 2   , "Number of Players"      },
	{0x1f, 0x01, 0x02, 0x02, "2"                      },
	{0x1f, 0x01, 0x02, 0x00, "3"                      },
};

STDDIPINFO(Drvb)

static struct BurnDIPInfo CtribeDIPList[]=
{
	// Default Values
	{0x1c, 0xff, 0xff, 0x10, NULL                     },
	{0x1d, 0xff, 0xff, 0xff, NULL                     },
	{0x1e, 0xff, 0xff, 0xff, NULL                     },
	{0x1f, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1c, 0x01, 0x10, 0x10, "Off"                    },
	{0x1c, 0x01, 0x10, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x1d, 0x01, 0x03, 0x00, "3 Coins 1 Credit"       },
	{0x1d, 0x01, 0x03, 0x01, "2 Coins 1 Credit"       },
	{0x1d, 0x01, 0x03, 0x03, "1 Coin  1 Credit"       },
	{0x1d, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Continue Discount"      },
	{0x1d, 0x01, 0x10, 0x10, "Off"                    },
	{0x1d, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x1d, 0x01, 0x20, 0x00, "Off"                    },
	{0x1d, 0x01, 0x20, 0x20, "On"                     },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x1e, 0x01, 0x03, 0x02, "Easy"                   },
	{0x1e, 0x01, 0x03, 0x03, "Normal"                 },
	{0x1e, 0x01, 0x03, 0x01, "Hard"                   },
	{0x1e, 0x01, 0x03, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Timer Speed"            },
	{0x1e, 0x01, 0x04, 0x04, "Normal"                 },
	{0x1e, 0x01, 0x04, 0x00, "Fast"                   },
	
	{0   , 0xfe, 0   , 2   , "FBI Logo"               },
	{0x1e, 0x01, 0x08, 0x00, "Off"                    },
	{0x1e, 0x01, 0x08, 0x08, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Test Mode"              },
	{0x1e, 0x01, 0x10, 0x10, "Off"                    },
	{0x1e, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Stage Clear Energy"     },
	{0x1e, 0x01, 0x20, 0x20, "0"                      },
	{0x1e, 0x01, 0x20, 0x00, "50"                     },
	
	// Dip 4
	{0   , 0xfe, 0   , 2   , "More Stage Clear Energy"},
	{0x1f, 0x01, 0x01, 0x01, "Off"                    },
	{0x1f, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Number of Players"      },
	{0x1f, 0x01, 0x02, 0x02, "2"                      },
	{0x1f, 0x01, 0x02, 0x00, "3"                      },
};

STDDIPINFO(Ctribe)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "30a14-0.ic78",  0x40000, 0xf42fe016, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "30a15-0.ic79",  0x20000, 0xad50e92c, BRF_ESS | BRF_PRG }, //	 1
	
	{ "30a13-0.ic43",  0x10000, 0x1e974d9b, BRF_ESS | BRF_PRG }, //  2	Z80 Program 
	
	{ "30j-7.ic4",     0x40000, 0x89d58d32, BRF_GRA },	     //  3	Tiles
	{ "30j-6.ic5",     0x40000, 0x9bf1538e, BRF_GRA },	     //  4
	{ "30j-5.ic6",     0x40000, 0x8f671a62, BRF_GRA },	     //  5
	{ "30j-4.ic7",     0x40000, 0x0f74ea1c, BRF_GRA },	     //  6
	
	{ "30j-3.ic9",     0x80000, 0xb3151871, BRF_GRA },	     //  7	Sprites
	{ "30a12-0.ic8",   0x10000, 0x20d64bea, BRF_GRA },	     //  8
	{ "30j-2.ic11",    0x80000, 0x41c6fb08, BRF_GRA },	     //  9
	{ "30a11-0.ic10",  0x10000, 0x785d71b0, BRF_GRA },	     //  10
	{ "30j-1.ic13",    0x80000, 0x67a6f114, BRF_GRA },	     //  11
	{ "30a10-0.ic12",  0x10000, 0x15e43d12, BRF_GRA },	     //  12
	{ "30j-0.ic15",    0x80000, 0xf15dafbe, BRF_GRA },	     //  13
	{ "30a9-0.ic14",   0x10000, 0x5a47e7a4, BRF_GRA },	     //  14
	
	{ "30j-8.ic73",    0x80000, 0xc3ad40f3, BRF_SND },	     //  15	Samples
	
	{ "mb7114h.ic38",  0x00100, 0x113c7443, BRF_GRA },	     //  16	PROM
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo DrvjRomDesc[] = {
	{ "30j15.ic78",    0x40000, 0x40618cbc, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "30j14.ic79",    0x20000, 0x96827e80, BRF_ESS | BRF_PRG }, //	 1
	
	{ "30j13.ic43",    0x10000, 0x1e974d9b, BRF_ESS | BRF_PRG }, //  2	Z80 Program 
	
	{ "30j-7.ic4",     0x40000, 0x89d58d32, BRF_GRA },	     //  3	Tiles
	{ "30j-6.ic5",     0x40000, 0x9bf1538e, BRF_GRA },	     //  4
	{ "30j-5.ic6",     0x40000, 0x8f671a62, BRF_GRA },	     //  5
	{ "30j-4.ic7",     0x40000, 0x0f74ea1c, BRF_GRA },	     //  6
	
	{ "30j-3.ic9",     0x80000, 0xb3151871, BRF_GRA },	     //  7	Sprites
	{ "30j12-0.ic8",   0x10000, 0x1e9290d7, BRF_GRA },	     //  8
	{ "30j-2.ic11",    0x80000, 0x41c6fb08, BRF_GRA },	     //  9
	{ "30j11-0.ic10",  0x10000, 0x99195b2a, BRF_GRA },	     //  10
	{ "30j-1.ic13",    0x80000, 0x67a6f114, BRF_GRA },	     //  11
	{ "30j10-0.ic12",  0x10000, 0xe3879b5d, BRF_GRA },	     //  12
	{ "30j-0.ic15",    0x80000, 0xf15dafbe, BRF_GRA },	     //  13
	{ "30j9-0.ic14",   0x10000, 0x2759ae84, BRF_GRA },	     //  14
	
	{ "30j-8.ic73",    0x80000, 0xc3ad40f3, BRF_SND },	     //  15	Samples
	
	{ "mb7114h.ic38",  0x00100, 0x113c7443, BRF_GRA },	     //  16	PROM
};

STD_ROM_PICK(Drvj)
STD_ROM_FN(Drvj)

static struct BurnRomInfo DrvpRomDesc[] = {
	{ "30a14-0.ic80",  0x40000, 0xf42fe016, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "30a15-0.ic79",  0x20000, 0xad50e92c, BRF_ESS | BRF_PRG }, //	 1
	
	{ "30a13-0.ic43",  0x10000, 0x1e974d9b, BRF_ESS | BRF_PRG }, //  2	Z80 Program 
	
	{ "14.ic45",       0x20000, 0xb036a27b, BRF_GRA },	     //  3	Tiles
	{ "15.ic46",       0x20000, 0x24d0bf41, BRF_GRA },	     //  4
	{ "30.ic13",       0x20000, 0x72fe2b16, BRF_GRA },	     //  5
	{ "31.ic14",       0x20000, 0xab48a0c8, BRF_GRA },	     //  6
	{ "23.ic29",       0x20000, 0x0768fedd, BRF_GRA },	     //  7
	{ "24.ic30",       0x20000, 0xec9db18a, BRF_GRA },	     //  8
	{ "21.ic25",       0x20000, 0x902744b9, BRF_GRA },	     //  9
	{ "22.ic26",       0x20000, 0x5b142d4d, BRF_GRA },	     //  10
	
	{ "9.ic39",        0x20000, 0x726c49b7, BRF_GRA },	     //  11	Sprites
	{ "10.ic40",       0x20000, 0x37a1c335, BRF_GRA },	     //  12
	{ "11.ic41",       0x20000, 0x2bcfe63c, BRF_GRA },	     //  13
	{ "12.ic42",       0x20000, 0xb864cf17, BRF_GRA },	     //  14
	{ "4.ic33",        0x20000, 0x8c71eb06, BRF_GRA },	     //  15
	{ "5.ic34",        0x20000, 0x3e134be9, BRF_GRA },	     //  16
	{ "6.ic35",        0x20000, 0xb4115ef0, BRF_GRA },	     //  17
	{ "7.ic36",        0x20000, 0x4639333d, BRF_GRA },	     //  18
	{ "16.ic19",       0x20000, 0x04420cc8, BRF_GRA },	     //  19
	{ "17.ic20",       0x20000, 0x33f97b2f, BRF_GRA },	     //  20
	{ "18.ic21",       0x20000, 0x0f9a8f2a, BRF_GRA },	     //  21
	{ "19.ic22",       0x20000, 0x15c91772, BRF_GRA },	     //  22
	{ "25.ic3",        0x20000, 0x894734b3, BRF_GRA },	     //  23
	{ "26.ic4",        0x20000, 0xcd504584, BRF_GRA },	     //  24
	{ "27.ic5",        0x20000, 0x38e8a9ad, BRF_GRA },	     //  25
	{ "28.ic6",        0x20000, 0x80c1cb74, BRF_GRA },	     //  26
	{ "30a12-0.ic43",  0x20000, 0x91da004c, BRF_GRA },	     //  27
	{ "30a11-0.ic37",  0x20000, 0x5f419232, BRF_GRA },	     //  28
	{ "30a10-0.ic23",  0x20000, 0x12f641ba, BRF_GRA },	     //  29
	{ "30a9-0.ic7",    0x20000, 0x9199a77b, BRF_GRA },	     //  30
	
	{ "2.ic73",        0x40000, 0x3af21dbe, BRF_SND },	     //  31	Samples
	{ "3.ic74",        0x40000, 0xc28b53cd, BRF_SND },	     //  32
	
	{ "30.ic38",       0x00100, 0x113c7443, BRF_GRA },	     //  33	PROM
};

STD_ROM_PICK(Drvp)
STD_ROM_FN(Drvp)

static struct BurnRomInfo DrvbRomDesc[] = {
	{ "dd3.01",        0x20000, 0x68321d8b, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "dd3.03",        0x20000, 0xbc05763b, BRF_ESS | BRF_PRG }, //	 1
	{ "dd3.02",        0x20000, 0x38d9ae75, BRF_ESS | BRF_PRG }, //	 2
	
	{ "dd3.06",        0x10000, 0x1e974d9b, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "dd3.f",         0x40000, 0x89d58d32, BRF_GRA },	     //  4	Tiles
	{ "dd3.e",         0x40000, 0x9bf1538e, BRF_GRA },	     //  5
	{ "dd3.b",         0x40000, 0x8f671a62, BRF_GRA },	     //  6
	{ "dd3.a",         0x40000, 0x0f74ea1c, BRF_GRA },	     //  7
	
	{ "dd3.3e",        0x20000, 0x726c49b7, BRF_GRA },	     //  8	Sprites
	{ "dd3.3d",        0x20000, 0x37a1c335, BRF_GRA },	     //  9
	{ "dd3.3c",        0x20000, 0x2bcfe63c, BRF_GRA },	     //  10
	{ "dd3.3b",        0x20000, 0xb864cf17, BRF_GRA },	     //  11
	{ "dd3.3a",        0x10000, 0x20d64bea, BRF_GRA },	     //  12
	{ "dd3.2e",        0x20000, 0x8c71eb06, BRF_GRA },	     //  13
	{ "dd3.2d",        0x20000, 0x3e134be9, BRF_GRA },	     //  14
	{ "dd3.2c",        0x20000, 0xb4115ef0, BRF_GRA },	     //  15
	{ "dd3.2b",        0x20000, 0x4639333d, BRF_GRA },	     //  16
	{ "dd3.2a",        0x10000, 0x785d71b0, BRF_GRA },	     //  17
	{ "dd3.1e",        0x20000, 0x04420cc8, BRF_GRA },	     //  18
	{ "dd3.1d",        0x20000, 0x33f97b2f, BRF_GRA },	     //  19
	{ "dd3.1c",        0x20000, 0x0f9a8f2a, BRF_GRA },	     //  20
	{ "dd3.1b",        0x20000, 0x15c91772, BRF_GRA },	     //  21
	{ "dd3.1a",        0x10000, 0x15e43d12, BRF_GRA },	     //  22
	{ "dd3.0e",        0x20000, 0x894734b3, BRF_GRA },	     //  23
	{ "dd3.0d",        0x20000, 0xcd504584, BRF_GRA },	     //  24
	{ "dd3.0c",        0x20000, 0x38e8a9ad, BRF_GRA },	     //  25
	{ "dd3.0b",        0x20000, 0x80c1cb74, BRF_GRA },	     //  26
	{ "dd3.0a",        0x10000, 0x5a47e7a4, BRF_GRA },	     //  27

	{ "dd3.j7",        0x40000, 0x3af21dbe, BRF_SND },	     //  28	Samples
	{ "dd3.j8",        0x40000, 0xc28b53cd, BRF_SND },	     //  29
	
	{ "mb7114h.38",    0x00100, 0x113c7443, BRF_GRA },	     //  30	PROM
};

STD_ROM_PICK(Drvb)
STD_ROM_FN(Drvb)

static struct BurnRomInfo CtribeRomDesc[] = {
	{ "28a16-2.ic26",  0x20000, 0xc46b2e63, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "28a15-2.ic25",  0x20000, 0x3221c755, BRF_ESS | BRF_PRG }, //	 1
	{ "28j17-0.104",   0x10000, 0x8c2c6dbd, BRF_ESS | BRF_PRG }, //	 2
	
	{ "28a10-0.ic89",  0x08000, 0x4346de13, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "28j7-0.ic11",   0x40000, 0xa8b773f1, BRF_GRA },	     //  4	Tiles
	{ "28j6-0.ic13",   0x40000, 0x617530fc, BRF_GRA },	     //  5
	{ "28j5-0.ic12",   0x40000, 0xcef0a821, BRF_GRA },	     //  6
	{ "28j4-0.ic14",   0x40000, 0xb84fda09, BRF_GRA },	     //  7
	
	{ "28j3-0.ic77",   0x80000, 0x1ac2a461, BRF_GRA },	     //  8	Sprites
	{ "28a14-0.ic60",  0x10000, 0x972faddb, BRF_GRA },	     //  9
	{ "28j2-0.ic78",   0x80000, 0x8c796707, BRF_GRA },	     //  10
	{ "28a13-0.ic61",  0x10000, 0xeb3ab374, BRF_GRA },	     //  11
	{ "28j1-0.ic97",   0x80000, 0x1c9badbd, BRF_GRA },	     //  12
	{ "28a12-0.ic85",  0x10000, 0xc602ac97, BRF_GRA },	     //  13
	{ "28j0-0.ic98",   0x80000, 0xba73c49e, BRF_GRA },	     //  14
	{ "28a11-0.ic86",  0x10000, 0x4da1d8e5, BRF_GRA },	     //  15

	{ "28j9-0.ic83",   0x20000, 0xf92a7f4a, BRF_SND },	     //  16	Samples
	{ "28j8-0.ic82",   0x20000, 0x1a3a0b39, BRF_SND },	     //  17
	
	{ "28.ic44",       0x00100, 0x964329ef, BRF_GRA },	     //  18	PROM
};

STD_ROM_PICK(Ctribe)
STD_ROM_FN(Ctribe)

static struct BurnRomInfo Ctribe1RomDesc[] = {
	{ "1_28a16-2.ic26",0x20000, 0xf00f8443, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "1_28a15-2.ic25",0x20000, 0xdd70079f, BRF_ESS | BRF_PRG }, //	 1
	{ "28j17-0.104",   0x10000, 0x8c2c6dbd, BRF_ESS | BRF_PRG }, //	 2
	
	{ "28a10-0.ic89",  0x08000, 0x4346de13, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "28j7-0.ic11",   0x40000, 0xa8b773f1, BRF_GRA },	     //  4	Tiles
	{ "28j6-0.ic13",   0x40000, 0x617530fc, BRF_GRA },	     //  5
	{ "28j5-0.ic12",   0x40000, 0xcef0a821, BRF_GRA },	     //  6
	{ "28j4-0.ic14",   0x40000, 0xb84fda09, BRF_GRA },	     //  7
	
	{ "28j3-0.ic77",   0x80000, 0x1ac2a461, BRF_GRA },	     //  8	Sprites
	{ "28a14-0.ic60",  0x10000, 0x972faddb, BRF_GRA },	     //  9
	{ "28j2-0.ic78",   0x80000, 0x8c796707, BRF_GRA },	     //  10
	{ "28a13-0.ic61",  0x10000, 0xeb3ab374, BRF_GRA },	     //  11
	{ "28j1-0.ic97",   0x80000, 0x1c9badbd, BRF_GRA },	     //  12
	{ "28a12-0.ic85",  0x10000, 0xc602ac97, BRF_GRA },	     //  13
	{ "28j0-0.ic98",   0x80000, 0xba73c49e, BRF_GRA },	     //  14
	{ "28a11-0.ic86",  0x10000, 0x4da1d8e5, BRF_GRA },	     //  15

	{ "28j9-0.ic83",   0x20000, 0xf92a7f4a, BRF_SND },	     //  16	Samples
	{ "28j8-0.ic82",   0x20000, 0x1a3a0b39, BRF_SND },	     //  17
	
	{ "28.ic44",       0x00100, 0x964329ef, BRF_GRA },	     //  18	PROM
};

STD_ROM_PICK(Ctribe1)
STD_ROM_FN(Ctribe1)

static struct BurnRomInfo CtribejRomDesc[] = {
	{ "28j16-02.26",   0x20000, 0x658b8568, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "28j15-12.25",   0x20000, 0x50aac7e7, BRF_ESS | BRF_PRG }, //	 1
	{ "28j17-0.104",   0x10000, 0x8c2c6dbd, BRF_ESS | BRF_PRG }, //	 2
	
	{ "28j10-0.89",    0x08000, 0x4346de13, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "28j7-0.ic11",   0x40000, 0xa8b773f1, BRF_GRA },	     //  4	Tiles
	{ "28j6-0.ic13",   0x40000, 0x617530fc, BRF_GRA },	     //  5
	{ "28j5-0.ic12",   0x40000, 0xcef0a821, BRF_GRA },	     //  6
	{ "28j4-0.ic14",   0x40000, 0xb84fda09, BRF_GRA },	     //  7
	
	{ "28j3-0.ic77",   0x80000, 0x1ac2a461, BRF_GRA },	     //  8	Sprites
	{ "28j14-0.60",    0x10000, 0x6869050a, BRF_GRA },	     //  9
	{ "28j2-0.ic78",   0x80000, 0x8c796707, BRF_GRA },	     //  10
	{ "28j13-0.61",    0x10000, 0x8b8addea, BRF_GRA },	     //  11
	{ "28j1-0.ic97",   0x80000, 0x1c9badbd, BRF_GRA },	     //  12
	{ "28j12-0.85",    0x10000, 0x422b041c, BRF_GRA },	     //  13
	{ "28j0-0.ic98",   0x80000, 0xba73c49e, BRF_GRA },	     //  14
	{ "28j11-0.86",    0x10000, 0x4a391c5b, BRF_GRA },	     //  15

	{ "28j9-0.ic83",   0x20000, 0xf92a7f4a, BRF_SND },	     //  16	Samples
	{ "28j8-0.ic82",   0x20000, 0x1a3a0b39, BRF_SND },	     //  17
	
	{ "28.ic44",       0x00100, 0x964329ef, BRF_GRA },	     //  18	PROM
};

STD_ROM_PICK(Ctribej)
STD_ROM_FN(Ctribej)

static struct BurnRomInfo CtribebRomDesc[] = {
	{ "ct_ep1.rom",    0x20000, 0x9cfa997f, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ct_ep3.rom",    0x20000, 0x2ece8681, BRF_ESS | BRF_PRG }, //	 1
	{ "ct_ep2.rom",    0x10000, 0x8c2c6dbd, BRF_ESS | BRF_PRG }, //	 2
	
	{ "ct_ep4.rom",    0x08000, 0x4346de13, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "ct_mr7.rom",    0x40000, 0xa8b773f1, BRF_GRA },	     //  4	Tiles
	{ "ct_mr6.rom",    0x40000, 0x617530fc, BRF_GRA },	     //  5
	{ "ct_mr5.rom",    0x40000, 0xcef0a821, BRF_GRA },	     //  6
	{ "ct_mr4.rom",    0x40000, 0xb84fda09, BRF_GRA },	     //  7
	
	{ "ct_mr3.rom",    0x80000, 0x1ac2a461, BRF_GRA },	     //  8	Sprites
	{ "ct_ep5.rom",    0x10000, 0x972faddb, BRF_GRA },	     //  9
	{ "ct_mr2.rom",    0x80000, 0x8c796707, BRF_GRA },	     //  10
	{ "ct_ep6.rom",    0x10000, 0xeb3ab374, BRF_GRA },	     //  11
	{ "ct_mr1.rom",    0x80000, 0x1c9badbd, BRF_GRA },	     //  12
	{ "ct_ep7.rom",    0x10000, 0xc602ac97, BRF_GRA },	     //  13
	{ "ct_mr0.rom",    0x80000, 0xba73c49e, BRF_GRA },	     //  14
	{ "ct_ep8.rom",    0x10000, 0x4da1d8e5, BRF_GRA },	     //  15

	{ "ct_mr8.rom",    0x40000, 0x9963a6be, BRF_SND },	     //  16	Samples
};

STD_ROM_PICK(Ctribeb)
STD_ROM_FN(Ctribeb)

static struct BurnRomInfo Ctribeb2RomDesc[] = {
	{ "1.bin",         0x20000, 0x9cfa997f, BRF_ESS | BRF_PRG }, // 0	68000 Program Code
	{ "3.bin",         0x20000, 0x2ece8681, BRF_ESS | BRF_PRG }, //	1
	{ "2.bin",         0x10000, 0x8c2c6dbd, BRF_ESS | BRF_PRG }, //	2
	
	{ "6.bin",         0x10000, 0x0101df2d, BRF_ESS | BRF_PRG }, // 3	Z80 Program 
	
	{ "7.bin",         0x40000, 0xa8b773f1, BRF_GRA },	     // 4	Tiles
	{ "8.bin",         0x40000, 0x617530fc, BRF_GRA },	     // 5
	{ "11.bin",        0x40000, 0xcef0a821, BRF_GRA },	     // 6
	{ "12.bin",        0x40000, 0xb84fda09, BRF_GRA },	     // 7
	{ "9.bin",         0x20000, 0x2719d7ce, BRF_GRA },	     // 8
	{ "10.bin",        0x20000, 0x753a4f53, BRF_GRA },	     // 9
	{ "13.bin",        0x20000, 0x59e01fe1, BRF_GRA },	     // 10
	{ "14.bin",        0x20000, 0xa69ab4f3, BRF_GRA },	     // 11
	
	{ "34.bin",        0x20000, 0x5b498f0e, BRF_GRA },	     // 12	Sprites
	{ "33.bin",        0x20000, 0x14d79049, BRF_GRA },	     // 13
	{ "32.bin",        0x20000, 0x9631ea23, BRF_GRA },	     // 14
	{ "31.bin",        0x20000, 0x0ca8d3b9, BRF_GRA },	     // 15
	{ "30.bin",        0x10000, 0x972faddb, BRF_GRA },	     // 16
	{ "29.bin",        0x20000, 0x479ae8ea, BRF_GRA },	     // 17
	{ "28.bin",        0x20000, 0x95598bbf, BRF_GRA },	     // 18
	{ "27.bin",        0x20000, 0x4a3d006d, BRF_GRA },	     // 19
	{ "26.bin",        0x20000, 0xaa34a3cb, BRF_GRA },	     // 20
	{ "25.bin",        0x10000, 0xeb3ab374, BRF_GRA },	     // 21
	{ "24.bin",        0x20000, 0xd60bbff0, BRF_GRA },	     // 22
	{ "23.bin",        0x20000, 0xd9595c47, BRF_GRA },	     // 23
	{ "22.bin",        0x20000, 0x5a19a911, BRF_GRA },	     // 24
	{ "21.bin",        0x20000, 0x071360f9, BRF_GRA },	     // 25
	{ "20.bin",        0x10000, 0xc602ac97, BRF_GRA },	     // 26
	{ "19.bin",        0x20000, 0x8d22736a, BRF_GRA },	     // 27
	{ "18.bin",        0x20000, 0x0f157822, BRF_GRA },	     // 28
	{ "17.bin",        0x20000, 0x7f48c824, BRF_GRA },	     // 29
	{ "16.bin",        0x20000, 0xcd1e9bd5, BRF_GRA },	     // 30
	{ "15.bin",        0x10000, 0x4da1d8e5, BRF_GRA },	     // 31

	{ "5.bin",         0x20000, 0xf92a7f4a, BRF_SND },	     // 32	Samples
	{ "4.bin",         0x20000, 0x1a3a0b39, BRF_SND },	     // 33
};

STD_ROM_PICK(Ctribeb2)
STD_ROM_FN(Ctribeb2)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Drv68KRom              = Next; Next += 0x80000;
	DrvZ80Rom              = Next; Next += 0x10000;
	MSM6295ROM             = Next; Next += 0x40000;
	DrvMSM6295ROMSrc       = Next; Next += 0x80000;

	RamStart               = Next;

	Drv68KRam              = Next; Next += 0x04800;
	DrvZ80Ram              = Next; Next += 0x00800;
	DrvFgVideoRam          = Next; Next += 0x01000;
	DrvSpriteRam           = Next; Next += 0x01000;
	DrvBgVideoRam          = Next; Next += 0x00800;
	DrvPaletteRam          = Next; Next += 0x00600;

	RamEnd                 = Next;

	DrvTiles               = Next; Next += 8192 * 16 * 16;
	DrvSprites             = Next; Next += 0x4800 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00600 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();
	
	ZetOpen(0);
	ZetReset();
	ZetClose();
	
	BurnYM2151Reset();
	MSM6295Reset(0);
	
	DrvBgTileBase = 0;
	DrvBgScrollX = 0;
	DrvBgScrollY = 0;
	DrvFgScrollX = 0;
	DrvFgScrollY = 0;
	DrvVReg = 0;
	DrvSoundLatch = 0;
	DrvOkiBank = 0;
	DrvVBlank = 0;
	
	return 0;
}

UINT8 __fastcall Ddragon368KReadByte(UINT32 a)
{
	switch (a) {
		case 0x100000: {
			return 0xff - DrvInput[2];
		}
		
		case 0x100001: {
			return 0xff - DrvInput[0];
		}
		
		case 0x100003: {
			return 0xff - DrvInput[1];
		}
		
		case 0x100007: {
			return 0xff - DrvInput[3];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Ddragon368KWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x100003: {
			DrvSoundLatch = d;
			ZetOpen(0);
			ZetNmi();
			nCyclesDone[1] += ZetRun(100);
			ZetClose();
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Ddragon368KReadWord(UINT32 a)
{
	switch (a) {
		case 0x100002: {
			return (0xff << 8) | (0xff - DrvInput[1]);
		}
		
		case 0x100004: {
			return (DrvDip[1] << 8) | DrvDip[0];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Ddragon368KWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x000004:
		case 0x000006: {
			// ???
			return;
		}

		case 0x0c0000: {
			DrvFgScrollX = d & 0x1ff;
			return;
		}
		
		case 0x0c0002: {
			DrvFgScrollY = d & 0x1ff;
			return;
		}
		
		case 0x0c0004: {
			DrvBgScrollX = d & 0x1ff;
			return;
		}
		
		case 0x0c0006: {
			DrvBgScrollY = d & 0x1ff;
			return;
		}
		
		case 0x0c0008: {
			return;
		}
		
		case 0x0c000c: {
			DrvBgTileBase = d & 0x1ff;
			return;
		}
		
		case 0x100000: {
			DrvVReg = d;
			return;
		}
		
		case 0x100002: {
			DrvSoundLatch = d;
			ZetOpen(0);
			ZetNmi();
			nCyclesDone[1] += ZetRun(100);
			ZetClose();
			return;
		}
		
		case 0x100004:
		case 0x100006:
		case 0x100008: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Ddragon3b68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x180000: {
			return 0xef - DrvInput[1] + (DrvDip[0] & 0x10);
		}
		
		case 0x180001: {
			return 0xff - DrvInput[0];
		}
		
		case 0x180002: {
			return DrvDip[1];
		}
		
		case 0x180003: {
			return 0xff - DrvInput[2];
		}

		case 0x180005: {
			return 0xff - DrvInput[3];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Ddragon3b68KWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x140003: {
			DrvSoundLatch = d;
			ZetOpen(0);
			ZetNmi();
			nCyclesDone[1] += ZetRun(100);
			ZetClose();
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Ddragon3b68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x180000: {
			return (((0xef - DrvInput[1]) + (DrvDip[0] & 0x10)) << 8) | (0xff - DrvInput[0]);
		}

		case 0x180004: {
			return (DrvDip[2] << 8) | (0xff - DrvInput[3]);
		}
		
		case 0x180006: {
			return (DrvDip[3] << 8) | 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Ddragon3b68KWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x000004:
		case 0x000006: {
			// ???
			return;
		}
		
		case 0x0c0000: {
			DrvFgScrollX = d & 0x1ff;
			return;
		}
		
		case 0x0c0002: {
			DrvFgScrollY = d & 0x1ff;
			return;
		}
		
		case 0x0c0004: {
			DrvBgScrollX = d & 0x1ff;
			return;
		}
		
		case 0x0c0006: {
			DrvBgScrollY = d & 0x1ff;
			return;
		}
		
		case 0x0c0008: {
			return;
		}
		
		case 0x0c000c: {
			DrvBgTileBase = d & 0x1ff;
			return;
		}
		
		case 0x140000: {
			DrvVReg = d;
			return;
		}
		
		case 0x140002: {
			DrvSoundLatch = d;
			ZetOpen(0);
			ZetNmi();
			nCyclesDone[1] += ZetRun(100);
			ZetClose();
			return;
		}
		
		case 0x140004:
		case 0x140006:
		case 0x140008: {
			// ???
			return;
		}
			
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Ddragon3Z80Read(UINT16 a)
{
	switch (a) {
		case 0xc801: {
			return BurnYM2151ReadStatus();
		}
		
		case 0xd800: {
			return MSM6295ReadStatus(0);
		}
		
		case 0xe000: {
			return DrvSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall Ddragon3Z80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xc800: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0xc801: {
			BurnYM2151WriteRegister(d);
			return;
		}
		
		case 0xd800: {
			MSM6295Command(0, d);
			return;
		}
		
		case 0xe800: {
			DrvOkiBank = d & 1;
			memcpy(MSM6295ROM + 0x00000, DrvMSM6295ROMSrc + (0x40000 * DrvOkiBank), 0x40000);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall Ctribeb68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x180000: {
			if (DrvVBlank) return 0xe7 - DrvInput[1] + (DrvDip[0] & 0x10);
			return 0xef - DrvInput[1] + (DrvDip[0] & 0x10);
		}
		
		case 0x180001: {
			return 0xff - DrvInput[0];
		}
		
		case 0x180003: {
			return 0xff - DrvInput[2];
		}
		
		case 0x180004: {
			return DrvDip[2];
		}
		
		case 0x180005: {
			return 0xff - DrvInput[3];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Ctribeb68KWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x0c000d: {
			DrvBgTileBase = d;
			return;
		}
		
		case 0x140001: {
			DrvVReg = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Ctribeb68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x180000: {
			if (DrvVBlank) return ((0xe7 - DrvInput[1] + (DrvDip[0] & 0x10)) << 8) | (0xff - DrvInput[0]);
			return ((0xef - DrvInput[1] + (DrvDip[0] & 0x10)) << 8) | (0xff - DrvInput[0]);
		}
		
		case 0x180002: {
			return (DrvDip[1] << 8) | (0xff - DrvInput[2]);
		}
		
		case 0x180004: {
			return (DrvDip[2] << 8) | (0xff - DrvInput[3]);
		}
		
		case 0x180006: {
			return (DrvDip[3] << 8) | 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Ctribeb68KWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x0c0000: {
			DrvFgScrollX = d & 0x1ff;
			return;
		}
		
		case 0x0c0002: {
			DrvFgScrollY = d & 0x1ff;
			return;
		}
		
		case 0x0c0004: {
			DrvBgScrollX = d & 0x1ff;
			return;
		}
		
		case 0x0c0006: {
			DrvBgScrollY = d & 0x1ff;
			return;
		}
		
		case 0x0c0008: {
			return;
		}
		
		case 0x0c000c: {
			DrvBgTileBase = d & 0x1ff;
			return;
		}
		
		case 0x140000: {
			DrvVReg = d;
			return;
		}
		
		case 0x140002: {
			DrvSoundLatch = d & 0xff;
			ZetOpen(0);
			ZetNmi();
			nCyclesDone[1] += ZetRun(100);
			ZetClose();
			return;
		}
		
		case 0x140004:
		case 0x140006:
		case 0x140008: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall CtribeZ80Read(UINT16 a)
{
	switch (a) {
		case 0x8801: {
			return BurnYM2151ReadStatus();
		}
		
		case 0x9800: {
			return MSM6295ReadStatus(0);
		}
		
		case 0xa000: {
			return DrvSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall CtribeZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0x8800: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0x8801: {
			BurnYM2151WriteRegister(d);
			return;
		}
		
		case 0x9800: {
			MSM6295Command(0, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static INT32 TilePlaneOffsets[4]     = { 0, 0x200000, 0x400000, 0x600000 };
static INT32 TileXOffsets[16]        = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
static INT32 TileYOffsets[16]        = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };
static INT32 SpritePlaneOffsets[4]   = { 0, 0x800000, 0x1000000, 0x1800000 };
static INT32 SpriteXOffsets[16]      = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
static INT32 SpriteYOffsets[16]      = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };

static void DrvYM2151IrqHandler(INT32 Irq)
{
	if (Irq) {
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvInit()
{
	INT32 nRet = 0, nLen;
	
	BurnSetRefreshRate(57.0);

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x400000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 2, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x040000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000, 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c0000, 6, 1); if (nRet != 0) return 1;
	GfxDecode(8192, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x400000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x180000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x200000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x280000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x380000, 14, 1); if (nRet != 0) return 1;
	GfxDecode(0x4800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x00000, 15, 1); if (nRet != 0) return 1;
	memcpy(MSM6295ROM, DrvMSM6295ROMSrc, 0x40000);
	
	BurnFree(DrvTempRom);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvFgVideoRam       , 0x080000, 0x080fff, SM_RAM);
	SekMapMemory(DrvBgVideoRam       , 0x082000, 0x0827ff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x140000, 0x1405ff, SM_RAM);
	SekMapMemory(DrvSpriteRam        , 0x180000, 0x180fff, SM_RAM);
	SekMapMemory(Drv68KRam           , 0x1c0000, 0x1c3fff, SM_RAM);
	SekSetReadWordHandler(0, Ddragon368KReadWord);
	SekSetWriteWordHandler(0, Ddragon368KWriteWord);
	SekSetReadByteHandler(0, Ddragon368KReadByte);
	SekSetWriteByteHandler(0, Ddragon368KWriteByte);
	SekClose();
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(Ddragon3Z80Read);
	ZetSetWriteHandler(Ddragon3Z80Write);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80Rom                );
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80Rom                );
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80Ram                );
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80Ram                );
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80Ram                );
	ZetMemEnd();
	ZetClose();
	
	// Setup the YM2151 emulation
	BurnYM2151Init(3579545, 25.0);
	BurnYM2151SetIrqHandler(&DrvYM2151IrqHandler);
	
	// Setup the OKIM6295 emulation
	MSM6295Init(0, 1000000 / 132, 100.0, 1);
	
	DrawFunction = DrvDraw;
	
	GenericTilesInit();

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 DrvpInit()
{
	INT32 nRet = 0, nLen;
	
	BurnSetRefreshRate(57.0);

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x400000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 2, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x020000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x040000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x060000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0a0000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c0000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0e0000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(8192, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x400000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x020000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x040000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x060000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x120000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x140000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x160000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x200000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x220000, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x240000, 21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x260000, 22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300000, 23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x320000, 24, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x340000, 25, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x360000, 26, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000, 27, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x180000, 28, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x280000, 29, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x380000, 30, 1); if (nRet != 0) return 1;
	GfxDecode(0x4800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x00000, 31, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x40000, 32, 1); if (nRet != 0) return 1;
	memcpy(MSM6295ROM, DrvMSM6295ROMSrc, 0x40000);
	
	BurnFree(DrvTempRom);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvFgVideoRam       , 0x080000, 0x080fff, SM_RAM);
	SekMapMemory(DrvBgVideoRam       , 0x082000, 0x0827ff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x140000, 0x1405ff, SM_RAM);
	SekMapMemory(DrvSpriteRam        , 0x180000, 0x180fff, SM_RAM);
	SekMapMemory(Drv68KRam           , 0x1c0000, 0x1c3fff, SM_RAM);
	SekSetReadWordHandler(0, Ddragon368KReadWord);
	SekSetWriteWordHandler(0, Ddragon368KWriteWord);
	SekSetReadByteHandler(0, Ddragon368KReadByte);
	SekSetWriteByteHandler(0, Ddragon368KWriteByte);
	SekClose();
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(Ddragon3Z80Read);
	ZetSetWriteHandler(Ddragon3Z80Write);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80Rom                );
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80Rom                );
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80Ram                );
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80Ram                );
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80Ram                );
	ZetMemEnd();
	ZetClose();
	
	// Setup the YM2151 emulation
	BurnYM2151Init(3579545, 25.0);
	BurnYM2151SetIrqHandler(&DrvYM2151IrqHandler);
	
	// Setup the OKIM6295 emulation
	MSM6295Init(0, 1000000 / 132, 100.0, 1);
	
	DrawFunction = DrvDraw;
	
	GenericTilesInit();

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 DrvbInit()
{
	INT32 nRet = 0, nLen;
	
	BurnSetRefreshRate(57.0);

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x400000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40000, 2, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 3, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x040000, 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000, 6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c0000, 7, 1); if (nRet != 0) return 1;
	GfxDecode(8192, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x400000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x020000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x040000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x060000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x120000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x140000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x160000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x180000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x200000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x220000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x240000, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x260000, 21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x280000, 22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300000, 23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x320000, 24, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x340000, 25, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x360000, 26, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x380000, 27, 1); if (nRet != 0) return 1;
	GfxDecode(0x4800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x00000, 28, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x40000, 29, 1); if (nRet != 0) return 1;
	memcpy(MSM6295ROM, DrvMSM6295ROMSrc, 0x40000);
	
	BurnFree(DrvTempRom);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvFgVideoRam       , 0x080000, 0x080fff, SM_RAM);
	SekMapMemory(DrvSpriteRam        , 0x081000, 0x081fff, SM_RAM);
	SekMapMemory(DrvBgVideoRam       , 0x082000, 0x0827ff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x100000, 0x1005ff, SM_RAM);
	SekMapMemory(Drv68KRam           , 0x1c0000, 0x1c3fff, SM_RAM);
	SekSetReadWordHandler(0, Ddragon3b68KReadWord);
	SekSetWriteWordHandler(0, Ddragon3b68KWriteWord);
	SekSetReadByteHandler(0, Ddragon3b68KReadByte);
	SekSetWriteByteHandler(0, Ddragon3b68KWriteByte);
	SekClose();
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(Ddragon3Z80Read);
	ZetSetWriteHandler(Ddragon3Z80Write);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80Rom                );
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80Rom                );
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80Ram                );
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80Ram                );
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80Ram                );
	ZetMemEnd();
	ZetClose();
	
	// Setup the YM2151 emulation
	BurnYM2151Init(3579545, 25.0);
	BurnYM2151SetIrqHandler(&DrvYM2151IrqHandler);
	
	// Setup the OKIM6295 emulation
	MSM6295Init(0, 1000000 / 132, 100.0, 1);
	
	DrawFunction = DrvDraw;
	
	GenericTilesInit();

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 CtribeInit()
{
	INT32 nRet = 0, nLen;
	
	BurnSetRefreshRate(57.0);

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x400000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40000, 2, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 3, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x040000, 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000, 6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c0000, 7, 1); if (nRet != 0) return 1;
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "ctribeb2")) {
		nRet = BurnLoadRom(DrvTempRom + 0x100000,  8, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x140000,  9, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x180000, 10, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x1c0000, 11, 1); if (nRet != 0) return 1;
	}
	GfxDecode(8192, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x400000);
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "ctribeb2")) {
		nRet = BurnLoadRom(DrvTempRom + 0x000000, 12, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x020000, 13, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x040000, 14, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x060000, 15, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x080000, 16, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x100000, 17, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x120000, 18, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x140000, 19, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x160000, 20, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x180000, 21, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x200000, 22, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x220000, 23, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x240000, 24, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x260000, 25, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x280000, 26, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x300000, 27, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x320000, 28, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x340000, 29, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x360000, 30, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x380000, 31, 1); if (nRet != 0) return 1;
	} else {
		nRet = BurnLoadRom(DrvTempRom + 0x000000,  8, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x080000,  9, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x100000, 10, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x180000, 11, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x200000, 12, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x280000, 13, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x300000, 14, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x380000, 15, 1); if (nRet != 0) return 1;
	}
	GfxDecode(0x4800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "ctribeb")) {
		memset(DrvTempRom, 0, 0x400000);
		nRet = BurnLoadRom(DrvTempRom, 16, 1); if (nRet != 0) return 1;
		memcpy(MSM6295ROM + 0x20000, DrvTempRom + 0x00000, 0x20000);
		memcpy(MSM6295ROM + 0x00000, DrvTempRom + 0x20000, 0x20000);
	} else {
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "ctribeb2")) {
			nRet = BurnLoadRom(MSM6295ROM + 0x00000, 32, 1); if (nRet != 0) return 1;
			nRet = BurnLoadRom(MSM6295ROM + 0x20000, 33, 1); if (nRet != 0) return 1;
		} else {
			nRet = BurnLoadRom(MSM6295ROM + 0x00000, 16, 1); if (nRet != 0) return 1;
			nRet = BurnLoadRom(MSM6295ROM + 0x20000, 17, 1); if (nRet != 0) return 1;
		}
	}
	memcpy(DrvMSM6295ROMSrc, MSM6295ROM, 0x40000);
	
	BurnFree(DrvTempRom);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvFgVideoRam       , 0x080000, 0x080fff, SM_RAM);
	SekMapMemory(DrvSpriteRam        , 0x081000, 0x081fff, SM_RAM);
	SekMapMemory(DrvBgVideoRam       , 0x082000, 0x0827ff, SM_RAM);
	SekMapMemory(Drv68KRam           , 0x082800, 0x082fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x100000, 0x1005ff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x800   , 0x1c0000, 0x1c3fff, SM_RAM);
	SekSetReadWordHandler(0, Ctribeb68KReadWord);
	SekSetWriteWordHandler(0, Ctribeb68KWriteWord);
	SekSetReadByteHandler(0, Ctribeb68KReadByte);
	SekSetWriteByteHandler(0, Ctribeb68KWriteByte);
	SekClose();
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(CtribeZ80Read);
	ZetSetWriteHandler(CtribeZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom                );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom                );
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80Ram                );
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80Ram                );
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80Ram                );
	ZetMemEnd();
	ZetClose();
	
	// Setup the YM2151 emulation
	BurnYM2151Init(3579545, 25.0);
	BurnYM2151SetIrqHandler(&DrvYM2151IrqHandler);
	
	// Setup the OKIM6295 emulation
	MSM6295Init(0, 1000000 / 132, 10.0, 1);
	
	DrawFunction = CtribeDraw;
	
	GenericTilesInit();

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	SekExit();
	ZetExit();
	
	BurnYM2151Exit();
	MSM6295Exit(0);
	
	GenericTilesExit();
	
	DrawFunction = NULL;
	
	DrvBgTileBase = 0;
	DrvBgScrollX = 0;
	DrvBgScrollY = 0;
	DrvFgScrollX = 0;
	DrvFgScrollY = 0;
	DrvVReg = 0;
	DrvSoundLatch = 0;
	DrvOkiBank = 0;
	DrvVBlank = 0;
	
	BurnFree(Mem);

	return 0;
}

static inline UINT8 pal4bit(UINT8 bits)
{
	bits &= 0x0f;
	return (bits << 4) | bits;
}

static inline UINT8 pal5bit(UINT8 bits)
{
	bits &= 0x1f;
	return (bits << 3) | (bits >> 2);
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal5bit(nColour >>  0);
	g = pal5bit(nColour >>  5);
	b = pal5bit(nColour >> 10);

	return BurnHighCol(r, g, b, 0);
}

inline static UINT32 CtribeCalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal4bit(nColour >> 0);
	g = pal4bit(nColour >> 4);
	b = pal4bit(nColour >> 8);

	return BurnHighCol(r, g, b, 0);
}

static void DrvCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)DrvPaletteRam, pd = DrvPalette; i < 0x600; i++, ps++, pd++) {
		*pd = CalcCol(BURN_ENDIAN_SWAP_INT16(*ps));
	}
}

static void CtribeCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)DrvPaletteRam, pd = DrvPalette; i < 0x600; i++, ps++, pd++) {
		*pd = CtribeCalcCol(BURN_ENDIAN_SWAP_INT16(*ps));
	}
}

static void DrvRenderBgLayer(INT32 Opaque)
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0;
	
	UINT16 *VideoRam = (UINT16*)DrvBgVideoRam;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
			Code = (Attr & 0xfff) | ((DrvBgTileBase & 0x01) << 12);
			Colour = ((Attr & 0xf000) >> 12);
			
			x = 16 * mx;
			y = 16 * my;
			
			x -= DrvBgScrollX;
			y -= DrvBgScrollY;
			if (x < -16) x += 512;
			if (y < -16) y += 512;
			
			y -= 8;

			if (Opaque) {
				if (x > 16 && x < 304 && y > 16 && y < 224) {
					Render16x16Tile(pTransDraw, Code, x, y, Colour, 4, 512, DrvTiles);
				} else {
					Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 512, DrvTiles);
				}
			} else {
				if (x > 16 && x < 304 && y > 16 && y < 224) {
					Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 512, DrvTiles);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 512, DrvTiles);
				}
			}
			
			TileIndex++;
		}
	}
}

static void DrvRenderFgLayer(INT32 Opaque)
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0, Offset, xFlip;
	
	UINT16 *VideoRam = (UINT16*)DrvFgVideoRam;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Offset = TileIndex * 2;
			Attr = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset]);
			Code = BURN_ENDIAN_SWAP_INT16(VideoRam[Offset + 1]) & 0x1fff;
			Colour = Attr & 0x0f;
			xFlip = Attr & 0x40;
			
			x = 16 * mx;
			y = 16 * my;
			
			x -= DrvFgScrollX;
			y -= DrvFgScrollY;
			if (x < -16) x += 512;
			if (y < -16) y += 512;
			
			y -= 8;

			if (Opaque) {
				if (x > 16 && x < 304 && y > 16 && y < 224) {
					if (xFlip) {
						Render16x16Tile_FlipX(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					} else {
						Render16x16Tile(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					}
				} else {
					if (xFlip) {
						Render16x16Tile_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					} else {
						Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					}
				}
			} else {
				if (x > 16 && x < 304 && y > 16 && y < 224) {
					if (xFlip) {
						Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvTiles);
					} else {
						Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvTiles);
					}
				} else {
					if (xFlip) {
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvTiles);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvTiles);
					}
				}
			}
			
			TileIndex++;
		}
	}
}

static void DrvRenderSprites()
{
	UINT16 *Source = (UINT16*)DrvSpriteRam;
	UINT16 *Finish = Source + 0x800;
	
	while (Source < Finish) {
		UINT16 Attr = BURN_ENDIAN_SWAP_INT16(Source[1]);
		
		if (Attr & 0x01) {
			INT32 i;
			INT32 Bank = BURN_ENDIAN_SWAP_INT16(Source[3]) & 0xff;
			INT32 Code = (BURN_ENDIAN_SWAP_INT16(Source[2]) & 0xff) + (Bank * 256);
			INT32 Colour = BURN_ENDIAN_SWAP_INT16(Source[4]) & 0xf;
			INT32 xFlip = Attr & 0x10;
			INT32 yFlip = Attr & 0x08;
			INT32 sx = BURN_ENDIAN_SWAP_INT16(Source[5]) & 0xff;
			INT32 sy = BURN_ENDIAN_SWAP_INT16(Source[0]) & 0xff;
			INT32 Height = (Attr >> 5) & 0x07;
			
			if (Attr & 0x04) sx |= 0x100;
			if (Attr & 0x02) {
				sy = 239 + (0x100 - sy);
			} else {
				sy = 240 - sy;
			}
			if (sx > 0x17f) sx = 0 - (0x200 - sx);
			
			sy -= 8;
			
			for (i = 0; i <= Height; i++) {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code + i, sx, sy - (i * 16), Colour, 4, 0, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code + i, sx, sy - (i * 16), Colour, 4, 0, 0, DrvSprites);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code + i, sx, sy - (i * 16), Colour, 4, 0, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, Code + i, sx, sy - (i * 16), Colour, 4, 0, 0, DrvSprites);
					}
				}
			}
		}
		
		Source += 8;
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	
	if ((DrvVReg & 0x60) == 0x40) {
		DrvRenderBgLayer(1);
		DrvRenderFgLayer(0);
		DrvRenderSprites();
	} else if ((DrvVReg & 0x60) == 0x60) {
		DrvRenderFgLayer(1);
		DrvRenderBgLayer(0);
		DrvRenderSprites();
	} else {
		DrvRenderBgLayer(1);
		DrvRenderSprites();
		DrvRenderFgLayer(0);
	}
	
	BurnTransferCopy(DrvPalette);
}

static void CtribeDraw()
{
	BurnTransferClear();
	CtribeCalcPalette();
	
	if (DrvVReg & 0x08) {
		DrvRenderFgLayer(1);
		DrvRenderSprites();
		DrvRenderBgLayer(0);
	} else {
		DrvRenderBgLayer(1);
		DrvRenderFgLayer(0);
		DrvRenderSprites();		
	}
	
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 10;
	INT32 nSoundBufferPos = 0;

	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	nCyclesTotal[0] = 12000000 / 60;
	nCyclesTotal[1] = 3579545 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	SekNewFrame();
	ZetNewFrame();
	
	DrvVBlank = 0;
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == 5) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
		if (i == 5) DrvVBlank = 1;
		SekClose();
		
		// Run Z80
		nCurrentCPU = 1;
		ZetOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetClose();
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	SekOpen(0);
	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	SekClose();
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			MSM6295Render(0, pSoundBuf, nSegmentLength);
		}
	}
	
	if (pBurnDraw) DrawFunction();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029674;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);			// Scan Z80
		BurnYM2151Scan(nAction);
		MSM6295Scan(0, nAction);

		// Scan critical driver variables
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
		SCAN_VAR(DrvDip);
		SCAN_VAR(DrvInput);
		SCAN_VAR(DrvBgTileBase);
		SCAN_VAR(DrvBgScrollX);
		SCAN_VAR(DrvBgScrollY);
		SCAN_VAR(DrvFgScrollX);
		SCAN_VAR(DrvFgScrollY);
		SCAN_VAR(DrvVReg);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(DrvOkiBank);
		SCAN_VAR(DrvVBlank);
	}
	
	if (nAction & ACB_WRITE) {
		memcpy(MSM6295ROM + 0x00000, DrvMSM6295ROMSrc + (0x40000 * DrvOkiBank), 0x40000);
	}

	return 0;
}

struct BurnDriver BurnDrvDdragon3 = {
	"ddragon3", NULL, NULL, NULL, "1990",
	"Double Dragon 3 - The Rosetta Stone (US)\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 320, 240, 4, 3
};

struct BurnDriver BurnDrvDdrago3j = {
	"ddragon3j", "ddragon3", NULL, NULL, "1990",
	"Double Dragon 3 - The Rosetta Stone (Japan)\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DrvjRomInfo, DrvjRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 320, 240, 4, 3
};

struct BurnDriver BurnDrvDdrago3p = {
	"ddragon3p", "ddragon3", NULL, NULL, "1990",
	"Double Dragon 3 - The Rosetta Stone (prototype)\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_PROTOTYPE, 3, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DrvpRomInfo, DrvpRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvpInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 320, 240, 4, 3
};

struct BurnDriver BurnDrvDdrago3b = {
	"ddragon3b", "ddragon3", NULL, NULL, "1990",
	"Double Dragon 3 - The Rosetta Stone (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DrvbRomInfo, DrvbRomName, NULL, NULL, DrvInputInfo, DrvbDIPInfo,
	DrvbInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 320, 240, 4, 3
};

struct BurnDriver BurnDrvCtribe = {
	"ctribe", NULL, NULL, NULL, "1990",
	"The Combatribes (US)\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, CtribeRomInfo, CtribeRomName, NULL, NULL, DrvInputInfo, CtribeDIPInfo,
	CtribeInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 320, 240, 4, 3
};

struct BurnDriver BurnDrvCtribe1 = {
	"ctribe1", "ctribe", NULL, NULL, "1990",
	"The Combatribes (US) - Set 1?\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, Ctribe1RomInfo, Ctribe1RomName, NULL, NULL, DrvInputInfo, CtribeDIPInfo,
	CtribeInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 320, 240, 4, 3
};

struct BurnDriver BurnDrvCtribej = {
	"ctribej", "ctribe", NULL, NULL, "1990",
	"The Combatribes (Japan)\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, CtribejRomInfo, CtribejRomName, NULL, NULL, DrvInputInfo, CtribeDIPInfo,
	CtribeInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 320, 240, 4, 3
};

struct BurnDriver BurnDrvCtribeb = {
	"ctribeb", "ctribe", NULL, NULL, "1990",
	"The Combatribes (bootleg set 1)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, CtribebRomInfo, CtribebRomName, NULL, NULL, DrvInputInfo, CtribeDIPInfo,
	CtribeInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 320, 240, 4, 3
};

struct BurnDriver BurnDrvCtribeb2 = {
	"ctribeb2", "ctribe", NULL, NULL, "1990",
	"The Combatribes (bootleg set 2)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, Ctribeb2RomInfo, Ctribeb2RomName, NULL, NULL, DrvInputInfo, CtribeDIPInfo,
	CtribeInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 320, 240, 4, 3
};
