// FB Alpha Skyfox Driver Module
// Based on MAME driver by Luca Elia

#include "tiles_generic.h"
#include "burn_ym2203.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvColPROM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvVidRegs;
static UINT32 *DrvPalette;
static UINT32 *Palette;

static UINT8 DrvRecalc;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[1];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[1];
static UINT8 DrvReset;

static UINT8 *soundlatch;
static UINT8 *DrvBgCtrl;
static INT32 *DrvBgPos;

static INT32 vblank;

static struct BurnInputInfo SkyfoxInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Skyfox)

static struct BurnDIPInfo SkyfoxDIPList[]=
{
	{0x09, 0xff, 0xff, 0x6f, NULL			},
	{0x0a, 0xff, 0xff, 0xf0, NULL			},
	{0x0b, 0xff, 0xff, 0xfa, NULL			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x09, 0x01, 0x18, 0x00, "20K"			},
	{0x09, 0x01, 0x18, 0x08, "30K"			},
	{0x09, 0x01, 0x18, 0x10, "40K"			},
	{0x09, 0x01, 0x18, 0x18, "50K"			},
	
	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x09, 0x01, 0x20, 0x20, "Medium"		},
	{0x09, 0x01, 0x20, 0x00, "Hard"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x09, 0x01, 0x40, 0x40, "Off"			},
	{0x09, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x09, 0x01, 0x80, 0x00, "Upright"		},
	{0x09, 0x01, 0x80, 0x80, "Cocktail"		},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x0a, 0x01, 0x0e, 0x0e, "5 Coins 1 Credits"	},
	{0x0a, 0x01, 0x0e, 0x0a, "4 Coins 1 Credits"	},
	{0x0a, 0x01, 0x0e, 0x06, "3 Coins 1 Credits"	},
	{0x0a, 0x01, 0x0e, 0x02, "2 Coins 1 Credits"	},
	{0x0a, 0x01, 0x0e, 0x00, "1 Coin  1 Credits"	},
	{0x0a, 0x01, 0x0e, 0x04, "1 Coin  2 Credits"	},
	{0x0a, 0x01, 0x0e, 0x08, "1 Coin  3 Credits"	},
	{0x0a, 0x01, 0x0e, 0x0c, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    6, "Lives"		},
	{0x0b, 0x01, 0x07, 0x00, "1"			},
	{0x0b, 0x01, 0x07, 0x01, "2"			},
	{0x0b, 0x01, 0x07, 0x02, "3"			},
	{0x0b, 0x01, 0x07, 0x03, "4"			},
	{0x0b, 0x01, 0x07, 0x04, "5"			},
	{0x0b, 0x01, 0x07, 0x07, "Infinite (Cheat)"	},
};

STDDIPINFO(Skyfox)

void __fastcall skyfox_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe008:
		case 0xe009:
		case 0xe00a:
		case 0xe00b:
		case 0xe00c:
		case 0xe00d:
		case 0xe00e:
		case 0xe00f:
			DrvVidRegs[address & 7] = data;
		return;
	}
}

UINT8 __fastcall skyfox_read(UINT16 address)
{
	switch (address)
	{
		case 0xe000:
			return DrvInputs[0];

		case 0xe001:
			return DrvDips[0];

		case 0xe002:
			return DrvDips[1] | vblank;

		case 0xf001:
			return DrvDips[2];
	}

	return 0;
}

void __fastcall skyfox_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xa000:
		case 0xa001:
			BurnYM2203Write(0, address & 1, data);
			return;

		case 0xc000:
		case 0xc001:
			BurnYM2203Write(1, address & 1, data);
			return;
	}
}

UINT8 __fastcall skyfox_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xa001:
			return BurnYM2203Read(0, 0);

		case 0xb000:
			return *soundlatch;
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x010000;
	DrvZ80ROM1	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x060000;
	DrvGfxROM1	= Next; Next += 0x008000;

	DrvColPROM	= Next; Next += 0x000300;

	DrvPalette	= (UINT32*)Next; Next += 0x0200 * sizeof(UINT32);
	Palette		= (UINT32*)Next; Next += 0x0200 * sizeof(UINT32);

	AllRam		= Next;

	DrvZ80RAM0	= Next; Next += 0x001000;
	DrvZ80RAM1	= Next; Next += 0x000800;
	DrvSprRAM	= Next; Next += 0x001000;

	DrvVidRegs	= Next;
	DrvBgCtrl	= Next;
	soundlatch	= Next + 1; Next += 0x000008;

	DrvBgPos	= (INT32*)Next; Next += 0x00001 * sizeof(INT32);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static void DrvGfxDecode()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x60000);

	for (INT32 i = 0; i < 0x60000; i++) {
		tmp[i] = DrvGfxROM0[(i&~0x3ff) | (i&7) | (((i>>3)&7)<<5) | (((i>>6)&3)<<3) | (i&0x300)];
	}

	memcpy (DrvGfxROM0, tmp, 0x60000);

	BurnFree (tmp);
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 256; i++)
	{
		INT32 bit0,bit1,bit2,bit3,r,g,b;

		bit0 = (DrvColPROM[i] >> 0) & 0x01;
		bit1 = (DrvColPROM[i] >> 1) & 0x01;
		bit2 = (DrvColPROM[i] >> 2) & 0x01;
		bit3 = (DrvColPROM[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[i + 256] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + 256] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + 256] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + 256] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[i + 2*256] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + 2*256] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + 2*256] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + 2*256] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		Palette[i] = (r << 16) | (g << 8) | b;
	}

	for (INT32 i = 0; i < 256; i++)
	{
		Palette[i | 0x100] = (i << 16) | (i << 8) | i;
	}
}

static INT32 DrvDoReset()
{
	memset(AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	BurnYM2203Reset();

	return 0;
}

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 1748000;
}

inline static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 1748000.0;
}

static INT32 DrvInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvZ80ROM0 + 0x00000,  0, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x08000,  1, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM1 + 0x00000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,  3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x10000,  4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x20000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x30000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x40000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x50000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  9, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x00000, 10, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x00100, 11, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x00200, 12, 1)) return 1;

		DrvPaletteInit();
		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xbfff, 1, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xcfff, 0, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xcfff, 1, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xcfff, 2, DrvZ80RAM0);
	ZetMapArea(0xd000, 0xdfff, 0, DrvSprRAM);
	ZetMapArea(0xd000, 0xdfff, 1, DrvSprRAM);
	ZetMapArea(0xd000, 0xdfff, 2, DrvSprRAM);
	ZetSetWriteHandler(skyfox_write);
	ZetSetReadHandler(skyfox_read);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM1);
	ZetSetWriteHandler(skyfox_sound_write);
	ZetSetReadHandler(skyfox_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2203Init(2, 1748000, NULL, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(1748000);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	BurnYM2203Exit();
	GenericTilesExit();
	ZetExit();

	BurnFree (AllMem);

	return 0;
}

static void draw_sprites()
{
	INT32 shift = (*DrvBgCtrl & 0x80) ? (4-1) : 4;

	for (INT32 offs = 0; offs < 0x400; offs += 4)
	{
		INT32 xstart, ystart, xend, yend;
		INT32 xinc, yinc, dx, dy;
		INT32 low_code, n;

		INT32 y     = DrvSprRAM[offs + 0];
		INT32 x     = DrvSprRAM[offs + 1];
		INT32 code  = DrvSprRAM[offs + 2] | (DrvSprRAM[offs+3] << 8);
		INT32 flipx = code & 0x02;
		INT32 flipy = code & 0x04;

		x = (x << 1) | (code & 1);

		INT32 high_code = ((code >> 4) & 0x7f0) +	((code & 0x8000) >> shift);

		switch (code & 0x88)
		{
			case 0x88:	n = 4;	low_code = 0;						break;
			case 0x08:	n = 2;	low_code = ((code & 0x20) >> 2) + ((code & 0x10) >> 3);	break;
			default:	n = 1;	low_code = (code >> 4) & 0x0f;				break;
		}

		if (*DrvBgCtrl & 1)
		{
			x = ((nScreenWidth-1)  - x - ((n - 1) << 3)) + 88;
			y = ((nScreenHeight-1) - y - ((n - 1) << 3)) + 8;
			flipx = !flipx;
			flipy = !flipy;
		} else {
			x -= 0x60;
			y -= 0x10;
		}

		if (flipx) { xstart = n-1;  xend = -1;  xinc = -1; }
		else	   { xstart = 0;    xend = n;   xinc = +1; }

		if (flipy) { ystart = n-1;  yend = -1;  yinc = -1; }
		else	   { ystart = 0;    yend = n;   yinc = +1; }

		code = low_code + high_code;

		for (dy = ystart; dy != yend; dy += yinc)
		{
			INT32 sy = y + (dy << 3);

			for (dx = xstart; dx != xend; dx += xinc)
			{
				INT32 sx = x + (dx << 3);

				if (flipy) {
					if (flipx) {
						Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code++, sx, sy, 0, 8, 0xff, 0, DrvGfxROM0);
					} else {
						Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code++, sx, sy, 0, 8, 0xff, 0, DrvGfxROM0);
					}
				} else {
					if (flipx) {
						Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code++,sx, sy, 0, 8, 0xff, 0, DrvGfxROM0);
					} else {
						Render8x8Tile_Mask_Clip(pTransDraw, code++, sx, sy, 0, 8, 0xff, 0, DrvGfxROM0);
					}
				}
			}

			code += n & 2;
		}
	}
}

static void draw_background()
{
	INT32 pos = (*DrvBgPos >> 4) & 0x3ff;

	for (INT32 i = 0; i < 0x1000; i++)
	{
		INT32 offs = ((*DrvBgCtrl << 9) & 0x6000) + (i << 1);
		INT32 attr = DrvGfxROM1[offs];
		INT32 x    = (DrvGfxROM1[offs + 1] << 1) + (i & 1) + pos + ((i & 8) << 6);
		INT32 y    = ((i >> 4) << 3) | (i & 7);
		INT32 pen	 = (attr & 0x7f) | 0x100;

		if (*DrvBgCtrl & 1)
		{
			x = 0x400 - (x & 0x3ff);
			y = 0x100 - (y & 0x0ff);
		}

		for (INT32 j = 0; j <= ((attr & 0x80) ? 0 : 3); j++)
		{
			INT32 sx = ((((j >> 0) & 1) + x) & 0x1ff) - 0x60;
			INT32 sy = ((((j >> 1) & 1) + y) & 0x0ff) - 0x10;

			if (sx < 0 || sy < 0 || sx >= nScreenWidth || sy >= nScreenHeight) continue;

			pTransDraw[sy * nScreenWidth + sx] = pen;
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x200; i++) {
			INT32 p = Palette[i];
			DrvPalette[i] =  BurnHighCol(p >> 16, p >> 8, p, 0);
		}
	}

	for (INT32 offs = 0; offs < nScreenWidth * nScreenHeight; offs++) {
		pTransDraw[offs] = 0x00ff;
	}

	draw_background();
	draw_sprites();

	BurnTransferCopy(DrvPalette);

	*DrvBgPos += (*DrvBgCtrl >> 1) & 7;

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = 0xff;
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
		}
	}

	ZetNewFrame();
	vblank = 0;

	INT32 nCycleSegment;
	INT32 nInterleave = 256;
	INT32 nCyclesTotal[2] = { 4000000 / 60, 1748000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		if (i == 8 || i == 248) vblank ^= 1;

		nCycleSegment = nCyclesTotal[0] / nInterleave;

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nCycleSegment);
		if (i == (nInterleave-1) && DrvJoy2[0]) ZetNmi();
		ZetClose();

		nCycleSegment = nCyclesTotal[1] / nInterleave;

		ZetOpen(1);
		//nCyclesDone[1] += ZetRun(nCycleSegment);
		BurnTimerUpdate(i * nCycleSegment);
		ZetClose();
	}

	ZetOpen(1);
	BurnTimerEndFrame(nCyclesTotal[1]);
	if (pBurnSoundOut) BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029702;
	}

	if (nAction & ACB_VOLATILE) {
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All RAM";
		BurnAcb(&ba);

		ZetScan(nAction);
		BurnYM2203Scan(nAction, pnMin);
	}

	return 0;
}


// Sky Fox

static struct BurnRomInfo skyfoxRomDesc[] = {
	{ "skyfox1.bin",	0x08000, 0xb4d4bb6f, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "skyfox2.bin",	0x08000, 0xe15e0263, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "skyfox9.bin",	0x08000, 0x0b283bf5, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 #1 Code

	{ "skyfox3.bin",	0x10000, 0x3a17a929, 3 | BRF_GRA },           //  3 Sprites
	{ "skyfox4.bin",	0x10000, 0x358053bb, 3 | BRF_GRA },           //  4
	{ "skyfox5.bin",	0x10000, 0xc1215a6e, 3 | BRF_GRA },           //  5
	{ "skyfox6.bin",	0x10000, 0xcc37e15d, 3 | BRF_GRA },           //  6
	{ "skyfox7.bin",	0x10000, 0xfa2ab5b4, 3 | BRF_GRA },           //  7
	{ "skyfox8.bin",	0x10000, 0x0e3edc49, 3 | BRF_GRA },           //  8

	{ "skyfox10.bin",	0x08000, 0x19f58f9c, 4 | BRF_GRA },           //  9 Starfield

	{ "sfoxrprm.bin",	0x00100, 0x79913c7f, 5 | BRF_GRA },           // 10 Color Proms
	{ "sfoxgprm.bin",	0x00100, 0xfb73d434, 5 | BRF_GRA },           // 11
	{ "sfoxbprm.bin",	0x00100, 0x60d2ab41, 5 | BRF_GRA },           // 12
};

STD_ROM_PICK(skyfox)
STD_ROM_FN(skyfox)

struct BurnDriver BurnDrvSkyfox = {
	"skyfox", NULL, NULL, NULL, "1987",
	"Sky Fox\0", NULL, "Jaleco (Nichibutsu USA license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 1, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, skyfoxRomInfo, skyfoxRomName, NULL, NULL, SkyfoxInputInfo, SkyfoxDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	224, 320, 3, 4
};


// Exerizer (Japan) (bootleg)

static struct BurnRomInfo exerizrbRomDesc[] = {
	{ "1-a",		0x08000, 0x5df72a5d, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "skyfox2.bin",	0x08000, 0xe15e0263, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "skyfox9.bin",	0x08000, 0x0b283bf5, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 #1 Code

	{ "1-c",		0x10000, 0x450e9381, 3 | BRF_GRA },           //  3 Sprites
	{ "skyfox4.bin",	0x10000, 0x358053bb, 3 | BRF_GRA },           //  4
	{ "1-e",		0x10000, 0x50a38c60, 3 | BRF_GRA },           //  5
	{ "skyfox6.bin",	0x10000, 0xcc37e15d, 3 | BRF_GRA },           //  6
	{ "1-g",		0x10000, 0xc9bbfe5c, 3 | BRF_GRA },           //  7
	{ "skyfox8.bin",	0x10000, 0x0e3edc49, 3 | BRF_GRA },           //  8

	{ "skyfox10.bin",	0x08000, 0x19f58f9c, 4 | BRF_GRA },           //  9 Starfield

	{ "sfoxrprm.bin",	0x00100, 0x79913c7f, 5 | BRF_GRA },           // 10 Color Proms
	{ "sfoxgprm.bin",	0x00100, 0xfb73d434, 5 | BRF_GRA },           // 11
	{ "sfoxbprm.bin",	0x00100, 0x60d2ab41, 5 | BRF_GRA },           // 12
};

STD_ROM_PICK(exerizrb)
STD_ROM_FN(exerizrb)

struct BurnDriver BurnDrvExerizrb = {
	"exerizerb", "skyfox", NULL, NULL, "1987",
	"Exerizer (Japan) (bootleg)\0", NULL, "Jaleco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 1, HARDWARE_MISC_PRE90S, GBF_VERSHOOT, 0,
	NULL, exerizrbRomInfo, exerizrbRomName, NULL, NULL, SkyfoxInputInfo, SkyfoxDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x200,
	224, 320, 3, 4
};
