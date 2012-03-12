#include "tiles_generic.h"
#include "burn_ym2203.h"
#include "msm5205.h"

static UINT8 Wc90b1InputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 Wc90b1InputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 Wc90b1Dip[2]        = {0, 0};
static UINT8 Wc90b1Input[2]      = {0x00, 0x00};
static UINT8 Wc90b1Reset         = 0;

static UINT8 *Mem                = NULL;
static UINT8 *MemEnd             = NULL;
static UINT8 *RamStart           = NULL;
static UINT8 *RamEnd             = NULL;
static UINT8 *Wc90b1Z80Rom1      = NULL;
static UINT8 *Wc90b1Z80Rom2      = NULL;
static UINT8 *Wc90b1Z80Rom3      = NULL;
static UINT8 *Wc90b1Z80Ram1      = NULL;
static UINT8 *Wc90b1Z80Ram2      = NULL;
static UINT8 *Wc90b1Z80Ram3      = NULL;
static UINT8 *Wc90b1FgVideoRam   = NULL;
static UINT8 *Wc90b1BgVideoRam   = NULL;
static UINT8 *Wc90b1TextVideoRam = NULL;
static UINT8 *Wc90b1SpriteRam    = NULL;
static UINT8 *Wc90b1PaletteRam   = NULL;
static UINT8 *Wc90b1SharedRam    = NULL;
static UINT32 *Wc90b1Palette     = NULL;
static UINT8 *Wc90b1CharTiles    = NULL;
static UINT8 *Wc90b1Tiles        = NULL;
static UINT8 *Wc90b1Sprites      = NULL;
static UINT8 *Wc90b1TempGfx      = NULL;

static UINT8 Wc90b1Scroll0Y;
static UINT8 Wc90b1Scroll0X;
static UINT8 Wc90b1Scroll1Y;
static UINT8 Wc90b1Scroll1X;
static UINT8 Wc90b1ScrollXLo;

static UINT8 Wc90b1SoundLatch = 0;
static INT32 Wc90b1MSM5205Next;

static INT32 nCyclesDone[3], nCyclesTotal[3];
static INT32 nCyclesSegment;

static struct BurnInputInfo Wc90b1InputList[] = {
	{"Coin 1"            , BIT_DIGITAL  , Wc90b1InputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , Wc90b1InputPort0 + 6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , Wc90b1InputPort1 + 7, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , Wc90b1InputPort1 + 6, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , Wc90b1InputPort0 + 3, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , Wc90b1InputPort0 + 2, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , Wc90b1InputPort0 + 1, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , Wc90b1InputPort0 + 0, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , Wc90b1InputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , Wc90b1InputPort0 + 5, "p1 fire 2" },

	{"P2 Up"             , BIT_DIGITAL  , Wc90b1InputPort1 + 3, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , Wc90b1InputPort1 + 2, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , Wc90b1InputPort1 + 1, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , Wc90b1InputPort1 + 0, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , Wc90b1InputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , Wc90b1InputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &Wc90b1Reset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, Wc90b1Dip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, Wc90b1Dip + 1       , "dip"       },
};

STDINPUTINFO(Wc90b1)

inline static void Wc90b1ClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

inline static void Wc90b1MakeInputs()
{
	Wc90b1Input[0] = Wc90b1Input[1] = 0x00;

	for (INT32 i = 0; i < 8; i++) {
		Wc90b1Input[0] |= (Wc90b1InputPort0[i] & 1) << i;
		Wc90b1Input[1] |= (Wc90b1InputPort1[i] & 1) << i;
	}
	
	Wc90b1ClearOpposites(&Wc90b1Input[0]);
	Wc90b1ClearOpposites(&Wc90b1Input[1]);
}

static struct BurnDIPInfo Wc90b1DIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0x7f, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 16  , "Coinage"                },
	{0x11, 0x01, 0x0f, 0x00, "10 Coins 1 Credit"      },
	{0x11, 0x01, 0x0f, 0x08, " 9 Coins 1 Credit"      },
	{0x11, 0x01, 0x0f, 0x04, " 8 Coins 1 Credit"      },
	{0x11, 0x01, 0x0f, 0x0c, " 7 Coins 1 Credit"      },
	{0x11, 0x01, 0x0f, 0x02, " 6 Coins 1 Credit"      },
	{0x11, 0x01, 0x0f, 0x0a, " 5 Coins 1 Credit"      },
	{0x11, 0x01, 0x0f, 0x06, " 4 Coins 1 Credit"      },
	{0x11, 0x01, 0x0f, 0x0e, " 3 Coins 1 Credit"      },
	{0x11, 0x01, 0x0f, 0x01, " 2 Coins 3 Credits"     },
	{0x11, 0x01, 0x0f, 0x09, " 2 Coins 1 Credit"      },
	{0x11, 0x01, 0x0f, 0x0f, " 1 Coin  1 Credit"      },
	{0x11, 0x01, 0x0f, 0x07, " 1 Coin  2 Credits"     },
	{0x11, 0x01, 0x0f, 0x0b, " 1 Coin  3 Credits"     },
	{0x11, 0x01, 0x0f, 0x03, " 1 Coin  4 Credits"     },
	{0x11, 0x01, 0x0f, 0x0d, " 1 Coin  5 Credits"     },
	{0x11, 0x01, 0x0f, 0x05, " 1 Coin  6 Credits"     },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x11, 0x01, 0x30, 0x30, "Easy"                   },
	{0x11, 0x01, 0x30, 0x10, "Normal"                 },
	{0x11, 0x01, 0x30, 0x20, "Hard"                   },
	{0x11, 0x01, 0x30, 0x00, "Hardest"                },

	{0   , 0xfe, 0   , 2   , "Countdown Speed"        },
	{0x11, 0x01, 0x40, 0x40, "Normal"                 },
	{0x11, 0x01, 0x40, 0x00, "Fast"                   },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x11, 0x01, 0x80, 0x00, "Off"                    },
	{0x11, 0x01, 0x80, 0x80, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "1 Player Game Time"     },
	{0x12, 0x01, 0x03, 0x01, "1:00"                   },
	{0x12, 0x01, 0x03, 0x02, "1:30"                   },
	{0x12, 0x01, 0x03, 0x03, "2:00"                   },
	{0x12, 0x01, 0x03, 0x00, "2:30"                   },

	{0   , 0xfe, 0   , 8   , "2 Players Game Time"    },
	{0x12, 0x01, 0x1c, 0x0c, "1:00"                   },
	{0x12, 0x01, 0x1c, 0x14, "1:30"                   },
	{0x12, 0x01, 0x1c, 0x04, "2:00"                   },
	{0x12, 0x01, 0x1c, 0x18, "2:30"                   },
	{0x12, 0x01, 0x1c, 0x1c, "3:00"                   },
	{0x12, 0x01, 0x1c, 0x08, "3:30"                   },
	{0x12, 0x01, 0x1c, 0x10, "4:00"                   },
	{0x12, 0x01, 0x1c, 0x00, "5:00"                   },

	{0   , 0xfe, 0   , 2   , "Language"               },
	{0x12, 0x01, 0x80, 0x00, "English"                },
	{0x12, 0x01, 0x80, 0x80, "Japanese"               },
};

STDDIPINFO(Wc90b1)

static struct BurnRomInfo Wc90b1RomDesc[] = {
	{ "a02.bin",                0x10000, 0x192a03dd, BRF_ESS | BRF_PRG },		//  0	Z80 #1 Program Code
	{ "a03.bin",                0x10000, 0xf54ff17a, BRF_ESS | BRF_PRG },		//  1	Z80 #1 Program Code

	{ "a04.bin",                0x10000, 0x3d535e2f, BRF_ESS | BRF_PRG },		//  2	Z80 #2 Program Code
	{ "a05.bin",                0x10000, 0x9e421c4b, BRF_ESS | BRF_PRG },		//  3	Z80 #2 Program Code

	{ "a01.bin",                0x10000, 0x3d317622, BRF_ESS | BRF_PRG },		//  4	Z80 #3 Program Code

	{ "a06.bin",                0x04000, 0x3b5387b7, BRF_GRA },					//  5	Characters
	{ "a08.bin",                0x04000, 0xc622a5a3, BRF_GRA },					//  6	Characters
	{ "a10.bin",                0x04000, 0x0923d9f6, BRF_GRA },					//  7	Characters
	{ "a20.bin",                0x04000, 0xb8dec83e, BRF_GRA },					//  8	Characters
	{ "a07.bin",                0x20000, 0x38c31817, BRF_GRA },					//  9	Fg Tiles
	{ "a09.bin",                0x20000, 0x32e39e29, BRF_GRA },					//  10	Fg Tiles
	{ "a11.bin",                0x20000, 0x5ccec796, BRF_GRA },					//  11	Bg Tiles
	{ "a21.bin",                0x20000, 0x0c54a091, BRF_GRA },					//  12	Bg Tiles
	{ "146_a12.bin",            0x10000, 0xd5a60096, BRF_GRA },					//  13	Sprites
	{ "147_a13.bin",            0x10000, 0x36bbf467, BRF_GRA },					//  14	Sprites
	{ "148_a14.bin",            0x10000, 0x26371c18, BRF_GRA },					//  15	Sprites
	{ "149_a15.bin",            0x10000, 0x75aa9b86, BRF_GRA },					//  16	Sprites
	{ "150_a16.bin",            0x10000, 0x0da825f9, BRF_GRA },					//  17	Sprites
	{ "151_a17.bin",            0x10000, 0x228429d8, BRF_GRA },					//  18	Sprites
	{ "152_a18.bin",            0x10000, 0x516b6c09, BRF_GRA },					//  19	Sprites
	{ "153_a19.bin",            0x10000, 0xf36390a9, BRF_GRA },					//  20	Sprites

	{ "el_ic39_gal16v8_0.bin",  0x00117, 0x00000000, BRF_OPT | BRF_NODUMP },	//  21	PLDs
	{ "el_ic44_gal16v8_1.bin",  0x00117, 0x00000000, BRF_OPT | BRF_NODUMP },	//  22	PLDs
	{ "el_ic54_gal16v8_2.bin",  0x00117, 0x00000000, BRF_OPT | BRF_NODUMP },	//  23	PLDs
	{ "el_ic100_gal16v8_3.bin", 0x00117, 0x00000000, BRF_OPT | BRF_NODUMP },	//  24	PLDs
	{ "el_ic143_gal16v8_4.bin", 0x00117, 0x00000000, BRF_OPT | BRF_NODUMP },	//  25	PLDs
};

STD_ROM_PICK(Wc90b1)
STD_ROM_FN(Wc90b1)

static struct BurnRomInfo Wc90b2RomDesc[] = {
	{ "a02",                    0x10000, 0x1e6e94c9, BRF_ESS | BRF_PRG },		//  0	Z80 #1 Program Code
	{ "a03.bin",                0x10000, 0xf54ff17a, BRF_ESS | BRF_PRG },		//  1	Z80 #1 Program Code

	{ "a04.bin",                0x10000, 0x3d535e2f, BRF_ESS | BRF_PRG },		//  2	Z80 #2 Program Code
	{ "a05.bin",                0x10000, 0x9e421c4b, BRF_ESS | BRF_PRG },		//  3	Z80 #2 Program Code

	{ "a01.bin",                0x10000, 0x3d317622, BRF_ESS | BRF_PRG },		//  4	Z80 #3 Program Code

	{ "a06",                    0x08000, 0x0c054481, BRF_GRA },					//  5	Characters
	{ "a08",                    0x08000, 0xebb3eb48, BRF_GRA },					//  6	Characters
	{ "a10",                    0x08000, 0xc0232af8, BRF_GRA },					//  7	Characters
	{ "a20",                    0x08000, 0xa36e17fb, BRF_GRA },					//  8	Characters
	{ "a07.bin",                0x20000, 0x38c31817, BRF_GRA },					//  9	Fg Tiles
	{ "a09.bin",                0x20000, 0x32e39e29, BRF_GRA },					//  10	Fg Tiles
	{ "a11.bin",                0x20000, 0x5ccec796, BRF_GRA },					//  11	Bg Tiles
	{ "a21.bin",                0x20000, 0x0c54a091, BRF_GRA },					//  12	Bg Tiles
	{ "146_a12.bin",            0x10000, 0xd5a60096, BRF_GRA },					//  13	Sprites
	{ "147_a13",                0x10000, 0x5b16fd48, BRF_GRA },					//  14	Sprites
	{ "148_a14.bin",            0x10000, 0x26371c18, BRF_GRA },					//  15	Sprites
	{ "149_a15",                0x10000, 0xb2423962, BRF_GRA },					//  16	Sprites
	{ "150_a16.bin",            0x10000, 0x0da825f9, BRF_GRA },					//  17	Sprites
	{ "151_a17",                0x10000, 0xaf98778e, BRF_GRA },					//  18	Sprites
	{ "152_a18.bin",            0x10000, 0x516b6c09, BRF_GRA },					//  19	Sprites
	{ "153_a19",                0x10000, 0x8caa2745, BRF_GRA },					//  20	Sprites

	{ "el_ic39_gal16v8_0.bin",  0x00117, 0x00000000, BRF_OPT | BRF_NODUMP },	//  21	PLDs
	{ "el_ic44_gal16v8_1.bin",  0x00117, 0x00000000, BRF_OPT | BRF_NODUMP },	//  22	PLDs
	{ "el_ic54_gal16v8_2.bin",  0x00117, 0x00000000, BRF_OPT | BRF_NODUMP },	//  23	PLDs
	{ "el_ic100_gal16v8_3.bin", 0x00117, 0x00000000, BRF_OPT | BRF_NODUMP },	//  24	PLDs
	{ "el_ic143_gal16v8_4.bin", 0x00117, 0x00000000, BRF_OPT | BRF_NODUMP },	//  25	PLDs
};

STD_ROM_PICK(Wc90b2)
STD_ROM_FN(Wc90b2)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Wc90b1Z80Rom1            = Next; Next += 0x20000;
	Wc90b1Z80Rom2            = Next; Next += 0x20000;
	Wc90b1Z80Rom3            = Next; Next += 0x10000;

	RamStart                 = Next;

	Wc90b1Z80Ram1            = Next; Next += 0x04000;
	Wc90b1Z80Ram2            = Next; Next += 0x01800;
	Wc90b1Z80Ram3            = Next; Next += 0x00800;
	Wc90b1FgVideoRam         = Next; Next += 0x01000;
	Wc90b1BgVideoRam         = Next; Next += 0x01000;
	Wc90b1TextVideoRam       = Next; Next += 0x01000;
	Wc90b1SpriteRam          = Next; Next += 0x00800;
	Wc90b1PaletteRam         = Next; Next += 0x00800;
	Wc90b1SharedRam          = Next; Next += 0x00400;
	
	RamEnd                   = Next;

	Wc90b1CharTiles          = Next; Next += (2048 *  8 *  8);
	Wc90b1Tiles              = Next; Next += (4096 * 16 * 16);
	Wc90b1Sprites            = Next; Next += (4096 * 16 * 16);
	Wc90b1Palette            = (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);

	MemEnd                   = Next;

	return 0;
}

static INT32 Wc90b1DoReset()
{
	Wc90b1Scroll0X = Wc90b1Scroll0Y = 0;
	Wc90b1Scroll1X = Wc90b1Scroll1Y = 0;
	Wc90b1ScrollXLo = 0;

	Wc90b1SoundLatch = 0;
	Wc90b1MSM5205Next = 0;

	for (INT32 i = 0; i < 3; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}
	
	BurnYM2203Reset();
	MSM5205Reset();
	
//	HiscoreReset();

	return 0;
}

UINT8 __fastcall Wc90b1Read1(UINT16 a)
{
	switch (a) {
		case 0xfd00: {
			return 0xff - Wc90b1Input[0];
		}
		
		case 0xfd02: {
			return 0xff - Wc90b1Input[1];
		}
		
		case 0xfd06: {
			return Wc90b1Dip[0];
		}

		case 0xfd08: {
			return Wc90b1Dip[1];
		}
		
		case 0xfd0c: {
			// ???
			return 0;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall Wc90b1Write1(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xfc00: {
			INT32 BankAddress;

			BankAddress = 0x10000 + ((d & 0xf8) << 8);
			ZetMapArea(0xf000, 0xf7ff, 0, Wc90b1Z80Rom1 + BankAddress);
			ZetMapArea(0xf000, 0xf7ff, 2, Wc90b1Z80Rom1 + BankAddress);
			return;
		}
		
		case 0xfd00: {
			Wc90b1SoundLatch = d;
			ZetClose();
			ZetOpen(2);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			ZetClose();
			ZetOpen(0);
			return;
		}
		
		case 0xfd04: {
			Wc90b1Scroll0Y = d;
			return;
		}
		
		case 0xfd06: {
			Wc90b1Scroll0X = d;
			return;
		}
		
		case 0xfd08: {
			Wc90b1Scroll1Y = d;
			return;
		}
		
		case 0xfd0a: {
			Wc90b1Scroll1X = d;
			return;
		}
		
		case 0xfd0e: {
			Wc90b1ScrollXLo = d;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall Wc90b1Read2(UINT16 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall Wc90b1Write2(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xfc00: {
			INT32 BankAddress;

			BankAddress = 0x10000 + ((d & 0xf8) << 8);
			ZetMapArea(0xf000, 0xf7ff, 0, Wc90b1Z80Rom2 + BankAddress);
			ZetMapArea(0xf000, 0xf7ff, 2, Wc90b1Z80Rom2 + BankAddress);
			return;
		}
		
		case 0xfd0c: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Write => %04X, %02X\n"), a, d);
		}
	}
}

UINT8 __fastcall Wc90b1Read3(UINT16 a)
{
	switch (a) {
		case 0xe800: {
			return BurnYM2203Read(0, 0);
		}
		
		case 0xf800: {
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return Wc90b1SoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #3 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall Wc90b1Write3(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xe000: {
			INT32 BankAddress;

			BankAddress = 0x8000 + ((d & 0x01) * 0x4000);
			ZetMapArea(0x8000, 0xbfff, 0, Wc90b1Z80Rom3 + BankAddress);
			ZetMapArea(0x8000, 0xbfff, 2, Wc90b1Z80Rom3 + BankAddress);
			
			MSM5205ResetWrite(0, d & 0x08);
			return;
		}
		
		case 0xe400: {
			Wc90b1MSM5205Next = d;
			return;
		}
		
		case 0xe800: {
			BurnYM2203Write(0, 0, d);
			return;
		}
		
		case 0xe801: {
			BurnYM2203Write(0, 1, d);
			return;
		}
		
		case 0xec00:
		case 0xec01: {
			// ???
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #3 Write => %04X, %02X\n"), a, d);
		}
	}
}

inline static INT32 Wc90b1SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 5000000;
}

inline static double Wc90b1GetTime()
{
	return (double)ZetTotalCycles() / 5000000;
}

static void Wc90b1MSM5205Vck0()
{
	static INT32 Toggle = 0;
	
	Toggle ^= 1;
	
	if (Toggle) {
		MSM5205DataWrite(0, (Wc90b1MSM5205Next & 0xf0) >> 4);
		ZetNmi();
	} else {
		MSM5205DataWrite(0, Wc90b1MSM5205Next & 0x0f);
	}
}

static INT32 CharPlaneOffsets[4]   = { 0, 0x20000, 0x40000, 0x60000 };
static INT32 CharXOffsets[8]       = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 CharYOffsets[8]       = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 TilePlaneOffsets[4]   = { 0, 0x100000, 0x200000, 0x300000 };
static INT32 TileXOffsets[16]      = { 0, 1, 2, 3, 4, 5, 6, 7, 0x8000, 0x8001, 0x8002, 0x8003, 0x8004, 0x8005, 0x8006, 0x8007 };
static INT32 TileYOffsets[16]      = { 0, 8, 16, 24, 32, 40, 48, 56, 0x4000, 0x4008, 0x4010, 0x4018, 0x4020, 0x4028, 0x4030, 0x4038 };
static INT32 SpritePlaneOffsets[4] = { 0x300000, 0x200000, 0x100000, 0 };
static INT32 SpriteXOffsets[16]    = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
static INT32 SpriteYOffsets[16]    = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };

static INT32 Wc90b1Init()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	Wc90b1TempGfx = (UINT8*)BurnMalloc(0x80000);
	if (Wc90b1TempGfx == NULL) return 1;

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(Wc90b1Z80Rom1 + 0x00000,  0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1Z80Rom1 + 0x10000,  1, 1); if (nRet != 0) return 1;

	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(Wc90b1Z80Rom2 + 0x00000,  2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1Z80Rom2 + 0x10000,  3, 1); if (nRet != 0) return 1;

	// Load Z80 #3 Program Rom
	nRet = BurnLoadRom(Wc90b1Z80Rom3 + 0x00000,  4, 1); if (nRet != 0) return 1;

	// Load and Decode Char Tile Roms
	memset(Wc90b1TempGfx, 0, 0x80000);
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x04000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x08000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x0c000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 4, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x40, Wc90b1TempGfx, Wc90b1CharTiles);

	// Load and Decode Tile Roms
	memset(Wc90b1TempGfx, 0, 0x80000);
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x20000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x40000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x60000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x00000, Wc90b1Tiles + ( 0 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x02000, Wc90b1Tiles + ( 1 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x04000, Wc90b1Tiles + ( 2 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x06000, Wc90b1Tiles + ( 3 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x08000, Wc90b1Tiles + ( 4 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x0a000, Wc90b1Tiles + ( 5 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x0c000, Wc90b1Tiles + ( 6 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x0e000, Wc90b1Tiles + ( 7 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x10000, Wc90b1Tiles + ( 8 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x12000, Wc90b1Tiles + ( 9 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x14000, Wc90b1Tiles + (10 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x16000, Wc90b1Tiles + (11 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x18000, Wc90b1Tiles + (12 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x1a000, Wc90b1Tiles + (13 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x1c000, Wc90b1Tiles + (14 * 256 * 16 * 16));
	GfxDecode(256, 4, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x40, Wc90b1TempGfx + 0x1e000, Wc90b1Tiles + (15 * 256 * 16 * 16));
	
	// Load and Decode Sprite Roms
	memset(Wc90b1TempGfx, 0, 0x80000);
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x00000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x10000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x20000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x30000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x40000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x50000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x60000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Wc90b1TempGfx + 0x70000, 20, 1); if (nRet != 0) return 1;
	for (INT32 i = 0; i < 0x80000; i++) {
		Wc90b1TempGfx[i] ^= 0xff;
	}
	GfxDecode(4096, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, Wc90b1TempGfx, Wc90b1Sprites);

	BurnFree(Wc90b1TempGfx);

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(Wc90b1Read1);
	ZetSetWriteHandler(Wc90b1Write1);
	ZetMapArea(0x0000, 0x7fff, 0, Wc90b1Z80Rom1          );
	ZetMapArea(0x0000, 0x7fff, 2, Wc90b1Z80Rom1          );
	ZetMapArea(0x8000, 0x9fff, 0, Wc90b1Z80Ram1 + 0x00000);
	ZetMapArea(0x8000, 0x9fff, 1, Wc90b1Z80Ram1 + 0x00000);
	ZetMapArea(0x8000, 0x9fff, 2, Wc90b1Z80Ram1 + 0x00000);
	ZetMapArea(0xa000, 0xafff, 0, Wc90b1FgVideoRam       );
	ZetMapArea(0xa000, 0xafff, 1, Wc90b1FgVideoRam       );
	ZetMapArea(0xa000, 0xafff, 2, Wc90b1FgVideoRam       );
	ZetMapArea(0xc000, 0xcfff, 0, Wc90b1BgVideoRam       );
	ZetMapArea(0xc000, 0xcfff, 1, Wc90b1BgVideoRam       );
	ZetMapArea(0xc000, 0xcfff, 2, Wc90b1BgVideoRam       );
	ZetMapArea(0xd000, 0xdfff, 0, Wc90b1Z80Ram1 + 0x03000);
	ZetMapArea(0xd000, 0xdfff, 1, Wc90b1Z80Ram1 + 0x03000);
	ZetMapArea(0xd000, 0xdfff, 2, Wc90b1Z80Ram1 + 0x03000);
	ZetMapArea(0xe000, 0xefff, 0, Wc90b1TextVideoRam     );
	ZetMapArea(0xe000, 0xefff, 1, Wc90b1TextVideoRam     );
	ZetMapArea(0xe000, 0xefff, 2, Wc90b1TextVideoRam     );
	ZetMapArea(0xf000, 0xf7ff, 0, Wc90b1Z80Rom1 + 0x10000);
	ZetMapArea(0xf000, 0xf7ff, 2, Wc90b1Z80Rom1 + 0x10000);
	ZetMapArea(0xf800, 0xfbff, 0, Wc90b1SharedRam        );
	ZetMapArea(0xf800, 0xfbff, 1, Wc90b1SharedRam        );
	ZetMapArea(0xf800, 0xfbff, 2, Wc90b1SharedRam        );
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetSetReadHandler(Wc90b1Read2);
	ZetSetWriteHandler(Wc90b1Write2);
	ZetMapArea(0x0000, 0xbfff, 0, Wc90b1Z80Rom2          );
	ZetMapArea(0x0000, 0xbfff, 2, Wc90b1Z80Rom2          );
	ZetMapArea(0xc000, 0xcfff, 0, Wc90b1Z80Ram2 + 0x00000);
	ZetMapArea(0xc000, 0xcfff, 1, Wc90b1Z80Ram2 + 0x00000);
	ZetMapArea(0xc000, 0xcfff, 2, Wc90b1Z80Ram2 + 0x00000);
	ZetMapArea(0xd000, 0xd7ff, 0, Wc90b1SpriteRam        );
	ZetMapArea(0xd000, 0xd7ff, 1, Wc90b1SpriteRam        );
	ZetMapArea(0xd000, 0xd7ff, 2, Wc90b1SpriteRam        );
	ZetMapArea(0xd800, 0xdfff, 0, Wc90b1Z80Ram2 + 0x01000);
	ZetMapArea(0xd800, 0xdfff, 1, Wc90b1Z80Ram2 + 0x01000);
	ZetMapArea(0xd800, 0xdfff, 2, Wc90b1Z80Ram2 + 0x01000);
	ZetMapArea(0xe000, 0xe7ff, 0, Wc90b1PaletteRam       );
	ZetMapArea(0xe000, 0xe7ff, 1, Wc90b1PaletteRam       );
	ZetMapArea(0xe000, 0xe7ff, 2, Wc90b1PaletteRam       );
	ZetMapArea(0xe800, 0xefff, 0, Wc90b1Z80Rom2 + 0x0e800);
	ZetMapArea(0xe800, 0xefff, 2, Wc90b1Z80Rom2 + 0x0e800);
	ZetMapArea(0xf000, 0xf7ff, 0, Wc90b1Z80Rom2 + 0x10000);
	ZetMapArea(0xf000, 0xf7ff, 2, Wc90b1Z80Rom2 + 0x10000);
	ZetMapArea(0xf800, 0xfbff, 0, Wc90b1SharedRam        );
	ZetMapArea(0xf800, 0xfbff, 1, Wc90b1SharedRam        );
	ZetMapArea(0xf800, 0xfbff, 2, Wc90b1SharedRam        );
	ZetMemEnd();
	ZetClose();

	ZetInit(2);
	ZetOpen(2);
	ZetSetReadHandler(Wc90b1Read3);
	ZetSetWriteHandler(Wc90b1Write3);
	ZetMapArea(0x0000, 0x7fff, 0, Wc90b1Z80Rom3          );
	ZetMapArea(0x0000, 0x7fff, 2, Wc90b1Z80Rom3          );
	ZetMapArea(0x8000, 0xbfff, 0, Wc90b1Z80Rom3 + 0x8000 );
	ZetMapArea(0x8000, 0xbfff, 2, Wc90b1Z80Rom3 + 0x8000 );
	ZetMapArea(0xf000, 0xf7ff, 0, Wc90b1Z80Ram3          );
	ZetMapArea(0xf000, 0xf7ff, 1, Wc90b1Z80Ram3          );
	ZetMapArea(0xf000, 0xf7ff, 2, Wc90b1Z80Ram3          );
	ZetMemEnd();
	ZetClose();

	GenericTilesInit();

	BurnYM2203Init(1, 1250000, NULL, Wc90b1SynchroniseStream, Wc90b1GetTime, 0);
	BurnTimerAttachZet(5000000);
	
	MSM5205Init(0, Wc90b1SynchroniseStream, 384000, Wc90b1MSM5205Vck0, MSM5205_S96_4B, 100, 1);
	
	Wc90b1DoReset();

	return 0;
}

static INT32 Wc90b1Exit()
{
	ZetExit();
	GenericTilesExit();
	BurnYM2203Exit();
	MSM5205Exit();

	BurnFree(Mem);
	
	Wc90b1Scroll0X = Wc90b1Scroll0Y = 0;
	Wc90b1Scroll1X = Wc90b1Scroll1Y = 0;
	Wc90b1ScrollXLo = 0;
	Wc90b1SoundLatch = 0;
	Wc90b1MSM5205Next = 0;
	
	return 0;
}

static void Wc90b1RenderBgLayer()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Attr = Wc90b1BgVideoRam[TileIndex];
			Code = Wc90b1BgVideoRam[TileIndex + 0x800];
			Colour = Attr >> 4;
			
			Code |= ((Attr & 3) + ((Attr >> 1) & 4)) * 256;
			Code |= 2048;
			
			x = 16 * mx;
			y = 16 * my;

			x -= 8 * (Wc90b1Scroll1X & 0x7f) + 256 - 4 + (Wc90b1ScrollXLo & 0x07);
			y -= Wc90b1Scroll1Y + 1 + ((Wc90b1Scroll1X & 0x80) ? 256 : 0);

			x &= 0x3ff;
			y &= 0x1ff;

			y -= 16;
			if (x > 968) x -= 1024;

			if (x > 15 && x < 240 && y > 15 && y < 208) {
				Render16x16Tile(pTransDraw, Code, x, y, Colour, 4, 768, Wc90b1Tiles);
			} else {
				Render16x16Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 768, Wc90b1Tiles);
			}

			TileIndex++;
		}
	}
}

static void Wc90b1RenderFgLayer()
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Attr = Wc90b1FgVideoRam[TileIndex];
			Code = Wc90b1FgVideoRam[TileIndex + 0x800];
			Colour = Attr >> 4;
			
			Code |= ((Attr & 3) + ((Attr >> 1) & 4)) * 256;
			
			x = 16 * mx;
			y = 16 * my;

			x -= 8 * (Wc90b1Scroll0X & 0x7f) + 256 - 6 + ((Wc90b1ScrollXLo & 0x38) >> 3);
			y -= Wc90b1Scroll0Y + 1 + ((Wc90b1Scroll0X & 0x80) ? 256 : 0);

			x &= 0x3ff;
			y &= 0x1ff;

			y -= 16;
			if (x > 968) x -= 1024;

			if (x > 15 && x < 240 && y > 15 && y < 208) {
				Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 15, 512, Wc90b1Tiles);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 15, 512, Wc90b1Tiles);
			}

			TileIndex++;
		}
	}
}

static void Wc90b1RenderCharLayer()
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = Wc90b1TextVideoRam[TileIndex + 0x800] + ((Wc90b1TextVideoRam[TileIndex] & 0x07) << 8);
			Colour = Wc90b1TextVideoRam[TileIndex] >> 4;

			x = 8 * mx;
			y = 8 * my;

			x &= 0x1ff;
			y &= 0x0ff;

			y -= 16;

			if (x > 7 && x < 248 && y > 7 && y < 216) {
				Render8x8Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 15, 256, Wc90b1CharTiles);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 15, 256, Wc90b1CharTiles);
			}

			TileIndex++;
		}
	}
}

static void Wc90b1RenderSpriteLayer(INT32 Priority)
{
	INT32 Offset, sx, sy;
	
	for (Offset = 0x800 - 8; Offset >= 0; Offset -= 8) {
		if ((~(Wc90b1SpriteRam[Offset + 3] >> 7) & 1) == Priority) {
			INT32 Code = (Wc90b1SpriteRam[Offset + 3] & 0x3f) << 4;
			INT32 Bank = Wc90b1SpriteRam[Offset + 0];
			INT32 Flags = Wc90b1SpriteRam[Offset + 4];
			INT32 xFlip = Bank & 0x01;
			INT32 yFlip = Bank & 0x02;
			
			Code += (Bank & 0xf0) >> 4;
			Code <<= 2;
			Code += (Bank & 0x0f) >> 2;
			
			sx = Wc90b1SpriteRam[Offset + 2];
			if (!(Wc90b1SpriteRam[Offset + 3] & 0x40)) sx -= 0x100;
			
			sy = 240 - Wc90b1SpriteRam[Offset + 1];
			
			sy -= 16;
			
			if (sx > 15 && sx < 240 && sy > 15 && sy < 208) {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_Mask_FlipXY(pTransDraw, Code, sx, sy, Flags >> 4, 4, 15, 0, Wc90b1Sprites);
					} else {
						Render16x16Tile_Mask_FlipX(pTransDraw, Code, sx, sy, Flags >> 4, 4, 15, 0, Wc90b1Sprites);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_Mask_FlipY(pTransDraw, Code, sx, sy, Flags >> 4, 4, 15, 0, Wc90b1Sprites);
					} else {
						Render16x16Tile_Mask(pTransDraw, Code, sx, sy, Flags >> 4, 4, 15, 0, Wc90b1Sprites);
					}
				}
			} else {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, sx, sy, Flags >> 4, 4, 15, 0, Wc90b1Sprites);
					} else {
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, sx, sy, Flags >> 4, 4, 15, 0, Wc90b1Sprites);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, sx, sy, Flags >> 4, 4, 15, 0, Wc90b1Sprites);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, sy, Flags >> 4, 4, 15, 0, Wc90b1Sprites);
					}
				}
			}
		}
	}
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour >> 0) & 0x0f;
	g = (nColour >> 4) & 0x0f;
	b = (nColour >> 8) & 0x0f;

	r = (r << 4) | r;
	g = (g << 4) | g;
	b = (b << 4) | b;

	return BurnHighCol(r, g, b, 0);
}

static INT32 Wc90b1CalcPalette()
{
	INT32 i;

	for (i = 0; i < 0x800; i++) {
		Wc90b1Palette[i / 2] = CalcCol(Wc90b1PaletteRam[i | 1] | (Wc90b1PaletteRam[i & ~1] << 8));
	}

	return 0;
}

static void Wc90b1Draw()
{
	BurnTransferClear();
	Wc90b1CalcPalette();
	Wc90b1RenderBgLayer();
	Wc90b1RenderFgLayer();
	Wc90b1RenderSpriteLayer(1);
	Wc90b1RenderCharLayer();
	Wc90b1RenderSpriteLayer(0);
	BurnTransferCopy(Wc90b1Palette);
}

static INT32 Wc90b1Frame()
{
	INT32 nInterleave = MSM5205CalcInterleave(0, 5000000);
	INT32 nVBlankIRQFire = (INT32)((double)242 / 260 * nInterleave);

	if (Wc90b1Reset) Wc90b1DoReset();

	Wc90b1MakeInputs();
	
	nCyclesTotal[0] = (INT32)((INT64)7159090 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = (INT32)((INT64)7159090 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[2] = (INT32)(double)(5000000 / 60);
	
	nCyclesDone[0] = nCyclesDone[1] = nCyclesDone[2] = 0;

	ZetNewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nNext, nCurrentCPU;

		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == nVBlankIRQFire) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		if (i == nVBlankIRQFire + 1) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		ZetClose();

		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == nVBlankIRQFire) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		if (i == nVBlankIRQFire + 1) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		ZetClose();
		
		nCurrentCPU = 2;
		ZetOpen(nCurrentCPU);
		BurnTimerUpdate(i * (nCyclesTotal[2] / nInterleave));
		MSM5205Update();
		ZetClose();
	}

	ZetOpen(2);
	BurnTimerEndFrame(nCyclesTotal[2]);
	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		MSM5205Render(0, pBurnSoundOut, nBurnSoundLen);
	}
	ZetClose();
	
	if (pBurnDraw) Wc90b1Draw();

	return 0;
}

static INT32 Wc90b1Scan(INT32 nAction,INT32 *pnMin)
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
		ZetScan(nAction);

		BurnYM2203Scan(nAction, pnMin);

		SCAN_VAR(Wc90b1SoundLatch);
		SCAN_VAR(Wc90b1MSM5205Next);
		SCAN_VAR(Wc90b1Input);
		SCAN_VAR(Wc90b1Dip);
		SCAN_VAR(Wc90b1Scroll0Y);
		SCAN_VAR(Wc90b1Scroll0X);
		SCAN_VAR(Wc90b1Scroll1Y);
		SCAN_VAR(Wc90b1Scroll1X);
		SCAN_VAR(Wc90b1ScrollXLo);
	}

	return 0;
}

struct BurnDriver BurnDrvWc90b1 = {
	"wc90b1", "wc90", NULL, NULL, "1989",
	"Euro League (Italian hack of Tecmo World Cup '90)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_SPORTSFOOTBALL, 0,
	NULL, Wc90b1RomInfo, Wc90b1RomName, NULL, NULL, Wc90b1InputInfo, Wc90b1DIPInfo,
	Wc90b1Init, Wc90b1Exit, Wc90b1Frame, NULL, Wc90b1Scan,
	NULL, 0x400, 256, 224, 4, 3
};

struct BurnDriver BurnDrvWc90b2 = {
	"wc90b2", "wc90", NULL, NULL, "1989",
	"World Cup '90 (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_PRE90S, GBF_SPORTSFOOTBALL, 0,
	NULL, Wc90b2RomInfo, Wc90b2RomName, NULL, NULL, Wc90b1InputInfo, Wc90b1DIPInfo,
	Wc90b1Init, Wc90b1Exit, Wc90b1Frame, NULL, Wc90b1Scan,
	NULL, 0x400, 256, 224, 4, 3
};
