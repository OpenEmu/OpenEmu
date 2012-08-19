#include "tiles_generic.h"
#include "zet.h"

#include "driver.h"
extern "C" {
 #include "ay8910.h"
}

static UINT8 KyugoInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 KyugoInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 KyugoInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 KyugoDip[2]        = {0, 0};
static UINT8 KyugoInput[3]      = {0x00, 0x00, 0x00};
static UINT8 KyugoReset         = 0;

static UINT8 *Mem                   = NULL;
static UINT8 *MemEnd                = NULL;
static UINT8 *RamStart              = NULL;
static UINT8 *RamEnd                = NULL;
static UINT8 *KyugoZ80Rom1          = NULL;
static UINT8 *KyugoZ80Rom2          = NULL;
static UINT8 *KyugoSharedZ80Ram     = NULL;
static UINT8 *KyugoZ80Ram2          = NULL;
static UINT8 *KyugoSprite1Ram       = NULL;
static UINT8 *KyugoSprite2Ram       = NULL;
static UINT8 *KyugoFgVideoRam       = NULL;
static UINT8 *KyugoBgVideoRam       = NULL;
static UINT8 *KyugoBgAttrRam        = NULL;
static UINT8 *KyugoPromRed          = NULL;
static UINT8 *KyugoPromGreen        = NULL;
static UINT8 *KyugoPromBlue         = NULL;
static UINT8 *KyugoPromCharLookup   = NULL;
static UINT8 *KyugoChars            = NULL;
static UINT8 *KyugoTiles            = NULL;
static UINT8 *KyugoSprites          = NULL;
static UINT8 *KyugoTempRom          = NULL;
static UINT32 *KyugoPalette         = NULL;
static INT16* pFMBuffer;
static INT16* pAY8910Buffer[6];

static UINT8 KyugoIRQEnable;
static UINT8 KyugoSubCPUEnable;
static UINT8 KyugoFgColour;
static UINT8 KyugoBgPaletteBank;
static UINT8 KyugoBgScrollXHi;
static UINT8 KyugoBgScrollXLo;
static UINT8 KyugoBgScrollY;
static UINT8 KyugoFlipScreen;

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static INT32 KyugoNumZ80Rom1;
static INT32 KyugoNumZ80Rom2;
static INT32 KyugoNumSpriteRom;
static INT32 KyugoSizeZ80Rom1;
static INT32 KyugoSizeZ80Rom2;
static INT32 KyugoSizeSpriteRom;

static struct BurnInputInfo KyugoInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , KyugoInputPort0 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , KyugoInputPort0 + 3, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , KyugoInputPort0 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , KyugoInputPort0 + 4, "p2 start"  },

	{"Up"                , BIT_DIGITAL  , KyugoInputPort1 + 2, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , KyugoInputPort1 + 3, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , KyugoInputPort1 + 0, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , KyugoInputPort1 + 1, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , KyugoInputPort1 + 4, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , KyugoInputPort1 + 5, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL  , KyugoInputPort2 + 2, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL  , KyugoInputPort2 + 3, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL  , KyugoInputPort2 + 0, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , KyugoInputPort2 + 1, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , KyugoInputPort2 + 4, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL  , KyugoInputPort2 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &KyugoReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , KyugoInputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, KyugoDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, KyugoDip + 1       , "dip"       },
};


STDINPUTINFO(Kyugo)

inline void KyugoClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

inline void KyugoMakeInputs()
{
	// Reset Inputs
	KyugoInput[0] = KyugoInput[1] = KyugoInput[2] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		KyugoInput[0] |= (KyugoInputPort0[i] & 1) << i;
		KyugoInput[1] |= (KyugoInputPort1[i] & 1) << i;
		KyugoInput[2] |= (KyugoInputPort2[i] & 1) << i;
	}

	// Clear Opposites
	KyugoClearOpposites(&KyugoInput[1]);
	KyugoClearOpposites(&KyugoInput[2]);
}

static struct BurnDIPInfo AirwolfDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xbb, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x03, "4"                      },
	{0x12, 0x01, 0x03, 0x02, "5"                      },
	{0x12, 0x01, 0x03, 0x01, "6"                      },
	{0x12, 0x01, 0x03, 0x00, "7"                      },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x12, 0x01, 0x04, 0x04, "Off"                    },
	{0x12, 0x01, 0x04, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Slow Motion"            },
	{0x12, 0x01, 0x08, 0x08, "Off"                    },
	{0x12, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x12, 0x01, 0x10, 0x10, "Off"                    },
	{0x12, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Sound Test"             },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x40, 0x00, "Upright"                },
	{0x12, 0x01, 0x40, 0x40, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },	
	
	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x13, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x01, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x07, 0x03, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0x07, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x13, 0x01, 0x38, 0x00, "5 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x08, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x10, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x18, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x38, 0x20, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
};

STDDIPINFO(Airwolf)

static struct BurnDIPInfo SkywolfDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xbb, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x02, "4"                      },
	{0x12, 0x01, 0x03, 0x01, "5"                      },
	{0x12, 0x01, 0x03, 0x00, "6"                      },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x12, 0x01, 0x04, 0x04, "Off"                    },
	{0x12, 0x01, 0x04, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Slow Motion"            },
	{0x12, 0x01, 0x08, 0x08, "Off"                    },
	{0x12, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x12, 0x01, 0x10, 0x10, "Off"                    },
	{0x12, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Sound Test"             },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x40, 0x00, "Upright"                },
	{0x12, 0x01, 0x40, 0x40, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },	
	
	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x13, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x01, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x07, 0x03, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0x07, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x13, 0x01, 0x38, 0x00, "5 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x08, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x10, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x18, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x38, 0x20, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
};

STDDIPINFO(Skywolf)

static struct BurnDIPInfo FlashgalDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xaf, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x02, "4"                      },
	{0x12, 0x01, 0x03, 0x01, "5"                      },
	{0x12, 0x01, 0x03, 0x00, "6"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x12, 0x01, 0x04, 0x04, "Every 50000"            },
	{0x12, 0x01, 0x04, 0x00, "Every 70000"            },
	
	{0   , 0xfe, 0   , 2   , "Slow Motion"            },
	{0x12, 0x01, 0x08, 0x08, "Off"                    },
	{0x12, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x12, 0x01, 0x10, 0x10, "Off"                    },
	{0x12, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Sound Test"             },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x40, 0x00, "Upright"                },
	{0x12, 0x01, 0x40, 0x40, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },	
	
	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x13, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x01, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x07, 0x03, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0x07, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x13, 0x01, 0x38, 0x00, "5 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x08, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x10, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x18, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x38, 0x20, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
};

STDDIPINFO(Flashgal)

static struct BurnDIPInfo GyrodineDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xbf, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x02, "4"                      },
	{0x12, 0x01, 0x03, 0x01, "5"                      },
	{0x12, 0x01, 0x03, 0x00, "6"                      },
	
	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x12, 0x01, 0x10, 0x10, "Easy"                   },
	{0x12, 0x01, 0x10, 0x00, "Hard"                   },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x12, 0x01, 0x20, 0x20, "20000 50000"            },
	{0x12, 0x01, 0x20, 0x00, "40000 70000"            },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x40, 0x00, "Upright"                },
	{0x12, 0x01, 0x40, 0x40, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },	
	
	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x13, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x01, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x07, 0x03, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0x07, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x13, 0x01, 0x38, 0x00, "5 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x08, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x10, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x18, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x38, 0x20, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
};

STDDIPINFO(Gyrodine)

static struct BurnDIPInfo LegendDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xbf, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x02, "4"                      },
	{0x12, 0x01, 0x03, 0x01, "5"                      },
	{0x12, 0x01, 0x03, 0x00, "6"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life / Continue"  },
	{0x12, 0x01, 0x04, 0x04, "Every 50000 / No"       },
	{0x12, 0x01, 0x04, 0x00, "Every 70000 / Yes"      },
	
	{0   , 0xfe, 0   , 2   , "Slow Motion"            },
	{0x12, 0x01, 0x08, 0x08, "Off"                    },
	{0x12, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Sound Test"             },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x40, 0x00, "Upright"                },
	{0x12, 0x01, 0x40, 0x40, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },	
	
	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x13, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x01, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x07, 0x03, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0x07, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x13, 0x01, 0x38, 0x00, "5 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x08, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x10, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x18, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x38, 0x20, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
};

STDDIPINFO(Legend)

static struct BurnDIPInfo SonofphxDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xbf, NULL                     },
	{0x13, 0xff, 0xff, 0xbf, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x02, "4"                      },
	{0x12, 0x01, 0x03, 0x01, "5"                      },
	{0x12, 0x01, 0x03, 0x00, "6"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x12, 0x01, 0x04, 0x04, "Every 50000"            },
	{0x12, 0x01, 0x04, 0x00, "Every 70000"            },
	
	{0   , 0xfe, 0   , 2   , "Slow Motion"            },
	{0x12, 0x01, 0x08, 0x08, "Off"                    },
	{0x12, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x12, 0x01, 0x10, 0x10, "Off"                    },
	{0x12, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Sound Test"             },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x40, 0x00, "Upright"                },
	{0x12, 0x01, 0x40, 0x40, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },	
	
	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x13, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x01, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x07, 0x03, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0x07, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x13, 0x01, 0x38, 0x00, "5 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x08, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x10, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x18, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x38, 0x20, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0xc0, 0xc0, "Easy"                   },
	{0x13, 0x01, 0xc0, 0x80, "Normal"                 },
	{0x13, 0x01, 0xc0, 0x40, "Hard"                   },
	{0x13, 0x01, 0xc0, 0x00, "Hardest"                },
};

STDDIPINFO(Sonofphx)

static struct BurnDIPInfo SrdmissnDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xbf, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x02, "4"                      },
	{0x12, 0x01, 0x03, 0x01, "5"                      },
	{0x12, 0x01, 0x03, 0x00, "6"                      },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life / Continue"  },
	{0x12, 0x01, 0x04, 0x04, "Every 50000 / No"       },
	{0x12, 0x01, 0x04, 0x00, "Every 70000 / Yes"      },
	
	{0   , 0xfe, 0   , 2   , "Slow Motion"            },
	{0x12, 0x01, 0x08, 0x08, "Off"                    },
	{0x12, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x12, 0x01, 0x10, 0x10, "Off"                    },
	{0x12, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Sound Test"             },
	{0x12, 0x01, 0x20, 0x20, "Off"                    },
	{0x12, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x40, 0x00, "Upright"                },
	{0x12, 0x01, 0x40, 0x40, "Cocktail"               },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },	
	
	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x13, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x01, "3 Coins 2 Plays"        },
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x07, 0x03, "1 Coin  6 Plays"        },
	{0x13, 0x01, 0x07, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x13, 0x01, 0x38, 0x00, "5 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x08, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x10, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x18, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x38, 0x20, "3 Coins 4 Plays"        },
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
};

STDDIPINFO(Srdmissn)

static struct BurnRomInfo AirwolfRomDesc[] = {
	{ "b.2s",          0x08000, 0x8c993cce, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	
	{ "a.7s",          0x08000, 0xa3c7af5c, BRF_ESS | BRF_PRG }, //  1	Z80 #2 Program 
	
	{ "f.4a",          0x01000, 0x4df44ce9, BRF_GRA },	     //  2	Characters
	
	{ "09h_14.bin",    0x02000, 0x25e57e1f, BRF_GRA },	     //  3	Tiles
	{ "10h_13.bin",    0x02000, 0xcf0de5e9, BRF_GRA },	     //  4
	{ "11h_12.bin",    0x02000, 0x4050c048, BRF_GRA },	     //  5
	
	{ "e.6a",          0x08000, 0xe8fbc7d2, BRF_GRA },	     //  6	Sprites
	{ "d.8a",          0x08000, 0xc5d4156b, BRF_GRA },	     //  7
	{ "c.10a",         0x08000, 0xde91dfb1, BRF_GRA },	     //  8
	
	{ "01j.bin",       0x00100, 0x6a94b2a3, BRF_GRA },	     //  9	PROMs
	{ "01h.bin",       0x00100, 0xec0923d3, BRF_GRA },	     //  10
	{ "01f.bin",       0x00100, 0xade97052, BRF_GRA },	     //  11
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  12
	
	{ "pal16l8a.2j",   0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },	     //  13	PLDs
	{ "epl12p6a.9j",   0x00034, 0x19808f14, BRF_OPT },		     //  14
	{ "epl12p6a.9k",   0x00034, 0xf5acad85, BRF_OPT },		     //  15
};

STD_ROM_PICK(Airwolf)
STD_ROM_FN(Airwolf)

static struct BurnRomInfo AirwolfaRomDesc[] = {
	{ "airwolf.2",     0x08000, 0xbc1a8587, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	
	{ "airwolf.1",     0x08000, 0xa3c7af5c, BRF_ESS | BRF_PRG }, //  1	Z80 #2 Program 
	
	{ "airwolf.6",     0x02000, 0x5b0a01e9, BRF_GRA },	     //  2	Characters
	
	{ "airwolf.9",     0x02000, 0x25e57e1f, BRF_GRA },	     //  3	Tiles
	{ "airwolf.8",     0x02000, 0xcf0de5e9, BRF_GRA },	     //  4
	{ "airwolf.7",     0x02000, 0x4050c048, BRF_GRA },	     //  5
	
	{ "airwolf.5",     0x08000, 0xe8fbc7d2, BRF_GRA },	     //  6	Sprites
	{ "airwolf.4",     0x08000, 0xc5d4156b, BRF_GRA },	     //  7
	{ "airwolf.3",     0x08000, 0xde91dfb1, BRF_GRA },	     //  8
	
	{ "01j.bin",       0x00100, 0x6a94b2a3, BRF_GRA },	     //  9	PROMs
	{ "01h.bin",       0x00100, 0xec0923d3, BRF_GRA },	     //  10
	{ "01f.bin",       0x00100, 0xade97052, BRF_GRA },	     //  11
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  12
	
	{ "pal16l8a.2j",   0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },	     //  13	PLDs
	{ "epl12p6a.9j",   0x00034, 0x19808f14, BRF_OPT },		     //  14
	{ "epl12p6a.9k",   0x00034, 0xf5acad85, BRF_OPT },		     //  15
};

STD_ROM_PICK(Airwolfa)
STD_ROM_FN(Airwolfa)

static struct BurnRomInfo SkywolfRomDesc[] = {
	{ "02s_03.bin",    0x04000, 0xa0891798, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "03s_04.bin",    0x04000, 0x5f515d46, BRF_ESS | BRF_PRG }, //  1	Z80
	
	{ "07s_01.bin",    0x04000, 0xc680a905, BRF_ESS | BRF_PRG }, //  2	Z80 #2 Program 
	{ "08s_02.bin",    0x04000, 0x3d66bf26, BRF_ESS | BRF_PRG }, //  3	Z80
	
	{ "04a_11.bin",    0x01000, 0x219de9aa, BRF_GRA },	     //  4	Characters
	
	{ "09h_14.bin",    0x02000, 0x25e57e1f, BRF_GRA },	     //  5	Tiles
	{ "10h_13.bin",    0x02000, 0xcf0de5e9, BRF_GRA },	     //  6
	{ "11h_12.bin",    0x02000, 0x4050c048, BRF_GRA },	     //  7
	
	{ "06a_10.bin",    0x04000, 0x1c809383, BRF_GRA },	     //  6	Sprites
	{ "07a_09.bin",    0x04000, 0x5665d774, BRF_GRA },	     //  7
	{ "08a_08.bin",    0x04000, 0x6dda8f2a, BRF_GRA },	     //  8
	{ "09a_07.bin",    0x04000, 0x6a21ddb8, BRF_GRA },	     //  9
	{ "10a_06.bin",    0x04000, 0xf2e548e0, BRF_GRA },	     //  10
	{ "11a_05.bin",    0x04000, 0x8681b112, BRF_GRA },	     //  11
	
	{ "01j.bin",       0x00100, 0x6a94b2a3, BRF_GRA },	     //  12	PROMs
	{ "01h.bin",       0x00100, 0xec0923d3, BRF_GRA },	     //  13
	{ "01f.bin",       0x00100, 0xade97052, BRF_GRA },	     //  14
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  15
};

STD_ROM_PICK(Skywolf)
STD_ROM_FN(Skywolf)

static struct BurnRomInfo Skywolf2RomDesc[] = {
	{ "z80_2.bin",     0x08000, 0x34db7bda, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	
	{ "07s_01.bin",    0x04000, 0xc680a905, BRF_ESS | BRF_PRG }, //  1	Z80 #2 Program 
	{ "08s_02.bin",    0x04000, 0x3d66bf26, BRF_ESS | BRF_PRG }, //  2	Z80
	
	{ "04a_11.bin",    0x01000, 0x219de9aa, BRF_GRA },	     //  3	Characters
	
	{ "09h_14.bin",    0x02000, 0x25e57e1f, BRF_GRA },	     //  4	Tiles
	{ "10h_13.bin",    0x02000, 0xcf0de5e9, BRF_GRA },	     //  5
	{ "11h_12.bin",    0x02000, 0x4050c048, BRF_GRA },	     //  6
	
	{ "06a_10.bin",    0x04000, 0x1c809383, BRF_GRA },	     //  7	Sprites
	{ "07a_09.bin",    0x04000, 0x5665d774, BRF_GRA },	     //  8
	{ "08a_08.bin",    0x04000, 0x6dda8f2a, BRF_GRA },	     //  9
	{ "09a_07.bin",    0x04000, 0x6a21ddb8, BRF_GRA },	     //  10
	{ "10a_06.bin",    0x04000, 0xf2e548e0, BRF_GRA },	     //  11
	{ "11a_05.bin",    0x04000, 0x8681b112, BRF_GRA },	     //  12
	
	{ "01j.bin",       0x00100, 0x6a94b2a3, BRF_GRA },	     //  13	PROMs
	{ "01h.bin",       0x00100, 0xec0923d3, BRF_GRA },	     //  14
	{ "01f.bin",       0x00100, 0xade97052, BRF_GRA },	     //  15
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  16
};

STD_ROM_PICK(Skywolf2)
STD_ROM_FN(Skywolf2)

static struct BurnRomInfo Skywolf3RomDesc[] = {
	{ "1.bin",         0x08000, 0x74a86ec8, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "2.bin",         0x08000, 0xf02143de, BRF_ESS | BRF_PRG }, //  1
	
	{ "3.bin",         0x08000, 0x787cdd0a, BRF_ESS | BRF_PRG }, //  2	Z80 #2 Program 
	{ "4.bin",         0x08000, 0x07a2c814, BRF_ESS | BRF_PRG }, //  3	Z80
	
	{ "8.bin",         0x08000, 0xb86d3dac, BRF_GRA },	     //  4	Characters
	
	{ "11.bin",        0x08000, 0xfc7bbf7a, BRF_GRA },	     //  5	Tiles
	{ "10.bin",        0x08000, 0x1a3710ab, BRF_GRA },	     //  6
	{ "9.bin",         0x08000, 0xa184349a, BRF_GRA },	     //  7
	
	{ "7.bin",         0x08000, 0x086612e8, BRF_GRA },	     //  8	Sprites
	{ "6.bin",         0x08000, 0x3a9beabd, BRF_GRA },	     //  9
	{ "5.bin",         0x08000, 0xbd83658e, BRF_GRA },	     //  10
	
	{ "82s129-1.bin",  0x00100, 0x6a94b2a3, BRF_GRA },	     //  11	PROMs
	{ "82s129-2.bin",  0x00100, 0xec0923d3, BRF_GRA },	     //  12
	{ "82s129-3.bin",  0x00100, 0xade97052, BRF_GRA },	     //  13
	{ "74s288-2.bin",  0x00020, 0x190a55ad, BRF_GRA },	     //  14
	{ "74s288-1.bin",  0x00020, 0x5ddb2d15, BRF_GRA },	     //  15
};

STD_ROM_PICK(Skywolf3)
STD_ROM_FN(Skywolf3)

static struct BurnRomInfo FlashgalRomDesc[] = {
	{ "epr-7167.4f",   0x02000, 0xcf5ad733, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "epr-7168.4h",   0x02000, 0x00c4851f, BRF_ESS | BRF_PRG }, //  1
	{ "epr-7169.4j",   0x02000, 0x1ef0b8f7, BRF_ESS | BRF_PRG }, //  2
	{ "epr-7170.4k",   0x02000, 0x885d53de, BRF_ESS | BRF_PRG }, //  3
	
	{ "epr-7163.2f",   0x02000, 0xeee2134d, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program 
	{ "epr-7164.2h",   0x02000, 0xe5e0cd22, BRF_ESS | BRF_PRG }, //  5
	{ "epr-7165.2j",   0x02000, 0x4cd3fe5e, BRF_ESS | BRF_PRG }, //  6
	{ "epr-7166.2k",   0x02000, 0x552ca339, BRF_ESS | BRF_PRG }, //  7
	
	{ "epr-7177.4a",   0x01000, 0xdca9052f, BRF_GRA },	     //  8	Characters
	
	{ "epr-7178.9h",   0x02000, 0x2f5b62c0, BRF_GRA },	     //  9	Tiles
	{ "epr-7179.10h",  0x02000, 0x8fbb49b5, BRF_GRA },	     //  10
	{ "epr-7180.11h",  0x02000, 0x26a8e5c3, BRF_GRA },	     //  11
	
	{ "epr-7171.6a",   0x04000, 0x62caf2a1, BRF_GRA },	     //  12	Sprites
	{ "epr-7172.7a",   0x04000, 0x10f78a10, BRF_GRA },	     //  13
	{ "epr-7173.8a",   0x04000, 0x36ea1d59, BRF_GRA },	     //  14
	{ "epr-7174.9a",   0x04000, 0xf527d837, BRF_GRA },	     //  15
	{ "epr-7175.10a",  0x04000, 0xba76e4c1, BRF_GRA },	     //  16
	{ "epr-7176.11a",  0x04000, 0xf095d619, BRF_GRA },	     //  17
	
	{ "7161.1j",       0x00100, 0x02c4043f, BRF_GRA },	     //  18	PROMs
	{ "7160.1h",       0x00100, 0x225938d1, BRF_GRA },	     //  19
	{ "7159.1f",       0x00100, 0x1e0a1cd3, BRF_GRA },	     //  20
	{ "7162.5j",       0x00020, 0xcce2e29f, BRF_GRA },	     //  21
	{ "bpr.2c",        0x00020, 0x83a39201, BRF_GRA },	     //  22
};

STD_ROM_PICK(Flashgal)
STD_ROM_FN(Flashgal)

static struct BurnRomInfo FlashgalaRomDesc[] = {
	{ "flashgal.5",    0x02000, 0xaa889ace, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "epr-7168.4h",   0x02000, 0x00c4851f, BRF_ESS | BRF_PRG }, //  1
	{ "epr-7169.4j",   0x02000, 0x1ef0b8f7, BRF_ESS | BRF_PRG }, //  2
	{ "epr-7170.4k",   0x02000, 0x885d53de, BRF_ESS | BRF_PRG }, //  3
	
	{ "flashgal.1",    0x02000, 0x55171cc1, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program 
	{ "flashgal.2",    0x02000, 0x3fd21aac, BRF_ESS | BRF_PRG }, //  5
	{ "flashgal.3",    0x02000, 0xa1223b74, BRF_ESS | BRF_PRG }, //  6
	{ "flashgal.4",    0x02000, 0x04d2a05f, BRF_ESS | BRF_PRG }, //  7
	
	{ "epr-7177.4a",   0x01000, 0xdca9052f, BRF_GRA },	     //  8	Characters
	
	{ "epr-7178.9h",   0x02000, 0x2f5b62c0, BRF_GRA },	     //  9	Tiles
	{ "epr-7179.10h",  0x02000, 0x8fbb49b5, BRF_GRA },	     //  10
	{ "epr-7180.11h",  0x02000, 0x26a8e5c3, BRF_GRA },	     //  11
	
	{ "epr-7171.6a",   0x04000, 0x62caf2a1, BRF_GRA },	     //  12	Sprites
	{ "epr-7172.7a",   0x04000, 0x10f78a10, BRF_GRA },	     //  13
	{ "epr-7173.8a",   0x04000, 0x36ea1d59, BRF_GRA },	     //  14
	{ "epr-7174.9a",   0x04000, 0xf527d837, BRF_GRA },	     //  15
	{ "epr-7175.10a",  0x04000, 0xba76e4c1, BRF_GRA },	     //  16
	{ "epr-7176.11a",  0x04000, 0xf095d619, BRF_GRA },	     //  17
	
	{ "7161.1j",       0x00100, 0x02c4043f, BRF_GRA },	     //  18	PROMs
	{ "7160.1h",       0x00100, 0x225938d1, BRF_GRA },	     //  19
	{ "7159.1f",       0x00100, 0x1e0a1cd3, BRF_GRA },	     //  20
	{ "7162.5j",       0x00020, 0xcce2e29f, BRF_GRA },	     //  21
	{ "bpr.2c",        0x00020, 0x83a39201, BRF_GRA },	     //  22
};

STD_ROM_PICK(Flashgala)
STD_ROM_FN(Flashgala)

static struct BurnRomInfo GyrodineRomDesc[] = {
	{ "rom2",          0x02000, 0x85ddea38, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "a21.03",        0x02000, 0x4e9323bd, BRF_ESS | BRF_PRG }, //  1
	{ "a21.04",        0x02000, 0x57e659d4, BRF_ESS | BRF_PRG }, //  2
	{ "a21.05",        0x02000, 0x1e7293f3, BRF_ESS | BRF_PRG }, //  3
	
	{ "a21.01",        0x02000, 0xb2ce0aa2, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program 
	
	{ "a21.15",        0x01000, 0xadba18d0, BRF_GRA },	     //  5	Characters
	
	{ "a21.08",        0x02000, 0xa57df1c9, BRF_GRA },	     //  6	Tiles
	{ "a21.07",        0x02000, 0x63623ba3, BRF_GRA },	     //  7
	{ "a21.06",        0x02000, 0x4cc969a9, BRF_GRA },	     //  8
	
	{ "a21.14",        0x02000, 0x9c5c4d5b, BRF_GRA },	     //  9	Sprites
	{ "a21.13",        0x02000, 0xd36b5aad, BRF_GRA },	     //  10
	{ "a21.12",        0x02000, 0xf387aea2, BRF_GRA },	     //  11
	{ "a21.11",        0x02000, 0x87967d7d, BRF_GRA },	     //  12
	{ "a21.10",        0x02000, 0x59640ab4, BRF_GRA },	     //  13
	{ "a21.09",        0x02000, 0x22ad88d8, BRF_GRA },	     //  14

	{ "a21.16",        0x00100, 0xcc25fb56, BRF_GRA },	     //  15	PROMs
	{ "a21.17",        0x00100, 0xca054448, BRF_GRA },	     //  16
	{ "a21.18",        0x00100, 0x23c0c449, BRF_GRA },	     //  17
	{ "a21.20",        0x00020, 0xefc4985e, BRF_GRA },	     //  18
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  19
};

STD_ROM_PICK(Gyrodine)
STD_ROM_FN(Gyrodine)

static struct BurnRomInfo GyrodinetRomDesc[] = {
	{ "a21.02",        0x02000, 0xc5ec4a50, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "a21.03",        0x02000, 0x4e9323bd, BRF_ESS | BRF_PRG }, //  1
	{ "a21.04",        0x02000, 0x57e659d4, BRF_ESS | BRF_PRG }, //  2
	{ "a21.05",        0x02000, 0x1e7293f3, BRF_ESS | BRF_PRG }, //  3
	
	{ "a21.01",        0x02000, 0xb2ce0aa2, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program 
	
	{ "a21.15",        0x01000, 0xadba18d0, BRF_GRA },	     //  5	Characters
	
	{ "a21.08",        0x02000, 0xa57df1c9, BRF_GRA },	     //  6	Tiles
	{ "a21.07",        0x02000, 0x63623ba3, BRF_GRA },	     //  7
	{ "a21.06",        0x02000, 0x4cc969a9, BRF_GRA },	     //  8
	
	{ "a21.14",        0x02000, 0x9c5c4d5b, BRF_GRA },	     //  9	Sprites
	{ "a21.13",        0x02000, 0xd36b5aad, BRF_GRA },	     //  10
	{ "a21.12",        0x02000, 0xf387aea2, BRF_GRA },	     //  11
	{ "a21.11",        0x02000, 0x87967d7d, BRF_GRA },	     //  12
	{ "a21.10",        0x02000, 0x59640ab4, BRF_GRA },	     //  13
	{ "a21.09",        0x02000, 0x22ad88d8, BRF_GRA },	     //  14

	{ "a21.16",        0x00100, 0xcc25fb56, BRF_GRA },	     //  15	PROMs
	{ "a21.17",        0x00100, 0xca054448, BRF_GRA },	     //  16
	{ "a21.18",        0x00100, 0x23c0c449, BRF_GRA },	     //  17
	{ "a21.20",        0x00020, 0xefc4985e, BRF_GRA },	     //  18
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  19
};

STD_ROM_PICK(Gyrodinet)
STD_ROM_FN(Gyrodinet)

static struct BurnRomInfo BuzzardRomDesc[] = {
	{ "rom2",          0x02000, 0x85ddea38, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "a21.03",        0x02000, 0x4e9323bd, BRF_ESS | BRF_PRG }, //  1
	{ "a21.04",        0x02000, 0x57e659d4, BRF_ESS | BRF_PRG }, //  2
	{ "a21.05",        0x02000, 0x1e7293f3, BRF_ESS | BRF_PRG }, //  3
	
	{ "a21.01",        0x02000, 0xb2ce0aa2, BRF_ESS | BRF_PRG }, //  4	Z80 #2 Program 
	
	{ "buzl01.bin",    0x01000, 0x65d728d0, BRF_GRA },	     //  5	Characters
	
	{ "a21.08",        0x02000, 0xa57df1c9, BRF_GRA },	     //  6	Tiles
	{ "a21.07",        0x02000, 0x63623ba3, BRF_GRA },	     //  7
	{ "a21.06",        0x02000, 0x4cc969a9, BRF_GRA },	     //  8
	
	{ "a21.14",        0x02000, 0x9c5c4d5b, BRF_GRA },	     //  9	Sprites
	{ "a21.13",        0x02000, 0xd36b5aad, BRF_GRA },	     //  10
	{ "a21.12",        0x02000, 0xf387aea2, BRF_GRA },	     //  11
	{ "a21.11",        0x02000, 0x87967d7d, BRF_GRA },	     //  12
	{ "a21.10",        0x02000, 0x59640ab4, BRF_GRA },	     //  13
	{ "a21.09",        0x02000, 0x22ad88d8, BRF_GRA },	     //  14

	{ "a21.16",        0x00100, 0xcc25fb56, BRF_GRA },	     //  15	PROMs
	{ "a21.17",        0x00100, 0xca054448, BRF_GRA },	     //  16
	{ "a21.18",        0x00100, 0x23c0c449, BRF_GRA },	     //  17
	{ "a21.20",        0x00020, 0xefc4985e, BRF_GRA },	     //  18
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  19
};

STD_ROM_PICK(Buzzard)
STD_ROM_FN(Buzzard)

static struct BurnRomInfo LegendRomDesc[] = {
	{ "a_r2.rom",      0x04000, 0x0cc1c4f4, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "a_r3.rom",      0x04000, 0x4b270c6b, BRF_ESS | BRF_PRG }, //  1
	
	{ "a_r7.rom",      0x02000, 0xabfe5eb4, BRF_ESS | BRF_PRG }, //  2	Z80 #2 Program 
	{ "a_r8.rom",      0x02000, 0x7e7b9ba9, BRF_ESS | BRF_PRG }, //  3
	{ "a_r9.rom",      0x02000, 0x66737f1e, BRF_ESS | BRF_PRG }, //  4
	{ "a_n7.rom",      0x02000, 0x13915a53, BRF_ESS | BRF_PRG }, //  5
	
	{ "b_a4.rom",      0x01000, 0xc7dd3cf7, BRF_GRA },	     //  6	Characters
	
	{ "b_h9.rom",      0x02000, 0x1fe8644a, BRF_GRA },	     //  7	Tiles
	{ "b_h10.rom",     0x02000, 0x5f7dc82e, BRF_GRA },	     //  8
	{ "b_h11.rom",     0x02000, 0x46741643, BRF_GRA },	     //  9
	
	{ "b_a6.rom",      0x04000, 0x1689f21c, BRF_GRA },	     //  10	Sprites
	{ "b_a7.rom",      0x04000, 0xf527c909, BRF_GRA },	     //  11
	{ "b_a8.rom",      0x04000, 0x8d618629, BRF_GRA },	     //  12
	{ "b_a9.rom",      0x04000, 0x7d7e2d55, BRF_GRA },	     //  13
	{ "b_a10.rom",     0x04000, 0xf12232fe, BRF_GRA },	     //  14
	{ "b_a11.rom",     0x04000, 0x8c09243d, BRF_GRA },	     //  15
	
	{ "82s129.1j",     0x00100, 0x40590ac0, BRF_GRA },	     //  16	PROMs
	{ "82s129.1h",     0x00100, 0xe542b363, BRF_GRA },	     //  17
	{ "82s129.1f",     0x00100, 0x75536fc8, BRF_GRA },	     //  18
	{ "82s123.5j",     0x00020, 0xc98f0651, BRF_GRA },	     //  19
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  20
	
	{ "epl10p8.2j",    0x0002c, 0x8abc03bf, BRF_OPT },	     //  21	PLDs
	{ "epl12p6.9k",    0x00034, 0x9b0bd6f8, BRF_OPT },	     //  22
	{ "epl12p6.9j",    0x00034, 0xdcae870d, BRF_OPT },	     //  23
};

STD_ROM_PICK(Legend)
STD_ROM_FN(Legend)

static struct BurnRomInfo SonofphxRomDesc[] = {
	{ "5.f4",          0x02000, 0xe0d2c6cf, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "6.h4",          0x02000, 0x3a0d0336, BRF_ESS | BRF_PRG }, //  1
	{ "7.j4",          0x02000, 0x57a8e900, BRF_ESS | BRF_PRG }, //  2
	
	{ "1.f2",          0x02000, 0xc485c621, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	{ "2.h2",          0x02000, 0xb3c6a886, BRF_ESS | BRF_PRG }, //  4
	{ "3.j2",          0x02000, 0x197e314c, BRF_ESS | BRF_PRG }, //  5
	{ "4.k2",          0x02000, 0x4f3695a1, BRF_ESS | BRF_PRG }, //  6
	
	{ "14.4a",         0x01000, 0xb3859b8b, BRF_GRA },	     //  7	Characters
	
	{ "15.9h",         0x02000, 0xc9213469, BRF_GRA },	     //  8	Tiles
	{ "16.10h",        0x02000, 0x7de5d39e, BRF_GRA },	     //  9
	{ "17.11h",        0x02000, 0x0ba5f72c, BRF_GRA },	     //  10
	
	{ "8.6a",          0x04000, 0x0e9f757e, BRF_GRA },	     //  11	Sprites
	{ "9.7a",          0x04000, 0xf7d2e650, BRF_GRA },	     //  12
	{ "10.8a",         0x04000, 0xe717baf4, BRF_GRA },	     //  13
	{ "11.9a",         0x04000, 0x04b2250b, BRF_GRA },	     //  14
	{ "12.10a",        0x04000, 0xd110e140, BRF_GRA },	     //  15
	{ "13.11a",        0x04000, 0x8fdc713c, BRF_GRA },	     //  16
	
	{ "b.1j",          0x00100, 0x3ea35431, BRF_GRA },	     //  17	PROMs
	{ "g.1h",          0x00100, 0xacd7a69e, BRF_GRA },	     //  18
	{ "r.1f",          0x00100, 0xb7f48b41, BRF_GRA },	     //  19
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  20
};

STD_ROM_PICK(Sonofphx)
STD_ROM_FN(Sonofphx)

static struct BurnRomInfo RepulseRomDesc[] = {
	{ "repulse.b5",    0x02000, 0xfb2b7c9d, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "repulse.b6",    0x02000, 0x99129918, BRF_ESS | BRF_PRG }, //  1
	{ "7.j4",          0x02000, 0x57a8e900, BRF_ESS | BRF_PRG }, //  2
	
	{ "1.f2",          0x02000, 0xc485c621, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	{ "2.h2",          0x02000, 0xb3c6a886, BRF_ESS | BRF_PRG }, //  4
	{ "3.j2",          0x02000, 0x197e314c, BRF_ESS | BRF_PRG }, //  5
	{ "repulse.b4",    0x02000, 0x86b267f3, BRF_ESS | BRF_PRG }, //  6
	
	{ "repulse.a11",   0x01000, 0x8e1de90a, BRF_GRA },	     //  7	Characters
	
	{ "15.9h",         0x02000, 0xc9213469, BRF_GRA },	     //  8	Tiles
	{ "16.10h",        0x02000, 0x7de5d39e, BRF_GRA },	     //  9
	{ "17.11h",        0x02000, 0x0ba5f72c, BRF_GRA },	     //  10
	
	{ "8.6a",          0x04000, 0x0e9f757e, BRF_GRA },	     //  11	Sprites
	{ "9.7a",          0x04000, 0xf7d2e650, BRF_GRA },	     //  12
	{ "10.8a",         0x04000, 0xe717baf4, BRF_GRA },	     //  13
	{ "11.9a",         0x04000, 0x04b2250b, BRF_GRA },	     //  14
	{ "12.10a",        0x04000, 0xd110e140, BRF_GRA },	     //  15
	{ "13.11a",        0x04000, 0x8fdc713c, BRF_GRA },	     //  16
	
	{ "b.1j",          0x00100, 0x3ea35431, BRF_GRA },	     //  17	PROMs
	{ "g.1h",          0x00100, 0xacd7a69e, BRF_GRA },	     //  18
	{ "r.1f",          0x00100, 0xb7f48b41, BRF_GRA },	     //  19
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  20
};

STD_ROM_PICK(Repulse)
STD_ROM_FN(Repulse)

static struct BurnRomInfo Lstwar99RomDesc[] = {
	{ "1999.4f",       0x02000, 0xe3cfc09f, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "1999.4h",       0x02000, 0xfd58c6e1, BRF_ESS | BRF_PRG }, //  1
	{ "7.j4",          0x02000, 0x57a8e900, BRF_ESS | BRF_PRG }, //  2
	
	{ "1.f2",          0x02000, 0xc485c621, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	{ "2.h2",          0x02000, 0xb3c6a886, BRF_ESS | BRF_PRG }, //  4
	{ "3.j2",          0x02000, 0x197e314c, BRF_ESS | BRF_PRG }, //  5
	{ "repulse.b4",    0x02000, 0x86b267f3, BRF_ESS | BRF_PRG }, //  6
	
	{ "1999.4a",       0x01000, 0x49a2383e, BRF_GRA },	     //  7	Characters
	
	{ "15.9h",         0x02000, 0xc9213469, BRF_GRA },	     //  8	Tiles
	{ "16.10h",        0x02000, 0x7de5d39e, BRF_GRA },	     //  9
	{ "17.11h",        0x02000, 0x0ba5f72c, BRF_GRA },	     //  10
	
	{ "8.6a",          0x04000, 0x0e9f757e, BRF_GRA },	     //  11	Sprites
	{ "9.7a",          0x04000, 0xf7d2e650, BRF_GRA },	     //  12
	{ "10.8a",         0x04000, 0xe717baf4, BRF_GRA },	     //  13
	{ "11.9a",         0x04000, 0x04b2250b, BRF_GRA },	     //  14
	{ "12.10a",        0x04000, 0xd110e140, BRF_GRA },	     //  15
	{ "13.11a",        0x04000, 0x8fdc713c, BRF_GRA },	     //  16
	
	{ "b.1j",          0x00100, 0x3ea35431, BRF_GRA },	     //  17	PROMs
	{ "g.1h",          0x00100, 0xacd7a69e, BRF_GRA },	     //  18
	{ "r.1f",          0x00100, 0xb7f48b41, BRF_GRA },	     //  19
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  20
};

STD_ROM_PICK(Lstwar99)
STD_ROM_FN(Lstwar99)

static struct BurnRomInfo Lstwra99RomDesc[] = {
	{ "4f.bin",        0x02000, 0xefe2908d, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "4h.bin",        0x02000, 0x5b79c342, BRF_ESS | BRF_PRG }, //  1
	{ "4j.bin",        0x02000, 0xd2a62c1b, BRF_ESS | BRF_PRG }, //  2
	
	{ "2f.bin",        0x02000, 0xcb9d8291, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	{ "2h.bin",        0x02000, 0x24dbddc3, BRF_ESS | BRF_PRG }, //  4
	{ "2j.bin",        0x02000, 0x16879c4c, BRF_ESS | BRF_PRG }, //  5
	{ "repulse.b4",    0x02000, 0x86b267f3, BRF_ESS | BRF_PRG }, //  6
	
	{ "1999.4a",       0x01000, 0x49a2383e, BRF_GRA },	     //  7	Characters
	
	{ "9h.bin",        0x02000, 0x59993c27, BRF_GRA },	     //  8	Tiles
	{ "10h.bin",       0x02000, 0xdfbf0280, BRF_GRA },	     //  9
	{ "11h.bin",       0x02000, 0xe4f29fc0, BRF_GRA },	     //  10
	
	{ "6a.bin",        0x04000, 0x98d44410, BRF_GRA },	     //  11	Sprites
	{ "7a.bin",        0x04000, 0x4c54d281, BRF_GRA },	     //  12
	{ "8a.bin",        0x04000, 0x81018101, BRF_GRA },	     //  13
	{ "9a.bin",        0x04000, 0x347b91fd, BRF_GRA },	     //  14
	{ "10a.bin",       0x04000, 0xf07de4fa, BRF_GRA },	     //  15
	{ "11a.bin",       0x04000, 0x34a04f48, BRF_GRA },	     //  16
	
	{ "b.1j",          0x00100, 0x3ea35431, BRF_GRA },	     //  17	PROMs
	{ "g.1h",          0x00100, 0xacd7a69e, BRF_GRA },	     //  18
	{ "r.1f",          0x00100, 0xb7f48b41, BRF_GRA },	     //  19
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  20
};

STD_ROM_PICK(Lstwra99)
STD_ROM_FN(Lstwra99)

static struct BurnRomInfo Lstwrk99RomDesc[] = {
	{ "1999.4f",       0x02000, 0xe3cfc09f, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "1999.4h",       0x02000, 0xfd58c6e1, BRF_ESS | BRF_PRG }, //  1
	{ "7.j4",          0x02000, 0x57a8e900, BRF_ESS | BRF_PRG }, //  2
	
	{ "1.f2",          0x02000, 0xc485c621, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	{ "2.h2",          0x02000, 0xb3c6a886, BRF_ESS | BRF_PRG }, //  4
	{ "3.j2",          0x02000, 0x197e314c, BRF_ESS | BRF_PRG }, //  5
	{ "repulse.b4",    0x02000, 0x86b267f3, BRF_ESS | BRF_PRG }, //  6
	
	{ "1999-14.rom",   0x01000, 0xb4995072, BRF_GRA },	     //  7	Characters
	
	{ "15.9h",         0x02000, 0xc9213469, BRF_GRA },	     //  8	Tiles
	{ "16.10h",        0x02000, 0x7de5d39e, BRF_GRA },	     //  9
	{ "17.11h",        0x02000, 0x0ba5f72c, BRF_GRA },	     //  10
	
	{ "8.6a",          0x04000, 0x0e9f757e, BRF_GRA },	     //  11	Sprites
	{ "9.7a",          0x04000, 0xf7d2e650, BRF_GRA },	     //  12
	{ "10.8a",         0x04000, 0xe717baf4, BRF_GRA },	     //  13
	{ "11.9a",         0x04000, 0x04b2250b, BRF_GRA },	     //  14
	{ "12.10a",        0x04000, 0xd110e140, BRF_GRA },	     //  15
	{ "13.11a",        0x04000, 0x8fdc713c, BRF_GRA },	     //  16
	
	{ "b.1j",          0x00100, 0x3ea35431, BRF_GRA },	     //  17	PROMs
	{ "g.1h",          0x00100, 0xacd7a69e, BRF_GRA },	     //  18
	{ "r.1f",          0x00100, 0xb7f48b41, BRF_GRA },	     //  19
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  20
	{ "1999-00.rom",   0x00800, 0x0c0c449f, BRF_GRA },	     //  21
};

STD_ROM_PICK(Lstwrk99)
STD_ROM_FN(Lstwrk99)

static struct BurnRomInfo SrdmissnRomDesc[] = {
	{ "5.t2",          0x04000, 0xa682b48c, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "7.t3",          0x04000, 0x1719c58c, BRF_ESS | BRF_PRG }, //  1
	
	{ "1.t7",          0x04000, 0xdc48595e, BRF_ESS | BRF_PRG }, //  2	Z80 #2 Program 
	{ "3.t8",          0x04000, 0x216be1e8, BRF_ESS | BRF_PRG }, //  3
	
	{ "15.4a",         0x01000, 0x4961f7fd, BRF_GRA },	     //  4	Characters
	
	{ "17.9h",         0x02000, 0x41211458, BRF_GRA },	     //  5	Tiles
	{ "18.10h",        0x02000, 0x740eccd4, BRF_GRA },	     //  6
	{ "16.11h",        0x02000, 0xc1f4a5db, BRF_GRA },	     //  7
	
	{ "14.6a",         0x04000, 0x3d4c0447, BRF_GRA },	     //  8	Sprites
	{ "13.7a",         0x04000, 0x22414a67, BRF_GRA },	     //  9
	{ "12.8a",         0x04000, 0x61e34283, BRF_GRA },	     //  10
	{ "11.9a",         0x04000, 0xbbbaffef, BRF_GRA },	     //  11
	{ "10.10a",        0x04000, 0xde564f97, BRF_GRA },	     //  12
	{ "9.11a",         0x04000, 0x890dc815, BRF_GRA },	     //  13
	
	{ "mr.1j",         0x00100, 0x110a436e, BRF_GRA },	     //  14	PROMs
	{ "mg.1h",         0x00100, 0x0fbfd9f0, BRF_GRA },	     //  15
	{ "mb.1f",         0x00100, 0xa342890c, BRF_GRA },	     //  16
	{ "m2.5j",         0x00020, 0x190a55ad, BRF_GRA },	     //  17
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  18
};

STD_ROM_PICK(Srdmissn)
STD_ROM_FN(Srdmissn)

static struct BurnRomInfo FxRomDesc[] = {
	{ "fx.01",         0x04000, 0xb651754b, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "fx.02",         0x04000, 0xf3d2dcc1, BRF_ESS | BRF_PRG }, //  1
	
	{ "fx.03",         0x04000, 0x8907df6b, BRF_ESS | BRF_PRG }, //  2	Z80 #2 Program 
	{ "fx.04",         0x04000, 0xc665834f, BRF_ESS | BRF_PRG }, //  3
	
	{ "fx.05",         0x01000, 0x4a504286, BRF_GRA },	     //  4	Characters
	
	{ "17.9h",         0x02000, 0x41211458, BRF_GRA },	     //  5	Tiles
	{ "18.10h",        0x02000, 0x740eccd4, BRF_GRA },	     //  6
	{ "16.11h",        0x02000, 0xc1f4a5db, BRF_GRA },	     //  7
	
	{ "14.6a",         0x04000, 0x3d4c0447, BRF_GRA },	     //  8	Sprites
	{ "13.7a",         0x04000, 0x22414a67, BRF_GRA },	     //  9
	{ "12.8a",         0x04000, 0x61e34283, BRF_GRA },	     //  10
	{ "11.9a",         0x04000, 0xbbbaffef, BRF_GRA },	     //  11
	{ "10.10a",        0x04000, 0xde564f97, BRF_GRA },	     //  12
	{ "9.11a",         0x04000, 0x890dc815, BRF_GRA },	     //  13
	
	{ "mr.1j",         0x00100, 0x110a436e, BRF_GRA },	     //  14	PROMs
	{ "mg.1h",         0x00100, 0x0fbfd9f0, BRF_GRA },	     //  15
	{ "mb.1f",         0x00100, 0xa342890c, BRF_GRA },	     //  16
	{ "m2.5j",         0x00020, 0x190a55ad, BRF_GRA },	     //  17
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  18
};

STD_ROM_PICK(Fx)
STD_ROM_FN(Fx)

// f205v id 989
// same data as fx, different format
static struct BurnRomInfo FxaRomDesc[] = {
	{ "fxa1.t4",       0x08000, 0xa71332aa, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	
	{ "fxa2.t8",       0x08000, 0xeb299381, BRF_ESS | BRF_PRG }, //  1	Z80 #2 Program 
	
	{ "fx.05",         0x01000, 0x4a504286, BRF_GRA },	     //  2	Characters
	
	{ "17.9h",         0x02000, 0x41211458, BRF_GRA },	     //  3	Tiles
	{ "18.10h",        0x02000, 0x740eccd4, BRF_GRA },	     //  4
	{ "16.11h",        0x02000, 0xc1f4a5db, BRF_GRA },	     //  5
	
	{ "fxa5.7a",       0x08000, 0x3e2289dc, BRF_GRA },	     //  6	Sprites	
	{ "fxa4.9a",       0x08000, 0x26963d7f, BRF_GRA },	     //  7
	{ "fxa3.11a",      0x08000, 0x8687f1a0, BRF_GRA },	     //  8
	
	{ "mr.1j",         0x00100, 0x110a436e, BRF_GRA },	     //  9	PROMs
	{ "mg.1h",         0x00100, 0x0fbfd9f0, BRF_GRA },	     //  10
	{ "mb.1f",         0x00100, 0xa342890c, BRF_GRA },	     //  11
	{ "m2.5j",         0x00020, 0x190a55ad, BRF_GRA },	     //  12
	{ "m1.2c",         0x00020, 0x83a39201, BRF_GRA },	     //  13
};

STD_ROM_PICK(Fxa)
STD_ROM_FN(Fxa)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	KyugoZ80Rom1             = Next; Next += 0x08000;
	KyugoZ80Rom2             = Next; Next += 0x08000;
	KyugoPromRed             = Next; Next += 0x00100;
	KyugoPromGreen           = Next; Next += 0x00100;
	KyugoPromBlue            = Next; Next += 0x00100;
	KyugoPromCharLookup      = Next; Next += 0x00020;

	RamStart               = Next;

	KyugoSharedZ80Ram        = Next; Next += 0x00800;
	KyugoZ80Ram2             = Next; Next += 0x00800;
	KyugoSprite1Ram          = Next; Next += 0x00800;
	KyugoSprite2Ram          = Next; Next += 0x00800;
	KyugoFgVideoRam          = Next; Next += 0x00800;
	KyugoBgVideoRam          = Next; Next += 0x00800;
	KyugoBgAttrRam           = Next; Next += 0x00800;

	RamEnd                 = Next;

	KyugoChars               = Next; Next += 0x100 * 8 * 8;
	KyugoTiles               = Next; Next += 0x400 * 8 * 8;
	KyugoSprites             = Next; Next += 0x400 * 16 * 16;
	pFMBuffer              = (INT16*)Next; Next += nBurnSoundLen * 6 * sizeof(INT16);
	KyugoPalette             = (UINT32*)Next; Next += 256 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 KyugoDoReset()
{
	for (INT32 i = 0; i < 2; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}
	
	for (INT32 i = 0; i < 2; i++) {
		AY8910Reset(i);
	}
	
	KyugoIRQEnable = 0;
	KyugoSubCPUEnable = 0;
	KyugoFgColour = 0;
	KyugoBgPaletteBank = 0;
	KyugoBgScrollXHi = 0;
	KyugoBgScrollXLo = 0;
	KyugoBgScrollY = 0;
	KyugoFlipScreen = 0;

	return 0;
}

UINT8 __fastcall KyugoRead1(UINT16 a)
{
	if (a >= 0x9800 && a <= 0x9fff) {
		return KyugoSprite2Ram[a - 0x9800] | 0xf0;
	}
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall KyugoWrite1(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xa800: {
			KyugoBgScrollXLo = d;
			return;
		}
		
		case 0xb000: {
			KyugoBgScrollXHi = d & 1;
			KyugoFgColour = (d & 0x20) >> 5;
			KyugoBgPaletteBank = (d & 0x40) >> 6;
			return;
		}
		
		case 0xb800: {
			KyugoBgScrollY = d;
			return;
		}
		
		
		case 0xe000: {
			// watchdog write
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

void __fastcall FlashgalPortWrite1(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x40: {
			KyugoIRQEnable = d & 1;
			return;
		}
		
		case 0x41: {
			KyugoFlipScreen = d & 1;
			return;
		}
		
		case 0x42: {
			KyugoSubCPUEnable = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void __fastcall FlashgalaPortWrite1(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0xc0: {
			KyugoIRQEnable = d & 1;
			return;
		}
		
		case 0xc1: {
			KyugoFlipScreen = d & 1;
			return;
		}
		
		case 0xc2: {
			KyugoSubCPUEnable = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void __fastcall GyrodinePortWrite1(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			KyugoIRQEnable = d & 1;
			return;
		}
		
		case 0x01: {
			KyugoFlipScreen = d & 1;
			return;
		}
		
		case 0x02: {
			KyugoSubCPUEnable = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void __fastcall SrdmissnPortWrite1(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x08: {
			KyugoIRQEnable = d & 1;
			return;
		}
		
		case 0x09: {
			KyugoFlipScreen = d & 1;
			return;
		}
		
		case 0x0a: {
			KyugoSubCPUEnable = d & 1;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall FlashgalRead2(UINT16 a)
{
	switch (a) {
		case 0xc000: {
			return KyugoInput[2];
		}
		
		case 0xc040: {
			return KyugoInput[1];
		}
	
		case 0xc080: {
			return KyugoInput[0];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

UINT8 __fastcall FlashgalaRead2(UINT16 a)
{
	switch (a) {
		case 0xc040: {
			return KyugoInput[0];
		}
		
		case 0xc080: {
			return KyugoInput[1];
		}
			
		case 0xc0c0: {
			return KyugoInput[2];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

UINT8 __fastcall GyrodineRead2(UINT16 a)
{
	switch (a) {
		case 0x8000: {
			return KyugoInput[2];
		}
		
		case 0x8040: {
			return KyugoInput[1];
		}
	
		case 0x8080: {
			return KyugoInput[0];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

UINT8 __fastcall SrdmissnRead2(UINT16 a)
{
	switch (a) {
		case 0xf400: {
			return KyugoInput[0];
		}
		
		case 0xf401: {
			return KyugoInput[1];
		}
			
		case 0xf402: {
			return KyugoInput[2];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

UINT8 __fastcall LegendRead2(UINT16 a)
{
	switch (a) {
		case 0xf800: {
			return KyugoInput[0];
		}
		
		case 0xf801: {
			return KyugoInput[1];
		}
			
		case 0xf802: {
			return KyugoInput[2];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall KyugoWrite2(UINT16 a, UINT8 d)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall KyugoPortRead2(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x02: {
			return AY8910Read(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

UINT8 __fastcall FlashgalaPortRead2(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x42: {
			return AY8910Read(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

UINT8 __fastcall SrdmissnPortRead2(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x82: {
			return AY8910Read(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall FlashgalPortWrite2(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x01: {
			AY8910Write(0, 1, d);
			return;
		}
		
		case 0x40: {
			AY8910Write(1, 0, d);
			return;
		}
		
		case 0x41: {
			AY8910Write(1, 1, d);
			return;
		}
				
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void __fastcall FlashgalaPortWrite2(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x40: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x41: {
			AY8910Write(0, 1, d);
			return;
		}
		
		case 0x80: {
			AY8910Write(1, 0, d);
			return;
		}
		
		case 0x81: {
			AY8910Write(1, 1, d);
			return;
		}
				
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void __fastcall GyrodinePortWrite2(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x01: {
			AY8910Write(0, 1, d);
			return;
		}
		
		case 0xc0: {
			AY8910Write(1, 0, d);
			return;
		}
		
		case 0xc1: {
			AY8910Write(1, 1, d);
			return;
		}
				
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void __fastcall SrdmissnPortWrite2(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x80: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x81: {
			AY8910Write(0, 1, d);
			return;
		}
		
		case 0x84: {
			AY8910Write(1, 0, d);
			return;
		}
		
		case 0x85: {
			AY8910Write(1, 1, d);
			return;
		}
				
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

static INT32 CharPlaneOffsets[2]   = { 0, 4 };
static INT32 CharXOffsets[8]       = { 0, 1, 2, 3, 64, 65, 66, 67 };
static INT32 CharYOffsets[8]       = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 TilePlaneOffsets[3]   = { 0, 0x10000, 0x20000 };
static INT32 TileXOffsets[8]       = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 TileYOffsets[8]       = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 SpritePlaneOffsets[3] = { 0, 0x40000, 0x80000 };
static INT32 SpriteXOffsets[16]    = { 0, 1, 2, 3, 4, 5, 6, 7, 64, 65, 66, 67, 68, 69, 70, 71 };
static INT32 SpriteYOffsets[16]    = { 0, 8, 16, 24, 32, 40, 48, 56, 128, 136, 144, 152, 160, 168, 176, 184 };

static UINT8 KyugoDip0Read(UINT32 /*a*/)
{
	return KyugoDip[0];
}

static UINT8 KyugoDip1Read(UINT32 /*a*/)
{
	return KyugoDip[1];
}

static INT32 KyugoInit()
{
	INT32 nRet = 0, nLen, i;
	
	KyugoNumZ80Rom1 = 4;
	KyugoNumZ80Rom2 = 4;
	KyugoNumSpriteRom = 6;
	KyugoSizeZ80Rom1 = 0x2000;
	KyugoSizeZ80Rom2 = 0x2000;
	KyugoSizeSpriteRom = 0x4000;
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "airwolf") || !strcmp(BurnDrvGetTextA(DRV_NAME), "airwolfa")) {
		 KyugoNumZ80Rom1 = 1;
		 KyugoNumZ80Rom2 = 1;
		 KyugoNumSpriteRom = 3;
		 KyugoSizeZ80Rom1 = 0x8000;
		 KyugoSizeZ80Rom2 = 0x8000;
		 KyugoSizeSpriteRom = 0x8000;
	}
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "gyrodine") || !strcmp(BurnDrvGetTextA(DRV_NAME), "gyrodinet") || !strcmp(BurnDrvGetTextA(DRV_NAME), "buzzard")) {
		KyugoNumZ80Rom2 = 1;
	}
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "legend")) {
		 KyugoNumZ80Rom1 = 2;
		 KyugoNumZ80Rom2 = 4;
		 KyugoSizeZ80Rom1 = 0x4000;
		 KyugoSizeZ80Rom2 = 0x2000;
	}
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "sonofphx") || !strcmp(BurnDrvGetTextA(DRV_NAME), "repulse") || !strcmp(BurnDrvGetTextA(DRV_NAME), "99lstwar") || !strcmp(BurnDrvGetTextA(DRV_NAME), "99lstwara") || !strcmp(BurnDrvGetTextA(DRV_NAME), "99lstwark")) {
		KyugoNumZ80Rom1 = 3;
	}
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "skywolf") || !strcmp(BurnDrvGetTextA(DRV_NAME), "srdmissn") || !strcmp(BurnDrvGetTextA(DRV_NAME), "fx")) {
		 KyugoNumZ80Rom1 = 2;
		 KyugoNumZ80Rom2 = 2;
		 KyugoSizeZ80Rom1 = 0x4000;
		 KyugoSizeZ80Rom2 = 0x4000;
	}
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "skywolf2")) {
	 	 KyugoNumZ80Rom1 = 1;
		 KyugoNumZ80Rom2 = 2;
		 KyugoSizeZ80Rom1 = 0x8000;
		 KyugoSizeZ80Rom2 = 0x4000;
	}
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "fxa")) {
		KyugoNumZ80Rom1 = 1;
		KyugoNumZ80Rom2 = 1;
		KyugoNumSpriteRom = 3;
		KyugoSizeZ80Rom1 = 0x8000;
		KyugoSizeZ80Rom2 = 0x8000;
		KyugoSizeSpriteRom = 0x8000;
	}

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	KyugoTempRom = (UINT8 *)BurnMalloc(0x18000);

	// Load Z80 #1 Program Roms
	for (i = 0; i < KyugoNumZ80Rom1; i++) {
		nRet = BurnLoadRom(KyugoZ80Rom1 + (KyugoSizeZ80Rom1 * i), i, 1); if (nRet != 0) return 1;
	}
	
	// Load Z80 #2 Program Roms
	for (i = KyugoNumZ80Rom1; i < KyugoNumZ80Rom2 + KyugoNumZ80Rom1; i++) {
		nRet = BurnLoadRom(KyugoZ80Rom2 + (KyugoSizeZ80Rom2 * (i - KyugoNumZ80Rom1)), i, 1); if (nRet != 0) return 1;
	}
	
	// Load and decode the chars
	nRet = BurnLoadRom(KyugoTempRom, KyugoNumZ80Rom2 + KyugoNumZ80Rom1 + 0, 1); if (nRet != 0) return 1;
	GfxDecode(0x100, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, KyugoTempRom, KyugoChars);
	
	// Load and decode the tiles
	memset(KyugoTempRom, 0, 0x18000);
	nRet = BurnLoadRom(KyugoTempRom + 0x00000,  KyugoNumZ80Rom2 + KyugoNumZ80Rom1 + 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(KyugoTempRom + 0x02000,  KyugoNumZ80Rom2 + KyugoNumZ80Rom1 + 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(KyugoTempRom + 0x04000,  KyugoNumZ80Rom2 + KyugoNumZ80Rom1 + 3, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 3, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, KyugoTempRom, KyugoTiles);
	
	// Load and decode the sprites
	memset(KyugoTempRom, 0, 0x18000);
	for (i = KyugoNumZ80Rom2 + KyugoNumZ80Rom1 + 4; i < KyugoNumSpriteRom + KyugoNumZ80Rom2 + KyugoNumZ80Rom1 + 4; i++) {
		nRet = BurnLoadRom(KyugoTempRom + (KyugoSizeSpriteRom * (i - KyugoNumZ80Rom2 - KyugoNumZ80Rom1 - 4)),  i, 1); if (nRet != 0) return 1;
	}
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "airwolf") || !strcmp(BurnDrvGetTextA(DRV_NAME), "airwolfa")) {
		UINT8 *Temp = (UINT8*)BurnMalloc(0x18000);
		memcpy(Temp, KyugoTempRom, 0x18000);
		memcpy(KyugoTempRom + 0x00000, Temp + 0x00000, 0x2000);
		memcpy(KyugoTempRom + 0x04000, Temp + 0x02000, 0x2000);
		memcpy(KyugoTempRom + 0x02000, Temp + 0x04000, 0x2000);
		memcpy(KyugoTempRom + 0x06000, Temp + 0x06000, 0x2000);
		memcpy(KyugoTempRom + 0x08000, Temp + 0x08000, 0x2000);
		memcpy(KyugoTempRom + 0x0c000, Temp + 0x0a000, 0x2000);
		memcpy(KyugoTempRom + 0x0a000, Temp + 0x0c000, 0x2000);
		memcpy(KyugoTempRom + 0x0e000, Temp + 0x0e000, 0x2000);
		memcpy(KyugoTempRom + 0x10000, Temp + 0x10000, 0x2000);
		memcpy(KyugoTempRom + 0x14000, Temp + 0x12000, 0x2000);
		memcpy(KyugoTempRom + 0x12000, Temp + 0x14000, 0x2000);
		memcpy(KyugoTempRom + 0x16000, Temp + 0x16000, 0x2000);
		BurnFree(Temp);
	}
	GfxDecode(0x400, 3, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, KyugoTempRom, KyugoSprites);
	
	// Load the PROMs
	nRet = BurnLoadRom(KyugoPromRed,          KyugoNumSpriteRom + KyugoNumZ80Rom2 + KyugoNumZ80Rom1 + 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(KyugoPromGreen,        KyugoNumSpriteRom + KyugoNumZ80Rom2 + KyugoNumZ80Rom1 + 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(KyugoPromBlue,         KyugoNumSpriteRom + KyugoNumZ80Rom2 + KyugoNumZ80Rom1 + 6, 1); if (nRet != 0) return 1;
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "flashgal") || !strcmp(BurnDrvGetTextA(DRV_NAME), "flashgala") || !strcmp(BurnDrvGetTextA(DRV_NAME), "gyrodine") || !strcmp(BurnDrvGetTextA(DRV_NAME), "gyrodinet") || !strcmp(BurnDrvGetTextA(DRV_NAME), "buzzard") || !strcmp(BurnDrvGetTextA(DRV_NAME), "legend") || !strcmp(BurnDrvGetTextA(DRV_NAME), "srdmissn") || !strcmp(BurnDrvGetTextA(DRV_NAME), "fx") || !strcmp(BurnDrvGetTextA(DRV_NAME), "fxa")) {
		nRet = BurnLoadRom(KyugoPromCharLookup,   KyugoNumSpriteRom + KyugoNumZ80Rom2 + KyugoNumZ80Rom1 + 7, 1); if (nRet != 0) return 1;
	}
	
	BurnFree(KyugoTempRom);
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(KyugoRead1);
	ZetSetWriteHandler(KyugoWrite1);
	ZetMapArea(0x0000, 0x7fff, 0, KyugoZ80Rom1             );
	ZetMapArea(0x0000, 0x7fff, 2, KyugoZ80Rom1             );
	ZetMapArea(0x8000, 0x87ff, 0, KyugoBgVideoRam          );
	ZetMapArea(0x8000, 0x87ff, 1, KyugoBgVideoRam          );
	ZetMapArea(0x8000, 0x87ff, 2, KyugoBgVideoRam          );
	ZetMapArea(0x8800, 0x8fff, 0, KyugoBgAttrRam           );
	ZetMapArea(0x8800, 0x8fff, 1, KyugoBgAttrRam           );
	ZetMapArea(0x8800, 0x8fff, 2, KyugoBgAttrRam           );
	ZetMapArea(0x9000, 0x97ff, 0, KyugoFgVideoRam          );
	ZetMapArea(0x9000, 0x97ff, 1, KyugoFgVideoRam          );
	ZetMapArea(0x9000, 0x97ff, 2, KyugoFgVideoRam          );
	ZetMapArea(0x9800, 0x9fff, 1, KyugoSprite2Ram          );
	ZetMapArea(0x9800, 0x9fff, 2, KyugoSprite2Ram          );
	ZetMapArea(0xa000, 0xa7ff, 0, KyugoSprite1Ram          );
	ZetMapArea(0xa000, 0xa7ff, 1, KyugoSprite1Ram          );
	ZetMapArea(0xa000, 0xa7ff, 2, KyugoSprite1Ram          );
	ZetMapArea(0xf000, 0xf7ff, 0, KyugoSharedZ80Ram        );
	ZetMapArea(0xf000, 0xf7ff, 1, KyugoSharedZ80Ram        );
	ZetMapArea(0xf000, 0xf7ff, 2, KyugoSharedZ80Ram        );
	ZetMemEnd();
	ZetClose();
	
	ZetInit(1);
	ZetOpen(1);
	ZetSetWriteHandler(KyugoWrite2);
	ZetSetInHandler(KyugoPortRead2);
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "gyrodine") || !strcmp(BurnDrvGetTextA(DRV_NAME), "gyrodinet") || !strcmp(BurnDrvGetTextA(DRV_NAME), "buzzard")) {
		ZetMapArea(0x0000, 0x1fff, 0, KyugoZ80Rom2             );
		ZetMapArea(0x0000, 0x1fff, 2, KyugoZ80Rom2             );
	} else {
		ZetMapArea(0x0000, 0x7fff, 0, KyugoZ80Rom2             );
		ZetMapArea(0x0000, 0x7fff, 2, KyugoZ80Rom2             );
	}
	ZetMemEnd();
	ZetClose();
	
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "airwolf") || !strcmp(BurnDrvGetTextA(DRV_NAME), "airwolfa") || !strcmp(BurnDrvGetTextA(DRV_NAME), "skywolf") || !strcmp(BurnDrvGetTextA(DRV_NAME), "skywolf2")) {
		ZetOpen(0);
		ZetSetOutHandler(SrdmissnPortWrite1);
		ZetMapArea(0xe000, 0xe7ff, 0, KyugoSharedZ80Ram        );
		ZetMapArea(0xe000, 0xe7ff, 1, KyugoSharedZ80Ram        );
		ZetMapArea(0xe000, 0xe7ff, 2, KyugoSharedZ80Ram        );
		ZetClose();
		
		ZetOpen(1);
		ZetSetReadHandler(SrdmissnRead2);
		ZetSetInHandler(SrdmissnPortRead2);
		ZetSetOutHandler(SrdmissnPortWrite2);
		ZetMapArea(0x8000, 0x87ff, 0, KyugoSharedZ80Ram        );
		ZetMapArea(0x8000, 0x87ff, 1, KyugoSharedZ80Ram        );
		ZetMapArea(0x8000, 0x87ff, 2, KyugoSharedZ80Ram        );
		ZetClose();
	}
	
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "flashgal")) {
		ZetOpen(0);
		ZetSetOutHandler(FlashgalPortWrite1);
		ZetClose();
		
		ZetOpen(1);
		ZetSetReadHandler(FlashgalRead2);
		ZetSetOutHandler(FlashgalPortWrite2);
		ZetMapArea(0xa000, 0xa7ff, 0, KyugoSharedZ80Ram        );
		ZetMapArea(0xa000, 0xa7ff, 1, KyugoSharedZ80Ram        );
		ZetMapArea(0xa000, 0xa7ff, 2, KyugoSharedZ80Ram        );
		ZetClose();
	}
	
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "flashgala")) {
		ZetOpen(0);
		ZetSetOutHandler(FlashgalaPortWrite1);
		ZetClose();
		
		ZetOpen(1);
		ZetSetReadHandler(FlashgalaRead2);
		ZetSetInHandler(FlashgalaPortRead2);
		ZetSetOutHandler(FlashgalaPortWrite2);
		ZetMapArea(0xe000, 0xe7ff, 0, KyugoSharedZ80Ram        );
		ZetMapArea(0xe000, 0xe7ff, 1, KyugoSharedZ80Ram        );
		ZetMapArea(0xe000, 0xe7ff, 2, KyugoSharedZ80Ram        );
		ZetClose();
	}
	
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "gyrodine") || !strcmp(BurnDrvGetTextA(DRV_NAME), "gyrodinet") || !strcmp(BurnDrvGetTextA(DRV_NAME), "buzzard")) {
		ZetOpen(0);
		ZetSetOutHandler(GyrodinePortWrite1);
		ZetClose();
		
		ZetOpen(1);
		ZetSetReadHandler(GyrodineRead2);
		ZetSetOutHandler(GyrodinePortWrite2);
		ZetMapArea(0x4000, 0x47ff, 0, KyugoSharedZ80Ram        );
		ZetMapArea(0x4000, 0x47ff, 1, KyugoSharedZ80Ram        );
		ZetMapArea(0x4000, 0x47ff, 2, KyugoSharedZ80Ram        );
		ZetClose();
	}
	
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "legend")) {
		ZetOpen(0);
		ZetSetOutHandler(GyrodinePortWrite1);
		ZetClose();
		
		ZetOpen(1);
		ZetSetReadHandler(LegendRead2);
		ZetSetInHandler(SrdmissnPortRead2);
		ZetSetOutHandler(SrdmissnPortWrite2);
		ZetMapArea(0xc000, 0xc7ff, 0, KyugoSharedZ80Ram        );
		ZetMapArea(0xc000, 0xc7ff, 1, KyugoSharedZ80Ram        );
		ZetMapArea(0xc000, 0xc7ff, 2, KyugoSharedZ80Ram        );
		ZetClose();
	}
	
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "sonofphx") || !strcmp(BurnDrvGetTextA(DRV_NAME), "repulse") || !strcmp(BurnDrvGetTextA(DRV_NAME), "99lstwar") || !strcmp(BurnDrvGetTextA(DRV_NAME), "99lstwara") || !strcmp(BurnDrvGetTextA(DRV_NAME), "99lstwark")) {
		ZetOpen(0);
		ZetSetOutHandler(GyrodinePortWrite1);
		ZetClose();
		
		ZetOpen(1);
		ZetSetReadHandler(FlashgalRead2);
		ZetSetOutHandler(FlashgalPortWrite2);
		ZetMapArea(0xa000, 0xa7ff, 0, KyugoSharedZ80Ram        );
		ZetMapArea(0xa000, 0xa7ff, 1, KyugoSharedZ80Ram        );
		ZetMapArea(0xa000, 0xa7ff, 2, KyugoSharedZ80Ram        );
		ZetClose();
	}
	
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "srdmissn") || !strcmp(BurnDrvGetTextA(DRV_NAME), "fx") || !strcmp(BurnDrvGetTextA(DRV_NAME), "fxa")) {
		ZetOpen(0);
		ZetSetOutHandler(SrdmissnPortWrite1);
		ZetMapArea(0xe000, 0xe7ff, 0, KyugoSharedZ80Ram        );
		ZetMapArea(0xe000, 0xe7ff, 1, KyugoSharedZ80Ram        );
		ZetMapArea(0xe000, 0xe7ff, 2, KyugoSharedZ80Ram        );
		ZetClose();
		
		ZetOpen(1);
		ZetSetReadHandler(SrdmissnRead2);
		ZetSetInHandler(SrdmissnPortRead2);
		ZetSetOutHandler(SrdmissnPortWrite2);
		ZetMapArea(0x8000, 0x87ff, 0, KyugoSharedZ80Ram        );
		ZetMapArea(0x8000, 0x87ff, 1, KyugoSharedZ80Ram        );
		ZetMapArea(0x8000, 0x87ff, 2, KyugoSharedZ80Ram        );
		ZetMapArea(0x8800, 0x8fff, 0, KyugoZ80Ram2             );
		ZetMapArea(0x8800, 0x8fff, 1, KyugoZ80Ram2             );
		ZetMapArea(0x8800, 0x8fff, 2, KyugoZ80Ram2             );
		ZetClose();
	}
	
	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;

	AY8910Init(0, 18432000 / 12, nBurnSoundRate, &KyugoDip0Read, &KyugoDip1Read, NULL, NULL);
	AY8910Init(1, 18432000 / 12, nBurnSoundRate, NULL, NULL, NULL, NULL);

	GenericTilesInit();

	// Reset the driver
	KyugoDoReset();

	return 0;
}

static INT32 Skywolf3Init()
{
	INT32 nRet = 0, nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	KyugoTempRom = (UINT8 *)BurnMalloc(0x18000);
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x8000);
	
	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(pTemp, 0, 1); if (nRet != 0) return 1;
	memcpy(KyugoZ80Rom1 + 0x00000, pTemp + 0x04000, 0x4000);
	nRet = BurnLoadRom(pTemp, 1, 1); if (nRet != 0) return 1;
	memcpy(KyugoZ80Rom1 + 0x04000, pTemp + 0x04000, 0x4000);
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(pTemp, 2, 1); if (nRet != 0) return 1;
	memcpy(KyugoZ80Rom2 + 0x00000, pTemp + 0x04000, 0x4000);
	nRet = BurnLoadRom(pTemp, 3, 1); if (nRet != 0) return 1;
	memcpy(KyugoZ80Rom2 + 0x04000, pTemp + 0x04000, 0x4000);
	
	// Load and decode the chars
	nRet = BurnLoadRom(pTemp, 4, 1); if (nRet != 0) return 1;
	memcpy(KyugoTempRom + 0x00000, pTemp + 0x07000, 0x1000);
	GfxDecode(0x100, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, KyugoTempRom, KyugoChars);
	
	// Load and decode the tiles
	memset(KyugoTempRom, 0, 0x18000);
	nRet = BurnLoadRom(pTemp, 5, 1); if (nRet != 0) return 1;
	memcpy(KyugoTempRom + 0x00000, pTemp + 0x06000, 0x2000);
	nRet = BurnLoadRom(pTemp, 6, 1); if (nRet != 0) return 1;
	memcpy(KyugoTempRom + 0x02000, pTemp + 0x06000, 0x2000);
	nRet = BurnLoadRom(pTemp, 7, 1); if (nRet != 0) return 1;
	memcpy(KyugoTempRom + 0x04000, pTemp + 0x06000, 0x2000);
	GfxDecode(0x400, 3, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, KyugoTempRom, KyugoTiles);
	
	// Load and decode the sprites
	memset(KyugoTempRom, 0, 0x18000);
	nRet = BurnLoadRom(KyugoTempRom + 0x00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(KyugoTempRom + 0x08000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(KyugoTempRom + 0x10000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 3, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, KyugoTempRom, KyugoSprites);
	
	// Load the PROMs
	nRet = BurnLoadRom(KyugoPromRed,   11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(KyugoPromGreen, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(KyugoPromBlue,  13, 1); if (nRet != 0) return 1;
	
	BurnFree(KyugoTempRom);
	BurnFree(pTemp);
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(KyugoRead1);
	ZetSetWriteHandler(KyugoWrite1);
	ZetMapArea(0x0000, 0x7fff, 0, KyugoZ80Rom1             );
	ZetMapArea(0x0000, 0x7fff, 2, KyugoZ80Rom1             );
	ZetMapArea(0x8000, 0x87ff, 0, KyugoBgVideoRam          );
	ZetMapArea(0x8000, 0x87ff, 1, KyugoBgVideoRam          );
	ZetMapArea(0x8000, 0x87ff, 2, KyugoBgVideoRam          );
	ZetMapArea(0x8800, 0x8fff, 0, KyugoBgAttrRam           );
	ZetMapArea(0x8800, 0x8fff, 1, KyugoBgAttrRam           );
	ZetMapArea(0x8800, 0x8fff, 2, KyugoBgAttrRam           );
	ZetMapArea(0x9000, 0x97ff, 0, KyugoFgVideoRam          );
	ZetMapArea(0x9000, 0x97ff, 1, KyugoFgVideoRam          );
	ZetMapArea(0x9000, 0x97ff, 2, KyugoFgVideoRam          );
	ZetMapArea(0x9800, 0x9fff, 1, KyugoSprite2Ram          );
	ZetMapArea(0x9800, 0x9fff, 2, KyugoSprite2Ram          );
	ZetMapArea(0xa000, 0xa7ff, 0, KyugoSprite1Ram          );
	ZetMapArea(0xa000, 0xa7ff, 1, KyugoSprite1Ram          );
	ZetMapArea(0xa000, 0xa7ff, 2, KyugoSprite1Ram          );
	ZetMapArea(0xf000, 0xf7ff, 0, KyugoSharedZ80Ram        );
	ZetMapArea(0xf000, 0xf7ff, 1, KyugoSharedZ80Ram        );
	ZetMapArea(0xf000, 0xf7ff, 2, KyugoSharedZ80Ram        );
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetSetWriteHandler(KyugoWrite2);
	ZetSetInHandler(KyugoPortRead2);
	ZetMapArea(0x0000, 0x7fff, 0, KyugoZ80Rom2             );
	ZetMapArea(0x0000, 0x7fff, 2, KyugoZ80Rom2             );
	ZetMemEnd();
	ZetClose();
	
	ZetOpen(0);
	ZetSetOutHandler(SrdmissnPortWrite1);
	ZetMapArea(0xe000, 0xe7ff, 0, KyugoSharedZ80Ram        );
	ZetMapArea(0xe000, 0xe7ff, 1, KyugoSharedZ80Ram        );
	ZetMapArea(0xe000, 0xe7ff, 2, KyugoSharedZ80Ram        );
	ZetClose();
		
	ZetOpen(1);
	ZetSetReadHandler(SrdmissnRead2);
	ZetSetInHandler(SrdmissnPortRead2);
	ZetSetOutHandler(SrdmissnPortWrite2);
	ZetMapArea(0x8000, 0x87ff, 0, KyugoSharedZ80Ram        );
	ZetMapArea(0x8000, 0x87ff, 1, KyugoSharedZ80Ram        );
	ZetMapArea(0x8000, 0x87ff, 2, KyugoSharedZ80Ram        );
	ZetClose();
	
	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;

	AY8910Init(0, 18432000 / 12, nBurnSoundRate, &KyugoDip0Read, &KyugoDip1Read, NULL, NULL);
	AY8910Init(1, 18432000 / 12, nBurnSoundRate, NULL, NULL, NULL, NULL);
	
	GenericTilesInit();

	// Reset the driver
	KyugoDoReset();

	return 0;
}

static INT32 KyugoExit()
{
	ZetExit();
	
	for (INT32 i = 0; i < 2; i++) {
		AY8910Exit(i);
	}
	
	GenericTilesExit();
	
	KyugoIRQEnable = 0;
	KyugoSubCPUEnable = 0;
	KyugoFgColour = 0;
	KyugoBgPaletteBank = 0;
	KyugoBgScrollXHi = 0;
	KyugoBgScrollXLo = 0;
	KyugoBgScrollY = 0;
	KyugoFlipScreen = 0;
	
	KyugoNumZ80Rom1 = 0;
	KyugoNumZ80Rom2 = 0;
	KyugoNumSpriteRom = 0;
	KyugoSizeZ80Rom1 = 0;
	KyugoSizeZ80Rom2 = 0;
	KyugoSizeSpriteRom = 0;
	
	BurnFree(Mem);

	return 0;
}

static void KyugoCalcPalette()
{
	INT32 i;
	
	for (i = 0; i < 256; i++) {
		INT32 bit0, bit1, bit2, bit3, r, g, b;
		
		bit0 = (KyugoPromRed[i] >> 0) & 0x01;
		bit1 = (KyugoPromRed[i] >> 1) & 0x01;
		bit2 = (KyugoPromRed[i] >> 2) & 0x01;
		bit3 = (KyugoPromRed[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		bit0 = (KyugoPromGreen[i] >> 0) & 0x01;
		bit1 = (KyugoPromGreen[i] >> 1) & 0x01;
		bit2 = (KyugoPromGreen[i] >> 2) & 0x01;
		bit3 = (KyugoPromGreen[i] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		bit0 = (KyugoPromBlue[i] >> 0) & 0x01;
		bit1 = (KyugoPromBlue[i] >> 1) & 0x01;
		bit2 = (KyugoPromBlue[i] >> 2) & 0x01;
		bit3 = (KyugoPromBlue[i] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		KyugoPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void KyugoRenderBgLayer()
{
	INT32 mx, my, Code, Attr, Colour, x, y, TileIndex = 0, xScroll, Flip, xFlip, yFlip;
	
	xScroll = KyugoBgScrollXLo + (KyugoBgScrollXHi * 256);
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = KyugoBgVideoRam[TileIndex];
			Attr = KyugoBgAttrRam[TileIndex];
			Code = Code | ((Attr & 0x03) << 8);
			Code &= 0x3ff;
			Colour = (Attr >> 4) | (KyugoBgPaletteBank << 4);
			Flip = (Attr & 0x0c) >> 2;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 8 * mx;
			y = 8 * my;
						
			if (KyugoFlipScreen) {
				xFlip = !xFlip;
				yFlip = !yFlip;
				y = 248 - y;
				x = 504 - x;
				x -= xScroll;
				y += KyugoBgScrollY;
			} else {
				x -= xScroll;
				y -= KyugoBgScrollY;
			}
			
			if (x < -8) x += 512;
			if (y < -8) y += 256;
			if (y > 264) y -= 256;
			
			x -= 32;
			y -= 16;

			if (x > 8 && x < 280 && y > 8 && y < 216) {
				if (xFlip) {
					if (yFlip) {
						Render8x8Tile_FlipXY(pTransDraw, Code, x, y, Colour, 3, 0, KyugoTiles);
					} else {
						Render8x8Tile_FlipX(pTransDraw, Code, x, y, Colour, 3, 0, KyugoTiles);
					}
				} else {
					if (yFlip) {
						Render8x8Tile_FlipY(pTransDraw, Code, x, y, Colour, 3, 0, KyugoTiles);
					} else {
						Render8x8Tile(pTransDraw, Code, x, y, Colour, 3, 0, KyugoTiles);
					}
				}
			} else {
				if (xFlip) {
					if (yFlip) {
						Render8x8Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 3, 0, KyugoTiles);
					} else {
						Render8x8Tile_FlipX_Clip(pTransDraw, Code, x, y, Colour, 3, 0, KyugoTiles);
					}
				} else {
					if (yFlip) {
						Render8x8Tile_FlipY_Clip(pTransDraw, Code, x, y, Colour, 3, 0, KyugoTiles);
					} else {
						Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 3, 0, KyugoTiles);
					}
				}
			}
			
			TileIndex++;
		}
	}
}

static void KyugoRenderSpriteLayer()
{
	UINT8 *SpriteRam_Area1 = &KyugoSprite1Ram[0x28];
	UINT8 *SpriteRam_Area2 = &KyugoSprite2Ram[0x28];
	UINT8 *SpriteRam_Area3 = &KyugoFgVideoRam[0x28];
	
	INT32 i;
	
	for (i = 0; i < 24; i++) {
		INT32 Offset, y, sy, sx, Colour;
		
		Offset = 2 * (i % 12) + 64 * (i / 12);
		
		sx = SpriteRam_Area3[Offset + 1] + 256 * (SpriteRam_Area2[Offset + 1] & 1);
		if (sx > 320) sx -= 512;
		sy = 255 - SpriteRam_Area1[Offset] + 2;
		if (sy > 0xf0) sy -= 256;
		if (KyugoFlipScreen) sy = 240 - sy;
		sy -= 16;
		
		Colour = SpriteRam_Area1[Offset + 1] & 0x1f;
		
		for (y = 0; y < 16; y++) {
			INT32 Code, Attr, FlipX, FlipY, yPos;

			Code = SpriteRam_Area3[Offset + 128 * y];
			Attr = SpriteRam_Area2[Offset + 128 * y];
			Code = Code | ((Attr & 0x01) << 9) | ((Attr & 0x02) << 7);
			Code &= 0x3ff;

			FlipX =  Attr & 0x08;
			FlipY =  Attr & 0x04;
			
			yPos = sy + 16 * y;
			
			if (KyugoFlipScreen) {
				FlipX = !FlipX;
				FlipY = !FlipY;
				yPos = sy - 16 * y;
			}
			
			if (sx > 16 && sx < 272 && yPos > 16 && yPos < 208) {
				if (FlipX) {
					if (FlipY) {
						Render16x16Tile_Mask_FlipXY(pTransDraw, Code, sx, yPos, Colour, 3, 0, 0, KyugoSprites);	
					} else {
						Render16x16Tile_Mask_FlipX(pTransDraw, Code, sx, yPos, Colour, 3, 0, 0, KyugoSprites);	
					}
				} else {
					if (FlipY) {
						Render16x16Tile_Mask_FlipY(pTransDraw, Code, sx, yPos, Colour, 3, 0, 0, KyugoSprites);
					} else {
						Render16x16Tile_Mask(pTransDraw, Code, sx, yPos, Colour, 3, 0, 0, KyugoSprites);	
					}
				}
			} else {
				if (FlipX) {
					if (FlipY) {
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx, yPos, Colour, 3, 0, 0, KyugoSprites);	
					} else {
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, sx, yPos, Colour, 3, 0, 0, KyugoSprites);	
					}
				} else {
					if (FlipY) {
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, sx, yPos, Colour, 3, 0, 0, KyugoSprites);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, yPos, Colour, 3, 0, 0, KyugoSprites);	
					}
				}
			}
		}
	}
}

static void KyugoRenderCharLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0, FlipX = 0, FlipY = 0;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = KyugoFgVideoRam[TileIndex];
			Colour = 2 * KyugoPromCharLookup[Code >> 3] + KyugoFgColour;
			Code &= 0xff;
			
			x = 8 * mx;
			y = 8 * my;
			
			if (KyugoFlipScreen) {
				FlipX = 1;
				FlipY = 1;
				y = 248 - y;
				x = 280 - x;
			}
			
			y -=  16;
			
			if (x > 0 && x < 280 && y > 0 && y < 216) {
				if (FlipX) {
					if (FlipY) {
						Render8x8Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 2, 0, 0, KyugoChars);
					} else {
						Render8x8Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 2, 0, 0, KyugoChars);
					}
				} else {
					if (FlipY) {
						Render8x8Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 2, 0, 0, KyugoChars);
					} else {
						Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 2, 0, 0, KyugoChars);
					}
				}
			} else {
				if (FlipX) {
					if (FlipY) {
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 2, 0, 0, KyugoChars);
					} else {
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 2, 0, 0, KyugoChars);
					}
				} else {
					if (FlipY) {
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 2, 0, 0, KyugoChars);
					} else {
						Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 2, 0, 0, KyugoChars);
					}
				}
			}

			TileIndex++;
		}
	}
}

static void KyugoDraw()
{
	BurnTransferClear();
	KyugoCalcPalette();
	KyugoRenderBgLayer();
	KyugoRenderSpriteLayer();
	KyugoRenderCharLayer();
	BurnTransferCopy(KyugoPalette);
}

static INT32 KyugoFrame()
{
	INT32 nInterleave = 10;
	INT32 nSoundBufferPos = 0;

	if (KyugoReset) KyugoDoReset();

	KyugoMakeInputs();

	nCyclesTotal[0] = (18432000 / 6) / 60;
	nCyclesTotal[1] = (18432000 / 6) / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == 9 && KyugoIRQEnable) ZetNmi();
		ZetClose();
		
		// Run Z80 #2
		nCurrentCPU = 1;
		if (KyugoSubCPUEnable) {
			ZetOpen(nCurrentCPU);
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesSegment = ZetRun(nCyclesSegment);
			nCyclesDone[nCurrentCPU] += nCyclesSegment;
			if (i == 2 || i == 4 || i == 6 || i == 8) ZetRaiseIrq(0);
			ZetClose();
		}
		
		// Render Sound Segment
		if (pBurnSoundOut) {
			INT32 nSample;
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n] >> 2;
				nSample += pAY8910Buffer[1][n] >> 2;
				nSample += pAY8910Buffer[2][n] >> 2;
				nSample += pAY8910Buffer[3][n] >> 2;
				nSample += pAY8910Buffer[4][n] >> 2;
				nSample += pAY8910Buffer[5][n] >> 2;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
    			}
				nSoundBufferPos += nSegmentLength;
		}

	}
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSample;
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n] >> 2;
				nSample += pAY8910Buffer[1][n] >> 2;
				nSample += pAY8910Buffer[2][n] >> 2;
				nSample += pAY8910Buffer[3][n] >> 2;
				nSample += pAY8910Buffer[4][n] >> 2;
				nSample += pAY8910Buffer[5][n] >> 2;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
 			}
		}
	}

	if (pBurnDraw) KyugoDraw();

	return 0;
}

static INT32 KyugoScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029674;
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
		AY8910Scan(nAction, pnMin);

		// Scan critical driver variables
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
		SCAN_VAR(KyugoDip);
		SCAN_VAR(KyugoInput);
		SCAN_VAR(KyugoIRQEnable);
		SCAN_VAR(KyugoSubCPUEnable);
		SCAN_VAR(KyugoFgColour);
		SCAN_VAR(KyugoBgPaletteBank);
		SCAN_VAR(KyugoBgScrollXHi);
		SCAN_VAR(KyugoBgScrollXLo);
		SCAN_VAR(KyugoBgScrollY);
		SCAN_VAR(KyugoFlipScreen);
	}


	return 0;
}

struct BurnDriver BurnDrvAirwolf = {
	"airwolf", NULL, NULL, NULL, "1987",
	"Airwolf\0", NULL, "Kyugo", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, AirwolfRomInfo, AirwolfRomName, NULL, NULL, KyugoInputInfo, AirwolfDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 288, 224, 4, 3
};

struct BurnDriver BurnDrvAirwolfa = {
	"airwolfa", "airwolf", NULL, NULL, "1987",
	"Airwolf (US)\0", NULL, "Kyugo (UA Theatre license)", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, AirwolfaRomInfo, AirwolfaRomName, NULL, NULL, KyugoInputInfo, AirwolfDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 288, 224, 4, 3
};

struct BurnDriver BurnDrvSkywolf = {
	"skywolf", "airwolf", NULL, NULL, "1987",
	"Sky Wolf (set 1)\0", NULL, "bootleg", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, SkywolfRomInfo, SkywolfRomName, NULL, NULL, KyugoInputInfo, SkywolfDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 288, 224, 4, 3
};

struct BurnDriver BurnDrvSkywolf2 = {
	"skywolf2", "airwolf", NULL, NULL, "1987",
	"Sky Wolf (set 2)\0", NULL, "bootleg", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, Skywolf2RomInfo, Skywolf2RomName, NULL, NULL, KyugoInputInfo, AirwolfDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 288, 224, 4, 3
};

struct BurnDriver BurnDrvSkywolf3 = {
	"skywolf3", "airwolf", NULL, NULL, "1987",
	"Sky Wolf (set 3)\0", NULL, "bootleg", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, Skywolf3RomInfo, Skywolf3RomName, NULL, NULL, KyugoInputInfo, AirwolfDIPInfo,
	Skywolf3Init, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 288, 224, 4, 3
};

struct BurnDriver BurnDrvFlashgal = {
	"flashgal", NULL, NULL, NULL, "1985",
	"Flashgal (set 1)\0", NULL, "Sega", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, FlashgalRomInfo, FlashgalRomName, NULL, NULL, KyugoInputInfo, FlashgalDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 288, 224, 4, 3
};

struct BurnDriver BurnDrvFlashgala = {
	"flashgala", "flashgal", NULL, NULL, "1985",
	"Flashgal (set 2)\0", NULL, "Sega", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, FlashgalaRomInfo, FlashgalaRomName, NULL, NULL, KyugoInputInfo, FlashgalDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 288, 224, 4, 3
};

struct BurnDriver BurnDrvGyrodine = {
	"gyrodine", NULL, NULL, NULL, "1984",
	"Gyrodine\0", NULL, "Crux", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, GyrodineRomInfo, GyrodineRomName, NULL, NULL, KyugoInputInfo, GyrodineDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 224, 288, 3, 4
};

struct BurnDriver BurnDrvGyrodinet = {
	"gyrodinet", "gyrodine", NULL, NULL, "1984",
	"Gyrodine (Taito Corporation license)\0", NULL, "Crux (Taito Corporation license)", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, GyrodinetRomInfo, GyrodinetRomName, NULL, NULL, KyugoInputInfo, GyrodineDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 224, 288, 3, 4
};

struct BurnDriver BurnDrvBuzzard = {
	"buzzard", "gyrodine", NULL, NULL, "1984",
	"Buzzard\0", NULL, "Crux", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, BuzzardRomInfo, BuzzardRomName, NULL, NULL, KyugoInputInfo, GyrodineDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 224, 288, 3, 4
};

struct BurnDriver BurnDrvLegend = {
	"legend", NULL, NULL, NULL, "1986",
	"Legend\0", NULL, "Sega / Coreland", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, LegendRomInfo, LegendRomName, NULL, NULL, KyugoInputInfo, LegendDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 288, 224, 4, 3
};

struct BurnDriver BurnDrvSonofphx = {
	"sonofphx", "repulse", NULL, NULL, "1985",
	"Son of Phoenix\0", NULL, "Associated Overseas MFR, Inc", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, SonofphxRomInfo, SonofphxRomName, NULL, NULL, KyugoInputInfo, SonofphxDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 224, 288, 3, 4
};

struct BurnDriver BurnDrvRepulse = {
	"repulse", NULL, NULL, NULL, "1985",
	"Repulse\0", NULL, "Sega", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, RepulseRomInfo, RepulseRomName, NULL, NULL, KyugoInputInfo, SonofphxDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 224, 288, 3, 4
};

struct BurnDriver BurnDrv99lstwar = {
	"99lstwar", "repulse", NULL, NULL, "1985",
	"'99: The Last War\0", NULL, "Proma", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, Lstwar99RomInfo, Lstwar99RomName, NULL, NULL, KyugoInputInfo, SonofphxDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 224, 288, 3, 4
};

struct BurnDriver BurnDrv99lstwara = {
	"99lstwara", "repulse", NULL, NULL, "1985",
	"'99: The Last War (alternate)\0", NULL, "Proma", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, Lstwra99RomInfo, Lstwra99RomName, NULL, NULL, KyugoInputInfo, SonofphxDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 224, 288, 3, 4
};

struct BurnDriver BurnDrv99lstwark = {
	"99lstwark", "repulse", NULL, NULL, "1985",
	"'99: The Last War (Kyugo)\0", NULL, "Kyugo", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, Lstwrk99RomInfo, Lstwrk99RomName, NULL, NULL, KyugoInputInfo, SonofphxDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 224, 288, 3, 4
};

struct BurnDriver BurnDrvSrdmissn = {
	"srdmissn", NULL, NULL, NULL, "1986",
	"S.R.D. Mission\0", NULL, "Taito Corporation", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, SrdmissnRomInfo, SrdmissnRomName, NULL, NULL, KyugoInputInfo, SrdmissnDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 224, 288, 3, 4
};

struct BurnDriver BurnDrvFx = {
	"fx", "srdmissn", NULL, NULL, "1986",
	"F-X\0", NULL, "bootleg", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, FxRomInfo, FxRomName, NULL, NULL, KyugoInputInfo, SrdmissnDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 224, 288, 3, 4
};

struct BurnDriver BurnDrvFxa = {
	"fxa", "srdmissn", NULL, NULL, "1986",
	"F-X (alternate set)\0", NULL, "bootleg", "Kyugo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, FxaRomInfo, FxaRomName, NULL, NULL, KyugoInputInfo, SrdmissnDIPInfo,
	KyugoInit, KyugoExit, KyugoFrame, NULL, KyugoScan,
	NULL, 0x100, 224, 288, 3, 4
};
