// FB Alpha Pac-Land driver module
// Based on MAME driver by Ernesto Corvi

#include "tiles_generic.h"
#include "m6809_intf.h"
#include "m6800_intf.h"
#include "namco_snd.h"
#include "burn_led.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvMainROM;
static UINT8 *DrvMCUROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvColPROM;
static UINT8 *DrvVidRAM0;
static UINT8 *DrvVidRAM1;
static UINT8 *DrvSprRAM;
static UINT8 *DrvMCURAM;
static UINT8 *DrvMCUIRAM;
static UINT8 *DrvSprMask;
static UINT8 *pPrioBitmap;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *flipscreen;

static INT32 mcu_reset;

static UINT16 *scroll;
static UINT8 *interrupt_enable;
static UINT8 *coin_lockout;
static UINT8 *palette_bank;

static INT32 watchdog;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[2];
static UINT8 DrvReset;

static struct BurnInputInfo PaclandInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 start"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 start"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 1,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Pacland)

static struct BurnDIPInfo PaclandDIPList[]=
{
	{0x0c, 0xff, 0xff, 0x80, NULL				},
	{0x0d, 0xff, 0xff, 0xff, NULL				},
	{0x0e, 0xff, 0xff, 0xff, NULL				},

// 	Not supported ATM
//	{0   , 0xfe, 0   ,    2, "Cabinet"			},
//	{0x0c, 0x01, 0x80, 0x80, "Upright"			},
//	{0x0c, 0x01, 0x80, 0x00, "Cocktail"			},

	{0   , 0xfe, 0   ,    4, "Coin B"			},
	{0x0d, 0x01, 0x03, 0x00, "3 Coins 1 Credits"		},
	{0x0d, 0x01, 0x03, 0x01, "2 Coins 1 Credits"		},
	{0x0d, 0x01, 0x03, 0x03, "1 Coin  1 Credits"		},
	{0x0d, 0x01, 0x03, 0x02, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x0d, 0x01, 0x04, 0x00, "Off"				},
	{0x0d, 0x01, 0x04, 0x04, "On"				},

	{0   , 0xfe, 0   ,    4, "Coin A"			},
	{0x0d, 0x01, 0x18, 0x00, "3 Coins 1 Credits"		},
	{0x0d, 0x01, 0x18, 0x08, "2 Coins 1 Credits"		},
	{0x0d, 0x01, 0x18, 0x18, "1 Coin  1 Credits"		},
	{0x0d, 0x01, 0x18, 0x10, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x0d, 0x01, 0x60, 0x40, "2"				},
	{0x0d, 0x01, 0x60, 0x60, "3"				},
	{0x0d, 0x01, 0x60, 0x20, "4"				},
	{0x0d, 0x01, 0x60, 0x00, "5"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x0d, 0x01, 0x80, 0x80, "Off"				},
	{0x0d, 0x01, 0x80, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Trip Select"			},
	{0x0e, 0x01, 0x01, 0x00, "Off"				},
	{0x0e, 0x01, 0x01, 0x01, "On"				},

	{0   , 0xfe, 0   ,    2, "Freeze"			},
	{0x0e, 0x01, 0x02, 0x02, "Off"				},
	{0x0e, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Round Select"			},
	{0x0e, 0x01, 0x04, 0x04, "Off"				},
	{0x0e, 0x01, 0x04, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x0e, 0x01, 0x18, 0x10, "B (Easy)"			},
	{0x0e, 0x01, 0x18, 0x18, "A (Average)"			},
	{0x0e, 0x01, 0x18, 0x08, "C (Hard)"			},
	{0x0e, 0x01, 0x18, 0x00, "D (Very Hard)"		},

	{0   , 0xfe, 0   ,    8, "Bonus Life"			},
	{0x0e, 0x01, 0xe0, 0xe0, "30K 80K 130K 300K 500K 1M"	},
	{0x0e, 0x01, 0xe0, 0x80, "30K 80K every 100K"		},
	{0x0e, 0x01, 0xe0, 0x40, "30K 80K 150K"			},
	{0x0e, 0x01, 0xe0, 0xc0, "30K 100K 200K 400K 600K 1M"	},
	{0x0e, 0x01, 0xe0, 0xa0, "40K 100K 180K 300K 500K 1M"	},
	{0x0e, 0x01, 0xe0, 0x20, "40K 100K 200K"		},
	{0x0e, 0x01, 0xe0, 0x00, "40K"				},
	{0x0e, 0x01, 0xe0, 0x60, "50K 150K every 200K"		},
};

STDDIPINFO(Pacland)

static void bankswitch(INT32 nBank)
{
	palette_bank[0] = (nBank & 0x18) >> 3;

	nBank = (nBank & 0x07) * 0x2000;

	M6809MapMemory(DrvMainROM + 0x10000 + nBank, 0x4000, 0x5fff, M6809_ROM);
}

static void pacland_main_write(UINT16 address, UINT8 data)
{
	if ((address & 0xfc00) == 0x6800) {
		namcos1_custom30_write(address & 0x3ff, data);
		return;
	}

	if ((address & 0xf000) == 0x7000) {
		INT32 bit = ~address & (1 << 11);
		interrupt_enable[0] = bit ? 1 : 0;
		if (!bit) M6809SetIRQLine(0, M6809_IRQSTATUS_NONE);
		return;
	}

	if ((address & 0xf000) == 0x8000) {
		INT32 bit = ~address & (1 << 11);
		if (!bit) HD63701Reset();
		mcu_reset = bit ^ (1<<11);
		return;
	}

	if ((address & 0xf000) == 0x9000) {
		*flipscreen = ~(address >> 11) & 1;
		BurnLEDSetFlipscreen(*flipscreen);
		return;
	}

	switch (address)
	{
		case 0x3800:
			scroll[0] = data;
		break;

		case 0x3801:
			scroll[0] = 0x100 | data;
		return;

		case 0x3a00:
			scroll[1] = data;
		return;

		case 0x3a01:
			scroll[1] = 0x100 | data;
		return;

		case 0x3c00:
			bankswitch(data);
		return;
	}
}

static UINT8 pacland_main_read(UINT16 address)
{
	if ((address & 0xfc00) == 0x6800) {
		return namcos1_custom30_read(address & 0x3ff);
	}

	if ((address & 0xf800) == 0x7800) {
		watchdog = 0;
		return 0;
	}

	return 0;
}

static void pacland_mcu_write(UINT16 address, UINT8 data)
{
	if ((address & 0xffe0) == 0x0000) {
		m6803_internal_registers_w(address, data);
		return;
	}

	if ((address & 0xff80) == 0x0080) {
		DrvMCUIRAM[address & 0x7f] = data;
		return;
	}

	if ((address & 0xfc00) == 0x1000) {
		namcos1_custom30_write(address & 0x3ff, data);
		return;
	}

	if ((address & 0xe000) == 0x2000) {
		watchdog = 0;
		return;
	}

	if ((address & 0xc000) == 0x4000) {
		INT32 bit = (~address >> 13) & 1;
		interrupt_enable[1] = bit;
		if (!bit) HD63701SetIRQLine(0, HD63701_IRQSTATUS_NONE);
		return;
	}
}

static UINT8 pacland_mcu_read(UINT16 address)
{
	if ((address & 0xffe0) == 0x0000) {
		return m6803_internal_registers_r(address);
	}

	if ((address & 0xff80) == 0x0080) {
		return DrvMCUIRAM[address & 0x7f];
	}

	if ((address & 0xfc00) == 0x1000) {
		return namcos1_custom30_read(address & 0x3ff);
	}

	switch (address)
	{
		case 0xd000:
			return (DrvDips[1] & 0xf0) | (DrvDips[2] >> 4);

		case 0xd001:
			return (DrvDips[1] << 4) | (DrvDips[2] & 0x0f);

		case 0xd002:
			return (DrvInputs[0] & 0xf0) | 0x0f;

		case 0xd003:
			return (DrvInputs[0] << 4) | 0x0f;
	}

	return 0;
}

static void pacland_mcu_write_port(UINT16 port, UINT8 data)
{
	switch (port)
	{
		case 0x100:
			coin_lockout[0] = data & 0x01;
			// coin counters ~data & 2 -> 0, ~data & 4 -> 1
		return;

		case 0x101:
			BurnLEDSetStatus(0, data & 0x08);
			BurnLEDSetStatus(1, data & 0x10);
		return;
	}
}

static UINT8 pacland_mcu_read_port(UINT16 port)
{
	switch (port)
	{
		case 0x100: return DrvInputs[1];
		case 0x101: return 0xff; // led status?
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvMainROM		= Next; Next += 0x020000;
	DrvMCUROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x008000;
	DrvGfxROM1		= Next; Next += 0x008000;
	DrvGfxROM2		= Next; Next += 0x020000;

	DrvColPROM		= Next; Next += 0x001400;

	DrvPalette		= (UINT32*)Next; Next += 0x0c00 * 4 * sizeof(UINT32);

	pPrioBitmap		= Next; Next += 288 * 224;

	DrvSprMask		= Next; Next += 0x000c00;

	AllRam			= Next;

	NamcoSoundProm		= Next; Next += 0x000400; // RAM

	DrvVidRAM0		= Next; Next += 0x001000;
	DrvVidRAM1		= Next; Next += 0x001000;
	DrvSprRAM		= Next; Next += 0x001800;

	DrvMCURAM		= Next; Next += 0x000800;
	DrvMCUIRAM		= Next; Next += 0x000080;

	flipscreen		= Next; Next += 0x000001;
	scroll			= (UINT16*)Next; Next += 0x000002 * sizeof(UINT16);
	interrupt_enable	= Next; Next += 0x000001;
	coin_lockout		= Next; Next += 0x000001;
	palette_bank		= Next; Next += 0x000001;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static INT32 DrvDoReset(INT32 full_reset)
{
	if (full_reset) {
		memset (AllRam, 0, RamEnd - AllRam);
	}

	M6809Open(0);
	M6809Reset();
	M6809Close();

	// open
	HD63701Reset();
	// close

	BurnLEDReset();

	watchdog = 0;
	mcu_reset = 0;

	return 0;
}

static void DrvGfxDecode()
{
	INT32 Plane[4]   = { 0, 4, RGN_FRAC(0x10000, 1, 2) + 0, RGN_FRAC(0x10000, 1, 2) + 4 };
	INT32 XOffs0[ 8] = { STEP4(8*8,1), STEP4(0*8,1) };
	INT32 XOffs1[16] = { STEP4(0*8,1), STEP4(8*8,1), STEP4(16*8,1), STEP4(24*8,1) };
	INT32 YOffs[16]  = { STEP8(0*8,8), STEP8(32*8,8) };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x10000);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, DrvGfxROM0, 0x02000);

	GfxDecode(0x0200, 2,  8,  8, Plane, XOffs0, YOffs, 0x080, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x02000);

	GfxDecode(0x0200, 2,  8,  8, Plane, XOffs0, YOffs, 0x080, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x10000);

	GfxDecode(0x0200, 4, 16, 16, Plane, XOffs1, YOffs, 0x200, tmp, DrvGfxROM2);

	BurnFree(tmp);
}

static void DrvPaletteInit() // Palette banks + Color tables
{
	UINT32 *tpal = (UINT32*)BurnMalloc(0x400 * sizeof(INT32));

	for (INT32 j = 0; j < 0x400; j++)
	{
		INT32 bit0, bit1, bit2, bit3, r,g,b;

		bit0 = (DrvColPROM[j+0x000] >> 0) & 0x01;
		bit1 = (DrvColPROM[j+0x000] >> 1) & 0x01;
		bit2 = (DrvColPROM[j+0x000] >> 2) & 0x01;
		bit3 = (DrvColPROM[j+0x000] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[j+0x000] >> 4) & 0x01;
		bit1 = (DrvColPROM[j+0x000] >> 5) & 0x01;
		bit2 = (DrvColPROM[j+0x000] >> 6) & 0x01;
		bit3 = (DrvColPROM[j+0x000] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[j+0x400] >> 0) & 0x01;
		bit1 = (DrvColPROM[j+0x400] >> 1) & 0x01;
		bit2 = (DrvColPROM[j+0x400] >> 2) & 0x01;
		bit3 = (DrvColPROM[j+0x400] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		tpal[j] = BurnHighCol(r, g, b, 0);
	}

	for (INT32 i = 0; i < 4; i++)
	{
		for (INT32 j = 0; j < 0x400; j++)
		{
			DrvPalette[(i * 0xc00) + 0x000 + j] = tpal[(i * 0x100) + DrvColPROM[j + 0x0800]];
			DrvPalette[(i * 0xc00) + 0x400 + j] = tpal[(i * 0x100) + DrvColPROM[j + 0x0c00]];
			DrvPalette[(i * 0xc00) + 0x800 + j] = tpal[(i * 0x100) + DrvColPROM[j + 0x1000]];

		}
	}

	BurnFree (tpal);
}

static void DrvCreateSpriteMask()
{
	memset (DrvSprMask, 0, 0xc00);

	for (INT32 i = 0; i < 0x400; i++) {
		INT32 p = DrvColPROM[0x1000 + i];

		if (p >= 0x80) DrvSprMask[i + 0x000] = 1;

		if ((p&0x7f) == 0x7f) DrvSprMask[i + 0x400] = 1;

		if (p < 0xf0 || p == 0xff) DrvSprMask[i + 0x800] = 1;
	}
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
		if (BurnLoadRom(DrvMainROM + 0x008000,  0, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x00c000,  1, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x010000,  2, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x014000,  3, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x018000,  4, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x01c000,  5, 1)) return 1;

		if (BurnLoadRom(DrvMCUROM  + 0x008000,  6, 1)) return 1;
		if (BurnLoadRom(DrvMCUROM  + 0x00f000,  7, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  9, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x004000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x008000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x00c000, 13, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x000000, 14, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x000400, 15, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x000800, 16, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x000c00, 17, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x001000, 18, 1)) return 1;

		DrvCreateSpriteMask();
		DrvPaletteInit();
		DrvGfxDecode();
	}

	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvVidRAM0,		0x0000, 0x0fff, M6809_RAM);
	M6809MapMemory(DrvVidRAM1,		0x1000, 0x1fff, M6809_RAM);
	M6809MapMemory(DrvSprRAM,		0x2000, 0x37ff, M6809_RAM);
	M6809MapMemory(DrvMainROM + 0x8000,	0x8000, 0xffff, M6809_ROM);
	M6809SetWriteHandler(pacland_main_write);
	M6809SetReadHandler(pacland_main_read);
	M6809Close();

	HD63701Init(1);
	// Open
	HD63701MapMemory(DrvMCUROM + 0x8000,	0x8000, 0xbfff, HD63701_ROM);
	HD63701MapMemory(DrvMCURAM,		0xc000, 0xc7ff, HD63701_RAM);
	HD63701MapMemory(DrvMCUROM + 0xf000,	0xf000, 0xffff, HD63701_ROM);
	HD63701SetWritePortHandler(pacland_mcu_write_port);
	HD63701SetReadPortHandler(pacland_mcu_read_port);
	HD63701SetWriteHandler(pacland_mcu_write);
	HD63701SetReadHandler(pacland_mcu_read);
	// Close

	NamcoSoundInit(49152000/2/1024, 8);
	NacmoSoundSetAllRoutes(0.50, BURN_SND_ROUTE_BOTH); // MAME uses 1.00, which is way too loud

	BurnLEDInit(2, LED_POSITION_BOTTOM_RIGHT, LED_SIZE_2x2, LED_COLOR_GREEN, 80);

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	M6809Exit();
	HD63701Exit();

	BurnLEDExit();

	NamcoSoundExit();
	NamcoSoundProm = NULL;

	BurnFree(AllMem);

	return 0;
}

static void draw_bg_layer()
{
	INT32 scrollx = (scroll[1] + 27) & 0x1ff;

	for (INT32 offs = 2 * 64; offs < (64 * 32) - (2 * 64); offs++)
	{
		INT32 sx = (offs & 0x3f) * 8;
		INT32 sy = (offs / 0x40) * 8;

		sx -= scrollx;
		if (sx < -7) sx += 512;
		if (sx >= nScreenWidth) continue;

		INT32 attr  = DrvVidRAM1[offs * 2 + 0] + (DrvVidRAM1[offs * 2 + 1] << 8);
		INT32 code  = (attr & 0x01ff);
		INT32 color =((attr & 0x01c0) >> 1) | ((attr & 0x3e00)>>9);
		INT32 flipy = (attr & 0x8000);
		INT32 flipx = (attr & 0x4000);

		if (*flipscreen) {
			sx = 280 - sx;
			sy = 216 - sy;
			flipy ^= 0x8000;
			flipx ^= 0x4000;
		}

		if (flipy) {
			if (flipx) {
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy-16, color, 2, 0x400, DrvGfxROM1);
			} else {
				Render8x8Tile_FlipY_Clip(pTransDraw, code, sx, sy-16, color, 2, 0x400, DrvGfxROM1);
			}
		} else {
			if (flipx) {
				Render8x8Tile_FlipX_Clip(pTransDraw, code, sx, sy-16, color, 2, 0x400, DrvGfxROM1);
			} else {
				Render8x8Tile_Clip(pTransDraw, code, sx, sy-16, color, 2, 0x400, DrvGfxROM1);
			}
		}
	}
}

static void draw_fg_layer(INT32 priority)
{
	UINT8 *coltable = DrvColPROM + 0x800;

	INT32 scrollx = (scroll[0] + 24) & 0x1ff;

	for (INT32 offs = 2 * 64; offs < (64 * 32) - (2 * 64); offs++)
	{
		INT32 sx = (offs & 0x3f) * 8;
		INT32 sy = (offs / 0x40) * 8;

		if (sy >= 40 && sy < 232) sx -= scrollx;
		if (sx < -7) sx += 512;
		if (sx >= nScreenWidth) continue;

		INT32 attr  = DrvVidRAM0[offs * 2 + 0] + (DrvVidRAM0[offs * 2 + 1] << 8);
		INT32 code  = (attr & 0x01ff);
		INT32 color =((attr & 0x01e0) >> 1) | ((attr & 0x1e00) >> 9);
		INT32 flipy = (attr & 0x8000);
		INT32 flipx = (attr & 0x4000);
		INT32 group = (attr & 0x2000) >> 13; // category

		if (*flipscreen) {
			sx = 280 - sx;
			sy = 216 - sy;
			flipy ^= 0x8000;
			flipx ^= 0x4000;
		}

		if (group == priority)
		{
			color <<= 2;
			sy -= 16;

			UINT8 *gfx = DrvGfxROM0 + (code * 8 * 8);
			INT32 flip = ((flipy) ? 0x38 : 0) + ((flipx) ? 0x07 : 0);

			for (INT32 y = 0; y < 8; y++, sy++) {
				if (sy < 0 || sy >= nScreenHeight) continue;

				for (INT32 x = 0; x < 8; x++, sx++) {
					if (sx < 0 || sx >= nScreenWidth) continue;

					INT32 pxl = gfx[((y*8)+x)^flip] + color;

					if ((coltable[pxl] & 0x7f) != 0x7f) {
						if (!pPrioBitmap[sy * nScreenWidth + sx]) {
							pTransDraw[sy * nScreenWidth + sx] = pxl;
						}
					}
				}

				sx -= 8;
			}
		}
	}
}

static void draw_sprite(INT32 code, INT32 sx, INT32 sy, INT32 color, INT32 flipx, INT32 flipy, INT32 prio)
{
	if (sx >= nScreenWidth || sy >= nScreenHeight || sx < -15 || sy < -15) return;

	INT32 flip = ((flipy) ? 0xf0 : 0) + ((flipx) ? 0x0f : 0);

	UINT8 *gfx = DrvGfxROM2 + code * 0x100;
	UINT8 *mask = DrvSprMask + prio * 0x400;

	if (prio == 0) prio = 1;
	else	prio = 0;

	color <<= 4;
//	color += 0x800;

	for (INT32 y = 0; y < 16; y++, sy++)
	{
		if (sy < 0 || sy >= nScreenHeight) continue;

		for (INT32 x = 0; x < 16; x++, sx++)
		{
			if (sx < 0 || sx >= nScreenWidth) continue;

			INT32 pxl = gfx[((y * 16) + x) ^ flip] + color;

			if (mask[pxl]) continue;

			pTransDraw[sy * nScreenWidth + sx] = pxl + 0x800;
			pPrioBitmap[sy * nScreenWidth + sx] = prio;
		}

		sx -= 16;
	}
}

static void draw_sprites(INT32 priority)
{
	for (INT32 offs = 0; offs < 0x80; offs += 2)
	{
		INT32 attr  = DrvSprRAM[0x1780 + offs];
		INT32 code  = DrvSprRAM[0x0780 + offs] + ((attr & 0x80) << 1);
		INT32 color = DrvSprRAM[0x0781 + offs] & 0x3f;
		INT32 sx    = (DrvSprRAM[0x0f81 + offs]) + 0x100*(DrvSprRAM[0x1781 + offs] & 1) - 47;
		INT32 sy    = 256 - DrvSprRAM[0xf80 + offs] + 9;
		INT32 flipx = (attr & 0x01);
		INT32 flipy = (attr & 0x02) >> 1;
		INT32 sizex = (attr & 0x04) >> 2;
		INT32 sizey = (attr & 0x08) >> 3;

		code &= ~((sizey << 1) | sizex);

		if (*flipscreen)
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		sy = ((sy - 16 * sizey) & 0xff) - 32;	// fix wraparound

		for (INT32 y = 0; y <= sizey; y++)
		{
			for (INT32 x = 0; x <= sizex; x++)
			{
				draw_sprite(code + ((y ^ (sizey * flipy)) * 2)  + (x ^ (sizex * flipx)), sx + 16*x - 24, sy + 16*y - 16, color, flipx, flipy, priority);
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvPaletteInit();
		DrvRecalc = 0;
	}

	memset (pPrioBitmap, 0, 288 * 224 * sizeof(UINT8));

	INT32 nLayer = nBurnLayer;

	draw_sprites(0);

	if (nLayer & 1) draw_bg_layer();

	if (nLayer & 2) draw_fg_layer(0);

	draw_sprites(1);

	if (nLayer & 4) draw_fg_layer(1);

	draw_sprites(2);

	BurnTransferCopy(DrvPalette + palette_bank[0] * 0xc00);

	BurnLEDRender();

	return 0;
}

static INT32 DrvFrame()
{
	watchdog++;
	if (watchdog > 180) {
		DrvDoReset(0);
	}

	if (DrvReset) {
		DrvDoReset(1);
	}

	{
		memset (DrvInputs, 0xff, 2);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}

		DrvInputs[0] = (DrvInputs[0] & 0x7f) | (DrvDips[0] & 0x80);
	//	if (coin_lockout[0]) DrvInputs[0] |= 0x0c;
	}

	INT32 nInterleave = nBurnSoundLen;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 49152000 / 32 / 60, 49152000 / 8 / 4 / 60 }; // refresh 60.606060
	INT32 nCyclesDone[2]  = { 0, 0 };

	M6809Open(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nSegment = nCyclesTotal[0] / nInterleave;

		nCyclesDone[0] += M6809Run(nSegment);
		if (i == (nInterleave - 1) && interrupt_enable[0]) M6809SetIRQLine(0, M6809_IRQSTATUS_ACK);
		nSegment = nCyclesTotal[1] / nInterleave;

		if (mcu_reset) {
			nCyclesDone[1] += nSegment;
		} else {
			nCyclesDone[1] += HD63701Run(nSegment);
			if (i == (nInterleave - 1) && interrupt_enable[1]) HD63701SetIRQLine(0, HD63701_IRQSTATUS_ACK);
		}
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			
			if (nSegmentLength) {
				NamcoSoundUpdate(pSoundBuf, nSegmentLength);
			}
			nSoundBufferPos += nSegmentLength;
		}
	}

	M6809Close();

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			NamcoSoundUpdate(pSoundBuf, nSegmentLength);
		}
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	return 1; // Broken :(

	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029707;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		M6809Scan(nAction);
		HD63701Scan(nAction);

		NamcoSoundScan(nAction, pnMin);

		BurnLEDScan(nAction, pnMin);

		SCAN_VAR(mcu_reset);
	}

	return 0;
}


// Pac-Land (World)

static struct BurnRomInfo paclandRomDesc[] = {
	{ "pl5_01b.8b",		0x4000, 0xb0ea7631, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "pl5_02.8d",		0x4000, 0xd903e84e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "pl1_3.8e",		0x4000, 0xaa9fa739, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "pl1_4.8f",		0x4000, 0x2b895a90, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "pl1_5.8h",		0x4000, 0x7af66200, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "pl3_6.8j",		0x4000, 0x2ffe3319, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "pl1_7.3e",		0x2000, 0x8c5becae, 2 | BRF_PRG | BRF_ESS }, //  6 HD63701 Code
	{ "cus60-60a1.mcu",	0x1000, 0x076ea82a, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "pl2_12.6n",		0x2000, 0xa63c8726, 3 | BRF_GRA },           //  8 Foreground Tiles

	{ "pl4_13.6t",		0x2000, 0x3ae582fd, 4 | BRF_GRA },           //  9 Background Tiles

	{ "pl1-9.6f",		0x4000, 0xf5d5962b, 5 | BRF_GRA },           // 10 Sprites
	{ "pl1-8.6e",		0x4000, 0xa2ebfa4a, 5 | BRF_GRA },           // 11
	{ "pl1-10.7e",		0x4000, 0xc7cf1904, 5 | BRF_GRA },           // 12
	{ "pl1-11.7f",		0x4000, 0x6621361a, 5 | BRF_GRA },           // 13

	{ "pl1-2.1t",		0x0400, 0x472885de, 6 | BRF_GRA },           // 14 Color PROMs
	{ "pl1-1.1r",		0x0400, 0xa78ebdaf, 6 | BRF_GRA },           // 15
	{ "pl1-5.5t",		0x0400, 0x4b7ee712, 6 | BRF_GRA },           // 16
	{ "pl1-4.4n",		0x0400, 0x3a7be418, 6 | BRF_GRA },           // 17
	{ "pl1-3.6l",		0x0400, 0x80558da8, 6 | BRF_GRA },           // 18
};

STD_ROM_PICK(pacland)
STD_ROM_FN(pacland)

struct BurnDriver BurnDrvPacland = {
	"pacland", NULL, NULL, NULL, "1984",
	"Pac-Land (World)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, paclandRomInfo, paclandRomName, NULL, NULL, PaclandInputInfo, PaclandDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0xc00 * 4,
	288, 224, 4, 3
};


// Pac-Land (Japan new)

static struct BurnRomInfo paclandjRomDesc[] = {
	{ "pl6_01.8b",		0x4000, 0x4c96e11c, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "pl6_02.8d",		0x4000, 0x8cf5bd8d, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "pl1_3.8e",		0x4000, 0xaa9fa739, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "pl1_4.8f",		0x4000, 0x2b895a90, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "pl1_5.8h",		0x4000, 0x7af66200, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "pl1_6.8j",		0x4000, 0xb01e59a9, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "pl1_7.3e",		0x2000, 0x8c5becae, 2 | BRF_PRG | BRF_ESS }, //  6 HD63701 Code
	{ "cus60-60a1.mcu",	0x1000, 0x076ea82a, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "pl6_12.6n",		0x2000, 0xc8cb61ab, 3 | BRF_GRA },           //  8 Foreground Tiles

	{ "pl1_13.6t",		0x2000, 0x6c5ed9ae, 4 | BRF_GRA },           //  9 Background Tiles

	{ "pl1_9b.6f",		0x4000, 0x80768a87, 5 | BRF_GRA },           // 10 Sprites
	{ "pl1_8.6e",		0x4000, 0x2b20e46d, 5 | BRF_GRA },           // 11
	{ "pl1_10b.7e",		0x4000, 0xffd9d66e, 5 | BRF_GRA },           // 12
	{ "pl1_11.7f",		0x4000, 0xc59775d8, 5 | BRF_GRA },           // 13

	{ "pl1-2.1t",		0x0400, 0x472885de, 6 | BRF_GRA },           // 14 Color PROMs
	{ "pl1-1.1r",		0x0400, 0xa78ebdaf, 6 | BRF_GRA },           // 15
	{ "pl1-5.5t",		0x0400, 0x4b7ee712, 6 | BRF_GRA },           // 16
	{ "pl1-4.4n",		0x0400, 0x3a7be418, 6 | BRF_GRA },           // 17
	{ "pl1-3.6l",		0x0400, 0x80558da8, 6 | BRF_GRA },           // 18
};

STD_ROM_PICK(paclandj)
STD_ROM_FN(paclandj)

struct BurnDriver BurnDrvPaclandj = {
	"paclandj", "pacland", NULL, NULL, "1984",
	"Pac-Land (Japan new)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, paclandjRomInfo, paclandjRomName, NULL, NULL,PaclandInputInfo, PaclandDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0xc00 * 4,
	288, 224, 4, 3
};


// Pac-Land (Japan old)

static struct BurnRomInfo paclandjoRomDesc[] = {
	{ "pl1_1.8b",		0x4000, 0xf729fb94, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "pl1_2.8d",		0x4000, 0x5c66eb6f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "pl1_3.8e",		0x4000, 0xaa9fa739, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "pl1_4.8f",		0x4000, 0x2b895a90, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "pl1_5.8h",		0x4000, 0x7af66200, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "pl1_6.8j",		0x4000, 0xb01e59a9, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "pl1_7.3e",		0x2000, 0x8c5becae, 2 | BRF_PRG | BRF_ESS }, //  6 HD63701 Code
	{ "cus60-60a1.mcu",	0x1000, 0x076ea82a, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "pl1_12.6n",		0x2000, 0xc159fbce, 3 | BRF_GRA },           //  8 Foreground Tiles

	{ "pl1_13.6t",		0x2000, 0x6c5ed9ae, 4 | BRF_GRA },           //  9 Background Tiles

	{ "pl1_9b.6f",		0x4000, 0x80768a87, 5 | BRF_GRA },           // 10 Sprites
	{ "pl1_8.6e",		0x4000, 0x2b20e46d, 5 | BRF_GRA },           // 11
	{ "pl1_10b.7e",		0x4000, 0xffd9d66e, 5 | BRF_GRA },           // 12
	{ "pl1_11.7f",		0x4000, 0xc59775d8, 5 | BRF_GRA },           // 13

	{ "pl1-2.1t",		0x0400, 0x472885de, 6 | BRF_GRA },           // 14 Color PROMs
	{ "pl1-1.1r",		0x0400, 0xa78ebdaf, 6 | BRF_GRA },           // 15
	{ "pl1-5.5t",		0x0400, 0x4b7ee712, 6 | BRF_GRA },           // 16
	{ "pl1-4.4n",		0x0400, 0x3a7be418, 6 | BRF_GRA },           // 17
	{ "pl1-3.6l",		0x0400, 0x80558da8, 6 | BRF_GRA },           // 18
};

STD_ROM_PICK(paclandjo)
STD_ROM_FN(paclandjo)

struct BurnDriver BurnDrvPaclandjo = {
	"paclandjo", "pacland", NULL, NULL, "1984",
	"Pac-Land (Japan old)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, paclandjoRomInfo, paclandjoRomName, NULL, NULL, PaclandInputInfo, PaclandDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0xc00 * 4,
	288, 224, 4, 3
};


// Pac-Land (Japan older)

static struct BurnRomInfo paclandjo2RomDesc[] = {
	{ "pl1_1.8b",		0x4000, 0xf729fb94, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "pl1_2.8d",		0x4000, 0x5c66eb6f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "pl1_3.8e",		0x4000, 0xaa9fa739, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "pl1_4.8f",		0x4000, 0x2b895a90, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "pl1_5.8h",		0x4000, 0x7af66200, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "pl1_6.8j",		0x4000, 0xb01e59a9, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "pl1_7.3e",		0x2000, 0x8c5becae, 2 | BRF_PRG | BRF_ESS }, //  6 HD63701 Code
	{ "cus60-60a1.mcu",	0x1000, 0x076ea82a, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "pl1_12.6n",		0x2000, 0xc159fbce, 3 | BRF_GRA },           //  8 Foreground Tiles

	{ "pl1_13.6t",		0x2000, 0x6c5ed9ae, 4 | BRF_GRA },           //  9 Background Tiles

	{ "pl1_9.6f",		0x4000, 0x80768a87, 5 | BRF_GRA },           // 10 Sprites
	{ "pl1_8.6e",		0x4000, 0x2b20e46d, 5 | BRF_GRA },           // 11
	{ "pl1_10.7e",		0x4000, 0xc62660e8, 5 | BRF_GRA },           // 12
	{ "pl1_11.7f",		0x4000, 0xc59775d8, 5 | BRF_GRA },           // 13
	
	{ "pl1-2.1t",		0x0400, 0x472885de, 6 | BRF_GRA },           // 14 Color PROMs
	{ "pl1-1.1r",		0x0400, 0xa78ebdaf, 6 | BRF_GRA },           // 15
	{ "pl1-5.5t",		0x0400, 0x4b7ee712, 6 | BRF_GRA },           // 16
	{ "pl1-4.4n",		0x0400, 0x3a7be418, 6 | BRF_GRA },           // 17
	{ "pl1-3.6l",		0x0400, 0x80558da8, 6 | BRF_GRA },           // 18
};

STD_ROM_PICK(paclandjo2)
STD_ROM_FN(paclandjo2)

struct BurnDriver BurnDrvPaclandjo2 = {
	"paclandjo2", "pacland", NULL, NULL, "1984",
	"Pac-Land (Japan older)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, paclandjo2RomInfo, paclandjo2RomName, NULL, NULL, PaclandInputInfo, PaclandDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0xc00 * 4,
	288, 224, 4, 3
};


// Pac-Land (Midway)

static struct BurnRomInfo paclandmRomDesc[] = {
	{ "pl1-1",		0x4000, 0xa938ae99, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "pl1-2",		0x4000, 0x3fe43bb5, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "pl1_3.8e",		0x4000, 0xaa9fa739, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "pl1_4.8f",		0x4000, 0x2b895a90, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "pl1_5.8h",		0x4000, 0x7af66200, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "pl1_6.8j",		0x4000, 0xb01e59a9, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "pl1_7.3e",		0x2000, 0x8c5becae, 2 | BRF_PRG | BRF_ESS }, //  6 HD63701 Code
	{ "cus60-60a1.mcu",	0x1000, 0x076ea82a, 2 | BRF_PRG | BRF_ESS }, //  7

	{ "pl1_12.6n",		0x2000, 0xc159fbce, 3 | BRF_GRA },           //  8 Foreground Tiles

	{ "pl1_13.6t",		0x2000, 0x6c5ed9ae, 4 | BRF_GRA },           //  9 Background Tiles

	{ "pl1-9.6f",		0x4000, 0xf5d5962b, 5 | BRF_GRA },           // 10 Sprites
	{ "pl1-8.6e",		0x4000, 0xa2ebfa4a, 5 | BRF_GRA },           // 11
	{ "pl1-10.7e",		0x4000, 0xc7cf1904, 5 | BRF_GRA },           // 12
	{ "pl1-11.7f",		0x4000, 0x6621361a, 5 | BRF_GRA },           // 13

	{ "pl1-2.1t",		0x0400, 0x472885de, 6 | BRF_GRA },           // 14 Color PROMs
	{ "pl1-1.1r",		0x0400, 0xa78ebdaf, 6 | BRF_GRA },           // 15
	{ "pl1-5.5t",		0x0400, 0x4b7ee712, 6 | BRF_GRA },           // 16
	{ "pl1-4.4n",		0x0400, 0x3a7be418, 6 | BRF_GRA },           // 17
	{ "pl1-3.6l",		0x0400, 0x80558da8, 6 | BRF_GRA },           // 18
};

STD_ROM_PICK(paclandm)
STD_ROM_FN(paclandm)

struct BurnDriver BurnDrvPaclandm = {
	"paclandm", "pacland", NULL, NULL, "1984",
	"Pac-Land (Midway)\0", NULL, "Namco (Bally Midway license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, paclandmRomInfo, paclandmRomName, NULL, NULL, PaclandInputInfo, PaclandDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0xc00 * 4,
	288, 224, 4, 3
};
