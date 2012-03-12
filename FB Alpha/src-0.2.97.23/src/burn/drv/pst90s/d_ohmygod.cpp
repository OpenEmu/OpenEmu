#include "tiles_generic.h"
#include "msm6295.h"

// define this to enable basic unmapped memory reads and writes
#define OHMYGODLOG 1

static UINT8  OhmygodInputPort0[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UINT8  OhmygodInputPort1[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UINT8  OhmygodDip[2]         = {0, 0};
static UINT16 OhmygodInput[2]       = {0x00, 0x00};
static UINT8  OhmygodReset          = 0;

static UINT8 *Mem                  = NULL;
static UINT8 *MemEnd               = NULL;
static UINT8 *RamStart             = NULL;
static UINT8 *RamEnd               = NULL;
static UINT8 *OhmygodRom           = NULL;
static UINT8 *OhmygodRam           = NULL;
static UINT8 *OhmygodVideoRam      = NULL;
static UINT8 *OhmygodPaletteRam    = NULL;
static UINT8 *OhmygodSpriteRam     = NULL;
static UINT32 *OhmygodPalette       = NULL;

UINT8 *OhmygodChars;
UINT8 *OhmygodSprites;

static INT32 AdpcmBankShift;
static INT32 SndBank;
static INT32 watch_tick=0;
static INT32 OhmygodSpriteBank;
static UINT32 OhmygodScrollx;
static UINT32 OhmygodScrolly;

static struct BurnInputInfo OhmygodInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , OhmygodInputPort0 + 8, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , OhmygodInputPort0 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , OhmygodInputPort1 + 8, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , OhmygodInputPort1 + 7, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , OhmygodInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , OhmygodInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , OhmygodInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , OhmygodInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , OhmygodInputPort0 + 4, "p1 fire 1" },

	{"P2 Up"             , BIT_DIGITAL  , OhmygodInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , OhmygodInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , OhmygodInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , OhmygodInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , OhmygodInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL  , &OhmygodReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , OhmygodInputPort1 + 9, "service"   },
	{"Diagnostic"        , BIT_DIGITAL  , OhmygodInputPort0 + 9, "diag"      },
	{"Dip 1"             , BIT_DIPSWITCH, OhmygodDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, OhmygodDip + 1       , "dip"       },
};

STDINPUTINFO(Ohmygod)

inline void OhmygodClearOpposites(UINT16* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

inline void OhmygodMakeInputs()
{
	// Reset Inputs
	OhmygodInput[0] = OhmygodInput[1] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 10; i++) {
		OhmygodInput[0] |= (OhmygodInputPort0[i] & 1) << i;
		OhmygodInput[1] |= (OhmygodInputPort1[i] & 1) << i;
	}

	// Clear Opposites
	OhmygodClearOpposites(&OhmygodInput[0]);
	OhmygodClearOpposites(&OhmygodInput[1]);
}

static struct BurnDIPInfo OhmygodDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 15  , "Coin A"                 },
	{0x11, 0x01, 0x0f, 0x05, "6 Coin-3 Play"          },
	{0x11, 0x01, 0x0f, 0x03, "5 Coin-6 Play"          },
	{0x11, 0x01, 0x0f, 0x07, "4 Coin-1 Play"          },
	{0x11, 0x01, 0x0f, 0x04, "4 Coin-3 Play"          },
	{0x11, 0x01, 0x0f, 0x02, "4 Coin-5 Play"          },
	{0x11, 0x01, 0x0f, 0x08, "3 Coin-1 Play"          },
	{0x11, 0x01, 0x0f, 0x09, "2 Coin-1 Play"          },
	{0x11, 0x01, 0x0f, 0x01, "2 Coin-3 Play"          },
	{0x11, 0x01, 0x0f, 0x0f, "1 Coin-1 Play"          },
	{0x11, 0x01, 0x0f, 0x0e, "1 Coin-2 Play"          },
	{0x11, 0x01, 0x0f, 0x0d, "1 Coin-3 Play"          },
	{0x11, 0x01, 0x0f, 0x0c, "1 Coin-4 Play"          },
	{0x11, 0x01, 0x0f, 0x0b, "1 Coin-5 Play"          },
	{0x11, 0x01, 0x0f, 0x0a, "1 Coin-6 Play"          },
	{0x11, 0x01, 0x0f, 0x00, "Free Play"              },

	{0   , 0xfe, 0   , 15  , "Coin B"                 },
	{0x11, 0x01, 0xf0, 0x50, "6 Coin-3 Play"          },
	{0x11, 0x01, 0xf0, 0x30, "5 Coin-6 Play"          },
	{0x11, 0x01, 0xf0, 0x70, "4 Coin-1 Play"          },
	{0x11, 0x01, 0xf0, 0x40, "4 Coin-3 Play"          },
	{0x11, 0x01, 0xf0, 0x20, "4 Coin-5 Play"          },
	{0x11, 0x01, 0xf0, 0x80, "3 Coin-1 Play"          },
	{0x11, 0x01, 0xf0, 0x90, "2 Coin-1 Play"          },
	{0x11, 0x01, 0xf0, 0x10, "2 Coin-3 Play"          },
	{0x11, 0x01, 0xf0, 0xf0, "1 Coin-1 Play"          },
	{0x11, 0x01, 0xf0, 0xe0, "1 Coin-2 Play"          },
	{0x11, 0x01, 0xf0, 0xd0, "1 Coin-3 Play"          },
	{0x11, 0x01, 0xf0, 0xc0, "1 Coin-4 Play"          },
	{0x11, 0x01, 0xf0, 0xb0, "1 Coin-5 Play"          },
	{0x11, 0x01, 0xf0, 0xa0, "1 Coin-6 Play"          },
	{0x11, 0x01, 0xf0, 0x00, "Free Play"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "1P Difficulty"          },
	{0x12, 0x01, 0x03, 0x02, "Easy"                   },
	{0x12, 0x01, 0x03, 0x03, "Normal"                 },
	{0x12, 0x01, 0x03, 0x01, "Hard"                   },
	{0x12, 0x01, 0x03, 0x00, "Very Hard"              },

	{0   , 0xfe, 0   , 4   , "VS Difficulty"          },
	{0x12, 0x01, 0x0c, 0x0c, "Normal Jake"            },
	{0x12, 0x01, 0x0c, 0x08, "Hard Jake"              },
	{0x12, 0x01, 0x0c, 0x04, "Normal"                 },
	{0x12, 0x01, 0x0c, 0x00, "Hard"                   },

	{0   , 0xfe, 0   , 2   , "VS Matches/Credit"      },
	{0x12, 0x01, 0x10, 0x00, "1 Game Match"           },
	{0x12, 0x01, 0x10, 0x10, "3 Game Match"           },

	{0   , 0xfe, 0   , 2   , "Attract Sound"          },
	{0x12, 0x01, 0x20, 0x00, "Off"                    },
	{0x12, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Balls Have Eyes"        },
	{0x12, 0x01, 0x40, 0x00, "No"                     },
	{0x12, 0x01, 0x40, 0x40, "Yes"                    },

	{0   , 0xfe, 0   , 2   , "Test Mode"              },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Ohmygod)

static struct BurnDIPInfo NanameDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 15  , "Coin A"                 },
	{0x11, 0x01, 0x0f, 0x05, "6 Coin-3 Play"          },
	{0x11, 0x01, 0x0f, 0x03, "5 Coin-6 Play"          },
	{0x11, 0x01, 0x0f, 0x07, "4 Coin-1 Play"          },
	{0x11, 0x01, 0x0f, 0x04, "4 Coin-3 Play"          },
	{0x11, 0x01, 0x0f, 0x02, "4 Coin-5 Play"          },
	{0x11, 0x01, 0x0f, 0x08, "3 Coin-1 Play"          },
	{0x11, 0x01, 0x0f, 0x09, "2 Coin-1 Play"          },
	{0x11, 0x01, 0x0f, 0x01, "2 Coin-3 Play"          },
	{0x11, 0x01, 0x0f, 0x0f, "1 Coin-1 Play"          },
	{0x11, 0x01, 0x0f, 0x0e, "1 Coin-2 Play"          },
	{0x11, 0x01, 0x0f, 0x0d, "1 Coin-3 Play"          },
	{0x11, 0x01, 0x0f, 0x0c, "1 Coin-4 Play"          },
	{0x11, 0x01, 0x0f, 0x0b, "1 Coin-5 Play"          },
	{0x11, 0x01, 0x0f, 0x0a, "1 Coin-6 Play"          },
	{0x11, 0x01, 0x0f, 0x00, "Free Play"              },

	{0   , 0xfe, 0   , 15  , "Coin B"                 },
	{0x11, 0x01, 0xf0, 0x50, "6 Coin-3 Play"          },
	{0x11, 0x01, 0xf0, 0x30, "5 Coin-6 Play"          },
	{0x11, 0x01, 0xf0, 0x70, "4 Coin-1 Play"          },
	{0x11, 0x01, 0xf0, 0x40, "4 Coin-3 Play"          },
	{0x11, 0x01, 0xf0, 0x20, "4 Coin-5 Play"          },
	{0x11, 0x01, 0xf0, 0x80, "3 Coin-1 Play"          },
	{0x11, 0x01, 0xf0, 0x90, "2 Coin-1 Play"          },
	{0x11, 0x01, 0xf0, 0x10, "2 Coin-3 Play"          },
	{0x11, 0x01, 0xf0, 0xf0, "1 Coin-1 Play"          },
	{0x11, 0x01, 0xf0, 0xe0, "1 Coin-2 Play"          },
	{0x11, 0x01, 0xf0, 0xd0, "1 Coin-3 Play"          },
	{0x11, 0x01, 0xf0, 0xc0, "1 Coin-4 Play"          },
	{0x11, 0x01, 0xf0, 0xb0, "1 Coin-5 Play"          },
	{0x11, 0x01, 0xf0, 0xa0, "1 Coin-6 Play"          },
	{0x11, 0x01, 0xf0, 0x00, "Free Play"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "1P Difficulty"          },
	{0x12, 0x01, 0x03, 0x02, "Easy"                   },
	{0x12, 0x01, 0x03, 0x03, "Normal"                 },
	{0x12, 0x01, 0x03, 0x01, "Hard"                   },
	{0x12, 0x01, 0x03, 0x00, "Very Hard"              },

	{0   , 0xfe, 0   , 4   , "VS Difficulty"          },
	{0x12, 0x01, 0x0c, 0x0c, "Normal Jake"            },
	{0x12, 0x01, 0x0c, 0x08, "Hard Jake"              },
	{0x12, 0x01, 0x0c, 0x04, "Normal"                 },
	{0x12, 0x01, 0x0c, 0x00, "Hard"                   },

	{0   , 0xfe, 0   , 2   , "VS Matches/Credit"      },
	{0x12, 0x01, 0x10, 0x00, "1 Game Match"           },
	{0x12, 0x01, 0x10, 0x10, "3 Game Match"           },

	{0   , 0xfe, 0   , 2   , "Attract Sound"          },
	{0x12, 0x01, 0x20, 0x00, "Off"                    },
	{0x12, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Balls Have Eyes"        },
	{0x12, 0x01, 0x40, 0x00, "No"                     },
	{0x12, 0x01, 0x40, 0x40, "Yes"                    },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Naname)

static struct BurnRomInfo OhmygodRomDesc[] = {
	{ "omg-p.114",    0x080000, 0x48fa40ca, BRF_ESS | BRF_PRG }, //  0	68000 Program Code

	{ "omg-b.117",    0x080000, 0x73621fa6, BRF_GRA },			 //  1	Characters
	{ "omg-s.120",    0x080000, 0x6413bd36, BRF_GRA },			 //  2	Sprites

	{ "omg-g.107",    0x200000, 0x7405573c, BRF_SND },			 //  3	Samples
};


STD_ROM_PICK(Ohmygod)
STD_ROM_FN(Ohmygod)

static struct BurnRomInfo NanameRomDesc[] = {
	{ "036-prg.114",  0x080000, 0x3b7362f7, BRF_ESS | BRF_PRG }, //  0	68000 Program Code

	{ "036-bg.117",   0x080000, 0xf53e8da5, BRF_GRA },			 //  1	Characters
	{ "036-spr.120",  0x080000, 0xe36d8731, BRF_GRA },			 //  2	Sprites

	{ "036-snd.107",  0x200000, 0xa3e0caf4, BRF_SND },			 //  3	Samples
};


STD_ROM_PICK(Naname)
STD_ROM_FN(Naname)

INT32 OhmygodDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	watch_tick=3*60;

	// Set the initial MSM6295 Bank
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "ohmygod")) {
		AdpcmBankShift = 4;
	} else {
		AdpcmBankShift = 0;
	}

	OhmygodSpriteBank = 0;

	SndBank = 0;
	memcpy(MSM6295ROM + 0x20000, MSM6295ROM + 0x40000 + 0x20000 * SndBank, 0x20000);

	MSM6295Reset(0);

	return 0;
}


void do_watchtick()
{
	if (watch_tick > 0)
	if (--watch_tick == 0) {
		SekOpen(0);
		SekReset();
		SekClose();
		watch_tick=3*60;
	}
}

UINT16 __fastcall OhmygodReadWord(UINT32 a)
{
	switch (a) {
		case 0x800000: {
			return 0xffff - OhmygodInput[0];
		}

		case 0x800002: {
			return 0xffff - OhmygodInput[1];
		}

		case 0xb00000: {
			return 0xfff0 | MSM6295ReadStatus(0);
		}

		case 0xc00000: {
			watch_tick= 3 * 60;
			return 0xffff;
		}
	}

#ifdef OHMYGODLOG
	bprintf(PRINT_NORMAL, _T("68000 Read Word: %06X\n"), a);
#endif
	SEK_DEF_READ_WORD(0, a);
}

UINT8 __fastcall OhmygodReadByte(UINT32 a)
{
	switch (a) {
		case 0xa00000: {
			return OhmygodDip[0];
		}

		case 0xa00001: {
			return 0xff;
		}

		case 0xa00002: {
			return OhmygodDip[1];
		}

		case 0xa00003: {
			return 0xff;
		}
	}

#ifdef OHMYGODLOG
	bprintf(PRINT_NORMAL, _T("68000 Read Byte: %06X\n"), a);
#endif

	return 0;
}

void __fastcall OhmygodWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x400000: {
			OhmygodScrollx = d - 0x81ec;
			return;
		}

		case 0x400002: {
			OhmygodScrolly = d - 0x81ef;
			return;
		}

		case 0x900000: {
			SEK_DEF_WRITE_WORD(0, a, d);
			return;
		}

		case 0xb00000: {
			SEK_DEF_WRITE_WORD(0, a, d);
			return;
		}

		case 0xd00000: {
			OhmygodSpriteBank = d & 0x8000;
			return;
		}
	}

#ifdef OHMYGODLOG
	bprintf(PRINT_NORMAL, _T("68000 Write Word: %06X, %04X\n"), a, d);
#endif
}

void __fastcall OhmygodWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x710000 && a <= 0x713fff) {
		return;
	}

	switch (a) {
		case 0x900000: {
			return;
		}

		case 0x900001: {
			if (SndBank != ((d >> AdpcmBankShift) & 0x0f)) {
				SndBank = (d >> AdpcmBankShift) & 0x0f;
				memcpy(MSM6295ROM + 0x20000,MSM6295ROM + 0x40000 + 0x20000 * SndBank,0x20000);
			}
			return;
		}
		
		case 0xb00000: {
			return;
		}

		case 0xb00001: {
			MSM6295Command(0, d);
			return;
		}
	}

#ifdef OHMYGODLOG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte: %06X, %02X\n"), a, d);
#endif
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	OhmygodRom           = Next; Next += 0x080000;
	MSM6295ROM           = Next; Next += 0x240000;

	RamStart = Next;

	OhmygodRam           = Next; Next += 0x018000;
	OhmygodVideoRam      = Next; Next += 0x004000;
	OhmygodPaletteRam    = Next; Next += 0x000800;
	OhmygodSpriteRam     = Next; Next += 0x004000;

	RamEnd = Next;

	OhmygodChars            = Next; Next += (16384 * 8 * 8);
	OhmygodSprites          = Next; Next += (4096 * 16 * 16);
	OhmygodPalette          = (UINT32*)Next; Next += 0x000800 * sizeof(UINT32);
	MemEnd = Next;

	return 0;
}

static INT32 TilePlaneOffsets[4]   = { 0, 1, 2, 3 };
static INT32 TileXOffsets[8]       = { 0, 4, 8, 12, 16, 20, 24, 28 };
static INT32 TileYOffsets[8]       = { 0, 32, 64, 96, 128, 160, 192, 224 };
static INT32 SpritePlaneOffsets[4] = { 0, 1, 2, 3 };
static INT32 SpriteXOffsets[16]    = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60 };
static INT32 SpriteYOffsets[16]    = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };

INT32 OhmygodInit()
{
	INT32 nRet = 0, nLen;
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	// Load 68000 Program Rom
	nRet = BurnLoadRom(OhmygodRom, 0x00, 1); if (nRet != 0) return 1;

	// malloc stuff
	UINT8 *TempGfx=(UINT8*)BurnMalloc(0x80000);
	
	// Load and decode Character Rom
	nRet = BurnLoadRom(TempGfx, 1, 1); if (nRet != 0) return 1;
	GfxDecode(16384, 4, 8, 8, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, TempGfx, OhmygodChars);

	// Load and decode Sprite Rom
	memset(TempGfx, 0, 0x80000);
	nRet = BurnLoadRom(TempGfx, 2, 1); if (nRet != 0) return 1;
	GfxDecode(4096, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x400, TempGfx, OhmygodSprites);
	BurnFree(TempGfx);

	// Load Sample Rom
	nRet = BurnLoadRom(MSM6295ROM + 0x00000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROM + 0x40000, 3, 1); if (nRet != 0) return 1;

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(OhmygodRom           , 0x000000, 0x07ffff, SM_ROM  );
	SekMapMemory(OhmygodRam + 0x000000, 0x300000, 0x303fff, SM_RAM  );
	SekMapMemory(OhmygodVideoRam      , 0x304000, 0x307fff, SM_RAM  );
	SekMapMemory(OhmygodRam + 0x004000, 0x308000, 0x30ffff, SM_RAM  );
	SekMapMemory(OhmygodPaletteRam    , 0x600000, 0x607fff, SM_WRITE);
	SekMapMemory(OhmygodSpriteRam     , 0x700000, 0x703fff, SM_RAM  );
	SekMapMemory(OhmygodRam + 0x00c000, 0x704000, 0x707fff, SM_RAM  );
	SekMapMemory(OhmygodRam + 0x010000, 0x708000, 0x70ffff, SM_RAM  );
	SekSetReadWordHandler(0, OhmygodReadWord);
	SekSetWriteWordHandler(0, OhmygodWriteWord);
	SekSetReadByteHandler(0, OhmygodReadByte);
	SekSetWriteByteHandler(0, OhmygodWriteByte);
	SekClose();

	// Setup the OKIM6295 emulation
	MSM6295Init(0, 14000000/8/132, 100, 0);

	GenericTilesInit();

	// Reset the driver
	OhmygodDoReset();

	return 0;
}

INT32 OhmygodExit()
{
	MSM6295Exit(0);
	SekExit();
	GenericTilesExit();

	BurnFree(Mem);

	return 0;
}

void OhmygodRenderCharLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = (OhmygodVideoRam[2 * TileIndex + 3] << 8) | OhmygodVideoRam[2 * TileIndex + 2];
			Colour = (OhmygodVideoRam[2 * TileIndex + 1] << 8) | OhmygodVideoRam[2 * TileIndex + 0];
			Colour = (Colour & 0x0f00) >> 8;
			x = 8 * mx;
			y = 8 * my;

			x -= OhmygodScrollx + 97;
			y -= OhmygodScrolly;

			if (x > 7 && x < 312 && y > 7 && y < 232) {
				Render8x8Tile(pTransDraw, Code, x, y, Colour, 4, 0, OhmygodChars);
			} else {
				Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 0, OhmygodChars);
			}

			TileIndex +=2 ;
		}
	}
}

void OhmygodRenderSpriteLayer()
{
	INT32 MemOffset = OhmygodSpriteBank ? 0x2000 : 0x0000;

	for (INT32 Offs = MemOffset; Offs < MemOffset + 0x2000; Offs += 8) {
		INT32 sx, sy, Code, Colour, Flipx;

		Code = ((OhmygodSpriteRam[Offs + 7] << 8) + OhmygodSpriteRam[Offs + 6]) & 0x0fff;
		Colour = ((OhmygodSpriteRam[Offs + 5] << 8) + OhmygodSpriteRam[Offs + 4]) & 0x000f;
		sx = ((OhmygodSpriteRam[Offs + 1] << 8) + OhmygodSpriteRam[Offs + 0]) - 29;
		sy = (OhmygodSpriteRam[Offs + 3] << 8) + OhmygodSpriteRam[Offs + 2];
		if (sy >= 32768) sy -= 65536;

		sx -= 96;

		Flipx = ((OhmygodSpriteRam[Offs + 7] << 8) + OhmygodSpriteRam[Offs + 6]) & 0x8000;

		if (sx > 15 && sx < 304 && sy > 15 && sy < 224) {
			if (!Flipx) {
				Render16x16Tile_Mask(pTransDraw, Code, sx, sy, Colour, 4, 0, 512, OhmygodSprites);
			} else {
				Render16x16Tile_Mask_FlipX(pTransDraw, Code, sx, sy, Colour, 4, 0, 512, OhmygodSprites);
			}
		} else {
			if (!Flipx) {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 512, OhmygodSprites);
			} else {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, sx, sy, Colour, 4, 0, 512, OhmygodSprites);
			}
		}
	}
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour >>  5) & 0x1f;
	g = (nColour >> 10) & 0x1f;
	b = (nColour >>  0) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	return BurnHighCol(r, g, b, 0);
}

INT32 OhmygodCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)OhmygodPaletteRam, pd = OhmygodPalette; i < 0x800; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}

	return 0;
}

void OhmygodDraw()
{
	OhmygodCalcPalette();
	OhmygodRenderCharLayer();
	OhmygodRenderSpriteLayer();
	BurnTransferCopy(OhmygodPalette);
}

INT32 OhmygodFrame()
{
	if (OhmygodReset) OhmygodDoReset();

	OhmygodMakeInputs();

	SekNewFrame();

	SekOpen(0);
	SekRun(12000000 / 60);
	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
	SekClose();
	do_watchtick();
	if (pBurnDraw) OhmygodDraw();
	if (pBurnSoundOut) MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);

	return 0;
}

static INT32 OhmygodScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {					// Return minimum compatible version
		*pnMin = 0x02944;
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

		MSM6295Scan(0, nAction);			// Scan OKIM6295

		// Scan critical driver variables
		SCAN_VAR(OhmygodInput);
		SCAN_VAR(OhmygodDip);
		SCAN_VAR(AdpcmBankShift);
		SCAN_VAR(SndBank);
		SCAN_VAR(watch_tick);
		SCAN_VAR(OhmygodSpriteBank);
		SCAN_VAR(OhmygodScrollx);
		SCAN_VAR(OhmygodScrolly);
	}
	
	if (nAction & ACB_WRITE) {
		memcpy(MSM6295ROM + 0x20000,MSM6295ROM + 0x40000 + 0x20000 * SndBank,0x20000);
	}

	return 0;
}

struct BurnDriver BurnDrvOhmygod = {
	"ohmygod", NULL, NULL, NULL, "1993",
	"Oh My God!\0", NULL, "Atlus", "Miscellaneous",
	L"Oh my God! (Japan)\0Oh my god! \u30AA\u30FC\u30DE\u30A4\u30AC\u30A1\u30FC\uFF01\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, OhmygodRomInfo, OhmygodRomName, NULL, NULL, OhmygodInputInfo, OhmygodDIPInfo,
	OhmygodInit, OhmygodExit, OhmygodFrame, NULL, OhmygodScan,
	NULL, 0x800, 320, 240, 4, 3
};

struct BurnDriver BurnDrvNaname = {
	"naname", NULL, NULL, NULL, "1994",
	"Naname de Magic!\0", NULL, "Atlus", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, NanameRomInfo, NanameRomName, NULL, NULL, OhmygodInputInfo, NanameDIPInfo,
	OhmygodInit, OhmygodExit, OhmygodFrame, NULL, OhmygodScan,
	NULL, 0x800, 320, 240, 4, 3
};
