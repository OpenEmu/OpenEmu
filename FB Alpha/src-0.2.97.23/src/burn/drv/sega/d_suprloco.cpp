// FB Alpha Super Locomotive driver module
// Based on MAME driver by Zsolt Vasvari

#include "tiles_generic.h"
#include "z80_intf.h"
#include "sn76496.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80Dec0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvColPROM;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvVidRAM;
static UINT8 *DrvScrRAM;
static UINT8 *DrvSprRAM;

static UINT32  *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *flipscreen;
static UINT8 *control;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[8];
static UINT8 DrvReset;
static UINT8 DrvInputs[3];

static struct BurnInputInfo SuprlocoInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 3,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Suprloco)

static struct BurnDIPInfo SuprlocoDIPList[]=
{
	{0x12, 0xff, 0xff, 0x40, NULL				},
	{0x13, 0xff, 0xff, 0x7c, NULL				},

	{0   , 0xfe, 0   ,    8, "Coin A"			},
	{0x12, 0x01, 0x07, 0x07, "5 Coins 1 Credits "		},
	{0x12, 0x01, 0x07, 0x06, "4 Coins 1 Credits "		},
	{0x12, 0x01, 0x07, 0x05, "3 Coins 1 Credits "		},
	{0x12, 0x01, 0x07, 0x04, "2 Coins 1 Credits "		},
	{0x12, 0x01, 0x07, 0x00, "1 Coin 1 Credits "		},
	{0x12, 0x01, 0x07, 0x01, "1 Coin 2 Credits "		},
	{0x12, 0x01, 0x07, 0x02, "1 Coin 3 Credits "		},
	{0x12, 0x01, 0x07, 0x03, "1 Coin 6 Credits "		},

	{0   , 0xfe, 0   ,    8, "Coin B"			},
	{0x12, 0x01, 0x38, 0x38, "5 Coins 1 Credits "		},
	{0x12, 0x01, 0x38, 0x30, "4 Coins 1 Credits "		},
	{0x12, 0x01, 0x38, 0x28, "3 Coins 1 Credits "		},
	{0x12, 0x01, 0x38, 0x20, "2 Coins 1 Credits "		},
	{0x12, 0x01, 0x38, 0x00, "1 Coin 1 Credits "		},
	{0x12, 0x01, 0x38, 0x08, "1 Coin 2 Credits "		},
	{0x12, 0x01, 0x38, 0x10, "1 Coin 3 Credits "		},
	{0x12, 0x01, 0x38, 0x18, "1 Coin 6 Credits "		},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x12, 0x01, 0xc0, 0x00, "2"				},
	{0x12, 0x01, 0xc0, 0x40, "3"				},
	{0x12, 0x01, 0xc0, 0x80, "4"				},
	{0x12, 0x01, 0xc0, 0xc0, "5"				},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x13, 0x01, 0x03, 0x00, "20000"			},
	{0x13, 0x01, 0x03, 0x01, "30000"			},
	{0x13, 0x01, 0x03, 0x02, "40000"			},
	{0x13, 0x01, 0x03, 0x03, "50000"			},

	{0   , 0xfe, 0   ,    2, "Free Play"			},
	{0x13, 0x01, 0x08, 0x08, "Off"				},
	{0x13, 0x01, 0x08, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Difficulty"			},
	{0x13, 0x01, 0x10, 0x10, "Easy"				},
	{0x13, 0x01, 0x10, 0x00, "Hard"				},

	{0   , 0xfe, 0   ,    2, "Infinite Lives (Cheat)"	},
	{0x13, 0x01, 0x20, 0x20, "Off"				},
	{0x13, 0x01, 0x20, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Initial Entry"		},
	{0x13, 0x01, 0x40, 0x00, "Off"				},
	{0x13, 0x01, 0x40, 0x40, "On"				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x13, 0x01, 0x80, 0x00, "Upright"			},
	{0x13, 0x01, 0x80, 0x80, "Cocktail"			},
};

STDDIPINFO(Suprloco)

void __fastcall suprloco_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe800:
			*soundlatch = data;
			ZetClose();
			ZetOpen(1);
			ZetNmi();
			ZetRun(50);
			ZetClose();
			ZetOpen(0);
		return;

		case 0xe801:
			*flipscreen = data & 0x80;
			*control = data;
		return;
	}
}

UINT8 __fastcall suprloco_main_read(UINT16 address)
{
	switch (address)
	{
		case 0xc800:
			return DrvInputs[0];

		case 0xd000:
			return DrvInputs[1];

		case 0xd800:
			return DrvInputs[2];

		case 0xe000:
			return DrvDips[0];

		case 0xe001:
			return DrvDips[1];

		case 0xe801:
			return *control;
	}

	return 0;
}

void __fastcall suprloco_sound_write(UINT16 address, UINT8 data)
{
	switch (address & ~0x0003)
	{
		case 0xa000:
			SN76496Write(0, data);
		return;

		case 0xc000:
			SN76496Write(1, data);
		return;
	}
}

UINT8 __fastcall suprloco_sound_read(UINT16 address)
{
	if (address == 0xe000) return *soundlatch;

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0		= Next; Next += 0x010000;
	DrvZ80Dec0		= Next; Next += 0x010000;
	DrvZ80ROM1		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x010000;
	DrvGfxROM1		= Next; Next += 0x010000;

	DrvColPROM		= Next; Next += 0x000600;

	DrvPalette		= (UINT32*)Next; Next += 0x0300 * sizeof(INT32);

	AllRam			= Next;

	DrvZ80RAM0		= Next; Next += 0x000800;
	DrvZ80RAM1		= Next; Next += 0x000800;

	DrvVidRAM		= Next; Next += 0x000700;
	DrvScrRAM		= Next; Next += 0x000100;
	DrvSprRAM		= Next; Next += 0x000200;

	soundlatch		= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;
	control			= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0;i < 512; i++)
	{
		INT32 bit0,bit1,bit2,r,g,b;

		bit0 = (DrvColPROM[i] >> 0) & 0x01;
		bit1 = (DrvColPROM[i] >> 1) & 0x01;
		bit2 = (DrvColPROM[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (DrvColPROM[i] >> 3) & 0x01;
		bit1 = (DrvColPROM[i] >> 4) & 0x01;
		bit2 = (DrvColPROM[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = (DrvColPROM[i] >> 6) & 0x01;
		bit2 = (DrvColPROM[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);

		if (i >= 256)
		{
			if ((i & 0x0f) == 0x09) b = 0xff;

			DrvPalette[i + 0x100] = BurnHighCol(r, g, b, 0);
		}
	}
}

static void DrvGfxDecode()
{
	UINT8 *src	= DrvGfxROM0;
	UINT8 *dst	= DrvGfxROM0 + 0x6000;
	UINT8 *lookup	= DrvColPROM + 0x0200;

	for (INT32 i = 0; i < 0x80; i++, lookup += 8)
	{
		for (INT32 j = 0; j < 0x40; j++, src++, dst++)
		{
			dst[0] = dst[0x2000] = dst[0x4000] = dst[0x6000] = 0;

			for (INT32 k = 0; k < 8; k++)
			{
				INT32 source = (((src[0] >> k) & 0x01) << 2) | (((src[0x2000] >> k) & 0x01) << 1) | ((src[0x4000] >> k) & 0x01);

				INT32 dest = lookup[source];

				dst[0x0000] |= (((dest >> 3) & 0x01) << k);
				dst[0x2000] |= (((dest >> 2) & 0x01) << k);
				dst[0x4000] |= (((dest >> 1) & 0x01) << k);
				dst[0x6000] |= (((dest >> 0) & 0x01) << k);
			}
		}
	}

	INT32 Plane[4] = { 0, 1024*8*8, 2*1024*8*8, 3*1024*8*8 };
	INT32 XOffs[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 YOffs[8] = { 0, 8, 16, 24, 32, 40, 48, 56 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x8000);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, DrvGfxROM0 + 0x6000, 0x8000);

	GfxDecode(0x0400, 4, 8, 8, Plane, XOffs, YOffs, 0x040, tmp, DrvGfxROM0);

	BurnFree (tmp);
}

static void suprloco_decode(UINT8 *rom, UINT8 *decrypted, INT32 length)
{
	static const UINT8 convtable[32][4] =
	{
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },	
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 },
		{ 0x28,0xa8,0x08,0x88 }, { 0x88,0x80,0x08,0x00 },
		{ 0x20,0x00,0xa0,0x80 }, { 0xa8,0xa0,0x88,0x80 },
		{ 0x88,0x08,0x80,0x00 }, { 0xa0,0x80,0xa8,0x88 }
	};

	memcpy (decrypted + 0x8000, rom + 0x8000, length - 0x8000);

	for (INT32 A = 0; A < 0x8000; A++)
	{
		INT32 xorval = 0;

		UINT8 src = rom[A];

		INT32 row = (A & 1) + (((A >> 4) & 1) << 1) + (((A >> 8) & 1) << 2) + (((A >> 12) & 1) << 3);
		INT32 col = ((src >> 3) & 1) + (((src >> 5) & 1) << 1);

		if (src & 0x80)
		{
			col = 3 - col;
			xorval = 0xa8;
		}

		decrypted[A] = (src & ~0xa8) | (convtable[2*row+0][col] ^ xorval);
		rom[A]       = (src & ~0xa8) | (convtable[2*row+1][col] ^ xorval);

		if (convtable[2*row+0][col] == 0xff) decrypted[A] = 0xee;
		if (convtable[2*row+1][col] == 0xff) rom[A] = 0xee;
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
		if (BurnLoadRom(DrvZ80ROM0 + 0x04000,  1, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x08000,  2, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM1 + 0x00000,  3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,  4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x02000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x04000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x04000,  8, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x00200,  9, 1)) return 1;
		memcpy (DrvColPROM + 0x0100, DrvColPROM + 0x0200, 0x0080);
		memcpy (DrvColPROM + 0x0000, DrvColPROM + 0x0280, 0x0080);
		memcpy (DrvColPROM + 0x0180, DrvColPROM + 0x0300, 0x0080);
		memcpy (DrvColPROM + 0x0080, DrvColPROM + 0x0380, 0x0080);

		if (BurnLoadRom(DrvColPROM + 0x00200, 10, 1)) return 1;

		suprloco_decode(DrvZ80ROM0, DrvZ80Dec0, 0xc000);

		DrvPaletteInit();
		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80Dec0, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xc1ff, 0, DrvSprRAM);
	ZetMapArea(0xc000, 0xc1ff, 1, DrvSprRAM);
	ZetMapArea(0xc000, 0xc1ff, 2, DrvSprRAM);
	ZetMapArea(0xf000, 0xf6ff, 0, DrvVidRAM);
	ZetMapArea(0xf000, 0xf6ff, 1, DrvVidRAM);
	ZetMapArea(0xf000, 0xf6ff, 2, DrvVidRAM);
	ZetMapArea(0xf700, 0xf7ff, 0, DrvScrRAM);
	ZetMapArea(0xf700, 0xf7ff, 1, DrvScrRAM);
	ZetMapArea(0xf700, 0xf7ff, 2, DrvScrRAM);
	ZetMapArea(0xf800, 0xffff, 0, DrvZ80RAM0);
	ZetMapArea(0xf800, 0xffff, 1, DrvZ80RAM0);
	ZetMapArea(0xf800, 0xffff, 2, DrvZ80RAM0);
	ZetSetWriteHandler(suprloco_main_write);
	ZetSetReadHandler(suprloco_main_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM1);
	ZetSetWriteHandler(suprloco_sound_write);
	ZetSetReadHandler(suprloco_sound_read);
	ZetMemEnd();
	ZetClose();

	SN76496Init(0, 4000000, 0);
	SN76496Init(1, 2000000, 1);
	SN76496SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	SN76496SetRoute(1, 1.00, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();
	ZetExit();

	SN76496Exit();

	BurnFree (AllMem);

	return 0;
}

static void draw_layer(INT32 priority)
{
	UINT8 *scrollx = DrvScrRAM + 0x00e0;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		sx -= scrollx[offs >> 5];
		if (sx < -7) sx += 256;

		INT32 attr = DrvVidRAM[offs * 2 + 1];
		INT32 code = DrvVidRAM[offs * 2 + 0] | ((attr & 3) << 8);
		INT32 color= (attr & 0x1c) >> 2;
		INT32 pri  =  attr & 0x20;

		if (pri != priority) continue;

		if (*flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, 238 - sx, 216 - sy, color, 4, 0, DrvGfxROM0);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
		}
	}
}

static inline void draw_pixel(INT32 x,INT32 y,INT32 color)
{
	if (*flipscreen) {
		x = nScreenWidth - x - 1;
		y = nScreenHeight - y - 1;
	}

	if (x < 0 || x >= nScreenWidth || y < 0 || y >= nScreenHeight) return;

	pTransDraw[(y * nScreenWidth) + x] = color;
}

static void draw_sprites()
{
	INT32 color_offset = (*control & 0x20) ? 0x200 : 0x100;

	for (INT32 offs = 0; offs < 512; offs += 0x10)
	{
		INT32 adjy,dy;

		INT32 sy	= DrvSprRAM[offs + 0] + 1;
		INT32 height	= DrvSprRAM[offs + 1] - (sy - 1);
		INT32 sx	= DrvSprRAM[offs + 2] - 8;
		INT32 pen_base	=(DrvSprRAM[offs + 3] & 0x03) * 0x10 + color_offset;
		INT16 skip	= DrvSprRAM[offs + 4] + (DrvSprRAM[offs + 5] << 8);
		INT32 src	= DrvSprRAM[offs + 6] + (DrvSprRAM[offs + 7] << 8);

		if (src == 0xff) continue;

		if (!*flipscreen)
		{
			adjy = sy;
			dy = 1;
		}
		else
		{
			adjy = sy + height - 1;
			dy = -1;
		}
	
		for (INT32 row = 0; row < height; row++, adjy+=dy)
		{
			INT32 color1,color2,data;
	
			src += skip;
	
			INT32 col = 0;
	
			UINT8 *gfx = DrvGfxROM1 + (src & 0x7fff);
	
			INT32 flipx = src & 0x8000;
	
			while (1)
			{
				if (flipx)
				{
					data = *gfx--;
					color1 = data & 0x0f;
					color2 = data >> 4;
				}
				else
				{
					data = *gfx++;
					color1 = data >> 4;
					color2 = data & 0x0f;
				}
	
				if (color1 == 15) break;
				if (color1) draw_pixel(sx+col+0, adjy, pen_base + color1);
	
				if (color2 == 15) break;
				if (color2) draw_pixel(sx+col+1, adjy, pen_base + color2);
	
				col += 2;
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

	draw_layer(0x00);
	draw_sprites();
	draw_layer(0x20);

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
	}

	INT32 nInterleave = 100;
	INT32 nCyclesTotal[2] = { 4000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nCyclesSegment = nCyclesTotal[0] / nInterleave;

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nCyclesSegment);
		if (i == 99) ZetRaiseIrq(0);
		ZetClose();
	
		ZetOpen(1);
		nCyclesDone[1] += ZetRun(nCyclesSegment);
		if (i == 24 || i == 49 || i == 74 || i == 99) ZetRaiseIrq(0);
		ZetClose();
	}

	if (pBurnSoundOut) {
		SN76496Update(0, pBurnSoundOut, nBurnSoundLen);
		SN76496Update(1, pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029714;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);

		SN76496Scan(nAction, pnMin);
	}

	return 0;
}


// Super Locomotive

static struct BurnRomInfo suprlocoRomDesc[] = {
	{ "ic37.bin",		0x4000, 0x57f514dd, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "ic15.bin",		0x4000, 0x5a1d2fb0, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr-5228.28",	0x4000, 0xa597828a, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "epr-5222.64",	0x2000, 0x0aa57207, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "epr-5225.63",	0x2000, 0xe571fe81, 3 | BRF_GRA },           //  4 Background data
	{ "epr-5224.62",	0x2000, 0x6130f93c, 3 | BRF_GRA },           //  5
	{ "epr-5223.61",	0x2000, 0x3b03004e, 3 | BRF_GRA },           //  6

	{ "epr-5229.55",	0x4000, 0xee2d3ed3, 4 | BRF_GRA },           //  7 Sprite data
	{ "epr-5230.56",	0x2000, 0xf04a4b50, 4 | BRF_GRA },           //  8

	{ "pr-5220.100",	0x0200, 0x7b0c8ce5, 5 | BRF_GRA },           //  9 Color Proms
	{ "pr-5219.89",		0x0400, 0x1d4b02cb, 5 | BRF_GRA },           // 10

	{ "pr-5221.7",		0x0020, 0x89ba674f, 0 | BRF_OPT },           // 11 Unknown
};

STD_ROM_PICK(suprloco)
STD_ROM_FN(suprloco)

struct BurnDriver BurnDrvSuprloco = {
	"suprloco", NULL, NULL, NULL, "1982",
	"Super Locomotive\0", NULL, "Sega", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SEGA_MISC, GBF_MISC, 0,
	NULL, suprlocoRomInfo, suprlocoRomName, NULL, NULL, SuprlocoInputInfo, SuprlocoDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x300,
	240, 224, 4, 3
};


// Super Locomotive (Rev.A)

static struct BurnRomInfo suprlocoaRomDesc[] = {
	{ "epr-5226a.37",	0x4000, 0x33b02368, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "epr-5227a.15",	0x4000, 0xa5e67f50, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr-5228.28",	0x4000, 0xa597828a, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "epr-5222.64",	0x2000, 0x0aa57207, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "epr-5225.63",	0x2000, 0xe571fe81, 3 | BRF_GRA },           //  4 Background data
	{ "epr-5224.62",	0x2000, 0x6130f93c, 3 | BRF_GRA },           //  5
	{ "epr-5223.61",	0x2000, 0x3b03004e, 3 | BRF_GRA },           //  6

	{ "epr-5229.55",	0x4000, 0xee2d3ed3, 4 | BRF_GRA },           //  7 Sprite data
	{ "epr-5230.56",	0x2000, 0xf04a4b50, 4 | BRF_GRA },           //  8

	{ "pr-5220.100",	0x0200, 0x7b0c8ce5, 5 | BRF_GRA },           //  9 Color Proms
	{ "pr-5219.89",		0x0400, 0x1d4b02cb, 5 | BRF_GRA },           // 10

	{ "pr-5221.7",		0x0020, 0x89ba674f, 0 | BRF_OPT },           // 11 Unknown
};

STD_ROM_PICK(suprlocoa)
STD_ROM_FN(suprlocoa)

struct BurnDriver BurnDrvSuprlocoa = {
	"suprlocoa", "suprloco", NULL, NULL, "1982",
	"Super Locomotive (Rev.A)\0", NULL, "Sega", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SEGA_MISC, GBF_MISC, 0,
	NULL, suprlocoaRomInfo, suprlocoaRomName, NULL, NULL, SuprlocoInputInfo, SuprlocoDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x300,
	240, 224, 4, 3
};
