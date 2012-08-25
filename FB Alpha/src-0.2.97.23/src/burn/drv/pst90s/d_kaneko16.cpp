#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "msm6295.h"
#include "burn_ym2151.h"
#include "eeprom.h"
#include "kanekotb.h"

#include "driver.h"
extern "C" {
#include "ay8910.h"
}

/*==============================================================================================
Driver Variables
===============================================================================================*/

static UINT8 Kaneko16InputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 Kaneko16InputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 Kaneko16InputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 Kaneko16InputPort3[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 Kaneko16Dip[2]        = {0, 0};
static UINT8 Kaneko16Input[4]      = {0x00, 0x00, 0x00, 0x00};
static UINT8 Kaneko16Reset         = 0;

static UINT8 *Mem                  = NULL;
static UINT8 *MemEnd               = NULL;
static UINT8 *RamStart             = NULL;
static UINT8 *RamEnd               = NULL;
static UINT8 *Kaneko16Rom          = NULL;
static UINT8 *Kaneko16Z80Rom       = NULL;
static UINT8 *MSM6295ROMData       = NULL;
static UINT8 *MSM6295ROMData2      = NULL;
static UINT8 *Kaneko16Ram          = NULL;
static UINT8 *Kaneko16Z80Ram       = NULL;
static UINT8 *Kaneko16MCURam       = NULL;
static UINT8 *Kaneko16NVRam        = NULL;
static UINT8 *Kaneko16PaletteRam   = NULL;
static UINT8 *Kaneko16SpriteRam    = NULL;
static UINT8 *Kaneko16Video0Ram    = NULL;
static UINT8 *Kaneko16Video1Ram    = NULL;
static UINT8 *Kaneko16Video2Ram    = NULL;
static UINT8 *Kaneko16Video3Ram    = NULL;
static UINT8 *Kaneko16VScrl0Ram    = NULL;
static UINT8 *Kaneko16VScrl1Ram    = NULL;
static UINT8 *Kaneko16VScrl2Ram    = NULL;
static UINT8 *Kaneko16VScrl3Ram    = NULL;
static UINT32 *Kaneko16Palette     = NULL;
static UINT8 *Kaneko16Tiles        = NULL;
static UINT8 *Kaneko16Tiles2       = NULL;
static UINT8 *Kaneko16Sprites      = NULL;
static UINT8 *Kaneko16TempGfx      = NULL;

static INT16* pFMBuffer;
static INT16* pAY8910Buffer[6];

static UINT32 Kaneko16SoundLatch;
static INT32 MSM6295Bank0;
static INT32 MSM6295Bank1;

static UINT16 ToyboxMCUCom[4];

static UINT16 Kaneko16Bg15Reg;
static UINT16 Kaneko16Bg15Select;
static UINT16 *Kaneko16Bg15Data = NULL;
static UINT8 Kaneko16RecalcBg15Palette;

static UINT16 Kaneko16SpriteFlipX;
static UINT16 Kaneko16SpriteFlipY;
static UINT16 Kaneko16SpriteRegs[0x20];
static UINT16 Kaneko16Layer0Regs[0x10];
static UINT16 Kaneko16Layer1Regs[0x10];
static UINT32 Kaneko16SpritesColourOffset;
static UINT32 Kaneko16LayersColourOffset;
static UINT32 Kaneko16SpritesColourMask;
static UINT16 Kaneko16DisplayEnable;
static INT32 Kaneko16TilesXOffset;
static INT32 Kaneko16TilesYOffset;
static INT32 Kaneko16SpriteXOffset;

static UINT32 *LayerQueueXY[4];
static UINT32 *LayerQueueColour[4];
static UINT8 *LayerQueuePriority[4];
static INT32 LayerQueueSize[4];

static INT32 Kaneko16NumTiles;
static INT32 Kaneko16NumTiles2;
static INT32 Kaneko16NumSprites;
static INT32 Kaneko16SpriteRamSize;

// Flags
static INT32 Kaneko168BppSprites;
static INT32 Kaneko16Eeprom;
static INT32 Kaneko16Bg15;
static INT32 Gtmr;
static INT32 Bloodwar;
static INT32 Bonkadv;
static INT32 Mgcrystl;

typedef void (*MCURun)();
MCURun ToyboxMCURun;

typedef void (*FrameRender)();
FrameRender Kaneko16FrameRender;
static void BerlwallFrameRender();
static void BlazeonFrameRender();
static void BloodwarFrameRender();
static void ExplbrkrFrameRender();
static void GtmrFrameRender();
static void MgcrystlFrameRender();

typedef INT32 (*ParseSprite)(INT32, struct tempsprite*);
ParseSprite Kaneko16ParseSprite;
static INT32 Kaneko16ParseSpriteType0(INT32 i, struct tempsprite *s);
static INT32 Kaneko16ParseSpriteType1(INT32 i, struct tempsprite *s);
static INT32 Kaneko16ParseSpriteType2(INT32 i, struct tempsprite *s);

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;
static INT32 nSoundBufferPos;

struct tempsprite
{
	INT32 code,color;
	INT32 x,y;
	INT32 xoffs,yoffs;
	INT32 flipx,flipy;
	INT32 priority;
};

static struct
{
	struct tempsprite *first_sprite;
}	spritelist;

/*==============================================================================================
Input Definitions
===============================================================================================*/

static struct BurnInputInfo BerlwallInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , Kaneko16InputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , Kaneko16InputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , Kaneko16InputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , Kaneko16InputPort2 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , Kaneko16InputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , Kaneko16InputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , Kaneko16InputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , Kaneko16InputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , Kaneko16InputPort0 + 6, "p1 fire 3" },

	{"P2 Up"             , BIT_DIGITAL  , Kaneko16InputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , Kaneko16InputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , Kaneko16InputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , Kaneko16InputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , Kaneko16InputPort1 + 6, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL  , &Kaneko16Reset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , Kaneko16InputPort2 + 6, "service"   },
	{"Test"              , BIT_DIGITAL  , Kaneko16InputPort2 + 4, "diag"      },
	{"Tilt"              , BIT_DIGITAL  , Kaneko16InputPort2 + 5, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH, Kaneko16Dip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, Kaneko16Dip + 1       , "dip"       },
};

STDINPUTINFO(Berlwall)

static struct BurnInputInfo BlazeonInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , Kaneko16InputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , Kaneko16InputPort0 + 6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , Kaneko16InputPort1 + 7, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , Kaneko16InputPort1 + 6, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , Kaneko16InputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , Kaneko16InputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , Kaneko16InputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , Kaneko16InputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort0 + 5, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL  , Kaneko16InputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , Kaneko16InputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , Kaneko16InputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , Kaneko16InputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &Kaneko16Reset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , Kaneko16InputPort2 + 7, "service"   },
	{"Test"              , BIT_DIGITAL  , Kaneko16InputPort2 + 5, "diag"      },
	{"Tilt"              , BIT_DIGITAL  , Kaneko16InputPort2 + 6, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH, Kaneko16Dip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, Kaneko16Dip + 1       , "dip"       },
};

STDINPUTINFO(Blazeon)

static struct BurnInputInfo BloodwarInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , Kaneko16InputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , Kaneko16InputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , Kaneko16InputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , Kaneko16InputPort2 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , Kaneko16InputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , Kaneko16InputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , Kaneko16InputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , Kaneko16InputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , Kaneko16InputPort0 + 6, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL  , Kaneko16InputPort0 + 7, "p1 fire 4" },

	{"P2 Up"             , BIT_DIGITAL  , Kaneko16InputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , Kaneko16InputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , Kaneko16InputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , Kaneko16InputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , Kaneko16InputPort1 + 6, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL  , Kaneko16InputPort3 + 2, "p2 fire 4" },

	{"Reset"             , BIT_DIGITAL  , &Kaneko16Reset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , Kaneko16InputPort2 + 6, "service"   },
	{"Test"              , BIT_DIGITAL  , Kaneko16InputPort2 + 4, "diag"      },
	{"Dip 1"             , BIT_DIPSWITCH, Kaneko16Dip + 0       , "dip"       },
};

STDINPUTINFO(Bloodwar)

static struct BurnInputInfo BonkadvInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , Kaneko16InputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , Kaneko16InputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , Kaneko16InputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , Kaneko16InputPort2 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , Kaneko16InputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , Kaneko16InputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , Kaneko16InputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , Kaneko16InputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort0 + 5, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL  , Kaneko16InputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , Kaneko16InputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , Kaneko16InputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , Kaneko16InputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &Kaneko16Reset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , Kaneko16InputPort2 + 6, "service"   },
	{"Test"              , BIT_DIGITAL  , Kaneko16InputPort2 + 4, "diag"      },
	{"Tilt"              , BIT_DIGITAL  , Kaneko16InputPort2 + 5, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH, Kaneko16Dip + 0       , "dip"       },
};

STDINPUTINFO(Bonkadv)

static struct BurnInputInfo ExplbrkrInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , Kaneko16InputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , Kaneko16InputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , Kaneko16InputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , Kaneko16InputPort2 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , Kaneko16InputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , Kaneko16InputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , Kaneko16InputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , Kaneko16InputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort0 + 5, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL  , Kaneko16InputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , Kaneko16InputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , Kaneko16InputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , Kaneko16InputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &Kaneko16Reset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , Kaneko16InputPort2 + 6, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, Kaneko16Dip + 0       , "dip"       },
};

STDINPUTINFO(Explbrkr)

static struct BurnInputInfo GtmrInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , Kaneko16InputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , Kaneko16InputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , Kaneko16InputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , Kaneko16InputPort2 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , Kaneko16InputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , Kaneko16InputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , Kaneko16InputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , Kaneko16InputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort0 + 5, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort0 + 4, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL  , Kaneko16InputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , Kaneko16InputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , Kaneko16InputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , Kaneko16InputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort1 + 5, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort1 + 4, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &Kaneko16Reset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , Kaneko16InputPort2 + 6, "service"   },
	{"Test"              , BIT_DIGITAL  , Kaneko16InputPort2 + 4, "diag"      },
	{"Tilt"              , BIT_DIGITAL  , Kaneko16InputPort2 + 5, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH, Kaneko16Dip + 0       , "dip"       },
};

STDINPUTINFO(Gtmr)

static struct BurnInputInfo MgcrystlInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , Kaneko16InputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , Kaneko16InputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , Kaneko16InputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , Kaneko16InputPort2 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , Kaneko16InputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , Kaneko16InputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , Kaneko16InputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , Kaneko16InputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort0 + 5, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL  , Kaneko16InputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , Kaneko16InputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , Kaneko16InputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , Kaneko16InputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , Kaneko16InputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , Kaneko16InputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &Kaneko16Reset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , Kaneko16InputPort2 + 6, "service"   },
	{"Test"              , BIT_DIGITAL  , Kaneko16InputPort2 + 4, "diag"      },
	{"Tilt"              , BIT_DIGITAL  , Kaneko16InputPort2 + 5, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH, Kaneko16Dip + 0       , "dip"       },
};

STDINPUTINFO(Mgcrystl)

inline void Kaneko16ClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

inline void Kaneko16MakeInputs()
{
	// Reset Inputs
	Kaneko16Input[0] = Kaneko16Input[1] = Kaneko16Input[2] = Kaneko16Input[3] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		Kaneko16Input[0] |= (Kaneko16InputPort0[i] & 1) << i;
		Kaneko16Input[1] |= (Kaneko16InputPort1[i] & 1) << i;
		Kaneko16Input[2] |= (Kaneko16InputPort2[i] & 1) << i;
		Kaneko16Input[3] |= (Kaneko16InputPort3[i] & 1) << i;
	}

	// Clear Opposites
	Kaneko16ClearOpposites(&Kaneko16Input[0]);
	Kaneko16ClearOpposites(&Kaneko16Input[1]);
}

/*==============================================================================================
Dip Switch Definitions
===============================================================================================*/

static struct BurnDIPInfo BerlwallDIPList[]=
{
	// Default Values
	{0x16, 0xff, 0xff, 0xff, NULL                                 },
	{0x17, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Screen Direction"                   },
	{0x16, 0x01, 0x01, 0x01, "Normal"                             },
	{0x16, 0x01, 0x01, 0x00, "Reverse"                            },
	
	{0   , 0xfe, 0   , 8   , "Coin Slot A"                        },
	{0x16, 0x01, 0x1c, 0x1c, "1 Coin  1 Credit"                   },
	{0x16, 0x01, 0x1c, 0x18, "1 Coins 2 Credits"                  },
	{0x16, 0x01, 0x1c, 0x14, "1 Coins 3 Credits"                  },
	{0x16, 0x01, 0x1c, 0x10, "1 Coins 4 Credits"                  },
	{0x16, 0x01, 0x1c, 0x0c, "2 Coins 1 Credit"                   },
	{0x16, 0x01, 0x1c, 0x08, "2 Coin  3 Credits"                  },
	{0x16, 0x01, 0x1c, 0x04, "3 Coins 1 Credit"                   },
	{0x16, 0x01, 0x1c, 0x00, "4 Coins 1 Credit"                   },
	
	{0   , 0xfe, 0   , 8   , "Coin Slot B"                        },
	{0x16, 0x01, 0xe0, 0xe0, "1 Coin  1 Credit"                   },
	{0x16, 0x01, 0xe0, 0xc0, "1 Coins 2 Credits"                  },
	{0x16, 0x01, 0xe0, 0xa0, "1 Coins 3 Credits"                  },
	{0x16, 0x01, 0xe0, 0x80, "1 Coins 4 Credits"                  },
	{0x16, 0x01, 0xe0, 0x60, "2 Coins 1 Credit"                   },
	{0x16, 0x01, 0xe0, 0x40, "2 Coin  3 Credits"                  },
	{0x16, 0x01, 0xe0, 0x20, "1 Coin  5 Credits"                  },
	{0x16, 0x01, 0xe0, 0x00, "1 Coin  6 Credits"                  },

	// Dip 2	
	{0   , 0xfe, 0   , 4   , "Game Level"                         },
	{0x17, 0x01, 0x03, 0x03, "Standard"                           },
	{0x17, 0x01, 0x03, 0x02, "Easy"                               },
	{0x17, 0x01, 0x03, 0x01, "Difficult"                          },
	{0x17, 0x01, 0x03, 0x00, "Very Difficult"                     },
	
	{0   , 0xfe, 0   , 4   , "Number of Players"                  },
	{0x17, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x17, 0x01, 0x0c, 0x08, "2"                                  },
	{0x17, 0x01, 0x0c, 0x04, "5"                                  },
	{0x17, 0x01, 0x0c, 0x00, "7"                                  },
	
	{0   , 0xfe, 0   , 4   , "Language Type"                      },
	{0x17, 0x01, 0x30, 0x30, "English"                            },
	{0x17, 0x01, 0x30, 0x20, "Italy"                              },
	{0x17, 0x01, 0x30, 0x10, "Germany"                            },
	{0x17, 0x01, 0x30, 0x00, "Pause Mode"                         },
	
	{0   , 0xfe, 0   , 2   , "Demonstration Sound"                },
	{0x17, 0x01, 0x40, 0x00, "Off"                                },
	{0x17, 0x01, 0x40, 0x40, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Testmode Switch"                    },
	{0x17, 0x01, 0x80, 0x80, "Normal Game"                        },
	{0x17, 0x01, 0x80, 0x00, "Testmode"                           },
};

STDDIPINFO(Berlwall)

static struct BurnDIPInfo BerlwalltDIPList[]=
{
	// Default Values
	{0x16, 0xff, 0xff, 0xff, NULL                                 },
	{0x17, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Screen Direction"                   },
	{0x16, 0x01, 0x01, 0x01, "Normal"                             },
	{0x16, 0x01, 0x01, 0x00, "Reverse"                            },
	
	{0   , 0xfe, 0   , 8   , "Coin Slot A"                        },
	{0x16, 0x01, 0x1c, 0x1c, "1 Coin  1 Credit"                   },
	{0x16, 0x01, 0x1c, 0x18, "1 Coins 2 Credits"                  },
	{0x16, 0x01, 0x1c, 0x14, "1 Coins 3 Credits"                  },
	{0x16, 0x01, 0x1c, 0x10, "1 Coins 4 Credits"                  },
	{0x16, 0x01, 0x1c, 0x0c, "2 Coins 1 Credit"                   },
	{0x16, 0x01, 0x1c, 0x08, "2 Coin  3 Credits"                  },
	{0x16, 0x01, 0x1c, 0x04, "3 Coins 1 Credit"                   },
	{0x16, 0x01, 0x1c, 0x00, "4 Coins 1 Credit"                   },
	
	{0   , 0xfe, 0   , 8   , "Coin Slot B"                        },
	{0x16, 0x01, 0xe0, 0xe0, "1 Coin  1 Credit"                   },
	{0x16, 0x01, 0xe0, 0xc0, "1 Coins 2 Credits"                  },
	{0x16, 0x01, 0xe0, 0xa0, "1 Coins 3 Credits"                  },
	{0x16, 0x01, 0xe0, 0x80, "1 Coins 4 Credits"                  },
	{0x16, 0x01, 0xe0, 0x60, "2 Coins 1 Credit"                   },
	{0x16, 0x01, 0xe0, 0x40, "2 Coin  3 Credits"                  },
	{0x16, 0x01, 0xe0, 0x20, "1 Coin  5 Credits"                   },
	{0x16, 0x01, 0xe0, 0x00, "1 Coin  6 Credits"                   },

	// Dip 2	
	{0   , 0xfe, 0   , 4   , "Game Level"                         },
	{0x17, 0x01, 0x03, 0x03, "Standard"                           },
	{0x17, 0x01, 0x03, 0x02, "Easy"                               },
	{0x17, 0x01, 0x03, 0x01, "Difficult"                          },
	{0x17, 0x01, 0x03, 0x00, "Very Difficult"                     },
	
	{0   , 0xfe, 0   , 4   , "Number of Players"                  },
	{0x17, 0x01, 0x0c, 0x0c, "2"                                  },
	{0x17, 0x01, 0x0c, 0x08, "3"                                  },
	{0x17, 0x01, 0x0c, 0x04, "5"                                  },
	{0x17, 0x01, 0x0c, 0x00, "1"                                  },
	
	{0   , 0xfe, 0   , 4   , "Language Type"                      },
	{0x17, 0x01, 0x30, 0x30, "English"                            },
	{0x17, 0x01, 0x30, 0x20, "Italy"                              },
	{0x17, 0x01, 0x30, 0x10, "Germany"                            },
	{0x17, 0x01, 0x30, 0x00, "Pause Mode"                         },
	
	{0   , 0xfe, 0   , 2   , "Demonstration Sound"                },
	{0x17, 0x01, 0x40, 0x00, "Off"                                },
	{0x17, 0x01, 0x40, 0x40, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Testmode Switch"                    },
	{0x17, 0x01, 0x80, 0x80, "Normal Game"                        },
	{0x17, 0x01, 0x80, 0x00, "Testmode"                           },
};

STDDIPINFO(Berlwallt)

static struct BurnDIPInfo BlazeonDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xff, NULL                                 },
	{0x15, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x14, 0x01, 0x03, 0x03, "Standard"                           },
	{0x14, 0x01, 0x03, 0x02, "Easy"                               },
	{0x14, 0x01, 0x03, 0x01, "Hard"                               },
	{0x14, 0x01, 0x03, 0x00, "Strong"                             },
	
	{0   , 0xfe, 0   , 4   , "Number of Players"                  },
	{0x14, 0x01, 0x0c, 0x00, "2"                                  },
	{0x14, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x14, 0x01, 0x0c, 0x08, "4"                                  },
	{0x14, 0x01, 0x0c, 0x04, "5"                                  },
	
	{0   , 0xfe, 0   , 2   , "Demonstration Sound"                },
	{0x14, 0x01, 0x10, 0x00, "Off"                                },
	{0x14, 0x01, 0x10, 0x10, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                       },
	{0x14, 0x01, 0x80, 0x80, "Off"                                },
	{0x14, 0x01, 0x80, 0x00, "On"                                 },
	
	// Dip 2	
	{0   , 0xfe, 0   , 16   , "Coin Rate Slot A"                  },
	{0x15, 0x01, 0x0f, 0x00, "1 Coin 1 Play"                      },
	{0x15, 0x01, 0x0f, 0x01, "2 Coin 3 Play"                      },
	{0x15, 0x01, 0x0f, 0x02, "4 Coin 5 Play"                      },
	{0x15, 0x01, 0x0f, 0x03, "5 Coin 6 Play"                      },
	{0x15, 0x01, 0x0f, 0x04, "4 Coin 3 Play"                      },
	{0x15, 0x01, 0x0f, 0x05, "6 Coin 3 Play"                      },
	{0x15, 0x01, 0x0f, 0x06, "2 Coin 3 Play"                      },
	{0x15, 0x01, 0x0f, 0x07, "4 Coin 1 Play"                      },
	{0x15, 0x01, 0x0f, 0x08, "3 Coin 1 Play"                      },
	{0x15, 0x01, 0x0f, 0x09, "2 Coin 1 Play"                      },
	{0x15, 0x01, 0x0f, 0x0a, "1 Coin 6 Play"                      },
	{0x15, 0x01, 0x0f, 0x0b, "1 Coin 5 Play"                      },
	{0x15, 0x01, 0x0f, 0x0c, "1 Coin 4 Play"                      },
	{0x15, 0x01, 0x0f, 0x0d, "1 Coin 3 Play"                      },
	{0x15, 0x01, 0x0f, 0x0e, "1 Coin 2 Play"                      },
	{0x15, 0x01, 0x0f, 0x0f, "1 Coin 1 Play"                      },
		
	{0   , 0xfe, 0   , 16   , "Coin Rate Slot B"                  },
	{0x15, 0x01, 0xf0, 0x00, "1 Coin 1 Play"                      },
	{0x15, 0x01, 0xf0, 0x10, "2 Coin 3 Play"                      },
	{0x15, 0x01, 0xf0, 0x20, "4 Coin 5 Play"                      },
	{0x15, 0x01, 0xf0, 0x30, "5 Coin 6 Play"                      },
	{0x15, 0x01, 0xf0, 0x40, "4 Coin 3 Play"                      },
	{0x15, 0x01, 0xf0, 0x50, "6 Coin 3 Play"                      },
	{0x15, 0x01, 0xf0, 0x60, "2 Coin 3 Play"                      },
	{0x15, 0x01, 0xf0, 0x70, "4 Coin 1 Play"                      },
	{0x15, 0x01, 0xf0, 0x80, "3 Coin 1 Play"                      },
	{0x15, 0x01, 0xf0, 0x90, "2 Coin 1 Play"                      },
	{0x15, 0x01, 0xf0, 0xa0, "1 Coin 6 Play"                      },
	{0x15, 0x01, 0xf0, 0xb0, "1 Coin 5 Play"                      },
	{0x15, 0x01, 0xf0, 0xc0, "1 Coin 4 Play"                      },
	{0x15, 0x01, 0xf0, 0xd0, "1 Coin 3 Play"                      },
	{0x15, 0x01, 0xf0, 0xe0, "1 Coin 2 Play"                      },
	{0x15, 0x01, 0xf0, 0xf0, "1 Coin 1 Play"                      },
};

STDDIPINFO(Blazeon)

static struct BurnDIPInfo BloodwarDIPList[]=
{
	// Default Values
	{0x17, 0xff, 0xff, 0x01, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sound"                         },
	{0x17, 0x01, 0x01, 0x00, "Off"                                },
	{0x17, 0x01, 0x01, 0x01, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                       },
	{0x17, 0x01, 0x02, 0x00, "Off"                                },
	{0x17, 0x01, 0x02, 0x02, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Screen Flip"                        },
	{0x17, 0x01, 0x04, 0x00, "Normal"                             },
	{0x17, 0x01, 0x04, 0x04, "Reverse"                            },
	
	{0   , 0xfe, 0   , 8   , "Difficulty"                         },
	{0x17, 0x01, 0x38, 0x00, "1 Easy"                             },
	{0x17, 0x01, 0x38, 0x08, "2"                                  },
	{0x17, 0x01, 0x38, 0x10, "3"                                  },
	{0x17, 0x01, 0x38, 0x18, "4"                                  },
	{0x17, 0x01, 0x38, 0x20, "5"                                  },
	{0x17, 0x01, 0x38, 0x28, "6"                                  },
	{0x17, 0x01, 0x38, 0x30, "7"                                  },
	{0x17, 0x01, 0x38, 0x38, "8 Hard"                             },
	
	{0   , 0xfe, 0   , 2   , "Join During Game"                   },
	{0x17, 0x01, 0x40, 0x40, "Impossible"                         },
	{0x17, 0x01, 0x40, 0x00, "Possible"                           },
	
	{0   , 0xfe, 0   , 2   , "Continue Coin"                      },
	{0x17, 0x01, 0x80, 0x00, "Normal"                             },
	{0x17, 0x01, 0x80, 0x80, "Continue coin"                      },
};

STDDIPINFO(Bloodwar)

static struct BurnDIPInfo BonkadvDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0x10, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Screen Direction"                   },
	{0x14, 0x01, 0x01, 0x00, "Standard"                           },
	{0x14, 0x01, 0x01, 0x01, "Reverse"                            },
	
	{0   , 0xfe, 0   , 2   , "Free Play"                          },
	{0x14, 0x01, 0x02, 0x00, "Off"                                },
	{0x14, 0x01, 0x02, 0x02, "On"                                 },
		
	{0   , 0xfe, 0   , 2   , "Demo Sound"                         },
	{0x14, 0x01, 0x04, 0x04, "Off"                                },
	{0x14, 0x01, 0x04, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Title Level Display"                },
	{0x14, 0x01, 0x10, 0x00, "Off"                                },
	{0x14, 0x01, 0x10, 0x10, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                       },
	{0x14, 0x01, 0x80, 0x00, "Off"                                },
	{0x14, 0x01, 0x80, 0x80, "On"                                 },
};

STDDIPINFO(Bonkadv)

static struct BurnDIPInfo ExplbrkrDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"                       },
	{0x12, 0x01, 0x02, 0x02, "Off"                                },
	{0x12, 0x01, 0x02, 0x00, "On"                                 },
};

STDDIPINFO(Explbrkr)

static struct BurnDIPInfo GtmrDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0x00, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"                       },
	{0x14, 0x01, 0x01, 0x00, "Off"                                },
	{0x14, 0x01, 0x01, 0x01, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"                        },
	{0x14, 0x01, 0x02, 0x00, "Off"                                },
	{0x14, 0x01, 0x02, 0x02, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x14, 0x01, 0x04, 0x00, "Upright"                            },
	{0x14, 0x01, 0x04, 0x04, "Cocktail"                           },
	
	{0   , 0xfe, 0   , 4   , "Controls"                           },
	{0x14, 0x01, 0x18, 0x00, "1 Joystick"                         },
	{0x14, 0x01, 0x18, 0x10, "2 Joysticks"                        },
	{0x14, 0x01, 0x18, 0x08, "Wheel (360)"                        },
	{0x14, 0x01, 0x18, 0x18, "Wheel (270)"                        },
	
	{0   , 0xfe, 0   , 2   , "Use Brake"                          },
	{0x14, 0x01, 0x20, 0x20, "Off"                                },
	{0x14, 0x01, 0x20, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "National Anthem & Flag"             },
	{0x14, 0x01, 0xc0, 0x00, "Use Memory"                         },
	{0x14, 0x01, 0xc0, 0x40, "Anthem Only"                        },
	{0x14, 0x01, 0xc0, 0x80, "Flag Only"                          },
	{0x14, 0x01, 0xc0, 0xc0, "None"                               },
};

STDDIPINFO(Gtmr)

static struct BurnDIPInfo Gtmr2DIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0x00, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 5   , "Communication"                      },
	{0x14, 0x01, 0x07, 0x00, "None"                               },
	{0x14, 0x01, 0x07, 0x01, "Machine 1"                          },
	{0x14, 0x01, 0x07, 0x02, "Machine 2"                          },
	{0x14, 0x01, 0x07, 0x03, "Machine 3"                          },
	{0x14, 0x01, 0x07, 0x04, "Machine 4"                          },
		
	{0   , 0xfe, 0   , 4   , "Controls"                           },
	{0x14, 0x01, 0x18, 0x00, "Joystick"                           },
	{0x14, 0x01, 0x18, 0x10, "Wheel (360)"                        },
	{0x14, 0x01, 0x18, 0x08, "Wheel (270D)"                       },
	{0x14, 0x01, 0x18, 0x18, "Wheel (270A)"                       },
	
	{0   , 0xfe, 0   , 2   , "Pedal Function"                     },
	{0x14, 0x01, 0x20, 0x00, "Microswitch"                        },
	{0x14, 0x01, 0x20, 0x20, "Potentiometer"                      },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"                        },
	{0x14, 0x01, 0x40, 0x00, "Off"                                },
	{0x14, 0x01, 0x40, 0x40, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                       },
	{0x14, 0x01, 0x80, 0x00, "Off"                                },
	{0x14, 0x01, 0x80, 0x80, "On"                                 },
};

STDDIPINFO(Gtmr2)

static struct BurnDIPInfo MgcrystlDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xff, NULL                                 },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                        },
	{0x14, 0x01, 0x01, 0x01, "Off"                                },
	{0x14, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"                       },
	{0x14, 0x01, 0x02, 0x02, "Off"                                },
	{0x14, 0x01, 0x02, 0x00, "On"                                 },
};

STDDIPINFO(Mgcrystl)

/*==============================================================================================
ROM Descriptions
===============================================================================================*/

static struct BurnRomInfo BerlwallRomDesc[] = {
	{ "u23_01.bin",        0x020000, 0x76b526ce, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "u39_01.bin",        0x020000, 0x78fa7ef2, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "bw001",             0x080000, 0xbc927260, BRF_GRA },		  //  2	Sprites
	{ "bw002",             0x080000, 0x223f5465, BRF_GRA },		  //  3	Sprites
	{ "bw300",             0x020000, 0xb258737a, BRF_GRA },		  //  4	Sprites
	
	{ "bw003",             0x080000, 0xfbb4b72d, BRF_GRA },		  //  5 Tiles
	
	{ "bw004",             0x080000, 0x5300c34d, BRF_GRA },		  //  6 High colour background
	{ "bw008",             0x080000, 0x9aaf2f2f, BRF_GRA },		  //  7 High colour background
	{ "bw005",             0x080000, 0x16db6d43, BRF_GRA },		  //  8 High colour background
	{ "bw009",             0x080000, 0x1151a0b0, BRF_GRA },		  //  9 High colour background
	{ "bw006",             0x080000, 0x73a35d1f, BRF_GRA },		  //  10 High colour background
	{ "bw00a",             0x080000, 0xf447dfc2, BRF_GRA },		  //  11 High colour background
	{ "bw007",             0x080000, 0x97f85c87, BRF_GRA },		  //  12 High colour background
	{ "bw00b",             0x080000, 0xb0a48225, BRF_GRA },		  //  13 High colour background

	{ "bw000",             0x040000, 0xd8fe869d, BRF_SND },		  //  14 Samples
};


STD_ROM_PICK(Berlwall)
STD_ROM_FN(Berlwall)

static struct BurnRomInfo BerlwalltRomDesc[] = {
	{ "bw100a",            0x020000, 0xe6bcb4eb, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "bw101a",            0x020000, 0x38056fb2, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "bw001",             0x080000, 0xbc927260, BRF_GRA },		  //  2	Sprites
	{ "bw002",             0x080000, 0x223f5465, BRF_GRA },		  //  3	Sprites
	{ "bw300",             0x020000, 0xb258737a, BRF_GRA },		  //  4	Sprites
	
	{ "bw003",             0x080000, 0xfbb4b72d, BRF_GRA },		  //  5 Tiles
	
	{ "bw004",             0x080000, 0x5300c34d, BRF_GRA },		  //  6 High colour background
	{ "bw008",             0x080000, 0x9aaf2f2f, BRF_GRA },		  //  7 High colour background
	{ "bw005",             0x080000, 0x16db6d43, BRF_GRA },		  //  8 High colour background
	{ "bw009",             0x080000, 0x1151a0b0, BRF_GRA },		  //  9 High colour background
	{ "bw006",             0x080000, 0x73a35d1f, BRF_GRA },		  //  10 High colour background
	{ "bw00a",             0x080000, 0xf447dfc2, BRF_GRA },		  //  11 High colour background
	{ "bw007",             0x080000, 0x97f85c87, BRF_GRA },		  //  12 High colour background
	{ "bw00b",             0x080000, 0xb0a48225, BRF_GRA },		  //  13 High colour background

	{ "bw000",             0x040000, 0xd8fe869d, BRF_SND },		  //  14 Samples
};


STD_ROM_PICK(Berlwallt)
STD_ROM_FN(Berlwallt)

static struct BurnRomInfo BlazeonRomDesc[] = {
	{ "bz_prg1.u80",       0x040000, 0x8409e31d, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "bz_prg2.u81",       0x040000, 0xb8a0a08b, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "bz_sp1.u20",        0x100000, 0x0d5809a1, BRF_GRA },		  //  2	Sprites
	{ "bz_sp2.u21",        0x100000, 0x56ead2bd, BRF_GRA },		  //  3	Sprites
	
	{ "bz_bg.u2",          0x100000, 0xfc67f19f, BRF_GRA },		  //  4 Tiles
	
	{ "3.u45",             0x020000, 0x52fe4c94, BRF_ESS | BRF_PRG }, //  5 Z80 Program Code
};


STD_ROM_PICK(Blazeon)
STD_ROM_FN(Blazeon)

static struct BurnRomInfo BloodwarRomDesc[] = {
	{ "ofp0f3.514",        0x080000, 0x0c93da15, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ofp1f3.513",        0x080000, 0x894ecbe5, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "ofd0x3.124",        0x020000, 0x399f2005, BRF_PRG | BRF_OPT }, //  2	MCU Code

	{ "of-200-0201.8",     0x200000, 0xbba63025, BRF_GRA },		  //  3	Sprites
	{ "of-201-0202.9",     0x200000, 0x4ffd9ddc, BRF_GRA },		  //  4	Sprites
	{ "of-202-0203.10",    0x200000, 0xfbcc5363, BRF_GRA },		  //  5	Sprites
	{ "of-203-0204.11",    0x200000, 0x8e818ce9, BRF_GRA },		  //  6	Sprites
	{ "of-204-0205.12",    0x200000, 0x70c4a76b, BRF_GRA },		  //  7	Sprites
	{ "of-205-0206.13",    0x200000, 0x80c667bb, BRF_GRA },		  //  8	Sprites
	{ "of-206-0207.14",    0x200000, 0xc2028c97, BRF_GRA },		  //  9	Sprites
	{ "of-207-0208.15",    0x200000, 0xb1f30c61, BRF_GRA },		  //  10 Sprites
	{ "of-208-0209.28",    0x200000, 0xa8f29545, BRF_GRA },		  //  11 Sprites
	{ "of-209e-0210.16",   0x100000, 0x93018468, BRF_GRA },		  //  12 Sprites
	{ "of-2090-2011.17",   0x100000, 0x3fb226a1, BRF_GRA },		  //  13 Sprites
	{ "of-210e-0212.18",   0x100000, 0x80f3fa1b, BRF_GRA },		  //  14 Sprites
	{ "of-2100-0213.19",   0x100000, 0x8ca3a3d6, BRF_GRA },		  //  15 Sprites
	{ "of-211e-0214.20",   0x100000, 0x8d3d96f7, BRF_GRA },		  //  16 Sprites
	{ "of-2110-0215.21",   0x100000, 0x78268230, BRF_GRA },		  //  17 Sprites
	{ "of-212e-0216.22",   0x100000, 0x5a013d99, BRF_GRA },		  //  18 Sprites
	{ "of-2120-0217.23",   0x100000, 0x84ed25bd, BRF_GRA },		  //  19 Sprites
	{ "of-213e-0218.24",   0x100000, 0x861bc5b1, BRF_GRA },		  //  20 Sprites
	{ "of-2130-0219.25",   0x100000, 0xa79b8119, BRF_GRA },		  //  21 Sprites
	{ "of-214e-0220.26",   0x100000, 0x43c622de, BRF_GRA },		  //  22 Sprites
	{ "of-2140-0221.27",   0x100000, 0xd10bf03c, BRF_GRA },		  //  23 Sprites
	
	{ "of-300-0225.51",    0x100000, 0xfbc3c08a, BRF_GRA },		  //  24 Tiles
	{ "of-301-0226.55",    0x100000, 0xfcf215de, BRF_GRA },		  //  25 Tiles (Layers 2 & 3)

	{ "of-101-f-0223.101", 0x100000, 0x295f3c93, BRF_SND },		  //  26 Samples
	{ "of-100-0222.99",    0x100000, 0x42b12269, BRF_SND },		  //  27 Samples
};


STD_ROM_PICK(Bloodwar)
STD_ROM_FN(Bloodwar)

static struct BurnRomInfo BonkadvRomDesc[] = {
	{ "prg.8",             0x080000, 0xaf2e60f8, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "prg.7",             0x080000, 0xa1cc6a78, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "mcu.124",           0x020000, 0x9d4e2724, BRF_PRG | BRF_OPT }, //  2	MCU Code

	{ "pc100101.37",       0x200000, 0xc96e7c10, BRF_GRA },		  //  3	Sprites
	{ "pc200102.40",       0x100000, 0xc2b7a26a, BRF_GRA },		  //  4	Sprites
	{ "pc300103.38",       0x100000, 0x51ee162c, BRF_GRA },		  //  5	Sprites
	{ "pc600106.42",       0x080000, 0x25877026, BRF_GRA },		  //  6	Sprites
	{ "pc700107.43",       0x080000, 0xbfe21c44, BRF_GRA },		  //  7	Sprites
	
	{ "pc400104.51",       0x100000, 0x3b176f84, BRF_GRA },		  //  8 Tiles
	{ "pc500105.55",       0x100000, 0xbebb3edc, BRF_GRA },		  //  9 Tiles (Layers 2 & 3)

	{ "pc604109.101",      0x100000, 0x76025530, BRF_SND },		  //  10 Samples
	{ "pc601106.99",       0x100000, 0xa893651c, BRF_SND },		  //  11 Samples
	{ "pc602107.100",      0x100000, 0x0fbb23aa, BRF_SND },		  //  12 Samples
	{ "pc603108.102",      0x100000, 0x58458985, BRF_SND },		  //  13 Samples
};


STD_ROM_PICK(Bonkadv)
STD_ROM_FN(Bonkadv)

static struct BurnRomInfo ExplbrkrRomDesc[] = {
	{ "ts100e.u18",        0x040000, 0xcc84a985, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ts101e.u19",        0x040000, 0x88f4afb7, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "ts001e.u37",        0x080000, 0x70b66e7e, BRF_GRA },		  //  2	Sprites
	{ "ts000e.u38",        0x080000, 0xa7a94143, BRF_GRA },		  //  3	Sprites
	{ "ts002e.u36",        0x040000, 0x611271e6, BRF_GRA },		  //  4	Sprites
	
	{ "ts010.u4",          0x100000, 0xdf935324, BRF_GRA },		  //  5	Tiles
	{ "ts020.u33",         0x100000, 0xeb58c35d, BRF_GRA },		  //  6 Tiles (Layers 2 & 3)

	{ "ts030.u5",          0x100000, 0x1d68e9d1, BRF_SND },		  //  7 Samples
};


STD_ROM_PICK(Explbrkr)
STD_ROM_FN(Explbrkr)

static struct BurnRomInfo BakubrkrRomDesc[] = {
	{ "ts100j.u18",        0x040000, 0x8cc0a4fd, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ts101j.u19",        0x040000, 0xaea92195, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "ts001j.u37",        0x080000, 0x70b66e7e, BRF_GRA },		  //  2	Sprites
	{ "ts000j.u38",        0x080000, 0xa7a94143, BRF_GRA },		  //  3	Sprites
	{ "ts002j.u36",        0x040000, 0x611271e6, BRF_GRA },		  //  4	Sprites
	
	{ "ts010.u4",          0x100000, 0xdf935324, BRF_GRA },		  //  5	Tiles
	{ "ts020.u33",         0x100000, 0xeb58c35d, BRF_GRA },		  //  6 Tiles (Layers 2 & 3)

	{ "ts030.u5",          0x100000, 0x1d68e9d1, BRF_SND },		  //  7 Samples
};


STD_ROM_PICK(Bakubrkr)
STD_ROM_FN(Bakubrkr)

static struct BurnRomInfo GtmrRomDesc[] = {
	{ "u2.bin",            0x080000, 0x031799f7, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "u1.bin",            0x080000, 0x6238790a, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "mmd0x2.u124.bin",   0x020000, 0x3d7cb329, BRF_PRG | BRF_OPT }, //  2	MCU Code

	{ "mm-200-402-s0.bin", 0x200000, 0xc0ab3efc, BRF_GRA },		  //  3	Sprites
	{ "mm-201-403-s1.bin", 0x200000, 0xcf6b23dc, BRF_GRA },	 	  //  4	Sprites
	{ "mm-202-404-s2.bin", 0x200000, 0x8f27f5d3, BRF_GRA },		  //  5	Sprites
	{ "mm-203-405-s3.bin", 0x080000, 0xe9747c8c, BRF_GRA },		  //  6	Sprites
	{ "mms1x2.u30.bin",    0x020000, 0xb42b426f, BRF_GRA },		  //  7	Sprites
	{ "mms0x2.u29.bin",    0x020000, 0xbd22b7d2, BRF_GRA },		  //  8	Sprites
	
	{ "mm-300-406-a0.bin", 0x200000, 0xb15f6b7f, BRF_GRA },		  //  9	Tiles

	{ "mm-100-401-e0.bin", 0x100000, 0xb9cbfbee, BRF_SND },		  //  10 Samples
};


STD_ROM_PICK(Gtmr)
STD_ROM_FN(Gtmr)

static struct BurnRomInfo GtmraRomDesc[] = {
	{ "mmp0x2.u514.bin",   0x080000, 0xba4a77c8, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "mmp1x2.u513.bin",   0x080000, 0xa2b9034e, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "mmd0x2.u124.bin",   0x020000, 0x3d7cb329, BRF_PRG | BRF_OPT }, //  2	MCU Code

	{ "mm-200-402-s0.bin", 0x200000, 0xc0ab3efc, BRF_GRA },		  //  3	Sprites
	{ "mm-201-403-s1.bin", 0x200000, 0xcf6b23dc, BRF_GRA },	 	  //  4	Sprites
	{ "mm-202-404-s2.bin", 0x200000, 0x8f27f5d3, BRF_GRA },		  //  5	Sprites
	{ "mm-203-405-s3.bin", 0x080000, 0xe9747c8c, BRF_GRA },		  //  6	Sprites
	{ "mms1x2.u30.bin",    0x020000, 0xb42b426f, BRF_GRA },		  //  7	Sprites
	{ "mms0x2.u29.bin",    0x020000, 0xbd22b7d2, BRF_GRA },		  //  8	Sprites
	
	{ "mm-300-406-a0.bin", 0x200000, 0xb15f6b7f, BRF_GRA },		  //  9	Tiles

	{ "mm-100-401-e0.bin", 0x100000, 0xb9cbfbee, BRF_SND },		  //  10 Samples
};


STD_ROM_PICK(Gtmra)
STD_ROM_FN(Gtmra)

static struct BurnRomInfo GtmreRomDesc[] = {
	{ "gmmu2.bin",         0x080000, 0x36dc4aa9, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "gmmu1.bin",         0x080000, 0x8653c144, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "gtmrusa.u12",       0x020000, 0x2e1a06ff, BRF_PRG | BRF_OPT }, //  2	MCU Code

	{ "gmmu27.bin",        0x200000, 0xc0ab3efc, BRF_GRA },		  //  3	Sprites
	{ "gmmu28.bin",        0x200000, 0xcf6b23dc, BRF_GRA },	 	  //  4	Sprites
	{ "gmmu29.bin",        0x200000, 0x8f27f5d3, BRF_GRA },		  //  5	Sprites
	{ "gmmu30.bin",        0x080000, 0xe9747c8c, BRF_GRA },		  //  6	Sprites
	{ "gmmu64.bin",        0x100000, 0x57d77b33, BRF_GRA },		  //  7	Sprites
	{ "gmmu65.bin",        0x100000, 0x05b8bdca, BRF_GRA },		  //  8	Sprites
	
	{ "gmmu52.bin",        0x200000, 0xb15f6b7f, BRF_GRA },		  //  9	Tiles

	{ "gmmu23.bin",        0x100000, 0xb9cbfbee, BRF_SND },		  //  10 Samples
	{ "gmmu24.bin",        0x100000, 0x380cdc7c, BRF_SND },		  //  11 Samples
};


STD_ROM_PICK(Gtmre)
STD_ROM_FN(Gtmre)

static struct BurnRomInfo GtmrusaRomDesc[] = {
	{ "gtmrusa.u2",        0x080000, 0x5be615c4, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "gtmrusa.u1",        0x080000, 0xae853e4e, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "gtmrusa.u12",       0x020000, 0x2e1a06ff, BRF_PRG | BRF_OPT }, //  2	MCU Code

	{ "gmmu27.bin",        0x200000, 0xc0ab3efc, BRF_GRA },		  //  3	Sprites
	{ "gmmu28.bin",        0x200000, 0xcf6b23dc, BRF_GRA },	 	  //  4	Sprites
	{ "gmmu29.bin",        0x200000, 0x8f27f5d3, BRF_GRA },		  //  5	Sprites
	{ "gmmu30.bin",        0x080000, 0xe9747c8c, BRF_GRA },		  //  6	Sprites
	{ "gmmu64.bin",        0x100000, 0x57d77b33, BRF_GRA },		  //  7	Sprites
	{ "gmmu65.bin",        0x100000, 0x05b8bdca, BRF_GRA },		  //  8	Sprites
	
	{ "gmmu52.bin",        0x200000, 0xb15f6b7f, BRF_GRA },		  //  9	Tiles

	{ "gmmu23.bin",        0x100000, 0xb9cbfbee, BRF_SND },		  //  10 Samples
	{ "gmmu24.bin",        0x100000, 0x380cdc7c, BRF_SND },		  //  11 Samples
};


STD_ROM_PICK(Gtmrusa)
STD_ROM_FN(Gtmrusa)

static struct BurnRomInfo Gtmr2RomDesc[] = {
	{ "m2p0x1a.u8",        0x080000, 0xc29039fb, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "m2p1x1a.u7",        0x080000, 0x8ef392c4, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "m2d0x0.u31",        0x020000, 0x2e1a06ff, BRF_PRG | BRF_OPT }, //  2	MCU Code

	{ "m2-200-0.u49",      0x400000, 0x93aafc53, BRF_GRA },		  //  3	Sprites
	{ "m2-201-0.u50",      0x200000, 0x39b60a83, BRF_GRA },	 	  //  4	Sprites
	{ "m2-202-0.u51",      0x200000, 0xfd06b339, BRF_GRA },		  //  5	Sprites
	{ "m2s0x1a.u32",       0x080000, 0xa485eec6, BRF_GRA },		  //  6	Sprites
	{ "m2s1x1a.u33",       0x080000, 0xc5b71bb2, BRF_GRA },		  //  7	Sprites
	
	{ "m2-300-0.u89",      0x200000, 0x4dc42fbb, BRF_GRA },		  //  8	Tiles
	{ "m2-301-0.u90",      0x200000, 0xf4e894f2, BRF_GRA },		  //  9	Tiles
	{ "m2b0x0.u93",        0x020000, 0xe023d51b, BRF_GRA },		  //  10 Tiles
	{ "m2b1x0.u94",        0x020000, 0x03c48bdb, BRF_GRA },		  //  11 Tiles

	{ "m2-100-0.u48",      0x100000, 0x5250fa45, BRF_SND },		  //  12 Samples
	{ "m2w1x0.u47",        0x040000, 0x1b0513c5, BRF_SND },		  //  13 Samples
};


STD_ROM_PICK(Gtmr2)
STD_ROM_FN(Gtmr2)

static struct BurnRomInfo Gtmr2aRomDesc[] = {
	{ "m2p0x1.u8",         0x080000, 0x525f6618, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "m2p1x1.u7",         0x080000, 0x914683e5, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "m2d0x0.u31",        0x020000, 0x2e1a06ff, BRF_PRG | BRF_OPT }, //  2	MCU Code

	{ "m2-200-0.u49",      0x400000, 0x93aafc53, BRF_GRA },		  //  3	Sprites
	{ "m2-201-0.u50",      0x200000, 0x39b60a83, BRF_GRA },	 	  //  4	Sprites
	{ "m2-202-0.u51",      0x200000, 0xfd06b339, BRF_GRA },		  //  5	Sprites
	{ "m2s0x1.u32",        0x080000, 0x4069d6c7, BRF_GRA },		  //  6	Sprites
	{ "m2s1x1.u33",        0x080000, 0xc53fe269, BRF_GRA },		  //  7	Sprites
	
	{ "m2-300-0.u89",      0x200000, 0x4dc42fbb, BRF_GRA },		  //  8	Tiles
	{ "m2-301-0.u90",      0x200000, 0xf4e894f2, BRF_GRA },		  //  9	Tiles
	{ "m2b0x0.u93",        0x020000, 0xe023d51b, BRF_GRA },		  //  10 Tiles
	{ "m2b1x0.u94",        0x020000, 0x03c48bdb, BRF_GRA },		  //  11 Tiles

	{ "m2-100-0.u48",      0x100000, 0x5250fa45, BRF_SND },		  //  12 Samples
	{ "m2w1x0.u47",        0x040000, 0x1b0513c5, BRF_SND },		  //  13 Samples
};


STD_ROM_PICK(Gtmr2a)
STD_ROM_FN(Gtmr2a)

static struct BurnRomInfo Gtmr2uRomDesc[] = {
	{ "m2p0a1.u8",         0x080000, 0x813e1d5e, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "m2p1a1.u7",         0x080000, 0xbee63666, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "m2d0x0.u31",        0x020000, 0x2e1a06ff, BRF_PRG | BRF_OPT }, //  2	MCU Code

	{ "m2-200-0.u49",      0x400000, 0x93aafc53, BRF_GRA },		  //  3	Sprites
	{ "m2-201-0.u50",      0x200000, 0x39b60a83, BRF_GRA },	 	  //  4	Sprites
	{ "m2-202-0.u51",      0x200000, 0xfd06b339, BRF_GRA },		  //  5	Sprites
	{ "m2s0a1.u32",        0x080000, 0x98977171, BRF_GRA },		  //  6	Sprites
	{ "m2s1a1.u33",        0x080000, 0xc69a732e, BRF_GRA },		  //  7	Sprites
	
	{ "m2-300-0.u89",      0x200000, 0x4dc42fbb, BRF_GRA },		  //  8	Tiles
	{ "m2-301-0.u90",      0x200000, 0xf4e894f2, BRF_GRA },		  //  9	Tiles
	{ "m2b0x0.u93",        0x020000, 0xe023d51b, BRF_GRA },		  //  10 Tiles
	{ "m2b1x0.u94",        0x020000, 0x03c48bdb, BRF_GRA },		  //  11 Tiles

	{ "m2-100-0.u48",      0x100000, 0x5250fa45, BRF_SND },		  //  12 Samples
	{ "m2w1a1.u47",        0x080000, 0x15f25342, BRF_SND },		  //  13 Samples
};


STD_ROM_PICK(Gtmr2u)
STD_ROM_FN(Gtmr2u)

static struct BurnRomInfo MgcrystlRomDesc[] = {
	{ "mc100e02.u18",      0x020000, 0x246a1335, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "mc101e02.u19",      0x040000, 0x708ea1dc, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "mc000.u38",         0x100000, 0x28acf6f4, BRF_GRA },		  //  2	Sprites
	{ "mc001.u37",         0x080000, 0x005bc43d, BRF_GRA },		  //  3	Sprites
	{ "mc002e02.u36",      0x020000, 0x27ac1056, BRF_GRA },		  //  4	Sprites
	
	{ "mc010.u04",         0x100000, 0x85072772, BRF_GRA },		  //  5	Tiles
	{ "mc020.u34",         0x100000, 0x1ea92ff1, BRF_GRA },		  //  6 Tiles (Layers 2 & 3)

	{ "mc030.u32",         0x040000, 0xc165962e, BRF_SND },		  //  7 Samples
};


STD_ROM_PICK(Mgcrystl)
STD_ROM_FN(Mgcrystl)

static struct BurnRomInfo MgcrystloRomDesc[] = {
	{ "mc100h00.u18",      0x020000, 0xc7456ba7, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "mc101h00.u19",      0x040000, 0xea8f9300, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "mc000.u38",         0x100000, 0x28acf6f4, BRF_GRA },		  //  2	Sprites
	{ "mc001.u37",         0x080000, 0x005bc43d, BRF_GRA },		  //  3	Sprites
	{ "mc002h00.u36",      0x020000, 0x22729037, BRF_GRA },		  //  4	Sprites
	
	{ "mc010.u04",         0x100000, 0x85072772, BRF_GRA },		  //  5	Tiles
	{ "mc020.u34",         0x100000, 0x1ea92ff1, BRF_GRA },		  //  6 Tiles (Layers 2 & 3)

	{ "mc030.u32",         0x040000, 0xc165962e, BRF_SND },		  //  7 Samples
};


STD_ROM_PICK(Mgcrystlo)
STD_ROM_FN(Mgcrystlo)

static struct BurnRomInfo MgcrystljRomDesc[] = {
	{ "mc100j02.u18",      0x020000, 0xafe5882d, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "mc101j02.u19",      0x040000, 0x60da5492, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	
	{ "mc000.u38",         0x100000, 0x28acf6f4, BRF_GRA },		  //  2	Sprites
	{ "mc001.u37",         0x080000, 0x005bc43d, BRF_GRA },		  //  3	Sprites
	{ "mc002e02.u36",      0x020000, 0x27ac1056, BRF_GRA },		  //  4	Sprites
	
	{ "mc010.u04",         0x100000, 0x85072772, BRF_GRA },		  //  5	Tiles
	{ "mc020.u34",         0x100000, 0x1ea92ff1, BRF_GRA },		  //  6 Tiles (Layers 2 & 3)

	{ "mc030.u32",         0x040000, 0xc165962e, BRF_SND },		  //  7 Samples
};


STD_ROM_PICK(Mgcrystlj)
STD_ROM_FN(Mgcrystlj)

/*==============================================================================================
Graphics Decoding
===============================================================================================*/

static void Kaneko16DecodeBg15Bitmaps()
{
	INT32 sx, x, y;
	
	for (sx = 0; sx < 32; sx++) {
		for (x = 0; x < 256; x++) {
			for (y = 0; y < 256; y++) {
				INT32 addr = (sx * 256 * 256) + (y * 256) + x;
				INT32 data = (Kaneko16TempGfx[addr * 2 + 0] * 256) + Kaneko16TempGfx[addr * 2 + 1];
				INT32 r, g, b;
				
				r = (data & 0x07c0) >> 6;
				g = (data & 0xf800) >> 11;
				b = (data & 0x003e) >> 1;
				
				r ^= 0x09;
				
				if (~g & 0x08) g ^= 0x010;
				g = (g - 1) & 0x1f;
				
				b ^= 0x03;
				if (~b & 0x08) b ^= 0x10;
				b = (b + 2) & 0x1f;
				
				if ((r & 0x10) && (b & 0x10)) g = (g - 1) & 0x1f;
				
				Kaneko16Bg15Data[addr] = 2048 + ((g << 10) | (r << 5) | b);
			}
		}
	}
}

/*==============================================================================================
Unscramble Tile/Sound ROM Functions
===============================================================================================*/

static void UnscrambleTiles(INT32 length)
{
	UINT8 *RAM = Kaneko16TempGfx;
	INT32 i;

	if (RAM == NULL) return;

	for (i = 0; i < length; i ++)
	{
		RAM[i] = ((RAM[i] & 0xF0)>>4) + ((RAM[i] & 0x0F)<<4);
	}
}

static void ExpandSampleBanks()
{
	INT32 bank;

	for (bank = 15; bank > 0; bank--)
	{
		UINT8 *src0 = MSM6295ROMData;
		UINT8 *srcn = src0 + 0x10000 * (bank < 3 ? 3 : bank);
		UINT8 *dst = src0 + 0x40000 * bank;

		memcpy(dst + 0x30000, srcn + 0x00000, 0x10000);
		memcpy(dst + 0x00000, src0 + 0x00000, 0x30000);
	}
}

/*==============================================================================================
Allocate Memory
===============================================================================================*/

static INT32 GtmrMemIndex()
{
	UINT8 *Next; Next = Mem;

	Kaneko16Rom           = Next; Next += 0x100000;
	MSM6295ROM            = Next; Next += 0x140000;
	MSM6295ROMData        = Next; Next += 0x400000;
	MSM6295ROMData2       = Next; Next += 0x300000;

	RamStart = Next;

	Kaneko16Ram           = Next; Next += 0x028000;
	Kaneko16MCURam        = Next; Next += 0x010000;
	Kaneko16NVRam         = Next; Next += 0x000100;
	Kaneko16PaletteRam    = Next; Next += 0x010000;
	Kaneko16SpriteRam     = Next; Next += Kaneko16SpriteRamSize;
	Kaneko16Video0Ram     = Next; Next += 0x001000;
	Kaneko16Video1Ram     = Next; Next += 0x001000;
	Kaneko16Video2Ram     = Next; Next += 0x001000;
	Kaneko16Video3Ram     = Next; Next += 0x001000;
	Kaneko16VScrl0Ram     = Next; Next += 0x001000;
	Kaneko16VScrl1Ram     = Next; Next += 0x001000;
	Kaneko16VScrl2Ram     = Next; Next += 0x001000;
	Kaneko16VScrl3Ram     = Next; Next += 0x001000;

	RamEnd = Next;

	Kaneko16Sprites       = Next; Next += (Kaneko16NumSprites * 16 * 16);
	Kaneko16Tiles         = Next; Next += (Kaneko16NumTiles * 16 * 16);
	LayerQueueXY[0]       = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueueXY[1]       = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueueColour[0]   = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueueColour[1]   = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueuePriority[0] = Next; Next += nScreenWidth * nScreenHeight;
	LayerQueuePriority[1] = Next; Next += nScreenWidth * nScreenHeight;
	if (Kaneko16NumTiles2) {
		Kaneko16Tiles2        = Next; Next += (Kaneko16NumTiles2 * 16 * 16);
		LayerQueueXY[2]       = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
		LayerQueueXY[3]       = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
		LayerQueueColour[2]   = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
		LayerQueueColour[3]   = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
		LayerQueuePriority[2] = Next; Next += nScreenWidth * nScreenHeight;
		LayerQueuePriority[3] = Next; Next += nScreenWidth * nScreenHeight;
	}
	Kaneko16Palette       = (UINT32*)Next; Next += 0x010000 * sizeof(UINT32);
	MemEnd = Next;

	return 0;
}

static INT32 ExplbrkrMemIndex()
{
	UINT8 *Next; Next = Mem;

	Kaneko16Rom           = Next; Next += 0x080000;
	MSM6295ROM            = Next; Next += 0x040000;
	MSM6295ROMData        = Next; Next += 0x200000;

	RamStart = Next;

	Kaneko16Ram           = Next; Next += 0x010000;
	Kaneko16PaletteRam    = Next; Next += 0x001000;
	Kaneko16SpriteRam     = Next; Next += Kaneko16SpriteRamSize;
	Kaneko16Video0Ram     = Next; Next += 0x001000;
	Kaneko16Video1Ram     = Next; Next += 0x001000;
	Kaneko16Video2Ram     = Next; Next += 0x001000;
	Kaneko16Video3Ram     = Next; Next += 0x001000;
	Kaneko16VScrl0Ram     = Next; Next += 0x001000;
	Kaneko16VScrl1Ram     = Next; Next += 0x001000;
	Kaneko16VScrl2Ram     = Next; Next += 0x001000;
	Kaneko16VScrl3Ram     = Next; Next += 0x001000;

	RamEnd = Next;

	Kaneko16Sprites       = Next; Next += (Kaneko16NumSprites * 16 * 16);
	Kaneko16Tiles         = Next; Next += (Kaneko16NumTiles * 16 * 16);
	LayerQueueXY[0]       = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueueXY[1]       = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueueColour[0]   = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueueColour[1]   = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueuePriority[0] = Next; Next += nScreenWidth * nScreenHeight;
	LayerQueuePriority[1] = Next; Next += nScreenWidth * nScreenHeight;
	if (Kaneko16NumTiles2) {
		Kaneko16Tiles2        = Next; Next += (Kaneko16NumTiles2 * 16 * 16);
		LayerQueueXY[2]       = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
		LayerQueueXY[3]       = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
		LayerQueueColour[2]   = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
		LayerQueueColour[3]   = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
		LayerQueuePriority[2] = Next; Next += nScreenWidth * nScreenHeight;
		LayerQueuePriority[3] = Next; Next += nScreenWidth * nScreenHeight;
	}
	pFMBuffer             = (INT16*)Next; Next += nBurnSoundLen * 6 * sizeof(INT16);
	if (Kaneko16Bg15) {
		Kaneko16Bg15Data     = (UINT16*)Next; Next += (32 * 256 * 256) * sizeof(UINT16); // 32 bitmaps - 256 x 256
		Kaneko16Palette      = (UINT32*)Next; Next += (0x001000 + 32768) * sizeof(UINT32);
	} else {
		Kaneko16Palette      = (UINT32*)Next; Next += 0x001000 * sizeof(UINT32);
	}
	MemEnd = Next;

	return 0;
}

static INT32 BlazeonMemIndex()
{
	UINT8 *Next; Next = Mem;

	Kaneko16Rom           = Next; Next += 0x080000;
	Kaneko16Z80Rom        = Next; Next += 0x020000;

	RamStart = Next;

	Kaneko16Ram           = Next; Next += 0x010000;
	Kaneko16Z80Ram        = Next; Next += 0x002000;
	Kaneko16PaletteRam    = Next; Next += 0x001000;
	Kaneko16SpriteRam     = Next; Next += Kaneko16SpriteRamSize;
	Kaneko16Video0Ram     = Next; Next += 0x001000;
	Kaneko16Video1Ram     = Next; Next += 0x001000;
	Kaneko16VScrl0Ram     = Next; Next += 0x001000;
	Kaneko16VScrl1Ram     = Next; Next += 0x001000;

	RamEnd = Next;

	Kaneko16Sprites       = Next; Next += (Kaneko16NumSprites * 16 * 16);
	Kaneko16Tiles         = Next; Next += (Kaneko16NumTiles * 16 * 16);
	LayerQueueXY[0]       = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueueXY[1]       = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueueColour[0]   = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueueColour[1]   = (UINT32*)Next; Next += nScreenWidth * nScreenHeight * sizeof(UINT32);
	LayerQueuePriority[0] = Next; Next += nScreenWidth * nScreenHeight;
	LayerQueuePriority[1] = Next; Next += nScreenWidth * nScreenHeight;
	Kaneko16Palette       = (UINT32*)Next; Next += 0x001000 * sizeof(UINT32);

	MemEnd = Next;

	return 0;
}

/*==============================================================================================
Protection Calculator
===============================================================================================*/

static struct {
	UINT16 x1p, y1p, x1s, y1s;
	UINT16 x2p, y2p, x2s, y2s;

	INT16 x12, y12, x21, y21;

	UINT16 mult_a, mult_b;
} hit;

static INT16 calc_compute_x(void)
{
	INT16 x_coll;

	// X distance
	if ((hit.x2p >= hit.x1p) && (hit.x2p < (hit.x1p + hit.x1s)))		// x2p inside x1
		x_coll = (hit.x1s - (hit.x2p - hit.x1p));
	else if ((hit.x1p >= hit.x2p) && (hit.x1p < (hit.x2p + hit.x2s)))	// x1p inside x2
		x_coll = (hit.x2s - (hit.x1p - hit.x2p));
	else																// normal/no overlap
	 	x_coll = ((hit.x1s + hit.x2s)/2) - abs((hit.x1p + hit.x1s/2) - (hit.x2p + hit.x2s/2));

	return x_coll;
}
static INT16 calc_compute_y(void)
{
	INT16 y_coll;

	// Y distance
	if ((hit.y2p >= hit.y1p) && (hit.y2p < (hit.y1p + hit.y1s)))		// y2p inside y1
		y_coll = (hit.y1s - (hit.y2p - hit.y1p));
	else if ((hit.y1p >= hit.y2p) && (hit.y1p < (hit.y2p + hit.y2s)))	// y1p inside y2
		y_coll = (hit.y2s - (hit.y1p - hit.y2p));
	else																// normal/no overlap
		y_coll = ((hit.y1s + hit.y2s)/2) - abs((hit.y1p + hit.y1s/2) - (hit.y2p + hit.y2s/2));

	return y_coll;
}

static UINT16 BloodwarCalcRead(INT32 offset)
{
	UINT16 data = 0;
	INT16 x_coll, y_coll;

	x_coll = calc_compute_x();
	y_coll = calc_compute_y();

	switch (offset)
	{
		case 0x00/2: // X distance
			return x_coll;

		case 0x02/2: // Y distance
			return y_coll;

		case 0x04/2: // similar to the hit detection from SuperNova, but much simpler

			// 4th nibble: Y Absolute Collision -> possible values = 9,8,4,3,2
			if      (hit.y1p >  hit.y2p)	data |= 0x2000;
			else if (hit.y1p == hit.y2p)	data |= 0x4000;
			else if (hit.y1p <  hit.y2p)	data |= 0x8000;
			if (y_coll<0) data |= 0x1000;

			// 3rd nibble: X Absolute Collision -> possible values = 9,8,4,3,2
			if      (hit.x1p >  hit.x2p)	data |= 0x0200;
			else if (hit.x1p == hit.x2p)	data |= 0x0400;
			else if (hit.x1p <  hit.x2p)	data |= 0x0800;
			if (x_coll<0) data |= 0x0100;

			// 2nd nibble: always set to 4
			data |= 0x0040;

			// 1st nibble: XY Overlap Collision -> possible values = 0,2,4,f
			if (x_coll>=0) data |= 0x0004;
			if (y_coll>=0) data |= 0x0002;
			if ((x_coll>=0)&&(y_coll>=0)) data |= 0x000F;

			return data;

		case 0x14/2:
			return rand() & 0xffff;

		case 0x20/2: return hit.x1p;
		case 0x22/2: return hit.x1s;
		case 0x24/2: return hit.y1p;
		case 0x26/2: return hit.y1s;

		case 0x2c/2: return hit.x2p;
		case 0x2e/2: return hit.x2s;
		case 0x30/2: return hit.y2p;
		case 0x32/2: return hit.y2s;
	}

	return 0;
}

static UINT16 BonkadvCalcRead(INT32 offset)
{
	UINT16 data = 0;

	switch (offset)
	{
		case 0x00/2: // watchdog
			return 0;

		case 0x02/2: // unknown (yet!), used by *MANY* games !!!
			break;

		case 0x04/2: // similar to the hit detection from SuperNova, but much simpler

			// X Absolute Collision
			if      (hit.x1p >  hit.x2p)	data |= 0x0200;
			else if (hit.x1p == hit.x2p)	data |= 0x0400;
			else if (hit.x1p <  hit.x2p)	data |= 0x0800;

			// Y Absolute Collision
			if      (hit.y1p >  hit.y2p)	data |= 0x2000;
			else if (hit.y1p == hit.y2p)	data |= 0x4000;
			else if (hit.y1p <  hit.y2p)	data |= 0x8000;

			// XY Overlap Collision
			hit.x12 = (hit.x1p) - (hit.x2p + hit.x2s);
			hit.y12 = (hit.y1p) - (hit.y2p + hit.y2s);
			hit.x21 = (hit.x1p + hit.x1s) - (hit.x2p);
			hit.y21 = (hit.y1p + hit.y1s) - (hit.y2p);

			if ((hit.x12 < 0) && (hit.y12 < 0) &&
				(hit.x21 >= 0) && (hit.y21 >= 0))
					data |= 0x0001;

			return data;

		case 0x10/2:
			return (((UINT32)hit.mult_a * (UINT32)hit.mult_b) >> 16);
		case 0x12/2:
			return (((UINT32)hit.mult_a * (UINT32)hit.mult_b) & 0xffff);

		case 0x14/2:
			return rand() & 0xffff;
	}

	return 0;
}

static void BloodwarCalcWrite(INT32 offset, UINT16 data)
{
	switch (offset) {
		case 0x20 >> 1: hit.x1p = data; return;
		case 0x22 >> 1: hit.x1s = data; return;
		case 0x24 >> 1: hit.y1p = data; return;
		case 0x26 >> 1: hit.y1s = data; return;
		
		case 0x2c >> 1: hit.x2p = data; return;
		case 0x2e >> 1: hit.x2s = data; return;
		case 0x30 >> 1: hit.y2p = data; return;
		case 0x32 >> 1: hit.y2s = data; return;
	}
}

static void BonkadvCalcWrite(INT32 offset, UINT16 data)
{
	switch (offset) {
		case 0x00 >> 1: hit.x1p = data; return;
		case 0x02 >> 1: hit.x1s = data; return;
		case 0x04 >> 1: hit.y1p = data; return;
		case 0x06 >> 1: hit.y1s = data; return;
		case 0x08 >> 1: hit.x2p = data; return;
		case 0x0a >> 1: hit.x2s = data; return;
		case 0x0c >> 1: hit.y2p = data; return;
		case 0x0e >> 1: hit.y2s = data; return;
		case 0x10 >> 1: hit.mult_a = data; return;
		case 0x12 >> 1: hit.mult_b = data; return;
	}
}

/*==============================================================================================
Toybox MCU
===============================================================================================*/

static void ToyboxMCUInit()
{
	memset(ToyboxMCUCom, 0, 4 * sizeof(UINT16));
}

static UINT16 ToyboxMCUStatusRead()
{
	return 0;
}

#define MCU_RESPONSE(d) memcpy(&MCURam[MCUOffset], d, sizeof(d))

static void BloodwarMCURun()
{
	UINT16 *MCURam = (UINT16*)Kaneko16MCURam;
	UINT16 *NVRam = (UINT16*)Kaneko16NVRam;

	UINT16 MCUCommand = MCURam[0x10/2];
	UINT16 MCUOffset = MCURam[0x12/2] >> 1;
	UINT16 MCUData = MCURam[0x14/2];
	
	switch (MCUCommand >> 8) {
		case 0x02: {
			memcpy(MCURam + MCUOffset, NVRam, 128);
			return;
		}
		
		case 0x03: {
			MCURam[MCUOffset + 0] = 0xff00 - (Kaneko16Dip[0] << 8);
			return;
		}
		
		case 0x04: {
			switch (MCUData) {
				// unknown purpose data
				case 0x01: MCU_RESPONSE(bloodwar_mcu_4_01); break; // Warrior 1
				case 0x02: MCU_RESPONSE(bloodwar_mcu_4_02); break; // Warrior 2
				case 0x03: MCU_RESPONSE(bloodwar_mcu_4_03); break; // Warrior 3
				case 0x04: MCU_RESPONSE(bloodwar_mcu_4_04); break; // Warrior 4
				case 0x05: MCU_RESPONSE(bloodwar_mcu_4_05); break; // Warrior 5
				case 0x06: MCU_RESPONSE(bloodwar_mcu_4_06); break; // Warrior 6
				case 0x07: MCU_RESPONSE(bloodwar_mcu_4_07); break; // Warrior 7
				case 0x08: MCU_RESPONSE(bloodwar_mcu_4_08); break; // Warrior 8
				case 0x09: MCU_RESPONSE(bloodwar_mcu_4_09); break; // Warrior 9

				// palette data
				case 0x0a: MCU_RESPONSE(bloodwar_mcu_4_0a); break; // Warrior 1 Player 1
				case 0x0b: MCU_RESPONSE(bloodwar_mcu_4_0b); break; // Warrior 1 Player 2
				case 0x0c: MCU_RESPONSE(bloodwar_mcu_4_0c); break; // Warrior 5 Player 1
				case 0x0d: MCU_RESPONSE(bloodwar_mcu_4_0d); break; // Warrior 5 Player 2
				case 0x0e: MCU_RESPONSE(bloodwar_mcu_4_0e); break; // Warrior 4 Player 2
				case 0x0f: MCU_RESPONSE(bloodwar_mcu_4_0f); break; // Warrior 4 Player 1
				case 0x10: MCU_RESPONSE(bloodwar_mcu_4_10); break; // Warrior 6 Player 1
				case 0x11: MCU_RESPONSE(bloodwar_mcu_4_11); break; // Warrior 6 Player 2
				case 0x12: MCU_RESPONSE(bloodwar_mcu_4_12); break; // Warrior 9 Player 1
				case 0x13: MCU_RESPONSE(bloodwar_mcu_4_13); break; // Warrior 9 Player 2
				case 0x14: MCU_RESPONSE(bloodwar_mcu_4_14); break; // Warrior 7 Player 1
				case 0x15: MCU_RESPONSE(bloodwar_mcu_4_15); break; // Warrior 7 Player 2
				case 0x16: MCU_RESPONSE(bloodwar_mcu_4_16); break; // Warrior 8 Player 1
				case 0x17: MCU_RESPONSE(bloodwar_mcu_4_17); break; // Warrior 8 Player 2
				case 0x18: MCU_RESPONSE(bloodwar_mcu_4_18); break; // Warrior 2 Player 2
				case 0x19: MCU_RESPONSE(bloodwar_mcu_4_19); break; // Warrior 2 Player 1
				case 0x1a: MCU_RESPONSE(bloodwar_mcu_4_1a); break; // Warrior 3 Player 1
				case 0x1b: MCU_RESPONSE(bloodwar_mcu_4_1b); break; // Warrior 3 Player 2

				// tilemap data
				case 0x1c: MCU_RESPONSE(bloodwar_mcu_4_1c); break; // Warrior 8
				case 0x1d: MCU_RESPONSE(bloodwar_mcu_4_1d); break; // Warrior 2
				case 0x1e: MCU_RESPONSE(bloodwar_mcu_4_1e); break; // Warrior 3
				case 0x1f: MCU_RESPONSE(bloodwar_mcu_4_1f); break; // Warrior 5
				case 0x20: MCU_RESPONSE(bloodwar_mcu_4_20); break; // Warrior 4
				case 0x21: MCU_RESPONSE(bloodwar_mcu_4_21); break; // Warrior 6
				case 0x22: MCU_RESPONSE(bloodwar_mcu_4_22); break; // Warrior 1
				case 0x23: MCU_RESPONSE(bloodwar_mcu_4_23); break; // Warrior 9
				case 0x24: MCU_RESPONSE(bloodwar_mcu_4_24); break; // Warrior 7

				// fighter data: pointers to ROM data
				case 0x25: MCU_RESPONSE(bloodwar_mcu_4_25); break; // Warrior 1
				case 0x26: MCU_RESPONSE(bloodwar_mcu_4_26); break; // Warrior 2
				case 0x27: MCU_RESPONSE(bloodwar_mcu_4_27); break; // Warrior 3
				case 0x28: MCU_RESPONSE(bloodwar_mcu_4_28); break; // Warrior 4
				case 0x29: MCU_RESPONSE(bloodwar_mcu_4_29); break; // Warrior 5
				case 0x2a: MCU_RESPONSE(bloodwar_mcu_4_2a); break; // Warrior 6
				case 0x2b: MCU_RESPONSE(bloodwar_mcu_4_2b); break; // Warrior 7
				case 0x2c: MCU_RESPONSE(bloodwar_mcu_4_2c); break; // Warrior 8
				case 0x2d: MCU_RESPONSE(bloodwar_mcu_4_2d); break; // Warrior 9
			}
			return;
		}
		
		case 0x42: {
			memcpy(NVRam, MCURam + MCUOffset, 128);
			return;
		}
	}
}

static void BonkadvMCURun()
{
	UINT16 *MCURam = (UINT16*)Kaneko16MCURam;
	UINT16 *NVRam = (UINT16*)Kaneko16NVRam;

	UINT16 MCUCommand = MCURam[0x10/2];
	UINT16 MCUOffset = MCURam[0x12/2] >> 1;
	UINT16 MCUData = MCURam[0x14/2];
	
	switch (MCUCommand >> 8) {
		case 0x02: {
			memcpy(MCURam + MCUOffset, NVRam, 128);
			return;
		}
		
		case 0x03: {
			MCURam[MCUOffset + 0] = 0xff00 - (Kaneko16Dip[0] << 8);
			return;
		}
		
		case 0x04: {
			switch (MCUData) {
				// static, in this order, at boot/reset
				case 0x34: MCU_RESPONSE(bonkadv_mcu_4_34); break;
				case 0x30: MCU_RESPONSE(bonkadv_mcu_4_30); break;
				case 0x31: MCU_RESPONSE(bonkadv_mcu_4_31); break;
				case 0x32: MCU_RESPONSE(bonkadv_mcu_4_32); break;
				case 0x33: MCU_RESPONSE(bonkadv_mcu_4_33); break;

				// dynamic, per-level (29), in level order
				case 0x00: MCU_RESPONSE(bonkadv_mcu_4_00); break;
				case 0x02: MCU_RESPONSE(bonkadv_mcu_4_02); break;
				case 0x01: MCU_RESPONSE(bonkadv_mcu_4_01); break;
				case 0x05: MCU_RESPONSE(bonkadv_mcu_4_05); break;
				case 0x07: MCU_RESPONSE(bonkadv_mcu_4_07); break;
				case 0x06: MCU_RESPONSE(bonkadv_mcu_4_06); break;
				case 0x09: MCU_RESPONSE(bonkadv_mcu_4_09); break;
				case 0x0D: MCU_RESPONSE(bonkadv_mcu_4_0D); break;
				case 0x03: MCU_RESPONSE(bonkadv_mcu_4_03); break;
				case 0x08: MCU_RESPONSE(bonkadv_mcu_4_08); break;
				case 0x04: MCU_RESPONSE(bonkadv_mcu_4_04); break;
				case 0x0C: MCU_RESPONSE(bonkadv_mcu_4_0C); break;
				case 0x0A: MCU_RESPONSE(bonkadv_mcu_4_0A); break;
				case 0x0B: MCU_RESPONSE(bonkadv_mcu_4_0B); break;
				case 0x10: MCU_RESPONSE(bonkadv_mcu_4_10); break;
				case 0x0E: MCU_RESPONSE(bonkadv_mcu_4_0E); break;
				case 0x13: MCU_RESPONSE(bonkadv_mcu_4_13); break;
				case 0x0F: MCU_RESPONSE(bonkadv_mcu_4_0F); break;
				case 0x11: MCU_RESPONSE(bonkadv_mcu_4_11); break;
				case 0x14: MCU_RESPONSE(bonkadv_mcu_4_14); break;
				case 0x12: MCU_RESPONSE(bonkadv_mcu_4_12); break;
				case 0x17: MCU_RESPONSE(bonkadv_mcu_4_17); break;
				case 0x1A: MCU_RESPONSE(bonkadv_mcu_4_1A); break;
				case 0x15: MCU_RESPONSE(bonkadv_mcu_4_15); break;
				case 0x18: MCU_RESPONSE(bonkadv_mcu_4_18); break;
				case 0x16: MCU_RESPONSE(bonkadv_mcu_4_16); break;
				case 0x19: MCU_RESPONSE(bonkadv_mcu_4_19); break;
				case 0x1B: MCU_RESPONSE(bonkadv_mcu_4_1B); break;
				case 0x1C: MCU_RESPONSE(bonkadv_mcu_4_1C); break;
			}
			return;
		}
		
		case 0x42: {
			memcpy(NVRam, MCURam + MCUOffset, 128);
			return;
		}
		
		case 0x43: {
			// Reset defaults
			memcpy(NVRam, bonkadv_mcu_43, sizeof(bonkadv_mcu_43));
			return;
		}
	}	
}

#undef MCU_RESPONSE

static void GtmrMCURun()
{
	UINT16 *MCURam = (UINT16*)Kaneko16MCURam;
	UINT16 *NVRam = (UINT16*)Kaneko16NVRam;

	UINT16 MCUCommand = MCURam[0x10/2];
	UINT16 MCUOffset = MCURam[0x12/2] >> 1;
	
	switch (MCUCommand >> 8) {
		case 0x02: {
			memcpy(MCURam + MCUOffset, NVRam, 128);
			return;
		}
		
		case 0x03: {
			MCURam[MCUOffset + 0] = 0xff00 - (Kaneko16Dip[0] << 8);
			return;
		}
		
		case 0x04: {
			/* MCU writes the string "MM0525-TOYBOX199" to shared ram */
			MCURam[MCUOffset + 0] = 0x4d4d;
			MCURam[MCUOffset + 1] = 0x3035;
			MCURam[MCUOffset + 2] = 0x3235;
			MCURam[MCUOffset + 3] = 0x2d54;
			MCURam[MCUOffset + 4] = 0x4f59;
			MCURam[MCUOffset + 5] = 0x424f;
			MCURam[MCUOffset + 6] = 0x5831;
			MCURam[MCUOffset + 7] = 0x3939;
			return;
		}
		
		case 0x42: {
			memcpy(NVRam, MCURam + MCUOffset, 128);
			return;
		}
	}
}

static void GtmrevoMCURun()
{
	UINT16 *MCURam = (UINT16*)Kaneko16MCURam;
	UINT16 *NVRam = (UINT16*)Kaneko16NVRam;

	UINT16 MCUCommand = MCURam[0x10/2];
	UINT16 MCUOffset = MCURam[0x12/2] >> 1;
	
	switch (MCUCommand >> 8) {
		case 0x02: {
			memcpy(MCURam + MCUOffset, NVRam, 128);
			return;
		}
		
		case 0x03: {
			MCURam[MCUOffset + 0] = 0xff00 - (Kaneko16Dip[0] << 8);
			return;
		}
		
		case 0x04: {
			/* MCU writes the string "USMM0713-TB1994 " to shared ram */
			MCURam[MCUOffset + 0] = 0x5553;
			MCURam[MCUOffset + 1] = 0x4d4d;
			MCURam[MCUOffset + 2] = 0x3037;
			MCURam[MCUOffset + 3] = 0x3133;
			MCURam[MCUOffset + 4] = 0x2d54;
			MCURam[MCUOffset + 5] = 0x4231;
			MCURam[MCUOffset + 6] = 0x3939;
			MCURam[MCUOffset + 7] = 0x3420;
			return;
		}
		
		case 0x42: {
			memcpy(NVRam, MCURam + MCUOffset, 128);
			return;
		}
	}
}

static void ToyboxMCUComWrite(INT32 which, UINT16 data)
{
	ToyboxMCUCom[which] = data;
	if (ToyboxMCUCom[0] != 0xffff) return;
	if (ToyboxMCUCom[1] != 0xffff) return;
	if (ToyboxMCUCom[2] != 0xffff) return;
	if (ToyboxMCUCom[3] != 0xffff) return;
	
	memset(ToyboxMCUCom, 0, 4 * sizeof(UINT16));
	ToyboxMCURun();
}

/*==============================================================================================
Reset Functions
===============================================================================================*/

static INT32 Kaneko16DoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();
	
	Kaneko16SpriteFlipX = 0;
	Kaneko16SpriteFlipY = 0;
	Kaneko16DisplayEnable = 0;
	memset(Kaneko16SpriteRegs, 0, 0x20 * sizeof(UINT16));
	memset(Kaneko16Layer0Regs, 0, 0x10 * sizeof(UINT16));
	memset(Kaneko16Layer1Regs, 0, 0x10 * sizeof(UINT16));

	return 0;
}

static INT32 BerlwallDoReset()
{
	INT32 nRet = Kaneko16DoReset();
	
	for (INT32 i = 0; i < 2; i++) {
		AY8910Reset(i);
	}
	
	MSM6295Reset(0);
	MSM6295Bank0 = 0;
	
	Kaneko16Bg15Reg = 0;
	Kaneko16Bg15Select = 0;

	return nRet;
}

static INT32 BlazeonDoReset()
{
	INT32 nRet = Kaneko16DoReset();
	
	ZetOpen(0);
	ZetReset();
	ZetClose();
	
	BurnYM2151Reset();
	
	Kaneko16SoundLatch = 0;
	
	return nRet;
}

static INT32 ExplbrkrDoReset()
{
	INT32 nRet = Kaneko16DoReset();
	
	for (INT32 i = 0; i < 2; i++) {
		AY8910Reset(i);
	}
	
	EEPROMReset();
	
	MSM6295Reset(0);
	MSM6295Bank0 = 0;

	return nRet;
}

static INT32 GtmrDoReset()
{
	INT32 nRet = Kaneko16DoReset();
	
	MSM6295Reset(0);
	MSM6295Reset(1);
	MSM6295Bank0 = 0;
	MSM6295Bank1 = 0;
	
	ToyboxMCUInit();
	
	return nRet;
}

/*==============================================================================================
Memory Handlers
===============================================================================================*/

UINT8 __fastcall BerlwallReadByte(UINT32 a)
{
	switch (a) {
		case 0x680000: {
			return 0xff - Kaneko16Input[0];
		}
		
		case 0x680002: {
			return 0xff - Kaneko16Input[1];
		}
		
		case 0x680004: {
			return 0xff - Kaneko16Input[2];
		}
		
		case 0x800001: {
			AY8910Write(0, 0, (a - 0x800000) >> 1);
			return AY8910Read(0);
		}
		
		case 0x800401: {
			return MSM6295ReadStatus(0);
		}
	
//		default: {
//			bprintf(PRINT_NORMAL, _T("Read byte -> %06X\n"), a);
//		}
	}
	
	return 0;
}

void __fastcall BerlwallWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x700000: {
			// Coin lockout
			return;
		}
		
		case 0x800010:
		case 0x800012:
		case 0x800014: {
			AY8910Write(0, 0, (a - 0x800000) >> 1);
			AY8910Write(0, 1, d & 0xff);
			return;
		}
		
		case 0x800210:
		case 0x800212:
		case 0x800214: {
			AY8910Write(1, 0, (a - 0x800200) >> 1);
			AY8910Write(1, 1, d & 0xff);
			return;
		}

		
		case 0x800401: {
			MSM6295Command(0, d & 0xff);
			return;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Write byte -> %06X, %02X\n"), a, d);
//		}
	}
}

UINT16 __fastcall BerlwallReadWord(UINT32 a)
{
	switch (a) {
		case 0x780000: {
			// watchdog reset
			return 0;
		}
		
		case 0x80000e:
		case 0x80001c:
		case 0x80001e: {
			AY8910Write(0, 0, (a - 0x800000) >> 1);
			return AY8910Read(0);
		}
		
		case 0x80020e: {
			AY8910Write(1, 0, (a - 0x800200) >> 1);
			return AY8910Read(1);
		}
		
		case 0x8003fe: {
			// NOP
			return 0;
		}
		
		case 0x800400: {
			return MSM6295ReadStatus(0);
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Read Word -> %06X\n"), a);
//		}
	}
	
	return 0;
}

void __fastcall BerlwallWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x480000: {
			// ??
			return;
		}
		
		case 0x500000: {
			Kaneko16Bg15Reg = d;
			return;
		}
		
		case 0x580000: {
			Kaneko16Bg15Select = d;
			return;
		}
	
		case 0x800000:
		case 0x800002:
		case 0x800004:
		case 0x800006:
		case 0x800008:
		case 0x80000a:
		case 0x80000c:
		case 0x80000e:
		case 0x800010:
		case 0x800012:
		case 0x800014:
		case 0x800016:
		case 0x800018:
		case 0x80001a: {
			AY8910Write(0, 0, (a - 0x800000) >> 1);
			AY8910Write(0, 1, d & 0xff);
			return;
		}
		
		case 0x800200:
		case 0x800202:
		case 0x800204:
		case 0x800206:
		case 0x800208:
		case 0x80020a:
		case 0x80020c:
		case 0x80020e:
		case 0x800210:
		case 0x800212:
		case 0x800214:
		case 0x800216:
		case 0x800218:
		case 0x80021a: {
			AY8910Write(1, 0, (a - 0x800200) >> 1);
			AY8910Write(1, 1, d & 0xff);
			return;
		}
		
		case 0x8003fe: {
			// NOP
			return;
		}
		
		case 0x800400: {
			MSM6295Command(0, d & 0xff);
			return;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Write word -> %06X, %04X\n"), a, d);
//		}
	}
}

UINT8 __fastcall BlazeonReadByte(UINT32 a)
{
	switch (a) {
		case 0xc00000: {
			return 0xff - Kaneko16Input[0];
		}
		
		case 0xc00001: {
			return Kaneko16Dip[0];
		}
		
		case 0xc00002: {
			return 0xff - Kaneko16Input[1];
		}
		
		case 0xc00003: {
			return Kaneko16Dip[1];
		}
		
		case 0xc00006: {
			return 0xff - Kaneko16Input[2];
		}
		
		case 0xc00007: {
			return 0xff;
		}
		
		case 0xec0000: {
			// NOP
			return 0;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Read byte -> %06X\n"), a);
//		}
	}
	
	return 0;
}

void __fastcall BlazeonWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xd00000: {
			// Coin lockout
			return;
		}
		
		case 0xe00000: {
			Kaneko16SoundLatch = d;
			ZetOpen(0);
			ZetNmi();
			ZetClose();
			return;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Write byte -> %06X, %02X\n"), a, d);
//		}
	}
}

UINT16 __fastcall BlazeonReadWord(UINT32 a)
{
	switch (a) {
		case 0xc00000: {
			SEK_DEF_READ_WORD(0, a);
			return 0;
		}
		
		case 0xc00002: {
			SEK_DEF_READ_WORD(0, a);
			return 0;
		}
		
		case 0xc00004: {
			return 0xffff;
		}
		
		case 0xc00006: {
			SEK_DEF_READ_WORD(0, a);
			return 0;
		}
	
//		default: {
//			bprintf(PRINT_NORMAL, _T("Read Word -> %06X\n"), a);
//		}
	}
	
	return 0;
}

void __fastcall BlazeonWriteWord(UINT32 a, UINT16 /*d*/)
{
	switch (a) {
		case 0xd00000: {
			// Coin lockout
			return;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Write word -> %06X, %04X\n"), a, d);
//		}
	}
}

UINT8 __fastcall ExplbrkrReadByte(UINT32 a)
{
	switch (a) {
		case 0x400001: {
			AY8910Write(0, 0, (a - 0x400000) >> 1);
			return AY8910Read(0);
		}
	
		case 0x40021d: {
			AY8910Write(1, 0, (a - 0x400200) >> 1);
			return AY8910Read(1);
		}
		
		case 0x400401: {
			return MSM6295ReadStatus(0);
		}
		
		case 0xe00000: {
			return 0xff - Kaneko16Input[0];
		}
		
		case 0xe00001: {
			return Kaneko16Dip[0];
		}
		
		case 0xe00002: {
			return 0xff - Kaneko16Input[1];
		}
		
		case 0xe00004: {
			return 0xff - Kaneko16Input[2];
		}
		
		case 0xe00006: {
			return 0xff;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Read byte -> %06X\n"), a);
//		}
	}
	
	return 0;
}

void __fastcall ExplbrkrWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x40000f:
		case 0x400010:
		case 0x400012:
		case 0x400014: {
			AY8910Write(0, 0, (a - 0x400000) >> 1);
			AY8910Write(0, 1, d & 0xff);
			return;
		}
		
		case 0x40020f:
		case 0x400210:
		case 0x400212:
		case 0x400214:
		case 0x40021f: {
			AY8910Write(1, 0, (a - 0x400200) >> 1);
			AY8910Write(1, 1, d & 0xff);
			return;
		}
		
		case 0x400401: {
			MSM6295Command(0, d & 0xff);
			return;
		}
		
		case 0xd00000: return;
		
		case 0xd00001: {
			EEPROMWriteBit(d & 0x02);
			EEPROMSetClockLine((d & 0x01) ? EEPROM_ASSERT_LINE : EEPROM_CLEAR_LINE );
			return;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Write byte -> %06X, %02X\n"), a, d);
//		}
	}
}

UINT16 __fastcall ExplbrkrReadWord(UINT32 a)
{
	switch (a) {
		case 0xa00000: {
			// Magical crystals Watchdog reset
			return 0;
		}
		
		case 0xa80000: {
			// Watchdog reset
			return 0;
		}
	
//		default: {
//			bprintf(PRINT_NORMAL, _T("Read Word -> %06X\n"), a);
//		}
	}
	
	return 0;
}

void __fastcall ExplbrkrWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x400000:
		case 0x400002:
		case 0x400004:
		case 0x400006:
		case 0x400008:
		case 0x40000a:
		case 0x40000c:
		case 0x40000e:
		case 0x400010:
		case 0x400012:
		case 0x400014:
		case 0x400016:
		case 0x400018:
		case 0x40001a:
		case 0x40001c: {
			AY8910Write(0, 0, (a - 0x400000) >> 1);
			AY8910Write(0, 1, d & 0xff);
			return;
		}
		
		case 0x40001e: {
			if (Mgcrystl) {
				AY8910Write(0, 0, (a - 0x400000) >> 1);
				AY8910Write(0, 1, d & 0xff);
			} else {
				MSM6295Bank0 = d & 7;
				memcpy(MSM6295ROM + 0x0000000, MSM6295ROMData + (0x40000 * (d & 7)), 0x40000);
			}
			return;
		}
		
		case 0x400200:
		case 0x400202:
		case 0x400204:
		case 0x400206:
		case 0x400208:
		case 0x40020a:
		case 0x40020c:
		case 0x40020e:
		case 0x400210:
		case 0x400212:
		case 0x400214:
		case 0x400216:
		case 0x400218:
		case 0x40021a:
		case 0x40021c:
		case 0x40021e: {
			AY8910Write(1, 0, (a - 0x400200) >> 1);
			AY8910Write(1, 1, d & 0xff);
			return;
		}
		
		case 0x900000: {
			Kaneko16SpriteRegs[0] = d;
			if (d & 0xff) {
				Kaneko16SpriteFlipX = d & 2;
				Kaneko16SpriteFlipY = d & 1;
			}
			return;
		}
		
		case 0xd00000: {
			SEK_DEF_WRITE_WORD(0, a, d);
			return;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Write word -> %06X, %04X\n"), a, d);
//		}
	}
}

UINT8 __fastcall MgcrystlReadByte(UINT32 a)
{
	switch (a) {
		case 0x400001: {
			AY8910Write(0, 0, (a - 0x400000) >> 1);
			return AY8910Read(0);
		}
		
		case 0x40021d: {
			AY8910Write(1, 0, (a - 0x400200) >> 1);
			return AY8910Read(1);
		}
		
		case 0x400401: {
			return MSM6295ReadStatus(0);
		}
		
		case 0xc00000: {
			return 0xff - Kaneko16Input[0];
		}
		
		case 0xc00001: {
			return Kaneko16Dip[0];
		}
		
		case 0xc00002: {
			return 0x3f - Kaneko16Input[1];
		}
		
		case 0xc00004: {
			return 0xff - Kaneko16Input[2];
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Read byte -> %06X\n"), a);
//		}
	}
	
	return 0;
}

UINT8 __fastcall GtmrReadByte(UINT32 a)
{
	switch (a) {
		case 0x900014: {
			if (Gtmr) return rand() & 0xff;
			return 0;
		}
		
		case 0x900015: {
			if (Bonkadv) return BonkadvCalcRead((a - 0x900000) >> 1);
			return 0;
		}
		
		case 0xb00000: {
			return 0xff - Kaneko16Input[0];
		}
		
		case 0xb00002: {
			return 0xff - Kaneko16Input[1];
		}
		
		case 0xb00004: {
			return 0xff - Kaneko16Input[2];
		}
		
		case 0xb00006: {
			return 0xff - Kaneko16Input[3];
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Read byte -> %06X\n"), a);
//		}
	}
	
	return 0;
}

void __fastcall GtmrWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x600000:
		case 0x600002:
		case 0x600004:
		case 0x600006:
		case 0x600008:
		case 0x600009:
		case 0x60000a:
		case 0x60000b:
		case 0x60000c:
		case 0x60000e:
		case 0x600010:
		case 0x600012:
		case 0x600014:
		case 0x600016:
		case 0x600018:
		case 0x60001a:
		case 0x60001c:
		case 0x60001e: {
			Kaneko16Layer0Regs[(a - 0x600000) >> 1] = d;
			return;
		}
		
		case 0x680000:
		case 0x680002:
		case 0x680004:
		case 0x680006:
		case 0x680008:
		case 0x680009:
		case 0x68000a:
		case 0x68000b:
		case 0x68000c:
		case 0x68000e:
		case 0x680010:
		case 0x680012:
		case 0x680014:
		case 0x680016:
		case 0x680018:
		case 0x68001a:
		case 0x68001c:
		case 0x68001e: {
			Kaneko16Layer1Regs[(a - 0x680000) >> 1] = d;
			return;
		}
		
		case 0x700000:
		case 0x700002:
		case 0x700004:
		case 0x700006:
		case 0x700008:
		case 0x70000a:
		case 0x70000c:
		case 0x70000e:
		case 0x700010:
		case 0x700012:
		case 0x700014:
		case 0x700016:
		case 0x700018:
		case 0x70001a:
		case 0x70001c:
		case 0x70001e: {
			UINT32 offset = (a - 0x700000) >> 1;
			Kaneko16SpriteRegs[offset] = d;
			if (offset == 0 && (d & 0xff)) {
				Kaneko16SpriteFlipX = d & 2;
				Kaneko16SpriteFlipY = d & 1;
			}
			return;
		}
		
		case 0x800001: {
			MSM6295Command(0, d);
			return;
		}
		
		case 0x880001: {
			MSM6295Command(1, d);
			return;
		}
		
		case 0xb80000: {
			// Coin lockout
			return;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Write byte -> %06X, %02X\n"), a, d);
//		}
	}
}

UINT16 __fastcall GtmrReadWord(UINT32 a)
{
	switch (a) {
		case 0x800000: {
			return MSM6295ReadStatus(0);
		}
		
		case 0x880000: {
			return MSM6295ReadStatus(1);
		}
		
		case 0x900000:
		case 0x900002:
		case 0x900004:
		case 0x900006:
		case 0x900008:
		case 0x90000a:
		case 0x90000c:
		case 0x90000e:
		case 0x900010:
		case 0x900012:
		case 0x900014:
		case 0x900016:
		case 0x900018:
		case 0x90001a:
		case 0x90001c:
		case 0x90001e:
		case 0x900020:
		case 0x900022:
		case 0x900024:
		case 0x900026:
		case 0x900028:
		case 0x90002a:
		case 0x90002c:
		case 0x90002e:
		case 0x900030:
		case 0x900032:
		case 0x900034:
		case 0x900036:
		case 0x900038: {
			if (Bloodwar) return BloodwarCalcRead((a - 0x900000) >> 1);
			if (Bonkadv) return BonkadvCalcRead((a - 0x900000) >> 1);
			if (Gtmr) return rand() & 0xffff;
			return 0;
		}
		
		case 0xa00000: {
			// Watchdog read
			return 0;
		}
		
		case 0xb00000: {
			return 0xff00 - (Kaneko16Input[0] << 8);
		}
		
		case 0xb00002: {
			return 0xff00 - (Kaneko16Input[1] << 8);
		}
		
		case 0xb00004: {
			return 0xff00 - (Kaneko16Input[2] << 8);
		}
		
		case 0xb00006: {
			return 0xffff;
		}
		
		case 0xd00000: {
			return ToyboxMCUStatusRead();
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Read Word -> %06X\n"), a);
//		}
	}
	
	return 0;
}

void __fastcall GtmrWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x2a0000: {
			ToyboxMCUComWrite(0, d);
			return;
		}
		
		case 0x2b0000: {
			ToyboxMCUComWrite(1, d);
			return;
		}
		
		case 0x2c0000: {
			ToyboxMCUComWrite(2, d);
			return;
		}
		
		case 0x2d0000: {
			ToyboxMCUComWrite(3, d);
			return;
		}
		
		case 0x700000: {
			Kaneko16SpriteRegs[0] = d;
			if (d & 0xff) {
				Kaneko16SpriteFlipX = d & 2;
				Kaneko16SpriteFlipY = d & 1;
			}
			return;
		}
		
		case 0x800000: {
			MSM6295Command(0, d);
			return;
		}
		
		case 0x880000: {
			MSM6295Command(1, d);
			return;
		}
		
		case 0x900000:
		case 0x900002:
		case 0x900004:
		case 0x900006:
		case 0x900008:
		case 0x90000a:
		case 0x90000c:
		case 0x90000e:
		case 0x900010:
		case 0x900012:
		case 0x900014:
		case 0x900016:
		case 0x900018:
		case 0x90001a:
		case 0x90001c:
		case 0x90001e:
		case 0x900020:
		case 0x900022:
		case 0x900024:
		case 0x900026:
		case 0x900028:
		case 0x90002a:
		case 0x90002c:
		case 0x90002e:
		case 0x900030:
		case 0x900032:
		case 0x900034:
		case 0x900036:
		case 0x900038: {
			if (Bonkadv) BonkadvCalcWrite((a - 0x900000) >> 1, d);
			if (Bloodwar) BloodwarCalcWrite((a - 0x900000) >> 1, d);
			return;
		}
		
		case 0xa00000: {
			// Watchdog write
			return;
		}
		
		case 0xb80000: {
			// Coin lockout
			return;
		}
		
		case 0xc00000: {
			Kaneko16DisplayEnable = d;
			return;
		}
		
		case 0xe00000: {
			MSM6295Bank0 = d & 0xf;
			memcpy(MSM6295ROM + 0x0000000, MSM6295ROMData + (0x40000 * (d & 0x0f)),0x40000);
			return;
		}
		
		case 0xe80000: {
			if (Gtmr) {
				MSM6295Bank1 = d & 1;
				memcpy(MSM6295ROM + 0x0100000, MSM6295ROMData2 + (0x40000 * (d & 0x01)),0x40000);
			} else {
				MSM6295Bank1 = d;
				memcpy(MSM6295ROM + 0x0100000, MSM6295ROMData2 + (0x40000 * d),0x40000);
			}
			return;
		}
		
//		default: {
//			bprintf(PRINT_NORMAL, _T("Write word -> %06X, %04X\n"), a, d);
//		}
	}
}

UINT8 __fastcall Kaneko16Z80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x03: {
			return BurnYM2151ReadStatus();
		}
		
		case 0x06: {
			return Kaneko16SoundLatch;
		}
	}

	return 0;
}

void __fastcall Kaneko16Z80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x02: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0x03: {
			BurnYM2151WriteRegister(d);
			return;
		}
	}
}

/*==============================================================================================
Sound Chip Handlers
===============================================================================================*/

static UINT8 Kaneko16EepromRead(UINT32 /*a*/)
{
	return EEPROMRead() & 0x01;
}

static void Kaneko16EepromReset(UINT32 /*a*/, UINT32 d)
{
	EEPROMSetCSLine((d & 0x01) ? EEPROM_CLEAR_LINE : EEPROM_ASSERT_LINE );
}

static UINT8 Kaneko16Dip0Read(UINT32 /*a*/)
{
	return Kaneko16Dip[0];
}

static UINT8 Kaneko16Dip1Read(UINT32 /*a*/)
{
	return Kaneko16Dip[1];
}

/*==============================================================================================
Driver Inits
===============================================================================================*/

static void Kaneko16VideoInit()
{
	GenericTilesInit();
	
	spritelist.first_sprite = (struct tempsprite *)BurnMalloc(0x400 * sizeof(spritelist.first_sprite[0]));
	
	Kaneko16ParseSprite = Kaneko16ParseSpriteType0;
	
	Kaneko16SpritesColourOffset = 0;
	Kaneko16SpritesColourMask = 0x0fff;
	Kaneko168BppSprites = 0;
	if (Gtmr || Bloodwar || Bonkadv) {
		Kaneko16SpritesColourOffset = 0x4000;
		Kaneko16SpritesColourMask = 0xffff;
		Kaneko168BppSprites = 1;
	}
	
	Kaneko16SpriteXOffset = 0;
	Kaneko16SpriteRamSize = 0x2000;
	
	Kaneko16LayersColourOffset = 0x40 * 16;
	if (Gtmr || Bloodwar || Bonkadv) Kaneko16LayersColourOffset = 0;
	Kaneko16TilesXOffset = 0;
	if (nScreenWidth == 256) Kaneko16TilesXOffset = 0x5b;
	if (nScreenWidth == 320) Kaneko16TilesXOffset = 0x33;
	Kaneko16TilesYOffset = 0;
	if (nScreenHeight == 224 || nScreenHeight == 232) Kaneko16TilesYOffset = -0x08;

	Kaneko16Bg15 = 0;
	Kaneko16Bg15Reg = 0;
	Kaneko16Bg15Select = 0;
	Kaneko16RecalcBg15Palette = 0;
}

static INT32 GtmrMachineInit()
{
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Kaneko16Rom          , 0x000000, 0x0ffffd, SM_ROM);
	SekMapMemory(Kaneko16Ram          , 0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(Kaneko16MCURam       , 0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(Kaneko16PaletteRam   , 0x300000, 0x30ffff, SM_RAM);
	SekMapMemory(Kaneko16Ram + 0x10000, 0x310000, 0x327fff, SM_RAM);
	SekMapMemory(Kaneko16SpriteRam    , 0x400000, 0x401fff, SM_RAM);
	SekMapMemory(Kaneko16Video1Ram    , 0x500000, 0x500fff, SM_RAM);
	SekMapMemory(Kaneko16Video0Ram    , 0x501000, 0x501fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl1Ram    , 0x502000, 0x502fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl0Ram    , 0x503000, 0x503fff, SM_RAM);
	SekMapMemory(Kaneko16Video3Ram    , 0x580000, 0x580fff, SM_RAM);
	SekMapMemory(Kaneko16Video2Ram    , 0x581000, 0x581fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl3Ram    , 0x582000, 0x582fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl2Ram    , 0x583000, 0x583fff, SM_RAM);
	SekMapMemory((UINT8*)Kaneko16Layer0Regs    , 0x600000, 0x60001f, SM_WRITE);
	SekMapMemory((UINT8*)Kaneko16Layer1Regs    , 0x680000, 0x68001f, SM_WRITE);
	SekMapMemory((UINT8*)Kaneko16SpriteRegs + 2, 0x700002, 0x70001f, SM_WRITE);
	SekSetReadByteHandler(0, GtmrReadByte);
	SekSetReadWordHandler(0, GtmrReadWord);
	SekSetWriteByteHandler(0, GtmrWriteByte);
	SekSetWriteWordHandler(0, GtmrWriteWord);
	SekClose();
	
	// Setup the OKIM6295 emulation
	MSM6295Init(0, 1980000 / 165, 0);
	MSM6295Init(1, 1980000 / 165, 0);
	MSM6295SetRoute(0, 0.50, BURN_SND_ROUTE_BOTH);
	MSM6295SetRoute(1, 0.50, BURN_SND_ROUTE_BOTH);

	return 0;
}

static INT32 FourBppPlaneOffsets[4]  = { 0, 1, 2, 3 };
static INT32 FourBppXOffsets[16]     = { 0, 4, 8, 12, 16, 20, 24, 28, 256, 260, 264, 268, 272, 276, 280, 284 };
static INT32 FourBppYOffsets[16]     = { 0, 32, 64, 96, 128, 160, 192, 224, 512, 544, 576, 608, 640, 672, 704, 736 };
static INT32 EightBppPlaneOffsets[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 EightBppXOffsets[16]    = { 0, 8, 16, 24, 32, 40, 48, 56, 512, 520, 528, 536, 544, 552, 560, 568 };
static INT32 EightBppYOffsets[16]    = { 0, 64, 128, 192, 256, 320, 384, 448, 1024, 1088, 1152, 1216, 1280, 1344, 1408, 1472 };

static INT32 BerlwallInit()
{
	INT32 nRet = 0, nLen;
	
	Kaneko16NumSprites = 0x2400;
	Kaneko16NumTiles = 0x1000;
	Kaneko16NumTiles2 = 0;
	
	Kaneko16VideoInit();
	Kaneko16ParseSprite = Kaneko16ParseSpriteType2;
	Kaneko16Bg15 = 1;
	Kaneko16RecalcBg15Palette = 1;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	ExplbrkrMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	ExplbrkrMemIndex();

	Kaneko16TempGfx = (UINT8*)BurnMalloc(0x400000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Kaneko16Rom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16Rom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0000000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0080000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0100000,  4, 1); if (nRet != 0) return 1;
	GfxDecode(Kaneko16NumSprites, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Sprites);
	
	// Load and Decode Tile Roms
	memset(Kaneko16TempGfx, 0, 0x400000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 5, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x080000);
	GfxDecode(Kaneko16NumTiles, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles);
	
	// Load bitmap roms
	memset(Kaneko16TempGfx, 0, 0x400000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000,  6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000001,  7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x100000,  8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x100001,  9, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x200000, 10, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x200001, 11, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x300000, 12, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x300001, 13, 2); if (nRet != 0) return 1;
	Kaneko16DecodeBg15Bitmaps();
	BurnFree(Kaneko16TempGfx);
			
	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 14, 1); if (nRet != 0) return 1;
	memcpy(MSM6295ROMData, MSM6295ROM, 0x40000);
	
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Kaneko16Rom          , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Kaneko16Ram          , 0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(Kaneko16SpriteRam    , 0x30e000, 0x30ffff, SM_RAM);
	SekMapMemory(Kaneko16PaletteRam   , 0x400000, 0x400fff, SM_RAM);
	SekMapMemory(Kaneko16Video1Ram    , 0xc00000, 0xc00fff, SM_RAM);
	SekMapMemory(Kaneko16Video0Ram    , 0xc01000, 0xc01fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl1Ram    , 0xc02000, 0xc02fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl0Ram    , 0xc03000, 0xc03fff, SM_RAM);
	SekMapMemory((UINT8*)Kaneko16Layer0Regs    , 0xd00000, 0xd0001f, SM_WRITE);
	SekMapMemory((UINT8*)Kaneko16SpriteRegs + 2, 0x600002, 0x60003f, SM_WRITE);
	SekSetReadByteHandler(0, BerlwallReadByte);
	SekSetReadWordHandler(0, BerlwallReadWord);
	SekSetWriteByteHandler(0, BerlwallWriteByte);
	SekSetWriteWordHandler(0, BerlwallWriteWord);
	SekClose();
	
	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;

	AY8910Init(0, 2000000, nBurnSoundRate, &Kaneko16Dip0Read, &Kaneko16Dip1Read, NULL, NULL);
	AY8910Init(1, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910SetAllRoutes(0, 1.00, BURN_SND_ROUTE_BOTH);
	AY8910SetAllRoutes(1, 1.00, BURN_SND_ROUTE_BOTH);
	
	// Setup the OKIM6295 emulation
	MSM6295Init(0, (12000000 / 6) / 132, 1);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	
	Kaneko16FrameRender = BerlwallFrameRender;

	// Reset the driver
	BerlwallDoReset();
	
	return 0;
}

static INT32 BlazeonInit()
{
	INT32 nRet = 0, nLen;
	
	Kaneko16NumSprites = 0x4000;
	Kaneko16NumTiles = 0x2000;
	Kaneko16NumTiles2 = 0;
	
	Kaneko16VideoInit();
	Kaneko16SpriteRamSize = 0x1000;
	Kaneko16SpriteXOffset = 0x10000 - 0x680;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	BlazeonMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	BlazeonMemIndex();

	Kaneko16TempGfx = (UINT8*)BurnMalloc(0x200000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Kaneko16Rom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16Rom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0000000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0100000,  3, 1); if (nRet != 0) return 1;
	GfxDecode(Kaneko16NumSprites, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Sprites);
	
	// Load and Decode Tile Roms
	memset(Kaneko16TempGfx, 0, 0x200000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 4, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x100000);
	GfxDecode(Kaneko16NumTiles, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles);
	
	BurnFree(Kaneko16TempGfx);
	
	// Load Z80 Rom
	nRet = BurnLoadRom(Kaneko16Z80Rom, 5, 1); if (nRet != 0) return 1;
	
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Kaneko16Rom          , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Kaneko16Ram          , 0x300000, 0x30ffff, SM_RAM);
	SekMapMemory(Kaneko16PaletteRam   , 0x500000, 0x500fff, SM_RAM);
	SekMapMemory(Kaneko16Video1Ram    , 0x600000, 0x600fff, SM_RAM);
	SekMapMemory(Kaneko16Video0Ram    , 0x601000, 0x601fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl1Ram    , 0x602000, 0x602fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl0Ram    , 0x603000, 0x603fff, SM_RAM);
	SekMapMemory(Kaneko16SpriteRam    , 0x700000, 0x700fff, SM_RAM);
	SekMapMemory((UINT8*)Kaneko16Layer0Regs    , 0x800000, 0x80000f, SM_WRITE);
	SekMapMemory((UINT8*)Kaneko16SpriteRegs + 2, 0x900002, 0x90001f, SM_WRITE);
	SekSetReadByteHandler(0, BlazeonReadByte);
	SekSetReadWordHandler(0, BlazeonReadWord);
	SekSetWriteByteHandler(0, BlazeonWriteByte);
	SekSetWriteWordHandler(0, BlazeonWriteWord);
	SekClose();
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, Kaneko16Z80Rom         );
	ZetMapArea(0x0000, 0x7fff, 2, Kaneko16Z80Rom         );
	ZetMapArea(0xc000, 0xdfff, 0, Kaneko16Z80Ram         );
	ZetMapArea(0xc000, 0xdfff, 1, Kaneko16Z80Ram         );
	ZetMapArea(0xc000, 0xdfff, 2, Kaneko16Z80Ram         );
	ZetMemEnd();
	ZetSetInHandler(Kaneko16Z80PortRead);
	ZetSetOutHandler(Kaneko16Z80PortWrite);
	ZetClose();
	
	// Setup the YM2151 emulation
	BurnYM2151Init(4000000);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);
	
	Kaneko16FrameRender = BlazeonFrameRender;
	
	// Reset the driver
	BlazeonDoReset();
	
	return 0;
}

static INT32 BloodwarInit()
{
	INT32 nRet = 0, nLen;
	
	Bloodwar = 1;
	
	Kaneko16NumSprites = 0x1e000;
	Kaneko16NumTiles = 0x2000;
	Kaneko16NumTiles2 = 0x2000;
	
	Kaneko16VideoInit();
	Kaneko16ParseSprite = Kaneko16ParseSpriteType1;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	GtmrMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	GtmrMemIndex();

	Kaneko16TempGfx = (UINT8*)BurnMalloc(0x1e00000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Kaneko16Rom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16Rom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0000000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0200000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0400000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0600000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0800000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0a00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0c00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0e00000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1000000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1200000, 12, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1200001, 13, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1400000, 14, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1400001, 15, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1600000, 16, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1600001, 17, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1800000, 18, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1800001, 19, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1a00000, 20, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1a00001, 21, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1c00000, 22, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x1c00001, 23, 2); if (nRet != 0) return 1;
	GfxDecode(Kaneko16NumSprites, 8, 16, 16, EightBppPlaneOffsets, EightBppXOffsets, EightBppYOffsets, 0x800, Kaneko16TempGfx, Kaneko16Sprites);
	
	// Load and Decode Tile Roms
	memset(Kaneko16TempGfx, 0, 0x1e00000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 24, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x100000);
	GfxDecode(Kaneko16NumTiles, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles);
	memset(Kaneko16TempGfx, 0, 0x1e00000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 25, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x100000);
	GfxDecode(Kaneko16NumTiles2, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles2);
	BurnFree(Kaneko16TempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROMData, 26, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROMData2, 27, 1); if (nRet != 0) return 1;
	ExpandSampleBanks();
	
	ToyboxMCURun = BloodwarMCURun;
	Kaneko16FrameRender = BloodwarFrameRender;
	
	nRet = GtmrMachineInit(); if (nRet != 0) return 1;
	
	// Reset the driver
	GtmrDoReset();

	return 0;
}

static INT32 BonkadvInit()
{
	INT32 nRet = 0, nLen;
	
	Bonkadv = 1;
	
	Kaneko16NumSprites = 0x5000;
	Kaneko16NumTiles = 0x4000;
	Kaneko16NumTiles2 = 0x4000;
	
	Kaneko16VideoInit();
	Kaneko16ParseSprite = Kaneko16ParseSpriteType1;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	GtmrMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	GtmrMemIndex();

	Kaneko16TempGfx = (UINT8*)BurnMalloc(0x500000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Kaneko16Rom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16Rom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0000000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0200000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0300000, 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0400000, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0400001, 7, 2); if (nRet != 0) return 1;
	GfxDecode(Kaneko16NumSprites, 8, 16, 16, EightBppPlaneOffsets, EightBppXOffsets, EightBppYOffsets, 0x800, Kaneko16TempGfx, Kaneko16Sprites);
	
	// Load and Decode Tile Roms
	memset(Kaneko16TempGfx, 0, 0x500000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 8, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x100000);
	GfxDecode(Kaneko16NumTiles, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles);
	memset(Kaneko16TempGfx, 0, 0x500000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 9, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x100000);
	GfxDecode(Kaneko16NumTiles2, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles2);
	BurnFree(Kaneko16TempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROMData, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROMData2, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROMData2 + 0x100000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROMData2 + 0x200000, 13, 1); if (nRet != 0) return 1;
	ExpandSampleBanks();
	
	ToyboxMCURun = BonkadvMCURun;
	Kaneko16FrameRender = BloodwarFrameRender;
	
	nRet = GtmrMachineInit(); if (nRet != 0) return 1;
	
	// Reset the driver
	GtmrDoReset();

	return 0;
}

static INT32 ExplbrkrInit()
{
	INT32 nRet = 0, nLen;
	
	Kaneko16NumSprites = 0x4800;
	Kaneko16NumTiles = 0x2000;
	Kaneko16NumTiles2 = 0x2000;
	
	Kaneko16VideoInit();
	
	// Allocate and Blank all required memory
	Mem = NULL;
	ExplbrkrMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	ExplbrkrMemIndex();

	Kaneko16TempGfx = (UINT8*)BurnMalloc(0x240000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Kaneko16Rom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16Rom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0000000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0080000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0100000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0180000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0200000,  4, 1); if (nRet != 0) return 1;
	GfxDecode(Kaneko16NumSprites, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Sprites);
	
	// Load and Decode Tile Roms
	memset(Kaneko16TempGfx, 0, 0x240000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 5, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x100000);
	GfxDecode(Kaneko16NumTiles, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles);
	memset(Kaneko16TempGfx, 0, 0x240000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 6, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x100000);
	GfxDecode(Kaneko16NumTiles2, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles2);
	
	// Load Sample Rom
	memset(Kaneko16TempGfx, 0, 0x240000);
	nRet = BurnLoadRom(Kaneko16TempGfx, 7, 1); if (nRet != 0) return 1;
	memcpy(MSM6295ROMData + 0x000000, Kaneko16TempGfx + 0x000000, 0x20000);
	memcpy(MSM6295ROMData + 0x020000, Kaneko16TempGfx + 0x020000, 0x20000);
	memcpy(MSM6295ROMData + 0x040000, Kaneko16TempGfx + 0x000000, 0x20000);
	memcpy(MSM6295ROMData + 0x060000, Kaneko16TempGfx + 0x040000, 0x20000);
	memcpy(MSM6295ROMData + 0x080000, Kaneko16TempGfx + 0x000000, 0x20000);
	memcpy(MSM6295ROMData + 0x0a0000, Kaneko16TempGfx + 0x060000, 0x20000);
	memcpy(MSM6295ROMData + 0x0c0000, Kaneko16TempGfx + 0x000000, 0x20000);
	memcpy(MSM6295ROMData + 0x0e0000, Kaneko16TempGfx + 0x080000, 0x20000);
	memcpy(MSM6295ROMData + 0x100000, Kaneko16TempGfx + 0x000000, 0x20000);
	memcpy(MSM6295ROMData + 0x120000, Kaneko16TempGfx + 0x0a0000, 0x20000);
	memcpy(MSM6295ROMData + 0x140000, Kaneko16TempGfx + 0x000000, 0x20000);
	memcpy(MSM6295ROMData + 0x160000, Kaneko16TempGfx + 0x0c0000, 0x20000);
	memcpy(MSM6295ROMData + 0x180000, Kaneko16TempGfx + 0x000000, 0x20000);
	memcpy(MSM6295ROMData + 0x1a0000, Kaneko16TempGfx + 0x0e0000, 0x20000);
	BurnFree(Kaneko16TempGfx);
	
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Kaneko16Rom          , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Kaneko16Ram          , 0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(Kaneko16Video1Ram    , 0x500000, 0x500fff, SM_RAM);
	SekMapMemory(Kaneko16Video0Ram    , 0x501000, 0x501fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl1Ram    , 0x502000, 0x502fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl0Ram    , 0x503000, 0x503fff, SM_RAM);
	SekMapMemory(Kaneko16Video3Ram    , 0x580000, 0x580fff, SM_RAM);
	SekMapMemory(Kaneko16Video2Ram    , 0x581000, 0x581fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl3Ram    , 0x582000, 0x582fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl2Ram    , 0x583000, 0x583fff, SM_RAM);
	SekMapMemory(Kaneko16SpriteRam    , 0x600000, 0x601fff, SM_RAM);
	SekMapMemory(Kaneko16PaletteRam   , 0x700000, 0x700fff, SM_RAM);
	SekMapMemory((UINT8*)Kaneko16Layer0Regs    , 0x800000, 0x80001f, SM_WRITE);
	SekMapMemory((UINT8*)Kaneko16SpriteRegs + 2, 0x900002, 0x90001f, SM_WRITE);
	SekMapMemory((UINT8*)Kaneko16Layer1Regs    , 0xb00000, 0xb0001f, SM_WRITE);
	SekSetReadByteHandler(0, ExplbrkrReadByte);
	SekSetReadWordHandler(0, ExplbrkrReadWord);
	SekSetWriteByteHandler(0, ExplbrkrWriteByte);
	SekSetWriteWordHandler(0, ExplbrkrWriteWord);
	SekClose();
	
	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;

	Kaneko16Eeprom = 1;
	EEPROMInit(&eeprom_interface_93C46);
	
	AY8910Init(0, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 2000000, nBurnSoundRate, &Kaneko16EepromRead, NULL, NULL, &Kaneko16EepromReset);
	
	// Setup the OKIM6295 emulation
	MSM6295Init(0, (12000000 / 6) / 132, 1);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	
	Kaneko16FrameRender = ExplbrkrFrameRender;
	
	// Reset the driver
	ExplbrkrDoReset();
	
	return 0;
}

static INT32 GtmrInit()
{
	INT32 nRet = 0, nLen;
	
	Gtmr = 1;
	
	Kaneko16NumSprites = 0x8400;
	Kaneko16NumTiles = 0x4000;
	Kaneko16NumTiles2 = 0x4000;
	
	Kaneko16VideoInit();
	Kaneko16ParseSprite = Kaneko16ParseSpriteType1;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	GtmrMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	GtmrMemIndex();

	Kaneko16TempGfx = (UINT8*)BurnMalloc(0x840000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Kaneko16Rom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16Rom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x200000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x400000, 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x600000, 6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x800001, 7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x800000, 8, 2); if (nRet != 0) return 1;
	GfxDecode(Kaneko16NumSprites, 8, 16, 16, EightBppPlaneOffsets, EightBppXOffsets, EightBppYOffsets, 0x800, Kaneko16TempGfx, Kaneko16Sprites);
	
	// Load and Decode Tile Roms
	memset(Kaneko16TempGfx, 0, 0x840000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 9, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x200000);
	GfxDecode(Kaneko16NumTiles, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles);
	BurnFree(Kaneko16TempGfx);
	memcpy(Kaneko16Tiles2, Kaneko16Tiles, Kaneko16NumTiles * 16 * 16);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROMData, 10, 1); if (nRet != 0) return 1;
	ExpandSampleBanks();
	
	ToyboxMCURun = GtmrMCURun;
	Kaneko16FrameRender = GtmrFrameRender;
		
	nRet = GtmrMachineInit(); if (nRet != 0) return 1;

	// Reset the driver
	GtmrDoReset();

	return 0;
}

static INT32 GtmrevoInit()
{
	INT32 nRet = 0, nLen;
	
	Gtmr = 1;
	
	Kaneko16NumSprites = 0x8000;
	Kaneko16NumTiles = 0x4000;
	Kaneko16NumTiles2 = 0x4000;
	
	Kaneko16VideoInit();
	Kaneko16ParseSprite = Kaneko16ParseSpriteType1;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	GtmrMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	GtmrMemIndex();

	Kaneko16TempGfx = (UINT8*)BurnMalloc(0x800000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Kaneko16Rom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16Rom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x600000, 7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x600001, 8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x200000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x400000, 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x600000, 6, 1); if (nRet != 0) return 1;
	GfxDecode(Kaneko16NumSprites, 8, 16, 16, EightBppPlaneOffsets, EightBppXOffsets, EightBppYOffsets, 0x800, Kaneko16TempGfx, Kaneko16Sprites);
	
	// Load and Decode Tile Roms
	memset(Kaneko16TempGfx, 0, 0x800000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 9, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x200000);
	GfxDecode(Kaneko16NumTiles, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles);
	BurnFree(Kaneko16TempGfx);
	memcpy(Kaneko16Tiles2, Kaneko16Tiles, Kaneko16NumTiles * 16 * 16);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROMData, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROMData2, 11, 1); if (nRet != 0) return 1;
	ExpandSampleBanks();
	
	nRet = GtmrMachineInit(); if (nRet != 0) return 1;
	
	ToyboxMCURun = GtmrevoMCURun;
	Kaneko16FrameRender = GtmrFrameRender;

	// Reset the driver
	GtmrDoReset();

	return 0;
}

static INT32 Gtmr2Init()
{
	INT32 nRet = 0, nLen;
	
	Gtmr = 1;
	
	Kaneko16NumSprites = 0x8000;
	Kaneko16NumTiles = 0x8800;
	Kaneko16NumTiles2 = 0x8800;
	
	Kaneko16VideoInit();
	Kaneko16ParseSprite = Kaneko16ParseSpriteType1;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	GtmrMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	GtmrMemIndex();

	Kaneko16TempGfx = (UINT8*)BurnMalloc(0x800000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Kaneko16Rom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16Rom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x400000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x600000, 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x700000, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x700001, 7, 2); if (nRet != 0) return 1;
	GfxDecode(Kaneko16NumSprites, 8, 16, 16, EightBppPlaneOffsets, EightBppXOffsets, EightBppYOffsets, 0x800, Kaneko16TempGfx, Kaneko16Sprites);
	
	// Load and Decode Tile Roms
	memset(Kaneko16TempGfx, 0, 0x800000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x200000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x400000, 10, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x400001, 11, 2); if (nRet != 0) return 1;
	UnscrambleTiles(0x440000);
	GfxDecode(Kaneko16NumTiles, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles);
	BurnFree(Kaneko16TempGfx);
	memcpy(Kaneko16Tiles2, Kaneko16Tiles, Kaneko16NumTiles * 16 * 16);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROMData, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROMData2 + 0x40000, 13, 1); if (nRet != 0) return 1;
	ExpandSampleBanks();
	
	nRet = GtmrMachineInit(); if (nRet != 0) return 1;
	
	ToyboxMCURun = GtmrevoMCURun;
	Kaneko16FrameRender = GtmrFrameRender;

	// Reset the driver
	GtmrDoReset();

	return 0;
}

static INT32 Gtmr2uInit()
{
	INT32 nRet = 0, nLen;
	
	Gtmr = 1;
	
	Kaneko16NumSprites = 0x8000;
	Kaneko16NumTiles = 0x8800;
	Kaneko16NumTiles2 = 0x8800;
	
	Kaneko16VideoInit();
	Kaneko16ParseSprite = Kaneko16ParseSpriteType1;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	GtmrMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	GtmrMemIndex();

	Kaneko16TempGfx = (UINT8*)BurnMalloc(0x800000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Kaneko16Rom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16Rom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x400000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x600000, 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x700000, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x700001, 7, 2); if (nRet != 0) return 1;
	GfxDecode(Kaneko16NumSprites, 8, 16, 16, EightBppPlaneOffsets, EightBppXOffsets, EightBppYOffsets, 0x800, Kaneko16TempGfx, Kaneko16Sprites);
	
	// Load and Decode Tile Roms
	memset(Kaneko16TempGfx, 0, 0x800000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x200000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x400000, 10, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x400001, 11, 2); if (nRet != 0) return 1;
	UnscrambleTiles(0x440000);
	GfxDecode(Kaneko16NumTiles, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles);
	BurnFree(Kaneko16TempGfx);
	memcpy(Kaneko16Tiles2, Kaneko16Tiles, Kaneko16NumTiles * 16 * 16);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROMData, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROMData2, 13, 1); if (nRet != 0) return 1;
	ExpandSampleBanks();
	
	nRet = GtmrMachineInit(); if (nRet != 0) return 1;
	
	ToyboxMCURun = GtmrevoMCURun;
	Kaneko16FrameRender = GtmrFrameRender;

	// Reset the driver
	GtmrDoReset();

	return 0;
}

static INT32 MgcrystlInit()
{
	INT32 nRet = 0, nLen;
	
	Mgcrystl = 1;
	
	Kaneko16NumSprites = 0x5000;
	Kaneko16NumTiles = 0x2000;
	Kaneko16NumTiles2 = 0x2000;
	
	Kaneko16VideoInit();
	
	// Allocate and Blank all required memory
	Mem = NULL;
	ExplbrkrMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	ExplbrkrMemIndex();

	Kaneko16TempGfx = (UINT8*)BurnMalloc(0x280000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Kaneko16Rom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16Rom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0000000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0100000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0180000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0200000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0220000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0240000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x0260000,  4, 1); if (nRet != 0) return 1;
	GfxDecode(Kaneko16NumSprites, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Sprites);
	
	// Load and Decode Tile Roms
	memset(Kaneko16TempGfx, 0, 0x280000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 5, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x100000);
	GfxDecode(Kaneko16NumTiles, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles);
	memset(Kaneko16TempGfx, 0, 0x280000);
	nRet = BurnLoadRom(Kaneko16TempGfx + 0x000000, 6, 1); if (nRet != 0) return 1;
	UnscrambleTiles(0x100000);
	GfxDecode(Kaneko16NumTiles2, 4, 16, 16, FourBppPlaneOffsets, FourBppXOffsets, FourBppYOffsets, 0x400, Kaneko16TempGfx, Kaneko16Tiles2);
	BurnFree(Kaneko16TempGfx);
	
	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 7, 1); if (nRet != 0) return 1;
	memcpy(MSM6295ROMData, MSM6295ROM, 0x40000);
	
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Kaneko16Rom          , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Kaneko16Ram          , 0x300000, 0x30ffff, SM_RAM);
	SekMapMemory(Kaneko16PaletteRam   , 0x500000, 0x500fff, SM_RAM);
	SekMapMemory(Kaneko16Video1Ram    , 0x600000, 0x600fff, SM_RAM);
	SekMapMemory(Kaneko16Video0Ram    , 0x601000, 0x601fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl1Ram    , 0x602000, 0x602fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl0Ram    , 0x603000, 0x603fff, SM_RAM);
	SekMapMemory(Kaneko16Video3Ram    , 0x680000, 0x680fff, SM_RAM);
	SekMapMemory(Kaneko16Video2Ram    , 0x681000, 0x681fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl3Ram    , 0x682000, 0x682fff, SM_RAM);
	SekMapMemory(Kaneko16VScrl2Ram    , 0x683000, 0x683fff, SM_RAM);
	SekMapMemory(Kaneko16SpriteRam    , 0x700000, 0x701fff, SM_RAM);
	SekMapMemory((UINT8*)Kaneko16Layer0Regs    , 0x800000, 0x80001f, SM_WRITE);
	SekMapMemory((UINT8*)Kaneko16SpriteRegs + 2, 0x900002, 0x90001f, SM_WRITE);
	SekMapMemory((UINT8*)Kaneko16Layer1Regs    , 0xb00000, 0xb0001f, SM_WRITE);
	SekSetReadByteHandler(0, MgcrystlReadByte);
	SekSetReadWordHandler(0, ExplbrkrReadWord);
	SekSetWriteByteHandler(0, ExplbrkrWriteByte);
	SekSetWriteWordHandler(0, ExplbrkrWriteWord);
	SekClose();

	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;

	Kaneko16Eeprom = 1;
	EEPROMInit(&eeprom_interface_93C46);
	
	AY8910Init(0, 2000000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 2000000, nBurnSoundRate, &Kaneko16EepromRead, NULL, NULL, &Kaneko16EepromReset);
	
	// Setup the OKIM6295 emulation
	MSM6295Init(0, (12000000 / 4) / 165, 1);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	
	Kaneko16FrameRender = MgcrystlFrameRender;

	// Reset the driver
	ExplbrkrDoReset();
	
	return 0;
}

static INT32 Kaneko16Exit()
{
	SekExit();
	if (Kaneko16Eeprom) EEPROMExit();

	GenericTilesExit();

	BurnFree(Mem);
	BurnFree(spritelist.first_sprite);
	
	Kaneko16NumTiles = 0;
	Kaneko16NumTiles2 = 0;
	Kaneko16NumSprites = 0;
	Kaneko16SpriteFlipX = 0;
	Kaneko16SpriteFlipY = 0;
	Kaneko16SpritesColourOffset = 0;
	Kaneko16SpritesColourMask = 0;
	Kaneko16LayersColourOffset = 0;
	Kaneko16DisplayEnable = 0;
	Kaneko168BppSprites = 0;
	Kaneko16Eeprom = 0;
	Kaneko16TilesXOffset = 0;
	Kaneko16TilesYOffset = 0;
	Kaneko16Bg15 = 0;
	Kaneko16Bg15Reg = 0;
	Kaneko16Bg15Select = 0;
	Kaneko16RecalcBg15Palette = 0;
	Kaneko16SpriteRamSize = 0;
	Kaneko16SpriteXOffset = 0;
	Kaneko16SoundLatch = 0;
	MSM6295Bank0 = 0;
	MSM6295Bank1 = 0;
	memset(Kaneko16SpriteRegs, 0, 0x20 * sizeof(UINT16));
	memset(Kaneko16Layer0Regs, 0, 0x10 * sizeof(UINT16));
	memset(Kaneko16Layer1Regs, 0, 0x10 * sizeof(UINT16));
	memset(ToyboxMCUCom, 0, 0x04 * sizeof(UINT16));
	
	ToyboxMCURun = NULL;
	Kaneko16FrameRender = NULL;
	Kaneko16ParseSprite = NULL;
	
	Gtmr = 0;
	Bloodwar = 0;
	Bonkadv = 0;
	Mgcrystl = 0;
	
	return 0;
}

static INT32 BerlwallExit()
{
	AY8910Exit(0);
	AY8910Exit(1);
	MSM6295Exit(0);
	
	return Kaneko16Exit();
}

static INT32 BlazeonExit()
{
	ZetExit();
	BurnYM2151Exit();
	
	return Kaneko16Exit();
}

static INT32 GtmrMachineExit()
{
	MSM6295Exit(0);
	MSM6295Exit(1);
		
	return Kaneko16Exit();
}

static INT32 ExplbrkrExit()
{
	AY8910Exit(0);
	AY8910Exit(1);
	MSM6295Exit(0);
		
	return Kaneko16Exit();
}

/*==============================================================================================
Sprite Rendering
===============================================================================================*/

#define USE_LATCHED_XY		1
#define USE_LATCHED_CODE	2
#define USE_LATCHED_COLOUR	4

static INT32 Kaneko16ParseSpriteType0(INT32 i, struct tempsprite *s)
{
	INT32 Attr, xOffs, Offset;
	UINT16 *SpriteRam = (UINT16*)Kaneko16SpriteRam;
	
	Offset = (i * 8) >> 1;
	
	if (Offset >= (Kaneko16SpriteRamSize >> 1)) return -1;
	
	Attr = SpriteRam[Offset + 0];
	s->code = SpriteRam[Offset + 1];
	s->x = SpriteRam[Offset + 2];
	s->y = SpriteRam[Offset + 3];
	
	s->color = (Attr & 0xfc) >> 2;
	s->priority = (Attr & 0x300) >> 8;
	s->flipy = Attr & 0x001;
	s->flipx = Attr & 0x002;
	
	xOffs = (Attr & 0x1800) >> 11;
	s->yoffs = Kaneko16SpriteRegs[0x08 + (xOffs * 2) + 1];
	s->xoffs = Kaneko16SpriteRegs[0x08 + (xOffs * 2) + 0];
	
	s->yoffs -= Kaneko16SpriteRegs[0x01];
	
	return ((Attr & 0x2000) ? USE_LATCHED_XY : 0) | ((Attr & 0x4000) ? USE_LATCHED_COLOUR: 0) | ((Attr & 0x8000) ? USE_LATCHED_CODE : 0);
}

static INT32 Kaneko16ParseSpriteType1(INT32 i, struct tempsprite *s)
{
	INT32 Attr, xOffs, Offset;
	UINT16 *SpriteRam = (UINT16*)Kaneko16SpriteRam;
	
	Offset = (i * 8) >> 1;
	
	if (Offset >= (Kaneko16SpriteRamSize >> 1)) return -1;
	
	Attr = SpriteRam[Offset + 0];
	s->code = SpriteRam[Offset + 1];
	s->x = SpriteRam[Offset + 2];
	s->y = SpriteRam[Offset + 3];
	
	s->color = Attr & 0x3f;
	s->priority = (Attr & 0xc0) >> 6;
	s->flipy = Attr & 0x100;
	s->flipx = Attr & 0x200;
	s->code += (s->y & 1) << 16;
	
	xOffs = (Attr & 0x1800) >> 11;
	s->yoffs = Kaneko16SpriteRegs[0x08 + (xOffs * 2) + 1];
	s->xoffs = Kaneko16SpriteRegs[0x08 + (xOffs * 2) + 0];
	
	s->yoffs -= Kaneko16SpriteRegs[0x01];
	
	return ((Attr & 0x2000) ? USE_LATCHED_XY : 0) | ((Attr & 0x4000) ? USE_LATCHED_COLOUR: 0) | ((Attr & 0x8000) ? USE_LATCHED_CODE : 0);
}

static INT32 Kaneko16ParseSpriteType2(INT32 i, struct tempsprite *s)
{
	INT32 Attr, xOffs, Offset;
	UINT16 *SpriteRam = (UINT16*)Kaneko16SpriteRam;
	
	Offset = (i * (16 >> 1)) + (8 >> 1);
	
	if (Offset >= (Kaneko16SpriteRamSize >> 1)) return -1;
	
	Attr = SpriteRam[Offset + 0];
	s->code = SpriteRam[Offset + 1];
	s->x = SpriteRam[Offset + 2];
	s->y = SpriteRam[Offset + 3];
	
	s->color = (Attr & 0xfc) >> 2;
	s->priority = (Attr & 0x300) >> 8;
	s->flipy = Attr & 0x001;
	s->flipx = Attr & 0x002;
	
	xOffs = (Attr & 0x1800) >> 11;
	s->yoffs = Kaneko16SpriteRegs[0x08 + (xOffs * 2) + 1];
	s->xoffs = Kaneko16SpriteRegs[0x08 + (xOffs * 2) + 0];
	
	s->yoffs -= Kaneko16SpriteRegs[0x01];
	
	return ((Attr & 0x2000) ? USE_LATCHED_XY : 0) | ((Attr & 0x4000) ? USE_LATCHED_COLOUR: 0) | ((Attr & 0x8000) ? USE_LATCHED_CODE : 0);
}

static void Kaneko16RenderSprite(UINT32 Code, UINT32 Colour, INT32 FlipX, INT32 FlipY, INT32 sx, INT32 sy)
{
	UINT8 *SourceBase = Kaneko16Sprites + ((Code % Kaneko16NumSprites) * 256);
	
	INT32 SpriteScreenHeight = ((1 << 16) * 16 + 0x8000) >> 16;
	INT32 SpriteScreenWidth = ((1 << 16) * 16 + 0x8000) >> 16;
	
	if (Kaneko168BppSprites) {
		Colour = 0x100 * (Colour % 0x40);
	} else {
		Colour = 0x10 * (Colour % 0x40);
	}
	
	if (SpriteScreenHeight && SpriteScreenWidth) {
		INT32 dx = (16 << 16) / SpriteScreenWidth;
		INT32 dy = (16 << 16) / SpriteScreenHeight;
		
		INT32 ex = sx + SpriteScreenWidth;
		INT32 ey = sy + SpriteScreenHeight;
		
		INT32 xIndexBase;
		INT32 yIndex;
		
		if (FlipX) {
			xIndexBase = (SpriteScreenWidth - 1) * dx;
			dx = -dx;
		} else {
			xIndexBase = 0;
		}
		
		if (FlipY) {
			yIndex = (SpriteScreenHeight - 1) * dy;
			dy = -dy;
		} else {
			yIndex = 0;
		}
		
		if (sx < 0) {
			INT32 Pixels = 0 - sx;
			sx += Pixels;
			xIndexBase += Pixels * dx;
		}
		
		if (sy < 0) {
			INT32 Pixels = 0 - sy;
			sy += Pixels;
			yIndex += Pixels * dy;
		}
		
		if (ex > nScreenWidth + 1) {
			INT32 Pixels = ex - nScreenWidth - 1;
			ex -= Pixels;
		}
		
		if (ey > nScreenHeight + 1) {
			INT32 Pixels = ey - nScreenHeight - 1;
			ey -= Pixels;	
		}
		
		if (ex > sx) {
			INT32 y;
			
			for (y = sy; y < ey; y++) {
				UINT8 *Source = SourceBase + ((yIndex >> 16) * 16);
				UINT16* pPixel = pTransDraw + (y * nScreenWidth);
				
				if (y < 0 || y > (nScreenHeight - 1)) continue;
				
				INT32 x, xIndex = xIndexBase;
				for (x = sx; x <ex; x++) {
					INT32 c = Source[xIndex >> 16];
					if (c != 0) {
						if (x >= 0 && x < nScreenWidth) pPixel[x] = (c | Colour | Kaneko16SpritesColourOffset) & Kaneko16SpritesColourMask;
					}
					xIndex += dx;
				}
				
				yIndex += dy;
			}
		}
	}
}

static void Kaneko16RenderSprites(INT32 PriorityDraw)
{
	struct tempsprite *s = spritelist.first_sprite;
	
	INT32 i = 0;
	INT32 x = 0;
	INT32 y = 0;
	INT32 Colour = 0;
	INT32 Code = 0;
	INT32 Priority = 0;
	INT32 xOffs = 0;
	INT32 yOffs = 0;
	INT32 FlipX = 0;
	INT32 FlipY = 0;
	
	while (1) {
		INT32 Flags;
		
		Flags = Kaneko16ParseSprite(i, s);
		
		if (Flags == -1) break;
		
		if (Flags & USE_LATCHED_CODE) {
			s->code = ++Code;
		} else {
			Code = s->code;
		}
		
		if (Flags & USE_LATCHED_COLOUR) {
			s->color = Colour;
			s->priority = Priority;
			s->xoffs = xOffs;
			s->yoffs = yOffs;
			s->flipx = FlipX;
			s->flipy = FlipY;
		} else {
			Colour = s->color;
			Priority = s->priority;
			xOffs = s->xoffs;
			yOffs = s->yoffs;
			FlipX = s->flipx;
			FlipY = s->flipy;
		}
		
		if (Flags & USE_LATCHED_XY)
		{
			s->x += x;
			s->y += y;
		}
		
		x = s->x;
		y = s->y;
		
		s->x = s->xoffs + s->x;
		s->y = s->yoffs + s->y;
		
		s->x += Kaneko16SpriteXOffset;
				
		s->x = ((s->x & 0x7fc0) - (s->x & 0x8000)) / 0x40;
		s->y = ((s->y & 0x7fc0) - (s->y & 0x8000)) / 0x40;
		
		i++;
		s++;
	}
	
	for (s = spritelist.first_sprite; s < spritelist.first_sprite + 0x400; s++) {
		if (s->priority == PriorityDraw) Kaneko16RenderSprite(s->code, s->color, s->flipx, s->flipy, s->x, s->y);
	}
}

#undef USE_LATCHED_XY
#undef USE_LATCHED_CODE
#undef USE_LATCHED_COLOUR

/*==============================================================================================
Tile Rendering
===============================================================================================*/

static INT32 Kaneko16GetLayerFlipOffset(INT32 curroffs)
{
	if (curroffs ==  0) return 15;
	if (curroffs ==  1) return 14;
	if (curroffs ==  2) return 13;
	if (curroffs ==  3) return 12;
	if (curroffs ==  4) return 11;
	if (curroffs ==  5) return 10;
	if (curroffs ==  6) return  9;
	if (curroffs ==  7) return  8;
	if (curroffs ==  8) return  7;
	if (curroffs ==  9) return  6;
	if (curroffs == 10) return  5;
	if (curroffs == 11) return  4;
	if (curroffs == 12) return  3;
	if (curroffs == 13) return  2;
	if (curroffs == 14) return  1;
	if (curroffs == 15) return  0;
	
	return 0;
}

static void Kaneko16QueueTilesLayer(INT32 Layer)
{
	INT32 x, y, mx, my, px, py, Code, Attr, Colour, Flip, Priority, LineScroll, TileIndex, pSrcXOffs, pSrcYOffs, xScroll, yScroll;
	UINT8 pTileSrc;
	
	LayerQueueSize[Layer] = 0;
	TileIndex = 0;
	
	UINT16 *VRAM = NULL;
	UINT16 *VSCROLLRAM = NULL;
	UINT16 *LAYERREGS = NULL;
	UINT8 *TILEDATA = NULL;
	INT32 xScrollReg = 0;
	INT32 yScrollReg = 0;
	INT32 xOffs = 0;
	INT32 numTiles = 0;
	
	switch (Layer) {
		case 0: {
			VRAM = (UINT16*)Kaneko16Video0Ram;
			VSCROLLRAM = (UINT16*)Kaneko16VScrl0Ram;
			LAYERREGS = Kaneko16Layer0Regs;
			TILEDATA = Kaneko16Tiles;
			xScrollReg = 2;
			yScrollReg = 3;
			xOffs = 0;
			numTiles = Kaneko16NumTiles;
			break;
		}
		
		case 1: {
			VRAM = (UINT16*)Kaneko16Video1Ram;
			VSCROLLRAM = (UINT16*)Kaneko16VScrl1Ram;
			LAYERREGS = Kaneko16Layer0Regs;
			TILEDATA = Kaneko16Tiles;
			xScrollReg = 0;
			yScrollReg = 1;
			xOffs = 2;
			numTiles = Kaneko16NumTiles;
			break;
		}
		
		case 2: {
			VRAM = (UINT16*)Kaneko16Video2Ram;
			VSCROLLRAM = (UINT16*)Kaneko16VScrl2Ram;
			LAYERREGS = Kaneko16Layer1Regs;
			TILEDATA = Kaneko16Tiles2;
			xScrollReg = 2;
			yScrollReg = 3;
			xOffs = 0;
			numTiles = Kaneko16NumTiles2;
			break;
		}
		
		case 3: {
			VRAM = (UINT16*)Kaneko16Video3Ram;
			VSCROLLRAM = (UINT16*)Kaneko16VScrl3Ram;
			LAYERREGS = Kaneko16Layer1Regs;
			TILEDATA = Kaneko16Tiles2;
			xScrollReg = 0;
			yScrollReg = 1;
			xOffs = 2;
			numTiles = Kaneko16NumTiles2;
			break;
		}
	}
	
	xScroll = LAYERREGS[xScrollReg];
	xOffs = Kaneko16TilesXOffset + xOffs;
	yScroll = ((LAYERREGS[yScrollReg] >> 6) & 0x1ff) - Kaneko16TilesYOffset;
	
	mx = my = -1;
	for (y = 0; y < 512; y++) {
		if (!(y % 16)) my++;
		
		LineScroll = VSCROLLRAM[y];
		py = y - yScroll;
		
		if (py < -30) py += 512;
		
		for (x = 0; x < 512; x++) {
			if (!(x % 16)) mx++;
			
			if (py < 0 || py >= nScreenHeight) continue;
			
			px = x - (((xScroll + LineScroll) >> 6) & 0x1ff) - xOffs;
			if (px < -7) px += 512;
			
			if (px < 0 || px >= nScreenWidth) continue;
			
			TileIndex = ((my * 32) + mx) * 2;
			
			Code = VRAM[TileIndex + 1];
			if (Code >= numTiles) continue;
			Attr = VRAM[TileIndex + 0];
			Priority = (Attr >> 8) & 7;
			Colour = (Attr >> 2) & 0x3f;
			Flip = Attr & 3;			
			
			pSrcXOffs = x % 16;
			pSrcYOffs = y % 16;
			if (Flip & 1) pSrcYOffs = Kaneko16GetLayerFlipOffset(pSrcYOffs);
			if (Flip & 2) pSrcXOffs = Kaneko16GetLayerFlipOffset(pSrcXOffs);
				
			pTileSrc = TILEDATA[(Code << 8) + (pSrcYOffs * 16) + pSrcXOffs];
			
			if (pTileSrc != 0) {
				LayerQueueXY[Layer][LayerQueueSize[Layer]] = (py << 9) | px;
				LayerQueueColour[Layer][LayerQueueSize[Layer]] = (Colour << 4) | pTileSrc;
				LayerQueuePriority[Layer][LayerQueueSize[Layer]] = Priority;
				LayerQueueSize[Layer]++;
			}
		}
		mx = -1;
	}
}

static void Kaneko16RenderLayerQueue(INT32 Layer, INT32 Priority)
{
	for (INT32 i = 0; i < LayerQueueSize[Layer]; i++) {
		if (LayerQueuePriority[Layer][i] == Priority) {
			UINT16* pPixel = pTransDraw + ((LayerQueueXY[Layer][i] >> 9) * nScreenWidth) + (LayerQueueXY[Layer][i] & 0x1ff);
			pPixel[0] = LayerQueueColour[Layer][i] | Kaneko16LayersColourOffset;
		}
	}
}

static void Kaneko16RenderTileLayer(INT32 Layer, INT32 PriorityDraw, INT32 xScroll)
{
	INT32 mx, my, Code, Attr, Colour, Flip, Priority, x, y, TileIndex = 0;
	
	UINT16 *VRAM = NULL;
	UINT16 *LAYERREGS = NULL;
	UINT8 *TILEDATA = NULL;
	INT32 yScrollReg = 0;
	INT32 xOffs = 0;
	INT32 numTiles = 0;
	
	switch (Layer) {
		case 0: {
			VRAM = (UINT16*)Kaneko16Video0Ram;
			LAYERREGS = Kaneko16Layer0Regs;
			TILEDATA = Kaneko16Tiles;
			yScrollReg = 3;
			xOffs = 0;
			numTiles = Kaneko16NumTiles;
			break;
		}
		
		case 1: {
			VRAM = (UINT16*)Kaneko16Video1Ram;
			LAYERREGS = Kaneko16Layer0Regs;
			TILEDATA = Kaneko16Tiles;
			yScrollReg = 1;
			xOffs = 2;
			numTiles = Kaneko16NumTiles;
			break;
		}
		
		case 2: {
			VRAM = (UINT16*)Kaneko16Video2Ram;
			LAYERREGS = Kaneko16Layer1Regs;
			TILEDATA = Kaneko16Tiles2;
			yScrollReg = 3;
			xOffs = 0;
			numTiles = Kaneko16NumTiles2;
			break;
		}
		
		case 3: {
			VRAM = (UINT16*)Kaneko16Video3Ram;
			LAYERREGS = Kaneko16Layer1Regs;
			TILEDATA = Kaneko16Tiles2;
			yScrollReg = 1;
			xOffs = 2;
			numTiles = Kaneko16NumTiles2;
			break;
		}
	}

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Code = VRAM[TileIndex + 1];
						
			if (Code >= numTiles) continue;
			
			Attr = VRAM[TileIndex + 0];
			Colour = (Attr >> 2) & 0x3f;
			Flip = Attr & 3;
			Priority = (Attr >> 8) & 7;
			
			if (Priority == PriorityDraw) {
				x = 16 * mx;
				y = 16 * my;
			
				x -= (xScroll >> 6) & 0x1ff;
				y -= (LAYERREGS[yScrollReg] >> 6) & 0x1ff;
			
				if (x < -7) x += 512;
				if (y < -30) y += 512;
			
				x -= Kaneko16TilesXOffset + xOffs;
				y += Kaneko16TilesYOffset;
							
				if (x > 0 && x < (nScreenWidth - 16) && y > 0 && y < (nScreenHeight - 16)) {
					if (Flip == 0) Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, Kaneko16LayersColourOffset, TILEDATA);
					if (Flip == 1) Render16x16Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, Kaneko16LayersColourOffset, TILEDATA);
					if (Flip == 2) Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, Kaneko16LayersColourOffset, TILEDATA);
					if (Flip == 3) Render16x16Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, Kaneko16LayersColourOffset, TILEDATA);
				} else {
					if (Flip == 0) Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, Kaneko16LayersColourOffset, TILEDATA);
					if (Flip == 1) Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, Kaneko16LayersColourOffset, TILEDATA);
					if (Flip == 2) Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, Kaneko16LayersColourOffset, TILEDATA);
					if (Flip == 3) Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, Kaneko16LayersColourOffset, TILEDATA);
				}
			}
			
			TileIndex += 2;
		}
	}
}

/*==============================================================================================
Background Bitmap Rendering
===============================================================================================*/

static void Kaneko16RenderBg15Bitmap()
{
	INT32 Select = Kaneko16Bg15Select;
	INT32 Flip = Select & 0x20;
	INT32 x, y;
	
	if (Flip) Select ^= 0x1f;
	Select &= 0x1f;
	
	for (y = 0; y < nScreenHeight; y++) {
		UINT16 *pPixel = pTransDraw + (y * nScreenWidth);
		UINT16 *data = Kaneko16Bg15Data + (Select * 256 * 256) + ((y + 16) * 256);
		
		for (x = 0; x < nScreenWidth; x++) {
			pPixel[x] = data[x];
		}
	}
}

/*==============================================================================================
Palette Handling
===============================================================================================*/

static inline UINT8 pal5bit(UINT8 bits)
{
	bits &= 0x1f;
	return (bits << 3) | (bits >> 2);
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal5bit(nColour >>  5);
	g = pal5bit(nColour >> 10);
	b = pal5bit(nColour >>  0);

	return BurnHighCol(r, g, b, 0);
}

INT32 Kaneko16CalcPalette(INT32 num)
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)Kaneko16PaletteRam, pd = Kaneko16Palette; i < num; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}

	return 0;
}

/*==============================================================================================
Graphics Rendering
===============================================================================================*/

// If the VScroll RAM is the same all the way through then just apply it as an xScroll value - much faster then doing line scroll on every line
#define HANDLE_VSCROLL(_N_) \
	if (Layer##_N_##Enabled) { \
		INT32 vScrollFast = 1; \
		UINT16 *VSCROLLRAM = (UINT16*)Kaneko16VScrl##_N_##Ram; \
		INT32 LineScroll = VSCROLLRAM[0]; \
		for (i = 0; i < 0x200; i++) { \
			if (VSCROLLRAM[i] != LineScroll) { \
				vScrollFast = 0; \
				break; \
			} \
		} \
		if (!vScrollFast) { \
			vScroll##_N_##Enabled = 1; \
			Kaneko16QueueTilesLayer(_N_); \
		} else { \
			vScroll##_N_##Enabled = 0; \
			xScroll##_N_ += LineScroll; \
		} \
	} \
\

static void BerlwallFrameRender()
{
	INT32 i;
	INT32 Layer0Enabled = 0;
	INT32 Layer1Enabled = 0;
	
	INT32 vScroll0Enabled = 0;
	INT32 vScroll1Enabled = 0;
	
	INT32 xScroll0 = Kaneko16Layer0Regs[2];
	INT32 xScroll1 = Kaneko16Layer0Regs[0];

	if (~Kaneko16Layer0Regs[4] & 0x1000) Layer0Enabled = 1;
	if (~Kaneko16Layer0Regs[4] & 0x0010) Layer1Enabled = 1;
	
	BurnTransferClear();
	Kaneko16CalcPalette(0x0800);
	
	if (Kaneko16RecalcBg15Palette) {
	 	for (i = 0; i < 32768; i++) {
			INT32 r = pal5bit(i >> 5);
			INT32 g = pal5bit(i >> 10);
			INT32 b = pal5bit(i >> 0);
			Kaneko16Palette[i + 2048] = BurnHighCol(r, g, b, 0);
		}
		
		Kaneko16RecalcBg15Palette = 0;
	}
	
	if (Kaneko16Layer0Regs[4] & 0x800) {
		HANDLE_VSCROLL(0)
	}
	
	if (Kaneko16Layer0Regs[4] & 0x008) {
		HANDLE_VSCROLL(1)
	}
	
	Kaneko16RenderBg15Bitmap();
	
	for (i = 0; i < 8; i++) {
		if (Layer0Enabled) { if (vScroll0Enabled) { Kaneko16RenderLayerQueue(0, i); } else { Kaneko16RenderTileLayer(0, i, xScroll0); }}
		if (Layer1Enabled) { if (vScroll1Enabled) { Kaneko16RenderLayerQueue(1, i); } else { Kaneko16RenderTileLayer(1, i, xScroll1); }}
	
		if (i == 0) Kaneko16RenderSprites(0);
		if (i == 1) Kaneko16RenderSprites(1);
		if (i == 2) Kaneko16RenderSprites(2);
		if (i == 7) Kaneko16RenderSprites(3);
	}
	
	BurnTransferCopy(Kaneko16Palette);
}

static void BlazeonFrameRender()
{
	INT32 i;
	INT32 Layer0Enabled = 0;
	INT32 Layer1Enabled = 0;
	
	INT32 vScroll0Enabled = 0;
	INT32 vScroll1Enabled = 0;
	
	INT32 xScroll0 = Kaneko16Layer0Regs[2];
	INT32 xScroll1 = Kaneko16Layer0Regs[0];

	if (~Kaneko16Layer0Regs[4] & 0x1000) Layer0Enabled = 1;
	if (~Kaneko16Layer0Regs[4] & 0x0010) Layer1Enabled = 1;
	
	BurnTransferClear();
	Kaneko16CalcPalette(0x0800);
	
	if (Kaneko16Layer0Regs[4] & 0x800) {
		HANDLE_VSCROLL(0)
	}
	
	if (Kaneko16Layer0Regs[4] & 0x008) {
		HANDLE_VSCROLL(1)
	}
	
	for (i = 0; i < 8; i++) {
		if (Layer0Enabled) { if (vScroll0Enabled) { Kaneko16RenderLayerQueue(0, i); } else { Kaneko16RenderTileLayer(0, i, xScroll0); }}
		if (Layer1Enabled) { if (vScroll1Enabled) { Kaneko16RenderLayerQueue(1, i); } else { Kaneko16RenderTileLayer(1, i, xScroll1); }}
	
		if (i == 0) Kaneko16RenderSprites(0);
		if (i == 1) Kaneko16RenderSprites(1);
		if (i == 7) Kaneko16RenderSprites(2);
		if (i == 7) Kaneko16RenderSprites(3);
	}
	
	BurnTransferCopy(Kaneko16Palette);
}

static void BloodwarFrameRender()
{
	INT32 i;
	INT32 Layer0Enabled = 0;
	INT32 Layer1Enabled = 0;
	INT32 Layer2Enabled = 0;
	INT32 Layer3Enabled = 0;
	
	INT32 vScroll0Enabled = 0;
	INT32 vScroll1Enabled = 0;
	INT32 vScroll2Enabled = 0;
	INT32 vScroll3Enabled = 0;
	
	INT32 xScroll0 = Kaneko16Layer0Regs[2];
	INT32 xScroll1 = Kaneko16Layer0Regs[0];
	INT32 xScroll2 = Kaneko16Layer1Regs[2];
	INT32 xScroll3 = Kaneko16Layer1Regs[0];
	
	if (~Kaneko16Layer0Regs[4] & 0x1000) Layer0Enabled = 1;
	if (~Kaneko16Layer0Regs[4] & 0x0010) Layer1Enabled = 1;
	if (~Kaneko16Layer1Regs[4] & 0x1000) Layer2Enabled = 1;
	if (~Kaneko16Layer1Regs[4] & 0x0010) Layer3Enabled = 1;
	
	BurnTransferClear();
	Kaneko16CalcPalette(0x10000);
	
	if (!Kaneko16DisplayEnable) return;
	
	if (Kaneko16Layer0Regs[4] & 0x800) {
		HANDLE_VSCROLL(0)
	}
	
	if (Kaneko16Layer0Regs[4] & 0x008) {
		HANDLE_VSCROLL(1)
	}
	
	if (Kaneko16Layer1Regs[4] & 0x800) {
		HANDLE_VSCROLL(2)
	}
	
	if (Kaneko16Layer1Regs[4] & 0x008) {
		HANDLE_VSCROLL(3)
	}
	
	for (i = 0; i < 8; i++) {
		if (Layer0Enabled) { if (vScroll0Enabled) { Kaneko16RenderLayerQueue(0, i); } else { Kaneko16RenderTileLayer(0, i, xScroll0); }}
		if (Layer1Enabled) { if (vScroll1Enabled) { Kaneko16RenderLayerQueue(1, i); } else { Kaneko16RenderTileLayer(1, i, xScroll1); }}
		if (Layer2Enabled) { if (vScroll2Enabled) { Kaneko16RenderLayerQueue(2, i); } else { Kaneko16RenderTileLayer(2, i, xScroll2); }}
		if (Layer3Enabled) { if (vScroll3Enabled) { Kaneko16RenderLayerQueue(3, i); } else { Kaneko16RenderTileLayer(3, i, xScroll3); }}
	
		if (i == 1) Kaneko16RenderSprites(0);
		if (i == 2) Kaneko16RenderSprites(1);
		if (i == 4) Kaneko16RenderSprites(2);
		if (i == 6) Kaneko16RenderSprites(3);
	}
	
	BurnTransferCopy(Kaneko16Palette);
}

static void ExplbrkrFrameRender()
{
	INT32 i;
	INT32 Layer0Enabled = 0;
	INT32 Layer1Enabled = 0;
	INT32 Layer2Enabled = 0;
	INT32 Layer3Enabled = 0;
	
	INT32 vScroll0Enabled = 0;
	INT32 vScroll1Enabled = 0;
	INT32 vScroll2Enabled = 0;
	INT32 vScroll3Enabled = 0;
	
	INT32 xScroll0 = Kaneko16Layer0Regs[2];
	INT32 xScroll1 = Kaneko16Layer0Regs[0];
	INT32 xScroll2 = Kaneko16Layer1Regs[2];
	INT32 xScroll3 = Kaneko16Layer1Regs[0];
	
	if (~Kaneko16Layer0Regs[4] & 0x1000) Layer0Enabled = 1;
	if (~Kaneko16Layer0Regs[4] & 0x0010) Layer1Enabled = 1;
	if (~Kaneko16Layer1Regs[4] & 0x1000) Layer2Enabled = 1;
	if (~Kaneko16Layer1Regs[4] & 0x0010) Layer3Enabled = 1;
	
	BurnTransferClear();
	Kaneko16CalcPalette(0x1000);
	
	if (Kaneko16Layer0Regs[4] & 0x800) {
		HANDLE_VSCROLL(0)
	}
	
	if (Kaneko16Layer0Regs[4] & 0x008) {
		HANDLE_VSCROLL(1)
	}
	
	if (Kaneko16Layer1Regs[4] & 0x800) {
		HANDLE_VSCROLL(2)
	}
	
	if (Kaneko16Layer1Regs[4] & 0x008) {
		HANDLE_VSCROLL(3)
	}
	
	for (i = 0; i < 8; i++) {
		if (Layer0Enabled) { if (vScroll0Enabled) { Kaneko16RenderLayerQueue(0, i); } else { Kaneko16RenderTileLayer(0, i, xScroll0); }}
		if (Layer1Enabled) { if (vScroll1Enabled) { Kaneko16RenderLayerQueue(1, i); } else { Kaneko16RenderTileLayer(1, i, xScroll1); }}
		if (Layer2Enabled) { if (vScroll2Enabled) { Kaneko16RenderLayerQueue(2, i); } else { Kaneko16RenderTileLayer(2, i, xScroll2); }}
		if (Layer3Enabled) { if (vScroll3Enabled) { Kaneko16RenderLayerQueue(3, i); } else { Kaneko16RenderTileLayer(3, i, xScroll3); }}
	}
	
	Kaneko16RenderSprites(0);
	Kaneko16RenderSprites(1);
	Kaneko16RenderSprites(2);
	Kaneko16RenderSprites(3);
	
	BurnTransferCopy(Kaneko16Palette);
}

static void GtmrFrameRender()
{
	INT32 i;
	INT32 Layer0Enabled = 0;
	INT32 Layer1Enabled = 0;
	INT32 Layer2Enabled = 0;
	INT32 Layer3Enabled = 0;
	
	INT32 vScroll0Enabled = 0;
	INT32 vScroll1Enabled = 0;
	INT32 vScroll2Enabled = 0;
	INT32 vScroll3Enabled = 0;
	
	INT32 xScroll0 = Kaneko16Layer0Regs[2];
	INT32 xScroll1 = Kaneko16Layer0Regs[0];
	INT32 xScroll2 = Kaneko16Layer1Regs[2];
	INT32 xScroll3 = Kaneko16Layer1Regs[0];
	
	if (~Kaneko16Layer0Regs[4] & 0x1000) Layer0Enabled = 1;
	if (~Kaneko16Layer0Regs[4] & 0x0010) Layer1Enabled = 1;
	if (~Kaneko16Layer1Regs[4] & 0x1000) Layer2Enabled = 1;
	if (~Kaneko16Layer1Regs[4] & 0x0010) Layer3Enabled = 1;
	
	BurnTransferClear();
	Kaneko16CalcPalette(0x10000);
	
	if (!Kaneko16DisplayEnable) return;
	
	if (Kaneko16Layer0Regs[4] & 0x800) {
		HANDLE_VSCROLL(0)
	}
	
	if (Kaneko16Layer0Regs[4] & 0x008) {
		HANDLE_VSCROLL(1)
	}
	
	if (Kaneko16Layer1Regs[4] & 0x800) {
		HANDLE_VSCROLL(2)
	}
	
	if (Kaneko16Layer1Regs[4] & 0x008) {
		HANDLE_VSCROLL(3)
	}
	
	for (i = 0; i < 8; i++) {
		if (Layer0Enabled) { if (vScroll0Enabled) { Kaneko16RenderLayerQueue(0, i); } else { Kaneko16RenderTileLayer(0, i, xScroll0); }}
		if (Layer1Enabled) { if (vScroll1Enabled) { Kaneko16RenderLayerQueue(1, i); } else { Kaneko16RenderTileLayer(1, i, xScroll1); }}
		if (Layer2Enabled) { if (vScroll2Enabled) { Kaneko16RenderLayerQueue(2, i); } else { Kaneko16RenderTileLayer(2, i, xScroll2); }}
		if (Layer3Enabled) { if (vScroll3Enabled) { Kaneko16RenderLayerQueue(3, i); } else { Kaneko16RenderTileLayer(3, i, xScroll3); }}
	
		if (i == 0) Kaneko16RenderSprites(0);
		if (i == 1) Kaneko16RenderSprites(1);
		if (i == 2) Kaneko16RenderSprites(2);
		if (i == 7) Kaneko16RenderSprites(3);
	}
	
	BurnTransferCopy(Kaneko16Palette);
}

static void MgcrystlFrameRender()
{
	INT32 i;
	INT32 Layer0Enabled = 0;
	INT32 Layer1Enabled = 0;
	INT32 Layer2Enabled = 0;
	INT32 Layer3Enabled = 0;
	
	INT32 vScroll0Enabled = 0;
	INT32 vScroll1Enabled = 0;
	INT32 vScroll2Enabled = 0;
	INT32 vScroll3Enabled = 0;
	
	INT32 xScroll0 = Kaneko16Layer0Regs[2];
	INT32 xScroll1 = Kaneko16Layer0Regs[0];
	INT32 xScroll2 = Kaneko16Layer1Regs[2];
	INT32 xScroll3 = Kaneko16Layer1Regs[0];
	
	if (~Kaneko16Layer0Regs[4] & 0x1000) Layer0Enabled = 1;
	if (~Kaneko16Layer0Regs[4] & 0x0010) Layer1Enabled = 1;
	if (~Kaneko16Layer1Regs[4] & 0x1000) Layer2Enabled = 1;
	if (~Kaneko16Layer1Regs[4] & 0x0010) Layer3Enabled = 1;
	
	BurnTransferClear();
	Kaneko16CalcPalette(0x1000);
	
	if (Kaneko16Layer0Regs[4] & 0x800) {
		HANDLE_VSCROLL(0)
	}
	
	if (Kaneko16Layer0Regs[4] & 0x008) {
		HANDLE_VSCROLL(1)
	}
	
	if (Kaneko16Layer1Regs[4] & 0x800) {
		HANDLE_VSCROLL(2)
	}
	
	if (Kaneko16Layer1Regs[4] & 0x008) {
		HANDLE_VSCROLL(3)
	}
	
	for (i = 0; i < 8; i++) {
		if (Layer0Enabled) { if (vScroll0Enabled) { Kaneko16RenderLayerQueue(0, i); } else { Kaneko16RenderTileLayer(0, i, xScroll0); }}
		if (Layer1Enabled) { if (vScroll1Enabled) { Kaneko16RenderLayerQueue(1, i); } else { Kaneko16RenderTileLayer(1, i, xScroll1); }}
		if (Layer2Enabled) { if (vScroll2Enabled) { Kaneko16RenderLayerQueue(2, i); } else { Kaneko16RenderTileLayer(2, i, xScroll2); }}
		if (Layer3Enabled) { if (vScroll3Enabled) { Kaneko16RenderLayerQueue(3, i); } else { Kaneko16RenderTileLayer(3, i, xScroll3); }}
	
		if (i == 1) Kaneko16RenderSprites(0);
		if (i == 2) Kaneko16RenderSprites(1);
		if (i == 4) Kaneko16RenderSprites(2);
		if (i == 6) Kaneko16RenderSprites(3);
	}
	
	BurnTransferCopy(Kaneko16Palette);
}

/*==============================================================================================
Frame functions
===============================================================================================*/

static INT32 ExplbrkrFrame()
{
	INT32 nInterleave = 10;
	nSoundBufferPos = 0;
	
	if (Kaneko16Reset) ExplbrkrDoReset();

	Kaneko16MakeInputs();
	
	nCyclesTotal[0] = 12000000 / 60;
	nCyclesDone[0] = 0;
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		nCurrentCPU = 0;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == 3) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		if (i == 6) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		if (i == 9) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
		SekClose();

		// Render Sound Segment
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Render(&pAY8910Buffer[0], pSoundBuf, nSegmentLength, 0);
			
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Render(&pAY8910Buffer[0], pSoundBuf, nSegmentLength, 0);
		}
		
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) Kaneko16FrameRender();
	
	return 0;
}

static INT32 BlazeonFrame()
{
	INT32 nInterleave = 10;
	nSoundBufferPos = 0;
		
	if (Kaneko16Reset) BlazeonDoReset();

	Kaneko16MakeInputs();
	
	nCyclesTotal[0] = 12000000 / 60;
	nCyclesTotal[1] = 4000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		nCurrentCPU = 0;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == 3) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		if (i == 6) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		if (i == 9) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
		SekClose();
		
		// Run Z80
		nCurrentCPU = 1;
		ZetOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetClose();

		// Render Sound Segment
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
		}

	}

	if (pBurnDraw) Kaneko16FrameRender();
	
	return 0;
}

static INT32 GtmrFrame()
{
	if (Kaneko16Reset) GtmrDoReset();

	Kaneko16MakeInputs();

	SekOpen(0);
	SekNewFrame();
	
	SekRun((16000000 / 60) / 4);
	SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
	SekRun((16000000 / 60) / 4);
	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	SekRun((16000000 / 60) / 4);
	SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
	SekRun((16000000 / 60) / 4);

	SekClose();
	
	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(1, pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) Kaneko16FrameRender();

	return 0;
}

/*==============================================================================================
Scan Driver
===============================================================================================*/

static INT32 Kaneko16Scan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {
		*pnMin = 0x029672;
	}
	
	if (Kaneko16Eeprom) EEPROMScan(nAction, pnMin);
	
	if (nAction & ACB_NVRAM && Kaneko16NVRam) {
		memset(&ba, 0, sizeof(ba));
		ba.Data = Kaneko16NVRam;
		ba.nLen = 0x100;
		ba.szName = "NVRam";
		BurnAcb(&ba);
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
				
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
		SCAN_VAR(Kaneko16Dip);
		SCAN_VAR(Kaneko16Input);
		SCAN_VAR(Kaneko16SoundLatch);
		SCAN_VAR(Kaneko16SpriteFlipX);
		SCAN_VAR(Kaneko16SpriteFlipY);
		SCAN_VAR(Kaneko16DisplayEnable);
		SCAN_VAR(Kaneko16SpriteRegs);
		SCAN_VAR(Kaneko16Layer0Regs);
		SCAN_VAR(Kaneko16Layer1Regs);
		
		if (Kaneko16Bg15) {
			SCAN_VAR(Kaneko16Bg15Reg);
			SCAN_VAR(Kaneko16Bg15Select);
		}
	}
	
	return 0;
}

static INT32 BlazeonScan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {
		*pnMin =  0x029672;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);
		BurnYM2151Scan(nAction);
		
		SCAN_VAR(nSoundBufferPos);
	}
	
	return Kaneko16Scan(nAction, pnMin);;
}

static INT32 ExplbrkrScan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {
		*pnMin =  0x029672;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		AY8910Scan(nAction, pnMin);
		MSM6295Scan(0, nAction);
		SCAN_VAR(MSM6295Bank0);
		SCAN_VAR(nSoundBufferPos);
	}
	
	if (nAction & ACB_WRITE) {
		memcpy(MSM6295ROM + 0x0000000, MSM6295ROMData  + (0x40000 * MSM6295Bank0),0x40000);
	}
	
	return Kaneko16Scan(nAction, pnMin);;
}

static INT32 GtmrScan(INT32 nAction,INT32 *pnMin)
{
	if (pnMin != NULL) {
		*pnMin =  0x029672;
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		MSM6295Scan(0, nAction);
		MSM6295Scan(1, nAction);
		SCAN_VAR(ToyboxMCUCom);
		SCAN_VAR(MSM6295Bank0);
		SCAN_VAR(MSM6295Bank1);
	}
	
	if (nAction & ACB_WRITE) {
		memcpy(MSM6295ROM + 0x0000000, MSM6295ROMData  + (0x40000 * MSM6295Bank0),0x40000);
		memcpy(MSM6295ROM + 0x0100000, MSM6295ROMData2 + (0x40000 * MSM6295Bank1),0x40000);
	}
	
	return Kaneko16Scan(nAction, pnMin);;
}

/*==============================================================================================
Driver defs
===============================================================================================*/

struct BurnDriver BurnDrvBerlwall = {
	"berlwall", NULL, NULL, NULL, "1991",
	"The Berlin Wall\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KANEKO16, GBF_PLATFORM, 0,
	NULL, BerlwallRomInfo, BerlwallRomName, NULL, NULL, BerlwallInputInfo, BerlwallDIPInfo,
	BerlwallInit, BerlwallExit, ExplbrkrFrame, NULL, ExplbrkrScan,
	&Kaneko16RecalcBg15Palette, 0x9000, 256, 224, 4, 3
};

struct BurnDriver BurnDrvBerlwallt = {
	"berlwallt", "berlwall", NULL, NULL, "1991",
	"The Berlin Wall (bootleg ?)\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_KANEKO16, GBF_PLATFORM, 0,
	NULL, BerlwalltRomInfo, BerlwalltRomName, NULL, NULL, BerlwallInputInfo, BerlwalltDIPInfo,
	BerlwallInit, BerlwallExit, ExplbrkrFrame, NULL, ExplbrkrScan,
	&Kaneko16RecalcBg15Palette, 0x9000, 256, 224, 4, 3
};

struct BurnDriver BurnDrvBlazeon = {
	"blazeon", NULL, NULL, NULL, "1991",
	"Blaze On (Japan)\0", NULL, "Atlus", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KANEKO16, GBF_HORSHOOT, 0,
	NULL, BlazeonRomInfo, BlazeonRomName, NULL, NULL, BlazeonInputInfo, BlazeonDIPInfo,
	BlazeonInit, BlazeonExit, BlazeonFrame, NULL, BlazeonScan,
	NULL, 0x1000, 320, 232, 4, 3
};

struct BurnDriver BurnDrvBloodwar = {
	"bloodwar", NULL, NULL, NULL, "1994",
	"Blood Warrior\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KANEKO16, GBF_VSFIGHT, 0,
	NULL, BloodwarRomInfo, BloodwarRomName, NULL, NULL, BloodwarInputInfo, BloodwarDIPInfo,
	BloodwarInit, GtmrMachineExit, GtmrFrame, NULL, GtmrScan,
	NULL, 0x10000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvBonkadv = {
	"bonkadv", NULL, NULL, NULL, "1994",
	"B.C. Kid / Bonk's Adventure / Kyukyoku!! PC Genjin\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KANEKO16, GBF_PLATFORM, 0,
	NULL, BonkadvRomInfo, BonkadvRomName, NULL, NULL, BonkadvInputInfo, BonkadvDIPInfo,
	BonkadvInit, GtmrMachineExit, GtmrFrame, NULL, GtmrScan,
	NULL, 0x10000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvExplbrkr = {
	"explbrkr", NULL, NULL, NULL, "1992",
	"Explosive Breaker\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_KANEKO16, GBF_VERSHOOT, 0,
	NULL, ExplbrkrRomInfo, ExplbrkrRomName, NULL, NULL, ExplbrkrInputInfo, ExplbrkrDIPInfo,
	ExplbrkrInit, ExplbrkrExit, ExplbrkrFrame, NULL, ExplbrkrScan,
	NULL, 0x1000, 224, 256, 3, 4
};

struct BurnDriver BurnDrvBakubrkr = {
	"bakubrkr", "explbrkr", NULL, NULL, "1992",
	"Bakuretsu Breaker\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_KANEKO16, GBF_VERSHOOT, 0,
	NULL, BakubrkrRomInfo, BakubrkrRomName, NULL, NULL, ExplbrkrInputInfo, ExplbrkrDIPInfo,
	ExplbrkrInit, ExplbrkrExit, ExplbrkrFrame, NULL, ExplbrkrScan,
	NULL, 0x1000, 224, 256, 3, 4
};

struct BurnDriver BurnDrvGtmr = {
	"gtmr", NULL, NULL, NULL, "1994",
	"1000 Miglia: Great 1000 Miles Rally (94/07/18)\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KANEKO16, GBF_RACING, 0,
	NULL, GtmrRomInfo, GtmrRomName, NULL, NULL, GtmrInputInfo, GtmrDIPInfo,
	GtmrInit, GtmrMachineExit, GtmrFrame, NULL, GtmrScan,
	NULL, 0x10000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvGtmra = {
	"gtmra", "gtmr", NULL, NULL, "1994",
	"1000 Miglia: Great 1000 Miles Rally (94/06/13)\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KANEKO16, GBF_RACING, 0,
	NULL, GtmraRomInfo, GtmraRomName, NULL, NULL, GtmrInputInfo, GtmrDIPInfo,
	GtmrInit, GtmrMachineExit, GtmrFrame, NULL, GtmrScan,
	NULL, 0x10000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvGtmre = {
	"gtmre", "gtmr", NULL, NULL, "1994",
	"Great 1000 Miles Rally: Evolution Model!!! (94/09/06)\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KANEKO16, GBF_RACING, 0,
	NULL, GtmreRomInfo, GtmreRomName, NULL, NULL, GtmrInputInfo, GtmrDIPInfo,
	GtmrevoInit, GtmrMachineExit, GtmrFrame, NULL, GtmrScan,
	NULL, 0x10000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvGtmrusa = {
	"gtmrusa", "gtmr", NULL, NULL, "1994",
	"Great 1000 Miles Rally: U.S.A Version! (94/09/06)\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KANEKO16, GBF_RACING, 0,
	NULL, GtmrusaRomInfo, GtmrusaRomName, NULL, NULL, GtmrInputInfo, GtmrDIPInfo,
	GtmrevoInit, GtmrMachineExit, GtmrFrame, NULL, GtmrScan,
	NULL, 0x10000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvGtmr2 = {
	"gtmr2", NULL, NULL, NULL, "1995",
	"Mille Miglia 2: Great 1000 Miles Rally (95/05/24)\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KANEKO16, GBF_RACING, 0,
	NULL, Gtmr2RomInfo, Gtmr2RomName, NULL, NULL, GtmrInputInfo, Gtmr2DIPInfo,
	Gtmr2Init, GtmrMachineExit, GtmrFrame, NULL, GtmrScan,
	NULL, 0x10000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvGtmr2a = {
	"gtmr2a", "gtmr2", NULL, NULL, "1995",
	"Mille Miglia 2: Great 1000 Miles Rally (95/04/04)\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KANEKO16, GBF_RACING, 0,
	NULL, Gtmr2aRomInfo, Gtmr2aRomName, NULL, NULL, GtmrInputInfo, Gtmr2DIPInfo,
	Gtmr2Init, GtmrMachineExit, GtmrFrame, NULL, GtmrScan,
	NULL, 0x10000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvGtmr2u = {
	"gtmr2u", "gtmr2", NULL, NULL, "1995",
	"Great 1000 Miles Rally 2 USA (95/05/18)\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KANEKO16, GBF_RACING, 0,
	NULL, Gtmr2uRomInfo, Gtmr2uRomName, NULL, NULL, GtmrInputInfo, Gtmr2DIPInfo,
	Gtmr2uInit, GtmrMachineExit, GtmrFrame, NULL, GtmrScan,
	NULL, 0x10000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvMgcrsytl = {
	"mgcrystl", NULL, NULL, NULL, "1991",
	"Magical Crystals (World, 92/01/10)\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KANEKO16, GBF_PLATFORM, 0,
	NULL, MgcrystlRomInfo, MgcrystlRomName, NULL, NULL, MgcrystlInputInfo, MgcrystlDIPInfo,
	MgcrystlInit, ExplbrkrExit, ExplbrkrFrame, NULL, ExplbrkrScan,
	NULL, 0x1000, 256, 224, 4, 3
};

struct BurnDriver BurnDrvMgcrsytlo = {
	"mgcrystlo", "mgcrystl", NULL, NULL, "1991",
	"Magical Crystals (World, 91/12/10)\0", NULL, "Kaneko", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KANEKO16, GBF_PLATFORM, 0,
	NULL, MgcrystloRomInfo, MgcrystloRomName, NULL, NULL, MgcrystlInputInfo, MgcrystlDIPInfo,
	MgcrystlInit, ExplbrkrExit, ExplbrkrFrame, NULL, ExplbrkrScan,
	NULL, 0x1000, 256, 224, 4, 3
};

struct BurnDriver BurnDrvMgcrsytlj = {
	"mgcrystlj", "mgcrystl", NULL, NULL, "1991",
	"Magical Crystals (Japan, 92/01/13)\0", NULL, "Kaneko (Atlus license)", "Kaneko16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KANEKO16, GBF_PLATFORM, 0,
	NULL, MgcrystljRomInfo, MgcrystljRomName, NULL, NULL, MgcrystlInputInfo, MgcrystlDIPInfo,
	MgcrystlInit, ExplbrkrExit, ExplbrkrFrame, NULL, ExplbrkrScan,
	NULL, 0x1000, 256, 224, 4, 3
};
