// FB Alpha Exed Exes driver module
// Based on MAME driver by Richard Davies, Paul Swan, and various others

#include "tiles_generic.h"
#include "zet.h"
#include "sn76496.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *Mem, *MemEnd;
static UINT8 *Rom0, *Rom1, *Gfx0, *Gfx1, *Gfx2, *Gfx3, *Gfx4, *Prom;
static INT16 *pAY8910Buffer[3], *pFMBuffer = NULL;
static UINT32 *Palette, *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *fg_tile_transp;

static UINT8 DrvJoy1[8], DrvJoy2[8], DrvJoy3[8], DrvDips[2], DrvInputs[3], DrvReset;

static UINT8 exedexes_soundlatch;

static UINT8 exedexes_txt_enable;
static UINT8 exedexes_obj_enable;
static UINT8 exedexes_bg_enable;
static UINT8 exedexes_fg_enable;

static UINT16 exedexes_nbg_yscroll;
static UINT16 exedexes_nbg_xscroll;
static UINT16 exedexes_bg_xscroll;

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy1 + 6,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy1 + 7,	"p2 coin"  },
	{"P1 Start"     , BIT_DIGITAL  , DrvJoy1 + 0,	"p1 start" },
	{"P2 Start"     , BIT_DIGITAL  , DrvJoy1 + 1,	"p2 start" },

	{"P1 Right"     , BIT_DIGITAL  , DrvJoy2 + 0, "p1 right" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy2 + 1, "p1 left"  },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy2 + 2, "p1 down"  },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy2 + 3, "p1 up"    },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 5,	"p1 fire 2"},

	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 0, "p2 right" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 1, "p2 left"  },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy3 + 2, "p2 down"  },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy3 + 3, "p2 up"    },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 4,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 5,	"p2 fire 2"},

	{"Service"      , BIT_DIGITAL  , DrvJoy1 + 5, "service"  },

	{"Reset"        , BIT_DIGITAL  , &DrvReset  ,	"reset"    },
	{"Dip 1"        , BIT_DIPSWITCH, DrvDips + 0, "dip 1"    },
	{"Dip 2"        , BIT_DIPSWITCH, DrvDips + 1, "dip 2"    },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xdf, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x12, 0x01, 0x03, 0x02, "Easy"                   },
	{0x12, 0x01, 0x03, 0x03, "Normal"                 },
	{0x12, 0x01, 0x03, 0x01, "Hard"                   },
	{0x12, 0x01, 0x03, 0x00, "Hardest"                },

	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0x0c, 0x08, "1"                      },
	{0x12, 0x01, 0x0c, 0x04, "2"                      },
	{0x12, 0x01, 0x0c, 0x0c, "3"                      },
	{0x12, 0x01, 0x0c, 0x00, "5"                      },

	{0   , 0xfe, 0   , 2   , "2 Players Game"         },
	{0x12, 0x01, 0x10, 0x00, "1 Credit"               },
	{0x12, 0x01, 0x10, 0x10, "2 Credits"              },

	{0   , 0xfe, 0   , 2   , "Language"               },
	{0x12, 0x01, 0x20, 0x00, "English"                },
	{0x12, 0x01, 0x20, 0x20, "Japanese"               },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x12, 0x01, 0x40, 0x40, "Off"                    },
	{0x12, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x12, 0x01, 0x80, 0x80, "Off"                    },
	{0x12, 0x01, 0x80, 0x00, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x13, 0x01, 0x07, 0x00, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x01, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x07, 0x03, "1 Coin  5 Plays"        },

	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x13, 0x01, 0x38, 0x00, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x08, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x10, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
	{0x13, 0x01, 0x38, 0x20, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x38, 0x18, "1 Coin  5 Plays"        },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x13, 0x01, 0x40, 0x00, "No"                     },
	{0x13, 0x01, 0x40, 0x40, "Yes"                    },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x13, 0x01, 0x80, 0x00, "Off"                    },
	{0x13, 0x01, 0x80, 0x80, "On"                     },
};

STDDIPINFO(Drv)

UINT8 __fastcall exedexes_cpu0_read(UINT16 address)
{
	switch (address)
	{
		case 0xc000:
		case 0xc001:
		case 0xc002:
			return DrvInputs[address & 3];	

		case 0xc003:
		case 0xc004:
			return DrvDips[~address & 1];
	}

	return 0;
}

void __fastcall exedexes_cpu0_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xc800:
			exedexes_soundlatch = data;
		break;

		case 0xc804:
			exedexes_txt_enable = data >> 7;
		break;

		case 0xc806:
		break;

		case 0xd800:
			exedexes_nbg_yscroll = (exedexes_nbg_yscroll & 0xff00) | (data << 0);
		break;

		case 0xd801:
			exedexes_nbg_yscroll = (exedexes_nbg_yscroll & 0x00ff) | (data << 8);
		break;

		case 0xd802:
			exedexes_nbg_xscroll = (exedexes_nbg_xscroll & 0xff00) | (data << 0);
		break;

		case 0xd803:
			exedexes_nbg_xscroll = (exedexes_nbg_xscroll & 0x00ff) | (data << 8);
		break;

		case 0xd804:
			exedexes_bg_xscroll  = (exedexes_bg_xscroll  & 0xff00) | (data << 0);
		break;

		case 0xd805:
			exedexes_bg_xscroll  = (exedexes_bg_xscroll  & 0x00ff) | (data << 8);
		break;

		case 0xd807:
			exedexes_bg_enable  = (data >> 4) & 1;
			exedexes_fg_enable  = (data >> 5) & 1;
			exedexes_obj_enable = (data >> 6) & 1;
		break;
	}
}

UINT8 __fastcall exedexes_cpu1_read(UINT16 address)
{
	switch (address)
	{
		case 0x6000:
			return exedexes_soundlatch;
	}

	return 0;
}

void __fastcall exedexes_cpu1_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x8000:
		case 0x8001:
			AY8910Write(0, address & 1, data);
		break;

		case 0x8002:
		case 0x8003:
			SN76496Write(address & 1, data);
		break;
	}
}

static inline void DrvMakeInputs()
{
	DrvInputs[0] = DrvInputs[1] = DrvInputs[2] = 0xff;

	for (INT32 i = 0; i < 8; i++) {
		DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
	}
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (Rom0 + 0xd000, 0, 0x3000);
	memset (Rom1 + 0x4000, 0, 0x0800);

	exedexes_soundlatch = 0;
	exedexes_txt_enable = 0;
	exedexes_obj_enable = 0;
	exedexes_bg_enable = 0;
	exedexes_fg_enable = 0;
	exedexes_nbg_yscroll = 0;
	exedexes_nbg_xscroll = 0;
	exedexes_bg_xscroll = 0;

	for (INT32 i = 0; i < 2; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}

	AY8910Reset(0);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Rom0           = Next; Next += 0x10000;
	Rom1           = Next; Next += 0x05000;

	Gfx0           = Next; Next += 0x08000;
	Gfx1           = Next; Next += 0x10000;
	Gfx2           = Next; Next += 0x10000;
	Gfx3           = Next; Next += 0x10000;

	Gfx4           = Next; Next += 0x06000;

	Prom           = Next; Next += 0x00800;

	fg_tile_transp = Next; Next += 0x00100;

	Palette	       = (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);
	DrvPalette     = (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);

	pFMBuffer      = (INT16*)Next; Next += (nBurnSoundLen * 3 * sizeof(INT16));

	MemEnd         = Next;

	return 0;
}

static INT32 PaletteInit()
{
	UINT32 *tmp = (UINT32*)BurnMalloc(0x100 * sizeof(UINT32));
	if (tmp == NULL) {
		return 1;
	}

	for (INT32 i = 0; i < 0x100; i++)
	{
		INT32 r = Prom[i + 0x000];
		INT32 g = Prom[i + 0x100];
		INT32 b = Prom[i + 0x200];

		tmp[i] = (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | b;
	}

	Prom += 0x300;

	for (INT32 i = 0; i < 0x100; i++) {
		Palette[i] = tmp[Prom[i] | 0xc0];
	}

	for (INT32 i = 0x100; i < 0x200; i++) {
		Palette[i] = tmp[Prom[i] | 0x00];
	}

	for (INT32 i = 0x200; i < 0x300; i++) {
		Palette[i] = tmp[Prom[i] | 0x40];
	}

	for (INT32 i = 0x300; i < 0x400; i++) {
		INT32 entry = Prom[i] | (Prom[i + 0x100] << 4) | 0x80;
		Palette[i] = tmp[entry];
	}

	BurnFree (tmp);

	return 0;
}

static INT32 GraphicsDecode()
{
	static INT32 TilePlanes[2] = { 0x004, 0x000 };
	static INT32 SpriPlanes[4] = { 0x20004, 0x20000, 0x00004, 0x00000 };
	static INT32 TileXOffs[32] = { 0x000, 0x001, 0x002, 0x003, 0x008, 0x009, 0x00a, 0x00b, 
				     0x200, 0x201, 0x202, 0x203, 0x208, 0x209, 0x20a, 0x20b, 
				     0x400, 0x401, 0x402, 0x403, 0x408, 0x409, 0x40a, 0x40b, 
				     0x600, 0x601, 0x602, 0x603, 0x608, 0x609, 0x60a, 0x60b };
	static INT32 SpriXOffs[16] = { 0x000, 0x001, 0x002, 0x003, 0x008, 0x009, 0x00a, 0x00b, 
				     0x100, 0x101, 0x102, 0x103, 0x108, 0x109, 0x10a, 0x10b };
	static INT32 TileYOffs[32] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070, 
				     0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0, 
				     0x100, 0x110, 0x120, 0x130, 0x140, 0x150, 0x160, 0x170, 
				     0x180, 0x190, 0x1a0, 0x1b0, 0x1c0, 0x1d0, 0x1e0, 0x1f0 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x8000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, Gfx0, 0x02000);

	GfxDecode(0x200, 2,  8,  8, TilePlanes, TileXOffs, TileYOffs, 0x080, tmp, Gfx0);

	memcpy (tmp, Gfx1, 0x04000);

	GfxDecode(0x040, 2, 32, 32, TilePlanes, TileXOffs, TileYOffs, 0x800, tmp, Gfx1);

	memcpy (tmp, Gfx2, 0x08000);

	GfxDecode(0x100, 4, 16, 16, SpriPlanes, SpriXOffs, TileYOffs, 0x200, tmp, Gfx2);

	memcpy (tmp, Gfx3, 0x08000);

	GfxDecode(0x100, 4, 16, 16, SpriPlanes, SpriXOffs, TileYOffs, 0x200, tmp, Gfx3);

	for (INT32 i = 0; i < 0x10000; i++) {
		if (Gfx2[i]) fg_tile_transp[i>>8] = 1;
	}

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

	for (INT32 i = 0; i < 3; i++) {
		pAY8910Buffer[i] = pFMBuffer + nBurnSoundLen * i;
	}

	{
		for (INT32 i = 0; i < 3; i++) {
			if (BurnLoadRom(Rom0 + i * 0x4000, i +  0, 1)) return 1;
		}

		if (BurnLoadRom(Rom1, 3, 1)) return 1;
		if (BurnLoadRom(Gfx0, 4, 1)) return 1;
		if (BurnLoadRom(Gfx1, 5, 1)) return 1;

		for (INT32 i = 0; i < 2; i++) {
			if (BurnLoadRom(Gfx2 + i * 0x4000, i +  6, 1)) return 1;
			if (BurnLoadRom(Gfx3 + i * 0x4000, i +  8, 1)) return 1;
			if (BurnLoadRom(Gfx4 + i * 0x4000, i + 10, 1)) return 1;
		}

		for (INT32 i = 0; i < 8; i++) {
			if (BurnLoadRom(Prom + i * 0x0100, i + 12, 1)) return 1;
		}

		if (GraphicsDecode()) return 1;
		if (PaletteInit()) return 1;
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, Rom0 + 0x0000);
	ZetMapArea(0x0000, 0xbfff, 2, Rom0 + 0x0000);
	ZetMapArea(0xd000, 0xd7ff, 0, Rom0 + 0xd000);
	ZetMapArea(0xd000, 0xd7ff, 1, Rom0 + 0xd000);
	ZetMapArea(0xe000, 0xefff, 0, Rom0 + 0xe000);
	ZetMapArea(0xe000, 0xefff, 1, Rom0 + 0xe000);
	ZetMapArea(0xe000, 0xefff, 2, Rom0 + 0xe000);
	ZetMapArea(0xf000, 0xffff, 0, Rom0 + 0xf000);
	ZetMapArea(0xf000, 0xffff, 1, Rom0 + 0xf000);
	ZetSetWriteHandler(exedexes_cpu0_write);
	ZetSetReadHandler(exedexes_cpu0_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x3fff, 0, Rom1 + 0x0000);
	ZetMapArea(0x0000, 0x3fff, 2, Rom1 + 0x0000);
	ZetMapArea(0x4000, 0x47ff, 0, Rom1 + 0x4000);
	ZetMapArea(0x4000, 0x47ff, 1, Rom1 + 0x4000);
	ZetMapArea(0x4000, 0x47ff, 2, Rom1 + 0x4000);
	ZetSetWriteHandler(exedexes_cpu1_write);
	ZetSetReadHandler(exedexes_cpu1_read);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);

	SN76489Init(0, 3000000, 0);
	SN76489Init(1, 3000000, 1);
	SN76496SetVolShift(0, 2);
	SN76496SetVolShift(1, 2);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	AY8910Exit(0);
	SN76496Exit();
	ZetExit();
	GenericTilesExit();

	BurnFree (Mem);

	Mem = MemEnd = Rom0 = Rom1 = NULL;
	Gfx0 = Gfx1 = Gfx2 = Gfx3 = Gfx4 = Prom = NULL;
	for (INT32 i = 0; i < 3; i++) pAY8910Buffer[i] = NULL;
	Palette = DrvPalette = NULL;
	fg_tile_transp = NULL;
	pFMBuffer = NULL;
	DrvRecalc = 0;

	return 0;
}


static void draw_sprites(INT32 priority)
{
	if (!exedexes_obj_enable) return;

	for (INT32 offs = 0x0fe0; offs >= 0; offs -= 0x20)
	{
		if ((Rom0[0xf000 + offs + 1] & 0x40) == priority)
		{
			INT32 code  = Rom0[0xf000 + offs];
			INT32 color = Rom0[0xf001 + offs] & 0x0f;
			INT32 flipx = Rom0[0xf001 + offs] & 0x10;
			INT32 flipy = Rom0[0xf001 + offs] & 0x20;
			INT32 sx    = Rom0[0xf003 + offs] - ((Rom0[0xf001 + offs] & 0x80) << 1);
			INT32 sy    = Rom0[0xf002 + offs] - 0x10;

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x300, Gfx3);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x300, Gfx3);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x300, Gfx3);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x300, Gfx3);
				}
			}
		}
	}
}

static inline void draw_8x8(INT32 sx, INT32 sy, INT32 code, INT32 color)
{
	UINT8 *src = Gfx0 + (code << 6);
	color <<= 2;

	for (INT32 y = sy; y < sy + 8; y++) {
		for (INT32 x = sx; x < sx + 8; x++, src++) {
			if (y < 0 || x < 0 || y >= nScreenHeight || x >= nScreenWidth) continue;

			INT32 pxl = color | *src;
			if (Prom[pxl] == 0x0f) continue;

			pTransDraw[(y * nScreenWidth) + x] = pxl;
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x400; i++) {
			INT32 col = Palette[i];
			DrvPalette[i] = BurnHighCol(col >> 16, col >> 8, col, 0);
		}
	}

	if (exedexes_bg_enable)
	{
		for (INT32 i = 0; i < 16 * 8; i++) {
			INT32 sx = ((i & 0x0f) << 5);
			INT32 sy = (i >> 4) << 5;

			INT32 sxscr = sx + exedexes_bg_xscroll;

			sx -= (exedexes_bg_xscroll & 0x1f);

			if (sx > 0x100) continue;
			sy -= 0x10;

			INT32 offset = ((sxscr & 0xe0) >> 5) | ((sy & 0xe0) >> 2) | ((sxscr & 0x3f00) >> 1) | 0x4000;

			INT32 attr  = Gfx4[offset];
			INT32 color = Gfx4[offset + 0x40];
			INT32 code  = attr & 0x3f;
			INT32 flipx = attr & 0x40;
			INT32 flipy = attr & 0x80;

			if (flipy) {
				if (flipx) {
					Render32x32Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0x100, Gfx1);
				} else {
					Render32x32Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 2, 0x100, Gfx1);
				}
			} else {
				if (flipx) {
					Render32x32Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 2, 0x100, Gfx1);
				} else {
					Render32x32Tile_Clip(pTransDraw, code, sx, sy, color, 2, 0x100, Gfx1);
				}
			}
		}
	} else {
		memset(pTransDraw, 0, nScreenWidth * nScreenHeight * sizeof (UINT16));
	}

	draw_sprites(0x40);

	if (exedexes_fg_enable)
	{
		for (INT32 i = 0; i < 32 * 16; i++) {
			INT32 sx = (i & 0x1f) << 4;
			INT32 sy = (i >> 5) << 4;

			INT32 vx = sx + exedexes_nbg_yscroll;
			INT32 vy = sy + exedexes_nbg_xscroll;

			sx -= exedexes_nbg_yscroll & 0x0f;
			sy -= exedexes_nbg_xscroll & 0x0f;

			if (sx > 0x100) continue;

			INT32 offset = ((vx & 0xf0) >> 4) | (vy & 0xf0) | (vx & 0x700) | ((vy & 0x700) << 3);

			INT32 code = Gfx4[offset];

			if (!fg_tile_transp[code]) continue; 
			sy -= 0x10;

			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, 0, 2, 0, 0x200, Gfx2);
		}
	}

	draw_sprites(0);

	if (exedexes_txt_enable)
	{
		for (INT32 i = 0x40; i < 0x3c0; i++) {
			INT32 sx    = (i & 0x1f) << 3;
			INT32 sy    = ((i >> 5) << 3) - 0x10;
			INT32 code  = Rom0[0xd000 | i] | ((Rom0[0xd400 | i] & 0x80) << 1);
			INT32 color = Rom0[0xd400 | i] & 0x3f;

			if (code == 0x0024) continue;

			draw_8x8(sx, sy, code, color);
			//Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, Gfx0);
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

	DrvMakeInputs();

	INT32 nInterleave = 16;
	INT32 nCyclesDone[2] = { 0, 0 };
	INT32 nCyclesTotal[2] = { 4000000 / 60, 3000000 / 60 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nCurrentCPU, nNext, nCyclesSegment;
		
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		if (i == 0) {
			ZetSetVector(0xcf);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			nCyclesDone[nCurrentCPU] += ZetRun(100);
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		}
		if (i == ( nInterleave - 2)) {
			ZetSetVector(0xd7);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
			nCyclesDone[nCurrentCPU] += ZetRun(100);
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		}
		ZetClose();
		
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		if ((i & 3) == 3) {
			ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		}
		ZetClose();
	}

	if (pBurnSoundOut) {
		SN76496Update(0, pBurnSoundOut, nBurnSoundLen);
		SN76496Update(1, pBurnSoundOut, nBurnSoundLen);

		INT32 nSample;
		if (nBurnSoundLen) {
			AY8910Update(0, &pAY8910Buffer[0], nBurnSoundLen);
			for (INT32 n = 0; n < nBurnSoundLen; n++) {
				nSample  = pAY8910Buffer[0][n];
				nSample += pAY8910Buffer[1][n];
				nSample += pAY8910Buffer[2][n];

				nSample >>= 4;

				nSample = BURN_SND_CLIP(nSample);
				
				pBurnSoundOut[(n << 1) + 0] += nSample;
				pBurnSoundOut[(n << 1) + 1] += nSample;
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

		ba.Data	  = Rom0 + 0xd000;
		ba.nLen	  = 0x3000;
		ba.szName = "All CPU #0 Ram";
		BurnAcb(&ba);

		ba.Data	  = Rom1 + 0x4000;
		ba.nLen	  = 0x0800;
		ba.szName = "All CPU #1 Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		AY8910Scan(nAction, pnMin);
		SN76496Scan(nAction, pnMin);

		SCAN_VAR(exedexes_soundlatch);
		SCAN_VAR(exedexes_txt_enable);
		SCAN_VAR(exedexes_obj_enable);
		SCAN_VAR(exedexes_bg_enable);
		SCAN_VAR(exedexes_fg_enable);
		SCAN_VAR(exedexes_nbg_yscroll);
		SCAN_VAR(exedexes_nbg_xscroll);
		SCAN_VAR(exedexes_bg_xscroll);
	}

	return 0;
}


// Exed Exes

static struct BurnRomInfo exedexesRomDesc[] = {
	{ "11m_ee04.bin", 0x4000, 0x44140dbd, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "10m_ee03.bin", 0x4000, 0xbf72cfba, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "09m_ee02.bin", 0x4000, 0x7ad95e2f, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "11e_ee01.bin", 0x4000, 0x73cdf3b2, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "05c_ee00.bin", 0x2000, 0xcadb75bd, 3 | BRF_GRA },	       //  4 Characters

	{ "h01_ee08.bin", 0x4000, 0x96a65c1d, 4 | BRF_GRA },	       //  5 32x32 tiles

	{ "a03_ee06.bin", 0x4000, 0x6039bdd1, 5 | BRF_GRA },	       //  6 16x16 tiles
	{ "a02_ee05.bin", 0x4000, 0xb32d8252, 5 | BRF_GRA },	       //  7

	{ "j11_ee10.bin", 0x4000, 0xbc83e265, 6 | BRF_GRA },	       //  8 Sprites
	{ "j12_ee11.bin", 0x4000, 0x0e0f300d, 6 | BRF_GRA },	       //  9

	{ "c01_ee07.bin", 0x4000, 0x3625a68d, 7 | BRF_GRA },	       // 10 Tile Maps
	{ "h04_ee09.bin", 0x2000, 0x6057c907, 7 | BRF_GRA },	       // 11

	{ "02d_e-02.bin", 0x0100, 0x8d0d5935, 8 | BRF_GRA },	       // 12 Color Proms
	{ "03d_e-03.bin", 0x0100, 0xd3c17efc, 8 | BRF_GRA },	       // 13
	{ "04d_e-04.bin", 0x0100, 0x58ba964c, 8 | BRF_GRA },	       // 14
	{ "06f_e-05.bin", 0x0100, 0x35a03579, 8 | BRF_GRA },	       // 15
	{ "l04_e-10.bin", 0x0100, 0x1dfad87a, 8 | BRF_GRA },	       // 16
	{ "c04_e-07.bin", 0x0100, 0x850064e0, 8 | BRF_GRA },	       // 17
	{ "l09_e-11.bin", 0x0100, 0x2bb68710, 8 | BRF_GRA },	       // 18
	{ "l10_e-12.bin", 0x0100, 0x173184ef, 8 | BRF_GRA },	       // 19

	{ "06l_e-06.bin", 0x0100, 0x712ac508, 0 | BRF_OPT },	       // 20 Misc. Proms
	{ "k06_e-08.bin", 0x0100, 0x0eaf5158, 0 | BRF_OPT },	       // 21
	{ "l03_e-09.bin", 0x0100, 0x0d968558, 0 | BRF_OPT },	       // 22
	{ "03e_e-01.bin", 0x0020, 0x1acee376, 0 | BRF_OPT },	       // 23
};

STD_ROM_PICK(exedexes)
STD_ROM_FN(exedexes)

struct BurnDriver BurnDrvExedexes = {
	"exedexes", NULL, NULL, NULL, "1985",
	"Exed Exes\0", NULL, "Capcom", "Miscellaneous",
	L"Exed Exes\0\u30A8\u30B0\u30BC\u30C9 \u30A8\u30B0\u30BC\u30B9\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, exedexesRomInfo, exedexesRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	224, 256, 3, 4
};


// Savage Bees

static struct BurnRomInfo savgbeesRomDesc[] = {
	{ "ee04e.11m",    0x4000, 0xc0caf442, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "ee03e.10m",    0x4000, 0x9cd70ae1, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ee02e.9m",     0x4000, 0xa04e6368, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "ee01e.11e",    0x4000, 0x93d3f952, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "ee00e.5c",     0x2000, 0x5972f95f, 3 | BRF_GRA },	       //  4 Characters

	{ "h01_ee08.bin", 0x4000, 0x96a65c1d, 4 | BRF_GRA },	       //  5 32x32 tiles

	{ "a03_ee06.bin", 0x4000, 0x6039bdd1, 5 | BRF_GRA },	       //  6 16x16 tiles
	{ "a02_ee05.bin", 0x4000, 0xb32d8252, 5 | BRF_GRA },	       //  7

	{ "j11_ee10.bin", 0x4000, 0xbc83e265, 6 | BRF_GRA },	       //  8 Sprites
	{ "j12_ee11.bin", 0x4000, 0x0e0f300d, 6 | BRF_GRA },	       //  9

	{ "c01_ee07.bin", 0x4000, 0x3625a68d, 7 | BRF_GRA },	       // 10 Tile Maps
	{ "h04_ee09.bin", 0x2000, 0x6057c907, 7 | BRF_GRA },	       // 11

	{ "02d_e-02.bin", 0x0100, 0x8d0d5935, 8 | BRF_GRA },	       // 12 Color Proms
	{ "03d_e-03.bin", 0x0100, 0xd3c17efc, 8 | BRF_GRA },	       // 13
	{ "04d_e-04.bin", 0x0100, 0x58ba964c, 8 | BRF_GRA },	       // 14
	{ "06f_e-05.bin", 0x0100, 0x35a03579, 8 | BRF_GRA },	       // 15
	{ "l04_e-10.bin", 0x0100, 0x1dfad87a, 8 | BRF_GRA },	       // 16
	{ "c04_e-07.bin", 0x0100, 0x850064e0, 8 | BRF_GRA },	       // 17
	{ "l09_e-11.bin", 0x0100, 0x2bb68710, 8 | BRF_GRA },	       // 18
	{ "l10_e-12.bin", 0x0100, 0x173184ef, 8 | BRF_GRA },	       // 19

	{ "06l_e-06.bin", 0x0100, 0x712ac508, 0 | BRF_OPT },	       // 20 Misc. Proms
	{ "k06_e-08.bin", 0x0100, 0x0eaf5158, 0 | BRF_OPT },	       // 21
	{ "l03_e-09.bin", 0x0100, 0x0d968558, 0 | BRF_OPT },	       // 22
	{ "03e_e-01.bin", 0x0020, 0x1acee376, 0 | BRF_OPT },	       // 23
};

STD_ROM_PICK(savgbees)
STD_ROM_FN(savgbees)

struct BurnDriver BurnDrvSavgbees = {
	"savgbees", "exedexes", NULL, NULL, "1985",
	"Savage Bees\0", NULL, "Capcom (Memetron license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, savgbeesRomInfo, savgbeesRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	224, 256, 3, 4
};
