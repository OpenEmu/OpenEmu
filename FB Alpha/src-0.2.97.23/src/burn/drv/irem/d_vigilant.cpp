#include "tiles_generic.h"
#include "z80_intf.h"
#include "burn_ym2151.h"
#include "burn_ym2203.h"
#include "dac.h"

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
static UINT8 *DrvZ80Ram1          = NULL;
static UINT8 *DrvZ80Ram2          = NULL;
static UINT8 *DrvVideoRam         = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvPaletteRam       = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvBackTiles        = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvSamples          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette         = NULL;

static UINT8 DrvRomBank;
static UINT8 DrvSoundLatch;
static UINT8 DrvIrqVector;

static INT32 DrvRearColour;
static INT32 DrvRearDisable;
static INT32 DrvHorizScrollLo;
static INT32 DrvHorizScrollHi;
static INT32 DrvRearHorizScrollLo;
static INT32 DrvRearHorizScrollHi;
static INT32 DrvSampleAddress;

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static UINT8 DrvHasYM2203 = 0;
static UINT8 DrvKikcubicDraw = 0;

#define VECTOR_INIT		0
#define YM2151_ASSERT		1
#define YM2151_CLEAR		2
#define Z80_ASSERT		3
#define Z80_CLEAR		4

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , DrvInputPort1 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , DrvInputPort1 + 5, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , DrvInputPort1 + 7, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 7, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Drv)

static struct BurnInputInfo BuccanrsInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 6, "p2 start"  },

	{"Up"                , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , DrvInputPort1 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , DrvInputPort1 + 7, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , DrvInputPort1 + 5, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 7, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Buccanrs)

static struct BurnInputInfo BuccanrsaInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , DrvInputPort1 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , DrvInputPort1 + 7, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , DrvInputPort1 + 5, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 7, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Buccanrsa)

static struct BurnInputInfo KikcubicInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 5, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , DrvInputPort1 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , DrvInputPort1 + 7, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , DrvInputPort1 + 5, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 7, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Kikcubic)

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
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xfd, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x02, "2"                      },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x01, "4"                      },
	{0x12, 0x01, 0x03, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x12, 0x01, 0x04, 0x04, "Normal"                 },
	{0x12, 0x01, 0x04, 0x00, "Hard"                   },
	
	{0   , 0xfe, 0   , 2   , "Energy Decrease Rate"   },
	{0x12, 0x01, 0x08, 0x08, "Slow"                   },
	{0x12, 0x01, 0x08, 0x00, "Fast"                   },
	
	{0   , 0xfe, 0   , 16  , "Coinage"                },
	{0x12, 0x01, 0xf0, 0xa0, "6 Coins 1 Play"         },
	{0x12, 0x01, 0xf0, 0xb0, "5 Coins 1 Play"         },
	{0x12, 0x01, 0xf0, 0xc0, "4 Coins 1 Play"         },
	{0x12, 0x01, 0xf0, 0xd0, "3 Coins 1 Play"         },
	{0x12, 0x01, 0xf0, 0x10, "8 Coins 3 Plays"        },
	{0x12, 0x01, 0xf0, 0xe0, "2 Coins 1 Play"         },
	{0x12, 0x01, 0xf0, 0x20, "5 Coins 3 Plays"        },
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 2 Plays"        },
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Play"         },
	{0x12, 0x01, 0xf0, 0x40, "2 Coins 3 Plays"        },
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  2 Plays"        },
	{0x12, 0x01, 0xf0, 0x80, "1 Coin  3 Plays"        },
	{0x12, 0x01, 0xf0, 0x70, "1 Coin  4 Plays"        },
	{0x12, 0x01, 0xf0, 0x60, "1 Coin  5 Plays"        },
	{0x12, 0x01, 0xf0, 0x50, "1 Coin  6 Plays"        },
	{0x12, 0x01, 0xf0, 0x00, "Freeplay"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x02, 0x00, "Upright"                },
	{0x13, 0x01, 0x02, 0x02, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Coin Mode"              },
	{0x13, 0x01, 0x04, 0x04, "Mode 1"                 },
	{0x13, 0x01, 0x04, 0x00, "Mode 2"                 },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x08, 0x00, "Off"                    },
	{0x13, 0x01, 0x08, 0x08, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x13, 0x01, 0x10, 0x00, "Off"                    },
	{0x13, 0x01, 0x10, 0x10, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Stop Mode"              },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo BuccanrsDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0x2f, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0x7f, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x11, 0x01, 0x01, 0x01, "Off"                    },
	{0x11, 0x01, 0x01, 0x00, "On"                     },
	
	// Dip 2	
	{0   , 0xfe, 0   , 12  , "Coin A"                 },
	{0x12, 0x01, 0x0f, 0x04, "4 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x07, "3 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x00, "5 Coins 2 Plays"        },
	{0x12, 0x01, 0x0f, 0x0a, "2 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x06, "3 Coins 2 Plays"        },
	{0x12, 0x01, 0x0f, 0x03, "4 Coins 3 Plays"        },
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Play"         },
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 5 Plays"        },
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Plays"        },
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Plays"        },
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Plays"        },
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Plays"        },
	
	{0   , 0xfe, 0   , 12  , "Coin B"                 },
	{0x12, 0x01, 0xf0, 0x40, "4 Coins 1 Play"         },
	{0x12, 0x01, 0xf0, 0x70, "3 Coins 1 Play"         },
	{0x12, 0x01, 0xf0, 0x00, "5 Coins 2 Plays"        },
	{0x12, 0x01, 0xf0, 0xa0, "2 Coins 1 Play"         },
	{0x12, 0x01, 0xf0, 0x60, "3 Coins 2 Plays"        },
	{0x12, 0x01, 0xf0, 0x30, "4 Coins 3 Plays"        },
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Play"         },
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 5 Plays"        },
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Plays"        },
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Plays"        },
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Plays"        },
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Plays"        },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x06, 0x06, "2"                      },
	{0x13, 0x01, 0x06, 0x04, "3"                      },
	{0x13, 0x01, 0x06, 0x02, "4"                      },
	{0x13, 0x01, 0x06, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0x18, 0x18, "Normal"                 },
	{0x13, 0x01, 0x18, 0x08, "Medium"                 },
	{0x13, 0x01, 0x18, 0x10, "Hard"                   },
	{0x13, 0x01, 0x18, 0x00, "Invincibilty"           },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x20, 0x20, "Upright"                },
	{0x13, 0x01, 0x20, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x40, 0x00, "Off"                    },
	{0x13, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Buccanrs)

static struct BurnDIPInfo BuccanrsaDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xf4, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0x7f, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x11, 0x01, 0xf4, 0xf4, "Off"                    },
	{0x11, 0x01, 0xf4, 0x00, "On"                     },
	
	// Dip 2	
	{0   , 0xfe, 0   , 12  , "Coin A"                 },
	{0x12, 0x01, 0x0f, 0x04, "4 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x07, "3 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x00, "5 Coins 2 Plays"        },
	{0x12, 0x01, 0x0f, 0x0a, "2 Coins 1 Play"         },
	{0x12, 0x01, 0x0f, 0x06, "3 Coins 2 Plays"        },
	{0x12, 0x01, 0x0f, 0x03, "4 Coins 3 Plays"        },
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Play"         },
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 5 Plays"        },
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Plays"        },
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Plays"        },
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Plays"        },
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Plays"        },
	
	{0   , 0xfe, 0   , 12  , "Coin B"                 },
	{0x12, 0x01, 0xf0, 0x40, "4 Coins 1 Play"         },
	{0x12, 0x01, 0xf0, 0x70, "3 Coins 1 Play"         },
	{0x12, 0x01, 0xf0, 0x00, "5 Coins 2 Plays"        },
	{0x12, 0x01, 0xf0, 0xa0, "2 Coins 1 Play"         },
	{0x12, 0x01, 0xf0, 0x60, "3 Coins 2 Plays"        },
	{0x12, 0x01, 0xf0, 0x30, "4 Coins 3 Plays"        },
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Play"         },
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 5 Plays"        },
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Plays"        },
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Plays"        },
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Plays"        },
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Plays"        },
	
	// Dip 3
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x06, 0x06, "2"                      },
	{0x13, 0x01, 0x06, 0x04, "3"                      },
	{0x13, 0x01, 0x06, 0x02, "4"                      },
	{0x13, 0x01, 0x06, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0x18, 0x18, "Normal"                 },
	{0x13, 0x01, 0x18, 0x08, "Medium"                 },
	{0x13, 0x01, 0x18, 0x10, "Hard"                   },
	{0x13, 0x01, 0x18, 0x00, "Invincibilty"           },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x13, 0x01, 0x20, 0x20, "Upright"                },
	{0x13, 0x01, 0x20, 0x00, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x40, 0x00, "Off"                    },
	{0x13, 0x01, 0x40, 0x40, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Buccanrsa)

static struct BurnDIPInfo KikcubicDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0xd5, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x11, 0x01, 0x03, 0x02, "Easy"                   },
	{0x11, 0x01, 0x03, 0x03, "Medium"                 },
	{0x11, 0x01, 0x03, 0x01, "Hard"                   },
	{0x11, 0x01, 0x03, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x0c, 0x08, "1"                      },
	{0x11, 0x01, 0x0c, 0x04, "2"                      },
	{0x11, 0x01, 0x0c, 0x0c, "3"                      },
	{0x11, 0x01, 0x0c, 0x00, "4"                      },
	
	{0   , 0xfe, 0   , 12  , "Coinage"                },
	{0x11, 0x01, 0xf0, 0xa0, "6 Coins 1 Play"         },
	{0x11, 0x01, 0xf0, 0xb0, "5 Coins 1 Play"         },
	{0x11, 0x01, 0xf0, 0xc0, "4 Coins 1 Play"         },
	{0x11, 0x01, 0xf0, 0xd0, "3 Coins 1 Play"         },
	{0x11, 0x01, 0xf0, 0xe0, "2 Coins 1 Play"         },
	{0x11, 0x01, 0xf0, 0xf0, "1 Coin  1 Play"         },
	{0x11, 0x01, 0xf0, 0x70, "1 Coin  2 Plays"        },
	{0x11, 0x01, 0xf0, 0x60, "1 Coin  3 Plays"        },
	{0x11, 0x01, 0xf0, 0x50, "1 Coin  4 Plays"        },
	{0x11, 0x01, 0xf0, 0x40, "1 Coin  5 Plays"        },
	{0x11, 0x01, 0xf0, 0x30, "1 Coin  6 Plays"        },
	{0x11, 0x01, 0xf0, 0x00, "Freeplay"               },
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x12, 0x01, 0x01, 0x01, "Off"                    },
	{0x12, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x02, 0x00, "Upright"                },
	{0x12, 0x01, 0x02, 0x02, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Coin Mode"              },
	{0x12, 0x01, 0x04, 0x04, "Mode 1"                 },
	{0x12, 0x01, 0x04, 0x00, "Mode 2"                 },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x08, 0x08, "Off"                    },
	{0x12, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x12, 0x01, 0x10, 0x10, "Off"                    },
	{0x12, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Level Select"           },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Player Adding"          },
	{0x12, 0x01, 0x40, 0x40, "Off"                    },
	{0x12, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Kikcubic)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "g07_c03.bin",   0x08000, 0x9dcca081, BRF_ESS | BRF_PRG },	//  0	Z80 #1 Program Code
	{ "j07_c04.bin",   0x10000, 0xe0159105, BRF_ESS | BRF_PRG },	//  1
	
	{ "g05_c02.bin",   0x10000, 0x10582b2d, BRF_ESS | BRF_PRG },	//  2	Z80 #2 Program 
	
	{ "f05_c08.bin",   0x10000, 0x01579d20, BRF_GRA },		//  3	Characters
	{ "h05_c09.bin",   0x10000, 0x4f5872f0, BRF_GRA },		//  4
	
	{ "n07_c12.bin",   0x10000, 0x10af8eb2, BRF_GRA },		//  5	Sprites
	{ "k07_c10.bin",   0x10000, 0x9576f304, BRF_GRA },		//  6
	{ "o07_c13.bin",   0x10000, 0xb1d9d4dc, BRF_GRA },		//  7
	{ "l07_c11.bin",   0x10000, 0x4598be4a, BRF_GRA },		//  8
	{ "t07_c16.bin",   0x10000, 0xf5425e42, BRF_GRA },		//  9
	{ "p07_c14.bin",   0x10000, 0xcb50a17c, BRF_GRA },		//  10
	{ "v07_c17.bin",   0x10000, 0x959ba3c7, BRF_GRA },		//  11
	{ "s07_c15.bin",   0x10000, 0x7f2e91c5, BRF_GRA },		//  12
	
	{ "d01_c05.bin",   0x10000, 0x81b1ee5c, BRF_GRA },		//  13	Background
	{ "e01_c06.bin",   0x10000, 0xd0d33673, BRF_GRA },		//  14
	{ "f01_c07.bin",   0x10000, 0xaae81695, BRF_GRA },		//  15
	
	{ "d04_c01.bin",   0x10000, 0x9b85101d, BRF_SND },		//  16	Samples
	
	{ "vg_b-8r.ic90",  0x00117, 0xdf368a7a, BRF_GRA },		//  17	PALs
	{ "vg_b-4m.ic38",  0x00117, 0xdbca4204, BRF_GRA },		//  18
	{ "vg_b-1b.ic1",   0x00117, 0x922e5167, BRF_GRA },		//  19
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo Drv1RomDesc[] = {
	{ "vg_a-8h-c.rom", 0x08000, 0xd72682e8, BRF_ESS | BRF_PRG },	//  0	Z80 #1 Program Code
	{ "vg_a-8l-a.rom", 0x10000, 0x690d812f, BRF_ESS | BRF_PRG },	//  1
	
	{ "g05_c02.bin",   0x10000, 0x10582b2d, BRF_ESS | BRF_PRG },	//  2	Z80 #2 Program 
	
	{ "f05_c08.bin",   0x10000, 0x01579d20, BRF_GRA },		//  3	Characters
	{ "h05_c09.bin",   0x10000, 0x4f5872f0, BRF_GRA },		//  4
	
	{ "n07_c12.bin",   0x10000, 0x10af8eb2, BRF_GRA },		//  5	Sprites
	{ "k07_c10.bin",   0x10000, 0x9576f304, BRF_GRA },		//  6
	{ "o07_c13.bin",   0x10000, 0xb1d9d4dc, BRF_GRA },		//  7
	{ "l07_c11.bin",   0x10000, 0x4598be4a, BRF_GRA },		//  8
	{ "t07_c16.bin",   0x10000, 0xf5425e42, BRF_GRA },		//  9
	{ "p07_c14.bin",   0x10000, 0xcb50a17c, BRF_GRA },		//  10
	{ "v07_c17.bin",   0x10000, 0x959ba3c7, BRF_GRA },		//  11
	{ "s07_c15.bin",   0x10000, 0x7f2e91c5, BRF_GRA },		//  12
	
	{ "d01_c05.bin",   0x10000, 0x81b1ee5c, BRF_GRA },		//  13	Background
	{ "e01_c06.bin",   0x10000, 0xd0d33673, BRF_GRA },		//  14
	{ "f01_c07.bin",   0x10000, 0xaae81695, BRF_GRA },		//  15
	
	{ "d04_c01.bin",   0x10000, 0x9b85101d, BRF_SND },		//  16	Samples
	
	{ "vg_b-8r.ic90",  0x00117, 0xdf368a7a, BRF_GRA },		//  17	PALs
	{ "vg_b-4m.ic38",  0x00117, 0xdbca4204, BRF_GRA },		//  18
	{ "vg_b-1b.ic1",   0x00117, 0x922e5167, BRF_GRA },		//  19
};

STD_ROM_PICK(Drv1)
STD_ROM_FN(Drv1)

static struct BurnRomInfo DrvuRomDesc[] = {
	{ "a-8h",          0x08000, 0x8d15109e, BRF_ESS | BRF_PRG },	//  0	Z80 #1 Program Code
	{ "a-8l",          0x10000, 0x7f95799b, BRF_ESS | BRF_PRG },	//  1
	
	{ "g05_c02.bin",   0x10000, 0x10582b2d, BRF_ESS | BRF_PRG },	//  2	Z80 #2 Program 
	
	{ "f05_c08.bin",   0x10000, 0x01579d20, BRF_GRA },		//  3	Characters
	{ "h05_c09.bin",   0x10000, 0x4f5872f0, BRF_GRA },		//  4
	
	{ "n07_c12.bin",   0x10000, 0x10af8eb2, BRF_GRA },		//  5	Sprites
	{ "k07_c10.bin",   0x10000, 0x9576f304, BRF_GRA },		//  6
	{ "o07_c13.bin",   0x10000, 0xb1d9d4dc, BRF_GRA },		//  7
	{ "l07_c11.bin",   0x10000, 0x4598be4a, BRF_GRA },		//  8
	{ "t07_c16.bin",   0x10000, 0xf5425e42, BRF_GRA },		//  9
	{ "p07_c14.bin",   0x10000, 0xcb50a17c, BRF_GRA },		//  10
	{ "v07_c17.bin",   0x10000, 0x959ba3c7, BRF_GRA },		//  11
	{ "s07_c15.bin",   0x10000, 0x7f2e91c5, BRF_GRA },		//  12
	
	{ "d01_c05.bin",   0x10000, 0x81b1ee5c, BRF_GRA },		//  13	Background
	{ "e01_c06.bin",   0x10000, 0xd0d33673, BRF_GRA },		//  14
	{ "f01_c07.bin",   0x10000, 0xaae81695, BRF_GRA },		//  15
	
	{ "d04_c01.bin",   0x10000, 0x9b85101d, BRF_SND },		//  16	Samples
	
	{ "vg_b-8r.ic90",  0x00117, 0xdf368a7a, BRF_GRA },		//  17	PALs
	{ "vg_b-4m.ic38",  0x00117, 0xdbca4204, BRF_GRA },		//  18
	{ "vg_b-1b.ic1",   0x00117, 0x922e5167, BRF_GRA },		//  19
};

STD_ROM_PICK(Drvu)
STD_ROM_FN(Drvu)

static struct BurnRomInfo Drvu2RomDesc[] = {
	{ "vg-a-8h-g.bin", 0x08000, 0x9444c04e, BRF_ESS | BRF_PRG },	//  0	Z80 #1 Program Code
	{ "a-8l",          0x10000, 0x7f95799b, BRF_ESS | BRF_PRG },	//  1
	
	{ "g05_c02.bin",   0x10000, 0x10582b2d, BRF_ESS | BRF_PRG },	//  2	Z80 #2 Program 
	
	{ "f05_c08.bin",   0x10000, 0x01579d20, BRF_GRA },		//  3	Characters
	{ "h05_c09.bin",   0x10000, 0x4f5872f0, BRF_GRA },		//  4
	
	{ "n07_c12.bin",   0x10000, 0x10af8eb2, BRF_GRA },		//  5	Sprites
	{ "k07_c10.bin",   0x10000, 0x9576f304, BRF_GRA },		//  6
	{ "o07_c13.bin",   0x10000, 0xb1d9d4dc, BRF_GRA },		//  7
	{ "l07_c11.bin",   0x10000, 0x4598be4a, BRF_GRA },		//  8
	{ "t07_c16.bin",   0x10000, 0xf5425e42, BRF_GRA },		//  9
	{ "p07_c14.bin",   0x10000, 0xcb50a17c, BRF_GRA },		//  10
	{ "v07_c17.bin",   0x10000, 0x959ba3c7, BRF_GRA },		//  11
	{ "s07_c15.bin",   0x10000, 0x7f2e91c5, BRF_GRA },		//  12
	
	{ "d01_c05.bin",   0x10000, 0x81b1ee5c, BRF_GRA },		//  13	Background
	{ "e01_c06.bin",   0x10000, 0xd0d33673, BRF_GRA },		//  14
	{ "f01_c07.bin",   0x10000, 0xaae81695, BRF_GRA },		//  15
	
	{ "d04_c01.bin",   0x10000, 0x9b85101d, BRF_SND },		//  16	Samples
	
	{ "vg_b-8r.ic90",  0x00117, 0xdf368a7a, BRF_GRA },		//  17	PALs
	{ "vg_b-4m.ic38",  0x00117, 0xdbca4204, BRF_GRA },		//  18
	{ "vg_b-1b.ic1",   0x00117, 0x922e5167, BRF_GRA },		//  19
};

STD_ROM_PICK(Drvu2)
STD_ROM_FN(Drvu2)

static struct BurnRomInfo DrvjRomDesc[] = {
	{ "vg_a-8h.rom",   0x08000, 0xba848713, BRF_ESS | BRF_PRG },	//  0	Z80 #1 Program Code
	{ "vg_a-8l.rom",   0x10000, 0x3b12b1d8, BRF_ESS | BRF_PRG },	//  1
	
	{ "g05_c02.bin",   0x10000, 0x10582b2d, BRF_ESS | BRF_PRG },	//  2	Z80 #2 Program 
	
	{ "f05_c08.bin",   0x10000, 0x01579d20, BRF_GRA },		//  3	Characters
	{ "h05_c09.bin",   0x10000, 0x4f5872f0, BRF_GRA },		//  4
	
	{ "n07_c12.bin",   0x10000, 0x10af8eb2, BRF_GRA },		//  5	Sprites
	{ "k07_c10.bin",   0x10000, 0x9576f304, BRF_GRA },		//  6
	{ "o07_c13.bin",   0x10000, 0xb1d9d4dc, BRF_GRA },		//  7
	{ "l07_c11.bin",   0x10000, 0x4598be4a, BRF_GRA },		//  8
	{ "t07_c16.bin",   0x10000, 0xf5425e42, BRF_GRA },		//  9
	{ "p07_c14.bin",   0x10000, 0xcb50a17c, BRF_GRA },		//  10
	{ "v07_c17.bin",   0x10000, 0x959ba3c7, BRF_GRA },		//  11
	{ "s07_c15.bin",   0x10000, 0x7f2e91c5, BRF_GRA },		//  12
	
	{ "d01_c05.bin",   0x10000, 0x81b1ee5c, BRF_GRA },		//  13	Background
	{ "e01_c06.bin",   0x10000, 0xd0d33673, BRF_GRA },		//  14
	{ "f01_c07.bin",   0x10000, 0xaae81695, BRF_GRA },		//  15
	
	{ "d04_c01.bin",   0x10000, 0x9b85101d, BRF_SND },		//  16	Samples
	
	{ "vg_b-8r.ic90",  0x00117, 0xdf368a7a, BRF_GRA },		//  17	PALs
	{ "vg_b-4m.ic38",  0x00117, 0xdbca4204, BRF_GRA },		//  18
	{ "vg_b-1b.ic1",   0x00117, 0x922e5167, BRF_GRA },		//  19
};

STD_ROM_PICK(Drvj)
STD_ROM_FN(Drvj)

static struct BurnRomInfo BuccanrsRomDesc[] = {
	{ "11.u58",        0x10000, 0xbf1d7e6f, BRF_ESS | BRF_PRG },	//  0	Z80 #1 Program Code
	{ "12.u25",        0x10000, 0x87303ba8, BRF_ESS | BRF_PRG },	//  1
	
	{ "1.u128",        0x10000, 0xeb65f8c3, BRF_ESS | BRF_PRG },	//  2	Z80 #2 Program 
	
	{ "7.u212",        0x10000, 0x95e3c517, BRF_GRA },		//  3	Characters
	{ "8.u189",        0x10000, 0xfe2377ab, BRF_GRA },		//  4
	
	{ "3.u100",        0x20000, 0x16dc435f, BRF_GRA },		//  5	Sprites
	{ "4.u80",         0x10000, 0x4fe3bf97, BRF_GRA },		//  6
	{ "6.u52",         0x20000, 0x078aef7f, BRF_GRA },		//  7
	{ "5.u70",         0x10000, 0xf650fa90, BRF_GRA },		//  8
	
	{ "9.u49",         0x20000, 0x0c6188fb, BRF_GRA },		//  9	Background
	{ "10.u27",        0x20000, 0x2d383ff8, BRF_GRA },		//  10
	
	{ "2.u74",         0x10000, 0x36ee1dac, BRF_SND },		//  11	Samples
	
	{ "prom1.u54",     0x00100, 0xc324835e, BRF_GRA },		//  12	PROMs
	{ "prom4.u79",     0x00100, 0xe6506ef4, BRF_GRA },		//  13
	{ "prom3.u88",     0x00100, 0xb43d094f, BRF_GRA },		//  14
	{ "prom2.u99",     0x00100, 0xe0aa8869, BRF_GRA },		//  15
	
	{ "pal16l8.u103",  0x00104, 0x00000000, BRF_GRA | BRF_NODUMP },	//  16	PALs
	{ "pal16l8.u156",  0x00104, 0x00000000, BRF_GRA | BRF_NODUMP },	//  17
	{ "pal16l8.u42",   0x00104, 0x00000000, BRF_GRA | BRF_NODUMP },	//  18
};

STD_ROM_PICK(Buccanrs)
STD_ROM_FN(Buccanrs)

static struct BurnRomInfo BuccanrsaRomDesc[] = {
	{ "bc-011",        0x08000, 0x6b657ef1, BRF_ESS | BRF_PRG },	//  0	Z80 #1 Program Code
	{ "12.u25",        0x10000, 0x87303ba8, BRF_ESS | BRF_PRG },	//  1
	
	{ "1.u128",        0x10000, 0xeb65f8c3, BRF_ESS | BRF_PRG },	//  2	Z80 #2 Program 
	
	{ "7.u212",        0x10000, 0x95e3c517, BRF_GRA },		//  3	Characters
	{ "8.u189",        0x10000, 0xfe2377ab, BRF_GRA },		//  4
	
	{ "3.u100",        0x20000, 0x16dc435f, BRF_GRA },		//  5	Sprites
	{ "4.u80",         0x10000, 0x4fe3bf97, BRF_GRA },		//  6
	{ "6.u52",         0x20000, 0x078aef7f, BRF_GRA },		//  7
	{ "5.u70",         0x10000, 0xf650fa90, BRF_GRA },		//  8
	
	{ "9.u49",         0x20000, 0x0c6188fb, BRF_GRA },		//  9	Background
	{ "10.u27",        0x20000, 0x2d383ff8, BRF_GRA },		//  10
	
	{ "2.u74",         0x10000, 0x36ee1dac, BRF_SND },		//  11	Samples
	
	{ "prom1.u54",     0x00100, 0xc324835e, BRF_GRA },		//  12	PROMs
	{ "prom4.u79",     0x00100, 0xe6506ef4, BRF_GRA },		//  13
	{ "prom3.u88",     0x00100, 0xb43d094f, BRF_GRA },		//  14
	{ "prom2.u99",     0x00100, 0xe0aa8869, BRF_GRA },		//  15
};

STD_ROM_PICK(Buccanrsa)
STD_ROM_FN(Buccanrsa)

static struct BurnRomInfo KikcubicRomDesc[] = {
	{ "mqj-p0",        0x08000, 0x9cef394a, BRF_ESS | BRF_PRG },	//  0	Z80 #1 Program Code
	{ "mqj-b0",        0x10000, 0xd9bcf4cd, BRF_ESS | BRF_PRG },	//  1
	{ "mqj-b1",        0x10000, 0x54a0abe1, BRF_ESS | BRF_PRG },	//  2
	
	{ "mqj-sp",        0x10000, 0xbbcf3582, BRF_ESS | BRF_PRG },	//  3	Z80 #2 Program 
	
	{ "mqj-c0",        0x10000, 0x975585c5, BRF_GRA },		//  4	Characters
	{ "mqj-c1",        0x10000, 0x49d9936d, BRF_GRA },		//  5
	
	{ "mqj-00",        0x40000, 0x7fb0c58f, BRF_GRA },		//  6	Sprites
	{ "mqj-10",        0x40000, 0x3a189205, BRF_GRA },		//  7
	
	{ "mqj-v0",        0x10000, 0x54762956, BRF_SND },		//  8	Samples
	
	{ "8d",            0x00100, 0x7379bb12, BRF_GRA },		//  9	PROMs
	{ "6h",            0x00020, 0xface0cbb, BRF_GRA },		//  10
	{ "7s",            0x00020, 0xface0cbb, BRF_GRA },		//  11
};

STD_ROM_PICK(Kikcubic)
STD_ROM_FN(Kikcubic)

static struct BurnRomInfo KikcubicbRomDesc[] = {
	{ "1.bin",         0x08000, 0xd3a589ba, BRF_ESS | BRF_PRG },	//  0	Z80 #1 Program Code
	{ "4.bin",         0x10000, 0x9ae1e1a6, BRF_ESS | BRF_PRG },	//  1
	{ "5.bin",         0x08000, 0xa5a6bffd, BRF_ESS | BRF_PRG },	//  2
	
	{ "mqj-sp",        0x10000, 0xbbcf3582, BRF_ESS | BRF_PRG },	//  3	Z80 #2 Program 
	
	{ "7.bin",         0x10000, 0x1788d51a, BRF_GRA },		//  4	Characters
	{ "mqj-c1",        0x10000, 0x49d9936d, BRF_GRA },		//  5
	
	{ "11.bin",        0x10000, 0x0f0cac92, BRF_GRA },		//  6	Sprites
	{ "10.bin",        0x10000, 0x7d3822a8, BRF_GRA },		//  7
	{ "9.bin",         0x10000, 0x56fb4fa3, BRF_GRA },		//  8
	{ "8.bin",         0x10000, 0x947dbd4e, BRF_GRA },		//  9
	
	{ "mqj-v0",        0x10000, 0x54762956, BRF_SND },		//  10	Samples
	
	{ "8d",            0x00100, 0x7379bb12, BRF_GRA },		//  11	PROMs
	{ "6h",            0x00020, 0xface0cbb, BRF_GRA },		//  12
	{ "7s",            0x00020, 0xface0cbb, BRF_GRA },		//  13
};

STD_ROM_PICK(Kikcubicb)
STD_ROM_FN(Kikcubicb)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom1             = Next; Next += 0x28000;
	DrvZ80Rom2             = Next; Next += 0x10000;
	DrvSamples             = Next; Next += 0x10000;

	RamStart               = Next;

	DrvZ80Ram1             = Next; Next += 0x02000;
	DrvZ80Ram2             = Next; Next += 0x01000;
	DrvSpriteRam           = Next; Next += 0x00100;
	DrvPaletteRam          = Next; Next += 0x00800;
	DrvVideoRam            = Next; Next += 0x01000;
	DrvPalette             = (UINT32*)Next; Next += (512 + 32) * sizeof(UINT32);

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x1000 * 8 * 8;
	DrvSprites             = Next; Next += 0x1000 * 16 * 16;
	DrvBackTiles           = Next; Next += 0x4000 * 32;
	
	MemEnd                 = Next;

	return 0;
}

static void DrvSetVector(INT32 Status)
{
	switch (Status) {
		case VECTOR_INIT: {
			DrvIrqVector = 0xff;
			break;
		}
		
		case YM2151_ASSERT: {
			DrvIrqVector &= 0xef;
			break;
		}
		
		case YM2151_CLEAR: {
			DrvIrqVector |= 0x10;
			break;
		}
		
		case Z80_ASSERT: {
			DrvIrqVector &= 0xdf;
			break;
		}
		
		case Z80_CLEAR: {
			DrvIrqVector |= 0x20;
			break;
		}
	}
	
	if (DrvIrqVector == 0xff) {
//		ZetSetVector(DrvIrqVector);
		ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
	} else {
		ZetSetVector(DrvIrqVector);
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		nCyclesDone[1] += ZetRun(1000);
	}
}

static INT32 DrvDoReset()
{
	for (INT32 i = 0; i < 2; i++) {
		ZetOpen(i);
		ZetReset();
		if (i == 1) DrvSetVector(VECTOR_INIT);
		ZetClose();
	}
	
	if (DrvHasYM2203) {
		BurnYM2203Reset();
	} else {
		BurnYM2151Reset();
	}	
	
	DACReset();
	
	DrvRomBank = 0;
	DrvSoundLatch = 0;
	DrvRearColour = 0;
	DrvRearDisable = 0;
	DrvHorizScrollLo = 0;
	DrvHorizScrollHi = 0;
	DrvRearHorizScrollLo = 0;
	DrvRearHorizScrollHi = 0;
	DrvSampleAddress = 0;

	return 0;
}

UINT8 __fastcall VigilanteZ80Read1(UINT16 a)
{
	switch (a) {
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall VigilanteZ80Write1(UINT16 a, UINT8 d)
{
	if (a >= 0xc020 && a <= 0xc0df) {
		DrvSpriteRam[a - 0xc020] = d;
		return;
	}
	
	if (a >= 0xc800 && a <= 0xcfff) {
		INT32 Offset = a & 0x7ff;
		INT32 Bank = Offset & 0x400;
		INT32 r, g, b;
		
		DrvPaletteRam[Offset] = d;
		
		Offset &= 0xff;
		r = (DrvPaletteRam[Bank + Offset + 0x000] << 3) & 0xff;
		g = (DrvPaletteRam[Bank + Offset + 0x100] << 3) & 0xff;
		b = (DrvPaletteRam[Bank + Offset + 0x200] << 3) & 0xff;
		
		DrvPalette[(Bank >> 2) + Offset] = BurnHighCol(r, g, b, 0);		
		return;
	}
	
	switch (a) {
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall VigilanteZ80PortRead1(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			return 0xff - DrvInput[0];
		}
		
		case 0x01: {
			return 0xff - DrvInput[1];
		}
		
		case 0x02: {
			return 0xff - DrvInput[2];
		}
		
		case 0x03: {
			return DrvDip[0];
		}
		
		case 0x04: {
			return DrvDip[1];
		}
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

UINT8 __fastcall BuccanrsZ80PortRead1(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			return (0xd0 - DrvInput[0]) | DrvDip[0];
		}
		
		case 0x01: {
			return 0xff - DrvInput[1];
		}
		
		case 0x02: {
			return 0xff - DrvInput[2];
		}
		
		case 0x03: {
			return DrvDip[1];
		}
		
		case 0x04: {
			return DrvDip[2];
		}
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

UINT8 __fastcall BuccanrsaZ80PortRead1(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			return (0x0b - DrvInput[0]) | DrvDip[0];
		}
		
		case 0x01: {
			return 0xff - DrvInput[1];
		}
		
		case 0x02: {
			return 0xff - DrvInput[2];
		}
		
		case 0x03: {
			return DrvDip[1];
		}
		
		case 0x04: {
			return DrvDip[2];
		}
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall VigilanteZ80PortWrite1(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			DrvSoundLatch = d;
			ZetClose();
			ZetOpen(1);
			DrvSetVector(Z80_ASSERT);
			ZetClose();
			ZetOpen(0);
			return;
		}
		
		case 0x01: {
			// output port?
			return;
		}
		
		case 0x04: {
			DrvRomBank = d & 0x07;
			ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
			return;
		}
		
		case 0x80: {
			DrvHorizScrollLo = d;
			return;
		}
		
		case 0x81: {
			DrvHorizScrollHi = (d & 0x01) << 8;
			return;
		}
		
		case 0x82: {
			DrvRearHorizScrollLo = d;
			return;
		}
		
		case 0x83: {
			DrvRearHorizScrollHi = (d & 0x07) << 8;
			return;
		}
		
		case 0x84: {
			DrvRearColour = d & 0x0d;
			DrvRearDisable = d & 0x40;
			return;
		}
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall KikcubicZ80Read1(UINT16 a)
{
	switch (a) {
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall KikcubicZ80Write1(UINT16 a, UINT8 d)
{
	if (a >= 0xc800 && a <= 0xcaff) {
		INT32 Offset = a & 0x3ff;
		INT32 Bank = Offset & 0x400;
		INT32 r, g, b;
		
		DrvPaletteRam[Offset] = d;
		
		Offset &= 0xff;
		r = (DrvPaletteRam[Bank + Offset + 0x000] << 3) & 0xff;
		g = (DrvPaletteRam[Bank + Offset + 0x100] << 3) & 0xff;
		b = (DrvPaletteRam[Bank + Offset + 0x200] << 3) & 0xff;
		
		DrvPalette[(Bank >> 2) + Offset] = BurnHighCol(r, g, b, 0);
		return;
	}
	
	switch (a) {
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall KikcubicZ80PortRead1(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			return DrvDip[0];
		}
		
		case 0x01: {
			return DrvDip[1];
		}
		
		case 0x02: {
			return 0xff - DrvInput[1];
		}
		
		case 0x03: {
			return 0xff - DrvInput[0];
		}
		
		case 0x04: {
			return 0xff - DrvInput[2];
		}
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall KikcubicZ80PortWrite1(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			// coin write
			return;
		}
		
		case 0x04: {
			DrvRomBank = d & 0x07;
			ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
			return;
		}
		
		case 0x06: {
			if (d == 0x20) return; // ???
			DrvSoundLatch = d;
			ZetClose();
			ZetOpen(1);
			DrvSetVector(Z80_ASSERT);
			ZetClose();
			ZetOpen(0);
			return;
		}
		
		case 0x07: {
			// nop???
			return;
		}
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall VigilanteZ80Read2(UINT16 a)
{
	switch (a) {
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall VigilanteZ80Write2(UINT16 a, UINT8 /*d*/)
{
	switch (a) {
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #2 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall VigilanteZ80PortRead2(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x01: {
			return BurnYM2151ReadStatus();
		}
		
		case 0x80: {
			return DrvSoundLatch;
		}
		
		case 0x84: {
			return DrvSamples[DrvSampleAddress];
		}
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall VigilanteZ80PortWrite2(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0x01: {			
			BurnYM2151WriteRegister(d);
			return;
		}
		
		case 0x80: {
			DrvSampleAddress = (DrvSampleAddress & 0xff00) | ((d << 0) & 0x00ff);
			return;
		}
		
		case 0x81: {
			DrvSampleAddress = (DrvSampleAddress & 0x00ff) | ((d << 8) & 0xff00);
			return;
		}
		
		case 0x82: {
			DACSignedWrite(0, d);
			DrvSampleAddress = (DrvSampleAddress + 1) & 0xffff;
			return;
		}
		
		case 0x83: {
			DrvSetVector(Z80_CLEAR);
			return;
		}
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall BuccanrsZ80PortRead2(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			return YM2203Read(0, 0);
		}
		
		case 0x02: {
			return YM2203Read(1, 0);
		}
		
		case 0x80: {
			return DrvSoundLatch;
		}
		
		case 0x84: {
			return DrvSamples[DrvSampleAddress];
		}
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall BuccanrsZ80PortWrite2(UINT16 a, UINT8 d)
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
			BurnYM2203Write(1, 0, d);
			return;
		}
		
		case 0x03: {
			BurnYM2203Write(1, 1, d);
			return;
		}
		
		case 0x80: {
			DrvSampleAddress = (DrvSampleAddress & 0xff00) | ((d << 0) & 0x00ff);
			return;
		}
		
		case 0x81: {
			DrvSampleAddress = (DrvSampleAddress & 0x00ff) | ((d << 8) & 0xff00);
			return;
		}
		
		case 0x82: {
			DACSignedWrite(0, d);
			DrvSampleAddress = (DrvSampleAddress + 1) & 0xffff;
			return;
		}
		
		case 0x83: {
			DrvSetVector(Z80_CLEAR);
			return;
		}
		
		default: {
//			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

static void VigilantYM2151IrqHandler(INT32 Irq)
{
	if (Irq) {
		DrvSetVector(YM2151_ASSERT);
	} else {
		DrvSetVector(YM2151_CLEAR);
	}
}

inline static void BuccanrsYM2203IRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		DrvSetVector(YM2151_ASSERT);
	} else {
		DrvSetVector(YM2151_CLEAR);
	}
}

inline static INT32 BuccanrsSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / (18432000 / 6);
}

inline static double BuccanrsGetTime()
{
	return (double)ZetTotalCycles() / (18432000 / 6);
}

static INT32 VigilantSyncDAC()
{
	return (INT32)(float)(nBurnSoundLen * (ZetTotalCycles() / ((nCyclesTotal[1] * 55.0000) / (nBurnFPS / 100.0000))));
}

static INT32 CharPlaneOffsets[4]         = { 0x80000, 0x80004, 0, 4 };
static INT32 CharXOffsets[8]             = { 0, 1, 2, 3, 64, 65, 66, 67 };
static INT32 CharYOffsets[8]             = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 SpritePlaneOffsets[4]       = { 0x200000, 0x200004, 0, 4 };
static INT32 SpriteXOffsets[16]          = { 0, 1, 2, 3, 128, 129, 130, 131, 256, 257, 258, 259, 384, 385, 386, 387 };
static INT32 SpriteYOffsets[16]          = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };
static INT32 BackTilePlaneOffsets[4]     = { 0, 2, 4, 6 };
static INT32 BackTileXOffsets[32]        = { 1, 0, 9, 8, 17, 16, 25, 24, 33, 32, 41, 40, 49, 48, 57, 56, 65, 64, 73, 72, 81, 80, 89, 88, 97, 96, 105, 104, 113, 112, 121, 120 };
static INT32 BackTileYOffsets[1]         = { 0 };
static INT32 BuccSpriteXOffsets[16]      = { 3, 2, 1, 0, 131, 130, 129, 128, 259, 258, 257, 256, 387, 386, 385, 384 };
static INT32 BuccBackTilePlaneOffsets[4] = { 6, 4, 2, 0 };

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

	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000,  1, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000,  2, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  4, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x50000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x70000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load and decode the bg tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 15, 1); if (nRet != 0) return 1;
	GfxDecode(0x4000, 4, 32, 1, BackTilePlaneOffsets, BackTileXOffsets, BackTileYOffsets, 0x80, DrvTempRom, DrvBackTiles);
	
	BurnFree(DrvTempRom);
	
	// Load sample Roms
	nRet = BurnLoadRom(DrvSamples + 0x00000, 16, 1); if (nRet != 0) return 1;
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(VigilanteZ80Read1);
	ZetSetWriteHandler(VigilanteZ80Write1);
	ZetSetInHandler(VigilanteZ80PortRead1);
	ZetSetOutHandler(VigilanteZ80PortWrite1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom1             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom1             );
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0xc800, 0xcfff, 0, DrvPaletteRam          );
	ZetMapArea(0xc800, 0xcfff, 2, DrvPaletteRam          );
	ZetMapArea(0xd000, 0xdfff, 0, DrvVideoRam            );
	ZetMapArea(0xd000, 0xdfff, 1, DrvVideoRam            );
	ZetMapArea(0xd000, 0xdfff, 2, DrvVideoRam            );
	ZetMapArea(0xe000, 0xefff, 0, DrvZ80Ram1             );
	ZetMapArea(0xe000, 0xefff, 1, DrvZ80Ram1             );
	ZetMapArea(0xe000, 0xefff, 2, DrvZ80Ram1             );
	ZetMemEnd();
	ZetClose();
	
	ZetInit(1);
	ZetOpen(1);
	ZetSetReadHandler(VigilanteZ80Read2);
	ZetSetWriteHandler(VigilanteZ80Write2);
	ZetSetInHandler(VigilanteZ80PortRead2);
	ZetSetOutHandler(VigilanteZ80PortWrite2);	
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80Rom2             );
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80Rom2             );
	ZetMapArea(0xf000, 0xffff, 0, DrvZ80Ram2             );
	ZetMapArea(0xf000, 0xffff, 1, DrvZ80Ram2             );
	ZetMapArea(0xf000, 0xffff, 2, DrvZ80Ram2             );
	ZetMemEnd();
	ZetClose();
	
	BurnSetRefreshRate(55.0);
	nCyclesTotal[0] = 3579645 / 55;
	nCyclesTotal[1] = 3579645 / 55;
	
	GenericTilesInit();
	BurnYM2151Init(3579645);
	BurnYM2151SetIrqHandler(&VigilantYM2151IrqHandler);	
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.55, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.55, BURN_SND_ROUTE_RIGHT);
	DACInit(0, 0, 1, VigilantSyncDAC);
	DACSetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	
	DrvDoReset();

	return 0;
}

static INT32 BuccanrsInit()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000,  1, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000,  2, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  4, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5, 1); if (nRet != 0) return 1;
	memcpy(DrvTempRom + 0x20000, DrvTempRom + 0x10000, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000,  7, 1); if (nRet != 0) return 1;
	memcpy(DrvTempRom + 0x60000, DrvTempRom + 0x50000, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x50000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, BuccSpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load and decode the bg tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x4000, 4, 32, 1, BuccBackTilePlaneOffsets, BackTileXOffsets, BackTileYOffsets, 0x80, DrvTempRom, DrvBackTiles);
	
	BurnFree(DrvTempRom);
	
	// Load sample Roms
	nRet = BurnLoadRom(DrvSamples + 0x00000, 11, 1); if (nRet != 0) return 1;
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(VigilanteZ80Read1);
	ZetSetWriteHandler(VigilanteZ80Write1);
	ZetSetInHandler(BuccanrsZ80PortRead1);
	ZetSetOutHandler(VigilanteZ80PortWrite1);
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "buccanrsa")) ZetSetInHandler(BuccanrsaZ80PortRead1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom1             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom1             );
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0xc800, 0xcfff, 0, DrvPaletteRam          );
	ZetMapArea(0xc800, 0xcfff, 2, DrvPaletteRam          );
	ZetMapArea(0xd000, 0xdfff, 0, DrvVideoRam            );
	ZetMapArea(0xd000, 0xdfff, 1, DrvVideoRam            );
	ZetMapArea(0xd000, 0xdfff, 2, DrvVideoRam            );
	ZetMapArea(0xe000, 0xefff, 0, DrvZ80Ram1             );
	ZetMapArea(0xe000, 0xefff, 1, DrvZ80Ram1             );
	ZetMapArea(0xe000, 0xefff, 2, DrvZ80Ram1             );
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetSetReadHandler(VigilanteZ80Read2);
	ZetSetWriteHandler(VigilanteZ80Write2);
	ZetSetInHandler(BuccanrsZ80PortRead2);
	ZetSetOutHandler(BuccanrsZ80PortWrite2);	
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80Rom2             );
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80Rom2             );
	ZetMapArea(0xf000, 0xffff, 0, DrvZ80Ram2             );
	ZetMapArea(0xf000, 0xffff, 1, DrvZ80Ram2             );
	ZetMapArea(0xf000, 0xffff, 2, DrvZ80Ram2             );
	ZetMemEnd();
	ZetClose();
	
	BurnSetRefreshRate(55.0);
	nCyclesTotal[0] = 5688800 / 55;
	nCyclesTotal[1] = (18432000 / 6) / 55;
	
	GenericTilesInit();
	
	DrvHasYM2203 = 1;
	BurnYM2203Init(2, 18432000 / 6, &BuccanrsYM2203IRQHandler, BuccanrsSynchroniseStream, BuccanrsGetTime, 0);
	BurnTimerAttachZet(18432000 / 6);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.50, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.35, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.35, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.35, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_YM2203_ROUTE, 0.50, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_1, 0.35, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_2, 0.35, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_3, 0.35, BURN_SND_ROUTE_BOTH);
	
	DACInit(0, 0, 1, VigilantSyncDAC);
	DACSetRoute(0, 0.35, BURN_SND_ROUTE_BOTH);
	
	DrvDoReset();

	return 0;
}

static INT32 KikcubicInit()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "kikcubicb")) {
		// Load Z80 #1 Program Roms
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000,  1, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x20000,  2, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x28000,  2, 1); if (nRet != 0) return 1;
	
		// Load Z80 #2 Program Roms
		nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000,  3, 1); if (nRet != 0) return 1;
	
		// Load and decode the chars
		nRet = BurnLoadRom(DrvTempRom + 0x00000,  4, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x10000,  5, 1); if (nRet != 0) return 1;
		GfxDecode(0x1000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
		// Load and decode the sprites
		memset(DrvTempRom, 0, 0x80000);
		nRet = BurnLoadRom(DrvTempRom + 0x00000,  6, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x20000,  6, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x10000,  7, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x30000,  7, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x40000,  8, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x60000,  8, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x50000,  9, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x70000,  9, 1); if (nRet != 0) return 1;
		GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
		
		// Load sample Roms
		nRet = BurnLoadRom(DrvSamples + 0x00000, 10, 1); if (nRet != 0) return 1;
	} else {
		// Load Z80 #1 Program Roms
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000,  1, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x20000,  2, 1); if (nRet != 0) return 1;
	
		// Load Z80 #2 Program Roms
		nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000,  3, 1); if (nRet != 0) return 1;
	
		// Load and decode the chars
		nRet = BurnLoadRom(DrvTempRom + 0x00000,  4, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x10000,  5, 1); if (nRet != 0) return 1;
		GfxDecode(0x1000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
		// Load and decode the sprites
		memset(DrvTempRom, 0, 0x80000);
		nRet = BurnLoadRom(DrvTempRom + 0x00000,  6, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x40000,  7, 1); if (nRet != 0) return 1;
		GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
		
		// Load sample Roms
		nRet = BurnLoadRom(DrvSamples + 0x00000, 8, 1); if (nRet != 0) return 1;
	}
	
	BurnFree(DrvTempRom);
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(KikcubicZ80Read1);
	ZetSetWriteHandler(KikcubicZ80Write1);
	ZetSetInHandler(KikcubicZ80PortRead1);
	ZetSetOutHandler(KikcubicZ80PortWrite1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom1             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom1             );
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0xc000, 0xc0ff, 0, DrvSpriteRam           );
	ZetMapArea(0xc000, 0xc0ff, 1, DrvSpriteRam           );
	ZetMapArea(0xc000, 0xc0ff, 2, DrvSpriteRam           );
	ZetMapArea(0xc800, 0xcaff, 0, DrvPaletteRam          );
	ZetMapArea(0xc800, 0xcaff, 2, DrvPaletteRam          );	
	ZetMapArea(0xd000, 0xdfff, 0, DrvVideoRam            );
	ZetMapArea(0xd000, 0xdfff, 1, DrvVideoRam            );
	ZetMapArea(0xd000, 0xdfff, 2, DrvVideoRam            );
	ZetMapArea(0xe000, 0xffff, 0, DrvZ80Ram1             );
	ZetMapArea(0xe000, 0xffff, 1, DrvZ80Ram1             );
	ZetMapArea(0xe000, 0xffff, 2, DrvZ80Ram1             );
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetSetReadHandler(VigilanteZ80Read2);
	ZetSetWriteHandler(VigilanteZ80Write2);
	ZetSetInHandler(VigilanteZ80PortRead2);
	ZetSetOutHandler(VigilanteZ80PortWrite2);	
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80Rom2             );
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80Rom2             );
	ZetMapArea(0xf000, 0xffff, 0, DrvZ80Ram2             );
	ZetMapArea(0xf000, 0xffff, 1, DrvZ80Ram2             );
	ZetMapArea(0xf000, 0xffff, 2, DrvZ80Ram2             );
	ZetMemEnd();
	ZetClose();
	
	BurnSetRefreshRate(55.0);
	nCyclesTotal[0] = 3579645 / 55;
	nCyclesTotal[1] = 3579645 / 55;
	
	GenericTilesInit();
	BurnYM2151Init(3579645);
	BurnYM2151SetIrqHandler(&VigilantYM2151IrqHandler);	
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.55, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.55, BURN_SND_ROUTE_RIGHT);
	DACInit(0, 0, 1, VigilantSyncDAC);
	DACSetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	
	DrvKikcubicDraw = 1;
	
	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	ZetExit();
	if (DrvHasYM2203) {
		BurnYM2203Exit();
	} else {
		BurnYM2151Exit();
	}
	DACExit();
	
	GenericTilesExit();
	
	BurnFree(Mem);
	
	DrvRomBank = 0;
	DrvSoundLatch = 0;
	DrvIrqVector = 0;
	DrvRearColour = 0;
	DrvRearDisable = 0;
	DrvHorizScrollLo = 0;
	DrvHorizScrollHi = 0;
	DrvRearHorizScrollLo = 0;
	DrvRearHorizScrollHi = 0;
	DrvSampleAddress = 0;
	
	DrvHasYM2203 = 0;
	DrvKikcubicDraw = 0;

	return 0;
}

static void DrvRenderBackground()
{
	INT32 CharCode = 0;
	INT32 Scroll = 0x17a - (DrvRearHorizScrollLo + DrvRearHorizScrollHi);
	if (Scroll > 0) Scroll -= 2048;
	
	for (INT32 i = 0; i < 16; i++) {
		INT32 r, g, b;

		r = (DrvPaletteRam[0x400 + 16 * DrvRearColour + i] << 3) & 0xff;
		g = (DrvPaletteRam[0x500 + 16 * DrvRearColour + i] << 3) & 0xff;
		b = (DrvPaletteRam[0x600 + 16 * DrvRearColour + i] << 3) & 0xff;

		DrvPalette[512 + i] = BurnHighCol(r, g, b, 0);

		r = (DrvPaletteRam[0x400 + 16 * DrvRearColour + 32 + i] << 3) & 0xff;
		g = (DrvPaletteRam[0x500 + 16 * DrvRearColour + 32 + i] << 3) & 0xff;
		b = (DrvPaletteRam[0x600 + 16 * DrvRearColour + 32 + i] << 3) & 0xff;

		DrvPalette[512 + 16 + i] = BurnHighCol(r, g, b, 0);
	}
	
	for (INT32 Page = 0; Page < 4; Page++) {
		for (INT32 Row = 0; Row < 256; Row++) {
			for (INT32 Col = 0; Col < 512; Col += 32) {
				INT32 Colour = (Row < 128) ? 0 : 1;
				UINT32 nPalette = 512 | (Colour << 4);
			
				for (INT32 px = 0; px < 32; px++) {
					UINT8 c = DrvBackTiles[(CharCode * 32) + px];
					INT32 xPos = (512 * Page) + Col + px + Scroll;
						
					if (Row >= 0 && Row < nScreenHeight) {					
						UINT16* pPixel = pTransDraw + (Row * nScreenWidth);
						
						if (xPos >= 0 && xPos < nScreenWidth) {
							pPixel[xPos] = c | nPalette;
						}
					}
				}				
				
				CharCode++;
			}
		}
	}
}

static void DrvDrawForeground(INT32 Priority, INT32 Opaque)
{
	INT32 Scroll = -(DrvHorizScrollLo + DrvHorizScrollHi);

	for (INT32 Offset = 0; Offset < 0x1000; Offset += 2) {
		INT32 sy = 8 * ((Offset / 2) / 64);
		INT32 sx = 8 * ((Offset / 2) % 64);
		INT32 Attr = DrvVideoRam[Offset + 1];
		INT32 Colour = Attr & 0x0f;
		INT32 Tile = DrvVideoRam[Offset + 0] | ((Attr & 0xf0) << 4);

		if (Priority) {
			// Sprite masking
			if ((Colour & 0x0c) == 0x0c) {
				if (sy >= 48) {
					sx = (sx + Scroll) & 0x1ff;
					
					sx -= 128;
					
					INT32 px, py;
					UINT32 nPalette = 256 | (Colour << 4);
			
					for (py = 0; py < 8; py++) {
						for (px = 0; px < 8; px++) {
							UINT8 c = DrvChars[(Tile * 64) + (py * 8) + px];
					
							if (((0xff >> c) & 0x01) == 0) {
								INT32 xPos = sx + px;
								INT32 yPos = sy + py;
						
								if (yPos >= 0 && yPos < nScreenHeight) {					
									UINT16* pPixel = pTransDraw + (yPos * nScreenWidth);
						
									if (xPos >= 0 && xPos < nScreenWidth) {
										pPixel[xPos] = c | nPalette;
									}
								}
							}
						}
					}
				}
			}
		} else {
			if (sy >= 48) sx = (sx + Scroll) & 0x1ff;
			
			sx -= 128;
			
			if (sx >= 0 && sx < (nScreenWidth - 8) && sy >= 0 && sy < (nScreenHeight - 8)) {
				if (Opaque || Colour >= 4) {
					Render8x8Tile(pTransDraw, Tile, sx, sy, Colour, 4, 256, DrvChars);
				} else {
					Render8x8Tile_Mask(pTransDraw, Tile, sx, sy, Colour, 4, 0, 256, DrvChars);
				}
			} else {
				if (Opaque || Colour >= 4) {
					Render8x8Tile_Clip(pTransDraw, Tile, sx, sy, Colour, 4, 256, DrvChars);
				} else {
					Render8x8Tile_Mask_Clip(pTransDraw, Tile, sx, sy, Colour, 4, 0, 256, DrvChars);
				}
			}
		}
	}
}

static void DrvDrawSprites()
{
	INT32 DrvSpriteRamSize = 0xc0;
	if (DrvKikcubicDraw) DrvSpriteRamSize = 0x100;
	
	for (INT32 Offset = 0; Offset < DrvSpriteRamSize; Offset += 8) {
		INT32 Code, Colour, sx, sy, xFlip, yFlip, h;

		Code = DrvSpriteRam[Offset + 4] | ((DrvSpriteRam[Offset + 5] & 0x0f) << 8);
		Colour = DrvSpriteRam[Offset + 0] & 0x0f;
		sx = (DrvSpriteRam[Offset + 6] | ((DrvSpriteRam[Offset + 7] & 0x01) << 8));
		sy = 256 + 128 - (DrvSpriteRam[Offset + 2] | ((DrvSpriteRam[Offset + 3] & 0x01) << 8));
		xFlip = DrvSpriteRam[Offset + 5] & 0x40;
		yFlip = DrvSpriteRam[Offset + 5] & 0x80;
		h = 1 << ((DrvSpriteRam[Offset + 5] & 0x30) >> 4);
		sy -= 16 * h;

		Code &= ~(h - 1);
		
		if (DrvKikcubicDraw) {
			sx -= 64;
		} else {
			sx -= 128;
		}

		for (INT32 y = 0; y < h; y++) {
			INT32 c = Code;

			if (yFlip) {
				c += h - 1 - y;
			} else {
				c += y;
			}

			if (sx > 16 && sx < (nScreenWidth - 16) && (sy + (16 * y)) > 16 && (sy + (16 * y)) < (nScreenHeight - 16)) {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_Mask_FlipXY(pTransDraw, c, sx, sy + (16 * y), Colour, 4, 0, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask_FlipX(pTransDraw, c, sx, sy + (16 * y), Colour, 4, 0, 0, DrvSprites);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_Mask_FlipY(pTransDraw, c, sx, sy + (16 * y), Colour, 4, 0, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask(pTransDraw, c, sx, sy + (16 * y), Colour, 4, 0, 0, DrvSprites);
					}
				}
			} else {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, c, sx, sy + (16 * y), Colour, 4, 0, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, c, sx, sy + (16 * y), Colour, 4, 0, 0, DrvSprites);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, c, sx, sy + (16 * y), Colour, 4, 0, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, c, sx, sy + (16 * y), Colour, 4, 0, 0, DrvSprites);
					}
				}
			}
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();	
		
	if (DrvRearDisable) {
		DrvDrawForeground(0, 1);
		DrvDrawSprites();
		DrvDrawForeground(1, 0);
	} else {
		DrvRenderBackground();
		DrvDrawForeground(0, 0);
		DrvDrawSprites();
		DrvDrawForeground(1, 0);
	}
	
	BurnTransferCopy(DrvPalette);
}

static void KikcubicDraw()
{
	BurnTransferClear();
	
	for (INT32 Offset = 0; Offset < 0x1000; Offset += 2) {
		INT32 sy = 8 * ((Offset / 2) / 64);
		INT32 sx = 8 * ((Offset / 2) % 64);
		INT32 Attr = DrvVideoRam[Offset + 1];
		INT32 Colour = (Attr & 0xf0) >> 4;
		INT32 Code = DrvVideoRam[Offset] | ((Attr & 0x0f) << 8);
		
		sx -= 64;
		if (sx >= 0 && sx < (nScreenWidth - 8) && sy >= 0 && sy < (nScreenHeight - 8)) {
			Render8x8Tile(pTransDraw, Code, sx, sy, Colour, 4, 0, DrvChars);
		} else {
			Render8x8Tile_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, DrvChars);
		}
	}	
		
	DrvDrawSprites();	
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 256; // dac needs 128 NMIs
	INT32 nSoundBufferPos = 0;
	
	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	ZetNewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == (nInterleave - 1)) ZetRaiseIrq(0);
		ZetClose();

		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		if (!DrvHasYM2203) {
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		} else {
			BurnTimerUpdate(i * (nCyclesTotal[nCurrentCPU] / nInterleave));
		}
		if (i & 1) {
			ZetNmi();
		}
		ZetClose();
		
		if (pBurnSoundOut && !DrvHasYM2203) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			ZetOpen(1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	if (DrvHasYM2203) {
		ZetOpen(1);
		BurnTimerEndFrame(nCyclesTotal[1]);
		ZetClose();
	}
	
	if (pBurnSoundOut && !DrvHasYM2203) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		
		if (nSegmentLength) {
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			ZetOpen(1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();			
		}
		DACUpdate(pBurnSoundOut, nBurnSoundLen);
	}
	
	if (DrvHasYM2203 && pBurnSoundOut) {
		ZetOpen(1);
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		ZetClose();
		DACUpdate(pBurnSoundOut, nBurnSoundLen);
	}
	
	if (pBurnDraw) {
		if (DrvKikcubicDraw) {
			KikcubicDraw();
		} else {
			DrvDraw();	
		}
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029705;
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
		if (DrvHasYM2203) {
			BurnYM2203Scan(nAction, pnMin);
		} else {
			BurnYM2151Scan(nAction);
		}
		DACScan(nAction, pnMin);

		// Scan critical driver variables
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
		SCAN_VAR(DrvRomBank);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(DrvDip);
		SCAN_VAR(DrvInput);
		SCAN_VAR(DrvIrqVector);
		SCAN_VAR(DrvRearColour);
		SCAN_VAR(DrvRearDisable);
		SCAN_VAR(DrvHorizScrollLo);
		SCAN_VAR(DrvHorizScrollHi);
		SCAN_VAR(DrvRearHorizScrollLo);
		SCAN_VAR(DrvRearHorizScrollHi);
		SCAN_VAR(DrvSampleAddress);
	}
	
	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
		ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
		ZetClose();
	}

	return 0;
}

#undef VECTOR_INIT
#undef YM2151_ASSERT
#undef YM2151_CLEAR
#undef Z80_ASSERT
#undef Z80_CLEAR

struct BurnDriver BurnDrvVigilant = {
	"vigilant", NULL, NULL, NULL, "1988",
	"Vigilante (World, set 1)\0", NULL, "Irem", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_MISC, GBF_SCRFIGHT, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 544, 256, 256, 4, 3
};

struct BurnDriver BurnDrvVigilant1 = {
	"vigilant1", "vigilant", NULL, NULL, "1988",
	"Vigilante (World, set 2)\0", NULL, "Irem", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_MISC, GBF_SCRFIGHT, 0,
	NULL, Drv1RomInfo, Drv1RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 544, 256, 256, 4, 3
};

struct BurnDriver BurnDrvVigilantu = {
	"vigilantu", "vigilant", NULL, NULL, "1988",
	"Vigilante (US)\0", NULL, "Irem (Data East USA License)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_MISC, GBF_SCRFIGHT, 0,
	NULL, DrvuRomInfo, DrvuRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 544, 256, 256, 4, 3
};

struct BurnDriver BurnDrvVigilantu2 = {
	"vigilantu2", "vigilant", NULL, NULL, "1988",
	"Vigilante (US, rev G)\0", NULL, "Irem (Data East USA License)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_MISC, GBF_SCRFIGHT, 0,
	NULL, Drvu2RomInfo, Drvu2RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 544, 256, 256, 4, 3
};

struct BurnDriver BurnDrvVigilantj = {
	"vigilantj", "vigilant", NULL, NULL, "1988",
	"Vigilante (Japan)\0", NULL, "Irem", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_MISC, GBF_SCRFIGHT, 0,
	NULL, DrvjRomInfo, DrvjRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 544, 256, 256, 4, 3
};

struct BurnDriver BurnDrvKikcubic = {
	"kikcubic", NULL, NULL, NULL, "1988",
	"Meikyu Jima (Japan)\0", NULL, "Irem", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_MISC, GBF_MAZE, 0,
	NULL, KikcubicRomInfo, KikcubicRomName, NULL, NULL, KikcubicInputInfo, KikcubicDIPInfo,
	KikcubicInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 544, 384, 256, 4, 3
};

struct BurnDriver BurnDrvKikcubicb = {
	"kikcubicb", "kikcubic", NULL, NULL, "1988",
	"Kickle Cubele\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_IREM_MISC, GBF_MAZE, 0,
	NULL, KikcubicbRomInfo, KikcubicbRomName, NULL, NULL, KikcubicInputInfo, KikcubicDIPInfo,
	KikcubicInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 544, 384, 256, 4, 3
};

struct BurnDriver BurnDrvBuccanrs = {
	"buccanrs", NULL, NULL, NULL, "1989",
	"Buccaneers (set 1)\0", NULL, "Duintronic", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_MISC, GBF_SCRFIGHT, 0,
	NULL, BuccanrsRomInfo, BuccanrsRomName, NULL, NULL, BuccanrsInputInfo, BuccanrsDIPInfo,
	BuccanrsInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 544, 256, 256, 4, 3
};

struct BurnDriver BurnDrvBuccanrsa = {
	"buccanrsa", "buccanrs", NULL, NULL, "1989",
	"Buccaneers (set 2)\0", NULL, "Duintronic", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_MISC, GBF_SCRFIGHT, 0,
	NULL, BuccanrsaRomInfo, BuccanrsaRomName, NULL, NULL, BuccanrsaInputInfo, BuccanrsaDIPInfo,
	BuccanrsInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 544, 256, 256, 4, 3
};
