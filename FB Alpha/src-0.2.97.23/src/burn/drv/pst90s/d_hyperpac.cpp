#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "timer.h"
#include "msm6295.h"
#include "burn_ym2151.h"
#include "burn_ym3812.h"
#include "bitswap.h"

static UINT8 HyperpacInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 HyperpacInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 HyperpacInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 HyperpacDip[2]        = {0, 0};
static UINT8 HyperpacInput[3]      = {0x00, 0x00, 0x00};
static UINT8 HyperpacReset         = 0;

static UINT8 *Mem                  = NULL;
static UINT8 *MemEnd               = NULL;
static UINT8 *RamStart             = NULL;
static UINT8 *RamEnd               = NULL;
static UINT8 *HyperpacRom          = NULL;
static UINT8 *HyperpacZ80Rom       = NULL;
static UINT8 *HyperpacRam          = NULL;
static UINT8 *HyperpacPaletteRam   = NULL;
static UINT8 *HyperpacSpriteRam    = NULL;
static UINT8 *HyperpacZ80Ram       = NULL;
static UINT32 *HyperpacPalette     = NULL;
static UINT8 *HyperpacSprites      = NULL;
static UINT8 *HyperpacSprites8bpp  = NULL;
static UINT8 *HyperpacTempGfx      = NULL;
static UINT8 *HyperpacProtData     = NULL;

static INT32 HyperpacSoundLatch;
static INT32 Moremore = 0;
static INT32 Threein1semi = 0;
static INT32 Cookbib3 = 0;
static INT32 Fourin1boot = 0;
static INT32 Finalttr = 0;
static INT32 Twinadv = 0;
static INT32 Honeydol = 0;
static INT32 Wintbob = 0;
static INT32 Snowbro3 = 0;

static INT32 HyperpacNumTiles = 0;
static INT32 HyperpacNumTiles8bpp = 0;

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static INT32 Snowbro3MusicPlaying;
static INT32 Snowbro3Music;

static struct BurnInputInfo HyperpacInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , HyperpacInputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , HyperpacInputPort2 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , HyperpacInputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , HyperpacInputPort2 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , HyperpacInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , HyperpacInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , HyperpacInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , HyperpacInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , HyperpacInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , HyperpacInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , HyperpacInputPort0 + 6, "p1 fire 3" },

	{"P2 Up"             , BIT_DIGITAL  , HyperpacInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , HyperpacInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , HyperpacInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , HyperpacInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , HyperpacInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , HyperpacInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , HyperpacInputPort1 + 6, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL  , &HyperpacReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, HyperpacDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, HyperpacDip + 1       , "dip"       },
};

STDINPUTINFO(Hyperpac)

static struct BurnInputInfo SnowbrosInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , HyperpacInputPort2 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , HyperpacInputPort2 + 0, "p1 start"  },

	{"P1 Up"             , BIT_DIGITAL  , HyperpacInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , HyperpacInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , HyperpacInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , HyperpacInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , HyperpacInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , HyperpacInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , HyperpacInputPort0 + 6, "p1 fire 3" },

	{"Coin 2"            , BIT_DIGITAL  , HyperpacInputPort2 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , HyperpacInputPort2 + 1, "p2 start"  },

	{"P2 Up"             , BIT_DIGITAL  , HyperpacInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , HyperpacInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , HyperpacInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , HyperpacInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , HyperpacInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , HyperpacInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , HyperpacInputPort1 + 6, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL  , &HyperpacReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , HyperpacInputPort2 + 6, "service"   },
	{"Tilt"              , BIT_DIGITAL  , HyperpacInputPort2 + 5, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH, HyperpacDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, HyperpacDip + 1       , "dip"       },
};

STDINPUTINFO(Snowbros)

static inline void HyperpacClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

static inline void HyperpacMakeInputs()
{
	// Reset Inputs
	HyperpacInput[0] = HyperpacInput[1] = HyperpacInput[2] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		HyperpacInput[0] |= (HyperpacInputPort0[i] & 1) << i;
		HyperpacInput[1] |= (HyperpacInputPort1[i] & 1) << i;
		HyperpacInput[2] |= (HyperpacInputPort2[i] & 1) << i;
	}

	// Clear Opposites
	HyperpacClearOpposites(&HyperpacInput[0]);
	HyperpacClearOpposites(&HyperpacInput[1]);
}

static struct BurnDIPInfo HyperpacDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfc, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x13, 0x01, 0x02, 0x02, "3"                      },
	{0x13, 0x01, 0x02, 0x00, "5"                      },

	{0   , 0xfe, 0   , 8   , "Coin : Credit"          },
	{0x13, 0x01, 0x1c, 0x00, "5 : 1"                  },
	{0x13, 0x01, 0x1c, 0x04, "4 : 1"                  },
	{0x13, 0x01, 0x1c, 0x08, "3 : 1"                  },
	{0x13, 0x01, 0x1c, 0x0c, "2 : 1"                  },
	{0x13, 0x01, 0x1c, 0x1c, "1 : 1"                  },
	{0x13, 0x01, 0x1c, 0x14, "2 : 3"                  },
	{0x13, 0x01, 0x1c, 0x18, "1 : 2"                  },
	{0x13, 0x01, 0x1c, 0x10, "1 : 3"                  },

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0x60, 0x00, "Easy"                   },
	{0x13, 0x01, 0x60, 0x60, "Normal"                 },
	{0x13, 0x01, 0x60, 0x40, "Hard"                   },
	{0x13, 0x01, 0x60, 0x20, "Very Hard"              },

	{0   , 0xfe, 0   , 2   , "Mode"                   },
	{0x13, 0x01, 0x80, 0x80, "Game"                   },
	{0x13, 0x01, 0x80, 0x00, "Test"                   },
};

STDDIPINFO(Hyperpac)

static struct BurnDIPInfo Cookbib2DIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfc, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Max Vs Round"           },
	{0x13, 0x01, 0x02, 0x02, "3"                      },
	{0x13, 0x01, 0x02, 0x00, "1"                      },

	{0   , 0xfe, 0   , 8   , "Coin : Credit"          },
	{0x13, 0x01, 0x1c, 0x00, "5 : 1"                  },
	{0x13, 0x01, 0x1c, 0x04, "4 : 1"                  },
	{0x13, 0x01, 0x1c, 0x08, "3 : 1"                  },
	{0x13, 0x01, 0x1c, 0x0c, "2 : 1"                  },
	{0x13, 0x01, 0x1c, 0x1c, "1 : 1"                  },
	{0x13, 0x01, 0x1c, 0x14, "2 : 3"                  },
	{0x13, 0x01, 0x1c, 0x18, "1 : 2"                  },
	{0x13, 0x01, 0x1c, 0x10, "1 : 3"                  },

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0x60, 0x00, "Easy"                   },
	{0x13, 0x01, 0x60, 0x60, "Normal"                 },
	{0x13, 0x01, 0x60, 0x40, "Hard"                   },
	{0x13, 0x01, 0x60, 0x20, "Very Hard"              },

	{0   , 0xfe, 0   , 2   , "Mode"                   },
	{0x13, 0x01, 0x80, 0x80, "Game"                   },
	{0x13, 0x01, 0x80, 0x00, "Test"                   },
};

STDDIPINFO(Cookbib2)

static struct BurnDIPInfo Cookbib3DIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xee, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },

	{0   , 0xfe, 0   , 8   , "Coin : Credit"          },
	{0x13, 0x01, 0x0e, 0x00, "5 : 1"                  },
	{0x13, 0x01, 0x0e, 0x02, "4 : 1"                  },
	{0x13, 0x01, 0x0e, 0x04, "3 : 1"                  },
	{0x13, 0x01, 0x0e, 0x06, "2 : 1"                  },
	{0x13, 0x01, 0x0e, 0x0e, "1 : 1"                  },
	{0x13, 0x01, 0x0e, 0x0a, "2 : 3"                  },
	{0x13, 0x01, 0x0e, 0x0c, "1 : 2"                  },
	{0x13, 0x01, 0x0e, 0x08, "1 : 3"                  },

	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x13, 0x01, 0x70, 0x20, "Level 1"                },
	{0x13, 0x01, 0x70, 0x10, "Level 2"                },
	{0x13, 0x01, 0x70, 0x00, "Level 3"                },
	{0x13, 0x01, 0x70, 0x70, "Level 4"                },
	{0x13, 0x01, 0x70, 0x60, "Level 5"                },
	{0x13, 0x01, 0x70, 0x50, "Level 6"                },
	{0x13, 0x01, 0x70, 0x40, "Level 7"                },
	{0x13, 0x01, 0x70, 0x30, "Level 8"                },

	{0   , 0xfe, 0   , 2   , "Mode"                   },
	{0x13, 0x01, 0x80, 0x80, "Game"                   },
	{0x13, 0x01, 0x80, 0x00, "Test"                   },
	
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x14, 0x01, 0x80, 0x80, "Off"                    },
	{0x14, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Cookbib3)

static struct BurnDIPInfo MoremoreDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xee, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },

	{0   , 0xfe, 0   , 8   , "Coin : Credit"          },
	{0x13, 0x01, 0x0e, 0x00, "5 : 1"                  },
	{0x13, 0x01, 0x0e, 0x02, "4 : 1"                  },
	{0x13, 0x01, 0x0e, 0x04, "3 : 1"                  },
	{0x13, 0x01, 0x0e, 0x06, "2 : 1"                  },
	{0x13, 0x01, 0x0e, 0x0e, "1 : 1"                  },
	{0x13, 0x01, 0x0e, 0x0a, "2 : 3"                  },
	{0x13, 0x01, 0x0e, 0x0c, "1 : 2"                  },
	{0x13, 0x01, 0x0e, 0x08, "1 : 3"                  },

	{0   , 0xfe, 0   , 8   , "Difficulty"             },
	{0x13, 0x01, 0x70, 0x20, "Level 1"                },
	{0x13, 0x01, 0x70, 0x10, "Level 2"                },
	{0x13, 0x01, 0x70, 0x00, "Level 3"                },
	{0x13, 0x01, 0x70, 0x70, "Level 4"                },
	{0x13, 0x01, 0x70, 0x60, "Level 5"                },
	{0x13, 0x01, 0x70, 0x50, "Level 6"                },
	{0x13, 0x01, 0x70, 0x40, "Level 7"                },
	{0x13, 0x01, 0x70, 0x30, "Level 8"                },

	{0   , 0xfe, 0   , 2   , "Mode"                   },
	{0x13, 0x01, 0x80, 0x80, "Game"                   },
	{0x13, 0x01, 0x80, 0x00, "Test"                   },
	
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x14, 0x01, 0x80, 0x80, "Off"                    },
	{0x14, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Moremore)

static struct BurnDIPInfo ToppyrapDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfe, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },

	{0   , 0xfe, 0   , 8   , "Coin : Credit"          },
	{0x13, 0x01, 0x1c, 0x00, "5 : 1"                  },
	{0x13, 0x01, 0x1c, 0x04, "4 : 1"                  },
	{0x13, 0x01, 0x1c, 0x08, "3 : 1"                  },
	{0x13, 0x01, 0x1c, 0x0c, "2 : 1"                  },
	{0x13, 0x01, 0x1c, 0x1c, "1 : 1"                  },
	{0x13, 0x01, 0x1c, 0x14, "2 : 3"                  },
	{0x13, 0x01, 0x1c, 0x18, "1 : 2"                  },
	{0x13, 0x01, 0x1c, 0x10, "1 : 3"                  },

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0x60, 0x00, "Easy"                   },
	{0x13, 0x01, 0x60, 0x60, "Normal"                 },
	{0x13, 0x01, 0x60, 0x40, "Hard"                   },
	{0x13, 0x01, 0x60, 0x20, "Hardest"                },

	{0   , 0xfe, 0   , 2   , "Mode"                   },
	{0x13, 0x01, 0x80, 0x80, "Game"                   },
	{0x13, 0x01, 0x80, 0x00, "Test"                   },
	
	{0   , 0xfe, 0   , 4   , "Player Count"           },
	{0x14, 0x01, 0x03, 0x00, "2"                      },
	{0x14, 0x01, 0x03, 0x03, "3"                      },
	{0x14, 0x01, 0x03, 0x02, "4"                      },
	{0x14, 0x01, 0x03, 0x01, "5"                      },
	
	{0   , 0xfe, 0   , 4   , "Game Time"              },
	{0x14, 0x01, 0x0c, 0x04, "40 seconds"             },
	{0x14, 0x01, 0x0c, 0x08, "50 seconds"             },
	{0x14, 0x01, 0x0c, 0x0c, "60 seconds"             },
	{0x14, 0x01, 0x0c, 0x00, "70 seconds"             },
	
	{0   , 0xfe, 0   , 2   , "God Mode"               },
	{0x14, 0x01, 0x20, 0x20, "Off"                    },
	{0x14, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Internal Test"          },
	{0x14, 0x01, 0x40, 0x40, "Off"                    },
	{0x14, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x14, 0x01, 0x80, 0x80, "Off"                    },
	{0x14, 0x01, 0x80, 0x00, "On"                     },
};      

STDDIPINFO(Toppyrap)

static struct BurnDIPInfo Fourin1bootDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfe, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0x06, 0x04, "1"                      },
	{0x13, 0x01, 0x06, 0x06, "2"                      },
	{0x13, 0x01, 0x06, 0x02, "3"                      },
	{0x13, 0x01, 0x06, 0x00, "4"                      },

	{0   , 0xfe, 0   , 16  , "Coin : Credit"          },
	{0x13, 0x01, 0x78, 0x18, "4 : 1"                  },
	{0x13, 0x01, 0x78, 0x38, "3 : 1"                  },
	{0x13, 0x01, 0x78, 0x10, "4 : 2"                  },
	{0x13, 0x01, 0x78, 0x58, "2 : 1"                  },
	{0x13, 0x01, 0x78, 0x30, "3 : 2"                  },
	{0x13, 0x01, 0x78, 0x08, "4 : 3"                  },
	{0x13, 0x01, 0x78, 0x00, "4 : 4"                  },
	{0x13, 0x01, 0x78, 0x28, "3 : 3"                  },
	{0x13, 0x01, 0x78, 0x50, "2 : 2"                  },
	{0x13, 0x01, 0x78, 0x78, "1 : 1"                  },
	{0x13, 0x01, 0x78, 0x20, "3 : 4"                  },
	{0x13, 0x01, 0x78, 0x48, "2 : 3"                  },
	{0x13, 0x01, 0x78, 0x40, "2 : 4"                  },
	{0x13, 0x01, 0x78, 0x70, "1 : 2"                  },
	{0x13, 0x01, 0x78, 0x68, "1 : 3"                  },
	{0x13, 0x01, 0x78, 0x60, "1 : 4"                  },

	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x03, 0x02, "1"                      },
	{0x14, 0x01, 0x03, 0x00, "2"                      },
	{0x14, 0x01, 0x03, 0x03, "3"                      },
	{0x14, 0x01, 0x03, 0x01, "4"                      },
	
	{0   , 0xfe, 0   , 2   , "Mode"                   },
	{0x14, 0x01, 0x80, 0x80, "Game"                   },
	{0x14, 0x01, 0x80, 0x00, "Test"                   },
};

STDDIPINFO(Fourin1boot)

static struct BurnDIPInfo FinalttrDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x13, 0x01, 0x01, 0x00, "Off"                    },
	{0x13, 0x01, 0x01, 0x01, "On"                     },

	{0   , 0xfe, 0   , 7   , "Coin : Credit"          },
	{0x13, 0x01, 0x38, 0x00, "4 : 1"                  },
	{0x13, 0x01, 0x38, 0x10, "3 : 1"                  },
	{0x13, 0x01, 0x38, 0x30, "2 : 1"                  },
	{0x13, 0x01, 0x38, 0x38, "1 : 1"                  },
	{0x13, 0x01, 0x38, 0x20, "2 : 3"                  },
	{0x13, 0x01, 0x38, 0x28, "1 : 2"                  },
	{0x13, 0x01, 0x38, 0x08, "1 : 4"                  },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Time"                   },
	{0x14, 0x01, 0x0c, 0x00, "60 seconds"             },
	{0x14, 0x01, 0x0c, 0x0c, "90 seconds"             },
	{0x14, 0x01, 0x0c, 0x08, "120 seconds"            },
	{0x14, 0x01, 0x0c, 0x04, "150 seconds"            },
};	

STDDIPINFO(Finalttr)

static struct BurnDIPInfo TwinadvDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                     },
	{0x14, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Coin : Credit"          },
	{0x13, 0x01, 0x03, 0x00, "3 : 1"                  },
	{0x13, 0x01, 0x03, 0x01, "2 : 1"                  },
	{0x13, 0x01, 0x03, 0x03, "1 : 1"                  },
	{0x13, 0x01, 0x03, 0x02, "1 : 2"                  },
	
	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x13, 0x01, 0x10, 0x00, "Off"                    },
	{0x13, 0x01, 0x10, 0x10, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Level Select"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x14, 0x01, 0x03, 0x03, "Normal"                 },
	{0x14, 0x01, 0x03, 0x02, "Hard"                   },
	{0x14, 0x01, 0x03, 0x01, "Harder"                 },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x14, 0x01, 0x04, 0x04, "3"                      },
	{0x14, 0x01, 0x04, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Ticket Mode 1"          },
	{0x14, 0x01, 0x08, 0x08, "Off"                    },
	{0x14, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Ticket Mode 2"          },
	{0x14, 0x01, 0x40, 0x40, "Off"                    },
	{0x14, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x14, 0x01, 0x80, 0x80, "Off"                    },
	{0x14, 0x01, 0x80, 0x00, "On"                     },
};
	
STDDIPINFO(Twinadv)

static struct BurnDIPInfo HoneydolDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                     },
	{0x14, 0xff, 0xff, 0xef, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x13, 0x01, 0x03, 0x00, "3 Coins 1 Credit"       },
	{0x13, 0x01, 0x03, 0x01, "2 Coins 1 Credit"       },
	{0x13, 0x01, 0x03, 0x03, "1 Coin  1 Credit"       },
	{0x13, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Show Girls"             },
	{0x13, 0x01, 0x04, 0x00, "Off"                    },
	{0x13, 0x01, 0x04, 0x04, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sound"             },
	{0x13, 0x01, 0x10, 0x00, "Off"                    },
	{0x13, 0x01, 0x10, 0x10, "On"                     },

	{0   , 0xfe, 0   , 2   , "Level Select"           },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Slide show on boot"     },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x14, 0x01, 0x03, 0x03, "Normal"                 },
	{0x14, 0x01, 0x03, 0x02, "Hard"                   },
	{0x14, 0x01, 0x03, 0x01, "Harder"                 },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 4   , "Timer speed"            },
	{0x14, 0x01, 0x0c, 0x0c, "Normal"                 },
	{0x14, 0x01, 0x0c, 0x08, "Fast"                   },
	{0x14, 0x01, 0x0c, 0x04, "Faster"                 },
	{0x14, 0x01, 0x0c, 0x00, "Fastest"                },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x14, 0x01, 0x30, 0x00, "1"                      },
	{0x14, 0x01, 0x30, 0x10, "2"                      },
	{0x14, 0x01, 0x30, 0x20, "3"                      },
	{0x14, 0x01, 0x30, 0x30, "5"                      },

	{0   , 0xfe, 0   , 2   , "Max vs Round"           },
	{0x14, 0x01, 0x80, 0x80, "3"                      },
	{0x14, 0x01, 0x80, 0x00, "1"                      },
};

STDDIPINFO(Honeydol)

static struct BurnDIPInfo SnowbrosDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0x01, NULL                     },
	{0x16, 0xff, 0xff, 0x00, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Country"                },
	{0x15, 0x01, 0x01, 0x01, "Europ Type"             },
	{0x15, 0x01, 0x01, 0x00, "America Type"           },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x15, 0x01, 0x02, 0x02, "Invert Screen"          },
	{0x15, 0x01, 0x02, 0x00, "Normal Screen"          },

	{0   , 0xfe, 0   , 2   , "Test Mode"              },
	{0x15, 0x01, 0x04, 0x00, "Off"                    },
	{0x15, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x15, 0x01, 0x08, 0x08, "Advertise Sound Off"    },
	{0x15, 0x01, 0x08, 0x00, "Advertise Sound On"     },

	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x15, 0x01, 0x30, 0x30, "4 Coin 1 Play"          },
	{0x15, 0x01, 0x30, 0x20, "3 Coin 1 Play"          },
	{0x15, 0x01, 0x30, 0x10, "2 Coin 1 Play"          },
	{0x15, 0x01, 0x30, 0x00, "1 Coin 1 Play"          },

	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x15, 0x01, 0xc0, 0x00, "1 Coin 2 Play"          },
	{0x15, 0x01, 0xc0, 0x40, "1 Coin 3 Play"          },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin 4 Play"          },
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin 6 Play"          },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x16, 0x01, 0x03, 0x01, "Game Difficulty A"      },
	{0x16, 0x01, 0x03, 0x00, "Game Difficulty B"      },
	{0x16, 0x01, 0x03, 0x02, "Game Difficulty C"      },
	{0x16, 0x01, 0x03, 0x03, "Game Difficulty D"      },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x16, 0x01, 0x0c, 0x0c, "Extend No Extend"       },
	{0x16, 0x01, 0x0c, 0x00, "Extend 100000 Only"     },
	{0x16, 0x01, 0x0c, 0x08, "Extend 100000, 200000"  },
	{0x16, 0x01, 0x0c, 0x04, "Extend 200000 Only"     },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x30, 0x10, "Hero Counts 1"          },
	{0x16, 0x01, 0x30, 0x30, "Hero Counts 2"          },
	{0x16, 0x01, 0x30, 0x00, "Hero Counts 3"          },
	{0x16, 0x01, 0x30, 0x20, "Hero Counts 4"          },

	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x16, 0x01, 0x40, 0x40, "No Death, Stop Mode"    },
	{0x16, 0x01, 0x40, 0x00, "Normal Game"            },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x80, 0x00, "Continue Play On"       },
	{0x16, 0x01, 0x80, 0x80, "Continue Play Off"      },
};

STDDIPINFO(Snowbros)

static struct BurnDIPInfo SnowbrojDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0x00, NULL                     },
	{0x16, 0xff, 0xff, 0x00, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet Type"           },
	{0x15, 0x01, 0x01, 0x01, "Up-Right Type"          },
	{0x15, 0x01, 0x01, 0x00, "Table Type"             },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x15, 0x01, 0x02, 0x02, "Invert Screen"          },
	{0x15, 0x01, 0x02, 0x00, "Normal Screen"          },

	{0   , 0xfe, 0   , 2   , "Test Mode"              },
	{0x15, 0x01, 0x04, 0x00, "Off"                    },
	{0x15, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x15, 0x01, 0x08, 0x08, "Advertise Sound Off"    },
	{0x15, 0x01, 0x08, 0x00, "Advertise Sound On"     },

	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x15, 0x01, 0x30, 0x30, "2 Coin 3 Play"          },
	{0x15, 0x01, 0x30, 0x20, "2 Coin 1 Play"          },
	{0x15, 0x01, 0x30, 0x10, "1 Coin 2 Play"          },
	{0x15, 0x01, 0x30, 0x00, "1 Coin 1 Play"          },

	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x15, 0x01, 0xc0, 0xc0, "2 Coin 3 Play"          },
	{0x15, 0x01, 0xc0, 0x80, "2 Coin 1 Play"          },
	{0x15, 0x01, 0xc0, 0x40, "1 Coin 2 Play"          },
	{0x15, 0x01, 0xc0, 0x00, "1 Coin 1 Play"          },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x16, 0x01, 0x03, 0x01, "Game Difficulty A"      },
	{0x16, 0x01, 0x03, 0x00, "Game Difficulty B"      },
	{0x16, 0x01, 0x03, 0x02, "Game Difficulty C"      },
	{0x16, 0x01, 0x03, 0x03, "Game Difficulty D"      },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x16, 0x01, 0x0c, 0x0c, "Extend No Extend"       },
	{0x16, 0x01, 0x0c, 0x00, "Extend 100000 Only"     },
	{0x16, 0x01, 0x0c, 0x08, "Extend 100000, 200000"  },
	{0x16, 0x01, 0x0c, 0x04, "Extend 200000 Only"     },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x16, 0x01, 0x30, 0x10, "Hero Counts 1"          },
	{0x16, 0x01, 0x30, 0x30, "Hero Counts 2"          },
	{0x16, 0x01, 0x30, 0x00, "Hero Counts 3"          },
	{0x16, 0x01, 0x30, 0x20, "Hero Counts 4"          },

	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x16, 0x01, 0x40, 0x40, "No Death, Stop Mode"    },
	{0x16, 0x01, 0x40, 0x00, "Normal Game"            },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x16, 0x01, 0x80, 0x00, "Continue Play On"       },
	{0x16, 0x01, 0x80, 0x80, "Continue Play Off"      },
};

STDDIPINFO(Snowbroj)

static struct BurnRomInfo HyperpacRomDesc[] = {
	{ "hyperpac.h12",  0x20000, 0x2cf0531a, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "hyperpac.i12",  0x20000, 0x9c7d85b8, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "hyperpac.a4",   0x40000, 0xbd8673da, BRF_GRA },			 //  2	Sprites
	{ "hyperpac.a5",   0x40000, 0x5d90cd82, BRF_GRA },			 //  3	Sprites
	{ "hyperpac.a6",   0x40000, 0x61d86e63, BRF_GRA },			 //  4	Sprites

	{ "hyperpac.u1",   0x10000, 0x03faf88e, BRF_SND },			 //  5	Z80 Program Code

	{ "hyperpac.j15",  0x40000, 0xfb9f468d, BRF_SND },			 //  6	Samples
	
	{ "at89c52.bin",   0x02000, 0x291f9326, BRF_PRG | BRF_OPT },
};


STD_ROM_PICK(Hyperpac)
STD_ROM_FN(Hyperpac)

static struct BurnRomInfo HyperpacbRomDesc[] = {
	{ "hpacuh12.bin",  0x20000, 0x633ab2c6, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "hpacui12.bin",  0x20000, 0x23dc00d1, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "hyperpac.a4",   0x40000, 0xbd8673da, BRF_GRA },			 //  2	Sprites
	{ "hyperpac.a5",   0x40000, 0x5d90cd82, BRF_GRA },			 //  3	Sprites
	{ "hyperpac.a6",   0x40000, 0x61d86e63, BRF_GRA },			 //  4	Sprites

	{ "hyperpac.u1",   0x10000, 0x03faf88e, BRF_SND },			 //  5	Z80 Program Code

	{ "hyperpac.j15",  0x40000, 0xfb9f468d, BRF_SND },			 //  6	Samples
};


STD_ROM_PICK(Hyperpacb)
STD_ROM_FN(Hyperpacb)

static struct BurnRomInfo Cookbib2RomDesc[] = {
	{ "cookbib2.02",   0x40000, 0xb2909460, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "cookbib2.01",   0x40000, 0x65aafde2, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "cookbib2.05",   0x80000, 0x89fb38ce, BRF_GRA },			 //  2	Sprites
	{ "cookbib2.04",   0x80000, 0xf240111f, BRF_GRA },			 //  3	Sprites
	{ "cookbib2.03",   0x40000, 0xe1604821, BRF_GRA },			 //  4	Sprites

	{ "cookbib2.07",   0x10000, 0xf59f1c9a, BRF_SND },			 //  5	Z80 Program Code

	{ "cookbib2.06",   0x20000, 0x5e6f76b8, BRF_SND },			 //  6	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },
	
	{ "protdata.bin",  0x00200, 0xae6d8ed5, BRF_ESS | BRF_PRG },		 // Data from shared RAM
};


STD_ROM_PICK(Cookbib2)
STD_ROM_FN(Cookbib2)

static struct BurnRomInfo Cookbib3RomDesc[] = {
	{ "u52.bin",       0x40000, 0x65134893, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "u74.bin",       0x40000, 0xc4ab8435, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "u75.bin",       0x80000, 0xcbe4d9c8, BRF_GRA },			 //  2	Sprites
	{ "u76.bin",       0x80000, 0x1be17b57, BRF_GRA },			 //  3	Sprites
	{ "u77.bin",       0x80000, 0x7823600d, BRF_GRA },			 //  4	Sprites

	{ "u35.bin",       0x10000, 0x5dfd2a98, BRF_SND },			 //  5	Z80 Program Code

	{ "u14.bin",       0x20000, 0xe5bf9288, BRF_SND },			 //  6	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },
	
	{ "protdata.bin",  0x00200, 0xc819b9a8, BRF_ESS | BRF_PRG },		 // Data from shared RAM
};


STD_ROM_PICK(Cookbib3)
STD_ROM_FN(Cookbib3)

static struct BurnRomInfo MoremoreRomDesc[] = {
	{ "u52.bin",       0x40000, 0xcea4b246, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "u74.bin",       0x40000, 0x2acdcb88, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "u75.bin",       0x80000, 0xd671815c, BRF_GRA },			 //  2	Sprites
	{ "u76.bin",       0x80000, 0xe0d479e8, BRF_GRA },			 //  3	Sprites
	{ "u77.bin",       0x80000, 0x60a281da, BRF_GRA },			 //  4	Sprites
	{ "u78.bin",       0x80000, 0xe2723b4e, BRF_GRA },			 //  5	Sprites

	{ "u35.bin",       0x10000, 0x92dc95fc, BRF_SND },			 //  6	Z80 Program Code

	{ "u14.bin",       0x40000, 0x90580088, BRF_SND },			 //  7	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },
	
	{ "protdata.bin",  0x00200, 0x782dd2aa, BRF_ESS | BRF_PRG },		 // Data from shared RAM
};


STD_ROM_PICK(Moremore)
STD_ROM_FN(Moremore)

static struct BurnRomInfo MoremorpRomDesc[] = {
	{ "mmp_u52.bin",   0x40000, 0x66baf9b2, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "mmp_u74.bin",   0x40000, 0x7c6fede5, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "mmp_u75.bin",   0x80000, 0xaf9e824e, BRF_GRA },			 //  2	Sprites
	{ "mmp_u76.bin",   0x80000, 0xc42af064, BRF_GRA },			 //  3	Sprites
	{ "mmp_u77.bin",   0x80000, 0x1d7396e1, BRF_GRA },			 //  4	Sprites
	{ "mmp_u78.bin",   0x80000, 0x5508d80b, BRF_GRA },			 //  5	Sprites

	{ "mmp_u35.bin",   0x10000, 0x4d098cad, BRF_SND },			 //  6	Z80 Program Code

	{ "mmp_u14.bin",   0x40000, 0x211a2566, BRF_SND },			 //  7	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },
	
	{ "protdata.bin",  0x00200, 0x782dd2aa, BRF_ESS | BRF_PRG },		 // Data from shared RAM
};


STD_ROM_PICK(Moremorp)
STD_ROM_FN(Moremorp)

static struct BurnRomInfo ToppyrapRomDesc[] = {
	{ "uh12.bin",      0x40000, 0x6f5ad699, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "ui12.bin",      0x40000, 0xcaf5a7e1, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "ua4.bin",       0x80000, 0xa9577bcf, BRF_GRA },			 //  2	Sprites
	{ "ua5.bin",       0x80000, 0x7179d32d, BRF_GRA },			 //  3	Sprites
	{ "ua6.bin",       0x80000, 0x4834e5b1, BRF_GRA },			 //  4	Sprites
	{ "ua7.bin",       0x80000, 0x663dd099, BRF_GRA },			 //  5	Sprites

	{ "u1.bin",        0x10000, 0x07f50947, BRF_SND },			 //  6	Z80 Program Code

	{ "uj15.bin",      0x20000, 0xa3bacfd7, BRF_SND },			 //  7	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },
	
	{ "protdata.bin",  0x00200, 0x0704e6c7, BRF_ESS | BRF_PRG },		 // Data from shared RAM
};


STD_ROM_PICK(Toppyrap)
STD_ROM_FN(Toppyrap)

static struct BurnRomInfo Threein1semiRomDesc[] = {
	{ "u52",           0x40000, 0xb0e4a0f7, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "u74",           0x40000, 0x266862c4, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "u75",           0x80000, 0xb66a0db6, BRF_GRA },			 //  2	Sprites
	{ "u76",           0x80000, 0x5f4b48ea, BRF_GRA },			 //  3	Sprites
	{ "u77",           0x80000, 0xd44211e3, BRF_GRA },			 //  4	Sprites
	{ "u78",           0x80000, 0xaf596afc, BRF_GRA },			 //  5	Sprites

	{ "u35",           0x10000, 0xe40481da, BRF_SND },			 //  6	Z80 Program Code

	{ "u14",           0x40000, 0xc83c11be, BRF_SND },			 //  7	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },
	
	{ "protdata.bin",  0x00200, 0x85deba7c, BRF_ESS | BRF_PRG },		 // Data from shared RAM
};


STD_ROM_PICK(Threein1semi)
STD_ROM_FN(Threein1semi)

static struct BurnRomInfo TwinkleRomDesc[] = {
	{ "uh12.bin",      0x20000, 0xa99626fe, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "ui12.bin",      0x20000, 0x5af73684, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "ua4.bin",       0x80000, 0x6b64bb09, BRF_GRA },			 //  2	Sprites

	{ "u1.bin",        0x10000, 0xe40481da, BRF_SND },			 //  3	Z80 Program Code

	{ "uj15.bin",      0x40000, 0x0a534b37, BRF_SND },			 //  4	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },
	
	{ "protdata.bin",  0x00200, 0x00d3e4b4, BRF_ESS | BRF_PRG },		 // Data from shared RAM
};


STD_ROM_PICK(Twinkle)
STD_ROM_FN(Twinkle)

static struct BurnRomInfo PzlbreakRomDesc[] = {
	{ "4.uh12",        0x20000, 0xb3f04f80, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "5.ui12",        0x20000, 0x13c298a0, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "2.ua4",         0x80000, 0xd211705a, BRF_GRA },			 //  2	Sprites
	{ "3.ua5",         0x80000, 0x6cdb73e9, BRF_GRA },			 //  3

	{ "0.u1",          0x10000, 0x1ad646b7, BRF_SND },			 //  4	Z80 Program Code

	{ "1.uj15",        0x40000, 0xdbfae77c, BRF_SND },			 //  5	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },
	
	{ "protdata.bin",  0x00200, 0x092cb794, BRF_ESS | BRF_PRG },		 // Data from shared RAM
};


STD_ROM_PICK(Pzlbreak)
STD_ROM_FN(Pzlbreak)

static struct BurnRomInfo Fourin1bootRomDesc[] = {
	{ "u52",           0x80000, 0x71815878, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "u74",           0x80000, 0xe22d3fa2, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "u78",           0x200000,0x6c1fbc9c, BRF_GRA },			 //  2	Sprites

	{ "u35",           0x10000, 0xc894ac80, BRF_SND },			 //  3	Z80 Program Code

	{ "u14",           0x40000, 0x94b09b0e, BRF_SND },			 //  4	Samples
};


STD_ROM_PICK(Fourin1boot)
STD_ROM_FN(Fourin1boot)

static struct BurnRomInfo FinalttrRomDesc[] = {
	{ "10.7o",         0x20000, 0xeecc83e5, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "9.5o",          0x20000, 0x58d3640e, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "5.1d",          0x40000, 0x64a450f3, BRF_GRA },			 //  2	Sprites
	{ "6.1f",          0x40000, 0x7281a3cc, BRF_GRA },			 //  3	Sprites
	{ "7.1g",          0x40000, 0xec80f442, BRF_GRA },			 //  4	Sprites
	{ "9.1h",          0x40000, 0x2ebd316d, BRF_GRA },			 //  5	Sprites

	{ "12.5r",         0x10000, 0x4bc21361, BRF_SND },			 //  6	Z80 Program Code

	{ "11.7p",         0x20000, 0x2e331022, BRF_SND },			 //  7	Samples
	
	{ "87c52.mcu",     0x10000, 0x00000000, BRF_NODUMP },
	
	{ "protdata.bin",  0x00200, 0xd5bbb006, BRF_ESS | BRF_PRG },		 // Data from shared RAM
};


STD_ROM_PICK(Finalttr)
STD_ROM_FN(Finalttr)

static struct BurnRomInfo TwinadvRomDesc[] = {
	{ "13.uh12",       0x20000, 0x9f70a39b, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "12.ui12",       0x20000, 0xd8776495, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "16.ua4",        0x80000, 0xf491e171, BRF_GRA },			 //  2	Sprites
	{ "15.ua5",        0x80000, 0x79a08b8d, BRF_GRA },			 //  3	Sprites
	{ "14.ua6",        0x80000, 0x79faee0b, BRF_GRA },			 //  4	Sprites

	{ "uh15.bin",      0x08000, 0x3d5acd08, BRF_SND },			 //  5	Z80 Program Code

	{ "sra.bin",       0x40000, 0x82f452c4, BRF_SND },			 //  6	Samples
	{ "srb.bin",       0x40000, 0x109e51e6, BRF_SND },			 //  7	Samples
};


STD_ROM_PICK(Twinadv)
STD_ROM_FN(Twinadv)

static struct BurnRomInfo TwinadvkRomDesc[] = {
	{ "uh12",          0x20000, 0xe0bcc738, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "ui12",          0x20000, 0xa3ee6451, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "ua4",           0x80000, 0xa5aff49b, BRF_GRA },			 //  2	Sprites
	{ "ua5",           0x80000, 0xf83b3b97, BRF_GRA },			 //  3	Sprites
	{ "ua6",           0x80000, 0x4dfcffb9, BRF_GRA },			 //  4	Sprites

	{ "uh15.bin",      0x08000, 0x3d5acd08, BRF_SND },			 //  5	Z80 Program Code

	{ "sra.bin",       0x40000, 0x82f452c4, BRF_SND },			 //  6	Samples
	{ "srb.bin",       0x40000, 0x109e51e6, BRF_SND },			 //  7	Samples
};


STD_ROM_PICK(Twinadvk)
STD_ROM_FN(Twinadvk)

static struct BurnRomInfo HoneydolRomDesc[] = {
	{ "d-16.uh12",     0x20000, 0xcee1a2e3, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "d-17.ui12",     0x20000, 0xcac44154, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "d-13.1",        0x80000, 0xff6a57fb, BRF_GRA },			 //  2	4bpp Sprites
	
	{ "d-14.5",        0x80000, 0x2178996f, BRF_GRA },			 //  3	8bpp Sprites
	{ "d-15.6",        0x80000, 0x6629239e, BRF_GRA },			 //  4	8bpp Sprites
	{ "d-18.9",        0x80000, 0x0210507a, BRF_GRA },			 //  5	8bpp Sprites
	{ "d-19.10",       0x80000, 0xd27375d5, BRF_GRA },			 //  6	8bpp Sprites

	{ "d-12.uh15",     0x08000, 0x386f1b63, BRF_SND },			 //  7	Z80 Program Code

	{ "d-11.u14",      0x40000, 0xf3ee4861, BRF_SND },			 //  8	Samples
};


STD_ROM_PICK(Honeydol)
STD_ROM_FN(Honeydol)

static struct BurnRomInfo SnowbrosRomDesc[] = {
	{ "sn6.bin",       0x20000, 0x4899ddcf, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "sn5.bin",       0x20000, 0xad310d3f, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "sbros-1.41",    0x80000, 0x16f06b3a, BRF_GRA },			 //  2	Sprites

	{ "sbros-4.29",    0x08000, 0xe6eab4e4, BRF_SND },			 //  3	Z80 Program Code
};


STD_ROM_PICK(Snowbros)
STD_ROM_FN(Snowbros)

static struct BurnRomInfo SnowbroaRomDesc[] = {
	{ "sbros-3a.5",    0x20000, 0x10cb37e1, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "sbros-2a.6",    0x20000, 0xab91cc1e, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "sbros-1.41",    0x80000, 0x16f06b3a, BRF_GRA },			 //  2	Sprites

	{ "sbros-4.29",    0x08000, 0xe6eab4e4, BRF_SND },			 //  3	Z80 Program Code
};


STD_ROM_PICK(Snowbroa)
STD_ROM_FN(Snowbroa)

static struct BurnRomInfo SnowbrobRomDesc[] = {
	{ "sbros3-a",      0x20000, 0x301627d6, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "sbros2-a",      0x20000, 0xf6689f41, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "sbros-1.41",    0x80000, 0x16f06b3a, BRF_GRA },			 //  2	Sprites

	{ "sbros-4.29",    0x08000, 0xe6eab4e4, BRF_SND },			 //  3	Z80 Program Code
};


STD_ROM_PICK(Snowbrob)
STD_ROM_FN(Snowbrob)

static struct BurnRomInfo SnowbrocRomDesc[] = {
	{ "3-a.ic5",       0x20000, 0xe1bc346b, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "2-a.ic6",       0x20000, 0x1be27f9d, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "sbros-1.41",    0x80000, 0x16f06b3a, BRF_GRA },			 //  2	Sprites

	{ "sbros-4.29",    0x08000, 0xe6eab4e4, BRF_SND },			 //  3	Z80 Program Code
};


STD_ROM_PICK(Snowbroc)
STD_ROM_FN(Snowbroc)

static struct BurnRomInfo SnowbrojRomDesc[] = {
	{ "snowbros.3",    0x20000, 0x3f504f9e, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "snowbros.2",    0x20000, 0x854b02bc, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "sbros-1.41",    0x80000, 0x16f06b3a, BRF_GRA },			 //  2	Sprites

	{ "sbros-4.29",    0x08000, 0xe6eab4e4, BRF_SND },			 //  3	Z80 Program Code
};


STD_ROM_PICK(Snowbroj)
STD_ROM_FN(Snowbroj)

static struct BurnRomInfo SnowbrodRomDesc[] = {
	{ "sbk_3-a.bin",   0x20000, 0x97174d40, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "sbk_2-a.bin",   0x20000, 0x80cc80e5, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "sbros-1.41",    0x80000, 0x16f06b3a, BRF_GRA },			 //  2	Sprites

	{ "sbros-4.29",    0x08000, 0xe6eab4e4, BRF_SND },			 //  3	Z80 Program Code
};


STD_ROM_PICK(Snowbrod)
STD_ROM_FN(Snowbrod)

static struct BurnRomInfo WintbobRomDesc[] = {
	{ "wb3",           0x10000, 0xb9719767, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "wb1",           0x10000, 0xa4488998, BRF_ESS | BRF_PRG }, //  1	68000 Program Code
	{ "wb04.bin",      0x10000, 0x53be758d, BRF_ESS | BRF_PRG }, //  2	68000 Program Code
	{ "wb02.bin",      0x10000, 0xfc8e292e, BRF_ESS | BRF_PRG }, //  3	68000 Program Code

	{ "wb13.bin",      0x10000, 0x426921de, BRF_GRA },			 //  4	Sprites
	{ "wb06.bin",      0x10000, 0x68204937, BRF_GRA },			 //  5	Sprites
	{ "wb12.bin",      0x10000, 0xef4e04c7, BRF_GRA },			 //  6	Sprites
	{ "wb07.bin",      0x10000, 0x53f40978, BRF_GRA },			 //  7	Sprites
	{ "wb11.bin",      0x10000, 0x41cb4563, BRF_GRA },			 //  8	Sprites
	{ "wb08.bin",      0x10000, 0x9497b88c, BRF_GRA },			 //  9	Sprites
	{ "wb10.bin",      0x10000, 0x5fa22b1e, BRF_GRA },			 //  10	Sprites
	{ "wb09.bin",      0x10000, 0x9be718ca, BRF_GRA },			 //  11	Sprites

	{ "wb05.bin",      0x10000, 0x53fe59df, BRF_SND },			 //  12	Z80 Program Code
};


STD_ROM_PICK(Wintbob)
STD_ROM_FN(Wintbob)

static struct BurnRomInfo Snowbro3RomDesc[] = {
	{ "ur4",           0x020000, 0x19c13ffd, BRF_ESS | BRF_PRG }, 		 //  0	68000 Program Code
	{ "ur3",           0x020000, 0x3f32fa15, BRF_ESS | BRF_PRG }, 		 //  1	68000 Program Code

	{ "ua5",           0x080000, 0x0604e385, BRF_GRA },			 //  2	4bpp Sprites
	
	{ "un7",           0x200000, 0x4a79da4c, BRF_GRA },			 //  3	8bpp Sprites
	{ "un8",           0x200000, 0x7a4561a4, BRF_GRA },			 //  4	8bpp Sprites

	{ "us5",           0x080000, 0x7c6368ef, BRF_SND },			 //  5	Samples
	
	{ "sound.mcu",     0x010000, 0x00000000, BRF_PRG | BRF_NODUMP },	 //  6	Sound MCU
};


STD_ROM_PICK(Snowbro3)
STD_ROM_FN(Snowbro3)

static INT32 HyperpacDoReset()
{
	HyperpacSoundLatch = 0;
	
	if(HyperpacProtData && !Finalttr) memcpy(HyperpacRam + 0xf000, HyperpacProtData, 0x200);
	if(HyperpacProtData && Finalttr) memcpy(HyperpacRam + 0x2000, HyperpacProtData, 0x200);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	MSM6295Reset(0);
	if (Honeydol) BurnYM3812Reset();
	if (!Twinadv && !Honeydol) BurnYM2151Reset();

	return 0;
}

static INT32 SnowbrosDoReset()
{
	HyperpacSoundLatch = 0;

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM3812Reset();

	return 0;
}

static INT32 Snowbro3DoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();
	
	MSM6295Reset(0);
	
	Snowbro3Music = 0;
	Snowbro3MusicPlaying = 0;

	return 0;
}

static void HyperpacYM2151IrqHandler(INT32 Irq)
{
	if (Irq) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

// ----------------------------------------------------------------------------
// CPU synchronisation

static INT32 nCycles68KSync;

static inline void snowbrosSynchroniseZ80(INT32 nExtraCycles)
{
	INT32 nCycles = ((INT64)SekTotalCycles() * nCyclesTotal[1] / nCyclesTotal[0]) + nExtraCycles;

	if (nCycles <= ZetTotalCycles()) {
		return;
	}

	nCycles68KSync = nCycles - nExtraCycles;

	BurnTimerUpdateYM3812(nCycles);
}

// Callbacks for the FM chip

static void snowbrosFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 snowbrosSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 6000000;
}

static INT32 HoneydolSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static void Snowbro3PlayMusic(INT32 data)
{
	Snowbro3Music = data;
	
	bprintf(PRINT_NORMAL, _T("%x\n"), data);
	
	switch (data) {
		case 0x23: {
			memcpy(MSM6295ROM + 0x20000, MSM6295ROM + 0x80000, 0x20000);
			Snowbro3MusicPlaying = 1;
			break;
		}
		
		case 0x24: {
			memcpy(MSM6295ROM + 0x20000, MSM6295ROM + 0xa0000, 0x20000);
			Snowbro3MusicPlaying = 1;
			break;
		}
		
		case 0x25: {
			memcpy(MSM6295ROM + 0x20000, MSM6295ROM + 0xc0000, 0x20000);
			Snowbro3MusicPlaying = 1;
			break;
		}
		
		case 0x26: {
			memcpy(MSM6295ROM + 0x20000, MSM6295ROM + 0x80000, 0x20000);
			Snowbro3MusicPlaying = 1;
			break;
		}
		
		case 0x27:
		case 0x28:
		case 0x29:
		case 0x2a:
		case 0x2b:
		case 0x2c:
		case 0x2d: {
			memcpy(MSM6295ROM + 0x20000, MSM6295ROM + 0xc0000, 0x20000);
			Snowbro3MusicPlaying = 1;
			break;
		}
		
		case 0x2e: {
			Snowbro3MusicPlaying = 0;
			break;
		}
	}
}

static void Snowbro3PlaySound(INT32 data)
{
	INT32 Status = MSM6295ReadStatus(0);
	
	if ((Status & 0x01) == 0x00) {
		MSM6295Command(0, 0x80 | data);
		MSM6295Command(0, 0x12);
	} else {
		if ((Status & 0x02) == 0x00) {
			MSM6295Command(0, 0x80 | data);
			MSM6295Command(0, 0x12);
		} else {
			if ((Status & 0x04) == 0x00) {
				MSM6295Command(0, 0x80 | data);
				MSM6295Command(0, 0x42);
			}
		}	
	}
}

// ----------------------------------------------------------------------------

UINT8 __fastcall HyperpacReadByte(UINT32 a)
{
	switch (a) {
		case 0x200000: {
			if (Threein1semi) return 0x0a;
		}
	
		case 0x200001: {
			if (Moremore || Threein1semi) return 0x0a;
		}
		
		case 0x500000: {
			return HyperpacInput[0];
		}

		case 0x500001: {
			return HyperpacDip[0];
		}

		case 0x500002: {
			return HyperpacInput[1];
		}
		
		case 0x500003: {
			return HyperpacDip[1];
		}

		case 0x500004: {
			return HyperpacInput[2];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Read byte -> %06X\n"), a);
		}
	}
	
	return 0;
}

UINT8 __fastcall HyperpacReadByteLow(UINT32 a)
{
	switch (a) {
		case 0x200001: {
			if (Fourin1boot) return 0x02;
		}
		
		case 0x500000: {
			return 0x7f - HyperpacInput[0];
		}

		case 0x500001: {
			return HyperpacDip[0];
		}

		case 0x500002: {
			return 0x7f - HyperpacInput[1];
		}
		
		case 0x500003: {
			return HyperpacDip[1];
		}

		case 0x500004: {
			return 0xff - HyperpacInput[2];
		}
		
		case 0x500005: {
			return 0xff - HyperpacInput[2];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Read byte -> %06X\n"), a);
		}
	}
	
	return 0xff;
}

void __fastcall HyperpacWriteByte(UINT32 a, UINT8 d)
{
	if (a < 0x100000) return;

	switch (a) {
		case 0x300001: {
			HyperpacSoundLatch = d & 0xff;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write byte -> %06X, %02X\n"), a, d);
		}
	}
}

void __fastcall TwinadvWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x200000:
		case 0x200001: {
			return;
		}
	
		case 0x300001: {
			HyperpacSoundLatch = d & 0xff;
			ZetNmi();
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write byte -> %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall HyperpacReadWord(UINT32 a)
{
	switch (a) {
		case 0x200000: {
			if (Cookbib3) return 0x2a2a;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Read Word -> %06X\n"), a);
		}
	}
	
	return 0;
}

UINT16 __fastcall HyperpacReadWordLow(UINT32 a)
{
	switch (a) {
		case 0x500000:
		case 0x500002:
		case 0x500004: {
			SEK_DEF_READ_WORD(0, a);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Read Word -> %06X\n"), a);
		}
		
	}
	
	return 0xffff;
}

void __fastcall HyperpacWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x00b7d6: 
		case 0x00b7d8: {
			if (Finalttr) return;
		}
		
		case 0x200000: {
			if (Fourin1boot) return; // Watchdog in original Snow Bros.
		}
		
		case 0x300000: {
			SEK_DEF_WRITE_WORD(0, a, d);
			return;
		}

		case 0x400000: {
			return;
		}
		
		case 0x800000: 
		case 0x900000: 
		case 0xa00000: {
			// IRQ ACK
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Write word -> %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall HoneydolReadByte(UINT32 a)
{
	switch (a) {
		case 0x900000: {
			return 0x7f - HyperpacInput[0];
		}
		
		case 0x900001: {
			return HyperpacDip[0];
		}
		
		case 0x900002: {
			return 0x7f - HyperpacInput[1];
		}
		
		case 0x900003: {
			return HyperpacDip[1];
		}
		
		case 0x900004: {
			return 0xff - HyperpacInput[2];
		}
		
		case 0x900005: {
			return 0xff;
		}
	}

	bprintf(PRINT_NORMAL, _T("Read byte -> %06X\n"), a);
	
	return 0;
}

UINT16 __fastcall HoneydolReadWord(UINT32 a)
{
	switch (a) {
		case 0x900000:
		case 0x900002:
		case 0x900004: {
			SEK_DEF_READ_WORD(0, a);
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Read Word -> %06X\n"), a);
	
	return 0;
}

void __fastcall HoneydolWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x300000: {
			return;
		}
		
		case 0x300001: {
			HyperpacSoundLatch = d;
			snowbrosSynchroniseZ80(0);
			ZetNmi();
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Write byte -> %06X, %02X\n"), a, d);
}

void __fastcall HoneydolWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x200000: {
			return; // Watchdog?
		}
		
		case 0x300000: {
			SEK_DEF_WRITE_WORD(0, a, d);
			return;
		}
	
		case 0x400000:
		case 0x500000: 
		case 0x600000: {
			// IRQ Ack
			return;
		}
		
		case 0x800000: {
			// ?
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Write word -> %06X, %04X\n"), a, d);
}

UINT8 __fastcall HyperpacZ80Read(UINT16 a)
{
	switch (a) {
		case 0xf001: {
			return BurnYM2151ReadStatus();
		}

		case 0xf008: {
			if (Fourin1boot) {
				if (HyperpacSoundLatch != 0xff) return HyperpacSoundLatch;
			} else {
				return HyperpacSoundLatch;
			}
		}
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 Read -> %04X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall HyperpacZ80Write(UINT16 a, UINT8 d)
{
	d &= 0xff;
	
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
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 Write -> %04X, %02x\n"), a, d);
		}
	}
}

UINT8 __fastcall TwinadvZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x02: {
			return HyperpacSoundLatch;
		}
		
		case 0x06: {
			return MSM6295ReadStatus(0);
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Z80 Port Read -> %02X\n"), a);

	return 0;
}

void __fastcall TwinadvZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x02: {
			HyperpacSoundLatch = d;
			return;
		}
		
		case 0x04: {
			INT32 bank = (d &0x02) >> 1;
			memcpy(MSM6295ROM + 0x00000, MSM6295ROM + (0x40000 * bank), 0x40000);
			return;
		}
		
		case 0x06: {
			MSM6295Command(0, d);
			return;
		}
	}

	bprintf(PRINT_NORMAL, _T("Z80 Port Write -> %02X, %02x\n"), a, d);
}

UINT8 __fastcall HoneydolZ80Read(UINT16 a)
{
	switch (a) {
		case 0xe010: {
			return MSM6295ReadStatus(0);
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Z80 Read -> %04X\n"), a);
	
	return 0;
}

void __fastcall HoneydolZ80Write(UINT16 a, UINT8 d)
{
	d &= 0xff;
	
	switch (a) {
		case 0xe010: {
			MSM6295Command(0, d);
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("Z80 Write -> %04X, %02x\n"), a, d);
}

UINT16 __fastcall SnowbrosReadWord(UINT32 a)
{
	switch (a) {
		case 0x300000:
		case 0x500000:
		case 0x500002:
		case 0x500004: {
			SEK_DEF_READ_WORD(0, a);
		}
	}

	return 0;
}

UINT8 __fastcall SnowbrosReadByte(UINT32 a)
{
	switch (a) {
		case 0x300001: {
			snowbrosSynchroniseZ80(0x0100);
			return HyperpacSoundLatch;
		}

		case 0x500000: {
			return 0x7f - HyperpacInput[0];
		}

		case 0x500001: {
			return 0xff - HyperpacDip[0];
		}

		case 0x500002: {
			return 0xff - HyperpacInput[1];
		}

		case 0x500003: {
			return 0xff - HyperpacDip[1];
		}

		case 0x500004: {
			return 0xff - HyperpacInput[2];
		}
	}

	return 0;
}

void __fastcall SnowbrosWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x300000: {
			SEK_DEF_WRITE_WORD(0, a, d);
			return;
		}
	}
}

void __fastcall SnowbrosWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x300001: {
			HyperpacSoundLatch = d;
			snowbrosSynchroniseZ80(0);
			ZetNmi();
			return;
		}
	}
}

UINT16 __fastcall Snowbro3ReadWord(UINT32 a)
{
	switch (a) {
		case 0x300000: {
			return 3;
		}
		
		case 0x500000:
		case 0x500002:
		case 0x500004: {
			SEK_DEF_READ_WORD(0, a);
		}
	}
	
	bprintf(PRINT_NORMAL, _T("68000 Read Word %06X\n"), a);

	return 0;
}

UINT8 __fastcall Snowbro3ReadByte(UINT32 a)
{
	switch (a) {
/*		case 0x300001: {
			snowbrosSynchroniseZ80(0x0100);
			return HyperpacSoundLatch;
		}*/

		case 0x500000: {
			return 0x7f - HyperpacInput[0];
		}

		case 0x500001: {
			return 0xff - HyperpacDip[0];
		}

		case 0x500002: {
			return 0xff - HyperpacInput[1];
		}

		case 0x500003: {
			return 0xff - HyperpacDip[1];
		}

		case 0x500004: {
			return 0xff - HyperpacInput[2];
		}
		
		case 0x500005: {
			return 0xff;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("68000 Read Byte %06X\n"), a);

	return 0;
}

void __fastcall Snowbro3WriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x300000: {
			if (d == 0xfe) {
				Snowbro3MusicPlaying = 0;
				MSM6295Command(0, 0x78);
			} else {
				d = d >> 8;
				
				if (d <= 0x21) Snowbro3PlaySound(d);
				if (d >= 0x22 && d <= 0x31) Snowbro3PlayMusic(d);
				if (d >= 0x30 && d <= 0x51) Snowbro3PlaySound(d - 0x30);
				if (d >= 0x52 && d <= 0x5f) Snowbro3PlayMusic(d - 0x30);
			}
			return;
		}
		
		case 0x200000:
		case 0x800000:
		case 0x900000:
		case 0xa00000: {
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("68000 Write Word %06X -> %04X\n"), a, d);
}

void __fastcall Snowbro3WriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x300000: {
			if (d == 0xfe) {
				Snowbro3MusicPlaying = 0;
				MSM6295Command(0, 0x78);
			} else {
				//d = d >> 8;
				
				if (d <= 0x21) Snowbro3PlaySound(d);
				if (d >= 0x22 && d <= 0x31) Snowbro3PlayMusic(d);
				if (d >= 0x30 && d <= 0x51) Snowbro3PlaySound(d - 0x30);
				if (d >= 0x52 && d <= 0x5f) Snowbro3PlayMusic(d - 0x30);
			}
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("68000 Write Byte %06X -> %02X\n"), a, d);
}

UINT8 __fastcall SnowbrosZ80PortRead(UINT16 a)
{
	a &= 0xff;

	switch (a) {
		case 0x02:
			return BurnYM3812Read(0);
		case 0x04: {
			if (ZetTotalCycles() > nCycles68KSync) {
				BurnTimerUpdateEndYM3812();
			}
			return HyperpacSoundLatch;
		}
	}

	return 0;
}

void __fastcall SnowbrosZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;

	switch (a) {
		case 0x02: {
			BurnYM3812Write(0, d);
			return;
		}

		case 0x03: {
			BurnYM3812Write(1, d);
			return;
		}

		case 0x04: {
			HyperpacSoundLatch = d;
			return;
		}
	}
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	HyperpacRom          = Next; Next += 0x100000;
	HyperpacZ80Rom       = Next; Next += 0x10000;
	if (Twinadv) {
		MSM6295ROM           = Next; Next += 0x80000;
	} else {
		MSM6295ROM           = Next; Next += 0x40000;
	}

	RamStart = Next;

	HyperpacRam          = Next; Next += 0x10000;
	if (Honeydol) {
		HyperpacPaletteRam   = Next; Next += 0x00800;
	} else {
		HyperpacPaletteRam   = Next; Next += 0x00200;
	}
	HyperpacSpriteRam    = Next; Next += 0x02000;
	HyperpacZ80Ram       = Next; Next += 0x00800;

	RamEnd = Next;

	HyperpacSprites      = Next; Next += (HyperpacNumTiles * 16 * 16);
	HyperpacSprites8bpp  = Next; Next += (HyperpacNumTiles8bpp * 16 * 16);
	HyperpacProtData     = Next; Next += 0x00200;
	if (Honeydol) {
		HyperpacPalette      = (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);
	} else {
		HyperpacPalette      = (UINT32*)Next; Next += 0x00200 * sizeof(UINT32);
	}
	MemEnd = Next;

	return 0;
}


static INT32 SnowbrosMemIndex()
{
	UINT8 *Next; Next = Mem;

	HyperpacRom          = Next; Next += 0x40000;
	HyperpacZ80Rom       = Next; Next += (Wintbob) ? 0x10000 : 0x08000;

	RamStart = Next;

	HyperpacRam          = Next; Next += 0x04000;
	HyperpacPaletteRam   = Next; Next += 0x00200;
	HyperpacSpriteRam    = Next; Next += 0x02000;
	HyperpacZ80Ram       = Next; Next += 0x00800;

	RamEnd = Next;

	HyperpacSprites      = Next; Next += (HyperpacNumTiles * 16 * 16);
	HyperpacPalette      = (UINT32*)Next; Next += 0x00200 * sizeof(UINT32);
	MemEnd = Next;

	return 0;
}

static INT32 Snowbro3MemIndex()
{
	UINT8 *Next; Next = Mem;

	HyperpacRom          = Next; Next += 0x40000;
	MSM6295ROM           = Next; Next += 0xe0000;

	RamStart = Next;

	HyperpacRam          = Next; Next += 0x04000;
	HyperpacPaletteRam   = Next; Next += 0x00400;
	HyperpacSpriteRam    = Next; Next += 0x02200;

	RamEnd = Next;

	HyperpacSprites      = Next; Next += (HyperpacNumTiles * 16 * 16);
	HyperpacSprites8bpp  = Next; Next += (HyperpacNumTiles8bpp * 16 * 16);
	HyperpacPalette      = (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);
	MemEnd = Next;

	return 0;
}

static INT32 HyperpacMachineInit()
{
	BurnSetRefreshRate(57.5);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(HyperpacRom       , 0x000000, 0x0fffff, SM_ROM);
	if (Finalttr) {
		SekMapMemory(HyperpacRam       , 0x100000, 0x103fff, SM_RAM);
	} else {
		SekMapMemory(HyperpacRam       , 0x100000, 0x10ffff, SM_RAM);
	}
	SekMapMemory(HyperpacPaletteRam, 0x600000, 0x6001ff, SM_RAM);
	SekMapMemory(HyperpacSpriteRam , 0x700000, 0x701fff, SM_RAM);
	if (Fourin1boot || Finalttr) {
		SekSetReadByteHandler(0, HyperpacReadByteLow);
		SekSetReadWordHandler(0, HyperpacReadWordLow);
	} else {
		SekSetReadByteHandler(0, HyperpacReadByte);
		SekSetReadWordHandler(0, HyperpacReadWord);
	}
	SekSetWriteByteHandler(0, HyperpacWriteByte);
	SekSetWriteWordHandler(0, HyperpacWriteWord);
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xcfff, 0, HyperpacZ80Rom);
	ZetMapArea(0x0000, 0xcfff, 2, HyperpacZ80Rom);
	ZetMapArea(0xd000, 0xd7ff, 0, HyperpacZ80Ram);
	ZetMapArea(0xd000, 0xd7ff, 1, HyperpacZ80Ram);
	ZetMapArea(0xd000, 0xd7ff, 2, HyperpacZ80Ram);
	ZetMemEnd();
	ZetSetReadHandler(HyperpacZ80Read);
	ZetSetWriteHandler(HyperpacZ80Write);
	ZetClose();

	// Setup the YM2151 emulation
	BurnYM2151Init(4000000);
	BurnYM2151SetIrqHandler(&HyperpacYM2151IrqHandler);
	BurnYM2151SetAllRoutes(0.10, BURN_SND_ROUTE_BOTH);

	// Setup the OKIM6295 emulation
	MSM6295Init(0, 999900 / 132, 1);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	// Reset the driver
	HyperpacDoReset();

	return 0;
}

static INT32 HyperpacSpritePlaneOffsets[4] = { 0, 1, 2, 3 };
static INT32 HyperpacSpriteXOffsets[16]    = { 4, 0, 260, 256, 20, 16, 276, 272, 12, 8, 268, 264, 28, 24, 284, 280 };
static INT32 HyperpacSpriteYOffsets[16]    = { 0, 64, 32, 96, 512, 576, 544, 608, 128, 192, 160, 224, 640, 704, 672, 736 };
static INT32 SnowbrosSpritePlaneOffsets[4] = { 0, 1, 2, 3 };
static INT32 SnowbrosSpriteXOffsets[16]    = { 0, 4, 8, 12, 16, 20, 24, 28, 256, 260, 264, 268, 272, 276, 280, 284 };
static INT32 SnowbrosSpriteYOffsets[16]    = { 0, 32, 64, 96, 128, 160, 192, 224, 512, 544, 576, 608, 640, 672, 704, 736 };
static INT32 WintbobSpritePlaneOffsets[4]  = { 0, 1, 2, 3 };
static INT32 WintbobSpriteXOffsets[16]     = { 12, 8, 4, 0, 28, 24, 20, 16, 44, 40, 36, 32, 60, 56, 52, 48 };
static INT32 WintbobSpriteYOffsets[16]     = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };
static INT32 Honeydol8BppPlaneOffsets[8]   = { 0, 1, 2, 3, 0x800000, 0x800001, 0x800002, 0x800003 };
static INT32 Honeydol8BppXOffsets[16]      = { 0, 4, 8, 12, 16, 20, 24, 28, 256, 260, 264, 268, 272, 276, 280, 284 };
static INT32 Honeydol8BppYOffsets[16]      = { 0, 32, 64, 96, 128, 160, 192, 224, 512, 544, 576, 608, 640, 672, 704, 736 };
static INT32 Snowbro38BppPlaneOffsets[8]   = { 8, 9, 10, 11, 0, 1, 2, 3 };
static INT32 Snowbro38BppXOffsets[16]      = { 0, 4, 16, 20, 32, 36, 48, 52, 512, 516, 528, 532, 544, 548, 560, 564 };
static INT32 Snowbro38BppYOffsets[16]      = { 0, 64, 128, 192, 256, 320, 384, 448, 1024, 1088, 1152, 1216, 1280, 1344, 1408, 1472 };

static INT32 HyperpacInit()
{
	INT32 nRet = 0, nLen;
	
	HyperpacNumTiles = 6144;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x0c0000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(HyperpacRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Rom
	nRet = BurnLoadRom(HyperpacZ80Rom, 5, 1); if (nRet != 0) return 1;

	// Simulate RAM initialisation done by the protection MCU (not needed for bootleg)
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "hyperpac")) {
		HyperpacRam[0xe000 + 0] = 0xf9;
		HyperpacRam[0xe000 + 1] = 0x4e;
		HyperpacRam[0xe002 + 0] = 0x00;
		HyperpacRam[0xe002 + 1] = 0x00;
		HyperpacRam[0xe004 + 0] = 0x2c;
		HyperpacRam[0xe004 + 1] = 0x06;
		HyperpacRam[0xe080 + 0] = 0xdc;
		HyperpacRam[0xe080 + 1] = 0xfe;
		HyperpacRam[0xe082 + 0] = 0x98;
		HyperpacRam[0xe082 + 1] = 0xba;
		HyperpacRam[0xe084 + 0] = 0x54;
		HyperpacRam[0xe084 + 1] = 0x76;
		HyperpacRam[0xe086 + 0] = 0x10;
		HyperpacRam[0xe086 + 1] = 0x32;
	}
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(HyperpacTempGfx + 0x00000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x40000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x80000, 4, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles, 4, 16, 16, HyperpacSpritePlaneOffsets, HyperpacSpriteXOffsets, HyperpacSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);	
	BurnFree(HyperpacTempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 6, 1); if (nRet != 0) return 1;
	
	nRet = HyperpacMachineInit(); if (nRet) return 1;

	return 0;
}

static INT32 Cookbib2Init()
{
	INT32 nRet = 0, nLen;
	
	HyperpacNumTiles = 10240;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x140000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(HyperpacRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Rom
	nRet = BurnLoadRom(HyperpacZ80Rom, 5, 1); if (nRet != 0) return 1;

	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(HyperpacTempGfx + 0x000000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x080000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x100000, 4, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles, 4, 16, 16, HyperpacSpritePlaneOffsets, HyperpacSpriteXOffsets, HyperpacSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);	
	BurnFree(HyperpacTempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 6, 1); if (nRet != 0) return 1;
	
	// Load Shared RAM data
	nRet = BurnLoadRom(HyperpacProtData, 8, 1); if (nRet) return 1;
	BurnByteswap(HyperpacProtData, 0x200);
	
	nRet = HyperpacMachineInit(); if (nRet) return 1;

	return 0;
}

static INT32 Cookbib3Init()
{
	INT32 nRet = 0, nLen;
	
	HyperpacNumTiles = 16384;
	
	Cookbib3 = 1;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x200000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(HyperpacRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load and descramble Z80 Program Rom
	nRet = BurnLoadRom(HyperpacZ80Rom, 5, 1); if (nRet != 0) return 1;
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x10000);
	memcpy(pTemp, HyperpacZ80Rom, 0x10000);
	memcpy(HyperpacZ80Rom + 0xc000, pTemp + 0x0000, 0x4000);
	memcpy(HyperpacZ80Rom + 0x8000, pTemp + 0x4000, 0x4000);
	memcpy(HyperpacZ80Rom + 0x4000, pTemp + 0x8000, 0x4000);
	memcpy(HyperpacZ80Rom + 0x0000, pTemp + 0xc000, 0x4000);
	BurnFree(pTemp);

	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(HyperpacTempGfx + 0x000000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x080000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x100000, 4, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles, 4, 16, 16, HyperpacSpritePlaneOffsets, HyperpacSpriteXOffsets, HyperpacSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);	
	BurnFree(HyperpacTempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 6, 1); if (nRet != 0) return 1;
	
	// Load Shared RAM data
	nRet = BurnLoadRom(HyperpacProtData, 8, 1); if (nRet) return 1;
	BurnByteswap(HyperpacProtData, 0x200);
	
	nRet = HyperpacMachineInit(); if (nRet) return 1;

	return 0;
}

static INT32 MoremoreInit()
{
	INT32 nRet = 0, nLen;
	
	HyperpacNumTiles = 16384;
	
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "moremore") || !strcmp(BurnDrvGetTextA(DRV_NAME), "moremorp")) Moremore = 1;
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "3in1semi")) Threein1semi = 1;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x200000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(HyperpacRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Rom
	nRet = BurnLoadRom(HyperpacZ80Rom, 6, 1); if (nRet != 0) return 1;

	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(HyperpacTempGfx + 0x000000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x080000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x100000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x180000, 5, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles, 4, 16, 16, HyperpacSpritePlaneOffsets, HyperpacSpriteXOffsets, HyperpacSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);	
	BurnFree(HyperpacTempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 7, 1); if (nRet != 0) return 1;
	
	// Load Shared RAM data
	nRet = BurnLoadRom(HyperpacProtData, 9, 1); if (nRet) return 1;
	BurnByteswap(HyperpacProtData, 0x200);
	
	nRet = HyperpacMachineInit(); if (nRet) return 1;

	return 0;
}

static INT32 TwinkleInit()
{
	INT32 nRet = 0, nLen;
	
	HyperpacNumTiles = 4096;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x080000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(HyperpacRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Rom
	nRet = BurnLoadRom(HyperpacZ80Rom, 3, 1); if (nRet != 0) return 1;

	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(HyperpacTempGfx + 0x000000, 2, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles, 4, 16, 16, HyperpacSpritePlaneOffsets, HyperpacSpriteXOffsets, HyperpacSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);	
	BurnFree(HyperpacTempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 4, 1); if (nRet != 0) return 1;
	
	// Load Shared RAM data
	nRet = BurnLoadRom(HyperpacProtData, 6, 1); if (nRet) return 1;
	BurnByteswap(HyperpacProtData, 0x200);
	
	nRet = HyperpacMachineInit(); if (nRet) return 1;

	return 0;
}

static INT32 PzlbreakInit()
{
	INT32 nRet = 0, nLen;
	
	HyperpacNumTiles = 8192;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x100000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(HyperpacRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Rom
	nRet = BurnLoadRom(HyperpacZ80Rom, 4, 1); if (nRet != 0) return 1;

	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(HyperpacTempGfx + 0x000000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x080000, 3, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles, 4, 16, 16, HyperpacSpritePlaneOffsets, HyperpacSpriteXOffsets, HyperpacSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);	
	BurnFree(HyperpacTempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 5, 1); if (nRet != 0) return 1;
	
	// Load Shared RAM data
	nRet = BurnLoadRom(HyperpacProtData, 7, 1); if (nRet) return 1;
	BurnByteswap(HyperpacProtData, 0x200);
	
	nRet = HyperpacMachineInit(); if (nRet) return 1;

	return 0;
}

static void Fourin1bootDescrambleRom()
{
	UINT8 *buffer;
	UINT8 *src = HyperpacRom;
	INT32 len = 0x100000;

	/* strange order */
	if ((buffer = (UINT8*)BurnMalloc(len)))
	{
		INT32 i;
		for (i = 0;i < len; i++)
			if (i&1) buffer[i] = BITSWAP08(src[i],6,7,5,4,3,2,1,0);
			else buffer[i] = src[i];

		memcpy(src,buffer,len);
		BurnFree(buffer);
	}

	src = HyperpacZ80Rom;
	len = 0x10000;

	/* strange order */
	if ((buffer = (UINT8*)BurnMalloc(len)))
	{
		INT32 i;
		for (i = 0;i < len; i++) {
			buffer[i] = src[i ^ 0x4000];
		}
		memcpy(src,buffer,len);
		BurnFree(buffer);
	}
}

static INT32 Fourin1bootInit()
{
	INT32 nRet = 0, nLen;
	
	HyperpacNumTiles = 16384;
	
	Fourin1boot = 1;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x200000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(HyperpacRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load and Z80 Program Rom
	nRet = BurnLoadRom(HyperpacZ80Rom, 3, 1); if (nRet != 0) return 1;
	
	Fourin1bootDescrambleRom();
	
	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(HyperpacTempGfx + 0x000000, 2, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles, 4, 16, 16, SnowbrosSpritePlaneOffsets, SnowbrosSpriteXOffsets, SnowbrosSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);		
	BurnFree(HyperpacTempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 4, 1); if (nRet != 0) return 1;
	
	nRet = HyperpacMachineInit(); if (nRet) return 1;
	
	return 0;
}

static INT32 FinalttrInit()
{
	INT32 nRet = 0, nLen;
	
	HyperpacNumTiles = 8192;
	
	Finalttr = 1;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x100000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(HyperpacRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Rom
	nRet = BurnLoadRom(HyperpacZ80Rom, 6, 1); if (nRet != 0) return 1;

	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(HyperpacTempGfx + 0x000000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x040000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x080000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x0c0000, 5, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles, 4, 16, 16, HyperpacSpritePlaneOffsets, HyperpacSpriteXOffsets, HyperpacSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);	
	BurnFree(HyperpacTempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM, 7, 1); if (nRet != 0) return 1;
	
	// Load Shared RAM data
	nRet = BurnLoadRom(HyperpacProtData, 9, 1); if (nRet) return 1;
	BurnByteswap(HyperpacProtData, 0x200);
	
	nRet = HyperpacMachineInit(); if (nRet) return 1;
	
	BurnYM2151SetAllRoutes(0.08, BURN_SND_ROUTE_BOTH);
	MSM6295SetRoute(0, 0.40, BURN_SND_ROUTE_BOTH);

	return 0;
}

static INT32 TwinadvInit()
{
	INT32 nRet = 0, nLen;
	
	HyperpacNumTiles = 12288;
	
	Twinadv = 1;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x180000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(HyperpacRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Rom
	nRet = BurnLoadRom(HyperpacZ80Rom, 5, 1); if (nRet != 0) return 1;

	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(HyperpacTempGfx + 0x000000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x080000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x100000, 4, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles, 4, 16, 16, SnowbrosSpritePlaneOffsets, SnowbrosSpriteXOffsets, SnowbrosSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);		
	BurnFree(HyperpacTempGfx);

	// Load Sample Roms
	nRet = BurnLoadRom(MSM6295ROM + 0x00000, 6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROM + 0x40000, 7, 1); if (nRet != 0) return 1;
	
	BurnSetRefreshRate(57.5);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(HyperpacRom       , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(HyperpacRam       , 0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(HyperpacPaletteRam, 0x600000, 0x6001ff, SM_RAM);
	SekMapMemory(HyperpacSpriteRam , 0x700000, 0x701fff, SM_RAM);
	SekSetReadByteHandler(0, HyperpacReadByteLow);
	SekSetReadWordHandler(0, HyperpacReadWordLow);
	SekSetWriteByteHandler(0, TwinadvWriteByte);
	SekSetWriteWordHandler(0, HyperpacWriteWord);
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, HyperpacZ80Rom);
	ZetMapArea(0x0000, 0x7fff, 2, HyperpacZ80Rom);
	ZetMapArea(0x8000, 0x87ff, 0, HyperpacZ80Ram);
	ZetMapArea(0x8000, 0x87ff, 1, HyperpacZ80Ram);
	ZetMapArea(0x8000, 0x87ff, 2, HyperpacZ80Ram);
	ZetMemEnd();
	ZetSetInHandler(TwinadvZ80PortRead);
	ZetSetOutHandler(TwinadvZ80PortWrite);
	ZetClose();

	// Setup the OKIM6295 emulation
	MSM6295Init(0, (12000000/12) / 132, 0);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	// Reset the driver
	HyperpacDoReset();

	return 0;
}

static INT32 HoneydolInit()
{
	INT32 nRet = 0, nLen;
	
	HyperpacNumTiles = 4096;
	HyperpacNumTiles8bpp = 8192;
	
	Honeydol = 1;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x200000);
	
	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(HyperpacRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Rom
	nRet = BurnLoadRom(HyperpacZ80Rom, 7, 1); if (nRet != 0) return 1;

	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(HyperpacTempGfx + 0x000000, 2, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles, 4, 16, 16, SnowbrosSpritePlaneOffsets, SnowbrosSpriteXOffsets, SnowbrosSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);		
		
	// Load and Decode 8bpp Sprite Roms
	memset(HyperpacTempGfx, 0, 0x200000);
	nRet = BurnLoadRom(HyperpacTempGfx + 0x000000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x080000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x100000, 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x180000, 6, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles8bpp, 8, 16, 16, Honeydol8BppPlaneOffsets, Honeydol8BppXOffsets, Honeydol8BppYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites8bpp);
	BurnFree(HyperpacTempGfx);

	// Load Sample Roms
	nRet = BurnLoadRom(MSM6295ROM + 0x00000, 8, 1); if (nRet != 0) return 1;
		
	BurnSetRefreshRate(57.5);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(HyperpacRom       , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(HyperpacRam       , 0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(HyperpacPaletteRam, 0xa00000, 0xa007ff, SM_RAM);
	SekMapMemory(HyperpacSpriteRam , 0xb00000, 0xb01fff, SM_RAM);
	SekSetReadByteHandler(0, HoneydolReadByte);
	SekSetReadWordHandler(0, HoneydolReadWord);
	SekSetWriteByteHandler(0, HoneydolWriteByte);
	SekSetWriteWordHandler(0, HoneydolWriteWord);
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, HyperpacZ80Rom);
	ZetMapArea(0x0000, 0x7fff, 2, HyperpacZ80Rom);
	ZetMapArea(0x8000, 0x87ff, 0, HyperpacZ80Ram);
	ZetMapArea(0x8000, 0x87ff, 1, HyperpacZ80Ram);
	ZetMapArea(0x8000, 0x87ff, 2, HyperpacZ80Ram);
	ZetMemEnd();
	ZetSetInHandler(SnowbrosZ80PortRead);
	ZetSetOutHandler(SnowbrosZ80PortWrite);
	ZetSetReadHandler(HoneydolZ80Read);
	ZetSetWriteHandler(HoneydolZ80Write);
	ZetClose();

	BurnYM3812Init(3000000, &snowbrosFMIRQHandler, &HoneydolSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(4000000);
	BurnYM3812SetRoute(BURN_SND_YM3812_ROUTE, 1.00, BURN_SND_ROUTE_BOTH);

	// Setup the OKIM6295 emulation
	MSM6295Init(0, 999900 / 132, 1);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	// Reset the driver
	HyperpacDoReset();

	return 0;
}

static INT32 SnowbrosInit()
{
	INT32 nRet = 0, nLen;

	BurnSetRefreshRate(57.5);
	
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "wintbob")) Wintbob = 1;
	
	HyperpacNumTiles = 4096;

	// Allocate and Blank all required memory
	Mem = NULL;
	SnowbrosMemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	SnowbrosMemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x80000);

	if (Wintbob) {
		// Load and byte-swap 68000 Program roms
		nRet = BurnLoadRom(HyperpacRom + 0x00001, 0, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(HyperpacRom + 0x00000, 1, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(HyperpacRom + 0x20001, 2, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(HyperpacRom + 0x20000, 3, 2); if (nRet != 0) return 1;

		// Load Z80 Program Rom
		nRet = BurnLoadRom(HyperpacZ80Rom, 12, 1); if (nRet != 0) return 1;

		// Load and Decode Sprite Roms
		nRet = BurnLoadRom(HyperpacTempGfx + 0x00000, 4, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(HyperpacTempGfx + 0x00001, 5, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(HyperpacTempGfx + 0x20000, 6, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(HyperpacTempGfx + 0x20001, 7, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(HyperpacTempGfx + 0x40000, 8, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(HyperpacTempGfx + 0x40001, 9, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(HyperpacTempGfx + 0x60000, 10, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(HyperpacTempGfx + 0x60001, 11, 2); if (nRet != 0) return 1;
		GfxDecode(HyperpacNumTiles, 4, 16, 16, WintbobSpritePlaneOffsets, WintbobSpriteXOffsets, WintbobSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);		
		BurnFree(HyperpacTempGfx);
	} else {
		// Load and byte-swap 68000 Program roms
		nRet = BurnLoadRom(HyperpacRom + 0x00001, 0, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(HyperpacRom + 0x00000, 1, 2); if (nRet != 0) return 1;

		// Load Z80 Program Rom
		nRet = BurnLoadRom(HyperpacZ80Rom, 3, 1); if (nRet != 0) return 1;

		// Load and Decode Sprite Roms
		nRet = BurnLoadRom(HyperpacTempGfx + 0x00000, 2, 1); if (nRet != 0) return 1;
		GfxDecode(HyperpacNumTiles, 4, 16, 16, SnowbrosSpritePlaneOffsets, SnowbrosSpriteXOffsets, SnowbrosSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);		
		BurnFree(HyperpacTempGfx);
	}

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(HyperpacRom       , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(HyperpacRam       , 0x100000, 0x103fff, SM_RAM);
	SekMapMemory(HyperpacPaletteRam, 0x600000, 0x6001ff, SM_RAM);
	SekMapMemory(HyperpacSpriteRam , 0x700000, 0x701fff, SM_RAM);
	SekSetReadWordHandler(0, SnowbrosReadWord);
	SekSetWriteWordHandler(0, SnowbrosWriteWord);
	SekSetReadByteHandler(0, SnowbrosReadByte);
	SekSetWriteByteHandler(0, SnowbrosWriteByte);
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, HyperpacZ80Rom);
	ZetMapArea(0x0000, 0x7fff, 2, HyperpacZ80Rom);
	ZetMapArea(0x8000, 0x87ff, 0, HyperpacZ80Ram);
	ZetMapArea(0x8000, 0x87ff, 1, HyperpacZ80Ram);
	ZetMapArea(0x8000, 0x87ff, 2, HyperpacZ80Ram);
	ZetMemEnd();
	ZetSetInHandler(SnowbrosZ80PortRead);
	ZetSetOutHandler(SnowbrosZ80PortWrite);
	ZetClose();

	BurnYM3812Init(3000000, &snowbrosFMIRQHandler, &snowbrosSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(6000000);
	BurnYM3812SetRoute(BURN_SND_YM3812_ROUTE, 1.00, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	// Reset the driver
	SnowbrosDoReset();

	return 0;
}

static INT32 Snowbro3Init()
{
	INT32 nRet = 0, nLen;
	
	Snowbro3 = 1;

	HyperpacNumTiles = 0x1000;
	HyperpacNumTiles8bpp = 0x4000;

	// Allocate and Blank all required memory
	Mem = NULL;
	Snowbro3MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	Snowbro3MemIndex();

	HyperpacTempGfx = (UINT8*)BurnMalloc(0x400000);

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(HyperpacRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	
	// Decode 68000 Program Rom
	UINT8 *buffer;
	buffer = (UINT8*)BurnMalloc(0x40000);
	for (INT32 i = 0; i < 0x40000; i++) {
		buffer[i] = HyperpacRom[BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,3,4,1,2,0)];
	}
	memcpy(HyperpacRom, buffer, 0x40000);
	BurnFree(buffer);

	// Load and Decode Sprite Roms
	nRet = BurnLoadRom(HyperpacTempGfx + 0x00000, 2, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles, 4, 16, 16, SnowbrosSpritePlaneOffsets, SnowbrosSpriteXOffsets, SnowbrosSpriteYOffsets, 0x400, HyperpacTempGfx, HyperpacSprites);
	
	// Load and Decode 8bpp Sprite Roms
	memset(HyperpacTempGfx, 0, 0x400000);
	nRet = BurnLoadRom(HyperpacTempGfx + 0x000000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(HyperpacTempGfx + 0x200000, 4, 1); if (nRet != 0) return 1;
	GfxDecode(HyperpacNumTiles8bpp, 8, 16, 16, Snowbro38BppPlaneOffsets, Snowbro38BppXOffsets, Snowbro38BppYOffsets, 0x800, HyperpacTempGfx, HyperpacSprites8bpp);
	
	// Load Sample Roms
	memset(HyperpacTempGfx, 0, 0x400000);
	nRet = BurnLoadRom(HyperpacTempGfx, 5, 1); if (nRet != 0) return 1;
	memcpy(MSM6295ROM + 0x00000, HyperpacTempGfx + 0x00000, 0x20000);
	memcpy(MSM6295ROM + 0x80000, HyperpacTempGfx + 0x20000, 0x60000);
	BurnFree(HyperpacTempGfx);

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(HyperpacRom       , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(HyperpacRam       , 0x100000, 0x103fff, SM_RAM);
	SekMapMemory(HyperpacPaletteRam, 0x600000, 0x6003ff, SM_RAM);
	SekMapMemory(HyperpacSpriteRam , 0x700000, 0x7021ff, SM_RAM);
	SekSetReadWordHandler(0, Snowbro3ReadWord);
	SekSetWriteWordHandler(0, Snowbro3WriteWord);
	SekSetReadByteHandler(0, Snowbro3ReadByte);
	SekSetWriteByteHandler(0, Snowbro3WriteByte);
	SekClose();
	
	// Setup the OKIM6295 emulation
	MSM6295Init(0, 999900 / 132, 0);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	// Reset the driver
	Snowbro3DoReset();

	return 0;
}

static INT32 HyperpacExit()
{
	MSM6295Exit(0);

	SekExit();
	ZetExit();
	
	GenericTilesExit();
	
	if (!Twinadv && !Honeydol) BurnYM2151Exit();
	if (Honeydol) BurnYM3812Exit();

	BurnFree(Mem);
	
	HyperpacNumTiles = 0;
	HyperpacNumTiles8bpp = 0;
	
	Moremore = 0;
	Threein1semi = 0;
	Cookbib3 = 0;
	Fourin1boot = 0;
	Finalttr = 0;
	Twinadv = 0;
	Honeydol = 0;
	Snowbro3 = 0;
	
	return 0;
}

static INT32 SnowbrosExit()
{
	if (!Snowbro3) BurnYM3812Exit();
	if (Snowbro3) MSM6295Exit(0);

	SekExit();
	if (!Snowbro3) ZetExit();

	GenericTilesExit();

	BurnFree(Mem);
	
	HyperpacNumTiles = 0;
	HyperpacNumTiles8bpp = 0;
	Wintbob = 0;
	
	Snowbro3Music = 0;
	Snowbro3MusicPlaying = 0;
	Snowbro3 = 0;

	return 0;
}

static void HyperpacRenderSpriteLayer()
{
	INT32 x = 0, y = 0, Offs;

	for (Offs = 0; Offs < 0x2000; Offs += 16) {
		INT32 dx         = ((HyperpacSpriteRam[Offs +  9]  << 8) + HyperpacSpriteRam[Offs +  8]) & 0xff;
		INT32 dy         = ((HyperpacSpriteRam[Offs + 11]  << 8) + HyperpacSpriteRam[Offs + 10]) & 0xff;
		INT32 TileColour = (HyperpacSpriteRam[Offs +  7]  << 8) + HyperpacSpriteRam[Offs +  6];
		INT32 Attr       = (HyperpacSpriteRam[Offs + 15]  << 8) + HyperpacSpriteRam[Offs + 14];
		INT32 Attr2      = (HyperpacSpriteRam[Offs + 13]  << 8) + HyperpacSpriteRam[Offs + 12];
		INT32 FlipX      = Attr & 0x80;
		INT32 FlipY      = (Attr & 0x40) << 1;
		INT32 Tile       = ((Attr & 0x3f) << 8) + (Attr2 & 0xff);
		INT32 Colour     = (TileColour & 0xf0) >> 4;

		if (TileColour & 1) dx = -1 - (dx ^ 0xff);
		if (TileColour & 2) dy = -1 - (dy ^ 0xff);
		if (TileColour & 4) {
			x += dx;
			y += dy;
		} else {
			x = dx;
			y = dy;
		}

		if (x > 511) x &= 0x1ff;
		if (y > 511) y &= 0x1ff;

		if (x > 15 && x < 240 && (y - 16) > 15 && (y - 16) <= 208) {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);

				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			}
		} else {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			}
		}
	}
}

static void TwinadvRenderSpriteLayer()
{
	INT32 x = 0, y = 0, Offs;

	for (Offs = 0; Offs < 0x2000; Offs += 16) {
		INT32 dx         = ((HyperpacSpriteRam[Offs +  9]  << 8) + HyperpacSpriteRam[Offs +  8]) & 0xff;
		INT32 dy         = ((HyperpacSpriteRam[Offs + 11]  << 8) + HyperpacSpriteRam[Offs + 10]) & 0xff;
		INT32 TileColour = (HyperpacSpriteRam[Offs +  7]  << 8) + HyperpacSpriteRam[Offs +  6];
		INT32 Attr       = (HyperpacSpriteRam[Offs + 15]  << 8) + HyperpacSpriteRam[Offs + 14];
		INT32 Attr2      = (HyperpacSpriteRam[Offs + 13]  << 8) + HyperpacSpriteRam[Offs + 12];
		INT32 FlipX      = Attr & 0x80;
		INT32 FlipY      = (Attr & 0x40) << 1;
		INT32 Tile       = ((Attr & 0x3f) << 8) + (Attr2 & 0xff);
		INT32 Colour     = (TileColour & 0xf0) >> 4;
		Colour        ^= 0xf;

		x = dx;
		y = dy;

		if (x > 511) x &= 0x1ff;
		if (y > 511) y &= 0x1ff;

		if (x > 15 && x < 240 && (y - 16) > 15 && (y - 16) <= 208) {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);

				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			}
		} else {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			}
		}
	}
}

static void HoneydolRenderSpriteLayer()
{
	INT32 x = 0, y = 0, Offs;

	for (Offs = 0; Offs < 0x2000; Offs += 16) {
		INT32 dx         = (((HyperpacSpriteRam[Offs +  9]  << 8) + HyperpacSpriteRam[Offs +  8]) >> 8) & 0xff;
		INT32 dy         = (((HyperpacSpriteRam[Offs + 11]  << 8) + HyperpacSpriteRam[Offs + 10]) >> 8) & 0xff;
		INT32 TileColour = (((HyperpacSpriteRam[Offs +  7]  << 8) + HyperpacSpriteRam[Offs +  6]) >> 8) & 3;
		INT32 Attr       = ((HyperpacSpriteRam[Offs + 15]  << 8) + HyperpacSpriteRam[Offs + 14]) >> 8;
		INT32 Attr2      = (((HyperpacSpriteRam[Offs + 13]  << 8) + HyperpacSpriteRam[Offs + 12]) >> 8) & 0xff;
		INT32 FlipX      = Attr & 0x80;
		INT32 FlipY      = (Attr & 0x40) << 1;
		INT32 Tile       = ((Attr & 0x3f) << 8) + (Attr2 & 0xff);
		INT32 Colour     = TileColour;

		x = dx;
		y = dy;

		if (x > 511) x &= 0x1ff;
		if (y > 511) y &= 0x1ff;
		
		if (x > 15 && x < 240 && (y - 16) > 15 && (y - 16) <= 208) {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask(pTransDraw, Tile, x, y - 16, Colour, 8, 0, 0, HyperpacSprites8bpp);

				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Tile, x, y - 16, Colour, 8, 0, 0, HyperpacSprites8bpp);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Tile, x, y - 16, Colour, 8, 0, 0, HyperpacSprites8bpp);
				} else {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Tile, x, y - 16, Colour, 8, 0, 0, HyperpacSprites8bpp);
				}
			}
		} else {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask_Clip(pTransDraw, Tile, x, y - 16, Colour, 8, 0, 0, HyperpacSprites8bpp);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Tile, x, y - 16, Colour, 8, 0, 0, HyperpacSprites8bpp);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Tile, x, y - 16, Colour, 8, 0, 0, HyperpacSprites8bpp);
				} else {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Tile, x, y - 16, Colour, 8, 0, 0, HyperpacSprites8bpp);
				}
			}
		}
		
		
		
		
		dx         = ((HyperpacSpriteRam[Offs +  9]  << 8) + HyperpacSpriteRam[Offs +  8]) & 0xff;
		dy         = ((HyperpacSpriteRam[Offs + 11]  << 8) + HyperpacSpriteRam[Offs + 10]) & 0xff;
		TileColour = (HyperpacSpriteRam[Offs +  7]  << 8) + HyperpacSpriteRam[Offs +  6];
		Attr       = (HyperpacSpriteRam[Offs + 15]  << 8) + HyperpacSpriteRam[Offs + 14];
		Attr2      = (HyperpacSpriteRam[Offs + 13]  << 8) + HyperpacSpriteRam[Offs + 12];
		FlipX      = Attr & 0x80;
		FlipY      = (Attr & 0x40) << 1;
		Tile       = ((Attr & 0x3f) << 8) + (Attr2 & 0xff);
		Colour     = (TileColour & 0x3f0) >> 4;
		Colour    ^= 0x3f;
		
		x = dx;
		y = dy;

		if (x > 511) x &= 0x1ff;
		if (y > 511) y &= 0x1ff;

		if (x > 15 && x < 240 && (y - 16) > 15 && (y - 16) <= 208) {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);

				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			}
		} else {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			}
		}
	}
}

static void SnowbrosRenderSpriteLayer()
{
	INT32 x = 0, y = 0, Offs;

	for (Offs = 0; Offs < 0x2000; Offs += 16) {
		INT32 dx         = ((HyperpacSpriteRam[Offs +  9]  << 8) + HyperpacSpriteRam[Offs +  8]) & 0xff;
		INT32 dy         = ((HyperpacSpriteRam[Offs + 11]  << 8) + HyperpacSpriteRam[Offs + 10]) & 0xff;
		INT32 TileColour = (HyperpacSpriteRam[Offs +  7]  << 8) + HyperpacSpriteRam[Offs +  6];
		INT32 Attr       = (HyperpacSpriteRam[Offs + 15]  << 8) + HyperpacSpriteRam[Offs + 14];
		INT32 Attr2      = (HyperpacSpriteRam[Offs + 13]  << 8) + HyperpacSpriteRam[Offs + 12];
		INT32 FlipX      = Attr & 0x80;
		INT32 FlipY      = (Attr & 0x40) << 1;
		INT32 Tile       = ((Attr & 0x1f) << 8) + (Attr2 & 0xff);
		INT32 Colour     = (TileColour & 0xf0) >> 4;

		if (TileColour & 1) dx = -1 - (dx ^ 0xff);
		if (TileColour & 2) dy = -1 - (dy ^ 0xff);
		if (TileColour & 4) {
			x += dx;
			y += dy;
		} else {
			x = dx;
			y = dy;
		}

		if (x > 511) x &= 0x1ff;
		if (y > 511) y &= 0x1ff;

		if (x > 15 && x < 240 && (y - 16) > 15 && (y - 16) <= 208) {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);

				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			}
		} else {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			}
		}
	}
}

static void WintbobRenderSpriteLayer()
{
	INT32 x = 0, y = 0, Offs;
	
	UINT16 *SpriteRam = (UINT16*)HyperpacSpriteRam;

	for (Offs = 0; Offs < 0x2000 >> 1; Offs += 8) {
		x              = SpriteRam[Offs + 0] & 0xff;
		y              = SpriteRam[Offs + 4] & 0xff;
		INT32 Attr       = SpriteRam[Offs + 1];
		INT32 Disbl      = Attr & 0x02;
		INT32 Wrapr      = Attr & 0x08;
		INT32 Colour     = (Attr & 0xf0) >> 4;
		INT32 Attr2      = SpriteRam[Offs + 2];
		INT32 Tile       = (Attr2 << 8) | (SpriteRam[Offs + 3] & 0xff);
		INT32 FlipX      = Attr2 & 0x80;
		INT32 FlipY      = (Attr2 & 0x40) << 1;

		if (Wrapr == 8) x -= 256;

		Tile &= 0xfff;

		if (Disbl == 2) continue;

		if (x > 15 && x < 240 && (y - 16) > 15 && (y - 16) <= 208) {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);

				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			}
		} else {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				} else {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Tile, x, y - 16, Colour, 4, 0, 0, HyperpacSprites);
				}
			}
		}
	}
}

static void Snowbro3RenderSpriteLayer()
{
	INT32 x = 0, y = 0, Offs;
	
	UINT16 *SpriteRam = (UINT16*)HyperpacSpriteRam;
	UINT8 *pTile = NULL;
	INT32 ColourDepth = 0;

	for (Offs = 0; Offs < 0x2200 >> 1; Offs += 8) {
		INT32 dx         = SpriteRam[Offs + 4] & 0xff;
		INT32 dy         = SpriteRam[Offs + 5] & 0xff;
		INT32 TileColour = SpriteRam[Offs + 3];
		INT32 Attr       = SpriteRam[Offs + 7];
		INT32 Attr2      = SpriteRam[Offs + 6];
		INT32 FlipX      = Attr & 0x80;
		INT32 FlipY      = (Attr & 0x40) << 1;
		INT32 Tile       = ((Attr & 0xff) << 8) + (Attr2 & 0xff);
		
		if (TileColour & 1) dx = -1 - (dx ^ 0xff);
		if (TileColour & 2) dy = -1 - (dy ^ 0xff);
		if (TileColour & 4) {
			x += dx;
			y += dy;
		} else {
			x = dx;
			y = dy;
		}

		if (x > 511) x &= 0x1ff;
		if (y > 511) y &= 0x1ff;
		
		if (Offs < 0x800) {
			TileColour = 0x10;
			Tile &= 0x3fff;
			ColourDepth = 8;
			pTile = HyperpacSprites8bpp;
		} else {
			Tile &= 0xfff;
			ColourDepth = 4;
			pTile = HyperpacSprites;
		}
		
		INT32 Colour     = (TileColour & 0xf0) >> 4;

		if (x > 15 && x < 240 && (y - 16) > 15 && (y - 16) <= 208) {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask(pTransDraw, Tile, x, y - 16, Colour, ColourDepth, 0, 0, pTile);

				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Tile, x, y - 16, Colour, ColourDepth, 0, 0, pTile);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Tile, x, y - 16, Colour, ColourDepth, 0, 0, pTile);
				} else {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Tile, x, y - 16, Colour, ColourDepth, 0, 0, pTile);
				}
			}
		} else {
			if (!FlipY) {
				if (!FlipX) {
					Render16x16Tile_Mask_Clip(pTransDraw, Tile, x, y - 16, Colour, ColourDepth, 0, 0, pTile);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Tile, x, y - 16, Colour, ColourDepth, 0, 0, pTile);
				}
			} else {
				if (!FlipX) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Tile, x, y - 16, Colour, ColourDepth, 0, 0, pTile);
				} else {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Tile, x, y - 16, Colour, ColourDepth, 0, 0, pTile);
				}
			}
		}
	}
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour >>  0) & 0x1f;
	g = (nColour >>  5) & 0x1f;
	b = (nColour >> 10) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	return BurnHighCol(r, g, b, 0);
}

static INT32 HyperpacCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)HyperpacPaletteRam, pd = HyperpacPalette; i < 0x200; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}

	return 0;
}

static INT32 HoneydolCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)HyperpacPaletteRam, pd = HyperpacPalette; i < 0x800; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}

	return 0;
}

static INT32 Snowbro3CalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)HyperpacPaletteRam, pd = HyperpacPalette; i < 0x400; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}

	return 0;
}

static void HyperpacRender()
{
	HyperpacCalcPalette();
	
	for (INT32 i = 0; i < nScreenHeight * nScreenWidth; i++) {
		pTransDraw[i] = 0xf0;	
	}
	
	HyperpacRenderSpriteLayer();
	BurnTransferCopy(HyperpacPalette);
}

static void PzlbreakRender()
{
	HyperpacCalcPalette();
	
	for (INT32 i = 0; i < nScreenHeight * nScreenWidth; i++) {
		pTransDraw[i] = 0xc0;	
	}
	
	HyperpacRenderSpriteLayer();
	BurnTransferCopy(HyperpacPalette);
}

static void TwinadvRender()
{
	HyperpacCalcPalette();
	
	for (INT32 i = 0; i < nScreenHeight * nScreenWidth; i++) {
		pTransDraw[i] = 0xf0;	
	}
	
	TwinadvRenderSpriteLayer();
	BurnTransferCopy(HyperpacPalette);
}

static void HoneydolRender()
{
	HoneydolCalcPalette();
	
	for (INT32 i = 0; i < nScreenHeight * nScreenWidth; i++) {
		pTransDraw[i] = 0xf0;	
	}
	
	HoneydolRenderSpriteLayer();
	BurnTransferCopy(HyperpacPalette);
}

static void SnowbrosRender()
{
	HyperpacCalcPalette();
	
	for (INT32 i = 0; i < nScreenHeight * nScreenWidth; i++) {
		pTransDraw[i] = 0xf0;	
	}
	
	if (Wintbob) {
		WintbobRenderSpriteLayer();
	} else {
		SnowbrosRenderSpriteLayer();
	}
	BurnTransferCopy(HyperpacPalette);
}

static void Snowbro3Render()
{
	Snowbro3CalcPalette();
	
	for (INT32 i = 0; i < nScreenHeight * nScreenWidth; i++) {
		pTransDraw[i] = 0xf0;
	
	}
	
	Snowbro3RenderSpriteLayer();
	BurnTransferCopy(HyperpacPalette);
}

static INT32 HyperpacFrame()
{
	INT32 nInterleave = 4;

	if (HyperpacReset) HyperpacDoReset();

	HyperpacMakeInputs();

	nCyclesTotal[0] = 16000000 / 60;
	nCyclesTotal[1] = 4000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	INT32 nSoundBufferPos = 0;

	SekNewFrame();
	ZetNewFrame();

	SekOpen(0);
	ZetOpen(0);
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);

		// Run Z80
		nCurrentCPU = 1;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}

		if (i == 1) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		if (i == 2) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		if (i == 3) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
	}

	SekClose();

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
		}
	}
	
	ZetClose();

	if (pBurnDraw) HyperpacRender();

	return 0;
}

static INT32 PzlbreakFrame()
{
	INT32 nInterleave = 4;

	if (HyperpacReset) HyperpacDoReset();

	HyperpacMakeInputs();

	nCyclesTotal[0] = 16000000 / 60;
	nCyclesTotal[1] = 4000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	INT32 nSoundBufferPos = 0;

	SekNewFrame();
	ZetNewFrame();

	SekOpen(0);
	ZetOpen(0);
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);

		// Run Z80
		nCurrentCPU = 1;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}

		if (i == 1) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		if (i == 2) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		if (i == 3) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
	}

	SekClose();

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
		}
	}
	
	ZetClose();

	if (pBurnDraw) PzlbreakRender();

	return 0;
}

static INT32 FinalttrFrame()
{
	INT32 nInterleave = 4;

	if (HyperpacReset) HyperpacDoReset();

	HyperpacMakeInputs();

	nCyclesTotal[0] = 12000000 / 60;
	nCyclesTotal[1] = 3578545 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	INT32 nSoundBufferPos = 0;

	SekNewFrame();
	ZetNewFrame();

	SekOpen(0);
	ZetOpen(0);
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);

		// Run Z80
		nCurrentCPU = 1;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}

		if (i == 1) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		if (i == 2) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		if (i == 3) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
	}

	SekClose();

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
		}
	}
	
	ZetClose();

	if (pBurnDraw) HyperpacRender();

	return 0;
}

static INT32 TwinadvFrame()
{
	INT32 nInterleave = 4;

	if (HyperpacReset) HyperpacDoReset();

	HyperpacMakeInputs();

	nCyclesTotal[0] = 16000000 / 60;
	nCyclesTotal[1] = 4000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	INT32 nSoundBufferPos = 0;

	SekNewFrame();
	ZetNewFrame();

	SekOpen(0);
	ZetOpen(0);
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);

		// Run Z80
		nCurrentCPU = 1;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		if (i == 2) ZetRaiseIrq(0);

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}

		if (i == 1) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		if (i == 2) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		if (i == 3) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
	}

	SekClose();
	ZetClose();

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			MSM6295Render(0, pSoundBuf, nSegmentLength);
		}
	}

	if (pBurnDraw) TwinadvRender();

	return 0;
}

static INT32 HoneydolFrame()
{
	INT32 nInterleave = 4;

	if (HyperpacReset) HyperpacDoReset();

	HyperpacMakeInputs();

	SekNewFrame();
	ZetNewFrame();
	
	SekOpen(0);
	ZetOpen(0);

	SekIdle(nCyclesDone[0]);
	ZetIdle(nCyclesDone[1]);

	nCyclesTotal[0] = 16000000 / 60;
	nCyclesTotal[1] = 4000000 / 60;

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - SekTotalCycles();
		SekRun(nCyclesSegment);

		if (i == 1) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		if (i == 2) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		if (i == 3) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
	}

	nCycles68KSync = SekTotalCycles();
	BurnTimerEndFrameYM3812(nCyclesTotal[1]);
	if (pBurnSoundOut) {
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}
	
	nCyclesDone[0] = SekTotalCycles() - nCyclesTotal[0];
	nCyclesDone[1] = ZetTotalCycles() - nCyclesTotal[1];

//	bprintf(PRINT_NORMAL, _T("%i\ %i\n"), nCyclesDone[0], nCyclesDone[1]);

	ZetClose();
	SekClose();

	if (pBurnDraw) HoneydolRender();

	return 0;
}

static INT32 SnowbrosFrame()
{
	INT32 nInterleave = 4;

	if (HyperpacReset) SnowbrosDoReset();

	HyperpacMakeInputs();

	SekNewFrame();
	ZetNewFrame();	
	
	SekOpen(0);
	ZetOpen(0);	

	SekIdle(nCyclesDone[0]);
	ZetIdle(nCyclesDone[1]);

	nCyclesTotal[0] = (Wintbob) ? 12000000 / 60 : 8000000 / 60;
	nCyclesTotal[1] = 6000000 / 60;

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - SekTotalCycles();
		SekRun(nCyclesSegment);

		if (i == 1) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		if (i == 2) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		if (i == 3) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
	}

	nCycles68KSync = SekTotalCycles();
	BurnTimerEndFrameYM3812(nCyclesTotal[1]);
	if (pBurnSoundOut) BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);

	nCyclesDone[0] = SekTotalCycles() - nCyclesTotal[0];
	nCyclesDone[1] = ZetTotalCycles() - nCyclesTotal[1];

//	bprintf(PRINT_NORMAL, _T("%i\ %i\n"), nCyclesDone[0], nCyclesDone[1]);

	ZetClose();
	SekClose();

	if (pBurnDraw) SnowbrosRender();

	return 0;
}

static INT32 Snowbro3Frame()
{
	INT32 nInterleave = 4;

	if (HyperpacReset) Snowbro3DoReset();

	HyperpacMakeInputs();

	SekOpen(0);

	SekNewFrame();

	SekIdle(nCyclesDone[0]);

	nCyclesTotal[0] = 16000000 / 60;

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - SekTotalCycles();
		SekRun(nCyclesSegment);

		if (i == 1) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		if (i == 2) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		if (i == 3) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		
		INT32 Status = MSM6295ReadStatus(0);
		if (Snowbro3MusicPlaying) {
			if ((Status & 0x08) == 0x00) {
				MSM6295Command(0, 0x80 | Snowbro3Music);
				MSM6295Command(0, 0x82);
			}
		} else {
			if ((Status & 0x08) == 0x00) {
				MSM6295Command(0, 0x40);
			}
		}
	}

	nCyclesDone[0] = SekTotalCycles() - nCyclesTotal[0];

	SekClose();
	
	if (pBurnSoundOut) MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);

	if (pBurnDraw) Snowbro3Render();

	return 0;
}

static INT32 HyperpacScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin = 0x02944;
	}

	if (nAction & ACB_MEMORY_RAM) {								// Scan all memory, devices & variables
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {	
		SekScan(nAction);				// Scan 68000
		ZetScan(nAction);					// Scan Z80

		MSM6295Scan(0, nAction);			// Scan OKIM6295
		BurnYM2151Scan(nAction);

		// Scan critical driver variables
		SCAN_VAR(HyperpacSoundLatch);
		SCAN_VAR(HyperpacInput);
		SCAN_VAR(HyperpacDip);
	}

	return 0;
}

static INT32 SnowbrosScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {				// Return minimum compatible version
		*pnMin = 0x029519;
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

		SCAN_VAR(nCyclesDone);

		BurnYM3812Scan(nAction, pnMin);

		// Scan critical driver variables
		SCAN_VAR(HyperpacSoundLatch);
		SCAN_VAR(HyperpacInput);
		SCAN_VAR(HyperpacDip);
	}

	return 0;
}

static INT32 Snowbro3Scan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin = 0x029672;
	}

	if (nAction & ACB_MEMORY_RAM) {								// Scan all memory, devices & variables
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {	
		SekScan(nAction);				// Scan 68000

		MSM6295Scan(0, nAction);			// Scan OKIM6295

		// Scan critical driver variables
		SCAN_VAR(HyperpacSoundLatch);
		SCAN_VAR(HyperpacInput);
		SCAN_VAR(HyperpacDip);
		SCAN_VAR(Snowbro3Music);
		SCAN_VAR(Snowbro3MusicPlaying);
	}

	return 0;
}

struct BurnDriver BurnDrvHyperpac = {
	"hyperpac", NULL, NULL, NULL, "1995",
	"Hyper Pacman\0", NULL, "SemiCom", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MAZE, 0,
	NULL, HyperpacRomInfo, HyperpacRomName, NULL, NULL, HyperpacInputInfo, HyperpacDIPInfo,
	HyperpacInit, HyperpacExit, HyperpacFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvHyperpacb = {
	"hyperpacb", "hyperpac", NULL, NULL, "1995",
	"Hyper Pacman (bootleg)\0", NULL, "SemiCom", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_POST90S, GBF_MAZE, 0,
	NULL, HyperpacbRomInfo, HyperpacbRomName, NULL, NULL, HyperpacInputInfo, HyperpacDIPInfo,
	HyperpacInit, HyperpacExit, HyperpacFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvCookbib2 = {
	"cookbib2", NULL, NULL, NULL, "1996",
	"Cookie & Bibi 2\0", NULL, "SemiCom", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, Cookbib2RomInfo, Cookbib2RomName, NULL, NULL, HyperpacInputInfo, Cookbib2DIPInfo,
	Cookbib2Init, HyperpacExit, HyperpacFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvCookbib3 = {
	"cookbib3", NULL, NULL, NULL, "1997",
	"Cookie & Bibi 3\0", NULL, "SemiCom", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, Cookbib3RomInfo, Cookbib3RomName, NULL, NULL, HyperpacInputInfo, Cookbib3DIPInfo,
	Cookbib3Init, HyperpacExit, HyperpacFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvMoremore = {
	"moremore", NULL, NULL, NULL, "1999",
	"More More\0", NULL, "SemiCom / Exit", "Kaneko Pandora based",
	L"More More\0\uBAA8\uC544\uBAA8\uC544 More More\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, MoremoreRomInfo, MoremoreRomName, NULL, NULL, HyperpacInputInfo, MoremoreDIPInfo,
	MoremoreInit, HyperpacExit, HyperpacFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvMoremorp = {
	"moremorp", NULL, NULL, NULL, "1999",
	"More More Plus\0", NULL, "SemiCom / Exit", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, MoremorpRomInfo, MoremorpRomName, NULL, NULL, HyperpacInputInfo, MoremoreDIPInfo,
	MoremoreInit, HyperpacExit, HyperpacFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvThreein1semi = {
	"3in1semi", NULL, NULL, NULL, "1997",
	"XESS - The New Revolution (SemiCom 3-in-1)\0", NULL, "SemiCom", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, Threein1semiRomInfo, Threein1semiRomName, NULL, NULL, HyperpacInputInfo, MoremoreDIPInfo,
	MoremoreInit, HyperpacExit, HyperpacFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvToppyrap = {
	"toppyrap", NULL, NULL, NULL, "1996",
	"Toppy & Rappy\0", NULL, "SemiCom", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, ToppyrapRomInfo, ToppyrapRomName, NULL, NULL, HyperpacInputInfo, ToppyrapDIPInfo,
	MoremoreInit, HyperpacExit, HyperpacFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvTwinkle = {
	"twinkle", NULL, NULL, NULL, "1997",
	"Twinkle\0", NULL, "SemiCom", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MAZE, 0,
	NULL, TwinkleRomInfo, TwinkleRomName, NULL, NULL, HyperpacInputInfo, MoremoreDIPInfo,
	TwinkleInit, HyperpacExit, HyperpacFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvPzlbreak = {
	"pzlbreak", NULL, NULL, NULL, "1997",
	"Puzzle Break\0", NULL, "SemiCom", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MAZE, 0,
	NULL, PzlbreakRomInfo, PzlbreakRomName, NULL, NULL, HyperpacInputInfo, MoremoreDIPInfo,
	PzlbreakInit, HyperpacExit, PzlbreakFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvFourin1boot = {
	"4in1boot", NULL, NULL, NULL, "2002",
	"Puzzle King\0", NULL, "K1 Soft", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_BOOTLEG, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, Fourin1bootRomInfo, Fourin1bootRomName, NULL, NULL, HyperpacInputInfo, Fourin1bootDIPInfo,
	Fourin1bootInit, HyperpacExit, HyperpacFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvFinalttr = {
	"finalttr", NULL, NULL, NULL, "1993",
	"Final Tetris\0", NULL, "Jeil Computer System", "Kaneko Pandora based",
	L"Final Tetris\0\uD30C\uC774\uB110 \uD14C\uD2B8\uB9AC\uC2A4\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, FinalttrRomInfo, FinalttrRomName, NULL, NULL, HyperpacInputInfo, FinalttrDIPInfo,
	FinalttrInit, HyperpacExit, FinalttrFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvTwinadv = {
	"twinadv", NULL, NULL, NULL, "1995",
	"Twin Adventure (World)\0", NULL, "Barko Corp", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, TwinadvRomInfo, TwinadvRomName, NULL, NULL, HyperpacInputInfo, TwinadvDIPInfo,
	TwinadvInit, HyperpacExit, TwinadvFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvTwinadvk = {
	"twinadvk", "twinadv", NULL, NULL, "1995",
	"Twin Adventure (Korea)\0", NULL, "Barko Corp", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, TwinadvkRomInfo, TwinadvkRomName, NULL, NULL, HyperpacInputInfo, TwinadvDIPInfo,
	TwinadvInit, HyperpacExit, TwinadvFrame, NULL, HyperpacScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvHoneydol = {
	"honeydol", NULL, NULL, NULL, "1995",
	"Honey Dolls\0", NULL, "Barko Corp", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, HoneydolRomInfo, HoneydolRomName, NULL, NULL, HyperpacInputInfo, HoneydolDIPInfo,
	HoneydolInit, HyperpacExit, HoneydolFrame, NULL, HyperpacScan,
	NULL, 0x800, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSnowbros = {
	"snowbros", NULL, NULL, NULL, "1990",
	"Snow Bros. - Nick & Tom (set 1)\0", NULL, "Toaplan", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, SnowbrosRomInfo, SnowbrosRomName, NULL, NULL, SnowbrosInputInfo, SnowbrosDIPInfo,
	SnowbrosInit, SnowbrosExit, SnowbrosFrame, NULL, SnowbrosScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSnowbroa = {
	"snowbrosa", "snowbros", NULL, NULL, "1990",
	"Snow Bros. - Nick & Tom (set 2)\0", NULL, "Toaplan", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, SnowbroaRomInfo, SnowbroaRomName, NULL, NULL, SnowbrosInputInfo, SnowbrosDIPInfo,
	SnowbrosInit, SnowbrosExit, SnowbrosFrame, NULL, SnowbrosScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSnowbrob = {
	"snowbrosb", "snowbros", NULL, NULL, "1990",
	"Snow Bros. - Nick & Tom (set 3)\0", NULL, "Toaplan", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, SnowbrobRomInfo, SnowbrobRomName, NULL, NULL, SnowbrosInputInfo, SnowbrosDIPInfo,
	SnowbrosInit, SnowbrosExit, SnowbrosFrame, NULL, SnowbrosScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSnowbroc = {
	"snowbrosc", "snowbros", NULL, NULL, "1990",
	"Snow Bros. - Nick & Tom (set 4)\0", NULL, "Toaplan", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, SnowbrocRomInfo, SnowbrocRomName, NULL, NULL, SnowbrosInputInfo, SnowbrosDIPInfo,
	SnowbrosInit, SnowbrosExit, SnowbrosFrame, NULL, SnowbrosScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSnowbroj = {
	"snowbrosj", "snowbros", NULL, NULL, "1990",
	"Snow Bros. - Nick & Tom (Japan)\0", NULL, "Toaplan", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, SnowbrojRomInfo, SnowbrojRomName, NULL, NULL, SnowbrosInputInfo, SnowbrojDIPInfo,
	SnowbrosInit, SnowbrosExit, SnowbrosFrame, NULL, SnowbrosScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSnowbrod = {
	"snowbrosd", "snowbros", NULL, NULL, "1990",
	"Snow Bros. - Nick & Tom (Dooyong license)\0", NULL, "Toaplan (Dooyong license)", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, SnowbrodRomInfo, SnowbrodRomName, NULL, NULL, SnowbrosInputInfo, SnowbrojDIPInfo,
	SnowbrosInit, SnowbrosExit, SnowbrosFrame, NULL, SnowbrosScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvWintbob = {
	"wintbob", "snowbros", NULL, NULL, "1990",
	"The Winter Bobble (bootleg)\0", NULL, "bootleg", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, WintbobRomInfo, WintbobRomName, NULL, NULL, SnowbrosInputInfo, SnowbrosDIPInfo,
	SnowbrosInit, SnowbrosExit, SnowbrosFrame, NULL, SnowbrosScan,
	NULL, 0x200, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSnowbro3 = {
	"snowbros3", "snowbros", NULL, NULL, "2002",
	"Snow Brothers 3 - Magical Adventure\0", NULL, "bootleg", "Kaneko Pandora based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, Snowbro3RomInfo, Snowbro3RomName, NULL, NULL, SnowbrosInputInfo, SnowbrojDIPInfo,
	Snowbro3Init, SnowbrosExit, Snowbro3Frame, NULL, Snowbro3Scan,
	NULL, 0x400, 256, 224, 4, 3
};
