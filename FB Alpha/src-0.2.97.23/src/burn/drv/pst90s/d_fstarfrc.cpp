#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "msm6295.h"
#include "burn_ym2151.h"

static UINT8  FstarfrcInputPort0[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UINT8  FstarfrcInputPort1[6]  = {0, 0, 0, 0, 0, 0};
static UINT8  FstarfrcDip[2]         = {0, 0};
static UINT16 FstarfrcInput[2]       = {0x00, 0x00};
static UINT8  FstarfrcReset          = 0;

static UINT8  *Mem                   = NULL;
static UINT8  *MemEnd                = NULL;
static UINT8  *RamStart              = NULL;
static UINT8  *RamEnd                = NULL;
static UINT8  *FstarfrcRom           = NULL;
static UINT8  *FstarfrcZ80Rom        = NULL;
static UINT8  *FstarfrcRam           = NULL;
static UINT8  *FstarfrcCharRam       = NULL;
static UINT8  *FstarfrcVideoRam      = NULL;
static UINT8  *FstarfrcColourRam     = NULL;
static UINT8  *FstarfrcVideo2Ram     = NULL;
static UINT8  *FstarfrcColour2Ram    = NULL;
static UINT8  *FstarfrcSpriteRam     = NULL;
static UINT8  *FstarfrcPaletteRam    = NULL;
static UINT8  *FstarfrcZ80Ram        = NULL;
static UINT32 *FstarfrcPalette       = NULL;
static UINT8  *FstarfrcCharTiles     = NULL;
static UINT8  *FstarfrcLayerTiles    = NULL;
static UINT8  *FstarfrcSpriteTiles   = NULL;
static UINT8  *FstarfrcTempGfx       = NULL;

static INT32 CharScrollX;
static INT32 CharScrollY;
static INT32 Scroll1X;
static INT32 Scroll1Y;
static INT32 Scroll2X;
static INT32 Scroll2Y;

static INT32 Ginkun = 0;
static INT32 Riot = 0;

static INT32 FstarfrcSoundLatch;

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static struct BurnInputInfo FstarfrcInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , FstarfrcInputPort0 + 14, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , FstarfrcInputPort0 +  6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , FstarfrcInputPort0 + 15, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , FstarfrcInputPort0 +  7, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , FstarfrcInputPort0 +  3, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , FstarfrcInputPort0 +  2, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , FstarfrcInputPort0 +  1, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , FstarfrcInputPort0 +  0, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , FstarfrcInputPort0 +  4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , FstarfrcInputPort0 +  5, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL  , FstarfrcInputPort0 + 11, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , FstarfrcInputPort0 + 10, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , FstarfrcInputPort0 +  9, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , FstarfrcInputPort0 +  8, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , FstarfrcInputPort0 + 12, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , FstarfrcInputPort0 + 13, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &FstarfrcReset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, FstarfrcDip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, FstarfrcDip + 1        , "dip"       },
};

STDINPUTINFO(Fstarfrc)

static struct BurnInputInfo RiotInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , FstarfrcInputPort0 + 14, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , FstarfrcInputPort0 +  6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , FstarfrcInputPort0 + 15, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , FstarfrcInputPort0 +  7, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , FstarfrcInputPort0 +  3, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , FstarfrcInputPort0 +  2, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , FstarfrcInputPort0 +  1, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , FstarfrcInputPort0 +  0, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , FstarfrcInputPort1 +  1, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , FstarfrcInputPort0 +  4, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , FstarfrcInputPort0 +  5, "p1 fire 3" },

	{"P2 Up"             , BIT_DIGITAL  , FstarfrcInputPort0 + 11, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , FstarfrcInputPort0 + 10, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , FstarfrcInputPort0 +  9, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , FstarfrcInputPort0 +  8, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , FstarfrcInputPort1 +  5, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , FstarfrcInputPort0 + 12, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , FstarfrcInputPort0 + 13, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL  , &FstarfrcReset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, FstarfrcDip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, FstarfrcDip + 1        , "dip"       },
};

STDINPUTINFO(Riot)

inline void FstarfrcMakeInputs()
{
	// Reset Inputs
	FstarfrcInput[0] = FstarfrcInput[1] = 0x3fff;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 14; i++) {
		FstarfrcInput[0] -= (FstarfrcInputPort0[i] & 1) << i;
	}

	for (INT32 i = 14; i < 16; i++) {
		FstarfrcInput[0] |= (FstarfrcInputPort0[i] & 1) << i;
	}

	for (INT32 i = 0; i < 6; i++) {
		FstarfrcInput[1] -= (FstarfrcInputPort1[i] & 1) << i;
	}
}

static struct BurnDIPInfo FstarfrcDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Coin SW 1"              },
	{0x11, 0x01, 0x03, 0x03, "1 Coin 1 Credit"        },
	{0x11, 0x01, 0x03, 0x02, "1 Coin 2 Credits"       },
	{0x11, 0x01, 0x03, 0x01, "1 Coin 3 Credits"       },
	{0x11, 0x01, 0x03, 0x00, "1 Coin 4 Credits"       },

	{0   , 0xfe, 0   , 4   , "Coin SW 2"              },
	{0x11, 0x01, 0x0c, 0x00, "4 Coins 1 Credit"       },
	{0x11, 0x01, 0x0c, 0x04, "3 Coins 1 Credit"       },
	{0x11, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"       },
	{0x11, 0x01, 0x0c, 0x0c, "1 Coin 1 Credit"        },

	{0   , 0xfe, 0   , 2   , "Screen Reverse"         },
	{0x11, 0x01, 0x10, 0x10, "Off"                    },
	{0x11, 0x01, 0x10, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demonstration Sound"    },
	{0x11, 0x01, 0x20, 0x00, "Off"                    },
	{0x11, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Continue Play"          },
	{0x11, 0x01, 0x40, 0x00, "Off"                    },
	{0x11, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x11, 0x01, 0x80, 0x80, "Off"                    },
	{0x11, 0x01, 0x80, 0x00, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "The Number of Player"   },
	{0x12, 0x01, 0x03, 0x00, "2"                      },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x02, "4"                      },
	{0x12, 0x01, 0x03, 0x01, "5"                      },

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x12, 0x01, 0x0c, 0x08, "Easy"                   },
	{0x12, 0x01, 0x0c, 0x0c, "Medium"                 },
	{0x12, 0x01, 0x0c, 0x04, "Hard"                   },
	{0x12, 0x01, 0x0c, 0x00, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Level Up Speed"         },
	{0x12, 0x01, 0x30, 0x00, "Slowest"                },
	{0x12, 0x01, 0x30, 0x10, "Slow"                   },
	{0x12, 0x01, 0x30, 0x30, "Fast"                   },
	{0x12, 0x01, 0x30, 0x20, "Fastest"                },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x12, 0x01, 0xc0, 0xc0, "200,000, 1,000,000"     },
	{0x12, 0x01, 0xc0, 0x80, "220,000, 1,200,000"     },
	{0x12, 0x01, 0xc0, 0x40, "240,000, 1,400,000"     },
	{0x12, 0x01, 0xc0, 0x00, "Every 500,000, Once at Highest Score"},
};

STDDIPINFO(Fstarfrc)

static struct BurnDIPInfo GinkunDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Coin SW 1"              },
	{0x11, 0x01, 0x03, 0x03, "1 Coin 1 Credit"        },
	{0x11, 0x01, 0x03, 0x02, "1 Coin 2 Credits"       },
	{0x11, 0x01, 0x03, 0x01, "1 Coin 3 Credits"       },
	{0x11, 0x01, 0x03, 0x00, "1 Coin 4 Credits"       },

	{0   , 0xfe, 0   , 4   , "Coin SW 2"              },
	{0x11, 0x01, 0x0c, 0x00, "4 Coins 1 Credit"       },
	{0x11, 0x01, 0x0c, 0x04, "3 Coins 1 Credit"       },
	{0x11, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"       },
	{0x11, 0x01, 0x0c, 0x0c, "1 Coin 1 Credit"        },

	{0   , 0xfe, 0   , 2   , "Continue Plus 1up"      },
	{0x11, 0x01, 0x10, 0x10, "Off"                    },
	{0x11, 0x01, 0x10, 0x00, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x00, "2"                      },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x02, "4"                      },
	{0x12, 0x01, 0x03, 0x01, "5"                      },

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x12, 0x01, 0x0c, 0x08, "Easy"                   },
	{0x12, 0x01, 0x0c, 0x0c, "Normal"                 },
	{0x12, 0x01, 0x0c, 0x04, "Hard"                   },
	{0x12, 0x01, 0x0c, 0x00, "Hardest"                },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x12, 0x01, 0x10, 0x10, "Off"                    },
	{0x12, 0x01, 0x10, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x20, 0x00, "Off"                    },
	{0x12, 0x01, 0x20, 0x20, "On"                     },
};

STDDIPINFO(Ginkun)

static struct BurnDIPInfo RiotDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                     },
	{0x14, 0xff, 0xff, 0xfc, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x13, 0x01, 0x07, 0x00, "5 Coins 1 Credit"       },
	{0x13, 0x01, 0x07, 0x01, "4 Coins 1 Credit"       },
	{0x13, 0x01, 0x07, 0x02, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x07, 0x04, "2 Coins 1 Credit"       },
	{0x13, 0x01, 0x07, 0x07, "1 Coin 1 Credit"        },
	{0x13, 0x01, 0x07, 0x06, "1 Coin 2 Credits"       },
	{0x13, 0x01, 0x07, 0x05, "1 Coin 3 Credits"       },
	{0x13, 0x01, 0x07, 0x03, "1 Coin 4 Credits"       },

	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x13, 0x01, 0x38, 0x00, "5 Coins 1 Credit"       },
	{0x13, 0x01, 0x38, 0x08, "4 Coins 1 Credit"       },
	{0x13, 0x01, 0x38, 0x10, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x38, 0x20, "2 Coins 1 Credit"       },
	{0x13, 0x01, 0x38, 0x38, "1 Coin 1 Credit"        },
	{0x13, 0x01, 0x38, 0x30, "2 Coins 1 Credit"       },
	{0x13, 0x01, 0x38, 0x28, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x38, 0x18, "4 Coins 1 Credit"       },

	{0   , 0xfe, 0   , 2   , "Starting Coins"         },
	{0x13, 0x01, 0x40, 0x40, "1"                      },
	{0x13, 0x01, 0x40, 0x00, "2"                      },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x03, 0x03, "1"                      },
	{0x14, 0x01, 0x03, 0x02, "2"                      },
	{0x14, 0x01, 0x03, 0x01, "3"                      },
	{0x14, 0x01, 0x03, 0x00, "4"                      },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x14, 0x01, 0x10, 0x10, "Off"                    },
	{0x14, 0x01, 0x10, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x14, 0x01, 0x20, 0x00, "Off"                    },
	{0x14, 0x01, 0x20, 0x20, "On"                     },
};

STDDIPINFO(Riot)

static struct BurnRomInfo FstarfrcRomDesc[] = {
	{ "fstarf01.rom",  0x40000, 0x94c71de6, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "fstarf02.rom",  0x40000, 0xb1a07761, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "fstarf03.rom",  0x20000, 0x54375335, BRF_GRA },			 //  2
	{ "fstarf05.rom",  0x80000, 0x77a281e7, BRF_GRA },			 //  3
	{ "fstarf04.rom",  0x80000, 0x398a920d, BRF_GRA },			 //  4
	{ "fstarf09.rom",  0x80000, 0xd51341d2, BRF_GRA },			 //  5
	{ "fstarf06.rom",  0x80000, 0x07e40e87, BRF_GRA },			 //  6

	{ "fstarf07.rom",  0x10000, 0xe0ad5de1, BRF_PRG | BRF_SND }, //  7	Z80 Program Code

	{ "fstarf08.rom",  0x20000, 0xf0ad5693, BRF_SND },			 //  8	Samples
};


STD_ROM_PICK(Fstarfrc)
STD_ROM_FN(Fstarfrc)


static struct BurnRomInfo FstarfrcjRomDesc[] = {
	{ "1.bin",         0x40000, 0x1905d85d, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "2.bin",         0x40000, 0xde9cfc39, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "fstarf03.rom",  0x20000, 0x54375335, BRF_GRA },			 //  2
	{ "fstarf05.rom",  0x80000, 0x77a281e7, BRF_GRA },			 //  3
	{ "fstarf04.rom",  0x80000, 0x398a920d, BRF_GRA },			 //  4
	{ "fstarf09.rom",  0x80000, 0xd51341d2, BRF_GRA },			 //  5
	{ "fstarf06.rom",  0x80000, 0x07e40e87, BRF_GRA },			 //  6

	{ "fstarf07.rom",  0x10000, 0xe0ad5de1, BRF_PRG | BRF_SND }, //  7	Z80 Program Code

	{ "fstarf08.rom",  0x20000, 0xf0ad5693, BRF_SND },			 //  8	Samples
};


STD_ROM_PICK(Fstarfrcj)
STD_ROM_FN(Fstarfrcj)

static struct BurnRomInfo GinkunRomDesc[] = {
	{ "ginkun01.i01",  0x40000, 0x98946fd5, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ginkun02.i02",  0x40000, 0xe98757f6, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "ginkun03.i03",  0x20000, 0x4456e0df, BRF_GRA },			 //  2
	{ "ginkun05.i09",  0x80000, 0x1263bd42, BRF_GRA },			 //  3
	{ "ginkun04.i05",  0x80000, 0x9e4cf611, BRF_GRA },			 //  4
	{ "ginkun09.i22",  0x80000, 0x233384b9, BRF_GRA },			 //  5
	{ "ginkun06.i16",  0x80000, 0xf8589184, BRF_GRA },			 //  6

	{ "ginkun07.i17",  0x10000, 0x8836b1aa, BRF_PRG | BRF_SND },	//  7	Z80 Program Code

	{ "ginkun08.i18",  0x20000, 0x8b7583c7, BRF_SND },			 //  8	Samples
};


STD_ROM_PICK(Ginkun)
STD_ROM_FN(Ginkun)

static struct BurnRomInfo RiotRomDesc[] = {
	{ "1.ic1",  0x40000, 0x9ef4232e, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "2.ic2",  0x40000, 0xf2c6fbbf, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "3.ic3",  0x20000, 0xf60f5c96, BRF_GRA }, //  2
	{ "5.ic9",  0x80000, 0x056fce78, BRF_GRA }, //  3
	{ "4.ic5",  0x80000, 0x0894e7b4, BRF_GRA }, //  4
	{ "9.ic22", 0x80000, 0x0ead54f3, BRF_GRA }, //  5
	{ "6.ic16", 0x80000, 0x96ef61da, BRF_GRA }, //  6

	{ "7.ic17", 0x10000, 0x0a95b8f3, BRF_PRG | BRF_SND },	//  7	Z80 Program Code
	
	{ "8.ic18", 0x20000, 0x4b70e266, BRF_SND }, //  8	Samples
};


STD_ROM_PICK(Riot)
STD_ROM_FN(Riot)

INT32 FstarfrcDoReset()
{
	CharScrollX = CharScrollY = Scroll1X = Scroll1Y = Scroll2X = Scroll2Y = 0;

	FstarfrcSoundLatch = 0;

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	MSM6295Reset(0);
	BurnYM2151Reset();

	return 0;
}

void FstarfrcYM2151IrqHandler(INT32 Irq)
{
	if (Irq) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0   , ZET_IRQSTATUS_NONE);
	}
}

UINT16 __fastcall FstarfrcReadWord(UINT32 a)
{
	switch (a) {
		case 0x150030: {
			SEK_DEF_READ_WORD(0, a);
		}

		case 0x150040: {
			SEK_DEF_READ_WORD(0, a);
		}

		case 0x150050: {
			return FstarfrcInput[0];
		}
	}

	return 0;
}

UINT8 __fastcall FstarfrcReadByte(UINT32 a)
{
	switch (a) {
		case 0x150030:
		case 0x150031: {
			return FstarfrcDip[1];
		}
		case 0x150040:
		case 0x150041: {
			return FstarfrcDip[0];
		}
	}

	return 0;
}

void __fastcall FstarfrcWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x150010: {
			SEK_DEF_WRITE_WORD(0, a, d);
			return;
		}

		case 0x160000: {
			CharScrollX = d;
			return;
		}

		case 0x16000c: {
			Scroll1X = d;
			return;
		}

		case 0x160012: {
			Scroll1Y = d;
			return;
		}

		case 0x160018: {
			Scroll2X = d;
			return;
		}

		case 0x16001e: {
			Scroll2Y = d;
			return;
		}
	}
}

void __fastcall FstarfrcWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x150011: {
			FstarfrcSoundLatch = d & 0xff;
			ZetOpen(0);
			ZetNmi();
			ZetClose();
			return;
		}
	}
}

UINT16 __fastcall GinkunReadWord(UINT32 a)
{
	switch (a) {
		case 0x150020: {
			return FstarfrcInput[1];
		}

		case 0x150030: {
			SEK_DEF_READ_WORD(0, a);
		}

		case 0x150040: {
			SEK_DEF_READ_WORD(0, a);
		}

		case 0x150050: {
			return FstarfrcInput[0];
		}
	}
	
//	bprintf(PRINT_NORMAL, _T("Read Word -> %06X\n"), a);

	return 0;
}

UINT8 __fastcall GinkunReadByte(UINT32 a)
{
	switch (a) {
		case 0x150030:
		case 0x150031: {
			return FstarfrcDip[1];
		}
		case 0x150040:
		case 0x150041: {
			return FstarfrcDip[0];
		}
	}
	

//	bprintf(PRINT_NORMAL, _T("Read Byte -> %06X\n"), a);

	return 0;
}

void __fastcall GinkunWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
//		case 0x150010: {
//			SEK_DEF_WRITE_WORD(0, a, d);
//			return;
//		}

		case 0x150066: {
			return;
		}
		
		case 0x16002e: {
			return;
		}

		case 0x160000: {
			CharScrollX = d;
			return;
		}
		
		case 0x160006: {
			CharScrollY = d;
			return;
		}

		case 0x16000c: {
			Scroll1X = d;
			return;
		}

		case 0x160012: {
			Scroll1Y = d;
			return;
		}

		case 0x160018: {
			Scroll2X = d;
			return;
		}

		case 0x16001e: {
			Scroll2Y = d;
			return;
		}
	}
	
//	bprintf(PRINT_NORMAL, _T("Write Word -> %06X, %04X\n"), a, d);
}

void __fastcall GinkunWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x150001: {
			return;	// Flipscreen
		}

		case 0x150021: {
			return;	// NOP
		}
		
		case 0x150031: {
			return;	// NOP
		}

		case 0x150011: {
			FstarfrcSoundLatch = d & 0xff;
			ZetOpen(0);
			ZetNmi();
			ZetClose();
			return;
		}
	}
	
//	bprintf(PRINT_NORMAL, _T("Write Byte -> %06X, %02X\n"), a, d);
}

UINT8 __fastcall FstarfrcZ80Read(UINT16 a)
{
	switch (a) {
		case 0xfc00: {
			return MSM6295ReadStatus(0);
		}

		case 0xfc05: {
			return BurnYM2151ReadStatus();
		}

		case 0xfc08: {
			return FstarfrcSoundLatch;
		}
	}

	return 0;
}

void __fastcall FstarfrcZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xfc00: {
			MSM6295Command(0, d);
			return;
		}

		case 0xfc04: {
			BurnYM2151SelectRegister(d);
			return;
		}

		case 0xfc05: {
			BurnYM2151WriteRegister(d);
			return;
		}
	}
}

static INT32 FstarfrcMemIndex()
{
	UINT8 *Next; Next = Mem;

	FstarfrcRom          = Next; Next += 0x80000;
	FstarfrcZ80Rom       = Next; Next += 0x10000;
	MSM6295ROM           = Next; Next += 0x20000;

	RamStart = Next;

	if (Ginkun || Riot) {
	FstarfrcRam          = Next; Next += 0x05000;
	FstarfrcCharRam      = Next; Next += 0x01000;
	FstarfrcVideoRam     = Next; Next += 0x01000;
	FstarfrcColourRam    = Next; Next += 0x01000;
	FstarfrcVideo2Ram    = Next; Next += 0x01000;
	FstarfrcColour2Ram   = Next; Next += 0x01000;
	} else {
	FstarfrcRam          = Next; Next += 0x0a000;
	FstarfrcCharRam      = Next; Next += 0x01000;
	FstarfrcVideoRam     = Next; Next += 0x00800;
	FstarfrcColourRam    = Next; Next += 0x00800;
	FstarfrcVideo2Ram    = Next; Next += 0x00800;
	FstarfrcColour2Ram   = Next; Next += 0x00800;
	}
	FstarfrcSpriteRam    = Next; Next += 0x01000;
	FstarfrcPaletteRam   = Next; Next += 0x02000;
	FstarfrcZ80Ram       = Next; Next += 0x0c002;

	RamEnd = Next;

	FstarfrcCharTiles    = Next; Next += (4096 * 8 * 8);
	FstarfrcLayerTiles   = Next; Next += (8192 * 16 * 16);
	FstarfrcSpriteTiles  = Next; Next += (32768 * 8 * 8);
	FstarfrcPalette      = (UINT32*)Next; Next += 0x02000 * sizeof(UINT32);
	MemEnd = Next;

	return 0;
}

static INT32 TilePlaneOffsets[4] = { 0, 1, 2, 3 };
static INT32 TileXOffsets[16]    = { 0, 4, 8, 12, 16, 20, 24, 28, 256, 260, 264, 268, 272, 276, 280, 284 };
static INT32 TileYOffsets[16]    = { 0, 32, 64, 96, 128, 160, 192, 224, 512, 544, 576, 608, 640, 672, 704, 736 };
static INT32 CharPlaneOffsets[4] = { 0, 1, 2, 3 };
static INT32 CharXOffsets[8]     = { 0, 4, 8, 12, 16, 20, 24, 28 };
static INT32 CharYOffsets[8]     = { 0, 32, 64, 96, 128, 160, 192, 224 };
	

INT32 FstarfrcInit()
{
	INT32 nRet = 0, nLen;

	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "ginkun")) Ginkun = 1;

	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "riot")) Riot = 1;

	// Allocate and Blank all required memory
	Mem = NULL;
	FstarfrcMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	FstarfrcMemIndex();

	FstarfrcTempGfx = (UINT8*)BurnMalloc(0x100000);

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(FstarfrcRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(FstarfrcRom + 0x00000, 1, 2); if (nRet != 0) return 1;

	// Load and decode Char Tiles rom
	memset(FstarfrcTempGfx, 0, 0x100000);
	nRet = BurnLoadRom(FstarfrcTempGfx, 2, 1); if (nRet != 0) return 1;
	GfxDecode(4096, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, FstarfrcTempGfx, FstarfrcCharTiles);

	// Load, byteswap and decode Bg and Fg Layer roms
	memset(FstarfrcTempGfx, 0, 0x100000);
	nRet = BurnLoadRom(FstarfrcTempGfx + 0x000000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(FstarfrcTempGfx + 0x000001, 4, 2); if (nRet != 0) return 1;
	GfxDecode(8192, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, FstarfrcTempGfx, FstarfrcLayerTiles);

	// Load, byteswap and decode Sprite Tile roms
	memset(FstarfrcTempGfx, 0, 0x100000);
	nRet = BurnLoadRom(FstarfrcTempGfx + 0x000000, 5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(FstarfrcTempGfx + 0x000001, 6, 2); if (nRet != 0) return 1;
	GfxDecode(32768, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, FstarfrcTempGfx, FstarfrcSpriteTiles);

	BurnFree(FstarfrcTempGfx);

	// Load Z80 Program rom
	nRet = BurnLoadRom(FstarfrcZ80Rom, 7, 1); if (nRet != 0) return 1;

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 8, 1); if (nRet != 0) return 1;

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	if (!strncmp(BurnDrvGetTextA(DRV_NAME), "fstarfrc", 8)) {
	SekMapMemory(FstarfrcRom         , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(FstarfrcRam         , 0x100000, 0x103fff, SM_RAM);
	SekMapMemory(FstarfrcCharRam     , 0x110000, 0x110fff, SM_RAM);
	SekMapMemory(FstarfrcVideoRam    , 0x120000, 0x1207ff, SM_RAM);
	SekMapMemory(FstarfrcColourRam   , 0x120800, 0x120fff, SM_RAM);
	SekMapMemory(FstarfrcVideo2Ram   , 0x121000, 0x1217ff, SM_RAM);
	SekMapMemory(FstarfrcColour2Ram  , 0x121800, 0x121fff, SM_RAM);
	SekMapMemory(FstarfrcRam + 0x4000, 0x122000, 0x127fff, SM_RAM);
	SekMapMemory(FstarfrcSpriteRam   , 0x130000, 0x130fff, SM_RAM);
	SekMapMemory(FstarfrcPaletteRam  , 0x140000, 0x141fff, SM_RAM);
	SekSetReadWordHandler(0, FstarfrcReadWord);
	SekSetWriteWordHandler(0, FstarfrcWriteWord);
	SekSetReadByteHandler(0, FstarfrcReadByte);
	SekSetWriteByteHandler(0, FstarfrcWriteByte);
	} else {
	SekMapMemory(FstarfrcRom         , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(FstarfrcRam         , 0x100000, 0x103fff, SM_RAM);
	SekMapMemory(FstarfrcCharRam     , 0x110000, 0x110fff, SM_RAM);
	SekMapMemory(FstarfrcVideoRam    , 0x120000, 0x120fff, SM_RAM);
	SekMapMemory(FstarfrcColourRam   , 0x121000, 0x121fff, SM_RAM);
	SekMapMemory(FstarfrcVideo2Ram   , 0x122000, 0x122fff, SM_RAM);
	SekMapMemory(FstarfrcColour2Ram  , 0x123000, 0x123fff, SM_RAM);
	SekMapMemory(FstarfrcRam + 0x4000, 0x124000, 0x124fff, SM_RAM);
	SekMapMemory(FstarfrcSpriteRam   , 0x130000, 0x130fff, SM_RAM);
	SekMapMemory(FstarfrcPaletteRam  , 0x140000, 0x141fff, SM_RAM);
	SekSetReadWordHandler(0, GinkunReadWord);
	SekSetWriteWordHandler(0, GinkunWriteWord);
	SekSetReadByteHandler(0, GinkunReadByte);
	SekSetWriteByteHandler(0, GinkunWriteByte);
	}
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xefff, 0, FstarfrcZ80Rom         );
	ZetMapArea(0x0000, 0xefff, 2, FstarfrcZ80Rom         );
	ZetMapArea(0xf000, 0xfbff, 0, FstarfrcZ80Ram         );
	ZetMapArea(0xf000, 0xfbff, 1, FstarfrcZ80Ram         );
	ZetMapArea(0xf000, 0xfbff, 2, FstarfrcZ80Ram         );
	ZetMapArea(0xfffe, 0xffff, 0, FstarfrcZ80Ram + 0xc000);
	ZetMapArea(0xfffe, 0xffff, 1, FstarfrcZ80Ram + 0xc000);
	ZetMapArea(0xfffe, 0xffff, 2, FstarfrcZ80Ram + 0xc000);
	ZetMemEnd();
	ZetSetReadHandler(FstarfrcZ80Read);
	ZetSetWriteHandler(FstarfrcZ80Write);
	ZetClose();

	// Setup the YM2151 emulation
	BurnYM2151Init(8000000 / 2);
	BurnYM2151SetIrqHandler(&FstarfrcYM2151IrqHandler);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.60, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.60, BURN_SND_ROUTE_RIGHT);

	// Setup the OKIM6295 emulation
	MSM6295Init(0, 7575, 1);
	MSM6295SetRoute(0, 0.40, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	// Reset the driver
	FstarfrcDoReset();

	return 0;
}

INT32 FstarfrcExit()
{
	BurnYM2151Exit();
	MSM6295Exit(0);

	SekExit();
	ZetExit();

	GenericTilesExit();

	BurnFree(Mem);
	
	Ginkun = 0;
	Riot = 0;

	return 0;
}

void GinkunRenderBgLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = ((FstarfrcVideo2Ram[TileIndex + 1] << 8) | FstarfrcVideo2Ram[TileIndex + 0]) & 0x1fff;
			Colour = (((FstarfrcColour2Ram[TileIndex + 1] << 8) | FstarfrcColour2Ram[TileIndex + 0]) & 0x0f);

			x = 16 * mx;
			y = 16 * my;

			x -= Scroll2X;
			y -= Scroll2Y;

			x &= 0x1ff;
			y &= 0x1ff;

			if (x >= 496) x -= 512;
			if (y >= 496) y -= 512;

			y -= 16;

			if (x > 15 && x < 240 && y > 15 && y < 208) {
				Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 512 + (0x10 << 4), FstarfrcLayerTiles);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 512 + (0x10 << 4), FstarfrcLayerTiles);
			}

			TileIndex += 2;
		}
	}
}

void FstarfrcRenderBgLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Code = ((FstarfrcVideo2Ram[TileIndex + 1] << 8) | FstarfrcVideo2Ram[TileIndex + 0]) & 0x1fff;
			Colour = (((FstarfrcColour2Ram[TileIndex + 1] << 8) | FstarfrcColour2Ram[TileIndex + 0]) & 0x0f);

			x = 16 * mx;
			y = 16 * my;

			x -= Scroll2X;
			y -= Scroll2Y;

			x &= 0x1ff;
			y &= 0x1ff;

			if (x >= 496) x -= 512;
			if (y >= 496) y -= 512;

			y -= 16;

			if (x > 15 && x < 240 && y > 15 && y < 208) {
				Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 512 + (0x10 << 4), FstarfrcLayerTiles);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 512 + (0x10 << 4), FstarfrcLayerTiles);
			}

			TileIndex += 2;
		}
	}
}

void GinkunRenderFgLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = ((FstarfrcVideoRam[TileIndex + 1] << 8) | FstarfrcVideoRam[TileIndex + 0]) & 0x1fff;
			Colour = ((FstarfrcColourRam[TileIndex + 1] << 8) | FstarfrcColourRam[TileIndex + 0]) & 0x0f;

			x = 16 * mx;
			y = 16 * my;

			x -= Scroll1X;
			y -= Scroll1Y;

			x &= 0x1ff;
			y &= 0x1ff;

			if (x >= 496) x -= 512;
			if (y >= 496) y -= 512;

			y -= 16;

			if (x > 15 && x < 240 && y > 15 && y < 208) {
				Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 512, FstarfrcLayerTiles);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 512, FstarfrcLayerTiles);
			}

			TileIndex += 2 ;
		}
	}
}

void FstarfrcRenderFgLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Code = ((FstarfrcVideoRam[TileIndex + 1] << 8) | FstarfrcVideoRam[TileIndex + 0]) & 0x1fff;
			Colour = ((FstarfrcColourRam[TileIndex + 1] << 8) | FstarfrcColourRam[TileIndex + 0]) & 0x0f;

			x = 16 * mx;
			y = 16 * my;

			x -= Scroll1X;
			y -= Scroll1Y;

			x &= 0x1ff;
			y &= 0x1ff;

			if (x >= 496) x -= 512;
			if (y >= 496) y -= 512;

			y -= 16;

			if (x > 15 && x < 240 && y > 15 && y < 208) {
				Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 512, FstarfrcLayerTiles);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 512, FstarfrcLayerTiles);
			}

			TileIndex += 2 ;
		}
	}
}

void FstarfrcRenderTextLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = (FstarfrcCharRam[TileIndex + 1] << 8) | FstarfrcCharRam[TileIndex + 0];
			Colour = Code >> 12;
			Code &= 0x0fff;

			x = 8 * mx;
			y = 8 * my;

			x -= CharScrollX;
			y -= CharScrollY;

			x &= 0x1ff;
			y &= 0x0ff;

			if (x > 7 && x < 248 && y > 7 && y < 216) {
				Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 256, FstarfrcCharTiles);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 256, FstarfrcCharTiles);
			}

			TileIndex += 2 ;
		}
	}
}

static void draw_sprites(INT32 layerpriority)
{
	UINT16* pFstarfrcSpriteRam = ((UINT16*)FstarfrcSpriteRam);


	INT32 offs;
	const UINT8 layout[8][8] =
	{
		{0,1,4,5,16,17,20,21},
		{2,3,6,7,18,19,22,23},
		{8,9,12,13,24,25,28,29},
		{10,11,14,15,26,27,30,31},
		{32,33,36,37,48,49,52,53},
		{34,35,38,39,50,51,54,55},
		{40,41,44,45,56,57,60,61},
		{42,43,46,47,58,59,62,63}
	};

	for (offs = 0; offs < 0x1000 / 2; offs += 8)
	{
		if (BURN_ENDIAN_SWAP_INT16(pFstarfrcSpriteRam[offs]) & 0x04)	/* enable */
		{
			INT32 code,color,sizex,sizey,flipx,flipy,xpos,ypos;
			INT32 x,y,priority;

			code = BURN_ENDIAN_SWAP_INT16(pFstarfrcSpriteRam[offs+1]);
			color = (BURN_ENDIAN_SWAP_INT16(pFstarfrcSpriteRam[offs+2]) & 0xf0) >> 4;
			sizex = 1 << ((BURN_ENDIAN_SWAP_INT16(pFstarfrcSpriteRam[offs+2]) & 0x03) >> 0);
			if(Riot)
				sizey = sizex;
			else
			sizey = 1 << ((BURN_ENDIAN_SWAP_INT16(pFstarfrcSpriteRam[offs+2]) & 0x0c) >> 2);
			if (sizex >= 2) code &= ~0x01;
			if (sizey >= 2) code &= ~0x02;
			if (sizex >= 4) code &= ~0x04;
			if (sizey >= 4) code &= ~0x08;
			if (sizex >= 8) code &= ~0x10;
			if (sizey >= 8) code &= ~0x20;
			flipx = BURN_ENDIAN_SWAP_INT16(pFstarfrcSpriteRam[offs]) & 0x01;
			flipy = BURN_ENDIAN_SWAP_INT16(pFstarfrcSpriteRam[offs]) & 0x02;
			xpos = BURN_ENDIAN_SWAP_INT16(pFstarfrcSpriteRam[offs+4]);
			if (xpos >= 0x8000) xpos -= 0x10000;
			ypos = BURN_ENDIAN_SWAP_INT16(pFstarfrcSpriteRam[offs+3]);
			if (ypos >= 0x8000) ypos -= 0x10000;
			priority = (BURN_ENDIAN_SWAP_INT16(pFstarfrcSpriteRam[offs]) & 0xc0) >> 6;

			if (priority != layerpriority) continue;

			for (y = 0;y < sizey;y++)
			{
				for (x = 0;x < sizex;x++)
				{
					INT32 sx = xpos + 8*(flipx?(sizex-1-x):x);
					INT32 sy = ypos + 8*(flipy?(sizey-1-y):y);

					if ((code + layout[y][x]) > 32767) break;

					sy -= 16;

					if (sx > 7 && sx < 248 && sy > 7 && sy < 216) {
						if (!flipx) {
							if (!flipy) {
								Render8x8Tile_Mask(pTransDraw, code + layout[y][x], sx, sy, color, 4, 0, 0, FstarfrcSpriteTiles);
							} else {
								Render8x8Tile_Mask_FlipY(pTransDraw, code + layout[y][x], sx, sy, color, 4, 0, 0, FstarfrcSpriteTiles);
							}
						} else {
							if (!flipy) {
								Render8x8Tile_Mask_FlipX(pTransDraw, code + layout[y][x], sx, sy, color, 4, 0, 0, FstarfrcSpriteTiles);
							} else {
								Render8x8Tile_Mask_FlipXY(pTransDraw, code + layout[y][x], sx, sy, color, 4, 0, 0, FstarfrcSpriteTiles);
							}
						}
					} else {
						if (!flipx) {
							if (!flipy) {
								Render8x8Tile_Mask_Clip(pTransDraw, code + layout[y][x], sx, sy, color, 4, 0, 0, FstarfrcSpriteTiles);
							} else {
								Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code + layout[y][x], sx, sy, color, 4, 0, 0, FstarfrcSpriteTiles);
							}
						} else {
							if (!flipy) {
								Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code + layout[y][x], sx, sy, color, 4, 0, 0, FstarfrcSpriteTiles);
							} else {
								Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code + layout[y][x], sx, sy, color, 4, 0, 0, FstarfrcSpriteTiles);
							}
						}
					}
				}
			}
		}
	}
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour >> 0) & 0x0f;
	g = (nColour >> 4) & 0x0f;
	b = (nColour >> 8) & 0x0f;

	r = (r << 4) | r;
	g = (g << 4) | g;
	b = (b << 4) | b;

	return BurnHighCol(r, g, b, 0);
}

INT32 FstarfrcCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)FstarfrcPaletteRam, pd = FstarfrcPalette; i < 0x2000; i++, ps++, pd++) {
		*pd = CalcCol(BURN_ENDIAN_SWAP_INT16(*ps));
	}

	return 0;
}

void GinkunRender()
{
	FstarfrcCalcPalette();
	
	for (INT32 i = 0; i < nScreenHeight * nScreenWidth; i++) {
		pTransDraw[i] = 0x300;
	}
	
	draw_sprites(3);
	GinkunRenderBgLayer();
	draw_sprites(2);
	GinkunRenderFgLayer();
	draw_sprites(1);
	FstarfrcRenderTextLayer();
	draw_sprites(0);
	BurnTransferCopy(FstarfrcPalette);
}

void FstarfrcRender()
{
	FstarfrcCalcPalette();
	
	for (INT32 i = 0; i < nScreenHeight * nScreenWidth; i++) {
		pTransDraw[i] = 0x300;
	}
	
	draw_sprites(3);
	FstarfrcRenderBgLayer();
	draw_sprites(2);
	FstarfrcRenderFgLayer();
	draw_sprites(1);
	FstarfrcRenderTextLayer();
	draw_sprites(0);
	BurnTransferCopy(FstarfrcPalette);
}

INT32 FstarfrcFrame()
{
	INT32 nInterleave = 10;

	if (FstarfrcReset) FstarfrcDoReset();

	FstarfrcMakeInputs();

	nCyclesTotal[0] = (24000000 / 2) / 60;
	nCyclesTotal[1] = (8000000 / 2) / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	INT32 nSoundBufferPos = 0;

	SekNewFrame();

	SekOpen(0);
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);

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

	SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
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

	if (pBurnDraw) {
		if (Ginkun || Riot) {
			GinkunRender();
		} else {
			FstarfrcRender();
		}
	}

	return 0;
}

static INT32 FstarfrcScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin = 0x02944;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);					// Scan 68000
		ZetScan(nAction);					// Scan Z80

		MSM6295Scan(0, nAction);			// Scan OKIM6295
		BurnYM2151Scan(nAction);

		// Scan critical driver variables
		SCAN_VAR(FstarfrcSoundLatch);
		SCAN_VAR(FstarfrcInput);
		SCAN_VAR(FstarfrcDip);
		SCAN_VAR(CharScrollX);
		SCAN_VAR(CharScrollY);
		SCAN_VAR(Scroll1X);
		SCAN_VAR(Scroll1Y);
		SCAN_VAR(Scroll2X);
		SCAN_VAR(Scroll2Y);
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
	}

	return 0;
}

struct BurnDriver BurnDrvFstarfrc = {
	"fstarfrc", NULL, NULL, NULL, "1992",
	"Final Star Force (US)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_POST90S, GBF_VERSHOOT, 0,
	NULL, FstarfrcRomInfo, FstarfrcRomName, NULL, NULL, FstarfrcInputInfo, FstarfrcDIPInfo,
	FstarfrcInit, FstarfrcExit, FstarfrcFrame, NULL, FstarfrcScan,
	NULL, 0x2000, 224, 256, 3, 4
};

struct BurnDriver BurnDrvFstarfrcj = {
	"fstarfrcj", "fstarfrc", NULL, NULL, "1992",
	"Final Star Force (Japan)\0", NULL, "Tecmo", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_POST90S, GBF_VERSHOOT, 0,
	NULL, FstarfrcjRomInfo, FstarfrcjRomName, NULL, NULL, FstarfrcInputInfo, FstarfrcDIPInfo,
	FstarfrcInit, FstarfrcExit, FstarfrcFrame, NULL, FstarfrcScan,
	NULL, 0x2000, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGinkun = {
	"ginkun", NULL, NULL, NULL, "1995",
	"Ganbare Ginkun\0", "Imperfect GFX", "Tecmo", "Miscellaneous",
	L"\u304C\u3093\u3070\u308C \u30AE\u30F3\u304F\u3093\0Ganbare Ginkun\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, GinkunRomInfo, GinkunRomName, NULL, NULL, FstarfrcInputInfo, GinkunDIPInfo,
	FstarfrcInit, FstarfrcExit, FstarfrcFrame, NULL, FstarfrcScan,
	NULL, 0x2000, 256, 224, 4, 3
};

struct BurnDriver BurnDrvRiot = {
	"riot", NULL, NULL, NULL, "1992",
	"Riot\0", NULL, "NMK", "Miscellaneous",
	L"\u96F7\u8ECB\u6597 Riot\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_SHOOT, 0,
	NULL, RiotRomInfo, RiotRomName, NULL, NULL, RiotInputInfo, RiotDIPInfo,
	FstarfrcInit, FstarfrcExit, FstarfrcFrame, NULL, FstarfrcScan,
	NULL, 0x2000, 256, 224, 4, 3
};
