// FB Alpha Eggs & Dommy driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "m6502_intf.h"
#include "bitswap.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv6502ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvColPROM;
static UINT8 *DrvVidRAM;
static UINT8 *Drv6502RAM;
static UINT8 *DrvColRAM;

static INT16 *pAY8910Buffer[6];
static INT16 *pFMBuffer = NULL;

static UINT32 *DrvPalette;
static UINT8  DrvRecalc;

static UINT8 *flipscreen;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[2];
static UINT8 DrvReset;

static INT32 vblank = 0;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy1 + 6,	"p1 coin"  },
	{"P1 start"  ,    BIT_DIGITAL  , DrvJoy2 + 6,	"p1 start" },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 right" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 1, 	"p1 left"  },
	{"P1 Up",	  BIT_DIGITAL,   DrvJoy1 + 2,   "p1 up",   },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy1 + 3,   "p1 down", },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	"p1 fire 1"},

	{"P2 Coin"      , BIT_DIGITAL  , DrvJoy1 + 7,	"p2 coin"  },
	{"P2 start"  ,    BIT_DIGITAL  , DrvJoy2 + 7,	"p2 start" },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 0, 	"p2 right" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 1, 	"p2 left"  },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy2 + 2,   "p2 up",   },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy2 + 3,   "p2 down", },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p2 fire 1"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip 1"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip 2"	   },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	{0x0f, 0xff, 0xff, 0x7f, NULL                     },

	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x0f, 0x01, 0x03, 0x00, "2C 1C"     		  },
	{0x0f, 0x01, 0x03, 0x03, "1C 1C"    		  },
	{0x0f, 0x01, 0x03, 0x01, "1C 2C"     		  },
	{0x0f, 0x01, 0x03, 0x02, "1C 3C"    		  },

	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x0f, 0x01, 0x0c, 0x00, "2C 1C"     		  },
	{0x0f, 0x01, 0x0c, 0x0c, "1C 3C"    		  },
	{0x0f, 0x01, 0x0c, 0x04, "1C 2C"     		  },
	{0x0f, 0x01, 0x0c, 0x08, "1C 3C"    		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x0f, 0x01, 0x40, 0x40, "Cocktail"       	  },
	{0x0f, 0x01, 0x40, 0x00, "Upright"     		  },

	{0x10, 0xff, 0xff, 0xff, NULL                     },

	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x10, 0x01, 0x01, 0x01, "3"     		  },
	{0x10, 0x01, 0x01, 0x00, "5"    		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x10, 0x01, 0x06, 0x04, "30000"     		  },
	{0x10, 0x01, 0x06, 0x02, "50000"    		  },
	{0x10, 0x01, 0x06, 0x06, "70000"     		  },
	{0x10, 0x01, 0x06, 0x00, "Never"    		  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x10, 0x01, 0x80, 0x80, "Easy"     		  },
	{0x10, 0x01, 0x80, 0x00, "Hard"    		  },
};

STDDIPINFO(Drv)

// Delay allocation of buffer. This allows us to use the proper frames/sec frame rate.
static inline void DrvDelayAY8910BufferAllocation()
{
	if (pFMBuffer == NULL && pBurnSoundOut)
	{
		pFMBuffer = (INT16*)BurnMalloc(nBurnSoundLen * sizeof(INT16) * 6);

		pAY8910Buffer[0] = pFMBuffer + nBurnSoundLen * 0;
		pAY8910Buffer[1] = pFMBuffer + nBurnSoundLen * 1;
		pAY8910Buffer[2] = pFMBuffer + nBurnSoundLen * 2;
		pAY8910Buffer[3] = pFMBuffer + nBurnSoundLen * 3;
		pAY8910Buffer[4] = pFMBuffer + nBurnSoundLen * 4;
		pAY8910Buffer[5] = pFMBuffer + nBurnSoundLen * 5;
	}
}

static inline INT32 calc_mirror_offset(UINT16 address)
{
	return ((address >> 5) & 0x1f) + ((address & 0x1f) << 5);
}

static UINT8 eggs_read(UINT16 address)
{
	if ((address & 0xf800) == 0x1800) {
    		return DrvVidRAM[calc_mirror_offset(address)];
	}

	if ((address & 0xf800) == 0x1c00) {
    		return DrvColRAM[calc_mirror_offset(address)];
	}

	switch (address)
	{
		case 0x2000:
			return DrvDips[0] | vblank;

		case 0x2001:
			return DrvDips[1];

		case 0x2002:
			return DrvInputs[0];

		case 0x2003:
			return DrvInputs[1];
	}

	return 0;
}

void eggs_write(UINT16 address, UINT8 data)
{
	if ((address & 0xfc00) == 0x1800) {
    		DrvVidRAM[calc_mirror_offset(address)] = data;
		return;
	}

	if ((address & 0xfc00) == 0x1c00) {
    		DrvColRAM[calc_mirror_offset(address)] = data;
		return;
	}

	switch (address)
	{
		case 0x2000:
			*flipscreen = data & 1;
		break;

		case 0x2001:
			M6502SetIRQLine(M6502_IRQ_LINE, M6502_IRQSTATUS_NONE);
		break;

		case 0x2004:
		case 0x2005:
		case 0x2006:
		case 0x2007:
			AY8910Write((address >> 1) & 1, address & 1, data);
		break;
	}
}

static UINT8 dommy_read(UINT16 address)
{
	if ((address & 0xfc00) == 0x2800) {
    		return DrvVidRAM[calc_mirror_offset(address)];
	}

	switch (address)
	{
		case 0x4000:
			return DrvDips[0] | vblank;

		case 0x4001:
			return DrvDips[1];

		case 0x4002:
			return DrvInputs[0];

		case 0x4003:
			return DrvInputs[1];
	}

	return 0;
}

static void dommy_write(UINT16 address, UINT8 data)
{
	if ((address & 0xfc00) == 0x2800) {
    		DrvVidRAM[calc_mirror_offset(address)] = data;
		return;
	}

	switch (address)
	{
		case 0x4000:
			M6502SetIRQLine(M6502_IRQ_LINE, M6502_IRQSTATUS_NONE);
		break;

		case 0x4001:
			*flipscreen = data & 1;
		break;

		case 0x4004:
		case 0x4005:
		case 0x4006:
		case 0x4007:
			AY8910Write((address >> 1) & 1, address & 1, data);
		break;
	}
}

static INT32 DrvDoReset()
{
	memset(AllRam, 0, RamEnd - AllRam);

	M6502Open(0);
	M6502Reset();
	M6502Close();

	AY8910Reset(0);
	AY8910Reset(1);

	return 0;
}

static INT32 DrvGfxDecode()
{
	static INT32 Planes[3] = { 0x20000, 0x10000, 0 };
	static INT32 XOffs[16] = { STEP8(128,1), STEP8(0,1) };
	static INT32 YOffs[16] = { STEP16(0, 8) };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x6000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x6000);

	GfxDecode(0x400, 3,  8,  8, Planes, XOffs + 8, YOffs, 0x040, tmp, DrvGfxROM0);
	GfxDecode(0x100, 3, 16, 16, Planes, XOffs + 0, YOffs, 0x100, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0;i < 8;i++)
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

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void scregg6502Init()
{
	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(Drv6502RAM,          0x0000, 0x07ff, M6502_RAM);
	M6502MapMemory(DrvVidRAM,           0x1000, 0x13ff, M6502_RAM);
	M6502MapMemory(DrvColRAM,           0x1400, 0x17ff, M6502_RAM);
	M6502MapMemory(Drv6502ROM + 0x3000, 0x3000, 0x7fff, M6502_ROM);
	M6502MapMemory(Drv6502ROM + 0x7000, 0xf000, 0xffff, M6502_ROM);
	M6502SetWriteMemIndexHandler(eggs_write);
	M6502SetReadMemIndexHandler(eggs_read);
	M6502SetReadOpArgHandler(eggs_read);
	M6502SetWriteHandler(eggs_write);
	M6502SetReadOpHandler(eggs_read);
	M6502SetReadHandler(eggs_read);
	M6502Close();
}

static void dommy6502Init()
{
	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(Drv6502RAM,          0x0000, 0x07ff, M6502_RAM);
	M6502MapMemory(DrvVidRAM,           0x2000, 0x23ff, M6502_RAM);
	M6502MapMemory(DrvColRAM,           0x2400, 0x27ff, M6502_RAM);
	M6502MapMemory(Drv6502ROM + 0xa000, 0xa000, 0xffff, M6502_ROM);
	M6502SetWriteMemIndexHandler(dommy_write);
	M6502SetReadMemIndexHandler(dommy_read);
	M6502SetReadOpArgHandler(dommy_read);
	M6502SetWriteHandler(dommy_write);
	M6502SetReadOpHandler(dommy_read);
	M6502SetReadHandler(dommy_read);
	M6502Close();
}

static INT32 rockduckLoadRoms()
{
	if (BurnLoadRom(Drv6502ROM + 0x4000,  0, 1)) return 1;
	if (BurnLoadRom(Drv6502ROM + 0x6000,  1, 1)) return 1;
	if (BurnLoadRom(Drv6502ROM + 0x8000,  2, 1)) return 1;
	memcpy (Drv6502ROM + 0x3000, Drv6502ROM + 0x5000, 0x1000);
	memcpy (Drv6502ROM + 0x5000, Drv6502ROM + 0x7000, 0x1000);
	memcpy (Drv6502ROM + 0xe000, Drv6502ROM + 0x8000, 0x2000);
	memcpy (Drv6502ROM + 0x7000, Drv6502ROM + 0x9000, 0x1000);

	if (BurnLoadRom(DrvGfxROM0 + 0x0000,  3, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x2000,  4, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x4000,  5, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x0000,  6, 1)) return 1;

	for (INT32 i = 0x2000; i < 0x6000; i++)
		DrvGfxROM0[i] = BITSWAP08(DrvGfxROM0[i],2,0,3,6,1,4,7,5);

	return 0;
}

static INT32 screggLoadRoms()
{
	if (BurnLoadRom(Drv6502ROM + 0x3000,  0, 1)) return 1;
	if (BurnLoadRom(Drv6502ROM + 0x4000,  1, 1)) return 1;
	if (BurnLoadRom(Drv6502ROM + 0x5000,  2, 1)) return 1;
	if (BurnLoadRom(Drv6502ROM + 0x6000,  3, 1)) return 1;
	if (BurnLoadRom(Drv6502ROM + 0x7000,  4, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x0000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x1000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x2000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x3000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x4000,  9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x5000, 10, 1)) return 1;

	if (BurnLoadRom(DrvColPROM, 11, 1)) return 1;

	return 0;
}

static INT32 dommyLoadRoms()
{
	if (BurnLoadRom(Drv6502ROM + 0xa000,  0, 1)) return 1;
	if (BurnLoadRom(Drv6502ROM + 0xc000,  1, 1)) return 1;
	if (BurnLoadRom(Drv6502ROM + 0xe000,  2, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x0000,  3, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x2000,  4, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x4000,  5, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x0000,  6, 1)) return 1;

	memcpy (DrvColPROM, DrvColPROM + 0x08, 8);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv6502ROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x010000;
	DrvGfxROM1	= Next; Next += 0x010000;

	DrvColPROM	= Next; Next += 0x000020;

	DrvPalette	= (UINT32*)Next; Next += 0x0008 * sizeof(UINT32);

	AllRam		= Next;

	Drv6502RAM	= Next; Next += 0x000800;
	DrvVidRAM	= Next; Next += 0x000400;
	DrvColRAM	= Next; Next += 0x000400;

	flipscreen 	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvInit(void (*m6502Init)(), INT32 (*pLoadCB)())
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (pLoadCB) {
		if (pLoadCB()) return 1;

		DrvGfxDecode();
		DrvPaletteInit();
	}

	if (m6502Init) {
		m6502Init();
	}

	BurnSetRefreshRate(57);

	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910SetAllRoutes(0, 0.23, BURN_SND_ROUTE_BOTH);
	AY8910SetAllRoutes(1, 0.23, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	M6502Exit();
	AY8910Exit(0);
	AY8910Exit(1);

	BurnFree (AllMem);
	BurnFree (pFMBuffer);

	return 0;
}

static void draw_layer()
{
	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 sx = (~offs >> 2) & 0xf8;
		INT32 sy = ( offs & 0x1f) << 3;

        	UINT16 code = DrvVidRAM[offs] | ((DrvColRAM[offs] & 3) << 8);

		if (*flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, (sx ^ 0xf8) - 8, (sy ^ 0xf8) - 8, 0, 3, 0, DrvGfxROM0);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx - 8, sy - 8, 0, 3, 0, DrvGfxROM0);
		}
	}
}

static void draw_sprites()
{
	for (INT32 i = 0, offs = 0; i < 8; i++, offs += 0x80)
	{
		if (~DrvVidRAM[offs] & 0x01) continue;

		INT32 sx = 240 - DrvVidRAM[offs + 0x60];
		INT32 sy = 240 - DrvVidRAM[offs + 0x40];

		INT32 flipx = DrvVidRAM[offs] & 0x04;
		INT32 flipy = DrvVidRAM[offs] & 0x02;

		INT32 code = DrvVidRAM[offs + 0x20];

		if (*flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;

			flipx = !flipx;
			flipy = !flipy;
		}

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx - 8, sy - 8, 0, 3, 0, 0, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx - 8, sy - 8, 0, 3, 0, 0, DrvGfxROM1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx - 8, sy - 8, 0, 3, 0, 0, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx - 8, sy - 8, 0, 3, 0, 0, DrvGfxROM1);
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

	draw_layer();
	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	DrvDelayAY8910BufferAllocation();

	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 2 * sizeof(UINT8));

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nInterleave = 32;
	INT32 nTotalCycles = 1500000 / 57;
	INT32 nCyclesRun = 0;

	M6502Open(0);

	vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++) {
		nCyclesRun += M6502Run(nTotalCycles / nInterleave);
		M6502SetIRQLine(M6502_IRQ_LINE, (i & 1) ? M6502_IRQSTATUS_ACK : M6502_IRQSTATUS_NONE);

		if (i == 28) vblank = 0x80;
	}

	M6502Close();

	if (pBurnSoundOut) {
		AY8910Render(&pAY8910Buffer[0], pBurnSoundOut, nBurnSoundLen, 0);
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

		M6502Scan(nAction);
		AY8910Scan(nAction, pnMin);
	}

	return 0;
}


// Dommy

static struct BurnRomInfo dommyRomDesc[] = {
	{ "dommy.e01",  0x2000, 0x9ae064ed, 1 | BRF_ESS | BRF_PRG }, //  0 M6502 Code
	{ "dommy.e11",  0x2000, 0x7c4fad5c, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "dommy.e21",  0x2000, 0xcd1a4d55, 1 | BRF_ESS | BRF_PRG }, //  2

	{ "dommy.e50",  0x2000, 0x5e9db0a4, 2 | BRF_GRA },	     //  3 Graphics
	{ "dommy.e40",  0x2000, 0x4d1c36fb, 2 | BRF_GRA },	     //  4
	{ "dommy.e30",  0x2000, 0x4e68bb12, 2 | BRF_GRA },	     //  5

	{ "dommy.e70",  0x0020, 0x50c1d86e, 3 | BRF_GRA },	     //  6 Palette

	{ "dommy.e60",  0x0020, 0x24da2b63, 4 | BRF_OPT },	     //  7
};

STD_ROM_PICK(dommy)
STD_ROM_FN(dommy)

static INT32 dommyInit()
{
	return DrvInit(dommy6502Init, dommyLoadRoms);
}

struct BurnDriver BurnDrvdommy = {
	"dommy", NULL, NULL, NULL, "198?",
	"Dommy\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TECHNOS, GBF_MAZE, 0,
	NULL, dommyRomInfo, dommyRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	dommyInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x08,
	240, 248, 3, 4
};


// Scrambled Egg

static struct BurnRomInfo screggRomDesc[] = {
	{ "d00.e14",      0x1000, 0x29226d77, 1 | BRF_ESS | BRF_PRG }, //  0 M6502 Code
	{ "d10.d14",      0x1000, 0xeb143880, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "d20.c14",      0x1000, 0x4455f262, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "d30.b14",      0x1000, 0x044ac5d2, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "d40.a14",      0x1000, 0xb5a0814a, 1 | BRF_ESS | BRF_PRG }, //  4

	{ "d50.j12",      0x1000, 0xa485c10c, 2 | BRF_GRA },	       //  5 Graphics
	{ "d60.j10",      0x1000, 0x1fd4e539, 2 | BRF_GRA },	       //  6
	{ "d70.h12",      0x1000, 0x8454f4b2, 2 | BRF_GRA },	       //  7
	{ "d80.h10",      0x1000, 0x72bd89ee, 2 | BRF_GRA },	       //  8
	{ "d90.g12",      0x1000, 0xff3c2894, 2 | BRF_GRA },	       //  9
	{ "da0.g10",      0x1000, 0x9c20214a, 2 | BRF_GRA },	       // 10

	{ "dc0.c6 ",      0x0020, 0xff23bdd6, 3 | BRF_GRA },	       // 11 Palette

	{ "db1.b4",       0x0020, 0x7cc4824b, 0 | BRF_OPT },	       // 12
};

STD_ROM_PICK(scregg)
STD_ROM_FN(scregg)

static INT32 screggInit()
{
	return DrvInit(scregg6502Init, screggLoadRoms);
}

struct BurnDriver BurnDrvscregg = {
	"scregg", NULL, NULL, NULL, "1983",
	"Scrambled Egg\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TECHNOS, GBF_MAZE, 0,
	NULL, screggRomInfo, screggRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	screggInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x08,
	240, 240, 3, 4
};


// Eggs

static struct BurnRomInfo eggsRomDesc[] = {
	{ "e14.bin",      0x1000, 0x4e216f9d, 1 | BRF_ESS | BRF_PRG }, //  0 M6502 Code
	{ "d14.bin",      0x1000, 0x4edb267f, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "c14.bin",      0x1000, 0x15a5c48c, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "b14.bin",      0x1000, 0x5c11c00e, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "a14.bin",      0x1000, 0x953faf07, 1 | BRF_ESS | BRF_PRG }, //  4

	{ "j12.bin",      0x1000, 0xce4a2e46, 2 | BRF_GRA },	       //  5 Graphics
	{ "j10.bin",      0x1000, 0xa1bcaffc, 2 | BRF_GRA },	       //  6
	{ "h12.bin",      0x1000, 0x9562836d, 2 | BRF_GRA },	       //  7
	{ "h10.bin",      0x1000, 0x3cfb3a8e, 2 | BRF_GRA },	       //  8
	{ "g12.bin",      0x1000, 0x679f8af7, 2 | BRF_GRA },	       //  9
	{ "g10.bin",      0x1000, 0x5b58d3b5, 2 | BRF_GRA },	       // 10

	{ "eggs.c6",      0x0020, 0xe8408c81, 3 | BRF_GRA },	       // 11 Palette

	{ "db1.b4",       0x0020, 0x7cc4824b, 0 | BRF_OPT },	       // 12
};

STD_ROM_PICK(eggs)
STD_ROM_FN(eggs)

struct BurnDriver BurnDrveggs = {
	"eggs", "scregg", NULL, NULL, "1983",
	"Eggs\0", NULL, "[Technos] Universal USA", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TECHNOS, GBF_MAZE, 0,
	NULL, eggsRomInfo, eggsRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	screggInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x08,
	240, 240, 3, 4
};


// Rock Duck (prototype?)

static struct BurnRomInfo rockduckRomDesc[] = {
	{ "rde.bin",	  0x2000, 0x56e2a030, 1 | BRF_ESS | BRF_PRG }, //  0 M6502 Code
	{ "rdc.bin",	  0x2000, 0x482d9a0c, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "rdb.bin",	  0x2000, 0x974626f2, 1 | BRF_ESS | BRF_PRG }, //  2

	{ "rd3.rdg",	  0x2000, 0x8a3f1e53, 2 | BRF_GRA },	       //  3 Graphics
	{ "rd2.rdh",	  0x2000, 0xe94e673e, 2 | BRF_GRA },	       //  4
	{ "rd1.rdj",	  0x2000, 0x654afff2, 2 | BRF_GRA },	       //  5

	{ "eggs.c6",      0x0020, 0xe8408c81, 3 | BRF_GRA },           //  6 Palette 
};

STD_ROM_PICK(rockduck)
STD_ROM_FN(rockduck)

static INT32 rockduckInit()
{
	return DrvInit(scregg6502Init, rockduckLoadRoms);
}

struct BurnDriver BurnDrvrockduck = {
	"rockduck", NULL, NULL, NULL, "1983",
	"Rock Duck (prototype?)\0", "incorrect colors", "Datel SAS", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TECHNOS, GBF_MAZE, 0,
	NULL, rockduckRomInfo, rockduckRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	rockduckInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x08,
	240, 240, 3, 4
};
