// FB Alpha Mr. Do! driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "z80_intf.h"
#include "sn76496.h"

static UINT8 *Mem, *Rom, *Gfx0, *Gfx1, *Gfx2, *Prom;
static UINT8 DrvJoy1[8], DrvJoy2[8], DrvReset, DrvDips[2];
static INT32 *Palette;

static INT32 flipscreen, scroll_x, scroll_y;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy2 + 6,	"p1 coin"  },
	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 5,	"p1 start" },
	{"P1 Up"	, BIT_DIGITAL  , DrvJoy1 + 3,   "p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 1,   "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 2, 	"p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	"p1 fire 1"},

	{"P2 Coin"      , BIT_DIGITAL  , DrvJoy2 + 7,	"p2 coin"  },
	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 6,	"p2 start" },
	{"P2 Up"	, BIT_DIGITAL  , DrvJoy2 + 3,   "p2 up"    },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy2 + 1,   "p2 down", },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 0, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 2, 	"p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p2 fire 1"},

	{"Tilt"         , BIT_DIGITAL  , DrvJoy1 + 7,	"tilt"     },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip 1"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip 2"	   },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0xdf, NULL                     },

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x10, 0x01, 0x03, 0x03, "Easy"     		  },
	{0x10, 0x01, 0x03, 0x02, "Medium"	          },
	{0x10, 0x01, 0x03, 0x01, "Hard"     		  },
	{0x10, 0x01, 0x03, 0x00, "Hardest"	          },

	{0   , 0xfe, 0   , 2   , "Rack Test (cheat)"      },
	{0x10, 0x01, 0x04, 0x04, "Off"       		  },
	{0x10, 0x01, 0x04, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 2   , "Special"                },
	{0x10, 0x01, 0x08, 0x08, "Easy"			  },
	{0x10, 0x01, 0x08, 0x00, "Hard"			  },

	{0   , 0xfe, 0   , 2   , "Extra"                  },
	{0x10, 0x01, 0x10, 0x10, "Easy"         	  },
	{0x10, 0x01, 0x10, 0x00, "Hard"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet" 	          },
	{0x10, 0x01, 0x20, 0x00, "Upright"		  },
	{0x10, 0x01, 0x20, 0x20, "Cocktail"    		  },

	{0   , 0xfe, 0   , 4   , "Lives" 	          },
	{0x10, 0x01, 0xc0, 0x00, "2"	        	  },
	{0x10, 0x01, 0xc0, 0xc0, "3"    		  },
	{0x10, 0x01, 0xc0, 0x80, "4"    		  },
	{0x10, 0x01, 0xc0, 0x40, "5"    		  },

	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 11  , "Coin B" 	          },
	{0x11, 0x01, 0x0f, 0x06, "4C_1C"                  },
	{0x11, 0x01, 0x0f, 0x08, "3C_1C"                  },
	{0x11, 0x01, 0x0f, 0x0a, "2C_1C"                  },
	{0x11, 0x01, 0x0f, 0x07, "3C_2C"                  },
	{0x11, 0x01, 0x0f, 0x0f, "1C_1C"                  },
	{0x11, 0x01, 0x0f, 0x09, "2C_3C"                  },
	{0x11, 0x01, 0x0f, 0x0e, "1C_2C"                  },
	{0x11, 0x01, 0x0f, 0x0d, "1C_3C"                  },
	{0x11, 0x01, 0x0f, 0x0c, "1C_4C"                  },
	{0x11, 0x01, 0x0f, 0x0b, "1C_5C"                  },
	{0x11, 0x01, 0x0f, 0x00, "Free_Play"              },

	{0   , 0xfe, 0   , 11  , "Coin A" 	          },
	{0x11, 0x01, 0xf0, 0x60, "4C_1C"                  },
	{0x11, 0x01, 0xf0, 0x80, "3C_1C"                  },
	{0x11, 0x01, 0xf0, 0xa0, "2C_1C"                  },
	{0x11, 0x01, 0xf0, 0x70, "3C_2C"                  },
	{0x11, 0x01, 0xf0, 0xf0, "1C_1C"                  },
	{0x11, 0x01, 0xf0, 0x90, "2C_3C"                  },
	{0x11, 0x01, 0xf0, 0xe0, "1C_2C"                  },
	{0x11, 0x01, 0xf0, 0xd0, "1C_3C"                  },
	{0x11, 0x01, 0xf0, 0xc0, "1C_4C"                  },
	{0x11, 0x01, 0xf0, 0xb0, "1C_5C"                  },
	{0x11, 0x01, 0xf0, 0x00, "Free_Play"              },
};

STDDIPINFO(Drv)

void __fastcall mrdo_write(UINT16 address, UINT8 data)
{
	if ((address & 0xf000) == 0xf000) address &= 0xf800;

	switch (address)
	{
		case 0x9800:
			flipscreen = data & 1;
		break;

		case 0x9801: 
			SN76496Write(0, data);
		
		case 0x9802:		
			SN76496Write(1, data);
		break;

		case 0xf000:
			scroll_x = data;
		break;

		case 0xf800:
			scroll_y = data ^ (flipscreen ? 0xff : 0);
		break;
	}
}

UINT8 __fastcall mrdo_read(UINT16 address)
{
	UINT8 ret = 0xff;

	switch (address)
	{
		case 0x9803: // Protection
			return Rom[ZetHL(-1)];

		case 0xa000:
		{
			for (INT32 i = 0; i < 8; i++) ret ^= DrvJoy1[i] << i;
			return ret;
		}

		case 0xa001:
		{
			for (INT32 i = 0; i < 8; i++) ret ^= DrvJoy2[i] << i;
			return ret;
		}

		case 0xa002:
			return DrvDips[0];

		case 0xa003:
			return DrvDips[1];
	}

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (Rom + 0x8000, 0, 0x8000);

	flipscreen = 0;
	scroll_x = scroll_y = 0;

	ZetOpen(0);
	ZetReset();
	ZetClose();

	return 0;
}

static void mrdo_palette_init()
{
	INT32 weight[16];

	for (INT32 i = 0x0f; i >= 0; i--)
	{
		float par = 0, pot = 0;

		if (i & 1) par += 1.0/150;
		if (i & 2) par += 1.0/120;
		if (i & 4) par += 1.0/100;
		if (i & 8) par += 1.0/75;
		if (par)
		{
			par = 1 / par;
			pot = 200 / (200 + par) - 0.2f;
		}
		else pot = 0;

		weight[i] = (INT32)(0xff * pot / 0.684615);
	}

	for (INT32 i = 0; i < 0x100; i++)
	{
		INT32 a1,a2;
		INT32 bits0, bits2;
		INT32 r, g, b;

		a1 = ((i >> 3) & 0x1c) + (i & 0x03) + 32;
		a2 = ((i >> 0) & 0x1c) + (i & 0x03);

		bits0 = (Prom[a1] >> 0) & 0x03;
		bits2 = (Prom[a2] >> 0) & 0x03;
		r = weight[bits0 + (bits2 << 2)];

		bits0 = (Prom[a1] >> 2) & 0x03;
		bits2 = (Prom[a2] >> 2) & 0x03;
		g = weight[bits0 + (bits2 << 2)];

		bits0 = (Prom[a1] >> 4) & 0x03;
		bits2 = (Prom[a2] >> 4) & 0x03;
		b = weight[bits0 + (bits2 << 2)];

		Palette[i] = (r << 16) | (g << 8) | b;
	}

	for (INT32 i = 0; i < 0x40; i++)
	{
		UINT8 ctbl = Prom[0x40 + (i & 0x1f)] >> ((i & 0x20) >> 3);

		Palette[0x100 + i] = Palette[ctbl & 0x0f];
	}
}

static void mrdo_gfx_decode()
{
	static INT32 CharPlane[2]  = { 0, 0x8000 };
	static INT32 CharXOffs[8]  = { 7, 6, 5, 4, 3, 2, 1, 0 };
	static INT32 CharYOffs[8]  = { 0, 8, 16, 24, 32, 40, 48, 56 };

	static INT32 SpriPlane[2]  = { 4, 0 };
	static INT32 SpriXOffs[16] = { 3, 2, 1, 0, 11, 10, 9, 8, 19, 18, 17, 16, 27, 26, 25, 24 };
	static INT32 SpriYOffs[16] = { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x2000);
	if (!tmp) return;

	memcpy (tmp, Gfx0, 0x2000);

	GfxDecode(0x200, 2,  8,  8, CharPlane, CharXOffs, CharYOffs, 0x040, tmp, Gfx0);

	memcpy (tmp, Gfx1, 0x2000);

	GfxDecode(0x200, 2,  8,  8, CharPlane, CharXOffs, CharYOffs, 0x040, tmp, Gfx1);

	memcpy (tmp, Gfx2, 0x2000);

	GfxDecode(0x080, 2, 16, 16, SpriPlane, SpriXOffs, SpriYOffs, 0x200, tmp, Gfx2);

	BurnFree (tmp);
}

static INT32 DrvInit()
{
	Mem = (UINT8*)BurnMalloc(0x10000 + 0x8000 + 0x8000 + 0x8000 + 0x80 + 0x500);
	if (!Mem) return 1;

	Rom  = Mem + 0x00000;
	Gfx0 = Mem + 0x10000;
	Gfx1 = Mem + 0x18000;
	Gfx2 = Mem + 0x20000;
	Prom = Mem + 0x28000;
	Palette = (INT32*)(Mem + 0x28080);

	{
		for (INT32 i = 0; i < 4; i++) {
			if(BurnLoadRom(Rom  + i * 0x2000,  0 + i, 1)) return 1;
			if(BurnLoadRom(Prom + i * 0x0020, 10 + i, 1)) return 1;
		}

		for (INT32 i = 0; i < 2; i++) {
			if(BurnLoadRom(Gfx0 + i * 0x1000,  4 + i, 1)) return 1;
			if(BurnLoadRom(Gfx1 + i * 0x1000,  6 + i, 1)) return 1;
			if(BurnLoadRom(Gfx2 + i * 0x1000,  8 + i, 1)) return 1;
		}

		mrdo_palette_init();
		mrdo_gfx_decode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(mrdo_read);
	ZetSetWriteHandler(mrdo_write);
	ZetMapArea(0x0000, 0x7fff, 0, Rom + 0x0000);
	ZetMapArea(0x0000, 0x7fff, 2, Rom + 0x0000);
	ZetMapArea(0x8000, 0x8fff, 0, Rom + 0x8000);
	ZetMapArea(0x8000, 0x8fff, 1, Rom + 0x8000);
	ZetMapArea(0x9000, 0x90ff, 1, Rom + 0x9000);
	ZetMapArea(0xe000, 0xefff, 0, Rom + 0xe000);
	ZetMapArea(0xe000, 0xefff, 1, Rom + 0xe000);
	ZetMapArea(0xe000, 0xefff, 2, Rom + 0xe000);
	ZetMemEnd();
	ZetClose();

	BurnSetRefreshRate(5000000.0/312/262);

	SN76489Init(0, 4000000, 0);
	SN76489Init(1, 4000000, 1);
	SN76496SetRoute(0, 0.50, BURN_SND_ROUTE_BOTH);
	SN76496SetRoute(1, 0.50, BURN_SND_ROUTE_BOTH);
 
	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	ZetExit();
	
	SN76496Exit();

	BurnFree (Mem);

	Mem = Rom = Gfx0 = Gfx1 = Gfx2 = Prom = NULL;
	Palette = NULL;

	flipscreen = scroll_x = scroll_y = 0;

	return 0;
}

static void draw_sprites()
{
	for (INT32 offs = 0x100 - 4; offs >= 0; offs -= 4)
	{
		if (Rom[0x9001 + offs])
		{
			INT32 sx = Rom[0x9003 + offs];
			INT32 sy = Rom[0x9001 + offs] ^ 0xff;

			INT32 code = Rom[0x9000 + offs] & 0x7f;
			INT32 color = ((Rom[0x9002 + offs] & 0x0f) << 2) | 0x100;

			INT32 flipx = Rom[0x9002 + offs] & 0x10;
			INT32 flipy = Rom[0x9002 + offs] & 0x20;

			UINT8 *src = Gfx2 + (code << 8);

			sx -= 8;
			sy -= 32;

			if (flipy)
			{
				for (INT32 y = sy + 15; y >= sy; y--)
				{
					if (flipx)
					{
						for (INT32 x = sx + 15; x >= sx; x--, src++)
						{
							if (y < 0 || x < 0 || y > 191 || x > 239) continue;
							if (!*src) continue;

							INT32 pxl = Palette[color | *src];

							PutPix(pBurnDraw + ((y * 240) + x) * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
						}
					} else {
						for (INT32 x = sx; x < sx + 16; x++, src++)
						{
							if (y < 0 || x < 0 || y > 191 || x > 239) continue;
							if (!*src) continue;

							INT32 pxl = Palette[color | *src];

							PutPix(pBurnDraw + ((y * 240) + x) * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
						}
					}
				}
			} else {
				for (INT32 y = sy; y < sy + 16; y++)
				{
					if (flipx)
					{
						for (INT32 x = sx + 15; x >= sx; x--, src++)
						{
							if (y < 0 || x < 0 || y > 191 || x > 239) continue;
							if (!*src) continue;

							INT32 pxl = Palette[color | *src];

							PutPix(pBurnDraw + ((y * 240) + x) * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
						}
					} else {
						for (INT32 x = sx; x < sx + 16; x++, src++)
						{
							if (y < 0 || x < 0 || y > 191 || x > 239) continue;
							if (!*src) continue;

							INT32 pxl = Palette[color | *src];

							PutPix(pBurnDraw + ((y * 240) + x) * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
						}
					}
				}
			}
		}
	}
}

static void draw_8x8_tiles(UINT8 *vram, UINT8 *gfx_base, INT32 scrollx, INT32 scrolly)
{
	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 2) & 0xf8; 

		INT32 code = vram[0x400 + offs] | ((vram[offs] & 0x80) << 1);
		INT32 color = (vram[offs] & 0x3f) << 2;
		INT32 forcelayer0 = vram[offs] & 0x40;

		UINT8 *src = gfx_base + (code << 6);

		sx = (UINT8)(sx - scrollx);
		sy = (UINT8)(sy - scrolly);

		sx -= 8;
		sy -= 32;

		for (INT32 y = sy; y < sy + 8; y++)
		{
			for (INT32 x = sx; x < sx + 8; x++, src++)
			{
				if (x < 0 || y < 0 || x > 239 || y > 191) continue;

				INT32 pxl = Palette[color | *src];
				if (!*src && !forcelayer0) continue;

				INT32 pos = y * 240 + x;
				if (flipscreen) pos = (192 - y) * 240 + (240 - x);

				PutPix(pBurnDraw + pos * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
			}
		}
	}
}

static INT32 DrvDraw()
{
	memset (pBurnDraw, 0, 240 * 191 * nBurnBpp);

	draw_8x8_tiles(Rom + 0x8000, Gfx1, scroll_x, scroll_y);
	draw_8x8_tiles(Rom + 0x8800, Gfx0, 0, 0);
	draw_sprites();

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetOpen(0);
	ZetRun(4000000 / 60);
	ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}
	
	if (pBurnSoundOut) {
		SN76496Update(0, pBurnSoundOut, nBurnSoundLen);
		SN76496Update(1, pBurnSoundOut, nBurnSoundLen);
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

		ba.Data	  = Rom + 0x8000;
		ba.nLen	  = 0x8000;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);

		SCAN_VAR(flipscreen);
		SCAN_VAR(scroll_x);
		SCAN_VAR(scroll_y);
	}

	return 0;
}


// Mr. Do!

static struct BurnRomInfo mrdoRomDesc[] = {
	{ "a4-01.bin",    0x2000, 0x03dcfba2, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "c4-02.bin",    0x2000, 0x0ecdd39c, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "e4-03.bin",    0x2000, 0x358f5dc2, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "f4-04.bin",    0x2000, 0xf4190cfc, 1 | BRF_ESS | BRF_PRG }, //  3

	{ "s8-09.bin",    0x1000, 0xaa80c5b6, 2 | BRF_GRA },	       //  4 FG Tiles
	{ "u8-10.bin",    0x1000, 0xd20ec85b, 2 | BRF_GRA },	       //  5

	{ "r8-08.bin",    0x1000, 0xdbdc9ffa, 3 | BRF_GRA },	       //  6 BG Tiles
	{ "n8-07.bin",    0x1000, 0x4b9973db, 3 | BRF_GRA },	       //  7

	{ "h5-05.bin",    0x1000, 0xe1218cc5, 4 | BRF_GRA },	       //  8 Sprite Tiles
	{ "k5-06.bin",    0x1000, 0xb1f68b04, 4 | BRF_GRA },	       //  9

	{ "u02--2.bin",   0x0020, 0x238a65d7, 5 | BRF_GRA },	       // 10 Palette (high bits)
	{ "t02--3.bin",   0x0020, 0xae263dc0, 5 | BRF_GRA },	       // 11 Palette (low bits)
	{ "f10--1.bin",   0x0020, 0x16ee4ca2, 5 | BRF_GRA },	       // 12 Sprite color lookup table
	{ "j10--4.bin",   0x0020, 0xff7fe284, 5 | BRF_GRA },	       // 13 Timing (not used)
};

STD_ROM_PICK(mrdo)
STD_ROM_FN(mrdo)

struct BurnDriver BurnDrvmrdo = {
	"mrdo", NULL, NULL, NULL, "1982",
	"Mr. Do!\0", NULL, "Universal", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, mrdoRomInfo, mrdoRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x140,
	192, 240, 3, 4
};


// Mr. Do! (Taito license)

static struct BurnRomInfo mrdotRomDesc[] = {
	{ "d1",           0x2000, 0x3dcd9359, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "d2",           0x2000, 0x710058d8, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "d3",           0x2000, 0x467d12d8, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "d4",           0x2000, 0xfce9afeb, 1 | BRF_ESS | BRF_PRG }, //  3

	{ "d9",           0x1000, 0xde4cfe66, 2 | BRF_GRA },	       //  4 FG Tiles
	{ "d10",          0x1000, 0xa6c2f38b, 2 | BRF_GRA },	       //  5

	{ "r8-08.bin",    0x1000, 0xdbdc9ffa, 3 | BRF_GRA },	       //  6 BG Tiles
	{ "n8-07.bin",    0x1000, 0x4b9973db, 3 | BRF_GRA },	       //  7

	{ "h5-05.bin",    0x1000, 0xe1218cc5, 4 | BRF_GRA },	       //  8 Sprite Tiles
	{ "k5-06.bin",    0x1000, 0xb1f68b04, 4 | BRF_GRA },	       //  9

	{ "u02--2.bin",   0x0020, 0x238a65d7, 5 | BRF_GRA },	       // 10 Palette (high bits)
	{ "t02--3.bin",   0x0020, 0xae263dc0, 5 | BRF_GRA },	       // 11 Palette (low bits)
	{ "f10--1.bin",   0x0020, 0x16ee4ca2, 5 | BRF_GRA },	       // 12 Sprite color lookup table
	{ "j10--4.bin",   0x0020, 0xff7fe284, 5 | BRF_GRA },	       // 13 Timing (not used)
};

STD_ROM_PICK(mrdot)
STD_ROM_FN(mrdot)

struct BurnDriver BurnDrvmrdot = {
	"mrdot", "mrdo", NULL, NULL, "1982",
	"Mr. Do! (Taito license)\0", NULL, "Universal (Taito license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, mrdotRomInfo, mrdotRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x140,
	192, 240, 3, 4
};

// Mr. Do! (bugfixed)

static struct BurnRomInfo mrdofixRomDesc[] = {
	{ "d1",           0x2000, 0x3dcd9359, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "d2",           0x2000, 0x710058d8, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "dofix.d3",     0x2000, 0x3a7d039b, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "dofix.d4",     0x2000, 0x32db845f, 1 | BRF_ESS | BRF_PRG }, //  3

	{ "d9",           0x1000, 0xde4cfe66, 2 | BRF_GRA },	       //  4 FG Tiles
	{ "d10",          0x1000, 0xa6c2f38b, 2 | BRF_GRA },	       //  5

	{ "r8-08.bin",    0x1000, 0xdbdc9ffa, 3 | BRF_GRA },	       //  6 BG Tiles
	{ "n8-07.bin",    0x1000, 0x4b9973db, 3 | BRF_GRA },	       //  7

	{ "h5-05.bin",    0x1000, 0xe1218cc5, 4 | BRF_GRA },	       //  8 Sprite Tiles
	{ "k5-06.bin",    0x1000, 0xb1f68b04, 4 | BRF_GRA },	       //  9

	{ "u02--2.bin",   0x0020, 0x238a65d7, 5 | BRF_GRA },	       // 10 Palette (high bits)
	{ "t02--3.bin",   0x0020, 0xae263dc0, 5 | BRF_GRA },	       // 11 Palette (low bits)
	{ "f10--1.bin",   0x0020, 0x16ee4ca2, 5 | BRF_GRA },	       // 12 Sprite color lookup table
	{ "j10--4.bin",   0x0020, 0xff7fe284, 5 | BRF_GRA },	       // 13 Timing (not used)
};

STD_ROM_PICK(mrdofix)
STD_ROM_FN(mrdofix)

struct BurnDriver BurnDrvmrdofix = {
	"mrdofix", "mrdo", NULL, NULL, "1982",
	"Mr. Do! (bugfixed)\0", NULL, "Universal (Taito license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, mrdofixRomInfo, mrdofixRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x140,
	192, 240, 3, 4
};


// Mr. Lo!

static struct BurnRomInfo mrloRomDesc[] = {
	{ "mrlo01.bin",   0x2000, 0x6f455e7d, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "d2",           0x2000, 0x710058d8, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "dofix.d3",     0x2000, 0x3a7d039b, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "mrlo04.bin",   0x2000, 0x49c10274, 1 | BRF_ESS | BRF_PRG }, //  3

	{ "mrlo09.bin",   0x1000, 0xfdb60d0d, 2 | BRF_GRA },	       //  4 FG Tiles
	{ "mrlo10.bin",   0x1000, 0x0492c10e, 2 | BRF_GRA },	       //  5

	{ "r8-08.bin",    0x1000, 0xdbdc9ffa, 3 | BRF_GRA },	       //  6 BG Tiles
	{ "n8-07.bin",    0x1000, 0x4b9973db, 3 | BRF_GRA },	       //  7

	{ "h5-05.bin",    0x1000, 0xe1218cc5, 4 | BRF_GRA },	       //  8 Sprite Tiles
	{ "k5-06.bin",    0x1000, 0xb1f68b04, 4 | BRF_GRA },	       //  9

	{ "u02--2.bin",   0x0020, 0x238a65d7, 5 | BRF_GRA },	       // 10 Palette (high bits)
	{ "t02--3.bin",   0x0020, 0xae263dc0, 5 | BRF_GRA },	       // 11 Palette (low bits)
	{ "f10--1.bin",   0x0020, 0x16ee4ca2, 5 | BRF_GRA },	       // 12 Sprite color lookup table
	{ "j10--4.bin",   0x0020, 0xff7fe284, 5 | BRF_GRA },	       // 13 Timing (not used)
};

STD_ROM_PICK(mrlo)
STD_ROM_FN(mrlo)

struct BurnDriver BurnDrvmrlo = {
	"mrlo", "mrdo", NULL, NULL, "1982",
	"Mr. Lo!\0", NULL, "Bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, mrloRomInfo, mrloRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x140,
	192, 240, 3, 4
};


// Mr. Du!

static struct BurnRomInfo mrduRomDesc[] = {
	{ "d1",           0x2000, 0x3dcd9359, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "d2",           0x2000, 0x710058d8, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "d3",           0x2000, 0x467d12d8, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "du4.bin",      0x2000, 0x893bc218, 1 | BRF_ESS | BRF_PRG }, //  3

	{ "du9.bin",      0x1000, 0x4090dcdc, 2 | BRF_GRA },	       //  4 FG Tiles
	{ "du10.bin",     0x1000, 0x1e63ab69, 2 | BRF_GRA },	       //  5

	{ "r8-08.bin",    0x1000, 0xdbdc9ffa, 3 | BRF_GRA },	       //  6 BG Tiles
	{ "n8-07.bin",    0x1000, 0x4b9973db, 3 | BRF_GRA },	       //  7

	{ "h5-05.bin",    0x1000, 0xe1218cc5, 4 | BRF_GRA },	       //  8 Sprite Tiles
	{ "k5-06.bin",    0x1000, 0xb1f68b04, 4 | BRF_GRA },	       //  9

	{ "u02--2.bin",   0x0020, 0x238a65d7, 5 | BRF_GRA },	       // 10 Palette (high bits)
	{ "t02--3.bin",   0x0020, 0xae263dc0, 5 | BRF_GRA },	       // 11 Palette (low bits)
	{ "f10--1.bin",   0x0020, 0x16ee4ca2, 5 | BRF_GRA },	       // 12 Sprite color lookup table
	{ "j10--4.bin",   0x0020, 0xff7fe284, 5 | BRF_GRA },	       // 13 Timing (not used)
};

STD_ROM_PICK(mrdu)
STD_ROM_FN(mrdu)

struct BurnDriver BurnDrvmrdu = {
	"mrdu", "mrdo", NULL, NULL, "1982",
	"Mr. Du!\0", NULL, "Bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, mrduRomInfo, mrduRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x140,
	192, 240, 3, 4
};


// Mr. Do! (prototype)

static struct BurnRomInfo mrdoyRomDesc[] = {
	{ "dosnow.1",     0x2000, 0xd3454e2c, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "dosnow.2",     0x2000, 0x5120a6b2, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "dosnow.3",     0x2000, 0x96416dbe, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "dosnow.4",     0x2000, 0xc05051b6, 1 | BRF_ESS | BRF_PRG }, //  3

	{ "dosnow.9",     0x1000, 0x85d16217, 2 | BRF_GRA },	       //  4 FG Tiles
	{ "dosnow.10",    0x1000, 0x61a7f54b, 2 | BRF_GRA },	       //  5

	{ "dosnow.8",     0x1000, 0x2bd1239a, 3 | BRF_GRA },	       //  6 BG Tiles
	{ "dosnow.7",     0x1000, 0xac8ffddf, 3 | BRF_GRA },	       //  7

	{ "dosnow.5",     0x1000, 0x7662d828, 4 | BRF_GRA },	       //  8 Sprite Tiles
	{ "dosnow.6",     0x1000, 0x413f88d1, 4 | BRF_GRA },	       //  9

	{ "u02--2.bin",   0x0020, 0x238a65d7, 5 | BRF_GRA },	       // 10 Palette (high bits)
	{ "t02--3.bin",   0x0020, 0xae263dc0, 5 | BRF_GRA },	       // 11 Palette (low bits)
	{ "f10--1.bin",   0x0020, 0x16ee4ca2, 5 | BRF_GRA },	       // 12 Sprite color lookup table
	{ "j10--4.bin",   0x0020, 0xff7fe284, 5 | BRF_GRA },	       // 13 Timing (not used)
};

STD_ROM_PICK(mrdoy)
STD_ROM_FN(mrdoy)

struct BurnDriver BurnDrvmrdoy = {
	"mrdoy", "mrdo", NULL, NULL, "1982",
	"Mr. Do! (prototype)\0", NULL, "Universal", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_PROTOTYPE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, mrdoyRomInfo, mrdoyRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x140,
	192, 240, 3, 4
};


// Yankee DO!

static struct BurnRomInfo yankeedoRomDesc[] = {
	{ "a4-01.bin",    0x2000, 0x03dcfba2, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 Code
	{ "yd_d2.c4",     0x2000, 0x7c9d7ce0, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "e4-03.bin",    0x2000, 0x358f5dc2, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "f4-04.bin",    0x2000, 0xf4190cfc, 1 | BRF_ESS | BRF_PRG }, //  3

	{ "s8-09.bin",    0x1000, 0xaa80c5b6, 2 | BRF_GRA },	       //  4 FG Tiles
	{ "u8-10.bin",    0x1000, 0xd20ec85b, 2 | BRF_GRA },	       //  5

	{ "r8-08.bin",    0x1000, 0xdbdc9ffa, 3 | BRF_GRA },	       //  6 BG Tiles
	{ "n8-07.bin",    0x1000, 0x4b9973db, 3 | BRF_GRA },	       //  7

	{ "yd_d5.h5",     0x1000, 0xf530b79b, 4 | BRF_GRA },	       //  8 Sprite Tiles
	{ "yd_d6.k5",     0x1000, 0x790579aa, 4 | BRF_GRA },	       //  9

	{ "u02--2.bin",   0x0020, 0x238a65d7, 5 | BRF_GRA },	       // 10 Palette (high bits)
	{ "t02--3.bin",   0x0020, 0xae263dc0, 5 | BRF_GRA },	       // 11 Palette (low bits)
	{ "f10--1.bin",   0x0020, 0x16ee4ca2, 5 | BRF_GRA },	       // 12 Sprite color lookup table
	{ "j10--4.bin",   0x0020, 0xff7fe284, 5 | BRF_GRA },	       // 13 Timing (not used)
};

STD_ROM_PICK(yankeedo)
STD_ROM_FN(yankeedo)

struct BurnDriver BurnDrvyankeedo = {
	"yankeedo", "mrdo", NULL, NULL, "1982",
	"Yankee DO!\0", NULL, "hack", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, yankeedoRomInfo, yankeedoRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x140,
	192, 240, 3, 4
};

