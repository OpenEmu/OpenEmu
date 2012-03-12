// FB Alpha Mysterious Stones Driver Module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "m6502_intf.h"
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
static UINT8 *DrvGfxROM2;
static UINT8 *DrvColPROM;
static UINT8 *Drv6502RAM;
static UINT8 *DrvFgRAM;
static UINT8 *DrvBgRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;

static UINT8 *flipscreen;
static UINT8 *soundlatch;
static UINT8 *scrolly;
static UINT8 *video_control;

static INT16 *pAY8910Buf = NULL;
static INT16 *pAY8910Buffer[6];

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 ay8910_select;
static INT32 vblank;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[2];
static UINT8 DrvReset;

static struct BurnInputInfo DrvInputList[] = {
	{"P1 Coin"      , BIT_DIGITAL  , DrvJoy1 + 6,	"p1 coin"  },
	{"P1 start"  ,    BIT_DIGITAL  , DrvJoy2 + 6,	"p1 start" },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 0, 	"p1 right" },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 1, 	"p1 left"  },
	{"P1 Up",	  BIT_DIGITAL,   DrvJoy1 + 2,   "p1 up",   },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy1 + 3,   "p1 down", },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 5,	"p1 fire 2"},

	{"P2 Coin"      , BIT_DIGITAL  , DrvJoy1 + 7,	"p2 coin"  },
	{"P2 start"  ,    BIT_DIGITAL  , DrvJoy2 + 7,	"p2 start" },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy2 + 0, 	"p2 right" },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy2 + 1, 	"p2 left"  },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy2 + 2,   "p2 up",   },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy2 + 3,   "p2 down", },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy2 + 5,	"p2 fire 2"},

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip 1"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip 2"	   },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	{0x11, 0xff, 0xff, 0xfb, NULL                     },
	{0x12, 0xff, 0xff, 0x1f, NULL                     },

	{0   , 0xfe, 0   , 2   , "Lives"	          },
	{0x11, 0x01, 0x01, 0x01, "3"     		  },
	{0x11, 0x01, 0x01, 0x00, "5"			  },

	{0   , 0xfe, 0   , 2   , "Difficulty"             },
	{0x11, 0x01, 0x02, 0x02, "Easy"       		  },
	{0x11, 0x01, 0x02, 0x00, "Hard"       		  },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x11, 0x01, 0x04, 0x04, "Off"     		  },
	{0x11, 0x01, 0x04, 0x00, "On"    		  },

	{0   , 0xfe, 0   , 4   , "Coin A"                 },
	{0x12, 0x01, 0x03, 0x00, "2C 1C"     		  },
	{0x12, 0x01, 0x03, 0x03, "1C 1C"    		  },
	{0x12, 0x01, 0x03, 0x02, "1C 2C"     		  },
	{0x12, 0x01, 0x03, 0x01, "1C 3C"    		  },

	{0   , 0xfe, 0   , 4   , "Coin B"                 },
	{0x12, 0x01, 0x0c, 0x00, "2C 1C"     		  },
	{0x12, 0x01, 0x0c, 0x0c, "1C 1C"    		  },
	{0x12, 0x01, 0x0c, 0x08, "1C 2C"     		  },
	{0x12, 0x01, 0x0c, 0x04, "1C 3C"    		  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x12, 0x01, 0x20, 0x00, "Off"       		  },
	{0x12, 0x01, 0x20, 0x20, "On"       		  },

	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x40, 0x00, "Upright"     		  },
	{0x12, 0x01, 0x40, 0x40, "Cocktail"    		  },
};

STDDIPINFO(Drv)

static void mystston_soundcontrol(UINT8 data)
{
	if (((ay8910_select & 0x20) == 0x20) && ((data & 0x20) == 0x00))
	{
		AY8910Write(0, (ay8910_select & 0x10) ? 0 : 1, *soundlatch);
	}

	if (((ay8910_select & 0x80) == 0x80) && ((data & 0x80) == 0x00))
	{
		AY8910Write(0, (ay8910_select & 0x40) ? 0 : 1, *soundlatch);
	}

	ay8910_select = data;
}

UINT8 mystston_read(UINT16 address)
{
	switch (address & ~0x1f8f)
	{
		case 0x2000:
			return DrvInputs[0];

		case 0x2010:
			return DrvInputs[1];

		case 0x2020:
			return DrvDips[0];

		case 0x2030:
			return DrvDips[1] | vblank;
	}

	return 0;
}

void mystston_write(UINT16 address, UINT8 data)
{
	if ((address & 0xe060) == 0x2060) {
		DrvPalRAM[address & 0x1f] = data;
		return;
	}

	switch (address & ~0x1f8f)
	{
		case 0x2000:
			*video_control = data;
		return;

		case 0x2010:
			M6502SetIRQ(M6502_IRQ_LINE, M6502_IRQSTATUS_NONE);
		break;

		case 0x2020:
			*scrolly = data;
		break;

		case 0x2030:
			*soundlatch = data;
		break;

		case 0x2040:
			mystston_soundcontrol(data);
		break;
	}
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	M6502Open(0);
	M6502Reset();
	M6502Close();

	AY8910Reset(0);
	AY8910Reset(1);

	ay8910_select = 0;

	return 0;
}

static void DrvPaletteUpdate(UINT8 *p, INT32 offs)
{
	for (INT32 i = 0; i < 0x20; i++)
	{
		INT32 bit0 = (p[i] >> 0) & 0x01;
		INT32 bit1 = (p[i] >> 1) & 0x01;
		INT32 bit2 = (p[i] >> 2) & 0x01;
		INT32 r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (p[i] >> 3) & 0x01;
		bit1 = (p[i] >> 4) & 0x01;
		bit2 = (p[i] >> 5) & 0x01;
		INT32 g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit1 = (p[i] >> 6) & 0x01;
		bit2 = (p[i] >> 7) & 0x01;
		INT32 b = 0x21 * 0 + 0x47 * bit1 + 0x97 * bit2;

		DrvPalette[offs + i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 DrvGfxDecode()
{
	static INT32 Planes[3] = { RGN_FRAC(0xc000,2,3), RGN_FRAC(0xc000,1,3), RGN_FRAC(0xc000,0,3) };
	static INT32 XOffs[16] = { STEP8(128, 1), STEP8(0, 1) };
	static INT32 YOffs[16] = { STEP16(0, 8) };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x10000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x10000);

	GfxDecode(0x800, 3,  8,  8, Planes, XOffs + 8, YOffs, 0x040, tmp, DrvGfxROM0);
	GfxDecode(0x200, 3, 16, 16, Planes, XOffs + 0, YOffs, 0x100, tmp, DrvGfxROM2);

	memcpy (tmp, DrvGfxROM1, 0x10000);

	GfxDecode(0x200, 3, 16, 16, Planes, XOffs + 0, YOffs, 0x100, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv6502ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x020000;
	DrvGfxROM1		= Next; Next += 0x020000;
	DrvGfxROM2		= Next; Next += 0x020000;

	DrvColPROM		= Next; Next += 0x000020;

	DrvPalette		= (UINT32*)Next; Next += 0x0040 * sizeof(UINT32);

	AllRam			= Next;

	Drv6502RAM		= Next; Next += 0x001000;
	DrvFgRAM		= Next; Next += 0x000800;
	DrvBgRAM		= Next; Next += 0x000800;
	DrvPalRAM		= Next; Next += 0x000020;

	flipscreen		= Next; Next += 0x000001;
	soundlatch		= Next; Next += 0x000001;
	scrolly			= Next; Next += 0x000001;
	video_control		= Next; Next += 0x000001;

	RamEnd			= Next;

	DrvSprRAM		= Drv6502RAM + 0x0780;

	MemEnd			= Next;

	return 0;
}

// nBurnSoundLen changes if the refresh rate is changed, but this only 
// occurs AFTER the init is called, so we can't allocate this there, so
// we call it during the frame function.
static void SoundBufferAlloc()
{
	pAY8910Buf = (INT16*)malloc(nBurnSoundLen * 6 * sizeof(INT16));

	for (INT32 i = 0; i < 6; i++) {
		pAY8910Buffer[i] = pAY8910Buf + i * nBurnSoundLen;
	}
}

static INT32 DrvInit()
{
	BurnSetRefreshRate(57.445);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv6502ROM + 0x4000,  0, 1)) return 1;
		if (BurnLoadRom(Drv6502ROM + 0x6000,  1, 1)) return 1;
		if (BurnLoadRom(Drv6502ROM + 0x8000,  2, 1)) return 1;
		if (BurnLoadRom(Drv6502ROM + 0xa000,  3, 1)) return 1;
		if (BurnLoadRom(Drv6502ROM + 0xc000,  4, 1)) return 1;
		if (BurnLoadRom(Drv6502ROM + 0xe000,  5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x0000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x2000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x4000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x6000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x8000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0xa000, 11, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x0000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x2000, 13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x4000, 14, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x6000, 15, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x8000, 16, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0xa000, 17, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x0000, 18, 1)) return 1;

		if (DrvGfxDecode()) return 1;

		DrvPaletteUpdate(DrvColPROM, 0x20);
	}

	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(Drv6502RAM,		0x0000, 0x0fff, M6502_RAM);
	M6502MapMemory(DrvFgRAM,		0x1000, 0x17ff, M6502_RAM);
	M6502MapMemory(DrvBgRAM,		0x1800, 0x1fff, M6502_RAM);
	M6502MapMemory(Drv6502ROM + 0x4000,	0x4000, 0xffff, M6502_ROM);
	M6502SetWriteByteHandler(mystston_write);
	M6502SetReadByteHandler(mystston_read);
	M6502Close();

	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);

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

	BurnFree(AllMem);
	BurnFree(pAY8910Buf);

	return 0;
}

static void draw_bg_layer()
{
	INT32 page = (*video_control & 0x04) * 256;

	for (INT32 offs = 0; offs < 16 * 32; offs++)
	{
		INT32 sy = ((offs & 0x1f) * 16);
		INT32 sx = ((offs / 0x20) ^ 0x0f) * 16;

		sy -= *scrolly;
		if (sy < -15) sy += 256;

		INT32 code  = (DrvBgRAM[page + offs]) + ((DrvBgRAM[page + 0x200 + offs] & 0x01) * 256);
		INT32 flipy = offs & 0x10;
		INT32 flipx = 0;

		if (*flipscreen) {
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (flipy) {
			if (flipx) {
				Render16x16Tile_FlipXY_Clip(pTransDraw, code, sx, sy - 8, 0, 3, 0x10, DrvGfxROM1);
			} else {
				Render16x16Tile_FlipY_Clip(pTransDraw, code, sx, sy - 8, 0, 3, 0x10, DrvGfxROM1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, sy - 8, 0, 3, 0x10, DrvGfxROM1);
			} else {
				Render16x16Tile_Clip(pTransDraw, code, sx, sy - 8, 0, 3, 0x10, DrvGfxROM1);
			}
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0; offs < 0x60; offs += 4)
	{
		INT32 attr = DrvSprRAM[offs];

		if (attr & 0x01)
		{
			INT32 code = ((attr & 0x10) << 4) | DrvSprRAM[offs + 1];
			INT32 color = (attr & 0x08) >> 3;
			INT32 flipx = attr & 0x04;
			INT32 flipy = attr & 0x02;
			INT32 sx = 240 - DrvSprRAM[offs + 3];
			INT32 sy = (240 - DrvSprRAM[offs + 2]) & 0xff;

			if (*flipscreen)
			{
				sx = 240 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy - 8, color, 3, 0, 0, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy - 8, color, 3, 0, 0, DrvGfxROM2);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy - 8, color, 3, 0, 0, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy - 8, color, 3, 0, 0, DrvGfxROM2);
				}
			}
		}
	}
}

static void draw_fg_layer()
{
	INT32 color = ((*video_control & 1) << 1) + ((*video_control & 0x02) >> 1);

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sy = ((offs & 0x1f) * 8);
		INT32 sx = ((offs / 0x20) ^ 0x1f) * 8;

		INT32 code = DrvFgRAM[offs] + ((DrvFgRAM[0x400 + offs] & 0x07) * 256);
		INT32 flipxy = 0;

		if (*flipscreen) {
			sx = 248 - sx;
			sy = 248 - sy;
			flipxy = !flipxy;
		}

		if (flipxy) {
			Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy - 8, color, 3, 0, 0x20, DrvGfxROM0);
		} else {
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy - 8, color, 3, 0, 0x20, DrvGfxROM0);
		}
	}
}

static INT32 DrvDraw()
{
	*flipscreen = (*video_control & 0x80) ^ ((DrvDips[0] & 0x20) << 2);

	if (DrvRecalc) {
		DrvPaletteUpdate(DrvColPROM, 0x20);
		DrvRecalc = 0;
	}

	DrvPaletteUpdate(DrvPalRAM, 0);

	draw_bg_layer();
	draw_sprites();
	draw_fg_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static void mystston_interrupt_handler(INT32 scanline)
{
	static INT32 coin;
	INT32 inp = (DrvJoy1[6] << 6) | (DrvJoy1[7] << 7);

	if ((~inp & 0xc0) != 0xc0)
	{
		if (coin == 0)
		{
			coin = 1;
			M6502SetIRQ(M6502_INPUT_LINE_NMI, M6502_IRQSTATUS_AUTO);
			return;
		}
	}
	else coin = 0;

	if (scanline == 8) vblank = 0;
	if (scanline == 248) vblank = 0x80;
	if ((scanline & 0x0f) == 0) M6502SetIRQ(M6502_IRQ_LINE, M6502_IRQSTATUS_ACK);
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	if (pAY8910Buf == NULL) { // Refresh rate != 60
		SoundBufferAlloc();
	}

	{
		memset (DrvInputs, 0xff, 2);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nTotalCycles = (INT32)((double)(1500000 / 57.45));
	INT32 nCyclesRun = 0;

	vblank = 0x80;

	M6502Open(0);

	for (INT32 i = 0; i < 272; i++)
	{
		nCyclesRun += M6502Run(nTotalCycles / 272);
		mystston_interrupt_handler(i);
	}

	M6502Close();

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

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029722;
	}

	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		M6502Scan(nAction);
		AY8910Scan(nAction, pnMin);

		SCAN_VAR(ay8910_select);
	}

	return 0;
}


// Mysterious Stones - Dr. John's Adventure

static struct BurnRomInfo myststonRomDesc[] = {
	{ "rom6.bin",     0x2000, 0x7bd9c6cd, 1 | BRF_PRG | BRF_ESS }, //  0 M6205 Code
	{ "rom5.bin",     0x2000, 0xa83f04a6, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "rom4.bin",     0x2000, 0x46c73714, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "rom3.bin",     0x2000, 0x34f8b8a3, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "rom2.bin",     0x2000, 0xbfd22cfc, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "rom1.bin",     0x2000, 0xfb163e38, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "ms6",          0x2000, 0x85c83806, 2 | BRF_GRA }, 	       //  6 Character + Sprite Tiles
	{ "ms9",          0x2000, 0xb146c6ab, 2 | BRF_GRA },  	       //  7
	{ "ms7",          0x2000, 0xd025f84d, 2 | BRF_GRA },  	       //  8
	{ "ms10",         0x2000, 0xd85015b5, 2 | BRF_GRA },  	       //  9
	{ "ms8",          0x2000, 0x53765d89, 2 | BRF_GRA },  	       // 10
	{ "ms11",         0x2000, 0x919ee527, 2 | BRF_GRA },  	       // 11

	{ "ms12",         0x2000, 0x72d8331d, 3 | BRF_GRA },  	       // 12 Background Tiles
	{ "ms13",         0x2000, 0x845a1f9b, 3 | BRF_GRA },  	       // 13
	{ "ms14",         0x2000, 0x822874b0, 3 | BRF_GRA },  	       // 14 
	{ "ms15",         0x2000, 0x4594e53c, 3 | BRF_GRA },  	       // 15 
	{ "ms16",         0x2000, 0x2f470b0f, 3 | BRF_GRA },  	       // 16
	{ "ms17",         0x2000, 0x38966d1b, 3 | BRF_GRA },  	       // 17

	{ "ic61",         0x0020, 0xe802d6cf, 4 | BRF_GRA },  	       // 18 Color Prom
};

STD_ROM_PICK(mystston)
STD_ROM_FN(mystston)

struct BurnDriver BurnDrvmystston = {
	"mystston", NULL, NULL, NULL, "1984",
	"Mysterious Stones - Dr. John's Adventure\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TECHNOS, GBF_MAZE, 0,
	NULL, myststonRomInfo, myststonRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 64,
	240, 256, 3, 4
};


// Mysterious Stones - Dr. Kick in Adventure

static struct BurnRomInfo myststnoRomDesc[] = {
	{ "ms0",          0x2000, 0x6dacc05f, 1 | BRF_PRG | BRF_ESS }, //  0 M6205 Code
	{ "ms1",          0x2000, 0xa3546df7, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "ms2",          0x2000, 0x43bc6182, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "ms3",          0x2000, 0x9322222b, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "ms4",          0x2000, 0x47cefe9b, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "ms5",          0x2000, 0xb37ae12b, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "ms6",          0x2000, 0x85c83806, 2 | BRF_GRA }, 	       //  6 Character + Sprite Tiles
	{ "ms9",          0x2000, 0xb146c6ab, 2 | BRF_GRA },  	       //  7
	{ "ms7",          0x2000, 0xd025f84d, 2 | BRF_GRA },  	       //  8
	{ "ms10",         0x2000, 0xd85015b5, 2 | BRF_GRA },  	       //  9
	{ "ms8",          0x2000, 0x53765d89, 2 | BRF_GRA },  	       // 10
	{ "ms11",         0x2000, 0x919ee527, 2 | BRF_GRA },  	       // 11

	{ "ms12",         0x2000, 0x72d8331d, 3 | BRF_GRA },  	       // 12 Background Tiles
	{ "ms13",         0x2000, 0x845a1f9b, 3 | BRF_GRA },  	       // 13
	{ "ms14",         0x2000, 0x822874b0, 3 | BRF_GRA },  	       // 14 
	{ "ms15",         0x2000, 0x4594e53c, 3 | BRF_GRA },  	       // 15 
	{ "ms16",         0x2000, 0x2f470b0f, 3 | BRF_GRA },  	       // 16
	{ "ms17",         0x2000, 0x38966d1b, 3 | BRF_GRA },  	       // 17

	{ "ic61",         0x0020, 0xe802d6cf, 4 | BRF_GRA },  	       // 18 Color Prom
};

STD_ROM_PICK(myststno)
STD_ROM_FN(myststno)

struct BurnDriver BurnDrvmyststno = {
	"myststono", "mystston", NULL, NULL, "1984",
	"Mysterious Stones - Dr. Kick in Adventure\0", NULL, "Technos", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_TECHNOS, GBF_MAZE, 0,
	NULL, myststnoRomInfo, myststnoRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 64,
	240, 256, 3, 4
};
