// FB Alpha Xor World driver module
// Based on MAME driver by Manuel Abadia

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "saa1099.h"
#include "eeprom.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvGfxROM;
static UINT8 *DrvColPROM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvSprRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvDips[1];
static UINT8 DrvReset;
static UINT8 DrvInputs[2];

static struct BurnInputInfo XorworldInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Xorworld)

static struct BurnDIPInfo XorworldDIPList[]=
{
	{0x11, 0xff, 0xff, 0xc7, NULL			},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x11, 0x01, 0x07, 0x00, "3 Coins 1 Credits"	},
	{0x11, 0x01, 0x07, 0x01, "2 Coins 2 Credits"	},
	{0x11, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x11, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},
	{0x11, 0x01, 0x07, 0x02, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x11, 0x01, 0x08, 0x08, "Off"			},
	{0x11, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x11, 0x01, 0x60, 0x40, "Easy"			},
	{0x11, 0x01, 0x60, 0x60, "Normal"		},
	{0x11, 0x01, 0x60, 0x20, "Hard"			},
	{0x11, 0x01, 0x60, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x11, 0x01, 0x80, 0x80, "Off"			},
	{0x11, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Xorworld)

void __fastcall xorworld_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x800001:
			saa1099DataWrite(0, data);
		return;
	
		case 0x800003:
			saa1099ControlWrite(0, data);
		return;

		case 0xa00009:
			EEPROMSetCSLine((data & 0x01) ? EEPROM_CLEAR_LINE : EEPROM_ASSERT_LINE);
		return;

		case 0xa0000b:
			EEPROMSetClockLine((data & 0x01) ? EEPROM_ASSERT_LINE : EEPROM_CLEAR_LINE);
		return;

		case 0xa0000d:
			EEPROMWriteBit((data & 0x01));
		return;
	}
}

UINT8 __fastcall xorworld_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x200001:
			return DrvInputs[0];

		case 0x400000:
			return DrvInputs[1];

		case 0x600001:
			return (DrvDips[0] & ~0x10) | (EEPROMRead() ? 0x10 : 0);
	}

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	EEPROMReset();

	saa1099Reset(0);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x020000;

	DrvGfxROM	= Next; Next += 0x040000;

	DrvColPROM	= Next; Next += 0x000300;

	DrvPalette	= (UINT32*)Next; Next += 0x0100 * sizeof(UINT32);

	AllRam		= Next;

	DrvSprRAM	= Next; Next += 0x003800;
	DrvVidRAM	= Next; Next += 0x000800;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 0x100; i++) {
		INT32 r,g,b;

		r  = ((DrvColPROM[i + 0x000] >> 0) & 0x01) * 0x0e;
		r += ((DrvColPROM[i + 0x000] >> 1) & 0x01) * 0x1e;
		r += ((DrvColPROM[i + 0x000] >> 2) & 0x01) * 0x44;
		r += ((DrvColPROM[i + 0x000] >> 3) & 0x01) * 0x8f;

		g  = ((DrvColPROM[i + 0x100] >> 0) & 0x01) * 0x0e;
		g += ((DrvColPROM[i + 0x100] >> 1) & 0x01) * 0x1e;
		g += ((DrvColPROM[i + 0x100] >> 2) & 0x01) * 0x44;
		g += ((DrvColPROM[i + 0x100] >> 3) & 0x01) * 0x8f;

		b  = ((DrvColPROM[i + 0x200] >> 0) & 0x01) * 0x0e;
		b += ((DrvColPROM[i + 0x200] >> 1) & 0x01) * 0x1e;
		b += ((DrvColPROM[i + 0x200] >> 2) & 0x01) * 0x44;
		b += ((DrvColPROM[i + 0x200] >> 3) & 0x01) * 0x8f;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4] = { 0x80000, 0x80004, 0x00000, 0x00004 };
	INT32 XOffs[8] = { 0x000, 0x001, 0x002, 0x003, 0x008, 0x009, 0x00a, 0x00b };
	INT32 YOffs[8] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x20000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM, 0x20000);

	GfxDecode((0x20000 * 2) / 0x40, 4, 8, 8, Plane, XOffs, YOffs, 0x080, tmp, DrvGfxROM);

	BurnFree (tmp);

	return 0;
}

static void xorworldPatch() // protection hack
{
	UINT16 *rom = (UINT16*)Drv68KROM;

	rom[0x1390 / 2] = 0x4239;
	rom[0x1392 / 2] = 0x00ff;
	rom[0x1394 / 2] = 0xe196;
	rom[0x1396 / 2] = 0x4239;
	rom[0x1398 / 2] = 0x00ff;
	rom[0x139a / 2] = 0xe197;
	rom[0x139c / 2] = 0x4239;
	rom[0x139e / 2] = 0x00ff;
	rom[0x13a0 / 2] = 0xe0bc;
	rom[0x13a2 / 2] = 0x41f9;
	rom[0x13a4 / 2] = 0x00ff;
	rom[0x13a6 / 2] = 0xcfce;
	rom[0x13a8 / 2] = 0x3e3c;
	rom[0x13aa / 2] = 0x000f;
	rom[0x13ac / 2] = 0x4218;
	rom[0x13ae / 2] = 0x51cf;
	rom[0x13b0 / 2] = 0xfffc;
	rom[0x13b2 / 2] = 0x4e75;
	rom[0x13b4 / 2] = 0x31ff;
}

static INT32 DrvInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x00001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x00000,  1, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM  + 0x00000,  2, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM  + 0x10000,  3, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x00000,  4, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x00100,  5, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x00200,  6, 1)) return 1;

		DrvGfxDecode();
		DrvPaletteInit();

		xorworldPatch();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x01ffff, SM_ROM);
	SekMapMemory(DrvVidRAM,		0xffc000, 0xffc7ff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0xffc800, 0xffffff, SM_RAM);
	SekSetWriteByteHandler(0,	xorworld_write_byte);
	SekSetReadByteHandler(0,	xorworld_read_byte);
	SekClose();

	saa1099Init(0, 8000000, 0);
	saa1099SetAllRoutes(0, 1.00, BURN_SND_ROUTE_BOTH);

	EEPROMInit(&eeprom_interface_93C46);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	EEPROMExit();

	SekExit();

	saa1099Exit(0);

	BurnFree (AllMem);

	return 0;
}

static void draw_layer()
{
	UINT16 *ram = (UINT16*)DrvVidRAM;

	for (INT32 offs = 32 * 2; offs < (32 * 32) - (32 * 2); offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		INT32 code  = ram[offs] & 0x0fff;
		INT32 color = ram[offs] >> 12;

		Render8x8Tile(pTransDraw, code, sx, sy - 16, color, 4, 0, DrvGfxROM);
	}
}

static void draw_sprites()
{
	UINT16 *ram = (UINT16*)DrvSprRAM;

	for (INT32 i = 0; i < 0x40; i += 2)
	{
		INT32 sx    = ram[i] & 0xff;
		INT32 sy    = (240 - (ram[i] >> 8)) - 16;
		INT32 code  = ram[i+1] & 0x0ffc;
		INT32 color = ram[i+1] >> 12;

		Render8x8Tile_Mask_Clip(pTransDraw, code + 0, sx + 0, sy + 0, color, 4, 0, 0, DrvGfxROM);
		Render8x8Tile_Mask_Clip(pTransDraw, code + 1, sx + 0, sy + 8, color, 4, 0, 0, DrvGfxROM);
		Render8x8Tile_Mask_Clip(pTransDraw, code + 2, sx + 8, sy + 0, color, 4, 0, 0, DrvGfxROM);
		Render8x8Tile_Mask_Clip(pTransDraw, code + 3, sx + 8, sy + 8, color, 4, 0, 0, DrvGfxROM);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvPaletteInit();
		DrvRecalc = 0;
	}

	draw_layer();
	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 2);

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	SekOpen(0);

	for (INT32 i = 0; i < 4; i++)
	{
		SekRun((10000000 / 60) / 4);
		if (i == 0 || i == 2) SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
		if (/*i==1||*/i == 3) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
	}

	if (pBurnSoundOut) {
		saa1099Update(0, pBurnSoundOut, nBurnSoundLen);
	}

	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029698;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);

		saa1099Scan(0, nAction);

		EEPROMScan(nAction, pnMin);
	}

	return 0;
}


// Xor World (prototype)

static struct BurnRomInfo xorworldRomDesc[] = {
	{ "c13.bin",	0x10000, 0x615a864d, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "b13.bin",	0x10000, 0x632e8ee5, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "d9.bin",	0x10000, 0xda8d4d65, 2 | BRF_GRA },           //  2 Tiles and Sprites
	{ "d10.bin",	0x10000, 0x3b1d6f24, 2 | BRF_GRA },           //  3

	{ "b4.bin",	0x00100, 0x75e468af, 3 | BRF_GRA },           //  4 Color PROMs
	{ "b7.bin",	0x00100, 0x7e1cd146, 3 | BRF_GRA },           //  5
	{ "b5.bin",	0x00100, 0xc1b9d9f9, 3 | BRF_GRA },           //  6
};

STD_ROM_PICK(xorworld)
STD_ROM_FN(xorworld)

struct BurnDriver BurnDrvXorworld = {
	"xorworld", NULL, NULL, NULL, "1990",
	"Xor World (prototype)\0", NULL, "Gaelco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, xorworldRomInfo, xorworldRomName, NULL, NULL, XorworldInputInfo, XorworldDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	256, 224, 4, 3
};
