// FB Alpha Vulgus drive module
// Based on MAME driver by Mirko Buffoni

// To do: flip screen

#include "tiles_generic.h"
#include "zet.h"

#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *Mem, *MemEnd, *Rom0, *Rom1, *Gfx0, *Gfx1, *Gfx2, *Prom;
static UINT8 DrvJoy1[8], DrvJoy2[8], DrvJoy3[8], DrvDips[2], DrvReset;
static INT16 *pAY8910Buffer[6], *pFMBuffer = NULL;
static UINT32 *DrvPalette, *Palette;
static UINT8 DrvRecalc;

static INT32 vulgus_soundlatch;
static INT32 vulgus_scroll[2];
static INT32 vulgus_palette_bank;
static INT32 vulgus_flipscreen;

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy1 + 7,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy1 + 6,	"p2 coin"  },
	{"P1 Start"     , BIT_DIGITAL  , DrvJoy1 + 0,	"p1 start" },
	{"P2 Start"     , BIT_DIGITAL  , DrvJoy1 + 1,	"p2 start" },

	{"P1 Right"     , BIT_DIGITAL  , DrvJoy2 + 0, 	"p1 right" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy2 + 1, 	"p1 left"  },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy2 + 2, 	"p1 down"  },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy2 + 3, 	"p1 up"    },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 5,	"p1 fire 2"},

	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 0, 	"p2 right" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 1, 	"p2 left"  },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy3 + 2, 	"p2 down"  },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy3 + 3, 	"p2 up"    },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 4,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 5,	"p2 fire 2"},

	
	{"Reset"        , BIT_DIGITAL  , &DrvReset  ,	"reset"    },
	{"Dip 1"        , BIT_DIPSWITCH, DrvDips + 0,   "dip 1"    },
	{"Dip 2"        , BIT_DIPSWITCH, DrvDips + 1,   "dip 2"    },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },
	{0x12, 0xff, 0xff, 0x7f, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x11, 0x01, 0x03, 0x01, "1"                      },
	{0x11, 0x01, 0x03, 0x02, "2"                      },
	{0x11, 0x01, 0x03, 0x03, "3"                      },
	{0x11, 0x01, 0x03, 0x00, "5"                      },

	{0   , 0xfe, 0   , 7   , "Coin B"                 },
	{0x11, 0x01, 0x1c, 0x10, "5 Coins 1 Play"         },
	{0x11, 0x01, 0x1c, 0x08, "4 Coins 1 Play"         },
	{0x11, 0x01, 0x1c, 0x18, "3 Coins 1 Play"         },
	{0x11, 0x01, 0x1c, 0x04, "2 Coins 1 Play"         },
	{0x11, 0x01, 0x1c, 0x1c, "1 Coin  1 Play"         },
	{0x11, 0x01, 0x1c, 0x0c, "1 Coin  2 Plays"        },
	{0x11, 0x01, 0x1c, 0x14, "1 Coin  3 Plays"        },
//	{0x11, 0x01, 0x1c, 0x00, "Invalid"                },

	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x11, 0x01, 0xe0, 0x80, "5 Coins 1 Play"         },
	{0x11, 0x01, 0xe0, 0x40, "4 Coins 1 Play"         },
	{0x11, 0x01, 0xe0, 0xc0, "3 Coins 1 Play"         },
	{0x11, 0x01, 0xe0, 0x20, "2 Coins 1 Play"         },
	{0x11, 0x01, 0xe0, 0xe0, "1 Coin  1 Play"         },
	{0x11, 0x01, 0xe0, 0x60, "1 Coin  2 Plays"        },
	{0x11, 0x01, 0xe0, 0xa0, "1 Coin  3 Plays"        },
	{0x11, 0x01, 0xe0, 0x00, "Freeplay"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty?"            },
	{0x12, 0x01, 0x03, 0x02, "Easy?"                  },
	{0x12, 0x01, 0x03, 0x03, "Normal?"                },
	{0x12, 0x01, 0x03, 0x01, "Hard?"                  },
	{0x12, 0x01, 0x03, 0x00, "Hardest?"               },
	
	{0   , 0xfe, 0   , 2   , "Demo Music"             },
	{0x12, 0x01, 0x04, 0x00, "Off"                    },
	{0x12, 0x01, 0x04, 0x04, "On"                     },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x08, 0x00, "Off"                    },
	{0x12, 0x01, 0x08, 0x08, "On"                     },

	{0   , 0xfe, 0   , 8   , "Bonus Life"             },
	{0x12, 0x01, 0x70, 0x30, "10000 50000"            },
	{0x12, 0x01, 0x70, 0x50, "10000 60000"            },
	{0x12, 0x01, 0x70, 0x10, "10000 70000"            },
	{0x12, 0x01, 0x70, 0x70, "20000 60000"            },
	{0x12, 0x01, 0x70, 0x60, "20000 70000"            },
	{0x12, 0x01, 0x70, 0x20, "20000 80000"            },
	{0x12, 0x01, 0x70, 0x40, "30000 70000"            },
	{0x12, 0x01, 0x70, 0x00, "None"	                  },	

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x80, 0x00, "Upright"                },
	{0x12, 0x01, 0x80, 0x80, "Cocktail"               },
};

STDDIPINFO(Drv)

void __fastcall vulgus_write_main(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xc800:
			vulgus_soundlatch = data;
		break;

		case 0xc802:
		case 0xc803:
			vulgus_scroll[address & 1] = (vulgus_scroll[address & 1] & 0xff00) | data;
		break;

		case 0xc804:
			vulgus_flipscreen = data >> 7;
		break;

		case 0xc805:
			vulgus_palette_bank = data & 3;
		break;

		case 0xc902:
		case 0xc903:
			vulgus_scroll[address & 1] = (vulgus_scroll[address & 1] & 0x00ff) | (data << 8);
		break;
	}
}

UINT8 __fastcall vulgus_read_main(UINT16 address)
{
	UINT8 ret;

	switch (address)
	{
		case 0xc000:
		{
			ret = 0xff;

			for (INT32 i = 0; i < 8; i++) ret ^= DrvJoy1[i] << i;

			return ret;
		}

		case 0xc001:
		{
			ret = 0xff;

			for (INT32 i = 0; i < 5; i++) ret ^= DrvJoy2[i] << i;

			return ret;
		}

		case 0xc002:
		{
			ret = 0xff;

			for (INT32 i = 0; i < 5; i++) ret ^= DrvJoy3[i] << i;

			return ret;
		}

		case 0xc003:
			return DrvDips[0];

		case 0xc004:
			return DrvDips[1];
	}

	return 0;
}

void __fastcall vulgus_write_sound(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x8000:
		case 0x8001:
		case 0xc000:
		case 0xc001:
			AY8910Write((address >> 14) & 1, address & 1, data);
			break;
	}
}

UINT8 __fastcall vulgus_read_sound(UINT16 address)
{
	switch (address)
	{
		case 0x6000:
			return vulgus_soundlatch;
	}

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	vulgus_flipscreen = 0;
	vulgus_soundlatch = 0;
	vulgus_palette_bank = 0;

	vulgus_scroll[0] = 0;
	vulgus_scroll[1] = 0;

	memset (Rom0 + 0xcc00, 0, 0x2400);
	memset (Rom1 + 0x4000, 0, 0x0800);

	for (INT32 i = 0; i < 2; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();

		AY8910Reset(i);
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Rom0		= Next; Next += 0x10000;
	Rom1		= Next; Next += 0x05000;
	Gfx0		= Next; Next += 0x08000;
	Gfx1		= Next; Next += 0x20000;
	Gfx2		= Next; Next += 0x10000;
	Prom		= Next; Next += 0x00600;

	Palette		= (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);
	DrvPalette	= (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);

	pFMBuffer	= (INT16*)Next; Next += (nBurnSoundLen * 6 * sizeof(INT16));

	MemEnd		= Next;

	return 0;
}

static INT32 DrvPaletteInit()
{
	UINT32 *tmp = (UINT32*)BurnMalloc(0x100 * sizeof(UINT32));
	if (tmp == NULL) { 
		return 1;
	}

	for (INT32 i = 0; i < 256; i++)
	{
		INT32 bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (Prom[  0 + i] >> 0) & 0x01;
		bit1 = (Prom[  0 + i] >> 1) & 0x01;
		bit2 = (Prom[  0 + i] >> 2) & 0x01;
		bit3 = (Prom[  0 + i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (Prom[256 + i] >> 0) & 0x01;
		bit1 = (Prom[256 + i] >> 1) & 0x01;
		bit2 = (Prom[256 + i] >> 2) & 0x01;
		bit3 = (Prom[256 + i] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (Prom[512 + i] >> 0) & 0x01;
		bit1 = (Prom[512 + i] >> 1) & 0x01;
		bit2 = (Prom[512 + i] >> 2) & 0x01;
		bit3 = (Prom[512 + i] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		tmp[i] = (r << 16) | (g << 8) | b;
	}

	for (INT32 i = 0; i < 0x100; i++) {
		Palette[i] = tmp[32 + Prom[0x300 + i]];
	}

	for (INT32 i = 0; i < 0x100; i++) {
		Palette[0x100 + i] = tmp[16 + Prom[0x400 + i]];
	}

	for (INT32 i = 0; i < 0x100; i++)	{
		Palette[0x400 + i] = tmp[Prom[0x500 + i] + 0x00];
		Palette[0x500 + i] = tmp[Prom[0x500 + i] + 0x40];
		Palette[0x600 + i] = tmp[Prom[0x500 + i] + 0x80];
		Palette[0x700 + i] = tmp[Prom[0x500 + i] + 0xc0];
	}

	BurnFree (tmp);

	return 0;
}

static INT32 DrvGfxDecode()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0xc000);
	if (tmp == NULL) {
		return 1;
	}

	static INT32 SpriPlanes[4] = { 0x20004, 0x20000, 0x00004, 0x00000 };
	static INT32 SpriXOffs[16] = { 0x000, 0x001, 0x002, 0x003, 0x008, 0x009, 0x00a, 0x00b,
				     0x100, 0x101, 0x102, 0x103, 0x108, 0x109, 0x10a, 0x10b };
	static INT32 SpriYOffs[16] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
				     0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0 };

	static INT32 TilePlanes[3] = { 0x00000, 0x20000, 0x40000 };
	static INT32 TileXOffs[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
				     0x080, 0x081, 0x082, 0x083, 0x084, 0x085, 0x086, 0x087 };
	static INT32 TileYOffs[16] = { 0x000, 0x008, 0x010, 0x018, 0x020, 0x028, 0x030, 0x038,
				     0x040, 0x048, 0x050, 0x058, 0x060, 0x068, 0x070, 0x078 };

	memcpy (tmp, Gfx0, 0x2000);

	GfxDecode(0x200, 2,  8,  8, SpriPlanes + 2, SpriXOffs, SpriYOffs, 0x080, tmp, Gfx0);

	memcpy (tmp, Gfx1, 0xc000);

	GfxDecode(0x200, 3, 16, 16, TilePlanes + 0, TileXOffs, TileYOffs, 0x100, tmp, Gfx1);

	memcpy (tmp, Gfx2, 0x8000);

	GfxDecode(0x100, 4, 16, 16, SpriPlanes + 0, SpriXOffs, SpriYOffs, 0x200, tmp, Gfx2);

	BurnFree (tmp);

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	for (INT32 i = 0; i < 6; i++) {
		pAY8910Buffer[i] = pFMBuffer + nBurnSoundLen * i;
	}

	{
		for (INT32 i = 0; i < 5; i++) {
			if (BurnLoadRom(Rom0 + i * 0x2000, i +  0, 1)) return 1;
		}

		if (BurnLoadRom(Rom1 + 0x0000, 5, 1)) return 1;
		if (BurnLoadRom(Gfx0 + 0x0000, 6, 1)) return 1;

		for (INT32 i = 0; i < 6; i++) {
			if (BurnLoadRom(Gfx1 + i * 0x2000, i +  7, 1)) return 1;
		}

		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(Gfx2 + i * 0x2000, i + 13, 1)) return 1;
		}

		for (INT32 i = 0; i < 6; i++) {
			if (BurnLoadRom(Prom + i * 0x0100, i + 17, 1)) return 1;
		}

		if (DrvPaletteInit()) return 1;
		if (DrvGfxDecode()) return 1;
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x9fff, 0, Rom0 + 0x0000);
	ZetMapArea(0x0000, 0x9fff, 2, Rom0 + 0x0000);
	ZetMapArea(0xcc00, 0xccff, 0, Rom0 + 0xcc00);
	ZetMapArea(0xcc00, 0xccff, 1, Rom0 + 0xcc00);
	ZetMapArea(0xd000, 0xdfff, 0, Rom0 + 0xd000);
	ZetMapArea(0xd000, 0xdfff, 1, Rom0 + 0xd000);
	ZetMapArea(0xe000, 0xefff, 0, Rom0 + 0xe000);
	ZetMapArea(0xe000, 0xefff, 1, Rom0 + 0xe000);
	ZetMapArea(0xe000, 0xefff, 2, Rom0 + 0xe000);
	ZetSetWriteHandler(vulgus_write_main);
	ZetSetReadHandler(vulgus_read_main);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x1fff, 0, Rom1 + 0x0000);
	ZetMapArea(0x0000, 0x1fff, 2, Rom1 + 0x0000);
	ZetMapArea(0x4000, 0x47ff, 0, Rom1 + 0x4000);
	ZetMapArea(0x4000, 0x47ff, 1, Rom1 + 0x4000);
	ZetMapArea(0x4000, 0x47ff, 2, Rom1 + 0x4000);
	ZetSetWriteHandler(vulgus_write_sound);
	ZetSetReadHandler(vulgus_read_sound);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	ZetExit();
	AY8910Exit(0);
	AY8910Exit(1);
	GenericTilesExit();

	BurnFree (Mem);

	Mem = MemEnd = Rom0 = Rom1 = NULL;
	Gfx0 = Gfx1 = Gfx2 = Prom = NULL;
	pFMBuffer = NULL;

	for (INT32 i = 0; i < 6; i++) {
		pAY8910Buffer[i] = NULL;
	}

	DrvPalette = Palette = NULL;

	DrvRecalc = 0;

	vulgus_soundlatch = 0;
	vulgus_scroll[0] = vulgus_scroll[1] = 0;
	vulgus_palette_bank = 0;
	vulgus_flipscreen = 0;

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x800; i++) {
			INT32 color = Palette[i];
			DrvPalette[i] = BurnHighCol(color >> 16, color >> 8, color, 0);
		}
	}

	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 sx, sy, color, code, flipx, flipy;

		sx = (offs >> 1) & 0x1f0;
		sy = (offs << 4) & 0x1f0;

		sx -= vulgus_scroll[1];
		sy -= vulgus_scroll[0];

		if (sx < -15) sx += 0x200; // wrap
		if (sy < -15) sy += 0x200;

		color = Rom0[0xdc00 + offs];
		code  = Rom0[0xd800 + offs] | ((color & 0x80) << 1);

		flipx = color & 0x20;
		flipy = color & 0x40;

		color = (color & 0x1f) | (vulgus_palette_bank << 5);

		sy -= 0x10;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 3, 0x400, Gfx1);
			} else {
				Render16x16Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 3, 0x400, Gfx1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 3, 0x400, Gfx1);
			} else {
				Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 3, 0x400, Gfx1);
			}
		}
	}

	for (INT32 offs = 0x7c; offs >= 0; offs -= 4)
	{
		INT32 code, i, color, sx, sy;

		code  = Rom0[0xcc00 + offs];
		color = Rom0[0xcc01 + offs] & 0x0f;
		sx    = Rom0[0xcc03 + offs];
		sy    = Rom0[0xcc02 + offs];

		sy -= 0x10;

		i = Rom0[0xcc01 + offs] >> 6;
		if (i == 2) i = 3;

		for (; i >= 0; i--) {
			INT32 ssy = sy + (i << 4);
			Render16x16Tile_Mask_Clip(pTransDraw, code + i, sx, ssy, color, 4, 0x0f, 0x100, Gfx2);
			if (ssy > 240) { // wrap
				Render16x16Tile_Mask_Clip(pTransDraw, code + i, sx, ssy - 256, color, 4, 0x0f, 0x100, Gfx2);
			}
		}
	}

	for (INT32 offs = 0x40; offs < 0x3c0; offs++)
	{
		INT32 sx, sy, color, code;

		color = Rom0[0xd400 + offs];
		code  = Rom0[0xd000 + offs] | ((color & 0x80) << 1);

		if (code == 0x20) continue;

		UINT8 *src = Gfx0 + (code << 6);

		color = (color & 0x3f) << 2;

		sx = (offs << 3) & 0xf8;
		sy = (offs >> 2) & 0xf8;

		sy -= 0x10;

		for (INT32 y = sy; y < sy + 8; y++) {
			for (INT32 x = sx; x < sx + 8; x++, src++) {
				INT32 pxl = color | *src;
				if (Prom[0x300 | pxl] == 0x0f) continue;
				pTransDraw[(y << 8) | x] = pxl;
			}
		}
	}

	if (vulgus_flipscreen) {
		INT32 nSize = (nScreenWidth * nScreenHeight) - 1;
		for (INT32 i = 0; i < nSize >> 1; i++) {
			INT32 n = pTransDraw[i];
			pTransDraw[i] = pTransDraw[nSize - i];
			pTransDraw[nSize - i] = n;
		}
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}


static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	INT32 nInterleave = 8;
	INT32 nSoundBufferPos = 0;
	INT32 nCycles[2] = { 4000000 / 60, 3000000 / 60 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		ZetOpen(0);
		nCycles[0] -= ZetRun(nCycles[0] / (nInterleave - i));
		if (i == ((nInterleave / 2) - 1)) {
			ZetSetVector(0xd7);
			ZetRaiseIrq(0);
		}
		if (i == ( nInterleave      - 1)) {
			ZetSetVector(0xcf);
			ZetRaiseIrq(0);
		}
		ZetClose();

		ZetOpen(1);
		nCycles[1] -= ZetRun(nCycles[1] / (nInterleave - i));
		ZetRaiseIrq(0);
		ZetClose();

		if (pBurnSoundOut) {
			INT32 nSample;
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n] >> 2;
				nSample += pAY8910Buffer[1][n] >> 2;
				nSample += pAY8910Buffer[2][n] >> 2;
				nSample += pAY8910Buffer[3][n] >> 2;
				nSample += pAY8910Buffer[4][n] >> 2;
				nSample += pAY8910Buffer[5][n] >> 2;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) | 0] = nSample;
				pSoundBuf[(n << 1) | 1] = nSample;
    		}
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (pBurnSoundOut) {
		INT32 nSample;
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			AY8910Update(1, &pAY8910Buffer[3], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n] >> 2;
				nSample += pAY8910Buffer[1][n] >> 2;
				nSample += pAY8910Buffer[2][n] >> 2;
				nSample += pAY8910Buffer[3][n] >> 2;
				nSample += pAY8910Buffer[4][n] >> 2;
				nSample += pAY8910Buffer[5][n] >> 2;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) | 0] = nSample;
				pSoundBuf[(n << 1) | 1] = nSample;
 			}
		}
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
		*pnMin = 0x029521;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = Rom0 + 0xcc00;
		ba.nLen	  = 0x2400;
		ba.szName = "All CPU #0 Ram";
		BurnAcb(&ba);

		ba.Data	  = Rom1 + 0x4000;
		ba.nLen	  = 0x0800;
		ba.szName = "All CPU #1 Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		AY8910Scan(nAction, pnMin);

		SCAN_VAR(vulgus_flipscreen);
		SCAN_VAR(vulgus_soundlatch);
		SCAN_VAR(vulgus_palette_bank);
		SCAN_VAR(vulgus_scroll[0]);
		SCAN_VAR(vulgus_scroll[1]);
	}

	return 0;
}


// Vulgus (set 1)

static struct BurnRomInfo vulgusRomDesc[] = {
	{ "vulgus.002",   0x2000, 0xe49d6c5d, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "vulgus.003",   0x2000, 0x51acef76, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "vulgus.004",   0x2000, 0x489e7f60, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "vulgus.005",   0x2000, 0xde3a24a8, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "1-8n.bin",     0x2000, 0x6ca5ca41, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "1-11c.bin",    0x2000, 0x3bd2acf4, 2 | BRF_PRG | BRF_ESS }, //  5 Z80 #1 Code

	{ "1-3d.bin",     0x2000, 0x8bc5d7a5, 3 | BRF_GRA },	       //  6 Foreground Tiles

	{ "2-2a.bin",     0x2000, 0xe10aaca1, 4 | BRF_GRA },	       //  7 Background Tiles
	{ "2-3a.bin",     0x2000, 0x8da520da, 4 | BRF_GRA },	       //  8
	{ "2-4a.bin",     0x2000, 0x206a13f1, 4 | BRF_GRA },	       //  9
	{ "2-5a.bin",     0x2000, 0xb6d81984, 4 | BRF_GRA },	       // 10
	{ "2-6a.bin",     0x2000, 0x5a26b38f, 4 | BRF_GRA },	       // 11 
	{ "2-7a.bin",     0x2000, 0x1e1ca773, 4 | BRF_GRA },	       // 12 

	{ "2-2n.bin",     0x2000, 0x6db1b10d, 5 | BRF_GRA },	       // 13 Sprites
	{ "2-3n.bin",     0x2000, 0x5d8c34ec, 5 | BRF_GRA },	       // 14
	{ "2-4n.bin",     0x2000, 0x0071a2e3, 5 | BRF_GRA },	       // 15
	{ "2-5n.bin",     0x2000, 0x4023a1ec, 5 | BRF_GRA },	       // 16

	{ "e8.bin",       0x0100, 0x06a83606, 6 | BRF_GRA },	       // 17 Color Proms
	{ "e9.bin",       0x0100, 0xbeacf13c, 6 | BRF_GRA },	       // 18
	{ "e10.bin",      0x0100, 0xde1fb621, 6 | BRF_GRA },	       // 19
	{ "d1.bin",       0x0100, 0x7179080d, 6 | BRF_GRA },	       // 20
	{ "j2.bin",       0x0100, 0xd0842029, 6 | BRF_GRA },	       // 21
	{ "c9.bin",       0x0100, 0x7a1f0bd6, 6 | BRF_GRA },	       // 22

	{ "82s126.9k",    0x0100, 0x32b10521, 0 | BRF_OPT },	       // 23 Misc. Proms
	{ "82s129.8n",    0x0100, 0x4921635c, 0 | BRF_OPT },	       // 24
};

STD_ROM_PICK(vulgus)
STD_ROM_FN(vulgus)

struct BurnDriver BurnDrvvulgus = {
	"vulgus", NULL, NULL, NULL, "1984",
	"Vulgus (set 1)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, vulgusRomInfo, vulgusRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	224, 256, 3, 4
};


// Vulgus (set 2)

static struct BurnRomInfo vulgusaRomDesc[] = {
	{ "v2",           0x2000, 0x3e18ff62, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "v3",           0x2000, 0xb4650d82, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "v4",           0x2000, 0x5b26355c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "v5",           0x2000, 0x4ca7f10e, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "1-8n.bin",     0x2000, 0x6ca5ca41, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "1-11c.bin",    0x2000, 0x3bd2acf4, 2 | BRF_PRG | BRF_ESS }, //  5 Z80 #1 Code

	{ "1-3d.bin",     0x2000, 0x8bc5d7a5, 3 | BRF_GRA },	       //  6 Foreground Tiles

	{ "2-2a.bin",     0x2000, 0xe10aaca1, 4 | BRF_GRA },	       //  7 Background Tiles
	{ "2-3a.bin",     0x2000, 0x8da520da, 4 | BRF_GRA },	       //  8
	{ "2-4a.bin",     0x2000, 0x206a13f1, 4 | BRF_GRA },	       //  9
	{ "2-5a.bin",     0x2000, 0xb6d81984, 4 | BRF_GRA },	       // 10
	{ "2-6a.bin",     0x2000, 0x5a26b38f, 4 | BRF_GRA },	       // 11 
	{ "2-7a.bin",     0x2000, 0x1e1ca773, 4 | BRF_GRA },	       // 12 

	{ "2-2n.bin",     0x2000, 0x6db1b10d, 5 | BRF_GRA },	       // 13 Sprites
	{ "2-3n.bin",     0x2000, 0x5d8c34ec, 5 | BRF_GRA },	       // 14
	{ "2-4n.bin",     0x2000, 0x0071a2e3, 5 | BRF_GRA },	       // 15
	{ "2-5n.bin",     0x2000, 0x4023a1ec, 5 | BRF_GRA },	       // 16

	{ "e8.bin",       0x0100, 0x06a83606, 6 | BRF_GRA },	       // 17 Color Proms
	{ "e9.bin",       0x0100, 0xbeacf13c, 6 | BRF_GRA },	       // 18
	{ "e10.bin",      0x0100, 0xde1fb621, 6 | BRF_GRA },	       // 19
	{ "d1.bin",       0x0100, 0x7179080d, 6 | BRF_GRA },	       // 20
	{ "j2.bin",       0x0100, 0xd0842029, 6 | BRF_GRA },	       // 21
	{ "c9.bin",       0x0100, 0x7a1f0bd6, 6 | BRF_GRA },	       // 22

	{ "82s126.9k",    0x0100, 0x32b10521, 0 | BRF_OPT },	       // 23 Misc. Proms
	{ "82s129.8n",    0x0100, 0x4921635c, 0 | BRF_OPT },	       // 24
};

STD_ROM_PICK(vulgusa)
STD_ROM_FN(vulgusa)

struct BurnDriver BurnDrvvulgusa = {
	"vulgusa", "vulgus", NULL, NULL, "1984",
	"Vulgus (set 2)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, vulgusaRomInfo, vulgusaRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	224, 256, 3, 4
};


// Vulgus (Japan)

static struct BurnRomInfo vulgusjRomDesc[] = {
	{ "1-4n.bin",     0x2000, 0xfe5a5ca5, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "1-5n.bin",     0x2000, 0x847e437f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "1-6n.bin",     0x2000, 0x4666c436, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "1-7n.bin",     0x2000, 0xff2097f9, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "1-8n.bin",     0x2000, 0x6ca5ca41, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "1-11c.bin",    0x2000, 0x3bd2acf4, 2 | BRF_PRG | BRF_ESS }, //  5 Z80 #1 Code

	{ "1-3d.bin",     0x2000, 0x8bc5d7a5, 3 | BRF_GRA },	       //  6 Foreground Tiles

	{ "2-2a.bin",     0x2000, 0xe10aaca1, 4 | BRF_GRA },	       //  7 Background Tiles
	{ "2-3a.bin",     0x2000, 0x8da520da, 4 | BRF_GRA },	       //  8
	{ "2-4a.bin",     0x2000, 0x206a13f1, 4 | BRF_GRA },	       //  9
	{ "2-5a.bin",     0x2000, 0xb6d81984, 4 | BRF_GRA },	       // 10
	{ "2-6a.bin",     0x2000, 0x5a26b38f, 4 | BRF_GRA },	       // 11 
	{ "2-7a.bin",     0x2000, 0x1e1ca773, 4 | BRF_GRA },	       // 12 

	{ "2-2n.bin",     0x2000, 0x6db1b10d, 5 | BRF_GRA },	       // 13 Sprites
	{ "2-3n.bin",     0x2000, 0x5d8c34ec, 5 | BRF_GRA },	       // 14
	{ "2-4n.bin",     0x2000, 0x0071a2e3, 5 | BRF_GRA },	       // 15
	{ "2-5n.bin",     0x2000, 0x4023a1ec, 5 | BRF_GRA },	       // 16

	{ "e8.bin",       0x0100, 0x06a83606, 6 | BRF_GRA },	       // 17 Color Proms
	{ "e9.bin",       0x0100, 0xbeacf13c, 6 | BRF_GRA },	       // 18
	{ "e10.bin",      0x0100, 0xde1fb621, 6 | BRF_GRA },	       // 19
	{ "d1.bin",       0x0100, 0x7179080d, 6 | BRF_GRA },	       // 20
	{ "j2.bin",       0x0100, 0xd0842029, 6 | BRF_GRA },	       // 21
	{ "c9.bin",       0x0100, 0x7a1f0bd6, 6 | BRF_GRA },	       // 22

	{ "82s126.9k",    0x0100, 0x32b10521, 0 | BRF_OPT },	       // 23 Misc. Proms
	{ "82s129.8n",    0x0100, 0x4921635c, 0 | BRF_OPT },	       // 24
};

STD_ROM_PICK(vulgusj)
STD_ROM_FN(vulgusj)

struct BurnDriver BurnDrvvulgusj = {
	"vulgusj", "vulgus", NULL, NULL, "1984",
	"Vulgus (Japan?)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, vulgusjRomInfo, vulgusjRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	224, 256, 3, 4
};
