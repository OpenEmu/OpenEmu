// FB Alpha Parodius Da! driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "zet.h"
#include "burn_ym2151.h"
#include "konami_intf.h"
#include "konamiic.h"
#include "k053260.h"

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

static UINT32  *Palette;
static UINT32  *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *nDrvRomBank;

static INT32 layer_colorbase[3];
static INT32 sprite_colorbase;
static INT32 layerpri[3];

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvReset;
static UINT8 DrvInputs[3];

static INT32 nCyclesDone[2];

static struct BurnInputInfo ParodiusInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Diagnostics",		BIT_DIGITAL,	DrvJoy3 + 1,	"diag"	},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 0,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Parodius)

static struct BurnDIPInfo ParodiusDIPList[]=
{
	{0x15, 0xff, 0xff, 0xff, NULL			},
	{0x16, 0xff, 0xff, 0x5a, NULL			},
	{0x17, 0xff, 0xff, 0xf0, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x15, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x15, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x15, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 	},
	{0x15, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x15, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x15, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x15, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x15, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x15, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x15, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    15, "Coin B"		},
	{0x15, 0x01, 0xf0, 0x20, "4 Coins 1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x50, "3 Coins 1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x80, "2 Coins 1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x15, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x15, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit" 	},
	{0x15, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x15, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x15, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x15, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x15, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x15, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x16, 0x01, 0x03, 0x03, "2"			},
	{0x16, 0x01, 0x03, 0x02, "3"			},
	{0x16, 0x01, 0x03, 0x01, "4"			},
	{0x16, 0x01, 0x03, 0x00, "7"			},

//	{0   , 0xfe, 0   ,    2, "Cabinet"		},
//	{0x16, 0x01, 0x04, 0x00, "Upright"		},
//	{0x16, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x16, 0x01, 0x18, 0x18, "20000 80000"		},
	{0x16, 0x01, 0x18, 0x10, "30000 100000"		},
	{0x16, 0x01, 0x18, 0x08, "20000"		},
	{0x16, 0x01, 0x18, 0x00, "70000"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x60, 0x60, "Easy"			},
	{0x16, 0x01, 0x60, 0x40, "Normal"		},
	{0x16, 0x01, 0x60, 0x20, "Difficult"		},
	{0x16, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x16, 0x01, 0x80, 0x80, "Off"			},
	{0x16, 0x01, 0x80, 0x00, "On"			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x17, 0x01, 0x10, 0x10, "Off"			},
//	{0x17, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Upright Controls"	},
	{0x17, 0x01, 0x20, 0x20, "Single"		},
	{0x17, 0x01, 0x20, 0x00, "Dual"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x17, 0x01, 0x40, 0x40, "Off"			},
	{0x17, 0x01, 0x40, 0x00, "On"			},
};

STDDIPINFO(Parodius)

void parodius_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x3fc0:
			K052109RMRDLine = data & 0x08;
		return;

		case 0x3fc4:
			nDrvRomBank[1] = data;
		return;

		case 0x3fc8:
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;

		case 0x3fcc:
		case 0x3fcd:
			K053260Write(0, address & 1, data);
		return;
	}

	if ((address & 0xf800) == 0x0000) {
		if (nDrvRomBank[1] & 1) {
			DrvPalRAM[((nDrvRomBank[1] & 0x04) << 9) + address] = data;
		} else {
			DrvBankRAM[address] = data;
		}
		return;
	}

	if ((address & 0xfff0) == 0x3fa0) {
		K053244Write(0, address & 0x0f, data);
		return;
	}

	if ((address & 0xfff0) == 0x3fb0) {
		K053251Write(address & 0x0f, data);
		return;
	}

	if ((address & 0xf800) == 0x2000) {
		if (nDrvRomBank[1] & 0x02) {
			K053245Write(0, address & 0x7ff, data);
			return;
		}
	}

	if (address >= 0x2000 && address <= 0x5fff) {
		K052109Write(address - 0x2000, data);
	}
}

UINT8 parodius_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x3f8c:
			return DrvInputs[0];

		case 0x3f8d:
			return DrvInputs[1];

		case 0x3f8e:
			return (DrvDips[2] & 0xf0) | (DrvInputs[2] & 0x0f);

		case 0x3f8f:
			return DrvDips[0];

		case 0x3f90:
			return DrvDips[1];

		case 0x3fc0: // watchdog
			return 0;

		case 0x3fcc:
		case 0x3fcd:
			return K053260Read(0, (address & 1)+2);
	}

	if ((address & 0xf800) == 0x0000) {
		if (nDrvRomBank[1] & 1) {
			return DrvPalRAM[((nDrvRomBank[1] & 0x04) << 9) + address];
		} else {
			return DrvBankRAM[address];
		}
	}

	if ((address & 0xfff0) == 0x3fa0) {
		return K053244Read(0, address & 0x0f);
	}

	if ((address & 0xf800) == 0x2000) {
		if (nDrvRomBank[1] & 0x02) {
			return K053245Read(0, address & 0x7ff);
		}
	}

	if (address >= 0x2000 && address <= 0x5fff) {
		return K052109Read(address - 0x2000);
	}

	return 0;
}

void __fastcall parodius_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xf800:
			BurnYM2151SelectRegister(data);
		return;

		case 0xf801:
			BurnYM2151WriteRegister(data);
		return;

		case 0xfa00:
			nCyclesDone[1] += ZetRun(100);
			ZetNmi();
		return;
	}

	if (address >= 0xfc00 && address <= 0xfc2f) {
		K053260Write(0, address & 0x3f, data);
	}
}

UINT8 __fastcall parodius_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xf800:
			return 0xff;
		case 0xf801:
			return BurnYM2151ReadStatus();
	}

	if (address >= 0xfc00 && address <= 0xfc2f) {
		if ((address & 0x3e) == 0x00) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);

		return K053260Read(0, address & 0x3f);
	}

	return 0;
}

static void K052109Callback(INT32 layer, INT32 bank, INT32 *code, INT32 *color, INT32 *flags, INT32 *)
{
	*flags = (*color & 0x80) >> 7;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

static void K053245Callback(INT32 *code, INT32 *color, INT32 *priority)
{
	INT32 pri = 0x20 | ((*color & 0x60) >> 2);
	if (pri <= layerpri[2])                           *priority = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1]) *priority = 1;
	else if (pri > layerpri[1] && pri <= layerpri[0]) *priority = 2;
	else 	                                          *priority = 3;

	*code &= 0x1fff;
	*color = sprite_colorbase + (*color & 0x1f);
}

static void parodius_set_lines(INT32 lines)
{
	nDrvRomBank[0] = lines;

	konamiMapMemory(DrvKonROM + 0x10000 + ((~lines & 0x0f) * 0x4000), 0x6000, 0x9fff, KON_ROM); 
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

	K053260Reset(0);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvKonROM		= Next; Next += 0x050000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x100000;
	DrvGfxROMExp0		= Next; Next += 0x200000;
	DrvGfxROM1		= Next; Next += 0x100000;
	DrvGfxROMExp1		= Next; Next += 0x200000;

	DrvSndROM		= Next; Next += 0x080000;

	Palette			= (UINT32*)Next; Next += 0x800 * sizeof(UINT32);
	DrvPalette		= (UINT32*)Next; Next += 0x800 * sizeof(UINT32);

	AllRam			= Next;

	DrvBankRAM		= Next; Next += 0x000800;
	DrvKonRAM		= Next; Next += 0x001800;
	DrvPalRAM		= Next; Next += 0x001000;

	DrvZ80RAM		= Next; Next += 0x000800;

	nDrvRomBank		= Next; Next += 0x000002;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4] = { 0x018, 0x010, 0x008, 0x000 };
	INT32 XOffs[8] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007 };
	INT32 YOffs[8] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0 };

	konami_rom_deinterleave_2(DrvGfxROM0, 0x100000);
	konami_rom_deinterleave_2(DrvGfxROM1, 0x100000);

	GfxDecode(0x8000, 4, 8, 8, Plane, XOffs, YOffs, 0x100, DrvGfxROM0, DrvGfxROMExp0);

	K053245GfxDecode(DrvGfxROM1, DrvGfxROMExp1, 0x100000);

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
		if (BurnLoadRom(DrvKonROM  + 0x030000,  1, 1)) return 1;
		memcpy (DrvKonROM + 0x08000, DrvKonROM + 0x48000, 0x8000);

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x080000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000,  6, 1)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000,  7, 1)) return 1;

		DrvGfxDecode();
	}

	konamiInit(1);
	konamiOpen(0);
	konamiMapMemory(DrvKonRAM,		0x0800, 0x1fff, KON_RAM);
	konamiMapMemory(DrvKonROM + 0x10000,	0x6000, 0x9fff, KON_ROM);
	konamiMapMemory(DrvKonROM + 0x0a000,	0xa000, 0xffff, KON_ROM);
	konamiSetWriteHandler(parodius_main_write);
	konamiSetReadHandler(parodius_main_read);
	konamiSetlinesCallback(parodius_set_lines);
	konamiClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xefff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0xefff, 2, DrvZ80ROM);
	ZetMapArea(0xf000, 0xf7ff, 0, DrvZ80RAM);
	ZetMapArea(0xf000, 0xf7ff, 1, DrvZ80RAM);
	ZetMapArea(0xf000, 0xf7ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(parodius_sound_write);
	ZetSetReadHandler(parodius_sound_read);
	ZetMemEnd();
	ZetClose();

	K052109Init(DrvGfxROM0, 0xfffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(8, 0);

	K053245Init(0, DrvGfxROM1, 0xfffff, K053245Callback);
	K053245SetSpriteOffset(0, -112, -16);

	BurnYM2151Init(3579545, 70.0);

	K053260Init(0, 3579545, DrvSndROM, 0x80000);

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
	K053260Exit();

	BurnFree (AllMem);

	return 0;
}

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

static void DrvRecalcPal()
{
	UINT8 r,g,b;
	UINT16 *p = (UINT16*)DrvPalRAM;
	for (INT32 i = 0; i < 0x1000 / 2; i++) {
		UINT16 d = (BURN_ENDIAN_SWAP_INT16(p[i]) << 8) | (BURN_ENDIAN_SWAP_INT16(p[i]) >> 8);

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

	if (nSpriteEnable & 1) K053245SpritesRender(0, DrvGfxROMExp1, 0); // used (back)?

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
		if ((DrvInputs[0] & 0x06) == 0) DrvInputs[0] |= 0x06;
		if ((DrvInputs[0] & 0x18) == 0) DrvInputs[0] |= 0x18;
		if ((DrvInputs[1] & 0x06) == 0) DrvInputs[1] |= 0x06;
		if ((DrvInputs[1] & 0x18) == 0) DrvInputs[1] |= 0x18;
	}

	INT32 nInterleave = nBurnSoundLen;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 3000000 / 60, 3579545 / 60 };
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	ZetOpen(0);
	konamiOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nNext, nCyclesSegment;

		nNext = (i + 1) * nCyclesTotal[0] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[0];
		nCyclesSegment = konamiRun(nCyclesSegment);
		nCyclesDone[0] += nCyclesSegment;

		nNext = (i + 1) * nCyclesTotal[1] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[1];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[1] += nCyclesSegment;

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K053260Update(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (K052109_irq_enabled) konamiSetIrqLine(KONAMI_IRQ_LINE, KONAMI_HOLD_LINE);

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K053260Update(0, pSoundBuf, nSegmentLength);
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
		parodius_set_lines(nDrvRomBank[0]);
		konamiClose();
	}

	return 0;
}

	
// Parodius DA! (World, set 1)

static struct BurnRomInfo parodiusRomDesc[] = {
	{ "955l01.bin",	0x20000, 0x49a658eb, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "955l02.bin",	0x20000, 0x161d7322, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "955e03.bin",	0x10000, 0x940aa356, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "955d07.bin",	0x80000, 0x89473fec, 3 | BRF_GRA },           //  3 Background Tiles
	{ "955d08.bin",	0x80000, 0x43d5cda1, 3 | BRF_GRA },           //  4

	{ "955d05.bin",	0x80000, 0x7a1e55e0, 4 | BRF_GRA },           //  5 Sprites
	{ "955d06.bin",	0x80000, 0xf4252875, 4 | BRF_GRA },           //  6

	{ "955d04.bin",	0x80000, 0xe671491a, 5 | BRF_SND },           //  7 K053260 Samples
};

STD_ROM_PICK(parodius)
STD_ROM_FN(parodius)

struct BurnDriver BurnDrvParodius = {
	"parodius", NULL, NULL, NULL, "1990",
	"Parodius DA! (World, set 1)\0", NULL, "Konami", "GX955",
	L"Parodius \u30D1\u30ED\u30C7\u30A3\u30A6\u30B9\u3060\uFF01 \uFF0D\u795E\u8A71\u304B\u3089\u304A\u7B11\u3044\u3078\uFF0D (World, set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, parodiusRomInfo, parodiusRomName, NULL, NULL, ParodiusInputInfo, ParodiusDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// Parodius DA! (World, set 2)

static struct BurnRomInfo parodiusaRomDesc[] = {
	{ "2.f5",		0x20000, 0x26a6410b, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "3.h5",		0x20000, 0x9410dbf2, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "955e03.bin",	0x10000, 0x940aa356, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "955d07.bin",	0x80000, 0x89473fec, 3 | BRF_GRA },           //  3 Background Tiles
	{ "955d08.bin",	0x80000, 0x43d5cda1, 3 | BRF_GRA },           //  4

	{ "955d05.bin",	0x80000, 0x7a1e55e0, 4 | BRF_GRA },           //  5 Sprites
	{ "955d06.bin",	0x80000, 0xf4252875, 4 | BRF_GRA },           //  6

	{ "955d04.bin",	0x80000, 0xe671491a, 5 | BRF_SND },           //  7 K053260 Samples
};

STD_ROM_PICK(parodiusa)
STD_ROM_FN(parodiusa)

struct BurnDriver BurnDrvParodiusa = {
	"parodiusa", "parodius", NULL, NULL, "1990",
	"Parodius DA! (World, set 2)\0", NULL, "Konami", "GX955",
	L"Parodius \u30D1\u30ED\u30C7\u30A3\u30A6\u30B9\u3060\uFF01 \uFF0D\u795E\u8A71\u304B\u3089\u304A\u7B11\u3044\u3078\uFF0D (World, set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, parodiusaRomInfo, parodiusaRomName, NULL, NULL, ParodiusInputInfo, ParodiusDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// Parodius DA! (Japan)

static struct BurnRomInfo parodisjRomDesc[] = {
	{ "955e01.bin",	0x20000, 0x49baa334, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "955e02.bin",	0x20000, 0x14010d6f, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "955e03.bin",	0x10000, 0x940aa356, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "955d07.bin",	0x80000, 0x89473fec, 3 | BRF_GRA },           //  3 Background Tiles
	{ "955d08.bin",	0x80000, 0x43d5cda1, 3 | BRF_GRA },           //  4

	{ "955d05.bin",	0x80000, 0x7a1e55e0, 4 | BRF_GRA },           //  5 Sprites
	{ "955d06.bin",	0x80000, 0xf4252875, 4 | BRF_GRA },           //  6

	{ "955d04.bin",	0x80000, 0xe671491a, 5 | BRF_SND },           //  7 K053260 Samples
};

STD_ROM_PICK(parodisj)
STD_ROM_FN(parodisj)

struct BurnDriver BurnDrvParodisj = {
	"parodiusj", "parodius", NULL, NULL, "1990",
	"Parodius DA! (Japan)\0", NULL, "Konami", "GX955",
	L"Parodius \u30D1\u30ED\u30C7\u30A3\u30A6\u30B9\u3060\uFF01 \uFF0D\u795E\u8A71\u304B\u3089\u304A\u7B11\u3044\u3078\uFF0D (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, parodisjRomInfo, parodisjRomName, NULL, NULL, ParodiusInputInfo, ParodiusDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};
