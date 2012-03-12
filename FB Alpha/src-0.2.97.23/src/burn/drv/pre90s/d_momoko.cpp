// FB Alpha Momoko 120% driver module
// Based on MAME driver by Uki

#include "tiles_generic.h"
#include "burn_ym2203.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM1a;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvBankROM;
static UINT8 *DrvBgCPROM;
static UINT8 *DrvFgMPROM;
static UINT8 *DrvColPROM;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvSprRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvVidRAM;

static UINT8 *DrvTransTab[4];

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *flipscreen;
static UINT8 *fg_scrolly;
static UINT8 *fg_scrollx;
static UINT8 *fg_select;
static UINT8 *tx_scrolly;
static UINT8 *tx_mode;
static UINT8 *bg_scrolly;
static UINT8 *bg_scrollx;
static UINT8 *bg_select;
static UINT8 *bg_priority;
static UINT8 *bg_bank;

static INT32 watchdog;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static struct BurnInputInfo MomokoInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 7,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Momoko)

static struct BurnDIPInfo MomokoDIPList[]=
{
	{0x10, 0xff, 0xff, 0x7f, NULL			},
	{0x11, 0xff, 0xff, 0xef, NULL			},
	{0x12, 0xff, 0xff, 0x00, NULL			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x10, 0x01, 0x03, 0x03, "3"			},
	{0x10, 0x01, 0x03, 0x02, "4"			},
	{0x10, 0x01, 0x03, 0x01, "5"			},
	{0x10, 0x01, 0x03, 0x00, "255 (Cheat)"		},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x10, 0x01, 0x1c, 0x10, "5 Coins 1 Credits"	},
	{0x10, 0x01, 0x1c, 0x14, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0x1c, 0x18, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0x1c, 0x1c, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0x1c, 0x0c, "1 Coin  2 Credits"	},
	{0x10, 0x01, 0x1c, 0x04, "2 Coins 5 Credits"	},
	{0x10, 0x01, 0x1c, 0x08, "1 Coin  5 Credits"	},
	{0x10, 0x01, 0x1c, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x10, 0x01, 0x60, 0x40, "Easy"			},
	{0x10, 0x01, 0x60, 0x60, "Normal"		},
	{0x10, 0x01, 0x60, 0x20, "Hard"			},
	{0x10, 0x01, 0x60, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x11, 0x01, 0x03, 0x01, "20k"			},
	{0x11, 0x01, 0x03, 0x03, "30k"			},
	{0x11, 0x01, 0x03, 0x02, "50k"			},
	{0x11, 0x01, 0x03, 0x00, "100k"			},

//	{0   , 0xfe, 0   ,    2, "Cabinet"		},
//	{0x11, 0x01, 0x10, 0x00, "Upright"		},
//	{0x11, 0x01, 0x10, 0x10, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x11, 0x01, 0x20, 0x00, "Off"			},
	{0x11, 0x01, 0x20, 0x20, "On"			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen (Fake)"	},
//	{0x12, 0x01, 0x01, 0x00, "Off"			},
//	{0x12, 0x01, 0x01, 0x01, "On"			},
};

STDDIPINFO(Momoko)

static inline void palette_write(UINT16 offset)
{
	UINT16 p = DrvPalRAM[offset + 1] + (DrvPalRAM[offset + 0] * 256);

	INT32 r = (p >> 8) & 0x0f;
	INT32 g = (p >> 4) & 0x0f;
	INT32 b = (p >> 0) & 0x0f;

	r |= r << 4;
	g |= g << 4;
	b |= b << 4;

	DrvPalette[offset/2] = BurnHighCol(r, g, b, 0);
}

static void bankswitch(INT32 data)
{
	*bg_bank = data;

	ZetMapArea(0xf000, 0xffff, 0, DrvBankROM + (data & 0x1f) * 0x1000);
	ZetMapArea(0xf000, 0xffff, 2, DrvBankROM + (data & 0x1f) * 0x1000);
}

void __fastcall momoko_main_write(UINT16 address, UINT8 data)
{
	if ((address & 0xf800) == 0xd800) {
		DrvPalRAM[(address & 0x3ff)] = data;
		palette_write(address & 0x3fe);
		return;
	}

	switch (address)
	{
		case 0xd402:
			*flipscreen = data & 0x01;
		return;

		case 0xd404:
			watchdog = 0;
		return;

		case 0xd406:
			*soundlatch = data;
		return;

		case 0xdc00:
			*fg_scrolly = data;
		return;

		case 0xdc01:
			*fg_scrollx = data;
		return;

		case 0xdc02:
			*fg_select = data;
		return;

		case 0xe800:
			*tx_scrolly = data;
		return;

		case 0xe801:
			*tx_mode = data;
		return;

		case 0xf000:
		case 0xf001:
			bg_scrolly[address & 1] = data;
		return;

		case 0xf002:
		case 0xf003:
			bg_scrollx[address & 1] = data;
		return;

		case 0xf004:
			bankswitch(data);
		return;

		case 0xf006:
			*bg_select = data;
		return;

		case 0xf007:
			*bg_priority = data & 0x01;
		return;
	}
}

UINT8 __fastcall momoko_main_read(UINT16 address)
{
	switch (address)
	{
		case 0xd400:
			return DrvInputs[0];

		case 0xd402:
			return DrvInputs[1];

		case 0xd406:
			return (DrvInputs[2] & 0x80) | (DrvDips[0] & 0x7f);

		case 0xd407:
			return DrvDips[1];
	}

	return 0;
}

void __fastcall momoko_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xa000:
		case 0xa001:
			BurnYM2203Write(0, address & 1, data);
		return;

		case 0xc000:
		case 0xc001:
			BurnYM2203Write(1, address & 1, data);
		return;
	}
}

UINT8 __fastcall momoko_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xa000:
		case 0xa001:
			return BurnYM2203Read(0, address & 1);

		case 0xc000:
		case 0xc001:
			return BurnYM2203Read(1, address & 1);
	}

	return 0;
}

static UINT8 momoko_sound_read_port_A(UINT32)
{
	return *soundlatch;
}

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 2500000;
}

inline static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 2500000.0;
}

static INT32 DrvDoReset(INT32 clear)
{
	if (clear) {
		memset(AllRam, 0, RamEnd - AllRam);
	}

	ZetOpen(0);
	ZetReset();
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	BurnYM2203Reset();

	watchdog = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0		= Next; Next += 0x00c000;
	DrvZ80ROM1		= Next; Next += 0x008000;

	DrvBankROM		= Next; Next += 0x020000;
	DrvBgCPROM		= Next; Next += 0x002000;
	DrvFgMPROM		= Next; Next += 0x004000;
	DrvColPROM		= Next; Next += 0x000120;

	DrvGfxROM0		= Next; Next += 0x008000;
	DrvGfxROM1		= Next; Next += 0x080000;
	DrvGfxROM1a		= Next; Next += 0x020000;
	DrvGfxROM2		= Next; Next += 0x008000;
	DrvGfxROM3		= Next; Next += 0x040000;

	DrvTransTab[0]		= Next; Next += 0x008000 / 0x08;
	DrvTransTab[1]		= Next; Next += 0x000200;
	DrvTransTab[2]		= Next; Next += 0x008000 / 0x40;
	DrvTransTab[3]		= Next; Next += 0x040000 / 0x80;

	DrvPalette		= (UINT32*)Next; Next += 0x0200 * sizeof(UINT32);

	AllRam			= Next;

	DrvSprRAM		= Next; Next += 0x000100;
	DrvZ80RAM0		= Next; Next += 0x001000;
	DrvZ80RAM1		= Next; Next += 0x000800;
	DrvPalRAM		= Next; Next += 0x000400;
	DrvVidRAM		= Next; Next += 0x000400;

	soundlatch		= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;

	fg_scrolly		= Next; Next += 0x000001;
	fg_scrollx		= Next; Next += 0x000001;
	fg_select		= Next; Next += 0x000001;
	tx_scrolly		= Next; Next += 0x000001;
	tx_mode			= Next; Next += 0x000001;
	bg_scrolly		= Next; Next += 0x000002;
	bg_scrollx		= Next; Next += 0x000002;
	bg_select		= Next; Next += 0x000001;
	bg_priority		= Next; Next += 0x000001;
	bg_bank			= Next; Next += 0x000001;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Planes[4]  = { 4,0,12,8 };
	INT32 XOffs0[8]  = { 0, 1, 2, 3, 256*8*8+0, 256*8*8+1, 256*8*8+2, 256*8*8+3 };
	INT32 YOffs0[8]  = { STEP8(0, 8) };
	INT32 XOffs1[8]  = { 0, 1, 2, 3, 4096*8+0, 4096*8+1, 4096*8+2, 4096*8+3 };
	INT32 YOffs1[16] = { STEP16(0, 16) };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x20000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x02000);

	GfxDecode(0x0200, 2, 8, 8, Planes, XOffs0, YOffs0, 0x040, tmp, DrvGfxROM0);

	memcpy (DrvGfxROM1a, DrvGfxROM1, 0x20000);

	GfxDecode(0x2000, 4, 8, 8, Planes, XOffs1, YOffs1, 0x080, DrvGfxROM1a, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x02000);

	GfxDecode(0x0800, 2, 8, 1, Planes, XOffs0, YOffs0, 0x008, tmp, DrvGfxROM2);

	memcpy (tmp, DrvGfxROM3, 0x10000);

	GfxDecode(0x0800, 4, 8, 16, Planes, XOffs1, YOffs1, 0x100, tmp, DrvGfxROM3);

	BurnFree(tmp);

	return 0;
}

static void DrvFillTransTab(INT32 tab, UINT8 *gfx, INT32 len, INT32 size)
{
	memset (DrvTransTab[tab], 1, len / size);

	for (INT32 i = 0; i < len; i+= size)
	{
		for (INT32 j = 0; j < size; j++)
		{
			if (gfx[i+j])
			{
				DrvTransTab[tab][i/size] = 0;
				break;
			}
		}
	}
}

static void DrvFillTransMask()
{
	for (INT32 i = 0x100; i < 0x200; i+=0x10) {
		memset (DrvTransTab[1] + i + 8, 0xff, 8);
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
		if (BurnLoadRom(DrvZ80ROM0 + 0x00000,  0, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x08000,  1, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM1 + 0x00000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,  3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x00000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x00001,  5, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x00000,  6, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  7, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x00001,  8, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x10000,  9, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x10001, 10, 2)) return 1;

		if (BurnLoadRom(DrvBankROM + 0x00000, 11, 1)) return 1;
		if (BurnLoadRom(DrvBankROM + 0x08000, 12, 1)) return 1;
		if (BurnLoadRom(DrvBankROM + 0x10000, 13, 1)) return 1;
		if (BurnLoadRom(DrvBankROM + 0x18000, 14, 1)) return 1;

		if (BurnLoadRom(DrvBgCPROM + 0x00000, 15, 1)) return 1;

		if (BurnLoadRom(DrvFgMPROM + 0x00000, 16, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x00000, 17, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x00100, 18, 1)) return 1;

		if (DrvGfxDecode()) return 1;

		DrvFillTransTab(0, DrvGfxROM0, 0x008000, 0x08);
		DrvFillTransTab(3, DrvGfxROM3, 0x040000, 0x80);
		DrvFillTransTab(2, DrvGfxROM2, 0x008000, 0x40);
		DrvFillTransMask();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xcfff, 0, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xcfff, 1, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xcfff, 2, DrvZ80RAM0);
	ZetMapArea(0xd000, 0xd0ff, 0, DrvSprRAM);
	ZetMapArea(0xd000, 0xd0ff, 1, DrvSprRAM);
	ZetMapArea(0xd000, 0xd0ff, 2, DrvSprRAM);
	ZetMapArea(0xd800, 0xdbff, 0, DrvPalRAM);
//	ZetMapArea(0xd800, 0xdbff, 1, DrvPalRAM);
	ZetMapArea(0xd800, 0xdbff, 2, DrvPalRAM);
	ZetMapArea(0xe000, 0xe3ff, 0, DrvVidRAM);
	ZetMapArea(0xe000, 0xe3ff, 1, DrvVidRAM);
	ZetMapArea(0xe000, 0xe3ff, 2, DrvVidRAM);
	ZetSetWriteHandler(momoko_main_write);
	ZetSetReadHandler(momoko_main_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM1);
	ZetSetWriteHandler(momoko_sound_write);
	ZetSetReadHandler(momoko_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2203Init(2, 1250000, NULL, DrvSynchroniseStream, DrvGetTime, 0);
	BurnYM2203SetPorts(1, momoko_sound_read_port_A, NULL, NULL, NULL);
	BurnYM2203SetVolumeShift(3);
	BurnTimerAttachZet(2500000);

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();

	BurnYM2203Exit();

	BurnFree (AllMem);

	return 0;
}

static void draw_bg_layer_pri()
{
	INT32 dx   = ~bg_scrollx[0] & 7;
	INT32 dy   = ~bg_scrolly[0] & 7;
	INT32 rx   = (bg_scrollx[0] + bg_scrollx[1] * 256) >> 3;
	INT32 ry   = (bg_scrolly[0] + bg_scrolly[1] * 256) >> 3;
	INT32 bank = (*bg_select & 0x0f) * 0x200;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f);
		INT32 sy = (offs / 0x20);

		INT32 ofst  = (((ry + sy + 2) & 0x3ff) << 7) + ((rx + sx) & 0x7f);
		INT32 code  = DrvBankROM[ofst] + bank;
		INT32 color = DrvBgCPROM[code + (*bg_priority * 0x100)];

		if (color & 0x10) continue;

		sx = (sx * 8) + dx - 6 - 8;
		sy = (sy * 8) + dy + 9 - 16;

		RenderTileTranstab(pTransDraw, DrvGfxROM1, code, ((color&0x0f)<<4)+0x100, 0, sx, sy, 0, 0, 8, 8, DrvTransTab[1]);
	}
}

static void draw_bg_layer()
{
	INT32 dx   = ~bg_scrollx[0] & 7;
	INT32 dy   = ~bg_scrolly[0] & 7;
	INT32 rx   = (bg_scrollx[0] + bg_scrollx[1] * 256) >> 3;
	INT32 ry   = (bg_scrolly[0] + bg_scrolly[1] * 256) >> 3;
	INT32 bank = (*bg_select & 0x0f) * 0x200;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f);
		INT32 sy = (offs / 0x20);

		INT32 ofst  = (((ry + sy + 2) & 0x3ff) << 7) + ((rx + sx) & 0x7f);
		INT32 code  = DrvBankROM[ofst] + bank;
		INT32 color = DrvBgCPROM[code + (*bg_priority * 0x100)] & 0x0f;

		sx = (sx * 8) + dx - 6;
		sy = (sy * 8) + dy + 9 - 16;

		Render8x8Tile_Clip(pTransDraw, code, sx - 8, sy, color, 4, 0x100, DrvGfxROM1);
	}
}

static void draw_fg_layer()
{
	INT32 dx   = ~*fg_scrollx & 7;
	INT32 dy   = ~*fg_scrolly & 7;
	INT32 rx   = (*fg_scrollx) >> 3;
	INT32 ry   = (*fg_scrolly) >> 3;
	INT32 bank = (*fg_select & 0x03) * 0x800;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f);
		INT32 sy = (offs / 0x20);

		INT32 ofst = ((ry + sy + 34) & 0x3f) * 0x20 + ((rx + sx) & 0x1f) + bank;
		INT32 code = DrvFgMPROM[ofst];

		sx = (sx * 8) + dx - 6;
		sy = (sy * 8) + dy + 9 - 16;

		if (DrvTransTab[2][code]) continue;

		Render8x8Tile_Mask_Clip(pTransDraw, code, sx - 8, sy, 0, 2, 0, 0, DrvGfxROM2);
	}
}

static void draw_txt_layer()
{
	for (INT32 offs = 16 * 32; offs < 240 * 32; offs++)
	{
		INT32 color;
		INT32 sx = (offs & 0x1f) * 8;
		INT32 sy = (offs / 0x20);

		if (tx_mode)
		{
			if ((DrvColPROM[sy] & 0xf8) == 0) sy -= *tx_scrolly;

			color = (DrvColPROM[sy] & 0x07) | 0x10;
		}
		else
		{
			color = DrvColPROM[(sy >> 3) + 0x100] & 0x0f;
		}

		INT32 code = DrvVidRAM[((sy >> 3) << 5) | (sx >> 3)] * 8 + (sy & 7);

		if (DrvTransTab[0][code]) continue;

		RenderCustomTile_Mask_Clip(pTransDraw, 8, 1, code, sx - 8, sy - 16, color, 2, 0, 0, DrvGfxROM0);
	}
}

static void draw_sprites(INT32 start, INT32 end)
{
	UINT8 *sprite_ram = DrvSprRAM + 0x64;

	for (INT32 offs = start; offs < end; offs += 4)
	{
		INT32 sy    = 239 - sprite_ram[offs + 0] - 16;
		INT32 code  = sprite_ram[offs + 1] | ((sprite_ram[offs + 2] & 0x60) << 3);
		      code  = ((code & 0x380) << 1) | (code & 0x7f);
		INT32 color = sprite_ram[offs + 2] & 0x07;
		INT32 flipy = sprite_ram[offs + 2] & 0x08;
		INT32 flipx = sprite_ram[offs + 2] & 0x10;
		INT32 sx    = sprite_ram[offs + 3] - 8;

		if (DrvTransTab[3][code]) continue;

#if 1		// 8x16 tiles don't seem to work well for custom tile drawing?
		{
			UINT8 *gfx = DrvGfxROM3 + (code * 0x80);

			INT32 flip = (flipx ? 0 : 7) | (flipy ? 0x78 : 0);

			color = (color << 4) + 0x80;

			for (INT32 y = 0; y < 16; y++, sy++, sx-=8) {
				for (INT32 x = 0; x < 8; x++, sx++) {
					INT32 pxl = gfx[((y * 8) + x) ^ flip];

					if (pxl) {
						if (sy >= 0 && sy < nScreenHeight && sx >= 0 && sx < nScreenWidth) {
							pTransDraw[sy * nScreenWidth + sx] = pxl + color;
						}
					}
				}
			}
		}
#else
		if (flipy) {
			if (!flipx) {
				RenderCustomTile_Mask_FlipXY_Clip(pTransDraw, 8, 16, code, sx, sy, color, 4, 0, 0x80, DrvGfxROM3);
			} else {
				RenderCustomTile_Mask_FlipY_Clip(pTransDraw, 8, 16, code, sx, sy, color, 4, 0, 0x80, DrvGfxROM3);
			}
		} else {
			if (!flipx) {
				RenderCustomTile_Mask_FlipX_Clip(pTransDraw, 8, 16, code, sx, sy, color, 4, 0, 0x80, DrvGfxROM3);
			} else {
				RenderCustomTile_Mask_Clip(pTransDraw, 8, 16, code, sx, sy, color, 4, 0, 0x80, DrvGfxROM3);
			}
		}
#endif
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x400; i+=2) {
			palette_write(i);
		}

		DrvRecalc = 0;
	}

	if ((*bg_select & 0x10) == 0)
	{
		draw_bg_layer();
	}
	else
	{
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
			pTransDraw[i] = 0x100;
		}
	}

	draw_sprites(0, 0x24);

	if ((*bg_select & 0x10) == 0)
	{
		draw_bg_layer_pri();
	}

	draw_sprites(0x24, 0x100-0x64);

	draw_txt_layer();

	if ((*fg_select & 0x10) == 0)
	{
		draw_fg_layer();
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	watchdog++;
	if (watchdog >= 180) {
		DrvDoReset(0);
	}

	if (DrvReset) {
		DrvDoReset(1);
	}

	ZetNewFrame();

	{
		memset (DrvInputs, 0xff, 3 * sizeof(UINT8));

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	INT32 nCycleSegment;
	INT32 nInterleave = 10;
	INT32 nCyclesTotal[2] = { 5000000 / 60, 2500000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCycleSegment = nCyclesTotal[0] / nInterleave;

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nCycleSegment);
		if (i == (nInterleave-1)) ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		ZetClose();

		nCycleSegment = nCyclesTotal[1] / nInterleave;

		ZetOpen(1);
		BurnTimerUpdate(i * nCycleSegment);
		nCyclesDone[1] += nCycleSegment;
		ZetClose();
	}

	ZetOpen(1);
	BurnTimerEndFrame(nCyclesTotal[1]);
	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
	}
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
		*pnMin = 0x029702;
	}

	if (nAction & ACB_VOLATILE) {
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All RAM";
		BurnAcb(&ba);

		ZetScan(nAction);

		BurnYM2203Scan(nAction, pnMin);
	}

	if (nAction & ACB_WRITE)
	{
		ZetOpen(0);
		bankswitch(*bg_bank);
		ZetClose();
	}

	return 0;
}


// Momoko 120%

static struct BurnRomInfo momokoRomDesc[] = {
	{ "momoko03.bin",	0x8000, 0x386e26ed, 0x01 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "momoko02.bin",	0x4000, 0x4255e351, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "momoko01.bin",	0x8000, 0xe8a6673c, 0x02 | BRF_PRG | BRF_ESS }, //  2 Z80 #1 Code

	{ "momoko13.bin",	0x2000, 0x2745cf5a, 0x03 | BRF_GRA },           //  3 Character Tiles

	{ "momoko14.bin",	0x2000, 0xcfccca05, 0x04 | BRF_GRA },           //  4 Foreground Tiles

	{ "momoko16.bin",	0x8000, 0xfc6876fc, 0x05 | BRF_GRA },           //  5 Sprite Tiles
	{ "momoko17.bin",	0x8000, 0x45dc0247, 0x05 | BRF_GRA },           //  6

	{ "momoko09.bin",	0x8000, 0x9f5847c7, 0x06 | BRF_GRA },           //  7 Background Tiles
	{ "momoko11.bin",	0x8000, 0x9c9fbd43, 0x06 | BRF_GRA },           //  8
	{ "momoko10.bin",	0x8000, 0xae17e74b, 0x06 | BRF_GRA },           //  9
	{ "momoko12.bin",	0x8000, 0x1e29c9c4, 0x06 | BRF_GRA },           // 10

	{ "momoko04.bin",	0x8000, 0x3ab3c2c3, 0x07 | BRF_GRA },           // 11 Background Map (Banks used by Z80 #0)
	{ "momoko05.bin",	0x8000, 0x757cdd2b, 0x07 | BRF_GRA },           // 12
	{ "momoko06.bin",	0x8000, 0x20cacf8b, 0x07 | BRF_GRA },           // 13
	{ "momoko07.bin",	0x8000, 0xb94b38db, 0x07 | BRF_GRA },           // 14

	{ "momoko08.bin",	0x2000, 0x69b41702, 0x08 | BRF_GRA },           // 15 Background Color/Priority Table

	{ "momoko15.bin",	0x4000, 0x8028f806, 0x09 | BRF_GRA },           // 16 Foreground Map

	{ "momoko-c.bin",	0x0100, 0xf35ccae0, 0x0a | BRF_GRA },           // 17 Text Layer Color PROMs
	{ "momoko-b.bin",	0x0020, 0x427b0e5c, 0x0a | BRF_GRA },           // 18
};

STD_ROM_PICK(momoko)
STD_ROM_FN(momoko)

struct BurnDriver BurnDrvMomoko = {
	"momoko", NULL, NULL, NULL, "1986",
	"Momoko 120%\0", NULL, "Jaleco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, momokoRomInfo, momokoRomName, NULL, NULL, MomokoInputInfo, MomokoDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	240, 224, 4, 3
};
