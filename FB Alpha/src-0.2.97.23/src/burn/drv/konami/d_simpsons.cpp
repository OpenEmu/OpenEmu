// FB Alpha The Simpsons driver module
// Based on MAME driver by Ernesto Corvi and various others

#include "tiles_generic.h"
#include "z80_intf.h"
#include "burn_ym2151.h"
#include "konami_intf.h"
#include "konamiic.h"
#include "k053260.h"
#include "eeprom.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvKonROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvSndROM;
static UINT8 *DrvKonRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvZ80RAM;

static UINT32 *Palette;
static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *nDrvBank;

static INT32 videobank;
static INT32 init_eeprom_count;
static INT32 simpsons_firq_enabled;
static INT32 K053246Irq;

static INT32 bg_colorbase;
static INT32 sprite_colorbase;
static INT32 layer_colorbase[3];
static INT32 layerpri[3];

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvJoy4[8];
static UINT8 DrvJoy5[8];
static UINT8 DrvDiag;
static UINT8 DrvReset;
static UINT8 DrvInputs[5];

static INT32 nCyclesDone[2];

static struct BurnInputInfo SimpsonsInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy5 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy5 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy5 + 2,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	DrvJoy3 + 7,	"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p3 fire 2"	},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy5 + 3,	"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	DrvJoy4 + 7,	"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	DrvJoy4 + 2,	"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy4 + 3,	"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy4 + 0,	"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	DrvJoy4 + 1,	"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy4 + 4,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy4 + 5,	"p4 fire 2"	},

	{"Diagnostics",		BIT_DIGITAL,	&DrvDiag,	"diag"		},
	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
};

STDINPUTINFO(Simpsons)

static struct BurnInputInfo Simpsons2pInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy5 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy5 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Diagnostics",		BIT_DIGITAL,	&DrvDiag,	"diag"		},
	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
};

STDINPUTINFO(Simpsons2p)

void simpsons_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x1fa0:
		case 0x1fa1:
		case 0x1fa2:
		case 0x1fa3:
		case 0x1fa4:
		case 0x1fa5:
		case 0x1fa6:
		case 0x1fa7:
			K053246Write(address & 7, data);
		return;

		case 0x1fc0:
			K052109RMRDLine = data & 0x08;
			K053246_set_OBJCHA_line(~data & 0x20);
		return;

		case 0x1fc2:
		{
			if (data == 0xff) return; // ok?

			EEPROMWrite((data & 0x10) >> 3, (data & 0x08) >> 3, (data & 0x80) >> 7);

			videobank = data & 3;
			simpsons_firq_enabled = data & 0x04;
		}
		return;

		case 0x1fc6:
		case 0x1fc7:
			K053260Write(0, address & 1, data);
		return;
	}

	if ((address & 0xf000) == 0x0000) {
		if (videobank & 1) {
			DrvPalRAM[address & 0x0fff] = data;
			return;
		}
	}

	if ((address & 0xfff0) == 0x1fb0) {
		K053251Write(address & 0x0f, data);
		return;
	}

	if ((address & 0xe000) == 0x2000) {
		if (videobank & 2) {
			address ^= 1;
			DrvSprRAM[address & 0x1fff] = data;
			return;
		}
	}

	if ((address & 0xc000) == 0x0000) {
		K052109Write(address & 0x3fff, data);
		return;
	}
}

UINT8 simpsons_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x1f81:
		{
			INT32 res = ((EEPROMRead() & 1) << 4) | 0x20 | (~DrvDiag & 1);

			if (init_eeprom_count > 0)
			{
				init_eeprom_count--;
				res &= 0xfe;
			}
			return res;
		}

		case 0x1f80:
			return DrvInputs[4];

		case 0x1f90:
			return DrvInputs[0];

		case 0x1f91:
			return DrvInputs[1];

		case 0x1f92:
			return DrvInputs[2];

		case 0x1f93:
			return DrvInputs[3];

		case 0x1fc4: 
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			return 0;

		case 0x1fc6:
		case 0x1fc7:
			return K053260Read(0, (address & 1)+2);
		
		case 0x1fc8:
      		case 0x1fc9:
         		return K053246Read(address & 1);

		case 0x1fca:
			return 0; // watchdog
	}

	if ((address & 0xf000) == 0x0000) {
		if (videobank & 1) {
			return DrvPalRAM[address & 0x0fff];
		}
	}

	if ((address & 0xe000) == 0x2000) {
		if (videobank & 2) {
			address ^= 1;
			return DrvSprRAM[address & 0x1fff];
		}
	}

	if ((address & 0xc000) == 0x0000) {
		return K052109Read(address & 0x3fff);
	}

	return 0;
}

static void DrvZ80Bankswitch(INT32 data)
{
	data &= 0x07;
	if (data < 2) return;

	INT32 nBank = (data & 7) * 0x4000;

	nDrvBank[1] = data;

	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + nBank);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + nBank);
}

void __fastcall simpsons_sound_write(UINT16 address, UINT8 data)
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

		case 0xfe00:
			DrvZ80Bankswitch(data);
		return;
	}

	if (address >= 0xfc00 && address < 0xfc30) {
		K053260Write(0, address & 0xff, data);
		return;
	}
}

UINT8 __fastcall simpsons_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xf800:
			return 0xff;
		case 0xf801:
			return BurnYM2151ReadStatus();
	}

	if (address >= 0xfc00 && address < 0xfc30) {
		if ((address & 0x3f) == 0x01) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);

		return K053260Read(0, address & 0xff);
	}

	return 0;
}

static void simpsons_set_lines(INT32 lines)
{
	nDrvBank[0] = lines;

	INT32 nBank = (lines & 0x3f) * 0x2000;

	konamiMapMemory(DrvKonROM + 0x10000 + nBank, 0x6000, 0x7fff, KON_ROM); 
}

static void K052109Callback(INT32 layer, INT32 bank, INT32 *code, INT32 *color, INT32 *, INT32 *)
{
	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*color = layer_colorbase[layer] + ((*color & 0xc0) >> 6);
	*code &= 0x7fff;
}

static void K053247Callback(INT32 *code, INT32 *color, INT32 *priority)
{
	INT32 pri = (*color & 0x0f80) >> 6;
	if (pri <= layerpri[2])					*priority = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority = 1;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority = 2;
	else 							*priority = 3;

	*color = sprite_colorbase + (*color & 0x001f);

	*code &= 0x7fff;
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

	EEPROMReset();

	videobank = 0;

	if (EEPROMAvailable()) {
		init_eeprom_count = 0;
	} else {
		init_eeprom_count = 10;
	}

	simpsons_firq_enabled = 0;
	K053246Irq = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvKonROM		= Next; Next += 0x090000;
	DrvZ80ROM		= Next; Next += 0x020000;

	DrvGfxROM0		= Next; Next += 0x100000;
	DrvGfxROMExp0		= Next; Next += 0x200000;
	DrvGfxROM1		= Next; Next += 0x400000;
	DrvGfxROMExp1		= Next; Next += 0x800000;

	DrvSndROM		= Next; Next += 0x200000;

	Palette			= (UINT32*)Next; Next += 0x800 * sizeof(UINT32);
	DrvPalette		= (UINT32*)Next; Next += 0x800 * sizeof(UINT32);

	AllRam			= Next;

	DrvZ80RAM		= Next; Next += 0x000800;

	DrvKonRAM		= Next; Next += 0x002000;
	DrvPalRAM		= Next; Next += 0x001000;
	DrvSprRAM		= Next; Next += 0x002000;

	nDrvBank		= Next; Next += 0x000002;

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
	konami_rom_deinterleave_4(DrvGfxROM1, 0x400000);

	GfxDecode(0x8000, 4, 8, 8, Plane, XOffs, YOffs, 0x100, DrvGfxROM0, DrvGfxROMExp0);

	K053247GfxDecode(DrvGfxROM1, DrvGfxROMExp1, 0x400000);

	return 0;
}

static const eeprom_interface simpsons_eeprom_intf =
{
	7,			// address bits
	8,			// data bits
	"011000",		// read command
	"011100",		// write command
	0,			// erase command
	"0100000000000",	// lock command
	"0100110000000",	// unlock command
	0,
	0
};

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
		if (BurnLoadRom(DrvKonROM  + 0x050000,  2, 1)) return 1;
		if (BurnLoadRom(DrvKonROM  + 0x070000,  3, 1)) return 1;
		memcpy (DrvKonROM + 0x08000, DrvKonROM + 0x88000, 0x8000);

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x080000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x100000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x200000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x300000, 10, 1)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000, 11, 1)) return 1;
		if (BurnLoadRom(DrvSndROM  + 0x100000, 12, 1)) return 1;

		DrvGfxDecode();
	}

	konamiInit(1);
	konamiOpen(0);
	konamiMapMemory(DrvKonRAM,		0x4000, 0x5fff, KON_RAM);
	konamiMapMemory(DrvKonROM + 0x10000,	0x6000, 0x7fff, KON_ROM);
	konamiMapMemory(DrvKonROM + 0x08000,	0x8000, 0xffff, KON_ROM);
	konamiSetWriteHandler(simpsons_main_write);
	konamiSetReadHandler(simpsons_main_read);
	konamiSetlinesCallback(simpsons_set_lines);
	konamiClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + 0x08000);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + 0x08000);
	ZetMapArea(0xf000, 0xf7ff, 0, DrvZ80RAM);
	ZetMapArea(0xf000, 0xf7ff, 1, DrvZ80RAM);
	ZetMapArea(0xf000, 0xf7ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(simpsons_sound_write);
	ZetSetReadHandler(simpsons_sound_read);
	ZetMemEnd();
	ZetClose();

	EEPROMInit(&simpsons_eeprom_intf);

	K052109Init(DrvGfxROM0, 0x0fffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(8, 0);

	K053247Init(DrvGfxROM1, 0x3fffff, K053247Callback, 0x03 /* shadows & highlights */);
	K053247SetSpriteOffset(-59, 39);

	BurnYM2151Init(3579545);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 1.00, BURN_SND_ROUTE_BOTH);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.00, BURN_SND_ROUTE_BOTH); // not connected

	K053260Init(0, 3579545, DrvSndROM, 0x140000);
	K053260SetRoute(0, BURN_SND_K053260_ROUTE_1, 0.75, BURN_SND_ROUTE_LEFT);
	K053260SetRoute(0, BURN_SND_K053260_ROUTE_2, 0.75, BURN_SND_ROUTE_RIGHT);

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

	EEPROMExit();

	BurnYM2151Exit();
	K053260Exit();

	BurnFree (AllMem);

	return 0;
}

static void simpsons_objdma()
{
	INT32 counter, num_inactive;
	UINT8 *dstptr;
	UINT16 *src, *dst;

	K053247Export(&dstptr, 0, 0, 0, &counter);
	src = (UINT16*)DrvSprRAM;
	dst = (UINT16*)dstptr;
	num_inactive = counter = 256;

	do {
		if ((*src & BURN_ENDIAN_SWAP_INT16(0x8000)) && (*src & BURN_ENDIAN_SWAP_INT16(0xff)))
		{
			memcpy(dst, src, 0x10);
			dst += 8;
			num_inactive--;
		}
		src += 8;
	}
	while (--counter);

	if (num_inactive) do { *dst = 0; dst += 8; } while (--num_inactive);
}

static void DrvRecalcPal()
{
	UINT8 r,g,b;
	UINT16 *p = (UINT16*)DrvPalRAM;
	for (INT32 i = 0; i < 0x1000 / 2; i++) {
		UINT16 d = BURN_ENDIAN_SWAP_INT16((p[i] << 8) | (p[i] >> 8));

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

	INT32 layer[3];

	bg_colorbase       = K053251GetPaletteIndex(0);
	sprite_colorbase   = K053251GetPaletteIndex(1);
	layer_colorbase[0] = K053251GetPaletteIndex(2);
	layer_colorbase[1] = K053251GetPaletteIndex(3);
	layer_colorbase[2] = K053251GetPaletteIndex(4);

	layerpri[0] = K053251GetPriority(2);
	layerpri[1] = K053251GetPriority(3);
	layerpri[2] = K053251GetPriority(4);
	layer[0] = 0;
	layer[1] = 1;
	layer[2] = 2;

	sortlayers(layer,layerpri);

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 16 * bg_colorbase;
	}

	if (nSpriteEnable & 8) K053247SpritesRender(DrvGfxROMExp1, 3);		// title (simpsons behind clouds)
	if (nBurnLayer & 1)    K052109RenderLayer(layer[0], 0, DrvGfxROMExp0);	// title (back-most cloud)
	if (nSpriteEnable & 4) K053247SpritesRender(DrvGfxROMExp1, 2);		// smithers' on first stage
	if (nBurnLayer & 2)    K052109RenderLayer(layer[1], 0, DrvGfxROMExp0);	// main layer (buildings, stage 1)
	if (nSpriteEnable & 2) K053247SpritesRender(DrvGfxROMExp1, 1);		// smithers' thugs on stage 1
	if (nBurnLayer & 4)    K052109RenderLayer(layer[2], 0, DrvGfxROMExp0);	// game over text
	if (nSpriteEnable & 1) K053247SpritesRender(DrvGfxROMExp1, 0);		// not used? seems to make sense here...

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
		memset (DrvInputs, 0xff, 5);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
		}

	  // Clear Opposites
		if ((DrvInputs[0] & 0x0c) == 0) DrvInputs[0] |= 0x0c;
		if ((DrvInputs[0] & 0x03) == 0) DrvInputs[0] |= 0x03;
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[2] & 0x0c) == 0) DrvInputs[2] |= 0x0c;
		if ((DrvInputs[2] & 0x03) == 0) DrvInputs[2] |= 0x03;
		if ((DrvInputs[3] & 0x0c) == 0) DrvInputs[3] |= 0x0c;
		if ((DrvInputs[3] & 0x03) == 0) DrvInputs[3] |= 0x03;
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

		if (i == 1 && K053246Irq && simpsons_firq_enabled) {
			konamiSetIrqLine(KONAMI_FIRQ_LINE, KONAMI_HOLD_LINE);
		}

		K053246Irq = K053246_is_IRQ_enabled();

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

	if (K053246Irq) simpsons_objdma();
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
		K053260Scan(nAction);

		KonamiICScan(nAction);

		EEPROMScan(nAction, pnMin);

		SCAN_VAR(videobank);
		SCAN_VAR(init_eeprom_count);
		SCAN_VAR(simpsons_firq_enabled);
		SCAN_VAR(K053246Irq);
	}

	if (nAction & ACB_WRITE) {
		konamiOpen(0);
		simpsons_set_lines(nDrvBank[0]);
		konamiClose();

		ZetOpen(0);
		DrvZ80Bankswitch(nDrvBank[1]);
		ZetClose();
	}

	return 0;
}


// The Simpsons (4 Players World, set 1)

static struct BurnRomInfo simpsonsRomDesc[] = {
	{ "072-g02.16c",	0x020000, 0x580ce1d6, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-g01.17c",	0x020000, 0x9f843def, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-j13.13c",	0x020000, 0xaade2abd, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-j12.15c",	0x020000, 0x479e12f2, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-e03.6g",		0x020000, 0x866b7a35, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12

	{ "simpsons.nv",  0x000080, 0xec3f0449, BRF_OPT },
};

STD_ROM_PICK(simpsons)
STD_ROM_FN(simpsons)

struct BurnDriver BurnDrvSimpsons = {
	"simpsons", NULL, NULL, NULL, "1991",
	"The Simpsons (4 Players World, set 1)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, simpsonsRomInfo, simpsonsRomName, NULL, NULL, SimpsonsInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// The Simpsons (4 Players World, set 2)

static struct BurnRomInfo simpsons4paRomDesc[] = {
	{ "072-g02.16c",	0x020000, 0x580ce1d6, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-g01.17c",	0x020000, 0x9f843def, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-m13.13c",	0x020000, 0xf36c9423, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-l12.15c",	0x020000, 0x84f9d9ba, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-e03.6g",		0x020000, 0x866b7a35, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12

	{ "simpsons4pa.nv",  0x000080, 0xec3f0449, BRF_OPT },
};

STD_ROM_PICK(simpsons4pa)
STD_ROM_FN(simpsons4pa)

struct BurnDriver BurnDrvSimpsons4pa = {
	"simpsons4pa", "simpsons", NULL, NULL, "1991",
	"The Simpsons (4 Players World, set 2)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, simpsons4paRomInfo, simpsons4paRomName, NULL, NULL, SimpsonsInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// The Simpsons (2 Players World, set 1)

static struct BurnRomInfo simpsons2pRomDesc[] = {
	{ "072-g02.16c",	0x020000, 0x580ce1d6, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-p01.17c",	0x020000, 0x07ceeaea, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-013.13c",	0x020000, 0x8781105a, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-012.15c",	0x020000, 0x244f9289, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-g03.6g",		0x020000, 0x76c1850c, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12

	{ "simpsons2p.nv",  0x000080, 0xfbac4e30, BRF_OPT },
};

STD_ROM_PICK(simpsons2p)
STD_ROM_FN(simpsons2p)

struct BurnDriver BurnDrvSimpsons2p = {
	"simpsons2p", "simpsons", NULL, NULL, "1991",
	"The Simpsons (2 Players World, set 1)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, simpsons2pRomInfo, simpsons2pRomName, NULL, NULL, Simpsons2pInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// The Simpsons (2 Players World, set 2)

static struct BurnRomInfo simpsons2p2RomDesc[] = {
	{ "072-g02.16c",	0x020000, 0x580ce1d6, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-p01.17c",	0x020000, 0x07ceeaea, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-_13.13c",	0x020000, 0x54e6df66, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-_12.15c",	0x020000, 0x96636225, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-g03.6g",		0x020000, 0x76c1850c, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12

	{ "simpsons2p2.nv",  0x000080, 0xfbac4e30, BRF_OPT },
};

STD_ROM_PICK(simpsons2p2)
STD_ROM_FN(simpsons2p2)

struct BurnDriver BurnDrvSimpsons2p2 = {
	"simpsons2p2", "simpsons", NULL, NULL, "1991",
	"The Simpsons (2 Players World, set 2)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, simpsons2p2RomInfo, simpsons2p2RomName, NULL, NULL, SimpsonsInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// The Simpsons (2 Players Asia)

static struct BurnRomInfo simpsons2paRomDesc[] = {
	{ "072-g02.16c",	0x020000, 0x580ce1d6, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-p01.17c",	0x020000, 0x07ceeaea, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-113.13c",	0x020000, 0x8781105a, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-112.15c",	0x020000, 0x3bd69404, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-e03.6g",		0x020000, 0x866b7a35, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12

	{ "simpsons2pa.nv",  0x000080, 0xfbac4e30, BRF_OPT },
};

STD_ROM_PICK(simpsons2pa)
STD_ROM_FN(simpsons2pa)

struct BurnDriver BurnDrvSimpsons2pa = {
	"simpsons2pa", "simpsons", NULL, NULL, "1991",
	"The Simpsons (2 Players Asia)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, simpsons2paRomInfo, simpsons2paRomName, NULL, NULL, Simpsons2pInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// The Simpsons (2 Players Japan)

static struct BurnRomInfo simpsons2pjRomDesc[] = {
	{ "072-s02.16c",	0x020000, 0x265f7a47, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "072-t01.17c",	0x020000, 0x91de5c2d, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "072-213.13c",	0x020000, 0xb326a9ae, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "072-212.15c",	0x020000, 0x584d9d37, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "072-g03.6g",		0x020000, 0x76c1850c, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "072-b07.18h",	0x080000, 0xba1ec910, 3 | BRF_GRA },           //  5 K052109 Tiles
	{ "072-b06.16h",	0x080000, 0xcf2bbcab, 3 | BRF_GRA },           //  6

	{ "072-b08.3n",		0x100000, 0x7de500ad, 4 | BRF_GRA },           //  7 K053247 Tiles
	{ "072-b09.8n",		0x100000, 0xaa085093, 4 | BRF_GRA },           //  8
	{ "072-b10.12n",	0x100000, 0x577dbd53, 4 | BRF_GRA },           //  9
	{ "072-b11.16l",	0x100000, 0x55fab05d, 4 | BRF_GRA },           // 10

	{ "072-d05.1f",		0x100000, 0x1397a73b, 5 | BRF_SND },           // 11 K053260 Samples
	{ "072-d04.1d",		0x040000, 0x78778013, 5 | BRF_SND },           // 12

	{ "simpsons2pj.nv",  0x000080, 0x3550a54e, BRF_OPT },
};

STD_ROM_PICK(simpsons2pj)
STD_ROM_FN(simpsons2pj)

struct BurnDriver BurnDrvSimpsons2pj = {
	"simpsons2pj", "simpsons", NULL, NULL, "1991",
	"The Simpsons (2 Players Japan)\0", NULL, "Konami", "GX072",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, simpsons2pjRomInfo, simpsons2pjRomName, NULL, NULL, Simpsons2pInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};
