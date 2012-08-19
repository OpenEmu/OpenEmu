// FB Alpha Funky Bee Driver Module
// Based on MAME driver by Zsolt Vasvari

#include "tiles_generic.h"
#include "zet.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvColPROM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvColRAM;

static UINT32 *DrvPalette;
static UINT8  DrvRecalc;

static INT16 *pAY8910Buffer[3];

static UINT8 *scroll;
static UINT8 *flipscreen;
static UINT8 *gfx_bank;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvInputs[3];
static UINT8 DrvDips[2];
static UINT8 DrvReset;

static INT32 watchdog;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy1 + 0,	"p1 coin"  },
	{"P1 start"  ,    BIT_DIGITAL  , DrvJoy1 + 3,	"p1 start" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy2 + 0, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy2 + 1, 	"p1 right" },
	{"P1 Up",	  BIT_DIGITAL,   DrvJoy2 + 2,   "p1 up",   },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy2 + 3,   "p1 down", },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p1 fire 1"},

	{"P2 Coin"      , BIT_DIGITAL  , DrvJoy1 + 1,	"p2 coin"  },
	{"P2 start"  ,    BIT_DIGITAL  , DrvJoy1 + 4,	"p2 start" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 0, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 1, 	"p2 right" },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy3 + 2,   "p2 up",   },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy3 + 3,   "p2 down", },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy3 + 4,	"p2 fire 1"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip"	   },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo funkybeeDIPList[]=
{
	{0x0f, 0xff, 0xff, 0x20, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0f, 0x01, 0x20, 0x20, "Off"       		  },
	{0x0f, 0x01, 0x20, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x10, 0x01, 0x03, 0x03, "1C 1C"    		  },
	{0x10, 0x01, 0x03, 0x02, "1C 2C"    		  },
	{0x10, 0x01, 0x03, 0x01, "1C 3C"     		  },
	{0x10, 0x01, 0x03, 0x00, "1C 4C"     		  },

	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x10, 0x01, 0x0c, 0x08, "2C 1C"     		  },
	{0x10, 0x01, 0x0c, 0x0c, "1C 1C"    		  },
	{0x10, 0x01, 0x0c, 0x04, "2C 3C"     		  },
	{0x10, 0x01, 0x0c, 0x00, "1C 6C"    		  },

	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x10, 0x01, 0x30, 0x30, "3"     		  },
	{0x10, 0x01, 0x30, 0x20, "4"    		  },
	{0x10, 0x01, 0x30, 0x10, "5"     		  },
	{0x10, 0x01, 0x30, 0x00, "6"    		  },

	{0   , 0xfe, 0   , 2   , "Bonus Life"	          },
	{0x10, 0x01, 0x40, 0x40, "20000"     		  },
	{0x10, 0x01, 0x40, 0x00, "None"			  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x80, 0x00, "Upright"     		  },
	{0x10, 0x01, 0x80, 0x80, "Cocktail"    		  },
};

STDDIPINFO(funkybee)

static struct BurnDIPInfo funkbeebDIPList[]=
{
	{0x0f, 0xff, 0xff, 0x20, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0f, 0x01, 0x20, 0x20, "Off"       		  },
	{0x0f, 0x01, 0x20, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x10, 0x01, 0x03, 0x03, "1C 1C"    		  },
	{0x10, 0x01, 0x03, 0x02, "1C 2C"    		  },
	{0x10, 0x01, 0x03, 0x01, "1C 3C"     		  },
	{0x10, 0x01, 0x03, 0x00, "1C 4C"     		  },

	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x10, 0x01, 0x0c, 0x08, "2C 1C"     		  },
	{0x10, 0x01, 0x0c, 0x0c, "1C 1C"    		  },
	{0x10, 0x01, 0x0c, 0x04, "2C 3C"     		  },
	{0x10, 0x01, 0x0c, 0x00, "1C 6C"    		  },

	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x10, 0x01, 0x30, 0x30, "1"     		  },
	{0x10, 0x01, 0x30, 0x20, "2"    		  },
	{0x10, 0x01, 0x30, 0x10, "3"     		  },
	{0x10, 0x01, 0x30, 0x00, "4"    		  },

	{0   , 0xfe, 0   , 2   , "Bonus Life"	          },
	{0x10, 0x01, 0x40, 0x40, "20000"     		  },
	{0x10, 0x01, 0x40, 0x00, "None"			  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x80, 0x00, "Upright"     		  },
	{0x10, 0x01, 0x80, 0x80, "Cocktail"    		  },
};

STDDIPINFO(funkbeeb)

static struct BurnDIPInfo skylancrDIPList[]=
{
	{0x0f, 0xff, 0xff, 0x20, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0f, 0x01, 0x20, 0x20, "Off"       		  },
	{0x0f, 0x01, 0x20, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x10, 0x01, 0x03, 0x03, "1C 1C"    		  },
	{0x10, 0x01, 0x03, 0x02, "1C 2C"    		  },
	{0x10, 0x01, 0x03, 0x01, "1C 3C"     		  },
	{0x10, 0x01, 0x03, 0x00, "1C 6C"     		  },

	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x10, 0x01, 0x0c, 0x08, "2C 1C"     		  },
	{0x10, 0x01, 0x0c, 0x0c, "1C 1C"    		  },
	{0x10, 0x01, 0x0c, 0x04, "2C 3C"     		  },
	{0x10, 0x01, 0x0c, 0x00, "1C 6C"    		  },

	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x10, 0x01, 0x30, 0x30, "1"     		  },
	{0x10, 0x01, 0x30, 0x20, "2"    		  },
	{0x10, 0x01, 0x30, 0x10, "3"     		  },
	{0x10, 0x01, 0x30, 0x00, "4"    		  },

	{0   , 0xfe, 0   , 2   , "Bonus Life"	          },
	{0x10, 0x01, 0x40, 0x40, "20000"     		  },
	{0x10, 0x01, 0x40, 0x00, "None"			  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x80, 0x00, "Upright"     		  },
	{0x10, 0x01, 0x80, 0x80, "Cocktail"    		  },
};

STDDIPINFO(skylancr)

static struct BurnDIPInfo skylanceDIPList[]=
{
	{0x0f, 0xff, 0xff, 0x20, NULL                     },
	{0x10, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0f, 0x01, 0x20, 0x20, "Off"       		  },
	{0x0f, 0x01, 0x20, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x10, 0x01, 0x03, 0x03, "1C 1C"    		  },
	{0x10, 0x01, 0x03, 0x02, "1C 2C"    		  },
	{0x10, 0x01, 0x03, 0x01, "1C 3C"     		  },
	{0x10, 0x01, 0x03, 0x00, "1C 6C"     		  },

	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x10, 0x01, 0x0c, 0x08, "2C 1C"     		  },
	{0x10, 0x01, 0x0c, 0x0c, "1C 1C"    		  },
	{0x10, 0x01, 0x0c, 0x04, "2C 3C"     		  },
	{0x10, 0x01, 0x0c, 0x00, "1C 6C"    		  },

	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x10, 0x01, 0x30, 0x30, "3"     		  },
	{0x10, 0x01, 0x30, 0x20, "4"    		  },
	{0x10, 0x01, 0x30, 0x10, "5"     		  },
	{0x10, 0x01, 0x30, 0x00, "64"    		  },

	{0   , 0xfe, 0   , 2   , "Bonus Life"	          },
	{0x10, 0x01, 0x40, 0x40, "20000 50000" 		  },
	{0x10, 0x01, 0x40, 0x00, "40000 70000"		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x80, 0x00, "Upright"     		  },
	{0x10, 0x01, 0x80, 0x80, "Cocktail"    		  },
};

STDDIPINFO(skylance)

UINT8 __fastcall funkybee_read(UINT16 address)
{
	switch (address)
	{
		case 0xf000:
			return 0;

		case 0xf800:
			watchdog = 0;
			return DrvInputs[0] | (DrvDips[0] & 0xe0);

		case 0xf801:
			return DrvInputs[1];

		case 0xf802:
			return DrvInputs[2];
	}

	return 0;
}

void __fastcall funkybee_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe000:
			*scroll = data;
		break;

		case 0xe800:
			*flipscreen = data & 1;
		break;

		case 0xe802: // coin counter
		case 0xe803:
		break;

		case 0xe805:
			*gfx_bank = data & 1;
		break;

		case 0xf800:
			watchdog = 0;
		break;
	}
}

UINT8 __fastcall funkybee_in_port(UINT16 address)
{
	switch (address & 0xff)
	{
		case 0x02:
			return AY8910Read(0);
	}

	return 0;
}

void __fastcall funkybee_out_port(UINT16 address, UINT8 data)
{
	switch (address & 0xff)
	{
		case 0x00:
		case 0x01:
			AY8910Write(0, address & 1, data);
		break; 
	}
}

static UINT8 funkybee_ay8910_read_A(UINT32)
{
	return DrvDips[1];
}

static INT32 DrvDoReset(INT32 clear_mem)
{
	if (clear_mem) {
		memset (AllRam, 0, RamEnd - AllRam);
	}

	ZetOpen(0);
	ZetReset();
	ZetClose();

	watchdog = 0;

	return 0;
}

static void DrvGfxDecode()
{
	INT32 Planes[2] = { 0, 4 };
	INT32 XOffs[8]  = { STEP4(0, 1), STEP4(0x40, 1) };
	INT32 YOffs[32] = { STEP8(0, 8), STEP8(0x80, 8), STEP8(0x100, 8), STEP8(0x180, 8) };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x4000);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, DrvGfxROM0, 0x4000);

	GfxDecode(0x400, 2, 8,  8, Planes, XOffs, YOffs, 0x080, tmp, DrvGfxROM0);
	GfxDecode(0x100, 2, 8, 32, Planes, XOffs, YOffs, 0x200, tmp, DrvGfxROM1);

	BurnFree (tmp);
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 32; i++)
	{
		INT32 bit0 = (DrvColPROM[i] >> 0) & 0x01;
		INT32 bit1 = (DrvColPROM[i] >> 1) & 0x01;
		INT32 bit2 = (DrvColPROM[i] >> 2) & 0x01;
		INT32 r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (DrvColPROM[i] >> 3) & 0x01;
		bit1 = (DrvColPROM[i] >> 4) & 0x01;
		bit2 = (DrvColPROM[i] >> 5) & 0x01;
		INT32 g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = (DrvColPROM[i] >> 6) & 0x01;
		bit2 = (DrvColPROM[i] >> 7) & 0x01;
		INT32 b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		DrvPalette[i] = (r << 16) | (g << 8) | b;
	}
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM		= Next; Next += 0x005000;

	DrvGfxROM0		= Next; Next += 0x010000;
	DrvGfxROM1		= Next; Next += 0x010000;

	DrvColPROM		= Next; Next += 0x000020;

	DrvPalette		= (UINT32*)Next; Next += 0x0020 * sizeof(UINT32);

	AllRam			= Next;

	DrvZ80RAM		= Next; Next += 0x000800;
	DrvVidRAM		= Next; Next += 0x002000;
	DrvColRAM		= Next; Next += 0x002000;

	flipscreen		= Next; Next += 0x000001;
	gfx_bank		= Next; Next += 0x000001;
	scroll			= Next; Next += 0x000001;

	RamEnd			= Next;

	pAY8910Buffer[0]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[1]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[2]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);

	MemEnd			= Next;

	return 0;
}

static INT32 DrvInit(INT32 game)
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (game) {
			if (BurnLoadRom(DrvZ80ROM  + 0x0000, 0, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM  + 0x2000, 1, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM  + 0x4000, 2, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0 + 0x0000, 3, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x2000, 4, 1)) return 1;

			if (BurnLoadRom(DrvColPROM + 0x0000, 5, 1)) return 1;
		} else {
			if (BurnLoadRom(DrvZ80ROM  + 0x0000, 0, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM  + 0x1000, 1, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM  + 0x2000, 2, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM  + 0x3000, 3, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0 + 0x0000, 4, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x2000, 5, 1)) return 1;

			if (BurnLoadRom(DrvColPROM + 0x0000, 6, 1)) return 1;
		}

		DrvPaletteInit();
		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x4fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x4fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetMapArea(0xa000, 0xbfff, 0, DrvVidRAM);
	ZetMapArea(0xa000, 0xbfff, 1, DrvVidRAM);
	ZetMapArea(0xa000, 0xbfff, 2, DrvVidRAM);
	ZetMapArea(0xc000, 0xdfff, 0, DrvColRAM);
	ZetMapArea(0xc000, 0xdfff, 1, DrvColRAM);
	ZetMapArea(0xc000, 0xdfff, 2, DrvColRAM);
	ZetSetWriteHandler(funkybee_write);
	ZetSetReadHandler(funkybee_read);
	ZetSetOutHandler(funkybee_out_port);
	ZetSetInHandler(funkybee_in_port);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 1500000, nBurnSoundRate, &funkybee_ay8910_read_A, NULL, NULL, NULL);

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();
	AY8910Exit(0);

	BurnFree(AllMem);

	return 0;
}

static void draw_bg_layer()
{
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) * 8;
		INT32 sy = (offs / 0x20) * 8;

		sx -= *scroll;
		if (sx < -7) sx += 256;

		INT32 ofst = ((offs / 0x20) * 0x100) + (offs & 0x1f);

		INT32 attr  = DrvColRAM[ofst];
		INT32 code  = DrvVidRAM[ofst] + ((attr & 0x80) << 1) + (*gfx_bank * 0x200);
		INT32 color = attr & 0x03;

		Render8x8Tile_Clip(pTransDraw, code, sx - 12, sy, color, 2, 0, DrvGfxROM0);
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0x0f; offs >= 0; offs--)
	{
		INT32 ofst  = offs + 0x1e00;
		INT32 attr  = DrvVidRAM[ofst];
		INT32 sy    = 224 - DrvColRAM[ofst];
		INT32 sx    = DrvVidRAM[ofst + 0x10];
		INT32 color = DrvColRAM[ofst + 0x10] & 3;
		INT32 code = (attr >> 2) + ((attr & 2) << 5) + (*gfx_bank * 0x080);
		INT32 flipy = attr & 1;
		INT32 flipx = 0;

		if (flipy) {
			if (flipx) {
				RenderCustomTile_Mask_FlipXY_Clip(pTransDraw, 8, 32, code, sx - 12, sy, color, 2, 0, 0x10, DrvGfxROM1);
			} else {
				RenderCustomTile_Mask_FlipY_Clip(pTransDraw, 8, 32, code, sx - 12, sy, color, 2, 0, 0x10, DrvGfxROM1);
			}
		} else {
			if (flipx) {
				RenderCustomTile_Mask_FlipX_Clip(pTransDraw, 8, 32, code, sx - 12, sy, color, 2, 0, 0x10, DrvGfxROM1);
			} else {
				RenderCustomTile_Mask_Clip(pTransDraw, 8, 32, code, sx - 12, sy, color, 2, 0, 0x10, DrvGfxROM1);
			}
		}
	}
}

static void draw_fg_layer()
{
	for (INT32 offs = 0x1f; offs >= 0; offs--)
	{
		int flip  = *flipscreen;
		int sy    = offs * 8;
		if (flip) sy = 248 - sy;

		int code  = DrvVidRAM[0x1c00 + offs] + (*gfx_bank * 0x200);
		int color = DrvColRAM[0x1f10] & 0x03;
		int sx = flip ? DrvVidRAM[0x1f1f] : DrvVidRAM[0x1f10];

		if (flip)
			Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, (232 - sx) + 4, sy ^ 0xf8, color, 2, 0, 0, DrvGfxROM0);
		else
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx - 12, sy, color, 2, 0, 0, DrvGfxROM0);

		code  = DrvVidRAM[0x1d00 + offs] + (*gfx_bank * 0x200);
		color = DrvColRAM[0x1f11] & 0x03;
		sx = flip ? DrvVidRAM[0x1f1e] : DrvVidRAM[0x1f11];

		if (flip)
			Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, (232 - sx) + 4, sy ^ 0xf8, color, 2, 0, 0, DrvGfxROM0);
		else
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx - 12, sy, color, 2, 0, 0, DrvGfxROM0);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvPaletteInit();
		DrvRecalc = 0;
	}

	draw_bg_layer();
	draw_sprites();
	draw_fg_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset(1);
	}

	watchdog++;
	if (watchdog >= 180) {
		DrvDoReset(0);
	}

	{
		memset (DrvInputs, 0, 3);

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	ZetOpen(0);
	ZetRun(3072000 / 60);
	ZetRaiseIrq(0);
	ZetClose();

	if (pBurnSoundOut) {
		INT32 nSample;
		INT32 nSegmentLength = nBurnSoundLen;
		INT16* pSoundBuf = pBurnSoundOut;
		if (nSegmentLength) {
			AY8910Update(0, &pAY8910Buffer[0], nSegmentLength);
			for (INT32 n = 0; n < nSegmentLength; n++) {
				nSample  = pAY8910Buffer[0][n];
				nSample += pAY8910Buffer[1][n];
				nSample += pAY8910Buffer[2][n];

				nSample /= 4;

				nSample = BURN_SND_CLIP(nSample);

				pSoundBuf[(n << 1) + 0] = nSample;
				pSoundBuf[(n << 1) + 1] = nSample;
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

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		AY8910Scan(nAction, pnMin);
	}

	return 0;
}


// Funky Bee

static struct BurnRomInfo funkybeeRomDesc[] = {
	{ "funkybee.1",    0x1000, 0x3372cb33, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "funkybee.3",    0x1000, 0x7bf7c62f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "funkybee.2",    0x1000, 0x8cc0fe8e, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "funkybee.4",    0x1000, 0x1e1aac26, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "funkybee.5",    0x2000, 0x86126655, 2 | BRF_GRA },		//  4 Graphics tiles
	{ "funkybee.6",    0x2000, 0x5fffd323, 2 | BRF_GRA },		//  5

	{ "funkybee.clr",  0x0020, 0xe2cf5fe2, 3 | BRF_GRA },		//  6 Color prom
};

STD_ROM_PICK(funkybee)
STD_ROM_FN(funkybee)

static INT32 funkybeeInit()
{
	return DrvInit(0);
}

struct BurnDriver BurnDrvfunkybee = {
	"funkybee", NULL, NULL, NULL, "1982",
	"Funky Bee\0", NULL, "Orca", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, funkybeeRomInfo, funkybeeRomName, NULL, NULL, DrvInputInfo, funkybeeDIPInfo,
	funkybeeInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	224, 236, 3, 4
};


// Funky Bee (bootleg, harder)

static struct BurnRomInfo funkbeebRomDesc[] = {
	{ "senza_orca.fb1", 0x1000, 0x7f2e7f85, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "funkybee.3",     0x1000, 0x7bf7c62f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "funkybee.2",     0x1000, 0x8cc0fe8e, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "senza_orca.fb4", 0x1000, 0x53c2db3b, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "funkybee.5",     0x2000, 0x86126655, 2 | BRF_GRA },		 //  4 Graphics tiles
	{ "funkybee.6",     0x2000, 0x5fffd323, 2 | BRF_GRA },		 //  5

	{ "funkybee.clr",   0x0020, 0xe2cf5fe2, 3 | BRF_GRA },		 //  6 Color prom
};

STD_ROM_PICK(funkbeeb)
STD_ROM_FN(funkbeeb)

struct BurnDriver BurnDrvfunkbeeb = {
	"funkybeeb", "funkybee", NULL, NULL, "1982",
	"Funky Bee (bootleg, harder)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, funkbeebRomInfo, funkbeebRomName, NULL, NULL, DrvInputInfo, funkbeebDIPInfo,
	funkybeeInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	224, 236, 3, 4
};


// Sky Lancer

static struct BurnRomInfo skylancrRomDesc[] = {
	{ "1sl.5a",        0x2000, 0xe80b315e, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "2sl.5c",        0x2000, 0x9d70567b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3sl.5d",        0x2000, 0x64c39457, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "4sl.6a",        0x2000, 0x9b4469a5, 2 | BRF_GRA },		//  3 Graphics tiles
	{ "5sl.6c",        0x2000, 0x29afa134, 2 | BRF_GRA },		//  4

	{ "18s030.1a",     0x0020, 0xe645bacb, 3 | BRF_GRA },		//  5 Color prom
};

STD_ROM_PICK(skylancr)
STD_ROM_FN(skylancr)

static INT32 skylancrInit()
{
	return DrvInit(1);
}

struct BurnDriver BurnDrvskylancr = {
	"skylancr", NULL, NULL, NULL, "1983",
	"Sky Lancer\0", NULL, "Orca", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, skylancrRomInfo, skylancrRomName, NULL, NULL, DrvInputInfo, skylancrDIPInfo,
	skylancrInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	224, 236, 3, 4
};


// Sky Lancer (Esco Trading Co license)

static struct BurnRomInfo skylanceRomDesc[] = {
	{ "1.5a",          0x2000, 0x82d55824, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 Code
	{ "2.5c",          0x2000, 0xdff3a682, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3.5d",          0x1000, 0x7c006ee6, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "4.6a",          0x2000, 0x0f8ede07, 2 | BRF_GRA },		//  3 Graphics tiles
	{ "5.6b",          0x2000, 0x24cec070, 2 | BRF_GRA },		//  4

	{ "18s030.1a",     0x0020, 0xe645bacb, 3 | BRF_GRA },		//  5 Color prom
};

STD_ROM_PICK(skylance)
STD_ROM_FN(skylance)

struct BurnDriver BurnDrvskylance = {
	"skylancre", "skylancr", NULL, NULL, "1983",
	"Sky Lancer (Esco Trading Co license)\0", NULL, "Orca (Esco Trading Co license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, skylanceRomInfo, skylanceRomName, NULL, NULL, DrvInputInfo, skylanceDIPInfo,
	skylancrInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	224, 236, 3, 4
};
