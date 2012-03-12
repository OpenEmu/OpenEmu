#include "tiles_generic.h"
#include "m6800_intf.h"
#include "taito_m68705.h"
#include "burn_ym2203.h"
#include "burn_ym3526.h"
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
static UINT8 *DrvZ80Rom1          = NULL;
static UINT8 *DrvZ80Rom2          = NULL;
static UINT8 *DrvZ80Rom3          = NULL;
static UINT8 *DrvMcuRom           = NULL;
static UINT8 *DrvProm             = NULL;
static UINT8 *DrvZ80Ram1          = NULL;
static UINT8 *DrvZ80Ram3          = NULL;
static UINT8 *DrvSharedRam        = NULL;
static UINT8 *DrvMcuRam           = NULL;
static UINT8 *DrvVideoRam         = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvPaletteRam       = NULL;
static UINT8 *DrvTiles            = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette          = NULL;

static UINT8 DrvRomBank;
static UINT8 DrvSlaveCPUActive;
static UINT8 DrvSoundCPUActive;
static UINT8 DrvMCUActive;
static UINT8 DrvVideoEnable;
static UINT8 DrvFlipScreen;
static INT32 IC43A;
static INT32 IC43B;
static INT32 DrvSoundStatus;
static INT32 DrvSoundNmiEnable;
static INT32 DrvSoundNmiPending;
static INT32 DrvSoundLatch;

static INT32 nCyclesDone[4], nCyclesTotal[4];
static INT32 nCyclesSegment;

typedef INT32 (*BublboblCallbackFunc)();
static BublboblCallbackFunc BublboblCallbackFunction;

static UINT8 DrvMCUInUse;

static INT32 mcu_address, mcu_latch;
static UINT8 ddr1, ddr2, ddr3, ddr4;
static UINT8 port1_in, port2_in, port3_in, port4_in;
static UINT8 port1_out, port2_out, port3_out, port4_out;

static struct BurnInputInfo BublboblInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort1 + 6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 3, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort2 + 6, "p2 start"  },

	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p1 fire 2" },
	
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL  , DrvInputPort0 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Bublbobl)

static struct BurnInputInfo BoblboblInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 2, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort1 + 6, "p2 start"  },

	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 2" },
	
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort1 + 3, "service"   },
	{"Tilt"              , BIT_DIGITAL  , DrvInputPort1 + 2, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Boblbobl)

static inline void BublboblMakeInputs()
{
	DrvInput[0] = 0xf3;
	if (DrvInputPort0[0]) DrvInput[0] -= 0x01;
	if (DrvInputPort0[1]) DrvInput[0] -= 0x02;
	if (DrvInputPort0[2]) DrvInput[0] |= 0x04;
	if (DrvInputPort0[3]) DrvInput[0] |= 0x08;
	if (DrvInputPort0[4]) DrvInput[0] -= 0x10;
	if (DrvInputPort0[5]) DrvInput[0] -= 0x20;
	if (DrvInputPort0[6]) DrvInput[0] -= 0x40;
	if (DrvInputPort0[7]) DrvInput[0] -= 0x80;
	
	DrvInput[1] = 0xff;
	DrvInput[2] = 0xff;
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[1] -= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] -= (DrvInputPort2[i] & 1) << i;
	}
}

static struct BurnDIPInfo BublboblDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0xfe, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Mode"                   },
	{0x0f, 0x01, 0x05, 0x04, "Game, English"          },
	{0x0f, 0x01, 0x05, 0x05, "Game, Japanese"         },
	{0x0f, 0x01, 0x05, 0x01, "Test (Grid and Inputs)" },
	{0x0f, 0x01, 0x05, 0x00, "Test (RAM and Sound)/Pause"},
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0f, 0x01, 0x02, 0x02, "Off"                    },
	{0x0f, 0x01, 0x02, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0f, 0x01, 0x08, 0x00, "Off"                    },
	{0x0f, 0x01, 0x08, 0x08, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x0f, 0x01, 0x30, 0x10, "2 Coins 1 Play"         },
	{0x0f, 0x01, 0x30, 0x30, "1 Coin  1 Play"         },
	{0x0f, 0x01, 0x30, 0x00, "2 Coins 3 Plays"        },
	{0x0f, 0x01, 0x30, 0x20, "1 Coin  2 Plays"        },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x0f, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x0f, 0x01, 0xc0, 0xc0, "1 Coin  1 Play"         },
	{0x0f, 0x01, 0xc0, 0x00, "2 Coins 3 Plays"        },
	{0x0f, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x10, 0x01, 0x03, 0x02, "Easy"                   },
	{0x10, 0x01, 0x03, 0x03, "Normal"                 },
	{0x10, 0x01, 0x03, 0x01, "Hard"                   },
	{0x10, 0x01, 0x03, 0x00, "Very Hard"              },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x10, 0x01, 0x0c, 0x08, "20k  80k 300k"          },
	{0x10, 0x01, 0x0c, 0x0c, "30k 100k 400k"          },
	{0x10, 0x01, 0x0c, 0x04, "40k 200k 500k"          },
	{0x10, 0x01, 0x0c, 0x00, "50k 250k 500k"          },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x10, 0x01, 0x30, 0x10, "1"                      },
	{0x10, 0x01, 0x30, 0x00, "2"                      },
	{0x10, 0x01, 0x30, 0x30, "3"                      },
	{0x10, 0x01, 0x30, 0x20, "5"                      },
	
	{0   , 0xfe, 0   , 2   , "ROM Type"               },
	{0x10, 0x01, 0x80, 0x80, "IC52=512kb, IC53=none"  },
	{0x10, 0x01, 0x80, 0x00, "IC52=256kb, IC53=256kb" },
};

STDDIPINFO(Bublbobl)

static struct BurnDIPInfo BoblboblDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0xfe, NULL                     },
	{0x10, 0xff, 0xff, 0x3f, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Mode"                   },
	{0x0f, 0x01, 0x05, 0x04, "Game, English"          },
	{0x0f, 0x01, 0x05, 0x05, "Game, Japanese"         },
	{0x0f, 0x01, 0x05, 0x01, "Test (Grid and Inputs)" },
	{0x0f, 0x01, 0x05, 0x00, "Test (RAM and Sound)"   },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0f, 0x01, 0x02, 0x02, "Off"                    },
	{0x0f, 0x01, 0x02, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0f, 0x01, 0x08, 0x00, "Off"                    },
	{0x0f, 0x01, 0x08, 0x08, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x0f, 0x01, 0x30, 0x10, "2 Coins 1 Play"         },
	{0x0f, 0x01, 0x30, 0x30, "1 Coin  1 Play"         },
	{0x0f, 0x01, 0x30, 0x00, "2 Coins 3 Plays"        },
	{0x0f, 0x01, 0x30, 0x20, "1 Coin  2 Plays"        },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x0f, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x0f, 0x01, 0xc0, 0xc0, "1 Coin  1 Play"         },
	{0x0f, 0x01, 0xc0, 0x00, "2 Coins 3 Plays"        },
	{0x0f, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x10, 0x01, 0x03, 0x02, "Easy"                   },
	{0x10, 0x01, 0x03, 0x03, "Normal"                 },
	{0x10, 0x01, 0x03, 0x01, "Hard"                   },
	{0x10, 0x01, 0x03, 0x00, "Very Hard"              },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x10, 0x01, 0x0c, 0x08, "20k  80k 300k"          },
	{0x10, 0x01, 0x0c, 0x0c, "30k 100k 400k"          },
	{0x10, 0x01, 0x0c, 0x04, "40k 200k 500k"          },
	{0x10, 0x01, 0x0c, 0x00, "50k 250k 500k"          },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x10, 0x01, 0x30, 0x10, "1"                      },
	{0x10, 0x01, 0x30, 0x00, "2"                      },
	{0x10, 0x01, 0x30, 0x30, "3"                      },
	{0x10, 0x01, 0x30, 0x20, "5"                      },
	
	{0   , 0xfe, 0   , 4   , "Monster Speed"          },
	{0x10, 0x01, 0xc0, 0x00, "Normal"                 },
	{0x10, 0x01, 0xc0, 0x40, "Medium"                 },
	{0x10, 0x01, 0xc0, 0x80, "High"                   },
	{0x10, 0x01, 0xc0, 0xc0, "Very High"              },
};

STDDIPINFO(Boblbobl)

static struct BurnDIPInfo SboblbobDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0xfe, NULL                     },
	{0x10, 0xff, 0xff, 0x3f, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Game"                   },
	{0x0f, 0x01, 0x01, 0x01, "Bobble Bobble"          },
	{0x0f, 0x01, 0x01, 0x00, "Super Bobble Bobble"    },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0f, 0x01, 0x02, 0x02, "Off"                    },
	{0x0f, 0x01, 0x02, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0f, 0x01, 0x04, 0x04, "Off"                    },
	{0x0f, 0x01, 0x04, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0f, 0x01, 0x08, 0x00, "Off"                    },
	{0x0f, 0x01, 0x08, 0x08, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x0f, 0x01, 0x30, 0x10, "2 Coins 1 Play"         },
	{0x0f, 0x01, 0x30, 0x30, "1 Coin  1 Play"         },
	{0x0f, 0x01, 0x30, 0x00, "2 Coins 3 Plays"        },
	{0x0f, 0x01, 0x30, 0x20, "1 Coin  2 Plays"        },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x0f, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x0f, 0x01, 0xc0, 0xc0, "1 Coin  1 Play"         },
	{0x0f, 0x01, 0xc0, 0x00, "2 Coins 3 Plays"        },
	{0x0f, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x10, 0x01, 0x03, 0x02, "Easy"                   },
	{0x10, 0x01, 0x03, 0x03, "Normal"                 },
	{0x10, 0x01, 0x03, 0x01, "Hard"                   },
	{0x10, 0x01, 0x03, 0x00, "Very Hard"              },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x10, 0x01, 0x0c, 0x08, "20k  80k 300k"          },
	{0x10, 0x01, 0x0c, 0x0c, "30k 100k 400k"          },
	{0x10, 0x01, 0x0c, 0x04, "40k 200k 500k"          },
	{0x10, 0x01, 0x0c, 0x00, "50k 250k 500k"          },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x10, 0x01, 0x30, 0x10, "1"                      },
	{0x10, 0x01, 0x30, 0x00, "2"                      },
	{0x10, 0x01, 0x30, 0x30, "3"                      },
	{0x10, 0x01, 0x30, 0x20, "5"                      },
	
	{0   , 0xfe, 0   , 4   , "Monster Speed"          },
	{0x10, 0x01, 0xc0, 0x00, "Normal"                 },
	{0x10, 0x01, 0xc0, 0x40, "Medium"                 },
	{0x10, 0x01, 0xc0, 0x80, "High"                   },
	{0x10, 0x01, 0xc0, 0xc0, "Very High"              },
};

STDDIPINFO(Sboblbob)

static struct BurnDIPInfo DlandDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0xff, NULL                     },
	{0x10, 0xff, 0xff, 0x3f, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2   , "Game"                   },
	{0x0f, 0x01, 0x01, 0x01, "Dream Land"             },
	{0x0f, 0x01, 0x01, 0x00, "Super Dream Land"       },

	{0   , 0xfe, 0   , 4   , "Mode"                   },
	{0x0f, 0x01, 0x05, 0x04, "Game, English"          },
	{0x0f, 0x01, 0x05, 0x05, "Game, Japanese"         },
	{0x0f, 0x01, 0x05, 0x01, "Test (Grid and Inputs)" },
	{0x0f, 0x01, 0x05, 0x00, "Test (RAM and Sound)"   },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x0f, 0x01, 0x02, 0x02, "Off"                    },
	{0x0f, 0x01, 0x02, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x0f, 0x01, 0x04, 0x00, "Off"                    },
	{0x0f, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x0f, 0x01, 0x08, 0x00, "Off"                    },
	{0x0f, 0x01, 0x08, 0x08, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x0f, 0x01, 0x30, 0x10, "2 Coins 1 Play"         },
	{0x0f, 0x01, 0x30, 0x30, "1 Coin  1 Play"         },
	{0x0f, 0x01, 0x30, 0x00, "2 Coins 3 Plays"        },
	{0x0f, 0x01, 0x30, 0x20, "1 Coin  2 Plays"        },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x0f, 0x01, 0xc0, 0x40, "2 Coins 1 Play"         },
	{0x0f, 0x01, 0xc0, 0xc0, "1 Coin  1 Play"         },
	{0x0f, 0x01, 0xc0, 0x00, "2 Coins 3 Plays"        },
	{0x0f, 0x01, 0xc0, 0x80, "1 Coin  2 Plays"        },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x10, 0x01, 0x03, 0x02, "Easy"                   },
	{0x10, 0x01, 0x03, 0x03, "Normal"                 },
	{0x10, 0x01, 0x03, 0x01, "Hard"                   },
	{0x10, 0x01, 0x03, 0x00, "Very Hard"              },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x10, 0x01, 0x0c, 0x08, "20k  80k 300k"          },
	{0x10, 0x01, 0x0c, 0x0c, "30k 100k 400k"          },
	{0x10, 0x01, 0x0c, 0x04, "40k 200k 500k"          },
	{0x10, 0x01, 0x0c, 0x00, "50k 250k 500k"          },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x10, 0x01, 0x30, 0x10, "1"                      },
	{0x10, 0x01, 0x30, 0x00, "2"                      },
	{0x10, 0x01, 0x30, 0x30, "3"                      },
	{0x10, 0x01, 0x30, 0x20, "100 (Cheat)"            },
	
	{0   , 0xfe, 0   , 4   , "Monster Speed"          },
	{0x10, 0x01, 0xc0, 0x00, "Normal"                 },
	{0x10, 0x01, 0xc0, 0x40, "Medium"                 },
	{0x10, 0x01, 0xc0, 0x80, "High"                   },
	{0x10, 0x01, 0xc0, 0xc0, "Very High"              },
};

STDDIPINFO(Dland)

static struct BurnInputInfo TokioInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvInputPort0 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvInputPort1 + 6,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvInputPort1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvInputPort1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvInputPort1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvInputPort1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvInputPort1 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvInputPort1 + 4,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvInputPort0 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvInputPort2 + 6,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvInputPort2 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvInputPort2 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvInputPort2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvInputPort2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvInputPort2 + 5,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvInputPort2 + 4,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Service",		BIT_DIGITAL,	DrvInputPort0 + 1,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvInputPort0 + 0,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,		"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDip + 1,		"dip"		},
};

STDINPUTINFO(Tokio)

static struct BurnDIPInfo TokioDIPList[]=
{
	{0x13, 0xff, 0xff, 0xfe, NULL			},
	{0x14, 0xff, 0xff, 0x7e, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x13, 0x01, 0x01, 0x00, "Upright"		},
	{0x13, 0x01, 0x01, 0x01, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Enemies"		},
	{0x14, 0x01, 0x01, 0x01, "Few (Easy)"		},
	{0x14, 0x01, 0x01, 0x00, "Many (Hard)"		},

	{0   , 0xfe, 0   ,    2, "Enemy Shots"		},
	{0x14, 0x01, 0x02, 0x02, "Few (Easy)"		},
	{0x14, 0x01, 0x02, 0x00, "Many (Hard)"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x0c, "100K 400K"		},
	{0x14, 0x01, 0x0c, 0x08, "200K 400K"		},
	{0x14, 0x01, 0x0c, 0x04, "300K 400K"		},
	{0x14, 0x01, 0x0c, 0x00, "400K 400K"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x20, "4"			},
	{0x14, 0x01, 0x30, 0x10, "5"			},
	{0x14, 0x01, 0x30, 0x00, "99 (Cheat)"		},

	{0   , 0xfe, 0   ,    0, "Language"		},
	{0x14, 0x01, 0x80, 0x00, "English"		},
	{0x14, 0x01, 0x80, 0x80, "Japanese"		},
};

STDDIPINFO(Tokio)

static struct BurnRomInfo BublboblRomDesc[] = {
	{ "a78-06-1.51",   0x08000, 0x567934b6, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "a78-05-1.52",   0x10000, 0x9f8ee242, BRF_ESS | BRF_PRG }, //	 1
	
	{ "a78-08.37",     0x08000, 0xae11a07b, BRF_ESS | BRF_PRG }, //  2	Z80 #2 Program 
	
	{ "a78-07.46",     0x08000, 0x4f9a26e8, BRF_ESS | BRF_PRG }, //  3	Z80 #3 Program 
	
	{ "a78-01.17",     0x01000, 0xb1bfb53d, BRF_ESS | BRF_PRG }, //  4	MCU Program 
	
	{ "a78-09.12",     0x08000, 0x20358c22, BRF_GRA },	     //  5	Tiles
	{ "a78-10.13",     0x08000, 0x930168a9, BRF_GRA },	     //  6
	{ "a78-11.14",     0x08000, 0x9773e512, BRF_GRA },	     //  7
	{ "a78-12.15",     0x08000, 0xd045549b, BRF_GRA },	     //  8
	{ "a78-13.16",     0x08000, 0xd0af35c5, BRF_GRA },	     //  9
	{ "a78-14.17",     0x08000, 0x7b5369a8, BRF_GRA },	     //  10
	{ "a78-15.30",     0x08000, 0x6b61a413, BRF_GRA },	     //  11
	{ "a78-16.31",     0x08000, 0xb5492d97, BRF_GRA },	     //  12
	{ "a78-17.32",     0x08000, 0xd69762d5, BRF_GRA },	     //  13
	{ "a78-18.33",     0x08000, 0x9f243b68, BRF_GRA },	     //  14
	{ "a78-19.34",     0x08000, 0x66e9438c, BRF_GRA },	     //  15
	{ "a78-20.35",     0x08000, 0x9ef863ad, BRF_GRA },	     //  16
	
	{ "a71-25.41",     0x00100, 0x2d0f8545, BRF_GRA },	     //  17	PROMs
	
	{ "pal16l8.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP }, //  18	PLDs
	{ "pal16l8.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP },	//  19
	{ "pal16r4.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP },	//  20
};

STD_ROM_PICK(Bublbobl)
STD_ROM_FN(Bublbobl)

static struct BurnRomInfo Bublbob1RomDesc[] = {
	{ "a78-06.51",     0x08000, 0x32c8305b, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "a78-05.52",     0x10000, 0x53f4bc6e, BRF_ESS | BRF_PRG }, //	 1
	
	{ "a78-08.37",     0x08000, 0xae11a07b, BRF_ESS | BRF_PRG }, //  2	Z80 #2 Program 
	
	{ "a78-07.46",     0x08000, 0x4f9a26e8, BRF_ESS | BRF_PRG }, //  3	Z80 #3 Program 
	
	{ "a78-01.17",     0x01000, 0xb1bfb53d, BRF_ESS | BRF_PRG }, //  4	MCU Program 
	
	{ "a78-09.12",     0x08000, 0x20358c22, BRF_GRA },	     //  5	Tiles
	{ "a78-10.13",     0x08000, 0x930168a9, BRF_GRA },	     //  6
	{ "a78-11.14",     0x08000, 0x9773e512, BRF_GRA },	     //  7
	{ "a78-12.15",     0x08000, 0xd045549b, BRF_GRA },	     //  8
	{ "a78-13.16",     0x08000, 0xd0af35c5, BRF_GRA },	     //  9
	{ "a78-14.17",     0x08000, 0x7b5369a8, BRF_GRA },	     //  10
	{ "a78-15.30",     0x08000, 0x6b61a413, BRF_GRA },	     //  11
	{ "a78-16.31",     0x08000, 0xb5492d97, BRF_GRA },	     //  12
	{ "a78-17.32",     0x08000, 0xd69762d5, BRF_GRA },	     //  13
	{ "a78-18.33",     0x08000, 0x9f243b68, BRF_GRA },	     //  14
	{ "a78-19.34",     0x08000, 0x66e9438c, BRF_GRA },	     //  15
	{ "a78-20.35",     0x08000, 0x9ef863ad, BRF_GRA },	     //  16
	
	{ "a71-25.41",     0x00100, 0x2d0f8545, BRF_GRA },	     //  17	PROMs
	
	{ "pal16l8.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP }, //  18	PLDs
	{ "pal16l8.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP },	//  19
	{ "pal16r4.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP },	//  20
};

STD_ROM_PICK(Bublbob1)
STD_ROM_FN(Bublbob1)

static struct BurnRomInfo BublbobrRomDesc[] = {
	{ "a78-25.51",     0x08000, 0x2d901c9d, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "a78-24.52",     0x10000, 0xb7afedc4, BRF_ESS | BRF_PRG }, //	 1
	
	{ "a78-08.37",     0x08000, 0xae11a07b, BRF_ESS | BRF_PRG }, //  2	Z80 #2 Program 
	
	{ "a78-07.46",     0x08000, 0x4f9a26e8, BRF_ESS | BRF_PRG }, //  3	Z80 #3 Program 
	
	{ "a78-01.17",     0x01000, 0xb1bfb53d, BRF_ESS | BRF_PRG }, //  4	MCU Program 
	
	{ "a78-09.12",     0x08000, 0x20358c22, BRF_GRA },	     //  5	Tiles
	{ "a78-10.13",     0x08000, 0x930168a9, BRF_GRA },	     //  6
	{ "a78-11.14",     0x08000, 0x9773e512, BRF_GRA },	     //  7
	{ "a78-12.15",     0x08000, 0xd045549b, BRF_GRA },	     //  8
	{ "a78-13.16",     0x08000, 0xd0af35c5, BRF_GRA },	     //  9
	{ "a78-14.17",     0x08000, 0x7b5369a8, BRF_GRA },	     //  10
	{ "a78-15.30",     0x08000, 0x6b61a413, BRF_GRA },	     //  11
	{ "a78-16.31",     0x08000, 0xb5492d97, BRF_GRA },	     //  12
	{ "a78-17.32",     0x08000, 0xd69762d5, BRF_GRA },	     //  13
	{ "a78-18.33",     0x08000, 0x9f243b68, BRF_GRA },	     //  14
	{ "a78-19.34",     0x08000, 0x66e9438c, BRF_GRA },	     //  15
	{ "a78-20.35",     0x08000, 0x9ef863ad, BRF_GRA },	     //  16
	
	{ "a71-25.41",     0x00100, 0x2d0f8545, BRF_GRA },	     //  17	PROMs
	
	{ "pal16l8.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP }, //  18	PLDs
	{ "pal16l8.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP },	//  19
	{ "pal16r4.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP },	//  20
};

STD_ROM_PICK(Bublbobr)
STD_ROM_FN(Bublbobr)

static struct BurnRomInfo Bubbobr1RomDesc[] = {
	{ "a78-06.51",     0x08000, 0x32c8305b, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "a78-21.52",     0x10000, 0x2844033d, BRF_ESS | BRF_PRG }, //	 1
	
	{ "a78-08.37",     0x08000, 0xae11a07b, BRF_ESS | BRF_PRG }, //  2	Z80 #2 Program 
	
	{ "a78-07.46",     0x08000, 0x4f9a26e8, BRF_ESS | BRF_PRG }, //  3	Z80 #3 Program 
	
	{ "a78-01.17",     0x01000, 0xb1bfb53d, BRF_ESS | BRF_PRG }, //  4	MCU Program 
	
	{ "a78-09.12",     0x08000, 0x20358c22, BRF_GRA },	     //  5	Tiles
	{ "a78-10.13",     0x08000, 0x930168a9, BRF_GRA },	     //  6
	{ "a78-11.14",     0x08000, 0x9773e512, BRF_GRA },	     //  7
	{ "a78-12.15",     0x08000, 0xd045549b, BRF_GRA },	     //  8
	{ "a78-13.16",     0x08000, 0xd0af35c5, BRF_GRA },	     //  9
	{ "a78-14.17",     0x08000, 0x7b5369a8, BRF_GRA },	     //  10
	{ "a78-15.30",     0x08000, 0x6b61a413, BRF_GRA },	     //  11
	{ "a78-16.31",     0x08000, 0xb5492d97, BRF_GRA },	     //  12
	{ "a78-17.32",     0x08000, 0xd69762d5, BRF_GRA },	     //  13
	{ "a78-18.33",     0x08000, 0x9f243b68, BRF_GRA },	     //  14
	{ "a78-19.34",     0x08000, 0x66e9438c, BRF_GRA },	     //  15
	{ "a78-20.35",     0x08000, 0x9ef863ad, BRF_GRA },	     //  16
	
	{ "a71-25.41",     0x00100, 0x2d0f8545, BRF_GRA },	     //  17	PROMs
	
	{ "pal16l8.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP }, //  18	PLDs
	{ "pal16l8.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP },	//  19
	{ "pal16r4.bin",   0x00001, 0x00000000, BRF_OPT | BRF_NODUMP },	//  20
};

STD_ROM_PICK(Bubbobr1)
STD_ROM_FN(Bubbobr1)

static struct BurnRomInfo BoblboblRomDesc[] = {
	{ "bb3",           0x08000, 0x01f81936, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bb5",           0x08000, 0x13118eb1, BRF_ESS | BRF_PRG }, //	 1
	{ "bb4",           0x08000, 0xafda99d8, BRF_ESS | BRF_PRG }, //	 2
	
	{ "a78-08.37",     0x08000, 0xae11a07b, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	
	{ "a78-07.46",     0x08000, 0x4f9a26e8, BRF_ESS | BRF_PRG }, //  4	Z80 #3 Program 
	
	{ "a78-09.12",     0x08000, 0x20358c22, BRF_GRA },	     //  5	Tiles
	{ "a78-10.13",     0x08000, 0x930168a9, BRF_GRA },	     //  6
	{ "a78-11.14",     0x08000, 0x9773e512, BRF_GRA },	     //  7
	{ "a78-12.15",     0x08000, 0xd045549b, BRF_GRA },	     //  8
	{ "a78-13.16",     0x08000, 0xd0af35c5, BRF_GRA },	     //  9
	{ "a78-14.17",     0x08000, 0x7b5369a8, BRF_GRA },	     //  10
	{ "a78-15.30",     0x08000, 0x6b61a413, BRF_GRA },	     //  11
	{ "a78-16.31",     0x08000, 0xb5492d97, BRF_GRA },	     //  12
	{ "a78-17.32",     0x08000, 0xd69762d5, BRF_GRA },	     //  13
	{ "a78-18.33",     0x08000, 0x9f243b68, BRF_GRA },	     //  14
	{ "a78-19.34",     0x08000, 0x66e9438c, BRF_GRA },	     //  15
	{ "a78-20.35",     0x08000, 0x9ef863ad, BRF_GRA },	     //  16
	
	{ "a71-25.41",     0x00100, 0x2d0f8545, BRF_GRA },	     //  17	PROMs
	
	{ "pal16r4.u36",   0x00104, 0x22fe26ac, BRF_OPT }, 	     //  18	PLDs
	{ "pal16l8.u38",   0x00104, 0xc02d9663, BRF_OPT },	     //  19
	{ "pal16l8.u4",    0x00104, 0x077d20a8, BRF_OPT },	     //  20
};

STD_ROM_PICK(Boblbobl)
STD_ROM_FN(Boblbobl)

static struct BurnRomInfo SboblboaRomDesc[] = {
	{ "1c.bin",        0x08000, 0xf304152a, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "1a.bin",        0x08000, 0x0865209c, BRF_ESS | BRF_PRG }, //	 1
	{ "1b.bin",        0x08000, 0x1f29b5c0, BRF_ESS | BRF_PRG }, //	 2
	
	{ "1e.rom",        0x08000, 0xae11a07b, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	
	{ "1d.rom",        0x08000, 0x4f9a26e8, BRF_ESS | BRF_PRG }, //  4	Z80 #3 Program 
	
	{ "1l.rom",        0x08000, 0x20358c22, BRF_GRA },	     //  5	Tiles
	{ "1m.rom",        0x08000, 0x930168a9, BRF_GRA },	     //  6
	{ "1n.rom",        0x08000, 0x9773e512, BRF_GRA },	     //  7
	{ "1o.rom",        0x08000, 0xd045549b, BRF_GRA },	     //  8
	{ "1p.rom",        0x08000, 0xd0af35c5, BRF_GRA },	     //  9
	{ "1q.rom",        0x08000, 0x7b5369a8, BRF_GRA },	     //  10
	{ "1f.rom",        0x08000, 0x6b61a413, BRF_GRA },	     //  11
	{ "1g.rom",        0x08000, 0xb5492d97, BRF_GRA },	     //  12
	{ "1h.rom",        0x08000, 0xd69762d5, BRF_GRA },	     //  13
	{ "1i.rom",        0x08000, 0x9f243b68, BRF_GRA },	     //  14
	{ "1j.rom",        0x08000, 0x66e9438c, BRF_GRA },	     //  15
	{ "1k.rom",        0x08000, 0x9ef863ad, BRF_GRA },	     //  16
	
	{ "a71-25.41",     0x00100, 0x2d0f8545, BRF_GRA },	     //  17	PROMs
};

STD_ROM_PICK(Sboblboa)
STD_ROM_FN(Sboblboa)

static struct BurnRomInfo SboblbobRomDesc[] = {
	{ "bbb-3.rom",     0x08000, 0xf304152a, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bb5",           0x08000, 0x13118eb1, BRF_ESS | BRF_PRG }, //	 1
	{ "bbb-4.rom",     0x08000, 0x94c75591, BRF_ESS | BRF_PRG }, //	 2
	
	{ "a78-08.37",     0x08000, 0xae11a07b, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	
	{ "a78-07.46",     0x08000, 0x4f9a26e8, BRF_ESS | BRF_PRG }, //  4	Z80 #3 Program 
	
	{ "a78-09.12",     0x08000, 0x20358c22, BRF_GRA },	     //  5	Tiles
	{ "a78-10.13",     0x08000, 0x930168a9, BRF_GRA },	     //  6
	{ "a78-11.14",     0x08000, 0x9773e512, BRF_GRA },	     //  7
	{ "a78-12.15",     0x08000, 0xd045549b, BRF_GRA },	     //  8
	{ "a78-13.16",     0x08000, 0xd0af35c5, BRF_GRA },	     //  9
	{ "a78-14.17",     0x08000, 0x7b5369a8, BRF_GRA },	     //  10
	{ "a78-15.30",     0x08000, 0x6b61a413, BRF_GRA },	     //  11
	{ "a78-16.31",     0x08000, 0xb5492d97, BRF_GRA },	     //  12
	{ "a78-17.32",     0x08000, 0xd69762d5, BRF_GRA },	     //  13
	{ "a78-18.33",     0x08000, 0x9f243b68, BRF_GRA },	     //  14
	{ "a78-19.34",     0x08000, 0x66e9438c, BRF_GRA },	     //  15
	{ "a78-20.35",     0x08000, 0x9ef863ad, BRF_GRA },	     //  16
	
	{ "a71-25.41",     0x00100, 0x2d0f8545, BRF_GRA },	     //  17	PROMs
};

STD_ROM_PICK(Sboblbob)
STD_ROM_FN(Sboblbob)

static struct BurnRomInfo Bub68705RomDesc[] = {
	{ "2.bin",         0x08000, 0x32c8305b, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "3-1.bin",       0x08000, 0x980c2615, BRF_ESS | BRF_PRG }, //	 1
	{ "3.bin",         0x08000, 0xe6c698f2, BRF_ESS | BRF_PRG }, //	 2
	
	{ "4.bin",         0x08000, 0xae11a07b, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	
	{ "1.bin",         0x08000, 0x4f9a26e8, BRF_ESS | BRF_PRG }, //  4	Z80 #3 Program 
	
	{ "a78-09.12",     0x08000, 0x20358c22, BRF_GRA },	     //  5	Tiles
	{ "a78-10.13",     0x08000, 0x930168a9, BRF_GRA },	     //  6
	{ "a78-11.14",     0x08000, 0x9773e512, BRF_GRA },	     //  7
	{ "a78-12.15",     0x08000, 0xd045549b, BRF_GRA },	     //  8
	{ "a78-13.16",     0x08000, 0xd0af35c5, BRF_GRA },	     //  9
	{ "a78-14.17",     0x08000, 0x7b5369a8, BRF_GRA },	     //  10
	{ "a78-15.30",     0x08000, 0x6b61a413, BRF_GRA },	     //  11
	{ "a78-16.31",     0x08000, 0xb5492d97, BRF_GRA },	     //  12
	{ "a78-17.32",     0x08000, 0xd69762d5, BRF_GRA },	     //  13
	{ "a78-18.33",     0x08000, 0x9f243b68, BRF_GRA },	     //  14
	{ "a78-19.34",     0x08000, 0x66e9438c, BRF_GRA },	     //  15
	{ "a78-20.35",     0x08000, 0x9ef863ad, BRF_GRA },	     //  16
	
	{ "a71-25.41",     0x00100, 0x2d0f8545, BRF_GRA },	     //  17	PROMs
	
	{ "68705.bin",     0x00800, 0x78caa635, BRF_ESS | BRF_PRG }, //  18	68705 Program Code
};

STD_ROM_PICK(Bub68705)
STD_ROM_FN(Bub68705)



static struct BurnRomInfo DlandRomDesc[] = {
	{ "dl_3.u69",	   0x08000, 0x01eb3e4f, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "dl_5.u67",	   0x08000, 0x75740b61, BRF_ESS | BRF_PRG }, //  1
	{ "dl_4.u68",	   0x08000, 0xc6a3776f, BRF_ESS | BRF_PRG }, //  2

	{ "dl_1.u42",      0x08000, 0xae11a07b, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	
	{ "dl_2.u74",      0x08000, 0x4f9a26e8, BRF_ESS | BRF_PRG }, //  4	Z80 #3 Program 

	{ "dl_6.58",       0x10000, 0x6352d3fa, BRF_GRA },	     //  5	Tiles
	{ "dl_7.59",       0x10000, 0x37a38b69, BRF_GRA },	     //  6
	{ "dl_8.60",       0x10000, 0x509ee5b1, BRF_GRA },	     //  7
	{ "dl_9.61",       0x10000, 0xae8514d7, BRF_GRA },	     //  8
	{ "dl_10.62",      0x10000, 0x6d406fb7, BRF_GRA },	     //  9
	{ "dl_11.63",      0x10000, 0xbdf9c0ab, BRF_GRA },	     //  10

	{ "a71-25.41",     0x00100, 0x2d0f8545, BRF_GRA },	     //  11	PROMs
};

STD_ROM_PICK(Dland)
STD_ROM_FN(Dland)


static struct BurnRomInfo tokioRomDesc[] = {
	{ "a71-02-1.ic4",	0x8000, 0xBB8DABD7, BRF_ESS | BRF_PRG }, //  0 Z80 #1 Program
	{ "a71-03-1.ic5",	0x8000, 0xEE49B383, BRF_ESS | BRF_PRG }, //  1
	{ "a71-04.ic6",		0x8000, 0xa0a4ce0e, BRF_ESS | BRF_PRG }, //  2
	{ "a71-05.ic7",		0x8000, 0x6da0b945, BRF_ESS | BRF_PRG }, //  3
	{ "a71-06-1.ic8",	0x8000, 0x56927b3f, BRF_ESS | BRF_PRG }, //  4

	{ "a71-01.ic1",		0x8000, 0x0867c707, BRF_ESS | BRF_PRG }, //  5 Z80 #2 Program 

	{ "a71-07.ic10",	0x8000, 0xf298cc7b, BRF_ESS | BRF_PRG }, //  6 Z80 #3 Program 

	{ "a71-08.ic12",	0x8000, 0x0439ab13, BRF_GRA },	         //  7 Tiles
	{ "a71-09.ic13",	0x8000, 0xedb3d2ff, BRF_GRA },	         //  8
	{ "a71-10.ic14",	0x8000, 0x69f0888c, BRF_GRA },	         //  9
	{ "a71-11.ic15",	0x8000, 0x4ae07c31, BRF_GRA },	         // 10
	{ "a71-12.ic16",	0x8000, 0x3f6bd706, BRF_GRA },	         // 11
	{ "a71-13.ic17",	0x8000, 0xf2c92aaa, BRF_GRA },	         // 12
	{ "a71-14.ic18",	0x8000, 0xc574b7b2, BRF_GRA },	         // 13
	{ "a71-15.ic19",	0x8000, 0x12d87e7f, BRF_GRA },	         // 14
	{ "a71-16.ic30",	0x8000, 0x0bce35b6, BRF_GRA },	         // 15
	{ "a71-17.ic31",	0x8000, 0xdeda6387, BRF_GRA },	         // 16
	{ "a71-18.ic32",	0x8000, 0x330cd9d7, BRF_GRA },	         // 17
	{ "a71-19.ic33",	0x8000, 0xfc4b29e0, BRF_GRA },	         // 18
	{ "a71-20.ic34",	0x8000, 0x65acb265, BRF_GRA },	         // 19
	{ "a71-21.ic35",	0x8000, 0x33cde9b2, BRF_GRA },	         // 20
	{ "a71-22.ic36",	0x8000, 0xfb98eac0, BRF_GRA },	         // 21
	{ "a71-23.ic37",	0x8000, 0x30bd46ad, BRF_GRA },	         // 22

	{ "a71-25.ic41",	0x0100, 0x2d0f8545, BRF_GRA },	         // 23 PROMs

	{ "a71-24.ic57",	0x0800, 0x00000000, 6 | BRF_NODUMP },    // 24 Mcu Code

	{ "a71-26.ic19",	0x0117, 0x4e1f119c, 7 | BRF_OPT },       // 25 PLDs
};

STD_ROM_PICK(tokio)
STD_ROM_FN(tokio)

static struct BurnRomInfo tokiooRomDesc[] = {
	{ "a71-02.ic4",		0x8000, 0xd556c908, BRF_ESS | BRF_PRG }, //  0 Z80 #1 Program
	{ "a71-03.ic5",		0x8000, 0x69dacf44, BRF_ESS | BRF_PRG }, //  1
	{ "a71-04.ic6",		0x8000, 0xa0a4ce0e, BRF_ESS | BRF_PRG }, //  2
	{ "a71-05.ic7",		0x8000, 0x6da0b945, BRF_ESS | BRF_PRG }, //  3
	{ "a71-06.ic8",		0x8000, 0x447d6779, BRF_ESS | BRF_PRG }, //  4

	{ "a71-01.ic1",		0x8000, 0x0867c707, BRF_ESS | BRF_PRG }, //  5 Z80 #2 Program 

	{ "a71-07.ic10",	0x8000, 0xf298cc7b, BRF_ESS | BRF_PRG }, //  6 Z80 #3 Program 

	{ "a71-08.ic12",	0x8000, 0x0439ab13, BRF_GRA },	         //  7 Tiles
	{ "a71-09.ic13",	0x8000, 0xedb3d2ff, BRF_GRA },	         //  8
	{ "a71-10.ic14",	0x8000, 0x69f0888c, BRF_GRA },	         //  9
	{ "a71-11.ic15",	0x8000, 0x4ae07c31, BRF_GRA },	         // 10
	{ "a71-12.ic16",	0x8000, 0x3f6bd706, BRF_GRA },	         // 11
	{ "a71-13.ic17",	0x8000, 0xf2c92aaa, BRF_GRA },	         // 12
	{ "a71-14.ic18",	0x8000, 0xc574b7b2, BRF_GRA },	         // 13
	{ "a71-15.ic19",	0x8000, 0x12d87e7f, BRF_GRA },	         // 14
	{ "a71-16.ic30",	0x8000, 0x0bce35b6, BRF_GRA },	         // 15
	{ "a71-17.ic31",	0x8000, 0xdeda6387, BRF_GRA },	         // 16
	{ "a71-18.ic32",	0x8000, 0x330cd9d7, BRF_GRA },	         // 17
	{ "a71-19.ic33",	0x8000, 0xfc4b29e0, BRF_GRA },	         // 18
	{ "a71-20.ic34",	0x8000, 0x65acb265, BRF_GRA },	         // 19
	{ "a71-21.ic35",	0x8000, 0x33cde9b2, BRF_GRA },	         // 20
	{ "a71-22.ic36",	0x8000, 0xfb98eac0, BRF_GRA },	         // 21
	{ "a71-23.ic37",	0x8000, 0x30bd46ad, BRF_GRA },	         // 22

	{ "a71-25.ic41",	0x0100, 0x2d0f8545, BRF_GRA },	         // 23 PROMs

	{ "a71-24.ic57",	0x0800, 0x00000000, 6 | BRF_NODUMP },    // 24 Mcu Code

	{ "a71-26.ic19",	0x0117, 0x4e1f119c, 7 | BRF_OPT },       // 25 PLDs
};

STD_ROM_PICK(tokioo)
STD_ROM_FN(tokioo)

static struct BurnRomInfo tokiouRomDesc[] = {
	{ "a71-27-1.ic4",	0x8000, 0x8c180896, BRF_ESS | BRF_PRG }, //  0 Z80 #1 Program Code
	{ "a71-28-1.ic5",	0x8000, 0x1b447527, BRF_ESS | BRF_PRG }, //  1
	{ "a71-04.ic6",		0x8000, 0xa0a4ce0e, BRF_ESS | BRF_PRG }, //  2
	{ "a71-05.ic7",		0x8000, 0x6da0b945, BRF_ESS | BRF_PRG }, //  3
	{ "a71-06-1.ic8",	0x8000, 0x56927b3f, BRF_ESS | BRF_PRG }, //  4

	{ "a71-01.ic1",		0x8000, 0x0867c707, BRF_ESS | BRF_PRG }, //  5 Z80 #2 Program 

	{ "a71-07.ic10",	0x8000, 0xf298cc7b, BRF_ESS | BRF_PRG }, //  6 Z80 #3 Program 

	{ "a71-08.ic12",	0x8000, 0x0439ab13, BRF_GRA },	         //  7 Tiles
	{ "a71-09.ic13",	0x8000, 0xedb3d2ff, BRF_GRA },	         //  8
	{ "a71-10.ic14",	0x8000, 0x69f0888c, BRF_GRA },	         //  9
	{ "a71-11.ic15",	0x8000, 0x4ae07c31, BRF_GRA },	         // 10
	{ "a71-12.ic16",	0x8000, 0x3f6bd706, BRF_GRA },	         // 11
	{ "a71-13.ic17",	0x8000, 0xf2c92aaa, BRF_GRA },	         // 12
	{ "a71-14.ic18",	0x8000, 0xc574b7b2, BRF_GRA },	         // 13
	{ "a71-15.ic19",	0x8000, 0x12d87e7f, BRF_GRA },	         // 14
	{ "a71-16.ic30",	0x8000, 0x0bce35b6, BRF_GRA },	         // 15
	{ "a71-17.ic31",	0x8000, 0xdeda6387, BRF_GRA },	         // 16
	{ "a71-18.ic32",	0x8000, 0x330cd9d7, BRF_GRA },	         // 17
	{ "a71-19.ic33",	0x8000, 0xfc4b29e0, BRF_GRA },	         // 18
	{ "a71-20.ic34",	0x8000, 0x65acb265, BRF_GRA },	         // 19
	{ "a71-21.ic35",	0x8000, 0x33cde9b2, BRF_GRA },	         // 20
	{ "a71-22.ic36",	0x8000, 0xfb98eac0, BRF_GRA },	         // 21
	{ "a71-23.ic37",	0x8000, 0x30bd46ad, BRF_GRA },	         // 22

	{ "a71-25.ic41",	0x0100, 0x2d0f8545, BRF_GRA },	         // 23 PROMs

	{ "a71-24.ic57",	0x0800, 0x00000000, 6 | BRF_NODUMP },    // 24 Mcu Code

	{ "a71-26.ic19",	0x0117, 0x4e1f119c, 7 | BRF_OPT },       // 25 PLDs
};

STD_ROM_PICK(tokiou)
STD_ROM_FN(tokiou)

static struct BurnRomInfo tokiobRomDesc[] = {
	{ "2.ic4",		0x8000, 0xf583b1ef, BRF_ESS | BRF_PRG }, //  0 Z80 #1 Program Code
	{ "a71-03.ic5",		0x8000, 0x69dacf44, BRF_ESS | BRF_PRG }, //  1
	{ "a71-04.ic6",		0x8000, 0xa0a4ce0e, BRF_ESS | BRF_PRG }, //  2
	{ "a71-05.ic7",		0x8000, 0x6da0b945, BRF_ESS | BRF_PRG }, //  3
	{ "6.ic8",		0x8000, 0x1490e95b, BRF_ESS | BRF_PRG }, //  4

	{ "a71-01.ic1",		0x8000, 0x0867c707, BRF_ESS | BRF_PRG }, //  5 Z80 #2 Program 

	{ "a71-07.ic10",	0x8000, 0xf298cc7b, BRF_ESS | BRF_PRG }, //  6 Z80 #3 Program 

	{ "a71-08.ic12",	0x8000, 0x0439ab13, BRF_GRA },	         //  7 Tiles
	{ "a71-09.ic13",	0x8000, 0xedb3d2ff, BRF_GRA },	         //  8
	{ "a71-10.ic14",	0x8000, 0x69f0888c, BRF_GRA },	         //  9
	{ "a71-11.ic15",	0x8000, 0x4ae07c31, BRF_GRA },	         // 10
	{ "a71-12.ic16",	0x8000, 0x3f6bd706, BRF_GRA },	         // 11
	{ "a71-13.ic17",	0x8000, 0xf2c92aaa, BRF_GRA },	         // 12
	{ "a71-14.ic18",	0x8000, 0xc574b7b2, BRF_GRA },	         // 13
	{ "a71-15.ic19",	0x8000, 0x12d87e7f, BRF_GRA },	         // 14
	{ "a71-16.ic30",	0x8000, 0x0bce35b6, BRF_GRA },	         // 15
	{ "a71-17.ic31",	0x8000, 0xdeda6387, BRF_GRA },	         // 16
	{ "a71-18.ic32",	0x8000, 0x330cd9d7, BRF_GRA },	         // 17
	{ "a71-19.ic33",	0x8000, 0xfc4b29e0, BRF_GRA },	         // 18
	{ "a71-20.ic34",	0x8000, 0x65acb265, BRF_GRA },	         // 19
	{ "a71-21.ic35",	0x8000, 0x33cde9b2, BRF_GRA },	         // 20
	{ "a71-22.ic36",	0x8000, 0xfb98eac0, BRF_GRA },	         // 21
	{ "a71-23.ic37",	0x8000, 0x30bd46ad, BRF_GRA },	         // 22

	{ "a71-25.ic41",	0x0100, 0x2d0f8545, BRF_GRA },	         // 23 PROMs
};

STD_ROM_PICK(tokiob)
STD_ROM_FN(tokiob)


static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom1             = Next; Next += 0x30000;
	DrvZ80Rom2             = Next; Next += 0x08000;
	DrvZ80Rom3             = Next; Next += 0x0a000;
	DrvProm                = Next; Next += 0x00100;
	if (DrvMCUInUse) {
		DrvMcuRom      = Next; Next += 0x01000;
	}

	RamStart               = Next;

	DrvPaletteRam          = Next; Next += 0x00200;
	DrvVideoRam            = Next; Next += 0x01d00;
	DrvZ80Ram1             = Next; Next += 0x00400;
	DrvZ80Ram3             = Next; Next += 0x01000;
	DrvSharedRam           = Next; Next += 0x01800;
	DrvMcuRam              = Next; Next += 0x000c0;
	DrvSpriteRam           = Next; Next += 0x00300;

	RamEnd                 = Next;

	DrvTiles               = Next; Next += 0x4000 * 8 * 8;
	DrvPalette             = (UINT32*)Next; Next += 0x00100 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	for (INT32 i = 0; i < 3; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}
	
	if (DrvMCUInUse == 1) {
		M6801Reset();
	} else if (DrvMCUInUse == 2) {
		m67805_taito_reset();
	}
	
	BurnYM3526Reset();
	BurnYM2203Reset();
		
	DrvRomBank = 0;
	DrvSlaveCPUActive = 0;
	DrvSoundCPUActive = 0;
	DrvMCUActive = 0;
	DrvVideoEnable = 0;
	DrvFlipScreen = 0;
	IC43A = 0;
	IC43B = 0;
	DrvSoundStatus = 0;
	DrvSoundNmiEnable = 0;
	DrvSoundNmiPending = 0;
	DrvSoundLatch = 0;
	mcu_latch = 0;
	mcu_address = 0;

	return 0;
}

static INT32 TokioDoReset()
{
	for (INT32 i = 0; i < 3; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}

	BurnYM2203Reset();
		
	DrvRomBank = 0;
	DrvVideoEnable = 1;
	DrvFlipScreen = 0;
	DrvSoundStatus = 0;
	DrvSoundNmiEnable = 0;
	DrvSoundNmiPending = 0;
	DrvSoundLatch = 0;

	return 0;
}

UINT8 __fastcall BublboblRead1(UINT16 a)
{
	switch (a) {
		case 0xfa00: {
			return DrvSoundStatus;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall BublboblWrite1(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xfa00: {
			DrvSoundLatch = d;
			DrvSoundNmiPending = 1;
			return;
		}
		
		case 0xfa03: {
			if (d) {
				ZetClose();
				ZetOpen(2);
				ZetReset();
				ZetClose();
				ZetOpen(0);
				DrvSoundCPUActive = 0;
			} else {
				DrvSoundCPUActive = 1;
			}
			return;
		}
		
		case 0xfa80: {
			// watchdog reset
			return;
		}
		case 0xfb40: {
			DrvRomBank = (d ^ 0x04) & 0x07;
			ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
			
			if (!(d & 0x10)) {
				ZetClose();
				ZetOpen(1);
				ZetReset();
				ZetClose();
				ZetOpen(0);
				DrvSlaveCPUActive = 0;
			} else {
				DrvSlaveCPUActive = 1;
			}
			
			if (!(d & 0x20)) {
				if (DrvMCUInUse == 2) {
					 m67805_taito_reset();
				} else {
					M6801Reset();
				}
				DrvMCUActive = 0;
			} else {
				DrvMCUActive = 1;
			}
			
			DrvVideoEnable = d & 0x40;
			DrvFlipScreen = d & 0x80;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall BoblboblRead1(UINT16 a)
{
	switch (a) {
		case 0xfe00: {
			return IC43A << 4;
		}
		
		case 0xfe01:
		case 0xfe02:
		case 0xfe03: {
			return rand() & 0xff;
		}
		
		case 0xfe80: {
			return IC43B << 4;
		}
		
		case 0xfe81:
		case 0xfe82:
		case 0xfe83: {
			return 0xff;
		}
		
		case 0xff00: {
			return DrvDip[0];
		}
		
		case 0xff01: {
			return DrvDip[1];
		}
		
		case 0xff02: {
			return DrvInput[0];
		}
		
		case 0xff03: {
			return DrvInput[1];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall BoblboblWrite1(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xfa00: {
			DrvSoundLatch = d;
			DrvSoundNmiPending = 1;
			return;
		}
		
		case 0xfa03: {
			if (d) {
				ZetClose();
				ZetOpen(2);
				ZetReset();
				ZetClose();
				ZetOpen(0);
				DrvSoundCPUActive = 0;
			} else {
				DrvSoundCPUActive = 1;
			}
			return;
		}
		
		case 0xfa80: {
			// nop
			return;
		}
		
		case 0xfb40: {
			DrvRomBank = (d ^ 0x04) & 0x07;
			ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
			
			if (!(d & 0x10)) {
				ZetClose();
				ZetOpen(1);
				ZetReset();
				ZetClose();
				ZetOpen(0);
				DrvSlaveCPUActive = 0;
			} else {
				DrvSlaveCPUActive = 1;
			}
			
			DrvVideoEnable = d & 0x40;
			DrvFlipScreen = d & 0x80;
			return;
		}
		
		case 0xfe00:
		case 0xfe01:
		case 0xfe02:
		case 0xfe03: {
			INT32 Res = 0;
			INT32 Offset = a - 0xfe00;
			
			switch (Offset) {
				case 0x00: {
					if (~IC43A & 8) Res ^= 1;
					if (~IC43A & 1) Res ^= 2;
					if (~IC43A & 1) Res ^= 4;
					if (~IC43A & 2) Res ^= 4;
					if (~IC43A & 4) Res ^= 8;
					break;
				}
				
				case 0x01: {
					if (~IC43A & 8) Res ^= 1;
					if (~IC43A & 2) Res ^= 1;
					if (~IC43A & 8) Res ^= 2;
					if (~IC43A & 1) Res ^= 4;
					if (~IC43A & 4) Res ^= 8;
					break;
				}
		
				case 0x02: {
					if (~IC43A & 4) Res ^= 1;
					if (~IC43A & 8) Res ^= 2;
					if (~IC43A & 2) Res ^= 4;
					if (~IC43A & 1) Res ^= 8;
					if (~IC43A & 4) Res ^= 8;
					break;
				}
				
				case 0x03: {
					if (~IC43A & 2) Res ^= 1;
					if (~IC43A & 4) Res ^= 2;
					if (~IC43A & 8) Res ^= 2;
					if (~IC43A & 8) Res ^= 4;
					if (~IC43A & 1) Res ^= 8;
					break;
				}
			}
			
			IC43A = Res;
			return;
		}
		
		case 0xfe80:
		case 0xfe81:
		case 0xfe82:
		case 0xfe83: {
			INT32 Offset = a - 0xfe80;
			static const INT32 XorVal[4] = { 4, 1, 8, 2 };
			
			IC43B = (d >> 4) ^ XorVal[Offset];
			return;
		}
		
		case 0xff94:
		case 0xff98: {
			// nop
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall DrvSoundRead3(UINT16 a)
{
	switch (a) {
		case 0x9000: {
			return BurnYM2203Read(0, 0);
		}
		
		case 0x9001: {
			return BurnYM2203Read(0, 1);
		}
		
		case 0xa000: {
			return BurnYM3526Read(0);
		}
		
		case 0xb000: {
			return DrvSoundLatch;
		}
		
		case 0xb001: {
			// nop
			return 0;
		}
		
		case 0xe000: {
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #3 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall DrvSoundWrite3(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0x9000: {
			BurnYM2203Write(0, 0, d);
			return;
		}
		
		case 0x9001: {
			BurnYM2203Write(0, 1, d);
			return;
		}
		
		case 0xa000: {
			BurnYM3526Write(0, d);
			return;
		}
		
		case 0xa001: {
			BurnYM3526Write(1, d);
			return;
		}
		
		case 0xb000: {
			DrvSoundStatus = d;
			return;
		}
		
		case 0xb001: {
			DrvSoundNmiEnable = 1;
			if (DrvSoundNmiPending) {
				ZetNmi();
				DrvSoundNmiPending = 0;
			}
			return;
		}
		
		case 0xb002: {
			DrvSoundNmiEnable = 0;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #3 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 BublboblMcuReadByte(UINT16 Address)
{
	if (Address >= 0x0040 && Address <= 0x00ff) {
		return DrvMcuRam[Address - 0x0040];
	}
	
	switch (Address) {
		case 0x00: {
			return ddr1;
		}
		
		case 0x01: {
			return ddr2;
		}
		
		case 0x02: {
			port1_in = DrvInput[0];
			return (port1_out & ddr1) | (port1_in & ~ddr1);
		}
		
		case 0x03: {
			return (port2_out & ddr2) | (port2_in & ~ddr2);
		}
		
		case 0x04: {
			return ddr3;
		}
		
		case 0x05: {
			return ddr4;
		}
		
		case 0x06: {
			return (port3_out & ddr3) | (port3_in & ~ddr3);
		}
		
		case 0x07: {
			return (port4_out & ddr4) | (port4_in & ~ddr4);
		}
	}
	
	bprintf(PRINT_NORMAL, _T("M6801 Read Byte -> %04X\n"), Address);
	
	return 0;
}

void BublboblMcuWriteByte(UINT16 Address, UINT8 Data)
{
	if (Address >= 0x0040 && Address <= 0x00ff) {
		DrvMcuRam[Address - 0x0040] = Data;
		return;
	}
	
	if (Address > 7 && Address <= 0x001f) {
		m6803_internal_registers_w(Address, Data);
		if (Address > 7) return;
	}
	
	switch (Address) {
		case 0x00: {
			ddr1 = Data;
			return;
		}
		
		case 0x01: {
			ddr2 = Data;
			return;
		}
		
		case 0x02: {
			if ((port1_out & 0x40) && (~Data & 0x40)) {
				ZetOpen(0);
				ZetSetVector(DrvZ80Ram1[0]);
				ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
				ZetClose();
			}
			
			port1_out = Data;
			return;
		}
		
		case 0x03: {
			if ((~port2_out & 0x10) && (Data & 0x10)) {
				INT32 nAddress = port4_out | ((Data & 0x0f) << 8);
				
				if (port1_out & 0x80) {
					if (nAddress == 0x0000) port3_in = DrvDip[0];
					if (nAddress == 0x0001) port3_in = DrvDip[1];
					if (nAddress == 0x0002) port3_in = DrvInput[1];
					if (nAddress == 0x0003) port3_in = DrvInput[2];
					
					if (nAddress >= 0x0c00 && nAddress <= 0x0fff) port3_in = DrvZ80Ram1[nAddress - 0x0c00];
				} else {
					if (nAddress >= 0x0c00 && nAddress <= 0x0fff) DrvZ80Ram1[nAddress - 0x0c00] = port3_out;
				}
			}

			port2_out = Data;
			return;
		}
		
		case 0x04: {
			ddr3 = Data;
			return;
		}
		
		case 0x05: {
			ddr4 = Data;
			return;
		}
		
		case 0x06: {
			port3_out = Data;
			return;
		}
		
		case 0x07: {
			port4_out = Data;
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("M6801 Write Byte -> %04X, %02X\n"), Address, Data);
}

void bublbobl_68705_portB_out(UINT8 *bytevalue)
{
	INT32 data = *bytevalue;//lazy

	if ((ddrB & 0x01) && (~data & 0x01) && (portB_out & 0x01))
	{
		portA_in = mcu_latch;
	}
	if ((ddrB & 0x02) && (data & 0x02) && (~portB_out & 0x02))
	{
		mcu_address = (mcu_address & 0xff00) | portA_out;
	}
	if ((ddrB & 0x04) && (data & 0x04) && (~portB_out & 0x04))
	{
		mcu_address = (mcu_address & 0x00ff) | ((portA_out & 0x0f) << 8);
	}
	if ((ddrB & 0x10) && (~data & 0x10) && (portB_out & 0x10))
	{
		if (data & 0x08)	/* read */
		{
			if ((mcu_address & 0x0800) == 0x0000)
			{
				switch (mcu_address & 3) {
					case 0: mcu_latch = DrvDip[0]; break;
					case 1: mcu_latch = DrvDip[1]; break;
					case 2: mcu_latch = DrvInput[1]; break;
					case 3: mcu_latch = DrvInput[2]; break;
				}
			}
			else if ((mcu_address & 0x0c00) == 0x0c00)
			{
				mcu_latch = DrvZ80Ram1[mcu_address & 0x03ff];
			}
		}
		else
		{
			if ((mcu_address & 0x0c00) == 0x0c00)
			{
				DrvZ80Ram1[mcu_address & 0x03ff] = portA_out;
			}
		}
	}
	if ((ddrB & 0x20) && (~data & 0x20) && (portB_out & 0x20))
	{
		ZetOpen(0);

		/* hack to get random EXTEND letters (who is supposed to do this? 68705? PAL?) */
		DrvZ80Ram1[0x7c] = (ZetTotalCycles() ^ ZetPc(-1)) % 6;

		ZetSetVector(DrvZ80Ram1[0]);
		ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		ZetClose();
	}
}

static void bublbobl_68705_portC_in()
{
	portC_out = DrvInput[0];
	ddrC = 0xff;
}

static m68705_interface bub68705_m68705_interface = {
	NULL, bublbobl_68705_portB_out, NULL,
	NULL, NULL, NULL,
	NULL, NULL, bublbobl_68705_portC_in
};


void __fastcall TokioWrite1(UINT16 a, UINT8 d)
{
	switch (a)
	{
		case 0xfa80: {
			DrvRomBank = d & 0x07;
			ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
			return;
		}

		case 0xfb00: {
			DrvFlipScreen = d & 0x80;
			return;
		}

		case 0xfb80: {
			ZetClose();
			ZetOpen(1);
			ZetNmi();
			ZetClose();
			ZetOpen(0);
			return;
		}

		case 0xfc00: {
			DrvSoundLatch = d;
			DrvSoundNmiPending = 1;
			return;
		}
	}
}

UINT8 __fastcall TokioRead1(UINT16 a)
{
	switch (a)
	{
		case 0xfa03: {
			return DrvDip[0];
		}

		case 0xfa04: {
			return DrvDip[1];
		}

		case 0xfa05: {
			return DrvInput[0] & ~0x20;
		}

		case 0xfa06: {
			return DrvInput[1];
		}

		case 0xfa07: {
			return DrvInput[2];
		}

		case 0xfc00: {
			return DrvSoundStatus;
		}

		case 0xfe00: {
			return 0xbf; // Mcu read...
		}
	}

	return 0;
}

void __fastcall TokioSoundWrite3(UINT16 a, UINT8 d)
{
	switch (a)
	{
		case 0x9000: {
			DrvSoundStatus = d;
			return;
		}

		case 0xa000: {
			DrvSoundNmiEnable = 0;
			return;
		}

		case 0xa800: {
			DrvSoundNmiEnable = 1;
			if (DrvSoundNmiPending) {
				ZetNmi();
				DrvSoundNmiPending = 0;
			}
			return;
		}

		case 0xb000: {
			BurnYM2203Write(0, 0, d);
			return;
		}
		
		case 0xb001: {
			BurnYM2203Write(0, 1, d);
			return;
		}
	}
}

UINT8 __fastcall TokioSoundRead3(UINT16 a)
{
	switch (a)
	{
		case 0x9000: {
			return DrvSoundLatch;
		}

		case 0xb000: {
			return BurnYM2203Read(0, 0);
		}
		
		case 0xb001: {
			return BurnYM2203Read(0, 1);
		}
	}

	return 0;
}

inline static void DrvYM2203IRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 3000000;
}

inline static INT32 DrvYM3526SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 6000000;
}

inline static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 3000000;
}

static INT32 TilePlaneOffsets[4] = { 0, 4, 0x200000, 0x200004 };
static INT32 TileXOffsets[8]     = { 3, 2, 1, 0, 11, 10, 9, 8 };
static INT32 TileYOffsets[8]     = { 0, 16, 32, 48, 64, 80, 96, 112 };

static INT32 MachineInit()
{
	INT32 nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(BublboblRead1);
	ZetSetWriteHandler(BublboblWrite1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom1             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom1             );
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0xc000, 0xdcff, 0, DrvVideoRam            );
	ZetMapArea(0xc000, 0xdcff, 1, DrvVideoRam            );
	ZetMapArea(0xc000, 0xdcff, 2, DrvVideoRam            );
	ZetMapArea(0xdd00, 0xdfff, 0, DrvSpriteRam           );
	ZetMapArea(0xdd00, 0xdfff, 1, DrvSpriteRam           );
	ZetMapArea(0xdd00, 0xdfff, 2, DrvSpriteRam           );
	ZetMapArea(0xe000, 0xf7ff, 0, DrvSharedRam           );
	ZetMapArea(0xe000, 0xf7ff, 1, DrvSharedRam           );
	ZetMapArea(0xe000, 0xf7ff, 2, DrvSharedRam           );
	ZetMapArea(0xf800, 0xf9ff, 0, DrvPaletteRam          );
	ZetMapArea(0xf800, 0xf9ff, 1, DrvPaletteRam          );
	ZetMapArea(0xf800, 0xf9ff, 2, DrvPaletteRam          );
	ZetMapArea(0xfc00, 0xffff, 0, DrvZ80Ram1             );
	ZetMapArea(0xfc00, 0xffff, 1, DrvZ80Ram1             );
	ZetMapArea(0xfc00, 0xffff, 2, DrvZ80Ram1             );	
	ZetMemEnd();
	ZetClose();
	
	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom2             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom2             );
	ZetMapArea(0xe000, 0xf7ff, 0, DrvSharedRam           );
	ZetMapArea(0xe000, 0xf7ff, 1, DrvSharedRam           );
	ZetMapArea(0xe000, 0xf7ff, 2, DrvSharedRam           );
	ZetMemEnd();
	ZetClose();

	ZetInit(2);
	ZetOpen(2);
	ZetSetReadHandler(DrvSoundRead3);
	ZetSetWriteHandler(DrvSoundWrite3);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom3             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom3             );
	ZetMapArea(0x8000, 0x8fff, 0, DrvZ80Ram3             );
	ZetMapArea(0x8000, 0x8fff, 1, DrvZ80Ram3             );
	ZetMapArea(0x8000, 0x8fff, 2, DrvZ80Ram3             );	
	ZetMemEnd();
	ZetClose();
	
	if (DrvMCUInUse == 1) {
		M6801Init(1);
		M6801MapMemory(DrvMcuRom, 0xf000, 0xffff, M6801_ROM);
		M6801SetReadByteHandler(BublboblMcuReadByte);
		M6801SetWriteByteHandler(BublboblMcuWriteByte);
	} else if (DrvMCUInUse == 2) {

		m67805_taito_init(DrvMcuRom, DrvMcuRam, &bub68705_m68705_interface);
	}
	
	BurnYM2203Init(1, 3000000, &DrvYM2203IRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(3000000);
	
	BurnYM3526Init(3000000, NULL, &DrvYM3526SynchroniseStream, 1);
	BurnTimerAttachZetYM3526(6000000);
	
	if (BublboblCallbackFunction()) return 1;

	GenericTilesInit();
	
	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 BublboblCallback()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 1, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 2, 1); if (nRet != 0) return 1;
	
	// Load Z80 #3 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom3 + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load MCU Rom
	nRet = BurnLoadRom(DrvMcuRom  + 0x00000, 4, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x48000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x50000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x58000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x68000, 16, 1); if (nRet != 0) return 1;
	for (INT32 i = 0; i < 0x80000; i++) DrvTempRom[i] ^= 0xff;
	GfxDecode(0x4000, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x80, DrvTempRom, DrvTiles);
	
	// Load the PROM
	nRet = BurnLoadRom(DrvProm + 0x00000,  17, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 BublboblInit()
{
	BublboblCallbackFunction = BublboblCallback;
	
	DrvMCUInUse = 1;
	
	return MachineInit();
}

static INT32 BoblboblCallback()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x18000, 2, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load Z80 #3 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom3 + 0x00000, 4, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x48000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x50000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x58000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x68000, 16, 1); if (nRet != 0) return 1;
	for (INT32 i = 0; i < 0x80000; i++) DrvTempRom[i] ^= 0xff;
	GfxDecode(0x4000, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x80, DrvTempRom, DrvTiles);
	
	// Load the PROM
	nRet = BurnLoadRom(DrvProm + 0x00000,  17, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	ZetOpen(0);
	ZetSetReadHandler(BoblboblRead1);
	ZetSetWriteHandler(BoblboblWrite1);
	ZetMemCallback(0xfe00, 0xffff, 0);
	ZetMemCallback(0xfe00, 0xffff, 1);
	ZetMemCallback(0xfe00, 0xffff, 2);
	ZetMemEnd();
	ZetClose();
	
	return 0;
}

static INT32 BoblboblInit()
{
	BublboblCallbackFunction = BoblboblCallback;
	
	return MachineInit();
}

static INT32 Bub68705Callback()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x18000, 2, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load Z80 #3 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom3 + 0x00000, 4, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x48000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x50000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x58000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x68000, 16, 1); if (nRet != 0) return 1;

	for (INT32 i = 0x00000; i < 0x80000; i++)
		DrvTempRom[i] ^= 0xff;

	GfxDecode(0x4000, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x80, DrvTempRom, DrvTiles);
	
	// Load the PROM
	nRet = BurnLoadRom(DrvProm + 0x00000,  17, 1); if (nRet != 0) return 1;
	
	// Load the 68705 Rom
	nRet = BurnLoadRom(DrvMcuRom + 0x00000,  18, 1); if (nRet != 0) return 1;

	BurnFree(DrvTempRom);
	
	return 0;
}

static INT32 Bub68705Init()
{
	BublboblCallbackFunction = Bub68705Callback;
	
	DrvMCUInUse = 2;
	
	return MachineInit();
}

static INT32 DlandCallback()
{
	INT32 nRet = 0;
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x18000, 2, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load Z80 #3 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom3 + 0x00000, 4, 1); if (nRet != 0) return 1;
	
	// Load and decode the tiles
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x50000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000, 10, 1); if (nRet != 0) return 1;

	for (INT32 i = 0x00000; i < 0x40000; i++)
		DrvTempRom[i] = BITSWAP08(DrvTempRom[i],7,6,5,4,0,1,2,3) ^ 0xff;

	for (INT32 i = 0x40000; i < 0x80000; i++)
		DrvTempRom[i] = BITSWAP08(DrvTempRom[i],7,4,5,6,3,0,1,2) ^ 0xff;

	GfxDecode(0x4000, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x80, DrvTempRom, DrvTiles);
	
	// Load the PROM
	nRet = BurnLoadRom(DrvProm + 0x00000,  11, 1); if (nRet != 0) return 1;

	BurnFree(DrvTempRom);

	ZetOpen(0);
	ZetSetReadHandler(BoblboblRead1);
	ZetSetWriteHandler(BoblboblWrite1);
	ZetMemCallback(0xfe00, 0xffff, 0);
	ZetMemCallback(0xfe00, 0xffff, 1);
	ZetMemCallback(0xfe00, 0xffff, 2);
	ZetMemEnd();
	ZetClose();

	return 0;
}

static INT32 DlandInit()
{
	BublboblCallbackFunction = DlandCallback;
	
	return MachineInit();
}

static INT32 TokioInit()
{
	INT32 nLen, nRet;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	{
		DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

		// Load Z80 #1 Program Roms
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 1, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x18000, 2, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x20000, 3, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvZ80Rom1 + 0x28000, 4, 1); if (nRet != 0) return 1;

		// Load Z80 #2 Program Roms
		nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 5, 1); if (nRet != 0) return 1;
	
		// Load Z80 #3 Program Roms
		nRet = BurnLoadRom(DrvZ80Rom3 + 0x00000, 6, 1); if (nRet != 0) return 1;
	
		// Load and decode the tiles
		nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x08000,  8, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x10000,  9, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x18000, 10, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x20000, 11, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x28000, 12, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x30000, 13, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x38000, 14, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x40000, 15, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x48000, 16, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x50000, 17, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x58000, 18, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x60000, 19, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x68000, 20, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x70000, 21, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x78000, 22, 1); if (nRet != 0) return 1;
		for (INT32 i = 0; i < 0x80000; i++) DrvTempRom[i] ^= 0xff;
		GfxDecode(0x4000, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x80, DrvTempRom, DrvTiles);
	
		// Load the PROM
		nRet = BurnLoadRom(DrvProm + 0x00000,  23, 1); if (nRet != 0) return 1;

		// Load MCU Rom
		//BurnLoadRom(DrvMcuRom  + 0x00000, 24, 1);

		BurnFree(DrvTempRom);
	}

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(TokioRead1);
	ZetSetWriteHandler(TokioWrite1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom1             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom1             );
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0xc000, 0xdcff, 0, DrvVideoRam            );
	ZetMapArea(0xc000, 0xdcff, 1, DrvVideoRam            );
	ZetMapArea(0xc000, 0xdcff, 2, DrvVideoRam            );
	ZetMapArea(0xdd00, 0xdfff, 0, DrvSpriteRam           );
	ZetMapArea(0xdd00, 0xdfff, 1, DrvSpriteRam           );
	ZetMapArea(0xdd00, 0xdfff, 2, DrvSpriteRam           );
	ZetMapArea(0xe000, 0xf7ff, 0, DrvSharedRam           );
	ZetMapArea(0xe000, 0xf7ff, 1, DrvSharedRam           );
	ZetMapArea(0xe000, 0xf7ff, 2, DrvSharedRam           );
	ZetMapArea(0xf800, 0xf9ff, 0, DrvPaletteRam          );
	ZetMapArea(0xf800, 0xf9ff, 1, DrvPaletteRam          );
	ZetMapArea(0xf800, 0xf9ff, 2, DrvPaletteRam          );
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom2             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom2             );
	ZetMapArea(0x8000, 0x97ff, 0, DrvSharedRam           );
	ZetMapArea(0x8000, 0x97ff, 1, DrvSharedRam           );
	ZetMapArea(0x8000, 0x97ff, 2, DrvSharedRam           );
	ZetMemEnd();
	ZetClose();

	ZetInit(2);
	ZetOpen(2);
	ZetSetReadHandler(TokioSoundRead3);
	ZetSetWriteHandler(TokioSoundWrite3);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom3             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom3             );
	ZetMapArea(0x8000, 0x8fff, 0, DrvZ80Ram3             );
	ZetMapArea(0x8000, 0x8fff, 1, DrvZ80Ram3             );
	ZetMapArea(0x8000, 0x8fff, 2, DrvZ80Ram3             );	
	ZetMemEnd();
	ZetClose();
	
	BurnYM2203Init(1, 3000000, &DrvYM2203IRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnYM2203SetVolumeShift(2);
	BurnTimerAttachZet(3000000);
	
	GenericTilesInit();

	DrvVideoEnable = 1;
	DrvMCUInUse = 0;

	// Reset the driver
	TokioDoReset();

	return 0;
}

static INT32 DrvExit()
{
	ZetExit();
	BurnYM2203Exit();
		
	if (DrvMCUInUse == 1) M6800Exit();
	if (DrvMCUInUse == 2) m6805Exit();
	
	GenericTilesExit();
	
	BurnFree(Mem);
	
	DrvRomBank = 0;
	DrvSlaveCPUActive = 0;
	DrvSoundCPUActive = 0;
	DrvMCUActive = 0;
	DrvVideoEnable = 0;
	DrvFlipScreen = 0;
	IC43A = 0;
	IC43B = 0;
	DrvSoundStatus = 0;
	DrvSoundNmiEnable = 0;
	DrvSoundNmiPending = 0;
	DrvSoundLatch = 0;	
	DrvMCUInUse = 0;

	mcu_latch = 0;
	mcu_address = 0;

	BublboblCallbackFunction = NULL;

	return 0;
}

static INT32 BublboblExit()
{
	BurnYM3526Exit();
	return DrvExit();
}

static inline UINT8 pal4bit(UINT8 bits)
{
	bits &= 0x0f;
	return (bits << 4) | bits;
}

inline static INT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal4bit(nColour >> 12);
	g = pal4bit(nColour >>  8);
	b = pal4bit(nColour >>  4);

	return BurnHighCol(r, g, b, 0);
}

static void DrvCalcPalette()
{
	for (INT32 i = 0; i < 0x200; i += 2) {
		DrvPalette[i / 2] = CalcCol(DrvPaletteRam[i | 1] | (DrvPaletteRam[i & ~1] << 8));
	}
}

static void DrvVideoUpdate()
{
	INT32 sx, sy, xc, yc;
	INT32 GfxNum, GfxAttr, GfxOffset;
	const UINT8 *Prom;
	const UINT8 *PromLine;
	
	if (!DrvVideoEnable) return;
	
	sx = 0;

	Prom = DrvProm;
	
	for (INT32 Offset = 0; Offset < 0x300; Offset += 4) {
		if (*(UINT32 *)(&DrvSpriteRam[Offset]) == 0) continue;

		GfxNum = DrvSpriteRam[Offset + 1];
		GfxAttr = DrvSpriteRam[Offset + 3];
		PromLine = Prom + 0x80 + ((GfxNum & 0xe0) >> 1);

		GfxOffset = ((GfxNum & 0x1f) * 0x80);
		if ((GfxNum & 0xa0) == 0xa0) GfxOffset |= 0x1000;

		sy = -DrvSpriteRam[Offset + 0];

		for (yc = 0; yc < 32; yc++) {
			if (PromLine[yc / 2] & 0x08) continue;

			if (!(PromLine[yc / 2] & 0x04))	{
				sx = DrvSpriteRam[Offset + 2];
				if (GfxAttr & 0x40) sx -= 256;
			}

			for (xc = 0; xc < 2; xc++) {
				INT32 gOffs, Code, Colour, xFlip, yFlip, x, y, yPos;

				gOffs = GfxOffset + xc * 0x40 + (yc & 7) * 0x02 + (PromLine[yc / 2] & 0x03) * 0x10;
				Code = DrvVideoRam[gOffs] + 256 * (DrvVideoRam[gOffs + 1] & 0x03) + 1024 * (GfxAttr & 0x0f);
				Colour = (DrvVideoRam[gOffs + 1] & 0x3c) >> 2;
				xFlip = DrvVideoRam[gOffs + 1] & 0x40;
				yFlip = DrvVideoRam[gOffs + 1] & 0x80;
				x = sx + xc * 8;
				y = (sy + yc * 8) & 0xff;

				if (DrvFlipScreen) {
					x = 248 - x;
					y = 248 - y;
					xFlip = !xFlip;
					yFlip = !yFlip;
				}

				yPos = y - 16;
				
				if (x > 8 && x < (nScreenWidth - 8) && yPos > 8 && yPos < (nScreenHeight - 8)) {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_Mask_FlipXY(pTransDraw, Code, x, yPos, Colour, 4, 15, 0, DrvTiles);
						} else {
							Render8x8Tile_Mask_FlipX(pTransDraw, Code, x, yPos, Colour, 4, 15, 0, DrvTiles);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_Mask_FlipY(pTransDraw, Code, x, yPos, Colour, 4, 15, 0, DrvTiles);
						} else {
							Render8x8Tile_Mask(pTransDraw, Code, x, yPos, Colour, 4, 15, 0, DrvTiles);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, yPos, Colour, 4, 15, 0, DrvTiles);
						} else {
							Render8x8Tile_Mask_FlipX_Clip(pTransDraw, Code, x, yPos, Colour, 4, 15, 0, DrvTiles);
						}
					} else {
						if (yFlip) {
							Render8x8Tile_Mask_FlipY_Clip(pTransDraw, Code, x, yPos, Colour, 4, 15, 0, DrvTiles);
						} else {
							Render8x8Tile_Mask_Clip(pTransDraw, Code, x, yPos, Colour, 4, 15, 0, DrvTiles);
						}
					}
				}	
			}
		}

		sx += 16;
	}
}

static void DrvDraw()
{
	for (INT32 i = 0; i < nScreenHeight * nScreenWidth; i++) {
		pTransDraw[i] = 0x00ff;
	}

	DrvCalcPalette();
	DrvVideoUpdate();
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 100;

	if (DrvReset) DrvDoReset();

	BublboblMakeInputs();

	nCyclesTotal[0] = 6000000 / 60;
	nCyclesTotal[1] = 6000000 / 60;
	nCyclesTotal[2] = 3000000 / 60;
	nCyclesTotal[3] = (DrvMCUInUse == 2) ? (4000000 / 60) : (1000000 / 60);
	nCyclesDone[0] = nCyclesDone[1] = nCyclesDone[2] = nCyclesDone[3] = 0;
	
	ZetNewFrame();

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		BurnTimerUpdateYM3526(i * (nCyclesTotal[nCurrentCPU] / nInterleave));
		if (i == 98 && !DrvMCUInUse) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		if (i == 99 && !DrvMCUInUse) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		ZetClose();

		// Run Z80 #2
		if (DrvSlaveCPUActive) {
			nCurrentCPU = 1;
			ZetOpen(nCurrentCPU);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
			if (i == 98) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			if (i == 99) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			ZetClose();
		}
	
		// Run Z80 #3
		if (DrvSoundCPUActive) {
			nCurrentCPU = 2;
			ZetOpen(nCurrentCPU);
			BurnTimerUpdate(i * (nCyclesTotal[nCurrentCPU] / nInterleave));
			if (DrvSoundNmiPending) {
				if (DrvSoundNmiEnable) {
					ZetNmi();
					DrvSoundNmiPending = 0;
				}
			}
			ZetClose();
		}

		if (DrvMCUInUse) {
			if (DrvMCUActive) {
				nCurrentCPU = 3;
				nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
				nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
				if (DrvMCUInUse == 2) {
					nCyclesSegment = m6805Run(nCyclesSegment);
					if (i == 49) m68705SetIrqLine(0, 1 /*ASSERT_LINE*/);
					if (i == 99) m68705SetIrqLine(0, 0 /*CLEAR_LINE*/);
				} else {
					nCyclesSegment = M6801Run(nCyclesSegment);
					if (i == 98) M6801SetIRQ(0, M6801_IRQSTATUS_ACK);
					if (i == 99) M6801SetIRQ(0, M6801_IRQSTATUS_NONE);
				} 

				nCyclesDone[nCurrentCPU] += nCyclesSegment;
			}
		}
	}

	ZetOpen(0);
	BurnTimerEndFrameYM3526(nCyclesTotal[0]);
	ZetClose();
	
	if (DrvSoundCPUActive) {
		ZetOpen(2);
		BurnTimerEndFrame(nCyclesTotal[2]);
		ZetClose();
	}
	
	if (pBurnSoundOut) {
		ZetOpen(2);
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		ZetClose();
		ZetOpen(0);
		BurnYM3526Update(pBurnSoundOut, nBurnSoundLen);
		ZetClose();
	}
	
	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 TokioFrame()
{
	INT32 nInterleave = 100;

	if (DrvReset) TokioDoReset();

	BublboblMakeInputs();

	nCyclesTotal[0] = 6000000 / 60;
	nCyclesTotal[1] = 6000000 / 60;
	nCyclesTotal[2] = 3000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = nCyclesDone[2] = 0;
	
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
		if (i == 98) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		if (i == 99) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		ZetClose();

		// Run Z80 #2
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		if (i == 98) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		if (i == 99) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		ZetClose();
	
		// Run Z80 #3
		nCurrentCPU = 2;
		ZetOpen(nCurrentCPU);
		BurnTimerUpdate(i * (nCyclesTotal[nCurrentCPU] / nInterleave));
		if (DrvSoundNmiPending) {
			if (DrvSoundNmiEnable) {
				ZetNmi();
				DrvSoundNmiPending = 0;
			}
		}
		ZetClose();
	}

	ZetOpen(2);
	BurnTimerEndFrame(nCyclesTotal[2]);
	ZetClose();

	if (pBurnSoundOut) {
		ZetOpen(2);
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		ZetClose();
	}
	
	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029706;
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
		if (DrvMCUInUse == 1) M6801Scan(nAction);
		if (DrvMCUInUse == 2) m68705_taito_scan(nAction);

		BurnYM2203Scan(nAction, pnMin);

		if (strncmp(BurnDrvGetTextA(DRV_NAME), "tokio", 5) != 0) {
			BurnYM3526Scan(nAction, pnMin);
		}

		SCAN_VAR(DrvRomBank);
		SCAN_VAR(DrvSlaveCPUActive);
		SCAN_VAR(DrvSoundCPUActive);
		SCAN_VAR(DrvMCUActive);
		SCAN_VAR(DrvVideoEnable);
		SCAN_VAR(DrvFlipScreen);
		SCAN_VAR(IC43A);
		SCAN_VAR(IC43B);
		SCAN_VAR(DrvSoundStatus);
		SCAN_VAR(DrvSoundNmiEnable);
		SCAN_VAR(DrvSoundNmiPending);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(ddr1);
		SCAN_VAR(ddr2);
		SCAN_VAR(ddr3);
		SCAN_VAR(ddr4);
		SCAN_VAR(port1_in);
		SCAN_VAR(port2_in);
		SCAN_VAR(port3_in);
		SCAN_VAR(port4_in);
		SCAN_VAR(port1_out);
		SCAN_VAR(port2_out);
		SCAN_VAR(port3_out);
		SCAN_VAR(port4_out);
		SCAN_VAR(mcu_latch);
		SCAN_VAR(mcu_address);
	}
	
	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
		ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x4000));
		ZetClose();
	}

	return 0;
}

struct BurnDriver BurnDrvBublbobl = {
	"bublbobl", NULL, NULL, NULL, "1986",
	"Bubble Bobble\0", NULL, "Taito Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, BublboblRomInfo, BublboblRomName, NULL, NULL, BublboblInputInfo, BublboblDIPInfo,
	BublboblInit, BublboblExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvBublbob1 = {
	"bublbobl1", "bublbobl", NULL, NULL, "1986",
	"Bubble Bobble (older)\0", NULL, "Taito Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, Bublbob1RomInfo, Bublbob1RomName, NULL, NULL, BublboblInputInfo, BublboblDIPInfo,
	BublboblInit, BublboblExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvBublbobr = {
	"bublboblr", "bublbobl", NULL, NULL, "1986",
	"Bubble Bobble (US with mode select)\0", NULL, "Taito America Corporation (Romstar license)", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, BublbobrRomInfo, BublbobrRomName, NULL, NULL, BublboblInputInfo, BublboblDIPInfo,
	BublboblInit, BublboblExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvBubbobr1 = {
	"bublboblr1", "bublbobl", NULL, NULL, "1986",
	"Bubble Bobble (US)\0", NULL, "Taito America Corporation (Romstar license)", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, Bubbobr1RomInfo, Bubbobr1RomName, NULL, NULL, BublboblInputInfo, BublboblDIPInfo,
	BublboblInit, BublboblExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvBoblbobl = {
	"boblbobl", "bublbobl", NULL, NULL, "1986",
	"Bobble Bobble\0", NULL, "bootleg", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, BoblboblRomInfo, BoblboblRomName, NULL, NULL, BoblboblInputInfo, BoblboblDIPInfo,
	BoblboblInit, BublboblExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSboblboa = {
	"sboblboa", "bublbobl", NULL, NULL, "1986",
	"Super Bobble Bobble (set 1)\0", NULL, "bootleg", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, SboblboaRomInfo, SboblboaRomName, NULL, NULL, BoblboblInputInfo, BoblboblDIPInfo,
	BoblboblInit, BublboblExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSboblbob = {
	"sboblbob", "bublbobl", NULL, NULL, "1986",
	"Super Bobble Bobble (set 2)\0", NULL, "bootleg", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, SboblbobRomInfo, SboblbobRomName, NULL, NULL, BoblboblInputInfo, SboblbobDIPInfo,
	BoblboblInit, BublboblExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvBub68705 = {
	"bub68705", "bublbobl", NULL, NULL, "1986",
	"Bubble Bobble (boolteg with 68705)\0", NULL, "bootleg", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, Bub68705RomInfo, Bub68705RomName, NULL, NULL, BublboblInputInfo, BublboblDIPInfo,
	Bub68705Init, BublboblExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvDland = {
	"dland", "bublbobl", NULL, NULL, "1987",
	"Dream Land / Super Dream Land (bootleg of Bubble Bobble)\0", NULL, "bootleg", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TAITO_MISC, GBF_PLATFORM, 0,
	NULL, DlandRomInfo, DlandRomName, NULL, NULL, BoblboblInputInfo, DlandDIPInfo,
	DlandInit, BublboblExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvTokio = {
	"tokio", NULL, NULL, NULL, "1986",
	"Tokio / Scramble Formation (newer)\0", NULL, "Taito Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_VERSHOOT, 0,
	NULL, tokioRomInfo, tokioRomName, NULL, NULL, TokioInputInfo, TokioDIPInfo,
	TokioInit, DrvExit, TokioFrame, NULL, DrvScan,
	NULL, 0x100, 224, 256, 3, 4
};

struct BurnDriverD BurnDrvTokioo = {
	"tokioo", "tokio", NULL, NULL, "1986",
	"Tokio / Scramble Formation (older)\0", NULL, "Taito Corporation", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_VERSHOOT, 0,
	NULL, tokiooRomInfo, tokiooRomName, NULL, NULL, TokioInputInfo, TokioDIPInfo,
	TokioInit, DrvExit, TokioFrame, NULL, DrvScan,
	NULL, 0x100, 224, 256, 3, 4
};

struct BurnDriverD BurnDrvTokiou = {
	"tokiou", "tokio", NULL, NULL, "1986",
	"Tokio / Scramble Formation (US)\0", NULL, "Taito America Corporation (Romstar license)", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_VERSHOOT, 0,
	NULL, tokiouRomInfo, tokiouRomName, NULL, NULL, TokioInputInfo, TokioDIPInfo,
	TokioInit, DrvExit, TokioFrame, NULL, DrvScan,
	NULL, 0x100, 224, 256, 3, 4
};

struct BurnDriver BurnDrvTokiob = {
	"tokiob", "tokio", NULL, NULL, "1986",
	"Tokio / Scramble Formation (bootleg)\0", NULL, "bootleg", "Taito Misc",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TAITO_MISC, GBF_VERSHOOT, 0,
	NULL, tokiobRomInfo, tokiobRomName, NULL, NULL, TokioInputInfo, TokioDIPInfo,
	TokioInit, DrvExit, TokioFrame, NULL, DrvScan,
	NULL, 0x100, 224, 256, 3, 4
};
