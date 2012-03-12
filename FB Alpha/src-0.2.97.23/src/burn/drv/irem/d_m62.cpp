#include "tiles_generic.h"
#include "m6800_intf.h"
#include "msm5205.h"

#include "driver.h"
extern "C" {
 #include "ay8910.h"
}

static UINT8 M62InputPort0[8]       = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 M62InputPort1[8]       = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 M62InputPort2[8]       = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 M62Dip[2]              = {0, 0};
static UINT8 M62Input[3]            = {0x00, 0x00, 0x00};
static UINT8 M62Reset               = 0;

static UINT32  M62Z80RomSize          = 0;
static UINT32  M62PromSize            = 0;
static UINT32  M62NumTiles            = 0;
static UINT32  M62NumSprites          = 0;
static UINT32  M62NumChars            = 0;
static UINT32  M62SpriteRamSize       = 0;
static UINT32  M62CharRamSize         = 0;
static UINT32  M62ScrollRamSize       = 0;

static UINT8 *Mem                   = NULL;
static UINT8 *MemEnd                = NULL;
static UINT8 *RamStart              = NULL;
static UINT8 *RamEnd                = NULL;
static UINT8 *M62Z80Rom             = NULL;
static UINT8 *M62M6803Rom           = NULL;
static UINT8 *M62TileRam            = NULL;
static UINT8 *M62SpriteRam          = NULL;
static UINT8 *M62CharRam            = NULL;
static UINT8 *M62ScrollRam          = NULL;
static UINT8 *M62Z80Ram             = NULL;
static UINT8 *M62M6803Ram           = NULL;
static UINT8 *M62Tiles              = NULL;
static UINT8 *M62Sprites            = NULL;
static UINT8 *M62Chars              = NULL;
static UINT8 *M62PromData           = NULL;
static UINT8 *M62TempRom            = NULL;
static UINT32  *M62Palette            = NULL;
static INT16* pFMBuffer;
static INT16* pAY8910Buffer[6];

static INT32 M62BackgroundHScroll;
static INT32 M62BackgroundVScroll;
static INT32 M62CharHScroll;
static INT32 M62CharVScroll;
static INT32 M62FlipScreen;
static INT32 M62SpriteHeightPromOffset;
static UINT8 M62SoundLatch;
static UINT8 M62Port1;
static UINT8 M62Port2;
static UINT8 M62SlaveMSM5205VClckReset;
static UINT32 M62PaletteEntries;
static UINT32 M62Z80Clock;
static UINT32 M62M6803Clock;
static UINT8 M62BankControl[2];
static UINT8 Ldrun2BankSwap;
static UINT8 Ldrun3TopBottomMask;
static UINT8 KidnikiBackgroundBank;
static UINT8 SpelunkrPaletteBank;
static INT32 M62BgxTileDim = 0;
static INT32 M62BgyTileDim = 0;
static INT32 M62CharxTileDim = 0;
static INT32 M62CharyTileDim = 0;

typedef void (*M62Render)();
static M62Render M62RenderFunction;
static void KungfumDraw();
static void BattroadDraw();
static void LdrunDraw();
static void Ldrun3Draw();
static void Ldrun4Draw();
static void LotlotDraw();
static void KidnikiDraw();
static void SpelunkrDraw();
static void Spelunk2Draw();
static void YoujyudnDraw();
static void HorizonDraw();

typedef void (*M62ExtendTileInfo)(INT32*, INT32*, INT32*, INT32*);
static M62ExtendTileInfo M62ExtendTileInfoFunction;
static void BattroadExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32* xFlip);
static void LdrunExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32* xFlip);
static void Ldrun2ExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32* xFlip);
static void Ldrun4ExtendTile(INT32* Code, INT32* Colour, INT32*, INT32*);
static void LotlotExtendTile(INT32* Code, INT32* Colour, INT32*, INT32* xFlip);
static void KidnikiExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32*);
static void SpelunkrExtendTile(INT32* Code, INT32* Colour, INT32*, INT32*);
static void Spelunk2ExtendTile(INT32* Code, INT32* Colour, INT32*, INT32*);
static void YoujyudnExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32*);
static void HorizonExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32*);

typedef void (*M62ExtendCharInfo)(INT32*, INT32*, INT32*, INT32*);
static M62ExtendCharInfo M62ExtendCharInfoFunction;
static void BattroadExtendChar(INT32* Code, INT32* Colour, INT32*, INT32*);
static void LotlotExtendChar(INT32* Code, INT32* Colour, INT32*, INT32*);
static void SpelunkrExtendChar(INT32* Code, INT32* Colour, INT32*, INT32*);
static void YoujyudnExtendChar(INT32* Code, INT32* Colour, INT32*, INT32*);

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static struct BurnInputInfo M62InputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , M62InputPort0 + 3, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , M62InputPort0 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , M62InputPort2 + 4, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , M62InputPort0 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL  , M62InputPort1 + 3, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , M62InputPort1 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , M62InputPort1 + 1, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , M62InputPort1 + 0, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , M62InputPort1 + 7, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , M62InputPort1 + 5, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL  , M62InputPort2 + 3, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL  , M62InputPort2 + 2, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL  , M62InputPort2 + 1, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , M62InputPort2 + 0, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , M62InputPort2 + 7, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL  , M62InputPort2 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &M62Reset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , M62InputPort0 + 2, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, M62Dip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, M62Dip + 1       , "dip"       },
};

STDINPUTINFO(M62)

static inline void M62ClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

static inline void M62MakeInputs()
{
	M62Input[0] = M62Input[1] = M62Input[2] = 0x00;

	for (INT32 i = 0; i < 8; i++) {
		M62Input[0] |= (M62InputPort0[i] & 1) << i;
		M62Input[1] |= (M62InputPort1[i] & 1) << i;
		M62Input[2] |= (M62InputPort2[i] & 1) << i;
	}

	M62ClearOpposites(&M62Input[0]);
	M62ClearOpposites(&M62Input[1]);
}

#define IREM_Z80_COINAGE_TYPE3										\
	{0   , 0xfe, 0   , 15  , "Coinage"                },			\
	{0x12, 0x02, 0xf0, 0x90, "7 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xa0, "6 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xb0, "5 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xc0, "4 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xd0, "3 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xe0, "2 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xf0, "1 Coin  1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x70, "1 Coin  2 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x60, "1 Coin  3 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x50, "1 Coin  4 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x40, "1 Coin  5 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x30, "1 Coin  6 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x20, "1 Coin  7 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x10, "1 Coin  8 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x00, "Freeplay"               },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
																	\
	{0   , 0xfe, 0   , 4   , "Coin A"                 },			\
	{0x12, 0x82, 0x30, 0x10, "3 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0x30, 0x20, "2 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0x30, 0x30, "1 Coin  1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0x30, 0x00, "Free Play"              },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0   , 0xfe, 0   , 4   , "Coin B"                 },			\
	{0x12, 0x82, 0xc0, 0xc0, "1 Coin  2 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0xc0, 0x80, "1 Coin  3 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0xc0, 0x40, "1 Coin  5 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0xc0, 0x00, "1 Coin  6 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\

#define IREM_Z80_COINAGE_TYPE4										\
	{0   , 0xfe, 0   , 16  , "Coinage"                },			\
	{0x12, 0x02, 0xf0, 0xa0, "6 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xb0, "5 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xc0, "4 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xd0, "3 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x10, "8 Coins 3 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xe0, "2 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x20, "5 Coins 3 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x30, "3 Coins 2 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xf0, "1 Coin  1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x40, "2 Coins 3 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x90, "1 Coin  2 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x80, "1 Coin  3 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x70, "1 Coin  4 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x60, "1 Coin  5 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x50, "1 Coin  6 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x00, "Freeplay"               },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
																	\
	{0   , 0xfe, 0   , 4   , "Coin A"                 },			\
	{0x12, 0x82, 0x30, 0x00, "5 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0x30, 0x10, "3 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0x30, 0x20, "2 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0x30, 0x30, "1 Coin  1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0   , 0xfe, 0   , 4   , "Coin B"                 },			\
	{0x12, 0x82, 0xc0, 0xc0, "1 Coin  2 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0xc0, 0x80, "1 Coin  3 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0xc0, 0x40, "1 Coin  5 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0xc0, 0x00, "1 Coin  6 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	
#define IREM_Z80_COINAGE_TYPE5										\
	{0   , 0xfe, 0   , 16  , "Coinage"                },			\
	{0x12, 0x02, 0xf0, 0x00, "8 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xa0, "6 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xb0, "5 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xc0, "4 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xd0, "3 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xe0, "2 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x10, "5 Coins 3 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x20, "3 Coins 5 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x30, "3 Coins 2 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0xf0, "1 Coin  1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x40, "2 Coins 3 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x90, "1 Coin  2 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x80, "1 Coin  3 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x70, "1 Coin  4 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x60, "1 Coin  5 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x02, 0xf0, 0x50, "1 Coin  6 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
																	\
	{0   , 0xfe, 0   , 4   , "Coin A"                 },			\
	{0x12, 0x82, 0x30, 0x00, "5 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0x30, 0x10, "3 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0x30, 0x20, "2 Coins 1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0x30, 0x30, "1 Coin  1 Play"         },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0   , 0xfe, 0   , 4   , "Coin B"                 },			\
	{0x12, 0x82, 0xc0, 0xc0, "1 Coin  2 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0xc0, 0x80, "1 Coin  3 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0xc0, 0x40, "1 Coin  5 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\
	{0x12, 0x82, 0xc0, 0x00, "1 Coin  6 Plays"        },			\
	{0x13, 0x00, 0x04, 0x04, NULL                     },			\

static struct BurnDIPInfo KungfumDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xfd, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2  ,  "Difficulty"             },
	{0x12, 0x01, 0x01, 0x01, "Easy"                   },
	{0x12, 0x01, 0x01, 0x00, "Hard"                   },
	
	{0   , 0xfe, 0   , 2  ,  "Energy Loss"            },
	{0x12, 0x01, 0x02, 0x02, "Slow"                   },
	{0x12, 0x01, 0x02, 0x00, "Fast"                   },
	
	{0   , 0xfe, 0   , 4  ,  "Lives"                  },
	{0x12, 0x01, 0x0c, 0x08, "2"                      },
	{0x12, 0x01, 0x0c, 0x0c, "3"                      },
	{0x12, 0x01, 0x0c, 0x04, "4"                      },
	{0x12, 0x01, 0x0c, 0x00, "5"                      },
	
	IREM_Z80_COINAGE_TYPE3	
	
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
	
	{0   , 0xfe, 0   , 2   , "Slow Motion Mode"       },
	{0x13, 0x01, 0x08, 0x08, "Off"                    },
	{0x13, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x10, 0x10, "Off"                    },
	{0x13, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Level Selection Mode"   },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Kungfum)

static struct BurnDIPInfo BattroadDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xfd, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4  ,  "Fuel Decrease"          },
	{0x12, 0x01, 0x03, 0x03, "Slow"                   },
	{0x12, 0x01, 0x03, 0x02, "Medium"                 },
	{0x12, 0x01, 0x03, 0x01, "Fast"                   },
	{0x12, 0x01, 0x03, 0x00, "Fastest"                },
	
	{0   , 0xfe, 0   , 2  ,  "Difficulty"             },
	{0x12, 0x01, 0x04, 0x04, "Easy"                   },
	{0x12, 0x01, 0x04, 0x00, "Hard"                   },
	
	IREM_Z80_COINAGE_TYPE3
	
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
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x10, 0x10, "Off"                    },
	{0x13, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Battroad)

static struct BurnDIPInfo LdrunDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xfd, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4  ,  "Timer"                  },
	{0x12, 0x01, 0x03, 0x03, "Slow"                   },
	{0x12, 0x01, 0x03, 0x02, "Medium"                 },
	{0x12, 0x01, 0x03, 0x01, "Fast"                   },
	{0x12, 0x01, 0x03, 0x00, "Fastest"                },
	
	{0   , 0xfe, 0   , 4  ,  "Lives"                  },
	{0x12, 0x01, 0x0c, 0x08, "2"                      },
	{0x12, 0x01, 0x0c, 0x0c, "3"                      },
	{0x12, 0x01, 0x0c, 0x04, "4"                      },
	{0x12, 0x01, 0x0c, 0x00, "5"                      },
	
	IREM_Z80_COINAGE_TYPE3
	
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
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x10, 0x10, "Off"                    },
	{0x13, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Level Selection Mode"   },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Ldrun)

static struct BurnDIPInfo Ldrun2DIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xfd, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2  ,  "Timer"                  },
	{0x12, 0x01, 0x01, 0x01, "Slow"                   },
	{0x12, 0x01, 0x01, 0x00, "Fast"                   },
	
	{0   , 0xfe, 0   , 2  ,  "Game Speed"             },
	{0x12, 0x01, 0x02, 0x00, "Low"                    },
	{0x12, 0x01, 0x02, 0x02, "High"                   },
	
	{0   , 0xfe, 0   , 4  ,  "Lives"                  },
	{0x12, 0x01, 0x0c, 0x08, "2"                      },
	{0x12, 0x01, 0x0c, 0x0c, "3"                      },
	{0x12, 0x01, 0x0c, 0x04, "4"                      },
	{0x12, 0x01, 0x0c, 0x00, "5"                      },
	
	IREM_Z80_COINAGE_TYPE3
	
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
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x10, 0x10, "Off"                    },
	{0x13, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Level Selection Mode"   },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Ldrun2)

static struct BurnDIPInfo Ldrun4DIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 2  ,  "Timer"                  },
	{0x12, 0x01, 0x01, 0x01, "Slow"                   },
	{0x12, 0x01, 0x01, 0x00, "Fast"                   },
	
	{0   , 0xfe, 0   , 2  ,  "2 Players Game"         },
	{0x12, 0x01, 0x02, 0x00, "1 Credit"               },
	{0x12, 0x01, 0x02, 0x02, "2 Credits"              },
	
	{0   , 0xfe, 0   , 4  ,  "1 Player Lives"         },
	{0x12, 0x01, 0x0c, 0x08, "2"                      },
	{0x12, 0x01, 0x0c, 0x0c, "3"                      },
	{0x12, 0x01, 0x0c, 0x04, "4"                      },
	{0x12, 0x01, 0x0c, 0x00, "5"                      },
	
	IREM_Z80_COINAGE_TYPE3
	
	// Dip 2
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x13, 0x01, 0x01, 0x01, "Off"                    },
	{0x13, 0x01, 0x01, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "2 Players Lives"        },
	{0x13, 0x01, 0x02, 0x02, "5"                      },
	{0x13, 0x01, 0x02, 0x00, "6"                      },
	
	{0   , 0xfe, 0   , 2   , "Coin Mode"              },
	{0x13, 0x01, 0x04, 0x04, "Mode 1"                 },
	{0x13, 0x01, 0x04, 0x00, "Mode 2"                 },
	
	{0   , 0xfe, 0   , 2   , "Allow 2 Player Game"    },
	{0x13, 0x01, 0x10, 0x00, "Off"                    },
	{0x13, 0x01, 0x10, 0x10, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Level Selection Mode"   },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Ldrun4)

static struct BurnDIPInfo LotlotDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xf5, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4  ,  "Speed"                  },
	{0x12, 0x01, 0x03, 0x03, "Very Slow"              },
	{0x12, 0x01, 0x03, 0x02, "Slow"                   },
	{0x12, 0x01, 0x03, 0x01, "Fast"                   },
	{0x12, 0x01, 0x03, 0x00, "Very Fast"              },
	
	{0   , 0xfe, 0   , 4  ,  "Lives"                  },
	{0x12, 0x01, 0x0c, 0x08, "1"                      },
	{0x12, 0x01, 0x0c, 0x0c, "2"                      },
	{0x12, 0x01, 0x0c, 0x04, "3"                      },
	{0x12, 0x01, 0x0c, 0x00, "4"                      },
	
	IREM_Z80_COINAGE_TYPE4
	
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
	{0x13, 0x01, 0x08, 0x08, "Off"                    },
	{0x13, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x10, 0x10, "Off"                    },
	{0x13, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Lotlot)

static struct BurnDIPInfo KidnikiDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4  ,  "Lives"                  },
	{0x12, 0x01, 0x03, 0x02, "2"                      },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x01, "4"                      },
	{0x12, 0x01, 0x03, 0x00, "5"                      },
	
	{0   , 0xfe, 0   , 2  ,  "Difficulty"             },
	{0x12, 0x01, 0x04, 0x04, "Normal"                 },
	{0x12, 0x01, 0x04, 0x00, "Hard"                   },
	
	{0   , 0xfe, 0   , 2  ,  "Bonus Life"             },
	{0x12, 0x01, 0x08, 0x08, "50000"                  },
	{0x12, 0x01, 0x08, 0x00, "80000"                  },
	
	IREM_Z80_COINAGE_TYPE4
	
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
	
	{0   , 0xfe, 0   , 2   , "Game Repeats"           },
	{0x13, 0x01, 0x08, 0x08, "Off"                    },
	{0x13, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x13, 0x01, 0x10, 0x00, "Off"                    },
	{0x13, 0x01, 0x10, 0x10, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Kidniki)

static struct BurnDIPInfo SpelunkrDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xf5, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4  ,  "Energy Decrease"        },
	{0x12, 0x01, 0x03, 0x03, "Slow"                   },
	{0x12, 0x01, 0x03, 0x02, "Medium"                 },
	{0x12, 0x01, 0x03, 0x01, "Fast"                   },
	{0x12, 0x01, 0x03, 0x00, "Fastest"                },
	
	{0   , 0xfe, 0   , 4  ,  "Lives"                  },
	{0x12, 0x01, 0x0c, 0x08, "2"                      },
	{0x12, 0x01, 0x0c, 0x0c, "3"                      },
	{0x12, 0x01, 0x0c, 0x04, "4"                      },
	{0x12, 0x01, 0x0c, 0x00, "5"                      },

	IREM_Z80_COINAGE_TYPE4
	
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
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x13, 0x01, 0x08, 0x08, "Off"                    },
	{0x13, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Teleport"               },
	{0x13, 0x01, 0x10, 0x10, "Off"                    },
	{0x13, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Spelunkr)

static struct BurnDIPInfo Spelunk2DIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4  ,  "Energy Decrease"        },
	{0x12, 0x01, 0x03, 0x03, "Slow"                   },
	{0x12, 0x01, 0x03, 0x02, "Medium"                 },
	{0x12, 0x01, 0x03, 0x01, "Fast"                   },
	{0x12, 0x01, 0x03, 0x00, "Fastest"                },
	
	{0   , 0xfe, 0   , 4  ,  "Lives"                  },
	{0x12, 0x01, 0x0c, 0x08, "2"                      },
	{0x12, 0x01, 0x0c, 0x0c, "3"                      },
	{0x12, 0x01, 0x0c, 0x04, "4"                      },
	{0x12, 0x01, 0x0c, 0x00, "5"                      },

	IREM_Z80_COINAGE_TYPE4
	
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
	
	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x13, 0x01, 0x08, 0x00, "Off"                    },
	{0x13, 0x01, 0x08, 0x08, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x10, 0x00, "Off"                    },
	{0x13, 0x01, 0x10, 0x10, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Spelunk2)

static struct BurnDIPInfo YoujyudnDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xfd, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4  ,  "Lives"                  },
	{0x12, 0x01, 0x03, 0x02, "2"                      },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x01, "4"                      },
	{0x12, 0x01, 0x03, 0x00, "5"                      },
	
	IREM_Z80_COINAGE_TYPE4
	
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
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"             },
	{0x13, 0x01, 0x08, 0x08, "20000 60000"            },
	{0x13, 0x01, 0x08, 0x00, "40000 80000"            },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x10, 0x00, "Off"                    },
	{0x13, 0x01, 0x10, 0x10, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Level Selection Mode"   },
	{0x13, 0x01, 0x20, 0x20, "Off"                    },
	{0x13, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Youjyudn)

static struct BurnDIPInfo HorizonDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xfd, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4  ,  "Lives"                  },
	{0x12, 0x01, 0x03, 0x00, "2"                      },
	{0x12, 0x01, 0x03, 0x03, "3"                      },
	{0x12, 0x01, 0x03, 0x01, "4"                      },
	{0x12, 0x01, 0x03, 0x02, "5"                      },
	
	{0   , 0xfe, 0   , 4  ,  "Bonus Life"             },
	{0x12, 0x01, 0x0c, 0x00, "100 and 80k"            },
	{0x12, 0x01, 0x0c, 0x0c, "40k and every 80k"      },
	{0x12, 0x01, 0x0c, 0x08, "60k and every 100k"     },
	{0x12, 0x01, 0x0c, 0x04, "80k and every 120k"     },
	
	IREM_Z80_COINAGE_TYPE5
	
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
	
	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x13, 0x01, 0x08, 0x08, "Off"                    },
	{0x13, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Frame Advance"          },
	{0x13, 0x01, 0x10, 0x10, "Off"                    },
	{0x13, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Invulnerability"        },
	{0x13, 0x01, 0x40, 0x40, "Off"                    },
	{0x13, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },	
};

STDDIPINFO(Horizon)

static struct BurnRomInfo KungfumRomDesc[] = {
	{ "a-4e-c.bin",           0x04000, 0xb6e2d083, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "a-4d-c.bin",           0x04000, 0x7532918e, BRF_ESS | BRF_PRG }, //  1
	
	{ "a-3e-.bin",            0x02000, 0x58e87ab0, BRF_ESS | BRF_PRG }, //  2	M6803 Program Code
	{ "a-3f-.bin",            0x02000, 0xc81e31ea, BRF_ESS | BRF_PRG }, //  3
	{ "a-3h-.bin",            0x02000, 0xd99fb995, BRF_ESS | BRF_PRG }, //  4
	
	{ "g-4c-a.bin",           0x02000, 0x6b2cc9c8, BRF_GRA },	    //  5	Characters
	{ "g-4d-a.bin",           0x02000, 0xc648f558, BRF_GRA },	    //  6
	{ "g-4e-a.bin",           0x02000, 0xfbe9276e, BRF_GRA },	    //  7
	
	{ "b-4k-.bin",            0x02000, 0x16fb5150, BRF_GRA },	    //  8	Sprites
	{ "b-4f-.bin",            0x02000, 0x67745a33, BRF_GRA },	    //  9
	{ "b-4l-.bin",            0x02000, 0xbd1c2261, BRF_GRA },	    //  10
	{ "b-4h-.bin",            0x02000, 0x8ac5ed3a, BRF_GRA },	    //  11
	{ "b-3n-.bin",            0x02000, 0x28a213aa, BRF_GRA },	    //  12
	{ "b-4n-.bin",            0x02000, 0xd5228df3, BRF_GRA },	    //  13
	{ "b-4m-.bin",            0x02000, 0xb16de4f2, BRF_GRA },	    //  14
	{ "b-3m-.bin",            0x02000, 0xeba0d66b, BRF_GRA },	    //  15
	{ "b-4c-.bin",            0x02000, 0x01298885, BRF_GRA },	    //  16
	{ "b-4e-.bin",            0x02000, 0xc77b87d4, BRF_GRA },	    //  17
	{ "b-4d-.bin",            0x02000, 0x6a70615f, BRF_GRA },	    //  18
	{ "b-4a-.bin",            0x02000, 0x6189d626, BRF_GRA },	    //  19
	
	{ "g-1j-.bin",            0x00100, 0x668e6bca, BRF_GRA },	    //  20	PROM (Tile Palette Red Component)
	{ "b-1m-.bin",            0x00100, 0x76c05a9c, BRF_GRA },	    //  21	PROM (Sprite Palette Red Component)
	{ "g-1f-.bin",            0x00100, 0x964b6495, BRF_GRA },	    //  22	PROM (Tile Palette Green Component)
	{ "b-1n-.bin",            0x00100, 0x23f06b99, BRF_GRA },	    //  23	PROM (Sprite Palette Green Component)
	{ "g-1h-.bin",            0x00100, 0x550563e1, BRF_GRA },	    //  24	PROM (Tile Palette Blue Component)
	{ "b-1l-.bin",            0x00100, 0x35e45021, BRF_GRA },	    //  25	PROM (Sprite Palette Blue Component)
	{ "b-5f-.bin",            0x00020, 0x7a601c3d, BRF_GRA },	    //  26	PROM (Sprite Height)
	{ "b-6f-.bin",            0x00100, 0x82c20d12, BRF_GRA },	    //  27	PROM (Video Timing)
};

STD_ROM_PICK(Kungfum)
STD_ROM_FN(Kungfum)

static struct BurnRomInfo KungfumdRomDesc[] = {
	{ "snx_a-4e-d",           0x04000, 0xfc330a46, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "snx_a-4d-d",           0x04000, 0x1b2fd32f, BRF_ESS | BRF_PRG }, //  1
	
	{ "snx_a-3d-b",           0x04000, 0x85ca7956, BRF_ESS | BRF_PRG }, //  2	M6803 Program Code
	{ "snx_a-3f-b",           0x04000, 0x3ef1100a, BRF_ESS | BRF_PRG }, //  3
	
	{ "g-4c-a.bin",           0x02000, 0x6b2cc9c8, BRF_GRA },	    //  4	Characters
	{ "g-4d-a.bin",           0x02000, 0xc648f558, BRF_GRA },	    //  5
	{ "g-4e-a.bin",           0x02000, 0xfbe9276e, BRF_GRA },	    //  6
	
	{ "snx_b-4k-b",           0x04000, 0x85591db2, BRF_GRA },	    //  7	Sprites
	{ "snx_b-4f-b",           0x04000, 0xed719d7b, BRF_GRA },	    //  8
	{ "snx_b-3n-b",           0x04000, 0x05fcce8b, BRF_GRA },	    //  9
	{ "snx_b-4n-b",           0x04000, 0xdc675003, BRF_GRA },	    //  10
	{ "snx_b-4c-b",           0x04000, 0x1df11d81, BRF_GRA },	    //  11
	{ "snx_b-4e-b",           0x04000, 0x2d3b69dd, BRF_GRA },	    //  12	
	
	{ "g-1j-.bin",            0x00100, 0x668e6bca, BRF_GRA },	    //  13	PROM (Tile Palette Red Component)
	{ "b-1m-.bin",            0x00100, 0x76c05a9c, BRF_GRA },	    //  14	PROM (Sprite Palette Red Component)
	{ "g-1f-.bin",            0x00100, 0x964b6495, BRF_GRA },	    //  15	PROM (Tile Palette Green Component)
	{ "b-1n-.bin",            0x00100, 0x23f06b99, BRF_GRA },	    //  16	PROM (Sprite Palette Green Component)
	{ "g-1h-.bin",            0x00100, 0x550563e1, BRF_GRA },	    //  17	PROM (Tile Palette Blue Component)
	{ "b-1l-.bin",            0x00100, 0x35e45021, BRF_GRA },	    //  18	PROM (Sprite Palette Blue Component)
	{ "b-5f-.bin",            0x00020, 0x7a601c3d, BRF_GRA },	    //  19	PROM (Sprite Height)
	{ "b-6f-.bin",            0x00100, 0x82c20d12, BRF_GRA },	    //  20	PROM (Video Timing)
};

STD_ROM_PICK(Kungfumd)
STD_ROM_FN(Kungfumd)

static struct BurnRomInfo SpartanxRomDesc[] = {
	{ "a-4e-c-j.bin",         0x04000, 0x32a0a9a6, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "a-4d-c-j.bin",         0x04000, 0x3173ea78, BRF_ESS | BRF_PRG }, //  1
	
	{ "a-3e-.bin",            0x02000, 0x58e87ab0, BRF_ESS | BRF_PRG }, //  2	M6803 Program Code
	{ "a-3f-.bin",            0x02000, 0xc81e31ea, BRF_ESS | BRF_PRG }, //  3
	{ "a-3h-.bin",            0x02000, 0xd99fb995, BRF_ESS | BRF_PRG }, //  4
	
	{ "g-4c-a-j.bin",         0x02000, 0x8af9c5a6, BRF_GRA },	    //  5	Characters
	{ "g-4d-a-j.bin",         0x02000, 0xb8300c72, BRF_GRA },	    //  6
	{ "g-4e-a-j.bin",         0x02000, 0xb50429cd, BRF_GRA },	    //  7
	
	{ "b-4k-.bin",            0x02000, 0x16fb5150, BRF_GRA },	    //  8	Sprites
	{ "b-4f-.bin",            0x02000, 0x67745a33, BRF_GRA },	    //  9
	{ "b-4l-.bin",            0x02000, 0xbd1c2261, BRF_GRA },	    //  10
	{ "b-4h-.bin",            0x02000, 0x8ac5ed3a, BRF_GRA },	    //  11
	{ "b-3n-.bin",            0x02000, 0x28a213aa, BRF_GRA },	    //  12
	{ "b-4n-.bin",            0x02000, 0xd5228df3, BRF_GRA },	    //  13
	{ "b-4m-.bin",            0x02000, 0xb16de4f2, BRF_GRA },	    //  14
	{ "b-3m-.bin",            0x02000, 0xeba0d66b, BRF_GRA },	    //  15
	{ "b-4c-.bin",            0x02000, 0x01298885, BRF_GRA },	    //  16
	{ "b-4e-.bin",            0x02000, 0xc77b87d4, BRF_GRA },	    //  17
	{ "b-4d-.bin",            0x02000, 0x6a70615f, BRF_GRA },	    //  18
	{ "b-4a-.bin",            0x02000, 0x6189d626, BRF_GRA },	    //  19
	
	{ "g-1j-.bin",            0x00100, 0x668e6bca, BRF_GRA },	    //  20	PROM (Tile Palette Red Component)
	{ "b-1m-.bin",            0x00100, 0x76c05a9c, BRF_GRA },	    //  21	PROM (Sprite Palette Red Component)
	{ "g-1f-.bin",            0x00100, 0x964b6495, BRF_GRA },	    //  22	PROM (Tile Palette Green Component)
	{ "b-1n-.bin",            0x00100, 0x23f06b99, BRF_GRA },	    //  23	PROM (Sprite Palette Green Component)
	{ "g-1h-.bin",            0x00100, 0x550563e1, BRF_GRA },	    //  24	PROM (Tile Palette Blue Component)
	{ "b-1l-.bin",            0x00100, 0x35e45021, BRF_GRA },	    //  25	PROM (Sprite Palette Blue Component)
	{ "b-5f-.bin",            0x00020, 0x7a601c3d, BRF_GRA },	    //  26	PROM (Sprite Height)
	{ "b-6f-.bin",            0x00100, 0x82c20d12, BRF_GRA },	    //  27	PROM (Video Timing)
};

STD_ROM_PICK(Spartanx)
STD_ROM_FN(Spartanx)

static struct BurnRomInfo KungfubRomDesc[] = {
	{ "c5.5h",                0x04000, 0x5d8e791d, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "c4.5k",                0x04000, 0x4000e2b8, BRF_ESS | BRF_PRG }, //  1
	
	{ "a-3e-.bin",            0x02000, 0x58e87ab0, BRF_ESS | BRF_PRG }, //  2	M6803 Program Code
	{ "a-3f-.bin",            0x02000, 0xc81e31ea, BRF_ESS | BRF_PRG }, //  3
	{ "a-3h-.bin",            0x02000, 0xd99fb995, BRF_ESS | BRF_PRG }, //  4
	
	{ "g-4c-a.bin",           0x02000, 0x6b2cc9c8, BRF_GRA },	    //  5	Characters
	{ "g-4d-a.bin",           0x02000, 0xc648f558, BRF_GRA },	    //  6
	{ "g-4e-a.bin",           0x02000, 0xfbe9276e, BRF_GRA },	    //  7
	
	{ "b-4k-.bin",            0x02000, 0x16fb5150, BRF_GRA },	    //  8	Sprites
	{ "b-4f-.bin",            0x02000, 0x67745a33, BRF_GRA },	    //  9
	{ "b-4l-.bin",            0x02000, 0xbd1c2261, BRF_GRA },	    //  10
	{ "b-4h-.bin",            0x02000, 0x8ac5ed3a, BRF_GRA },	    //  11
	{ "b-3n-.bin",            0x02000, 0x28a213aa, BRF_GRA },	    //  12
	{ "b-4n-.bin",            0x02000, 0xd5228df3, BRF_GRA },	    //  13
	{ "b-4m-.bin",            0x02000, 0xb16de4f2, BRF_GRA },	    //  14
	{ "b-3m-.bin",            0x02000, 0xeba0d66b, BRF_GRA },	    //  15
	{ "b-4c-.bin",            0x02000, 0x01298885, BRF_GRA },	    //  16
	{ "b-4e-.bin",            0x02000, 0xc77b87d4, BRF_GRA },	    //  17
	{ "b-4d-.bin",            0x02000, 0x6a70615f, BRF_GRA },	    //  18
	{ "b-4a-.bin",            0x02000, 0x6189d626, BRF_GRA },	    //  19
	
	{ "tbp24s10-main-1c.bin", 0x00100, 0x668e6bca, BRF_GRA },	    //  20	PROM (Tile Palette Red Component)
	{ "tbp24s10-gfx-1r.bin",  0x00100, 0x76c05a9c, BRF_GRA },	    //  21	PROM (Sprite Palette Red Component)
	{ "tbp24s10-main-1a.bin", 0x00100, 0x964b6495, BRF_GRA },	    //  22	PROM (Tile Palette Green Component)
	{ "tbp24s10-gfx-1s.bin",  0x00100, 0x23f06b99, BRF_GRA },	    //  23	PROM (Sprite Palette Green Component)
	{ "tbp24s10-main-1b.bin", 0x00100, 0x550563e1, BRF_GRA },	    //  24	PROM (Tile Palette Blue Component)
	{ "tbp24s10-gfx-1p.bin",  0x00100, 0x35e45021, BRF_GRA },	    //  25	PROM (Sprite Palette Blue Component)
	{ "18s030-gfx-8t.bin",    0x00020, 0x7a601c3d, BRF_GRA },	    //  26	PROM (Sprite Height)
	{ "tbp24s10-gfx-9k.bin",  0x00100, 0x82c20d12, BRF_GRA },	    //  27	PROM (Video Timing)
	{ "18s030-gfx-10a.bin",   0x00020, 0x3858acd0, BRF_OPT },	    //  27
	{ "18s030-gfx-5d.bin",    0x00020, 0x51304fcd, BRF_OPT },	    //  28
	{ "18s030-gfx-5e.bin",    0x00020, 0x51304fcd, BRF_OPT },	    //  29
	{ "18s030-gfx-6l.bin",    0x00020, 0x3858acd0, BRF_OPT },	    //  30
	{ "tbp24s10-gfx-3b.bin",  0x00100, 0xe6506ef4, BRF_OPT },	    //  31
	{ "tbp24s10-gfx-4a.bin",  0x00100, 0xe0aa8869, BRF_OPT },	    //  32
	{ "tbp24s10-gfx-4c.bin",  0x00100, 0xb43d094f, BRF_OPT },	    //  33
	{ "tbp24s10-gfx-6d.bin",  0x00100, 0x48bb39c9, BRF_OPT },	    //  34
	{ "tbp24s10-gfx-6e.bin",  0x00100, 0x48bb39c9, BRF_OPT },	    //  35
	{ "tbp24s10-gfx-6m.bin",  0x00100, 0x9f7a1a4d, BRF_OPT },	    //  36
	{ "tbp24s10-gfx-6n.bin",  0x00100, 0x35e5b39e, BRF_OPT },	    //  37
	{ "tbp24s10-gfx-8a.bin",  0x00100, 0x35e5b39e, BRF_OPT },	    //  38
	{ "tbp24s10-gfx-9a.bin",  0x00100, 0x9f7a1a4d, BRF_OPT },	    //  39
	{ "tbp24s10-gfx-9k.bin",  0x00100, 0x82c20d12, BRF_OPT },	    //  40
	{ "tbp24s10-main-8b.bin", 0x00100, 0x180fbc57, BRF_OPT },	    //  41
	{ "tbp24s10-main-8c.bin", 0x00100, 0x3bb32e5a, BRF_OPT },	    //  42
	{ "tbp24s10-main-8d.bin", 0x00100, 0x599c319f, BRF_OPT },	    //  43
};

STD_ROM_PICK(Kungfub)
STD_ROM_FN(Kungfub)

static struct BurnRomInfo Kungfub2RomDesc[] = {
	{ "kf4",                  0x04000, 0x3f65313f, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "kf5",                  0x04000, 0x9ea325f3, BRF_ESS | BRF_PRG }, //  1
	
	{ "a-3e-.bin",            0x02000, 0x58e87ab0, BRF_ESS | BRF_PRG }, //  2	M6803 Program Code
	{ "a-3f-.bin",            0x02000, 0xc81e31ea, BRF_ESS | BRF_PRG }, //  3
	{ "a-3h-.bin",            0x02000, 0xd99fb995, BRF_ESS | BRF_PRG }, //  4
	
	{ "g-4c-a.bin",           0x02000, 0x6b2cc9c8, BRF_GRA },	    //  5	Characters
	{ "g-4d-a.bin",           0x02000, 0xc648f558, BRF_GRA },	    //  6
	{ "g-4e-a.bin",           0x02000, 0xfbe9276e, BRF_GRA },	    //  7
	
	{ "b-4k-.bin",            0x02000, 0x16fb5150, BRF_GRA },	    //  8	Sprites
	{ "b-4f-.bin",            0x02000, 0x67745a33, BRF_GRA },	    //  9
	{ "b-4l-.bin",            0x02000, 0xbd1c2261, BRF_GRA },	    //  10
	{ "b-4h-.bin",            0x02000, 0x8ac5ed3a, BRF_GRA },	    //  11
	{ "b-3n-.bin",            0x02000, 0x28a213aa, BRF_GRA },	    //  12
	{ "b-4n-.bin",            0x02000, 0xd5228df3, BRF_GRA },	    //  13
	{ "b-4m-.bin",            0x02000, 0xb16de4f2, BRF_GRA },	    //  14
	{ "b-3m-.bin",            0x02000, 0xeba0d66b, BRF_GRA },	    //  15
	{ "b-4c-.bin",            0x02000, 0x01298885, BRF_GRA },	    //  16
	{ "b-4e-.bin",            0x02000, 0xc77b87d4, BRF_GRA },	    //  17
	{ "b-4d-.bin",            0x02000, 0x6a70615f, BRF_GRA },	    //  18
	{ "b-4a-.bin",            0x02000, 0x6189d626, BRF_GRA },	    //  19
	
	{ "g-1j-.bin",            0x00100, 0x668e6bca, BRF_GRA },	    //  20	PROM (Tile Palette Red Component)
	{ "b-1m-.bin",            0x00100, 0x76c05a9c, BRF_GRA },	    //  21	PROM (Sprite Palette Red Component)
	{ "g-1f-.bin",            0x00100, 0x964b6495, BRF_GRA },	    //  22	PROM (Tile Palette Green Component)
	{ "b-1n-.bin",            0x00100, 0x23f06b99, BRF_GRA },	    //  23	PROM (Sprite Palette Green Component)
	{ "g-1h-.bin",            0x00100, 0x550563e1, BRF_GRA },	    //  24	PROM (Tile Palette Blue Component)
	{ "b-1l-.bin",            0x00100, 0x35e45021, BRF_GRA },	    //  25	PROM (Sprite Palette Blue Component)
	{ "b-5f-.bin",            0x00020, 0x7a601c3d, BRF_GRA },	    //  26	PROM (Sprite Height)
	{ "b-6f-.bin",            0x00100, 0x82c20d12, BRF_GRA },	    //  27	PROM (Video Timing)
};

STD_ROM_PICK(Kungfub2)
STD_ROM_FN(Kungfub2)

static struct BurnRomInfo BattroadRomDesc[] = {
	{ "br-a-4e.b",            0x02000, 0x9bf14768, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "br-a-4d.b",            0x02000, 0x39ca1627, BRF_ESS | BRF_PRG }, //  1
	{ "br-a-4b.b",            0x02000, 0x1865bb22, BRF_ESS | BRF_PRG }, //  2
	{ "br-a-4a",              0x02000, 0x65b61c21, BRF_ESS | BRF_PRG }, //  3
	{ "br-c-7c",              0x02000, 0x2e1eca52, BRF_ESS | BRF_PRG }, //  4
	{ "br-c-7l",              0x02000, 0xf2178578, BRF_ESS | BRF_PRG }, //  5
	{ "br-c-7d",              0x02000, 0x3aa9fa30, BRF_ESS | BRF_PRG }, //  6
	{ "br-c-7b",              0x02000, 0x0b31b90b, BRF_ESS | BRF_PRG }, //  7
	{ "br-c-7a",              0x02000, 0xec3b0080, BRF_ESS | BRF_PRG }, //  8
	{ "br-c-7k",              0x02000, 0xedc75f7f, BRF_ESS | BRF_PRG }, //  9
	
	{ "br-a-3e",              0x02000, 0xa7140871, BRF_ESS | BRF_PRG }, //  10	M6803 Program Code
	{ "br-a-3f",              0x02000, 0x1bb51b30, BRF_ESS | BRF_PRG }, //  11
	{ "br-a-3h",              0x02000, 0xafb3e083, BRF_ESS | BRF_PRG }, //  12
	
	{ "br-c-6h",              0x02000, 0xca50841c, BRF_GRA },	    //  13	Tiles
	{ "br-c-6n",              0x02000, 0x7d53163a, BRF_GRA },	    //  14
	{ "br-c-6k",              0x02000, 0x5951e12a, BRF_GRA },	    //  15
	
	{ "br-b-4k.a",            0x02000, 0xd3c5e85b, BRF_GRA },	    //  16	Sprites
	{ "br-b-4f.a",            0x02000, 0x4354232a, BRF_GRA },	    //  17
	{ "br-b-3n.a",            0x02000, 0x2668dbef, BRF_GRA },	    //  18
	{ "br-b-4n.a",            0x02000, 0xc719a324, BRF_GRA },	    //  19
	{ "br-b-4c.a",            0x02000, 0x0b3193bf, BRF_GRA },	    //  20
	{ "br-b-4e.a",            0x02000, 0x3662e8fb, BRF_GRA },	    //  21
	
	{ "br-c-1b",              0x02000, 0x8088911e, BRF_GRA },	    //  22	Chars
	{ "br-c-1c",              0x02000, 0x3d78b653, BRF_GRA },	    //  23
	
	{ "br-c-3j",              0x00100, 0xaceaed79, BRF_GRA },	    //  24	PROM (Tile Palette Red Component)
	{ "br-b-1m",              0x00100, 0x3bd30c7d, BRF_GRA },	    //  25	PROM (Sprite Palette Red Component)
	{ "br-c-3l",              0x00100, 0x7cf6f380, BRF_GRA },	    //  26	PROM (Tile Palette Green Component)
	{ "br-b-1n",              0x00100, 0xb7f3dc3b, BRF_GRA },	    //  27	PROM (Sprite Palette Green Component)
	{ "br-c-3k",              0x00100, 0xd90e4a54, BRF_GRA },	    //  28	PROM (Tile Palette Blue Component)
	{ "br-b-1l",              0x00100, 0x5271c7d8, BRF_GRA },	    //  29	PROM (Sprite Palette Blue Component)
	{ "br-b-5p",              0x00020, 0xce746937, BRF_GRA },	    //  30	PROM (Sprite Height)
	{ "br-b-6f",              0x00100, 0x82c20d12, BRF_GRA },	    //  31	PROM (Video Timing)
	{ "br-c-1j",              0x00020, 0x78eb5d77, BRF_GRA },	    //  32	PROM (Char Palette)
};

STD_ROM_PICK(Battroad)
STD_ROM_FN(Battroad)

static struct BurnRomInfo LdrunRomDesc[] = {
	{ "lr-a-4e",              0x02000, 0x5d7e2a4d, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "lr-a-4d",              0x02000, 0x96f20473, BRF_ESS | BRF_PRG }, //  1
	{ "lr-a-4b",              0x02000, 0xb041c4a9, BRF_ESS | BRF_PRG }, //  2
	{ "lr-a-4a",              0x02000, 0x645e42aa, BRF_ESS | BRF_PRG }, //  3
	
	{ "lr-a-3f",              0x02000, 0x7a96accd, BRF_ESS | BRF_PRG }, //  4	M6803 Program Code
	{ "lr-a-3h",              0x02000, 0x3f7f3939, BRF_ESS | BRF_PRG }, //  5
	
	{ "lr-e-2d",              0x02000, 0x24f9b58d, BRF_GRA },	    //  6	Characters
	{ "lr-e-2j",              0x02000, 0x43175e08, BRF_GRA },	    //  7
	{ "lr-e-2f",              0x02000, 0xe0317124, BRF_GRA },	    //  8
	
	{ "lr-b-4k",              0x02000, 0x8141403e, BRF_GRA },	    //  9	Sprites
	{ "lr-b-3n",              0x02000, 0x55154154, BRF_GRA },	    //  10
	{ "lr-b-4c",              0x02000, 0x924e34d0, BRF_GRA },	    //  11
	
	{ "lr-e-3m",              0x00100, 0x53040416, BRF_GRA },	    //  20	PROM (Tile Palette Red Component)
	{ "lr-b-1m",              0x00100, 0x4bae1c25, BRF_GRA },	    //  21	PROM (Sprite Palette Red Component)
	{ "lr-e-3l",              0x00100, 0x67786037, BRF_GRA },	    //  22	PROM (Tile Palette Green Component)
	{ "lr-b-1n",              0x00100, 0x9cd3db94, BRF_GRA },	    //  23	PROM (Sprite Palette Green Component)
	{ "lr-e-3n",              0x00100, 0x5b716837, BRF_GRA },	    //  24	PROM (Tile Palette Blue Component)
	{ "lr-b-1l",              0x00100, 0x08d8cf9a, BRF_GRA },	    //  25	PROM (Sprite Palette Blue Component)
	{ "lr-b-5p",              0x00020, 0xe01f69e2, BRF_GRA },	    //  26	PROM (Sprite Height)
	{ "lr-b-6f",              0x00100, 0x34d88d3c, BRF_GRA },	    //  27	PROM (Video Timing)
};

STD_ROM_PICK(Ldrun)
STD_ROM_FN(Ldrun)

static struct BurnRomInfo LdrunaRomDesc[] = {
	{ "roma4c",               0x02000, 0x279421e1, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "lr-a-4d",              0x02000, 0x96f20473, BRF_ESS | BRF_PRG }, //  1
	{ "roma4b",               0x02000, 0x3c464bad, BRF_ESS | BRF_PRG }, //  2
	{ "roma4a",               0x02000, 0x899df8e0, BRF_ESS | BRF_PRG }, //  3
	
	{ "lr-a-3f",              0x02000, 0x7a96accd, BRF_ESS | BRF_PRG }, //  4	M6803 Program Code
	{ "lr-a-3h",              0x02000, 0x3f7f3939, BRF_ESS | BRF_PRG }, //  5
	
	{ "lr-e-2d",              0x02000, 0x24f9b58d, BRF_GRA },	    //  6	Characters
	{ "lr-e-2j",              0x02000, 0x43175e08, BRF_GRA },	    //  7
	{ "lr-e-2f",              0x02000, 0xe0317124, BRF_GRA },	    //  8
	
	{ "lr-b-4k",              0x02000, 0x8141403e, BRF_GRA },	    //  9	Sprites
	{ "lr-b-3n",              0x02000, 0x55154154, BRF_GRA },	    //  10
	{ "lr-b-4c",              0x02000, 0x924e34d0, BRF_GRA },	    //  11
	
	{ "lr-e-3m",              0x00100, 0x53040416, BRF_GRA },	    //  20	PROM (Tile Palette Red Component)
	{ "lr-b-1m",              0x00100, 0x4bae1c25, BRF_GRA },	    //  21	PROM (Sprite Palette Red Component)
	{ "lr-e-3l",              0x00100, 0x67786037, BRF_GRA },	    //  22	PROM (Tile Palette Green Component)
	{ "lr-b-1n",              0x00100, 0x9cd3db94, BRF_GRA },	    //  23	PROM (Sprite Palette Green Component)
	{ "lr-e-3n",              0x00100, 0x5b716837, BRF_GRA },	    //  24	PROM (Tile Palette Blue Component)
	{ "lr-b-1l",              0x00100, 0x08d8cf9a, BRF_GRA },	    //  25	PROM (Sprite Palette Blue Component)
	{ "lr-b-5p",              0x00020, 0xe01f69e2, BRF_GRA },	    //  26	PROM (Sprite Height)
	{ "lr-b-6f",              0x00100, 0x34d88d3c, BRF_GRA },	    //  27	PROM (Video Timing)
};

STD_ROM_PICK(Ldruna)
STD_ROM_FN(Ldruna)

static struct BurnRomInfo Ldrun2RomDesc[] = {
	{ "lr2-a-4e.a",           0x02000, 0x22313327, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "lr2-a-4d",             0x02000, 0xef645179, BRF_ESS | BRF_PRG }, //  1
	{ "lr2-a-4a.a",           0x02000, 0xb11ddf59, BRF_ESS | BRF_PRG }, //  2
	{ "lr2-a-4a",             0x02000, 0x470cc8a1, BRF_ESS | BRF_PRG }, //  3
	{ "lr2-h-1c.a",           0x02000, 0x7ebcadbc, BRF_ESS | BRF_PRG }, //  4
	{ "lr2-h-1d.a",           0x02000, 0x64cbb7f9, BRF_ESS | BRF_PRG }, //  5
	
	{ "lr2-a-3e",             0x02000, 0x853f3898, BRF_ESS | BRF_PRG }, //  6	M6803 Program Code
	{ "lr2-a-3f",             0x02000, 0x7a96accd, BRF_ESS | BRF_PRG }, //  7
	{ "lr2-a-3h",             0x02000, 0x2a0e83ca, BRF_ESS | BRF_PRG }, //  8
	
	{ "lr2-h-1e",             0x02000, 0x9d63a8ff, BRF_GRA },	    //  9	Characters
	{ "lr2-h-1j",             0x02000, 0x40332bbd, BRF_GRA },	    //  10
	{ "lr2-h-1h",             0x02000, 0x9404727d, BRF_GRA },	    //  11
	
	{ "lr2-b-4k",             0x02000, 0x79909871, BRF_GRA },	    //  12	Sprites
	{ "lr2-b-4f",             0x02000, 0x06ba1ef4, BRF_GRA },	    //  13
	{ "lr2-b-3n",             0x02000, 0x3cc5893f, BRF_GRA },	    //  14
	{ "lr2-b-4n",             0x02000, 0x49c12f42, BRF_GRA },	    //  15
	{ "lr2-b-4c",             0x02000, 0xfbe6d24c, BRF_GRA },	    //  16
	{ "lr2-b-4e",             0x02000, 0x75172d1f, BRF_GRA },	    //  17
	
	{ "lr2-h-3m",             0x00100, 0x2c5d834b, BRF_GRA },	    //  18	PROM (Tile Palette Red Component)
	{ "lr2-b-1m",             0x00100, 0x4ec9bb3d, BRF_GRA },	    //  19	PROM (Sprite Palette Red Component)
	{ "lr2-h-3l",             0x00100, 0x3ae69aca, BRF_GRA },	    //  20	PROM (Tile Palette Green Component)
	{ "lr2-b-1n",             0x00100, 0x1daf1fa4, BRF_GRA },	    //  21	PROM (Sprite Palette Green Component)
	{ "lr2-h-3n",             0x00100, 0x2b28aec5, BRF_GRA },	    //  22	PROM (Tile Palette Blue Component)
	{ "lr2-b-1l",             0x00100, 0xc8fb708a, BRF_GRA },	    //  23	PROM (Sprite Palette Blue Component)
	{ "lr2-b-5p",             0x00020, 0xe01f69e2, BRF_GRA },	    //  24	PROM (Sprite Height)
	{ "lr2-b-6f",             0x00100, 0x34d88d3c, BRF_GRA },	    //  25	PROM (Video Timing)
};

STD_ROM_PICK(Ldrun2)
STD_ROM_FN(Ldrun2)

static struct BurnRomInfo Ldrun3RomDesc[] = {
	{ "lr3a4eb.bin",          0x04000, 0x09affc47, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "lr3a4db.bin",          0x04000, 0x23a02178, BRF_ESS | BRF_PRG }, //  1
	{ "lr3a4bb.bin",          0x04000, 0x3d501a1a, BRF_ESS | BRF_PRG }, //  2
	
	{ "lr3-a-3d",             0x04000, 0x28be68cd, BRF_ESS | BRF_PRG }, //  3	M6803 Program Code
	{ "lr3-a-3f",             0x04000, 0xcb7186b7, BRF_ESS | BRF_PRG }, //  4
	
	{ "lr3-n-2a",             0x04000, 0xf9b74dee, BRF_GRA },	    //  5	Characters
	{ "lr3-n-2c",             0x04000, 0xfef707ba, BRF_GRA },	    //  6
	{ "lr3-n-2b",             0x04000, 0xaf3d27b9, BRF_GRA },	    //  7
	
	{ "lr3b4kb.bin",          0x04000, 0x21ecd8c5, BRF_GRA },	    //  8	Sprites
	{ "snxb4fb.bin",          0x04000, 0xed719d7b, BRF_GRA },	    //  9
	{ "lr3b3nb.bin",          0x04000, 0xda8cffab, BRF_GRA },	    //  10
	{ "snxb4nb.bin",          0x04000, 0xdc675003, BRF_GRA },	    //  11
	{ "snxb4cb.bin",          0x04000, 0x585aa244, BRF_GRA },	    //  12
	{ "snxb4eb.bin",          0x04000, 0x2d3b69dd, BRF_GRA },	    //  13
	
	{ "lr3-n-2l",             0x00100, 0xe880b86b, BRF_GRA },	    //  14	PROM (Tile Palette Red Component)
	{ "lr3-b-1m",             0x00100, 0xf02d7167, BRF_GRA },	    //  15	PROM (Sprite Palette Red Component)
	{ "lr3-n-2k",             0x00100, 0x047ee051, BRF_GRA },	    //  16	PROM (Tile Palette Green Component)
	{ "lr3-b-1n",             0x00100, 0x9e37f181, BRF_GRA },	    //  17	PROM (Sprite Palette Green Component)
	{ "lr3-n-2m",             0x00100, 0x69ad8678, BRF_GRA },	    //  18	PROM (Tile Palette Blue Component)
	{ "lr3-b-1l",             0x00100, 0x5b11c41d, BRF_GRA },	    //  19	PROM (Sprite Palette Blue Component)
	{ "lr3-b-5p",             0x00020, 0xe01f69e2, BRF_GRA },	    //  20	PROM (Sprite Height)
	{ "lr3-b-6f",             0x00100, 0x34d88d3c, BRF_GRA },	    //  21	PROM (Video Timing)
	{ "lr3-n-4f",             0x00100, 0xdf674be9, BRF_OPT },	    //  22	PROM (Unknown)
};

STD_ROM_PICK(Ldrun3)
STD_ROM_FN(Ldrun3)

static struct BurnRomInfo Ldrun3jRomDesc[] = {
	{ "lr3-a-4e",             0x04000, 0x5b334e8e, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "lr3-a-4d.a",           0x04000, 0xa84bc931, BRF_ESS | BRF_PRG }, //  1
	{ "lr3-a-4b.a",           0x04000, 0xbe09031d, BRF_ESS | BRF_PRG }, //  2
	
	{ "lr3-a-3d",             0x04000, 0x28be68cd, BRF_ESS | BRF_PRG }, //  3	M6803 Program Code
	{ "lr3-a-3f",             0x04000, 0xcb7186b7, BRF_ESS | BRF_PRG }, //  4
	
	{ "lr3-n-2a",             0x04000, 0xf9b74dee, BRF_GRA },	    //  5	Characters
	{ "lr3-n-2c",             0x04000, 0xfef707ba, BRF_GRA },	    //  6
	{ "lr3-n-2b",             0x04000, 0xaf3d27b9, BRF_GRA },	    //  7
	
	{ "lr3-b-4k",             0x04000, 0x63f070c7, BRF_GRA },	    //  8	Sprites
	{ "lr3-b-3n",             0x04000, 0xeab7ad91, BRF_GRA },	    //  9
	{ "lr3-b-4c",             0x04000, 0x1a460a46, BRF_GRA },	    //  10
	
	{ "lr3-n-2l",             0x00100, 0xe880b86b, BRF_GRA },	    //  11	PROM (Tile Palette Red Component)
	{ "lr3-b-1m",             0x00100, 0xf02d7167, BRF_GRA },	    //  12	PROM (Sprite Palette Red Component)
	{ "lr3-n-2k",             0x00100, 0x047ee051, BRF_GRA },	    //  13	PROM (Tile Palette Green Component)
	{ "lr3-b-1n",             0x00100, 0x9e37f181, BRF_GRA },	    //  14	PROM (Sprite Palette Green Component)
	{ "lr3-n-2m",             0x00100, 0x69ad8678, BRF_GRA },	    //  15	PROM (Tile Palette Blue Component)
	{ "lr3-b-1l",             0x00100, 0x5b11c41d, BRF_GRA },	    //  16	PROM (Sprite Palette Blue Component)
	{ "lr3-b-5p",             0x00020, 0xe01f69e2, BRF_GRA },	    //  17	PROM (Sprite Height)
	{ "lr3-b-6f",             0x00100, 0x34d88d3c, BRF_GRA },	    //  18	PROM (Video Timing)
	{ "lr3-n-4f",             0x00100, 0xdf674be9, BRF_OPT },	    //  19	PROM (Unknown)
};

STD_ROM_PICK(Ldrun3j)
STD_ROM_FN(Ldrun3j)

static struct BurnRomInfo Ldrun4RomDesc[] = {
	{ "lr4-a-4e",             0x04000, 0x5383e9bf, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "lr4-a-4d.c",           0x04000, 0x298afa36, BRF_ESS | BRF_PRG }, //  1
	{ "lr4-v-4k",             0x08000, 0x8b248abd, BRF_ESS | BRF_PRG }, //  2
	
	{ "lr4-a-3d",             0x04000, 0x86c6d445, BRF_ESS | BRF_PRG }, //  3	M6803 Program Code
	{ "lr4-a-3f",             0x04000, 0x097c6c0a, BRF_ESS | BRF_PRG }, //  4
	
	{ "lr4-v-2b",             0x04000, 0x4118e60a, BRF_GRA },	    //  5	Characters
	{ "lr4-v-2d",             0x04000, 0x542bb5b5, BRF_GRA },	    //  6
	{ "lr4-v-2c",             0x04000, 0xc765266c, BRF_GRA },	    //  7
	
	{ "lr4-b-4k",             0x04000, 0xe7fe620c, BRF_GRA },	    //  8	Sprites
	{ "lr4-b-4f",             0x04000, 0x6f0403db, BRF_GRA },	    //  9
	{ "lr4-b-3n",             0x04000, 0xad1fba1b, BRF_GRA },	    //  10
	{ "lr4-b-4n",             0x04000, 0x0e568fab, BRF_GRA },	    //  11
	{ "lr4-b-4c",             0x04000, 0x82c53669, BRF_GRA },	    //  12
	{ "lr4-b-4e",             0x04000, 0x767a1352, BRF_GRA },	    //  13
	
	{ "lr4-v-1m",             0x00100, 0xfe51bf1d, BRF_GRA },	    //  14	PROM (Tile Palette Red Component)
	{ "lr4-b-1m",             0x00100, 0x5d8d17d0, BRF_GRA },	    //  15	PROM (Sprite Palette Red Component)
	{ "lr4-v-1n",             0x00100, 0xda0658e5, BRF_GRA },	    //  16	PROM (Tile Palette Green Component)
	{ "lr4-b-1n",             0x00100, 0xda1129d2, BRF_GRA },	    //  17	PROM (Sprite Palette Green Component)
	{ "lr4-v-1p",             0x00100, 0x0df23ebe, BRF_GRA },	    //  18	PROM (Tile Palette Blue Component)
	{ "lr4-b-1l",             0x00100, 0x0d89b692, BRF_GRA },	    //  19	PROM (Sprite Palette Blue Component)
	{ "lr4-b-5p",             0x00020, 0xe01f69e2, BRF_GRA },	    //  20	PROM (Sprite Height)
	{ "lr4-b-6f",             0x00100, 0x34d88d3c, BRF_GRA },	    //  21	PROM (Video Timing)
	{ "lr4-v-4h",             0x00100, 0xdf674be9, BRF_OPT },	    //  22	PROM (Unknown)
};

STD_ROM_PICK(Ldrun4)
STD_ROM_FN(Ldrun4)

static struct BurnRomInfo LotlotRomDesc[] = {
	{ "lot-a-4e",             0x04000, 0x2913d08f, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "lot-a-4d",             0x04000, 0x0443095f, BRF_ESS | BRF_PRG }, //  1
	
	{ "lot-a-3h",             0x02000, 0x0781cee7, BRF_ESS | BRF_PRG }, //  2	M6803 Program Code
	
	{ "lot-k-4a",             0x02000, 0x1b3695f4, BRF_GRA },	    //  3	Tiles
	{ "lot-k-4c",             0x02000, 0xbd2b0730, BRF_GRA },	    //  4
	{ "lot-k-4b",             0x02000, 0x930ddd55, BRF_GRA },	    //  5
	
	{ "lot-b-4k",             0x02000, 0xfd27cb90, BRF_GRA },	    //  6	Sprites
	{ "lot-b-3n",             0x02000, 0xbd486fff, BRF_GRA },	    //  7
	{ "lot-b-4c",             0x02000, 0x3026ee6c, BRF_GRA },	    //  8
	
	{ "lot-k-4p",             0x02000, 0x3b7d95ba, BRF_GRA },	    //  9	Characters
	{ "lot-k-4l",             0x02000, 0xf98dca1f, BRF_GRA },	    //  10
	{ "lot-k-4n",             0x02000, 0xf0cd76a5, BRF_GRA },	    //  11
	
	{ "lot-k-2f",             0x00100, 0xb820a05e, BRF_GRA },	    //  12	PROM (Tile Palette Red Component)
	{ "lot-b-1m",             0x00100, 0xc146461d, BRF_GRA },	    //  13	PROM (Sprite Palette Red Component)
	{ "lot-k-2l",             0x00100, 0xac3e230d, BRF_GRA },	    //  14	PROM (Char Palette Red Component)
	{ "lot-k-2e",             0x00100, 0x9b1fa005, BRF_GRA },	    //  15	PROM (Tile Palette Green Component)
	{ "lot-b-1n",             0x00100, 0x01e07db6, BRF_GRA },	    //  16	PROM (Sprite Palette Green Component)
	{ "lot-k-2k",             0x00100, 0x1811ad2b, BRF_GRA },	    //  17	PROM (Char Palette Green Component)
	{ "lot-k-2d",             0x00100, 0x315ed9a8, BRF_GRA },	    //  18	PROM (Tile Palette Blue Component)
	{ "lot-b-1l",             0x00100, 0x8b6fcde3, BRF_GRA },	    //  19	PROM (Sprite Palette Blue Component)
	{ "lot-k-2j",             0x00100, 0xe791ef2a, BRF_GRA },	    //  20	PROM (Char Palette Blue Component)
	{ "lot-b-5p",             0x00020, 0x110b21fd, BRF_GRA },	    //  21	PROM (Sprite Height)
	{ "lot-b-6f",             0x00100, 0x34d88d3c, BRF_GRA },	    //  22	PROM (Video Timing)
	{ "lot-k-7e",             0x00200, 0x6cef0fbd, BRF_GRA },	    //  23	PROM (Unknown)
	{ "lot-k-7h",             0x00200, 0x04442bee, BRF_GRA },	    //  24	PROM (Unknown)
};

STD_ROM_PICK(Lotlot)
STD_ROM_FN(Lotlot)

static struct BurnRomInfo KidnikiRomDesc[] = {
	{ "ky_a-4e-g.bin",        0x04000, 0x2edcbcd7, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "dr03.4cd",             0x04000, 0xdba20934, BRF_ESS | BRF_PRG }, //  1
	{ "ky_t-8k-g.bin",        0x08000, 0xdbc42f31, BRF_ESS | BRF_PRG }, //  2
	{ "dr12.8l",              0x10000, 0xc0b255fd, BRF_ESS | BRF_PRG }, //  3
	
	{ "dr00.3a",              0x04000, 0x458309f7, BRF_ESS | BRF_PRG }, //  4	M6803 Program Code
	{ "dr01.3cd",             0x04000, 0xe66897bd, BRF_ESS | BRF_PRG }, //  5
	{ "dr02.3f",              0x04000, 0xf9e31e26, BRF_ESS | BRF_PRG }, //  6
	
	{ "dr06.2b",              0x08000, 0x4d9a970f, BRF_GRA },	    //  7	Tiles
	{ "dr07.2dc",             0x08000, 0xab59a4c4, BRF_GRA },	    //  8
	{ "dr05.2a",              0x08000, 0x2e6dad0c, BRF_GRA },	    //  9
	
	{ "dr21.4k",              0x04000, 0xa06cea9a, BRF_GRA },	    //  10	Sprites
	{ "dr19.4f",              0x04000, 0xb34605ad, BRF_GRA },	    //  11
	{ "dr22.4l",              0x04000, 0x41303de8, BRF_GRA },	    //  12
	{ "dr20.4jh",             0x04000, 0x5fbe6f61, BRF_GRA },	    //  13
	{ "dr14.3p",              0x04000, 0x76cfbcbc, BRF_GRA },	    //  14
	{ "dr24.4p",              0x04000, 0xd51c8db5, BRF_GRA },	    //  15
	{ "dr23.4nm",             0x04000, 0x03469df8, BRF_GRA },	    //  16
	{ "dr13.3nm",             0x04000, 0xd5c3dfe0, BRF_GRA },	    //  17
	{ "dr16.4cb",             0x04000, 0xf1d1bb93, BRF_GRA },	    //  18
	{ "dr18.4e",              0x04000, 0xedb7f25b, BRF_GRA },	    //  19
	{ "dr17.4dc",             0x04000, 0x4fb87868, BRF_GRA },	    //  20
	{ "dr15.4a",              0x04000, 0xe0b88de5, BRF_GRA },	    //  21
	
	{ "dr08.4l",              0x04000, 0x32d50643, BRF_GRA },	    //  22	Characters
	{ "dr09.4m",              0x04000, 0x17df6f95, BRF_GRA },	    //  23
	{ "dr10.4n",              0x04000, 0x820ce252, BRF_GRA },	    //  24

	{ "dr25.3f",              0x00100, 0x8e91430b, BRF_GRA },	    //  25	PROM (Tile Palette Red Component)
	{ "dr30.1m",              0x00100, 0x28c73263, BRF_GRA },	    //  26	PROM (Sprite Palette Red Component)
	{ "dr26.3h",              0x00100, 0xb563b93f, BRF_GRA },	    //  27	PROM (Tile Palette Green Component)
	{ "dr31.1n",              0x00100, 0x3529210e, BRF_GRA },	    //  28	PROM (Sprite Palette Green Component)
	{ "dr27.3j",              0x00100, 0x70d668ef, BRF_GRA },	    //  29	PROM (Tile Palette Blue Component)
	{ "dr29.1l",              0x00100, 0x1173a754, BRF_GRA },	    //  30	PROM (Sprite Palette Blue Component)
	{ "dr32.5p",              0x00020, 0x11cd1f2e, BRF_GRA },	    //  31	PROM (Sprite Height)
	{ "dr33.6f",              0x00100, 0x34d88d3c, BRF_GRA },	    //  32	PROM (Video Timing)
	{ "dr28.8f",              0x00200, 0x6cef0fbd, BRF_OPT },	    //  33	PROM (Unknown)
};

STD_ROM_PICK(Kidniki)
STD_ROM_FN(Kidniki)

static struct BurnRomInfo KidnikiuRomDesc[] = {
	{ "dr04.4e",              0x04000, 0x80431858, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "dr03.4cd",             0x04000, 0xdba20934, BRF_ESS | BRF_PRG }, //  1
	{ "dr11.8k",              0x08000, 0x04d82d93, BRF_ESS | BRF_PRG }, //  2
	{ "dr12.8l",              0x10000, 0xc0b255fd, BRF_ESS | BRF_PRG }, //  3
	
	{ "dr00.3a",              0x04000, 0x458309f7, BRF_ESS | BRF_PRG }, //  4	M6803 Program Code
	{ "dr01.3cd",             0x04000, 0xe66897bd, BRF_ESS | BRF_PRG }, //  5
	{ "dr02.3f",              0x04000, 0xf9e31e26, BRF_ESS | BRF_PRG }, //  6
	
	{ "dr06.2b",              0x08000, 0x4d9a970f, BRF_GRA },	    //  7	Tiles
	{ "dr07.2dc",             0x08000, 0xab59a4c4, BRF_GRA },	    //  8
	{ "dr05.2a",              0x08000, 0x2e6dad0c, BRF_GRA },	    //  9
	
	{ "dr21.4k",              0x04000, 0xa06cea9a, BRF_GRA },	    //  10	Sprites
	{ "dr19.4f",              0x04000, 0xb34605ad, BRF_GRA },	    //  11
	{ "dr22.4l",              0x04000, 0x41303de8, BRF_GRA },	    //  12
	{ "dr20.4jh",             0x04000, 0x5fbe6f61, BRF_GRA },	    //  13
	{ "dr14.3p",              0x04000, 0x76cfbcbc, BRF_GRA },	    //  14
	{ "dr24.4p",              0x04000, 0xd51c8db5, BRF_GRA },	    //  15
	{ "dr23.4nm",             0x04000, 0x03469df8, BRF_GRA },	    //  16
	{ "dr13.3nm",             0x04000, 0xd5c3dfe0, BRF_GRA },	    //  17
	{ "dr16.4cb",             0x04000, 0xf1d1bb93, BRF_GRA },	    //  18
	{ "dr18.4e",              0x04000, 0xedb7f25b, BRF_GRA },	    //  19
	{ "dr17.4dc",             0x04000, 0x4fb87868, BRF_GRA },	    //  20
	{ "dr15.4a",              0x04000, 0xe0b88de5, BRF_GRA },	    //  21
	
	{ "dr08.4l",              0x04000, 0x32d50643, BRF_GRA },	    //  22	Characters
	{ "dr09.4m",              0x04000, 0x17df6f95, BRF_GRA },	    //  23
	{ "dr10.4n",              0x04000, 0x820ce252, BRF_GRA },	    //  24

	{ "dr25.3f",              0x00100, 0x8e91430b, BRF_GRA },	    //  25	PROM (Tile Palette Red Component)
	{ "dr30.1m",              0x00100, 0x28c73263, BRF_GRA },	    //  26	PROM (Sprite Palette Red Component)
	{ "dr26.3h",              0x00100, 0xb563b93f, BRF_GRA },	    //  27	PROM (Tile Palette Green Component)
	{ "dr31.1n",              0x00100, 0x3529210e, BRF_GRA },	    //  28	PROM (Sprite Palette Green Component)
	{ "dr27.3j",              0x00100, 0x70d668ef, BRF_GRA },	    //  29	PROM (Tile Palette Blue Component)
	{ "dr29.1l",              0x00100, 0x1173a754, BRF_GRA },	    //  30	PROM (Sprite Palette Blue Component)
	{ "dr32.5p",              0x00020, 0x11cd1f2e, BRF_GRA },	    //  31	PROM (Sprite Height)
	{ "dr33.6f",              0x00100, 0x34d88d3c, BRF_GRA },	    //  32	PROM (Video Timing)
	{ "dr28.8f",              0x00200, 0x6cef0fbd, BRF_OPT },	    //  33	PROM (Unknown)
};

STD_ROM_PICK(Kidnikiu)
STD_ROM_FN(Kidnikiu)

static struct BurnRomInfo YanchamrRomDesc[] = {
	{ "ky_a-4e-.bin",         0x04000, 0xc73ad2d6, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "ky_a-4d-.bin",         0x04000, 0x401af828, BRF_ESS | BRF_PRG }, //  1
	{ "ky_t-8k-.bin",         0x08000, 0xe967de88, BRF_ESS | BRF_PRG }, //  2
	{ "ky_t-8l-.bin",         0x08000, 0xa929110b, BRF_ESS | BRF_PRG }, //  3
	
	{ "ky_a-3a-.bin",         0x04000, 0xcb365f3b, BRF_ESS | BRF_PRG }, //  4	M6803 Program Code
	{ "dr01.3cd",             0x04000, 0xe66897bd, BRF_ESS | BRF_PRG }, //  5
	{ "dr02.3f",              0x04000, 0xf9e31e26, BRF_ESS | BRF_PRG }, //  6
	
	{ "ky_t-2c-.bin",         0x08000, 0xcb9761fc, BRF_GRA },	    //  7	Tiles
	{ "ky_t-2d-.bin",         0x08000, 0x59732741, BRF_GRA },	    //  8
	{ "ky_t-2a-.bin",         0x08000, 0x0370fd82, BRF_GRA },	    //  9

	{ "ky_b-4k-.bin",         0x04000, 0x263a9d10, BRF_GRA },	    //  10	Sprites
	{ "ky_b-4f-.bin",         0x04000, 0x86e3d4a8, BRF_GRA },	    //  11
	{ "ky_b-4l-.bin",         0x04000, 0x19fa7558, BRF_GRA },	    //  12
	{ "ky_b-4h-.bin",         0x04000, 0x93e6665c, BRF_GRA },	    //  13
	{ "ky_b-3n-.bin",         0x04000, 0x0287c525, BRF_GRA },	    //  14
	{ "ky_b-4n-.bin",         0x04000, 0x764946e0, BRF_GRA },	    //  15
	{ "ky_b-4m-.bin",         0x04000, 0xeced5db9, BRF_GRA },	    //  16
	{ "ky_b-3m-.bin",         0x04000, 0xbe6cee44, BRF_GRA },	    //  17
	{ "ky_b-4c-.bin",         0x04000, 0x84d6b65d, BRF_GRA },	    //  18
	{ "ky_b-4e-.bin",         0x04000, 0xf91f9273, BRF_GRA },	    //  19
	{ "ky_b-4d-.bin",         0x04000, 0xa2fc15f0, BRF_GRA },	    //  20
	{ "ky_b-4a-.bin",         0x04000, 0xff2b9c8a, BRF_GRA },	    //  21

	{ "ky_t-4l-.bin",         0x04000, 0x1d0a9253, BRF_GRA },	    //  22	Characters
	{ "ky_t-4m-.bin",         0x04000, 0x4075c396, BRF_GRA },	    //  23
	{ "ky_t-4n-.bin",         0x04000, 0x7564f2ff, BRF_GRA },	    //  24

	{ "dr25.3f",              0x00100, 0x8e91430b, BRF_GRA },	    //  25	PROM (Tile Palette Red Component)
	{ "dr30.1m",              0x00100, 0x28c73263, BRF_GRA },	    //  26	PROM (Sprite Palette Red Component)
	{ "dr26.3h",              0x00100, 0xb563b93f, BRF_GRA },	    //  27	PROM (Tile Palette Green Component)
	{ "dr31.1n",              0x00100, 0x3529210e, BRF_GRA },	    //  28	PROM (Sprite Palette Green Component)
	{ "dr27.3j",              0x00100, 0x70d668ef, BRF_GRA },	    //  29	PROM (Tile Palette Blue Component)
	{ "dr29.1l",              0x00100, 0x1173a754, BRF_GRA },	    //  30	PROM (Sprite Palette Blue Component)
	{ "dr32.5p",              0x00020, 0x11cd1f2e, BRF_GRA },	    //  31	PROM (Sprite Height)
	{ "dr33.6f",              0x00100, 0x34d88d3c, BRF_GRA },	    //  32	PROM (Video Timing)
	{ "dr28.8f",              0x00200, 0x6cef0fbd, BRF_OPT },	    //  33	PROM (Unknown)
};

STD_ROM_PICK(Yanchamr)
STD_ROM_FN(Yanchamr)

static struct BurnRomInfo LitheroRomDesc[] = {
	{ "4.bin",                0x08000, 0x80903766, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "11.bin",               0x08000, 0x7a1ef8cb, BRF_ESS | BRF_PRG }, //  1
	{ "12.bin",               0x08000, 0xa929110b, BRF_ESS | BRF_PRG }, //  2
	
	{ "ky_a-3a-.bin",         0x04000, 0xcb365f3b, BRF_ESS | BRF_PRG }, //  3	M6803 Program Code
	{ "dr01.3cd",             0x04000, 0xe66897bd, BRF_ESS | BRF_PRG }, //  4
	{ "dr02.3f",              0x04000, 0xf9e31e26, BRF_ESS | BRF_PRG }, //  5
	
	{ "7.bin",                0x08000, 0xb55e8d19, BRF_GRA },	    //  6	Tiles
	{ "6.bin",                0x08000, 0x7bbbb209, BRF_GRA },	    //  7
	{ "5.bin",                0x08000, 0x0370fd82, BRF_GRA },	    //  8

	{ "16.bin",               0x08000, 0x5045a507, BRF_GRA },	    //  9	Sprites
	{ "15.bin",               0x08000, 0x946b16a0, BRF_GRA },	    //  10
	{ "18.bin",               0x08000, 0x901b69ff, BRF_GRA },	    //  11
	{ "17.bin",               0x08000, 0x504eed93, BRF_GRA },	    //  12
	{ "14.bin",               0x08000, 0x429d760b, BRF_GRA },	    //  13
	{ "13.bin",               0x08000, 0x1700cd64, BRF_GRA },	    //  14

	{ "8.bin",                0x04000, 0x4f388d63, BRF_GRA },	    //  15	Characters
	{ "9.bin",                0x04000, 0xdaafa2c1, BRF_GRA },	    //  16
	{ "10.bin",               0x04000, 0x60649d19, BRF_GRA },	    //  17

	{ "dr25.3f",              0x00100, 0x8e91430b, BRF_GRA },	    //  18	PROM (Tile Palette Red Component)
	{ "dr30.1m",              0x00100, 0x28c73263, BRF_GRA },	    //  19	PROM (Sprite Palette Red Component)
	{ "dr26.3h",              0x00100, 0xb563b93f, BRF_GRA },	    //  20	PROM (Tile Palette Green Component)
	{ "dr31.1n",              0x00100, 0x3529210e, BRF_GRA },	    //  21	PROM (Sprite Palette Green Component)
	{ "dr27.3j",              0x00100, 0x70d668ef, BRF_GRA },	    //  22	PROM (Tile Palette Blue Component)
	{ "dr29.1l",              0x00100, 0x1173a754, BRF_GRA },	    //  23	PROM (Sprite Palette Blue Component)
	{ "dr32.5p",              0x00020, 0x11cd1f2e, BRF_GRA },	    //  24	PROM (Sprite Height)
	{ "dr33.6f",              0x00100, 0x34d88d3c, BRF_GRA },	    //  25	PROM (Video Timing)
	{ "dr28.8f",              0x00200, 0x6cef0fbd, BRF_OPT },	    //  26	PROM (Unknown)
};

STD_ROM_PICK(Lithero)
STD_ROM_FN(Lithero)

static struct BurnRomInfo SpelunkrRomDesc[] = {
	{ "spra.4e",              0x04000, 0xcf811201, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "spra.4d",              0x04000, 0xbb4faa4f, BRF_ESS | BRF_PRG }, //  1
	{ "sprm.7c",              0x04000, 0xfb6197e2, BRF_ESS | BRF_PRG }, //  2
	{ "sprm.7b",              0x04000, 0x26bb25a4, BRF_ESS | BRF_PRG }, //  3
	
	{ "spra.3d",              0x04000, 0x4110363c, BRF_ESS | BRF_PRG }, //  4	M6803 Program Code
	{ "spra.3f",              0x04000, 0x67a9d2e6, BRF_ESS | BRF_PRG }, //  5
	
	{ "sprm.1d",              0x04000, 0x4ef7ae89, BRF_GRA },	    //  6	Tiles
	{ "sprm.1e",              0x04000, 0xa3755180, BRF_GRA },	    //  7
	{ "sprm.3c",              0x04000, 0xb4008e6a, BRF_GRA },	    //  8
	{ "sprm.3b",              0x04000, 0xf61cf012, BRF_GRA },	    //  9
	{ "sprm.1c",              0x04000, 0x58b21c76, BRF_GRA },	    //  10
	{ "sprm.1b",              0x04000, 0xa95cb3e5, BRF_GRA },	    //  11
	
	{ "sprb.4k",              0x04000, 0xe7f0e861, BRF_GRA },	    //  12	Sprites
	{ "sprb.4f",              0x04000, 0x32663097, BRF_GRA },	    //  13
	{ "sprb.3p",              0x04000, 0x8fbaf373, BRF_GRA },	    //  14
	{ "sprb.4p",              0x04000, 0x37069b76, BRF_GRA },	    //  15
	{ "sprb.4c",              0x04000, 0xcfe46a88, BRF_GRA },	    //  16
	{ "sprb.4e",              0x04000, 0x11c48979, BRF_GRA },	    //  17
	
	{ "sprm.4p",              0x04000, 0x4dfe2e63, BRF_GRA },	    //  18	Characters
	{ "sprm.4l",              0x04000, 0x239f2cd4, BRF_GRA },	    //  19
	{ "sprm.4m",              0x04000, 0xd6d07d70, BRF_GRA },	    //  20
	
	{ "sprm.2k",              0x00100, 0xfd8fa991, BRF_GRA },	    //  21	PROM (Tile Palette Red Component)
	{ "sprb.1m",              0x00100, 0x8d8cccad, BRF_GRA },	    //  22	PROM (Sprite Palette Red Component)
	{ "sprm.2j",              0x00100, 0x0e3890b4, BRF_GRA },	    //  23	PROM (Tile Palette Green Component)
	{ "sprb.1n",              0x00100, 0xc40e1cb2, BRF_GRA },	    //  24	PROM (Sprite Palette Green Component)
	{ "sprm.2h",              0x00100, 0x0478082b, BRF_GRA },	    //  25	PROM (Tile Palette Blue Component)
	{ "sprb.1l",              0x00100, 0x3ec46248, BRF_GRA },	    //  26	PROM (Sprite Palette Blue Component)
	{ "sprb.5p",              0x00020, 0x746c6238, BRF_GRA },	    //  27	PROM (Sprite Height)
	{ "sprb.6f",              0x00100, 0x34d88d3c, BRF_GRA },	    //  28	PROM (Video Timing)
	{ "sprm.8h",              0x00200, 0x875cc442, BRF_OPT },	    //  29	PROM (Unknown)
};

STD_ROM_PICK(Spelunkr)
STD_ROM_FN(Spelunkr)

static struct BurnRomInfo SpelunkrjRomDesc[] = {
	{ "spr_a4ec.bin",         0x04000, 0x4e94a80c, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "spr_a4dd.bin",         0x04000, 0xe7c0cbce, BRF_ESS | BRF_PRG }, //  1
	{ "spr_m7cc.bin",         0x04000, 0x57598a36, BRF_ESS | BRF_PRG }, //  2
	{ "spr_m7bd.bin",         0x04000, 0xecf5137f, BRF_ESS | BRF_PRG }, //  3
	
	{ "spra.3d",              0x04000, 0x4110363c, BRF_ESS | BRF_PRG }, //  4	M6803 Program Code
	{ "spra.3f",              0x04000, 0x67a9d2e6, BRF_ESS | BRF_PRG }, //  5
	
	{ "sprm.1d",              0x04000, 0x4ef7ae89, BRF_GRA },	    //  6	Tiles
	{ "sprm.1e",              0x04000, 0xa3755180, BRF_GRA },	    //  7
	{ "sprm.3c",              0x04000, 0xb4008e6a, BRF_GRA },	    //  8
	{ "sprm.3b",              0x04000, 0xf61cf012, BRF_GRA },	    //  9
	{ "sprm.1c",              0x04000, 0x58b21c76, BRF_GRA },	    //  10
	{ "sprm.1b",              0x04000, 0xa95cb3e5, BRF_GRA },	    //  11
	
	{ "sprb.4k",              0x04000, 0xe7f0e861, BRF_GRA },	    //  12	Sprites
	{ "sprb.4f",              0x04000, 0x32663097, BRF_GRA },	    //  13
	{ "sprb.3p",              0x04000, 0x8fbaf373, BRF_GRA },	    //  14
	{ "sprb.4p",              0x04000, 0x37069b76, BRF_GRA },	    //  15
	{ "sprb.4c",              0x04000, 0xcfe46a88, BRF_GRA },	    //  16
	{ "sprb.4e",              0x04000, 0x11c48979, BRF_GRA },	    //  17
	
	{ "sprm.4p",              0x04000, 0x4dfe2e63, BRF_GRA },	    //  18	Characters
	{ "sprm.4l",              0x04000, 0x239f2cd4, BRF_GRA },	    //  19
	{ "sprm.4m",              0x04000, 0xd6d07d70, BRF_GRA },	    //  20
	
	{ "sprm.2k",              0x00100, 0xfd8fa991, BRF_GRA },	    //  21	PROM (Tile Palette Red Component)
	{ "sprb.1m",              0x00100, 0x8d8cccad, BRF_GRA },	    //  22	PROM (Sprite Palette Red Component)
	{ "sprm.2j",              0x00100, 0x0e3890b4, BRF_GRA },	    //  23	PROM (Tile Palette Green Component)
	{ "sprb.1n",              0x00100, 0xc40e1cb2, BRF_GRA },	    //  24	PROM (Sprite Palette Green Component)
	{ "sprm.2h",              0x00100, 0x0478082b, BRF_GRA },	    //  25	PROM (Tile Palette Blue Component)
	{ "sprb.1l",              0x00100, 0x3ec46248, BRF_GRA },	    //  26	PROM (Sprite Palette Blue Component)
	{ "sprb.5p",              0x00020, 0x746c6238, BRF_GRA },	    //  27	PROM (Sprite Height)
	{ "sprb.6f",              0x00100, 0x34d88d3c, BRF_GRA },	    //  28	PROM (Video Timing)
	{ "sprm.8h",              0x00200, 0x875cc442, BRF_OPT },	    //  29	PROM (Unknown)
};

STD_ROM_PICK(Spelunkrj)
STD_ROM_FN(Spelunkrj)

static struct BurnRomInfo Spelunk2RomDesc[] = {
	{ "sp2-a.4e",             0x04000, 0x96c04bbb, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "sp2-a.4d",             0x04000, 0xcb38c2ff, BRF_ESS | BRF_PRG }, //  1
	{ "sp2-r.7d",             0x08000, 0x558837ea, BRF_ESS | BRF_PRG }, //  2
	{ "sp2-r.7c",             0x08000, 0x4b380162, BRF_ESS | BRF_PRG }, //  3
	{ "sp2-r.7b",             0x04000, 0x7709a1fe, BRF_ESS | BRF_PRG }, //  4
	
	{ "sp2-a.3d",             0x04000, 0x839ec7e2, BRF_ESS | BRF_PRG }, //  5	M6803 Program Code
	{ "sp2-a.3f",             0x04000, 0xad3ce898, BRF_ESS | BRF_PRG }, //  6
	
	{ "sp2-r.1d",             0x08000, 0xc19fa4c9, BRF_GRA },	    //  7	Tiles
	{ "sp2-r.3b",             0x08000, 0x366604af, BRF_GRA },	    //  8
	{ "sp2-r.1b",             0x08000, 0x3a0c4d47, BRF_GRA },	    //  9
	
	{ "sp2-b.4k",             0x04000, 0x6cb67a17, BRF_GRA },	    //  10	Sprites
	{ "sp2-b.4f",             0x04000, 0xe4a1166f, BRF_GRA },	    //  11
	{ "sp2-b.3n",             0x04000, 0xf59e8b76, BRF_GRA },	    //  12
	{ "sp2-b.4n",             0x04000, 0xfa65bac9, BRF_GRA },	    //  13
	{ "sp2-b.4c",             0x04000, 0x1caf7013, BRF_GRA },	    //  14
	{ "sp2-b.4e",             0x04000, 0x780a463b, BRF_GRA },	    //  15
	
	{ "sp2-r.4l",             0x04000, 0x6a4b2d8b, BRF_GRA },	    //  16	Characters
	{ "sp2-r.4m",             0x04000, 0xe1368b61, BRF_GRA },	    //  17
	{ "sp2-r.4p",             0x04000, 0xfc138e13, BRF_GRA },	    //  18
	
	{ "sp2-r.1k",             0x00200, 0x31c1bcdc, BRF_GRA },	    //  19	PROM (Char Palette Red and Green Component)
	{ "sp2-r.2k",             0x00100, 0x1cf5987e, BRF_GRA },	    //  20	PROM (Char Palette Blue Component)
	{ "sp2-r.2j",             0x00100, 0x1acbe2a5, BRF_GRA },	    //  21	PROM (Char Palette Blue Component)
	{ "sp2-b.1m",             0x00100, 0x906104c7, BRF_GRA },	    //  22	PROM (Sprite Palette Red Component)
	{ "sp2-b.1n",             0x00100, 0x5a564c06, BRF_GRA },	    //  23	PROM (Sprite Palette Green Component)
	{ "sp2-b.1l",             0x00100, 0x8f4a2e3c, BRF_GRA },	    //  24	PROM (Sprite Palette Blue Component)
	{ "sp2-b.5p",             0x00020, 0xcd126f6a, BRF_GRA },	    //  25	PROM (Sprite Height)	
	{ "sp2-b.6f",             0x00100, 0x34d88d3c, BRF_GRA },	    //  26	PROM (Video Timing)
	{ "sp2-r.8j",             0x00200, 0x875cc442, BRF_OPT },	    //  27	PROM (Unknown)
	
	{ "ampal16r4a-sp2-r-3h.bin", 0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },	    //  28	PAL
};

STD_ROM_PICK(Spelunk2)
STD_ROM_FN(Spelunk2)

static struct BurnRomInfo YoujyudnRomDesc[] = {
	{ "yju_a4eb.bin",         0x04000, 0x0d356bdc, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "yju_a4db.bin",         0x04000, 0xc169be13, BRF_ESS | BRF_PRG }, //  1
	{ "yju_p4cb.0",           0x04000, 0x60baf3b1, BRF_ESS | BRF_PRG }, //  2
	{ "yju_p4eb.1",           0x04000, 0x8d0521f8, BRF_ESS | BRF_PRG }, //  3
	
	{ "yju_a3fb.bin",         0x04000, 0xe15c8030, BRF_ESS | BRF_PRG }, //  4	M6803 Program Code
	
	{ "yju_p3bb.0",           0x08000, 0xc017913c, BRF_GRA },	    //  5	Tiles
	{ "yju_p1bb.1",           0x08000, 0x94627523, BRF_GRA },	    //  6
	{ "yju_p1cb.2",           0x08000, 0x6a378c56, BRF_GRA },	    //  7
	
	{ "yju_b4ka.00",          0x04000, 0x1bbb864a, BRF_GRA },	    //  8	Sprites
	{ "yju_b4fa.01",          0x04000, 0x14b4dd24, BRF_GRA },	    //  9
	{ "yju_b3na.10",          0x04000, 0x68879321, BRF_GRA },	    //  10
	{ "yju_b4na.11",          0x04000, 0x2860a68b, BRF_GRA },	    //  11
	{ "yju_b4ca.20",          0x04000, 0xab365829, BRF_GRA },	    //  12
	{ "yju_b4ea.21",          0x04000, 0xb36c31e4, BRF_GRA },	    //  13
	
	{ "yju_p4lb.2",           0x04000, 0x87878d9b, BRF_GRA },	    //  14	Characters
	{ "yju_p4mb.1",           0x04000, 0x1e1a0d09, BRF_GRA },	    //  15
	{ "yju_p4pb.0",           0x04000, 0xb4ab520b, BRF_GRA },	    //  16

	{ "yju_p2jb.bpr",         0x00100, 0xa4483631, BRF_GRA },	    //  17	PROM (Tile Palette Red Component)
	{ "yju_b1ma.r",           0x00100, 0xa8340e13, BRF_GRA },	    //  18	PROM (Sprite Palette Red Component)
	{ "yju_p2kb.bpr",         0x00100, 0x85481103, BRF_GRA },	    //  19	PROM (Tile Palette Green Component)
	{ "yju_b1na.g",           0x00100, 0xf5b4bc41, BRF_GRA },	    //  20	PROM (Sprite Palette Green Component)
	{ "yju_p2hb.bpr",         0x00100, 0xa6fd355c, BRF_GRA },	    //  21	PROM (Tile Palette Blue Component)
	{ "yju_b1la.b",           0x00100, 0x45e10491, BRF_GRA },	    //  22	PROM (Sprite Palette Blue Component)
	{ "yju_b-5p.bpr",         0x00020, 0x2095e6a3, BRF_GRA },	    //  23	PROM (Sprite Height)
	{ "yju_b-6f.bpr",         0x00100, 0x82c20d12, BRF_GRA },	    //  24	PROM (Video Timing)
	{ "yju_p-8d.bpr",         0x00200, 0x6cef0fbd, BRF_OPT },	    //  25	PROM (Unknown)
	
	{ "yju_b-pal16r4a-8m.pal", 0x00104, 0x3ece8e61, BRF_OPT },	    //  26	PAL
};

STD_ROM_PICK(Youjyudn)
STD_ROM_FN(Youjyudn)

static struct BurnRomInfo HorizonRomDesc[] = {
	{ "hrza-4e",              0x04000, 0x98b96ba2, BRF_ESS | BRF_PRG }, //  0	Z80 Program Code
	{ "hrza-4d",              0x04000, 0x06b06ac7, BRF_ESS | BRF_PRG }, //  1
	{ "hrza-4b",              0x04000, 0x39c0bd02, BRF_ESS | BRF_PRG }, //  2
	
	{ "hrza-3f",              0x04000, 0x7412c99f, BRF_ESS | BRF_PRG }, //  3	M6803 Program Code
	
	{ "hrzd-4d",              0x02000, 0xb93ed581, BRF_GRA },	    //  4	Tiles
	{ "hrzd-4c",              0x02000, 0x1cf73b53, BRF_GRA },	    //  5
	{ "hrzd-4a",              0x02000, 0xeace8d53, BRF_GRA },	    //  6
	
	{ "hrzb-4k.00",           0x04000, 0x11d2f3a1, BRF_GRA },	    //  7	Sprites
	{ "hrzb-4f.01",           0x04000, 0x356902ea, BRF_GRA },	    //  8
	{ "hrzb-3n.10",           0x04000, 0x87078a02, BRF_GRA },	    //  9
	{ "hrzb-4n.11",           0x04000, 0x5019cb1f, BRF_GRA },	    //  10
	{ "hrzb-4c.20",           0x04000, 0x90b385e7, BRF_GRA },	    //  11
	{ "hrzb-4e.21",           0x04000, 0xd05d77a2, BRF_GRA },	    //  12

	{ "hrzd-1d",              0x00100, 0xb33b08f9, BRF_GRA },	    //  13	PROM (Tile Palette Red Component)
	{ "hrzb-1m.r",            0x00100, 0x0871690a, BRF_GRA },	    //  14	PROM (Sprite Palette Red Component)
	{ "hrzd-1c",              0x00100, 0x6e696f3a, BRF_GRA },	    //  15	PROM (Tile Palette Green Component)
	{ "hrzb-1n.g",            0x00100, 0xf247d0a9, BRF_GRA },	    //  16	PROM (Sprite Palette Green Component)
	{ "hrzd-1e",              0x00100, 0x1fa60379, BRF_GRA },	    //  17	PROM (Tile Palette Blue Component)
	{ "hrzb-1l.b",            0x00100, 0x9ad0a0c8, BRF_GRA },	    //  18	PROM (Sprite Palette Blue Component)
	{ "hrzb-5p",              0x00020, 0x208b49e9, BRF_GRA },	    //  19	PROM (Sprite Height)
	{ "hrzb-6f",              0x00100, 0x82c20d12, BRF_GRA },	    //  20	PROM (Video Timing)
};

STD_ROM_PICK(Horizon)
STD_ROM_FN(Horizon)

static INT32 M62MemIndex()
{
	UINT8 *Next; Next = Mem;
	
	if (!M62BgxTileDim) M62BgxTileDim = 8;
	if (!M62BgyTileDim) M62BgyTileDim = 8;
	if (!M62CharxTileDim) M62CharxTileDim = 8;
	if (!M62CharyTileDim) M62CharyTileDim = 8;
	if (!M62SpriteRamSize) M62SpriteRamSize = 0x100;

	M62Z80Rom              = Next; Next += M62Z80RomSize;
	M62M6803Rom            = Next; Next += 0x0c000;

	RamStart               = Next;
	
	M62SpriteRam           = Next; Next += M62SpriteRamSize;
	M62TileRam             = Next; Next += 0x02000;
	if (M62CharRamSize)   M62CharRam   = Next; Next += M62CharRamSize;
	if (M62ScrollRamSize) M62ScrollRam = Next; Next += M62ScrollRamSize;
	M62Z80Ram              = Next; Next += 0x01000;
	M62M6803Ram            = Next; Next += 0x00080;

	RamEnd                 = Next;

	M62Tiles               = Next; Next += M62NumTiles * M62BgxTileDim * M62BgyTileDim;
	M62Sprites             = Next; Next += M62NumSprites * 16 * 16;
	if (M62NumChars) M62Chars = Next; Next += M62NumChars * M62CharxTileDim * M62CharyTileDim;
	M62Palette             = (UINT32*)Next; Next += M62PaletteEntries * sizeof(UINT32);
	M62PromData            = Next; Next += M62PromSize;
	pFMBuffer              = (INT16*)Next; Next += nBurnSoundLen * 6 * sizeof(INT16);

	MemEnd                 = Next;

	return 0;
}

static INT32 M62DoReset()
{
	ZetOpen(0);
	ZetReset();
	ZetClose();
	
	M6803Reset();
	
	AY8910Reset(0);
	AY8910Reset(1);
	
	MSM5205Reset();
	
	M62BackgroundHScroll = 0;
	M62BackgroundVScroll = 0;
	M62CharHScroll = 0;
	M62CharVScroll = 0;
	M62FlipScreen = 0;
	M62SoundLatch = 0;
	M62Port1 = 0;
	M62Port2 = 0;
	M62SlaveMSM5205VClckReset = 0;
	M62BankControl[0] = M62BankControl[1] = 0;
	Ldrun2BankSwap = 0;
	Ldrun3TopBottomMask = 0;
	KidnikiBackgroundBank = 0;
	SpelunkrPaletteBank = 0;
	
	return 0;
}

UINT8 __fastcall M62Z80Read(UINT16 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall M62Z80Write(UINT16 a, UINT8 d)
{
	if (a <= 0xbfff) return;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall KungfumZ80Read(UINT16 a)
{
	switch (a) {
		case 0xf000:
		case 0xf001:
		case 0xf002:
		case 0xf003: {
			// ???
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall KungfumZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xa000: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0xff00) | d;
			return;
		}
		
		case 0xb000: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0x00ff) | (d << 8);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall Ldrun3Z80Read(UINT16 a)
{
	switch (a) {
		case 0xc800: {
			return 0x05;
		}
		
		case 0xcc00: {
			return 0x07;
		}
		
		case 0xcfff: {
			return 0x07;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall Ldrun4Z80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xc800: {
			INT32 BankAddress = 0x8000 + ((d & 0x01) * 0x4000);
			ZetMapArea(0x8000, 0xbfff, 0, M62Z80Rom + BankAddress);
			ZetMapArea(0x8000, 0xbfff, 2, M62Z80Rom + BankAddress);
			return;
		}
			
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

void __fastcall SpelunkrZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xd000: {
			M62BackgroundVScroll = (M62BackgroundVScroll & 0xff00) | d;
			return;
		}
		
		case 0xd001: {
			M62BackgroundVScroll = (M62BackgroundVScroll & 0x00ff) | (d << 8);
			return;
		}
		
		case 0xd002: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0xff00) | d;
			return;
		}
		
		case 0xd003: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0x00ff) | (d << 8);
			return;
		}
		
		case 0xd004: {
			INT32 BankAddress = 0x8000 + ((d & 0x03) * 0x2000);
			ZetMapArea(0x8000, 0x9fff, 0, M62Z80Rom + BankAddress);
			ZetMapArea(0x8000, 0x9fff, 2, M62Z80Rom + BankAddress);
			return;
		}
		
		case 0xd005: {
			SpelunkrPaletteBank = d & 0x01;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

void __fastcall Spelunk2Z80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xd000: {
			M62BackgroundVScroll = (M62BackgroundVScroll & 0xff00) | d;
			return;
		}
		
		case 0xd001: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0xff00) | d;
			return;
		}
		
		case 0xd002: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0x00ff) | (((d & 0x02) >> 1) << 8);
			M62BackgroundVScroll = (M62BackgroundVScroll & 0x00ff) | ((d & 0x01) << 8);
			SpelunkrPaletteBank = (d & 0x0c) >> 2;
			return;
		}
		
		case 0xd003: {
			INT32 BankAddress1 = 0x18000 + (((d & 0xc0) >> 6) * 0x1000);
			INT32 BankAddress2 = 0x08000 + (((d & 0x3c) >> 2) * 0x1000);
			ZetMapArea(0x8000, 0x8fff, 0, M62Z80Rom + BankAddress1);
			ZetMapArea(0x8000, 0x8fff, 2, M62Z80Rom + BankAddress1);
			ZetMapArea(0x9000, 0x9fff, 0, M62Z80Rom + BankAddress2);
			ZetMapArea(0x9000, 0x9fff, 2, M62Z80Rom + BankAddress2);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall M62Z80PortRead(UINT16 a)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			return 0xff - M62Input[0];
		}
		
		case 0x01: {
			return 0xff - M62Input[1];
		}
		
		case 0x02: {
			return 0xff - M62Input[2];
		}
		
		case 0x03: {
			return M62Dip[0];
		}
		
		case 0x04: {
			return M62Dip[1];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall M62Z80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	switch (a) {
		case 0x00: {
			if ((d & 0x80) == 0) {
				M62SoundLatch = d & 0x7f;
			} else {
				M6803SetIRQ(M6803_IRQ_LINE, M6803_IRQSTATUS_ACK);
			}
			return;
		}
		
		case 0x01: {
			d ^= ~M62Dip[1] & 0x01;
			M62FlipScreen = d & 0x01;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void __fastcall BattroadZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	if (a <= 0x01) {
		M62Z80PortWrite(a, d);
		return;
	}
	
	switch (a) {
		case 0x80: {
			M62BackgroundVScroll = (M62BackgroundVScroll & 0xff00) | d;
			return;
		}
		
		case 0x81: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0x00ff) | (d << 8);
			return;
		}
		
		case 0x82: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0xff00) | d;
			return;
		}
		
		case 0x83: {
			INT32 BankAddress = 0x8000 + ((d & 0x0f) * 0x2000);
			ZetMapArea(0xa000, 0xbfff, 0, M62Z80Rom + BankAddress);
			ZetMapArea(0xa000, 0xbfff, 2, M62Z80Rom + BankAddress);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall Ldrun2Z80PortRead(UINT16 a)
{
	a &= 0xff;
	
	if (a <= 0x04) return M62Z80PortRead(a);
	
	switch (a) {
		case 0x80: {
			if (Ldrun2BankSwap) {
				Ldrun2BankSwap--;
				if (Ldrun2BankSwap == 0) {
					ZetMapArea(0x8000, 0x9fff, 0, M62Z80Rom + 0xa000);
					ZetMapArea(0x8000, 0x9fff, 2, M62Z80Rom + 0xa000);
				}
			}
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall Ldrun2Z80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	if (a <= 0x01) {
		M62Z80PortWrite(a, d);
		return;
	}
	
	switch (a) {
		case 0x80:
		case 0x81: {
			static const INT32 Banks[30] = { 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 };
			INT32 Offset = a - 0x80;
			
			M62BankControl[Offset] = d;
			
			if (Offset == 0x00) {
				if (d >= 1 && d <= 30) {
					INT32 BankAddress = 0x8000 + (Banks[d - 1] * 0x2000);
					ZetMapArea(0x8000, 0x9fff, 0, M62Z80Rom + BankAddress);
					ZetMapArea(0x8000, 0x9fff, 2, M62Z80Rom + BankAddress);
				}
			} else {
				if (M62BankControl[0] == 0x01 && d == 0x0d) {
					Ldrun2BankSwap = 2;
				} else {
					Ldrun2BankSwap = 0;
				}
			}
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void __fastcall Ldrun3Z80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	if (a <= 0x01) {
		M62Z80PortWrite(a, d);
		return;
	}
	
	switch (a) {
		case 0x80: {
			M62BackgroundVScroll = (M62BackgroundVScroll & 0xff00) | d;
			return;
		}
		
		case 0x81: {
			Ldrun3TopBottomMask = d & 0x01;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void __fastcall Ldrun4Z80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	if (a <= 0x01) {
		M62Z80PortWrite(a, d);
		return;
	}
	
	switch (a) {
		case 0x80: {
			// ???
			return;
		}
		
		case 0x81: {
			// ???
			return;
		}
		
		case 0x82: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0x00ff) | (d << 8);
			return;
		}
		
		case 0x83: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0xff00) | d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall KidnikiZ80PortRead(UINT16 a)
{
	a &= 0xff;
	
	if (a <= 0x04) return M62Z80PortRead(a);
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Read => %02X\n"), a);
		}
	}

	return 0;
}

void __fastcall KidnikiZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	if (a <= 0x01) {
		M62Z80PortWrite(a, d);
		return;
	}
	
	switch (a) {
		case 0x80: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0xff00) | d;
			return;
		}
		
		case 0x81: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0x00ff) | (d << 8);
			return;
		}
		
		case 0x82: {
			M62CharVScroll = (M62CharVScroll & 0xff00) | d;
			return;
		}
		
		case 0x83: {
			M62CharVScroll = (M62CharVScroll & 0x00ff) | (d << 8);
			return;
		}
		
		case 0x84: {
			KidnikiBackgroundBank = d & 0x01;
			return;
		}
		
		case 0x85: {
			INT32 BankAddress = 0x8000 + ((d & 0x0f) * 0x2000);
			ZetMapArea(0x8000, 0x9fff, 0, M62Z80Rom + BankAddress);
			ZetMapArea(0x8000, 0x9fff, 2, M62Z80Rom + BankAddress);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

void __fastcall YoujyudnZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	
	if (a <= 0x01) {
		M62Z80PortWrite(a, d);
		return;
	}
	
	switch (a) {
		case 0x80: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0x00ff) | (d << 8);
			return;
		}
		
		case 0x81: {
			M62BackgroundHScroll = (M62BackgroundHScroll & 0xff00) | d;
			return;
		}
		
		case 0x83: {
			INT32 BankAddress = 0x8000 + ((d & 0x01) * 0x4000);
			ZetMapArea(0x8000, 0xbfff, 0, M62Z80Rom + BankAddress);
			ZetMapArea(0x8000, 0xbfff, 2, M62Z80Rom + BankAddress);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write => %02X, %02X\n"), a, d);
		}
	}
}

UINT8 M62M6803ReadByte(UINT16 a)
{
	if (a <= 0x001f) {
		return m6803_internal_registers_r(a);
	}
	
	if (a >= 0x0080 && a <= 0x00ff) {
		return M62M6803Ram[a - 0x0080];
	}

	bprintf(PRINT_NORMAL, _T("M6803 Read Byte -> %04X\n"), a);
	
	return 0;
}

void M62M6803WriteByte(UINT16 a, UINT8 d)
{
	if (a <= 0x001f) {
		m6803_internal_registers_w(a, d);
		return;
	}
	
	if (a >= 0x0080 && a <= 0x00ff) {
		M62M6803Ram[a - 0x0080] = d;
		return;
	}
	
	switch (a) {
		case 0x0800: {
			M6803SetIRQ(M6803_IRQ_LINE, M6803_IRQSTATUS_NONE);
			return;
		}
		
		case 0x0801: {
			MSM5205DataWrite(0, d);
			return;
		}
		
		case 0x0802: {
			MSM5205DataWrite(1, d);
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("M6803 Write Byte -> %04X, %02X\n"), a, d);
}

UINT8 M62M6803ReadPort(UINT16 a)
{
	switch (a) {
		case M6803_PORT1: {
  			if (M62Port2 & 0x08) return AY8910Read(0);
			if (M62Port2 & 0x10) return AY8910Read(1);
			return 0xff;
		}
		
		case M6803_PORT2: {
			return 0;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("M6803 Read Port -> %04X\n"), a);
	
	return 0;
}

void M62M6803WritePort(UINT16 a, UINT8 d)
{
	switch (a) {
		case M6803_PORT1: {
			M62Port1 = d;
			return;
		}
		
		case M6803_PORT2: {
			if ((M62Port2 & 0x01) && !(d & 0x01)) {
				if (M62Port2 & 0x04) {
					if (M62Port2 & 0x08) {
						AY8910Write(0, 0, M62Port1);
					}
					if (M62Port2 & 0x10) {
						AY8910Write(1, 0, M62Port1);
					}
				} else {
					if (M62Port2 & 0x08) {
						AY8910Write(0, 1, M62Port1);
					}
					if (M62Port2 & 0x10) {
						AY8910Write(1, 1, M62Port1);
					}
				}
			}
			
			M62Port2 = d;
			return;
		}
	}
	
	bprintf(PRINT_NORMAL, _T("M6803 Write Port -> %04X, %02X\n"), a, d);
}

static INT32 Tile1024PlaneOffsets[3]       = { 0x20000, 0x10000, 0 };
static INT32 Tile2048PlaneOffsets[3]       = { 0x40000, 0x20000, 0 };
static INT32 Tile4096PlaneOffsets[3]       = { 0x80000, 0x40000, 0 };
static INT32 TileXOffsets[8]               = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 TileYOffsets[8]               = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 KungfumSpritePlaneOffsets[3]  = { 0x80000, 0x40000, 0 };
static INT32 BattroadSpritePlaneOffsets[3] = { 0x40000, 0x20000, 0 };
static INT32 LdrunSpritePlaneOffsets[3]    = { 0x20000, 0x10000, 0 };
static INT32 KidnikiSpritePlaneOffsets[3]  = { 0x100000, 0x80000, 0 };
static INT32 SpriteXOffsets[16]            = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
static INT32 SpriteYOffsets[16]            = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };
static INT32 BattroadCharPlaneOffsets[2]   = { 0, 0x10000 };
static INT32 LotlotPlaneOffsets[3]         = { 0, 0x10000, 0x20000 };
static INT32 LotlotXOffsets[12]            = { 0, 1, 2, 3, 128, 129, 130, 131, 132, 133, 134, 135 };
static INT32 LotlotYOffsets[10]            = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72 };
static INT32 KidnikiPlaneOffsets[3]        = { 0, 0x20000, 0x40000 };
static INT32 KidnikiXOffsets[12]           = { 0, 1, 2, 3, 64, 65, 66, 67, 68, 69, 70, 71 };
static INT32 KidnikiYOffsets[8]            = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 Spelunk2PlaneOffsets[3]       = { 0, 0x20000, 0x40000 };
static INT32 Spelunk2XOffsets[12]          = { 0, 1, 2, 3, 0x10000, 0x10001, 0x10002, 0x10003, 0x10004, 0x10005, 0x10006, 0x10007 };
static INT32 Spelunk2YOffsets[8]           = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 YoujyudnTilePlaneOffsets[3]   = { 0x40000, 0x20000, 0 };
static INT32 YoujyudnTileXOffsets[8]       = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 YoujyudnTileYOffsets[16]      = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };

UINT8 M62SoundLatchRead(UINT32)
{
	return M62SoundLatch;
}

static void AY8910_0PortBWrite(UINT32, UINT32 d)
{
	MSM5205PlaymodeWrite(0, (d >> 2) & 0x07);
	MSM5205PlaymodeWrite(1, ((d >> 2) & 0x04) | 0x03);
	
	MSM5205ResetWrite(0, d & 0x01);
	MSM5205ResetWrite(1, d & 0x02);
}

inline static INT32 M62SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)(ZetTotalCycles() * nSoundRate / M62Z80Clock);
}

static void M62MSM5205Vck0()
{
	M6803SetIRQ(M6803_INPUT_LINE_NMI, M6803_IRQSTATUS_AUTO);
	M62SlaveMSM5205VClckReset = 1;
}

static INT32 M62MemInit()
{
	INT32 nLen;
	
	M62PaletteEntries = BurnDrvGetPaletteEntries();
	
	Mem = NULL;
	M62MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	M62MemIndex();
	
	return 0;
}

static INT32 KungfumLoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x18000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  1, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x06000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x0a000,  4, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile1024PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x06000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0a000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0c000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0e000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x12000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x14000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x16000, 19, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, KungfumSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 24, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 25, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 26, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 27, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 KungfumdLoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x18000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  1, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x04000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  3, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile1024PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0c000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x14000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, KungfumSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 20, 1); if (nRet != 0) return 1;

	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 BattroadLoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x0c000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x02000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x06000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x08000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x0a000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x0c000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x0e000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x10000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x14000,  9, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x06000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x08000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x0a000, 12, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x0c000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 15, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile1024PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x0c000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x06000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0a000, 21, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, BattroadSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load and decode the chars
	memset(M62TempRom, 0, 0x0c000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000, 22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000, 23, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumChars, 2, M62CharxTileDim, M62CharyTileDim, BattroadCharPlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Chars);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 24, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 25, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 26, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 27, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 28, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 29, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 30, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 31, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00720, 32, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 LdrunLoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x06000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x02000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x06000,  3, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x0a000,  5, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x06000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile1024PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x06000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 11, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, LdrunSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 19, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 Ldrun2LoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x0c000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x02000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x06000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x08000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x0a000,  5, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x06000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x0a000,  8, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x0c000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 11, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile1024PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x0c000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x06000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0a000, 17, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, BattroadSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 24, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 25, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 Ldrun3LoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x18000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x08000,  2, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x04000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  4, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile2048PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0c000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x14000, 13, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, KungfumSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 21, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 Ldrun3jLoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x0c000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x08000,  2, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x04000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  4, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x0c000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile2048PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x0c000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, BattroadSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 18, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 LotlotLoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x06000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  1, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x0a000,  2, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x06000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  5, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, LotlotPlaneOffsets, LotlotXOffsets, LotlotYOffsets, 0x100, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x06000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, LdrunSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load and decode the chars
	memset(M62TempRom, 0, 0x06000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 11, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumChars, 3, M62CharxTileDim, M62CharyTileDim, LotlotPlaneOffsets, LotlotXOffsets, LotlotYOffsets, 0x100, M62TempRom, M62Chars);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 14, 1); if (nRet != 0) return 1;	
	nRet = BurnLoadRom(M62PromData + 0x00300, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 17, 1); if (nRet != 0) return 1;	
	nRet = BurnLoadRom(M62PromData + 0x00600, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00700, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00800, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00900, 21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00920, 22, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 KidnikiLoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x30000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x08000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x10000,  3, 1); if (nRet != 0) return 1;
	memcpy(M62Z80Rom + 0x20000, M62Z80Rom + 0x18000, 0x8000);
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x00000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x04000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  6, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x30000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000,  9, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile4096PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x30000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0c000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x14000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x18000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x1c000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x20000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x24000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x28000, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x2c000, 21, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, KidnikiSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load and decode the chars
	memset(M62TempRom, 0, 0x30000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000, 22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 24, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumChars, 3, M62CharxTileDim, M62CharyTileDim, KidnikiPlaneOffsets, KidnikiXOffsets, KidnikiYOffsets, 0x80, M62TempRom, M62Chars);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 25, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 26, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 27, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 28, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 29, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 30, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 31, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 32, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 LitheroLoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x30000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x08000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x10000,  2, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x04000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  5, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x30000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile4096PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x30000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x18000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x20000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x28000, 14, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, KidnikiSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load and decode the chars
	memset(M62TempRom, 0, 0x30000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 17, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumChars, 3, M62CharxTileDim, M62CharyTileDim, KidnikiPlaneOffsets, KidnikiXOffsets, KidnikiYOffsets, 0x80, M62TempRom, M62Chars);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 24, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 25, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 SpelunkrLoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x18000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x08000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x0c000,  3, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x04000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  5, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0c000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x14000, 11, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile4096PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0c000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x14000, 17, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, KungfumSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load and decode the chars
	memset(M62TempRom, 0, 0x18000);
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x18000);
	nRet = BurnLoadRom(pTemp  + 0x00000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp  + 0x04000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp  + 0x08000, 20, 1); if (nRet != 0) return 1;
	memcpy(M62TempRom + 0x0000, pTemp + 0x0000, 0x800);
	memcpy(M62TempRom + 0x2000, pTemp + 0x0800, 0x800);
	memcpy(M62TempRom + 0x0800, pTemp + 0x1000, 0x800);
	memcpy(M62TempRom + 0x2800, pTemp + 0x1800, 0x800);
	memcpy(M62TempRom + 0x1000, pTemp + 0x2000, 0x800);
	memcpy(M62TempRom + 0x3000, pTemp + 0x2800, 0x800);
	memcpy(M62TempRom + 0x0800, pTemp + 0x3000, 0x800);
	memcpy(M62TempRom + 0x3800, pTemp + 0x3800, 0x800);
	memcpy(M62TempRom + 0x4000, pTemp + 0x4000, 0x800);
	memcpy(M62TempRom + 0x6000, pTemp + 0x4800, 0x800);
	memcpy(M62TempRom + 0x4800, pTemp + 0x5000, 0x800);
	memcpy(M62TempRom + 0x6800, pTemp + 0x5800, 0x800);
	memcpy(M62TempRom + 0x5000, pTemp + 0x6000, 0x800);
	memcpy(M62TempRom + 0x7000, pTemp + 0x6800, 0x800);
	memcpy(M62TempRom + 0x5800, pTemp + 0x7000, 0x800);
	memcpy(M62TempRom + 0x7800, pTemp + 0x7800, 0x800);
	memcpy(M62TempRom + 0x8000, pTemp + 0x8000, 0x800);
	memcpy(M62TempRom + 0xa000, pTemp + 0x8800, 0x800);
	memcpy(M62TempRom + 0x8800, pTemp + 0x9000, 0x800);
	memcpy(M62TempRom + 0xa800, pTemp + 0x9800, 0x800);
	memcpy(M62TempRom + 0x9000, pTemp + 0xa000, 0x800);
	memcpy(M62TempRom + 0xb000, pTemp + 0xa800, 0x800);
	memcpy(M62TempRom + 0x9800, pTemp + 0xb000, 0x800);
	memcpy(M62TempRom + 0xb800, pTemp + 0xb800, 0x800);
	BurnFree(pTemp);
	GfxDecode(M62NumChars, 3, M62CharxTileDim, M62CharyTileDim, Spelunk2PlaneOffsets, Spelunk2XOffsets, Spelunk2YOffsets, 0x40, M62TempRom, M62Chars);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 24, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 25, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 26, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 27, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 28, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 Spelunk2LoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x18000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x08000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x10000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x18000,  4, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x04000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  6, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000,  9, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile4096PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0c000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x14000, 15, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, KungfumSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load and decode the chars
	memset(M62TempRom, 0, 0x18000);
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x18000);
	nRet = BurnLoadRom(pTemp  + 0x00000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp  + 0x04000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp  + 0x08000, 18, 1); if (nRet != 0) return 1;
	memcpy(M62TempRom + 0x0000, pTemp + 0x0000, 0x800);
	memcpy(M62TempRom + 0x2000, pTemp + 0x0800, 0x800);
	memcpy(M62TempRom + 0x0800, pTemp + 0x1000, 0x800);
	memcpy(M62TempRom + 0x2800, pTemp + 0x1800, 0x800);
	memcpy(M62TempRom + 0x1000, pTemp + 0x2000, 0x800);
	memcpy(M62TempRom + 0x3000, pTemp + 0x2800, 0x800);
	memcpy(M62TempRom + 0x0800, pTemp + 0x3000, 0x800);
	memcpy(M62TempRom + 0x3800, pTemp + 0x3800, 0x800);
	memcpy(M62TempRom + 0x4000, pTemp + 0x4000, 0x800);
	memcpy(M62TempRom + 0x6000, pTemp + 0x4800, 0x800);
	memcpy(M62TempRom + 0x4800, pTemp + 0x5000, 0x800);
	memcpy(M62TempRom + 0x6800, pTemp + 0x5800, 0x800);
	memcpy(M62TempRom + 0x5000, pTemp + 0x6000, 0x800);
	memcpy(M62TempRom + 0x7000, pTemp + 0x6800, 0x800);
	memcpy(M62TempRom + 0x5800, pTemp + 0x7000, 0x800);
	memcpy(M62TempRom + 0x7800, pTemp + 0x7800, 0x800);
	memcpy(M62TempRom + 0x8000, pTemp + 0x8000, 0x800);
	memcpy(M62TempRom + 0xa000, pTemp + 0x8800, 0x800);
	memcpy(M62TempRom + 0x8800, pTemp + 0x9000, 0x800);
	memcpy(M62TempRom + 0xa800, pTemp + 0x9800, 0x800);
	memcpy(M62TempRom + 0x9000, pTemp + 0xa000, 0x800);
	memcpy(M62TempRom + 0xb000, pTemp + 0xa800, 0x800);
	memcpy(M62TempRom + 0x9800, pTemp + 0xb000, 0x800);
	memcpy(M62TempRom + 0xb800, pTemp + 0xb800, 0x800);
	BurnFree(pTemp);
	GfxDecode(M62NumChars, 3, M62CharxTileDim, M62CharyTileDim, Spelunk2PlaneOffsets, Spelunk2XOffsets, Spelunk2YOffsets, 0x40, M62TempRom, M62Chars);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 24, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00700, 25, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00720, 26, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 YoujyudnLoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x18000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x08000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x0c000,  3, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  4, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x18000);
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x18000);
	nRet = BurnLoadRom(pTemp  + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp  + 0x08000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp  + 0x10000,  7, 1); if (nRet != 0) return 1;
	memcpy(M62TempRom + 0x0000, pTemp + 0x04000, 0x4000);
	memcpy(M62TempRom + 0x4000, pTemp + 0x0c000, 0x4000);
	memcpy(M62TempRom + 0x8000, pTemp + 0x14000, 0x4000);
	BurnFree(pTemp);
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, YoujyudnTilePlaneOffsets, YoujyudnTileXOffsets, YoujyudnTileYOffsets, 0x80, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0c000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x14000, 13, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, KungfumSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load and decode the chars
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000, 16, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumChars, 3, M62CharxTileDim, M62CharyTileDim, KidnikiPlaneOffsets, KidnikiXOffsets, KidnikiYOffsets, 0x80, M62TempRom, M62Chars);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 24, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static INT32 HorizonLoadRoms()
{
	INT32 nRet = 0;
	
	M62TempRom = (UINT8 *)BurnMalloc(0x18000);

	// Load Z80 Program Roms
	nRet = BurnLoadRom(M62Z80Rom   + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x04000,  1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62Z80Rom   + 0x08000,  2, 1); if (nRet != 0) return 1;
	
	// Load M6803 Program Roms
	nRet = BurnLoadRom(M62M6803Rom + 0x08000,  3, 1); if (nRet != 0) return 1;
		
	// Load and decode the tiles
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x02000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  6, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumTiles, 3, M62BgxTileDim, M62BgyTileDim, Tile1024PlaneOffsets, TileXOffsets, TileYOffsets, 0x40, M62TempRom, M62Tiles);
	
	// Load and decode the sprites
	memset(M62TempRom, 0, 0x18000);
	nRet = BurnLoadRom(M62TempRom  + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x04000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x08000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x0c000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x10000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62TempRom  + 0x14000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(M62NumSprites, 3, 16, 16, KungfumSpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, M62TempRom, M62Sprites);
	
	// Load the Proms
	nRet = BurnLoadRom(M62PromData + 0x00000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00100, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00200, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00300, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00400, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00500, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00600, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(M62PromData + 0x00620, 20, 1); if (nRet != 0) return 1;
	
	BurnFree(M62TempRom);
	
	return 0;
}

static void M62MachineInit()
{
//	BurnSetRefreshRate(55.0);
	
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(M62Z80Read);
	ZetSetWriteHandler(M62Z80Write);
	ZetSetInHandler(M62Z80PortRead);
	ZetSetOutHandler(M62Z80PortWrite);
	ZetMapArea(0x0000, 0x7fff, 0, M62Z80Rom   );
	ZetMapArea(0x0000, 0x7fff, 2, M62Z80Rom   );
	ZetMapArea(0xc000, 0xc0ff, 0, M62SpriteRam);
	ZetMapArea(0xc000, 0xc0ff, 1, M62SpriteRam);
	ZetMapArea(0xc000, 0xc0ff, 2, M62SpriteRam);
	ZetMapArea(0xd000, 0xdfff, 0, M62TileRam  );
	ZetMapArea(0xd000, 0xdfff, 1, M62TileRam  );
	ZetMapArea(0xd000, 0xdfff, 2, M62TileRam  );
	ZetMapArea(0xe000, 0xefff, 0, M62Z80Ram   );
	ZetMapArea(0xe000, 0xefff, 1, M62Z80Ram   );
	ZetMapArea(0xe000, 0xefff, 2, M62Z80Ram   );
	ZetMemEnd();
	ZetClose();
	
	M6803Init(1);
	M6803MapMemory(M62M6803Rom, 0x4000, 0xffff, M6803_ROM);
	M6803SetReadByteHandler(M62M6803ReadByte);
	M6803SetWriteByteHandler(M62M6803WriteByte);
	M6803SetReadPortHandler(M62M6803ReadPort);
	M6803SetWritePortHandler(M62M6803WritePort);
	
	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;

	MSM5205Init(0, M62SynchroniseStream, 384000, M62MSM5205Vck0, MSM5205_S96_4B, 50, 1);
	MSM5205Init(1, M62SynchroniseStream, 384000, NULL, MSM5205_SEX_4B, 50, 1);
	
	AY8910Init(0, 894886, nBurnSoundRate, &M62SoundLatchRead, NULL, NULL, &AY8910_0PortBWrite);
	AY8910Init(1, 894886, nBurnSoundRate, NULL, NULL, NULL, NULL);
	
	GenericTilesInit();
	
	M62Z80Clock = 4000000;
	M62M6803Clock = 894886;
	
	M62SpriteHeightPromOffset = (M62PaletteEntries & 0xf00) * 3;
}

static INT32 KungfumMachineInit()
{
	M62MachineInit();
	
	ZetOpen(0);
	ZetSetReadHandler(KungfumZ80Read);
	ZetSetWriteHandler(KungfumZ80Write);
	ZetMemEnd();
	ZetClose();
	
	M62Z80Clock = 3072000;
	M62RenderFunction = KungfumDraw;

	M62DoReset();
	
	return 0;
}

static INT32 BattroadMachineInit()
{
	M62MachineInit();
	
	ZetOpen(0);
	ZetSetOutHandler(BattroadZ80PortWrite);
	ZetMapArea(0xa000, 0xbfff, 0, M62Z80Rom + 0x8000);
	ZetMapArea(0xa000, 0xbfff, 2, M62Z80Rom + 0x8000);
	ZetMapArea(0xc800, 0xcfff, 0, M62CharRam        );
	ZetMapArea(0xc800, 0xcfff, 1, M62CharRam        );
	ZetMapArea(0xc800, 0xcfff, 2, M62CharRam        );
	ZetMemEnd();
	ZetClose();
	
	M62Z80Clock = 3072000;

	M62RenderFunction = BattroadDraw;
	M62ExtendTileInfoFunction = BattroadExtendTile;
	M62ExtendCharInfoFunction = BattroadExtendChar;

	M62DoReset();
	
	return 0;
}

static INT32 LdrunMachineInit()
{
	M62MachineInit();

	M62RenderFunction = LdrunDraw;
	M62ExtendTileInfoFunction = LdrunExtendTile;

	M62DoReset();
	
	return 0;
}

static INT32 Ldrun2MachineInit()
{
	M62MachineInit();
	
	ZetOpen(0);
	ZetSetInHandler(Ldrun2Z80PortRead);
	ZetSetOutHandler(Ldrun2Z80PortWrite);
	ZetMapArea(0x8000, 0x9fff, 0, M62Z80Rom + 0x8000);
	ZetMapArea(0x8000, 0x9fff, 2, M62Z80Rom + 0x8000);
	ZetMemEnd();
	ZetClose();
	
	M62RenderFunction = LdrunDraw;
	M62ExtendTileInfoFunction = Ldrun2ExtendTile;

	M62DoReset();
	
	return 0;
}

static INT32 Ldrun3MachineInit()
{
	M62MachineInit();
	
	ZetOpen(0);
	ZetSetReadHandler(Ldrun3Z80Read);
	ZetSetOutHandler(Ldrun3Z80PortWrite);
	ZetMapArea(0x8000, 0xbfff, 0, M62Z80Rom + 0x8000);
	ZetMapArea(0x8000, 0xbfff, 2, M62Z80Rom + 0x8000);
	ZetMemEnd();
	ZetClose();
	
	M62RenderFunction = Ldrun3Draw;
	M62ExtendTileInfoFunction = Ldrun2ExtendTile;

	M62DoReset();
	
	return 0;
}

static INT32 Ldrun4MachineInit()
{
	M62MachineInit();
	
	ZetOpen(0);
	ZetSetWriteHandler(Ldrun4Z80Write);
	ZetSetOutHandler(Ldrun4Z80PortWrite);
	ZetMapArea(0x8000, 0xbfff, 0, M62Z80Rom + 0x8000);
	ZetMapArea(0x8000, 0xbfff, 2, M62Z80Rom + 0x8000);
	ZetMemEnd();
	ZetClose();
	
	M62RenderFunction = Ldrun4Draw;
	M62ExtendTileInfoFunction = Ldrun4ExtendTile;

	M62DoReset();
	
	return 0;
}

static INT32 LotlotMachineInit()
{
	M62MachineInit();
	
	ZetOpen(0);
	ZetMapArea(0xa000, 0xafff, 0, M62CharRam);
	ZetMapArea(0xa000, 0xafff, 1, M62CharRam);
	ZetMapArea(0xa000, 0xafff, 2, M62CharRam);
	ZetMemEnd();
	ZetClose();
	
	M62RenderFunction = LotlotDraw;
	M62ExtendTileInfoFunction = LotlotExtendTile;
	M62ExtendCharInfoFunction = LotlotExtendChar;

	M62DoReset();
	
	return 0;
}

static INT32 KidnikiMachineInit()
{
	M62MachineInit();
	
	ZetOpen(0);
	ZetSetInHandler(KidnikiZ80PortRead);
	ZetSetOutHandler(KidnikiZ80PortWrite);
	ZetMapArea(0x8000, 0x9fff, 0, M62Z80Rom + 0x8000);
	ZetMapArea(0x8000, 0x9fff, 2, M62Z80Rom + 0x8000);
	ZetMapArea(0xa000, 0xafff, 0, M62TileRam        );
	ZetMapArea(0xa000, 0xafff, 1, M62TileRam        );
	ZetMapArea(0xa000, 0xafff, 2, M62TileRam        );
	ZetMapArea(0xd000, 0xdfff, 0, M62CharRam        );
	ZetMapArea(0xd000, 0xdfff, 1, M62CharRam        );
	ZetMapArea(0xd000, 0xdfff, 2, M62CharRam        );
	ZetMemEnd();
	ZetClose();
	
	M62RenderFunction = KidnikiDraw;
	M62ExtendTileInfoFunction = KidnikiExtendTile;
	M62ExtendCharInfoFunction = LotlotExtendChar;

	M62DoReset();
	
	return 0;
}

static INT32 SpelunkrMachineInit()
{
	M62MachineInit();
	
	ZetOpen(0);
	ZetSetWriteHandler(SpelunkrZ80Write);
	ZetMapArea(0x8000, 0x9fff, 0, M62Z80Rom + 0x8000);
	ZetMapArea(0x8000, 0x9fff, 2, M62Z80Rom + 0x8000);
	ZetMapArea(0xa000, 0xbfff, 0, M62TileRam        );
	ZetMapArea(0xa000, 0xbfff, 1, M62TileRam        );
	ZetMapArea(0xa000, 0xbfff, 2, M62TileRam        );
	ZetMapArea(0xc800, 0xcfff, 0, M62CharRam        );
	ZetMapArea(0xc800, 0xcfff, 1, M62CharRam        );
	ZetMapArea(0xc800, 0xcfff, 2, M62CharRam        );
	ZetMemCallback(0xd000, 0xdfff, 0);
	ZetMemCallback(0xd000, 0xdfff, 1);
	ZetMemCallback(0xd000, 0xdfff, 2);
	ZetMemEnd();
	ZetClose();
	
	M62RenderFunction = SpelunkrDraw;
	M62ExtendTileInfoFunction = SpelunkrExtendTile;
	M62ExtendCharInfoFunction = SpelunkrExtendChar;

	M62DoReset();
	
	return 0;
}

static INT32 Spelunk2MachineInit()
{
	M62MachineInit();
	
	ZetOpen(0);
	ZetSetWriteHandler(Spelunk2Z80Write);
	ZetMapArea(0x8000, 0x8fff, 0, M62Z80Rom + 0x18000);
	ZetMapArea(0x8000, 0x8fff, 2, M62Z80Rom + 0x18000);
	ZetMapArea(0x9000, 0x9fff, 0, M62Z80Rom + 0x08000);
	ZetMapArea(0x9000, 0x9fff, 2, M62Z80Rom + 0x08000);
	ZetMapArea(0xa000, 0xbfff, 0, M62TileRam        );
	ZetMapArea(0xa000, 0xbfff, 1, M62TileRam        );
	ZetMapArea(0xa000, 0xbfff, 2, M62TileRam        );
	ZetMapArea(0xc800, 0xcfff, 0, M62CharRam        );
	ZetMapArea(0xc800, 0xcfff, 1, M62CharRam        );
	ZetMapArea(0xc800, 0xcfff, 2, M62CharRam        );
	ZetMemCallback(0xd000, 0xdfff, 0);
	ZetMemCallback(0xd000, 0xdfff, 1);
	ZetMemCallback(0xd000, 0xdfff, 2);
	ZetMemEnd();
	ZetClose();
	
	M62RenderFunction = Spelunk2Draw;
	M62ExtendTileInfoFunction = Spelunk2ExtendTile;
	M62ExtendCharInfoFunction = SpelunkrExtendChar;
	
	M62SpriteHeightPromOffset = 0x700;

	M62DoReset();
	
	return 0;
}

static INT32 YoujyudnMachineInit()
{
	M62MachineInit();
	
	ZetOpen(0);
	ZetSetOutHandler(YoujyudnZ80PortWrite);
	ZetMapArea(0x8000, 0xbfff, 0, M62Z80Rom + 0x8000);
	ZetMapArea(0x8000, 0xbfff, 2, M62Z80Rom + 0x8000);
	ZetMapArea(0xc800, 0xcfff, 0, M62CharRam        );
	ZetMapArea(0xc800, 0xcfff, 1, M62CharRam        );
	ZetMapArea(0xc800, 0xcfff, 2, M62CharRam        );
	ZetMemCallback(0xd800, 0xdfff, 0);
	ZetMemCallback(0xd800, 0xdfff, 1);
	ZetMemCallback(0xd800, 0xdfff, 2);
	ZetMemEnd();
	ZetClose();
	
	M62RenderFunction = YoujyudnDraw;
	M62ExtendTileInfoFunction = YoujyudnExtendTile;
	M62ExtendCharInfoFunction = YoujyudnExtendChar;
	
	M62Z80Clock = 3072000;

	M62DoReset();
	
	return 0;
}

static INT32 HorizonMachineInit()
{
	M62MachineInit();
	
	ZetOpen(0);
	ZetMapArea(0x8000, 0xbfff, 0, M62Z80Rom + 0x8000);
	ZetMapArea(0x8000, 0xbfff, 2, M62Z80Rom + 0x8000);
	ZetMapArea(0xc000, 0xc1ff, 0, M62SpriteRam      );
	ZetMapArea(0xc000, 0xc1ff, 1, M62SpriteRam      );
	ZetMapArea(0xc000, 0xc1ff, 2, M62SpriteRam      );
	ZetMapArea(0xc800, 0xc83f, 0, M62ScrollRam      );
	ZetMapArea(0xc800, 0xc83f, 1, M62ScrollRam      );
	ZetMapArea(0xc800, 0xc83f, 2, M62ScrollRam      );
	ZetMemEnd();
	ZetClose();
		
	M62RenderFunction = HorizonDraw;
	M62ExtendTileInfoFunction = HorizonExtendTile;
	
	M62DoReset();
	
	return 0;
}

static INT32 KungfumInit()
{
	M62Z80RomSize = 0x8000;
	M62PromSize = 0x720;
	M62NumTiles = 0x400;
	M62NumSprites = 0x400;
	
	if (M62MemInit()) return 1;
	if (KungfumLoadRoms()) return 1;
	if (KungfumMachineInit()) return 1;

	return 0;
}

static INT32 KungfumdInit()
{
	M62Z80RomSize = 0x8000;
	M62PromSize = 0x720;
	M62NumTiles = 0x400;
	M62NumSprites = 0x400;
	
	if (M62MemInit()) return 1;
	if (KungfumdLoadRoms()) return 1;
	if (KungfumMachineInit()) return 1;

	return 0;
}

static INT32 BattroadInit()
{
	M62Z80RomSize = 0x16000;
	M62PromSize = 0x740;
	M62NumTiles = 0x400;
	M62NumSprites = 0x200;
	M62NumChars = 0x400;
	M62CharRamSize = 0x800;
	
	if (M62MemInit()) return 1;
	if (BattroadLoadRoms()) return 1;
	if (BattroadMachineInit()) return 1;

	return 0;
}

static INT32 LdrunInit()
{
	M62Z80RomSize = 0x8000;
	M62PromSize = 0x720;
	M62NumTiles = 0x400;
	M62NumSprites = 0x100;
	
	if (M62MemInit()) return 1;
	if (LdrunLoadRoms()) return 1;
	if (LdrunMachineInit()) return 1;

	return 0;
}

static INT32 Ldrun2Init()
{
	M62Z80RomSize = 0xc000;
	M62PromSize = 0x720;
	M62NumTiles = 0x400;
	M62NumSprites = 0x200;
	
	if (M62MemInit()) return 1;
	if (Ldrun2LoadRoms()) return 1;
	if (Ldrun2MachineInit()) return 1;

	return 0;
}

static INT32 Ldrun3Init()
{
	M62Z80RomSize = 0xc000;
	M62PromSize = 0x720;
	M62NumTiles = 0x800;
	M62NumSprites = 0x400;
	
	if (M62MemInit()) return 1;
	if (Ldrun3LoadRoms()) return 1;
	if (Ldrun3MachineInit()) return 1;

	return 0;
}

static INT32 Ldrun3jInit()
{
	M62Z80RomSize = 0xc000;
	M62PromSize = 0x720;
	M62NumTiles = 0x800;
	M62NumSprites = 0x200;
	
	if (M62MemInit()) return 1;
	if (Ldrun3jLoadRoms()) return 1;
	if (Ldrun3MachineInit()) return 1;

	return 0;
}

static INT32 Ldrun4Init()
{
	M62Z80RomSize = 0x10000;
	M62PromSize = 0x720;
	M62NumTiles = 0x800;
	M62NumSprites = 0x400;
	
	if (M62MemInit()) return 1;
	if (Ldrun3LoadRoms()) return 1;
	if (Ldrun4MachineInit()) return 1;

	return 0;
}

static INT32 LotlotInit()
{
	M62Z80RomSize = 0x8000;
	M62PromSize = 0xa20;
	M62NumTiles = 0x100;
	M62NumSprites = 0x100;
	M62NumChars = 0x100;
	M62CharRamSize = 0x1000;
	M62BgxTileDim = 12;
	M62BgyTileDim = 10;
	M62CharxTileDim = 12;
	M62CharyTileDim = 10;
	
	if (M62MemInit()) return 1;
	if (LotlotLoadRoms()) return 1;
	if (LotlotMachineInit()) return 1;

	return 0;
}

static INT32 KidnikiInit()
{
	M62Z80RomSize = 0x28000;
	M62PromSize = 0x720;
	M62NumTiles = 0x1000;
	M62NumSprites = 0x800;
	M62NumChars = 0x400;
	M62CharRamSize = 0x1000;
	M62BgxTileDim = 8;
	M62BgyTileDim = 8;
	M62CharxTileDim = 12;
	M62CharyTileDim = 8;
	
	if (M62MemInit()) return 1;
	if (KidnikiLoadRoms()) return 1;
	if (KidnikiMachineInit()) return 1;

	return 0;
}

static INT32 LitheroInit()
{
	M62Z80RomSize = 0x28000;
	M62PromSize = 0x720;
	M62NumTiles = 0x1000;
	M62NumSprites = 0x800;
	M62NumChars = 0x400;
	M62CharRamSize = 0x1000;
	M62BgxTileDim = 8;
	M62BgyTileDim = 8;
	M62CharxTileDim = 12;
	M62CharyTileDim = 8;
	
	if (M62MemInit()) return 1;
	if (LitheroLoadRoms()) return 1;
	if (KidnikiMachineInit()) return 1;

	return 0;
}

static INT32 SpelunkrInit()
{
	M62Z80RomSize = 0x10000;
	M62PromSize = 0x720;
	M62NumTiles = 0x1000;
	M62NumSprites = 0x400;
	M62NumChars = 0x200;
	M62CharRamSize = 0x800;
	M62BgxTileDim = 8;
	M62BgyTileDim = 8;
	M62CharxTileDim = 12;
	M62CharyTileDim = 8;
	
	if (M62MemInit()) return 1;
	if (SpelunkrLoadRoms()) return 1;
	if (SpelunkrMachineInit()) return 1;

	return 0;
}

static INT32 Spelunk2Init()
{
	M62Z80RomSize = 0x1c000;
	M62PromSize = 0x820;
	M62NumTiles = 0x1000;
	M62NumSprites = 0x400;
	M62NumChars = 0x200;
	M62CharRamSize = 0x800;
	M62BgxTileDim = 8;
	M62BgyTileDim = 8;
	M62CharxTileDim = 12;
	M62CharyTileDim = 8;
	
	if (M62MemInit()) return 1;
	if (Spelunk2LoadRoms()) return 1;
	if (Spelunk2MachineInit()) return 1;

	return 0;
}

static INT32 YoujyudnInit()
{
	M62Z80RomSize = 0x10000;
	M62PromSize = 0x720;
	M62NumTiles = 0x400;
	M62NumSprites = 0x400;
	M62NumChars = 0x400;
	M62CharRamSize = 0x800;
	M62BgxTileDim = 8;
	M62BgyTileDim = 16;
	M62CharxTileDim = 12;
	M62CharyTileDim = 8;
	
	if (M62MemInit()) return 1;
	if (YoujyudnLoadRoms()) return 1;
	if (YoujyudnMachineInit()) return 1;

	return 0;
}

static INT32 HorizonInit()
{
	M62Z80RomSize = 0xc000;
	M62PromSize = 0x720;
	M62NumTiles = 0x400;
	M62NumSprites = 0x400;
	M62SpriteRamSize = 0x200;
	M62ScrollRamSize = 0x40;
	
	if (M62MemInit()) return 1;
	if (HorizonLoadRoms()) return 1;
	if (HorizonMachineInit()) return 1;

	return 0;
}

static INT32 M62Exit()
{
	ZetExit();
	M6800Exit();
	AY8910Exit(0);
	AY8910Exit(1);
	MSM5205Exit();

	GenericTilesExit();
	
	BurnFree(Mem);
	
	M62Z80RomSize = 0;
	M62PromSize = 0;
	M62NumTiles = 0;
	M62NumSprites = 0;
	M62NumChars = 0;
	M62SpriteRamSize = 0;
	M62CharRamSize = 0;
	M62ScrollRamSize = 0;
	
	M62BackgroundHScroll = 0;
	M62BackgroundVScroll = 0;
	M62CharHScroll = 0;
	M62CharVScroll = 0;
	M62FlipScreen = 0;
	M62SoundLatch = 0;
	M62Port1 = 0;
	M62Port2 = 0;
	M62SlaveMSM5205VClckReset = 0;
	M62PaletteEntries = 0;
	M62Z80Clock = 0;
	M62M6803Clock = 0;
	M62RenderFunction = NULL;
	M62ExtendTileInfoFunction = NULL;
	M62ExtendCharInfoFunction = NULL;
	M62BankControl[0] = M62BankControl[1] = 0;
	Ldrun2BankSwap = 0;
	Ldrun3TopBottomMask = 0;
	KidnikiBackgroundBank = 0;
	SpelunkrPaletteBank = 0;
	M62BgxTileDim = 0;
	M62BgyTileDim = 0;
	M62CharxTileDim = 0;
	M62CharyTileDim = 0;
	M62SpriteHeightPromOffset = 0;

	return 0;
}

static void M62CalcPalette()
{
	UINT8 *ColourProm = (UINT8*)M62PromData;
	
	for (UINT32 i = 0; i < M62PaletteEntries; i++) {
		INT32 Bit0, Bit1, Bit2, Bit3, r, g, b;

		Bit0 = (ColourProm[M62PaletteEntries * 0] >> 0) & 0x01;
		Bit1 = (ColourProm[M62PaletteEntries * 0] >> 1) & 0x01;
		Bit2 = (ColourProm[M62PaletteEntries * 0] >> 2) & 0x01;
		Bit3 = (ColourProm[M62PaletteEntries * 0] >> 3) & 0x01;
		r =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		Bit0 = (ColourProm[M62PaletteEntries * 1] >> 0) & 0x01;
		Bit1 = (ColourProm[M62PaletteEntries * 1] >> 1) & 0x01;
		Bit2 = (ColourProm[M62PaletteEntries * 1] >> 2) & 0x01;
		Bit3 = (ColourProm[M62PaletteEntries * 1] >> 3) & 0x01;
		g =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		Bit0 = (ColourProm[M62PaletteEntries * 2] >> 0) & 0x01;
		Bit1 = (ColourProm[M62PaletteEntries * 2] >> 1) & 0x01;
		Bit2 = (ColourProm[M62PaletteEntries * 2] >> 2) & 0x01;
		Bit3 = (ColourProm[M62PaletteEntries * 2] >> 3) & 0x01;
		b =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		M62Palette[i] = BurnHighCol(r, g, b, 0);

		ColourProm++;
	}
}

static void BattroadCalcPalette()
{
	UINT8 *ColourProm = (UINT8*)M62PromData;
	
	for (UINT32 i = 0; i < 0x200; i++) {
		INT32 Bit0, Bit1, Bit2, Bit3, r, g, b;

		Bit0 = (ColourProm[0x000] >> 0) & 0x01;
		Bit1 = (ColourProm[0x000] >> 1) & 0x01;
		Bit2 = (ColourProm[0x000] >> 2) & 0x01;
		Bit3 = (ColourProm[0x000] >> 3) & 0x01;
		r =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		Bit0 = (ColourProm[0x200] >> 0) & 0x01;
		Bit1 = (ColourProm[0x200] >> 1) & 0x01;
		Bit2 = (ColourProm[0x200] >> 2) & 0x01;
		Bit3 = (ColourProm[0x200] >> 3) & 0x01;
		g =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		Bit0 = (ColourProm[0x400] >> 0) & 0x01;
		Bit1 = (ColourProm[0x400] >> 1) & 0x01;
		Bit2 = (ColourProm[0x400] >> 2) & 0x01;
		Bit3 = (ColourProm[0x400] >> 3) & 0x01;
		b =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		M62Palette[i] = BurnHighCol(r, g, b, 0);

		ColourProm++;
	}
	
	ColourProm = (UINT8*)M62PromData + 0x720;
	
	for (UINT32 i = 0; i < 0x20; i++) {
		INT32 Bit0, Bit1, Bit2, r, g, b;

		Bit0 = (ColourProm[i] >> 0) & 0x01;
		Bit1 = (ColourProm[i] >> 1) & 0x01;
		Bit2 = (ColourProm[i] >> 2) & 0x01;
		r = 0x21 * Bit0 + 0x47 * Bit1 + 0x97 * Bit2;
		
		Bit0 = (ColourProm[i] >> 3) & 0x01;
		Bit1 = (ColourProm[i] >> 4) & 0x01;
		Bit2 = (ColourProm[i] >> 5) & 0x01;
		g = 0x21 * Bit0 + 0x47 * Bit1 + 0x97 * Bit2;
		
		Bit0 = 0;
		Bit1 = (ColourProm[i] >> 6) & 0x01;
		Bit2 = (ColourProm[i] >> 7) & 0x01;
		b = 0x21 * Bit0 + 0x47 * Bit1 + 0x97 * Bit2;

		M62Palette[i + 0x200] = BurnHighCol(r, g, b, 0);
	}
}

static void Spelunk2CalcPalette()
{
	UINT8 *ColourProm = (UINT8*)M62PromData;
	
	for (UINT32 i = 0; i < 0x200; i++) {
		INT32 Bit0, Bit1, Bit2, Bit3, r, g, b;

		Bit0 = (ColourProm[0x000] >> 0) & 0x01;
		Bit1 = (ColourProm[0x000] >> 1) & 0x01;
		Bit2 = (ColourProm[0x000] >> 2) & 0x01;
		Bit3 = (ColourProm[0x000] >> 3) & 0x01;
		r =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		Bit0 = (ColourProm[0x000] >> 4) & 0x01;
		Bit1 = (ColourProm[0x000] >> 5) & 0x01;
		Bit2 = (ColourProm[0x000] >> 6) & 0x01;
		Bit3 = (ColourProm[0x000] >> 7) & 0x01;
		g =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		Bit0 = (ColourProm[0x200] >> 0) & 0x01;
		Bit1 = (ColourProm[0x200] >> 1) & 0x01;
		Bit2 = (ColourProm[0x200] >> 2) & 0x01;
		Bit3 = (ColourProm[0x200] >> 3) & 0x01;
		b =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		M62Palette[i] = BurnHighCol(r, g, b, 0);

		ColourProm++;
	}
	
	ColourProm += 0x200;

	for (UINT32 i = 0; i < 0x100; i++) {
		INT32 Bit0, Bit1, Bit2, Bit3, r, g, b;

		Bit0 = (ColourProm[0x000] >> 0) & 0x01;
		Bit1 = (ColourProm[0x000] >> 1) & 0x01;
		Bit2 = (ColourProm[0x000] >> 2) & 0x01;
		Bit3 = (ColourProm[0x000] >> 3) & 0x01;
		r =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		Bit0 = (ColourProm[0x100] >> 0) & 0x01;
		Bit1 = (ColourProm[0x100] >> 1) & 0x01;
		Bit2 = (ColourProm[0x100] >> 2) & 0x01;
		Bit3 = (ColourProm[0x100] >> 3) & 0x01;
		g =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		Bit0 = (ColourProm[0x200] >> 0) & 0x01;
		Bit1 = (ColourProm[0x200] >> 1) & 0x01;
		Bit2 = (ColourProm[0x200] >> 2) & 0x01;
		Bit3 = (ColourProm[0x200] >> 3) & 0x01;
		b =  0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		M62Palette[i + 0x200] = BurnHighCol(r, g, b, 0);

		ColourProm++;
	}
}

static void BattroadExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32* xFlip)
{
	*Code |= ((*Colour & 0x40) << 3) | ((*Colour & 0x10) << 4);
	
	*xFlip = *Colour & 0x20;
	
	if (((*Colour & 0x1f) >> 1) >= 0x04) {
		*Priority = 1;
	} else {
		*Priority = 0;
	}
	
	*Colour &= 0x0f;
}

static void LdrunExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32* xFlip)
{
	*Code |= (*Colour & 0xc0) << 2;
	
	*xFlip = *Colour & 0x20;
	
	if (((*Colour & 0x1f) >> 1) >= 0x0c) {
		*Priority = 1;
	} else {
		*Priority = 0;
	}
	
	*Colour &= 0x1f;
}

static void Ldrun2ExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32* xFlip)
{
	*Code |= (*Colour & 0xc0) << 2;
	
	*xFlip = *Colour & 0x20;
	
	if (((*Colour & 0x1f) >> 1) >= 0x04) {
		*Priority = 1;
	} else {
		*Priority = 0;
	}
	
	*Colour &= 0x1f;
}

static void Ldrun4ExtendTile(INT32* Code, INT32* Colour, INT32*, INT32*)
{
	*Code |= ((*Colour & 0xc0) << 2) | ((*Colour & 0x20) << 5);
	*Colour &= 0x1f;
}

static void LotlotExtendTile(INT32* Code, INT32* Colour, INT32*, INT32* xFlip)
{
	*Code |= (*Colour & 0xc0) << 2;
	*xFlip = *Colour & 0x20;
	*Colour &= 0x1f;
}

static void KidnikiExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32*)
{
	*Code |= ((*Colour & 0xe0) << 3) | (KidnikiBackgroundBank << 11);
	*Priority = ((*Colour & 0xe0) == 0xe0) ? 1 : 0;	
	*Colour &= 0x1f;
}

static void SpelunkrExtendTile(INT32* Code, INT32* Colour, INT32*, INT32*)
{
	*Code |= ((*Colour & 0x10) << 4) | ((*Colour & 0x20) << 6) | ((*Colour & 0xc0) << 3);
	*Colour &= 0x0f | (SpelunkrPaletteBank << 4);
}

static void Spelunk2ExtendTile(INT32* Code, INT32* Colour, INT32*, INT32*)
{
	*Code |= ((*Colour & 0xf0) << 4);
	*Colour &= 0x0f | (SpelunkrPaletteBank << 4);
}

static void YoujyudnExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32*)
{
	*Code |= (*Colour & 0x60) << 3;
	if (((*Colour & 0x1f) >> 1) >= 0x08) {
		*Priority = 1;
	} else {
		*Priority = 0;
	}
	*Colour &= 0x1f;
}

static void HorizonExtendTile(INT32* Code, INT32* Colour, INT32* Priority, INT32*)
{
	*Code |= ((*Colour & 0xc0) << 2) | ((*Colour & 0x20) << 5);
	if (((*Colour & 0x1f) >> 1) >= 0x08) {
		*Priority = 1;
	} else {
		*Priority = 0;
	}
	*Colour &= 0x1f;
}
		
static void M62RenderBgLayer(INT32 PriorityToRender, INT32 xOffset, INT32 yOffset, INT32 Cols, INT32 Rows, INT32 Transparent)
{
	INT32 Code, mx, my, Colour, x, y, TileIndex = 0, Priority, xFlip, yFlip;
	
	for (my = 0; my < Rows; my++) {
		for (mx = 0; mx < Cols; mx++) {
			Code = M62TileRam[TileIndex << 1];
			Colour = M62TileRam[(TileIndex << 1) | 0x01];
			Priority = 0;
			xFlip = 0;
			yFlip = 0;
			
			if (M62ExtendTileInfoFunction) M62ExtendTileInfoFunction(&Code, &Colour, &Priority, &xFlip);
			Code &= (M62NumTiles - 1);
			
			x = M62BgxTileDim * mx;
			y = M62BgyTileDim * my;
		
			if (M62FlipScreen) {
				xFlip = !xFlip;
				yFlip = !yFlip;
				
				y = (Rows * M62BgyTileDim) - M62BgyTileDim - y;
				x = (Cols * M62BgxTileDim) - M62BgxTileDim - x;
			}

			x -= xOffset;
			y -= yOffset;
			
			if (M62FlipScreen) {
				x += M62BackgroundHScroll & (Cols * M62BgxTileDim - 1);
			} else {
				x -= M62BackgroundHScroll & (Cols * M62BgxTileDim - 1);
			}
			
			y -= M62BackgroundVScroll & (Rows * M62BgyTileDim - 1);
	
			INT32 px, py;
		
			UINT32 nPalette = Colour << 3;
			
			if (Priority == PriorityToRender) {
				for (py = 0; py < M62BgyTileDim; py++) {
					for (px = 0; px < M62BgxTileDim; px++) {
						UINT8 c = M62Tiles[(Code * M62BgxTileDim * M62BgyTileDim) + (py * M62BgxTileDim) + px];
						if (xFlip) c = M62Tiles[(Code *M62BgxTileDim * M62BgyTileDim) + (py * M62BgxTileDim) + (M62BgxTileDim - 1 - px)];
						if (yFlip) c = M62Tiles[(Code * M62BgxTileDim * M62BgyTileDim) + ((M62BgyTileDim - 1 - py) * M62BgxTileDim) + px];
						if (xFlip && yFlip) c = M62Tiles[(Code * M62BgxTileDim * M62BgyTileDim) + ((M62BgyTileDim - 1 - py) * M62BgxTileDim) + (M62BgxTileDim - 1 - px)];
						
						if (Transparent && c == 0x00) continue;
						
						INT32 xPos = x + px;
						INT32 yPos = y + py;
						
						if (M62ScrollRamSize) {
//							tilemap_set_scrollx(state->bg_tilemap, i, state->scrollram[i << 1] | (state->scrollram[(i << 1) | 1] << 8));
							
							xPos -= (M62ScrollRam[my << 1] | (M62ScrollRam[(my << 1) | 0x01] << 8)) & (Cols * M62BgxTileDim - 1);
						}
						
						if (xPos < 0) xPos += Cols * M62BgxTileDim;
						if (xPos > (Cols * M62BgxTileDim - 1)) xPos -= Cols * M62BgxTileDim;
						
						if (yPos < 0) yPos += Rows * M62BgyTileDim;
						if (yPos > (Rows * M62BgyTileDim - 1)) yPos -= Rows * M62BgyTileDim;
					
						if (yPos >= 0 && yPos < nScreenHeight) {					
							UINT16* pPixel = pTransDraw + (yPos * nScreenWidth);
					
							if (xPos >= 0 && xPos < nScreenWidth) {
								pPixel[xPos] = c | nPalette;
							}
						}
					}
				}
			}
		
			TileIndex++;
		}
	}
}

static void KungfumRenderBgLayer(INT32 PriorityToRender, INT32 Cols, INT32 Rows, INT32 Transparent)
{
	INT32 Code, mx, my, Colour, x, y, TileIndex = 0, Priority, xFlip, yFlip;
	
	for (my = 0; my < Rows; my++) {
		for (mx = 0; mx < Cols; mx++) {
			Code = M62TileRam[TileIndex];
			Colour = M62TileRam[TileIndex + 0x800];
			xFlip = Colour & 0x20;
			yFlip = 0;
			
			Code |= (Colour & 0xc0) << 2;			
			Code &= (M62NumTiles - 1);
			
			if ((TileIndex / 64) << 6 || ((Colour & 0x1f) >> 1) > 0x0c) {
				Priority = 1;
			} else {
				Priority = 0;
			}
			
			x = M62BgxTileDim * mx;
			y = M62BgyTileDim * my;
		
			if (M62FlipScreen) {
				xFlip = !xFlip;
				yFlip = !yFlip;
				
				y = (Rows * M62BgyTileDim - 1) - M62BgyTileDim - y;
				x = (Cols * M62BgxTileDim - 1) - M62BgxTileDim - x;
			}

			x -= 128;
	
			INT32 px, py;
		
			UINT32 nPalette = (Colour & 0x1f) << 3;
			
			if (Priority == PriorityToRender) {
				for (py = 0; py < M62BgyTileDim; py++) {
					for (px = 0; px < M62BgxTileDim; px++) {
						UINT8 c = M62Tiles[(Code * M62BgxTileDim * M62BgyTileDim) + (py * M62BgxTileDim) + px];
						if (xFlip) c = M62Tiles[(Code *M62BgxTileDim * M62BgyTileDim) + (py * M62BgxTileDim) + (M62BgxTileDim - 1 - px)];
						if (yFlip) c = M62Tiles[(Code * M62BgxTileDim * M62BgyTileDim) + ((M62BgyTileDim - 1 - py) * M62BgxTileDim) + px];
						if (xFlip && yFlip) c = M62Tiles[(Code * M62BgxTileDim * M62BgyTileDim) + ((M62BgyTileDim - 1 - py) * M62BgxTileDim) + (M62BgxTileDim - 1 - px)];
				
						if (Transparent && c == 0x00) continue;
						
						INT32 xPos = x + px;
						INT32 yPos = y + py;
						
						if (my >= 6) {
							if (M62FlipScreen) {
								xPos += M62BackgroundHScroll & (Cols * M62BgxTileDim - 1);
							} else {
								xPos -= M62BackgroundHScroll & (Cols * M62BgxTileDim - 1);
							}
						}
						
						if (xPos < 0) xPos += Cols * M62BgxTileDim;
						if (xPos > (Cols * M62BgxTileDim - 1)) xPos -= Cols * M62BgxTileDim;
						
						if (yPos < 0) yPos += Rows * M62BgyTileDim;
						if (yPos > (Rows * M62BgyTileDim - 1)) yPos -= Rows * M62BgyTileDim;
					
						if (yPos >= 0 && yPos < nScreenHeight) {					
							UINT16* pPixel = pTransDraw + (yPos * nScreenWidth);
					
							if (xPos >= 0 && xPos < nScreenWidth) {
								pPixel[xPos] = c | nPalette;
							}
						}
					}
				}
			}
		
			TileIndex++;
		}
	}
}

static void M62RenderSprites(INT32 ColourMask, INT32 PriorityMask, INT32 Priority, INT32 VisibleOffset, INT32 PaletteOffset)
{
	for (UINT32 Offset = 0; Offset < M62SpriteRamSize; Offset += 8) {
		INT32 i, Incr, Code, Colour, xFlip, yFlip, sx, sy;

		if ((M62SpriteRam[Offset] & PriorityMask) == Priority) {
			Code = M62SpriteRam[Offset + 4] + ((M62SpriteRam[Offset + 5] & 0x07) << 8);
			Colour = M62SpriteRam[Offset + 0] & ColourMask;
			sx = 256 * (M62SpriteRam[Offset + 7] & 1) + M62SpriteRam[Offset + 6],
			sy = 256 + 128 - 15 - (256 * (M62SpriteRam[Offset + 3] & 1) + M62SpriteRam[Offset + 2]),
			xFlip = M62SpriteRam[Offset + 5] & 0x40;
			yFlip = M62SpriteRam[Offset + 5] & 0x80;

			i = M62PromData[M62SpriteHeightPromOffset + ((Code >> 5) & 0x1f)];
			if (i == 1)	{
				// double height
				Code &= ~1;
				sy -= 16;
			} else if (i == 2) {
				// quadruple height
				i = 3;
				Code &= ~3;
				sy -= 3*16;
			}

			if (M62FlipScreen) {
				sx = 496 - sx;
				sy = 242 - (i * 16) - sy;
				xFlip = !xFlip;
				yFlip = !yFlip;
			}

			if (yFlip) {
				Incr = -1;
				Code += i;
			} else {
				Incr = 1;
			}
			
			sx -= VisibleOffset;

			do {
				INT32 DrawCode = Code + (i * Incr);
				INT32 DrawY = sy + (i * 16);
				DrawCode &= (M62NumSprites - 1);
				
				if (sx > 15 && sx < (nScreenWidth - 16) && DrawY > 15 && DrawY < (nScreenHeight - 16)) {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY(pTransDraw, DrawCode, sx, DrawY, Colour, 3, 0, PaletteOffset, M62Sprites);
						} else {
							Render16x16Tile_Mask_FlipX(pTransDraw, DrawCode, sx, DrawY, Colour, 3, 0, PaletteOffset, M62Sprites);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY(pTransDraw, DrawCode, sx, DrawY, Colour, 3, 0, PaletteOffset, M62Sprites);
						} else {
							Render16x16Tile_Mask(pTransDraw, DrawCode, sx, DrawY, Colour, 3, 0, PaletteOffset, M62Sprites);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, DrawCode, sx, DrawY, Colour, 3, 0, PaletteOffset, M62Sprites);
						} else {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, DrawCode, sx, DrawY, Colour, 3, 0, PaletteOffset, M62Sprites);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, DrawCode, sx, DrawY, Colour, 3, 0, PaletteOffset, M62Sprites);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, DrawCode, sx, DrawY, Colour, 3, 0, PaletteOffset, M62Sprites);
						}
					}
				}
				
				i--;
			} while (i >= 0);
		}
	}
}

static void BattroadExtendChar(INT32* Code, INT32* Colour, INT32*, INT32*)
{
	*Code |= ((*Colour & 0x40) << 3) | ((*Colour & 0x10) << 4);
	*Colour &= 0x0f;
}

static void LotlotExtendChar(INT32* Code, INT32* Colour, INT32*, INT32*)
{
	*Code |= (*Colour & 0xc0) << 2;
	*Colour &= 0x1f;
}

static void SpelunkrExtendChar(INT32* Code, INT32* Colour, INT32*, INT32*)
{
	*Code |= (*Colour & 0x10) << 4;
	*Colour &= 0x0f | (SpelunkrPaletteBank << 4);
}

static void YoujyudnExtendChar(INT32* Code, INT32* Colour, INT32*, INT32*)
{
	*Code |= (*Colour & 0xc0) << 2;
	*Colour &= 0x0f;
}

static void M62RenderCharLayer(INT32 Cols, INT32 Rows, INT32 ColourDepth, INT32 xOffset, INT32 yOffset, INT32 PaletteOffset)
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0, xFlip, yFlip;

	for (my = 0; my < Rows; my++) {
		for (mx = 0; mx < Cols; mx++) {
			Code = M62CharRam[TileIndex << 1];
			Colour = M62CharRam[(TileIndex << 1) | 0x01];
			xFlip = 0;
			yFlip = 0;
			
			if (M62ExtendCharInfoFunction) M62ExtendCharInfoFunction(&Code, &Colour, 0, 0);
			Code &= (M62NumChars - 1);

			x = M62CharxTileDim * mx;
			y = M62CharyTileDim * my;
			
			x -= xOffset;
			y -= yOffset;
			
			if (M62FlipScreen) {
				x += M62CharHScroll & (Cols * M62CharxTileDim - 1);
			} else {
				x -= M62CharHScroll & (Cols * M62CharxTileDim - 1);
			}
			
			y -= M62CharVScroll & (Rows * M62CharyTileDim - 1);
			
			INT32 px, py;
		
			UINT32 nPalette = Colour << ColourDepth;
			
			for (py = 0; py < M62CharyTileDim; py++) {
				for (px = 0; px < M62CharxTileDim; px++) {
					UINT8 c = M62Chars[(Code * M62CharxTileDim * M62CharyTileDim) + (py * M62CharxTileDim) + px];
					if (xFlip) c = M62Chars[(Code *M62CharxTileDim * M62CharyTileDim) + (py * M62CharxTileDim) + (M62CharxTileDim - 1 - px)];
					if (yFlip) c = M62Chars[(Code * M62CharxTileDim * M62CharyTileDim) + ((M62CharyTileDim - 1 - py) * M62CharxTileDim) + px];
					if (xFlip && yFlip) c = M62Chars[(Code * M62CharxTileDim * M62CharyTileDim) + ((M62CharyTileDim - 1 - py) * M62CharxTileDim) + (M62CharxTileDim - 1 - px)];
		
					if (c != 0) {
						INT32 xPos = x + px;
						INT32 yPos = y + py;
					
						if (xPos < 0) xPos += Cols * M62CharxTileDim;
						if (xPos > (Cols * M62CharxTileDim - 1)) xPos -= Cols * M62CharxTileDim;
						
						if (yPos < 0) yPos += Rows * M62CharyTileDim;
						if (yPos > (Rows * M62CharyTileDim - 1)) yPos -= Rows * M62CharyTileDim;
				
						if (yPos >= 0 && yPos < nScreenHeight) {					
							UINT16* pPixel = pTransDraw + (yPos * nScreenWidth);
					
							if (xPos >= 0 && xPos < nScreenWidth) {
								pPixel[xPos] = c | nPalette | PaletteOffset;
							}
						}
					}
				}
			}

			TileIndex++;
		}
	}
}

static void KungfumDraw()
{
	BurnTransferClear();
	M62CalcPalette();
	KungfumRenderBgLayer(0, 64, 32, 0);
	KungfumRenderBgLayer(1, 64, 32, 0);
	M62RenderSprites(0x1f, 0, 0, 128, 256);
	KungfumRenderBgLayer(0, 64, 32, 1);
	BurnTransferCopy(M62Palette);
}

static void BattroadDraw()
{
	BurnTransferClear();
	BattroadCalcPalette();
	M62RenderBgLayer(0, 128, 0, 64, 32, 0);
	M62RenderBgLayer(1, 128, 0, 64, 32, 0);
	M62RenderSprites(0x0f, 0x10, 0x00, 128, 256);
	M62RenderBgLayer(1, 128, 0, 64, 32, 1);
	M62RenderSprites(0x0f, 0x10, 0x10, 128, 256);
	M62RenderCharLayer(32, 32, 2, 0, 0, 512);
	BurnTransferCopy(M62Palette);
}

static void LdrunDraw()
{
	BurnTransferClear();
	M62CalcPalette();
	M62RenderBgLayer(0, 64, 0, 64, 32, 0);
	M62RenderBgLayer(1, 64, 0, 64, 32, 0);
	M62RenderSprites(0x0f, 0x10, 0x00, 64, 256);
	M62RenderBgLayer(1, 64, 0, 64, 32, 1);
	M62RenderSprites(0x0f, 0x10, 0x10, 64, 256);
	BurnTransferCopy(M62Palette);
}

static void Ldrun3Draw()
{
	BurnTransferClear();
	M62CalcPalette();
	M62RenderBgLayer(0, 64, 0, 64, 32, 0);
	M62RenderBgLayer(1, 64, 0, 64, 32, 0);
	M62RenderSprites(0x0f, 0x10, 0x00, 64, 256);
	M62RenderBgLayer(1, 64, 0, 64, 32, 1);
	M62RenderSprites(0x0f, 0x10, 0x10, 64, 256);	
	
	if (Ldrun3TopBottomMask) {
		INT32 x, y;
		
		for (x = 0; x < nScreenWidth; x++) {
			for (y = 0; y < 8; y++) {
				pTransDraw[(y * nScreenWidth) + x] = BurnHighCol(0, 0, 0, 0);
				pTransDraw[((y + 248) * nScreenWidth) + x] = BurnHighCol(0, 0, 0, 0);
			}
		}
	}
	
	BurnTransferCopy(M62Palette);
}

static void Ldrun4Draw()
{
	BurnTransferClear();
	M62CalcPalette();
	M62RenderBgLayer(0, 64 - 2, 0, 64, 32, 0);
	M62RenderSprites(0x0f, 0x00, 0x00, 64, 256);
	BurnTransferCopy(M62Palette);
}

static void LotlotDraw()
{
	M62BackgroundVScroll = 32;
	M62BackgroundHScroll = -64;
	
	BurnTransferClear();
	M62CalcPalette();
	M62RenderBgLayer(0, 64, 0, 32, 64, 0);
	M62RenderCharLayer(32, 64, 3, -64 + 64, 32, 512);
	M62RenderSprites(0x1f, 0x00, 0x00, 64, 256);
	BurnTransferCopy(M62Palette);
}

static void KidnikiDraw()
{
	BurnTransferClear();
	M62CalcPalette();
	M62RenderBgLayer(0, 64, 0, 64, 32, 0);
	M62RenderBgLayer(1, 64, 0, 64, 32, 0);
	M62RenderSprites(0x1f, 0x00, 0x00, 64, 256);
	M62RenderBgLayer(1, 64, 0, 64, 32, 1);
	M62RenderCharLayer(32, 64, 3, 0, 128, 0);
	BurnTransferCopy(M62Palette);
}

static void SpelunkrDraw()
{
	BurnTransferClear();
	M62CalcPalette();
	M62RenderBgLayer(0, 64, 128, 64, 64, 0);
	M62RenderSprites(0x1f, 0x00, 0x00, 64, 256);
	M62RenderCharLayer(32, 32, 3, 0, 0, 0);
	BurnTransferCopy(M62Palette);
}

static void Spelunk2Draw()
{
	BurnTransferClear();
	Spelunk2CalcPalette();
	M62RenderBgLayer(0, 64 - 1, 128, 64, 64, 0);
	M62RenderSprites(0x1f, 0x00, 0x00, 64, 512);
	M62RenderCharLayer(32, 32, 3, 64 - 65, 0, 0);
	BurnTransferCopy(M62Palette);
}

static void YoujyudnDraw()
{
	BurnTransferClear();
	M62CalcPalette();
	M62RenderBgLayer(0, 128, 0, 64, 16, 0);
	M62RenderBgLayer(1, 128, 0, 64, 16, 0);
	M62RenderSprites(0x1f, 0x00, 0x00, 128, 256);
	M62RenderBgLayer(1, 128, 0, 64, 16, 1);
	M62RenderCharLayer(32, 32, 3, 64, 0, 128);
	BurnTransferCopy(M62Palette);
}

static void HorizonDraw()
{
	BurnTransferClear();
	M62CalcPalette();
	M62RenderBgLayer(0, 128, 0, 64, 32, 0);
	M62RenderBgLayer(1, 128, 0, 64, 32, 0);
	M62RenderSprites(0x1f, 0x00, 0x00, 128, 256);
	M62RenderBgLayer(1, 128, 0, 64, 32, 1);
	BurnTransferCopy(M62Palette);
}

static INT32 M62Frame()
{
	INT32 nInterleave = MSM5205CalcInterleave(0, M62Z80Clock);
	INT32 nSoundBufferPos = 0;

	if (M62Reset) M62DoReset();

	M62MakeInputs();

//	nCyclesTotal[0] = M62Z80Clock / 55;
//	nCyclesTotal[1] = M62M6803Clock / 55;
	nCyclesTotal[0] = M62Z80Clock / 60;
	nCyclesTotal[1] = M62M6803Clock / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	ZetNewFrame();
	M6803NewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		nCurrentCPU = 0;
		ZetOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == (nInterleave - 1)) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		ZetClose();
		
		nCurrentCPU = 1;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = M6803Run(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
						
		if (pBurnSoundOut) {
			INT32 nSample;
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
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
		
		ZetOpen(0);
		MSM5205Update();
		if (M62SlaveMSM5205VClckReset) {
			MSM5205VCLKWrite(1, 1);
			MSM5205VCLKWrite(1, 0);
			M62SlaveMSM5205VClckReset = 0;
		}
		ZetClose();
	}
	
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
		
		ZetOpen(0);
		MSM5205Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM5205Render(1, pBurnSoundOut, nBurnSoundLen);
		ZetClose();
	}
	
	if (pBurnDraw) {
		if (M62RenderFunction) M62RenderFunction();
	}
	
	return 0;
}

static INT32 M62Scan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029709;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {

	}

	return 0;
}

struct BurnDriver BurnDrvKungfum = {
	"kungfum", NULL, NULL, NULL, "1984",
	"Kung-Fu Master\0", NULL, "Irem", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M62, GBF_SCRFIGHT, 0,
	NULL, KungfumRomInfo, KungfumRomName, NULL, NULL, M62InputInfo, KungfumDIPInfo,
	KungfumInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 256, 256, 4, 3
};

struct BurnDriver BurnDrvKungfumd = {
	"kungfumd", "kungfum", NULL, NULL, "1984",
	"Kung-Fu Master (Data East)\0", NULL, "Irem (Data East License)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M62, GBF_SCRFIGHT, 0,
	NULL, KungfumdRomInfo, KungfumdRomName, NULL, NULL, M62InputInfo, KungfumDIPInfo,
	KungfumdInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 256, 256, 4, 3
};

struct BurnDriver BurnDrvSpartanx = {
	"spartanx", "kungfum", NULL, NULL, "1984",
	"Spartan X (Japan)\0", NULL, "Irem", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M62, GBF_SCRFIGHT, 0,
	NULL, SpartanxRomInfo, SpartanxRomName, NULL, NULL, M62InputInfo, KungfumDIPInfo,
	KungfumInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 256, 256, 4, 3
};

struct BurnDriver BurnDrvKungfub = {
	"kungfub", "kungfum", NULL, NULL, "1984",
	"Kung-Fu Master (bootleg set 1)\0", NULL, "bootleg", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_IREM_M62, GBF_SCRFIGHT, 0,
	NULL, KungfubRomInfo, KungfubRomName, NULL, NULL, M62InputInfo, KungfumDIPInfo,
	KungfumInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 256, 256, 4, 3
};

struct BurnDriver BurnDrvKungfub2 = {
	"kungfub2", "kungfum", NULL, NULL, "1984",
	"Kung-Fu Master (bootleg set 2)\0", NULL, "bootleg", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_IREM_M62, GBF_SCRFIGHT, 0,
	NULL, Kungfub2RomInfo, Kungfub2RomName, NULL, NULL, M62InputInfo, KungfumDIPInfo,
	KungfumInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 256, 256, 4, 3
};

struct BurnDriver BurnDrvBattroad = {
	"battroad", NULL, NULL, NULL, "1984",
	"The Battle-Road\0", NULL, "Irem", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_IREM_M62, GBF_SCRFIGHT, 0,
	NULL, BattroadRomInfo, BattroadRomName, NULL, NULL, M62InputInfo, BattroadDIPInfo,
	BattroadInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x220, 256, 256, 3, 4
};

struct BurnDriver BurnDrvLdrun = {
	"ldrun", NULL, NULL, NULL, "1984",
	"Lode Runner (set 1)\0", NULL, "Irem (licensed from Broderbund)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, LdrunRomInfo, LdrunRomName, NULL, NULL, M62InputInfo, LdrunDIPInfo,
	LdrunInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvLdruna = {
	"ldruna", "ldrun", NULL, NULL, "1984",
	"Lode Runner (set 2)\0", NULL, "Irem (licensed from Broderbund)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, LdrunaRomInfo, LdrunaRomName, NULL, NULL, M62InputInfo, LdrunDIPInfo,
	LdrunInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvLdrun2 = {
	"ldrun2", NULL, NULL, NULL, "1984",
	"Lode Runner II - The Bungeling Strikes Back\0", NULL, "Irem (licensed from Broderbund)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, Ldrun2RomInfo, Ldrun2RomName, NULL, NULL, M62InputInfo, Ldrun2DIPInfo,
	Ldrun2Init, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvLdrun3 = {
	"ldrun3", NULL, NULL, NULL, "1985",
	"Lode Runner III - The Golden Labyrinth\0", NULL, "Irem (licensed from Broderbund)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, Ldrun3RomInfo, Ldrun3RomName, NULL, NULL, M62InputInfo, Ldrun2DIPInfo,
	Ldrun3Init, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvLdrun3j = {
	"ldrun3j", "ldrun3", NULL, NULL, "1985",
	"Lode Runner III - Majin No Fukkatsu\0", NULL, "Irem (licensed from Broderbund)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, Ldrun3jRomInfo, Ldrun3jRomName, NULL, NULL, M62InputInfo, Ldrun2DIPInfo,
	Ldrun3jInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvLdrun4 = {
	"ldrun4", NULL, NULL, NULL, "1986",
	"Lode Runner IV - Teikoku Karano Dasshutsu\0", NULL, "Irem (licensed from Broderbund)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, Ldrun4RomInfo, Ldrun4RomName, NULL, NULL, M62InputInfo, Ldrun4DIPInfo,
	Ldrun4Init, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvLotlot = {
	"lotlot", NULL, NULL, NULL, "1985",
	"Lot Lot\0", NULL, "Irem (licensed from Tokuma Shoten)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M62, GBF_PUZZLE, 0,
	NULL, LotlotRomInfo, LotlotRomName, NULL, NULL, M62InputInfo, LotlotDIPInfo,
	LotlotInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x300, 384, 256, 4, 3
};

struct BurnDriver BurnDrvKidniki = {
	"kidniki", NULL, NULL, NULL, "1986",
	"Kid Niki - Radical Ninja (World)\0", NULL, "Irem", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, KidnikiRomInfo, KidnikiRomName, NULL, NULL, M62InputInfo, KidnikiDIPInfo,
	KidnikiInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvKidnikiu = {
	"kidnikiu", "kidniki", NULL, NULL, "1986",
	"Kid Niki - Radical Ninja (US)\0", NULL, "Irem (Data East USA license)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, KidnikiuRomInfo, KidnikiuRomName, NULL, NULL, M62InputInfo, KidnikiDIPInfo,
	KidnikiInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvYanchamr = {
	"yanchamr", "kidniki", NULL, NULL, "1986",
	"Kaiketsu Yanchamaru (Japan)\0", NULL, "Irem", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, YanchamrRomInfo, YanchamrRomName, NULL, NULL, M62InputInfo, KidnikiDIPInfo,
	KidnikiInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvLithero = {
	"lithero", "kidniki", NULL, NULL, "1987",
	"Little Hero\0", NULL, "bootleg", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, LitheroRomInfo, LitheroRomName, NULL, NULL, M62InputInfo, KidnikiDIPInfo,
	LitheroInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvSpelunkr = {
	"spelunkr", NULL, NULL, NULL, "1985",
	"Spelunker\0", NULL, "Irem (licensed from Broderbund)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, SpelunkrRomInfo, SpelunkrRomName, NULL, NULL, M62InputInfo, SpelunkrDIPInfo,
	SpelunkrInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvSpelunkrj = {
	"spelunkrj", "spelunkr", NULL, NULL, "1985",
	"Spelunker (Japan)\0", NULL, "Irem (licensed from Broderbund)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, SpelunkrjRomInfo, SpelunkrjRomName, NULL, NULL, M62InputInfo, SpelunkrDIPInfo,
	SpelunkrInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 384, 256, 4, 3
};

struct BurnDriver BurnDrvSpelunk2 = {
	"spelunk2", NULL, NULL, NULL, "1986",
	"Spelunker II\0", NULL, "Irem (licensed from Broderbund)", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M62, GBF_PLATFORM, 0,
	NULL, Spelunk2RomInfo, Spelunk2RomName, NULL, NULL, M62InputInfo, Spelunk2DIPInfo,
	Spelunk2Init, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x300, 384, 256, 4, 3
};

struct BurnDriver BurnDrvYoujyudn = {
	"youjyudn", NULL, NULL, NULL, "1986",
	"Youjyuden (Japan)\0", NULL, "Irem", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_IREM_M62, GBF_VERSHOOT, 0,
	NULL, YoujyudnRomInfo, YoujyudnRomName, NULL, NULL, M62InputInfo, YoujyudnDIPInfo,
	YoujyudnInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 256, 256, 3, 4
};

struct BurnDriver BurnDrvHorizon = {
	"horizon", NULL, NULL, NULL, "1985",
	"Horizon\0", NULL, "Irem", "Irem M62",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M62, GBF_HORSHOOT, 0,
	NULL, HorizonRomInfo, HorizonRomName, NULL, NULL, M62InputInfo, HorizonDIPInfo,
	HorizonInit, M62Exit, M62Frame, NULL, M62Scan,
	NULL, 0x200, 256, 256, 4, 3
};
