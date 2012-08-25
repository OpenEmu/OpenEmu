// FB Alpha Pocket Gal deluxe driver module
// Based on MAME driver by David Haywood and Bryan McPhail

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "deco16ic.h"
#include "msm6295.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *Drv68KCode;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *Drv68KRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvProtRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *flipscreen;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT16 DrvInputs[2];

static struct BurnInputInfo PktgaldxInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},

	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 13,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 14,	"p2 fire 3"}	,

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Pktgaldx)

static struct BurnDIPInfo PktgaldxDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL					},
	{0x15, 0xff, 0xff, 0x7f, NULL					},

	{0   , 0xfe, 0   ,    8, "Coin A"				},
	{0x14, 0x01, 0x07, 0x00, "3 Coins 1 Credits"			},
	{0x14, 0x01, 0x07, 0x01, "2 Coins 1 Credits"			},
	{0x14, 0x01, 0x07, 0x07, "1 Coin  1 Credits"			},
	{0x14, 0x01, 0x07, 0x06, "1 Coin  2 Credits"			},
	{0x14, 0x01, 0x07, 0x05, "1 Coin  3 Credits"			},
	{0x14, 0x01, 0x07, 0x04, "1 Coin  4 Credits"			},
	{0x14, 0x01, 0x07, 0x03, "1 Coin  5 Credits"			},
	{0x14, 0x01, 0x07, 0x02, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    8, "Coin B"				},
	{0x14, 0x01, 0x38, 0x00, "3 Coins 1 Credits"			},
	{0x14, 0x01, 0x38, 0x08, "2 Coins 1 Credits"			},
	{0x14, 0x01, 0x38, 0x38, "1 Coin  1 Credits"			},
	{0x14, 0x01, 0x38, 0x30, "1 Coin  2 Credits"			},
	{0x14, 0x01, 0x38, 0x28, "1 Coin  3 Credits"			},
	{0x14, 0x01, 0x38, 0x20, "1 Coin  4 Credits"			},
	{0x14, 0x01, 0x38, 0x18, "1 Coin  5 Credits"			},
	{0x14, 0x01, 0x38, 0x10, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x14, 0x01, 0x40, 0x40, "Off"					},
	{0x14, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "2 Coins to Start, 1 to Continue"	},
	{0x14, 0x01, 0x80, 0x80, "Off"					},
	{0x14, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x15, 0x01, 0x03, 0x00, "2"					},
	{0x15, 0x01, 0x03, 0x01, "3"					},
	{0x15, 0x01, 0x03, 0x03, "4"					},
	{0x15, 0x01, 0x03, 0x02, "5"					},

	{0   , 0xfe, 0   ,    4, "Time"					},
	{0x15, 0x01, 0x0c, 0x00, "60"					},
	{0x15, 0x01, 0x0c, 0x04, "80"					},
	{0x15, 0x01, 0x0c, 0x0c, "100"					},
	{0x15, 0x01, 0x0c, 0x08, "120"					},

	{0   , 0xfe, 0   ,    2, "Free Play"				},
	{0x15, 0x01, 0x20, 0x20, "Off"					},
	{0x15, 0x01, 0x20, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x15, 0x01, 0x80, 0x80, "Off"					},
	{0x15, 0x01, 0x80, 0x00, "On"					},
};

STDDIPINFO(Pktgaldx)

void __fastcall pktgaldx_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffff0) == 0x140000) {
		MSM6295Command(0, data);
		return;
	}

	if ((address & 0xfffff0) == 0x160000) {
		MSM6295Command(1, data);
		return;
	}

	deco16_write_control_word(0, address, 0x161800, data)

	if ((address & 0xfffff0) == 0x164800) {
		memcpy (DrvSndROM0 + 0x100000, DrvSndROM1 + (data & 3) * 0x40000, 0x40000);
		return;
	}
}

void __fastcall pktgaldx_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xfffff0) == 0x140000) {
		MSM6295Command(0, data);
		return;
	}

	if ((address & 0xfffff0) == 0x160000) {
		MSM6295Command(1, data);
		return;
	}

	deco16_write_control_word(0, address, 0x161800, data)

	if ((address & 0xfffff0) == 0x164800) {
		memcpy (DrvSndROM0 + 0x100000, DrvSndROM1 + (data & 3) * 0x40000, 0x40000);
		return;
	}
}

UINT16 __fastcall pktgaldx_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x140006:
			return MSM6295ReadStatus(0);

		case 0x150006:
			return MSM6295ReadStatus(1);

		case 0x167db2:
			return (DrvInputs[0] & 0xfff7) | (deco16_vblank & 0x08);

		case 0x167c4c:
			return (DrvDips[1] << 8) | (DrvDips[0] << 0);

		case 0x167842:
			return DrvInputs[1];

		case 0x167d10:
			return BURN_ENDIAN_SWAP_INT16(*((UINT16*)(DrvProtRAM + 0)));

		case 0x167d1a:
			return BURN_ENDIAN_SWAP_INT16(*((UINT16*)(DrvProtRAM + 2)));
	}

	return 0;
}

UINT8 __fastcall pktgaldx_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x140006:
		case 0x140007:
			return MSM6295ReadStatus(0);

		case 0x150006:
		case 0x150007:
			return MSM6295ReadStatus(1);

		case 0x167db2:
		case 0x167db3:
			return (DrvInputs[0] & 0xf7) | (deco16_vblank & 0x08);

		case 0x167c4c:
			return DrvDips[1];

		case 0x167c4d:
			return DrvDips[0];

		case 0x167842:
			return DrvInputs[1] >> 8;

		case 0x167843:
			return DrvInputs[1] >> 0;

		case 0x167d10:
		case 0x167d11:
			return DrvProtRAM[((address & 1) ^ 1) + 0];

		case 0x167d1a:
		case 0x167d1b:
			return DrvProtRAM[((address & 1) ^ 1) + 2];
		
	}

	return 0;
}

static INT32 pktgaldx_bank_callback( const INT32 bank )
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	MSM6295Reset(0);
	MSM6295Reset(1);

	deco16Reset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x080000;
	Drv68KCode	= Next; Next += 0x080000;

	DrvGfxROM0	= Next; Next += 0x200000;
	DrvGfxROM1	= Next; Next += 0x200000;
	DrvGfxROM2	= Next; Next += 0x200000;

	MSM6295ROM	= Next;
	DrvSndROM0	= Next; Next += 0x140000;
	DrvSndROM1	= Next; Next += 0x100000;

	DrvPalette	= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvSprRAM	= Next; Next += 0x000800;
	DrvPalRAM	= Next; Next += 0x001000;
	DrvProtRAM	= Next; Next += 0x000800;

	flipscreen	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvInit()
{
	BurnSetRefreshRate(58.00);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x000000,  0, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  1, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000,  2, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x000001,  3, 2)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000,  4, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x000000,  5, 1)) return 1;

		deco102_decrypt_cpu(Drv68KROM, Drv68KCode, 0x80000, 0x42ba, 0x00, 0x00);

		deco56_decrypt_gfx(DrvGfxROM1, 0x100000);

		deco16_tile_decode(DrvGfxROM1, DrvGfxROM0, 0x100000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x100000, 0);
		deco16_sprite_decode(DrvGfxROM2, 0x100000);
	}	

	deco16Init(1, 0, 1);
	deco16_set_graphics(DrvGfxROM0, 0x100000 * 2, DrvGfxROM1, 0x100000 * 2, NULL, 0);
	deco16_set_global_offsets(0, 8);
	deco16_set_bank_callback(1, pktgaldx_bank_callback);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x07ffff, SM_READ);
	SekMapMemory(Drv68KCode,		0x000000, 0x07ffff, SM_FETCH);
	SekMapMemory(deco16_pf_ram[0],		0x100000, 0x100fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x102000, 0x102fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x110000, 0x1107ff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x112000, 0x1127ff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x120000, 0x1207ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x130000, 0x130fff, SM_RAM);
	SekMapMemory(DrvProtRAM,		0x167800, 0x167fff, SM_WRITE);
	SekMapMemory(Drv68KRAM,			0x170000, 0x17ffff, SM_RAM);
	SekSetWriteWordHandler(0,		pktgaldx_write_word);
	SekSetWriteByteHandler(0,		pktgaldx_write_byte);
	SekSetReadWordHandler(0,		pktgaldx_read_word);
	SekSetReadByteHandler(0,		pktgaldx_read_byte);
	SekClose();

	MSM6295Init(0, 1006875 / 132, 1);
	MSM6295Init(1, 2013750 / 132, 1);
	MSM6295SetRoute(0, 0.75, BURN_SND_ROUTE_BOTH);
	MSM6295SetRoute(1, 0.60, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();
	deco16Exit();

	MSM6295Exit(0);
	MSM6295Exit(1);

	SekExit();

	BurnFree (AllMem);

	MSM6295ROM = NULL;

	return 0;
}

static void draw_sprites()
{
	UINT16 *spriteram = (UINT16 *)DrvSprRAM;

	for (INT32 offs = 0; offs < 0x400; offs += 4)
	{
		INT32 inc, mult;

		INT32 sprite = BURN_ENDIAN_SWAP_INT16(spriteram[offs+1]);
		if (!sprite) continue;

		INT32 y = BURN_ENDIAN_SWAP_INT16(spriteram[offs]);

		if ((y & 0x1000) && (nCurrentFrame & 1)) continue; // flash

		INT32 x = BURN_ENDIAN_SWAP_INT16(spriteram[offs + 2]);
		INT32 colour = (x >> 9) & 0x1f;

		INT32 fx = y & 0x2000;
		INT32 fy = y & 0x4000;
		INT32 multi = (1 << ((y & 0x0600) >> 9)) - 1;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;
		y = 240 - y;
		x = 304 - x;

		if (x > 320) continue;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (!*flipscreen)
		{
			y = 240 - y;
			x = 304 - x;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = 16;
		}
		else mult = -16;

		while (multi >= 0)
		{
			if (fy) {
				if (fx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, sprite - multi * inc, x, (y + mult * multi) - 8, colour, 4, 0, 0x200, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, sprite - multi * inc, x, (y + mult * multi) - 8, colour, 4, 0, 0x200, DrvGfxROM2);
				}
			} else {
				if (fx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, sprite - multi * inc, x, (y + mult * multi) - 8, colour, 4, 0, 0x200, DrvGfxROM2);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, sprite - multi * inc, x, (y + mult * multi) - 8, colour, 4, 0, 0x200, DrvGfxROM2);
				}
			}

			multi--;
		}
	}
}

static INT32 DrvDraw()
{
//	if (DrvRecalc) {
		deco16_palette_recalculate(DrvPalette, DrvPalRAM);
		DrvRecalc = 0;
//	}

	deco16_pf12_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 00;
	}

	if (nBurnLayer & 1) deco16_draw_layer(1, pTransDraw, 0);

	if (nBurnLayer & 2) draw_sprites();

	if (nBurnLayer & 4) deco16_draw_layer(0, pTransDraw, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 2 * sizeof(UINT16)); 
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nInterleave = 256;
	INT32 nCyclesTotal[1] = { 14000000 / 58 };
	INT32 nCyclesDone[1] = { 0 };

	SekOpen(0);

	deco16_vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);

		if (i == 240) deco16_vblank = 0x08;
	}

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);

	SekClose();

	if (pBurnSoundOut) {
		memset (pBurnSoundOut, 0, nBurnSoundLen * 2 * sizeof(INT16));
		MSM6295Render(1, pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029682;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);

		deco16Scan();

		MSM6295Scan(0, nAction);
		MSM6295Scan(1, nAction);
	}

	return 0;
}


// Pocket Gal Deluxe (Euro v3.00)

static struct BurnRomInfo pktgaldxRomDesc[] = {
	{ "ke00-2.12a",		0x080000, 0xb04baf3a, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code

	{ "maz-02.2h",		0x100000, 0xc9d35a59, 2 | BRF_GRA },           //  1 Character and Background Tiles

	{ "maz-00.1b",		0x080000, 0xfa3071f4, 3 | BRF_GRA },           //  2 Sprites
	{ "maz-01.3b",		0x080000, 0x4934fe21, 3 | BRF_GRA },           //  3

	{ "ke01.14f",		0x020000, 0x8a106263, 4 | BRF_SND },           //  4 OKI M6295 Sample 0
	
	{ "maz-03.13f",		0x100000, 0xa313c964, 5 | BRF_SND },           //  5 OKI M6295 Sample 1
};

STD_ROM_PICK(pktgaldx)
STD_ROM_FN(pktgaldx)

struct BurnDriver BurnDrvPktgaldx = {
	"pktgaldx", NULL, NULL, NULL, "1992",
	"Pocket Gal Deluxe (Euro v3.00)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_DATAEAST, GBF_MISC, 0,
	NULL, pktgaldxRomInfo, pktgaldxRomName, NULL, NULL, PktgaldxInputInfo, PktgaldxDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 240, 4, 3
};


// Pocket Gal Deluxe (Japan v3.00)

static struct BurnRomInfo pktgaldxjRomDesc[] = {
	{ "kg00-2.12a",		0x080000, 0x62dc4137, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code

	{ "maz-02.2h",		0x100000, 0xc9d35a59, 2 | BRF_GRA },           //  1 Character and Background Tiles

	{ "maz-00.1b",		0x080000, 0xfa3071f4, 3 | BRF_GRA },           //  2 Sprites
	{ "maz-01.3b",		0x080000, 0x4934fe21, 3 | BRF_GRA },           //  3

	{ "ke01.14f",		0x020000, 0x8a106263, 4 | BRF_SND },           //  4 OKI M6295 Sample 0

	{ "maz-03.13f",		0x100000, 0xa313c964, 5 | BRF_SND },           //  5 OKI M6295 Sample 1
};

STD_ROM_PICK(pktgaldxj)
STD_ROM_FN(pktgaldxj)

struct BurnDriver BurnDrvPktgaldxj = {
	"pktgaldxj", "pktgaldx", NULL, NULL, "1993",
	"Pocket Gal Deluxe (Japan v3.00)\0", NULL, "Nihon System", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_MISC, 0,
	NULL, pktgaldxjRomInfo, pktgaldxjRomName, NULL, NULL, PktgaldxInputInfo, PktgaldxDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 240, 4, 3
};


// Pocket Gal Deluxe (Euro v3.00, bootleg)

static struct BurnRomInfo pktgaldxbRomDesc[] = {
	{ "4.bin",		0x80000, 0x67ce30aa, 1 | BRF_PRG | BRF_ESS }, //  0 maincpu
	{ "5.bin",		0x80000, 0x64cb4c33, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "11.bin",		0x80000, 0xa8c8f1fd, 2 | BRF_GRA },           //  2 Sprites
	{ "6.bin",		0x80000, 0x0e3335a1, 2 | BRF_GRA },           //  3
	{ "10.bin",		0x80000, 0x9dd743a9, 2 | BRF_GRA },           //  4
	{ "7.bin",		0x80000, 0x0ebf12b5, 2 | BRF_GRA },           //  5
	{ "9.bin",		0x80000, 0x078f371c, 2 | BRF_GRA },           //  6
	{ "8.bin",		0x80000, 0x40f5a032, 2 | BRF_GRA },           //  7

	{ "kg01.14f",		0x20000, 0x8a106263, 3 | BRF_SND },           //  8 OKI M6295 Sample 0

	{ "3.bin",		0x80000, 0x4638747b, 4 | BRF_SND },           //  9 OKI M6295 Sample 1
	{ "2.bin",		0x80000, 0xf841d995, 4 | BRF_SND },           // 10
};

STD_ROM_PICK(pktgaldxb)
STD_ROM_FN(pktgaldxb)

static INT32 pkgaldxbInit()
{
	return 1;
}

struct BurnDriverD BurnDrvPktgaldxb = {
	"pktgaldxb", "pktgaldx", NULL, NULL, "1992",
	"Pocket Gal Deluxe (Euro v3.00, bootleg)\0", NULL, "bootleg", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_PREFIX_DATAEAST, GBF_MISC, 0,
	NULL, pktgaldxbRomInfo, pktgaldxbRomName, NULL, NULL, PktgaldxInputInfo, PktgaldxDIPInfo,
	pkgaldxbInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	320, 240, 4, 3
};
