#include "tiles_generic.h"
#include "z80_intf.h"
#include "m6809_intf.h"
#include "burn_ym2203.h"

static UINT8 DrvInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[2]        = {0, 0};
static UINT8 DrvInput[3]      = {0x00, 0x00, 0x00};
static UINT8 DrvReset         = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *DrvM6809Rom         = NULL;
static UINT8 *DrvZ80Rom           = NULL;
static UINT8 *DrvM6809Ram         = NULL;
static UINT8 *DrvZ80Ram           = NULL;
static UINT8 *DrvFgVideoRam       = NULL;
static UINT8 *DrvBgVideoRam       = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvSpriteRamBuffer  = NULL;
static UINT8 *DrvPaletteRam1      = NULL;
static UINT8 *DrvPaletteRam2      = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvTiles            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette          = NULL;

static UINT8 DrvRomBank;
static UINT8 DrvBgScrollX[2];
static UINT8 DrvBgScrollY[2];
static UINT8 DrvSoundLatch;

static INT32 RomLoadOffset = 0;

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static INT32 Diamond;

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 7, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , DrvInputPort1 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , DrvInputPort1 + 4, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , DrvInputPort1 + 5, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0 + 5, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Drv)

static struct BurnInputInfo DiamondInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 7, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , DrvInputPort1 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , DrvInputPort1 + 4, "p1 fire 1" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Diamond)

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
	DrvInput[0] = DrvInput[1] = DrvInput[2] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] |= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] |= (DrvInputPort2[i] & 1) << i;
	}

	// Clear Opposites
	DrvClearOpposites(&DrvInput[1]);
	DrvClearOpposites(&DrvInput[2]);
}

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xdf, NULL                     },
	{0x13, 0xff, 0xff, 0xfb, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 16  , "Coinage"                },
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Plays"        },
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Plays"        },	
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Play"         },
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Plays"        },
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Plays"        },
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Plays"        },
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Plays"        },
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Plays"        },
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Plays"        },
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Plays"        },
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Plays"        },
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Plays"        },
	{0x12, 0x01, 0x0f, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 2   , "Coinage affects"        },
	{0x12, 0x01, 0x10, 0x10, "Coin A"                 },
	{0x12, 0x01, 0x10, 0x00, "Coin B"                 },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x40, 0x40, "Off"                    },
	{0x12, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x03, 0x03, "3"                      },
	{0x13, 0x01, 0x03, 0x02, "4"                      },
	{0x13, 0x01, 0x03, 0x01, "5"                      },
	{0x13, 0x01, 0x03, 0x00, "7"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x04, 0x00, "Upright"                },
	{0x13, 0x01, 0x04, 0x04, "Cocktail"               },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x18, 0x18, "20k 70k 70k"            },
	{0x13, 0x01, 0x18, 0x10, "30k 80k 80k"            },
	{0x13, 0x01, 0x18, 0x08, "20k 80k"                },
	{0x13, 0x01, 0x18, 0x00, "30k 80k"                },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0x60, 0x40, "Easy"                   },
	{0x13, 0x01, 0x60, 0x60, "Normal"                 },
	{0x13, 0x01, 0x60, 0x20, "Difficult"              },
	{0x13, 0x01, 0x60, 0x00, "Very Difficult"         },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo DrvjDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xdf, NULL                     },
	{0x13, 0xff, 0xff, 0xfb, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 16  , "Coinage"                },
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Plays"        },
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Plays"        },	
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Play"         },
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Plays"        },
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Plays"        },
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Plays"        },
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Plays"        },
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Plays"        },
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Plays"        },
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Plays"        },
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Plays"        },
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Plays"        },
	{0x12, 0x01, 0x0f, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 2   , "Coinage affects"        },
	{0x12, 0x01, 0x10, 0x10, "Coin A"                 },
	{0x12, 0x01, 0x10, 0x00, "Coin B"                 },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x40, 0x40, "Off"                    },
	{0x12, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x03, 0x03, "3"                      },
	{0x13, 0x01, 0x03, 0x02, "4"                      },
	{0x13, 0x01, 0x03, 0x01, "5"                      },
	{0x13, 0x01, 0x03, 0x00, "7"                      },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x04, 0x00, "Upright"                },
	{0x13, 0x01, 0x04, 0x04, "Cocktail"               },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x18, 0x18, "20k 70k 70k"            },
	{0x13, 0x01, 0x18, 0x10, "30k 80k 80k"            },
	{0x13, 0x01, 0x18, 0x08, "20k 80k"                },
	{0x13, 0x01, 0x18, 0x00, "30k 80k"                },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0x60, 0x40, "Easy"                   },
	{0x13, 0x01, 0x60, 0x60, "Normal"                 },
	{0x13, 0x01, 0x60, 0x20, "Difficult"              },
	{0x13, 0x01, 0x60, 0x00, "Very Difficult"         },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Drvj)

static struct BurnDIPInfo DiamondDIPList[]=
{
	// Default Values
	{0x0a, 0xff, 0xff, 0x81, NULL                     },
	{0x0b, 0xff, 0xff, 0x07, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0a, 0x01, 0x03, 0x00, "2"                      },
	{0x0a, 0x01, 0x03, 0x01, "3"                      },
	{0x0a, 0x01, 0x03, 0x02, "4"                      },
	{0x0a, 0x01, 0x03, 0x03, "5"                      },
	
	{0   , 0xfe, 0   , 4   , "Credits A"              },
	{0x0a, 0x01, 0x0c, 0x00, "x1"                     },
	{0x0a, 0x01, 0x0c, 0x04, "x2"                     },
	{0x0a, 0x01, 0x0c, 0x08, "x3"                     },
	{0x0a, 0x01, 0x0c, 0x0c, "x4"                     },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x0a, 0x01, 0x30, 0x30, "4 Coins 1 Play"         },
	{0x0a, 0x01, 0x30, 0x20, "3 Coins 1 Play"         },
	{0x0a, 0x01, 0x30, 0x10, "2 Coins 1 Play"         },
	{0x0a, 0x01, 0x30, 0x00, "1 Coin  1 Play"         },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0a, 0x01, 0x80, 0x80, "Off"                    },
	{0x0a, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2	
	{0   , 0xfe, 0   , 4   , "Energy Loss"            },
	{0x0b, 0x01, 0x0f, 0x00, "Slowest"                },
	{0x0b, 0x01, 0x0f, 0x01, "-6 Slower"              },
	{0x0b, 0x01, 0x0f, 0x02, "-5 Slower"              },
	{0x0b, 0x01, 0x0f, 0x03, "-4 Slower"              },
	{0x0b, 0x01, 0x0f, 0x04, "-3 Slower"              },
	{0x0b, 0x01, 0x0f, 0x05, "-2 Slower"              },
	{0x0b, 0x01, 0x0f, 0x06, "-1 Slower"              },
	{0x0b, 0x01, 0x0f, 0x07, "Normal"                 },
	{0x0b, 0x01, 0x0f, 0x08, "+1 Faster"              },
	{0x0b, 0x01, 0x0f, 0x09, "+2 Faster"              },
	{0x0b, 0x01, 0x0f, 0x0a, "+3 Faster"              },
	{0x0b, 0x01, 0x0f, 0x0b, "+4 Faster"              },
	{0x0b, 0x01, 0x0f, 0x0c, "+5 Faster"              },
	{0x0b, 0x01, 0x0f, 0x0d, "+6 Faster"              },
	{0x0b, 0x01, 0x0f, 0x0e, "+7 Faster"              },
	{0x0b, 0x01, 0x0f, 0x0f, "Fastest"                },
	
	{0   , 0xfe, 0   , 4   , "Credits B"              },
	{0x0b, 0x01, 0x30, 0x00, "x1"                     },
	{0x0b, 0x01, 0x30, 0x10, "x2"                     },
	{0x0b, 0x01, 0x30, 0x20, "x3"                     },
	{0x0b, 0x01, 0x30, 0x30, "x4"                     },
};

STDDIPINFO(Diamond)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "gg4.bin",       0x04000, 0x66606beb, BRF_ESS | BRF_PRG }, //  0	M6809 Program Code
	{ "gg3.bin",       0x08000, 0x9e01c65e, BRF_ESS | BRF_PRG }, //	 1
	{ "gg5.bin",       0x08000, 0xd6397b2b, BRF_ESS | BRF_PRG }, //	 2
	
	{ "gg2.bin",       0x08000, 0x615f5b6f, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "gg1.bin",       0x04000, 0xecfccf07, BRF_GRA },	     //  4	Characters
	
	{ "gg11.bin",      0x04000, 0xddd56fa9, BRF_GRA },	     //  5	Tiles
	{ "gg10.bin",      0x04000, 0x7302529d, BRF_GRA },	     //  6
	{ "gg9.bin",       0x04000, 0x20035bda, BRF_GRA },	     //  7
	{ "gg8.bin",       0x04000, 0xf12ba271, BRF_GRA },	     //  8
	{ "gg7.bin",       0x04000, 0xe525207d, BRF_GRA },	     //  9
	{ "gg6.bin",       0x04000, 0x2d77e9b2, BRF_GRA },	     //  10
	
	{ "gg17.bin",      0x04000, 0x93e50a8f, BRF_GRA },	     //  11	Sprites
	{ "gg16.bin",      0x04000, 0x06d7e5ca, BRF_GRA },	     //  12
	{ "gg15.bin",      0x04000, 0xbc1fe02d, BRF_GRA },	     //  13
	{ "gg14.bin",      0x04000, 0x6aaf12f9, BRF_GRA },	     //  14
	{ "gg13.bin",      0x04000, 0xe80c3fca, BRF_GRA },	     //  15
	{ "gg12.bin",      0x04000, 0x7780a925, BRF_GRA },	     //  16
	
	{ "tbp24s10.14k",  0x00100, 0x0eaf5158, BRF_GRA },	     //  17	PROMs
	{ "63s141.2e",     0x00100, 0x4a1285a4, BRF_GRA },	     //  18
	
	{ "gg-pal10l8.bin",0x0002c, 0x87f1b7e0, BRF_GRA },	     //  19	PLDs
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo DrvaRomDesc[] = {
	{ "gng.n10",       0x04000, 0x60343188, BRF_ESS | BRF_PRG }, //  0	M6809 Program Code
	{ "gng.n9",        0x04000, 0xb6b91cfb, BRF_ESS | BRF_PRG }, //	 1
	{ "gng.n8",        0x04000, 0xa5cfa928, BRF_ESS | BRF_PRG }, //	 2
	{ "gng.n13",       0x04000, 0xfd9a8dda, BRF_ESS | BRF_PRG }, //	 3
	{ "gng.n12",       0x04000, 0x13cf6238, BRF_ESS | BRF_PRG }, //	 4
	
	{ "gg2.bin",       0x08000, 0x615f5b6f, BRF_ESS | BRF_PRG }, //  5	Z80 Program 
	
	{ "gg1.bin",       0x04000, 0xecfccf07, BRF_GRA },	     //  6	Characters
	
	{ "gg11.bin",      0x04000, 0xddd56fa9, BRF_GRA },	     //  7	Tiles
	{ "gg10.bin",      0x04000, 0x7302529d, BRF_GRA },	     //  8
	{ "gg9.bin",       0x04000, 0x20035bda, BRF_GRA },	     //  9
	{ "gg8.bin",       0x04000, 0xf12ba271, BRF_GRA },	     //  10
	{ "gg7.bin",       0x04000, 0xe525207d, BRF_GRA },	     //  11
	{ "gg6.bin",       0x04000, 0x2d77e9b2, BRF_GRA },	     //  12
	
	{ "gg17.bin",      0x04000, 0x93e50a8f, BRF_GRA },	     //  13	Sprites
	{ "gg16.bin",      0x04000, 0x06d7e5ca, BRF_GRA },	     //  14
	{ "gg15.bin",      0x04000, 0xbc1fe02d, BRF_GRA },	     //  15
	{ "gg14.bin",      0x04000, 0x6aaf12f9, BRF_GRA },	     //  16
	{ "gg13.bin",      0x04000, 0xe80c3fca, BRF_GRA },	     //  17
	{ "gg12.bin",      0x04000, 0x7780a925, BRF_GRA },	     //  18
	
	{ "tbp24s10.14k",  0x00100, 0x0eaf5158, BRF_GRA },	     //  19	PROMs
	{ "63s141.2e",     0x00100, 0x4a1285a4, BRF_GRA },	     //  20
};

STD_ROM_PICK(Drva)
STD_ROM_FN(Drva)

static struct BurnRomInfo DrvcRomDesc[] = {
	{ "mm_c_04",       0x04000, 0x4f94130f, BRF_ESS | BRF_PRG }, //  0	M6809 Program Code
	{ "mm_c_03",       0x08000, 0x1def138a, BRF_ESS | BRF_PRG }, //	 1
	{ "mm_c_05",       0x08000, 0xed28e86e, BRF_ESS | BRF_PRG }, //	 2
	
	{ "gg2.bin",       0x08000, 0x615f5b6f, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "gg1.bin",       0x04000, 0xecfccf07, BRF_GRA },	     //  4	Characters
	
	{ "gg11.bin",      0x04000, 0xddd56fa9, BRF_GRA },	     //  5	Tiles
	{ "gg10.bin",      0x04000, 0x7302529d, BRF_GRA },	     //  6
	{ "gg9.bin",       0x04000, 0x20035bda, BRF_GRA },	     //  7
	{ "gg8.bin",       0x04000, 0xf12ba271, BRF_GRA },	     //  8
	{ "gg7.bin",       0x04000, 0xe525207d, BRF_GRA },	     //  9
	{ "gg6.bin",       0x04000, 0x2d77e9b2, BRF_GRA },	     //  10
	
	{ "gg17.bin",      0x04000, 0x93e50a8f, BRF_GRA },	     //  11	Sprites
	{ "gg16.bin",      0x04000, 0x06d7e5ca, BRF_GRA },	     //  12
	{ "gg15.bin",      0x04000, 0xbc1fe02d, BRF_GRA },	     //  13
	{ "gg14.bin",      0x04000, 0x6aaf12f9, BRF_GRA },	     //  14
	{ "gg13.bin",      0x04000, 0xe80c3fca, BRF_GRA },	     //  15
	{ "gg12.bin",      0x04000, 0x7780a925, BRF_GRA },	     //  16
	
	{ "tbp24s10.14k",  0x00100, 0x0eaf5158, BRF_GRA },	     //  17	PROMs
	{ "63s141.2e",     0x00100, 0x4a1285a4, BRF_GRA },	     //  18
};

STD_ROM_PICK(Drvc)
STD_ROM_FN(Drvc)

static struct BurnRomInfo DrvblRomDesc[] = {
	{ "5.84490.10n",   0x04000, 0x66606beb, BRF_ESS | BRF_PRG }, //  0	M6809 Program Code
	{ "4.84490.9n",    0x04000, 0x527f5c39, BRF_ESS | BRF_PRG }, //	 1
	{ "3.84490.8n",    0x04000, 0x1c5175d5, BRF_ESS | BRF_PRG }, //	 2
	{ "7.84490.13n",   0x04000, 0xfd9a8dda, BRF_ESS | BRF_PRG }, //	 3
	{ "6.84490.12n",   0x04000, 0xc83dbd10, BRF_ESS | BRF_PRG }, //	 4
	
	{ "2.8529.13h",    0x08000, 0x55cfb196, BRF_ESS | BRF_PRG }, //  5	Z80 Program 
	
	{ "1.84490.11e",   0x04000, 0xecfccf07, BRF_GRA },	     //  6	Characters
	
	{ "13.84490.3e",   0x04000, 0xddd56fa9, BRF_GRA },	     //  7	Tiles
	{ "12.84490.1e",   0x04000, 0x7302529d, BRF_GRA },	     //  8
	{ "11.84490.3c",   0x04000, 0x20035bda, BRF_GRA },	     //  9
	{ "10.84490.1c",   0x04000, 0xf12ba271, BRF_GRA },	     //  10
	{ "9.84490.3b",    0x04000, 0xe525207d, BRF_GRA },	     //  11
	{ "8.84490.1b",    0x04000, 0x2d77e9b2, BRF_GRA },	     //  12
	
	{ "19.84472.4n",   0x04000, 0x4613afdc, BRF_GRA },	     //  13	Sprites
	{ "18.84472.3n",   0x04000, 0x06d7e5ca, BRF_GRA },	     //  14
	{ "17.84472.1n",   0x04000, 0xbc1fe02d, BRF_GRA },	     //  15
	{ "16.84472.4l",   0x04000, 0x608d68d5, BRF_GRA },	     //  16
	{ "15.84490.3l",   0x04000, 0xe80c3fca, BRF_GRA },	     //  17
	{ "14.84490.1l",   0x04000, 0x7780a925, BRF_GRA },	     //  18
};

STD_ROM_PICK(Drvbl)
STD_ROM_FN(Drvbl)

static struct BurnRomInfo DrvblitaRomDesc[] = {
	{ "3",             0x04000, 0x4859d068, BRF_ESS | BRF_PRG }, //  0	M6809 Program Code
	{ "4-5",           0x08000, 0x233a4589, BRF_ESS | BRF_PRG }, //	 1
	{ "1-2",           0x08000, 0xed28e86e, BRF_ESS | BRF_PRG }, //	 2
	
	{ "gg2.bin",       0x08000, 0x615f5b6f, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "gg1.bin",       0x04000, 0xecfccf07, BRF_GRA },	     //  4	Characters
	
	{ "gg11.bin",      0x04000, 0xddd56fa9, BRF_GRA },	     //  5	Tiles
	{ "gg10.bin",      0x04000, 0x7302529d, BRF_GRA },	     //  6
	{ "gg9.bin",       0x04000, 0x20035bda, BRF_GRA },	     //  7
	{ "gg8.bin",       0x04000, 0xf12ba271, BRF_GRA },	     //  8
	{ "gg7.bin",       0x04000, 0xe525207d, BRF_GRA },	     //  9
	{ "gg6.bin",       0x04000, 0x2d77e9b2, BRF_GRA },	     //  10
	
	{ "gg17.bin",      0x04000, 0x93e50a8f, BRF_GRA },	     //  11	Sprites
	{ "gg16.bin",      0x04000, 0x06d7e5ca, BRF_GRA },	     //  12
	{ "gg15.bin",      0x04000, 0xbc1fe02d, BRF_GRA },	     //  13
	{ "gg14.bin",      0x04000, 0x6aaf12f9, BRF_GRA },	     //  14
	{ "gg13.bin",      0x04000, 0xe80c3fca, BRF_GRA },	     //  15
	{ "gg12.bin",      0x04000, 0x7780a925, BRF_GRA },	     //  16
	
	{ "tbp24s10.14k",  0x00100, 0x0eaf5158, BRF_GRA },	     //  17	PROMs
	{ "63s141.2e",     0x00100, 0x4a1285a4, BRF_GRA },	     //  18
	
	{ "gg-pal10l8.bin",0x0002c, 0x87f1b7e0, BRF_GRA },	     //  19	PLDs
};

STD_ROM_PICK(Drvblita)
STD_ROM_FN(Drvblita)

static struct BurnRomInfo DrvtRomDesc[] = {
	{ "mm04",          0x04000, 0x652406f6, BRF_ESS | BRF_PRG }, //  0	M6809 Program Code
	{ "mm03",          0x08000, 0xfb040b42, BRF_ESS | BRF_PRG }, //	 1
	{ "mm05",          0x08000, 0x8f7cff61, BRF_ESS | BRF_PRG }, //	 2
	
	{ "gg2.bin",       0x08000, 0x615f5b6f, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "gg1.bin",       0x04000, 0xecfccf07, BRF_GRA },	     //  4	Characters
	
	{ "gg11.bin",      0x04000, 0xddd56fa9, BRF_GRA },	     //  5	Tiles
	{ "gg10.bin",      0x04000, 0x7302529d, BRF_GRA },	     //  6
	{ "gg9.bin",       0x04000, 0x20035bda, BRF_GRA },	     //  7
	{ "gg8.bin",       0x04000, 0xf12ba271, BRF_GRA },	     //  8
	{ "gg7.bin",       0x04000, 0xe525207d, BRF_GRA },	     //  9
	{ "gg6.bin",       0x04000, 0x2d77e9b2, BRF_GRA },	     //  10
	
	{ "gg17.bin",      0x04000, 0x93e50a8f, BRF_GRA },	     //  11	Sprites
	{ "gg16.bin",      0x04000, 0x06d7e5ca, BRF_GRA },	     //  12
	{ "gg15.bin",      0x04000, 0xbc1fe02d, BRF_GRA },	     //  13
	{ "gg14.bin",      0x04000, 0x6aaf12f9, BRF_GRA },	     //  14
	{ "gg13.bin",      0x04000, 0xe80c3fca, BRF_GRA },	     //  15
	{ "gg12.bin",      0x04000, 0x7780a925, BRF_GRA },	     //  16
	
	{ "tbp24s10.14k",  0x00100, 0x0eaf5158, BRF_GRA },	     //  17	PROMs
	{ "63s141.2e",     0x00100, 0x4a1285a4, BRF_GRA },	     //  18
};

STD_ROM_PICK(Drvt)
STD_ROM_FN(Drvt)

static struct BurnRomInfo MakaimurRomDesc[] = {
	{ "10n.rom",       0x04000, 0x81e567e0, BRF_ESS | BRF_PRG }, //  0	M6809 Program Code
	{ "8n.rom",        0x08000, 0x9612d66c, BRF_ESS | BRF_PRG }, //	 1
	{ "12n.rom",       0x08000, 0x65a6a97b, BRF_ESS | BRF_PRG }, //	 2
	
	{ "gg2.bin",       0x08000, 0x615f5b6f, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "gg1.bin",       0x04000, 0xecfccf07, BRF_GRA },	     //  4	Characters
	
	{ "gg11.bin",      0x04000, 0xddd56fa9, BRF_GRA },	     //  5	Tiles
	{ "gg10.bin",      0x04000, 0x7302529d, BRF_GRA },	     //  6
	{ "gg9.bin",       0x04000, 0x20035bda, BRF_GRA },	     //  7
	{ "gg8.bin",       0x04000, 0xf12ba271, BRF_GRA },	     //  8
	{ "gg7.bin",       0x04000, 0xe525207d, BRF_GRA },	     //  9
	{ "gg6.bin",       0x04000, 0x2d77e9b2, BRF_GRA },	     //  10
	
	{ "gng13.n4",      0x04000, 0x4613afdc, BRF_GRA },	     //  11	Sprites
	{ "gg16.bin",      0x04000, 0x06d7e5ca, BRF_GRA },	     //  12
	{ "gg15.bin",      0x04000, 0xbc1fe02d, BRF_GRA },	     //  13
	{ "gng16.l4",      0x04000, 0x608d68d5, BRF_GRA },	     //  14
	{ "gg13.bin",      0x04000, 0xe80c3fca, BRF_GRA },	     //  15
	{ "gg12.bin",      0x04000, 0x7780a925, BRF_GRA },	     //  16
	
	{ "tbp24s10.14k",  0x00100, 0x0eaf5158, BRF_GRA },	     //  17	PROMs
	{ "63s141.2e",     0x00100, 0x4a1285a4, BRF_GRA },	     //  18
};

STD_ROM_PICK(Makaimur)
STD_ROM_FN(Makaimur)

static struct BurnRomInfo MakaimucRomDesc[] = {
	{ "mj04c.bin",     0x04000, 0x1294edb1, BRF_ESS | BRF_PRG }, //  0	M6809 Program Code
	{ "mj03c.bin",     0x08000, 0xd343332d, BRF_ESS | BRF_PRG }, //	 1
	{ "mj05c.bin",     0x08000, 0x535342c2, BRF_ESS | BRF_PRG }, //	 2
	
	{ "gg2.bin",       0x08000, 0x615f5b6f, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "gg1.bin",       0x04000, 0xecfccf07, BRF_GRA },	     //  4	Characters
	
	{ "gg11.bin",      0x04000, 0xddd56fa9, BRF_GRA },	     //  5	Tiles
	{ "gg10.bin",      0x04000, 0x7302529d, BRF_GRA },	     //  6
	{ "gg9.bin",       0x04000, 0x20035bda, BRF_GRA },	     //  7
	{ "gg8.bin",       0x04000, 0xf12ba271, BRF_GRA },	     //  8
	{ "gg7.bin",       0x04000, 0xe525207d, BRF_GRA },	     //  9
	{ "gg6.bin",       0x04000, 0x2d77e9b2, BRF_GRA },	     //  10
	
	{ "gng13.n4",      0x04000, 0x4613afdc, BRF_GRA },	     //  11	Sprites
	{ "gg16.bin",      0x04000, 0x06d7e5ca, BRF_GRA },	     //  12
	{ "gg15.bin",      0x04000, 0xbc1fe02d, BRF_GRA },	     //  13
	{ "gng16.l4",      0x04000, 0x608d68d5, BRF_GRA },	     //  14
	{ "gg13.bin",      0x04000, 0xe80c3fca, BRF_GRA },	     //  15
	{ "gg12.bin",      0x04000, 0x7780a925, BRF_GRA },	     //  16
	
	{ "tbp24s10.14k",  0x00100, 0x0eaf5158, BRF_GRA },	     //  17	PROMs
	{ "63s141.2e",     0x00100, 0x4a1285a4, BRF_GRA },	     //  18
};

STD_ROM_PICK(Makaimuc)
STD_ROM_FN(Makaimuc)

static struct BurnRomInfo MakaimugRomDesc[] = {
	{ "mj04g.bin",     0x04000, 0x757c94d3, BRF_ESS | BRF_PRG }, //  0	M6809 Program Code
	{ "mj03g.bin",     0x08000, 0x61b043bb, BRF_ESS | BRF_PRG }, //	 1
	{ "mj05g.bin",     0x08000, 0xf2fdccf5, BRF_ESS | BRF_PRG }, //	 2
	
	{ "gg2.bin",       0x08000, 0x615f5b6f, BRF_ESS | BRF_PRG }, //  3	Z80 Program 
	
	{ "gg1.bin",       0x04000, 0xecfccf07, BRF_GRA },	     //  4	Characters
	
	{ "gg11.bin",      0x04000, 0xddd56fa9, BRF_GRA },	     //  5	Tiles
	{ "gg10.bin",      0x04000, 0x7302529d, BRF_GRA },	     //  6
	{ "gg9.bin",       0x04000, 0x20035bda, BRF_GRA },	     //  7
	{ "gg8.bin",       0x04000, 0xf12ba271, BRF_GRA },	     //  8
	{ "gg7.bin",       0x04000, 0xe525207d, BRF_GRA },	     //  9
	{ "gg6.bin",       0x04000, 0x2d77e9b2, BRF_GRA },	     //  10
	
	{ "gng13.n4",      0x04000, 0x4613afdc, BRF_GRA },	     //  11	Sprites
	{ "gg16.bin",      0x04000, 0x06d7e5ca, BRF_GRA },	     //  12
	{ "gg15.bin",      0x04000, 0xbc1fe02d, BRF_GRA },	     //  13
	{ "gng16.l4",      0x04000, 0x608d68d5, BRF_GRA },	     //  14
	{ "gg13.bin",      0x04000, 0xe80c3fca, BRF_GRA },	     //  15
	{ "gg12.bin",      0x04000, 0x7780a925, BRF_GRA },	     //  16
	
	{ "tbp24s10.14k",  0x00100, 0x0eaf5158, BRF_GRA },	     //  17	PROMs
	{ "63s141.2e",     0x00100, 0x4a1285a4, BRF_GRA },	     //  18
};

STD_ROM_PICK(Makaimug)
STD_ROM_FN(Makaimug)

static struct BurnRomInfo DiamondRomDesc[] = {
	{ "d3o",           0x04000, 0xba4bf9f1, BRF_ESS | BRF_PRG }, //  0	M6809 Program Code
	{ "d3",            0x08000, 0xf436d6fa, BRF_ESS | BRF_PRG }, //	 1
	{ "d5o",           0x08000, 0xae58bd3a, BRF_ESS | BRF_PRG }, //	 2
	{ "d5",            0x08000, 0x453f3f9e, BRF_ESS | BRF_PRG }, //	 3
	
	{ "d2",            0x08000, 0x615f5b6f, BRF_ESS | BRF_PRG }, //  4	Z80 Program 
	
	{ "d1",            0x04000, 0x3a24e504, BRF_GRA },	     //  5	Characters
	
	{ "d11",           0x04000, 0x754357d7, BRF_GRA },	     //  6	Tiles
	{ "d10",           0x04000, 0x7531edcd, BRF_GRA },	     //  7
	{ "d9",            0x04000, 0x22eeca08, BRF_GRA },	     //  8
	{ "d8",            0x04000, 0x6b61be60, BRF_GRA },	     //  9
	{ "d7",            0x04000, 0xfd595274, BRF_GRA },	     //  10
	{ "d6",            0x04000, 0x7f51dcd2, BRF_GRA },	     //  11
	
	{ "d17",           0x04000, 0x8164b005, BRF_GRA },	     //  12	Sprites
	{ "d14",           0x04000, 0x6f132163, BRF_GRA },	     //  13
	
	{ "prom1",         0x00100, 0x0eaf5158, BRF_GRA },	     //  14	PROMs
	{ "prom2",         0x00100, 0x4a1285a4, BRF_GRA },	     //  15
};

STD_ROM_PICK(Diamond)
STD_ROM_FN(Diamond)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvM6809Rom            = Next; Next += 0x14000;
	DrvZ80Rom              = Next; Next += 0x08000;

	RamStart               = Next;

	DrvM6809Ram            = Next; Next += 0x01e00;
	DrvZ80Ram              = Next; Next += 0x00800;
	DrvSpriteRam           = Next; Next += 0x00200;
	DrvSpriteRamBuffer     = Next; Next += 0x00200;
	DrvFgVideoRam          = Next; Next += 0x00800;
	DrvBgVideoRam          = Next; Next += 0x00800;
	DrvPaletteRam1         = Next; Next += 0x00100;
	DrvPaletteRam2         = Next; Next += 0x00100;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x400 * 8 * 8;
	DrvTiles               = Next; Next += 0x400 * 16 * 16;
	DrvSprites             = Next; Next += 0x400 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00100 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	M6809Open(0);
	M6809Reset();
	M6809Close();
	
	ZetOpen(0);
	ZetReset();
	ZetClose();
	
	BurnYM2203Reset();
	
	DrvRomBank = 0;
	DrvBgScrollX[0] = DrvBgScrollX[1] = 0;
	DrvBgScrollY[0] = DrvBgScrollY[1] = 0;
	DrvSoundLatch = 0;

	return 0;
}

UINT8 DrvGngM6809ReadByte(UINT16 Address)
{
	switch (Address) {
		case 0x3000: {
			return 0xff - DrvInput[0];
		}
		
		case 0x3001: {
			return 0xff - DrvInput[1];
		}
		
		case 0x3002: {
			return 0xff - DrvInput[2];
		}
		
		case 0x3003: {
			return DrvDip[0];
		}
		
		case 0x3004: {
			return DrvDip[1];
		}
		
		case 0x3c00: {
			// nop
			return 0;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("M6809 Read Byte -> %04X\n"), Address);
	
	return 0;
}


void DrvGngM6809WriteByte(UINT16 Address, UINT8 Data)
{
	switch (Address) {
		case 0x3a00: {
			DrvSoundLatch = Data;
			return;
		}
		
		case 0x3b08: {
			DrvBgScrollX[0] = Data;
			return;
		}
		
		case 0x3b09: {
			DrvBgScrollX[1] = Data;
			return;
		}
		
		case 0x3b0a: {
			DrvBgScrollY[0] = Data;
			return;
		}
		
		case 0x3b0b: {
			DrvBgScrollY[1] = Data;
			return;
		}
		
		case 0x3c00: {
			// nop
			return;
		}
		
		case 0x3d00: {
			// flipscreen
			return;
		}
		
		case 0x3d01: {
			// ???
			return;
		}
		
		case 0x3e00: {
			DrvRomBank = Data & 3;
			if (Data == 4) {
				DrvRomBank = 4;
				M6809MapMemory(DrvM6809Rom, 0x4000, 0x5fff, M6809_ROM);
			} else {
				M6809MapMemory(DrvM6809Rom + 0xc000 + (DrvRomBank * 0x2000), 0x4000, 0x5fff, M6809_ROM);
			}
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("M6809 Write Byte -> %04X, %02X\n"), Address, Data);
}

UINT8 __fastcall DrvGngZ80Read(UINT16 a)
{
	switch (a) {
		case 0xc800: {
			return DrvSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall DrvGngZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xe000: {
			BurnYM2203Write(0, 0, d);
			return;
		}
		
		case 0xe001: {
			BurnYM2203Write(0, 1, d);
			return;
		}
		
		case 0xe002: {
			BurnYM2203Write(1, 0, d);
			return;
		}
		
		case 0xe003: {
			BurnYM2203Write(1, 1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static INT32 CharPlaneOffsets[2]   = { 4, 0 };
static INT32 CharXOffsets[8]       = { 0, 1, 2, 3, 8, 9, 10, 11 };
static INT32 CharYOffsets[8]       = { 0, 16, 32, 48, 64, 80, 96, 112 };
static INT32 TilePlaneOffsets[3]   = { 0x80000, 0x40000, 0 };
static INT32 TileXOffsets[16]      = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
static INT32 TileYOffsets[16]      = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };
static INT32 SpritePlaneOffsets[4] = { 0x80004, 0x80000, 4, 0 };
static INT32 SpriteXOffsets[16]    = { 0, 1, 2, 3, 8, 9, 10, 11, 256, 257, 258, 259, 264, 265, 266, 267 };
static INT32 SpriteYOffsets[16]    = { 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240 };

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)(ZetTotalCycles() * nSoundRate / 3000000);
}

inline static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 3000000;
}

static INT32 DrvInit()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x20000);

	// Load M6809 Program Roms
	if (RomLoadOffset == 2) {
		nRet = BurnLoadRom(DrvM6809Rom + 0x00000, 0, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvM6809Rom + 0x04000, 1, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvM6809Rom + 0x08000, 2, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvM6809Rom + 0x0c000, 3, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvM6809Rom + 0x10000, 4, 1); if (nRet != 0) return 1;
	} else {
		nRet = BurnLoadRom(DrvM6809Rom + 0x00000, 0, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvM6809Rom + 0x04000, 1, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvM6809Rom + 0x0c000, 2, 1); if (nRet != 0) return 1;
	}
		
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom + 0x00000, 3 + RomLoadOffset, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 4 + RomLoadOffset, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x20000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5 + RomLoadOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000,  6 + RomLoadOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000,  7 + RomLoadOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c000,  8 + RomLoadOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  9 + RomLoadOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x14000, 10 + RomLoadOffset, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 3, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0xff, 0x20000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11 + RomLoadOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000, 12 + RomLoadOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 13 + RomLoadOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 14 + RomLoadOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x14000, 15 + RomLoadOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 16 + RomLoadOffset, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	// Setup the M6809 emulation
	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvM6809Ram          , 0x0000, 0x1dff, M6809_RAM);
	M6809MapMemory(DrvSpriteRam         , 0x1e00, 0x1fff, M6809_RAM);
	M6809MapMemory(DrvFgVideoRam        , 0x2000, 0x27ff, M6809_RAM);
	M6809MapMemory(DrvBgVideoRam        , 0x2800, 0x2fff, M6809_RAM);
	M6809MapMemory(DrvPaletteRam2       , 0x3800, 0x38ff, M6809_RAM);
	M6809MapMemory(DrvPaletteRam1       , 0x3900, 0x39ff, M6809_RAM);
	M6809MapMemory(DrvM6809Rom          , 0x4000, 0x5fff, M6809_ROM);
	M6809MapMemory(DrvM6809Rom + 0x2000 , 0x6000, 0xffff, M6809_ROM);
	M6809SetReadHandler(DrvGngM6809ReadByte);
	M6809SetWriteHandler(DrvGngM6809WriteByte);
	M6809Close();
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(DrvGngZ80Read);
	ZetSetWriteHandler(DrvGngZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom             );
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80Ram             );
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80Ram             );
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80Ram             );
	ZetMemEnd();
	ZetClose();	
	
	BurnYM2203Init(2, 1500000, NULL, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(3000000);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_YM2203_ROUTE, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_1, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_2, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_3, 0.40, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 DiamondInit()
{
	INT32 nRet = 0, nLen;
	
	Diamond = 1;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x20000);

	// Load M6809 Program Roms
	nRet = BurnLoadRom(DrvM6809Rom + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvM6809Rom + 0x04000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvM6809Rom + 0x0c000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvM6809Rom + 0x14000, 3, 1); if (nRet != 0) return 1;
		
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom + 0x00000, 4, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 5, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x20000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x14000, 11, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 3, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0xff, 0x20000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 13 + RomLoadOffset, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	// Setup the M6809 emulation
	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvM6809Ram          , 0x0000, 0x1dff, M6809_RAM);
	M6809MapMemory(DrvSpriteRam         , 0x1e00, 0x1fff, M6809_RAM);
	M6809MapMemory(DrvFgVideoRam        , 0x2000, 0x27ff, M6809_RAM);
	M6809MapMemory(DrvBgVideoRam        , 0x2800, 0x2fff, M6809_RAM);
	M6809MapMemory(DrvPaletteRam2       , 0x3800, 0x38ff, M6809_RAM);
	M6809MapMemory(DrvPaletteRam1       , 0x3900, 0x39ff, M6809_RAM);
	M6809MapMemory(DrvM6809Rom          , 0x4000, 0x5fff, M6809_ROM);
	M6809MapMemory(DrvM6809Rom + 0x2000 , 0x6000, 0xffff, M6809_ROM);
	M6809SetReadHandler(DrvGngM6809ReadByte);
	M6809SetWriteHandler(DrvGngM6809WriteByte);
	M6809Close();
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(DrvGngZ80Read);
	ZetSetWriteHandler(DrvGngZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom             );
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80Ram             );
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80Ram             );
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80Ram             );
	ZetMemEnd();
	ZetClose();	
	
	BurnYM2203Init(2, 1500000, NULL, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(3000000);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_YM2203_ROUTE, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_1, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_2, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_3, 0.40, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();
	
	DrvM6809Rom[0x2000] = 0;

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 GngaInit()
{
	RomLoadOffset = 2;
	
	return DrvInit();
}

static INT32 DrvExit()
{
	M6809Exit();
	ZetExit();
	
	BurnYM2203Exit();
	
	GenericTilesExit();
	
	BurnFree(Mem);
	
	DrvRomBank = 0;
	DrvBgScrollX[0] = DrvBgScrollX[1] = 0;
	DrvBgScrollY[0] = DrvBgScrollY[1] = 0;
	DrvSoundLatch = 0;
	
	RomLoadOffset = 0;
	Diamond = 0;

	return 0;
}

static inline UINT8 pal4bit(UINT8 bits)
{
	bits &= 0x0f;
	return (bits << 4) | bits;
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal4bit(nColour >> 12);
	g = pal4bit(nColour >>  8);
	b = pal4bit(nColour >>  4);

	return BurnHighCol(r, g, b, 0);
}

static void DrvCalcPalette()
{
	for (INT32 i = 0; i < 0x100; i++) {
		INT32 Val = DrvPaletteRam1[i] + (DrvPaletteRam2[i] << 8);
		
		DrvPalette[i] = CalcCol(Val);
	}
}

static void DrvRenderBgLayer(INT32 Priority, INT32 Opaque)
{
	INT32 mx, my, Code, Attr, Colour, x, y, TileIndex, xScroll, yScroll, Split, Flip, xFlip, yFlip;
	
	xScroll = DrvBgScrollX[0] | (DrvBgScrollX[1] << 8);
	xScroll &= 0x1ff;
	
	yScroll = DrvBgScrollY[0] | (DrvBgScrollY[1] << 8);
	yScroll &= 0x1ff;
	
	for (mx = 0; mx < 32; mx++) {
		for (my = 0; my < 32; my++) {
			TileIndex = (my * 32) + mx;
			
			Attr = DrvBgVideoRam[TileIndex + 0x400];
			Code = DrvBgVideoRam[TileIndex + 0x000];
			
			Code += (Attr & 0xc0) << 2;
			Colour = Attr & 0x07;
			
			Split = (Attr & 0x08) >> 3;
			
			if (Split != Priority) continue;
			
			Flip = (Attr & 0x30) >> 4;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			y = 16 * mx;
			x = 16 * my;

			x -= xScroll;
			if (x < -16) x += 512;
			y -= yScroll;
			if (y < -16) y += 512;
			
			y -= 16;

			if (Opaque) {
				if (x > 16 && x < 240 && y > 16 && y < 208) {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_FlipXY(pTransDraw, Code, x, y, Colour, 3, 0, DrvTiles);
						} else {
							Render16x16Tile_FlipX(pTransDraw, Code, x, y, Colour, 3, 0, DrvTiles);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_FlipY(pTransDraw, Code, x, y, Colour, 3, 0, DrvTiles);
						} else {
							Render16x16Tile(pTransDraw, Code, x, y, Colour, 3, 0, DrvTiles);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 3, 0, DrvTiles);
						} else {
							Render16x16Tile_FlipX_Clip(pTransDraw, Code, x, y, Colour, 3, 0, DrvTiles);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_FlipY_Clip(pTransDraw, Code, x, y, Colour, 3, 0, DrvTiles);
						} else {
							Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 3, 0, DrvTiles);
						}
					}
				}
			} else {
				if (x > 16 && x < 240 && y > 16 && y < 208) {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 3, 0, 0, DrvTiles);
						} else {
							Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 3, 0, 0, DrvTiles);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 3, 0, 0, DrvTiles);
						} else {
							Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 3, 0, 0, DrvTiles);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 3, 0, 0, DrvTiles);
						} else {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 3, 0, 0, DrvTiles);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 3, 0, 0, DrvTiles);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 3, 0, 0, DrvTiles);
						}
					}
				}
			}
		}
	}
}

static void DrvRenderSprites()
{
	for (INT32 Offs = 0x200 - 4; Offs >= 0; Offs -= 4) {
		UINT8 Attr = DrvSpriteRamBuffer[Offs + 1];
		INT32 sx = DrvSpriteRamBuffer[Offs + 3] - (0x100 * (Attr & 0x01));
		INT32 sy = DrvSpriteRamBuffer[Offs + 2];
		INT32 xFlip = Attr & 0x04;
		INT32 yFlip = Attr & 0x08;
		INT32 Code = DrvSpriteRamBuffer[Offs + 0] + ((Attr << 2) & 0x300);
		INT32 Colour = (Attr >> 4) & 3;
		
		if (sx > 16 && sx < 240 && sy > 16 && sy < 208) {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Code, sx, sy - 16, Colour, 4, 15, 0x40, DrvSprites);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Code, sx, sy - 16, Colour, 4, 15, 0x40, DrvSprites);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Code, sx, sy - 16, Colour, 4, 15, 0x40, DrvSprites);
				} else {
					Render16x16Tile_Mask(pTransDraw, Code, sx, sy - 16, Colour, 4, 15, 0x40, DrvSprites);
				}
			}
		} else {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx, sy - 16, Colour, 4, 15, 0x40, DrvSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, sx, sy - 16, Colour, 4, 15, 0x40, DrvSprites);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, sx, sy - 16, Colour, 4, 15, 0x40, DrvSprites);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, sy - 16, Colour, 4, 15, 0x40, DrvSprites);
				}
			}
		}
	}
}

static void DrvRenderCharLayer()
{
	INT32 mx, my, Code, Attr, Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Attr = DrvFgVideoRam[TileIndex + 0x400];
			Code = DrvFgVideoRam[TileIndex + 0x000];
			
			Code += (Attr & 0xc0) << 2;
			Colour = Attr & 0x0f;
			
			Flip = (Attr & 0x30) >> 4;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
			
			y -= 16;
			
			if (x > 8 && x < 248 && y > 8 && y < 216) {
				if (xFlip) {
					if (yFlip) {
						Render8x8Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 2, 3, 0x80, DrvChars);
					} else {
						Render8x8Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 2, 3, 0x80, DrvChars);
					}
				} else {
					if (yFlip) {
						Render8x8Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 2, 3, 0x80, DrvChars);
					} else {
						Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 2, 3, 0x80, DrvChars);
					}
				}
			} else {
				if (xFlip) {
					if (yFlip) {
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 2, 3, 0x80, DrvChars);
					} else {
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 2, 3, 0x80, DrvChars);
					}
				} else {
					if (yFlip) {
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 2, 3, 0x80, DrvChars);
					} else {
						Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 2, 3, 0x80, DrvChars);
					}
				}
			}

			TileIndex++;
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	DrvRenderBgLayer(0, 1);
	DrvRenderSprites();
	DrvRenderBgLayer(1, 0);
	DrvRenderCharLayer();
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 25;
	INT32 nSoundBufferPos = 0;
	
	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	nCyclesTotal[0] = 1500000 / 60;
	nCyclesTotal[1] = 3000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	ZetNewFrame();

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;
		
		// Run M6809
		nCurrentCPU = 0;
		M6809Open(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += M6809Run(nCyclesSegment);
		if (i == 24) {
			M6809SetIRQLine(0, M6809_IRQSTATUS_AUTO);
		}
		M6809Close();
		
		// Run Z80
		nCurrentCPU = 1;
		ZetOpen(0);
		BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));
		if (i == 5 || i == 10 || i == 15 || i == 20) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		if (i == 6 || i == 11 || i == 16 || i == 21) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		ZetClose();
		
		// Render Sound Segment
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			ZetOpen(0);
			BurnYM2203Update(pSoundBuf, nSegmentLength);
			ZetClose();
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	ZetOpen(0);
	BurnTimerEndFrame(nCyclesTotal[1]);
	ZetClose();
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			ZetOpen(0);
			BurnYM2203Update(pSoundBuf, nSegmentLength);
			ZetClose();
		}
	}
	
	if (pBurnDraw) DrvDraw();
	
	memcpy(DrvSpriteRamBuffer, DrvSpriteRam, 0x200);

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029696;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		M6809Scan(nAction);
		ZetScan(nAction);
		
		BurnYM2203Scan(nAction, pnMin);

		// Scan critical driver variables
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
		SCAN_VAR(DrvRomBank);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(DrvBgScrollX);
		SCAN_VAR(DrvBgScrollY);
		SCAN_VAR(DrvDip);
		SCAN_VAR(DrvInput);
	}
	
	return 0;
}

struct BurnDriver BurnDrvGng = {
	"gng", NULL, NULL, NULL, "1985",
	"Ghosts'n Goblins (World? set 1)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvGnga = {
	"gnga", "gng", NULL, NULL, "1985",
	"Ghosts'n Goblins (World? set 2)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, DrvaRomInfo, DrvaRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	GngaInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvGngbl = {
	"gngbl", "gng", NULL, NULL, "1985",
	"Ghosts'n Goblins (bootleg with Cross)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, DrvblRomInfo, DrvblRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	GngaInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvGngblita = {
	"gngblita", "gng", NULL, NULL, "1985",
	"Ghosts'n Goblins (Italian bootleg, harder)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, DrvblitaRomInfo, DrvblitaRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvGngt = {
	"gngt", "gng", NULL, NULL, "1985",
	"Ghosts'n Goblins (US)\0", NULL, "Capcom (Taito America License)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, DrvtRomInfo, DrvtRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvGngc = {
	"gngc", "gng", NULL, NULL, "1985",
	"Ghosts'n Goblins (World Revision C)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, DrvcRomInfo, DrvcRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvMakaimur = {
	"makaimur", "gng", NULL, NULL, "1985",
	"Makai-Mura (Japan)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, MakaimurRomInfo, MakaimurRomName, NULL, NULL, DrvInputInfo, DrvjDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvMakaimuc = {
	"makaimurc", "gng", NULL, NULL, "1985",
	"Makai-Mura (Japan revision C)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, MakaimucRomInfo, MakaimucRomName, NULL, NULL, DrvInputInfo, DrvjDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvMakaimug = {
	"makaimurg", "gng", NULL, NULL, "1985",
	"Makai-Mura (Japan revision G)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, MakaimugRomInfo, MakaimugRomName, NULL, NULL, DrvInputInfo, DrvjDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvDiamond = {
	"diamond", NULL, NULL, NULL, "1985",
	"Diamond Run\0", NULL, "KH Video", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, DiamondRomInfo, DiamondRomName, NULL, NULL, DiamondInputInfo, DiamondDIPInfo,
	DiamondInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};
