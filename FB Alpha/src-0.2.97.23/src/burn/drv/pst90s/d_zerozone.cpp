// FB Alpha Zero Zone driver module
// Based on MAME driver by Brad Oliver

#include "tiles_generic.h"
#include "sek.h"
#include "zet.h"
#include "msm6295.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvSndROM;
static UINT8 *DrvGfxROM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvVidRAM;
static UINT32 *DrvPalette;
static UINT32 *Palette;

static UINT8 DrvRecalc;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy4[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT16 DrvInputs[4];

static UINT8 soundlatch;
static UINT8 tilebank;

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1"       	  , BIT_DIGITAL  , DrvJoy1 + 0,	 "p1 coin"  },
	{"Coin 2"       	  , BIT_DIGITAL  , DrvJoy1 + 1,	 "p2 coin"  },

	{"P1 Start"     	  , BIT_DIGITAL  , DrvJoy1 + 3,	 "p1 start" },
	{"P1 Up"        	  , BIT_DIGITAL  , DrvJoy2 + 3,  "p1 up"    },
	{"P1 Down"      	  , BIT_DIGITAL  , DrvJoy2 + 2,  "p1 down"  },
	{"P1 Left"      	  , BIT_DIGITAL  , DrvJoy2 + 1,  "p1 left"  },
	{"P1 Right"     	  , BIT_DIGITAL  , DrvJoy2 + 0,  "p1 right" },
	{"P1 Button 1"  	  , BIT_DIGITAL  , DrvJoy2 + 4,  "p1 fire 1"},
	{"P1 Button 2"  	  , BIT_DIGITAL  , DrvJoy2 + 5,  "p1 fire 2"},
	{"P1 Score Line (cheat)"  , BIT_DIGITAL  , DrvJoy2 + 6,  "p1 fire 3"},

	{"P2 Start"     	  , BIT_DIGITAL  , DrvJoy1 + 4,	 "p2 start" },
	{"P2 Up"        	  , BIT_DIGITAL  , DrvJoy2 + 11, "p2 up"    },
	{"P2 Down"      	  , BIT_DIGITAL  , DrvJoy2 + 10, "p2 down"  },
	{"P2 Left"      	  , BIT_DIGITAL  , DrvJoy2 + 9,  "p2 left"  },
	{"P2 Right"     	  , BIT_DIGITAL  , DrvJoy2 + 8,  "p2 right" },
	{"P2 Button 1"  	  , BIT_DIGITAL  , DrvJoy2 + 12, "p2 fire 1"},
	{"P2 Button 2"  	  , BIT_DIGITAL  , DrvJoy2 + 13, "p2 fire 2"},
	{"P2 Score Line (cheat)"  , BIT_DIGITAL  , DrvJoy2 + 14, "p2 fire 3"},

	{"Service"      	  , BIT_DIGITAL  , DrvJoy4 + 7,  "service"  },

	{"Reset",		    BIT_DIGITAL  , &DrvReset,	 "reset"    },
	{"Dip 1",		    BIT_DIPSWITCH, DrvDips + 0,	 "dip"	   },
	{"Dip 2",		    BIT_DIPSWITCH, DrvDips + 1,	 "dip"	   },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	{0x14, 0xff, 0xff, 0xdf, NULL },
	{0x15, 0xff, 0xff, 0xf7, NULL },

	{0x14, 0xfe, 0,       8, "Coinage" },
	{0x14, 0x01, 0x07, 0x00, "5C 1C" },
	{0x14, 0x01, 0x07, 0x01, "4C 1C" },
	{0x14, 0x01, 0x07, 0x02, "3C 1C" },
	{0x14, 0x01, 0x07, 0x03, "2C 1C" },
	{0x14, 0x01, 0x07, 0x07, "1C 1C" },
	{0x14, 0x01, 0x07, 0x06, "1C 2C" },
	{0x14, 0x01, 0x07, 0x05, "1C 3C" },
	{0x14, 0x01, 0x07, 0x04, "1C 4C" },

	{0x14, 0xfe, 0,       2, "Difficulty" },
	{0x14, 0x01, 0x08, 0x08, "In Game Default" },
	{0x14, 0x01, 0x08, 0x00, "Always Hard" },

	{0x14, 0xfe, 0,       2, "Speed" },
	{0x14, 0x01, 0x10, 0x10, "Normal" },
	{0x14, 0x01, 0x10, 0x00, "Fast" },

	{0x14, 0xfe, 0,       2, "Demo Sounds" },
	{0x14, 0x01, 0x20, 0x00, "Off" },
	{0x14, 0x01, 0x20, 0x20, "On" },

	{0x15, 0xfe, 0,       2, "Helps" },
	{0x15, 0x01, 0x04, 0x04, "1" },
	{0x15, 0x01, 0x04, 0x00, "2" },

	{0x15, 0xfe, 0,       2, "Bonus Help" },
	{0x15, 0x01, 0x08, 0x00, "30000" },
	{0x15, 0x01, 0x08, 0x08, "None" },

	{0x15, 0xfe, 0,       2, "Activate 'Score Line'? (Cheat)" },
	{0x15, 0x01, 0x10, 0x10, "No" },
	{0x15, 0x01, 0x10, 0x00, "Yes" },
};

STDDIPINFO(Drv)

UINT8 __fastcall zerozone_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x080000:
		case 0x080001:
			return DrvInputs[0] >> ((~address & 1) << 3);

		case 0x080002:
		case 0x080003:
			return DrvInputs[1] >> ((~address & 1) << 3);

		case 0x080008:
		case 0x080009:
			return DrvInputs[2] >> ((~address & 1) << 3);

		case 0x08000a:
		case 0x08000b:
			return DrvInputs[3] >> ((~address & 1) << 3);
	}

	return 0;
}

UINT16 __fastcall zerozone_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x080000:
			return DrvInputs[0];

		case 0x080002:
			return DrvInputs[1];

		case 0x080008:
			return DrvInputs[2];

		case 0x08000a:
			return DrvInputs[3];
	}

	return 0;
}

static void palette_write(INT32 offset)
{
	UINT8 r, b, g;
	UINT16 data = *((UINT16*)(DrvPalRAM + offset));

	r  = (data >> 11) & 0x1e;
	r |= (data >>  3) & 0x01;
	r  = (r << 3) | (r >> 2);

	g  = (data >>  7) & 0x1e;
	g |= (data >>  2) & 0x01;
	g  = (g << 3) | (g >> 2);

	b  = (data >>  3) & 0x1e;
	b |= (data >>  1) & 0x01;
	b  = (b << 3) | (b >> 2);

	Palette[offset>>1] = (r << 16) | (g << 8) | b;
	DrvPalette[offset>>1] = BurnHighCol(r, g, b, 0);

	return;
}

void __fastcall zerozone_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xffe00) == 0x88000) {
		*((UINT16*)(DrvPalRAM + (address & 0x1fe))) = data;
		palette_write(address & 0x1fe);
		return;
	}

	switch (address)
	{
		case 0x84000:
			soundlatch = data >> 8;
			ZetRaiseIrq(0xff);
		return;

		case 0xb4000:
			tilebank = data & 7;
		return;
	}

	return;
}

void __fastcall zerozone_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x84000:
			soundlatch = data;
			ZetRaiseIrq(0xff);
		return;

		case 0xb4001:
			tilebank = data & 7;
		return;
	}

	return;
}

void __fastcall zerozone_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x9800:
			MSM6295Command(0, data);
		return;
	}
}

UINT8 __fastcall zerozone_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x9800:
			return MSM6295ReadStatus(0);

		case 0xa000:
			return soundlatch;
	}

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	soundlatch = 0;
	tilebank = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x020000;
	DrvZ80ROM	= Next; Next += 0x008000;

	DrvGfxROM	= Next; Next += 0x100000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x040000;

	DrvPalette	= (UINT32*)Next; Next += 0x0100 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvPalRAM	= Next; Next += 0x000200;
	DrvVidRAM	= Next; Next += 0x004000;

	DrvZ80RAM	= Next; Next += 0x000800;

	Palette		= (UINT32*)Next; Next += 0x0100 * sizeof(UINT32);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4] = { 0,  1,  2,  3 };
	INT32 XOffs[8] = { 0,  4,  8, 12, 16, 20, 24, 28 };
	INT32 YOffs[8] = { 0, 32, 64, 96, 128, 160, 192, 224 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM, 0x80000);

	GfxDecode(0x4000, 4, 8, 8, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM);

	BurnFree (tmp);

	return 0;
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
		if (BurnLoadRom(Drv68KROM + 1,		0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0,		1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,		2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM,		3, 1)) return 1;

		if (BurnLoadRom(DrvSndROM,		4, 1)) return 1;
		if (BurnLoadRom(DrvSndROM + 0x20000,    5, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x01ffff, SM_ROM);
	SekMapMemory(DrvPalRAM,		0x088000, 0x0881ff, SM_ROM);
	SekMapMemory(DrvVidRAM,		0x09ce00, 0x09ffff, SM_RAM);
	SekMapMemory(Drv68KRAM,		0x0c0000, 0x0cffff, SM_RAM);
	SekSetWriteByteHandler(0,	zerozone_write_byte);
	SekSetWriteWordHandler(0,	zerozone_write_word);
	SekSetReadByteHandler(0,	zerozone_read_byte);
	SekSetReadWordHandler(0,	zerozone_read_word);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(zerozone_sound_write);
	ZetSetReadHandler(zerozone_sound_read);
	ZetMemEnd();
	ZetClose();

	MSM6295Init(0, 1056000 / 132, 100.0, 0);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	MSM6295Exit(0);
	SekExit();
	ZetExit();

	BurnFree (AllMem);

	MSM6295ROM = NULL;

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x100; i++) {
			INT32 rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	UINT16 *vram = (UINT16*)DrvVidRAM;

	for (INT32 offs = 0; offs < 0x800; offs++)
	{
		INT32 sy = (offs & 0x1f) << 3;
		INT32 sx = (offs >> 5) << 3;

		sy -= 16;
		sx -=  8;
		if (sy < 0 || sx < 0 || sy >= nScreenHeight || sx >= nScreenWidth) continue;

		INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[offs]) & 0x7ff;
		INT32 color = BURN_ENDIAN_SWAP_INT16(vram[offs]) >> 12;

		code += tilebank * (BURN_ENDIAN_SWAP_INT16(vram[offs]) & 0x800);

		Render8x8Tile(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM);
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}


static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = ~0;
		DrvInputs[1] = ~0;
		DrvInputs[2] = 0x00ff | (DrvDips[1] << 8);
		DrvInputs[3] = 0xff00 | DrvDips[0];

		for (INT32 i = 0; i < 16; i++)
		{
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
		}
	}

	INT32 nSegment;
	INT32 nInterleave = 10;
	INT32 nTotalCycles[2] = { 10000000 / 60, 1000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nSegment = (nTotalCycles[0] - nCyclesDone[0]) / (nInterleave - i);

		nCyclesDone[0] += SekRun(nSegment);

		nSegment = (nTotalCycles[1] - nCyclesDone[1]) / (nInterleave - i);

		nCyclesDone[1] += ZetRun(nSegment);
	}

	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

	ZetClose();
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
		*pnMin = 0x029692;
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
		ZetScan(nAction);

		MSM6295Scan(0, nAction);

		SCAN_VAR(soundlatch);
		SCAN_VAR(tilebank);
	}

	return 0;
}


// Zero Zone

static struct BurnRomInfo zerozoneRomDesc[] = {
	{ "zz-4.rom",	0x10000, 0x83718b9b, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "zz-5.rom",	0x10000, 0x18557f41, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "zz-1.rom",	0x08000, 0x223ccce5, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "zz-6.rom",	0x80000, 0xc8b906b9, 3 | BRF_GRA },	      //  3 - Tiles

	{ "zz-2.rom",	0x20000, 0xc7551e81, 4 | BRF_SND },	      //  4 - MSM6295 Samples
	{ "zz-3.rom",	0x20000, 0xe348ff5e, 4 | BRF_SND },	      //  5
};

STD_ROM_PICK(zerozone)
STD_ROM_FN(zerozone)

struct BurnDriver BurnDrvZerozone = {
	"zerozone", NULL, NULL, NULL, "1993",
	"Zero Zone\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, 0,
	NULL, zerozoneRomInfo, zerozoneRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	368, 224, 4, 3
};


// Las Vegas Girl (Girl '94)

static struct BurnRomInfo lvgirl94RomDesc[] = {
	{ "rom4",	0x10000, 0xc4fb449e, 1 | BRF_PRG | BRF_ESS }, //  0 - 68k Code
	{ "rom5",	0x10000, 0x5d446a1a, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "rom1",	0x08000, 0x223ccce5, 2 | BRF_PRG | BRF_ESS }, //  2 - Z80 Code

	{ "rom6",	0x40000, 0xeeeb94ba, 3 | BRF_GRA },	      //  3 - Tiles

	{ "rom2",	0x20000, 0xc7551e81, 4 | BRF_SND },	      //  4 - MSM6295 Samples
	{ "rom3",	0x20000, 0xe348ff5e, 4 | BRF_SND },	      //  5
};

STD_ROM_PICK(lvgirl94)
STD_ROM_FN(lvgirl94)

struct BurnDriver BurnDrvLvgirl94 = {
	"lvgirl94", NULL, NULL, NULL, "1994",
	"Las Vegas Girl (Girl '94)\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, 0,
	NULL, lvgirl94RomInfo, lvgirl94RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	368, 224, 4, 3
};
