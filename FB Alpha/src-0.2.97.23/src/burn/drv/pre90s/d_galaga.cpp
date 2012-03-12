#include "tiles_generic.h"
#include "namco_snd.h"
#include "samples.h"

static UINT8 DrvInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[3]        = {0, 0, 0};
static UINT8 DrvInput[3]      = {0x00, 0x00, 0x00};
static UINT8 DrvReset         = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *DrvZ80Rom1          = NULL;
static UINT8 *DrvZ80Rom2          = NULL;
static UINT8 *DrvZ80Rom3          = NULL;
static UINT8 *DrvVideoRam         = NULL;
static UINT8 *DrvSharedRam1       = NULL;
static UINT8 *DrvSharedRam2       = NULL;
static UINT8 *DrvSharedRam3       = NULL;
static UINT8 *DrvPromPalette      = NULL;
static UINT8 *DrvPromCharLookup   = NULL;
static UINT8 *DrvPromSpriteLookup = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette          = NULL;

static UINT8 DrvCPU1FireIRQ;
static UINT8 DrvCPU2FireIRQ;
static UINT8 DrvCPU3FireIRQ;
static UINT8 DrvCPU2Halt;
static UINT8 DrvCPU3Halt;
static UINT8 DrvFlipScreen;
static UINT8 DrvStarControl[6];
static UINT32  DrvStarScrollX;
static UINT32  DrvStarScrollY;

static UINT8 IOChipCustomCommand;
static UINT8 IOChipCPU1FireIRQ;
static UINT8 IOChipMode;
static UINT8 IOChipCredits;
static UINT8 IOChipCoinPerCredit;
static UINT8 IOChipCreditPerCoin;
static UINT8 IOChipCustom[16];
static UINT8 PrevInValue;

static INT32 nCyclesDone[3], nCyclesTotal[3];
static INT32 nCyclesSegment;

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 5, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 3, "p2 start"  },

	{"Left"              , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , DrvInputPort1 + 4, "p1 fire 1" },
	
	{"Left (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0 + 6, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Drv)

static inline void DrvMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = 0x7f;
	DrvInput[1] = 0xff;
	DrvInput[2] = 0xff;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] -= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] -= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] -= (DrvInputPort2[i] & 1) << i;
	}
}

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x80, NULL                     },
	{0x0d, 0xff, 0xff, 0xf7, NULL                     },
	{0x0e, 0xff, 0xff, 0x97, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x80, 0x80, "Off"                    },
	{0x0c, 0x01, 0x80, 0x00, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x0d, 0x01, 0x03, 0x03, "Easy"                   },
	{0x0d, 0x01, 0x03, 0x00, "Medium"                 },
	{0x0d, 0x01, 0x03, 0x01, "Hard"                   },
	{0x0d, 0x01, 0x03, 0x02, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0d, 0x01, 0x08, 0x08, "Off"                    },
	{0x0d, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0d, 0x01, 0x10, 0x10, "Off"                    },
	{0x0d, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Rack Test"              },
	{0x0d, 0x01, 0x20, 0x20, "Off"                    },
	{0x0d, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0d, 0x01, 0x80, 0x80, "Upright"                },
	{0x0d, 0x01, 0x80, 0x00, "Cocktail"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x0e, 0x01, 0x07, 0x04, "4 Coins 1 Play"         },
	{0x0e, 0x01, 0x07, 0x02, "3 Coins 1 Play"         },
	{0x0e, 0x01, 0x07, 0x06, "2 Coins 1 Play"         },
	{0x0e, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x0e, 0x01, 0x07, 0x01, "2 Coins 3 Plays"        },
	{0x0e, 0x01, 0x07, 0x03, "1 Coin  2 Plays"        },
	{0x0e, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x0e, 0x01, 0x07, 0x00, "Freeplay"               },	
	
	{0   , 0xfe, 0   , 8   , "Bonus Life"             },
	{0x0e, 0x01, 0x38, 0x20, "20k  60k  60k"          },
	{0x0e, 0x01, 0x38, 0x18, "20k  60k"               },
	{0x0e, 0x01, 0x38, 0x10, "20k  70k  70k"          },
	{0x0e, 0x01, 0x38, 0x30, "20k  80k  80k"          },
	{0x0e, 0x01, 0x38, 0x38, "30k  80k"               },
	{0x0e, 0x01, 0x38, 0x08, "30k 100k 100k"          },
	{0x0e, 0x01, 0x38, 0x28, "30k 120k 120k"          },
	{0x0e, 0x01, 0x38, 0x00, "None"                   },	
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0xc0, 0x00, "2"                      },
	{0x0e, 0x01, 0xc0, 0x80, "3"                      },
	{0x0e, 0x01, 0xc0, 0x40, "4"                      },
	{0x0e, 0x01, 0xc0, 0xc0, "5"                      },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo GalagamwDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0x80, NULL                     },
	{0x0d, 0xff, 0xff, 0xf7, NULL                     },
	{0x0e, 0xff, 0xff, 0x97, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0c, 0x01, 0x80, 0x80, "Off"                    },
	{0x0c, 0x01, 0x80, 0x00, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 2   , "2 Credits Game"         },
	{0x0d, 0x01, 0x01, 0x00, "1 Player"               },
	{0x0d, 0x01, 0x01, 0x01, "2 Players"              },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x0d, 0x01, 0x06, 0x06, "Easy"                   },
	{0x0d, 0x01, 0x06, 0x00, "Medium"                 },
	{0x0d, 0x01, 0x06, 0x02, "Hard"                   },
	{0x0d, 0x01, 0x06, 0x04, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0d, 0x01, 0x08, 0x08, "Off"                    },
	{0x0d, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0d, 0x01, 0x10, 0x10, "Off"                    },
	{0x0d, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Rack Test"              },
	{0x0d, 0x01, 0x20, 0x20, "Off"                    },
	{0x0d, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0d, 0x01, 0x80, 0x80, "Upright"                },
	{0x0d, 0x01, 0x80, 0x00, "Cocktail"               },
	
	// Dip 3	
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x0e, 0x01, 0x07, 0x04, "4 Coins 1 Play"         },
	{0x0e, 0x01, 0x07, 0x02, "3 Coins 1 Play"         },
	{0x0e, 0x01, 0x07, 0x06, "2 Coins 1 Play"         },
	{0x0e, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x0e, 0x01, 0x07, 0x01, "2 Coins 3 Plays"        },
	{0x0e, 0x01, 0x07, 0x03, "1 Coin  2 Plays"        },
	{0x0e, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x0e, 0x01, 0x07, 0x00, "Freeplay"               },	
	
	{0   , 0xfe, 0   , 8   , "Bonus Life"             },
	{0x0e, 0x01, 0x38, 0x20, "20k  60k  60k"          },
	{0x0e, 0x01, 0x38, 0x18, "20k  60k"               },
	{0x0e, 0x01, 0x38, 0x10, "20k  70k  70k"          },
	{0x0e, 0x01, 0x38, 0x30, "20k  80k  80k"          },
	{0x0e, 0x01, 0x38, 0x38, "30k  80k"               },
	{0x0e, 0x01, 0x38, 0x08, "30k 100k 100k"          },
	{0x0e, 0x01, 0x38, 0x28, "30k 120k 120k"          },
	{0x0e, 0x01, 0x38, 0x00, "None"                   },	
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x0e, 0x01, 0xc0, 0x00, "2"                      },
	{0x0e, 0x01, 0xc0, 0x80, "3"                      },
	{0x0e, 0x01, 0xc0, 0x40, "4"                      },
	{0x0e, 0x01, 0xc0, 0xc0, "5"                      },
};

STDDIPINFO(Galagamw)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "gg1_1b.3p",     0x01000, 0xab036c9f, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "gg1_2b.3m",     0x01000, 0xd9232240, BRF_ESS | BRF_PRG }, //	 1
	{ "gg1_3.2m",      0x01000, 0x753ce503, BRF_ESS | BRF_PRG }, //	 2
	{ "gg1_4b.2l",     0x01000, 0x499fcc76, BRF_ESS | BRF_PRG }, //	 3
	
	{ "gg1_5b.3f",     0x01000, 0xbb5caae3, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program Code
	
	{ "gg1_7b.2c",     0x01000, 0xd016686b, BRF_ESS | BRF_PRG }, //  5	Z80 #3 Program Code
	
	{ "gg1_9.4l",      0x01000, 0x58b2f47c, BRF_GRA },	     //  6	Characters
	
	{ "gg1_11.4d",     0x01000, 0xad447c80, BRF_GRA },	     //  7	Sprites
	{ "gg1_10.4f",     0x01000, 0xdd6f1afc, BRF_GRA },	     //  8
	
	{ "prom-5.5n",     0x00020, 0x54603c6b, BRF_GRA },	     //  9	PROMs
	{ "prom-4.2n",     0x00100, 0x59b6edab, BRF_GRA },	     //  10
	{ "prom-3.1c",     0x00100, 0x4a04bb6b, BRF_GRA },	     //  11
	{ "prom-1.1d",     0x00100, 0x7a2815b4, BRF_GRA },	     //  12
	{ "prom-2.5c",     0x00100, 0x77245b66, BRF_GRA },	     //  13
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo GalagaoRomDesc[] = {
	{ "gg1-1.3p",      0x01000, 0xa3a0f743, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "gg1-2.3m",      0x01000, 0x43bb0d5c, BRF_ESS | BRF_PRG }, //	 1
	{ "gg1-3.2m",      0x01000, 0x753ce503, BRF_ESS | BRF_PRG }, //	 2
	{ "gg1-4.2l",      0x01000, 0x83874442, BRF_ESS | BRF_PRG }, //	 3
	
	{ "gg1-5.3f",      0x01000, 0x3102fccd, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program Code
	
	{ "gg1-7.2c",      0x01000, 0x8995088d, BRF_ESS | BRF_PRG }, //  5	Z80 #3 Program Code
	
	{ "gg1-9.4l",      0x01000, 0x58b2f47c, BRF_GRA },	     //  6	Characters
	
	{ "gg1-11.4d",     0x01000, 0xad447c80, BRF_GRA },	     //  7	Sprites
	{ "gg1-10.4f",     0x01000, 0xdd6f1afc, BRF_GRA },	     //  8
	
	{ "prom-5.5n",     0x00020, 0x54603c6b, BRF_GRA },	     //  9	PROMs
	{ "prom-4.2n",     0x00100, 0x59b6edab, BRF_GRA },	     //  10
	{ "prom-3.1c",     0x00100, 0x4a04bb6b, BRF_GRA },	     //  11
	{ "prom-1.1d",     0x00100, 0x7a2815b4, BRF_GRA },	     //  12
	{ "prom-2.5c",     0x00100, 0x77245b66, BRF_GRA },	     //  13
};

STD_ROM_PICK(Galagao)
STD_ROM_FN(Galagao)

static struct BurnRomInfo GalagamwRomDesc[] = {
	{ "3200a.bin",     0x01000, 0x3ef0b053, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "3300b.bin",     0x01000, 0x1b280831, BRF_ESS | BRF_PRG }, //	 1
	{ "3400c.bin",     0x01000, 0x16233d33, BRF_ESS | BRF_PRG }, //	 2
	{ "3500d.bin",     0x01000, 0x0aaf5c23, BRF_ESS | BRF_PRG }, //	 3
	
	{ "3600e.bin",     0x01000, 0xbc556e76, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program Code
	
	{ "3700g.bin",     0x01000, 0xb07f0aa4, BRF_ESS | BRF_PRG }, //  5	Z80 #3 Program Code
	
	{ "2600j.bin",     0x01000, 0x58b2f47c, BRF_GRA },	     //  6	Characters
	
	{ "2800l.bin",     0x01000, 0xad447c80, BRF_GRA },	     //  7	Sprites
	{ "2700k.bin",     0x01000, 0xdd6f1afc, BRF_GRA },	     //  8
	
	{ "prom-5.5n",     0x00020, 0x54603c6b, BRF_GRA },	     //  9	PROMs
	{ "prom-4.2n",     0x00100, 0x59b6edab, BRF_GRA },	     //  10
	{ "prom-3.1c",     0x00100, 0x4a04bb6b, BRF_GRA },	     //  11
	{ "prom-1.1d",     0x00100, 0x7a2815b4, BRF_GRA },	     //  12
	{ "prom-2.5c",     0x00100, 0x77245b66, BRF_GRA },	     //  13
};

STD_ROM_PICK(Galagamw)
STD_ROM_FN(Galagamw)

static struct BurnRomInfo GalagamfRomDesc[] = {
	{ "3200a.bin",     0x01000, 0x3ef0b053, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "3300b.bin",     0x01000, 0x1b280831, BRF_ESS | BRF_PRG }, //	 1
	{ "3400c.bin",     0x01000, 0x16233d33, BRF_ESS | BRF_PRG }, //	 2
	{ "3500d.bin",     0x01000, 0x0aaf5c23, BRF_ESS | BRF_PRG }, //	 3
	
	{ "3600fast.bin",  0x01000, 0x23d586e5, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program Code
	
	{ "3700g.bin",     0x01000, 0xb07f0aa4, BRF_ESS | BRF_PRG }, //  5	Z80 #3 Program Code
	
	{ "2600j.bin",     0x01000, 0x58b2f47c, BRF_GRA },	     //  6	Characters
	
	{ "2800l.bin",     0x01000, 0xad447c80, BRF_GRA },	     //  7	Sprites
	{ "2700k.bin",     0x01000, 0xdd6f1afc, BRF_GRA },	     //  8
	
	{ "prom-5.5n",     0x00020, 0x54603c6b, BRF_GRA },	     //  9	PROMs
	{ "prom-4.2n",     0x00100, 0x59b6edab, BRF_GRA },	     //  10
	{ "prom-3.1c",     0x00100, 0x4a04bb6b, BRF_GRA },	     //  11
	{ "prom-1.1d",     0x00100, 0x7a2815b4, BRF_GRA },	     //  12
	{ "prom-2.5c",     0x00100, 0x77245b66, BRF_GRA },	     //  13
};

STD_ROM_PICK(Galagamf)
STD_ROM_FN(Galagamf)

static struct BurnRomInfo GalagamkRomDesc[] = {
	{ "mk2-1",         0x01000, 0x23cea1e2, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "mk2-2",         0x01000, 0x89695b1a, BRF_ESS | BRF_PRG }, //	 1
	{ "3400c.bin",     0x01000, 0x16233d33, BRF_ESS | BRF_PRG }, //	 2
	{ "mk2-4",         0x01000, 0x24b767f5, BRF_ESS | BRF_PRG }, //	 3
	
	{ "gg1-5.3f",      0x01000, 0x3102fccd, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program Code
	
	{ "gg1-7b.2c",     0x01000, 0xd016686b, BRF_ESS | BRF_PRG }, //  5	Z80 #3 Program Code
	
	{ "gg1-9.4l",      0x01000, 0x58b2f47c, BRF_GRA },	     //  6	Characters
	
	{ "gg1-11.4d",     0x01000, 0xad447c80, BRF_GRA },	     //  7	Sprites
	{ "gg1-10.4f",     0x01000, 0xdd6f1afc, BRF_GRA },	     //  8
	
	{ "prom-5.5n",     0x00020, 0x54603c6b, BRF_GRA },	     //  9	PROMs
	{ "prom-4.2n",     0x00100, 0x59b6edab, BRF_GRA },	     //  10
	{ "prom-3.1c",     0x00100, 0x4a04bb6b, BRF_GRA },	     //  11
	{ "prom-1.1d",     0x00100, 0x7a2815b4, BRF_GRA },	     //  12
	{ "prom-2.5c",     0x00100, 0x77245b66, BRF_GRA },	     //  13
};

STD_ROM_PICK(Galagamk)
STD_ROM_FN(Galagamk)

static struct BurnRomInfo GallagRomDesc[] = {
	{ "gallag.1",      0x01000, 0xa3a0f743, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "gallag.2",      0x01000, 0x5eda60a7, BRF_ESS | BRF_PRG }, //	 1
	{ "gallag.3",      0x01000, 0x753ce503, BRF_ESS | BRF_PRG }, //	 2
	{ "gallag.4",      0x01000, 0x83874442, BRF_ESS | BRF_PRG }, //	 3
	
	{ "gallag.5",      0x01000, 0x3102fccd, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program Code
	
	{ "gallag.7",      0x01000, 0x8995088d, BRF_ESS | BRF_PRG }, //  5	Z80 #3 Program Code
	
	{ "gallag.6",      0x01000, 0x001b70bc, BRF_ESS | BRF_PRG }, //  6	Z80 #4 Program Code
	
	{ "gallag.8",      0x01000, 0x169a98a4, BRF_GRA },	     //  7	Characters
	
	{ "gallag.a",      0x01000, 0xad447c80, BRF_GRA },	     //  8	Sprites
	{ "gallag.9",      0x01000, 0xdd6f1afc, BRF_GRA },	     //  9
	
	{ "prom-5.5n",     0x00020, 0x54603c6b, BRF_GRA },	     //  10	PROMs
	{ "prom-4.2n",     0x00100, 0x59b6edab, BRF_GRA },	     //  11
	{ "prom-3.1c",     0x00100, 0x4a04bb6b, BRF_GRA },	     //  12
	{ "prom-1.1d",     0x00100, 0x7a2815b4, BRF_GRA },	     //  13
	{ "prom-2.5c",     0x00100, 0x77245b66, BRF_GRA },	     //  14
};

STD_ROM_PICK(Gallag)
STD_ROM_FN(Gallag)

static struct BurnSampleInfo GalagaSampleDesc[] = {
#if !defined (ROM_VERIFY)
   { "bang.wav", SAMPLE_NOLOOP },
   { "bang.wav", SAMPLE_NOLOOP },
#endif
   { "", 0 }
};

STD_SAMPLE_PICK(Galaga)
STD_SAMPLE_FN(Galaga)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom1             = Next; Next += 0x04000;
	DrvZ80Rom2             = Next; Next += 0x04000;
	DrvZ80Rom3             = Next; Next += 0x04000;
	DrvPromPalette         = Next; Next += 0x00020;
	DrvPromCharLookup      = Next; Next += 0x00100;
	DrvPromSpriteLookup    = Next; Next += 0x00100;
	NamcoSoundProm         = Next; Next += 0x00100;
	
	RamStart               = Next;

	DrvVideoRam            = Next; Next += 0x00800;
	DrvSharedRam1          = Next; Next += 0x00400;
	DrvSharedRam1          = Next; Next += 0x04000;
	DrvSharedRam2          = Next; Next += 0x00400;
	DrvSharedRam3          = Next; Next += 0x00400;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x100 * 8 * 8;
	DrvSprites             = Next; Next += 0x080 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 576 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	for (INT32 i = 0; i < 3; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}
	
	BurnSampleReset();
	
	DrvCPU1FireIRQ = 0;
	DrvCPU2FireIRQ = 0;
	DrvCPU3FireIRQ = 0;
	DrvCPU2Halt = 0;
	DrvCPU3Halt = 0;
	DrvFlipScreen = 0;
	for (INT32 i = 0; i < 6; i++) {
		DrvStarControl[i] = 0;
	}
	DrvStarScrollX = 0;
	DrvStarScrollY = 0;
	
	IOChipCustomCommand = 0;
	IOChipCPU1FireIRQ = 0;
	IOChipMode = 0;
	IOChipCredits = 0;
	IOChipCoinPerCredit = 0;
	IOChipCreditPerCoin = 0;
	for (INT32 i = 0; i < 16; i++) {
		IOChipCustom[i] = 0;
	}

	return 0;
}

static void Namco54XXWrite(INT32 Data)
{
	static INT32 Fetch;
	static INT32 FetchMode;
	static UINT8 Config1[4], Config2[4], Config3[5];
	
	if (Fetch) {
		switch (FetchMode) {
			default:
			case 1:
				Config1[4 - (Fetch--)] = Data;
				break;

			case 2:
				Config2[4 - (Fetch--)] = Data;
				break;

			case 3:
				Config3[5 - (Fetch--)] = Data;
				break;
		}
	} else {			
		switch (Data & 0xf0) {
			case 0x00:	// nop
				break;

			case 0x10:	// output sound on pins 4-7 only
				if (memcmp(Config1,"\x40\x00\x02\xdf",4) == 0)
					// bosco
					// galaga
					// xevious
					BurnSamplePlay(0);
//				else if (memcmp(Config1,"\x10\x00\x80\xff",4) == 0)
					// xevious
//					sample_start(0, 1, 0);
//				else if (memcmp(Config1,"\x80\x80\x01\xff",4) == 0)
					// xevious
//					sample_start(0, 2, 0);
				break;

			case 0x20:	// output sound on pins 8-11 only
//				if (memcmp(Config2,"\x40\x40\x01\xff",4) == 0)
					// xevious
//					sample_start(1, 3, 0);
//					BurnSamplePlay(1);
				/*else*/ if (memcmp(Config2,"\x30\x30\x03\xdf",4) == 0)
					// bosco
					// galaga
					BurnSamplePlay(1);
//				else if (memcmp(Config2,"\x60\x30\x03\x66",4) == 0)
					// polepos
//					sample_start( 0, 0, 0 );
				break;

			case 0x30:
				Fetch = 4;
				FetchMode = 1;
				break;

			case 0x40:
				Fetch = 4;
				FetchMode = 2;
				break;

			case 0x50:	// output sound on pins 17-20 only
//				if (memcmp(Config3,"\x08\x04\x21\x00\xf1",5) == 0)
					// bosco
//					sample_start(2, 2, 0);
				break;

			case 0x60:
				Fetch = 5;
				FetchMode = 3;
				break;

			case 0x70:
				// polepos
				/* 0x7n = Screech sound. n = pitch (if 0 then no sound) */
				/* followed by 0x60 command? */
				if (( Data & 0x0f ) == 0) {
//					if (sample_playing(1))
//						sample_stop(1);
				} else {
//					INT32 freq = (INT32)( ( 44100.0f / 10.0f ) * (float)(Data & 0x0f) );

//					if (!sample_playing(1))
//						sample_start(1, 1, 1);
//					sample_set_freq(1, freq);
				}
				break;
		}
	}
}

UINT8 __fastcall GalagaZ80ProgRead(UINT16 a)
{
	switch (a) {
		case 0x6800:
		case 0x6801:
		case 0x6802:
		case 0x6803:
		case 0x6804:
		case 0x6805:
		case 0x6806:
		case 0x6807: {
			INT32 Offset = a - 0x6800;
			INT32 Bit0 = (DrvDip[2] >> Offset) & 0x01;
			INT32 Bit1 = (DrvDip[1] >> Offset) & 0x01;

			return Bit0 | (Bit1 << 1);
		}
		
		case 0x7000:
		case 0x7001:
		case 0x7002:
		case 0x7003:
		case 0x7004:
		case 0x7005:
		case 0x7006:
		case 0x7007:
		case 0x7008:
		case 0x7009:
		case 0x700a:
		case 0x700b:
		case 0x700c:
		case 0x700d:
		case 0x700e:
		case 0x700f: {
			INT32 Offset = a - 0x7000;
			
			switch (IOChipCustomCommand) {
				case 0x71:
				case 0xb1: {
					if (Offset == 0) {
						if (IOChipMode) {
							return DrvInput[0] | DrvDip[0];
						} else {
							UINT8 In;
							static UINT8 CoinInserted;
							
							In = DrvInput[0] | DrvDip[0];
							if (In != PrevInValue) {
								if (IOChipCoinPerCredit > 0) {
									if ((In & 0x70) != 0x70 && IOChipCredits < 99) {
										CoinInserted++;
										if (CoinInserted >= IOChipCoinPerCredit) {
											IOChipCredits += IOChipCreditPerCoin;
											CoinInserted = 0;
										}
									}
								} else {
									IOChipCredits = 2;
								}
								
								if ((In & 0x04) == 0) {
									if (IOChipCredits >= 1) IOChipCredits--;
								}
							
								if ((In & 0x08) == 0) {
									if (IOChipCredits >= 2) IOChipCredits -= 2;
								}
							}
							
							PrevInValue = In;
							
							return (IOChipCredits / 10) * 16 + IOChipCredits % 10;
						}
					}
					
					if (Offset == 1) return DrvInput[1];
					if (Offset == 2) return DrvInput[2];
				}
			}
			
			return 0xff;
		}
		
		case 0x7100: {
			return IOChipCustomCommand;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #%i Read %04x\n"), ZetGetActive(), a);
		}
	}
	
	return 0;
}

void __fastcall GalagaZ80ProgWrite(UINT16 a, UINT8 d)
{
	if (a >= 0x6800 && a <= 0x681f) { NamcoSoundWrite(a - 0x6800, d); return; }
	
	switch (a) {
		case 0x6820: {
			DrvCPU1FireIRQ = d & 0x01;
			if (!DrvCPU1FireIRQ) {
			
			}
			return;
		}
		
		case 0x6821: {
			DrvCPU2FireIRQ = d & 0x01;
			if (!DrvCPU2FireIRQ) {
			
			}
			return;
		}
		
		case 0x6822: {
			DrvCPU3FireIRQ = !(d & 0x01);
			return;
		}
		
		case 0x6823: {
			if (!(d & 0x01)) {
				INT32 nActive = ZetGetActive();
				ZetClose();
				ZetOpen(1);
				ZetReset();
				ZetClose();
				ZetOpen(2);
				ZetReset();
				ZetClose();
				ZetOpen(nActive);
				DrvCPU2Halt = 1;
				DrvCPU3Halt = 1;
				return;
			} else {
				DrvCPU2Halt = 0;
				DrvCPU3Halt = 0;
			}
		}
		
		case 0x6830: {
			// watchdog write
			return;
		}
		
		case 0x7000:
		case 0x7001:
		case 0x7002:
		case 0x7003:
		case 0x7004:
		case 0x7005:
		case 0x7006:
		case 0x7007:
		case 0x7008:
		case 0x7009:
		case 0x700a:
		case 0x700b:
		case 0x700c:
		case 0x700d:
		case 0x700e:
		case 0x700f: {
			INT32 Offset = a - 0x7000;
			IOChipCustom[Offset] = d;
			Namco54XXWrite(d);
			
			switch (IOChipCustomCommand) {
				case 0xe1: {
					if (Offset == 7) {
						IOChipCoinPerCredit = IOChipCustom[1];
						IOChipCreditPerCoin = IOChipCustom[2];
					}
					break;
				}
			}
			
			return;
		}
	
		case 0x7100: {
			IOChipCustomCommand = d;
			IOChipCPU1FireIRQ = 1;
			
			switch (IOChipCustomCommand) {
				case 0x10: {
					IOChipCPU1FireIRQ = 0;
					return;
				}
				
				case 0xa1: {
					IOChipMode = 1;
					return;
				}
				
				case 0xe1: {
					IOChipCredits = 0;
					IOChipMode = 0;
					return;
				}
			}
			
			return;
		}
		
		case 0xa000:
		case 0xa001:
		case 0xa002:
		case 0xa003:
		case 0xa004:
		case 0xa005:
		case 0xa006: {
			DrvStarControl[a - 0xa000] = d & 0x01;
			return;
		}
		
		case 0xa007: {
			DrvFlipScreen = d & 0x01;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #%i Write %04x, %02x\n"), ZetGetActive(), a, d);
		}
	}
}

static INT32 CharPlaneOffsets[2]   = { 0, 4 };
static INT32 CharXOffsets[8]       = { 64, 65, 66, 67, 0, 1, 2, 3 };
static INT32 CharYOffsets[8]       = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 SpritePlaneOffsets[2] = { 0, 4 };
static INT32 SpriteXOffsets[16]    = { 0, 1, 2, 3, 64, 65, 66, 67, 128, 129, 130, 131, 192, 193, 194, 195 };
static INT32 SpriteYOffsets[16]    = { 0, 8, 16, 24, 32, 40, 48, 56, 256, 264, 272, 280, 288, 296, 304, 312 };

static void MachineInit()
{
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(GalagaZ80ProgRead);
	ZetSetWriteHandler(GalagaZ80ProgWrite);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80Rom1);
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80Rom1);
	ZetMapArea(0x8000, 0x87ff, 0, DrvVideoRam);
	ZetMapArea(0x8000, 0x87ff, 1, DrvVideoRam);
	ZetMapArea(0x8000, 0x87ff, 2, DrvVideoRam);
	ZetMapArea(0x8800, 0x8bff, 0, DrvSharedRam1);
	ZetMapArea(0x8800, 0x8bff, 1, DrvSharedRam1);
	ZetMapArea(0x8800, 0x8bff, 2, DrvSharedRam1);
	ZetMapArea(0x9000, 0x93ff, 0, DrvSharedRam2);
	ZetMapArea(0x9000, 0x93ff, 1, DrvSharedRam2);
	ZetMapArea(0x9000, 0x93ff, 2, DrvSharedRam2);
	ZetMapArea(0x9800, 0x9bff, 0, DrvSharedRam3);
	ZetMapArea(0x9800, 0x9bff, 1, DrvSharedRam3);
	ZetMapArea(0x9800, 0x9bff, 2, DrvSharedRam3);
	ZetMemEnd();
	ZetClose();
	
	ZetInit(1);
	ZetOpen(1);
	ZetSetReadHandler(GalagaZ80ProgRead);
	ZetSetWriteHandler(GalagaZ80ProgWrite);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80Rom2);
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80Rom2);
	ZetMapArea(0x8000, 0x87ff, 0, DrvVideoRam);
	ZetMapArea(0x8000, 0x87ff, 1, DrvVideoRam);
	ZetMapArea(0x8000, 0x87ff, 2, DrvVideoRam);
	ZetMapArea(0x8800, 0x8bff, 0, DrvSharedRam1);
	ZetMapArea(0x8800, 0x8bff, 1, DrvSharedRam1);
	ZetMapArea(0x8800, 0x8bff, 2, DrvSharedRam1);
	ZetMapArea(0x9000, 0x93ff, 0, DrvSharedRam2);
	ZetMapArea(0x9000, 0x93ff, 1, DrvSharedRam2);
	ZetMapArea(0x9000, 0x93ff, 2, DrvSharedRam2);
	ZetMapArea(0x9800, 0x9bff, 0, DrvSharedRam3);
	ZetMapArea(0x9800, 0x9bff, 1, DrvSharedRam3);
	ZetMapArea(0x9800, 0x9bff, 2, DrvSharedRam3);
	ZetMemEnd();
	ZetClose();
	
	ZetInit(2);
	ZetOpen(2);
	ZetSetReadHandler(GalagaZ80ProgRead);
	ZetSetWriteHandler(GalagaZ80ProgWrite);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80Rom3);
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80Rom3);
	ZetMapArea(0x8000, 0x87ff, 0, DrvVideoRam);
	ZetMapArea(0x8000, 0x87ff, 1, DrvVideoRam);
	ZetMapArea(0x8000, 0x87ff, 2, DrvVideoRam);
	ZetMapArea(0x8800, 0x8bff, 0, DrvSharedRam1);
	ZetMapArea(0x8800, 0x8bff, 1, DrvSharedRam1);
	ZetMapArea(0x8800, 0x8bff, 2, DrvSharedRam1);
	ZetMapArea(0x9000, 0x93ff, 0, DrvSharedRam2);
	ZetMapArea(0x9000, 0x93ff, 1, DrvSharedRam2);
	ZetMapArea(0x9000, 0x93ff, 2, DrvSharedRam2);
	ZetMapArea(0x9800, 0x9bff, 0, DrvSharedRam3);
	ZetMapArea(0x9800, 0x9bff, 1, DrvSharedRam3);
	ZetMapArea(0x9800, 0x9bff, 2, DrvSharedRam3);
	ZetMemEnd();
	ZetClose();
	
	NamcoSoundInit(18432000 / 6 / 32);
	BurnSampleInit(80, 0);

	GenericTilesInit();

	// Reset the driver
	DrvDoReset();
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

	DrvTempRom = (UINT8 *)BurnMalloc(0x02000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x01000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x02000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x03000,  3, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000,  4, 1); if (nRet != 0) return 1;
	
	// Load Z80 #3 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom3 + 0x00000,  5, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom,            6, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x02000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x01000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x80, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);

	// Load the PROMs
	nRet = BurnLoadRom(DrvPromPalette,        9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,    10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup,  11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(NamcoSoundProm,       12, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	MachineInit();

	return 0;
}

static INT32 GallagInit()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x02000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x01000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x02000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x03000,  3, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000,  4, 1); if (nRet != 0) return 1;
	
	// Load Z80 #3 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom3 + 0x00000,  5, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom,            7, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x02000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x01000,  9, 1); if (nRet != 0) return 1;
	GfxDecode(0x80, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);

	// Load the PROMs
	nRet = BurnLoadRom(DrvPromPalette,       10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,    11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup,  12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(NamcoSoundProm,       13, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	MachineInit();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();
	NamcoSoundExit();
	BurnSampleExit();
	ZetExit();
	
	BurnFree(Mem);
	
	DrvCPU1FireIRQ = 0;
	DrvCPU2FireIRQ = 0;
	DrvCPU3FireIRQ = 0;
	DrvCPU2Halt = 0;
	DrvCPU3Halt = 0;
	DrvFlipScreen = 0;
	for (INT32 i = 0; i < 6; i++) {
		DrvStarControl[i] = 0;
	}
	DrvStarScrollX = 0;
	DrvStarScrollY = 0;
	
	IOChipCustomCommand = 0;
	IOChipCPU1FireIRQ = 0;
	IOChipMode = 0;
	IOChipCredits = 0;
	IOChipCoinPerCredit = 0;
	IOChipCreditPerCoin = 0;
	for (INT32 i = 0; i < 16; i++) {
		IOChipCustom[i] = 0;
	}

	return 0;
}

static void DrvCalcPalette()
{
	INT32 i;
	UINT32 Palette[96];
	
	for (i = 0; i < 32; i++) {
		INT32 bit0, bit1, bit2, r, g, b;
		
		bit0 = (DrvPromPalette[i] >> 0) & 0x01;
		bit1 = (DrvPromPalette[i] >> 1) & 0x01;
		bit2 = (DrvPromPalette[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (DrvPromPalette[i] >> 3) & 0x01;
		bit1 = (DrvPromPalette[i] >> 4) & 0x01;
		bit2 = (DrvPromPalette[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = (DrvPromPalette[i] >> 6) & 0x01;
		bit2 = (DrvPromPalette[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		
		Palette[i] = BurnHighCol(r, g, b, 0);
	}
	
	for (i = 0; i < 64; i++) {
		INT32 bits, r, g, b;
		static const INT32 map[4] = { 0x00, 0x47, 0x97, 0xde };
		
		bits = (i >> 0) & 0x03;
		r = map[bits];
		bits = (i >> 2) & 0x03;
		g = map[bits];
		bits = (i >> 4) & 0x03;
		b = map[bits];
		
		Palette[32 + i] = BurnHighCol(r, g, b, 0);
	}
	
	for (i = 0; i < 256; i++) {
		DrvPalette[i] = Palette[((DrvPromCharLookup[i]) & 0x0f) + 0x10];
	}
	
	for (i = 0; i < 256; i++) {
		DrvPalette[256 + i] = Palette[DrvPromSpriteLookup[i] & 0x0f];
	}
	
	for (i = 0; i < 64; i++) {
		DrvPalette[512 + i] = Palette[32 + i];
	}
}

struct Star {
	UINT16 x, y;
	UINT8 Colour, Set;
};

const struct Star StarSeedTab[252] = {
	// set 0
	{0x0085, 0x0006, 0x35, 0x00},
	{0x008F, 0x0008, 0x30, 0x00},
	{0x00E5, 0x001B, 0x07, 0x00},
	{0x0022, 0x001C, 0x31, 0x00},
	{0x00E5, 0x0025, 0x1D, 0x00},
	{0x0015, 0x0026, 0x29, 0x00},
	{0x0080, 0x002D, 0x3B, 0x00},
	{0x0097, 0x002E, 0x1C, 0x00},
	{0x00BA, 0x003B, 0x05, 0x00},
	{0x0036, 0x003D, 0x36, 0x00},
	{0x0057, 0x0044, 0x09, 0x00},
	{0x00CF, 0x0044, 0x3D, 0x00},
	{0x0061, 0x004E, 0x27, 0x00},
	{0x0087, 0x0064, 0x1A, 0x00},
	{0x00D6, 0x0064, 0x17, 0x00},
	{0x000B, 0x006C, 0x3C, 0x00},
	{0x0006, 0x006D, 0x24, 0x00},
	{0x0018, 0x006E, 0x3A, 0x00},
	{0x00A9, 0x0079, 0x23, 0x00},
	{0x008A, 0x007B, 0x11, 0x00},
	{0x00D6, 0x0080, 0x0C, 0x00},
	{0x0067, 0x0082, 0x3F, 0x00},
	{0x0039, 0x0083, 0x38, 0x00},
	{0x0072, 0x0083, 0x14, 0x00},
	{0x00EC, 0x0084, 0x16, 0x00},
	{0x008E, 0x0085, 0x10, 0x00},
	{0x0020, 0x0088, 0x25, 0x00},
	{0x0095, 0x008A, 0x0F, 0x00},
	{0x000E, 0x008D, 0x00, 0x00},
	{0x0006, 0x0091, 0x2E, 0x00},
	{0x0007, 0x0094, 0x0D, 0x00},
	{0x00AE, 0x0097, 0x0B, 0x00},
	{0x0000, 0x0098, 0x2D, 0x00},
	{0x0086, 0x009B, 0x01, 0x00},
	{0x0058, 0x00A1, 0x34, 0x00},
	{0x00FE, 0x00A1, 0x3E, 0x00},
	{0x00A2, 0x00A8, 0x1F, 0x00},
	{0x0041, 0x00AA, 0x0A, 0x00},
	{0x003F, 0x00AC, 0x32, 0x00},
	{0x00DE, 0x00AC, 0x03, 0x00},
	{0x00D4, 0x00B9, 0x26, 0x00},
	{0x006D, 0x00BB, 0x1B, 0x00},
	{0x0062, 0x00BD, 0x39, 0x00},
	{0x00C9, 0x00BE, 0x18, 0x00},
	{0x006C, 0x00C1, 0x04, 0x00},
	{0x0059, 0x00C3, 0x21, 0x00},
	{0x0060, 0x00CC, 0x0E, 0x00},
	{0x0091, 0x00CC, 0x12, 0x00},
	{0x003F, 0x00CF, 0x06, 0x00},
	{0x00F7, 0x00CF, 0x22, 0x00},
	{0x0044, 0x00D0, 0x33, 0x00},
	{0x0034, 0x00D2, 0x08, 0x00},
	{0x00D3, 0x00D9, 0x20, 0x00},
	{0x0071, 0x00DD, 0x37, 0x00},
	{0x0073, 0x00E1, 0x2C, 0x00},
	{0x00B9, 0x00E3, 0x2F, 0x00},
	{0x00A9, 0x00E4, 0x13, 0x00},
	{0x00D3, 0x00E7, 0x19, 0x00},
	{0x0037, 0x00ED, 0x02, 0x00},
	{0x00BD, 0x00F4, 0x15, 0x00},
	{0x000F, 0x00F6, 0x28, 0x00},
	{0x004F, 0x00F7, 0x2B, 0x00},
	{0x00FB, 0x00FF, 0x2A, 0x00},

	// set 1
	{0x00FE, 0x0004, 0x3D, 0x01},
	{0x00C4, 0x0006, 0x10, 0x01},
	{0x001E, 0x0007, 0x2D, 0x01},
	{0x0083, 0x000B, 0x1F, 0x01},
	{0x002E, 0x000D, 0x3C, 0x01},
	{0x001F, 0x000E, 0x00, 0x01},
	{0x00D8, 0x000E, 0x2C, 0x01},
	{0x0003, 0x000F, 0x17, 0x01},
	{0x0095, 0x0011, 0x3F, 0x01},
	{0x006A, 0x0017, 0x35, 0x01},
	{0x00CC, 0x0017, 0x02, 0x01},
	{0x0000, 0x0018, 0x32, 0x01},
	{0x0092, 0x001D, 0x36, 0x01},
	{0x00E3, 0x0021, 0x04, 0x01},
	{0x002F, 0x002D, 0x37, 0x01},
	{0x00F0, 0x002F, 0x0C, 0x01},
	{0x009B, 0x003E, 0x06, 0x01},
	{0x00A4, 0x004C, 0x07, 0x01},
	{0x00EA, 0x004D, 0x13, 0x01},
	{0x0084, 0x004E, 0x21, 0x01},
	{0x0033, 0x0052, 0x0F, 0x01},
	{0x0070, 0x0053, 0x0E, 0x01},
	{0x0006, 0x0059, 0x08, 0x01},
	{0x0081, 0x0060, 0x28, 0x01},
	{0x0037, 0x0061, 0x29, 0x01},
	{0x008F, 0x0067, 0x2F, 0x01},
	{0x001B, 0x006A, 0x1D, 0x01},
	{0x00BF, 0x007C, 0x12, 0x01},
	{0x0051, 0x007F, 0x31, 0x01},
	{0x0061, 0x0086, 0x25, 0x01},
	{0x006A, 0x008F, 0x0D, 0x01},
	{0x006A, 0x0091, 0x19, 0x01},
	{0x0090, 0x0092, 0x05, 0x01},
	{0x003B, 0x0096, 0x24, 0x01},
	{0x008C, 0x0097, 0x0A, 0x01},
	{0x0006, 0x0099, 0x03, 0x01},
	{0x0038, 0x0099, 0x38, 0x01},
	{0x00A8, 0x0099, 0x18, 0x01},
	{0x0076, 0x00A6, 0x20, 0x01},
	{0x00AD, 0x00A6, 0x1C, 0x01},
	{0x00EC, 0x00A6, 0x1E, 0x01},
	{0x0086, 0x00AC, 0x15, 0x01},
	{0x0078, 0x00AF, 0x3E, 0x01},
	{0x007B, 0x00B3, 0x09, 0x01},
	{0x0027, 0x00B8, 0x39, 0x01},
	{0x0088, 0x00C2, 0x23, 0x01},
	{0x0044, 0x00C3, 0x3A, 0x01},
	{0x00CF, 0x00C5, 0x34, 0x01},
	{0x0035, 0x00C9, 0x30, 0x01},
	{0x006E, 0x00D1, 0x3B, 0x01},
	{0x00D6, 0x00D7, 0x16, 0x01},
	{0x003A, 0x00D9, 0x2B, 0x01},
	{0x00AB, 0x00E0, 0x11, 0x01},
	{0x00E0, 0x00E2, 0x1B, 0x01},
	{0x006F, 0x00E6, 0x0B, 0x01},
	{0x00B8, 0x00E8, 0x14, 0x01},
	{0x00D9, 0x00E8, 0x1A, 0x01},
	{0x00F9, 0x00E8, 0x22, 0x01},
	{0x0004, 0x00F1, 0x2E, 0x01},
	{0x0049, 0x00F8, 0x26, 0x01},
	{0x0010, 0x00F9, 0x01, 0x01},
	{0x0039, 0x00FB, 0x33, 0x01},
	{0x0028, 0x00FC, 0x27, 0x01},

	// set 2
	{0x00FA, 0x0006, 0x19, 0x02},
	{0x00E4, 0x0007, 0x2D, 0x02},
	{0x0072, 0x000A, 0x03, 0x02},
	{0x0084, 0x001B, 0x00, 0x02},
	{0x00BA, 0x001D, 0x29, 0x02},
	{0x00E3, 0x0022, 0x04, 0x02},
	{0x00D1, 0x0026, 0x2A, 0x02},
	{0x0089, 0x0032, 0x30, 0x02},
	{0x005B, 0x0036, 0x27, 0x02},
	{0x0084, 0x003A, 0x36, 0x02},
	{0x0053, 0x003F, 0x0D, 0x02},
	{0x0008, 0x0040, 0x1D, 0x02},
	{0x0055, 0x0040, 0x1A, 0x02},
	{0x00AA, 0x0041, 0x31, 0x02},
	{0x00FB, 0x0041, 0x2B, 0x02},
	{0x00BC, 0x0046, 0x16, 0x02},
	{0x0093, 0x0052, 0x39, 0x02},
	{0x00B9, 0x0057, 0x10, 0x02},
	{0x0054, 0x0059, 0x28, 0x02},
	{0x00E6, 0x005A, 0x01, 0x02},
	{0x00A7, 0x005D, 0x1B, 0x02},
	{0x002D, 0x005E, 0x35, 0x02},
	{0x0014, 0x0062, 0x21, 0x02},
	{0x0069, 0x006D, 0x1F, 0x02},
	{0x00CE, 0x006F, 0x0B, 0x02},
	{0x00DF, 0x0075, 0x2F, 0x02},
	{0x00CB, 0x0077, 0x12, 0x02},
	{0x004E, 0x007C, 0x23, 0x02},
	{0x004A, 0x0084, 0x0F, 0x02},
	{0x0012, 0x0086, 0x25, 0x02},
	{0x0068, 0x008C, 0x32, 0x02},
	{0x0003, 0x0095, 0x20, 0x02},
	{0x000A, 0x009C, 0x17, 0x02},
	{0x005B, 0x00A3, 0x08, 0x02},
	{0x005F, 0x00A4, 0x3E, 0x02},
	{0x0072, 0x00A4, 0x2E, 0x02},
	{0x00CC, 0x00A6, 0x06, 0x02},
	{0x008A, 0x00AB, 0x0C, 0x02},
	{0x00E0, 0x00AD, 0x26, 0x02},
	{0x00F3, 0x00AF, 0x0A, 0x02},
	{0x0075, 0x00B4, 0x13, 0x02},
	{0x0068, 0x00B7, 0x11, 0x02},
	{0x006D, 0x00C2, 0x2C, 0x02},
	{0x0076, 0x00C3, 0x14, 0x02},
	{0x00CF, 0x00C4, 0x1E, 0x02},
	{0x0004, 0x00C5, 0x1C, 0x02},
	{0x0013, 0x00C6, 0x3F, 0x02},
	{0x00B9, 0x00C7, 0x3C, 0x02},
	{0x0005, 0x00D7, 0x34, 0x02},
	{0x0095, 0x00D7, 0x3A, 0x02},
	{0x00FC, 0x00D8, 0x02, 0x02},
	{0x00E7, 0x00DC, 0x09, 0x02},
	{0x001D, 0x00E1, 0x05, 0x02},
	{0x0005, 0x00E6, 0x33, 0x02},
	{0x001C, 0x00E9, 0x3B, 0x02},
	{0x00A2, 0x00ED, 0x37, 0x02},
	{0x0028, 0x00EE, 0x07, 0x02},
	{0x00DD, 0x00EF, 0x18, 0x02},
	{0x006D, 0x00F0, 0x38, 0x02},
	{0x00A1, 0x00F2, 0x0E, 0x02},
	{0x0074, 0x00F7, 0x3D, 0x02},
	{0x0069, 0x00F9, 0x22, 0x02},
	{0x003F, 0x00FF, 0x24, 0x02},

	// set 3
	{0x0071, 0x0010, 0x34, 0x03},
	{0x00AF, 0x0011, 0x23, 0x03},
	{0x00A0, 0x0014, 0x26, 0x03},
	{0x0002, 0x0017, 0x02, 0x03},
	{0x004B, 0x0019, 0x31, 0x03},
	{0x0093, 0x001C, 0x0E, 0x03},
	{0x001B, 0x001E, 0x25, 0x03},
	{0x0032, 0x0020, 0x2E, 0x03},
	{0x00EE, 0x0020, 0x3A, 0x03},
	{0x0079, 0x0022, 0x2F, 0x03},
	{0x006C, 0x0023, 0x17, 0x03},
	{0x00BC, 0x0025, 0x11, 0x03},
	{0x0041, 0x0029, 0x30, 0x03},
	{0x001C, 0x002E, 0x32, 0x03},
	{0x00B9, 0x0031, 0x01, 0x03},
	{0x0083, 0x0032, 0x05, 0x03},
	{0x0095, 0x003A, 0x12, 0x03},
	{0x000D, 0x003F, 0x07, 0x03},
	{0x0020, 0x0041, 0x33, 0x03},
	{0x0092, 0x0045, 0x2C, 0x03},
	{0x00D4, 0x0047, 0x08, 0x03},
	{0x00A1, 0x004B, 0x2D, 0x03},
	{0x00D2, 0x004B, 0x3B, 0x03},
	{0x00D6, 0x0052, 0x24, 0x03},
	{0x009A, 0x005F, 0x1C, 0x03},
	{0x0016, 0x0060, 0x3D, 0x03},
	{0x001A, 0x0063, 0x1F, 0x03},
	{0x00CD, 0x0066, 0x28, 0x03},
	{0x00FF, 0x0067, 0x10, 0x03},
	{0x0035, 0x0069, 0x20, 0x03},
	{0x008F, 0x006C, 0x04, 0x03},
	{0x00CA, 0x006C, 0x2A, 0x03},
	{0x005A, 0x0074, 0x09, 0x03},
	{0x0060, 0x0078, 0x38, 0x03},
	{0x0072, 0x0079, 0x1E, 0x03},
	{0x0037, 0x007F, 0x29, 0x03},
	{0x0012, 0x0080, 0x14, 0x03},
	{0x0029, 0x0082, 0x2B, 0x03},
	{0x0084, 0x0098, 0x36, 0x03},
	{0x0032, 0x0099, 0x37, 0x03},
	{0x00BB, 0x00A0, 0x19, 0x03},
	{0x003E, 0x00A3, 0x3E, 0x03},
	{0x004A, 0x00A6, 0x1A, 0x03},
	{0x0029, 0x00A7, 0x21, 0x03},
	{0x009D, 0x00B7, 0x22, 0x03},
	{0x006C, 0x00B9, 0x15, 0x03},
	{0x000C, 0x00C0, 0x0A, 0x03},
	{0x00C2, 0x00C3, 0x0F, 0x03},
	{0x002F, 0x00C9, 0x0D, 0x03},
	{0x00D2, 0x00CE, 0x16, 0x03},
	{0x00F3, 0x00CE, 0x0B, 0x03},
	{0x0075, 0x00CF, 0x27, 0x03},
	{0x001A, 0x00D5, 0x35, 0x03},
	{0x0026, 0x00D6, 0x39, 0x03},
	{0x0080, 0x00DA, 0x3C, 0x03},
	{0x00A9, 0x00DD, 0x00, 0x03},
	{0x00BC, 0x00EB, 0x03, 0x03},
	{0x0032, 0x00EF, 0x1B, 0x03},
	{0x0067, 0x00F0, 0x3F, 0x03},
	{0x00EF, 0x00F1, 0x18, 0x03},
	{0x00A8, 0x00F3, 0x0C, 0x03},
	{0x00DE, 0x00F9, 0x1D, 0x03},
	{0x002C, 0x00FA, 0x13, 0x03}
};

static void DrvRenderStars()
{
	if (DrvStarControl[5] == 1) {
  		INT32 StarCounter;
		INT32 SetA, SetB;

		SetA = DrvStarControl[3];
		SetB = DrvStarControl[4] | 0x02;

		for (StarCounter = 0; StarCounter < 252; StarCounter++) {
			INT32 x, y;

			if ((SetA == StarSeedTab[StarCounter].Set) || (SetB == StarSeedTab[StarCounter].Set)) {
				x = (StarSeedTab[StarCounter].x + DrvStarScrollX) % 256 + 16;
				y = (112 + StarSeedTab[StarCounter].y + DrvStarScrollY) % 256;

				if (x >= 0 && x < 288 && y >= 0 && y < 224) {
					pTransDraw[(y * nScreenWidth) + x] = StarSeedTab[StarCounter].Colour + 512;
				}
			}

		}
	}
}

static void DrvRenderTilemap()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex, Row, Col;

	for (mx = 0; mx < 28; mx++) {
		for (my = 0; my < 36; my++) {
			Row = mx + 2;
			Col = my - 2;
			if (Col & 0x20) {
				TileIndex = Row + ((Col & 0x1f) << 5);
			} else {
				TileIndex = Col + (Row << 5);
			}
			
			Code = DrvVideoRam[TileIndex + 0x000] & 0x7f;
			Colour = DrvVideoRam[TileIndex + 0x400] & 0x3f;

			y = 8 * mx;
			x = 8 * my;
			
			if (DrvFlipScreen) {
				x = 280 - x;
				y = 216 - y;
			}
			
			if (x > 8 && x < 280 && y > 8 && y < 216) {
				if (DrvFlipScreen) {
					Render8x8Tile_FlipXY(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
				} else {
					Render8x8Tile(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
				}
			} else {
				if (DrvFlipScreen) {
					Render8x8Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
				} else {
					Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
				}
			}
		}
	}
}

static void DrvRenderSprites()
{
	UINT8 *SpriteRam1 = DrvSharedRam1 + 0x380;
	UINT8 *SpriteRam2 = DrvSharedRam2 + 0x380;
	UINT8 *SpriteRam3 = DrvSharedRam3 + 0x380;
	
	for (INT32 Offset = 0; Offset < 0x80; Offset += 2) {
		static const INT32 GfxOffset[2][2] = {
			{ 0, 1 },
			{ 2, 3 }
		};
		INT32 Sprite = SpriteRam1[Offset + 0] & 0x7f;
		INT32 Colour = SpriteRam1[Offset + 1] & 0x3f;
		INT32 sx = SpriteRam2[Offset + 1] - 40 + (0x100 * (SpriteRam3[Offset + 1] & 0x03));
		INT32 sy = 256 - SpriteRam2[Offset + 0] + 1;
		INT32 xFlip = (SpriteRam3[Offset + 0] & 0x01);
		INT32 yFlip = (SpriteRam3[Offset + 0] & 0x02) >> 1;
		INT32 xSize = (SpriteRam3[Offset + 0] & 0x04) >> 2;
		INT32 ySize = (SpriteRam3[Offset + 0] & 0x08) >> 3;

		sy -= 16 * ySize;
		sy = (sy & 0xff) - 32;

		if (DrvFlipScreen) {
			xFlip = !xFlip;
			yFlip = !yFlip;
		}

		for (INT32 y = 0; y <= ySize; y++) {
			for (INT32 x = 0; x <= xSize; x++) {
				INT32 Code = Sprite + GfxOffset[y ^ (ySize * yFlip)][x ^ (xSize * xFlip)];
				INT32 xPos = sx + 16 * x;
				INT32 yPos = sy + 16 * y;
				
				if (xPos > 16 && xPos < 272 && yPos > 16 && yPos < 208) {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY(pTransDraw, Code, xPos, yPos, Colour, 2, 0, 256, DrvSprites);
						} else {
							Render16x16Tile_Mask_FlipX(pTransDraw, Code, xPos, yPos, Colour, 2, 0, 256, DrvSprites);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY(pTransDraw, Code, xPos, yPos, Colour, 2, 0, 256, DrvSprites);
						} else {
							Render16x16Tile_Mask(pTransDraw, Code, xPos, yPos, Colour, 2, 0, 256, DrvSprites);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, xPos, yPos, Colour, 2, 0, 256, DrvSprites);
						} else {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, xPos, yPos, Colour, 2, 0, 256, DrvSprites);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, xPos, yPos, Colour, 2, 0, 256, DrvSprites);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, Code, xPos, yPos, Colour, 2, 0, 256, DrvSprites);
						}
					}
				}
			}
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	DrvRenderTilemap();
	DrvRenderStars();
	DrvRenderSprites();	
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nInterleave = nBurnSoundLen;
	
	if (DrvReset) DrvDoReset();

	DrvMakeInputs();
	
	INT32 nSoundBufferPos = 0;

	nCyclesTotal[0] = (18432000 / 6) / 60;
	nCyclesTotal[1] = (18432000 / 6) / 60;
	nCyclesTotal[2] = (18432000 / 6) / 60;
	nCyclesDone[0] = nCyclesDone[1] = nCyclesDone[2] = 0;
	
	ZetNewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;
		
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		if (i == (nInterleave - 1) && DrvCPU1FireIRQ) {
			ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		}
		if ((i == 0 || i == (nInterleave / 3) || i == (nInterleave / 3 * 2)) && IOChipCPU1FireIRQ) {
			ZetNmi();
		}
		ZetClose();
		
		if (!DrvCPU2Halt) {
			nCurrentCPU = 1;
			ZetOpen(nCurrentCPU);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
			if (i == (nInterleave - 1) && DrvCPU2FireIRQ) {
				ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
			}
			ZetClose();
		}
		
		if (!DrvCPU3Halt) {
			nCurrentCPU = 2;
			ZetOpen(nCurrentCPU);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
			if ((i == (nInterleave / 2) || i == (nInterleave - 1)) && DrvCPU3FireIRQ) {
				ZetNmi();
			}
			ZetClose();
		}
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			
			if (nSegmentLength) {
				NamcoSoundUpdate(pSoundBuf, nSegmentLength);
				BurnSampleRender(pSoundBuf, nSegmentLength);
			}
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			NamcoSoundUpdate(pSoundBuf, nSegmentLength);
			BurnSampleRender(pSoundBuf, nSegmentLength);
		}
	}

	if (pBurnDraw) DrvDraw();
	
	static const INT32 Speeds[8] = { -1, -2, -3, 0, 3, 2, 1, 0 };

	DrvStarScrollX += Speeds[DrvStarControl[0] + (DrvStarControl[1] * 2) + (DrvStarControl[2] * 4)];

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029698;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);			// Scan Z80
		NamcoSoundScan(nAction, pnMin);

		// Scan critical driver variables
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
		SCAN_VAR(DrvCPU1FireIRQ);
		SCAN_VAR(DrvCPU2FireIRQ);
		SCAN_VAR(DrvCPU3FireIRQ);
		SCAN_VAR(DrvCPU2Halt);
		SCAN_VAR(DrvCPU3Halt);
		SCAN_VAR(DrvFlipScreen);
		SCAN_VAR(DrvStarScrollX);
		SCAN_VAR(DrvStarScrollY);
		SCAN_VAR(IOChipCustomCommand);
		SCAN_VAR(IOChipCPU1FireIRQ);
		SCAN_VAR(IOChipMode);
		SCAN_VAR(IOChipCredits);
		SCAN_VAR(IOChipCoinPerCredit);
		SCAN_VAR(IOChipCreditPerCoin);
		SCAN_VAR(PrevInValue);
		for (INT32 i = 0; i < 6; i++) {
			SCAN_VAR(DrvStarControl[i]);
		}
		for (INT32 i = 0; i < 16; i++) {
			SCAN_VAR(IOChipCustom[i]);
		}		
		SCAN_VAR(DrvDip);
		SCAN_VAR(DrvInput);
	}
	
	return 0;
}

struct BurnDriver BurnDrvGalaga = {
	"galaga", NULL, NULL, "galaga", "1981",
	"Galaga (Namco rev. B)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, DrvRomInfo, DrvRomName, GalagaSampleInfo, GalagaSampleName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 576, 224, 288, 3, 4
};

struct BurnDriver BurnDrvGalagao = {
	"galagao", "galaga", NULL, "galaga", "1981",
	"Galaga (Namco)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, GalagaoRomInfo, GalagaoRomName, GalagaSampleInfo, GalagaSampleName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 576, 224, 288, 3, 4
};

struct BurnDriver BurnDrvGalagamw = {
	"galagamw", "galaga", NULL, "galaga", "1981",
	"Galaga (Midway set 1)\0", NULL, "Namco (Midway License)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, GalagamwRomInfo, GalagamwRomName, GalagaSampleInfo, GalagaSampleName, DrvInputInfo, GalagamwDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 576, 224, 288, 3, 4
};

struct BurnDriver BurnDrvGalagamk = {
	"galagamk", "galaga", NULL, "galaga", "1981",
	"Galaga (Midway set 2)\0", NULL, "Namco (Midway License)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, GalagamkRomInfo, GalagamkRomName, GalagaSampleInfo, GalagaSampleName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 576, 224, 288, 3, 4
};

struct BurnDriver BurnDrvGalagamf = {
	"galagamf", "galaga", NULL, "galaga", "1981",
	"Galaga (Midway set 1 with fast shoot hack)\0", NULL, "Namco (Midway License)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, GalagamfRomInfo, GalagamfRomName, GalagaSampleInfo, GalagaSampleName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 576, 224, 288, 3, 4
};

struct BurnDriver BurnDrvGallag = {
	"gallag", "galaga", NULL, "galaga", "1981",
	"Gallag\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, GallagRomInfo, GallagRomName, GalagaSampleInfo, GalagaSampleName, DrvInputInfo, DrvDIPInfo,
	GallagInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 576, 224, 288, 3, 4
};
