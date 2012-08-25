// FB Alpha Son Son driver module
// Based on MAME driver by Mirko Buffoni

#include "tiles_generic.h"
#include "m6809_intf.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvM6809ROM0;
static UINT8 *DrvM6809ROM1;
static UINT8 *DrvM6809RAM0;
static UINT8 *DrvM6809RAM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvColPROM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvColRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvScrollX;
static UINT32 *DrvPalette;
static UINT32 *Palette;
static UINT8 DrvRecalc;

static INT16 *pAY8910Buffer[6];
static INT16 *pFMBuffer;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvInps[3];
static UINT8 DrvDips[2];
static UINT8 DrvReset;

static UINT8 soundlatch;
static UINT8 flipscreen;

static UINT8 DrvSoundTrigger;
static UINT8 DrvSoundIrqTrigger;

static INT32 sonsonj;

static struct BurnInputInfo SonsonInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Sonson)

static struct BurnDIPInfo SonsonDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0xdf, NULL			},
	{0x10, 0xff, 0xff, 0xeb, NULL			},

	{0   , 0xfe, 0   ,    16, "Coinage"		},
	{0x0f, 0x01, 0x0f, 0x02, "4C_1C"		},
	{0x0f, 0x01, 0x0f, 0x05, "3C_1C"		},
	{0x0f, 0x01, 0x0f, 0x08, "2C_1C"		},
	{0x0f, 0x01, 0x0f, 0x04, "3C_2C"		},
	{0x0f, 0x01, 0x0f, 0x01, "4C_3C"		},
	{0x0f, 0x01, 0x0f, 0x0f, "1C_1C"		},
	{0x0f, 0x01, 0x0f, 0x03, "3C_4C"		},
	{0x0f, 0x01, 0x0f, 0x07, "2C_3C"		},
	{0x0f, 0x01, 0x0f, 0x0e, "1C_2C"		},
	{0x0f, 0x01, 0x0f, 0x06, "2C_5C"		},
	{0x0f, 0x01, 0x0f, 0x0d, "1C_3C"		},
	{0x0f, 0x01, 0x0f, 0x0c, "1C_4C"		},
	{0x0f, 0x01, 0x0f, 0x0b, "1C_5C"		},
	{0x0f, 0x01, 0x0f, 0x0a, "1C_6C"		},
	{0x0f, 0x01, 0x0f, 0x09, "1C_7C"		},
	{0x0f, 0x01, 0x0f, 0x00, "Free_Play"		},

	{0   , 0xfe, 0   ,    2, "Coinage affects"	},
	{0x0f, 0x01, 0x10, 0x10, "Coin A"		},
	{0x0f, 0x01, 0x10, 0x00, "Coin B"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x0f, 0x01, 0x20, 0x20, "Off"			},
	{0x0f, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0f, 0x01, 0x40, 0x40, "Off"			},
	{0x0f, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x10, 0x01, 0x03, 0x03, "3"			},
	{0x10, 0x01, 0x03, 0x02, "4"			},
	{0x10, 0x01, 0x03, 0x01, "5"			},
	{0x10, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "2 Players Game"	},
	{0x10, 0x01, 0x04, 0x04, "1 Credit"		},
	{0x10, 0x01, 0x04, 0x00, "2 Credits"		},

	{0   , 0xfe, 0   ,    4, "Bonus_Life"		},
	{0x10, 0x01, 0x18, 0x08, "20000 80000 100000"	},
	{0x10, 0x01, 0x18, 0x00, "30000 90000 120000"	},
	{0x10, 0x01, 0x18, 0x18, "20000"		},
	{0x10, 0x01, 0x18, 0x10, "30000"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x10, 0x01, 0x60, 0x60, "Easy"			},
	{0x10, 0x01, 0x60, 0x40, "Medium"		},
	{0x10, 0x01, 0x60, 0x20, "Hard"			},
	{0x10, 0x01, 0x60, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x10, 0x01, 0x80, 0x80, "Off"			},
	{0x10, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Sonson)

static void sonson_sound_irqtrigger(UINT8 data)
{
	data &= 1;

	if (DrvSoundTrigger == 0 && data == 1)
	{
		DrvSoundIrqTrigger = 1;
	}

	DrvSoundTrigger = data;
}

void sonson_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x3000:
			for (INT32 i = 5; i < 32; i++) {
				DrvScrollX[i] = data;
			}
		return;

		case 0x3008:
		return;

		case 0x3010:
			soundlatch = data;
		return;

		case 0x3018:
			flipscreen = ~data & 1;
		return;

		case 0x3019:
			sonson_sound_irqtrigger(data);
		return;
	}
}

UINT8 sonson_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x3002:
		case 0x3003:
		case 0x3004:
			return DrvInps[address - 0x3002];

		case 0x3005:
		case 0x3006:
			return DrvDips[address - 0x3005];
	}

	return 0;
}

void sonson_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x2000:
			AY8910Write(0, 0, data);
		return;

		case 0x2001:
			AY8910Write(0, 1, data);
		return;

		case 0x4000:
			AY8910Write(1, 0, data);
		return;

		case 0x4001:
			AY8910Write(1, 1, data);
		return;
	}
}

UINT8 sonson_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xa000:
			return soundlatch;
	}

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[2] = { 0x10000,  0x00000 };
	INT32 Plane1[3] = { 0x40000,  0x20000, 0x00000 };
	INT32 XOffs0[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 XOffs[16] = { 8*16+7, 8*16+6, 8*16+5, 8*16+4, 8*16+3, 8*16+2, 8*16+1, 8*16+0, 7, 6, 5, 4, 3, 2, 1, 0 };
	INT32 YOffs[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0xC000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x4000);

	GfxDecode(0x400, 2,  8,  8, Plane0, XOffs0, YOffs, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0xc000);

	GfxDecode(0x200, 3, 16, 16, Plane1, XOffs,  YOffs, 0x100, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 DrvPaletteInit()
{
	UINT32 *tmp = (UINT32*)BurnMalloc(0x20 * sizeof (UINT32));
	if (tmp == NULL) {
		return 1;
	}

	for (INT32 i = 0; i < 0x20; i++)
	{
		INT32 bit0, bit1, bit2, bit3;
		INT32 r, g, b;

		bit0 = (DrvColPROM[i + 0x100] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + 0x100] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + 0x100] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + 0x100] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[i + 0x000] >> 4) & 0x01;
		bit1 = (DrvColPROM[i + 0x000] >> 5) & 0x01;
		bit2 = (DrvColPROM[i + 0x000] >> 6) & 0x01;
		bit3 = (DrvColPROM[i + 0x000] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[i + 0x000] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + 0x000] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + 0x000] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + 0x000] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		tmp[i] = (r << 16) | (g << 8) | b;
	}

	for (INT32 i = 0; i < 0x200; i++) {
		Palette[i] = tmp[(DrvColPROM[0x200 + i] & 0x0f) | ((i >> 4) & 0x10)];
	}

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvM6809ROM0	= Next; Next += 0x010000;
	DrvM6809ROM1	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x010000;
	DrvGfxROM1	= Next; Next += 0x020000;

	DrvColPROM	= Next; Next += 0x000400;

	Palette		= (UINT32*)Next; Next += 0x00200 * sizeof(UINT32);
	DrvPalette	= (UINT32*)Next; Next += 0x00200 * sizeof(UINT32);

	AllRam		= Next;

	DrvM6809RAM0	= Next; Next += 0x001000;
	DrvM6809RAM1	= Next; Next += 0x001800;

	DrvVidRAM	= Next; Next += 0x000400;
	DrvColRAM	= Next; Next += 0x000400;
	DrvSprRAM	= Next; Next += 0x000100;

	DrvScrollX	= Next; Next += 0x000020;

	RamEnd		= Next;

	pFMBuffer	= (INT16*)Next; Next += nBurnSoundLen * 6 * sizeof (INT16);

	MemEnd		= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	for (INT32 i = 0; i < 2; i++) {
		M6809Open(i);
		M6809Reset();
		M6809Close();
	}

	AY8910Reset(0);
	AY8910Reset(1);

	soundlatch = 0;
	flipscreen = 0;

	DrvSoundIrqTrigger = 0;
	DrvSoundTrigger = 0;

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

	for (INT32 i = 0; i < 6; i++) {
		pAY8910Buffer[i] = pFMBuffer + nBurnSoundLen * i;
	}

	if (sonsonj)
	{
		for (INT32 i = 0; i < 6; i++) {
			if (BurnLoadRom(DrvM6809ROM0 + 0x4000 + i * 0x2000, i, 1)) return 1;
		}

		if (BurnLoadRom(DrvM6809ROM1 + 0xe000, 6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0   + 0x0000, 7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0   + 0x2000, 8, 1)) return 1;

		for (INT32 i = 0; i < 6; i++) {
			if (BurnLoadRom(DrvGfxROM1 + i * 0x2000, i +  9, 1)) return 1;
		}

		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(DrvColPROM + i * 0x0100, i + 15, 1)) return 1;
		}
	} else {
		if (BurnLoadRom(DrvM6809ROM0 + 0x4000, 0, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM0 + 0x8000, 1, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM0 + 0xc000, 2, 1)) return 1;

		if (BurnLoadRom(DrvM6809ROM1 + 0xe000, 3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0   + 0x0000, 4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0   + 0x2000, 5, 1)) return 1;

		for (INT32 i = 0; i < 6; i++) {
			if (BurnLoadRom(DrvGfxROM1 + i * 0x2000, i +  6, 1)) return 1;
		}

		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(DrvColPROM + i * 0x0100, i + 12, 1)) return 1;
		}
	}

	DrvPaletteInit();
	DrvGfxDecode();

	M6809Init(2);
	M6809Open(0);
	M6809MapMemory(DrvM6809RAM0,		0x0000, 0x0fff, M6809_RAM);
	M6809MapMemory(DrvVidRAM,		0x1000, 0x13ff, M6809_RAM);
	M6809MapMemory(DrvColRAM,		0x1400, 0x17ff, M6809_RAM);
	M6809MapMemory(DrvSprRAM,		0x2020, 0x207f, M6809_RAM); // 0x100 min
	M6809MapMemory(DrvM6809ROM0 + 0x04000,	0x4000, 0xffff, M6809_ROM);
	M6809SetReadHandler(sonson_main_read);
	M6809SetWriteHandler(sonson_main_write);
	M6809Close();

	M6809Open(1);
	M6809MapMemory(DrvM6809RAM1,		0x0000, 0x07ff, M6809_RAM);
	M6809MapMemory(DrvM6809ROM1 + 0x0e000,	0xe000, 0xffff, M6809_ROM);
	M6809SetReadHandler(sonson_sound_read);
	M6809SetWriteHandler(sonson_sound_write);
	M6809Close();

	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910SetAllRoutes(0, 0.30, BURN_SND_ROUTE_BOTH);
	AY8910SetAllRoutes(1, 0.30, BURN_SND_ROUTE_BOTH);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	M6809Exit();
	AY8910Exit(0);
	AY8910Exit(1);

	BurnFree (AllMem);

	sonsonj = 0;

	return 0;
}

static void draw_background()
{
	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		if (flipscreen)
		{
			sx ^= 0xf8;
			sy ^= 0xf8;

			sx += DrvScrollX[sy >> 3];
			if (sx > 255) sx -= 256;
		} else {
			sx -= DrvScrollX[sy >> 3];
			if (sx < -7) sx += 256;
		}

		sy -= 8;
		sx -= 8;

		INT32 attr  = DrvColRAM[offs];
		INT32 code  = DrvVidRAM[offs] | ((attr & 0x03) << 8);
		INT32 color = attr >> 2;

		if (flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 2, 0, DrvGfxROM0);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 2, 0, DrvGfxROM0);
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0x80 - 4; offs >= 0x20; offs -= 4)
	{
		INT32 color = DrvSprRAM[offs + 1];
		INT32 code = DrvSprRAM[offs + 2] + ((color & 0x20) << 3);
		INT32 flipx = ~color & 0x40;
		INT32 flipy = ~color & 0x80;
		INT32 sx = DrvSprRAM[offs + 3];
		INT32 sy = DrvSprRAM[offs + 0];

		color &= 0x1f;

		if (flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		sy -= 8;
		sx -= 8;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 3, 0, 0x100, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 3, 0, 0x100, DrvGfxROM1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 3, 0, 0x100, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 3, 0, 0x100, DrvGfxROM1);
			}
		}
	}
}


static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x200; i++) {
			INT32 rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	draw_background();

	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}


static INT32 DrvFrame()
{
	INT32 nInterleave = 16;
	
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInps[0] = DrvInps[1] = DrvInps[2] = 0xff;

		for (INT32 i = 0 ; i < 8; i++) {
			DrvInps[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInps[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInps[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	INT32 nCyclesSegment = 0;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] =  { 2000000 / 60, 2000000 / 60 };
	INT32 nCyclesDone[2] =  { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;
		
		nCurrentCPU = 0;
		M6809Open(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += M6809Run(nCyclesSegment);
		if (i == (nInterleave - 1)) {
			M6809SetIRQLine(0, M6809_IRQSTATUS_AUTO);
		}
		M6809Close();

		nCurrentCPU = 1;
		M6809Open(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (DrvSoundIrqTrigger) {
			M6809SetIRQLine(1, M6809_IRQSTATUS_AUTO);
			DrvSoundIrqTrigger = 0;
		}
		nCyclesDone[nCurrentCPU] += M6809Run(nCyclesSegment);
		if (i == 3 || i == 7 || i == 11 || i == 15) {
			M6809SetIRQLine(0, M6809_IRQSTATUS_AUTO);
		}
		M6809Close();

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			AY8910Render(&pAY8910Buffer[0], pSoundBuf, nSegmentLength, 0);
			nSoundBufferPos += nSegmentLength;
		}
	}

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

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029695;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		M6809Scan(nAction);
		AY8910Scan(nAction, pnMin);

		SCAN_VAR(soundlatch);
		SCAN_VAR(flipscreen);
		SCAN_VAR(DrvSoundTrigger);
	}

	return 0;
}


// Son Son

static struct BurnRomInfo sonsonRomDesc[] = {
	{ "ss.01e",	0x4000, 0xcd40cc54, 1 | BRF_PRG | BRF_ESS }, //  0 Main m6809
	{ "ss.02e",	0x4000, 0xc3476527, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ss.03e",	0x4000, 0x1fd0e729, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "ss_6.c11",	0x2000, 0x1135c48a, 2 | BRF_PRG | BRF_ESS }, //  3 Sound m6809

	{ "ss_7.b6",	0x2000, 0x990890b1, 3 | BRF_GRA },           //  4 Characters
	{ "ss_8.b5",	0x2000, 0x9388ff82, 3 | BRF_GRA },           //  5

	{ "ss_9.m5",	0x2000, 0x8cb1cacf, 4 | BRF_GRA },           //  6 Sprites
	{ "ss_10.m6",	0x2000, 0xf802815e, 4 | BRF_GRA },           //  7
	{ "ss_11.m3",	0x2000, 0x4dbad88a, 4 | BRF_GRA },           //  8
	{ "ss_12.m4",	0x2000, 0xaa05e687, 4 | BRF_GRA },           //  9
	{ "ss_13.m1",	0x2000, 0x66119bfa, 4 | BRF_GRA },           // 10
	{ "ss_14.m2",	0x2000, 0xe14ef54e, 4 | BRF_GRA },           // 11

	{ "ssb4.b2",	0x0020, 0xc8eaf234, 5 | BRF_GRA },           // 12 Color Proms
	{ "ssb5.b1",	0x0020, 0x0e434add, 5 | BRF_GRA },           // 13
	{ "ssb2.c4",	0x0100, 0xc53321c6, 5 | BRF_GRA },           // 14
	{ "ssb3.h7",	0x0100, 0x7d2c324a, 5 | BRF_GRA },           // 15

	{ "ssb1.k11",	0x0100, 0xa04b0cfe, 0 | BRF_OPT },           // 16 Timing Prom
};

STD_ROM_PICK(sonson)
STD_ROM_FN(sonson)

struct BurnDriver BurnDrvSonson = {
	"sonson", NULL, NULL, NULL, "1984",
	"Son Son\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, sonsonRomInfo, sonsonRomName, NULL, NULL, SonsonInputInfo, SonsonDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	240, 240, 4, 3
};


// Son Son (Japan)

static struct BurnRomInfo sonsonjRomDesc[] = {
	{ "ss_0.l9",	0x2000, 0x705c168f, 1 | BRF_PRG | BRF_ESS }, //  0 Main m6809
	{ "ss_1.j9",	0x2000, 0x0f03b57d, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ss_2.l8",	0x2000, 0xa243a15d, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ss_3.j8",	0x2000, 0xcb64681a, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "ss_4.l7",	0x2000, 0x4c3e9441, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "ss_5.j7",	0x2000, 0x847f660c, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "ss_6.c11",	0x2000, 0x1135c48a, 2 | BRF_PRG | BRF_ESS }, //  6 Sound m6809

	{ "ss_7.b6",	0x2000, 0x990890b1, 3 | BRF_GRA },           //  7 Characters
	{ "ss_8.b5",	0x2000, 0x9388ff82, 3 | BRF_GRA },           //  8

	{ "ss_9.m5",	0x2000, 0x8cb1cacf, 4 | BRF_GRA },           //  9 Sprites
	{ "ss_10.m6",	0x2000, 0xf802815e, 4 | BRF_GRA },           // 10
	{ "ss_11.m3",	0x2000, 0x4dbad88a, 4 | BRF_GRA },           // 11
	{ "ss_12.m4",	0x2000, 0xaa05e687, 4 | BRF_GRA },           // 12
	{ "ss_13.m1",	0x2000, 0x66119bfa, 4 | BRF_GRA },           // 13
	{ "ss_14.m2",	0x2000, 0xe14ef54e, 4 | BRF_GRA },           // 14

	{ "ssb4.b2",	0x0020, 0xc8eaf234, 5 | BRF_GRA },           // 15 Color Proms
	{ "ssb5.b1",	0x0020, 0x0e434add, 5 | BRF_GRA },           // 16
	{ "ssb2.c4",	0x0100, 0xc53321c6, 5 | BRF_GRA },           // 17
	{ "ssb3.h7",	0x0100, 0x7d2c324a, 5 | BRF_GRA },           // 18

	{ "ssb1.k11",	0x0100, 0xa04b0cfe, 0 | BRF_OPT },           // 19 Timing Prom
};

STD_ROM_PICK(sonsonj)
STD_ROM_FN(sonsonj)

static INT32 SonsonjInit()
{
	sonsonj = 1;

	return DrvInit();
}

struct BurnDriver BurnDrvSonsonj = {
	"sonsonj", "sonson", NULL, NULL, "1984",
	"Son Son (Japan)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM, 0,
	NULL, sonsonjRomInfo, sonsonjRomName, NULL, NULL, SonsonInputInfo, SonsonDIPInfo,
	SonsonjInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	240, 240, 4, 3
};
