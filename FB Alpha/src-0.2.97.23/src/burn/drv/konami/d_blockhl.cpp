// FB Alpha Block Hole driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "zet.h"
#include "konami_intf.h"
#include "konamiic.h"
#include "burn_ym2151.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvKonROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvBankRAM;
static UINT8 *DrvKonRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;
static UINT32  *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *nDrvKonamiBank;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static struct BurnInputInfo BlockhlInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"	},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Blockhl)

static struct BurnDIPInfo BlockhlDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0x5f, NULL			},
	{0x16, 0xff, 0xff, 0xf0, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x14, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"	  },
	{0x14, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"	  },
	{0x14, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"	  },
	{0x14, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x14, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x14, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"	  },
	{0x14, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x14, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x14, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x14, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x14, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    15, "Coin B"		},
	{0x14, 0x01, 0xf0, 0x20, "4 Coins 1 Credit"	  },
	{0x14, 0x01, 0xf0, 0x50, "3 Coins 1 Credit"	  },
	{0x14, 0x01, 0xf0, 0x80, "2 Coins 1 Credit"	  },
	{0x14, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x14, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x14, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"	  },
	{0x14, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x14, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x14, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x14, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x15, 0x01, 0x01, 0x01, "1"			},
	{0x15, 0x01, 0x01, 0x00, "2"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x60, 0x60, "Easy"			},
	{0x15, 0x01, 0x60, 0x40, "Normal"		},
	{0x15, 0x01, 0x60, 0x20, "Difficult"		},
	{0x15, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x16, 0x01, 0x10, 0x10, "Off"			},
//	{0x16, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x16, 0x01, 0x40, 0x40, "Off"			},
	{0x16, 0x01, 0x40, 0x00, "On"			},
};

STDDIPINFO(Blockhl)

void blockhl_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x1f84:
			*soundlatch = data;
		return;

		case 0x1f88:
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;

		case 0x1f8c:
			// watchdog
		return;
	}

	if ((address & 0xc000) == 0x0000) {
		K052109_051960_w(address & 0x3fff, data);
		return;
	}
}

UINT8 blockhl_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x1f94:
			return DrvDips[2] | (DrvInputs[2] & 0x0f);

		case 0x1f95:
			return DrvInputs[0];

		case 0x1f96:
			return DrvInputs[1];

		case 0x1f97:
			return DrvDips[0];

		case 0x1f98:
			return DrvDips[1];
	}

	if ((address & 0xc000) == 0x0000) {
		return K052109_051960_r(address & 0x3fff);
	}

	return 0;
}

void __fastcall blockhl_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xc000:
			BurnYM2151SelectRegister(data);
		return;

		case 0xc001:
			BurnYM2151WriteRegister(data);
		return;
	}
}

UINT8 __fastcall blockhl_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xa000:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;

		case 0xc001:
			return BurnYM2151ReadStatus();
	}

	return 0;
}

static void blockhl_set_lines(INT32 lines)
{
	INT32 nBank = (lines & 0x03) * 0x2000;

	konamiMapMemory(DrvKonROM + nBank, 0x6000, 0x7fff, KON_ROM); 

	if (~lines & 0x20) {
		konamiMapMemory(DrvPalRAM,  0x5800, 0x5fff, KON_RAM);
	} else {
		konamiMapMemory(DrvBankRAM, 0x5800, 0x5fff, KON_RAM);
	}

	K052109RMRDLine = lines & 0x40;

	nDrvKonamiBank[0] = lines;
}

static void K052109Callback(INT32 layer, INT32, INT32 *code, INT32 *color, INT32 *, INT32 *)
{
	*code |= ((*color & 0x0f) << 8);
	*code &= 0xfff;
	*color = (layer << 4) + ((*color & 0xe0) >> 5);
}

static void K051960Callback(INT32 *code, INT32 *color, INT32 *priority, INT32 *)
{
	*priority = (*color & 0x10) >> 4;

	*code &= 0x3ff;
	*color = 0x30 + (*color & 0x0f);
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	konamiOpen(0);
	konamiReset();
	konamiClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2151Reset();

	KonamiICReset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvKonROM		= Next; Next += 0x010000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x020000;
	DrvGfxROM1		= Next; Next += 0x020000;
	DrvGfxROMExp0		= Next; Next += 0x040000;
	DrvGfxROMExp1		= Next; Next += 0x040000;

	DrvPalette		= (UINT32*)Next; Next += 0x400 * sizeof(UINT32);

	AllRam			= Next;

	DrvBankRAM		= Next; Next += 0x000800;
	DrvKonRAM		= Next; Next += 0x001800;
	DrvPalRAM		= Next; Next += 0x000800;

	DrvZ80RAM		= Next; Next += 0x000800;

	soundlatch		= Next; Next += 0x000001;

	nDrvKonamiBank		= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[4] = { 0x018, 0x010, 0x008, 0x000 };
	INT32 Plane1[4] = { 0x000, 0x008, 0x010, 0x018 };
	INT32 XOffs[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			  0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107 };
	INT32 YOffs[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
			  0x200, 0x220, 0x240, 0x260, 0x280, 0x2a0, 0x2c0, 0x2e0 };

	konami_rom_deinterleave_2(DrvGfxROM0, 0x20000);
	konami_rom_deinterleave_2(DrvGfxROM1, 0x20000);

	GfxDecode(0x01000, 4,  8,  8, Plane0, XOffs, YOffs, 0x100, DrvGfxROM0, DrvGfxROMExp0);
	GfxDecode(0x00400, 4, 16, 16, Plane1, XOffs, YOffs, 0x400, DrvGfxROM1, DrvGfxROMExp1);

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
		if (BurnLoadRom(DrvKonROM  + 0x000000,  0, 1)) return 1;
	
		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  1, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  2, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001,  3, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x010000,  4, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x010001,  5, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  6, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x000001,  7, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x010000,  8, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x010001,  9, 2)) return 1;

		DrvGfxDecode();
	}

	konamiInit(1);
	konamiOpen(0);
	konamiMapMemory(DrvKonRAM,           0x4000, 0x57ff, KON_RAM);
	konamiMapMemory(DrvBankRAM,          0x5800, 0x5fff, KON_RAM);
	konamiMapMemory(DrvKonROM + 0x00000, 0x6000, 0x7fff, KON_ROM);
	konamiMapMemory(DrvKonROM + 0x08000, 0x8000, 0xffff, KON_ROM);
	konamiSetWriteHandler(blockhl_main_write);
	konamiSetReadHandler(blockhl_main_read);
	konamiSetlinesCallback(blockhl_set_lines);
	konamiClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(blockhl_sound_write);
	ZetSetReadHandler(blockhl_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(3579545, 100.0);

	K052109Init(DrvGfxROM0, 0x1ffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(8, 0);

	K051960Init(DrvGfxROM1, 0x1ffff);
	K051960SetCallback(K051960Callback);
	K051960SetSpriteOffset(8, 0);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	KonamiICExit();

	konamiExit();
	ZetExit();

	BurnYM2151Exit();

	BurnFree (AllMem);

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		KonamiRecalcPal(DrvPalRAM, DrvPalette, 0x800);
	}

	K052109UpdateScroll();

	K052109RenderLayer(2, 1, DrvGfxROMExp0);
	K051960SpritesRender(DrvGfxROMExp1, 0); 
	K052109RenderLayer(1, 0, DrvGfxROMExp0);
	K051960SpritesRender(DrvGfxROMExp1, 1);
	K052109RenderLayer(0, 0, DrvGfxROMExp0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		// Clear opposites
		if ((DrvInputs[0] & 0x03) == 0) DrvInputs[0] |= 0x03;
		if ((DrvInputs[0] & 0x0c) == 0) DrvInputs[0] |= 0x0c;
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
	}

	konamiNewFrame();
	ZetNewFrame();

	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = 100;
	INT32 nCyclesTotal[2] = { (((3000000 / 60) * 133) / 100) /* 33% overclock */, 3579545 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	ZetOpen(0);
	konamiOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nSegment = (nCyclesTotal[0] / nInterleave) * (i + 1);

		nCyclesDone[0] += konamiRun(nSegment - nCyclesDone[0]);

		nSegment = (nCyclesTotal[1] / nInterleave) * (i + 1);

		nCyclesDone[1] += ZetRun(nSegment - nCyclesDone[1]);

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (K052109_irq_enabled && (nDrvKonamiBank[0] & 0x03) == 0)
		konamiSetIrqLine(KONAMI_IRQ_LINE, KONAMI_HOLD_LINE);

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		if (nSegmentLength) {
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
		}
	}

	konamiClose();
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029705;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		konamiCpuScan(nAction, pnMin);
		ZetScan(nAction);

		BurnYM2151Scan(nAction);
		KonamiICScan(nAction);
	}

	if (nAction & ACB_WRITE) {
		konamiOpen(0);
		blockhl_set_lines(nDrvKonamiBank[0]);
		konamiClose();
	}

	return 0;
}


// Block Hole

static struct BurnRomInfo blockhlRomDesc[] = {
	{ "973l02.e21",	0x10000, 0xe14f849a, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code

	{ "973d01.g6",	0x08000, 0xeeee9d92, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "973f07.k15",	0x08000, 0x1a8cd9b4, 3 | BRF_GRA },           //  2 Background Tiles
	{ "973f08.k18",	0x08000, 0x952b51a6, 3 | BRF_GRA },           //  3
	{ "973f09.k20",	0x08000, 0x77841594, 3 | BRF_GRA },           //  4
	{ "973f10.k23",	0x08000, 0x09039fab, 3 | BRF_GRA },           //  5

	{ "973f06.k12",	0x08000, 0x51acfdb6, 4 | BRF_GRA },           //  6 Sprites
	{ "973f05.k9",	0x08000, 0x4cfea298, 4 | BRF_GRA },           //  7
	{ "973f04.k7",	0x08000, 0x69ca41bd, 4 | BRF_GRA },           //  8
	{ "973f03.k4",	0x08000, 0x21e98472, 4 | BRF_GRA },           //  9

	{ "973a11.h10",	0x00100, 0x46d28fe9, 5 | BRF_OPT },           // 10 Proms
};

STD_ROM_PICK(blockhl)
STD_ROM_FN(blockhl)

struct BurnDriver BurnDrvBlockhl = {
	"blockhl", NULL, NULL, NULL, "1989",
	"Block Hole\0", NULL, "Konami", "GX973",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_PUZZLE, 0,
	NULL, blockhlRomInfo, blockhlRomName, NULL, NULL, BlockhlInputInfo, BlockhlDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 224, 4, 3
};


// Quarth (Japan)

static struct BurnRomInfo quarthRomDesc[] = {
	{ "973j02.e21",	0x10000, 0x27a90118, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code

	{ "973d01.g6",	0x08000, 0xeeee9d92, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "973e07.k15",	0x08000, 0x0bd6b0f8, 3 | BRF_GRA },           //  2 Background Tiles
	{ "973e08.k18",	0x08000, 0x104d0d5f, 3 | BRF_GRA },           //  3
	{ "973e09.k20",	0x08000, 0xbd3a6f24, 3 | BRF_GRA },           //  4
	{ "973e10.k23",	0x08000, 0xcf5e4b86, 3 | BRF_GRA },           //  5

	{ "973e06.k12",	0x08000, 0x0d58af85, 4 | BRF_GRA },           //  6 Sprites
	{ "973e05.k9",	0x08000, 0x15d822cb, 4 | BRF_GRA },           //  7
	{ "973e04.k7",	0x08000, 0xd70f4a2c, 4 | BRF_GRA },           //  8
	{ "973e03.k4",	0x08000, 0x2c5a4b4b, 4 | BRF_GRA },           //  9

	{ "973a11.h10",	0x00100, 0x46d28fe9, 5 | BRF_OPT },           // 10 Proms
};

STD_ROM_PICK(quarth)
STD_ROM_FN(quarth)

struct BurnDriver BurnDrvQuarth = {
	"quarth", "blockhl", NULL, NULL, "1989",
	"Quarth (Japan)\0", NULL, "Konami", "GX973",
	L"Quarth\0\u30AF\u30A9\u30FC\u30B9 (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_PUZZLE, 0,
	NULL, quarthRomInfo, quarthRomName, NULL, NULL, BlockhlInputInfo, BlockhlDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 224, 4, 3
};
