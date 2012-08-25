// FB Alpha Surprise Attack driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "burn_ym2151.h"
#include "konami_intf.h"
#include "konamiic.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvKonROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvBankRAM;
static UINT8 *DrvKonRAM;
static UINT8 *DrvPalRAM;

static UINT32  *Palette;
static UINT32  *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *nDrvRomBank;
static INT32 videobank;

static INT32 layer_colorbase[3];
static INT32 sprite_colorbase;
static INT32 layerpri[3];

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvReset;
static UINT8 DrvInputs[3];

static struct BurnInputInfo SurpratkInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 0,	"service"	},
	{"Test",		BIT_DIGITAL,	DrvJoy3 + 1,	"diag"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Surpratk)

static struct BurnDIPInfo SuratkjDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0x5a, NULL			},
	{0x15, 0xff, 0xff, 0xf0, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x13, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 	},
	{0x13, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 	},
	{0x13, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 	},
	{0x13, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 	},
	{0x13, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x13, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x13, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x13, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x13, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x13, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    15, "Coin B"		},
	{0x13, 0x01, 0xf0, 0x20, "4 Coins 1 Credit" 	},
	{0x13, 0x01, 0xf0, 0x50, "3 Coins 1 Credit" 	},
	{0x13, 0x01, 0xf0, 0x80, "2 Coins 1 Credit" 	},
	{0x13, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit" 	},
	{0x13, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x13, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x13, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x13, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x13, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x03, 0x03, "2"			},
	{0x14, 0x01, 0x03, 0x02, "3"			},
	{0x14, 0x01, 0x03, 0x01, "5"			},
	{0x14, 0x01, 0x03, 0x00, "7"			},

//	{0   , 0xfe, 0   ,    2, "Cabinet"		},
//	{0x14, 0x01, 0x04, 0x00, "Upright"		},
//	{0x14, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x60, 0x60, "Easy"			},
	{0x14, 0x01, 0x60, 0x40, "Normal"		},
	{0x14, 0x01, 0x60, 0x20, "Difficult"		},
	{0x14, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x80, 0x80, "Off"			},
	{0x14, 0x01, 0x80, 0x00, "On"			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x15, 0x01, 0x10, 0x10, "Off"			},
//	{0x15, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Upright Controls"	},
	{0x15, 0x01, 0x20, 0x20, "Single"		},
	{0x15, 0x01, 0x20, 0x00, "Dual"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x15, 0x01, 0x40, 0x40, "Off"			},
	{0x15, 0x01, 0x40, 0x00, "On"			},
};

static struct BurnDIPInfo BonusQuizDIPList[]=
{
	{0   , 0xfe, 0   ,    2, "Bonus Quiz"		},
	{0x15, 0x01, 0x80, 0x80, "Include"			},
	{0x15, 0x01, 0x80, 0x00, "Except"			},
};

STDDIPINFO(Suratkj)
STDDIPINFOEXT(Surpratk, Suratkj, BonusQuiz)

UINT8 supratk_read(UINT16 address)
{
	switch (address)
	{
		case 0x5f8c:
			return DrvInputs[0];

		case 0x5f8d:
			return DrvInputs[1];

		case 0x5f8e:
			return (DrvInputs[2] & 0x0f) | (DrvDips[2] & 0xf0);

		case 0x5f8f:
			return DrvDips[0];

		case 0x5f90:
			return DrvDips[1];

		case 0x5fc0:
			// watchdog
			return 0;
	}

	if ((address & 0xf800) == 0x0000) {
		if (videobank & 0x02) {
			return DrvPalRAM[((videobank & 4) << 9) + address];
		} else if (videobank & 0x01) {
			return K053245Read(0, address);
		}
		return DrvBankRAM[address];
	}

	if ((address & 0xfff0) == 0x5fa0) {
		return K053244Read(0, address & 0x0f);
	}

	if ((address & 0xc000) == 0x4000) {
		return K052109Read(address & 0x3fff);
	}

	return 0;
}

void supratk_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x5fc0:
			K052109RMRDLine = data & 0x08;
		return;

		case 0x5fc4:
			videobank = data;
		return;

		case 0x5fd0:
			BurnYM2151SelectRegister(data);
		return;

		case 0x5fd1:
			BurnYM2151WriteRegister(data);
		return;
	}

	if ((address & 0xf800) == 0x0000) {
		if (videobank & 0x02) {
			DrvPalRAM[((videobank & 4) << 9) + address] = data;
			return;
		} else if (videobank & 0x01) {
			K053245Write(0, address, data);
			return;
		}
		DrvBankRAM[address] = data;
		return;
	}

	if ((address & 0xfff0) == 0x5fa0) {
		K053244Write(0, address & 0x0f, data);
		return;
	}

	if ((address & 0xfff0) == 0x5fb0) {
		K053251Write(address & 0x0f, data);
		return;
	}

	if ((address & 0xc000) == 0x4000) {
		K052109Write(address & 0x3fff, data);
		return;
	}
}

static void K052109Callback(INT32 layer, INT32 bank, INT32 *code, INT32 *color, INT32 *flags, INT32 *)
{
	*flags = (*color & 0x80) >> 7;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0x60) >> 5);
}

static void K053245Callback(INT32 *code, INT32 *color, INT32 *priority)
{
	INT32 pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= layerpri[2])                           *priority = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1]) *priority = 1;
	else if (pri > layerpri[1] && pri <= layerpri[0]) *priority = 2;
	else 	                                          *priority = 3;

	*code &= 0xfff;
	*color = sprite_colorbase + (*color & 0x1f);
}

static void supratk_set_lines(INT32 lines)
{
	nDrvRomBank[0] = lines;
	konamiMapMemory(DrvKonROM + 0x10000 + ((lines & 0x1f) * 0x2000), 0x2000, 0x3fff, KON_ROM); 
}

static void DrvYM2151IRQHandler(INT32 nStatus)
{
	konamiSetIrqLine(KONAMI_FIRQ_LINE, nStatus ? 3 /* use 3 as ACK */ : 0);
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	konamiOpen(0);
	konamiReset();
	konamiClose();

	BurnYM2151Reset();

	KonamiICReset();

	videobank = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvGfxROM0		= Next; Next += 0x080000;
	DrvGfxROMExp0		= Next; Next += 0x100000;
	DrvGfxROM1		= Next; Next += 0x080000;
	DrvGfxROMExp1		= Next; Next += 0x100000;

	DrvKonROM		= Next; Next += 0x050000;

	Palette			= (UINT32*)Next; Next += 0x800 * sizeof(UINT32);
	DrvPalette		= (UINT32*)Next; Next += 0x800 * sizeof(UINT32);

	AllRam			= Next;

	DrvBankRAM		= Next; Next += 0x000800;
	DrvKonRAM		= Next; Next += 0x001800;
	DrvPalRAM		= Next; Next += 0x001000;

	nDrvRomBank		= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4] = { 0x018, 0x010, 0x008, 0x000 };
	INT32 XOffs[8] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007 };
	INT32 YOffs[8] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0 };

	konami_rom_deinterleave_2(DrvGfxROM0, 0x080000);
	konami_rom_deinterleave_2(DrvGfxROM1, 0x080000);

	GfxDecode(0x4000, 4, 8, 8, Plane, XOffs, YOffs, 0x100, DrvGfxROM0, DrvGfxROMExp0);

	K053245GfxDecode(DrvGfxROM1, DrvGfxROMExp1, 0x80000);

	return 0;
}

static INT32 DrvInit()
{
	AllMem = NULL;
	MemIndex();
	int nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvKonROM  + 0x010000,  0, 1)) return 1;
		if (BurnLoadRom(DrvKonROM  + 0x030000,  1, 1)) return 1;
		memcpy (DrvKonROM + 0x08000, DrvKonROM + 0x48000, 0x8000);

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  2, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x040000,  3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040000,  5, 1)) return 1;

		DrvGfxDecode();
	}

	konamiInit(1);
	konamiOpen(0);
	konamiMapMemory(DrvKonRAM,           0x0800, 0x1fff, KON_RAM);
	konamiMapMemory(DrvKonROM + 0x10000, 0x2000, 0x3fff, KON_ROM);
	konamiMapMemory(DrvKonROM + 0x08000, 0x8000, 0xffff, KON_ROM);
	konamiSetWriteHandler(supratk_write);
	konamiSetReadHandler(supratk_read);
	konamiSetlinesCallback(supratk_set_lines);
	konamiClose();

	K052109Init(DrvGfxROM0, 0x7ffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(8, 0);

	K053245Init(0, DrvGfxROM1, 0x7ffff, K053245Callback);
	K053245SetSpriteOffset(0, -112, 16);

	BurnYM2151Init(3579545);
	YM2151SetIrqHandler(0, &DrvYM2151IRQHandler);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	KonamiICExit();
	K053245Exit();

	konamiExit();

	BurnYM2151Exit();

	BurnFree (AllMem);

	return 0;
}

static void DrvRecalcPal()
{
	UINT8 r,g,b;
	UINT16 *p = (UINT16*)DrvPalRAM;
	for (INT32 i = 0; i < 0x1000 / 2; i++) {
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

// stolen directly from mame
static void sortlayers(INT32 *layer,INT32 *pri)
{
#define SWAP(a,b) \
	if (pri[a] < pri[b]) \
	{ \
		INT32 t; \
		t = pri[a]; pri[a] = pri[b]; pri[b] = t; \
		t = layer[a]; layer[a] = layer[b]; layer[b] = t; \
	}

	SWAP(0,1)
	SWAP(0,2)
	SWAP(1,2)
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvRecalcPal();
	}

	K052109UpdateScroll();

	INT32 bg_colorbase, layer[3];

	bg_colorbase       = K053251GetPaletteIndex(0);
	sprite_colorbase   = K053251GetPaletteIndex(1);
	layer_colorbase[0] = K053251GetPaletteIndex(2);
	layer_colorbase[1] = K053251GetPaletteIndex(4);
	layer_colorbase[2] = K053251GetPaletteIndex(3);

	layerpri[0] = K053251GetPriority(2);
	layerpri[1] = K053251GetPriority(4);
	layerpri[2] = K053251GetPriority(3);
	layer[0] = 0;
	layer[1] = 1;
	layer[2] = 2;

	sortlayers(layer,layerpri);

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 16 * bg_colorbase;
	}

	if (nBurnLayer & 1) K052109RenderLayer(layer[0], 0, DrvGfxROMExp0);
	if (nSpriteEnable & 4) K053245SpritesRender(0, DrvGfxROMExp1, 2); // right?
	if (nSpriteEnable & 8) K053245SpritesRender(0, DrvGfxROMExp1, 3);
	if (nBurnLayer & 2) K052109RenderLayer(layer[1], 0, DrvGfxROMExp0);
	if (nSpriteEnable & 2) K053245SpritesRender(0, DrvGfxROMExp1, 1);
	if (nBurnLayer & 4) K052109RenderLayer(layer[2], 0, DrvGfxROMExp0);

	if (nSpriteEnable & 1) K053245SpritesRender(0, DrvGfxROMExp1, 0); // used?

	KonamiBlendCopy(Palette, DrvPalette);

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

	  // Clear Opposites
		if ((DrvInputs[0] & 0x18) == 0) DrvInputs[0] |= 0x18;
		if ((DrvInputs[0] & 0x06) == 0) DrvInputs[0] |= 0x06;
		if ((DrvInputs[1] & 0x18) == 0) DrvInputs[1] |= 0x18;
		if ((DrvInputs[1] & 0x06) == 0) DrvInputs[1] |= 0x06;
	}
	
	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = 10;
	INT32 nCyclesTotal = (((3000000 / 60) * 133) / 100); // 33% overclock
	INT32 nCyclesDone = 0;

	konamiOpen(0);
	
	for (INT32 i = 0; i < nInterleave; i++)	{
		INT32 nSegment = (nCyclesTotal / nInterleave) * (i + 1);

		nCyclesDone += konamiRun(nSegment - nCyclesDone);
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}	
	}

	if (K052109_irq_enabled) konamiSetIrqLine(KONAMI_IRQ_LINE, KONAMI_HOLD_LINE);

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		if (nSegmentLength) {
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
		}
	}

	konamiClose();

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

		BurnYM2151Scan(nAction);

		KonamiICScan(nAction);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(videobank);
	}

	if (nAction & ACB_WRITE) {
		konamiOpen(0);
		supratk_set_lines(nDrvRomBank[0]);
		konamiClose();
	}

	return 0;
}


// Surprise Attack (World ver. K)

static struct BurnRomInfo suratkRomDesc[] = {
	{ "911j01.f5",	0x20000, 0x1e647881, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "911k02.h5",	0x20000, 0xef10e7b6, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "911d05.bin",	0x40000, 0x308d2319, 2 | BRF_GRA },           //  2 Background Tiles
	{ "911d06.bin",	0x40000, 0x91cc9b32, 2 | BRF_GRA },           //  3

	{ "911d03.bin",	0x40000, 0xe34ff182, 3 | BRF_GRA },           //  4 Sprites
	{ "911d04.bin",	0x40000, 0x20700bd2, 3 | BRF_GRA },           //  5
};

STD_ROM_PICK(suratk)
STD_ROM_FN(suratk)

struct BurnDriver BurnDrvSuratk = {
	"suratk", NULL, NULL, NULL, "1990",
	"Surprise Attack (World ver. K)\0", NULL, "Konami", "GX911",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT | GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, suratkRomInfo, suratkRomName, NULL, NULL, SurpratkInputInfo, SurpratkDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// Surprise Attack (Asia ver. L)

static struct BurnRomInfo suratkaRomDesc[] = {
	{ "911j01.f5",	0x20000, 0x1e647881, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "911l02.h5",	0x20000, 0x11db8288, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "911d05.bin",	0x40000, 0x308d2319, 2 | BRF_GRA },           //  2 Background Tiles
	{ "911d06.bin",	0x40000, 0x91cc9b32, 2 | BRF_GRA },           //  3

	{ "911d03.bin",	0x40000, 0xe34ff182, 3 | BRF_GRA },           //  4 Sprites
	{ "911d04.bin",	0x40000, 0x20700bd2, 3 | BRF_GRA },           //  5
};

STD_ROM_PICK(suratka)
STD_ROM_FN(suratka)

struct BurnDriver BurnDrvSuratka = {
	"suratka", "suratk", NULL, NULL, "1990",
	"Surprise Attack (Asia ver. L)\0", NULL, "Konami", "GX911",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT | GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, suratkaRomInfo, suratkaRomName, NULL, NULL, SurpratkInputInfo, SurpratkDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// Surprise Attack (Japan ver. M)

static struct BurnRomInfo suratkjRomDesc[] = {
	{ "911m01.f5",	0x20000, 0xee5b2cc8, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "911m02.h5",	0x20000, 0x5d4148a8, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "911d05.bin",	0x40000, 0x308d2319, 2 | BRF_GRA },           //  2 Background Tiles
	{ "911d06.bin",	0x40000, 0x91cc9b32, 2 | BRF_GRA },           //  3

	{ "911d03.bin",	0x40000, 0xe34ff182, 3 | BRF_GRA },           //  4 Sprites
	{ "911d04.bin",	0x40000, 0x20700bd2, 3 | BRF_GRA },           //  5
};

STD_ROM_PICK(suratkj)
STD_ROM_FN(suratkj)

struct BurnDriver BurnDrvSuratkj = {
	"suratkj", "suratk", NULL, NULL, "1990",
	"Surprise Attack (Japan ver. M)\0", NULL, "Konami", "GX911",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT | GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, suratkjRomInfo, suratkjRomName, NULL, NULL, SurpratkInputInfo, SuratkjDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};
