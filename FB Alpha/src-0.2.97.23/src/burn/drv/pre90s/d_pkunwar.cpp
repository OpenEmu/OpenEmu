// FB Alpha Penguin-Kun Wars Driver Module
// Based on MAME Driver by David Haywood and Phil Stroffolino

#include "tiles_generic.h"
#include "driver.h"
extern "C" {
 #include "ay8910.h"
}


static UINT8 DrvJoy1[8], DrvJoy2[8], DrvDips, DrvReset;
static INT16 *pAY8910Buffer[6], *pFMBuffer = NULL;
static UINT8 *Mem, *Rom, *Gfx0, *Gfx1;
static INT32 *Palette;

static INT32 flipscreen, vblank;


//-------------------------------------------------------------------------------------------------
// Input Handlers


static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy2 + 7,	"p1 coin"  },
	{"P1 start"  ,    BIT_DIGITAL  , DrvJoy1 + 5,	"p1 start" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 1, 	"p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 2,	"p1 fire 1"},

	{"P2 start"  ,    BIT_DIGITAL  , DrvJoy2 + 5,	"p2 start" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 0, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 1, 	"p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 2,	"p2 fire 1"},

	{"Service Mode",  BIT_DIGITAL,   DrvJoy2 + 6,   "diag"     },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, &DrvDips ,	"dip"	   },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xfb, NULL 			},

	{0   , 0xfe, 0   , 4   , "Coinage"		},
	{0x0b, 0x01, 0x03, 0x00, "3C 1C"		},
	{0x0b, 0x01, 0x03, 0x02, "2C 1C"		},
	{0x0b, 0x01, 0x03, 0x03, "1C 1C"		},
	{0x0b, 0x01, 0x03, 0x01, "1C 2C"		},

	{0   , 0xfe, 0   , 2   , "Cabinet"		},
	{0x0b, 0x01, 0x04, 0x00, "Upright"		},
	{0x0b, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   , 2   , "Demo Sounds"		},
	{0x0b, 0x01, 0x08, 0x00, "Off"			},
	{0x0b, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   , 4   , "Difficulty"		},
	{0x0b, 0x01, 0x30, 0x10, "Easy"			},
	{0x0b, 0x01, 0x30, 0x30, "Medium"		},
	{0x0b, 0x01, 0x30, 0x20, "Hard"			},
	{0x0b, 0x01, 0x30, 0x00, "Hardest"		},

	{0   , 0xfe, 0   , 2   , "Flip screen"		},
	{0x0b, 0x01, 0x40, 0x40, "Off"			},
	{0x0b, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   , 2   , "Free Play"		},
	{0x0b, 0x01, 0x80, 0x80, "Off"			},
	{0x0b, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Drv)


//-------------------------------------------------------------------------------------------------
// Memory Handlers


UINT8 __fastcall pkunwar_read(UINT16 address)
{
	switch (address)
	{
		case 0xa001:
			return AY8910Read(0);
		break;

		case 0xa003:
			return AY8910Read(1);
		break;
	}

	return 0;
}

void __fastcall pkunwar_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xa000:
			AY8910Write(0, 0, data);
		break;

		case 0xa001:
			AY8910Write(0, 1, data);
		break;

		case 0xa002:
			AY8910Write(1, 0, data);
		break;

		case 0xa003:
			AY8910Write(1, 1, data);
		break;
	}
}

void __fastcall pkunwar_out(UINT16 address, UINT8 data)
{
	address &= 0xff;

	if (address == 0) flipscreen = data & 1;
}


//-------------------------------------------------------------------------------------------------
// AY8910 Ports


static UINT8 pkunwar_port_0(UINT32)
{
	UINT8 ret = 0x7f | (vblank ^= 0x80);

	for (INT32 i = 0; i < 8; i++) {
		ret ^= DrvJoy1[i] << i;
	}

	return ret;
}

static UINT8 pkunwar_port_1(UINT32)
{
	UINT8 ret = 0xff;

	for (INT32 i = 0; i < 8; i++) {
		ret ^= DrvJoy2[i] << i;
	}

	return ret;
}

static UINT8 pkunwar_port_2(UINT32)
{
	return 0xff;
}

static UINT8 pkunwar_port_3(UINT32)
{
	return DrvDips;
}


//-------------------------------------------------------------------------------------------------
// Initialization Routines


static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (Rom + 0x8000, 0, 0x1000);
	memset (Rom + 0xc000, 0, 0x0800);

	flipscreen = 0;

	ZetOpen(0);
	ZetReset();
	ZetClose();

	AY8910Reset(0);
	AY8910Reset(1);

	return 0;
}

static void pkunwar_palette_init(UINT8 *Prom)
{
	for (INT32 i = 0; i < 0x200; i++)
	{
		INT32 entry;
		INT32 intensity,r,g,b;

		if ((i & 0xf) == 1)
		{
			entry = ((i & 0xf0) >> 4) | ((i & 0x100) >> 4);
		}
		else
		{
			entry = ((i & 0x0f) >> 0) | ((i & 0x100) >> 4);
		}

		intensity = Prom[entry] & 0x03;

		r = (((Prom[entry] >> 0) & 0x0c) | intensity) * 0x11;
		g = (((Prom[entry] >> 2) & 0x0c) | intensity) * 0x11;
		b = (((Prom[entry] >> 4) & 0x0c) | intensity) * 0x11;

		Palette[i] = (r << 16) | (g << 8) | b;
	}
}

static void pkunwar_gfx_decode(UINT8 *Gfx)
{
	static INT32 PlaneOffsets[4] = { 0, 1, 2, 3 };

	static INT32 XOffsets[16] = {
		0x00000, 0x00004, 0x40000, 0x40004, 0x00008, 0x0000c, 0x40008, 0x4000c,
		0x00080, 0x00084, 0x40080, 0x40084, 0x00088, 0x0008c, 0x40088, 0x4008c
	};

	static INT32 YOffsets[16] = {
		0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
		0x100, 0x110, 0x120, 0x130, 0x140, 0x150, 0x160, 0x170
	};

	GfxDecode(0x800, 4,  8,  8, PlaneOffsets, XOffsets, YOffsets, 0x080, Gfx, Gfx0);
	GfxDecode(0x200, 4, 16, 16, PlaneOffsets, XOffsets, YOffsets, 0x200, Gfx, Gfx1);
}

static INT32 LoadRoms()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x10000);
	if (tmp == NULL) {
		return 1;
	}

	if (BurnLoadRom(Rom  + 0x0000, 0, 1)) return 1;
	if (BurnLoadRom(Rom  + 0x4000, 1, 1)) return 1;
	if (BurnLoadRom(Rom  + 0xe000, 2, 1)) return 1;

	if (BurnLoadRom(Gfx0 + 0x0000, 3, 1)) return 1;
	if (BurnLoadRom(Gfx0 + 0x4000, 4, 1)) return 1;
	if (BurnLoadRom(Gfx0 + 0x8000, 5, 1)) return 1;
	if (BurnLoadRom(Gfx0 + 0xc000, 6, 1)) return 1;

	for (INT32 i = 0; i < 8; i++) {
		INT32 j = ((i & 1) << 2) | ((i >> 1) & 3);
		memcpy (tmp + j * 0x2000, Gfx0 + i * 0x2000, 0x2000);
	}

	pkunwar_gfx_decode(tmp);

	if (BurnLoadRom(tmp +  0x0000, 7, 1)) return 1;

	pkunwar_palette_init(tmp);

	BurnFree(tmp);

	return 0;
}

static INT32 DrvInit()
{
	Mem = (UINT8*)BurnMalloc(0x10000 + (0x20000 * 2) + (0x200 * sizeof(INT32)));
	if (Mem == NULL) {
		return 1;
	}

	pFMBuffer = (INT16 *)BurnMalloc (nBurnSoundLen * 6 * sizeof(INT16));
	if (pFMBuffer == NULL) {
		return 1;
	}

	Rom  = Mem + 0x00000;
	Gfx0 = Mem + 0x10000;
	Gfx1 = Mem + 0x30000;
	Palette = (INT32*)(Mem + 0x40000);

	if (LoadRoms()) return 1;

	ZetInit(0);
	ZetOpen(0);
	ZetSetOutHandler(pkunwar_out);
	ZetSetReadHandler(pkunwar_read);
	ZetSetWriteHandler(pkunwar_write);
	ZetMapArea(0x0000, 0x7fff, 0, Rom + 0x00000);
	ZetMapArea(0x0000, 0x7fff, 2, Rom + 0x00000);
	ZetMapArea(0x8000, 0x8fff, 0, Rom + 0x08000);
	ZetMapArea(0x8000, 0x8fff, 1, Rom + 0x08000);
	ZetMapArea(0xc000, 0xc7ff, 0, Rom + 0x0c000);
	ZetMapArea(0xc000, 0xc7ff, 1, Rom + 0x0c000);
	ZetMapArea(0xc000, 0xc7ff, 2, Rom + 0x0c000);
	ZetMapArea(0xe000, 0xffff, 0, Rom + 0x0e000);
	ZetMapArea(0xe000, 0xffff, 2, Rom + 0x0e000);
	ZetMemEnd();
	ZetClose();

	pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
	pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
	pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
	pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
	pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
	pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;

	AY8910Init(0, 1500000, nBurnSoundRate, &pkunwar_port_0, &pkunwar_port_1, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, &pkunwar_port_2, &pkunwar_port_3, NULL, NULL);

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	ZetExit();
	AY8910Exit(0);
	AY8910Exit(1);

	BurnFree (pFMBuffer);
	BurnFree (Mem);

	Rom = Gfx0 = Gfx1 = Mem = NULL;
	Palette = NULL;

	vblank = flipscreen = 0;

	return 0;
}


//-------------------------------------------------------------------------------------------------
// Drawing Routines


static INT32 DrawChars(INT32 priority)
{
	INT32 offs;

	for (offs = 0x3c0 - 1;offs >= 0x40;offs--)
	{
		if (Rom[0x8c00 + offs] & 0x08 || !priority)
		{
			INT32 sx,sy,num,color,pxl,pos;

			sx = (offs & 0x1f) << 3;
			sy = (offs >> 2) & 0xf8;

			if (sy < 32 || sy > 223) continue;
			sy -= 32;

			num = Rom[0x8800 + offs] | ((Rom[0x8c00 + offs] & 7) << 8);
			color = 0x100 | (Rom[0x8c00 + offs] & 0xf0);

			UINT8 *src = Gfx0 + (num << 6);

			for (INT32 y = sy; y < sy + 8; y++)
			{
				for (INT32 x = sx; x < sx + 8; x++, src++)
				{
					if (!*src && priority) continue;

					pxl = Palette[color | *src];

					if (flipscreen)
						pos = ((191 - y) << 8) | (~x & 0xff);
					else
						pos = (y << 8) | (x & 0xff);
	
					PutPix(pBurnDraw + pos * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
	 			}
			}
		}
	}

	return 0;
}

static INT32 DrvDraw()
{
	INT32 offs;

	DrawChars(0);

	for (offs = 0;offs < 0x800;offs += 32)
	{
		INT32 sx,sy,flipx,flipy,num,color,pxl,pos;

		sx = Rom[0x8001 + offs];
		sy = Rom[0x8002 + offs];

		if (sy < 16 || sy > 223) continue;
		sy -= 32;

		flipx = Rom[0x8000 + offs] & 0x01;
		flipy = Rom[0x8000 + offs] & 0x02;

		num = ((Rom[0x8000 + offs] & 0xfc) >> 2) + ((Rom[0x8003 + offs] & 7) << 6);
		color = Rom[0x8003 + offs] & 0xf0;

		UINT8 *src = Gfx1 + (num << 8);

		if (flipy)
		{
			for (INT32 y = sy + 15; y >= sy; y--)
			{
				if (flipx)
				{
					for (INT32 x = sx + 15; x >= sx; x--, src++)
					{
						if (y > 191 || y < 0 || !*src) continue;
		
						pxl = Palette[color | *src];
		
						if (flipscreen)
							pos = ((191 - y) << 8) | (~x & 0xff);
						else
							pos = (y << 8) | (x & 0xff);

						PutPix(pBurnDraw + pos * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
		 			}
				} else {
					for (INT32 x = sx; x < sx + 16; x++, src++)
					{
						if (y > 191 || y < 0 || !*src) continue;
		
						pxl = Palette[color | *src];
		
						if (flipscreen)
							pos = ((191 - y) << 8) | (~x & 0xff);
						else
							pos = (y << 8) | (x & 0xff);
		
						PutPix(pBurnDraw + pos * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
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
						if (y > 191 || y < 0 || !*src) continue;
		
						pxl = Palette[color | *src];
		
						if (flipscreen)
							pos = ((191 - y) << 8) | (~x & 0xff);
						else
							pos = (y << 8) | (x & 0xff);
		
						PutPix(pBurnDraw + pos * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
		 			}
				} else {
					for (INT32 x = sx; x < sx + 16; x++, src++)
					{
						if (y > 191 || y < 0 || !*src) continue;
		
						pxl = Palette[color | *src];
		
						if (flipscreen)
							pos = ((191 - y) << 8) | (~x & 0xff);
						else
							pos = (y << 8) | (x & 0xff);
		
						PutPix(pBurnDraw + pos * nBurnBpp, BurnHighCol(pxl >> 16, pxl >> 8, pxl, 0));
		 			}
				}
			}
		}
	}

	DrawChars(1);

	return 0;
}


static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	vblank = 0;

	ZetOpen(0);
	ZetRun(3000000 / 60);
	ZetRaiseIrq(0);
	ZetClose();

	if (pBurnSoundOut) {
		INT32 nSample;
		INT32 nSegmentLength = nBurnSoundLen;
		INT16* pSoundBuf = pBurnSoundOut;
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


//-------------------------------------------------------------------------------------------------
// Save State


static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029521;
	}

	if (nAction & ACB_VOLATILE) {	
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = Rom + 0x8000;
		ba.nLen	  = 0x1000;
		ba.szName = "Video Ram";
		BurnAcb(&ba);

		ba.Data	  = Rom + 0xc000;
		ba.nLen	  = 0x0800;
		ba.szName = "Work Ram";
		BurnAcb(&ba);

		ZetScan(nAction);

		AY8910Scan(nAction, pnMin);

		SCAN_VAR(flipscreen);
	}

	return 0;
}


//-------------------------------------------------------------------------------------------------
// Game drivers


// Penguin-Kun Wars (US)

static struct BurnRomInfo pkunwarRomDesc[] = {
	{ "pkwar.01r",    0x4000, 0xce2d2c7b, 1 | BRF_PRG | BRF_ESS },	//  0 Z80 Code
	{ "pkwar.02r",    0x4000, 0xabc1f661, 1 | BRF_PRG | BRF_ESS },	//  1
	{ "pkwar.03r",    0x2000, 0x56faebea, 1 | BRF_PRG | BRF_ESS },	//  2

	{ "pkwar.01y",    0x4000, 0x428d3b92, 2 | BRF_GRA },		//  3 Graphics
	{ "pkwar.02y",    0x4000, 0xce1da7bc, 2 | BRF_GRA },		//  4
	{ "pkwar.03y",    0x4000, 0x63204400, 2 | BRF_GRA },		//  5
	{ "pkwar.04y",    0x4000, 0x061dfca8, 2 | BRF_GRA },		//  6

	{ "pkwar.col",    0x0020, 0xaf0fc5e2, 3 | BRF_GRA },		//  7 Color Prom
};

STD_ROM_PICK(pkunwar)
STD_ROM_FN(pkunwar)

struct BurnDriver BurnDrvpkunwar = {
	"pkunwar", NULL, NULL, NULL, "1985",
	"Penguin-Kun Wars (US)\0", NULL, "UPL", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_SPORTSMISC, 0,
	NULL, pkunwarRomInfo, pkunwarRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x200,
	256, 192, 4, 3
};


// Penguin-Kun Wars (Japan)

static struct BurnRomInfo pkunwarjRomDesc[] = {
	{ "pgunwar.6",    0x4000, 0x357f3ef3, 1 | BRF_PRG | BRF_ESS },	//  0 Z80 Code
	{ "pgunwar.5",    0x4000, 0x0092e49e, 1 | BRF_PRG | BRF_ESS },	//  1
	{ "pkwar.03r",    0x2000, 0x56faebea, 1 | BRF_PRG | BRF_ESS },	//  2

	{ "pkwar.01y",    0x4000, 0x428d3b92, 2 | BRF_GRA },		//  3 Graphics
	{ "pkwar.02y",    0x4000, 0xce1da7bc, 2 | BRF_GRA },		//  4
	{ "pgunwar.2",    0x4000, 0xa2a43443, 2 | BRF_GRA },		//  5
	{ "pkwar.04y",    0x4000, 0x061dfca8, 2 | BRF_GRA },		//  6

	{ "pkwar.col",    0x0020, 0xaf0fc5e2, 3 | BRF_GRA },		//  7 Color Prom
};

STD_ROM_PICK(pkunwarj)
STD_ROM_FN(pkunwarj)

struct BurnDriver BurnDrvpkunwarj = {
	"pkunwarj", "pkunwar", NULL, NULL, "1985",
	"Penguin-Kun Wars (Japan)\0", NULL, "UPL", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SPORTSMISC, 0,
	NULL, pkunwarjRomInfo, pkunwarjRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x200,
	256, 192, 4, 3
};

