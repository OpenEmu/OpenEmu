// FB Alpha Gang Busters driver module
// Based on MAME by Manuel Abadia

#include "tiles_generic.h"
#include "zet.h"
#include "konami_intf.h"
#include "konamiic.h"
#include "burn_ym2151.h"
#include "k007232.h"

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
static UINT8 *DrvSndROM;
static UINT8 *DrvBankRAM;
static UINT8 *DrvKonRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;
static UINT32  *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *nDrvRamBank;
static UINT8 *nDrvKonamiBank;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static struct BurnInputInfo GbustersInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"	},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Gbusters)

static struct BurnDIPInfo GbustersDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x56, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,   16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits" 	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"	  },
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

	{0   , 0xfe, 0   ,    15, "Coin B"		},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x03, 0x03, "2"			},
	{0x13, 0x01, 0x03, 0x02, "3"			},
	{0x13, 0x01, 0x03, 0x01, "5"			},
	{0x13, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "Bullets"		},
	{0x13, 0x01, 0x04, 0x04, "50"			},
	{0x13, 0x01, 0x04, 0x00, "60"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0x18, 0x18, "50k, 200k & 400k"	},
	{0x13, 0x01, 0x18, 0x10, "70k, 250k & 500k"	},
	{0x13, 0x01, 0x18, 0x08, "50k"			},
	{0x13, 0x01, 0x18, 0x00, "70k"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x60, 0x60, "Easy"			},
	{0x13, 0x01, 0x60, 0x40, "Normal"		},
	{0x13, 0x01, 0x60, 0x20, "Difficult"		},
	{0x13, 0x01, 0x60, 0x00, "Very Difficult"	},

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

STDDIPINFO(Gbusters)

static void set_ram_bank(INT32 data)
{
	nDrvRamBank[0] = data;

	if (~data & 0x01) {
		konamiMapMemory(DrvPalRAM,  0x5800, 0x5fff, KON_RAM);
	} else {
		konamiMapMemory(DrvBankRAM, 0x5800, 0x5fff, KON_RAM);
	}
}

void gbusters_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x1f80:
			set_ram_bank(data);
		return;

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

		case 0x1f98:
			K052109RMRDLine = data & 0x01;
		return;
	}

	if ((address & 0xc000) == 0x0000) {
		K052109_051960_w(address & 0x3fff, data);
		return;
	}
}

UINT8 gbusters_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x1f90:
			return DrvInputs[0];

		case 0x1f91:
			return DrvInputs[1];

		case 0x1f92:
			return DrvInputs[2];

		case 0x1f93:
			return DrvDips[2];

		case 0x1f94:
			return DrvDips[0];

		case 0x1f95:
			return DrvDips[1];
	}

	if ((address & 0xc000) == 0x0000) {
		return K052109_051960_r(address & 0x3fff);
	}

	return 0;
}

static void gbusters_snd_bankswitch_w(INT32 data)
{
	INT32 bank_B = ((data >> 2) & 0x01);
	INT32 bank_A = ((data) & 0x01);
	k007232_set_bank(0, bank_A, bank_B );
}

void __fastcall gbusters_sound_write(UINT16 address, UINT8 data)
{
	if ((address & 0xfff0) == 0xb000) {
		K007232WriteReg(0, address & 0x0f, data);
		return;
	}

	switch (address)
	{
		case 0xc000:
			BurnYM2151SelectRegister(data);
		return;

		case 0xc001:
			BurnYM2151WriteRegister(data);
		return;

		case 0xf000:
			gbusters_snd_bankswitch_w(data);
		return;
	}
}

UINT8 __fastcall gbusters_sound_read(UINT16 address)
{
	if ((address & 0xfff0) == 0xb000) {
		return K007232ReadReg(0, address & 0x0f);
	}

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

static void gbusters_set_lines(INT32 lines)
{
	INT32 nBank = 0x10000 + (lines & 0x0f) * 0x2000;

	nDrvKonamiBank[1] = lines;

	konamiMapMemory(DrvKonROM + nBank, 0x6000, 0x7fff, KON_ROM); 
}

static void DrvK007232VolCallback(INT32 v)
{
	K007232SetVolume(0, 0, (v >> 0x4) * 0x11, 0);
	K007232SetVolume(0, 1, 0, (v & 0x0f) * 0x11);
}

static void K052109Callback(INT32 layer, INT32 bank, INT32 *code, INT32 *color, INT32 *, INT32 *)
{
	INT32 layer_colorbase[3] = { 0x30, 0x00, 0x10 };

	*code |= ((*color & 0x0d) << 8) | ((*color & 0x10) << 5) | (bank << 12);
	*code &= 0x3fff;
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

static void K051960Callback(INT32 *code, INT32 *color, INT32 *priority, INT32 *)
{
	*priority = (*color & 0x30) >> 4;
	*color = 0x20 + (*color & 0x0f);
	*code &= 0x0fff;
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

	DrvKonROM		= Next; Next += 0x030000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x080000;
	DrvGfxROM1		= Next; Next += 0x080000;
	DrvGfxROMExp0		= Next; Next += 0x100000;
	DrvGfxROMExp1		= Next; Next += 0x100000;

	DrvSndROM		= Next; Next += 0x040000;

	DrvPalette		= (UINT32*)Next; Next += 0x400 * sizeof(UINT32);

	AllRam			= Next;

	DrvBankRAM		= Next; Next += 0x000800;
	DrvKonRAM		= Next; Next += 0x001800;
	DrvPalRAM		= Next; Next += 0x000800;

	DrvZ80RAM		= Next; Next += 0x000800;

	soundlatch		= Next; Next += 0x000001;

	nDrvRamBank		= Next; Next += 0x000001;
	nDrvKonamiBank		= Next; Next += 0x000002;

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

	konami_rom_deinterleave_2(DrvGfxROM0, 0x80000);
	konami_rom_deinterleave_2(DrvGfxROM1, 0x80000);

	GfxDecode(0x04000, 4,  8,  8, Plane0, XOffs, YOffs, 0x100, DrvGfxROM0, DrvGfxROMExp0);
	GfxDecode(0x01000, 4, 16, 16, Plane1, XOffs, YOffs, 0x400, DrvGfxROM1, DrvGfxROMExp1);

	return 0;
}

static INT32 DrvInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)malloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvKonROM  + 0x000000,  0, 1)) return 1;
		if (BurnLoadRom(DrvKonROM  + 0x020000,  1, 1)) return 1;
		memcpy (DrvKonROM + 0x10000, DrvKonROM + 0x00000, 0x08000);
		memcpy (DrvKonROM + 0x18000, DrvKonROM + 0x00000, 0x08000);

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x040000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040000,  6, 1)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000,  7, 1)) return 1;

		DrvGfxDecode();
	}

	konamiInit(1);
	konamiOpen(0);
	konamiMapMemory(DrvKonRAM,           0x4000, 0x57ff, KON_RAM);
	konamiMapMemory(DrvBankRAM,          0x5800, 0x5fff, KON_RAM);
	konamiMapMemory(DrvKonROM + 0x10000, 0x6000, 0x7fff, KON_ROM);
	konamiMapMemory(DrvKonROM + 0x08000, 0x8000, 0xffff, KON_ROM);
	konamiSetWriteHandler(gbusters_main_write);
	konamiSetReadHandler(gbusters_main_read);
	konamiSetlinesCallback(gbusters_set_lines);
	konamiClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(gbusters_sound_write);
	ZetSetReadHandler(gbusters_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(3579545, 100.0);

	K007232Init(0, 3579545, DrvSndROM, 0x40000);
	K007232SetPortWriteHandler(0, DrvK007232VolCallback);

	K052109Init(DrvGfxROM0, 0x7ffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(8, 0);

	K051960Init(DrvGfxROM1, 0x7ffff);
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

	K007232Exit();
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

	if (nDrvRamBank[0] & 0x08)
	{

		if (nBurnLayer & 1) K052109RenderLayer(2, 1, DrvGfxROMExp0);
		K051960SpritesRender(DrvGfxROMExp1, 2);
		if (nBurnLayer & 2) K052109RenderLayer(1, 0, DrvGfxROMExp0);
	}
	else
	{
		if (nBurnLayer & 4) K052109RenderLayer(1, 1, DrvGfxROMExp0);
		K051960SpritesRender(DrvGfxROMExp1, 2);
		if (nBurnLayer & 8) K052109RenderLayer(2, 0, DrvGfxROMExp0);
	}

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
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
		if ((DrvInputs[2] & 0x03) == 0) DrvInputs[2] |= 0x03;
		if ((DrvInputs[2] & 0x0c) == 0) DrvInputs[2] |= 0x0c;
	}

	konamiNewFrame();
	ZetNewFrame();

	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = nBurnSoundLen;
	INT32 nCyclesTotal[2] = { 3000000 / 60, 3579545 / 60 };
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
			K007232Update(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (K052109_irq_enabled) konamiSetIrqLine(KONAMI_IRQ_LINE, KONAMI_HOLD_LINE);

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		if (nSegmentLength) {
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K007232Update(0, pSoundBuf, nSegmentLength);
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
		*pnMin = 0x029704;
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
		K007232Scan(nAction, pnMin);

		KonamiICScan(nAction);
	}

	if (nAction & ACB_WRITE) {
		konamiOpen(0);
		set_ram_bank(nDrvRamBank[0]);
		gbusters_set_lines(nDrvKonamiBank[1]);
		konamiClose();
	}

	return 0;
}


// Gang Busters (set 1)

static struct BurnRomInfo gbustersRomDesc[] = {
	{ "878n02.k13",	0x10000, 0x51697aaa, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "878j03.k15",	0x10000, 0x3943a065, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "878h01.f8",	0x08000, 0x96feafaa, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "878c07.h27",	0x40000, 0xeeed912c, 3 | BRF_GRA },           //  3 Background Tiles
	{ "878c08.k27",	0x40000, 0x4d14626d, 3 | BRF_GRA },           //  4

	{ "878c05.h5",	0x40000, 0x01f4aea5, 4 | BRF_GRA },           //  5 Sprites
	{ "878c06.k5",	0x40000, 0xedfaaaaf, 4 | BRF_GRA },           //  6

	{ "878c04.d5",	0x40000, 0x9e982d1c, 5 | BRF_SND },           //  7 K007232 Samples

	{ "878a09.f20",	0x00100, 0xe2d09a1b, 6 | BRF_OPT },           //  8 Proms
};

STD_ROM_PICK(gbusters)
STD_ROM_FN(gbusters)

struct BurnDriver BurnDrvGbusters = {
	"gbusters", NULL, NULL, NULL, "1988",
	"Gang Busters (set 1)\0", NULL, "Konami", "GX878",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_SHOOT, 0,
	NULL, gbustersRomInfo, gbustersRomName, NULL, NULL, GbustersInputInfo, GbustersDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	224, 288, 3, 4
};


// Gang Busters (set 2)

static struct BurnRomInfo gbusteraRomDesc[] = {
	{ "878_02.k13",	0x10000, 0x57178414, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "878_03.k15",	0x10000, 0x6c59e660, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "878h01.f8",	0x08000, 0x96feafaa, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "878c07.h27",	0x40000, 0xeeed912c, 3 | BRF_GRA },           //  3 Background Tiles
	{ "878c08.k27",	0x40000, 0x4d14626d, 3 | BRF_GRA },           //  4

	{ "878c05.h5",	0x40000, 0x01f4aea5, 4 | BRF_GRA },           //  5 Sprites
	{ "878c06.k5",	0x40000, 0xedfaaaaf, 4 | BRF_GRA },           //  6

	{ "878c04.d5",	0x40000, 0x9e982d1c, 5 | BRF_SND },           //  7 K007232 Samples

	{ "878a09.f20",	0x00100, 0xe2d09a1b, 6 | BRF_OPT },           //  8 Proms
};

STD_ROM_PICK(gbustera)
STD_ROM_FN(gbustera)

struct BurnDriver BurnDrvGbustera = {
	"gbustersa", "gbusters", NULL, NULL, "1988",
	"Gang Busters (set 2)\0", NULL, "Konami", "GX878",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_SHOOT, 0,
	NULL, gbusteraRomInfo, gbusteraRomName, NULL, NULL, GbustersInputInfo, GbustersDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	224, 288, 3, 4
};


// Crazy Cop (Japan)

static struct BurnRomInfo crazycopRomDesc[] = {
	{ "878m02.k13",	0x10000, 0x9c1c9f52, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "878j03.k15",	0x10000, 0x3943a065, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "878h01.f8",	0x08000, 0x96feafaa, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "878c07.h27",	0x40000, 0xeeed912c, 3 | BRF_GRA },           //  3 Background Tiles
	{ "878c08.k27",	0x40000, 0x4d14626d, 3 | BRF_GRA },           //  4

	{ "878c05.h5",	0x40000, 0x01f4aea5, 4 | BRF_GRA },           //  5 Sprites
	{ "878c06.k5",	0x40000, 0xedfaaaaf, 4 | BRF_GRA },           //  6

	{ "878c04.d5",	0x40000, 0x9e982d1c, 5 | BRF_SND },           //  7 K007232 Samples

	{ "878a09.f20",	0x00100, 0xe2d09a1b, 6 | BRF_OPT },           //  8 Proms
};

STD_ROM_PICK(crazycop)
STD_ROM_FN(crazycop)

struct BurnDriver BurnDrvCrazycop = {
	"crazycop", "gbusters", NULL, NULL, "1988",
	"Crazy Cop (Japan)\0", NULL, "Konami", "GX878",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_SHOOT, 0,
	NULL, crazycopRomInfo, crazycopRomName, NULL, NULL, GbustersInputInfo, GbustersDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	224, 288, 3, 4
};
