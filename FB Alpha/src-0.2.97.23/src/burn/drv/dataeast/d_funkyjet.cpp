// FB Alpha Funky Jet driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "h6280_intf.h"
#include "deco16ic.h"
#include "msm6295.h"
#include "burn_ym2151.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvHucROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvSndROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvHucRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvPrtRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *flipscreen;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT16 DrvInputs[3];

static struct BurnInputInfo FunkyjetInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 14,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Funkyjet)

static struct BurnDIPInfo FunkyjetDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x02, 0x02, "Off"			},
	{0x14, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x14, 0x01, 0x1c, 0x00, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x1c, 0x10, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x1c, 0x1c, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x1c, 0x0c, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x1c, 0x14, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x1c, 0x04, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0x1c, 0x18, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0x1c, 0x08, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x14, 0x01, 0xe0, 0x00, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0xe0, 0x80, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0xe0, 0xe0, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0xe0, 0x60, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0xe0, 0x20, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0xe0, 0xc0, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0xe0, 0x40, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x01, 0x00, "Off"			},
	{0x15, 0x01, 0x01, 0x01, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x15, 0x01, 0x02, 0x00, "No"			},
	{0x15, 0x01, 0x02, 0x02, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x15, 0x01, 0x04, 0x04, "Off"			},
	{0x15, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x15, 0x01, 0x08, 0x08, "Off"			},
	{0x15, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x30, 0x10, "Easy"			},
	{0x15, 0x01, 0x30, 0x30, "Normal"		},
	{0x15, 0x01, 0x30, 0x20, "Hard"			},
	{0x15, 0x01, 0x30, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0xc0, 0x80, "1"			},
	{0x15, 0x01, 0xc0, 0xc0, "2"			},
	{0x15, 0x01, 0xc0, 0x40, "3"			},
	{0x15, 0x01, 0xc0, 0x00, "4"			},
};

STDDIPINFO(Funkyjet)

static struct BurnDIPInfo FunkyjetjDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x02, 0x02, "Off"			},
	{0x14, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x14, 0x01, 0x1c, 0x00, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x1c, 0x10, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x1c, 0x1c, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x1c, 0x0c, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x1c, 0x14, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x1c, 0x04, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0x1c, 0x18, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0x1c, 0x08, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x14, 0x01, 0xe0, 0x00, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0xe0, 0x80, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0xe0, 0xe0, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0xe0, 0x60, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0xe0, 0x20, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0xe0, 0xc0, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0xe0, 0x40, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x01, 0x01, "Off"			},
	{0x15, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x15, 0x01, 0x02, 0x00, "No"			},
	{0x15, 0x01, 0x02, 0x02, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x15, 0x01, 0x04, 0x04, "Off"			},
	{0x15, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x15, 0x01, 0x08, 0x08, "Off"			},
	{0x15, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x30, 0x10, "Easy"			},
	{0x15, 0x01, 0x30, 0x30, "Normal"		},
	{0x15, 0x01, 0x30, 0x20, "Hard"			},
	{0x15, 0x01, 0x30, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0xc0, 0x80, "1"			},
	{0x15, 0x01, 0xc0, 0xc0, "2"			},
	{0x15, 0x01, 0xc0, 0x40, "3"			},
	{0x15, 0x01, 0xc0, 0x00, "4"			},
};

STDDIPINFO(Funkyjetj)

static struct BurnDIPInfo SotsugyoDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xed, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x02, 0x02, "Off"			},
	{0x14, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x14, 0x01, 0x1c, 0x00, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x1c, 0x10, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x1c, 0x1c, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x1c, 0x0c, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x1c, 0x14, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x1c, 0x04, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0x1c, 0x18, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0x1c, 0x08, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x14, 0x01, 0xe0, 0x00, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0xe0, 0x80, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0xe0, 0xe0, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0xe0, 0x60, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0xe0, 0x20, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0xe0, 0xc0, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0xe0, 0x40, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    0, "Demo Sounds"		},
	{0x15, 0x01, 0x02, 0x02, "Off"			},
	{0x15, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x15, 0x01, 0x0c, 0x04, "1"			},
	{0x15, 0x01, 0x0c, 0x08, "2"			},
	{0x15, 0x01, 0x0c, 0x0c, "3"			},
	{0x15, 0x01, 0x0c, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x30, 0x30, "Easy"			},
	{0x15, 0x01, 0x30, 0x20, "Normal"		},
	{0x15, 0x01, 0x30, 0x10, "Hard"			},
	{0x15, 0x01, 0x30, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Free Play"		},
	{0x15, 0x01, 0x40, 0x40, "Off"			},
	{0x15, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Sotsugyo)

void __fastcall funkyjet_main_write_word(UINT32 address, UINT16 data)
{
	deco16_write_control_word(0, address, 0x300000, data)

	switch (address)
	{
		case 0x18010a:
			deco16_soundlatch = data & 0xff;
			h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
		break;
	}

	if ((address & 0xfff800) == 0x180000) {
		*((UINT16*)(DrvPrtRAM + (address & 0x7fe))) = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}
}

void __fastcall funkyjet_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x18010b:
			deco16_soundlatch = data;
			h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
		break;
	}

	if ((address & 0xfff800) == 0x180000) {
		DrvPrtRAM[(address & 0x7ff)^1] = data;
		return;
	}
}

UINT16 __fastcall funkyjet_main_read_word(UINT32 address)
{
	if ((address & 0xfff800) == 0x180000) {
		return deco16_146_funkyjet_prot_r(address);
	}

	return 0;
}

UINT8 __fastcall funkyjet_main_read_byte(UINT32 address)
{
	if ((address & 0xfff800) == 0x180000) {
		return deco16_146_funkyjet_prot_r(address) >> ((~address & 1) << 3);
	}

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	deco16SoundReset();

	deco16Reset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x080000;
	DrvHucROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x100000;
	DrvGfxROM1	= Next; Next += 0x100000;
	DrvGfxROM2	= Next; Next += 0x200000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x040000;

	DrvPalette	= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x004000;
	DrvHucRAM	= Next; Next += 0x002000;
	DrvSprRAM	= Next; Next += 0x000800;
	deco16_prot_ram	= (UINT16*)Next;
	DrvPrtRAM	= Next; Next += 0x000800;
	DrvPalRAM	= Next; Next += 0x000800;

	flipscreen	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvInit()
{
	BurnSetRefreshRate(58.00);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x00001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x00000,  1, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x00000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x00000,  4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x80000,  5, 1)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x00000,  6, 1)) return 1;

		deco74_decrypt_gfx(DrvGfxROM1, 0x080000);

		deco16_tile_decode(DrvGfxROM1, DrvGfxROM0, 0x080000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x080000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x100000, 0);
	}	

	deco16Init(1, 0, 1);
	deco16_set_graphics(DrvGfxROM0, 0x80000 * 2, DrvGfxROM1, 0x80000 * 2, NULL, 0);
	deco16_set_color_base(0, 256);
	deco16_set_color_base(1, 512);
	deco16_set_global_offsets(0, 8);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvPalRAM,			0x120000, 0x1207ff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0x140000, 0x143fff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x160000, 0x1607ff, SM_RAM);
	SekMapMemory(deco16_pf_ram[0],		0x320000, 0x321fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x322000, 0x323fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x340000, 0x340bff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x342000, 0x342bff, SM_RAM);
	SekSetWriteWordHandler(0,		funkyjet_main_write_word);
	SekSetWriteByteHandler(0,		funkyjet_main_write_byte);
	SekSetReadWordHandler(0,		funkyjet_main_read_word);
	SekSetReadByteHandler(0,		funkyjet_main_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 8055000, 0, NULL, 0.45, 1000000, 0.50, 0, 0);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.45, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.45, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();
	deco16Exit();

	SekExit();
	deco16SoundExit();

	BurnFree (AllMem);

	return 0;
}

static void DrvPaletteRecalc()
{
	UINT16 *p = (UINT16*)DrvPalRAM;

	for (INT32 i = 0; i < 0x800 / 2; i++) {
		INT32 b = (BURN_ENDIAN_SWAP_INT16(p[i]) >> 8) & 0x0f;
		INT32 g = (BURN_ENDIAN_SWAP_INT16(p[i]) >> 4) & 0x0f;
		INT32 r = (BURN_ENDIAN_SWAP_INT16(p[i]) >> 0) & 0x0f;

		r |= r << 4;
		g |= g << 4;
		b |= b << 4;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void draw_sprites()
{
	UINT16 *ram = (UINT16*)DrvSprRAM;

	for (INT32 offs = 0; offs < 0x400; offs += 4)
	{
		INT32 inc, mult;

		INT32 sy     = BURN_ENDIAN_SWAP_INT16(ram[offs + 0]);
		INT32 code   = BURN_ENDIAN_SWAP_INT16(ram[offs + 1]) & 0x3fff;
		INT32 sx     = BURN_ENDIAN_SWAP_INT16(ram[offs + 2]);

		if ((sy & 0x1000) && (nCurrentFrame & 1)) continue;

		INT32 color = (sx >> 9) & 0x1f;

		INT32 flipx = sy & 0x2000;
		INT32 flipy = sy & 0x4000;
		INT32 multi = (1 << ((sy & 0x0600) >> 9)) - 1;

		sx &= 0x01ff;
		sy &= 0x01ff;
		if (sx >= 320) sx -= 512;
		if (sy >= 256) sy -= 512;
		sy = 240 - sy;
		sx = 304 - sx;

		code &= ~multi;

		if (flipy) {
			inc = -1;
		} else {
			code += multi;
			inc = 1;
		}

		if (*flipscreen)
		{
			sy = 240 - sy;
			sx = 304 - sx;
			flipx = !flipx;
			flipy = !flipy;
			mult = 16;
		}
		else
			mult = -16;

		if (sx >= 320 || sx < -15 || sy >= nScreenHeight) continue;

		while (multi >= 0)
		{
			INT32 y = ((sy + mult * multi) & 0x1ff) - 8;
			INT32 c = (code - multi * inc) & 0x3fff;

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, c, sx, y, color, 4, 0, 0, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, c, sx, y, color, 4, 0, 0, DrvGfxROM2);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, c, sx, y, color, 4, 0, 0, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, c, sx, y, color, 4, 0, 0, DrvGfxROM2);
				}
			}

			multi--;
		}
	}
}

static INT32 DrvDraw()
{
//	if (DrvRecalc) {
		DrvPaletteRecalc();
		DrvRecalc = 0;
//	}

	deco16_pf12_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x300;
	}

	if (nBurnLayer & 1) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_OPAQUE);
	if (nBurnLayer & 2) deco16_draw_layer(0, pTransDraw, 0);

	if (nBurnLayer & 4) draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		deco16_prot_inputs = DrvInputs;
		memset (DrvInputs, 0xff, 2 * sizeof(UINT16)); 
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
		DrvInputs[2] = (DrvDips[1] << 8) | (DrvDips[0] << 0);
	}

	INT32 nInterleave = 256;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 14000000 / 58, 8055000 / 58 };
	INT32 nCyclesDone[2] = { 0, 0 };

	h6280NewFrame();
	
	SekOpen(0);
	h6280Open(0);

	deco16_vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);
		nCyclesDone[1] += h6280Run(nCyclesTotal[1] / nInterleave);

		if (i == 248) deco16_vblank = 0x08;
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			deco16SoundUpdate(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			deco16SoundUpdate(pSoundBuf, nSegmentLength);
		}
	}
	
	h6280Close();
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
		*pnMin = 0x029722;
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
	
		deco16SoundScan(nAction, pnMin);

		deco16Scan();
	}

	return 0;
}


// Funky Jet (World)

static struct BurnRomInfo funkyjetRomDesc[] = {
	{ "jk00.12f",		0x40000, 0x712089c1, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "jk01.13f",		0x40000, 0xbe3920d7, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "jk02.16f",		0x10000, 0x748c0bd8, 2 | BRF_PRG | BRF_ESS }, //  2 Huc6280 Code

	{ "mat02",		0x80000, 0xe4b94c7e, 3 | BRF_GRA },           //  3 Characters & Background Tiles

	{ "mat01",		0x80000, 0x24093a8d, 4 | BRF_GRA },           //  4 Sprites
	{ "mat00",		0x80000, 0xfbda0228, 4 | BRF_GRA },           //  5

	{ "jk03.15h",		0x20000, 0x69a0eaf7, 5 | BRF_SND },           //  6 OKI M6295 Samples
};

STD_ROM_PICK(funkyjet)
STD_ROM_FN(funkyjet)

struct BurnDriver BurnDrvFunkyjet = {
	"funkyjet", NULL, NULL, NULL, "1992",
	"Funky Jet (World)\0", NULL, "[Data East] (Mitchell license)", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM, 0,
	NULL, funkyjetRomInfo, funkyjetRomName, NULL, NULL, FunkyjetInputInfo, FunkyjetDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 240, 4, 3
};


// Funky Jet (Japan)

static struct BurnRomInfo funkyjetjRomDesc[] = {
	{ "jh00-2.11f",		0x40000, 0x5b98b700, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "jh01-2.13f",		0x40000, 0x21280220, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "jk02.16f",		0x10000, 0x748c0bd8, 2 | BRF_PRG | BRF_ESS }, //  2 Huc6280 Code

	{ "mat02",		0x80000, 0xe4b94c7e, 3 | BRF_GRA },           //  3 Characters & Background Tiles

	{ "mat01",		0x80000, 0x24093a8d, 4 | BRF_GRA },           //  4 Sprites
	{ "mat00",		0x80000, 0xfbda0228, 4 | BRF_GRA },           //  5

	{ "jk03.15h",		0x20000, 0x69a0eaf7, 5 | BRF_SND },           //  6 OKI M6295 Samples
};

STD_ROM_PICK(funkyjetj)
STD_ROM_FN(funkyjetj)

struct BurnDriver BurnDrvFunkyjetj = {
	"funkyjetj", "funkyjet", NULL, NULL, "1992",
	"Funky Jet (Japan)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_PLATFORM, 0,
	NULL, funkyjetjRomInfo, funkyjetjRomName, NULL, NULL, FunkyjetInputInfo, FunkyjetjDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 240, 4, 3
};


// Sotsugyo Shousho

static struct BurnRomInfo sotsugyoRomDesc[] = {
	{ "03.12f",		0x40000, 0xd175dfd1, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "04.13f",		0x40000, 0x2072477c, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "sb020.16f",		0x10000, 0xbaf5ec93, 2 | BRF_PRG | BRF_ESS }, //  2 Huc6280 Code

	{ "02.2f",		0x80000, 0x337b1451, 3 | BRF_GRA },           //  3 Characters & Background Tiles

	{ "01.4a",		0x80000, 0xfa10dd54, 4 | BRF_GRA },           //  4 Sprites
	{ "00.2a",		0x80000, 0xd35a14ef, 4 | BRF_GRA },           //  5

	{ "sb030.15h",		0x20000, 0x1ea43f48, 5 | BRF_SND },           //  6 OKI M6295 Samples
};

STD_ROM_PICK(sotsugyo)
STD_ROM_FN(sotsugyo)

struct BurnDriver BurnDrvSotsugyo = {
	"sotsugyo", NULL, NULL, NULL, "1995",
	"Sotsugyo Shousho\0", NULL, "Mitchell (Atlus license)", "DECO IC16",
	L"\u5352\u696D\u8A3C\u66F8\0Sotsugyo Shousho\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_MINIGAMES, 0,
	NULL, sotsugyoRomInfo, sotsugyoRomName, NULL, NULL, FunkyjetInputInfo, SotsugyoDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 240, 4, 3
};
