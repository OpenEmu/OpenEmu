// FB Alpha Gun.Smoke driver module
// Based on MAME driver by Paul Leaman

#include "tiles_generic.h"
#include "zet.h"
#include "burn_ym2203.h"

static UINT8 *Mem, *MemEnd, *Rom0, *Rom1, *Ram;
static UINT8 *Gfx0, *Gfx1, *Gfx2, *Gfx3, *Prom;
static UINT8 DrvJoy1[8], DrvJoy2[8], DrvJoy3[8], DrvDips[2], DrvReset;
static UINT32 *Palette, *DrvPal;
static UINT8 DrvCalcPal;
static UINT8 *SprTrnsp;

static UINT8 soundlatch;
static UINT8 flipscreen;
static INT32 nGunsmokeBank;

static UINT8 sprite3bank;
static UINT8 chon, bgon, objon;
static UINT8 gunsmoke_scrollx[2], gunsmoke_scrolly;


static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy1 + 6,	"p1 coin"  },
	{"P1 start"     , BIT_DIGITAL  , DrvJoy1 + 0,	"p1 start" },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy2 + 0, "p1 right" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy2 + 1, "p1 left"  },
	{"P1 Down"      ,	BIT_DIGITAL  , DrvJoy2 + 2, "p1 down"  },
	{"P1 Up"        ,	BIT_DIGITAL  , DrvJoy2 + 3, "p1 up"    },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 5,	"p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy2 + 6,	"p1 fire 3"},

	{"P2 Coin"      , BIT_DIGITAL  , DrvJoy1 + 7,	"p2 coin"  },
	{"P2 start"     , BIT_DIGITAL  , DrvJoy1 + 1,	"p2 start" },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 0, "p2 right" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 1, "p2 left"  },
	{"P2 Down"      ,	BIT_DIGITAL  , DrvJoy3 + 2, "p2 down"  },
	{"P2 Up"        ,	BIT_DIGITAL  , DrvJoy3 + 3, "p2 up"    },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 4,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy3 + 5,	"p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 6,	"p2 fire 3"},

	{"Service"      ,	BIT_DIGITAL  , DrvJoy1 + 4,	"service"  },

	{"Reset"        ,	BIT_DIGITAL  , &DrvReset  ,	"reset"    },
	{"Dip 1"        ,	BIT_DIPSWITCH, DrvDips + 0,	"dip"	     },
	{"Dip 2"        ,	BIT_DIPSWITCH, DrvDips + 1,	"dip"	     },

};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xf7, NULL               },

	{0   , 0xfe, 0   , 4   , "Bonus Life"       },
	{0x14, 0x01, 0x03, 0x01, "30k 80k 80k+"     },
	{0x14, 0x01, 0x03, 0x03, "30k 100k 100k+"	  },
	{0x14, 0x01, 0x03, 0x00, "30k 100k 150k+"	  },
	{0x14, 0x01, 0x03, 0x02, "30k 100k"    		  },

	{0   , 0xfe, 0   , 2   , "Demo"             },
	{0x14, 0x01, 0x04, 0x00, "Off"     		      },
	{0x14, 0x01, 0x04, 0x04, "On"    		        },
};

static struct BurnDIPInfo gunsmokaDIPList[]=
{
	// Default Values
	{0x14, 0xff, 0xff, 0xf7, NULL               },

	{0   , 0xfe, 0   , 4   , "Bonus Life"       },
	{0x14, 0x01, 0x03, 0x01, "30k 80k 80k+"     },
	{0x14, 0x01, 0x03, 0x03, "30k 100k 100k+"	  },
	{0x14, 0x01, 0x03, 0x00, "30k 100k 150k+"	  },
	{0x14, 0x01, 0x03, 0x02, "30k 100k"    		  },

	{0   , 0xfe, 0   , 2   , "Lifes"            },
	{0x14, 0x01, 0x04, 0x04, "3"     		        },
	{0x14, 0x01, 0x04, 0x00, "5"    		        },
};

static struct BurnDIPInfo gunsmokeDIPList[]=
{
	{0   , 0xfe, 0   , 2   , "Cabinet"          },
	{0x14, 0x01, 0x08, 0x00, "Upright"     		  },
	{0x14, 0x01, 0x08, 0x08, "Cocktail"    		  },

	{0   , 0xfe, 0   , 4   , "Difficulty"       },
	{0x14, 0x01, 0x30, 0x20, "Easy"     		    },
	{0x14, 0x01, 0x30, 0x30, "Normal"    		    },
	{0x14, 0x01, 0x30, 0x10, "Difficult"   		  },
	{0x14, 0x01, 0x30, 0x00, "Very Difficult"	  },

	{0   , 0xfe, 0   , 2   , "Freeze"           },
	{0x14, 0x01, 0x40, 0x40, "Off"     		      },
	{0x14, 0x01, 0x40, 0x00, "On"    		        },

	{0   , 0xfe, 0   , 2   , "Service Mode"     },
	{0x14, 0x01, 0x80, 0x80, "Off"     		      },
	{0x14, 0x01, 0x80, 0x00, "On"    		        },

	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL               },

	{0   , 0xfe, 0   , 8  ,  "Coin A"           },
	{0x15, 0x01, 0x07, 0x00, "4 Coins 1 Credit" },
	{0x15, 0x01, 0x07, 0x01, "3 Coins 1 Credit"	},
	{0x15, 0x01, 0x07, 0x02, "2 Coins 1 Credit"	},
	{0x15, 0x01, 0x07, 0x07, "1 Coin 1 Credit"  },
	{0x15, 0x01, 0x07, 0x06, "1 Coin 2 Credits" },
	{0x15, 0x01, 0x07, 0x05, "1 Coin 3 Credits"	},
	{0x15, 0x01, 0x07, 0x04, "1 Coin 4 Credits"	},
	{0x15, 0x01, 0x07, 0x03, "1 Coin 6 Credits" },

	{0   , 0xfe, 0   , 8  ,  "Coin B"           },
	{0x15, 0x01, 0x38, 0x00, "4 Coins 1 Credit" },
	{0x15, 0x01, 0x38, 0x08, "3 Coins 1 Credit"	},
	{0x15, 0x01, 0x38, 0x10, "2 Coins 1 Credit"	},
	{0x15, 0x01, 0x38, 0x38, "1 Coin 1 Credit"  },
	{0x15, 0x01, 0x38, 0x30, "1 Coin 2 Credits" },
	{0x15, 0x01, 0x38, 0x28, "1 Coin 3 Credits"	},
	{0x15, 0x01, 0x38, 0x20, "1 Coin 4 Credits"	},
	{0x15, 0x01, 0x38, 0x18, "1 Coin 6 Credits" },

	{0   , 0xfe, 0   , 2   , "Allow Continue"   },
	{0x15, 0x01, 0x40, 0x00, "No"    		        },
	{0x15, 0x01, 0x40, 0x40, "Yes"     		      },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"      },
	{0x15, 0x01, 0x80, 0x00, "Off"    		      },
	{0x15, 0x01, 0x80, 0x80, "On"     		      },
};

STDDIPINFOEXT(Drv, Drv, gunsmoke)
STDDIPINFOEXT(gunsmoka, gunsmoka, gunsmoke)


static inline void gunsmoke_bankswitch(INT32 nBank)
{
	if (nGunsmokeBank != nBank) {
		nGunsmokeBank = nBank;

		ZetMapArea(0x8000, 0xbfff, 0, Rom0 + 0x10000 + nBank * 0x04000);
		ZetMapArea(0x8000, 0xbfff, 2, Rom0 + 0x10000 + nBank * 0x04000);
	}
}

void __fastcall gunsmoke_cpu0_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xc800:
			soundlatch = data;
		break;

		case 0xc804:
			gunsmoke_bankswitch((data >> 2) & 3);

			flipscreen = data & 0x40;
			chon       = data & 0x80;
		break;

		case 0xc806:
		break;

		case 0xd800:
		case 0xd801:
			gunsmoke_scrollx[address & 1] = data;
		break;

		case 0xd802:
		case 0xd803:
			gunsmoke_scrolly = data;
		break;

		case 0xd806:
			sprite3bank = data & 0x07;
			
			bgon        = data & 0x10;
			objon       = data & 0x20;
		break;
	}
}

UINT8 __fastcall gunsmoke_cpu0_read(UINT16 address)
{
	UINT8 ret = 0xff;

	switch (address)
	{
		case 0xc000:
		{
			for (INT32 i = 0; i < 8; i++)
				ret ^= DrvJoy1[i] << i;

			return ret | 0x08;
		}

		case 0xc001:
		{
			for (INT32 i = 0; i < 8; i++)
				ret ^= DrvJoy2[i] << i;

			return ret;
		}

		case 0xc002:
		{
			for (INT32 i = 0; i < 8; i++)
				ret ^= DrvJoy3[i] << i;

			return ret;
		}

		case 0xc003: // dips
			return DrvDips[0];

		case 0xc004:
			return DrvDips[1];

		// reads at c4c9 - c4cb are part of some sort of protection or bug
		case 0xc4c9:
			return 0xff;
		case 0xc4ca:
		case 0xc4cb:
			return 0;
	}

	return 0;
}

void __fastcall gunsmoke_cpu1_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe000: // control 0
			BurnYM2203Write(0, 0, data);
		break;

		case 0xe001: // write 0
			BurnYM2203Write(0, 1, data);
		break;

		case 0xe002: // control 1
			BurnYM2203Write(1, 0, data);
		break;

		case 0xe003: // write 1
			BurnYM2203Write(1, 1, data);
		break;
	}
}

UINT8 __fastcall gunsmoke_cpu1_read(UINT16 address)
{
	if (address == 0xc800) return soundlatch;

	return 0;
}


static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (Ram, 0, 0x4000);

	nGunsmokeBank = -1;
	soundlatch = 0;
	flipscreen = 0;

	sprite3bank = 0;
	chon = bgon = objon = 0;
	gunsmoke_scrollx[0] = gunsmoke_scrollx[1] = 0;
	gunsmoke_scrolly = 0;

	ZetOpen(0);
	ZetReset();
	gunsmoke_bankswitch(0);
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	BurnYM2203Reset();

	return 0;
}

static INT32 gunsmoke_palette_init()
{
	INT32 i, ctabentry;
	UINT32 tmp[0x100];

	for (i = 0; i < 0x100; i++)
	{
		UINT8 r, g, b;

		r  = Prom[i + 0x000] & 0x0f;
		r |= r << 4;
		g  = Prom[i + 0x100] & 0x0f;
		g |= g << 4;
		b  = Prom[i + 0x200] & 0x0f;
 		b |= b << 4;

		tmp[i] = (r << 16) | (g << 8) | b;
	}

	for (i = 0; i < 0x100; i++)
	{
		ctabentry = Prom[0x300 + i] | 0x40;
		Palette[0x000 + i] = tmp[ctabentry];

		ctabentry = Prom[0x400 + i] | ((Prom[0x500 + i] & 0x03) << 4);
		Palette[0x100 + i] = tmp[ctabentry];

		ctabentry = Prom[0x600 + i] | ((Prom[0x700 + i] & 0x07) << 4) | 0x80;
		Palette[0x200 + i] = tmp[ctabentry];
	}

	return 0;
}

static INT32 gunsmoke_gfx_decode()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x80000);
	if (!tmp) return 1;

	static INT32 Planes[4]     = { 0x100004, 0x100000, 4, 0 };

	static INT32 CharXOffs[8]  = { 11, 10, 9, 8, 3, 2, 1, 0 };
	static INT32 CharYOffs[8]  = { 112, 96, 80, 64, 48, 32, 16, 0 };

	static INT32 TileXOffs[32] = {    0,    1,    2,    3,    8,    9,   10,   11,
				      512,  513,  514,  515,  520,  521,  522,  523,
				     1024, 1025, 1026, 1027, 1032, 1033, 1034, 1035,
				     1536, 1537, 1538, 1539, 1544, 1545, 1546, 1547 };

	static INT32 TileYOffs[32] = {   0,  16,  32,  48,  64,  80,  96, 112,
				     128, 144, 160, 176, 192, 208, 224, 240,
				     256, 272, 288, 304, 320, 336, 352, 368,
				     384, 400, 416, 432, 448, 464, 480, 496 };

	static INT32 SpriXOffs[16] = {   0,   1,   2,   3,   8,   9,  10,  11,
				     256, 257, 258, 259, 264, 265, 266, 267 };

	memcpy (tmp, Gfx0, 0x04000);
	GfxDecode(0x400, 2,  8,  8, Planes + 2, CharXOffs, CharYOffs, 0x080, tmp, Gfx0);

	memcpy (tmp, Gfx1, 0x40000);
	GfxDecode(0x200, 4, 32, 32, Planes + 0, TileXOffs, TileYOffs, 0x800, tmp, Gfx1);

	memcpy (tmp, Gfx2, 0x40000);
	GfxDecode(0x800, 4, 16, 16, Planes + 0, SpriXOffs, TileYOffs, 0x200, tmp, Gfx2);

	BurnFree (tmp);

	{
		memset (SprTrnsp, 1, 0x800);

		for (INT32 i = 0; i < 0x80000; i++)
			if (Gfx2[i]) SprTrnsp[i >> 8] = 0;
	}

	return 0;
}

static INT32 gunsmokeSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 3000000;
}

static double gunsmokeGetTime()
{
	return (double)ZetTotalCycles() / 3000000;
}


static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Rom0           = Next; Next += 0x20000;
	Rom1           = Next; Next += 0x08000;
	Ram            = Next; Next += 0x04000;
	Gfx0           = Next; Next += 0x10000;
	Gfx1           = Next; Next += 0x80000;
	Gfx2           = Next; Next += 0x80000;
	Gfx3           = Next; Next += 0x08000;
	Prom           = Next; Next += 0x00800;

	SprTrnsp       = Next; Next += 0x00800;

	Palette	       = (UINT32*)Next; Next += 0x00300 * sizeof(UINT32);
	DrvPal	       = (UINT32*)Next; Next += 0x00300 * sizeof(UINT32);

	MemEnd                 = Next;

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

	{
		if (BurnLoadRom(Rom0 + 0x00000,  0, 1)) return 1;
		if (BurnLoadRom(Rom0 + 0x10000,  1, 1)) return 1;
		if (BurnLoadRom(Rom0 + 0x18000,  2, 1)) return 1;

		if (BurnLoadRom(Rom1 + 0x00000,  3, 1)) return 1;

		if (BurnLoadRom(Gfx0 + 0x00000,  4, 1)) return 1;

		if (BurnLoadRom(Gfx3 + 0x00000, 21, 1)) return 1;

		for (INT32 i = 0; i < 8; i++) {
			if (BurnLoadRom(Gfx1 + i * 0x8000,  5 + i, 1)) return 1;
			if (BurnLoadRom(Gfx2 + i * 0x8000, 13 + i, 1)) return 1;
			if (BurnLoadRom(Prom + i * 0x0100, 22 + i, 1)) return 1;
		}

		gunsmoke_gfx_decode();
		gunsmoke_palette_init();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, Rom0 + 0x00000);
	ZetMapArea(0x0000, 0x7fff, 2, Rom0 + 0x00000);
	ZetMapArea(0x8000, 0xbfff, 0, Rom0 + 0x10000);
	ZetMapArea(0x8000, 0xbfff, 2, Rom0 + 0x10000);
	ZetMapArea(0xd000, 0xd7ff, 0, Ram  + 0x00000);
	ZetMapArea(0xd000, 0xd7ff, 1, Ram  + 0x00000);
	ZetMapArea(0xe000, 0xefff, 0, Ram  + 0x01000);
	ZetMapArea(0xe000, 0xefff, 1, Ram  + 0x01000);
	ZetMapArea(0xe000, 0xefff, 2, Ram  + 0x01000);
	ZetMapArea(0xf000, 0xffff, 0, Ram  + 0x02000);
	ZetMapArea(0xf000, 0xffff, 1, Ram  + 0x02000);
	ZetSetReadHandler(gunsmoke_cpu0_read);
	ZetSetWriteHandler(gunsmoke_cpu0_write);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, Rom1 + 0x00000);
	ZetMapArea(0x0000, 0x7fff, 2, Rom1 + 0x00000);
	ZetMapArea(0xc000, 0xc7ff, 0, Ram  + 0x03000);
	ZetMapArea(0xc000, 0xc7ff, 1, Ram  + 0x03000);
	ZetMapArea(0xc000, 0xc7ff, 2, Ram  + 0x03000);
	ZetSetReadHandler(gunsmoke_cpu1_read);
	ZetSetWriteHandler(gunsmoke_cpu1_write);
	ZetMemEnd();
	ZetClose();

	GenericTilesInit();

	BurnYM2203Init(2, 1500000, NULL, gunsmokeSynchroniseStream, gunsmokeGetTime, 0);
	BurnYM2203SetVolumeShift(2);
	BurnTimerAttachZet(3000000);

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();
	BurnYM2203Exit();

	BurnFree (Mem);

	Mem = MemEnd = Rom0 = Rom1 = Ram = NULL;
	Gfx0 = Gfx1 = Gfx2 = Gfx3 = Prom = NULL;
	SprTrnsp = NULL;
	Palette = DrvPal = NULL;

	soundlatch = flipscreen = nGunsmokeBank;

	sprite3bank = chon = bgon = objon = 0;
	gunsmoke_scrollx[0] = gunsmoke_scrollx[1] = 0;
	gunsmoke_scrolly = 0;

	return 0;
}

static void draw_bg_layer()
{
	UINT16 scroll = gunsmoke_scrollx[0] + (gunsmoke_scrollx[1] << 8);

 	UINT8 *tilerom = Gfx3 + ((scroll >> 1) & ~0x0f);

	for (INT32 offs = 0; offs < 0x50; offs++)
	{
		INT32 attr = tilerom[1];
		INT32 code = tilerom[0] + ((attr & 1) << 8);
		INT32 color = (attr & 0x3c) >> 2;
		INT32 flipy = attr & 0x80;
		INT32 flipx = attr & 0x40;

		INT32 sy = (offs & 7) << 5;
		INT32 sx = (offs >> 3) << 5;

		sy -= gunsmoke_scrolly;
		sx -= (scroll & 0x1f);

		if (flipscreen) {
			flipy ^= 0x80;
			flipx ^= 0x40;

			sy = 224 - sy;
			sx = 224 - sx;
		}

		sy -= 16;

		if (flipy) {
			if (flipx) {
				Render32x32Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0x100, Gfx1);
			} else {
				Render32x32Tile_FlipY_Clip(pTransDraw,  code, sx, sy, color, 4, 0x100, Gfx1);
			}
		} else {
			if (flipx) {
				Render32x32Tile_FlipX_Clip(pTransDraw,  code, sx, sy, color, 4, 0x100, Gfx1);
			} else {
				Render32x32Tile_Clip(pTransDraw,        code, sx, sy, color, 4, 0x100, Gfx1);
			}
		}

		tilerom += 2;
	}
}

static void draw_fg_layer()
{
	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 sx = (offs << 3) & 0xf8;
		INT32 sy = (offs >> 2) & 0xf8;

		INT32 attr = Ram[0x0400 + offs];
		INT32 code = Ram[0x0000 + offs] + ((attr & 0xe0) << 2);
		INT32 color = attr & 0x1f;

		if (code == 0x0024) continue;

		UINT8 *src = Gfx0 + (code << 6);
		color <<= 2;

		if (flipscreen) {
			sy = 240 - sy;
			sx = 240 - sx;

			sy -= 8;

			for (INT32 y = sy + 7; y >= sy; y--)
			{
				for (INT32 x = sx + 7; x >= sx; x--, src++)
				{
					if (y < 0 || x < 0 || y > 223 || x > 255) continue;
					if (!Palette[color|*src]) continue;

					pTransDraw[(y << 8) | x] = color | *src;
				}
			}
		} else {
			sy -= 16;

			for (INT32 y = sy; y < sy + 8; y++)
			{
				for (INT32 x = sx; x < sx + 8; x++, src++)
				{
					if (y < 0 || x < 0 || y > 223 || x > 255) continue;
					if (!Palette[color|*src]) continue;

					pTransDraw[(y << 8) | x] = color | *src;
				}
			}
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0x1000 - 32; offs >= 0; offs -= 32)
	{
		INT32 attr = Ram[0x2001 + offs];
		INT32 bank = (attr & 0xc0) >> 6;
		INT32 code = Ram[0x2000 + offs];
		INT32 color = attr & 0x0f;
		INT32 flipx = 0;
		INT32 flipy = attr & 0x10;
		INT32 sx = Ram[0x2003 + offs] - ((attr & 0x20) << 3);
		INT32 sy = Ram[0x2002 + offs];

		if (sy == 0 || sy > 0xef) continue;

		if (bank == 3) bank += sprite3bank;
		code += 256 * bank;

		if (SprTrnsp[code]) continue;

		if (flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sy -= 16;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x200, Gfx2);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x200, Gfx2);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x200, Gfx2);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0x200, Gfx2);
			}
		}
	}
}

static INT32 DrvDraw()
{
	// Recalculate palette
	if (DrvCalcPal) {
		for (INT32 i = 0; i < 0x300; i++) {
			UINT32 col = Palette[i];
			DrvPal[i] = BurnHighCol(col >> 16, col >> 8, col, 0);
		}
	}

	if (!bgon) memset (pTransDraw, 0, 224 * 256 * 2);

	if (bgon)  draw_bg_layer();
	if (objon) draw_sprites();
	if (chon)  draw_fg_layer();

	BurnTransferCopy(DrvPal);

	return 0;
}


static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();

	INT32 nInterleave = 25;
	INT32 nSoundBufferPos = 0;

	INT32 nCyclesSegment;
	INT32 nCyclesDone[2], nCyclesTotal[2];

	nCyclesTotal[0] = 4000000 / 60;
	nCyclesTotal[1] = 3000000 / 60;
	
	nCyclesDone[0] = nCyclesDone[1] = 0;

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run Z80 #0
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == 20) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		if (i == 21) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		ZetClose();

		// Run Z80 #1
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));
		if (i == 5 || i == 10 || i == 15 || i == 20) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
 		if (i == 6 || i == 11 || i == 16 || i == 21) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
	
		ZetClose();
		
		// Render Sound Segment
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			ZetOpen(1);
			BurnYM2203Update(pSoundBuf, nSegmentLength);
			ZetClose();
			nSoundBufferPos += nSegmentLength;
		}

	}
	
	ZetOpen(1);
	BurnTimerEndFrame(nCyclesTotal[1]);
	ZetClose();

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			ZetOpen(1);
			BurnYM2203Update(pSoundBuf, nSegmentLength);
			ZetClose();
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

		ba.Data	  = Ram;
		ba.nLen	  = 0x4000;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		BurnYM2203Scan(nAction, pnMin);

		// Scan critical driver variables
		SCAN_VAR(soundlatch);
		SCAN_VAR(flipscreen);
		SCAN_VAR(nGunsmokeBank);
		SCAN_VAR(sprite3bank);
		SCAN_VAR(chon);
		SCAN_VAR(bgon);
		SCAN_VAR(objon);
		SCAN_VAR(gunsmoke_scrollx[0]);
		SCAN_VAR(gunsmoke_scrollx[1]);
		SCAN_VAR(gunsmoke_scrolly);
	}

	return 0;
}


// Gun. Smoke (World)

static struct BurnRomInfo gunsmokeRomDesc[] = {
	{ "09n_gs03.bin", 0x8000, 0x40a06cef, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "10n_gs04.bin", 0x8000, 0x8d4b423f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "12n_gs05.bin", 0x8000, 0x2b5667fb, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "14h_gs02.bin", 0x8000, 0xcd7a2c38, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "11f_gs01.bin", 0x4000, 0xb61ece9b, 3 | BRF_GRA },	       //  4 Character Tiles

	{ "06c_gs13.bin", 0x8000, 0xf6769fc5, 4 | BRF_GRA },	       //  5 32x32 Tiles
	{ "05c_gs12.bin", 0x8000, 0xd997b78c, 4 | BRF_GRA },	       //  6
	{ "04c_gs11.bin", 0x8000, 0x125ba58e, 4 | BRF_GRA },	       //  7
	{ "02c_gs10.bin", 0x8000, 0xf469c13c, 4 | BRF_GRA },	       //  8
	{ "06a_gs09.bin", 0x8000, 0x539f182d, 4 | BRF_GRA },	       //  9
	{ "05a_gs08.bin", 0x8000, 0xe87e526d, 4 | BRF_GRA },	       // 10
	{ "04a_gs07.bin", 0x8000, 0x4382c0d2, 4 | BRF_GRA },	       // 11 
	{ "02a_gs06.bin", 0x8000, 0x4cafe7a6, 4 | BRF_GRA },	       // 12 

	{ "06n_gs22.bin", 0x8000, 0xdc9c508c, 5 | BRF_GRA },	       // 13 Sprites
	{ "04n_gs21.bin", 0x8000, 0x68883749, 5 | BRF_GRA },	       // 14
	{ "03n_gs20.bin", 0x8000, 0x0be932ed, 5 | BRF_GRA },	       // 15
	{ "01n_gs19.bin", 0x8000, 0x63072f93, 5 | BRF_GRA },	       // 16
	{ "06l_gs18.bin", 0x8000, 0xf69a3c7c, 5 | BRF_GRA },	       // 17
	{ "04l_gs17.bin", 0x8000, 0x4e98562a, 5 | BRF_GRA },	       // 18
	{ "03l_gs16.bin", 0x8000, 0x0d99c3b3, 5 | BRF_GRA },	       // 19
	{ "01l_gs15.bin", 0x8000, 0x7f14270e, 5 | BRF_GRA },	       // 20

	{ "11c_gs14.bin", 0x8000, 0x0af4f7eb, 6 | BRF_GRA },	       // 21 Background Tilemaps

	{ "03b_g-01.bin", 0x0100, 0x02f55589, 7 | BRF_GRA },	       // 22 Color Proms
	{ "04b_g-02.bin", 0x0100, 0xe1e36dd9, 7 | BRF_GRA },	       // 23
	{ "05b_g-03.bin", 0x0100, 0x989399c0, 7 | BRF_GRA },	       // 24
	{ "09d_g-04.bin", 0x0100, 0x906612b5, 7 | BRF_GRA },	       // 25
	{ "14a_g-06.bin", 0x0100, 0x4a9da18b, 7 | BRF_GRA },	       // 26
	{ "15a_g-07.bin", 0x0100, 0xcb9394fc, 7 | BRF_GRA },	       // 27
	{ "09f_g-09.bin", 0x0100, 0x3cee181e, 7 | BRF_GRA },	       // 28
	{ "08f_g-08.bin", 0x0100, 0xef91cdd2, 7 | BRF_GRA },	       // 29

	{ "02j_g-10.bin", 0x0100, 0x0eaf5158, 0 | BRF_OPT },	       // 30 Video Timing
	{ "01f_g-05.bin", 0x0100, 0x25c90c2a, 0 | BRF_OPT },	       // 31 Priority
};

STD_ROM_PICK(gunsmoke)
STD_ROM_FN(gunsmoke)

struct BurnDriver BurnDrvGunsmoke = {
	"gunsmoke", NULL, NULL, NULL, "1985",
	"Gun. Smoke (World)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, gunsmokeRomInfo, gunsmokeRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvCalcPal, 0x300,
	224, 256, 3, 4
};


// Gun. Smoke (Japan)

static struct BurnRomInfo gunsmokjRomDesc[] = {
	{ "gs03_9n.rom",  0x8000, 0xb56b5df6, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "10n_gs04.bin", 0x8000, 0x8d4b423f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "12n_gs05.bin", 0x8000, 0x2b5667fb, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "14h_gs02.bin", 0x8000, 0xcd7a2c38, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "11f_gs01.bin", 0x4000, 0xb61ece9b, 3 | BRF_GRA },	       //  4 Character Tiles

	{ "06c_gs13.bin", 0x8000, 0xf6769fc5, 4 | BRF_GRA },	       //  5 32x32 Tiles
	{ "05c_gs12.bin", 0x8000, 0xd997b78c, 4 | BRF_GRA },	       //  6
	{ "04c_gs11.bin", 0x8000, 0x125ba58e, 4 | BRF_GRA },	       //  7
	{ "02c_gs10.bin", 0x8000, 0xf469c13c, 4 | BRF_GRA },	       //  8
	{ "06a_gs09.bin", 0x8000, 0x539f182d, 4 | BRF_GRA },	       //  9
	{ "05a_gs08.bin", 0x8000, 0xe87e526d, 4 | BRF_GRA },	       // 10
	{ "04a_gs07.bin", 0x8000, 0x4382c0d2, 4 | BRF_GRA },	       // 11 
	{ "02a_gs06.bin", 0x8000, 0x4cafe7a6, 4 | BRF_GRA },	       // 12 

	{ "06n_gs22.bin", 0x8000, 0xdc9c508c, 5 | BRF_GRA },	       // 13 Sprites
	{ "04n_gs21.bin", 0x8000, 0x68883749, 5 | BRF_GRA },	       // 14
	{ "03n_gs20.bin", 0x8000, 0x0be932ed, 5 | BRF_GRA },	       // 15
	{ "01n_gs19.bin", 0x8000, 0x63072f93, 5 | BRF_GRA },	       // 16
	{ "06l_gs18.bin", 0x8000, 0xf69a3c7c, 5 | BRF_GRA },	       // 17
	{ "04l_gs17.bin", 0x8000, 0x4e98562a, 5 | BRF_GRA },	       // 18
	{ "03l_gs16.bin", 0x8000, 0x0d99c3b3, 5 | BRF_GRA },	       // 19
	{ "01l_gs15.bin", 0x8000, 0x7f14270e, 5 | BRF_GRA },	       // 20

	{ "11c_gs14.bin", 0x8000, 0x0af4f7eb, 6 | BRF_GRA },	       // 21 Background Tilemaps

	{ "03b_g-01.bin", 0x0100, 0x02f55589, 7 | BRF_GRA },	       // 22 Color Proms
	{ "04b_g-02.bin", 0x0100, 0xe1e36dd9, 7 | BRF_GRA },	       // 23
	{ "05b_g-03.bin", 0x0100, 0x989399c0, 7 | BRF_GRA },	       // 24
	{ "09d_g-04.bin", 0x0100, 0x906612b5, 7 | BRF_GRA },	       // 25
	{ "14a_g-06.bin", 0x0100, 0x4a9da18b, 7 | BRF_GRA },	       // 26
	{ "15a_g-07.bin", 0x0100, 0xcb9394fc, 7 | BRF_GRA },	       // 27
	{ "09f_g-09.bin", 0x0100, 0x3cee181e, 7 | BRF_GRA },	       // 28
	{ "08f_g-08.bin", 0x0100, 0xef91cdd2, 7 | BRF_GRA },	       // 29

	{ "02j_g-10.bin", 0x0100, 0x0eaf5158, 0 | BRF_OPT },	       // 30 Video Timing
	{ "01f_g-05.bin", 0x0100, 0x25c90c2a, 0 | BRF_OPT },	       // 31 Priority
};

STD_ROM_PICK(gunsmokj)
STD_ROM_FN(gunsmokj)

struct BurnDriver BurnDrvGunsmokj = {
	"gunsmokej", "gunsmoke", NULL, NULL, "1985",
	"Gun. Smoke (Japan)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, gunsmokjRomInfo, gunsmokjRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvCalcPal, 0x300,
	224, 256, 3, 4
};


// Gun. Smoke (US set 1)

static struct BurnRomInfo gunsmokuRomDesc[] = {
	{ "9n_gs03.bin",  0x8000, 0x592f211b, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "10n_gs04.bin", 0x8000, 0x8d4b423f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "12n_gs05.bin", 0x8000, 0x2b5667fb, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "14h_gs02.bin", 0x8000, 0xcd7a2c38, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "11f_gs01.bin", 0x4000, 0xb61ece9b, 3 | BRF_GRA },	       //  4 Character Tiles

	{ "06c_gs13.bin", 0x8000, 0xf6769fc5, 4 | BRF_GRA },	       //  5 32x32 Tiles
	{ "05c_gs12.bin", 0x8000, 0xd997b78c, 4 | BRF_GRA },	       //  6
	{ "04c_gs11.bin", 0x8000, 0x125ba58e, 4 | BRF_GRA },	       //  7
	{ "02c_gs10.bin", 0x8000, 0xf469c13c, 4 | BRF_GRA },	       //  8
	{ "06a_gs09.bin", 0x8000, 0x539f182d, 4 | BRF_GRA },	       //  9
	{ "05a_gs08.bin", 0x8000, 0xe87e526d, 4 | BRF_GRA },	       // 10
	{ "04a_gs07.bin", 0x8000, 0x4382c0d2, 4 | BRF_GRA },	       // 11 
	{ "02a_gs06.bin", 0x8000, 0x4cafe7a6, 4 | BRF_GRA },	       // 12 

	{ "06n_gs22.bin", 0x8000, 0xdc9c508c, 5 | BRF_GRA },	       // 13 Sprites
	{ "04n_gs21.bin", 0x8000, 0x68883749, 5 | BRF_GRA },	       // 14
	{ "03n_gs20.bin", 0x8000, 0x0be932ed, 5 | BRF_GRA },	       // 15
	{ "01n_gs19.bin", 0x8000, 0x63072f93, 5 | BRF_GRA },	       // 16
	{ "06l_gs18.bin", 0x8000, 0xf69a3c7c, 5 | BRF_GRA },	       // 17
	{ "04l_gs17.bin", 0x8000, 0x4e98562a, 5 | BRF_GRA },	       // 18
	{ "03l_gs16.bin", 0x8000, 0x0d99c3b3, 5 | BRF_GRA },	       // 19
	{ "01l_gs15.bin", 0x8000, 0x7f14270e, 5 | BRF_GRA },	       // 20

	{ "11c_gs14.bin", 0x8000, 0x0af4f7eb, 6 | BRF_GRA },	       // 21 Background Tilemaps

	{ "03b_g-01.bin", 0x0100, 0x02f55589, 7 | BRF_GRA },	       // 22 Color Proms
	{ "04b_g-02.bin", 0x0100, 0xe1e36dd9, 7 | BRF_GRA },	       // 23
	{ "05b_g-03.bin", 0x0100, 0x989399c0, 7 | BRF_GRA },	       // 24
	{ "09d_g-04.bin", 0x0100, 0x906612b5, 7 | BRF_GRA },	       // 25
	{ "14a_g-06.bin", 0x0100, 0x4a9da18b, 7 | BRF_GRA },	       // 26
	{ "15a_g-07.bin", 0x0100, 0xcb9394fc, 7 | BRF_GRA },	       // 27
	{ "09f_g-09.bin", 0x0100, 0x3cee181e, 7 | BRF_GRA },	       // 28
	{ "08f_g-08.bin", 0x0100, 0xef91cdd2, 7 | BRF_GRA },	       // 29

	{ "02j_g-10.bin", 0x0100, 0x0eaf5158, 0 | BRF_OPT },	       // 30 Video Timing
	{ "01f_g-05.bin", 0x0100, 0x25c90c2a, 0 | BRF_OPT },	       // 31 Priority
};

STD_ROM_PICK(gunsmoku)
STD_ROM_FN(gunsmoku)

struct BurnDriver BurnDrvGunsmoku = {
	"gunsmokeu", "gunsmoke", NULL, NULL, "1985",
	"Gun. Smoke (US set 1)\0", NULL, "Capcom (Romstar License)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, gunsmokuRomInfo, gunsmokuRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvCalcPal, 0x300,
	224, 256, 3, 4
};


// Gun. Smoke (US set 2)

static struct BurnRomInfo gunsmokaRomDesc[] = {
	{ "gs03.9n",      0x8000, 0x51dc3f76, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "gs04.10n",     0x8000, 0x5ecf31b8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "gs05.12n",     0x8000, 0x1c9aca13, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "14h_gs02.bin", 0x8000, 0xcd7a2c38, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "11f_gs01.bin", 0x4000, 0xb61ece9b, 3 | BRF_GRA },	       //  4 Character Tiles

	{ "06c_gs13.bin", 0x8000, 0xf6769fc5, 4 | BRF_GRA },	       //  5 32x32 Tiles
	{ "05c_gs12.bin", 0x8000, 0xd997b78c, 4 | BRF_GRA },	       //  6
	{ "04c_gs11.bin", 0x8000, 0x125ba58e, 4 | BRF_GRA },	       //  7
	{ "02c_gs10.bin", 0x8000, 0xf469c13c, 4 | BRF_GRA },	       //  8
	{ "06a_gs09.bin", 0x8000, 0x539f182d, 4 | BRF_GRA },	       //  9
	{ "05a_gs08.bin", 0x8000, 0xe87e526d, 4 | BRF_GRA },	       // 10
	{ "04a_gs07.bin", 0x8000, 0x4382c0d2, 4 | BRF_GRA },	       // 11 
	{ "02a_gs06.bin", 0x8000, 0x4cafe7a6, 4 | BRF_GRA },	       // 12 

	{ "06n_gs22.bin", 0x8000, 0xdc9c508c, 5 | BRF_GRA },	       // 13 Sprites
	{ "04n_gs21.bin", 0x8000, 0x68883749, 5 | BRF_GRA },	       // 14
	{ "03n_gs20.bin", 0x8000, 0x0be932ed, 5 | BRF_GRA },	       // 15
	{ "01n_gs19.bin", 0x8000, 0x63072f93, 5 | BRF_GRA },	       // 16
	{ "06l_gs18.bin", 0x8000, 0xf69a3c7c, 5 | BRF_GRA },	       // 17
	{ "04l_gs17.bin", 0x8000, 0x4e98562a, 5 | BRF_GRA },	       // 18
	{ "03l_gs16.bin", 0x8000, 0x0d99c3b3, 5 | BRF_GRA },	       // 19
	{ "01l_gs15.bin", 0x8000, 0x7f14270e, 5 | BRF_GRA },	       // 20

	{ "11c_gs14.bin", 0x8000, 0x0af4f7eb, 6 | BRF_GRA },	       // 21 Background Tilemaps

	{ "03b_g-01.bin", 0x0100, 0x02f55589, 7 | BRF_GRA },	       // 22 Color Proms
	{ "04b_g-02.bin", 0x0100, 0xe1e36dd9, 7 | BRF_GRA },	       // 23
	{ "05b_g-03.bin", 0x0100, 0x989399c0, 7 | BRF_GRA },	       // 24
	{ "09d_g-04.bin", 0x0100, 0x906612b5, 7 | BRF_GRA },	       // 25
	{ "14a_g-06.bin", 0x0100, 0x4a9da18b, 7 | BRF_GRA },	       // 26
	{ "15a_g-07.bin", 0x0100, 0xcb9394fc, 7 | BRF_GRA },	       // 27
	{ "09f_g-09.bin", 0x0100, 0x3cee181e, 7 | BRF_GRA },	       // 28
	{ "08f_g-08.bin", 0x0100, 0xef91cdd2, 7 | BRF_GRA },	       // 29

	{ "02j_g-10.bin", 0x0100, 0x0eaf5158, 0 | BRF_OPT },	       // 30 Video Timing
	{ "01f_g-05.bin", 0x0100, 0x25c90c2a, 0 | BRF_OPT },	       // 31 Priority
};

STD_ROM_PICK(gunsmoka)
STD_ROM_FN(gunsmoka)

struct BurnDriver BurnDrvGunsmoka = {
	"gunsmokeua", "gunsmoke", NULL, NULL, "1986",
	"Gun. Smoke (US set 2)\0", NULL, "Capcom (Romstar License)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, gunsmokaRomInfo, gunsmokaRomName, NULL, NULL, DrvInputInfo, gunsmokaDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvCalcPal, 0x300,
	224, 256, 3, 4
};

