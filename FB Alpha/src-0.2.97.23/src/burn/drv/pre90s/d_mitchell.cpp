#include "tiles_generic.h"
#include "eeprom.h"
#include "burn_ym2413.h"
#include "msm6295.h"

static UINT8 DrvInputPort0[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort3[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort4[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort5[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort6[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort7[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort8[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort9[8]  = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort10[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort11[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[12]      = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static UINT8 DrvDip[2]         = {0, 0};
static UINT8 DrvReset          = 0;
static INT16 DrvDial1          = 0;
static INT16 DrvDial2          = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *DrvZ80Rom           = NULL;
static UINT8 *DrvZ80Code          = NULL;
static UINT8 *DrvZ80Rom2          = NULL;
static UINT8 *DrvSoundRom         = NULL;
static UINT8 *DrvZ80Ram           = NULL;
static UINT8 *DrvZ80Ram2          = NULL;
static UINT8 *DrvPaletteRam       = NULL;
static UINT8 *DrvAttrRam          = NULL;
static UINT8 *DrvVideoRam         = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette          = NULL;
static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static UINT8 DrvRomBank;
static UINT8 DrvPaletteRamBank;
static UINT8 DrvOkiBank;
static UINT8 DrvFlipScreen;
static UINT8 DrvVideoBank;
static UINT8 DrvInput5Toggle;
static UINT8 DrvPort5Kludge;
static INT32 DrvTileMask;
static UINT8 DrvHasEEPROM;
static INT32 DrvNumColours;
static INT32 DrvNVRamSize;
static INT32 DrvNVRamAddress;
static UINT8 DrvDialSelected;
static INT32 DrvDial[2];
static UINT8 DrvSoundLatch;

static UINT8 DrvInputType;
static INT32 DrvMahjongKeyMatrix;

#define DRV_INPUT_TYPE_MAHJONG		1
#define DRV_INPUT_TYPE_BLOCK		2

static struct BurnInputInfo MgakuenInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0  + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort1  + 2, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort7  + 2, "p2 start"  },

	{"P1 A"              , BIT_DIGITAL  , DrvInputPort1  + 7, "mah a"     },
	{"P1 B"              , BIT_DIGITAL  , DrvInputPort2  + 7, "mah b"     },
	{"P1 C"              , BIT_DIGITAL  , DrvInputPort4  + 7, "mah c"     },
	{"P1 D"              , BIT_DIGITAL  , DrvInputPort5  + 7, "mah d"     },
	{"P1 E"              , BIT_DIGITAL  , DrvInputPort1  + 6, "mah e"     },
	{"P1 F"              , BIT_DIGITAL  , DrvInputPort2  + 6, "mah f"     },
	{"P1 G"              , BIT_DIGITAL  , DrvInputPort4  + 6, "mah g"     },
	{"P1 H"              , BIT_DIGITAL  , DrvInputPort5  + 6, "mah h"     },
	{"P1 I"              , BIT_DIGITAL  , DrvInputPort1  + 5, "mah i"     },
	{"P1 J"              , BIT_DIGITAL  , DrvInputPort2  + 5, "mah j"     },
	{"P1 K"              , BIT_DIGITAL  , DrvInputPort4  + 5, "mah k"     },
	{"P1 L"              , BIT_DIGITAL  , DrvInputPort5  + 5, "mah l"     },
	{"P1 M"              , BIT_DIGITAL  , DrvInputPort1  + 4, "mah m"     },
	{"P1 N"              , BIT_DIGITAL  , DrvInputPort2  + 4, "mah n"     },
	{"P1 Pon"            , BIT_DIGITAL  , DrvInputPort5  + 4, "mah pon"   },
	{"P1 Chi"            , BIT_DIGITAL  , DrvInputPort4  + 4, "mah chi"   },
	{"P1 Kan"            , BIT_DIGITAL  , DrvInputPort1  + 3, "mah kan"   },
	{"P1 Ron"            , BIT_DIGITAL  , DrvInputPort4  + 3, "mah ron"   },
	{"P1 Reach"          , BIT_DIGITAL  , DrvInputPort2  + 3, "mah reach" },
	{"P1 Flip Flop"      , BIT_DIGITAL  , DrvInputPort6  + 4, "mah ff"    },
	
	{"P2 A"              , BIT_DIGITAL  , DrvInputPort7  + 7, "mah a"     },
	{"P2 B"              , BIT_DIGITAL  , DrvInputPort8  + 7, "mah b"     },
	{"P2 C"              , BIT_DIGITAL  , DrvInputPort9  + 7, "mah c"     },
	{"P2 D"              , BIT_DIGITAL  , DrvInputPort10 + 7, "mah d"     },
	{"P2 E"              , BIT_DIGITAL  , DrvInputPort7  + 6, "mah e"     },
	{"P2 F"              , BIT_DIGITAL  , DrvInputPort8  + 6, "mah f"     },
	{"P2 G"              , BIT_DIGITAL  , DrvInputPort9  + 6, "mah g"     },
	{"P2 H"              , BIT_DIGITAL  , DrvInputPort10 + 6, "mah h"     },
	{"P2 I"              , BIT_DIGITAL  , DrvInputPort7  + 5, "mah i"     },
	{"P2 J"              , BIT_DIGITAL  , DrvInputPort8  + 5, "mah j"     },
	{"P2 K"              , BIT_DIGITAL  , DrvInputPort9  + 5, "mah k"     },
	{"P2 L"              , BIT_DIGITAL  , DrvInputPort10 + 5, "mah l"     },
	{"P2 M"              , BIT_DIGITAL  , DrvInputPort7  + 4, "mah m"     },
	{"P2 N"              , BIT_DIGITAL  , DrvInputPort2  + 4, "mah n"     },
	{"P2 Pon"            , BIT_DIGITAL  , DrvInputPort10 + 4, "mah pon"   },
	{"P2 Chi"            , BIT_DIGITAL  , DrvInputPort9  + 4, "mah chi"   },
	{"P2 Kan"            , BIT_DIGITAL  , DrvInputPort7  + 3, "mah kan"   },
	{"P2 Ron"            , BIT_DIGITAL  , DrvInputPort9  + 3, "mah ron"   },
	{"P2 Reach"          , BIT_DIGITAL  , DrvInputPort8  + 3, "mah reach" },
	{"P2 Flip Flop"      , BIT_DIGITAL  , DrvInputPort11 + 4, "mah ff"    },	

	{"Reset"             , BIT_DIGITAL  , &DrvReset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1        , "dip"       },
};

STDINPUTINFO(Mgakuen)

static struct BurnInputInfo MarukinInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0  + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort1  + 2, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort7  + 2, "p2 start"  },

	{"P1 A"              , BIT_DIGITAL  , DrvInputPort1  + 7, "mah a"     },
	{"P1 B"              , BIT_DIGITAL  , DrvInputPort2  + 7, "mah b"     },
	{"P1 C"              , BIT_DIGITAL  , DrvInputPort4  + 7, "mah c"     },
	{"P1 D"              , BIT_DIGITAL  , DrvInputPort5  + 7, "mah d"     },
	{"P1 E"              , BIT_DIGITAL  , DrvInputPort1  + 6, "mah e"     },
	{"P1 F"              , BIT_DIGITAL  , DrvInputPort2  + 6, "mah f"     },
	{"P1 G"              , BIT_DIGITAL  , DrvInputPort4  + 6, "mah g"     },
	{"P1 H"              , BIT_DIGITAL  , DrvInputPort5  + 6, "mah h"     },
	{"P1 I"              , BIT_DIGITAL  , DrvInputPort1  + 5, "mah i"     },
	{"P1 J"              , BIT_DIGITAL  , DrvInputPort2  + 5, "mah j"     },
	{"P1 K"              , BIT_DIGITAL  , DrvInputPort4  + 5, "mah k"     },
	{"P1 L"              , BIT_DIGITAL  , DrvInputPort5  + 5, "mah l"     },
	{"P1 M"              , BIT_DIGITAL  , DrvInputPort1  + 4, "mah m"     },
	{"P1 N"              , BIT_DIGITAL  , DrvInputPort2  + 4, "mah n"     },
	{"P1 Pon"            , BIT_DIGITAL  , DrvInputPort5  + 4, "mah pon"   },
	{"P1 Chi"            , BIT_DIGITAL  , DrvInputPort4  + 4, "mah chi"   },
	{"P1 Kan"            , BIT_DIGITAL  , DrvInputPort1  + 3, "mah kan"   },
	{"P1 Ron"            , BIT_DIGITAL  , DrvInputPort4  + 3, "mah ron"   },
	{"P1 Reach"          , BIT_DIGITAL  , DrvInputPort2  + 3, "mah reach" },
	{"P1 Flip Flop"      , BIT_DIGITAL  , DrvInputPort6  + 4, "mah ff"    },
	
	{"P2 A"              , BIT_DIGITAL  , DrvInputPort7  + 7, "mah a"     },
	{"P2 B"              , BIT_DIGITAL  , DrvInputPort8  + 7, "mah b"     },
	{"P2 C"              , BIT_DIGITAL  , DrvInputPort9  + 7, "mah c"     },
	{"P2 D"              , BIT_DIGITAL  , DrvInputPort10 + 7, "mah d"     },
	{"P2 E"              , BIT_DIGITAL  , DrvInputPort7  + 6, "mah e"     },
	{"P2 F"              , BIT_DIGITAL  , DrvInputPort8  + 6, "mah f"     },
	{"P2 G"              , BIT_DIGITAL  , DrvInputPort9  + 6, "mah g"     },
	{"P2 H"              , BIT_DIGITAL  , DrvInputPort10 + 6, "mah h"     },
	{"P2 I"              , BIT_DIGITAL  , DrvInputPort7  + 5, "mah i"     },
	{"P2 J"              , BIT_DIGITAL  , DrvInputPort8  + 5, "mah j"     },
	{"P2 K"              , BIT_DIGITAL  , DrvInputPort9  + 5, "mah k"     },
	{"P2 L"              , BIT_DIGITAL  , DrvInputPort10 + 5, "mah l"     },
	{"P2 M"              , BIT_DIGITAL  , DrvInputPort7  + 4, "mah m"     },
	{"P2 N"              , BIT_DIGITAL  , DrvInputPort2  + 4, "mah n"     },
	{"P2 Pon"            , BIT_DIGITAL  , DrvInputPort10 + 4, "mah pon"   },
	{"P2 Chi"            , BIT_DIGITAL  , DrvInputPort9  + 4, "mah chi"   },
	{"P2 Kan"            , BIT_DIGITAL  , DrvInputPort7  + 3, "mah kan"   },
	{"P2 Ron"            , BIT_DIGITAL  , DrvInputPort9  + 3, "mah ron"   },
	{"P2 Reach"          , BIT_DIGITAL  , DrvInputPort8  + 3, "mah reach" },
	{"P2 Flip Flop"      , BIT_DIGITAL  , DrvInputPort11 + 4, "mah ff"    },	

	{"Reset"             , BIT_DIGITAL  , &DrvReset         , "reset"     },
	{"Diagnostics"       , BIT_DIGITAL  , DrvInputPort3  + 1, "diag"      },
};

STDINPUTINFO(Marukin)

static struct BurnInputInfo PkladiesInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0  + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort1  + 2, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort7  + 2, "p2 start"  },

	{"P1 A"              , BIT_DIGITAL  , DrvInputPort1  + 7, "mah a"     },
	{"P1 B"              , BIT_DIGITAL  , DrvInputPort2  + 7, "mah b"     },
	{"P1 C"              , BIT_DIGITAL  , DrvInputPort4  + 7, "mah c"     },
	{"P1 D"              , BIT_DIGITAL  , DrvInputPort5  + 7, "mah d"     },
	{"P1 E"              , BIT_DIGITAL  , DrvInputPort1  + 6, "mah e"     },
	{"P1 Deal"           , BIT_DIGITAL  , DrvInputPort1  + 5, "p1 fire 1" },
	{"P1 Cancel"         , BIT_DIGITAL  , DrvInputPort2  + 5, "p1 fire 2" },
	{"P1 Flip"           , BIT_DIGITAL  , DrvInputPort4  + 5, "p1 fire 3" },
	
	{"P2 A"              , BIT_DIGITAL  , DrvInputPort7  + 7, "mah a"     },
	{"P2 B"              , BIT_DIGITAL  , DrvInputPort8  + 7, "mah b"     },
	{"P2 C"              , BIT_DIGITAL  , DrvInputPort9  + 7, "mah c"     },
	{"P2 D"              , BIT_DIGITAL  , DrvInputPort10 + 7, "mah d"     },
	{"P2 E"              , BIT_DIGITAL  , DrvInputPort7  + 6, "mah e"     },
	{"P2 Deal"           , BIT_DIGITAL  , DrvInputPort7  + 5, "p2 fire 1" },
	{"P2 Cancel"         , BIT_DIGITAL  , DrvInputPort8  + 5, "p2 fire 2" },
	{"P2 Flip"           , BIT_DIGITAL  , DrvInputPort9  + 5, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset         , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0  + 0, "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , DrvInputPort3  + 1, "diag"      },
};

STDINPUTINFO(Pkladies)

static struct BurnInputInfo PangInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0  + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0  + 3, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0  + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort1  + 7, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort1  + 6, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort1  + 5, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort1  + 4, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort1  + 3, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort1  + 2, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort2  + 7, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort2  + 6, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort2  + 5, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort2  + 4, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort2  + 3, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort2  + 2, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset         , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0  + 6, "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , DrvInputPort3  + 1, "diag"      },
};

STDINPUTINFO(Pang)

static struct BurnInputInfo Qtono1InputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0  + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort1  + 2, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort2  + 2, "p2 start"  },

	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort1  + 7, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort1  + 6, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , DrvInputPort1  + 5, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL  , DrvInputPort1  + 4, "p1 fire 4" },
	
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort2  + 7, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort2  + 6, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , DrvInputPort2  + 5, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL  , DrvInputPort2  + 4, "p2 fire 4" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset         , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0  + 6, "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , DrvInputPort3  + 1, "diag"      },
};

STDINPUTINFO(Qtono1)

static struct BurnInputInfo MstworldInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0  + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0  + 3, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0  + 5, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0  + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort1  + 7, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort1  + 6, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort1  + 5, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort1  + 4, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort1  + 3, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort1  + 2, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , DrvInputPort1  + 1, "p1 fire 3" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort2  + 7, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort2  + 6, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort2  + 5, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort2  + 4, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort2  + 3, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort2  + 2, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , DrvInputPort2  + 1, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset         , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0  + 6, "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , DrvInputPort3  + 1, "diag"      },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0        , "dip"       },
};

STDINPUTINFO(Mstworld)

static struct BurnInputInfo BlockInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0  + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0  + 3, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0  + 1, "p2 start"  },

	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort11 + 0, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort11 + 1, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort1  + 7, "p1 fire 1" },
		
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort11 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort11 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort2  + 7, "p2 fire 1" },
	
	{"Reset"             , BIT_DIGITAL  , &DrvReset         , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0  + 6, "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , DrvInputPort3  + 1, "diag"      },
};

STDINPUTINFO(Block)

static struct BurnInputInfo BlockjoyInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0  + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0  + 3, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0  + 1, "p2 start"  },

	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort1  + 5, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort1  + 4, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort1  + 7, "p1 fire 1" },
	
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort2  + 5, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort2  + 4, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort2  + 7, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset         , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0  + 6, "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , DrvInputPort3  + 1, "diag"      },
};

STDINPUTINFO(Blockjoy)

static inline void DrvClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x30) == 0x30) {
		*nJoystickInputs &= ~0x30;
	}
	if ((*nJoystickInputs & 0xc0) == 0xc0) {
		*nJoystickInputs &= ~0xc0;
	}
}

static inline void DrvMakeInputs()
{
	for (INT32 i = 0; i < 12; i++) DrvInput[i] = 0x00;

	for (INT32 i = 0; i < 8; i++) {
		DrvInput[ 0] |= (DrvInputPort0[ i] & 1) << i;
		DrvInput[ 1] |= (DrvInputPort1[ i] & 1) << i;
		DrvInput[ 2] |= (DrvInputPort2[ i] & 1) << i;
		DrvInput[ 3] |= (DrvInputPort3[ i] & 1) << i;
		DrvInput[ 4] |= (DrvInputPort4[ i] & 1) << i;
		DrvInput[ 5] |= (DrvInputPort5[ i] & 1) << i;
		DrvInput[ 6] |= (DrvInputPort6[ i] & 1) << i;
		DrvInput[ 7] |= (DrvInputPort7[ i] & 1) << i;
		DrvInput[ 8] |= (DrvInputPort8[ i] & 1) << i;
		DrvInput[ 9] |= (DrvInputPort9[ i] & 1) << i;
		DrvInput[10] |= (DrvInputPort10[i] & 1) << i;
		DrvInput[11] |= (DrvInputPort11[i] & 1) << i;
	}

	if (DrvInputType == DRV_INPUT_TYPE_BLOCK) {
		if (DrvInputPort11[0]) DrvDial1 -= 0x04;
		if (DrvInputPort11[1]) DrvDial1 += 0x04;
		if (DrvDial1 >= 0x100) DrvDial1 = 0;
		if (DrvDial1 < 0) DrvDial1 = 0xfc;
		
		if (DrvInputPort11[2]) DrvDial2 -= 0x04;
		if (DrvInputPort11[3]) DrvDial2 += 0x04;
		if (DrvDial2 >= 0x100) DrvDial2 = 0;
		if (DrvDial2 < 0) DrvDial2 = 0xfc;
	} else {
		if (DrvInputType != DRV_INPUT_TYPE_MAHJONG) {
			DrvClearOpposites(&DrvInput[1]);
			DrvClearOpposites(&DrvInput[2]);
		}
	}
}

static struct BurnDIPInfo MgakuenDIPList[]=
{
	// Default Values
	{0x2c, 0xff, 0xff, 0xef, NULL                     },
	{0x2d, 0xff, 0xff, 0x8f, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x2c, 0x01, 0x07, 0x00, "4 Coins 1 Play"         },
	{0x2c, 0x01, 0x07, 0x01, "3 Coins 1 Play"         },
	{0x2c, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x2c, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x2c, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x2c, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x2c, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x2c, 0x01, 0x07, 0x03, "1 Coin  6 Plays"        },
	
	{0   , 0xfe, 0   , 2   , "Rules"                  },
	{0x2c, 0x01, 0x08, 0x08, "Kantou"                 },
	{0x2c, 0x01, 0x08, 0x00, "Kansai"                 },
	
	{0   , 0xfe, 0   , 2   , "Harness Type"           },
	{0x2c, 0x01, 0x10, 0x10, "Generic"                },
	{0x2c, 0x01, 0x10, 0x00, "Royal Mahjong"          },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x2c, 0x01, 0x20, 0x20, "Off"                    },
	{0x2c, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x2c, 0x01, 0x40, 0x40, "Off"                    },
	{0x2c, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x2c, 0x01, 0x80, 0x80, "Off"                    },
	{0x2c, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Player 1 Skill"         },
	{0x2d, 0x01, 0x03, 0x03, "Weak"                   },
	{0x2d, 0x01, 0x03, 0x02, "Normal"                 },
	{0x2d, 0x01, 0x03, 0x01, "Strong"                 },
	{0x2d, 0x01, 0x03, 0x00, "Very Strong"            },
	
	{0   , 0xfe, 0   , 4   , "Player 2 Skill"         },
	{0x2d, 0x01, 0x0c, 0x0c, "Weak"                   },
	{0x2d, 0x01, 0x0c, 0x08, "Normal"                 },
	{0x2d, 0x01, 0x0c, 0x04, "Strong"                 },
	{0x2d, 0x01, 0x0c, 0x00, "Very Strong"            },
	
	{0   , 0xfe, 0   , 2   , "Music"                  },
	{0x2d, 0x01, 0x10, 0x10, "Off"                    },
	{0x2d, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x2d, 0x01, 0x20, 0x20, "Off"                    },
	{0x2d, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Help Mode"              },
	{0x2d, 0x01, 0x40, 0x40, "Off"                    },
	{0x2d, 0x01, 0x40, 0x00, "On"                     },
};

STDDIPINFO(Mgakuen)

static struct BurnDIPInfo MstworldDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xb0, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coinage"                },
	{0x15, 0x01, 0x07, 0x03, "A 1C/4P B 1C/4P"        },
	{0x15, 0x01, 0x07, 0x02, "A 1C/3P B 1C/3P"        },
	{0x15, 0x01, 0x07, 0x01, "A 1C/2P B 1C/2P"        },
	{0x15, 0x01, 0x07, 0x00, "A 1C/1P B 1C/4P"        },
	{0x15, 0x01, 0x07, 0x04, "A 2C/1P B 1C/2P"        },
	{0x15, 0x01, 0x07, 0x05, "A 2C/1P B 1C/3P"        },
	{0x15, 0x01, 0x07, 0x06, "A 3C/1P B 1C/2P"        },
	{0x15, 0x01, 0x07, 0x07, "A 4C/1P B 1C/1P"        },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x15, 0x01, 0x18, 0x00, "1"                      },
	{0x15, 0x01, 0x18, 0x08, "2"                      },
	{0x15, 0x01, 0x18, 0x10, "3"                      },
	{0x15, 0x01, 0x18, 0x18, "4"                      },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x15, 0x01, 0x60, 0x00, "Easy"                   },
	{0x15, 0x01, 0x60, 0x20, "Normal"                 },
	{0x15, 0x01, 0x60, 0x40, "Hard"                   },
	{0x15, 0x01, 0x60, 0x60, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x15, 0x01, 0x80, 0x00, "Off"                    },
	{0x15, 0x01, 0x80, 0x80, "On"                     },
};

STDDIPINFO(Mstworld)

static struct BurnRomInfo MgakuenRomDesc[] = {
	{ "mg-1.1j",       0x08000, 0xbf02ea6b, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "mg-2.1l",       0x20000, 0x64141b0c, BRF_ESS | BRF_PRG }, //	 1
	
	{ "mg-1.13h",      0x80000, 0xfd6a0805, BRF_GRA },	     //  2	Characters
	{ "mg-2.14h",      0x80000, 0xe26e871e, BRF_GRA },	     //  3
	{ "mg-3.16h",      0x80000, 0xdd781d9a, BRF_GRA },	     //  4
	{ "mg-4.17h",      0x80000, 0x97afcc79, BRF_GRA },	     //  5
	
	{ "mg-6.4l",       0x20000, 0x34594e62, BRF_GRA },	     //  6	Sprites
	{ "mg-7.6l",       0x20000, 0xf304c806, BRF_GRA },	     //  7
	
	{ "mg-5.1c",       0x80000, 0x170332f1, BRF_SND },	     //  8	Samples
};

STD_ROM_PICK(Mgakuen)
STD_ROM_FN(Mgakuen)

static struct BurnRomInfo SeventoitsuRomDesc[] = {
	{ "mc01.1j",       0x08000, 0x0bebe45f, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "mc02.1l",       0x20000, 0x375378b0, BRF_ESS | BRF_PRG }, //	 1
	
	{ "mg-1.13h",      0x80000, 0xfd6a0805, BRF_GRA },	     //  2	Characters
	{ "mg-2.14h",      0x80000, 0xe26e871e, BRF_GRA },	     //  3
	{ "mg-3.16h",      0x80000, 0xdd781d9a, BRF_GRA },	     //  4
	{ "mg-4.17h",      0x80000, 0x97afcc79, BRF_GRA },	     //  5
	
	{ "mc06.4l",       0x20000, 0x0ef83926, BRF_GRA },	     //  6	Sprites
	{ "mc07.6l",       0x20000, 0x59f9ffb1, BRF_GRA },	     //  7
	
	{ "mg-5.1c",       0x80000, 0x170332f1, BRF_SND },	     //  8	Samples
};

STD_ROM_PICK(Seventoitsu)
STD_ROM_FN(Seventoitsu)

static struct BurnRomInfo Mgakuen2RomDesc[] = {
	{ "mg2-xf.1j",     0x08000, 0xc8165d2d, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "mg2-y.1l",      0x20000, 0x75bbcc14, BRF_ESS | BRF_PRG }, //	 1
	{ "mg2-z.3l",      0x20000, 0xbfdba961, BRF_ESS | BRF_PRG }, //	 2
	
	{ "mg2-a.13h",     0x80000, 0x31a0c55e, BRF_GRA },	     //  3	Characters
	{ "mg2-b.14h",     0x80000, 0xc18488fa, BRF_GRA },	     //  4
	{ "mg2-c.16h",     0x80000, 0x9425b364, BRF_GRA },	     //  5
	{ "mg2-d.17h",     0x80000, 0x6cc9eeba, BRF_GRA },	     //  6
	
	{ "mg2-f.4l",      0x20000, 0x3172c9fe, BRF_GRA },	     //  7	Sprites
	{ "mg2-g.6l",      0x20000, 0x19b8b61c, BRF_GRA },	     //  8
	
	{ "mg2-e.1c",      0x80000, 0x70fd0809, BRF_SND },	     //  9	Samples
};

STD_ROM_PICK(Mgakuen2)
STD_ROM_FN(Mgakuen2)

static struct BurnRomInfo PkladiesRomDesc[] = {
	{ "pko-prg1.14f",  0x08000, 0x86585a94, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "pko-prg2.15f",  0x10000, 0x86cbe82d, BRF_ESS | BRF_PRG }, //	 1
	
	{ "pko-001.8h",    0x80000, 0x1ead5d9b, BRF_GRA },	     //  2	Characters
	{ "pko-003.8j",    0x80000, 0x339ab4e6, BRF_GRA },	     //  3
	{ "pko-002.9h",    0x80000, 0x1cf02586, BRF_GRA },	     //  4
	{ "pko-004.9j",    0x80000, 0x09ccb442, BRF_GRA },	     //  5
	
	{ "pko-chr1.2j",   0x20000, 0x31ce33cd, BRF_GRA },	     //  6	Sprites
	{ "pko-chr2.3j",   0x20000, 0xad7e055f, BRF_GRA },	     //  7
	
	{ "pko-voi1.2d",   0x20000, 0x07e0f531, BRF_SND },	     //  8	Samples
	{ "pko-voi2.3d",   0x20000, 0x18398bf6, BRF_SND },	     //  9
};

STD_ROM_PICK(Pkladies)
STD_ROM_FN(Pkladies)

static struct BurnRomInfo PkladieslRomDesc[] = {
	{ "pk05.14f",      0x08000, 0xea1740a6, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "pk06.15f",      0x20000, 0x3078ff5e, BRF_ESS | BRF_PRG }, //	 1
	
	{ "pko-001.8h",    0x80000, 0x1ead5d9b, BRF_GRA },	     //  2	Characters
	{ "pko-003.8j",    0x80000, 0x339ab4e6, BRF_GRA },	     //  3
	{ "pko-002.9h",    0x80000, 0x1cf02586, BRF_GRA },	     //  4
	{ "pko-004.9j",    0x80000, 0x09ccb442, BRF_GRA },	     //  5
	
	{ "pko-chr1.2j",   0x20000, 0x31ce33cd, BRF_GRA },	     //  6	Sprites
	{ "pko-chr2.3j",   0x20000, 0xad7e055f, BRF_GRA },	     //  7
	
	{ "pko-voi1.2d",   0x20000, 0x07e0f531, BRF_SND },	     //  8	Samples
	{ "pko-voi2.3d",   0x20000, 0x18398bf6, BRF_SND },	     //  9
};

STD_ROM_PICK(Pkladiesl)
STD_ROM_FN(Pkladiesl)

static struct BurnRomInfo PkladieslaRomDesc[] = {
	{ "05.14f",        0x08000, 0xfa18e16a, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "06.15f",        0x10000, 0xa2fb7646, BRF_ESS | BRF_PRG }, //	 1
	
	{ "pko-001.8h",    0x80000, 0x1ead5d9b, BRF_GRA },	     //  2	Characters
	{ "pko-003.8j",    0x80000, 0x339ab4e6, BRF_GRA },	     //  3
	{ "pko-002.9h",    0x80000, 0x1cf02586, BRF_GRA },	     //  4
	{ "pko-004.9j",    0x80000, 0x09ccb442, BRF_GRA },	     //  5
	
	{ "pko-chr1.2j",   0x20000, 0x31ce33cd, BRF_GRA },	     //  6	Sprites
	{ "pko-chr2.3j",   0x20000, 0xad7e055f, BRF_GRA },	     //  7
	
	{ "pko-voi1.2d",   0x20000, 0x07e0f531, BRF_SND },	     //  8	Samples
	{ "pko-voi2.3d",   0x20000, 0x18398bf6, BRF_SND },	     //  9
};

STD_ROM_PICK(Pkladiesla)
STD_ROM_FN(Pkladiesla)

static struct BurnRomInfo DokabenRomDesc[] = {
	{ "db06.11h",      0x08000, 0x413e0886, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "db07.13h",      0x20000, 0x8bdcf49e, BRF_ESS | BRF_PRG }, //	 1
	{ "db08.14h",      0x20000, 0x1643bdd9, BRF_ESS | BRF_PRG }, //	 2
	
	{ "db02.1e",       0x20000, 0x9aa8470c, BRF_GRA },	     //  3	Characters
	{ "db03.2e",       0x20000, 0x3324e43d, BRF_GRA },	     //  4
	{ "db04.1g",       0x20000, 0xc0c5b6c2, BRF_GRA },	     //  5
	{ "db05.2g",       0x20000, 0xd2ab25f2, BRF_GRA },	     //  6
	
	{ "db10.2k",       0x20000, 0x9e70f7ae, BRF_GRA },	     //  7	Sprites
	{ "db09.1k",       0x20000, 0x2d9263f7, BRF_GRA },	     //  8
	
	{ "db01.1d",       0x20000, 0x62fa6b81, BRF_SND },	     //  9	Samples
};

STD_ROM_PICK(Dokaben)
STD_ROM_FN(Dokaben)

static struct BurnRomInfo PangRomDesc[] = {
	{ "pang6.bin",     0x08000, 0x68be52cd, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "pang7.bin",     0x20000, 0x4a2e70f6, BRF_ESS | BRF_PRG }, //	 1
	
	{ "pang_09.bin",   0x20000, 0x3a5883f5, BRF_GRA },	     //  2	Characters
	{ "bb3.bin",       0x20000, 0x79a8ed08, BRF_GRA },	     //  3
	{ "pang_11.bin",   0x20000, 0x166a16ae, BRF_GRA },	     //  4
	{ "bb5.bin",       0x20000, 0x2fb3db6c, BRF_GRA },	     //  5
	
	{ "bb10.bin",      0x20000, 0xfdba4f6e, BRF_GRA },	     //  6	Sprites
	{ "bb9.bin",       0x20000, 0x39f47a63, BRF_GRA },	     //  7
	
	{ "bb1.bin",       0x20000, 0xc52e5b8e, BRF_SND },	     //  8	Samples
};

STD_ROM_PICK(Pang)
STD_ROM_FN(Pang)

static struct BurnRomInfo BbrosRomDesc[] = {
	{ "bb6.bin",       0x08000, 0xa3041ca4, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bb7.bin",       0x20000, 0x09231c68, BRF_ESS | BRF_PRG }, //	 1
	
	{ "bb2.bin",       0x20000, 0x62f29992, BRF_GRA },	     //  2	Characters
	{ "bb3.bin",       0x20000, 0x79a8ed08, BRF_GRA },	     //  3
	{ "bb4.bin",       0x20000, 0xf705aa89, BRF_GRA },	     //  4
	{ "bb5.bin",       0x20000, 0x2fb3db6c, BRF_GRA },	     //  5
	
	{ "bb10.bin",      0x20000, 0xfdba4f6e, BRF_GRA },	     //  6	Sprites
	{ "bb9.bin",       0x20000, 0x39f47a63, BRF_GRA },	     //  7
	
	{ "bb1.bin",       0x20000, 0xc52e5b8e, BRF_SND },	     //  8	Samples
};

STD_ROM_PICK(Bbros)
STD_ROM_FN(Bbros)

static struct BurnRomInfo PompingwRomDesc[] = {
	{ "pwj_06.11h",    0x08000, 0x4a0a6426, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "pwj_07.13h",    0x20000, 0xa9402420, BRF_ESS | BRF_PRG }, //	 1
	
	{ "pw_02.1e",      0x20000, 0x4b5992e4, BRF_GRA },	     //  2	Characters
	{ "bb3.bin",       0x20000, 0x79a8ed08, BRF_GRA },	     //  3
	{ "pwj_04.1g",     0x20000, 0x01e49081, BRF_GRA },	     //  4
	{ "bb5.bin",       0x20000, 0x2fb3db6c, BRF_GRA },	     //  5
	
	{ "bb10.bin",      0x20000, 0xfdba4f6e, BRF_GRA },	     //  6	Sprites
	{ "bb9.bin",       0x20000, 0x39f47a63, BRF_GRA },	     //  7
	
	{ "bb1.bin",       0x20000, 0xc52e5b8e, BRF_SND },	     //  8	Samples
};

STD_ROM_PICK(Pompingw)
STD_ROM_FN(Pompingw)

static struct BurnRomInfo PangbRomDesc[] = {
	{ "pang_04.bin",   0x10000, 0xf68f88a5, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "pang_02.bin",   0x20000, 0x3f15bb61, BRF_ESS | BRF_PRG }, //	 1
	{ "pang_03.bin",   0x20000, 0x0c8477ae, BRF_ESS | BRF_PRG }, //	 2
	
	{ "pang_09.bin",   0x20000, 0x3a5883f5, BRF_GRA },	     //  3	Characters
	{ "bb3.bin",       0x20000, 0x79a8ed08, BRF_GRA },	     //  4
	{ "pang_11.bin",   0x20000, 0x166a16ae, BRF_GRA },	     //  5
	{ "bb5.bin",       0x20000, 0x2fb3db6c, BRF_GRA },	     //  6
	
	{ "bb10.bin",      0x20000, 0xfdba4f6e, BRF_GRA },	     //  7	Sprites
	{ "bb9.bin",       0x20000, 0x39f47a63, BRF_GRA },	     //  8
	
	{ "bb1.bin",       0x20000, 0xc52e5b8e, BRF_SND },	     //  9	Samples
};

STD_ROM_PICK(Pangb)
STD_ROM_FN(Pangb)

static struct BurnRomInfo PangboldRomDesc[] = {
	{ "4.6l",          0x10000, 0xf68f88a5, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "2.3l",          0x20000, 0x3f15bb61, BRF_ESS | BRF_PRG }, //	 1
	{ "3.5l",          0x20000, 0xce6375e4, BRF_ESS | BRF_PRG }, //	 2
	
	{ "9.10o",         0x20000, 0x3a5883f5, BRF_GRA },	     //  3	Characters
	{ "10.14o",        0x20000, 0x79a8ed08, BRF_GRA },	     //  4
	{ "11.17j",        0x20000, 0x166a16ae, BRF_GRA },	     //  5
	{ "12.20j",        0x20000, 0x2fb3db6c, BRF_GRA },	     //  6
	
	{ "8.7o",          0x10000, 0xf3188aa1, BRF_GRA },	     //  7	Sprites
	{ "7.5o",          0x10000, 0x011da14b, BRF_GRA },	     //  8
	{ "6.3o",          0x10000, 0x0e25e797, BRF_GRA },	     //  9
	{ "5.1o",          0x10000, 0x6daa4e27, BRF_GRA },	     //  10
	
	{ "1.1a",          0x10000, 0xb6463907, BRF_SND },	     //  11	Samples
};

STD_ROM_PICK(Pangbold)
STD_ROM_FN(Pangbold)

static struct BurnRomInfo CworldRomDesc[] = {
	{ "cw05.bin",      0x08000, 0xd3c1723d, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "cw06.bin",      0x20000, 0xd71ed4a3, BRF_ESS | BRF_PRG }, //	 1
	{ "cw07.bin",      0x20000, 0xd419ce08, BRF_ESS | BRF_PRG }, //	 2
	
	{ "cw08.bin",      0x20000, 0x6c80da3c, BRF_GRA },	     //  3	Characters
	{ "cw09.bin",      0x20000, 0x7607da71, BRF_GRA },	     //  4
	{ "cw10.bin",      0x20000, 0x6f0e639f, BRF_GRA },	     //  5
	{ "cw11.bin",      0x20000, 0x130bd7c0, BRF_GRA },	     //  6
	{ "cw18.bin",      0x20000, 0xbe6ee0c9, BRF_GRA },	     //  7
	{ "cw19.bin",      0x20000, 0x51fc5532, BRF_GRA },	     //  8
	{ "cw20.bin",      0x20000, 0x58381d58, BRF_GRA },	     //  9
	{ "cw21.bin",      0x20000, 0x910cc753, BRF_GRA },	     //  10
	
	{ "cw16.bin",      0x20000, 0xf90217d1, BRF_GRA },	     //  11	Sprites
	{ "cw17.bin",      0x20000, 0xc953c702, BRF_GRA },	     //  12
	
	{ "cw01.bin",      0x20000, 0xf4368f5b, BRF_SND },	     //  13	Samples
};

STD_ROM_PICK(Cworld)
STD_ROM_FN(Cworld)

static struct BurnRomInfo HatenaRomDesc[] = {
	{ "q2-05.rom",     0x08000, 0x66c9e1da, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "q2-06.rom",     0x20000, 0x5fc39916, BRF_ESS | BRF_PRG }, //	 1
	{ "q2-07.rom",     0x20000, 0xec6d5e5e, BRF_ESS | BRF_PRG }, //	 2
	
	{ "q2-08.rom",     0x20000, 0x6c80da3c, BRF_GRA },	     //  3	Characters
	{ "q2-09.rom",     0x20000, 0xabe3e15c, BRF_GRA },	     //  4
	{ "q2-10.rom",     0x20000, 0x6963450d, BRF_GRA },	     //  5
	{ "q2-11.rom",     0x20000, 0x1e319fa2, BRF_GRA },	     //  6
	{ "q2-18.rom",     0x20000, 0xbe6ee0c9, BRF_GRA },	     //  7
	{ "q2-19.rom",     0x20000, 0x70300445, BRF_GRA },	     //  8
	{ "q2-20.rom",     0x20000, 0x21a6ff42, BRF_GRA },	     //  9
	{ "q2-21.rom",     0x20000, 0x076280c9, BRF_GRA },	     //  10
	
	{ "q2-16.rom",     0x20000, 0xec19b2f0, BRF_GRA },	     //  11	Sprites
	{ "q2-17.rom",     0x20000, 0xecd69d92, BRF_GRA },	     //  12
	
	{ "q2-01.rom",     0x20000, 0x149e7a89, BRF_SND },	     //  13	Samples
};

STD_ROM_PICK(Hatena)
STD_ROM_FN(Hatena)

static struct BurnRomInfo SpangRomDesc[] = {
	{ "spe_06.rom",    0x08000, 0x1af106fb, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "spe_07.rom",    0x20000, 0x208b5f54, BRF_ESS | BRF_PRG }, //	 1
	{ "spe_08.rom",    0x20000, 0x2bc03ade, BRF_ESS | BRF_PRG }, //	 2
	
	{ "spe_02.rom",    0x20000, 0x63c9dfd2, BRF_GRA },	     //  3	Characters
	{ "03.f2",         0x20000, 0x3ae28bc1, BRF_GRA },	     //  4
	{ "spe_04.rom",    0x20000, 0x9d7b225b, BRF_GRA },	     //  5
	{ "05.g2",         0x20000, 0x4a060884, BRF_GRA },	     //  6
	
	{ "spj10_2k.bin",  0x20000, 0xeedd0ade, BRF_GRA },	     //  7	Sprites
	{ "spj09_1k.bin",  0x20000, 0x04b41b75, BRF_GRA },	     //  8
	
	{ "spe_01.rom",    0x20000, 0x2d19c133, BRF_SND },	     //  9	Samples
	
	{ "eeprom-spang.bin", 0x80, 0xdeae1291, BRF_PRG },
};

STD_ROM_PICK(Spang)
STD_ROM_FN(Spang)

static struct BurnRomInfo SpangjRomDesc[] = {
	{ "spj_11h.bin",   0x08000, 0x1a548b0b, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "spj7_13h.bin",  0x20000, 0x14c2b765, BRF_ESS | BRF_PRG }, //	 1
	{ "spj8_14h.bin",  0x20000, 0x4be4e5b7, BRF_ESS | BRF_PRG }, //	 2
	
	{ "spj02_1e.bin",  0x20000, 0x419f69d7, BRF_GRA },	     //  3	Characters
	{ "03.f2",         0x20000, 0x3ae28bc1, BRF_GRA },	     //  4
	{ "spj04_1g.bin",  0x20000, 0x6870506f, BRF_GRA },	     //  5
	{ "05.g2",         0x20000, 0x4a060884, BRF_GRA },	     //  6
	
	{ "spj10_2k.bin",  0x20000, 0xeedd0ade, BRF_GRA },	     //  7	Sprites
	{ "spj09_1k.bin",  0x20000, 0x04b41b75, BRF_GRA },	     //  8
	
	{ "01.d1",         0x20000, 0xb96ea126, BRF_SND },	     //  9	Samples
	
	{ "eeprom-spangj.bin",0x80, 0x237c00eb, BRF_PRG },
};

STD_ROM_PICK(Spangj)
STD_ROM_FN(Spangj)

static struct BurnRomInfo SbbrosRomDesc[] = {
	{ "06.j12",        0x08000, 0x292eee6a, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "07.j13",        0x20000, 0xf46b698d, BRF_ESS | BRF_PRG }, //	 1
	{ "08.j14",        0x20000, 0xa75e7fbe, BRF_ESS | BRF_PRG }, //	 2
	
	{ "02.f1",         0x20000, 0x0c22ffc6, BRF_GRA },	     //  3	Characters
	{ "03.f2",         0x20000, 0x3ae28bc1, BRF_GRA },	     //  4
	{ "04.g2",         0x20000, 0xbb3dee5b, BRF_GRA },	     //  5
	{ "05.g2",         0x20000, 0x4a060884, BRF_GRA },	     //  6
	
	{ "10.l2",         0x20000, 0xd6675d8f, BRF_GRA },	     //  7	Sprites
	{ "09.l1",         0x20000, 0x8f678bc8, BRF_GRA },	     //  8
	
	{ "01.d1",         0x20000, 0xb96ea126, BRF_SND },	     //  9	Samples
	
	{ "eeprom-sbbros.bin",0x80, 0xed69d3cd, BRF_PRG },
};

STD_ROM_PICK(Sbbros)
STD_ROM_FN(Sbbros)

static struct BurnRomInfo MstworldRomDesc[] = {
	{ "mw-1.rom",      0x80000, 0xc4e51fb4, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	
	{ "mw-2.rom",      0x08000, 0x12c4fea9, BRF_ESS | BRF_PRG }, //  1	Z80 #2 Program Code

	{ "mw-4.rom",      0x20000, 0x28a3af15, BRF_GRA },	     //  2	Characters
	{ "mw-5.rom",      0x20000, 0xffdf7e9f, BRF_GRA },	     //  3
	{ "mw-6.rom",      0x20000, 0x1ed773a3, BRF_GRA },	     //  4
	{ "mw-7.rom",      0x20000, 0x8eb7525c, BRF_GRA },	     //  5
	
	{ "mw-8.rom",      0x20000, 0xb9b92a3c, BRF_GRA },	     //  6	Sprites
	{ "mw-9.rom",      0x20000, 0x75fc3375, BRF_GRA },	     //  7
	
	{ "mw-3.rom",      0x80000, 0x110c6a68, BRF_SND },	     //  8	Samples
};

STD_ROM_PICK(Mstworld)
STD_ROM_FN(Mstworld)

static struct BurnRomInfo MarukinRomDesc[] = {
	{ "mg3-01.9d",     0x08000, 0x04357973, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "mg3-02.10d",    0x20000, 0x50d08da0, BRF_ESS | BRF_PRG }, //	 1
	
	{ "mg3-a.3k",      0x80000, 0x420f1de7, BRF_GRA },	     //  2	Characters
	{ "mg3-b.4k",      0x80000, 0xd8de13fa, BRF_GRA },	     //  3
	{ "mg3-c.6k",      0x80000, 0xfbeb66e8, BRF_GRA },	     //  4
	{ "mg3-d.7k",      0x80000, 0x8f6bd831, BRF_GRA },	     //  5
	
	{ "mg3-05.2g",     0x20000, 0x7a738d2d, BRF_GRA },	     //  6	Sprites
	{ "mg3-04.1g",     0x20000, 0x56f30515, BRF_GRA },	     //  7
	
	{ "mg3-e.1d",      0x80000, 0x106c2fa9, BRF_SND },	     //  8	Samples
};

STD_ROM_PICK(Marukin)
STD_ROM_FN(Marukin)

static struct BurnRomInfo Qtono1RomDesc[] = {
	{ "q3-05.rom",     0x08000, 0x1dd0a344, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "q3-06.rom",     0x20000, 0xbd6a2110, BRF_ESS | BRF_PRG }, //	 1
	{ "q3-07.rom",     0x20000, 0x61e53c4f, BRF_ESS | BRF_PRG }, //	 2
	
	{ "q3-08.rom",     0x20000, 0x1533b978, BRF_GRA },	     //  3	Characters
	{ "q3-09.rom",     0x20000, 0xa32db2f2, BRF_GRA },	     //  4
	{ "q3-10.rom",     0x20000, 0xed681aa8, BRF_GRA },	     //  5
	{ "q3-11.rom",     0x20000, 0x38b2fd10, BRF_GRA },	     //  6
	{ "q3-18.rom",     0x20000, 0x9e4292ac, BRF_GRA },	     //  7
	{ "q3-19.rom",     0x20000, 0xb7f6d40f, BRF_GRA },	     //  8
	{ "q3-20.rom",     0x20000, 0x6cd7f38d, BRF_GRA },	     //  9
	{ "q3-21.rom",     0x20000, 0xb4aa6b4b, BRF_GRA },	     //  10
	
	{ "q3-16.rom",     0x20000, 0x863d6836, BRF_GRA },	     //  11	Sprites
	{ "q3-17.rom",     0x20000, 0x459bf59c, BRF_GRA },	     //  12
	
	{ "q3-01.rom",     0x20000, 0x6c1be591, BRF_SND },	     //  13	Samples
};

STD_ROM_PICK(Qtono1)
STD_ROM_FN(Qtono1)

static struct BurnRomInfo QsangokuRomDesc[] = {
	{ "q4-05c.rom",    0x08000, 0xe1d010b4, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "q4-06.rom",     0x20000, 0xa0301849, BRF_ESS | BRF_PRG }, //	 1
	{ "q4-07.rom",     0x20000, 0x2941ef5b, BRF_ESS | BRF_PRG }, //	 2
	
	{ "q4-08.rom",     0x20000, 0xdc84c6cb, BRF_GRA },	     //  3	Characters
	{ "q4-09.rom",     0x20000, 0xcbb6234c, BRF_GRA },	     //  4
	{ "q4-10.rom",     0x20000, 0xc20a27a8, BRF_GRA },	     //  5
	{ "q4-11.rom",     0x20000, 0x4ff66aed, BRF_GRA },	     //  6
	{ "q4-18.rom",     0x20000, 0xca3acea5, BRF_GRA },	     //  7
	{ "q4-19.rom",     0x20000, 0x1fd92b7d, BRF_GRA },	     //  8
	{ "q4-20.rom",     0x20000, 0xb02dc6a1, BRF_GRA },	     //  9
	{ "q4-21.rom",     0x20000, 0x432b1dc1, BRF_GRA },	     //  10
	
	{ "q4-16.rom",     0x20000, 0x77342320, BRF_GRA },	     //  11	Sprites
	{ "q4-17.rom",     0x20000, 0x1275c436, BRF_GRA },	     //  12
	
	{ "q4-01.rom",     0x20000, 0x5d0d07d8, BRF_SND },	     //  13	Samples
};

STD_ROM_PICK(Qsangoku)
STD_ROM_FN(Qsangoku)

static struct BurnRomInfo BlockRomDesc[] = {
	{ "ble_05.rom",    0x08000, 0xc12e7f4c, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "ble_06.rom",    0x20000, 0xcdb13d55, BRF_ESS | BRF_PRG }, //	 1
	{ "ble_07.rom",    0x20000, 0x1d114f13, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bl_08.rom",     0x20000, 0xaa0f4ff1, BRF_GRA },	     //  3	Characters
	{ "bl_09.rom",     0x20000, 0x6fa8c186, BRF_GRA },	     //  4
	{ "bl_18.rom",     0x20000, 0xc0acafaf, BRF_GRA },	     //  5
	{ "bl_19.rom",     0x20000, 0x1ae942f5, BRF_GRA },	     //  6
	
	{ "bl_16.rom",     0x20000, 0xfadcaff7, BRF_GRA },	     //  7	Sprites
	{ "bl_17.rom",     0x20000, 0x5f8cab42, BRF_GRA },	     //  8
	
	{ "bl_01.rom",     0x20000, 0xc2ec2abb, BRF_SND },	     //  9	Samples
};

STD_ROM_PICK(Block)
STD_ROM_FN(Block)

static struct BurnRomInfo BlockjRomDesc[] = {
	{ "blj_05.rom",    0x08000, 0x3b55969a, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "ble_06.rom",    0x20000, 0xcdb13d55, BRF_ESS | BRF_PRG }, //	 1
	{ "blj_07.rom",    0x20000, 0x1723883c, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bl_08.rom",     0x20000, 0xaa0f4ff1, BRF_GRA },	     //  3	Characters
	{ "bl_09.rom",     0x20000, 0x6fa8c186, BRF_GRA },	     //  4
	{ "bl_18.rom",     0x20000, 0xc0acafaf, BRF_GRA },	     //  5
	{ "bl_19.rom",     0x20000, 0x1ae942f5, BRF_GRA },	     //  6
	
	{ "bl_16.rom",     0x20000, 0xfadcaff7, BRF_GRA },	     //  7	Sprites
	{ "bl_17.rom",     0x20000, 0x5f8cab42, BRF_GRA },	     //  8
	
	{ "bl_01.rom",     0x20000, 0xc2ec2abb, BRF_SND },	     //  9	Samples
};

STD_ROM_PICK(Blockj)
STD_ROM_FN(Blockj)

static struct BurnRomInfo BlockjoyRomDesc[] = {
	{ "ble_05.bin",    0x08000, 0xfa2a4536, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "blf_06.bin",    0x20000, 0xe114ebde, BRF_ESS | BRF_PRG }, //	 1
	{ "ble_07.rom",    0x20000, 0x1d114f13, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bl_08.rom",     0x20000, 0xaa0f4ff1, BRF_GRA },	     //  3	Characters
	{ "bl_09.rom",     0x20000, 0x6fa8c186, BRF_GRA },	     //  4
	{ "bl_18.rom",     0x20000, 0xc0acafaf, BRF_GRA },	     //  5
	{ "bl_19.rom",     0x20000, 0x1ae942f5, BRF_GRA },	     //  6
	
	{ "bl_16.rom",     0x20000, 0xfadcaff7, BRF_GRA },	     //  7	Sprites
	{ "bl_17.rom",     0x20000, 0x5f8cab42, BRF_GRA },	     //  8
	
	{ "bl_01.rom",     0x20000, 0xc2ec2abb, BRF_SND },	     //  9	Samples
};

STD_ROM_PICK(Blockjoy)
STD_ROM_FN(Blockjoy)

static struct BurnRomInfo BlockblRomDesc[] = {
	{ "m7.l6",         0x10000, 0x3b576fd9, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "m5.l3",         0x40000, 0x7c988bb7, BRF_ESS | BRF_PRG }, //	 1
	{ "m6.l5",         0x20000, 0x5768d8eb, BRF_ESS | BRF_PRG }, //	 2
	
	{ "m12.o10",       0x20000, 0x963154d9, BRF_GRA },	     //  3	Characters
	{ "m13.o14",       0x20000, 0x069480bb, BRF_GRA },	     //  4
	{ "m4.j17",        0x20000, 0x9e3b6f4f, BRF_GRA },	     //  5
	{ "m3.j20",        0x20000, 0x629d58fe, BRF_GRA },	     //  6
	
	{ "m11.o7",        0x10000, 0x255180a5, BRF_GRA },	     //  7	Sprites
	{ "m10.o5",        0x10000, 0x3201c088, BRF_GRA },	     //  8
	{ "m9.o3",         0x10000, 0x29357fe4, BRF_GRA },	     //  9
	{ "m8.o2",         0x10000, 0xabd665d1, BRF_GRA },	     //  10
	
	{ "bl_01.rom",     0x20000, 0xc2ec2abb, BRF_SND },	     //  11	Samples
};

STD_ROM_PICK(Blockbl)
STD_ROM_FN(Blockbl)

static const eeprom_interface MitchellEEPROMIntf =
{
	6,
	16,
	"0110",
	"0101",
	"0111",
	0,
	0,
	0,
	0
};

static INT32 MgakuenMemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom              = Next; Next += 0x50000;
	MSM6295ROM             = Next; Next += 0x40000;
	DrvSoundRom            = Next; Next += 0x80000;
	
	RamStart               = Next;

	DrvZ80Ram              = Next; Next += 0x01000;
	DrvPaletteRam          = Next; Next += 0x00800;
	DrvAttrRam             = Next; Next += 0x00800;
	DrvVideoRam            = Next; Next += 0x01000;
	DrvSpriteRam           = Next; Next += 0x01000;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x10000 * 8 * 8;
	DrvSprites             = Next; Next += 0x00800 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 PangMemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom              = Next; Next += 0x50000;
	DrvZ80Code             = Next; Next += 0x50000;
	DrvSoundRom            = Next; Next += 0x20000;
	
	RamStart               = Next;

	DrvZ80Ram              = Next; Next += 0x02000;
	DrvPaletteRam          = Next; Next += 0x01000;
	DrvAttrRam             = Next; Next += 0x00800;
	DrvVideoRam            = Next; Next += 0x01000;
	DrvSpriteRam           = Next; Next += 0x01000;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x8000 * 8 * 8;
	DrvSprites             = Next; Next += 0x0800 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 MahjongMemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom              = Next; Next += 0x50000;
	DrvZ80Code             = Next; Next += 0x50000;
	MSM6295ROM             = Next; Next += 0x40000;
	DrvSoundRom            = Next; Next += 0x80000;
	
	RamStart               = Next;

	DrvZ80Ram              = Next; Next += 0x02000;
	DrvPaletteRam          = Next; Next += 0x01000;
	DrvAttrRam             = Next; Next += 0x00800;
	DrvVideoRam            = Next; Next += 0x01000;
	DrvSpriteRam           = Next; Next += 0x01000;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x10000 * 8 * 8;
	DrvSprites             = Next; Next += 0x00800 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 MstworldMemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom              = Next; Next += 0x50000;
	DrvZ80Code             = Next; Next += 0x50000;
	DrvZ80Rom2             = Next; Next += 0x08000;
	MSM6295ROM             = Next; Next += 0x40000;
	DrvSoundRom            = Next; Next += 0x100000;
	
	RamStart               = Next;

	DrvZ80Ram              = Next; Next += 0x02000;
	DrvZ80Ram2             = Next; Next += 0x00800;
	DrvPaletteRam          = Next; Next += 0x01000;
	DrvAttrRam             = Next; Next += 0x00800;
	DrvVideoRam            = Next; Next += 0x01000;
	DrvSpriteRam           = Next; Next += 0x01000;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x4000 * 8 * 8;
	DrvSprites             = Next; Next += 0x0800 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	ZetOpen(0);
	DrvRomBank = 0;
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
	if (DrvHasEEPROM) {
		ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Code + 0x10000 + (DrvRomBank * 0x4000), DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
	} else {
		ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
	}
	ZetReset();
	ZetClose();
	
	BurnYM2413Reset();
	MSM6295Reset(0);
	
	if (DrvHasEEPROM) EEPROMReset();
	
	DrvPaletteRamBank = 0;
	DrvVideoBank = 0;
	DrvDialSelected = 0;
	
	return 0;
}

static INT32 MstworldDoReset()
{
	ZetOpen(0);
	DrvRomBank = 0;
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Code + 0x10000 + (DrvRomBank * 0x4000), DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
	ZetReset();
	ZetClose();
	
	ZetOpen(1);
	ZetReset();
	ZetClose();
	
	MSM6295Reset(0);
	
	DrvPaletteRamBank = 0;
	DrvVideoBank = 0;
	DrvSoundLatch = 0;
	
	return 0;
}

static UINT8 BlockDialRead(INT32 Offset)
{
	static INT32 Dir[2];
					
	if (DrvDialSelected) {
		INT32 Delta;

		Delta = ((Offset ? DrvDial2 : DrvDial1) - DrvDial[Offset]) & 0xff;
		if (Delta & 0x80) {
			Delta = (-Delta) & 0xff;
			if (Dir[Offset]) {
				Dir[Offset] = 0;
				Delta = 0;
			}
		} else {
			if (Delta > 0) {
				if (Dir[Offset] == 0) {
					Dir[Offset] = 1;
					Delta = 0;
				}
			}
		}
						
		if (Delta > 0x3f) Delta = 0x3f;
				
		return Delta << 2;
	} else {
		INT32 Res = (0xff - DrvInput[Offset + 1]) & 0xf7;
		if (Dir[Offset]) Res |= 0x08;

		return Res;
	}
}

UINT8 __fastcall MitchellZ80Read(UINT16 a)
{
	if (a >= 0xc000 && a <= 0xc7ff) {
		return DrvPaletteRam[(a - 0xc000) + (DrvPaletteRamBank ? 0x800 : 0x000)];
	}
	
	if (a >= 0xd000 && a <= 0xdfff) {
		INT32 Offset = a - 0xd000;
		if (DrvVideoBank) {
			return DrvSpriteRam[Offset];
		} else {
			return DrvVideoRam[Offset];
		}
	}
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall MitchellZ80Write(UINT16 a, UINT8 d)
{
	if (a >= 0xc000 && a <= 0xc7ff) {
		DrvPaletteRam[(a - 0xc000) + (DrvPaletteRamBank ? 0x800 : 0x000)] = d;
		return;
	}
	
	if (a >= 0xd000 && a <= 0xdfff) {
		INT32 Offset = a - 0xd000;
		if (DrvVideoBank) {
			DrvSpriteRam[Offset] = d;
		} else {
			DrvVideoRam[Offset] = d;
		}
		return;
	}
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall MitchellZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			return 0xff - DrvInput[0];
		}
		
		case 0x01: {
			switch (DrvInputType) {
				case DRV_INPUT_TYPE_MAHJONG: {
					if (DrvMahjongKeyMatrix & (0x80 >> 0)) return 0xff - DrvInput[1];
					if (DrvMahjongKeyMatrix & (0x80 >> 1)) return 0xff - DrvInput[2];
					if (DrvMahjongKeyMatrix & (0x80 >> 2)) return 0xff - DrvInput[4];
					if (DrvMahjongKeyMatrix & (0x80 >> 3)) return 0xff - DrvInput[5];
					if (DrvMahjongKeyMatrix & (0x80 >> 4)) return 0xff - DrvInput[6];
				}
				
				case DRV_INPUT_TYPE_BLOCK: {
					return BlockDialRead(0);
				}
			}
			
			return 0xff - DrvInput[1];
		}
		
		case 0x02: {
			switch (DrvInputType) {
				case DRV_INPUT_TYPE_MAHJONG: {
					if (DrvMahjongKeyMatrix & (0x80 >> 0)) return 0xff - DrvInput[7];
					if (DrvMahjongKeyMatrix & (0x80 >> 1)) return 0xff - DrvInput[8];
					if (DrvMahjongKeyMatrix & (0x80 >> 2)) return 0xff - DrvInput[9];
					if (DrvMahjongKeyMatrix & (0x80 >> 3)) return 0xff - DrvInput[10];
					if (DrvMahjongKeyMatrix & (0x80 >> 4)) return 0xff - DrvInput[11];
				}
				
				case DRV_INPUT_TYPE_BLOCK: {
					return BlockDialRead(1);
				}
			}
			
			return 0xff - DrvInput[2];
		}
		
		case 0x03: {
			return DrvDip[0];
		}
		
		case 0x04: {
			return DrvDip[1];
		}
		
		case 0x05: {
			INT32 Bit = DrvHasEEPROM ? (EEPROMRead() & 0x01) << 7 : 0x80;
			if (DrvInput5Toggle) {
				Bit |= 0x01;
			} else {
				Bit |= 0x08;
			}
			
			if (DrvPort5Kludge) Bit ^= 0x08;
			
			return ((0xff - DrvInput[3]) & 0x76) | Bit;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall MitchellZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			DrvFlipScreen = d & 0x04;
			if (DrvOkiBank != (d & 0x10)) {
				DrvOkiBank = d & 0x10;
				if (DrvOkiBank) {
					memcpy(MSM6295ROM, DrvSoundRom + 0x40000, 0x40000);
				} else {
					memcpy(MSM6295ROM, DrvSoundRom + 0x00000, 0x40000);
				}
			}
			DrvPaletteRamBank = d & 0x20;
			return;
		}
		
		case 0x01: {
			switch (DrvInputType) {
				case DRV_INPUT_TYPE_MAHJONG: {
					DrvMahjongKeyMatrix = d;
					return;
				}
				
				case DRV_INPUT_TYPE_BLOCK: {
					if (d == 0x08) {
						DrvDial[0] = DrvDial1;
						DrvDial[1] = DrvDial2;
					} else {
						if (d == 0x80) {
							DrvDialSelected = 0;
						} else {
							DrvDialSelected = 1;
						}
					}
					return;
				}
			}
			
			return;
		}
		
		case 0x02: {
			DrvRomBank = d & 0x0f;
			ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
			if (DrvHasEEPROM) {
				ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Code + 0x10000 + (DrvRomBank * 0x4000), DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
			} else {
				ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
			}
			return;
		}
		
		case 0x03: {
			BurnYM2413Write(1, d);
			return;
		}
		
		case 0x04: {
			BurnYM2413Write(0, d);
			return;
		}
		
		case 0x05: {
			MSM6295Command(0, d);
			return;
		}
		
		case 0x06:{
			// nop
			return;
		}
		
		case 0x07: {
			DrvVideoBank = d;
			return;
		}
		
		case 0x08 :{
			if (DrvHasEEPROM) EEPROMSetCSLine(d ? 0 : 1);
			return;
		}
		
		case 0x10 :{
			if (DrvHasEEPROM) EEPROMSetClockLine(d ? 0 : 1);
			return;
		}
		
		case 0x18 :{
			if (DrvHasEEPROM) EEPROMWriteBit(d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall MstworldZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			return 0xff - DrvInput[0];
		}
		
		case 0x01: {
			return 0xfe - DrvInput[1];
		}
		
		case 0x02: {
			return 0xfe - DrvInput[2];
		}
		
		case 0x03: {
			return DrvDip[0];
		}
		
		case 0x05: {
			return 0xff - DrvInput[3];
		}
		
		case 0x06: {
			return 0xff;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0xff;
}

void __fastcall MstworldZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			DrvFlipScreen = d & 0x04;
			DrvPaletteRamBank = d & 0x20;
			return;
		}
		
		case 0x02: {
			DrvRomBank = d & 0x0f;
			ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Code + 0x10000 + (DrvRomBank * 0x4000), DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
			return;
		}
		
		case 0x03: {
			DrvSoundLatch = d;
			ZetClose();
			ZetOpen(1);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			ZetOpen(0);
		}
		
		case 0x06: {
			// nop
			return;
		}
		
		case 0x07: {
			DrvVideoBank = d & 0x01;
			return;
		}
		
		case 0x08:
		case 0x10:
		case 0x18: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall MstworldSoundZ80Read(UINT16 a)
{
	switch (a) {
		case 0x9800: {
			return MSM6295ReadStatus(0);
		}
		
		case 0xa000: {
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return DrvSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall MstworldSoundZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0x9000: {
			DrvOkiBank = d & 0x03;
			memcpy(MSM6295ROM, DrvSoundRom + (DrvOkiBank * 0x40000), 0x40000);
			return;
		}
		
		case 0x9800: {
			MSM6295Command(0, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Write => %04X, %02X\n"), a, d);
		}
	}
}

// Kabuki - we use the module from the CPS-1 Q-Sound games
extern void kabuki_decode(UINT8 *src, UINT8 *dest_op, UINT8 *dest_data, INT32 base_addr, INT32 length, INT32 swap_key1, INT32 swap_key2, INT32 addr_key, INT32 xor_key);

static void mitchell_decode(INT32 swap_key1, INT32 swap_key2, INT32 addr_key, INT32 xor_key)
{
	UINT8 *rom = DrvZ80Rom;
	UINT8 *decrypt = DrvZ80Code;
	INT32 numbanks = (0x50000 - 0x10000) / 0x4000;
	INT32 i;

	kabuki_decode(rom, decrypt, rom, 0x0000, 0x8000, swap_key1, swap_key2, addr_key, xor_key);

	rom += 0x10000;
	decrypt += 0x10000;
	for (i = 0; i < numbanks; i++)
		kabuki_decode(rom + i * 0x4000,decrypt + i * 0x4000, rom + i * 0x4000, 0x8000, 0x4000, swap_key1, swap_key2, addr_key, xor_key);
}

static void mgakuen2_decode() { mitchell_decode(0x76543210, 0x01234567, 0xaa55, 0xa5); }
static void pang_decode()     { mitchell_decode(0x01234567, 0x76543210, 0x6548, 0x24); }
static void cworld_decode()   { mitchell_decode(0x04152637, 0x40516273, 0x5751, 0x43); }
static void hatena_decode()   { mitchell_decode(0x45670123, 0x45670123, 0x5751, 0x43); }
static void spang_decode()    { mitchell_decode(0x45670123, 0x45670123, 0x5852, 0x43); }
static void spangj_decode()   { mitchell_decode(0x45123670, 0x67012345, 0x55aa, 0x5a); }
static void sbbros_decode()   { mitchell_decode(0x45670123, 0x45670123, 0x2130, 0x12); }
static void marukin_decode()  { mitchell_decode(0x54321076, 0x54321076, 0x4854, 0x4f); }
static void qtono1_decode()   { mitchell_decode(0x12345670, 0x12345670, 0x1111, 0x11); }
static void qsangoku_decode() { mitchell_decode(0x23456701, 0x23456701, 0x1828, 0x18); }
static void block_decode()    { mitchell_decode(0x02461357, 0x64207531, 0x0002, 0x01); }

static INT32 CharPlaneOffsets[4]           = { 0x400004, 0x400000, 4, 0 };
static INT32 CharXOffsets[8]               = { 0, 1, 2, 3, 8, 9, 10, 11 };
static INT32 CharYOffsets[8]               = { 0, 16, 32, 48, 64, 80, 96, 112 };
static INT32 MahjongCharPlaneOffsets[4]    = { 12, 8, 4, 0 };
static INT32 MahjongCharXOffsets[8]        = { 0, 1, 2, 3, 16, 17, 18, 19 };
static INT32 MahjongCharYOffsets[8]        = { 0, 32, 64, 96, 128, 160, 192, 224 };
static INT32 SpritePlaneOffsets[4]         = { 0x100004, 0x100000, 4, 0 };
static INT32 SpriteXOffsets[16]            = { 0, 1, 2, 3, 8, 9, 10, 11, 256, 257, 258, 259, 264, 265, 266, 267 };
static INT32 SpriteYOffsets[16]            = { 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240 };
static INT32 MstworldCharPlaneOffsets[4]   = { 0x200000, 0x300000, 0, 0x100000 };
static INT32 MstworldCharXOffsets[8]       = { 7, 6, 5, 4, 3, 2, 1, 0 };
static INT32 MstworldCharYOffsets[8]       = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 MstworldSpritePlaneOffsets[4] = { 4, 0, 0x100004, 0x100000 };
static INT32 MstworldSpriteXOffsets[16]    = { 0, 1, 2, 3, 8, 9, 10, 11, 256, 257, 258, 259, 264, 265, 266, 267 };
static INT32 MstworldSpriteYOffsets[16]    = { 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240 };

static void MitchellMachineInit()
{
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(MitchellZ80Read);
	ZetSetWriteHandler(MitchellZ80Write);
	ZetSetInHandler(MitchellZ80PortRead);
	ZetSetOutHandler(MitchellZ80PortWrite);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom  + 0x00000                     );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Code + 0x00000, DrvZ80Rom + 0x00000);
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom  + 0x10000                     );
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Code + 0x10000, DrvZ80Rom + 0x10000);
	ZetMapArea(0xc800, 0xcfff, 0, DrvAttrRam                               );
	ZetMapArea(0xc800, 0xcfff, 1, DrvAttrRam                               );
	ZetMapArea(0xc800, 0xcfff, 2, DrvAttrRam                               );
	ZetMapArea(0xe000, 0xffff, 0, DrvZ80Ram                                );
	ZetMapArea(0xe000, 0xffff, 1, DrvZ80Ram                                );
	ZetMapArea(0xe000, 0xffff, 2, DrvZ80Ram                                );
	ZetMemEnd();
	ZetClose();
	
	BurnYM2413Init(4000000, 1.0);
	BurnYM2413DecreaseVolume(40);
	MSM6295Init(0, 1000000 / 132, 10.0, 1);
	MSM6295ROM = DrvSoundRom;
	
	EEPROMInit(&MitchellEEPROMIntf);
	DrvHasEEPROM = 1;

	GenericTilesInit();
	
	DrvTileMask = 0x7fff;
	DrvNumColours = 0x800;
}

static void MahjongMachineInit()
{
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(MitchellZ80Read);
	ZetSetWriteHandler(MitchellZ80Write);
	ZetSetInHandler(MitchellZ80PortRead);
	ZetSetOutHandler(MitchellZ80PortWrite);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom  + 0x00000                     );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Code + 0x00000, DrvZ80Rom + 0x00000);
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom  + 0x10000                     );
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Code + 0x10000, DrvZ80Rom + 0x10000);
	ZetMapArea(0xc800, 0xcfff, 0, DrvAttrRam                               );
	ZetMapArea(0xc800, 0xcfff, 1, DrvAttrRam                               );
	ZetMapArea(0xc800, 0xcfff, 2, DrvAttrRam                               );
	ZetMapArea(0xe000, 0xffff, 0, DrvZ80Ram                                );
	ZetMapArea(0xe000, 0xffff, 1, DrvZ80Ram                                );
	ZetMapArea(0xe000, 0xffff, 2, DrvZ80Ram                                );
	ZetMemEnd();
	ZetClose();
	
	BurnYM2413Init(4000000, 1.0);
	BurnYM2413DecreaseVolume(40);
	MSM6295Init(0, 990000 / 132, 10.0, 1);
	memcpy(MSM6295ROM, DrvSoundRom, 0x40000);
	
	EEPROMInit(&MitchellEEPROMIntf);
	DrvHasEEPROM = 1;

	GenericTilesInit();
	
	DrvTileMask = 0xffff;
	DrvNumColours = 0x800;
}

static INT32 MgakuenInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	MgakuenMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MgakuenMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x200000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x000000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x010000,  1, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x180000,  5, 1); if (nRet != 0) return 1;
	GfxDecode(0x10000, 4, 8, 8, MahjongCharPlaneOffsets, MahjongCharXOffsets, MahjongCharYOffsets, 0x100, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x020000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(0x00800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x000000, 8, 1); if (nRet != 0) return 1;
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetInHandler(MitchellZ80PortRead);
	ZetSetOutHandler(MitchellZ80PortWrite);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom + 0x00000);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom + 0x00000);
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom + 0x10000);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom + 0x10000);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvPaletteRam      );
	ZetMapArea(0xc000, 0xc7ff, 1, DrvPaletteRam      );
	ZetMapArea(0xc000, 0xc7ff, 2, DrvPaletteRam      );
	ZetMapArea(0xc800, 0xcfff, 0, DrvAttrRam         );
	ZetMapArea(0xc800, 0xcfff, 1, DrvAttrRam         );
	ZetMapArea(0xc800, 0xcfff, 2, DrvAttrRam         );
	ZetMapArea(0xd000, 0xdfff, 0, DrvVideoRam        );
	ZetMapArea(0xd000, 0xdfff, 1, DrvVideoRam        );
	ZetMapArea(0xd000, 0xdfff, 2, DrvVideoRam        );
	ZetMapArea(0xe000, 0xefff, 0, DrvZ80Ram          );
	ZetMapArea(0xe000, 0xefff, 1, DrvZ80Ram          );
	ZetMapArea(0xe000, 0xefff, 2, DrvZ80Ram          );
	ZetMapArea(0xf000, 0xffff, 0, DrvSpriteRam       );
	ZetMapArea(0xf000, 0xffff, 1, DrvSpriteRam       );
	ZetMapArea(0xf000, 0xffff, 2, DrvSpriteRam       );
	ZetMemEnd();
	ZetClose();
	
	BurnYM2413Init(4000000, 1.0);
	BurnYM2413DecreaseVolume(40);
	MSM6295Init(0, 990000 / 132, 10.0, 1);
	memcpy(MSM6295ROM, DrvSoundRom, 0x40000);
	
	GenericTilesInit();
	
	DrvTileMask = 0xffff;
	DrvNumColours = 0x400;
	DrvHasEEPROM = 0;
	
	DrvInputType = DRV_INPUT_TYPE_MAHJONG;
	
	DrvDoReset();

	return 0;
}

static INT32 Mgakuen2Init()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	MahjongMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MahjongMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x200000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x000000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x010000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x030000,  2, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x180000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(0x10000, 4, 8, 8, MahjongCharPlaneOffsets, MahjongCharXOffsets, MahjongCharYOffsets, 0x100, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x020000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x00800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x000000, 9, 1); if (nRet != 0) return 1;
	
	mgakuen2_decode();
	
	MahjongMachineInit();
	
	DrvInputType = DRV_INPUT_TYPE_MAHJONG;
	
	DrvPort5Kludge = 1;
	
	DrvDoReset();

	return 0;
}

static INT32 PkladiesInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	MahjongMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MahjongMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x200000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x000000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x010000,  1, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x000001,  3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000,  4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100001,  5, 2); if (nRet != 0) return 1;
	GfxDecode(0x10000, 4, 8, 8, MahjongCharPlaneOffsets, MahjongCharXOffsets, MahjongCharYOffsets, 0x100, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x020000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(0x00800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x000000, 8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvSoundRom + 0x020000, 9, 1); if (nRet != 0) return 1;
	
	mgakuen2_decode();
	
	MahjongMachineInit();
	
	DrvInputType = DRV_INPUT_TYPE_MAHJONG;
	
	DrvDoReset();

	return 0;
}

static INT32 DokabenInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x30000,  2, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	
	mgakuen2_decode();
	
	MitchellMachineInit();
	
	DrvDoReset();

	return 0;
}

static INT32 PangInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  1, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  5, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000,  8, 1); if (nRet != 0) return 1;
	
	pang_decode();
	
	MitchellMachineInit();
	
	DrvDoReset();

	return 0;
}

static INT32 PangbInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);
	
	nRet = BurnLoadRom(DrvZ80Code + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Code + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  2, 1); if (nRet != 0) return 1;
	memcpy(DrvZ80Rom + 0x0000, DrvZ80Code + 0x8000, 0x8000);
	memset(DrvZ80Code + 0x8000, 0, 0x8000);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	
	MitchellMachineInit();
	
	DrvDoReset();

	return 0;
}

static INT32 PangboldInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);
	
	nRet = BurnLoadRom(DrvZ80Code + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Code + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  2, 1); if (nRet != 0) return 1;
	memcpy(DrvZ80Rom + 0x0000, DrvZ80Code + 0x8000, 0x8000);
	memset(DrvZ80Code + 0x8000, 0, 0x8000);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	
	MitchellMachineInit();
	
	DrvDoReset();

	return 0;
}

static INT32 CworldInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x30000,  2, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xc0000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xe0000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000, 13, 1); if (nRet != 0) return 1;
	
	cworld_decode();
	
	MitchellMachineInit();
	
	DrvDoReset();

	return 0;
}

static INT32 HatenaInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x30000,  2, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xc0000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xe0000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000, 13, 1); if (nRet != 0) return 1;
	
	hatena_decode();
	
	MitchellMachineInit();
	
	DrvDoReset();

	return 0;
}

static INT32 SpangInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x30000,  2, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 10, 1); if (nRet != 0) return 1;
	
	spang_decode();
	
	MitchellMachineInit();
	
	if (!EEPROMAvailable()) EEPROMFill(DrvTempRom, 0, 128);
	
	BurnFree(DrvTempRom);
	
	DrvNVRamSize = 0x0080;
	DrvNVRamAddress = 0x0000;
	
	DrvDoReset();

	return 0;
}

static INT32 SpangjInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x30000,  2, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 10, 1); if (nRet != 0) return 1;
	
	spangj_decode();
	
	MitchellMachineInit();
	
	if (!EEPROMAvailable()) EEPROMFill(DrvTempRom, 0, 128);

	BurnFree(DrvTempRom);
	
	DrvNVRamSize = 0x0080;
	DrvNVRamAddress = 0x0000;
	
	DrvDoReset();

	return 0;
}

static INT32 SbbrosInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x30000,  2, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 10, 1); if (nRet != 0) return 1;
	
	sbbros_decode();
	
	MitchellMachineInit();
	
	if (!EEPROMAvailable()) EEPROMFill(DrvTempRom, 0, 128);

	BurnFree(DrvTempRom);
	
	DrvNVRamSize = 0x0080;
	DrvNVRamAddress = 0x0000;
	
	DrvDoReset();

	return 0;
}

static INT32 MstworldInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	MstworldMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MstworldMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000,  1, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0x00, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000,  5, 1); if (nRet != 0) return 1;
	for (INT32 i = 0; i < 0x80000; i++) DrvTempRom[i] ^= 0xff;
	GfxDecode(0x4000, 4, 8, 8, MstworldCharPlaneOffsets, MstworldCharXOffsets, MstworldCharYOffsets, 0x40, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0x00, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  7, 1); if (nRet != 0) return 1;
	for (INT32 i = 0; i < 0x40000; i++) DrvTempRom[i] ^= 0xff;
	GfxDecode(0x0800, 4, 16, 16, MstworldSpritePlaneOffsets, MstworldSpriteXOffsets, MstworldSpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  8, 1); if (nRet != 0) return 1;
	memcpy(DrvSoundRom + 0x000000, DrvTempRom + 0x000000, 0x20000);
	memcpy(DrvSoundRom + 0x020000, DrvTempRom + 0x000000, 0x20000);
	memcpy(DrvSoundRom + 0x040000, DrvTempRom + 0x000000, 0x20000);
	memcpy(DrvSoundRom + 0x060000, DrvTempRom + 0x020000, 0x20000);
	memcpy(DrvSoundRom + 0x080000, DrvTempRom + 0x000000, 0x20000);
	memcpy(DrvSoundRom + 0x0a0000, DrvTempRom + 0x040000, 0x20000);
	memcpy(DrvSoundRom + 0x0c0000, DrvTempRom + 0x000000, 0x20000);
	memcpy(DrvSoundRom + 0x0e0000, DrvTempRom + 0x060000, 0x20000);
	
	static const INT32 tablebank[] = {
		0,  0,
		1,  1,
		-1, -1,
		-1, -1,
		10,  4,
		5, 13,
		7, 17,
		21,  2,
		18,  9,
		15,  3,
		6, 11,
		19,  8,
		-1, -1,
		-1, -1,
		-1, -1,
		-1, -1,
		20, 20,
		14, 14,
		16, 16,
		12, 12,
	};

	memcpy(DrvTempRom, DrvZ80Rom, 0x80000);
	for (INT32 x = 0; x < 40; x += 2) {
		if (tablebank[x] != -1) {
			memcpy(&DrvZ80Rom[(x / 2) * 0x4000], &DrvTempRom[tablebank[x] * 0x4000], 0x4000);
			memcpy(&DrvZ80Code[(x / 2) * 0x4000], &DrvTempRom[tablebank[x + 1] * 0x4000], 0x4000);
		}
	}
	
	BurnFree(DrvTempRom);
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(MitchellZ80Read);
	ZetSetWriteHandler(MitchellZ80Write);
	ZetSetInHandler(MstworldZ80PortRead);
	ZetSetOutHandler(MstworldZ80PortWrite);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom  + 0x00000                     );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Code + 0x00000, DrvZ80Rom + 0x00000);
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom  + 0x10000                     );
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Code + 0x10000, DrvZ80Rom + 0x10000);
	ZetMapArea(0xc800, 0xcfff, 0, DrvAttrRam                               );
	ZetMapArea(0xc800, 0xcfff, 1, DrvAttrRam                               );
	ZetMapArea(0xc800, 0xcfff, 2, DrvAttrRam                               );
	ZetMapArea(0xe000, 0xffff, 0, DrvZ80Ram                                );
	ZetMapArea(0xe000, 0xffff, 1, DrvZ80Ram                                );
	ZetMapArea(0xe000, 0xffff, 2, DrvZ80Ram                                );
	ZetMemEnd();
	ZetClose();
	
	ZetOpen(1);
	ZetSetReadHandler(MstworldSoundZ80Read);
	ZetSetWriteHandler(MstworldSoundZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom2);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom2);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80Ram2);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80Ram2);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80Ram2);
	ZetMemEnd();
	ZetClose();
	
	MSM6295Init(0, 990000 / 132, 25.0, 0);
	memcpy(MSM6295ROM, DrvSoundRom, 0x40000);
	
	DrvHasEEPROM = 0;

	GenericTilesInit();
	
	DrvTileMask = 0x3fff;
	DrvNumColours = 0x800;
	
	MstworldDoReset();

	return 0;
}

static INT32 MarukinInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	MahjongMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MahjongMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x200000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x000000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x010000,  1, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x180000,  5, 1); if (nRet != 0) return 1;
	GfxDecode(0x10000, 4, 8, 8, MahjongCharPlaneOffsets, MahjongCharXOffsets, MahjongCharYOffsets, 0x100, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x020000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(0x00800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x000000,  8, 1); if (nRet != 0) return 1;
	
	marukin_decode();
	
	MahjongMachineInit();
	
	DrvInputType = DRV_INPUT_TYPE_MAHJONG;
	
	DrvDoReset();

	return 0;
}

static INT32 Qtono1Init()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x30000,  2, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xc0000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xe0000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000, 13, 1); if (nRet != 0) return 1;
	
	qtono1_decode();
	
	MitchellMachineInit();
	
	DrvDoReset();

	return 0;
}

static INT32 QsangokuInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x30000,  2, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xc0000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xe0000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000, 13, 1); if (nRet != 0) return 1;
	
	qsangoku_decode();
	
	MitchellMachineInit();
	
	DrvDoReset();

	return 0;
}

static INT32 BlockInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x30000,  2, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	
	block_decode();
	
	MitchellMachineInit();
	
	DrvInputType = DRV_INPUT_TYPE_BLOCK;
	
	DrvNVRamSize = 0x0080;
	DrvNVRamAddress = 0x1f80;
	
	DrvDoReset();

	return 0;
}

static INT32 BlockjoyInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);

	nRet = BurnLoadRom(DrvZ80Rom  + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x30000,  2, 1); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000, 9, 1); if (nRet != 0) return 1;
	
	block_decode();
	
	MitchellMachineInit();
	
	DrvDoReset();

	return 0;
}

static INT32 BlockblInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	PangMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	PangMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x100000);
	
	nRet = BurnLoadRom(DrvZ80Code + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Code + 0x10000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom  + 0x30000,  2, 1); if (nRet != 0) return 1;
	memcpy(DrvZ80Rom + 0x00000, DrvZ80Code + 0x08000, 0x08000);
	memset(DrvZ80Code + 0x08000, 0, 0x08000);
	memcpy(DrvZ80Rom + 0x10000, DrvZ80Code + 0x30000, 0x20000);
	memset(DrvZ80Code + 0x30000, 0, 0x20000);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x80000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0xa0000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(0x8000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0xff, 0x100000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	BurnFree(DrvTempRom);
	
	nRet = BurnLoadRom(DrvSoundRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	
	MitchellMachineInit();
	
	DrvInputType = DRV_INPUT_TYPE_BLOCK;
	
	DrvNVRamSize = 0x0080;
	DrvNVRamAddress = 0x1f80;
	
	DrvDoReset();

	return 0;
}

static INT32 CommonExit()
{
	ZetExit();
		
	MSM6295Exit(0);
	if (DrvHasEEPROM) EEPROMExit();
	
	GenericTilesExit();
	
	BurnFree(Mem);
	
	DrvRomBank = 0;
	DrvPaletteRamBank = 0;
	DrvOkiBank = 0;
	DrvFlipScreen = 0;
	DrvVideoBank = 0;
	DrvInputType = 0;
	DrvMahjongKeyMatrix = 0;
	DrvTileMask = 0;
	DrvInput5Toggle = 0;
	DrvPort5Kludge = 0;
	DrvHasEEPROM = 0;
	DrvNumColours = 0;
	DrvNVRamSize = 0;
	DrvNVRamAddress = 0;
	DrvDialSelected = 0;
	DrvSoundLatch = 0;
	
	return 0;
}

static INT32 DrvExit()
{
	BurnYM2413Exit();

	return CommonExit();
}

static inline UINT8 pal4bit(UINT8 bits)
{
	bits &= 0x0f;
	return (bits << 4) | bits;
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal4bit(nColour >> 8);
	g = pal4bit(nColour >> 4);
	b = pal4bit(nColour >> 0);

	return BurnHighCol(r, g, b, 0);
}

static void DrvCalcPalette()
{
	for (INT32 i = 0; i < DrvNumColours * 2; i += 2) {
		INT32 Val = DrvPaletteRam[i & ~1] + (DrvPaletteRam[i | 1] << 8);
		
		DrvPalette[i >> 1] = CalcCol(Val);
	}
}

static void DrvRenderBgLayer()
{
	INT32 mx, my, Code, Attr, Colour, x, y, TileIndex = 0, xFlip;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Attr = DrvAttrRam[TileIndex];
			Code = DrvVideoRam[(2 * TileIndex) + 0] + (DrvVideoRam[(2 * TileIndex) + 1] << 8);
			Colour = Attr & (DrvNumColours == 0x800 ? 0x7f : 0x3f);
			xFlip = (Attr & 0x80);
			
			x = 8 * mx;
			y = 8 * my;
			
			if (DrvFlipScreen) {
				x = 504 - x;
				y = 248 - y;
				xFlip = !xFlip;
			}
			
			x -= 64;
			y -= 8;
			
			Code &= DrvTileMask;
			
			if (DrvFlipScreen) {
				if (x > 8 && x < (nScreenWidth - 8) && y > 8 && y < (nScreenHeight - 8)) {
					if (xFlip) {
						Render8x8Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 15, 0, DrvChars);
					} else {
						Render8x8Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 15, 0, DrvChars);
					}
				} else {
					if (xFlip) {
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 15, 0, DrvChars);
					} else {
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 15, 0, DrvChars);
					}
				}
			} else {
				if (x > 8 && x < (nScreenWidth - 8) && y > 8 && y < (nScreenHeight - 8)) {
					if (xFlip) {
						Render8x8Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 15, 0, DrvChars);
					} else {
						Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 15, 0, DrvChars);
					}
				} else {
					if (xFlip) {
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 15, 0, DrvChars);
					} else {
						Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 15, 0, DrvChars);
					}
				}
			}

			TileIndex++;
		}
	}
}

static void DrvRenderSpriteLayer()
{
	INT32 sx, sy;

	for (INT32 Offset = 0x1000 - 0x40; Offset >= 0; Offset -= 0x20) {
		INT32 Code = DrvSpriteRam[Offset + 0];
		INT32 Attr = DrvSpriteRam[Offset + 1];
		INT32 Colour = Attr & 0x0f;
		sx = DrvSpriteRam[Offset + 3] + ((Attr & 0x10) << 4);
		sy = ((DrvSpriteRam[Offset + 2] + 8) & 0xff) - 8;
		Code += (Attr & 0xe0) << 3;

		if (DrvFlipScreen) {
			sx = 496 - sx;
			sy = 240 - sy;
		}
		
		sx -= 64;
		sy -= 8;
		
		if (DrvFlipScreen) {
			if (sx > 16 && sx < (nScreenWidth - 16) && sy > 16 && sy < (nScreenHeight - 16)) {
				Render16x16Tile_Mask_FlipXY(pTransDraw, Code, sx, sy, Colour, 4, 15, 0, DrvSprites);
			} else {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx, sy, Colour, 4, 15, 0, DrvSprites);
			}
		} else {
			if (sx > 16 && sx < (nScreenWidth - 16) && sy > 16 && sy < (nScreenHeight - 16)) {
				Render16x16Tile_Mask(pTransDraw, Code, sx, sy, Colour, 4, 15, 0, DrvSprites);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, sy, Colour, 4, 15, 0, DrvSprites);
			}
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	DrvRenderBgLayer();
	DrvRenderSpriteLayer();	 
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 10;
	INT32 nSoundBufferPos = 0;

	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	nCyclesTotal[0] = 8000000 / 60;
	nCyclesDone[0] = 0;
	
	DrvInput5Toggle = 0;
	
	ZetNewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == 4) {
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			nCyclesDone[nCurrentCPU] += ZetRun(500);
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		}
		if (i == 7) DrvInput5Toggle = 1;
		if (i == 9) {
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			nCyclesDone[nCurrentCPU] += ZetRun(500);
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		}
		ZetClose();
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2413Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			BurnYM2413Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
		}
	}

	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 MstworldFrame()
{
	INT32 nInterleave = 10;
	INT32 nSoundBufferPos = 0;

	if (DrvReset) MstworldDoReset();

	DrvMakeInputs();

	nCyclesTotal[0] = 24000000 / 60;
	nCyclesTotal[1] = 6000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	ZetNewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == 9) {
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			nCyclesDone[nCurrentCPU] += ZetRun(500);
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		}
		ZetClose();
		
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		ZetClose();
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			MSM6295Render(0, pSoundBuf, nSegmentLength);
		}
	}

	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029707;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_NVRAM && DrvNVRamSize) {
		memset(&ba, 0, sizeof(ba));
		ba.Data = DrvZ80Ram + DrvNVRamAddress;
		ba.nLen = DrvNVRamSize;
		ba.szName = "NVRam";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);
		BurnYM2413Scan(nAction);
		MSM6295Scan(0, nAction);
		
		if (DrvHasEEPROM) EEPROMScan(nAction, pnMin);
		
		SCAN_VAR(nCyclesDone[0]);
		SCAN_VAR(nCyclesDone[1]);
		SCAN_VAR(DrvRomBank);
		SCAN_VAR(DrvPaletteRamBank);
		SCAN_VAR(DrvOkiBank);
		SCAN_VAR(DrvFlipScreen);
		SCAN_VAR(DrvVideoBank);
		SCAN_VAR(DrvInput5Toggle);
		SCAN_VAR(DrvDialSelected);
		SCAN_VAR(DrvDial[0]);
		SCAN_VAR(DrvDial[1]);
		SCAN_VAR(DrvDial1);
		SCAN_VAR(DrvDial2);
		SCAN_VAR(DrvMahjongKeyMatrix);
	}
	
	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
		if (DrvHasEEPROM) {
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Code + 0x10000 + (DrvRomBank * 0x4000), DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
		} else {
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
		}
		ZetClose();
		
		if (DrvOkiBank) {
			memcpy(MSM6295ROM, DrvSoundRom + 0x40000, 0x40000);
		}
	}

	return 0;
}

static INT32 MstworldScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029707;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);
		MSM6295Scan(0, nAction);
		
		SCAN_VAR(nCyclesDone[0]);
		SCAN_VAR(nCyclesDone[1]);
		SCAN_VAR(DrvRomBank);
		SCAN_VAR(DrvPaletteRamBank);
		SCAN_VAR(DrvOkiBank);
		SCAN_VAR(DrvFlipScreen);
		SCAN_VAR(DrvVideoBank);
		SCAN_VAR(DrvSoundLatch);
	}
	
	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
		ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Code + 0x10000 + (DrvRomBank * 0x4000), DrvZ80Rom + 0x10000 + (DrvRomBank * 0x4000));
		ZetClose();
		
		memcpy(MSM6295ROM, DrvSoundRom + (DrvOkiBank * 0x40000), 0x40000);
	}

	return 0;
}

struct BurnDriver BurnDrvMgakuen = {
	"mgakuen", NULL, NULL, NULL, "1988",
	"Mahjong Gakuen\0", NULL, "Yuga", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MAHJONG, 0,
	NULL, MgakuenRomInfo, MgakuenRomName, NULL, NULL, MgakuenInputInfo, MgakuenDIPInfo,
	MgakuenInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 384, 240, 4, 3
};

struct BurnDriver BurnDrvSeventoitsu = {
	"7toitsu", "mgakuen", NULL, NULL, "1988",
	"Chi-Toitsu\0", NULL, "Yuga", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAHJONG, 0,
	NULL, SeventoitsuRomInfo, SeventoitsuRomName, NULL, NULL, MgakuenInputInfo, MgakuenDIPInfo,
	MgakuenInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 384, 240, 4, 3
};

struct BurnDriver BurnDrvMgakuen2 = {
	"mgakuen2", NULL, NULL, NULL, "1989",
	"Mahjong Gakuen 2 Gakuen-chou no Fukushuu\0", NULL, "Face", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MAHJONG, 0,
	NULL, Mgakuen2RomInfo, Mgakuen2RomName, NULL, NULL, MarukinInputInfo, NULL,
	Mgakuen2Init, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvPkladies = {
	"pkladies", NULL, NULL, NULL, "1989",
	"Poker Ladies\0", NULL, "Mitchell", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MAHJONG, 0,
	NULL, PkladiesRomInfo, PkladiesRomName, NULL, NULL, PkladiesInputInfo, NULL,
	PkladiesInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvPkladiesl = {
	"pkladiesl", "pkladies", NULL, NULL, "1989",
	"Poker Ladies (Leprechaun ver. 510)\0", NULL, "Leprechaun", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAHJONG, 0,
	NULL, PkladieslRomInfo, PkladieslRomName, NULL, NULL, PkladiesInputInfo, NULL,
	PkladiesInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvPkladiesla = {
	"pkladiesla", "pkladies", NULL, NULL, "1989",
	"Poker Ladies (Leprechaun ver. 401)\0", NULL, "Leprechaun", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAHJONG, 0,
	NULL, PkladieslaRomInfo, PkladieslaRomName, NULL, NULL, PkladiesInputInfo, NULL,
	PkladiesInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvDokaben = {
	"dokaben", NULL, NULL, NULL, "1989",
	"Dokaben (Japan)\0", NULL, "Dokaben", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_SPORTSMISC, 0,
	NULL, DokabenRomInfo, DokabenRomName, NULL, NULL, PangInputInfo, NULL,
	DokabenInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvPang = {
	"pang", NULL, NULL, NULL, "1989",
	"Pang (World)\0", NULL, "Mitchell", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_PUZZLE, 0,
	NULL, PangRomInfo, PangRomName, NULL, NULL, PangInputInfo, NULL,
	PangInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvBbros = {
	"bbros", "pang", NULL, NULL, "1989",
	"Buster Bros. (US)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PUZZLE, 0,
	NULL, BbrosRomInfo, BbrosRomName, NULL, NULL, PangInputInfo, NULL,
	PangInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvPompingw = {
	"pompingw", "pang", NULL, NULL, "1989",
	"Pomping World (Japan)\0", NULL, "Mitchell", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PUZZLE, 0,
	NULL, PompingwRomInfo, PompingwRomName, NULL, NULL, PangInputInfo, NULL,
	PangInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvPangb = {
	"pangb", "pang", NULL, NULL, "1989",
	"Pang (bootleg, set 1)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_PUZZLE, 0,
	NULL, PangbRomInfo, PangbRomName, NULL, NULL, PangInputInfo, NULL,
	PangbInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvPangbold = {
	"pangbold", "pang", NULL, NULL, "1989",
	"Pang (bootleg, set 2)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_PUZZLE, 0,
	NULL, PangboldRomInfo, PangboldRomName, NULL, NULL, PangInputInfo, NULL,
	PangboldInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvCworld = {
	"cworld", NULL, NULL, NULL, "1990",
	"Capcom World (Japan)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_QUIZ, 0,
	NULL, CworldRomInfo, CworldRomName, NULL, NULL, Qtono1InputInfo, NULL,
	CworldInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvHatena = {
	"hatena", NULL, NULL, NULL, "1990",
	"Adventure Quiz 2 Hatena Hatena no Dai-Bouken (Japan 900228)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_QUIZ, 0,
	NULL, HatenaRomInfo, HatenaRomName, NULL, NULL, Qtono1InputInfo, NULL,
	HatenaInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvSpang = {
	"spang", NULL, NULL, NULL, "1990",
	"Super Pang (World 900914)\0", NULL, "Mitchell", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, SpangRomInfo, SpangRomName, NULL, NULL, PangInputInfo, NULL,
	SpangInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvSpangj = {
	"spangj", "spang", NULL, NULL, "1990",
	"Super Pang (Japan 901023)\0", NULL, "Mitchell", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, SpangjRomInfo, SpangjRomName, NULL, NULL, PangInputInfo, NULL,
	SpangjInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvSbbros = {
	"sbbros", "spang", NULL, NULL, "1990",
	"Super Buster Bros. (US 901001)\0", NULL, "Mitchell + Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, SbbrosRomInfo, SbbrosRomName, NULL, NULL, PangInputInfo, NULL,
	SbbrosInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvMstworld = {
	"mstworld", NULL, NULL, NULL, "1994",
	"Monsters World\0", NULL, "TCH", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, MstworldRomInfo, MstworldRomName, NULL, NULL, MstworldInputInfo, MstworldDIPInfo,
	MstworldInit, CommonExit, MstworldFrame, NULL, MstworldScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvMarukin = {
	"marukin", NULL, NULL, NULL, "1990",
	"Super Marukin-Ban (Japan 901017)\0", NULL, "Yuga", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MAHJONG, 0,
	NULL, MarukinRomInfo, MarukinRomName, NULL, NULL, MarukinInputInfo, NULL,
	MarukinInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvQtono1 = {
	"qtono1", NULL, NULL, NULL, "1991",
	"Quiz Tonosama no Yabou (Japan)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_QUIZ, 0,
	NULL, Qtono1RomInfo, Qtono1RomName, NULL, NULL, Qtono1InputInfo, NULL,
	Qtono1Init, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvQsangoku = {
	"qsangoku", NULL, NULL, NULL, "1991",
	"Quiz Sangokushi (Japan)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_QUIZ, 0,
	NULL, QsangokuRomInfo, QsangokuRomName, NULL, NULL, Qtono1InputInfo, NULL,
	QsangokuInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 384, 240, 4, 3
};

struct BurnDriver BurnDrvBlock = {
	"block", NULL, NULL, NULL, "1991",
	"Block Block (World 910910)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_POST90S, GBF_BREAKOUT, 0,
	NULL, BlockRomInfo, BlockRomName, NULL, NULL, BlockInputInfo, NULL,
	BlockInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 240, 384, 3, 4
};

struct BurnDriver BurnDrvBlockj = {
	"blockj", "block", NULL, NULL, "1991",
	"Block Block (Japan 910910)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_BREAKOUT, 0,
	NULL, BlockjRomInfo, BlockjRomName, NULL, NULL, BlockInputInfo, NULL,
	BlockInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 240, 384, 3, 4
};

struct BurnDriver BurnDrvBlockjoy = {
	"blockjoy", "block", NULL, NULL, "1991",
	"Block Block (World 911116 Joystick)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_BREAKOUT, 0,
	NULL, BlockjoyRomInfo, BlockjoyRomName, NULL, NULL, BlockjoyInputInfo, NULL,
	BlockjoyInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 240, 384, 3, 4
};

struct BurnDriver BurnDrvBlockbl = {
	"blockbl", "block", NULL, NULL, "1991",
	"Block Block (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_POST90S, GBF_BREAKOUT, 0,
	NULL, BlockblRomInfo, BlockblRomName, NULL, NULL, BlockInputInfo, NULL,
	BlockblInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x800, 240, 384, 3, 4
};
