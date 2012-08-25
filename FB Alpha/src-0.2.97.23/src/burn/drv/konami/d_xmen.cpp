// FB Alpha X-Men driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "konamiic.h"
#include "burn_ym2151.h"
#include "k054539.h"
#include "eeprom.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvSndROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;

static UINT32 *Palette;
static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *soundlatch2;
static UINT8 *nDrvZ80Bank;

static INT32 interrupt_enable;
static INT32 init_eeprom_count;

static INT32 sprite_colorbase;
static INT32 bg_colorbase;
static INT32 layerpri[3];
static INT32 layer_colorbase[3];

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvReset;
static UINT16 DrvInputs[3];

static struct BurnInputInfo XmenInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 8,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 9,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 fire 3"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy2 + 15,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	DrvJoy3 + 10,	"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy2 + 8,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy2 + 9,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy2 + 10,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy2 + 11,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy2 + 12,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy2 + 13,	"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	DrvJoy2 + 14,	"p3 fire 3"	},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy1 + 15,	"p4 coin"	},
	{"P4 Start",		BIT_DIGITAL,	DrvJoy3 + 11,	"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p4 fire 2"	},
	{"P4 Button 3",		BIT_DIGITAL,	DrvJoy1 + 14,	"p4 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Diagnostics",		BIT_DIGITAL,	DrvJoy3 + 14,	"diag"	},
};

STDINPUTINFO(Xmen)

static struct BurnInputInfo Xmen2pInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 8,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 9,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Diagnostics",		BIT_DIGITAL,	DrvJoy3 + 14,	"diag"	},
	{"Service 1",		BIT_DIGITAL,	DrvJoy3 + 0,	"service"	},
	{"Service 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"service2"	},
};

STDINPUTINFO(Xmen2p)

void __fastcall xmen_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x108000:
			K052109RMRDLine = data & 0x02;
			K053246_set_OBJCHA_line(data & 0x01);
		return;

		case 0x108001:
			EEPROMWrite(data & 0x08, data & 0x10, data & 0x04);
		return;

		case 0x10804d:
			*soundlatch = data;
		return;

		case 0x10804e:
		case 0x10804f: 
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;

		case 0x10a001:
			// watchdog
		return;

		case 0x18fa01:
			interrupt_enable = data & 0x04;
		return;
	}

	if (address >= 0x18c000 && address <= 0x197fff) {
		if (address & 1) K052109Write((address - 0x18c000) >> 1, data);
		return;
	}

	if ((address & 0xfff000) == 0x100000) {
		K053247Write((address & 0xfff) ^ 1, data);
		return;
	}

	if ((address & 0xfffff8) == 0x108020) {
		K053246Write((address & 0x007)^1, data);
		return;
	}
		
	if ((address & 0xffffe0) == 0x108060) {
		if (address & 1) K053251Write((address >> 1) & 0x0f, data);
		return;
	}
}

void __fastcall xmen_main_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfff000) == 0x100000) {
		K053247Write(address & 0xffe, data | 0x10000);
		return;
	}

	if ((address & 0xfffff8) == 0x108020) {
		K053246Write((address & 0x006) | 0, data & 0xff);
		K053246Write((address & 0x006) | 1, data >> 8);
		return;
	}
}

UINT8 __fastcall xmen_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x108054:
		case 0x108055:
			return *soundlatch2;

		case 0x10a000:
		case 0x10a001:
			return DrvInputs[0] >> ((~address & 1) << 3);

		case 0x10a002:
		case 0x10a003:
			return DrvInputs[1] >> ((~address & 1) << 3);

		case 0x10a004:
			if (init_eeprom_count > 0) {
				init_eeprom_count--;
				return 0xbf;
			}
			return DrvInputs[2] >> 8;

		case 0x10a005: // eeprom_r
			init_eeprom_count--;
			return (DrvInputs[2] & 0xbf) | (EEPROMRead() << 6);

		case 0x10a00c:
		case 0x10a00d:
			return K053246Read(~address & 1);
	}

	if ((address & 0xfff000) == 0x100000) {
		return K053247Read((address & 0xfff)^1);
	}

	if (address >= 0x18c000 && address <= 0x197fff) {
		return K052109Read((address - 0x18c000) >> 1);
	}

	return 0;
}

UINT16 __fastcall xmen_main_read_word(UINT32 /*address*/)
{
	return 0;
}

static void bankswitch(INT32 bank)
{
	nDrvZ80Bank[0] = bank & 7;

	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + nDrvZ80Bank[0] * 0x4000);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + nDrvZ80Bank[0] * 0x4000);
}

void __fastcall xmen_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe800:
		case 0xec00:
			BurnYM2151SelectRegister(data);
		return;

		case 0xe801:
		case 0xec01:
			BurnYM2151WriteRegister(data);
		return;

		case 0xf000:
			*soundlatch2 = data;
		return;

		case 0xf800:
			bankswitch(data);
		return;
	}

	if (address >= 0xe000 && address <= 0xe22f) {
		return K054539Write(0, address & 0x3ff, data);
	}
}

UINT8 __fastcall xmen_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xe800:
		case 0xe801:
		case 0xec00:
		case 0xec01:
			return BurnYM2151ReadStatus();

		case 0xf002:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;
	}

	if (address >= 0xe000 && address <= 0xe22f) {
		return K054539Read(0, address & 0x3ff);
	}

	return 0;
}

static void K052109Callback(INT32 layer, INT32 , INT32 *, INT32 *color, INT32 *, INT32 *)
{
	if (layer == 0)
		*color = layer_colorbase[layer] + ((*color & 0xf0) >> 4);
	else
		*color = layer_colorbase[layer] + ((*color & 0x7c) >> 2);
}

static void K053247Callback(INT32 *code, INT32 *color, INT32 *priority_mask)
{
	INT32 pri = (*color & 0x00e0) >> 4;
	if (pri <= layerpri[2])					*priority_mask = 0;
	else if (pri > layerpri[2] && pri <= layerpri[1])	*priority_mask = 1;
	else if (pri > layerpri[1] && pri <= layerpri[0])	*priority_mask = 2;
	else 							*priority_mask = 3;

	*color = sprite_colorbase + (*color & 0x001f);
	*code &= 0x7fff;
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

	BurnYM2151Reset();
	K054539Reset(0);

	KonamiICReset();

	EEPROMReset();

	if (EEPROMAvailable()) {
		init_eeprom_count = 0;
	} else {
		init_eeprom_count = 10;
	}

	interrupt_enable = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x100000;
	DrvZ80ROM		= Next; Next += 0x020000;

	DrvGfxROM0		= Next; Next += 0x200000;
	DrvGfxROMExp0		= Next; Next += 0x400000;
	DrvGfxROM1		= Next; Next += 0x400000;
	DrvGfxROMExp1		= Next; Next += 0x800000;

	DrvSndROM		= Next; Next += 0x200000;

	Palette			= (UINT32*)Next; Next += 0x800 * sizeof(UINT32);
	DrvPalette		= (UINT32*)Next; Next += 0x800 * sizeof(UINT32);

	AllRam			= Next;

	DrvPalRAM		= Next; Next += 0x001000;
	Drv68KRAM		= Next; Next += 0x005000;

	DrvZ80RAM		= Next; Next += 0x002000;

	soundlatch		= Next; Next += 0x000001;
	soundlatch2		= Next; Next += 0x000001;

	nDrvZ80Bank		= Next; Next += 0x000001;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4] = { 0x018, 0x010, 0x008, 0x000 };
	INT32 XOffs[8] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007 };
	INT32 YOffs[8] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0 };

	konami_rom_deinterleave_2(DrvGfxROM0, 0x200000);
	konami_rom_deinterleave_4(DrvGfxROM1, 0x400000);

	GfxDecode(0x10000, 4, 8, 8, Plane, XOffs, YOffs, 0x100, DrvGfxROM0, DrvGfxROMExp0);

	K053247GfxDecode(DrvGfxROM1, DrvGfxROMExp1, 0x400000);

	return 0;
}

static const eeprom_interface xmen_eeprom_intf =
{
	7,		 // address bits
	8,		 // data bits
	"011000",	 // read command
	"011100",	 // write command
	0,		 // erase command
	"0100000000000", // lock command
	"0100110000000", // unlock command
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
		if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080000,  3, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x100000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x100000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x200000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x300000, 10, 1)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000, 11, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM + 0x00000,	0x101000, 0x101fff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x104000, 0x104fff, SM_RAM);
	SekMapMemory(Drv68KRAM + 0x01000,	0x110000, 0x113fff, SM_RAM);
	SekSetWriteByteHandler(0,		xmen_main_write_byte);
	SekSetWriteWordHandler(0,		xmen_main_write_word);
	SekSetReadByteHandler(0,		xmen_main_read_byte);
	SekSetReadWordHandler(0,		xmen_main_read_word);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + 0x8000);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + 0x8000);
	ZetMapArea(0xc000, 0xdfff, 0, DrvZ80RAM);
	ZetMapArea(0xc000, 0xdfff, 1, DrvZ80RAM);
	ZetMapArea(0xc000, 0xdfff, 2, DrvZ80RAM);
	ZetSetWriteHandler(xmen_sound_write);
	ZetSetReadHandler(xmen_sound_read);
	ZetMemEnd();
	ZetClose();

	EEPROMInit(&xmen_eeprom_intf);

	K052109Init(DrvGfxROM0, 0x1fffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(8, 0);

	K053247Init(DrvGfxROM1, 0x3fffff, K053247Callback, 1);
	K053247SetSpriteOffset(-510, 158);

	BurnYM2151Init(4000000);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.20, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.20, BURN_SND_ROUTE_RIGHT);

	K054539Init(0, 48000, DrvSndROM, 0x200000);
	K054539SetRoute(0, BURN_SND_K054539_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	K054539SetRoute(0, BURN_SND_K054539_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	KonamiICExit();

	SekExit();
	ZetExit();

	BurnYM2151Exit();
	K054539Exit();

	EEPROMExit();

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

static inline void DrvRecalcPalette()
{
	UINT8 r,g,b;
	UINT16 *p = (UINT16*)DrvPalRAM;
	for (INT32 i = 0; i < 0x1000 / 2; i++) {
		INT32 d = BURN_ENDIAN_SWAP_INT16(p[i]);

		r = (d >>  0) & 0x1f;
		g = (d >>  5) & 0x1f;
		b = (d >> 10) & 0x1f;

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		Palette[i] = (r << 16) | (g << 8) | b;
		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvRecalcPalette();
	}

	K052109UpdateScroll();

	INT32 layer[3];

	bg_colorbase       = K053251GetPaletteIndex(4);
	sprite_colorbase   = K053251GetPaletteIndex(1);
	layer_colorbase[0] = K053251GetPaletteIndex(3);
	layer_colorbase[1] = K053251GetPaletteIndex(0);
	layer_colorbase[2] = K053251GetPaletteIndex(2);

	layerpri[0] = K053251GetPriority(3);
	layerpri[1] = K053251GetPriority(0);
	layerpri[2] = K053251GetPriority(2);
	layer[0] = 0;
	layer[1] = 1;
	layer[2] = 2;

	sortlayers(layer,layerpri);

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 16 * bg_colorbase+1;
	}

	if (nSpriteEnable & 8) K053247SpritesRender(DrvGfxROMExp1, 3);

	if (nBurnLayer & 1) K052109RenderLayer(layer[0], 0, DrvGfxROMExp0);

	if (nBurnLayer & 2) K052109RenderLayer(layer[1], 0, DrvGfxROMExp0);

if (nBurnLayer & 8) {
	if (nSpriteEnable & 1) K053247SpritesRender(DrvGfxROMExp1, 0);
	if (nSpriteEnable & 2) K053247SpritesRender(DrvGfxROMExp1, 1);
	if (nSpriteEnable & 4) K053247SpritesRender(DrvGfxROMExp1, 2);
}

	if (nBurnLayer & 4) K052109RenderLayer(layer[2], 0, DrvGfxROMExp0);

	KonamiBlendCopy(Palette, DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 3 * sizeof(INT16));
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

	  // Clear Opposites
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[0] & 0x0c) == 0) DrvInputs[0] |= 0x0c;
		if ((DrvInputs[0] & 0x03) == 0) DrvInputs[0] |= 0x03;
		if ((DrvInputs[1] & 0xc00) == 0) DrvInputs[1] |= 0xc00;
		if ((DrvInputs[1] & 0x300) == 0) DrvInputs[1] |= 0x300;
		if ((DrvInputs[0] & 0xc00) == 0) DrvInputs[0] |= 0xc00;
		if ((DrvInputs[0] & 0x300) == 0) DrvInputs[0] |= 0x300;
	}

	INT32 nInterleave = nBurnSoundLen;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 16000000 / 60, 8000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nNext, nCyclesSegment;

		nNext = (i + 1) * nCyclesTotal[0] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[0];
		nCyclesSegment = SekRun(nCyclesSegment);
		nCyclesDone[0] += nCyclesSegment;

		if (i == (nInterleave / 2) && interrupt_enable) {
			SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		}

		nNext = (i + 1) * nCyclesTotal[1] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[1];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[1] += nCyclesSegment;

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K054539Update(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (interrupt_enable) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
	
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K054539Update(0, pSoundBuf, nSegmentLength);
		}
	}

	ZetClose();
	SekClose();

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

		SekScan(nAction);
		ZetScan(nAction);

		BurnYM2151Scan(nAction);

		K054539Scan(nAction);

		KonamiICScan(nAction);
		EEPROMScan(nAction, pnMin);

		SCAN_VAR(interrupt_enable);
		SCAN_VAR(init_eeprom_count);
	}

	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		bankswitch(nDrvZ80Bank[0]);
		ZetClose();
	}

	EEPROMScan(nAction, pnMin);

	return 0;
}


// X-Men (4 Players ver UBB)

static struct BurnRomInfo xmenRomDesc[] = {
	{ "065-ubb04.10d",	0x020000, 0xf896c93b, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "065-ubb05.10f",	0x020000, 0xe02e5d64, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "065-a02.9d",		0x040000, 0xb31dc44c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "065-a03.9f",		0x040000, 0x13842fe6, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "065-a01.6f",		0x020000, 0x147d3a4d, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "065-a08.15l",	0x100000, 0x6b649aca, 3 | BRF_GRA },           //  5 Background Tiles
	{ "065-a07.16l",	0x100000, 0xc5dc8fc4, 3 | BRF_GRA },           //  6

	{ "065-a09.2h",		0x100000, 0xea05d52f, 4 | BRF_GRA },           //  7 Sprites
	{ "065-a10.2l",		0x100000, 0x96b91802, 4 | BRF_GRA },           //  8
	{ "065-a12.1h",		0x100000, 0x321ed07a, 4 | BRF_GRA },           //  9
	{ "065-a11.1l",		0x100000, 0x46da948e, 4 | BRF_GRA },           // 10

	{ "065-a06.1f",		0x200000, 0x5adbcee0, 5 | BRF_SND },           // 11 K054539 Samples

	{ "xmen_ubb.nv",  0x000080, 0x52f334ba, BRF_OPT },
};

STD_ROM_PICK(xmen)
STD_ROM_FN(xmen)

struct BurnDriver BurnDrvXmen = {
	"xmen", NULL, NULL, NULL, "1992",
	"X-Men (4 Players ver UBB)\0", NULL, "Konami", "GX065",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, xmenRomInfo, xmenRomName, NULL, NULL, XmenInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// X-Men (4 Players ver ADA)

static struct BurnRomInfo xmenaRomDesc[] = {
	{ "065-ada.10d",	0x020000, 0xb8276624, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "065-ada.10f",	0x020000, 0xc68582ad, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "065-a02.9d",		0x040000, 0xb31dc44c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "065-a03.9f",		0x040000, 0x13842fe6, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "065-a01.6f",		0x020000, 0x147d3a4d, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "065-a08.15l",	0x100000, 0x6b649aca, 3 | BRF_GRA },           //  5 Background Tiles
	{ "065-a07.16l",	0x100000, 0xc5dc8fc4, 3 | BRF_GRA },           //  6

	{ "065-a09.2h",		0x100000, 0xea05d52f, 4 | BRF_GRA },           //  7 Sprites
	{ "065-a10.2l",		0x100000, 0x96b91802, 4 | BRF_GRA },           //  8
	{ "065-a12.1h",		0x100000, 0x321ed07a, 4 | BRF_GRA },           //  9
	{ "065-a11.1l",		0x100000, 0x46da948e, 4 | BRF_GRA },           // 10

	{ "065-a06.1f",		0x200000, 0x5adbcee0, 5 | BRF_SND },           // 11 K054539 Samples

	{ "xmen_ada.nv",  0x000080, 0xa77a3891, BRF_OPT },
};

STD_ROM_PICK(xmena)
STD_ROM_FN(xmena)

struct BurnDriver BurnDrvXmena = {
	"xmena", "xmen", NULL, NULL, "1992",
	"X-Men (4 Players ver ADA)\0", NULL, "Konami", "GX065",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, xmenaRomInfo, xmenaRomName, NULL, NULL, XmenInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// X-Men (4 Players ver EBA)

static struct BurnRomInfo xmeneRomDesc[] = {
	{ "065-eba04.10d",	0x020000, 0x3588c5ec, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "065-eba05.10f",	0x020000, 0x79ce32f8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "065-a02.9d",		0x040000, 0xb31dc44c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "065-a03.9f",		0x040000, 0x13842fe6, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "065-a01.6f",		0x020000, 0x147d3a4d, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "065-a08.15l",	0x100000, 0x6b649aca, 3 | BRF_GRA },           //  5 Background Tiles
	{ "065-a07.16l",	0x100000, 0xc5dc8fc4, 3 | BRF_GRA },           //  6

	{ "065-a09.2h",		0x100000, 0xea05d52f, 4 | BRF_GRA },           //  7 Sprites
	{ "065-a10.2l",		0x100000, 0x96b91802, 4 | BRF_GRA },           //  8
	{ "065-a12.1h",		0x100000, 0x321ed07a, 4 | BRF_GRA },           //  9
	{ "065-a11.1l",		0x100000, 0x46da948e, 4 | BRF_GRA },           // 10

	{ "065-a06.1f",		0x200000, 0x5adbcee0, 5 | BRF_SND },           // 11 K054539 Samples

	{ "xmen_eba.nv",  0x000080, 0x37f8e77a, BRF_OPT },
};

STD_ROM_PICK(xmene)
STD_ROM_FN(xmene)

struct BurnDriver BurnDrvXmene = {
	"xmene", "xmen", NULL, NULL, "1992",
	"X-Men (4 Players ver EBA)\0", NULL, "Konami", "GX065",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, xmeneRomInfo, xmeneRomName, NULL, NULL, XmenInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// X-Men (4 Players ver JBA)

static struct BurnRomInfo xmenjRomDesc[] = {
	{ "065-jba04.10d",	0x020000, 0xd86cf5eb, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "065-jba05.10f",	0x020000, 0xabbc8126, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "065-a02.9d",		0x040000, 0xb31dc44c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "065-a03.9f",		0x040000, 0x13842fe6, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "065-a01.6f",		0x020000, 0x147d3a4d, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "065-a08.15l",	0x100000, 0x6b649aca, 3 | BRF_GRA },           //  5 Background Tiles
	{ "065-a07.16l",	0x100000, 0xc5dc8fc4, 3 | BRF_GRA },           //  6

	{ "065-a09.2h",		0x100000, 0xea05d52f, 4 | BRF_GRA },           //  7 Sprites
	{ "065-a10.2l",		0x100000, 0x96b91802, 4 | BRF_GRA },           //  8
	{ "065-a12.1h",		0x100000, 0x321ed07a, 4 | BRF_GRA },           //  9
	{ "065-a11.1l",		0x100000, 0x46da948e, 4 | BRF_GRA },           // 10

	{ "065-a06.1f",		0x200000, 0x5adbcee0, 5 | BRF_SND },           // 11 K054539 Samples

	{ "xmen_jba.nv",  0x000080, 0x7439cea7, BRF_OPT },
};

STD_ROM_PICK(xmenj)
STD_ROM_FN(xmenj)

struct BurnDriver BurnDrvXmenj = {
	"xmenj", "xmen", NULL, NULL, "1992",
	"X-Men (4 Players ver JBA)\0", NULL, "Konami", "GX065",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, xmenjRomInfo, xmenjRomName, NULL, NULL, XmenInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// X-Men (2 Players ver EAA)

static struct BurnRomInfo xmen2peRomDesc[] = {
	{ "065-eaa04.10d",	0x020000, 0x502861e7, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "065-eaa05.10f",	0x020000, 0xca6071bf, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "065-a02.9d",		0x040000, 0xb31dc44c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "065-a03.9f",		0x040000, 0x13842fe6, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "065-a01.6f",		0x020000, 0x147d3a4d, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "065-a08.15l",	0x100000, 0x6b649aca, 3 | BRF_GRA },           //  5 Background Tiles
	{ "065-a07.16l",	0x100000, 0xc5dc8fc4, 3 | BRF_GRA },           //  6

	{ "065-a09.2h",		0x100000, 0xea05d52f, 4 | BRF_GRA },           //  7 Sprites
	{ "065-a10.2l",		0x100000, 0x96b91802, 4 | BRF_GRA },           //  8
	{ "065-a12.1h",		0x100000, 0x321ed07a, 4 | BRF_GRA },           //  9
	{ "065-a11.1l",		0x100000, 0x46da948e, 4 | BRF_GRA },           // 10

	{ "065-a06.1f",		0x200000, 0x5adbcee0, 5 | BRF_SND },           // 11 K054539 Samples

	{ "xmen_eaa.nv",  0x000080, 0x1cbcb653, BRF_OPT },
};

STD_ROM_PICK(xmen2pe)
STD_ROM_FN(xmen2pe)

struct BurnDriver BurnDrvXmen2pe = {
	"xmen2pe", "xmen", NULL, NULL, "1992",
	"X-Men (2 Players ver EAA)\0", NULL, "Konami", "GX065",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, xmen2peRomInfo, xmen2peRomName, NULL, NULL, Xmen2pInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// X-Men (2 Players ver AAA)

static struct BurnRomInfo xmen2paRomDesc[] = {
	{ "065-aaa04.10d",	0x020000, 0x7f8b27c2, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "065-aaa05.10f",	0x020000, 0x841ed636, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "065-a02.9d",		0x040000, 0xb31dc44c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "065-a03.9f",		0x040000, 0x13842fe6, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "065-a01.6f",		0x020000, 0x147d3a4d, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "065-a08.15l",	0x100000, 0x6b649aca, 3 | BRF_GRA },           //  5 Background Tiles
	{ "065-a07.16l",	0x100000, 0xc5dc8fc4, 3 | BRF_GRA },           //  6

	{ "065-a09.2h",		0x100000, 0xea05d52f, 4 | BRF_GRA },           //  7 Sprites
	{ "065-a10.2l",		0x100000, 0x96b91802, 4 | BRF_GRA },           //  8
	{ "065-a12.1h",		0x100000, 0x321ed07a, 4 | BRF_GRA },           //  9
	{ "065-a11.1l",		0x100000, 0x46da948e, 4 | BRF_GRA },           // 10

	{ "065-a06.1f",		0x200000, 0x5adbcee0, 5 | BRF_SND },           // 11 K054539 Samples

	{ "xmen_aaa.nv",  0x000080, 0x750fd447, BRF_OPT },
};

STD_ROM_PICK(xmen2pa)
STD_ROM_FN(xmen2pa)

struct BurnDriver BurnDrvXmen2pa = {
	"xmen2pa", "xmen", NULL, NULL, "1992",
	"X-Men (2 Players ver AAA)\0", NULL, "Konami", "GX065",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, xmen2paRomInfo, xmen2paRomName, NULL, NULL, Xmen2pInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// X-Men (2 Players ver JAA)

static struct BurnRomInfo xmen2pjRomDesc[] = {
	{ "065-jaa04.10d",	0x020000, 0x66746339, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "065-jaa05.10f",	0x020000, 0x1215b706, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "065-a02.9d",		0x040000, 0xb31dc44c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "065-a03.9f",		0x040000, 0x13842fe6, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "065-a01.6f",		0x020000, 0x147d3a4d, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "065-a08.15l",	0x100000, 0x6b649aca, 3 | BRF_GRA },           //  5 Background Tiles
	{ "065-a07.16l",	0x100000, 0xc5dc8fc4, 3 | BRF_GRA },           //  6

	{ "065-a09.2h",		0x100000, 0xea05d52f, 4 | BRF_GRA },           //  7 Sprites
	{ "065-a10.2l",		0x100000, 0x96b91802, 4 | BRF_GRA },           //  8
	{ "065-a12.1h",		0x100000, 0x321ed07a, 4 | BRF_GRA },           //  9
	{ "065-a11.1l",		0x100000, 0x46da948e, 4 | BRF_GRA },           // 10

	{ "065-a06.1f",		0x200000, 0x5adbcee0, 5 | BRF_SND },           // 11 K054539 Samples

	{ "xmen_jaa.nv",  0x000080, 0x849a9e19, BRF_OPT },
};

STD_ROM_PICK(xmen2pj)
STD_ROM_FN(xmen2pj)

struct BurnDriver BurnDrvXmen2pj = {
	"xmen2pj", "xmen", NULL, NULL, "1992",
	"X-Men (2 Players ver JAA)\0", NULL, "Konami", "GX065",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, xmen2pjRomInfo, xmen2pjRomName, NULL, NULL, Xmen2pInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	288, 224, 4, 3
};


// X-Men (6 Players ver ECB)

static struct BurnRomInfo xmen6pRomDesc[] = {
	{ "065-ecb04.18g",	0x020000, 0x258eb21f, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "065-ecb05.18j",	0x020000, 0x25997bcd, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "065-a02.17g",	0x040000, 0xb31dc44c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "065-a03.17j",	0x040000, 0x13842fe6, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "065-a01.7b",		0x020000, 0x147d3a4d, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "065-a08.1l",		0x100000, 0x6b649aca, 3 | BRF_GRA },           //  5 Background Tiles
	{ "065-a07.1h",		0x100000, 0xc5dc8fc4, 3 | BRF_GRA },           //  6

	{ "065-a09.12l",	0x100000, 0xea05d52f, 4 | BRF_GRA },           //  7 Sprites
	{ "065-a10.17l",	0x100000, 0x96b91802, 4 | BRF_GRA },           //  8
	{ "065-a12.22h",	0x100000, 0x321ed07a, 4 | BRF_GRA },           //  9
	{ "065-a11.22l",	0x100000, 0x46da948e, 4 | BRF_GRA },           // 10

	{ "065-a06.1d",		0x200000, 0x5adbcee0, 5 | BRF_SND },           // 11 K054539 Samples

	{ "xmen_ecb.nv",  0x000080, 0x462c6e1a, BRF_OPT },
};

STD_ROM_PICK(xmen6p)
STD_ROM_FN(xmen6p)

struct BurnDriverD BurnDrvXmen6p = {
	"xmen6p", "xmen", NULL, NULL, "1992",
	"X-Men (6 Players ver ECB)\0", NULL, "Konami", "GX065",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 6, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, xmen6pRomInfo, xmen6pRomName, NULL, NULL, XmenInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 256, 4, 3
};


// X-Men (6 Players ver UCB)

static struct BurnRomInfo xmen6puRomDesc[] = {
	{ "065-ucb04.18g",	0x020000, 0x0f09b8e0, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "065-ucb05.18j",	0x020000, 0x867becbf, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "065-a02.17g",	0x040000, 0xb31dc44c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "065-a03.17j",	0x040000, 0x13842fe6, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "065-a01.7b",		0x020000, 0x147d3a4d, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "065-a08.1l",		0x100000, 0x6b649aca, 3 | BRF_GRA },           //  5 Background Tiles
	{ "065-a07.1h",		0x100000, 0xc5dc8fc4, 3 | BRF_GRA },           //  6

	{ "065-a09.12l",	0x100000, 0xea05d52f, 4 | BRF_GRA },           //  7 Sprites
	{ "065-a10.17l",	0x100000, 0x96b91802, 4 | BRF_GRA },           //  8
	{ "065-a12.22h",	0x100000, 0x321ed07a, 4 | BRF_GRA },           //  9
	{ "065-a11.22l",	0x100000, 0x46da948e, 4 | BRF_GRA },           // 10

	{ "065-a06.1d",		0x200000, 0x5adbcee0, 5 | BRF_SND },           // 11 K054539 Samples

	{ "xmen_ucb.nv",  0x000080, 0xf3d0f682, BRF_OPT },
};

STD_ROM_PICK(xmen6pu)
STD_ROM_FN(xmen6pu)

struct BurnDriverD BurnDrvXmen6pu = {
	"xmen6pu", "xmen", NULL, NULL, "1992",
	"X-Men (6 Players ver UCB)\0", NULL, "Konami", "GX065",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 6, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, xmen6puRomInfo, xmen6puRomName, NULL, NULL, XmenInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	256, 256, 4, 3
};
