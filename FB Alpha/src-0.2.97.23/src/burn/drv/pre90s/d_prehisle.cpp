#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "burn_ym3812.h"
#include "upd7759.h"

// Input Related Variables
static UINT8 PrehisleInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 PrehisleInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 PrehisleInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 PrehisleDip[2]        = {0, 0};
static UINT8 PrehisleInput[3]      = {0x00, 0x00, 0x00};
static UINT8 PrehisleReset         = 0;

// Memory Holders
static UINT8 *Mem                  = NULL;
static UINT8 *MemEnd               = NULL;
static UINT8 *RamStart             = NULL;
static UINT8 *RamEnd               = NULL;
static UINT8 *PrehisleRom          = NULL;
static UINT8 *PrehisleZ80Rom       = NULL;
static UINT8 *PrehisleTileMapRom   = NULL;
static UINT8 *PrehisleADPCMSamples = NULL;
static UINT8 *PrehisleRam          = NULL;
static UINT8 *PrehisleVideoRam     = NULL;
static UINT8 *PrehisleSpriteRam    = NULL;
static UINT8 *PrehisleVideo2Ram    = NULL;
static UINT8 *PrehislePaletteRam   = NULL;
static UINT8 *PrehisleZ80Ram       = NULL;
static UINT32 *PrehislePalette     = NULL;
static UINT8 *PrehisleTextTiles    = NULL;
static UINT8 *PrehisleSprites      = NULL;
static UINT8 *PrehisleBack1Tiles   = NULL;
static UINT8 *PrehisleBack2Tiles   = NULL;
static UINT8 *PrehisleTempGfx      = NULL;

// Misc Variables, system control values, etc.
static INT32 ControlsInvert;
static UINT16 VidControl[7];
static INT32 SoundLatch;

// CPU Interleave Variables
static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

// Dip Switch and Input Definitions
static struct BurnInputInfo PrehisleInputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , PrehisleInputPort2 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , PrehisleInputPort0 + 7, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , PrehisleInputPort2 + 1, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , PrehisleInputPort1 + 7, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , PrehisleInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , PrehisleInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , PrehisleInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , PrehisleInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , PrehisleInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , PrehisleInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , PrehisleInputPort0 + 6, "p1 fire 3" },

	{"P2 Up"             , BIT_DIGITAL  , PrehisleInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , PrehisleInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , PrehisleInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , PrehisleInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , PrehisleInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , PrehisleInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , PrehisleInputPort1 + 6, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL  , &PrehisleReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , PrehisleInputPort2 + 2, "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , PrehisleInputPort2 + 3, "diag"      },
	{"Tilt"              , BIT_DIGITAL  , PrehisleInputPort2 + 4, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH, PrehisleDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, PrehisleDip + 1       , "dip"       },
};

STDINPUTINFO(Prehisle)

inline void PrehisleClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

inline void PrehisleMakeInputs()
{
	// Reset Inputs
	PrehisleInput[0] = PrehisleInput[1] = PrehisleInput[2] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		PrehisleInput[0] |= (PrehisleInputPort0[i] & 1) << i;
		PrehisleInput[1] |= (PrehisleInputPort1[i] & 1) << i;
		PrehisleInput[2] |= (PrehisleInputPort2[i] & 1) << i;
	}

	// Clear Opposites
	PrehisleClearOpposites(&PrehisleInput[0]);
	PrehisleClearOpposites(&PrehisleInput[1]);
}

static struct BurnDIPInfo PrehisleDIPList[]=
{
	// Default Values
	{0x16, 0xff, 0xff, 0xff, NULL                     },
	{0x17, 0xff, 0xff, 0x7f, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Display"                },
	{0x16, 0x01, 0x01, 0x01, "Normal"                 },
	{0x16, 0x01, 0x01, 0x00, "Inverse"                },

	{0   , 0xfe, 0   , 2   , "Level Select"           },
	{0x16, 0x01, 0x02, 0x02, "Off"                    },
	{0x16, 0x01, 0x02, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Bonus"                  },
	{0x16, 0x01, 0x04, 0x04, "2nd"                    },
	{0x16, 0x01, 0x04, 0x00, "Every"                  },

//	{0   , 0xfe, 0   , 2   , "Unknown"                },
//	{0x16, 0x01, 0x08, 0x08, "Off"                    },
//	{0x16, 0x01, 0x08, 0x00, "On"                     },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x16, 0x01, 0x30, 0x30, "A 1-1 B 1-1"            },
	{0x16, 0x01, 0x30, 0x20, "A 2-1 B 1-2"            },
	{0x16, 0x01, 0x30, 0x10, "A 3-1 B 1-3"            },
	{0x16, 0x01, 0x30, 0x00, "A 4-1 B 1-4"            },

	{0   , 0xfe, 0   , 4   , "Hero"                   },
	{0x16, 0x01, 0xc0, 0x80, "2"                      },
	{0x16, 0x01, 0xc0, 0xc0, "3"                      },
	{0x16, 0x01, 0xc0, 0x40, "4"                      },
	{0x16, 0x01, 0xc0, 0x00, "5"                      },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Level"                  },
	{0x17, 0x01, 0x03, 0x02, "1 (Easy)"               },
	{0x17, 0x01, 0x03, 0x03, "2 (Standard)"           },
	{0x17, 0x01, 0x03, 0x01, "3 (Middle)"             },
	{0x17, 0x01, 0x03, 0x00, "4 (Difficult)"          },

	{0   , 0xfe, 0   , 4   , "Game Mode"              },
	{0x17, 0x01, 0x0c, 0x08, "Demo Sound Off"         },
	{0x17, 0x01, 0x0c, 0x0c, "Demo Sound On"          },
	{0x17, 0x01, 0x0c, 0x00, "Stop Video"             },
	{0x17, 0x01, 0x0c, 0x04, "Never Finish"           },

	{0   , 0xfe, 0   , 4   , "Bonus 1st/2nd"          },
	{0x17, 0x01, 0x30, 0x30, "100000/200000"          },
	{0x17, 0x01, 0x30, 0x20, "150000/300000"          },
	{0x17, 0x01, 0x30, 0x10, "300000/500000"          },
	{0x17, 0x01, 0x30, 0x00, "No Bonus"               },

	{0   , 0xfe, 0   , 2   , "Continue"               },
	{0x17, 0x01, 0x40, 0x00, "Off"                    },
	{0x17, 0x01, 0x40, 0x40, "On"                     },
};

STDDIPINFO(Prehisle)

// Rom Definitions
static struct BurnRomInfo PrehisleRomDesc[] = {
	{ "gt-e2.2h",      0x20000, 0x7083245a, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "gt-e3.3h",      0x20000, 0x6d8cdf58, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "gt15.b15",      0x08000, 0xac652412, BRF_GRA },			 //  2	Text Layer Tiles
	{ "pi8914.b14",    0x40000, 0x207d6187, BRF_GRA },			 //  3	Background2 Layer Tiles
	{ "pi8916.h16",    0x40000, 0x7cffe0f6, BRF_GRA },			 //  4	Background1 Layer Tiles
	{ "pi8910.k14",    0x80000, 0x5a101b0b, BRF_GRA },			 //  5	Sprite Layer Tiles
	{ "gt5.5",         0x20000, 0x3d3ab273, BRF_GRA },			 //  6	Sprite Layer Tiles
	{ "gt11.11",       0x10000, 0xb4f0fcf0, BRF_GRA },			 //  7	Background 2 TileMap

	{ "gt1.1",         0x10000, 0x80a4c093, BRF_SND },			 //  8	Z80 Program Code

	{ "gt4.4",         0x20000, 0x85dfb9ec, BRF_SND },			 //  9	ADPCM Samples
};


STD_ROM_PICK(Prehisle)
STD_ROM_FN(Prehisle)

static struct BurnRomInfo PrehisluRomDesc[] = {
	{ "gt-u2.2h",      0x20000, 0xa14f49bb, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "gt-u3.3h",      0x20000, 0xf165757e, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "gt15.b15",      0x08000, 0xac652412, BRF_GRA },			 //  2	Text Layer Tiles
	{ "pi8914.b14",    0x40000, 0x207d6187, BRF_GRA },			 //  3	Background2 Layer Tiles
	{ "pi8916.h16",    0x40000, 0x7cffe0f6, BRF_GRA },			 //  4	Background1 Layer Tiles
	{ "pi8910.k14",    0x80000, 0x5a101b0b, BRF_GRA },			 //  5	Sprite Layer Tiles
	{ "gt5.5",         0x20000, 0x3d3ab273, BRF_GRA },			 //  6	Sprite Layer Tiles
	{ "gt11.11",       0x10000, 0xb4f0fcf0, BRF_GRA },			 //  7	Background 2 TileMap

	{ "gt1.1",         0x10000, 0x80a4c093, BRF_SND },			 //  8	Z80 Program Code

	{ "gt4.4",         0x20000, 0x85dfb9ec, BRF_SND },			 //  9	ADPCM Samples
};


STD_ROM_PICK(Prehislu)
STD_ROM_FN(Prehislu)

static struct BurnRomInfo PrehislkRomDesc[] = {
	{ "gt-k2.2h",      0x20000, 0xf2d3544d, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "gt-k3.3h",      0x20000, 0xebf7439b, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "gt15.b15",      0x08000, 0xac652412, BRF_GRA },			 //  2	Text Layer Tiles
	{ "pi8914.b14",    0x40000, 0x207d6187, BRF_GRA },			 //  3	Background2 Layer Tiles
	{ "pi8916.h16",    0x40000, 0x7cffe0f6, BRF_GRA },			 //  4	Background1 Layer Tiles
	{ "pi8910.k14",    0x80000, 0x5a101b0b, BRF_GRA },			 //  5	Sprite Layer Tiles
	{ "gt5.5",         0x20000, 0x3d3ab273, BRF_GRA },			 //  6	Sprite Layer Tiles
	{ "gt11.11",       0x10000, 0xb4f0fcf0, BRF_GRA },			 //  7	Background 2 TileMap

	{ "gt1.1",         0x10000, 0x80a4c093, BRF_SND },			 //  8	Z80 Program Code

	{ "gt4.4",         0x20000, 0x85dfb9ec, BRF_SND },			 //  9	ADPCM Samples
};


STD_ROM_PICK(Prehislk)
STD_ROM_FN(Prehislk)

static struct BurnRomInfo GensitouRomDesc[] = {
	{ "gt-j2.2h",      0x20000, 0xa2da0b6b, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "gt-j3.3h",      0x20000, 0xc1a0ae8e, BRF_ESS | BRF_PRG }, //  1	68000 Program Code

	{ "gt15.b15",      0x08000, 0xac652412, BRF_GRA },			 //  2	Text Layer Tiles
	{ "pi8914.b14",    0x40000, 0x207d6187, BRF_GRA },			 //  3	Background2 Layer Tiles
	{ "pi8916.h16",    0x40000, 0x7cffe0f6, BRF_GRA },			 //  4	Background1 Layer Tiles
	{ "pi8910.k14",    0x80000, 0x5a101b0b, BRF_GRA },			 //  5	Sprite Layer Tiles
	{ "gt5.5",         0x20000, 0x3d3ab273, BRF_GRA },			 //  6	Sprite Layer Tiles
	{ "gt11.11",       0x10000, 0xb4f0fcf0, BRF_GRA },			 //  7	Background 2 TileMap

	{ "gt1.1",         0x10000, 0x80a4c093, BRF_SND },			 //  8	Z80 Program Code

	{ "gt4.4",         0x20000, 0x85dfb9ec, BRF_SND },			 //  9	ADPCM Samples
};


STD_ROM_PICK(Gensitou)
STD_ROM_FN(Gensitou)

// Misc Driver Functions and Memory Handlers
INT32 PrehisleDoReset()
{
	ControlsInvert = 0;
	SoundLatch = 0;
	VidControl[0] = VidControl[1] = VidControl[2] = VidControl[3] = VidControl[4] = VidControl[5] = VidControl[6] = 0;

	SekOpen(0);
	SekReset();
	SekClose();
	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM3812Reset();
	UPD7759Reset();

	return 0;
}

// ----------------------------------------------------------------------------
// Callbacks for the FM chip

static void prehisleFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 prehisleSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

// VBlank

inline UINT16 PrehisleVBlankRegister()
{
	INT32 nCycles = SekTotalCycles();

	// 262 == approximate number of scanlines on an arcade monitor
	if (nCycles >= (262 - 16) * ((9000000 / 60) / 262)) {
		return 0x80;
	} else {
		if (nCycles < (262 - 210 - 16) * ((9000000 / 60) / 262)) {
			return 0x80;
		}
	}

	return 0x00;
}

UINT16 __fastcall PrehisleReadWord(UINT32 a)
{
	switch (a) {
		case 0x0e0010: {
			return 0xff - PrehisleInput[1];
		}

		case 0x0e0020: {
			return 0xff - PrehisleInput[2];
		}

		case 0x0e0040: {
			return 0xff - (PrehisleInput[0] ^ ControlsInvert);
		}

		case 0x0e0042: {
			return PrehisleDip[0];
		}

		case 0x0e0044: {
			return PrehisleDip[1] + PrehisleVBlankRegister();
		}
	}

	return 0;
}

void __fastcall PrehisleWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x0f0000: {
			VidControl[0] = d;
			return;
		}

		case 0x0f0010: {
			VidControl[1] = d;
			return;
		}

		case 0x0f0020: {
			VidControl[2] = d;
			return;
		}

		case 0x0f0030: {
			VidControl[3] = d;
			return;
		}

		case 0x0f0046: {
			ControlsInvert = d ? 0xff : 0x00;
			return;
		}

		case 0x0f0050: {
			VidControl[4] = d;
			return;
		}

		case 0x0f0052: {
			VidControl[5] = d;
			return;
		}

		case 0x0f0060: {
			VidControl[6] = d;
			return;
		}

		case 0x0f0070: {
			SoundLatch = d & 0xff;
			ZetNmi();
			return;
		}
	}
}

UINT8 __fastcall PrehisleZ80PortRead(UINT16 a)
{
	a &= 0xff;
	switch (a) {
		case 0x00: {
			return BurnYM3812Read(0);
		}
	}

	return 0;
}

void __fastcall PrehisleZ80PortWrite(UINT16 a, UINT8 d)
{
	a &= 0xff;
	switch (a) {
		case 0x00: {
			BurnYM3812Write(0, d);
			return;
		}

		case 0x20: {
			BurnYM3812Write(1, d);
			return;
		}

		case 0x40: {
			UPD7759PortWrite(0,d);
			UPD7759StartWrite(0,0);
			UPD7759StartWrite(0,1);
			return;
		}

		case 0x80: {
			UPD7759ResetWrite(0,d);
			return;
		}
	}
}

UINT8 __fastcall PrehisleZ80Read(UINT16 a)
{
	switch (a) {
		case 0xf800: {
			return SoundLatch;
		}
	}

	return 0;
}

// Function to Allocate and Index required memory
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	PrehisleRom          = Next; Next += 0x40000;
	PrehisleZ80Rom       = Next; Next += 0x10000;
	PrehisleTileMapRom   = Next; Next += 0x10000;
	PrehisleADPCMSamples = Next; Next += 0x20000;

	RamStart = Next;

	PrehisleRam          = Next; Next += 0x04000;
	PrehisleVideoRam     = Next; Next += 0x00800;
	PrehisleSpriteRam    = Next; Next += 0x00800;
	PrehisleVideo2Ram    = Next; Next += 0x04000;
	PrehislePaletteRam   = Next; Next += 0x00800;
	PrehisleZ80Ram       = Next; Next += 0x00800;

	RamEnd = Next;

	PrehisleTextTiles    = Next; Next += (1024 * 8 * 8);
	PrehisleSprites      = Next; Next += (5120 * 16 * 16);
	PrehisleBack1Tiles   = Next; Next += (2048 * 16 * 16);
	PrehisleBack2Tiles   = Next; Next += (2048 * 16 * 16);
	PrehislePalette = (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);
	MemEnd = Next;

	return 0;
}

static INT32 CharPlaneOffsets[4]   = { 0, 1, 2, 3 };
static INT32 CharXOffsets[8]       = { 0, 4, 8, 12, 16, 20, 24, 28 };
static INT32 CharYOffsets[8]       = { 0, 32, 64, 96, 128, 160, 192, 224 };
static INT32 TilePlaneOffsets[4]   = { 0, 1, 2, 3 };
static INT32 TileXOffsets[16]      = { 0, 4, 8, 12, 16, 20, 24, 28, 512, 516, 520, 524, 528, 532, 536, 540 };
static INT32 TileYOffsets[16]      = { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480 };

// Driver Init and Exit Functions
INT32 PrehisleInit()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	PrehisleTempGfx = (UINT8*)BurnMalloc(0xa0000);

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(PrehisleRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(PrehisleRom + 0x00000, 1, 2); if (nRet != 0) return 1;

	// Load and decode Text Tiles rom
	memset(PrehisleTempGfx, 0, 0xa0000);
	nRet = BurnLoadRom(PrehisleTempGfx, 2, 1); if (nRet != 0) return 1;
	GfxDecode(1024, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, PrehisleTempGfx, PrehisleTextTiles);

	// Load and decode Background2 Tile rom
	memset(PrehisleTempGfx, 0, 0xa0000);
	nRet = BurnLoadRom(PrehisleTempGfx, 3, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, PrehisleTempGfx, PrehisleBack2Tiles);

	// Load and decode Background1 Tile rom
	memset(PrehisleTempGfx, 0, 0xa0000);
	nRet = BurnLoadRom(PrehisleTempGfx, 4, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, PrehisleTempGfx, PrehisleBack1Tiles);

	// Load and decode Sprite roms
	memset(PrehisleTempGfx, 0, 0xa0000);
	nRet = BurnLoadRom(PrehisleTempGfx + 0x00000, 5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(PrehisleTempGfx + 0x80000, 6, 1); if (nRet != 0) return 1;
	GfxDecode(5120, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, PrehisleTempGfx, PrehisleSprites);

	BurnFree(PrehisleTempGfx);

	// Load Background2 Tilemap rom
	nRet = BurnLoadRom(PrehisleTileMapRom, 7, 1); if (nRet != 0) return 1;

	// Load Z80 Program rom
	nRet = BurnLoadRom(PrehisleZ80Rom, 8, 1); if (nRet != 0) return 1;

	// Load ADPCM Samples
	nRet = BurnLoadRom(PrehisleADPCMSamples, 9, 1); if (nRet != 0) return 1;

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(PrehisleRom       , 0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(PrehisleRam       , 0x070000, 0x073fff, SM_RAM);
	SekMapMemory(PrehisleVideoRam  , 0x090000, 0x0907ff, SM_RAM);
	SekMapMemory(PrehisleSpriteRam , 0x0a0000, 0x0a07ff, SM_RAM);
	SekMapMemory(PrehisleVideo2Ram , 0x0b0000, 0x0b3fff, SM_RAM);
	SekMapMemory(PrehislePaletteRam, 0x0d0000, 0x0d07ff, SM_RAM);
	SekSetReadWordHandler(0, PrehisleReadWord);
	SekSetWriteWordHandler(0, PrehisleWriteWord);
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xefff, 0, PrehisleZ80Rom);
	ZetMapArea(0x0000, 0xefff, 2, PrehisleZ80Rom);
	ZetMapArea(0xf000, 0xf7ff, 0, PrehisleZ80Ram);
	ZetMapArea(0xf000, 0xf7ff, 1, PrehisleZ80Ram);
	ZetMapArea(0xf000, 0xf7ff, 2, PrehisleZ80Ram);
	ZetMemEnd();
	ZetSetReadHandler(PrehisleZ80Read);
	ZetSetInHandler(PrehisleZ80PortRead);
	ZetSetOutHandler(PrehisleZ80PortWrite);
	ZetClose();

	BurnYM3812Init(4000000, &prehisleFMIRQHandler, &prehisleSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(4000000);
	BurnYM3812SetRoute(BURN_SND_YM3812_ROUTE, 1.00, BURN_SND_ROUTE_BOTH);
	
	UPD7759Init(0, UPD7759_STANDARD_CLOCK, PrehisleADPCMSamples);
	UPD7759SetRoute(0, 0.90, BURN_SND_ROUTE_BOTH);
	
	GenericTilesInit();

	// Reset the driver
	PrehisleDoReset();

	return 0;
}

INT32 PrehisleExit()
{
	BurnYM3812Exit();
	UPD7759Exit();

	SekExit();
	ZetExit();

	GenericTilesExit();

	BurnFree(Mem);

	return 0;
}

// Graphics Emulation
void PrehisleRenderBack2TileLayer()
{
	INT32 TileBase, mx, my, Tile, Colour, Scrollx, Scrolly, x, y, Flipx;

	TileBase = ((VidControl[3] >> 4) & 0x3ff) * 32;
	TileBase &= 0x7fff;
	Scrollx = -(VidControl[3] & 0x0f);
	Scrolly = -VidControl[2];

	for (mx = 0; mx < 17; mx++) {
		for (my = 0; my < 32; my++) {
			Tile = (PrehisleTileMapRom[2 * TileBase + 0] << 8) + PrehisleTileMapRom[2 * TileBase + 1];
			Colour = Tile >> 12;
			Flipx = Tile & 0x800;
			x = 16 * mx + Scrollx;
			y = (16 * my + Scrolly) & 0x1ff;
			y -= 16;

			if (x > 15 && x < 240 && y > 15 && y < 208) {
				if (!Flipx) {
					Render16x16Tile(pTransDraw, Tile & 0x7ff, x, y, Colour, 4, 768, PrehisleBack2Tiles);
				} else {
					Render16x16Tile_FlipX(pTransDraw, Tile & 0x7ff, x, y, Colour, 4, 768, PrehisleBack2Tiles);
				}
			} else {
				if (!Flipx) {
					Render16x16Tile_Clip(pTransDraw, Tile & 0x7ff, x, y, Colour, 4, 768, PrehisleBack2Tiles);
				} else {
					Render16x16Tile_FlipX_Clip(pTransDraw, Tile & 0x7ff, x, y, Colour, 4, 768, PrehisleBack2Tiles);
				}
			}

			TileBase ++;
			if (TileBase == 0x8000) TileBase = 0;
		}
	}
}

void PrehisleRenderBack1TileLayer()
{
	INT32 TileBase, mx, my, Tile, Colour, Scrollx, Scrolly, x, y, Flipy;

	TileBase = ((VidControl[1] >> 4) & 0xff) * 32;
	TileBase &= 0x1fff;
	Scrollx = -(VidControl[1] & 0x0f);
	Scrolly = -VidControl[0];

	for (mx = 0; mx < 17; mx++) {
		for (my = 0; my < 32; my++) {
			Tile = (PrehisleVideo2Ram[2 * TileBase + 1] << 8) + PrehisleVideo2Ram[2 * TileBase + 0];
			Colour = Tile >> 12;
			Flipy = Tile & 0x800;
			x = 16 * mx + Scrollx;
			y = (16 * my + Scrolly) & 0x1ff;
			y -= 16;

			if (x > 15 && x < 240 && y > 15 && y < 208) {
				if (!Flipy) {
					Render16x16Tile_Mask(pTransDraw, Tile & 0x7ff, x, y, Colour, 4, 0x0f, 512, PrehisleBack1Tiles);
				} else {
					Render16x16Tile_Mask_FlipY(pTransDraw, Tile & 0x7ff, x, y, Colour, 4, 0x0f, 512, PrehisleBack1Tiles);
				}
			} else {
				if (!Flipy) {
					Render16x16Tile_Mask_Clip(pTransDraw, Tile & 0x7ff, x, y, Colour, 4, 0x0f, 512, PrehisleBack1Tiles);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Tile & 0x7ff, x, y, Colour, 4, 0x0f, 512, PrehisleBack1Tiles);
				}
			}

			TileBase ++;
			if (TileBase == 0x2000) TileBase = 0;
		}
	}
}

void PrehisleRenderSpriteLayer()
{
	INT32 offs;

	for (offs = 0; offs < 0x800; offs += 8) {
		INT32 x, y, Sprite, Colour, Flipx, Flipy;

		y = (PrehisleSpriteRam[offs + 1] << 8) + PrehisleSpriteRam[offs + 0];
		if (y > 254) continue;
		y -= 16;
		x = (PrehisleSpriteRam[offs + 3] << 8) +  PrehisleSpriteRam[offs + 2];
		if (x & 0x200) x = -(0xff - (x & 0xff));
		if (x > 256) continue;

		Sprite = (PrehisleSpriteRam[offs + 5] << 8) + PrehisleSpriteRam[offs + 4];
		Colour = ((PrehisleSpriteRam[offs + 7] << 8) + PrehisleSpriteRam[offs + 6]) >> 12;
		Flipy = Sprite & 0x8000;
		Flipx = Sprite & 0x4000;
		Sprite &= 0x1fff;
		if (Sprite > 0x13ff) Sprite = 0x13ff;

		if (x > 15 && x < 240 && y > 15 && y < 208) {
			if (!Flipy) {
				if (!Flipx) {
					Render16x16Tile_Mask(pTransDraw, Sprite, x, y, Colour, 4, 0x0f, 256, PrehisleSprites);
				} else {
					Render16x16Tile_Mask_FlipX(pTransDraw, Sprite, x, y, Colour, 4, 0x0f, 256, PrehisleSprites);
				}
			} else {
				if (!Flipx) {
					Render16x16Tile_Mask_FlipY(pTransDraw, Sprite, x, y, Colour, 4, 0x0f, 256, PrehisleSprites);
				} else {
					Render16x16Tile_Mask_FlipXY(pTransDraw, Sprite, x, y, Colour, 4, 0x0f, 256, PrehisleSprites);
				}
			}
		} else {
			if (!Flipy) {
				if (!Flipx) {
					Render16x16Tile_Mask_Clip(pTransDraw, Sprite, x, y, Colour, 4, 0x0f, 256, PrehisleSprites);
				} else {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Sprite, x, y, Colour, 4, 0x0f, 256, PrehisleSprites);
				}
			} else {
				if (!Flipx) {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Sprite, x, y, Colour, 4, 0x0f, 256, PrehisleSprites);
				} else {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Sprite, x, y, Colour, 4, 0x0f, 256, PrehisleSprites);
				}
			}
		}
	}
}

void PrehisleRenderTextLayer()
{
	INT32 offs, mx, my, Colour, Tile, x, y;

	mx = -1;
	my = 0;
	for (offs = 0x000; offs < 0x800; offs+=2) {
		mx++;
		if (mx == 32) {
			mx = 0;
			my++;
		}
		Tile = (PrehisleVideoRam[offs + 1] << 8) + PrehisleVideoRam[offs + 0];
		Colour = Tile >> 12;
		x = 8 * mx;
		y = 8 * my;
		y -= 16;

		if (x > 7 && x < 248 && y > 7 && y < 216) {
			Render8x8Tile_Mask(pTransDraw, Tile & 0xfff, x, y, Colour, 4, 0x0f, 0, PrehisleTextTiles);
		} else {
			Render8x8Tile_Mask_Clip(pTransDraw, Tile & 0xfff, x, y, Colour, 4, 0x0f, 0, PrehisleTextTiles);
		}
	}
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour >> 12) & 0x0f;
	g = (nColour >> 8) & 0x0f;
	b = (nColour >> 4) & 0x0f;

	r = (r << 4) | r;
	g = (g << 4) | g;
	b = (b << 4) | b;

	return BurnHighCol(r, g, b, 0);
}

INT32 PrehisleCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)PrehislePaletteRam, pd = PrehislePalette; i < 0x800; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}

	return 0;
}

void PrehisleDraw()
{
	PrehisleCalcPalette();
	PrehisleRenderBack2TileLayer();
	PrehisleRenderBack1TileLayer();
	PrehisleRenderSpriteLayer();
	PrehisleRenderTextLayer();
	BurnTransferCopy(PrehislePalette);
}

// Frame Function
INT32 PrehisleFrame()
{
	INT32 nInterleave = 1;
	
	if (PrehisleReset) PrehisleDoReset();

	PrehisleMakeInputs();

	nCyclesTotal[0] = 9000000 / 60;
	nCyclesTotal[1] = 4000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	SekOpen(0);
	ZetOpen(0);

	SekNewFrame();
	ZetNewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == (nInterleave - 1)) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	}
	
	BurnTimerEndFrameYM3812(nCyclesTotal[1]);
	if (pBurnSoundOut) {
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		UPD7759Update(0, pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
	SekClose();

	if (pBurnDraw) PrehisleDraw();

	return 0;
}

// Scan RAM
static INT32 PrehisleScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029521;
	}

	if (nAction & ACB_MEMORY_RAM) {								// Scan all memory, devices & variables
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);			// Scan 68000
		ZetScan(nAction);			// Scan Z80

		BurnYM3812Scan(nAction, pnMin);
		UPD7759Scan(0, nAction, pnMin);

		// Scan critical driver variables
		SCAN_VAR(PrehisleInput);
		SCAN_VAR(PrehisleDip);
		SCAN_VAR(ControlsInvert);
		SCAN_VAR(VidControl);
		SCAN_VAR(nCyclesDone);
	}

	return 0;
}

// Driver Declarations
struct BurnDriver BurnDrvPrehisle = {
	"prehisle", NULL, NULL, NULL, "1989",
	"Prehistoric Isle in 1930 (World)\0", NULL, "SNK", "Prehistoric Isle (SNK)",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, PrehisleRomInfo, PrehisleRomName, NULL, NULL, PrehisleInputInfo, PrehisleDIPInfo,
	PrehisleInit, PrehisleExit, PrehisleFrame, NULL, PrehisleScan,
	NULL, 0x800, 256, 224, 4, 3
};

struct BurnDriver BurnDrvPrehislu = {
	"prehisleu", "prehisle", NULL, NULL, "1989",
	"Prehistoric Isle in 1930 (US)\0", NULL, "SNK of America", "Prehistoric Isle (SNK)",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, PrehisluRomInfo, PrehisluRomName, NULL, NULL, PrehisleInputInfo, PrehisleDIPInfo,
	PrehisleInit, PrehisleExit, PrehisleFrame, NULL, PrehisleScan,
	NULL, 0x800, 256, 224, 4, 3
};

struct BurnDriver BurnDrvPrehislk = {
	"prehislek", "prehisle", NULL, NULL, "1989",
	"Prehistoric Isle in 1930 (Korea)\0", NULL, "SNK (Victor license)", "Prehistoric Isle (SNK)",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, PrehislkRomInfo, PrehislkRomName, NULL, NULL, PrehisleInputInfo, PrehisleDIPInfo,
	PrehisleInit, PrehisleExit, PrehisleFrame, NULL, PrehisleScan,
	NULL, 0x800, 256, 224, 4, 3
};

struct BurnDriver BurnDrvGensitou = {
	"gensitou", "prehisle", NULL, NULL, "1989",
	"Genshi-Tou 1930's (Japan)\0", NULL, "SNK", "Prehistoric Isle (SNK)",
	L"Genshi-Tou 1930's (Japan)\0\u539F\u59CB\u5CF6 1930's\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, GensitouRomInfo, GensitouRomName, NULL, NULL, PrehisleInputInfo, PrehisleDIPInfo,
	PrehisleInit, PrehisleExit, PrehisleFrame, NULL, PrehisleScan,
	NULL, 0x800, 256, 224, 4, 3
};
