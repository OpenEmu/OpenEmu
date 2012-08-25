#include "tiles_generic.h"
#include "m68000_intf.h"
#include "msm6295.h"

static UINT8 DrvInputPort0[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8]     = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[2]            = {0, 0};
static UINT8 DrvInput[2]          = {0x00, 0x00};
static UINT8 DrvReset             = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *Drv68KRom           = NULL;
static UINT8 *Drv68KRam           = NULL;
static UINT8 *DrvMSM6295ROMSrc    = NULL;
static UINT8 *DrvVRam0            = NULL;
static UINT8 *DrvVRam1            = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvPaletteRam       = NULL;
static UINT8 *DrvScroll0          = NULL;
static UINT8 *DrvScroll1          = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette         = NULL;

static UINT8 DrvEncrypted         = 0;
static UINT8 DrvIsWatrball        = 0;
static UINT8 BlmbyPotWheel        = 0;
static UINT32 DrvOkiBank          = 0;
static INT32 DrvToggle            = 0; 

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort1 + 6, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 7, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 2" },	
	
	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Drv)

static inline void DrvMakeInputs()
{
	DrvInput[0] = DrvInput[1] = 0xff;

	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] -= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] -= (DrvInputPort1[i] & 1) << i;
	}
}

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xdf, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x11, 0x01, 0x03, 0x02, "Easy"                   },
	{0x11, 0x01, 0x03, 0x03, "Normal"                 },
	{0x11, 0x01, 0x03, 0x01, "Hard"                   },
	{0x11, 0x01, 0x03, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Joysticks"              },
	{0x11, 0x01, 0x04, 0x00, "1"                      },
	{0x11, 0x01, 0x04, 0x04, "2"                      },
	
	{0   , 0xfe, 0   , 1   , "Controls"               },
	{0x11, 0x01, 0x18, 0x18, "Joystick"               },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x11, 0x01, 0x20, 0x20, "Off"                    },
	{0x11, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x11, 0x01, 0x80, 0x80, "Off"                    },
	{0x11, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2	
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Credit"       },
	{0x12, 0x01, 0x07, 0x00, "3 Coins 4 Credits"      },
	{0x12, 0x01, 0x07, 0x01, "2 Coins 3 Credits"      },
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0x07, 0x05, "1 Coin  3 Credits"      },
	{0x12, 0x01, 0x07, 0x04, "1 Coin  4 Credits"      },
	{0x12, 0x01, 0x07, 0x03, "1 Coin  5 Credits"      },
	{0x12, 0x01, 0x07, 0x02, "1 Coin  6 Credits"      },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x12, 0x01, 0x38, 0x10, "6 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x18, "5 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x20, "4 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x28, "3 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x30, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x08, "3 Coins 2 Credits"      },
	{0x12, 0x01, 0x38, 0x00, "4 Coins 3 Credits"      },
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credit"       },
	
	{0   , 0xfe, 0   , 2   , "Credits to Start"       },
	{0x12, 0x01, 0x40, 0x40, "1"                      },
	{0x12, 0x01, 0x40, 0x00, "2"                      },
		
	{0   , 0xfe, 0   , 2   , "Free Play"              },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo WatrballDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xdf, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },
	
	// Dip 1
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x11, 0x01, 0x03, 0x02, "Easy"                   },
	{0x11, 0x01, 0x03, 0x03, "Normal"                 },
	{0x11, 0x01, 0x03, 0x01, "Hard"                   },
	{0x11, 0x01, 0x03, 0x00, "Hardest"                },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x11, 0x01, 0x20, 0x20, "Off"                    },
	{0x11, 0x01, 0x20, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x11, 0x01, 0x80, 0x80, "Off"                    },
	{0x11, 0x01, 0x80, 0x00, "On"                     },
	
	// Dip 2	
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Credit"       },
	{0x12, 0x01, 0x07, 0x00, "3 Coins 4 Credits"      },
	{0x12, 0x01, 0x07, 0x01, "2 Coins 3 Credits"      },
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Credits"      },
	{0x12, 0x01, 0x07, 0x05, "1 Coin  3 Credits"      },
	{0x12, 0x01, 0x07, 0x04, "1 Coin  4 Credits"      },
	{0x12, 0x01, 0x07, 0x03, "1 Coin  5 Credits"      },
	{0x12, 0x01, 0x07, 0x02, "1 Coin  6 Credits"      },
	
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x12, 0x01, 0x38, 0x10, "6 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x18, "5 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x20, "4 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x28, "3 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x30, "2 Coins 1 Credit"       },
	{0x12, 0x01, 0x38, 0x08, "3 Coins 2 Credits"      },
	{0x12, 0x01, 0x38, 0x00, "4 Coins 3 Credits"      },
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credit"       },
};

STDDIPINFO(Watrball)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "bcrom4.bin",    0x80000, 0x06d490ba, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "bcrom6.bin",    0x80000, 0x33aca664, BRF_ESS | BRF_PRG }, //	 1
	
	{ "bc_rom7",       0x80000, 0xe55ca79b, BRF_GRA },	     //  2	Sprites
	{ "bc_rom8",       0x80000, 0xcdf38c96, BRF_GRA },	     //  3
	{ "bc_rom9",       0x80000, 0x0337ab3d, BRF_GRA },	     //  4
	{ "bc_rom10",      0x80000, 0x5458917e, BRF_GRA },	     //  5
	
	{ "bc_rom1",       0x80000, 0xac6f8ba1, BRF_SND },	     //  6	Samples
	{ "bc_rom2",       0x80000, 0xa4bc31bf, BRF_SND },	     //  7
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo DrvuRomDesc[] = {
	{ "bc_rom4",       0x80000, 0x76f054a2, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "bc_rom6",       0x80000, 0x2570b4c5, BRF_ESS | BRF_PRG }, //	 1
	
	{ "bc_rom7",       0x80000, 0xe55ca79b, BRF_GRA },	     //  2	Sprites
	{ "bc_rom8",       0x80000, 0xcdf38c96, BRF_GRA },	     //  3
	{ "bc_rom9",       0x80000, 0x0337ab3d, BRF_GRA },	     //  4
	{ "bc_rom10",      0x80000, 0x5458917e, BRF_GRA },	     //  5
	
	{ "bc_rom1",       0x80000, 0xac6f8ba1, BRF_SND },	     //  6	Samples
	{ "bc_rom2",       0x80000, 0xa4bc31bf, BRF_SND },	     //  7
};

STD_ROM_PICK(Drvu)
STD_ROM_FN(Drvu)

static struct BurnRomInfo WatrballRomDesc[] = {
	{ "rom4.bin",      0x20000, 0xbfbfa720, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "rom6.bin",      0x20000, 0xacff9b01, BRF_ESS | BRF_PRG }, //	 1
	
	{ "rom7.bin",      0x80000, 0xe7e5c311, BRF_GRA },	     //  2	Sprites
	{ "rom8.bin",      0x80000, 0xfd27ce6e, BRF_GRA },	     //  3
	{ "rom9.bin",      0x80000, 0x122cc0ad, BRF_GRA },	     //  4
	{ "rom10.bin",     0x80000, 0x22a2a706, BRF_GRA },	     //  5
	
	{ "rom1.bin",      0x80000, 0x7f88dee7, BRF_SND },	     //  6	Samples
};

STD_ROM_PICK(Watrball)
STD_ROM_FN(Watrball)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Drv68KRom              = Next; Next += 0x100000;
	MSM6295ROM             = Next; Next += 0x040000;
	DrvMSM6295ROMSrc       = Next; Next += 0x100000;

	RamStart               = Next;
	
	Drv68KRam              = Next; Next += 0x00e000;
	DrvVRam0               = Next; Next += 0x002000;
	DrvVRam1               = Next; Next += 0x002000;
	DrvSpriteRam           = Next; Next += 0x002000;
	DrvPaletteRam          = Next; Next += 0x000600;
	DrvScroll0             = Next; Next += 0x000004;
	DrvScroll1             = Next; Next += 0x000004;

	RamEnd                 = Next;

	DrvSprites             = Next; Next += 0x4000 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00300 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static void DrvDoOkiBank(UINT8 Bank)
{
	DrvOkiBank = Bank;
	memcpy(MSM6295ROM + 0x30000, DrvMSM6295ROMSrc + (0x10000 * DrvOkiBank), 0x10000);
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();
	
	MSM6295Reset(0);
	DrvDoOkiBank(0);
	
	BlmbyPotWheel = 0;
	DrvToggle = 0;
	
	return 0;
}

UINT8 __fastcall Blmbycar68KReadByte(UINT32 a)
{
	if (a >= 0x204000 && a <= 0x2045ff) {
		UINT16 *RAM = (UINT16*)DrvPaletteRam;
		return RAM[(a - 0x204000) >> 1] & 0xff;
	}
	
	if (a >= 0x204600 && a <= 0x207fff) {
		UINT16 *RAM = (UINT16*)Drv68KRam + (0x8000 / 2);
		return RAM[(a - 0x204600) >> 1] & 0xff;
	}
	
	switch (a) {
		case 0x700006: {
			return 0x00;
		}
		
		case 0x700009: {
			if (DrvIsWatrball) {
				DrvToggle ^= 0x08;
				return DrvToggle & 0xff;
			}
			return ((BlmbyPotWheel & 0x80) ? 0x04 : 0x00) | (rand() & 0x08);
		}
		
		case 0x70000f: {
			return MSM6295ReadStatus(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Blmbycar68KWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x100000 && a <= 0x103fff) {
		// ???
		return;
	}
	
	if (a >= 0x200000 && a <= 0x2005ff) {
		UINT16 *RAM = (UINT16*)DrvPaletteRam;
		RAM[(a - 0x200000) >> 1] = d;
		return;
	}
	
	if (a >= 0x200600 && a <= 0x203fff) {
		UINT16 *RAM = (UINT16*)Drv68KRam + (0x4000 / 2);
		RAM[(a - 0x200600) >> 1] = d;
		return;
	}
	
	switch (a) {
		case 0x70000a: {
			// nop
			return;
		}
		
		case 0x70000d: {
			DrvDoOkiBank(d & 0x0f);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Blmbycar68KReadWord(UINT32 a)
{
	if (a >= 0x204000 && a <= 0x2045ff) {
		UINT16 *RAM = (UINT16*)DrvPaletteRam;
		return BURN_ENDIAN_SWAP_INT16(RAM[(a - 0x204000) >> 1]);
	}
	
	if (a >= 0x204600 && a <= 0x207fff) {
		UINT16 *RAM = (UINT16*)Drv68KRam + (0x8000 / 2);
		return BURN_ENDIAN_SWAP_INT16(RAM[(a - 0x204600) >> 1]);
	}
	
	switch (a) {
		case 0x700000: {
			return (DrvDip[1] << 8) | DrvDip[0];
		}
		
		case 0x700002: {
			return (DrvInput[1] << 8) | DrvInput[0];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Blmbycar68KWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x108000 && a <= 0x10bfff) {
		// ???
		return;
	}
	
	if (a >= 0x200000 && a <= 0x2005ff) {
		UINT16 *RAM = (UINT16*)DrvPaletteRam;
		RAM[(a - 0x200000) >> 1] = BURN_ENDIAN_SWAP_INT16(d);
		return;
	}
	
	if (a >= 0x204000 && a <= 0x2045ff) {
		UINT16 *RAM = (UINT16*)DrvPaletteRam;
		RAM[(a - 0x204000) >> 1] = BURN_ENDIAN_SWAP_INT16(d);
		return;
	}
	
	if (a >= 0x204600 && a <= 0x207fff) {
		UINT16 *RAM = (UINT16*)Drv68KRam + (0x8000 / 2);
		RAM[(a - 0x204600) >> 1] = BURN_ENDIAN_SWAP_INT16(d);
		return;
	}
	
	switch (a) {
		case 0x10c000:
		case 0x10c002: {
			UINT16 *RAM = (UINT16*)DrvScroll1;
			RAM[(a - 0x10c000) >> 1] = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x10c004:
		case 0x10c006: {
			UINT16 *RAM = (UINT16*)DrvScroll0;
			RAM[(a - 0x10c004) >> 1] = BURN_ENDIAN_SWAP_INT16(d);
			return;
		}
		
		case 0x70000e: {
			MSM6295Command(0, d & 0xff);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

static INT32 SpritePlaneOffsets[4]   = { RGN_FRAC(0x200000,3,4),RGN_FRAC(0x200000,2,4),RGN_FRAC(0x200000,1,4),RGN_FRAC(0x200000,0,4) };
static INT32 SpriteXOffsets[16]      = { STEP8(0,1), STEP8(8*8*2,1) };
static INT32 SpriteYOffsets[16]      = { STEP8(0,8), STEP8(8*8*1,8) };

static INT32 DrvInit()
{
	INT32 nRet = 0, nLen;
	
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x200000);
	
	nRet = BurnLoadRom(Drv68KRom + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x000000, 1, 2); if (nRet != 0) return 1;
	
	memset(DrvTempRom, 0, 0x200000);
	nRet = BurnLoadRom(DrvTempRom + 0x000000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x080000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x100000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x180000, 5, 1); if (nRet != 0) return 1;
	GfxDecode(0x4000, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x100, DrvTempRom, DrvSprites);	
	
	nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x000000, 6, 1); if (nRet != 0) return 1;
	if (!DrvIsWatrball) { nRet = BurnLoadRom(DrvMSM6295ROMSrc + 0x080000, 7, 1); if (nRet != 0) return 1; }
	memcpy(MSM6295ROM, DrvMSM6295ROMSrc, 0x040000);
	
	BurnFree(DrvTempRom);
	
	if (DrvEncrypted) {
		UINT16 *RAM = (UINT16*)Drv68KRom;
		for (INT32 i = 0; i < 0x80000; i++) {
			UINT16 x = BURN_ENDIAN_SWAP_INT16(RAM[i]);
			x = (x & ~0x0606) | ((x & 0x0202) << 1) | ((x & 0x0404) >> 1);
			RAM[i] = BURN_ENDIAN_SWAP_INT16(x);
		}
	}
	
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(DrvVRam1            , 0x104000, 0x105fff, SM_RAM);
	SekMapMemory(DrvVRam0            , 0x106000, 0x107fff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x00c000, 0x440000, 0x441fff, SM_RAM);
	SekMapMemory(DrvSpriteRam        , 0x444000, 0x445fff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x000000, 0xfec000, 0xfeffff, SM_RAM);
	SekSetReadWordHandler(0, Blmbycar68KReadWord);
	SekSetWriteWordHandler(0, Blmbycar68KWriteWord);
	SekSetReadByteHandler(0, Blmbycar68KReadByte);
	SekSetWriteByteHandler(0, Blmbycar68KWriteByte);
	SekClose();
	
	MSM6295Init(0, 1056000 / 132, 0);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	
	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 BlmbycarInit()
{
	DrvEncrypted = 1;

	return DrvInit();
}

static INT32 WatrballInit()
{
	DrvIsWatrball = 1;
	
	return DrvInit();
}

static INT32 DrvExit()
{
	SekExit();
	
	MSM6295Exit(0);
	
	GenericTilesExit();
	
	BurnFree(Mem);
	
	DrvEncrypted = 0;
	DrvIsWatrball = 0;
	BlmbyPotWheel = 0;
	DrvOkiBank = 0;
	DrvToggle = 0;
	
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

	r = pal4bit(nColour >> 4);
	g = pal4bit(nColour >> 0);
	b = pal4bit(nColour >> 8);

	return BurnHighCol(r, g, b, 0);
}

static void DrvCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)DrvPaletteRam, pd = DrvPalette; i < 0x300; i++, ps++, pd++) {
		*pd = CalcCol(BURN_ENDIAN_SWAP_INT16(*ps));
	}
}

static void DrvRenderBgLayer(INT32 RenderCategory)
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0, xFlip, yFlip, Category;
	
	UINT16 *RAM = (UINT16*)DrvVRam0;
	UINT16 *ScrollRAM = (UINT16*)DrvScroll0;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = BURN_ENDIAN_SWAP_INT16(RAM[(TileIndex << 1) + 0]);
			Attr = BURN_ENDIAN_SWAP_INT16(RAM[(TileIndex << 1) + 1]);
			Colour = Attr & 0x1f;
			xFlip = (Attr >> 6) & 0x01;
			yFlip = (Attr >> 6) & 0x02;
			Category = (Attr >> 5) & 0x01;
			
			if (Category == RenderCategory) {			
				x = 16 * mx;
				y = 16 * my;
			
				x -= BURN_ENDIAN_SWAP_INT16(ScrollRAM[1]) & 0x3ff;
				y -= BURN_ENDIAN_SWAP_INT16(ScrollRAM[0]) & 0x1ff;
				if (x < -16) x += 1024;
				if (y < -16) y += 512;
				
				if (DrvIsWatrball) y -= 16;
			
				if (x > 16 && x < 368 && y > 16 && y < (nScreenHeight - 16)) {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						} else {
							Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						} else {
							Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						} else {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}

static void DrvRenderFgLayer(INT32 RenderCategory)
{
	INT32 mx, my, Attr, Code, Colour, x, y, TileIndex = 0, xFlip, yFlip, Category;
	
	UINT16 *RAM = (UINT16*)DrvVRam1;
	UINT16 *ScrollRAM = (UINT16*)DrvScroll1;
	
	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = BURN_ENDIAN_SWAP_INT16(RAM[(TileIndex << 1) + 0]);
			Attr = BURN_ENDIAN_SWAP_INT16(RAM[(TileIndex << 1) + 1]);
			Colour = Attr & 0x1f;
			xFlip = (Attr >> 6) & 0x01;
			yFlip = (Attr >> 6) & 0x02;
			Category = (Attr >> 5) & 0x01;
			
			if (Category == RenderCategory) {
				x = 16 * mx;
				y = 16 * my;
			
				x -= (BURN_ENDIAN_SWAP_INT16(ScrollRAM[1]) + 5) & 0x3ff;
				y -= (BURN_ENDIAN_SWAP_INT16(ScrollRAM[0]) + 1) & 0x1ff;
				if (x < -16) x += 1024;
				if (y < -16) y += 512;
				
				if (DrvIsWatrball) y -= 16;
			
				if (x > 16 && x < 368 && y > 16 && y < (nScreenHeight - 16)) {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						} else {
							Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						} else {
							Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						}
					}
				} else {
					if (xFlip) {
						if (yFlip) {
							Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						} else {
							Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						}
					} else {
						if (yFlip) {
							Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						} else {
							Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
						}
					}
				}
			}
			
			TileIndex++;
		}
	}
}

static void DrawSprites(INT32 RenderPriority)
{
	UINT16 *Source, *Finish;
	UINT16 *RAM = (UINT16*)DrvSpriteRam;

	Source = RAM + 0x06 / 2;
	Finish = RAM + (0x2000 / 2) - (8 / 2);

	for (; Source < Finish; Source += 8 / 2) {
		if (BURN_ENDIAN_SWAP_INT16(Source[0]) & 0x8000)	break;
	}

	Source -= 8 / 2;
	Finish = RAM;

	for (; Source >= Finish; Source -= 8 / 2) {
		INT32 y = BURN_ENDIAN_SWAP_INT16(Source[0]);
		INT32 Code = BURN_ENDIAN_SWAP_INT16(Source[1]);
		INT32 Attr = BURN_ENDIAN_SWAP_INT16(Source[2]);
		INT32 x = BURN_ENDIAN_SWAP_INT16(Source[3]);

		INT32 xFlip = Attr & 0x4000;
		INT32 yFlip = Attr & 0x8000;
		INT32 Pri = (~Attr >> 3) & 0x1;
		
		if (RenderPriority == Pri) {		
			INT32 Colour = 0x20 + (Attr & 0x0f);

			if (x & 0x4000)	continue;

			x = (x & 0x1ff) - 0x10;
			y = 0xf0 - ((y & 0xff)  - (y & 0x100));
			
			if (DrvIsWatrball) y -= 16;

			if (x > 16 && x < 368 && y > 16 && y < (nScreenHeight - 16)) {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_Mask_FlipXY(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask_FlipX(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_Mask_FlipY(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
					}
				}
			} else {
				if (xFlip) {
					if (yFlip) {
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
					}
				} else {
					if (yFlip) {
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 4, 0, 0, DrvSprites);
					}
				}
			}			
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	if (nBurnLayer & 0x01) DrvRenderBgLayer(0);
	if (nBurnLayer & 0x02) DrvRenderBgLayer(1);
	if (nBurnLayer & 0x04) DrvRenderFgLayer(0);
	if (nSpriteEnable & 0x01) DrawSprites(0);
	if (nBurnLayer & 0x08) DrvRenderFgLayer(1);
	if (nSpriteEnable & 0x02) DrawSprites(1);
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nCyclesTotal = 10000000 / 60;
	
	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	SekNewFrame();
	SekOpen(0);
	SekRun(nCyclesTotal);
	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
	SekClose();
	
	if (pBurnSoundOut) MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	
	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029717;
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
		MSM6295Scan(0, nAction);
		
		SCAN_VAR(DrvInput);
		SCAN_VAR(DrvDip);
		SCAN_VAR(BlmbyPotWheel);
		SCAN_VAR(DrvOkiBank);
		SCAN_VAR(DrvToggle);
		
		if (nAction & ACB_WRITE) {
			DrvDoOkiBank(DrvOkiBank);
		}
	}

	return 0;
}

struct BurnDriver BurnDrvBlmbycar = {
	"blmbycar", NULL, NULL, NULL, "1994",
	"Blomby Car\0", NULL, "ABM & Gecas", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_RACING, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	BlmbycarInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x300, 384, 256, 4, 3
};

struct BurnDriver BurnDrvBlmbycaru = {
	"blmbycaru", "blmbycar", NULL, NULL, "1994",
	"Blomby Car (not encrypted)\0", NULL, "ABM & Gecas", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_RACING, 0,
	NULL, DrvuRomInfo, DrvuRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x300, 384, 256, 4, 3
};

struct BurnDriver BurnDrvWatrball = {
	"watrball", NULL, NULL, NULL, "1996",
	"Water Balls\0", NULL, "ABM", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, WatrballRomInfo, WatrballRomName, NULL, NULL, DrvInputInfo, WatrballDIPInfo,
	WatrballInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x300, 384, 240, 4, 3
};
