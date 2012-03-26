#include "tiles_generic.h"
#include "sek.h"
#include "zet.h"
#include "burn_ym3526.h"
#include "burn_ym2203.h"
#include "dac.h"

static UINT8 DrvInputPort0[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[3]            = {0, 0, 0};
static UINT8 DrvInput[3]          = {0x00, 0x00, 0x00};
static UINT8 DrvReset             = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *Drv68KRom           = NULL;
static UINT8 *DrvZ80Rom           = NULL;
static UINT8 *Drv68KRam           = NULL;
static UINT8 *DrvZ80Ram           = NULL;
static UINT8 *DrvProms            = NULL;
static UINT8 *DrvSpritePalBank    = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvBgVideoRam       = NULL;
static UINT8 *DrvFgVideoRam       = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvTiles            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette         = NULL;

static UINT8 DrvRecalcPal         = 0;
static UINT16 DrvScrollX          = 0;
static UINT16 DrvScrollY          = 0;
static UINT16 DrvDisableBg        = 0;
static UINT16 DrvFlipScreen       = 0;
static UINT8 DrvSoundLatch        = 0;

static UINT8 AmazonProtCmd;
static UINT8 AmazonProtReg[6];

static UINT8 DrvUseYM2203         = 0;
static UINT8 DrvIsHorekid         = 0;
static UINT16 *AmazonProtDataPtr  = NULL;

typedef INT32 (*TerracreLoadRomsType)();
static TerracreLoadRomsType LoadRomsFunction;

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },	
	
	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort2 + 4, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Drv)

static struct BurnInputInfo AmazonInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort2 + 1, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },	
	
	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort2 + 4, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Amazon)

static struct BurnInputInfo HorekidInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },	
	
	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort2 + 4, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Horekid)

static inline void DrvMakeInputs()
{
	DrvInput[0] = DrvInput[1] = 0xff;
	DrvInput[2] = 0xdf;

	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] -= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] -= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] -= (DrvInputPort2[i] & 1) << i;
	}
}

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0x20, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x03, 0x03, "3"                      },
	{0x13, 0x01, 0x03, 0x02, "4"                      },
	{0x13, 0x01, 0x03, 0x01, "5"                      },
	{0x13, 0x01, 0x03, 0x00, "6"                      },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x0c, 0x0c, "20k then every 60k"     },
	{0x13, 0x01, 0x0c, 0x08, "30k then every 70k"     },
	{0x13, 0x01, 0x0c, 0x04, "40k then every 80k"     },
	{0x13, 0x01, 0x0c, 0x00, "50k then every 90k"     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x10, 0x00, "Off"                    },
	{0x13, 0x01, 0x10, 0x10, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x20, 0x00, "Upright"                },
	{0x13, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x14, 0x01, 0x03, 0x01, "2 Coins 1 Credit"       },	
	{0x14, 0x01, 0x03, 0x03, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	{0x14, 0x01, 0x03, 0x00, "Free Play"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x14, 0x01, 0x0c, 0x00, "3 Coins 1 Credit"       },
	{0x14, 0x01, 0x0c, 0x04, "2 Coins 3 Credits"      },
	{0x14, 0x01, 0x0c, 0x0c, "1 Coin  3 Credits"      },
	{0x14, 0x01, 0x0c, 0x08, "1 Coin  6 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x14, 0x01, 0x10, 0x10, "Easy"                   },
	{0x14, 0x01, 0x10, 0x00, "Hard"                   },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x14, 0x01, 0x20, 0x20, "Off"                    },
	{0x14, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Complete Invulnerability"},
	{0x14, 0x01, 0x40, 0x40, "Off"                    },
	{0x14, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Base Ship Invulnerability"},
	{0x14, 0x01, 0x80, 0x80, "Off"                    },
	{0x14, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo AmazonDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0x20, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x03, 0x03, "3"                      },
	{0x13, 0x01, 0x03, 0x02, "4"                      },
	{0x13, 0x01, 0x03, 0x01, "5"                      },
	{0x13, 0x01, 0x03, 0x00, "6"                      },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x0c, 0x0c, "20k then every 40k"     },
	{0x13, 0x01, 0x0c, 0x08, "50k then every 40k"     },
	{0x13, 0x01, 0x0c, 0x04, "20k then every 70k"     },
	{0x13, 0x01, 0x0c, 0x00, "50k then every 70k"     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x10, 0x00, "Off"                    },
	{0x13, 0x01, 0x10, 0x10, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x20, 0x00, "Upright"                },
	{0x13, 0x01, 0x20, 0x20, "Cocktail"               },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x14, 0x01, 0x03, 0x01, "2 Coins 1 Credit"       },	
	{0x14, 0x01, 0x03, 0x03, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	{0x14, 0x01, 0x03, 0x00, "Free Play"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x14, 0x01, 0x0c, 0x00, "3 Coins 1 Credit"       },
	{0x14, 0x01, 0x0c, 0x08, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x0c, 0x04, "2 Coins 3 Credits"      },
	{0x14, 0x01, 0x0c, 0x0c, "1 Coin  3 Credits"      },
		
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x14, 0x01, 0x10, 0x10, "Easy"                   },
	{0x14, 0x01, 0x10, 0x00, "Hard"                   },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x14, 0x01, 0x20, 0x20, "Off"                    },
	{0x14, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Level"                  },
	{0x14, 0x01, 0x40, 0x40, "Low"                    },
	{0x14, 0x01, 0x40, 0x00, "High"                   },
	
	{0   , 0xfe, 0   , 2   , "Sprite Test"            },
	{0x14, 0x01, 0x80, 0x80, "Off"                    },
	{0x14, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Amazon)

static struct BurnDIPInfo HorekidDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0x20, NULL                     },
	{0x13, 0xff, 0xff, 0xef, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x03, 0x03, "3"                      },
	{0x13, 0x01, 0x03, 0x02, "4"                      },
	{0x13, 0x01, 0x03, 0x01, "5"                      },
	{0x13, 0x01, 0x03, 0x00, "6"                      },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x0c, 0x0c, "20k then every 60k"     },
	{0x13, 0x01, 0x0c, 0x08, "50k then every 60k"     },
	{0x13, 0x01, 0x0c, 0x04, "20k then every 90k"     },
	{0x13, 0x01, 0x0c, 0x00, "50k then every 90k"     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x10, 0x10, "Off"                    },
	{0x13, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x20, 0x00, "Upright"                },
	{0x13, 0x01, 0x20, 0x20, "Cocktail"               },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0xc0, 0xc0, "Easy"                   },
	{0x13, 0x01, 0xc0, 0x80, "Normal"                 },
	{0x13, 0x01, 0xc0, 0x40, "Hard"                   },
	{0x13, 0x01, 0xc0, 0x00, "Hardest"                },
	
	// Dip 3
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x14, 0x01, 0x03, 0x01, "2 Coins 1 Credit"       },	
	{0x14, 0x01, 0x03, 0x03, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	{0x14, 0x01, 0x03, 0x00, "Free Play"              },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x14, 0x01, 0x0c, 0x04, "2 Coins 1 Credit"       },
	{0x14, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x0c, 0x00, "2 Coins 3 Credits"      },
	{0x14, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"      },	
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x14, 0x01, 0x20, 0x20, "Off"                    },
	{0x14, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Debug Mode"             },
	{0x14, 0x01, 0xc0, 0xc0, "Off"                    },
	{0x14, 0x01, 0xc0, 0x80, "On"                     },
};

STDDIPINFO(Horekid)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "bk1_1.4b",      0x08000, 0x60932770, BRF_ESS | BRF_PRG },	//  0	68000 Program Code
	{ "bk1_3.4d",      0x08000, 0xcb36240e, BRF_ESS | BRF_PRG },	//  1
	{ "bk1_2.6b",      0x08000, 0x539352f2, BRF_ESS | BRF_PRG },	//  2
	{ "bk1_4.6d",      0x08000, 0x19387586, BRF_ESS | BRF_PRG },	//  3
	
	{ "bk2_11.15b",    0x04000, 0x604c3b11, BRF_ESS | BRF_PRG },	//  4	Z80 Program Code
	{ "bk2_12.17b",    0x04000, 0xaffc898d, BRF_ESS | BRF_PRG },	//  5
	{ "bk2_13.18b",    0x04000, 0x302dc0ab, BRF_ESS | BRF_PRG },	//  6
	
	{ "bk2_14.16g",    0x02000, 0x591a3804, BRF_GRA },				//  7	Chars
	
	{ "bk1_5.15f",     0x08000, 0x984a597f, BRF_GRA },				//  8	Tiles
	{ "bk1_6.17f",     0x08000, 0x30e297ff, BRF_GRA },				//  9
	
	{ "bk2_7.6e",      0x04000, 0xbcf7740b, BRF_GRA },				// 10	Sprites
	{ "bk2_8.7e",      0x04000, 0xa70b565c, BRF_GRA },				// 11
	{ "bk2_9.6g",      0x04000, 0x4a9ec3e6, BRF_GRA },				// 12
	{ "bk2_10.7g",     0x04000, 0x450749fc, BRF_GRA },				// 13
	
	{ "bk1_3.10f",     0x00100, 0xce07c544, BRF_GRA },				// 14	PROMs
	{ "bk1_2.11f",     0x00100, 0x566d323a, BRF_GRA },				// 15
	{ "bk1_1.12f",     0x00100, 0x7ea63946, BRF_GRA },				// 16
	{ "bk2_4.2g",      0x00100, 0x08609bad, BRF_GRA },				// 17
	
	{ "bk2_5.4e",      0x00100, 0x2c43991f, BRF_GRA },				// 18	Sprite Palette Bank
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo DrvoRomDesc[] = {
	{ "1a_4b.rom",     0x04000, 0x76f17479, BRF_ESS | BRF_PRG },	//  0	68000 Program Code
	{ "1a_4d.rom",     0x04000, 0x8119f06e, BRF_ESS | BRF_PRG },	//  1
	{ "1a_6b.rom",     0x04000, 0xba4b5822, BRF_ESS | BRF_PRG },	//  2
	{ "1a_6d.rom",     0x04000, 0xca4852f6, BRF_ESS | BRF_PRG },	//  3
	{ "1a_7b.rom",     0x04000, 0xd0771bba, BRF_ESS | BRF_PRG },	//  4
	{ "1a_7d.rom",     0x04000, 0x029d59d9, BRF_ESS | BRF_PRG },	//  5
	{ "1a_9b.rom",     0x04000, 0x69227b56, BRF_ESS | BRF_PRG },	//  6
	{ "1a_9d.rom",     0x04000, 0x5a672942, BRF_ESS | BRF_PRG },	//  7
	
	{ "2a_15b.rom",    0x04000, 0x604c3b11, BRF_ESS | BRF_PRG },	//  8	Z80 Program Code
	{ "2a_17b.rom",    0x04000, 0xaffc898d, BRF_ESS | BRF_PRG },	//  9
	{ "2a_18b.rom",    0x04000, 0x302dc0ab, BRF_ESS | BRF_PRG },	// 10
	
	{ "2a_16b.rom",    0x02000, 0x591a3804, BRF_GRA },				// 11	Chars
	
	{ "1a_15f.rom",    0x08000, 0x984a597f, BRF_GRA },				// 12	Tiles
	{ "1a_17f.rom",    0x08000, 0x30e297ff, BRF_GRA },				// 13
	
	{ "2a_6e.rom",     0x04000, 0xbcf7740b, BRF_GRA },				// 14	Sprites
	{ "2a_7e.rom",     0x04000, 0xa70b565c, BRF_GRA },				// 15
	{ "2a_6g.rom",     0x04000, 0x4a9ec3e6, BRF_GRA },				// 16
	{ "2a_7g.rom",     0x04000, 0x450749fc, BRF_GRA },				// 17
	
	{ "tc1a_10f.bin",  0x00100, 0xce07c544, BRF_GRA },				// 18	PROMs
	{ "tc1a_11f.bin",  0x00100, 0x566d323a, BRF_GRA },				// 19
	{ "tc1a_12f.bin",  0x00100, 0x7ea63946, BRF_GRA },				// 20
	{ "tc2a_2g.bin",   0x00100, 0x08609bad, BRF_GRA },				// 21
	
	{ "tc2a_4e.bin",   0x00100, 0x2c43991f, BRF_GRA },				// 22	Sprite Palette Bank
};

STD_ROM_PICK(Drvo)
STD_ROM_FN(Drvo)

static struct BurnRomInfo DrvaRomDesc[] = {
	{ "1a_4b.rom",     0x04000, 0x76f17479, BRF_ESS | BRF_PRG },	//  0	68000 Program Code
	{ "1a_4d.rom",     0x04000, 0x8119f06e, BRF_ESS | BRF_PRG },	//  1
	{ "1a_6b.rom",     0x04000, 0xba4b5822, BRF_ESS | BRF_PRG },	//  2
	{ "1a_6d.rom",     0x04000, 0xca4852f6, BRF_ESS | BRF_PRG },	//  3
	{ "1a_7b.rom",     0x04000, 0xd0771bba, BRF_ESS | BRF_PRG },	//  4
	{ "1a_7d.rom",     0x04000, 0x029d59d9, BRF_ESS | BRF_PRG },	//  5
	{ "1a_9b.rom",     0x04000, 0x69227b56, BRF_ESS | BRF_PRG },	//  6
	{ "1a_9d.rom",     0x04000, 0x5a672942, BRF_ESS | BRF_PRG },	//  7
	
	{ "2a_15b.rom",    0x04000, 0x604c3b11, BRF_ESS | BRF_PRG },	//  8	Z80 Program Code
	{ "dg.12",         0x04000, 0x9e9b3808, BRF_ESS | BRF_PRG },	//  9
	{ "2a_18b.rom",    0x04000, 0x302dc0ab, BRF_ESS | BRF_PRG },	// 10
	
	{ "2a_16b.rom",    0x02000, 0x591a3804, BRF_GRA },				// 11	Chars
	
	{ "1a_15f.rom",    0x08000, 0x984a597f, BRF_GRA },				// 12	Tiles
	{ "1a_17f.rom",    0x08000, 0x30e297ff, BRF_GRA },				// 13
	
	{ "2a_6e.rom",     0x04000, 0xbcf7740b, BRF_GRA },				// 14	Sprites
	{ "2a_7e.rom",     0x04000, 0xa70b565c, BRF_GRA },				// 15
	{ "2a_6g.rom",     0x04000, 0x4a9ec3e6, BRF_GRA },				// 16
	{ "2a_7g.rom",     0x04000, 0x450749fc, BRF_GRA },				// 17
	
	{ "tc1a_10f.bin",  0x00100, 0xce07c544, BRF_GRA },				// 18	PROMs
	{ "tc1a_11f.bin",  0x00100, 0x566d323a, BRF_GRA },				// 19
	{ "tc1a_12f.bin",  0x00100, 0x7ea63946, BRF_GRA },				// 20
	{ "tc2a_2g.bin",   0x00100, 0x08609bad, BRF_GRA },				// 21
	
	{ "tc2a_4e.bin",   0x00100, 0x2c43991f, BRF_GRA },				// 22	Sprite Palette Bank
};

STD_ROM_PICK(Drva)
STD_ROM_FN(Drva)

static struct BurnRomInfo DrvnRomDesc[] = {
	{ "1a_4b.rom",     0x04000, 0x76f17479, BRF_ESS | BRF_PRG },	//  0	68000 Program Code
	{ "1a_4d.rom",     0x04000, 0x8119f06e, BRF_ESS | BRF_PRG },	//  1
	{ "1a_6b.rom",     0x04000, 0xba4b5822, BRF_ESS | BRF_PRG },	//  2
	{ "1a_6d.rom",     0x04000, 0xca4852f6, BRF_ESS | BRF_PRG },	//  3
	{ "1a_7b.rom",     0x04000, 0xd0771bba, BRF_ESS | BRF_PRG },	//  4
	{ "1a_7d.rom",     0x04000, 0x029d59d9, BRF_ESS | BRF_PRG },	//  5
	{ "1a_9b.rom",     0x04000, 0x69227b56, BRF_ESS | BRF_PRG },	//  6
	{ "1a_9d.rom",     0x04000, 0x5a672942, BRF_ESS | BRF_PRG },	//  7
	
	{ "tc2a_15b.bin",  0x04000, 0x790ddfa9, BRF_ESS | BRF_PRG },	//  8	Z80 Program Code
	{ "tc2a_17b.bin",  0x04000, 0xd4531113, BRF_ESS | BRF_PRG },	//  9
	
	{ "2a_16b.rom",    0x02000, 0x591a3804, BRF_GRA },				// 10	Chars
	
	{ "1a_15f.rom",    0x08000, 0x984a597f, BRF_GRA },				// 11	Tiles
	{ "1a_17f.rom",    0x08000, 0x30e297ff, BRF_GRA },				// 12
	
	{ "2a_6e.rom",     0x04000, 0xbcf7740b, BRF_GRA },				// 13	Sprites
	{ "2a_7e.rom",     0x04000, 0xa70b565c, BRF_GRA },				// 14
	{ "2a_6g.rom",     0x04000, 0x4a9ec3e6, BRF_GRA },				// 15
	{ "2a_7g.rom",     0x04000, 0x450749fc, BRF_GRA },				// 16
	
	{ "tc1a_10f.bin",  0x00100, 0xce07c544, BRF_GRA },				// 17	PROMs
	{ "tc1a_11f.bin",  0x00100, 0x566d323a, BRF_GRA },				// 18
	{ "tc1a_12f.bin",  0x00100, 0x7ea63946, BRF_GRA },				// 19
	{ "tc2a_2g.bin",   0x00100, 0x08609bad, BRF_GRA },				// 20
	
	{ "tc2a_4e.bin",   0x00100, 0x2c43991f, BRF_GRA },				// 21	Sprite Palette Bank
};

STD_ROM_PICK(Drvn)
STD_ROM_FN(Drvn)

static struct BurnRomInfo AmazonRomDesc[] = {
	{ "11.4d",         0x08000, 0x6c7f85c5, BRF_ESS | BRF_PRG },	//  0	68000 Program Code
	{ "9.4b",          0x08000, 0xe1b7a989, BRF_ESS | BRF_PRG },	//  1
	{ "12.6d",         0x08000, 0x4de8a3ee, BRF_ESS | BRF_PRG },	//  2
	{ "10.6b",         0x08000, 0xd86bad81, BRF_ESS | BRF_PRG },	//  3
	
	{ "1.15b",         0x04000, 0x55a8b5e7, BRF_ESS | BRF_PRG },	//  4	Z80 Program Code
	{ "2.17b",         0x04000, 0x427a7cca, BRF_ESS | BRF_PRG },	//  5
	{ "3.18b",         0x04000, 0xb8cceaf7, BRF_ESS | BRF_PRG },	//  6
	
	{ "8.16g",         0x02000, 0x0cec8644, BRF_GRA },				//  7	Chars
	
	{ "13.15f",        0x08000, 0x415ff4d9, BRF_GRA },				//  8	Tiles
	{ "14.17f",        0x08000, 0x492b5c48, BRF_GRA },				//  9
	{ "15.18f",        0x08000, 0xb1ac0b9d, BRF_GRA },				// 10
	
	{ "4.6e",          0x04000, 0xf77ced7a, BRF_GRA },				// 11	Sprites
	{ "5.7e",          0x04000, 0x16ef1465, BRF_GRA },				// 12
	{ "6.6g",          0x04000, 0x936ec941, BRF_GRA },				// 13
	{ "7.7g",          0x04000, 0x66dd718e, BRF_GRA },				// 14
	
	{ "clr.10f",       0x00100, 0x6440b341, BRF_GRA },				// 15	PROMs
	{ "clr.11f",       0x00100, 0x271e947f, BRF_GRA },				// 16
	{ "clr.12f",       0x00100, 0x7d38621b, BRF_GRA },				// 17
	{ "2g",            0x00100, 0x44ca16b9, BRF_GRA },				// 18
	
	{ "4e",            0x00100, 0x035f2c7b, BRF_GRA },				// 19	Sprite Palette Bank
	
	{ "16.18g",        0x02000, 0x1d8d592b, BRF_OPT },				// 20	Unknown
};

STD_ROM_PICK(Amazon)
STD_ROM_FN(Amazon)

static struct BurnRomInfo AmatelasRomDesc[] = {
	{ "a11.4d",        0x08000, 0x3d226d0b, BRF_ESS | BRF_PRG },	//  0	68000 Program Code
	{ "a9.4b",         0x08000, 0xe2a0d21d, BRF_ESS | BRF_PRG },	//  1
	{ "a12.6d",        0x08000, 0xe6607c51, BRF_ESS | BRF_PRG },	//  2
	{ "a10.6b",        0x08000, 0xdbc1f1b4, BRF_ESS | BRF_PRG },	//  3
	
	{ "1.15b",         0x04000, 0x55a8b5e7, BRF_ESS | BRF_PRG },	//  4	Z80 Program Code
	{ "2.17b",         0x04000, 0x427a7cca, BRF_ESS | BRF_PRG },	//  5
	{ "3.18b",         0x04000, 0xb8cceaf7, BRF_ESS | BRF_PRG },	//  6
	
	{ "a8.16g",        0x02000, 0xaeba2102, BRF_GRA },				//  7	Chars
	
	{ "13.15f",        0x08000, 0x415ff4d9, BRF_GRA },				//  8	Tiles
	{ "14.17f",        0x08000, 0x492b5c48, BRF_GRA },				//  9
	{ "15.18f",        0x08000, 0xb1ac0b9d, BRF_GRA },				// 10
	
	{ "4.6e",          0x04000, 0xf77ced7a, BRF_GRA },				// 11	Sprites
	{ "a5.7e",         0x04000, 0x5fbf9a16, BRF_GRA },				// 12
	{ "6.6g",          0x04000, 0x936ec941, BRF_GRA },				// 13
	{ "7.7g",          0x04000, 0x66dd718e, BRF_GRA },				// 14
	
	{ "clr.10f",       0x00100, 0x6440b341, BRF_GRA },				// 15	PROMs
	{ "clr.11f",       0x00100, 0x271e947f, BRF_GRA },				// 16
	{ "clr.12f",       0x00100, 0x7d38621b, BRF_GRA },				// 17
	{ "2g",            0x00100, 0x44ca16b9, BRF_GRA },				// 18
	
	{ "4e",            0x00100, 0x035f2c7b, BRF_GRA },				// 19	Sprite Palette Bank
	
	{ "16.18g",        0x02000, 0x1d8d592b, BRF_OPT },				// 20	Unknown
};

STD_ROM_PICK(Amatelas)
STD_ROM_FN(Amatelas)

static struct BurnRomInfo HorekidRomDesc[] = {
	{ "horekid.03",    0x08000, 0x90ec840f, BRF_ESS | BRF_PRG },	//  0	68000 Program Code
	{ "horekid.01",    0x08000, 0xa282faf8, BRF_ESS | BRF_PRG },	//  1
	{ "horekid.04",    0x08000, 0x375c0c50, BRF_ESS | BRF_PRG },	//  2
	{ "horekid.02",    0x08000, 0xee7d52bb, BRF_ESS | BRF_PRG },	//  3
	
	{ "horekid.09",    0x04000, 0x49cd3b81, BRF_ESS | BRF_PRG },	//  4	Z80 Program Code
	{ "horekid.10",    0x04000, 0xc1eaa938, BRF_ESS | BRF_PRG },	//  5
	{ "horekid.11",    0x04000, 0x0a2bc702, BRF_ESS | BRF_PRG },	//  6
	
	{ "horekid.16",    0x02000, 0x104b77cc, BRF_GRA },				//  7	Chars
	
	{ "horekid.05",    0x08000, 0xda25ae10, BRF_GRA },				//  8	Tiles
	{ "horekid.06",    0x08000, 0x616e4321, BRF_GRA },				//  9
	{ "horekid.07",    0x08000, 0x8c7d2be2, BRF_GRA },				// 10
	{ "horekid.08",    0x08000, 0xa0066b02, BRF_GRA },				// 11
	
	{ "horekid.12",    0x08000, 0xa3caa07a, BRF_GRA },				// 12	Sprites
	{ "horekid.13",    0x08000, 0x0e48ff8e, BRF_GRA },				// 13
	{ "horekid.14",    0x08000, 0xe300747a, BRF_GRA },				// 14
	{ "horekid.15",    0x08000, 0x51105741, BRF_GRA },				// 15
	
	{ "kid_prom.10f",  0x00100, 0xca13ce23, BRF_GRA },				// 16	PROMs
	{ "kid_prom.11f",  0x00100, 0xfb44285a, BRF_GRA },				// 17
	{ "kid_prom.12f",  0x00100, 0x40d41237, BRF_GRA },				// 18
	{ "kid_prom.2g",   0x00100, 0x4b9be0ed, BRF_GRA },				// 19
	
	{ "kid_prom.4e",   0x00100, 0xe4fb54ee, BRF_GRA },				// 20	Sprite Palette Bank
	
	{ "horekid.17",    0x02000, 0x1d8d592b, BRF_OPT },				// 21	Unknown
};

STD_ROM_PICK(Horekid)
STD_ROM_FN(Horekid)

static struct BurnRomInfo HorekidbRomDesc[] = {
	{ "knhhd5",        0x08000, 0x786619c7, BRF_ESS | BRF_PRG },	//  0	68000 Program Code
	{ "knhhd7",        0x08000, 0x3bbb475b, BRF_ESS | BRF_PRG },	//  1
	{ "horekid.04",    0x08000, 0x375c0c50, BRF_ESS | BRF_PRG },	//  2
	{ "horekid.02",    0x08000, 0xee7d52bb, BRF_ESS | BRF_PRG },	//  3
	
	{ "horekid.09",    0x04000, 0x49cd3b81, BRF_ESS | BRF_PRG },	//  4	Z80 Program Code
	{ "horekid.10",    0x04000, 0xc1eaa938, BRF_ESS | BRF_PRG },	//  5
	{ "horekid.11",    0x04000, 0x0a2bc702, BRF_ESS | BRF_PRG },	//  6
	
	{ "horekid.16",    0x02000, 0x104b77cc, BRF_GRA },				//  7	Chars
	
	{ "horekid.05",    0x08000, 0xda25ae10, BRF_GRA },				//  8	Tiles
	{ "horekid.06",    0x08000, 0x616e4321, BRF_GRA },				//  9
	{ "horekid.07",    0x08000, 0x8c7d2be2, BRF_GRA },				// 10
	{ "horekid.08",    0x08000, 0xa0066b02, BRF_GRA },				// 11
	
	{ "horekid.12",    0x08000, 0xa3caa07a, BRF_GRA },				// 12	Sprites
	{ "horekid.13",    0x08000, 0x0e48ff8e, BRF_GRA },				// 13
	{ "horekid.14",    0x08000, 0xe300747a, BRF_GRA },				// 14
	{ "horekid.15",    0x08000, 0x51105741, BRF_GRA },				// 15
	
	{ "kid_prom.10f",  0x00100, 0xca13ce23, BRF_GRA },				// 16	PROMs
	{ "kid_prom.11f",  0x00100, 0xfb44285a, BRF_GRA },				// 17
	{ "kid_prom.12f",  0x00100, 0x40d41237, BRF_GRA },				// 18
	{ "kid_prom.2g",   0x00100, 0x4b9be0ed, BRF_GRA },				// 19
	
	{ "kid_prom.4e",   0x00100, 0xe4fb54ee, BRF_GRA },				// 20	Sprite Palette Bank
	
	{ "horekid.17",    0x02000, 0x1d8d592b, BRF_OPT },				// 21	Unknown
};

STD_ROM_PICK(Horekidb)
STD_ROM_FN(Horekidb)

static struct BurnRomInfo BoobhackRomDesc[] = {
	{ "1-c.bin",       0x08000, 0x786619c7, BRF_ESS | BRF_PRG },	//  0	68000 Program Code
	{ "1-b.bin",       0x08000, 0x3bbb475b, BRF_ESS | BRF_PRG },	//  1
	{ "1-d.bin",       0x08000, 0x375c0c50, BRF_ESS | BRF_PRG },	//  2
	{ "1-a.bin",       0x08000, 0xee7d52bb, BRF_ESS | BRF_PRG },	//  3
	
	{ "1-i.bin",       0x04000, 0x49cd3b81, BRF_ESS | BRF_PRG },	//  4	Z80 Program Code
	{ "1-j.bin",       0x04000, 0xc1eaa938, BRF_ESS | BRF_PRG },	//  5
	{ "1-k.bin",       0x04000, 0x0a2bc702, BRF_ESS | BRF_PRG },	//  6
	
	{ "1-p.bin",       0x02000, 0x104b77cc, BRF_GRA },				//  7	Chars
	
	{ "1-e.bin",       0x08000, 0xda25ae10, BRF_GRA },				//  8	Tiles
	{ "1-f.bin",       0x08000, 0x616e4321, BRF_GRA },				//  9
	{ "1-g.bin",       0x08000, 0x8c7d2be2, BRF_GRA },				// 10
	{ "1-h.bin",       0x08000, 0xa0066b02, BRF_GRA },				// 11

	{ "1-l.bin",       0x08000, 0xa3caa07a, BRF_GRA },				// 12	Sprites
	{ "1-m.bin",       0x08000, 0x15b6cbdf, BRF_GRA },				// 13
	{ "1-n.bin",       0x08000, 0xe300747a, BRF_GRA },				// 14
	{ "1-o.bin",       0x08000, 0xcddc6a6c, BRF_GRA },				// 15
	
	{ "kid_prom.10f",  0x00100, 0xca13ce23, BRF_GRA },				// 16	PROMs
	{ "kid_prom.11f",  0x00100, 0xfb44285a, BRF_GRA },				// 17
	{ "kid_prom.12f",  0x00100, 0x40d41237, BRF_GRA },				// 18
	{ "kid_prom.2g",   0x00100, 0x4b9be0ed, BRF_GRA },				// 19
	
	{ "kid_prom.4e",   0x00100, 0xe4fb54ee, BRF_GRA },				// 20	Sprite Palette Bank
};

STD_ROM_PICK(Boobhack)
STD_ROM_FN(Boobhack)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Drv68KRom              = Next; Next += 0x020000;
	DrvZ80Rom              = Next; Next += 0x00c000;
	DrvProms               = Next; Next += 0x000400;
	DrvSpritePalBank       = Next; Next += 0x000100;

	RamStart               = Next;
	
	Drv68KRam              = Next; Next += 0x001000;
	DrvSpriteRam           = Next; Next += 0x002000;
	DrvBgVideoRam          = Next; Next += 0x001000;
	DrvFgVideoRam          = Next; Next += 0x001000;
	DrvZ80Ram              = Next; Next += 0x001000;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x100 * 8 * 8;
	DrvTiles               = Next; Next += 0x400 * 16 * 16;
	DrvSprites             = Next; Next += 0x400 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x1110 * sizeof(UINT32);

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
	if (DrvUseYM2203) {
		BurnYM2203Reset();
	} else {
		BurnYM3526Reset();
	}
	ZetClose();
	
	DACReset();
	
	DrvScrollX = 0;
	DrvScrollY = 0;
	DrvDisableBg = 0;
	DrvFlipScreen = 0;
	DrvSoundLatch = 0;
	AmazonProtCmd = 0;
	memset(AmazonProtReg, 0, 6);
	
	return 0;
}

UINT8 __fastcall Terracre68KReadByte(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Terracre68KWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Terracre68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x024000: {
			return DrvInput[0];
		}
		
		case 0x024002: {
			return DrvInput[1];
		}
		
		case 0x024004: {
			return (DrvInput[2] | DrvDip[0]) << 8;
		}
		
		case 0x024006: {
			return (DrvDip[2] << 8) | DrvDip[1];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Terracre68KWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x026000: {
			DrvFlipScreen = d;
			return;
		}
		
		case 0x026002: {
			DrvScrollX = d & 0x3ff;
			DrvDisableBg = (d & 0x2000) ? 1 : 0;
			return;
		}
		
		case 0x026004: {
			DrvScrollY = d & 0x1ff;
			return;
		}
		
		case 0x02600a: {
			// ???
			return;
		}
		
		case 0x02600c: {
			DrvSoundLatch = ((d & 0x7f) << 1) | 1;
			return;
		}
		
		case 0x02600e: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Amazon68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x070001: {
			INT32 Offset = AmazonProtReg[2];
			if (Offset <= 0x56) {
				UINT16 Data;
				Data = AmazonProtDataPtr[Offset >> 1];
				if (Offset & 0x01) return Data & 0xff;
				return Data >> 8;
			}
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Amazon68KWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x070001: {
			if (AmazonProtCmd >= 32 && AmazonProtCmd <= 0x37) {
				AmazonProtReg[AmazonProtCmd - 0x32] = d;
			}
			return;
		}
		
		case 0x070003: {
			AmazonProtCmd = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Amazon68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x044000: {
			if (DrvIsHorekid) {
				return (DrvDip[2] << 8) | DrvDip[1];
			}
			return DrvInput[0];
		}
		
		case 0x044002: {
			if (DrvIsHorekid) {
				return (DrvInput[2] | DrvDip[0]) << 8;
			}
			return DrvInput[1];
		}
		
		case 0x044004: {
			if (DrvIsHorekid) {
				return DrvInput[1];
			}
			return (DrvInput[2] | DrvDip[0]) << 8;
		}
		
		case 0x044006: {
			if (DrvIsHorekid) {
				return DrvInput[0];
			}
			return (DrvDip[2] << 8) | DrvDip[1];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Amazon68KWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x046000: {
			DrvFlipScreen = d;
			return;
		}
		
		case 0x046002: {
			DrvScrollX = d & 0x3ff;
			DrvDisableBg = (d & 0x2000) ? 1 : 0;
			return;
		}
		
		case 0x046004: {
			DrvScrollY = d & 0x1ff;
			return;
		}
		
		case 0x04600a: {
			// ???
			return;
		}
		
		case 0x04600c: {
			DrvSoundLatch = ((d & 0x7f) << 1) | 1;
			return;
		}
		
		case 0x04600e: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall TerracreZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x04: {
			DrvSoundLatch = 0;
			return 0;
		}
		
		case 0x06: {
			return DrvSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall TerracreZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			BurnYM3526Write(0, d);
			return;
		}
		
		case 0x01: {
			BurnYM3526Write(1, d);
			return;
		}
		
		case 0x02: {
			DACSignedWrite(0, d);
			return;
		}
		
		case 0x03: {
			DACSignedWrite(1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void __fastcall TerracreYM2203Z80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			BurnYM2203Write(0, 0, d);
			return;
		}
		
		case 0x01: {
			BurnYM2203Write(0, 1, d);
			return;
		}
		
		case 0x02: {
			DACSignedWrite(0, d);
			return;
		}
		
		case 0x03: {
			DACSignedWrite(1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

inline static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 4000000;
}

static INT32 TerracreSyncDAC()
{
	return (INT32)(float)(nBurnSoundLen * (ZetTotalCycles() / (4000000.0000 / (nBurnFPS / 100.0000))));
}

static INT32 CharPlaneOffsets[4]       = { 0, 1, 2, 3 };
static INT32 CharXOffsets[8]           = { 4, 0, 12, 8, 20, 16, 28, 24 };
static INT32 CharYOffsets[8]           = { 0, 32, 64, 96, 128, 160, 192, 224 };
static INT32 TilePlaneOffsets[4]       = { 0, 1, 2, 3 };
static INT32 TileXOffsets[16]          = { 4, 0, 12, 8, 20, 16, 28, 24, 36, 32, 44, 40, 52, 48, 60, 56 };
static INT32 TileYOffsets[16]          = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };
static INT32 SpritePlaneOffsets[4]     = { 0, 1, 2, 3 };
static INT32 SpriteXOffsets[16]        = { 4, 0, RGN_FRAC(0x10000,1,2)+4,RGN_FRAC(0x10000,1,2)+0, 12, 8, RGN_FRAC(0x10000,1,2)+12, RGN_FRAC(0x10000,1,2)+8, 20, 16, RGN_FRAC(0x10000,1,2)+20, RGN_FRAC(0x10000,1,2)+16, 28, 24, RGN_FRAC(0x10000,1,2)+28, RGN_FRAC(0x10000,1,2)+24 };
static INT32 HorekidSpriteXOffsets[16] = { 4, 0, RGN_FRAC(0x20000,1,2)+4,RGN_FRAC(0x20000,1,2)+0, 12, 8, RGN_FRAC(0x20000,1,2)+12, RGN_FRAC(0x20000,1,2)+8, 20, 16, RGN_FRAC(0x20000,1,2)+20, RGN_FRAC(0x20000,1,2)+16, 28, 24, RGN_FRAC(0x20000,1,2)+28, RGN_FRAC(0x20000,1,2)+24 };
static INT32 SpriteYOffsets[16]        = { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480 };

static INT32 DrvInit()
{
	INT32 nLen;
	
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();
	
	if (LoadRomsFunction()) return 1;

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x01ffff, SM_ROM);
	SekMapMemory(DrvSpriteRam        , 0x020000, 0x021fff, SM_RAM);
	SekMapMemory(DrvBgVideoRam       , 0x022000, 0x022fff, SM_RAM);
//	SekMapMemory(Drv68KRam           , 0x023000, 0x023fff, SM_RAM);
	SekMapMemory(DrvFgVideoRam       , 0x028000, 0x0287ff, SM_RAM);
	SekSetReadWordHandler(0, Terracre68KReadWord);
	SekSetWriteWordHandler(0, Terracre68KWriteWord);
	SekSetReadByteHandler(0, Terracre68KReadByte);
	SekSetWriteByteHandler(0, Terracre68KWriteByte);
	SekClose();
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetInHandler(TerracreZ80PortRead);
	if (DrvUseYM2203) {		
		ZetSetOutHandler(TerracreYM2203Z80PortWrite);
	} else {
		ZetSetOutHandler(TerracreZ80PortWrite);
	}
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80Rom);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80Rom);
	ZetMapArea(0xc000, 0xcfff, 0, DrvZ80Ram);
	ZetMapArea(0xc000, 0xcfff, 1, DrvZ80Ram);
	ZetMapArea(0xc000, 0xcfff, 2, DrvZ80Ram);
	ZetMemEnd();
	ZetClose();
	
	if (DrvUseYM2203) {
		BurnYM2203Init(1, 4000000, NULL, DrvSynchroniseStream, DrvGetTime, 0);
		BurnYM2203SetVolumeShift(2);
		BurnTimerAttachZet(4000000);
	} else {
		BurnYM3526Init(4000000, NULL, &DrvSynchroniseStream, 0);
		BurnTimerAttachZetYM3526(4000000);
	}
	
	DACInit(0, 0, 1, TerracreSyncDAC);
	DACInit(1, 0, 1, TerracreSyncDAC);
	DACSetVolShift(0, 1);
	DACSetVolShift(1, 1);
	
	GenericTilesInit();
	
	DrvRecalcPal = 1;

	DrvDoReset();

	return 0;
}

static INT32 DrvAmazonInit()
{
	INT32 nLen;
	
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();
	
	if (LoadRomsFunction()) return 1;

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x01ffff, SM_ROM);
	SekMapMemory(DrvSpriteRam        , 0x040000, 0x040fff, SM_RAM);
	SekMapMemory(DrvBgVideoRam       , 0x042000, 0x042fff, SM_RAM);
	SekMapMemory(DrvFgVideoRam       , 0x050000, 0x050fff, SM_RAM);
	SekSetReadWordHandler(0, Amazon68KReadWord);
	SekSetWriteWordHandler(0, Amazon68KWriteWord);
	SekSetReadByteHandler(0, Amazon68KReadByte);
	SekSetWriteByteHandler(0, Amazon68KWriteByte);
	SekClose();
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetInHandler(TerracreZ80PortRead);
	ZetSetOutHandler(TerracreZ80PortWrite);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80Rom);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80Rom);
	ZetMapArea(0xc000, 0xcfff, 0, DrvZ80Ram);
	ZetMapArea(0xc000, 0xcfff, 1, DrvZ80Ram);
	ZetMapArea(0xc000, 0xcfff, 2, DrvZ80Ram);
	ZetMemEnd();
	ZetClose();
	
	BurnYM3526Init(4000000, NULL, &DrvSynchroniseStream, 0);
	BurnTimerAttachZetYM3526(4000000);
	
	DACInit(0, 0, 1, TerracreSyncDAC);
	DACInit(1, 0, 1, TerracreSyncDAC);
	DACSetVolShift(0, 2);
	DACSetVolShift(1, 2);
	
	GenericTilesInit();
	
	DrvRecalcPal = 1;

	DrvDoReset();

	return 0;
}

static INT32 TerracreLoadRoms()
{
	INT32 nRet;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x10000);
	
	nRet = BurnLoadRom(Drv68KRom + 0x000000,  0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x000001,  1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x010000,  2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x010001,  3, 2); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvZ80Rom + 0x000000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom + 0x004000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom + 0x008000,  6, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, DrvTempRom, DrvChars);

	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000,  9, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvTiles);
	
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c000, 13, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	nRet = BurnLoadRom(DrvProms   + 0x00000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00100, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00200, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00300, 17, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvSpritePalBank    , 18, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 TerracreoLoadRoms()
{
	INT32 nRet;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x10000);
	
	nRet = BurnLoadRom(Drv68KRom + 0x000000,  0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x000001,  1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x008000,  2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x008001,  3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x010000,  4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x010001,  5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x018000,  6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x018001,  7, 2); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvZ80Rom + 0x000000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom + 0x004000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom + 0x008000, 10, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, DrvTempRom, DrvChars);

	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 13, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvTiles);
	
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c000, 17, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	nRet = BurnLoadRom(DrvProms   + 0x00000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00100, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00200, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00300, 21, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvSpritePalBank    , 22, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 TerracrenLoadRoms()
{
	INT32 nRet;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x10000);
	
	nRet = BurnLoadRom(Drv68KRom + 0x000000,  0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x000001,  1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x008000,  2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x008001,  3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x010000,  4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x010001,  5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x018000,  6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x018001,  7, 2); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvZ80Rom + 0x000000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom + 0x004000,  9, 1); if (nRet != 0) return 1;
	memset(DrvZ80Rom + 0x8000, 0xff, 0x4000);
	
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, DrvTempRom, DrvChars);

	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvTiles);
	
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c000, 16, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	nRet = BurnLoadRom(DrvProms   + 0x00000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00100, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00200, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00300, 20, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvSpritePalBank    , 21, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 AmazonLoadRoms()
{
	INT32 nRet;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x20000);
	
	nRet = BurnLoadRom(Drv68KRom + 0x000001,  0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x000000,  1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x010001,  2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x010000,  3, 2); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvZ80Rom + 0x000000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom + 0x004000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom + 0x008000,  6, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0, 0x20000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, DrvTempRom, DrvChars);

	memset(DrvTempRom, 0, 0x20000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvTiles);
	
	memset(DrvTempRom, 0, 0x20000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c000, 14, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	nRet = BurnLoadRom(DrvProms   + 0x00000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00100, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00200, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00300, 18, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvSpritePalBank    , 19, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 HorekidLoadRoms()
{
	INT32 nRet;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x20000);
	
	nRet = BurnLoadRom(Drv68KRom + 0x000001,  0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x000000,  1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x010001,  2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x010000,  3, 2); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvZ80Rom + 0x000000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom + 0x004000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom + 0x008000,  6, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0, 0x20000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, DrvTempRom, DrvChars);

	memset(DrvTempRom, 0, 0x20000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 11, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvTiles);
	
	memset(DrvTempRom, 0, 0x20000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 15, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 4, 16, 16, SpritePlaneOffsets, HorekidSpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	nRet = BurnLoadRom(DrvProms   + 0x00000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00100, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00200, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProms   + 0x00300, 19, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvSpritePalBank    , 20, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static UINT16 AmazonProtData[] = {
	0x0000, 0x5000, 0x5341, 0x4b45, 0x5349, 0x4755, 0x5245,
	0x0000, 0x4000, 0x0e4b, 0x4154, 0x5544, 0x4f4e, 0x0e0e,
	0x0000, 0x3000, 0x414e, 0x4b41, 0x4b45, 0x5544, 0x4f4e,
	0x0000, 0x2000, 0x0e0e, 0x4b49, 0x5455, 0x4e45, 0x0e0e,
	0x0000, 0x1000, 0x0e4b, 0x414b, 0x4553, 0x4f42, 0x410e,

	0x4ef9, 0x0000, 0x62fa, 0x0000, 0x4ef9, 0x0000, 0x805E, 0x0000,	0xc800
};

static UINT16 AmatelasProtData[] = {
	0x0000, 0x5000, 0x5341, 0x4b45, 0x5349, 0x4755, 0x5245,
	0x0000, 0x4000, 0x0e4b, 0x4154, 0x5544, 0x4f4e, 0x0e0e,
	0x0000, 0x3000, 0x414e, 0x4b41, 0x4b45, 0x5544, 0x4f4e,
	0x0000, 0x2000, 0x0e0e, 0x4b49, 0x5455, 0x4e45, 0x0e0e,
	0x0000, 0x1000, 0x0e4b, 0x414b, 0x4553, 0x4f42, 0x410e,

	0x4ef9, 0x0000, 0x632e, 0x0000, 0x4ef9, 0x0000, 0x80C2, 0x0000, 0x6100
};

static UINT16 HorekidProtData[] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,

	0x4e75, 0x4e75, 0x4e75, 0x4e75, 0x4e75, 0x4e75, 0x4e75, 0x4e75, 0x1800
};

static INT32 TerracreInit()
{
	LoadRomsFunction = TerracreLoadRoms;

	return DrvInit();
}

static INT32 TerracreoInit()
{
	LoadRomsFunction = TerracreoLoadRoms;

	return DrvInit();
}

static INT32 TerracrenInit()
{
	LoadRomsFunction = TerracrenLoadRoms;
	DrvUseYM2203 = 1;

	return DrvInit();
}

static INT32 AmazonInit()
{
	LoadRomsFunction = AmazonLoadRoms;
	AmazonProtDataPtr = AmazonProtData;

	return DrvAmazonInit();
}

static INT32 AmatelasInit()
{
	LoadRomsFunction = AmazonLoadRoms;
	AmazonProtDataPtr = AmatelasProtData;

	return DrvAmazonInit();
}

static INT32 HorekidInit()
{
	LoadRomsFunction = HorekidLoadRoms;
	AmazonProtDataPtr = HorekidProtData;
	DrvIsHorekid = 1;

	return DrvAmazonInit();
}

static INT32 DrvExit()
{
	SekExit();
	ZetExit();
	
	if (DrvUseYM2203) {
		BurnYM2203Exit();
	} else {
		BurnYM3526Exit();
	}
	DACExit();
	
	GenericTilesExit();
	
	BurnFree(Mem);
	
	DrvScrollX = 0;
	DrvScrollY = 0;
	DrvDisableBg = 0;
	DrvFlipScreen = 0;
	DrvSoundLatch = 0;
	AmazonProtCmd = 0;
	memset(AmazonProtReg, 0, 6);
	
	DrvUseYM2203 = 0;
	DrvIsHorekid = 0;
	
	LoadRomsFunction = NULL;
	AmazonProtDataPtr = NULL;
	
	return 0;
}

static inline UINT8 pal4bit(UINT8 bits)
{
	bits &= 0x0f;
	return (bits << 4) | bits;
}

static void DrvCalcPalette()
{
	UINT32 PalLookup[0x100];
	INT32 i;

	for (i = 0; i < 0x100; i++) {
		INT32 r = pal4bit(DrvProms[i + 0x000]);
		INT32 g = pal4bit(DrvProms[i + 0x100]);
		INT32 b = pal4bit(DrvProms[i + 0x200]);

		PalLookup[i] = BurnHighCol(r, g, b, 0);
	}

	for (i = 0; i < 0x10; i++) {
		DrvPalette[i] = PalLookup[i];
	}

	for (i = 0; i < 0x100; i++) {
		UINT8 ctabentry;

		if (i & 0x08) {
			ctabentry = 0xc0 | (i & 0x0f) | ((i & 0xc0) >> 2);
		} else {
			ctabentry = 0xc0 | (i & 0x0f) | ((i & 0x30) >> 0);
		}

		DrvPalette[0x10 + i] = PalLookup[ctabentry];
	}
	
	for (i = 0; i < 0x1000; i++) {
		UINT8 ctabentry;
		INT32 i_swapped = ((i & 0x0f) << 8) | ((i & 0xff0) >> 4);

		if (i & 0x80) {
			ctabentry = 0x80 | ((i & 0x0c) << 2) | (DrvProms[0x300 + (i >> 4)] & 0x0f);
		} else {
			ctabentry = 0x80 | ((i & 0x03) << 4) | (DrvProms[0x300 + (i >> 4)] & 0x0f);
		}

		DrvPalette[0x110 + i_swapped] = PalLookup[ctabentry];
	}
	
	DrvRecalcPal = 0;
}

static void DrvRenderBgLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex;
	
	UINT16 *VideoRam = (UINT16*)DrvBgVideoRam;
	
	for (mx = 0; mx < 32; mx++) {
		for (my = 0; my < 64; my++) {
			TileIndex = (my * 32) + mx;
			Code = VideoRam[TileIndex];
			Colour = (Code >> 11) + (0x10 >> 4);
			Code &= 0x3ff;
			
			y = 16 * mx;
			x = 16 * my;
			
			x -= DrvScrollX;
			y -= DrvScrollY;
			if (x < -16) x += 1024;
			if (y < -16) y += 512;
			
			if (DrvFlipScreen) {
				x = 240 - x;
				y = 240 - y;
			}

			y -= 16;

			if (DrvFlipScreen) {
				if (x > 16 && x < 240 && y > 16 && y < 208) {
					Render16x16Tile_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, DrvTiles);
				} else {
					Render16x16Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, DrvTiles);
				}
			} else {
				if (x > 16 && x < 240 && y > 16 && y < 208) {
					Render16x16Tile(pTransDraw, Code, x, y, Colour, 4, 0, DrvTiles);
				} else {
					Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 0, DrvTiles);
				}
			}
		}
	}
}

static void DrvRenderFgLayer()
{
	INT32 mx, my, Code, x, y, TileIndex;
	
	UINT16 *VideoRam = (UINT16*)DrvFgVideoRam;
	
	for (mx = 0; mx < 32; mx++) {
		for (my = 0; my < 64; my++) {
			TileIndex = (my * 32) + mx;
			Code = VideoRam[TileIndex] & 0xff;
			
			y = 8 * mx;
			x = 8 * my;
			
			if (DrvFlipScreen) {
				x = 248 - x;
				y = 248 - y;
			}
			
			y -= 16;

			if (DrvFlipScreen) {
				if (x > 8 && x < 248 && y > 8 && y < 216) {
					Render8x8Tile_Mask_FlipXY(pTransDraw, Code, x, y, 0, 4, 0x0f, 0, DrvChars);
				} else {
					Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, 0, 4, 0x0f, 0, DrvChars);
				}
			} else {
				if (x > 8 && x < 248 && y > 8 && y < 216) {
					Render8x8Tile_Mask(pTransDraw, Code, x, y, 0, 4, 0x0f, 0, DrvChars);
				} else {
					Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, 0, 4, 0x0f, 0, DrvChars);
				}
			}
		}
	}
}

static void DrawSprites()
{
	UINT16 *pSource = (UINT16*)DrvSpriteRam;
	INT32 TransparentPen = 0x00;
	if (DrvIsHorekid) TransparentPen = 0x0f;

	for (INT32 i = 0; i < 0x200; i += 8)	{
		INT32 Tile = pSource[1] & 0xff;
		INT32 Attrs = pSource[2];
		INT32 xFlip = Attrs & 0x04;
		INT32 yFlip = Attrs & 0x08;
		INT32 Colour = (Attrs & 0xf0) >> 4;
		INT32 sx = (pSource[3] & 0xff) - 0x80 + 256 * (Attrs & 1);
		INT32 sy = 240 - (pSource[0] & 0xff);

		if (DrvIsHorekid) {
			INT32 Bank;

			if (Attrs & 0x02) Tile |= 0x200;
			if (Attrs & 0x10) Tile |= 0x100;

			Bank = (Tile & 0xfc) >> 1;
			if (Tile & 0x200) Bank |= 0x80;
			if (Tile & 0x100) Bank |= 0x01;

			Colour &= 0xe;
			Colour += 16 * (DrvSpritePalBank[Bank] & 0x0f);
		} else {
			if (Attrs & 0x02) Tile |= 0x100;
			Colour += 16 * (DrvSpritePalBank[(Tile >> 1) & 0xff] & 0x0f);
		}
		
		Colour += (0x110) >> 4;
		
		if (DrvFlipScreen) {
			sx = 240 - sx;
			sy = 240 - sy;
			xFlip = !xFlip;
			yFlip = !yFlip;
		}
		
		sy -= 16;
			
		if (sx > 16 && sx < 240 && sy > 16 && sy < 208) {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Tile, sx, sy, Colour, 4, TransparentPen, 0, DrvSprites);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Tile, sx, sy, Colour, 4, TransparentPen, 0, DrvSprites);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Tile, sx, sy, Colour, 4, TransparentPen, 0, DrvSprites);
				} else {
					Render16x16Tile_Mask(pTransDraw, Tile, sx, sy, Colour, 4, TransparentPen, 0, DrvSprites);
				}
			}
		} else {
			if (xFlip) {
				if (yFlip) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Tile, sx, sy, Colour, 4, TransparentPen, 0, DrvSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Tile, sx, sy, Colour, 4, TransparentPen, 0, DrvSprites);
				}
			} else {
				if (yFlip) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Tile, sx, sy, Colour, 4, TransparentPen, 0, DrvSprites);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Tile, sx, sy, Colour, 4, TransparentPen, 0, DrvSprites);
				}
			}
		}

		pSource += 4;
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	if (DrvRecalcPal) DrvCalcPalette();
	if (nBurnLayer & 0x01 && !DrvDisableBg) DrvRenderBgLayer();
	if (nSpriteEnable & 0x01) DrawSprites();
	if (nBurnLayer & 0x02) DrvRenderFgLayer();
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nCyclesTotal[2] = { 8000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };
	
	INT32 nInterleave = 100;
	
	if (DrvReset) DrvDoReset();

	DrvMakeInputs();
	
	INT32 Z80IRQSlice[9];
	for (INT32 i = 0; i < 9; i++) {
		Z80IRQSlice[i] = (INT32)((double)((nInterleave * (i + 1)) / 10));
	}
	
	SekNewFrame();
	ZetNewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext, nCyclesSegment;
		
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == (nInterleave - 1)) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
		SekClose();
		
		nCurrentCPU = 1;
		ZetOpen(0);
		if (DrvUseYM2203) {
#if 0
			// The sound cpu fails to read the latches if we run this here
			BurnTimerUpdate(i * (nCyclesTotal[nCurrentCPU] / nInterleave));
#endif
		} else {
			BurnTimerUpdateYM3526(i * (nCyclesTotal[nCurrentCPU] / nInterleave));
		}
		for (INT32 j = 0; j < 9; j++) {
			if (i == Z80IRQSlice[j]) {
				ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
				nCyclesDone[1] += ZetRun(4000);
				ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			}
		}
		ZetClose();
	}
	
	ZetOpen(0);
	if (DrvUseYM2203) {
		BurnTimerEndFrame(nCyclesTotal[1]);
	} else {
		BurnTimerEndFrameYM3526(nCyclesTotal[1]);
	}
	if (pBurnSoundOut) {
		if (DrvUseYM2203) {
			BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		} else {
			BurnYM3526Update(pBurnSoundOut, nBurnSoundLen);
		}
		DACUpdate(pBurnSoundOut, nBurnSoundLen);
	}
	ZetClose();
	
	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029719;
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
		ZetScan(nAction);
		
		if (DrvUseYM2203) {
			BurnYM2203Scan(nAction, pnMin);
		} else {
			BurnYM3526Scan(nAction, pnMin);
		}
		
		DACScan(nAction, pnMin);
		
		SCAN_VAR(DrvScrollX);
		SCAN_VAR(DrvScrollY);
		SCAN_VAR(DrvDisableBg);
		SCAN_VAR(DrvFlipScreen);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(AmazonProtCmd);
		SCAN_VAR(AmazonProtReg);
		
		if (nAction & ACB_WRITE) {
			DrvRecalcPal = 1;
		}
	}

	return 0;
}

struct BurnDriver BurnDrvTerracre = {
	"terracre", NULL, NULL, NULL, "1985",
	"Terra Cresta (YM3526 set 1)\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	TerracreInit, DrvExit, DrvFrame, NULL, DrvScan,
	&DrvRecalcPal, 0x1110, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTerracreo = {
	"terracreo", "terracre", NULL, NULL, "1985",
	"Terra Cresta (YM3526 set 2)\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, DrvoRomInfo, DrvoRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	TerracreoInit, DrvExit, DrvFrame, NULL, DrvScan,
	&DrvRecalcPal, 0x1110, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTerracrea = {
	"terracrea", "terracre", NULL, NULL, "1985",
	"Terra Cresta (YM3526 set 3)\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, DrvaRomInfo, DrvaRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	TerracreoInit, DrvExit, DrvFrame, NULL, DrvScan,
	&DrvRecalcPal, 0x1110, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTerracren = {
	"terracren", "terracre", NULL, NULL, "1985",
	"Terra Cresta (YM2203)\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, DrvnRomInfo, DrvnRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	TerracrenInit, DrvExit, DrvFrame, NULL, DrvScan,
	&DrvRecalcPal, 0x1110, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAmazon = {
	"amazon", NULL, NULL, NULL, "1986",
	"Soldier Girl Amazon\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, AmazonRomInfo, AmazonRomName, NULL, NULL, AmazonInputInfo, AmazonDIPInfo,
	AmazonInit, DrvExit, DrvFrame, NULL, DrvScan,
	&DrvRecalcPal, 0x1110, 224, 256, 3, 4
};

struct BurnDriver BurnDrvAmatelas = {
	"amatelas", "amazon", NULL, NULL, "1986",
	"Sei Senshi Amatelass\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, AmatelasRomInfo, AmatelasRomName, NULL, NULL, AmazonInputInfo, AmazonDIPInfo,
	AmatelasInit, DrvExit, DrvFrame, NULL, DrvScan,
	&DrvRecalcPal, 0x1110, 224, 256, 3, 4
};

struct BurnDriver BurnDrvHorekid = {
	"horekid", NULL, NULL, NULL, "1987",
	"Kid no Hore Hore Daisakusen\0", NULL, "Nichibutsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, HorekidRomInfo, HorekidRomName, NULL, NULL, HorekidInputInfo, HorekidDIPInfo,
	HorekidInit, DrvExit, DrvFrame, NULL, DrvScan,
	&DrvRecalcPal, 0x1110, 224, 256, 3, 4
};

struct BurnDriver BurnDrvHorekidb = {
	"horekidb", "horekid", NULL, NULL, "1987",
	"Kid no Hore Hore Daisakusen (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, HorekidbRomInfo, HorekidbRomName, NULL, NULL, HorekidInputInfo, HorekidDIPInfo,
	HorekidInit, DrvExit, DrvFrame, NULL, DrvScan,
	&DrvRecalcPal, 0x1110, 224, 256, 3, 4
};

struct BurnDriver BurnDrvBoobhack = {
	"boobhack", "horekid", NULL, NULL, "1987",
	"Booby Kids (Italian manufactured graphic hack / bootleg of Kid no Hore Hore Daisakusen (bootleg))\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, BoobhackRomInfo, BoobhackRomName, NULL, NULL, HorekidInputInfo, HorekidDIPInfo,
	HorekidInit, DrvExit, DrvFrame, NULL, DrvScan,
	&DrvRecalcPal, 0x1110, 224, 256, 3, 4
};
