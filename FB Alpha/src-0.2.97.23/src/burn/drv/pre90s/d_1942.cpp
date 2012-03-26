#include "tiles_generic.h"
#include "zet.h"

#include "driver.h"
extern "C" {
 #include "ay8910.h"
}

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
static UINT8 *DrvZ80Rom1          = NULL;
static UINT8 *DrvZ80Rom2          = NULL;
static UINT8 *DrvZ80Ram1          = NULL;
static UINT8 *DrvZ80Ram2          = NULL;
static UINT8 *DrvFgVideoRam       = NULL;
static UINT8 *DrvBgVideoRam       = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvPromRed          = NULL;
static UINT8 *DrvPromGreen        = NULL;
static UINT8 *DrvPromBlue         = NULL;
static UINT8 *DrvPromCharLookup   = NULL;
static UINT8 *DrvPromTileLookup   = NULL;
static UINT8 *DrvPromSpriteLookup = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvTiles            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette          = NULL;
static INT16* pFMBuffer;
static INT16* pAY8910Buffer[6];

static UINT8 DrvRomBank;
static UINT8 DrvPaletteBank;
static UINT8 DrvBgScroll[2];
static UINT8 DrvFlipScreen;
static UINT8 DrvSoundLatch;

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 6, "p2 coin"   },
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
	{"Service"           , BIT_DIGITAL  , DrvInputPort0 + 4, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
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
	{0x12, 0xff, 0xff, 0xf7, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x12, 0x01, 0x07, 0x01, "4 Coins 1 Play"         },
	{0x12, 0x01, 0x07, 0x02, "3 Coins 1 Play"         },
	{0x12, 0x01, 0x07, 0x04, "2 Coins 1 Play"         },
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x12, 0x01, 0x07, 0x03, "2 Coins 3 Plays"        },
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x12, 0x01, 0x07, 0x05, "1 Coin  4 Plays"        },
	{0x12, 0x01, 0x07, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x08, 0x00, "Upright"                },
	{0x12, 0x01, 0x08, 0x08, "Cocktail"               },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x12, 0x01, 0x30, 0x30, "20k  80k  80k+"         },
	{0x12, 0x01, 0x30, 0x20, "20k 100k 100k+"         },
	{0x12, 0x01, 0x30, 0x10, "30k  80k  80k+"         },
	{0x12, 0x01, 0x30, 0x00, "30k 100k 100k+"         },	
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0xc0, 0x80, "1"                      },
	{0x12, 0x01, 0xc0, 0x40, "2"                      },
	{0x12, 0x01, 0xc0, 0xc0, "3"                      },
	{0x12, 0x01, 0xc0, 0x00, "5"                      },
	
	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x13, 0x01, 0x07, 0x01, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x02, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x04, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x07, 0x03, "2 Coins 3 Plays"        },
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x07, 0x05, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x07, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x08, 0x08, "Off"                    },
	{0x13, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x13, 0x01, 0x10, 0x10, "Off"                    },
	{0x13, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0x60, 0x40, "Easy"                   },
	{0x13, 0x01, 0x60, 0x60, "Normal"                 },
	{0x13, 0x01, 0x60, 0x20, "Difficult"              },
	{0x13, 0x01, 0x60, 0x00, "Very Difficult"         },
	
	{0   , 0xfe, 0   , 2   , "Screen Stop"            },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Drv)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "srb-03.m3",     0x04000, 0xd9dafcc3, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "srb-04.m4",     0x04000, 0xda0cf924, BRF_ESS | BRF_PRG }, //	 1
	{ "srb-05.m5",     0x04000, 0xd102911c, BRF_ESS | BRF_PRG }, //	 2
	{ "srb-06.m6",     0x02000, 0x466f8248, BRF_ESS | BRF_PRG }, //	 3
	{ "srb-07.m7",     0x04000, 0x0d31038c, BRF_ESS | BRF_PRG }, //	 4
	
	{ "sr-01.c11",     0x04000, 0xbd87f06b, BRF_ESS | BRF_PRG }, //  5	Z80 #2 Program 
	
	{ "sr-02.f2",      0x02000, 0x6ebca191, BRF_GRA },	     //  6	Characters
	
	{ "sr-08.a1",      0x02000, 0x3884d9eb, BRF_GRA },	     //  7	Tiles
	{ "sr-09.a2",      0x02000, 0x999cf6e0, BRF_GRA },	     //  8
	{ "sr-10.a3",      0x02000, 0x8edb273a, BRF_GRA },	     //  9
	{ "sr-11.a4",      0x02000, 0x3a2726c3, BRF_GRA },	     //  10
	{ "sr-12.a5",      0x02000, 0x1bd3d8bb, BRF_GRA },	     //  11
	{ "sr-13.a6",      0x02000, 0x658f02c4, BRF_GRA },	     //  12
	
	{ "sr-14.l1",      0x04000, 0x2528bec6, BRF_GRA },	     //  13	Sprites
	{ "sr-15.l2",      0x04000, 0xf89287aa, BRF_GRA },	     //  14
	{ "sr-16.n1",      0x04000, 0x024418f8, BRF_GRA },	     //  15
	{ "sr-17.n2",      0x04000, 0xe2c7e489, BRF_GRA },	     //  16
	
	{ "sb-5.e8",       0x00100, 0x93ab8153, BRF_GRA },	     //  17	PROMs
	{ "sb-6.e9",       0x00100, 0x8ab44f7d, BRF_GRA },	     //  18
	{ "sb-7.e10",      0x00100, 0xf4ade9a4, BRF_GRA },	     //  19
	{ "sb-0.f1",       0x00100, 0x6047d91b, BRF_GRA },	     //  20
	{ "sb-4.d6",       0x00100, 0x4858968d, BRF_GRA },	     //  21
	{ "sb-8.k3",       0x00100, 0xf6fad943, BRF_GRA },	     //  22
	{ "sb-2.d1",       0x00100, 0x8bb8b3df, BRF_GRA },	     //  23
	{ "sb-3.d2",       0x00100, 0x3b0c99af, BRF_GRA },	     //  24
	{ "sb-1.k6",       0x00100, 0x712ac508, BRF_GRA },	     //  25
	{ "sb-9.m11",      0x00100, 0x4921635c, BRF_GRA },	     //  26
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo DrvaRomDesc[] = {
	{ "sra-03.m3",     0x04000, 0x40201bab, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "sr-04.m4",      0x04000, 0xa60ac644, BRF_ESS | BRF_PRG }, //	 1
	{ "sr-05.m5",      0x04000, 0x835f7b24, BRF_ESS | BRF_PRG }, //	 2
	{ "sr-06.m6",      0x02000, 0x821c6481, BRF_ESS | BRF_PRG }, //	 3
	{ "sr-07.m7",      0x04000, 0x5df525e1, BRF_ESS | BRF_PRG }, //	 4
	
	{ "sr-01.c11",     0x04000, 0xbd87f06b, BRF_ESS | BRF_PRG }, //  5	Z80 #2 Program 
	
	{ "sr-02.f2",      0x02000, 0x6ebca191, BRF_GRA },	     //  6	Characters
	
	{ "sr-08.a1",      0x02000, 0x3884d9eb, BRF_GRA },	     //  7	Tiles
	{ "sr-09.a2",      0x02000, 0x999cf6e0, BRF_GRA },	     //  8
	{ "sr-10.a3",      0x02000, 0x8edb273a, BRF_GRA },	     //  9
	{ "sr-11.a4",      0x02000, 0x3a2726c3, BRF_GRA },	     //  10
	{ "sr-12.a5",      0x02000, 0x1bd3d8bb, BRF_GRA },	     //  11
	{ "sr-13.a6",      0x02000, 0x658f02c4, BRF_GRA },	     //  12
	
	{ "sr-14.l1",      0x04000, 0x2528bec6, BRF_GRA },	     //  13	Sprites
	{ "sr-15.l2",      0x04000, 0xf89287aa, BRF_GRA },	     //  14
	{ "sr-16.n1",      0x04000, 0x024418f8, BRF_GRA },	     //  15
	{ "sr-17.n2",      0x04000, 0xe2c7e489, BRF_GRA },	     //  16
	
	{ "sb-5.e8",       0x00100, 0x93ab8153, BRF_GRA },	     //  17	PROMs
	{ "sb-6.e9",       0x00100, 0x8ab44f7d, BRF_GRA },	     //  18
	{ "sb-7.e10",      0x00100, 0xf4ade9a4, BRF_GRA },	     //  19
	{ "sb-0.f1",       0x00100, 0x6047d91b, BRF_GRA },	     //  20
	{ "sb-4.d6",       0x00100, 0x4858968d, BRF_GRA },	     //  21
	{ "sb-8.k3",       0x00100, 0xf6fad943, BRF_GRA },	     //  22
	{ "sb-2.d1",       0x00100, 0x8bb8b3df, BRF_GRA },	     //  23
	{ "sb-3.d2",       0x00100, 0x3b0c99af, BRF_GRA },	     //  24
	{ "sb-1.k6",       0x00100, 0x712ac508, BRF_GRA },	     //  25
	{ "sb-9.m11",      0x00100, 0x4921635c, BRF_GRA },	     //  26
};

STD_ROM_PICK(Drva)
STD_ROM_FN(Drva)

static struct BurnRomInfo DrvablRomDesc[] = {
	{ "3.bin",         0x08000, 0xf3184f5a, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "5.bin",         0x04000, 0x835f7b24, BRF_ESS | BRF_PRG }, //	 1
	{ "7.bin",         0x08000, 0x2f456c6e, BRF_ESS | BRF_PRG }, //	 2
	
	{ "1.bin",         0x04000, 0xbd87f06b, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	
	{ "2.bin",         0x02000, 0x6ebca191, BRF_GRA },	     //  4	Characters
	
	{ "9.bin",         0x04000, 0x60329fa4, BRF_GRA },	     //  5	Tiles
	{ "11.bin",        0x04000, 0x66bac116, BRF_GRA },	     //  6
	{ "13.bin",        0x04000, 0x623fcec1, BRF_GRA },	     //  7
	
	{ "14.bin",        0x08000, 0xdf2345ef, BRF_GRA },	     //  8	Sprites
	{ "16.bin",        0x08000, 0xc106b1ed, BRF_GRA },	     //  9
	
	{ "sb-5.e8",       0x00100, 0x93ab8153, BRF_GRA },	     //  10	PROMs
	{ "sb-6.e9",       0x00100, 0x8ab44f7d, BRF_GRA },	     //  11
	{ "sb-7.e10",      0x00100, 0xf4ade9a4, BRF_GRA },	     //  12
	{ "sb-0.f1",       0x00100, 0x6047d91b, BRF_GRA },	     //  13
	{ "sb-4.d6",       0x00100, 0x4858968d, BRF_GRA },	     //  14
	{ "sb-8.k3",       0x00100, 0xf6fad943, BRF_GRA },	     //  15
	{ "sb-2.d1",       0x00100, 0x8bb8b3df, BRF_GRA },	     //  16
	{ "sb-3.d2",       0x00100, 0x3b0c99af, BRF_GRA },	     //  17
	{ "sb-1.k6",       0x00100, 0x712ac508, BRF_GRA },	     //  18
	{ "sb-9.m11",      0x00100, 0x4921635c, BRF_GRA },	     //  19
};

STD_ROM_PICK(Drvabl)
STD_ROM_FN(Drvabl)

static struct BurnRomInfo DrvbRomDesc[] = {
	{ "sr-03.m3",      0x04000, 0x612975f2, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "sr-04.m4",      0x04000, 0xa60ac644, BRF_ESS | BRF_PRG }, //	 1
	{ "sr-05.m5",      0x04000, 0x835f7b24, BRF_ESS | BRF_PRG }, //	 2
	{ "sr-06.m6",      0x02000, 0x821c6481, BRF_ESS | BRF_PRG }, //	 3
	{ "sr-07.m7",      0x04000, 0x5df525e1, BRF_ESS | BRF_PRG }, //	 4
	
	{ "sr-01.c11",     0x04000, 0xbd87f06b, BRF_ESS | BRF_PRG }, //  5	Z80 #2 Program 
	
	{ "sr-02.f2",      0x02000, 0x6ebca191, BRF_GRA },	     //  6	Characters
	
	{ "sr-08.a1",      0x02000, 0x3884d9eb, BRF_GRA },	     //  7	Tiles
	{ "sr-09.a2",      0x02000, 0x999cf6e0, BRF_GRA },	     //  8
	{ "sr-10.a3",      0x02000, 0x8edb273a, BRF_GRA },	     //  9
	{ "sr-11.a4",      0x02000, 0x3a2726c3, BRF_GRA },	     //  10
	{ "sr-12.a5",      0x02000, 0x1bd3d8bb, BRF_GRA },	     //  11
	{ "sr-13.a6",      0x02000, 0x658f02c4, BRF_GRA },	     //  12
	
	{ "sr-14.l1",      0x04000, 0x2528bec6, BRF_GRA },	     //  13	Sprites
	{ "sr-15.l2",      0x04000, 0xf89287aa, BRF_GRA },	     //  14
	{ "sr-16.n1",      0x04000, 0x024418f8, BRF_GRA },	     //  15
	{ "sr-17.n2",      0x04000, 0xe2c7e489, BRF_GRA },	     //  16
	
	{ "sb-5.e8",       0x00100, 0x93ab8153, BRF_GRA },	     //  17	PROMs
	{ "sb-6.e9",       0x00100, 0x8ab44f7d, BRF_GRA },	     //  18
	{ "sb-7.e10",      0x00100, 0xf4ade9a4, BRF_GRA },	     //  19
	{ "sb-0.f1",       0x00100, 0x6047d91b, BRF_GRA },	     //  20
	{ "sb-4.d6",       0x00100, 0x4858968d, BRF_GRA },	     //  21
	{ "sb-8.k3",       0x00100, 0xf6fad943, BRF_GRA },	     //  22
	{ "sb-2.d1",       0x00100, 0x8bb8b3df, BRF_GRA },	     //  23
	{ "sb-3.d2",       0x00100, 0x3b0c99af, BRF_GRA },	     //  24
	{ "sb-1.k6",       0x00100, 0x712ac508, BRF_GRA },	     //  25
	{ "sb-9.m11",      0x00100, 0x4921635c, BRF_GRA },	     //  26
};

STD_ROM_PICK(Drvb)
STD_ROM_FN(Drvb)

static struct BurnRomInfo DrvwRomDesc[] = {
	{ "sw-03.m3",      0x04000, 0xafd79770, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "sw-04.m4",      0x04000, 0x933d9910, BRF_ESS | BRF_PRG }, //	 1
	{ "sw-05.m5",      0x04000, 0xe9a71bb6, BRF_ESS | BRF_PRG }, //	 2
	{ "sw-06.m6",      0x02000, 0x466f8248, BRF_ESS | BRF_PRG }, //	 3
	{ "sw-07.m7",      0x04000, 0xec41655e, BRF_ESS | BRF_PRG }, //	 4
	
	{ "sr-01.c11",     0x04000, 0xbd87f06b, BRF_ESS | BRF_PRG }, //  5	Z80 #2 Program 
	
	{ "sw-02.f2",      0x02000, 0xf8e9ada2, BRF_GRA },	     //  6	Characters
	
	{ "sr-08.a1",      0x02000, 0x3884d9eb, BRF_GRA },	     //  7	Tiles
	{ "sr-09.a2",      0x02000, 0x999cf6e0, BRF_GRA },	     //  8
	{ "sr-10.a3",      0x02000, 0x8edb273a, BRF_GRA },	     //  9
	{ "sr-11.a4",      0x02000, 0x3a2726c3, BRF_GRA },	     //  10
	{ "sr-12.a5",      0x02000, 0x1bd3d8bb, BRF_GRA },	     //  11
	{ "sr-13.a6",      0x02000, 0x658f02c4, BRF_GRA },	     //  12
	
	{ "sr-14.l1",      0x04000, 0x2528bec6, BRF_GRA },	     //  13	Sprites
	{ "sr-15.l2",      0x04000, 0xf89287aa, BRF_GRA },	     //  14
	{ "sr-16.n1",      0x04000, 0x024418f8, BRF_GRA },	     //  15
	{ "sr-17.n2",      0x04000, 0xe2c7e489, BRF_GRA },	     //  16
	
	{ "sb-5.e8",       0x00100, 0x93ab8153, BRF_GRA },	     //  17	PROMs
	{ "sb-6.e9",       0x00100, 0x8ab44f7d, BRF_GRA },	     //  18
	{ "sb-7.e10",      0x00100, 0xf4ade9a4, BRF_GRA },	     //  19
	{ "sb-0.f1",       0x00100, 0x6047d91b, BRF_GRA },	     //  20
	{ "sb-4.d6",       0x00100, 0x4858968d, BRF_GRA },	     //  21
	{ "sb-8.k3",       0x00100, 0xf6fad943, BRF_GRA },	     //  22
	{ "sb-2.d1",       0x00100, 0x8bb8b3df, BRF_GRA },	     //  23
	{ "sb-3.d2",       0x00100, 0x3b0c99af, BRF_GRA },	     //  24
	{ "sb-1.k6",       0x00100, 0x712ac508, BRF_GRA },	     //  25
	{ "sb-9.m11",      0x00100, 0x4921635c, BRF_GRA },	     //  26
};

STD_ROM_PICK(Drvw)
STD_ROM_FN(Drvw)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom1             = Next; Next += 0x1c000;
	DrvZ80Rom2             = Next; Next += 0x04000;
	DrvPromRed             = Next; Next += 0x00100;
	DrvPromGreen           = Next; Next += 0x00100;
	DrvPromBlue            = Next; Next += 0x00100;
	DrvPromCharLookup      = Next; Next += 0x00100;
	DrvPromTileLookup      = Next; Next += 0x00100;
	DrvPromSpriteLookup    = Next; Next += 0x00100;

	RamStart               = Next;

	DrvZ80Ram1             = Next; Next += 0x01000;
	DrvZ80Ram2             = Next; Next += 0x00800;
	DrvSpriteRam           = Next; Next += 0x00080;
	DrvFgVideoRam          = Next; Next += 0x00800;
	DrvBgVideoRam          = Next; Next += 0x00400;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x200 * 8 * 8;
	DrvTiles               = Next; Next += 0x200 * 16 * 16;
	DrvSprites             = Next; Next += 0x200 * 16 * 16;
	pFMBuffer              = (INT16*)Next; Next += nBurnSoundLen * 6 * sizeof(INT16);
	DrvPalette             = (UINT32*)Next; Next += 0x00600 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	for (INT32 i = 0; i < 2; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}
	
	for (INT32 i = 0; i < 2; i++) {
		AY8910Reset(i);
	}

	DrvPaletteBank = 0;
	DrvBgScroll[0] = 0;
	DrvBgScroll[1] = 0;
	DrvFlipScreen = 0;
	DrvSoundLatch = 0;
	DrvRomBank = 0;

	return 0;
}

UINT8 __fastcall Drv1942Read1(UINT16 a)
{
	switch (a) {
		case 0xc000: {
			return 0xff - DrvInput[0];
		}
		
		case 0xc001: {
			return 0xff - DrvInput[1];
		}
		
		case 0xc002: {
			return 0xff - DrvInput[2];
		}
		
		case 0xc003: {
			return DrvDip[0];
		}
		
		case 0xc004: {
			return DrvDip[1];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall Drv1942Write1(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xc800: {
			DrvSoundLatch = d;
			return;
		}
		
		case 0xc802: {
			DrvBgScroll[0] = d;
			return;
		}
		
		case 0xc803: {
			DrvBgScroll[1] = d;
			return;
		}
		
		case 0xc804: {
			DrvFlipScreen = d & 0x80;
			if (d & 0x10) {
				ZetClose();
				ZetOpen(1);
				ZetReset();
				ZetClose();
				ZetOpen(0);
			}
			return;
		}
		
		case 0xc805: {
			DrvPaletteBank = d;
			return;
		}
		
		case 0xc806: {
			d &= 0x03;
			DrvRomBank = d;
			ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000 + DrvRomBank * 0x4000 );
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000 + DrvRomBank * 0x4000 );
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall Drv1942Read2(UINT16 a)
{
	switch (a) {
		case 0x6000: {
			return DrvSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall Drv1942Write2(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0x8000: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x8001: {
			AY8910Write(0, 1, d);
			return;
		}
		
		case 0xc000: {
			AY8910Write(1, 0, d);
			return;
		}
		
		case 0xc001: {
			AY8910Write(1, 1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Write => %04X, %02X\n"), a, d);
		}
	}
}


static INT32 CharPlaneOffsets[2]   = { 4, 0 };
static INT32 CharXOffsets[8]       = { 0, 1, 2, 3, 8, 9, 10, 11 };
static INT32 CharYOffsets[8]       = { 0, 16, 32, 48, 64, 80, 96, 112 };
static INT32 TilePlaneOffsets[3]   = { 0, 0x20000, 0x40000 };
static INT32 TileXOffsets[16]      = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
static INT32 TileYOffsets[16]      = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };
static INT32 SpritePlaneOffsets[4] = { 0x40004, 0x40000, 4, 0 };
static INT32 SpriteXOffsets[16]    = { 0, 1, 2, 3, 8, 9, 10, 11, 256, 257, 258, 259, 264, 265, 266, 267 };
static INT32 SpriteYOffsets[16]    = { 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240 };

static void MachineInit()
{
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(Drv1942Read1);
	ZetSetWriteHandler(Drv1942Write1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom1             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom1             );
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0xcc00, 0xcc7f, 0, DrvSpriteRam           );
	ZetMapArea(0xcc00, 0xcc7f, 1, DrvSpriteRam           );
	ZetMapArea(0xcc00, 0xcc7f, 2, DrvSpriteRam           );
	ZetMapArea(0xd000, 0xd7ff, 0, DrvFgVideoRam          );
	ZetMapArea(0xd000, 0xd7ff, 1, DrvFgVideoRam          );
	ZetMapArea(0xd000, 0xd7ff, 2, DrvFgVideoRam          );
	ZetMapArea(0xd800, 0xdbff, 0, DrvBgVideoRam          );
	ZetMapArea(0xd800, 0xdbff, 1, DrvBgVideoRam          );
	ZetMapArea(0xd800, 0xdbff, 2, DrvBgVideoRam          );
	ZetMapArea(0xe000, 0xefff, 0, DrvZ80Ram1             );
	ZetMapArea(0xe000, 0xefff, 1, DrvZ80Ram1             );
	ZetMapArea(0xe000, 0xefff, 2, DrvZ80Ram1             );
	ZetMemEnd();
	ZetClose();
	
	ZetInit(1);
	ZetOpen(1);
	ZetSetReadHandler(Drv1942Read2);
	ZetSetWriteHandler(Drv1942Write2);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80Rom2             );
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80Rom2             );
	ZetMapArea(0x4000, 0x47ff, 0, DrvZ80Ram2             );
	ZetMapArea(0x4000, 0x47ff, 1, DrvZ80Ram2             );
	ZetMapArea(0x4000, 0x47ff, 2, DrvZ80Ram2             );
	ZetMemEnd();
	ZetClose();
	
	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;

	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);

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

	DrvTempRom = (UINT8 *)BurnMalloc(0x10000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x04000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x14000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x18000, 4, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 5, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 6, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x02000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x06000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0a000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 3, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c000, 16, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);

	// Load the PROMs
	nRet = BurnLoadRom(DrvPromRed,          17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromGreen,        18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBlue,         19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,   20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromTileLookup,   21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup, 22, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	MachineInit();
	
	return 0;
}

static INT32 DrvablInit()
{
	INT32 nRet = 0, nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x18000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x14000, 2, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 4, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 3, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  8, 1); if (nRet != 0) return 1;
	memcpy(DrvTempRom + 0x04000, DrvTempRom + 0x10000, 0x4000);
	memcpy(DrvTempRom + 0x00000, DrvTempRom + 0x14000, 0x4000);
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  9, 1); if (nRet != 0) return 1;
	memcpy(DrvTempRom + 0x0c000, DrvTempRom + 0x10000, 0x4000);
	memcpy(DrvTempRom + 0x08000, DrvTempRom + 0x14000, 0x4000);
	GfxDecode(0x200, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load the PROMs
	nRet = BurnLoadRom(DrvPromRed,          10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromGreen,        11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBlue,         12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,   13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromTileLookup,   14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup, 15, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	MachineInit();
	
	return 0;
}

static INT32 DrvExit()
{
	ZetExit();
	
	for (INT32 i = 0; i < 2; i++) {
		AY8910Exit(i);
	}

	GenericTilesExit();
	
	DrvPaletteBank = 0;
	DrvBgScroll[0] = 0;
	DrvBgScroll[1] = 0;
	DrvFlipScreen = 0;
	DrvSoundLatch = 0;
	DrvRomBank = 0;

	BurnFree(Mem);

	return 0;
}

static void DrvCalcPalette()
{
	INT32 i;
	UINT32 Palette[256];
	
	for (i = 0; i < 256; i++) {
		INT32 bit0, bit1, bit2, bit3, r, g, b;
		
		bit0 = (DrvPromRed[i] >> 0) & 0x01;
		bit1 = (DrvPromRed[i] >> 1) & 0x01;
		bit2 = (DrvPromRed[i] >> 2) & 0x01;
		bit3 = (DrvPromRed[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		bit0 = (DrvPromGreen[i] >> 0) & 0x01;
		bit1 = (DrvPromGreen[i] >> 1) & 0x01;
		bit2 = (DrvPromGreen[i] >> 2) & 0x01;
		bit3 = (DrvPromGreen[i] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		bit0 = (DrvPromBlue[i] >> 0) & 0x01;
		bit1 = (DrvPromBlue[i] >> 1) & 0x01;
		bit2 = (DrvPromBlue[i] >> 2) & 0x01;
		bit3 = (DrvPromBlue[i] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		Palette[i] = BurnHighCol(r, g, b, 0);
	}
	
	for (i = 0; i < 256; i++) {
		DrvPalette[i] = Palette[0x80 | DrvPromCharLookup[i]];
	}
	
	for (i = 0; i < 256; i++) {
		DrvPalette[256 + i +   0] = Palette[0x00 | DrvPromTileLookup[i]];
		DrvPalette[256 + i + 256] = Palette[0x10 | DrvPromTileLookup[i]];
		DrvPalette[256 + i + 512] = Palette[0x20 | DrvPromTileLookup[i]];
		DrvPalette[256 + i + 768] = Palette[0x30 | DrvPromTileLookup[i]];
	}
	
	for (i = 0; i < 256; i++) {
		DrvPalette[1280 + i] = Palette[0x40 | DrvPromSpriteLookup[i]];
	}
}

static void DrvRenderBgLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex, xScroll, Flip, xFlip, yFlip;
	
	xScroll = DrvBgScroll[0] | (DrvBgScroll[1] << 8);
	xScroll &= 0x1ff;
	
	for (mx = 0; mx < 16; mx++) {
		for (my = 0; my < 32; my++) {
			TileIndex = (my * 16) + mx;
			TileIndex = (TileIndex & 0x0f) | ((TileIndex & 0x01f0) << 1);
			
			Code = DrvBgVideoRam[TileIndex];
			Colour = DrvBgVideoRam[TileIndex + 0x10];
			Code += (Colour & 0x80) << 1;
			Flip = (Colour & 0x60) >> 5;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			Colour &= 0x1f;
			
			y = 16 * mx;
			x = 16 * my;
			x -= xScroll;
			if (x < -16) x += 512;
			y -= 16;

			if (x > 0 && x < 240 && y > 0 && y < 208) {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_FlipXY(pTransDraw, Code, x, y, Colour, 3, 256 + (0x100 * DrvPaletteBank), DrvTiles);
					} else {
						Render16x16Tile_FlipX(pTransDraw, Code, x, y, Colour, 3, 256 + (0x100 * DrvPaletteBank), DrvTiles);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_FlipY(pTransDraw, Code, x, y, Colour, 3, 256 + (0x100 * DrvPaletteBank), DrvTiles);
					} else {
						Render16x16Tile(pTransDraw, Code, x, y, Colour, 3, 256 + (0x100 * DrvPaletteBank), DrvTiles);
					}
				}
			} else {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 3, 256 + (0x100 * DrvPaletteBank), DrvTiles);
					} else {
						Render16x16Tile_FlipX_Clip(pTransDraw, Code, x, y, Colour, 3, 256 + (0x100 * DrvPaletteBank), DrvTiles);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_FlipY_Clip(pTransDraw, Code, x, y, Colour, 3, 256 + (0x100 * DrvPaletteBank), DrvTiles);
					} else {
						Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 3, 256 + (0x100 * DrvPaletteBank), DrvTiles);
					}
				}
			}
		}
	}
}

static void DrvRenderSpriteLayer()
{
	INT32 Offset;
	
	for (Offset = 0x80 - 4; Offset >= 0; Offset -= 4) {
		INT32 i, Code, Colour, sx, sy, Dir;
		
		Code = (DrvSpriteRam[Offset] & 0x7f) + (4 * (DrvSpriteRam[Offset + 1] & 0x20)) + (2 * (DrvSpriteRam[Offset] & 0x80));
		Colour = DrvSpriteRam[Offset + 1] & 0x0f;
		sx = DrvSpriteRam[Offset + 3] - (0x10 * (DrvSpriteRam[Offset + 1] & 0x10));
		sy = DrvSpriteRam[Offset + 2];
		Dir = 1;
		
		i = (DrvSpriteRam[Offset + 1] & 0xc0) >> 6;
		if (i == 2) i = 3;
		
		do {
			Render16x16Tile_Mask_Clip(pTransDraw, Code + i, sx, (sy + (16 * i) * Dir) - 16, Colour, 4, 15, 1280, DrvSprites);
		
			i--;
		} while (i >= 0);
	}
}

static void DrvRenderCharLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Code = DrvFgVideoRam[TileIndex];
			Colour = DrvFgVideoRam[TileIndex + 0x400];
			Code += (Colour & 0x80) << 1;
			Colour &= 0x3f;
			
			x = 8 * mx;
			y = 8 * my;
			
			y -= 16;

			if (x > 0 && x < 248 && y > 0 && y < 216) {
				Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 2, 0, 0, DrvChars);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 2, 0, 0, DrvChars);
			}

			TileIndex++;
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	DrvRenderBgLayer();
	DrvRenderSpriteLayer();
	DrvRenderCharLayer();
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 4;
	INT32 nSoundBufferPos = 0;

	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	nCyclesTotal[0] = 4000000 / 60;
	nCyclesTotal[1] = 3000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
//	ZetNewFrame();

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == 1) {
			ZetSetVector(0xcf);
			ZetRaiseIrq(0);
		}
		if (i == 3) {
			ZetSetVector(0xd7);
			ZetRaiseIrq(0);
		}
		ZetClose();

		// Run Z80 #2
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetRaiseIrq(0);
		ZetClose();
		
		// Render Sound Segment
		if (pBurnSoundOut) {
			INT32 nSample;
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n] >> 2;
				nSample += pAY8910Buffer[1][n] >> 2;
				nSample += pAY8910Buffer[2][n] >> 2;
				nSample += pAY8910Buffer[3][n] >> 2;
				nSample += pAY8910Buffer[4][n] >> 2;
				nSample += pAY8910Buffer[5][n] >> 2;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
    		}
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSample;
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n] >> 2;
				nSample += pAY8910Buffer[1][n] >> 2;
				nSample += pAY8910Buffer[2][n] >> 2;
				nSample += pAY8910Buffer[3][n] >> 2;
				nSample += pAY8910Buffer[4][n] >> 2;
				nSample += pAY8910Buffer[5][n] >> 2;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
 			}
		}
	}

	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029672;
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
		AY8910Scan(nAction, pnMin);

		// Scan critical driver variables
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
		SCAN_VAR(DrvRomBank);
		SCAN_VAR(DrvPaletteBank);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(DrvBgScroll);
		SCAN_VAR(DrvFlipScreen);		
		SCAN_VAR(DrvDip);
		SCAN_VAR(DrvInput);
	}
	
	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000 + DrvRomBank * 0x4000 );
		ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000 + DrvRomBank * 0x4000 );
		ZetClose();
	}

	return 0;
}

struct BurnDriver BurnDrvNineteen42 = {
	"1942", NULL, NULL, NULL, "1984",
	"1942 (Revision B)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen42a = {
	"1942a", "1942", NULL, NULL, "1984",
	"1942 (Revision A)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvaRomInfo, DrvaRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen42abl = {
	"1942abl", "1942", NULL, NULL, "1984",
	"1942 (Revision A, bootleg)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvablRomInfo, DrvablRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvablInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen42b = {
	"1942b", "1942", NULL, NULL, "1984",
	"1942 (First Version)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvbRomInfo, DrvbRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen42w = {
	"1942w", "1942", NULL, NULL, "1985",
	"1942 (Williams Electronics license)\0", NULL, "Capcom (Williams Electronics license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvwRomInfo, DrvwRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x600, 224, 256, 3, 4
};
