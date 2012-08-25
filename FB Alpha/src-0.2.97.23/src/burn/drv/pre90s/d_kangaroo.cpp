#include "tiles_generic.h"
#include "z80_intf.h"

#include "driver.h"
extern "C" {
 #include "ay8910.h"
}

static UINT8 *Mem, *MemEnd, *AllRAM, *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvGfxROM;
static UINT32 *DrvPalette;
static UINT32 *Palette;
static UINT8 DrvRecalc;

static UINT32 *DrvVidRAM32;

static INT16* pAY8910Buffer[3];
static INT16 *pFMBuffer;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvDips[2];
static UINT8 DrvReset;

static UINT8 kangaroo_clock;
static UINT8 soundlatch;
static UINT8 *DrvVidControl;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"    ,   BIT_DIGITAL  , DrvJoy1 + 0,	"p1 coin"  },
	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 1,	"p1 start" },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 2, 	"p1 right" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 3, 	"p1 left"  },
	{"P1 Up",	  BIT_DIGITAL,   DrvJoy1 + 4,   "p1 up"    },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy1 + 5,   "p1 down", },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 6,	"p1 fire 1"},

	{"P2 Coin"    ,   BIT_DIGITAL  , DrvJoy2 + 0,	"p2 coin"  },
	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy2 + 1,	"p2 start" },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 2, 	"p2 right" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 3, 	"p2 left"  },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy2 + 4,   "p2 up"    },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy2 + 5,   "p2 down", },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 6,	"p2 fire 1"},

	{"Service Mode",  BIT_DIGITAL,   DrvJoy1 + 7,   "diag"     },
	{"Reset"        , BIT_DIGITAL  ,&DrvReset  ,	"reset"    },
	{"Dip 1"     ,    BIT_DIPSWITCH, DrvDips + 0  ,	"dip"	   },
	{"Dip 2"     ,    BIT_DIPSWITCH, DrvDips + 1  ,	"dip"	   },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x10, 0xff, 0xff, 0x00, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Music"                  },
	{0x10, 0x01, 0x20, 0x20, "Off"       		  },
	{0x10, 0x01, 0x20, 0x00, "On"       		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x10, 0x01, 0x40, 0x00, "Upright"     		  },
	{0x10, 0x01, 0x40, 0x40, "Cocktail"    		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x10, 0x01, 0x80, 0x00, "Off"     		  },
	{0x10, 0x01, 0x80, 0x80, "On"    		  },

	// Default Values
	{0x11, 0xff, 0xff, 0x00, NULL                     },

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Lives"                  },
	{0x11, 0x01, 0x01, 0x00, "3"       		  },
	{0x11, 0x01, 0x01, 0x01, "5"       		  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x11, 0x01, 0x02, 0x00, "Easy"       		  },
	{0x11, 0x01, 0x02, 0x02, "Hard"       		  },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x11, 0x01, 0x0c, 0x08, "10000 30000"       	  },
	{0x11, 0x01, 0x0c, 0x0c, "20000 40000"       	  },
	{0x11, 0x01, 0x0c, 0x04, "10000"             	  },
	{0x11, 0x01, 0x0c, 0x00, "None"			  },

	{0   , 0xfe, 0   , 0x10, "Coinage"		  },
	{0x11, 0x01, 0xf0, 0x10,  "2C_1C"		  },
	{0x11, 0x01, 0xf0, 0x20, "A 2C/1C B 1C/3C"	  },
	{0x11, 0x01, 0xf0, 0x00, "1C_1C"		  },
	{0x11, 0x01, 0xf0, 0x30, "A 1C/1C B 1C/2C"	  },
	{0x11, 0x01, 0xf0, 0x40, "A 1C/1C B 1C/3C"	  },
	{0x11, 0x01, 0xf0, 0x50, "A 1C/1C B 1C/4C"	  },
	{0x11, 0x01, 0xf0, 0x60, "A 1C/1C B 1C/5C"	  },
	{0x11, 0x01, 0xf0, 0x70, "A 1C/1C B 1C/6C"	  },
	{0x11, 0x01, 0xf0, 0x80, "1C_2C"		  },
	{0x11, 0x01, 0xf0, 0x90, "A 1C/2C B 1C/4C"	  },
	{0x11, 0x01, 0xf0, 0xa0, "A 1C/2C B 1C/5C"	  },
	{0x11, 0x01, 0xf0, 0xe0, "A 1C/2C B 1C/6C"	  },
	{0x11, 0x01, 0xf0, 0xb0, "A 1C/2C B 1C/10C"	  },
	{0x11, 0x01, 0xf0, 0xc0, "A 1C/2C B 1C/11C"	  },
	{0x11, 0x01, 0xf0, 0xd0, "A 1C/2C B 1C/12C"	  },
	{0x11, 0x01, 0xf0, 0xf0, "Free_Play"		  },
};

STDDIPINFO(Drv)


static void videoram_write(UINT16 offset, UINT8 data, UINT8 mask)
{
	UINT32 expdata, layermask;

	// data contains 4 2-bit values packed as DCBADCBA; expand these into 4 8-bit values
	expdata = 0;
	if (data & 0x01) expdata |= 0x00000055;
	if (data & 0x10) expdata |= 0x000000aa;
	if (data & 0x02) expdata |= 0x00005500;
	if (data & 0x20) expdata |= 0x0000aa00;
	if (data & 0x04) expdata |= 0x00550000;
	if (data & 0x40) expdata |= 0x00aa0000;
	if (data & 0x08) expdata |= 0x55000000;
	if (data & 0x80) expdata |= 0xaa000000;

	// determine which layers are enabled
	layermask = 0;
	if (mask & 0x08) layermask |= 0x30303030;
	if (mask & 0x04) layermask |= 0xc0c0c0c0;
	if (mask & 0x02) layermask |= 0x03030303;
	if (mask & 0x01) layermask |= 0x0c0c0c0c;

	// update layers
	DrvVidRAM32[offset] = (DrvVidRAM32[offset] & ~layermask) | (expdata & layermask);
}

static void blitter_execute()
{
	UINT16 src = DrvVidControl[0] | (DrvVidControl[1] << 8);
	UINT16 dst = DrvVidControl[2] | (DrvVidControl[3] << 8);
	UINT8 height = DrvVidControl[5];
	UINT8 width = DrvVidControl[4];
	UINT8 mask = DrvVidControl[8];

	// during DMA operations, the top 2 bits are ORed together, as well as the bottom 2 bits
	// adjust the mask to account for this
	mask |= ((mask & 0x41) << 1) | ((mask & 0x82) >> 1);

	// loop over height, then width
	for (INT32 y = 0; y <= height; y++, dst += 256) {
		for (INT32 x = 0; x <= width; x++) {
			UINT16 effdst = (dst + x) & 0x3fff;
			UINT16 effsrc = src++ & 0x1fff;
			videoram_write(effdst, DrvGfxROM[0x0000 + effsrc], mask & 0x05);
			videoram_write(effdst, DrvGfxROM[0x2000 + effsrc], mask & 0x0a);
		}
	}
}

void __fastcall kangaroo_main_write(UINT16 address, UINT8 data)
{
	if (address >= 0x8000 && address <= 0xbfff) {
		videoram_write(address & 0x3fff, data, DrvVidControl[8]);
		return;
	}

	if ((address & 0xec00) == 0xe800) address &= 0xfc0f;
	if ((address & 0xec00) == 0xec00) address &= 0xff00;

	switch (address)
	{
		case 0xe800:
		case 0xe801:
		case 0xe802:
		case 0xe803:
		case 0xe804:
			DrvVidControl[address & 0x0f] = data;
		return;

		case 0xe805:
			DrvVidControl[address & 0x0f] = data;

			blitter_execute();
		return;

		case 0xe806:
		case 0xe807:
			DrvVidControl[address & 0x0f] = data;
		return;

		case 0xe808:
			DrvVidControl[address & 0x0f] = data;

			ZetMapArea(0xc000, 0xdfff, 0, DrvGfxROM + ((data & 0x05) ? 0 : 0x2000));
		return;

		case 0xe809:
		case 0xe80a:
			DrvVidControl[address & 0x0f] = data;
		return;

		case 0xec00:
			soundlatch = data;
		return;

		case 0xed00:
			// coin counter
		return;
	}
	
	
	
	if (address > 0x5fff) bprintf (PRINT_NORMAL, _T("%4.4x, %2.2x wm\n"), address, data);

	return;
}

UINT8 __fastcall kangaroo_main_read(UINT16 address)
{
	if ((address & 0xec00) == 0xe400) address &= 0xfc00;
	if ((address & 0xec00) == 0xec00) address &= 0xff00;

	switch (address)
	{
		case 0xe400:
			return DrvDips[1]; 

		case 0xec00: // in0
		{
			UINT8 ret = DrvDips[0];
			if (DrvJoy1[7]) ret |= 0x01;	// service
			if (DrvJoy1[1]) ret |= 0x02;
			if (DrvJoy2[1]) ret |= 0x04;
			if (DrvJoy1[0]) ret |= 0x08;
			if (DrvJoy2[0]) ret |= 0x10;
			return ret;
		}

		case 0xed00: // in1
		{
			UINT8 ret = 0;
			if (DrvJoy1[2]) ret |= 0x01;
			if (DrvJoy1[3]) ret |= 0x02;
			if (DrvJoy1[4]) ret |= 0x04;
			if (DrvJoy1[5]) ret |= 0x08;
			if (DrvJoy1[6]) ret |= 0x10;
		//	if (DrvJoy1[7]) ret |= 0x80;	// service?
			return ret;
		} 


		case 0xee00: // in2
		{
			UINT8 ret = 0;
			if (DrvJoy2[2]) ret |= 0x01;
			if (DrvJoy2[3]) ret |= 0x02;
			if (DrvJoy2[4]) ret |= 0x04;
			if (DrvJoy2[5]) ret |= 0x08;
			if (DrvJoy2[6]) ret |= 0x10;
			return ret;
		}

		case 0xef00:
			return ++kangaroo_clock & 0x0f;
	}
	
	bprintf (PRINT_NORMAL, _T("%4.4x, rm\n"), address);

	return 0;
}

UINT8 __fastcall kangaroo_sound_read(UINT16 address)
{
	switch (address & 0xf000)
	{
		case 0x6000:
			return soundlatch;
	}
	
	bprintf (PRINT_NORMAL, _T("%4.4x, rs\n"), address);

	return 0;
}

void __fastcall kangaroo_sound_write(UINT16 address, UINT8 data)
{
	switch (address & 0xf000)
	{
		case 0x7000:
			AY8910Write(0, 1, data);
			return;

		case 0x8000:
			AY8910Write(0, 0, data);
			return;
	}
	
	bprintf (PRINT_NORMAL, _T("%4.4x, %2.2x ws\n"), address, data);

	return;
}

static INT32 MemIndex()
{
	UINT8 *Next;

	Next		= Mem; Next += 0x10000; // buffer

	DrvZ80ROM0	= Next; Next += 0x06000;
	DrvZ80ROM1	= Next; Next += 0x01000;
	DrvGfxROM	= Next; Next += 0x04000;

	Palette		= (UINT32*)Next; Next += 0x00008 * sizeof(UINT32);
	DrvPalette	= (UINT32*)Next; Next += 0x00008 * sizeof(UINT32);

	AllRAM		= Next;

	DrvZ80RAM0	= Next; Next += 0x00400;
	DrvZ80RAM1	= Next; Next += 0x00400;

	DrvVidRAM32	= (UINT32*)Next; Next += 0x4000 * sizeof(UINT32);

	DrvVidControl	= Next; Next += 16;

	RamEnd		= Next;

	pFMBuffer	= (INT16 *)Next; Next += nBurnSoundLen * 3 * sizeof(INT16);

	MemEnd		= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRAM, 0, RamEnd - AllRAM);

	ZetOpen(0);
	ZetReset();
	ZetNmi();
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	AY8910Reset(0);

	soundlatch = 0;
	kangaroo_clock = 0;

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

	for (INT32 i = 0; i < 3; i++)
		pAY8910Buffer[i] = pFMBuffer + nBurnSoundLen * i;

	// kangaroo
	if (strncmp ("kangaro", BurnDrvGetTextA(DRV_NAME), 7) == 0)
	{
		for (INT32 i = 0; i < 6; i++)
			if (BurnLoadRom(DrvZ80ROM0 + i * 0x1000, 0 + i, 0)) return 1;

		if (BurnLoadRom(DrvZ80ROM1, 6, 0)) return 1;

		for (INT32 i = 0; i < 4; i++)
			if (BurnLoadRom(DrvGfxROM  + i * 0x1000, 7 + i, 0)) return 1;
	}
	// funkyfish
	else
	{
		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(DrvZ80ROM0 + i * 0x1000, 0 + i, 0)) return 1;
			if (BurnLoadRom(DrvGfxROM  + i * 0x1000, 5 + i, 0)) return 1;
		}

		if (BurnLoadRom(DrvZ80ROM1, 4, 0)) return 1;
	}

	for (INT32 i = 0; i < 8; i++)
		Palette[i] = ((i & 4 ? 0xff : 0) << 16) | ((i & 2 ? 0xff : 0) << 8) | (i & 1 ? 0xff : 0);


	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x5fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x5fff, 2, DrvZ80ROM0);

	ZetMapArea(0xc000, 0xdfff, 0, DrvGfxROM ); // Gfx are banked here

	ZetMapArea(0xe000, 0xe3ff, 0, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xe3ff, 1, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xe3ff, 2, DrvZ80RAM0);

	ZetSetWriteHandler(kangaroo_main_write);
	ZetSetReadHandler(kangaroo_main_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x0fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x0fff, 2, DrvZ80ROM1);

	// mirrored 4x
	ZetMapArea(0x4000, 0x43ff, 0, DrvZ80RAM1);
	ZetMapArea(0x4000, 0x43ff, 1, DrvZ80RAM1);
	ZetMapArea(0x4000, 0x43ff, 2, DrvZ80RAM1);

	ZetSetWriteHandler(kangaroo_sound_write);
	ZetSetReadHandler(kangaroo_sound_read);
	ZetSetInHandler(kangaroo_sound_read);
	ZetSetOutHandler(kangaroo_sound_write);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 1250000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910SetAllRoutes(0, 0.50, BURN_SND_ROUTE_BOTH);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	BurnFree (Mem);

	GenericTilesExit();
	ZetExit();
	AY8910Exit(0);

	return 0;
}


static INT32 DrvDraw()
{
	UINT8 scrolly = DrvVidControl[6];
	UINT8 scrollx = DrvVidControl[7];
	UINT8 maska = (DrvVidControl[10] & 0x28) >> 3;
	UINT8 maskb = (DrvVidControl[10] & 0x07) >> 0;
	UINT8 xora = (DrvVidControl[9] & 0x20) ? 0xff : 0x00;
	UINT8 xorb = (DrvVidControl[9] & 0x10) ? 0xff : 0x00;
	UINT8 enaa = (DrvVidControl[9] & 0x08);
	UINT8 enab = (DrvVidControl[9] & 0x04);
	UINT8 pria = (~DrvVidControl[9] & 0x02);
	UINT8 prib = (~DrvVidControl[9] & 0x01);
	INT32 x, y;

	if (DrvRecalc) {
		for (x = 0; x < 8; x++) {
			UINT32 col = Palette[x];
			DrvPalette[x] = BurnHighCol(col >> 16, col >> 8, col, 0);
		}
	}

	// iterate over pixels
	for (y = 0; y < nScreenHeight; y++)
	{
		UINT16 *dest = pTransDraw + (y * nScreenWidth);

		for (x = 0; x < nScreenWidth; x += 2)
		{
			UINT8 effxa = scrollx + ((x / 2) ^ xora);
			UINT8 effya = scrolly + (y ^ xora);
			UINT8 effxb = (x / 2) ^ xorb;
			UINT8 effyb = y ^ xorb;
			UINT8 pixa = (DrvVidRAM32[effya + 256 * (effxa / 4)] >> (8 * (effxa % 4) + 0)) & 0x0f;
			UINT8 pixb = (DrvVidRAM32[effyb + 256 * (effxb / 4)] >> (8 * (effxb % 4) + 4)) & 0x0f;
			UINT8 finalpens;

			// for each layer, contribute bits if (a) enabled, and (b) either has priority or the opposite plane is 0
			finalpens = 0;
			if (enaa && (pria || pixb == 0))
				finalpens |= pixa;
			if (enab && (prib || pixa == 0))
				finalpens |= pixb;

			// store the first of two pixels, which is always full brightness
			dest[x + 0] = finalpens & 7;

			// KOS1 alternates at 5MHz, offset from the pixel clock by 1/2 clock
			// when 0, it enables the color mask for pixels with Z = 0
			finalpens = 0;
			if (enaa && (pria || pixb == 0))
			{
				if (!(pixa & 0x08)) pixa &= maska;
				finalpens |= pixa;
			}
			if (enab && (prib || pixa == 0))
			{
				if (!(pixb & 0x08)) pixb &= maskb;
				finalpens |= pixb;
			}

			// store the second of two pixels, which is affected by KOS1 and the A/B masks
			dest[x + 1] = finalpens & 7;
		}
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();

	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = 10;
	INT32 nCyclesTotal[2] = { 2500000 / 60, 1250000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext, nCyclesSegment;
		
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		if (i == (nInterleave - 1)) {
			ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		}
		ZetClose();
		
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesSegment = ZetRun(nCyclesSegment);
		nCyclesDone[nCurrentCPU] += nCyclesSegment;
		if (i == (nInterleave - 1)) {
			ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		}
		ZetClose();

/*		ZetOpen(0);
		nCyclesDone[0] -= ZetRun(nCyclesDone[0] / (nInterleave - i));
		if (i == (nInterleave - 1)) ZetRaiseIrq(0);
		ZetClose();

		// Run Z80 #1
		ZetOpen(1);
		nCyclesDone[1] -= ZetRun(nCyclesDone[1] / (nInterleave - i));
		if (i == (nInterleave - 1)) ZetRaiseIrq(0);
		ZetClose();*/

		// Render Sound Segment
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Render(&pAY8910Buffer[0], pSoundBuf, nSegmentLength, 0);
			nSoundBufferPos += nSegmentLength;
		}
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
		if (nSegmentLength) {
			AY8910Render(&pAY8910Buffer[0], pSoundBuf, nSegmentLength, 0);
		}
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

// Funky Fish

static struct BurnRomInfo fnkyfishRomDesc[] = {
	{ "tvg_64.0",    0x1000, 0xaf728803, 1 | BRF_ESS | BRF_PRG }, //  0 Main Z80 Code
	{ "tvg_65.1",    0x1000, 0x71959e6b, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "tvg_66.2",    0x1000, 0x5ccf68d4, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "tvg_67.3",    0x1000, 0x938ff36f, 1 | BRF_ESS | BRF_PRG }, //  3

	{ "tvg_68.8",    0x1000, 0xd36bb2be, 2 | BRF_ESS | BRF_PRG }, //  4 Sound Z80 Code

	{ "tvg_69.v0",   0x1000, 0xcd532d0b, 3 | BRF_GRA },	      //  5 Graphics
	{ "tvg_71.v2",   0x1000, 0xa59c9713, 3 | BRF_GRA },	      //  6
	{ "tvg_70.v1",   0x1000, 0xfd308ef1, 3 | BRF_GRA },	      //  7
	{ "tvg_72.v3",   0x1000, 0x6ae9b584, 3 | BRF_GRA },	      //  8
};

STD_ROM_PICK(fnkyfish)
STD_ROM_FN(fnkyfish)

struct BurnDriver BurnDrvfnkyfish = {
	"fnkyfish", NULL, NULL, NULL, "1981",
	"Funky Fish\0", NULL, "Sun Electronics", "Kangaroo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, fnkyfishRomInfo, fnkyfishRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, NULL, 
	&DrvRecalc, 0x08, 256, 512, 3, 4
};


// Kangaroo

static struct BurnRomInfo kangarooRomDesc[] = {
	{ "tvg_75.0",    0x1000, 0x0d18c581, 1 | BRF_ESS | BRF_PRG }, //  0 Main Z80 Code
	{ "tvg_76.1",    0x1000, 0x5978d37a, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "tvg_77.2",    0x1000, 0x522d1097, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "tvg_78.3",    0x1000, 0x063da970, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "tvg_79.4",    0x1000, 0x9e5cf8ca, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "tvg_80.5",    0x1000, 0x2fc18049, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "tvg_81.8",    0x1000, 0xfb449bfd, 2 | BRF_ESS | BRF_PRG }, //  6 Sound Z80 Code

	{ "tvg_83.v0",   0x1000, 0xc0446ca6, 3 | BRF_GRA },	      //  7 Graphics
	{ "tvg_85.v2",   0x1000, 0x72c52695, 3 | BRF_GRA },	      //  8
	{ "tvg_84.v1",   0x1000, 0xe4cb26c2, 3 | BRF_GRA },	      //  9
	{ "tvg_86.v3",   0x1000, 0x9e6a599f, 3 | BRF_GRA },	      // 10

	{ "tvg_82.12",   0x0800, 0x57766f69, 0 | BRF_ESS | BRF_PRG }, // 11 MB8841 custom microcontroller data (unused)
	
	{ "mb8841.ic29", 0x0800, 0x00000000, 0 | BRF_OPT | BRF_NODUMP }, // 12 MB8841 custom microcontroller prg (unused)
};

STD_ROM_PICK(kangaroo)
STD_ROM_FN(kangaroo)

struct BurnDriver BurnDrvkangaroo = {
	"kangaroo", NULL, NULL, NULL, "1982",
	"Kangaroo\0", NULL, "Sun Electronics", "Kangaroo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, kangarooRomInfo, kangarooRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, NULL, 
	&DrvRecalc, 0x08, 256, 512, 3, 4
};


// Kangaroo (Atari)
static struct BurnRomInfo kangaroaRomDesc[] = {
	{ "136008-101.ic7",  0x1000, 0x0d18c581, 1 | BRF_ESS | BRF_PRG }, //  0 Main Z80 Code
	{ "136008-102.ic8",  0x1000, 0x5978d37a, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "136008-103.ic9",  0x1000, 0x522d1097, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "136008-104.ic10", 0x1000, 0x063da970, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "136008-105.ic16", 0x1000, 0x82a26c7d, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "136008-106.ic17", 0x1000, 0x3dead542, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "136008-107.ic24", 0x1000, 0xfb449bfd, 2 | BRF_ESS | BRF_PRG }, //  6 Sound Z80 Code

	{ "136008-108.ic76", 0x1000, 0xc0446ca6, 3 | BRF_GRA },	      //  7 Graphics
	{ "136008-110.ic77", 0x1000, 0x72c52695, 3 | BRF_GRA },	      //  8
	{ "136008-109.ic52", 0x1000, 0xe4cb26c2, 3 | BRF_GRA },	      //  9
	{ "136008-111.ic53", 0x1000, 0x9e6a599f, 3 | BRF_GRA },	      // 10

	{ "136008-112.ic28", 0x0800, 0x57766f69, 0 | BRF_ESS | BRF_PRG }, // 11 MB8841 custom microcontroller (unused)
	
	{ "mb8841.ic29",     0x0800, 0x00000000, 0 | BRF_OPT | BRF_NODUMP }, // 12 MB8841 custom microcontroller prg (unused)
};

STD_ROM_PICK(kangaroa)
STD_ROM_FN(kangaroa)

struct BurnDriver BurnDrvkangaroa = {
	"kangarooa", "kangaroo", NULL, NULL, "1982",
	"Kangaroo (Atari)\0", NULL, "[Sun Electronics] (Atari license)", "Kangaroo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, kangaroaRomInfo, kangaroaRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, NULL, 
	&DrvRecalc, 0x08, 256, 512, 3, 4
};


// Kangaroo (bootleg)

static struct BurnRomInfo kangarobRomDesc[] = {
	{ "tvg_75.0",    0x1000, 0x0d18c581, 1 | BRF_ESS | BRF_PRG }, //  0 Main Z80 Code
	{ "tvg_76.1",    0x1000, 0x5978d37a, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "tvg_77.2",    0x1000, 0x522d1097, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "tvg_78.3",    0x1000, 0x063da970, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "tvg_79.4",    0x1000, 0x9e5cf8ca, 1 | BRF_ESS | BRF_PRG }, //  4
	{ "k6",          0x1000, 0x7644504a, 1 | BRF_ESS | BRF_PRG }, //  5

	{ "tvg_81.8",    0x1000, 0xfb449bfd, 2 | BRF_ESS | BRF_PRG }, //  6 Sound Z80 Code

	{ "tvg_83.v0",   0x1000, 0xc0446ca6, 3 | BRF_GRA },	      //  7 Graphics
	{ "tvg_85.v2",   0x1000, 0x72c52695, 3 | BRF_GRA },	      //  8
	{ "tvg_84.v1",   0x1000, 0xe4cb26c2, 3 | BRF_GRA },	      //  9
	{ "tvg_86.v3",   0x1000, 0x9e6a599f, 3 | BRF_GRA },	      // 10
};

STD_ROM_PICK(kangarob)
STD_ROM_FN(kangarob)

struct BurnDriver BurnDrvkangarob = {
	"kangaroob", "kangaroo", NULL, NULL, "1982",
	"Kangaroo (bootleg)\0", NULL, "Bootleg", "Kangaroo",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, kangarobRomInfo, kangarobRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, NULL, 
	&DrvRecalc, 0x08, 256, 512, 3, 4
};
