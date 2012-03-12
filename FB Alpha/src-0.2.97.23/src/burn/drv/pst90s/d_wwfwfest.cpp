#include "tiles_generic.h"
#include "burn_ym2151.h"
#include "msm6295.h"

static UINT8 DrvInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort3[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort4[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[2]        = {0, 0};
static UINT8 DrvInput[5]      = {0, 0, 0, 0, 0};
static UINT8 DrvReset         = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *Drv68KRom           = NULL;
static UINT8 *Drv68KRam           = NULL;
static UINT8 *DrvZ80Rom           = NULL;
static UINT8 *DrvZ80Ram           = NULL;
static UINT8 *DrvMSM6295ROMSrc    = NULL;
static UINT8 *DrvCharVideoRam     = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvSpriteRamBuff    = NULL;
static UINT8 *DrvBg0VideoRam      = NULL;
static UINT8 *DrvBg1VideoRam      = NULL;
static UINT8 *DrvPaletteRam       = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvTiles            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette         = NULL;

static UINT8 DrvVBlank;
static UINT8 DrvOkiBank;
static UINT8 DrvSoundLatch;
static UINT16 DrvBg0ScrollX;
static UINT16 DrvBg0ScrollY;
static UINT16 DrvBg1ScrollX;
static UINT16 DrvBg1ScrollY;
static UINT16 DrvVReg;

static INT32 DrvSpriteXOffset;
static INT32 DrvBg0XOffset;
static INT32 DrvBg1XOffset[2];

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort4 + 0, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 start"  },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 start"  },
	{"Start 3"           , BIT_DIGITAL  , DrvInputPort2 + 7, "p3 start"  },
	{"Start 4"           , BIT_DIGITAL  , DrvInputPort3 + 7, "p4 start"  },
	
	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },
	
	{"P3 Up"             , BIT_DIGITAL  , DrvInputPort2 + 2, "p3 up"     },
	{"P3 Down"           , BIT_DIGITAL  , DrvInputPort2 + 3, "p3 down"   },
	{"P3 Left"           , BIT_DIGITAL  , DrvInputPort2 + 1, "p3 left"   },
	{"P3 Right"          , BIT_DIGITAL  , DrvInputPort2 + 0, "p3 right"  },
	{"P3 Fire 1"         , BIT_DIGITAL  , DrvInputPort2 + 4, "p3 fire 1" },
	{"P3 Fire 2"         , BIT_DIGITAL  , DrvInputPort2 + 5, "p3 fire 2" },
	
	{"P4 Up"             , BIT_DIGITAL  , DrvInputPort3 + 2, "p4 up"     },
	{"P4 Down"           , BIT_DIGITAL  , DrvInputPort3 + 3, "p4 down"   },
	{"P4 Left"           , BIT_DIGITAL  , DrvInputPort3 + 1, "p4 left"   },
	{"P4 Right"          , BIT_DIGITAL  , DrvInputPort3 + 0, "p4 right"  },
	{"P4 Fire 1"         , BIT_DIGITAL  , DrvInputPort3 + 4, "p4 fire 1" },
	{"P4 Fire 2"         , BIT_DIGITAL  , DrvInputPort3 + 5, "p4 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort4 + 1, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Drv)

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
	DrvInput[0] = DrvInput[1] = DrvInput[2] = DrvInput[3] = DrvInput[4] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] |= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] |= (DrvInputPort2[i] & 1) << i;
		DrvInput[3] |= (DrvInputPort3[i] & 1) << i;
		DrvInput[4] |= (DrvInputPort4[i] & 1) << i;
	}

	// Clear Opposites
	DrvClearOpposites(&DrvInput[0]);
	DrvClearOpposites(&DrvInput[1]);
	DrvClearOpposites(&DrvInput[2]);
	DrvClearOpposites(&DrvInput[3]);
}

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x1f, 0xff, 0xff, 0xef, NULL                     },
	{0x20, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x1f, 0x01, 0x03, 0x00, "3 Coins 1 Credit"       },
	{0x1f, 0x01, 0x03, 0x01, "2 Coins 1 Credit"       },
	{0x1f, 0x01, 0x03, 0x03, "1 Coin  1 Credit"       },
	{0x1f, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Buy In Price"           },
	{0x1f, 0x01, 0x04, 0x04, "1 Coin"                 },
	{0x1f, 0x01, 0x04, 0x00, "As Start Price"         },
	
	{0   , 0xfe, 0   , 2   , "Regain Power Price"     },
	{0x1f, 0x01, 0x08, 0x08, "1 Coin"                 },
	{0x1f, 0x01, 0x08, 0x00, "As Start Price"         },
	
	{0   , 0xfe, 0   , 2   , "Continue Price"         },
	{0x1f, 0x01, 0x10, 0x10, "1 Coin"                 },
	{0x1f, 0x01, 0x10, 0x00, "As Start Price"         },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x1f, 0x01, 0x20, 0x00, "Off"                    },
	{0x1f, 0x01, 0x20, 0x20, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x1f, 0x01, 0x40, 0x40, "Off"                    },
	{0x1f, 0x01, 0x40, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "FBI Logo"               },
	{0x1f, 0x01, 0x80, 0x00, "Off"                    },
	{0x1f, 0x01, 0x80, 0x80, "On"                     },
	
	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x20, 0x01, 0x03, 0x02, "Easy"                   },
	{0x20, 0x01, 0x03, 0x03, "Normal"                 },
	{0x20, 0x01, 0x03, 0x01, "Hard"                   },
	{0x20, 0x01, 0x03, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 3   , "Number of Players"      },
	{0x20, 0x01, 0x0c, 0x04, "2"                      },
	{0x20, 0x01, 0x0c, 0x08, "3"                      },
	{0x20, 0x01, 0x0c, 0x0c, "4"                      },
	
	{0   , 0xfe, 0   , 4   , "Clear Stage Power Up"   },
	{0x20, 0x01, 0x60, 0x00, "0"                      },
	{0x20, 0x01, 0x60, 0x20, "12"                     },
	{0x20, 0x01, 0x60, 0x60, "24"                     },
	{0x20, 0x01, 0x60, 0x40, "32"                     },
	
	{0   , 0xfe, 0   , 2   , "Championship Game"      },
	{0x20, 0x01, 0x80, 0x00, "4th"                    },
	{0x20, 0x01, 0x80, 0x80, "5th"                    },
};

STDDIPINFO(Drv)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "31a13-2.ic19",  0x040000, 0x7175bca7, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "31a14-2.ic18",  0x040000, 0x5d06bfd1, BRF_ESS | BRF_PRG }, //  1
	
	{ "31a11-2.ic42",  0x010000, 0x5ddebfea, BRF_ESS | BRF_PRG }, //  2	Z80 Program 
	
	{ "31a12-0.ic33",  0x020000, 0xd0803e20, BRF_GRA },	      //  3	Chars
	
	{ "31j0.ic1",      0x040000, 0x8a12b450, BRF_GRA },	      //  4	Tiles
	{ "31j1.ic2",      0x040000, 0x82ed7155, BRF_GRA },	      //  5
	
	{ "31j3.ic9",      0x100000, 0xe395cf1d, BRF_GRA },	      //  6	Sprites
	{ "31j2.ic8",      0x100000, 0xb5a97465, BRF_GRA },	      //  7
	{ "31j5.ic11",     0x100000, 0x2ce545e8, BRF_GRA },	      //  8
	{ "31j4.ic10",     0x100000, 0x00edb66a, BRF_GRA }, 	      //  9
	{ "31j6.ic12",     0x100000, 0x79956cf8, BRF_GRA },  	      //  10
	{ "31j7.ic13",     0x100000, 0x74d774c3, BRF_GRA }, 	      //  11
	{ "31j9.ic15",     0x100000, 0xdd387289, BRF_GRA },	      //  12
	{ "31j8.ic14",     0x100000, 0x44abe127, BRF_GRA },	      //  13
	
	{ "31j10.ic73",    0x080000, 0x6c522edb, BRF_SND },	      //  14	Samples
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo DrvaRomDesc[] = {
	{ "wf_19.rom",     0x040000, 0xbd02e3c4, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "wf_18.rom",     0x040000, 0x933ea1a0, BRF_ESS | BRF_PRG }, //  1	
	
	{ "31a11-2.ic42",  0x010000, 0x5ddebfea, BRF_ESS | BRF_PRG }, //  2	Z80 Program 
	
	{ "wf_33.rom",     0x020000, 0x06f22615, BRF_GRA },	      //  3	Chars
	
	{ "31j0.ic1",      0x040000, 0x8a12b450, BRF_GRA },	      //  4	Tiles
	{ "31j1.ic2",      0x040000, 0x82ed7155, BRF_GRA },	      //  5
	
	{ "31j3.ic9",      0x100000, 0xe395cf1d, BRF_GRA },	      //  6	Sprites
	{ "31j2.ic8",      0x100000, 0xb5a97465, BRF_GRA },	      //  7
	{ "31j5.ic11",     0x100000, 0x2ce545e8, BRF_GRA },	      //  8
	{ "31j4.ic10",     0x100000, 0x00edb66a, BRF_GRA }, 	      //  9
	{ "31j6.ic12",     0x100000, 0x79956cf8, BRF_GRA },  	      //  10
	{ "31j7.ic13",     0x100000, 0x74d774c3, BRF_GRA }, 	      //  11
	{ "31j9.ic15",     0x100000, 0xdd387289, BRF_GRA },	      //  12
	{ "31j8.ic14",     0x100000, 0x44abe127, BRF_GRA },	      //  13
	
	{ "31j10.ic73",    0x080000, 0x6c522edb, BRF_SND },	      //  14	Samples
};

STD_ROM_PICK(Drva)
STD_ROM_FN(Drva)

static struct BurnRomInfo DrvbRomDesc[] = {
	{ "2",             0x040000, 0x632bb3a4, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "3",             0x040000, 0xea73369c, BRF_ESS | BRF_PRG }, //  1	
	
	{ "1",             0x010000, 0xd9e8cda2, BRF_ESS | BRF_PRG }, //  2	Z80 Program 
	
	{ "4",             0x020000, 0x520ef575, BRF_GRA },	      //  3	Chars
	
	{ "5",             0x020000, 0x35e4d6eb, BRF_GRA },	      //  4	Tiles
	{ "6",             0x020000, 0xa054a5b2, BRF_GRA },	      //  5
	{ "7",             0x020000, 0x101f0136, BRF_GRA },	      //  6
	{ "8",             0x020000, 0x7b2ecba7, BRF_GRA },	      //  7
	
	{ "wf_09.rom",     0x100000, 0xe395cf1d, BRF_GRA },	      //  8	Sprites
	{ "wf_08.rom",     0x100000, 0xb5a97465, BRF_GRA },	      //  9
	{ "wf_11.rom",     0x100000, 0x2ce545e8, BRF_GRA },	      //  10
	{ "wf_10.rom",     0x100000, 0x00edb66a, BRF_GRA }, 	      //  11
	{ "wf_12.rom",     0x100000, 0x79956cf8, BRF_GRA },  	      //  12
	{ "wf_13.rom",     0x100000, 0x74d774c3, BRF_GRA }, 	      //  13
	{ "wf_15.rom",     0x100000, 0xdd387289, BRF_GRA },	      //  14
	{ "wf_14.rom",     0x100000, 0x44abe127, BRF_GRA },	      //  15
	
	{ "wf_73a.rom",    0x080000, 0x6c522edb, BRF_SND },	      //  16	Samples
};

STD_ROM_PICK(Drvb)
STD_ROM_FN(Drvb)

static struct BurnRomInfo DrvjRomDesc[] = {
	{ "31j13-0.ic19",  0x040000, 0x2147780d, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "31j14-0.ic18",  0x040000, 0xd76fc747, BRF_ESS | BRF_PRG }, //  1
	
	{ "31a11-2.ic42",  0x010000, 0x5ddebfea, BRF_ESS | BRF_PRG }, //  2	Z80 Program 
	
	{ "31j12-0.ic33",  0x020000, 0xf4821fe0, BRF_GRA },	      //  3	Chars
	
	{ "31j0.ic1",      0x040000, 0x8a12b450, BRF_GRA },	      //  4	Tiles
	{ "31j1.ic2",      0x040000, 0x82ed7155, BRF_GRA },	      //  5
	
	{ "31j3.ic9",      0x100000, 0xe395cf1d, BRF_GRA },	      //  6	Sprites
	{ "31j2.ic8",      0x100000, 0xb5a97465, BRF_GRA },	      //  7
	{ "31j5.ic11",     0x100000, 0x2ce545e8, BRF_GRA },	      //  8
	{ "31j4.ic10",     0x100000, 0x00edb66a, BRF_GRA }, 	      //  9
	{ "31j6.ic12",     0x100000, 0x79956cf8, BRF_GRA },  	      //  10
	{ "31j7.ic13",     0x100000, 0x74d774c3, BRF_GRA }, 	      //  11
	{ "31j9.ic15",     0x100000, 0xdd387289, BRF_GRA },	      //  12
	{ "31j8.ic14",     0x100000, 0x44abe127, BRF_GRA },	      //  13
	
	{ "31j10.ic73",    0x080000, 0x6c522edb, BRF_SND },	      //  14	Samples
};

STD_ROM_PICK(Drvj)
STD_ROM_FN(Drvj)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Drv68KRom              = Next; Next += 0x80000;
	DrvZ80Rom              = Next; Next += 0x10000;
	MSM6295ROM             = Next; Next += 0x40000;
	DrvMSM6295ROMSrc       = Next; Next += 0x80000;

	RamStart               = Next;

	Drv68KRam              = Next; Next += 0x04000;
	DrvZ80Ram              = Next; Next += 0x00800;
	DrvCharVideoRam        = Next; Next += 0x02000;
	DrvSpriteRam           = Next; Next += 0x02000;
	DrvSpriteRamBuff       = Next; Next += 0x02000;
	DrvBg0VideoRam         = Next; Next += 0x01000;
	DrvBg1VideoRam         = Next; Next += 0x01000;
	DrvPaletteRam          = Next; Next += 0x04000;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x01000 * 8 * 8;
	DrvTiles               = Next; Next += 0x01000 * 16 * 16;
	DrvSprites             = Next; Next += 0x10000 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x02000 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();
	
	ZetOpen(0);
	ZetReset();
	ZetClose();
	
	BurnYM2151Reset();
	MSM6295Reset(0);
	
	DrvVBlank = 0;
	DrvBg0ScrollX = 0;
	DrvBg0ScrollY = 0;
	DrvBg1ScrollX = 0;
	DrvBg1ScrollY = 0;
	DrvVReg = 0;
	DrvOkiBank = 0;
	DrvSoundLatch = 0;
	
	return 0;
}

UINT8 __fastcall Wwfwfest68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x140020: {
			return (0xcf - DrvInput[4]) | ((DrvDip[1] & 0xc0) >> 2);
		}
		
		case 0x140021: {
			return 0xff - DrvInput[0];
		}
		
		case 0x140023: {
			return 0xff - DrvInput[1];
		}
		
		case 0x140025: {
			return 0xff - DrvInput[2];
		}
		
		case 0x140026: {
			UINT8 Temp = 0xfc;
			if (DrvVBlank) Temp = 0xfb;
			Temp |= (DrvDip[0] & 0xc0) >> 6;
			return Temp;
		}
		
		case 0x140027: {
			return 0xff - DrvInput[3];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Wwfwfest68KWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x0c0000 && a <= 0x0c1fff) {
		UINT16 *CharRam = (UINT16*)DrvCharVideoRam;
		CharRam[(a - 0x0c0000) >> 1] = d;
		return;
	}
	
	switch (a) {
		case 0x140011: {
			DrvVReg = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Wwfwfest68KReadWord(UINT32 a)
{
	if (a >= 0x180000 && a <= 0x18ffff) {
		UINT16 *PaletteRam = (UINT16*)DrvPaletteRam;
		INT32 Offset = (a - 0x180000) >> 1;
		Offset = (Offset & 0x0f) | (Offset & 0x7fc0) >> 2;
		return PaletteRam[Offset];
	}
	
	switch (a) {
		case 0x140020: {
			UINT16 Temp = (0xff - DrvInput[0]) | ((0xff - DrvInput[4]) << 8);
			Temp &= 0xcfff;
			Temp |= ((DrvDip[1] & 0xc0) << 6);
			return Temp;
		}
		
		case 0x140022: {
			UINT16 Temp = 0xff - DrvInput[1];
			Temp |= ((DrvDip[1] & 0x3f) << 8);
			return Temp;
		}
		
		case 0x140024: {
			UINT16 Temp = 0xff - DrvInput[2];
			Temp |= ((DrvDip[0] & 0x3f) << 8);
			return Temp;
		}
		
		case 0x140026: {
			UINT16 Temp = 0xff - DrvInput[3];
			if (DrvVBlank) {
				Temp |= (0xfb << 8);
			} else {
				Temp |= (0xff << 8);
			}
			Temp &= 0xfcff;
			Temp |= ((DrvDip[0] & 0xc0) << 2);
			return Temp;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Wwfwfest68KWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x0c0000 && a <= 0x0c1fff) {
		UINT16 *CharRam = (UINT16*)DrvCharVideoRam;
		CharRam[(a - 0x0c0000) >> 1] = d;
		return;
	}
	
	if (a >= 0x180000 && a <= 0x18ffff) {
		UINT16 *PaletteRam = (UINT16*)DrvPaletteRam;
		INT32 Offset = (a - 0x180000) >> 1;
		Offset = (Offset & 0x0f) | (Offset & 0x7fc0) >> 2;
		PaletteRam[Offset] = d;
		return;
	}
	
	if (a >= 0x120000 && a <= 0x121fff) {
		return;
	}
	
	switch (a) {
		case 0x100000: {
			DrvBg0ScrollX = d & 0x1ff;
			return;
		}
		
		case 0x100002: {
			DrvBg0ScrollY = d & 0x1ff;
			return;
		}
		
		case 0x100004: {
			DrvBg1ScrollX = d & 0x1ff;
			return;
		}
		
		case 0x100006: {
			DrvBg1ScrollY = d & 0x1ff;
			return;
		}
		
		case 0x140010: {
			DrvVReg = d;
			return;
		}
		
		case 0x14000c: {
			DrvSoundLatch = d & 0xff;
			ZetOpen(0);
			ZetNmi();
			nCyclesDone[1] += ZetRun(100);
			ZetClose();
			return;
		}
		
		case 0x100008: {
			// ???
			return;
		}
		
		case 0x10000c: {
			// ???
			return;
		}
		
		case 0x140000:
		case 0x140002: {
			// NOP
			return;
		}

		case 0x140006:
		case 0x140008:
		case 0x140012:
		case 0x140014:
		case 0x140016: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 __fastcall WwfwfestZ80Read(UINT16 a)
{
	switch (a) {
		case 0xc801: {
			return BurnYM2151ReadStatus();
		}
		
		case 0xd800: {
			return MSM6295ReadStatus(0);
		}
		
		case 0xe000: {
			return DrvSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall WwfwfestZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xc800: {
			BurnYM2151SelectRegister(d);
			return;
		}
		
		case 0xc801: {
			BurnYM2151WriteRegister(d);
			return;
		}
		
		case 0xd800: {
			MSM6295Command(0, d);
			return;
		}
		
		case 0xe800: {
			DrvOkiBank = d & 1;
			memcpy(MSM6295ROM + 0x00000, DrvMSM6295ROMSrc + (0x40000 * DrvOkiBank), 0x40000);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static INT32 CharPlaneOffsets[4]    = { 0, 2, 4, 6 };
static INT32 CharXOffsets[8]        = { 1, 0, 65, 64, 129, 128, 193, 192 };
static INT32 CharYOffsets[8]        = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 TilePlaneOffsets[4]    = { 8, 0, 0x200008, 0x200000 };
static INT32 TileXOffsets[16]       = { 0, 1, 2, 3, 4, 5, 6, 7, 256, 257, 258, 259, 260, 261, 262, 263 };
static INT32 TileYOffsets[16]       = { 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240 };
static INT32 SpritePlaneOffsets[4]  = { 0, 0x1000000, 0x2000000, 0x3000000 };
static INT32 SpriteXOffsets[16]     = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
static INT32 SpriteYOffsets[16]     = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };

static void DrvYM2151IrqHandler(INT32 Irq)
{
	if (Irq) {
		ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvInit()
{
	INT32 nRet = 0, nLen, RomOffset;
	
	RomOffset = 0;
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "wwfwfestb")) RomOffset = 2;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x800000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 1, 2); if (nRet != 0) return 1;
	
	// Load Z80 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom, 2, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 3, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x100, DrvTempRom, DrvChars);

	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x800000);
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "wwfwfestb")) {
		nRet = BurnLoadRom(DrvTempRom + 0x040000, 4, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x040001, 5, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x000000, 6, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x000001, 7, 2); if (nRet != 0) return 1;
	} else {
		nRet = BurnLoadRom(DrvTempRom + 0x040000, 4, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(DrvTempRom + 0x000000, 5, 1); if (nRet != 0) return 1;
	}
	GfxDecode(0x1000, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x200, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x800000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000,  6 + RomOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000,  7 + RomOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x200000,  8 + RomOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x300000,  9 + RomOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x400000, 10 + RomOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x500000, 11 + RomOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x600000, 12 + RomOffset, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x700000, 13 + RomOffset, 1); if (nRet != 0) return 1;
	GfxDecode(0x10000, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, DrvTempRom, DrvSprites);
	
	// Load Sample Roms
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x00000, 14 + RomOffset, 1); if (nRet != 0) return 1;
	memcpy(MSM6295ROM, DrvMSM6295ROMSrc, 0x40000);
	
	BurnFree(DrvTempRom);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvBg0VideoRam      , 0x080000, 0x080fff, SM_RAM);
	SekMapMemory(DrvBg1VideoRam      , 0x082000, 0x082fff, SM_RAM);
	SekMapMemory(DrvCharVideoRam     , 0x0c0000, 0x0c1fff, SM_ROM);
	SekMapMemory(DrvSpriteRam        , 0x0c2000, 0x0c3fff, SM_RAM);
	SekMapMemory(Drv68KRam           , 0x1c0000, 0x1c3fff, SM_RAM);
	SekSetReadWordHandler(0, Wwfwfest68KReadWord);
	SekSetWriteWordHandler(0, Wwfwfest68KWriteWord);
	SekSetReadByteHandler(0, Wwfwfest68KReadByte);
	SekSetWriteByteHandler(0, Wwfwfest68KWriteByte);
	SekClose();
	
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(WwfwfestZ80Read);
	ZetSetWriteHandler(WwfwfestZ80Write);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80Rom                );
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80Rom                );
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80Ram                );
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80Ram                );
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80Ram                );
	ZetMemEnd();
	ZetClose();
	
	// Setup the YM2151 emulation
	BurnYM2151Init(3579545, 25.0);
	BurnYM2151SetIrqHandler(&DrvYM2151IrqHandler);
	
	// Setup the OKIM6295 emulation
	MSM6295Init(0, 1024188 / 132, 100.0, 1);
	
	DrvSpriteXOffset = 0;
	DrvBg0XOffset = 0;
	DrvBg1XOffset[0] = 0;
	DrvBg1XOffset[1] = 0;
	
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "wwfwfestb")) {
		DrvSpriteXOffset = 2;
		DrvBg0XOffset = -4;
		DrvBg1XOffset[0] = -4;
		DrvBg1XOffset[1] = -2;
	}
	
	GenericTilesInit();

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	SekExit();
	ZetExit();
	
	BurnYM2151Exit();
	MSM6295Exit(0);
	
	GenericTilesExit();
	
	DrvVBlank = 0;
	DrvBg0ScrollX = 0;
	DrvBg0ScrollY = 0;
	DrvBg1ScrollX = 0;
	DrvBg1ScrollY = 0;
	DrvVReg = 0;
	DrvOkiBank = 0;
	DrvSoundLatch = 0;
	
	DrvSpriteXOffset = 0;
	DrvBg0XOffset = 0;
	DrvBg1XOffset[0] = 0;
	DrvBg1XOffset[1] = 0;
	
	BurnFree(Mem);

	return 0;
}

static inline UINT8 pal4bit(UINT8 bits)
{
	bits &= 0x0f;
	return (bits << 4) | bits;
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal4bit(nColour >> 0);
	g = pal4bit(nColour >> 4);
	b = pal4bit(nColour >> 8);

	return BurnHighCol(r, g, b, 0);
}

static void DrvCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)DrvPaletteRam, pd = DrvPalette; i < 0x2000; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}
}

static void DrvRenderBg0Layer(INT32 Opaque)
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0, xScroll, yScroll, Flip, xFlip, yFlip;
	
	UINT16 *VideoRam = (UINT16*)DrvBg0VideoRam;
	UINT16 *TileBase;
	
	if (DrvVReg == 0x78) {
		xScroll = DrvBg0ScrollX + DrvBg0XOffset;
		yScroll = DrvBg0ScrollY;
	} else {
		xScroll = DrvBg1ScrollX + DrvBg0XOffset;
		yScroll = DrvBg1ScrollY;
	}
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			TileBase = &VideoRam[TileIndex * 2];
			Code = TileBase[1] & 0xfff;
			Colour = TileBase[0] & 0x0f;
			Flip = (TileBase[0] & 0xc0) >> 6;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			
			x = 16 * mx;
			y = 16 * my;
			
			x -= xScroll;
			y -= yScroll;
			if (x < -16) x += 512;
			if (y < -16) y += 512;
			
			y -= 8;

			if (Opaque) {
				if (x > 16 && x < 304 && y > 16 && y < 224) {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0x1000, DrvTiles);
						} else {
							Render16x16Tile_FlipX(pTransDraw, Code, x, y, Colour, 4, 0x1000, DrvTiles);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_FlipY(pTransDraw, Code, x, y, Colour, 4, 0x1000, DrvTiles);
						} else {
							Render16x16Tile(pTransDraw, Code, x, y, Colour, 4, 0x1000, DrvTiles);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0x1000, DrvTiles);
						} else {
							Render16x16Tile_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0x1000, DrvTiles);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0x1000, DrvTiles);
						} else {
							Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 0x1000, DrvTiles);
						}
					}
				}
			} else {
				if (x > 16 && x < 304 && y > 16 && y < 224) {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, 0x1000, DrvTiles);
						} else {
							Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 0x1000, DrvTiles);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, 0x1000, DrvTiles);
						} else {
							Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0x1000, DrvTiles);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0x1000, DrvTiles);
						} else {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0x1000, DrvTiles);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0x1000, DrvTiles);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0x1000, DrvTiles);
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}

static void DrvRenderBg1Layer(INT32 Opaque)
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0, xScroll, yScroll;
	
	UINT16 *VideoRam = (UINT16*)DrvBg1VideoRam;
	UINT16 *TileBase;
	
	if (DrvVReg == 0x78) {
		xScroll = DrvBg1ScrollX + DrvBg1XOffset[0];
		yScroll = DrvBg1ScrollY;
	} else {
		xScroll = DrvBg0ScrollX + DrvBg1XOffset[1];
		yScroll = DrvBg0ScrollY;
	}

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			TileBase = &VideoRam[TileIndex];
			Code = TileBase[0] & 0xfff;
			Colour = (TileBase[0] & 0xf000) >> 12;
		
			x = 16 * mx;
			y = 16 * my;
			
			x -= xScroll;
			y -= yScroll;
			if (x < -16) x += 512;
			if (y < -16) y += 512;
			
			y -= 8;

			if (Opaque) {
				if (x > 0 && x < 304 && y > 0 && y < 224) {
					Render16x16Tile(pTransDraw, Code, x, y, Colour, 4, 0x0c00, DrvTiles);
				} else {
					Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 0x0c00, DrvTiles);
				}
			} else {
				if (x > 0 && x < 304 && y > 0 && y < 224) {
					Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0x0c00, DrvTiles);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0x0c00, DrvTiles);
				}
			}
			
			TileIndex++;
		}
	}
}

static void DrvRenderSprites()
{
	UINT16 *Source = (UINT16*)DrvSpriteRamBuff;
	UINT16 *Finish = (UINT16*)DrvSpriteRamBuff + 0x1000;
	
	while (Source < Finish) {
		INT32 x, y, Colour, xFlip, yFlip, Chain, Enable, Code, Count;
		
		Enable = Source[1] & 1;
		
		if (Enable) {
			x = +(Source[5] & 0x00ff) | (Source[1] & 0x0004) << 6;
			if (x > 512 - 16) x -= 512;
			x += DrvSpriteXOffset;
			y = (Source[0] & 0x00ff) | (Source[1] & 0x0002) << 7;
			y = (256 - y) & 0x1ff;
			y -= 16;
			y -= 8;
			xFlip = (Source[1] & 0x0010) >> 4;
			yFlip = (Source[1] & 0x0008) >> 3;
			Chain = (Source[1] & 0x00e0) >> 5;
			Chain += 1;
			Code = (Source[2] & 0x00ff) | (Source[3] & 0x00ff) << 8;
			Colour = (Source[4] & 0x000f);
			
			for (Count = 0; Count < Chain; Count++) {
				INT32 yPos;
				yPos = y - 16 * Count;
				if (yFlip) yPos = y - (16 * (Chain - 1)) + (16 * Count);
				
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code + Count, x, yPos, Colour, 4, 0, 0x400, DrvSprites);
					} else {
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code + Count, x, yPos, Colour, 4, 0, 0x400, DrvSprites);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code + Count, x, yPos, Colour, 4, 0, 0x400, DrvSprites);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, Code + Count, x, yPos, Colour, 4, 0, 0x400, DrvSprites);
					}
				}
			}
		}
		
		Source += 8;	
	}
}

static void DrvRenderCharLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;
	
	UINT16 *VideoRam = (UINT16*)DrvCharVideoRam;
	UINT16 *TileBase;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			TileBase = &VideoRam[TileIndex * 2];
			Code = (TileBase[0] & 0xff) | ((TileBase[1] & 0x0f) << 8);
			Colour = (TileBase[1] & 0xf0) >> 4;
			
			x = 8 * mx;
			y = 8 * my;
			
			y -= 8;

			if (x > 0 && x < 312 && y > 0 && y < 232) {
				Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvChars);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvChars);
			}
			
			TileIndex++;
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	
	if (DrvVReg == 0x7b) {
		DrvRenderBg0Layer(1);
		DrvRenderBg1Layer(0);
		DrvRenderSprites();
		DrvRenderCharLayer();
	}
	
	if (DrvVReg == 0x7c) {
		DrvRenderBg0Layer(1);
		DrvRenderSprites();
		DrvRenderBg1Layer(0);
		DrvRenderCharLayer();
	}
	
	if (DrvVReg == 0x78) {
		DrvRenderBg1Layer(1);
		DrvRenderBg0Layer(0);
		DrvRenderSprites();
		DrvRenderCharLayer();
	}	
	
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 10;
	INT32 nSoundBufferPos = 0;

	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	nCyclesTotal[0] = (24000000 / 2) / 60;
	nCyclesTotal[1] = 3579545 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	SekNewFrame();
	ZetNewFrame();
	
	DrvVBlank = 0;
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == 5) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		if (i == 5) DrvVBlank = 1;
		SekClose();
		
		// Run Z80
		nCurrentCPU = 1;
		ZetOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		ZetClose();
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	SekOpen(0);
	SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
	SekClose();
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			ZetOpen(0);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			ZetClose();
			MSM6295Render(0, pSoundBuf, nSegmentLength);
		}
	}
	
	if (pBurnDraw) DrvDraw();
	
	memcpy(DrvSpriteRamBuff, DrvSpriteRam, 0x2000);

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
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
		SekScan(nAction);
		ZetScan(nAction);			// Scan Z80
		BurnYM2151Scan(nAction);
		MSM6295Scan(0, nAction);

		// Scan critical driver variables
		SCAN_VAR(nCyclesDone);
		SCAN_VAR(nCyclesSegment);
		SCAN_VAR(DrvDip);
		SCAN_VAR(DrvInput);
		SCAN_VAR(DrvVBlank);
		SCAN_VAR(DrvOkiBank);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(DrvBg0ScrollX);
		SCAN_VAR(DrvBg0ScrollY);
		SCAN_VAR(DrvBg1ScrollX);
		SCAN_VAR(DrvBg1ScrollY);
		SCAN_VAR(DrvVReg);	
	}
	
	if (nAction & ACB_WRITE) {
		memcpy(MSM6295ROM + 0x00000, DrvMSM6295ROMSrc + (0x40000 * DrvOkiBank), 0x40000);
	}
	
	return 0;
}

struct BurnDriver BurnDrvWwfwfest = {
	"wwfwfest", NULL, NULL, NULL, "1991",
	"WWF WrestleFest (US set 1)\0", NULL, "Technos Japan", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_TECHNOS, GBF_VSFIGHT, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x2000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvWwfwfesta = {
	"wwfwfesta", "wwfwfest", NULL, NULL, "1991",
	"WWF WrestleFest (US Tecmo)\0", NULL, "Technos Japan (Tecmo License)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_TECHNOS, GBF_VSFIGHT, 0,
	NULL, DrvaRomInfo, DrvaRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x2000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvWwfwfestb = {
	"wwfwfestb", "wwfwfest", NULL, NULL, "1991",
	"WWF WrestleFest (US bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 4, HARDWARE_TECHNOS, GBF_VSFIGHT, 0,
	NULL, DrvbRomInfo, DrvbRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x2000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvWwfwfestj = {
	"wwfwfestj", "wwfwfest", NULL, NULL, "1991",
	"WWF WrestleFest (Japan)\0", NULL, "Technos Japan", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_TECHNOS, GBF_VSFIGHT, 0,
	NULL, DrvjRomInfo, DrvjRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x2000, 320, 240, 4, 3
};
