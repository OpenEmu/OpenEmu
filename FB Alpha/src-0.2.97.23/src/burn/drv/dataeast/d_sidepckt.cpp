#include "tiles_generic.h"
#include "m6809_intf.h"
#include "m6502_intf.h"
#include "burn_ym2203.h"
#include "burn_ym3526.h"

static UINT8 DrvInputPort0[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[2]            = {0, 0};
static UINT8 DrvInput[3]          = {0x00, 0x00, 0x00};
static UINT8 DrvReset             = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *DrvM6809Rom         = NULL;
static UINT8 *DrvM6502Rom         = NULL;
static UINT8 *DrvProm             = NULL;
static UINT8 *DrvM6809Ram         = NULL;
static UINT8 *DrvM6502Ram         = NULL;
static UINT8 *DrvVideoRam         = NULL;
static UINT8 *DrvColourRam        = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette         = NULL;

static INT32 I8751Return;
static INT32 CurrentPtr;
static INT32 CurrentTable;
static INT32 InMath;
static INT32 MathParam;

static UINT8 DrvSoundLatch;

typedef INT32 (*SidePcktLoadRoms)();
static SidePcktLoadRoms LoadRomsFunction;

typedef void (*SidePcktI8751Write)(UINT8);
static SidePcktI8751Write I8751WriteFunction;

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort1 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort1 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 7, "p2 start"  },
	
	{"Up"                , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },
	
	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort2 + 6, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Drv)

static inline void DrvMakeInputs()
{
	DrvInput[0] = DrvInput[1] = 0xff;
	DrvInput[2] = 0x40;

	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] -= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] -= (DrvInputPort1[i] & 1) << i;
	}
	
	if (DrvInputPort2[6]) DrvInput[2] -= 0x40;
}

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xfb, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x12, 0x01, 0x0c, 0x00, "4 Coins 1 Credit"       },
	{0x12, 0x01, 0x0c, 0x04, "3 Coins 1 Credit"       },
	{0x12, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"       },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x12, 0x01, 0x03, 0x03, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0x03, 0x02, "1 Coin  3 Credits"      },
	{0x12, 0x01, 0x03, 0x01, "1 Coin  4 Credits"      },
	{0x12, 0x01, 0x03, 0x00, "1 Coin  6 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x40, 0x00, "Off"                    },
	{0x12, 0x01, 0x40, 0x40, "On"                     },

	// Dip 2	
	{0   , 0xfe, 0   , 4   , "Timer Speed"            },
	{0x13, 0x01, 0x03, 0x00, "Stopped"                },
	{0x13, 0x01, 0x03, 0x03, "Slow"                   },
	{0x13, 0x01, 0x03, 0x02, "Medium"                 },
	{0x13, 0x01, 0x03, 0x01, "Fast"                   },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x0c, 0x04, "2"                      },
	{0x13, 0x01, 0x0c, 0x08, "3"                      },
	{0x13, 0x01, 0x0c, 0x0c, "6"                      },
	{0x13, 0x01, 0x0c, 0x00, "Infinite"               },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x30, 0x30, "10k 60k 50k+"           },
	{0x13, 0x01, 0x30, 0x20, "20k 70k 50k+"           },
	{0x13, 0x01, 0x30, 0x10, "30k 80k 50k+"           },
	{0x13, 0x01, 0x30, 0x00, "20k 70k 50k+"           },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo DrvjDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xfb, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x12, 0x01, 0x0c, 0x00, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"       },
	{0x12, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0x0c, 0x04, "1 Coin  3 Credits"      },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x12, 0x01, 0x03, 0x00, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0x03, 0x03, "1 Coin  1 Credit"       },
	{0x12, 0x01, 0x03, 0x02, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0x03, 0x01, "1 Coin  3 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x40, 0x00, "Off"                    },
	{0x12, 0x01, 0x40, 0x40, "On"                     },

	// Dip 2	
	{0   , 0xfe, 0   , 4   , "Timer Speed"            },
	{0x13, 0x01, 0x03, 0x00, "Stopped"                },
	{0x13, 0x01, 0x03, 0x03, "Slow"                   },
	{0x13, 0x01, 0x03, 0x02, "Medium"                 },
	{0x13, 0x01, 0x03, 0x01, "Fast"                   },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x0c, 0x04, "2"                      },
	{0x13, 0x01, 0x0c, 0x08, "3"                      },
	{0x13, 0x01, 0x0c, 0x0c, "6"                      },
	{0x13, 0x01, 0x0c, 0x00, "Infinite"               },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x13, 0x01, 0x30, 0x30, "10k 60k 50k+"           },
	{0x13, 0x01, 0x30, 0x20, "20k 70k 50k+"           },
	{0x13, 0x01, 0x30, 0x10, "30k 80k 50k+"           },
	{0x13, 0x01, 0x30, 0x00, "20k 70k 50k+"           },
};

STDDIPINFO(Drvj)

static struct BurnDIPInfo DrvbDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	{0x13, 0xff, 0xff, 0xfb, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x12, 0x01, 0x0c, 0x00, "4 Coins 1 Credit"       },
	{0x12, 0x01, 0x0c, 0x04, "3 Coins 1 Credit"       },
	{0x12, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"       },
	
	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x12, 0x01, 0x03, 0x03, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0x03, 0x02, "1 Coin  3 Credits"      },
	{0x12, 0x01, 0x03, 0x01, "1 Coin  4 Credits"      },
	{0x12, 0x01, 0x03, 0x00, "1 Coin  6 Credits"      },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x40, 0x00, "Off"                    },
	{0x12, 0x01, 0x40, 0x40, "On"                     },

	// Dip 2	
	{0   , 0xfe, 0   , 4   , "Timer Speed"            },
	{0x13, 0x01, 0x03, 0x00, "Stopped"                },
	{0x13, 0x01, 0x03, 0x03, "Medium"                 },
	{0x13, 0x01, 0x03, 0x02, "Fast"                   },
	{0x13, 0x01, 0x03, 0x01, "Fastest"                },
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x13, 0x01, 0x0c, 0x04, "2"                      },
	{0x13, 0x01, 0x0c, 0x08, "3"                      },
	{0x13, 0x01, 0x0c, 0x0c, "6"                      },
	{0x13, 0x01, 0x0c, 0x00, "Infinite"               },
};

STDDIPINFO(Drvb)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "dh00",          0x10000, 0x251b316e, BRF_ESS | BRF_PRG }, 	//  0	M6809 Program Code
	
	{ "dh04.bin",      0x08000, 0xd076e62e, BRF_ESS | BRF_PRG }, 	//  1	M6502 Program Code
	
	{ "i8751.mcu",     0x08000, 0x00000000, BRF_OPT | BRF_NODUMP }, //  2	I8751 MCU
	
	{ "sp_07.bin",     0x08000, 0x9d6f7969, BRF_GRA },	    		//  3	Chars
	{ "sp_06.bin",     0x08000, 0x580e4e43, BRF_GRA },	     		//  4
	{ "sp_05.bin",     0x08000, 0x05ab71d2, BRF_GRA },	     		//  5
	
	{ "dh01.bin",      0x08000, 0xa2cdfbea, BRF_GRA },	     		//  6	Sprites
	{ "dh02.bin",      0x08000, 0xeeb5c3e7, BRF_GRA },	     		//  7
	{ "dh03.bin",      0x08000, 0x8e18d21d, BRF_GRA },	     		//  8
	
	{ "dh-09.bpr",     0x00100, 0xce049b4f, BRF_GRA },	     		//  9	PROMS
	{ "dh-08.bpr",     0x00100, 0xcdf2180f, BRF_GRA },	     		// 10
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo DrvjRomDesc[] = {
	{ "dh00.bin",      0x10000, 0xa66bc28d, BRF_ESS | BRF_PRG }, 	//  0	M6809 Program Code
	
	{ "dh04.bin",      0x08000, 0xd076e62e, BRF_ESS | BRF_PRG }, 	//  1	M6502 Program Code
	
	{ "i8751.mcu",     0x08000, 0x00000000, BRF_OPT | BRF_NODUMP }, //  2	I8751 MCU
	
	{ "dh07.bin",      0x08000, 0x7d0ce858, BRF_GRA },	    		//  3	Chars
	{ "dh06.bin",      0x08000, 0xb86ddf72, BRF_GRA },	     		//  4
	{ "dh05.bin",      0x08000, 0xdf6f94f2, BRF_GRA },	     		//  5
	
	{ "dh01.bin",      0x08000, 0xa2cdfbea, BRF_GRA },	     		//  6	Sprites
	{ "dh02.bin",      0x08000, 0xeeb5c3e7, BRF_GRA },	     		//  7
	{ "dh03.bin",      0x08000, 0x8e18d21d, BRF_GRA },	     		//  8
	
	{ "dh-09.bpr",     0x00100, 0xce049b4f, BRF_GRA },	     		//  9	PROMS
	{ "dh-08.bpr",     0x00100, 0xcdf2180f, BRF_GRA },	     		// 10
};

STD_ROM_PICK(Drvj)
STD_ROM_FN(Drvj)

static struct BurnRomInfo DrvbRomDesc[] = {
	{ "sp_09.bin",     0x04000, 0x3c6fe54b, BRF_ESS | BRF_PRG }, 	//  0	M6809 Program Code
	{ "sp_08.bin",     0x08000, 0x347f81cd, BRF_ESS | BRF_PRG }, 	//  1
	
	{ "dh04.bin",      0x08000, 0xd076e62e, BRF_ESS | BRF_PRG }, 	//  2	M6502 Program Code

	{ "sp_07.bin",     0x08000, 0x9d6f7969, BRF_GRA },	    		//  3	Chars
	{ "sp_06.bin",     0x08000, 0x580e4e43, BRF_GRA },	     		//  4
	{ "sp_05.bin",     0x08000, 0x05ab71d2, BRF_GRA },	     		//  5
	
	{ "dh01.bin",      0x08000, 0xa2cdfbea, BRF_GRA },	     		//  6	Sprites
	{ "dh02.bin",      0x08000, 0xeeb5c3e7, BRF_GRA },	     		//  7
	{ "dh03.bin",      0x08000, 0x8e18d21d, BRF_GRA },	     		//  8
	
	{ "dh-09.bpr",     0x00100, 0xce049b4f, BRF_GRA },	     		//  9	PROMS
	{ "dh-08.bpr",     0x00100, 0xcdf2180f, BRF_GRA },	     		// 10
};

STD_ROM_PICK(Drvb)
STD_ROM_FN(Drvb)

static struct BurnRomInfo Drvb2RomDesc[] = {
	{ "b-9.2a",        0x04000, 0x40fd0d85, BRF_ESS | BRF_PRG }, 	//  0	M6809 Program Code
	{ "b-8.3a",        0x08000, 0x26e0116a, BRF_ESS | BRF_PRG }, 	//  1
	
	{ "b-4.7a",        0x08000, 0xd076e62e, BRF_ESS | BRF_PRG }, 	//  2	M6502 Program Code

	{ "b-7.9m",        0x08000, 0x9d6f7969, BRF_GRA },	    		//  3	Chars
	{ "b-6.8m",        0x08000, 0x580e4e43, BRF_GRA },	     		//  4
	{ "b-5.7m",        0x08000, 0x05ab71d2, BRF_GRA },	     		//  5
	
	{ "b-1.1p",        0x08000, 0xa2cdfbea, BRF_GRA },	     		//  6	Sprites
	{ "b-2.1r",        0x08000, 0xeeb5c3e7, BRF_GRA },	     		//  7
	{ "b-3.1t",        0x08000, 0x8e18d21d, BRF_GRA },	     		//  8
	
	{ "dh-09.bpr",     0x00100, 0xce049b4f, BRF_GRA },	     		//  9	PROMS
	{ "dh-08.bpr",     0x00100, 0xcdf2180f, BRF_GRA },	     		// 10
};

STD_ROM_PICK(Drvb2)
STD_ROM_FN(Drvb2)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvM6809Rom            = Next; Next += 0x010000;
	DrvM6502Rom            = Next; Next += 0x008000;
	DrvProm                = Next; Next += 0x000200;

	RamStart               = Next;
	
	DrvM6809Ram            = Next; Next += 0x001c00;
	DrvM6502Ram            = Next; Next += 0x001000;
	DrvVideoRam            = Next; Next += 0x000400;
	DrvColourRam           = Next; Next += 0x000400;
	DrvSpriteRam           = Next; Next += 0x000100;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x0800 * 8 * 8;
	DrvSprites             = Next; Next += 0x0400 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00100 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	M6809Open(0);
	M6809Reset();
	BurnYM2203Reset();
	M6809Close();
	
	M6502Open(0);
	M6502Reset();
	BurnYM3526Reset();
	M6502Close();	
	
	I8751Return = 0;
	CurrentPtr = 0;
	CurrentTable = 0;
	InMath = 0;
	MathParam = 0;
	
	DrvSoundLatch = 0;
	
	return 0;
}

static void SidecpcktI8751Write(UINT8 Data)
{
	static const INT32 table_1[] = { 5,3,2 };
	static const INT32 table_2[] = { 0x8e, 0x42, 0xad, 0x58, 0xec, 0x85, 0xdd, 0x4c, 0xad, 0x9f, 0x00, 0x4c, 0x7e, 0x42, 0xa2, 0xff };
	static const INT32 table_3[] = { 0xbd, 0x73, 0x80, 0xbd, 0x73, 0xa7, 0xbd, 0x73, 0xe0, 0x7e, 0x72, 0x56, 0xff, 0xff, 0xff, 0xff };

	M6809SetIRQLine(M6809_FIRQ_LINE, M6809_IRQSTATUS_AUTO);

	if (InMath == 1) {
		InMath=2;
		I8751Return = MathParam = Data;
	}
	else if (InMath == 2) {
		InMath = 0;
		I8751Return = MathParam / Data;
	}
	else switch (Data) {
		case 1: {
			CurrentTable = 1;
			CurrentPtr = 0;
			I8751Return = table_1[CurrentPtr++];
			break;
		}

		case 2: {
			CurrentTable = 2;
			CurrentPtr = 0;
			I8751Return = table_2[CurrentPtr++];
			break;
		}

		case 3: {
			CurrentTable = 3;
			CurrentPtr = 0;
			I8751Return = table_3[CurrentPtr++];
			break;
		}

		case 4: {
			InMath = 1;
			I8751Return = 4;
			break;
		}

		case 6: {
			if (CurrentTable == 1) I8751Return = table_1[CurrentPtr++];
			if (CurrentTable == 2) I8751Return = table_2[CurrentPtr++];
			if (CurrentTable == 3) I8751Return = table_3[CurrentPtr++];
			break;
		}
	}
}

static void SidecpcktjI8751Write(UINT8 Data)
{
	static const INT32 table_1[] = { 5, 3, 0 };
	static const INT32 table_2[] = { 0x8e, 0x42, 0xb2, 0x58, 0xec, 0x85, 0xdd, 0x4c, 0xad, 0x9f, 0x00, 0x4c, 0x7e, 0x42, 0xa7, 0xff };
	static const INT32 table_3[] = { 0xbd, 0x71, 0xc8, 0xbd, 0x71, 0xef, 0xbd, 0x72, 0x28, 0x7e, 0x70, 0x9e, 0xff, 0xff, 0xff, 0xff };

	M6809SetIRQLine(M6809_FIRQ_LINE, M6809_IRQSTATUS_AUTO);

	if (InMath == 1) {
		InMath = 2;
		I8751Return = MathParam = Data;
	}
	else if (InMath == 2) {
		InMath = 0;
		I8751Return = MathParam / Data;
	}
	else switch (Data) {
		case 1: {
			CurrentTable = 1;
			CurrentPtr = 0;
			I8751Return = table_1[CurrentPtr++];
			break;
		}

		case 2: {
			CurrentTable = 2;
			CurrentPtr = 0;
			I8751Return = table_2[CurrentPtr++];
			break;
		}

		case 3: {
			CurrentTable = 3;
			CurrentPtr = 0;
			I8751Return = table_3[CurrentPtr++];
			break;
		}

		case 4: {
			InMath = 1;
			I8751Return = 4;
			break;
		}

		case 6: {
			if (CurrentTable == 1) I8751Return = table_1[CurrentPtr++];
			if (CurrentTable == 2) I8751Return = table_2[CurrentPtr++];
			if (CurrentTable == 3) I8751Return = table_3[CurrentPtr++];
			break;
		}
	}
}

UINT8 SidepcktM6809ReadByte(UINT16 Address)
{
	switch (Address) {
		case 0x3000: {
			return DrvInput[0];
		}
		
		case 0x3001: {
			return DrvInput[1];
		}
		
		case 0x3002: {
			return DrvDip[0];
		}
		
		case 0x3003: {
			return DrvDip[1] | DrvInput[2];
		}
		
		case 0x300c: {
			// nop
			return 0;
		}
		
		case 0x3014: {
			return I8751Return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("M6809 Read Byte %04X\n"), Address);
		}
	}

	return 0;
}

void SidepcktM6809WriteByte(UINT16 Address, UINT8 Data)
{
	switch (Address) {
		case 0x3004: {
			DrvSoundLatch = Data;
			M6502SetIRQLine(M6502_INPUT_LINE_NMI, M6502_IRQSTATUS_AUTO);
			return;
		}
		
		case 0x300c: {
			// flipscreen
			return;
		}
		
		case 0x3018: {
			if (I8751WriteFunction) I8751WriteFunction(Data);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("M6809 Write Byte %04X, %02X\n"), Address, Data);
		}
	}
}

UINT8 SidepcktSoundReadByte(UINT16 Address)
{
	switch (Address) {
		case 0x3000: {
			return DrvSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("M6502 Read Byte %04X\n"), Address);
		}
	}

	return 0;
}

void SidepcktSoundWriteByte(UINT16 Address, UINT8 Data)
{
	switch (Address) {
		case 0x1000: {
			BurnYM2203Write(0, 0, Data);
			return;
		}
		
		case 0x1001: {
			BurnYM2203Write(0, 1, Data);
			return;
		}
		
		case 0x2000: {
			BurnYM3526Write(0, Data);
			return;
		}
		
		case 0x2001: {
			BurnYM3526Write(1, Data);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("M6502 Write Byte %04X, %02X\n"), Address, Data);
		}
	}
}

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)(M6809TotalCycles() * nSoundRate / 2000000);
}

inline static double DrvGetTime()
{
	return (double)M6809TotalCycles() / 2000000;
}

static void DrvFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		M6502SetIRQLine(M6502_IRQ_LINE, M6502_IRQSTATUS_ACK);
	} else {
		M6502SetIRQLine(M6502_IRQ_LINE, M6502_IRQSTATUS_NONE);
	}
}

static INT32 DrvYM3526SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)M6502TotalCycles() * nSoundRate / 1500000;
}

static INT32 CharPlaneOffsets[3]   = { 0, 0x40000, 0x80000 };
static INT32 CharXOffsets[8]       = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 CharYOffsets[8]       = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 SpritePlaneOffsets[3] = { 0, 0x40000, 0x80000 };
static INT32 SpriteXOffsets[16]    = { 128, 129, 130, 131, 132, 133, 134, 135, 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 SpriteYOffsets[16]    = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };

static INT32 SidepcktLoadRoms()
{
	INT32 nRet;
	
	nRet = BurnLoadRom(DrvM6809Rom + 0x00000,  0, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvM6502Rom + 0x00000,  1, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvTempRom  + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom  + 0x08000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom  + 0x10000,  5, 1); if (nRet != 0) return 1;
	GfxDecode(0x800, 3, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0, 0x18000);
	nRet = BurnLoadRom(DrvTempRom  + 0x00000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom  + 0x08000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom  + 0x10000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 3, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, DrvTempRom, DrvSprites);
	
	nRet = BurnLoadRom(DrvProm     + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProm     + 0x00100, 10, 1); if (nRet != 0) return 1;
	
	return 0;
}

static INT32 SidepcktbLoadRoms()
{
	INT32 nRet;
	
	nRet = BurnLoadRom(DrvM6809Rom + 0x04000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvM6809Rom + 0x08000,  1, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvM6502Rom + 0x00000,  2, 1); if (nRet != 0) return 1;
	
	nRet = BurnLoadRom(DrvTempRom  + 0x00000,  3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom  + 0x08000,  4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom  + 0x10000,  5, 1); if (nRet != 0) return 1;
	GfxDecode(0x800, 3, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, DrvTempRom, DrvChars);
	
	memset(DrvTempRom, 0, 0x18000);
	nRet = BurnLoadRom(DrvTempRom  + 0x00000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom  + 0x08000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom  + 0x10000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(0x400, 3, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, DrvTempRom, DrvSprites);
	
	nRet = BurnLoadRom(DrvProm     + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvProm     + 0x00100, 10, 1); if (nRet != 0) return 1;
	
	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;
	
	BurnSetRefreshRate(58.0);

	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();
	
	DrvTempRom = (UINT8 *)BurnMalloc(0x18000);
	
	if (LoadRomsFunction()) return 1;
		
	BurnFree(DrvTempRom);
	
	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvM6809Ram + 0x0000, 0x0000, 0x0fff, M6809_RAM);
	M6809MapMemory(DrvVideoRam         , 0x1000, 0x13ff, M6809_RAM);
	M6809MapMemory(DrvM6809Ram + 0x1000, 0x1400, 0x17ff, M6809_RAM);
	M6809MapMemory(DrvColourRam        , 0x1800, 0x1bff, M6809_RAM);
	M6809MapMemory(DrvM6809Ram + 0x1400, 0x1c00, 0x1fff, M6809_RAM);
	M6809MapMemory(DrvSpriteRam        , 0x2000, 0x20ff, M6809_RAM);
	M6809MapMemory(DrvM6809Ram + 0x1800, 0x2100, 0x24ff, M6809_RAM);
	M6809MapMemory(DrvM6809Rom + 0x4000, 0x4000, 0xffff, M6809_ROM);
	M6809SetReadHandler(SidepcktM6809ReadByte);
	M6809SetWriteHandler(SidepcktM6809WriteByte);
	M6809Close();
	
	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(DrvM6502Ram            , 0x0000, 0x0fff, M6502_RAM);
	M6502MapMemory(DrvM6502Rom            , 0x8000, 0xffff, M6502_ROM);
	M6502SetReadHandler(SidepcktSoundReadByte);
	M6502SetWriteHandler(SidepcktSoundWriteByte);
	M6502Close();	
	
	BurnYM2203Init(1, 1500000, NULL, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachM6809(2000000);
	BurnYM2203SetAllRoutes(0, 0.25, BURN_SND_ROUTE_BOTH);
	
	BurnYM3526Init(3000000, &DrvFMIRQHandler, &DrvYM3526SynchroniseStream, 1);
	BurnTimerAttachM6502YM3526(1500000);
	BurnYM3526SetRoute(BURN_SND_YM3526_ROUTE, 1.00, BURN_SND_ROUTE_BOTH);
	
	GenericTilesInit();
	
	DrvDoReset();

	return 0;
}

static INT32 SidepcktInit()
{
	I8751WriteFunction = SidecpcktI8751Write;
	LoadRomsFunction = SidepcktLoadRoms;
	
	return DrvInit();
}

static INT32 SidepcktjInit()
{
	I8751WriteFunction = SidecpcktjI8751Write;
	LoadRomsFunction = SidepcktLoadRoms;
	
	return DrvInit();
}

static INT32 SidepcktbInit()
{
	LoadRomsFunction = SidepcktbLoadRoms;
	
	return DrvInit();
}

static INT32 DrvExit()
{
	M6809Exit();
	M6502Exit();
	
	BurnYM2203Exit();
	BurnYM3526Exit();
	
	GenericTilesExit();
	
	BurnFree(Mem);
	
	I8751Return = 0;
	CurrentPtr = 0;
	CurrentTable = 0;
	InMath = 0;
	MathParam = 0;
	
	DrvSoundLatch = 0;
	
	I8751WriteFunction = NULL;
	LoadRomsFunction = NULL;
	
	return 0;
}

static void DrvCalcPalette()
{
	for (INT32 i = 0; i < 0x100; i++) {
		INT32 Bit0, Bit1, Bit2, Bit3, r, g, b;

		Bit0 = (DrvProm[i] >> 4) & 0x01;
		Bit1 = (DrvProm[i] >> 5) & 0x01;
		Bit2 = (DrvProm[i] >> 6) & 0x01;
		Bit3 = (DrvProm[i] >> 7) & 0x01;
		r = 0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		Bit0 = (DrvProm[i] >> 0) & 0x01;
		Bit1 = (DrvProm[i] >> 1) & 0x01;
		Bit2 = (DrvProm[i] >> 2) & 0x01;
		Bit3 = (DrvProm[i] >> 3) & 0x01;
		g = 0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		Bit0 = (DrvProm[i + 0x100] >> 0) & 0x01;
		Bit1 = (DrvProm[i + 0x100] >> 1) & 0x01;
		Bit2 = (DrvProm[i + 0x100] >> 2) & 0x01;
		Bit3 = (DrvProm[i + 0x100] >> 3) & 0x01;
		b = 0x0e * Bit0 + 0x1f * Bit1 + 0x43 * Bit2 + 0x8f * Bit3;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void DrvRenderBgLayer(INT32 RenderCategory)
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0, Category;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 32; mx++) {
			Attr = DrvColourRam[TileIndex];
			Code = (DrvVideoRam[TileIndex] + ((Attr & 0x07) << 8)) & 0x7ff;
			Colour = ((Attr & 0x10) >> 3) | ((Attr & 0x20) >> 5);
			Category = (Attr & 0x80) >> 7;
			
			if (Category == RenderCategory) {			
				x = 8 * mx;
				y = 8 * my;
				
				y -= 16;
				
				x = 248 - x;
				
				if (RenderCategory) {
					if (x > 8 && x < 248 && y > 8 && y < 216) {
						Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 3, 0, 128, DrvChars);
					} else {
						Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 3, 0, 128, DrvChars);
					}
				} else {
					if (x > 8 && x < 248 && y > 8 && y < 216) {
						Render8x8Tile(pTransDraw, Code, x, y, Colour, 3, 128, DrvChars);
					} else {
						Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 3, 128, DrvChars);
					}
				}
			}
			
			TileIndex++;
		}
	}
}

static void DrawSprites()
{
	for (INT32 Offs = 0; Offs < 0x100; Offs += 4) {
		INT32 sx, sy, Code, Colour, xFlip, yFlip;

		Code = (DrvSpriteRam[Offs + 3] + ((DrvSpriteRam[Offs + 1] & 0x03) << 8)) & 0x3ff;
		Colour = (DrvSpriteRam[Offs + 1] & 0xf0) >> 4;

		sx = DrvSpriteRam[Offs + 2] - 2;
		sy = DrvSpriteRam[Offs];

		xFlip = DrvSpriteRam[Offs + 1] & 0x08;
		yFlip = DrvSpriteRam[Offs + 1] & 0x04;
		
		sy -= 16;

		if (xFlip) {
			if (yFlip) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx, sy, Colour, 3, 0, 0, DrvSprites);
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx - 256, sy, Colour, 3, 0, 0, DrvSprites);
			} else {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, sx, sy, Colour, 3, 0, 0, DrvSprites);
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, sx - 256, sy, Colour, 3, 0, 0, DrvSprites);
			}
		} else {
			if (yFlip) {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, sx, sy, Colour, 3, 0, 0, DrvSprites);
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, sx - 256, sy, Colour, 3, 0, 0, DrvSprites);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, sy, Colour, 3, 0, 0, DrvSprites);
				Render16x16Tile_Mask_Clip(pTransDraw, Code, sx - 256, sy, Colour, 3, 0, 0, DrvSprites);
			}
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	if (nBurnLayer & 0x01) DrvRenderBgLayer(0);
	if (nSpriteEnable & 0x02) DrawSprites();
	if (nBurnLayer & 0x04) DrvRenderBgLayer(1);
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nCyclesTotal[2] = { 2000000 / 58, 1500000 / 58 };
	INT32 nInterleave = 100;
	
	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	M6809NewFrame();
	M6502NewFrame();
	
	M6809Open(0);
	M6502Open(0);
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;
		
		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		BurnTimerUpdate(nNext);
		if (i == (nInterleave - 1)) M6809SetIRQLine(0x20, M6809_IRQSTATUS_AUTO);
		
		nCurrentCPU = 1;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		BurnTimerUpdateYM3526(nNext);
	}
	
	BurnTimerEndFrame(nCyclesTotal[0]);
	BurnTimerEndFrameYM3526(nCyclesTotal[1]);
	
	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		BurnYM3526Update(pBurnSoundOut, nBurnSoundLen);
	}
	
	M6809Close();
	M6502Close();
	
	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029719;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		M6809Scan(nAction);
		M6502Scan(nAction);
		
		BurnYM2203Scan(nAction, pnMin);
		BurnYM3526Scan(nAction, pnMin);
	
		SCAN_VAR(I8751Return);
		SCAN_VAR(CurrentPtr);
		SCAN_VAR(CurrentTable);
		SCAN_VAR(InMath);
		SCAN_VAR(MathParam);
		SCAN_VAR(DrvSoundLatch);
	}

	return 0;
}

struct BurnDriver BurnDrvSidepckt = {
	"sidepckt", NULL, NULL, NULL, "1986",
	"Side Pocket (World)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_PREFIX_DATAEAST, GBF_SPORTSMISC, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	SidepcktInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSidepcktj = {
	"sidepcktj", "sidepckt", NULL, NULL, "1986",
	"Side Pocket (Japan)\0", NULL, "Data East Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 3, HARDWARE_PREFIX_DATAEAST, GBF_SPORTSMISC, 0,
	NULL, DrvjRomInfo, DrvjRomName, NULL, NULL, DrvInputInfo, DrvjDIPInfo,
	SidepcktjInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSidepcktb = {
	"sidepcktb", "sidepckt", NULL, NULL, "1986",
	"Side Pocket (bootleg set 1)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_PREFIX_DATAEAST, GBF_SPORTSMISC, 0,
	NULL, DrvbRomInfo, DrvbRomName, NULL, NULL, DrvInputInfo, DrvbDIPInfo,
	SidepcktbInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};

struct BurnDriver BurnDrvSidepcktb2 = {
	"sidepcktb2", "sidepckt", NULL, NULL, "1986",
	"Side Pocket (bootleg set 2)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 3, HARDWARE_PREFIX_DATAEAST, GBF_SPORTSMISC, 0,
	NULL, Drvb2RomInfo, Drvb2RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	SidepcktbInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x100, 256, 224, 4, 3
};
