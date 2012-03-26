#include "tiles_generic.h"
#include "sek.h"
#include "zet.h"
#include "burn_ym2151.h"
#include "msm6295.h"
#include "burn_ym3812.h"

static UINT8 DrvInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[2]        = {0, 0};
static UINT8 DrvInput[3]      = {0, 0, 0};
static UINT8 DrvReset         = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *Drv68KRom           = NULL;
static UINT8 *Drv68KRam           = NULL;
static UINT8 *DrvZ80Rom           = NULL;
static UINT8 *DrvZ80Ram           = NULL;
static UINT8 *DrvProtData         = NULL;
static UINT8 *DrvMSM6295ROMSrc    = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvPf1Ram           = NULL;
static UINT8 *DrvPf2Ram           = NULL;
static UINT8 *DrvPaletteRam       = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvTiles            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette         = NULL;
static UINT16 *DrvControl         = NULL;

static UINT8 DrvVBlank;
static UINT8 DrvOkiBank;
static UINT8 DrvZ80Bank;
static UINT16 DrvTileBank;
static INT32 DrvSoundLatch;
static INT32 Tumbleb2MusicCommand;
static INT32 Tumbleb2MusicBank;
static INT32 Tumbleb2MusicIsPlaying;

static INT32 DrvSpriteXOffset;
static INT32 DrvSpriteYOffset;
static INT32 DrvSpriteRamSize;
static INT32 DrvSpriteMask;
static INT32 DrvSpriteColourMask;
static INT32 DrvYM2151Freq;
static INT32 DrvNumSprites;
static INT32 DrvNumChars;
static INT32 DrvNumTiles;
static INT32 DrvHasZ80;
static INT32 DrvHasYM2151;
static INT32 DrvHasYM3812;
static INT32 DrvHasProt;
static INT32 Tumbleb2;
static INT32 Jumpkids;
static INT32 Chokchok;
static INT32 Wlstar;
static INT32 Wondl96;
static INT32 Bcstry;
static INT32 Semibase;
static INT32 SemicomSoundCommand;
static INT32 Pf1XOffset;
static INT32 Pf1YOffset;
static INT32 Pf2XOffset;
static INT32 Pf2YOffset;

typedef INT32 (*LoadRoms)();
static LoadRoms DrvLoadRoms;
typedef void (*Map68k)();
static Map68k DrvMap68k;
typedef void (*MapZ80)();
static MapZ80 DrvMapZ80;
typedef void (*Render)();
static Render DrvRender;

static void DrvDraw();
static void PangpangDraw();
static void SuprtrioDraw();
static void HtchctchDraw();
static void FncywldDraw();
static void SdfightDraw();
static void JumppopDraw();

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static struct BurnInputInfo TumblebInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort2 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 start"  },

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
	{"Service"           , BIT_DIGITAL  , DrvInputPort2 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Tumbleb)

static struct BurnInputInfo MetlsavrInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort2 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 fire 3" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , DrvInputPort1 + 6, "p2 fire 3" },
	
	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Metlsavr)

static struct BurnInputInfo SuprtrioInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort2 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 fire 3" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , DrvInputPort1 + 6, "p2 fire 3" },
	
	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
};

STDINPUTINFO(Suprtrio)

static struct BurnInputInfo HtchctchInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort2 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 start"  },

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
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Htchctch)

static struct BurnInputInfo FncywldInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort2 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 fire 3" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , DrvInputPort1 + 6, "p2 fire 3" },
	
	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort2 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Fncywld)

static struct BurnInputInfo SemibaseInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort2 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 start"  },
	{"Coin 3"            , BIT_DIGITAL  , DrvInputPort2 + 2, "p3 coin"   },
	{"Coin 4"            , BIT_DIGITAL  , DrvInputPort2 + 3, "p4 coin"   },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 fire 3" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , DrvInputPort1 + 6, "p2 fire 3" },
	
	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Semibase)

static struct BurnInputInfo JumppopInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort2 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort2 + 2, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 start"  },

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
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Jumppop)

static struct BurnDIPInfo TumblebDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xfe, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x12, 0x01, 0xe0, 0x00, "3 Coins 1 Credit"       },
	{0x12, 0x01, 0xe0, 0x80, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0xe0, 0xe0, "1 Coin  1 Credit"       },
	{0x12, 0x01, 0xe0, 0x60, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits"      },
	{0x12, 0x01, 0xe0, 0x20, "1 Coin  4 Credits"      },
	{0x12, 0x01, 0xe0, 0xc0, "1 Coin  5 Credits"      },
	{0x12, 0x01, 0xe0, 0x40, "1 Coin  6 Credits"      },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x12, 0x01, 0x1c, 0x00, "3 Coins 1 Credit"       },
	{0x12, 0x01, 0x1c, 0x10, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0x1c, 0x1c, "1 Coin  1 Credit"       },
	{0x12, 0x01, 0x1c, 0x0c, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0x1c, 0x14, "1 Coin  3 Credits"      },
	{0x12, 0x01, 0x1c, 0x04, "1 Coin  4 Credits"      },
	{0x12, 0x01, 0x1c, 0x18, "1 Coin  5 Credits"      },
	{0x12, 0x01, 0x1c, 0x08, "1 Coin  6 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x12, 0x01, 0x02, 0x02, "Off"                    },
	{0x12, 0x01, 0x02, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "2 Coins to Start, 1 to Continue" },
	{0x12, 0x01, 0x01, 0x01, "Off"                    },
	{0x12, 0x01, 0x01, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0xc0, 0x80, "1"                      },
	{0x13, 0x01, 0xc0, 0x00, "2"                      },
	{0x13, 0x01, 0xc0, 0xc0, "3"                      },
	{0x13, 0x01, 0xc0, 0x40, "4"                      },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0x30, 0x10, "Easy"                   },
	{0x13, 0x01, 0x30, 0x30, "Normal"                 },
	{0x13, 0x01, 0x30, 0x20, "Hard"                   },
	{0x13, 0x01, 0x30, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x13, 0x01, 0x02, 0x00, "Off"                    },
	{0x13, 0x01, 0x02, 0x02, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
};

STDDIPINFO(Tumbleb)

static struct BurnDIPInfo MetlsavrDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x0c, 0x00, "2"                      },
	{0x13, 0x01, 0x0c, 0x0c, "3"                      },
	{0x13, 0x01, 0x0c, 0x08, "4"                      },
	{0x13, 0x01, 0x0c, 0x04, "5"                      },
	
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x13, 0x01, 0x70, 0x00, "5 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x10, "4 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x20, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x30, "2 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x70, "1 Coin  1 Credit"       },
	{0x13, 0x01, 0x70, 0x60, "1 Coin  2 Credits"      },
	{0x13, 0x01, 0x70, 0x50, "1 Coin  3 Credits"      },
	{0x13, 0x01, 0x70, 0x40, "1 Coin  5 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x80, 0x00, "Off"                    },
	{0x13, 0x01, 0x80, 0x80, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Language"               },
	{0x14, 0x01, 0x08, 0x08, "English"                },
	{0x14, 0x01, 0x08, 0x00, "Korean"                 },
	
	{0   , 0xfe, 0   , 4   , "Life Meter"             },
	{0x14, 0x01, 0x30, 0x00, "66%"                    },
	{0x14, 0x01, 0x30, 0x30, "100%"                   },
	{0x14, 0x01, 0x30, 0x20, "133%"                   },
	{0x14, 0x01, 0x30, 0x10, "166%"                   },
	
	{0   , 0xfe, 0   , 4   , "Time"                   },
	{0x14, 0x01, 0xc0, 0x40, "30 Seconds"             },
	{0x14, 0x01, 0xc0, 0x80, "40 Seconds"             },
	{0x14, 0x01, 0xc0, 0xc0, "60 Seconds"             },
	{0x14, 0x01, 0xc0, 0x00, "80 Seconds"             },
};

STDDIPINFO(Metlsavr)

static struct BurnDIPInfo SuprtrioDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0x10, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x12, 0x01, 0x07, 0x06, "5 Coins 1 Credit"       },
	{0x12, 0x01, 0x07, 0x05, "4 Coins 1 Credit"       },
	{0x12, 0x01, 0x07, 0x04, "3 Coins 1 Credit"       },
	{0x12, 0x01, 0x07, 0x03, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0x07, 0x00, "1 Coin  1 Credit"       },
	{0x12, 0x01, 0x07, 0x01, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0x07, 0x02, "1 Coin  3 Credits"      },
	{0x12, 0x01, 0x07, 0x07, "Free Play"              },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x18, 0x00, "1"                      },
	{0x12, 0x01, 0x18, 0x08, "2"                      },
	{0x12, 0x01, 0x18, 0x10, "3"                      },
	{0x12, 0x01, 0x18, 0x18, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x12, 0x01, 0x20, 0x00, "Normal"                 },
	{0x12, 0x01, 0x20, 0x20, "Hard"                   },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x12, 0x01, 0x40, 0x00, "50000"                  },
	{0x12, 0x01, 0x40, 0x40, "60000"                  },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x80, 0x00, "Off"                    },
	{0x12, 0x01, 0x80, 0x80, "On"                     },
};

STDDIPINFO(Suprtrio)

static struct BurnDIPInfo HtchctchDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0x7f, NULL                     },

	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x01, 0x01, "Off"                    },
	{0x12, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x12, 0x01, 0x06, 0x00, "Easy"                   },
	{0x12, 0x01, 0x06, 0x06, "Normal"                 },
	{0x12, 0x01, 0x06, 0x02, "Hard"                   },
	{0x12, 0x01, 0x06, 0x04, "Very Hard"              },
	
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x12, 0x01, 0x38, 0x00, "5 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x20, "4 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x10, "3 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x30, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credit"       },
	{0x12, 0x01, 0x38, 0x28, "2 Coins 3 Credits"      },
	{0x12, 0x01, 0x38, 0x18, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0x38, 0x08, "1 Coin  3 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Stage Skip"             },
	{0x12, 0x01, 0x40, 0x40, "Off"                    },
	{0x12, 0x01, 0x40, 0x00, "On"                     },	
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Htchctch)

static struct BurnDIPInfo CookbibDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0x7f, NULL                     },

	// Dip 1
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x01, 0x01, "Off"                    },
	{0x12, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x12, 0x01, 0x06, 0x00, "Easy"                   },
	{0x12, 0x01, 0x06, 0x06, "Normal"                 },
	{0x12, 0x01, 0x06, 0x02, "Hard"                   },
	{0x12, 0x01, 0x06, 0x04, "Very Hard"              },
	
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x12, 0x01, 0x38, 0x00, "5 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x20, "4 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x10, "3 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x30, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credit"       },
	{0x12, 0x01, 0x38, 0x28, "2 Coins 3 Credits"      },
	{0x12, 0x01, 0x38, 0x18, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0x38, 0x08, "1 Coin  3 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Winning Rounds (vs mode)"},
	{0x12, 0x01, 0x40, 0x00, "1"                       },
	{0x12, 0x01, 0x40, 0x40, "2"                       },	
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Cookbib)

static struct BurnDIPInfo ChokchokDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0x7f, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Winning Rounds (vs mode)"},
	{0x11, 0x01, 0x01, 0x00, "2"                       },
	{0x11, 0x01, 0x01, 0x01, "3"                       },
	
	{0   , 0xfe, 0   , 2   , "Time"                   },
	{0x11, 0x01, 0x20, 0x20, "60 Seconds"             },
	{0x11, 0x01, 0x20, 0x00, "90 Seconds"             },
	
	{0   , 0xfe, 0   , 4   , "Starting Balls"         },
	{0x11, 0x01, 0xc0, 0x00, "3"                      },
	{0x11, 0x01, 0xc0, 0xc0, "4"                      },
	{0x11, 0x01, 0xc0, 0x40, "5"                      },
	{0x11, 0x01, 0xc0, 0x80, "6"                      },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x01, 0x01, "Off"                    },
	{0x12, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x12, 0x01, 0x06, 0x00, "Easy"                   },
	{0x12, 0x01, 0x06, 0x06, "Normal"                 },
	{0x12, 0x01, 0x06, 0x02, "Hard"                   },
	{0x12, 0x01, 0x06, 0x04, "Very Hard"              },
	
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x12, 0x01, 0x38, 0x00, "5 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x20, "4 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x10, "3 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x30, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credit"       },
	{0x12, 0x01, 0x38, 0x28, "2 Coins 3 Credits"      },
	{0x12, 0x01, 0x38, 0x18, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0x38, 0x08, "1 Coin  3 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Chokchok)

static struct BurnDIPInfo WlstarDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                     },
	{0x14, 0xff, 0xff, 0x7f, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "2 Players Game"         },
	{0x13, 0x01, 0x10, 0x00, "1 Credit"               },
	{0x13, 0x01, 0x10, 0x10, "2 Credits"              },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0xc0, 0x00, "Easy"                   },
	{0x13, 0x01, 0xc0, 0xc0, "Normal"                 },
	{0x13, 0x01, 0xc0, 0x40, "Hard"                   },
	{0x13, 0x01, 0xc0, 0x80, "Hardest"                },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Last Inning"            },
	{0x14, 0x01, 0x01, 0x00, "9"                      },
	{0x14, 0x01, 0x01, 0x01, "12"                     },
	
	{0   , 0xfe, 0   , 2   , "Versus CPU Game Ends"   },
	{0x14, 0x01, 0x02, 0x02, "+10"                    },
	{0x14, 0x01, 0x02, 0x00, "+7"                     },
	
	{0   , 0xfe, 0   , 2   , "Versus Game"            },
	{0x14, 0x01, 0x04, 0x00, "1 Credit / 2 Innings"   },
	{0x14, 0x01, 0x04, 0x04, "1 Credit / 3 Innings"   },	
	
	{0   , 0xfe, 0   , 2   , "Full 2 Players Game"    },
	{0x14, 0x01, 0x08, 0x00, "4 Credits"              },
	{0x14, 0x01, 0x08, 0x08, "6 Credits"              },

	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x14, 0x01, 0x70, 0x00, "5 Coins 1 Credit"       },
	{0x14, 0x01, 0x70, 0x40, "4 Coins 1 Credit"       },
	{0x14, 0x01, 0x70, 0x20, "3 Coins 1 Credit"       },
	{0x14, 0x01, 0x70, 0x60, "2 Coins 1 Credit"       },
	{0x14, 0x01, 0x70, 0x70, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x70, 0x50, "2 Coins 3 Credits"      },
	{0x14, 0x01, 0x70, 0x30, "1 Coin  2 Credits"      },
	{0x14, 0x01, 0x70, 0x10, "1 Coin  3 Credits"      },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x14, 0x01, 0x80, 0x80, "Off"                    },
	{0x14, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Wlstar)

static struct BurnDIPInfo Wondl96DIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0x7f, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Field Colour"           },
	{0x13, 0x01, 0x10, 0x10, "Blue"                   },
	{0x13, 0x01, 0x10, 0x00, "Green"                  },
	
	{0   , 0xfe, 0   , 2   , "Versus CPU Game Ends"   },
	{0x13, 0x01, 0x20, 0x20, "+10"                    },
	{0x13, 0x01, 0x20, 0x00, "+7"                     },
	
	{0   , 0xfe, 0   , 2   , "Versus Game"            },
	{0x13, 0x01, 0x40, 0x00, "1 Credit / 2 Innings"   },
	{0x13, 0x01, 0x40, 0x40, "1 Credit / 3 Innings"   },
	
	{0   , 0xfe, 0   , 2   , "Full 2 Players Game"    },
	{0x13, 0x01, 0x80, 0x80, "4 Credits"              },
	{0x13, 0x01, 0x80, 0x00, "6 Credits"              },
	
	// Dip 2
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x14, 0x01, 0x0e, 0x04, "Level 1"                },
	{0x14, 0x01, 0x0e, 0x08, "Level 2"                },
	{0x14, 0x01, 0x0e, 0x00, "Level 3"                },
	{0x14, 0x01, 0x0e, 0x0e, "Level 4"                },
	{0x14, 0x01, 0x0e, 0x06, "Level 5"                },
	{0x14, 0x01, 0x0e, 0x0a, "Level 6"                },
	{0x14, 0x01, 0x0e, 0x02, "Level 7"                },
	{0x14, 0x01, 0x0e, 0x0c, "Level 8"                },

	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x14, 0x01, 0x70, 0x00, "5 Coins 1 Credit"       },
	{0x14, 0x01, 0x70, 0x40, "4 Coins 1 Credit"       },
	{0x14, 0x01, 0x70, 0x20, "3 Coins 1 Credit"       },
	{0x14, 0x01, 0x70, 0x60, "2 Coins 1 Credit"       },
	{0x14, 0x01, 0x70, 0x70, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0x70, 0x50, "2 Coins 3 Credits"      },
	{0x14, 0x01, 0x70, 0x30, "1 Coin  2 Credits"      },
	{0x14, 0x01, 0x70, 0x10, "1 Coin  3 Credits"      },
};

STDDIPINFO(Wondl96)

static struct BurnDIPInfo FncywldDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xf7, NULL                     },
	{0x15, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x14, 0x01, 0xe0, 0x20, "4 Coins 1 Credit"       },
	{0x14, 0x01, 0xe0, 0x40, "3 Coins 1 Credit"       },
	{0x14, 0x01, 0xe0, 0x60, "2 Coins 1 Credit"       },
	{0x14, 0x01, 0xe0, 0x00, "2 Coins 1 Credit"       },
	{0x14, 0x01, 0xe0, 0xe0, "1 Coin  1 Credit"       },
	{0x14, 0x01, 0xe0, 0xc0, "1 Coin  2 Credits"      },
	{0x14, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits"      },
	{0x14, 0x01, 0xe0, 0x80, "1 Coin  4 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x14, 0x01, 0x10, 0x00, "Off"                    },
	{0x14, 0x01, 0x10, 0x10, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x14, 0x01, 0x08, 0x08, "Off"                    },
	{0x14, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Language"               },
	{0x14, 0x01, 0x04, 0x04, "English"                },
	{0x14, 0x01, 0x04, 0x00, "Korean"                 },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x14, 0x01, 0x02, 0x02, "Off"                    },
	{0x14, 0x01, 0x02, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "2 Coins to Start, 1 to Continue" },
	{0x14, 0x01, 0x01, 0x01, "Off"                    },
	{0x14, 0x01, 0x01, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x15, 0x01, 0xc0, 0x80, "1"                      },
	{0x15, 0x01, 0xc0, 0x00, "2"                      },
	{0x15, 0x01, 0xc0, 0xc0, "3"                      },
	{0x15, 0x01, 0xc0, 0x40, "4"                      },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x15, 0x01, 0x30, 0x30, "Easy"                   },
	{0x15, 0x01, 0x30, 0x20, "Normal"                 },
	{0x15, 0x01, 0x30, 0x10, "Hard"                   },
	{0x15, 0x01, 0x30, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x15, 0x01, 0x01, 0x01, "Off"                    },
	{0x15, 0x01, 0x01, 0x00, "On"                     },	
};

STDDIPINFO(Fncywld)

static struct BurnDIPInfo SdfightDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0x7f, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x13, 0x01, 0x0e, 0x04, "1"                      },
	{0x13, 0x01, 0x0e, 0x08, "2"                      },
	{0x13, 0x01, 0x0e, 0x00, "3"                      },
	{0x13, 0x01, 0x0e, 0x0e, "4"                      },
	{0x13, 0x01, 0x0e, 0x06, "5"                      },
	{0x13, 0x01, 0x0e, 0x0a, "6"                      },
	{0x13, 0x01, 0x0e, 0x02, "7"                      },
	{0x13, 0x01, 0x0e, 0x0c, "8"                      },
	
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x13, 0x01, 0x70, 0x00, "5 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x40, "4 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x20, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x60, "2 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x70, "1 Coin  1 Credit"       },
	{0x13, 0x01, 0x70, 0x50, "2 Coins 3 Credits"      },
	{0x13, 0x01, 0x70, 0x30, "1 Coin  2 Credits"      },
	{0x13, 0x01, 0x70, 0x10, "1 Coin  3 Credits"      },
		
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x14, 0x01, 0x01, 0x01, "Off"                    },
	{0x14, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Winning Rounds"         },
	{0x14, 0x01, 0x08, 0x08, "2"                      },
	{0x14, 0x01, 0x08, 0x00, "3"                      },
	
	{0   , 0xfe, 0   , 4   , "Time"                   },
	{0x14, 0x01, 0xc0, 0x40, "30"                     },
	{0x14, 0x01, 0xc0, 0x80, "50"                     },
	{0x14, 0x01, 0xc0, 0xc0, "70"                     },
	{0x14, 0x01, 0xc0, 0x00, "90"                     },
};

STDDIPINFO(Sdfight)

static struct BurnDIPInfo BcstryDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0x7f, NULL                     },
	{0x14, 0xff, 0xff, 0xdf, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x13, 0x01, 0x0e, 0x04, "1"                      },
	{0x13, 0x01, 0x0e, 0x08, "2"                      },
	{0x13, 0x01, 0x0e, 0x00, "3"                      },
	{0x13, 0x01, 0x0e, 0x0e, "4"                      },
	{0x13, 0x01, 0x0e, 0x06, "5"                      },
	{0x13, 0x01, 0x0e, 0x0a, "6"                      },
	{0x13, 0x01, 0x0e, 0x02, "7"                      },
	{0x13, 0x01, 0x0e, 0x0c, "8"                      },
	
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x13, 0x01, 0x70, 0x00, "5 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x40, "4 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x20, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x60, "2 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x70, "1 Coin  1 Credit"       },
	{0x13, 0x01, 0x70, 0x50, "2 Coins 3 Credits"      },
	{0x13, 0x01, 0x70, 0x30, "1 Coin  2 Credits"      },
	{0x13, 0x01, 0x70, 0x10, "1 Coin  3 Credits"      },
		
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x14, 0x01, 0x01, 0x01, "Off"                    },
	{0x14, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Event Selection"        },
	{0x14, 0x01, 0x20, 0x20, "Off"                    },
	{0x14, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Control Type"           },
	{0x14, 0x01, 0x40, 0x40, "Joysticks & Buttons"    },
	{0x14, 0x01, 0x40, 0x00, "Buttons"                },
	
	{0   , 0xfe, 0   , 2   , "Debug Mode"             },
	{0x14, 0x01, 0x80, 0x80, "Off"                    },
	{0x14, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Bcstry)

static struct BurnDIPInfo SemibaseDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0x7f, NULL                     },
	{0x16, 0xff, 0xff, 0xdf, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x15, 0x01, 0x01, 0x01, "Off"                    },
	{0x15, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x15, 0x01, 0x0e, 0x04, "1"                      },
	{0x15, 0x01, 0x0e, 0x08, "2"                      },
	{0x15, 0x01, 0x0e, 0x00, "3"                      },
	{0x15, 0x01, 0x0e, 0x0e, "4"                      },
	{0x15, 0x01, 0x0e, 0x06, "5"                      },
	{0x15, 0x01, 0x0e, 0x0a, "6"                      },
	{0x15, 0x01, 0x0e, 0x02, "7"                      },
	{0x15, 0x01, 0x0e, 0x0c, "8"                      },
	
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x15, 0x01, 0x70, 0x00, "5 Coins 1 Credit"       },
	{0x15, 0x01, 0x70, 0x40, "4 Coins 1 Credit"       },
	{0x15, 0x01, 0x70, 0x20, "3 Coins 1 Credit"       },
	{0x15, 0x01, 0x70, 0x60, "2 Coins 1 Credit"       },
	{0x15, 0x01, 0x70, 0x70, "1 Coin  1 Credit"       },
	{0x15, 0x01, 0x70, 0x50, "2 Coins 3 Credits"      },
	{0x15, 0x01, 0x70, 0x30, "1 Coin  2 Credits"      },
	{0x15, 0x01, 0x70, 0x10, "1 Coin  3 Credits"      },
		
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x15, 0x01, 0x80, 0x80, "Off"                    },
	{0x15, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x16, 0x01, 0x01, 0x01, "Off"                    },
	{0x16, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "vs CPU Game Ends"       },
	{0x16, 0x01, 0x20, 0x20, "+10"                    },
	{0x16, 0x01, 0x20, 0x00, "+7"                     },
	
	{0   , 0xfe, 0   , 2   , "Vs Game"                },
	{0x16, 0x01, 0x40, 0x40, "1 Credit / 2 Innings"   },
	{0x16, 0x01, 0x40, 0x00, "1 Credit / 3 Innings"   },
	
	{0   , 0xfe, 0   , 2   , "Full 2 Players Game"    },
	{0x16, 0x01, 0x80, 0x00, "4 Credits"              },
	{0x16, 0x01, 0x80, 0x80, "6 Credits"              },
};

STDDIPINFO(Semibase)

static struct BurnDIPInfo DquizgoDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0x7f, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x13, 0x01, 0x0e, 0x04, "1"                      },
	{0x13, 0x01, 0x0e, 0x08, "2"                      },
	{0x13, 0x01, 0x0e, 0x00, "3"                      },
	{0x13, 0x01, 0x0e, 0x0e, "4"                      },
	{0x13, 0x01, 0x0e, 0x06, "5"                      },
	{0x13, 0x01, 0x0e, 0x0a, "6"                      },
	{0x13, 0x01, 0x0e, 0x02, "7"                      },
	{0x13, 0x01, 0x0e, 0x0c, "8"                      },
	
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x13, 0x01, 0x70, 0x00, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x40, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x20, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x60, "2 Coins 1 Credit"       },
	{0x13, 0x01, 0x70, 0x70, "1 Coin  1 Credit"       },
	{0x13, 0x01, 0x70, 0x50, "2 Coins 3 Credits"      },
	{0x13, 0x01, 0x70, 0x30, "1 Coin  2 Credits"      },
	{0x13, 0x01, 0x70, 0x10, "1 Coin  3 Credits"      },
		
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x14, 0x01, 0x01, 0x01, "Off"                    },
	{0x14, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0xc0, 0x00, "2"                    },
	{0x14, 0x01, 0xc0, 0xc0, "3"                    },
	{0x14, 0x01, 0xc0, 0x40, "4"                    },
	{0x14, 0x01, 0xc0, 0x80, "5"                    },
};

STDDIPINFO(Dquizgo)

static struct BurnDIPInfo JumppopDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0xfe, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x11, 0x01, 0x01, 0x01, "Off"                    },
	{0x11, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x11, 0x01, 0x02, 0x02, "Off"                    },
	{0x11, 0x01, 0x02, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x11, 0x01, 0xe0, 0x00, "3 Coins 1 Credit"       },
	{0x11, 0x01, 0xe0, 0x80, "2 Coins 1 Credit"       },
	{0x11, 0x01, 0xe0, 0xe0, "1 Coin  1 Credit"       },
	{0x11, 0x01, 0xe0, 0x60, "1 Coin  2 Credits"      },
	{0x11, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits"      },
	{0x11, 0x01, 0xe0, 0x20, "1 Coin  4 Credits"      },
	{0x11, 0x01, 0xe0, 0xc0, "1 Coin  5 Credits"      },
	{0x11, 0x01, 0xe0, 0x40, "1 Coin  6 Credits"      },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x11, 0x01, 0x1c, 0x00, "3 Coins 1 Credit"       },
	{0x11, 0x01, 0x1c, 0x10, "2 Coins 1 Credit"       },
	{0x11, 0x01, 0x1c, 0x1c, "1 Coin  1 Credit"       },
	{0x11, 0x01, 0x1c, 0x0c, "1 Coin  2 Credits"      },
	{0x11, 0x01, 0x1c, 0x14, "1 Coin  3 Credits"      },
	{0x11, 0x01, 0x1c, 0x04, "1 Coin  4 Credits"      },
	{0x11, 0x01, 0x1c, 0x18, "1 Coin  5 Credits"      },
	{0x11, 0x01, 0x1c, 0x08, "1 Coin  6 Credits"      },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x01, 0x01, "Off"                    },
	{0x12, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x12, 0x01, 0x02, 0x00, "Off"                    },
	{0x12, 0x01, 0x02, 0x02, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Picture Viewer"         },
	{0x12, 0x01, 0x04, 0x04, "Off"                    },
	{0x12, 0x01, 0x04, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Background Type"        },
	{0x12, 0x01, 0x08, 0x08, "1"                      },
	{0x12, 0x01, 0x08, 0x00, "2"                      },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x12, 0x01, 0x30, 0x20, "Easy"                   },
	{0x12, 0x01, 0x30, 0x30, "Normal"                 },
	{0x12, 0x01, 0x30, 0x10, "Hard"                   },
	{0x12, 0x01, 0x30, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0xc0, 0x80, "1"                      },
	{0x12, 0x01, 0xc0, 0x00, "2"                      },
	{0x12, 0x01, 0xc0, 0xc0, "3"                      },
	{0x12, 0x01, 0xc0, 0x40, "4"                      },	
};

STDDIPINFO(Jumppop)

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
	DrvClearOpposites(&DrvInput[0]);
	DrvClearOpposites(&DrvInput[1]);
}

static struct BurnRomInfo TumblebRomDesc[] = {
	{ "thumbpop.12",   0x40000, 0x0c984703, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "thumbpop.13",   0x40000, 0x864c4053, BRF_ESS | BRF_PRG }, //	 1
	
	{ "thumbpop.19",   0x40000, 0x0795aab4, BRF_GRA },	     //  2	Tiles
	{ "thumbpop.18",   0x40000, 0xad58df43, BRF_GRA },	     //  3
	
	{ "map-01.rom",    0x80000, 0xe81ffa09, BRF_GRA },	     //  4	Sprites
	{ "map-00.rom",    0x80000, 0x8c879cfe, BRF_GRA },	     //  5
	
	{ "thumbpop.snd",  0x80000, 0xfabbf15d, BRF_SND },	     //  6	Samples
};

STD_ROM_PICK(Tumbleb)
STD_ROM_FN(Tumbleb)

static struct BurnRomInfo Tumbleb2RomDesc[] = {
	{ "thumbpop.2",    0x40000, 0x34b016e1, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "thumbpop.3",    0x40000, 0x89501c71, BRF_ESS | BRF_PRG }, //	 1
	
	{ "thumbpop.19",   0x40000, 0x0795aab4, BRF_GRA },	     //  2	Tiles
	{ "thumbpop.18",   0x40000, 0xad58df43, BRF_GRA },	     //  3
	
	{ "map-01.rom",    0x80000, 0xe81ffa09, BRF_GRA },	     //  4	Sprites
	{ "map-00.rom",    0x80000, 0x8c879cfe, BRF_GRA },	     //  5
	
	{ "thumbpop.snd",  0x80000, 0xfabbf15d, BRF_SND },	     //  6	Samples
	
	{ "pic_16c57",     0x02d4c, 0x00000000, BRF_NODUMP },
};

STD_ROM_PICK(Tumbleb2)
STD_ROM_FN(Tumbleb2)

static struct BurnRomInfo JumpkidsRomDesc[] = {
	{ "23-ic29.15c",   0x40000, 0x6ba11e91, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "24-ic30.17c",   0x40000, 0x5795d98b, BRF_ESS | BRF_PRG }, //	 1
	
	{ "22-ic19.3c",    0x08000, 0xbd619530, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
	{ "30-ic125.15j",  0x40000, 0x44b9a089, BRF_GRA },	     //  3	Tiles
	{ "29-ic124.13j",  0x40000, 0x3f98ec69, BRF_GRA },	     //  4
	
	{ "25-ic69.1g",    0x40000, 0x176ae857, BRF_GRA },	     //  5	Sprites
	{ "28-ic131.1l",   0x40000, 0xed837757, BRF_GRA },	     //  6
	{ "26-ic70.2g",    0x40000, 0xe8b34980, BRF_GRA },	     //  7
	{ "27-ic100.1j",   0x40000, 0x3918dda3, BRF_GRA },	     //  8
	
	{ "21-ic17.1c",    0x80000, 0xe5094f75, BRF_SND },	     //  9	Samples
	{ "ic18.2c",       0x20000, 0xa63736c3, BRF_SND },	     //  10
};

STD_ROM_PICK(Jumpkids)
STD_ROM_FN(Jumpkids)

static struct BurnRomInfo MetlsavrRomDesc[] = {
	{ "first-4.ub17",  0x40000, 0x667a494d, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "first-3.ub18",  0x40000, 0x87bf4ed2, BRF_ESS | BRF_PRG }, //	 1
	
	{ "first-2.ua7",   0x10000, 0x49505edf, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
	{ "protdata.bin",  0x00200, 0x17aa17a9, BRF_ESS | BRF_PRG }, //  3	Shared RAM Data
	
	{ "first-5.rom5",  0x40000, 0xdd4af746, BRF_GRA },	     //  4	Tiles
	{ "first-6.rom6",  0x40000, 0x808b0e0b, BRF_GRA },	     //  5
	
	{ "first-7.uor1",  0x80000, 0xa6816747, BRF_GRA },	     //  6	Sprites
	{ "first-8.uor2",  0x80000, 0x377020e5, BRF_GRA },	     //  7
	{ "first-9.uor3",  0x80000, 0xfccf1bb7, BRF_GRA },	     //  8
	{ "first-10.uor4", 0x80000, 0xa22b587b, BRF_GRA },	     //  9
	
	{ "first-1.uc1",   0x40000, 0xe943dacb, BRF_SND },	     //  10	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },	     //  11
};

STD_ROM_PICK(Metlsavr)
STD_ROM_FN(Metlsavr)

static struct BurnRomInfo PangpangRomDesc[] = {
	{ "2.bin",         0x40000, 0x45436666, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "3.bin",         0x40000, 0x2725cbe7, BRF_ESS | BRF_PRG }, //	 1
	
	{ "11.bin",        0x40000, 0xa2b9fec8, BRF_GRA },	     //  2	Tiles
	{ "10.bin",        0x40000, 0x4f59d7b9, BRF_GRA },	     //  3
	{ "6.bin",         0x40000, 0x1ebbc4f1, BRF_GRA },	     //  4
	{ "7.bin",         0x40000, 0xcd544173, BRF_GRA },	     //  5
	
	{ "8.bin",         0x40000, 0xea0fa1e0, BRF_GRA },	     //  6	Sprites
	{ "9.bin",         0x40000, 0x1da5fe49, BRF_GRA },	     //  7
	{ "4.bin",         0x40000, 0x4f282eb1, BRF_GRA },	     //  8
	{ "5.bin",         0x40000, 0x00694df9, BRF_GRA },	     //  9
	
	{ "1.bin",         0x80000, 0xe722bb02, BRF_SND },	     //  10	Samples
	
	{ "pic_16c57",     0x02d4c, 0x1ca515b4, BRF_OPT },
};

STD_ROM_PICK(Pangpang)
STD_ROM_FN(Pangpang)

static struct BurnRomInfo SuprtrioRomDesc[] = {
	{ "rom2",          0x40000, 0x4102e59d, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "rom1",          0x40000, 0xcc3a83c3, BRF_ESS | BRF_PRG }, //	 1
	
	{ "rom4l",         0x10000, 0x466aa96d, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
	{ "rom4",          0x80000, 0xcd2dfae4, BRF_GRA },	     //  3	Tiles
	{ "rom5",          0x80000, 0x4e64da64, BRF_GRA },	     //  4
	
	{ "rom9l",         0x40000, 0xcc45f437, BRF_GRA },	     //  5	Sprites
	{ "rom8l",         0x40000, 0x9bc90169, BRF_GRA },	     //  6
	{ "rom7l",         0x40000, 0xbfc7c756, BRF_GRA },	     //  7
	{ "rom6l",         0x40000, 0xbb3499af, BRF_GRA },	     //  8
	
	{ "rom3h",         0x80000, 0x34ea7ec9, BRF_SND },	     //  9	Samples
	{ "rom3l",         0x20000, 0x1b73233b, BRF_SND },	     //  10
};

STD_ROM_PICK(Suprtrio)
STD_ROM_FN(Suprtrio)

static struct BurnRomInfo HtchctchRomDesc[] = {
	{ "p04.b17",       0x20000, 0x6991483a, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "p03.b16",       0x20000, 0xeff14c40, BRF_ESS | BRF_PRG }, //	 1
	
	{ "p02.b5",        0x10000, 0xc5a03186, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
	{ "protdata.bin",  0x00200, 0x5b27adb6, BRF_ESS | BRF_PRG }, //  3	Shared RAM Data
	
	{ "p06srom5.bin",  0x40000, 0x3d2cbb0d, BRF_GRA },	     //  4	Tiles
	{ "p07srom6.bin",  0x40000, 0x0207949c, BRF_GRA },	     //  5
	
	{ "p08uor1.bin",   0x20000, 0x6811e7b6, BRF_GRA },	     //  6	Sprites
	{ "p09uor2.bin",   0x20000, 0x1c6549cf, BRF_GRA },	     //  7
	{ "p10uor3.bin",   0x20000, 0x6462e6e0, BRF_GRA },	     //  8
	{ "p11uor4.bin",   0x20000, 0x9c511d98, BRF_GRA },	     //  9
	
	{ "p01.c1",        0x20000, 0x18c06829, BRF_SND },	     //  10	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },	     //  11
};

STD_ROM_PICK(Htchctch)
STD_ROM_FN(Htchctch)

static struct BurnRomInfo CookbibRomDesc[] = {
	{ "prg2.ub17",     0x20000, 0x2664a335, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "prg1.ub16",     0x20000, 0xcda6335f, BRF_ESS | BRF_PRG }, //	 1
	
	{ "prg-s.ub5",     0x10000, 0x547d6ea3, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
	{ "protdata.bin",  0x00200, 0xa77d13f4, BRF_ESS | BRF_PRG }, //  3	Shared RAM Data
	
	{ "srom5.bin",     0x40000, 0x73a46e43, BRF_GRA },	     //  4	Tiles
	{ "srom6.bin",     0x40000, 0xade2dbec, BRF_GRA },	     //  5
	
	{ "uor1.bin",      0x20000, 0xa7d91f23, BRF_GRA },	     //  6	Sprites
	{ "uor2.bin",      0x20000, 0x9aacbec2, BRF_GRA },	     //  7
	{ "uor3.bin",      0x20000, 0x3fee0c3c, BRF_GRA },	     //  8
	{ "uor4.bin",      0x20000, 0xbed9ed2d, BRF_GRA },	     //  9
	
	{ "sound.uc1",     0x20000, 0x545e19b6, BRF_SND },	     //  10	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },	     //  11
};

STD_ROM_PICK(Cookbib)
STD_ROM_FN(Cookbib)

static struct BurnRomInfo ChokchokRomDesc[] = {
	{ "ub17.bin",      0x40000, 0xecdb45ca, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ub18.bin",      0x40000, 0xb183852a, BRF_ESS | BRF_PRG }, //	 1
	
	{ "ub5.bin",       0x10000, 0x30c2171d, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
	{ "protdata.bin",  0x00200, 0x0bd39834, BRF_ESS | BRF_PRG }, //  3	Shared RAM Data
	
	{ "srom5.bin",     0x80000, 0x836608b8, BRF_GRA },	     //  4	Tiles
	{ "srom6.bin",     0x80000, 0x31d5715d, BRF_GRA },	     //  5
	
	{ "uor1.bin",      0x80000, 0xded6642a, BRF_GRA },	     //  6	Sprites
	{ "uor2.bin",      0x80000, 0x493f9516, BRF_GRA },	     //  7
	{ "uor3.bin",      0x80000, 0xe2dc3e12, BRF_GRA },	     //  8
	{ "uor4.bin",      0x80000, 0x6f377530, BRF_GRA },	     //  9
	
	{ "uc1.bin",       0x40000, 0xf3f57abd, BRF_SND },	     //  10	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },	     //  11
};

STD_ROM_PICK(Chokchok)
STD_ROM_FN(Chokchok)

static struct BurnRomInfo WlstarRomDesc[] = {
	{ "n-4.u817",      0x40000, 0xfc3e829b, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "n-3.u818",      0x40000, 0xf01bc623, BRF_ESS | BRF_PRG }, //	 1
	
	{ "ua7",           0x10000, 0x90cafa5f, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
#if !defined (ROM_VERIFY)
	{ "protdata.bin",  0x00200, 0xb7ffde5b, BRF_ESS | BRF_PRG }, //  3	Shared RAM Data
#else
	{ "87c52.mcu",     0x02000, 0xab5e2a7e, BRF_ESS | BRF_PRG }, //  3	MCU Program Code
#endif
	
	{ "5.srom5",       0x80000, 0xf7f8c859, BRF_GRA },	     //  4	Tiles
	{ "6.srom6",       0x80000, 0x34ace2a8, BRF_GRA },	     //  5
	
	{ "7.udr1",        0x80000, 0x6e47c31d, BRF_GRA },	     //  6	Sprites
	{ "8.udr2",        0x80000, 0x09c5d57c, BRF_GRA },	     //  7
	{ "9.udr3",        0x80000, 0x3ec064f0, BRF_GRA },	     //  8
	{ "10.udr4",       0x80000, 0xb4693cdd, BRF_GRA },	     //  9
	
	{ "ua1",           0x40000, 0xde217d30, BRF_SND },	     //  10	Samples
};

STD_ROM_PICK(Wlstar)
STD_ROM_FN(Wlstar)

static struct BurnRomInfo Wondl96RomDesc[] = {
	{ "ub17.bin",      0x40000, 0x41d8e03c, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ub18.bin",      0x40000, 0x0e4963af, BRF_ESS | BRF_PRG }, //	 1	
	
	{ "ub5.bin",       0x10000, 0xd99d19c4, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
#if !defined (ROM_VERIFY)
	{ "protdata.bin",  0x00200, 0xd7578b1e, BRF_ESS | BRF_PRG }, //  3	Shared RAM Data
#else
	{ "87c52.mcu",     0x02000, 0x6f4c659a, BRF_ESS | BRF_PRG }, //  3	MCU Program Code
#endif
	
	{ "srom5.bin",     0x80000, 0xdb8010c3, BRF_GRA },	     //  4	Tiles
	{ "srom6.bin",     0x80000, 0x2f364e54, BRF_GRA },	     //  5
	
	{ "uor1.bin",      0x80000, 0xe1e9eebb, BRF_GRA },	     //  6	Sprites
	{ "uor2.bin",      0x80000, 0xddebfe83, BRF_GRA },	     //  7
	{ "uor3.bin",      0x80000, 0x7efe4d67, BRF_GRA },	     //  8
	{ "uor4.bin",      0x80000, 0x7b1596d1, BRF_GRA },	     //  9
	
	{ "uc1.bin",       0x40000, 0x0e7913e6, BRF_SND },	     //  10	Samples
};

STD_ROM_PICK(Wondl96)
STD_ROM_FN(Wondl96)

static struct BurnRomInfo FncywldRomDesc[] = {
	{ "01_fw02.bin",   0x80000, 0xecb978c1, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "02_fw03.bin",   0x80000, 0x2d233b42, BRF_ESS | BRF_PRG }, //	 1
	
	{ "08_fw09.bin",   0x40000, 0xa4a00de9, BRF_GRA },	     //  2	Tiles
	{ "07_fw08.bin",   0x40000, 0xb48cd1d4, BRF_GRA },	     //  3
	{ "10_fw11.bin",   0x40000, 0xf21bab48, BRF_GRA },	     //  4
	{ "09_fw10.bin",   0x40000, 0x6aea8e0f, BRF_GRA },	     //  5
	
	{ "05_fw06.bin",   0x40000, 0xe141ecdc, BRF_GRA },	     //  6	Sprites
	{ "06_fw07.bin",   0x40000, 0x0058a812, BRF_GRA },	     //  7
	{ "03_fw04.bin",   0x40000, 0x6ad38c14, BRF_GRA },	     //  8
	{ "04_fw05.bin",   0x40000, 0xb8d079a6, BRF_GRA },	     //  9
	
	{ "00_fw01.bin",   0x40000, 0xb395fe01, BRF_SND },	     //  10	Samples
};

STD_ROM_PICK(Fncywld)
STD_ROM_FN(Fncywld)

static struct BurnRomInfo SdfightRomDesc[] = {
	{ "u817",          0x80000, 0x9f284f4d, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "u818",          0x80000, 0xa60e5b22, BRF_ESS | BRF_PRG }, //	 1
	
	{ "ua7",           0x10000, 0xc3d36da4, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
	{ "protdata.bin",  0x00200, 0xefb8b822, BRF_ESS | BRF_PRG }, //  3	Shared RAM Data
	
	{ "9.ug11",        0x80000, 0xbf809ccd, BRF_GRA },	     //  4	Tiles
	{ "10.ug12",       0x80000, 0xa5a3bfa2, BRF_GRA },	     //  5
	{ "15.ui11",       0x80000, 0x3bc8aa6d, BRF_GRA },	     //  6
	{ "16.ui12",       0x80000, 0x71e6b78d, BRF_GRA },	     //  7
	
	{ "11.uk2",        0x80000, 0xd006fadc, BRF_GRA },	     //  8	Sprites
	{ "12.uk3",        0x80000, 0x2a2f4153, BRF_GRA },	     //  9
	{ "5.uj2",         0x80000, 0xf1246cbf, BRF_GRA },	     //  10
	{ "6.uj3",         0x80000, 0xd346878c, BRF_GRA },	     //  11
	{ "13.uk4",        0x80000, 0x9bc40774, BRF_GRA },	     //  12
	{ "14.uk5",        0x80000, 0xa1e61674, BRF_GRA },	     //  13
	{ "7.uj4",         0x80000, 0xdbdece8a, BRF_GRA },	     //  14
	{ "8.uj5",         0x80000, 0x60be7dd1, BRF_GRA },	     //  15
	
	{ "uc1",           0x40000, 0x535cae2c, BRF_SND },	     //  16	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },	     //  17
};

STD_ROM_PICK(Sdfight)
STD_ROM_FN(Sdfight)

static struct BurnRomInfo BcstryRomDesc[] = {
	{ "bcstry_u.62",   0x40000, 0x7f7aa244, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "bcstry_u.35",   0x40000, 0xd25b80a4, BRF_ESS | BRF_PRG }, //	 1
	
	{ "bcstry_u.21",   0x10000, 0x3ba072d4, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
	{ "protdata.bin",  0x00200, 0xe84e328c, BRF_ESS | BRF_PRG }, //  3	Shared RAM Data
	
	{ "bcstry_u.109",  0x80000, 0xeb04d37a, BRF_GRA },	     //  4	Tiles
	{ "bcstry_u.113",  0x80000, 0x746ecdd7, BRF_GRA },	     //  5
	{ "bcstry_u.110",  0x80000, 0x1bfe65c3, BRF_GRA },	     //  6
	{ "bcstry_u.111",  0x80000, 0xc8bf3a3c, BRF_GRA },	     //  7
	
	{ "bcstry_u.100",  0x80000, 0x8c11cbed, BRF_GRA },	     //  8	Sprites
	{ "bcstry_u.106",  0x80000, 0x5219bcbf, BRF_GRA },	     //  9
	{ "bcstry_u.99",   0x80000, 0xcdb1af87, BRF_GRA },	     //  10
	{ "bcstry_u.105",  0x80000, 0x8166b596, BRF_GRA },	     //  11
	{ "bcstry_u.104",  0x80000, 0x377c0c71, BRF_GRA },	     //  12
	{ "bcstry_u.108",  0x80000, 0x442307ed, BRF_GRA },	     //  13
	{ "bcstry_u.102",  0x80000, 0x71b40ece, BRF_GRA },	     //  14
	{ "bcstry_u.107",  0x80000, 0xab3c923a, BRF_GRA },	     //  15
	
	{ "bcstry_u.64",   0x40000, 0x23f0e0fe, BRF_SND },	     //  16	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },	     //  17
};

STD_ROM_PICK(Bcstry)
STD_ROM_FN(Bcstry)

static struct BurnRomInfo BcstryaRomDesc[] = {
	{ "prg2.ic62",     0x40000, 0xf54c0a96, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "prg1.ic35",     0x40000, 0x2c55100a, BRF_ESS | BRF_PRG }, //	 1
	
	{ "bcstry_u.21",   0x10000, 0x3ba072d4, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
	{ "protdata.bin",  0x00200, 0xe84e328c, BRF_ESS | BRF_PRG }, //  3	Shared RAM Data
	
	{ "bcstry_u.109",  0x80000, 0xeb04d37a, BRF_GRA },	     //  4	Tiles
	{ "bcstry_u.113",  0x80000, 0x746ecdd7, BRF_GRA },	     //  5
	{ "bcstry_u.110",  0x80000, 0x1bfe65c3, BRF_GRA },	     //  6
	{ "bcstry_u.111",  0x80000, 0xc8bf3a3c, BRF_GRA },	     //  7
	
	{ "bcstry_u.100",  0x80000, 0x8c11cbed, BRF_GRA },	     //  8	Sprites
	{ "bcstry_u.106",  0x80000, 0x5219bcbf, BRF_GRA },	     //  9
	{ "bcstry_u.99",   0x80000, 0xcdb1af87, BRF_GRA },	     //  10
	{ "bcstry_u.105",  0x80000, 0x8166b596, BRF_GRA },	     //  11
	{ "bcstry_u.104",  0x80000, 0x377c0c71, BRF_GRA },	     //  12
	{ "bcstry_u.108",  0x80000, 0x442307ed, BRF_GRA },	     //  13
	{ "bcstry_u.102",  0x80000, 0x71b40ece, BRF_GRA },	     //  14
	{ "bcstry_u.107",  0x80000, 0xab3c923a, BRF_GRA },	     //  15
	
	{ "bcstry_u.64",   0x40000, 0x23f0e0fe, BRF_SND },	     //  16	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },	     //  17
};

STD_ROM_PICK(Bcstrya)
STD_ROM_FN(Bcstrya)

static struct BurnRomInfo SemibaseRomDesc[] = {
	{ "ic62.68k",      0x40000, 0x85ea81c3, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ic35.68k",      0x40000, 0xd2249605, BRF_ESS | BRF_PRG }, //	 1
	
	{ "ic21.z80",      0x10000, 0xd95c64d0, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
	{ "protdata.bin",  0x00200, 0xecbf2163, BRF_ESS | BRF_PRG }, //  3	Shared RAM Data
	
	{ "ic109.gfx",     0x80000, 0x2b86e983, BRF_GRA },	     //  4	Tiles
	{ "ic113.gfx",     0x80000, 0xe39b6610, BRF_GRA },	     //  5
	{ "ic110.gfx",     0x80000, 0xbba4a015, BRF_GRA },	     //  6
	{ "ic111.gfx",     0x80000, 0x61133b63, BRF_GRA },	     //  7
	
	{ "ic100.gfx",     0x80000, 0x01c3d12a, BRF_GRA },	     //  8	Sprites
	{ "ic106.gfx",     0x80000, 0xdb282ac2, BRF_GRA },	     //  9
	{ "ic99.gfx",      0x80000, 0x349df821, BRF_GRA },	     //  10
	{ "ic105.gfx",     0x80000, 0xf7caa81c, BRF_GRA },	     //  11
	{ "ic104.gfx",     0x80000, 0x51a5d38a, BRF_GRA },	     //  12
	{ "ic108.gfx",     0x80000, 0xb253d60e, BRF_GRA },	     //  13
	{ "ic102.gfx",     0x80000, 0x3caefe97, BRF_GRA },	     //  14
	{ "ic107.gfx",     0x80000, 0x68109898, BRF_GRA },	     //  15
	
	{ "ic64.snd",      0x40000, 0x8a60649c, BRF_SND },	     //  16	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },	     //  17
};

STD_ROM_PICK(Semibase)
STD_ROM_FN(Semibase)

static struct BurnRomInfo DquizgoRomDesc[] = {
	{ "ub17",          0x80000, 0x0b96ab14, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ub18",          0x80000, 0x07f869f2, BRF_ESS | BRF_PRG }, //	 1
	
	{ "ub5",           0x10000, 0xe40481da, BRF_ESS | BRF_PRG }, //  2	Z80 Program Code
	
	{ "protdata.bin",  0x00200, 0x6064b9e0, BRF_ESS | BRF_PRG }, //  3	Shared RAM Data
	
	{ "srom5",         0x80000, 0xf1cdd21d, BRF_GRA },	     //  4	Tiles
	{ "srom6",         0x80000, 0xf848939e, BRF_GRA },	     //  5
	
	{ "uor1",          0x80000, 0xb4912bf6, BRF_GRA },	     //  6	Sprites
	{ "uor2",          0x80000, 0xb011cf93, BRF_GRA },	     //  7
	{ "uor3",          0x80000, 0xd96c3582, BRF_GRA },	     //  8
	{ "uor4",          0x80000, 0x77ff23eb, BRF_GRA },	     //  9
	
	{ "uc1",           0x40000, 0xd0f4c4ba, BRF_SND },	     //  10	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },	     //  11
};

STD_ROM_PICK(Dquizgo)
STD_ROM_FN(Dquizgo)

static struct BurnRomInfo JumppopRomDesc[] = {
	{ "68k_prg.bin",   0x080000, 0x123536b9, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	
	{ "z80_prg.bin",   0x040000, 0xa88d4424, BRF_ESS | BRF_PRG }, //  1	Z80 Program Code
	
	{ "bg0.bin",       0x100000, 0x35a1363d, BRF_GRA },	      //  2	Tiles
	{ "bg1.bin",       0x100000, 0x5b37f943, BRF_GRA },	      //  3
	
	{ "sp0.bin",       0x100000, 0x7c5d0633, BRF_GRA },	      //  4	Sprites
	{ "sp1.bin",       0x100000, 0x7eae782e, BRF_GRA },	      //  5
	
	{ "samples.bin",   0x040000, 0x066f30a7, BRF_SND },	      //  6	Samples
};

STD_ROM_PICK(Jumppop)
STD_ROM_FN(Jumppop)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Drv68KRom                   = Next; Next += 0x100000;
	if (DrvHasZ80)DrvZ80Rom     = Next; Next += 0x010000;
	if (DrvHasProt) DrvProtData = Next; Next += 0x000200;
	MSM6295ROM                  = Next; Next += 0x040000;
	DrvMSM6295ROMSrc            = Next; Next += 0x100000;

	RamStart                    = Next;

	Drv68KRam                   = Next; Next += 0x010800;
	if (DrvHasZ80)DrvZ80Ram     = Next; Next += 0x000800;
	DrvSpriteRam                = Next; Next += DrvSpriteRamSize;
	DrvPf1Ram                   = Next; Next += 0x002000;
	DrvPf2Ram                   = Next; Next += 0x002000;
	DrvPaletteRam               = Next; Next += 0x001000;
	DrvControl                  = (UINT16*)Next; Next += 8 * sizeof(UINT16);

	RamEnd                      = Next;

	DrvChars                    = Next; Next += DrvNumChars * 8 * 8;
	DrvTiles                    = Next; Next += DrvNumTiles * 16 * 16;
	DrvSprites                  = Next; Next += DrvNumSprites * 16 * 16;
	DrvPalette                  = (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);

	MemEnd                      = Next;

	return 0;
}

static INT32 JumppopMemIndex()
{
	UINT8 *Next; Next = Mem;

	Drv68KRom                   = Next; Next += 0x080000;
	DrvZ80Rom   		    = Next; Next += 0x040000;
	MSM6295ROM                  = Next; Next += 0x040000;

	RamStart                    = Next;

	Drv68KRam                   = Next; Next += 0x0c0000;
	DrvZ80Ram     		    = Next; Next += 0x000800;
	DrvSpriteRam                = Next; Next += DrvSpriteRamSize;
	DrvPf1Ram                   = Next; Next += 0x004000;
	DrvPf2Ram                   = Next; Next += 0x004000;
	DrvPaletteRam               = Next; Next += 0x000800;
	DrvControl                  = (UINT16*)Next; Next += 8 * sizeof(UINT16);

	RamEnd                      = Next;

	DrvChars                    = Next; Next += DrvNumChars * 8 * 8;
	DrvTiles                    = Next; Next += DrvNumTiles * 16 * 16;
	DrvSprites                  = Next; Next += DrvNumSprites * 16 * 16;
	DrvPalette                  = (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);

	MemEnd                      = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	if (DrvHasProt == 1) memcpy(Drv68KRam + 0x000, DrvProtData, 0x200);
	if (DrvHasProt == 2) memcpy(Drv68KRam + 0x200, DrvProtData, 0x200);
	
	SekOpen(0);
	SekReset();
	SekClose();
	
	if (DrvHasZ80) {
		ZetOpen(0);
		ZetReset();
		ZetClose();
	}
	
	if (DrvHasYM2151) BurnYM2151Reset();
	if (DrvHasYM3812) BurnYM3812Reset();
	
	MSM6295Reset(0);
	
	DrvVBlank = 0;
	DrvOkiBank = 0;
	DrvTileBank = 0;
	DrvSoundLatch = 0;
	Tumbleb2MusicCommand = 0;
	Tumbleb2MusicBank = 0;
	Tumbleb2MusicIsPlaying = 0;
	memset(DrvControl, 0, 8);
	
	return 0;
}

static const INT32 Tumbleb2SoundLookup[256] = {
	/*0     1     2     3     4     5     6     7     8     9     a     b     c     d     e    f*/
	0x00,  -2,  0x00, 0x00,   -2,   -2,   -2,   -2,   -2,   -2,   -2,   -2,   -2,   -2, 0x00,   -2, /* 0 */
	  -2, 0x00,   -2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 1 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, /* 2 */
	0x19, 0x00, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, /* 3 */
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, /* 4 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 5 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 6 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 7 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 8 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 9 */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* a */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* b */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* c */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* d */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* e */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* f */
};

static void Tumbleb2PlayMusic()
{
	INT32 Status = MSM6295ReadStatus(0);

	if (Tumbleb2MusicIsPlaying)
	{
		if ((Status & 0x08) == 0) {
			MSM6295Command(0, 0x80 | Tumbleb2MusicCommand);
			MSM6295Command(0, 0x00 | 0x82);
		}
	}
}

static void Tumbleb2SetMusicBank(INT32 Bank)
{
	memcpy(MSM6295ROM + 0x38000, DrvMSM6295ROMSrc + 0x38000 + (Bank * 0x8000), 0x8000);
}

static void Tumbleb2PlaySound(UINT16 data)
{
	INT32 Status = MSM6295ReadStatus(0);
	
	if ((Status & 0x01) == 0) {
		MSM6295Command(0, 0x80 | data);
		MSM6295Command(0, 0x00 | 0x12);
	} else {
		if ((Status & 0x02) == 0) {
			MSM6295Command(0, 0x80 | data);
			MSM6295Command(0, 0x00 | 0x22);
		} else {
			if ((Status & 0x04) == 0) {
				MSM6295Command(0, 0x80 | data);
				MSM6295Command(0, 0x00 | 0x42);
			}	
		}
	}
}

static void Tumbleb2ProcessMusicCommand(UINT16 data)
{
	INT32 Status = MSM6295ReadStatus(0);
	
	if (data == 1) {
		if ((Status & 0x08) == 0x08) {
			MSM6295Command(0, 0x40);
			Tumbleb2MusicIsPlaying = 0;
		}
	} else {
		if (Tumbleb2MusicIsPlaying != data) {
			Tumbleb2MusicIsPlaying = data;
			
			MSM6295Command(0, 0x40);
			
			switch (data) {
				case 0x04: // map screen
					Tumbleb2MusicBank = 1;
					Tumbleb2MusicCommand = 0x38;
					break;

				case 0x05: // america
					Tumbleb2MusicBank = 6;
					Tumbleb2MusicCommand = 0x38;
					break;

				case 0x06: // asia
					Tumbleb2MusicBank = 2;
					Tumbleb2MusicCommand = 0x38;
					break;

				case 0x07: // africa/egypt -- don't seem to have a tune for this one
					Tumbleb2MusicBank = 4;
					Tumbleb2MusicCommand = 0x38;
					break;

				case 0x08: // antartica
					Tumbleb2MusicBank = 3;
					Tumbleb2MusicCommand = 0x38;
					break;

				case 0x09: // brazil / south america
					Tumbleb2MusicBank = 4;
					Tumbleb2MusicCommand = 0x38;
					break;

				case 0x0a: // japan -- don't seem to have a tune
					Tumbleb2MusicBank = 2;
					Tumbleb2MusicCommand = 0x38;
					break;

				case 0x0b: // australia
					Tumbleb2MusicBank = 5;
					Tumbleb2MusicCommand = 0x38;
					break;

				case 0x0c: // france/europe
					Tumbleb2MusicBank = 6;
					Tumbleb2MusicCommand = 0x38;
					break;

				case 0x0d: // how to play
					Tumbleb2MusicBank = 7;
					Tumbleb2MusicCommand = 0x38;
					break;

				case 0x0f: // stage clear
					Tumbleb2MusicBank = 0;
					Tumbleb2MusicCommand = 0x33;
					break;

				case 0x10: // boss stage
					Tumbleb2MusicBank = 8;
					Tumbleb2MusicCommand = 0x38;
					break;

				case 0x12: // world clear
					Tumbleb2MusicBank = 0;
					Tumbleb2MusicCommand = 0x34;
					break;

				default: // anything else..
					Tumbleb2MusicBank = 8;
					Tumbleb2MusicCommand = 0x38;
					break;
			}
			
			Tumbleb2SetMusicBank(Tumbleb2MusicBank);
			Tumbleb2PlayMusic();
		}
	}
}

static void Tumbleb2SoundMCUCommand(UINT16 data)
{
	INT32 Sound = Tumbleb2SoundLookup[data & 0xff];
	
	if (Sound == 0) {
	
	} else {
		if (Sound == -2) {
			Tumbleb2ProcessMusicCommand(data);
		} else {
			Tumbleb2PlaySound(Sound);
		}
	}
}

UINT8 __fastcall Tumbleb68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x100001: {
			return ~0;
		}
		
		case 0x180002: {
			return DrvDip[1];
		}
		
		case 0x180003: {
			return DrvDip[0];
		}
		
		case 0x180009: {
			if (Semibase) return 0xff - DrvInput[2];
			if (DrvVBlank) {
				if (Wondl96) {
					return 0xf3 - DrvInput[2];
				} else {
					return 0xf7 - DrvInput[2];
				}
			}
			if (Wondl96) return 0xfb - DrvInput[2];
			return 0xff - DrvInput[2];
		}
		
		case 0x18000a: {
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Tumbleb68KWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x100000: {
			if (Tumbleb2) {
				Tumbleb2SoundMCUCommand(d);
				return;
			} else {
				MSM6295Command(0, d);
				return;
			}
		}
		
		case 0x100001: {
			if (SemicomSoundCommand) DrvSoundLatch = d;
			return;
		}
		
		case 0x100002: {
			if (Chokchok) DrvTileBank = (d << 8) << 1;
			if (Bcstry) DrvTileBank = d << 8;
			return;
		}
		
		case 0x100003: {
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Tumbleb68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x100004: {
			return rand() % 0x10000;
		}
		
		case 0x180000: {
			return ((0xff - DrvInput[1]) << 8) | (0xff - DrvInput[0]);
		}
		
		case 0x180002: {
			return (DrvDip[1] << 8) | DrvDip[0];
		}
		
		case 0x180004: {
			return -0;
		}
		
		case 0x180006: {
			return -0;
		}
		
		case 0x180008: {
			if (Bcstry && (SekGetPC(0) == 0x560)) {
				return 0x1a0;
			} else {
				if (Semibase) return 0xffff - DrvInput[2];
				if (DrvVBlank) {
					if (Wondl96) {
						return 0xfff3 - DrvInput[2];
					} else {
						return 0xfff7 - DrvInput[2];
					}
				}
			}
			if (Wondl96) return 0xfff3 - DrvInput[2];
			return 0xffff - DrvInput[2];
		}
		
		case 0x18000a: {
			return 0;
		}
		
		case 0x18000c: {
			return 0;
		}
		
		case 0x18000e: {
			return -0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Tumbleb68KWriteWord(UINT32 a, UINT16 d)
{
#if 1 && defined FBA_DEBUG
	if (a >= 0x160800 && a <= 0x160807) return;
	if (a >= 0x198000 && a <= 0x1a8015) return;
	if (a >= 0x321000 && a <= 0x321fff) return;
	if (a >= 0x323000 && a <= 0x331fff) return;
	if (a >= 0x340000 && a <= 0x3401ff) return;
	if (a >= 0x340400 && a <= 0x34047f) return;
	if (a >= 0x342000 && a <= 0x3421ff) return;
	if (a >= 0x342400 && a <= 0x34247f) return;
#endif
	
	if (a >= 0x300000 && a <= 0x30000f) {
		DrvControl[(a - 0x300000) >> 1] = d;
		return;
	}
	
	switch (a) {
		case 0x100000: {
			if (Tumbleb2) {
				Tumbleb2SoundMCUCommand(d);
				return;
			} else {
				if (Jumpkids) {
					DrvSoundLatch = d & 0xff;
					ZetOpen(0);
					ZetRaiseIrq(0);
					ZetClose();
					return;
				} else {
					if (SemicomSoundCommand) {
						if (d & 0xff) DrvSoundLatch = d & 0xff;
						return;
					} else {
						MSM6295Command(0, d & 0xff);
						return;
					}
				}
			}
		}
		
		case 0x100002: {
			if (Wlstar) DrvTileBank = d & 0x4000;
			return;
		}
		
		case 0x18000c: {
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT16 __fastcall Suprtrio68KReadWord(UINT32 a)
{
	switch (a) {
		case 0xe00000: {
			return ((0xff - DrvInput[1]) << 8) | (0xff - DrvInput[0]);
		}
		
		case 0xe40000: {
			return 0xffff - DrvInput[2];
		}
		
		case 0xe80002: {
			return 0xff00 | DrvDip[0];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Suprtrio68KWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0xa00000 && a <= 0xa0000f) {
		DrvControl[(a - 0xa00000) >> 1] = d;
		return;
	}
	
	switch (a) {
		case 0xe00000: {
			DrvTileBank = d << 14;
			return;
		}
		
		case 0xec0000: {
			if (SemicomSoundCommand) {
				if (d & 0xff) DrvSoundLatch = d & 0xff;
			}
			return;
		}
	
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall Fncywld68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x100003: {
			return 0;
		}
		
		case 0x100005: {
			return MSM6295ReadStatus(0);
		}
		
		case 0x180002: {
			return DrvDip[1];
		}
		
		case 0x180005: {
			return -0;
		}
		
		case 0x180009: {
			if (DrvVBlank) return 0xf7 - DrvInput[2];
			return 0xff - DrvInput[2];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Fncywld68KWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x100001: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0x100003: {
			BurnYM2151WriteRegister(d);
			return;
		}
		
		case 0x100005: {
			MSM6295Command(0, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Fncywld68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x180000: {
			return ((0xff - DrvInput[1]) << 8) | (0xff - DrvInput[0]);
		}
		
		case 0x180002: {
			return (DrvDip[1] << 8) | DrvDip[0];
		}
		
		case 0x180004: {
			return -0;
		}
		
		case 0x180006: {
			return -0;
		}
		
		case 0x180008: {
			if (DrvVBlank) return 0xfff7 - DrvInput[2];
			return 0xffff - DrvInput[2];
		}
		
		case 0x18000a: {
			return 0;
		}
		
		case 0x18000c: {
			return 0;
		}
		
		case 0x18000e: {
			return -0;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Fncywld68KWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x160800 && a <= 0x160807) return;
	
	if (a >= 0x300000 && a <= 0x30000f) {
		DrvControl[(a - 0x300000) >> 1] = d;
		return;
	}
	
	switch (a) {
		case 0x100000: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT16 __fastcall Jumppop68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x180002: {
			return ((0xff - DrvInput[1]) << 8) | (0xff - DrvInput[0]);
		}
		
		case 0x180004: {
			return 0xffff - DrvInput[2];
		}
		
		case 0x180006: {
			return (DrvDip[1] << 8) | DrvDip[0];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Jumppop68KWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x380000 && a <= 0x38000f) {
		DrvControl[(a - 0x380000) >> 1] = d;
		return;
	}
	
	switch (a) {
		case 0x180000: {
			// NOP
			return;
		}
		
		case 0x18000c: {
			DrvSoundLatch = d & 0xff;
			ZetOpen(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			return;
		}
		
		case 0x180008:
		case 0x18000a: {
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall JumpkidsZ80Read(UINT16 a)
{
	switch (a) {
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

void __fastcall JumpkidsZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0x9000: {
			DrvOkiBank = d & 3;
			memcpy(MSM6295ROM + 0x20000, DrvMSM6295ROMSrc + (DrvOkiBank * 0x20000), 0x20000);
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

UINT8 __fastcall SemicomZ80Read(UINT16 a)
{
	switch (a) {
		case 0xf001: {
			return BurnYM2151ReadStatus();
		}
		
		case 0xf002: {
			return MSM6295ReadStatus(0);
		}
		
		case 0xf008: {
			return DrvSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall SemicomZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xf000: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0xf001: {
			BurnYM2151WriteRegister(d);
			return;
		}
		
		case 0xf002: {
			MSM6295Command(0, d);
			return;
		}
		
		case 0xf006: {
			return;
		}
		
		case 0xf00e: {
			DrvOkiBank = d;
			memcpy(MSM6295ROM + 0x30000, DrvMSM6295ROMSrc + 0x30000 + (DrvOkiBank * 0x10000), 0x10000);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall JumppopZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x02: {
			return MSM6295ReadStatus(0);
		}
		
		case 0x03: {
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return DrvSoundLatch;
		}
		
		case 0x06: {
			// NOP
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Read -> %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall JumppopZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			BurnYM3812Write(0, d);
			return;
		}
		
		case 0x01: {
			BurnYM3812Write(1, d);
			return;
		}
		
		case 0x02: {
			MSM6295Command(0, d);
			return;
		}
		
		case 0x04: {
			// NOP
			return;
		}
		
		case 0x05: {
			DrvZ80Bank = d;
			ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom + (DrvZ80Bank * 0x4000));
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom + (DrvZ80Bank * 0x4000));
			return;
		}
		
		case 0x06: {
			// NOP
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write -> %02X, %02x\n"), a, d);
		}
	}
}

static INT32 CharPlaneOffsets[4]     = { 0x200008, 0x200000, 8, 0 };
static INT32 CharXOffsets[8]         = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 CharYOffsets[8]         = { 0, 16, 32, 48, 64, 80, 96, 112 };
static INT32 SpritePlaneOffsets[4]   = { 0x400008, 0x400000, 8, 0 };
static INT32 Sprite2PlaneOffsets[4]  = { 0x800008, 0x800000, 8, 0 };
static INT32 Sprite3PlaneOffsets[4]  = { 0x1000008, 0x1000000, 8, 0 };
static INT32 SpriteXOffsets[16]      = { 256, 257, 258, 259, 260, 261, 262, 263, 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 SpriteYOffsets[16]      = { 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240 };
static INT32 SuprtrioPlaneOffsets[4] = { 0x400000, 0, 0x600000, 0x200000 };
static INT32 SuprtrioXOffsets[16]    = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
static INT32 SuprtrioYOffsets[16]    = { 8, 0, 16, 24, 40, 32, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };
static INT32 JpCharPlaneOffsets[8]   = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 JpCharXOffsets[8]       = { 0x800000, 0x800008, 0, 8, 0x800010, 0x800018, 16, 24 };
static INT32 JpCharYOffsets[8]       = { 0, 32, 64, 96, 128, 160, 192, 224 };
static INT32 JpTilePlaneOffsets[8]   = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 JpTileXOffsets[16]      = { 0x800000, 0x800008, 0, 8, 0x800010, 0x800018, 16, 24, 0x800100, 0x800108, 256, 264, 0x800110, 0x800118, 272, 280 };
static INT32 JpTileYOffsets[16]      = { 0, 32, 64, 96, 128, 160, 192, 224, 512, 544, 576, 608, 640, 672, 704, 736 };

static void TumblebTilesRearrange()
{
	UINT8 *rom = DrvTempRom;
	INT32 len = DrvNumTiles * 128;
	INT32 i;

	/* gfx data is in the wrong order */
	for (i = 0;i < len;i++)
	{
		if ((i & 0x20) == 0)
		{
			INT32 t = rom[i]; rom[i] = rom[i + 0x20]; rom[i + 0x20] = t;
		}
	}
	/* low/high half are also swapped */
	for (i = 0;i < len/2;i++)
	{
		INT32 t = rom[i]; rom[i] = rom[i + len/2]; rom[i + len/2] = t;
	}
}

static INT32 TumblebLoadRoms()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001, 3, 2); if (nRet != 0) return 1;
	TumblebTilesRearrange();
	GfxDecode(DrvNumChars, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(DrvNumTiles, 4, 16, 16, CharPlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvTiles);
		
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000,  5, 1); if (nRet != 0) return 1;
	GfxDecode(DrvNumSprites, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x00000, 6, 1); if (nRet != 0) return 1;
	if (Tumbleb2) nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x80000, 6, 1); if (nRet != 0) return 1;
	memcpy(MSM6295ROM, DrvMSM6295ROMSrc, 0x40000);
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 JumpkidsLoadRoms()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 2, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001, 4, 2); if (nRet != 0) return 1;
	TumblebTilesRearrange();
	GfxDecode(DrvNumChars, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(DrvNumTiles, 4, 16, 16, CharPlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvTiles);
		
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001,  6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000,  7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080001,  8, 2); if (nRet != 0) return 1;
	GfxDecode(DrvNumSprites, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x00000, 9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROM + 0x00000, 10, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);

	return 0;
}

static INT32 MetlsavrLoadRoms()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x200000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 2, 1); if (nRet != 0) return 1;
	
	// Load Shared RAM data
	nRet = BurnLoadRom(DrvProtData, 3, 1); if (nRet) return 1;
	BurnByteswap(DrvProtData, 0x200);
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 5, 2); if (nRet != 0) return 1;
	TumblebTilesRearrange();
	GfxDecode(DrvNumChars, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(DrvNumTiles, 4, 16, 16, CharPlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvTiles);
		
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001,  7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000,  8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100001,  9, 2); if (nRet != 0) return 1;
	GfxDecode(DrvNumSprites, 4, 16, 16, Sprite2PlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(MSM6295ROM, 10, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 PangpangLoadRoms()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080001, 5, 2); if (nRet != 0) return 1;
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x100000);
	memcpy(pTemp, DrvTempRom, 0x100000);
	memset(DrvTempRom, 0, 0x100000);
	memcpy(DrvTempRom + 0x000000, pTemp + 0x000000, 0x40000);
	memcpy(DrvTempRom + 0x080000, pTemp + 0x040000, 0x40000);
	memcpy(DrvTempRom + 0x040000, pTemp + 0x080000, 0x40000);
	memcpy(DrvTempRom + 0x0c0000, pTemp + 0x0c0000, 0x40000);
	BurnFree(pTemp);
	TumblebTilesRearrange();
	GfxDecode(DrvNumChars, 4, 8, 8, SpritePlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(DrvNumTiles, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001,  7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000,  8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080001,  9, 2); if (nRet != 0) return 1;
	GfxDecode(DrvNumSprites, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x00000, 10, 1); if (nRet != 0) return 1;
	memcpy(MSM6295ROM, DrvMSM6295ROMSrc, 0x40000);
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static void SuprtrioDecrypt68KRom()
{
	UINT16 *Rom = (UINT16*)Drv68KRom;
	UINT16 *pTemp = (UINT16*)BurnMalloc(0x80000);
	INT32 i;
	
	memcpy(pTemp, Rom, 0x80000);
	for (i = 0; i < 0x40000; i++) {
		INT32 j = i ^ 0x06;
		if ((i & 1) == 0) j ^= 0x02;
		if ((i & 3) == 0) j ^= 0x08;
		Rom[i] = pTemp[j];
	}
	BurnFree(pTemp);
}

static void SuprtrioDecryptTiles()
{
	UINT16 *Rom = (UINT16*)DrvTempRom;
	UINT16 *pTemp = (UINT16*)BurnMalloc(0x100000);
	INT32 i;
	
	memcpy(pTemp, Rom, 0x100000);
	for (i = 0; i < 0x80000; i++) {
		INT32 j = i ^ 0x02;
		if (i & 1) j ^= 0x04;
		Rom[i] = pTemp[j];
	}
	BurnFree(pTemp);
}

static INT32 SuprtrioLoadRoms()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);
	
	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	SuprtrioDecrypt68KRom();
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 2, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000, 4, 1); if (nRet != 0) return 1;
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x100000);
	memcpy(pTemp, DrvTempRom, 0x100000);
	memset(DrvTempRom, 0, 0x100000);
	memcpy(DrvTempRom + 0x000000, pTemp + 0x000000, 0x20000);
	memcpy(DrvTempRom + 0x040000, pTemp + 0x020000, 0x20000);
	memcpy(DrvTempRom + 0x020000, pTemp + 0x040000, 0x20000);
	memcpy(DrvTempRom + 0x060000, pTemp + 0x060000, 0x20000);
	memcpy(DrvTempRom + 0x080000, pTemp + 0x080000, 0x20000);
	memcpy(DrvTempRom + 0x0c0000, pTemp + 0x0a0000, 0x20000);
	memcpy(DrvTempRom + 0x0a0000, pTemp + 0x0c0000, 0x20000);
	memcpy(DrvTempRom + 0x0e0000, pTemp + 0x0e0000, 0x20000);
	BurnFree(pTemp);
	SuprtrioDecryptTiles();
	GfxDecode(DrvNumTiles, 4, 16, 16, SuprtrioPlaneOffsets, SuprtrioXOffsets, SuprtrioYOffsets, 0x100, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001,  6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000,  7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080001,  8, 2); if (nRet != 0) return 1;
	GfxDecode(DrvNumSprites, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x00000, 9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x80000, 10, 1); if (nRet != 0) return 1;
	memcpy(MSM6295ROM, DrvMSM6295ROMSrc, 0x40000);
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 HtchctchLoadRoms()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 2, 1); if (nRet != 0) return 1;
	
	// Load Shared RAM data
	nRet = BurnLoadRom(DrvProtData, 3, 1); if (nRet) return 1;
	BurnByteswap(DrvProtData, 0x200);
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 5, 2); if (nRet != 0) return 1;
	TumblebTilesRearrange();
	GfxDecode(DrvNumChars, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(DrvNumTiles, 4, 16, 16, CharPlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvTiles);
		
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001,  7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x040000,  8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x040001,  9, 2); if (nRet != 0) return 1;
	GfxDecode(DrvNumSprites, 4, 16, 16, CharPlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(MSM6295ROM, 10, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 ChokchokLoadRoms()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x200000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 2, 1); if (nRet != 0) return 1;
	
	// Load Shared RAM data
	nRet = BurnLoadRom(DrvProtData, 3, 1); if (nRet) return 1;
	BurnByteswap(DrvProtData, 0x200);
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 5, 2); if (nRet != 0) return 1;
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x100000);
	memcpy(pTemp, DrvTempRom, 0x100000);
	memset(DrvTempRom, 0, 0x200000);
	memcpy(DrvTempRom + 0x000000, pTemp + 0x000000, 0x40000);
	memcpy(DrvTempRom + 0x100000, pTemp + 0x040000, 0x40000);
	memcpy(DrvTempRom + 0x040000, pTemp + 0x080000, 0x40000);
	memcpy(DrvTempRom + 0x140000, pTemp + 0x0c0000, 0x40000);
	BurnFree(pTemp);
	TumblebTilesRearrange();
	GfxDecode(DrvNumChars, 4, 8, 8, Sprite2PlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(DrvNumTiles, 4, 16, 16, Sprite2PlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvTiles);
		
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001,  7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000,  8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100001,  9, 2); if (nRet != 0) return 1;
	GfxDecode(DrvNumSprites, 4, 16, 16, Sprite2PlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(MSM6295ROM, 10, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 FncywldLoadRoms()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080001, 5, 2); if (nRet != 0) return 1;
	TumblebTilesRearrange();
	GfxDecode(DrvNumChars, 4, 8, 8, SpritePlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(DrvNumTiles, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvTiles);
		
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001,  7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000,  8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080001,  9, 2); if (nRet != 0) return 1;
	GfxDecode(DrvNumSprites, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(MSM6295ROM + 0x00000, 10, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 SdfightLoadRoms()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x400000);
	
	// Load 68000 Program Roms
	nRet = BurnLoadRom(DrvTempRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	memcpy(Drv68KRom + 0xc0000, DrvTempRom + 0x00000, 0x40000);
	memcpy(Drv68KRom + 0x80000, DrvTempRom + 0x40000, 0x40000);
	memcpy(Drv68KRom + 0x40000, DrvTempRom + 0x80000, 0x40000);
	memcpy(Drv68KRom + 0x00000, DrvTempRom + 0xc0000, 0x40000);
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 2, 1); if (nRet != 0) return 1;
	
	// Load Shared RAM data
	nRet = BurnLoadRom(DrvProtData, 3, 1); if (nRet) return 1;
	BurnByteswap(DrvProtData, 0x200);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x400000);
	nRet = BurnLoadRom(DrvTempRom + 0x200001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x200000, 5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300001, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300000, 7, 2); if (nRet != 0) return 1;
	memcpy(DrvTempRom + 0x000000, DrvTempRom + 0x200000, 0x40000);
	memcpy(DrvTempRom + 0x100000, DrvTempRom + 0x240000, 0x40000);
	memcpy(DrvTempRom + 0x040000, DrvTempRom + 0x280000, 0x40000);
	memcpy(DrvTempRom + 0x140000, DrvTempRom + 0x2c0000, 0x40000);
	memcpy(DrvTempRom + 0x080000, DrvTempRom + 0x300000, 0x40000);
	memcpy(DrvTempRom + 0x180000, DrvTempRom + 0x340000, 0x40000);
	memcpy(DrvTempRom + 0x0c0000, DrvTempRom + 0x380000, 0x40000);
	memcpy(DrvTempRom + 0x1c0000, DrvTempRom + 0x3c0000, 0x40000);
	TumblebTilesRearrange();
	GfxDecode(DrvNumChars, 4, 8, 8, Sprite2PlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(DrvNumTiles, 4, 16, 16, Sprite2PlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvTiles);
		
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001,  9, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000, 10, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100001, 11, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x200000, 12, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x200001, 13, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300000, 14, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300001, 15, 2); if (nRet != 0) return 1;
	GfxDecode(DrvNumSprites, 4, 16, 16, Sprite3PlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(MSM6295ROM, 16, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 BcstryLoadRoms()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x400000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(DrvTempRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	memcpy(Drv68KRom + 0x40000, DrvTempRom + 0x00000, 0x40000);
	memcpy(Drv68KRom + 0x00000, DrvTempRom + 0x40000, 0x40000);
	
	// Load Z80 Program Roms
	memset(DrvTempRom, 0, 0x400000);
	nRet = BurnLoadRom(DrvTempRom, 2, 1); if (nRet != 0) return 1;
	memcpy(DrvZ80Rom + 0x04000, DrvTempRom + 0x00000, 0x04000);
	memcpy(DrvZ80Rom + 0x00000, DrvTempRom + 0x04000, 0x04000);
	memcpy(DrvZ80Rom + 0x0c000, DrvTempRom + 0x08000, 0x04000);
	memcpy(DrvZ80Rom + 0x08000, DrvTempRom + 0x0c000, 0x04000);
	
	// Load Shared RAM data
	memset(DrvTempRom, 0, 0x400000);
	nRet = BurnLoadRom(DrvProtData, 3, 1); if (nRet) return 1;
	BurnByteswap(DrvProtData, 0x200);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x400000);
	nRet = BurnLoadRom(DrvTempRom + 0x200000, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x200001, 5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300000, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300001, 7, 2); if (nRet != 0) return 1;
	memcpy(DrvTempRom + 0x000000, DrvTempRom + 0x200000, 0x40000);
	memcpy(DrvTempRom + 0x100000, DrvTempRom + 0x240000, 0x40000);
	memcpy(DrvTempRom + 0x040000, DrvTempRom + 0x280000, 0x40000);
	memcpy(DrvTempRom + 0x140000, DrvTempRom + 0x2c0000, 0x40000);
	memcpy(DrvTempRom + 0x080000, DrvTempRom + 0x300000, 0x40000);
	memcpy(DrvTempRom + 0x180000, DrvTempRom + 0x340000, 0x40000);
	memcpy(DrvTempRom + 0x0c0000, DrvTempRom + 0x380000, 0x40000);
	memcpy(DrvTempRom + 0x1c0000, DrvTempRom + 0x3c0000, 0x40000);
	TumblebTilesRearrange();
	GfxDecode(DrvNumChars, 4, 8, 8, Sprite2PlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	GfxDecode(DrvNumTiles, 4, 16, 16, Sprite2PlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvTiles);
		
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001,  9, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000, 10, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100001, 11, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x200000, 12, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x200001, 13, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300000, 14, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300001, 15, 2); if (nRet != 0) return 1;
	GfxDecode(DrvNumSprites, 4, 16, 16, Sprite3PlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(MSM6295ROM, 16, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static void TumblebMap68k()
{
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRam           , 0x120000, 0x123fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x140000, 0x1407ff, SM_RAM);
	SekMapMemory(DrvSpriteRam        , 0x160000, 0x1607ff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x4000  , 0x1a0000, 0x1a07ff, SM_RAM);
	SekMapMemory(DrvPf1Ram           , 0x320000, 0x320fff, SM_RAM);
	SekMapMemory(DrvPf2Ram           , 0x322000, 0x322fff, SM_RAM);
	SekSetReadWordHandler(0, Tumbleb68KReadWord);
	SekSetWriteWordHandler(0, Tumbleb68KWriteWord);
	SekSetReadByteHandler(0, Tumbleb68KReadByte);
	SekSetWriteByteHandler(0, Tumbleb68KWriteByte);
	SekClose();
}

static void PangpangMap68k()
{
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRam           , 0x120000, 0x123fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x140000, 0x1407ff, SM_RAM);
	SekMapMemory(DrvSpriteRam        , 0x160000, 0x1607ff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x4000  , 0x1a0000, 0x1a07ff, SM_RAM);
	SekMapMemory(DrvPf1Ram           , 0x320000, 0x321fff, SM_RAM);
	SekMapMemory(DrvPf2Ram           , 0x340000, 0x341fff, SM_RAM);
	SekSetReadWordHandler(0, Tumbleb68KReadWord);
	SekSetWriteWordHandler(0, Tumbleb68KWriteWord);
	SekSetReadByteHandler(0, Tumbleb68KReadByte);
	SekSetWriteByteHandler(0, Tumbleb68KWriteByte);
	SekClose();
}

static void SuprtrioMap68k()
{
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvSpriteRam        , 0x700000, 0x7007ff, SM_RAM);
	SekMapMemory(DrvPf1Ram           , 0xa20000, 0xa20fff, SM_RAM);
	SekMapMemory(DrvPf2Ram           , 0xa22000, 0xa22fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0xcf0000, 0xcf05ff, SM_RAM);
	SekMapMemory(Drv68KRam           , 0xf00000, 0xf07fff, SM_RAM);
	SekSetReadWordHandler(0, Suprtrio68KReadWord);
	SekSetWriteWordHandler(0, Suprtrio68KWriteWord);
	SekClose();
}

static void HtchctchMap68k()
{
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRam           , 0x120000, 0x123fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x140000, 0x1407ff, SM_RAM);
	SekMapMemory(DrvSpriteRam        , 0x160000, 0x160fff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x4000  , 0x1a0000, 0x1a0fff, SM_RAM);
	SekMapMemory(DrvPf1Ram           , 0x320000, 0x321fff, SM_RAM);
	SekMapMemory(DrvPf2Ram           , 0x322000, 0x322fff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x5000  , 0x341000, 0x342fff, SM_RAM);
	SekSetReadWordHandler(0, Tumbleb68KReadWord);
	SekSetWriteWordHandler(0, Tumbleb68KWriteWord);
	SekSetReadByteHandler(0, Tumbleb68KReadByte);
	SekSetWriteByteHandler(0, Tumbleb68KWriteByte);
	SekClose();
}

static void FncywldMap68k()
{
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(DrvPaletteRam       , 0x140000, 0x140fff, SM_RAM);
	SekMapMemory(DrvSpriteRam        , 0x160000, 0x1607ff, SM_RAM);
	SekMapMemory(DrvPf1Ram           , 0x320000, 0x321fff, SM_RAM);
	SekMapMemory(DrvPf2Ram           , 0x322000, 0x323fff, SM_RAM);
	SekMapMemory(Drv68KRam           , 0xff0000, 0xffffff, SM_RAM);
	SekSetReadWordHandler(0, Fncywld68KReadWord);
	SekSetWriteWordHandler(0, Fncywld68KWriteWord);
	SekSetReadByteHandler(0, Fncywld68KReadByte);
	SekSetWriteByteHandler(0, Fncywld68KWriteByte);
	SekClose();
}

static void JumpkidsMapZ80()
{
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(JumpkidsZ80Read);
	ZetSetWriteHandler(JumpkidsZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom                );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom                );
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80Ram                );
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80Ram                );
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80Ram                );
	ZetMemEnd();
	ZetClose();
}

static void SemicomMapZ80()
{
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(SemicomZ80Read);
	ZetSetWriteHandler(SemicomZ80Write);
	ZetMapArea(0x0000, 0xcfff, 0, DrvZ80Rom                );
	ZetMapArea(0x0000, 0xcfff, 2, DrvZ80Rom                );
	ZetMapArea(0xd000, 0xd7ff, 0, DrvZ80Ram                );
	ZetMapArea(0xd000, 0xd7ff, 1, DrvZ80Ram                );
	ZetMapArea(0xd000, 0xd7ff, 2, DrvZ80Ram                );
	ZetMemEnd();
	ZetClose();
}

static void SemicomYM2151IrqHandler(INT32 Irq)
{
	if (Irq) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvInit(bool bReset, INT32 SpriteRamSize, INT32 SpriteMask, INT32 SpriteXOffset, INT32 SpriteYOffset, INT32 NumSprites, INT32 NumChars, INT32 NumTiles, double Refresh, INT32 OkiFreq)
{
	INT32 nRet = 0, nLen;
	
	DrvSpriteRamSize = SpriteRamSize;
	DrvNumSprites = NumSprites,
	DrvNumChars = NumChars,
	DrvNumTiles = NumTiles,

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	nRet = DrvLoadRoms();
	
	DrvMap68k();
	
	if (DrvHasZ80) DrvMapZ80();
	
	if (DrvHasYM2151) {
		if (!DrvYM2151Freq) DrvYM2151Freq = 3427190;
		BurnYM2151Init(DrvYM2151Freq, 25.0);
		if (DrvHasZ80) { 
			BurnYM2151SetIrqHandler(&SemicomYM2151IrqHandler);
		}
	}
	
	// Setup the OKIM6295 emulation
	if (DrvHasYM2151) {
		MSM6295Init(0, OkiFreq / 132, 20.0, 1);
	} else {
		MSM6295Init(0, OkiFreq / 132, 20.0, 0);
	}
	
	BurnSetRefreshRate(Refresh);
	
	nCyclesTotal[0] = 14000000 / 60;
	
	DrvSpriteXOffset = SpriteXOffset;
	DrvSpriteYOffset = SpriteYOffset;
	DrvSpriteMask = SpriteMask;
	DrvSpriteColourMask = 0x0f;
	Pf1XOffset = -5;
	Pf1YOffset = 0;
	Pf2XOffset = -1;
	Pf2YOffset = 0;
	
	GenericTilesInit();

	// Reset the driver
	if (bReset) DrvDoReset();

	return 0;
}

static INT32 TumblebInit()
{
	DrvLoadRoms = TumblebLoadRoms;
	DrvMap68k = TumblebMap68k;
	DrvRender = DrvDraw;

	return DrvInit(1, 0x800, 0x3fff, -1, 0, 0x2000, 0x4000, 0x1000, 58.0, 8000000 / 10);
}

static INT32 Tumbleb2Init()
{
	Tumbleb2 = 1;
	return TumblebInit();	
}

static INT32 JumpkidsInit()
{
	INT32 nRet;
	
	Jumpkids = 1;
	DrvHasZ80 = 1;
	DrvLoadRoms = JumpkidsLoadRoms;
	DrvMap68k = TumblebMap68k;
	DrvMapZ80 = JumpkidsMapZ80;
	DrvRender = DrvDraw;
	
	nRet = DrvInit(1, 0x800, 0x7fff, -1, 0, 0x2000, 0x4000, 0x1000, 60.0, 8000000 / 8);
	
	nCyclesTotal[0] = 12000000 / 60;
	nCyclesTotal[1] = (8000000 / 2) / 60;
	
	return nRet;
}

static INT32 MetlsavrInit()
{
	INT32 nRet;
	
	DrvHasZ80 = 1;
	DrvHasYM2151 = 1;
	DrvHasProt = 1;
	SemicomSoundCommand = 1;
	DrvLoadRoms = MetlsavrLoadRoms;
	DrvMap68k = HtchctchMap68k;
	DrvMapZ80 = SemicomMapZ80;
	DrvRender = DrvDraw;
	
	nRet = DrvInit(1, 0x1000, 0x7fff, -1, 0, 0x4000, 0x4000, 0x1000, 60.0, 1024000);
	
	nCyclesTotal[0] = 15000000 / 60;
	nCyclesTotal[1] = (15000000 / 4) / 60;
	
	return nRet;
}

static INT32 PangpangInit()
{
	Tumbleb2 = 1;
	DrvLoadRoms = PangpangLoadRoms;
	DrvMap68k = PangpangMap68k;
	DrvRender = PangpangDraw;

	return DrvInit(1, 0x800, 0x7fff, -1, 0, 0x2000, 0x8000, 0x2000, 58.0, 8000000 / 10);
}

static INT32 SuprtrioInit()
{
	INT32 nRet;
	
	DrvHasZ80 = 1;
	SemicomSoundCommand = 1;
	DrvLoadRoms = SuprtrioLoadRoms;
	DrvMap68k = SuprtrioMap68k;
	DrvMapZ80 = SemicomMapZ80;
	DrvRender = SuprtrioDraw;

	nRet = DrvInit(1, 0x800, 0x7fff, 0, 0, 0x2000, 0x8000, 0x2000, 60.0, 875000);
	
	Pf1XOffset = -6;
	Pf2XOffset = -2;
	nCyclesTotal[1] = 8000000 / 60;
	
	return nRet;
}

static INT32 HtchctchInit()
{
	INT32 nRet;
	
	DrvHasZ80 = 1;
	DrvHasYM2151 = 1;
	DrvHasProt = 1;
	SemicomSoundCommand = 1;
	DrvLoadRoms = HtchctchLoadRoms;
	DrvMap68k = HtchctchMap68k;
	DrvMapZ80 = SemicomMapZ80;
	DrvRender = HtchctchDraw;
	
	nRet = DrvInit(1, 0x1000, 0x7fff, -1, 0, 0x1000, 0x4000, 0x1000, 60.0, 1024000);
	
	nCyclesTotal[0] = 15000000 / 60;
	nCyclesTotal[1] = (15000000 / 4) / 60;
	
	return nRet;
}

static INT32 CookbibInit()
{
	INT32 nRet = HtchctchInit();
	
	Pf1XOffset = -5;
	Pf1YOffset = 0;
	Pf2XOffset = -1;
	Pf2YOffset = 2;
	
	return nRet;
}

static INT32 ChokchokInit()
{
	INT32 nRet;
	
	Chokchok = 1;
	DrvHasZ80 = 1;
	DrvHasYM2151 = 1;
	DrvHasProt = 1;
	SemicomSoundCommand = 1;
	DrvLoadRoms = ChokchokLoadRoms;
	DrvMap68k = HtchctchMap68k;
	DrvMapZ80 = SemicomMapZ80;
	DrvRender = DrvDraw;
	
	nRet = DrvInit(1, 0x1000, 0x7fff, -1, 0, 0x4000, 0x10000, 0x4000, 60.0, 1024000);
	
	nCyclesTotal[0] = 15000000 / 60;
	nCyclesTotal[1] = (15000000 / 4) / 60;
	
	Pf1XOffset = -5;
	Pf1YOffset = 0;
	Pf2XOffset = -1;
	Pf2YOffset = 2;
	
	return nRet;
}

static INT32 WlstarInit()
{
	INT32 nRet;
	
	Wlstar = 1;
	DrvHasZ80 = 1;
	DrvHasYM2151 = 1;
	DrvHasProt = 1;
	SemicomSoundCommand = 1;
	DrvLoadRoms = ChokchokLoadRoms;
	DrvMap68k = HtchctchMap68k;
	DrvMapZ80 = SemicomMapZ80;
	DrvRender = HtchctchDraw;
	
	nRet = DrvInit(1, 0x1000, 0x7fff, -1, 0, 0x4000, 0x10000, 0x4000, 60.0, 1024000);
	
	nCyclesTotal[0] = 15000000 / 60;
	nCyclesTotal[1] = (15000000 / 4) / 60;
	
	Pf1XOffset = -5;
	Pf1YOffset = 0;
	Pf2XOffset = -1;
	Pf2YOffset = 2;
	
	return nRet;
}

static INT32 Wondl96Init()
{
	INT32 nRet;
	
	Wlstar = 1;
	Wondl96 = 1;
	DrvHasZ80 = 1;
	DrvHasYM2151 = 1;
	DrvHasProt = 2;
	SemicomSoundCommand = 1;
	DrvLoadRoms = ChokchokLoadRoms;
	DrvMap68k = HtchctchMap68k;
	DrvMapZ80 = SemicomMapZ80;
	DrvRender = HtchctchDraw;
	
	nRet = DrvInit(1, 0x1000, 0x7fff, -1, 0, 0x4000, 0x10000, 0x4000, 60.0, 1024000);
	
	nCyclesTotal[0] = 15000000 / 60;
	nCyclesTotal[1] = (15000000 / 4) / 60;
	
	Pf1XOffset = -5;
	Pf1YOffset = 0;
	Pf2XOffset = -1;
	Pf2YOffset = 2;
	
	return nRet;
}

static INT32 FncywldInit()
{
	INT32 nRet;
	
	DrvHasYM2151 = 1;
	DrvHasZ80 = 0;
	DrvYM2151Freq = 32220000 / 9;
	DrvLoadRoms = FncywldLoadRoms;
	DrvMap68k = FncywldMap68k;
	DrvRender = FncywldDraw;
	
	nRet = DrvInit(1, 0x800, 0x3fff, -1, 0, 0x2000, 0x8000, 0x2000, 60.0, 1023924);
	
	nCyclesTotal[0] = 12000000 / 60;
	DrvSpriteColourMask = 0x3f;
	
	return nRet;
}

static INT32 SdfightInit()
{
	INT32 nRet;
	
	Bcstry = 1;
	DrvHasZ80 = 1;
	DrvHasYM2151 = 1;
	DrvYM2151Freq = 3427190;
	DrvHasProt = 1;
	SemicomSoundCommand = 1;
	DrvLoadRoms = SdfightLoadRoms;
	DrvMap68k = HtchctchMap68k;
	DrvMapZ80 = SemicomMapZ80;
	DrvRender = SdfightDraw;
	
	nRet = DrvInit(1, 0x1000, 0x7fff, 0, 1, 0x8000, 0x10000, 0x4000, 60.0, 1024000);
	
	nCyclesTotal[0] = 15000000 / 60;
	nCyclesTotal[1] = (15000000 / 4) / 60;
	
	Pf1XOffset = -5;
	Pf1YOffset = -16;
	Pf2XOffset = -1;
	Pf2YOffset = 0;
	
	return nRet;
}

static INT32 BcstryInit()
{
	INT32 nRet;
	
	Bcstry = 1;
	DrvHasZ80 = 1;
	DrvHasYM2151 = 1;
	DrvYM2151Freq = 3427190;
	DrvHasProt = 1;
	SemicomSoundCommand = 1;
	DrvLoadRoms = BcstryLoadRoms;
	DrvMap68k = HtchctchMap68k;
	DrvMapZ80 = SemicomMapZ80;
	DrvRender = HtchctchDraw;
	
	nRet = DrvInit(1, 0x1000, 0x7fff, -1, 0, 0x8000, 0x10000, 0x4000, 60.0, 1024000);
	
	nCyclesTotal[0] = 15000000 / 60;
	nCyclesTotal[1] = (15000000 / 4) / 60;
	
	//Pf1XOffset = 8;
	Pf1XOffset = -5;
	Pf1YOffset = 0;
	//Pf2XOffset = 8;
	Pf2XOffset = -1;
	Pf2YOffset = 0;
	
	return nRet;
}

static INT32 SemibaseInit()
{
	INT32 nRet;
	
	Semibase = 1;
	Bcstry = 1;
	DrvHasZ80 = 1;
	DrvHasYM2151 = 1;
	DrvYM2151Freq = 3427190;
	DrvHasProt = 1;
	SemicomSoundCommand = 1;
	DrvLoadRoms = BcstryLoadRoms;
	DrvMap68k = HtchctchMap68k;
	DrvMapZ80 = SemicomMapZ80;
	DrvRender = HtchctchDraw;
	
	nRet = DrvInit(1, 0x1000, 0x7fff, -1, 0, 0x8000, 0x10000, 0x4000, 60.0, 1024000);
	
	nCyclesTotal[0] = 15000000 / 60;
	nCyclesTotal[1] = (15000000 / 4) / 60;
	
	Pf1XOffset = -2;
	Pf1YOffset = 0;
	Pf2XOffset = -1;
	Pf2YOffset = 0;
	
	return nRet;
}

static INT32 DquizgoInit()
{
	INT32 nRet;
	
	DrvHasZ80 = 1;
	DrvHasYM2151 = 1;
	DrvHasProt = 2;
	SemicomSoundCommand = 1;
	DrvLoadRoms = MetlsavrLoadRoms;
	DrvMap68k = HtchctchMap68k;
	DrvMapZ80 = SemicomMapZ80;
	DrvRender = HtchctchDraw;
	
	nRet = DrvInit(1, 0x1000, 0x7fff, -1, 0, 0x4000, 0x4000, 0x1000, 60.0, 1024000);
	
	nCyclesTotal[0] = 15000000 / 60;
	nCyclesTotal[1] = (15000000 / 4) / 60;
	
	Pf1XOffset = -5;
	Pf1YOffset = 0;
	Pf2XOffset = -1;
	Pf2YOffset = 2;
	
	return nRet;
}

inline static INT32 JumppopSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)(ZetTotalCycles() * nSoundRate / 3500000);
}

static INT32 JumppopInit()
{
	INT32 nRet = 0, nLen;
	
	DrvSpriteRamSize = 0x1000;
	DrvNumSprites = 0x4000,
	DrvNumChars = 0x8000,
	DrvNumTiles = 0x2000,
	
	DrvHasZ80 = 1;
	DrvHasYM3812 = 1;

	// Allocate and Blank all required memory
	Mem = NULL;
	JumppopMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	JumppopMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x200000);
	
	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 0, 1); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	memset(DrvTempRom, 0, 0x200000);
	nRet = BurnLoadRom(DrvZ80Rom, 1, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000, 3, 1); if (nRet != 0) return 1;
	GfxDecode(DrvNumChars, 8, 8, 8, JpCharPlaneOffsets, JpCharXOffsets, JpCharYOffsets, 0x100, DrvTempRom, DrvChars);
	GfxDecode(DrvNumTiles, 8, 16, 16, JpTilePlaneOffsets, JpTileXOffsets, JpTileYOffsets, 0x400, DrvTempRom, DrvTiles);
		
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000, 5, 1); if (nRet != 0) return 1;
	GfxDecode(DrvNumSprites, 4, 16, 16, Sprite2PlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(MSM6295ROM, 6, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRam           , 0x120000, 0x123fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x140000, 0x1407ff, SM_RAM);
	SekMapMemory(DrvSpriteRam        , 0x160000, 0x160fff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x4000  , 0x1a0000, 0x1a7fff, SM_RAM);
	SekMapMemory(DrvPf1Ram           , 0x320000, 0x323fff, SM_RAM);
	SekMapMemory(DrvPf2Ram           , 0x300000, 0x303fff, SM_RAM);
	SekSetReadWordHandler(0, Jumppop68KReadWord);
	SekSetWriteWordHandler(0, Jumppop68KWriteWord);
	SekClose();	
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetInHandler(JumppopZ80PortRead);
	ZetSetOutHandler(JumppopZ80PortWrite);
	ZetMapArea(0x0000, 0x2fff, 0, DrvZ80Rom                );
	ZetMapArea(0x0000, 0x2fff, 2, DrvZ80Rom                );
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom + 0x8000       );
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom + 0x8000       );
	ZetMapArea(0xf800, 0xffff, 0, DrvZ80Ram                );
	ZetMapArea(0xf800, 0xffff, 1, DrvZ80Ram                );
	ZetMapArea(0xf800, 0xffff, 2, DrvZ80Ram                );
	ZetMemEnd();
	ZetClose();
	
	BurnYM3812Init(3500000, NULL, JumppopSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(3500000);
	
	// Setup the OKIM6295 emulation
	MSM6295Init(0, 875000 / 132, 100.0, 1);
	
	BurnSetRefreshRate(60.0);
	
	nCyclesTotal[0] = 16000000 / 60;
	nCyclesTotal[1] = 3500000 / 60;
	
	DrvSpriteXOffset = 1;
	DrvSpriteYOffset = 0;
	DrvSpriteMask = 0x7fff;
	DrvSpriteColourMask = 0x0f;
	Pf1XOffset = -0x3a0;
	Pf1YOffset = 0;
	Pf2XOffset = -0x3a2;
	Pf2YOffset = 0;
	
	GenericTilesInit();
	DrvRender = JumppopDraw;

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	SekExit();
	if (DrvHasZ80) ZetExit();
	if (DrvHasYM2151) BurnYM2151Exit();
	MSM6295Exit(0);
	
	GenericTilesExit();
	
	DrvVBlank = 0;
	DrvOkiBank = 0;
	DrvZ80Bank = 0;
	DrvTileBank = 0;
	DrvSoundLatch = 0;
	Tumbleb2MusicCommand = 0;
	Tumbleb2MusicBank = 0;
	Tumbleb2MusicIsPlaying = 0;
	
	DrvSpriteXOffset = 0;
	DrvSpriteYOffset = 0;
	DrvSpriteRamSize = 0;
	DrvSpriteMask = 0;
	DrvSpriteColourMask = 0;
	DrvYM2151Freq = 0;
	DrvNumSprites = 0;
	DrvNumChars = 0;
	DrvNumTiles = 0;
	DrvHasZ80 = 0;
	DrvHasYM2151 = 0;
	DrvHasYM3812 = 0;
	DrvHasProt = 0;
	Tumbleb2 = 0;
	Jumpkids = 0;
	Chokchok = 0;
	Wlstar = 0;
	Wondl96 = 0;
	Bcstry = 0;
	Semibase = 0;
	SemicomSoundCommand = 0;
	Pf1XOffset = 0;
	Pf1YOffset = 0;
	Pf2XOffset = 0;
	Pf2YOffset = 0;
	
	DrvLoadRoms = NULL;
	DrvMap68k = NULL;
	DrvMapZ80 = NULL;
	DrvRender = NULL;
	
	BurnFree(Mem);

	return 0;
}

static INT32 JumppopExit()
{
	BurnYM3812Exit();
	return DrvExit();
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

	r = pal4bit(nColour >> 0);
	g = pal4bit(nColour >> 4);
	b = pal4bit(nColour >> 8);

	return BurnHighCol(r, g, b, 0);
}

inline static UINT32 HtchctchCalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal5bit(nColour >>  0);
	g = pal5bit(nColour >>  5);
	b = pal5bit(nColour >> 10);

	return BurnHighCol(r, g, b, 0);
}

inline static UINT32 FncywldCalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal4bit(nColour >> 8);
	g = pal4bit(nColour >> 4);
	b = pal4bit(nColour >> 0);

	return BurnHighCol(r, g, b, 0);
}

inline static UINT32 JumppopCalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal5bit(nColour >> 10);
	g = pal5bit(nColour >>  5);
	b = pal5bit(nColour >>  0);

	return BurnHighCol(r, g, b, 0);
}

static void DrvCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)DrvPaletteRam, pd = DrvPalette; i < 0x400; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}
}

static void HtchctchCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)DrvPaletteRam, pd = DrvPalette; i < 0x400; i++, ps++, pd++) {
		*pd = HtchctchCalcCol(*ps);
	}
}

static void FncywldCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)DrvPaletteRam, pd = DrvPalette; i < 0x800; i++, ps++, pd++) {
		*pd = FncywldCalcCol(*ps);
	}
}

static void JumppopCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)DrvPaletteRam, pd = DrvPalette; i < 0x400; i++, ps++, pd++) {
		*pd = JumppopCalcCol(*ps);
	}
}

static void DrvRenderPf2Layer(INT32 ScrollX, INT32 ScrollY)
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex;
	
	UINT16 *VideoRam = (UINT16*)DrvPf2Ram;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			TileIndex = (mx & 0x1f) + ((my & 0x1f) << 5) + ((mx & 0x60) << 5);
			Attr = VideoRam[TileIndex];
			Code = (Attr & 0xfff) | (DrvTileBank >> 2);
			Colour = Attr >> 12;
			
			Code &= (DrvNumTiles - 1);
			
			x = 16 * mx;
			y = 16 * my;
			
			x -= ((ScrollX + Pf2XOffset) & 0x3ff);
			y -= ((ScrollY + Pf2YOffset) & 0x1ff);
			if (x < -16) x += 1024;
			if (y < -16) y += 512;
			
			y -= 8;

			if (x > 0 && x < 304 && y > 0 && y < 224) {
				Render16x16Tile(pTransDraw, Code, x, y, Colour, 4, 512, DrvTiles);
			} else {
				Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 512, DrvTiles);
			}
		}
	}
}

static void PangpangRenderPf2Layer()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex;
	
	UINT16 *VideoRam = (UINT16*)DrvPf2Ram;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			TileIndex = (mx & 0x1f) + ((my & 0x1f) << 5) + ((mx & 0x60) << 5);
			Attr = VideoRam[TileIndex * 2 + 0];
			Code = VideoRam[TileIndex * 2 + 1] & 0xfff;
			Code |= 0x1000;
			Colour = (Attr >> 12) & 0x0f;
			
			Code &= (DrvNumTiles - 1);
			
			x = 16 * mx;
			y = 16 * my;
			
			x -= ((DrvControl[3] + Pf2XOffset) & 0x3ff);
			y -= ((DrvControl[4] + Pf2YOffset) & 0x1ff);
			if (x < -16) x += 1024;
			if (y < -16) y += 512;
			
			y -= 8;

			if (x > 0 && x < 304 && y > 0 && y < 224) {
				Render16x16Tile(pTransDraw, Code, x, y, Colour, 4, 512, DrvTiles);
			} else {
				Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 512, DrvTiles);
			}
		}
	}
}

static void FncywldRenderPf2Layer()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex;
	
	UINT16 *VideoRam = (UINT16*)DrvPf2Ram;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			TileIndex = (mx & 0x1f) + ((my & 0x1f) << 5) + ((mx & 0x60) << 5);
			Attr = VideoRam[TileIndex * 2 + 1];
			Code = VideoRam[TileIndex * 2 + 0];
			Colour = Attr & 0x1f;
			
			Code &= (DrvNumTiles - 1);
			
			x = 16 * mx;
			y = 16 * my;
			
			x -= ((DrvControl[3] + Pf2XOffset) & 0x3ff);
			y -= ((DrvControl[4] + Pf2YOffset) & 0x1ff);
			if (x < -16) x += 1024;
			if (y < -16) y += 512;
		
			y -= 8;

			if (x > 0 && x < 304 && y > 0 && y < 224) {
				Render16x16Tile(pTransDraw, Code, x, y, Colour, 4, 0x400, DrvTiles);
			} else {
				Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 0x400, DrvTiles);
			}
		}
	}
}

static void JumppopRenderPf2Layer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;
	
	UINT16 *VideoRam = (UINT16*)DrvPf2Ram;
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = VideoRam[TileIndex];
			Code &= (DrvNumTiles - 1);
			Colour = 0;
			
			x = 16 * mx;
			y = 16 * my;
			
			x -= ((DrvControl[0] + Pf2XOffset) & 0x3ff);
			y -= ((DrvControl[1] + Pf2YOffset) & 0x3ff);
			if (x < -16) x += 1024;
			if (y < -16) y += 1024;
			
			y -= 8;

			if (x > 0 && x < 304 && y > 0 && y < 224) {
				Render16x16Tile(pTransDraw, Code, x, y, Colour, 8, 512, DrvTiles);
			} else {
				Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 8, 512, DrvTiles);
			}
			
			TileIndex++;
		}
	}
}

static void JumppopRenderPf2AltLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;
	
	UINT16 *VideoRam = (UINT16*)DrvPf2Ram;
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < 128; mx++) {
			Code = VideoRam[TileIndex];
			Colour = 0;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= ((DrvControl[0] + Pf2XOffset) & 0x3ff);
			y -= ((DrvControl[1] + Pf2YOffset) & 0x1ff);
			if (x < -8) x += 1024;
			if (y < -8) y += 512;
			
			y -= 8;

			if (x > 0 && x < 312 && y > 0 && y < 232) {
				Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 8, 0, 512, DrvChars);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 8, 0, 512, DrvChars);
			}
			
			TileIndex++;
		}
	}
}

static void DrvRenderPf1Layer(INT32 ScrollX, INT32 ScrollY)
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex;
	
	UINT16 *VideoRam = (UINT16*)DrvPf1Ram;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			TileIndex = (mx & 0x1f) + ((my & 0x1f) << 5) + ((mx & 0x60) << 5);
			Attr = VideoRam[TileIndex];
			Code = (Attr & 0xfff) | (DrvTileBank >> 2);
			Colour = Attr >> 12;
			
			Code &= (DrvNumTiles - 1);
			
			x = 16 * mx;
			y = 16 * my;
			
			x -= ((ScrollX + Pf1XOffset) & 0x3ff);
			y -= ((ScrollY + Pf1YOffset) & 0x1ff);
			if (x < -16) x += 1024;
			if (y < -16) y += 512;
			
			y -= 8;

			if (x > 0 && x < 304 && y > 0 && y < 224) {
				Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvTiles);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvTiles);
			}
		}
	}
}

static void PangpangRenderPf1Layer()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex;
	
	UINT16 *VideoRam = (UINT16*)DrvPf1Ram;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			TileIndex = (mx & 0x1f) + ((my & 0x1f) << 5) + ((mx & 0x60) << 5);
			Attr = VideoRam[TileIndex * 2 + 0];
			Code = VideoRam[TileIndex * 2 + 1];
			Colour = (Attr >> 12) & 0x0f;
			
			Code &= (DrvNumTiles - 1);
			
			x = 16 * mx;
			y = 16 * my;
			
			x -= ((DrvControl[1] + Pf1XOffset) & 0x3ff);
			y -= ((DrvControl[2] + Pf1YOffset) & 0x1ff);
			if (x < -16) x += 1024;
			if (y < -16) y += 512;
			
			y -= 8;

			if (x > 0 && x < 304 && y > 0 && y < 224) {
				Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvTiles);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvTiles);
			}
		}
	}
}

static void FncywldRenderPf1Layer()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex;
	
	UINT16 *VideoRam = (UINT16*)DrvPf1Ram;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			TileIndex = (mx & 0x1f) + ((my & 0x1f) << 5) + ((mx & 0x60) << 5);
			Attr = VideoRam[TileIndex * 2 + 1];
			Code = VideoRam[TileIndex * 2 + 0];
			Colour = Attr & 0x1f;
			
			Code &= (DrvNumTiles - 1);
			
			x = 16 * mx;
			y = 16 * my;
			
			x -= ((DrvControl[1] + Pf1XOffset) & 0x3ff);
			y -= ((DrvControl[2] + Pf1YOffset) & 0x1ff);
			if (x < -16) x += 1024;
			if (y < -16) y += 512;
			
			y -= 8;

			if (x > 0 && x < 304 && y > 0 && y < 224) {
				Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0x0f, 0x200, DrvTiles);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0x0f, 0x200, DrvTiles);
			}
		}
	}
}

static void JumppopRenderPf1Layer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;
	
	UINT16 *VideoRam = (UINT16*)DrvPf1Ram;
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = VideoRam[TileIndex] & 0x1fff;
			Code &= (DrvNumTiles - 1);
			Colour = 0;
			
			x = 16 * mx;
			y = 16 * my;
			
			x -= ((DrvControl[2] + Pf1XOffset) & 0x3ff);
			y -= ((DrvControl[3] + Pf1YOffset) & 0x3ff);
			if (x < -16) x += 1024;
			if (y < -16) y += 1024;
			
			y -= 8;

			if (x > 0 && x < 304 && y > 0 && y < 224) {
				Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 8, 0, 256, DrvTiles);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 8, 0, 256, DrvTiles);
			}
			
			TileIndex++;
		}
	}
}

static void DrvRenderCharLayer()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0;
	
	UINT16 *VideoRam = (UINT16*)DrvPf1Ram;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Attr = VideoRam[TileIndex];
			Code = (Attr & 0xfff) | DrvTileBank;
			Colour = Attr >> 12;
			Code &= (DrvNumChars - 1);
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= ((DrvControl[1] + Pf1XOffset) & 0x1ff);
			y -= ((DrvControl[2] + Pf1YOffset) & 0xff);
			if (x < -8) x += 512;
			if (y < -8) y += 256;
			
			y -= 8;

			if (x > 0 && x < 312 && y > 0 && y < 232) {
				Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvChars);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvChars);
			}
			
			TileIndex++;
		}
	}
}

static void PangpangRenderCharLayer()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0;
	
	UINT16 *VideoRam = (UINT16*)DrvPf1Ram;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Attr = VideoRam[TileIndex * 2 + 0];
			Code = VideoRam[TileIndex * 2 + 1] & 0x1fff;
			Colour = (Attr >> 12) & 0x1f;
			Code &= (DrvNumChars - 1);
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= ((DrvControl[1] + Pf1XOffset) & 0x1ff);
			y -= ((DrvControl[2] + Pf1YOffset) & 0xff);
			if (x < -8) x += 512;
			if (y < -8) y += 256;
			
			y -= 8;

			if (x > 0 && x < 312 && y > 0 && y < 232) {
				Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvChars);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvChars);
			}
			
			TileIndex++;
		}
	}
}

static void FncywldRenderCharLayer()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0;
	
	UINT16 *VideoRam = (UINT16*)DrvPf1Ram;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Attr = VideoRam[TileIndex * 2 + 1];
			Code = VideoRam[TileIndex * 2 + 0] & 0x1fff;
			if (Code) {
				Colour = Attr & 0x1f;
				Code &= (DrvNumChars - 1);
			
				x = 8 * mx;
				y = 8 * my;
			
				x -= ((DrvControl[1] + Pf1XOffset) & 0x1ff);
				y -= ((DrvControl[2] + Pf1YOffset) & 0xff);
				if (x < -8) x += 512;
				if (y < -8) y += 256;
			
				y -= 8;

				if (x > 0 && x < 312 && y > 0 && y < 232) {
					Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0x0f, 0x400, DrvChars);
				} else {
					Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0x0f, 0x400, DrvChars);
				}
			}
			
			TileIndex++;
		}
	}
}

static void SdfightRenderCharLayer()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0;
	
	UINT16 *VideoRam = (UINT16*)DrvPf1Ram;
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < 64; mx++) {
			Attr = VideoRam[TileIndex];
			Code = (Attr & 0xfff) | DrvTileBank;
			Colour = Attr >> 12;
			Code &= (DrvNumChars - 1);
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= ((DrvControl[1] + Pf1XOffset) & 0x1ff);
			y -= ((DrvControl[2] + Pf1YOffset) & 0x1ff);
			if (x < -8) x += 512;
			if (y < -8) y += 512;
			
			y -= 8;

			if (x > 0 && x < 312 && y > 0 && y < 232) {
				Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvChars);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 256, DrvChars);
			}
			
			TileIndex++;
		}
	}
}

static void JumppopRenderCharLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;
	
	UINT16 *VideoRam = (UINT16*)DrvPf1Ram;
	
	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < 128; mx++) {
			Code = VideoRam[TileIndex];
			Colour = 0;
			
			x = 8 * mx;
			y = 8 * my;
			
			x -= ((DrvControl[2] + Pf1XOffset) & 0x3ff);
			y -= ((DrvControl[3] + Pf1YOffset) & 0x1ff);
			if (x < -8) x += 1024;
			if (y < -8) y += 512;
			
			y -= 8;

			if (x > 0 && x < 312 && y > 0 && y < 232) {
				Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 8, 0, 256, DrvChars);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 8, 0, 256, DrvChars);
			}
			
			TileIndex++;
		}
	}
}

static void DrvRenderSprites(INT32 MaskColour, INT32 xFlipped)
{
	INT32 Offset;
	UINT16 *SpriteRam = (UINT16*)DrvSpriteRam;
	
	for (Offset = 0; Offset < DrvSpriteRamSize / 2; Offset += 4) {
		INT32 x, y, Code, Colour, Flash, Multi, xFlip, yFlip, Inc, Mult;
		
		Code = SpriteRam[Offset + 1] & DrvSpriteMask;
		if (!Code) continue;
		
		y = SpriteRam[Offset];
		Flash = y & 0x1000;
		if (Flash && (GetCurrentFrame() & 1)) continue;		
		
		x = SpriteRam[Offset + 2];
		Colour = (x >> 9) & DrvSpriteColourMask;
		xFlip = y & 0x2000;
		yFlip = y & 0x4000;
		
		Multi = (1 << ((y & 0x600) >> 9)) - 1;
		
		x &= 0x1ff;
		y &= 0x1ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
		x = 304 - x;
		
		y -= 8;
		
		if (yFlip) {
			Inc = -1;
		} else {
			Code += Multi;
			Inc = 1;
		}
		
		if (xFlipped) {
			xFlip = !xFlip;
			x = 304 - x;
		}
		
		Mult = -16;
		
		while (Multi >= 0) {
			INT32 RenderCode = Code - (Multi * Inc);
			INT32 RenderX = DrvSpriteXOffset + x;
			INT32 RenderY = DrvSpriteYOffset + y + (Mult * Multi);
			RenderCode &= (DrvNumSprites - 1);
			
			if (RenderX > 16 && RenderX < 304 && RenderY > 16 && RenderY < 224) {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_Mask_FlipXY(pTransDraw, RenderCode, RenderX, RenderY, Colour, 4, MaskColour, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask_FlipX(pTransDraw, RenderCode, RenderX, RenderY, Colour, 4, MaskColour, 0, DrvSprites);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_Mask_FlipY(pTransDraw, RenderCode, RenderX, RenderY, Colour, 4, MaskColour, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask(pTransDraw, RenderCode, RenderX, RenderY, Colour, 4, MaskColour, 0, DrvSprites);
					}
				}
			} else {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, RenderCode, RenderX, RenderY, Colour, 4, MaskColour, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, RenderCode, RenderX, RenderY, Colour, 4, MaskColour, 0, DrvSprites);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, RenderCode, RenderX, RenderY, Colour, 4, MaskColour, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, RenderCode, RenderX, RenderY, Colour, 4, MaskColour, 0, DrvSprites);
					}
				}
			}
			
			Multi--;
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	DrvRenderPf2Layer(DrvControl[3], DrvControl[4]);
	
	if (DrvControl[6] & 0x80) {
		DrvRenderCharLayer();
	} else {
		DrvRenderPf1Layer(DrvControl[1], DrvControl[2]);
	}
	
	DrvRenderSprites(0, 0);
	BurnTransferCopy(DrvPalette);
}

static void PangpangDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	PangpangRenderPf2Layer();
	
	if (DrvControl[6] & 0x80) {
		PangpangRenderCharLayer();
	} else {
		PangpangRenderPf1Layer();
	}
	
	DrvRenderSprites(0, 0);
	BurnTransferCopy(DrvPalette);
}

static void SuprtrioDraw()
{
	BurnTransferClear();
	HtchctchCalcPalette();
	DrvRenderPf2Layer(-DrvControl[3], -DrvControl[4]);
	DrvRenderPf1Layer(-DrvControl[1], -DrvControl[2]);
	DrvRenderSprites(0, 0);
	BurnTransferCopy(DrvPalette);
}

static void HtchctchDraw()
{
	BurnTransferClear();
	HtchctchCalcPalette();
	DrvRenderPf2Layer(DrvControl[3], DrvControl[4]);
	
	if (DrvControl[6] & 0x80) {
		DrvRenderCharLayer();
	} else {
		DrvRenderPf1Layer(DrvControl[1], DrvControl[2]);
	}
	
	DrvRenderSprites(0, 0);
	BurnTransferCopy(DrvPalette);
}

static void FncywldDraw()
{
	BurnTransferClear();
	FncywldCalcPalette();
	FncywldRenderPf2Layer();
	
	if (DrvControl[6] & 0x80) {
		FncywldRenderCharLayer();
	} else {
		FncywldRenderPf1Layer();
	}
	
	DrvRenderSprites(0x0f, 0);
	BurnTransferCopy(DrvPalette);
}

static void SdfightDraw()
{
	BurnTransferClear();
	HtchctchCalcPalette();
	DrvRenderPf2Layer(DrvControl[3], DrvControl[4]);
	
	if (DrvControl[6] & 0x80) {
		SdfightRenderCharLayer();
	} else {
		DrvRenderPf1Layer(DrvControl[1], DrvControl[2]);
	}
	
	DrvRenderSprites(0, 0);
	BurnTransferCopy(DrvPalette);
}

static void JumppopDraw()
{
	BurnTransferClear();
	JumppopCalcPalette();
	
	if (DrvControl[7] & 0x01) {
		JumppopRenderPf2Layer();
	} else {
		JumppopRenderPf2AltLayer();
	}
	
	if (DrvControl[7] & 0x02) {
		JumppopRenderPf1Layer();
	} else {
		JumppopRenderCharLayer();
	}
	
	DrvRenderSprites(0, 1);
	BurnTransferCopy(DrvPalette);
}

#define NUM_SCANLINES		315
#define SCANLINE_VBLANK_START	37
#define SCANLINE_VBLANK_END	SCANLINE_VBLANK_START + 240

static INT32 DrvFrame()
{
	INT32 nInterleave = NUM_SCANLINES;
	INT32 nSoundBufferPos = 0;
	
	if (DrvReset) DrvDoReset();

	DrvMakeInputs();
	
	nCyclesDone[0] = nCyclesDone[1] = 0;

	SekNewFrame();
	if (DrvHasZ80) ZetNewFrame();
	
	DrvVBlank = 0;
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == SCANLINE_VBLANK_START) DrvVBlank = 1;
		if (i == SCANLINE_VBLANK_END) DrvVBlank = 0;
		if (i == NUM_SCANLINES - 1) {
			SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
			if (Tumbleb2) Tumbleb2PlayMusic();
		}
		SekClose();
		
		if (DrvHasZ80) {
			// Run Z80
			nCurrentCPU = 1;
			ZetOpen(0);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
			ZetClose();
		}
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			if (DrvHasYM2151) {
				if (DrvHasZ80) ZetOpen(0);
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				if (DrvHasZ80) ZetClose();
			}
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			if (DrvHasYM2151) {
				if (DrvHasZ80) ZetOpen(0);
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				if (DrvHasZ80) ZetClose();
			}
			MSM6295Render(0, pSoundBuf, nSegmentLength);
		}
	}
	
	if (pBurnDraw) DrvRender();

	return 0;
}

static INT32 JumppopFrame()
{
	INT32 nInterleave = 1953 / 60;
	
	if (DrvReset) DrvDoReset();

	DrvMakeInputs();
	
	nCyclesDone[0] = nCyclesDone[1] = 0;

	SekNewFrame();
	ZetNewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == (nInterleave - 1)) {
			SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
		}
		SekClose();
		
		// Run Z80
		nCurrentCPU = 1;
		ZetOpen(0);
		BurnTimerUpdateYM3812(i * (nCyclesTotal[nCurrentCPU] / nInterleave));
		ZetNmi();
		ZetClose();
	}
	
	ZetOpen(0);
	BurnTimerEndFrameYM3812(nCyclesTotal[1]);
	BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
	ZetClose();
	MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	
	if (pBurnDraw) JumppopDraw();

	return 0;
}

#undef NUM_SCANLINES
#undef SCANLINE_VBLANK_START
#undef SCANLINE_VBLANK_END

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029676;
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
		if (DrvHasZ80) ZetScan(nAction);
		if (DrvHasYM2151) BurnYM2151Scan(nAction);
		MSM6295Scan(0, nAction);

		// Scan critical driver variables
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
		SCAN_VAR(DrvDip);
		SCAN_VAR(DrvInput);
		SCAN_VAR(DrvVBlank);
		SCAN_VAR(DrvOkiBank);
		SCAN_VAR(DrvZ80Bank);
		SCAN_VAR(DrvTileBank);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(Tumbleb2MusicCommand);
		SCAN_VAR(Tumbleb2MusicBank);
		SCAN_VAR(Tumbleb2MusicIsPlaying);
	}
	
	if (nAction & ACB_WRITE) {
		if (DrvOkiBank) {
			if (Jumpkids) {
				memcpy(MSM6295ROM + 0x20000, DrvMSM6295ROMSrc + (DrvOkiBank * 0x20000), 0x20000);
			} else {
				memcpy(MSM6295ROM + 0x30000, DrvMSM6295ROMSrc + 0x30000 + (DrvOkiBank * 0x10000), 0x10000);
			}
		}
		
		if (DrvZ80Bank) {
			ZetOpen(0);
			ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom + (DrvZ80Bank * 0x4000));
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom + (DrvZ80Bank * 0x4000));
			ZetClose();
		}
	}
	
	return 0;
}

struct BurnDriver BurnDrvTumbleb = {
	"tumbleb", "tumblep", NULL, NULL, "1991",
	"Tumble Pop (bootleg set 1)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, TumblebRomInfo, TumblebRomName, NULL, NULL, TumblebInputInfo, TumblebDIPInfo,
	TumblebInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvTumbleb2 = {
	"tumbleb2", "tumblep", NULL, NULL, "1991",
	"Tumble Pop (bootleg set 2)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, Tumbleb2RomInfo, Tumbleb2RomName, NULL, NULL, TumblebInputInfo, TumblebDIPInfo,
	Tumbleb2Init, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvJumpkids = {
	"jumpkids", NULL, NULL, NULL, "1993",
	"Jump Kids\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, JumpkidsRomInfo, JumpkidsRomName, NULL, NULL, TumblebInputInfo, TumblebDIPInfo,
	JumpkidsInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvMetlsavr = {
	"metlsavr", NULL, NULL, NULL, "1994",
	"Metal Saver\0", NULL, "First Amusement", "Miscellaneous",
	L"Metal Saver\0\uBA54\uD0C8\uC138\uC774\uBC84\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, MetlsavrRomInfo, MetlsavrRomName, NULL, NULL, MetlsavrInputInfo, MetlsavrDIPInfo,
	MetlsavrInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvPangpang = {
	"pangpang", NULL, NULL, NULL, "1994",
	"Pang Pang\0", NULL, "Dong Gue La Mi Ltd.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, PangpangRomInfo, PangpangRomName, NULL, NULL, TumblebInputInfo, TumblebDIPInfo,
	PangpangInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvSuprtrio = {
	"suprtrio", NULL, NULL, NULL, "1994",
	"Super Trio\0", NULL, "Gameace", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, SuprtrioRomInfo, SuprtrioRomName, NULL, NULL, SuprtrioInputInfo, SuprtrioDIPInfo,
	SuprtrioInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvHtchctch = {
	"htchctch", NULL, NULL, NULL, "1995",
	"Hatch Catch\0", NULL, "SemiCom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, HtchctchRomInfo, HtchctchRomName, NULL, NULL, HtchctchInputInfo, HtchctchDIPInfo,
	HtchctchInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvCookbib = {
	"cookbib", NULL, NULL, NULL, "1995",
	"Cookie & Bibi\0", NULL, "SemiCom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, CookbibRomInfo, CookbibRomName, NULL, NULL, HtchctchInputInfo, CookbibDIPInfo,
	CookbibInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvChokChok = {
	"chokchok", NULL, NULL, NULL, "1995",
	"Choky! Choky!\0", NULL, "SemiCom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, ChokchokRomInfo, ChokchokRomName, NULL, NULL, HtchctchInputInfo, ChokchokDIPInfo,
	ChokchokInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvWlstar = {
	"wlstar", NULL, NULL, NULL, "1995",
	"Wonder League Star - Sok-Magicball Fighting (Korea)\0", NULL, "Mijin", "Miscellaneous",
	L"\uC6D0\uB354\uB9AC\uADF8\uC2A4\uD0C0 - \uC18D \uB9E4\uC9C1\uBCFC \uD30C\uC774\uD305 (Korea)\0Wonder League Star - Sok-Magicball Fighting\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_SPORTSMISC, 0,
	NULL, WlstarRomInfo, WlstarRomName, NULL, NULL, MetlsavrInputInfo, WlstarDIPInfo,
	WlstarInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvWondl96 = {
	"wondl96", NULL, NULL, NULL, "1995",
	"Wonder League '96 (Korea)\0", NULL, "SemiCom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_SPORTSMISC, 0,
	NULL, Wondl96RomInfo, Wondl96RomName, NULL, NULL, MetlsavrInputInfo, Wondl96DIPInfo,
	Wondl96Init, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvFancywld = {
	"fncywld", NULL, NULL, NULL, "1996",
	"Fancy World - Earth of Crisis\0", NULL, "Unico", "Miscellaneous",
	L"Fancy World - Earth of Crisis\0\uD39C\uC2DC\uC6D4\uB4DC \uD658\uC0C1\uC758 \uC138\uACC4 - Earth of Crisis\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, FncywldRomInfo, FncywldRomName, NULL, NULL, FncywldInputInfo, FncywldDIPInfo,
	FncywldInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvSdfight = {
	"sdfight", NULL, NULL, NULL, "1996",
	"SD Fighters (Korea)\0", NULL, "SemiCom", "Miscellaneous",
	L"\uFF33\uFF24 \uD30C\uC774\uD130\uC988 (Korea)\0SD Fighters\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, 0,
	NULL, SdfightRomInfo, SdfightRomName, NULL, NULL, MetlsavrInputInfo, SdfightDIPInfo,
	SdfightInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvBcstry = {
	"bcstry", NULL, NULL, NULL, "1997",
	"B.C. Story (set 1)\0", NULL, "SemiCom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_SPORTSMISC, 0,
	NULL, BcstryRomInfo, BcstryRomName, NULL, NULL, MetlsavrInputInfo, BcstryDIPInfo,
	BcstryInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvBcstrya = {
	"bcstrya", "bcstry", NULL, NULL, "1997",
	"B.C. Story (set 2)\0", NULL, "SemiCom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_SPORTSMISC, 0,
	NULL, BcstryaRomInfo, BcstryaRomName, NULL, NULL, MetlsavrInputInfo, BcstryDIPInfo,
	BcstryInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvSemibase = {
	"semibase", NULL, NULL, NULL, "1997",
	"MuHanSeungBu (SemiCom Baseball) (Korea)\0", NULL, "SemiCom", "Miscellaneous",
	L"\u7121\u9650\u52DD\u8CA0\0\uC804\uC6D0 \uAD6D\uC81C\uB9AC\uADF8 \uC804 (SemiCom Baseball) (Korea)\0MuHanSeungBu\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_SPORTSMISC, 0,
	NULL, SemibaseRomInfo, SemibaseRomName, NULL, NULL, SemibaseInputInfo, SemibaseDIPInfo,
	SemibaseInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvDquizgo = {
	"dquizgo", NULL, NULL, NULL, "1998",
	"Date Quiz Go Go (Korea)\0", NULL, "SemiCom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_QUIZ, 0,
	NULL, DquizgoRomInfo, DquizgoRomName, NULL, NULL, MetlsavrInputInfo, DquizgoDIPInfo,
	DquizgoInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvJumppop = {
	"jumppop", NULL, NULL, NULL, "2001",
	"Jumping Pop\0", "Missing Sounds", "ESD", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, JumppopRomInfo, JumppopRomName, NULL, NULL, JumppopInputInfo, JumppopDIPInfo,
	JumppopInit, JumppopExit, JumppopFrame, NULL, DrvScan,
	NULL, 0x400, 320, 240, 4, 3
};
