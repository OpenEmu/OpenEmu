// FB Alpha Funky Bee Driver Module
// Based on MAME driver by Zsolt Vasvari

#include "tiles_generic.h"
#include "zet.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *Mem, *MemEnd, *Rom, *Gfx, *Prom;
static UINT8 DrvJoy1[8], DrvJoy2[8], DrvJoy3[8], DrvDips[2], DrvReset;
static UINT32 *Palette, *DrvPal;
static UINT8 DrvRecalcPal;

static INT16 *pAY8910Buffer[3], *pFMBuffer = NULL;
static INT32 skylancr = 0;

static INT32 funkybee_gfx_bank, funkybee_scroll_x, funkybee_flipscreen;

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
	// Default Values
	{0x0f, 0xff, 0xff, 0x20, NULL                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0f, 0x01, 0x20, 0x20, "Off"       		  },
	{0x0f, 0x01, 0x20, 0x00, "On"       		  },

	// Default Values
	{0x10, 0xff, 0xff, 0xff, NULL                     },

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
	// Default Values
	{0x0f, 0xff, 0xff, 0x20, NULL                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0f, 0x01, 0x20, 0x20, "Off"       		  },
	{0x0f, 0x01, 0x20, 0x00, "On"       		  },

	// Default Values
	{0x10, 0xff, 0xff, 0xff, NULL                     },

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
	// Default Values
	{0x0f, 0xff, 0xff, 0x20, NULL                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0f, 0x01, 0x20, 0x20, "Off"       		  },
	{0x0f, 0x01, 0x20, 0x00, "On"       		  },

	// Default Values
	{0x10, 0xff, 0xff, 0xff, NULL                     },

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
	// Default Values
	{0x0f, 0xff, 0xff, 0x20, NULL                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x0f, 0x01, 0x20, 0x20, "Off"       		  },
	{0x0f, 0x01, 0x20, 0x00, "On"       		  },

	// Default Values
	{0x10, 0xff, 0xff, 0xff, NULL                     },

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
	UINT8 ret = 0;

	switch (address)
	{
		case 0xf000:
			return 0;

		case 0xf800:
		{
			for (INT32 i = 0; i < 8; i++)
				ret |= DrvJoy1[i] << i;

			return ret | DrvDips[0];
		}

		case 0xf801:
		{
			for (INT32 i = 0; i < 8; i++)
				ret |= DrvJoy2[i] << i;

			return ret;

		}

		case 0xf802:
		{
			for (INT32 i = 0; i < 8; i++)
				ret |= DrvJoy3[i] << i;

			return ret;
		}
	}

	return 0;
}

void __fastcall funkybee_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe000:
			funkybee_scroll_x = data;
		break;

		case 0xe800:
			funkybee_flipscreen = data & 1;
		break;

		case 0xe802: // coin counter
		case 0xe803:
		break;

		case 0xe805:
			funkybee_gfx_bank = (data & 1) << 9;
		break;

		case 0xf800: // watchdog
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

static INT32 DrvDoReset()
{
	DrvReset = 0;
	memset (Rom + 0x8000, 0, 0x0800);
	memset (Rom + 0xa000, 0, 0x4000);

	funkybee_gfx_bank = 0;
	funkybee_scroll_x = 0;
	funkybee_flipscreen = 0;
	ZetOpen(0);
	ZetReset();
	ZetClose();

	return 0;
}

static void funkybee_gfx_decode()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x4000);
	if (tmp == NULL) {
		return;
	}

	static INT32 PlaneOffsets[2] = { 0, 4 };

	static INT32 XOffsets[8] = {
		0, 1, 2, 3, 64, 65, 66, 67
	};

	static INT32 YOffsets[32] = {
		  0,   8,  16,  24,  32,  40,  48,  56, 128, 136, 144, 152, 160, 168, 176, 184,
	  	256, 264, 272, 280, 288, 296, 304, 312, 384, 392, 400, 408, 416, 424, 432, 440
	};

	memcpy (tmp, Gfx, 0x4000);

	GfxDecode(0x400, 2, 8,  8, PlaneOffsets, XOffsets, YOffsets, 0x080, tmp, Gfx + 0x00000);
	GfxDecode(0x100, 2, 8, 32, PlaneOffsets, XOffsets, YOffsets, 0x200, tmp, Gfx + 0x10000);

	BurnFree (tmp);
}

static void funkybee_palette_init()
{
	for (INT32 i = 0; i < 32; i++)
	{
		INT32 bit0,bit1,bit2,r,g,b;

		bit0 = (Prom[i] >> 0) & 0x01;
		bit1 = (Prom[i] >> 1) & 0x01;
		bit2 = (Prom[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (Prom[i] >> 3) & 0x01;
		bit1 = (Prom[i] >> 4) & 0x01;
		bit2 = (Prom[i] >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = (Prom[i] >> 6) & 0x01;
		bit2 = (Prom[i] >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		Palette[i] = (r << 16) | (g << 8) | b;
	}
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Rom           = Next; Next += 0x10000;
	Gfx           = Next; Next += 0x20000;
	Prom          = Next; Next += 0x00020;

	pFMBuffer     = (INT16*)Next; Next += nBurnSoundLen * 3 * sizeof(INT16);

	Palette	      = (UINT32*)Next; Next += 0x00020 * sizeof(UINT32);
	DrvPal	      = (UINT32*)Next; Next += 0x00020 * sizeof(UINT32);

	MemEnd        = Next;

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

	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;

	if (skylancr)
	{
		if (BurnLoadRom(Rom  + 0x0000, 0, 1)) return 1;
		if (BurnLoadRom(Rom  + 0x2000, 1, 1)) return 1;
		if (BurnLoadRom(Rom  + 0x4000, 2, 1)) return 1;
		if (BurnLoadRom(Gfx  + 0x0000, 3, 1)) return 1;
		if (BurnLoadRom(Gfx  + 0x2000, 4, 1)) return 1;
		if (BurnLoadRom(Prom + 0x0000, 5, 1)) return 1;
	} else {
		if (BurnLoadRom(Rom  + 0x0000, 0, 1)) return 1;
		if (BurnLoadRom(Rom  + 0x1000, 1, 1)) return 1;
		if (BurnLoadRom(Rom  + 0x2000, 2, 1)) return 1;
		if (BurnLoadRom(Rom  + 0x3000, 3, 1)) return 1;
		if (BurnLoadRom(Gfx  + 0x0000, 4, 1)) return 1;
		if (BurnLoadRom(Gfx  + 0x2000, 5, 1)) return 1;
		if (BurnLoadRom(Prom + 0x0000, 6, 1)) return 1;
	}

	funkybee_gfx_decode();
	funkybee_palette_init();

	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(funkybee_read);
	ZetSetWriteHandler(funkybee_write);
	ZetSetInHandler(funkybee_in_port);
	ZetSetOutHandler(funkybee_out_port);
	ZetMapArea(0x0000, 0x4fff, 0, Rom + 0x0000);
	ZetMapArea(0x0000, 0x4fff, 2, Rom + 0x0000);
	ZetMapArea(0x8000, 0x87ff, 0, Rom + 0x8000);
	ZetMapArea(0x8000, 0x87ff, 1, Rom + 0x8000);
	ZetMapArea(0x8000, 0x87ff, 2, Rom + 0x8000);
	ZetMapArea(0xa000, 0xdfff, 0, Rom + 0xa000);
	ZetMapArea(0xa000, 0xdfff, 1, Rom + 0xa000);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 1500000, nBurnSoundRate, &funkybee_ay8910_read_A, NULL, NULL, NULL);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	skylancr = 0;

	ZetExit();
	AY8910Exit(0);
	GenericTilesExit();

	BurnFree (Mem);

	Palette = DrvPal = NULL;

	pFMBuffer = NULL;
	Mem = MemEnd = Rom = Gfx = Prom = NULL;

	funkybee_gfx_bank = 0;
	funkybee_scroll_x = 0;
	funkybee_flipscreen = 0;
	DrvRecalcPal = 0;

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalcPal) {
		for (INT32 i = 0; i < 32; i++) {
			UINT32 col = Palette[i];
			DrvPal[i] = BurnHighCol(col >> 16, col >> 8, col, 0);
		}
	}

	for (INT32 offa = 0; offa < 0x1c00; offa += 0x100)
	{
		for (INT32 offb = 0; offb < 0x20; offb++)
		{
			INT32 sy = (offa >> 5) ^ (funkybee_flipscreen * 0xf8);
			INT32 sx = (offb << 3) ^ (funkybee_flipscreen * 0xf8);

			sx = (sx - funkybee_scroll_x) & 0xff;

			INT32 offs = offa + offb;

			INT32 code = Rom[0xa000 + offs] + ((Rom[0xc000 + offs] & 0x80) << 1) + funkybee_gfx_bank;
			INT32 color = Rom[0xc000 + offs] & 0x03;

			if (funkybee_flipscreen)
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx - 8, sy - 32, color, 2, 0, Gfx);
			else
				Render8x8Tile_Clip(pTransDraw, code, sx - 12, sy, color, 2, 0, Gfx);
		}
	}

	for (INT32 offs = 0x0f; offs >= 0; offs--)
	{
		INT32 attr = Rom[0xbe00 + offs];
		INT32 code = (attr >> 2) | ((attr & 2) << 5);
		INT32 color = 4 | (Rom[0xde10 + offs] & 3);
		INT32 flipy = attr & 1;
		INT32 flipx = funkybee_flipscreen;
		INT32 sx = Rom[0xbe10 + offs] - 12;
		INT32 sy =  224 - Rom[0xde00 + offs];

		code = (code << 2) + funkybee_gfx_bank;

		if (flipy) {
			if (flipx) {
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code + 0, sx, sy + 24, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code + 1, sx, sy + 16, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code + 2, sx, sy +  8, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code + 3, sx, sy +  0, color, 2, 0, 0, Gfx);
			} else {
				Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code + 0, sx, sy + 24, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code + 1, sx, sy + 16, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code + 2, sx, sy +  8, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code + 3, sx, sy +  0, color, 2, 0, 0, Gfx);
			}
		} else {
			if (flipx) {
				Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code + 0, sx, sy +  0, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code + 1, sx, sy +  8, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code + 2, sx, sy + 16, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code + 3, sx, sy + 24, color, 2, 0, 0, Gfx);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, code + 0, sx, sy +  0, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_Clip(pTransDraw, code + 1, sx, sy +  8, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_Clip(pTransDraw, code + 2, sx, sy + 16, color, 2, 0, 0, Gfx);
				Render8x8Tile_Mask_Clip(pTransDraw, code + 3, sx, sy + 24, color, 2, 0, 0, Gfx);
			}
		}
	}

	for (INT32 offs = 0x1f;offs >= 0;offs--)
	{
		INT32 code = Rom[0xbc00 + offs] + funkybee_gfx_bank;
		INT32 color = Rom[0xdf10] & 0x03;
		INT32 sx = Rom[0xbf10];
		INT32 sy = offs << 3;

		if (funkybee_flipscreen)
			Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, (232 - sx) + 4, sy ^ 0xf8, color, 2, 0, 0, Gfx);
		else
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx - 12, sy, color, 2, 0, 0, Gfx);
		
		code = Rom[0xbd00 + offs] + funkybee_gfx_bank;
		color = Rom[0xdf11] & 0x03;
		sx = Rom[0xbf11];
		sy = offs << 3;

		if (funkybee_flipscreen)
			Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, (232 - sx) + 4, sy ^ 0xf8, color, 2, 0, 0, Gfx);
		else
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx - 12, sy, color, 2, 0, 0, Gfx);
	}

	BurnTransferCopy(DrvPal);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
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

		ba.Data	  = Rom + 0x8000;
		ba.nLen	  = 0x0800;
		ba.szName = "Work Ram";
		BurnAcb(&ba);

		ba.Data	  = Rom + 0xa000;
		ba.nLen	  = 0x4000;
		ba.szName = "Video Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		AY8910Scan(nAction, pnMin);

		SCAN_VAR(funkybee_gfx_bank);
		SCAN_VAR(funkybee_scroll_x);
		SCAN_VAR(funkybee_flipscreen);
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

struct BurnDriver BurnDrvfunkybee = {
	"funkybee", NULL, NULL, NULL, "1982",
	"Funky Bee\0", NULL, "Orca", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, funkybeeRomInfo, funkybeeRomName, NULL, NULL, DrvInputInfo, funkybeeDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalcPal, 0x20,
	224, 232, 3, 4
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
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalcPal, 0x20,
	224, 232, 3, 4
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
	skylancr = 1;

	return DrvInit();
}

struct BurnDriver BurnDrvskylancr = {
	"skylancr", NULL, NULL, NULL, "1983",
	"Sky Lancer\0", NULL, "Orca", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, skylancrRomInfo, skylancrRomName, NULL, NULL, DrvInputInfo, skylancrDIPInfo,
	skylancrInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalcPal, 0x20,
	224, 232, 3, 4
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
	skylancrInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalcPal, 0x20,
	224, 232, 3, 4
};
