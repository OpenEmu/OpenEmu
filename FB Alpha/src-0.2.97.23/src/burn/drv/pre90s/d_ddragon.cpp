#include "tiles_generic.h"
#include "z80_intf.h"
#include "hd6309_intf.h"
#include "m6800_intf.h"
#include "m6805_intf.h"
#include "m6809_intf.h"
#include "burn_ym2151.h"
#include "msm5205.h"
#include "msm6295.h"
#include "bitswap.h"

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
static UINT8 *DrvHD6309Rom        = NULL;
static UINT8 *DrvSubCPURom        = NULL;
static UINT8 *DrvSoundCPURom      = NULL;
static UINT8 *DrvMCURom           = NULL;
static UINT8 *DrvMSM5205Rom       = NULL;
static UINT8 *DrvHD6309Ram        = NULL;
static UINT8 *DrvSubCPURam        = NULL;
static UINT8 *DrvSoundCPURam      = NULL;
static UINT8 *DrvMCURam           = NULL;
static UINT8 *DrvMCUPorts         = NULL;
static UINT8 *DrvFgVideoRam       = NULL;
static UINT8 *DrvBgVideoRam       = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvPaletteRam1      = NULL;
static UINT8 *DrvPaletteRam2      = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvTiles            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette          = NULL;

static UINT8 DrvRomBank;
static UINT8 DrvVBlank;
static UINT8 DrvSubCPUBusy;
static UINT8 DrvSoundLatch;
static UINT8 DrvADPCMIdle[2];
static UINT32 DrvADPCMPos[2];
static UINT32 DrvADPCMEnd[2];
static INT32 DrvADPCMData[2];

static UINT16 DrvScrollXHi;
static UINT16 DrvScrollYHi;
static UINT8  DrvScrollXLo;
static UINT8  DrvScrollYLo;

static INT32 nCyclesDone[4], nCyclesTotal[4];
static INT32 nCyclesSegment;

#define DD_CPU_TYPE_NONE		0
#define DD_CPU_TYPE_HD63701		1
#define DD_CPU_TYPE_HD6309		2
#define DD_CPU_TYPE_M6803		3
#define DD_CPU_TYPE_Z80			4
#define DD_CPU_TYPE_M6809		5
static INT32 DrvSubCPUType;
static INT32 DrvSoundCPUType;

#define DD_VID_TYPE_DD2			1
static INT32 DrvVidHardwareType;

#define DD_GAME_DARKTOWR		1
static INT32 DrvGameType;

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort1 + 6, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 7, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , DrvInputPort2 + 1, "p1 fire 3" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort2 + 0, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Drv)

static inline void DrvMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = DrvInput[1] = 0xff;
	DrvInput[2] = 0xe7;

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
	{0x14, 0xff, 0xff, 0xff, NULL                     },
	{0x15, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x14, 0x01, 0x07, 0x00, "4 Coins 1 Play"         },
	{0x14, 0x01, 0x07, 0x01, "3 Coins 1 Play"         },
	{0x14, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x14, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x14, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x14, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x14, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x14, 0x01, 0x07, 0x03, "1 Coin  5 Plays"        },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x14, 0x01, 0x38, 0x00, "4 Coins 1 Play"         },
	{0x14, 0x01, 0x38, 0x08, "3 Coins 1 Play"         },
	{0x14, 0x01, 0x38, 0x10, "2 Coins 1 Play"         },
	{0x14, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x14, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x14, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
	{0x14, 0x01, 0x38, 0x20, "1 Coin  4 Plays"        },
	{0x14, 0x01, 0x38, 0x18, "1 Coin  5 Plays"        },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x14, 0x01, 0x40, 0x40, "Upright"                },
	{0x14, 0x01, 0x40, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x14, 0x01, 0x80, 0x80, "Off"                    },
	{0x14, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x15, 0x01, 0x03, 0x01, "Easy"                   },
	{0x15, 0x01, 0x03, 0x03, "Medium"                 },
	{0x15, 0x01, 0x03, 0x02, "Hard"                   },
	{0x15, 0x01, 0x03, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x15, 0x01, 0x04, 0x00, "Off"                    },
	{0x15, 0x01, 0x04, 0x04, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x15, 0x01, 0x30, 0x10, "20k"                    },
	{0x15, 0x01, 0x30, 0x00, "40k"                    },
	{0x15, 0x01, 0x30, 0x30, "30k and every 60k"      },
	{0x15, 0x01, 0x30, 0x20, "40k and every 80k"      },	
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x15, 0x01, 0xc0, 0xc0, "2"                      },
	{0x15, 0x01, 0xc0, 0x80, "3"                      },
	{0x15, 0x01, 0xc0, 0x40, "4"                      },
	{0x15, 0x01, 0xc0, 0x00, "Infinite"               },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo Drv2DIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xff, NULL                     },
	{0x15, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x14, 0x01, 0x07, 0x00, "4 Coins 1 Play"         },
	{0x14, 0x01, 0x07, 0x01, "3 Coins 1 Play"         },
	{0x14, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x14, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x14, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x14, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x14, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x14, 0x01, 0x07, 0x03, "1 Coin  5 Plays"        },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x14, 0x01, 0x38, 0x00, "4 Coins 1 Play"         },
	{0x14, 0x01, 0x38, 0x08, "3 Coins 1 Play"         },
	{0x14, 0x01, 0x38, 0x10, "2 Coins 1 Play"         },
	{0x14, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x14, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x14, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
	{0x14, 0x01, 0x38, 0x20, "1 Coin  4 Plays"        },
	{0x14, 0x01, 0x38, 0x18, "1 Coin  5 Plays"        },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x14, 0x01, 0x40, 0x40, "Upright"                },
	{0x14, 0x01, 0x40, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x14, 0x01, 0x80, 0x80, "Off"                    },
	{0x14, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Hurricane Kick"         },
	{0x15, 0x01, 0x08, 0x00, "Easy"                   },
	{0x15, 0x01, 0x08, 0x08, "Normal"                 },
	
	{0   , 0xfe, 0   , 4   , "Timer"                  },
	{0x15, 0x01, 0x30, 0x00, "60"                     },
	{0x15, 0x01, 0x30, 0x10, "65"                     },
	{0x15, 0x01, 0x30, 0x30, "70"                     },
	{0x15, 0x01, 0x30, 0x20, "80"                     },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x15, 0x01, 0xc0, 0xc0, "1"                      },
	{0x15, 0x01, 0xc0, 0x80, "2"                      },
	{0x15, 0x01, 0xc0, 0x40, "3"                      },
	{0x15, 0x01, 0xc0, 0x00, "4"                      },
};

STDDIPINFO(Drv2)

static struct BurnDIPInfo DdungeonDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0x9d, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 16  , "Coin A"                 },
	{0x14, 0x01, 0x0f, 0x03, "4 Coins 1 Play"         },
	{0x14, 0x01, 0x0f, 0x02, "3 Coins 1 Play"         },
	{0x14, 0x01, 0x0f, 0x07, "4 Coins 2 Plays"        },
	{0x14, 0x01, 0x0f, 0x01, "2 Coins 1 Play"         },
	{0x14, 0x01, 0x0f, 0x06, "3 Coins 2 Plays"        },
	{0x14, 0x01, 0x0f, 0x0b, "4 Coins 3 Plays"        },
	{0x14, 0x01, 0x0f, 0x0f, "4 Coins 4 Plays"        },
	{0x14, 0x01, 0x0f, 0x0a, "3 Coins 3 Plays"        },
	{0x14, 0x01, 0x0f, 0x05, "2 Coins 2 Plays"        },
	{0x14, 0x01, 0x0f, 0x00, "1 Coin  1 Play"         },
	{0x14, 0x01, 0x0f, 0x0e, "3 Coins 4 Plays"        },
	{0x14, 0x01, 0x0f, 0x09, "2 Coins 3 Plays"        },
	{0x14, 0x01, 0x0f, 0x0d, "2 Coins 4 Plays"        },
	{0x14, 0x01, 0x0f, 0x04, "1 Coin  2 Plays"        },
	{0x14, 0x01, 0x0f, 0x08, "1 Coin  3 Plays"        },
	{0x14, 0x01, 0x0f, 0x0c, "1 Coin  4 Plays"        },
	
	{0   , 0xfe, 0   , 16  , "Coin B"                 },
	{0x14, 0x01, 0xf0, 0x30, "4 Coins 1 Play"         },
	{0x14, 0x01, 0xf0, 0x20, "3 Coins 1 Play"         },
	{0x14, 0x01, 0xf0, 0x70, "4 Coins 2 Plays"        },
	{0x14, 0x01, 0xf0, 0x10, "2 Coins 1 Play"         },
	{0x14, 0x01, 0xf0, 0x60, "3 Coins 2 Plays"        },
	{0x14, 0x01, 0xf0, 0xb0, "4 Coins 3 Plays"        },
	{0x14, 0x01, 0xf0, 0xf0, "4 Coins 4 Plays"        },
	{0x14, 0x01, 0xf0, 0xa0, "3 Coins 3 Plays"        },
	{0x14, 0x01, 0xf0, 0x50, "2 Coins 2 Plays"        },
	{0x14, 0x01, 0xf0, 0x00, "1 Coin  1 Play"         },
	{0x14, 0x01, 0xf0, 0xe0, "3 Coins 4 Plays"        },
	{0x14, 0x01, 0xf0, 0x90, "2 Coins 3 Plays"        },
	{0x14, 0x01, 0xf0, 0xd0, "2 Coins 4 Plays"        },
	{0x14, 0x01, 0xf0, 0x40, "1 Coin  2 Plays"        },
	{0x14, 0x01, 0xf0, 0x80, "1 Coin  3 Plays"        },
	{0x14, 0x01, 0xf0, 0xc0, "1 Coin  4 Plays"        },
	
	// Dip 2	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x15, 0x01, 0x03, 0x00, "1"                      },
	{0x15, 0x01, 0x03, 0x01, "2"                      },
	{0x15, 0x01, 0x03, 0x02, "3"                      },
	{0x15, 0x01, 0x03, 0x03, "4"                      },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x15, 0x01, 0xf0, 0xf0, "Easy"                   },
	{0x15, 0x01, 0xf0, 0x90, "Medium"                 },
	{0x15, 0x01, 0xf0, 0x70, "Hard"                   },
	{0x15, 0x01, 0xf0, 0x00, "Hardest"                },
};

STDDIPINFO(Ddungeon)

static struct BurnDIPInfo DarktowrDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0x00, NULL                     },
	{0x15, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 16  , "Coin A"                 },
	{0x14, 0x01, 0x0f, 0x03, "4 Coins 1 Play"         },
	{0x14, 0x01, 0x0f, 0x02, "3 Coins 1 Play"         },
	{0x14, 0x01, 0x0f, 0x07, "4 Coins 2 Plays"        },
	{0x14, 0x01, 0x0f, 0x01, "2 Coins 1 Play"         },
	{0x14, 0x01, 0x0f, 0x06, "3 Coins 2 Plays"        },
	{0x14, 0x01, 0x0f, 0x0b, "4 Coins 3 Plays"        },
	{0x14, 0x01, 0x0f, 0x0f, "4 Coins 4 Plays"        },
	{0x14, 0x01, 0x0f, 0x0a, "3 Coins 3 Plays"        },
	{0x14, 0x01, 0x0f, 0x05, "2 Coins 2 Plays"        },
	{0x14, 0x01, 0x0f, 0x00, "1 Coin  1 Play"         },
	{0x14, 0x01, 0x0f, 0x0e, "3 Coins 4 Plays"        },
	{0x14, 0x01, 0x0f, 0x09, "2 Coins 3 Plays"        },
	{0x14, 0x01, 0x0f, 0x0d, "2 Coins 4 Plays"        },
	{0x14, 0x01, 0x0f, 0x04, "1 Coin  2 Plays"        },
	{0x14, 0x01, 0x0f, 0x08, "1 Coin  3 Plays"        },
	{0x14, 0x01, 0x0f, 0x0c, "1 Coin  4 Plays"        },
	
	{0   , 0xfe, 0   , 16  , "Coin B"                 },
	{0x14, 0x01, 0xf0, 0x30, "4 Coins 1 Play"         },
	{0x14, 0x01, 0xf0, 0x20, "3 Coins 1 Play"         },
	{0x14, 0x01, 0xf0, 0x70, "4 Coins 2 Plays"        },
	{0x14, 0x01, 0xf0, 0x10, "2 Coins 1 Play"         },
	{0x14, 0x01, 0xf0, 0x60, "3 Coins 2 Plays"        },
	{0x14, 0x01, 0xf0, 0xb0, "4 Coins 3 Plays"        },
	{0x14, 0x01, 0xf0, 0xf0, "4 Coins 4 Plays"        },
	{0x14, 0x01, 0xf0, 0xa0, "3 Coins 3 Plays"        },
	{0x14, 0x01, 0xf0, 0x50, "2 Coins 2 Plays"        },
	{0x14, 0x01, 0xf0, 0x00, "1 Coin  1 Play"         },
	{0x14, 0x01, 0xf0, 0xe0, "3 Coins 4 Plays"        },
	{0x14, 0x01, 0xf0, 0x90, "2 Coins 3 Plays"        },
	{0x14, 0x01, 0xf0, 0xd0, "2 Coins 4 Plays"        },
	{0x14, 0x01, 0xf0, 0x40, "1 Coin  2 Plays"        },
	{0x14, 0x01, 0xf0, 0x80, "1 Coin  3 Plays"        },
	{0x14, 0x01, 0xf0, 0xc0, "1 Coin  4 Plays"        },
};

STDDIPINFO(Darktowr)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "21j-1-5.26",    0x08000, 0x42045dfd, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "21j-2-3.25",    0x08000, 0x5779705e, BRF_ESS | BRF_PRG }, //  1
	{ "21j-3.24",      0x08000, 0x3bdea613, BRF_ESS | BRF_PRG }, //  2
	{ "21j-4-1.23",    0x08000, 0x728f87b9, BRF_ESS | BRF_PRG }, //  3
	
	{ "21jm-0.ic55",   0x04000, 0xf5232d03, BRF_ESS | BRF_PRG }, //  4	HD63701 Program Code
	
	{ "21j-0-1",       0x08000, 0x9efa95bb, BRF_ESS | BRF_PRG }, //  5	M6809 Program Code
	
	{ "21j-5",         0x08000, 0x7a8b8db4, BRF_GRA },	     //  6	Characters
	
	{ "21j-a",         0x10000, 0x574face3, BRF_GRA },	     //  7	Sprites
	{ "21j-b",         0x10000, 0x40507a76, BRF_GRA },	     //  8
	{ "21j-c",         0x10000, 0xbb0bc76f, BRF_GRA },	     //  9
	{ "21j-d",         0x10000, 0xcb4f231b, BRF_GRA },	     //  10
	{ "21j-e",         0x10000, 0xa0a0c261, BRF_GRA },	     //  11
	{ "21j-f",         0x10000, 0x6ba152f6, BRF_GRA },	     //  12
	{ "21j-g",         0x10000, 0x3220a0b6, BRF_GRA },	     //  13
	{ "21j-h",         0x10000, 0x65c7517d, BRF_GRA },	     //  14
	
	{ "21j-8",         0x10000, 0x7c435887, BRF_GRA },	     //  15	Tiles
	{ "21j-9",         0x10000, 0xc6640aed, BRF_GRA },	     //  16
	{ "21j-i",         0x10000, 0x5effb0a0, BRF_GRA },	     //  17
	{ "21j-j",         0x10000, 0x5fb42e7c, BRF_GRA },	     //  18
	
	{ "21j-6",         0x10000, 0x34755de3, BRF_GRA },	     //  19	Samples
	{ "21j-7",         0x10000, 0x904de6f8, BRF_GRA },	     //  20
	
	{ "21j-k-0",       0x00100, 0xfdb130a9, BRF_GRA },	     //  21	PROMs
	{ "21j-l-0",       0x00200, 0x46339529, BRF_GRA },	     //  22
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo DrvwRomDesc[] = {
	{ "21j-1.26",      0x08000, 0xae714964, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "21j-2-3.25",    0x08000, 0x5779705e, BRF_ESS | BRF_PRG }, //  1
	{ "21a-3.24",      0x08000, 0xdbf24897, BRF_ESS | BRF_PRG }, //  2
	{ "21j-4.23",      0x08000, 0x6c9f46fa, BRF_ESS | BRF_PRG }, //  3
	
	{ "21jm-0.ic55",   0x04000, 0xf5232d03, BRF_ESS | BRF_PRG }, //  4	HD63701 Program Code
	
	{ "21j-0-1",       0x08000, 0x9efa95bb, BRF_ESS | BRF_PRG }, //  5	M6809 Program Code
	
	{ "21j-5",         0x08000, 0x7a8b8db4, BRF_GRA },	     //  6	Characters
	
	{ "21j-a",         0x10000, 0x574face3, BRF_GRA },	     //  7	Sprites
	{ "21j-b",         0x10000, 0x40507a76, BRF_GRA },	     //  8
	{ "21j-c",         0x10000, 0xbb0bc76f, BRF_GRA },	     //  9
	{ "21j-d",         0x10000, 0xcb4f231b, BRF_GRA },	     //  10
	{ "21j-e",         0x10000, 0xa0a0c261, BRF_GRA },	     //  11
	{ "21j-f",         0x10000, 0x6ba152f6, BRF_GRA },	     //  12
	{ "21j-g",         0x10000, 0x3220a0b6, BRF_GRA },	     //  13
	{ "21j-h",         0x10000, 0x65c7517d, BRF_GRA },	     //  14
	
	{ "21j-8",         0x10000, 0x7c435887, BRF_GRA },	     //  15	Tiles
	{ "21j-9",         0x10000, 0xc6640aed, BRF_GRA },	     //  16
	{ "21j-i",         0x10000, 0x5effb0a0, BRF_GRA },	     //  17
	{ "21j-j",         0x10000, 0x5fb42e7c, BRF_GRA },	     //  18
	
	{ "21j-6",         0x10000, 0x34755de3, BRF_GRA },	     //  19	Samples
	{ "21j-7",         0x10000, 0x904de6f8, BRF_GRA },	     //  20
	
	{ "21j-k-0",       0x00100, 0xfdb130a9, BRF_GRA },	     //  21	PROMs
	{ "21j-l-0",       0x00200, 0x46339529, BRF_GRA },	     //  22
};

STD_ROM_PICK(Drvw)
STD_ROM_FN(Drvw)

static struct BurnRomInfo Drvw1RomDesc[] = {
	{ "e1-1.26",       0x08000, 0x4b951643, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "21a-2-4.25",    0x08000, 0x5cd67657, BRF_ESS | BRF_PRG }, //  1
	{ "21a-3.24",      0x08000, 0xdbf24897, BRF_ESS | BRF_PRG }, //  2
	{ "e4-1.23",       0x08000, 0xb1e26935, BRF_ESS | BRF_PRG }, //  3
	
	{ "21jm-0.ic55",   0x04000, 0xf5232d03, BRF_ESS | BRF_PRG }, //  4	HD63701 Program Code
	
	{ "21j-0-1",       0x08000, 0x9efa95bb, BRF_ESS | BRF_PRG }, //  5	M6809 Program Code
	
	{ "21j-5",         0x08000, 0x7a8b8db4, BRF_GRA },	     //  6	Characters
	
	{ "21j-a",         0x10000, 0x574face3, BRF_GRA },	     //  7	Sprites
	{ "21j-b",         0x10000, 0x40507a76, BRF_GRA },	     //  8
	{ "21j-c",         0x10000, 0xbb0bc76f, BRF_GRA },	     //  9
	{ "21j-d",         0x10000, 0xcb4f231b, BRF_GRA },	     //  10
	{ "21j-e",         0x10000, 0xa0a0c261, BRF_GRA },	     //  11
	{ "21j-f",         0x10000, 0x6ba152f6, BRF_GRA },	     //  12
	{ "21j-g",         0x10000, 0x3220a0b6, BRF_GRA },	     //  13
	{ "21j-h",         0x10000, 0x65c7517d, BRF_GRA },	     //  14
	
	{ "21j-8",         0x10000, 0x7c435887, BRF_GRA },	     //  15	Tiles
	{ "21j-9",         0x10000, 0xc6640aed, BRF_GRA },	     //  16
	{ "21j-i",         0x10000, 0x5effb0a0, BRF_GRA },	     //  17
	{ "21j-j",         0x10000, 0x5fb42e7c, BRF_GRA },	     //  18
	
	{ "21j-6",         0x10000, 0x34755de3, BRF_GRA },	     //  19	Samples
	{ "21j-7",         0x10000, 0x904de6f8, BRF_GRA },	     //  20
	
	{ "21j-k-0",       0x00100, 0xfdb130a9, BRF_GRA },	     //  21	PROMs
	{ "21j-l-0",       0x00200, 0x46339529, BRF_GRA },	     //  22
};

STD_ROM_PICK(Drvw1)
STD_ROM_FN(Drvw1)

static struct BurnRomInfo DrvuRomDesc[] = {
	{ "21a-1-5.26",    0x08000, 0xe24a6e11, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "21j-2-3.25",    0x08000, 0x5779705e, BRF_ESS | BRF_PRG }, //  1
	{ "21a-3.24",      0x08000, 0xdbf24897, BRF_ESS | BRF_PRG }, //  2
	{ "21a-4.23",      0x08000, 0x6ea16072, BRF_ESS | BRF_PRG }, //  3
	
	{ "21jm-0.ic55",   0x04000, 0xf5232d03, BRF_ESS | BRF_PRG }, //  4	HD63701 Program Code
	
	{ "21j-0-1",       0x08000, 0x9efa95bb, BRF_ESS | BRF_PRG }, //  5	M6809 Program Code
	
	{ "21j-5",         0x08000, 0x7a8b8db4, BRF_GRA },	     //  6	Characters
	
	{ "21j-a",         0x10000, 0x574face3, BRF_GRA },	     //  7	Sprites
	{ "21j-b",         0x10000, 0x40507a76, BRF_GRA },	     //  8
	{ "21j-c",         0x10000, 0xbb0bc76f, BRF_GRA },	     //  9
	{ "21j-d",         0x10000, 0xcb4f231b, BRF_GRA },	     //  10
	{ "21j-e",         0x10000, 0xa0a0c261, BRF_GRA },	     //  11
	{ "21j-f",         0x10000, 0x6ba152f6, BRF_GRA },	     //  12
	{ "21j-g",         0x10000, 0x3220a0b6, BRF_GRA },	     //  13
	{ "21j-h",         0x10000, 0x65c7517d, BRF_GRA },	     //  14
	
	{ "21j-8",         0x10000, 0x7c435887, BRF_GRA },	     //  15	Tiles
	{ "21j-9",         0x10000, 0xc6640aed, BRF_GRA },	     //  16
	{ "21j-i",         0x10000, 0x5effb0a0, BRF_GRA },	     //  17
	{ "21j-j",         0x10000, 0x5fb42e7c, BRF_GRA },	     //  18
	
	{ "21j-6",         0x10000, 0x34755de3, BRF_GRA },	     //  19	Samples
	{ "21j-7",         0x10000, 0x904de6f8, BRF_GRA },	     //  20
	
	{ "21j-k-0",       0x00100, 0xfdb130a9, BRF_GRA },	     //  21	PROMs
	{ "21j-l-0",       0x00200, 0x46339529, BRF_GRA },	     //  22
};

STD_ROM_PICK(Drvu)
STD_ROM_FN(Drvu)

static struct BurnRomInfo DrvuaRomDesc[] = {
	{ "21a-1",         0x08000, 0x1d625008, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "21a-2_4",       0x08000, 0x5cd67657, BRF_ESS | BRF_PRG }, //  1
	{ "21a-3",         0x08000, 0xdbf24897, BRF_ESS | BRF_PRG }, //  2
	{ "21a-4_2",       0x08000, 0x9b019598, BRF_ESS | BRF_PRG }, //  3
	
	{ "21jm-0.ic55",   0x04000, 0xf5232d03, BRF_ESS | BRF_PRG }, //  4	HD63701 Program Code
	
	{ "21j-0-1",       0x08000, 0x9efa95bb, BRF_ESS | BRF_PRG }, //  5	M6809 Program Code
	
	{ "21j-5",         0x08000, 0x7a8b8db4, BRF_GRA },	     //  6	Characters
	
	{ "21j-a",         0x10000, 0x574face3, BRF_GRA },	     //  7	Sprites
	{ "21j-b",         0x10000, 0x40507a76, BRF_GRA },	     //  8
	{ "21j-c",         0x10000, 0xbb0bc76f, BRF_GRA },	     //  9
	{ "21j-d",         0x10000, 0xcb4f231b, BRF_GRA },	     //  10
	{ "21j-e",         0x10000, 0xa0a0c261, BRF_GRA },	     //  11
	{ "21j-f",         0x10000, 0x6ba152f6, BRF_GRA },	     //  12
	{ "21j-g",         0x10000, 0x3220a0b6, BRF_GRA },	     //  13
	{ "21j-h",         0x10000, 0x65c7517d, BRF_GRA },	     //  14
	
	{ "21j-8",         0x10000, 0x7c435887, BRF_GRA },	     //  15	Tiles
	{ "21j-9",         0x10000, 0xc6640aed, BRF_GRA },	     //  16
	{ "21j-i",         0x10000, 0x5effb0a0, BRF_GRA },	     //  17
	{ "21j-j",         0x10000, 0x5fb42e7c, BRF_GRA },	     //  18
	
	{ "21j-6",         0x10000, 0x34755de3, BRF_GRA },	     //  19	Samples
	{ "21j-7",         0x10000, 0x904de6f8, BRF_GRA },	     //  20
	
	{ "21j-k-0",       0x00100, 0xfdb130a9, BRF_GRA },	     //  21	PROMs
	{ "21j-l-0",       0x00200, 0x46339529, BRF_GRA },	     //  22
};

STD_ROM_PICK(Drvua)
STD_ROM_FN(Drvua)

static struct BurnRomInfo DrvubRomDesc[] = {
	{ "21a-1_6.bin",   0x08000, 0xf354b0e1, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "21a-2_4",       0x08000, 0x5cd67657, BRF_ESS | BRF_PRG }, //  1
	{ "21a-3",         0x08000, 0xdbf24897, BRF_ESS | BRF_PRG }, //  2
	{ "21a-4_2",       0x08000, 0x9b019598, BRF_ESS | BRF_PRG }, //  3
	
	{ "21jm-0.ic55",   0x04000, 0xf5232d03, BRF_ESS | BRF_PRG }, //  4	HD63701 Program Code
	
	{ "21j-0-1",       0x08000, 0x9efa95bb, BRF_ESS | BRF_PRG }, //  5	M6809 Program Code
	
	{ "21j-5",         0x08000, 0x7a8b8db4, BRF_GRA },	     //  6	Characters
	
	{ "21j-a",         0x10000, 0x574face3, BRF_GRA },	     //  7	Sprites
	{ "21j-b",         0x10000, 0x40507a76, BRF_GRA },	     //  8
	{ "21j-c",         0x10000, 0xbb0bc76f, BRF_GRA },	     //  9
	{ "21j-d",         0x10000, 0xcb4f231b, BRF_GRA },	     //  10
	{ "21j-e",         0x10000, 0xa0a0c261, BRF_GRA },	     //  11
	{ "21j-f",         0x10000, 0x6ba152f6, BRF_GRA },	     //  12
	{ "21j-g",         0x10000, 0x3220a0b6, BRF_GRA },	     //  13
	{ "21j-h",         0x10000, 0x65c7517d, BRF_GRA },	     //  14
	
	{ "21j-8",         0x10000, 0x7c435887, BRF_GRA },	     //  15	Tiles
	{ "21j-9",         0x10000, 0xc6640aed, BRF_GRA },	     //  16
	{ "21j-i",         0x10000, 0x5effb0a0, BRF_GRA },	     //  17
	{ "21j-j",         0x10000, 0x5fb42e7c, BRF_GRA },	     //  18
	
	{ "21j-6",         0x10000, 0x34755de3, BRF_GRA },	     //  19	Samples
	{ "21j-7",         0x10000, 0x904de6f8, BRF_GRA },	     //  20
	
	{ "21j-k-0",       0x00100, 0xfdb130a9, BRF_GRA },	     //  21	PROMs
	{ "21j-l-0",       0x00200, 0x46339529, BRF_GRA },	     //  22
};

STD_ROM_PICK(Drvub)
STD_ROM_FN(Drvub)

static struct BurnRomInfo Drvb2RomDesc[] = {
	{ "b2_4.bin",      0x08000, 0x668dfa19, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "b2_5.bin",      0x08000, 0x5779705e, BRF_ESS | BRF_PRG }, //  1
	{ "b2_6.bin",      0x08000, 0x3bdea613, BRF_ESS | BRF_PRG }, //  2
	{ "b2_7.bin",      0x08000, 0x728f87b9, BRF_ESS | BRF_PRG }, //  3
	
	{ "63701.bin",     0x04000, 0xf5232d03, BRF_ESS | BRF_PRG }, //  4	HD63701 Program Code
	
	{ "b2_3.bin",      0x08000, 0x9efa95bb, BRF_ESS | BRF_PRG }, //  5	M6809 Program Code
	
	{ "b2_8.bin",      0x08000, 0x7a8b8db4, BRF_GRA },	     //  6	Characters
	
	{ "11.bin",        0x10000, 0x574face3, BRF_GRA },	     //  7	Sprites
	{ "12.bin",        0x10000, 0x40507a76, BRF_GRA },	     //  8
	{ "13.bin",        0x10000, 0xc8b91e17, BRF_GRA },	     //  9
	{ "14.bin",        0x10000, 0xcb4f231b, BRF_GRA },	     //  10
	{ "15.bin",        0x10000, 0xa0a0c261, BRF_GRA },	     //  11
	{ "16.bin",        0x10000, 0x6ba152f6, BRF_GRA },	     //  12
	{ "17.bin",        0x10000, 0x3220a0b6, BRF_GRA },	     //  13
	{ "18.bin",        0x10000, 0x65c7517d, BRF_GRA },	     //  14
	
	{ "9.bin",         0x10000, 0x7c435887, BRF_GRA },	     //  15	Tiles
	{ "10.bin",        0x10000, 0xc6640aed, BRF_GRA },	     //  16
	{ "19.bin",        0x10000, 0x22d65df2, BRF_GRA },	     //  17
	{ "20.bin",        0x10000, 0x5fb42e7c, BRF_GRA },	     //  18
	
	{ "b2_1.bin",      0x10000, 0x34755de3, BRF_GRA },	     //  19	Samples
	{ "2.bin",         0x10000, 0x904de6f8, BRF_GRA },	     //  20
	
	{ "21j-k-0",       0x00100, 0xfdb130a9, BRF_GRA },	     //  21	PROMs
	{ "21j-l-0",       0x00200, 0x46339529, BRF_GRA },	     //  22
};

STD_ROM_PICK(Drvb2)
STD_ROM_FN(Drvb2)

static struct BurnRomInfo DrvbRomDesc[] = {
	{ "21j-1.26",      0x08000, 0xae714964, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "21j-2-3.25",    0x08000, 0x5779705e, BRF_ESS | BRF_PRG }, //  1
	{ "21a-3.24",      0x08000, 0xdbf24897, BRF_ESS | BRF_PRG }, //  2
	{ "21j-4.23",      0x08000, 0x6c9f46fa, BRF_ESS | BRF_PRG }, //  3
	
	{ "ic38",          0x04000, 0x6a6a0325, BRF_ESS | BRF_PRG }, //  4	HD6309 Program Code
	
	{ "21j-0-1",       0x08000, 0x9efa95bb, BRF_ESS | BRF_PRG }, //  5	M6809 Program Code
	
	{ "21j-5",         0x08000, 0x7a8b8db4, BRF_GRA },	     //  6	Characters
	
	{ "21j-a",         0x10000, 0x574face3, BRF_GRA },	     //  7	Sprites
	{ "21j-b",         0x10000, 0x40507a76, BRF_GRA },	     //  8
	{ "21j-c",         0x10000, 0xbb0bc76f, BRF_GRA },	     //  9
	{ "21j-d",         0x10000, 0xcb4f231b, BRF_GRA },	     //  10
	{ "21j-e",         0x10000, 0xa0a0c261, BRF_GRA },	     //  11
	{ "21j-f",         0x10000, 0x6ba152f6, BRF_GRA },	     //  12
	{ "21j-g",         0x10000, 0x3220a0b6, BRF_GRA },	     //  13
	{ "21j-h",         0x10000, 0x65c7517d, BRF_GRA },	     //  14
	
	{ "21j-8",         0x10000, 0x7c435887, BRF_GRA },	     //  15	Tiles
	{ "21j-9",         0x10000, 0xc6640aed, BRF_GRA },	     //  16
	{ "21j-i",         0x10000, 0x5effb0a0, BRF_GRA },	     //  17
	{ "21j-j",         0x10000, 0x5fb42e7c, BRF_GRA },	     //  18
	
	{ "21j-6",         0x10000, 0x34755de3, BRF_GRA },	     //  19	Samples
	{ "21j-7",         0x10000, 0x904de6f8, BRF_GRA },	     //  20
	
	{ "21j-k-0",       0x00100, 0xfdb130a9, BRF_GRA },	     //  21	PROMs
	{ "21j-l-0",       0x00200, 0x46339529, BRF_GRA },	     //  22
};

STD_ROM_PICK(Drvb)
STD_ROM_FN(Drvb)

static struct BurnRomInfo DrvbaRomDesc[] = {
	{ "5.bin",         0x08000, 0xae714964, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "4.bin",         0x10000, 0x48045762, BRF_ESS | BRF_PRG }, //  1
	{ "3.bin",         0x08000, 0xdbf24897, BRF_ESS | BRF_PRG }, //  2
	
	{ "2_32.bin",      0x04000, 0x67875473, BRF_ESS | BRF_PRG }, //  3	M6803 Program Code
	
	{ "6.bin",         0x08000, 0x9efa95bb, BRF_ESS | BRF_PRG }, //  4	M6809 Program Code
	
	{ "1.bin",         0x08000, 0x7a8b8db4, BRF_GRA },	     //  5	Characters
	
	{ "21j-a",         0x10000, 0x574face3, BRF_GRA },	     //  6	Sprites
	{ "21j-b",         0x10000, 0x40507a76, BRF_GRA },	     //  7
	{ "21j-c",         0x10000, 0xbb0bc76f, BRF_GRA },	     //  8
	{ "21j-d",         0x10000, 0xcb4f231b, BRF_GRA },	     //  9
	{ "21j-e",         0x10000, 0xa0a0c261, BRF_GRA },	     //  10
	{ "21j-f",         0x10000, 0x6ba152f6, BRF_GRA },	     //  11
	{ "21j-g",         0x10000, 0x3220a0b6, BRF_GRA },	     //  12
	{ "21j-h",         0x10000, 0x65c7517d, BRF_GRA },	     //  13
	
	{ "21j-8",         0x10000, 0x7c435887, BRF_GRA },	     //  14	Tiles
	{ "21j-9",         0x10000, 0xc6640aed, BRF_GRA },	     //  15
	{ "21j-i",         0x10000, 0x5effb0a0, BRF_GRA },	     //  16
	{ "21j-j",         0x10000, 0x5fb42e7c, BRF_GRA },	     //  17
	
	{ "8.bin",         0x10000, 0x34755de3, BRF_GRA },	     //  18	Samples
	{ "7.bin",         0x10000, 0xf9311f72, BRF_GRA },	     //  19
	
	{ "21j-k-0",       0x00100, 0xfdb130a9, BRF_GRA },	     //  20	PROMs
	{ "21j-l-0",       0x00200, 0x46339529, BRF_GRA },	     //  21
};

STD_ROM_PICK(Drvba)
STD_ROM_FN(Drvba)

static struct BurnRomInfo Drv2RomDesc[] = {
	{ "26a9-04.bin",   0x08000, 0xf2cfc649, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "26aa-03.bin",   0x08000, 0x44dd5d4b, BRF_ESS | BRF_PRG }, //  1
	{ "26ab-0.bin",    0x08000, 0x49ddddcd, BRF_ESS | BRF_PRG }, //  2
	{ "26ac-0e.63",    0x08000, 0x57acad2c, BRF_ESS | BRF_PRG }, //  3
	
	{ "26ae-0.bin",    0x10000, 0xea437867, BRF_ESS | BRF_PRG }, //  4	Z80 #1 Program Code
	
	{ "26ad-0.bin",    0x08000, 0x75e36cd6, BRF_ESS | BRF_PRG }, //  5	Z80 #2 Program Code
	
	{ "26a8-0e.19",    0x10000, 0x4e80cd36, BRF_GRA },	     //  6	Characters
	
	{ "26j0-0.bin",    0x20000, 0xdb309c84, BRF_GRA },	     //  7	Sprites
	{ "26j1-0.bin",    0x20000, 0xc3081e0c, BRF_GRA },	     //  8
	{ "26af-0.bin",    0x20000, 0x3a615aad, BRF_GRA },	     //  9
	{ "26j2-0.bin",    0x20000, 0x589564ae, BRF_GRA },	     //  10
	{ "26j3-0.bin",    0x20000, 0xdaf040d6, BRF_GRA },	     //  11
	{ "26a10-0.bin",   0x20000, 0x6d16d889, BRF_GRA },	     //  12
	
	{ "26j4-0.bin",    0x20000, 0xa8c93e76, BRF_GRA },	     //  13	Tiles
	{ "26j5-0.bin",    0x20000, 0xee555237, BRF_GRA },	     //  14
	
	{ "26j6-0.bin",    0x20000, 0xa84b2a29, BRF_GRA },	     //  15	Samples
	{ "26j7-0.bin",    0x20000, 0xbc6a48d5, BRF_GRA },	     //  16
	
	{ "prom.16",       0x00200, 0x46339529, BRF_GRA },	     //  17	PROMs
};

STD_ROM_PICK(Drv2)
STD_ROM_FN(Drv2)

static struct BurnRomInfo Drv2uRomDesc[] = {
	{ "26a9-04.bin",   0x08000, 0xf2cfc649, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "26aa-03.bin",   0x08000, 0x44dd5d4b, BRF_ESS | BRF_PRG }, //  1
	{ "26ab-0.bin",    0x08000, 0x49ddddcd, BRF_ESS | BRF_PRG }, //  2
	{ "26ac-02.bin",   0x08000, 0x097eaf26, BRF_ESS | BRF_PRG }, //  3
	
	{ "26ae-0.bin",    0x10000, 0xea437867, BRF_ESS | BRF_PRG }, //  4	Z80 #1 Program Code
	
	{ "26ad-0.bin",    0x08000, 0x75e36cd6, BRF_ESS | BRF_PRG }, //  5	Z80 #2 Program Code
	
	{ "26a8-0.bin",    0x10000, 0x3ad1049c, BRF_GRA },	     //  6	Characters
	
	{ "26j0-0.bin",    0x20000, 0xdb309c84, BRF_GRA },	     //  7	Sprites
	{ "26j1-0.bin",    0x20000, 0xc3081e0c, BRF_GRA },	     //  8
	{ "26af-0.bin",    0x20000, 0x3a615aad, BRF_GRA },	     //  9
	{ "26j2-0.bin",    0x20000, 0x589564ae, BRF_GRA },	     //  10
	{ "26j3-0.bin",    0x20000, 0xdaf040d6, BRF_GRA },	     //  11
	{ "26a10-0.bin",   0x20000, 0x6d16d889, BRF_GRA },	     //  12
	
	{ "26j4-0.bin",    0x20000, 0xa8c93e76, BRF_GRA },	     //  13	Tiles
	{ "26j5-0.bin",    0x20000, 0xee555237, BRF_GRA },	     //  14
	
	{ "26j6-0.bin",    0x20000, 0xa84b2a29, BRF_GRA },	     //  15	Samples
	{ "26j7-0.bin",    0x20000, 0xbc6a48d5, BRF_GRA },	     //  16
	
	{ "prom.16",       0x00200, 0x46339529, BRF_GRA },	     //  17	PROMs
};

STD_ROM_PICK(Drv2u)
STD_ROM_FN(Drv2u)

static struct BurnRomInfo DdungeonRomDesc[] = {
	{ "dd26.26",       0x08000, 0xa6e7f608, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "dd25.25",       0x08000, 0x922e719c, BRF_ESS | BRF_PRG }, //  1
	
	{ "63701.bin",     0x04000, 0xf5232d03, BRF_ESS | BRF_PRG }, //  2	HD63701 Program Code
	
	{ "dd30.30",       0x08000, 0xef1af99a, BRF_ESS | BRF_PRG }, //  3	M6809 Program Code
	
	{ "dd_mcu.bin",    0x00800, 0x34cbb2d3, BRF_ESS | BRF_PRG }, //  4	M68705 MCU Program Code
	
	{ "dd20.20",       0x08000, 0xd976b78d, BRF_GRA },	     //  5	Characters
	
	{ "dd117.117",     0x08000, 0xe912ca81, BRF_GRA },	     //  6	Sprites
	{ "dd113.113",     0x08000, 0x43264ad8, BRF_GRA },	     //  7
		
	{ "dd78.78",       0x08000, 0x3deacae9, BRF_GRA },	     //  8	Tiles
	{ "dd109.109",     0x08000, 0x5a2f31eb, BRF_GRA },	     //  9
	
	{ "21j-6",         0x10000, 0x34755de3, BRF_GRA },	     //  10	Samples
	{ "21j-7",         0x10000, 0x904de6f8, BRF_GRA },	     //  11
	
	{ "21j-k-0",       0x00100, 0xfdb130a9, BRF_GRA },	     //  12	PROMs
	{ "21j-l-0",       0x00200, 0x46339529, BRF_GRA },	     //  13
};

STD_ROM_PICK(Ddungeon)
STD_ROM_FN(Ddungeon)

static struct BurnRomInfo DarktowrRomDesc[] = {
	{ "dt.26",         0x08000, 0x8134a472, BRF_ESS | BRF_PRG }, //  0	HD6309 Program Code
	{ "21j-2-3.25",    0x08000, 0x5779705e, BRF_ESS | BRF_PRG }, //  1
	{ "dt.24",         0x08000, 0x523a5413, BRF_ESS | BRF_PRG }, //  2
	
	{ "63701.bin",     0x04000, 0xf5232d03, BRF_ESS | BRF_PRG }, //  3	HD63701 Program Code
	
	{ "21j-0-1",       0x08000, 0x9efa95bb, BRF_ESS | BRF_PRG }, //  4	M6809 Program Code
	
	{ "68705prt.mcu",  0x00800, 0x34cbb2d3, BRF_ESS | BRF_PRG }, //  5	M68705 MCU Program Code
	
	{ "dt.20",         0x08000, 0x860b0298, BRF_GRA },	     //  6	Characters
	
	{ "dt.117",        0x10000, 0x750dd0fa, BRF_GRA },	     //  7	Sprites
	{ "dt.116",        0x10000, 0x22cfa87b, BRF_GRA },	     //  8
	{ "dt.115",        0x10000, 0x8a9f1c34, BRF_GRA },	     //  9
	{ "21j-d",         0x10000, 0xcb4f231b, BRF_GRA },	     //  10
	{ "dt.113",        0x10000, 0x7b4bbf9c, BRF_GRA },	     //  11
	{ "dt.112",        0x10000, 0xdf3709d4, BRF_GRA },	     //  12
	{ "dt.111",        0x10000, 0x59032154, BRF_GRA },	     //  13
	{ "21j-h",         0x10000, 0x65c7517d, BRF_GRA },	     //  14
	
	{ "dt.78",         0x10000, 0x72c15604, BRF_GRA },	     //  15	Tiles
	{ "21j-9",         0x10000, 0xc6640aed, BRF_GRA },	     //  16
	{ "dt.109",        0x10000, 0x15bdcb62, BRF_GRA },	     //  17
	{ "21j-j",         0x10000, 0x5fb42e7c, BRF_GRA },	     //  18
	
	{ "21j-6",         0x10000, 0x34755de3, BRF_GRA },	     //  19	Samples
	{ "21j-7",         0x10000, 0x904de6f8, BRF_GRA },	     //  20
	
	{ "21j-k-0",       0x00100, 0xfdb130a9, BRF_GRA },	     //  21	PROMs
	{ "21j-l-0",       0x00200, 0x46339529, BRF_GRA },	     //  22
};

STD_ROM_PICK(Darktowr)
STD_ROM_FN(Darktowr)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvHD6309Rom           = Next; Next += 0x30000;
	DrvSubCPURom           = Next; Next += 0x04000;
	DrvSoundCPURom         = Next; Next += 0x08000;
	DrvMSM5205Rom          = Next; Next += 0x20000;

	RamStart               = Next;

	DrvHD6309Ram           = Next; Next += 0x01000;
	DrvSubCPURam           = Next; Next += 0x00fd0;
	DrvSoundCPURam         = Next; Next += 0x01000;
	DrvFgVideoRam          = Next; Next += 0x00800;
	DrvSpriteRam           = Next; Next += 0x01000;
	DrvBgVideoRam          = Next; Next += 0x00800;
	DrvPaletteRam1         = Next; Next += 0x00200;
	DrvPaletteRam2         = Next; Next += 0x00200;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x0400 * 8 * 8;
	DrvTiles               = Next; Next += 0x0800 * 16 * 16;
	DrvSprites             = Next; Next += 0x1000 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00180 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 Drv2MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvHD6309Rom           = Next; Next += 0x30000;
	DrvSubCPURom           = Next; Next += 0x10000;
	DrvSoundCPURom         = Next; Next += 0x08000;
	MSM6295ROM             = Next; Next += 0x40000;

	RamStart               = Next;

	DrvHD6309Ram           = Next; Next += 0x01800;
	DrvSoundCPURam         = Next; Next += 0x00800;
	DrvFgVideoRam          = Next; Next += 0x00800;
	DrvSpriteRam           = Next; Next += 0x01000;
	DrvBgVideoRam          = Next; Next += 0x00800;
	DrvPaletteRam1         = Next; Next += 0x00200;
	DrvPaletteRam2         = Next; Next += 0x00200;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x0800 * 8 * 8;
	DrvTiles               = Next; Next += 0x0800 * 16 * 16;
	DrvSprites             = Next; Next += 0x1800 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00180 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DarktowrMemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvHD6309Rom           = Next; Next += 0x30000;
	DrvSubCPURom           = Next; Next += 0x04000;
	DrvSoundCPURom         = Next; Next += 0x08000;
	DrvMCURom              = Next; Next += 0x00800;
	DrvMSM5205Rom          = Next; Next += 0x20000;

	RamStart               = Next;

	DrvHD6309Ram           = Next; Next += 0x01000;
	DrvSubCPURam           = Next; Next += 0x00fd0;
	DrvSoundCPURam         = Next; Next += 0x01000;
	DrvMCURam              = Next; Next += 0x00078;
	DrvMCUPorts            = Next; Next += 0x00008;
	DrvFgVideoRam          = Next; Next += 0x00800;
	DrvSpriteRam           = Next; Next += 0x01000;
	DrvBgVideoRam          = Next; Next += 0x00800;
	DrvPaletteRam1         = Next; Next += 0x00200;
	DrvPaletteRam2         = Next; Next += 0x00200;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x0400 * 8 * 8;
	DrvTiles               = Next; Next += 0x0800 * 16 * 16;
	DrvSprites             = Next; Next += 0x1000 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00180 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	HD6309Open(0);
	HD6309Reset();
	HD6309Close();
	
	if (DrvSubCPUType == DD_CPU_TYPE_HD63701) {
		HD63701Reset();
	}
	
	if (DrvSubCPUType == DD_CPU_TYPE_HD6309) {
		HD6309Open(1);
		HD6309Reset();
		HD6309Close();
	}
	
	if (DrvSubCPUType == DD_CPU_TYPE_M6803) {
		M6803Reset();
	}
	
	if (DrvSubCPUType == DD_CPU_TYPE_Z80) {
		ZetOpen(0);
		ZetReset();
		ZetClose();
	}
	
	if (DrvSoundCPUType == DD_CPU_TYPE_M6809) {
		M6809Open(0);
		M6809Reset();
		M6809Close();
		
		MSM5205Reset();
	}
	
	if (DrvSoundCPUType == DD_CPU_TYPE_Z80) {
		ZetOpen(1);
		ZetReset();
		ZetClose();
		
		MSM6295Reset(0);
	}
	
	if (DrvGameType == DD_GAME_DARKTOWR) {
		m68705Reset();
	}
	
	BurnYM2151Reset();	
	
	DrvRomBank = 0;
	DrvVBlank = 0;
	DrvSubCPUBusy = 1;
	DrvSoundLatch = 0;
	DrvScrollXHi = 0;
	DrvScrollYHi = 0;
	DrvScrollXLo = 0;
	DrvScrollYLo = 0;
	
	DrvADPCMIdle[0] = 1;
	DrvADPCMIdle[1] = 1;
	DrvADPCMPos[0] = 0;
	DrvADPCMPos[1] = 0;
	DrvADPCMEnd[0] = 0;
	DrvADPCMEnd[1] = 0;
	DrvADPCMData[0] = -1;
	DrvADPCMData[1] = -1;
	
	return 0;
}

UINT8 DrvDdragonHD6309ReadByte(UINT16 Address)
{
	if (Address >= 0x2000 && Address <= 0x2fff) {
		if (Address == 0x2049 && HD6309GetPC() == 0x6261 && DrvSpriteRam[0x0049] == 0x1f) return 0x01;
		return DrvSpriteRam[Address - 0x2000];
	}
	
	if (DrvGameType == DD_GAME_DARKTOWR && Address >= 0x4000 && Address <= 0x7fff) {
		UINT32 Offset = Address - 0x4000;
		
		if (Offset == 0x1401 || Offset == 0x0001) return DrvMCUPorts[0];
		return 0xff;
	}
	
	switch (Address) {
		case 0x3800: {
			return DrvInput[0];
		}
		
		case 0x3801: {
			return DrvInput[1];
		}
		
		case 0x3802: {
			return DrvInput[2] | ((DrvVBlank) ? 0x08 : 0) | (DrvSubCPUBusy ? 0x10 : 0);
		}
		
		case 0x3803: {
			return DrvDip[0];
		}
		
		case 0x3804: {
			return DrvDip[1];
		}
		
		case 0x380b: {
			// ???
			return 0;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("HD6309 Read Byte -> %04X\n"), Address);
	
	return 0;
}

void DrvDdragonHD6309WriteByte(UINT16 Address, UINT8 Data)
{
	if (DrvGameType == DD_GAME_DARKTOWR && Address >= 0x4000 && Address <= 0x7fff) {
		UINT32 Offset = Address - 0x4000;
		
		if (Offset == 0x1400 || Offset == 0x0000) {
			DrvMCUPorts[1] = BITSWAP08(Data, 0, 1, 2, 3, 4, 5, 6, 7);
		}
		return;
	}
	
	switch (Address) {
		case 0x3808: {
			UINT8 DrvOldRomBank = DrvRomBank;
			DrvRomBank = (Data & 0xe0) >> 5;
			HD6309MapMemory(DrvHD6309Rom + 0x8000 + (DrvRomBank * 0x4000), 0x4000, 0x7fff, M6809_ROM);
			
			DrvScrollXHi = (Data & 0x01) << 8;
			DrvScrollYHi = (Data & 0x02) << 7;
			
			if (Data & 0x10) {
				DrvSubCPUBusy = 0;
			} else {
				if (DrvSubCPUBusy == 0) {
					if (DrvSubCPUType == DD_CPU_TYPE_HD63701) {
						HD63701SetIRQLine(HD63701_INPUT_LINE_NMI, HD63701_IRQSTATUS_ACK);
					}
				
					if (DrvSubCPUType == DD_CPU_TYPE_HD6309) {
						HD6309Close();
						HD6309Open(1);
						HD6309SetIRQLine(HD6309_INPUT_LINE_NMI, HD6309_IRQSTATUS_ACK);
						HD6309Close();
						HD6309Open(0);
					}
				
					if (DrvSubCPUType == DD_CPU_TYPE_M6803) {
						M6803SetIRQLine(M6803_INPUT_LINE_NMI, M6803_IRQSTATUS_ACK);
					}
				
					if (DrvSubCPUType == DD_CPU_TYPE_Z80) {
						ZetOpen(0);
						ZetNmi();
						ZetClose();
					}
				}
			}
			
			if (DrvGameType == DD_GAME_DARKTOWR) {
				if (DrvRomBank == 4 && DrvOldRomBank != 4) {
					HD6309MemCallback(0x4000, 0x7fff, HD6309_RAM);
				} else {
					if (DrvRomBank != 4 && DrvOldRomBank == 4) {
						HD6309MapMemory(DrvHD6309Rom + 0x8000 + (DrvRomBank * 0x4000), 0x4000, 0x7fff, M6809_ROM);
					}
				}
			}			
			
			return;			
		}
		
		case 0x3809: {
			DrvScrollXLo = Data;
			return;
		}
		
		case 0x380a: {
			DrvScrollYLo = Data;
			return;
		}
		
		case 0x380b: {
			HD6309SetIRQLine(HD6309_INPUT_LINE_NMI, HD6309_IRQSTATUS_NONE);
			return;
		}
		
		case 0x380c: {
			HD6309SetIRQLine(HD6309_FIRQ_LINE, HD6309_IRQSTATUS_NONE);
			return;
		}

		case 0x380d: {
			HD6309SetIRQLine(HD6309_IRQ_LINE, HD6309_IRQSTATUS_NONE);
			return;
		}
		
		case 0x380e: {
			DrvSoundLatch = Data;
			if (DrvSoundCPUType == DD_CPU_TYPE_M6809) {
				M6809Open(0);
				M6809SetIRQLine(M6809_IRQ_LINE, M6809_IRQSTATUS_ACK);
				M6809Close();
			}
			
			if (DrvSoundCPUType == DD_CPU_TYPE_Z80) {
				ZetOpen(1);
				ZetNmi();
				ZetClose();
			}
			return;
		}
		
		case 0x380f: {
			// ???
			return;
		}
	}	
	
	bprintf(PRINT_NORMAL, _T("HD6309 Write Byte -> %04X, %02X\n"), Address, Data);
}

UINT8 DrvDdragonHD63701ReadByte(UINT16 Address)
{
	if (Address >= 0x0020 && Address <= 0x0fff) {
		return DrvSubCPURam[Address - 0x0020];
	}
	
	if (Address >= 0x8000 && Address <= 0x8fff) {
		if (Address == 0x8049 && HD63701GetPC() == 0x6261 && DrvSpriteRam[0x0049] == 0x1f) return 0x01;
		return DrvSpriteRam[Address - 0x8000];
	}
	
	bprintf(PRINT_NORMAL, _T("M6800 Read Byte -> %04X\n"), Address);
	
	return 0;
}

void DrvDdragonHD63701WriteByte(UINT16 Address, UINT8 Data)
{
	if (Address <= 0x001f) {
		if (Address == 0x17) {
			if (Data & 3) {
				HD6309Open(0);
				HD6309SetIRQLine(HD6309_IRQ_LINE, HD6309_IRQSTATUS_ACK);
				HD6309Close();
				
				HD63701SetIRQLine(HD63701_INPUT_LINE_NMI, HD63701_IRQSTATUS_NONE);
			}
		}
		return;
	}
	
	if (Address >= 0x0020 && Address <= 0x0fff) {
		DrvSubCPURam[Address - 0x0020] = Data;
		return;
	}
	
	if (Address >= 0x8000 && Address <= 0x8fff) {
		if (Address == 0x8000) DrvSubCPUBusy = 1;
		DrvSpriteRam[Address - 0x8000] = Data;
		return;
	}
	
	bprintf(PRINT_NORMAL, _T("M6800 Write Byte -> %04X, %02X\n"), Address, Data);
}

UINT8 DrvDdragonbSubHD6309ReadByte(UINT16 Address)
{
	if (Address >= 0x0020 && Address <= 0x0fff) {
		return DrvSubCPURam[Address - 0x0020];
	}
	
	if (Address >= 0x8000 && Address <= 0x8fff) {
		return DrvSpriteRam[Address - 0x8000];
	}
	
	bprintf(PRINT_NORMAL, _T("Sub HD6309 Read Byte -> %04X\n"), Address);
	
	return 0;
}

void DrvDdragonbSubHD6309WriteByte(UINT16 Address, UINT8 Data)
{
	if (Address <= 0x001f) {
		if (Address == 0x17) {
			if (Data & 3) {
				HD6309Close();
				
				HD6309Open(0);
				HD6309SetIRQLine(HD6309_IRQ_LINE, HD6309_IRQSTATUS_ACK);
				HD6309Close();
				
				HD6309Open(1);
				HD6309SetIRQLine(HD6309_INPUT_LINE_NMI, HD6309_IRQSTATUS_NONE);
			}
		}
		return;
	}
	
	if (Address >= 0x0020 && Address <= 0x0fff) {
		DrvSubCPURam[Address - 0x0020] = Data;
		return;
	}
	
	if (Address >= 0x8000 && Address <= 0x8fff) {
		if (Address == 0x8000) DrvSubCPUBusy = 1;
		DrvSpriteRam[Address - 0x8000] = Data;
		return;
	}
	
	bprintf(PRINT_NORMAL, _T("Sub HD6309 Write Byte -> %04X, %02X\n"), Address, Data);
}

UINT8 DrvDdragonbaM6803ReadByte(UINT16 Address)
{
	if (Address >= 0x0020 && Address <= 0x0fff) {
		return DrvSubCPURam[Address - 0x0020];
	}
	
	if (Address >= 0x8000 && Address <= 0x8fff) {
		return DrvSpriteRam[Address - 0x8000];
	}
	
	bprintf(PRINT_NORMAL, _T("M6803 Read Byte -> %04X\n"), Address);
	
	return 0;
}

void DrvDdragonbaM6803WriteByte(UINT16 Address, UINT8 Data)
{
	if (Address >= 0x0020 && Address <= 0x0fff) {
		DrvSubCPURam[Address - 0x0020] = Data;
		return;
	}
	
	if (Address >= 0x8000 && Address <= 0x8fff) {
		if (Address == 0x8000) DrvSubCPUBusy = 1;
		DrvSpriteRam[Address - 0x8000] = Data;
		return;
	}
	
	if (Address <= 0x001f) {
		m6803_internal_registers_w(Address, Data);
		return;
	}
	
	bprintf(PRINT_NORMAL, _T("M6803 Write Byte -> %04X, %02X\n"), Address, Data);
}

void DrvDdragonbaM6803WritePort(UINT16, UINT8)
{
	M6803SetIRQLine(M6803_INPUT_LINE_NMI, M6803_IRQSTATUS_NONE);
	
	HD6309Open(0);
	HD6309SetIRQLine(HD6309_IRQ_LINE, HD6309_IRQSTATUS_ACK);
	HD6309Close();
}

void __fastcall Ddragon2SubZ80Write(UINT16 Address, UINT8 Data)
{
	if (Address >= 0xc000 && Address <= 0xc3ff) {
		if (Address == 0xc000) DrvSubCPUBusy = 1;
		DrvSpriteRam[Address - 0xc000] = Data;
		return;
	}
	
	switch (Address) {
		case 0xd000: {
			// Lower NMI
			return;
		}
		
		case 0xe000: {
			HD6309Open(0);
			HD6309SetIRQLine(HD6309_IRQ_LINE, HD6309_IRQSTATUS_ACK);
			HD6309Close();
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Sub Z80 Write => %04X, %02X\n"), Address, Data);
		}
	}
}

UINT8 DrvDdragonM6809ReadByte(UINT16 Address)
{
	switch (Address) {
		case 0x1000: {
			M6809SetIRQLine(M6809_IRQ_LINE, M6809_IRQSTATUS_NONE);
			return DrvSoundLatch;
		}
		
		case 0x1800: {
			return DrvADPCMIdle[0] + (DrvADPCMIdle[1] << 1);
		}
		
		case 0x2801: {
			return BurnYM2151ReadStatus();
		}
	}
	
	bprintf(PRINT_NORMAL, _T("M6809 Read Byte -> %04X\n"), Address);
	
	return 0;
}


void DrvDdragonM6809WriteByte(UINT16 Address, UINT8 Data)
{
	switch (Address) {
		case 0x2800: {
			BurnYM2151SelectRegister(Data);
			return;
		}
		
		case 0x2801: {
			BurnYM2151WriteRegister(Data);
			return;
		}
	
		case 0x3800: {
			DrvADPCMIdle[0] = 0;
			MSM5205ResetWrite(0, 0);
			return;
		}
		
		case 0x3801: {
			DrvADPCMIdle[1] = 0;
			MSM5205ResetWrite(1, 0);
			return;
		}
		
		case 0x3802: {
			DrvADPCMEnd[0] = (Data & 0x7f) * 0x200;			
			return;
		}
		
		case 0x3803: {
			DrvADPCMEnd[1] = (Data & 0x7f) * 0x200;			
			return;
		}
		
		case 0x3804: {
			DrvADPCMPos[0] = (Data & 0x7f) * 0x200;			
			return;
		}
		
		case 0x3805: {
			DrvADPCMPos[1] = (Data & 0x7f) * 0x200;
			return;
		}
		
		case 0x3806: {
			DrvADPCMIdle[0] = 1;
			MSM5205ResetWrite(0, 1);
			return;
		}
		
		case 0x3807: {
			DrvADPCMIdle[1] = 1;
			MSM5205ResetWrite(1, 1);
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("M6809 Write Byte -> %04X, %02X\n"), Address, Data);
}

UINT8 __fastcall Ddragon2SoundZ80Read(UINT16 Address)
{
	switch (Address) {
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
			bprintf(PRINT_NORMAL, _T("Sound Z80 Read => %04X\n"), Address);
		}
	}

	return 0;
}

void __fastcall Ddragon2SoundZ80Write(UINT16 Address, UINT8 Data)
{
	switch (Address) {
		case 0x8800: {
			BurnYM2151SelectRegister(Data);
			return;
		}
		
		case 0x8801: {
			BurnYM2151WriteRegister(Data);
			return;
		}
		
		case 0x9800: {
			MSM6295Command(0, Data);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Sound Z80 Write => %04X, %02X\n"), Address, Data);
		}
	}
}

UINT8 DrvMCUReadByte(UINT16 Address)
{
	if (Address <= 0x0007) {
		return DrvMCUPorts[Address];
	}
	
	bprintf(PRINT_NORMAL, _T("M68705 Read Byte -> %04X\n"), Address);
	
	return 0;
}

void DrvMCUWriteByte(UINT16 Address, UINT8 Data)
{
	if (Address <= 0x0007) {
		DrvMCUPorts[Address] = Data;
		return;
	}
	
	bprintf(PRINT_NORMAL, _T("M68705 Write Byte -> %04X, %02X\n"), Address, Data);
}

static INT32 CharPlaneOffsets[4]          = { 0, 2, 4, 6 };
static INT32 CharXOffsets[8]              = { 1, 0, 65, 64, 129, 128, 193, 192 };
static INT32 CharYOffsets[8]              = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 TilePlaneOffsets[4]          = { 0x100000, 0x100004, 0, 4 };
static INT32 DdungeonTilePlaneOffsets[4]  = { 0x080000, 0x080004, 0, 4 };
static INT32 TileXOffsets[16]             = { 3, 2, 1, 0, 131, 130, 129, 128, 259, 258, 257, 256, 387, 386, 385, 384 };
static INT32 TileYOffsets[16]             = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };
static INT32 SpritePlaneOffsets[4]        = { 0x200000, 0x200004, 0, 4 };
static INT32 Dd2SpritePlaneOffsets[4]     = { 0x300000, 0x300004, 0, 4 };

static void DrvYM2151IrqHandler(INT32 Irq)
{
	if (Irq) {
		M6809SetIRQLine(M6809_FIRQ_LINE, M6809_IRQSTATUS_ACK);
#if 1
		// This fixes music tempo but breaks MSM5205 sound if we use the M6809 for the MSM5205 timing,
		// because the interleave run count is 190 cycles
		nCyclesDone[2] += M6809Run(1000);
#endif
	} else {
		M6809SetIRQLine(M6809_FIRQ_LINE, M6809_IRQSTATUS_NONE);
	}
}

static void Ddragon2YM2151IrqHandler(INT32 Irq)
{
	if (Irq) {
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
	}
}

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)((double)HD6309TotalCycles() * nSoundRate / ((INT32)(nCyclesTotal[0] * 57.444853)));
}

static void DrvMSM5205Vck0()
{
	if (DrvADPCMPos[0] >= DrvADPCMEnd[0] || DrvADPCMPos[0] >= 0x10000) {
		DrvADPCMIdle[0] = 1;
		MSM5205ResetWrite(0, 1);
	} else {
		if (DrvADPCMData[0] != -1) {
			MSM5205DataWrite(0, DrvADPCMData[0] & 0x0f);
			DrvADPCMData[0] = -1;
		} else {
			UINT8 *ROM = DrvMSM5205Rom + 0x00000;

			DrvADPCMData[0] = ROM[(DrvADPCMPos[0]++) & 0xffff];
			MSM5205DataWrite(0, DrvADPCMData[0] >> 4);
		}
	}
}

static void DrvMSM5205Vck1()
{
	if (DrvADPCMPos[1] >= DrvADPCMEnd[1] || DrvADPCMPos[1] >= 0x10000) {
		DrvADPCMIdle[1] = 1;
		MSM5205ResetWrite(1, 1);
	} else {
		if (DrvADPCMData[1] != -1) {
			MSM5205DataWrite(1, DrvADPCMData[1] & 0x0f);
			DrvADPCMData[1] = -1;
		} else {
			UINT8 *ROM = DrvMSM5205Rom + 0x10000;

			DrvADPCMData[1] = ROM[(DrvADPCMPos[1]++) & 0xffff];
			MSM5205DataWrite(1, DrvADPCMData[1] >> 4);
		}
	}
}

static INT32 DrvMemInit()
{
	INT32 nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();
	
	return 0;
}

static INT32 Drv2MemInit()
{
	INT32 nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	Drv2MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	Drv2MemIndex();
	
	return 0;
}

static INT32 DarktowrMemInit()
{
	INT32 nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	DarktowrMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	DarktowrMemIndex();
	
	return 0;
}

static INT32 DrvLoadRoms()
{
	INT32 nRet = 0;

	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load HD6309 Program Roms
	nRet = BurnLoadRom(DrvHD6309Rom + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvHD6309Rom + 0x08000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvHD6309Rom + 0x10000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvHD6309Rom + 0x18000, 3, 1); if (nRet != 0) return 1;
	
	// Load HD63701 Program Roms
	nRet = BurnLoadRom(DrvSubCPURom + 0x00000, 4, 1); if (nRet != 0) return 1;
	
	// Load M6809 Program Roms
	nRet = BurnLoadRom(DrvSoundCPURom + 0x00000, 5, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 6, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, DrvTempRom, DrvChars);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x50000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000, 13, 1); if (nRet != 0) return 1;	
	nRet = BurnLoadRom(DrvTempRom + 0x70000, 14, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, TileXOffsets, TileYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 18, 1); if (nRet != 0) return 1;
	GfxDecode(0x800, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x200, DrvTempRom, DrvTiles);
	
	// Load samples
	nRet = BurnLoadRom(DrvMSM5205Rom + 0x00000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvMSM5205Rom + 0x10000, 20, 1); if (nRet != 0) return 1;

	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 DrvbaLoadRoms()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load HD6309 Program Roms
	nRet = BurnLoadRom(DrvHD6309Rom + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvHD6309Rom + 0x08000, 1, 1); if (nRet != 0) return 1;
	memcpy(DrvHD6309Rom + 0x18000, DrvHD6309Rom + 0x10000, 0x8000);
	nRet = BurnLoadRom(DrvHD6309Rom + 0x10000, 2, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(DrvSubCPURom + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load M6809 Program Roms
	nRet = BurnLoadRom(DrvSoundCPURom + 0x00000, 4, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 5, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, DrvTempRom, DrvChars);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x50000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000, 12, 1); if (nRet != 0) return 1;	
	nRet = BurnLoadRom(DrvTempRom + 0x70000, 13, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, TileXOffsets, TileYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 17, 1); if (nRet != 0) return 1;
	GfxDecode(0x800, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x200, DrvTempRom, DrvTiles);
	
	// Load samples
	nRet = BurnLoadRom(DrvMSM5205Rom + 0x00000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvMSM5205Rom + 0x10000, 19, 1); if (nRet != 0) return 1;

	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 Drv2LoadRoms()
{
	INT32 nRet = 0;

	DrvTempRom = (UINT8 *)BurnMalloc(0xc0000);

	// Load HD6309 Program Roms
	nRet = BurnLoadRom(DrvHD6309Rom + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvHD6309Rom + 0x08000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvHD6309Rom + 0x10000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvHD6309Rom + 0x18000, 3, 1); if (nRet != 0) return 1;
	
	// Load HD63701 Program Roms
	nRet = BurnLoadRom(DrvSubCPURom + 0x00000, 4, 1); if (nRet != 0) return 1;
	
	// Load M6809 Program Roms
	nRet = BurnLoadRom(DrvSoundCPURom + 0x00000, 5, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 6, 1); if (nRet != 0) return 1;
	GfxDecode(0x800, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, DrvTempRom, DrvChars);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0xc0000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(0x1800, 4, 16, 16, Dd2SpritePlaneOffsets, TileXOffsets, TileYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0xc0000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 14, 1); if (nRet != 0) return 1;
	GfxDecode(0x800, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x200, DrvTempRom, DrvTiles);
	
	// Load samples
	nRet = BurnLoadRom(MSM6295ROM + 0x00000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROM + 0x20000, 16, 1); if (nRet != 0) return 1;

	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 DdungeonLoadRoms()
{
	INT32 nRet = 0;

	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load HD6309 Program Roms
	nRet = BurnLoadRom(DrvHD6309Rom + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvHD6309Rom + 0x08000, 1, 1); if (nRet != 0) return 1;
	
	// Load HD63701 Program Roms
	nRet = BurnLoadRom(DrvSubCPURom + 0x00000, 2, 1); if (nRet != 0) return 1;
	
	// Load M6809 Program Roms
	nRet = BurnLoadRom(DrvSoundCPURom + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load M68705 MCU Program Roms
	nRet = BurnLoadRom(DrvMCURom + 0x00000, 4, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 5, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, DrvTempRom, DrvChars);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, TileXOffsets, TileYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  9, 1); if (nRet != 0) return 1;
	GfxDecode(0x800, 4, 16, 16, DdungeonTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x200, DrvTempRom, DrvTiles);
	
	// Load samples
	nRet = BurnLoadRom(DrvMSM5205Rom + 0x00000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvMSM5205Rom + 0x10000, 11, 1); if (nRet != 0) return 1;

	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 DarktowrLoadRoms()
{
	INT32 nRet = 0;

	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load HD6309 Program Roms
	nRet = BurnLoadRom(DrvHD6309Rom + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvHD6309Rom + 0x08000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvHD6309Rom + 0x10000, 2, 1); if (nRet != 0) return 1;
	
	// Load HD63701 Program Roms
	nRet = BurnLoadRom(DrvSubCPURom + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load M6809 Program Roms
	nRet = BurnLoadRom(DrvSoundCPURom + 0x00000, 4, 1); if (nRet != 0) return 1;
	
	// Load M68705 MCU Program Roms
	nRet = BurnLoadRom(DrvMCURom + 0x00000, 5, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 6, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, DrvTempRom, DrvChars);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x50000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000, 13, 1); if (nRet != 0) return 1;	
	nRet = BurnLoadRom(DrvTempRom + 0x70000, 14, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, TileXOffsets, TileYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 18, 1); if (nRet != 0) return 1;
	GfxDecode(0x800, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x200, DrvTempRom, DrvTiles);
	
	// Load samples
	nRet = BurnLoadRom(DrvMSM5205Rom + 0x00000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvMSM5205Rom + 0x10000, 20, 1); if (nRet != 0) return 1;

	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 DrvMachineInit()
{
	BurnSetRefreshRate(57.444853);
	
	// Setup the HD6309 emulation
	if (DrvSubCPUType == DD_CPU_TYPE_HD6309) {
		HD6309Init(2);
	} else {
		HD6309Init(1);
	}
	HD6309Open(0);
	HD6309MapMemory(DrvHD6309Ram         , 0x0000, 0x0fff, M6809_RAM);
	HD6309MapMemory(DrvPaletteRam1       , 0x1000, 0x11ff, M6809_RAM);
	HD6309MapMemory(DrvPaletteRam2       , 0x1200, 0x13ff, M6809_RAM);
	HD6309MapMemory(DrvFgVideoRam        , 0x1800, 0x1fff, M6809_RAM);
	HD6309MapMemory(DrvSpriteRam         , 0x2000, 0x2fff, M6809_WRITE);
	HD6309MapMemory(DrvBgVideoRam        , 0x3000, 0x37ff, M6809_RAM);
	HD6309MapMemory(DrvHD6309Rom + 0x8000, 0x4000, 0x7fff, M6809_ROM);
	HD6309MapMemory(DrvHD6309Rom         , 0x8000, 0xffff, M6809_ROM);
	HD6309SetReadHandler(DrvDdragonHD6309ReadByte);
	HD6309SetWriteHandler(DrvDdragonHD6309WriteByte);
	HD6309Close();
	
	if (DrvSubCPUType == DD_CPU_TYPE_HD63701) {
		HD63701Init(1);
		HD63701MapMemory(DrvSubCPURom        , 0xc000, 0xffff, HD63701_ROM);
		HD63701SetReadHandler(DrvDdragonHD63701ReadByte);
		HD63701SetWriteHandler(DrvDdragonHD63701WriteByte);
	}
	
	if (DrvSubCPUType == DD_CPU_TYPE_HD6309) {
		HD6309Open(1);
		HD6309MapMemory(DrvSubCPURom        , 0xc000, 0xffff, HD6309_ROM);
		HD6309SetReadHandler(DrvDdragonbSubHD6309ReadByte);
		HD6309SetWriteHandler(DrvDdragonbSubHD6309WriteByte);
		HD6309Close();
	}
	
	if (DrvSubCPUType == DD_CPU_TYPE_M6803) {
		M6803Init(1);
		M6803MapMemory(DrvSubCPURom        , 0xc000, 0xffff, M6803_ROM);
		M6803SetReadHandler(DrvDdragonbaM6803ReadByte);
		M6803SetWriteHandler(DrvDdragonbaM6803WriteByte);
		M6803SetWritePortHandler(DrvDdragonbaM6803WritePort);
	}
	
	if (DrvSoundCPUType == DD_CPU_TYPE_M6809) {
		M6809Init(1);
		M6809Open(0);
		M6809MapMemory(DrvSoundCPURam      , 0x0000, 0x0fff, M6809_RAM);
		M6809MapMemory(DrvSoundCPURom      , 0x8000, 0xffff, M6809_ROM);
		M6809SetReadHandler(DrvDdragonM6809ReadByte);
		M6809SetWriteHandler(DrvDdragonM6809WriteByte);
		M6809Close();
	
		BurnYM2151Init(3579545);
		BurnYM2151SetIrqHandler(&DrvYM2151IrqHandler);
		BurnYM2151SetAllRoutes(0.60, BURN_SND_ROUTE_BOTH);
	
		MSM5205Init(0, DrvSynchroniseStream, 375000, DrvMSM5205Vck0, MSM5205_S48_4B, 1);
		MSM5205Init(1, DrvSynchroniseStream, 375000, DrvMSM5205Vck1, MSM5205_S48_4B, 1);
		MSM5205SetRoute(0, 0.50, BURN_SND_ROUTE_BOTH);
		MSM5205SetRoute(1, 0.50, BURN_SND_ROUTE_BOTH);
	}
	
	if (DrvGameType == DD_GAME_DARKTOWR) {
		m6805Init(1, 0x800);
		m6805MapMemory(DrvMCURom + 0x80, 0x080, 0x7ff, M6805_ROM);
		m6805MapMemory(DrvMCURom, 0x008, 0x07f, M6805_RAM);
		m6805SetReadHandler(DrvMCUReadByte);
		m6805SetWriteHandler(DrvMCUWriteByte);
	}
	
	nCyclesTotal[0] = (INT32)((double)4000000 / 57.44853);
	nCyclesTotal[1] = (INT32)((double)4000000 / 57.44853);
	nCyclesTotal[2] = (INT32)((double)1500000 / 57.44853);

	GenericTilesInit();

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 Drv2MachineInit()
{
	// Setup the HD6309 emulation
	HD6309Init(1);
	HD6309Open(0);
	HD6309MapMemory(DrvHD6309Ram         , 0x0000, 0x17ff, M6809_RAM);
	HD6309MapMemory(DrvFgVideoRam        , 0x1800, 0x1fff, M6809_RAM);
	HD6309MapMemory(DrvSpriteRam         , 0x2000, 0x2fff, M6809_WRITE);
	HD6309MapMemory(DrvBgVideoRam        , 0x3000, 0x37ff, M6809_RAM);
	HD6309MapMemory(DrvPaletteRam1       , 0x3c00, 0x3dff, M6809_RAM);
	HD6309MapMemory(DrvPaletteRam2       , 0x3e00, 0x3fff, M6809_RAM);
	HD6309MapMemory(DrvHD6309Rom + 0x8000, 0x4000, 0x7fff, M6809_ROM);
	HD6309MapMemory(DrvHD6309Rom         , 0x8000, 0xffff, M6809_ROM);
	HD6309SetReadHandler(DrvDdragonHD6309ReadByte);
	HD6309SetWriteHandler(DrvDdragonHD6309WriteByte);
	HD6309Close();
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetWriteHandler(Ddragon2SubZ80Write);
	ZetMapArea(0x0000, 0xbfff, 0, DrvSubCPURom);
	ZetMapArea(0x0000, 0xbfff, 2, DrvSubCPURom);
	ZetMapArea(0xc000, 0xc3ff, 0, DrvSpriteRam);
	ZetMapArea(0xc000, 0xc3ff, 2, DrvSpriteRam);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetSetReadHandler(Ddragon2SoundZ80Read);
	ZetSetWriteHandler(Ddragon2SoundZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, DrvSoundCPURom);
	ZetMapArea(0x0000, 0x7fff, 2, DrvSoundCPURom);
	ZetMapArea(0x8000, 0x87ff, 0, DrvSoundCPURam);
	ZetMapArea(0x8000, 0x87ff, 1, DrvSoundCPURam);
	ZetMapArea(0x8000, 0x87ff, 2, DrvSoundCPURam);
	ZetMemEnd();
	ZetClose();
	
	BurnYM2151Init(3579545);
	BurnYM2151SetIrqHandler(&Ddragon2YM2151IrqHandler);
	BurnYM2151SetAllRoutes(0.60, BURN_SND_ROUTE_BOTH);
	
	MSM6295Init(0, 1056000 / 132, 1);
	MSM6295SetRoute(0, 0.20, BURN_SND_ROUTE_BOTH);
	
	BurnSetRefreshRate(57.444853);
	
	nCyclesTotal[0] = (INT32)((double)4000000 / 57.44853);
	nCyclesTotal[1] = (INT32)((double)4000000 / 57.44853);
	nCyclesTotal[2] = (INT32)((double)3579545 / 57.44853);
	nCyclesTotal[3] = (INT32)((double)4000000 / 57.44853);

	GenericTilesInit();

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 DrvInit()
{
	DrvSubCPUType = DD_CPU_TYPE_HD63701;
	DrvSoundCPUType = DD_CPU_TYPE_M6809;
	
	if (DrvMemInit()) return 1;
	if (DrvLoadRoms()) return 1;
	if (DrvMachineInit()) return 1;
	
	return 0;
}

static INT32 DrvbInit()
{
	DrvSubCPUType = DD_CPU_TYPE_HD6309;
	DrvSoundCPUType = DD_CPU_TYPE_M6809;
	
	if (DrvMemInit()) return 1;
	if (DrvLoadRoms()) return 1;
	if (DrvMachineInit()) return 1;
	
	nCyclesTotal[1] = (INT32)((double)1500000 / 57.44853);

	return 0;
}

static INT32 DrvbaInit()
{
	DrvSubCPUType = DD_CPU_TYPE_M6803;
	DrvSoundCPUType = DD_CPU_TYPE_M6809;
	
	if (DrvMemInit()) return 1;
	if (DrvbaLoadRoms()) return 1;
	if (DrvMachineInit()) return 1;

	return 0;
}

static INT32 Drv2Init()
{
	DrvSubCPUType = DD_CPU_TYPE_Z80;
	DrvSoundCPUType = DD_CPU_TYPE_Z80;
	DrvVidHardwareType = DD_VID_TYPE_DD2;
	
	if (Drv2MemInit()) return 1;
	if (Drv2LoadRoms()) return 1;
	if (Drv2MachineInit()) return 1;

	return 0;
}

static INT32 DdungeonInit()
{
	DrvSubCPUType = DD_CPU_TYPE_HD63701;
	DrvSoundCPUType = DD_CPU_TYPE_M6809;
	DrvGameType = DD_GAME_DARKTOWR;
	
	if (DarktowrMemInit()) return 1;
	if (DdungeonLoadRoms()) return 1;
	if (DrvMachineInit()) return 1;
	
	return 0;
}

static INT32 DarktowrInit()
{
	DrvSubCPUType = DD_CPU_TYPE_HD63701;
	DrvSoundCPUType = DD_CPU_TYPE_M6809;
	DrvGameType = DD_GAME_DARKTOWR;
	
	if (DarktowrMemInit()) return 1;
	if (DarktowrLoadRoms()) return 1;
	if (DrvMachineInit()) return 1;
	
	return 0;
}

static INT32 DrvExit()
{
	HD6309Exit();
	if (DrvSubCPUType == DD_CPU_TYPE_M6803 || DrvSubCPUType == DD_CPU_TYPE_HD63701) M6800Exit();
	if (DrvGameType == DD_GAME_DARKTOWR) m6805Exit();
	if (DrvSoundCPUType == DD_CPU_TYPE_M6809) M6809Exit();
	if (DrvSubCPUType == DD_CPU_TYPE_Z80 || DrvSoundCPUType == DD_CPU_TYPE_Z80) ZetExit();
	
	BurnYM2151Exit();
	if (DrvSoundCPUType == DD_CPU_TYPE_Z80) {
		MSM6295Exit(0);
	} else {
		MSM5205Exit();
	}
		
	GenericTilesExit();
	
	BurnFree(Mem);
	
	DrvRomBank = 0;
	DrvVBlank = 0;
	DrvSubCPUBusy = 0;
	DrvSoundLatch = 0;
	DrvScrollXHi = 0;
	DrvScrollYHi = 0;
	DrvScrollXLo = 0;
	DrvScrollYLo = 0;
	
	DrvADPCMIdle[0] = 0;
	DrvADPCMIdle[1] = 0;
	DrvADPCMPos[0] = 0;
	DrvADPCMPos[1] = 0;
	DrvADPCMEnd[0] = 0;
	DrvADPCMEnd[1] = 0;
	DrvADPCMData[0] = 0;
	DrvADPCMData[1] = 0;
	
	DrvSubCPUType = DD_CPU_TYPE_NONE;
	DrvSoundCPUType = DD_CPU_TYPE_NONE;
	DrvVidHardwareType = 0;
	DrvGameType = 0;

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

	r = pal4bit(nColour >> 0);
	g = pal4bit(nColour >> 4);
	b = pal4bit(nColour >> 8);

	return BurnHighCol(r, g, b, 0);
}

static void DrvCalcPalette()
{
	for (INT32 i = 0; i < 0x180; i++) {
		INT32 Val = DrvPaletteRam1[i] + (DrvPaletteRam2[i] << 8);
		
		DrvPalette[i] = CalcCol(Val);
	}
}

static void DrvRenderBgLayer()
{
	INT32 mx, my, Code, Attr, Colour, x, y, TileIndex, xScroll, yScroll, Flip, xFlip, yFlip;
	
	xScroll = DrvScrollXHi + DrvScrollXLo;
	xScroll &= 0x1ff;
		
	yScroll = DrvScrollYHi + DrvScrollYLo;
	yScroll &= 0x1ff;

	for (mx = 0; mx < 32; mx++) {
		for (my = 0; my < 32; my++) {
			TileIndex = (my & 0x0f) + ((mx & 0x0f) << 4) + ((my & 0x10) << 4) + ((mx & 0x10) << 5);
			
			Attr = DrvBgVideoRam[(2 * TileIndex) + 0];
			Code = DrvBgVideoRam[(2 * TileIndex) + 1] + ((Attr & 0x07) << 8);
			Colour = (Attr >> 3) & 0x07;
			Flip = (Attr & 0xc0) >> 6;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			y = 16 * mx;
			x = 16 * my;

			x -= xScroll;
			if (x < -16) x += 512;
			
			y -= yScroll;
			if (y < -16) y += 512;
			
			y -= 8;

			if (x > 16 && x < 240 && y > 16 && y < 224) {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_FlipXY(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					} else {
						Render16x16Tile_FlipX(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_FlipY(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					} else {
						Render16x16Tile(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					}
				}
			} else {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					} else {
						Render16x16Tile_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					} else {
						Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 256, DrvTiles);
					}
				}
			}
		}
	}
}

#define DRAW_SPRITE(Order, sx, sy)													\
	if (sx > 16 && sx < 240 && sy > 16 && sy < 224) {										\
		if (xFlip) {														\
			if (yFlip) {													\
				Render16x16Tile_Mask_FlipXY(pTransDraw, Code + Order, sx, sy, Colour, 4, 0, 128, DrvSprites);		\
			} else {													\
				Render16x16Tile_Mask_FlipX(pTransDraw, Code + Order, sx, sy, Colour, 4, 0, 128, DrvSprites);		\
			}														\
		} else {														\
			if (yFlip) {													\
				Render16x16Tile_Mask_FlipY(pTransDraw, Code + Order, sx, sy, Colour, 4, 0, 128, DrvSprites);		\
			} else {													\
				Render16x16Tile_Mask(pTransDraw, Code + Order, sx, sy, Colour, 4, 0, 128, DrvSprites);			\
			}														\
		}															\
	} else {															\
		if (xFlip) {														\
			if (yFlip) {													\
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code + Order, sx, sy, Colour, 4, 0, 128, DrvSprites);	\
			} else {													\
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code + Order, sx, sy, Colour, 4, 0, 128, DrvSprites);	\
			}														\
		} else {														\
			if (yFlip) {													\
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code + Order, sx, sy, Colour, 4, 0, 128, DrvSprites);	\
			} else {													\
				Render16x16Tile_Mask_Clip(pTransDraw, Code + Order, sx, sy, Colour, 4, 0, 128, DrvSprites);		\
			}														\
		}															\
	}

static void DrvRenderSpriteLayer()
{
	UINT8 *Src = &DrvSpriteRam[0x800];
	
	for (INT32 i = 0; i < (64 * 5); i += 5) {
		INT32 Attr = Src[i + 1];
		
		if (Attr & 0x80) {
			INT32 sx = 240 - Src[i + 4] + ((Attr & 2) << 7);
			INT32 sy = 232 - Src[i + 0] + ((Attr & 1) << 8);
			INT32 Size = (Attr & 0x30) >> 4;
			INT32 xFlip = Attr & 0x08;
			INT32 yFlip = Attr & 0x04;
			INT32 dx = -16;
			INT32 dy = -16;
			
			INT32 Colour;
			INT32 Code;
			
			if (DrvVidHardwareType == DD_VID_TYPE_DD2) {
				Colour = (Src[i + 2] >> 5);
				Code = Src[i + 3] + ((Src[i + 2] & 0x1f) << 8);
			} else {
				Colour = (Src[i + 2] >> 4) & 0x07;
				Code = Src[i + 3] + ((Src[i + 2] & 0x0f) << 8);
			}
			
			Code &= ~Size;
			
			switch (Size) {
				case 0: {
					DRAW_SPRITE(0, sx, sy)
					break;
				}
				
				case 1: {
					DRAW_SPRITE(0, sx, sy + dy)
					DRAW_SPRITE(1, sx, sy)
					break;
				}
				
				case 2: {
					DRAW_SPRITE(0, sx + dx, sy)
					DRAW_SPRITE(2, sx, sy)
					break;
				}
				
				case 3: {
					DRAW_SPRITE(0, sx + dx, sy + dy)
					DRAW_SPRITE(1, sx + dx, sy)
					DRAW_SPRITE(2, sx, sy + dy)
					DRAW_SPRITE(3, sx, sy)
					break;
				}
			}
		}
	}
}

#undef DRAW_SPRITE

static void DrvRenderCharLayer()
{
	INT32 mx, my, Code, Attr, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Attr = DrvFgVideoRam[(2 * TileIndex) + 0];
			Code = DrvFgVideoRam[(2 * TileIndex) + 1] + ((Attr & 0x07) << 8);
			if (DrvVidHardwareType != DD_VID_TYPE_DD2) Code &= 0x3ff;
			
			Colour = Attr >> 5;
			
			x = 8 * mx;
			y = 8 * my;
			
			y -= 8;

			if (x > 0 && x < 248 && y > 0 && y < 232) {
				Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvChars);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvChars);
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
	INT32 nInterleave = 272;
	if (DrvSoundCPUType == DD_CPU_TYPE_M6809) nInterleave = MSM5205CalcInterleave(0, (INT32)(nCyclesTotal[0] * 57.444853));
	INT32 nSoundBufferPos = 0;
	
	INT32 VBlankSlice = (INT32)((double)(nInterleave * 240) / 272);
	INT32 FIRQFireSlice[16];
	for (INT32 i = 0; i < 16; i++) {
		FIRQFireSlice[i] = (INT32)((double)((nInterleave * (i + 1)) / 17));
	}
	
	if (DrvReset) DrvDoReset();

	DrvMakeInputs();
	
	UINT32 nCyclesToDo[2];
	nCyclesToDo[0] = (INT32)((double)nCyclesTotal[0] * nBurnCPUSpeedAdjust / 0x0100);
	nCyclesToDo[1] = (INT32)((double)nCyclesTotal[1] * nBurnCPUSpeedAdjust / 0x0100);
	
	nCyclesDone[0] = nCyclesDone[1] = nCyclesDone[2] = nCyclesDone[3] = 0;
	
	HD6309NewFrame();
	if (DrvSubCPUType == DD_CPU_TYPE_HD63701) HD63701NewFrame();
	if (DrvSubCPUType == DD_CPU_TYPE_M6803) M6803NewFrame();
	if (DrvSubCPUType == DD_CPU_TYPE_Z80 || DrvSoundCPUType == DD_CPU_TYPE_Z80) ZetNewFrame();
	if (DrvSoundCPUType == DD_CPU_TYPE_M6809) M6809NewFrame();
	if (DrvGameType == DD_GAME_DARKTOWR) m6805NewFrame();
	
	DrvVBlank = 0;
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		nCurrentCPU = 0;
		HD6309Open(0);
		nNext = (i + 1) * nCyclesToDo[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += HD6309Run(nCyclesSegment);
		if (DrvSoundCPUType == DD_CPU_TYPE_M6809) MSM5205Update();
		HD6309Close();
		
		if (DrvSubCPUType == DD_CPU_TYPE_HD63701) {
			nCurrentCPU = 1;
			nNext = (i + 1) * nCyclesToDo[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = HD63701Run(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
		}
		
		if (DrvSubCPUType == DD_CPU_TYPE_HD6309) {
			nCurrentCPU = 1;
			HD6309Open(1);
			nNext = (i + 1) * nCyclesToDo[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = HD6309Run(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
			HD6309Close();
		}
		
		if (DrvSubCPUType == DD_CPU_TYPE_M6803) {
			nCurrentCPU = 1;
			nNext = (i + 1) * nCyclesToDo[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = M6803Run(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
		}
		
		if (DrvSubCPUType == DD_CPU_TYPE_Z80) {
			nCurrentCPU = 1;
			ZetOpen(0);
			nNext = (i + 1) * nCyclesToDo[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
			ZetClose();
		}
		
		if (DrvSoundCPUType == DD_CPU_TYPE_M6809) {
			nCurrentCPU = 2;
			M6809Open(0);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesDone[nCurrentCPU] += M6809Run(nCyclesSegment);
			M6809Close();
		}
		
		if (DrvSoundCPUType == DD_CPU_TYPE_Z80) {
			nCurrentCPU = 2;
			ZetOpen(1);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
			ZetClose();
		}
		
		if (DrvGameType == DD_GAME_DARKTOWR) {
			nCurrentCPU = 3;
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = m6805Run(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
		}
		
		if (i == VBlankSlice) {
			DrvVBlank = 1;
			HD6309Open(0);
			HD6309SetIRQLine(HD6309_INPUT_LINE_NMI, HD6309_IRQSTATUS_ACK);
			HD6309Close();
		}
		
		for (INT32 j = 0; j < 16; j++) {
			if (i == FIRQFireSlice[j]) {
				HD6309Open(0);
				HD6309SetIRQLine(HD6309_FIRQ_LINE, HD6309_IRQSTATUS_ACK);
				HD6309Close();
			}
		}
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			
			if (DrvSoundCPUType == DD_CPU_TYPE_M6809) {
				M6809Open(0);
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				M6809Close();
			}
			
			if (DrvSoundCPUType == DD_CPU_TYPE_Z80) {
				ZetOpen(1);
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				ZetClose();
				MSM6295Render(0, pSoundBuf, nSegmentLength);
			}
			
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			if (DrvSoundCPUType == DD_CPU_TYPE_M6809) {
				M6809Open(0);
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				M6809Close();
			}
			
			if (DrvSoundCPUType == DD_CPU_TYPE_Z80) {
				ZetOpen(1);
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				ZetClose();
				MSM6295Render(0, pSoundBuf, nSegmentLength);
			}
		}
	}
	
	if (DrvSoundCPUType == DD_CPU_TYPE_M6809 && pBurnSoundOut) {
		M6809Open(0);
		MSM5205Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM5205Render(1, pBurnSoundOut, nBurnSoundLen);
		M6809Close();
	}

	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
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
		HD6309Scan(nAction);
		
		if (DrvSubCPUType == DD_CPU_TYPE_HD63701) HD63701Scan(nAction);
		if (DrvSubCPUType == DD_CPU_TYPE_M6803) M6803Scan(nAction);
		if (DrvSubCPUType == DD_CPU_TYPE_Z80 || DrvSoundCPUType == DD_CPU_TYPE_Z80) ZetScan(nAction);
		if (DrvSoundCPUType == DD_CPU_TYPE_M6809) M6809Scan(nAction);
		if (DrvGameType == DD_GAME_DARKTOWR) m68705Scan(nAction, pnMin);
		
		BurnYM2151Scan(nAction);
		if (DrvSoundCPUType == DD_CPU_TYPE_Z80) MSM6295Scan(0, nAction);
		if (DrvSoundCPUType == DD_CPU_TYPE_M6809) MSM5205Scan(nAction, pnMin);
		
		SCAN_VAR(DrvRomBank);
		SCAN_VAR(DrvVBlank);
		SCAN_VAR(DrvSubCPUBusy);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(DrvScrollXHi);
		SCAN_VAR(DrvScrollYHi);
		SCAN_VAR(DrvScrollXLo);
		SCAN_VAR(DrvScrollYLo);
		SCAN_VAR(DrvADPCMIdle);
		SCAN_VAR(DrvADPCMPos);
		SCAN_VAR(DrvADPCMEnd);
		SCAN_VAR(DrvADPCMData);
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
		
		if (nAction & ACB_WRITE) {
			HD6309Open(0);
			HD6309MapMemory(DrvHD6309Rom + 0x8000 + (DrvRomBank * 0x4000), 0x4000, 0x7fff, M6809_ROM);
			HD6309Close();
			
			if (DrvSubCPUBusy == 0) {
				if (DrvSubCPUType == DD_CPU_TYPE_HD63701) {
					HD63701SetIRQLine(HD63701_INPUT_LINE_NMI, HD63701_IRQSTATUS_ACK);
				}
				
				if (DrvSubCPUType == DD_CPU_TYPE_M6803) {
					M6803SetIRQLine(M6803_INPUT_LINE_NMI, M6803_IRQSTATUS_ACK);
				}
			}
		}
	}

	return 0;
}

struct BurnDriver BurnDrvDdragon = {
	"ddragon", NULL, NULL, NULL, "1987",
	"Double Dragon (Japan)\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriver BurnDrvDdragonw = {
	"ddragonw", "ddragon", NULL, NULL, "1987",
	"Double Dragon (World set 1)\0", NULL, "[Technos] (Taito license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DrvwRomInfo, DrvwRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriver BurnDrvDdragnw1 = {
	"ddragonw1", "ddragon", NULL, NULL, "1987",
	"Double Dragon (World set 2)\0", NULL, "[Technos] (Taito license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, Drvw1RomInfo, Drvw1RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriver BurnDrvDdragonu = {
	"ddragonu", "ddragon", NULL, NULL, "1987",
	"Double Dragon (US set 1)\0", NULL, "[Technos] (Taito America license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DrvuRomInfo, DrvuRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriver BurnDrvDdragoua = {
	"ddragonua", "ddragon", NULL, NULL, "1987",
	"Double Dragon (US set 2)\0", NULL, "[Technos] (Taito America license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DrvuaRomInfo, DrvuaRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriver BurnDrvDdragoub = {
	"ddragonub", "ddragon", NULL, NULL, "1987",
	"Double Dragon (US set 3)\0", NULL, "[Technos] (Taito America license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DrvubRomInfo, DrvubRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriver BurnDrvDdragob2 = {
	"ddragonb2", "ddragon", NULL, NULL, "1987",
	"Double Dragon (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, Drvb2RomInfo, Drvb2RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriver BurnDrvDdragonb = {
	"ddragonb", "ddragon", NULL, NULL, "1987",
	"Double Dragon (bootleg with HD6309)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DrvbRomInfo, DrvbRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvbInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriver BurnDrvDdragnba = {
	"ddragonba", "ddragon", NULL, NULL, "1987",
	"Double Dragon (bootleg with M6803)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DrvbaRomInfo, DrvbaRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvbaInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriver BurnDrvDdragon2 = {
	"ddragon2", NULL, NULL, NULL, "1988",
	"Double Dragon II - The Revenge (World)\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, Drv2RomInfo, Drv2RomName, NULL, NULL, DrvInputInfo, Drv2DIPInfo,
	Drv2Init, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriver BurnDrvDdragon2u = {
	"ddragon2u", "ddragon2", NULL, NULL, "1988",
	"Double Dragon II - The Revenge (US)\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, Drv2uRomInfo, Drv2uRomName, NULL, NULL, DrvInputInfo, Drv2DIPInfo,
	Drv2Init, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriverD BurnDrvDdungeon = {
	"ddungeon", NULL, NULL, NULL, "1992",
	"Dangerous Dungeons\0", NULL, "Game Room", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DdungeonRomInfo, DdungeonRomName, NULL, NULL, DrvInputInfo, DdungeonDIPInfo,
	DdungeonInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};

struct BurnDriver BurnDrvDarktowr = {
	"darktowr", NULL, NULL, NULL, "1992",
	"Dark Tower\0", NULL, "Game Room", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TECHNOS, GBF_SCRFIGHT, 0,
	NULL, DarktowrRomInfo, DarktowrRomName, NULL, NULL, DrvInputInfo, DarktowrDIPInfo,
	DarktowrInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x180, 256, 240, 4, 3
};
