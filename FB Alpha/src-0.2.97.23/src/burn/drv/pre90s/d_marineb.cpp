// Based on original MAME driver writen by Zsolt Vasvari


#include "tiles_generic.h"
#include "zet.h"

#include "driver.h"
extern "C" {
 #include "ay8910.h"
}

enum { SPRINGER = 0, MARINEB };

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *RamStart;
static UINT8 *RamEnd;
static UINT32 *TempPalette;
static UINT32 *DrvPalette;
static UINT8 DrvRecalcPalette;

static UINT8 DrvInputPort0[8];
static UINT8 DrvInputPort1[8];
static UINT8 DrvInputPort2[8];
static UINT8 DrvDip;
static UINT8 DrvInput[3];
static UINT8 DrvReset;

static UINT8 *DrvZ80ROM;  
static UINT8 *DrvZ80RAM;  
static UINT8 *DrvColPROM; 
static UINT8 *DrvVidRAM;  
static UINT8 *DrvColRAM;  
static UINT8 *DrvSprRAM;  
static UINT8 *DrvGfxROM0; 
static UINT8 *DrvGfxROM1; 
static UINT8 *DrvGfxROM2; 

static UINT8 DrvPaletteBank;	
static UINT8 DrvColumnScroll;	
static UINT8 ActiveLowFlipscreen;
static UINT8 DrvFlipScreenY;
static UINT8 DrvFlipScreenX;
static INT32 DrvInterruptEnable;
static INT32 hardware;

static INT16 *pAY8910Buffer[3];



static struct BurnInputInfo MarinebInputList[] =
{

	{"P1 Coin"  	   , BIT_DIGITAL  , DrvInputPort2 + 0,  "p1 coin"   },
	
	{"P1 Start"        , BIT_DIGITAL  , DrvInputPort2 + 2, "p1 start"  },	
	{"P1 Up"           , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 up"     },
	{"P1 Down"         , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 down"   },
	{"P1 Left"         , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 left"   },
	{"P1 Right"        , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 right"  },	
	{"P1 Fire"         , BIT_DIGITAL  , DrvInputPort2 + 4, "p1 fire 1" },
	
	{"P2 Start"        , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 start"  },	
	{"P2 Up"           , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 up"     },
	{"P2 Down"         , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 down"   },
	{"P2 Left"         , BIT_DIGITAL  , DrvInputPort1 + 6, "p2 left"   },
	{"P2 Right"        , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 right"  },	
	{"P2 Fire"         , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 1" },

	{"Reset"           , BIT_DIGITAL  , &DrvReset        , "reset"     },	
	{"Dip"             , BIT_DIPSWITCH, &DrvDip          , "dip"       },
	
};

STDINPUTINFO(Marineb)

static struct BurnDIPInfo MarinebDIPList[]=
{	
	{0x0E, 0xFF, 0xFF, 0x40, NULL			},
	
	{0   , 0xFE, 0   ,    4, "Lives"		},
	{0x0E, 0x01, 0x03, 0x00, "3"			},
	{0x0E, 0x01, 0x03, 0x01, "4"			},
	{0x0E, 0x01, 0x03, 0x02, "5"			},
	{0x0E, 0x01, 0x03, 0x03, "6"			},
	
	{0   , 0xFE, 0   ,    2, "Coinage"		},
	{0x0E, 0x01, 0x1C, 0x00, "1 Coin 1 Credit"		},
	{0x0E, 0x01, 0x1C, 0x1C, "Free Play"	},
	
	{0   , 0xFE, 0   ,    2, "Bonus Life"	},
	{0x0E, 0x01, 0x20, 0x00, "20000 50000"	},
	{0x0E, 0x01, 0x20, 0x20, "40000 70000"	},
	
	{0   , 0xFE, 0   ,    2, "Cabinet"		},
	{0x0E, 0x01, 0x40, 0x40, "Upright"		},
	{0x0E, 0x01, 0x40, 0x00, "Cocktail"		},	
	
};

STDDIPINFO(Marineb)

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM			= Next; Next += 0x10000;	
	DrvColPROM    		= Next; Next += 0x200;	
	DrvGfxROM0    		= Next; Next += 512 * 8 * 8;
	DrvGfxROM1   		= Next; Next += 64 * 16 * 16;
	DrvGfxROM2    		= Next; Next += 64 * 32 * 32;
	
	TempPalette = (UINT32*)Next; Next += 0x100 * sizeof(UINT32); // calculate one time
	DrvPalette  = (UINT32*)Next; Next += 0x100 * sizeof(UINT32);
	
	pAY8910Buffer[0]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[1]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[2]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	
	RamStart	= Next;

	DrvZ80RAM	= Next;  Next += 0x800;
	DrvVidRAM	= Next;  Next += 0x400;	
	DrvSprRAM   = Next;  Next += 0x100;
	DrvColRAM    = Next; Next += 0x400;
	
	RamEnd		= Next;
	MemEnd		= Next;	
	
	return 0;
}

static void CleanAndInitStuff()
{
	DrvPaletteBank = 0;	
	DrvColumnScroll = 0;	
	DrvFlipScreenY = 0;
	DrvFlipScreenX = 0;
	DrvInterruptEnable = 0; 
	hardware = 0;
	ActiveLowFlipscreen = 0;	
	
	memset(DrvInputPort0, 0, 8);
	memset(DrvInputPort1, 0, 8);
	memset(DrvInputPort2, 0, 8);
	memset(DrvInput, 0, 3);
	
    DrvDip = 0;    
    DrvReset = 0;
}


UINT8 __fastcall marineb_read(UINT16 address)
{
	switch (address) {		
		case 0xa800:
			return DrvInput[0];		
			
		case 0xa000: 
			return DrvInput[1];		
			
		case 0xb000:
			return DrvDip;		
			
		case 0xb800:
			return DrvInput[2];	
	}

	return 0;
}


void __fastcall marineb_write(UINT16 address, UINT8 data)
{
	switch (address) {
	
		case 0x9800:
			DrvColumnScroll = data; 
			return;

		case 0x9a00:
			DrvPaletteBank = (DrvPaletteBank & 0x02) | (data & 0x01);
			return;

		case 0x9c00:
			DrvPaletteBank = (DrvPaletteBank & 0x01) | ((data & 0x01) << 1);
			return;

	
		case 0xa000:
			DrvInterruptEnable = data;
			return;

		case 0xa001:
			DrvFlipScreenY = data ^ ActiveLowFlipscreen;
			return;

		case 0xa002:
			DrvFlipScreenX = data ^ ActiveLowFlipscreen;
			return; 
	}	
}

void __fastcall marineb_write_port(UINT16 port, UINT8 data)
{
	switch (port & 0xFF) {	
		case 0x08:
		case 0x09:
		AY8910Write(0, port & 1, data);
		break;
	}	
}

static INT32 DrvDoReset()
{	
	memset (RamStart, 0, RamEnd - RamStart);
	
	ZetOpen(0);
	ZetReset();
	ZetClose();	
	
	AY8910Reset(0);
	
	DrvPaletteBank = 0;	
	DrvColumnScroll = 0;	
	DrvFlipScreenY = 0;
	DrvFlipScreenX = 0;
	DrvInterruptEnable = 0; 

	return 0;
}

static void DrvCreatePalette()
{
	for (INT32 i = 0; i < 256; i++)
	{
		INT32 bit0, bit1, bit2, r, g, b;

		// Red
		bit0 = (DrvColPROM[i] >> 0) & 0x01;
		bit1 = (DrvColPROM[i] >> 1) & 0x01;
		bit2 = (DrvColPROM[i] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// Green
		bit0 = (DrvColPROM[i] >> 3) & 0x01;
		bit1 = (DrvColPROM[i + 256] >> 0) & 0x01;
		bit2 = (DrvColPROM[i + 256] >> 1) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		// Blue
		bit0 = 0;
		bit1 = (DrvColPROM[i + 256] >> 2) & 0x01;
		bit2 = (DrvColPROM[i + 256] >> 3) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		
		TempPalette[i] = (r << 16) | (g << 8) | (b << 0); 		
	}	
}


static INT32 MarinebCharPlane[2] = { 0, 4 };
static INT32 MarinebCharXOffs[8] = { 0, 1, 2, 3, 64, 65, 66, 67 };
static INT32 MarinebCharYOffs[8] = { 0, 8, 16, 24, 32, 40, 48, 56 };

static INT32 MarinebSmallSpriteCharPlane[2] = { 0, 65536 };
static INT32 MarinebSmallSpriteXOffs[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 64, 65, 66, 67, 68, 69, 70, 71 };
static INT32 MarinebSmallSpriteYOffs[16] = { 0, 8, 16, 24, 32, 40, 48, 56, 128, 136, 144, 152, 160, 168, 176, 184 };

static INT32 MarinebBigSpriteCharPlane[2] = { 0, 65536 };
static INT32 MarinebBigSpriteXOffs[32] = { 0, 1, 2, 3, 4, 5, 6, 7, 64, 65, 66, 67, 68, 69, 70, 71, 256, 257, 258, 259, 260, 261, 262, 263, 320, 321, 322, 323, 324, 325, 326, 327 };
static INT32 MarinebBigSpriteYOffs[32] = { 0, 8, 16, 24, 32, 40, 48, 56, 128, 136, 144, 152, 160, 168, 176, 184, 512, 520, 528, 536, 544, 552, 560, 568, 640, 648, 656, 664, 672, 680, 688, 696 };


static INT32 MarinebLoadRoms()
{						   
	// Load roms	
	
	// Z80
	for (INT32 i = 0; i < 5; i++) {
		if (BurnLoadRom(DrvZ80ROM + (i * 0x1000), i, 1)) return 1;			
	}			

	UINT8 *tmp = (UINT8*)BurnMalloc(0x4000);
	if (tmp == NULL) return 1;
		
	// Chars
	memset(tmp, 0, 0x2000);			
	if (BurnLoadRom(tmp, 5, 1)) return 1;		
	GfxDecode(0x200, 2, 8, 8, MarinebCharPlane, MarinebCharXOffs, MarinebCharYOffs, 0x80, tmp, DrvGfxROM0);			
		
	// Sprites
	memset(tmp, 0, 0x4000);		
	if (BurnLoadRom(tmp, 6, 1))     return 1;
	if (BurnLoadRom(tmp + 0x2000, 7, 1)) 	 return 1;	
		
	// Small Sprites
	GfxDecode(0x40, 2, 16, 16, MarinebSmallSpriteCharPlane, MarinebSmallSpriteXOffs, MarinebSmallSpriteYOffs, 0x100, tmp, DrvGfxROM1);	
		
	// Big Sprites
	GfxDecode(0x40, 2, 32, 32, MarinebBigSpriteCharPlane, MarinebBigSpriteXOffs, MarinebBigSpriteYOffs, 0x400, tmp, DrvGfxROM2);				

	BurnFree(tmp);
		
	// ColorRoms
	if (BurnLoadRom(DrvColPROM, 8, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x100, 9, 1)) return 1;	

	return 0;
}			  

static INT32 SpringerLoadRoms()
{						   
	// Load roms	
	
	// Z80
	for (INT32 i = 0; i < 5; i++) {
		if (BurnLoadRom(DrvZ80ROM + (i * 0x1000), i, 1)) return 1;			
	}			

	UINT8 *tmp = (UINT8*)BurnMalloc(0x4000);
	if (tmp == NULL) return 1;
		
	// Chars
	memset(tmp, 0, 0x4000);			
	if (BurnLoadRom(tmp, 5, 1)) return 1;
	if (BurnLoadRom(tmp + 0x1000, 6, 1)) return 1;	
		
	GfxDecode(0x200, 2, 8, 8, MarinebCharPlane, MarinebCharXOffs, MarinebCharYOffs, 0x80, tmp, DrvGfxROM0);			
		
	memset(tmp, 0, 0x4000);		
	if (BurnLoadRom(tmp, 7, 1))     return 1;
	if (BurnLoadRom(tmp + 0x2000, 8, 1)) 	 return 1;			
		
	// Small Sprites
	GfxDecode(0x40, 2, 16, 16, MarinebSmallSpriteCharPlane, MarinebSmallSpriteXOffs, MarinebSmallSpriteYOffs, 0x100, tmp, DrvGfxROM1);		
	// Big Sprites
	GfxDecode(0x40, 2, 32, 32, MarinebBigSpriteCharPlane, MarinebBigSpriteXOffs, MarinebBigSpriteYOffs, 0x400, tmp, DrvGfxROM2);				

	BurnFree(tmp);
		
	// ColorRoms
	if (BurnLoadRom(DrvColPROM, 9, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x100, 10, 1)) return 1;		

	return 0;
}

static INT32 DrvInit() 
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();
	
	switch(hardware) {
		case MARINEB:
			MarinebLoadRoms();
			break;
		case SPRINGER:
			SpringerLoadRoms();
			break;		
	}	
	
	DrvCreatePalette();	
	
	ZetInit(0);
	ZetOpen(0);
	
	ZetMapArea (0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea (0x0000, 0x7fff, 2, DrvZ80ROM);
	
	ZetMapArea (0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea (0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea (0x8000, 0x87ff, 2, DrvZ80RAM);
	
	ZetMapArea (0x8800, 0x8bff, 0, DrvVidRAM);
	ZetMapArea (0x8800, 0x8bff, 1, DrvVidRAM);
	ZetMapArea (0x8800, 0x8bff, 2, DrvVidRAM);
	
	ZetMapArea (0x8c00, 0x8c3f, 0, DrvSprRAM);
	ZetMapArea (0x8c00, 0x8c3f, 1, DrvSprRAM);
	ZetMapArea (0x8c00, 0x8c3f, 2, DrvSprRAM);
	
	ZetMapArea (0x9000, 0x93ff, 0, DrvColRAM);
	ZetMapArea (0x9000, 0x93ff, 1, DrvColRAM);
	ZetMapArea (0x9000, 0x93ff, 2, DrvColRAM);
	
	ZetSetReadHandler(marineb_read);
	ZetSetWriteHandler(marineb_write);
	ZetSetOutHandler(marineb_write_port);

	ZetMemEnd();
	ZetClose();	
	
	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	
	GenericTilesInit();
	DrvDoReset();	
	
	return 0; 

}

static INT32 DrvExit()
{
	GenericTilesExit();
	ZetExit();	
	AY8910Exit(0);

	BurnFree (AllMem);
	
	CleanAndInitStuff();
	
	return 0;
}

static void RenderMarinebBg()
{	
	INT32 TileIndex = 0;

	for (INT32 my = 0; my < 32; my++) {		
		for (INT32 mx = 0; mx < 32; mx++) {	
			
			TileIndex = (my * 32) + mx; 
			
			INT32 code = DrvVidRAM[TileIndex];			
		    INT32 color = DrvColRAM[TileIndex];
			
			code |= ((color & 0xc0) << 2);
			
			color &= 0x0f;
			color |= DrvPaletteBank << 4;
			
			INT32 flipx = (color >> 4) & 0x02; 
			INT32 flipy = (color >> 4) & 0x01;					
			
			INT32 x = mx << 3;
			INT32 y = my << 3;		
			
			// stuff from 192 to 256 does not scroll
			if ((x >> 3) < 24) {
				y -= DrvColumnScroll;
				if (y < -7) y += 256;
			}			
			
			y -= 16;

			if (flipy) {
				if (flipx) {
					Render8x8Tile_FlipXY_Clip(pTransDraw, code, x, y, color, 2, 0, DrvGfxROM0);
				} else {
					Render8x8Tile_FlipY_Clip(pTransDraw, code, x, y, color, 2, 0, DrvGfxROM0);
				}
			} else {
				if (flipx) {
					Render8x8Tile_FlipX_Clip(pTransDraw, code, x, y, color, 2, 0, DrvGfxROM0);
				} else {
					Render8x8Tile_Clip(pTransDraw, code, x, y, color, 2, 0, DrvGfxROM0);
				}
			}	
		}
	}
}


static void RenderSpringerBg()
{	
	INT32 TileIndex = 0;

	for (INT32 my = 0; my < 32; my++) {		
		for (INT32 mx = 0; mx < 32; mx++) {	
			
			TileIndex = (my * 32) + mx; 
			
			INT32 code = DrvVidRAM[TileIndex];			
		    INT32 color = DrvColRAM[TileIndex];
			
			code |= ((color & 0xc0) << 2);
			
			color &= 0x0f;
			color |= DrvPaletteBank << 4;
			
			INT32 flipx = (color >> 4) & 0x02; 
			INT32 flipy = (color >> 4) & 0x01;					
			
			INT32 x = mx << 3;				
			INT32 y = my << 3;

			y -= 16; // remove garbage on left side

			if (flipy) {
				if (flipx) {
					Render8x8Tile_FlipXY_Clip(pTransDraw, code, x, y, color, 2, 0, DrvGfxROM0);
				} else {
					Render8x8Tile_FlipY_Clip(pTransDraw, code, x, y, color, 2, 0, DrvGfxROM0);
				}
			} else {
				if (flipx) {
					Render8x8Tile_FlipX_Clip(pTransDraw, code, x, y, color, 2, 0, DrvGfxROM0);
				} else {
					Render8x8Tile_Clip(pTransDraw, code, x, y, color, 2, 0, DrvGfxROM0);
				}
			}	
		}
	}
}

static void MarinebDrawSprites()
{
	// Render Tiles
	RenderMarinebBg();	
	
	for (INT32 offs = 0x0f; offs >= 0; offs--) {
	
		INT32 gfx, sx, sy, code, color, flipx, flipy, offs2;

		if ((offs == 0) || (offs == 2))
			continue; 

		if (offs < 8) {
			offs2 = 0x0018 + offs;
		} else {		
			offs2 = 0x03d8 - 8 + offs;
		}
		
		code = DrvVidRAM[offs2];
		sx = DrvVidRAM[offs2 + 0x20];
		sy = DrvColRAM[offs2];
		color = (DrvColRAM[offs2 + 0x20] & 0x0f) + 16 * DrvPaletteBank;
		flipx = code & 0x02;
		flipy = !(code & 0x01);

		if (offs < 4) {				
			gfx = 2;
			code = (code >> 4) | ((code & 0x0c) << 2);
		} else {			
			gfx = 1;
			code >>= 2;
		}

		if (!DrvFlipScreenY) {
				
			if (gfx == 1) {
				sy = 256 - 16 - sy;
			} else {
				sy = 256 - 32 - sy;
			}			
			flipy = !flipy;
		}

		if (DrvFlipScreenX) {		
			sx++;
		}
		
		sy -= 16; // proper alignement
		
		// Small Sprites
		if (gfx == 1) {
			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				}
			}
		// Big sprites
		} else {
			if (flipy) {
				if (flipx) {
					Render32x32Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
				} else {
					Render32x32Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
				}
			} else {
				if (flipx) {
					Render32x32Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
				} else {
					Render32x32Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
				}
			}		
		}		
	}
}

static void SpringerDrawSprites()
{
	// Render Tiles
	RenderSpringerBg();	
	
	for (INT32 offs = 0x0f; offs >= 0; offs--)
	{
		INT32 gfx, sx, sy, code, color, flipx, flipy, offs2;

		if ((offs == 0) || (offs == 2))
			continue;  

		offs2 = 0x0010 + offs;

		code = DrvVidRAM[offs2];
		sx = 240 - DrvVidRAM[offs2 + 0x20];
		sy = DrvColRAM[offs2];
		color = (DrvColRAM[offs2 + 0x20] & 0x0f) + 16 * DrvPaletteBank;
		flipx = !(code & 0x02);
		flipy = !(code & 0x01);

		if (offs < 4)
		{			
			sx -= 0x10;
			gfx = 2;
			code = (code >> 4) | ((code & 0x0c) << 2);
		}
		else
		{			
			gfx = 1;
			code >>= 2;
		}		
		
		if (!DrvFlipScreenY) {
				
			if (gfx == 1) {
				sy = 256 - 16 - sy;
			} else {
				sy = 256 - 32 - sy;
			}			
			flipy = !flipy;
		}

		if (!DrvFlipScreenX)
		{
			sx--;
		}
		
		sy -= 16; // proper alignement
		
		// Small Sprites
		if (gfx == 1) {
			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM1);
				}
			}
		// Big Sprites
		} else {
			if (flipy) {
				if (flipx) {
					Render32x32Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
				} else {
					Render32x32Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
				}
			} else {
				if (flipx) {
					Render32x32Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
				} else {
					Render32x32Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM2);
				}
			}		
		}		
	}
}

static INT32 DrvDraw()
{	
	if (DrvRecalcPalette) {
		for (INT32 i = 0; i < 256; i++) {
			UINT32 tmp = TempPalette[i];
			// Recalc Colors 
			DrvPalette[i] = BurnHighCol((tmp >> 16)  & 0xFF, (tmp >> 8) & 0xFF, tmp & 0xFF, 0);			
		}
		DrvRecalcPalette = 0;
	}
	
	switch(hardware) {
		case MARINEB:			
			MarinebDrawSprites();			
			break;
			
		case SPRINGER:			
			SpringerDrawSprites();
			break;			
	}
	
	BurnTransferCopy(DrvPalette);	
	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) { 
		DrvDoReset(); 
	}

	DrvInput[0] = DrvInput[1] = DrvInput[2] = 0;
	
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvInputPort0[i] & 1) << i; 
		DrvInput[1] |= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] |= (DrvInputPort2[i] & 1) << i;
	}

	ZetOpen(0);
	ZetRun(3072000 / 60);	
	
	if (DrvInterruptEnable) ZetNmi();		
	ZetClose();
	
	if (pBurnSoundOut) {
		INT32 nSample;
		AY8910Update(0, &pAY8910Buffer[0], nBurnSoundLen);
		for (INT32 n = 0; n < nBurnSoundLen; n++) {
			nSample  = pAY8910Buffer[0][n] >> 2;
			nSample += pAY8910Buffer[1][n] >> 2;
			nSample += pAY8910Buffer[2][n] >> 2;

			nSample /= 4;

			nSample = BURN_SND_CLIP(nSample);

			pBurnSoundOut[(n << 1) + 0] = nSample;
			pBurnSoundOut[(n << 1) + 1] = nSample;
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
		*pnMin = 0x029708;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd - RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		AY8910Scan(nAction, pnMin);
	}

	
	return 0;
}

static INT32 MarinebInit()
{	
	CleanAndInitStuff();
	hardware = MARINEB;	
	
	return DrvInit();
}

static INT32 SpringerInit()
{
	CleanAndInitStuff();
	
	ActiveLowFlipscreen = 1;
	hardware = SPRINGER;	
	
	return DrvInit();
}

//  Marine Boy

static struct BurnRomInfo MarinebRomDesc[] = {

	{ "marineb.1", 0x1000, 0x661d6540, BRF_ESS | BRF_PRG }, // 0 maincpu
	{ "marineb.2", 0x1000, 0x922da17f, BRF_ESS | BRF_PRG }, // 1
	{ "marineb.3", 0x1000, 0x820a235b, BRF_ESS | BRF_PRG }, // 2
	{ "marineb.4", 0x1000, 0xa157a283, BRF_ESS | BRF_PRG }, // 3
	{ "marineb.5", 0x1000, 0x9ffff9c0, BRF_ESS | BRF_PRG }, // 4

	{ "marineb.6", 0x2000, 0xee53ec2e, BRF_GRA }, // 5 gfx1

	{ "marineb.8", 0x2000, 0xdc8bc46c, BRF_GRA }, // 6 gfx2
	{ "marineb.7", 0x2000, 0x9d2e19ab, BRF_GRA }, // 7

	{ "marineb.1b", 0x100, 0xf32d9472, BRF_GRA }, // 8 proms
	{ "marineb.1c", 0x100, 0x93c69d3e, BRF_GRA }, // 9 
};

STD_ROM_PICK(Marineb)
STD_ROM_FN(Marineb)

struct BurnDriver BurnDrvMarineb = {
	"marineb", NULL, NULL, NULL, "1982",
	"Marine Boy\0", NULL, "Orca", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, 0, 0,
	NULL, MarinebRomInfo, MarinebRomName, NULL, NULL, MarinebInputInfo, MarinebDIPInfo,
	MarinebInit, DrvExit, DrvFrame, DrvDraw, DrvScan,	
	&DrvRecalcPalette, 0x100, 256, 224, 4, 3
};

//  Springer

static struct BurnRomInfo SpringerRomDesc[] = {

	{ "springer.1", 0x1000, 0x0794103a, BRF_ESS | BRF_PRG }, // 0 maincpu
	{ "springer.2", 0x1000, 0xf4aecd9a, BRF_ESS | BRF_PRG }, // 1
	{ "springer.3", 0x1000, 0x2f452371, BRF_ESS | BRF_PRG }, // 2
	{ "springer.4", 0x1000, 0x859d1bf5, BRF_ESS | BRF_PRG }, // 3
	{ "springer.5", 0x1000, 0x72adbbe3, BRF_ESS | BRF_PRG }, // 4

	{ "springer.6", 0x1000, 0x6a961833, BRF_GRA }, // 5 gfx1
	{ "springer.7", 0x1000, 0x95ab8fc0, BRF_GRA }, // 6

	{ "springer.8", 0x1000, 0xa54bafdc, BRF_GRA }, // 7 gfx2
	{ "springer.9", 0x1000, 0xfa302775, BRF_GRA }, // 8

	{ "1b.vid", 0x100, 0xa2f935aa, BRF_GRA }, // 9 proms
	{ "1c.vid", 0x100, 0xb95421f4, BRF_GRA }, // 10
};

STD_ROM_PICK(Springer)
STD_ROM_FN(Springer)

struct BurnDriver BurnDrvSpringer = {
	"springer", NULL, NULL, NULL, "1982",
	"Springer\0", NULL, "Orca", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_PRE90S, 0, 0,
	NULL, SpringerRomInfo, SpringerRomName, NULL, NULL, MarinebInputInfo, MarinebDIPInfo,
	SpringerInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalcPalette, 0x100, 224, 256, 3, 4
};

