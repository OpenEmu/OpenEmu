// FB Alpha Diverboy driver module
// Based on MAME driver by David Haywood

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
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
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT32 *DrvPalette;

static UINT8 DrvRecalc;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[1];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static UINT8 *soundlatch;
static UINT8 *samplebank;

static struct BurnInputInfo DiverboyInputList[] = {
	{"Coin 1",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"Coin 2",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},
	{"Coin 3",		BIT_DIGITAL,	DrvJoy2 + 2,	"p3 coin"	},

	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Diverboy)

static struct BurnDIPInfo DiverboyDIPList[]=
{
	{0x12, 0xff, 0xff, 0xb8, NULL			},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x12, 0x01, 0x07, 0x07, "4 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x06, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x05, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x00, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x07, 0x01, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x07, 0x02, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x07, 0x03, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x07, 0x04, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x12, 0x01, 0x08, 0x00, "2"			},
	{0x12, 0x01, 0x08, 0x08, "3"			},

	{0   , 0xfe, 0   ,    2, "Display Copyright"	},
	{0x12, 0x01, 0x10, 0x00, "No"			},
	{0x12, 0x01, 0x10, 0x10, "Yes"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x60, 0x00, "Easy"			},
	{0x12, 0x01, 0x60, 0x20, "Normal"		},
	{0x12, 0x01, 0x60, 0x40, "Hard"			},
	{0x12, 0x01, 0x60, 0x60, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x12, 0x01, 0x80, 0x80, "No"			},
	{0x12, 0x01, 0x80, 0x00, "Yes"			},
};

STDDIPINFO(Diverboy)

void __fastcall diverboy_write_byte(UINT32 /*address*/, UINT8 /*data*/)
{
}

void __fastcall diverboy_write_word(UINT32 address, UINT16 data)
{
	if (address == 0x100000) {
		*soundlatch = data;
		ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		return;
	}
}

UINT8 __fastcall diverboy_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x180000:
			return DrvInputs[0] >> 8;

		case 0x180001:
			return DrvInputs[0] & 0xff;

		case 0x180009:
			return DrvInputs[1] & 0xf7;
	}

	return 0;
}

UINT16 __fastcall diverboy_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x180002:
			return DrvDips[0];

		case 0x180008:
			return DrvInputs[1];
	}

	return 0;
}

static inline void sample_bank(INT32 data)
{
	*samplebank = data & 3;

	MSM6295ROM = DrvSndROM + ((data & 3) << 18);
}

void __fastcall diverboy_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x9000:
			sample_bank(data);
		return;

		case 0x9800:
			MSM6295Command(0, data);
		return;
	}
}

UINT8 __fastcall diverboy_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x9800:
		return MSM6295ReadStatus(0);

		case 0xa000:
			return *soundlatch;
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

	sample_bank(0);
	MSM6295Reset(0);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x040000;
	DrvZ80ROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x200000;
	DrvGfxROM1	= Next; Next += 0x100000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x100000;

	DrvPalette	= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvPalRAM	= Next; Next += 0x000800;
	DrvSprRAM	= Next; Next += 0x004000;

	DrvZ80RAM	= Next; Next += 0x000800;

	soundlatch	= Next; Next += 0x000001;
	samplebank	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4]  = { 0,  1,  2,  3 };
	INT32 XOffs[16] = { 56, 60, 48, 52, 40, 44, 32, 36, 24, 28, 16, 20, 8, 12, 0, 4 };
	INT32 YOffs[16] = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x100000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x100000);

	GfxDecode(0x2000, 4, 16, 16, Plane, XOffs, YOffs, 0x400, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x080000);

	GfxDecode(0x1000, 4, 16, 16, Plane, XOffs, YOffs, 0x400, tmp, DrvGfxROM1);

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
		if (BurnLoadRom(Drv68KROM  + 0x000001,	 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,	 1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,		 2, 1)) return 1;
		memcpy (DrvZ80ROM, DrvZ80ROM + 0x8000, 0x8000);

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,	 3, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001,	 4, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,	 5, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x000001,	 6, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040000,	 7, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040001,	 8, 2)) return 1;

		if (BurnLoadRom(DrvSndROM + 0x000000,	 9, 1)) return 1;
		memcpy (DrvSndROM + 0xc0000, DrvSndROM + 0x60000, 0x20000);
		memcpy (DrvSndROM + 0x80000, DrvSndROM + 0x40000, 0x20000);
		memcpy (DrvSndROM + 0x40000, DrvSndROM + 0x20000, 0x20000);

		if (BurnLoadRom(DrvSndROM + 0x020000,   10, 1)) return 1;
		memcpy (DrvSndROM + 0xe0000, DrvSndROM + 0x20000, 0x20000);
		memcpy (DrvSndROM + 0xa0000, DrvSndROM + 0x20000, 0x20000);
		memcpy (DrvSndROM + 0x60000, DrvSndROM + 0x20000, 0x20000);

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x040000, 0x04ffff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x080000, 0x083fff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x140000, 0x1407ff, SM_RAM);
	SekSetWriteByteHandler(0,	diverboy_write_byte);
	SekSetWriteWordHandler(0,	diverboy_write_word);
	SekSetReadByteHandler(0,	diverboy_read_byte);
	SekSetReadWordHandler(0,	diverboy_read_word);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(diverboy_sound_write);
	ZetSetReadHandler(diverboy_sound_read);
	ZetMemEnd();
	ZetClose();

	MSM6295Init(0, 1320000 / 132, 0);
	MSM6295SetRoute(0, 0.50, BURN_SND_ROUTE_BOTH);

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

static void draw_sprites()
{
	UINT16 *vram = (UINT16*)DrvSprRAM;

	for (INT32 offs = 0; offs < 0x4000 / 2; offs+=8)
	{
		INT32 sy   = ( 256 - BURN_ENDIAN_SWAP_INT16(vram[offs + 4])) - 16;
		INT32 sx   = ((480 - BURN_ENDIAN_SWAP_INT16(vram[offs + 0])) & 0x1ff) - 173;
		INT32 attr = BURN_ENDIAN_SWAP_INT16(vram[offs + 1]);
		INT32 code = BURN_ENDIAN_SWAP_INT16(vram[offs + 3]);
		INT32 color = ((attr & 0x00f0) >> 4) | ((attr & 0x000c) << 2);
		INT32 flash =  (attr & 0x1000);
		INT32 bank  =  (attr & 0x0002) >> 1;

		if (flash && (GetCurrentFrame() & 1)) continue;
		if (sx >= nScreenWidth || sy >= nScreenHeight || sx < -15 || sy < -15) continue;

		if (attr & 0x0008) {
			Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, bank ? DrvGfxROM1 : DrvGfxROM0);
		} else {
			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, bank ? DrvGfxROM1 : DrvGfxROM0);
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		UINT8 r,g,b;
		UINT16 *pal = (UINT16*)DrvPalRAM;
		for (INT32 i = 0; i < 0x800 / 2; i++, pal++) {
			r = ((BURN_ENDIAN_SWAP_INT16(*pal) << 4) & 0xf0) | ((BURN_ENDIAN_SWAP_INT16(*pal) << 0) & 0x0f);
			g = ((BURN_ENDIAN_SWAP_INT16(*pal) >> 0) & 0xf0) | ((BURN_ENDIAN_SWAP_INT16(*pal) >> 4) & 0x0f);
			b = ((BURN_ENDIAN_SWAP_INT16(*pal) >> 4) & 0xf0) | ((BURN_ENDIAN_SWAP_INT16(*pal) >> 8) & 0x0f);

			DrvPalette[i] = BurnHighCol(r, g, b, 0);
		}
	}

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
		DrvInputs[0] = ~0;
		DrvInputs[1] = ~8;

		for (INT32 i = 0; i < 16; i++)
		{
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nSegment;
	INT32 nInterleave = 10;
	INT32 nTotalCycles[2] = { 12000000 / 60, 4000000 / 60 };
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

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

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
		ZetScan(nAction);

		MSM6295Scan(0, nAction);

		if (nAction & ACB_WRITE) {
			sample_bank(*samplebank);
		}
	}

	return 0;
}


// Diver Boy

static struct BurnRomInfo diverboyRomDesc[] = {
	{ "db_01.bin",	0x20000, 0x6aa11366, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "db_02.bin",	0x20000, 0x45f8a673, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "db_05.bin",	0x10000, 0xffeb49ec, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "db_08.bin",	0x80000, 0x7bb96220, 3 | BRF_GRA },           //  3 Bank 0 Graphics
	{ "db_09.bin",	0x80000, 0x12b15476, 3 | BRF_GRA },           //  4

	{ "db_07.bin",	0x20000, 0x18485741, 4 | BRF_GRA },           //  5 Bank 1 Graphics
	{ "db_10.bin",	0x20000, 0xc381d1cc, 4 | BRF_GRA },           //  6
	{ "db_06.bin",	0x20000, 0x21b4e352, 4 | BRF_GRA },           //  7
	{ "db_11.bin",	0x20000, 0x41d29c81, 4 | BRF_GRA },           //  8

	{ "db_03.bin",	0x80000, 0x50457505, 5 | BRF_SND },           //  9 Oki6295 Samples
	{ "db_04.bin",	0x20000, 0x01b81da0, 5 | BRF_SND },           // 10
};

STD_ROM_PICK(diverboy)
STD_ROM_FN(diverboy)

struct BurnDriver BurnDrvDiverboy = {
	"diverboy", NULL, NULL, NULL, "1992",
	"Diver Boy\0", NULL, "Electronic Devices Italy", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MISC, 0,
	NULL, diverboyRomInfo, diverboyRomName, NULL, NULL, DiverboyInputInfo, DiverboyDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 240, 4, 3
};
