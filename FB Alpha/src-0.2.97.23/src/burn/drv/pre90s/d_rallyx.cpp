#include "tiles_generic.h"
#include "z80_intf.h"
#include "namco_snd.h"
#include "samples.h"

static UINT8 DrvInputPort0[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[2]            = {0, 0};
static UINT8 DrvInput[3]          = {0, 0, 0};
static UINT8 DrvReset             = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *DrvZ80Rom1          = NULL;
static UINT8 *DrvZ80Rom2          = NULL;
static UINT8 *DrvZ80Ram1          = NULL;
static UINT8 *DrvZ80Ram2          = NULL;
static UINT8 *DrvVideoRam         = NULL;
static UINT8 *DrvRadarAttrRam     = NULL;
static UINT8 *DrvPromPalette      = NULL;
static UINT8 *DrvPromLookup       = NULL;
static UINT8 *DrvPromVidLayout    = NULL;
static UINT8 *DrvPromVidTiming    = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvDots             = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette         = NULL;

static UINT8 DrvCPUFireIRQ;
static UINT8 DrvCPUIRQVector;
static UINT8 xScroll;
static UINT8 yScroll;
static UINT8 DrvLastBang;

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static struct BurnInputInfo DrvInputList[] =
{
	{ "Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 coin"   },
	{ "Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 start"  },
	{ "Coin 2"            , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 coin"   },
	{ "Start 2"           , BIT_DIGITAL  , DrvInputPort1 + 6, "p2 start"  },

	{ "Up"                , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 up"     },
	{ "Down"              , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 down"   },
	{ "Left"              , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 left"   },
	{ "Right"             , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 right"  },
	{ "Fire 1"            , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 fire 1" },
	
	{ "Up (Cocktail)"     , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 up"     },
	{ "Down (Cocktail)"   , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 down"   },
	{ "Left (Cocktail)"   , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 left"   },
	{ "Right (Cocktail)"  , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 right"  },
	{ "Fire 1 (Cocktail)" , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 fire 1" },

	{ "Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{ "Service"           , BIT_DIGITAL  , DrvInputPort0 + 0, "service"   },
	{ "Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{ "Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Drv)

static struct BurnInputInfo JunglerInputList[] =
{
	{ "Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 coin"   },
	{ "Start 1"           , BIT_DIGITAL  , DrvInputPort1 + 7, "p1 start"  },
	{ "Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 6, "p2 coin"   },
	{ "Start 2"           , BIT_DIGITAL  , DrvInputPort1 + 6, "p2 start"  },

	{ "Up"                , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 up"     },
	{ "Down"              , BIT_DIGITAL  , DrvInputPort2 + 7, "p1 down"   },
	{ "Left"              , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 left"   },
	{ "Right"             , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 right"  },
	{ "Fire 1"            , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 fire 1" },
	
	{ "Up (Cocktail)"     , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 up"     },
	{ "Down (Cocktail)"   , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 down"   },
	{ "Left (Cocktail)"   , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 left"   },
	{ "Right (Cocktail)"  , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 right"  },
	{ "Fire 1 (Cocktail)" , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 fire 1" },

	{ "Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{ "Service"           , BIT_DIGITAL  , DrvInputPort0 + 2, "service"   },
	{ "Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
};

STDINPUTINFO(Jungler)

static inline void DrvMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = 0xff;
	DrvInput[1] = 0xfe;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] -= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] -= (DrvInputPort1[i] & 1) << i;
	}
}

static inline void JunglerMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = 0xff;
	DrvInput[1] = 0xff;
	DrvInput[2] = 0xff;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] -= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] -= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] -= (DrvInputPort1[i] & 1) << i;
	}
}

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x01, NULL                     },
	{0x11, 0xff, 0xff, 0xcb, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x01, 0x00, "Cocktail"               },
	{0x10, 0x01, 0x01, 0x01, "Upright"                },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x11, 0x01, 0x01, 0x01, "Off"                    },
	{0x11, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             }, // This is conditional on difficulty
	{0x11, 0x01, 0x06, 0x02, "15000"                  },
	{0x11, 0x01, 0x06, 0x04, "30000"                  },
	{0x11, 0x01, 0x06, 0x06, "40000"                  },
	{0x11, 0x01, 0x06, 0x00, "None"                   },
	
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x11, 0x01, 0x38, 0x10, "1 Car,  Medium"         },
	{0x11, 0x01, 0x38, 0x28, "1 Car,  Hard"           },
	{0x11, 0x01, 0x38, 0x00, "2 Cars, Easy"           },
	{0x11, 0x01, 0x38, 0x18, "2 Cars, Medium"         },
	{0x11, 0x01, 0x38, 0x30, "2 Cars, Hard"           },
	{0x11, 0x01, 0x38, 0x08, "3 Cars, Easy"           },
	{0x11, 0x01, 0x38, 0x20, "3 Cars, Medium"         },
	{0x11, 0x01, 0x38, 0x38, "3 Cars, Hard"           },
	
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x11, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x11, 0x01, 0xc0, 0xc0, "1 Coin  1 Play"         },
	{0x11, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	{0x11, 0x01, 0xc0, 0x00, "Freeplay"               },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo JunglerDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0xbf, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x10, 0x01, 0x07, 0x01, "4 Coins 1 Play"         },
	{0x10, 0x01, 0x07, 0x02, "3 Coins 1 Play"         },
	{0x10, 0x01, 0x07, 0x03, "2 Coins 1 Play"         },
	{0x10, 0x01, 0x07, 0x00, "4 Coins 3 Plays"        },
	{0x10, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x10, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x10, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x10, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x10, 0x01, 0x38, 0x08, "4 Coins 1 Play"         },
	{0x10, 0x01, 0x38, 0x10, "3 Coins 1 Play"         },
	{0x10, 0x01, 0x38, 0x18, "2 Coins 1 Play"         },
	{0x10, 0x01, 0x38, 0x00, "4 Coins 3 Plays"        },
	{0x10, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x10, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x10, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
	{0x10, 0x01, 0x38, 0x20, "1 Coin  4 Plays"        },
		
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x40, 0x40, "Cocktail"               },
	{0x10, 0x01, 0x40, 0x00, "Upright"                },
	
	{0   , 0xfe, 0   , 2   , "Test Mode (255 lives)"  },
	{0x10, 0x01, 0x80, 0x80, "Off"                    },
	{0x10, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Jungler)

static struct BurnRomInfo RallyxRomDesc[] = {
	{ "1b",            0x01000, 0x5882700d, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "rallyxn.1e",    0x01000, 0xed1eba2b, BRF_ESS | BRF_PRG }, //	 1
	{ "rallyxn.1h",    0x01000, 0x4f98dd1c, BRF_ESS | BRF_PRG }, //	 2
	{ "rallyxn.1k",    0x01000, 0x9aacccf0, BRF_ESS | BRF_PRG }, //	 3
	
	{ "8e",            0x01000, 0x277c1de5, BRF_GRA },	     //  4	Characters & Sprites
	
	{ "rx1-6.8m",      0x00100, 0x3c16f62c, BRF_GRA },	     //  5	Dots
	
	{ "rx1-1.11n",     0x00020, 0xc7865434, BRF_GRA },	     //  6	Palette PROM
	{ "rx1-7.8p",      0x00100, 0x834d4fda, BRF_GRA },	     //  7	Lookup PROM
	{ "rx1-2.4n",      0x00020, 0x8f574815, BRF_GRA },	     //  8	Video Layout PROM
	{ "rx1-3.7k",      0x00020, 0xb8861096, BRF_GRA },	     //  9	Video Timing PROM
	
	{ "rx1-5.3p",      0x00100, 0x4bad7017, BRF_SND },	     //  10	Sound PROMs
	{ "rx1-4.2m",      0x00100, 0x77245b66, BRF_SND },	     //  11
};

STD_ROM_PICK(Rallyx)
STD_ROM_FN(Rallyx)

static struct BurnRomInfo RallyxaRomDesc[] = {
	{ "rx1_prg_1.1b",  0x00800, 0xef9238db, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "rx1_prg_2.1c",  0x00800, 0x7cbeb656, BRF_ESS | BRF_PRG }, //	 1
	{ "rx1_prg_3.1d",  0x00800, 0x334b1042, BRF_ESS | BRF_PRG }, //	 2
	{ "rx1_prg_4.1e",  0x00800, 0xd6618add, BRF_ESS | BRF_PRG }, //	 3
	{ "rx1_prg_5.bin", 0x00800, 0x3d69f24e, BRF_ESS | BRF_PRG }, //	 4
	{ "rx1_prg_6.bin", 0x00800, 0xe9740f16, BRF_ESS | BRF_PRG }, //	 5
	{ "rx1_prg_7.1k",  0x00800, 0x843109f2, BRF_ESS | BRF_PRG }, //	 6
	{ "rx1_prg_8.1l",  0x00800, 0x9b846ec9, BRF_ESS | BRF_PRG }, //	 7
	
	{ "rx1_chg_1.8e",  0x00800, 0x1fff38a4, BRF_GRA },	     //  8	Characters & Sprites
	{ "rx1_chg_2.8d",  0x00800, 0x68dff552, BRF_GRA },	     //  9
	
	{ "rx1-6.8m",      0x00100, 0x3c16f62c, BRF_GRA },	     //  10	Dots
	
	{ "rx1-1.11n",     0x00020, 0xc7865434, BRF_GRA },	     //  11	Palette PROM
	{ "rx1-7.8p",      0x00100, 0x834d4fda, BRF_GRA },	     //  12	Lookup PROM
	{ "rx1-2.4n",      0x00020, 0x8f574815, BRF_GRA },	     //  13	Video Layout PROM
	{ "rx1-3.7k",      0x00020, 0xb8861096, BRF_GRA },	     //  14	Video Timing PROM
	
	{ "rx1-5.3p",      0x00100, 0x4bad7017, BRF_SND },	     //  15	Sound PROMs
	{ "rx1-4.2m",      0x00100, 0x77245b66, BRF_SND },	     //  16
};

STD_ROM_PICK(Rallyxa)
STD_ROM_FN(Rallyxa)

static struct BurnRomInfo RallyxmRomDesc[] = {
	{ "1b",            0x01000, 0x5882700d, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "1e",            0x01000, 0x786585ec, BRF_ESS | BRF_PRG }, //	 1
	{ "1h",            0x01000, 0x110d7dcd, BRF_ESS | BRF_PRG }, //	 2
	{ "1k",            0x01000, 0x473ab447, BRF_ESS | BRF_PRG }, //	 3
	
	{ "8e",            0x01000, 0x277c1de5, BRF_GRA },	     //  4	Characters & Sprites
	
	{ "rx1-6.8m",      0x00100, 0x3c16f62c, BRF_GRA },	     //  5	Dots
	
	{ "rx1-1.11n",     0x00020, 0xc7865434, BRF_GRA },	     //  6	Palette PROM
	{ "rx1-7.8p",      0x00100, 0x834d4fda, BRF_GRA },	     //  7	Lookup PROM
	{ "rx1-2.4n",      0x00020, 0x8f574815, BRF_GRA },	     //  8	Video Layout PROM
	{ "rx1-3.7k",      0x00020, 0xb8861096, BRF_GRA },	     //  9	Video Timing PROM
	
	{ "rx1-5.3p",      0x00100, 0x4bad7017, BRF_SND },	     //  10	Sound PROMs
	{ "rx1-4.2m",      0x00100, 0x77245b66, BRF_SND },	     //  11
};

STD_ROM_PICK(Rallyxm)
STD_ROM_FN(Rallyxm)

static struct BurnRomInfo NrallyxRomDesc[] = {
	{ "nrx_prg1.1d",   0x01000, 0xba7de9fc, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "nrx_prg2.1e",   0x01000, 0xeedfccae, BRF_ESS | BRF_PRG }, //	 1
	{ "nrx_prg3.1k",   0x01000, 0xb4d5d34a, BRF_ESS | BRF_PRG }, //	 2
	{ "nrx_prg4.1l",   0x01000, 0x7da5496d, BRF_ESS | BRF_PRG }, //	 3
	
	{ "nrx_chg1.8e",   0x00800, 0x1fff38a4, BRF_GRA },	     //  4	Characters & Sprites
	{ "nrx_chg2.8d",   0x00800, 0x85d9fffd, BRF_GRA },	     //  5
	
	{ "rx1-6.8m",      0x00100, 0x3c16f62c, BRF_GRA },	     //  6	Dots
	
	{ "nrx1-1.11n",    0x00020, 0xa0a49017, BRF_GRA },	     //  7	Palette PROM
	{ "nrx1-7.8p",     0x00100, 0x4e46f485, BRF_GRA },	     //  8	Lookup PROM
	{ "rx1-2.4n",      0x00020, 0x8f574815, BRF_GRA },	     //  9	Video Layout PROM
	{ "rx1-3.7k",      0x00020, 0xb8861096, BRF_GRA },	     //  10	Video Timing PROM
	
	{ "rx1-5.3p",      0x00100, 0x4bad7017, BRF_SND },	     //  11	Sound PROMs
	{ "rx1-4.2m",      0x00100, 0x77245b66, BRF_SND },	     //  12
};

STD_ROM_PICK(Nrallyx)
STD_ROM_FN(Nrallyx)




static struct BurnRomInfo JunglerRomDesc[] = {
	{ "jungr1",        0x01000, 0x5bd6ad15, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code #1
	{ "jungr2",        0x01000, 0xdc99f1e3, BRF_ESS | BRF_PRG }, //  1
	{ "jungr3",        0x01000, 0x3dcc03da, BRF_ESS | BRF_PRG }, //  2
	{ "jungr4",        0x01000, 0xf92e9940, BRF_ESS | BRF_PRG }, //  3
	
	{ "1b",            0x01000, 0xf86999c3, BRF_ESS | BRF_PRG }, //  4	Z80 Program Code #2
	
	{ "5k",            0x00800, 0x924262bf, BRF_GRA },	     //  5	Characters & Sprites
	{ "5m",            0x00800, 0x131a08ac, BRF_GRA },	     //  6
	
	{ "82s129.10g",    0x00100, 0xc59c51b7, BRF_GRA },	     //  7	Dots
	
	{ "18s030.8b",     0x00020, 0x55a7e6d1, BRF_GRA },	     //  8	Palette PROM
	{ "tbp24s10.9d",   0x00100, 0xd223f7b8, BRF_GRA },	     //  9	Lookup PROM
	{ "18s030.7a",     0x00020, 0x8f574815, BRF_GRA },	     //  10	Video Layout PROM
	{ "6331-1.10a",    0x00020, 0xb8861096, BRF_GRA },	     //  11	Video Timing PROM
};

STD_ROM_PICK(Jungler)
STD_ROM_FN(Jungler)

static struct BurnSampleInfo RallyxSampleDesc[] = {
#if !defined (ROM_VERIFY)
   { "bang.wav", SAMPLE_NOLOOP },
#else
	{ "bang", SAMPLE_NOLOOP },
#endif
   { "", 0 }
};

STD_SAMPLE_PICK(Rallyx)
STD_SAMPLE_FN(Rallyx)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom1             = Next; Next += 0x04000;	
	DrvPromPalette         = Next; Next += 0x00020;
	DrvPromLookup          = Next; Next += 0x00100;
	DrvPromVidLayout       = Next; Next += 0x00020;
	DrvPromVidTiming       = Next; Next += 0x00020;
	NamcoSoundProm         = Next; Next += 0x00100;
	
	RamStart               = Next;

	DrvZ80Ram1             = Next; Next += 0x00800;	
	DrvVideoRam            = Next; Next += 0x01000;
	DrvRadarAttrRam        = Next; Next += 0x00010;
	
	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x100 * 8 * 8;
	DrvSprites             = Next; Next += 0x040 * 16 * 16;
	DrvDots                = Next; Next += 0x008 * 4 * 4;
	DrvPalette             = (UINT32*)Next; Next += 260 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 JunglerMemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom1             = Next; Next += 0x08000;
	DrvZ80Rom2             = Next; Next += 0x02000;
	DrvPromPalette         = Next; Next += 0x00020;
	DrvPromLookup          = Next; Next += 0x00100;
	DrvPromVidLayout       = Next; Next += 0x00020;
	DrvPromVidTiming       = Next; Next += 0x00020;
	
	RamStart               = Next;

	DrvZ80Ram1             = Next; Next += 0x00800;	
	DrvZ80Ram2             = Next; Next += 0x00400;	
	DrvVideoRam            = Next; Next += 0x01000;
	DrvRadarAttrRam        = Next; Next += 0x00010;
	
	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x100 * 8 * 8;
	DrvSprites             = Next; Next += 0x040 * 16 * 16;
	DrvDots                = Next; Next += 0x008 * 4 * 4;
	DrvPalette             = (UINT32*)Next; Next += 324 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	ZetOpen(0);
	ZetReset();
	ZetClose();
	
	BurnSampleReset();
	
	DrvCPUFireIRQ = 0;
	DrvCPUIRQVector = 0;
	xScroll = 0;
	yScroll = 0;
	DrvLastBang = 0;

	return 0;
}

static INT32 JunglerDoReset()
{
	ZetOpen(0);
	ZetReset();
	ZetClose();
	
	DrvCPUFireIRQ = 0;
	DrvCPUIRQVector = 0;
	xScroll = 0;
	yScroll = 0;

	return 0;
}

UINT8 __fastcall RallyxZ80ProgRead(UINT16 a)
{
	switch (a) {
		case 0xa000: {
			return DrvInput[0];
		}
		
		case 0xa080: {
			return DrvInput[1] | DrvDip[0];
		}
		
		case 0xa100: {
			return DrvDip[1];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read %04x\n"), a);
		}
	}
	
	return 0;
}

void __fastcall RallyxZ80ProgWrite(UINT16 a, UINT8 d)
{
	if (a >= 0xa100 && a <= 0xa11f) { NamcoSoundWrite(a - 0xa100, d); return; }
	
	switch (a) {
		case 0xa080: {
			// watchdog write
			return;
		}
		
		case 0xa130: {
			xScroll = d;
			return;
		}
		
		case 0xa140: {
			yScroll = d;
			return;
		}
		
		case 0xa170: {
			// NOP
			return;
		}
		
		case 0xa180: {
			UINT8 Bit = d & 0x01;
			if (Bit == 0 && DrvLastBang != 0) {
				BurnSamplePlay(0);
			}
			DrvLastBang = Bit;
			return;
		}
		
		case 0xa181: {
			DrvCPUFireIRQ = d & 0x01;
			if (!DrvCPUFireIRQ) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return;
		}
		
		case 0xa182: {
			// sound on
			return;
		}
		
		case 0xa183: {
			// flip screen !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			return;
		}
		
		case 0xa184: {
			// lamp
			return;
		}
		
		case 0xa185: {
			// lamp
			return;
		}
		
		case 0xa186: {
			// coin lockout
			return;
		}
		
		case 0xa187: {
			// coin counter
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write %04x, %02x\n"), a, d);
		}
	}
}

UINT8 __fastcall RallyxZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall RallyxZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			DrvCPUIRQVector = d;
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return;
		}
				
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall JunglerZ80ProgRead1(UINT16 a)
{
	switch (a) {
		case 0xa000: {
			return DrvInput[0];
		}
		
		case 0xa080: {
			return DrvInput[1];
		}
		
		case 0xa100: {
			return DrvInput[2];
		}
		
		case 0xa180: {
			return DrvDip[0];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read %04x\n"), a);
		}
	}
	
	return 0;
}

void __fastcall JunglerZ80ProgWrite1(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xa080: {
			// watchdog write
			return;
		}
		
		case 0xa130: {
			xScroll = d;
			return;
		}
		
		case 0xa140: {
			yScroll = d;
			return;
		}
		
		case 0xa181: {
			DrvCPUFireIRQ = d & 0x01;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write %04x, %02x\n"), a, d);
		}
	}
}

UINT8 __fastcall JunglerZ80PortRead1(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall JunglerZ80PortWrite1(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall JunglerZ80ProgRead2(UINT16 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read %04x\n"), a);
		}
	}
	
	return 0;
}

void __fastcall JunglerZ80ProgWrite2(UINT16 a, UINT8 d)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Write %04x, %02x\n"), a, d);
		}
	}
}

UINT8 __fastcall JunglerZ80PortRead2(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall JunglerZ80PortWrite2(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

static INT32 CharPlaneOffsets[2]          = { 0, 4 };
static INT32 CharXOffsets[8]              = { 64, 65, 66, 67, 0, 1, 2, 3 };
static INT32 CharYOffsets[8]              = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 JunglerCharPlaneOffsets[2]   = { 4, 0 };
static INT32 SpritePlaneOffsets[2]        = { 0, 4 };
static INT32 SpriteXOffsets[16]           = { 64, 65, 66, 67, 128, 129, 130, 131, 192, 193, 194, 195, 0, 1, 2, 3 };
static INT32 SpriteYOffsets[16]           = { 0, 8, 16, 24, 32, 40, 48, 56, 256, 264, 272, 280, 288, 296, 304, 312 };
static INT32 JunglerSpritePlaneOffsets[2] = { 4, 0 };
static INT32 JunglerSpriteXOffsets[16]    = { 64, 65, 66, 67, 0, 1, 2, 3, 192, 193, 194, 195, 128, 129, 130, 131 };
static INT32 DotPlaneOffsets[2]           = { 6, 7 };
static INT32 DotXOffsets[4]               = { 0, 8, 16, 24 };
static INT32 DotYOffsets[4]               = { 0, 32, 64, 96 };

static void MachineInit()
{
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(RallyxZ80ProgRead);
	ZetSetWriteHandler(RallyxZ80ProgWrite);
	ZetSetInHandler(RallyxZ80PortRead);
	ZetSetOutHandler(RallyxZ80PortWrite);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80Rom1);
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80Rom1);
	ZetMapArea(0x8000, 0x8fff, 0, DrvVideoRam);
	ZetMapArea(0x8000, 0x8fff, 1, DrvVideoRam);
	ZetMapArea(0x8000, 0x8fff, 2, DrvVideoRam);
	ZetMapArea(0x9800, 0x9fff, 0, DrvZ80Ram1);
	ZetMapArea(0x9800, 0x9fff, 1, DrvZ80Ram1);
	ZetMapArea(0x9800, 0x9fff, 2, DrvZ80Ram1);
	ZetMapArea(0xa000, 0xa00f, 1, DrvRadarAttrRam);	
	ZetMemEnd();
	ZetClose();
	
	NamcoSoundInit(18432000 / 6 / 32, 3);
	NacmoSoundSetAllRoutes(1.00, BURN_SND_ROUTE_BOTH);
	BurnSampleInit(0);
	BurnSampleSetAllRoutesAllSamples(0.80, BURN_SND_ROUTE_BOTH);
	
	GenericTilesInit();

	DrvDoReset();
}

static void JunglerMachineInit()
{
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(JunglerZ80ProgRead1);
	ZetSetWriteHandler(JunglerZ80ProgWrite1);
	ZetSetInHandler(JunglerZ80PortRead1);
	ZetSetOutHandler(JunglerZ80PortWrite1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom1);
	ZetMapArea(0x8000, 0x8fff, 0, DrvVideoRam);
	ZetMapArea(0x8000, 0x8fff, 1, DrvVideoRam);
	ZetMapArea(0x8000, 0x8fff, 2, DrvVideoRam);
	ZetMapArea(0x9800, 0x9fff, 0, DrvZ80Ram1);
	ZetMapArea(0x9800, 0x9fff, 1, DrvZ80Ram1);
	ZetMapArea(0x9800, 0x9fff, 2, DrvZ80Ram1);
	ZetMapArea(0xa000, 0xa00f, 1, DrvRadarAttrRam);	
	ZetMapArea(0xa030, 0xa03f, 1, DrvRadarAttrRam);	
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	// not used?
	
	GenericTilesInit();

	JunglerDoReset();
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

	DrvTempRom = (UINT8 *)BurnMalloc(0x01000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x01000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x02000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x03000,  3, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars and sprites
	nRet = BurnLoadRom(DrvTempRom,            4, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(0x40, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load and decode the dots
	memset(DrvTempRom, 0, 0x1000);
	nRet = BurnLoadRom(DrvTempRom,            5, 1); if (nRet != 0) return 1;
	GfxDecode(0x08, 2, 4, 4, DotPlaneOffsets, DotXOffsets, DotYOffsets, 0x80, DrvTempRom, DrvDots);

	// Load the PROMs
	nRet = BurnLoadRom(DrvPromPalette,        6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromLookup,         7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromVidLayout,      8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromVidTiming,      9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(NamcoSoundProm,       10, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	MachineInit();

	return 0;
}

static INT32 DrvaInit()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x01000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00800,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x01000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x01800,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x02000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x02800,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x03000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x03800,  7, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars and sprites
	nRet = BurnLoadRom(DrvTempRom + 0x0000,   8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0800,   9, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(0x40, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load and decode the dots
	memset(DrvTempRom, 0, 0x1000);
	nRet = BurnLoadRom(DrvTempRom,           10, 1); if (nRet != 0) return 1;
	GfxDecode(0x08, 2, 4, 4, DotPlaneOffsets, DotXOffsets, DotYOffsets, 0x80, DrvTempRom, DrvDots);

	// Load the PROMs
	nRet = BurnLoadRom(DrvPromPalette,       11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromLookup,        12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromVidLayout,     13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromVidTiming,     14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(NamcoSoundProm,       15, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	MachineInit();

	return 0;
}

static INT32 NrallyxInit()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x01000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvTempRom, 0, 1); if (nRet != 0) return 1;
	memcpy(DrvZ80Rom1 + 0x0000, DrvTempRom + 0x0000, 0x800);
	memcpy(DrvZ80Rom1 + 0x1000, DrvTempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(DrvTempRom, 1, 1); if (nRet != 0) return 1;
	memcpy(DrvZ80Rom1 + 0x0800, DrvTempRom + 0x0000, 0x800);
	memcpy(DrvZ80Rom1 + 0x1800, DrvTempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(DrvTempRom, 2, 1); if (nRet != 0) return 1;
	memcpy(DrvZ80Rom1 + 0x2000, DrvTempRom + 0x0000, 0x800);
	memcpy(DrvZ80Rom1 + 0x3000, DrvTempRom + 0x0800, 0x800);
	nRet = BurnLoadRom(DrvTempRom, 3, 1); if (nRet != 0) return 1;
	memcpy(DrvZ80Rom1 + 0x2800, DrvTempRom + 0x0000, 0x800);
	memcpy(DrvZ80Rom1 + 0x3800, DrvTempRom + 0x0800, 0x800);
	
	// Load and decode the chars and sprites
	memset(DrvTempRom, 0, 0x1000);
	nRet = BurnLoadRom(DrvTempRom + 0x0000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0800,  5, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(0x40, 2, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load and decode the dots
	memset(DrvTempRom, 0, 0x1000);
	nRet = BurnLoadRom(DrvTempRom,           6, 1); if (nRet != 0) return 1;
	GfxDecode(0x08, 2, 4, 4, DotPlaneOffsets, DotXOffsets, DotYOffsets, 0x80, DrvTempRom, DrvDots);

	// Load the PROMs
	nRet = BurnLoadRom(DrvPromPalette,       7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromLookup,        8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromVidLayout,     9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromVidTiming,    10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(NamcoSoundProm,      11, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	MachineInit();

	return 0;
}

static INT32 JunglerInit()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	JunglerMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	JunglerMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x01000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x01000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x02000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x03000,  3, 1); if (nRet != 0) return 1;
	
	// Load Z80 Sound Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000,  4, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars and sprites
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00800,  6, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 2, 8, 8, JunglerCharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(0x40, 2, 16, 16, JunglerSpritePlaneOffsets, JunglerSpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load and decode the dots
	memset(DrvTempRom, 0, 0x1000);
	nRet = BurnLoadRom(DrvTempRom,            7, 1); if (nRet != 0) return 1;
	GfxDecode(0x08, 2, 4, 4, DotPlaneOffsets, DotXOffsets, DotYOffsets, 0x80, DrvTempRom, DrvDots);

	// Load the PROMs
	nRet = BurnLoadRom(DrvPromPalette,        8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromLookup,         9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromVidLayout,     10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromVidTiming,     11, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	JunglerMachineInit();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();
	NamcoSoundExit();
	BurnSampleExit();
	ZetExit();
	
	BurnFree(Mem);
	
	DrvCPUFireIRQ = 0;
	DrvCPUIRQVector = 0;
	xScroll = 0;
	yScroll = 0;
	DrvLastBang = 0;
	
	return 0;
}

#define MAX_NETS			3
#define MAX_RES_PER_NET			18
#define Combine2Weights(tab,w0,w1)	((int)(((tab)[0]*(w0) + (tab)[1]*(w1)) + 0.5))
#define Combine3Weights(tab,w0,w1,w2)	((int)(((tab)[0]*(w0) + (tab)[1]*(w1) + (tab)[2]*(w2)) + 0.5))

static double ComputeResistorWeights(INT32 MinVal, INT32 MaxVal, double Scaler, INT32 Count1, const INT32 *Resistances1, double *Weights1, INT32 PullDown1, INT32 PullUp1,	INT32 Count2, const INT32 *Resistances2, double *Weights2, INT32 PullDown2, INT32 PullUp2, INT32 Count3, const INT32 *Resistances3, double *Weights3, INT32 PullDown3, INT32 PullUp3)
{
	INT32 NetworksNum;

	INT32 ResCount[MAX_NETS];
	double r[MAX_NETS][MAX_RES_PER_NET];
	double w[MAX_NETS][MAX_RES_PER_NET];
	double ws[MAX_NETS][MAX_RES_PER_NET];
	INT32 r_pd[MAX_NETS];
	INT32 r_pu[MAX_NETS];

	double MaxOut[MAX_NETS];
	double *Out[MAX_NETS];

	INT32 i, j, n;
	double Scale;
	double Max;

	NetworksNum = 0;
	for (n = 0; n < MAX_NETS; n++) {
		INT32 Count, pd, pu;
		const INT32 *Resistances;
		double *Weights;

		switch (n) {
			case 0: {
				Count = Count1;
				Resistances = Resistances1;
				Weights = Weights1;
				pd = PullDown1;
				pu = PullUp1;
				break;
			}
			
			case 1: {
				Count = Count2;
				Resistances = Resistances2;
				Weights = Weights2;
				pd = PullDown2;
				pu = PullUp2;
				break;
			}
		
			case 2:
			default: {
				Count = Count3;
				Resistances = Resistances3;
				Weights = Weights3;
				pd = PullDown3;
				pu = PullUp3;
				break;
			}
		}

		if (Count > 0) {
			ResCount[NetworksNum] = Count;
			for (i = 0; i < Count; i++) {
				r[NetworksNum][i] = 1.0 * Resistances[i];
			}
			Out[NetworksNum] = Weights;
			r_pd[NetworksNum] = pd;
			r_pu[NetworksNum] = pu;
			NetworksNum++;
		}
	}

	for (i = 0; i < NetworksNum; i++) {
		double R0, R1, Vout, Dst;

		for (n = 0; n < ResCount[i]; n++) {
			R0 = (r_pd[i] == 0) ? 1.0 / 1e12 : 1.0 / r_pd[i];
			R1 = (r_pu[i] == 0) ? 1.0 / 1e12 : 1.0 / r_pu[i];

			for (j = 0; j < ResCount[i]; j++) {
				if (j == n) {
					if (r[i][j] != 0.0) R1 += 1.0 / r[i][j];
				} else {
					if (r[i][j] != 0.0) R0 += 1.0 / r[i][j];
				}
			}

			R0 = 1.0/R0;
			R1 = 1.0/R1;
			Vout = (MaxVal - MinVal) * R0 / (R1 + R0) + MinVal;

			Dst = (Vout < MinVal) ? MinVal : (Vout > MaxVal) ? MaxVal : Vout;

			w[i][n] = Dst;
		}
	}

	j = 0;
	Max = 0.0;
	for (i = 0; i < NetworksNum; i++) {
		double Sum = 0.0;

		for (n = 0; n < ResCount[i]; n++) Sum += w[i][n];

		MaxOut[i] = Sum;
		if (Max < Sum) {
			Max = Sum;
			j = i;
		}
	}

	if (Scaler < 0.0) {
		Scale = ((double)MaxVal) / MaxOut[j];
	} else {
		Scale = Scaler;
	}

	for (i = 0; i < NetworksNum; i++) {
		for (n = 0; n < ResCount[i]; n++) {
			ws[i][n] = w[i][n] * Scale;
			(Out[i])[n] = ws[i][n];
		}
	}

	return Scale;

}

static void DrvCalcPalette()
{
	static const INT32 ResistancesRG[3] = { 1000, 470, 220 };
	static const INT32 ResistancesB[2] = { 470, 220 };
	double rWeights[3], gWeights[3], bWeights[2];
	UINT32 Palette[32];
	UINT32 i;
	
	ComputeResistorWeights(0, 255, -1.0, 3, &ResistancesRG[0], rWeights, 0, 0, 3, &ResistancesRG[0], gWeights, 0, 0, 2, &ResistancesB[0], bWeights, 1000, 0);
	
	for (i = 0; i < 32; i++) {
		INT32 Bit0, Bit1, Bit2;
		INT32 r, g, b;

		Bit0 = (DrvPromPalette[i] >> 0) & 0x01;
		Bit1 = (DrvPromPalette[i] >> 1) & 0x01;
		Bit2 = (DrvPromPalette[i] >> 2) & 0x01;
		r = Combine3Weights(rWeights, Bit0, Bit1, Bit2);

		/* green component */
		Bit0 = (DrvPromPalette[i] >> 3) & 0x01;
		Bit1 = (DrvPromPalette[i] >> 4) & 0x01;
		Bit2 = (DrvPromPalette[i] >> 5) & 0x01;
		g = Combine3Weights(gWeights, Bit0, Bit1, Bit2);

		/* blue component */
		Bit0 = (DrvPromPalette[i] >> 6) & 0x01;
		Bit1 = (DrvPromPalette[i] >> 7) & 0x01;
		b = Combine2Weights(bWeights, Bit0, Bit1);

		Palette[i] = BurnHighCol(r, g, b, 0);
	}
	
	for (i = 0; i < 256; i++) {
		UINT8 PaletteEntry = DrvPromLookup[i] & 0x0f;
		DrvPalette[i] = Palette[PaletteEntry];
	}
	
	for (i = 256; i < 260; i++) {
		DrvPalette[i] = Palette[(i - 0x100) | 0x10];
	}
}

#undef MAX_NETS
#undef MAX_RES_PER_NET
#undef Combine2Weights
#undef Combine3Weights

static void DrvRenderBgLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Code   = DrvVideoRam[TileIndex + 0x400 + 0x000];
			Colour = DrvVideoRam[TileIndex + 0x400 + 0x800];
			Flip = ((Colour >> 6) & 0x03) ^ 1;
			xFlip = Flip & 0x01;
			yFlip = Flip & 0x02;
			Colour &= 0x3f;

			x = 8 * mx;
			y = 8 * my;
			
			x -= xScroll;
			y -= yScroll;
			
			x += 3;
			y -= 16;
				
			if (x < -8) x += 256;
			if (x > 247) x -= 256;
			if (y < -8) y += 256;
			
			if (x < 224) {
				if (x > 8 && x < 280 && y > 8 && y < 216) {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_FlipXY(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
						} else {
							Render8x8Tile_FlipX(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_FlipY(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
						} else {
							Render8x8Tile(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
						} else {
							Render8x8Tile_FlipX_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_FlipY_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
						} else {
							Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
						}
					}
				}				
			}
			
			TileIndex++;
		}
	}
}

static void DrvRenderFgLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex, Flip, xFlip, yFlip;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 8; mx++) {
			TileIndex = mx + (my << 5);
			Code   = DrvVideoRam[TileIndex + 0x000 + 0x000];
			Colour = DrvVideoRam[TileIndex + 0x000 + 0x800];
			Flip = ((Colour >> 6) & 0x03) ^ 1;
			xFlip = Flip & 0x01;
			yFlip = Flip & 0x02;
			Colour &= 0x3f;

			x = 8 * mx;
			y = 8 * my;
			
			y -= 16;
			
			x -= 32;
			if (x < 0) x += 64;
			
			x += 224;
						
			if (x > 8 && x < 280 && y > 8 && y < 216) {
				if (xFlip) {
					if (yFlip) {
						Render8x8Tile_FlipXY(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
					} else {
						Render8x8Tile_FlipX(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
					}
				} else {
					if (yFlip) {
						Render8x8Tile_FlipY(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
					} else {
						Render8x8Tile(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
					}
				}
			} else {
				if (xFlip) {
					if (yFlip) {
						Render8x8Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
					} else {
						Render8x8Tile_FlipX_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
					}
				} else {
					if (yFlip) {
						Render8x8Tile_FlipY_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
					} else {
						Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
					}
				}
			}
		}
	}
}

static void DrvRenderSprites(INT32 Displacement)
{
	UINT32 SpriteRamBase = 0x14;
	UINT8 *SpriteRam = DrvVideoRam;
	UINT8 *SpriteRam2 = DrvVideoRam + 0x800;
	
	for (UINT32 Offs = 0x20 - 2; Offs >= SpriteRamBase; Offs -= 2) {
		INT32 sx = SpriteRam[Offs + 1] + ((SpriteRam2[Offs + 1] & 0x80) << 1) - Displacement;
		INT32 sy = 241 - SpriteRam2[Offs] - Displacement;
		INT32 Colour = SpriteRam2[Offs + 1] & 0x3f;
		INT32 xFlip = SpriteRam[Offs] & 1;
		INT32 yFlip = SpriteRam[Offs] & 2;
		INT32 Code = (SpriteRam[Offs] & 0xfc) >> 2;

//		if (flip_screen_get(machine))
//			sx -= 2 * displacement;


		sy -= 16;
		
		if (sx > 16 && sx < 272 && sy > 16 && sy < 208) {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Code, sx, sy, Colour, 2, 0, 0, DrvSprites);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Code, sx, sy, Colour, 2, 0, 0, DrvSprites);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Code, sx, sy, Colour, 2, 0, 0, DrvSprites);
				} else {
					Render16x16Tile_Mask(pTransDraw, Code, sx, sy, Colour, 2, 0, 0, DrvSprites);
				}
			}
		} else {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx, sy, Colour, 2, 0, 0, DrvSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, sx, sy, Colour, 2, 0, 0, DrvSprites);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, sx, sy, Colour, 2, 0, 0, DrvSprites);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, sy, Colour, 2, 0, 0, DrvSprites);
				}
			}
		}
	}
}

static void DrvRenderBullets()
{
	UINT32 SpriteRamBase = 0x14;
	UINT8 *RadarX = DrvVideoRam + 0x020;
	UINT8 *RadarY = DrvVideoRam + 0x820;
	
	for (UINT32 Offs = SpriteRamBase; Offs < 0x20; Offs++) {
		INT32 x, y, Code;

		x = RadarX[Offs] + ((~DrvRadarAttrRam[Offs & 0x0f] & 0x01) << 8);
		y = 253 - RadarY[Offs];
		Code = ((DrvRadarAttrRam[Offs & 0x0f] & 0x0e) >> 1) ^ 0x07;
		
		y -= 16;
		
//		if (flip_screen_get(machine))
//			x -= 3;

//		if (transpen)
//			drawgfx_transpen(bitmap,cliprect,machine->gfx[2],
//					((state->radarattr[offs & 0x0f] & 0x0e) >> 1) ^ 0x07,
//					0,
//					0,0,
//					x,y,
//					3);
//		else
//			drawgfx_transtable(bitmap,cliprect,machine->gfx[2],
//					((state->radarattr[offs & 0x0f] & 0x0e) >> 1) ^ 0x07,
//					0,
//					0,0,
//					x,y,
///					state->drawmode_table,machine->shadow_table);

		RenderCustomTile_Mask_Clip(pTransDraw, 4, 4, Code, x, y, 0, 2, 3, 0x100, DrvDots);
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	DrvRenderBgLayer();
	DrvRenderSprites(1);
	DrvRenderFgLayer();
	DrvRenderBullets();
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nInterleave = nBurnSoundLen;
	
	if (DrvReset) DrvDoReset();

	DrvMakeInputs();
	
	INT32 nSoundBufferPos = 0;

	nCyclesTotal[0] = (18432000 / 6) / 60;
	nCyclesDone[0] = 0;
	
	ZetNewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;
		
		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		if (i == (nInterleave - 1) && DrvCPUFireIRQ) {
			ZetSetVector(DrvCPUIRQVector);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		}
		ZetClose();
		
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

	return 0;
}

static INT32 JunglerFrame()
{
	INT32 nInterleave = 10;
	
	if (DrvReset) JunglerDoReset();

	JunglerMakeInputs();
	
//	INT32 nSoundBufferPos = 0;

	nCyclesTotal[0] = (18432000 / 6) / 60;
	nCyclesTotal[1] = (14318180 / 8) / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	ZetNewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;
		
		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		if (i == (nInterleave - 1) && DrvCPUFireIRQ) {
			ZetNmi();
		}
		ZetClose();
		
/*		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			
			if (nSegmentLength) {
				NamcoSoundUpdate(pSoundBuf, nSegmentLength);
				BurnSampleRender(pSoundBuf, nSegmentLength);
			}
			nSoundBufferPos += nSegmentLength;
		}*/
	}
	
/*	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			NamcoSoundUpdate(pSoundBuf, nSegmentLength);
			BurnSampleRender(pSoundBuf, nSegmentLength);
		}
	}*/

	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029708;
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
	}
	
	return 0;
}

struct BurnDriver BurnDrvRallyx = {
	"rallyx", NULL, NULL, "rallyx", "1980",
	"Rally X (32k Ver.?))\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, RallyxRomInfo, RallyxRomName, RallyxSampleInfo, RallyxSampleName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 260, 288, 224, 4, 3
};

struct BurnDriver BurnDrvRallyxa = {
	"rallyxa", "rallyx", NULL, "rallyx", "1980",
	"Rally X\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, RallyxaRomInfo, RallyxaRomName, RallyxSampleInfo, RallyxSampleName, DrvInputInfo, DrvDIPInfo,
	DrvaInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 260, 288, 224, 4, 3
};

struct BurnDriver BurnDrvRallyxm = {
	"rallyxm", "rallyx", NULL, "rallyx", "1980",
	"Rally X (Midway)\0", NULL, "Namco (Midway License)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, RallyxmRomInfo, RallyxmRomName, RallyxSampleInfo, RallyxSampleName, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 260, 288, 224, 4, 3
};

struct BurnDriver BurnDrvNrallyx = {
	"nrallyx", NULL, NULL, "rallyx", "1981",
	"New Rally X\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, NrallyxRomInfo, NrallyxRomName, RallyxSampleInfo, RallyxSampleName, DrvInputInfo, DrvDIPInfo,
	NrallyxInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 260, 288, 224, 4, 3
};



struct BurnDriverD BurnDrvJungler = {
	"jungler", NULL, NULL, NULL, "1981",
	"Jungler\0", NULL, "Konami", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, JunglerRomInfo, JunglerRomName, NULL, NULL, JunglerInputInfo, JunglerDIPInfo,
	JunglerInit, DrvExit, JunglerFrame, NULL, DrvScan,
	NULL, 324, 224, 288, 3, 4
};
