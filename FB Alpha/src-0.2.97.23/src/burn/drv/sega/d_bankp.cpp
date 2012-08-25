// FB Alpha Bank Panic Driver Module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "z80_intf.h"
#include "sn76496.h"

static UINT8 *Mem, *Rom, *Gfx0, *Gfx1, *Prom;
static UINT8 DrvJoy1[8], DrvJoy2[8], DrvJoy3[8], DrvReset, DrvDips;
static INT32 *Palette;

static UINT8 scroll_x, priority, flipscreen, interrupt_enable;

static struct BurnInputInfo bankpInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy1 + 5,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy1 + 6,	"p2 coin"  },
	{"Coin 3"       , BIT_DIGITAL  , DrvJoy3 + 2,	"p3 coin"  },
	{"P1 Start"     , BIT_DIGITAL  , DrvJoy2 + 5,	"p1 start" },
	{"P2 Start"     , BIT_DIGITAL  , DrvJoy2 + 6,	"p2 start" },

	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 3, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 1, 	"p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 7,	"p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 0,	"p1 fire 3"},

	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 3, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 1, 	"p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 7,	"p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 1,	"p2 fire 3"},

	{"Reset"        , BIT_DIGITAL  , &DrvReset  ,	"reset"    },
	{"Dip 1"        , BIT_DIPSWITCH, &DrvDips,      "dip"      },
};

STDINPUTINFO(bankp)

static struct BurnInputInfo combhInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy1 + 5,	"p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy1 + 6,	"p2 coin"  },
	{"Coin 3"       , BIT_DIGITAL  , DrvJoy3 + 2,	"p3 coin"  },
	{"P1 Start"     , BIT_DIGITAL  , DrvJoy2 + 5,	"p1 start" },
	{"P2 Start"     , BIT_DIGITAL  , DrvJoy2 + 6,	"p2 start" },

	{"P1 Up"        , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 2, 	"p1 down"  },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 7,	"p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 0,	"p1 fire 3"},

	{"P2 Up"        , BIT_DIGITAL  , DrvJoy2 + 0, 	"p2 up"    },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy2 + 2, 	"p2 down"  },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 7,	"p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy3 + 1,	"p2 fire 3"},

	{"Reset"        , BIT_DIGITAL  , &DrvReset  ,	"reset"    },
	{"Dip 1"        , BIT_DIPSWITCH, &DrvDips,      "dip"      },

};

STDINPUTINFO(combh)

static struct BurnDIPInfo bankpDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0xc0, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coin A/B"               },
	{0x10, 0x01, 0x03, 0x03, "3C 1C"     		  },
	{0x10, 0x01, 0x03, 0x02, "2C 1C"    		  },
	{0x10, 0x01, 0x03, 0x00, "1C 1C"     		  },
	{0x10, 0x01, 0x03, 0x01, "1C 2C"    		  },

	{0   , 0xfe, 0   , 2   , "Coin C"                 },
	{0x10, 0x01, 0x04, 0x04, "2C 1C"     		  },
	{0x10, 0x01, 0x04, 0x00, "1C 1C"    		  },

	{0   , 0xfe, 0   , 2   , "Lives"	          },
	{0x10, 0x01, 0x08, 0x00, "3"     		  },
	{0x10, 0x01, 0x08, 0x08, "4"			  },

	{0   , 0xfe, 0   , 2   , "Bonus Life"	          },
	{0x10, 0x01, 0x10, 0x00, "70K 200K 500K..."       },
	{0x10, 0x01, 0x10, 0x10, "100K 400K 800K..."	  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x10, 0x01, 0x20, 0x00, "Easy"       		  },
	{0x10, 0x01, 0x20, 0x20, "Hard"       		  },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x10, 0x01, 0x40, 0x00, "Off"     		  },
	{0x10, 0x01, 0x40, 0x40, "On"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x80, 0x80, "Upright"     		  },
	{0x10, 0x01, 0x80, 0x00, "Cocktail"    		  },
};

STDDIPINFO(bankp)

static struct BurnDIPInfo combhDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x10, NULL                     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x10, 0x01, 0x01, 0x00, "Off"     		  },
	{0x10, 0x01, 0x01, 0x01, "On"    		  },

	{0   , 0xfe, 0   , 4   , "Coinage"                },
	{0x10, 0x01, 0x06, 0x06, "2C 1C"     		  },
	{0x10, 0x01, 0x06, 0x00, "1C 1C"    		  },
	{0x10, 0x01, 0x06, 0x02, "1C 2C"     		  },
	{0x10, 0x01, 0x06, 0x04, "1C 3C"    		  },

	{0   , 0xfe, 0   , 2   , "Lives"	          },
	{0x10, 0x01, 0x08, 0x00, "3"     		  },
	{0x10, 0x01, 0x08, 0x08, "4"			  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x10, 0x10, "Upright"     		  },
	{0x10, 0x01, 0x10, 0x00, "Cocktail"    		  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x10, 0x01, 0x40, 0x00, "Easy"       		  },
	{0x10, 0x01, 0x40, 0x40, "Hard"       		  },

	{0   , 0xfe, 0   , 2   , "Fuel"                   },
	{0x10, 0x01, 0x80, 0x00, "120 Units"     	  },
	{0x10, 0x01, 0x80, 0x80, "90 Units"    		  },
};

STDDIPINFO(combh)

UINT8 __fastcall bankp_in(UINT16 address)
{
	UINT8 ret = 0;

	switch (address & 0xff)
	{
		case 0x00:
		{
			for (INT32 i = 0; i < 8; i++) ret |= DrvJoy1[i] << i;

			// limit controls to 2-way
			if ((ret & 0x05) == 0x05) ret &= 0xfa;
			if ((ret & 0x0a) == 0x0a) ret &= 0xf5;

			return ret;
		}

		case 0x01:
		{
			for (INT32 i = 0; i < 8; i++) ret |= DrvJoy2[i] << i;

			// limit controls to 2-way
			if ((ret & 0x05) == 0x05) ret &= 0xfa;
			if ((ret & 0x0a) == 0x0a) ret &= 0xf5;

			return ret;
		}

		case 0x02:
		{
			for (INT32 i = 0; i < 8; i++) ret |= DrvJoy3[i] << i;

			return ret;
		}

		case 0x04:
			return DrvDips;
	}

	return 0;
}

void __fastcall bankp_out(UINT16 address, UINT8 data)
{
	switch (address & 0xff)
	{
		case 0x00: 
			SN76496Write(0, data);
		break;
		
		case 0x01:
			SN76496Write(1, data);
		break;
		
		case 0x02:
			SN76496Write(2, data);
		break;

		case 0x05:
			scroll_x = data;
		break;

		case 0x07:
		{
			priority = data & 0x03;

			interrupt_enable = (data >> 4) & 1;

			flipscreen = data & 0x20;
		}
		break;
	}
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (Mem + 0xe000, 0, 0x2000);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	scroll_x = 0, priority = 0, flipscreen = 0;
	interrupt_enable = 0;

	return 0;
}

static INT32 bankp_palette_init()
{
	INT32 i;

	UINT32 t_pal[32];
	UINT8 *color_prom = Prom;

	for (i = 0;i < 32;i++)
	{
		INT32 bit0,bit1,bit2,r,g,b;

		bit0 = (*color_prom >> 0) & 0x01;
		bit1 = (*color_prom >> 1) & 0x01;
		bit2 = (*color_prom >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (*color_prom >> 3) & 0x01;
		bit1 = (*color_prom >> 4) & 0x01;
		bit2 = (*color_prom >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = (*color_prom >> 6) & 0x01;
		bit2 = (*color_prom >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		t_pal[i] = (r << 16) | (g << 8) | b;

		color_prom++;
	}

	for (i = 0;i < 32 * 4;i++) {
		Palette[i] = t_pal[*color_prom & 0x0f];
		color_prom++;
	}

	color_prom += 128;

	for (i = 0;i < 16 * 8;i++) {
		Palette[i + 0x80] = t_pal[*color_prom & 0x0f];
		color_prom++;
	}

	return 0;
}

static INT32 bankp_gfx_decode()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x10000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, Gfx0, 0x10000);

	static INT32 Char1PlaneOffsets[2] = { 0x00, 0x04 };
	static INT32 Char2PlaneOffsets[3] = { 0x00, 0x20000, 0x40000 };
	static INT32 Char1XOffsets[8]     = { 0x43, 0x42, 0x41, 0x40, 0x03, 0x02, 0x01, 0x00 };
	static INT32 Char2XOffsets[8]     = { 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00 };
	static INT32 CharYOffsets[8]      = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 };

	GfxDecode(0x400, 2, 8, 8, Char1PlaneOffsets, Char1XOffsets, CharYOffsets, 0x080, tmp, Gfx0);

	memcpy (tmp, Gfx1, 0x10000);

	GfxDecode(0x800, 3, 8, 8, Char2PlaneOffsets, Char2XOffsets, CharYOffsets, 0x040, tmp, Gfx1);

	for (INT32 i = 0; i < 0x20000; i++) {
		Gfx1[i] |= 0x80;
	}

	BurnFree (tmp);

	return 0;
}

static INT32 DrvInit()
{
	Mem = (UINT8*)BurnMalloc(0x10000 + 0x10000 + 0x20000 + 0x200 + (0x100 * sizeof(INT32)));
	if (Mem == NULL) {
		return 1;
	}

	Rom  = Mem + 0x00000;
	Gfx0 = Mem + 0x10000;
	Gfx1 = Mem + 0x20000;
	Prom = Mem + 0x40000;
	Palette = (INT32*)(Mem + 0x40200);

	{
		for (INT32 i = 0; i < 4; i++)
			if (BurnLoadRom(Rom + i * 0x4000, i +  0, 1)) return 1;

		if (BurnLoadRom(Gfx0 + 0x0000, 4, 1)) return 1;
		if (BurnLoadRom(Gfx0 + 0x2000, 5, 1)) return 1;

		for (INT32 i = 0; i < 6; i++)
			if (BurnLoadRom(Gfx1 + i * 0x2000, i +  6, 1)) return 1;

		if (BurnLoadRom(Prom + 0x0000, 12, 1)) return 1;
		if (BurnLoadRom(Prom + 0x0020, 13, 1)) return 1;
		if (BurnLoadRom(Prom + 0x0120, 14, 1)) return 1;

		if (bankp_gfx_decode()) return 1;
		bankp_palette_init();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetSetInHandler(bankp_in);
	ZetSetOutHandler(bankp_out);
	ZetMapArea(0x0000, 0xdfff, 0, Rom + 0x0000);
	ZetMapArea(0x0000, 0xdfff, 2, Rom + 0x0000);
	ZetMapArea(0xe000, 0xefff, 0, Rom + 0xe000);
	ZetMapArea(0xe000, 0xefff, 1, Rom + 0xe000);
	ZetMapArea(0xe000, 0xefff, 2, Rom + 0xe000);
	ZetMapArea(0xf000, 0xffff, 0, Rom + 0xf000);
	ZetMapArea(0xf000, 0xffff, 1, Rom + 0xf000);
	ZetMemEnd();
	ZetClose();
	
	SN76489Init(0, 15468000 / 6, 0);
	SN76489Init(1, 15468000 / 6, 1);
	SN76489Init(2, 15468000 / 6, 1);
	SN76496SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	SN76496SetRoute(1, 1.00, BURN_SND_ROUTE_BOTH);
	SN76496SetRoute(2, 1.00, BURN_SND_ROUTE_BOTH);

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	ZetExit();
	
	SN76496Exit();

	BurnFree (Mem);

	scroll_x = priority = flipscreen = interrupt_enable = 0;

	Rom = Gfx0 = Gfx1 = Prom = NULL;
	Palette = NULL;

	return 0;
}


static void bankp_plot_pixel(INT32 x, INT32 y, INT32 color, UINT8 src, INT32 transp)
{
	if (x > 223 || x < 0 || y > 223 || y < 0) return;

	INT32 pxl = Palette[color | src];
	if (transp && !pxl) return;

	PutPix(pBurnDraw + (y * 224 + x) * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
}


static void draw_8x8_tiles(UINT8 *gfx_base, INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 transp)
{
	UINT8 *src = gfx_base + (code << 6);

	if (flipy)
	{
		for (INT32 y = sy + 7; y >= sy; y--)
		{
			if (flipx)
			{
				for (INT32 x = sx + 7; x >= sx; x--, src++) {
					bankp_plot_pixel(x, y, color, *src, transp);
				}
			} else {
				for (INT32 x = sx; x < sx + 8; x++, src++) {
					bankp_plot_pixel(x, y, color, *src, transp);
				}
			}
		}	
	} else {
		for (INT32 y = sy; y < sy + 8; y++)
		{
			if (flipx)
			{
				for (INT32 x = sx + 7; x >= sx; x--, src++) {
					bankp_plot_pixel(x, y, color, *src, transp);

				}
			} else {
				for (INT32 x = sx; x < sx + 8; x++, src++) {
					bankp_plot_pixel(x, y, color, *src, transp);

				}
			}
		}
	}
}

static void draw_bg_tiles(INT32 prio)
{
	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 code, color, flipx, sx, sy;

		code = Rom[0xf800 + offs] | ((Rom[0xfc00 + offs] & 7) << 8);
		color = (Rom[0xfc00 + offs] >> 1) & 0x78;
		flipx = Rom[0xfc00 + offs] & 0x08;

		if (flipscreen) {
			sx = (~offs << 3) & 0xf8;
			sy = (~offs >> 2) & 0xf8;
			flipx ^= 0x08;
		} else {
			sx = ( offs << 3) & 0xf8;
			sy = ( offs >> 2) & 0xf8;
		}

		draw_8x8_tiles(Gfx1, code, color, sx - 24, sy - 16, flipx, flipscreen, prio);
	}
}

static void draw_fg_tiles(INT32 prio)
{
	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 code, color, flipx, sx, sy;

		code = Rom[0xf000 + offs] | ((Rom[0xf400 + offs] & 3) << 8);
		color = (Rom[0xf400 + offs] >> 1) & 0x7c;
		flipx = Rom[0xf400 + offs] & 0x04;

		if (flipscreen) {
			sx = ((~offs << 3) - scroll_x) & 0xff;
			sy = (~offs >> 2) & 0xf8;
			flipx ^= 4;
		} else {
			sx = (( offs << 3) - scroll_x) & 0xff;
			sy = ( offs >> 2) & 0xf8;
		}

		draw_8x8_tiles(Gfx0, code, color, sx - 24, sy - 16, flipx, flipscreen, prio);
	}
}

static INT32 DrvDraw()
{
	if (priority & 0x02)
	{
		draw_fg_tiles(0);
		draw_bg_tiles(1);
	} else {
		draw_bg_tiles(0);
		draw_fg_tiles(1);
	}

	return 0;
}


static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetOpen(0);
	ZetRun(2578000 / 60);
	if (interrupt_enable) ZetNmi();
	ZetClose();
	
	SN76496Update(0, pBurnSoundOut, nBurnSoundLen);
	SN76496Update(1, pBurnSoundOut, nBurnSoundLen);
	SN76496Update(2, pBurnSoundOut, nBurnSoundLen);

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

		ba.Data	  = Rom + 0xe000;
		ba.nLen	  = 0x2000;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		SN76496Scan(nAction, pnMin);

		SCAN_VAR(scroll_x);
		SCAN_VAR(priority);
		SCAN_VAR(flipscreen);
		SCAN_VAR(interrupt_enable);
	}

	return 0;
}

// Bank Panic

static struct BurnRomInfo bankpRomDesc[] = {
	{ "epr-6175.7e",       0x4000, 0x044552b8, 1 | BRF_ESS | BRF_PRG },    //  0 Z80 Code	
	{ "epr-6174.7f",       0x4000, 0xd29b1598, 1 | BRF_ESS | BRF_PRG },    //  1	
	{ "epr-6173.7h",       0x4000, 0xb8405d38, 1 | BRF_ESS | BRF_PRG },    //  2	
	{ "epr-6176.7d",       0x2000, 0xc98ac200, 1 | BRF_ESS | BRF_PRG },    //  3	

	{ "epr-6165.5l",       0x2000, 0xaef34a93, 2 | BRF_GRA },	       //  4 Playfield #1 chars
	{ "epr-6166.5k",       0x2000, 0xca13cb11, 2 | BRF_GRA },	       //  5

	{ "epr-6172.5b",       0x2000, 0xc4c4878b, 3 | BRF_GRA },	       //  6 Playfield #2 chars
	{ "epr-6171.5d",       0x2000, 0xa18165a1, 3 | BRF_GRA },	       //  7
	{ "epr-6170.5e",       0x2000, 0xb58aa8fa, 3 | BRF_GRA },	       //  8
	{ "epr-6169.5f",       0x2000, 0x1aa37fce, 3 | BRF_GRA },	       //  9
	{ "epr-6168.5h",       0x2000, 0x05f3a867, 3 | BRF_GRA },	       // 10
	{ "epr-6167.5i",       0x2000, 0x3fa337e1, 3 | BRF_GRA },	       // 11

	{ "pr-6177.8a",        0x0020, 0xeb70c5ae, 4 | BRF_GRA },	       // 12 (UINT32*)Palette
	{ "pr-6178.6f",        0x0100, 0x0acca001, 4 | BRF_GRA },	       // 13 Charset #1 lut
	{ "pr-6179.5a",        0x0100, 0xe53bafdb, 4 | BRF_GRA },	       // 14 Charset #2 lut

	{ "315-5074.2c.bin",   0x025b, 0x2e57bbba, 0 | BRF_OPT },	       // 15
	{ "315-5073.pal16l4",  0x0001, 0x00000000, 0 | BRF_OPT | BRF_NODUMP }, // 16 read protected
};

STD_ROM_PICK(bankp)
STD_ROM_FN(bankp)

struct BurnDriver BurnDrvbankp = {
	"bankp", NULL, NULL, NULL, "1984",
	"Bank Panic\0", NULL, "[Sanritsu] Sega", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 3, HARDWARE_SEGA_MISC, GBF_MISC, 0,
	NULL, bankpRomInfo, bankpRomName, NULL, NULL, bankpInputInfo, bankpDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x100,
	224, 224, 4, 3
};

// Combat Hawk

static struct BurnRomInfo combhRomDesc[] = {
	{ "epr-10904.7e",      0x4000, 0x4b106335, 1 | BRF_ESS | BRF_PRG },    //  0 Z80 Code	
	{ "epr-10905.7f",      0x4000, 0xa76fc390, 1 | BRF_ESS | BRF_PRG },    //  1	
	{ "epr-10906.7h",      0x4000, 0x16d54885, 1 | BRF_ESS | BRF_PRG },    //  2	
	{ "epr-10903.7d",      0x2000, 0xb7a59cab, 1 | BRF_ESS | BRF_PRG },    //  3	

	{ "epr-10914.5l",      0x2000, 0x7d7a2340, 2 | BRF_GRA },	       //  4 Playfield #1 chars
	{ "epr-10913.5k",      0x2000, 0xd5c1a8ae, 2 | BRF_GRA },	       //  5

	{ "epr-10907.5b",      0x2000, 0x08e5eea3, 3 | BRF_GRA },	       //  6 Playfield #2 chars
	{ "epr-10908.5d",      0x2000, 0xd9e413f5, 3 | BRF_GRA },	       //  7
	{ "epr-10909.5e",      0x2000, 0xfec7962c, 3 | BRF_GRA },	       //  8
	{ "epr-10910.5f",      0x2000, 0x33db0fa7, 3 | BRF_GRA },	       //  9
	{ "epr-10911.5h",      0x2000, 0x565d9e6d, 3 | BRF_GRA },	       // 10
	{ "epr-10912.5i",      0x2000, 0xcbe22738, 3 | BRF_GRA },	       // 11

	{ "pr-10900.8a",       0x0020, 0xf95fcd66, 4 | BRF_GRA },	       // 12 (UINT32*)Palette
	{ "pr-10901.6f",       0x0100, 0x6fd981c8, 4 | BRF_GRA },	       // 13 Charset #1 lut
	{ "pr-10902.5a",       0x0100, 0x84d6bded, 4 | BRF_GRA },	       // 14 Charset #2 lut

	{ "315-5074.2c.bin",   0x025b, 0x2e57bbba, 0 | BRF_OPT },	       // 15
	{ "315-5073.pal16l4",  0x0001, 0x00000000, 0 | BRF_OPT | BRF_NODUMP }, // 16 read protected
};

STD_ROM_PICK(combh)
STD_ROM_FN(combh)

struct BurnDriver BurnDrvcombh = {
	"combh", NULL, NULL, NULL, "1987",
	"Combat Hawk\0", NULL, "Sega / Sanritsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 3, HARDWARE_SEGA_MISC, GBF_MISC, 0,
	NULL, combhRomInfo, combhRomName, NULL, NULL, combhInputInfo, combhDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x100,
	224, 224, 3, 4
};

