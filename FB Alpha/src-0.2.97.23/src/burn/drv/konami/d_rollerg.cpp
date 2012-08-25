// FB Alpha Roller Games driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "z80_intf.h"
#include "burn_ym3812.h"
#include "konami_intf.h"
#include "konamiic.h"
#include "k053260.h"

static UINT8 *AllMem;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *MemEnd;
static UINT8 *DrvKonROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvSndROM;
static UINT8 *DrvKonRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;

static UINT32  *Palette;
static UINT32  *DrvPalette;
static UINT8  DrvRecalc;

static INT32 readzoomroms;
static UINT8 *nDrvBank;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static struct BurnInputInfo RollergInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Rollerg)

static struct BurnDIPInfo RollergDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x56, NULL			},
	{0x14, 0xff, 0xff, 0x0f, NULL			},

	{0   , 0xfe, 0   ,   16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,   16, "Coin B"		},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0xf0, 0x00, "No Credits"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x03, 0x03, "1"			},
	{0x13, 0x01, 0x03, 0x02, "2"			},
	{0x13, 0x01, 0x03, 0x01, "3"			},
	{0x13, 0x01, 0x03, 0x00, "5"			},

	{0   , 0xfe, 0   ,    4, "Bonus Energy"		},
	{0x13, 0x01, 0x18, 0x00, "1/2 for Stage Winner"	},
	{0x13, 0x01, 0x18, 0x08, "1/4 for Stage Winner"	},
	{0x13, 0x01, 0x18, 0x10, "1/4 for Cycle Winner"	},
	{0x13, 0x01, 0x18, 0x18, "None"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x60, 0x60, "Easy"			},
	{0x13, 0x01, 0x60, 0x40, "Normal"		},
	{0x13, 0x01, 0x60, 0x20, "Hard"			},
	{0x13, 0x01, 0x60, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x14, 0x01, 0x01, 0x01, "Off"			},
//	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Rollerg)

void rollerg_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x0010:
			readzoomroms = data & 0x04;
			K051316WrapEnable(0, data & 0x20);
		return;

		case 0x0020:
			// watchdog
		return;

		case 0x0030:
		case 0x0031:
			K053260Write(0, address & 1, data);
		return;

		case 0x0040:
			ZetSetVector(0xff);
		//	ZetRaiseIrq(0);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;
	}

	if ((address & 0xfff0) == 0x0200) {
		K051316WriteCtrl(0, address & 0x0f, data);
		return;
	}

	if ((address & 0xfff0) == 0x0300) {
		K053244Write(0, address & 0x0f, data);
		return;
	}

	if ((address & 0xf800) == 0x0800) {
		K051316Write(0, address & 0x7ff, data);
		return;
	}

	if ((address & 0xf800) == 0x1000) {
		K053245Write(0, address & 0x7ff, data);
		return;
	}
}

UINT8 rollerg_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x0020: // watchdog
			return 0;

		case 0x0030:
		case 0x0031:
			return K053260Read(0, (address & 1)+2);

		case 0x0050:
			return DrvInputs[0];

		case 0x0051:
			return DrvInputs[1];

		case 0x0052:
			return (DrvInputs[2] & 0xf0) | (DrvDips[2] & 0x0f);

		case 0x0053:
			return DrvDips[0];

		case 0x0060:
			return DrvDips[1];

		case 0x0061:
			return 0x7f;
	}

	if ((address & 0xfff0) == 0x0300) {
		return K053244Read(0, address & 0x0f);
	}

	if ((address & 0xf800) == 0x0800) {
		if (readzoomroms)
			return K051316ReadRom(0, address & 0x7ff);
		else
			return K051316Read(0, address & 0x7ff);
	}

	if ((address & 0xf800) == 0x1000) {
		return K053245Read(0, address & 0x7ff);
	}

	return 0;
}

void __fastcall rollerg_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xc000:
		case 0xc001:
			BurnYM3812Write(address & 1, data);
		return;

		case 0xfc00:
			ZetRun(100);
			ZetNmi();
		return;
	}

	if (address >= 0xa000 && address <= 0xa02f) {
		K053260Write(0, address & 0x3f, data);
		return;
	}
}

UINT8 __fastcall rollerg_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xc000:
		case 0xc001:
			return BurnYM3812Read(address & 1);
	}

	if (address >= 0xa000 && address <= 0xa02f) {
		// not sure...
		if ((address & 0x3e) == 0x00) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);

		return K053260Read(0, address & 0x3f);
	}

	return 0;
}

static void rollerg_set_lines(INT32 lines)
{
	nDrvBank[0] = lines;

	INT32 offs = 0x10000 + ((lines & 0x07) * 0x4000);

	konamiMapMemory(DrvKonROM + offs, 0x4000, 0x7fff, KON_ROM); 
}

static void K053245Callback(INT32 *, INT32 *color, INT32 *priority)
{
	*priority = *color & 0x10;
	*color = 16 + (*color & 0x0f);
}

static void K051316Callback(INT32 *code, INT32 *color, INT32 *flags)
{
	*flags =  (*color & 0xc0) >> 6;
	*code |= ((*color & 0x0f) << 8);
	*color = ((*color & 0x30) >> 4);
}

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 3579545;
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

	K053260Reset(0);
	BurnYM3812Reset();

	KonamiICReset();

	readzoomroms = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvKonROM		= Next; Next += 0x030000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x200000;
	DrvGfxROM1		= Next; Next += 0x080000;
	DrvGfxROMExp0		= Next; Next += 0x400000;
	DrvGfxROMExp1		= Next; Next += 0x100000;

	DrvSndROM		= Next; Next += 0x080000;

	Palette			= (UINT32*)Next; Next += 0x400 * sizeof(UINT32);
	DrvPalette		= (UINT32*)Next; Next += 0x400 * sizeof(UINT32);

	AllRam			= Next;

	DrvKonRAM		= Next; Next += 0x001b00;
	DrvPalRAM		= Next; Next += 0x000800;

	DrvZ80RAM		= Next; Next += 0x000800;

	nDrvBank		= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

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
		if (BurnLoadRom(DrvKonROM  + 0x010000,  0, 1)) return 1;
		memcpy (DrvKonROM + 0x08000, DrvKonROM + 0x28000, 0x8000);

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  1, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  2, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x100000,  3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040000,  5, 1)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000,  6, 1)) return 1;

		konami_rom_deinterleave_2(DrvGfxROM0, 0x200000);

		K053245GfxDecode(DrvGfxROM0, DrvGfxROMExp0, 0x200000);
	}

	konamiInit(1);
	konamiOpen(0);
	konamiMapMemory(DrvPalRAM,	    0x1800, 0x1fff, KON_RAM);
	konamiMapMemory(DrvKonRAM,          0x2000, 0x3aff, KON_RAM);
	konamiMapMemory(DrvKonROM + 0x4000, 0x4000, 0x7fff, KON_ROM);
	konamiMapMemory(DrvKonROM + 0x8000, 0x8000, 0xffff, KON_ROM);
	konamiSetWriteHandler(rollerg_main_write);
	konamiSetReadHandler(rollerg_main_read);
	konamiSetlinesCallback(rollerg_set_lines);
	konamiClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(rollerg_sound_write);
	ZetSetReadHandler(rollerg_sound_read);
	ZetMemEnd();
	ZetClose();

	K053245Init(0, DrvGfxROM0, 0x1fffff, K053245Callback);
	K053245SetSpriteOffset(0, -112, 17);

	K051316Init(0, DrvGfxROM1, DrvGfxROMExp1, 0x7ffff, K051316Callback, 4, 0);
	K051316SetOffset(0, -90, -15);

	BurnYM3812Init(3579545, NULL, DrvSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(3579545);
	BurnYM3812SetRoute(BURN_SND_YM3812_ROUTE, 1.00, BURN_SND_ROUTE_BOTH);

	K053260Init(0, 3579545, DrvSndROM, 0x80000);
	K053260PCMSetAllRoutes(0, 0.70, BURN_SND_ROUTE_BOTH);

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

	BurnYM3812Exit();
	K053260Exit();

	BurnFree (AllMem);

	return 0;
}

static void DrvRecalcPal()
{
	UINT8 r,g,b;
	UINT16 *p = (UINT16*)DrvPalRAM;
	for (INT32 i = 0; i < 0x800 / 2; i++) {
		UINT16 d = (p[i] << 8) | (p[i] >> 8);

		b = (d >> 10) & 0x1f;
		g = (d >>  5) & 0x1f;
		r = (d >>  0) & 0x1f;

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
		Palette[i] = (r << 16) | (g << 8) | b;
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvRecalcPal();
	}

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x100;
	}

	K053245SpritesRender(0, DrvGfxROMExp0, 0x00);
	K051316_zoom_draw(0, 1);
	K053245SpritesRender(0, DrvGfxROMExp0, 0x10);

	KonamiBlendCopy(Palette, DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();
	konamiNewFrame();

	{
		memset (DrvInputs, 0xff, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		// Clear opposites
		if ((DrvInputs[0] & 0x18) == 0) DrvInputs[0] |= 0x18;
		if ((DrvInputs[0] & 0x60) == 0) DrvInputs[0] |= 0x60;
		if ((DrvInputs[1] & 0x18) == 0) DrvInputs[1] |= 0x18;
		if ((DrvInputs[1] & 0x60) == 0) DrvInputs[1] |= 0x60;
	}

	INT32 nInterleave = nBurnSoundLen;
	INT32 nCyclesTotal[2] = { 3000000 / 60, 3579545 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	ZetOpen(0);
	konamiOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nNext, nCyclesSegment;

		nNext = (i + 1) * nCyclesTotal[0] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[0];
		nCyclesSegment = konamiRun(nCyclesSegment);
		nCyclesDone[0] += nCyclesSegment;

		BurnTimerUpdateYM3812(i * (nCyclesTotal[1] / nInterleave));
	}

	konamiSetIrqLine(KONAMI_IRQ_LINE, KONAMI_HOLD_LINE);
	
	BurnTimerEndFrameYM3812(nCyclesTotal[1]);

	if (pBurnSoundOut) {
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		K053260Update(0, pBurnSoundOut, nBurnSoundLen);
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

		BurnYM3812Scan(nAction, pnMin);
		K053260Scan(nAction);

		KonamiICScan(nAction);

		SCAN_VAR(readzoomroms);
	}

	if (nAction & ACB_WRITE) {
		konamiOpen(0);
		rollerg_set_lines(nDrvBank[0]);
		konamiClose();
	}

	return 0;
}


// Rollergames (US)

static struct BurnRomInfo rollergRomDesc[] = {
	{ "999m02.g7",	0x020000, 0x3df8db93, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code

	{ "999m01.e11",	0x008000, 0x1fcfb22f, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "999h06.k2",	0x100000, 0xeda05130, 3 | BRF_GRA },           //  2 K053245 Tiles
	{ "999h05.k8",	0x100000, 0x5f321c7d, 3 | BRF_GRA },           //  3

	{ "999h03.d23",	0x040000, 0xea1edbd2, 4 | BRF_GRA },           //  4 K051316 Tiles
	{ "999h04.f23",	0x040000, 0xc1a35355, 4 | BRF_GRA },           //  5

	{ "999h09.c5",	0x080000, 0xc5188783, 5 | BRF_SND },           //  6 K053260 Samples
};

STD_ROM_PICK(rollerg)
STD_ROM_FN(rollerg)

struct BurnDriver BurnDrvRollerg = {
	"rollerg", NULL, NULL, NULL, "1991",
	"Rollergames (US)\0", NULL, "Konami", "GX999",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, rollergRomInfo, rollergRomName, NULL, NULL, RollergInputInfo, RollergDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 224, 4, 3
};


// Rollergames (Japan)

static struct BurnRomInfo rollergjRomDesc[] = {
	{ "999v02.bin",	0x020000, 0x0dd8c3ac, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code

	{ "999m01.e11",	0x008000, 0x1fcfb22f, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 Code

	{ "999h06.k2",	0x100000, 0xeda05130, 3 | BRF_GRA },           //  2 K053245 Tiles
	{ "999h05.k8",	0x100000, 0x5f321c7d, 3 | BRF_GRA },           //  3

	{ "999h03.d23",	0x040000, 0xea1edbd2, 4 | BRF_GRA },           //  4 K051316 Tiles
	{ "999h04.f23",	0x040000, 0xc1a35355, 4 | BRF_GRA },           //  5

	{ "999h09.c5",	0x080000, 0xc5188783, 5 | BRF_SND },           //  6 K053260 Samples
};

STD_ROM_PICK(rollergj)
STD_ROM_FN(rollergj)

struct BurnDriver BurnDrvRollergj = {
	"rollergj", "rollerg", NULL, NULL, "1991",
	"Rollergames (Japan)\0", NULL, "Konami", "GX999",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, rollergjRomInfo, rollergjRomName, NULL, NULL, RollergInputInfo, RollergDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 224, 4, 3
};
